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
#include "phonode.h"
#include "resutils.h"

/* return 1 if looked up, and 0 if not
 * Also find the best gateway match. ie.,
 * do the "longest prefix match".
 * Also, if a valid src VpnGroup is specified,
 * check that the match being done is in the same 
 * vpn group. IF a zone is specified, Do not allow
 * the gateway to be accessed across zones.
 * Note that there are two locks being handled here.
 */
// DOES not implement route priority
int
GwLookup_debug(
	char 	*srcVpnGrp, 	/* src vpn group */
	char 	*srcVpnId, 	/* src vpn id */
	int 	checkVpnGrp,	/* execute vpn group policy ? */
	char 	*srcZone, 	/* src zone */
	int 	checkZone, 	/* execute zone policy ? */
	int 	isVpnCall,	/* is Vpn Call */
	PhoNode *destnode,	/* Dest phonode */
	PhoNode *phonode,	/* return phonode */
	CacheTableInfo *cacheInfo,	/* return cacheInfo entry */
	unsigned short *psigport, /* call signalling port - return value */
	int 	primary,
	int		reservePort, /* Reserve the gateway port if nonzero then caller */
						/* has to free the gwport when its done using it */
	int 	phoneChange, /* Are we wo routing the call */
	RouteLogFn logfn
)
{
	char fn[] = "GwLookup():";
	CacheTableInfo *info, infoCopy, *gwCacheEntry, *prefGw = 0;
	InfoEntry *pentry;
	int matchVpnGrps = 0;
	int confidenceVal, oldConfidenceVal = -1;
	VpnEntry dstVpn;
	char *phone; 	/* phone to be looked up */
	struct gw_match_set newset;
	struct gw_match_set oldset = { 0 }; // For no default matching
	VpnRouteKey routeKey = { 0 };
	CacheVpnRouteEntry *cacheRouteEntry = 0, *cacheRouteEntryMatch = 0;
	CacheCPBEntry *cacheCPBEntry = 0;
	int plen = 0, ngw = 0, 
		 hasReject = 0, hasRejectAllElse = 0, hasMatch = 0,
		 tmphasReject = 0, tmphasMatch = 0;
	int rc, matchLen = 0, inMatchLoop, gwReason;
	char *outdest;
	RouteNode	routeNode = { 0 };
	int cmpResult;

	// Initialize the oldset
	if (allowDestAll)
	{
		oldset.zoneVal = -1;
		oldset.vpnVal = -1;
		oldset.cpVal = -1;
		oldset.gwPriority = -1;
	}
	else
	{
		memset(&oldset, 0, sizeof(struct gw_match_set));
	}

	oldset.cTime = time(0);

	if (isVpnCall)
	{
		phone = destnode->vpnPhone;
	}
	else
	{
		phone = destnode->phone;
	}

	DEBUG(MFIND, NETLOG_DEBUG2,
		("%s phone %s for gateway lookup\n", fn, phone));

	DEBUG(MFIND, NETLOG_DEBUG2,
		("%s isVpnCall %d checkVpnGroup %d checkZone %d\n", 
			fn, isVpnCall, checkVpnGrp, checkZone));

	/* What we will do for now is try to walk the list and see
	 * if a gateway prefix can match the phonumber prefix.
	 */

	if (logfn)
	{
		memcpy(&routeNode.xphonode, destnode, sizeof(PhoNode));
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	for (gwCacheEntry = CacheGetFirst(gwCache); gwCacheEntry;
		gwCacheEntry = CacheGetNext(gwCache, &gwCacheEntry->data))
	{
		ngw ++;

		DEBUG(MFIND, NETLOG_DEBUG2,
			("GwLookup:: Examining gateway no %d\n", ngw));

		DEBUG(MFIND, NETLOG_DEBUG2,
			("gateway %s %lu %s %s\n",
				gwCacheEntry->data.regid,
				gwCacheEntry->data.uport,
				gwCacheEntry->data.phone,
				gwCacheEntry->data.vpnPhone));

		if (logfn)
		{
			InitPhonodeFromInfoEntry(&gwCacheEntry->data, &routeNode.yphonode);
		}

		if (!(gwCacheEntry->data.stateFlags & CL_STATIC) &&
			!(gwCacheEntry->data.stateFlags & CL_ACTIVE))
		{
			DEBUG(MFIND, NETLOG_DEBUG2,
				("gateway entry is neither STATIC nor ACTIVE - REJECTED\n"));
			if (logfn)
			{
				routeNode.rejectReason = nextoneGatewayInactive;
				routeNode.forwardReason = 0;
				routeNode.branch = primary;
				logfn(&routeNode);
			}
			continue;
		}

		if ((gwCacheEntry->data.stateFlags & CL_DND) || 
			((gwCacheEntry->data.ncalls >= gwCacheEntry->data.xcalls) &&
			(gwCacheEntry->data.xcalls!=0)))
		{
			 DEBUG(MFIND, NETLOG_DEBUG2,
				   ("gateway is in a CALL - REJECTED. Max Calls = %d active Calls = %d\n",
				   gwCacheEntry->data.xcalls,gwCacheEntry->data.ncalls));
			if (logfn)
			{
				routeNode.rejectReason = nextoneGatewayInCall;
				routeNode.forwardReason = 0;
				routeNode.branch = primary;
				logfn(&routeNode);
			}
			continue;
		}

		/* release the locks, lookup the vpn group database
		 * if needed
		 */
		/* A gateway in NULL zone should be available for
		 * global usage, otherwise the src and dst zones 
		 * should match
		 */
		if (checkZone && strlen(gwCacheEntry->data.zone))
		{
			if (strcmp(srcZone, gwCacheEntry->data.zone) != 0)
			{
				DEBUG(MFIND, NETLOG_DEBUG2,
					("Zones (%s) of source and (%s) of gateway dont match - REJECTED\n", 
					srcZone, gwCacheEntry->data.zone));

				if (logfn)
				{
					routeNode.rejectReason = nextoneZoneMismatch;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}
				continue;
			}

			/* Zones actually match */
			newset.zoneVal = 1;
		}
		else
		{
			newset.zoneVal = -1;
		}

		DEBUG(MFIND, NETLOG_DEBUG2,
			("Zones (%s) of src and (%s) of gateway match\n", 
			srcZone, gwCacheEntry->data.zone));

		if (strlen(srcVpnGrp) &&
			checkVpnGrp && 
			(GetIedgeVpn(gwCacheEntry->data.vpnName, &dstVpn) >= 0))
		{
			matchVpnGrps = 0;

			DEBUG(MFIND, NETLOG_DEBUG4,
				("Dst Vpn Group is %s srvVpnGroup is %s\n",
					dstVpn.vpnGroup, srcVpnGrp));

			if (strcmp(dstVpn.vpnGroup, srcVpnGrp) != 0)
			{
				DEBUG(MFIND, NETLOG_DEBUG2,
					("Src(%s) And Dest(%s) are not in the same grps - REJECTED\n", 
					srcVpnGrp, dstVpn.vpnGroup));

				if (logfn)
				{
					routeNode.rejectReason = nextoneVpnGroupMismatch;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}
				continue;
			}
			matchVpnGrps = 1;

			/* Vpn groups match . Check if the VpnId's match */
			if (!strcmp(dstVpn.vpnId, srcVpnId))
			{
				newset.vpnVal = 1;
			}
			else
			{
				newset.vpnVal = -1;
			}
		}
		else
		{
			newset.vpnVal = -1;
		}

		/* Check the dest prefix routes */
		if (strlen(gwCacheEntry->data.cpname))
		{
			// GLOBALS

			hasReject = 0;
			hasRejectAllElse = 0;
			hasMatch = 0;
			cacheRouteEntryMatch = 0;
			matchLen = 0;

			DEBUG(MFIND, NETLOG_DEBUG4,
				("%s gateway is in calling plan %s, examining routes\n",
					fn, gwCacheEntry->data.cpname));

			/* Look at the bindings.
			 * Of all the routes in a calling plan,
			 * if there is more than one match, the last
			 * match will be taken. If there is a reject,
			 * the entire plane will be rejected.
			 */
			inMatchLoop = 0;
			for (cacheCPBEntry = CacheGetFirst(cpbCache);
				cacheCPBEntry;
				cacheCPBEntry = CacheGetNext(cpbCache, 
							&cacheCPBEntry->cpbEntry))	
			{
				if (strcmp(cacheCPBEntry->cpbEntry.cpname, 
					gwCacheEntry->data.cpname))
				{
					if (!inMatchLoop)
					{
						continue;
					}
					else
					{
						// We have exhausted the plans which match
						break;
					}
				}
	
				inMatchLoop = 1;
				
				if ((cacheCPBEntry->cpbEntry.crflags & CRF_TIME) &&
					(CRApplyTime(&cacheCPBEntry->cpbEntry) < 0))
				{
					/* This route does not apply right now.
					 */
					NETDEBUG(MFIND, NETLOG_DEBUG4, 
					("route  %s/%s not applicable at this time\n",
					cacheCPBEntry->cpbEntry.cpname,
					cacheCPBEntry->cpbEntry.crname));

					if (logfn)
					{
						routeNode.rejectReason = nextoneTimeMismatch;
						routeNode.forwardReason = 0;
						routeNode.branch = primary;
						logfn(&routeNode);
					}

					continue;
				}

				/* Found a route, extract it now */
				strncpy(routeKey.crname, cacheCPBEntry->cpbEntry.crname, 
					CALLPLAN_ATTR_LEN);
				cacheRouteEntry = CacheGet(cpCache, &routeKey);

				if (cacheRouteEntry == NULL)
				{
					NETERROR(MFIND, 
						("Found No route corresponding to %s\n",
						routeKey.crname));
					continue;
				}

				if ((cacheRouteEntry->routeEntry.crflags & CRF_CALLDEST) ==
					0)
				{
					NETDEBUG(MFIND, NETLOG_DEBUG4,
						("Found call src route corresponding to %s\n",
						routeKey.crname));
					continue;
				}

				DEBUG(MFIND, NETLOG_DEBUG4,
					("\tfound a route %s/%s/%s\n",
					cacheRouteEntry->routeEntry.crname,
					cacheRouteEntry->routeEntry.dest,
					cacheRouteEntry->routeEntry.prefix));

				tmphasMatch = 0;
				tmphasReject = 0;

				rc = CPMatchPattern(cacheRouteEntry->routeEntry.dest,
								 &outdest, 
								 &tmphasMatch, 
								 &hasRejectAllElse,
								 &tmphasReject, phone);

				if (tmphasReject)
				{
					 NETDEBUG(MFIND, NETLOG_DEBUG4,
						   ("\tmatched a reject route\n"));
					 hasReject = tmphasReject;
					 break;
				}
				else if (tmphasMatch)
				{
					if ((cacheRouteEntry->routeEntry.destlen) &&
						(cacheRouteEntry->routeEntry.destlen != strlen(phone)))
					{
						/* Cannot consider this one as a match */
					 	NETDEBUG(MFIND, NETLOG_DEBUG4,
						   ("\tdestlen mismatch\n"));
						continue;
					}

					NETDEBUG(MFIND, NETLOG_DEBUG4, 
						("\tmatched, len=%d.\n", rc));

					// Either the first match,
					// Or the second one has to be better
					if ((!hasMatch) ||
						((hasMatch) && (rc >= matchLen)))
					{
						cacheRouteEntryMatch = cacheRouteEntry;
				  		matchLen = rc;
						/* set the globals to the tmps */

						hasMatch = tmphasMatch;
					}
				}

			} /* for - loop over all routes in a plan */

			/* Look up the cache for cp/dest */
			if (hasReject)
			{
				/* Case 2.  Has a reject route
				 */
				/* Less than what the default preference is */
				newset.cpVal = -2;	/* Dont match anything */
				if (logfn)
				{
					routeNode.rejectReason = nextoneHasReject;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}
			}
			else if (cacheRouteEntryMatch)
			{
				/* Case 1. Has a Route */
				newset.cpVal = matchLen+1;
				memcpy(&newset.route, 
					&cacheRouteEntryMatch->routeEntry,
					sizeof(VpnRouteEntry));
			}
			else if (hasRejectAllElse == 1)
			{
				/* Case 3. Does not have a route and has a 
				 * a global reject route.
				 */
				/* Less than what the default preference is */
				newset.cpVal = -2;	/* Dont match anything */
				if (logfn)
				{
					routeNode.rejectReason = nextoneHasReject;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}
			}
			else
			{
				newset.cpVal = -1;
			}
		}
		else
		{
			newset.cpVal = -1;
		}

		newset.cTime = gwCacheEntry->data.cTime;
		newset.gwPriority = gwCacheEntry->data.priority;

		cmpResult = GwCompare(&newset, &oldset, &gwReason);
		if ((cmpResult > 0) || (!prefGw && (cmpResult == 0)))
		{
			 /* new set is preferable */
			 oldset = newset;
			 prefGw = gwCacheEntry;
			 memcpy(&infoCopy, prefGw, sizeof(infoCopy));
			 NETDEBUG(MFIND, NETLOG_DEBUG2,
				   ("Selected new gateway - (vpn %d zone %d cpval %d)\n",
					newset.vpnVal, newset.zoneVal, 
					newset.cpVal));
			if (logfn)
			{
				routeNode.rejectReason = 0;
				routeNode.forwardReason = gwReason;
				routeNode.branch = primary;
				logfn(&routeNode);
			}
		}
		else
		{
			if (logfn)
			{
				routeNode.rejectReason = gwReason;
				routeNode.forwardReason = 0;
				routeNode.branch = primary;
				logfn(&routeNode);
			}
		}
	}

	CacheFreeIterator(gwCache);

	if(prefGw && reservePort)
	{
		if(ctInfoAllocCall(prefGw,1,1)!=0)
		{
			/* should never fail here */
			NETERROR(MFIND, 
				("%s ctInfoAllocCall failed\n", fn));
		}			
			;
		// timestamp the gw
		prefGw->data.cTime = time(0);
	}

	CacheReleaseLocks(vpnCache);
	CacheReleaseLocks(cpCache);
	CacheReleaseLocks(cpbCache);
	CacheReleaseLocks(regCache);

	if (prefGw)
	{
		info   = (CacheTableInfo *)&infoCopy;

		if (cacheInfo)
		{
			memcpy(cacheInfo, info, sizeof(CacheTableInfo));
		}

		DEBUG(MFIND, NETLOG_DEBUG2, ("Gateway selected finally - \n"));

		pentry = &info->data;
		memcpy(phonode->regid,
			   pentry->regid, REG_ID_LEN);
		phonode->uport = pentry->uport;

		BIT_SET(phonode->sflags, ISSET_REGID);
		BIT_SET(phonode->sflags, ISSET_UPORT);

		if (BIT_TEST((pentry->sflags|pentry->dflags),
					 ISSET_IPADDRESS))
		{	
			 phonode->ipaddress.l =
				  pentry->ipaddress.l;
			 *psigport = pentry->callsigport;
			 BIT_SET(phonode->sflags, ISSET_IPADDRESS);
		}

		if (!BIT_TEST(phonode->sflags, ISSET_UPORT))
		{
			 if (BIT_TEST((pentry->sflags|pentry->dflags),
						  ISSET_UPORT))
			 {
				  phonode->uport = pentry->uport;
				  BIT_SET(phonode->sflags, ISSET_UPORT);
			 }
		}

		phonode->cap = pentry->cap;
		phonode->clientState = pentry->stateFlags;

		phoneChange = (pentry->ispcorgw != IEDGE_1000) && phoneChange;

		NETDEBUG(MFIND, NETLOG_DEBUG4,
			("%s phone change = %d\n", fn, phoneChange));

		/* If the gateway selected is a 1000, then we should not
		 * alter any numbers. Rel 1.2
		 */
		if (oldset.cpVal > 0)
		{
			char phone[PHONE_NUM_LEN] = { 0 };

			oldset.cpVal --;

			/* Take off the dest from the dest phone,
			 * and put the prefix instead.
			 */

			strcpy(phone, oldset.route.prefix);

			if (isVpnCall)
			{
				nx_strlcat(phone, phonode->vpnPhone+oldset.cpVal,
					PHONE_NUM_LEN);
				if (phoneChange)
					strcpy(phonode->vpnPhone, phone);

				if (isVpnCall != 1)
				{
					BIT_RESET(phonode->sflags, ISSET_PHONE);
				}
			}
			else
			{
				nx_strlcat(phone, phonode->phone+oldset.cpVal,
					PHONE_NUM_LEN);
				if (phoneChange)
					strcpy(phonode->phone, phone);
				strncpy(cacheInfo->data.phone, phonode->phone, PHONE_NUM_LEN);
			}

			DEBUG(MFIND, NETLOG_DEBUG2, 
				("New Phone to be dialled on gateway is %s\n", phone));
		}
		else if (isVpnCall)
		{
			int plen = strlen(phonode->vpnPhone) - phonode->vpnExtLen;

			/* Strip off the VPN id, to preserve old behavior */
			if (plen > 0)	
			{
				char phone[PHONE_NUM_LEN] = { 0 };
				strcpy(phone, phonode->vpnPhone + plen);
				if (phoneChange)
					strcpy(phonode->vpnPhone, phone);
			}	

			DEBUG(MFIND, NETLOG_DEBUG2, 
				("New Vpn Phone to be dialled on gateway is %s\n", phonode->vpnPhone));
		}

		BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);
		BIT_RESET(cacheInfo->data.sflags, ISSET_VPNPHONE);
		strncpy(cacheInfo->data.phone, phonode->phone, PHONE_NUM_LEN);

		return 1;
	}

	return -1;
}
