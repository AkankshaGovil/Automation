/*
 * lrq.c
 * contains functions for handling incoming 
 * and outgoing lrq's
 */

#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "ipc.h"

#include "uh323.h"
#include "uh323cb.h"

#include "gis.h"
#include "uh323inc.h"
#include "h323realm.h"
#include "callsm.h"
#include "nxosd.h"
#include <malloc.h>

#include "gk.h"
#include "bridge.h"
#include "handles.h"
#include "callid.h"
#include "stkutils.h"
#include "resutils.h"
#include "ls.h"
#include "ipstring.h"
#include "log.h"

extern void GkGetAlternateEP( HRAS hsRas, char *responseName, char *calledpn, CallHandle *callHandle );

int
GkInitLRQHandleFromLRQ(HRAS hsRas, HCALL hsCall, 
					   LRQHandle *lrqHandle)
{
	char fn[] = "GkInitLRQHandleFromLRQ():";
	INT32 tlen;
	cmAlias number;
	BYTE string[80];
	int index = 0;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);

	/* Extract the Destination Info */
	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = (char *)string;

	while (cmRASGetParam(hsRas, cmRASTrStageRequest, 
							  cmRASParamDestInfo, index++, 
							  &tlen, (char *)&number) >= 0)
	{
		  if (number.type == cmAliasTypeE164) 
		  {
			   NETDEBUG(MLRQ, NETLOG_DEBUG1, 
						("%s DestInfo contains %s\n",
						 fn, number.string));
			   nx_strlcpy(lrqHandle->rfphonode.phone, 
					number.string, PHONE_NUM_LEN);
			   BIT_SET(lrqHandle->rfphonode.sflags, ISSET_PHONE);
		  }

		  /* Extract the Destination Info */
		  tlen = sizeof(cmAlias);
		  number.length = 80;
		  number.string = (char *)string;
	}

	return 0;
}

int
GKHandleLCF(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	char fn[] = "GKHandleLCF():";
	cmTransportAddress dstSignalAddr;
	int addrlen = sizeof(cmTransportAddress);
	cmEndpointType eType;
	char *callID1 = (char *)haRas;
	CallHandle *callHandle1, *callHandle2;
	char callID2[CALL_ID_LEN];
	SCC_EventBlock *evtPtr = (SCC_EventBlock *)haRas; 
	H323EventData		*pH323Data;
	INT32 tlen;
	cmAlias number;
	BYTE string[80];
	int index = 0;
	ConfHandle *confHandle;
	int egressTokenNodeId;

	if (evtPtr == NULL)
	{
		NETERROR(MLRQ, ("%s No Event Pointer\n", fn));
		return -1;
	}

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	/* Extract the call id, and see if the call is still up.
	 * If it is, we now modify the handle, and insert the information
	 * we have obtained from the LRQ. After this we call MakeCall
	 */ 
	callHandle1 = CacheGet(callCache,evtPtr->callID);
	if (callHandle1 == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
		goto _error;
	} 

	callHandle1->lastEvent = SCC_evt323LCFRx;

	/* We must extract the first call handle, and fill in
	 * the data
	 */
	getPeerCallID(evtPtr->confID,evtPtr->callID,callID2);

	callHandle2 = CacheGet(callCache,callID2);
	if (callHandle2 == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle for other leg \n",fn));
		CacheReleaseLocks(callCache);
	 	return GkHandleLRQFailure(haRas, hsRas, -2);
	} 

	/* Extract the Endpoint type */
	if (cmRASGetParam(hsRas, cmRASTrStageConfirm,
				   cmRASParamEndpointType,
					   0,
					   (int *)&eType,
					   NULL) < 0)
	{
		  NETDEBUG(MLRQ, NETLOG_DEBUG1,
				   ("%s cmRASGetParam cmRASParamEndpointType failed\n", fn));
	}
	else
	{
		  NETDEBUG(MLRQ, NETLOG_DEBUG1,
				   ("%s Endpoint Type is %d\n", fn, eType));
	}
	 
	/* Extract the Q.931 IP address and port */
	if (cmRASGetParam(hsRas, cmRASTrStageConfirm,
					   cmRASParamCallSignalAddress,
					   0,
					   &addrlen,
					   (char *)&dstSignalAddr) < 0)
	{
		  NETDEBUG(MLRQ, NETLOG_DEBUG1,
				   ("%s cmRASGetParam cmRASParamDestCallSignalAddress failed\n", fn));
	}

	/* Change the signalling address.
	 * Also, copy the old address (gk address),
	 * so that we can send a DRQ...
	 */ 
	callHandle2->rfphonode.ipaddress.l = ntohl(dstSignalAddr.ip);
	H323rfcallsigport(callHandle2) = dstSignalAddr.port;

	if(!(pH323Data = evtPtr->data))
	{
		NETERROR(MLRQ,("%s : H323Data Null in event Ptr", fn ));
		goto _error;
	}
	
	// Get token node ID from LCF for leg2.
	// Store it in conf handle and keep a copy of it
	// in call handle of leg 2. The copy in the call handle
	// is never deleted. The token node ID gets deleted 
	// when the conf handle is deleted.
	if (callHandle1->callSource == 1)
	{
		if ((egressTokenNodeId = getLcfTokenNodeId(hsRas)) < 0)
		{
        	NETDEBUG(MLRQ,NETLOG_DEBUG4,
				("%s getLcfTokenNodeId failed",fn ));
		}
		else
		{
			CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
			if (!(confHandle = CacheGet(confCache,evtPtr->confID)))
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
			   		("%s Did not find conf handle \n",fn));
				CacheReleaseLocks(confCache);
				goto _error;
			} 

			GisFreeEgressTokenNodeId(confHandle);

			confHandle->egressTokenNodeId = egressTokenNodeId;
    		H323egressTokenNodeId(callHandle1) = egressTokenNodeId;

			CacheReleaseLocks(confCache);
		}
	}

	/* Extract the Destination Info */
	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = (char *)string;

	while (cmRASGetParam(hsRas, cmRASTrStageConfirm, 
							  cmRASParamDestInfo, index++, 
							  &tlen, (char *)&number) >= 0)
	{
		  if (number.type == cmAliasTypeE164) 
		  {
			   NETDEBUG(MLRQ, NETLOG_DEBUG1, 
						("%s DestInfo contains %s\n",
						 fn, number.string));
			   nx_strlcpy(pH323Data->calledpn, 
					number.string, PHONE_NUM_LEN);

				// Fix the dialled # field in the CDRs
				strcpy(callHandle2->rfphonode.phone, 
					pH323Data->calledpn);
				strcpy(callHandle1->phonode.phone, 
					pH323Data->calledpn);
		  }

		  /* Extract the Destination Info */
		  tlen = sizeof(cmAlias);
		  number.length = 80;
		  number.string = (char *)string;
	}

	if (BIT_TEST(callHandle1->phonode.cap, CAP_TPG))
	{
		NETDEBUG(MLRQ, NETLOG_DEBUG4, ("%s checking tech prefix\n", fn));

		// We need to find what was originally dialled
		// this in in the callhandle1, as of now...
		strcpy(pH323Data->calledpn, H323dialledNumber(callHandle2));

		// Fix the dialled # field in the CDRs
		strcpy(callHandle2->rfphonode.phone, 
			H323dialledNumber(callHandle2));

		strcpy(callHandle1->phonode.phone, 
			H323dialledNumber(callHandle2));
	}

	NETDEBUG(MLRQ, NETLOG_DEBUG4,
		("%s LRQ Destination is %s/%d phone=%s\n",
		fn, 
		ULIPtostring(callHandle2->rfphonode.ipaddress.l), 
		H323rfcallsigport(callHandle2), pH323Data->calledpn));

	pH323Data->destip = ntohl(dstSignalAddr.ip);
	pH323Data->destport = dstSignalAddr.port;
	/* get alternate endPoints */
	GkGetAlternateEP( hsRas, "locationConfirm", pH323Data->calledpn, callHandle2 );

	CacheReleaseLocks(callCache);

	/* Call the call function */
	evtPtr->evtProcessor = SCCLeg2BridgeMakeCall;
	SCC_QueueEvent(evtPtr);

	return 0;

_error:
	CacheReleaseLocks(callCache);

    H323FreeEvData((H323EventData *)(evtPtr->data));
    free(evtPtr);
    return 0;
}

int
GkHandleLRQFailure(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	char 		fn[] = "GkHandleLRQFailure():";
	char 		callIdStr[CALL_ID_STRLEN];
	SCC_EventBlock *evtPtr = (SCC_EventBlock *)haRas;
	int callError;
	CallHandle 	*callHandle = NULL;


	NETDEBUG(MLRQ, NETLOG_DEBUG4,
		   ("%s Entering haRas = %p, hsRas = %p\n",
			fn, haRas, hsRas));

	/* We have to basically generate an error, so that
	 * leg 1 call is released.
	 */	
    if (evtPtr == NULL)
	{
		NETERROR(MLRQ, ("%s Null appHandle\n", fn));
		return 0;
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if(!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle %s\n",
				fn, (char*) CallID2String(evtPtr->callID, callIdStr)));

		CacheReleaseLocks(callCache);
		goto _error;
	}

	callHandle->lastEvent = SCC_evt323LRJRx;

	switch (reason)
	{
	case -1:
		// -1 means timeout, there is no reason
		callError = SCC_errorDestTimeout;
		callHandle->lastEvent = SCC_evt323LRQTmout;
		break;
	case -2:
		// -2 means that by the time LCF was received, 
		// the call handle for the other leg was deleted
		callError = SCC_errorNoCallHandle;
		callHandle->lastEvent = SCC_evt323LCFRx;
		// Set reason to -1 so it is translated to correct RAS reason
		reason++;
		break;
	case cmRASReasonNotRegistered:
		callError = SCC_errorDestNoRoute;
		break;
	case cmRASIncompleteAddress:
		callError = SCC_errorIncompleteAddress;
		break;
    case cmRASReasonResourceUnavailable:
        callError = SCC_errorResourceUnavailable;
        break;

	default:
		callError = SCC_errorUndefinedReason;
		break;
	}

	callHandle->callError = callError;

	// Take reason off by 1, 0=> timeout or no call handle
	callHandle->rasReason = reason+1;

	/* Close this call */
	GkCallDropReasonRAS(H323hsCall(callHandle), 
		callHandle->callError, callHandle->rasReason);

	cmCallDrop(H323hsCall(callHandle));

	CacheReleaseLocks(callCache);

_error:
    H323FreeEvent(evtPtr);
	return 0;
}

int
GKHandleLRJ(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	 char fn[] = "GKHandleLRJ():";

	 NETDEBUG(MLRQ, NETLOG_DEBUG4, 
		   ("%s Entering haRas = %p, hsRas = %p, reason = %d\n",
			fn, haRas, hsRas, reason));

	 return GkHandleLRQFailure(haRas, hsRas, reason);
}

int
GKHandleLRQTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "GKHandleLRQTimeout():";

	 NETDEBUG(MLRQ, NETLOG_DEBUG4, 
		   ("%s Entering haRas = %p, hsRas = %p, reason = %d\n",
			fn, haRas, hsRas, 0));

	 return GkHandleLRQFailure(haRas, hsRas, -1);
}

int
GkFindSrcForLrq(
	IN	HRAS			hsRas,
	LRQHandle 			*lrqHandle,
	CacheTableInfo		*scacheInfo)
{
	char fn[] = "GkFindSrcForLrq:";
	int rc = -1, done = 0;
	PhoNode *phonodep;
	char string[80];
	INT32  tlen;
	int index = 0; 
	cmAlias number;
	char h323id[H323ID_LEN] = { 0 };
	char tg[PHONE_NUM_LEN] = { 0 };
	int nodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	int tglen;

	phonodep = &lrqHandle->phonode;

	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = string;

	cmMeiEnter(UH323Globals()->hApp);

	if ((nodeId = cmGetProperty((HPROTOCOL)hsRas)) > 0)
	{
		if ((nodeId = pvtGetNodeIdByPath(hVal, nodeId,
			"request.locationRequest.circuitInfo.sourceCircuitID.group.group")) > 0)
		{
			if ((tglen = pvtGetString(hVal, nodeId, PHONE_NUM_LEN-1, tg)) > 0)
			{
				tg[tglen] = '\0';
			}
			else
			{
				tg[0] = '\0';
			}
		}
	}

	cmMeiExit(UH323Globals()->hApp);

	while ( (cmRASGetParam(hsRas, cmRASTrStageRequest, 
			cmRASParamSrcInfo, index++, 
			&tlen, (char *)&number)) >= 0)
	{
		if (number.type != cmAliasTypeE164 && number.type != cmAliasTypeH323ID)
		{
			 goto _continue;
		}

		NETDEBUG(MH323, NETLOG_DEBUG4, 
			("%s Found Alias %s in LRQ\n", fn, number.string));

        if (number.type == cmAliasTypeE164)
		{
			strcpy(phonodep->phone, number.string);
			BIT_SET(phonodep->sflags, ISSET_PHONE);
		}
        else if (number.type == cmAliasTypeH323ID)
        {
			utlBmp2Chr(&h323id[0], number.string, number.length);
		}

		rc = FillSourceCacheForCallerId(phonodep, h323id, tg, 0, 
				lrqHandle->rfphonode.phone, scacheInfo);
		done = 1;

		if (rc > 0)
		{
			break;
		}

	_continue:
		tlen = sizeof(cmAlias);
		number.length = 80;
		number.string = string;

	}

	if (!done)
	{
		// We did not find any interesting thing in the aliases
		rc = FillSourceCacheForCallerId(phonodep, h323id, tg, 0,
				lrqHandle->rfphonode.phone, scacheInfo);
	}

	return rc;
}

int
GKHandleLRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	 char fn[] = "GKHandleLRQ():";
	 int rc = -cmRASReasonRequestDenied, callError = SCC_errorNone;
	 INT32 tlen;
	 ResolveHandle *rhandle = NULL;
	 LRQHandle *lrqHandle = NULL;
	 cmTransportAddress 
		  dstSignalAddr = { 0, 0, 1720, cmRASTransportTypeIP },
		  dstRASAddr = { 0, 0, 1719, cmRASTransportTypeIP },
		  rasaddr;
	 int addrlen = sizeof(cmTransportAddress);
	CacheTableInfo scacheInfoEntry = { 0 };
	cmAlias alias;
	BOOL canMapAlias, isstring;
	int nodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	int domainIp,sd,realmId;
	unsigned short domainPort;

	 NETDEBUG(MLRQ, NETLOG_DEBUG1,
		   ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));

#if 0
	// Spec does not mention that we have to do this for LRQs
	 if (GkCompareGkID(hsRas) < 0)
	 {
		  DEBUG(MRRQ, NETLOG_DEBUG4,
				("%s GKID mismatch\n", fn));

		//	rc = -cmRASReasonInvalidPermission;

		  cmRASClose(hsRas);
		  return 0;
	 }
#endif

	 /* Allocate a client handle */
	 lrqHandle = GisAllocCallHandle();

	 lrqHandle->handleType = SCC_eH323CallHandle;
	 H323controlState(lrqHandle) = UH323_sControlIdle;
	 lrqHandle->state = SCC_sIdle;
	 timedef_cur(&lrqHandle->callStartTime);
	 lrqHandle->callSource = 0;
	 lrqHandle->lastEvent = SCC_evt323LRQRx;

	 // lrq doesnt't have a callid
	 generateCallId(lrqHandle->callID);

	 GkInitLRQHandleFromLRQ(hsRas, hsCall, lrqHandle);
	 nx_strlcpy(lrqHandle->inputNumber, lrqHandle->rfphonode.phone, PHONE_NUM_LEN);

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		 NETERROR(MLRQ, ("%s No socket\n", fn));
	}

	if(getQ931RealmInfo(sd,&domainIp,&domainPort,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		callError = SCC_errorH323Internal;
		rc = -GkCallDropLRJReason(NULL, callError);
		goto _return;
	}

	 // Source Identification for calling plan
	 lrqHandle->phonode.ipaddress.l = ntohl(srcAddress->ip);
	 BIT_SET(lrqHandle->phonode.sflags, ISSET_IPADDRESS);
	 lrqHandle->phonode.realmId = realmId;

	if (GkFindSrcForLrq(hsRas, lrqHandle, &scacheInfoEntry) < 0)
	{
		NETDEBUG(MLRQ, NETLOG_DEBUG4,
			("%s Src Cache not found\n", fn));

		if (!allowSrcAll)
		{
			callError = SCC_errorBlockedUser;
			rc = -GkCallDropLRJReason(NULL, callError);
			goto _return;
		}

		lrqHandle->vpnName = strdup("");
		lrqHandle->zone = strdup("");
		lrqHandle->cpname = strdup("");
	}
	else
	{
		strcpy(lrqHandle->phonode.regid, scacheInfoEntry.data.regid);
		lrqHandle->phonode.uport = scacheInfoEntry.data.uport;
		lrqHandle->vendor = scacheInfoEntry.data.vendor;

		if (scacheInfoEntry.data.custID[0] != '\0')
		{
			lrqHandle->custID = 
				CStrdup(callCache, scacheInfoEntry.data.custID);
		}

		lrqHandle->maxHunts = scacheInfoEntry.data.maxHunts;

		if (lrqHandle->maxHunts == 0)
		{
			lrqHandle->maxHunts = maxHunts;
		}
		else if (lrqHandle->maxHunts > SYSTEM_MAX_HUNTS)
		{
			lrqHandle->maxHunts = SYSTEM_MAX_HUNTS;
		}

		lrqHandle->ecaps1 = scacheInfoEntry.data.ecaps1;
		lrqHandle->vpnName = strdup(scacheInfoEntry.data.vpnName);
		lrqHandle->zone = strdup(scacheInfoEntry.data.zone);
		lrqHandle->cpname = strdup(scacheInfoEntry.data.cpname);
		if (BIT_TEST(scacheInfoEntry.data.sflags, ISSET_PHONE))
		{
			strcpy(lrqHandle->phonode.phone, scacheInfoEntry.data.phone);
			BIT_SET(lrqHandle->phonode.sflags, ISSET_PHONE);
		}
	}

	if(GwPortAvailCall(&lrqHandle->phonode, 1, 0)<0)
	{
		NETERROR(MSCC,
			("%s GwPortAvailCall Failed source= %s/%lu called No.= %s\n",
			fn,lrqHandle->phonode.regid, lrqHandle->phonode.uport,
			H323dialledNumber(lrqHandle)));

		callError = SCC_errorInadequateBandwidth;
		rc = -GkCallDropLRJReason(NULL, callError);
		goto _return;
	}

	if(!allowHairPin)
	{
		// Hairpin is NOT allowed, add src into reject list.
		// If src is not found, add the src ip addr
		GwAddPhoNodeToRejectList(&lrqHandle->phonode, NULL,
			&lrqHandle->destRejectList, CMalloc(callCache));
	}

	cmMeiEnter(UH323Globals()->hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	if (pvtGetByPath(hVal, nodeId,
		"request.locationRequest.canMapAlias", NULL, 
			&canMapAlias, &isstring) < 0)
	{
		NETERROR(MH323, ("%s could not build MapAlias\n", fn));
	}

	cmMeiExit(UH323Globals()->hApp);

	 /* Setup Resolve Handle */

	 /* Initialize the rhandle */
	 rhandle 			= GisAllocRHandle();
	 rhandle->phonodep 	= &lrqHandle->phonode;
	 rhandle->rfphonodep = &lrqHandle->rfphonode;

	 rhandle->crname = lrqHandle->crname;

	 /* NO POLICY SUPPORT !!! */
	 rhandle->checkZone = 0;
	 rhandle->checkVpnGroup = 0;
	 rhandle->reservePort = 1;
	 // Do a find remote 
	 rhandle->resolveRemote = 1;
	rhandle->scpname = lrqHandle->cpname;
	FindIedgeVpn(lrqHandle->vpnName, &rhandle->sVpn);
	nx_strlcpy(rhandle->sZone, lrqHandle->zone, ZONE_LEN);

	 rhandle->phoneChange = canMapAlias &&
				scacheInfoEntry.data.ecaps1 & ECAPS1_MAPALIAS;

	 NETDEBUG(MLRQ, NETLOG_DEBUG4,
		("%s phone change = %d\n", fn, rhandle->phoneChange));

	 /* Call the main function */
	 ResolvePhone(rhandle);

	if (BIT_TEST(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE))
	{
		strcpy(H323dialledNumber(lrqHandle),
			rhandle->fCacheInfoEntry.data.phone);
	}

	 switch (rhandle->result)
	 {
	 case CACHE_FOUND:
	   /* Fill up the response */
	   NETDEBUG(MLRQ, NETLOG_DEBUG4,
				("%s CACHE_FOUND\n", fn));
		GwPortFreeCall(rhandle->rfphonodep, 1, 1);
	   rc = 1;
	   break;
	 case CACHE_NOTFOUND:
	   NETDEBUG(MLRQ, NETLOG_DEBUG4,
				("%s CACHE_NOTFOUND\n", fn));

		callError = rhandle->callError;

		// We must switch the SCC Error to the right LRJ error
		rc = -GkCallDropLRJReason(NULL, callError);
	   goto _return;

	   break;
	 default:
	   break;
	 }

 _reply:


	if (routecall == 0)
	{
		dstSignalAddr.ip = htonl(RH_FOUND_NODE(rhandle, 1)->ipaddress.l);
		dstSignalAddr.port = rhandle->rfcallsigport;
	}
	else
	{
		// check vports first.
		// If vports are available and max calls seems less, its an error.
		if (nlm_getvport() <0)
		{
			callError = SCC_errorNoVports;
	  		rc = -GkCallDropLRJReason(NULL, callError);
	  		goto _return;
		}

		if (UH323DetermineBestSigAddr(&dstSignalAddr) < 0 )
		{
			NETERROR(MARQ, ("%s UH323DetermineBestSigAddr error\n", fn));
			
			callError = SCC_errorH323MaxCalls;
	  		rc = -GkCallDropLRJReason(NULL, callError);
	  		goto _return;
		}

		dstSignalAddr.ip = htonl(domainIp);
	}

	 /* Set the Call Signal Address */
	 if (cmRASSetParam(hsRas,
					   cmRASTrStageConfirm,
					   cmRASParamCallSignalAddress, 0, 
					   addrlen, (char *)&dstSignalAddr) < 0)
	 {
		  NETERROR(MLRQ,
				   ("%s cmRASSetParam cmRASParamCallSignalAddress failed\n", 
					fn));
	 }

	/* Set the RAS Endpoint address */
	dstRASAddr.ip = dstSignalAddr.ip;
	dstRASAddr.port = 1719;

	if (cmRASSetParam(hsRas,
					   cmRASTrStageConfirm,
					   cmRASParamRASAddress, 0, 
					   addrlen, (char *)&dstRASAddr) < 0)
	{
		  NETERROR(MLRQ,
				   ("%s cmRASSetParam cmRASParamRASAddress failed\n", 
					fn));
	}

	if (rhandle->phoneChange)
	{
		alias.type = cmAliasTypeE164;
		alias.string = rhandle->rfphonodep->phone;
		alias.length = strlen(alias.string);
	
		NETDEBUG(MLRQ, NETLOG_DEBUG1,
			("new mapped Alias - %s\n", alias.string));

		if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
				cmRASParamDestInfo, 0, sizeof(cmAlias), 
				(char *)&alias) < 0)
		{
			NETERROR(MLRQ, ("cmRASSetParam cmRASParamDestInfo failed\n"));
		}
	}

 _return:

	/* Free the remote handle here */
	GisFreeRHandle(rhandle);

	if (lrqHandle && (rc < 0)) 
	{
		// No need to acquire locks
		// We must log the dropped call
		timedef_cur(&lrqHandle->callEndTime);
		lrqHandle->lastEvent = SCC_evt323LRJTx;
		lrqHandle->callError = callError;
		lrqHandle->rasReason = -rc + 1 ;
		BillCall(lrqHandle, CDR_CALLDROPPED);	
	}

	if (lrqHandle)
	{
		// Free the call handle
		GisFreeCallHandle(lrqHandle);
	}

	return rc;
}

int
GkSendLRQ(PhoNode *rfphonodep, ARQHandle *arqHandle2, void *data)
{
	char fn[] = "GkSendLRQ():";
	cmTransportAddress 
		  gkAddr = { 0, 0, 1719, cmRASTransportTypeIP },
			addr = { 0, 0, 1719, cmRASTransportTypeIP };
	int i;
	HRAS hsRas = NULL;
	BYTE h323_id[256];
	BYTE gk_id[128];
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;	
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;	
	cmAlias alias, endpointID;
	int index = 0;
	char *callID = (char *)malloc(CALL_ID_LEN);
	char callIdStr[CALL_ID_LEN];
	int nodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	INT32 tlen;
	int sd,domainIp;
	unsigned short domainPort;

	/* We need the endpoint id, assigned by this gatekeeper
	 * to us. For that, we will query the entry from the cache
	 */
	cacheInfo = &cacheInfoEntry;
	if (CacheFind(regCache, rfphonodep, cacheInfo,
		sizeof(CacheTableInfo)) < 0)
	{
		NETERROR(MLRQ, ("%s Could not locate cache entry for gk\n", fn));

		return -1;
	}

	/* Open a RAS Transaction */
	gkAddr.ip = htonl(rfphonodep->ipaddress.l);

	NETDEBUG(MLRQ, NETLOG_DEBUG4,
		   ("%s Sending LRQ to %s/%d for:\n", 
			fn, ULIPtostring(ntohl(gkAddr.ip)), gkAddr.port));

	memcpy(callID, arqHandle2->callID, CALL_ID_LEN);

	if (cmRASStartTransaction(UH323Globals()->hApp,
		   (HAPPRAS)data, /* Application Ras Handle */
		   &hsRas,
		   cmRASLocation,
		   &gkAddr,
		   NULL) < 0)
	{
		NETERROR(MLRQ, 
		   ("%s cmRASStartTransaction failed\n", fn));
		goto _error;
	}	

	alias.type = cmAliasTypeE164;
	alias.string = rfphonodep->phone;
	alias.length = strlen(alias.string);
	
	DEBUG(MH323, NETLOG_DEBUG1,
		("dst Alias - %s\n", alias.string));

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamDestInfo, 0, sizeof(cmAlias), (char *)&alias) < 0)
	{
		NETERROR(MLRQ, ("cmRASSetParam cmRASParamDestInfo failed\n"));
	}

	alias.type = cmAliasTypeE164;
	alias.string = cacheInfo->data.phone;
	alias.length = strlen(alias.string);
	
	uh323AddRASAlias(hsRas, &alias, &index, 
				 cmRASTrStageRequest, cmRASParamSrcInfo);

	alias.type = cmAliasTypeH323ID;
	alias.length = utlChr2Bmp(cacheInfo->data.h323id, &h323_id[0]);
	alias.string = &h323_id[0];
	
	DEBUG(MH323, NETLOG_DEBUG1,
		("my Alias - %s\n", cacheInfo->data.h323id));

	if (alias.length > 0)
	{
        if (arqHandle2->callSource == 1)
        {
            nx_strlcpy(H323h323Id(arqHandle2), cacheInfo->data.h323id, H323ID_LEN);
            // Save h323id in conf handle so it is sent in
            // setups to alternate endpoints
            saveEgressH323Id(arqHandle2->confID, cacheInfo->data.h323id);
        }
	}

	uh323AddRASAlias(hsRas, &alias, &index, 
				 cmRASTrStageRequest, cmRASParamSrcInfo);

	index = 0;
	if (strlen(cacheInfo->data.pgkid))
	{
		alias.type = cmAliasTypeGatekeeperID;
		alias.length = utlChr2Bmp(cacheInfo->data.pgkid,
							&gk_id[0]);
		alias.string = (char *)&gk_id[0];

		NETDEBUG(MLRQ, NETLOG_DEBUG4,
			("%s Using GKID %s\n", fn, cacheInfo->data.pgkid));

		uh323AddRASAlias(hsRas, &alias, &index, cmRASTrStageRequest,
				cmRASParamGatekeeperID);
	}

	cmMeiEnter(UH323Globals()->hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);
	if (pvtBuildByPath(hVal, nodeId,
		"request.locationRequest.canMapAlias", 1, NULL) < 0)
	{
		NETERROR(MH323, ("%s could not build MapAlias\n", fn));
	}

	if (pvtBuildByPath(hVal, nodeId,
		"request.locationRequest.hopCount", 6, NULL) < 0)
	{
		NETERROR(MH323, ("%s could not build HopCount\n", fn));
	}

	if (arqHandle2->vendor == Vendor_eCisco)
	// Add Cisco specific data
	{
		cmNonStandardIdentifier nsid;
		struct LRQnonStandardInfo { int ttl; } lrqnsinfo;
		cmNonStandardParam nsparam;
		
		// setup the h221 info for cisco
		nsid.objectLength = 0;
		nsid.t35CountryCode = 181;
		nsid.t35Extension = 0;
		nsid.manufacturerCode = 18;	// cisco

		lrqnsinfo.ttl = 6;	

		nsparam.info = nsid;	
		nsparam.length = sizeof(lrqnsinfo);
		nsparam.data = (char *)&lrqnsinfo;

		tlen = sizeof(nsparam);
		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamNonStandard, 0, tlen,
			(char*)&nsparam) < 0)
		{
			NETERROR(MH323,
				("%s Could not set non standard parameter\n", fn));
		}
	}

	if (arqHandle2->tg && arqHandle2->tg[0])
	{
    	if (pvtBuildByPath(hVal, nodeId,
       		"request.locationRequest.circuitInfo.sourceCircuitID.group.group",
       		strlen(arqHandle2->tg), arqHandle2->tg) < 0)
    	{
     		NETERROR(MARQ, ("%s could not build trunk group\n", fn));
    	}
	}

	if (arqHandle2->destTg && arqHandle2->destTg[0] && (arqHandle2->ecaps1 & ECAPS1_SETDESTTG))
	{
    	if (pvtBuildByPath(hVal, nodeId,
       		"request.locationRequest.circuitInfo.destinationCircuitID.group.group",
       		strlen(arqHandle2->destTg), arqHandle2->destTg) < 0)
    	{
     		NETERROR(MARQ, ("%s could not build dest trunk group\n", fn));
    	}
	}

	cmMeiExit(UH323Globals()->hApp);

	/* Add our ras address */
	addr = UH323Globals()->rasAddr;
	tlen = sizeof(cmTransportAddress);
	if(arqHandle2->realmInfo)
	{
		if(getRasInfoFromRealmId(arqHandle2->realmInfo->realmId,Ras_eArq, &sd,
			&domainIp,&domainPort) >=0)
		{
		    if (cmRASSetParam(hsRas, cmRASTrStageRequest,
                      cmRASParamSocketDesc, 0,
                      sd, NULL) < 0)
			{
				NETERROR(MLRQ,("%s Could not set SocketDesc %d hsRas=%p\n",
					fn,sd,hsRas));
			}
		}
		addr.ip = htonl(domainIp);
		addr.port = domainPort;
	}
	else {
		  NETERROR(MLRQ,("%s no realm info present hsRas=%p %s\n", 
				fn,hsRas, (char*) CallID2String(callID, callIdStr)));
	}

	
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamReplyAddress, 0, tlen,
		(char*)&addr) < 0)
	{
		NETERROR(MH323,
			("%s Could not set Signalling Address \n", fn));
	}

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MLRQ, ("%s cmRASRequest failed for admission\n", fn));
		goto _error;
	}

	NETDEBUG(MLRQ, NETLOG_DEBUG1,
		("%s Started the RAS Transaction successfully, hsRas = %p haRas = %p callId = %s\n", 
		fn, hsRas, callID, (char*) CallID2String(callID, callIdStr)));

	free(callID);
	return 0;

_error:
	if (hsRas) 
	{
		cmRASClose(hsRas);
	}

	free(callID);
	return -1;
}
