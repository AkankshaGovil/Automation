#include "cli.h"
#include "serverp.h"
#include "cacheinit.h"
#include "callutils.h"
#include "licenseIf.h"

extern int checkMapReady;

int
HandleCacheCreate(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCacheCreate():";
	int rc = xleOk;

	// Create the cache, if it doent already exist
	if (CacheAttach() < 0)
	{
		checkMapReady = 0;

		if (CacheInit() < 0)
		{
	 		CLIPRINTF((stdout, "cache initialization failed\n"));
			rc = -xleOpNoPerm;
		}
		else
		{
			// Cache is useless if not populated
			LsMemPopulate(lsMem);

			// set it ready
			ShmSetReady();

			CacheDetach();
		}
	}
	else
	{
	 	CLIPRINTF((stdout, "cache already exists, and must be cleaned\n"));

		CacheDetach();

		rc = -xleOpNoPerm;
	}

	checkMapReady = 1;

	return rc;
}

int
CachePurge(char *cachename)
{
	char fn[] = "CachePurge():";
	int rc = xleOk;

	if (!strcmp(cachename, "iedge"))
	{
		// lock the cache
		CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

		IedgeCacheDestroyData(lsMem);
		IedgeCacheInstantiate(lsMem);
	
		if (lsMem)
		{
			nlm_setconfigport(0);
			nlm_initConfigPort();
		}

		CacheReleaseLocks(regCache);
	}

	if (!strcmp(cachename, "vpn"))
	{
		CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);

		VpnCacheDestroyData(lsMem);
		VpnCacheInstantiate(lsMem);
	
		CacheReleaseLocks(vpnCache);
	}

	if (!strcmp(cachename, "vpng"))
	{
		CacheGetLocks(vpnGCache, LOCK_WRITE, LOCK_BLOCK);
		VpnGCacheDestroyData(lsMem);
		VpnGCacheInstantiate(lsMem);
	
		CacheReleaseLocks(vpnGCache);
	}

	if (!strcmp(cachename, "cp"))
	{
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

		CPCacheDestroyData(lsMem);
		CPCacheInstantiate(lsMem);
	
		CacheReleaseLocks(cpCache);
	}

	if (!strcmp(cachename, "cpb"))
	{
		CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

		CPBCacheDestroyData(lsMem);
		CPBCacheInstantiate(lsMem);
	
		CacheReleaseLocks(cpbCache);
	}

    if (!strcmp(cachename, "realm"))
    {
        CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

        RealmCacheDestroyData(lsMem);
        RealmCacheInstantiate(lsMem);

        CacheReleaseLocks(realmCache);
    }

    if (!strcmp(cachename, "igrp"))
    {
        CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);

        IgrpCacheDestroyData(lsMem);
        IgrpCacheInstantiate(lsMem);

        CacheReleaseLocks(igrpCache);
    }
	return rc;
}

int
HandleCacheClean(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleCacheClean():";
	MemoryMap  *mapl;
	void *alShmStart = (void *)ISERVER_SHM_STARTADDR;
	int totalSize = 0x100000;
	int nsegs, npages;
	int key = ISERVER_SHM_KEY;

	DbDeleteLocks ();

	IpcEnd ();

	if (map == NULL)
	{
		return xleOk;
	}
		
	mapl = (MemoryMap *)malloc(map->len);
	memcpy(mapl, map, map->len);
	/* 	mapl->segs = ( SegMap * )(mapl+1);*/
	SHMAPP_ReleaseMap(mapl);
	free(mapl);

	return xleOk;
}
