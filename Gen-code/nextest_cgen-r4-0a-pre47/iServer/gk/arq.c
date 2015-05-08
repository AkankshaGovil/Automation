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
#include "callsm.h"
#include "h323realm.h"
#include "nxosd.h"
#include <malloc.h>
#include "gk.h"

#include "log.h"
#include "stkutils.h"
#include "bridge.h"
#include "callid.h"
#include "ls.h"
#include "ipstring.h"

extern HPST	hPstSetup; 
int GkSendIRQ(struct Timer*);
void GkGetAlternateEP( HRAS hsRas, char *responseName, char *calledpn, CallHandle *callHandle );

/*
 * Start ARQ processing.
 */

int
GkHandleARQ(
		IN	HRAS				hsRas,
		IN	HCALL				hsCall,
		OUT	LPHAPPRAS			lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL			haCall)
{
	char fn[] = "GkHandleARQ():";
	int createCallHandle = 0, sd, realmId;
	int rc = -cmRASReasonRequestDenied, index = 0;
	int callError = SCC_errorNone;
	INT32 tlen;
	int answerCall, newCallHandle = 1, handleInserted = 0;
	ARQHandle *arqHandle = NULL;
	ResolveHandle *rhandle = NULL;
	cmTransportAddress dstSignalAddr = { 0, 0, 1720, cmRASTransportTypeIP };
	int domainIp;
	unsigned short domainPort;
	int addrlen = sizeof(cmTransportAddress);
	CacheTableInfo scacheInfoEntry = { 0 };
	char *rPhone, guessPhone[PHONE_NUM_LEN] = { 0 };
	cmAlias alias;
	int bandwidth = 0;
	char callIDStr[32];
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	int routeDirect = 0, trackLicense = 0, trackGw = 0, trackSrcGw = 0;
	int findMSW = 0;
	time_t now;
	BOOL canMapAlias, isstring;
	int nodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	char guid[GUID_LEN];

	NETDEBUG(MARQ, NETLOG_DEBUG1,
		 ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));

	if (GkCompareGkID(hsRas) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG4, ("%s GKID mismatch\n", fn));

		//rc = -cmRASReasonInvalidPermission;
		if (hsCall) cmCallDrop(hsCall);
		cmRASClose(hsRas);
		return 0;
	}

	// appHandle MUST be there, as it was exchanged when newCall
	// happened
	if (!appHandle)
	{
		NETERROR(MARQ,
			("%s ARQ, w/o any app handle\n", fn));

		// should we set up the call signaling addresses here
		if (hsCall) cmCallDrop(hsCall);
		return -cmRASReasonRouteCallToGatekeeper;
	}

	/* Extract the Answer Call parameter */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 cmRASParamAnswerCall, 0, 
					 &answerCall, NULL) < 0)
	{
		NETERROR(MARQ, ("%s No AnswerCall field found\n", fn));
		answerCall = FALSE;
	}

	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					  cmRASParamSocketDesc, 0, 
					  &sd, NULL) < 0)
	{
		NETDEBUG(MRRQ, NETLOG_DEBUG4,
			("%s SocketDesc not found\n", fn));
		if (hsCall) cmCallDrop(hsCall);
		cmRASClose(hsRas);
		return 0;
	}

	if (answerCall != TRUE)
	{
		time(&now);
		UH323UpdateStats(&UH323Globals()->arqLast, &now, 
			&UH323Globals()->narqs);
		if (UH323Globals()->narqs > h323Cps)
		{
			findMSW = 1;
#if 0
			// delay this
			if (hsCall) cmCallDrop(hsCall);
			return -cmRASResourceUnavailable;
#endif
		}
	}

	/* Extract the Call Id */
	if (answerCall == FALSE)
	{
		// SRC
		tlen = CALL_ID_LEN;
		if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 cmRASParamCallID, 0, &tlen,
					 (char *)appHandle->callID) < 0)
		{
			NETERROR(MARQ,
				 ("%s Could not get call id\n", fn));

			// SRC: we can generate a new one
			generateCallId(appHandle->callID);
		}
		else if (CacheFind(callCache, appHandle->callID, NULL, 0) >= 0)
		{
			newCallHandle = 0;
		}
	}
	else
	{
		// DEST

		if (CacheFind(callCache, appHandle->callID, NULL, 0) >= 0)
		{
			newCallHandle = 0;
		}
		else
		{
			tlen = GUID_LEN;
			if ((cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 cmRASParamCallID, 0, &tlen,
					 (char *)guid) < 0)
					   && !allowDestArq)
			{
				NETERROR(MARQ,
					 ("%s Could not get call id\n", fn));

				// Dest: we can never generate, 
				// get one in the app

				// No callid, there is nothing to do for
				// this scenario, so reject it.
				callError = SCC_errorNoCallHandle;
				rc = -cmRASReasonRouteCallToGatekeeper;
				goto _return;
			}

			if (CacheFind(guidCache,guid, NULL, 0) >= 0)
			{
				newCallHandle = 0;
			}
		}
	}


	// Drop the call in all cases EXCEPT when answerCall==TRUE
	// and its not a new call handle & when anserCall == TRUE 
	// no new call handle and allowDestArq  is false.
	
	if ((answerCall != TRUE) || newCallHandle 
					|| (nh323Instances > 1))
	{
		cmCallDrop(hsCall); hsCall = 0; 
		
		// appHandle is unusable at this point
		appHandle = NULL;
	}

	if(getQ931RealmInfo(sd,&domainIp,&domainPort,&realmId) <0)
	{
		NETERROR(MRRQ, ("%s getQ931RealmInfo failed\n", fn));
		if (hsCall) cmCallDrop(hsCall);
		cmRASClose(hsRas);
		return 0;
	}
	domainIp = htonl(domainIp);

	/* We MUST return here, the rest of the processing
	 * is source based
	 */
	if (answerCall == TRUE)
	{
		/* Its the destination ARQ, and the call id matches with what
		 * we have
		 */
		if (newCallHandle && !allowDestArq)
		{
			NETERROR(MARQ,
				 ("%s ARQ by call dest, no call handle\n", fn));

			// We should send an error here, however if we do so,
			// the gateway is going to send release complete back to us,
			// and the ultimately the fault becomes iServers'.
			// To get around this, we should send a confirm
			// and let the gateway handle the release complete from the
			// source, because it was late. We will send an error
			// if we are not routing the call

			callError = SCC_errorNoCallHandle;
			rc = -cmRASReasonRouteCallToGatekeeper;
			goto _return;
		}
		else
		{
			rc = 1;
		}

		tlen = sizeof(cmTransportAddress);

		/* Extract the IP address */
		/* Set the Dest Call Signal Address */
		if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 cmRASParamSrcCallSignalAddress, 0, 
					 &tlen, (char *)&dstSignalAddr) < 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s No Signalling Address found\n", fn));

			// not found, we must put in our ip address in this case
			// Just use default instance
			dstSignalAddr = uh323Globals[0].sigAddr;
			dstSignalAddr.ip = domainIp;
		}

		if ( (cmRASSetParam(hsRas,
					 cmRASTrStageConfirm,
					 cmRASParamDestCallSignalAddress, 0, 
					 tlen, (char *)&dstSignalAddr) < 0))
		{
			NETERROR(MARQ,
				 ("%s cmRASSetParam cmRASParamDestCallSignalAddress failed\n", fn));
		}

		goto _return;
	}
	else if (newCallHandle == 0)
	{
		// Initial ARQ w/ already existing call handle
		NETERROR(MARQ,
		("%s ARQ by src, handle exists, call rejected. NO CDR.\n", fn));

		callError = SCC_errorUndefinedReason;
		rc = -cmRASReasonUndefined;
		goto _return;
	}

	arqHandle = GisAllocCallHandle() /* XXX */;

	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
		 cmRASParamCallID, 0, &tlen,
		 (char *)arqHandle->callID) < 0)
	{
		// SRC: we can generate a new one
		generateCallId(arqHandle->callID);

		NETERROR(MARQ,
			 ("%s Could not get call id, generating %s\n", 
			fn, (char*) CallID2String(arqHandle->callID, callIDStr)));
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		("%s New Call with Call Id:%s\n",
		fn, (char*) CallID2String(arqHandle->callID, callIDStr)));

	arqHandle->handleType = SCC_eH323CallHandle;
 	H323controlState(arqHandle) = UH323_sControlIdle;
	arqHandle->state = SCC_sIdle;
	timedef_cur(&arqHandle->callStartTime);
	arqHandle->callSource = 0;
	arqHandle->lastEvent = SCC_evt323ARQRx;

	H323waitForDRQ(arqHandle) = 1;

	
	// Do policy checks, fill up src/dest aliases
	if (GkAdmitCall(hsRas, hsCall, arqHandle, newCallHandle, 
			answerCall, &scacheInfoEntry) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			 ("%s GkAdmitCall failed. Rejecting Call\n", fn));

		callError = SCC_errorBlockedUser;
		rc = -GkCallDropARJReason(NULL, callError);

		// The original ANI is stored here
		if (strlen(arqHandle->phonode.phone))
		{
			strcpy(CallInputANI(arqHandle), arqHandle->phonode.phone);
		}
		goto _return;
	}

	// The original input number is stored here
	strcpy(CallInputNumber(arqHandle), arqHandle->rfphonode.phone);

	// The original ANI is stored here
	strcpy(CallInputANI(arqHandle), arqHandle->phonode.phone);

	// Logic for maintaining state:
	// If dest is MSW, do not maintain state. (stateless)
	// If src is MSW, allocate call handle, but no license (little state)
	// In all cases, if dest does not allow direct calls,
	// abort stateless mode

	cmMeiEnter(UH323Globals()->hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	if (pvtGetByPath(hVal, nodeId,
		"request.admissionRequest.canMapAlias", NULL, 
			&canMapAlias, &isstring) < 0)
	{
		NETERROR(MH323, ("%s could not build MapAlias\n", fn));
	}

	cmMeiExit(UH323Globals()->hApp);

	/* Setup Resolve Handle */

	/* Initialize the rhandle */
	rhandle 			= GisAllocRHandle();
	rhandle->phonodep 	= &arqHandle->phonode;
	rhandle->rfphonodep = &arqHandle->rfphonode;
	rhandle->crname 	= arqHandle->crname;

	/* Determine the vpn group of this entry */
	FindIedgeVpn(arqHandle->vpnName, &rhandle->sVpn);

	/* Zones are valid only for LUS */
	nx_strlcpy(rhandle->sZone, arqHandle->zone, ZONE_LEN);

	rhandle->checkZone = 1;
	rhandle->checkVpnGroup = 1;
	rhandle->reservePort = 1;
	rhandle->resolveRemote = 0;
	rhandle->findMSW = findMSW;
	rhandle->phoneChange = canMapAlias && 
			scacheInfoEntry.data.ecaps1 & ECAPS1_MAPALIAS;
	rhandle->scpname = arqHandle->cpname;
	rhandle->dtg = arqHandle->destTg;

	// Do a find remote only if we are not routing the call
	rhandle->resolveRemote = (routecall)?0:1;

	rhandle->primary = 0;
	ResolvePhone(rhandle);

	if (BIT_TEST(rhandle->fCacheInfoEntry.data.sflags, ISSET_PHONE))
	{
		strcpy(H323dialledNumber(arqHandle),
			rhandle->fCacheInfoEntry.data.phone);
	}

	switch (rhandle->result)
	{
	case CACHE_FOUND:

	  	rc = 1;
		NETDEBUG(MARQ, NETLOG_DEBUG4, ("%s CACHE_FOUND\n", fn));

		break;

	case CACHE_NOTFOUND:
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s CACHE_NOTFOUND\n", fn));

		callError = rhandle->callError;

		// We must switch the SCC Error to the right ARJ error
		rc = -GkCallDropARJReason(NULL, callError);
		goto _return;
		break;
	default:
		break;
	}

	// routeDirect - dest is MSW or src is MSW or routecall is clear
	// trackLicense - routeDirect and src is not MSW and dest is not MSW
	// trackGw - routeDirect and dst is not MSW
	// trackSrcGw - routeDirect and src is not MSW

	routeDirect = 
		newCallHandle &&
		!(rhandle->rfCacheInfoEntry.data.ecaps1 & ECAPS1_NODIRECTCALLS) &&
			(!routecall || 
			BIT_TEST(arqHandle->phonode.cap, CAP_MSW) ||
			BIT_TEST(rhandle->rfCacheInfoEntry.data.cap, CAP_MSW));

	trackLicense = trackVports && routeDirect &&
			!BIT_TEST(arqHandle->phonode.cap, CAP_MSW) &&
			!BIT_TEST(rhandle->rfCacheInfoEntry.data.cap, CAP_MSW);

	trackGw = trackMaxCalls && routeDirect &&
			!BIT_TEST(rhandle->rfCacheInfoEntry.data.cap, CAP_MSW);

	//trackSrcGw = routeDirect &&
	//		!BIT_TEST(arqHandle->phonode.cap, CAP_MSW);

	trackSrcGw = trackMaxCalls && routeDirect;

	if (!trackGw)
	{
		// release the port which we have allocated
		GwPortFreeCall(&arqHandle->rfphonode, 1, 1);
		arqHandle->flags |= FL_CALL_NODSTPORTALLOC;
	}

	if(!routecall && !BIT_TEST(rhandle->rfphonodep->cap, CAP_H323) )
	{
		NETERROR(MARQ,("%s Cannot do IWF if route call disabled\n",fn));
		callError = SCC_errorNoRoute;
	  	rc = -GkCallDropARJReason(NULL, callError);
		goto _return;
	}

	if (routeDirect)
	{
		arqHandle->flags |= FL_CALL_NOSTATE;
	}

	if (trackLicense)
	{
		
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Using license...\n", fn));

		if(GisAddCallToConf(arqHandle) != 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s GisAddCallToConf failed. Rejecting Call\n", fn));

			callError = SCC_errorNoVports;
  			rc = -GkCallDropARJReason(NULL, callError);
			goto _return;	
		}
	}

	if (trackSrcGw)
	{
		if(GwPortAllocCall(&arqHandle->phonode, 1, 0)<0)
		{
			NETERROR(MSCC,
				("%s GwPortAllocCall Failed source= %s/%lu called No.= %s\n",
				fn,arqHandle->phonode.regid, arqHandle->phonode.uport,
				H323dialledNumber(arqHandle)));

			callError = SCC_errorInadequateBandwidth;
			rc = -GkCallDropARJReason(NULL, callError);
			goto _return;
		}
	}
	else
	{
		arqHandle->flags |= FL_CALL_NOSRCPORTALLOC;
	}

	/* The rhandle, now has both the srcCache entry as well
	 * as destination cache entry
	 */
	
	/* Set Direct Call parameter */
	if (cmRASSetParam(hsRas,
			 cmRASTrStageConfirm,
			 cmRASParamCallModel, 0, 
			 (routeDirect)?cmCallModelTypeDirect:cmCallModelTypeGKRouted,
			 NULL) < 0)
	{
		NETERROR(MARQ,
		("%s cmRASSetParam cmRASParamCallModelType failed\n", fn));
	}

	memset(&dstSignalAddr, 0, sizeof(cmTransportAddress));
	dstSignalAddr.port = 1720;
	dstSignalAddr.type = cmRASTransportTypeIP;
	addrlen = sizeof(cmTransportAddress);
	
	if (routeDirect)
	{
		H323callModel(arqHandle) = cmCallModelTypeDirect;

		dstSignalAddr.ip = htonl(RH_FOUND_NODE(rhandle, 1)->ipaddress.l);
		dstSignalAddr.port = rhandle->rfcallsigport;
	}
	else 
	{
		H323callModel(arqHandle) = cmCallModelTypeGKRouted;
		
		// check vports first.
		// If vports are available and max calls seems less, its an error.
		if (nlm_getvport() <0)
		{
			callError = SCC_errorNoVports;
	  		rc = -GkCallDropARJReason(NULL, callError);
	  		goto _return;
		}

		if (UH323DetermineBestSigAddr(&dstSignalAddr) < 0 )
		{
			NETERROR(MARQ, ("%s UH323DetermineBestSigAddr error\n", fn));
			
			callError = SCC_errorH323MaxCalls;
	  		rc = -GkCallDropARJReason(NULL, callError);
	  		goto _return;
		}
		
		dstSignalAddr.ip = domainIp;
	}

	H323rfcallsigport(arqHandle) = rhandle->rfcallsigport;

	/* Set the Dest Call Signal Address */
	if (cmRASSetParam(hsRas,
			 cmRASTrStageConfirm,
			 cmRASParamDestCallSignalAddress, 0, 
			 addrlen, (char *)&dstSignalAddr) < 0)
	{
		NETERROR(MARQ,
		("%s cmRASSetParam cmRASParamDestCallSignalAddress failed\n", 
		fn));
	}

	/* Set the destination call info field */
	alias.type = cmAliasTypeE164;
	alias.string = CallInputNumber(arqHandle);
	alias.length = strlen(alias.string);
	uh323AddRASAlias(hsRas, &alias, &index, cmRASTrStageConfirm,
					cmRASParamDestInfo);

	DEBUG(MARQ, NETLOG_DEBUG4,
		("%s Replying with destination address %s/%d\n",
		fn, (char*) ULIPtostring(ntohl(dstSignalAddr.ip)), 
		dstSignalAddr.port));

	// CDR stuff
	if (routeDirect)
	{
		// We must log a call start cdr
		BillCall(arqHandle, CDR_CALLSETUP);	
	}

	if (trackSrcGw || trackGw)
	{
		CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

		// soft state
		if (CacheInsert(callCache, arqHandle) < 0)
		{
			NETERROR(MARQ, 
			("%s could not insert call handle into cache\n",fn));
		}
		else
		{
			handleInserted = 1;
		}

		CacheReleaseLocks(callCache);
	}

 _return:

	/* Extract the bandwith in the ARQ and set it back in the response */
	if (rc > 0)
	{
		if (cmRASGetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamBandwidth, 0,
			&bandwidth, NULL) >= 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s found bandwith of %d\n",
				fn, bandwidth));

			if (cmRASSetParam(hsRas,
					cmRASTrStageConfirm,
					cmRASParamBandwidth, 0,
					bandwidth, 0) < 0)
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Could not set bandwith to %d\n",
					fn, bandwidth));
			}
		}		
	}

	/* Free the remote handle here */
	GisFreeRHandle(rhandle);

	if (arqHandle && (rc < 0) && (answerCall != TRUE))
	{
		// No need to acquire locks
		// We must log the dropped call
		timedef_cur(&arqHandle->callEndTime);
		arqHandle->lastEvent = SCC_evt323ARJTx;
		arqHandle->callError = callError;
		arqHandle->rasReason = -rc + 1 ;
		BillCall(arqHandle, CDR_CALLDROPPED);	
	}

	if (!handleInserted && arqHandle)
	{
		// Free the call handle
		GisFreeCallHandle(arqHandle);
	}

	return rc;
}

/* return 1 if admission was successful,
 * -1 if failed
 */
/*
 * GkAdmitCall
 *
 * Fill up source and destination aliases
 */
int
GkAdmitCall(
	HRAS hsRas, 
	HCALL hsCall, 
	ARQHandle *arqHandle,
	int newCallHandle,
	int answerCall,
	CacheTableInfo *scacheInfo
)
{
	char fn[] = "GkAdmitCall():";
	int rc = -1;
	PhoNode *phonodep;
	char endptIdStr[128] = { 0 };
	BYTE endptId[128], string[80];
	cmAlias alias, number;
	INT32 tlen;
	cmTransportAddress addr;
	ARQHandle *call;
	int index = 0;
	int CRV;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	char tg[PHONE_NUM_LEN] = { 0 };
	int nodeId;
	int tglen;

 	call = arqHandle;

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		("%s AnswerCall = %s New Call = %s\n",
		fn,
		(answerCall == TRUE)?"YES":"NO", 
		(newCallHandle)?"YES":"NO"));

	if (answerCall == TRUE)
	{
		phonodep = &arqHandle->rfphonode;
	}
	else
	{
		phonodep = &arqHandle->phonode;
	}

	/* Extract the endpoint information */
	/* Set up alias */
	alias.string = (char *)&endptId[0];
	alias.length = 128;
	tlen = sizeof(cmAlias);

	/* Extract the endpoint Id */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
		 cmRASParamEndpointID, 0, &tlen,
		 (char *)&alias) < 0)
	{
		NETERROR(MARQ,
				 ("%s Could not get endpoint id\n", fn));
		rc = -1;
		goto _return;
	}

	/* Convert it back to a string */
	utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

	if (newCallHandle)
	{
		GisExtractRegIdInfo(endptIdStr, phonodep);
	}
	else if (GisVerifyRegIdInfo(endptIdStr, phonodep) != 0)
	{
		NETERROR(MARQ,
		 ("%s GisVerifyRegIdInfo failed for %s/%lu, epid %s\n", 
		fn, phonodep->regid, phonodep->uport, SVal(endptIdStr)));
		rc = -1;
		goto _return;
	}

	tlen = sizeof(cmTransportAddress);

	/* Extract the IP address */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 (answerCall==FALSE)?cmRASParamSrcCallSignalAddress:
					 cmRASParamDestCallSignalAddress, 0, 
					 &tlen, (char *)&addr) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s No Signalling Address found\n", fn));
	}
	else
	{
		if (newCallHandle)
		{
			phonodep->ipaddress.l = ntohl(addr.ip);
			BIT_SET(phonodep->sflags, ISSET_IPADDRESS);
		}
		else
		{
			if (BIT_TEST(phonodep->sflags, ISSET_IPADDRESS) &&
				(phonodep->ipaddress.l != ntohl(addr.ip)))
			{
				NETERROR(MARQ, 
					("%s IP Address check failed %lx %x\n",
					fn, 
					phonodep->ipaddress.l,
					ntohl(addr.ip)));
				rc = -1;
				goto _return;
			}
		}
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		("%s Received from endpoint %s/%s\n",
		fn, phonodep->regid, 
		(char*) ULIPtostring(phonodep->ipaddress.l)));

_admit_arq:
#if 0
	/* Now we have set up the src of the call, we can
	 * see if it is valid.
	 */
	/* 
	if (GkAdmitARQ(hsRas, hsCall, answerCall, newCallHandle, 
		arqHandle, scacheInfo) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
				 ("%s Src Admission failed\n", fn));
		rc = -1;
		goto _return;
	}
	*/
#endif

	cmMeiEnter(UH323Globals()->hApp);

	if ((nodeId = cmGetProperty((HPROTOCOL)hsRas)) > 0)
	{
		if ((nodeId = pvtGetNodeIdByPath(hVal, nodeId,
			"request.admissionRequest.circuitInfo.sourceCircuitID.group.group")) > 0)
		{
			if ((tglen = pvtGetString(hVal, nodeId, PHONE_NUM_LEN-1, tg)) > 0)
			{
				tg[tglen] = '\0';
				arqHandle->tg = CStrdup(callCache, tg);
			}
			else
			{
				tg[0] = '\0';
			}
		}
	}

	cmMeiExit(UH323Globals()->hApp);

	H323nphones(arqHandle) = 0;
	
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
			NETDEBUG(MARQ, NETLOG_DEBUG1, 
						("%s DestInfo contains %s\n",
						fn, number.string));

			nx_strlcpy(arqHandle->rfphonode.phone, number.string, 
						 PHONE_NUM_LEN);
			BIT_SET(arqHandle->rfphonode.sflags, ISSET_PHONE);
		}

		/* Extract the Destination Info */
		tlen = sizeof(cmAlias);
		number.length = 80;
		number.string = (char *)string;
	}

	if (GkFindSrcForArq(hsRas, hsCall, answerCall, newCallHandle, 
				arqHandle, scacheInfo) < 0)
	{
		NETERROR(MARQ, 
			("%s Src Cache not found\n", fn));

		if (!allowSrcAll)
		{
			rc = -1;
			goto _return;
		}

		arqHandle->vpnName = strdup("");
		arqHandle->zone = strdup("");
		arqHandle->cpname = strdup("");
	}
	else
	{
		if (crids && (scacheInfo->data.crId != iservercrId))
		{
			NETERROR(MARQ, 
				("%s crid of the endpoint (%d) does not match ours (%d)\n",
				fn, scacheInfo->data.crId, iservercrId));	

			rc = -1;
			goto _return;
		}

		arqHandle->maxHunts = scacheInfo->data.maxHunts;

		if (arqHandle->maxHunts == 0)
		{
			arqHandle->maxHunts = maxHunts;
		}
		else if (arqHandle->maxHunts > SYSTEM_MAX_HUNTS)
		{
			arqHandle->maxHunts = SYSTEM_MAX_HUNTS;
		}

		arqHandle->ecaps1 = scacheInfo->data.ecaps1;
		arqHandle->vpnName = strdup(scacheInfo->data.vpnName);
		arqHandle->zone = strdup(scacheInfo->data.zone);
		arqHandle->cpname = strdup(scacheInfo->data.cpname);
		if (BIT_TEST(scacheInfo->data.sflags, ISSET_PHONE))
		{
			strcpy(arqHandle->phonode.phone, scacheInfo->data.phone);
			BIT_SET(arqHandle->phonode.sflags, ISSET_PHONE);
		}

		strcpy(arqHandle->phonode.regid,scacheInfo->data.regid);
		arqHandle->phonode.uport = scacheInfo->data.uport;
		arqHandle->vendor = scacheInfo->data.vendor;

		if (scacheInfo->data.custID[0] != '\0')
		{
			arqHandle->custID = 
				CStrdup(callCache, scacheInfo->data.custID);
		}

		if (scacheInfo->data.srcEgressTG[0] != '\0')
		{
			/* now replace destTg with srcEgressTG */
			if ( arqHandle->destTg )
			{
				/* free it first */
				(CFree(callCache))(arqHandle->destTg);
			}
			arqHandle->destTg = 
				CStrdup(callCache, scacheInfo->data.srcEgressTG);
		}
	}

	if(!allowHairPin)
	{
		// Hairpin is NOT allowed, add src into reject list.
		// If src is not found, add the src ip addr
		GwAddPhoNodeToRejectList(&arqHandle->phonode, NULL,
			&arqHandle->destRejectList, CMalloc(callCache));
	}

	if (cmRASGetParam(hsRas,
		cmRASTrStageRequest, 
		cmRASParamCRV,
		0,
		&CRV,
		NULL) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG1,
			("cmCallGetParam cmParamCRV returned error\n"));
	}

	if (answerCall == TRUE)
	{
		H323rfrasip(call) = scacheInfo->data.rasip;
		H323rfrasport(call) = scacheInfo->data.rasport;
		H323outCRV(call) = CRV;
	}
	else
	{
		H323rasip(call)= scacheInfo->data.rasip;
		H323rasport(call) = scacheInfo->data.rasport;
		H323inCRV(call) = CRV;
	}

	rc = 1;

	if (answerCall == TRUE)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
				 ("%s Admission Request by destination\n", fn));

		goto _return;
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Admission Request by src\n", fn));

 _return:
	return rc;
}

/* If the source has sent an ARQ, we will do some special checks and
 * initialize the remote phone in the arq handle. Also return the
 * scacheInfo found in our database of configured entries.
 * return -1 if an error was encountered.
 * 1 if everything was a success
 */
int
GkAdmitARQ(
	IN	HRAS			hsRas,
	IN	HCALL			hsCall,
	int					answerCall,
	int					newCallHandle,
	ARQHandle 			*arqHandle,
	CacheTableInfo		*scacheInfo)
{
	char fn[] = "GkAdmitARQ():";
	CacheTableInfo *cacheInfo = 0, *ipCacheInfo;
	PhoNode *phonodep;
	char string[80], endptIdStr[128] = { 0 };
	cmAlias number, alias;
	cmTransportAddress addr;
	RealmIP realmip;
	int index = 0, ttl = -1, keepAlive = 0;
	INT32  tlen;
	BYTE endptId[128];
	int rc = -1;

	if (answerCall == TRUE)
	{
		phonodep = &arqHandle->rfphonode;
	}
	else
	{
		phonodep = &arqHandle->phonode;
	}

	realmip.ipaddress = phonodep->ipaddress.l;
	realmip.realmId = phonodep->realmId;
	ipCacheInfo = CacheGet(ipCache, &realmip);

	if (ipCacheInfo == NULL)
	{
		goto _release_locks;
	}

	/* Extract srcInfo, actually calling party numbers */
	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = string;

	while ( (cmRASGetParam(hsRas, cmRASTrStageRequest, 
							(answerCall==FALSE)?cmRASParamSrcInfo:
							cmRASParamDestInfo, index++, 
							&tlen, (char *)&number)) >= 0)
	{
		if (number.type != cmAliasTypeE164)
		{
			 goto _continue;
		}

		NETDEBUG(MARQ, NETLOG_DEBUG4, 
			("%s Found Alias %s in ARQ\n", fn, number.string));

		cacheInfo = (CacheTableInfo *)CacheGet(phoneCache, number.string);
		if (cacheInfo)
		{
			 /* If ipCacheInfo, was found, its reg-id should
				* match with this reg-id
				*/
			 if (ipCacheInfo)
			 {
					if (memcmp(&ipCacheInfo->data, &cacheInfo->data,
							 REG_ID_LEN))
					{
						/* Mismatch */
						goto _continue;
					}
					
					/* There is a match.
					* We will return this as the right entry.
					*/
					if (newCallHandle)
					{
						strcpy(phonodep->phone, number.string);
						BIT_SET(phonodep->sflags, ISSET_PHONE);
					}
					else if (answerCall == FALSE)
					{
						if (strncmp(phonodep->phone, number.string,
								PHONE_NUM_LEN))
						{
							NETDEBUG(MARQ, NETLOG_DEBUG4,
								("%s phone mismatch in call cache %s %s\n",
									fn, phonodep->phone, number.string));

							goto _continue;
						}
					}

					break;
			 }
		}
	_continue:
		tlen = sizeof(cmAlias);
		number.length = 80;
		number.string = string;
	}

	if (cacheInfo)
	{
		rc = 1;
		/* copy this into the srcCacheInfo */
		memcpy(scacheInfo, cacheInfo, sizeof(CacheTableInfo));
	}
	else
	{
		rc =1;
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s IP Address and alias mismatch\n", fn));
	}

_release_locks:

	return rc;
}

/*
 * Clean up our call and conference cache.
 * Call cache is cleared based on the CRV mentioned
 * in the call, and conference cache is cleared
 * based on the clearing of the call.
 */
int
GkHandleDRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	char fn[] = "GkHandleDRQ():";
	INT32 tlen;
	CallHandle *callHandle = NULL;
	char callID[CALL_ID_LEN] = { 0 };
	int rc = 1; 
	int CRV;
	PhoNode phonode;
	char endptIdStr[128] = { 0 };
	BYTE endptId[128];
	cmAlias alias;
	SCC_EventBlock	sccEvt = { 0 };
	char	callIDStr[CALL_ID_LEN] = {0};
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	cmRASDisengageReason disengageReason = cmRASDisengageReasonForcedDrop;

	NETDEBUG(MARQ, NETLOG_DEBUG1,
		 ("%s Entering, hsRas = %p, hsCall = %p, haCall = %p\n",
			fn, hsRas, hsCall, haCall));

	if (GkCompareGkID(hsRas) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG4, ("%s GKID mismatch\n", fn));

		//	rc = -cmRASReasonInvalidPermission;

		if (hsCall) cmCallDrop(hsCall);
		cmRASClose(hsRas);
		return 0;
	 }

	/* Extract the endpoint information */

	alias.string = (char *)&endptId[0];
	alias.length = 128;
	tlen = sizeof(cmAlias);

	/* Extract the endpoint Id */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
				 cmRASParamEndpointID, 0, &tlen,
				 (char *)&alias) < 0)
	{
		NETERROR(MARQ,
				 ("%s Could not get endpoint id\n", fn));
	}
	else
	{
		/* Convert it back to a string */
		utlBmp2Chr(&endptIdStr[0], alias.string, alias.length);

		GisExtractRegIdInfo(endptIdStr, &phonode);
	}

	/* Extract the reason */
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
				 cmRASParamDisengageReason, 0, 
				(int *)&disengageReason, (char *)NULL) < 0)
	{
		NETERROR(MARQ,
		("%s Could not get disengage reason\n", fn));
	}

	if (appHandle == NULL)
	{
		// This call has already been freed by us and we
		// have absolutely no association for it.
		cmCallDrop(hsCall);
		return rc;
	}

	/* Extract the Call Id */
	tlen = CALL_ID_LEN;
	if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
					 cmRASParamCallID, 0, &tlen,
					 (char *)&callID[0]) < 0)
	{
		NETERROR(MARQ,
				 ("%s Could not get call id\n", fn));
		cmCallDrop(hsCall);
		return rc;
	}

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = (CallHandle *)CacheGet(callCache, appHandle->callID);

	if (callHandle == NULL)
	{
		callHandle = (CallHandle *)CacheGet(callCache, callID);
	}

	if (callHandle == NULL)
	{
		NETDEBUG(MARQ,NETLOG_DEBUG4,
		("%s Could not find Call Id %s in cache\n",
		fn, (char*) CallID2String(appHandle->callID,callIDStr)));

		CacheReleaseLocks(callCache);
		cmCallDrop(hsCall);

		goto _return;
	}

	/* Beyond this point we will always send DCF */

	if ((callHandle->flags & FL_CALL_NOSTATE) && hsCall)
	{
		cmCallDrop(hsCall); hsCall = NULL;	
	}

	rc = 1;

	// MUST be done
	H323waitForDRQ(callHandle) = 0;

	// If the DRQ is from an sgatekeeper, we must initiate the
	// disconnect in the peer direction, and remove this
	// call from the conf.
	// If the DRQ is from an endpoint and we are routing the call,
	// we must wait for more info in the relcomp
	// if we are not routing the call, drop the call in stack.

	if (routecall == 0)
	{
		/* Make sure we drop the call */
		cmCallDrop(hsCall);
	}
	else if (disengageReason == cmRASDisengageReasonForcedDrop) 
	{
		NETDEBUG(MSCC, NETLOG_DEBUG1, 
		("%s force drop for leg %d, dropping call\n", 
		fn, callHandle->leg));
		
		callHandle->lastEvent = SCC_evt323DRQForceDropRx;

		cmCallDrop(hsCall);
	}

	CacheReleaseLocks(callCache);

 _return:
	return rc;
}

/*
 * An ARQ will always be sent as part of leg 2 call setup.
 * It will lead the creation of a call handle; may require
 * a conf handle which is already existing as part of the leg1/2
 * call. The client handle must be allocated (alongwith ARQ handle),
 * We must obtain the Conference handle, and set it. The call handle,
 * must be generated and added to the call cache. All this must be done
 * outside the SendARQ function, in the Prepare ARQ function.
 */
int
ARQSetParamsFromHandle(ARQHandle *arqHandle, HRAS hsRas)
{
	char fn[] = "ARQSetParamsFromHandle():";
	BOOL origin = FALSE;
	char CallID[80], CID[80];
	INT32 CallIDLen, CIDLen;
	INT32	crv;
	int tlen;

	DEBUG(MARQ, NETLOG_DEBUG1, ("%s ARQ for originating call\n", fn));

	if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamAnswerCall, 0, arqHandle->callSource?FALSE:TRUE, NULL) < 0)
	{
		NETERROR(MARQ, ("cmRASSetParam cmRASParamAnswerCall failed\n"));
	}

	DEBUG(MARQ, NETLOG_DEBUG1, ("%s CRV %d\n", fn, H323outCRV(arqHandle)));

	if (cmCallGetParam(H323hsCall(arqHandle),
		cmParamCRV,
		0,
		&H323outCRV(arqHandle),
		NULL) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG1,
			("cmCallGetParam cmParamCRV returned error\n"));
	}

	if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamCRV,
			0,
			H323outCRV(arqHandle),
			NULL) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG1,
			("cmRASSetParam cmRASParamCRV returned error\n"));
	}

    if (cmCallGetParam(H323hsCall(arqHandle),cmParamCallID,0,
		&tlen,(char *)CallID)<0) 
    {
		NETDEBUG(MARQ, NETLOG_DEBUG2, ("%s Could not get CallID\n", fn));
		return -1;
    }

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallID,
		0,
		CALL_ID_LEN,
		CallID) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG1,
			("cmCallSetParam CallId returned error\n"));
	}
	
    if (cmCallGetParam(H323hsCall(arqHandle),cmParamCID,0,
		&tlen,(char *)CID)<0) 
    {
		NETDEBUG(MARQ, NETLOG_DEBUG2, ("%s Could not get CID\n", fn));
		return -1;
    }

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCID,
		0,
		CONF_ID_LEN,
		CID) < 0)
	{
		DEBUG(MARQ, NETLOG_DEBUG1,
			("cmCallgetParam cmParamCID returned error\n"));
	}

	return origin;
}

int
GkSendARQ(PhoNode *rfphonodep, ARQHandle *arqHandle2, void *data, char *id)
{
	char fn[] = "GkSendARQ():";
	cmTransportAddress 
		  gkAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	int i;
	HRAS hsRas = NULL;
	BYTE h323_id[256];
	BYTE gk_id[128];
	BYTE endptId[128];
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;	
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;	
	cmAlias alias, endpointID;
	int index = 0;
	char callIdStr[CALL_ID_LEN];
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	int nodeId, tokenNodeId, arqTokenNodeId;
	char	epstr[ENDPOINTID_LEN];
	int sd,domainIp;
	unsigned short domainPort;
	cmTransportAddress   sinfo = { 	0,
									htonl(arqHandle2->phonode.ipaddress.l), 
									H323callsigport(arqHandle2), 
									cmRASTransportTypeIP };
	HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);

	/* Open a RAS Transaction */
	gkAddr.ip = rfphonodep->ipaddress.l;

	if (H323rfrasport(arqHandle2) != 0)
	{
		gkAddr.port = H323rfrasport(arqHandle2);
	}
	
	if (IN_MULTICAST(gkAddr.ip))
	{
		gkAddr.port = 1718;
	}

	/* We need the endpoint id, assigned by this gatekeeper
	 * to us. For that, we will query the entry from the cache
	 */
	cacheInfo = &cacheInfoEntry;
	if (CacheFind(regCache, rfphonodep, cacheInfo,
		sizeof(CacheTableInfo)) < 0)
	{
		NETERROR(MARQ, ("%s Could not locate cache entry for gk %s/%lu\n", 
			fn, rfphonodep->regid, rfphonodep->uport));
		goto _error;
	}

	cacheGkInfo = &cacheGkInfoEntry;
	if (CacheFind(gkCache, rfphonodep, cacheGkInfo,
		sizeof(CacheGkInfo)) < 0)
	{
		NETERROR(MARQ, ("%s Could not locate GK cache entry for gk %s/%lu\n", 
			fn, rfphonodep->regid, rfphonodep->uport));
		goto _error;
	}

	if ((cacheGkInfo->endpointIDLen <= 0) ||
		(cacheGkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MARQ, ("%s endpointID from gk %s/%lu is invalid\n",
			fn, cacheGkInfo->regid, cacheGkInfo->uport));
		goto _error;
	}

	gkAddr.ip = htonl(gkAddr.ip);
	NETDEBUG(MARQ, NETLOG_DEBUG4,
		   ("%s Sending ARQ to %s/%d for:\n", 
			fn, (char*) ULIPtostring(ntohl(gkAddr.ip)), gkAddr.port));

	if (cmRASStartTransaction(UH323Globals()->hApp,
		   (HAPPRAS)data, /* Application Ras Handle */
		   &hsRas,
		   cmRASAdmission,
		   &gkAddr,
		   NULL) < 0)
	{
		NETERROR(MARQ, 
		   ("%s cmRASStartTransaction failed\n", fn));
		goto _error;
	}	

	/* Set the ARQ parameters from the handle passed */
	ARQSetParamsFromHandle(arqHandle2, hsRas);
	
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallType, 0, cmCallTypeP2P, NULL) < 0)
	{
		NETERROR(MARQ, ("%s cmRASSetParam cmRASParamCallType failed\n", fn));
	}
	
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallModel, 0, cmCallModelTypeDirect, NULL) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASSetParam cmRASParamCallModelType failed\n", fn));
	}
	
	alias.type = cmAliasTypeE164;
	alias.string = rfphonodep->phone;
	alias.length = strlen(alias.string);
	
	DEBUG(MARQ, NETLOG_DEBUG1,
		("dst Alias - %s\n", alias.string));

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamDestInfo, 0, sizeof(cmAlias), (char *)&alias) < 0)
	{
		NETERROR(MARQ, ("cmRASSetParam cmRASParamDestInfo failed\n"));
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamBandwidth, 0, 64000, NULL) < 0)
	{
		NETERROR(MARQ, ("cmRASSetParam cmRASParamBandwidth failed\n"));
	}

	H323arqcrid(arqHandle2) = cacheGkInfo->crId;
	H323arqip(arqHandle2) = rfphonodep->ipaddress.l;

	endpointID.type = cmAliasTypeEndpointID;
	
	if (BIT_TEST(rfphonodep->cap, CAP_MSW))
	{
		char *tmpstr;

		tmpstr = malloc(cacheGkInfo->endpointIDLen+20);
		utlBmp2Chr(&tmpstr[0], cacheGkInfo->endpointIDString, cacheGkInfo->endpointIDLen);
		sprintf(&tmpstr[strlen(tmpstr)], "!%d", rfphonodep->cap);
		endpointID.length = utlChr2Bmp(&tmpstr[0], &endptId[0]);
		endpointID.string = &endptId[0];

		utlBmp2Chr(epstr, endpointID.string, endpointID.length);

		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamEndpointID, 0, sizeof(cmAlias), 
			(char *)&endpointID) < 0)
		{
			NETERROR(MARQ, ("%s cmRASSetParam cmRASParamEndpointID failed\n", fn));
		}
	
		free(tmpstr);
	}
	else
	{
		endpointID.length = cacheGkInfo->endpointIDLen;
		endpointID.string = cacheGkInfo->endpointIDString;
		utlBmp2Chr(epstr, endpointID.string, endpointID.length);

		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamEndpointID, 0, sizeof(cmAlias), 
			(char *)&endpointID) < 0)
		{
			NETERROR(MARQ, ("%s cmRASSetParam cmRASParamEndpointID failed\n", fn));
		}
	}

    NETDEBUG(MARQ, NETLOG_DEBUG4, ("%s endpointId = %s len = %d\n", 
	 	fn,epstr,cacheGkInfo->endpointIDLen));
       
#if 0
	alias.type = cmAliasTypeE164;
	alias.string = cacheInfo->data.phone;
	alias.length = strlen(alias.string);
	
	uh323AddRASAlias(hsRas, &alias, &index, 
				 cmRASTrStageRequest, cmRASParamSrcInfo);
#endif

	alias.type = cmAliasTypeE164;
	alias.string = H323callingPartyNumber(arqHandle2);
	alias.length = H323callingPartyNumberLen(arqHandle2);

	uh323AddRASAlias(hsRas, &alias, &index, 
				 cmRASTrStageRequest, cmRASParamSrcInfo);

	alias.type = cmAliasTypeH323ID;
    if (id)
	{
		alias.length = utlChr2Bmp(id, &h323_id[0]);
	}
	else
	{
		alias.length = utlChr2Bmp(cacheInfo->data.h323id, &h323_id[0]);
		if (arqHandle2->callSource == 1)
		{
			nx_strlcpy(H323h323Id(arqHandle2), cacheInfo->data.h323id, H323ID_LEN);
			// Save h323id in conf handle so it is sent in 
			// setups to alternate endpoints
			saveEgressH323Id(arqHandle2->confID, cacheInfo->data.h323id);
		}
	}
	alias.string = &h323_id[0];
	
	DEBUG(MARQ, NETLOG_DEBUG1,
		("my Alias - %s\n", (id)? id : cacheInfo->data.h323id));

	uh323AddRASAlias(hsRas, &alias, &index, 
				 cmRASTrStageRequest, cmRASParamSrcInfo);

	index = 0;

// We only add the SrcCallSignalAddress if this ARQ is associated with
// an ingress (leg1) call.  If we try to add this address to an egress
// call, this code will put the destination's address as our source
// address.  So to avoid the need to figure out which interface/public/private
// source address should used be for this call, we do not add
// SrcCallSignalAddress on a leg2 call.
	if (!arqHandle2->callSource)
	{
		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamSrcCallSignalAddress, 0, 0,
			(char *)&sinfo) < 0)
		{
			NETERROR(MARQ, ("%s cmRASSetParam cmRASParamSrcCallSignalAddress failed\n", fn));
		}
	}

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
		"request.admissionRequest.canMapAlias", 1, NULL) < 0)
	{
		NETERROR(MARQ, ("%s could not build MapAlias\n", fn));
	}

	if ( arqHandle2->tg && arqHandle2->tg[0] )
	{
    	if (pvtBuildByPath(hVal, nodeId,
       		"request.admissionRequest.circuitInfo.sourceCircuitID.group.group",
       		strlen(arqHandle2->tg), arqHandle2->tg) < 0)
    	{
     		NETERROR(MARQ, ("%s could not build trunk group\n", fn));
    	}
	}

	if ( arqHandle2->destTg && arqHandle2->destTg[0] && (arqHandle2->ecaps1 & ECAPS1_SETDESTTG))
	{
    	if (pvtBuildByPath(hVal, nodeId,
       		"request.admissionRequest.circuitInfo.destinationCircuitID.group.group",
       		strlen(arqHandle2->destTg), arqHandle2->destTg) < 0)
    	{
     		NETERROR(MARQ, ("%s could not build dest trunk group\n", fn));
    	}
	}

	// Set Tokens if Any
	if((tokenNodeId = H323tokenNodeId(arqHandle2)) >0)
	{
		if ((arqTokenNodeId = pvtGetNodeIdByPath(hVal,
			nodeId,
			"request.admissionRequest")) > 0)
		{
			if(pvtAddTree(hVal,arqTokenNodeId,peerhVal,tokenNodeId) < 0)
			{
				NETERROR(MH323, ("%s could not set token in ARQ hsRas=%p\n",fn,hsRas));
			}
		}
		else {
				NETERROR(MH323, ("%s could not get arqTokenNodeId hsRas=%p\n",fn,hsRas));
		}
	}

	cmMeiExit(UH323Globals()->hApp);

	arqAddAuthentication(hsRas,cacheInfo->data.h323id,cacheInfo->data.passwd);
	if(arqHandle2->realmInfo)
	{
		if(getRasInfoFromRealmId(arqHandle2->realmInfo->realmId,Ras_eSgk, &sd,
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
	}
	else {
		  NETERROR(MLRQ,("%s no realm info present hsRas=%p %s\n", 
				fn,hsRas, (char*) CallID2String(arqHandle2->callID, callIdStr)));
	}
	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MARQ, ("%s cmRASRequest failed for admission\n", fn));
		goto _error;
	}

	NETDEBUG(MARQ, NETLOG_DEBUG1,
		("%s Started the RAS Transaction successfully, hsRas = %p callId = %s\n", 
		fn, hsRas, (char*) CallID2String(arqHandle2->callID, callIdStr)));

	return 0;

_error:
	if (hsRas) 
	{
		cmRASClose(hsRas);
	}

	return -1;
}

int
GkHandleACF(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	char fn[] = "GkHandleACF():";
	CallHandle *callHandle1, *callHandle2;
	char callID2[CALL_ID_LEN];
	cmTransportAddress dstSignalAddr;
	int addrlen = sizeof(cmTransportAddress);
	SCC_EventBlock *evtPtr = (SCC_EventBlock *)haRas; 
	H323EventData		*pH323Data;
	char *start;
	char tmpstr[PHONE_NUM_LEN];
	INT32 tlen;
	cmAlias number;
	BYTE string[80];
	int index = 0, answerCall;
	ConfHandle *confHandle;
	int egressTokenNodeId;

	if (evtPtr == NULL)
	{
		NETERROR(MARQ, ("%s No Event Pointer\n", fn));
		return -1;
	}

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	/* Extract the call id, and see if the call is still up.
	 * If it is, we now modify the handle, and insert the information
	 * we have obtained from the ARQ. After this we call MakeCall
	 */ 
	callHandle1 = CacheGet(callCache,evtPtr->callID);

	if (callHandle1 == NULL)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
		goto _error;
	} 

	callHandle1->lastEvent = SCC_evt323ACFRx;

	/* We must extract the first call handle, and fill in
	 * the data
	 */
	getPeerCallID(evtPtr->confID,evtPtr->callID,callID2);

	callHandle2 = CacheGet(callCache, callID2);
	if (callHandle2 == NULL)
	{
		/* Extract the Answer Call parameter */
		if (cmRASGetParam(hsRas, cmRASTrStageRequest, 
						 cmRASParamAnswerCall, 0, 
						 &answerCall, NULL) < 0)
		{
			NETERROR(MARQ, ("%s No AnswerCall field found\n", fn));
			answerCall = FALSE;
		}

		if ( answerCall != TRUE)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
			   ("%s Did not find Call handle for other leg \n",fn));
			CacheReleaseLocks(callCache);
	 		return GkHandleARQFailure(haRas, hsRas, -2);
		}

		/* Handle case of Ingress ARQ */
		evtPtr->evtProcessor = SCCNetworkSetup;
		SCC_QueueEvent(evtPtr);
		CacheReleaseLocks(callCache);
		return 0;
	} 

	if (cmRASGetParam(hsRas, cmRASTrStageConfirm,
		cmRASParamDestCallSignalAddress,
		0,
		&addrlen,
		(char *)&dstSignalAddr) < 0)
	{
		NETERROR(MARQ, 
			("%s No Destination address\n", fn));
		goto _error;
	}

	/* Change the signalling address.
	 * Also, copy the old address (gk address),
	 * so that we can send a DRQ...
	 */ 
	H323arqip(callHandle1) = callHandle2->rfphonode.ipaddress.l;

	callHandle1->phonode.ipaddress.l = callHandle2->rfphonode.ipaddress.l = ntohl(dstSignalAddr.ip);
	H323rfcallsigport(callHandle2) = dstSignalAddr.port;

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		("%s ARQ Destination is %s/%d\n",
		fn, 
		(char*) ULIPtostring(callHandle2->rfphonode.ipaddress.l), 
		(int) H323rfcallsigport(callHandle2)));

	if(!(pH323Data = evtPtr->data))
	{
		NETERROR(MARQ,("%s : H323Data Null in event Ptr", fn ));
		goto _error;
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
			   NETDEBUG(MARQ, NETLOG_DEBUG1, 
						("%s DestInfo contains %s\n",
						 fn, number.string));
			   nx_strlcpy(pH323Data->calledpn, 
					number.string, PHONE_NUM_LEN);

				// Fix the dialled # field in the CDRs
				strcpy(callHandle2->rfphonode.phone, 
					pH323Data->calledpn);
				strcpy(callHandle1->phonode.phone, 
					pH323Data->calledpn);
				strcpy(H323dialledNumber(callHandle1), 
					pH323Data->calledpn);
		  }

		  /* Extract the Destination Info */
		  tlen = sizeof(cmAlias);
		  number.length = 80;
		  number.string = (char *)string;
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		("%s ARQ Destination is %s/%d\n",
		fn, 
		(char*) ULIPtostring(callHandle2->rfphonode.ipaddress.l), 
		(int) H323rfcallsigport(callHandle2)));

	if (BIT_TEST(callHandle1->phonode.cap, CAP_TPG))
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4, ("%s checking tech prefix\n", fn));

		// We need to find what was originally dialled
		// this in in the callhandle1, as of now...
		strcpy(pH323Data->calledpn, H323dialledNumber(callHandle2));

		// Fix the dialled # field in the CDRs
		strcpy(callHandle2->rfphonode.phone, 
			H323dialledNumber(callHandle2));

		strcpy(callHandle1->phonode.phone, 
			H323dialledNumber(callHandle2));
	}

	pH323Data->destip = ntohl(dstSignalAddr.ip);
	pH323Data->destport = dstSignalAddr.port;

	// Get token node ID from ACF for leg2.
	// Store it in conf handle and keep a copy of it
	// in call handle of leg 2. The copy in the call handle
	// is never deleted. The egress token node ID gets 
	// deleted when the conf handle is deleted.
	if (callHandle1->callSource == 1)
	{
		if ((egressTokenNodeId = getAcfTokenNodeId(hsRas)) < 0)
		{
        	NETDEBUG(MARQ,NETLOG_DEBUG4,
				("%s getAcfTokenNodeId failed",fn ));
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

	    /* check for vendor type == cisco */
    	if (callHandle1->vendor == Vendor_eCisco && getAniFromAcf) 
	    	decodeACFNonStandardInfo(hsRas, haRas, callHandle1, callHandle2);
	}


	/* get alternate endPoints */
	GkGetAlternateEP( hsRas, "admissionConfirm", pH323Data->calledpn, callHandle2 );

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
GkHandleARQFailure(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	char 		fn[] = "GkHandleARQFailure():";
	CallHandle 	*callHandle = NULL;
	char 		callIdStr[CALL_ID_STRLEN];
	SCC_EventBlock *evtPtr = (SCC_EventBlock *)haRas;
	int callError;

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		   ("%s Entering haRas = %p, hsRas = %p\n",
			fn, haRas, hsRas));

	/* We have to basically generate an error, so that
	 * leg 1 call is released.
	 */	
	if (evtPtr == NULL)
	{
		NETERROR(MARQ, ("%s Null appHandle\n", fn));
		return 0;
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if(!(callHandle = CacheGet(callCache, evtPtr->callID)))
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			   ("%s Did not find Call handle %s\n",
				fn, (char*) CallID2String(evtPtr->callID, callIdStr)));

		goto _error;
	}

	H323arqip(callHandle) = 0;

	callHandle->lastEvent = SCC_evt323ARJRx;

	switch (reason)
	{
	case -1:
		// -1 means timeout, there is no reason
		callError = SCC_errorDestTimeout;
		callHandle->lastEvent = SCC_evt323ARQTmout;
		break;
	case -2:
        	// -2 means that by the time ACF was received,
        	// the call handle for other leg was deleted
        	callError = SCC_errorNoCallHandle;
        	callHandle->lastEvent = SCC_evt323ACFRx;
        	// Set reason to -1 so it is translated to correct RAS reason
        	reason++;
        	break;
	case cmRASReasonCalledPartyNotRegistered:
		callError = SCC_errorDestNoRoute;
		break;
	case cmRASReasonResourceUnavailable:
        	callError = SCC_errorResourceUnavailable;
        	break;

	// Following are reasons which pertain to MSW.
	// To caller these will be undefined
	// but must be logged on the MSW.
	case cmRASReasonInvalidEndpointID:
		callError = SCC_errorMswInvalidEpId;
		break;
	case cmRASReasonRouteCallToGatekeeper:
		callError = SCC_errorMswRouteCallToGk;
		break;
	case cmRASReasonCallerNotRegistered:
		callError = SCC_errorMswCallerNotRegistered;
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

_error:
	CacheReleaseLocks(callCache);
	H323FreeEvent(evtPtr);

	return 0;
}

int
GkHandleARJ(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas,
		IN	cmRASReason		reason)
{
	 char fn[] = "GKHandleARJ():";

	 NETDEBUG(MARQ, NETLOG_DEBUG4, 
		   ("%s Entering haRas = %p, hsRas = %p, reason = %d\n",
			fn, haRas, hsRas, reason));

	 return GkHandleARQFailure(haRas, hsRas, reason);
}

int
GkHandleARQTimeout(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	 char fn[] = "GKHandleARQTimeout():";

	 NETDEBUG(MARQ, NETLOG_DEBUG4, 
		   ("%s Entering haRas = %p, hsRas = %p, reason = %d\n",
			fn, haRas, hsRas, 0));

	 return GkHandleARQFailure(haRas, hsRas, -1);
}

/* Who do we send the DRQ to ?
 * The address to which we sent ARQ is lost,
 * replaced by the address which came in ACF
 */
int
GkSendDRQ(ARQHandle *arqHandle)
{
	char fn[] = "GkSendDRQ():";
	cmTransportAddress 
		  gkAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	int i;
	HRAS hsRas = NULL;
	BYTE h323_id[256];
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;	
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;	
	cmAlias alias, endpointID;
	int index = 0;
	char callIdStr[CALL_ID_LEN];
	int tokenNodeId,nodeId,drqNodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	BYTE gk_id[128];
	RealmIP  realmip;
	int sd,domainIp;
	unsigned short domainPort;

	if (arqHandle == NULL)
	{
		return -1;
	}

	gkAddr.ip = htonl(H323arqip(arqHandle));

	// prefer crid, if its there
	if (crids && H323arqcrid(arqHandle))
	{
		cacheInfo = &cacheInfoEntry;
		if (CacheFind(cridCache, &H323arqcrid(arqHandle), cacheInfo,
			sizeof(CacheTableInfo)) < 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Could not locate cache entry for crid %lu\n", 
				fn, H323arqcrid(arqHandle)));

			realmip.ipaddress = H323arqip(arqHandle);
			realmip.realmId = REALM_ID_TBD;
			if (CacheFind(ipCache, &realmip, cacheInfo, sizeof(CacheTableInfo)) < 0)
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Could not locate ip cache entry for %lx\n", 
					fn, H323arqip(arqHandle)));
			}
		}
		else
		{
			gkAddr.ip = htonl(cacheInfo->data.ipaddress.l);
		}
	}
	else
	{
		cacheInfo = &cacheInfoEntry;
		if (CacheFind(regCache, &arqHandle->phonode, cacheInfo,
			sizeof(CacheTableInfo)) < 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Could not locate cache entry for %s/%lu\n", 
				fn, arqHandle->phonode.regid, arqHandle->phonode.uport));

			realmip.ipaddress = H323arqip(arqHandle);
			realmip.realmId = REALM_ID_TBD;
			if (CacheFind(ipCache, &realmip, cacheInfo, sizeof(CacheTableInfo)) < 0)
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Could not locate ip cache entry for %lx\n", 
					fn, H323arqip(arqHandle)));

				cacheInfo = NULL;
			}
		}
	}

	if  (SrcARQEnabled() && sgkSN.regid[0] && 
		(BIT_TEST(H323flags(arqHandle), H323_FLAG_AUTHARQ_SENT)))
	{
		cacheInfo = &cacheInfoEntry;
		if (CacheFind(regCache, &sgkSN, cacheInfo, sizeof(CacheTableInfo)) 
			< 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Could not locate SGK cache entry for %s/%d\n", 
				fn, sgkSN.regid, sgkSN.uport));
			return 0;
		}
		NETDEBUG(MARQ, NETLOG_DEBUG1,
			("%s Destination for DRQ set to SGK %s/%d\n",
			fn, sgkSN.regid, sgkSN.uport));
	}

	if (gkAddr.ip == 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s No DRQ Needs to be sent for this call %s\n",
			fn, (char*) CallID2String(arqHandle->callID, callIdStr)));
		return 0;
	}

	/* We need the endpoint id, assigned by this gatekeeper
	 * to us. For that, we will query the entry from the cache
	 */

	cacheGkInfo = &cacheGkInfoEntry;
	if (CacheFind(gkCache, cacheInfo?(PhoNode *)&cacheInfo->data:&arqHandle->phonode, 
			cacheGkInfo, sizeof(CacheGkInfo)) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Could not locate gk cache entry based on ip\n", fn));

		cacheGkInfo = NULL;
	}

	if (cacheGkInfo == NULL)
	{
		NETERROR(MARQ,
			("%s Could not locate gk cache entry for %lx\n", 
			fn, H323arqip(arqHandle)));

		goto _error;
	}

	if (cacheGkInfo->regState != GKREG_REGISTERED)
	{
		NETERROR(MARQ,
			("%s We are not registered to Gk %s/%lu anymore, dropping DRQ\n",
			fn, cacheGkInfo->regid,cacheGkInfo->uport));

		goto _error;
	}

	if ((cacheGkInfo->endpointIDLen <= 0) ||
		(cacheGkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MARQ, ("%s endpointID from gk %s/%lu is invalid\n",
			fn, cacheGkInfo->regid, cacheGkInfo->uport));

		goto _error;
	}

	if (H323rfrasport(arqHandle)!= 0)
	{
		gkAddr.port = H323rfrasport(arqHandle);
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		   ("%s Sending DRQ to %s/%d for:\n", 
			fn, (char*) ULIPtostring(ntohl(gkAddr.ip)), gkAddr.port));

	if (cmRASStartTransaction(UH323Globals()->hApp,
		   (HAPPRAS)NULL, /* Application Ras Handle */
		   &hsRas,
		   cmRASDisengage,
		   &gkAddr,
		   NULL) < 0)
	{
		NETERROR(MARQ, 
		   ("%s cmRASStartTransaction failed\n", fn));
		goto _error;
	}	

	/* Set the ARQ parameters from the handle passed */
	ARQSetParamsFromHandle(arqHandle, hsRas);
	
	endpointID.type = cmAliasTypeEndpointID;
	endpointID.length = cacheGkInfo->endpointIDLen;
	endpointID.string = cacheGkInfo->endpointIDString;

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamEndpointID, 0, sizeof(cmAlias), 
		(char *)&endpointID) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASSetParam cmRASParamEndpointID failed\n", fn));

		goto _error;
	}

	/* Set the DRQ reason */
	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamDisengageReason, 0,
		cmRASDisengageReasonNormalDrop, NULL) < 0)
	{
		NETERROR(MARQ,
			("%s cmRASSetParam cmRASParamDisengageReason failed\n",
			fn));
	}



	if((tokenNodeId = H323tokenNodeId(arqHandle)) >0)
	{
		HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
		pvtPrintStd(peerhVal,tokenNodeId,72);
	}
#if 0
	/* Add Token if we have one */
	if((tokenNodeId = H323tokenNodeId(arqHandle)) >0)
    {
		HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
		cmMeiEnter(UH323Globals()->hApp);
		cmMeiEnter(UH323Globals()->peerhApp);
        if((nodeId = cmGetProperty((HPROTOCOL)hsRas)) >0)
        {
			if ((drqNodeId = pvtGetNodeIdByPath(hVal,
				nodeId,
				"request.disengageRequest")) >= 0)
			{
				if(pvtAddTree(hVal,drqNodeId,peerhVal,tokenNodeId) < 0)
				{
					NETERROR(MARQ, ("%s could not set token in drq\n",fn));
				}
			}
			else {
					NETERROR(MARQ, ("%s could not get drqNodeId\n",fn));
			}
        }
		else {
            NETERROR(MARQ,("%s : failed to get nodeId\n", fn ));
		}
		cmMeiExit(UH323Globals()->peerhApp);
		cmMeiExit(UH323Globals()->hApp);
    }
#endif

	index = 0;
	{
		drqAddAuthentication(hsRas,cacheInfo->data.h323id,cacheInfo->data.passwd, arqHandle);
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
	}

	if(arqHandle->realmInfo)
	{
		if(getRasInfoFromRealmId(arqHandle->realmInfo->realmId,Ras_eSgk, &sd,
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
	}
	else {
		  NETERROR(MLRQ,("%s no realm info present hsRas=%p %s\n", 
				fn,hsRas, (char*) CallID2String(arqHandle->callID, callIdStr)));
	}
	/* Now start the transaction */
	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASRequest failed for admission\n", fn));
		goto _error;
	}

	NETDEBUG(MARQ, NETLOG_DEBUG1,
		("%s Started the RAS Transaction successfully, hsRas = %p callId = %s\n",
		fn, hsRas, (char*) CallID2String(arqHandle->callID, callIdStr)));

	return 0;

_error:
	if (hsRas) 
	{
		cmRASClose(hsRas);
	}

	return -1;
}

//Not used currently - cmRasConfirm in stack code needs to be fixed for this to work.
int
GkSendIRR(ARQHandle *arqHandle)
{
	char fn[] = "GkSendIRR():";
	cmTransportAddress 
		  gkAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	int i;
	HRAS hsRas = NULL;
	BYTE h323_id[256];
	CacheTableInfo *cacheInfo = NULL, cacheInfoEntry;	
	CacheGkInfo *cacheGkInfo = NULL, cacheGkInfoEntry;	
	cmAlias alias, endpointID;
	int index = 0;
	char callIdStr[CALL_ID_LEN];
	int tokenNodeId,nodeId,irrNodeId;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	BYTE gk_id[128];

	if (arqHandle == NULL)
	{
		return -1;
	}

	gkAddr.ip = htonl(H323arqip(arqHandle));

	// prefer crid, if its there
	if (crids && H323arqcrid(arqHandle))
	{
		cacheInfo = &cacheInfoEntry;
		if (CacheFind(cridCache, &H323arqcrid(arqHandle), cacheInfo,
			sizeof(CacheTableInfo)) < 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Could not locate cache entry for crid %lu\n", 
				fn, H323arqcrid(arqHandle)));

			if (CacheFind(ipCache, &H323arqip(arqHandle), cacheInfo,
				sizeof(CacheTableInfo)) < 0)
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Could not locate ip cache entry for %lx\n", 
					fn, H323arqip(arqHandle)));
			}
		}
		else
		{
			gkAddr.ip = htonl(cacheInfo->data.ipaddress.l);
		}
	}
	else
	{
		cacheInfo = &cacheInfoEntry;
		if (CacheFind(regCache, &arqHandle->phonode, cacheInfo,
			sizeof(CacheTableInfo)) < 0)
		{
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Could not locate cache entry for %s/%lu\n", 
				fn, arqHandle->phonode.regid, arqHandle->phonode.uport));

			if (CacheFind(ipCache, &H323arqip(arqHandle), cacheInfo,
				sizeof(CacheTableInfo)) < 0)
			{
				NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s Could not locate ip cache entry for %lx\n", 
					fn, H323arqip(arqHandle)));

				cacheInfo = NULL;
			}
		}
	}

	if (gkAddr.ip == 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s No IRR Needs to be sent for this call %s\n",
			fn, (char*) CallID2String(arqHandle->callID, callIdStr)));
		return 0;
	}

	/* We need the endpoint id, assigned by this gatekeeper
	 * to us. For that, we will query the entry from the cache
	 */

	cacheGkInfo = &cacheGkInfoEntry;
	if (CacheFind(gkCache, cacheInfo?(PhoNode *)&cacheInfo->data:&arqHandle->phonode, 
			cacheGkInfo, sizeof(CacheGkInfo)) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Could not locate gk cache entry based on ip\n", fn));

		cacheGkInfo = NULL;
	}

	if (cacheGkInfo == NULL)
	{
		NETERROR(MARQ,
			("%s Could not locate gk cache entry for %lx\n", 
			fn, H323arqip(arqHandle)));

		goto _error;
	}

	if (cacheGkInfo->regState != GKREG_REGISTERED)
	{
		NETERROR(MARQ,
			("%s We are not registered to Gk %s/%lu anymore, dropping IRR\n",
			fn, cacheGkInfo->regid,cacheGkInfo->uport));

		goto _error;
	}

	if ((cacheGkInfo->endpointIDLen <= 0) ||
		(cacheGkInfo->endpointIDLen > ENDPOINTID_LEN))
	{
		NETERROR(MARQ, ("%s endpointID from gk %s/%lu is invalid\n",
			fn, cacheGkInfo->regid, cacheGkInfo->uport));

		goto _error;
	}

	if (H323rfrasport(arqHandle)!= 0)
	{
		gkAddr.port = H323rfrasport(arqHandle);
	}

	NETDEBUG(MARQ, NETLOG_DEBUG4,
		   ("%s Sending DRQ to %s/%d for:hsCall = %p\n", 
			fn, (char*) ULIPtostring(ntohl(gkAddr.ip)), gkAddr.port,H323hsCall(arqHandle)));

	if (cmRASStartTransaction(UH323Globals()->hApp,
		   (HAPPRAS)NULL, /* Application Ras Handle */
		   &hsRas,
		   cmRASUnsolicitedIRR,
		   &gkAddr,
		   H323hsCall(arqHandle)) < 0)
	{
		NETERROR(MARQ, 
		   ("%s cmRASStartTransaction failed\n", fn));
		goto _error;
	}	

	/* Set the ARQ parameters from the handle passed */
	// ARQSetParamsFromHandle(arqHandle, hsRas);
	
	endpointID.type = cmAliasTypeEndpointID;
	endpointID.length = cacheGkInfo->endpointIDLen;
	endpointID.string = cacheGkInfo->endpointIDString;

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamEndpointID, 0, sizeof(cmAlias), 
		(char *)&endpointID) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASSetParam cmRASParamEndpointID failed string = %s\n", fn,endpointID.string));
	}

	endpointID.type = cmAliasTypeH323ID;
	endpointID.length = utlChr2Bmp(cacheInfo->data.h323id, &h323_id[0]);
	endpointID.string = &h323_id[0];

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamEndpointAlias, 0, sizeof(cmAlias), 
		(char *)&endpointID) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASSetParam cmRASParamEndpointID failed string = %s\n", fn,endpointID.string));
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamNeedResponse, 0, TRUE, NULL) < 0)
	{
		NETERROR(MARQ, 
			("%s cmRASSetParam NeedResponse failed \n", fn));
	}
	/* Add Token if we have one */
	//if((tokenNodeId = H323tokenNodeId(arqHandle)) >0)
    {
		HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
		cmMeiEnter(UH323Globals()->hApp);
		cmMeiEnter(UH323Globals()->peerhApp);
        if((nodeId = cmGetProperty((HPROTOCOL)hsRas)) >0)
        {
			irrAddAuthentication(nodeId,cacheInfo->data.h323id,cacheInfo->data.passwd);
        }
		else {
            NETERROR(MARQ,("%s : failed to get nodeId\n", fn ));
		}
		{
			if ((irrNodeId = pvtGetNodeIdByPath(hVal,
				nodeId,
				"request.infoRequestResponse")) >= 0)
			{
				if(pvtAddTree(hVal,irrNodeId,peerhVal,tokenNodeId) < 0)
				{
					NETERROR(MARQ, ("%s could not set token in irr\n",fn));
				}
			}
			else {
					NETERROR(MARQ, ("%s could not get irrnodeid\n",fn));
			}
        }
		cmMeiExit(UH323Globals()->peerhApp);
		cmMeiExit(UH323Globals()->hApp);
    }
	index = 0;
	{
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
	}

	/* Now start the transaction */
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MARQ, 
			("%s  cmRasRequest failed for IRR\n", fn));
	}

	NETDEBUG(MARQ, NETLOG_DEBUG1,
		("%s Started RAS Transaction successfully, hsRas = %p callId = %s\n",
		fn, hsRas, (char*) CallID2String(arqHandle->callID, callIdStr)));

	return 0;

_error:
	if (hsRas) 
	{
		cmRASClose(hsRas);
	}

	return -1;
}/* Find the source cache information for the ARQ
   In case answercall = true, then we just return.
*/
int
GkFindSrcForArq(
	IN	HRAS		hsRas,
	IN	HCALL		hsCall,
	int				answerCall,
	int				newCallHandle,
	ARQHandle 		*arqHandle,
	CacheTableInfo	*scacheInfo)
{
	char fn[] = "GkFindSrcForArq:";
	int rc = -1, done = 0;
	PhoNode *phonodep;
	char string[80];
	INT32  tlen;
	int index = 0; 
	cmAlias number;
	char h323id[H323ID_LEN] = { 0 };

	if (answerCall == TRUE)
	{
		return 1;	
	}
	else
	{
		phonodep = &arqHandle->phonode;
	}


	tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = string;

	while ( (cmRASGetParam(hsRas, cmRASTrStageRequest, 
			cmRASParamSrcInfo, index++, 
			&tlen, (char *)&number)) >= 0)
	{
		if (number.type != cmAliasTypeE164 && number.type != cmAliasTypeH323ID)
		{
			 goto _continue;
		}

		NETDEBUG(MARQ, NETLOG_DEBUG4, 
			("%s Found Alias %s in ARQ\n", fn, number.string));

        if (number.type == cmAliasTypeE164)
		{
			strcpy(phonodep->phone, number.string);
			BIT_SET(phonodep->sflags, ISSET_PHONE);
		}
        else if (number.type == cmAliasTypeH323ID)
        {
			utlBmp2Chr(&h323id[0], number.string, number.length);
		}

		rc = FillSourceCacheForCallerIdForceLkup(phonodep, h323id, arqHandle->tg, 
				"", arqHandle->rfphonode.phone, scacheInfo, 1);

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
		rc = FillSourceCacheForCallerIdForceLkup(phonodep, h323id, arqHandle->tg, 
				"", arqHandle->rfphonode.phone, scacheInfo, 1);
	}

	return rc;
}

#if 0
int GkSendIRQ(struct Timer *t)
{
	char fn[] = "GkSendIRQ():";
	char *callid = (char *)t->data;
	HRAS hsRas = NULL;
	cmTransportAddress 
		  gkAddr = { 0, 0, 1719, cmRASTransportTypeIP };
	int crv;
	CallHandle *callHandle = NULL;
	int sd,domainIp;
	unsigned short domainPort;
	char callIdStr[32];
	void *timerdata;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = (CallHandle *)CacheGet(callCache, callid);

	if (callHandle == NULL)
	{
		NETDEBUG(MIRQ, NETLOG_DEBUG4, ("%s Call already deleted\n", fn));
		
		// Get rid of the timer
		if (timerDeleteFromList(&h323timerPrivate[0], t->id, &timerdata))
		{
			NETDEBUG(MIRQ, NETLOG_DEBUG4, ("%s timer deleted\n", fn));
			if(timerdata)
			{
				free(timerdata);
			}
		}

		t->id = 0;

		goto _return;
	}

	// Send an IRQ for the call handle ONLY for the origin of 
	// the call
	/* Open a RAS Transaction */
	gkAddr.ip = htonl(H323rasip(callHandle));

	if (H323rasport(callHandle) != 0)
	{
		gkAddr.port = H323rasport(callHandle);
	}
	
	NETDEBUG(MIRQ, NETLOG_DEBUG4,
		   ("%s Sending IRQ to %s/%d for:\n", 
			fn, (char*) ULIPtostring(ntohl(gkAddr.ip)), gkAddr.port));

	if (cmRASStartTransaction(UH323Globals()->hApp,
		   (HAPPRAS)NULL, /* Application Ras Handle */
		   &hsRas,
		   cmRASInfo,
		   &gkAddr,
		   NULL) < 0)
	{
		NETERROR(MIRQ, 
		   ("%s cmRASStartTransaction failed\n", fn));

		goto _return;
	}	

	if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamCRV,
			0,
			H323outCRV(callHandle),
			NULL) < 0)
	{
		DEBUG(MIRQ, NETLOG_DEBUG1,
			("cmRASSetParam cmRASParamCRV returned error\n"));
	}

	if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamReplyAddress,
			0,
			sizeof(UH323Globals()->rasAddr),
			(char *)&UH323Globals()->rasAddr) < 0)
	{
		DEBUG(MIRQ, NETLOG_DEBUG1,
			("cmRASSetParam cmRASParamReplyAddress returned error\n"));
	}

	if (cmRASSetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamCallID,
		0,
		CALL_ID_LEN,
		callid) < 0)
	{
		DEBUG(MIRQ, NETLOG_DEBUG1,
			("cmCallSetParam CallId returned error\n"));
	}
	if(callHandle->realmInfo)
	{
		if(getRasInfoFromRealmId(callHandle->realmInfo->realmId,Ras_eSgk, &sd,
			&domainIp,&domainPort) >=0)
		{
		    if (cmRASSetParam(hsRas, cmRASTrStageRequest,
                      cmRASParamSocketDesc, 0,
                      sd, NULL) < 0)
			{
				NETERROR(MLRQ,("%s Could not set SocketDesc %d hsRas=0x%x\n",
					fn,sd,hsRas));
			}
		}
	}
	else {
		  NETERROR(MLRQ,("%s no realm info present hsRas=0x%x %s\n", 
				fn,hsRas, (char*) CallID2String(callHandle->callID, callIdStr)));
	}
	/* Now start the transaction */
	
	if (cmRASRequest(hsRas) < 0)
	{
		NETERROR(MIRQ, ("%s cmRASRequest failed for info\n", fn));

		cmRASClose(hsRas);
	}

_return:
	CacheReleaseLocks(callCache);
	return 0;
}
#endif

int
GkHandleIRR(
		IN	HAPPRAS 		haRas,
		IN	HRAS			hsRas)
{
	char fn[] = "GkHandleIRR():";
	
	NETDEBUG(MIRQ, NETLOG_DEBUG4,
		("%s Received an IRR\n", fn));
	
	return 0;
}

void
GkGetAlternateEP(		/* get alternate endPoints from ACF or LCF */
	HRAS hsRas,
	char *responseName, /* "admissionConfirm" or "locationConfirm" */
	char *calledpn,		/* dailed number */
	CallHandle *callHandle )
{
	int nodeIdAddr, nodeId, index, altNodeId, altNodeIdn;
	char str[256];
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	nodeId = cmGetProperty((HPROTOCOL)hsRas);
	index = 1;

	sprintf( str, "response.%s.alternateEndpoints", responseName );
	altNodeId = pvtGetNodeIdByPath(hVal, nodeId, str);
	if ( altNodeId <= 0 )
	{
		return;
	}
	while ((altNodeIdn = pvtGetByIndex( hVal, altNodeId, index, NULL)) > 0)
	{
		
		INT32	tmpPort;
		unsigned char tmpIp[4];
		header_url_list *new_list_entry = NULL;
		int n, len;

		nodeIdAddr = pvtGetNodeFromSeq(hVal, altNodeIdn, "callSignalAddress", "ipAddress.ip", &n, NULL);
		if ( nodeIdAddr >0 && pvtGetString(hVal, nodeIdAddr, 4, (char *)tmpIp ) > 0 )	/* get IP address */  
		{
			int len, nodeIdAlias;
			sprintf( str, "callSignalAddress.%d.ipAddress.port", n );
			/* get port */
			if (pvtGetByPath(hVal, altNodeIdn, str, NULL, &tmpPort, NULL) <= 0)
			{
				/* port not found, use default */
				tmpPort = 1720;
			}
			/* build as url first */
			new_list_entry = MMalloc (sipCallCache->malloc, sizeof(header_url_list));
			new_list_entry->url = (header_url *) MMalloc(sipCallCache->malloc, sizeof(header_url));
			memset(new_list_entry->url, 0, sizeof(header_url));
			sprintf(str, "%d.%d.%d.%d", tmpIp[0], tmpIp[1], tmpIp[2], tmpIp[3] );
			new_list_entry->url->host = MMalloc(sipCallCache->malloc, strlen(str) + 1);
			strcpy(new_list_entry->url->host, str);
			new_list_entry->url->port = tmpPort;
			/* now get aliasAddress */
			nodeIdAlias = pvtGetNodeFromSeq(hVal, altNodeIdn, "aliasAddress", "e164", &n, &len);
			if ( nodeIdAlias > 0 && pvtGetString(hVal, nodeIdAlias, len, (char *)str ) > 0 )
			{
				/* found new e164, use it */
				str[len] = 0;
				new_list_entry->url->name = MMalloc(sipCallCache->malloc, len+1);
				strcpy(new_list_entry->url->name, str);
			}
			else
			{
				/* no new e164 is found, use orignal phone number */
				new_list_entry->url->name = MMalloc(sipCallCache->malloc, strlen(calledpn) + 1);
				strcpy(new_list_entry->url->name, calledpn);
			}
			new_list_entry->url->type = HEADER_ADDRESS_H323;

			/* now add it to list */
			if(callHandle->remotecontact_list)
			{
				ListInsert(callHandle->remotecontact_list->prev, new_list_entry);
			}
			else
			{
				ListInitElem(new_list_entry);
				callHandle->remotecontact_list = new_list_entry;
			}
		}
		index++;	/* go to next */
	}
}


/*
pvtGetNodeFromSeq -- get nodeid from root (nodeId) and sequence path startPath.n.itemPath,
returns -- -1 not found, otherwise the nodeid of first itemPath found
index will the index of first itemPath, if it is string, the len will be the length of string
*/
int pvtGetNodeFromSeq( HPVT hVal, int nodeId, char *startPath, char *itemPath, int *index, int *len )
{
	int i=1, nodeidIdx, nodeid, nodeidItem;
	/* search for starting point */
	nodeid = pvtGetNodeIdByPath(hVal, nodeId, startPath );
	if ( nodeid <= 0 )
	{
		return nodeid;
	}
	/* now search for the sequnce or array */
	while ( (nodeidIdx = pvtGetByIndex(hVal, nodeid, i, NULL))  > 0 )
	{
		/* see if we can find itemPath */
		if( (nodeidItem = pvtGetByPath(hVal, nodeidIdx, itemPath, NULL, len, NULL))  > 0 )
		{
			/* found */
			*index = i;
			return nodeidItem;
		}
		i++;
	}
	/* not found */
	return -1;
}

int
decodeACFNonStandardInfo(
				IN	HRAS				hsRas,
				IN	HAPPRAS 			haRas,
				CallHandle 				*callHandle1,
				CallHandle 				*callHandle2)
{
    int                 		nodeId = -1;
	static char 				fn[] = "decodeACFNonStandardInfo";

    int 						decoded_length;
    HPVT 						hVal;

	HPST						synTreeCisco = cmGetSynTreeByRootName(UH323Globals()->hApp, "ciscoacf");
    HAPP 						hApp=(HAPP)(UH323Globals()->hApp);
    BYTE						*encodeBuffer = NULL;
	cmNonStandardIdentifier 	*nsid;
	cmNonStandardParam 			nsparam;
	int							tlen =0;
	SCC_EventBlock 				*evtPtr = (SCC_EventBlock *)haRas; 
	H323EventData				*pH323Data;

    if (!hsRas ) return -1;
    if (!hApp) return -1;
	if (callHandle1 == NULL)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			   ("%s: Did not find Call handle \n",fn));
    	return -1;
	}
	if (evtPtr == NULL)
	{
		NETERROR(MARQ, ("%s No Event Pointer\n", fn));
    	return -1;
	}
	if(!(pH323Data = evtPtr->data))
	{
		NETERROR(MARQ,("%s : H323Data Null in event Ptr", fn ));
    	return -1;
	}

	hVal = cmGetValTree(UH323Globals()->hApp);
	memset(&nsparam, 0 , sizeof(nsparam));
    if (!(getEncodeDecodeBuffer(MAX_ENCODEDECODE_BUFFERSIZE, &encodeBuffer) ||  !encodeBuffer))
	{
		NETERROR(MARQ,
			("%s: Could not get decode buffer\n", fn));
    	return FALSE;
	}
		
	nsid = &(nsparam.info);	
	nsparam.length = MAX_ENCODEDECODE_BUFFERSIZE;
	nsparam.data = (char *)encodeBuffer;

	tlen = sizeof(nsparam);
	if (cmRASGetParam(hsRas,
		cmRASTrStageConfirm,
		cmRASParamNonStandard, 0, &tlen,
		(char*)&nsparam) < 0)
	{
		NETERROR(MARQ,
			("%s: Could not get non standard parameter\n", fn));
	}
	else
    {
		NETDEBUG(MARQ, NETLOG_DEBUG4,
					("%s:  Nonstandard info :t35CountryCode = %d, t35Extension = %d, manufacturerCode = %d \n",
					 		fn, nsid->t35CountryCode , nsid->t35Extension , nsid->manufacturerCode ));
        if (((nsid->t35CountryCode == COUNTRY_CODE_181) && (nsid->manufacturerCode == MANUFACTURER_CISCO)) &&
				nsparam.length > 0)
        {
    		int             nonstdMsgId = -1;
    		int             callingOctet;

            /* Decode an ACF NonstandardInfo  message */
            nonstdMsgId=pvtAddRoot(hVal, synTreeCisco, 0, NULL);

            if (cmEmDecode(hVal, nonstdMsgId, encodeBuffer, nsparam.length, &decoded_length)>=0)
            {

				NETDEBUG(MARQ, NETLOG_DEBUG4,
						("%s:  Suggested Nonstandard info decoded: nonstdMsgId = %d\n",fn, nonstdMsgId));
				

				if ((nodeId = pvtGetNodeIdByPath(hVal, nonstdMsgId,
							"srcTerminalAlias")) > 0)
				{
           			cmAlias    	alias;
					int			index = 0;
	 				char 		aliasstr[256];
					NETDEBUG(MARQ, NETLOG_DEBUG4,
							("%s: Found srcTerminalAlias = %d\n",fn, nodeId));

		            /* Convert the alias */
					alias.string = aliasstr;
					alias.length = 256;
					memset(alias.string, 0, 256);
           			while ((nodeId = pvtGetByIndex(hVal, nodeId, index + 1, NULL))>0)
					{
						int status = -1;
   						status = cmVt2Alias(hVal, &alias, nodeId);
					    if (status >= 0)
					    {
        					if (alias.type == cmAliasTypeE164)
				            {
								if(alias.length)
								{
									//strlcpy(pH323Data->callingpn, alias.string, PHONE_NUM_LEN);
									strncpy(pH323Data->callingpn, alias.string, PHONE_NUM_LEN);
									callHandle2->flags |= FL_CALL_CISCO_ANI;

									/* Fix the dialled # field in the radius */
									//strlcpy(callHandle2->gkXlatedCgn, pH323Data->callingpn, PHONE_NUM_LEN);
									strncpy(callHandle2->gkXlatedCgn, pH323Data->callingpn, PHONE_NUM_LEN);
									//strlcpy(callHandle1->gkXlatedCgn, pH323Data->callingpn, PHONE_NUM_LEN);
									strncpy(callHandle1->gkXlatedCgn, pH323Data->callingpn, PHONE_NUM_LEN);

									NETDEBUG(MARQ, NETLOG_DEBUG4,
										("%s: E164 Alias = TEL:%s\n",fn,pH323Data->callingpn));
								}
								break;
				            }
					    }
					}
				}
				else
				{
					NETDEBUG(MARQ, NETLOG_DEBUG4,
						("%s: Did not find srcTerminalAlias \n",fn));
				}
				if ((nodeId = pvtGetNodeIdByPath(hVal, nonstdMsgId,
							"dstTerminalAlias")) > 0)
				{
           			cmAlias    	alias;
					int			index = 0;
	 				char 		aliasstr[256];

					NETDEBUG(MARQ, NETLOG_DEBUG4,
							("%s: Found dstTerminalAlias = %d\n",fn, nodeId));

		            /* Convert the alias */
					alias.string = aliasstr;
					alias.length = 256;
					memset(alias.string, 0, 256);
           			while ((nodeId = pvtGetByIndex(hVal, nodeId, index + 1, NULL))>0)
					{
						int status = -1;
   						status = cmVt2Alias(hVal, &alias, nodeId);
					    if (status >= 0)
					    {
        					if (alias.type == cmAliasTypeE164)
				            {
				                alias.string[alias.length] = '\0';
								NETDEBUG(MARQ, NETLOG_DEBUG4,
										("%s: E164 Alias = TEL:%s\n",fn, alias.string));
				            }
					    }
					}
				}
				else
				{
					NETDEBUG(MARQ, NETLOG_DEBUG4,
						("%s: Did not find dstTerminalAlias \n",fn));
				}
            }
            else
            {
				NETDEBUG(MARQ, NETLOG_DEBUG4,
						("%s: Decoding Problems!\n",fn));
            }
            pvtDelete(hVal, nonstdMsgId);
        }
    }
    
    return  TRUE;
}
