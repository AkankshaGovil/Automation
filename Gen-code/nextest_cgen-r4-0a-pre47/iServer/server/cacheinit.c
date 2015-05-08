/*
 * Functions which involve both the db management
 * and shared memory
 */

#include <syslog.h>
#include <errno.h>
#include <stdio.h>

#include "alerror.h"
#include "ipc.h"
#include "bits.h"
#include "mem.h"
#include "db.h"
#include "srvrlog.h"
#include "serverp.h"
#include "license.h"
#include "timer.h"
#include "gis.h"
#undef malloc
#undef calloc
#undef free
#include <malloc.h>
#include "cacheinit.h"
#include "log.h"
#include "dbs.h"

CacheEntry *updateList;
cache_t regCache, regidCache, phoneCache, vpnPhoneCache, emailCache, 
	gwCache, ipCache, gkCache,
	vpnCache, vpnGCache, cpCache, callCache, confCache, cpbCache, 
	uriCache, h323idCache, transCache, sipCallCache, cpdestCache, 
	cpbcrCache, gwcpCache, cpbcpCache, subnetCache, cpsrcCache, cridCache,
	transDestCache, tgCache, guidCache, triggerCache, cptransitCache, lruRoutesCache, cporigDNISCache, cporigANICache,
	realmCache, rsaCache, tipCache, igrpCache, rsapubnetsCache, sipregCache, dtgCache, vnetCache;

// Also protected by the realm Cache locks
CacheRealmEntry **defaultRealm = NULL;

data2keyfn CacheData2KeyArray[] = 
{
	NULL,
	CacheIedgePhoneData2Key,
	CacheCPData2Key,
	CacheCPBCRData2Key,
	CacheIedgeGwCPData2Key,
	CacheCPBCPData2Key,
};

data2keyfn CacheInsData2KeyArray[] = 
{
	NULL,
	CacheIedgePhoneInsData2Key,
	CacheCPInsData2Key,
	CacheCPBCRInsData2Key,
	CacheIedgeGwCPInsData2Key,
	CacheCPBCPInsData2Key,
	CacheCPSrcInsData2Key,
};

avl_comparison_func
CacheinscmpArray[] = 
{
	NULL,
	CacheIedgeRegInsCmp,
	CacheIedgeRegidInsCmp,
	CacheIedgePhoneInsCmp,
	CacheIedgeVpnPhoneInsCmp,
	CacheIedgeEmailInsCmp,
	CacheIedgeIpInsCmp,
	CacheVpnCmp,
	CacheVpnGCmp,
	CacheCPInsCmp,
	CacheCallInsCmp,
	CacheIedgeUriInsCmp,
	CacheConfInsCmp,
	CacheCPBInsCmp,
	CacheIedgeGkCmp,
	CacheIedgeH323IdInsCmp,
	CacheTransInsCmp,
	CacheSipCallInsCmp,
	timerCompare,
	timerHandleInsCompare,
	CacheIedgeSubnetInsCmp,
	0,	// For DLIC
	CacheIedgeCrIdInsCmp,
	CacheTransDestInsCmp,
	CacheIedgeTGInsCmp,
	CacheGuidInsCmp,
	CacheTriggerInsCmp,
	CacheRoutesLRUInsCmp,
	CacheRealmInsCmp,
	CacheRsaInsCmp,
	CacheTipInsCmp,
	CacheIgrpInsCmp,
	CacheRsapubnetsInsCmp,
	CacheSipRegInsCmp,
	0,	// For SCM
	CacheVnetInsCmp,
	NULL,
};

avl_comparison_func
CachecmpArray[] = 
{
	NULL,
	CacheIedgeRegCmp,
	CacheIedgeRegidCmp,
	CacheIedgePhoneCmp,
	CacheIedgeVpnPhoneCmp,
	CacheIedgeEmailCmp,
	CacheIedgeIpCmp,
	CacheVpnCmp,
	CacheVpnGCmp,
	CacheCPCmp,
	CacheCallCmp,
	CacheIedgeUriCmp,
	CacheConfCmp,
	CacheCPBCmp,
	CacheIedgeGkCmp,
	CacheIedgeH323IdCmp,
	CacheTransCmp,
	CacheSipCallCmp,
	timerCompare,
	timerHandleCompare,
	CacheIedgeSubnetCmp,
	0,	// For DLIC
	CacheIedgeCrIdCmp,
	CacheTransDestCmp,
	CacheIedgeTGCmp,
	CacheGuidCmp,
	CacheTriggerCmp,
	CacheRealmCmp,
    CacheRsaCmp,
    CacheTipCmp,
	CacheIgrpCmp,
    CacheRsapubnetsCmp,
	CacheSipRegCmp,
	0,	// For SCM
	CacheVnetCmp,
	NULL,
};

/*
avl_comparison_func
CachedupArray[] = 
{
	NULL,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
	avl_dup_error,
};
*/

CacheCallbacks CachePreCbArray[] = 
{
	NULL,
	CacheIedgeCachePreCb,		
	CacheVpnCachePreCb,
	CacheVpnGCachePreCb,	
	TimerPreCbPrint,
	TimerHandlePreCbPrint,
	CacheCallCachePreCb,
};

CacheCallbacks CachePostCbArray[] = 
{
	NULL,
	CacheIedgeCachePostCb,		
	CacheVpnCachePostCb,
	CacheVpnGCachePostCb,	
	TimerPostCbPrint,
	TimerHandlePostCbPrint,
	LruRoutesPostCbCheck,
};

/* Populate all of the iedge caches, simultaneously */
int
IedgeCachePopulate(LsMemStruct *lsMem, DB dbin, unsigned short flags)
{
	char fn[] = "IedgeCachePopulate():";
	DB db;
	DB_tDb dbstruct = { 0 };
	NetoidInfoEntry *info;
	NetoidSNKey *key, *okey;
	int keylen = sizeof(NetoidSNKey);
	int no = 0, all = 0, duplicates = 0;
	CacheTableInfo *cacheInfo, *dupInfo;

	if (lsMem == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		dbstruct.read_write = GDBM_WRCREAT;
		if (!(db = DbOpenByID(NETOIDS_DB_FILE, DB_eNetoids, &dbstruct)))
		{
			NETERROR(MCACHE, ("%s Unable to open database %s\n", 
							fn, NETOIDS_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

	/* Acquire locks on the cache */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	// Check to see if a cache exists, if it does, we must
	// first update the db with runtime information from the
	// cache - like ncalls etc.
	if (flags & CACHE_SAVE_DYNAMIC)
	{
		for (cacheInfo = CacheGetFirst(regCache);
			cacheInfo; 
			cacheInfo = CacheGetNext(regCache, &cacheInfo->data))
		{
			/* Get the info entry */
			info = DbFindInfoEntry(db, (char *)&cacheInfo->data, keylen);
			if (info)
			{
				UpdateIedgeDynamicInfo(info, &cacheInfo->data);

	 			if (DbStoreInfoEntry(db, info, 
			      (char *)info, sizeof(NetoidSNKey)) < 0)
	 			{
	      			NETERROR(MINIT, ("database store error\n"));
	 			}
			}
		}
	}
	
	if (flags & CACHE_PURGE_OLD)
	{
		IedgeCacheDestroyData(lsMem);
		IedgeCacheInstantiate(lsMem);
	}

	for (key = (NetoidSNKey *)DbGetFirstInfoKey(db); key != 0; 
		  key = (NetoidSNKey *)DbGetNextInfoKey(db, (char *)key, keylen),
			   free(okey), free(info))
	{
		/* Get the info entry */
		info = DbFindInfoEntry(db, (char *)key, keylen);

		if (info == NULL)
		{
			   NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			   goto _return;
		}

		okey = key;
	
		all ++;

		if (!BIT_TEST(info->sflags, ISSET_PHONE) &&
		      !BIT_TEST(info->sflags, ISSET_EMAIL) &&
			  !BIT_TEST(info->sflags, ISSET_VPNPHONE) &&
			  !BIT_TEST(info->sflags, ISSET_IPADDRESS) &&
			  !BIT_TEST(info->sflags, ISSET_REGID) &&
			  !BIT_TEST(info->sflags, ISSET_UPORT))
		{ 
			   /* We dont need this entry ... */
			   NETDEBUG(MCACHE, NETLOG_DEBUG4,
						("%s Skipping Entry %d:\n", fn, all));
			   DEBUG_PrintInfoEntry(MCACHE, NETLOG_DEBUG4, info);
			   continue;
		}

		if (!(flags & CACHE_POPULATE_ALL) && !(CLIENT_GRADUATE(info)))
		{
			   /* We dont need this entry ... */
			   NETDEBUG(MCACHE, NETLOG_DEBUG4,
						("%s Skipping Entry (Inactive)%d:\n", fn, all));
			   DEBUG_PrintInfoEntry(MCACHE, NETLOG_DEBUG4, info);
			   continue;
		}

		if (dupInfo = GetDuplicateIedge(info))
		{
			NETERROR(MCACHE,
				("%s Duplicate Iedge Entry %s/%lu with %s/%lu\n",
				fn, dupInfo->data.regid, dupInfo->data.uport,
				info->regid, info->uport));
			ERROR_PrintInfoEntry(MCACHE, info);
			continue;
		}

		/* If an entry is considered active, we need to review it
		 * Look at the last time it was active and compare it with the
		 * time when we are coming up. If we have been down for a long
		 * time, or if the entry did not properly unregister, we should
		 * make it inactive
		 */

		/* Create a cache entry for this */
		cacheInfo = CacheDupInfoEntry(info);

		ResetIedgeFields(cacheInfo, 1);

		if (!(flags & CACHE_SAVE_DYNAMIC))
		{
			UpdateIedgeDynamicInfo(&cacheInfo->data, NULL);
		}

		if (AddIedge(cacheInfo) < 0)
		{
			NETERROR(MCACHE, 
						("%s Duplicate Iedge Entry found\n", fn));
			ERROR_PrintInfoEntry(MCACHE, info);

			duplicates ++;			
			//SHM_Free(cacheInfo); - let this go wasted

			continue;
		}

		no ++;

		DEBUG_PrintInfoEntry(MCACHE, NETLOG_DEBUG4, info);
	}

 _return:
 _release_locks:
     
	CacheReleaseLocks(regCache);

	if (!dbin)
	{
		/* Close the database */
		DbClose(&dbstruct);
	}

	/* Set the number of configured ports for licensing purposes */
	nlm_setconfigport(all);

	return 0;
}

int
LsMemStructInit(LsMemStruct *ms)
{
	int i;

	lsMem = ms;

	IedgeInit(ms);
	UpdateListInit(ms);
	VpnInit(ms);
	VpnGInit(ms);
	CPInit(ms);
	CPBInit(ms);
	TriggerInit(ms);
	AlarmInit(ms);
	RsdInit(ms);
	RealmInit(ms);
	IgrpInit(ms);

	CallInit(ms);
	ConfInit(ms);
	SipCallCacheInit(ms);
	VnetInit(ms);

	for (i=0; i<MAX_H323INSTANCES; i++)
	{
		lsMem->allocStats[i] = SHM_Malloc(sizeof(AllocStats));
		memset(lsMem->allocStats[i], 0, sizeof(AllocStats));
	}

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("xthreads : %d", xthreads));
	lsMem->sipStats = SHM_Malloc(sizeof(SipStats) * xthreads);
	memset(lsMem->sipStats, 0, sizeof(SipStats) * xthreads);

	lsMem->cfgParms = SHM_Malloc(sizeof(CfgParms));
	memset(lsMem->cfgParms, 0, sizeof(CfgParms));

	allocCallStats();

	lsMem->scm = SHM_Malloc(sizeof(SCMInfo));
	memset(lsMem->scm, 0, sizeof(SCMInfo));

	LsMemStructAtt(ms);

	return 0;
}

int
LsMemStructInitLocal(LsMemStruct *ms)
{

	// Initialize the call and conf caches so that
	// the data lies in gis's local memory
	CacheInstantiate(ms->callCache);
	CacheInstantiate(ms->confCache);
	CacheInstantiate(ms->sipCallCache);

	SipTransCacheInitLocal(ms);
	CallInitLocal(ms);
	RegCacheInit(ms);

	LsMemStructAttLocal(ms);

	return 0;
}

int
LsMemPopulate(LsMemStruct *ms)
{
	if (ms == NULL)
	{
		return -1;
	}

	/* Initialize the cache here */
	IedgeCachePopulate(lsMem, NULL, CACHE_POPULATE_ALL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Iedge Cache Populated\n"));

	VpnPopulate(lsMem, NULL);

	VpnGPopulate(lsMem, NULL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Vpn Cache Populated\n"));

	CPPopulate(lsMem, NULL);

	CPBPopulate(lsMem, NULL);

	TriggerPopulate(lsMem, NULL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Call Routes Populated\n"));

	RealmPopulate(lsMem, NULL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Realm Populated\n"));

	IgrpPopulate(lsMem, NULL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Igrp Populated\n"));

	VnetPopulate(lsMem, NULL);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Vnet Populated\n"));

	lsMem->fwParams = NULL;
	lsMem->fwLock = NULL;

	return 0;
}

int
LsMemStructReset()
{
	/* Set up global pointers */
	lsMem = NULL;
	updateList = NULL;

	regCache =  NULL;
	regidCache =  NULL;
	phoneCache =  NULL;
	vpnPhoneCache = NULL;
	emailCache =  NULL;
	h323idCache =  NULL;
	tgCache = NULL;
	dtgCache = NULL;
        
	gwCache =  NULL;
	gkCache =  NULL;
	ipCache =  NULL;
	uriCache =  NULL;
	gwcpCache =  NULL;
	subnetCache =  NULL;
	cridCache =  NULL;

	realmCache = NULL;
	rsaCache = NULL;
	rsapubnetsCache = NULL;

	vpnCache = NULL;
	vpnGCache =  NULL;

	cpCache = NULL;
	cpdestCache =  NULL;
	cpsrcCache = NULL;
	cporigDNISCache = NULL;
	cporigANICache = NULL;

	cpbCache =  NULL;
	cpbcrCache =  NULL;
	cpbcpCache = NULL;
	lruRoutesCache = NULL;

	triggerCache = NULL;

	igrpCache = NULL;

	vnetCache = NULL;

	return 0;
}

int
LsMemStructAtt(LsMemStruct *m)
{
	/* Set up global pointers */
	lsMem = m;
	updateList = &m->updateList;

	regCache = m->regCache;
	regidCache = m->regidCache;
	phoneCache = m->phoneCache;
	vpnPhoneCache = m->vpnPhoneCache;
//	emailCache = m->emailCache; - not used anymore
	h323idCache = m->h323idCache;
	tgCache = m->tgCache;
	dtgCache = m->dtgCache;
        
	gwCache = m->gwCache;
	gkCache = m->gkCache;
	ipCache = m->ipCache;
	uriCache = m->uriCache;
	gwcpCache = m->gwcpCache;
	subnetCache = m->subnetCache;
	cridCache = m->cridCache;

	realmCache = m->realmCache;
	rsaCache = m->rsaCache;
	rsapubnetsCache = m->rsapubnetsCache;

	igrpCache = m->igrpCache;

	vpnCache = m->vpnCache;
	vpnGCache = m->vpnGCache;

	cpCache = m->cpCache;
	cpdestCache = m->cpdestCache;
	cpsrcCache = m->cpsrcCache;
	cptransitCache = m->cptransitCache;
	lruRoutesCache = m->lruRoutesCache;
	cporigDNISCache = m->cporigDNISCache;
	cporigANICache = m->cporigANICache;

	cporigDNISCache = m->cporigDNISCache;
	cporigANICache = m->cporigANICache;

	cpbCache = m->cpbCache;
	cpbcrCache = m->cpbcrCache;
	cpbcpCache = m->cpbcpCache;

	triggerCache = m->triggerCache;

	defaultRealm = &m->defaultRealm;

	callCache = m->callCache;
	confCache = m->confCache;

	sipCallCache = m->sipCallCache;

	vnetCache = m->vnetCache;

	return 0;
}

int
LsMemStructAttLocal(LsMemStruct *m)
{
	transCache = m->transCache;
	transDestCache = m->transDestCache;
	
	guidCache = m->guidCache;
	tipCache = m->tipCache;

	callCache = m->callCache;
	confCache = m->confCache;

	sipCallCache = m->sipCallCache;

	sipregCache = m->sipregCache;
	
	return 0;
}

int
CacheIedgeCachePreCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG1,
		("Cache %s: Operation %s Begin\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
CacheIedgeCachePostCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG1,
		("Cache %s: Operation %s End\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
CacheVpnCachePreCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG4,
		("Cache %s: Operation %s Begin\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
CacheVpnCachePostCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG4,
		("Cache %s: Operation %s End\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
CacheVpnGCachePreCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG4,
		("Cache %s: Operation %s Begin\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
CacheVpnGCachePostCb(cache_t cache, int op, void *data, size_t size)
{
	NETDEBUG(MSHM, NETLOG_DEBUG4,
		("Cache %s: Operation %s End\n", 
			cache->name, cache_operations_strings[op]));
	return 0;
}

int
LruRoutesPostCbCheck(cache_t cache, int op, void *data, size_t size)
{
	CacheRouteEntry *routeEntry;

	NETDEBUG(MSHM, NETLOG_DEBUG4,
		("Cache %s: Operation %s End\n", 
			cache->name, cache_operations_strings[op]));
	switch (op)
	{
	case CACHEOP_INSERT:
	case CACHEOP_GET:
	case CACHEOP_LOCK:
		// check the maxitem limit, delete at most one item
		if (cache->nitems > cache->xitems)
		{
			// We have to delete the least recently used
			routeEntry = CacheGetLast(cache);

			if (routeEntry)
			{
				// schedule a db update to delete this entry
				DbScheduleRouteDelete(&routeEntry->routeEntry);

				// delete this entry from the database
				DeleteRoute(&routeEntry->routeEntry);
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

int maxcalls = 0;
int 
CacheCallCachePreCb(struct cache_t *cache, int op, void *data, size_t size)
{
	// If operation is insert, count the number of inserts
	switch (op)
	{
	case CACHEOP_INSERT: 
		maxcalls++;
		break;
	case CACHEOP_DELETE:
		break;
	default:
		break;
	}

	return 0;
}

int
CallInit(LsMemStruct *m)
{
	LockInit(&m->callmutex, 1);

	/* Initialize the cp cache set */
	m->callCache = CacheCreate(MEM_SHARED);

	m->callCache->dt = CACHE_DT_AVL;
	CacheSetName(m->callCache, "CALL Cache");
	m->callCache->cachecmp = CallCmp;
	m->callCache->cacheinscmp = CallInsCmp;
	m->callCache->cachedup = CallDup;
	m->callCache->lock = &m->callmutex;

	// We want data to be local
	m->callCache->malloc = MEM_LOCAL;
	m->callCache->free = MEM_LOCAL;

	// Instantiation postponed inside gis
	return(0);
}

int
CallInitLocal(LsMemStruct *m)
{
	// GUID cache - let's keep in local memory
	m->guidCache = CacheCreate(1);
	m->guidCache->dt = CACHE_DT_AVL;
	CacheSetName(m->guidCache, "GUID Call Cache");
	m->guidCache->cachecmp = GuidCmp;
	m->guidCache->cacheinscmp = GuidInsCmp;
	m->guidCache->cachedup = CallDup;
	m->guidCache->lock = &m->callmutex;
	CacheInstantiate(m->guidCache);

	// TIP cache - let's keep it in local memory
	m->tipCache = CacheCreate(1);
	m->tipCache->dt = CACHE_DT_AVL;
	CacheSetName(m->tipCache, "TIP Cache");
	m->tipCache->cachecmp = TipCmp;
	m->tipCache->cacheinscmp = TipInsCmp;
	m->tipCache->cachedup = CallDup;
	m->tipCache->lock = &m->callmutex;
	CacheInstantiate(m->tipCache);

	return 0;
}

int
ConfInit(LsMemStruct *m)
{
	LockInit(&m->confmutex, 1);

	/* Initialize the cp cache set */
	m->confCache = CacheCreate(MEM_SHARED);
	m->confCache->dt = CACHE_DT_AVL;
	CacheSetName(m->confCache, "CONF Cache");
	m->confCache->cachecmp = ConfCmp;
	m->confCache->cacheinscmp = ConfInsCmp;
	m->confCache->cachedup = ConfDup;
	m->confCache->lock = &m->confmutex;

	// We want data to be local
	m->confCache->malloc = MEM_LOCAL;
	m->confCache->free = MEM_LOCAL;

	// Instantiation postponed inside gis

	return(0);
}

int
SipCallCacheInit(LsMemStruct *m)
{
	/* Initialize the sip call cache set */
	m->sipCallCache = CacheCreate(MEM_SHARED);
	m->sipCallCache->dt = CACHE_DT_AVL;
	CacheSetName(m->sipCallCache, "SIP Call Cache");
	m->sipCallCache->cachecmp = SipCallCmp;
	m->sipCallCache->cacheinscmp = SipCallInsCmp;
	m->sipCallCache->cachedup = SipCallDup;
//	m->sipCallCache->pre_cond = IedgeCachePreCb;
	m->sipCallCache->lock = &m->callmutex;

	// We want data to be local
	m->sipCallCache->malloc = MEM_LOCAL;
	m->sipCallCache->free = MEM_LOCAL;

	// Instantiation postponed inside gis

	return(0);
}

int
RegCacheInit(LsMemStruct *m)
{
	m->sipregCache = CacheCreate(1);
	m->sipregCache->dt = CACHE_DT_AVL;
	CacheSetName(m->sipregCache, "SIP REG Cache");
	m->sipregCache->cachecmp = SipRegCmp;
	m->sipregCache->cacheinscmp = SipRegInsCmp;
	m->sipregCache->cachedup = SipRegDup;
	m->sipregCache->lock = &m->sipregmutex;

	CacheInstantiate(m->sipregCache);
	return(0);
}

int
SipTransCacheInitLocal(LsMemStruct *m)
{
	LockInit(&m->transmutex, 1);

	/* Initialize the trans cache set */
	m->transCache = CacheCreate(1);
	m->transCache->dt = CACHE_DT_AVL;
	CacheSetName(m->transCache, "TRANS Cache");
	m->transCache->cachecmp = TransCmp;
	m->transCache->cacheinscmp = TransInsCmp;
	m->transCache->cachedup = TransDup;
	m->transCache->lock = &m->transmutex;
	m->transCache->pre_cond = IedgeCachePreCb;
	m->transCache->post_cond = IedgeCachePostCb;
	CacheInstantiate(m->transCache);

	m->transDestCache = CacheCreate(1);
	m->transDestCache->dt = CACHE_DT_AVL;
	CacheSetName(m->transDestCache, "TRANSD Cache");
	m->transDestCache->cachecmp = TransDestCmp;
	m->transDestCache->cacheinscmp = TransDestInsCmp;
	m->transDestCache->cachedup = TransDestDup;
	m->transDestCache->lock = &m->transmutex;
	CacheInstantiate(m->transDestCache);

	return(0);
}

int
AlarmInit(LsMemStruct *m)
{
	LockInit(&m->alarmmutex, 1);
        m->lsVportAlarm  = SHM_Malloc(MAX_LS_ALARM*sizeof(long));
        memset(m->lsVportAlarm,0,MAX_LS_ALARM*sizeof(long));	
        m->lsMRVportAlarm  = SHM_Malloc(MAX_LS_ALARM*sizeof(long));
        memset(m->lsMRVportAlarm,0,MAX_LS_ALARM*sizeof(long));	

	return 0;
}

int
RsdInit (LsMemStruct *m)
{
	LockInit(&m->rsdmutex, 1);
	m->rsdInfo = SHM_Malloc(sizeof(RSDInfo));
	memset(m->rsdInfo, 0, sizeof(RSDInfo));

	return 0;
}

void
CPCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheDestroyKeys(m->cpdestCache);
	CacheDestroyKeys(m->cpsrcCache);
	CacheDestroyKeys(m->cporigDNISCache);
	CacheDestroyKeys(m->cporigANICache);
	CacheDestroyKeys(m->cptransitCache);
	CacheDestroyKeys(m->lruRoutesCache);

	// Free up the main cache
	CacheDestroyData(m->cpCache);
}

void
CPCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheInstantiate(m->cpCache);

	CacheInstantiate(m->cpdestCache);

	CacheInstantiate(m->cpsrcCache);

	CacheInstantiate(m->cptransitCache);

	CacheInstantiate(m->lruRoutesCache);

	CacheInstantiate(m->cporigDNISCache);
	CacheInstantiate(m->cporigANICache);
}

int
CPInit(LsMemStruct *m)
{
	/* Initialize the cp cache set */
	LockInit(&m->cpmutex, 1);

	m->cpCache = CacheCreate(0);
	m->cpCache->dt = CACHE_DT_AVLT;
	CacheSetName(m->cpCache, "CP Cache");
	m->cpCache->cachecmp = CPCmp;
	m->cpCache->cacheinscmp = CPInsCmp;
	m->cpCache->cachedup = CPDup;
	m->cpCache->lock = &m->cpmutex;

	/* Initialize the cp cache set based on dest */
	m->cpdestCache = CacheCreate(0);
	m->cpdestCache->dt = CACHE_DT_TST;
	CacheSetName(m->cpdestCache, "CPDest Cache");
	m->cpdestCache->cachecmp = CPCmp;
	m->cpdestCache->cacheinscmp = CPCmp;
	m->cpdestCache->cachedup = CPDup;
	m->cpdestCache->nodeLineWidth = 250;
	m->cpdestCache->cachedata2Key = CPData2Key;
	m->cpdestCache->cachedatains2Key = CPInsData2Key;
	m->cpdestCache->lock = &m->cpmutex;

	/* Initialize the cp cache set based on dest */
	m->cpsrcCache = CacheCreate(0);
	m->cpsrcCache->dt = CACHE_DT_TST;
	CacheSetName(m->cpdestCache, "CPSrc Cache");
	m->cpsrcCache->cachecmp = CPCmp;
	m->cpsrcCache->cacheinscmp = CPCmp;
	m->cpsrcCache->cachedup = CPDup;
	m->cpsrcCache->nodeLineWidth = 250;
	m->cpsrcCache->cachedata2Key = CPData2Key;
	m->cpsrcCache->cachedatains2Key = CPSrcInsData2Key;
	m->cpsrcCache->lock = &m->cpmutex;

	/* Initialize the cp cache set based on dest */
	m->cptransitCache = CacheCreate(0);
	m->cptransitCache->dt = CACHE_DT_TST;
	CacheSetName(m->cptransitCache, "CPTransit Cache");
	m->cptransitCache->cachecmp = CPCmp;
	m->cptransitCache->cacheinscmp = CPCmp;
	m->cptransitCache->cachedup = CPDup;
	m->cptransitCache->nodeLineWidth = 250;
	m->cptransitCache->cachedata2Key = CPData2Key;
	m->cptransitCache->cachedatains2Key = CPInsData2Key;
	m->cptransitCache->lock = &m->cpmutex;

	/* Initialize the cp cache set based on dest */
	m->lruRoutesCache = CacheCreate(0);
	m->lruRoutesCache->dt = CACHE_DT_CLIST;
	CacheSetName(m->lruRoutesCache, "LRURoutes Cache");
	m->lruRoutesCache->cachecmp = RoutesLRUInsCmp;
	m->lruRoutesCache->cacheinscmp = RoutesLRUInsCmp;
	m->lruRoutesCache->cachedup = CPDup;
	m->lruRoutesCache->lock = &m->cpmutex;
	m->lruRoutesCache->listoffset = LRUROUTESOFFSET;
	m->lruRoutesCache->xitems = nlruRoutes;
	m->lruRoutesCache->post_cond = LruRoutesPostCb;

	m->cporigDNISCache = CacheCreate(0);
	m->cporigDNISCache->dt = CACHE_DT_TST;
	CacheSetName(m->cporigDNISCache, "CPorgDNIS Cache");
	m->cporigDNISCache->cachecmp = CPCmp;
	m->cporigDNISCache->cacheinscmp = CPCmp;
	m->cporigDNISCache->cachedup = CPDup;
	m->cporigDNISCache->nodeLineWidth = 250;
	m->cporigDNISCache->cachedata2Key = CPData2Key;
	m->cporigDNISCache->cachedatains2Key = CPInsData2Key;
	m->cporigDNISCache->lock = &m->cpmutex;

	m->cporigANICache = CacheCreate(0);
	m->cporigANICache->dt = CACHE_DT_TST;
	CacheSetName(m->cporigANICache, "CPorgANI Cache");
	m->cporigANICache->cachecmp = CPCmp;
	m->cporigANICache->cacheinscmp = CPCmp;
	m->cporigANICache->cachedup = CPDup;
	m->cporigANICache->nodeLineWidth = 250;
	m->cporigANICache->cachedata2Key = CPData2Key;
	m->cporigANICache->cachedatains2Key = CPSrcInsData2Key;
	m->cporigANICache->lock = &m->cpmutex;

	CPCacheInstantiate(m);

	return(0);
}

void
CPBCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheDestroyKeys(m->cpbcpCache);
	CacheDestroyKeys(m->cpbcrCache);

	// Free up the main cache
	CacheDestroyData(m->cpbCache);
}

void
CPBCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheInstantiate(m->cpbCache);
	CacheInstantiate(m->cpbcrCache);
	CacheInstantiate(m->cpbcpCache);
}

int
CPBInit(LsMemStruct *m)
{
	/* Initialize the cpb cache set */
	LockInit(&m->cpbmutex, 1);

	m->cpbCache = CacheCreate(0);
	m->cpbCache->dt = CACHE_DT_AVLT;
	CacheSetName(m->cpbCache, "CPB Cache");
	m->cpbCache->cachecmp = CPBCmp;
	m->cpbCache->cacheinscmp = CPBInsCmp;
	m->cpbCache->cachedup = CPBDup;
	m->cpbCache->lock = &m->cpbmutex;

	/* Initialize the cpb cache set based on routenames */
	m->cpbcrCache = CacheCreate(0);
	m->cpbcrCache->dt = CACHE_DT_TST;
	CacheSetName(m->cpbcrCache, "CPBCR Cache");
	m->cpbcrCache->cachecmp = CPBCmp;
	m->cpbcrCache->cacheinscmp = CPBCmp;
	m->cpbcrCache->cachedup = CPBDup;
	m->cpbcrCache->lock = &m->cpbmutex;
	m->cpbcrCache->nodeLineWidth = 250;
	m->cpbcrCache->cachedata2Key = CPBCRData2Key;
	m->cpbcrCache->cachedatains2Key = CPBCRInsData2Key;

	/* Initialize the cpb cache set based on routenames */
	m->cpbcpCache = CacheCreate(0);
	m->cpbcpCache->dt = CACHE_DT_TST;
	CacheSetName(m->cpbcpCache, "CPBCP Cache");
	m->cpbcpCache->cachecmp = CPBCmp;
	m->cpbcpCache->cacheinscmp = CPBCmp;
	m->cpbcpCache->cachedup = CPBDup;
	m->cpbcpCache->lock = &m->cpbmutex;
	m->cpbcpCache->nodeLineWidth = 250;
	m->cpbcpCache->cachedata2Key = CPBCPData2Key;
	m->cpbcpCache->cachedatains2Key = CPBCPInsData2Key;

	CPBCacheInstantiate(m);

	return(0);
}

int
IedgeInit(LsMemStruct *m)
{
	/* Initialize the iedge cache set */
	LockInit(&m->iedgemutex, 1);

	m->regCache = CacheCreate(0);
	m->regidCache = CacheCreate(0);
	m->phoneCache = CacheCreate(0);
	m->vpnPhoneCache = CacheCreate(0);
//	m->emailCache = CacheCreate(0);
	m->h323idCache = CacheCreate(0);
	m->tgCache = CacheCreate(0);
	m->dtgCache = CacheCreate(0);
	m->gwCache = CacheCreate(0);
	m->gkCache = CacheCreate(0);
	m->ipCache = CacheCreate(0);
	m->uriCache = CacheCreate(0);
	m->gwcpCache = CacheCreate(0);
	m->subnetCache = CacheCreate(0);
	m->cridCache = CacheCreate(0);

	m->regCache->dt = CACHE_DT_AVL;
	m->regidCache->dt = CACHE_DT_AVL;
	m->phoneCache->dt = CACHE_DT_AVL;
	m->phoneCache->dt = CACHE_DT_TST;
	m->phoneCache->nodeLineWidth = 250;
//	m->vpnPhoneCache->dt = CACHE_DT_AVL;
//	m->emailCache->dt = CACHE_DT_AVL;
	m->h323idCache->dt = CACHE_DT_AVL;
	m->tgCache->dt = CACHE_DT_AVL;
	m->dtgCache->dt = CACHE_DT_AVL;
	m->gwCache->dt = CACHE_DT_AVL;
	m->gkCache->dt = CACHE_DT_AVLT;
	m->ipCache->dt = CACHE_DT_AVL;
	m->uriCache->dt = CACHE_DT_AVL;
	m->gwcpCache->dt = CACHE_DT_TST;
	m->subnetCache->dt = CACHE_DT_AVL;
	m->cridCache->dt = CACHE_DT_AVLT;

	CacheSetName(m->regCache, "Reg Cache");
	CacheSetName(m->regidCache, "Regid Cache");
	CacheSetName(m->phoneCache, "Phone Cache");
	CacheSetName(m->vpnPhoneCache, "VpnPhone Cache");
//	CacheSetName(m->emailCache, "Email Cache");
	CacheSetName(m->h323idCache, "H323id Cache");
	CacheSetName(m->tgCache, "TG Cache");
	CacheSetName(m->dtgCache, "DTG Cache");
        
	CacheSetName(m->gwCache, "Gw Cache");
	CacheSetName(m->gkCache, "Gk Cache");
	CacheSetName(m->ipCache, "Ip Cache");
	CacheSetName(m->uriCache, "Uri Cache");
	CacheSetName(m->gwcpCache, "GwCp Cache");
	CacheSetName(m->subnetCache, "Subnet Cache");
	CacheSetName(m->cridCache, "CrId Cache");

	/* Initialize the cache callback functions */
#ifdef debugging_needed
	m->regCache->pre_cond = IedgeCachePreCb;
	m->regCache->post_cond = IedgeCachePostCb;
	m->regidCache->pre_cond = IedgeCachePreCb;
	m->regidCache->post_cond = IedgeCachePostCb;
	m->phoneCache->pre_cond = IedgeCachePreCb;
	m->phoneCache->post_cond = IedgeCachePostCb;
	m->vpnPhoneCache->pre_cond = IedgeCachePreCb;
	m->vpnPhoneCache->post_cond = IedgeCachePostCb;
	m->emailCache->pre_cond = IedgeCachePreCb;
	m->emailCache->post_cond = IedgeCachePostCb;
	m->h323idCache->pre_cond = IedgeCachePreCb;
	m->h323idCache->post_cond = IedgeCachePostCb;
	m->tgCache->pre_cond = IedgeCachePreCb;
	m->tgCache->post_cond = IedgeCachePostCb;
	m->gwCache->pre_cond = IedgeCachePreCb;
	m->gwCache->post_cond = IedgeCachePostCb;
	m->ipCache->pre_cond = IedgeCachePreCb;
	m->ipCache->post_cond = IedgeCachePostCb;
	m->uriCache->pre_cond = IedgeCachePreCb;
	m->uriCache->post_cond = IedgeCachePostCb;
#endif

	m->regCache->cachecmp = IedgeRegCmp;
	m->regidCache->cachecmp = IedgeRegidCmp;
	m->phoneCache->cachecmp = IedgePhoneCmp;
	m->phoneCache->cachedata2Key = IedgePhoneData2Key;
	m->vpnPhoneCache->cachecmp = IedgeVpnPhoneCmp;
//	m->emailCache->cachecmp = IedgeEmailCmp;
	m->h323idCache->cachecmp = IedgeH323IdCmp;
	m->tgCache->cachecmp = IedgeTGCmp;
	m->dtgCache->cachecmp = IedgeTGCmp;
        
	m->gwCache->cachecmp = IedgeRegCmp;
	m->ipCache->cachecmp = IedgeIpCmp;
	m->uriCache->cachecmp = IedgeUriCmp;
	m->gkCache->cachecmp = IedgeGkCmp;
	m->gwcpCache->cachecmp = IedgeRegCmp;
	m->subnetCache->cachecmp = IedgeSubnetCmp;
	m->cridCache->cachecmp = IedgeCrIdCmp;

	m->regCache->cacheinscmp = IedgeRegInsCmp;
	m->regidCache->cacheinscmp = IedgeRegidInsCmp;
	m->phoneCache->cacheinscmp = IedgePhoneInsCmp;
	m->phoneCache->cachedatains2Key = IedgePhoneInsData2Key;
	m->vpnPhoneCache->cacheinscmp = IedgeVpnPhoneInsCmp;
//	m->emailCache->cacheinscmp = IedgeEmailInsCmp;
	m->h323idCache->cacheinscmp = IedgeH323IdInsCmp;
	m->tgCache->cacheinscmp = IedgeTGInsCmp;
	m->dtgCache->cacheinscmp = IedgeTGInsCmp;
        
	m->gwCache->cacheinscmp = IedgeRegInsCmp;
	m->ipCache->cacheinscmp = IedgeIpInsCmp;
	m->uriCache->cacheinscmp = IedgeUriInsCmp;
	m->gkCache->cacheinscmp = IedgeGkInsCmp;
	m->gwcpCache->cacheinscmp = IedgeRegInsCmp;
	m->subnetCache->cacheinscmp = IedgeSubnetInsCmp;
	m->cridCache->cacheinscmp = IedgeCrIdInsCmp;

	m->regCache->cachedup = IedgeRegDup;
	m->regidCache->cachedup = IedgeRegidDup;
	m->phoneCache->cachedup = IedgePhoneDup;
	m->vpnPhoneCache->cachedup = IedgeVpnPhoneDup;
//	m->emailCache->cachedup = IedgeEmailDup;
	m->h323idCache->cachedup = IedgeH323IdDup;
	m->tgCache->cachedup = IedgeTGDup;
	m->dtgCache->cachedup = IedgeTGDup;
        
	m->gwCache->cachedup = IedgeRegDup;
	m->gkCache->cachedup = IedgeGkDup;
	m->ipCache->cachedup = IedgeIpDup;
	m->uriCache->cachedup = IedgeUriDup;
	m->gwcpCache->cachedup = IedgeRegDup;
	m->ipCache->cachedup = IedgeSubnetDup;
	m->cridCache->cachedup = IedgeCrIdDup;

	m->regCache->lock = &m->iedgemutex;
	m->regidCache->lock = &m->iedgemutex;
	m->phoneCache->lock = &m->iedgemutex;
	m->vpnPhoneCache->lock = &m->iedgemutex;
//	m->emailCache->lock = &m->iedgemutex;
	m->h323idCache->lock = &m->iedgemutex;
	m->tgCache->lock = &m->iedgemutex;
	m->dtgCache->lock = &m->iedgemutex;
	m->gwCache->lock = &m->iedgemutex;
	m->gkCache->lock = &m->iedgemutex;
	m->ipCache->lock = &m->iedgemutex;
	m->uriCache->lock = &m->iedgemutex;
	m->gwcpCache->lock = &m->iedgemutex;
	m->subnetCache->lock = &m->iedgemutex;
	m->cridCache->lock = &m->iedgemutex;

	m->gwcpCache->nodeLineWidth = 250;
	m->gwcpCache->cachedata2Key = IedgeGwCPData2Key;
	m->gwcpCache->cachedatains2Key = IedgeGwCPInsData2Key;

	IedgeCacheInstantiate(m);

	return 0;
}

void
IedgeCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	// Since the data is multiply indexed,
	// we must delete all trees before we finally
	// delete the data	

	CacheDestroyKeys(m->regidCache);
	CacheDestroyKeys(m->phoneCache);
	CacheDestroyKeys(m->vpnPhoneCache);
//	CacheDestroyKeys(m->emailCache);
	CacheDestroyKeys(m->h323idCache);
	CacheDestroyKeys(m->tgCache);
	CacheDestroyKeys(m->dtgCache);
	CacheDestroyKeys(m->gwCache);
	CacheDestroyKeys(m->ipCache);
	CacheDestroyKeys(m->uriCache);
	CacheDestroyKeys(m->gwcpCache);
	CacheDestroyKeys(m->subnetCache);

	// Destroy the data from the main cache
	// each iedge is guaranteed to have a regid/uport
	CacheDestroyData(m->regCache);

	// gk cache is separate
	CacheDestroyData(m->gkCache);
}

int
IedgeCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}

	CacheInstantiate(m->regCache);
	CacheInstantiate(m->regidCache);
	CacheInstantiate(m->phoneCache);
	CacheInstantiate(m->vpnPhoneCache);
//	CacheInstantiate(m->emailCache);
	CacheInstantiate(m->h323idCache);
	CacheInstantiate(m->tgCache);
	CacheInstantiate(m->dtgCache);
	CacheInstantiate(m->gwCache);
	CacheInstantiate(m->gkCache);
	CacheInstantiate(m->ipCache);
	CacheInstantiate(m->uriCache);
	CacheInstantiate(m->gwcpCache);
	CacheInstantiate(m->subnetCache);
	CacheInstantiate(m->cridCache);

	return 0;
}

int
UpdateListInit(LsMemStruct *m)
{
	pthread_mutexattr_t mattr;

	updateList = &m->updateList;

	memset(&m->updateList, 0, sizeof(CacheEntry));

	/* Initialize the list structure */
	m->updateList.prev = m->updateList.next = &m->updateList;
	updateList = &m->updateList;
	
#ifdef SUNOS
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&m->updatemutex, &mattr);
#else
	pthread_mutexattr_init(&mattr);
	pthread_mutex_init(&m->updatemutex, &mattr);
#endif

	return 0;
}

void
VpnCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheDestroyData(m->vpnCache);
}

void
VpnCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheInstantiate(m->vpnCache);
}

int
VpnInit(LsMemStruct *m)
{
	LockInit(&m->vpnmutex, 1);

	m->vpnCache = CacheCreate(0);
	m->vpnCache->dt = CACHE_DT_AVL;

	CacheSetName(m->vpnCache, "Vpn Cache");

//	m->vpnCache->pre_cond = VpnCachePreCb;
//	m->vpnCache->post_cond = VpnCachePostCb;

	m->vpnCache->cachecmp = VpnCmp;
	m->vpnCache->cacheinscmp = VpnCmp;
	m->vpnCache->cachedup = VpnDup;
	m->vpnCache->lock = &m->vpnmutex;
	VpnCacheInstantiate(m);

	return 0;
}

void
VpnGCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheDestroyData(m->vpnGCache);
}

void
VpnGCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheInstantiate(m->vpnGCache);
}

int
VpnGInit(LsMemStruct *m)
{
	LockInit(&m->vpngmutex, 1);

	m->vpnGCache = CacheCreate(0);
	m->vpnGCache->dt = CACHE_DT_AVL;
	CacheSetName(m->vpnGCache, "VpnG Cache");

//	m->vpnGCache->pre_cond = VpnGCachePreCb;
//	m->vpnGCache->post_cond = VpnGCachePostCb;

	m->vpnGCache->cachecmp = VpnGCmp;
	m->vpnGCache->cacheinscmp = VpnGCmp;
	m->vpnGCache->cachedup = VpnGDup;
	m->vpnGCache->lock = &m->vpngmutex;
	VpnGCacheInstantiate(m);

	return 0;
}

void
TriggerCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return;
	}

	CacheInstantiate(m->triggerCache);
}

int
TriggerInit(LsMemStruct *m)
{
	LockInit(&m->triggermutex, 1);

	m->triggerCache = CacheCreate(0);
	m->triggerCache->dt = CACHE_DT_AVL;
	CacheSetName(m->triggerCache, "Trigger Cache");

	m->triggerCache->cachecmp = TriggerCmp;
	m->triggerCache->cacheinscmp = TriggerInsCmp;
	m->triggerCache->cachedup = TriggerDup;
	m->triggerCache->lock = &m->triggermutex;

	TriggerCacheInstantiate(m);

	return 0;
}

int
RealmCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}

	CacheInstantiate(m->realmCache);
	CacheInstantiate(m->rsaCache);
	CacheInstantiate(m->rsapubnetsCache);

	return 0;
}

int
RealmInit(LsMemStruct *m)
{
	LockInit(&m->realmmutex, 1);

	m->realmCache = CacheCreate(0);
	m->realmCache->dt = CACHE_DT_AVL;
	CacheSetName(m->realmCache, "Realm Cache");
	m->realmCache->cachecmp = RealmCmpFnId;
	m->realmCache->cacheinscmp = RealmInsCmpFnId;
	m->realmCache->lock = &m->realmmutex;

	m->rsaCache = CacheCreate(0);
	m->rsaCache->dt = CACHE_DT_AVL;
	CacheSetName(m->rsaCache, "Rsa Cache");
	m->rsaCache->cachecmp = RsaCmpFnId;
	m->rsaCache->cacheinscmp = RsaInsCmpFnId;
	m->rsaCache->lock = &m->realmmutex;

	m->rsapubnetsCache = CacheCreate(0);
	m->rsapubnetsCache->dt = CACHE_DT_AVL;
	CacheSetName(m->rsapubnetsCache, "Rsapubnets Cache");
	m->rsapubnetsCache->cachecmp = RsapubnetsCmpFnId;
	m->rsapubnetsCache->cacheinscmp = RsapubnetsInsCmpFnId;
	m->rsapubnetsCache->lock = &m->realmmutex;

	RealmCacheInstantiate(m);

	defaultRealm = &m->defaultRealm;

	return 0;
}

int
RealmCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}

	// Free up the main cache
    CacheDestroyKeys(m->rsaCache);
    CacheDestroyKeys(m->rsapubnetsCache);
	CacheDestroyData(m->realmCache);

	m->defaultRealm = NULL;

    return 0;
}


int
IgrpCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}
	CacheInstantiate(m->igrpCache);
	return 0;
}

int
IgrpInit(LsMemStruct *m)
{
	LockInit(&m->igrpmutex, 1);

	m->igrpCache= CacheCreate(0);
	m->igrpCache->dt = CACHE_DT_AVL;
	CacheSetName(m->igrpCache, "IGRP Cache");
	m->igrpCache->cachecmp = IgrpCmpFnId;
	m->igrpCache->cacheinscmp = IgrpInsCmpFnId;
	m->igrpCache->lock = &m->igrpmutex;

	IgrpCacheInstantiate(m);

	return 0;
}

int
IgrpCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}

	// Free up the main cache
    CacheDestroyKeys(m->igrpCache);
    return 0;
}

int
VnetCacheInstantiate(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}
	CacheInstantiate(m->vnetCache);
	return 0;
}

int
VnetInit(LsMemStruct *m)
{
	LockInit(&m->vnetmutex, 1);

	m->vnetCache= CacheCreate(0);
	m->vnetCache->dt = CACHE_DT_AVL;
	CacheSetName(m->vnetCache, "VNET Cache");
	m->vnetCache->cachecmp = VnetCmpFnId;
	m->vnetCache->cacheinscmp = VnetInsCmpFnId;
	m->vnetCache->lock = &m->vnetmutex;

	VnetCacheInstantiate(m);

	return 0;
}

int
VnetCacheDestroyData(LsMemStruct *m)
{
	if (m == NULL)
	{
		return -1;
	}

	// Free up the main cache
    CacheDestroyKeys(m->vnetCache);
    return 0;
}

int
CacheVpnCmp(const void *v1, const void *v2, void *param)
{
	VpnEntry *e1 = (VpnEntry *)v1, *e2 = (VpnEntry *)v2;

	return strcmp(e1->vpnName, e2->vpnName);	
}

int
CacheIedgeRegInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, 
			*c2 = (CacheTableInfo *)v2;
	int rc;
	
	rc = memcmp(c1->data.regid, c2->data.regid, REG_ID_LEN);
	if (rc == 0)
	{
		if (c1->data.uport < c2->data.uport)
		{
			return -1;
		}
		else if (c1->data.uport > c2->data.uport)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return rc;
	}
}

int
CacheIedgeRegCmp(const void *v1, const void *v2, void *param)
{
	InfoEntry *c1 = (InfoEntry *)v1;
	CacheTableInfo *c2 = (CacheTableInfo *)v2;
	int rc;
	
	rc = memcmp(c1->regid, c2->data.regid, REG_ID_LEN);
	if (rc == 0)
	{
		if (c1->uport < c2->data.uport)
		{
			return -1;
		}
		else if (c1->uport > c2->data.uport)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return rc;
	}
}

int
CacheIedgeRegidInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, 
			*c2 = (CacheTableInfo *)v2;
	int rc;
	
	return memcmp(c1->data.regid, c2->data.regid, REG_ID_LEN);
}

int
CacheIedgeRegidCmp(const void *v1, const void *v2, void *param)
{
	InfoEntry *c1 = (InfoEntry *)v1;
	CacheTableInfo *c2 = (CacheTableInfo *)v2;
	int rc;
	
	return memcmp(c1->regid, c2->data.regid, REG_ID_LEN);
}

int
CacheIedgePhoneInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;

	return strncmp(c1->data.phone, c2->data.phone, PHONE_NUM_LEN);	
}

int
CacheIedgePhoneCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	return strncmp(v1, c2->data.phone, PHONE_NUM_LEN);	
}

int
CacheIedgeVpnPhoneCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	return strncmp(v1, c2->data.vpnPhone, PHONE_NUM_LEN);	
}

int
CacheIedgeVpnPhoneInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;

	return strncmp(c1->data.vpnPhone, c2->data.vpnPhone, PHONE_NUM_LEN);	
}

int
CacheIedgeCrIdInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;

	return (c1->data.crId > c2->data.crId) ? 1
		 : ((c1->data.crId < c2->data.crId) ? -1 : 0);
}

int
CacheIedgeCrIdCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;
	int crId1 = *(int*)v1;

	return (crId1 > c2->data.crId) ? 1
		 : ((crId1 < c2->data.crId) ? -1 : 0);
}

int
CacheIedgeIpInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;
	int   rc;
/* change the ordering to realm and then ip */
	rc  = (c1->data.realmId > c2->data.realmId) ? 1 :
			((c1->data.realmId < c2->data.realmId) ? -1 : 0);
	if (!rc) 
	{
		rc = (c1->data.ipaddress.l > c2->data.ipaddress.l) ? 1
			 : ((c1->data.ipaddress.l < c2->data.ipaddress.l) ? -1 : 0);
	}

	return rc;
}

int
CacheIedgeIpCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;
	RealmIP  *rmip = (RealmIP *)v1;
	unsigned long ip1 = rmip->ipaddress;
	unsigned long rmid1 = rmip->realmId;
	int rc ;

	rc = (rmid1 > c2->data.realmId) ? 1
			: ((rmid1 < c2->data.realmId) ? -1 : 0);
	if (!rc)
	{
		rc = (ip1 > c2->data.ipaddress.l) ? 1
			 : ((ip1 < c2->data.ipaddress.l) ? -1 : 0);
	}
	return rc;
}

int
SubnetCmp(Subnet *subnet1p, Subnet *subnet2p)
{
	unsigned long subnet1, subnet2, mask1, mask2, realm1, realm2;
	int rc=0;

	subnet1 = subnet1p->subnetip;
	mask1 = subnet1p->mask;

	subnet2 = subnet2p->subnetip;
	mask2 = subnet2p->mask;

	subnet1 &= mask1;	
	subnet2 &= mask2;	

    rc = (subnet1 > subnet2) ? 1
		 : ((subnet1 < subnet2) ? -1 : 0);

	if (!rc)
	{
		rc = (mask1 > mask2) ? 1
			 : ((mask1 < mask2) ? -1 : 0);
	}

	return rc;
}

int
CacheIedgeSubnetInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;
	Subnet	 sub1={0}, sub2 = {0};
	int rc;

	sub1.subnetip = c1->data.subnetip;
	sub1.mask = c1->data.subnetmask;

	sub2.subnetip = c2->data.subnetip;
	sub2.mask = c2->data.subnetmask;

	rc = (c1->data.realmId > c2->data.realmId) ? 1
		 : ( (c1->data.realmId < c2->data.realmId) ? -1  : 0);

	if (rc) return rc;
	else return SubnetCmp(&sub1, &sub2);
}

int
CacheIedgeSubnetCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;
	RealmSubnet *rmsub1 = (RealmSubnet *)v1;
	Subnet	 sub1={0}, sub2 = {0};
	int rc;

	sub1.subnetip = rmsub1->subnetip;
	sub1.mask = rmsub1->mask;

	sub2.subnetip = c2->data.subnetip;
	sub2.mask = c2->data.subnetmask;

	rc = (rmsub1->realmId > c2->data.realmId) ? 1
		 : ( (rmsub1->realmId < c2->data.realmId) ? -1  : 0);

	if (rc) return rc;
	else return SubnetCmp(&sub1, &sub2);
}

int 
CacheRealmInsCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c1 = (CacheRealmEntry *)v1;
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	int 		val;

	val = c1->realm.realmId - c2->realm.realmId;
	return ((val > 0) ? 1 : ( (val < 0) ? -1 : 0 ) );
}

int 
CacheRealmCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	unsigned long realmId1 = *(unsigned long *)v1;
	int			val;

	val = realmId1 - c2->realm.realmId;
	return ((val > 0) ? 1 : ((val < 0) ? -1 : 0));
}

int 
CacheRsaInsCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c1 = (CacheRealmEntry *)v1;
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	int 		val;

	val = c1->realm.rsa - c2->realm.rsa;
	return ((val > 0) ? 1 : ( (val < 0) ? -1 : 0 ) );
}

int 
CacheRsaCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	unsigned long rsa1 = *(unsigned long *)v1;
	int			val;

	val = rsa1 - c2->realm.rsa;
	return ((val > 0) ? 1 : ((val < 0) ? -1 : 0));
}

int 
CacheRsapubnetsInsCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c1 = (CacheRealmEntry *)v1;
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	Subnet	 sub1={0}, sub2 = {0};

	sub1.subnetip = c1->realm.rsa;
	sub1.mask = c1->realm.mask;

	sub2.subnetip = c2->realm.rsa;
	sub2.mask = c2->realm.mask;

	return SubnetCmp(&sub1, &sub2);
}

int 
CacheRsapubnetsCmp(const void *v1, const void *v2, void *param)
{
	CacheRealmEntry *c2 = (CacheRealmEntry *)v2;
	Subnet  *sub1 = (Subnet *)v1;
	Subnet	 sub2 = {0};

	sub2.subnetip = c2->realm.rsa;
	sub2.mask = c2->realm.mask;

	return SubnetCmp(sub1, &sub2);
}

int
CacheTriggerInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTriggerEntry *c1 = (CacheTriggerEntry *)v1, *c2 = (CacheTriggerEntry *)v2;
	int rc;

	rc = (c1->trigger.event > c2->trigger.event) ? 1
		 : ((c1->trigger.event < c2->trigger.event) ? -1 : 0);

	if (rc == 0)
	{
		rc = (c1->trigger.srcvendor > c2->trigger.srcvendor) ? 1
				: ((c1->trigger.srcvendor < c2->trigger.srcvendor) ? -1: 0);

		if (rc == 0)
		{
			rc = (c1->trigger.dstvendor > c2->trigger.dstvendor) ? 1
					: ((c1->trigger.dstvendor < c2->trigger.dstvendor) ? -1: 0);
		}
	}

	return rc;
}

int
CacheTriggerCmp(const void *v1, const void *v2, void *param)
{
	CacheTriggerEntry *c2 = (CacheTriggerEntry *)v2;
	int event = *(int*)v1;
	TriggerEntry *tge = (TriggerEntry *)v1;
	int rc;

	rc = (tge->event > c2->trigger.event) ? 1
		 : ((tge->event < c2->trigger.event) ? -1 : 0);

	if (rc == 0)
	{
		rc = (tge->srcvendor > c2->trigger.srcvendor) ? 1
				: ((tge->srcvendor < c2->trigger.srcvendor) ? -1: 0);

		if (rc == 0)
		{
			rc = (tge->dstvendor > c2->trigger.dstvendor) ? 1
					: ((tge->dstvendor < c2->trigger.dstvendor) ? -1: 0);
		}
	}

	return rc;
}

int
CacheVpnGCmp(const void *v1, const void *v2, void *param)
{
	VpnGroupEntry *e1 = (VpnGroupEntry *)v1, *e2 = (VpnGroupEntry *)v2;

	return strcmp(e1->vpnGroup, e2->vpnGroup);
}

int
CacheIedgeEmailInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, 
					*c2 = (CacheTableInfo *)v2;

	return strcmp(c1->data.email, c2->data.email);
}

int
CacheIedgeEmailCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	return strcmp(v1, c2->data.email);
}


int
CacheIedgeH323IdInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, 
					*c2 = (CacheTableInfo *)v2;

	return strcmp(c1->data.h323id, c2->data.h323id);
}

int
CacheIedgeH323IdCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	return strcmp(v1, c2->data.h323id);
}

int
CacheIedgeTGCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	if (param == tgCache) return strcmp(v1, c2->data.tg);
	else return strcmp(v1, c2->data.dtg);
}

int
CacheIedgeTGInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, 
					*c2 = (CacheTableInfo *)v2;
	
	if (param == tgCache) return strcmp(c1->data.tg, c2->data.tg);
	else return strcmp(c1->data.dtg, c2->data.dtg);
}


int
CacheIedgeUriInsCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c1 = (CacheTableInfo *)v1, *c2 = (CacheTableInfo *)v2;

	return strncmp(c1->data.uri, c2->data.uri, SIPURL_LEN);
}

int
CacheIedgeUriCmp(const void *v1, const void *v2, void *param)
{
	CacheTableInfo *c2 = (CacheTableInfo *)v2;

	return strncmp(v1, c2->data.uri, SIPURL_LEN);	
}

int
CacheIedgeGkCmp(const void *v1, const void *v2, void *param)
{
	CacheGkInfo *c1 = (CacheGkInfo *)v1;
	CacheGkInfo *c2 = (CacheGkInfo *)v2;
	int rc;
	
	rc = memcmp(c1->regid, c2->regid, REG_ID_LEN);
	if (rc == 0)
	{
		if (c1->uport < c2->uport)
		{
			return -1;
		}
		else if (c1->uport > c2->uport)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return rc;
	}
}

int
CacheCPCmp(const void *v1, const void *v2, void *param)
{
	VpnRouteKey *r1 = (VpnRouteKey *)v1;
	RouteEntry *r2 = &((CacheRouteEntry *)v2)->routeEntry;

	return strcmp(r1->crname, r2->crname);	
}

int
CacheCPInsCmp(const void *v1, const void *v2, void *param)
{
	RouteEntry *r1 = &((CacheRouteEntry *)v1)->routeEntry, 
				*r2 = &((CacheRouteEntry *)v2)->routeEntry;

	return strcmp(r1->crname, r2->crname);	
}

int
CacheRoutesLRUInsCmp(const void *v1, const void *v2, void *param)
{
	RouteEntry *r1 = &((CacheRouteEntry *)v1)->routeEntry, 
				*r2 = &((CacheRouteEntry *)v2)->routeEntry;

	if(r1->rTime >= r2->rTime)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int
CacheCPBCmp(const void *v1, const void *v2, void *param)
{
	CallPlanBindKey *r1 = (CallPlanBindKey *)v1;
	CacheCPBEntry *r2 = (CacheCPBEntry *)v2;
	int rc;

	rc = strcmp(r1->cpname, r2->cpbEntry.cpname);
	if (rc != 0)
	{
		return rc;
	}
	
	return strcmp(r1->crname, r2->cpbEntry.crname);
}

int
CacheCPBInsCmp(const void *v1, const void *v2, void *param)
{
	CacheCPBEntry *r1 = (CacheCPBEntry *)v1,
					*r2 = (CacheCPBEntry *)v2;
	int rc;

	rc = strcmp(r1->cpbEntry.cpname, r2->cpbEntry.cpname);	
	if (rc != 0)
	{
		return rc;
	}
	
	return strcmp(r1->cpbEntry.crname, r2->cpbEntry.crname);
}

int
CacheCallCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c2 = (CallHandle *)v2;

	return memcmp(v1, c2->callID, CALL_ID_LEN);
}

int
CacheCallInsCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c1 = (CallHandle *)v1, 
		  *c2 = (CallHandle *)v2;

	return memcmp(c1->callID, c2->callID, CALL_ID_LEN);
}

int
CacheGuidCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c2 = (CallHandle *)v2;

	return memcmp(v1, c2->handle.h323CallHandle.guid, GUID_LEN);
}

int
CacheGuidInsCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c1 = (CallHandle *)v1, 
		  *c2 = (CallHandle *)v2;

	return memcmp(c1->handle.h323CallHandle.guid, c2->handle.h323CallHandle.guid, GUID_LEN);
}

int
CacheTipCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c2 = (CallHandle *)v2;
	IPPort *tip1 = (IPPort *)v1;

	if (tip1->ipaddress < CallFceTranslatedIp(c2))
	{
		return -1;
	}

	if (tip1->ipaddress > CallFceTranslatedIp(c2))
	{
		return 1;
	}

	if (tip1->port < CallFceTranslatedPort(c2))
	{
		return -1;
	}

	if (tip1->port > CallFceTranslatedPort(c2))
	{
		return 1;
	}

	return 0;
}

int
CacheTipInsCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c1 = (CallHandle *)v1, 
		  *c2 = (CallHandle *)v2;

	if (CallFceTranslatedIp(c1) < CallFceTranslatedIp(c2))
	{
		return -1;
	}

	if (CallFceTranslatedIp(c1) > CallFceTranslatedIp(c2))
	{
		return 1;
	}

	if (CallFceTranslatedPort(c1) < CallFceTranslatedPort(c2))
	{
		return -1;
	}

	if (CallFceTranslatedPort(c1) > CallFceTranslatedPort(c2))
	{
		return 1;
	}

	return 0;
}

int
CacheConfCmp(const void *v1, const void *v2, void *param)
{
	ConfHandle *c2 = (ConfHandle *)v2;

	return memcmp(v1, c2->confID, CONF_ID_LEN);
}

int
CacheConfInsCmp(const void *v1, const void *v2, void *param)
{
	ConfHandle *c1 = (ConfHandle *)v1, 
		  *c2 = (ConfHandle *)v2;

	return memcmp(c1->confID, c2->confID, CONF_ID_LEN);
}

/* COmpare a leg with a call entry */
int
CacheSipCallCmp(const void *v1, const void *v2, void *param)
{
	SipCallLegKey *k1 = (SipCallLegKey *)v1;
	CallHandle *c2 = (CallHandle *)v2;
	SipCallLegKey *k2 = &SipCallHandle(c2)->callLeg;

	return CacheCallLegCmp(k1, k2);
}

/* compare two call entries */
int
CacheSipCallInsCmp(const void *v1, const void *v2, void *param)
{
	CallHandle *c1 = (CallHandle *)v1;
	SipCallLegKey *k1 = &SipCallHandle(c1)->callLeg;
	CallHandle *c2 = (CallHandle *)v2;
	SipCallLegKey *k2 = &SipCallHandle(c2)->callLeg;

	return CacheCallLegCmp((const void *)k1, (const void *)k2);
}


int
CacheIgrpCmp(const void *v1, const void *v2, void *param)
{
	unsigned char *igrpname1= (unsigned char *)v1;
	CacheIgrpInfo  *c2 = (CacheIgrpInfo *)v2;

	return strcmp(igrpname1, c2->igrp.igrpName);	
}

int
CacheIgrpInsCmp(const void *v1, const void *v2, void *param)
{
	CacheIgrpInfo *e1 = (CacheIgrpInfo *)v1;
	CacheIgrpInfo *e2 = (CacheIgrpInfo *)v2;

	return strcmp (e1->igrp.igrpName, e2->igrp.igrpName);
}

int
CacheVnetCmp(const void *v1, const void *v2, void *param)
{
	unsigned char *vnetname1= (unsigned char *)v1;
	CacheVnetEntry  *c2 = (CacheVnetEntry *)v2;

	return strcmp(vnetname1, c2->vnet.vnetName);	
}

int
CacheVnetInsCmp(const void *v1, const void *v2, void *param)
{
	CacheVnetEntry *e1 = (CacheVnetEntry *)v1;
	CacheVnetEntry *e2 = (CacheVnetEntry *)v2;

	return strcmp (e1->vnet.vnetName, e2->vnet.vnetName);
}

int
CacheSipRegCmp(const void *v1, const void *v2, void *param)
{
	SipCallLegKey *k1 = (SipCallLegKey *)v1;
	SipCallHandle *c2 = &((SipRegistration *)v2)->sipch;
	SipCallLegKey *k2 = &c2->callLeg;

	return SipCompareUrls(k1->remote, k2->remote);
}

/* compare two call entries */
int
CacheSipRegInsCmp(const void *v1, const void *v2, void *param)
{
	SipCallHandle *c1 = &((SipRegistration *)v1)->sipch;
	SipCallLegKey *k1 = &c1->callLeg;
	SipCallHandle *c2 = &((SipRegistration *)v2)->sipch;
	SipCallLegKey *k2 = &c2->callLeg;

	return SipCompareUrls(k1->remote, k2->remote);
}

/* common fn used by call cache and tsm cache */
int
CacheCallLegCmp(const void *v1, const void *v2)
{
	SipCallLegKey *k1 = (SipCallLegKey *)v1;
	SipCallLegKey *k2 = (SipCallLegKey *)v2;
	int rc = 0;

	if (!k1->callid)
	{
		// looks like this leg is not initialized yet
		return 1;
	}

	if (!rc)
	{
		rc = strcmp(k1->callid, k2->callid);
	}

	if (!rc)
	{
		rc = SipCompareUrls(k1->local, k2->local);
	}

	if (!rc)
	{
		rc = SipCompareUrls(k1->remote, k2->remote);
	}

	return rc;
}

int
CacheTransCmp(const void *v1, const void *v2, void *param)
{
	SipTransKey *k1 = (SipTransKey *)v1;
	SipTransKey *k2 = (SipTransKey *)v2;
	int rc = 0;

	if (!rc)
	{
		rc = CacheCallLegCmp((const void *)&k1->callLeg, (const void *)&k2->callLeg);
	}

	if (!k1->method)
	{
		// unitialized
		return 1;
	}

	if (!rc)
	{
		if(strcmp(k1->method, "ACK") == 0)
		{
			rc = strcmp("INVITE", k2->method);
		}
		else if(strcmp(k2->method, "ACK") == 0)
		{
			rc = strcmp(k1->method, "INVITE");
		}
		else
		{
			rc = strcmp(k1->method, k2->method);
		}
	}

	if (!rc)
	{
		rc = (k1->cseqno < k2->cseqno)? -1:
				(k1->cseqno > k2->cseqno)? 1: 0;
	}

	if (!rc)
	{
		rc = (k1->type < k2->type)? -1:
				(k1->type > k2->type)? 1: 0;
	}

	return rc;
}

int
CacheTransInsCmp(const void *v1, const void *v2, void *param)
{
	return CacheTransCmp(v1, v2, param);
}

int
CacheTransDestCmp(const void *v1, const void *v2, void *param)
{
	char fn[] = "CacheTransDestCmp():";
	SipTransDestKey *key = (SipTransDestKey *)v1;
	unsigned int dip = *(unsigned int *)v1;
	unsigned short dport = *(unsigned short *)(v1+4);
	SipTrans *trans = (SipTrans *)v2;
	CallRealmInfo *realmInfo;
	
	// Only INVITE UACs are in the cache
	// Request context must be set then
	if (trans->request.context)
	{
		realmInfo = (CallRealmInfo *)trans->request.context->pData;
		
		if (realmInfo == NULL)
		{
			NETERROR(MCACHE, ("%s realmInfo is null!\n", fn));
			return 0;
		}
	}
	else
	{
		NETERROR(MCACHE, ("%s context is null!\n", fn));
		return 0;
	}

	// Check to see if the rsa on the transaction
	// matches the one we got
	if (realmInfo->rsa < key->srcip)
	{
		return -1;
	}

	if (realmInfo->rsa > key->srcip)
	{
		return 1;
	}

	// rsa's match !!

	if (SipTranRequestSendhost(trans) < key->destip)
		return -1;
	if (SipTranRequestSendhost(trans) > key->destip)
		return 1;
	
	if (SipTranRequestSendport(trans) < key->destport)
		return -1;
	if (SipTranRequestSendport(trans) > key->destport)
		return 1;
	
	return 0;
}

int
CacheTransDestInsCmp(const void *v1, const void *v2, void *param)
{
	char fn[] ="CacheTransDestInsCmp():";
	SipTrans *trans1 = (SipTrans *)v1;
	SipTransDestKey key;
	CallRealmInfo *realmInfo;

	if (trans1->request.context)
	{
		realmInfo = (CallRealmInfo *)trans1->request.context->pData;
		
		if (realmInfo == NULL)
		{
			NETERROR(MCACHE, ("%s realmInfo is null!\n", fn));
			return 0;
		}

		key.srcip = realmInfo->rsa;
	}
	else
	{
		NETERROR(MCACHE, ("%s context is null!\n", fn));
		return 0;
	}

	key.destip = SipTranRequestSendhost(trans1);
	key.destport = SipTranRequestSendport(trans1);

	return CacheTransDestCmp(&key, v2, param);
}

int
VpnPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "VpnPopulate()";
    VpnEntry *vpnEntry, *lastVpnEntry;
	char *vpnName;
    DB db;
	DB_tDb dbstruct;

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{	
		/* Open the database and populate */
     	dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(VPNS_DB_FILE, DB_eVpns, &dbstruct)))
		{
	    	NETERROR(MDB, ("%s Unable to open database %s\n", fn, VPNS_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

   	for (vpnEntry = DbGetFirstVpnEntry(db); vpnEntry != 0; 
 		vpnEntry = DbGetNextVpnEntry(db, vpnName, sizeof(VpnKey)), 
	 	free(lastVpnEntry))
    {
		char *entry;

	 	NETDEBUG(MCACHE, NETLOG_DEBUG4,
			("Retrieved vpn entry %s", vpnEntry->vpnId));
	 	NETDEBUG(MCACHE, NETLOG_DEBUG4,
			("\tvpn entry ext len %u\n", vpnEntry->vpnExtLen));
	 	NETDEBUG(MCACHE, NETLOG_DEBUG4,
			("\tvpn entry group \"%s\"\n", vpnEntry->vpnGroup));

		entry = (char *)CacheDupVpnEntry(vpnEntry);

		if (entry)
		{
			/* lock the cache */
			CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

			CacheInsert(vpnCache, entry);

			CacheReleaseLocks(vpnCache);	
		}

	 	vpnName = vpnEntry->vpnName;
	 	lastVpnEntry = vpnEntry;
    }

_release_db:
	if (!dbin)
	{	
		DbClose(&dbstruct);
	}

	return(0);
}

int
VpnGPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "VpnGPopulate()";
    char *vpnG;
    VpnGroupEntry *vpnGroupEntry,*lastVpnGroupEntry;
    DB db;
	DB_tDb dbstruct;

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{	
		/* Open the database and populate */
     	dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(VPNG_DB_FILE, DB_eVpnG, &dbstruct)))
		{
	    	NETERROR(MDB, ("%s Unable to open database %s\n", fn, VPNG_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

    for (vpnGroupEntry = DbGetFirstVpnGEntry(db); 
				vpnGroupEntry != 0; 
	 	vpnGroupEntry = DbGetNextVpnGEntry(db, 
				vpnG, sizeof(VpnGroupKey)), 
	 	free(lastVpnGroupEntry))
    {
		char *entry;

	 	NETDEBUG(MCACHE, NETLOG_DEBUG4,
			("Retrieved vpn entry \"%s\"\n", vpnGroupEntry->vpnGroup));

		entry = (char *)CacheDupVpnGEntry(vpnGroupEntry);

		if (entry)
		{
			/* lock the cache */
			CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);

			CacheInsert(vpnGCache, entry);

			CacheReleaseLocks(vpnGCache);	
		}

	 	vpnG = vpnGroupEntry->vpnGroup;
	 	lastVpnGroupEntry = vpnGroupEntry;
    }

_release_db:
	if (!dbin)
	{	
		DbClose(&dbstruct);
	}

	return(0);
}

int
CPPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "CPPopulate()";
    VpnRouteEntry *routeEntry,*lastRouteEntry;
    VpnRouteKey *key, *okey;
	CacheVpnRouteEntry *entry;
    int keylen = sizeof(VpnRouteKey);
    DB db;
	DB_tDb dbstruct;
	int all = 0, n = 0;

	if (m == NULL)
	{
		return -1;
	}

	/* Open the database and populate */
	if (!dbin)
	{	
     	dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(CALLROUTE_DB_FILE, DB_eCallRoute, &dbstruct)))
		{
	    	NETERROR(MDB, ("%s Unable to open database %s\n", fn, CALLROUTE_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

   	for (key = (VpnRouteKey *)DbGetFirstKey(db); key != 0; 
	 		key = (VpnRouteKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(routeEntry))
   	{
		/* Get the info entry */
	 	routeEntry = (VpnRouteEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (routeEntry == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	
	 	all ++; n++;

		entry = CacheDupVpnRouteEntry(routeEntry);

		if (entry)
		{
			/* lock the cache */
			CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

			//CacheInsert(cpCache, entry);
			if (AddRoute(entry) < 0)
			{
				NETERROR(MCACHE, ("%s Duplicate Route: %s\n", 
					fn, entry->routeEntry.crname));
				n--;
			}

			CacheReleaseLocks(cpCache);

		}
		else
		{
			n--;
		}
   	}

	NETDEBUG(MCACHE, NETLOG_DEBUG4,
		("%s %d Routes read, %d Routes added:\n", fn, all, n));
 _return:
     
	if (!dbin)
	{	
     	/* Close the database */
     	DbClose(&dbstruct);
	}

	return 0;
}

int
CPBPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "CPBPopulate()";
    CallPlanBindEntry *bindEntry,*lastBindEntry;
    CallPlanBindKey *key, *okey;
	CacheCPBEntry *entry;
    int keylen = sizeof(CallPlanBindKey);
    DB db;
	DB_tDb dbstruct;
	int all = 0;

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		/* Open the database and populate */
   		dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(CALLPLANBIND_DB_FILE, DB_eCallPlanBind, &dbstruct)))
		{
	   		NETERROR(MDB, 
				("%s Unable to open database %s\n", fn, CALLPLANBIND_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

   	for (key = (CallPlanBindKey *)DbGetFirstKey(db); key != 0; 
 		key = (CallPlanBindKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(bindEntry))
   	{
	 	/* Get the info entry */
	 	bindEntry = (CallPlanBindEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (bindEntry == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	
	 	all ++;

		entry = CacheDupCPBEntry(bindEntry);

		if (entry)
		{
			/* lock the cache */
			CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

			//CacheInsert(cpbCache, entry);
			if (AddCPB(entry) < 0)
			{
				NETERROR(MCACHE, ("%s Duplicate Binding %s/%s\n",
					fn, entry->cpbEntry.cpname, entry->cpbEntry.crname));
			}

			CacheReleaseLocks(cpbCache);
		}
   	}

 _return:
     
	if (!dbin)
	{
   		/* Close the database */
   		DbClose(&dbstruct);
	}

	return 0;
}

int
TriggerPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "TriggerPopulate()";
    TriggerKey *key, *okey;
	TriggerEntry *tgPtr;
	CacheTriggerEntry *cacheTriggerEntry;
    int keylen = sizeof(TriggerKey);
    DB db;
	DB_tDb dbstruct;
	int all = 0;

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		/* Open the database and populate */
   		dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(TRIGGER_DB_FILE, DB_eTrigger, &dbstruct)))
		{
	   		NETERROR(MDB, 
				("%s Unable to open database %s\n", fn, TRIGGER_DB_FILE));
			return -1;
		}
	}
	else
	{
		db = dbin;
	}

   	for (key = (TriggerKey *)DbGetFirstKey(db); key != 0; 
 		key = (TriggerKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(tgPtr))
   	{
	 	/* Get the info entry */
	 	tgPtr = (TriggerEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (tgPtr == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	
	 	all ++;

		if (!tgPtr->srcvendor || !tgPtr->dstvendor)
		{
			NETERROR(MCACHE, ("%s Skipping generic trigger %s\n",
				fn, tgPtr->name));
			continue;
		}

		cacheTriggerEntry = CacheDupTriggerEntry(tgPtr);

		if (cacheTriggerEntry)
		{
			/* lock the cache */
			CacheGetLocks(triggerCache, LOCK_WRITE, LOCK_BLOCK);

			CacheInsert(triggerCache, cacheTriggerEntry);

			CacheReleaseLocks(triggerCache);
		}
   	}

 _return:
     
	if (!dbin)
	{
   		/* Close the database */
   		DbClose(&dbstruct);
	}

	return 0;
}


int
VnetPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "VnetPopulate()";
    VnetKey 		*key, *okey;
	VnetEntry 		*vnetEntryPtr;
	CacheVnetEntry *cacheVnetEntry;
    int 			keylen = sizeof(VnetKey), all = 0;
    DB 				db;
	DB_tDb 			dbstruct = {0};

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		/* Open the database and populate */
   		dbstruct.read_write = GDBM_WRCREAT;

		if (!(db = DbOpenByID(VNET_DB_FILE, DB_eVnet, &dbstruct)))
		{
	   		NETERROR(MDB, 
				("%s Unable to open database %s\n", fn, VNET_DB_FILE));
			goto _return;
		}
	}
	else
	{
		db = dbin;
	}

   	for (key = (VnetKey *)DbGetFirstKey(db); key != 0; 
 		key = (VnetKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(vnetEntryPtr))
   	{
	 	/* Get the info entry */
	 	vnetEntryPtr = (VnetEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (vnetEntryPtr == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	 	all ++;
		cacheVnetEntry = CacheDupVnetEntry(vnetEntryPtr);
		//UpdateVnetDynamicInfo(&cacheVnetEntry->vnet, NULL);

		if (cacheVnetEntry)
		{
			/* lock the cache */
			CacheGetLocks(vnetCache, LOCK_WRITE, LOCK_BLOCK);

			if (AddVnet(cacheVnetEntry) < 0)
			{
				NETERROR(MCACHE, ("%s Duplicate Vnet: %s\n", 
					fn, cacheVnetEntry->vnet.vnetName));
			}

			CacheReleaseLocks(vnetCache);
		}
   	}

 _return:
	if (!dbin)
	{
   		/* Close the database */
   		DbClose(&dbstruct);
	}

	return 0;
}


int
RealmPopulate(LsMemStruct *m, DB dbin)
{
	char fn[] = "RealmPopulate()";
    RealmKey 		*key, *okey;
	RealmEntry 		*rmEntryPtr;
	CacheRealmEntry *cacheRealmEntry;
    int 			keylen = sizeof(RealmKey), all = 0;
    DB 				db;
	DB_tDb 			dbstruct = {0};

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		/* Open the database and populate */
   		dbstruct.read_write = GDBM_WRCREAT;

		if (!(db = DbOpenByID(REALM_DB_FILE, DB_eRealm, &dbstruct)))
		{
	   		NETERROR(MDB, 
				("%s Unable to open database %s\n", fn, REALM_DB_FILE));
			goto _return;
		}
	}
	else
	{
		db = dbin;
	}

   	for (key = (RealmKey *)DbGetFirstKey(db); key != 0; 
 		key = (RealmKey *)DbGetNextKey(db, (char *)key, keylen),
	      	free(okey), free(rmEntryPtr))
   	{
	 	/* Get the info entry */
	 	rmEntryPtr = (RealmEntry *)DbFindEntry(db, (char *)key, keylen);

	 	if (rmEntryPtr == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	 	all ++;
		cacheRealmEntry = CacheDupRealmEntry(rmEntryPtr);

		if (cacheRealmEntry)
		{
			/* lock the cache */
			CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

			if (AddRealm(cacheRealmEntry) < 0)
			{
				NETERROR(MCACHE, ("%s Duplicate Realm: %s\n", 
					fn, cacheRealmEntry->realm.realmName));
			}

			CacheReleaseLocks(realmCache);
		}
   	}

 _return:
	if (!dbin)
	{
   		/* Close the database */
   		DbClose(&dbstruct);
	}

	return 0;
}

int
IgrpPopulate(LsMemStruct *m, DB dbin)
{
    char fn[] = "IgrpPopulate()";
    IgrpKey         *key, *okey;
    IgrpInfo        *igrpPtr;
    CacheIgrpInfo   *igrpCacheEntry;
    CacheIgrpInfo   *cacheIgrpInfo;
    int             keylen = sizeof(IgrpKey), all = 0;
    DB              db;
    DB_tDb          dbstruct = {0};

	if (m == NULL)
	{
		return -1;
	}

	if (!dbin)
	{
		/* Open the database and populate */
   		dbstruct.read_write = GDBM_READER;

		if (!(db = DbOpenByID(IGRP_DB_FILE, DB_eIgrp, &dbstruct)))
		{
	   		NETERROR(MDB, 
				("%s Unable to open database %s\n", fn, IGRP_DB_FILE));
			goto _return;
		}
	}
	else
	{
		db = dbin;
	}


	// Check to see if a IgrpCache exists, if it does, we must
	// first update the db with runtime information from the
	// cache - like maxcallstotal, maxcallsin, maxcallsout etc.
	for (cacheIgrpInfo = CacheGetFirst(igrpCache);
		cacheIgrpInfo; 
		cacheIgrpInfo = CacheGetNext(igrpCache, &cacheIgrpInfo->igrp))
	{
		/* Get the info entry */
		igrpPtr = DbFindIgrpEntry(db, (char *)&cacheIgrpInfo->igrp, keylen);
		if (igrpPtr)
		{
			UpdateIgrpDynamicInfo(igrpPtr, &cacheIgrpInfo->igrp);

			if((DbStoreInfoEntry(db, igrpPtr, (char *)igrpPtr, keylen)) < 0)
			{
				NETERROR(MINIT, ("database store error\n"));
			}
		}
	}

    for (key = (IgrpKey *)DbGetFirstKey(db); key != 0;
        key = (IgrpKey *)DbGetNextKey(db, (char *)key, keylen),
            free(okey), free(igrpPtr))
    {
        /* Get the info entry */
        igrpPtr = (IgrpInfo *)DbFindEntry(db, (char *)key, keylen);

	 	if (igrpPtr == NULL)
	 	{
			NETERROR(MCACHE, ("%s Db Format Error !!\n", fn));
			goto _return;
	 	}

	 	okey = key;
	 	all ++;
		igrpCacheEntry = CacheDupIgrpInfo(igrpPtr);

		if (igrpCacheEntry)
		{
			/* lock the cache */
			CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

			CacheInsert(igrpCache, igrpCacheEntry);

			CacheReleaseLocks(igrpCache);
		}
   	}

 _return:
     
	if (!dbin)
	{
   		/* Close the database */
   		DbClose(&dbstruct);
	}

	return 0;
}

// Caller should print error
int
CacheAttach(void)
{
	int shmId;

	/* Attach to shared memory */
	if ((shmId = SHM_Attach(ISERVER_CACHE_INDEX, (void **)&map)) == -1)
	{
		//NETERROR(MCLI, ("Shared Memory Not Created\n"));
		map = NULL;
		return -1;
	}

	lsMem = (LsMemStruct *)map->appStruct;

	if (lsMem == NULL)
	{
		NETERROR(MCLI, ("Shared Memory not initialized properly\n"));
		return -1;
	}

	LsMemStructAtt(lsMem);

	return 1;
}

int
CacheDetach(void)
{
	if (map)
	{
		SHM_Detach(ISERVER_CACHE_INDEX, map); 
	}
	return(0);
}

int
CacheConfig(void)
{
	if (max_segs > 0)
	{
		SHM_Config(ISERVER_CACHE_INDEX)->msegs = max_segs;
		SHM_Config(ISERVER_CACHE_INDEX)->mpages =
			SHM_Config(ISERVER_CACHE_INDEX)->msegs * 256;
	}

	if (max_segsize > 0)
	{
		SHM_Config(ISERVER_CACHE_INDEX)->segSize =
			max_segsize;
		SHM_Config(ISERVER_CACHE_INDEX)->mpages =
			SHM_Config(ISERVER_CACHE_INDEX)->msegs *
				(SHM_Config(ISERVER_CACHE_INDEX)->segSize/4096);
	}		

	SHM_Config(ISERVER_CACHE_INDEX)->segaddrtype = segaddrtype;
	if (segaddrtype == 0) /* dynamic segment address */
		SHM_Config(ISERVER_CACHE_INDEX)->startAddr = 0;
	else /* Fixed address for first segment */
		SHM_Config(ISERVER_CACHE_INDEX)->startAddr = segaddr ? segaddr : (void *)ISERVER_SHM_STARTADDR;

	return 0;
}

/* return < 0 if there is an error. Generally the app
 * should exit if an error happens
 */
int
CacheInit(void)
{
	int shmId;

	CacheConfig();

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Creating the shared memory .... \n"));

	if ((shmId = SHM_Create(ISERVER_CACHE_INDEX, (void **)&map)) == -1)
	{
		NETERROR(MINIT, ("No shared memory available\n"));
		return -1;
	}

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Created the shared memory\n"));

	lsMem = (LsMemStruct *)SHM_Malloc(sizeof(LsMemStruct));
	if (lsMem == NULL)
	{
		NETERROR(MINIT, ("shm initialization error\n"));
		return -1;
	}

	map->appStruct = lsMem;

	LsMemStructInit(lsMem);

	return 1;
}

int
CacheClean(void)
{
	MemoryMap *map, *mapl;
	void *alShmStart = (void *)ISERVER_SHM_STARTADDR;
	int totalSize = 0x100000;
	int nsegs, npages;
	int key;

	key = ISERVER_SHM_KEY;

	/* Cleanup the cache */
	map = SHMAPP_UseMap(key, &alShmStart, totalSize, 
			&nsegs, &npages);
	if (map == NULL)
	{
		return 0;
	}
		
	mapl = (MemoryMap *)malloc(map->len);
	memcpy(mapl, map, map->len);
#if     0
	mapl->segs = ( SegMap * )(mapl+1);
#endif
	SHMAPP_ReleaseMap(mapl);
	free(mapl);

	return 1;
}

int
ReviewIedgeEntry(InfoEntry *info)
{
	char fn[] = "ReviewIedgeEntry():";
	time_t	timenow;
	
	time(&timenow);
	
	/* 90 minutes or earlier */
	if (info->rTime < (timenow - 5400))
	{
		return 0;
	}

	return 1;
}

int
UpdateIedgeDynamicInfo(InfoEntry *dinfo, InfoEntry *sinfo)
{
	if (sinfo)
	{
		IedgeCalls(dinfo) = IedgeCalls(sinfo);
		IedgeInCalls(dinfo) = IedgeInCalls(sinfo);
		IedgeOutCalls(dinfo) = IedgeOutCalls(sinfo);
		dinfo->cTime = sinfo->cTime;
		dinfo->iaTime = sinfo->iaTime;
		dinfo->attTime = sinfo->attTime;
		dinfo->dndTime = sinfo->dndTime;

		// Any dynamic state Information must be propagated
		dinfo->stateFlags &= ~(CL_ACTIVE|CL_DND); // clear out
		dinfo->stateFlags |= (sinfo->stateFlags & (CL_ACTIVE|CL_DND));
	}
	else
	{
		IedgeCalls(dinfo) = 0;
		IedgeInCalls(dinfo) = 0;
		IedgeOutCalls(dinfo) = 0;
		dinfo->cTime = 0;
		dinfo->iaTime = 0;
		dinfo->attTime = 0;
		dinfo->dndTime = 0;
	}
	
	return 0;
}

void
ResetIedgeDbFields(CacheTableInfo *cacheInfo, unsigned short startup)
{
	if (IsSGatekeeper(&cacheInfo->data))
	{
		if (startup)
		{
			cacheInfo->data.stateFlags &= ~CL_ACTIVE; 
		}
		
		// backwards compatibility.
		cacheInfo->data.stateFlags &= ~CL_STATIC;
	}
}

// Reset the cache fields from the database which shouldn't have persistence
int
ResetIedgeFields(CacheTableInfo *cacheInfo, unsigned short startup)
{
	ResetIedgeDbFields(cacheInfo, startup);

	/* Reset the CL_REGISTERED bit in the cache only... */
	cacheInfo->data.stateFlags &= ~CL_REGISTERED;
		  
	if ((cacheInfo->data.stateFlags & CL_ACTIVE) && 
		!ReviewIedgeEntry(&cacheInfo->data))
	{
		cacheInfo->data.stateFlags &= ~CL_ACTIVE;
	}

	// reset ua registrations
	cacheInfo->data.stateFlags &= ~CL_UAREGSM;

	return 0;
}

int
UpdateIgrpDynamicInfo(IgrpInfo *dinfo, IgrpInfo *sinfo)
{
	if (sinfo)
	{
		IgrpInCalls(dinfo) = IgrpInCalls(sinfo);
		IgrpOutCalls(dinfo) = IgrpOutCalls(sinfo);
		IgrpCallsTotal(dinfo) = IgrpCallsTotal(sinfo);
		dinfo->dndTime = sinfo->dndTime;
	}
	else
	{
		IgrpInCalls(dinfo) = 0;
		IgrpOutCalls(dinfo) = 0;
		IgrpCallsTotal(dinfo) = 0;
		dinfo->dndTime = 0;
	}
	
	return 0;
}

int
UpdateVnetDynamicInfo(VnetEntry *dinfo, VnetEntry *sinfo)
{
	if (sinfo)
	{
		dinfo->rtgTblId = sinfo->rtgTblId;
	}
	else
	{
		dinfo->rtgTblId = 0;
	}
	
	return 0;
}

/* return if src < dst or src > dest or src = dest */
int 
SipCompareUrls(header_url *src, header_url *dst)
{
	/* Routine to compare two URLs. return 0 if they are not, 1 if they are */
	int rc = 0;

	if (!rc && src && dst)
	{
		/* Match URLs */
		if (!rc && src->name && dst->name)
		{
			rc = strcmp(src->name, dst->name);
		}

		if (!rc && src->host && dst->host)
		{
			/* Case Insensitive comparison */
			rc = strcasecmp(src->host, dst->host);
		}

		if (!rc && src->tag && dst->tag)
		{
			rc = strcmp(src->tag, dst->tag);
		}

		if (!rc && src->port && dst->port)
		{
			/* ports must be assigned in the urls to at
			 * least 5060.
			 */
			rc = (src->port < dst->port)? -1: 
					(src->port > dst->port)? 1: 0;
		}
	}

	return rc;
}

void *
CacheIedgePhoneData2Key(void *data)
{
	char *c = (char *)data;

	if (c[0] == '\0') return "%NULL%";
	else return c;
}

void *
CacheIedgePhoneInsData2Key(void *data)
{
	CacheTableInfo *c = (CacheTableInfo *)data;

	if (c->data.phone[0] == '\0') return "%NULL%";
	else return c->data.phone;
}

void *
CacheCPData2Key(void *data)
{
	char *c = (char *)data;

	if (c[0] == '\0') return "%NULL%";
	else return c;
}

void *
CacheCPInsData2Key(void *data)
{
	CacheRouteEntry *c = (CacheRouteEntry *)data;

	if (c->routeEntry.dest[0] == '\0') return "%NULL%";
	else return c->routeEntry.dest;
}

void *
CacheCPSrcInsData2Key(void *data)
{
	CacheRouteEntry *c = (CacheRouteEntry *)data;

	if (c->routeEntry.src[0] == '\0') return "%NULL%";
	else return c->routeEntry.src;
}

void *
CacheCPBCRData2Key(void *data)
{
	char *c = (char *)data;

	if (c[0] == '\0') return "%NULL%";
	else return c;
}

void *
CacheCPBCRInsData2Key(void *data)
{
	CacheCPBEntry *c = (CacheCPBEntry *)data;

	if (c->cpbEntry.crname[0] == '\0') return "%NULL%";
	else return c->cpbEntry.crname;
}

void *
CacheCPBCPData2Key(void *data)
{
	char *c = (char *)data;

	if (c[0] == '\0') return "%NULL%";
	else return c;
}

void *
CacheCPBCPInsData2Key(void *data)
{
	CacheCPBEntry *c = (CacheCPBEntry *)data;

	if (c->cpbEntry.cpname[0] == '\0') return "%NULL%";
	else return c->cpbEntry.cpname;
}

void *
CacheIedgeGwCPData2Key(void *data)
{
	char *c = (char *)data;

	if (c[0] == '\0') return "%NULL%";
	else return c;
}

void *
CacheIedgeGwCPInsData2Key(void *data)
{
	CacheTableInfo *info = (CacheTableInfo *)data;
	
	if (info->data.cpname[0] == '\0') return "%NULL%";
	else return info->data.cpname;
}

int
InitCfgParms(CfgParms *cfgParms)
{
	if (cfgParms)
	{
		cfgParms->allowSrcAll = allowSrcAll;
		cfgParms->allowDestAll = allowDestAll;

		NETDEBUG(MINIT, NETLOG_DEBUG4, 
			("allowSrcAll = %d, allowDestCall=%d\n", 
			cfgParms->allowSrcAll, cfgParms->allowDestAll));

		cfgParms->cacheTimeout = cacheTimeout;
		cfgParms->routeDebug = routeDebug;
		cfgParms->nh323Instances = nh323Instances;
		cfgParms->maxHunts = maxHunts;
		cfgParms->crids = crids;
		cfgParms->HistDbSize = histdb_size;
		cfgParms->RSDConfig = RSDConfig;

		cfgParms->allowHairPin = allowHairPin;
	}
	return(0);
}

int
InitCfgFromCfgParms(CfgParms *cfgParms)
{
		allowSrcAll = cfgParms->allowSrcAll;
		allowDestAll = cfgParms->allowDestAll;
		cacheTimeout = cfgParms->cacheTimeout;
		routeDebug = cfgParms->routeDebug;
		nh323Instances = cfgParms->nh323Instances;
		maxHunts = cfgParms->maxHunts;
		crids = cfgParms->crids;
		histdb_size = cfgParms->HistDbSize;
		RSDConfig = cfgParms->RSDConfig;

		allowHairPin = cfgParms->allowHairPin;

		return(0);
}

void allocCallStats(void)
{
    CallStats *pCallStats;
    IntervalStats *pIntervalStats;

    pCallStats = SHM_Malloc(sizeof(CallStats));

    pCallStats->secStat = SHM_Malloc(60*sizeof(IntervalStats));
    pCallStats->minStat = SHM_Malloc(60*sizeof(IntervalStats));
    pCallStats->hourStat = SHM_Malloc(24*sizeof(IntervalStats));

    lsMem->callStats = pCallStats;
}
