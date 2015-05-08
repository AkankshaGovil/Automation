/**************************************************************************
 * FILE: redundscache.c
 *
 * DATE:  MARCH 8th 1998
 *
 * Copyright (c) 1998 Netoids Inc.
 ***************************************************************************/ 	

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

#include <sys/wait.h>

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
#include <malloc.h>

extern int PktLen(char *buf);
extern long offsetSecs;

/* returns TRUE, if his is newer than mine */
#define INFO_NEWER(emine, ehis)	((((emine)->mTime) <= ((ehis)->mTime)) ? 1 :\
					(((emine)->rTime) <= ((ehis)->rTime)))

#define ENTRY_NEWER(emine, ehis) ((((emine)->mTime) <= ((ehis)->mTime)) ? 1 : 0)

int ExamineUpdateCache(LsMemStruct *lsMem, int fd);

int
CleanupIserverCache(unsigned long iserver_addr)
{
     	CacheTableInfo *info;
	CacheVpnEntry *cacheVpnEntry;
	CacheVpnGEntry *cacheVpnGEntry;

	if (iserver_addr == 0)
	{
		return 0;
	}

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("CleanupIserverCache for %s\n", ULIPtostring(htonl(iserver_addr))));

	/* vpn cache */
	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnEntry = (CacheVpnEntry *)CacheGetFirst(vpnCache);
		cacheVpnEntry != NULL; 
		cacheVpnEntry = (CacheVpnEntry *)CacheGetNext(vpnCache, &cacheVpnEntry->vpnEntry))
	{
		if (cacheVpnEntry->iserver_addr == iserver_addr)
		{
			cacheVpnEntry->iserver_addr = 0;
			cacheVpnEntry->state &= ~CACHE_NEEDS_UPDATE;
		}
	}

	CacheReleaseLocks(vpnCache);

	/* Sync VpnG Cache */

	CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnGEntry = (CacheVpnGEntry *)CacheGetFirst(vpnGCache);
		cacheVpnGEntry != NULL; 
		cacheVpnGEntry = (CacheVpnGEntry *)CacheGetNext(vpnGCache, &cacheVpnGEntry->vpnGroupEntry))
	{
		if (cacheVpnGEntry->iserver_addr == iserver_addr)
		{
			cacheVpnGEntry->iserver_addr = 0;
			cacheVpnGEntry->state &= ~CACHE_NEEDS_UPDATE;
		}
	}

	CacheReleaseLocks(vpnGCache);
	
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = (CacheTableInfo *)CacheGetFirst(regCache);
		info != NULL;
		info = (CacheTableInfo *)CacheGetNext(regCache, &info->data))
	{
		if (info->iserver_addr == iserver_addr)
		{
			info->iserver_addr = 0;
			info->state &= ~CACHE_NEEDS_UPDATE;
		}
	}

	CacheReleaseLocks(regCache);
	return(1);
}
     
int
PerformSync(void)
{
	 char fn[] = "PerformSync():";
	 char buffer[4096];
	 NetoidInfoEntry *netInfo;
	 int len = 0, i = 0;
	 CacheTableInfo *info;
	 CacheVpnEntry *cacheVpnEntry;
	 VpnEntry *vpnEntry;
	 VpnGroupEntry *vpnGroupEntry;
	 CacheVpnGEntry *cacheVpnGEntry;
	 CacheVpnRouteEntry *cacheRouteEntry;
	 VpnRouteEntry *routeEntry;
	 CallPlanEntry *callPlanEntry;
	 DB db;
	 DB_tDb dbstruct;
	 CallPlanKey *cpkey, *ocpkey;

	/* Sync Vpn Cache */

	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnEntry = (CacheVpnEntry *)CacheGetFirst(vpnCache);
		cacheVpnEntry != NULL; 
		cacheVpnEntry = (CacheVpnEntry *)CacheGetNext(vpnCache, &cacheVpnEntry->vpnEntry))
	{
		vpnEntry = &cacheVpnEntry->vpnEntry;

		len += AddAdvertisement(TAG_VPN, buffer+len, 
			4096-len, vpnEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED,
					  ("%s length is invalid for vpn sync\n", fn));
		}
	}

	CacheReleaseLocks(vpnCache);

	/* Sync VpnG Cache */

	CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnGEntry = (CacheVpnGEntry *)CacheGetFirst(vpnGCache);
		cacheVpnGEntry != NULL; 
		cacheVpnGEntry = (CacheVpnGEntry *)CacheGetNext(vpnGCache, &cacheVpnGEntry->vpnGroupEntry))
	{
		vpnGroupEntry = &cacheVpnGEntry->vpnGroupEntry;
		len += AddAdvertisement(TAG_VPNG, buffer+len, 
			4096-len, vpnGroupEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED,
					  ("%s length is invalid for vpn sync\n", fn));
		}
	}

	CacheReleaseLocks(vpnGCache);

	/* Sync the CP Cache */
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheRouteEntry = (CacheVpnRouteEntry *)CacheGetFirst(cpCache);
		cacheRouteEntry != NULL; 
		cacheRouteEntry = (CacheVpnRouteEntry *)CacheGetNext(cpCache, &cacheRouteEntry->routeEntry))
	{
		routeEntry = &cacheRouteEntry->routeEntry;

		len += AddAdvertisement(TAG_CR, buffer+len, 
			4096-len, routeEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED,
					  ("%s length is invalid for cr sync\n", fn));
		}
	}

	CacheReleaseLocks(cpCache);

	/* Sync the Calling Plan database */
	/* Too bad, we dont have it in the cache. The only way to sync it up is
	 * to throw all our entries towards the peer and the peer does the same.
	 * Then based on time, we decide which entries to keep from the peer.
	 */
	dbstruct.read_write = GDBM_WRCREAT;
	
	if (db = DbOpen(CALLPLAN_DB_FILE, CALLPLAN_LOCK_FILE, &dbstruct))
	{
		 for (cpkey = (CallPlanKey *)DbGetFirstKey(db);
			  cpkey != 0; 	
			  cpkey = (CallPlanKey *)DbGetNextKey(db,
				   (char *)cpkey, 	
				   sizeof(CallPlanKey)), free(ocpkey), free(callPlanEntry))
         	{	
			  callPlanEntry = (CallPlanEntry *)DbFindEntry(db, (char *)cpkey, sizeof(CallPlanKey));
			  len += AddAdvertisement(TAG_CP, buffer+len, 4096-len, callPlanEntry);

			  if (len > 1500)
			  {
				   /* Send this packet */
				   SendXML(localConfig.redconfig.fd, buffer, len);
				   len = 0;
			  }
			  else if (len < 0)
			  {
				   NETERROR(MRED,
							("%s length is invalid for cr sync\n", fn));
			  }

			ocpkey = cpkey;
		 }

		 DbClose(&dbstruct);
	}
	else
	{
		 NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLPLAN_DB_FILE));
	}

	/* Sync Iedge Cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = (CacheTableInfo *)CacheGetFirst(regCache);
		info != NULL;
		info = (CacheTableInfo *)CacheGetNext(regCache, &info->data))
	{
		len += AddAdvertisement(TAG_IEDGE, buffer+len, 
			4096-len, &info->data);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED,
					  ("%s length is invalid for iedge sync\n", fn));
		}
	}

	CacheReleaseLocks(regCache);

	if (len > 0)
	{
		/* Something is still left */
		SendXML(localConfig.redconfig.fd, buffer, len);
	}

_return:
	Sync(0);

	return 1;
}

CacheVpnEntry *
RedundsDeleteVpn(VpnEntry *vpnEntry)
{
	CacheVpnEntry *cacheVpnEntry;

	cacheVpnEntry = CacheGet(vpnCache, vpnEntry);

	if (cacheVpnEntry)
	{
		if (cacheVpnEntry->vpnEntry.mTime <
			vpnEntry->mTime)
		{
			/* Delete the entry */
			cacheVpnEntry = CacheDelete(vpnCache, vpnEntry);
		}
		else
		{
			cacheVpnEntry = NULL;
		}
	}

	return cacheVpnEntry;
}

CacheVpnGEntry *
RedundsDeleteVpnG(VpnGroupEntry *vpnGroupEntry)
{
	CacheVpnGEntry *cacheVpnGEntry;

	cacheVpnGEntry = CacheGet(vpnCache, vpnGroupEntry);

	if (cacheVpnGEntry)
	{
		if (cacheVpnGEntry->vpnGroupEntry.mTime <
			vpnGroupEntry->mTime)
		{
			/* Delete the entry */
			cacheVpnGEntry = CacheDelete(vpnGCache, vpnGroupEntry);
		}
		else
		{
			cacheVpnGEntry = NULL;
		}
	}

	return cacheVpnGEntry;
}

CacheVpnRouteEntry *
RedundsDeleteCR(VpnRouteEntry *routeEntry)
{
	CacheVpnRouteEntry *cacheRouteEntry;

	cacheRouteEntry = CacheGet(cpCache, routeEntry);

	if (cacheRouteEntry)
	{
		if (cacheRouteEntry->routeEntry.mTime <
			routeEntry->mTime)
		{
			/* Delete the entry */
			cacheRouteEntry = CacheDelete(cpCache, routeEntry);
		}
		else
		{
			cacheRouteEntry = NULL;
		}
	}

	return cacheRouteEntry;
}

int
RedundsHandleVpnCacheUpdate(XMLCacheCb *cb)
{
	 char fn[] = "RedundsHandleVpnCacheUpdate():";
	VpnEntry *vpnEntry = &cb->vpnEntry;
	CacheVpnEntry *cacheVpnEntry;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s Incoming cache update for %s",
			fn, vpnEntry->vpnId));
	
	if (BITA_TEST(cb->tagh, TAG_DELETE))
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete it from the cache */
		cacheVpnEntry = RedundsDeleteVpn(vpnEntry);
		
		CacheReleaseLocks(vpnCache);

		if (cacheVpnEntry)
		{
			SHM_Free(cacheVpnEntry);

			/* Delete it from the database */
			DeleteVpnFromDatabase(vpnEntry);
		}
	}
	return(0);
}

int
RedundsHandleVpnGCacheUpdate(XMLCacheCb *cb)
{
	 char fn[]= "RedundsHandleVpnGCacheUpdate():";
	VpnGroupEntry *vpnGroupEntry = &cb->vpnGroupEntry;
	CacheVpnGEntry *cacheVpnGEntry;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s Incoming cache update for %s",
			fn, vpnGroupEntry->vpnGroup));
    
	if (BITA_TEST(cb->tagh, TAG_DELETE))
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete it from the cache */
		cacheVpnGEntry = RedundsDeleteVpnG(vpnGroupEntry);
		
		CacheReleaseLocks(vpnGCache);

		if (cacheVpnGEntry)
		{
			SHM_Free(cacheVpnGEntry);

			/* Delete it from the database */
			DeleteVpnGFromDatabase(cacheVpnGEntry);
		}
	}
	return(0);
}

int
RedundsHandleCPCacheUpdate(XMLCacheCb *cb)
{
	 char fn[] = "RedundsHandleCPCacheUpdate():";
	 CallPlanEntry *cpentry = &cb->cpEntry;
	 DB db;
	 DB_tDb dbstruct;
	 int rc = 0;
	 CallPlanEntry *callPlanEntry = NULL;

	 NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s Incoming update for %s",
			fn, cpentry->cpname));

	/* We dont have these entries in the cache, we will first look up
	 * the database, and compare the times, and then see if we want 
	 * to overwrite the entries we have
	 */
	dbstruct.read_write = GDBM_WRCREAT;
	
	if (!(db = DbOpen(CALLPLAN_DB_FILE, CALLPLAN_LOCK_FILE, &dbstruct)))
	{
		 NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLPLAN_DB_FILE));
		 return 0;
	}

	if (callPlanEntry = (CallPlanEntry *)DbFindEntry(db,
			(char *)cpentry, sizeof(CallPlanKey)))
	{
		 /* Compare the times and update the entry, if needed */
		 if (ENTRY_NEWER(callPlanEntry, cpentry))
		 {
			  rc = 1;
		 }
  	}
	else
	{
		 /* Entry does not exist, we will not do anything as this is an update */
		 DbClose(&dbstruct); 		 
		 return 0;
	}

	DbClose(&dbstruct);
	
	if (BITA_TEST(cb->tagh, TAG_DELETE))
	{
		/* If there are any call routes in this
		 * calling plan in our cache/db, we must
		 * delete them all also.
		 */
		/* Delete it from the database */
		DeleteCPFromDatabase(cpentry);
	}
	return(rc);
}

int
RedundsHandleCRCacheUpdate(XMLCacheCb *cb)
{
	char fn[] = "RedundsHandleCRCacheUpdate():";
	CacheVpnRouteEntry *cacheRouteEntry;
	VpnRouteEntry *routeEntry = &cb->routeEntry;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s Incoming update for %s",
			fn, routeEntry->crname));
	
	if (BITA_TEST(cb->tagh, TAG_DELETE))
	{
		/* If there are any call routes in this
		 * calling plan in our cache/db, we must
		 * delete them all also.
		 */

		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

		/* Delete it from the cache */
		cacheRouteEntry = RedundsDeleteCR(routeEntry);
		
		CacheReleaseLocks(cpCache);

		if (cacheRouteEntry)
		{
			SHM_Free(cacheRouteEntry);

			/* Delete it from the database */
			DeleteCRFromDatabase(routeEntry);
		}
	}
	return(0);
}

int
RedundsHandleNetoidCacheUpdate(XMLCacheCb *cb)
{
	char fn[] = "RedundsHandleNetoidCacheUpdate():";
	char *sphone = 0;
	int new = 0;
	CacheTableInfo *cacheInfo = 0;
	InfoEntry * entry = 0;
	InfoEntry *infoEntry = &cb->infoEntry, tmpInfo;
	CacheTableInfo *gwCacheInfo;
	int isproxys = 0, isproxyc = 0;
	char xphone[PHONE_NUM_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s Incoming cache update for %s %s",
			fn, infoEntry->phone, infoEntry->vpnPhone));
	
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	/* First see if we have a duplicate of this entry in
	 * our own cache
	 * If we have a duplicate, then we should query the peer
	 * about both the iedge he sent us and the duplicate
	 * we found.
	 */
	cacheInfo = GetDuplicateIedge(infoEntry);

	if (cacheInfo)
	{
		/* We found a duplicate, based on some field.
		 */
		NETERROR(MRED,
			("%s Duplicate Cache Entry found:\n", fn));
		ERROR_PrintInfoEntry(MRED, &cacheInfo->data);
		NETERROR(MRED,
			("%s Incoming Cache Entry:\n", fn));
		ERROR_PrintInfoEntry(MRED, infoEntry);

		goto _return;
	}

	cacheInfo = CacheGet(regCache, infoEntry);

	if (!cacheInfo)
	{
		NETERROR(MRED,
			("%s No cache entry present, treating as ADV\n", fn));

		/* Must handle Proxy entries here, as they may not
		 * be in the cache
		 */

		CacheReleaseLocks(regCache);

		RedundsHandleInfoAdv(infoEntry);

		/* Send a Query */
		RedundsQueryInfoEntry(infoEntry);

		return 0;
	}

	entry = &cacheInfo->data;

#ifdef needed
	TaghPrint(MRED, NETLOG_DEBUG4, cb->tagh, TAGH_LEN);
#endif
		
	/* If update is marked older, disregard it */
	/* Check the refresh times */
	if (!INFO_NEWER(entry, infoEntry))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Update older than our entry. Skipping"));
		goto _return;
	}

	/* First Delete this iedge and take control over the entry */

	cacheInfo = DeleteIedge(entry);
	if (cacheInfo)
	{
		license_release(1);
	}

	/* Now check for deleted entry */
	if (BITA_TEST(cb->tagh, TAG_DELETE))
	{
		int delete = 0;

		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Got an Update which deletes an iedge entry us=%d, them=%d", 
			entry->mTime, infoEntry->mTime));
		
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Delete more recent than our entry. Deleting local"));

		DeleteNetoidFromDatabase(entry);

		if (cacheInfo)
		{
			SHM_Free(cacheInfo);
		}

		goto _return;
	}

	/* Now update the fields and insert it back into the cache */

	if (BITA_TEST(cb->tagh, TAG_ACTIVE))
	{
		if (!(infoEntry->stateFlags & CL_ACTIVE))
		{
			/* The iedge has aged */
			entry->stateFlags &= ~CL_ACTIVE;
			NETDEBUG(MRED, NETLOG_DEBUG3, ("Iedge has unregistered..."));
		}
		else
		{
			entry->stateFlags |= CL_ACTIVE;
			NETDEBUG(MRED, NETLOG_DEBUG3, ("Iedge is active..."));
			cacheInfo->iserver_addr =
				localConfig.redconfig.connEntry.addr.sin_addr.s_addr;
		}
	}
	else if (BITA_TEST(cb->tagh, TAG_AGED))
	{
		cacheInfo->iserver_addr = 0;
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Iedge has aged..."));
	} 
	else if (cacheInfo->iserver_addr != 
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("RedundsHandleNetoidCacheUpdate: Dropping update from 0x%x, entry is owned by 0x%x", localConfig.redconfig.connEntry.addr.sin_addr.s_addr, cacheInfo->iserver_addr));
		
		goto _return;
	}

	if (BITA_TEST(cb->tagh, TAG_ENDPTTYPE))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("DeviceType %d", infoEntry->ispcorgw));
		cacheInfo->data.ispcorgw = infoEntry->ispcorgw;
	}

	if (BITA_TEST(cb->tagh, TAG_PHONE))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Phone Number change"));

		if (BIT_TEST(infoEntry->sflags, ISSET_PHONE) &&
			(strlen(infoEntry->phone) > 0))
		{
			strncpy(cacheInfo->data.phone, infoEntry->phone, 
				PHONE_NUM_LEN);
			cacheInfo->data.phone[PHONE_NUM_LEN-1] = '\0';
			BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);
		}
		else
		{
			BIT_RESET(cacheInfo->data.sflags, ISSET_PHONE);
		}
	}

	if (BITA_TEST(cb->tagh, TAG_VPNPHONE))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Vpn Phone Number change"));
		if (BIT_TEST(infoEntry->sflags, ISSET_VPNPHONE) &&
			(strlen(infoEntry->vpnPhone) > 0))
		{
			strncpy(cacheInfo->data.vpnPhone, infoEntry->vpnPhone, 
				PHONE_NUM_LEN);
			cacheInfo->data.vpnPhone[PHONE_NUM_LEN-1] = '\0';
			cacheInfo->data.vpnExtLen = infoEntry->vpnExtLen;
			BIT_SET(cacheInfo->data.sflags, ISSET_VPNPHONE);
		}
		else
		{
			BIT_RESET(cacheInfo->data.sflags, ISSET_VPNPHONE);
		}
	}

	if (BITA_TEST(cb->tagh, TAG_RTIME))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("RTIME"));
		cacheInfo->data.rTime = infoEntry->rTime;
	}

	if (BITA_TEST(cb->tagh, TAG_MTIME))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("MTIME"));
		cacheInfo->data.mTime = infoEntry->mTime;
	}

	if (BITA_TEST(cb->tagh, TAG_IPADDRESS))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("IPADDR"));
		cacheInfo->data.ipaddress.l = infoEntry->ipaddress.l;
	}

	if (BITA_TEST(cb->tagh, TAG_DND))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("DND"));
		if (infoEntry->stateFlags & CL_DND)
		{
			cacheInfo->data.stateFlags |= CL_DND;
		}
		else
		{
			cacheInfo->data.stateFlags &= ~CL_DND;
		}
	}

	if (BITA_TEST(cb->tagh, TAG_FWDINFO))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Fwd Info change"));

		if (infoEntry->stateFlags & CL_FORWARD)
		{
			cacheInfo->data.stateFlags |= CL_FORWARD;
		}
		else
		{
			cacheInfo->data.stateFlags &= ~CL_FORWARD;
		}

		if (BIT_TEST(infoEntry->nsflags, ISSET_PHONE) &&
			(strlen(infoEntry->nphone) > 0))
		{
			strncpy(cacheInfo->data.nphone, 
				infoEntry->phone, PHONE_NUM_LEN);
			cacheInfo->data.nphone[PHONE_NUM_LEN-1] = '\0';
			BIT_SET(cacheInfo->data.nsflags, ISSET_PHONE);
		}
		if (BIT_TEST(infoEntry->nsflags, ISSET_VPNPHONE) &&
			(strlen(infoEntry->nvpnPhone) > 0))
		{
			strncpy(cacheInfo->data.nvpnPhone, 
				infoEntry->nvpnPhone, PHONE_NUM_LEN);
			cacheInfo->data.nvpnPhone[PHONE_NUM_LEN-1] = '\0';
			BIT_SET(cacheInfo->data.nsflags, ISSET_VPNPHONE);
		}

		cacheInfo->data.protocol = infoEntry->protocol;
	}

	isproxys = cacheInfo->data.stateFlags & CL_PROXY;
	if (BITA_TEST(cb->tagh, TAG_PROXYS))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Proxy Server"));

		if (infoEntry->stateFlags & CL_PROXY)
		{
			cacheInfo->data.stateFlags |= CL_PROXY;
		}
		else
		{
			cacheInfo->data.stateFlags &= ~CL_PROXY;
		}
	}

	isproxyc = ((cacheInfo->data.stateFlags & (CL_PROXYING|CL_PROXY)) ==
			CL_PROXYING);

	if (BITA_TEST(cb->tagh, TAG_PROXYC))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Proxy Client"));
		if (infoEntry->stateFlags & CL_PROXYING)
		{
			cacheInfo->data.stateFlags |= CL_PROXYING;
		}
		else
		{
			cacheInfo->data.stateFlags &= ~CL_PROXYING;
		}
	}

#if 0
	if (BITA_TEST(cb->tagh, TAG_PROXYINFO))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Proxy Info"));
		
		strncpy(cacheInfo->data.xphone, infoEntry->xphone, 
			PHONE_NUM_LEN);
		cacheInfo->data.xphone[PHONE_NUM_LEN-1] = '\0';
		strncpy(cacheInfo->data.xvpnPhone, infoEntry->xvpnPhone, 
			VPN_LEN);
		cacheInfo->data.xvpnPhone[VPN_LEN-1] = '\0';
	}

	if (isproxys && !(cacheInfo->data.stateFlags & CL_PROXY))
	{
		/* Device has disabled proxy */
		HandleProxyDisable(cacheInfo);
	}
#endif

#if 0
	/* Check all the comments/location stuff */
	if (BITA_TEST(cb->tagh, TAG_FNAME))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("First Name"));
		strncpy(cacheInfo->data.clFname, infoEntry->clFname,
			CLIENT_ATTR_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_LNAME))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Last Name"));
		strncpy(cacheInfo->data.clLname, infoEntry->clLname,
			CLIENT_ATTR_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_LOCATION))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Location"));
		strncpy(cacheInfo->data.clLoc, infoEntry->clLoc,
			CLIENT_ATTR_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_COUNTRY))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Country"));
		strncpy(cacheInfo->data.clCountry, infoEntry->clCountry,
			CLIENT_ATTR_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_COMMENTS))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("Comments"));
		strncpy(cacheInfo->data.clComments, infoEntry->clComments,
			CLIENT_ATTR_LEN-1);
	}
#endif

	if (BITA_TEST(cb->tagh, TAG_ZONE))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("zone"));
		strncpy(cacheInfo->data.zone, infoEntry->zone,
			ZONE_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_EMAIL))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("email"));
		strncpy(cacheInfo->data.email, infoEntry->email,
			EMAIL_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_CPNAME))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("cpname"));
		strncpy(cacheInfo->data.cpname, infoEntry->cpname,
			CALLPLAN_ATTR_LEN-1);
	}

	if (BITA_TEST(cb->tagh, TAG_RASIP))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("rasip"));
		cacheInfo->data.rasip = infoEntry->rasip;
	}

	if (BITA_TEST(cb->tagh, TAG_RASPT))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("rasport"));
		cacheInfo->data.rasport = infoEntry->rasport;
	}

	if (BITA_TEST(cb->tagh, TAG_H323SIGPT))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("callsigport"));
		cacheInfo->data.callsigport = infoEntry->callsigport;
	}

	if (license_allocate(1))
	{
		NETERROR(MRED, ("%s No License available\n", fn));
		SHM_Free(cacheInfo);
		goto _return;
	}

	/* At this point, we can commit this entry into the cache */
	if (AddIedge(cacheInfo) < 0)
	{
		/* We found a duplicate */
		NETERROR(MRED,
			("%s Found a duplicate in cache for:\n", fn));

		ERROR_PrintInfoEntry(MCACHE, &cacheInfo->data);
		SHM_Free(cacheInfo);

		goto _return;
	}

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleNetoidCacheUpdate: Updating Netoid Database"));

	/* Put this entry into the database */
	UpdateNetoidDatabase(&tmpInfo);

#if 0
	if (isproxyc && !(tmpInfo.stateFlags & CL_PROXYING))
	{
		/* Device is no longer proxying */
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Device no longer proxying for %s\n", xphone));

		UnregisterProxy(&tmpInfo);
	} 
	else if (!isproxyc && (tmpInfo.stateFlags & CL_PROXYING))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Device proxying for %s\n", xphone));

		/* Here we must do what proxy registration does */
		HandleProxyRegistration(&tmpInfo);
	}
#endif

_return:
	CacheReleaseLocks(regCache);

	return 0;
}

int
SlaveQuery(void)
{
	 char fn[] = "SlaveQuery():";
	 char buffer[4096];
	 NetoidInfoEntry *netInfo;
	 int len = 0;
	 CacheTableInfo *info;
	 int i=0;

	NETDEBUG(MRED, NETLOG_DEBUG3, ("SlaveQuery: Entering\n"));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = (CacheTableInfo *)CacheGetFirst(regCache);
		info != NULL;
		info = (CacheTableInfo *)CacheGetNext(regCache, &info->data))
	{
		if (info->iserver_addr !=
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
		{
			continue;	
		}

		len += AddQuery(TAG_IEDGE, buffer+len, 
			4096-len, &info->data);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED, ("%s length is invalid for iedge query\n", fn));
		}
	}

	CacheReleaseLocks(regCache);

	if (len > 0)
	{
		/* Something is still left */
		SendXML(localConfig.redconfig.fd, buffer, len);
	}

_return:
     	return 1;
} 

int
SlaveSync(void)
{
#if 0
	 char fn[] = "SlaveSync():";
	 char buffer[4096];
	 NetoidInfoEntry *netInfo;
	 int len = 0, i=0;
	 CacheTableInfo *info;
	 Avlnode *vpnList, *vpnGList;
	 VpnGroupEntry *vpnGroupEntry;
	 CacheVpnGEntry *cacheVpnGEntry;
	 CacheVpnEntry *cacheVpnEntry;
	 VpnEntry *vpnEntry;
	 CacheVpnRouteEntry *cacheRouteEntry;
	 VpnRouteEntry *routeEntry;
	 CallPlanEntry *callPlanEntry;
	 DB db;
	 DB_tDb dbstruct;
	 CallPlanKey *cpkey, *ocpkey;

	NETDEBUG(MRED, NETLOG_DEBUG3, ("SlaveSync: Entering\n"));

	Sync(1);

	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnEntry = (CacheVpnEntry *)CacheGetFirst(vpnCache);
		cacheVpnEntry != NULL;
		cacheVpnEntry = (CacheVpnEntry *)CacheGetNext(vpnCache, &cacheVpnEntry->vpnEntry))
	{
		if (cacheVpnEntry->iserver_addr ==
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
		{
			continue;	
		}

		vpnEntry = &cacheVpnEntry->vpnEntry;

		len += AddAdvertisement(TAG_VPN, buffer+len, 
			4096-len, vpnEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED, ("%s length is invalid for vpn query\n", fn));
		}
	}

	CacheReleaseLocks(vpnCache);

	CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheVpnGEntry = (CacheVpnGEntry *)CacheGetFirst(vpnGCache);
		cacheVpnGEntry != NULL;
		cacheVpnGEntry = (CacheVpnGEntry *)CacheGetNext(vpnGCache, &cacheVpnGEntry->vpnGroupEntry))
	{
		if (cacheVpnGEntry->iserver_addr ==
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
		{
			continue;	
		}

		vpnGroupEntry = &cacheVpnGEntry->vpnGroupEntry;
		len += AddAdvertisement(TAG_VPNG, buffer+len, 
			4096-len, vpnGroupEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED, ("%s length is invalid for vpng query\n", fn));
		}
	}

	CacheReleaseLocks(vpnGCache);

	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheRouteEntry = (CacheVpnRouteEntry *)CacheGetFirst(cpCache);
		cacheRouteEntry != NULL; 
		cacheRouteEntry = (CacheVpnRouteEntry *)CacheGetNext(cpCache, &cacheRouteEntry->routeEntry))
	{
		if (cacheRouteEntry->iserver_addr ==
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
		{
			continue;	
		}

		routeEntry = &cacheRouteEntry->routeEntry;

		len += AddAdvertisement(TAG_CR, buffer+len, 
			4096-len, routeEntry);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED, ("%s length is invalid for cr query\n", fn));
		}
	}

	CacheReleaseLocks(cpCache);

	dbstruct.read_write = GDBM_WRCREAT;
	
	if (db = DbOpen(CALLPLAN_DB_FILE, CALLPLAN_LOCK_FILE, &dbstruct))
	{
		 for (cpkey = (CallPlanKey *)DbGetFirstKey(db);
			  cpkey != 0; 	
			  cpkey = (CallPlanKey *)DbGetNextKey(db,
				   (char *)cpkey, 	
				   sizeof(CallPlanKey)), free(ocpkey), free(callPlanEntry))
         {	
			  callPlanEntry = (CallPlanEntry *)DbFindEntry(db, (char *)cpkey, sizeof(CallPlanKey));
			  len += AddAdvertisement(TAG_CP, buffer+len, 4096-len, callPlanEntry);

			  if (len > 1500)
			  {
				   /* Send this packet */
				   SendXML(localConfig.redconfig.fd, buffer, len);
				   len = 0;
			  }
			  else if (len < 0)
			  {
				   NETERROR(MRED,
							("%s length is invalid for cr sync\n", fn));
			  }
		 }

		 DbClose(&dbstruct);
	}
	else
	{
		 NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLPLAN_DB_FILE));
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (info = (CacheTableInfo *)CacheGetFirst(regCache);
		info != NULL;
		info = (CacheTableInfo *)CacheGetNext(regCache, &info->data))
	{
		if (info->iserver_addr ==
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr)
		{
			continue;	
		}

		len += AddEntry(TAG_IEDGE, buffer+len, 
			4096-len, &info->data);

		if (len > 1500)
		{
			/* Send this packet */
			SendXML(localConfig.redconfig.fd, buffer, len);
			len = 0;
		}
		else if (len < 0)
		{
			 NETERROR(MRED, ("%s length is invalid for iedge query\n", fn));
		}
	}

	CacheReleaseLocks(regCache);

	if (len > 0)
	{
		/* Something is still left */
		SendXML(localConfig.redconfig.fd, buffer, len);
	}

_return:
	Sync(0);

	localConfig.redconfig.state = PEERSTATE_UPDT;

     	return 1;
#endif
	return(1);
}

int
RedundsHandleQuery(InfoEntry *infoEntry)
{
	 char fn[] = "RedundsHandleQuery():";
	 char *sphone = 0;
	 int new = 0;
	 CacheTableInfo *cacheInfo = 0;
	 InfoEntry * entry = 0;
	 char buffer[4096];
	 int len = 0;

	/* Someone has queried for an iedge. Check the proxy
	 * flags, so that we return the right entry
	 */
	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleQuery %s %s\n",
			infoEntry->phone, infoEntry->vpnPhone));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheInfo = CacheGet(regCache, infoEntry);

	if (!cacheInfo)
	{
		/* If it doesnt exist, 
		 * dont do anything for now.
	 	*/
		
		NETERROR(MRED,
			("RedundsHandleQuery: Peer asked for entry which I dont have"));

		CacheReleaseLocks(regCache);

		return -1;
	}

	/* We have the entry, reply with a cache entry */

	len += AddEntry(TAG_IEDGE, buffer+len, 
		4096-len, &cacheInfo->data);

	CacheReleaseLocks(regCache);

	SendXML(localConfig.redconfig.fd, buffer, len);
	return(0);
}

int
RedundsHandleNetoidCacheEntry(InfoEntry *infoEntry)
{
	char fn[] = "RedundsHandleNetoidCacheEntry():";
	char *sphone = 0;
	int new = 0;
	CacheTableInfo *cacheInfo = 0;
	InfoEntry * entry = 0, tmpInfo;
	int isproxys = 0, isproxyc = 0;
	int wasGateway = 0;
	char xphone[PHONE_NUM_LEN] = { 0 };

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleNetoidCacheEntry %s %d\n", 
		infoEntry->regid, infoEntry->uport));

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleNetoidCacheEntry phone %s vpn phone %s\n", 
		infoEntry->phone, infoEntry->vpnPhone));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	/* First see if we have a duplicate of this entry in
	 * our own cache
	 * If we have a duplicate, then we should query the peer
	 * about both the iedge he sent us and the duplicate
	 * we found.
	 */
	cacheInfo = GetDuplicateIedge(infoEntry);

	if (cacheInfo)
	{
		/* We found a duplicate, based on some field.
		 */
		NETERROR(MRED,
			("%s Duplicate Cache Entry found:\n", fn));
		ERROR_PrintInfoEntry(MRED, &cacheInfo->data);
		NETERROR(MRED,
			("%s Incoming Cache Entry:\n", fn));
		ERROR_PrintInfoEntry(MRED, infoEntry);

		goto _return;
	}

	cacheInfo = CacheGet(regCache, infoEntry);

	/* If update is marked older, disregard it */
	/* Check the refresh times */
	if (cacheInfo && !INFO_NEWER(&cacheInfo->data, infoEntry))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Update older than our entry. Skipping"));
		goto _return;
	}

	if (!cacheInfo)
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("%s No cacheInfo\n", fn));

		cacheInfo = CacheDupInfoEntry(infoEntry);

		goto _insert_cache;
	}

	/* We have the entry here, first we will delete it */
	cacheInfo = DeleteIedge(&cacheInfo->data);

	if (cacheInfo)
	{
		CacheTableEntry *cacheHead;

		/* This is a complete entry coming in. Not an update.
		 * If there is no phone, it means that the phone was
		 * erased. Delete the iedge from cache.
		 * However, it goes into the database.
		 */
		isproxys = cacheInfo->data.stateFlags & CL_PROXY;
		isproxyc = ((cacheInfo->data.stateFlags & (CL_PROXYING|CL_PROXY)) ==
				CL_PROXYING);

		memcpy(&cacheInfo->data, infoEntry, sizeof(InfoEntry));

		memcpy(&tmpInfo, infoEntry, sizeof(tmpInfo));

		cacheInfo->iserver_addr = 
			localConfig.redconfig.connEntry.addr.sin_addr.s_addr;

		cacheInfo->state &= ~CACHE_NEEDS_UPDATE;

		/* Release the license */
		license_release(1);
	}
	else
	{
		NETERROR(MRED, ("%s Cache Entry cannot be found\n", fn));
		goto _return;
	}

_insert_cache:

	/* Allocate a licence for the iedge */
	if (license_allocate(1))
	{
		NETERROR(MRED, ("%s No License available\n", fn));
		SHM_Free(cacheInfo);
		goto _return;
	}

	if (AddIedge(cacheInfo) < 0)
	{
		/* We found a duplicate */
		NETERROR(MRED,
			("%s Found a duplicate in cache for:\n", fn));

		ERROR_PrintInfoEntry(MCACHE, &cacheInfo->data);
		SHM_Free(cacheInfo);

		goto _return;
	}

	memcpy(&tmpInfo, &cacheInfo->data, sizeof(tmpInfo));

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleNetoidCacheEntry: Updating Netoid Database"));

#if 0
	if (isproxyc && !(tmpInfo.stateFlags & CL_PROXYING))
	{
		/* Device is no longer proxying */
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Device no longer proxying for %s\n", xphone));
		UnregisterProxy(&tmpInfo);
	} 
	else if (!isproxyc && (tmpInfo.stateFlags & CL_PROXYING))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("Device proxying for %s\n", xphone));
		/* Here we must do what proxy registration does */
		HandleProxyRegistration(&tmpInfo);
	}
#endif

	/* Put this entry into the database */
	UpdateNetoidDatabase(&tmpInfo);

_return:
	CacheReleaseLocks(regCache);

	return 0;
}

int
RedundsHandleInfoAdv(InfoEntry *infoEntry)
{
	char fn[] = "RedundsHandleInfoAdv():";
	char *sphone = 0;
	int new = 0;
	CacheTableInfo *cacheInfo = 0;
	InfoEntry * entry = 0;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("RedundsHandleInfoAdv %s %s\n",
			infoEntry->phone, infoEntry->vpnPhone));

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	/* First see if we have a duplicate of this entry in
	 * our own cache
	 * If we have a duplicate, then we should query the peer
	 * about both the iedge he sent us and the duplicate
	 * we found.
	 */
	cacheInfo = GetDuplicateIedge(infoEntry);

	if (cacheInfo)
	{
		/* We found a duplicate, based on some field.
		 */
		NETERROR(MRED,
			("%s Duplicate Cache Entry found:\n", fn));
		ERROR_PrintInfoEntry(MRED, &cacheInfo->data);
		NETERROR(MRED,
			("%s Incoming Cache Entry:\n", fn));
		ERROR_PrintInfoEntry(MRED, infoEntry);

		/* Mark this entry for an update anyway */
		cacheInfo->state |= CACHE_NEEDS_UPDATE;

		goto _return;
	}

	cacheInfo = CacheGet(regCache, infoEntry);

	/* If update is marked older, disregard it */
	/* Check the refresh times */
	if (cacheInfo && !INFO_NEWER(&cacheInfo->data, infoEntry))
	{
		NETDEBUG(MRED, NETLOG_DEBUG3,
			("%s Adv older than our entry. Skipping", fn));
		goto _return;
	}

	if (!cacheInfo)
	{
		NETDEBUG(MRED, NETLOG_DEBUG3, ("%s No cacheInfo\n", fn));

		cacheInfo = CacheDupInfoEntry(infoEntry);

		if (cacheInfo == NULL)
		{
			NETERROR(MRED, ("%s No Memory\n", fn));
			goto _return;
		}

		/* Insert the entry just in the reg cache */
		CacheInsert(regCache, cacheInfo);
	}

	/* Master will own the entry */
	cacheInfo->iserver_addr = 
		localConfig.redconfig.connEntry.addr.sin_addr.s_addr;

	cacheInfo->state |= CACHE_NEEDS_UPDATE;

_return:
	CacheReleaseLocks(regCache);
	return(0);
}

int
RedundsHandleVpnAdv(VpnEntry *vpnEntry)
{
	char fn[] = "RedundsHandleVpnAdv():";
	DB db;
	DB_tDb dbstruct;
	int rc = 0;
	char oldVpnGroup[VPN_GROUP_LEN], newVpnGroup[VPN_GROUP_LEN];
	CacheVpnEntry *cacheVpnEntry;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		  ("RedundsHandleVpnAdv %s %d %s\n", 
			vpnEntry->vpnId, (vpnEntry->vpnExtLen),
			vpnEntry->vpnGroup));

	/* Sync Vpn Cache */
	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

	cacheVpnEntry = 
		 (CacheVpnEntry *)CacheGet(vpnCache, vpnEntry);

	if (cacheVpnEntry == NULL)
	{
		 /* Insert the entry into the cache */
		 cacheVpnEntry = CacheDupVpnEntry(vpnEntry);
		 if (cacheVpnEntry == NULL)
		 {
			  NETERROR(MSHM, ("%s Out of memory\n", fn));
			  goto _release;
		 }

		 CacheInsert(vpnCache, cacheVpnEntry);
		 rc = 1;
	} 
	else if (ENTRY_NEWER(&cacheVpnEntry->vpnEntry, vpnEntry))
	{
		 cacheVpnEntry->iserver_addr = 
			  localConfig.redconfig.connEntry.addr.sin_addr.s_addr;
		 cacheVpnEntry->state &= ~CACHE_NEEDS_UPDATE;

		 /* We will store this entry in the database */
		 MemAgeIedgesInVpnGs(oldVpnGroup, newVpnGroup);
		 
		 rc = 1;
	}
	else
	{
		 NETDEBUG(MRED, NETLOG_DEBUG3,
			   ("RedundsHandleVpnAdv: Adv discarded"));
	}

_release:
	CacheReleaseLocks(vpnCache);

	if (rc == 1)
	{
		 dbstruct.read_write = GDBM_WRCREAT;

		 if (!(db = DbOpen(VPNS_DB_FILE, VPNS_LOCK_FILE, &dbstruct)))
		 {
			  NETERROR(MDB, ("%s Unable to open database %s\n", fn, VPNS_DB_FILE));
			  return 0;
		 }

		 strcpy(newVpnGroup, vpnEntry->vpnGroup);
		 rc = StoreVpn(db, vpnEntry, oldVpnGroup, newVpnGroup);
	
		 DbClose(&dbstruct);
	}

	return rc;
}

int
RedundsHandleVpnGAdv(VpnGroupEntry *vpnGroupEntry)
{
    DB db;
	DB_tDb dbstruct;
	int rc = 0;
	CacheVpnGEntry *cacheVpnGEntry;
	char fn[] = "RedundsHandleVpnGAdv():";

	NETDEBUG(MRED, NETLOG_DEBUG3,
		("%s : entered, vpngroup %s\n", fn, vpnGroupEntry->vpnGroup));

	/* Sync VpnG Cache */
	CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);
	
	cacheVpnGEntry = (CacheVpnGEntry *)CacheGet(vpnGCache, vpnGroupEntry);
		
	if (cacheVpnGEntry == NULL)
	{
		cacheVpnGEntry = CacheDupVpnGEntry(vpnGroupEntry);

		if (cacheVpnGEntry)
		{
			CacheInsert(vpnGCache, cacheVpnGEntry);
		}
		else
		{
			 goto _release;
		}
		rc = 1;
	}
	else if (ENTRY_NEWER(&cacheVpnGEntry->vpnGroupEntry, vpnGroupEntry))
	{
		 /* Mark this as belonging to the redundant iserver */ 
		 cacheVpnGEntry->iserver_addr = 
			  localConfig.redconfig.connEntry.addr.sin_addr.s_addr;
		 cacheVpnGEntry->state &= ~CACHE_NEEDS_UPDATE;
		 rc = 1;
	}
	else
	{
		 NETDEBUG(MRED, NETLOG_DEBUG3,
			   ("RedundsHandleVpnGAdv: Adv discarded"));
	}

_release:
	CacheReleaseLocks(vpnGCache);

	if (rc > 0)
	{
		 dbstruct.read_write = GDBM_WRCREAT;

		 if (!(db = DbOpen(VPNG_DB_FILE, VPNG_LOCK_FILE, &dbstruct)))
		 {
			  NETERROR(MDB, ("%s Unable to open database %s\n",
						fn, VPNG_DB_FILE));
			  return 0;
		 }

		 rc = StoreVpnG(db, vpnGroupEntry);

		 DbClose(&dbstruct);
	}

	return 0;
}

int
RedundsHandleCPAdv(CallPlanEntry *cpentry)
{
	 char fn[] = "RedundsHandleCPAdv():";
	 DB db;
	 DB_tDb dbstruct;
	 int rc = 0;
	 CallPlanEntry *callPlanEntry = NULL;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		  ("%s %s\n", 
			fn, cpentry->cpname));

	/* We dont have these entries in the cache, we will first look up
	 * the database, and compare the times, and then see if we want 
	 * to overwrite the entries we have
	 */
	dbstruct.read_write = GDBM_WRCREAT;
	
	if (!(db = DbOpen(CALLPLAN_DB_FILE, CALLPLAN_LOCK_FILE, &dbstruct)))
	{
		 NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLPLAN_DB_FILE));
		 return 0;
	}

	if (callPlanEntry = (CallPlanEntry *)DbFindEntry(db,
			(char *)cpentry, sizeof(CallPlanKey)))
	{
		 /* Compare the times and update the entry, if needed */
		 if (ENTRY_NEWER(callPlanEntry, cpentry))
		 {
			  rc = 1;
		 }
		 else
		 {
			  NETDEBUG(MRED, NETLOG_DEBUG3,
					("Update older than our entry. Skipping"));
		 }
  	}
	else
	{
		 /* Entry does not exist, we can insert it */
		 rc = 1;
	}

	DbClose(&dbstruct);

	if (rc == 1)
	{
		 dbstruct.read_write = GDBM_WRCREAT;

		 if (!(db = DbOpen(CALLPLAN_DB_FILE, CALLPLAN_LOCK_FILE, &dbstruct)))
		 {
			  NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLPLAN_DB_FILE));
			  return 0;
		 }

		/* Store the entry */
		 if (DbStoreEntry(db,
						  (char *)cpentry, sizeof(CallPlanEntry), 
						  (char *)cpentry, sizeof(CallPlanKey)) < 0)
		 {
			  NETERROR(MDB, ("%s database store error %d\n", fn, errno));
		 }

		DbClose(&dbstruct);
	}

	return rc;
}

int
RedundsHandleCRAdv(VpnRouteEntry *routeEntry)
{
	 char fn[] = "RedundsHandleCRAdv():";
	 DB db;
	 DB_tDb dbstruct;
	 int rc = 0;
	 CacheVpnRouteEntry *cacheRouteEntry;

	NETDEBUG(MRED, NETLOG_DEBUG3,
		  ("%s %s/%s/%s\n", 
			fn, routeEntry->crname, routeEntry->dest, routeEntry->prefix));

	/* Sync CP Cache */
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	cacheRouteEntry = 
		 (CacheVpnRouteEntry *)CacheGet(cpCache, routeEntry);

	if (cacheRouteEntry == NULL)
	{
		 /* Insert the entry into the cache */
		 cacheRouteEntry = CacheDupVpnRouteEntry(routeEntry);
		 if (cacheRouteEntry == NULL)
		 {
			  NETERROR(MSHM, ("%s Out of memory\n", fn));
			  goto _release;
		 }

		 CacheInsert(cpCache, cacheRouteEntry);
		 rc = 1;
	} 
	else if (ENTRY_NEWER(&cacheRouteEntry->routeEntry, routeEntry))
	{
		 cacheRouteEntry->iserver_addr = 
			  localConfig.redconfig.connEntry.addr.sin_addr.s_addr;

		 cacheRouteEntry->state &= ~CACHE_NEEDS_UPDATE;

		 rc = 1;
	}
	else
	{
		 NETDEBUG(MRED, NETLOG_DEBUG3,
			   ("%s Adv discarded", fn));
	}

_release:
	CacheReleaseLocks(cpCache);

	if (rc == 1)
	{
		 dbstruct.read_write = GDBM_WRCREAT;

		 if (!(db = DbOpen(CALLROUTE_DB_FILE, CALLROUTE_LOCK_FILE, &dbstruct)))
		 {
			  NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLROUTE_DB_FILE));
			  return 0;
		 }

		/* Store the entry */
		 if (DbStoreEntry(db,
						  (char *)routeEntry, sizeof(VpnRouteEntry), 
						  (char *)routeEntry, sizeof(VpnRouteKey)) < 0)
		 {
			  NETERROR(MDB, ("%s database store error %d\n", fn, errno));
			  rc = 0;
		 }

		DbClose(&dbstruct);
	}

	return rc;
}



#endif
