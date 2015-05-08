#include "gis.h"
#include "callsm.h"
#include "uh323.h"
#include "uh323cb.h"

#include "uh323inc.h"
#include "firewallcontrol.h"
#include "uh323proto.h"
#include "uh323.h"
#include "nxosd.h"
#include <malloc.h>

#include "gk.h"
#include "bridge.h"
#include "arq.h"
#include "callutils.h"
#include "log.h"
#include "stkutils.h"

#define ROLLOVER_TIME_OUT 15
int	CallRolloverTimer(struct Timer *t);
void copyH323Rtp2RtpSet(RTPSet rtpSet[],H323RTPSet h323Set[],int nset);
void copyRtp2H323RtpSet(H323RTPSet h323Set[],RTPSet rtpSet[],int nset);
void setAlertingQ931(HCALL hsCall,int srcNodeId);
extern int findNonStandardCodec( HPVT hVal, int nonsNodeId );

CallHandle * CallAllocHandle(void);
int freeCallHandle(CallHandle * pCallHandle);
int initCallHandle(CallHandle * pCallHandle);
static char userUserStr[] = "Nextone iServer Proxy";
static char displayStr[] = "Nextone iServer Proxy";
// The default bearer capability layer1 protocol value
int bcapLayer1Default = BCAPLAYER1_G711alaw;

extern unsigned char * cmEmGetQ931Syntax(void);
extern CallHandle * CallDelete(cache_t cache, char *callid);

int disconnectCallAtMaxCallDuration(struct Timer*);

/* release the media pinholes and the h245 pinholes associated with this call handle */
void CloseFceHoles (CallHandle *callHandle)
{
	NETDEBUG(MFCE, NETLOG_DEBUG4, ("Closing pinholes for the current call leg (%d, %d, %d)\n", CallFceBundleId(callHandle), H323remoteBundleId(callHandle), H323localBundleId(callHandle)));
	if (CallFceBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), CallFceBundleId(callHandle));
		CallFceBundleId(callHandle) = 0;
	}
	if (H323remoteBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), H323remoteBundleId(callHandle));
		H323remoteBundleId(callHandle) = 0;
	}
	if (H323localBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), H323localBundleId(callHandle));
		H323localBundleId(callHandle) = 0;
	}
}

/* release the media pinholes and the h245 pinholes associated with this call handle */
void CloseFceH245Holes (CallHandle *callHandle)
{
	NETDEBUG(MFCE, NETLOG_DEBUG4, ("Closing h.245 pinholes for the current call leg (%d, %d)\n", 
		H323remoteBundleId(callHandle), H323localBundleId(callHandle)));

	if (H323remoteBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), H323remoteBundleId(callHandle));
		H323remoteBundleId(callHandle) = 0;
	}
	if (H323localBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), H323localBundleId(callHandle));
		H323localBundleId(callHandle) = 0;
	}
}

void openH245RemotePinhole (char *fn, CallHandle *callHandle, char *event, unsigned long *ip, unsigned short *port)
{
	char				ipstr[16] = {0};
	MFCP_Request			*mfcpReq;

	if (callHandle->realmInfo->sPoolId != 0 && H323h245ip(callHandle) != 0)
	{
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("Opening H.245 pinhole upon incoming %s from %s:%lu\n", event, FormatIpAddress(H323h245ip(callHandle), ipstr), H323h245port(callHandle)));
                if (CallFceSession(callHandle))
			H323remoteBundleId(callHandle) = FCEGetNextBundleId(CallFceSession(callHandle));
                else
			CallFceSession(callHandle) = FCEAllocateBundleId(&H323remoteBundleId(callHandle));
                if (CallFceSession(callHandle) == NULL)
		{
			NETERROR(MFCE, ("%s: Error opening H.245 pinhole upon incoming %s\n", fn, event));
		}
                else
		{
			mfcpReq = FCEOpenResourceSync(CallFceSession(callHandle),
									H323remoteBundleId(callHandle),
									0,
									FCE_ANY_IP,
									FCE_ANY_PORT,
									H323h245ip(callHandle),
									H323h245port(callHandle),
									0,
									0,
									0,
									0,
									callHandle->realmInfo->sPoolId,
									callHandle->realmInfo->sPoolId,
									"tcp",
									0,
									0,
									0);
			if (mfcpReq == NULL)
			{
				NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s: Error opening H.245 pinhole upon incoming %s\n", fn, event));
				H323remoteBundleId(callHandle) = 0;
			}
			else
			{
				*ip = mfcp_get_nat_dest_addr(mfcpReq);
			        *port = mfcp_get_nat_dest_port(mfcpReq);
				mfcp_req_free(mfcpReq);
			}
	        }
	}
	else
	{
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("no need to open pinhole for incoming %s from %s:%lu\n", event, FormatIpAddress(H323h245ip(callHandle), ipstr), H323h245port(callHandle)));
	}

}

int SCCNetworkSetup(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkSetup() : ";
	CallHandle 			*callHandle = NULL,callHandleBlk = {0};
	ConfHandle 			*confHandle = NULL;
	cmTransportAddress 	cmTransAddr;
	int 				addrlen;
	int 				rc;
	int 				len;
	H323EventData		*pH323Data;
	SCC_EventBlock 		*pNextEvt;
	char				callIDStr[CALL_ID_LEN] = {0};
	char				callID[CALL_ID_LEN] = { 0 }, confID[CONF_ID_LEN] = { 0 };
	unsigned long		rra;
	unsigned short		rrp;
	int					retval = -1,isString,tunnel = 0;
	int					callError = SCC_errorNone;
	char				ipStr[16] = {0};
	int					cause = 0;
	int 				bcNodeId = -1, propNodeId = -1, newNodeId = -1, cpnNodeId = -1;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 				peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	struct itimerval	timerval;
	char				*callid;
	int			        ip = 0, cpnData = 0;
	cmH245Stage			h245Stage;
	CacheTableInfo		sgkentry;
	PhoNode				arqphonode;
	char				q931display[128]= {0};
	CLIRData            *clirData = NULL;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		callError = SCC_errorH323Internal;
		goto _error;
	}
	timedef_cur(&callHandle->callStartTime);
	callHandle->callSource = 0;

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache, evtPtr->confID)) )
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
			fn, (char*) CallID2String(evtPtr->confID,callIDStr)));
		CacheReleaseLocks(confCache);
		callError = SCC_errorH323Internal;
		goto _error;
	}

	// Extract Bearer Capability and store it in conf handle
	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);
	if((propNodeId = cmGetProperty((HPROTOCOL)H323hsCall(callHandle))) >=0)
	{
		if ((bcNodeId = pvtGetNodeIdByPath(hVal,
			propNodeId ,
			"setup.message.setup.bearerCapability")) > 0)
		{
			if((newNodeId = pvtAddRoot(peerhVal,hSynBearerCap,0,NULL))>0)
			{
				if(pvtSetTree(peerhVal,newNodeId,hVal,bcNodeId) < 0)
				{
					NETERROR(MH323, ("%s could not copy bcNodeId\n",fn));
					pvtDelete(peerhVal,newNodeId);
				}
			}
			else {
				NETERROR(MH323, 
					("%s pvtAddRoot failed.Could not get new NodeId\n",fn));
			}
		}
	}
	else {
		NETERROR(MH323,("%s : failed to get property nodeid\n", fn ));
	}
	cmMeiExit(UH323Globals()->hApp);
	cmMeiExit(UH323Globals()->peerhApp);

	confHandle->setupQ931NodeId = newNodeId;
	CacheReleaseLocks(confCache);

	//if we are not doing h245 - transparently pass h245 address
#if 0
		if (cmCallSetParam(H323hsCall(callHandle),
				 cmParamEstablishH245, 
				 0, 
				 0,
				 NULL) < 0)
		{
			NETERROR(MH323, 
				 ("%s Could not set h245 off \n", fn));
			goto _error;
		}
#endif
	if (SrcARQEnabled())
	{
		/* Ingress ARQ */
		if (sgkSN.regid[0] != 0) 
		{
			if (BIT_TEST(H323flags(callHandle), H323_FLAG_AUTHARQ_SENT))
			{
				goto cont_nw_setup;
			}

			memset(&arqphonode, 0 , sizeof(PhoNode));
			memset(&sgkentry, 0 , sizeof(CacheTableInfo));

			CacheFind(regCache, &sgkSN, &sgkentry, sizeof(CacheTableInfo));
			SetPhonodeFromDb(&arqphonode, &sgkentry.data);

			callHandle->lastEvent = SCC_evt323ARQTx;
			memcpy(arqphonode.phone, callHandle->rfphonode.phone, PHONE_NUM_LEN);

			if (GkSendARQ(&arqphonode, callHandle,evtPtr, H323h323Id(callHandle)) >= 0)
			{
			    BIT_SET(H323flags(callHandle), H323_FLAG_AUTHARQ_SENT);
				NETDEBUG(MSCC,NETLOG_DEBUG2,
						("%s AUTH ARQ sent to SGK %s/%lu for callid = %s hsCall = %p\n", fn,
						 sgkentry.data.regid, sgkentry.data.uport, 
						(char*) CallID2String(evtPtr->callID,callIDStr),
						 H323hsCall(callHandle)));
				CacheReleaseLocks(callCache);
				return 0;
			}
			else
			{
				NETDEBUG(MSCC,NETLOG_DEBUG2,
						("%s Failed to send AUTH ARQ to SGK %s/%lu for callid = %s hsCall = %p\n", fn,
						 sgkentry.data.regid, sgkentry.data.uport, 
						(char*) CallID2String(evtPtr->callID,callIDStr),
						 H323hsCall(callHandle)));
			}
		}
		callError = SCC_errorMswRouteCallToGk;
		CacheReleaseLocks(callCache);
		goto _error;
	}


cont_nw_setup:

	addrlen = sizeof(cmTransportAddress);
	if(cmCallGetParam(H323hsCall(callHandle),
				cmParamSetupH245Address,
				0,
				&addrlen,
				(char *)&cmTransAddr) <0)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s Unable to retrieve cmParamSetupH245Address\n",fn));
	}
	else {
		H323h245ip(callHandle) = cmTransAddr.ip;
		H323h245port(callHandle) = cmTransAddr.port;
	}
	if (cmCallGetParam (H323hsCall (callHandle), cmParamLocalIp,
		0, &ip, NULL) < 0)
	{
		 NETERROR(MH323,
			("%s Could not get localIp\n", fn));
	}
	NETDEBUG (MH323, NETLOG_DEBUG4, ("%s localIP = %x\n", fn, ip));

	if(H323nlocalset(callHandle)>0  && (doFastStart) && !IsRolledOver(callHandle->dialledNumber))
	{
		callHandle->fastStart = 1;
	}
	H323doOlc(callHandle) = 1;

	if(callHandle->fastStart && (callHandle->ecaps1 & ECAPS1_NOCONNH245))
	{
		h245Stage = cmTransH245Stage_facility;
	}
	else 
	{
		h245Stage = cmTransH245Stage_connect;
	}

	if (cmCallSetParam(H323hsCall(callHandle),
		cmParamH245Stage,
		0,
		h245Stage,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set H245Stage to %d\n", fn,h245Stage));
	}

	addrlen = sizeof(cmTransAddr);
	if(cmCallGetParam(H323hsCall(callHandle),
				cmParamRemoteIpAddress,
				0,
				&addrlen,
				(char *)&cmTransAddr) <0)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s Unable to retrieve cmParamRemoteIpAddress\n",fn));
	}
	else {
		callHandle->peerIp = ntohl(cmTransAddr.ip);
		callHandle->peerPort = cmTransAddr.port;
	}

	rra = H323h245ip(callHandle);
	rrp = H323h245port(callHandle);

	openH245RemotePinhole(fn, callHandle, "SETUP", &rra, &rrp);

	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));

	if(callHandle->fastStart)
	{
		callHandle->lastMediaIp = H323localSet(callHandle)[0].rtpaddr;
		callHandle->lastMediaPort = H323localSet(callHandle)[0].rtpport;
	}

	/* Start max call duration timer */
	if(max_call_duration > 0)
	{
		if((callid = malloc(CALL_ID_LEN)))
		{
			memcpy(callid, callHandle->callID, CALL_ID_LEN);

			bzero(&timerval, sizeof(struct itimerval));

			timerval.it_value.tv_sec = max_call_duration;;

			callHandle->max_call_duration_tid = timerAddToList(&localConfig.timerPrivate,
																	&timerval,
																	0,
																	PSOS_TIMER_REL,
																	"MAX_CALL_DURATION",
																	disconnectCallAtMaxCallDuration,
																	callid);
		}
		else
		{
			NETERROR(MH323, ("%s unable to malloc callid\n", fn));
		}
	}

	CacheReleaseLocks(callCache);

	BillCall(&callHandleBlk, CDR_CALLSETUP);	

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeSetup;

	if(callHandleBlk.fastStart)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Copying Fast start data\n", fn));
		pH323Data->localSet = (RTPSet *)malloc(H323nlocalset(&callHandleBlk)*sizeof(RTPSet));
		memset(pH323Data->localSet,0,H323nlocalset(&callHandleBlk)*sizeof(RTPSet));
		copyH323Rtp2RtpSet(pH323Data->localSet,H323localSet(&callHandleBlk),H323nlocalset(&callHandleBlk));
		pH323Data->nlocalset = H323nlocalset(&callHandleBlk);
#if 0 // No longer needed, peer might free it
		if(cmCallGetParam(H323hsCall(callHandle),
			cmParamSetupFastStart,
			0,
			&pH323Data->nodeId, NULL) < 0)
		{
			NETERROR(MH323,
				("%s Error is extracting fast start\n", fn));
		}
#endif
	}

	pH323Data->h245ip = rra;
	pH323Data->h245port = rrp;

	if(strlen(H323callingPartyNumber(callHandle)))
	{
		strcpy(pH323Data->callingpn,H323callingPartyNumber(&callHandleBlk));
	}

	// copy the src signaling information
	pH323Data->srcsigip = callHandle->phonode.ipaddress.l;
	pH323Data->srcsigport = H323callsigport(callHandle);

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandleBlk.confID,CONF_ID_LEN);

	strcpy(pH323Data->calledpn,callHandleBlk.rfphonode.phone);

	// Fill in the h.323 conf ID in the data
	len = CONF_ID_LEN;
	if (cmCallGetParam(H323hsCall(callHandle), 
		 cmParamCID, 
		 0, 
		 &len,
		 confID) < 0)
	{
		NETERROR(MH323, ("%s Could not get conf ID id\n", fn));
		pH323Data->confID = NULL;
	}
	else
	{
		pH323Data->confID = (char *)malloc(CONF_ID_LEN);
		memcpy(pH323Data->confID, confID, CONF_ID_LEN);
	}

	pH323Data->displaylen = sizeof(q931display) - 1;
	if (cmCallGetParam(H323hsCall(callHandle), 
		 cmParamDisplay, 
		 0, 
		 &pH323Data->displaylen,
		 q931display) < 0)
	{
		NETERROR(MH323, ("%s Could not get Q931 Display IE\n", fn));
		pH323Data->display= NULL;
		pH323Data->displaylen= 0;
	}
	else
	{
		pH323Data->display = (char *)malloc(sizeof(q931display));
		nx_strlcpy(pH323Data->display, q931display, sizeof(q931display));
		pH323Data->displaylen = strlen(q931display);
	}


	if(propNodeId  >=0)
	{
		if ((cpnNodeId = pvtGetNodeIdByPath(hVal,
				propNodeId,
				"setup.message.setup.callingPartyNumber")) > 0)
		{
            clirData = &(pH323Data->clirData);
		    if (pvtGetByPath(hVal, cpnNodeId,
			        "octet3.presentationIndicator", NULL,
				    (INT32 *)&cpnData, &isString) < 0)
		    {
			    NETDEBUG(MH323, NETLOG_DEBUG4,
				    ("%s could not get presentationIndicator \n",fn));
	    	}
            else
            {
                clirData->data.presentationIndicator = (unsigned char) cpnData;
                clirData->data.flags |= PR_INDICATOR_SET;
            }
		    if (pvtGetByPath(hVal, cpnNodeId,
			        "octet3.screeningIndicator", NULL,
				    (INT32 *)&cpnData, &isString) < 0)
		    {
			    NETDEBUG(MH323, NETLOG_DEBUG4,
				    ("%s could not get screeningIndicator \n",fn));
	    	}
            else
            {
                clirData->data.screeningIndicator = (unsigned char) cpnData;
                clirData->data.flags |= SCR_INDICATOR_SET;
            }
		}
    }

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
		callError = SCC_errorGeneral;
		goto _error;
	}

	H323FreeEvent(evtPtr);
	return 0;

_error:
	H323FreeEvent(evtPtr);
	GkCallDropReasonNone(H323hsCall(&callHandleBlk), callError);
	cmCallDrop(H323hsCall(&callHandleBlk));
	return 0;
}


/*
	- release the call
	- the handles will be freed when we get call state idle
	- received ReleaseComp from network
*/
int SCCNetworkReleaseComp(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkReleaseComp";
	H323EventData		*pH323Data;
	SCC_EventBlock 		*pNextEvt;
	CallHandle 			*callHandle;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));
	
	pH323Data = (H323EventData *)malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data =  (void *) pH323Data;

	memcpy(&pNextEvt->callDetails, &evtPtr->callDetails,
		sizeof(CallDetails));

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

	pNextEvt->event = SCC_eBridgeReleaseComp;

	// Release resources associated with this call leg

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	callHandle = CacheGet(callCache, evtPtr->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		goto _error;
	}

	pNextEvt->callDetails.lastEvent = callHandle->lastEvent;

	if (callHandle->flags & FL_CALL_TAP)
	{
		CallTap(callHandle, 0);
	}

	CloseFceHoles(callHandle);

	//if (callHandle->callSource == 0)
	{
		timedef_cur(&callHandle->callEndTime);

		// mark the call as billed
		GkCallSetRemoteErrorCodes(callHandle, &evtPtr->callDetails);

		BillCall(callHandle, CDR_CALLDROPPED);	
	}

_error:
	CacheReleaseLocks(callCache);
	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventPorcessor failed\n",fn));
	}

	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int SCCLeg1WORReleaseComp(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCLeg1WORReleaseComp() : ";
	CallHandle 			*callHandle = NULL;
	char				callIDStr[CALL_ID_LEN];
	char				callID2[CALL_ID_LEN] = {0};
	char				confID[CONF_ID_LEN] = { 0 };
	H323EventData		*pH323Data;
	SCC_EventBlock 		*pNextEvt;
	HCALL			hcall;
	int			mediaResponseSent = 0, willHunt = 0;
	int			i, len;
	header_url_list *list_entry, *new_list_entry = NULL;
	header_url *contact_url = NULL;
	unsigned short  tmpcap = 0;
	H323EventData*          tmpH323Data = NULL;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = CacheGet(callCache, evtPtr->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		goto _error;
	}

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s callError = %d\n",
		fn, evtPtr->callDetails.callError));

	if (callHandle->flags & FL_CALL_TAP)
	{
		CallTap(callHandle, 0);
	}

	// Check to see if we are logging any CDRs,
	// as this will be an expensive operation o/w
	timedef_cur(&callHandle->callEndTime);
	
	// If any channels have been opened, or acked by us.
	for(i = 0; i<MAX_LOGICAL_CHAN;++i)
	{
		if ((callHandle->fastStart &&
			(H323localSet(callHandle)[0].index == -1)) ||
			H323outChan(callHandle)[i].hsChan ||
			H323inChan(callHandle)[i].hsChan)
		{
			mediaResponseSent = 1;
			break;
		}
	}

	pH323Data = (H323EventData*)evtPtr->data;

	if(pH323Data && pH323Data->remotecontact_list)
	{
		list_entry = pH323Data->remotecontact_list->prev;

		do
		{
			contact_url = list_entry->url;

			new_list_entry = MMalloc (sipCallCache->malloc, sizeof(header_url_list));
			new_list_entry->url = UrlDup(contact_url, sipCallCache->malloc);
			if (new_list_entry->url)
			{
				new_list_entry->url->type = HEADER_ADDRESS_SIP;
			}

			if(callHandle->remotecontact_list)
			{
				ListInsert(callHandle->remotecontact_list->prev, new_list_entry);
				callHandle->remotecontact_list = new_list_entry;
			}
			else
			{
				ListInitElem(new_list_entry);
				callHandle->remotecontact_list = new_list_entry;
			}
			if (new_list_entry->url)
			{
				new_list_entry->url->type = HEADER_ADDRESS_SIP;
			}

			list_entry = list_entry->prev;
		}
		while(list_entry != pH323Data->remotecontact_list->prev);
	}

	// Hunting logic
	willHunt = !mediaResponseSent &&
		(max_hunt_allowable_duration == 0 ||
			difftime(time(NULL), timedef_sec(&callHandle->callStartTime)) < max_hunt_allowable_duration) &&
		(evtPtr->callDetails.flags&REDIRECT ||
		(callHandle->nhunts < callHandle->maxHunts) &&
		BIT_TEST(callHandle->rfphonode.cap, CAP_IGATEWAY) &&
		((evtPtr->callDetails.flags&HUNT_TRIGGER) ||
        h323HuntError(evtPtr->callDetails.callError, 
		evtPtr->callDetails.cause - 1)));

	/* make sure we have cause code and h225 reason */
	if ( evtPtr->callDetails.callError )
	{
		if ( evtPtr->callDetails.cause <= 0)
		{
			evtPtr->callDetails.cause = GkMapCallErrorToCauseCode( evtPtr->callDetails.callError );
		}
		if ( evtPtr->callDetails.h225Reason <= 0)
		{
			GkMapCallErrorToH225nRasReason( evtPtr->callDetails.callError, &(evtPtr->callDetails.h225Reason), NULL );
		}
	}
	// mark the call as billed
	GkCallSetRemoteErrorCodes(callHandle, &evtPtr->callDetails);

	if ((callHandle->ecaps1 & ECAPS1_MAPISDNCC) || mapisdncc)
	{
		callHandle->cause = GkMapCauseCode(callHandle->cause);
	}
	memcpy(&callHandle->callDetails2, &evtPtr->callDetails,
		sizeof(CallDetails));

	// If we are not hunting generate a CDR
	if (!willHunt)
	{
		BillCall(callHandle, CDR_CALLDROPPED);
	}
	else 
	{
		BillCall(callHandle, CDR_CALLHUNT);
	}

	/* a RELEASECOMPLETE came in, close all holes for the current leg */
	CloseFceHoles(callHandle);


	/* retry call only if failed 1st resolution - not subsequent */
	if (willHunt)
	{
		char 			temp_regid[REG_ID_LEN];
		unsigned long	uport;
		strcpy(temp_regid, callHandle->rfphonode.regid);
		uport = callHandle->rfphonode.uport;
		BIT_COPY(tmpcap, CAP_IGATEWAY, callHandle->rfphonode.cap, CAP_IGATEWAY);
		memset(&callHandle->callDetails2, 0, sizeof(CallDetails));

		// Push the current dest into the callHandle list
		GwAddPhoNodeToRejectList(&callHandle->rfphonode, callHandle->crname,
			&callHandle->destRejectList, CMalloc(callCache));
		
		// Also see if we have any channels open with the src.
		// If so, close them. We may have to close
		// certain holes also
		for(i = 0; i<MAX_LOGICAL_CHAN;++i)
		{
			if (H323outChan(callHandle)[i].hsChan)
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Dropping out channel, for hunting\n", fn));
				cmChannelDrop(H323outChan(callHandle)[i].hsChan);
				memset(&(H323outChan(callHandle)[i]), 0, sizeof(ChanInfo));
			}

			if (H323inChan(callHandle)[i].hsChan)
			{
				NETDEBUG(MH323, NETLOG_DEBUG4,
					("%s Dropping in channel, for hunting\n", fn));
				cmChannelDrop(H323inChan(callHandle)[i].hsChan);
				memset(&(H323inChan(callHandle)[i]), 0, sizeof(ChanInfo));
			}
		}

		if(!(evtPtr->callDetails.flags & REDIRECT))
		{
			// Reset the dest information in the phonode
			memset(&callHandle->rfphonode, 0, sizeof(PhoNode));
			strcpy(callHandle->rfphonode.phone, CallInputNumber(callHandle));
			callHandle->rfphonode.cap = tmpcap;
		}

		// Re-Create...
		GkCallResetRemoteErrorCodes(callHandle);
		callHandle->state = SCC_sWaitOnRemote;

		getPeerCallID(evtPtr->confID,evtPtr->callID,callID2);
		GisDeleteCallFromConf(callID2, evtPtr->confID);

		pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
		memset(pNextEvt,0,sizeof(SCC_EventBlock));

		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
		memset(pH323Data,0,sizeof(H323EventData));

		pNextEvt->data = pH323Data;

		if(evtPtr->callDetails.flags&REDIRECT)
		{
			pH323Data->requri = ((H323EventData*)evtPtr->data)->requri;
			((H323EventData*)evtPtr->data)->requri = NULL;
		}
		else if(callHandle->remotecontact_list)
		{
			contact_url = SipPopUrlFromContactList(&callHandle->remotecontact_list, sipCallCache->free);
			pH323Data->requri = UrlDup(contact_url, MEM_LOCAL);
			UrlFree(contact_url, sipCallCache->free);

			memset(&callHandle->rfphonode, 0, sizeof(PhoNode));
			nx_strlcpy(callHandle->rfphonode.phone, pH323Data->requri->name,
							sizeof(callHandle->rfphonode.phone));
			strcpy(callHandle->rfphonode.regid, temp_regid );
			callHandle->rfphonode.uport = uport;
			callHandle->rfphonode.cap = tmpcap;
		}

		if(callHandle->fastStart)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s Copying Fast start data\n", fn));
			pH323Data->localSet = (RTPSet *)malloc(H323nlocalset(callHandle)*sizeof(RTPSet));
			memset(pH323Data->localSet,0,H323nlocalset(callHandle)*sizeof(RTPSet));
			copyH323Rtp2RtpSet(pH323Data->localSet,H323localSet(callHandle),
				H323nlocalset(callHandle));
			pH323Data->nlocalset = H323nlocalset(callHandle);
		}
		pH323Data->h245ip = H323h245ip(callHandle);

		pH323Data->h245port = H323h245port(callHandle);
		if(strlen(H323callingPartyNumber(callHandle)))
		{
			strcpy(pH323Data->callingpn,H323callingPartyNumber(callHandle));
		}

		memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
		memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
		memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);
		
		tmpH323Data  = (H323EventData*)evtPtr->data;
		if(tmpH323Data) 
		{
			if( (tmpH323Data->displaylen > 0) && tmpH323Data->display )
			{
				pH323Data->display      = strdup(tmpH323Data->display);
				pH323Data->displaylen   = tmpH323Data->displaylen;
			}
		}

		strcpy(pH323Data->calledpn,callHandle->rfphonode.phone);
		pNextEvt->event = SCC_eBridgeSetup;
		hcall = H323hsCall(callHandle);

		// Fill in the h.323 conf ID in the data
		len = CONF_ID_LEN;
		if (cmCallGetParam(H323hsCall(callHandle), 
			 cmParamCID, 
			 0, 
			 &len,
			 confID) < 0)
		{
			NETERROR(MH323, ("%s Could not get conf ID id\n", fn));
			pH323Data->confID = NULL;
		}
		else
		{
			pH323Data->confID = (char *)malloc(CONF_ID_LEN);
			memcpy(pH323Data->confID, confID, CONF_ID_LEN);
		}

		if(bridgeH323EventProcessor(pNextEvt)!=0)
		{
			NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
			callHandle->callError = SCC_errorGeneral;
			callHandle->lastEvent = SCC_evtHunt;
			goto _error;
		}

		CacheReleaseLocks(callCache);
		H323FreeEvent(evtPtr);
		return 0;
	}

	GkCallDropReasonSig(H323hsCall(callHandle),
			callHandle->callError, callHandle->h225Reason, callHandle->cause);

	if(cmCallDrop(H323hsCall(callHandle)) <0)
	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s cmCallDrop failed for %p\n",
			fn,H323hsCall(callHandle)));

		SCC_CallStateIdle(evtPtr);
	} else {
		NETDEBUG(MSCC,NETLOG_DEBUG1,("%s cmCallDrop success for %p\n",
			fn,H323hsCall(callHandle)));
	}


_error :
	H323FreeEvent(evtPtr);
	CacheReleaseLocks(callCache);
	return 0;
	
}

/*****************************LEG 2 Action Routines************************/
int SCCInitiateReleaseComp(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCInitiateReleaseComp() : ";
	CallHandle 			*callHandle, *callHandle2 = NULL;
	char				callIDStr[CALL_ID_LEN];
	H323EventData		*pH323Data;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	callHandle = CacheGet(callCache, evtPtr->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		goto _error;
	}

	if (callHandle->flags & FL_CALL_TAP)
	{
		CallTap(callHandle, 0);
	}

	// Check to see if we are logging any CDRs,
	// as this will be an expensive operation o/w
	timedef_cur(&callHandle->callEndTime);

	// mark the call as billed
	GkCallSetRemoteErrorCodes(callHandle, &evtPtr->callDetails);
	if ((callHandle->ecaps1 & ECAPS1_MAPISDNCC) || mapisdncc)
	{
		callHandle->cause = GkMapCauseCode(callHandle->cause);
	}
	memcpy(&callHandle->callDetails2, &evtPtr->callDetails, 
			sizeof(CallDetails));
	BillCall(callHandle, CDR_CALLDROPPED);	

	/* a RELEASECOMPLETE came in, close all holes for the current leg */
	CloseFceHoles(callHandle);

	GkCallDropReasonSig(H323hsCall(callHandle),
				callHandle->callError, callHandle->h225Reason, callHandle->cause);


	if(cmCallDrop(H323hsCall(callHandle)) <0)
	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s cmCallDrop failed for %p\n",
			fn,H323hsCall(callHandle)));

		SCC_CallStateIdle(evtPtr);
	} else {
		NETDEBUG(MSCC,NETLOG_DEBUG1,("%s cmCallDrop success for %p\n",
			fn,H323hsCall(callHandle)));
	}

_error :
	H323FreeEvent(evtPtr);
	CacheReleaseLocks(callCache);
	return 0;
	
}

// ANY ERROR which happens in this function, before a stack call is
// created must be able to cause a port release. 
// The assumption that this function alwas returns success (??)
// allows the cleanup to happen in InitiateReleaseComp
// in the WaitOnRemote state. If the callDrop there
// is unsuccessful, the port MUST be released manually

int SCCLeg2BridgeCreateCall(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCLeg2BridgeCreateCall() : ";
	CallHandle 			callHandleBlk2 = {0},*callHandle2;
	UH323CallAppHandle 	*appHandle2; // Call Handle for leg2
	cmTransportAddress 	cmTransAddr = { 0, 0, 1720, cmTransportTypeIP };
	int					len;
	int 				rc;
	HCALL				hcall = (HCALL)NULL;
	char 				callIdStr[CALL_ID_LEN] = {0};
	char 				confID[CONF_ID_LEN] = {0}, callID[CALL_ID_LEN] = { 0 };
	H323EventData		*pH323Data,*pNextData;
	SCC_EventBlock		*pNextEvt;
	int					callError = SCC_errorNone;
	ConfHandle 			*confHandle;
	char 				zero[GUID_LEN] = {0};

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	//extract parameters to be sent to source

	pH323Data = (H323EventData *)(evtPtr->data);
	if(!(appHandle2 = uh323CallAllocAppHandle()))
	{
		NETERROR(MSCC,("%s Unable to allocate appHandle\n",fn));
		callError = SCC_errorResourceUnavailable;
		goto _error;
	}

	// COPY these here FIRST...
	memcpy(appHandle2->confID,evtPtr->confID,CONF_ID_LEN);
	memcpy(appHandle2->callID,evtPtr->callID,CALL_ID_LEN);

	// Check if this instance has enough maxcalls resources
	// e.g. At minimum 100 max calls or 20% of configured maxcalls should
	// be left
	if (((nh323Instances == 1) &&
			(2*UH323Globals()->nCalls + h323maxCallsPadFixed > UH323Globals()->maxCalls) &&
			(2*UH323Globals()->nCalls*100 > h323maxCallsPadVariable*UH323Globals()->maxCalls)) ||
		((nh323Instances > 1) &&
			(UH323Globals()->nCalls + h323maxCallsPadFixed > UH323Globals()->maxCalls) &&
			(UH323Globals()->nCalls*100 > h323maxCallsPadVariable*UH323Globals()->maxCalls)))
	{
		NETERROR(MSCC, ("%s cmCallNew check failed ncalls=%d, xcalls = %d\n",
			fn, UH323Globals()->nCalls, UH323Globals()->maxCalls));

		uh323CallFreeAppHandle(appHandle2);
		callError = SCC_errorH323MaxCalls;

		SCC_CallStateIdle(evtPtr);

		goto _dropsrc;
	}

	rc = cmCallNew(UH323Globals()->hApp,(HAPPCALL)appHandle2,&hcall);
	if(rc <0)
	{
		NETERROR(MSCC, ("%s cmCallNew return error = %d, ncalls=%d, xcalls = %d\n",
			fn,rc, UH323Globals()->nCalls, UH323Globals()->maxCalls));

		uh323CallFreeAppHandle(appHandle2);
		callError = SCC_errorH323Internal;

		SCC_CallStateIdle(evtPtr);

		goto _dropsrc;
	}

	if (nh323Instances > 1)
	{
		UH323Globals()->nCalls++;
	}

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle2 = CacheGet(callCache, evtPtr->callID)) )
	{
		CacheReleaseLocks(callCache);
		NETERROR(MH323, ("%s Could not find call handle\n", fn));
		uh323CallFreeAppHandle(appHandle2);
		callError = SCC_errorNoCallHandle;
		goto _error;
	}

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(confHandle = CacheGet(confCache, evtPtr->confID)) )
	{
		CacheReleaseLocks(confCache);
		CacheReleaseLocks(callCache);
		NETERROR(MH323, ("%s Could not find conf handle\n", fn));
		uh323CallFreeAppHandle(appHandle2);
		callError = SCC_errorNoCallHandle;
		goto _error;
	}

	// We are the origin of the call
	callHandle2->callSource=1;
	timedef_cur(&callHandle2->callStartTime);

	H323appHandle(callHandle2) = appHandle2;
	H323callingPartyNumberLen(callHandle2) = strlen(pH323Data->callingpn);
	
	strcpy(H323callingPartyNumber(callHandle2),pH323Data->callingpn);
	callHandle2->leg = SCC_CallLeg2;
	callHandle2->networkEventProcessor = &SCC_H323Leg2NetworkEventProcessor;
	strcpy(H323dialledNumber(callHandle2),pH323Data->calledpn);
	H323hsCall(callHandle2) = hcall;

	// Pass the egress ACF/LCF token node ID to leg 2
	if (confHandle->egressTokenNodeId > 0)
	{
		H323egressTokenNodeId(callHandle2) = confHandle->egressTokenNodeId;
	}

	// Pass the egress H323 ID to leg 2
	if (confHandle->egressH323Id[0] != '\0')
	{
		nx_strlcpy(H323h323Id(callHandle2), confHandle->egressH323Id, H323ID_LEN);
	}

	if (pH323Data->confID != NULL)
	{
		// Set the conf ID for H323-H323 calls.
		// For an H323-H323 call, the confID is passed from leg1 to leg2
    	// through pH323Data.
    	if( cmCallSetParam(hcall, cmParamCID, 0, GUID_LEN,
         	(char *)pH323Data->confID) < 0)
    	{
        	NETERROR(MH323, ("%s Could not set conf id for call\n", fn));
			// The stack should use its own ID !!
    	}
	}
	else
	{
		// Set the conf ID for SIP-H323 calls.
		// For a SIP-H323 call, the confID is set by the stack for the 
		// first hunt attempt. This confID is stored in the conf handle 
		// and is used in subsequent hunt attempts for that call.
		if (memcmp(confHandle->h323ConfId, zero, GUID_LEN) != 0)
		{
    		if( cmCallSetParam(hcall, cmParamCID, 0, GUID_LEN,
         		(char *)confHandle->h323ConfId) < 0)
    		{
        		NETERROR(MH323, ("%s Could not set conf id for call\n", fn));
				// The stack should use its own ID !!
    		}
		}
	}

	// Set the CID in the call handle for leg2. 
	// Store the CID in the conf handle so that it can be used 
	// in leg2 call handles for subsequent hunt attempts.
	len = CONF_ID_LEN;
	if (cmCallGetParam(hcall, 
		 cmParamCID, 
		 0, 
		 &len,
		 callID) < 0)
	{
		NETERROR(MH323, 
		("%s Could not get CID id\n", fn));
	}
	else
	{
		callHandle2->conf_id = CMalloc(callCache)(CONF_ID_STRLEN);
		ConfID2String2(callID, callHandle2->conf_id);
		if (memcmp(confHandle->h323ConfId, zero, GUID_LEN) == 0)
		{
			memcpy(confHandle->h323ConfId, callID, GUID_LEN);
		}
	}

	CacheReleaseLocks(confCache);

	if(!(callHandle2->ecaps1&ECAPS1_NOH323DISPLAY))
	{
		if(pH323Data->displaylen >0)
		{
			if(cmCallSetParam(hcall,
					cmParamDisplay,
					0,
					pH323Data->displaylen,
					pH323Data->display) <0)
			{
				NETERROR(MSCC,("%s cmCallSetParam display failed\n",fn));
			}
			
		}
		else if (strlen(pH323Data->callingpn))
		{
			if(cmCallSetParam(hcall,
					cmParamDisplay,
					0,
					strlen(pH323Data->callingpn),
					pH323Data->callingpn) <0)
			{
				NETERROR(MSCC,("%s cmCallSetParam display failed for cpn\n",fn));
			}
		}
		else {
				if(cmCallSetParam(hcall,
								cmParamDisplay,
								0,
								strlen("NexTone"),
								"NexTone") <0)
				{
						NETERROR(MSCC,("%s cmCallSetParam display failed for cpn\n",fn));
				}
		}
	}

	if(routeH245 == 0)
	{
	// if transparently passing h245
		cmTransAddr.ip = pH323Data->h245ip;
		cmTransAddr.port = pH323Data->h245port; 
		len = sizeof(cmTransportAddress);
		if((cmTransAddr.ip !=0) && (cmCallSetParam(hcall,
				cmParamSetupH245Address,
				-1,
				len,
				(char *)&cmTransAddr) <0))
		{
			NETERROR(MSCC,
				("%s Unable to set cmParamSetupH245Address %p\n", fn,hcall));
			callError = SCC_errorH323Internal;
			CacheReleaseLocks(callCache);
			goto _error;
		}
	}
	if ((pH323Data->nlocalset > 0) && (doFastStart) && !IsRolledOver(callHandle2->dialledNumber))
	{
		callHandle2->fastStart = 1;
	}

	// Respond to Open logical Channel unless we get a fastconnect response 
	H323doOlc(callHandle2) = 1;

	if(pH323Data->nlocalset >0)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Copying Fast start data\n", fn));
		H323remoteSet(callHandle2) = (H323RTPSet *)(CMalloc(callCache))(pH323Data->nlocalset*sizeof(H323RTPSet));
		memset(H323remoteSet(callHandle2), 0, pH323Data->nlocalset*sizeof(H323RTPSet));
		copyRtp2H323RtpSet(H323remoteSet(callHandle2),pH323Data->localSet,pH323Data->nlocalset);
		H323nremoteset(callHandle2)	 = pH323Data->nlocalset; 
	}

	// set up the call_id
	len = CALL_ID_LEN;
	if (cmCallGetParam(hcall, 
		 cmParamCallID, 
		 0, 
		 &len,
		 callID) < 0)
	{
		NETERROR(MH323, 
		("%s Could not get Call id\n", fn));
	}
	else
	{
		memcpy(callHandle2->handle.h323CallHandle.guid,callID,GUID_LEN);
		CacheInsert(guidCache,callHandle2);
	}
	CacheReleaseLocks(callCache);
	NETDEBUG(MSCC,NETLOG_DEBUG2,
		("%s - Initiatialized CallHandle %s",
			fn, (char*) CallID2String(callHandle2->callID,callIdStr)));

	// LOCKS HAVE BEEN RELEASED !!

	/* If we have to send an ARQ, then we will send it, otherwise,
	 * we will just send the setup directly
	 */
	if (BIT_TEST(callHandle2->phonode.cap, CAP_ARQ))
	{
		
		callHandle2->lastEvent = SCC_evt323ARQTx;
		//BillCall(callHandle2, CDR_CALLSETUP);	

		if (GkSendARQ(&callHandle2->phonode, callHandle2,evtPtr, 0) >= 0)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG2,
				("%s Sent the ARQ\n", fn));
			return 0;
		}
	}

	if (BIT_TEST(callHandle2->phonode.cap, CAP_LRQ))
	{
		callHandle2->lastEvent = SCC_evt323LRQTx;
		//BillCall(callHandle2, CDR_CALLSETUP);	

		if (GkSendLRQ(&callHandle2->phonode, callHandle2,evtPtr) >= 0)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG2,
				("%s Sent the LRQ\n", fn));
			return 0;
		}
	}

//TimeStamp(MINIT, NETLOG_DEBUG1, "leg2bcc 1");
	return SCCLeg2BridgeMakeCall(evtPtr);

_dropsrc :
	// send an event to other leg to drop the call
	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pNextData = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pNextData,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pNextData;
	pNextEvt->event = SCC_eBridgeReleaseComp;

	pNextEvt->callDetails.callError = callError;
	pNextEvt->callDetails.lastEvent = SCC_evt323NewCall;

	memcpy(pNextData->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	}
_error:
	if(hcall != 0) 
	{
		GkCallDropReasonNone(hcall, callError);
		cmCallDrop(hcall);
	}
	H323FreeEvent(evtPtr);
	return 0;
}

/* 	This function drops the call in case of error
*/
int SCCLeg2BridgeMakeCall(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCLeg2BridgeMakeCall() : ";
	CallHandle 			callHandleBlk = {0}, *callHandle;
	ConfHandle			*confHandle = NULL;
	cmTransportAddress 	cmTransAddr = { 0, 0, 1720, cmTransportTypeIP };
	int					len;
	int 				rc;
	static char 		srcAddress[254], dstAddress[254], ipStr[20];
	static char 		userUserStr[128] = "Nextone iServer Proxy";
	static char 		displayStr[128] = "Nextone iServer Proxy";
	struct 				itimerval rollovertmr;
	H323EventData 		*pH323Data;
	char 				callIDStr[CALL_ID_LEN] = {0};
	int					callError = SCC_errorNone;
	int					nodeId = -1,tokenNodeId = -1,setupNodeId = -1,bcsNodeId = -1;
    HPVT                hVal = cmGetValTree(UH323Globals()->hApp);
	cmAlias				srcAlias, destAlias;
	HPVT				peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	int					srcindex = 0;
	BYTE 				h323_id[256];
	HCALL				hcall;
	int					tmpInfoTransCap;
	unsigned long 		localIP = 0;
	UH323CallAppHandle 	*appHandle;
	cmH245Stage			h245Stage;
	int                 cpnNodeId = -1, cpnData = 0;
	CLIRData            *clirData = NULL;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	if(!(pH323Data = evtPtr->data))
	{
		NETERROR(MSCC,("%s Null H323 Data\n", fn ));
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	callHandle = CacheGet(callCache,evtPtr->callID);

	if(callHandle == NULL)
	{
		NETERROR(MSCC,("%s Could not find call handle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		callError = SCC_errorNoCallHandle;
		goto _error;
	}
	
#if 0
	if (cmCallSetParam(H323hsCall(&callHandleBlk),
			 cmParamEstablishH245, 
			 0, 
			 routeH245,
			 NULL) < 0)
	{
		NETERROR(MH323, 
			 ("%s Could not set h245 off \n", fn));
		goto _error;
	}
#endif
	hcall = H323hsCall(callHandle);
	cmTransAddr.length = 4;
	callHandle->peerIp = cmTransAddr.ip = pH323Data->destip;
	callHandle->peerPort = cmTransAddr.port = pH323Data->destport;

	// We are the origin of the call
	callHandle->callSource=1;

#if 0
	if ((routeH245 == 0) && callHandle->fastStart &&
			(pH323Data->nodeId > 0))
	{
		// set the fast start node id
		if (cmCallSetParam(H323hsCall(callHandle),
			cmParamSetupFastStart,
			0, pH323Data->nodeId, NULL) < 0)
		{
			NETERROR(MH323, ("%s Could not set fast start param\n", fn));
		}

		callHandle->fastStart = 0;
	}
#endif

	if(pH323Data->nlocalset<=0)
	{
		if (cmCallSetParam(H323hsCall(callHandle),
				 cmParamH245Tunneling, 
				 0, 
				 0,
				 NULL) < 0)
		{
			NETERROR(MH323, 
				 ("%s Could not set h245 Tunnelling off \n", fn));
			CacheReleaseLocks(callCache);
			goto _error;
		}
	}

	h245Stage = cmTransH245Stage_connect;
	if (cmCallSetParam(H323hsCall(callHandle),
		cmParamH245Stage,
		0,
		h245Stage,
		NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set H245Stage to %d\n", fn,h245Stage));
	}
	callHandle->lastEvent = SCC_evt323SetupTx;


	// set up the tg
	cmMeiEnter(UH323Globals()->hApp);
	
	if ((nodeId = cmGetProperty((HPROTOCOL)H323hsCall(callHandle))) >0)
	{
		if (callHandle->tg)
		{
        	if (pvtBuildByPath(hVal, nodeId,
            	"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.sourceCircuitID.group.group", 
				strlen(callHandle->tg), callHandle->tg) < 0)
        	{
           	 	NETERROR(MH323, ("%s could not build trunk group %s %p\n", fn, callHandle->tg,hcall));
        	}
		}
       	
		if (callHandle->destTg && callHandle->destTg[0] && (callHandle->ecaps1 & ECAPS1_SETDESTTG))
		{
	       	if (pvtBuildByPath(hVal, nodeId,
	           	"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.destinationCircuitID.group.group", 
				strlen(callHandle->destTg), callHandle->destTg) < 0)
	       	{
	       	 	NETERROR(MH323, ("%s could not build dest trunk group %s %p\n", fn, callHandle->destTg,hcall));
	       	}
		}
    

        if (nodeId >0)
        {
            clirData = &(pH323Data->clirData);
	        if ((pH323Data->callingpn[0]!=0) &&
                ((clirData->data.flags & PR_INDICATOR_SET) || (clirData->data.flags & SCR_INDICATOR_SET)))
            {
                
        		if ((cpnNodeId = pvtBuildByPath(hVal,
				        nodeId,
            			"setup.message.setup.callingPartyNumber", 0 , NULL)) > 0)
		        {
                    if (clirData->data.flags & PR_INDICATOR_SET)
                    {
                        cpnData = clirData->data.presentationIndicator;   
		        		if (pvtBuildByPath(hVal, cpnNodeId, "octet3.presentationIndicator", cpnData,NULL) < 0)
        				{
		        			NETERROR(MH323, ("%s could not build presentationIndicator %s \n", fn));
				        }
                    }
                    cpnData = 0;
                    if (clirData->data.flags & SCR_INDICATOR_SET)
                    {
                        cpnData = clirData->data.screeningIndicator;   
		        		if (pvtBuildByPath(hVal, cpnNodeId, "octet3.screeningIndicator", cpnData,NULL) < 0)
        				{
		        			NETERROR(MH323, ("%s could not build screeningIndicator %s \n", fn));
        				}
                    }
        		}
            }
        }

		if(!(callHandle->ecaps1&ECAPS1_NOH323DISPLAY) && pH323Data->displaylen >0 ) 
		{
			if(strcasecmp(pH323Data->display,"Anonymous")==0)
			{
				if ((cpnNodeId = pvtGetNodeIdByPath(hVal,
					nodeId,
					"setup.message.setup.callingPartyNumber")) > 0)
				{
					rc= pvtBuildByPath(hVal, cpnNodeId, "octet3.presentationIndicator", 1,NULL);	
					if ( rc  < 0)
					{
						NETERROR(MH323, ("%s could not build presentationIndicator \n", fn));
					}
				}
			}
		}

	}
	else {
		NETERROR(MH323,("%s failed to get property nodeId\n", fn));
	}
	
	cmMeiExit(UH323Globals()->hApp);

	memcpy(&callHandleBlk, callHandle, sizeof(CallHandle));

	ST_outSetup();
	if (cmCallGetHandle(H323hsCall(callHandle), (HAPPCALL *)&appHandle) < 0)
	{
		NETERROR(MSCC,("%s Cannot locate app handle for this call\n", fn));
	}
	else
	{
		if(!callHandle->realmInfo)
		{
			NETERROR(MSCC,("%s no realm info present on call %p\n", fn,hcall));
			callError = SCC_errorH323Internal;
			CacheReleaseLocks(callCache);
			goto _error;
		} 
		appHandle->localIP = (callHandle->realmInfo->rsa);
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s rsa = %s\n",fn,FormatIpAddress(callHandle->realmInfo->rsa,ipStr)));
		cmTransAddr.type = cmTransportTypeIP;
		cmTransAddr.ip = htonl (callHandle->realmInfo->rsa);
		cmTransAddr.port = 1720;
		len = sizeof(cmTransportAddress);
		if (cmCallSetParam(H323hsCall(callHandle), cmParamSrcCallSignalAddress, -1, len, (char *)&cmTransAddr) <0)
		{
			NETERROR(MH323, 
				("%s Unable to set cmParamSrcCallSignalAddress %p\n", fn,hcall));
			callError = SCC_errorH323Internal;
			CacheReleaseLocks(callCache);
			goto _error;
		}
	}

	CacheReleaseLocks(callCache);

	sprintf(dstAddress, "TA:%s:%d,TEL:%s",	/* do not follow dialable party number here, the called number is set below */
		liIpToString(htonl(pH323Data->destip),ipStr),
		pH323Data->destport,
		pH323Data->calledpn);

	cmTransAddr.type = cmTransportTypeIP;
	len = sizeof(cmTransportAddress);
	if (cmCallSetParam(H323hsCall(&callHandleBlk),
		cmParamDestCallSignalAddress,
		0,
		len,
		(char *)&cmTransAddr) <0)
	{
		NETERROR(MSCC,("%s Unable to set DestCallSignalAddress %p\n",fn,hcall));
		callError = SCC_errorH323Internal;
		goto _error;
	}

#if 0
    len = CALL_ID_LEN;
    if( cmCallSetParam(H323hsCall(&callHandleBlk),
             cmParamCallID,
         0,
         len,
         (char *)evtPtr->callID) < 0)
    {
        NETERROR(MH323,
             ("%s Could not set call id\n", fn));
		callError = SCC_errorH323Internal;
        goto _error;
    }
#endif

	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);

	if((nodeId = cmGetProperty((HPROTOCOL)H323hsCall(&callHandleBlk))) >0)
	{
		int layer1;

		// Get Setup Q931 Node ID from conf handle
		CacheGetLocks(confCache, LOCK_READ, LOCK_BLOCK);
		if (!(confHandle = CacheGet(confCache, evtPtr->confID)) )
		{
			NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
				fn, (char*) CallID2String(evtPtr->confID,callIDStr)));
			CacheReleaseLocks(confCache);
			cmMeiExit(UH323Globals()->peerhApp);
			cmMeiExit(UH323Globals()->hApp);
			callError = SCC_errorH323Internal;
			goto _error;
		}

		// Set Bearer Capability
		if(confHandle->setupQ931NodeId > 0)
		{
			if ((bcsNodeId = pvtGetNodeIdByPath(hVal,
				nodeId,
				"setup.message.setup.bearerCapability")) > 0)
			{
				if(pvtAddTree(hVal,bcsNodeId,peerhVal,confHandle->setupQ931NodeId) < 0)
				{
					NETERROR(MH323, 
						("%s pvtAddtree to set bearer capability failed %p\n",fn,hcall));
				}
				else if(pvtSetTree(hVal,bcsNodeId,peerhVal,confHandle->setupQ931NodeId) < 0)
				{
					NETERROR(MH323, 
						("%s pvtSettree to set bearer capability failed %p\n",fn,hcall));
				}
			}
			else {
					NETERROR(MH323, ("%s could not get bcsNodeId %p\n",fn,hcall));
			}
		}

		CacheReleaseLocks(confCache);

		layer1 = (H323Callbcap(&callHandleBlk)[BCAP_LAYER1]);
		if (layer1 != BCAPLAYER1_Pass)
		{
			if(layer1 == BCAPLAYER1_Default) 
			{
				layer1 = bcapLayer1Default;
			}
			if(pvtBuildByPath(hVal,nodeId,"setup.message.setup.bearerCapability.octet5.userInformationLayer1Protocol",
				layer1, NULL) < 0)
			{
				NETERROR(MH323, 
					("%s Could Not set userInformationLayer1Protocol %p\n",fn,hcall));
			}
		}
		// Set Tokens if Any
		if((tokenNodeId = H323egressTokenNodeId(&callHandleBlk)) >0)
		{
			if ((setupNodeId = pvtGetNodeIdByPath(hVal,
				nodeId,
				"setup.message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup")) > 0)
			{
				if(pvtAddTree(hVal,setupNodeId,peerhVal,tokenNodeId) < 0)
				{
					NETERROR(MH323, ("%s could not set token in setup %p\n",fn,hcall));
				}
			}
			else {
					NETERROR(MH323, ("%s could not get setupNodeId %p\n",fn,hcall));
			}
		}
    }
	else {
		NETERROR(MH323,("%s : failed to get property nodeId %p\n", fn,hcall));
	}
	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);

	/* If calling party number was present on leg 1 */
	if(pH323Data->callingpn[0]!=0)
	{
		len = sizeof(cmAlias);
		memset(&srcAlias, 0, len);
		srcAlias.string = pH323Data->callingpn;
		srcAlias.length = strlen(pH323Data->callingpn);
		srcAlias.type = cmAliasTypeE164;
		
		if (cmCallSetParam(H323hsCall(&callHandleBlk),
			cmParamSourceAddress,
			srcindex,
			len,
			(char *)&srcAlias) <0)
		{
			NETERROR(MSCC,("%s Unable to set sourceAddress - ANI %p\n",fn,hcall));
			callError = SCC_errorH323Internal;
			goto _error;
		}
		else
		{
			srcindex ++;
		}	

		/* Set Q.931 calling party number and its number type */
		len = sizeof(cmAlias);
		memset(&srcAlias, 0, len);
		srcAlias.string = pH323Data->callingpn;
		srcAlias.length = strlen(pH323Data->callingpn);
		srcAlias.type = cmAliasTypePartyNumber;	
		srcAlias.pnType = callHandleBlk.destCallingPartyNumType;

		if (cmCallSetParam(H323hsCall(&callHandleBlk),
			cmParamCallingPartyNumber,0,len,(char*)&srcAlias)<0)
		{
			NETERROR(MH323,
				 ("%s Could not set cmParamCallingPartyNumber (0x%x)\n",
					fn,hcall));
			callError = SCC_errorH323Internal;
			goto _error;
		}	 
		sprintf(srcAddress,"TEL:%s",pH323Data->callingpn);
	}
	else
	{
		srcAddress[0] = '\0';
	}
	
	tmpInfoTransCap = callHandleBlk.handle.h323CallHandle.infoTransCap;
	/* if it is set to default, get systemwide default value */
	if (tmpInfoTransCap == INFO_TRANSCAP_DEFAULT)
	{
		tmpInfoTransCap = h323infoTransCap;
	}
	if ((tmpInfoTransCap >= INFO_TRANSCAP_SPEECH ) &&
		(tmpInfoTransCap <= INFO_TRANSCAP_VIDEO) )
	{
		if( cmCallSetParam(H323hsCall(&callHandleBlk),
				 cmParamInformationTransferCapability,
				 0,
				 tmpInfoTransCap - INFO_TRANSCAP_BASE,
				 NULL) < 0)
		{
			NETERROR(MH323,
				 ("%s Could not set cmParamInformationTransferCapability(%p)\n",
					fn,hcall));
			callError = SCC_errorH323Internal;
			goto _error;
		}
	}

	/* now set Q.931 called party number and its number type */
	len = sizeof(cmAlias);
	memset(&destAlias, 0, len);
	destAlias.string = pH323Data->calledpn;
	destAlias.length = strlen(pH323Data->calledpn);

	// bridge has already determined this
	destAlias.pnType = callHandleBlk.destCalledPartyNumType;

	destAlias.type = cmAliasTypePartyNumber;	/* has to be cmAliasTypePartyNumber type to enable stack to set typeOfNumber */
	if (cmCallSetParam(H323hsCall(&callHandleBlk),
		cmParamCalledPartyNumber,0,len,(char*)&destAlias)<0)
	{
			NETERROR(MH323,
				 ("%s Could not set cmParamCalledPartyNumber(%p)\n",
					fn,hcall));
			callError = SCC_errorH323Internal;
			goto _error;
	} 

	if (H323h323Id(&callHandleBlk)[0] != '\0')
	{
		len = sizeof(cmAlias);
		memset(&srcAlias, 0, len);
		srcAlias.length = utlChr2Bmp(H323h323Id(&callHandleBlk), &h323_id[0]);
		srcAlias.string = &h323_id[0];
		srcAlias.type = cmAliasTypeH323ID;
		
		if (cmCallSetParam(H323hsCall(&callHandleBlk),
			cmParamSourceAddress,
			srcindex,
			len,
			(char *)&srcAlias) <0)
		{
			NETERROR(MSCC,("%s Unable to set sourceAddress - h323id %p\n",fn,hcall));

			// Not a termination error
		}
		else
		{
			srcindex ++;
		}	
	}

	BillCall(&callHandleBlk, CDR_CALLSETUP);	

	rc = cmCallMake(H323hsCall(&callHandleBlk),
			64000,
			0,
			dstAddress,
			srcAddress,
			/* strlen(display)?display:displayStr */NULL,
			/* userUserStr*/NULL,
			/* sizeof(userUserStr)*/0);

	if(rc <0)
	{
		NETERROR(MSCC, ("%s cmCallMake return error = %d %p\n",
			fn, rc,hcall));
		callError = SCC_errorH323Internal;
		goto _error;
	}


	NETDEBUG(MSCC,NETLOG_DEBUG3,
			("%s Leg2 Sending Setup for %s to %s: %p\n",fn,srcAddress,dstAddress,hcall));

	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;

_error:
	H323FreeEvent(evtPtr);
	GkCallDropReasonNone(H323hsCall(&callHandleBlk), callError);
	cmCallDrop(H323hsCall(&callHandleBlk));
	return 0;
}

// Check
int SCCErrorHandler(SCC_EventBlock *evtPtr)
{
	static char fn[] = "SCCErrorHandler() : ";

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	H323FreeEvData((H323EventData *)evtPtr->data);
	free(evtPtr);

	/* MUST GENERATE AN ERROR into the S/M */
	return -1;
}

int SCCLogEvent(SCC_EventBlock *evtPtr)
{
	static char fn[] = "SCCLogEvent() : ";
	char str[256];

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s received event %s:\n",
			fn, SCC_EventToStr(evtPtr->event,str)));
	H323FreeEvData((H323EventData *)evtPtr->data);
	free(evtPtr);
	return 0;
}


int SCCNetworkProceeding(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkProceeding() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	int					nodeId = -1;
	cmTransportAddress 	cmTransAddr;
	int					addrlen;
	int					retval = -1;
	char				ipstr[24];
	int					sid = 0; // In fast start only audio supported for now
	int 				dstInfoNodeId = -1;
	h221VendorInfo	 		h221ns;
	BOOL 				isstring;
	int 				vendor;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	
	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Log Error and try to continue
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	if(cmCallGetParam(H323hsCall(callHandle),
				cmParamH245Address,
				0,
				&addrlen,
				(char *)&cmTransAddr) >0)
	{
		H323h245ip(callHandle) = ntohl(cmTransAddr.ip);
		H323h245port(callHandle) = cmTransAddr.port;
	}

    if (cmCallGetParam(H323hsCall(callHandle), 
                           cmParamFullDestinationInfo, 0, 
                           &dstInfoNodeId, NULL) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No src info nodeid found\n", fn));
		dstInfoNodeId = -1;
    }

	if (dstInfoNodeId >= 0)
	{	
		HPVT 	hVal = cmGetValTree(UH323Globals()->hApp);

    	cmMeiEnter(UH323Globals()->hApp);

		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.t35CountryCode", NULL,
				(INT32 *)&h221ns.t35CountryCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s t35 country code not present\n",fn));
				h221ns.t35CountryCode = -1;
		}

		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.manufacturerCode", NULL,
				(INT32 *)&h221ns.manufacturerCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s mfr code not present\n",fn));
			h221ns.manufacturerCode = -1;
		}

		vendor = GetVendorFromH221Info(&h221ns);
		if (vendor > 0)
		{
			callHandle->vendor = vendor;
		}

    	cmMeiExit(UH323Globals()->hApp);
	}

    if( cmCallGetParam(H323hsCall(callHandle),
        cmParamCallProcFastStart,
        0,
        &nodeId,
        NULL) >=0)
    {
        NETDEBUG(MSCC,NETLOG_DEBUG4,
            ("%s cmCallGetParam nodeid = %d. Connect\n",
            fn,nodeId));
		// Faststart succeeded. Don't respond to Open logical Channel 
		H323doOlc(callHandle) = 0;
		callHandle->fastStartStatus = 1;
		/* only if we are not doing h245 routing will we decode fast start */
		if(!H323outChan(callHandle)[sid].ip)
		{
			if(!localProceeding)
			{
				NETDEBUG(MSCC,NETLOG_DEBUG4,
					("%s fast connect Connect channel address not set. nodid = %d, deferring\n",
					fn,nodeId));
				H323chanEvt(callHandle) = SCC_eNetworkProceeding;
			}
			CacheReleaseLocks(callCache);
			goto _return;
		}
    }
	if(localProceeding)
	{
		CacheReleaseLocks(callCache);
		goto _return;
	}

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeProceeding;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);

#if 0 // peer might free this
	pH323Data->nodeId = nodeId;
#endif

	if(routeH245==0)
	{
		pH323Data->h245ip = H323h245ip(callHandle);
		pH323Data->h245port = H323h245port(callHandle);
	}

    if(H323outChan(callHandle)[sid].ip)
    {
		updateNonStandardCodecType(callHandle, sid, 1);
        pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
        memset(pH323Data->localSet,0,sizeof(RTPSet));
        callHandle->lastMediaIp = pH323Data->localSet[0].rtpaddr = H323outChan(callHandle)[sid].ip;
        callHandle->lastMediaPort = pH323Data->localSet[0].rtpport = H323outChan(callHandle)[sid].port;
        pH323Data->localSet[0].codecType = H323outChan(callHandle)[sid].codec;
        pH323Data->localSet[0].param = H323outChan(callHandle)[sid].param;
        pH323Data->nlocalset = 1;

		NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s ip=%s/%d codec = %d/%d\n",
					fn,
				  	FormatIpAddress(pH323Data->localSet[0].rtpaddr, ipstr),
					pH323Data->localSet[0].rtpport,
					pH323Data->localSet[0].codecType,
					pH323Data->localSet[0].param));
    }

	openH245RemotePinhole(fn, callHandle, "PROCEEDING", &pH323Data->h245ip, &pH323Data->h245port);

	CacheReleaseLocks(callCache);

	// uh323FastStart2RTPSet(H323hsCall(&callHandleBlk),pH323Data);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	}
_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
}

// Send Alerting to Leg1 source
int SCCInitiateAlerting(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCInitiateAlerting() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	int					fs = 0;
	HCALL				hsCall;
	H323RTPSet			*pH323Set;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Nothing can be done if we cannot find the callHandle
		CacheReleaseLocks(callCache);
		NETERROR(MSCC,("%s Unable to locate my CallHandle\n",fn));
		goto _return;
	}
	
	hsCall = H323hsCall(callHandle);
	if ((callHandle->fastStart == 1) &&
				(callHandle->fastStartStatus != 1))
	{
		if(pH323Data->nlocalset && pH323Data->localSet[0].rtpaddr)
		{
			fs = uh323SetFastStartParam(callHandle,
					pH323Data->localSet,pH323Data->nlocalset);

			if(fs > 0)
			{
				H323doOlc(callHandle) = 0;
				callHandle->fastStartStatus = 1;
			}
			else if (fs < 0)
			{
				NETERROR(MH323, ("%s Skipping, FST failed\n", fn));
				CacheReleaseLocks(callCache);
				goto _return;
			}
			/* Set the q931 progressIndicator and Signal */
			if(pH323Data->q931NodeId || pH323Data->msgFlags&MSGFLAG_PI)
			{
				setAlertingQ931(hsCall,pH323Data->q931NodeId);
			}
			pH323Data->q931NodeId = -1;
		}
	}
	else if (pH323Data->nlocalset && pH323Data->localSet[0].rtpaddr)
	{
		cmH245Stage			h245Stage;
		pH323Set =(CMalloc(callCache))(pH323Data->nlocalset*sizeof(H323RTPSet));
		copyRtp2H323RtpSet(pH323Set,pH323Data->localSet,pH323Data->nlocalset);
		if(H323nremoteset(callHandle))
		{
			(CFree(callCache))H323remoteSet(callHandle);
		}
		H323nremoteset(callHandle) = pH323Data->nlocalset;
		H323remoteSet(callHandle) = pH323Set;
		/* start H245 */
#if 0 /* sending facility to start h245 */
		if (cmCallConnectControl(H323hsCall(callHandle)) < 0)
		{
			NETERROR(MH323, ("%s cmCallConnectControl failed %x %s\n",
			fn,H323hsCall(callHandle),
			CallID2String(evtPtr->callID,callIDStr)));
		}
#else	/* start early H.245 in alerting */
		h245Stage = cmTransH245Stage_alerting;
		if (cmCallSetParam(H323hsCall(callHandle),
			cmParamH245Stage,
			0,
			h245Stage,
			NULL) < 0)
		{
			NETERROR(MH323,
				("%s Could not set H245Stage to %d\n", fn,h245Stage));
		}
#endif
	}
	callHandle->lastEvent = SCC_evt323AlertTx;
	if (timedef_iszero(&callHandle->callRingbackTime))
	{
		timedef_cur(&callHandle->callRingbackTime);
	}

	CacheReleaseLocks(callCache);
	cmCallAccept(hsCall);	

_return:
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

// Send Proceeding to Leg1 source
int SCCInitiateProceeding(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCInitiateProceeding() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	HCALL				hsCall;
	int					fs = 0;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Nothing can be done if we cannot find the callHandle
		CacheReleaseLocks(callCache);
		NETERROR(MSCC,("%s Unable to locate my CallHandle\n",fn));
		goto _return;
	}
	
	if ((callHandle->fastStart == 1) &&
				(callHandle->fastStartStatus != 1))
	{
		if(pH323Data->nlocalset && pH323Data->localSet[0].rtpaddr)
		{
			fs = uh323SetFastStartParam(callHandle,
					pH323Data->localSet,pH323Data->nlocalset);

			if(fs > 0)
			{
				H323doOlc(callHandle) = 0;
				callHandle->fastStartStatus = 1;
			}
			else if (fs < 0)
			{
				NETERROR(MH323, ("%s Skipping, FST failed\n", fn));
				CacheReleaseLocks(callCache);
				goto _return;
			}
		}
	}
	
	// set up last event
	callHandle->lastEvent = SCC_evt323ProcTx;

	hsCall = H323hsCall(callHandle);	
	CacheReleaseLocks(callCache);
	cmCallSendCallProceeding(hsCall);

_return:
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
}

// Send an accept at the leg1. This is done once we receive a Bridge Connect
int SCCLeg1ConnectCall(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCLeg1ConnectCall() : ";
	cmTransportAddress 	cmTransAddr = { 0, 0, 1720, cmTransportTypeIP };
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	int					len;
	HCALL				hsCall;
	int					fs = 0;
	H323RTPSet			*pH323Set = NULL;
	int					callError = SCC_errorNone;
	void				*timerdata;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Nothing can be done if we cannot find the callHandle
		NETERROR(MSCC,("%s Unable to locate my CallHandle\n",fn));
		callError = SCC_errorNoCallHandle;
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	hsCall = H323hsCall(callHandle);

	if (callHandle->rolloverTimer != 0)
	{
		if(timerDeleteFromList(&localConfig.timerPrivate, 
					callHandle->rolloverTimer, &timerdata))
		{
			if(timerdata)
			{
				free(timerdata);
			}
		}

		callHandle->rolloverTimer = 0;
	}
	
	/* This should be done before we do FastStartAckIndex, because stack
	* 	calls channes state connected with the call back
	* 	If AckIndex fails - then we will undo this 
	*/
	if((callHandle->fastStart == 1) && (pH323Data->nlocalset > 0))
	{
		H323doOlc(callHandle) = 0;
	}

	// If we are not routing h.245 and an h245 address is specified
	if(routeH245 == 0 && pH323Data->h245ip)
	{
		cmTransAddr.length = 4;
		cmTransAddr.ip = pH323Data->h245ip;
		cmTransAddr.port = pH323Data->h245port;
		len = sizeof(cmTransportAddress);

		if(cmCallSetParam(H323hsCall(callHandle),
			cmParamH245Address,
			0,
			len,
			(char *)&cmTransAddr) <0)
		{
			NETERROR(MSCC,("%s Unable to Set cmParamSetupH245Address\n",
				fn));
			callError = SCC_errorH323Internal;
			CacheReleaseLocks(callCache);
			goto _error;
		}
	}

    if(routeH245)
    {
		if(!callHandle->realmInfo)
		{
			NETERROR(MSCC,("%s no realm info present on call %p\n", 
				fn, H323hsCall(callHandle)));
			callError = SCC_errorH323Internal;
			CacheReleaseLocks(callCache);
			goto _error;
		} 
        if(cmCallGetParam(H323hsCall(callHandle),
                    cmParamH245Address,
                    0,
                    &len,
                    (char *)&cmTransAddr) >0)
        {
			cmTransAddr.ip = htonl(callHandle->realmInfo->rsa);
			cmTransAddr.type = cmTransportTypeIP;
			len = sizeof(cmTransportAddress);

		    if(cmCallSetParam(H323hsCall(callHandle),
				cmParamH245Address,
				0,
				len,
			    (char *)&cmTransAddr) <0)
			{
					NETERROR(MSCC,("%s Unable to Set rsa cmParamH245Address %p\n",
						fn,H323hsCall(callHandle)));
			}
        }
    }

	if ((callHandle->fastStart == 1) &&
				(callHandle->fastStartStatus != 1))
	{
		/* Ack the Fast Start Channel Index */
		if(pH323Data->nlocalset && pH323Data->localSet[0].rtpaddr)
		{
			fs = uh323SetFastStartParam(callHandle,
							pH323Data->localSet,
							pH323Data->nlocalset);

			if(fs > 0)
			{
				H323doOlc(callHandle) = 0;
				callHandle->fastStartStatus = 1;
			}
			else if (fs < 0)
			{
				NETERROR(MH323, ("%s Skipping, FST failed\n", fn));
				CacheReleaseLocks(callCache);
				goto _return;
			}
		}
	}

	// Store the connect time
	timedef_cur(&callHandle->callConnectTime);

	/* Copy the remote rtpSet. This should be done AFTER acking the channels i.e
	*  after uh323SetFastStartParam. This would be used if we ever have to 
	*  send out TCS
	*/

	if(pH323Data->nlocalset > 0)
	{
		pH323Set = (CMalloc(callCache))(pH323Data->nlocalset*sizeof(H323RTPSet));
		copyRtp2H323RtpSet(pH323Set,pH323Data->localSet,pH323Data->nlocalset);
	}

	if(H323nremoteset(callHandle))
	{
		(CFree(callCache))H323remoteSet(callHandle);
	}

	H323nremoteset(callHandle) = pH323Data->nlocalset;
	H323remoteSet(callHandle) = pH323Set;

	if(!(callHandle->ecaps1&ECAPS1_NOH323DISPLAY))
	{
		len = strlen(CallInputNumber(callHandle));
		if( cmCallSetParam(H323hsCall(callHandle),
				 cmParamConnectDisplay,
				 0,
				 len,
				 CallInputNumber(callHandle)) < 0)
		{
			NETERROR(MH323,
				 ("%s Could not set connectDisplay %s\n",
				 fn,CallInputNumber(callHandle)));
		}
	}

	CacheReleaseLocks(callCache);

	if (cmCallAnswer(hsCall))
	{
		NETERROR(MSCC,("%s cmCallAnswerfailed\n",fn));
	}

_return:
	H323FreeEvData(pH323Data);
	free(evtPtr);
	return 0;
_error:
	H323FreeEvData(pH323Data);
	free(evtPtr);
	GkCallDropReasonNone(hsCall, callError);
	cmCallDrop(hsCall);

	return 0;
}


// UTILITY ROUTINES

static int eventBlockAllocs = 0,eventBlockFrees = 0;

SCC_EventBlock * sccAllocEventBlock(void)
{
	SCC_EventBlock *p;
	p = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(p,0,sizeof(SCC_EventBlock));
	eventBlockAllocs++;
	return p; 
}
	
int sccUh323InitEventBlock(SCC_EventBlock *evPtr,UH323CallAppHandle *appHandle)
{
	memset(evPtr, 0, sizeof(SCC_EventBlock));
	memcpy(evPtr->confID,appHandle->confID,CONF_ID_LEN); 
	memcpy(evPtr->callID,appHandle->callID,CALL_ID_LEN); 
	return 0;
}
	
void sccFreeEventBlock(SCC_EventBlock * eventPtr)
{
	free (eventPtr);
	eventBlockFrees++;
}

void sccCopyEventBlock(SCC_EventBlock *dest, const SCC_EventBlock *src)
{
	dest->event = src->event;
	dest->data = src->data;
	memcpy(dest->confID,src->confID,CONF_ID_LEN); 
	memcpy(dest->callID,src->callID,CALL_ID_LEN); 
}
	


void sccDumpEventBlockStats(void)
{
	char fn[]="sccDumpEventBlockStats:";

	NETDEBUG(MSCC,NETLOG_DEBUG4,("%s Alloc = %d freed = %d inUse = %d\n",
			fn, eventBlockAllocs,eventBlockFrees,eventBlockAllocs-eventBlockFrees));

}


#ifdef _call_rollover
int CallRolloverTimer(struct Timer *t)
{
	static char fn[]="CallRolloverTimer:";
	char 				callID[CALL_ID_LEN]={0};
	memcpy(callID, t->data, CALL_ID_LEN);

	free(t->data);
	timerFreeHandle(t);

 	return CallRollover(callID);
}


// returns -1 if rollover could not be initiated. 
int CallRollover(char callID[CALL_ID_LEN])
{
	char fn[]="CallRollover:";
	char 				newCallID[CALL_ID_LEN]={0};
	char 				confID[CONF_ID_LEN]={0};
	CallHandle 			callHandleBlk1={0}, callHandleBlk2={0}, newCallHandleBlk={0};
	CallHandle 			*newCallHandle;
	ConfHandle 			confHandleBlk;
	char 				callIDStr[CALL_ID_LEN];
	UH323CallAppHandle 	*appHandle2; // Call Handle for leg2
	char				*pCallingPartyNumber;
	int					callingPartyNumberLen;
	HCALL				hcall;
	int					rc;
	int					len;
	SCC_EventBlock 		event;
	char				str[256];


	// Find Leg2 CallHandle
	if (CacheFind(callCache, callID, &callHandleBlk2, sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle for call 1\n",fn));
		goto _error;
	}

	memcpy(confID, callHandleBlk2.confID, CONF_ID_LEN);

	if (CacheFind(confCache,confID,&confHandleBlk,sizeof(ConfHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle\n",fn));
		goto _error;
	}

	// Find Leg1 CallHandle
	if (CacheFind(callCache, confHandleBlk.callID[0],&callHandleBlk1,sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle for call 0\n",fn));
		goto _error;
	}
	
	// Make Sure we can get everything we need for rollover b4 dropping the call
	// Resolve the rollover number
	memset(&callHandleBlk1.rfphonode,0, sizeof(PhoNode));
	strcpy(callHandleBlk1.rfphonode.phone,H323dialledNumber(&callHandleBlk1));
    if(GkResolve2Rollover(&callHandleBlk1,1,1)<=0)
    {
        NETDEBUG(MSCC,NETLOG_DEBUG2,
            ("%s GkResolve2RollOver Failed for = %x\n",fn,hcall));
        goto _error;
    }

	
	if(!(appHandle2 = uh323CallAllocAppHandle()))
	{
		NETERROR(MSCC,("%s Unable to allocate appHandle\n",fn));
		goto _error;
	}

	
	rc = cmCallNew(UH323Globals()->hApp,(HAPPCALL)appHandle2,&hcall);

	if(rc <0)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,("%s cmCallNew return error = %d\n",
			fn, rc));
		uh323CallFreeAppHandle(appHandle2);
		goto _error;
	}

	/* Set the Conf Id */
	len = CONF_ID_LEN;
	if( cmCallSetParam(hcall,
			 cmParamCID, 
			 0, 
			 len,
			 confID) < 0)
	{
		NETERROR(MH323, 
			 ("%s Could not set conf id\n", fn));
		uh323CallFreeAppHandle(appHandle2);
		goto _error;
	}

	pCallingPartyNumber = H323callingPartyNumber(&callHandleBlk1);
	callingPartyNumberLen = H323callingPartyNumberLen(&callHandleBlk1);


	/* Get the Call Id */
	memset(newCallID, 0, CALL_ID_LEN);
	len = CALL_ID_LEN;
	if (cmCallGetParam(hcall, 
				 cmParamCallID, 
				 0, 
				 &len,
				 newCallID) < 0)
	{
		NETERROR(MH323, 
				 ("%s Could not get call ID id\n", fn));
		goto _error;
	}
	NETDEBUG(MH323,NETLOG_DEBUG4,
		("%s CallID Leg2 = %s, hscall = %x\n", fn,newCallID,hcall));

	memcpy(appHandle2->confID,confID,CONF_ID_LEN);
	memcpy(appHandle2->callID,newCallID,CALL_ID_LEN);

	newCallHandle = &newCallHandleBlk;
	GisInitCallHandle(newCallHandle,hcall);
	H323appHandle(newCallHandle) = appHandle2;
	
	H323callingPartyNumberLen(newCallHandle) = callingPartyNumberLen;
	memcpy(H323callingPartyNumber(newCallHandle),pCallingPartyNumber,callingPartyNumberLen);
	newCallHandle->leg = SCC_CallLeg2;
	newCallHandle->networkEventProcessor = &SCC_H323Leg2NetworkEventProcessor;
	newCallHandle->bridgeEventProcessor = &SCC_H323Leg2BridgeEventProcessor;
	newCallHandle->state = SCC_sWaitOnRemote;



    memcpy(&newCallHandle->phonode,&callHandleBlk1.rfphonode,sizeof(PhoNode));

	if (CachePut(callCache, newCallHandle, sizeof(CallHandle)) < 0)
	{
		NETERROR(MSCC,
			("%s Failed to insert new callHandle in callCache\n",fn));
		goto _error;
	}

	// We are all ready to make the other call - drop the original call..
	GisDeleteCallFromConf(callID, confID);
	CallID2String(callID, callIDStr);
	NETDEBUG(MSCC, NETLOG_DEBUG4, 
		("%s Dropping the call with callID %s\n", fn, callIDStr));
	/* Set the conf id of app handle to null and then drop the call */
	memset(H323appHandle(&callHandleBlk2)->confID, 0, CONF_ID_LEN);
	cmCallDrop((HCALL )H323hsCall(&callHandleBlk2));


	/* Create a new rolled over call */
	// Leg2
	if(GisAddCallToConf(newCallHandle)!=0 )
	{
		NETERROR(MSCC,
			("%s GisAddCallToConf Failed\n",fn));
		goto _droprolledover;
	}

	callHandleBlk1.rolledOver = 1;	
	/* Save the call handle into the cache. */
	if (CacheOverwrite(callCache, &callHandleBlk1, sizeof(CallHandle), callHandleBlk1.callID) < 0)
	{
		NETERROR(MSCC,
			("%s Failed to overwrite callHandle in callCache\n",fn));
		goto _droprolledover;
	}

	NETDEBUG(MSCC,NETLOG_DEBUG2,
		("%s - Inserted CallHandle %s",
			fn, (char*) CallID2String(newCallHandle->callID,callIDStr)));

	memcpy(event.callID,newCallID,CALL_ID_LEN);
	memcpy(event.confID,confID,CONF_ID_LEN);

    /* If we have to send an ARQ, then we will send it, otherwise,
     * we will just send the setup directly
     */
    if (BIT_TEST(newCallHandle->phonode.cap, CAP_ARQ))
    {
        if (GkSendARQ(&callHandleBlk1.rfphonode, newCallHandle,NULL, 0) >= 0)
        {
            NETDEBUG(MSCC,NETLOG_DEBUG3,
                ("%s Sent the ARQ\n", fn));
            return 0;
        }
		NETDEBUG(MSCC,NETLOG_DEBUG2, ("%s GkSendARQ failed \n", fn));
		goto _droprolledover;
    }

    if (BIT_TEST(newCallHandle->phonode.cap, CAP_LRQ))
    {
        if (GkSendLRQ(&callHandleBlk1.rfphonode, newCallHandle,NULL) >= 0)
        {
            NETDEBUG(MSCC,NETLOG_DEBUG3,
                ("%s Sent the LRQ\n", fn));
            return 0;
        }
		NETDEBUG(MSCC,NETLOG_DEBUG2, ("%s GkSendLRQ failed \n", fn));
		goto _droprolledover;
    }

	NETDEBUG(MSCC,NETLOG_DEBUG2,
		("%s - Invoking SCCLeg2BridgeMakeCall event = %s\n",fn,sccEventBlockToStr(&event, str)));

	return SCCLeg2BridgeMakeCall(&event);

_error:
	return -1;
_droprolledover:
	cmCallClose(hcall);
	return 0;
}
#endif

// If have remote capability send it and move to cap sent state
// else move to TransportConnected and generate the bridge event 
// which will be ignored
int SCCNetworkTransportConnected(SCC_EventBlock *evtPtr)
{
	static 		char fn[] = "SCCNetworkTransportConnected() : ";
	CallHandle 	*callHandle;
	char		callIDStr[CALL_ID_STRLEN];
	int			controlState;
	int			rv = -1;
    UH323CallAppHandle 	*appHandle = NULL;
	HPROTCONN	h245conn;
	int			restrictedTCS= 1;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}

	// Check to see if any holes need to be closed
	if (cmCallGetHandle(H323hsCall(callHandle), (HAPPCALL *)&appHandle) < 0)
	{
		NETERROR(MSCC,("%s Cannot locate app handle for this call\n", fn));
	}
	else if (appHandle) // appHandle was found and is initialized
	{
			if (appHandle->flags & FL_CALL_H245CTRLIN)
			{
				// Connection is incoming, close the hole
				// opened for remote
				if (H323remoteBundleId(callHandle) != 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s Closing remote H.245 hole, local hole used\n",
						fn));
					FCECloseBundle(CallFceSession(callHandle), H323remoteBundleId(callHandle));
					H323remoteBundleId(callHandle) = 0;
				}
			}
			else
			{
				// Connection is outgoing, close the hole
				// opened for local
				if (H323localBundleId(callHandle) != 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s Closing local H.245 hole, remote hole used\n",
						fn));
					FCECloseBundle(CallFceSession(callHandle), H323localBundleId(callHandle));
					H323localBundleId(callHandle) = 0;
				}
			}

		if (!(appHandle->flags & FL_CALL_H245APPINIT))
		{
			if (h245conn = cmGetTpktChanHandle(H323hsCall(callHandle),cmH245TpktChannel))
			{
				if(cmSetTpktChanApplHandle(h245conn,(HAPPCONN)appHandle) < 0)
				{
					// Not an error as we may be using tunelling
					// Also, since a tunnelled connection may open a separate h.245 connection
					// this check probably needs to be there.
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s : cmSetTpktChanApplHandle Failed %p\n",
						fn,H323hsCall(callHandle)));
				}
				else
				{
					appHandle->flags |= FL_CALL_H245APPINIT;
					appHandle->h245Conn = h245conn;

					NETDEBUG(MSCC, NETLOG_DEBUG4,
						("%s : H245 Tpkt app handle initialized.\n", fn));
				}
			}
			else {
					// Not an error since we may be using tunneling
					NETDEBUG(MH323, NETLOG_DEBUG4, 
						("%s : cmGetTpktChanApplHandle H245 Failed %p\n",
						fn,H323hsCall(callHandle)));
			}
		}
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s setting state of H.245 to connected\n", fn));

		// After this is done, we don't want to approach
		// this topic ever again
		appHandle->flags |= FL_CALL_H245CTRLCONN;
	}
	else // call was successful, but appHandle is not initialized
	{
		// don't think this happens ever
		NETERROR(MSCC,("%s app handle unitialized for this call\n", fn));
	}

	controlState = H323controlState(callHandle);

	// if we have peer's TCS and we haven't sent it out yet
	// or if we have the fast start element from the peer
	// for iwf - setup non fast start we have the remoteset which we should fwd.
	if ( H323remoteTCSNodeId(callHandle) ||
// 	((callHandle->fastStartStatus || !callHandle->fastStart) && H323nremoteset(callHandle)))
	H323nremoteset(callHandle) )
	{
		rv = uh323CapSend(H323hsCall(callHandle),
				H323remoteSet(callHandle),H323nremoteset(callHandle),H323remoteTCSNodeId(callHandle), callHandle->ecaps1, callHandle->vendor);
	}

	if(rv <0)
    {   
        H323controlState(callHandle) =  UH323_sControlTransportConnected;
    }
    else 
	{
        /* Store the state into the call handle */
        H323controlState(callHandle) =  UH323_sControlCapSent;
	if (H323remoteTCSNodeId(callHandle)>0)
		{
			freeNodeTree(UH323Globals()->peerhApp,
				H323remoteTCSNodeId(callHandle), 0);
			H323remoteTCSNodeId(callHandle) = 0;
		}
	}
_return:
	CacheReleaseLocks(callCache);
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
}

int SCCBridgeTransportConnected(SCC_EventBlock *evtPtr)
{
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
}

int SCCNetworkTCSAck(SCC_EventBlock *evtPtr)
{
	static 		char fn[] = "SCCNetworkTCSAck() : ";
	CallHandle 		callHandleBlk = {0},callHandleBlk2 = {0};
	SCC_EventBlock	*pNextEvt;
	H323EventData	*pH323Data;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

    pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
    memset(pNextEvt,0,sizeof(SCC_EventBlock));

    pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
    memset(pH323Data,0,sizeof(H323EventData));

    pNextEvt->data = (void *) pH323Data;
    pNextEvt->event = SCC_eBridgeTCSAck;
    memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

    if(bridgeH323EventProcessor(pNextEvt)!=0)
    {
        NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
    }

_return:
    H323FreeEvData((H323EventData *)(evtPtr->data));
    free(evtPtr);
	return 0;
}

int SCCNetworkControlConnected(SCC_EventBlock *evtPtr)
{
	static 		char fn[] = "SCCNetworkControlConnected() : ";
	CallHandle 		callHandleBlk = {0},callHandleBlk2 = {0};
	SCC_EventBlock	*pNextEvt;
	H323EventData	*pH323Data;
	CallHandle 	*callHandle;
	char		callIDStr[CALL_ID_STRLEN];

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	H323controlState(callHandle) =  UH323_sControlConnected;

	CacheReleaseLocks(callCache);

    pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
    memset(pNextEvt,0,sizeof(SCC_EventBlock));

    pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
    memset(pH323Data,0,sizeof(H323EventData));

    pNextEvt->data = (void *) pH323Data;
    pNextEvt->event = SCC_eBridgeControlConnected;
    memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

    if(bridgeH323EventProcessor(pNextEvt)!=0)
    {
        NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
    }

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	processPendingEvents(callHandle);
	CacheReleaseLocks(callCache);

_return:
    H323FreeEvData((H323EventData *)(evtPtr->data));
    free(evtPtr);
	return 0;
}

int SCCBridgeTCSAck(SCC_EventBlock *evtPtr)
{
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
}

int SCCBridgeControlConnected(SCC_EventBlock *evtPtr)
{
	static 	char 	fn[] = "SCCBridgeControlConnected() : ";
	CallHandle 		*callHandle;
	char			callIDStr[CALL_ID_STRLEN];

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	callHandle = CacheGet(callCache,evtPtr->callID);
	if(callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	} 

	if ((callHandle->vendor == Vendor_eAvaya) &&
	 	(H323controlState(callHandle) == UH323_sControlIdle))
	{
		SCC_EventBlock	*pNextEvt;
		H323EventData	*pH323Data;

	    pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
    	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	    pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
    	memset(pH323Data,0,sizeof(H323EventData));

	    pNextEvt->data = (void *) pH323Data;
    	pNextEvt->event = SCC_eBridgeControlConnected;
	    memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	    memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
    	memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

	    if(bridgeH323EventProcessor(pNextEvt)!=0)
    	{
        	NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	    }
		goto _return;
	}
	
	if (H323controlState(callHandle) == UH323_sControlIdle)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4, 
			("%s Control state Idle %p. Sending Facility\n",
			fn,H323hsCall(callHandle)));
		if (cmCallConnectControl(H323hsCall(callHandle)) < 0)
		{
			NETERROR(MH323, ("%s cmCallConnectControl failed %p %s\n",
			fn,H323hsCall(callHandle),
			CallID2String(evtPtr->callID,callIDStr)));
		}
	}
_return:
	CacheReleaseLocks(callCache);
	H323FreeEvent(evtPtr);
	return 0;
}

// if butterfly tell him to shut up
// if butterfly tell him to shut up
// else not much use. set a flag or something to indicate we have the endpoint's
// capability
int SCCNetworkCapability(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCNetworkCapability() : ";
	CallHandle 		*callHandle,callHandleBlk = {0},callHandleBlk2 = {0};
	int 			nodeid = -1;
	SCC_EventBlock	*pNextEvt;
	H323EventData	*pH323Data;
	char			callIDStr[CALL_ID_STRLEN];
	char			callID2[CALL_ID_LEN];
	ConfHandle		confHandleBlk = {0};
	int				nlocalset;
	H323EventData *evData = (H323EventData *)evtPtr->data;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	if (CacheFind(callCache,evtPtr->callID,&callHandleBlk,sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}
	
#ifndef _no_hack_for_butterfly
	// Hack for Butterfly signalling
	// This case will surface for Butterfly signaling. 
	// Just tell the endpoint to shut up
	getPeerCallID(evtPtr->confID,evtPtr->callID,callID2);
	if(CacheFind(callCache,callID2,&callHandleBlk2,sizeof(CallHandle))<0 )
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle for other leg \n",fn));
		if ((nodeid = uh323GetLocalCaps()) < 0)
		{
			NETERROR(MH323,
				("%s Could not obtain Local capabilities\n", fn));
			goto _return;
		}
		if (cmCallSendCapability(H323hsCall(&callHandleBlk), nodeid) < 0)
		{
			NETERROR(MH323,
				("%s Could not forward Local capabilities\n", fn));
		}
		if (cmCallMasterSlaveDetermine(H323hsCall(&callHandleBlk), 120) < 0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4,
				("%s %p cmCallMSD failed.\n", 
					fn,H323hsCall(&callHandleBlk)));
		}
		goto _return;
	} 
#endif

    pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
    memset(pNextEvt,0,sizeof(SCC_EventBlock));

    pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
    memset(pH323Data,0,sizeof(H323EventData));

    pNextEvt->data = (void *) pH323Data;
    pNextEvt->event = SCC_eBridgeCapabilities;
    memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->confID,callHandleBlk.confID,CONF_ID_LEN);

	if (CacheFind(confCache,callHandleBlk.confID,&confHandleBlk,sizeof(ConfHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle\n",fn));
		goto _return;
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	callHandle = CacheGet(callCache,evtPtr->callID);
	nlocalset = evData->nlocalset;
	if (nlocalset)
	{
		if(confHandleBlk.confType == Conf_eH323P2P)
		{
			/* for h323-h323 calls dup the nodeId and pass */ 
			if ((nodeid = cmCallGetRemoteCapabilities(H323hsCall(&callHandleBlk))) >= 0)
			{
				int tmpnodeId = -1;
				HPVT hVal = cmGetValTree(UH323Globals()->hApp);
				HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);

				cmMeiEnter(UH323Globals()->hApp);
				cmMeiEnter(UH323Globals()->peerhApp);
				tmpnodeId = pvtAddRootByPath(peerhVal, 
								cmGetSynTreeByRootName(UH323Globals()->peerhApp, "h245"),
								"request.terminalCapabilitySet", 0, NULL);

				if(tmpnodeId<0)
				{
					NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
				}
				else if(pvtSetTree(peerhVal, tmpnodeId, hVal, nodeid) <0)
				{
					pvtDelete(peerhVal,tmpnodeId);
					NETERROR(MH323,("%s pvtSetTree failed\n",fn));
				}
				else {
					pH323Data->nodeId = tmpnodeId;
				}
				cmMeiExit(UH323Globals()->hApp);
				cmMeiExit(UH323Globals()->peerhApp);
			}
			else {
					NETERROR(MH323,("%s cmCallGetRemoteCapabilities failed \n",fn));
			}
		}
		else {
			/* pass the capability */
			if(evData->nlocalset)
			{
				pH323Data->nlocalset = evData->nlocalset;
				pH323Data->localSet = evData->localSet;
				evData->localSet = NULL;
				evData->nlocalset = 0;
			}


			if (callHandle)
			{
				/*update the localset everytime */
				if  (!H323nlocalset(callHandle) ||
                      (callHandle->fastStart == 0) || (callHandle->fastStartStatus == 1))
                {
				    if (H323nlocalset(callHandle) > 0)
				    {
					    (CFree(callCache))H323localSet(callHandle);
				    }
    				H323RTPSet 	*pRtpSet;
	    			pRtpSet = (H323RTPSet *) (CMalloc(callCache))(pH323Data->nlocalset*sizeof (H323RTPSet));
		    		copyRtp2H323RtpSet(pRtpSet,pH323Data->localSet,pH323Data->nlocalset);
			    	H323nlocalset(callHandle) = pH323Data->nlocalset;
				    H323localSet(callHandle) = pRtpSet;
                }
			}
			else {
				NETERROR(MSCC,("%s Unable to locate ConfHandle %s\n",
					fn,CallID2String(evtPtr->callID,callIDStr)));
			}

		}
	}
	else
	{
		/* make sure its it not null */
		BIT_SET(H323flags(callHandle), H323_FLAG_NULLTCS_RECVD);
	}
	CacheReleaseLocks(callCache);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
    }

	if (nlocalset && (callHandle) &&
		BIT_TEST(H323flags(callHandle), H323_FLAG_NULLTCS_RECVD))
	{
		/* In response to First Non-NULL TCS after a NULL TCS csco 2600 
		 * is not responding with its TCS so we never do the MSD with Cisco
		 * CCM
		 */
		BIT_RESET(H323flags(callHandle), H323_FLAG_NULLTCS_RECVD);
		cmCallMasterSlaveDetermine(H323hsCall(&callHandleBlk), 120);
	}
_return:
    H323FreeEvData((H323EventData *)(evtPtr->data));
    free(evtPtr);
	return 0;
}

// we should forward capabilities - in all cases except CapSent && ControlConnected
// This is to avoid retransmission of the same capabilities
int SCCBridgeCapability(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeCapability() : ";
	CallHandle 		*callHandle;
	int 			controlState;
	char			callIDStr[CALL_ID_STRLEN];
	H323RTPSet		*pH323Set = 0;
	H323EventData	*pH323Data;
	int				nullTCS = 0;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
    pH323Data = (H323EventData *)(evtPtr->data);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}
	
	if(!pH323Data->nodeId && !pH323Data->nlocalset)
	{
		nullTCS = 1;
	}

	/* Save the peer Capability Set */
	if(pH323Data->nlocalset)
	{
		pH323Set =(CMalloc(callCache))(pH323Data->nlocalset*sizeof(H323RTPSet));
		copyRtp2H323RtpSet(pH323Set,pH323Data->localSet,pH323Data->nlocalset);
		if(H323nremoteset(callHandle))
		{
			(CFree(callCache))H323remoteSet(callHandle);
		}
		H323nremoteset(callHandle) = pH323Data->nlocalset;
		H323remoteSet(callHandle) = pH323Set;
	}

	controlState = H323controlState(callHandle);
#if 0
	if (!callHandle->fastStartStatus && controlState == UH323_sControlIdle)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4, 
			("%s Nonfastart call. Control state Idle %x. Sending Facility\n",
			fn,H323hsCall(callHandle)));
		if (cmCallConnectControl(H323hsCall(callHandle)) < 0)
		{
			NETERROR(MH323, ("%s cmCallConnectControl failed %x %s\n",
			fn,H323hsCall(callHandle),
			CallID2String(evtPtr->callID,callIDStr)));
		}
		SCCQueueEvent(evtPtr);
		// evtPtr should not be free in this case
		CacheReleaseLocks(callCache);
		return 0;
	}
#endif
	NETDEBUG(MSCC,NETLOG_DEBUG3,
		("%s hsCall(%p) controlState = %s. ncodecs = %d nodeId = %d:\n",
		fn,H323hsCall(callHandle),GetUh323ControlState(controlState),
		pH323Data->nlocalset,pH323Data->nodeId));

	if(nullTCS)
	{
		H323doOlc(callHandle) = 1;
		switch (controlState)
		{
			case UH323_sControlCapSent: 
			case UH323_sControlHeldCapSent: 
			case UH323_sControlTransportConnected: 
			case UH323_sControlHeldByLocal: 
			case UH323_sControlConnected: 
			NETDEBUG(MSCC,NETLOG_DEBUG3,
				("%s %p Sending Null Capabilities:\n",
				fn,H323hsCall(callHandle)));
				if(nullTCS && (uh323NullCapSend2(H323hsCall(callHandle))!=0))
				{
					NETERROR(MH323,("%s hsCall(%p) uh323NullCapSend2 failed.\n",
							fn,H323hsCall(callHandle)));
				}
				else {
					/* Store the state into the call handle */
					if(H323controlState(callHandle)== UH323_sControlHeldByLocal)					{
						H323controlState(callHandle) = UH323_sControlHeldByBoth;
					}
					else {
						H323controlState(callHandle) = UH323_sControlHeldByRemote;
					}
				}
				break;
			default :
				NETDEBUG(MSCC,NETLOG_DEBUG3,
					("%s hsCall(%p)Ignoring Null TCS :\n",
					fn,H323hsCall(callHandle)));
				break;
		}
	}
	else 
	{
		switch (controlState)
		{
			case UH323_sControlHeldByBoth: 
			case UH323_sControlHeldByRemote: 
			case UH323_sControlTransportConnected: 
				if(uh323CapSend(H323hsCall(callHandle),
					H323remoteSet(callHandle),
					pH323Data->nlocalset,pH323Data->nodeId,callHandle->ecaps1,callHandle->vendor)!=0)
				{
					NETERROR(MH323,
						("%s hsCall(%p) uh323CapSend failed.Could not forward TCS\n",
							fn,H323hsCall(callHandle)));
				}
				else {
					/* Store the state into the call handle */
					if(H323controlState(callHandle) == UH323_sControlHeldByBoth)
					{
						H323controlState(callHandle) =  UH323_sControlHeldByLocal;
					}
					else {
						H323controlState(callHandle) =  UH323_sControlHeldCapSent;
					}
				}
				freeNodeTree(UH323Globals()->peerhApp, pH323Data->nodeId, 0);
				pH323Data->nodeId = -1;
				break;
			case UH323_sControlIdle: 
				H323remoteTCSNodeId(callHandle) = pH323Data->nodeId;
				pH323Data->nodeId = -1;
				break;
			default :
				freeNodeTree(UH323Globals()->peerhApp, pH323Data->nodeId, 0);
				pH323Data->nodeId = -1;
				NETDEBUG(MSCC,NETLOG_DEBUG2,
					("%s hsCall(%p) Not forwarding Capabilities:\n", 
					fn,H323hsCall(callHandle)));
				break;
		}
	}
_return:
	CacheReleaseLocks(callCache);
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int SCCBridgeOLC(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeOLC() : ";
	CallHandle 		*callHandle;
	char			callIDStr[CALL_ID_STRLEN];
	H323RTPSet		h323Set;
	H323EventData	*pH323Data;
	ChanInfo		chanInfo = {0};
	H323RTPSet		*pH323Set=NULL;
	char			ipstr[24];
	int				sid;
	int				controlState;

    pH323Data = (H323EventData *)(evtPtr->data);
	sid = pH323Data->sid;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering: sid = %d\n",fn,sid));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	controlState = H323controlState(callHandle);

	if ((callHandle->vendor == Vendor_eAvaya) &&
			(controlState == UH323_sControlIdle))
	{
		HCALL					hsCall = (HCALL)NULL;
		UH323CallAppHandle 		*appHandle;
		
		/* Send Facility Faststart - OLC */
		hsCall = H323hsCall(callHandle);
		if (cmCallGetHandle(hsCall, (HAPPCALL *)&appHandle) < 0)
		{
			NETERROR(MSCC,("%s Cannot locate app handle for this call\n", fn));
			CacheReleaseLocks(callCache);
		}
		else
		{

			/* Save the peer Capability Set */
			if(pH323Data->nlocalset)
			{
				INT16		lcn = -1;
				pH323Set =(CMalloc(callCache))(pH323Data->nlocalset*sizeof(H323RTPSet));
				copyRtp2H323RtpSet(pH323Set,pH323Data->localSet,pH323Data->nlocalset);
				if(H323nremoteset(callHandle))
				{
					(CFree(callCache))H323remoteSet(callHandle);
				}
				H323nremoteset(callHandle) = pH323Data->nlocalset;
				H323remoteSet(callHandle) = pH323Set;
				if (callHandle->handle.h323CallHandle.lcnNumber > 0)
					lcn = callHandle->handle.h323CallHandle.lcnNumber;
				CacheReleaseLocks(callCache);
				if (sendFacilityFastStartOpenChannel((HAPPCALL)appHandle, hsCall, lcn) < 0)
				{
					NETERROR(MSCC, ("%s Failed to send facility Fast Start\n",fn));
					goto _return;
				}
					
			}
			else
			{
			   CacheReleaseLocks(callCache);
			}
			
		}
		
		goto _return;
	}

	if((controlState != UH323_sControlConnected) ||
		(sid == 2 && H323outChan(callHandle)[sid].hsChan != NULL) || 
		H323outChan(callHandle)[sid].active || 
		(sid == 0 && H323outChan(callHandle)[2].active && H323outChan(callHandle)[2].codec == T38Fax))
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s controlState = %s. Queueing Event.",
			fn,GetUh323ControlState(controlState)));

		if (H323outChan(callHandle)[sid].active)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG4,
				("%s Dropping Event.", fn));
			CacheReleaseLocks(callCache);
			H323FreeEvData(evtPtr->data);
			free(evtPtr);
			return 0;
		}
		
		SCCQueueEvent(evtPtr);
		// evtPtr should not be free in this case
		CacheReleaseLocks(callCache);
		return 0;
	}


#if 0
	if(pH323Data->nodeId)
	{
		/* In H323 to H323 Case - we don't have rtp address here 
		* Also - we are using nodeId given be other leg - this will be a 
		*  problem in multiple instance. We will have to do a copy 
		*/
		chanInfo.dataTypeHandle = pH323Data->nodeId;
	}
	else 
#endif
	
	if(H323doOlc(callHandle))
	{
		chanInfo.rtcpip = pH323Data->localSet[0].rtpaddr;
		chanInfo.rtcpport = pH323Data->localSet[0].rtpport+1;
		chanInfo.codec = pH323Data->localSet[0].codecType;
		chanInfo.param = pH323Data->localSet[0].param;
		chanInfo.flags = pH323Data->localSet[0].flags;;
		switch(pH323Data->sid)
		{
			case 0:
				/* Always fabricate nodeId */
				copyRtp2H323RtpSet(&h323Set,pH323Data->localSet,pH323Data->nlocalset);
				chanInfo.dataType = cmCapAudio;
				chanInfo.dataTypeHandle = createDataHandleOLC(&h323Set);
				break;
			case 1:
				/* Fix this - this should be duped in networkolc */
				chanInfo.dataType = cmCapVideo;
				chanInfo.dataTypeHandle = pH323Data->nodeId;
				break;
			case 2:
				chanInfo.dataType = cmCapData;
				chanInfo.dataTypeHandle = pH323Data->nodeId;
			break;
		}

		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s %s/%d codec = %d param = %d Data TypeHandle %d flags = 0x%x\n",
			fn,
			FormatIpAddress(chanInfo.rtcpip, ipstr),
			chanInfo.rtcpport,chanInfo.codec,
			pH323Data->localSet[0].param,chanInfo.dataTypeHandle,chanInfo.flags));
		GkOpenChannel(callHandle,&chanInfo,sid);
	}
	else
	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s ignoring OLC as doOlc set to 0\n", fn));
	}
	CacheReleaseLocks(callCache);

_return:
    H323FreeEvData(evtPtr->data);
    free(evtPtr);
    return 0;
}


int SCCBridgeChanConnect(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeChanConnect() : ";
	CallHandle 		*callHandle;
	char			callIDStr[CALL_ID_STRLEN];
	unsigned long	ip;
	unsigned short	port;
	H323EventData	*pH323Data = evtPtr->data;
	char			ipstr[24];
	HCHAN           hsChannel;

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	callHandle = CacheGet(callCache,evtPtr->callID);

	if(callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	if(pH323Data && pH323Data->nlocalset)
	{
		ip = pH323Data->localSet[0].rtpaddr;
		port = pH323Data->localSet[0].rtpport;
	}
	else if(H323nremoteset(callHandle) )
		{
			ip = H323remoteSet(callHandle)[0].rtpaddr;
			port = H323remoteSet(callHandle)[0].rtpport;
		}
	else {
			NETERROR(MSCC,("%s %p ip address not set\n",
				fn,H323hsCall(callHandle)));
			CacheReleaseLocks(callCache);
			goto _return;
		}

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Setting rtp Address to %s/%d: sid = %d\n",
		fn,FormatIpAddress(ip, ipstr),port,pH323Data->sid));

	/* for avaya this fucntion calls certain stack apis which may not make sense for Annex F */
	GkSetChannelAddress(callHandle,ip,port,pH323Data->sid);

	hsChannel = H323inChan(callHandle)[pH323Data->sid].hsChan;

	CacheReleaseLocks(callCache);

	if (cmChannelAnswer(hsChannel) < 0)
	{
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			("%s cmChannelAnswer(%p) returned error\n", fn,hsChannel));
	}

_return:
    H323FreeEvData(evtPtr->data);
    free(evtPtr);
    return 0;
}


/*  - Do not Free localSet. Its used to identify media change when we get 
*		OLC nexte time (for hold). 
*	- Zero out outChanInfo
*	- Drop outgoing channel 
*/
int SCCBridgeCLC(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeCLC() : ";
	CallHandle 		*callHandle;
	char			callIDStr[CALL_ID_STRLEN];
	H323RTPSet		*pH323Set;
	H323EventData	*pH323Data;
	ChanInfo		chanInfo = {0};
	int				controlState;
	HCHAN			outChan;
	int				sid;

    pH323Data = (H323EventData *)(evtPtr->data);
	sid = pH323Data->sid;
	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering: sid=%d\n",fn,sid));

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s. sid=%d\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr),sid));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	controlState = H323controlState(callHandle);
	if ((callHandle->vendor == Vendor_eAvaya) &&
			(controlState == UH323_sControlIdle))
	{
		HCALL					hsCall = (HCALL)NULL;
		UH323CallAppHandle 		*appHandle;
		/* Send Facility Faststart - CLC */

		hsCall = H323hsCall(callHandle);
		if (cmCallGetHandle(hsCall, (HAPPCALL *)&appHandle) < 0)
		{
			NETERROR(MSCC,("%s Cannot locate app handle for this call\n", fn));
			CacheReleaseLocks(callCache);
		}
		else
		{
			INT16			lcn = -1;
			HCHAN			outChan = (HCHAN)NULL;

			outChan = H323outChan(callHandle)[sid].hsChan;
			
			/* Save the peer Capability Set */
			memset(H323remoteSet(callHandle), 0, H323nremoteset(callHandle)*sizeof(H323RTPSet));
			H323nremoteset(callHandle) = 0;
			/*if (buildFacilityFastStartCloseChannels(haCall, hsCall) < 0)*/
			if (callHandle->handle.h323CallHandle.lcnNumber > 0)
				lcn = callHandle->handle.h323CallHandle.lcnNumber;
			else
			{
				/* get lcn from hsChan */
				if (outChan)
				{
					if ((lcn = cmChannelGetNumber(outChan)) < 0)
					{
						NETERROR(MSCC, ("%s Failed to get LCN from hsChan = %p \n",fn, outChan));
					}
				}
			}
			CacheReleaseLocks(callCache);
			if (sendFacilityFastStartCloseChannel((HAPPCALL)appHandle, hsCall, lcn) < 0)
			{
				NETERROR(MSCC, ("%s Failed to send facility Fast Start\n",fn));
			}
			/* close the channel */
			if (outChan)
			{
				if (cmChannelDrop(outChan) < 0)
				{
					DEBUG(MH323, NETLOG_DEBUG1,
						("%s cmChannelDrop returned error %s. outchan = %p, sid=%d\n",
							fn, (char*) CallID2String(evtPtr->callID,callIDStr),outChan,sid));
				}
			}
			goto _return;
		}
	}

#if 0
	/* This interpretation is incorrect */
	controlState = H323controlState(callHandle);
	if(controlState == UH323_sControlHeldByLocal)
	{
		H323controlState(callHandle) = UH323_sControlHeldByBoth;
	}
	else {
		H323controlState(callHandle) = UH323_sControlHeldByRemote;
	}
	NETDEBUG(MH323, NETLOG_DEBUG4, 
		("%s hscall %x changing state from %s to %s\n",
			fn,H323hsCall(callHandle),
			GetUh323ControlState(controlState),
			GetUh323ControlState(H323controlState(callHandle))));
#endif

	outChan = H323outChan(callHandle)[sid].hsChan;
	CacheReleaseLocks(callCache);

	if (cmChannelDrop(outChan) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1,
			("%s cmChannelDrop returned error %s. outchan = %p, sid=%d\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr),outChan,sid));
	}
	
_return:
    H323FreeEvData(evtPtr->data);
    free(evtPtr);
    return 0;
}
/*
	- release the call
	- the handles will be freed when we get call state idle
*/
int SCCLeg2NetworkWORReleaseComp(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCLeg2NetworkWORReleaseComp";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	cmTransportAddress 	cmTransAddr;
	int					len;
	void 				*timerdata;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeReleaseComp;

	memcpy(&pNextEvt->callDetails, &evtPtr->callDetails,
		sizeof(CallDetails));

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,evtPtr->confID,CONF_ID_LEN);

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	//extract parameters to be sent to source

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	callHandle = CacheGet(callCache, evtPtr->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		goto _error;
	}
	// rasReason on Leg2 was stored in its callHandle, copy it to callDetails
	pNextEvt->callDetails.rasReason = evtPtr->callDetails.rasReason = callHandle->rasReason;
	pNextEvt->callDetails.lastEvent = callHandle->lastEvent;
	if(h323HuntError(pNextEvt->callDetails.callError,pNextEvt->callDetails.cause-1))
	{
		pNextEvt->callDetails.flags |= HUNT_TRIGGER;
	}

	CloseFceHoles(callHandle);

	timedef_cur(&callHandle->callEndTime);

	// mark the call as billed
	GkCallSetRemoteErrorCodes(callHandle, &evtPtr->callDetails);
	BillCall(callHandle, CDR_CALLDROPPED);	

_error:
	CacheReleaseLocks(callCache);

	// Handle Rollover in leg1

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		// Can't do much 
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	}

	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
#ifdef _nonfaststart
	if (CacheFind(callCache,eventPtr->callID,&callHandleBlk2,sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle  1 \n",fn));
		return -1;
	}

	if (CacheFind(confCache,callHandleBlk2.confID,&confHandleBlk,sizeof(ConfHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate ConfHandle\n",fn));
		return -1;
	}

	// Find Leg1 CallHandle
	if (CacheFind(callCache, confHandleBlk.callID[0],&callHandleBlk1,sizeof(CallHandle))<0 )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle for call 0\n",fn));
		return -1;
	}
	
	// Make Sure we can get everything we need for rollover b4 dropping the call
	if(IsRolledOver(H323dialledNumber(&callHandleBlk1)) &&
                (callHandleBlk1.rolledOver == 0))

	{
		NETDEBUG(MSCC,NETLOG_DEBUG3,
				("%s Call is Rolled Over .. trying alternate number:\n",fn));

		if (callHandleBlk1.rolloverTimer != NULL)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG3,
					("%s Deleting rollOver Timer:\n",fn));
			if(timerDeleteFromList(&localConfig.timerPrivate, 
								callHandleBlk1.rolloverTimer, &timerdata))
			{
				if(timerdata)
				{
					free(timerdata);
				}
			}
			callHandleBlk1.rolloverTimer = 0;
			if (CacheOverwrite(callCache, &callHandleBlk1, sizeof(CallHandle), 
				callHandleBlk1.callID) < 0)
			{
				NETERROR(MSCC,
					("%s Failed to overwrite callHandle in callCache\n",fn));
			}
		}

		if(CallRollover(callHandleBlk2.callID) == 0)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG3,
				("%s CallRollover succeded:\n",fn));
			//HACK - to prevent the state machine from passing releascomp
			// to the other leg
			return -1;
		}
		// else - couldn't rollover return 0  and pass releasecomp to other leg
	}
#endif
	return 0;
}


//	Handle Incoming Alerting
//  Extract Parameters from H323 Alerting to send to Leg1 source
int SCCNetworkAlerting(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkAlerting() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	int					nodeId = -1;
	cmTransportAddress 	cmTransAddr;
	int					addrlen;
	unsigned int		rra;
	unsigned short		rrp;
	int					retval = -1;
	char				ipstr[24];
	int 				propNodeId = -1, newNodeId = -1, q931NodeId = -1;
	int					sid = 0;
	h221VendorInfo		h221ns;
	int vendor;
	int dstInfoNodeId = -1;
	BOOL isstring;
	int					progressIndicator = -1, progressDescription = -1;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	
	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Log Error and try to continue
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	if(cmCallGetParam(H323hsCall(callHandle),
				cmParamH245Address,
				0,
				&addrlen,
				(char *)&cmTransAddr) >0)
	{
		H323h245ip(callHandle) = ntohl(cmTransAddr.ip);
		H323h245port(callHandle) = cmTransAddr.port;
	}

    if( cmCallGetParam(H323hsCall(callHandle),
        cmParamAlertingFastStart,
        0,
        &nodeId,
        NULL) >=0)
    {
        NETDEBUG(MSCC,NETLOG_DEBUG4,
            ("%s cmCallGetParam nodeid = %d. Connect\n",
            fn,nodeId));
	// Faststart succeeded. Don't respond to Open logical Channel 
	H323doOlc(callHandle) = 0;
	callHandle->fastStartStatus = 1;
		/* only if we are not doing h245 routing will we decode fast start */
		if(!H323outChan(callHandle)[sid].ip)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG4,
				("%s fast connect Connect channel address not set. nodeid = %d, deferring\n",
				fn,nodeId));
			H323chanEvt(callHandle) = SCC_eNetworkAlerting;
			CacheReleaseLocks(callCache);
			goto _return;
		}
    }

    if (cmCallGetParam(H323hsCall(callHandle), 
                           cmParamFullDestinationInfo, 0, 
                           &dstInfoNodeId, NULL) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No src info nodeid found\n", fn));
		dstInfoNodeId = -1;
    }

	{
		HPVT hVal = cmGetValTree(UH323Globals()->hApp);
		HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);

        cmMeiEnter(UH323Globals()->hApp);
		cmMeiEnter(UH323Globals()->peerhApp);
        if((propNodeId = cmGetProperty((HPROTOCOL)H323hsCall(callHandle))) <0)
        {
            NETERROR(MH323,("%s cmGetProperty failed to get nodeId\n", fn ));
        }
		else
		{
        	if ((q931NodeId = pvtGetNodeIdByPath(hVal, propNodeId ,
            "alerting")) > 0)
        	{
				if((newNodeId = pvtAddRoot (peerhVal,hSynAlerting,0,NULL))>0)
				{
					if(pvtSetTree(peerhVal,newNodeId,hVal,q931NodeId) < 0)
					{
						NETERROR(MH323, 
							("%s could not copy q931AlertingNodeId\n",fn));
					}
				}
				else 
				{
                	NETERROR(MH323, 
						("%s pvtAddRoot failed.Could not get new NodeId\n",fn));
				}

				if (pvtGetByPath(hVal, q931NodeId, 
					"message.alerting.progressIndicator.octet4.progressDescription", 
					NULL, (INT32 *) &progressDescription, &isstring) < 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s Could not get Progress Description\n",fn));
					progressDescription = -1;
				}
				else
				{
					// PI is present
					progressIndicator = 1;
					NETDEBUG(MH323, NETLOG_DEBUG4,
						("%s PI is present, ProgressDescription = %d\n",
						fn, progressDescription));
				}
        	}
			else 
			{
                NETERROR(MH323, 
					("%s Could not get alerting nodeId from Property tree\n",fn));
			}
		}

		if (dstInfoNodeId >= 0)
		{	
			if (pvtGetByPath(hVal, dstInfoNodeId,
				"vendor.vendor.t35CountryCode", NULL,
					(INT32 *)&h221ns.t35CountryCode, &isstring) < 0)
			{
				NETDEBUG(MH323, NETLOG_DEBUG1,
					("%s t35 country code not present\n",fn));
					h221ns.t35CountryCode = -1;
			}

			if (pvtGetByPath(hVal, dstInfoNodeId,
				"vendor.vendor.manufacturerCode", NULL,
					(INT32 *)&h221ns.manufacturerCode, &isstring) < 0)
			{
				NETDEBUG(MH323, NETLOG_DEBUG1,
					("%s mfr code not present\n",fn));
				h221ns.manufacturerCode = -1;
			}
	
			vendor = GetVendorFromH221Info(&h221ns);
			if (vendor > 0)
			{
				callHandle->vendor = vendor;
			}
		}

        cmMeiExit(UH323Globals()->hApp);
		cmMeiExit(UH323Globals()->peerhApp);
	}

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeAlerting;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);

	if (callHandle->fastStartStatus &&  
		(callHandle->ecaps1 & ECAPS1_PIONFASTSTART))
	{
		pH323Data->msgFlags |= MSGFLAG_PI;
	}
#if 0 // peer might free this
	pH323Data->nodeId = nodeId;
#endif
	pH323Data->q931NodeId = newNodeId;

	pH323Data->pi_IE = progressIndicator;
	pH323Data->progress = progressDescription;

	if(routeH245==0)
	{
		pH323Data->h245ip = H323h245ip(callHandle);
		pH323Data->h245port = H323h245port(callHandle);
	}

    if(H323outChan(callHandle)[sid].ip)
    {
		updateNonStandardCodecType(callHandle, sid, 1);
        pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
        memset(pH323Data->localSet,0,sizeof(RTPSet));
        callHandle->lastMediaIp = pH323Data->localSet[0].rtpaddr = H323outChan(callHandle)[sid].ip;
        callHandle->lastMediaPort = pH323Data->localSet[0].rtpport = H323outChan(callHandle)[sid].port;
        pH323Data->localSet[0].codecType = H323outChan(callHandle)[sid].codec;
        pH323Data->localSet[0].param = H323outChan(callHandle)[sid].param;
        pH323Data->nlocalset = 1;
		NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s ip=%s/%d codec = %d/%d\n",
					fn,FormatIpAddress(pH323Data->localSet[0].rtpaddr, ipstr),
					pH323Data->localSet[0].rtpport,
					pH323Data->localSet[0].codecType,
					pH323Data->localSet[0].param));
    }

	openH245RemotePinhole(fn, callHandle, "ALERTING", &pH323Data->h245ip, &pH323Data->h245port);

	if (timedef_iszero(&callHandle->callRingbackTime))
	{
		timedef_cur(&callHandle->callRingbackTime);
	}

	CacheReleaseLocks(callCache);

	// uh323FastStart2RTPSet(H323hsCall(&callHandleBlk),pH323Data);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	}

_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return 0;
}

//	Handle IncomingConnect 
//  Extract Parameters from H323 Alerting to send to Leg1 source
int SCCNetworkConnect(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkConnect() : ";
	CallHandle 			*callHandle,callHandleBlk = {0};
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	cmTransportAddress 	cmTransAddr;
	int					len;
	int					nodeId = -1;
	int					rv = -1;
	unsigned int		rra;
	unsigned short		rrp;
	int					retval = -1;
	H323RTPSet			*pRtpSetRx;
	char				ipstr[24];
	int					callError = SCC_errorNone;
	int					cause = 0;
	int					sid = 0;
	h221VendorInfo		h221ns;
	int vendor;
	int dstInfoNodeId = -1;
	BOOL isstring;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		callError = SCC_errorNoCallHandle;
		goto _return;
	}

	// Retrieve h245 address if we are not doing h245 actively
	if(cmCallGetParam(H323hsCall(callHandle),
				cmParamH245Address,
				0,
				&len,
				(char *)&cmTransAddr) >= 0)
	{
		H323h245ip(callHandle) = ntohl(cmTransAddr.ip);
		H323h245port(callHandle) = cmTransAddr.port;
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s retrieved h245address %s:%lu\n", 
			fn,FormatIpAddress(H323h245ip(callHandle), ipstr), H323h245port(callHandle)));
	}

	if( cmCallGetParam(H323hsCall(callHandle),
		cmParamConnectFastStart,
		0,
		&nodeId,
		NULL) >=0)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s cmCallGetParam nodeid = %d. Connect\n",
			fn,nodeId));

		// Faststart succeeded. Don't respond to Open logical Channel 
		H323doOlc(callHandle) = 0;
		callHandle->fastStartStatus = 1;

		/* only if we are not doing h245 routing will we decode fast start */
		if ( routeH245 && (!H323outChan(callHandle)[sid].ip) &&
			callHandle->fastStart)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG4,
				("%s fast connect Connect channel address not set. nodeid = %d, deferring\n",
				fn,nodeId));
			H323chanEvt(callHandle) = SCC_eNetworkConnect;
			callHandle->state = SCC_sWaitOnRemote;
			CacheReleaseLocks(callCache);
			goto _return;
		}
	}

	if (callHandle->fastStart && !H323outChan(callHandle)[sid].ip) 
	{
			callHandle->fastStart = 0;
			NETDEBUG(MSCC,NETLOG_DEBUG4,
				("%s Fast Start failed.\n", fn));
	}

    if (cmCallGetParam(H323hsCall(callHandle), 
                           cmParamFullDestinationInfo, 0, 
                           &dstInfoNodeId, NULL) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No src info nodeid found\n", fn));
		dstInfoNodeId = -1;
    }

    cmMeiEnter(UH323Globals()->hApp);

	if (dstInfoNodeId >= 0)
	{	
		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.t35CountryCode", NULL,
				(INT32 *)&h221ns.t35CountryCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s t35 country code not present\n",fn));
				h221ns.t35CountryCode = -1;
		}

		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.manufacturerCode", NULL,
				(INT32 *)&h221ns.manufacturerCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s mfr code not present\n",fn));
			h221ns.manufacturerCode = -1;
		}

		vendor = GetVendorFromH221Info(&h221ns);
		if (vendor > 0)
		{
			callHandle->vendor = vendor;
		}
	}

    cmMeiExit(UH323Globals()->hApp);

	ST_connect();
	if(H323outChan(callHandle)[sid].ip)
	{
		updateNonStandardCodecType(callHandle, sid, 1);
		pRtpSetRx = (H323RTPSet *) (CMalloc(callCache))(1*sizeof (H323RTPSet));
		memset(pRtpSetRx,0,sizeof(H323RTPSet));
		callHandle->lastMediaIp = pRtpSetRx->rtpaddr = H323outChan(callHandle)[sid].ip;
		callHandle->lastMediaPort = pRtpSetRx->rtpport = H323outChan(callHandle)[sid].port;
		pRtpSetRx->codecType = H323outChan(callHandle)[sid].codec;
		pRtpSetRx->param = H323outChan(callHandle)[sid].param;
		H323nlocalset(callHandle) = 1;
		H323localSet(callHandle) = pRtpSetRx;
	}

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeConnect;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	len = sizeof(cmTransportAddress);
	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);


	/* CONNECT from external to internal, open hole for H245 (if not already done) */
	if (H323remoteBundleId(callHandle) == 0)
		openH245RemotePinhole(fn, callHandle, "CONNECT", &pH323Data->h245ip, &pH323Data->h245port);

	timedef_cur(&callHandle->callConnectTime);
	CacheReleaseLocks(callCache);

	if (routeH245 !=0)
	{
		// If we have peer H245 address
		if (H323h245ip(&callHandleBlk) &&
			(H323controlState(&callHandleBlk) == UH323_sControlIdle))
		{
			if(callHandleBlk.fastStart)
			{
			DEBUG(MH323, NETLOG_DEBUG1, ("%s Opening H.245\n",fn));

			if (cmCallConnectControl(H323hsCall(&callHandleBlk)) < 0)
			{
				NETERROR(MH323, ("%s cmCallConnectControl failed\n",fn));
			}
			}
		}
	}

	pH323Data->h245ip = H323h245ip(&callHandleBlk);
	pH323Data->h245ip = H323h245ip(&callHandleBlk);
	pH323Data->h245port = H323h245port(&callHandleBlk);
	
#if 0 // peer might free this
	if (!callHandleBlk.fastStart && nodeId)
	{
		pH323Data->nodeId = nodeId;
	}
#endif

	/*	relay rtpSet if we have it */
	if(H323outChan(&callHandleBlk)[sid].ip)
	{
		pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
		memset(pH323Data->localSet,0,sizeof(RTPSet));
		pH323Data->localSet[0].rtpaddr = H323outChan(&callHandleBlk)[sid].ip;
		pH323Data->localSet[0].rtpport = H323outChan(&callHandleBlk)[sid].port;
		pH323Data->localSet[0].codecType = H323outChan(&callHandleBlk)[sid].codec;
		pH323Data->localSet[0].param = H323outChan(&callHandleBlk)[sid].param;
		pH323Data->nlocalset = 1;
		NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s ip=%s/%d codec = %d/%d\n",
				fn,FormatIpAddress(pH323Data->localSet[0].rtpaddr, ipstr),
				pH323Data->localSet[0].rtpport,
				pH323Data->localSet[0].codecType,
				pH323Data->localSet[0].param));
	}

	/*  We have to delete the rollover timer when we get the connect
		for the second call. But we have to be careful for butterfly
		case where we get connect in leg2 for the source also. */	
	// Handle Rollover in leg1

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
		GkCallDropReasonNone(H323hsCall(&callHandleBlk), SCC_errorH323Internal);
		cmCallDrop(H323hsCall(&callHandleBlk));
	}
	rv = 0;

_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return rv;
}

/*	Handle Incoming Chanel Connect 
*	This routine just generates an event in the next stateMachine. Everything 
*	else is already done in uh323callcb.c
*/
int SCCNetworkChanConnect(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkChanConnect() : ";
	CallHandle 			*callHandle,callHandleBlk = {0};
	H323EventData		*pH323Data,*evtData;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	cmTransportAddress 	cmTransAddr;
	int					len;
	int					rv = -1;
	int					cause = 0;
	int					sid;

	evtData = (H323EventData *)evtPtr->data;
	sid = evtData->sid;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering: sid = %d\n",fn,sid));
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));
	H323outChan(callHandle)[sid].mediaChange = 0;

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeChanConnect;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);

	if(H323outChan(callHandle)[sid].ip)
	{
		updateNonStandardCodecType(callHandle, sid, 1);
		pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
		memset(pH323Data->localSet,0,sizeof(RTPSet));
		pH323Data->localSet[0].rtpaddr = H323outChan(callHandle)[sid].ip;
		pH323Data->localSet[0].rtpport = H323outChan(callHandle)[sid].port;
		pH323Data->localSet[0].codecType = H323outChan(callHandle)[sid].codec;
		pH323Data->localSet[0].param = H323outChan(callHandle)[sid].param;
		pH323Data->nlocalset = 1;
		pH323Data->sid = sid;

		if(sid == 0 || pH323Data->localSet[0].codecType == T38Fax)
		{
			if(callHandle->lastMediaIp && callHandle->lastMediaPort && 
				((callHandle->lastMediaIp != pH323Data->localSet[0].rtpaddr)||
				(callHandle->lastMediaPort != pH323Data->localSet[0].rtpport)))
			{
				pH323Data->mediaChanged = 1;
				NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Media Changed sid = %d codecType = %d"
				" last media Address = 0x%x/%d new media address = 0x%lx/%d\n", fn,sid,
				pH323Data->localSet[0].codecType,callHandle->lastMediaIp, callHandle->lastMediaPort,
				pH323Data->localSet[0].rtpaddr,pH323Data->localSet[0].rtpport));
			}

			callHandle->lastMediaIp = pH323Data->localSet[0].rtpaddr;
			callHandle->lastMediaPort = pH323Data->localSet[0].rtpport;
		}

	}

	CacheReleaseLocks(callCache);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,
			("%s bridgeH323EventProcessor failed. Dropping Call !!\n",fn));

		GkCallDropReasonNone(H323hsCall(&callHandleBlk), SCC_errorH245Incomplete);
		cmCallDrop(H323hsCall(&callHandleBlk));
	}

	rv = 0;

_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return rv;
}

int SCCNetworkOLC(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkOLC() : ";
	CallHandle 			*callHandle,callHandleBlk = {0};
	H323EventData		*pH323Data,*evtData;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	int					len;
	int					rv = -1;
	char 				ipstr[24];
	int					cause = 0;
	int					sid;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 				peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	int					newNodeId = -1;

	evtData = (H323EventData *)evtPtr->data;
	sid = evtData->sid;
	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering: sid = %d\n",fn,sid));
	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _return;
	}

	H323inChan(callHandle)[sid].mediaChange = 0;


	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeOLC;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);

	pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
	memset(pH323Data->localSet,0,sizeof(RTPSet));
	updateNonStandardCodecType(callHandle, sid, 0);
	pH323Data->localSet[0].rtpaddr = H323inChan(callHandle)[sid].rtcpip;

	pH323Data->localSet[0].rtpport = H323inChan(callHandle)[sid].rtcpport-1;
	pH323Data->localSet[0].codecType = H323inChan(callHandle)[sid].codec;
	pH323Data->localSet[0].param = H323inChan(callHandle)[sid].param;
	pH323Data->localSet[0].flags = H323inChan(callHandle)[sid].flags;
	pH323Data->nlocalset = 1;
	pH323Data->sid = sid;
	if(sid == 0 || pH323Data->localSet[0].codecType == T38Fax)
	{
		if(callHandle->lastMediaIp && callHandle->lastMediaPort && 
			((callHandle->lastMediaIp != pH323Data->localSet[0].rtpaddr)||
			(callHandle->lastMediaPort != pH323Data->localSet[0].rtpport)) &&
			!((callHandle->vendor == Vendor_eCisco) && (pH323Data->localSet[0].rtpport == 4000)))
		{
			pH323Data->mediaChanged = 1;
			NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Media Changed sid = %d codecType = %d"
			" last media Address = 0x%x/%d new media address = 0x%lx/%d\n", fn,sid,
			pH323Data->localSet[0].codecType,callHandle->lastMediaIp, callHandle->lastMediaPort,
			pH323Data->localSet[0].rtpaddr,pH323Data->localSet[0].rtpport));
		
		}

		callHandle->lastMediaIp = pH323Data->localSet[0].rtpaddr;
		callHandle->lastMediaPort = pH323Data->localSet[0].rtpport;
	}
	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));
	CacheReleaseLocks(callCache);
#if 1 // peer may free this
	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);
	if(pH323Data->sid == 1)
	{
		if((newNodeId = pvtAddRoot (hVal,hSynVideo,0,NULL))>0)
		{
			if(pvtSetTree(hVal,newNodeId,hVal,H323inChan(&callHandleBlk)[sid].dataTypeHandle) < 0)
			{
				NETERROR(MH323, ("%s could not copy OLC NodeID\n",fn));
			}
		}
		else {
			NETERROR(MH323, 
				("%s pvtAddRoot failed.Could not get new NodeId\n",fn));
		}
		pH323Data->nodeId = newNodeId;
	}
    else if(pH323Data->sid == 2)
    {
        if((newNodeId = pvtAddRoot (peerhVal,hSynData,0,NULL))>0)
        {
            if(pvtSetTree(peerhVal,newNodeId,hVal,H323inChan(&callHandleBlk)[sid].dataTypeHandle) < 0)
            {
                NETERROR(MH323, ("%s could not copy OLC NodeID\n",fn));
            }
        }
        else {
            NETERROR(MH323, 
                ("%s pvtAddRoot failed.Could not get new NodeId\n",fn));
        }
        pH323Data->nodeId = newNodeId;
//        pvtPrintStd(peerhVal,newNodeId,62);
    }
	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);
#endif



	NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s codecType = %d rtpaddr = %s port = %d param = %d. mediaChange = %lu flags =  0x%x\n",fn,
			pH323Data->localSet[0].codecType,
			FormatIpAddress(pH323Data->localSet[0].rtpaddr, ipstr),
			pH323Data->localSet[0].rtpport,pH323Data->localSet[0].param,
			pH323Data->mediaChanged,pH323Data->localSet[0].flags));

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,
			("%s bridgeH323EventProcessor failed. Dropping Call !!\n",fn));
		GkCallDropReasonNone(H323hsCall(&callHandleBlk), SCC_errorH245Incomplete);
		cmCallDrop(H323hsCall(&callHandleBlk));
	}
	rv = 0;
_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return rv;
}

/*  Free remoteSet 
*/
int SCCNetworkCLC(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkCLC() : ";
	CallHandle 			*callHandle,callHandleBlk = {0};
	H323EventData		*pH323Data,*evtData;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	int					len;
	int					rv = -1;
	int					cause = 0;
	int					sid;

	evtData = (H323EventData *)evtPtr->data;
	sid = evtData->sid;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n sid=%d",fn,sid));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s sid = %d\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr),sid));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	/* I don't think we need to play with control state here */
	H323doOlc(callHandle) = 1;
	
	if (H323remoteSet(callHandle)!= NULL)
	{
		(CFree(callCache))(H323remoteSet(callHandle));
	}
	H323remoteSet(callHandle) = NULL;
	H323nremoteset(callHandle) = 0;

	H323inChan(callHandle)[sid].hsChan = NULL;
	H323inChan(callHandle)[sid].active = FALSE;

	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));
	CacheReleaseLocks(callCache);

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Data->sid = sid;
	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeCLC;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandleBlk.confID,CONF_ID_LEN);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,
			("%s bridgeH323EventProcessor failed sid = %d. Dropping Call !!\n",fn,sid));

		GkCallDropReasonNone(H323hsCall(&callHandleBlk), SCC_errorH245Incomplete);
		cmCallDrop(H323hsCall(&callHandleBlk));
	}
	rv = 0;

_return:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return rv;
}

int SCCQueueEvent(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCQueueEvent() : ";
	CallHandle 			*callHandle;
	int					rv = -1;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle\n",fn));
		CacheReleaseLocks(callCache);
		goto _error;
	}

	if(listAddItem(callHandle->evtList,evtPtr)!=0)
	{
		NETERROR(MSCC,("%s Failed to queue Event\n",fn));
		CacheReleaseLocks(callCache);
		goto _error;

	}
	CacheReleaseLocks(callCache);
	return 0;

_error:
	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);
	return -1;
}

/* Assumes that CacheLocks are acquired */
int processPendingEvents(CallHandle *callHandle)
{
	static char 		fn[] = "processPendingEvents() : ";
	int					rv = -1;
	SCC_EventBlock 		*evtPtr = NULL;
	char 				str[256] = {0};

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	while((evtPtr = listDeleteFirstItem(callHandle->evtList)))
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,("%s Processing Event %s\n",
			fn,sccEventBlockToStr(evtPtr, str)));
		//if(bridgeH323EventProcessor(evtPtr)!=0)
		if(callHandle->bridgeEventProcessor)
		{
			if(callHandle->bridgeEventProcessor(evtPtr)!=0)
			{
				NETERROR(MH323,
					("%s bridgeEventProcessor failed!\n",
					fn));
			}
		}
	}
	return 0;
}

void
copyH323Rtp2RtpSet(RTPSet rtpSet[],H323RTPSet h323Set[],int nset)
{
	static char	fn[] = "copyH323Rtp2RtpSet";
	int 	i;
	char	ipstr[24];

	for(i = 0; i<nset; ++i)
	{
		rtpSet[i].codecType = h323Set[i].codecType;
		rtpSet[i].rtpaddr = h323Set[i].rtpaddr;
		rtpSet[i].rtpport = h323Set[i].rtpport;
		rtpSet[i].param	= h323Set[i].param;
		rtpSet[i].direction = h323Set[i].direction;
		rtpSet[i].flags = h323Set[i].flags;
#if 1 
		rtpSet[i].dataTypeHandle = h323Set[i].dataTypeHandle;
#endif
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s codecType = %d rtpaddr = %s port = %d param = %d flags = 0x%x\n",fn,
			rtpSet[i].codecType,FormatIpAddress(rtpSet[i].rtpaddr, ipstr),
			rtpSet[i].rtpport,rtpSet[i].param, rtpSet[i].flags));
	}


	return;
}

void
copyRtp2H323RtpSet(H323RTPSet h323Set[],RTPSet rtpSet[],int nset)
{
	static char	fn[] = "copyRtp2H323RtpSet";
	int 	i;
	char	ipstr[24];

	NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s Copying Rtp to H323Rtp\n", fn));

	for(i = 0; i<nset; ++i)
	{
		h323Set[i].codecType = rtpSet[i].codecType;
		h323Set[i].rtpaddr = rtpSet[i].rtpaddr;
		h323Set[i].rtpport = rtpSet[i].rtpport;
		h323Set[i].param	= rtpSet[i].param;
		h323Set[i].direction = rtpSet[i].direction;
		h323Set[i].flags = rtpSet[i].flags;
#if 1 
		h323Set[i].dataTypeHandle = rtpSet[i].dataTypeHandle;
#endif

		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s codecType = %d rtpaddr = %s port = %d param = %d flags = 0x%x\n",fn,
			h323Set[i].codecType,FormatIpAddress(h323Set[i].rtpaddr, ipstr),
			h323Set[i].rtpport, h323Set[i].param,h323Set[i].flags));
	}
	return;
}

int
SCC_CallStateIdle(SCC_EventBlock *evtPtr)
{
	char fn[] = "SCC_CallStateIdle():";
	CallHandle	*callHandle;
	char				callIDStr[CALL_ID_LEN];

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s Unable to locate CallHandle %s\n",
			fn,
			(char*) CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}

	GwPortFreeCall(&callHandle->phonode,1, callHandle->callSource);

	// free the port for the other leg
	if(!callHandle->callSource && (callHandle->flags & FL_FREE_DESTPORT))
	{
		callHandle->flags &= ~FL_FREE_DESTPORT;
		GwPortFreeCall(&callHandle->rfphonode, 1, 1);
	}

	NETDEBUG(MSCC, NETLOG_DEBUG1, 
		("%s Deleting Call Handle for leg %d\n", 
				fn, callHandle->leg));
	GisDeleteCallFromConf(evtPtr->callID,
			evtPtr->confID);

	CacheDelete(guidCache, callHandle->handle.h323CallHandle.guid);
	callHandle = CallDelete(callCache, callHandle->callID);

	if(callHandle)
	{
		GisFreeCallHandle(callHandle);
	}
	else 
	{
		NETDEBUG(MSCC,NETLOG_DEBUG1,
			("%s failed to find callHandle in cache call\n", fn));
	}

_return:
	CacheReleaseLocks(callCache);

	return 0;
}

int
SCC_NetworkCallStateIdle(SCC_EventBlock *evtPtr)
{
	char fn[] = "SCC_NetworkCallStateIdle():";
	CallHandle	*callHandle;
	char		callIDStr[CALL_ID_LEN];
	char            callID2[CALL_ID_LEN];
	CallHandle      *callHandle2;
	int             freePort = 1;

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}

	CloseFceHoles(callHandle);

	getPeerCallID(evtPtr->confID, evtPtr->callID, callID2);
	if((callHandle2 = CacheGet(callCache, callID2)) && !callHandle2->callSource && callHandle2->remotecontact_list)
	{
		// if the other call handle exists and has a remote contact list, which means
		// it is leg1, make it responsible for freeing the port
		freePort = 0;
		callHandle2->flags |= FL_FREE_DESTPORT;
	}

	if(freePort)
	{
		// we are freeing the port
		if(callHandle2 && !callHandle2->callSource && (callHandle2->flags & FL_FREE_DESTPORT))
		{
			// unset the flag so that the other leg does not try to
			// free this port again
			callHandle2->flags &= ~FL_FREE_DESTPORT;
		}
		GwPortFreeCall(&callHandle->phonode,1, callHandle->callSource);
	}

	if((callHandle->flags & FL_FREE_DESTPORT) && !callHandle->callSource && !callHandle2)
	{
		// this will be executed only for leg1
		// free the remote port if the flag is set
		// also check if the other leg is present. 
		// if we are deleting leg1 before leg2, leg2 will try to delete the port again
		// so, we should delete this port only if leg2 is not present
		callHandle->flags &= ~FL_FREE_DESTPORT;
		GwPortFreeCall(&callHandle->rfphonode, 1, 1);
	}

	H323hsCall(callHandle) = 0;

	// Just log the fact that the DRQ has arrived yet or not.
	// The call handle MUST be deleted.

	if(H323waitForDRQ(callHandle))
	{
		NETDEBUG(MSCC, NETLOG_DEBUG1, 
		("%s DRQ for Call Handle for leg %d not arrived yet \n", 
		fn, callHandle->leg));
	}

	NETDEBUG(MSCC, NETLOG_DEBUG1, 
		("%s Deleting Call Handle for leg %d\n", 
			fn, callHandle->leg));

	GisDeleteCallFromConf(evtPtr->callID,
			evtPtr->confID);

	CacheDelete(guidCache, callHandle->handle.h323CallHandle.guid);
	callHandle = CallDelete(callCache, callHandle->callID);

	if(callHandle)
	{
		GisFreeCallHandle(callHandle);
	}
	else 
	{
		NETDEBUG(MSCC,NETLOG_DEBUG1,
		("%s failed to find callHandle in cache call\n", fn));
	}

_return:
	CacheReleaseLocks(callCache);

	H323FreeEvData((H323EventData *)(evtPtr->data));
	free(evtPtr);

	return 0;
}

int
SCC_ChannelStateConnected(SCC_EventBlock *evtPtr)
{
	char fn[] = "SCC_ChannelStateConnected():";
	CallHandle	*callHandle;
	char callIDStr[CALL_ID_LEN];
	SCC_EventBlock sccEvt = { 0 };

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,(char*) CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}

	// fwd logical channel i.e. remote's receive
	memcpy(&sccEvt.callID, callHandle->callID, CALL_ID_LEN);
	memcpy(&sccEvt.confID, callHandle->confID, CONF_ID_LEN);

	/* The Channel is connected now - subsequently any time we close 
	*	and reopen channels iServer has to be involved
	*/
	/* set this to 1 only if we have explicitly closed it
	* H323doOlc(&callHandleBlk1) = 1; 
	*/
	if(H323chanEvt(callHandle))
	{
		/* This is the case when we have received fastconnect or fast
		*  Alerting. The event was differed in wait for chan connect
		*/
		sccEvt.event = (H323chanEvt(callHandle));
		H323chanEvt(callHandle) = 0;
	}
	else {
		sccEvt.event = SCC_eNetworkChanConnect;
		sccEvt.data = evtPtr->data;
		evtPtr->data = NULL;
	}

_return:
	CacheReleaseLocks(callCache);

	H323FreeEvent(evtPtr);

	SCC_DelegateEvent(&sccEvt);

	return 0;
}


int 
SCCBridgeRequestModeAck(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeRequestModeAck():";
	CallHandle 		*callHandle;
	int 			controlState;
	char			callIDStr[CALL_ID_STRLEN];
	char 			*responseName;
	H323EventData	*pH323Data;
#if 0

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

    pH323Data = (H323EventData *)(evtPtr->data);
    responseName = (char *)(pH323Data->modeData);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if (responseName == NULL)
	{
		NETERROR(MSCC,("%s responseName is NULL\n", fn));
		goto _return;
	}

	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}
	
	controlState = H323controlState(callHandle);

	NETDEBUG(MSCC,NETLOG_DEBUG3,
		("%s hsCall(0x%x) controlState = %s.\n",
		fn,H323hsCall(callHandle),GetUh323ControlState(controlState)));

	if (cmCallRequestModeAck(H323hsCall(callHandle), responseName) < 0)
	{
		NETERROR(MSCC,("%s cmCallRequestModeAck error for %s\n", fn, responseName));
	}

_return:
	CacheReleaseLocks(callCache);
#endif
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int 
SCCBridgeRequestModeReject(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCBridgeRequestModeReject():";
	CallHandle 		*callHandle;
	int 			controlState;
	char			callIDStr[CALL_ID_STRLEN];
	H323EventData	*pH323Data;
	char 			*causeName;

#if 0
	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

    causeName = (char *)(pH323Data->modeData);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if (causeName == NULL)
	{
		NETERROR(MSCC,("%s causeName is NULL\n", fn));
		goto _return;
	}

	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	}
	
	controlState = H323controlState(callHandle);

	NETDEBUG(MSCC,NETLOG_DEBUG3,
		("%s hsCall(0x%x) controlState = %s.\n",
		fn,H323hsCall(callHandle),GetUh323ControlState(controlState)));

	if (cmCallRequestModeReject(H323hsCall(callHandle), causeName) < 0)
	{
		NETERROR(MSCC,("%s cmCallRequestModeReject error for %s\n", fn, causeName));
	}

_return:
	CacheReleaseLocks(callCache);
#endif
	H323FreeEvData(evtPtr->data);
	free(evtPtr);
	return 0;
}

int 
SCCNetworkRequestMode(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkRequestMode() : ";
	CallHandle 			*callHandle,callHandleBlk = {0};
	H323EventData		*pH323Data,*inEvtH323Data;
	char				callIDStr[CALL_ID_STRLEN] = {0};
	SCC_EventBlock 		*pNextEvt;
	int					len;
	int					rv = -1;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	H323doOlc(callHandle) = 1;
	memcpy(&callHandleBlk,callHandle,sizeof(CallHandle));
	CacheReleaseLocks(callCache);

	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pNextEvt->data = (void *) pH323Data;
	pNextEvt->event = SCC_eBridgeRequestMode;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandleBlk.confID,CONF_ID_LEN);
	if((inEvtH323Data = evtPtr->data))
	{
		pH323Data->nodeId = inEvtH323Data->nodeId; 
		pH323Data->modeStatus = inEvtH323Data->modeStatus;
		strcpy(pH323Data->modeName,inEvtH323Data->modeName); 
		inEvtH323Data->nodeId = 0;
	}

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,
			("%s bridgeH323EventProcessor failed!!\n",fn));
	}
	rv = 0;

_return:
	H323FreeEvent(evtPtr);
	return rv;
}

int 
SCCNetworkRequestModeAck(SCC_EventBlock *evtPtr)
{
	return(0);
}
int
SCCNetworkRequestModeReject(SCC_EventBlock *evtPtr)
{
	return(0);
}

int
SCCBridgeRequestMode(SCC_EventBlock *evtPtr)
{
	char 			fn[] = "SCCBridgeRequestMode():";
	CallHandle 		*callHandle;
	char 			callID[CALL_ID_LEN] = {0};
	HCALL           hsCall;
	int				modeId;
	H323EventData	*pH323Data;
	char			callIDStr[CALL_ID_STRLEN];
	int				controlState;
	int				status;

	DEBUG(MH323, NETLOG_DEBUG1, (" Entering %s\n",fn));

	pH323Data = (H323EventData *)evtPtr->data;
	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	callHandle = CacheGet(callCache,evtPtr->callID);
	if(callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	} 
	controlState = H323controlState(callHandle);
	hsCall = H323hsCall(callHandle);

	if(controlState != UH323_sControlConnected)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s controlState = %s. Queueing Event.",
			fn,GetUh323ControlState(controlState)));
		SCCQueueEvent(evtPtr);
		// evtPtr should not be free in this case
		if (H323controlState(callHandle) == UH323_sControlIdle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4, 
				("%s Control state Idle %p. Sending Facility\n",
				fn,hsCall));
			if (cmCallConnectControl(H323hsCall(callHandle)) < 0)
			{
				NETERROR(MH323, ("%s cmCallConnectControl failed %p\n",
				fn,H323hsCall(callHandle)));
			}
		}
		CacheReleaseLocks(callCache);
		return 0;
	}

	// Set the doOLC for this leg
	H323doOlc(callHandle) = 1;

	status = pH323Data->modeStatus;
	if (status == cmReqModeRequest)
	{
		if (pH323Data->nodeId <= 0)
		{
			if (!strcmp(pH323Data->modeName, "t38fax"))
			{
				cmReqModeEntry entries[] = {
				{"t38fax", -1},
				};
				cmReqModeEntry *desc1[] = {&entries[0],0};
				cmReqModeEntry **modes[] = {desc1, 0};
				pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);

				if(pH323Data->nodeId <=0)
				{
					NETERROR(MSCC,("%s cmRequestModeBuild failed for %s\n",
						fn, pH323Data->modeName));
					goto _return;
				}
            }
			else if (pH323Data->modeName[0] = 'g') /* G Series codecs - voice */
			{
				if (!strcmp(pH323Data->modeName, "g711Ulaw64k"))
				{
					cmReqModeEntry entries[] = {
					{ "g711Ulaw64k", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName, "g711Alaw64k"))
				{
					cmReqModeEntry entries[] = {
					{ "g711Alaw64k", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName,"g7231"))
				{
					cmReqModeEntry entries[] = {
					{ "g7231", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName,"g728"))
				{
					cmReqModeEntry entries[] = {
					{ "g728", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName, "g729AnnexA"))
				{
					cmReqModeEntry entries[] = {
					{ "g729AnnexA", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName, "g729wAnnexB"))
				{
					cmReqModeEntry entries[] = {
					{ "g729wAnnexB", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
				else if (!strcmp(pH323Data->modeName, "g729AnnexAwAnnexB"))
				{
					cmReqModeEntry entries[] = {
					{ "g729AnnexAwAnnexB", -1},
					};
					cmReqModeEntry *desc1[] = {&entries[0],0};
					cmReqModeEntry **modes[] = {desc1, 0};
					pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
				}
#if 0
				cmReqModeEntry *desc1[] = {&entries[0],0};
				cmReqModeEntry **modes[] = {desc1, 0};
				pH323Data->nodeId = cmRequestModeBuild(UH323Globals()->hApp,modes);
#endif

				if(pH323Data->nodeId <=0)
				{
					NETERROR(MSCC,("%s cmRequestModeBuild failed for %s\n",
						fn, pH323Data->modeName));
					goto _return;
				}

			}
		}
		else if (nh323Instances > 1)
		{
			cmReqModeEntry		entry;
			cmReqModeEntry		*ep[1];
			cmReqModeEntry		**epp = NULL;
			void				*ptrs[10];
			cmReqModeEntry		***modes = NULL;
			HPVT peerhVal = 	cmGetValTree(UH323Globals()->peerhApp);
			HPVT hVal = cmGetValTree(UH323Globals()->hApp);
			int		tmpNodeId = -1;

			// Multi-instancing, we will have to do another
			// transformation between instances
			cmMeiEnter(UH323Globals()->hApp);
			cmMeiEnter(UH323Globals()->peerhApp);

			tmpNodeId = pvtAddRootByPath(hVal, 
							cmGetSynTreeByRootName(UH323Globals()->hApp, "h245"),
							"request.requestModes", 0, NULL);

			if(tmpNodeId<0)
			{
				NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
				cmMeiExit(UH323Globals()->peerhApp);
				cmMeiExit(UH323Globals()->hApp);
				goto _return;
			}
			else if(pvtSetTree(hVal, tmpNodeId, peerhVal, pH323Data->nodeId) <0)
			{
				pvtDelete(hVal,tmpNodeId);
				NETERROR(MH323,("%s pvtSetTree failed\n",fn));
				cmMeiExit(UH323Globals()->peerhApp);
				cmMeiExit(UH323Globals()->hApp);
				goto _return;
			}
			// Delete the nodeId in the peerhApp
			pvtDelete(peerhVal, pH323Data->nodeId);
			cmMeiExit(UH323Globals()->peerhApp);
			cmMeiExit(UH323Globals()->hApp);
			pH323Data->nodeId = tmpNodeId;
		}

		if (!strcmp(pH323Data->modeName, "t38fax"))
		{
			callHandle->flags |= FL_CALL_FAX;
		}

		if (cmCallRequestMode(hsCall, pH323Data->nodeId) < 0)
		{
			NETERROR(MSCC,("%s cmCallRequestMode error for %s\n",
				fn, pH323Data->modeName));
		}
		
		pH323Data->nodeId = -1;

	}
	else if (status == cmReqModeAccept)
	{
		if (cmCallRequestModeAck(hsCall, "willTransmitMostPreferredMode") < 0)
		{
			NETERROR(MSCC,("%s cmCallRequestModeAck error\n", fn));
		}
	}
	else
	{
		if (cmCallRequestModeReject(hsCall, "requestDenied") < 0)
		{
			NETERROR(MSCC,("%s cmCallRequestModeReject error\n", fn));
		}
	}

_return:
	CacheReleaseLocks(callCache);
	H323FreeEvent(evtPtr);
	return 0;
}

int SCCInitiateProgress(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCInitiateProgress() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	int					fs = 0;
	HCALL				hsCall;
	int					msg;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	pH323Data = (H323EventData *) evtPtr->data;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Nothing can be done if we cannot find the callHandle
		CacheReleaseLocks(callCache);
		NETERROR(MSCC,("%s Unable to locate my CallHandle\n",fn));
		goto _return;
	}
	
	if ((callHandle->fastStart == 1) &&
				(callHandle->fastStartStatus != 1))
	{
		if(pH323Data->nlocalset && pH323Data->localSet[0].rtpaddr)
		{
			fs = uh323SetFastStartParam(callHandle,
				pH323Data->localSet,pH323Data->nlocalset);

			if(fs > 0)
			{
				H323doOlc(callHandle) = 0;
				callHandle->fastStartStatus = 1;
			}
			else if (fs < 0)
			{
				NETERROR(MH323, ("%s Skipping, FST failed\n", fn));
				CacheReleaseLocks(callCache);
				goto _return;
			}
		}
	}

	hsCall = H323hsCall(callHandle);
	if((msg = cmCallProgressCreate(hsCall,NULL,pH323Data->cause,pH323Data->pi_IE,pH323Data->progress,pH323Data->notify,NULL)) <0)
	{
		NETERROR(MSCC,("%s cmCallProgressCreate failed %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
	}
	else {
		cmCallProgress(hsCall,msg);
		callHandle->lastEvent = SCC_evt323ProgTx;
		timedef_cur(&callHandle->callRingbackTime);
	}
	CacheReleaseLocks(callCache);

_return:
	H323FreeEvent(evtPtr);
	return 0;
}

//	Handle Incoming Progress 
//  Extract Parameters from H323  Progress to send to Leg1 source
int SCCNetworkProgress(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkProgress() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data;
	char				callIDStr[CALL_ID_LEN];
	SCC_EventBlock 		*pNextEvt;
	cmTransportAddress 	cmTransAddr;
	int					addrlen;
	unsigned long		rra;
	unsigned short		rrp;
	int					retval = -1;
	char				ipstr[24];
	int					sid = 0;
	int 				dstInfoNodeId = -1;
	h221VendorInfo 		h221ns;
	BOOL 				isstring;
	int 				vendor;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));
	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache,evtPtr->callID)))
	{
		// Log Error and try to continue
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		CacheReleaseLocks(callCache);
		goto _return;
	}
	
	pNextEvt = (SCC_EventBlock *) malloc(sizeof(SCC_EventBlock));
	memset(pNextEvt,0,sizeof(SCC_EventBlock));

	pNextEvt->data = pH323Data = evtPtr->data;
	pNextEvt->event = SCC_eBridgeProgress;
	evtPtr->data = NULL;

	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->callID,evtPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,callHandle->confID,CONF_ID_LEN);

	if(routeH245==0)
	{
		pH323Data->h245ip = H323h245ip(callHandle);
		pH323Data->h245port = H323h245port(callHandle);
	}

    if(H323outChan(callHandle)[sid].ip)
    {
		if (callHandle->fastStart)
		{
			callHandle->fastStartStatus = 1;
		}

		updateNonStandardCodecType(callHandle, sid, 1);
        pH323Data->localSet = (RTPSet *)malloc(sizeof(RTPSet));
        memset(pH323Data->localSet,0,sizeof(RTPSet));
        callHandle->lastMediaIp = pH323Data->localSet[0].rtpaddr = H323outChan(callHandle)[sid].ip;
        callHandle->lastMediaPort = pH323Data->localSet[0].rtpport = H323outChan(callHandle)[sid].port;
        pH323Data->localSet[0].codecType = H323outChan(callHandle)[sid].codec;
        pH323Data->localSet[0].param = H323outChan(callHandle)[sid].param;
        pH323Data->nlocalset = 1;
		NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s ip=%s/%d codec = %d/%d\n",
					fn,FormatIpAddress(pH323Data->localSet[0].rtpaddr, ipstr),
					pH323Data->localSet[0].rtpport,
					pH323Data->localSet[0].codecType,
					pH323Data->localSet[0].param));
    }

	if ((callHandle->fastStartStatus) && (callHandle->ecaps1 & ECAPS1_PIONFASTSTART))
	{
		pH323Data->msgFlags |= MSGFLAG_PI;
	}

	timedef_cur(&callHandle->callRingbackTime);

    if (cmCallGetParam(H323hsCall(callHandle), 
                           cmParamFullDestinationInfo, 0, 
                           &dstInfoNodeId, NULL) < 0)
    {
		NETDEBUG(MH323,NETLOG_DEBUG3,("%s No src info nodeid found\n", fn));
		dstInfoNodeId = -1;
    }

	if (dstInfoNodeId >= 0)
	{	
		HPVT 	hVal = cmGetValTree(UH323Globals()->hApp);

    	cmMeiEnter(UH323Globals()->hApp);

		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.t35CountryCode", NULL,
				(INT32 *)&h221ns.t35CountryCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s t35 country code not present\n",fn));
				h221ns.t35CountryCode = -1;
		}

		if (pvtGetByPath(hVal, dstInfoNodeId,
			"vendor.vendor.manufacturerCode", NULL,
				(INT32 *)&h221ns.manufacturerCode, &isstring) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
				("%s mfr code not present\n",fn));
			h221ns.manufacturerCode = -1;
		}

		vendor = GetVendorFromH221Info(&h221ns);
		if (vendor > 0)
		{
			callHandle->vendor = vendor;
		}

    	cmMeiExit(UH323Globals()->hApp);
	}

	CacheReleaseLocks(callCache);

	openH245RemotePinhole(fn, callHandle, "PROGRESS", &rra, &rrp);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,("%s bridgeH323EventProcessor failed\n",fn));
	}
_return:
	H323FreeEvent(evtPtr);
	return 0;
}


int 
SCCNetworkGenericMsg(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "SCCNetworkGenericMsg() : ";
	CallHandle 			*callHandle;
	H323EventData		*pH323Data,*inEvtH323Data;
	char				callIDStr[CALL_ID_STRLEN] = {0};
	SCC_EventBlock 		*pNextEvt;
	int					rv = -1;

	NETDEBUG(MSCC,NETLOG_DEBUG3,("%s Entering:\n",fn));

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	if (!(callHandle = CacheGet(callCache, evtPtr->callID)) )
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn, (char*) CallID2String(evtPtr->callID,callIDStr)));
		H323FreeEvent(evtPtr);
		goto _return;
	}

	pNextEvt = evtPtr;
	pH323Data = evtPtr->data;

	pNextEvt->event = SCC_eBridgeGenericMsg;
	memcpy(pH323Data->callID,evtPtr->callID,CALL_ID_LEN);

	if(bridgeH323EventProcessor(pNextEvt)!=0)
	{
		NETERROR(MH323,
			("%s bridgeH323EventProcessor failed!!\n",fn));
	}

	rv = 0;

_return:
	CacheReleaseLocks(callCache);
	return rv;
}

int
SCCInitiateGenericMsg(SCC_EventBlock *evtPtr)
{
	static char 	fn[] = "SCCInitiateGenericMsg():";
	CallHandle 		*callHandle;
	char 			callID[CALL_ID_LEN] = {0};
	HCALL           hsCall;
	H323EventData	*pH323Data;
	char			callIDStr[CALL_ID_STRLEN];
	int				controlState;
	cmUserInputSignalStruct uis;

	DEBUG(MH323, NETLOG_DEBUG1, (" Entering %s\n",fn));

	pH323Data = (H323EventData *)evtPtr->data;
	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	callHandle = CacheGet(callCache,evtPtr->callID);
	if(callHandle == NULL)
	{
		NETERROR(MSCC,("%s Unable to locate CallHandle %s\n",
			fn,CallID2String(evtPtr->callID,callIDStr)));
		goto _return;
	} 
	controlState = H323controlState(callHandle);

	hsCall = H323hsCall(callHandle);

	if(controlState != UH323_sControlConnected)
	{
		NETDEBUG(MSCC,NETLOG_DEBUG4,
			("%s controlState = %s. Queueing Event.",
			fn,GetUh323ControlState(controlState)));
		SCCQueueEvent(evtPtr);
		// evtPtr should not be free in this case
		if (H323controlState(callHandle) == UH323_sControlIdle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4, 
				("%s Control state Idle %p. Sending Facility\n",
				fn,hsCall));
			if (cmCallConnectControl(H323hsCall(callHandle)) < 0)
			{
				NETERROR(MH323, ("%s cmCallConnectControl failed %p\n",
				fn,H323hsCall(callHandle)));
			}
		}
		CacheReleaseLocks(callCache);
		return 0;
	}

	hsCall = H323hsCall(callHandle);

	if ((pH323Data->nodeId > 0) && (nh323Instances > 1))
	{
		HPVT peerhVal = 	cmGetValTree(UH323Globals()->peerhApp);
		HPVT hVal = cmGetValTree(UH323Globals()->hApp);
		int		tmpNodeId = -1;

		// Multi-instancing, we will have to do another
		// transformation between instances
		cmMeiEnter(UH323Globals()->hApp);
		cmMeiEnter(UH323Globals()->peerhApp);

		tmpNodeId = pvtAddRootByPath(hVal, 
						cmGetSynTreeByRootName(UH323Globals()->hApp, "h245"),
						"indication.userInput", 0, NULL);
		if(tmpNodeId<0)
		{
			NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
			cmMeiExit(UH323Globals()->peerhApp);
			cmMeiExit(UH323Globals()->hApp);
			goto _return;
		}
		else if(pvtSetTree(hVal, tmpNodeId, peerhVal, pH323Data->nodeId) <0)
		{
			pvtDelete(hVal,tmpNodeId);
			NETERROR(MH323,("%s pvtSetTree failed\n",fn));
			cmMeiExit(UH323Globals()->peerhApp);
			cmMeiExit(UH323Globals()->hApp);
			goto _return;
		}
		// Delete the nodeId in the peerhApp
		pvtDelete(peerhVal, pH323Data->nodeId);
		cmMeiExit(UH323Globals()->peerhApp);
		cmMeiExit(UH323Globals()->hApp);
		pH323Data->nodeId = tmpNodeId;
	}
	else if (pH323Data->nodeId <= 0)
	{
		if (pH323Data->dtmf) {
			memset (&uis, 0, sizeof (uis));
			uis.signalType = pH323Data->dtmf->sig;
			uis.duration = pH323Data->dtmf->duration;

			pH323Data->nodeId = 
				cmUserInputSignalBuild (UH323Globals()->hApp, &uis);
		}
		else
		{
			// Cannot create the nodeId ? - error
			NETERROR(MSCC,("%s nodeId missing for Generic message %s\n",
				fn, pH323Data->modeName));

			goto _return;
		}
	}
	
	if (cmCallSendUserInput(hsCall, pH323Data->nodeId) < 0)
	{
		NETERROR(MSCC,("%s cmCallSendUserInput error for %s\n",
			fn, pH323Data->modeName));
	}
	
	pH323Data->nodeId = -1;

_return:
	CacheReleaseLocks(callCache);
	H323FreeEvent(evtPtr);
	return 0;
}

//	Handle IncomingConnect 
/* 	delete the tree for srcNodeId 
* 	srcNodeId is of type AlertingMessage
*/
void setAlertingQ931(HCALL hsCall,int srcNodeId)
{
	static char 	fn[] = "setAlertingQ931";
	HPVT 			hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 			peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	int 			tmpNodeId = -1,nodeId = -1,alertingNodeId = -1;

	cmMeiEnter(UH323Globals()->hApp);
    cmMeiEnter(UH323Globals()->peerhApp);

	if((nodeId = cmGetProperty((HPROTOCOL)hsCall)) >0)
	{
		if ((alertingNodeId = pvtGetNodeIdByPath(hVal,
			nodeId,
			"alerting.message.alerting")) > 0)
		{
			if(srcNodeId)
			{
				if ((tmpNodeId = pvtGetNodeIdByPath(peerhVal,
					srcNodeId,
					"message.alerting.progressIndicator")) > 0)
				{
					if(pvtAddTree(hVal,alertingNodeId,peerhVal,tmpNodeId) < 0)
					{
						NETERROR(MH323, ("%s could not add ProgressIndicator\n",fn));
					}
				}
				else {
						NETDEBUG(MH323,NETLOG_DEBUG4,
							("%s could not get progress Indicator from q931NodeId = %d\n",
							fn,srcNodeId));
				}


				if ((tmpNodeId = pvtGetNodeIdByPath(peerhVal,
					srcNodeId,
					"message.alerting.signal")) > 0)
				{
					if(pvtAddTree(hVal,alertingNodeId,peerhVal,tmpNodeId) < 0)
					{
						NETERROR(MH323, ("%s could not set add q931NodeId\n",fn));
					}
				}
				else {
						NETDEBUG(MH323,NETLOG_DEBUG4,
							("%s could not get signal from q931NodeId\n",fn));
				}

				/* delete the srcNodeId */
				freeNodeTree(UH323Globals()->peerhApp, srcNodeId, 0);
        	}
			else 
			{
				if(pvtAddTree(hVal,alertingNodeId,peerhVal,PINodeId) < 0)
				{
					NETERROR(MH323, ("%s could not add ProgressIndicator\n",fn));
				}
			}
		}
	}

	cmMeiExit(UH323Globals()->hApp);
	cmMeiExit(UH323Globals()->peerhApp);
}


/* works without hasroot */
int
freeNodeTree(HAPP hApp, int nodeId, int hasroot)
{
	char fn[] = "freeNodeTree():";
	HPVT hVal = cmGetValTree(hApp);
	int			rootId;

	if(nodeId <= 0)
	{
		NETDEBUG(MH323,NETLOG_DEBUG4, 
			("%s nodeid not positive %d\n", fn, nodeId));
		return 0;
	}

	cmMeiEnter(hApp);

	rootId = pvtGetRoot(hVal,nodeId);

	if(rootId > 0)
	{
		NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Deleting Root %d\n", fn, rootId));
		pvtDelete(hVal,rootId);
	}
	else
	{
		pvtDelete(hVal,nodeId);
		NETDEBUG(MH323,NETLOG_DEBUG4, ("%s Deleting Node %d\n", fn, nodeId));
	}

	cmMeiExit(hApp);

	return 0;
}

int updateNonStandardCodecType( CallHandle *callHandle, int sid, int isOut )
{
	ChanInfo *ch;
	if (isOut)
	{
		ch = &(H323outChan(callHandle)[sid]);
	}
	else
	{
		ch = &(H323inChan(callHandle)[sid]);
	}
	if ( (ch->dataType == cmCapAudio) && (ch->codec ==  CodecNonStandard) )
	{
		HPVT 	hVal = cmGetValTree(UH323Globals()->hApp);
		int nscodec;
	    nscodec = findNonStandardCodec( hVal, ch->dataTypeHandle );
		if ( nscodec > 0 )
		{
			ch->codec = nscodec;
		}
	}
	return 0;
}

// Save egress H323 ID in conf handle
void saveEgressH323Id(char *confID, char *egressH323Id)
{
	ConfHandle *confHandle = NULL;

	CacheGetLocks(confCache, LOCK_WRITE, LOCK_BLOCK);
	if (confHandle = CacheGet(confCache, confID))
	{
		nx_strlcpy(confHandle->egressH323Id, egressH323Id, H323ID_LEN);
	}
   	CacheReleaseLocks(confCache);
}
