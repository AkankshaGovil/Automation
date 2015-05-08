#include "gis.h"
#include "age.h"
#include "db.h"
#include "tags.h"
#include "lsprocess.h"
#include "log.h"
#include "phonode.h"
#include "gk.h"
#include "dbs.h"
#include "ls.h"
#include "server.h"

static int _netfd;
extern int uh323init, uh323ready;

void *
LsAgeCache(void *arg)
{
	char fn[] = "LsAgeCache():";
	int i;
	CacheTableInfo *info = 0, *nextInfo = 0, *tmpInfo, cacheInfoEntry, nextCacheInfoEntry;
	CacheVpnEntry *cacheVpnEntry;
	CacheVpnGEntry *cacheVpnGEntry;
	CacheVpnRouteEntry *cacheRouteEntry;
	PhoNode phonode;
	int isproxyc = 0, present = 0, nextPresent = 0;
	time_t presentTime, lastTime, delta;
	
	if (uh323init >= 0)
	{
		UH323GlobalsAttach(0);

		// This should actually eliminate the need to do
		// checks below, but that is not too much of a concern
		// in the age part
		while (uh323ready < 0)
		{
			NETDEBUG(MAGE, NETLOG_DEBUG4, 
				("%s Waiting for H.323 to initialize", fn));
			sleep(5);
		}

		while (IServerIsSecondary())
		{
			NETDEBUG(MAGE, NETLOG_DEBUG4,
				("%s Waiting for VIP to initialize", fn));
			sleep(5);
		}
	}

	NETDEBUG(MAGE, NETLOG_DEBUG4, 
		("%s Age Thread starting up...", fn));

	// Set up the two buffer areas, one to store current and
	// the other to store future.
	info = &cacheInfoEntry;
	nextInfo = &nextCacheInfoEntry;

	while (1)
	{
		time(&presentTime);
	
		NETDEBUG(MAGE, NETLOG_DEBUG4, ("%s Examining Iedge Cache", fn));
	
		if (uh323ready >= 0)
		{
			if (cmMeiEnter(UH323Globals()->hApp) < 0)
			{
				NETERROR(MH323, ("uh323Stack locks failed!!!\n"));
			}
		}

		// Keep the locks here, as an entry might get deleted
		// while we are walking, causing us to skip everything else

		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		present = CacheFindFirst(ipCache, info, sizeof(CacheTableInfo));
	
		while (present > 0)
		{
			 /* Compute the next one, as this entry might get deleted */
			 nextPresent = CacheFindNext(ipCache, &info->data, nextInfo, 
								sizeof(CacheTableInfo));
	
			if ((info->data.stateFlags & CL_ACTIVE) &&
				IsAged((info->data.rTime), presentTime) &&
				!IsSGatekeeper(&info->data))
			{
				NETDEBUG(MAGE, NETLOG_DEBUG4,
				("%s This Endpoint is Aged Now \n", fn));
	
				NETDEBUG(MAGE, NETLOG_DEBUG4,
				("rTime = %ld, presentTime = %ld, diff = %ld",
				(info->data.rTime),
				presentTime, 
				presentTime-(info->data.rTime)));
		
				/* Delete this Netoid */
				DEBUG_PrintInfoEntry(MAGE, NETLOG_DEBUG4, 
					&info->data);

				HandleAgedIedgeIpAddr(&info->data);
				  
				InitPhonodeFromInfoEntry(&info->data, &phonode);
						
				if (!IServerIsSecondary())
				{
			 		if (BIT_TEST(info->data.cap, CAP_UCC))
				  	{
						SrvrReportErrorToPhoNode(XERRT_SERPLEX,
						XERRS_REGISTER, 0, &phonode);	
				  	}
	
			 	  	if ((uh323ready >= 0) &&
						(BIT_TEST(info->data.cap, CAP_IRQ)))
				  	{
						/* Send URQ */
						GkSendURQ(&info->data);
				  	}
				}
	
				  goto _next;
			 }

		_next:
			 present = nextPresent;

			 // Swap info and nextInfo
			 tmpInfo = info;
			 info = nextInfo;
			 nextInfo = tmpInfo;
		}

		CacheReleaseLocks(regCache);

		if (uh323ready >= 0)
		{
			cmMeiExit(UH323Globals()->hApp);
		}
	
	_continue:
		lastTime = presentTime;
		time(&presentTime);
		delta = cacheTimeout + lastTime - presentTime;
		if (delta > 0)
		{
			NETDEBUG(MAGE, NETLOG_DEBUG4, ("%s sleeping for %lds", fn, delta));

			// sleep here
			sleep(delta);
		}
	}
}

int
HandleAgedIedgeIpAddr(NetoidInfoEntry *infoEntry)
{
	PhoNode phonode;
	CacheTableInfo *info = 0;
	InfoEntry tmpInfo;
	int i;
	char tags[TAGH_LEN] = { 0 };

	/* This aged ip address represents a bunch of 
	 * iedge ports, which we must now disable.
	 * Using the ports mentioned in this iedge, 
	 * traverse the regCache, and disable the ports
	 */
	memcpy(&phonode, infoEntry, REG_ID_LEN);

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (i=0; i<MAX_IEDGE_PORTS; i++)
	{
		 if (BITA_TEST(infoEntry->ports, i))
		 {
			  /* Look for this iedge port */
			  phonode.uport = i;

			  info = (CacheTableInfo *)CacheGet(regCache, &phonode);

			  if (info)
			  {
					/* This iedge port is to be aged now */

					/* We must mark in the db that this
					* netoid is inactive now
					*/
					if (!(info->data.stateFlags & CL_STATIC))
					{
						memset(info->data.contact, 0, SIPURL_LEN);
					}
					info->data.stateFlags &= ~CL_ACTIVE;
					time(&info->data.iaTime);

					BITA_SET(tags, TAG_REGSTATUS);

					/* Update the database also */
					//UpdateNetoidDatabase(&info->data);
					DbScheduleIedgeUpdate(&info->data);
					GisPostCliIedgeRegCmd(info->data.regid, 
						info->data.uport, tags);

					if (!(info->data.stateFlags & CL_STATIC))
					{
						if (!(info->data.stateFlags & CL_DYNAMIC))
						{
							DeleteIedgeIpAddr(ipCache, &info->data);
						}
						else
						{
							DeleteIedge(&info->data);

							if (!IServerIsSecondary())
							{
								infoEntry->uport = i;
								DbScheduleIedgeDelete(infoEntry);
							}
						}
					}
			  }
		 }
	}			  
	
	CacheReleaseLocks(regCache);
	return(1);
}

int
SrvrReportErrorToPhoNode(int errtype, int errcode, unsigned long myip,
				PhoNode *phonodep)
{
	char fn[] = "SrvrReportErrorToPhoNode():";
	Pkt pkt;
	struct sockaddr_in addr;

	if (!BIT_TEST(phonodep->sflags, ISSET_IPADDRESS))
	{
		return 0;
	}

	pkt.type = PKT_ERROR;
	pkt.totalLen = (sizeof(Pkt));
	
	pkt.data.errormsg.node.type = (NODE_PHONODE);
	memcpy(&pkt.data.errormsg.node.unode.phonode,
		phonodep, sizeof(PhoNode));

	pkt.data.errormsg.snode.type = (NODE_SRVR);

	pkt.data.errormsg.snode.unode.srvr.ipaddress = myip;
	pkt.data.errormsg.snode.unode.srvr.cport = (ALOID_LOOKUP_PORT_NUMBER);

	pkt.data.errormsg.errortype = (errtype);
	pkt.data.errormsg.errornumber = (errcode);

	switch (errtype)
	{
	case XERRT_SERPLEX:
		strcpy(pkt.data.errormsg.msg, XErrorSStrings[errcode]);
		break;
	default:
		break;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	/* addr.sin_addr.s_addr = htonl(phonodep->ipaddress.l); */

	/* Send the error packet to the right remote address, which
	 * may be the firewall address.
	 */
	addr.sin_addr.s_addr = htonl(phonodep->ipaddress.l); 
	addr.sin_port = htons(CONTROL_PORT);
	
	htonPkt(PKT_ERROR, &pkt);

	/* Send this to the phonode as UDP */
	if (sendto(_netfd, (char *)&pkt, sizeof(Pkt), 0, 
			(struct sockaddr *)&addr,
			sizeof(struct sockaddr_in)) < 0)
	{
		NETERROR(MAGE, ("%s for 0x%x", fn,
			ntohl(addr.sin_addr.s_addr)));
	}
	else
	{
		NETDEBUG(MAGE, NETLOG_DEBUG4,
			("%s Error Packet Sent to 0x%x\n",
				fn, ntohl(addr.sin_addr.s_addr)));
	}
	return(1);
}

int
LsAgeInit()
{
    struct  itimerval cachetmr;
	struct sockaddr_in myaddr;
	int i = 1, retval;

	/* Create Socket */
	_netfd = socket (AF_INET, SOCK_DGRAM, 0 );
	setsockopt(_netfd, SOL_SOCKET, SO_REUSEADDR,  
				(void *)&i, sizeof(i));

	if (_netfd < 0)
	{
		perror ("Unable to create socket ");
		return -1;
	}

	bzero ((char *)&myaddr, sizeof(myaddr));  /*  Zeroes the struct */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (ALOID_AGING_PORT_NUMBER);
	myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

	/* Bind */
	retval = bind (_netfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	if ( retval < 0 )
	{
		perror ("Unable to bind socket");
		return -1;
	}

	/* Add cachetimer for Cache Poll*/
	memset(&cachetmr, 0, sizeof(struct itimerval));
	cachetmr.it_interval.tv_sec = CACHE_AGE_TIME;
	
	ThreadLaunch(LsAgeCache, NULL, 1);
	return(0);
}

