#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif

#include "log.h"
#include "alerror.h"
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
/* Server prototypes */

#include "db.h"
#include "profile.h"
#include "mem.h"
#include "entry.h"
#include "lsprocess.h"
#include "phone.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "xmltags.h"
#include "timer.h"
#include "connapi.h"
#include "sconfig.h"

#include "gis.h"
#include "uh323.h"
#include "lrq.h"
#include "resolvers.h"
#include "gw.h"
#include "nxosd.h"
#include <malloc.h>

#include "ls.h"
#include "dbs.h"

extern int debug;

#if 0
/* addr must be in host order */
int
TcpSockAddressMatch(int sockfd, unsigned long addr, struct sockaddr_in *newaddr)
{
	char fn[] = "DoFirewallCheck():";

	struct sockaddr_in peer;
	int addrlen = sizeof(peer);

	if (getpeername(sockfd, (struct sockaddr *)&peer, &addrlen) < 0)
	{
		NETERROR(MSEL,
			("%s Couldn't get remote address!\n", fn));	
		return -1;
	}
	
	if (newaddr)
	{
		memcpy(newaddr, &peer, sizeof(struct sockaddr_in));
	}

	if (ntohl(peer.sin_addr.s_addr) != addr)
	{
		return 0;
	}

	return 1;
}

int
ProcessRegister (int sockfd, 
		Pkt * data_pkt,  
		Pkt * reply_pkt, /* We probably wont use it... */
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
				      */
		int (*writecb)())
{
	char fn[] = "ProcessRegister():";
	InfoEntry  * entry;
	struct sockaddr_in new;

	/* Add the entry */
	if (ProcessIedgeRegistration (data_pkt) < 0)
	{
	    	NETDEBUG(MREGISTER, NETLOG_DEBUG1,
				 ("%s Failed to add Entry reason (%d)\n", fn,
			(data_pkt->data.reginfo.reason)));

	    	data_pkt->type = PKT_REGISTER_FAILURE;
			htonPkt(data_pkt->type, data_pkt);

			writecb(sockfd, opaque, opaquelen, data_pkt, sizeof(Pkt),
					PKT_REGISTER_FAILURE);
	}
	else
	{
	    	NETDEBUG(MREGISTER, NETLOG_DEBUG1,
				 ("%s Successful in adding Entry\n", fn));

	    	/* Successful addition */
	    	data_pkt->type = PKT_REGISTER_SUCCESS;

			/* Check if the iedge came through a firewall */
			if (!TcpSockAddressMatch(sockfd, 
				data_pkt->data.reginfo.node.ipaddress.l, &new))
			{
				DEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s Registration address does not match with src\n",
					fn));

				/* Change address */
				data_pkt->data.reginfo.node.ipaddress.l = 
					ntohl(new.sin_addr.s_addr);

				data_pkt->data.reginfo.reason = nextoneFirewallMismatch;
			}

			htonPkt(data_pkt->type, data_pkt);

	    	writecb(sockfd, opaque, opaquelen, data_pkt, sizeof(Pkt), 
			PKT_REGISTER_SUCCESS);

	}

    NETDEBUG(MREGISTER, NETLOG_DEBUG1,
		 ("%s Response sent\n", fn));

	return 0;
}
#endif

int
ResolvePhone(
	ResolveHandle *rhandle
)
{
	char fn[] = "ResolvePhone():";
	int rc;

	rc = ResolvePhoneLocally(rhandle, 0);

	rhandle->result = rc;

	return rc;
}

#if 0
/* We support various kinds of find's. ipaddress finds,
 * phone finds, vpn phone finds. Note that there are no
 * registration id finds.
 */
int
ProcessFindPhone (
	ClientHandle *chandle,
	Pkt * data_pkt, 
	Pkt * reply_pkt, 
	void *opaque, int opaquelen, /* Arguments to be passed
								  * to the write call back.
								  */	
	int (*writecb)())
{
	char fn[] = "ProcessFindPhone():";
	int sockfd = CHUCCHandle(chandle)->fd;
	CacheTableInfo cacheInfoEntry, *cacheInfo, 
		 srcCacheInfoEntry, *srcCacheInfo, tempCacheInfo;
	char *rPhone, guessPhone[PHONE_NUM_LEN] = { 0 };
	InfoEntry *entry = 0x0, fentry, aentry;
	int result;
	int checkZone = 0, checkVpnGroup = 0;
	int fPolicy = 1, aPolicy = 1;
	PhoNode *phonodep, *fphonodep, 
		 *rphonodep, *rfphonodep, *raphonodep;
	ResolveHandle *rhandle = NULL;
	int rc, retval = 1;
	VpnEntry *vpnEntry;

	cacheInfo = &cacheInfoEntry;
	srcCacheInfo = &srcCacheInfoEntry;

	phonodep  = &data_pkt->data.find.node;
	fphonodep = &data_pkt->data.find.fnode;

	rphonodep  = &reply_pkt->data.find.node;
	rfphonodep = &reply_pkt->data.find.fnode;
	raphonodep = &reply_pkt->data.find.anode;

	/* Initialize the raphonode to be rfphonode */
	memcpy(raphonodep, rfphonodep, sizeof(PhoNode));

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		  ("%s Processing starts Source:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, phonodep);
	
	NETDEBUG(MFIND, NETLOG_DEBUG4,
		  ("%s Processing starts Dest:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, fphonodep);
	
	/* Get the originator's entry, straight from database. It may
	* be faster to look him up from the database (not true when there
	* are lesser netoids
	*/
	if (!BIT_TEST(phonodep->sflags, ISSET_REGID) ||
		!BIT_TEST(phonodep->sflags, ISSET_UPORT))
	{
		 return -1;
	}

	if (BIT_TEST(fphonodep->sflags, ISSET_VPNPHONE))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Vpn Phone found in packet\n", fn));
		strncpy(fphonodep->phone, fphonodep->vpnPhone, PHONE_NUM_LEN);
		BIT_SET(fphonodep->sflags, ISSET_PHONE);
		BIT_RESET(fphonodep->sflags, ISSET_VPNPHONE);
	}

	if (BIT_TEST(fphonodep->sflags, ISSET_PHONE))
	{
		 NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Phone found in packet\n", fn));
	}
	
	/* Assign source - h323id is not available */
	if (FillSourceCacheForCallerId(phonodep, "", "", "", "", srcCacheInfo) < 0)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s: Source cache not found\n", fn));
		reply_pkt->type = PKT_NOTFOUND;
		goto _reply;
	}

	/* Initialize the rhandle */
	rhandle 		= GisAllocRHandle();
	rhandle->phonodep 	= phonodep;
	rhandle->rfphonodep 	= rfphonodep;
	rhandle->chandle 	= chandle;

	memset(&rhandle->sVpn, 0, sizeof(VpnEntry));
	memset(rhandle->sZone, 0, ZONE_LEN);

	/* Set policy */
	rhandle->checkZone = 1;
	rhandle->checkVpnGroup = 1;

	/* Setup source information in the rhandle */
	entry = &srcCacheInfo->data;
	memcpy(&rhandle->scacheInfoEntry, srcCacheInfo, sizeof(CacheTableInfo));
	FindIedgeVpn(entry->vpnName, &rhandle->sVpn);
	strncpy(rhandle->sZone, entry->zone, ZONE_LEN);
	rhandle->sZone[ZONE_LEN-1] = '\0';

	rhandle->primary = 0;
	rhandle->resolveRemote = (routecall)?0:1;
	ResolvePhone(rhandle);

	switch (rhandle->result)
	{
	case CACHE_NOTFOUND:
	  	reply_pkt->type = PKT_NOTFOUND;
		goto _reply;
	  	break;
	case CACHE_FOUND:
	  	reply_pkt->type = PKT_FOUND;
	  	break;
	case CACHE_INPROG:
	  	retval = 0;
	  	return retval;
	  	break;
	default:
	  	break;
	}

	if (!routecall)
	{
		/* We must resolve the Secondary Phone */
		rhandle->primary = 1;
		rhandle->resolveRemote = 0;
		rhandle->rfphonodep 	= raphonodep;
		ResolvePhone(rhandle);
	}
	else
	{
		/* We need not resolve Secondary */
		memset(raphonodep, 0, sizeof(PhoNode));
	}

	/* First find the endpoint or the gateway it must use.
	 * Based on the endpoint's protocol (SIP/H.323) and
	 * the setting of the ucc protocol, we will return
	 * a result to the caller
	 */
	/* Basically AND our uccproto settings with found entry */
    if (routecall)
    {
		rfphonodep->ipaddress.l = ntohl(iServerIP);
		BIT_SET(rfphonodep->sflags, ISSET_IPADDRESS);
		rfphonodep->clientState |= CL_ACTIVE;

		/* If endpoint supports both protocols...
		 * or none of them...
		 */
		if (((BIT_TEST(rfphonodep->cap, CAP_IGATEWAY)||
			 BIT_TEST(rfphonodep->cap, CAP_H323)) &&
			BIT_TEST(rfphonodep->cap, CAP_SIP)) ||
			(!BIT_TEST(rfphonodep->cap, CAP_IGATEWAY) &&
			!BIT_TEST(rfphonodep->cap, CAP_H323) &&
			!BIT_TEST(rfphonodep->cap, CAP_SIP)) )
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Both/None of H.323/SIP on this endpt\n", fn));
			BIT_RESET(rfphonodep->cap, CAP_IGATEWAY);
			BIT_RESET(rfphonodep->cap, CAP_H323);
			BIT_RESET(rfphonodep->cap, CAP_SIP);
		}
		else
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Routing configured protocol on this endpt\n", fn));

			goto _reply;
		}

		if (uccProto == CPROTO_SIP)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s UCC Protocol set to SIP\n", fn));
           	BIT_SET(rfphonodep->cap, CAP_SIP);
		}
		else
		{
			/* default case ... */
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s UCC Protocol set to H.323\n", fn));
           	BIT_SET(rfphonodep->cap, CAP_IGATEWAY);
           	BIT_SET(rfphonodep->cap, CAP_H323);
		}

        goto _reply;
    }

 _reply:
	
	/* Handle reply */
	htonPkt(reply_pkt->type, reply_pkt);

	PktSend (sockfd, reply_pkt);

	/* Free the remote handle here */
	GisFreeRHandle(rhandle);

	return retval; 
}

int
ProcessRedirect (
		int sockfd, 
		Pkt * data_pkt, Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					     * to the write call back.
					     */
		int (*writecb)())
{
	char fn[] = "ProcessRedirect();";
	CacheTableInfo *cacheInfo = 0;
	InfoEntry * entry, tmpInfo;
	char *phone = 0;
	int type = data_pkt->type;
	int error = -1, redop;
	unsigned char tags[TAGH_LEN] = { 0 };
	PhoNode *phonodep;


	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s Processing starts Source:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, 
		&data_pkt->data.redirect.onode);
	
	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s Processing starts Redirect Dest:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, 
		&data_pkt->data.redirect.nnode);
	
	phonodep = &data_pkt->data.redirect.onode;

	/* Lock the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, phonodep);

	if (!cacheInfo )
	{
	    /* Redirect from an unregistered client */
	    goto _return;
	}

	entry = &cacheInfo->data;

	if (!(entry->stateFlags & CL_ACTIVE))
	{
	    /* Redirect from an unregistered client */
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Unregistered Client, dropping request\n", fn));
	    goto _return;
	}
	else if (entry->stateFlags & CL_FORWARDSTATIC)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Client has static forwarding, dropping request\n", fn));
	    goto _return;
	}
	else
	{
		redop = data_pkt->data.redirect.valid;

		error = 0;
		if (redop == REDIRECT_ON)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s red op is REDIRECT_ON:\n", fn));

			/* Make an entry in the state flags */
			if (type == PKT_REDIRECT)
			{
				entry->stateFlags |= CL_FORWARD;

				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("Enabling Fwding proto %d req type(lus=%d,vpns=%d)\n",
				data_pkt->data.redirect.protocol, isLusReq, isVpnsReq));

				/* At this point, we must completely
				* wipe out any old configuration
				*/
				BIT_RESET(entry->nsflags, ISSET_PHONE);
				BIT_RESET(entry->nsflags, ISSET_VPNPHONE);

				BITA_SET(tags, TAG_FWDINFO);

				/* Save redirect information */
				if (BIT_TEST(data_pkt->data.redirect.nnode.sflags, 
						ISSET_PHONE))
				{
					strcpy (entry->nphone, 
						data_pkt->data.redirect.nnode.phone);
					BIT_SET(entry->nsflags, ISSET_PHONE);
					NETDEBUG(MREGISTER, NETLOG_DEBUG4,
						(" New phone: %s\n", entry->nphone));
				}

				/* Save redirect information */
				if (BIT_TEST(data_pkt->data.redirect.nnode.sflags, 
						ISSET_VPNPHONE))
				{
					strcpy (entry->nphone, 
						data_pkt->data.redirect.nnode.vpnPhone);
					BIT_SET(entry->nsflags, ISSET_PHONE);
					NETDEBUG(MREGISTER, NETLOG_DEBUG4,
						(" New phone: %s\n", entry->nphone));

					strcpy (entry->nvpnPhone, 
						data_pkt->data.redirect.nnode.vpnPhone);

					entry->nvpnExtLen = 
						data_pkt->data.redirect.nnode.vpnExtLen;

					BIT_SET(entry->nsflags, ISSET_VPNPHONE);
					NETDEBUG(MREGISTER, NETLOG_DEBUG4,
						(" New vpn phone: %s\n", entry->nvpnPhone));
				}

				entry->protocol = 
					data_pkt->data.redirect.protocol;
			}
			else if (type == PKT_PROXY)
			{
				if (entry->ispcorgw == 1)
				{
					NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s Client is a pc, dropping\n", fn));
					error = 1;
					goto _return;
				}

				entry->stateFlags |= CL_PROXY;
				NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("Enabling Proxy\n"));
				BITA_SET(tags, TAG_PROXYS);
				BITA_SET(tags, TAG_PROXYC);
			}
		}
		else if (redop == REDIRECT_OFF)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s red op is REDIRECT_OFF:\n", fn));

			if (type == PKT_REDIRECT)
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4,
					("%s red op is REDIRECT_OFF:\n", fn));

				memset (entry->nphone, 0, PHONE_NUM_LEN);
				memset (entry->nvpnPhone, 0, PHONE_NUM_LEN);

				/* Remove forwarded state */
				BIT_RESET(entry->nsflags, ISSET_PHONE);
				BIT_RESET(entry->nsflags, ISSET_VPNPHONE);
				entry->stateFlags &= ~CL_FORWARD;
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("Disabling Fwding\n"));

				BITA_SET(tags, TAG_FWDINFO);
			}
			else if (type == PKT_PROXY)
			{
				if (entry->ispcorgw == 1)
				{
					NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s Client is a pc, dropping\n", fn));
					error = 1;
					goto _return;
				}

#if 0
				memset (entry->xphone, 0, PHONE_NUM_LEN);
				memset (entry->xvpnPhone, 0, PHONE_NUM_LEN);
#endif

				/* Remove proxy state */
				entry->stateFlags &= ~(CL_PROXY|CL_PROXYING);
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("Disabling Proxy\n"));

				/* Here we must delete any cache entry which
				* may be proxying... 
				*/
				HandleProxyDisable(cacheInfo);

				BITA_SET(tags, TAG_PROXYS);
				BITA_SET(tags, TAG_PROXYC);
			}
		}
		else if (redop == REDIRECT_GET)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s red op is REDIRECT_GET:\n", fn));

			memset(&data_pkt->data.redirect.nnode, 0,
				sizeof(PhoNode));

			if (!(entry->stateFlags & CL_FORWARD))
			{
				/* For now we are sending fwding info
				* only. Later on, we can send the proxy
				* client information also.
				*/
				data_pkt->data.redirect.protocol = 
					(NEXTONE_REDIRECT_OFF);
				error = 1;
				goto _return;
			}

			/* Client is asking us for what state we have */	
			if (BIT_TEST(entry->nsflags, ISSET_PHONE))
			{
				strcpy(data_pkt->data.redirect.nnode.phone,
					entry->nphone);
				BIT_SET(data_pkt->data.redirect.nnode.sflags, 
						ISSET_PHONE);
			}

			if (BIT_TEST(entry->nsflags, ISSET_VPNPHONE))
			{
				strcpy(data_pkt->data.redirect.nnode.vpnPhone,
					entry->nvpnPhone);

				data_pkt->data.redirect.nnode.vpnExtLen = 
					entry->nvpnExtLen;

				BIT_SET(data_pkt->data.redirect.nnode.sflags, 
						ISSET_VPNPHONE);
			}

			data_pkt->data.redirect.protocol = 
				entry->protocol;
		}
	}
	
	cacheInfo->data.rTime = (time(0));
	/* Update the refresh time */
	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));
_return:

	CacheReleaseLocks(regCache);

	/* Update the database */
	if (error == 0)
	{
		//UpdateNetoidDatabase(&tmpInfo);
		DbScheduleIedgeUpdate(&tmpInfo);
	}

	if (error == 0)
	{
		/* Reflect the same packet back to the sender */
		ntohPkt(data_pkt->type, data_pkt);
		PktSend (sockfd, data_pkt);
	}
	else
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s error\n", fn));

		/* Send an error back to the sender */
		reply_pkt->type = PKT_ERROR;
		reply_pkt->totalLen = sizeof(Pkt);
		reply_pkt->data.errormsg.errornumber = 0;

		ntohPkt(data_pkt->type, data_pkt);
		PktSend (sockfd, reply_pkt);
	}

	return 0;
}


int
ProcessDND (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
					      */
		int (*writecb)())
{
	char fn[] = "ProcessDND():";
	CacheTableInfo *cacheInfo;
	InfoEntry * entry, tmpInfo;
	char *phone = 0;
	unsigned char tags[TAGH_LEN] = { 0 };
	int error = -1;
	PhoNode *phonodep;

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s Processing starts Source:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, 
		&data_pkt->data.dnd.node);
	
	phonodep = &data_pkt->data.dnd.node;

	/* Lock the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, phonodep);

	if (!cacheInfo )
	{
	    	/* Redirect from an unregistered client */
	    	goto _return;
	}

	entry = &cacheInfo->data;

	if (!(entry->stateFlags & CL_ACTIVE))
	{
		goto _return;
	}
	else
	{
		error = 0;

		/* Go into DND state */
		if ( ! data_pkt->data.dnd.enableCall )
		{
			/* Make an entry in the state flags */
			entry->stateFlags |= CL_DND;
			NETDEBUG(MFIND, NETLOG_DEBUG4, ("Enabling DND\n"));
		}
		else
		{
			/* Remove DND state */
			entry->stateFlags &= ~CL_DND;
			NETDEBUG(MFIND, NETLOG_DEBUG4, ("Disabling DND\n"));
		}

		BITA_SET(tags, TAG_DND);
	}

	/* Update the refresh time */
	cacheInfo->data.rTime = (time(0));

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));
	
_return:
	CacheReleaseLocks(regCache);

	if (error == 0)
	{
		/* Update the database */
		//UpdateNetoidDatabase(&tmpInfo);
		DbScheduleIedgeUpdate(&tmpInfo);
	}

	return 0;
}

int
ProcessNexTime (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
					      */
		int (*writecb)())		
{
	if (time((time_t *)&reply_pkt->data.nextime.receiveTimestamp[0]) ==
		((time_t)-1))
	{
		NETERROR(MFIND, ("time system call returned error\n"));
	}

	reply_pkt->data.nextime.li = NEXTIME_ALARM;
	reply_pkt->data.nextime.vn = NEXTIME_VERSION;
	reply_pkt->data.nextime.mode = NEXTIME_SERVER;
	reply_pkt->data.nextime.stratum = NEXTIME_UTCLOCAL;
	
	memset(&reply_pkt->data.nextime.receiveTimestamp[1], 0, sizeof(unsigned int));
	memset(&reply_pkt->data.nextime.transmitTimestamp[1], 0, sizeof(unsigned int));
	time((time_t *)&reply_pkt->data.nextime.transmitTimestamp[0]);

	htonPkt(reply_pkt->type, reply_pkt);

	PktSend (sockfd, reply_pkt);

	return 0;
}

int
ProcessProfile (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
					      */
		int (*writecb)())		
{
     	struct in_addr in;
        char buf[INET_ADDRSTRLEN];
     	int h, m, secs, dur;

     	in.s_addr = htonl(data_pkt->credo.ipaddress.l);
     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR START %s (%s.%d) ******************\n", 
		BIT_TEST(data_pkt->data.profile.flags, CDR_CALLSETUP) ?
		"Call Setup": "Call Termination",
		inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN), (data_pkt->credo.uport)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		("Local (%s)\n", 
		BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Registration ID %s\n", data_pkt->data.profile.local.regid));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Port %d\n", (data_pkt->data.profile.local.uport)));
     	NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Phone %s\n", data_pkt->data.profile.local.phone));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", data_pkt->data.profile.local.vpnPhone));


     	NETCDR(MCDR, NETLOG_DEBUG1,("Remote (%s)\n",
		!BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR) ? 
		"Call Origin": "Call Destination"));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Registration ID %s\n", data_pkt->data.profile.remote.regid));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Port %d\n", (data_pkt->data.profile.remote.uport)));
     	in.s_addr = htonl(data_pkt->data.profile.remote.ipaddress.l);
     	NETCDR(MCDR, NETLOG_DEBUG1,(" o Ip %s\n", inet_ntop( AF_INET, &in, buf, INET_ADDRSTRLEN)));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o Phone %s\n", data_pkt->data.profile.remote.phone));
     	NETCDR(MCDR, NETLOG_DEBUG1,
		(" o VpnPhone %s\n", data_pkt->data.profile.remote.vpnPhone));

     	if (!BIT_TEST(data_pkt->data.profile.flags, CDR_CALLSETUP))
	{
     		NETCDR(MCDR, NETLOG_DEBUG1,("Duration\n"));
     		dur = (data_pkt->data.profile.ftime) - 
			(data_pkt->data.profile.ctime);
     		h = dur/3600; m = dur/60 - h*60; secs = dur - h*3600 - m*60;

     		NETCDR(MCDR, NETLOG_DEBUG1,
			(" o %d Hrs, %d Mts and %d Secs\n", h, m, secs));
	}

     	NETCDR(MCDR, NETLOG_DEBUG1,
		("\n***************** CDR END ******************\n"));

		/* We need termination cdr's from both ends */
		if (!BIT_TEST(data_pkt->data.profile.flags, CDR_CALLSETUP)
#ifdef _filter_cdrs_
			&& BIT_TEST(data_pkt->data.profile.flags, CDR_ORIGINATOR)
#endif
			)
		{
			CdrLogMindCti(data_pkt);
		}

     return 0;
}

/* Proxy server has disallowed proxy forwarding from now on */
/* Locks on cacheInfo's head are handled by the caller */
int
HandleProxyDisable(CacheTableInfo *cacheInfo)
{
	CacheTableInfo *peer = NULL;

#if 0
	/* Before we disable proxy, we must unregister the
	* guy who may be proxying for us.
	*/
	if (cacheInfo->data.stateFlags & CL_PROXYING)
	{
		if ( !peer && strlen(cacheInfo->data.xvpnPhone))
		{
			peer = (CacheTableInfo *) CacheGet(vpnPhoneCache, 
					cacheInfo->data.xvpnPhone);
		}

		if ( !peer && strlen(cacheInfo->data.phone))
		{
			peer = (CacheTableInfo *) CacheGet(phoneCache, 
					cacheInfo->data.xphone);
		}

		if (peer)
		{
			peer->data.stateFlags &= ~CL_ACTIVE;
		}

	}

	cacheInfo->data.stateFlags &= ~(CL_PROXY|CL_PROXYING);

	/* Set the peer proxy information to 0 */
	memset(cacheInfo->data.xphone, 0, PHONE_NUM_LEN);
	memset(cacheInfo->data.xvpnPhone, 0, VPN_LEN);
#endif
	
	return 1;
}

int
matchActive(CacheTableInfo *info, void *opaque)
{
	if (info->data.stateFlags & CL_ACTIVE)
	{
		return 0;
	}
	return 1;
}

extern CacheTableInfo *
ConstructProxyInfo(InfoEntry *entry, PhoNode *phonodep);

int
ProcessUnregister (int sockfd, Pkt * data_pkt,  Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
					      */
		int (*writecb)())
{
	char fn[] = "ProcessUnregister():";
	NetoidInfoEntry *netInfo = 0, tmpInfo;
	CacheTableInfo *cacheInfo = 0, *masterCacheInfo = 0;
	int existsInCache = 0, isproxyc = 0;
	char phone[PHONE_NUM_LEN];
	char *sphone = 0, *trashphone = &phone[0];
	unsigned short rflags = 0, xflags = 0, pflags = 0;
	int tmpreason;
	unsigned char tags[TAGH_LEN] = { 0 };
	PhoNode *phonodep;
	int error = 0;

	phonodep = &data_pkt->data.reginfo.node;

	NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Processing starts\n", fn));
	DEBUG_PrintPhoNode(MREGISTER, NETLOG_DEBUG4, phonodep);
		
	/* We may have to unregister all the entries for this
	* registration id and port
	*/

	/* First obtain entry from database and cache */

	if (!BIT_TEST(phonodep->sflags, ISSET_REGID) ||
		!BIT_TEST(phonodep->sflags, ISSET_UPORT))
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("Registration ID missing... \n"));
		 return -1;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, phonodep);

	/* If still, we havent found any cache entry, we must do the first
      	* registration id search here itself
      	*/
	if (cacheInfo == 0)
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Entry for %s %d does not exist\n", 
					fn, phonodep->regid, phonodep->uport));
		 error = 1;
		 goto _return;
	}

	netInfo = &cacheInfo->data;
	memset(&tags[0], 0, TAGH_LEN);

	DiffNetoid(netInfo, phonodep, &rflags, &xflags);

	if ((netInfo->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)	
	{
		 netInfo->stateFlags &= ~CL_PROXYING;

		 isproxyc = 1;
		 BITA_SET(tags, TAG_PROXYC);
	}

	if (isproxyc == 1)
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Proxy Unregistration Request\n", fn));

		masterCacheInfo = ConstructProxyInfo( 	&cacheInfo->data,
												phonodep);
	}

	if (masterCacheInfo)
	{
		 /* Find the difference between the iedge and its master entry */
		 DiffNetoid(&masterCacheInfo->data, phonodep, &rflags, &pflags);
	}
	else if (isproxyc == 1)
	{
		 /* This means that the registration ID is not in our db */
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Master Entry for %s does not exist in cache\n", 
					fn, phonodep->regid));
		 data_pkt->data.reginfo.reason = nextoneInvalidEndpointID;
		 error = 1;
		 goto _return;
	}

	if (ApplyAdmissionPolicy(phonodep, netInfo, isproxyc, xflags, 
							 pflags, &tmpreason) < 0)
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("Entry does not match database information\n"));
		 error = 1;
		 goto _return;
	}

	DEBUG_PrintInfoEntry(MREGISTER, NETLOG_DEBUG4, netInfo);
		
	/* Get rid of this ip address in our cache */
	DeleteIedgeIpAddr(ipCache, netInfo);

	netInfo->stateFlags &= ~CL_ACTIVE;
	BITA_SET(tags, TAG_ACTIVE);

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));

	/* Do the proxy deregistration after releasing the previous lock
	* otherwise, there are chances of a deadlock
	*/

	/* Well, we can now delete this one from the cache */
	if (isproxyc)
	{
		 /* Find the master entry and mark it as absent */
		 UnregisterProxy(netInfo);
	}

 _return:
	CacheReleaseLocks(regCache);

	if (error == 0)
	{
		 UpdateNetoidDatabase(&tmpInfo);
	}

	return 0;
}

int
ProcessUnregisterIedge (
	NetoidInfoEntry *netInfoIn
)
{
	char fn[] = "ProcessUnregisterIedge():";
	NetoidInfoEntry *netInfo, tmpInfo;
	CacheTableInfo *cacheInfo = 0, *masterCacheInfo = 0;
	int existsInCache = 0, isproxyc = 0;
	char phone[PHONE_NUM_LEN];
	char *sphone = 0, *trashphone = &phone[0];
	unsigned short rflags = 0, xflags = 0, pflags = 0;
	int tmpreason;
	unsigned char tags[TAGH_LEN] = { 0 };
	PhoNode *phonodep, phonode;
	int error = -1;

	phonodep = &phonode;

	NETDEBUG(MREGISTER, NETLOG_DEBUG4, ("%s Processing starts\n", fn));
		
	/* We may have to unregister all the entries for this
	* registration id and port
	*/

	/* First obtain entry from database and cache */

	if (!BIT_TEST(netInfoIn->sflags, ISSET_REGID) ||
		!BIT_TEST(netInfoIn->sflags, ISSET_UPORT))
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("Registration ID missing... \n"));
		 return -1;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, netInfoIn);

	/* If still, we havent found any cache entry, we must do the first
      	* registration id search here itself
      	*/
	if (cacheInfo == 0)
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Entry for %s %d does not exist\n", 
					fn, netInfoIn->regid, netInfoIn->uport));
		 error = 1;
		 goto _return;
	}

	netInfo = &cacheInfo->data;
	memset(&tags[0], 0, TAGH_LEN);

	InitPhonodeFromInfoEntry(netInfo, phonodep);

	DiffNetoid(netInfo, phonodep, &rflags, &xflags);

	if ((netInfo->stateFlags & (CL_PROXY|CL_PROXYING)) == CL_PROXYING)	
	{
		 netInfo->stateFlags &= ~CL_PROXYING;

		 isproxyc = 1;
		 BITA_SET(tags, TAG_PROXYC);
	}

	if (isproxyc == 1)
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Proxy Unregistration Request\n", fn));

		masterCacheInfo = ConstructProxyInfo(	&cacheInfo->data, 
												phonodep );
	}

	if (masterCacheInfo)
	{
		 /* Find the difference between the iedge and its master entry */
		 DiffNetoid(&masterCacheInfo->data, phonodep, &rflags, &pflags);
	}
	else if (isproxyc == 1)
	{
		 /* This means that the registration ID is not in our db */
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Master Entry for %s does not exist in cache\n", 
					fn, phonodep->regid));
		 isproxyc = 0;
	}

	if (ApplyAdmissionPolicy(phonodep, netInfo, isproxyc, xflags, 
							 pflags, &tmpreason) < 0)
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("Entry does not match database information\n"));
	}

	DEBUG_PrintInfoEntry(MREGISTER, NETLOG_DEBUG4, netInfo);
		
	/* Get rid of this ip address in our cache */
	DeleteIedgeIpAddr(ipCache, netInfo);

	netInfo->stateFlags &= ~CL_ACTIVE;
	BITA_SET(tags, TAG_ACTIVE);

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));
	error = 0;

#if 0

	/* Do the proxy deregistration after releasing the previous lock
	* otherwise, there are chances of a deadlock
	*/

	/* Well, we can now delete this one from the cache */
	if (isproxyc)
	{
		 /* Find the master entry and mark it as absent */
		 UnregisterProxy(netInfo);
	}
#endif

 _return:
	CacheReleaseLocks(regCache);

	if (error == 0)
	{
		 //UpdateNetoidDatabase(&tmpInfo);
		 DbScheduleIedgeUpdate(&tmpInfo);
	}

	return 0;
}

/*
 * Query Clients
 *  Sends back a variable length packet.
 */
int
ProcessServerMsg (int sockfd, Pkt * data_pkt,  Pkt * reply_pkt,
		void *opaque, int opaquelen, /* Arguments to be passed
					      * to the write call back.
					      */
		int (*writecb)())
{
	char fn[] = "ProcessServerMsg():";
	unsigned int type = ntohl(data_pkt->data.serverMsg.subType);

	/* Now we gotta mux on the basis of type */
	switch (type)
	{
	case ServerMsg_eQueryClient:
	  ProcessQueryClient (sockfd, data_pkt, reply_pkt);
	  break;
	case ServerMsg_eQueryVpns:
	  ProcessQueryVpns(sockfd, data_pkt, reply_pkt);
	  break;
	case ServerMsg_eQueryNode:
	  ProcessQueryNode(sockfd, data_pkt, reply_pkt);
	  break;
	case ServerMsg_eQueryCP:
	  ProcessQueryCP(sockfd, data_pkt, reply_pkt);
	  break;
	case ServerMsg_eQueryCR:
	  ProcessQueryCR(sockfd, data_pkt, reply_pkt);
	  break;
	case ServerMsg_eRegister:
	  ProcessSrvrRegister(sockfd, data_pkt, reply_pkt);	
	  break;
	default:
      NETERROR(MFIND,
	           ("%s Undefined Type %d\n", fn, (int) type ));
	  /* Put the error code and send the packet back */
	  data_pkt->data.serverMsg.status = htonl(ServerMsg_eTypeUnsupp);
	  htonPkt(data_pkt->type, data_pkt);
	  PktSend (sockfd, data_pkt);
	}
	return 0;
}

int
ProcessQueryClient(int sockfd, Pkt * data_pkt,  Pkt * not_needed)
{
    char fn[] = "ProcessQueryClient():";
    Pkt *reply_pkt;
    unsigned long 
	len = sizeof(Pkt), 
		max = QUERY_CLIENT_MAX*QUERY_CLIENT_SIZE + sizeof(Pkt);
    long retPkt[(QUERY_CLIENT_MAX*QUERY_CLIENT_SIZE + sizeof(Pkt))/sizeof(long)+1];
    QueryClientIndex *queryClientIndex;
    QueryClientData *queryClientData;
    NetoidInfoEntry *netInfo;
	ClientAttribs *clAttribs;
	NetoidSNKey *key;
	int keylen = sizeof(NetoidSNKey);
	int no = 0, all = 0, duplicates = 0;
    CacheTableInfo *info, cacheInfoEntry;
    int i, present = 0;
    int nusers = 0, npkts = 0; /* debug stuff */
	char vpnName[VPNS_ATTR_LEN] = { 0 }, vpnPhone[PHONE_NUM_LEN];
    unsigned short vpnPresent;
	unsigned long xLen = 0, vpnIdLen;
	PhoNode *phonodep = &data_pkt->credo;
	VpnEntry vpn = { 0 };

    /* First search the entry and find its VPN ID. Then lookup the VPN
    * database and find its vpn group
    */
	info = &cacheInfoEntry;
	if (CacheFind(regCache, phonodep,
		info, sizeof(CacheTableInfo)) < 0)
	{
     	reply_pkt->data.serverMsg.status = 
			htonl(ServerMsg_eBadField); /* By default */
		goto _error;
	}

	FindIedgeVpn(&info->data.vpnName, &vpn);

	if (VpnId2Name(data_pkt->data.serverMsg.index.clientIndex.vpnId,
		vpn.vpnGroup, &vpn) < 0)
	{
     	reply_pkt->data.serverMsg.status = 
			htonl(ServerMsg_eBadField); /* By default */
		goto _error;
	}
			
	strncpy(vpnName, vpn.vpnName, VPNS_ATTR_LEN-1);
    vpnPresent = strlen(vpnName);

    memset(retPkt, 0, max);

    /* Setup the return header */
    memcpy(retPkt, (char *)data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *)retPkt;
    reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */
     /* We will set the length in the end */

     queryClientIndex = (QueryClientIndex *)(reply_pkt + 1);

     /* Walk the client list, in the easiest way for now */
     
	info = &cacheInfoEntry;

	for (present = CacheFindFirst(regCache, info, sizeof(CacheTableInfo)); 
		(present > 0);
		present = CacheFindNext(regCache, &info->data, info, sizeof(CacheTableInfo)))
	{	
		if (info->data.uport == 2)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Skipping Entry, Marked FAX\n",
					fn));
			continue;			
		}

		if (!(info->data.stateFlags & CL_ACTIVE) &&
			!(info->data.stateFlags & CL_FORWARD) &&
			!(info->data.stateFlags & CL_PROXY) &&
			!(info->data.stateFlags & CL_STATIC))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Skipping Entry, Marked INACTIVE, neither fwd nor proxy\n",
					fn));
			continue;
		}

		if (vpnPresent)
		{
#if 0
			if (strncmp(info->data.vpnName,
				 vpnName, VPNS_ATTR_LEN) != 0)
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Entry %s does not match the vpnName %s\n", 
				fn, info->data.vpnName, 
				vpnName));

				continue;
			}
#endif

#ifdef _APPLY_ROUTE_TO_PHONES
			if (ApplyRoutePrefix(vpnName, 
				info->data.phone, vpnPhone, &xLen) < 0)
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Cannot Apply route %s\n", 
				fn, vpnName));

				continue;
			}
			
			vpnIdLen = strlen(vpnPhone)-xLen;

			strncpy(queryClientIndex->vpnId,
				vpnPhone, vpnIdLen-1);
			queryClientIndex->vpnId[vpnIdLen-1] = '\0';

			strcpy(queryClientIndex->vpnExt, vpnPhone+vpnIdLen);
#endif
	
			if (strcmp(vpnName, info->data.vpnName) ||
				!BIT_TEST(info->data.sflags, ISSET_VPNPHONE))
			{
				continue;
			}

			strcpy(queryClientIndex->vpnId, vpn.vpnId);
			strcpy(queryClientIndex->vpnExt, info->data.vpnPhone);
#if 0
			vpnPrefixLen = strlen(info->data.vpnPhone)
			     - info->data.vpnExtLen;
			
			if (vpnPresent != vpnPrefixLen)
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4,
					("%s Entry %s does not match the vpnId %s\n", 
				fn, info->data.vpnPhone, 
				data_pkt->data.serverMsg.index.vpnsIndex.vpnId));
				continue;
			}

			/* Check the vpn */
			if (BIT_TEST(info->data.sflags|info->data.dflags, 
					ISSET_VPNPHONE))
			{
				if (strncmp(info->data.vpnPhone,
				 data_pkt->data.serverMsg.index.vpnsIndex.vpnId,
				    vpnPrefixLen) != 0)
				{
					NETDEBUG(MFIND, NETLOG_DEBUG4,
					("%s Entry %s does not match the vpnId %s\n", 
					fn, info->data.vpnPhone, 
					data_pkt->data.serverMsg.index.vpnsIndex.vpnId));

			     		continue;
				}
			}
#endif
		}

		if (BIT_TEST(info->data.sflags | info->data.dflags, 
				ISSET_PHONE))
		{
			strcpy(queryClientIndex->phone,
				info->data.phone);
		}

		if (BIT_TEST(info->data.sflags|info->data.dflags,
				ISSET_IPADDRESS))
		{
		   	queryClientIndex->ipAddress = 
				htonl(info->data.ipaddress.l);
		}

		queryClientIndex->uport = htonl(info->data.uport);

		queryClientData = (QueryClientData *)(queryClientIndex + 1);

		/* Copy the attributes... */

		key = &info->data;	

		if (clAttribs = DbExtractEntry(ATTRIBS_DB_FILE, DB_eAttribs,
							(char *)key, keylen))
		{
			strcpy(queryClientData->clFname, clAttribs->clFname);
			strcpy(queryClientData->clLname, clAttribs->clLname);
			strcpy(queryClientData->clLoc, clAttribs->clLoc);
			strcpy(queryClientData->clCountry, clAttribs->clCountry);
			strcpy(queryClientData->clComments, clAttribs->clComments);
			
			free(clAttribs);
		}
	
		queryClientIndex = (QueryClientIndex *)
					((char *)queryClientIndex + 
						QUERY_CLIENT_SIZE); 
		len += QUERY_CLIENT_SIZE;
		nusers ++;

		/* Right now, we dont send any info in the data
		* section
		*/

		if ((len+QUERY_CLIENT_SIZE) >= max)
		{
			/* Send this packet now. We must set the rest
			 * of the fields properly.
			 */
			reply_pkt->data.serverMsg.status = 
					htonl(ServerMsg_eCont);
			reply_pkt->totalLen = (len);
			reply_pkt->data.serverMsg.blockLen = 
					htonl(QUERY_CLIENT_SIZE);
			npkts ++;
			reply_pkt->type = (PKT_SERVERMSG);
			/* Send .... */

     			htonPkt(reply_pkt->type, reply_pkt);

			if (write (sockfd, retPkt, len) < 0)
			{
				NETERROR(MFIND,
				 ("%s write returned error %d\n",
				 fn, errno));
			}

			len = sizeof(Pkt);
			queryClientIndex = 
			     (QueryClientIndex *)(reply_pkt + 1);
			/* zero out the rest of packet - re-used */
			memset(queryClientIndex, 0, (max-len));
		}

	} 
	
	CacheFreeIterator(regCache);
	
     reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

_error:
	reply_pkt->type = (PKT_SERVERMSG);
     reply_pkt->totalLen = (len);
     reply_pkt->data.serverMsg.blockLen = htonl(QUERY_CLIENT_SIZE);

     npkts++;
     /* Send whatever is remaining */

     htonPkt(reply_pkt->type, retPkt);

     if (write (sockfd, retPkt, len) < 0)
     {
	  	NETERROR(MFIND, 
	     		("%s write returned error %d\n",
	     		fn, errno));
     }

     NETDEBUG(MFIND, NETLOG_DEBUG1, 
		("%s npkts %d nusers %d\n", fn, npkts, nusers));
		
     return 1;
}

#if 0
int
ProcessQueryVpns(int sockfd, Pkt * data_pkt,  Pkt * reply_pkt)
{
     	char fn[] = "ProcessQueryVpns():";
     	unsigned long 
	 	len = sizeof(Pkt), 
	 	max = QUERY_VPNS_MAX*QUERY_VPNS_SIZE + sizeof(Pkt);
     	long retPkt[ (QUERY_VPNS_MAX*QUERY_VPNS_SIZE + sizeof(Pkt))/sizeof(long)+1];
     	QueryVpnsIndex *queryVpnsIndex;
     	QueryVpnsData *queryVpnsData;
     	VpnEntry *vpnEntry, vpn;
     	InfoEntry *netInfo;
	CacheTableInfo cacheInfoEntry, *cacheInfo;	
	CacheVpnEntry *cacheVpnEntry, tmpCacheVpnEntry;
	int present = 0;
	PhoNode *phonodep = &data_pkt->credo;

     	/* First search the entry and find its VPN ID. Then lookup the VPN
      	* database and find its vpn group
      	*/
	cacheInfo = &cacheInfoEntry;
	cacheVpnEntry = &tmpCacheVpnEntry;

    memset(retPkt, 0, max);

    /* Setup the return header */
    memcpy(retPkt, data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *)retPkt;

	if (CacheFind(regCache, phonodep,
		cacheInfo, sizeof(CacheTableInfo)) < 0)
	{
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadField); /* By default */
		goto _error;
	}

	FindIedgeVpn(&cacheInfo->data.vpnName, &vpn);

     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */
     	/* We will set the length in the end */

     	queryVpnsIndex = (QueryVpnsIndex *)(reply_pkt + 1);

	for (present = CacheFindFirst(vpnCache, cacheVpnEntry, sizeof(CacheVpnEntry));
		(present > 0);
		present = CacheFindNext(vpnCache, &cacheVpnEntry->vpnEntry,
				cacheVpnEntry, sizeof(CacheVpnEntry)))
     	{
		vpnEntry = &cacheVpnEntry->vpnEntry;

	 	if (strlen(vpnEntry->vpnId) == 0)
	 	{
			NETERROR(MFIND, ("Found a NULL VpnId in the vpn list\n"));
			continue;
	 	}

	 	if (strcmp(vpnEntry->vpnGroup, vpn.vpnGroup) != 0)
	 	{
	      		NETDEBUG(MFIND, NETLOG_DEBUG4,
		    		("vpn entry %s in grp %s is not in group %s", 
		     		vpnEntry->vpnId, vpnEntry->vpnGroup, vpn.vpnGroup));
	      		continue;
	 	}

	 	NETDEBUG(MFIND, NETLOG_DEBUG4,
			("Retrieved vpn entry %s", vpnEntry->vpnId));

	 	strcpy(queryVpnsIndex->vpnId, vpnEntry->vpnId);

	 	queryVpnsData = (QueryVpnsData *) (queryVpnsIndex + 1);

	 	/* Network ordered */
	 	queryVpnsData->vpnExtLen = htonl(vpnEntry->vpnExtLen);

		/* Put in the name/loc/contact */
		strcpy(queryVpnsData->vpnName, vpnEntry->vpnName);
		strcpy(queryVpnsData->vpnLoc, vpnEntry->vpnLoc);
		strcpy(queryVpnsData->vpnContact, vpnEntry->vpnContact);
	 	queryVpnsIndex = (QueryVpnsIndex *)
			(((char *)queryVpnsIndex) + QUERY_VPNS_SIZE); 
	 	len += QUERY_VPNS_SIZE;

	 	if ((len+QUERY_VPNS_SIZE) >= max)
	 	{
	      		/* Send this packet now. We must set the rest
			* of the fields properly.
			*/
	      		reply_pkt->data.serverMsg.status = htonl(ServerMsg_eCont);

     			reply_pkt->type = (PKT_SERVERMSG);
	      		reply_pkt->totalLen = (len);
	      		reply_pkt->data.serverMsg.blockLen = htonl(QUERY_VPNS_SIZE);
	      
     			htonPkt(reply_pkt->type, reply_pkt);

	      		/* Send .... */
	      		if (write (sockfd, retPkt, len) < 0)
	      		{
		   		log(LOG_DEBUG, 0,
					"%s write returned error %d\n",
					fn, errno);
	      		}
	      
	      		len = sizeof(Pkt);
	      		queryVpnsIndex = 
		   		(QueryVpnsIndex *)(reply_pkt + 1);
	 	}
     }

	CacheFreeIterator(vpnCache);

     reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

_error:
     reply_pkt->data.serverMsg.blockLen = htonl(QUERY_VPNS_SIZE);

     reply_pkt->type = (PKT_SERVERMSG);
     reply_pkt->totalLen = len;

     htonPkt(reply_pkt->type, reply_pkt);

     /* Send whatever is remaining */
     if (write (sockfd, retPkt, len) < 0)
     {
	 log(LOG_DEBUG, 0,
	     "%s write returned error %d \n",
	     fn, errno);
     }
     
     return 1;
}
#endif

/* When an iedge attempts to download vpn's,
 * we will go over his calling plan, and look at the routes
 * in it. Using these routes, we will construct a vpn list,
 * and give it back to the iedge in a vpn-id/extn manner.
 * This enables the iedge to follow old vpn's.
 */
int
ProcessQueryVpns(int sockfd, Pkt * data_pkt,  Pkt * reply_pkt)
{
    	char fn[] = "ProcessQueryVpns():";
    	unsigned long 
		len = sizeof(Pkt), 
		max = QUERY_VPNS_MAX*QUERY_VPNS_SIZE + sizeof(Pkt);
    	long retPkt[ (QUERY_VPNS_MAX*QUERY_VPNS_SIZE + sizeof(Pkt))/sizeof(long)+1];
    	QueryVpnsIndex *queryVpnsIndex;
    	QueryVpnsData *queryVpnsData;
    	VpnEntry *vpnEntry, vpn;
    	InfoEntry *netInfo;
	CacheVpnRouteEntry *cacheRouteEntry = 0, *cacheRouteEntryMatch = 0;
	CacheCPBEntry *cacheCPBEntry = 0;
	VpnRouteKey routeKey = { 0 };
	int reject = 0, vpnPrefix = 0;
	int rlen = 0, extlen = 0;
	char *dest;
	CacheTableInfo cacheInfoEntry, *cacheInfo;	
	CacheVpnEntry *cacheVpnEntry, tmpCacheVpnEntry;
	int present = 0;
	PhoNode *phonodep = &data_pkt->credo;

    	/* First search the entry and find its VPN ID. Then lookup the VPN
    	* database and find its vpn group
    	*/
		cacheInfo = &cacheInfoEntry;
		cacheVpnEntry = &tmpCacheVpnEntry;

    	memset(retPkt, 0, max);

    	/* Setup the return header */
    	memcpy(retPkt, data_pkt, sizeof(Pkt));

    	reply_pkt = (Pkt *)retPkt;

	if (CacheFind(regCache, phonodep,
		cacheInfo, sizeof(CacheTableInfo)) < 0)
	{
     		reply_pkt->data.serverMsg.status = 
				htonl(ServerMsg_eBadField); /* By default */
		goto _error;
	}

    	reply_pkt->data.serverMsg.status = 
		htonl(ServerMsg_eOk); /* By default */

    	/* We will set the length in the end */

    	queryVpnsIndex = (QueryVpnsIndex *)(reply_pkt + 1);

	/* Download routes into the client */
	if (strlen( cacheInfo->data.cpname ))
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		DEBUG(MFIND, NETLOG_DEBUG4,
			("%s endpoint is in calling plan %s, examining routes\n",
				fn, cacheInfo->data.cpname));

		/* Look at the bindings */
		for (cacheCPBEntry = CacheGetFirst(cpbCache);
				cacheCPBEntry;
				cacheCPBEntry = CacheGetNext(cpbCache, 
							&cacheCPBEntry->cpbEntry))	
		{
			if (strcmp(cacheCPBEntry->cpbEntry.cpname, 
					cacheInfo->data.cpname))
			{
				/* We can optimize the search in this condition,
				 * but later.
				 */
				continue;
			}

			/* Found a route, extract it now */
			strncpy(routeKey.crname, 
				cacheCPBEntry->cpbEntry.crname, CALLPLAN_ATTR_LEN);

			cacheRouteEntry = CacheGet(cpCache, &routeKey);

			if (cacheRouteEntry == NULL)
			{
				NETERROR(MFIND, 
					("Found No route corresponding to %s\n",
					routeKey.crname));
				continue;
			}

			DEBUG(MFIND, NETLOG_DEBUG4,
				("\tfound a route %s/%s/%s\n",
				cacheRouteEntry->routeEntry.crname,
				cacheRouteEntry->routeEntry.dest,
				cacheRouteEntry->routeEntry.prefix));

			reject = 0; 
			vpnPrefix = 0;
			rlen = 0; 
			dest = NULL;

	 		rlen = CPAnalyzePattern(cacheRouteEntry->routeEntry.dest, 
									&reject, &vpnPrefix, &dest);

			/* See if this route represents a vpn in the former
			 * sense. It will be considered to be a vpn, if there
			 * is a destination, and a dest len which is greater 
			 * in length than the destination prefix itself.
			 * If this is a reject route, we will download the positive
			 * route to teh endpoint, and let the call be rejected
			 * on the iServer. 
			 * If this is a route with 0 as the destlen, then it means
			 * that no length is enough. In this case we will choose a big
			 * number as the extension length and download it to the endpoint.
			 */

			if ( rlen > 0 )
			{
				/* vpn id = dest, extlen = destlen-rlen */
						
	 			queryVpnsData = (QueryVpnsData *) (queryVpnsIndex + 1);

	 			strcpy(queryVpnsIndex->vpnId, dest);

				extlen = cacheRouteEntry->routeEntry.destlen-rlen;

				if (extlen < 0)
				{
					/* extlen should be made a very huge number */
					extlen = 200;
				}

	 			queryVpnsData->vpnExtLen = htonl(extlen);

	 			NETDEBUG(MFIND, NETLOG_DEBUG4, 
					("Retrieved vpn entry %s/%d", dest, extlen));

				/* Copy the name of the route into the vpn name :( */
				strncpy(queryVpnsData->vpnName, 
					cacheRouteEntry->routeEntry.crname, VPNS_ATTR_LEN-1);

	 			queryVpnsIndex = (QueryVpnsIndex *)
					(((char *)queryVpnsIndex) + QUERY_VPNS_SIZE); 
	 			len += QUERY_VPNS_SIZE;

	 			if ((len+QUERY_VPNS_SIZE) >= max)
	 			{
	      			/* Send this packet now. We must set the rest
					* of the fields properly.
					*/
	      			reply_pkt->data.serverMsg.status = htonl(ServerMsg_eCont);

     				reply_pkt->type = (PKT_SERVERMSG);
	      			reply_pkt->totalLen = (len);
	      			reply_pkt->data.serverMsg.blockLen = htonl(QUERY_VPNS_SIZE);
	      
     				htonPkt(reply_pkt->type, reply_pkt);

	      			/* Send .... */
	      			if (write (sockfd, retPkt, len) < 0)
	      			{
	 					NETERROR(MFIND,
	     					("%s write returned error %d \n",
	     					fn, errno));
	      			}
	      
	      			len = sizeof(Pkt);
	      			queryVpnsIndex = 
		   				(QueryVpnsIndex *)(reply_pkt + 1);
	 			}
			}

		} /* for - loop over all routes in a plan */

		CacheReleaseLocks(cpCache);
		CacheReleaseLocks(cpbCache);
	}

	/* Now go over the iedge vpn group */
	FindIedgeVpn(&cacheInfo->data.vpnName, &vpn);

	for (present = CacheFindFirst(vpnCache, 
				cacheVpnEntry, sizeof(CacheVpnEntry));
		(present > 0);
		present = CacheFindNext(vpnCache, &cacheVpnEntry->vpnEntry,
				cacheVpnEntry, sizeof(CacheVpnEntry)))
     	{
		vpnEntry = &cacheVpnEntry->vpnEntry;

	 	if (strlen(vpnEntry->vpnId) == 0)
	 	{
			NETERROR(MFIND, 
				("Found a NULL VpnId in the vpn list\n"));
			continue;
	 	}

	 	if (strcmp(vpnEntry->vpnGroup, vpn.vpnGroup) != 0)
	 	{
	      		NETDEBUG(MFIND, NETLOG_DEBUG4,
		    		("vpn entry %s in grp %s is not in group %s", 
		     		vpnEntry->vpnId, vpnEntry->vpnGroup, 
				vpn.vpnGroup));
	      		continue;
	 	}

	 	NETDEBUG(MFIND, NETLOG_DEBUG4,
			("Retrieved vpn entry %s", vpnEntry->vpnId));

	 	strcpy(queryVpnsIndex->vpnId, vpnEntry->vpnId);

	 	queryVpnsData = (QueryVpnsData *) (queryVpnsIndex + 1);

	 	/* Network ordered */
	 	queryVpnsData->vpnExtLen = htonl(vpnEntry->vpnExtLen);

		/* Put in the name/loc/contact */
		strcpy(queryVpnsData->vpnName, vpnEntry->vpnName);
		strcpy(queryVpnsData->vpnLoc, vpnEntry->vpnLoc);
		strcpy(queryVpnsData->vpnContact, vpnEntry->vpnContact);
	 	queryVpnsIndex = (QueryVpnsIndex *)
			(((char *)queryVpnsIndex) + QUERY_VPNS_SIZE); 

	 	len += QUERY_VPNS_SIZE;

	 	if ((len+QUERY_VPNS_SIZE) >= max)
	 	{
	      		/* Send this packet now. We must set the rest
			* of the fields properly.
			*/
	      		reply_pkt->data.serverMsg.status = 
				htonl(ServerMsg_eCont);

     			reply_pkt->type = (PKT_SERVERMSG);
	      		reply_pkt->totalLen = (len);
	      		reply_pkt->data.serverMsg.blockLen = htonl(QUERY_VPNS_SIZE);
	      
     			htonPkt(reply_pkt->type, reply_pkt);

	      		/* Send .... */
	      		if (write (sockfd, retPkt, len) < 0)
	      		{
	 				NETERROR(MFIND,
						("%s write returned error %d\n",
						fn, errno));
	      		}
	      
	      		len = sizeof(Pkt);
	      		queryVpnsIndex = 
		   		(QueryVpnsIndex *)(reply_pkt + 1);
	 	}
	}
	CacheFreeIterator(vpnCache);

	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

_error:
	reply_pkt->data.serverMsg.blockLen = htonl(QUERY_VPNS_SIZE);

	reply_pkt->type = (PKT_SERVERMSG);
	reply_pkt->totalLen = len;

	htonPkt(reply_pkt->type, reply_pkt);

	/* Send whatever is remaining */
	if (write (sockfd, retPkt, len) < 0)
	{
	 	NETERROR(MFIND,
	     	("%s write returned error %d \n",
	     	fn, errno));
	}
     
	return 1;
}

int
ProcessQueryNode(int sockfd, Pkt * data_pkt,  Pkt * reply_pkt)
{
	switch(htonl(data_pkt->data.serverMsg.opn))
	{
	case Server_eOpGet:
		ProcessGetQueryNode(sockfd, data_pkt, reply_pkt);
		break;
	case Server_eOpGetNext:
	case Server_eOpGetAll:
		break;
	}

	return 0;
}

int
ProcessGetQueryNode(int sockfd, Pkt * data_pkt,  Pkt * reply_pkt)
{
	Node *node = &data_pkt->data.serverMsg.index.nodeIndex;
     
	switch (htonl(node->type))
	{
	case NODE_PHONODE:
		ProcessGetQueryPhoNode(sockfd, data_pkt, reply_pkt);
		break;
	case NODE_SRVR:
	default:
		break;
	}
	return(0);
}

/* Generalize this: this does only phone lookups right now */
int
ProcessGetQueryPhoNode(int sockfd, Pkt * data_pkt,  Pkt * reply_pkt)
{
	char fn[] = "ProcessGetQueryPhoNode():";
	InfoEntry infoEntry;
	PhoNode *phonode = 
		&data_pkt->data.serverMsg.index.nodeIndex.unode.phonode;
	CacheTableInfo *cacheInfo, tmpCacheInfo;
	InfoEntry	*pentry = 0x0;
	InfoEntry	*entry = 0x0;
	int phoneType;
	char rPhone[PHONE_NUM_LEN];

	ntohPhonode(phonode);

	reply_pkt = data_pkt;
        reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); 

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s Processing starts src:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, &data_pkt->credo);

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s query:\n", fn));

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, phonode);
	
	cacheInfo = &tmpCacheInfo;

	/* fix phonode first */
	if (BIT_TEST(phonode->sflags, ISSET_PHONE))
	{
		/* reset the vpn phone field */
		BIT_RESET(phonode->sflags, ISSET_VPNPHONE);
	}
	else if (BIT_TEST(phonode->sflags, ISSET_VPNPHONE))
	{
		memcpy(phonode->phone, phonode->vpnPhone, PHONE_NUM_LEN);
		BIT_SET(phonode->sflags, ISSET_PHONE);
		BIT_RESET(phonode->sflags, ISSET_VPNPHONE);
	}

	if (BIT_TEST(data_pkt->credo.sflags, ISSET_REGID) &&
		BIT_TEST(data_pkt->credo.sflags, ISSET_UPORT))
	{
		if (CacheFind(regCache, &data_pkt->credo, cacheInfo, 
			sizeof(CacheTableInfo)) < 0)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4, 
				("%s Cannot locate Source Node in cache\n", fn));
			goto _error;
		}

		entry = &cacheInfo->data;

		/* Apply the calling plans for the source */
		if (strlen(entry->cpname))
		{
#if 0
			if (BIT_TEST(phonode->sflags, ISSET_PHONE))
			{
				ApplyNetworkPolicy(entry,
					phonode->phone,
					phonode->phone,
					CRF_CALLORIGIN|CRF_CALLDEST|CRF_POSITIVE|
						CRF_REJECT|CRF_VPNINT|CRF_VPNEXT,
					APPLY_DEST,
					NULL,
					NULL,
				 	NULL);
			}
			else if (BIT_TEST(phonode->sflags, ISSET_VPNPHONE))
			{
				ApplyNetworkPolicy(entry,
					phonode->vpnPhone,
					phonode->phone,
					CRF_CALLORIGIN|CRF_CALLDEST|CRF_POSITIVE|
						CRF_REJECT|CRF_VPNINT|CRF_VPNEXT,
					APPLY_DEST,
					NULL,
					NULL,
				 	NULL);
				BIT_SET(phonode->sflags, ISSET_PHONE);
				BIT_RESET(phonode->sflags, ISSET_VPNPHONE);
			}
#endif
		}

		Phonode2NetInfo(phonode, &infoEntry);
	}
	else
	{
		Phonode2NetInfo(phonode, &infoEntry);
	}

	/* cacheInfo, is now re-used --- !!! */
	if (FindIedge(&infoEntry, cacheInfo, sizeof(CacheTableInfo)) < 0)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s No Entry for node\n", fn));
		goto _error;
	}

	pentry = &cacheInfo->data;
	
	if (pentry)
	{
		/* Set up the node, to be returned */
		memcpy(phonode->regid, pentry->regid, REG_ID_LEN);
		if (BIT_TEST((pentry->sflags|pentry->dflags), 
			      ISSET_PHONE))
		{
		    strcpy(phonode->phone, pentry->phone);
		    BIT_SET(phonode->sflags, ISSET_PHONE); 
		}

		if (BIT_TEST((pentry->sflags|pentry->dflags), 
			      ISSET_IPADDRESS))
		{
		    phonode->ipaddress.l = (pentry->ipaddress.l);
		    BIT_SET(phonode->sflags, ISSET_IPADDRESS);
		}

		if (BIT_TEST((pentry->sflags|pentry->dflags), 
			      ISSET_UPORT))
		{		 
		    phonode->uport = (pentry->uport);
		    BIT_SET(phonode->sflags, ISSET_UPORT);
		}

		if (BIT_TEST((pentry->sflags|pentry->dflags), 
			      ISSET_VPNPHONE))
		{		 
		    strcpy(phonode->vpnPhone, pentry->vpnPhone);
		    phonode->vpnExtLen = (pentry->vpnExtLen);
		    BIT_SET(phonode->sflags, ISSET_VPNPHONE);
		}

		phonode->clientState = (pentry->stateFlags);
		 
		htonPhonode(phonode);
	
		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Found\n", fn));

		DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, 
			&reply_pkt->data.find.fnode);
	}

	htonPkt(reply_pkt->type, reply_pkt);
	PktSend (sockfd, reply_pkt);

	return 0;

_error:
	/* No entry was found in the cache. We will lookup the 
	* database now
	*/
	/* Dont have to do that ... everything is in the cache
	* now
	*/
	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Not Found\n", fn));

       	reply_pkt->data.serverMsg.status = 
		htonl(ServerMsg_eGenError); /* By default */
	htonPhonode(phonode);

	htonPkt(reply_pkt->type, reply_pkt);
	PktSend (sockfd, reply_pkt);

	return -1;
}

int
ProcessQueryCP(int sockfd, Pkt * data_pkt,  Pkt * not_needed)
{
    char fn[] = "ProcessQueryCP():";
    Pkt *reply_pkt;
    unsigned long 
	len = sizeof(Pkt), 
	max = QUERY_CP_MAX*QUERY_CP_SIZE + sizeof(Pkt);
    long retPkt[(QUERY_CP_MAX*QUERY_CP_SIZE + sizeof(Pkt))/sizeof(long)+1];
    QueryCPIndex *queryCPIndex, *inCPIndex;
    QueryCPData *queryCPData;
	NetoidInfoEntry *infoEntry;
    CacheTableInfo *info, cacheInfoEntry = { 0 };
	int browseports = 0, done = 0, nplans = 0, npkts = 0;
	unsigned long uport, reqId, blockLen;
     
    memset(retPkt, 0, max);

    /* Setup the return header */
    memcpy(retPkt, (char *)data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *)retPkt;
    reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */
    /* We will set the length in the end */

    queryCPIndex = (QueryCPIndex *)(reply_pkt + 1);
	inCPIndex = (QueryCPIndex *)(data_pkt + 1);
	reqId = data_pkt->data.serverMsg.reqId;
	blockLen = ntohl(data_pkt->data.serverMsg.blockLen);

	/* For the ports mentioned in the CP message, send back the
	* calling plan the ports are in
	*/
	info = &cacheInfoEntry;

	/* Either there should be an index specified in the
	* header, or we should find something in the body.
	* If that is absent also, we will use the uport
	* specified in the credo field.
	*/
	uport = data_pkt->credo.uport;
	
	if (data_pkt->data.serverMsg.index.cpIndex.uport != -1)
	{
		uport = ntohl(data_pkt->data.serverMsg.index.cpIndex.uport);
	}
	
	if (data_pkt->totalLen >= (sizeof(Pkt)+sizeof(QueryCPIndex)))
	{
		/* There are ports present in the payload */
		browseports = 1;
		uport = ntohl(inCPIndex->uport);
	}

	while (!done)
	{
		/* Set up the cache entry to lookup */	
		memcpy(info->data.regid, &data_pkt->credo.regid, REG_ID_LEN);
		info->data.uport = uport;

		if (CacheFind(regCache, &info->data, info, sizeof(CacheTableInfo)) >= 0)
		{
			/* We must put this entry into the packet */
			queryCPIndex->uport = htonl(uport);

			/* Fill up the data portion */
			queryCPData = (QueryCPData *)(queryCPIndex + 1);	
			memcpy(queryCPData->cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
			
			queryCPIndex = (QueryCPIndex *)
						((char *)queryCPIndex + QUERY_CP_SIZE);
			len += QUERY_CP_SIZE;
			nplans ++;

			/* Right now, we dont send any info in the data
			* section
			*/

			if ((len+QUERY_CP_SIZE) >= max)
			{
				/* Send this packet now. We must set the rest
			 	* of the fields properly.
			 	*/
				reply_pkt->data.serverMsg.status = 
						htonl(ServerMsg_eCont);
				reply_pkt->totalLen = (len);
				reply_pkt->data.serverMsg.blockLen = 
						htonl(QUERY_CP_SIZE);

				reply_pkt->data.serverMsg.reqId = reqId;

				npkts ++;
				reply_pkt->type = (PKT_SERVERMSG);
				/* Send .... */

     				htonPkt(reply_pkt->type, reply_pkt);

				if (write (sockfd, retPkt, len) < 0)
				{
					NETERROR(MFIND,
				 	("%s write returned error %d\n",
				 	fn, errno));
				}

				len = sizeof(Pkt);
				queryCPIndex = 
			     		(QueryCPIndex *)(reply_pkt + 1);
				/* zero out the rest of packet - re-used */
				memset(queryCPIndex, 0, (max-len));
			}
		}
	
		if (browseports)
		{
			/* See if there are any other ports */
			inCPIndex = (QueryCPIndex *)
					((char *)inCPIndex + blockLen);
			if (((char *)inCPIndex + sizeof(QueryCPIndex)) <= 
				((char *)data_pkt + data_pkt->totalLen))
			{
				uport = ntohl(inCPIndex->uport);
			}
			else
			{
				done = 1;
			}
		}
		else
		{
			done = 1;
		}
	}

	reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->totalLen = (len);
    reply_pkt->data.serverMsg.blockLen = htonl(QUERY_CP_SIZE);
	reply_pkt->data.serverMsg.reqId = reqId;

    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

   	npkts++;

    /* Send whatever is remaining */
    htonPkt(reply_pkt->type, retPkt);

    if (write (sockfd, retPkt, len) < 0)
    {
	 	NETERROR(MFIND, 
	     		("%s write returned error %d\n",
	     		fn, errno));
    }

    NETDEBUG(MFIND, NETLOG_DEBUG1, 
		("%s npkts %d nplans %d\n", fn, npkts, nplans));
		
    return 1;
}

int
ProcessQueryCR(int sockfd, Pkt * data_pkt,  Pkt * not_needed)
{
    char fn[] = "ProcessQueryCR():";
    Pkt *reply_pkt;
    unsigned long 
	len = sizeof(Pkt), 
	max = QUERY_CR_MAX*QUERY_CR_SIZE + sizeof(Pkt);
    long retPkt[(QUERY_CR_MAX*QUERY_CR_SIZE + sizeof(Pkt))/sizeof(long)+1];
    QueryCRIndex *queryCRIndex, *inCRIndex;
    QueryCRData *queryCRData;
	CacheVpnRouteEntry cacheRouteEntry = { 0 }, *cacheRoute;
	VpnRouteKey routeKey = { 0 };
	CacheCPBEntry *cacheCPBEntry = 0, tmpCacheCPBEntry = { 0 };
    CacheTableInfo *info, cacheInfoEntry = { 0 };
	int browseplans = 0, done = 0, nplans = 0, nroutes = 0, 
			npkts = 0, tnpkts = 0;
	unsigned long uport, reqId, blockLen;
	char cpname[CALLPLAN_ATTR_LEN] = { 0 };
	int present;
     
    memset(retPkt, 0, max);

    /* Setup the return header */
    memcpy(retPkt, (char *)data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *)retPkt;
    reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */

    /* We will set the length in the end */
    queryCRIndex = (QueryCRIndex *)(reply_pkt + 1);
	inCRIndex = (QueryCRIndex *)(data_pkt + 1);
	reqId = data_pkt->data.serverMsg.reqId;
	blockLen = ntohl(data_pkt->data.serverMsg.blockLen);

	/* For the plans mentioned in the CR message, send back the
	* routes.
	*/
	cacheRoute = &cacheRouteEntry;
	info = &cacheInfoEntry;

	/* Either there should be an index specified in the
	* header, or we should find something in the body.
	* If that is absent also, we will use the call plan
	* of the uport specified in the header field.
	*/
	
	if (strlen(data_pkt->data.serverMsg.index.crIndex.cpname))
	{
		memcpy(cpname, data_pkt->data.serverMsg.index.crIndex.cpname,
			CALLPLAN_ATTR_LEN);
	}
	else if (data_pkt->totalLen >= (sizeof(Pkt)+sizeof(QueryCRIndex)))
	{
		/* There are ports present in the payload */
		browseplans = 1;
		memcpy(cpname, inCRIndex->cpname, CALLPLAN_ATTR_LEN);
	}
	else 
	{
		uport = data_pkt->credo.uport;
		memcpy(info->data.regid, &data_pkt->credo.regid, REG_ID_LEN);
		info->data.uport = uport;

		if (CacheFind(regCache, &info->data, info, sizeof(CacheTableInfo)) >= 0)
		{
			memcpy(cpname, info->data.cpname, CALLPLAN_ATTR_LEN);
		}
	}

#if 0
	if (strlen(cpname) == 0)
	{
		/* We must send back an error */
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadFormat); 
		goto _send_pkt;
	}
#endif

	while (!done)
	{
		/* Browse the routes in the route cache,
		* We may have to browse our route cache multiple times,
		* but we will optimize it a little by stopping when 
		* the cpname changes in the cache.
		*/ 
		memcpy(data_pkt->data.serverMsg.index.crIndex.cpname, cpname,
			CALLPLAN_ATTR_LEN);

		nplans ++;

		cacheCPBEntry = &tmpCacheCPBEntry;

		for (present = CacheFindFirst(cpbCache, cacheCPBEntry, 
										sizeof(CacheCPBEntry));
			(present > 0);
			present = CacheFindNext(cpbCache, &cacheCPBEntry->cpbEntry, 
					cacheCPBEntry, sizeof(CacheCPBEntry)) )
		{
			if (strcmp(cacheCPBEntry->cpbEntry.cpname, cpname))
			{
				continue;
			}

			strncpy(routeKey.crname, cacheCPBEntry->cpbEntry.crname, 
				CALLPLAN_ATTR_LEN);

			if (CacheFind(cpCache, &routeKey, 
					cacheRoute, sizeof(CacheVpnRouteEntry)) < 0)
			{
				continue;
			}

			/* This route is part of the calling plan */
			memcpy(queryCRIndex->cpname, cpname, CALLPLAN_ATTR_LEN);
		
			/* Fill up the data portion */
			queryCRData = (QueryCRData *)(queryCRIndex + 1);	
			memcpy(queryCRData->dest, cacheRoute->routeEntry.dest, PHONE_NUM_LEN);
			memcpy(queryCRData->prefix, cacheRoute->routeEntry.prefix, PHONE_NUM_LEN);	

			queryCRIndex = (QueryCRIndex *)
						((char *)queryCRIndex + QUERY_CR_SIZE);
			len += QUERY_CR_SIZE;
			nroutes ++;

			/* Right now, we dont send any info in the data
			* section
			*/

			if ((len+QUERY_CR_SIZE) >= max)
			{
				/* Send this packet now. We must set the rest
			 	* of the fields properly.
			 	*/
				reply_pkt->data.serverMsg.status = 
						htonl(ServerMsg_eCont);
				reply_pkt->totalLen = (len);
				reply_pkt->data.serverMsg.blockLen = 
						htonl(QUERY_CR_SIZE);
				reply_pkt->data.serverMsg.reqId = reqId;
				npkts ++;
				reply_pkt->type = (PKT_SERVERMSG);

				/* Send .... */

     			htonPkt(reply_pkt->type, reply_pkt);

				if (write (sockfd, retPkt, len) < 0)
				{
					NETERROR(MFIND,
				 	("%s write returned error %d\n",
				 	fn, errno));
				}

				len = sizeof(Pkt);
				queryCRIndex = 
			     		(QueryCRIndex *)(reply_pkt + 1);

				/* zero out the rest of packet - re-used */
				memset(queryCRIndex, 0, (max-len));
			}
		}

		/* One calling plan is completely done. If there is anything left,
		* we should send it, before we start sending another calling plan
		*/
     		reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);
     		reply_pkt->data.serverMsg.blockLen = htonl(QUERY_CP_SIZE);
		reply_pkt->data.serverMsg.reqId = reqId;

     		npkts++;
     		/* Send whatever is remaining */

		reply_pkt->type = (PKT_SERVERMSG);
     		reply_pkt->totalLen = (len);

		htonPkt(reply_pkt->type, retPkt);

     		if (write (sockfd, retPkt, len) < 0)
     		{
	  		NETERROR(MFIND, 
	     			("%s write returned error %d\n",
	     			fn, errno));
     		}
	
		tnpkts += npkts;

		npkts = 0;

		if (browseplans)
		{
			/* See if there are any other ports */
			inCRIndex = (QueryCRIndex *)
					((char *)inCRIndex + blockLen);
			if (((char *)inCRIndex + sizeof(QueryCRIndex)) <= 
				((char *)data_pkt + data_pkt->totalLen))
			{
				memcpy(cpname, inCRIndex->cpname, CALLPLAN_ATTR_LEN);
			}
			else
			{
				done = 1;
			}
		}
		else
		{
			done = 1;
		}
	}

     	NETDEBUG(MFIND, NETLOG_DEBUG1, 
		("%s npkts %d nplans %d\n", fn, tnpkts, nplans));
		
	CacheFreeIterator(cpbCache);

	return 1;

_send_pkt:
	reply_pkt->type = (PKT_SERVERMSG);
     	reply_pkt->totalLen = (len);
	reply_pkt->data.serverMsg.reqId = reqId;

     	htonPkt(reply_pkt->type, retPkt);

     	if (write (sockfd, retPkt, len) < 0)
     	{
	  	NETERROR(MFIND, 
	     		("%s write returned error %d\n",
	     		fn, errno));
     	}

     	return 1;
}

int
ProcessSrvrRegister(int sockfd, Pkt * data_pkt,  Pkt * not_needed)
{
    char fn[] = "ProcessSrvrRegister():";
    Pkt *reply_pkt;
    unsigned long 
		len = sizeof(PktHeader)+sizeof(ServerMsg), 
	 	max = SRVR_REG_MAX*SRVR_REG_SIZE+sizeof(PktHeader)+
				sizeof(ServerMsg);
    long retPkt[(SRVR_REG_MAX*SRVR_REG_SIZE+sizeof(PktHeader)+sizeof(ServerMsg))/sizeof(long)+1];
    RegisterIndex *registerIndex;
    CacheTableInfo *info, cacheInfoEntry = { 0 }, *ipCacheInfo, 
		*regCacheInfo;
	int browseports = 0, done = 0, npkts = 0;
	unsigned long uport, reqId, blockLen, *pUport;
     
#if 0	// not needed anymore applies only to iedge 1000
	if (data_pkt->totalLen > (sizeof(PktHeader)+sizeof(ServerMsg)))
	{
		return ProcessSrvrPortRegister(sockfd, data_pkt, NULL);
	}

    memset(retPkt, 0, max);

    /* Setup the return header */
    memcpy(retPkt, (char *)data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *)retPkt;
    reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */
	reply_pkt->data.serverMsg.blockLen = ntohl(sizeof(long));
	pUport = (unsigned long *)(&reply_pkt->data.serverMsg + 1);

    /* We will set the length in the end */

	registerIndex = &data_pkt->data.serverMsg.index.regIndex;
	reqId = data_pkt->data.serverMsg.reqId;
	blockLen = ntohl(data_pkt->data.serverMsg.blockLen);

	memcpy(&reply_pkt->data.serverMsg.index.regIndex,
		registerIndex, sizeof(RegisterIndex));

	ntohRegisterIndex(registerIndex);

	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
		("%s regid = %s ip = %s\n", fn, registerIndex->regid, 
		ULIPtostring(registerIndex->ipaddress)));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	/* Lookup the regid.
	* If there are ports mentioned in the packet, make only those
	* alive, otherwise make all configured ports alive
	*/ 
	regCacheInfo = CacheGet(regidCache, registerIndex->regid);

	if (regCacheInfo == NULL)
	{
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadField); 
		goto _error;
	}

	if ((registerIndex->ipaddress != 0) &&
		(ipCacheInfo = CheckDuplicateIedgeIpAddr(ipCache,
			&regCacheInfo->data, registerIndex->ipaddress)))
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG1,
			("%s Found an iedge with the same IP address %s/%d\n",
			fn, ipCacheInfo->data.regid, ipCacheInfo->data.uport));
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadField); 
		goto _error;
	}
	
	if (registerIndex->ipaddress == 0)
	{
		/* Write the up address back into the reply */
		registerIndex->ipaddress = ipCacheInfo->data.ipaddress.l;
	}

	/* Right now we will assume there are
	* no ports in this message, and we will use
	* the set of ports locally configured on the
	* iServer
	* In our reply, we would send back the information, about
	* which ports are configured.
	*/
	memcpy(cacheInfoEntry.data.regid, registerIndex->regid, REG_ID_LEN);

	for (uport=0; uport<MAX_IEDGE_PORTS; uport++)
	{
		if (!BITA_TEST(regCacheInfo->data.cfgports, uport))
		{
			/* This port is not configured */
			continue;
		}

		cacheInfoEntry.data.uport = uport;

		/* Port is configured */
		info = CacheGet(regCache, &cacheInfoEntry.data);
	
		if (info == NULL)
		{
			NETERROR(MREGISTER,
				("%s Port configured, but not found in reg cache\n",
				fn));
			continue;
		}

		if (registerIndex->ipaddress > 0)
		{
			NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s regid = %s uport %d ip = %s\n", fn, registerIndex->regid, 
				uport, ULIPtostring(registerIndex->ipaddress)));

			/* Mark this port as alive */
			info->data.stateFlags |= CL_ALIVE;
			info->data.ipaddress.l = registerIndex->ipaddress;
			info->data.rTime = (time(0));

			info->iserver_addr = 0;
			info->state = 0;

			BIT_SET(info->data.cap, CAP_UCC);
		}

		/* Fill in this port in our reply */
		*pUport++ = htonl(uport);
		len += sizeof(long);

		if ((len+SRVR_REG_SIZE) > max)
		{
			/* Send this packet now. We must set the rest
		 	* of the fields properly.
		 	*/
			reply_pkt->data.serverMsg.status = 
					htonl(ServerMsg_eCont);
			reply_pkt->totalLen = (len);
			reply_pkt->data.serverMsg.blockLen = 
						htonl(sizeof(long));

			reply_pkt->data.serverMsg.reqId = reqId;

			npkts ++;
			reply_pkt->type = (PKT_SERVERMSG);
			/* Send .... */

     		htonPkt(reply_pkt->type, reply_pkt);

			if (write (sockfd, retPkt, len) < 0)
			{
				NETERROR(MREGISTER,
			 		("%s write returned error %d\n",
				 	fn, errno));
			}

			len = sizeof(PktHeader)+sizeof(ServerMsg);
			pUport = (unsigned long *)(&reply_pkt->data.serverMsg + 1);
			memset(pUport, 0, max-len);
		}
	}
	
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

_error:
	CacheReleaseLocks(regCache);

	reply_pkt->type = (PKT_SERVERMSG);
   	reply_pkt->totalLen = (len);
   	reply_pkt->data.serverMsg.blockLen = htonl(sizeof(long));
	reply_pkt->data.serverMsg.reqId = reqId;

   	npkts++;

    /* Send whatever is remaining */
    htonPkt(reply_pkt->type, retPkt);

    if (write (sockfd, retPkt, len) < 0)
    {
	 	NETERROR(MFIND, 
	     		("%s write returned error %d\n",
	     		fn, errno));
    }

    NETDEBUG(MFIND, NETLOG_DEBUG1, 
		("%s npkts %d\n", fn, npkts));
		
#endif
    return 1;
}

int
ProcessSrvrPortRegister( int sockfd, Pkt * data_pkt,  Pkt * not_needed )
{
    char fn[] = "ProcessSrvrPortRegister():";
    Pkt *reply_pkt;
    unsigned long 
		len = sizeof(PktHeader)+sizeof(ServerMsg), 
	 	max = SRVR_PORTSTATUS_MAX*SRVR_PORTSTATUS_SIZE+sizeof(PktHeader)+
				sizeof(ServerMsg);
    long retPkt[(SRVR_PORTSTATUS_MAX*SRVR_PORTSTATUS_SIZE+
			sizeof(PktHeader)+sizeof(ServerMsg))/sizeof(long)+1];
    RegisterIndex *registerIndex;
    CacheTableInfo *info, *ipCacheInfo, *regCacheInfo;
	PhoNode phonode = { 0 };
	int browseports = 0, done = 0, npkts = 0;
	unsigned long uport, reqId, blockLen;
	PortStatusData *portStatus, *inPortStatus;
	int reason, rc;
	char *final = ((char *)data_pkt + data_pkt->totalLen);
     
    memset( retPkt, 0, max );

    /* Setup the return header */
    memcpy(retPkt, (char *)data_pkt, sizeof(Pkt));

    reply_pkt = (Pkt *) retPkt;
    reply_pkt->type = (PKT_SERVERMSG);
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk); /* By default */
	reply_pkt->data.serverMsg.blockLen = ntohl(sizeof(long));
	portStatus = (PortStatusData *)(&reply_pkt->data.serverMsg + 1);

    /* We will set the length in the end */

	registerIndex = &data_pkt->data.serverMsg.index.regIndex;
	reqId = data_pkt->data.serverMsg.reqId;
	blockLen = ntohl(data_pkt->data.serverMsg.blockLen);
	inPortStatus = (PortStatusData *)(&data_pkt->data.serverMsg + 1);

	if (registerIndex->ipaddress == 0)
	{
		NETERROR(MREGISTER,
			("%s No Ip Address found\n", fn));
		goto _error;
	}

	memcpy(&reply_pkt->data.serverMsg.index.regIndex,
		registerIndex, sizeof(RegisterIndex));

	ntohRegisterIndex(registerIndex);

	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
		("%s regid = %s ip = %s\n", fn, registerIndex->regid, 
		ULIPtostring(registerIndex->ipaddress)));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	regCacheInfo = CacheGet(regidCache, registerIndex->regid);

	if (regCacheInfo == NULL)
	{
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadField); 

		goto _error;
	}

	if ((registerIndex->ipaddress != 0) &&
		(ipCacheInfo = CheckDuplicateIedgeIpAddr(ipCache,
			&regCacheInfo->data, registerIndex->ipaddress)))
	{
		NETDEBUG(MREGISTER, NETLOG_DEBUG1,
			("%s Found an iedge with the same IP address %s/%d\n",
			fn, ipCacheInfo->data.regid, ipCacheInfo->data.uport));
     	reply_pkt->data.serverMsg.status = htonl(ServerMsg_eBadField); 

		goto _error;
	}
	
	while (((char *)inPortStatus + blockLen) <= final)
	{
		ntohPortStatus(inPortStatus);
		uport = inPortStatus->uport;

		if ((uport < 0) || (uport >= MAX_IEDGE_PORTS))
		{
			NETERROR(MREGISTER,	
				("%s Uport %d is out of range\n", fn, uport));
			rc = -1;
			reason = nextoneNoAlias;
		
			/* We must fill an error code in the packet */
			memcpy(portStatus, inPortStatus, sizeof(PortStatusData));

			portStatus->clientState &= ~CL_ACTIVE;
			portStatus->reason = reason;
			goto _report_port;
		}

		if (!BITA_TEST(regCacheInfo->data.cfgports, uport))
		{
			/* This port is not configured */
			NETDEBUG(MREGISTER,	NETLOG_DEBUG4,
				("%s Uport %d is not configured\n", fn, uport));
			rc = -1;
			reason = nextoneNoAlias;
		
			/* We must fill an error code in the packet */
			memcpy(portStatus, inPortStatus, sizeof(PortStatusData));
			portStatus->clientState &= ~CL_ACTIVE;
			portStatus->reason = reason;
			goto _report_port;
		}

		memset(&phonode, 0, sizeof(PhoNode));
		PortStatus2Phonode(inPortStatus, &phonode);

		memcpy(phonode.regid, registerIndex->regid, REG_ID_LEN);
		BIT_SET(phonode.sflags, ISSET_REGID);

		phonode.uport = uport;
		BIT_SET(phonode.sflags, ISSET_UPORT);

		phonode.ipaddress.l = registerIndex->ipaddress;
		BIT_TEST(phonode.sflags, ISSET_IPADDRESS);

		rc = ProcessPhonodeRegistration(&phonode, 0, &reason);

		if (rc < 0)
		{
			NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				("%s uport %d registration failed.\n", fn, uport));
		
			/* We must fill an error code in the packet */
			memcpy(portStatus, inPortStatus, sizeof(PortStatusData));
			portStatus->reason = reason;
		}
		else
		{
			/* Fill in this port in our reply */
			Phonode2PortStatus(&phonode, portStatus);
		}

	_report_port:
		htonPortStatus(portStatus);
	
		len += sizeof(PortStatusData);

		if ((len+SRVR_PORTSTATUS_SIZE) > max)
		{
			/* Send this packet now. We must set the rest
		 	* of the fields properly.
		 	*/
			reply_pkt->data.serverMsg.status = 
					htonl(ServerMsg_eCont);
			reply_pkt->totalLen = (len);
			reply_pkt->data.serverMsg.blockLen = 
						htonl(sizeof(PortStatusData));

			reply_pkt->data.serverMsg.reqId = reqId;

			npkts ++;
			reply_pkt->type = (PKT_SERVERMSG);
			/* Send .... */

     		htonPkt(reply_pkt->type, reply_pkt);

			if (write( sockfd, retPkt, len) < 0)
			{
				int lerrno = errno;

				NETERROR(MREGISTER,
			 		("%s write on fd %d returned - errno %d - %s \n",
				 	fn,
					sockfd,
					lerrno,
					strerror( lerrno ) ));
			}

			len = sizeof(PktHeader)+sizeof(ServerMsg);
			portStatus = (PortStatusData *)(&reply_pkt->data.serverMsg + 1);
			memset(portStatus, 0, max-len);
		}

		portStatus ++;

	_continue:
		inPortStatus = (PortStatusData *)((char *)inPortStatus + blockLen);
	}
	
    reply_pkt->data.serverMsg.status = htonl(ServerMsg_eOk);

_error:

	CacheReleaseLocks(regCache);

	reply_pkt->type = (PKT_SERVERMSG);
   	reply_pkt->totalLen = (len);
   	reply_pkt->data.serverMsg.blockLen = htonl(sizeof(PortStatusData));
	reply_pkt->data.serverMsg.reqId = reqId;

   	npkts++;

    /* Send whatever is remaining */
    htonPkt(reply_pkt->type, retPkt);

    if (write (sockfd, retPkt, len) < 0)
    {
	 	NETERROR(MFIND, 
	     		("%s write returned error %d\n",
	     		fn, errno));
    }

    NETDEBUG(MFIND, NETLOG_DEBUG1, 
		("%s npkts %d\n", fn, npkts));
		
    return 1;
}

#endif

/* return NOT_FOUND/INPROG/FOUND/ERROR */
int
FindRemote(ResolveHandle *rhandle)
{
	char fn[] = "FindRemote():";
	int retval;
	PhoNode *phonodep, *fphonodep, *rphonodep, *rfphonodep;

	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Entering\n", fn));
 
	// Initialize retval to what we are passed
	retval = rhandle->result;

	if (retval != CACHE_FOUND)
	{
		// This means that we dont have to do
		// anything here, as no ENUM gw is there in the
		// rfphonodep

		return retval;
	}

	rphonodep = rhandle->phonodep;
	rfphonodep = rhandle->rfphonodep;

	if (BIT_TEST(rfphonodep->cap, CAP_LRQ))
	{
#ifdef DONT_USE_ARQ_INSTEAD_OF_LRQ 
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s remote is an lrq gateway  - not supported\n", fn));
		/* use sgatekeeper instead of xgatekeeper */
		retval = FindRemoteByLRQ(rhandle);
#endif
	}
	else if (BIT_TEST(rfphonodep->cap, CAP_ENUMS))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s remote is an enum server\n", fn));
		retval = FindRemoteByEnum(rhandle);
	}

#if 0
	if (retval >= 0)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, 
			("%s Sent Request, removing client\n", fn));

		/* Remove the client handle from our client list */
	 	listDeleteItem(gisActiveCHandles, rhandle->chandle);
	}
#endif

	rhandle->result = retval;

	return retval;
}

/* this one, we will create a thread for now,
 * until the whole iServer behaves in a multithreaded
 * way
 */ 
#define MAX_RESULTS	2
int
FindRemoteByEnum(ResolveHandle *rhandle)
{
	char fn[] = "FindRemoteByEnum():";
	ENUM_record *results[MAX_RESULTS];
	char *client_default_domains[]={"trial.e164.com.",NULL,NULL,NULL};
	char filter[]="sip";
	int retval = CACHE_ERR, i;
	
	if (strlen(enumdomain))
	{
		client_default_domains[0] = enumdomain;
	}

	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Entering %s\n", fn, client_default_domains[0]));

	retval = enum_resolve(rhandle->rfphonodep->phone,
				filter, client_default_domains, results, MAX_RESULTS);


	if (retval < 0)
	{
		retval = CACHE_ERR;
		goto _return;
	}
	else if(retval == 0)
	{
		retval = CACHE_NOTFOUND;
		goto _return;
	}

	
	for (i=0; i<retval; i++)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s i=%d, uri=%s\n", fn, i , results[i]->uri));

		ProcessEnumUri(results[i]->uri, &(RH_FOUND_CACHE(rhandle, i)->data));

		/* Set the call sig port */
		rhandle->rfcallsigport = RH_FOUND_CACHE(rhandle, i)->data.callsigport;

		free_enum_record(results[i]);
	}

	retval = CACHE_FOUND;

_return:
	return retval;
}

/* Must be called inside thread */
int
ProcessEnumUri(char *inuri, InfoEntry *info)
{
	char fn[] = "ProcessEnumUri():";
	char *uri = NULL, *protStr = NULL, *nameStr = NULL, 
		*hostStr = NULL, *portStr = NULL, *rest = NULL;
	struct hostent hostentry, *hostp;
	char buffer[256];
	char s1[48];
	int herror;
	int retval = -1;
	
	uri = strdup(inuri);

	/* H.323 or Sip uri ? */

	protStr = strstr(uri, "h323://");
	if (protStr == NULL)
	{
		protStr = strstr(uri, "sip:");
		if (protStr)
		{
			/* sip url
			 * copy whole url into the contact field
			 */
			strncpy(info->contact, inuri + strlen("sip:"), SIPURL_LEN);
			BIT_SET(info->cap, CAP_SIP);
			retval = 1;
			goto _return;
		}
		goto _return;
	}

	rest = uri+strlen("h323://");

	/* Its an H.323 uri */
	nameStr = strtok_r(rest, "@", &hostStr);

	if (!hostStr)
	{
		hostStr = nameStr;
		nameStr = NULL;
	}

	if (nameStr)
	{
		strncpy(info->phone, nameStr, PHONE_NUM_LEN);
		BIT_SET(info->sflags, ISSET_PHONE);
	}

	hostStr = strtok_r(hostStr, ":", &portStr);

	if (hostStr == NULL)
	{
		goto _return;
	}

	/* Do DNS on hostStr */
	hostp = &hostentry;
	if (nx_gethostbyname_r(hostStr, hostp, buffer, 256, &herror))
	{
		info->ipaddress.l = ntohl(*(int *)hostp->h_addr_list[0]);
		BIT_SET(info->sflags, ISSET_IPADDRESS);
	}
	else
	{
		goto _return;
	}

	if (portStr)
	{
		info->callsigport = atoi(portStr);
	}
	else
	{
		info->callsigport = 1720;
	}

	BIT_SET(info->cap, CAP_IGATEWAY);
	BIT_SET(info->cap, CAP_H323);

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		("%s H.323 addr=%s/%d phone = %s\n", 
		fn, FormatIpAddress(info->ipaddress.l, s1), info->callsigport,
		info->phone));
_return:
	free(uri);
	return retval;
}

#if 0
int
FindRemoteByLRQ(ResolveHandle *rhandle)
{
	char fn[] = "FindRemote():";
	PhoNode *phonodep, *fphonodep, *rphonodep, *rfphonodep;
	LRQHandle *lrqHandle = NULL;
	int lrqsent = CACHE_ERR;

	if (!H323Enabled())
	{
		return lrqsent;
	}

	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* We may have to send an LRQ for either the primary 
	 * or rollover number, or BOTH
	 */
	rphonodep = rhandle->phonodep;
	rfphonodep = rhandle->rfphonodep;

	if (BIT_TEST(rfphonodep->cap, CAP_LRQ))
	{
		 /* Initialize the LRQ Handle */
		 if (rhandle->rfchandle)
		 {
			  NETERROR(MFIND, 
				("%s LRQ Handle already exists for primary\n", fn));
			  goto _return;
		 }
		 
		 rhandle->rfchandle = GisAllocCHandle(NULL, CLIENTHANDLE_H323LRQ);
		 CHRHandle(rhandle->rfchandle) = rhandle;

		 /* Create an LRQ Handle */
		 lrqHandle = GisAllocLRQHandle();
		 H323LRQHandle(CHH323Handle(rhandle->rfchandle)) = lrqHandle;

		 GisSetupLRQFromResolve(rhandle, lrqHandle, 1);
		 lrqsent = GkSendLRQ(rhandle->rfchandle);
	}

_return:
	if (lrqsent >= 0)
	{
		lrqsent = CACHE_INPROG;
	}

	return lrqsent;
}

/* Sending LRQs from the iServer is depracated as
 * of Rel 1.3
 */
int
GisProcessLCF(ClientHandle *chandle)
{
	char fn[] = "GisProcessLCF():";
	ResolveHandle *rhandle;
	int type = 0;
	Pkt *reply_pkt;

#if 0
	/* See if there is a ResolveHandle. If there is
	 * one, set up the relevent phonode.
	 */
	rhandle = (ResolveHandle *)chandle->rhandle;
	
	if (rhandle == NULL)
	{
		 NETDEBUG(MFIND, NETLOG_DEBUG1,
				("%s No ResolveHandle found for LRQ Handle\n", fn));
		 return 0;
	}

	if (rhandle->rfchandle == chandle)
	{
		 type = 1;
	}
	else
	{
		 NETERROR(MFIND, ("%s Unable to find type of node pr/rollover\n",
						  fn));
		 return 0;
	}

	/* Set up the Phonodes */
	RH_FOUND_NODE(rhandle, type)->ipaddress.l = 
		 H323LRQHandle(CHH323Handle(chandle))->ipaddress;
	BIT_SET(RH_FOUND_NODE(rhandle, type)->sflags, ISSET_IPADDRESS);
	rhandle->rfcallsigport = 
		 H323LRQHandle(CHH323Handle(chandle))->port;

	/* Reset the caps etc */
	RH_FOUND_NODE(rhandle, type)->cap = 0;
	BIT_SET(RH_FOUND_NODE(rhandle, type)->cap, CAP_IGATEWAY);
	RH_FOUND_NODE(rhandle, type)->clientState |= CL_ACTIVE;

	/* Depending on who the source is, we may need to do
	 * separate things
	 */
	switch(rhandle->chandle->type)
	{
	case CLIENTHANDLE_UCC:
	  /* Reply back to the iedge */
	  reply_pkt = rhandle->reply_pkt;
	  reply_pkt->type = PKT_FOUND;
	
	  htonPkt(reply_pkt->type, reply_pkt);

	  PktSend(CHUCCHandle(rhandle->chandle)->fd, reply_pkt);

	  break;
	case CLIENTHANDLE_H323ARQ:
	  GkSendARQConfirm(rhandle);
	  break;
	case CLIENTHANDLE_H323LRQ:
	  GkSendLRQConfirm(rhandle);
	  break;
	default:
		break;
	}

	GisDisableCHandle(rhandle->chandle);
	GisDeleteCHandle(rhandle->chandle);

	GisFreeRHandle(rhandle);
	
#endif
	return 0;
}

int
ProcessUnregisterAllIedgePorts(NetoidInfoEntry *infoEntry)
{
    CacheTableInfo *info = 0;
    char fn[] = "ProcessUnregisterAllIedgePorts():";

    /* Extract the ip entry, and using it, unregister all ports */
    CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

    /* Get rid of this ip address in our cache also */
    info = CacheGet(regidCache, infoEntry->regid);
    if (info)
    {
        GkUnregisterAllIedgePorts(NULL, &info->data);
    }
    else
    {
	NETDEBUG(MREGISTER, NETLOG_DEBUG4,
		("%s regid %s not configured\n", fn, infoEntry->regid));
    }

   CacheReleaseLocks(regCache);

   return(0);
}

/* Note: this does not do any proxy checks */
// Do unregistration by regid
int
GkUnregisterAllIedgePorts(DB db /* UNUSED */, NetoidInfoEntry *infoEntry)
{
	 int i;
	 PhoNode phonode;
	 CacheTableInfo *info = 0;
	 unsigned char tags[TAGH_LEN] = { 0 };
	 char fn[] = "GkUnregisterAllIedgePorts():";
	 RealmIP realmip;

	 /* This aged ip address represents a bunch of 
	  * iedge ports, which we must now disable.
	  * Using the ports mentioned in this iedge, 
	  * traverse the regCache, and disable the ports
	  */
	 memcpy(phonode.regid, infoEntry->regid, REG_ID_LEN);
	 BIT_SET(phonode.sflags, ISSET_REGID);

	 BITA_SET(tags, TAG_ACTIVE);

	 for (i=0; i<MAX_IEDGE_PORTS; i++)
	 {
		  if (BITA_TEST(infoEntry->cfgports, i))
		  {
			/* Look for this iedge port */
			phonode.uport = i;
	 		BIT_SET(phonode.sflags, ISSET_UPORT);

			info = (CacheTableInfo *)CacheGet(regCache, &phonode);

			if (info)
			{
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s regid %s/%d\n", fn, info->data.regid, 
					info->data.uport));
				/* This iedge port is to be aged now */

				/* We must mark in the db that this
				 * netoid is inactive now
				 */
				info->data.stateFlags &= ~(CL_ACTIVE);

	 			/* Update the database also */
				//UpdateNetoidDatabase(&info->data);
				DbScheduleIedgeUpdate(&info->data);
			}
		  }
	 }			   
	 
	 realmip.ipaddress = infoEntry->ipaddress.l;
	 realmip.realmId = infoEntry->realmId;
	 CacheDelete(ipCache, &realmip);
	 return(0);
}

#endif
int
GkUnregisterAllIedgePortsByIP(DB db, NetoidInfoEntry *infoEntry, int gws, int sgks)
{
	 int i;
	 PhoNode phonode;
	 CacheTableInfo *info = 0;
	 unsigned char tags[TAGH_LEN] = { 0 };
	 char fn[] = "GkUnregisterAllIedgePortsByIP():";
	 int rc;

	 /* This aged ip address represents a bunch of 
	  * iedge ports, which we must now disable.
	  * Using the ports mentioned in this iedge, 
	  * traverse the regCache, and disable the ports
	  */
	 memcpy(phonode.regid, infoEntry->regid, REG_ID_LEN);
	 BIT_SET(phonode.sflags, ISSET_REGID);

	 BITA_SET(tags, TAG_ACTIVE);

	 for (i=0; i<MAX_IEDGE_PORTS; i++)
	 {
		  if (BITA_TEST(infoEntry->ports, i))
		  {
			/* Look for this iedge port */
			phonode.uport = i;
	 		BIT_SET(phonode.sflags, ISSET_UPORT);

			info = (CacheTableInfo *)CacheGet(regCache, &phonode);

			if ((info) &&
				((gws && !IsSGatekeeper(&info->data)) ||
				 (sgks && IsSGatekeeper(&info->data))) )
			{
				NETDEBUG(MREGISTER, NETLOG_DEBUG4,
					("%s regid %s/%lu\n", fn, info->data.regid, 
					info->data.uport));

				/* This iedge port is to be aged now */

				/* We must mark in the db that this
				 * netoid is inactive now
				 */
				info->data.stateFlags &= ~(CL_ACTIVE);

				DeleteIedgeIpAddr(ipCache, &info->data);

	 			/* Update the database also */
				//UpdateNetoidDatabase(&info->data);
				DbScheduleIedgeUpdate(&info->data);
			}
		  }
	 }			   
	 
	 /* Get rid of this ip address in our cache also */
	 /* if this line is uncommented make sure you pass a RealmIP *
	  * as second argument
	  */
//	 CacheDelete(ipCache, &infoEntry->ipaddress.l);
	 return(0);
}

#if 0
int
GisHandleRegister(XMLCacheCb *cb)
{
	char fn[] = "GisHandleRegister():";
	NetoidInfoEntry *infoEntry;
	unsigned long uport, nports = 0;
	int type;

	infoEntry = &cb->infoEntry;

	if (!BIT_TEST(infoEntry->sflags, ISSET_IPADDRESS) ||
		(infoEntry->ipaddress.l == 0))
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Unregister Message for regid %s\n",
					fn, infoEntry->regid));
		 type = PKT_UNREGISTER;
	}
	else 
	{
		 NETDEBUG(MREGISTER, NETLOG_DEBUG4,
				  ("%s Register Message for regid %s\n",
					fn, infoEntry->regid));
		 type = PKT_REGISTER;
	}
		
	if (infoEntry->nports == 0)
	{
		/* This is unconditional */
		if (type == PKT_UNREGISTER)
		{
			ProcessUnregisterAllIedgePorts(infoEntry);
		}
	}

	/* All the uports are mentioned inside the infoEntry */
	for (uport = 0; (uport < MAX_IEDGE_PORTS)&&(nports < infoEntry->nports);
		 uport ++)
	{
		 if (BITA_TEST(infoEntry->ports, uport))
		 {
			  infoEntry->uport = uport;
			  BIT_SET(infoEntry->sflags, ISSET_UPORT);
			  nports ++;

			  if (type == PKT_UNREGISTER)
			  {
					ProcessUnregisterIedge(infoEntry);
			  }
			  else if (type == PKT_REGISTER)
			  {
					/*
					* ProcessRegisterIedge(infoEntry);
					*/
			  }
		 }
	}

	return(0);
}

#endif
