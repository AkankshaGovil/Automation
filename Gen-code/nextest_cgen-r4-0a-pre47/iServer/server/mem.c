/*
 * File for manipulating the netoid cache table
 * which resides in shared memory
 * Note that this is not a general purpose
 * memory management library.
 * The cache table resides in the top portion of the 
 * shared memory, while the lower portion consists of
 * a huge free list.
 */

/* The creator firt calls the MemCacheCreate
 * function. After this any process which wants
 * to attach calls the MemCacheAttach Function.
 * The following parameters must be provided:
 * the place where we want the start address of the cache table
 */
#include "mem.h"
#include "alerror.h"
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "md5.h"
#include "srvrlog.h"

#define USE_MEMMGR
#include "shm.h"
#include "shmapp.h"
#include "serverp.h"
#include <malloc.h>

static int backOffset = 0;

int
ShmAttached()
{
        return SHM_IsAttached(ISERVER_CACHE_INDEX);
}

int
ShmSetReady()
{
        return SHM_MakeReady(ISERVER_CACHE_INDEX);
}

int
PrintLibMap(void *addr)
{
	MemoryMap *map = (MemoryMap *)((char *)addr-backOffset);
	PrintMap(map);
	return(0);
}

/* Locking Functions */
/*
 * Right now, we just use a mutex to lock/unlock,
 * But if the # of readers increase, then we may need 
 * to synchronize with R/W locks which alternate priority
 * between readers and writers
 */

/* GetRwLock: R/W locks can be acquired in read mode or
 * write mode (mode argument), flags will tell whether
 * we want to block on the r/w lock or not
 */
#if 0
AlStatus
MemGetRwLock(MemRwLock *lock, uint mode, uint flags)
{
     char fn[] = "MemGetRwLock():";
     int error;
     /* Right now, the mode argument is unused */
     
     switch (flags)
     {
     case LOCK_BLOCK:
	  error = pthread_mutex_lock(lock);
	  break;
     case LOCK_TRY:
	  error = pthread_mutex_trylock(lock);
	  break;
     default:
	  ERROR(MSHM, ("%s Invalid flags 0x%x\n", fn, flags));
     }

     switch (error)
     {
     case 0:
	  return AL_OK;
     case EBUSY:
	  return AL_LOCKBUSY;
     default:
	  NETERROR( MSHM, ("%s lock error %d\n", fn, errno ));
	  return AL_GENERROR;
     }
}
#endif

AlStatus
MemReleaseRwLock(MemRwLock *lock)
{
     char fn[] = "MemReleaseRwLock():";
     int error;
     /* Right now, the mode argument is unused */
     
     if (pthread_mutex_unlock(lock) == 0)
     {
	  return AL_OK;
     }
     else
     {
	   ERROR(MSHM, ("%s locking error %d\n", fn, errno));
	   return AL_GENERROR;
     }
}

CacheTableInfo *
CacheDupInfoEntry(NetoidInfoEntry *info)
{
	char fn[] = "CacheDupInfoEntry():";
	CacheTableInfo *cacheInfo;

	cacheInfo = GetFreeCacheInfo();

	if (cacheInfo == 0)
    {
		if (lsMem)
		{
        	ALARM(MCACHE,
                 ("%s Not enough memory. Please increase the RAM size\n",
                 fn));
		}

	    return 0;
    }

    InitCacheInfo(cacheInfo, info);

	return cacheInfo;
}

CacheVpnEntry *
CacheDupVpnEntry(VpnEntry *in)
{
	char fn[] = "CacheDupVpnEntry():";
	CacheVpnEntry *cacheVpnEntry;

	cacheVpnEntry = GetFreeVpnEntry();

    if (cacheVpnEntry == 0)
    {
		if (lsMem)
		{
        	ALARM(MCACHE,
                 ("%s Not enough memory. Please increase the RAM size\n",
                 fn));
		}

	    return 0;
    }

	memcpy(&cacheVpnEntry->vpnEntry, in, sizeof(VpnEntry));

	return cacheVpnEntry;
}

CacheVpnGEntry *
CacheDupVpnGEntry(VpnGroupEntry *in)
{
	char fn[] = "CacheDupVpnGEntry():";
	CacheVpnGEntry *cacheVpnGEntry;

	cacheVpnGEntry = GetFreeVpnGEntry();

    if (cacheVpnGEntry == 0)
    {
		if (lsMem)
		{
             ALARM(MCACHE,
                 ("%s Not enough memory. Please increase the RAM size\n",
                 fn));
		}

	    return 0;
    }

	memcpy(&cacheVpnGEntry->vpnGroupEntry, in, sizeof(VpnGroupEntry));

	return cacheVpnGEntry;
}

CacheVpnRouteEntry *
CacheDupVpnRouteEntry(VpnRouteEntry *in)
{
	char fn[] = "CacheDupVpnRouteEntry():";
	CacheVpnRouteEntry *cacheRouteEntry;

	cacheRouteEntry = GetFreeVpnRouteEntry();

    if (cacheRouteEntry == 0)
    {
		if (lsMem)
		{
             ALARM(MCACHE,
                 ("%s Not enough memory. Please increase the RAM size\n",
                 fn));
		}

	    return 0;
    }

	memcpy(&cacheRouteEntry->routeEntry, in, sizeof(VpnRouteEntry));

	return cacheRouteEntry;
}

CacheCPEntry *
CacheDupCPEntry(CallPlanEntry *in)
{
	char fn[] = "CacheDupCpEntry():";
	CacheCPEntry *cacheCPEntry;

	cacheCPEntry = GetFreeCPEntry();

	if (cacheCPEntry == 0)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}

	    return 0;
	}

	memcpy(&cacheCPEntry->cpEntry, in, sizeof(CallPlanEntry));

	return cacheCPEntry;
}

CacheCPBEntry *
CacheDupCPBEntry(CallPlanBindEntry *in)
{
	char fn[] = "CacheDupCPBEntry():";
	CacheCPBEntry *cacheCPBEntry;

	cacheCPBEntry = GetFreeCPBEntry();

	if (cacheCPBEntry == 0)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}

	    return 0;
	}

	memcpy(&cacheCPBEntry->cpbEntry, in, sizeof(CallPlanBindEntry));

	return cacheCPBEntry;
}

CacheTriggerEntry *
CacheDupTriggerEntry(TriggerEntry *in)
{
	char fn[] = "CacheDupTriggerEntry():";
	CacheTriggerEntry *cacheTriggerEntry;

	cacheTriggerEntry = GetFreeTriggerEntry();

	if (cacheTriggerEntry == 0)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}

	    return 0;
	}

	memcpy(&cacheTriggerEntry->trigger, in, sizeof(TriggerEntry));

	return cacheTriggerEntry;
}

CacheRealmEntry *
CacheDupRealmEntry(RealmEntry *in)
{
	char 			fn[] = "CacheDupRealmEntry():";
	CacheRealmEntry *cacheRealmEntry;

	cacheRealmEntry = GetFreeRealmEntry();
	if (cacheRealmEntry == NULL)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}
	    return NULL;
	}
	memcpy(&cacheRealmEntry->realm, in, sizeof(RealmEntry));
	return cacheRealmEntry;
}

CacheIgrpInfo *
CacheDupIgrpInfo(IgrpInfo *in)
{
	char 			fn[] = "CacheDupIgrpInfo():";
	CacheIgrpInfo	*igrpCacheEntry;

	igrpCacheEntry = GetFreeIgrpInfo();
	if (igrpCacheEntry == NULL)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}
	    return NULL;
	}
	memcpy(&igrpCacheEntry->igrp, in, sizeof(IgrpInfo));
	return igrpCacheEntry;
}

CacheVnetEntry *
CacheDupVnetEntry(VnetEntry *in)
{
	char 			fn[] = "CacheDupVnetEntry():";
	CacheVnetEntry	*vnetCacheEntry;

	vnetCacheEntry = GetFreeVnetEntry();
	if (vnetCacheEntry == NULL)
	{
		if (lsMem)
		{
			ALARM(MCACHE,
			   ("%s Not enough memory. Please increase the RAM size\n",
				fn));
		}
	    return NULL;
	}
	memcpy(&vnetCacheEntry->vnet, in, sizeof(VnetEntry));
	return vnetCacheEntry;
}

/*
 * Gets an entry from the free list, if there is one
 */
CacheTableInfo *
GetFreeCacheInfo()
{
    CacheTableInfo *info = 0;

	info = SHM_Malloc(sizeof(CacheTableInfo));
	if (info)
	{
		memset(info, 0, sizeof(CacheTableInfo));
	}
	return info;
}

CacheVpnEntry *
GetFreeVpnEntry()
{
	CacheVpnEntry *vpnEntry = 0;

	vpnEntry = SHM_Malloc(sizeof(CacheVpnEntry));
	if (vpnEntry)
	{
		memset(vpnEntry, 0, sizeof(CacheVpnEntry));
	}
	return vpnEntry;
}

CacheVpnGEntry *
GetFreeVpnGEntry()
{
	CacheVpnGEntry *vpnGroupEntry = 0;

	vpnGroupEntry = SHM_Malloc(sizeof(CacheVpnGEntry));
	if (vpnGroupEntry)
	{
		memset(vpnGroupEntry, 0, sizeof(CacheVpnGEntry));
	}
	return vpnGroupEntry;
}

CacheVpnRouteEntry *
GetFreeVpnRouteEntry()
{
	CacheVpnRouteEntry *cacheRouteEntry;

	cacheRouteEntry = SHM_Malloc(sizeof(CacheVpnRouteEntry));
	if (cacheRouteEntry)
	{
		memset(cacheRouteEntry, 0, sizeof(CacheVpnRouteEntry));
	}
	return cacheRouteEntry;
}

CacheCPEntry *
GetFreeCPEntry()
{
	CacheCPEntry *cacheCPEntry;

	cacheCPEntry = SHM_Malloc(sizeof(CacheCPEntry));
	if (cacheCPEntry)
	{
		memset(cacheCPEntry, 0, sizeof(CacheCPEntry));
	}
	return cacheCPEntry;
}

CacheCPBEntry *
GetFreeCPBEntry()
{
	CacheCPBEntry *cacheCPBEntry;

	cacheCPBEntry = SHM_Malloc(sizeof(CacheCPBEntry));
	if (cacheCPBEntry) 
	{
		memset(cacheCPBEntry, 0, sizeof(CacheCPBEntry));
	}
	return cacheCPBEntry;
}

CacheTriggerEntry *
GetFreeTriggerEntry()
{
	CacheTriggerEntry *cacheTriggerEntry;

	cacheTriggerEntry = SHM_Malloc(sizeof(CacheTriggerEntry));
	if (cacheTriggerEntry)
	{
		memset(cacheTriggerEntry, 0, sizeof(CacheTriggerEntry));
	}
	return cacheTriggerEntry;
}

CacheRealmEntry *
GetFreeRealmEntry()
{
	CacheRealmEntry	*cacheRealmEntry;

	cacheRealmEntry = MMalloc(realmCache->malloc, sizeof(CacheRealmEntry));
	if (cacheRealmEntry)
	{
		memset(cacheRealmEntry, 0, sizeof(CacheRealmEntry));
	}
	return cacheRealmEntry;
}

CacheVnetEntry *
GetFreeVnetEntry()
{
	CacheVnetEntry	*cacheVnetEntry;

	cacheVnetEntry = SHM_Malloc(sizeof(CacheVnetEntry));
	if (cacheVnetEntry)
	{
		memset(cacheVnetEntry, 0, sizeof(CacheVnetEntry));
	}
	return cacheVnetEntry;
}

CacheIgrpInfo *
GetFreeIgrpInfo()
{
	CacheIgrpInfo *m;

	m = SHM_Malloc(sizeof(CacheIgrpInfo));

	if (m)
	{
		memset(m, 0, sizeof(CacheIgrpInfo));
	}
	return m;
}

int
InitCacheInfo(CacheTableInfo *cacheInfo, InfoEntry *info)
{
     memcpy(&cacheInfo->data, info, sizeof(InfoEntry));

     strncpy(cacheInfo->data.phone, info->phone, PHONE_NUM_LEN);
     strncpy(cacheInfo->data.vpnPhone, info->vpnPhone, PHONE_NUM_LEN);

     cacheInfo->prev = cacheInfo->next = cacheInfo;
     cacheInfo->head = 0;

	 return(0);
}
