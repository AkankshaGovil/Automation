#include <unistd.h>
#include "cli.h"
#include "serverp.h"
#include "rs.h"
#include "lsconfig.h"
#include "log.h"

int
HandleGkReg(Command *comm, int argc, char **argv)
{
	char fn[] = "HandleGkReg():";
   	char *regid;
    unsigned int uport, rc = xleOk;
	CacheTableInfo *cacheInfo, *tmpCacheInfo;
	CacheGkInfo *cacheGkInfo, **gkInfoPtr = 0, 
		**gkInfoNextPtr, **gkInfoPrevPtr;
	NetoidSNKey snkey;
	void *addrs[2];
	int regstatus;
	char str[24];
	CacheEntry *centryList = NULL;

	// This command can only run when the MSW is up
	if (CacheAttach() < 0)
	{
		CLIPRINTF((stdout, "Unable to attach to GIS cache, skipping command\n"));
		return -xleOpNoPerm;
	}

	// Extract the regid and uport 
    if (argc <= 2)
    {
		/* Here we prompt the user for the rest of the 
	  	* information
	  	*/
		
	 	NetoidEditHelp(comm, argc, argv);
	 	return -xleInsuffArgs;
    }

    /* Registration id */
    regid = argv[0];
	uport = atoi(argv[1]);

	memset(&snkey, 0, sizeof(snkey));
	nx_strlcpy(snkey.regid, regid, REG_ID_LEN);
	snkey.uport = uport;

	// Extract the entry
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	cacheGkInfo = CacheGet(gkCache, &snkey);

	if (cacheGkInfo == NULL)
	{
		CLIPRINTF((stdout,
			"%s Could not locate gkcache entry for gk %s/%d\n", 
			fn, snkey.regid, snkey.uport));
		rc = -xleNoEntry;
		goto _error;
	}
	
	cacheInfo = CacheGet(regCache, &snkey);

	if (cacheInfo == NULL)
	{
		CLIPRINTF((stdout,
			"%s Could not locate cache entry for gk %s/%d\n", 
			fn, snkey.regid, snkey.uport));
		rc = -xleNoEntry;
		goto _error;
	}
	
	regstatus = cacheInfo->data.stateFlags & CL_REGISTERED;

	addrs[0] = cacheInfo;
	addrs[1] = cacheGkInfo;

	// We must make sure that only one Alternate Gk is active
	// at a given time
	GetAttrPairs(comm->name, &argc, &argv, addrs, CLI_GET_ATTR_GK);

	if (regstatus != (cacheInfo->data.stateFlags & CL_REGISTERED))
	{
		gkInfoPtr = (CacheGkInfo **)CacheGetFast(gkCache, &snkey);
	
		if (gkInfoPtr == NULL)
		{
			// This is a fatal error
			CLIPRINTF((stdout, "%s GetFast failed for %s/%d\n", 
				fn, snkey.regid, snkey.uport));
			return -xleUndefined;
		}

		if (regstatus)
		{
			// previously registered, now unregistered
			// Delete the ip address from the IP cache
			DeleteIedgeIpAddr(ipCache, &cacheInfo->data);
		}
		else
		{
			// previously unregistered, now registered
			// look at all previous entries in the list and take them out
			gkInfoNextPtr = (CacheGkInfo **)CacheGetNextFast(gkCache, 
					(void **)gkInfoPtr); 

			while (gkInfoNextPtr &&
					!memcmp((*gkInfoPtr)->regid, (*gkInfoNextPtr)->regid, REG_ID_LEN))
			{
				(*gkInfoNextPtr)->regState = GKREG_TRYALT;

				tmpCacheInfo = CacheGet(regCache, (*gkInfoNextPtr)->regid);

				if (tmpCacheInfo)
				{
					tmpCacheInfo->data.stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);

					// from gk/rrq.c
					time(&tmpCacheInfo->data.iaTime);

					// Delete the ip address from the IP cache
					DeleteIedgeIpAddr(ipCache, &tmpCacheInfo->data);

					// Save into the database
					CliDbQueueEntry(&centryList, CACHE_INFO_ENTRY, 
						&tmpCacheInfo->data, sizeof(NetoidSNKey));
				}

				gkInfoNextPtr = (CacheGkInfo **)CacheGetNextFast(gkCache, 
					(void **)gkInfoNextPtr); 
			}

			gkInfoPrevPtr = (CacheGkInfo **)CacheGetPrevFast(gkCache, 
								(void **)gkInfoPtr);

			while (gkInfoPrevPtr &&
					!memcmp((*gkInfoPtr)->regid, (*gkInfoPrevPtr)->regid, REG_ID_LEN))
			{
				(*gkInfoPrevPtr)->regState = GKREG_TRYALT;

				// Delete the ip address from the IP cache
				tmpCacheInfo = CacheGet(regCache, (*gkInfoPrevPtr)->regid);

				if (tmpCacheInfo)
				{
					tmpCacheInfo->data.stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);

					// from gk/rrq.c
					time(&tmpCacheInfo->data.iaTime);

					// Delete the ip address from the IP cache
					DeleteIedgeIpAddr(ipCache, &tmpCacheInfo->data);

					// Save into the database
					CliDbQueueEntry(&centryList, CACHE_INFO_ENTRY, 
						&tmpCacheInfo->data, sizeof(NetoidSNKey));
				}

				gkInfoPrevPtr = (CacheGkInfo **)CacheGetPrevFast(gkCache, 
								(void **)gkInfoPrevPtr);
			}

			// Add the ipaddress in the cache for this uport
			if (AddIedgeIpAddr(ipCache, cacheInfo) < 0)
			{
				NETERROR(MRRQ, ("%s Unable to add ip address %s in the cache\n",
					fn, FormatIpAddress(cacheInfo->data.ipaddress.l, str)));
			}
		}
	}

	// Save into the database
	CliDbQueueEntry(&centryList, CACHE_INFO_ENTRY, 
			&cacheInfo->data, sizeof(NetoidSNKey));

_error:
	CacheReleaseLocks(regCache);

	// Complete the save operation
	CliDbQueueSave(centryList);

	CacheDetach();

	return rc;
}

void
GkStorePair(void *addrs[], char *f, char *v)
{
	CacheTableInfo *cacheInfo = addrs[0];
	CacheGkInfo *cacheGkInfo = addrs[1];

    if (strcmp(f, "epid") == 0)
	{
		cacheGkInfo->endpointIDLen = 
			chr2hex(v, cacheGkInfo->endpointIDString);
	}
	else if (strcmp(f, "reg") == 0)
	{
		if (!strcmp(v, "active"))
		{
			cacheInfo->data.stateFlags |= CL_ACTIVE;
			cacheInfo->data.stateFlags |= CL_REGISTERED;
			time(&cacheInfo->data.rTime);	

			cacheGkInfo->regState = GKREG_REGISTERED;
		}
		else
		{
			cacheInfo->data.stateFlags &= ~(CL_ACTIVE|CL_REGISTERED);

			// from gk/rrq.c
			time(&cacheInfo->data.iaTime);
			cacheGkInfo->regState = GKREG_TRYALT;
			cacheGkInfo->nextEvent = time(NULL);
		}
	}
	else if (strcmp(f, "ttl") == 0)
	{
		cacheGkInfo->regttl = atoi(v);
	}
	else if (strcmp(f, "flags") == 0)
	{
		cacheGkInfo->flags = atoi(v);
	}
}
