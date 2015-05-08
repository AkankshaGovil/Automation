#include <strings.h>
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "mem.h"
#include "serverp.h"
#include "ipcerror.h"
#include "gw.h"
#include "lsconfig.h"
#include "calldefs.h"
#include <malloc.h>
#include "tags.h"
#include <ssip.h>

#ifdef NORESLOG
#include "nodebuglog.h"
#endif
#include "nxosd.h"
#include "phonode.h"
#include "resutils.h"
#include "dbs.h"


char * strdupextra(char *src, int extra);

// return < 0 and indicate error
// return 0 if error doesnt matter, but stull no match
// return 1 if match
int
GwCheckHuntParams(
	ResolveHandle *rhandle,
	CacheTableInfo *gwCacheEntry,
	struct gw_match_set *newset,
	int 	primary,
	RouteNode	*routeNode,
	RouteLogFn logfn
)
{
	DEBUG(MFIND, NETLOG_DEBUG2,
		("gateway %s %lu %s %s\n",
			gwCacheEntry->data.regid,
			gwCacheEntry->data.uport,
			gwCacheEntry->data.phone,
			gwCacheEntry->data.vpnPhone));

	if (!(gwCacheEntry->data.stateFlags & CL_STATIC) &&
		!(gwCacheEntry->data.stateFlags & CL_ACTIVE))
	{
		DEBUG(MFIND, NETLOG_DEBUG2,
			("gateway entry is neither STATIC nor ACTIVE - REJECTED\n"));
		if (logfn)
		{
			routeNode->rejectReason = nextoneGatewayInactive;
			routeNode->forwardReason = 0;
			routeNode->branch = primary;
			logfn(routeNode);
		}
		return -SCC_errorResourceUnavailable;
	}

	if ((gwCacheEntry->data.ipaddress.l == (unsigned long)-1) ||
		!BIT_TEST(gwCacheEntry->data.sflags, ISSET_IPADDRESS))
	{
		NETERROR(MFIND,
			("gateway entry is ACTIVE but no ip found - REJECTED\n"));
		if (logfn)
		{
			routeNode->rejectReason = nextoneGatewayInactive;
			routeNode->forwardReason = 0;
			routeNode->branch = primary;
			logfn(routeNode);
		}
		return -SCC_errorResourceUnavailable;
	}

	if ((gwCacheEntry->data.stateFlags & CL_DND) || 
		ctInfoAvailCall(gwCacheEntry, 1, 1) != 0 )
	{
		if (logfn)
		{
			routeNode->rejectReason = nextoneGatewayInCall;
			routeNode->forwardReason = 0;
			routeNode->branch = primary;
			logfn(routeNode);
		}
		return -SCC_errorResourceUnavailable;
	}

	return 1;
}

int
GwCheckConfigParams(
	ResolveHandle *rhandle,
	CacheTableInfo *gwCacheEntry,
	struct gw_match_set *newset,
	int 	primary,
	RouteNode	*routeNode,
	RouteLogFn logfn
)
{
	VpnEntry dstVpn;

	DEBUG(MFIND, NETLOG_DEBUG2,
		("gateway %s %lu %s %s\n",
			gwCacheEntry->data.regid,
			gwCacheEntry->data.uport,
			gwCacheEntry->data.phone,
			gwCacheEntry->data.vpnPhone));

	if ((IedgeXOutCalls(&gwCacheEntry->data) < 0) ||
		(IedgeXCalls(&gwCacheEntry->data) < 0))
	{
		NETDEBUG(MFIND,NETLOG_DEBUG4,
			("gw is in a CALL - REJECTED regid = %s/%lu xcalls = %d xin = %d xout = %d"
			"nin = %d nout = %d",
			gwCacheEntry->data.regid,gwCacheEntry->data.uport,
			IedgeXCalls(&gwCacheEntry->data), IedgeXInCalls(&gwCacheEntry->data), IedgeXOutCalls(&gwCacheEntry->data),
			IedgeInCalls(&gwCacheEntry->data), IedgeOutCalls(&gwCacheEntry->data)));

		if (logfn)
		{
			routeNode->rejectReason = nextoneGatewayInCall;
			routeNode->forwardReason = 0;
			routeNode->branch = primary;
			logfn(routeNode);
		}
		return -SCC_errorResourceUnavailable;
	}

	/* A gateway in NULL zone should be available for
	 * global usage, otherwise the src and dst zones 
	 * should match
	 */
	if (strlen(gwCacheEntry->data.zone))
	{
		if (strcmp(rhandle->sZone, gwCacheEntry->data.zone) != 0)
		{
			DEBUG(MFIND, NETLOG_DEBUG2,
			("Zones (%s) of source and (%s) of gateway dont match - REJECTED\n", 
			rhandle->sZone, gwCacheEntry->data.zone));

			if (logfn)
			{
				routeNode->rejectReason = nextoneZoneMismatch;
				routeNode->forwardReason = 0;
				routeNode->branch = primary;
				logfn(routeNode);
			}
			return -SCC_errorBlockedUser;
		}

		/* Zones actually match */
		newset->zoneVal = 1;
	}
	else
	{
		newset->zoneVal = 0;
	}

	DEBUG(MFIND, NETLOG_DEBUG2,
		("Zones (%s) of src and (%s) of gateway match\n", 
		rhandle->sZone, gwCacheEntry->data.zone));

	if (strlen(rhandle->sVpn.vpnGroup) &&
		(GetIedgeVpn(gwCacheEntry->data.vpnName, &dstVpn) >= 0))
	{
		DEBUG(MFIND, NETLOG_DEBUG4,
			("Dst Vpn Group is %s srvVpnGroup is %s\n",
				dstVpn.vpnGroup, rhandle->sVpn.vpnGroup));

		if (strcmp(dstVpn.vpnGroup, rhandle->sVpn.vpnGroup) != 0)
		{
			DEBUG(MFIND, NETLOG_DEBUG2,
				("Src(%s) And Dest(%s) are not in the same grps - REJECTED\n", 
				rhandle->sVpn.vpnGroup, dstVpn.vpnGroup));

			if (logfn)
			{
				routeNode->rejectReason = nextoneVpnGroupMismatch;
				routeNode->forwardReason = 0;
				routeNode->branch = primary;
				logfn(routeNode);
			}
			return -SCC_errorBlockedUser;
		}

		/* Vpn groups match . Check if the VpnId's match */
		if (!strcmp(dstVpn.vpnId, rhandle->sVpn.vpnId))
		{
			newset->vpnVal = 1;
		}
		else
		{
			newset->vpnVal = -1;
		}
	}
	// if either rhandle has a vpn group name or if the gw has a vpn group
	// associated with it, there has been a mismatch
	else if(strlen(rhandle->sVpn.vpnGroup) || (GetIedgeVpn(gwCacheEntry->data.vpnName, &dstVpn) >= 0))
	{
		newset->vpnVal = -1;
	}
	else
	{
		newset->vpnVal = 0;
	}

	return 1;
}

int
GwFreeRejectList(ListEntry *head, void (*freefn)(void *))
{
	ListEntry *elem, *list;

	if (head == NULL)
	{
		return 0;
	}

	list = head;

	do
	{
		elem = list; 
		list = list->next;

		freefn(elem);
	}
	while (list != head);

	return 0;
}

// Reject List is a list of gw_match_set
int
GwCheckRejectList(CacheTableInfo *gwCacheEntry, ListEntry *rejectList, char *crname)
{
	char fn[] = "GwCheckRejectList()";
	struct gw_match_set *gwRejectList;
	
	if (rejectList == NULL)
	{
		return 0;
	}

	gwRejectList = (struct gw_match_set *)rejectList;

	do
	{
		// == '\0' check takes care of checking for hairpin situations

		if ((gwRejectList->route.crname[0] != '\0') &&
				gwRejectList->route.crflags & CRF_STICKY &&
				strcmp(crname, gwRejectList->route.crname))
		{
			// route is different from sticky route.
			// Reject this!!
			return 1;
		}

		// non sticky route logic
		if ((gwRejectList->route.crname[0] == '\0') ||
				!strcmp(crname, gwRejectList->route.crname))
		{
			if (BIT_TEST(gwCacheEntry->data.sflags, ISSET_IPADDRESS))
			{
				if (gwCacheEntry->data.ipaddress.l == gwRejectList->ipaddr)
				{
					return 1;
				}
			}
	
			if (!memcmp(&gwCacheEntry->data, gwRejectList->regid, 
					sizeof(NetoidSNKey)))
			{
				return 1;
			}
		}
	}
	while ((gwRejectList = gwRejectList->next) != 
				(struct gw_match_set *)rejectList);

	return 0;
}

int
GwAddPhoNodeToRejectList(PhoNode *phonode, char *crname, ListEntry **rejectList, 
		void *(*mallocfn)(size_t))
{
	char fn[] = "GwAddPhoNodeToRejectList()";
	struct gw_match_set *gwRejectList, *gwNode;
	CacheRouteEntry *cacheRouteEntry;
	
	gwRejectList = (struct gw_match_set *)rejectList;

	gwNode = (struct gw_match_set *)mallocfn(sizeof(struct gw_match_set)); 
	memset(gwNode, 0, sizeof(struct gw_match_set));
	
	if (phonode->regid[0])
	{
		memcpy(gwNode->regid, phonode->regid, sizeof(NetoidSNKey));
	}

	if (BIT_TEST(phonode->sflags, ISSET_IPADDRESS))
	{
		gwNode->ipaddr = phonode->ipaddress.l;
	}

	if (crname && (crname[0] != '\0'))
	{
		// Lookup the route to get some info on flags like sticky etc...
		CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
		
		cacheRouteEntry = CacheGet(cpCache, crname);
		if (cacheRouteEntry)
		{
			memcpy(&gwNode->route, &cacheRouteEntry->routeEntry, sizeof(RouteEntry));
		}
		else
		{
			strcpy(gwNode->route.crname, crname);

			// This is weird, flag an error
			NETERROR(MDEF, ("%s route %s has disappeared!\n", fn, crname));	
		}

		CacheReleaseLocks(cpCache);
		
	}

	if (*rejectList)
	{
		ListInsert((*rejectList)->next, gwNode);
	}	
	else
	{
		ListInitElem(gwNode);
		*rejectList = (ListEntry *)gwNode;
	}

	return 0;
}

/* return 1 if looked up, and 0 if not
 * Also find the best gateway match. ie.,
 * do the "longest prefix match".
 * Also, if a valid src VpnGroup is specified,
 * check that the match being done is in the same 
 * vpn group. IF a zone is specified, Do not allow
 * the gateway to be accessed across zones.
 * Note that there are two locks being handled here.
 * In case or error return < 0, and set the returned value to
 * -sccError
 */
int
GwLookup(
	ResolveHandle *rhandle,
	RouteLogFn logfn
)
{
	char fn[] = "GwLookup():";
	CacheTableInfo *info, *gwCacheEntry, *prefGw = 0, *startGwEntry = 0;
	CacheTableInfo *cacheInfo;
	InfoEntry *pentry;
	char *phone, *prefix = NULL; 	/* phone to be looked up */
	struct gw_match_set newset = { 0 }, oldset = { 0 };
	CacheVpnRouteEntry *cacheRouteEntry = 0, *cacheRouteEntryMatch = 0;
	CacheCPBEntry *cacheCPBEntry = 0;
	int plen = 0, ngw = 0;
	int rc, matchLen = 0, gwReason, i;
	RouteNode	routeNode = { 0 };
	CacheRouteEntry *startRouteEntry;
	CacheCPBEntry *startCPBEntry;
	int callError = SCC_errorNone;
	PhoNode *destnode, *phonode;
	int 	primary, reservePort, phoneChange;
	int cmpResult;

	destnode = rhandle->rfphonodep;
	phonode = rhandle->rfphonodep;
	cacheInfo = RH_FOUND_CACHE(rhandle, 0);

	primary = rhandle->primary;
	reservePort = rhandle->reservePort;
	phoneChange = PhoneChange(rhandle);	

 	phone = destnode->phone;

	if (logfn)
	{
		memcpy(&routeNode.xphonode, destnode, sizeof(PhoNode));
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(vpnCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);
		
	oldset.cTime = sglobalTime;
	oldset.utilization = 1;

	if ( rhandle->dtg && rhandle->dtg[0] && 
		 (startGwEntry = CacheGet(dtgCache, rhandle->dtg)))
	{
		gwCacheEntry = startGwEntry;

		do
		{
			if (logfn)
			{
				InitPhonodeFromInfoEntry(&gwCacheEntry->data, 
					&routeNode.yphonode);
			}

			// A gateway is available, so if we don't find one it should be no-ports now			
			callError = SCC_errorNoPorts;

			if (GwCheckHuntParams(rhandle, gwCacheEntry, &newset, primary,
				&routeNode, logfn) < 0)
			{
				time(&gwCacheEntry->data.attTime);
				continue;
			}

			// Check to see if the current gw is in the reject list
			if (GwCheckRejectList(gwCacheEntry, rhandle->destRejectList, ""))
			{
				DEBUG(MFIND, NETLOG_DEBUG2,
					("GwLookup:: Gw in Reject List\n"));

				continue;
			}

			newset.cpVal = 0;

			newset.cTime = gwCacheEntry->data.cTime;
			newset.gwPriority = gwCacheEntry->data.priority;
			newset.gwFlags = gwCacheEntry->data.stateFlags;
			if ((cpRoutingPolicy == CP_ROUTE_UTILZ) &&
					(IedgeXCalls(&gwCacheEntry->data) > 0))
			{
				newset.utilization = 
					((float)IedgeCalls(&gwCacheEntry->data))/
						IedgeXCalls(&gwCacheEntry->data);
			}
			else
			{
				newset.utilization = 0;
			}

			cmpResult = GwCompare(&newset, &oldset, &gwReason);
			if ((cmpResult > 0) || (!prefGw && (cmpResult == 0)))
			{
	 			/* new set is preferable */
	 			oldset = newset;
	 			prefGw = gwCacheEntry;

	 			NETDEBUG(MFIND, NETLOG_DEBUG2,
		   			("Selected new gateway - (cpval %d)\n", i));

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
		} while (startGwEntry != (gwCacheEntry = (CacheTableInfo*)Listg(gwCacheEntry, sizeof(ListEntry))->next));

		if (prefGw)
		{
			goto _found_gw; 
		}
		else
		{
			goto _no_gw_found; 
		}
	}
	
	// Initialize the oldset
	if (allowDestAll)
	{
		oldset.zoneVal = -1;
		oldset.vpnVal = -1;
		oldset.cpVal = -1;
		oldset.gwPriority = -100;
		oldset.routePriority = -1;
	}
	else
	{
		memset(&oldset, 0, sizeof(struct gw_match_set));
	}


	DEBUG(MFIND, NETLOG_DEBUG2,
		("%s phone %s for gateway lookup\n", fn, phone));

	DEBUG(MFIND, NETLOG_DEBUG2,
		("%s checkVpnGroup %d checkZone %d\n", 
			fn, rhandle->checkVpnGroup, rhandle->checkZone));

	/* What we will do for now is try to walk the list and see
	 * if a gateway prefix can match the phonumber prefix.
	 */

	// new iteration of the loop - protected by locks above
	srejectTime ++;

	// Select the route set in the order of longest match
	// search first
	prefix = strdupextra(phone, strlen("%NULL%")+1);

	for (i=strlen(phone); i >= 0; i--)
	{
		prefix[i] = '\0';

		if (i == 0)
		{
			strcpy(prefix, "%NULL%");
		}

		// Also do a phone based lookup, matching all routes
		cacheRouteEntry = CacheGet(cpdestCache, prefix);

		if (cacheRouteEntry == NULL)
		{
			continue;
		}

		startRouteEntry = cacheRouteEntry;

		do
		{
			if (logfn)
			{
				routeNode.crname = cacheRouteEntry->routeEntry.crname;
				routeNode.crflags = cacheRouteEntry->routeEntry.crflags;
				routeNode.cpname = NULL;
			}

			if ((cacheRouteEntry->routeEntry.destlen) &&
					(cacheRouteEntry->routeEntry.destlen != strlen(phone)))
			{
				/* Cannot consider this one as a match */
			 	NETDEBUG(MFIND, NETLOG_DEBUG4,
				   ("\tdestlen mismatch\n"));

				if (logfn)
				{
					routeNode.rejectReason = nextoneRouteLenMismatch;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}

				goto _continue_route_entry;
			}

			if ((cacheRouteEntry->routeEntry.crflags & CRF_CALLDEST) == 0)
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4,
					("Found call src route corresponding to %s\n",
					cacheRouteEntry->routeEntry.crname));

				// We don't want this message in R2, as it is not an error. 
				// This code should not be merged into future releases, 
				// where this is an error scenario (because of the separation routes)

				// if (logfn)
				// {
				// 	routeNode.rejectReason = nextoneRouteTypeMismatch;
				// 	routeNode.forwardReason = 0;
				// 	routeNode.branch = primary;
				// 	logfn(&routeNode);
				// }

				goto _continue_route_entry;
			}

			cacheCPBEntry = CacheGet(cpbcrCache, cacheRouteEntry->routeEntry.crname);

			if (cacheCPBEntry == NULL)
			{
				if (logfn)
				{
					routeNode.rejectReason = nextoneNoRouteBinding;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}

				goto _continue_route_entry;
			}

			startCPBEntry = cacheCPBEntry;

			do
			{
				routeNode.cpname = cacheCPBEntry->cpbEntry.cpname;

				if (cacheCPBEntry->cpbEntry.crflags & CRF_TIME)
				{
					if (CRApplyTime(&cacheCPBEntry->cpbEntry) < 0)
					{
						/* This route does not apply right now.
						 */
						NETDEBUG(MFIND, NETLOG_DEBUG4, 
							("route %s/%s not applicable now\n",
							cacheCPBEntry->cpbEntry.cpname,
							cacheCPBEntry->cpbEntry.crname));
	
						if (logfn)
						{
							routeNode.rejectReason = 
								nextoneTimeMismatch;
							routeNode.forwardReason = 0;
							routeNode.branch = primary;
							logfn(&routeNode);
						}

						goto _continue_cpb_entry;
					}
				}

				newset.routePriority = cacheCPBEntry->cpbEntry.priority;
				gwCacheEntry = CacheGet(gwcpCache, 
									cacheCPBEntry->cpbEntry.cpname);

				if (gwCacheEntry == NULL)
				{
					if (logfn)
					{
						routeNode.rejectReason = nextoneNoGateway;
						routeNode.forwardReason = 0;
						routeNode.branch = primary;
						logfn(&routeNode);
					}

					goto _continue_cpb_entry;
				}

				startGwEntry = gwCacheEntry;
				do
				{
					// Analyze this gw
					ngw ++;

					DEBUG(MFIND, NETLOG_DEBUG2,
						("GwLookup:: Examining gateway no %d\n", ngw));

					if (logfn)
					{
						InitPhonodeFromInfoEntry(&gwCacheEntry->data, 
							&routeNode.yphonode);
					}

					if (gwCacheEntry->data.rejectTime == srejectTime)
					{
						// This gateway has been rejected in this iteration of the
						// loop
						if (logfn)
						{
							routeNode.rejectReason = nextoneRejectRoute;
							routeNode.forwardReason = 0;
							routeNode.branch = primary;
							logfn(&routeNode);
						}
						goto _continue_gw_entry;
					}

					if (rhandle->findMSW &&
						!BIT_TEST(rhandle->phonodep->cap, CAP_MSW) &&
						!BIT_TEST(gwCacheEntry->data.cap, CAP_MSW))
					{
						callError = SCC_errorResourceUnavailable;

						DEBUG(MFIND, NETLOG_DEBUG2,
							("GwLookup:: Dest is not an MSW\n"));

						goto _continue_gw_entry;
					}

					if (GwCheckConfigParams(rhandle, gwCacheEntry, &newset, primary,
						&routeNode, logfn) < 0)
					{
						callError = SCC_errorNoPorts;

						DEBUG(MFIND, NETLOG_DEBUG2,
							("GwLookup:: resource unavailable\n"));

						if (logfn)
						{
							routeNode.rejectReason = nextoneNoPorts;
							routeNode.forwardReason = 0;
							routeNode.branch = primary;
							logfn(&routeNode);
						}

						time(&gwCacheEntry->data.attTime);
						goto _continue_gw_entry;
					}

					// Check REJECT routes
					if ((cacheCPBEntry->cpbEntry.crflags & CRF_REJECT) ||
						(cacheRouteEntry->routeEntry.crflags & CRF_REJECT))
					{
						// Add this gateway to the reject list and move on.
						gwCacheEntry->data.rejectTime = srejectTime;

						if (logfn)
						{
							routeNode.rejectReason = nextoneRejectRoute;
							routeNode.forwardReason = 0;
							routeNode.branch = primary;
							logfn(&routeNode);
						}

						goto _continue_gw_entry;
					}

					if (GwCheckHuntParams(rhandle, gwCacheEntry, &newset, primary,
						&routeNode, logfn) < 0)
					{
						callError = SCC_errorNoPorts;

						time(&gwCacheEntry->data.attTime);
						goto _continue_gw_entry;
					}

					// Check to see if the current gw is in the reject list
					if (GwCheckRejectList(gwCacheEntry, rhandle->destRejectList, 
							cacheRouteEntry->routeEntry.crname))
					{
						DEBUG(MFIND, NETLOG_DEBUG2,
							("GwLookup:: Gw in Reject List\n"));

						goto _continue_gw_entry;
					}

					if (BIT_TEST(gwCacheEntry->data.cap, CAP_MSW) &&
						BIT_TEST(rhandle->phonodep->cap, CAP_MSW))
					{
						DEBUG(MFIND, NETLOG_DEBUG2,
							("GwLookup:: Src and Dest are MSW\n"));

						goto _continue_gw_entry;
					}

					newset.cpVal = i;
					memcpy(&newset.route, 
						&cacheRouteEntry->routeEntry, sizeof(VpnRouteEntry));

					newset.cTime = gwCacheEntry->data.cTime;
					newset.gwPriority = gwCacheEntry->data.priority;
					newset.gwFlags = gwCacheEntry->data.stateFlags;
					if ((cpRoutingPolicy == CP_ROUTE_UTILZ) &&
						(IedgeXCalls(&gwCacheEntry->data) > 0))
					{
						newset.utilization = 
							((float)IedgeCalls(&gwCacheEntry->data))/
								IedgeXCalls(&gwCacheEntry->data);
					}
					else
					{
						newset.utilization = 0;
					}

					cmpResult = GwCompare(&newset, &oldset, &gwReason);
					if ((cmpResult > 0) || (!prefGw && (cmpResult == 0)))
					{
			 			/* new set is preferable */
			 			oldset = newset;
			 			prefGw = gwCacheEntry;

			 			NETDEBUG(MFIND, NETLOG_DEBUG2,
				   			("Selected new gateway - (cpval %d)\n", i));

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

			_continue_gw_entry:
					gwCacheEntry = gwCacheEntry->next;
				} while (gwCacheEntry != startGwEntry);

		_continue_cpb_entry:
				cacheCPBEntry = cacheCPBEntry->crnext;
			} while (cacheCPBEntry != startCPBEntry);

	_continue_route_entry:
			cacheRouteEntry = cacheRouteEntry->next;
		} while (cacheRouteEntry != startRouteEntry);

	}

	_found_gw:

		if(prefGw && reservePort)
		{
			if(ctInfoAllocCall(prefGw, 1, 1)!=0)
			{
				/* should never fail here */
				NETERROR(MFIND, 
					("%s ctInfoAllocCall failed\n", fn));

				callError = SCC_errorNoPorts;
				if (logfn)
				{
					routeNode.rejectReason = nextoneNoPorts;
					routeNode.forwardReason = 0;
					routeNode.branch = primary;
					logfn(&routeNode);
				}

				time(&prefGw->data.attTime);
				prefGw = NULL;
			}			
			else
			{
				// timestamp the gw
				prefGw->data.cTime = sglobalTime++;
			}
		}

		// Before we migrate to another route
		if (prefGw)
		{
			char tmpphone[PHONE_NUM_LEN] = { 0 };
			int newlen;

			info = prefGw;
	
			if (cacheInfo)
			{
				memcpy(cacheInfo, info, sizeof(CacheTableInfo));
			}
	
			// copy the route
			if (rhandle->crname)
			{
				strcpy(rhandle->crname, oldset.route.crname);
			}

			DEBUG(MFIND, NETLOG_DEBUG2, ("Gateway selected finally - \n"));
	
			pentry = &info->data;

			memcpy(phonode->regid,
				   pentry->regid, REG_ID_LEN);
			phonode->uport = pentry->uport;
			phonode->realmId = pentry->realmId;
	
			BIT_SET(phonode->sflags, ISSET_REGID);
			BIT_SET(phonode->sflags, ISSET_UPORT);
	
			if (BIT_TEST((pentry->sflags|pentry->dflags),
						 ISSET_IPADDRESS))
			{	
				 phonode->ipaddress.l =
					  pentry->ipaddress.l;
				 rhandle->rfcallsigport = pentry->callsigport;
				 BIT_SET(phonode->sflags, ISSET_IPADDRESS);
			}
	
			phonode->cap = pentry->cap;
			phonode->clientState = pentry->stateFlags;
	
			phoneChange = (pentry->ispcorgw != IEDGE_1000) && phoneChange;
	
			NETDEBUG(MFIND, NETLOG_DEBUG4,
				("%s phone change = %d\n", fn, phoneChange));
	
			/* If the gateway selected is a 1000, then we should not
			 * alter any numbers. Rel 1.2
			 */
			if (phoneChange)
			{
				// Create new phone in tmpphone first
				if (info->data.ogprefix[0])
				{
					strcpy(tmpphone, info->data.ogprefix);
				}

				if (oldset.cpVal > 0)
				{
					/* Take off the dest from the dest phone,
					 * and put the prefix instead.
					 */
		
					newlen = nx_strlcat(tmpphone, oldset.route.prefix, PHONE_NUM_LEN);
	
					if (tmpphone[newlen-1] == '$')
					{
						tmpphone[--newlen] = '\0';
					}
					else
					{
						nx_strlcat(tmpphone, phonode->phone+oldset.cpVal,
							PHONE_NUM_LEN);
					}

					strcpy(phonode->phone, tmpphone);
				}
				else
				{
					nx_strlcat(tmpphone, phonode->phone, PHONE_NUM_LEN);
					strcpy(phonode->phone, tmpphone);
				}
			}

			strcpy(cacheInfo->data.phone, phonode->phone);
	
			NETDEBUG(MFIND, NETLOG_DEBUG2, 
				("New Phone to be dialled on gateway is %s\n", phonode->phone));

			BIT_SET(cacheInfo->data.sflags, ISSET_PHONE);
			BIT_RESET(cacheInfo->data.sflags, ISSET_VPNPHONE);

			CacheReleaseLocks(vpnCache);
			CacheReleaseLocks(cpCache);
			CacheReleaseLocks(cpbCache);
			CacheReleaseLocks(regCache);

			if (prefix)
				free(prefix);

			return 1;
		}

_no_gw_found:
	CacheReleaseLocks(vpnCache);
	CacheReleaseLocks(cpCache);
	CacheReleaseLocks(cpbCache);
	CacheReleaseLocks(regCache);

	if (prefix)
		free(prefix);

	if (callError == SCC_errorNone)
	{
		// No Gateway was found
		callError = SCC_errorNoRoute;
	}

	return -callError;
}

/* Return < if CP policy rejects the call.
 * Also return -reason
 * 0 if CP Policy does not take any effect.
 * 1 if the CP Policy allows.
 */
int
ApplyCPPolicy(
	char *cpname,
	char *sphone,
	char *phone,
	char *newphone,
	unsigned long crflags,
	unsigned long type, /* PREFIX/DEST */
	int *matchVal,
	char *crname,
	RouteLogFn logfn
)
{
	char fn[] = "ApplyCPPolicy():";
	CacheCPBEntry *cacheCPBEntry = 0;
	CacheVpnRouteEntry *cacheRouteEntry = 0, *cacheRouteEntryMatch = 0;
	cp_match_set newset = { 0 }, oldset = { 0 };	/* default here is allow */
	int rc = 0, matchLen = 0, tmpMatchLen, srcMatchLen = 0, tmpSrcMatchLen, rcSrc;
	RouteNode	routeNode = { 0 };
	char *prefix = NULL, tmpphone[PHONE_NUM_LEN];
	int i, transitRoute = 0;
	CacheRouteEntry *startRouteEntry;
	CacheCPBEntry *startCPBEntry;
	char tags[TAGH_LEN] = { 0 };
	cache_t dniscache = NULL;
	route_match_set currset = { 0, 0, 0 }, compset = { 0, 0, 0 };

	// Wont apply reject routes at dest or src
	// If cpname is unknown, will only apply transit routes

	// reject routes applicable only on transit routes
	if ((crflags & CRF_REJECT) &&
		 !(crflags & CRF_TRANSIT))
	{
		goto _return;
	}

	if ((cpname[0] == '\0') &&
		!(crflags & CRF_TRANSIT))
	{
		goto _return;
	}

	if (crflags & CRF_TRANSIT)
	{
		dniscache = cptransitCache;
	}
	else if (crflags & CRF_CALLORIGIN)
	{
		dniscache = cporigDNISCache;
	}
	else
	{
		dniscache = cpdestCache;
	}

	if (logfn)
	{
		strcpy(routeNode.xphonode.phone, phone);
		BIT_SET(routeNode.xphonode.sflags, ISSET_PHONE);
	}

	strcpy(tmpphone, phone);

	prefix = strdupextra(phone, strlen("%NULL%")+1);

	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	for (i=strlen(phone); i >= 0; i--)
	{
		prefix[i] = '\0';

		if (i == 0)
		{
			strcpy(prefix, "%NULL%");
		}

		cacheRouteEntry = CacheGet(dniscache, prefix);

		if (cacheRouteEntry == NULL)
		{
			continue;
		}

		startRouteEntry = cacheRouteEntry;
		
		do
		{
			if ((cacheRouteEntry->routeEntry.destlen) &&
				(cacheRouteEntry->routeEntry.destlen != strlen(phone)))
			{
				/* Cannot consider this one as a match */
				NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tdestlen mismatch\n"));
				goto _continue_route_entry;
			}	

			if ((crflags & CRF_CALLORIGIN) !=
				(cacheRouteEntry->routeEntry.crflags & CRF_CALLORIGIN))
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tcall type mismatch\n"));
				goto _continue_route_entry;
			}

			if ((crflags & CRF_CALLDEST) !=
				(cacheRouteEntry->routeEntry.crflags & CRF_CALLDEST))
			{
				NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tcall type mismatch\n"));
				goto _continue_route_entry;
			}

			// Check if we are looking for REJECT routes at all
			if (!(crflags & CRF_REJECT) && 
				(cacheRouteEntry->routeEntry.crflags & CRF_REJECT))
			{
				// if we are not checking for reject routes, 
				// skip this route entry
				goto _continue_route_entry;
			}

			if (cpname[0] != '\0')
			{
				// Check the binding

				cacheCPBEntry = CacheGet(cpbcrCache, 
								cacheRouteEntry->routeEntry.crname);

				if (cacheCPBEntry == NULL)
				{
					goto _continue_route_entry;
				}

				startCPBEntry = cacheCPBEntry;

				do
				{
					if (strcmp(cacheCPBEntry->cpbEntry.cpname, cpname))
					{
						/* We can optimize the search in this condition,
						 * but later.
						 */
						goto _continue_cpb_entry;
					}

					if ((crflags & CRF_ROLLOVER) !=
						(cacheCPBEntry->cpbEntry.crflags & CRF_ROLLOVER))
					{
						goto _continue_cpb_entry;
					}

					if ((crflags & CRF_FORWARD) !=
						(cacheCPBEntry->cpbEntry.crflags & CRF_FORWARD))
					{
						goto _continue_cpb_entry;
					}

					if (cacheCPBEntry->cpbEntry.crflags & CRF_TIME)
					{
						if (CRApplyTime(&cacheCPBEntry->cpbEntry) < 0)
	
						{
							/* This route does not apply right now.
							 */
							goto _continue_route_entry;
						}
					}

					// binding applies
					// compare with existing route match
					if(currset.cacheRouteEntry)
					{
						compset.matchLen = i;
						compset.routePriority = cacheCPBEntry->cpbEntry.priority;
						compset.cacheRouteEntry = cacheRouteEntry;
						RouteCompareSet(&currset, &compset);
					}
					else
					{
						currset.matchLen = i;
						currset.routePriority = cacheCPBEntry->cpbEntry.priority;
						currset.cacheRouteEntry = cacheRouteEntry;
					}

				_continue_cpb_entry:
					cacheCPBEntry = cacheCPBEntry->crnext;
				} while (cacheCPBEntry != startCPBEntry);

				// no binding found, when one should have been found
				// skip to next route
				goto _continue_route_entry;
			}
			else
			{
				// we are considering a transit route
				if(currset.cacheRouteEntry)
				{
					compset.matchLen = i;
					// transit routes don't have a route priority 
					// since they are unbound
					compset.routePriority = 0;
					compset.cacheRouteEntry = cacheRouteEntry;
					RouteCompareSet(&currset, &compset);
				}
				else
				{
					currset.matchLen = i;
					currset.routePriority = 0;
					currset.cacheRouteEntry = cacheRouteEntry;
				}
			}

		_continue_route_entry:
			cacheRouteEntry = cacheRouteEntry->next;

		} while (cacheRouteEntry != startRouteEntry);
	}

	if(currset.cacheRouteEntry)
	{
		cacheRouteEntry = currset.cacheRouteEntry;


		if ((crflags & CRF_REJECT) && 
				(cacheRouteEntry->routeEntry.crflags & CRF_REJECT))
		{
			if (logfn)
			{
				routeNode.rejectReason = nextoneRejectRoute;
				routeNode.forwardReason = 0;
				logfn(&routeNode);
			}

			rc = -SCC_errorRejectRoute;

			CacheReleaseLocks(cpCache);
			CacheReleaseLocks(cpbCache);

			goto _return;
		}

		if (cacheRouteEntry->routeEntry.crflags & CRF_DYNAMICLRU)
		{
			// Re-adjust the routes to make sure we do LRU
			// when time comes to delete

			CacheDelete(lruRoutesCache, cacheRouteEntry);

			// timestamp the entry
			time(&cacheRouteEntry->routeEntry.rTime);

			CacheInsert(lruRoutesCache, cacheRouteEntry);
				
			BITA_SET(tags, TAG_CRMRU);
			GisPostCliCRAddCmd(cacheRouteEntry->routeEntry.crname, 
				tags);
		}

		strcpy(newphone, cacheRouteEntry->routeEntry.prefix);

		nx_strlcat(newphone, tmpphone + currset.matchLen, PHONE_NUM_LEN);

		if(crname)
		{
			nx_strlcpy(crname, cacheRouteEntry->routeEntry.crname, CALLPLAN_ATTR_LEN);
		}

		DEBUG(MFIND, NETLOG_DEBUG2, 
				("New Phone is %s\n", newphone));

		if (matchVal)
		{
			*matchVal = currset.matchLen;
		}

		if(prefix) free(prefix);

		CacheReleaseLocks(cpCache);
		CacheReleaseLocks(cpbCache);
	
		return 1;
	}
	else
	{
		CacheReleaseLocks(cpCache);
		CacheReleaseLocks(cpbCache);
	}
					
	// no match found
_return:
	strcpy(newphone, phone);

	if (prefix) free(prefix);

	if (matchVal)
	{
		*matchVal = 0;
	}

	return rc;
}

int
ApplyANIPlan(
	char *cpname,
	char *phone,
	char *newphone,
	unsigned long crflags,
	unsigned long type, /* PREFIX/DEST */
	int *matchVal,
	RouteLogFn logfn
)
{
	char fn[] = "ApplyANIPlan():";
	CacheCPBEntry *cacheCPBEntry = 0;
	CacheVpnRouteEntry *cacheRouteEntry = 0, *cacheRouteEntryMatch = 0;
	cp_match_set newset = { 0 }, oldset = { 0 };	/* default here is allow */
	int rc = 0, matchLen = 0, tmpMatchLen, srcMatchLen = 0, tmpSrcMatchLen, rcSrc;
	RouteNode	routeNode = { 0 };
	char *prefix = NULL, tmpphone[PHONE_NUM_LEN];
	int i, newlen;
	CacheRouteEntry *startRouteEntry;
	CacheCPBEntry *startCPBEntry;
	cache_t anicache;
	route_match_set currset = { 0, 0, 0 }, compset = { 0, 0, 0 };

	/* Look at the source and the number it is dialling first */	

	// Wont support negative routes yet
	if (!(crflags & CRF_POSITIVE))
	{
		goto _return;
	}

	if (logfn)
	{
		strcpy(routeNode.xphonode.phone, phone);
		BIT_SET(routeNode.xphonode.sflags, ISSET_PHONE);
	}

	strcpy(tmpphone, phone);

	if (crflags & CRF_CALLORIGIN)
	{
		anicache = cporigANICache;
	}
	else
	{
		anicache = cpsrcCache;
	}

	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	/* SRC */
	if (strlen(cpname))
	{
		prefix = strdupextra(phone, strlen("%NULL%")+1);

		for (i=strlen(phone); i >= 0; i--)
		{
			prefix[i] = '\0';

			if (i == 0)
			{
				strcpy(prefix, "%NULL%");
			}

			cacheRouteEntry = CacheGet(anicache, prefix);

			if (cacheRouteEntry == NULL)
			{
				continue;
			}

			startRouteEntry = cacheRouteEntry;
		
			do
			{
				if ((cacheRouteEntry->routeEntry.srclen) &&
					(cacheRouteEntry->routeEntry.srclen != strlen(phone)))
				{
					/* Cannot consider this one as a match */
					NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tsrclen mismatch\n"));
					goto _continue_route_entry;
				}	

				if ((crflags & CRF_CALLORIGIN) !=
					(cacheRouteEntry->routeEntry.crflags & CRF_CALLORIGIN))
				{
					NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tcall type mismatch\n"));
					goto _continue_route_entry;
				}

				if ((crflags & CRF_CALLDEST) !=
					(cacheRouteEntry->routeEntry.crflags & CRF_CALLDEST))
				{
					NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tcall type mismatch\n"));
					goto _continue_route_entry;
				}

				// Check if we are looking for REJECT routes at all
				if (!(crflags & CRF_REJECT) && 
					(cacheRouteEntry->routeEntry.crflags & CRF_REJECT))
				{
					goto _continue_route_entry;
				}

				cacheCPBEntry = CacheGet(cpbcrCache, cacheRouteEntry->routeEntry.crname);

				if (cacheCPBEntry == NULL)
				{
					goto _continue_route_entry;
				}

				startCPBEntry = cacheCPBEntry;

				do
				{

					if (strcmp(cacheCPBEntry->cpbEntry.cpname, cpname))
					{
						/* We can optimize the search in this condition,
						 * but later.
						 */
						goto _continue_cpb_entry;
					}

					if ((crflags & CRF_ROLLOVER) !=
						(cacheCPBEntry->cpbEntry.crflags & CRF_ROLLOVER))
					{
						goto _continue_cpb_entry;
					}

					if ((crflags & CRF_FORWARD) !=
						(cacheCPBEntry->cpbEntry.crflags & CRF_FORWARD))
					{
						goto _continue_cpb_entry;
					}
			
					if (cacheCPBEntry->cpbEntry.crflags & CRF_TIME)
					{
						rc = CRApplyTime(&cacheCPBEntry->cpbEntry);
	
						if (rc < 0)
						{
							/* This route for rollover/forward
							 * does not apply right now.
							 */
							goto _continue_cpb_entry;
						}
					}

					if(currset.cacheRouteEntry)
					{
						compset.matchLen = i;
						compset.routePriority = cacheCPBEntry->cpbEntry.priority;
						compset.cacheRouteEntry = cacheRouteEntry;
						RouteCompareSet(&currset, &compset);
					}
					else
					{
						currset.matchLen = i;
						currset.routePriority = cacheCPBEntry->cpbEntry.priority;
						currset.cacheRouteEntry = cacheRouteEntry;
					}

			_continue_cpb_entry:
					cacheCPBEntry = cacheCPBEntry->crnext;
				} while (cacheCPBEntry != startCPBEntry);

		_continue_route_entry:
				cacheRouteEntry = cacheRouteEntry->snext;
			} while (cacheRouteEntry != startRouteEntry);

		}
	}
					
	if(currset.cacheRouteEntry)
	{
		cacheRouteEntry = currset.cacheRouteEntry;

		if ((crflags & CRF_REJECT) && 
				(cacheRouteEntry->routeEntry.crflags & CRF_REJECT))
		{
			if (logfn)
			{
				routeNode.rejectReason = nextoneRejectRoute;
				routeNode.forwardReason = 0;
				logfn(&routeNode);
			}

			rc = -SCC_errorRejectRoute;

			CacheReleaseLocks(cpCache);
			CacheReleaseLocks(cpbCache);

			goto _return;
		}

		DEBUG(MFIND, NETLOG_DEBUG4, ("\tmatched calling plan\n"));

		if (cacheRouteEntry->routeEntry.srcprefix[0] == '@')
		{
			ReplaceANI(newphone, cacheRouteEntry->routeEntry.srcprefix + 1);
		}
		else
		{
			newlen = GeneratePatternInstance(cacheRouteEntry->routeEntry.srcprefix, 
				cacheRouteEntry->routeEntry.tmpsrcprefix, newphone, 
				strlen(cacheRouteEntry->routeEntry.srcprefix));

			if (!newlen || (newphone[newlen-1] != '$'))
			{
				nx_strlcat(newphone, tmpphone+currset.matchLen, PHONE_NUM_LEN);
			}
			else
			{
				newphone[--newlen] = '\0';
			}
		}

		DEBUG(MFIND, NETLOG_DEBUG2, 
			("New ANI is %s\n", newphone));

		if (matchVal)
		{
			*matchVal = currset.matchLen;
		}

		CacheReleaseLocks(cpCache);
		CacheReleaseLocks(cpbCache);

		if(prefix) free(prefix);

		return 1;
	}

	CacheReleaseLocks(cpCache);
	CacheReleaseLocks(cpbCache);

	// no match found
_return:
	strcpy(newphone, phone);

	if (prefix) free(prefix);

	if (matchVal)
	{
		*matchVal = 0;
	}

	return rc;
}

// Based on a given pattern and the cache to look
// up the pattern in, find out the gateway which best matches
// If a regid is input, the gateway must have the same regid
// as specified (in this case match the uport on the regid).
// Also return the matching criteria used for the match,
// (route priority, pattern match length)
// Note that we are not trying to route a call in this API,
// but trying to identify which route was used. No reason
// code needs to be returned in this case, and checks for
// resource unavailable, maxcalls do not have to be made.
// return -1 if no match was found
CacheTableInfo *
GwRouteLookup(
	cache_t cache, 
	int offset,
	char *pattern, 
	char *regid, 
	struct gw_match_set *gwset, 
	struct gw_match_set *gwsetin, 
	RouteLogFn logfn
)
{
	char fn[] = "GwRouteLookup():";
	int defaultMatchDone = 0, routeLen, gwReason;
	struct gw_match_set newset = { 0 }, oldset = { 0 };
	CacheTableInfo *gwCacheEntry, *prefGw = 0, *startGwEntry = 0;
	CacheRouteEntry *cacheRouteEntry = 0, *startRouteEntry;
	CacheCPBEntry *startCPBEntry, *cacheCPBEntry = 0;
	RouteNode	routeNode = { 0 };

	if (!gwsetin)
	{
		// Initialize the oldset
		oldset.gwPriority = -100;
		oldset.routePriority = -1;
		*gwset = oldset;
	}
	else
	{
		oldset = *gwsetin;
	}

	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);

	for (cacheRouteEntry = CacheMatchFirstPrefix(cache, pattern);
		!defaultMatchDone;
		cacheRouteEntry = CacheMatchNextPrefix(cache, pattern))
	{
		if ((cacheRouteEntry == NULL) && !defaultMatchDone)
		{
			defaultMatchDone = 1;
			cacheRouteEntry = CacheGet(cache, "%NULL%");
			if (cacheRouteEntry == NULL)
			{
				break;
			}
		}
		
		routeLen = cache->iterator.tst.key_index;

		startRouteEntry = cacheRouteEntry;
		
		do
		{
			if ((cacheRouteEntry->routeEntry.destlen) &&
					(cacheRouteEntry->routeEntry.destlen != strlen(pattern)))
			{
				/* Cannot consider this one as a match */
			 	NETDEBUG(MFIND, NETLOG_DEBUG4, ("\tdestlen mismatch\n"));

				goto _continue_route_entry;
			}

			// Get to the bindings corresponding to the route
			cacheCPBEntry = CacheGet(cpbcrCache, 
								cacheRouteEntry->routeEntry.crname);

			if (cacheCPBEntry == NULL)
			{
				goto _continue_route_entry;
			}

			startCPBEntry = cacheCPBEntry;

			do
			{
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
						logfn(&routeNode);
					}

					goto _continue_cpb_entry;
				}

				newset.routePriority = cacheCPBEntry->cpbEntry.priority;

				gwCacheEntry = CacheGet(gwcpCache, 
									cacheCPBEntry->cpbEntry.cpname);

				if (gwCacheEntry == NULL)
				{
					goto _continue_cpb_entry;
				}

				startGwEntry = gwCacheEntry;
				do
				{
					if (regid && memcmp(gwCacheEntry->data.regid, regid, REG_ID_LEN))
					{
						// We have to ignore this gateway
						goto _continue_gw_entry;
					}
						
					if (logfn)
					{
						InitPhonodeFromInfoEntry(&gwCacheEntry->data, 
							&routeNode.yphonode);
					}

					newset.cpVal = routeLen;

					memcpy(&newset.route, 
						&cacheRouteEntry->routeEntry, sizeof(VpnRouteEntry));

					newset.cTime = gwCacheEntry->data.cTime;
					newset.gwPriority = gwCacheEntry->data.priority;
					newset.gwFlags = gwCacheEntry->data.stateFlags;

					newset.utilization = 0;

					if (GwRouteCompare(&newset, &oldset, &gwReason) > 0)
					{
			 			/* new set is preferable */
			 			oldset = newset;
			 			prefGw = gwCacheEntry;

						if (logfn)
						{
							routeNode.rejectReason = 0;
							routeNode.forwardReason = gwReason;
							logfn(&routeNode);
						}
					}
					else
					{
						if (logfn)
						{
							routeNode.rejectReason = gwReason;
							routeNode.forwardReason = 0;
							logfn(&routeNode);
						}
					}

			_continue_gw_entry:
					gwCacheEntry = gwCacheEntry->next;
				} while (gwCacheEntry != startGwEntry);

		_continue_cpb_entry:
				cacheCPBEntry = cacheCPBEntry->crnext;
			} while (cacheCPBEntry != startCPBEntry);

	_continue_route_entry:
			cacheRouteEntry = (CacheRouteEntry *)Listg(cacheRouteEntry, offset)->next;
		} while (cacheRouteEntry != startRouteEntry);
	}

	if (prefGw)
	{
		*gwset = oldset;
	}

	CacheReleaseLocks(cpCache);
	CacheReleaseLocks(cpbCache);
	CacheReleaseLocks(regCache);

	return prefGw;
}
