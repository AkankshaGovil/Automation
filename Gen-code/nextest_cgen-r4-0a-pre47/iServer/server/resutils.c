#include <stdio.h>
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "mem.h"
#include "ipcerror.h"
#include "gw.h"
#include "lsconfig.h"
#include "nxosd.h"
#ifdef NORESLOG
#include "nodebuglog.h"
#endif
#include <stdlib.h>
#include "resutils.h"
#include <sys/types.h>
#include <sys/stat.h>

// List containing information about ANI files. 
// It is keyed on filename.
List 		ANIFileList;	

// Comparison function for FileInfo structures
static int FileInfoCmp(void *item, void *key);

static FILE * GetANIFile(char *filename);
static int GetNumberFromANIFile(char *phonenum, char *filename, FILE *fileptr);
static FileInfo * AddToANIFileList(char *filename);

/*
 * If there is a match, return the length of match.
 * If there is a reject, return the length of reject.
 * Reject all is retrned as 0 length.  * If the route has no effect return -1.
 * Note that if there is no affect, the function WILL
 * NOT alter any of the input parameters. So the user
 * must initialize them properly.
 */
int
CPMatchPattern(
	 char *inpattern,
	 char **outpattern,
	 int *hasMatch,
	 int *hasRejectAll,
	 int *hasReject,
	 char *phone
)
{
	 char fn[] = "CPMatchPattern():";
	 int reject = 0;
	 int rlen = 0, rc = -1;
	 char *dest;

	 rlen = CPAnalyzePattern(inpattern, &reject, &dest);

	 if (reject)
	 {
		  if (!rlen)
		  {
				/* Its a reject all for this call */
				*hasRejectAll = 1;
				rc = rlen;
		  }
		  else
		  {
			   /* Its a specific reject route */
				if (!strncmp(dest, phone, rlen))
				{
					 /* Its a specific reject */
					 *hasReject = 1;
					 rc = rlen;
				}
		  }
	 }
	 else	/* not a reject route */
	 {
		if (!strcmp(phone, "*"))
		{
			*hasMatch = 1;
			rc = rlen;
		}
		else if (!strncmp(dest, phone, rlen))
		{
			*hasMatch = 1;
			rc = rlen;

			if (!strcmp(inpattern, "*"))
			{
				rc = strlen(phone);
			}
		}
	 }

	 *outpattern = dest;

	 return rc;
}

/* Analyze the destination in a CP,
 * and report information back, and the place where
 * the actual destination prefix starts.
 * return the length of the remaining prefix,
 * which can be zero.
 * This function should never return < 0
 */
int
CPAnalyzePattern(
	 char *dest,
	 int *reject, 
	 char **rdest
)
{
	 char fn[] = "CPAnalyzePattern():";
	 int len;

	 if (dest == NULL)
	 {
		  *rdest = NULL;
		  return 0;
	 }

	 *rdest = dest;
	 len = strlen(dest);

	 if (len == 0)
	 {
		  return 0;
	 }

	 /* reject */
	 if (!strncmp(*rdest, "-", 1))
	 {
		  *reject = 1;
		  (*rdest)++;
	 }

	 if (!strncmp(*rdest, "*", 1))
	 {
		  (*rdest)++;
	 }

	 return (len-(*rdest-dest));
}

/* Initialize set to { -1,-1,-1... }
 * GwCompare returs when comparison is either > or <.
 * It proceeds to the next level if its ==
 * generally a higher value for each of these parameters
 * indicates a higher preference
 */
/* return -1 if g1 < g2, 0 if g1=g2, 1 if g1 > g2,
 * where '>' means preference
 */
int
GwCompare(struct gw_match_set *g1, struct gw_match_set *g2, int *reason)
{
        /* Now we need to evaluate the new set with the
         * old set.
         */
	 
	// We are trying to find out if g1 > g2 (return 1)
	// or g1 < g2 (return -1)
	// All cases for (return 1) are written before
	// cases of (return -1), for simplicity

	if (g1->routePriority > g2->routePriority)
	{
		*reason = nextonePreferPriority;
		return 1;
	}

	if (g1->routePriority < g2->routePriority)
	{
		*reason = nextoneInferiorPriority;
		return -1;
	}

    if (g1->cpVal > g2->cpVal)
    {
		*reason = nextonePreferCallingPlan;
        return 1;
    }

    if (g1->cpVal < g2->cpVal)
    {
		*reason = nextoneInferiorCallingPlan;
        return -1;
    }

	if (g1->gwPriority > g2->gwPriority)
	{
		*reason = nextonePreferPriority;
		return 1;
	}

	if (g1->gwPriority < g2->gwPriority)
	{
		*reason = nextoneInferiorPriority;
		return -1;
	}

	// If the old gw route is sticky, keep the old gw route
	if (g2->route.crflags & CRF_STICKY)
	{
		*reason = nextoneStickyExists;
		return -1;
	}

	// If the new gw route is sticky, replace the old gw route
	if (g1->route.crflags & CRF_STICKY)
	{
		*reason = nextonePreferSticky;
		return 1;
	}

	// If the old gw is sticky, keep the old gw
	if (g2->gwFlags & CL_STICKY)
	{
		*reason = nextoneStickyExists;
		return -1;
	}

	// If the new gw is sticky, replace the old gw
	if (g1->gwFlags & CL_STICKY)
	{
		*reason = nextonePreferSticky;
		return 1;
	}

	if (g1->utilization < g2->utilization)
	{
		*reason = nextoneLowerUtilz;
		return 1;
	}

	if (g1->utilization > g2->utilization)
	{
		*reason = nextoneHigherUtilz;
		return -1;
	}

	if (g1->cTime < g2->cTime)
	{
		*reason = nextonePreferLRU;
		return 1;
	}

	if (g1->cTime > g2->cTime)
	{
		*reason = nextoneInferiorLRU;
		return -1;
	}

        if (g1->vpnVal > g2->vpnVal)
        {
			*reason = nextonePreferVpn;
            return 1;
        }

        if (g1->vpnVal < g2->vpnVal)
        {
			*reason = nextoneInferiorVpn;
            return -1;
        }

        if (g1->zoneVal > g2->zoneVal)
        {
			*reason = nextonePreferZone;
            return 1;
        }

        if (g1->zoneVal < g2->zoneVal)
        {
			*reason = nextoneInferiorZone;
            return -1;
        }

        return 0;
}

/* Initialize set to { -1,-1,-1... }
 * GwCompare returs when comparison is either > or <.
 * It proceeds to the next level if its ==
 * generally a higher value for each of these parameters
 * indicates a higher preference
 */
/* return -1 if g1 < g2, 0 if g1=g2, 1 if g1 > g2,
 * where '>' means preference
 */
int
GwRouteCompare(struct gw_match_set *g1, struct gw_match_set *g2, int *reason)
{
    // Now we need to evaluate the new set with the
    // old set.
	
	// We are trying to find out if g1 > g2 (return 1)
	// or g1 < g2 (return -1)
	// All cases for (return 1) are written before
	// cases of (return -1), for simplicity

	if (g1->routePriority > g2->routePriority)
	{
		*reason = nextonePreferPriority;
		return 1;
	}

	if (g1->routePriority < g2->routePriority)
	{
		*reason = nextoneInferiorPriority;
		return -1;
	}

    if (g1->cpVal > g2->cpVal)
    {
		*reason = nextonePreferCallingPlan;
        return 1;
    }

    if (g1->cpVal < g2->cpVal)
    {
		*reason = nextoneInferiorCallingPlan;
        return -1;
    }

    return -1;
}

int RouteCompareSet(route_match_set *old, route_match_set *new)
{
	if((new->cacheRouteEntry->routeEntry.crflags & CRF_DYNAMICLRU) && !(old->cacheRouteEntry->routeEntry.crflags & CRF_DYNAMICLRU))
	{
		old->routePriority = new->routePriority;
		old->matchLen = new->matchLen;
		old->cacheRouteEntry = new->cacheRouteEntry;
		return 1;
	}
	else if(!(old->cacheRouteEntry->routeEntry.crflags & CRF_DYNAMICLRU) && (new->routePriority > old->routePriority))
	{
		old->routePriority = new->routePriority;
		old->matchLen = new->matchLen;
		old->cacheRouteEntry = new->cacheRouteEntry;
		return 1;
	}
	else
	{
		return 0;
	}
}

/* Cache must be attached at this point */
int
GwUpdateCP(char *cpname)
{
	char fn[] = "GwUpdateCP():";
	CacheTableInfo *gwCacheEntry;

	/* All gateways which are in the calling plan, must be informed
	 * for now just re-register them
	 */

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	for (gwCacheEntry = CacheGetFirst(regCache); gwCacheEntry;
		gwCacheEntry = CacheGetNext(regCache, &gwCacheEntry->data))
	{
		if (strcmp(gwCacheEntry->data.cpname, cpname))
		{
			continue;
		}

		// Only the iedge 1000 should be aged out

		if ((gwCacheEntry->data.ispcorgw == IEDGE_1000) ||
			(gwCacheEntry->data.ispcorgw == IEDGE_1000G))
		{
			/* Age out this entry */
			gwCacheEntry->data.stateFlags &= ~CL_REGISTERED;
		}
	}

	CacheFreeIterator(regCache);

	CacheReleaseLocks(regCache);

	return 0;
}

int
GwUpdateCR(char *crname)
{
	char fn[] = "GwUpdateCR():";
	CacheCPBEntry *cacheCPBEntry = 0, tmpCacheCPBEntry;
	int present;

	/* All Calling Plans which have this route should be
	 * updated. Walk the cache to find all cp's which
	 * have this cr, and then call Update CP.
	 */
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheCPBEntry = CacheGetFirst(cpbCache); cacheCPBEntry;
	cacheCPBEntry = CacheGetNext(cpbCache, &cacheCPBEntry->cpbEntry))
	{
		if (!strcmp(cacheCPBEntry->cpbEntry.crname, crname))
		{
			GwUpdateCP(cacheCPBEntry->cpbEntry.cpname);
		}
	}

	CacheFreeIterator(cpbCache);

	CacheReleaseLocks(cpbCache);
	CacheReleaseLocks(regCache);
	return(0);
}

// decrement the no. of Active Calls on the port 
// Only serial no. and uport need to be set in PhoNode
// callSource = Is MSW call source
void 
GwPortFreeCall(PhoNode *phonode, int ncalls, int callSource)
{
	static char fn[] = "GwPortFreeCall():";
	CacheTableInfo * info;
	InfoEntry *pentry;
	IgrpInfo  *igrp = NULL;
	CacheIgrpInfo *entry = NULL;
	unsigned char  grpfree = 0;
	int haveIgrpLock = 0;

	if (!phonode || (phonode->regid[0] == '\0'))
	{
		return;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	info = CacheGet(regCache, phonode);

	if (!info)
	{
		CacheReleaseLocks(regCache);
		DEBUG(MFIND,NETLOG_DEBUG2,
			("%s: CacheGet failed to find infoEntry for %s/%lu\n",
			fn,phonode->regid,phonode->uport));
		return;
	}

	pentry = &info->data;

	// GwPortFreeCall function has been extended to free
	// ports all endpoints regardless of their type.
	// (as per bug 6891)
	
		NETDEBUG(MFIND,NETLOG_DEBUG4,
			("%s regid = %s/%lu xcalls = %d xin = %d xout = %d"
			"nin = %d nout = %d"
			"freeing = %d msw-src = %d\n",
			fn,pentry->regid,pentry->uport,
			IedgeXCalls(pentry), IedgeXInCalls(pentry), IedgeXOutCalls(pentry),
			IedgeInCalls(pentry), IedgeOutCalls(pentry),
			ncalls, callSource));


		if (pentry->igrpName[0] != '\0')
		{
			CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
			haveIgrpLock = 1;
			if (entry = CacheGet(igrpCache, pentry->igrpName))
			{
				igrp = &entry->igrp;
			}
		}

		if (igrp)
		{
			NETDEBUG(MFIND,NETLOG_DEBUG4,
				("%s igrp %s xcalls = %d xin = %d xout = %d"
				"nin = %d nout = %d",
				fn,igrp->igrpName,igrp->nMaxCallsTotal,
				igrp->nMaxCallsIn, igrp->nMaxCallsOut, 
				igrp->nCallsIn, igrp->nCallsOut)); 
		}

		if (callSource)
		{
			if(igrp)
			{
				if (ncalls <= IgrpOutCalls(igrp))
				{
					grpfree = 1;
				}
				else
				{
					NETERROR(MFIND,("%s trying to free more than allocated on grp %s\n",fn, igrp->igrpName));
				}
			}
			else
			{
				grpfree = 1;
			}


			if (grpfree && (ncalls <= IedgeOutCalls(pentry)) )
			{
				IedgeOutCalls(pentry) -= ncalls;
				IedgeCalls(pentry) -= ncalls;

				if (igrp)
				{
					IgrpOutCalls(igrp) -= ncalls;
					IgrpCallsTotal(igrp) -= ncalls;
				}
			}
			else 
			{
				if (grpfree) /* so that only one of the two statements is printed */
				{
					NETERROR(MFIND,("%s trying to free more than allocated\n",fn));
				}
			}
		}
		else
		{
			if(igrp)
			{
				if (ncalls <= IgrpInCalls(igrp))
				{
					grpfree = 1;
				}
				else
				{
					NETERROR(MFIND,("%s trying to free more than allocated on grp %s\n",fn, igrp->igrpName));
				}
			}
			else
			{
				grpfree = 1;
			}


			if (grpfree && (ncalls <= IedgeInCalls(pentry)) )
			{
				IedgeInCalls(pentry) -= ncalls;
				IedgeCalls(pentry) -= ncalls;

				if (igrp)
				{
					IgrpInCalls(igrp) -= ncalls;
					IgrpCallsTotal(igrp) -= ncalls;
				}
			}
			else 
			{
				if (grpfree) /* only one of the two NETERROR should print */
				{	
					NETERROR(MFIND,("%s trying to free more than allocated\n",fn));
				}
			}
		}

		if (haveIgrpLock)
		{
			CacheReleaseLocks(igrpCache);
			haveIgrpLock = 0;
		}
	

	  CacheReleaseLocks(regCache);
}


/* returns error only if it found a phonode but cud no reserver port
*/
int 
GwPortAllocCall(PhoNode *phonode, int ncalls, int callSource)
{
	static char fn[] = "GwPortAllocCall():";
	CacheTableInfo * info;
	InfoEntry *pentry;

	if (!phonode || (phonode->regid[0] == '\0'))
	{
		return 0;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	info = CacheGet(regCache, phonode);

	if (!info)
	{
		CacheReleaseLocks(regCache);
		DEBUG(MFIND,NETLOG_DEBUG2,
			("%s: CacheGet failed to find infoEntry for %s/%lu\n",
			fn,phonode->regid,phonode->uport));
		return(-1);
	}

	pentry = &info->data;

	// GwPortAllocCall function has been extended to allocate
	// ports all endpoints regardless of their type.
	// In other words, GwPortAllocCall will not longer allocate calls exclusively
	// to gateways (as per bug 6891)

	if(ctInfoAllocCall(info, ncalls, callSource) <0)
	{
		CacheReleaseLocks(regCache);
		NETDEBUG(MFIND,NETLOG_DEBUG4,("%s failed to allocate call\n",fn));
		return(-1);
	}

	CacheReleaseLocks(regCache);
	return 0;
}

/* returns error only if it found a phonode but cud no reserver port
*  Checks if more calls can be placed on this gateway  
*/
int 
GwPortAvailCall(PhoNode *phonode, int ncalls, int callSource)
{
	static char fn[] = "GwPortAvailCall():";
	CacheTableInfo * info;
	InfoEntry *pentry;

	if (!phonode || (phonode->regid[0] == '\0'))
	{
		return 0;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	info = CacheGet(regCache, phonode);

	if (!info)
	{
		CacheReleaseLocks(regCache);
		DEBUG(MFIND,NETLOG_DEBUG2,
			("%s: CacheGet failed to find infoEntry for %s/%lu\n",
			fn,phonode->regid,phonode->uport));
		return(-1);
	}

	pentry = &info->data;

	// GwAvailCall function has been extended to perform
	// port availability check for all endpoints regardless of their type.
	// (as per bug 6891)

	if(ctInfoAvailCall(info, ncalls, callSource) <0)
	{
		CacheReleaseLocks(regCache);
		NETDEBUG(MFIND,NETLOG_DEBUG4,("%s No more calls available\n",fn));
		return(-1);
	}

	CacheReleaseLocks(regCache);
	return 0;
}

/* The regCache locks should be already acquired
*/
int 
ctInfoAllocCall(CacheTableInfo *ctInfo,int ncalls, int callSource)
{
	static char	fn[] = "ctInfoAllocCall";
	InfoEntry *pentry;
	IgrpInfo  *igrp = NULL;
	CacheIgrpInfo *entry = NULL;
	unsigned char	grpcac = 0;
	int haveIgrpLock = 0;

	pentry = &ctInfo->data;

	if (strlen(pentry->igrpName))
	{
		CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
		haveIgrpLock = 1;
		if (entry = CacheGet(igrpCache, pentry->igrpName))
		{
			igrp = &entry->igrp;
		}
	}

	NETDEBUG(MFIND,NETLOG_DEBUG4,
		("%s regid = %s/%lu xcalls = %d xin = %d xout = %d"
		"nin = %d nout = %d"
		"allocating = %d msw-src = %d\n",
		fn,pentry->regid,pentry->uport,
		IedgeXCalls(pentry), IedgeXInCalls(pentry), IedgeXOutCalls(pentry),
		IedgeInCalls(pentry), IedgeOutCalls(pentry),
		ncalls, callSource));

	if (igrp)
	{
		NETDEBUG(MFIND,NETLOG_DEBUG4,
			("%s igrp %s xcalls = %d xin = %d xout = %d"
			"nin = %d nout = %d",
			fn,igrp->igrpName,igrp->nMaxCallsTotal,
			igrp->nMaxCallsIn, igrp->nMaxCallsOut, 
			igrp->nCallsIn, igrp->nCallsOut)); 
	}

	if (callSource)
	{

		if (igrp)
		{
			if (((IgrpOutCalls(igrp) + ncalls <= IgrpXOutCalls(igrp)) ||
					!IgrpXOutCalls(igrp)) &&
				((IgrpCallsTotal(igrp) + ncalls <= IgrpXCallsTotal(igrp)) ||
					!IgrpXCallsTotal(igrp)))
			{
				grpcac =1;
			}
		}
		else
		{
			grpcac =1;
		}


		if (grpcac && 
			((IedgeOutCalls(pentry) + ncalls <= IedgeXOutCalls(pentry)) ||
				!IedgeXOutCalls(pentry)) &&
			((IedgeCalls(pentry) + ncalls <= IedgeXCalls(pentry)) ||
				!IedgeXCalls(pentry)))
		{
			IedgeOutCalls(pentry) += ncalls;
			IedgeCalls(pentry) += ncalls;

			if (IedgeCalls(pentry) == IedgeXOutCalls(pentry))
			{
				time(&ctInfo->data.dndTime);
			}

			if (igrp)
			{
				IgrpOutCalls(igrp) += ncalls;
				IgrpCallsTotal(igrp) += ncalls;

				if (IgrpCallsTotal(igrp) == IgrpXOutCalls(igrp))
				{
					time(&igrp->dndTime);
				}
			}

			if (haveIgrpLock)
			{
				CacheReleaseLocks(igrpCache);
				haveIgrpLock = 0;
			}
			return 0;
		}
	}
	else
	{
		if (igrp)
		{
			if (((IgrpInCalls(igrp) + ncalls <= IgrpXInCalls(igrp)) ||
					!IgrpXInCalls(igrp)) &&
				((IgrpCallsTotal(igrp) + ncalls <= IgrpXCallsTotal(igrp)) ||
					!IgrpXCallsTotal(igrp)))
			{
				grpcac =1;
			}
		}
		else
		{
			grpcac =1;
		}

		if (grpcac &&
			((IedgeInCalls(pentry) + ncalls <= IedgeXInCalls(pentry)) ||
				!IedgeXInCalls(pentry)) &&
			((IedgeCalls(pentry) + ncalls <= IedgeXCalls(pentry)) ||
				!IedgeXCalls(pentry)))
		{
			IedgeInCalls(pentry) += ncalls;
			IedgeCalls(pentry) += ncalls;

			if (IedgeCalls(pentry) == IedgeXInCalls(pentry))
			{
				time(&ctInfo->data.dndTime);
			}

			if (igrp)
			{
				IgrpInCalls(igrp) += ncalls;
				IgrpCallsTotal(igrp) += ncalls;

				if (IgrpCallsTotal(igrp) == IgrpXInCalls(igrp))
				{
					time(&igrp->dndTime);
				}
			}

			if (haveIgrpLock)
			{
				CacheReleaseLocks(igrpCache);
				haveIgrpLock = 0;
			}
			return 0;
		}
	}

	if (haveIgrpLock)
	{
		CacheReleaseLocks(igrpCache);
		haveIgrpLock = 0;
	}
	return -1;
}

/* The regCache locks should be already acquired
*  Checks if more calls can be placed on this gateway  
*/
int 
ctInfoAvailCall(CacheTableInfo *ctInfo,int ncalls, int callSource)
{
	static char	fn[] = "ctInfoAvailCall";
	InfoEntry *pentry;
	IgrpInfo  *igrp = NULL;
	CacheIgrpInfo *entry = NULL;
	unsigned char	grpcac = 0;
	int haveIgrpLock = 0;

	pentry = &ctInfo->data;

	if (strlen(pentry->igrpName))
	{
		CacheGetLocks(igrpCache, LOCK_WRITE, LOCK_BLOCK);
		haveIgrpLock = 1;
		if (entry = CacheGet(igrpCache, pentry->igrpName))
		{
			igrp = &entry->igrp;
		}
	}

	if (igrp)
	{
		NETDEBUG(MFIND,NETLOG_DEBUG4,
			("%s igrp %s xcalls = %d xin = %d xout = %d"
			"nin = %d nout = %d",
			fn,igrp->igrpName,igrp->nMaxCallsTotal,
			igrp->nMaxCallsIn, igrp->nMaxCallsOut, 
			igrp->nCallsIn, igrp->nCallsOut)); 
	}

	NETDEBUG(MFIND,NETLOG_DEBUG4,
		("%s regid = %s/%lu xcalls = %d xin = %d xout = %d"
		"nin = %d nout = %d"
		"allocating = %d msw-src = %d\n",
		fn,pentry->regid,pentry->uport,
		IedgeXCalls(pentry), IedgeXInCalls(pentry), IedgeXOutCalls(pentry),
		IedgeInCalls(pentry), IedgeOutCalls(pentry),
		ncalls, callSource));

	if (callSource)
	{
		if (igrp)
		{
			if (((IgrpOutCalls(igrp) + ncalls <= IgrpXOutCalls(igrp)) ||
					!IgrpXOutCalls(igrp)) &&
				((IgrpCallsTotal(igrp) + ncalls <= IgrpXCallsTotal(igrp)) ||
					!IgrpXCallsTotal(igrp)))
			{
				grpcac =1;
			}
		}
		else
		{
			grpcac =1;
		}

		if (grpcac &&
			((IedgeOutCalls(pentry) + ncalls <= IedgeXOutCalls(pentry)) ||
				!IedgeXOutCalls(pentry)) &&
			((IedgeCalls(pentry) + ncalls <= IedgeXCalls(pentry)) ||
				!IedgeXCalls(pentry)))
		{
			if (haveIgrpLock)
			{
				CacheReleaseLocks(igrpCache);
				haveIgrpLock = 0;
			}
			return 0;
		}
	}
	else
	{
		if (igrp)
		{
			if (((IgrpInCalls(igrp) + ncalls <= IgrpXInCalls(igrp)) ||
					!IgrpXInCalls(igrp)) &&
				((IgrpCallsTotal(igrp) + ncalls <= IgrpXCallsTotal(igrp)) ||
					!IgrpXCallsTotal(igrp)))
			{
				grpcac =1;
			}
		}
		else
		{
			grpcac =1;
		}

		if (grpcac &&
			((IedgeInCalls(pentry) + ncalls <= IedgeXInCalls(pentry)) ||
				!IedgeXInCalls(pentry)) &&
			((IedgeCalls(pentry) + ncalls <= IedgeXCalls(pentry)) ||
				!IedgeXCalls(pentry)))
		{
			if (haveIgrpLock)
			{
				CacheReleaseLocks(igrpCache);
				haveIgrpLock = 0;
			}
			return 0;
		}
	}

	if (haveIgrpLock)
	{
		CacheReleaseLocks(igrpCache);
		haveIgrpLock = 0;
	}
	return -1;
}

/* return -1 if g1 < g2, 0 if g1=g2, 1 if g1 > g2,
 * where '>' means preference
 */
int
CpCompare(cp_match_set *g1, cp_match_set *g2)
{
        /* Now we need to evaluate the new set with the
         * old set.
         * The policy is as follows:
         * Prefer same zone / vpn. Then
         * Prefer a better match. Then
         */
	 
		if (g1->srcVal > g2->srcVal)
		{
			return 1;
		}

		if (g1->srcVal < g2->srcVal)
		{
			return -1;
		}

        if (g1->cpVal > g2->cpVal)
        {
                return 1;
        }

        if (g1->cpVal < g2->cpVal)
        {
                return -1;
        }

        return 0;
}

/* Apply the route to phone, yielding a new phone
 * number. This new phone number one which anyone else 
 * having this route, will use to access this iedge
 * return 1 if the application is possible, 
 * -1 if not.
 */
int
ApplyRoutePrefix(char *crname, char *phone, char *newphone, int *xlen)
{
	VpnRouteKey routeKey = { 0 };
	CacheVpnRouteEntry *cacheRouteEntry = 0;
	int rc = -1;

	/* Lookup the call route */	
	strncpy(routeKey.crname, crname, CALLPLAN_ATTR_LEN);

	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);

	cacheRouteEntry = CacheGet(cpCache, &routeKey);

	if (cacheRouteEntry == NULL)
	{
		goto _return;
	}

	rc = CRApplyPrefix(&cacheRouteEntry->routeEntry, 
			phone, newphone, xlen);

_return:
	CacheReleaseLocks(cpCache);

	return rc;
}

int
CRApplyPrefix(VpnRouteEntry *routeEntry, char *phone, char *newphone, int *xlen)
{
	char unspec[PHONE_NUM_LEN] = { 0 }, *newdest = NULL;
	int unspeclen = 0, reject = 0, rlen = 0;

	/* leave destlen - strlen(dest) digits at the end
	 * of the phone, as those are unspecified.
	 * The rest of the phone number must match
	 * the prefix. This is the test whether the route will
	 * apply or not. If it does, the new phone is dest followed
	 * by the unspecified digits.
	 */

	if (xlen)
	{
		*xlen = 0;
	}

	rlen = CPAnalyzePattern(routeEntry->dest, &reject, &newdest);

	unspeclen = routeEntry->destlen - rlen;
	if ((unspeclen < 0) || (unspeclen >= PHONE_NUM_LEN))
	{
		return 0;
	}

	if ( memcmp(routeEntry->prefix, phone, (strlen(phone)-unspeclen)) )
	{
		return 0;
	}

	/* Match */
	if (newphone)
	{
		strncpy(newphone, newdest, PHONE_NUM_LEN-1);
		nx_strlcat(newphone, phone + strlen(phone)- unspeclen, PHONE_NUM_LEN);
	}

	if (xlen)
	{
		*xlen = unspeclen;
	}

	if (reject)
	{
		return -1;
	}

	return 1;
}

/* return -1 if the route rejects phone,
 * 0 if it doesnt apply,
 * 1 if it applies.
 * The functions are inverse functions of ApplyRoutePrefix
 * functions. However unlike the latter, we don not compute
 * the new phone right here
 */
int
ApplyRouteDest(char *crname, char *phone, char *newphone)
{
	VpnRouteKey routeKey = { 0 };
	CacheVpnRouteEntry *cacheRouteEntry = 0;
	int matchLen = 0, hasRejectAllElse = 0;

	/* Lookup the call route */	
	strncpy(routeKey.crname, crname, CALLPLAN_ATTR_LEN);
	cacheRouteEntry = CacheGet(cpCache, &routeKey);

	if (cacheRouteEntry == NULL)
	{
		return -1;
	}

	return CRApplyDest(&cacheRouteEntry->routeEntry, phone, newphone,
			&matchLen, &hasRejectAllElse);
}

int
CRApplyDest(
	VpnRouteEntry *routeEntry, 
	char *phone, 
	char *newphone,
	int *matchLen,
	int *hasRejectAllElse
)
{
	char *outdest;
	int hasReject = 0, hasMatch = 0;
	int rc;

	if (matchLen)
	{
		*matchLen = 0;
	}
			
	if ((routeEntry->destlen) &&
		(routeEntry->destlen != strlen(phone)))
	{
		/* route doesn't apply */
		return 0;
	}

	rc = CPMatchPattern(
		routeEntry->dest,
		&outdest, 
		&hasMatch, 
		hasRejectAllElse,
		&hasReject, phone);

	if (hasReject)
	{
		return -1;
	}
	else if (hasMatch) 
	{
		if (matchLen)
		{
			*matchLen = rc;
		}
			
		if (newphone)
		{
			strcpy(newphone, routeEntry->prefix);
			nx_strlcat(newphone, phone+rc, PHONE_NUM_LEN);
		}

		return 1;
	}
	else
	{
		if (newphone)
		{
			strcpy(newphone, phone);
		}

		return 0;
	}
}

int
CRApplySrc(
	VpnRouteEntry *routeEntry, 
	char *sphone, 
	char *newsphone,
	int *matchLen,
	int *hasRejectAllElse
)
{
	char *outdest;
	int hasReject = 0, hasMatch = 0, hasVpnMatch = 0;
	int rc;

	if (matchLen)
	{
		*matchLen = 0;
	}
			
	if ((routeEntry->srclen) &&
		(routeEntry->srclen != strlen(sphone)))
	{
		/* route doesn't apply */
		return 0;
	}

	rc = CPMatchPattern(
		routeEntry->src,
		&outdest, 
		&hasMatch,
		hasRejectAllElse,
		&hasReject, sphone);

	if (hasReject)
	{
		return -1;
	}
	else if (hasMatch) 
	{
		if (matchLen)
		{
			*matchLen = rc;
		}
			
		if (newsphone)
		{
			strcpy(newsphone, routeEntry->srcprefix);
			nx_strlcat(newsphone, sphone+rc, PHONE_NUM_LEN);
		}

		return 1;
	}
	else
	{
		if (newsphone)
		{
			strcpy(newsphone, sphone);
		}

		return 0;
	}
}

int
CRApplyTime(
	CallPlanBindEntry *cpbEntry
)
{
	char fn[] = "CRApplyTime():";
	time_t now, t1, t2;
	struct tm tmnow = { 0 }, tm1 = { 0 }, tm2 = { 0 };

	time(&now);
	if (localtime_r(&now, &tmnow) < 0)
	{
		NETERROR(MFIND,
			("%s localtime_r error %d\n", fn, errno));
		return -1;
	}

	/* tmnow should be between the interval specified
	 * by cpb entry
	 */
	memcpy(&tm1, &cpbEntry->sTime, sizeof(struct tm));
	memcpy(&tm2, &cpbEntry->fTime, sizeof(struct tm));
	
	// specify dom, and retrieve the current value of
	// wday and yday. These will have to be compared...

	if (tm1.tm_year == TM_ANY) tm1.tm_year = tmnow.tm_year;
	if (tm1.tm_mon == TM_ANY) tm1.tm_mon = tmnow.tm_mon;
	if (tm1.tm_mday == TM_ANY) tm1.tm_mday = tmnow.tm_mday;
	if (tm1.tm_hour == TM_ANY) tm1.tm_hour = tmnow.tm_hour;
	if (tm1.tm_min == TM_ANY) tm1.tm_min = tmnow.tm_min;
	if (tm1.tm_sec == TM_ANY) tm1.tm_sec = tmnow.tm_sec;
	/* if (tm1.tm_wday == TM_ANY) */ tm1.tm_wday = tmnow.tm_wday;
	/* if (tm1.tm_yday == TM_ANY) */ tm1.tm_yday = tmnow.tm_yday;
	tm1.tm_isdst = -1;

	if (tm2.tm_year == TM_ANY) tm2.tm_year = tmnow.tm_year;
	if (tm2.tm_mon == TM_ANY) tm2.tm_mon = tmnow.tm_mon;
	if (tm2.tm_mday == TM_ANY) tm2.tm_mday = tmnow.tm_mday;
	if (tm2.tm_hour == TM_ANY) tm2.tm_hour = tmnow.tm_hour;
	if (tm2.tm_min == TM_ANY) tm2.tm_min = tmnow.tm_min;
	if (tm2.tm_sec == TM_ANY) tm2.tm_sec = tmnow.tm_sec;
	/* if (tm2.tm_wday == TM_ANY) */ tm2.tm_wday = tmnow.tm_wday;
	/* if (tm2.tm_yday == TM_ANY) */ tm2.tm_yday = tmnow.tm_yday;
	tm2.tm_isdst = -1;

	t1 = mktime(&tm1);
	t2 = mktime(&tm2);

	if ((t1 == (time_t)-1) || (t2 == (time_t)-1))
	{
		NETERROR(MFIND,
			("%s Error in mktime %d\n", fn, errno));
		return -1;
	}

	// check to see if the wday and yday are within the limits
	if (cpbEntry->sTime.tm_wday != TM_ANY)
	{
		t1 += (cpbEntry->sTime.tm_wday - tmnow.tm_wday)*24*60*60;
	}

	if (cpbEntry->fTime.tm_wday != TM_ANY)
	{
		t2 += (cpbEntry->fTime.tm_wday - tmnow.tm_wday)*24*60*60;
	}

	if (cpbEntry->sTime.tm_yday != TM_ANY)
	{
		t1 += (cpbEntry->sTime.tm_yday - tmnow.tm_yday)*24*60*60;
	}

	if (cpbEntry->fTime.tm_yday != TM_ANY)
	{
		t2 += (cpbEntry->fTime.tm_yday - tmnow.tm_yday)*24*60*60;
	}

	// if t1 < t2, then see if time now is between t1 and t2
	// else see if time now is between t2 and t1
	if (t1 < t2)
	{
		if ((difftime(now, t1) >= 0) && (difftime(now, t2) <= 0))
		{
			/* Interval applies */
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if ((difftime(now, t2) > 0) && (difftime(now, t1) < 0))
		{
			/* Interval applies */
			return -1;
		}
		else
		{
			return 1;
		}
	}
}

/*
 * Find the source cache entry, or construct one if possible.
 * Return the entry in srcCacheInfo, if one is found (or constructed).
 * return 1 in this case.
 * else return -1.
 * 1) see if caller-id is there; return with info
 * 2) see if h323id is there; return with info
 * 3) see if regid is there
 * 4) see if ip address is there; try finding exact port for ip
 * 5) if nothing is found or if vpn is not found,
 *	  see if we can construct one entry from our vpns (caller-id)
 */
int
FillSourceCacheForCallerIdForceLkup(
	PhoNode *phonodep,
	char *h323id,
	char *tg,
	char *cic,
	char *dnis,
	CacheTableInfo *srcCacheInfo,
	int force
)
{
	char fn[] = "FillSourceCacheForCallerId";
	RealmIP realmip;
	RealmSubnet rmsub;
	int rc = -1;
	CacheTableInfo *cacheInfo = NULL, // main entry
		*cacheInfoTmp = NULL, // matched uport
		*cacheInfoTmp2 = NULL;
	struct gw_match_set dnisgwset, anigwset;
	int uport, reason;
	
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);

	// if no match was found and the regid was already set, return that
	if (phonodep && !force &&
		(phonodep->regid[0] != '\0') &&
		(cacheInfo = CacheGet(regCache, phonodep->regid)))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Src already located before %s/%lu\n",
			fn, 
			cacheInfo->data.regid, cacheInfo->data.uport));

       		memcpy(srcCacheInfo, cacheInfo, sizeof(CacheTableInfo));

		rc = 1;	
		goto _return;
	}

	// First find the ip/port based match, then narrow down the regid
	// subnet is exact match, so no need to narrow down in that case

	realmip.ipaddress = phonodep->ipaddress.l;
	realmip.realmId = phonodep->realmId;
	if (phonodep && BIT_TEST(phonodep->sflags, ISSET_IPADDRESS) &&
		(cacheInfo = CacheGet(ipCache, &realmip)))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Found src %s/%lu based on <ip,realmid>\n",
			fn, 
			cacheInfo->data.regid, cacheInfo->data.uport));
		
		rc = 1;
	}

	rmsub.subnetip= phonodep->ipaddress.l;
	rmsub.realmId = phonodep->realmId;
	if (phonodep && BIT_TEST(phonodep->sflags, ISSET_IPADDRESS) &&
		(cacheInfoTmp = GetIedgeLongestMatch(&rmsub)))
	{
		// check if regid's match
		if ((rc < 0) || !memcmp(cacheInfoTmp->data.regid, cacheInfo->data.regid, REG_ID_LEN))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Found src %s/%lu based on subnet\n",
				fn, 
				cacheInfoTmp->data.regid, cacheInfoTmp->data.uport));
		
			cacheInfo = cacheInfoTmp;
    			memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

			rc = 1;
			goto _return;
		}
		else
		{
			cacheInfoTmp = NULL;
		}
	}

	if (tg && tg[0] 
		&& (cacheInfoTmp = CacheGet(tgCache, tg)))
	{
		// check if regid's match
		if (((rc < 0) || !memcmp(cacheInfoTmp->data.regid, cacheInfo->data.regid, REG_ID_LEN))
				&& (cacheInfoTmp->data.realmId == phonodep->realmId))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Found src %s/%lu based on tg %s\n",
				fn, 
				cacheInfoTmp->data.regid, cacheInfoTmp->data.uport, tg));
		
			cacheInfo = cacheInfoTmp;
    			memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

			rc = 1;
			goto _return;
		}
		else
		{
			cacheInfoTmp = NULL;
		}
	}

	if (phonodep && phonodep->phone[0] 
		&& (cacheInfoTmp = CacheGet(phoneCache, phonodep->phone)))
	{
		// check if regid's match
		if (((rc < 0) || !memcmp(cacheInfoTmp->data.regid, cacheInfo->data.regid, REG_ID_LEN))
				&& (cacheInfoTmp->data.realmId == phonodep->realmId))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Found src %s/%lu based on phone %s\n",
				fn, 
				cacheInfoTmp->data.regid, cacheInfoTmp->data.uport, phonodep->phone));
		
			cacheInfo = cacheInfoTmp;
    			memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

			rc = 1;
			goto _return;
		}
		else
		{
			cacheInfoTmp = NULL;
		}
	}

	if (h323id && h323id[0] &&
		(cacheInfoTmp = CacheGet(h323idCache, h323id)))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Found src %s/%lu based on h323id %s\n",
			fn, 
			cacheInfoTmp->data.regid, cacheInfoTmp->data.uport, h323id));
	
		// check if regid's match
		if (((rc < 0) || !memcmp(cacheInfoTmp->data.regid, cacheInfo->data.regid, REG_ID_LEN))
				&& (cacheInfoTmp->data.realmId == phonodep->realmId))
		{
			cacheInfo = cacheInfoTmp;
    			memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

			rc = 1;
			goto _return;
		}
		else
		{
			cacheInfoTmp = NULL;
		}
	}

	if (phonodep && phonodep->regid[0] &&
		(cacheInfoTmp = CacheGet(regCache, phonodep)))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Found src %s/%lu based on regid\n",
			fn, 
			cacheInfoTmp->data.regid, cacheInfoTmp->data.uport));
		
		// check if regid's match
		if (((rc < 0) || !memcmp(cacheInfoTmp->data.regid, cacheInfo->data.regid, REG_ID_LEN))
				&& (cacheInfoTmp->data.realmId == phonodep->realmId))
		{
			cacheInfo = cacheInfoTmp;
       			memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

			rc = 1;
			goto _return;
		}
		else
		{
			cacheInfoTmp = NULL;
		}
	}

	// if no match was found and the regid was already set, return that
	if (phonodep && force &&
		(phonodep->regid[0] != '\0') &&
		(cacheInfo = CacheGet(regCache, phonodep->regid)))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Src already located before %s/%lu\n",
			fn, 
			cacheInfo->data.regid, cacheInfo->data.uport));

       		memcpy(srcCacheInfo, cacheInfo, sizeof(CacheTableInfo));

		rc = 1;	
		goto _return;
	}

	if (cacheInfoTmp)
	{
		NETERROR(MFIND, ("%s cacheInfoTmp already exists !\n", fn));
	}

	// Once we start matching routes, ONLY the one with the highest
	// priority will be matched

	// initialize
	dnisgwset.gwPriority = -100;
	dnisgwset.routePriority = -1;

	// Do a DNIS/ANI based match
	// If allowANIPortSel/allowDNISPortSel is set, then allow the cacheInfo to be non-null. This way
	// we can select the uport
	if (dnis && dnis[0] && 
		((!allowDNISPortSel && cacheInfo) || allowDNISPortSel))
	{
		cacheInfoTmp2 = GwRouteLookup(cporigDNISCache, 0, dnis, 
						cacheInfo?cacheInfo->data.regid:NULL, 
						&dnisgwset, NULL, NULL);
	}

	if (cacheInfoTmp2) cacheInfoTmp = cacheInfoTmp2;

	if (phonodep && phonodep->phone[0] && 
		((!allowANIPortSel && cacheInfo) || allowANIPortSel))
	{
		cacheInfoTmp2 = GwRouteLookup(cporigANICache, sizeof(ListEntry), 
						phonodep->phone, cacheInfo?cacheInfo->data.regid:NULL, 
						&anigwset, &dnisgwset, NULL);
	}

	if (cacheInfoTmp2) cacheInfoTmp = cacheInfoTmp2;

	if (cacheInfoTmp && (cacheInfoTmp->data.realmId == phonodep->realmId))
	{
		cacheInfo = cacheInfoTmp;
		memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));

		rc = 1;
		goto _return;
	}

	// if no direct match is found, use uport 0
	// only if its in the same realm
	if (rc > 0)
	{
		if (cacheInfo)
		{
			// Check if uport 0 exists, as if it does, it will
			// always be our default IP src. This makes the
			// src selection process much deterministic. 
			// - For 2.05R2 branch only.
	
			if ((uport = cacheInfo->data.uport) != 0)
			{
				memcpy(srcCacheInfo->data.regid, cacheInfo->data.regid, REG_ID_LEN);
				srcCacheInfo->data.uport = 0;
			
				if (cacheInfoTmp = CacheGet(regCache, &srcCacheInfo->data))
				{
					if (cacheInfoTmp->data.realmId == phonodep->realmId)
					{
						memcpy(srcCacheInfo, cacheInfoTmp, sizeof(CacheTableInfo));
	
						rc = 1;
						goto _return;
					}
				}

			}

			memcpy(srcCacheInfo, cacheInfo, sizeof(CacheTableInfo));

			// keep going
		}
		else
		{
			NETERROR(MFIND, ("%s rc>0 but no cacheInfo!\n", fn));
		}
	}

_return:
	CacheReleaseLocks(regCache);
	
	return rc;
}

// Given a pattern (potentially with variables,
// and a last set of assignments, generate
// an instance of the pattern
// return the length of new pattern
// act as replacement for strcpy, so always null terminate
int
GeneratePatternInstance(char *pat, char *vars, char *newpat, int patlen)
{
	int i = 0, j = 0, k = 0;
	int range;
	
	while (i<patlen)
	{
		if (pat[i] == '?')
		{
			vars[j] = (vars[j]+1*((int)(drand48()*10)))%10;
			newpat[k] = '0' + vars[j];
			i++;
			j++;
		}
		else if ((i+2<patlen) && (pat[i] == '%'))
		{
			range = pat[i+2]-pat[i+1]+1;
			vars[j] = (vars[j]+1*((int)(drand48()*range)))%range;
			newpat[k] = pat[i+1]+vars[j];
			i+=3;
			j++;
		}
		else
		{
			newpat[k] = pat[i];
			i++;
		}
		k++;
	}

	newpat[k] = '\0';

	return k;
}

/* Functions to replace original ANI with an ANI from a file */

// Replace the phone number with a number from the given file
int
ReplaceANI(char *phonenum, char *filename)
{
	char fn[] = "ReplaceANI():";
	FILE *fileptr = NULL;
	
	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (!(fileptr = GetANIFile(filename)))
	{
		return -1;
	}

	if (GetNumberFromANIFile(phonenum, filename, fileptr) < 0)
	{
		rewind(fileptr);
		if (GetNumberFromANIFile(phonenum, filename, fileptr) < 0)
		{
			NETERROR(MFIND,("%s Could not read ANI from file %s\n", fn, filename));
			return -1;
		}
	}

	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s New ANI selected from file %s is %s\n", 
		fn, filename, phonenum));
	
	return 0;
}

// Get the file pointer for the given filename
static FILE *
GetANIFile(char *filename)
{
	char fn[] = "GetANIFile():";
	FileInfo *fileinfo = NULL;
	struct stat filestat = {0};

	// Invalid or no filename
	if ((filename == NULL) || (filename[0] == '\0'))
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s No file specified\n", fn));
		return NULL;
	}

	// If cli calls this function for trace route, 
	// ANIFileList will not be intialized
	if (ANIFileList == NULL)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s ANI file list not initialized\n", fn));
		return NULL;
	}

	// Based on the given filename, search the ANIFileList for a matching
	// record. If not found, open the file and create a new record for it.
	// If found, check if the modification time on the file has changed
    // from the last time it was accessed. If it has, close the file,
    // delete it from the list, open it and add it back to the list.
	if (!(fileinfo = listSearchItem(ANIFileList, FileInfoCmp, filename)))
	{
		fileinfo = AddToANIFileList(filename);
	}
	else
	{
		if ((stat(filename, &filestat) < 0) || 
			(fileinfo->modtime != filestat.st_mtime))
		{
			NETERROR(MFIND,("%s File %s has changed, trying to reopen it\n",
				fn, filename));
			listDeleteItem(ANIFileList, fileinfo);
			free(fileinfo);
			fileinfo = AddToANIFileList(filename);
		}	
	}

	if (fileinfo)
	{
		return fileinfo->fileptr;
	}

	return NULL;
}

static FileInfo *
AddToANIFileList(char *filename)
{
	char fn[] = "AddToANIFileList():";
	FileInfo *fileinfo = NULL;
	FILE *fileptr = NULL;
	struct stat filestat = {0};

	if (stat(filename, &filestat) < 0)
	{
		NETERROR(MFIND,("%s Error in stat for file %s - %s\n",
			fn, filename, strerror(errno)));
		return NULL;
	}

	if (!(fileptr = fopen(filename, "r")))
	{
		NETERROR(MFIND,("%s Error opening file %s - %s\n",
			fn, filename, strerror(errno)));
		return NULL;
	}

	if (!(fileinfo = malloc(sizeof(FileInfo))))
	{
		NETERROR(MFIND,("%s Malloc error!!\n",fn));
		fclose(fileptr);
		return NULL;
	}

	memset(fileinfo, 0, sizeof(FileInfo));
	nx_strlcpy(fileinfo->filename, filename, PHONE_NUM_LEN);
	fileinfo->fileptr = fileptr;
	fileinfo->modtime = filestat.st_mtime;

	if (listAddItem(ANIFileList, fileinfo) < 0) 
	{
		NETERROR(MFIND,("%s Could not add file %s to list\n",fn, filename));
		fclose(fileptr);
		free(fileinfo);
		return NULL; 
	}

	return fileinfo; 
}

// Replace the phone number with a number from the given file
static int
GetNumberFromANIFile(char *phonenum, char *filename, FILE *fileptr)
{
    char buf[256];
    char num[256];

    // Read a number from the file
    while (fgets(buf, 256, fileptr))
    {
        if (sscanf(buf, "%s", num) > 0)
        {
            strncpy(phonenum, num, PHONE_NUM_LEN);
            return 0;
        }
    }
 
    return -1; 
}

// Compare function to compare FileInfo structures
static int
FileInfoCmp(void *item, void *key)
{
	FileInfo *fileinfo = (FileInfo *) item;
	char *filename = (char *) key;

	if (fileinfo)
	{
		return !strcmp(fileinfo->filename, filename);
	}

	return 0;
}

