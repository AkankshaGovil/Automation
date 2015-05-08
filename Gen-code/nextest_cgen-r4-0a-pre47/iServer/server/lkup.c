#include <stdio.h>
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "mem.h"
#include "ipcerror.h"
#include "handles.h"
#include "gw.h"
#include "calldefs.h"
#include "nxosd.h"
#ifdef NORESLOG
#include "nodebuglog.h"
#endif
#include "phonode.h"
#include "log.h"

int
SetPhonodeFromDb(PhoNode *phonode, InfoEntry *pentry)
{
	/* First set the phonode to 0 */
	memset(phonode, 0, sizeof(PhoNode));

	if (BIT_TEST(pentry->sflags, ISSET_REGID))
	{
		memcpy(phonode->regid, 
			pentry->regid, REG_ID_LEN);

		BIT_SET(phonode->sflags, ISSET_REGID);
	}

	if (((pentry->stateFlags & CL_ACTIVE) ||
		(pentry->stateFlags & CL_STATIC)) &&
		BIT_TEST((pentry->sflags|pentry->dflags), 
		      ISSET_IPADDRESS))
	{
	      phonode->ipaddress.l = pentry->ipaddress.l;
	      BIT_SET(phonode->sflags, ISSET_IPADDRESS);
	}

	if (BIT_TEST((pentry->sflags|pentry->dflags), 
		      ISSET_UPORT))
	{		 
	      phonode->uport = pentry->uport;
	      BIT_SET(phonode->sflags, ISSET_UPORT);
	}

	if (BIT_TEST((pentry->sflags|pentry->dflags), 
		      ISSET_PHONE))
	{
	      strcpy(phonode->phone, pentry->phone);
	      BIT_SET(phonode->sflags, ISSET_PHONE); 
	}

	if (BIT_TEST((pentry->sflags|pentry->dflags), 
		      ISSET_EMAIL))
	{
	      BIT_SET(phonode->sflags, ISSET_EMAIL); 
	}

	if (BIT_TEST((pentry->sflags), ISSET_NATIP))
	{
		BIT_SET(phonode->sflags, ISSET_NATIP);
	}

	if (BIT_TEST((pentry->sflags), ISSET_NATPORT))
	{
		BIT_SET(phonode->sflags, ISSET_NATPORT);
	}

	phonode->clientState =
			pentry->stateFlags;

	/* Short term hack - is static bit is set, set ACTIVE bit also,
	* as imobile does not check the static bit */
	if ((pentry->stateFlags & CL_STATIC) &&
		BIT_TEST(pentry->sflags, ISSET_IPADDRESS))
	{
		phonode->clientState |= CL_ACTIVE;
	}

	phonode->cap = pentry->cap;
	phonode->realmId = pentry->realmId;

	return 0;
}

int
SetRASInfoFromDb(ResolveHandle *rhandle, int branch, InfoEntry *pentry)
{
	rhandle->rfrasaddr = pentry->rasip;
	rhandle->rfrasport = pentry->rasport;
	rhandle->rfcallsigport = pentry->callsigport;
	return(0);
}

int
SetPhonodeFromFwdInfo(PhoNode *phonode, InfoEntry *pentry)
{
	/* First set the phonode to 0 */
	memset(phonode, 0, sizeof(PhoNode));

	if (BIT_TEST(pentry->nsflags, ISSET_PHONE))
	{
	      strcpy(phonode->phone, pentry->nphone);
	      BIT_SET(phonode->sflags, ISSET_PHONE); 
	}

	if (BIT_TEST(pentry->nsflags, ISSET_VPNPHONE))
	{
	      phonode->vpnExtLen = pentry->nvpnExtLen;
	      strcpy(phonode->vpnPhone, pentry->nvpnPhone);
	      BIT_SET(phonode->sflags, ISSET_VPNPHONE);

	}

	phonode->clientState = 0;

	return 0;
}

/*
 * CacheTableInfo: Cache Entry (IN/OUT)
 * branch: Forward/Rollover
 * rlevel: IN/OUT, indicates whether progress was made.
 * logfn: logging routine.
 * return value: 1 if further resolution can take place.
 * -1 for error.
 * 0 if no further resolution can happen. The cacheInfo
 * structure may contain modified information in this
 * case too.
 */
int
ResolveCacheEntry (
	char *sphone,
	CacheTableInfo *cacheInfoIn, 	/* IN */
	CacheTableInfo *cacheInfo, 		/* OUT */
	int fwdType, 		/* FORWARD=1 or ROLLOVER=0 */
	int	branch,			/* primary = 0, secondary = 1 */
	int *rlevel,		/* Depth */
	RouteLogFn logfn
)
{
	char 		fn[] = "ResolveCacheEntry():";
	InfoEntry 	*pentry, tmpInfo;
	char 		*nphone = 0;
	int 		rc, eprotocol;
	RouteNode	routeNode = { 0 };
	char newphone[25] = { 0 };
	unsigned long crflags;

	if (cacheInfo == 0)
	{
		NETERROR(MFIND,
			("%s No CacheInfo Entry passed\n", fn));
		return -1;
	}

	if (*rlevel == MAX_FIND_RECURSION)
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4, 
				("%s Reached Max depth for %s\n",
				fn,
				cacheInfo->data.phone));
		 return 0;
	}
	
	crflags = (fwdType==NEXTONE_REDIRECT_FORWARD)?CRF_FORWARD:CRF_ROLLOVER;
 	crflags |= CRF_CALLDEST|CRF_POSITIVE|CRF_REJECT;

	/* Input */
	pentry = &cacheInfoIn->data;

	if (logfn)
	{
		InitPhonodeFromInfoEntry(pentry, &routeNode.xphonode);
	}

	/* Check to see if entry is proxied by someone */
	if (((pentry->stateFlags &(CL_PROXY|CL_PROXYING)) == 
		(CL_PROXY|CL_PROXYING)) && (fwdType == NEXTONE_REDIRECT_FORWARD))
	{
#if 0
		NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Entry is Proxied by another entry %s %s\n",
				fn, pentry->xphone, pentry->xvpnPhone));
#endif		
		rc = BranchProxyPhone(pentry, cacheInfo, newphone);

		if (rc > 0)
		{
			if (logfn)
			{
				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				routeNode.forwardReason = nextoneProxied;
				routeNode.branch = branch;
				(*logfn)(&routeNode);
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			(*rlevel) ++;
			return 1;
		}
		else
		{
			memset(cacheInfo, 0, sizeof(CacheTableInfo));
			strcpy(cacheInfo->data.phone, newphone);
			BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);

			if (logfn)
			{
				routeNode.forwardReason = nextoneForwarded;

				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				(*logfn)(&routeNode);
				routeNode.branch = branch;
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			goto _return;
		}
	}

	/* Only one rollover happens in the lifetime of a find */
	eprotocol = pentry->protocol;
	
	/* If in forwarded state */
	if ((pentry->stateFlags & CL_FORWARD) &&
		(eprotocol == fwdType))
	{
		if (eprotocol == NEXTONE_REDIRECT_ROLLOVER)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Entry is rolled over to  another entry %s %s\n",
				fn, pentry->nphone, pentry->nvpnPhone));
		}
		else
		{
		 	NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Entry is fwded to  another entry %s %s\n",
				fn, pentry->nphone, pentry->nvpnPhone));
		}

		rc = BranchFwdPhone(pentry, cacheInfo, newphone);

		if (rc > 0)
		{
			if (logfn)
			{
				routeNode.forwardReason = 
					(eprotocol == NEXTONE_REDIRECT_ROLLOVER)?
					nextoneRollover:nextoneForwarded;
				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				routeNode.branch = branch;
				(*logfn)(&routeNode);
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			/* We have a locked entry, Find Info will release the locks
			* when its done 
			*/
			/* Recurse, if needed */
			
			(*rlevel) ++;

			return 1;
		 }
		 else
		 {
			memset(cacheInfo, 0, sizeof(CacheTableInfo));
			strcpy(cacheInfo->data.phone, newphone);
			BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);

			if (logfn)
			{
				routeNode.forwardReason = 
					(eprotocol == NEXTONE_REDIRECT_ROLLOVER)?
					nextoneRollover:nextoneForwarded;

				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				routeNode.branch = branch;

				(*logfn)(&routeNode);
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			/* Cant go ahead anymore... */
			/* its forwarded, but not in our cache */
		}
	}
	else if ((ApplyCPPolicy(pentry->cpname, 
				sphone,
				pentry->phone, 
				newphone, 
				crflags,
				APPLY_DEST,
				NULL,
				NULL,
				logfn) > 0))
	{
		/* We found a new phone number in the plan.
		 * Now lookup the phone in our cache. Initialize the entry,
		 * and just return it.
		 */
		rc = CacheFind(phoneCache, newphone,
				cacheInfo, sizeof(CacheTableInfo));

		if (rc > 0)
		{
			if (logfn)
			{
				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				routeNode.forwardReason = 
					(fwdType==NEXTONE_REDIRECT_FORWARD)?
						nextoneForwardedCP:nextoneRolloverCP;
				routeNode.branch = branch;
				(*logfn)(&routeNode);
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			(*rlevel) ++;
			return 1;
		}
		else
		{
			memset(cacheInfo, 0, sizeof(CacheTableInfo));
			strcpy(cacheInfo->data.phone, newphone);
			BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);

			if (logfn)
			{
				routeNode.forwardReason = 
					(fwdType==NEXTONE_REDIRECT_FORWARD)?
						nextoneForwardedCP:nextoneRolloverCP;

				InitPhonodeFromInfoEntry(&cacheInfo->data, 
					&routeNode.yphonode);
				routeNode.branch = branch;
				(*logfn)(&routeNode);
				memset(&routeNode, 0, sizeof(RouteNode));
			}

			/* Cant go ahead anymore... */
			/* its forwarded, but not in our cache */
		}
	}
	else if (fwdType != NEXTONE_REDIRECT_ROLLOVER)
	{
		memcpy(&cacheInfo->data, pentry, sizeof(InfoEntry));
	}
	
_return:
	return 0;
}

int
ResolvePhoneCache (
	ResolveHandle *rhandle,	/* resolve handle */
	CacheTableInfo *cacheInfo, /* Main cache entry (ROOT) */
	RouteLogFn logfn
)
{
	char fn[] = "ResolvePhoneCache():";
	int rlevel0 = 0, rlevel1 = 0, rc;
	InfoEntry 	*pentry;
	CacheTableInfo *cacheInfoOut;

	if (IsSecondaryResolve(rhandle))
	{
		/* Now do this for the alternate node */
		cacheInfoOut = RH_FOUND_CACHE(rhandle, 1);

		rc = ResolveCacheEntry(
				rhandle->phonodep->phone,
				cacheInfo, cacheInfoOut, 
				NEXTONE_REDIRECT_ROLLOVER,
				1, &rlevel1, logfn);
		if (rc > 0)
		{
			while ((rlevel1 < MAX_FIND_RECURSION) && 
				(ResolveCacheEntry(
					rhandle->phonodep->phone,	
					cacheInfoOut, cacheInfoOut, 
					NEXTONE_REDIRECT_FORWARD,
					1, &rlevel1, logfn) > 0));
		}

		if (rc >= 0)
		{
			pentry = &cacheInfoOut->data;
			SetPhonodeFromDb(RH_FOUND_NODE(rhandle, 1), pentry);
			SetRASInfoFromDb(rhandle, 1, pentry);
		}
	}

	if (IsPrimaryResolve(rhandle))
	{
		/* Do the primary now */
		cacheInfoOut = RH_FOUND_CACHE(rhandle, 0);

		rc = ResolveCacheEntry(
				rhandle->phonodep->phone,
				cacheInfo, cacheInfoOut, 
				NEXTONE_REDIRECT_FORWARD,
				0, &rlevel0, logfn);

		if (rc > 0)
		{
			while ((rlevel0 <= MAX_FIND_RECURSION) &&
				(ResolveCacheEntry(
					rhandle->phonodep->phone,
					cacheInfoOut, cacheInfoOut,
					NEXTONE_REDIRECT_FORWARD,
					0, &rlevel0, logfn) > 0));
		}

		pentry = &cacheInfoOut->data;
		SetPhonodeFromDb(RH_FOUND_NODE(rhandle, 0), pentry);
		SetRASInfoFromDb(rhandle, 0, pentry);
	}

	/* Forward as well as alternate nodes have been resolved */
	
	return (rlevel0+rlevel1);
}

/* logfn:: adjacencies are printed by the recursive function which we call
 * and the final values are printed by us.
 */
int
ResolvePhoneLocally(
	ResolveHandle *rhandle,
	RouteLogFn logfn
)
{
	char fn[] = "ResolvePhoneLocally():";
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;
	InfoEntry *entry = 0x0;
	char *rPhone, *rUrl, crname[CALLPLAN_ATTR_LEN];
	int checkZone = 0, checkVpnGroup = 0;
	int result, chainLookup;
	PhoNode *phonodep, *fphonodep, *rphonodep, *rfphonodep;
	RouteNode	routeNode = { 0 };
	int callError = SCC_errorNone, rc;
	char prevNumber[PHONE_NUM_LEN];

	/* temporary location for the cache entry which we look up */
	rphonodep = rhandle->phonodep;
	rfphonodep = rhandle->rfphonodep;

	checkZone = rhandle->checkZone;
	checkVpnGroup = rhandle->checkVpnGroup;

	if (logfn)
	{
		memcpy(&routeNode.xphonode, rfphonodep, sizeof(PhoNode));
	}

	ApplyANIPlan(
		rhandle->scpname,
		rhandle->phonodep->phone,
		rhandle->phonodep->phone,
		CRF_POSITIVE|CRF_REJECT|CRF_CALLORIGIN,
		APPLY_DEST,
		NULL,
		logfn);
		
		
	strcpy(prevNumber, rhandle->rfphonodep->phone);

	/* The iedge MUST have one route which allows
	 * the call to go through.
	 */
	if (ApplyNetworkPolicy(
		rhandle->sVpn.vpnName,
		rhandle->scpname,
		rhandle->phonodep->phone,
		rhandle->rfphonodep->phone,
		rhandle->rfphonodep->phone,
		CRF_POSITIVE|CRF_CALLORIGIN|CRF_VPNINT|CRF_VPNEXT,
		APPLY_DEST,
		NULL,
		crname,
		logfn ) < 0)
	{
		callError = SCC_errorInvalidPhone;
		goto _not_found;
	}
	if(strcmp(rhandle->rfphonodep->phone, prevNumber) != 0)
	{
		rhandle->routeflag |= SRC_ROUTE_APPLIED;
		if(rhandle->srccrname)
		{
			nx_strlcpy(rhandle->srccrname, crname, CALLPLAN_ATTR_LEN);
		}
	}

	NETDEBUG(MFIND, NETLOG_DEBUG4, 
		("%s phone = %s after src policy\n", fn, rhandle->rfphonodep->phone));

	cacheInfo = &cacheInfoEntry;
	rPhone = rfphonodep->phone;
	if (CacheFind(phoneCache, rfphonodep->phone, cacheInfo,
					sizeof(CacheTableInfo)) < 0)
	{
		if (CacheFind(uriCache, rfphonodep->phone, cacheInfo,
					sizeof(CacheTableInfo)) < 0)
		{
			cacheInfo = NULL;
		}
	}

	if (cacheInfo)
	{
		memcpy(&rhandle->fCacheInfoEntry, cacheInfo,
			sizeof(CacheTableInfo));

		InitPhonodeFromInfoEntry(&cacheInfo->data, &routeNode.yphonode);

		if (rhandle->primary == 0)
		{
			if (logfn)
			{
				routeNode.forwardReason = nextoneRegistration;

				routeNode.branch = rhandle->primary;
				(*logfn)(&routeNode);
			}

			if (cacheInfo->data.stateFlags & CL_DND)
			{
				if (logfn)
				{
					routeNode.forwardReason = nextoneDND;
					(*logfn)(&routeNode);
				}
				rhandle->primary = 1;
			}

			memset(&routeNode, 0, sizeof(RouteNode));
		}

		if (rhandle->primary == 0)
		{
			/* We are dealing w/ the primary entry,
			 * we can initialize what we found now
			 */
			memcpy(RH_FOUND_CACHE(rhandle, 0), cacheInfo, 
				sizeof(CacheTableInfo));
		}

		/* Recurse through entries, if needed */
		result = ResolvePhoneCache (rhandle, cacheInfo, logfn);

		if (result < 0)
		{
			if (logfn)
			{
				InitPhonodeFromInfoEntry(&cacheInfo->data, &routeNode.xphonode);
			}
			callError = SCC_errorTemporarilyUnavailable;
			goto _no_cache;
		}

		/* Check for DND */
		if (rfphonodep->clientState & CL_DND)
		{
			if (logfn)
			{
				memcpy(&routeNode.xphonode, rfphonodep, sizeof(PhoNode));
				routeNode.rejectReason = nextoneDND;
				(*logfn)(&routeNode);
			}

			/* Cannot proceed on this one anymore */
			callError = SCC_errorTemporarilyUnavailable;
			goto _not_found;
		}

		if (IsPrimaryResolve(rhandle))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4, 
				("%s -- primary destination --\n", fn));
		}
		else if (IsSecondaryResolve(rhandle))
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4, 
				("%s -- secondary destindation --\n", fn));
		}

		DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, rfphonodep);

		NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s -- * --\n", fn));

		rPhone = 0;

		if (BIT_TEST(rfphonodep->sflags, ISSET_PHONE) &&
			 !BIT_TEST(rfphonodep->sflags, ISSET_IPADDRESS))
		{
			  rPhone = &rfphonodep->phone[0];

			  NETDEBUG(MFIND, NETLOG_DEBUG2, 
					("%s Trying gateway for %s...\n", fn, rPhone));

			  if ((rc = GwLookup(rhandle, logfn)) < 0)
			  {
					NETDEBUG(MFIND, NETLOG_DEBUG2,
						 ("%s No Gateway found for entry\n", fn));

					callError = -rc;
		 			goto _not_found;
			  }
		 }
		 else
		 {
			  if (GwCheckRejectList(cacheInfo, rhandle->destRejectList, ""))
			  {
					NETDEBUG(MFIND, NETLOG_DEBUG2,
						 ("%s No Gateway found for entry\n", fn));

					callError = SCC_errorNoRoute;
		 			goto _not_found;
			  }
		 }
	}
	else
	{
		/* Must initialize the fCacheInfoEntry here from the rfphonode */
		memcpy(rhandle->fCacheInfoEntry.data.phone, rfphonodep->phone,
			PHONE_NUM_LEN);
		BIT_SET(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE);

		if (IsPrimaryResolve(rhandle))
		{
			/* Lookup gateways */
			goto _no_cache;
		}
		else
		{
			/* No cache entry found, this is a rollover resolution.
		 	* so we really have no rollover info.
		 	*/
			callError = SCC_errorNoRoute;
			goto _not_found;
		}
	}

	/* Now we must check again, for valid addresses, as the above check
	* may have reset some addresses.
	*/
	if (!BIT_TEST(rfphonodep->sflags, ISSET_IPADDRESS))
	{
		 NETDEBUG(MFIND, NETLOG_DEBUG2,
				("%s No Ip Address Found\n", fn));

		 callError = SCC_errorTemporarilyUnavailable;
		 goto _not_found;
	}

	DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, rfphonodep);

	strcpy(prevNumber, rhandle->rfphonodep->phone);

	if (ApplyCPPolicy(
		rhandle->scpname,
		rhandle->phonodep->phone,
		rhandle->rfphonodep->phone,
		rhandle->rfphonodep->phone,
		CRF_REJECT|CRF_CALLDEST,
		APPLY_DEST,
		NULL,
		crname,
		logfn ) < 0)
	{
		callError = SCC_errorInvalidPhone;
		goto _not_found;
	}

	if(strcmp(prevNumber, rhandle->rfphonodep->phone) != 0)
	{
		rhandle->routeflag |= DEST_ROUTE_APPLIED;
		if(rhandle->destcrname)
		{
			nx_strlcpy(rhandle->destcrname, crname, CALLPLAN_ATTR_LEN);
		}
	}

	ApplyANIPlan(
		RH_FOUND_CACHE(rhandle, 0)->data.cpname,
		rhandle->phonodep->phone,
		rhandle->phonodep->phone,
		CRF_POSITIVE|CRF_CALLDEST,
		APPLY_DEST,
		NULL,
		logfn);
		
	rhandle->result = CACHE_FOUND;

	return CACHE_FOUND; 

 _no_cache:

	strcpy(prevNumber, rhandle->rfphonodep->phone);
	// Apply Transit routes
	if ((rc = ApplyCPPolicy(
		"", "",
		rhandle->rfphonodep->phone,
		rhandle->rfphonodep->phone,
		CRF_REJECT|CRF_POSITIVE|CRF_TRANSIT,
		APPLY_DEST,
		NULL,
		crname,
		logfn )) < 0)
	{
		callError = -rc;
		goto _not_found;
	}

	if(strcmp(prevNumber, rhandle->rfphonodep->phone) != 0)
	{
		rhandle->routeflag |= TRANSIT_ROUTE_APPLIED;
		if(rhandle->transRouteNumber)
		{
			strcpy(rhandle->transRouteNumber, rhandle->rfphonodep->phone);
		}
		if(rhandle->transitcrname)
		{
			nx_strlcpy(rhandle->transitcrname, crname, CALLPLAN_ATTR_LEN);
		}
	}

	NETDEBUG(MFIND, NETLOG_DEBUG2, ("No cache. Trying gateway...\n"));

	if ((rc = GwLookup(rhandle, logfn)) >= 0)
	{
		DEBUG_PrintPhoNode(MFIND, NETLOG_DEBUG1, rfphonodep);

		ApplyANIPlan(
			RH_FOUND_CACHE(rhandle, 0)->data.cpname,
			rhandle->phonodep->phone,
			rhandle->phonodep->phone,
			CRF_POSITIVE|CRF_CALLDEST,
			APPLY_DEST,
			NULL,
			logfn);
		
		rhandle->result = CACHE_FOUND;

		return CACHE_FOUND;
	}
	else
	{
		// set the error, and let it fall through
		callError = -rc;
	}

 _not_found:

	// At this point callError should be set
	if (callError == SCC_errorNone)
	{
		NETERROR(MFIND,
			("%s Error not set, setting it to default\n", fn));

		callError = SCC_errorResourceUnavailable;
	}

	/* if no entry was found ...*/

	if (logfn)
	{
		/* Nothing was found, must log */
		routeNode.rejectReason = nextoneNoEntry;
		routeNode.branch = rhandle->primary;
		(*logfn)(&routeNode);
	}

	NETDEBUG(MFIND, NETLOG_DEBUG4,
		  ("%s Directory Lookup was unsuccessful \n", fn));

	rhandle->callError = callError;
	rhandle->result = CACHE_NOTFOUND;

	return CACHE_NOTFOUND; 
}

/* Apply the three kinds of policies:
 * Vpn internal,
 * Vpn External (Vpn group),
 * the iedge specific policy.
 * After that, we determine, based on some hardcoded
 * logic, what to do: ie., what is the new number, and
 * according to which policy.
 */
int
ApplyNetworkPolicy(
	char *vpnName,
	char *cpname,
	char *sphone,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int type,
	int *matchVal,
	char *crname,
	RouteLogFn logfn
)
{
	char fn[] = "ApplyNetwokPolicy():";
	int rc_vpni = 0, rc_vpne = 0, rc_iedge = 0;
	int mv_vpni = 0, mv_vpne = 0, mv_iedge = 0;
	int present = 0, rc = 0;
	CacheVpnEntry *cacheVpnEntry, tmpCacheVpnEntry;
	char tmpnewi[PHONE_NUM_LEN] = { 0 },
	     tmpnewe[PHONE_NUM_LEN] = { 0 },
	     tmpnew[PHONE_NUM_LEN] = { 0 };

	/* Do only negative routes in the beginning for iedge */
	rc_iedge = ApplyCPPolicy(cpname,
				sphone,
				phone, tmpnew, 
				crflags & (CRF_POSITIVE|CRF_REJECT|CRF_TRANSIT|
								CRF_CALLORIGIN|CRF_CALLDEST), 
				type,
				&mv_iedge,
				crname,
				logfn);

	if (rc_iedge < 0)
	{
		/* Dont need to go through the rest. */
		goto _evaluate_routing;
	}

	cacheVpnEntry = &tmpCacheVpnEntry;
	if (CacheFind(vpnCache, vpnName, cacheVpnEntry, 
				sizeof(CacheVpnEntry)) >= 0)
    {
		rc_vpni = ApplyVpnPolicy(&cacheVpnEntry->vpnEntry,
				phone, tmpnewi, crflags & CRF_VPNINT, type, &mv_vpni, logfn);
		
		if (type != APPLY_PREFIX)
		{
			rc_vpne = ApplyVpnGPolicy(cacheVpnEntry->vpnEntry.vpnGroup,
					phone, tmpnewe, crflags & CRF_VPNEXT, &mv_vpne, logfn);
		}
	}

	if (rc_vpni > 0)
	{
		/* Do once more */
		if (ApplyCPPolicy(cpname,
				sphone,
				tmpnewi, tmpnew, crflags, type, &mv_vpni, crname, logfn) < 0)
		{
			rc_vpni = -1;
		}
	}

	if (rc_vpne > 0)
	{
		/* Do once more */
		if (ApplyCPPolicy(cpname,
				sphone,
				tmpnewe, tmpnew, crflags, type, &mv_vpne, crname, logfn) < 0)
		{
			rc_vpne = -1;
		}
	}

_evaluate_routing:
	if ((rc_vpni < 0) || 
		(rc_vpne < 0) ||
		(rc_iedge < 0))
	{
		/* Some policy has rejected this */
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s Rejected rc_vpni=%d rc_vpne=%d, rc_iedge=%d\n",
			fn, rc_vpni, rc_vpne, rc_iedge));
		return -1;
	}

	if (rc_vpni > 0)
	{
		strcpy(newphone, tmpnewi);
		if (matchVal)
		{
			*matchVal = mv_vpni;
		}
	}
	else if (rc_vpne > 0)
	{
		strcpy(newphone, tmpnewe);
		if (matchVal)
		{
			*matchVal = mv_vpne;
		}
	}
	else if (rc_iedge > 0)
	{
		strcpy(newphone, tmpnew);
		if (matchVal)
		{
			*matchVal = mv_iedge;
		}
	}

	return (rc_vpni+rc_vpne+rc_iedge);
}

int
ApplyVpnGPolicy(
	char *vpng,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int *matchVal,
	RouteLogFn logfn
)
{
	char fn[] = "ApplyVpnGPolicy():";
	int present = 0, rc = 0;
	CacheVpnEntry *cacheVpnEntry, tmpCacheVpnEntry;
     	VpnEntry *vpnEntry;
	
	/* Go over the list if vpn's in this
	 * vpn group and apply the external policy of each
	 * one.
	 */
	cacheVpnEntry = &tmpCacheVpnEntry;
	for (present = CacheFindFirst(vpnCache, cacheVpnEntry, 
				sizeof(CacheVpnEntry));
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

	 	if (strcmp(vpnEntry->vpnGroup, vpng) != 0)
	 	{
	      		NETDEBUG(MFIND, NETLOG_DEBUG4,
		    		("vpn entry %s in grp %s is not in group %s", 
		     		vpnEntry->vpnId, vpnEntry->vpnGroup, vpng));
	      		continue;
	 	}
	
		rc = ApplyVpnPolicy(vpnEntry, phone, newphone,
					CRF_VPNEXT, APPLY_DEST, matchVal, logfn);
					
		if (rc > 0)
		{
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s Matched External Policy (%s/%s)\n",
				fn, vpng, vpnEntry->vpnName));
			break;
		}
	}

	CacheFreeIterator(vpnCache);

	/* We are not looking for negative routes here */
	return rc;
}

/* Apply the implicit route hidden inside the
 * vpn entry. The flags will determine which one
 * we look at, the external or the internal hidden
 * route.
 */
int
ApplyVpnPolicy(
	VpnEntry *vpn,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int type,
	int *matchVal,
	RouteLogFn logfn
)
{
	char fn[] = "ApplyVpnPolicy():";
	RouteNode routeNode = { 0 };
	VpnRouteEntry routeEntry = { 0 };
	char *outdest;
	int rc = -1, matchLen = 0, hasRejectAllElse = 0;

	routeEntry.destlen = vpn->vpnExtLen;
	memcpy(routeEntry.prefix, vpn->prefix, PHONE_NUM_LEN);

	if ((crflags&CRF_VPNEXT) || (type == APPLY_PREFIX))
	{
		memcpy(routeEntry.dest, vpn->vpnId, PHONE_NUM_LEN);
		routeEntry.destlen += strlen(vpn->vpnId);
	}

	if (type == APPLY_DEST)
	{
		rc = CRApplyDest(&routeEntry,
				phone,
				newphone,
				&matchLen,
				&hasRejectAllElse);
	}
	else if (type == APPLY_PREFIX)
	{
	   	rc = CRApplyPrefix(&routeEntry, 
				phone, 
				newphone,
				&matchLen);
	}

	if (matchVal)
	{
		*matchVal = matchLen;
	}

	return rc;
}

int 
IsRolledOver(char *phone)
{
	char fn[] = "IsRolledOver():";
	CacheTableInfo  cacheInfo;
	InfoEntry *pentry;

	if (!phone)
	{
		return 0;
	}

	if (CacheFind(phoneCache, phone,
			&cacheInfo, sizeof(CacheTableInfo)) > 0)
	{
		pentry = &cacheInfo.data;
		return(pentry->protocol == NEXTONE_REDIRECT_ROLLOVER);
	}
	else
	{
		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s phone lookup for %s in cache failed\n", fn, phone));
	}

	return 0;
}

