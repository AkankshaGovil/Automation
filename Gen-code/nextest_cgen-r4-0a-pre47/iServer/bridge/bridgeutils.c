#include "bridge.h"
#include "nxosd.h"

#include "ls.h"
#include "licenseIf.h"
#include "gk.h"
#include "scm_call.h"
#include "sipcallactions.h"
#include "log.h"
#include "iwfsmproto.h"

int *bridgeinPool;
int *bridgeinClass;
int *bridgeinhpClass;
extern int nBridgeThreads;

CallRealmInfo *
getRealmInfo (int realmId, int mallocfn)
{
	CacheRealmEntry *realmEntry;
	char fn[] = "getRealmInfo()";
	CallRealmInfo realmInfo = {0};
        char* ptr1;
        char* ptr2;

	CacheGetLocks (realmCache, LOCK_READ, LOCK_BLOCK);
	realmEntry = CacheGet (realmCache, &realmId);
	if (realmEntry == NULL) 
	{
		NETERROR  (MBRIDGE, ("%s: Failed to get realm entry for id %d", fn, realmId));
		CacheReleaseLocks (realmCache);
		return NULL;
	}
	realmInfo.realmId = realmId;
	realmInfo.rsa = realmEntry->realm.rsa;
	realmInfo.sPoolId = realmEntry->realm.sigPoolId;
	realmInfo.mPoolId = realmEntry->realm.medPoolId;
	realmInfo.addrType = realmEntry->realm.addrType;
	realmInfo.interRealm_mr = realmEntry->realm.interRealm_mr;
	realmInfo.intraRealm_mr = realmEntry->realm.intraRealm_mr;
        
	CacheReleaseLocks (realmCache);
	return ((CallRealmInfo *)RealmInfoDup (&realmInfo, mallocfn));
}

int bridgeSipEventProcessor(SCC_EventBlock *evtPtr)
{
	static char fn[] = "bridgeSipEventProcessor";

	evtPtr->evtProcessor = bridgeSipEventProcessorWorker;

	return bridgeQueueEvent(evtPtr);
}

ConfType
DetermineConfType(SCC_CallHandleType call1, SCC_CallHandleType call2)
{
	static char fn[] = "DetermineConfType";
	static int callMatrix[SCC_eMaxCallHandleType][SCC_eMaxCallHandleType] = 
		{ 
			{Conf_eH323P2P,Conf_eH3232SipP2P}, 
			{Conf_eH3232SipP2P,Conf_eSipP2P}
		};
	static char cfName[4][20] = { "Sip","H323","Sip2H323","H3232Sip"};
	ConfType cfType;

	cfType = (callMatrix[call1][call2]);
	NETDEBUG(MBRIDGE,NETLOG_DEBUG4,
		("%s: Conf type is %s call1 = %d call2 = %d\n",
		fn,cfName[cfType],call1,call2));

	return cfType;
}


int getConfAndPeerCallID(char * confID, char *callID,
							ConfHandle *conf, char *peerCallID)
{
	static char	fn[] = "getConfAndPeerCallID";
	int			i;

	if (CacheFind(confCache,confID, conf, sizeof(ConfHandle)) < 0)
	{
		NETERROR(MSCC,("%s Unable to locate Conf handle for call 0\n",fn));
		return -1;
	}

	for(i=0; i < conf->ncalls;i++)
	{
		// Not needed, as we wont have any zero callid's 
		// if (memcmp(conf->callID[i], zeroCallID, CALL_ID_LEN) == 0)
		// {
		// 	continue;
		// }

		if (memcmp(conf->callID[i],callID,CALL_ID_LEN))
		{
			memcpy(peerCallID,conf->callID[i],CALL_ID_LEN);
			return 0;
		}
	}

	return -1;
}

int getPeerCallIDFromConf(char *callID, ConfHandle *conf, char *peerCallID)
{
	static char	fn[] = "getConfAndPeerCallID";
	int			i;

	memset(peerCallID, 0, CALL_ID_LEN);

	for(i=0; i < conf->ncalls;i++)
	{
		if (memcmp(conf->callID[i],callID,CALL_ID_LEN))
		{
			memcpy(peerCallID,conf->callID[i],CALL_ID_LEN);
			return 0;
		}
	}

	{
	char	callIDStr1[CALL_ID_STRLEN] = {0},
		callIDStr2[CALL_ID_STRLEN] = {0},
		confIDStr[CONF_ID_STRLEN] = {0};

	NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
		("%s failed. ncalls = %d ConfHandl=%s.\n",
		fn,conf->ncalls,
		(char*) ConfID2String(conf->callID[1],confIDStr)));

	NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
			("%s CallId 1 = %s.CallID2 = %s.\n",
			fn, (char*) CallID2String(conf->callID[0],callIDStr1),
			(char*) CallID2String(conf->callID[1],callIDStr2)));
	}
	return -1;
}

int getPeerCallID(char * confID, char *callID,char *peerCallID)
{
	static char	fn[] = "getPeerCallID";
	ConfHandle	*confHandle = NULL;
	int			i;

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	memset(peerCallID, 0, CALL_ID_LEN);

	if ((confHandle = CacheGet(confCache,confID)) == NULL)
	{
		NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
			("%s Unable to locate Conf handle for call 0\n",fn));
		CacheReleaseLocks(confCache);
		return -1;
	}

	for(i=0; i < confHandle->ncalls;i++)
	{
		if (memcmp(confHandle->callID[i],callID,CALL_ID_LEN))
		{
			memcpy(peerCallID,confHandle->callID[i],CALL_ID_LEN);
			CacheReleaseLocks(confCache);
			return 0;
		}
	}

	CacheReleaseLocks(confCache);

	return -1;
}

int bridgeSipResolveCache( 
		CallHandle *callHandle, 
		SipAppCallHandle *pSipData,
		int *callError,
		InfoEntry *sinfo,
		InfoEntry *dinfo
	)
{
	char fn[] = "bridgeSipResolveCache():";
	PhoNode *phonodep, *rphonodep, *rfphonodep;
	CacheTableInfo srcCacheInfoEntry = { 0 }, *srcCacheInfo, tempCacheInfo,
					*cacheInfo = NULL;
	ResolveHandle *rhandle = NULL;
	header_url *from_uri = NULL;
	char *tmpstr, *requri = NULL;
	unsigned short port;
	SipError err;
	char s1[25];
	int	requrilen;
	int rc = -1, rcSrc = -1;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Starting Resolution in cache\n", fn));

	/* First step is to identify the source. This can be done
	 * on the basis of (1) From address:name,
	 * (2) From address:address,
	 * (3) Via address (implemented later... )
	 */
	from_uri = pSipData->callingpn;

	phonodep = &callHandle->phonode;
	rfphonodep = &callHandle->rfphonode;

	sipCallHandle = SipCallHandle(callHandle);

	// Do a Request URI lookup first
	
	if(pSipData->calledpn == NULL)
	{
		NETERROR(MBRIDGE,
			("%s callingpn NULL\n", fn));
		*callError = SCC_errorSipInternalError;
		rc = -1;
		goto _error;
	}

	requrilen = strlen(SVal(pSipData->calledpn->name)) + 
					strlen(pSipData->calledpn->host)+2;

	if(!(requri = (char *)calloc(requrilen,sizeof(char))))
	{
		NETERROR(MBRIDGE,
			("%s failed to allocate %d bytes for requri\n", 
			fn, requrilen));
		*callError = SCC_errorSipInternalError;
		rc = -1;
		goto _error;
	}
	
	sprintf(requri, "%s@%s",
			SVal(pSipData->calledpn->name),SVal(pSipData->calledpn->host));

	cacheInfo = &tempCacheInfo;
	if (CacheFind(uriCache, requri, cacheInfo, sizeof(CacheTableInfo)) >= 0)
	{
		NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
			("%s found requri %s in uri cache\n",
			fn, requri));

		if(strlen(cacheInfo->data.phone) > 0)
		{
			strcpy(rfphonodep->phone, cacheInfo->data.phone);
		}
		else
		{
			nx_strlcpy(rfphonodep->phone, requri, PHONE_NUM_LEN);
		}
		BIT_SET(rfphonodep->sflags, ISSET_PHONE);
	}

	// ANI has been stored in ua when the call handle was inited

	if(!callHandle->srccrname)
	{
		callHandle->srccrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
		if(callHandle->srccrname)
		{
			callHandle->srccrname[0] = '\0';
		}
	}

	if(!callHandle->destcrname)
	{
		callHandle->destcrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
		if(callHandle->destcrname)
		{
			callHandle->destcrname[0] = '\0';
		}
	}

	if(!callHandle->transitcrname)
	{
		callHandle->transitcrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
		if(callHandle->transitcrname)
		{
			callHandle->transitcrname[0] = '\0';
		}
	}

	if(!callHandle->transRouteNumber)
	{
		callHandle->transRouteNumber = (char *)CMalloc(callCache)(PHONE_NUM_LEN);
		if(callHandle->transRouteNumber)
		{
			callHandle->transRouteNumber[0] = '\0';
		}
	}


	/* Proceed still to the next phase */
	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= phonodep;
	rhandle->rfphonodep = rfphonodep;
	rhandle->checkVpnGroup = 1;
	rhandle->checkZone = 1;
	rhandle->reservePort = 1;
	rhandle->destRejectList = callHandle->destRejectList;
	rhandle->scpname = callHandle->cpname;
	rhandle->dtg = callHandle->destTg;
	rhandle->srccrname = callHandle->srccrname;
	rhandle->transitcrname = callHandle->transitcrname;
	rhandle->destcrname = callHandle->destcrname;
	rhandle->transRouteNumber = callHandle->transRouteNumber;
	rhandle->routeflag = callHandle->routeflag;

	/* Determine the vpn group of this entry */
	FindIedgeVpn(callHandle->vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	nx_strlcpy(rhandle->sZone, callHandle->zone, ZONE_LEN);

	ResolvePhone(rhandle);
	FindRemote(rhandle);

	callHandle->routeflag = rhandle->routeflag;

	if (BIT_TEST(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE))
	{
		strcpy(H323dialledNumber(callHandle),
			rhandle->fCacheInfoEntry.data.phone);
	}
	else
	{
		NETERROR(MBRIDGE, 
			("%s Phone Number not present in fCacheInfoEntry\n", fn));
	}

	if (((from_uri->name) && strcmp(from_uri->name, phonodep->phone)) ||
		(!from_uri->name && (phonodep->phone[0] != '\0')))
	{
		// ani has changed
		if (from_uri->name) 
		{
			free(from_uri->name);
			from_uri->name = NULL;
		}

		if (phonodep->phone[0] != '\0')
		{
			from_uri->name = strdup(phonodep->phone);
		}

		// change the display name IF the ani changed
		if (from_uri->display_name)
		{
			free(from_uri->display_name);
			from_uri->display_name = NULL;
		}
	}

	switch (rhandle->result)
	{
	case CACHE_FOUND:
		callHandle->rfcallsigport = rhandle->rfcallsigport;	

		callHandle->nhunts ++;

		if(strlen(rhandle->transRouteNumber) > 0)
		{
			strcpy(callHandle->transRouteNumber, rhandle->transRouteNumber);
		}

		// No need for this anymore, can free it
		BillCallPrevCdr(callHandle);

		SipReqUriFromPhonode(&callHandle->rfphonode,
			RH_FOUND_CACHE(rhandle, 0)->data.contact, &pSipData->requri);

		if(BIT_TEST(callHandle->rfphonode.cap, CAP_H323) && !h323Enabled())
		{
			NETERROR(MBRIDGE,("%s h323 Not Enabled\n",fn));
			*callError = SCC_errorInvalidPhone;
			rc = -1;
			goto _error;
		}
		rc = 1;
		break;
	case CACHE_NOTFOUND:
		NETDEBUG(MBRIDGE, NETLOG_DEBUG1, ("%s CACHE_NOTFOUND\n", fn));
		*callError = rhandle->callError;
		callHandle->nhunts += SYSTEM_MAX_HUNTS+1;
		rc = -1;
		break;
	case CACHE_INPROG:
		goto _error;
	default:
		break;
	}

	*dinfo = RH_FOUND_CACHE(rhandle, 0)->data;

	/* Free the remote handle here */
	GisFreeRHandle(rhandle);
	SipCheckFree(requri);
	return rc;
	
_error:
	/* Free the remote handle here */
	GisFreeRHandle(rhandle);
	SipCheckFree(requri);
	return rc;
}

/* reserves the allocated port if reservePort is nonZero 
*  resolve locally only if locally is nonzero 
*  If the phone is DND then resolve aphonode and overwrite callHandle->rfphonode
*  with alternate phone number
*/
int
bridgeResolveH323Cache(CallHandle *callHandle, H323EventData *pH323Data,int reservePort,int resolveRemote, int *callError,InfoEntry *sinfo,InfoEntry *dinfo)
{
	char 			fn[] = "bridgeResolveH323Cache():";
	int				rc = -1;
	ResolveHandle 	*rhandle = NULL;
	PhoNode			rfphonode = {0};
	header_url		*requri;
	char			ipstr[20] = {0};

	NETDEBUG(MFIND, NETLOG_DEBUG1,("%s Entering\n",fn));

	if(!callHandle->srccrname)
	{
		callHandle->srccrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
	}

	if(!callHandle->destcrname)
	{
		callHandle->destcrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
	}

	if(!callHandle->transitcrname)
	{
		callHandle->transitcrname = (char *)CMalloc(callCache)(CALLPLAN_ATTR_LEN);
	}

	if(!callHandle->transRouteNumber)
	{
		callHandle->transRouteNumber = (char *)CMalloc(callCache)(PHONE_NUM_LEN);
		callHandle->transRouteNumber[0] = '\0';
	}

	/* Initialize the rhandle */
	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= &callHandle->phonode;
	rhandle->rfphonodep = &callHandle->rfphonode;

	rhandle->reservePort = reservePort;
	rhandle->resolveRemote = resolveRemote;
	rhandle->destRejectList = callHandle->destRejectList;
	rhandle->scpname = callHandle->cpname;
	rhandle->srccrname = callHandle->srccrname;
	rhandle->transitcrname = callHandle->transitcrname;
	rhandle->destcrname = callHandle->destcrname;
	rhandle->transRouteNumber = callHandle->transRouteNumber;
	rhandle->routeflag = callHandle->routeflag;
	rhandle->dtg = callHandle->destTg;

	/* Determine the vpn group of this entry */
	FindIedgeVpn(callHandle->vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	nx_strlcpy(rhandle->sZone, callHandle->zone, ZONE_LEN);

	rhandle->checkZone = 1;
	rhandle->checkVpnGroup = 1;

	// Save a copy of rfphonode 
	rfphonode = callHandle->rfphonode;

	/* Call the main function */
	ResolvePhone(rhandle);
	FindRemote(rhandle);

	callHandle->routeflag = rhandle->routeflag;

	if (BIT_TEST(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE))
	{
		strcpy(H323dialledNumber(callHandle),
			rhandle->fCacheInfoEntry.data.phone);
	}
	else
	{
		NETERROR(MBRIDGE, 
			("%s Phone Number not present in fCacheInfoEntry\n", fn));
	}

	switch (rhandle->result)
	{
	case CACHE_FOUND:
		/* Fill up the response */
	   	NETDEBUG(MFIND, NETLOG_DEBUG4, ("%s CACHE_FOUND\n", fn));

		callHandle->nhunts ++;

		if(strlen(rhandle->transRouteNumber) > 0)
		{
			strcpy(callHandle->transRouteNumber, rhandle->transRouteNumber);
		}

		// No need for this anymore, can free it
		BillCallPrevCdr(callHandle);

		if(BIT_TEST(callHandle->rfphonode.cap, CAP_H323))
        	{
			H323rfcallsigport(callHandle) = rhandle->rfcallsigport;
			H323rfrasport(callHandle) = rhandle->rfrasport;
            		pH323Data->destip = callHandle->rfphonode.ipaddress.l;

					if (H323rfcallsigport(callHandle) == 0)
					{
						NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
							("%s Call Signalling port was 0, setting it to 1720\n",
							fn));
						H323rfcallsigport(callHandle) = 1720;
					}
            		pH323Data->destport= callHandle->rfcallsigport;

					if(!allowHairPin && 
						(pH323Data->destip == callHandle->phonode.ipaddress.l))
					{	
						NETERROR(MBRIDGE,
							("%s Hairpin disabled. Call desination is same as source"
							"- dropping Call. ip = %s\n",fn,
							FormatIpAddress(pH323Data->destip, ipstr)));
						if(reservePort)
						{
							GwPortFreeCall(&callHandle->rfphonode, 1, 1);
							if(callHandle->flags & FL_FREE_DESTPORT)
							{
								callHandle->flags &= ~FL_FREE_DESTPORT;
							}
						}
						rc = -1;
						*callError = SCC_errorHairPin;
						goto _return;
					}
        	}
        	else if(BIT_TEST(callHandle->rfphonode.cap, CAP_SIP) && sipEnabled() )
        	{
			/* Set the realmid of callHandle. This is set in bridgeh323 code later */
			callHandle->rfphonode.realmId = rhandle->rfCacheInfoEntry.data.realmId;
			SipReqUriFromPhonode(&callHandle->rfphonode,
				RH_FOUND_CACHE(rhandle, 0)->data.contact, &pH323Data->requri);
        	}
        	else 
		{
                	NETERROR(MBRIDGE,("%s UnSupported Remote Endpoint\n",fn));
			rc = -1;
			*callError = SCC_errorInvalidPhone;
			goto _return;
        	}

	   	rc = 1;
	   	break;

	 case CACHE_NOTFOUND:
	   NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s CACHE_NOTFOUND\n", fn));
	   rc = -1;
		*callError = rhandle->callError;

		callHandle->nhunts += SYSTEM_MAX_HUNTS+1;

	   goto _return;
	   break;
	 case CACHE_INPROG:
	   NETDEBUG(MBRIDGE, NETLOG_DEBUG1,
				("%s CACHE_INPROG\n", fn));
	   /* Dont do anything here, The rhandle, and the client
		* handle should be preserved, until it times out
		*/
	   rc = 0;
	   return rc;
	   break;
	 default:
	   break;
	 }

	*dinfo = rhandle->rfCacheInfoEntry.data;
 _reply:
 _return:
	GisFreeRHandle(rhandle);
	return rc;
}

int 
bridgeH323EventProcessor(SCC_EventBlock *evtPtr)
{
	static char fn[] = "bridgeH323EventProcessor";

	evtPtr->evtProcessor = bridgeH323EventProcessorWorker;

	return bridgeQueueEvent(evtPtr);
}

int
BridgeEnd(void)
{
	int i;

	for (i = 0; i<nBridgeThreads; i++)
	{
		ThreadPoolEnd(bridgeinPool[i]);
	}

	return(0);
}

int
BridgeInit(void)
{
	char fn[] = "BridgeInit():";
	int i;

	bridgeinPool = (int *)malloc(nBridgeThreads*sizeof(int));
	bridgeinClass = (int *)malloc(nBridgeThreads*sizeof(int));
	bridgeinhpClass = (int *)malloc(nBridgeThreads*sizeof(int));

	for (i = 0; i<nBridgeThreads; i++)
	{
		bridgeinPool[i] = ThreadPoolInit("bridge", 1, PTHREAD_SCOPE_PROCESS,
								0, 1);
		bridgeinClass[i] = ThreadAddPoolClass("bridge-class", bridgeinPool[i], 0, 100000000);

//		bridgeinhpClass[i] = ThreadAddPoolClass("bridge-hp", bridgeinPool[i], 0, 10000000);

		ThreadPoolStart(bridgeinPool[i]);
	}

	return 0;
}

int
bridgeQueueEvent(SCC_EventBlock *evtPtr)
{
	char fn[] = "bridgeQueueEvent():";
	unsigned short index = 0;

	memcpy(&index, evtPtr->confID+14, 2);
	index %= nBridgeThreads;

	if (ThreadDispatch(bridgeinPool[index],
//			(evtPtr->event==SCC_eBridgeReleaseComp||evtPtr->event==SCC_eBridgeProceeding||evtPtr->event==SCC_eBridgeSetup||evtPtr->event==SCC_eBridgeConnect||evtPtr->event==SCC_eBridgeAlerting||evtPtr->evern==SCC_eBridgeProgress)?bridgeinhpClass[index]:bridgeinClass[index],
			bridgeinClass[index],
			(void *(*)(void*))evtPtr->evtProcessor, evtPtr,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MBRIDGE, ("%s Error in dispatching\n", fn));
	}

	return 0;
}

int
FindH323CpnPOTS(SCC_EventBlock *evtPtr)
{
	char fn[] = "FindH323CpnPOTS():";
	CallHandle 	*callHandle,callHandleBlk1 = {0};
	H323EventData   	*pH323Data;
	char cpn[256] = { 0 }, cmd[256], ipstr[256];
	char callIDStr[CALL_ID_LEN];

	pH323Data = (H323EventData *) evtPtr->data;

	if (pH323Data == NULL)
	{
		// drop this
		NETERROR(MBRIDGE, ("%s Null Data passed\n", fn));
		return -1;
	}

	callHandle = &callHandleBlk1;

	// Find the SRC IP address	
	// We will find it in the call handle of src
	if(CacheFind(callCache,evtPtr->callID,&callHandleBlk1,sizeof(CallHandle)) <0)
	{
		NETERROR(MBRIDGE,("%s Failed to find src CallIhandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		return -1;
	}

	// Execute the script
	sprintf(cmd, "%s %s", e911loc, 
		FormatIpAddress(callHandle->phonode.ipaddress.l, ipstr));

	ExecuteScript(cmd, cpn, 256);

	// change the CPN
	if (strlen(cpn))
	{
		strncpy(pH323Data->callingpn, cpn, PHONE_NUM_LEN);
		pH323Data->callingpn[PHONE_NUM_LEN-1] = '\0';
	}

	// set the flag in event
	pH323Data->flags |= FL_CPN_POTS;

	// re-queue the event
	bridgeH323EventProcessor(evtPtr);

	return 0;
}

int
FindSipCpnPOTS(SCC_EventBlock *evtPtr)
{
	char fn[] = "FindSipCpnPOTS():";
	CallHandle 	*callHandle,callHandleBlk1 = {0};
	SipAppCallHandle	*pSipData;
	char cpn[256] = { 0 }, cmd[256], ipstr[256];
	char callIDStr[CALL_ID_LEN];
	header_url *from_uri = NULL;
	ulong ipaddr;
	int herror =0;
	SipMsgHandle *msgHandle;

	pSipData = (SipAppCallHandle *) evtPtr->data;

	if (pSipData == NULL)
	{
		// drop this
		NETERROR(MBRIDGE, ("%s Null Data passed\n", fn));
		return -1;
	}

	msgHandle = pSipData->msgHandle;
	if ((msgHandle == NULL) ||
			(msgHandle->srchost == NULL))
	{
		NETERROR(MBRIDGE, ("%s Null msg handle\n", fn));
		return -1;
	}

	from_uri = pSipData->callingpn;

	if ((from_uri == NULL) ||
		(from_uri->host == NULL))
	{
		NETERROR(MBRIDGE, ("%s Null Data passed\n", fn));
		return -1;
	}

	if((ipaddr = ResolveDNS(msgHandle->srchost, &herror)) == -1)
	{
		NETERROR(MBRIDGE, ("%s Cannot locate src %s\n", fn, msgHandle->srchost));
		return -1;
	}

	// Execute the script
	sprintf(cmd, "%s %s", e911loc, FormatIpAddress(ipaddr, ipstr));

	ExecuteScript(cmd, cpn, 256);

	// change the CPN
	if (strlen(cpn))
	{
		if (from_uri->name)
		{
			free(from_uri->name);
		}
		from_uri->name = strdup(cpn);
	}

	// set the flag in event
	pSipData->flags |= FL_CPN_POTS;

	return 0;
}

int
CallTap(CallHandle 	*callHandle, int status)
{
	char fn[] = "CallTap():";
	char cmd[256] = { 0 };
	unsigned int ip1 = 0, ip2 = 0;
	char str1[25], str2[25];

	if (((status == 1) && !strlen(tapcall)) ||
		((status == 0) && !strlen(untapcall)))
	{
		return 0;
	}
	
	// find ip1 and ip2 based on call type
	switch (callHandle->handleType)
	{
	case SCC_eH323CallHandle:
		if (callHandle->handle.h323CallHandle.remoteSet)
		{
			ip1 = callHandle->handle.h323CallHandle.remoteSet[0].rtpaddr;
		}
		if (callHandle->handle.h323CallHandle.localSet)
		{
			ip2 = callHandle->handle.h323CallHandle.localSet[0].rtpaddr;
		}
		break;
	case SCC_eSipCallHandle:
		if (callHandle->handle.sipCallHandle.remoteSet.remoteSet_val)
		{
			ip1 = callHandle->handle.sipCallHandle.remoteSet.remoteSet_val[0].rtpaddr;
		}
		if (callHandle->handle.sipCallHandle.localSet.localSet_val)
		{
			ip2 = callHandle->handle.sipCallHandle.localSet.localSet_val[0].rtpaddr;
		}
		break;
	default:
		break;
	}

	if (ip1 && ip2)
	{
		if (status == 1)
		{
			sprintf(cmd, "%s %s %s", tapcall, FormatIpAddress(ip1, str1),
				FormatIpAddress(ip2, str2));
		}
		else
		{
			sprintf(cmd, "%s %s %s", untapcall, FormatIpAddress(ip1, str1),
				FormatIpAddress(ip2, str2));
		}

		NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s Launching script %s\n", fn, cmd));
		ThreadDispatch(poolid, lpcid, (void *(*)(void*))ExecuteScript2, cmd, 1, PTHREAD_SCOPE_PROCESS,
			 SCHED_RR, 50);
		return 0;
	}
	else
	{
		NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s Could not launch script\n", fn));
	}

	return -1;
}

// Frees up the evtptr in all cases
int
bridgeSendH323Event (SCC_EventBlock *evtPtr)
{
	static char	fn[] = "bridgeSendH323Event";
	CallHandle 			callHandleBlk = {0};
	char	str[256];

    if(CacheFind(callCache,evtPtr->callID,&callHandleBlk,sizeof(CallHandle)) <0)
    {
		NETERROR(MIWF,("%s CallHandle Not found %s\n",
			fn,sccEventBlockToStr(evtPtr, str)));
		goto _error;
    }

	if(callHandleBlk.bridgeEventProcessor(evtPtr) <0 )
	{
		NETDEBUG(MIWF, NETLOG_DEBUG2,
			("%s bridgeEventProcessor returned error\n",fn));
		goto _error;
	}

	return 0;

_error:
	H323FreeEvent(evtPtr);
	return -1;
}

// callError is callError for CDRs which
// will be generated
void *
BridgeEndCalls(void *arg)
{
	char fn[] = "BridgeEndCalls():";
	SCC_EventBlock *evtPtr;
	CallHandle *callHandle;
	SipAppCallHandle *pSipData;
	H323EventData *pH323Data;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	for (callHandle = CacheGetFirst(callCache); callHandle;
			callHandle = CacheGetNext(callCache, callHandle->callID))
	{
		evtPtr = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
		memset(evtPtr, 0,sizeof(SCC_EventBlock));

		// Fill up the block
		switch (callHandle->handleType)
		{
 		case SCC_eSipCallHandle:

			if (SCMCALL_CheckState(callHandle->callID))
			{
				// call has been replciated, it should not be hung up
				break;
			}

			evtPtr->event = Sip_eBridgeError;
			memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
			memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

			evtPtr->callDetails.callError = SCC_errorSwitchover;
			evtPtr->callDetails.lastEvent = callHandle->lastEvent;

			pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
			memset(pSipData,0,sizeof(SipAppCallHandle));

			memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
			memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
			
			evtPtr->data = pSipData;

			GisDeleteCallFromConf(pSipData->callID,evtPtr->confID);
			if (sipBridgeEventProcessor(evtPtr) !=0)
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
					("%s sipBridgeEventProcessor Failed\n",fn));
			}

			break;
			
		case SCC_eH323CallHandle:
			evtPtr->event = SCC_eBridgeReleaseComp;
			memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
			memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

			evtPtr->callDetails.callError = SCC_errorSwitchover;
			evtPtr->callDetails.lastEvent = callHandle->lastEvent;

			if (callHandle->bridgeEventProcessor == NULL)
			{
				NETERROR(MBRIDGE, ("%s bridgeEventProcessor is NULL \n",
					fn));
				break;
			}

           	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
            memset(pH323Data,0,sizeof(H323EventData));
			memcpy(pH323Data->callID, evtPtr->callID, CALL_ID_LEN);
            evtPtr->data = (void *) pH323Data;

			if (callHandle->bridgeEventProcessor(evtPtr) <0 )
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
					("%s bridgeEventProcessor Failed\n",fn));
			}

			break;
			
		default:
			NETERROR(MBRIDGE, ("%s Unknown call handle type=%d\n",
				fn, callHandle->handleType));
			break;
		}
	}
	
	CacheReleaseLocks(callCache);
	return(NULL);
}

// Called when backup becomes primary to recompute CAC
void *
BridgeInitCalls(void *arg)
{
	char fn[] = "BridgeInitCalls():";
	int hangup, i;
	CallHandle *callHandle;
	ConfHandle *confHandle;
	char zerocallID[CALL_ID_LEN] = { 0 };
	char callIDStr[CALL_ID_LEN];
	timedef now, ellapsed;
	int remaining_duration;
	int orig_session_expires;
	struct itimerval timerval;
	char *callid;

	if (doScm == 0)
	{
		return NULL;
	}

	if( !scmEnabled() )
	{
		NETDEBUG(MSCM, NETLOG_DEBUG1, 
			("%s SCM Feature not licensed\n", fn));
		return NULL;
	}

	NETDEBUG(MSCM, NETLOG_DEBUG1,
		("%s Checking for existing calls\n", fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);

	// Make sure what we have are SIP call legs, they
	// are all with confs with two legs
	// and recompute the CAC
	for (confHandle = CacheGetFirst(confCache); confHandle;
			confHandle = CacheGetNext(confCache, confHandle->confID))
	{
		hangup = 0;

	 	if (confHandle->ncalls <= 0)
		{
			NETERROR(MSCM, ("%s Found a conf with no call legs. Delete Failure!\n",
				fn));
			continue;
		}

	 	if (confHandle->ncalls != 2)
		{
			hangup = 1;
		}
		
		for(i = 0; i<confHandle->ncalls;++i)
		{
			if (memcmp(confHandle->callID[i], zerocallID, CALL_ID_LEN))
			{
				callHandle = CacheGet(callCache, confHandle->callID[i]);
				if (!callHandle ||
					(callHandle->handleType != SCC_eSipCallHandle) ||
						(callHandle->state != Sip_sConnectedAck))
				{
					hangup = 1;
				}
			}
			else
			{
				hangup = 1;
			}

			if (!hangup)
			{
				NETDEBUG(MSCM, NETLOG_DEBUG1, ("%s Checking for CAC for %s\n", 
					fn, CallID2String(callHandle->callID,callIDStr)));

				// If regid does not exist, this call will still work
				// so we don't need to check for allowSrcAll
				if (GwPortAllocCall(&callHandle->phonode,1, callHandle->callSource) < 0)
				{
					NETDEBUG(MSCM, NETLOG_DEBUG1, 
						("%s Check for CAC failed for %s on %s/%lu\n", 
						fn, CallID2String(callHandle->callID,callIDStr), 
						callHandle->phonode.regid, callHandle->phonode.uport));
					hangup = 1;
				}
			}

			if (!hangup)
			{
				if(max_call_duration != 0)
				{
					NETDEBUG(MSCM, NETLOG_DEBUG1, ("%s Starting max call duration timer for %s\n", 
						fn, CallID2String(callHandle->callID,callIDStr)));

					timedef_cur(&now);

					timedef_sub(&now, &callHandle->callStartTime, &ellapsed);

					if((remaining_duration = max_call_duration - timedef_rndsec(&ellapsed)) > 0)
					{
						if((callid = malloc(CALL_ID_LEN)))
						{
							memcpy(callid, callHandle->callID, CALL_ID_LEN);

							bzero(&timerval, sizeof(struct itimerval));

							timerval.it_value.tv_sec = remaining_duration;

							callHandle->max_call_duration_tid = timerAddToList(&localConfig.timerPrivate, &timerval,
																			0, PSOS_TIMER_REL, "MAX_CALL_DURATION",
																			disconnectCallAtMaxCallDuration,
																			callid);
						}
						else
						{
							NETERROR(MSIP, ("%s unable to malloc callid\n", fn));
						}
					}
					else
					{
						hangup = 1;
					}
				}
			}

			if (!hangup)
			{
				if (callHandle->sessionTimerActive)
				{
					NETDEBUG(MSCM, NETLOG_DEBUG1, ("%s Starting session timer for %s\n", 
						fn, CallID2String(callHandle->callID,callIDStr)));

					// Save the current value
					orig_session_expires = callHandle->sessionExpires;

					timedef_cur(&now);

					timedef_sub(&now, &callHandle->callConnectTime, &ellapsed);

					// Calculate the remaining duration for session
				 	if((remaining_duration = timedef_rndsec(&ellapsed) % (callHandle->sessionExpires / 2)) < 0)
					{
						remaining_duration = 0;
					}

				 	callHandle->sessionExpires = 2 * remaining_duration;

					// Start the session timer with the remaining value
					SipStartSessionTimer(callHandle);

					// Restore the original value
					callHandle->sessionExpires = orig_session_expires;
				}
			}

			if (hangup)
			{
				NETERROR(MSCM, ("%s Disconnecting call %s type %d state %d, legs %d\n",
					fn, CallID2String(confHandle->callID[i],callIDStr), 
					callHandle->handleType, callHandle->state, confHandle->ncalls));

				disconnectCallLeg(callHandle, SCC_errorSCM);
			}
		}
	}

	CacheReleaseLocks(confCache);
	CacheReleaseLocks(callCache);

	return NULL;
}

int
disconnectCallsOnRealm(unsigned long rsa, int sccError)
{
	char fn[] = "disconnectCallsOnRealm():";
	SCC_EventBlock *evtPtr;
	CallHandle *callHandle;
	SipAppCallHandle *pSipData;
	char callIDStr[CALL_ID_LEN];
	H323EventData *pH323Data;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	for (callHandle = CacheGetFirst(callCache); callHandle;
			callHandle = CacheGetNext(callCache, callHandle->callID))
	{
		if (callHandle->realmInfo == NULL)
		{
			NETERROR(MBRIDGE, ("%s call %s does not have realm info\n", 
				fn, (char*) CallID2String(callHandle->callID,callIDStr)));
			continue;
		}

		if (callHandle->realmInfo->rsa != rsa)
		{
			continue;
		}

		evtPtr = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
		memset(evtPtr, 0,sizeof(SCC_EventBlock));

		// Fill up the block
		switch (callHandle->handleType)
		{
 		case SCC_eSipCallHandle:

			evtPtr->event = Sip_eBridgeError;
			memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
			memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

			evtPtr->callDetails.callError = sccError;
			evtPtr->callDetails.lastEvent = callHandle->lastEvent;

			pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
			memset(pSipData,0,sizeof(SipAppCallHandle));

			memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
			memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
			
			evtPtr->data = pSipData;

			GisDeleteCallFromConf(pSipData->callID,evtPtr->confID);
			if (sipBridgeEventProcessor(evtPtr) !=0)
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
					("%s sipBridgeEventProcessor Failed\n",fn));
			}

			break;
			
		case SCC_eH323CallHandle:
			evtPtr->event = SCC_eBridgeReleaseComp;
			memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
			memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

			evtPtr->callDetails.callError = sccError;
			evtPtr->callDetails.lastEvent = callHandle->lastEvent;

			if (callHandle->bridgeEventProcessor == NULL)
			{
				NETERROR(MBRIDGE, ("%s bridgeEventProcessor is NULL \n",
					fn));
				break;
			}

       		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
       		memset(pH323Data,0,sizeof(H323EventData));
       		memcpy(pH323Data->callID, evtPtr->callID, CALL_ID_LEN);
       		evtPtr->data = (void *) pH323Data;

			if (callHandle->bridgeEventProcessor(evtPtr) <0 )
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
					("%s bridgeEventProcessor Failed\n",fn));
			}

			break;
			
		default:
			NETERROR(MBRIDGE, ("%s Unknown call handle type=%d\n",
				fn, callHandle->handleType));
			break;
		}
	}
	
	CacheReleaseLocks(callCache);

	return 0;
}


int 
disconnectCall(char *callid1, int sccError)
{
	char fn[] = "disconnectCall():";
	char callid[CALL_ID_LEN] = {0};
	CallHandle *callHandle1;
	CallHandle	callHandle2 = { 0 };
	SCC_EventBlock *evPtr;
	SipAppMsgHandle *appMsgHandle;
	H323EventData *pH323Data;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if (!(callHandle1 = CacheGet(callCache, callid1)))
	{
		NETERROR(MBRIDGE, ("%s cannot find callHandle\n", fn));
		CacheReleaseLocks(callCache);
		return -1;
	}

	if(getPeerCallID(callHandle1->confID, callHandle1->callID, callid) < 0)
	{
		NETERROR(MBRIDGE, ("%s failed to get callid for leg2\n", fn));
	}
	else
	{
		if(CacheFind(callCache, callid, &callHandle2, sizeof(CallHandle)) < 0 )
		{
			NETERROR(MBRIDGE, ("%s failed to find callHandle for leg2\n", fn));
		}
		else
		{
			GisDeleteCallFromConf(callHandle2.callID, callHandle2.confID);

			/* Allocate an SCC Event Block and fill it with data */
			evPtr = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
			memset(evPtr, 0, sizeof(SCC_EventBlock));

			memcpy(evPtr->confID, callHandle2.confID, CONF_ID_LEN);
			memcpy(evPtr->callID, callHandle2.callID, CALL_ID_LEN);
			evPtr->callDetails.callError = sccError;
			evPtr->callDetails.lastEvent = callHandle2.lastEvent;

			if(callHandle2.handleType == SCC_eSipCallHandle)
			{
				evPtr->event = Sip_eBridgeError;
				evPtr->data = appMsgHandle = SipAllocAppMsgHandle();
				memcpy(appMsgHandle->confID, evPtr->confID, CONF_ID_LEN);
				memcpy(appMsgHandle->callID, evPtr->callID, CALL_ID_LEN);
				appMsgHandle->maxForwards = sipmaxforwards;

				if(sipBridgeEventProcessor(evPtr) != 0)
				{
					NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s sipBridgeEventProcessor Failed\n",fn));
				}
			}
			else
			{
				evPtr->event = SCC_eBridgeReleaseComp;

				if(callHandle2.bridgeEventProcessor)
				{
       				pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
		       		memset(pH323Data,0,sizeof(H323EventData));
       				memcpy(pH323Data->callID, evPtr->callID, CALL_ID_LEN);
       				evPtr->data = (void *) pH323Data;

					if(callHandle2.bridgeEventProcessor(evPtr) < 0)
					{
						NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s bridgeEventProcessor Failed\n",fn));
					}
				}
				else
				{
					NETERROR(MBRIDGE, ("%s bridgeEventProcessor is NULL \n", fn));
				}
			}
		}
	}

	GisDeleteCallFromConf(callHandle1->callID, callHandle1->confID);

	/* Allocate an SCC Event Block and fill it with data */
	evPtr = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(evPtr, 0, sizeof(SCC_EventBlock));

	memcpy(evPtr->confID, callHandle1->confID, CONF_ID_LEN);
	memcpy(evPtr->callID, callHandle1->callID, CALL_ID_LEN);
	evPtr->callDetails.callError = sccError;
	evPtr->callDetails.lastEvent = callHandle1->lastEvent;

	if(callHandle1->handleType == SCC_eSipCallHandle)
	{
		evPtr->event = Sip_eBridgeError;
		evPtr->data = appMsgHandle = SipAllocAppMsgHandle();
		memcpy(appMsgHandle->confID, evPtr->confID, CONF_ID_LEN);
		memcpy(appMsgHandle->callID, evPtr->callID, CALL_ID_LEN);
		appMsgHandle->maxForwards = sipmaxforwards;

		if(sipBridgeEventProcessor(evPtr) != 0)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s sipBridgeEventProcessor Failed\n",fn));
		}
	}
	else
	{
		evPtr->event = SCC_eBridgeReleaseComp;

		if(callHandle1->bridgeEventProcessor)
		{
       		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
       		memset(pH323Data,0,sizeof(H323EventData));
       		memcpy(pH323Data->callID, evPtr->callID, CALL_ID_LEN);
       		evPtr->data = (void *) pH323Data;

			if(callHandle1->bridgeEventProcessor(evPtr) < 0)
			{
				NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s bridgeEventProcessor Failed\n",fn));
			}
		}
		else
		{
			NETERROR(MBRIDGE, ("%s bridgeEventProcessor is NULL \n", fn));
		}
	}

	CacheReleaseLocks(callCache);

	return 1;
}


int 
disconnectCallAtMaxCallDuration(struct Timer *timer)
{
	char fn[] = "disconnectCallAtMaxCallDuration():";
	char		*callid;
	CallHandle	*callHandle;

	if(timer != NULL)
	{
		callid = timer->data;

		disconnectCall(callid, SCC_errorMaxCallDuration);

		CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
		if (callHandle = CacheGet(callCache, callid))
		{
			callHandle->max_call_duration_tid = 0;
		}
		else
		{
			NETERROR(MBRIDGE, ("%s cannot find callHandle \n", fn));
		}
		CacheReleaseLocks(callCache);

		free(timer->data);
		timerFreeHandle(timer);

		return 0;
	}
	else
	{
		NETERROR(MBRIDGE, ("%s timerid Null\n", fn));
		return -1;
	}
}

void 
disconnectCallLeg(CallHandle *callHandle, int sccError)
{
	char fn[] = "disconnectCallLeg():";
	SCC_EventBlock *evtPtr;
	SipAppCallHandle *pSipData;
	H323EventData *pH323Data;

	evtPtr = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(evtPtr, 0,sizeof(SCC_EventBlock));

	// Fill up the block
	switch (callHandle->handleType)
	{
 	case SCC_eSipCallHandle:

		evtPtr->event = Sip_eBridgeError;
		memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
		memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

		evtPtr->callDetails.callError = sccError;
		evtPtr->callDetails.lastEvent = callHandle->lastEvent;

		pSipData = (SipAppCallHandle *) malloc(sizeof(SipAppCallHandle));
		memset(pSipData,0,sizeof(SipAppCallHandle));

		memcpy(pSipData->confID,evtPtr->confID,CONF_ID_LEN);
		memcpy(pSipData->callID,evtPtr->callID,CALL_ID_LEN);
			
		evtPtr->data = pSipData;

		GisDeleteCallFromConf(pSipData->callID,evtPtr->confID);
		if (sipBridgeEventProcessor(evtPtr) !=0)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
				("%s sipBridgeEventProcessor Failed\n",fn));
		}

		break;
			
	case SCC_eH323CallHandle:
		evtPtr->event = SCC_eBridgeReleaseComp;
		memcpy(evtPtr->confID, callHandle->confID, CONF_ID_LEN);
		memcpy(evtPtr->callID, callHandle->callID, CALL_ID_LEN);

		evtPtr->callDetails.callError = SCC_errorSwitchover;
		evtPtr->callDetails.lastEvent = callHandle->lastEvent;

		if (callHandle->bridgeEventProcessor == NULL)
		{
			NETERROR(MBRIDGE, ("%s bridgeEventProcessor is NULL \n",
				fn));
			break;
		}

		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
   		memset(pH323Data,0,sizeof(H323EventData));
		memcpy(pH323Data->callID, evtPtr->callID, CALL_ID_LEN);
   		evtPtr->data = (void *) pH323Data;

		if (callHandle->bridgeEventProcessor(evtPtr) <0 )
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4,
				("%s bridgeEventProcessor Failed\n",fn));
		}

		break;
		
	default:
		NETERROR(MBRIDGE, ("%s Unknown call handle type=%d\n",
			fn, callHandle->handleType));
		break;
	}
}

// Locks must be acquired before calling this fn
int
bridgeFindCallLegsInfoForEvent(
	SCC_EventBlock *evtPtr, 
	CallHandle **callHandle1, 
	CallHandle **callHandle2, 
	ConfHandle **confHandle
)
{
	int i;
	char *destCallID = NULL;

	*callHandle2 = NULL;

	*confHandle = CacheGet(confCache, evtPtr->confID);

	*callHandle1 = CacheGet(callCache, evtPtr->callID);

	if (!(*confHandle))
	{
		return 0;
	}

	for (i=0; i < (*confHandle)->ncalls; i++)
	{
		if (memcmp((*confHandle)->callID[i], evtPtr->callID, CALL_ID_LEN))
		{
			destCallID = (*confHandle)->callID[i];
			break;
		}
	}

	if (destCallID)
	{
		*callHandle2 = CacheGet(callCache, destCallID);
	}

	return 0;
}

int
BridgeCheckPendingFCEvents(CallHandle *callHandle)
{
	char fn[] = "BridgeCheckPendingFCEvents():";
	SCC_EventBlock 		*evtPtr = NULL;

	if (callHandle->flags & FL_CALL_FCPEND)
	{
		evtPtr = listDeleteFirstItem(callHandle->fcevtList);

		if (callHandle->fcevtList->head->nitems == 0)
		{
			callHandle->flags &= ~FL_CALL_FCPEND;
		}

		if (evtPtr == NULL)
		{
			NETDEBUG(MBRIDGE, NETLOG_DEBUG4, ("%s no more events\n", fn));
			return 0;
		}

  		evtPtr->evtProcessor(evtPtr);
	}

	return 0;
}

// Locate the realm given the realm on which the host has been received from
// hostip is used when host is NULL.
// If an exact match is found with one of the MSWs domains, mydomain
// is returned
int
RealmLocate(int srealmId, char *host, unsigned long hostip, int *mydomain)
{
	CacheRealmEntry *srealmEntry;
	Subnet sub;
	int realmId = -1;
	CacheTableInfo *pxCacheInfo = NULL;
	int herror = 0;
	*mydomain = 0;

	CacheGetLocks(regCache, LOCK_WRITE,LOCK_BLOCK);

	if (pxCacheInfo = CacheGet(uriCache, host))
	{
		*mydomain = pxCacheInfo->data.stateFlags & CL_SIPDOMAIN;
		realmId = pxCacheInfo->data.realmId;
	}

	CacheReleaseLocks(regCache);

	if (*mydomain || (realmId > 0))
	{
		return realmId;
	}

	if (host)
	{
		hostip = ResolveDNS(host, &herror);
	}

	CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);
	if (srealmEntry = CacheGet(realmCache, &srealmId))
	{
		if (hostip == srealmEntry->realm.rsa)
		{
			*mydomain = 1;
			realmId =  srealmId;
			goto _return;
		}

		if ((hostip & srealmEntry->realm.mask) ==
			(srealmEntry->realm.rsa & srealmEntry->realm.mask))
		{
			realmId =  srealmId;
			goto _return;
		}
		else if(srealmEntry = CacheGet(rsaCache, &hostip))
		{
			*mydomain = 1;
			realmId =  srealmEntry->realm.realmId;
			goto _return;
		}
		
	}
	
	sub.subnetip = hostip;

	// Try to do a longest match search
	srealmEntry = GetSubnetLongestMatch(rsapubnetsCache, &sub);

	if (srealmEntry)
	{
		if (hostip == srealmEntry->realm.rsa)
		{
			*mydomain = 1;
		}

		realmId = srealmEntry->realm.realmId;
		goto _return;
	}
	else if (*defaultRealm)
	{
		realmId = (*defaultRealm)->realm.realmId;
		goto _return;
	}


_return:
	CacheReleaseLocks(realmCache);
	return realmId;
}

void
AddCodecsAttribs(SipAppCallHandle *pSipData, CodecType codecs[], int numCodecs, char *AttribPairs[][2], int numAttribs, int vendor)
{	

	RTPSet		*newSet;
	SDPAttr		*newAttr;
	int 		i, n;	

	n = pSipData->nlocalset;
	pSipData->nlocalset += numCodecs;
	newSet = (RTPSet *)(calloc(pSipData->nlocalset, sizeof(RTPSet)));
	memcpy(newSet, pSipData->localSet, n*sizeof(RTPSet));
	for (i = 0; i < numCodecs; i++) {
		newSet[n+i].codecType = codecs[i];
		newSet[n+i].rtpaddr = newSet[0].rtpaddr;
		newSet[n+i].rtpport = newSet[0].rtpport;
	}
	if (pSipData->localSet) {
		free(pSipData->localSet);
	}
	pSipData->localSet = newSet;

	n = pSipData->attr_count;
	pSipData->attr_count += numAttribs;
	newAttr = (SDPAttr *) calloc(pSipData->attr_count, sizeof(SDPAttr));
	memcpy(newAttr, pSipData->attr, n*sizeof(SDPAttr));
	for (i = 0; i < numAttribs; i++) {
		SipCreateMediaAttrib(&newAttr[n+i], 0, AttribPairs[i][0], AttribPairs[i][1]);
	}
	if (pSipData->attr) {
		free(pSipData->attr);
	}
	pSipData->attr = newAttr;

	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, vendor);
}

void
RemoveCodecsAttribs(SipAppCallHandle *pSipData, CodecType codecs[], int numCodecs, char *AttribPairs[][2], int numAttribs, int vendor)
{	
  	int i, j;

	for (i = 0; i < pSipData->nlocalset; i++) {
	  	for (j = 0; j < numCodecs; j++) {
			if (pSipData->localSet[i].codecType == codecs[j]) {
				pSipData->nlocalset -= 1;
				if (i < pSipData->nlocalset) {	//shift the array
					bcopy((void *)(&pSipData->localSet[i+1]), (void *)(&pSipData->localSet[i]), (pSipData->nlocalset-i)*sizeof(RTPSet));
				}
				bzero((void *)(&pSipData->localSet[pSipData->nlocalset]), sizeof(RTPSet));
				i--; //return to the same index in array
				break;
			}
		}
	}
	for (i = 0; i < pSipData->attr_count; i++) {
		for (j = 0; j < numAttribs; j++) {
			if ( ( (!strcmp(pSipData->attr[i].name, AttribPairs[j][0]) ) &&
				 ( pSipData->attr[i].value &&
				 (!strncmp(pSipData->attr[i].value, AttribPairs[j][1], strlen(AttribPairs[j][1])))  ) ) ) {
				pSipData->attr_count -= 1;
				if (i < pSipData->attr_count) {	//shift the array
					bcopy((void *)(&pSipData->attr[i+1]), (void *)(&pSipData->attr[i]), (pSipData->attr_count-i)*sizeof(SDPAttr));
				}
				bzero((void *)(&pSipData->attr[pSipData->attr_count]), sizeof(SDPAttr));
				i--; //return to the same index in array
				break;
			}
		}
	}
	sanitizeCodecs4sip(pSipData, &pSipData->nlocalset, vendor);
}

/*
 * input ipaddr argument is a pointer to an ipaddr, port location
 * should have callCache locks before calling this function
 */
CallHandle *
GetMediaSrcFromTip(unsigned int *ipaddr)
{
	CallHandle *ch;	

	ch = CacheGet(tipCache, ipaddr);
	if (!ch) {
		return NULL;
	}

	return (CacheGet(callCache, CallFceSrcCallID(ch)));
}

int
CallFceRxPortUsed(CallHandle *ch)
{
	unsigned char *cnt = &CallFceRefCount(ch);
	char ipstr[32];

	if (*cnt & 0x1) {
		NETERROR(MBRIDGE, 
				 ("Attempt to use rx for %s:%d, which is already in use\n", \
				  FormatIpAddress(CallFceTranslatedIp(ch), ipstr), \
				  CallFceTranslatedPort(ch)));
	}

	*cnt |= 0x1;	
	return (*cnt & 0x1);
}

int
CallFceRxPortFreed(CallHandle *ch)
{
	unsigned char *cnt = &CallFceRefCount(ch);
	char ipstr[32];

	if ((*cnt & 0x1) == 0) {
		NETERROR(MBRIDGE, 
				 ("Attempt to free rx for %s:%d, which is already free\n", \
				  FormatIpAddress(CallFceTranslatedIp(ch), ipstr), \
				  CallFceTranslatedPort(ch)));
	}
	else {
		*cnt |= (~0x1);	
	}

	return (*cnt & 0x1);
}

int
CallFceTxPortUsed(CallHandle *ch)
{
	unsigned char *cnt = &CallFceRefCount(ch);

	*cnt = (((*cnt >> 1)  + 1) << 1) + (*cnt & 0x1);
	return (*cnt >> 1);
}

int
CallFceTxPortFreed(CallHandle *ch)
{
	unsigned char *cnt = &CallFceRefCount(ch);
	char ipstr[32];

	if ((*cnt >> 0x1) == 0) {
		NETERROR(MBRIDGE, 
				 ("Attempt to free tx for %s:%d, which is already free\n", \
				  FormatIpAddress(CallFceTranslatedIp(ch), ipstr), \
				  CallFceTranslatedPort(ch)));
	}
	else {
		*cnt = (((*cnt >> 1)  - 1) << 1) + (*cnt & 0x1);
	}

	return (*cnt >> 1);
}
