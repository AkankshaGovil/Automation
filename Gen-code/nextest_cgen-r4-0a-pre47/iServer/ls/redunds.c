/**************************************************************************
 * FILE: redunds.c
 *
 * DATE:  MARCH 8th 1998
 *
 * Copyright (c) 1998 Netoids Inc.
 ***************************************************************************/ 	
static char const rcsid[] = "$Id: redunds.c,v 1.34.2.4 2004/08/09 16:27:55 amar Exp $";

#if 0
#include "generic.h"
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
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define LS_READ_SINGLESHOT	4096	/* Attempt to read 4k */

#include "spversion.h"

#include "generic.h"
#include "srvrlog.h"
#include "bits.h"
#include "ipc.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"

#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "sconfig.h"
#include "avl.h"
#include "gis.h"

extern int PktLen(char *buf);
unsigned long timeSetReference = 0;
extern long offsetSecs;

#define RedundsActive()	(localConfig.redconfig.fd > 0)

int ExamineUpdateCache(LsMemStruct *lsMem, int fd);

int
GisStartElem(void *userData, int tag, int depth, const char **atts)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;
	int i;

	switch (tag)
	{
	case TAG_SYN:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("GisStartElem: Starting SYN\n"));
		break;
	case TAG_DSYN:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("GisStartElem: Ending SYN\n"));
		RedundsHandleSyncEnd();
		break;
	case TAG_ADV:
	case TAG_CACHEENTRY:
	case TAG_CACHEUPDATE:
	case TAG_QUERY:
		/* We want to wash out uneeded history */
		memset(&cb->tagh[0], 0, TAGH_LEN);
		for (i=0; i < cb->depth + 1; i++)
		{
			BITA_SET(cb->tagh, cb->inFieldType[i]);
		}
		break;
	case TAG_REG:
		/* As part of rel 1.2, there may be parameters
		 * here instead of just data types
		 */
		break;

	default:
		break;
	}

	return rc;
}

int
GisEndElem(void *userData, int tag, int depth)
{
	int rc = 0;
  	XMLCacheCb *cb = (XMLCacheCb *)userData;

	switch (tag)
	{
	case TAG_SYN:
		break;
	case TAG_ADV:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("GisEndElem: Ending ADV"));

		RedundsHandleAdv(cb);

		break;
	case TAG_CACHEENTRY:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("RedundsEndElem: Ending CACHEENTRY"));

		RedundsHandleCacheEntry(cb);

		break;
	case TAG_QUERY:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("RedundsEndElem: Ending QUERY"));
		
		RedundsHandleQuery(&cb->infoEntry);

		break;
	case TAG_CACHEUPDATE:
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("RedundsEndElem: Ending Cache Update"));

		RedundsHandleCacheUpdate(cb);

		break;
	case TAG_REG:
	    NETDEBUG(MRED, NETLOG_DEBUG2,
				 ("GisEndElem: Ending REGISTER\n"));
		GisHandleRegister(cb);
		break;

#if 0
	case TAG_UPORT:
	    NETDEBUG(MRED, NETLOG_DEBUG2,
				 ("GisEndElem: Ending REGISTER for UPORT\n"));
		GisHandleRegister(cb);
		break;
#endif
		
	default:
		break;
	}

	return rc;
}

int
RedundsConnReceive(int csock, FD_MODE rw, void *data)
{
	char fn[] ="RedundsConnReceive():";
     	int rc, len = sizeof(struct sockaddr_in), one = 1;
     	struct sockaddr_in peer, me;
        char buf[INET_ADDRSTRLEN];
	int 	nbio = 0;

     	/* Accepts signalling connections and adds them back to the set of fds
      	* to be chosen with control packet receive as the callback... ;-)
      	*/
     	bzero (&peer, sizeof(struct sockaddr_in));

     	rc = accept(csock, (PROTOID_SOCKADDR *)&peer,  &len);
     
     	if ( rc < 0)
     	{
	  	NETERROR(MRED, ("%s accept error....%d\n", fn, errno));
	  	return 0;
     	}
	
	NETDEBUG(MRED, NETLOG_DEBUG1,
		("%s Received connection request from %s\n",
			fn, 
			inet_ntop( AF_INET, &peer.sin_addr, buf, INET_ADDRSTRLEN)));

	NETDEBUG(MRED, NETLOG_DEBUG1,
		("%s expect from %s\n",
			fn, 
			inet_ntop( AF_INET, &localConfig.redconfig.redunds.location.address.sin_addr, buf, INET_ADDRSTRLEN)));

	if (memcmp(&peer.sin_addr.s_addr, 
		&localConfig.redconfig.redunds.location.address.sin_addr,
		sizeof(peer.sin_addr.s_addr)))
	{
		NETERROR(MRED, 
			("%s Addresses dont match 0x%x != 0x%x, disconnecting\n",
				fn,
				peer.sin_addr.s_addr, 
			localConfig.redconfig.redunds.location.address.sin_addr.s_addr));
		shutdown(rc, 2);
		close(rc);
		return -1;
	}

	/* Make this socket blocking */
	if (ioctl(rc, FIONBIO, &nbio) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	if (RedundsPolarity(rc) < 0)
	{
		NETERROR(MRED,
			("%s Peer should have lower address, disconnecting", fn));
		shutdown(rc, 2);
		close(rc);
		return -1;
	}

	if (RedundsActive())
	{
		NETERROR(MRED,
			("%s Peer already connected\n", fn));
		NETERROR(MRED,
			("%s Disconnecting previous connection, accepting current one\n",
			fn));

		NetFdsDeactivate(&lsnetfds, localConfig.redconfig.fd, FD_RW);
		shutdown(localConfig.redconfig.fd, 2);
		close(localConfig.redconfig.fd);
		localConfig.redconfig.fd = -1;
	}

	RedundsSetOptions(rc);

     	localConfig.redconfig.fd = rc;
	localConfig.redconfig.state = PEERSTATE_SYN;
     
     	NetFdsAdd (&lsnetfds, rc, FD_READ, (NetFn) RedundsPacketReceive, 
			(NetFn) 0, NULL, NULL);
     
	/* Now we should probably kill our own efforts to set up a redundant peer */
	RedundsDisconnectLocal();

	RedundsDoSync();

     	return 0;
}

int
RedundsConnected(int rc, void *data)
{
	char fn[] = "RedundsConnected():";
	ConnHandle *handle = (ConnHandle *)data;
	int s = CONN_ConnFd(handle);

	NETDEBUG(MRED, NETLOG_DEBUG1,
		("RedundsConnected:: Got Connected to redundant server\n"));

	if (RedundsPolarity(s) > 0)
	{
		NETERROR(MRED,
			("%s Peer has a lower Ip address, shutting down !!...\n", fn));
		shutdown(s, 2);
		close(s);
		goto _return;
	}

	/* If we got connected, and we initiated the connection. This
	 * one is our parent.
	 */
	localConfig.redconfig.isparent = 1;

	RedundsSetOptions(s);

     	localConfig.redconfig.fd = s;
	localConfig.redconfig.state = PEERSTATE_SYN;
     
     	NetFdsAdd (&lsnetfds, s, FD_READ, (NetFn) RedundsPacketReceive, 
			(NetFn) 0, NULL, NULL);

	RedundsDoSync();

_return:
	CONN_ConnFd(handle) = -1;
	CONN_FreeHandle(handle);

	/* Cleanup the handle */
	localConfig.redconfig.handle = NULL;
	return(0);

}

int
RedundsDoSync(void)
{
	/* If we are the master (parent), start the sync */
	if (localConfig.redconfig.isparent == 0)
	{
		/* Synchronize the time first */
		if ((localConfig.redconfig.havetime == 1) &&
			(localConfig.redconfig.hastime == 1))
		{
			/* We are master */
			if (Sync(1) > 0)
			{
				PerformSync();
			}
		}
		else if (localConfig.redconfig.timereq == 0)
		{
			SyncTime();
		}
	}
	else if (localConfig.redconfig.timereq == 0)
	{
		SyncTime();
	}
	return(0);
}

int
Sync(int start)
{
	static char xmlEnc[256];
	int len = 0;

	if (start == 1)
	{
		len += sprintf(xmlEnc, "<MSG><SYN></SYN>");
	}
	else
	{
		len += sprintf(xmlEnc, "<DSYN></DSYN></MSG>");
	}

	return SendXML(localConfig.redconfig.fd, xmlEnc, len);
}

int
RedundsHandleSyncEnd(void)
{
	/* If we are the slave, we have to start the sync now */
	if (localConfig.redconfig.isparent == 1)
	{
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("RedundsHandleSyncEnd: Sync End from Master"));
		SlaveSync();
		SlaveQuery();
	}
	else
	{
		NETDEBUG(MRED, NETLOG_DEBUG2,
			("RedundsHandleSyncEnd: Sync End from slave, changing state"));
		localConfig.redconfig.state = PEERSTATE_UPDT;
	}
	return(0);
	
}

int
RedundsWaitExpiry(struct Timer *t)
{
	RedundsConnTmout(APP_CONNTIMEOUT, (ConnHandle *)t->data);
	return(0);
}

int
RedundsFreeHandle(ConnHandle *handle)
{
	CONN_ConnFd(handle) = -1;
	CONN_FreeHandle(handle);

	/* Cleanup the handle */
	localConfig.redconfig.handle = NULL;

	return 0;
}

int
RedundsConnError(int rc, void *data)
{
	ConnHandle *handle = (ConnHandle *)data;
	static struct itimerval wait;
	void *timerdata;

	NETDEBUG(MRED, NETLOG_DEBUG1,
		("RedundsConnError:: Error connecting to redundant server\n"));

	if (CONN_ConnFd(handle)>= 0)
	{
		/* Must remove this fd from net fds... */
		NetFdsDeactivate(&lsnetfds, CONN_ConnFd(handle), FD_RW);
		close(CONN_ConnFd(handle));
		CONN_ConnFd(handle)= -1;
	}

	/* Kill the timer running on connection */
	if (CONN_ConnTimer(handle))
	{
		NETDEBUG(MRED, NETLOG_DEBUG1,
			("Deleting the timer on conn...\n"));

		if(timerDeleteFromList(&localConfig.timerPrivate, 
			CONN_ConnTimer(handle), &timerdata))
		{
			if(timerdata)
			{
				free(timerdata);
			}
		}

		CONN_ConnTimer(handle)= 0;
	}

	/* Free the handle */
	CONN_ConnFd(handle) = -1;
	CONN_FreeHandle(handle);

	/* Cleanup the handle */
	localConfig.redconfig.handle = NULL;


	/* Setup a timer to expire, which will later on call
	 * RedundsConnTimeout
	 */
	memset(&wait, 0, sizeof(struct itimerval));
	wait.it_value.tv_sec = 15;

	localConfig.timer = timerAddToList(&localConfig.timerPrivate, &wait, 1,
		PSOS_TIMER_REL, "RedErr", RedundsWaitExpiry, NULL);
	return(0);
}

int
RedundsConnTmout(int rc, void *data)
{
	NETDEBUG(MRED, NETLOG_DEBUG1,
		("RedundsConnTmout:: Timed out connecting to redundant server\n"));

	SetupRedunds(RedundsConnected, RedundsConnError, RedundsConnTmout);
	return(0);
}

/* s must be closed by the caller */
int
RedundsDeactivate()
{
	NETDEBUG(MRED, NETLOG_DEBUG1,
		("RedundsDeactivate: Entering\n"));

	if (RedundsActive())
	{
		NetFdsDeactivate(&lsnetfds, localConfig.redconfig.fd, FD_RW);
		ResetRedundsCfg();
	}

	/* If any iedges in my cache are owned by the redundant server,
	 * then clean them up
	 */	
	CleanupIserverCache(localConfig.redconfig.connEntry.addr.sin_addr.s_addr);
	return(0);
}

int
RedundsDisconnectLocal()
{
	ConnHandle *handle = localConfig.redconfig.handle;
	void *timerdata;

	if (handle != NULL)
	{
		NETDEBUG(MRED, NETLOG_DEBUG1, 
			("Bring down local efforts to connect with redundant server\n"));

		if (CONN_ConnFd(handle)>= 0)
		{
			close(CONN_ConnFd(handle));
			CONN_ConnFd(handle)= -1;
		}

		if (CONN_ConnTimer(handle))
		{
			if(timerDeleteFromList(&localConfig.timerPrivate,
				CONN_ConnTimer(handle), &timerdata))
			{
				free(timerdata);
			}

			CONN_ConnTimer(handle) = 0;
		}

		CONN_FreeHandle(handle);
	}

	/* Close any timers running */
	if (localConfig.timer)
	{
		if(timerDeleteFromList(&localConfig.timerPrivate,
			localConfig.timer, &timerdata))
		{
			free(timerdata);
		}

		localConfig.timer = 0;
	}

	localConfig.redconfig.handle = NULL;
	return(0);
}

/* Disconnect the connection with the redundant Server completely,
 * release the handle.
 * Also call the Deactivate function, so that the application handles
 * also get cleared out
 */
int
RedundsDisconnect()
{
	RedundsDeactivate();

	RedundsDisconnectLocal();

	if (RedundsActive())
	{
		/* Check if the fd is active, as it might be a connection
		 * which we received
		 */
		close(localConfig.redconfig.fd);
	}

	localConfig.redconfig.fd = -1;
	return(0);
}

int
RedundsReconnect()
{
	 char fn[] = "RedundsReconnect():";
	 static struct itimerval wait;
	 void *timerdata;

	 if (localConfig.redconfig.isparent == 1)
	 {
		  NETDEBUG(MRED, NETLOG_DEBUG1,
				("%s Re-initiating connection process with parent\n", fn));
		if (localConfig.timer)
		{
			if(timerDeleteFromList(&localConfig.timerPrivate,
				localConfig.timer, &timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}

			localConfig.timer = 0;
		}

		if (localConfig.redconfig.handle)
		{
			RedundsDisconnect();
		}

		NETDEBUG(MRED, NETLOG_DEBUG1,
			("%s Installing timer for connecting to redundant\n", fn));

		/* Setup a timer to expire, which will later on call
	 	* RedundsConnTimeout
	 	*/
		memset(&wait, 0, sizeof(struct itimerval));
		wait.it_value.tv_sec = 15;

		localConfig.timer = 
			 timerAddToList(&localConfig.timerPrivate, &wait, 1,
			 PSOS_TIMER_REL, "RedErr", RedundsWaitExpiry, NULL);
	 }
	 return(0);
}

int
RedundsCacheUpdate(int type, void *entry, unsigned char tags[])
{
	char buffer[4096];
	int len = 0;

	if (!RedundsActive())
	{
		NETDEBUG(MRED, NETLOG_DEBUG1,
			("RedundsCacheUpdate: No Redundant server. Update Dropped"));
		return 0;
	}

	/* First check if there are any pending updates
	 * in the update list. We should send them first.
	 * Move this into a separate thread, when we get
	 * threads to work on Linux.
	 */
	ExamineUpdateCache(lsMem, localConfig.redconfig.fd);

#ifdef needed
	TaghPrint(MRED, NETLOG_DEBUG4, tags, TAGH_LEN);
#endif
		
	/* Construct the update */
#if 0
	len += AddUpdate(type, &buffer[0], 1500, entry, tags);
#endif

	/* Send the whole entry */
	len += AddEntry(type, &buffer[0], 4096, entry);

	SendXML(localConfig.redconfig.fd, buffer, len);	
	return(1);
}

int
RedundsQueryInfoEntry(InfoEntry *infoEntry)
{
	char buffer[4096];
	int len = 0;

	if (!RedundsActive())
	{
		NETDEBUG(MRED, NETLOG_DEBUG1,
			("RedundsQueryInfoEntry: No Redundant server. Update Dropped"));
		return 0;
	}

#ifdef needed
	TaghPrint(MRED, NETLOG_DEBUG4, tags, TAGH_LEN);
#endif
		
	/* Construct the update */
	len += AddQuery(TAG_IEDGE, &buffer[0], 4096, infoEntry, NULL);

	SendXML(localConfig.redconfig.fd, buffer, len);	
	return(1);
}

/* For both vpn and vpng */
int
AddAdvertisement(int type, char *buf, int buflen, void *entry)
{
	NetoidInfoEntry *netInfo = (NetoidInfoEntry *)entry;
	VpnEntry *vpnEntry = (VpnEntry *)entry;
	VpnGroupEntry *vpnGroupEntry = (VpnGroupEntry *)entry;
	VpnRouteEntry *routeEntry = (VpnRouteEntry *)entry;
	CallPlanEntry *cpEntry = (CallPlanEntry *)entry;
	char tags[TAGH_LEN] = { 0 };
	int len = 0;

	len += sprintf(buf+len, "<ADV>");

	switch (type)
	{
	case TAG_IEDGE:
		BITA_SET(tags, TAG_IEDGE);
		BITA_SET(tags, TAG_REGID);
		BITA_SET(tags, TAG_UPORT);
		BITA_SET(tags, TAG_PHONE);
		BITA_SET(tags, TAG_VPNPHONE);
		BITA_SET(tags, TAG_PROXYS);
		BITA_SET(tags, TAG_PROXYC);
		BITA_SET(tags, TAG_RTIME);

		len += XMLEncodeInfoEntry(netInfo, buf+len, buflen-len, tags);

		break;
	case TAG_VPN:
		BITA_SET(tags, TAG_VPN);
		BITA_SET(tags, TAG_VPNEXTLEN);
		BITA_SET(tags, TAG_VPNG);

		len += XMLEncodeVpnEntry(vpnEntry, buf+len, buflen-len, tags);

		break;

	case TAG_VPNG:
		BITA_SET(tags, TAG_VPNG);

		len += XMLEncodeVpnGEntry(vpnGroupEntry, buf+len, buflen-len, tags);
	
		break;
	case TAG_CP:
	    len += XMLEncodeCPEntry(cpEntry, buf+len, buflen-len, NULL);
		break;
	case TAG_CR:
	    len += XMLEncodeCREntry(routeEntry, buf+len, buflen-len, NULL);
		break;
	default:
		break;
	}

	len += sprintf(buf+len, "</ADV>");

	return len;
}

/* Only for iedge's right now */
int
AddQuery(int type, char *buf, int buflen, void *entry)
{
	NetoidInfoEntry *netInfo = (NetoidInfoEntry *)entry;
	char tags[TAGH_LEN] = { 0 };
	int len = 0;

	len += sprintf(buf+len, "<QY>");

	BITA_SET(tags, TAG_REGID);
	BITA_SET(tags, TAG_UPORT);
	BITA_SET(tags, TAG_PHONE);
	BITA_SET(tags, TAG_VPNPHONE);
	BITA_SET(tags, TAG_PROXYS);
	BITA_SET(tags, TAG_PROXYC);
	BITA_SET(tags, TAG_RTIME);

	len += XMLEncodeInfoEntry(netInfo, buf+len, buflen-len, tags);

	len += sprintf(buf+len, "</QY>");

	return len;
}

int
RedundsHandleAdv(XMLCacheCb *cb)
{
	InfoEntry *infoEntry = &cb->infoEntry;
	VpnEntry *vpnEntry = &cb->vpnEntry;
	VpnGroupEntry *vpnGroupEntry = &cb->vpnGroupEntry;
	CallPlanEntry *cpentry = &cb->cpEntry;
	VpnRouteEntry *crentry = &cb->routeEntry;

	/* Look at what kind of advertisement came in */
	if (BITA_TEST(cb->tagh, TAG_IEDGE))
	{
		RedundsHandleInfoAdv(infoEntry);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPN))
	{
		RedundsHandleVpnAdv(vpnEntry);
	}
	else if (BITA_TEST(cb->tagh, TAG_CP))
	{
		RedundsHandleCPAdv(cpentry);
	}
	else if (BITA_TEST(cb->tagh, TAG_CR))
	{
		RedundsHandleCRAdv(crentry);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPNG))
	{
		RedundsHandleVpnGAdv(vpnGroupEntry);
	}
	
	return 0;
}

int
RedundsHandleCacheEntry(XMLCacheCb *cb)
{
	InfoEntry *infoEntry = &cb->infoEntry;
	VpnEntry *vpnEntry = &cb->vpnEntry;
	VpnGroupEntry *vpnGroupEntry = &cb->vpnGroupEntry;
	CallPlanEntry *cpentry = &cb->cpEntry;
	VpnRouteEntry *routeEntry = &cb->routeEntry;

#ifdef needed
	TaghPrint(MRED, NETLOG_DEBUG1, cb->tagh, TAGH_LEN);
#endif

	/* Look at what kind of advertisement came in */
	if (BITA_TEST(cb->tagh, TAG_IEDGE))
	{
		RedundsHandleNetoidCacheEntry(infoEntry);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPN))
	{
		RedundsHandleVpnAdv(vpnEntry);
	}
	else if (BITA_TEST(cb->tagh, TAG_CP))
	{
		RedundsHandleCPAdv(cpentry);
	}
	else if (BITA_TEST(cb->tagh, TAG_CR))
	{
		RedundsHandleCRAdv(routeEntry);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPNG))
	{
		RedundsHandleVpnGAdv(vpnGroupEntry);
	}
	
	return 0;
}

int
RedundsHandleCacheUpdate(XMLCacheCb *cb)
{
	InfoEntry *infoEntry = &cb->infoEntry;
	VpnEntry *vpnEntry = &cb->vpnEntry;
	VpnGroupEntry *vpnGroupEntry = &cb->vpnGroupEntry;
	CallPlanEntry *cpentry = &cb->cpEntry;
	VpnRouteEntry *routeEntry = &cb->routeEntry;

	/* Look at what kind of advertisement came in */
	if (BITA_TEST(cb->tagh, TAG_IEDGE))
	{
		RedundsHandleNetoidCacheUpdate(cb);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPN))
	{
		RedundsHandleVpnCacheUpdate(cb);
	}
	else if (BITA_TEST(cb->tagh, TAG_CP))
	{
		RedundsHandleCPCacheUpdate(cb);
	}
	else if (BITA_TEST(cb->tagh, TAG_CR))
	{
		RedundsHandleCRCacheUpdate(cb);
	}
	else if (BITA_TEST(cb->tagh, TAG_VPNG))
	{
		RedundsHandleVpnGCacheUpdate(cb);
	}
	
	return 0;
}

int
RedundsPacketReceive(int csock, FD_MODE rw, void *data)
{
	char *start, *end;
	static long buffer[4096/sizeof(long)+1];
	static struct sockaddr_in  remoteaddr;
	static int remotelen, nbytes;

	NETDEBUG(MRED, NETLOG_DEBUG2,
		("RedundsPacketReceive: Received packet\n"));

	/* Check for any kinds of errors */
	if (SocksCheckError(csock) < 0)
	{
		NETERROR(MRED,
			("Detected an error on the socket %d\n", csock));
		RedundsDisconnect();
		RedundsReconnect();
		return -1;
	}

	bzero (&remoteaddr, sizeof(struct sockaddr_in));
	remotelen = sizeof(struct sockaddr_in);

	start = end = (char *)&buffer[0];

	nbytes = get_next_packet_from(csock, (char *)&buffer[0], &start, &end, PktLen,
		(unsigned int) sizeof(GPktHeader), (unsigned int) 3500,
		&remoteaddr, &remotelen);

	if (nbytes <= 0)
	{
		NETERROR(MRED,
			("RedundsPacketReceive: Read Error on socket %d", csock ));

		RedundsDisconnect();
		RedundsReconnect();

		return -1;
	}

	ProcessRedundsPacket(csock, buffer, nbytes);
	return(0);
}

int
RedundsCfg(void)
{
	int static ncalls = 0;
	void *timerdata;

	if (ncalls ++ == 0)
	{
		/* Do not do anything at first time calling */
		return 0;
	}

	if (redunds == NULL)
	{
		/* Check if we have any connection with any redundant server */
		NETDEBUG(MRED, NETLOG_DEBUG1,
			("No redundant server\n"));
		RedundsDisconnect();

		memset(&localConfig.redconfig.redunds.location.address,
			0, sizeof(struct sockaddr_in));

		if (localConfig.timer)
		{
			if(timerDeleteFromList(&localConfig.timerPrivate,
				localConfig.timer, &timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}

			localConfig.timer = 0;
		}
	}
	else
	{
		/* Check if we have an ongoing connection with a different server */
		if (RedundsActive())
		{
			if (localConfig.redconfig.redunds.location.address.sin_addr.s_addr ==
				redunds->location.address.sin_addr.s_addr)
			{
				NETDEBUG(MRED, NETLOG_DEBUG1,
					("Redundant configuration unchanged\n"));
				return 0;
			}
			else
			{
				NETDEBUG(MRED, NETLOG_DEBUG1,
					("Redundant configuration changed\n"));
				RedundsDisconnect();
			}
		}

		if (localConfig.timer)
		{
			if(timerDeleteFromList(&localConfig.timerPrivate,
				localConfig.timer, &timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}

			localConfig.timer = 0;
		}

		redunds->location.address.sin_port = htons(LUS_REDUNDS_PORT_NUMBER);

		memcpy(&localConfig.redconfig.redunds,
			redunds, sizeof(serplex_config));
		memcpy(&localConfig.redconfig.connEntry.addr,
			&redunds->location.address, 
			sizeof(struct sockaddr_in));
		memset(&localConfig.redconfig.connEntry.tmout, 0,
			sizeof(struct itimerval));
		localConfig.redconfig.connEntry.tmout.it_value.tv_sec = 15;
		localConfig.redconfig.connEntry.tmout.it_value.tv_usec = 0;
		localConfig.redconfig.connEntry.xTries = 1;

		ResetRedundsCfg();

		/* See if its our higher IP address */
		SetupRedunds(RedundsConnected, RedundsConnError, RedundsConnTmout);
	}
	return(1);
}

int
ResetRedundsCfg(void)
{
	localConfig.redconfig.timereq = 0;
	localConfig.redconfig.havetime = 0;
	localConfig.redconfig.hastime = 0;
	return(0);
}

ConnHandle *
RedundsGetHandle(connAppCB connectCB, connAppCB errorCB, connAppCB tmoutCB)
{
	ConnHandle *handle;
	
	/* Initiate the connection and install a
	 * callback. When the connection is done, we
	 * will send the lookup packet.
	 */

	handle = CONN_GetNewHandle();

	handle->appTimers = &localConfig.timerPrivate;
	handle->netfds = &lsnetfds;
	handle->nTries = 0;
	handle->userCB = USER_ConnectResponse;
	handle->storeData = &localConfig.redconfig;

	handle->appCB[APP_CONNECTED] = connectCB;
	handle->appCB[APP_CONNERROR] = errorCB;
	handle->appCB[APP_CONNTIMEOUT] = tmoutCB;

	handle->appCB[APP_CONNINPROG] = USER_ConnectResponse;

	CONN_NBlock(handle) = 1;
	CONN_ConnFd(handle) = -1;
	CONN_ConnTimer(handle) = 0;
	CONN_RefCount(handle) = 0;

	return handle;
}

int
SetupRedunds(connAppCB connectCB, connAppCB errorCB, connAppCB tmoutCB)
{
	ConnHandle *handle;
	
	/* Initiate the connection and install a
	 * callback. When the connection is done, we
	 * will send the lookup packet.
	 */

	handle = RedundsGetHandle(connectCB, errorCB, tmoutCB);

	CONN_ConnEntry(handle) = 
		(ConnEntry *)&localConfig.redconfig.connEntry;

	if (CONN_ConnEntry(handle))
	{
		USER_ConnectResponse(CONN_Connect(handle), handle);
	}
	else
	{
		NETDEBUG(MRED, NETLOG_DEBUG1, ("No Server Found\n"));

		/* Argh,... */
		CONN_FreeHandle(handle);

		return 1;
	}

	/* Save the handle in case the user wants to delete
	 * the connection before we are successful
	 */
	localConfig.redconfig.handle = handle;

	return 0;
}

int
ProcessRedundsPacket(int s, char *buf, int len)
{
	PktHeader *hdr;
	int     type;

	hdr = (PktHeader *)buf;
	type = ntohl(hdr->type);

	ntohPkt(type, (Pkt *)buf);

	switch (type)
	{
	case PKT_XML:
		NETDEBUG(MRED, NETLOG_DEBUG4,
			("ProcessRedundsPacket: Received XML\n"));

		ProcessXMLEncoded(s, buf, NULL,
			0, 0, 0);
		break;
	case PKT_NEXTIME:
	     	NETDEBUG(MRED, NETLOG_DEBUG1, ("Recd. PKT_NEXTIME \n"));
	     	ProcessTimePkt(s, buf, buf, NULL, NULL, NULL);
		break;
	default:
		NETERROR(MRED, ("ProcessRedundsPacket Unknown type %d\n",
			type));
		break;
	}	
	return(0);
}

/* This must be cleaned up... */
int
ProcessTimePkt(int s, char *buf)
{
	Pkt *pktP = (Pkt *)buf;

	if (pktP->data.nextime.mode == NEXTIME_CLIENT)
	{
	     	NETDEBUG(MRED, NETLOG_DEBUG1, ("PKT_NEXTIME: Request\n"));
	     	ProcessNexTime(s, buf, buf, NULL, NULL, NULL);

		/* Hack for time synchronization in redundancy */
		localConfig.redconfig.hastime = 1;
		RedundsDoSync();
	}
	else
	{
	     	NETDEBUG(MRED, NETLOG_DEBUG1, ("PKT_NEXTIME: Reply\n"));
		ProcessTime(pktP);
	}
	return(0);
}

/* If me > peer, return 1, else return -1
 * error : return 0.
 */
int
RedundsPolarity(int s)
{
	char fn[] = "RedundsPolarity():";
     	int rc, len = sizeof(struct sockaddr_in);
     	struct sockaddr_in peer, me;
	char* errmsg;

	if (getsockname(s, (struct sockaddr *)&me, &len) < 0)
	{
		errmsg = strerror( errno );

		NETERROR(MRED,
			("%s : Local Address not bound...%s\n", fn, errmsg ));
		return 0;
	}

	if (getpeername(s, (struct sockaddr *)&peer, &len) < 0)
	{
		errmsg = strerror( errno );

		NETERROR(MRED,
			("%s Remote Address not bound : %s\n", fn, errmsg ));
		return 0;
	}

	if (me.sin_addr.s_addr < peer.sin_addr.s_addr)
	{
		return -1;
	}

	return 1;
}

int
RedundsSetOptions(int s)
{
	int one = 1;
	char fn[] = "RedundsSetOptions():";
	char* errmsg;

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one)) < 0)
	{
		errmsg = strerror( errno );

		NETERROR(MRED, ("%s Could not set the SO_KEEPALIVE option %s",
				fn, errmsg ));
		return(-1);
	}
	return(0);
}

int
SocksCheckError(int s)
{
	int err = 0, len = sizeof(err);
	char fn[] = "SocksCheckError():";
	char* errmsg;

	if ((getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &len) < 0) ||
		(err < 0))
	{
		errmsg = strerror( errno );

		NETERROR(MRED, ("%s Could not set the SO_ERROR option %s",
				fn, errmsg ));

		return -1;
	}
	
	return 1;
}

int
SyncTime(void)
{
	SendTimeRequest();
	return(0);
}

int
SendTimeRequest()
{
	static Pkt pkt;

	memset(&pkt, 0, sizeof(Pkt));

	pkt.type = (PKT_NEXTIME);
	pkt.totalLen = (sizeof(Pkt));

	pkt.data.nextime.vn = NEXTIME_VERSION & 0x3;
	pkt.data.nextime.mode = NEXTIME_CLIENT & 0x3;

	if (time((time_t *)&pkt.data.nextime.originateTimestamp[0]) ==
		((time_t)-1))
	{
		NETERROR(MRED,
			("Time system call returned error...\n"));
	}

	pkt.data.nextime.clientReference = (timeSetReference);

	htonPkt(PKT_NEXTIME, &pkt);

	if (write(localConfig.redconfig.fd, &pkt, sizeof(pkt)) 
		!= sizeof(pkt))
	{
		NETERROR(MRED,
			("Error in sending Time Req...\n"));
		return -1;
	}

	NETDEBUG(MRED, NETLOG_DEBUG1, ("Time request sent...\n"));
	localConfig.redconfig.timereq = 1;

	return 0;
}

int
ProcessTime(Pkt *pktP)
{
	time_t T1, T2, T3, T4, now;	/* cf rfc 2030 */
	struct tm *tmnow, tmnows;
	unsigned long date, hms, ticks;
	extern char *tzname[2];
	struct timeval offset; 
	unsigned long pol;
	unsigned long old_mode, tmp_mode;

	/* A Time packet has arrived, and we must set 
	 * the time, now
	 */
	if ((pktP->data.nextime.clientReference) != 
			timeSetReference)
	{
		NETDEBUG(MRED, NETLOG_DEBUG1,
			("ignoring time response from peer\n"));

		return 0;
	}

	T1 = (pktP->data.nextime.originateTimestamp[0]);
	T2 = (pktP->data.nextime.receiveTimestamp[0]);
	T3 = (pktP->data.nextime.transmitTimestamp[0]);
	time(&T4);

	now = T3 + ((T4-T1) - (T2-T3))/2;

	offsetSecs = ((T2-T1) + (T3-T4))/2;

	/* offsetSecs is the seconds we are offset from the peer's time */
	NETDEBUG(MRED, NETLOG_DEBUG2,
		("T1 %d T2 %d T3 %d", T1, T2, T3));
	NETDEBUG(MRED, NETLOG_DEBUG2,
		("We are %d seconds offset from peer's time\n", offsetSecs));

	localConfig.redconfig.havetime = 1;
	timeSetReference ++;

	RedundsDoSync();

	return 0;
}

#endif

