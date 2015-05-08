#include "gis.h"
#include "uh323.h"
#include "uh323cb.h"
#include "arq.h"
#include "callconf.h"
#include "serverp.h"
#include "callsm.h"
#include "uh323proto.h"
#include "firewallcontrol.h"
#include "uh323inc.h"
#include "callstats.h"
#include "h323realm.h"
#include "lsconfig.h"
#include "nxosd.h"
#include <malloc.h>

#include "gk.h"
#include "callid.h"
#include "ipstring.h"
#include "log.h"

/* PROTOTYPES OF CALL CALLBACKS */
#define _ROUTE_H245_

      

int     CALLCONV cmEvRegEvent(
     IN      HAPP                hApp,
     IN      cmRegState          regState,
     IN      cmRegEvent          regEvent,
     IN      int                 regEventHandle);

int     CALLCONV cmEvNewCall(
	IN      HAPP            hApp,
	IN      HCALL           hsCall,
	IN      LPHAPPCALL      lphaCall);

SCMEVENT cmEvent={  cmEvNewCall , NULL /* cmEvRegEvent */};

int CALLCONV cmEvCallStateChanged(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      UINT32              state,
	IN      UINT32              stateMode);

int CALLCONV cmEvCallNewRate(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      UINT32              rate);

int CALLCONV cmEvCallInfo(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      char*               display,
	IN      char*               userUser,
	IN      int                 userUserSize);

int CALLCONV cmCallNonStandardParam(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      char*               data,
	IN      int                 dataSize);

int CALLCONV cmEvCallFacility(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      int                 handle,
	OUT IN	BOOL		 *proceed);

int CALLCONV cmEvCallFastStartSetup(
	IN	HAPPCALL haCall,
	IN	HCALL hsCall,
	OUT IN	cmFastStartMessage *fsMessage);

int CALLCONV cmEvCallUserInfo(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle);

int CALLCONV cmEvCallStatus(
	IN      HAPPCALL haCall,
	IN      HCALL hsCall,
	OUT IN  cmCallStatusMessage *callStatusMsg);

int CALLCONV cmEvCallProgress(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 handle);


SCMCALLEVENT cmCallEvent = 
{
	cmEvCallStateChanged, 
	NULL, //cmEvCallNewRate, 
	NULL,	//cmEvCallInfo,  
	NULL, //cmCallNonStandardParam, 
	cmEvCallFacility, 
	cmEvCallFastStartSetup, 
	NULL, //cmEvCallStatus 
    cmEvCallUserInfo,
    NULL, // cmEvCallH450SupplServ;
    NULL, // cmEvCallIncompleteAddress;
    NULL, // cmEvCallAdditionalAddress;
    NULL, // cmEvCallTunnNewMessage;
    NULL, // cmEvCallFastStart;
    cmEvCallProgress,
    NULL, // cmEvCallNotify;
    NULL, // cmEvCallNewAnnexLMessage;
    NULL, // cmEvCallNewAnnexMMessage;
    NULL, // cmEvCallRecvMessage;
    NULL, // cmEvCallSendMessage;
};

#ifdef _ROUTE_H245_
int CALLCONV cmEvCallCapabilities(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmCapStruct*        capabilities[]);

int CALLCONV cmEvCallCapabilitiesExt(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmCapStruct***      capabilities[]);

int CALLCONV cmEvCallNewChannel(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      HCHAN               hsChan,
		OUT     LPHAPPCHAN          lphaChan);

int CALLCONV cmEvCallCapabilitiesResponse(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              status);

int CALLCONV cmEvCallMasterSlaveStatus(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              status);

int CALLCONV cmEvCallRoundTripDelay(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      INT32               delay);

int CALLCONV cmEvCallUserInput(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      INT32               userInputId);

int CALLCONV cmEvCallRequestMode(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmReqModeStatus     status,
		IN      INT32               nodeId);

int CALLCONV cmEvCallMiscStatus(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmMiscStatus        status);

int CALLCONV cmEvCallControlStateChanged(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              state,
		IN      UINT32              stateMode);

int CALLCONV cmEvCallMasterSlave(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              terminalType,
    		IN      UINT32              statusDeterminationNumber);

int CALLCONV cmEvCallMasterSlaveStatus(
		IN 		HAPPCALL 			haCall,
		IN 		HCALL 				hsCall,
		IN 		UINT32 				status);

SCMCONTROLEVENT cmControlEvent = 
{
	NULL,// cmEvCallCapabilities, 
	cmEvCallCapabilitiesExt, 
	cmEvCallNewChannel, 
	cmEvCallCapabilitiesResponse,
	cmEvCallMasterSlaveStatus,
	0,
	cmEvCallUserInput, 
	cmEvCallRequestMode,
	cmEvCallMiscStatus,
	cmEvCallControlStateChanged, 
	cmEvCallMasterSlave	
};

int CALLCONV cmEvChannelStateChanged(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              state,
		IN      UINT32              stateMode);

int CALLCONV cmEvChannelNewRate(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              rate);

int CALLCONV cmEvChannelMaxSkew(
		IN      HAPPCHAN            haChan1,
		IN      HCHAN               hsChan1,
		IN      HAPPCHAN            haChan2,
		IN      HCHAN               hsChan2,
		IN      UINT32              skew);

int CALLCONV cmEvChannelSetAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              ip,
		IN      UINT16              port);

int CALLCONV cmEvChannelSetRTCPAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              ip,
		IN      UINT16              port);

int CALLCONV cmEvChannelParameters(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      char*               channelName,
		IN      HAPPCHAN            haChanSameSession,
		IN      HCHAN               hsChanSameSession,
		IN      HAPPCHAN            haChanAssociated,
		IN      HCHAN               hsChanAssociated,
		IN      UINT32              rate);

int CALLCONV cmEvChannelRTPDynamicPayloadType(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      INT8                dynamicPayloadType);

int CALLCONV cmEvChannelVideoFastUpdatePicture(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan);

int CALLCONV cmEvChannelVideoFastUpdateGOB(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 firstGOB,
		IN      int                 numberOfGOBs);

int CALLCONV cmEvChannelVideoFastUpdateMB(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 firstGOB,
		IN      int                 firstMB,
		IN      int                 numberOfMBs);

int CALLCONV cmEvChannelHandle(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 dataTypeHandle,
		IN      cmCapDataType       dataType);


int CALLCONV cmEvChannelGetRTCPAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32*             ip,
		IN      UINT16*             port);

int CALLCONV cmEvChannelRequestCloseStatus(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      cmRequestCloseStatus  status);

int CALLCONV cmEvChannelTSTO(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      INT8                  isCommand,
		IN      INT8                  tradeoffValue);

int CALLCONV cmEvChannelMediaLoopStatus(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      cmMediaLoopStatus     status);


int CALLCONV cmEvChannelReplace(
		IN 	HAPPCHAN	      haChan,
		IN 	HCHAN	 	      hsChan,
		IN 	HAPPCHAN 	      haReplacedChannel,
		IN 	HCHAN	 	      hsReplacedChannel);

int CALLCONV cmEvChannelFlowControlToZero(
		IN 	HAPPCHAN	      haChan,
		IN      HCHAN                 hsChan);

SCMCHANEVENT cmChannelEvent = 
{
	cmEvChannelStateChanged, 
	NULL, //cmEvChannelNewRate, 
	NULL, //cmEvChannelMaxSkew, 
	cmEvChannelSetAddress,
    cmEvChannelSetRTCPAddress, 
	cmEvChannelParameters, 
	cmEvChannelRTPDynamicPayloadType,  // Ashish
	NULL, //cmEvChannelVideoFastUpdatePicture,
    NULL, //cmEvChannelVideoFastUpdateGOB, 
	NULL, //cmEvChannelVideoFastUpdateMB, 
	cmEvChannelHandle, 
	cmEvChannelGetRTCPAddress, 
	cmEvChannelRequestCloseStatus,
	NULL, //cmEvChannelTSTO, 
	NULL, //cmEvChannelMediaLoopStatus, 
	cmEvChannelReplace, 
	NULL //cmEvChannelFlowControlToZero
};

#endif
int     CALLCONV cmEvNewCall(
	IN      HAPP            hApp,
	IN      HCALL           hsCall,
	OUT     LPHAPPCALL      lphaCall)
{
	static char fn[] = "cmEvNewCall";
	INT32 localIP=0;
	UH323CallAppHandle *appHandle;

	// Allocate Handle
	appHandle = uh323CallAllocAppHandle();

#if 0
	// turn off h245 if we are not doing h245
    if (cmCallSetParam(hsCall, cmParamEstablishH245,
                                 0,
                                 routeH245,
                                 NULL) < 0)
    {
		NETERROR(MH323,
			("%s Could not set h245 off \n", fn));
        goto _error;
    }
#endif

	if (cmCallSetParam(hsCall, cmParamIsMultiplexed,
		0, FALSE, NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set multiplexed to off \n", fn));
	}

	if (cmCallSetParam(hsCall, cmParamShutdownEmptyConnection,
		0, TRUE, NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not set shutdown empty to on \n", fn));
	}

	if (cmCallGetParam(hsCall, cmParamLocalIp,
		0, &localIP, NULL) < 0)
	{
		NETERROR(MH323,
			("%s Could not get localIp\n", fn));
	}

	appHandle->localIP = ntohl(localIP);
	*lphaCall = (HAPPCALL)appHandle;

	DEBUG(MH323, NETLOG_DEBUG1, ( "cmEvNewCall:: %p\n", hsCall));

	UH323Globals()->nCalls ++;

	return 0;

_error:
	uh323CallFreeAppHandle(appHandle);
	cmCallDrop(hsCall);
	return -1;
}

/* Try to obtain CallId from hsCall - if it fails or if it is 0
*  then use a generated value. Same thing applies for confid
*/
int
GisInitCallHandle(CallHandle *callHandle, HCALL hsCall)
{
  	static char fn[] = "GisInitCallHandle(): ";
  	int tlen;
  	int rc = 0;
	char callID[CALL_ID_LEN] = {0};
	char confID[CONF_ID_LEN] = {0};
	char	callIdStr[CALL_ID_LEN+1] ;

  	memset(callHandle,0,sizeof(CallHandle));
	H323hsCall(callHandle) = hsCall;
    tlen = CONF_ID_LEN;
    if (cmCallGetParam(hsCall,cmParamCID,0,&tlen,(char *)callHandle->confID)<0) 
    {
		NETDEBUG(MH323, NETLOG_DEBUG2,
		   ("%s Could not get Confid\n", fn));
		generateCallId((char *) callHandle->confID);	
    }

	if(!memcmp(callHandle->confID,confID,CONF_ID_LEN))
	{
		generateCallId((char *) callHandle->confID);	
		NETDEBUG(MH323,NETLOG_DEBUG2,("%s : confId 0 setting it to = %s \n",
			fn, (char*) CallID2String(callHandle->confID,callIdStr)));
	}
    tlen = CALL_ID_LEN;
    if ((rc = cmCallGetParam(hsCall,
                             cmParamCallID, 0, &tlen,
                             (char *)callHandle->callID)) < 0)
   	{
		NETDEBUG(MH323,NETLOG_DEBUG2,
		   ("%s Could not get call id\n", fn));
		generateCallId((char *) callHandle->callID);	
   	} 
	if(!memcmp(callHandle->callID,callID,CALL_ID_LEN) )
	{
		generateCallId((char *) callHandle->callID);	
		NETDEBUG(MH323,NETLOG_DEBUG2,("%s : callId 0 setting it to = %s \n",
			fn, (char*) CallID2String(callHandle->callID,callIdStr)));
	}
	callHandle->state = SCC_sIdle;
 	H323controlState(callHandle) = UH323_sControlIdle;
	callHandle->handleType= SCC_eH323CallHandle;

	/* Extract the CRV also */
	if (cmCallGetParam(hsCall,
		cmParamRASCRV,
		0,
		&H323outCRV(callHandle),
		NULL) < 0)
	{
		return rc;
	}

	return 0;
}

int
CallDoH245(struct Timer *t)
{
	char fn[] = "CallDoH245():";
	char *callid = (char *)t->data;
	CallHandle 	callHandleBlk = {0},*callHandle = NULL;
	HCALL hsCall;
	
	if(CacheFind(callCache,callid,&callHandleBlk,sizeof(CallHandle)) < 0 )
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s No call handle found\n", fn));
		goto _return;
	} 

	// The caller already did a successful ARQ with us
	NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Found Call handle \n",fn));
	callHandle = &callHandleBlk;
	hsCall = H323hsCall(&callHandleBlk);

	if (H323controlState(callHandle) == UH323_sControlIdle)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Control state Idle %p\n",
			fn,hsCall));

		if (cmCallConnectControl(hsCall) < 0)
		{
			NETERROR(MH323, ("%s cmCallConnectControl failed %p\n",fn,hsCall));
		}
	}
	else
	{
		NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Control state non-Idle %p\n",
			fn,hsCall));
	}

_return:
	if (callid) free(callid);
	timerFreeHandle(t);
	return 0;
}

int CALLCONV cmEvCallStateChanged(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      UINT32              state,
	IN      UINT32              stateMode)
{
	static char 			fn[] = "cmEvCallStateChanged() : ";
	UINT32 					ipOrig;
	int						rc;
	char 					ipStr[20];
	BOOL 					origin = FALSE;
	UH323CallAppHandle 		*appHandle = (UH323CallAppHandle *)haCall;
	CallHandle 				callHandleBlk = {0},*callHandle = NULL;
	ConfHandle 				confHandleBlk = {0},*confHandle = NULL;
	SCC_EventBlock			sccEvt;
	SCC_CallEvent			event;
	char 					callIdStr[CALL_ID_STRLEN],confIdStr[CONF_ID_STRLEN];
	char 					callID[CALL_ID_LEN] = {0};
	HPROTCONN				protocon;
	int						callError = SCC_errorNone;
	hrtime_t				start,end;
	int						cause = 0;
	cmReasonType 			reason = -1;
	time_t					now;
	char 					str[256] = {0};

	if (cmCallGetOrigin(hsCall, &origin) < 0)
	{
		 DEBUG(MH323, NETLOG_DEBUG1, ("cmCallGetOrigin:: returned error\n"));
	}

	DEBUG(MH323, NETLOG_DEBUG1, ("cmEvCallStateChanged:: %p origin = %d\n",
								hsCall, origin));
	if(haCall == NULL)
	{
		NETERROR(MH323,
			("%s : NULL haCall state = %d Mode = %d. origin = %d Dropping call %p!!\n",
			fn,state,stateMode,origin,hsCall));
		if(state == cmCallStateIdle)
		{ 
			if (cmCallClose(hsCall) < 0)
			{
				NETERROR(MH323, 
					 ("cmCallClose returned error %p\n",hsCall));
			}
			
			if ((nh323Instances > 1) || (origin == 0))
			{
				UH323Globals()->nCalls--;
			}
		}
		else {
			cmCallDrop(hsCall);
		}
		return 0;
	}

	//If new call and callID not initiatllized yet (if its faststart it is init in
	// cmEvCallFastStartSetup
	if(state == cmCallStateOffering && 
		!memcmp(appHandle->callID,callID,CALL_ID_LEN))
	{
		if( uh323CallInitAppHandle(hsCall, appHandle) <0)
		{
			NETERROR(MH323,("%s : uh323CallInitAppHandle Failed\n",fn));
			cmCallDrop(hsCall);
			return 0;
		}
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if((callHandle = CacheGet(callCache,appHandle->callID)))
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Found Call handle \n",fn));
		if ((state != cmCallStateIdle) && (state != cmCallStateDisconnected))
		{
			// Add one, so we dont get the default as Dialtone always
			H323StackCallState(callHandle) = state+1;
			CallSetEvtFromH323StackState(state, origin, 
				&callHandle->lastEvent);
		}
		memcpy(&callHandleBlk, callHandle, sizeof(CallHandle));
		callHandle = &callHandleBlk;
	}
	else 
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s did not find Call handle %s \n",
		   fn, (char*) CallID2String(appHandle->callID,callIdStr)));
	}

	CacheReleaseLocks(callCache);

	if(CacheFind(confCache,appHandle->confID,&confHandleBlk,sizeof(ConfHandle))>=0)

	{
		// The caller already did a successful ARQ with us
		NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s Found Conf handle \n",fn));
		confHandle = &confHandleBlk;
	} 
	else {
			NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s did not find Conf handle %s \n",
			   fn, (char*) CallID2String(appHandle->confID,callIdStr)));
	}

	switch( state )
	{
	case cmCallStateDialtone:
		if(!callHandle ||!confHandle)
		{
			NETERROR(MH323,("cmEvCallStateChanged: Null "
				"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
				"hsCall = %p CallID %s ConfID %s\n",
				callHandle, confHandle, state,stateMode,hsCall,
			    CallID2String(appHandle->callID,callIdStr),
			    ConfID2String(appHandle->confID,confIdStr)));

			cmCallDrop(hsCall);
			return 0;
		}

		if(callHandle && callHandle->fastStart)
		{
			uh323FastStartSetupInit(callHandle);
		}
		protocon = cmGetTpktChanHandle(hsCall,cmQ931TpktChannel);
		if(cmSetTpktChanApplHandle(protocon,(HAPPCONN)appHandle)<0)
		{
			NETERROR(MH323,("%s : cmSetTpktChanApplHandle Failed %p\n",
				fn,protocon));
		}

		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateDialtone -- \n"));
		return 0;
		break;

	case cmCallStateProceeding:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateProceeding -- \n"));
		// forward proceeding 
		// CHECK - Alerting vs Proceeding
		if(!callHandle ||!confHandle)
		{
			NETERROR(MH323,("cmEvCallStateChanged: Null "
				"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
				"hsCall = %p CallID %s ConfID %s\n",
				callHandle, confHandle, state,stateMode,hsCall,
				CallID2String(appHandle->callID,callIdStr),
				ConfID2String(appHandle->confID,confIdStr)));
			cmCallDrop(hsCall);
			return 0;
		}

		event = SCC_eNetworkProceeding;
		break;

	case cmCallStateRingBack:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateRingBack -- \n"));
		if(!callHandle ||!confHandle)
		{
			NETERROR(MH323,("cmEvCallStateChanged: Null "
				"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
				"hsCall = %p CallID %s ConfID %s\n",
				callHandle, confHandle, state,stateMode,hsCall,
			    CallID2String(appHandle->callID,callIdStr),
			    ConfID2String(appHandle->confID,confIdStr)));
			cmCallDrop(hsCall);
			return 0;
		}

		event = SCC_eNetworkAlerting;
	break;

	case cmCallStateConnected:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateConnected -- \n"));
		NETDEBUG(MH323, NETLOG_DEBUG1, ("mode is %d\n", stateMode));
		if(!callHandle ||!confHandle)
		{
			NETERROR(MH323,("cmEvCallStateChanged: Null "
				"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
				"hsCall = %p CallID %s ConfID %s\n",
				callHandle, confHandle,	state,stateMode,hsCall,
			    CallID2String(appHandle->callID,callIdStr),
			    ConfID2String(appHandle->confID,confIdStr)));
			cmCallDrop(hsCall);
			return 0;
		}
		// if state mode is that the setup has connected, we will process it
		if ( ((callHandle->ecaps1&ECAPS1_FORCEH245) || 
			 (callHandle->vendor == Vendor_eClarent)) && 
			 (stateMode == cmCallStateModeCallSetupConnected))
		{
			int earlyH245 = 0;

			cmCallGetParam(hsCall, cmParamEarlyH245,0,(INT32*)&earlyH245,NULL);
			NETDEBUG(MH323, NETLOG_DEBUG1, ("origin = %d earlyH245 = %d\n", 
						origin,earlyH245));
			NETDEBUG(MH323, NETLOG_DEBUG4, 
				("%s Remote Endpoint = %d. hsCall = %p\n",
				fn,callHandle->vendor,hsCall));

			if ((H323controlState(callHandle) == UH323_sControlIdle))
			{
				NETDEBUG(MH323, NETLOG_DEBUG4, 
					("%s Control state Idle. Sending Facility %p\n",
					fn,hsCall));

				if (cmCallConnectControl(hsCall) < 0)
				{
					NETERROR(MH323, ("%s cmCallConnectControl failed %p\n",fn,hsCall));
				}
			}
			else
			{
				NETDEBUG(MH323, NETLOG_DEBUG4, 
				("%s Not Sending Facility.Remote Endpoint = %d. hsCall = %p\n",
				fn,callHandle->vendor,hsCall));
			}
		}

		if ((stateMode != cmCallStateModeCallSetupConnected) &&
			(stateMode != cmCallStateModeCallConnected))
		{
			// this is not an actual H.323 Connect message
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("Q.931 not connected yet\n"));
			return 0;
		}
		
		// at this point we may be feeding the wrong event, as
		// after sending a connect, we may send another connect
		// if the H.245 gets connected
#if 0
		if(stateMode == cmCallStateModeControlConnected)
		{
			CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
			if((callHandle = CacheGet(callCache,appHandle->callID)))
			{
				H323controlState(callHandle) =  UH323_sControlConnected;
			}
			CacheReleaseLocks(callCache);
		}
#endif

		if(origin == TRUE)
		{
			event = SCC_eNetworkConnect;
		}
		else 
		{
			return 0;
		}
	break;

	case cmCallStateDisconnected:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateDisconnecetd -- \n"));
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			("mode is %d\n", stateMode));

		if(!callHandle || !confHandle)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("cmEvCallStateChanged: Null "
				"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
				"hsCall = %p CallID %s ConfID %s\n",
				callHandle, confHandle,	state,stateMode,hsCall,
			    CallID2String(appHandle->callID,callIdStr),
			    ConfID2String(appHandle->confID,confIdStr)));
			// Should not call this, as this will lead to
			// recursive behavior
			//cmCallDrop(hsCall);
			return 0;
		}

		event = SCC_eNetworkReleaseComp;

		if (cmCallGetParam(hsCall,
			cmParamReleaseCompleteCause,
			0,
			&cause,
			NULL) < 0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4,("Failed to get ReleaseCompleteCause. hsCall = %p\n",hsCall));
			cause = 0;
		}
		else
		{
			// Take this off by 1
			cause ++;
		}

		if (cmCallGetParam(hsCall,
			 cmParamReleaseCompleteReason, 
			 0, 
			 (int *)&reason,
			 NULL) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4, ("%s Could not get reason\n", fn));
			reason = 0;
		}
		else
		{
			// Take this off by 1
			reason ++;
		}

		callError = BillCallDropInferReason(hsCall, &callHandleBlk, stateMode, 
						reason, cause);
		if ( stateMode == cmCallStateModeDisconnectedUnreachable )
		{
        	if (!(appHandle->flags & FL_CALL_RXRELCOMP))
        	{
				// We need to handle the timeout scenario
				if (recoveryOnTimerExpiry)
				{
					cause = recoveryOnTimerExpiry + 1;
				}
				else
				{
					cause = Cause_eRecoveryOnExpiresTimeout +1;
				}
			}
		}

		if ( cause == 0 && reason == 0 && callError == SCC_errorNone && stateMode == cmCallStateModeDisconnectedNormal )
		{
			cause = Cause_eNormalCallClearing + 1;
		}
	break;

	case cmCallStateIdle:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateIdle -- \n"));

		NETDEBUG(MH323, NETLOG_DEBUG1, 
			("Closing Call %p / %p\n", hsCall, appHandle));
					
		GkSendDRQ(callHandle);

		if (cmCallClose(hsCall) < 0)
		{
			NETDEBUG(MH323, NETLOG_DEBUG1, 
			 ("cmCallClose returned error %p\n",hsCall));
		}
		else
		{
			NETDEBUG(MH323, NETLOG_DEBUG1,
			 ("cmCallClose success %p\n",hsCall));
		}

		if ((nh323Instances > 1) || (origin == 0))
		{
			UH323Globals()->nCalls--;
		}

		if(!callHandle)
		{
			NETDEBUG(MSCC,NETLOG_DEBUG1,
				("%s No Call Handle exists for this call %p\n", fn,hsCall));
				if(appHandle)
				   free(appHandle);
			return 0;
		}
			
		sccUh323InitEventBlock(&sccEvt,appHandle);
		sccEvt.evtProcessor = SCC_NetworkCallStateIdle;
		SCC_DelegateEvent(&sccEvt);

		free(appHandle);
		return 0;

	break;

	case cmCallStateOffering:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateOffering -- \n"));

		time(&now);
		UH323UpdateStats(&UH323Globals()->setupLast, &now, 
			&UH323Globals()->nsetups);

		NETDEBUG(MH323, NETLOG_DEBUG1, 
			("mode is %d\n", stateMode));

		ST_setup();
		if (appHandle->fastStartError)
		{
			// An error happened during fast start
			// get rid of the call. The CDR etc has already
			// been taken care of at fast start

			cmCallDrop(hsCall);

			return -1;
		}

		protocon = cmGetTpktChanHandle(hsCall,cmQ931TpktChannel);
		if(cmSetTpktChanApplHandle(protocon,(HAPPCONN)appHandle) <0)
		{
			NETERROR(MH323,
				("%s Could not set  TpktAppHandle %p\n",fn,protocon));
		}

		//if we found a cache entry its already admitted or admit it
		if(!callHandle )
		{
			if(GkAdmitCallFromSetup(hsCall, haCall)>=0)
			{
						// try to admit it 
				NETDEBUG(MSCC, NETLOG_DEBUG4,
				   ("%s GkAdmitCallFromSetup successful\n",fn));
			}
			else {
				// ?? CHECK - Free CallHandle at idle 
				cmCallDrop(hsCall);
				return -1;
			}
		}
		else if(callHandle->state != SCC_sIdle) 
		{
			NETERROR(MH323, ("%s Received setup in call state %s. Src = %s/%lu Dest = %s/%lu hcall = %p\n", 
			fn, SCC_CallStateToStr(callHandle->state,str),
			callHandle->phonode.regid,callHandle->phonode.uport,
			callHandle->rfphonode.regid,callHandle->rfphonode.uport,hsCall));
			return 0;
		}

		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
		callHandle = CacheGet(callCache,appHandle->callID);
		H323StackCallState(callHandle) = state+1;
		CallSetEvtFromH323StackState(state, origin, &callHandle->lastEvent);
		callHandle->leg = SCC_CallLeg1;
		callHandle->networkEventProcessor = SCC_H323Leg1NetworkEventProcessor;
		callHandle->bridgeEventProcessor = SCC_H323Leg1BridgeEventProcessor;

		event = SCC_eNetworkSetup;
		callHandle->state = SCC_sIdle;
		H323appHandle(callHandle) = appHandle;
		CacheReleaseLocks(callCache);

		CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
		if(!(confHandle = CacheGet(confCache,appHandle->confID)))
		{
			CacheReleaseLocks(confCache);
			NETERROR(MH323, ("%s, ConfHandle not found! dropping call\n", fn));
			cmCallDrop(hsCall);
			return 0;
		}
		confHandle->state = SCC_sIdle;
		CacheReleaseLocks(confCache);
	break;
	
	case cmCallStateTransfering:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" -- cmEvCallStateChanged: cmCallStateTransfering -- \n"));
		goto 	_return;
	break;

	default:
		NETDEBUG(MH323, NETLOG_DEBUG1, 
			(" ?? cmEvCallStateChanged: Unknown Call State ??\n\n"));
		goto _return;
	break;
	}

	if(!callHandle || !confHandle)
	{
		NETERROR(MH323,("cmEvCallStateChanged: Null "
			"CallHandle/ConfHandle %p/%p state/mode = %d/%d"
			"hsCall = %p CallID %s ConfID %s\n",
			callHandle, confHandle, state,stateMode,hsCall,
			CallID2String(appHandle->callID,callIdStr),
			ConfID2String(appHandle->confID,confIdStr)));
		return 0;
	}

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = event;
	sccEvt.callDetails.callError = callError;
	sccEvt.callDetails.cause = cause;
	sccEvt.callDetails.h225Reason = reason;
	SCC_DelegateEvent(&sccEvt);

_return:
	return 0;
}

int CALLCONV cmEvCallNewRate(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      UINT32              rate)
{
	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallNewRate -- \n"));
	NETDEBUG(MH323, NETLOG_DEBUG1, ("New Call Rate = %u\n", rate));

	return 0;
}

int CALLCONV cmEvCallInfo(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      char*               display,
	IN      char*               userUser,
	IN      int                 userUserSize)
{
	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallInfo -- \n"));
	return 0;
}

int CALLCONV cmCallNonStandardParam(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      char*               data,
	IN      int                 dataSize)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallNonStandardParam -- \n"));
	return 0;
}

int CALLCONV cmEvCallFacility(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	IN      int                 handle,
	OUT IN	BOOL		 *proceed)
{
	static char fn[] = "cmEvFacility(): ";
	CallHandle callHandleBlk1, callHandleBlk2, *callHandle1;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	ConfHandle confHandleBlk;
	char *callID2;
	HCALL               hsCall2;
	INT32 ip, port;
	HPVT hVal = cmGetValTree(UH323Globals()->hApp);
	int nodeId;
	int h245nodeId;
	BOOL string;
	cmTransportAddress cmTransAddr  = { 0, 0, 0, cmTransportTypeIP };
	int length;
	unsigned int		rra;
	unsigned short		rrp;
	int					retval = -1;
	int noreason = 0;
	BOOL 					origin = FALSE;
	SCC_EventBlock 			sccEvt;
	H323EventData			*pH323Data=NULL;
	char				callIdStr[CALL_ID_STRLEN];

	if (cmCallGetOrigin(hsCall, &origin) < 0)
	{
		 DEBUG(MH323, NETLOG_DEBUG1, ("cmCallGetOrigin:: returned error\n"));
	}

	NETDEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallFacility origin = %d-- \n",
			origin));

	*proceed = FALSE;

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	cmMeiEnter(UH323Globals()->hApp);

	// Unless we are setting proceed to true, do
	// not check !!

	if ((nodeId = pvtGetNodeIdByPath(hVal, handle, 
		"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1,
			("%s:: No reason\n",fn));
		noreason = 1;
	}
	else
	{
#ifdef _check_routeCallToGatekeeper
		if (pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.routeCallToGatekeeper") >= 0 )
		{

			DEBUG(MH323, NETLOG_DEBUG1, ("%s routeCallToGatekeeper\n",fn));
			/* Extract the gatekeeper address from the alternate
			* address
			*/
			if (pvtGetByPath(hVal, nodeId,
				"alternativeAddress.ipAddress.ip", NULL,
				&ip, &string) < 0)
			{
				DEBUG(MH323, NETLOG_DEBUG1,
					("%s GK Ip not present\n",fn));
			}

			if (pvtGetByPath(hVal, nodeId,
				"alternativeAddress.ipAddress.port", NULL,
				&port, &string) < 0)
			{
				DEBUG(MH323, NETLOG_DEBUG1,
					("%s GK port not present\n",fn));
			}

			DEBUG(MH323, NETLOG_DEBUG1,
				("%s IP: 0x%x, port %d\n",fn, ip, port));
		}
#endif
		if (pvtGetNodeIdByPath(hVal, nodeId,
				"reason.callForwarded") >= 0 )
		{
			DEBUG(MH323, NETLOG_DEBUG1, ("%s callForwarded\n",fn));

#ifdef _need_ip_address
			/* Extract the forwarded-to address from the 
			* alternate address or the alternate alias address
			*/
			if (pvtGetByPath(hVal, nodeId,
				"alternativeAddress.ipAddress.ip", NULL,
				&ip, &string) < 0)
			{
				DEBUG(MH323, NETLOG_DEBUG1,
					("Fwd Ip not present\n"));
			}

			if (pvtGetByPath(hVal, nodeId,
				"alternativeAddress.ipAddress.port", NULL,
				&port, &string) < 0)
			{
				DEBUG(MH323, NETLOG_DEBUG1,
					("Fwd port not present\n"));
			}

			DEBUG(MH323, NETLOG_DEBUG1,
				("IP: 0x%x, port %d\n", ip, port));
#endif

			*proceed = TRUE;

		}
#ifdef _other_scenarios
		else if (pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.routeCallToMC") >= 0 )
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("facility reason = routeCallToMC\n"));
		}
		else if (pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.undefinedReason") >= 0 )
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("facility reason = undefinedReason\n"));
		}
		else if (pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.conferenceListChoice") >= 0 )
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("facility reason = conferenceListChoice\n"));
		}
#endif
		else if ((pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.starth245") >= 0) || 
			(pvtGetNodeIdByPath(hVal, nodeId,
                                "reason.startH245") >= 0) )
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("facility reason = starth245\n"));

			if(routeH245) 
			{
				if(appHandle->flags & FL_CALL_H245CTRLCONN)
				{
					// We must discard this facility message
					// and not let the stack process it

					NETDEBUG(MH323, NETLOG_DEBUG1, 
						("%s Control Already setup, facility msg not needed\n", fn));
				}
				else
				{
					DEBUG(MH323, NETLOG_DEBUG1,	("Letting stack Open H.245\n"));

					/* Extract the h.245 address */
					if ( (h245nodeId = pvtGetNodeIdByPath(hVal, nodeId,
						"h245Address.ipAddress.ip"))< 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 Ip not present\n",fn));
					}

					if (pvtGetString(hVal, h245nodeId,
						4, 
						(char *)&ip) < 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 Ip not present\n",fn));
					}

					if (pvtGetByPath(hVal, nodeId,
						"h245Address.ipAddress.port", NULL,
						&port, &string) < 0)
					{
						DEBUG(MH323, NETLOG_DEBUG1,
							("%s H.245 port not present\n",fn));
					}

					callHandle1 = CacheGet(callCache,appHandle->callID);
                    if (!callHandle1)
                    {
                        NETERROR(MH323, ("%s Call handle %s not found\n",
                            fn, CallID2String(appHandle->callID,callIdStr)));
                        cmMeiExit(UH323Globals()->hApp);
                        CacheReleaseLocks(callCache);
                        *proceed = FALSE;
                        return 0;
                    }

					if((callHandle1->vendor != Vendor_eClarent) && 
						(cmCallGetParam(hsCall, 
							origin?cmParamSetupH245Address:cmParamH245Address,0,
							&length, (char *)&cmTransAddr) >= 0) )
					{
						// if our ip is smaller than peer's
						// ip, we should skip any control setup
						if ((cmTransAddr.ip < ip) ||
							((cmTransAddr.ip == ip) &&
								(cmTransAddr.port < port)))
						{
							// skip connect control
							cmTransAddr.ip = 0;
							NETDEBUG(MH323, NETLOG_DEBUG4,
								("Our H.245 Address is lower than peer \n"));
						}
						else
						{
							cmTransAddr.ip = ip;
							cmTransAddr.port = port;
						}
					}
					else
					{
						cmTransAddr.ip = ip;
						cmTransAddr.port = port;
					}

#if 0
					if (!origin && cmTransAddr.ip)
					{
						// set up the h.245 address of peer
						cmCallSetParam(hsCall, cmParamSetupH245Address,
							0, sizeof(cmTransportAddress),(char*)&cmTransAddr);
					}
					else if (origin && cmTransAddr.ip)
					{
						// set up the h.245 address of peer
						cmCallSetParam(hsCall, cmParamH245Address,
							0, sizeof(cmTransportAddress),(char*)&cmTransAddr);
					}

#endif
					// do the following only if call is connected
					DEBUG(MH323, NETLOG_DEBUG1,
						("%s IP: 0x%x, port %d\n",fn, ip, port));

				}

				if (cmTransAddr.ip > 0)
				{
					NETDEBUG(MH323, NETLOG_DEBUG4, ("Processing facility\n"));
					*proceed = TRUE;
				}
				else
				{
					NETDEBUG(MH323, NETLOG_DEBUG4, ("Ignoring facility\n"));
					*proceed = FALSE;
					cmMeiExit(UH323Globals()->hApp);
					CacheReleaseLocks(callCache);
					return 0;
				}
			}			
			else {
					DEBUG(MH323, NETLOG_DEBUG1,
						("%s Not routing h245 - Passing the message to remote\n",fn));
			}
					
		}
		else if (pvtGetNodeIdByPath(hVal, nodeId, 
				"reason.undefinedReason") >= 0 )
		{
			cmFastStartMessage 			fsMessage;
			DEBUG(MH323, NETLOG_DEBUG1,
				("facility reason = undefinedReason\n"));

            if (pvtGetNodeIdByPath(hVal, nodeId, 
				"fastStart") >= 0 )
            {
				INT16 		lcn = -1;
				DEBUG(MH323, NETLOG_DEBUG1,
					("fastStart Found\n"));

				if (decodeFacilityFastStartMsg(hsCall, nodeId, &fsMessage, &lcn) != RVERROR)
				{
    				int index,channelType;

					// Translate cmFastStartMessage to H323RTPSet
					// Check for DataType == NULL 
					// Should this be done only for Open Channel
					// Is memory to be release on a CLC
					int					rxcount = 0, txcount = 0, rxcountpart, txcountpart;
					int					i;
					H323RTPSet			*pRtpSetRx = NULL,*pRtpSetTx = NULL;
					int					nonStdCodecs = 0;
					CallHandle 			*callHandle = NULL;

    				if(fsMessage.partnerChannelsNum > 0)
					{	
						// Put different mline numbers here later for each partner
						for (i = 0; i < fsMessage.partnerChannelsNum; i ++)
						{
							if (fsMessage.partnerChannels[i].type  == cmCapEmpty)
								continue;
							rxcount += fsMessage.partnerChannels[i].receive.altChannelNumber;
							txcount += fsMessage.partnerChannels[i].transmit.altChannelNumber;
						}

						if (rxcount > 0)
						{
							pRtpSetRx = (H323RTPSet *) (CMalloc(callCache))(rxcount*sizeof (H323RTPSet));
							memset(pRtpSetRx,0,rxcount*sizeof(H323RTPSet));
						}

						if (txcount > 0)
						{
							pRtpSetTx = (H323RTPSet *) (CMalloc(callCache))(txcount*sizeof (H323RTPSet));
							memset(pRtpSetTx,0,txcount*sizeof(H323RTPSet));
						}

						rxcount = 0;
						txcount = 0;
						for (i = 0; i < fsMessage.partnerChannelsNum; i ++)
						{
							if (fsMessage.partnerChannels[i].type  == cmCapEmpty)
								continue;
							rxcountpart = fsMessage.partnerChannels[i].receive.altChannelNumber;
							if(uh323ExtractFSAudioRtpSet(pRtpSetRx+rxcount,&rxcountpart,fsMessage.partnerChannels[i].receive.channels) < 0)
							{
								nonStdCodecs = 1;		
							}
							else
							{
								rxcount += rxcountpart;
							}
							txcountpart = fsMessage.partnerChannels[i].transmit.altChannelNumber;
							if(uh323ExtractFSAudioRtpSet(pRtpSetTx+txcount,&txcountpart,fsMessage.partnerChannels[i].transmit.channels) < 0)
							{
								nonStdCodecs = 1;		
							}
							else
							{
								txcount += txcountpart;
							}
						}
						if (txcount == 0 && pRtpSetTx != NULL)
						{
							(CFree(callCache))(pRtpSetTx);
							pRtpSetTx = NULL;
						}
						if (rxcount == 0 && pRtpSetRx != NULL)
						{
							(CFree(callCache))(pRtpSetRx);
							pRtpSetRx = NULL;
						}
						if((callHandle = CacheGet(callCache,appHandle->callID)))
						{
							H323nlocalset(callHandle) = txcount;
							H323localSet(callHandle) = pRtpSetTx;
							H323nremoteset(callHandle) = rxcount;
							H323remoteSet(callHandle) = pRtpSetRx;
							if (nonStdCodecs > 0)
							{
								NETERROR(MH323, 
									("%s Received FS setup with non-std codecs from %s/%lu, cgpn %s, cdpn %s\n", 
									fn, callHandle->phonode.regid, callHandle->phonode.uport, 
									H323callingPartyNumber(callHandle), 
									H323dialledNumber(callHandle)));
							}
						}
					}
        	        // Parse FastStart
	
    	     		/* This is the main loop that goes over the offered channls in the given structure
			         and build from it the sub-tree to be saved in the H245 machine and attached to the
			         FACILITYSETUP message. The order is acccording to the channel type (Audio, Video etc.) */
			        for (channelType = 0; channelType< fsMessage.partnerChannelsNum ;channelType++)
			        {
			            cmAlternativeChannels* aChannel;
			            /* We currently handle only audio and video channels in faststart */
            			/*if ( (fsMessage->partnerChannels[channelType].type < cmCapEmpty) || (fsMessage->partnerChannels[channelType].type > cmCapData) )
			                continue;*/ 

	                    int						sid;

    	                /*sid = cmChannelSessionId(hsChan) - 1;
        	            if (sid <0 || sid >= MAX_LOGICAL_CHAN)
            	        {
                	        NETERROR(MH323, 
                    	        ("%s sessionId out of range = %d. hschan = %x\n",
                        	    fn,sid,hsChan));
	                        return -1;
    	                }*/
						
						sid = 0;	// Audio is 0
						if (fsMessage.partnerChannels[channelType].type  == cmCapEmpty)
						{
                			//cmChannelAnswer(hsChan);  
			                //cmChannelDrop(hsChan);
							
							DEBUG(MH323, NETLOG_DEBUG1,
								("DataType == NULL Sending CLC to SCC, \n"));
							if (lcn>0 && (callHandle->handle.h323CallHandle.lcnNumber == lcn))
										/*|| (lcnChan == lcn))*/
								callHandle->handle.h323CallHandle.lcnNumber = -1;

							sccUh323InitEventBlock(&sccEvt,appHandle);
							sccEvt.event = SCC_eNetworkCLC;
							sccEvt.data = pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
							memset(pH323Data,0,sizeof(H323EventData));
							pH323Data->sid = sid;
							SCC_DelegateEvent(&sccEvt);
						}
						else
						{
							int		codecType,param,flags = 0;
							int dataTypeHandle, dataType;
							int sid;
							BOOL origin = FALSE;
                			cmFastStartChannel *fsChannel;

							DEBUG(MH323, NETLOG_DEBUG1,
								("DataType not NULL Sending OLC to SCC, \n"));

							if (lcn>0)
								callHandle->handle.h323CallHandle.lcnNumber = lcn;
							
							dataType = fsMessage.partnerChannels[0].type;
							sid = 0;
							fsChannel = &(fsMessage.partnerChannels[0].transmit.channels[0]);
							dataTypeHandle = fsChannel->dataTypeHandle;

							if (dataType == cmCapAudio)
							{
								getCodecParam(dataTypeHandle,&codecType,&param,&flags);
							}

							/*NETDEBUG(MH323, NETLOG_DEBUG4,
						   	("%s Channel for leg %d handle = %d codec %d/%d doOlc = %d flag = 0x%x\n",
								fn,callHandle->leg,dataTypeHandle,codecType,param,
								H323doOlc(callHandle),flags));*/

							//GkInitChannelHandle(callHandle, origin, /*hsChannel*/ -1,sid);
							GkInitChannelDataType(callHandle, origin, dataTypeHandle,
								dataType,param,sid,flags);
							GkInitChannelRTCPAddress(callHandle,origin,ntohl(fsChannel->rtcp.ip),fsChannel->rtcp.port,sid);
							GkInitChannelAddress(callHandle, origin,
								ntohl(fsChannel->rtp.ip), fsChannel->rtp.port,sid);

							sccUh323InitEventBlock(&sccEvt,appHandle);
							sccEvt.event = SCC_eNetworkOLC;
							pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
							memset(pH323Data,0,sizeof(H323EventData));
							pH323Data->sid = sid;
							sccEvt.data = pH323Data;
							SCC_DelegateEvent(&sccEvt);
						}
						
						//sccEvt.event = SCC_eNetworkChanConnect;
						//sccEvt.event = SCC_eNetworkCapabilities;

				        aChannel=&fsMessage.partnerChannels[channelType].receive;

			            /* Go over the offered receive channels */
			            for (index=0;index<aChannel->altChannelNumber;index++)
			            {
            			   //fsMessage->partnerChannels[channelType].type, dirReceive, &aChannel->channels[index];
           				}

				        aChannel=&fsMessage.partnerChannels[channelType].transmit;

			            /* Now go over the offered transmit channels */
			            for (index=0;index<aChannel->altChannelNumber;index++)
            			{
            			    //fsMessage->partnerChannels[channelType].type, dirTransmit, &aChannel->channels[index];
			            }
        			}
				}
				else
				{
					DEBUG(MH323, NETLOG_DEBUG1,
						("Error in decoding facility fastStart\n"));
				}

            }
            else    
			{
				DEBUG(MH323, NETLOG_DEBUG1,
					("fastStart not Found\n"));
			}
		}
		else
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("This facility message cannot be handled\n"));
			noreason = 1;
		}
	}

	cmMeiExit(UH323Globals()->hApp);
	CacheReleaseLocks(callCache);

	if (noreason == 1)
	{
		*proceed = TRUE;

		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Letting stack handle Facility message\n", fn)); 

		return 0;
	}

	/* a new h245 ip/port received, if it is from outside the firewall, open a hole */
	if (cmTransAddr.ip != 0)
	{
		CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
		callHandle1 = CacheGet(callCache,appHandle->callID);
        if (!callHandle1)
        {
            NETERROR(MH323, ("%s NULL call handle %s\n",
                fn, CallID2String(appHandle->callID,callIdStr)));
            CacheReleaseLocks(callCache);
            *proceed = FALSE;
            return 0;
        }

		if (callHandle1->state!=SCC_sConnected)
		{
			CacheReleaseLocks(callCache);
			*proceed = FALSE;
			return 0;
		}

		// BUG 3906 - outgoing TCP connection holes may lead to leaks
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("%s Received H.245 address in facility, skipping opening hole for outgoing connection\n", fn));
#if 0

		NETDEBUG(MFCE, NETLOG_DEBUG4, 
			("Opening H.245 pinhole upon incoming FACILITY from %s:%d\n", 
				(char*) ULIPtostring(cmTransAddr.ip), cmTransAddr.port));

		openH245RemotePinhole(fn, callHandle1, "FACILITY", &H323h245ip(callHandle1), &H323h245port(callHandle1));
#endif

		CacheReleaseLocks(callCache);
	}

	if (*proceed == TRUE)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			("%s Letting stack handle Facility message\n", fn)); 
		return 0;
	}

#ifdef  _fwd_facility
// We should not blindly forward facility. #2768. To forward facility, We 
// should dup the nodeid and set the protocol id correctly 
	if(CacheFind(callCache,appHandle->callID,&callHandleBlk1,sizeof(CallHandle))<0 )
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
		return 0;
	} 

	if(CacheFind(confCache,appHandle->confID,&confHandleBlk,sizeof(ConfHandle))>=0)

	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			   ("%s Found Conf handle \n",fn));
	} 
	
	NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s  Received facility message on Leg %d \n",fn,callHandleBlk1.leg));

	if(!memcmp(callHandleBlk1.callID,confHandleBlk.callID[0],CALL_ID_LEN))
	{
			callID2 = confHandleBlk.callID[1];
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received facility on call 0\n",fn));
	}
	else if(!memcmp(callHandleBlk1.callID,confHandleBlk.callID[1],CALL_ID_LEN))
	{
			callID2 = confHandleBlk.callID[0];
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received facility on call 1\n",fn));
	}
	else {
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received facility on unknown leg\n",fn));
			return 0;
	}

	if(CacheFind(callCache,callID2,&callHandleBlk2,sizeof(CallHandle))<0 )
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle for other leg \n",fn));
		return 0;
	} 

	if(callHandleBlk2.handleType == SCC_eH323CallHandle)
	{
		hsCall2 = H323hsCall(&callHandleBlk2);
		if(cmCallFacility(hsCall2,handle) <0)
		{
			NETERROR(MH323, 
			   ("%s cmCallFacility on failed.Unable to pass facility message\n",
			   fn));
		}
	}
	else {
			NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Received Facility - NOT HANDLING YET\n",
			   fn));
	}	

#endif
	
	return 0;
}

int CALLCONV cmEvCallFastStartSetup(
	IN	HAPPCALL haCall,
	IN	HCALL hsCall,
	OUT IN	cmFastStartMessage *fsMessage)
{
	static	char 		fn[] = "cmEvCallFastStartSetup :";
	UH323CallAppHandle 	*appHandle = (UH323CallAppHandle *)haCall;
	CallHandle 			*callHandle;
	int					rxcount = 0, txcount = 0, rxcountpart, txcountpart;
	int					i;
	H323RTPSet			*pRtpSetRx = NULL,*pRtpSetTx = NULL;
	int					nonStdCodecs = 0;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallFastStartSetup -- \n"));

	if( uh323CallInitAppHandle(hsCall, appHandle) <0)
	{
		NETERROR(MH323,("%s : uh323CallInitAppHandle Failed "
			"hsCall %p appHandle %p \n",fn,hsCall,appHandle));

		appHandle->fastStartError = 1;

		return -1;
	}

	if(GkAdmitCallFromSetup(hsCall, haCall)>=0)
	{
				// try to admit it 
		NETDEBUG(MSCC, NETLOG_DEBUG4,
		   ("%s GkAdmitCallFromSetup successful\n",fn));
	}
	else {
		// ?? CHECK - Free CallHandle at idle 
		// Cannot call call drop at this stage, but must flag it
		// so that we can call it at offering
		// cmCallDrop(hsCall);

		appHandle->fastStartError = 1;

		return -1;
	}

    if(fsMessage->partnerChannelsNum > 0)
	{
		// Put different mline numbers here later for each partner

		for (i = 0; i < fsMessage->partnerChannelsNum; i ++)
		{
			rxcount += fsMessage->partnerChannels[i].receive.altChannelNumber;
			txcount += fsMessage->partnerChannels[i].transmit.altChannelNumber;
		}

		if (rxcount > 0)
		{
			pRtpSetRx = (H323RTPSet *) (CMalloc(callCache))(rxcount*sizeof (H323RTPSet));
			memset(pRtpSetRx,0,rxcount*sizeof(H323RTPSet));
		}

		if (txcount > 0)
		{
			pRtpSetTx = (H323RTPSet *) (CMalloc(callCache))(txcount*sizeof (H323RTPSet));
			memset(pRtpSetTx,0,txcount*sizeof(H323RTPSet));
		}

		rxcount = 0;
		txcount = 0;

		for (i = 0; i < fsMessage->partnerChannelsNum; i ++)
		{
			rxcountpart = fsMessage->partnerChannels[i].receive.altChannelNumber;
			if(uh323ExtractFSAudioRtpSet(pRtpSetRx+rxcount,&rxcountpart,fsMessage->partnerChannels[i].receive.channels) < 0)
			{
				nonStdCodecs = 1;		
			}
			else
			{
				rxcount += rxcountpart;
			}

			txcountpart = fsMessage->partnerChannels[i].transmit.altChannelNumber;
			if(uh323ExtractFSAudioRtpSet(pRtpSetTx+txcount,&txcountpart,fsMessage->partnerChannels[i].transmit.channels) < 0)
			{
				nonStdCodecs = 1;		
			}
			else
			{
				txcount += txcountpart;
			}
		}

		if (txcount == 0 && pRtpSetTx != NULL)
		{
			(CFree(callCache))(pRtpSetTx);
			pRtpSetTx = NULL;
		}

		if (rxcount == 0 && pRtpSetRx != NULL)
		{
			(CFree(callCache))(pRtpSetRx);
			pRtpSetRx = NULL;
		}

		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
		if((callHandle = CacheGet(callCache,appHandle->callID)))
		{
			H323nlocalset(callHandle) = txcount;
			H323localSet(callHandle) = pRtpSetTx;
			H323nremoteset(callHandle) = rxcount;
			H323remoteSet(callHandle) = pRtpSetRx;

			if (nonStdCodecs > 0)
			{
				NETERROR(MH323, 
					("%s Received FS setup with non-std codecs from %s/%lu, cgpn %s, cdpn %s\n", 
				fn, callHandle->phonode.regid, callHandle->phonode.uport, 
				H323callingPartyNumber(callHandle), 
				H323dialledNumber(callHandle)));
			}
		}
		CacheReleaseLocks(callCache);
	}

	return 0;
}

int CALLCONV cmEvCallUserInfo(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle)
{
	static char 		fn[] = "cmEvCallUserInfo(): ";
	int 				nodeId = -1, keypadNodeId = -1;
	CallHandle 			*callHandle = NULL;
	UH323CallAppHandle 	*appHandle = (UH323CallAppHandle *)haCall;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	char				keypadFacStr[10];
	char 				callIdStr[CALL_ID_LEN];
	SCC_EventBlock		sccEvt;
	H323EventData		*pH323Data;
	int 				size = 0;
	BOOL				string = 0;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- %s -- \n", fn));

	memset(keypadFacStr,0,sizeof(keypadFacStr));
	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s, No Application Handle %p\n",fn,hsCall));
		return(0);
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if ((callHandle = CacheGet(callCache,appHandle->callID)))
	{
		// do vendor check for avaya
		if (callHandle->vendor != Vendor_eAvaya)
		{
				DEBUG(MH323, NETLOG_DEBUG1,
					("%s Info message recived for vendor != avaya. Ignored\n",fn));
				CacheReleaseLocks(callCache);
				return(0);
		}
	}
	else
	{
		NETDEBUG(MH323, NETLOG_DEBUG2,
		   ("%s Did not find callID. callID=%s \n",
		   fn, (char*) CallID2String(appHandle->callID,callIdStr)));
		CacheReleaseLocks(callCache);
		return(0);
	} 

	cmMeiEnter(UH323Globals()->hApp);
	if (!((nodeId = pvtGetNodeIdByPath(hVal, handle, 
		"message.information")) < 0))
	{
		if ((keypadNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
				"keypadFacility")) >= 0 )
		{
			int 	keypadFacSize = sizeof(keypadFacStr); 
			if (pvtGetByPath(hVal, nodeId,
					"keypadFacility", NULL,
					&size, &string) < 0)
			{
				NETERROR(MH323, 
					("%s Could not find Keypad facility data\n",fn));
			}
			else
			{
				pvtGetString(hVal, keypadNodeId, size, keypadFacStr);
				if (size<keypadFacSize) keypadFacStr[size]='\0';
				DEBUG(MH323, NETLOG_DEBUG1,
					("%s Found Keypad facility : %s, length = %d, dtmf = %c\n",fn, keypadFacStr, size, keypadFacStr[0]));
			}
		}
		else
		{
			DEBUG(MH323, NETLOG_DEBUG1,
				("%s Could not find Keypad facility Node\n",fn));
		}
	}
	else
	{
		NETERROR(MH323, 
			("%s Not Information Message\n",fn));
	}

	cmMeiExit(UH323Globals()->hApp);
	CacheReleaseLocks(callCache);

	if (size)
	{
		sccUh323InitEventBlock(&sccEvt,appHandle);
		sccEvt.event = SCC_eNetworkGenericMsg;
		sccEvt.subEvent = SCC_eDTMF;
		pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
		memset(pH323Data,0,sizeof(H323EventData));
		pH323Data->dtmf = (DTMFParams *) malloc (sizeof (DTMFParams));
		memset (pH323Data->dtmf, 0, sizeof (DTMFParams));
		/*!!! we just asume it has 1 digit for now! need to support more than one digit later !!!*/
		pH323Data->dtmf->sig = keypadFacStr[0];
		pH323Data->dtmf->duration = DTMF_DEFAULT_DURATION;

		sccEvt.data = pH323Data;
		SCC_DelegateEvent(&sccEvt);
	}

	return(0);
}

int CALLCONV cmEvCallStatus(
	IN      HAPPCALL haCall,
	IN      HCALL hsCall,
	OUT IN  cmCallStatusMessage *callStatusMsg)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallStatus --\n"));
	return(0);
}


int CALLCONV cmEvCallProgress(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 handle)
{
	static char 			fn[] = "cmEvCallProgress() : ";
	UH323CallAppHandle 		*appHandle = (UH323CallAppHandle *)haCall;
	SCC_EventBlock			sccEvt;
	H323EventData			*pH323Data;

	DEBUG(MH323, NETLOG_DEBUG1, ("%s %p\n",fn,hsCall));
	if(haCall == NULL)
	{
		NETERROR(MH323, ("%s : NULL haCall. Dropping call %p!!\n",
			fn,hsCall));
		cmCallDrop(hsCall);
		return 0;
	}

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));

	pH323Data->progress = -1,
	pH323Data->notify = -1;
	pH323Data->cause = -1;
	pH323Data->pi_IE = -1;

	cmCallProgressGet(UH323Globals()->hApp,handle,NULL,0,&pH323Data->cause,
		(progressInd_IE *)&pH323Data->pi_IE,&pH323Data->progress,&pH323Data->notify,NULL);
	NETDEBUG(MH323,NETLOG_DEBUG4,
		("%s pi_IE = %d cause = %d progress = %d notify = %d\n",fn,
		pH323Data->pi_IE,pH323Data->cause,pH323Data->progress,pH323Data->notify));

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = SCC_eNetworkProgress;
	sccEvt.data = pH323Data;
	SCC_DelegateEvent(&sccEvt);
	return 0;
}

#ifdef _ROUTE_H245_
/* H.245 Callbacks */

int CALLCONV cmEvCallCapabilities(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmCapStruct*        capabilities[])
{
	static char fn[] = "cmEvCallCapabilities() ";
	SCC_EventBlock sccEvt;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallCapabilities -- \n"));
	DEBUG(MH323, NETLOG_DEBUG1, ("Capabilities reported by remote:\n"));
	LogCalledLeg(haCall,hsCall,fn);

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No App Handle for %p\n",fn,hsCall));
		return -1;
	}

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = SCC_eNetworkCapabilities;
	SCC_DelegateEvent(&sccEvt);

	return TRUE;
}

int CALLCONV cmEvCallCapabilitiesExt(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmCapStruct***      capabilities[])
{
	static char 	fn[] = "cmEvCallCapabilitiesExt():";
	SCC_EventBlock 	sccEvt;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	int 			i,j,k;
	RTPSet			localSet[MAX_TCS_ENTRIES] = {0};
	int 			ncodecs = 0;
	H323EventData	*pH323Data;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallCapabilitiesExt -- \n"));

	//	DEBUG(MH323, NETLOG_DEBUG4, ("Capability Descriptors: \n\n"));
	for ( i = 0; capabilities[i]; i++)
	{
 //    	DEBUG(MH323, NETLOG_DEBUG4, ("Simultaneous capabilities:\n"));
		for ( j = 0; capabilities[i][j]; j++)
		{
		 //	DEBUG(MH323, NETLOG_DEBUG4, ("  Alternative Capabilities:\n"));
		 for ( k = 0; capabilities[i][j][k]; k++)
		 {
			int codecType = 0,param = 0;
			cmCapStruct *cap = capabilities[i][j][k];
			//DEBUG(MH323, NETLOG_DEBUG4, ("    %d) Capability Name: %s\n", capabilities[i][j][k]->capabilityId, capabilities[i][j][k]->name));
			if ((cap->type == cmCapAudio) && (ncodecs < MAX_TCS_ENTRIES) )
			{
				if(getCodecParam(cap->capabilityHandle, (int *) &localSet[ncodecs].codecType,&localSet[ncodecs].param,&localSet[ncodecs].flags) >= 0 )
				{
				  ncodecs++;
	/*			  DEBUG(MH323, NETLOG_DEBUG4, 
					("dataType = %d dataTypeHandle = %d codec = %d/%d\n", 
					cap->type,cap->capabilityId,localSet[ncodecs].codecType,
					localSet[ncodecs].param));
	*/
				}
			}
		  }
			//DEBUG(MH323, NETLOG_DEBUG4, ("\n"));
		}
	     //DEBUG(MH323, NETLOG_DEBUG4, ("---------------------------------------\n"));
	}

	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));
	if(ncodecs)
	{
		pH323Data->nlocalset = ncodecs;
		pH323Data->localSet = (RTPSet *)calloc(ncodecs,sizeof(RTPSet));
		memcpy(pH323Data->localSet,localSet,ncodecs*sizeof(RTPSet));
	}

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = SCC_eNetworkCapabilities;
	sccEvt.data = pH323Data;
	SCC_DelegateEvent(&sccEvt);

	return TRUE;
}

int CALLCONV cmEvCallNewChannel(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      HCHAN               hsChannel,
		OUT     LPHAPPCHAN          lphaChannel)
{
	char fn[] = "cmEvCallNewChannel():";
  	BOOL	origin = 0;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	UH323CallAppHandle *chanAppHandle;
	CallHandle *callHandle1;
	int						sid;

	if (appHandle == 0)
	{
		NETERROR(MH323, ("%s No Application handle found\n", fn));
		return -1;
	}

    if (cmChannelGetOrigin(hsChannel, &origin) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("cmChannelGetOrigin failed\n"));
	}

	sid = cmChannelSessionId(hsChannel) - 1;
	DEBUG(MH323, NETLOG_DEBUG1, ("%s origin %d sid = %d\n",
		fn, origin,sid));

	// Duplicate the channel app handle
	chanAppHandle = uh323CallAllocAppHandle();
	memcpy(chanAppHandle, appHandle, sizeof(UH323CallAppHandle));

	*lphaChannel = (HAPPCHAN)chanAppHandle;

#if 0
// The initialization is taking place at cmChannelStateOffering for nonFS
// and cmEvChannelSetRTCPAddress for FS channels
	if (sid <0 || sid >= MAX_LOGICAL_CHAN )
	{
		// This is a fast start channel - do initialization

		DEBUG(MH323, NETLOG_DEBUG1, ("%s sid = %d\n",fn, sid));
		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

		callHandle1 = CacheGet(callCache,appHandle->callID);

		if(callHandle1 == NULL)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				   ("%s Did not find Call handle \n",fn));
			goto _return;
		} 

		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Control State = %d\n",fn,H323controlState(callHandle1)));

		/* Fast Start Channels */
		GkInitChannelHandle(callHandle1, origin, hsChannel,sid);
	}
_return:
	CacheReleaseLocks(callCache);
#endif

	return 0;
}

int CALLCONV cmEvCallCapabilitiesResponse(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              status)
{
	static char fn[] = "cmEvCallCapabilitiesResponse() ";
	SCC_EventBlock 			sccEvt;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;

	LogCalledLeg(haCall,hsCall,fn);

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No App Handle for %p\n",fn,hsCall));
		return -1;
	}


	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallCapabilitiesResponse -- \n"));

	if ( status == cmCapAccept)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("Capabilities Accepted By Remote Station\n"));
		sccUh323InitEventBlock(&sccEvt,appHandle);
		sccEvt.event = SCC_eNetworkTCSAck;
		SCC_DelegateEvent(&sccEvt);
	}
	else if ( status == cmCapReject)
		DEBUG(MH323, NETLOG_DEBUG1, 
			("Capabilities Rejected By Remote Station\n"));
	else
		DEBUG(MH323, NETLOG_DEBUG1, 
			("Unknown Status In Capabilities Response\n"));

	// See if there are any other pending capabilities and send them if
	// necessary
	return 0;
}

int CALLCONV cmEvCallMasterSlaveStatus(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              status)
{
	static char fn[] = "cmEvCallMasterSlaveStatus :";
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	CallHandle *callHandle,callHandleBlk;
	int 			event = SCC_eEventNone;
	SCC_EventBlock 	sccEvt;

	NETDEBUG(MH323, NETLOG_DEBUG1,
		("-- cmEvCallMasterSlaveStatus Remote Side = %d --\n", status));

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s, No Application Handle %p\n",fn,hsCall));
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if((callHandle = CacheGet(callCache,appHandle->callID)))
	{
		if(H323controlState(callHandle) == UH323_sControlHeldCapSent) 
		{
			NETDEBUG(MH323, NETLOG_DEBUG3,
				("%s State is ControlHeldCapSent, Sending ControlConnected--\n",
				fn));
			event = SCC_eNetworkControlConnected;
		}
	}
	CacheReleaseLocks(callCache);
	if(event!= SCC_eEventNone)
	{
		sccUh323InitEventBlock(&sccEvt,appHandle);
		sccEvt.event = event;
		SCC_DelegateEvent(&sccEvt);
	}
	return 0;
}

int CALLCONV cmEvCallRoundTripDelay(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      INT32               delay)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallRoundTripDelay -- \n"));
	return 0;
}

int CALLCONV cmEvCallUserInput(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      INT32               nodeId)
{
	static char 		fn[] = "cmEvCallUserInput():";
	UH323CallAppHandle	*appHandle = (UH323CallAppHandle *)haCall;
	SCC_EventBlock 		sccEvt;
	H323EventData		*pH323Data;
	int 				tmpNodeId;
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	HPVT 				peerhVal = cmGetValTree(UH323Globals()->peerhApp);
	cmUserInputSignalStruct  userInputSig;
	int                      nsid;
	cmUserInputIndication    userInputIndication;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallUserInput -- \n"));

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s, No Application Handle\n", fn));
	}

	cmMeiEnter(UH323Globals()->hApp);
	cmMeiEnter(UH323Globals()->peerhApp);


	tmpNodeId = pvtAddRootByPath(peerhVal, 
		cmGetSynTreeByRootName(UH323Globals()->peerhApp, "h245"), 
		"indication.userInput", 0, NULL);

	if(tmpNodeId<0)
	{
		NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
		cmMeiExit(UH323Globals()->peerhApp);
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	else if(pvtSetTree(peerhVal, tmpNodeId, hVal, nodeId) <0)
	{
		pvtDelete(peerhVal,tmpNodeId);
		NETERROR(MH323,("%s pvtSetTree failed\n",fn));
		cmMeiExit(UH323Globals()->peerhApp);
		cmMeiExit(UH323Globals()->hApp);
		return 0;
	}
	nsid = cmUserInputGetDetail (UH323Globals()->hApp, nodeId, &userInputIndication);
	if (nsid < 0) 
	{
		pvtDelete(peerhVal, tmpNodeId);
		NETERROR ( MH323, ("%s cmUserInputGetDetail failed.\n", fn));
		cmMeiExit (UH323Globals()->peerhApp);
		cmMeiExit (UH323Globals()->hApp);
		return 0;
	}

	cmMeiExit(UH323Globals()->peerhApp);
	cmMeiExit(UH323Globals()->hApp);

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = SCC_eNetworkGenericMsg;
	sccEvt.subEvent = SCC_eDTMF;
	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));
	pH323Data->dtmf = (DTMFParams *) malloc (sizeof (DTMFParams));
	memset (pH323Data->dtmf, 0, sizeof (DTMFParams));
	if (userInputIndication == cmUserInputSignal) {
		cmUserInputGetSignal(UH323Globals()->hApp, nsid, &userInputSig);
		pH323Data->dtmf->sig = userInputSig.signalType;
		pH323Data->dtmf->duration = userInputSig.duration;
	}
	else if (userInputIndication == cmUserInputAlphanumeric)
	{
	    char str[64];
	    cmUserInputData userData={str,64};
	    cmNonStandardIdentifier identifier;
	    char data[64];
	    INT32 dataLength;
		if (cmUserInputGet(UH323Globals()->hApp,nodeId,&identifier,data,&dataLength,&userData) >= 0)
		{
			/*!!! we just asume it has 1 digit for now! need to support more than one digit later !!!*/
			pH323Data->dtmf->sig = userData.data[0];
			pH323Data->dtmf->duration = DTMF_DEFAULT_DURATION;
		}
		else
		{
			pH323Data->dtmf->sig = -1;
			pH323Data->dtmf->duration = 0;
		}
	}
	else
	{
		pH323Data->dtmf->sig = -1;
		pH323Data->dtmf->duration = 0;
	}

	pH323Data->nodeId = tmpNodeId;
	sccEvt.data = pH323Data;
	SCC_DelegateEvent(&sccEvt);
	return 0;
}


int CALLCONV cmEvCallRequestMode(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmReqModeStatus     status,
		IN      INT32               nodeId)
{
	char fn[] = "cmEvCallRequestMode():";
	SCC_EventBlock 			sccEvt = {0};
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	CallHandle *callHandle1;
	ConfHandle *confHandle;
	cmReqModeEntry		entry;
	cmReqModeEntry		*ep[1];
	cmReqModeEntry		**epp = NULL;
	void				*ptrs[10];
	cmReqModeEntry		***modes = NULL;
	int					modeId = 0;
	H323EventData		*pH323Data;
	char				modeName[256] = {0};
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	int 				tmpNodeId = 0, len=0, dataNodeId;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallRequestMode -- \n"));

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No App Handle for %p\n",fn,hsCall));
		return -1;
	}

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	callHandle1 = CacheGet(callCache,appHandle->callID);
	if(callHandle1 == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
    	if (status == cmReqModeRequest)
    	{
        	cmCallRequestModeReject(hsCall, "requestDenied");
    	}
		goto _return;
	} 
	
	NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s Received mode message on %p - status %d\n",
		   fn,H323hsCall(callHandle1),status));

	if (status == cmReqModeRequest)
	{
		ep[0] = &entry;
		epp = ep;
		{
			HPVT peerhVal = cmGetValTree(UH323Globals()->peerhApp);
			cmMeiEnter(UH323Globals()->hApp);
			cmMeiEnter(UH323Globals()->peerhApp);

			tmpNodeId = pvtAddRootByPath(peerhVal, 
							cmGetSynTreeByRootName(UH323Globals()->peerhApp, "h245"),
							"request.requestModes", 0, NULL);

			if(tmpNodeId<0)
			{
				NETERROR(MH323,("%s pvtAddRootByPath failed\n",fn));
				cmMeiExit(UH323Globals()->peerhApp);
				cmMeiExit(UH323Globals()->hApp);
				goto _return_reqMode_rel;
			}
			else if(pvtSetTree(peerhVal, tmpNodeId, hVal, nodeId) <0)
			{
				pvtDelete(peerhVal,tmpNodeId);
				NETERROR(MH323,("%s pvtSetTree failed\n",fn));
				cmMeiExit(UH323Globals()->peerhApp);
				cmMeiExit(UH323Globals()->hApp);
				goto _return_reqMode_rel;
			}
			cmMeiExit(UH323Globals()->peerhApp);
			cmMeiExit(UH323Globals()->hApp);
		}

		if (cmRequestModeStructBuild(UH323Globals()->hApp,
			nodeId, epp, 1, ptrs, 10, &modes) != TRUE)				
		{
			NETERROR(MH323, ("%s cmRequestModeStructBuild 1 error %d\n",fn,modeId));
			goto _return_reqMode_rel;
		}

		if (modes == NULL)
		{
			NETERROR(MH323, ("%s modes not found\n", fn));
			goto _return_reqMode_rel;
		}

		NETDEBUG(MH323, NETLOG_DEBUG1, ("%s modes %s\n", fn, modes[0][0]->name));
		modes[0][0]->entryId = -1;

		if (!strcmp(modes[0][0]->name, "t38fax"))
		{
			callHandle1->flags |= FL_CALL_FAX;
		}
		else if ( (dataNodeId = pvtGetByPath( hVal, nodeId, 
				"1.1.type.dataMode.application.nonStandard.data", NULL, &len, NULL )) > 0 )
		{
			if ( pvtGetString(hVal, dataNodeId, len, modeName) > 0 && !strcmp(modeName, "T38FaxUDP") )
			{
				callHandle1->flags |= FL_CALL_FAX;
			}
		}
		if ( callHandle1->flags & FL_CALL_FAX )
		{
			strcpy( modeName, "t38fax" );
		}
		else
		{
			nx_strlcpy(modeName,modes[0][0]->name,256);
		}
	}

	H323doOlc(callHandle1) = 1;

    CacheReleaseLocks(callCache);

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = SCC_eNetworkRequestMode;
	pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
	memset(pH323Data,0,sizeof(H323EventData));
	pH323Data->modeStatus = status;
	pH323Data->nodeId = tmpNodeId;
	strcpy(pH323Data->modeName,modeName);
	sccEvt.data = pH323Data;
	SCC_DelegateEvent(&sccEvt);
    return 0;

_return_reqMode_rel:
	cmCallRequestModeReject(hsCall, "requestDenied");
	H323doOlc(callHandle1) = 0;

_return:
    CacheReleaseLocks(callCache);
    return -1;
}


int CALLCONV cmEvCallMiscStatus(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      cmMiscStatus        status)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallMiscStatus -- \n"));
	return 0;
}

int CALLCONV cmEvCallControlStateChanged(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              state,
		IN      UINT32              stateMode)
{
	static char fn[] = "cmEvCallControlStateChanged():";
	BOOL origin = FALSE;
	int event = SCC_eEventNone;
	SCC_EventBlock sccEvt;
	CallHandle *callHandle,callHandleBlk;
	ConfHandle confHandleBlk;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;

#if 0

	// Capability and MSD is automatic
	    if(state != cmControlStateConnected)
		{
			return 0;
		}
	// End Automatic Capability and MSD
#endif

	LogCalledLeg(haCall,hsCall,fn);

	if (cmCallGetOrigin(hsCall, &origin) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("cmCallGetOrigin:: returned error\n"));
	}

	NETDEBUG(MH323, NETLOG_DEBUG1, 
		(" -- cmEvCallControlStateChanged origin %d -- \n", origin));
	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No App Handle for %p\n",fn,hsCall));
		return -1;
	}

    switch(state)
    {
		case cmControlStateTransportConnected:
			DEBUG(MH323, NETLOG_DEBUG1, 
				(" -- cmEvCallControlStateChanged: cmControlStateTransportConnected -- \n"));

			event = SCC_eNetworkTransportConnected;

		   	break;
	    case cmControlStateTransportDisconnected:
		   DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallControlStateChanged: cmControlStateTransportDisconnected -- \n"));
			CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
			if((callHandle = CacheGet(callCache,appHandle->callID)))
			{
				CloseFceH245Holes(callHandle);
			}
			CacheReleaseLocks(callCache);

		   break;

	    case cmControlStateConnected:
		   	DEBUG(MH323, NETLOG_DEBUG1, 
				(" -- cmEvCallControlStateChanged: cmControlStateConnected -- \n"));
			event = SCC_eNetworkControlConnected;
		   	break;

	    case cmControlStateConference:
		   DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallControlStateChanged: cmControlStateConference -- \n"));
		   break;

	    case cmControlStateFastStart:
		   	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvCallControlStateChanged: cmControlStateFastSetup -- \n"));

			CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
			if((callHandle = CacheGet(callCache,appHandle->callID)))
			{
				H323doOlc(callHandle) = 0;
			}
			CacheReleaseLocks(callCache);

			break;

	    default:
		   DEBUG(MH323, NETLOG_DEBUG1, (" ?? cmEvCallControlStateChanged: Unknown Call Control State ?? \n"));
		   break;
	      }
	
	if(event == SCC_eEventNone)
	{
		return 0;
	}

	sccUh323InitEventBlock(&sccEvt,appHandle);
	sccEvt.event = event;
	SCC_DelegateEvent(&sccEvt);

	return 0;
}

int CALLCONV cmEvCallMasterSlave(
		IN      HAPPCALL            haCall,
		IN      HCALL               hsCall,
		IN      UINT32              terminalType,
    		IN      UINT32              statusDeterminationNumber)
{
	NETDEBUG(MH323, NETLOG_DEBUG4, (" -- cmEvCallMasterSlave -- \n"));
	NETDEBUG(MH323, NETLOG_DEBUG4,
		("Remote Side Terminal Type = %d, StatusDeterminationNumber = %d\n", 
		terminalType, statusDeterminationNumber));
	return 0;
}

int CALLCONV cmEvChannelStateChanged(
		IN      HAPPCHAN            haChannel,
		IN      HCHAN               hsChannel,
		IN      UINT32              state,
		IN      UINT32              stateMode)
{
	char fn[] = "cmEvChannelStateChanged():";
   	BOOL origin;
	int chanState = 0;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haChannel;
	CallHandle *callHandle;
	char 					callIdStr[CALL_ID_LEN];
	SCC_EventBlock sccEvt;
	H323EventData	*pH323Data;
	int sid = -5;

	if(state == cmChannelStateOffering || 
		state ==cmChannelStateConnected || 
		state == cmChannelStateDisconnected )
	{
		sid = cmChannelSessionId(hsChannel) - 1;
		if (sid <0 || sid >= MAX_LOGICAL_CHAN)
		{
			NETERROR(MH323, 
				("%s sessionId out of range = %d. hschan = %p state = %d/%d\n",
				fn,sid,hsChannel,state,stateMode));
			return -1;
		}
	}

	if (appHandle == NULL)
	{
		if(state == cmChannelStateIdle)
		{
			DEBUG(MH323, NETLOG_DEBUG1, 
				(" %s cmChannelStateIdle %p\n", fn,hsChannel));
			
			/* release local channel resources */
	
			DEBUG(MH323, NETLOG_DEBUG1, ("Closing Channelhandle %p\n",
				hsChannel));
	
			cmChannelClose(hsChannel);
		}

		NETERROR(MH323, ("%s No Application Handle found state = %d\n", 
			fn,state));
		return 0;
	}

	if (cmChannelGetOrigin(hsChannel, &origin) < 0)
	{
		NETERROR(MH323, ("%s cmChannelGetOrigin returned error\n", fn));
		return -1;
	}

	DEBUG(MH323, NETLOG_DEBUG1,
		("%s %p origin=%d, state=%d, mode=%d sid = %d\n",
			fn, hsChannel, origin, state, stateMode,sid));

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	if(!(callHandle = CacheGet(callCache, appHandle->callID)))
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s did not find Call handle %s \n",
		   fn, (char*) CallID2String(appHandle->callID,callIdStr)));
		goto _return;
	} 

	switch ( state)
	{
	case cmChannelStateDialtone:
	     DEBUG(MH323, NETLOG_DEBUG1, (" %s cmChannelStateDialtone\n", fn));
	     break;

	case cmChannelStateRingBack:
	     DEBUG(MH323, NETLOG_DEBUG1, (" %s cmChannelStateRingBack -- \n", fn));
	     break;

	case cmChannelStateConnected:
	     DEBUG(MH323, NETLOG_DEBUG1, (" %s cmChannelStateConnected -- \n", fn));
		/* We must send an OLC ack to the peer, only if we were the originator
		 */	

#ifdef H323v30
		if((origin == TRUE) && 
			!((callHandle->leg == SCC_CallLeg1) && (H323doOlc(callHandle) == 0)) )
			/* we should not generate this for channels from fastconnect on leg1 
			   There is no need and it also causes problem in iwf sm 
			   */
		{
			sccUh323InitEventBlock(&sccEvt,appHandle);
			sccEvt.evtProcessor = SCC_ChannelStateConnected;
			sccEvt.data = pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
			memset(pH323Data,0,sizeof(H323EventData));
			pH323Data->sid  = sid;

			SCC_DelegateEvent(&sccEvt);
		}
#else

		/* For version 4 stack faststart, the channel state connected comes 
		 * before callstate changed for alerting and proceeding but not for connect.
		 * Hence we don't need the kludge to convert channelconnect to connect.
		 */
		if((origin == TRUE) && 
		!((callHandle->leg == SCC_CallLeg1) && (H323doOlc(callHandle) == 0)) )
		/* we should not generate this for channels from fastconnect on leg1 
		   There is no need and it also causes problem in iwf sm 
		   */
		{
			if(H323doOlc(callHandle))
			/* Non Fast Start Channel */
			{
				NETDEBUG(MH323, NETLOG_DEBUG1,
					(" %s Received ChannelConnect for non fast start channel.\n", fn));
				if(callHandle->flags & FL_CALL_FAX)
				{
					/* Cisco bug - sends sid = 1 in olcAck for t38 fax */
					int i;
					for(i = MAX_LOGICAL_CHAN-1;i>=0;--i)
					{
						if(H323outChan(callHandle)[i].hsChan == hsChannel)
						{
							//H323inChan(callHandle)[sid].active == TRUE;
							sid = i;
							break;
						}
					}
				}
				sccUh323InitEventBlock(&sccEvt,appHandle);
				sccEvt.event = SCC_eNetworkChanConnect;
				sccEvt.data = pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
				memset(pH323Data,0,sizeof(H323EventData));
				pH323Data->sid  = sid;
				SCC_DelegateEvent(&sccEvt);
			}
			else if(callHandle->state != SCC_sConnected && H323chanEvt(callHandle))
			{
				/* This is the case when we have received fastconnect without receiveing 
				* channel callbacks first. The event was differed in wait for chan connect
				*/
				char str[128];
				sccUh323InitEventBlock(&sccEvt,appHandle);
				sccEvt.event = (H323chanEvt(callHandle));
				H323chanEvt(callHandle) = 0;
				NETDEBUG(MH323, NETLOG_DEBUG1,
					(" %s Received ChannelConnect for fast start channel. Transforming to %s\n",
					fn,SCC_EventToStr(sccEvt.event,str)));
				SCC_DelegateEvent(&sccEvt);
			}
			else {
				NETDEBUG(MH323, NETLOG_DEBUG1,
					(" %s Ignoring ChannelConnect .\n", fn));
			}
		}

#endif
	    break;

	case cmChannelStateDisconnected:
	     DEBUG(MH323, NETLOG_DEBUG1, 
			(" %s cmChannelStateDisconnected \n", fn));

		if(origin != TRUE)
		{
			sccUh323InitEventBlock(&sccEvt,appHandle);
			sccEvt.event = SCC_eNetworkCLC;
			sccEvt.data = pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
			memset(pH323Data,0,sizeof(H323EventData));
			pH323Data->sid  = sid;
			SCC_DelegateEvent(&sccEvt);
		}
		else {
			H323outChan(callHandle)[sid].hsChan = NULL;
			H323outChan(callHandle)[sid].active = FALSE;
			DEBUG(MH323, NETLOG_DEBUG1, 
				(" %s CLCAck - checking for pending olc\n", fn));
			processPendingEvents(callHandle);
		}

	    break;

	case cmChannelStateIdle:
	     	DEBUG(MH323, NETLOG_DEBUG1, 
				(" %s cmChannelStateIdle \n", fn));
		
		/* release local channel resources */

		DEBUG(MH323, NETLOG_DEBUG1, ("Closing Channelhandle %p\n",
			hsChannel));

   		cmChannelClose(hsChannel);
		free(appHandle);
		if(origin == TRUE)
		{
		} else {
		}


	    break;

	case cmChannelStateOffering:
	     	DEBUG(MH323, NETLOG_DEBUG1, 
				("%s cmChannelStateOffering\n", fn));
			/* Non Fast Start Channels */
			GkInitChannelHandle(callHandle, origin, hsChannel,sid);

		/* new incoming channel, we should not originate the channel to
		 * the peer yet. We will do it in another callback function.
		 */

		break;
	}

_return:
	CacheReleaseLocks(callCache);
	return 0;
}

int CALLCONV cmEvChannelNewRate(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              rate)
{
	return 0;
}

int CALLCONV cmEvChannelMaxSkew(
		IN      HAPPCHAN            haChan1,
		IN      HCHAN               hsChan1,
		IN      HAPPCHAN            haChan2,
		IN      HCHAN               hsChan2,
		IN      UINT32              skew)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelMaxSkew -- \n"));
	return 0;
}



/* 	Save channel Address in the callhandle. 
*	If peer is H323 and we originated this channel (outgoing channel) then
*	copy and set the in channel address on peer.
*/
int CALLCONV cmEvChannelSetAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              ip,
		IN      UINT16              port)
{
   	BOOL	origin, callOrigin;
	char fn[] = "cmEvChannelSetAddress():";
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haChan;
	CallHandle *callHandle1, *callHandle2;
	ConfHandle *confHandle;
	char 		callID2[CALL_ID_LEN] = {0};
	char 		callIdStr[CALL_ID_LEN];
	int			sid;

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No Application Handle found\n", fn));
		return -1;
	}

	ip = ntohl(ip);
	if (cmChannelGetOrigin(hsChan, &origin) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("%s cmCallGetOrigin:: returned error\n", fn));
	}

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelSetAddress %x:%d origin %d -- \n",
		ip,port, origin));

	sid = cmChannelSessionId(hsChan) - 1;
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelSetAddress %x:%d origin %d sid = %d-- \n",
		ip,port, origin,sid));

	if (sid <0 || sid >= MAX_LOGICAL_CHAN)
	{
		NETERROR(MH323, 
			("%s sessionId out of range = %d. hschan = %p origin = %d port = %d\n",
			fn,sid,hsChan,origin,port));
		return -1;
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);

	callHandle1 = CacheGet(callCache, appHandle->callID);

	if(callHandle1 == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG2,
		   ("%s Did not find callID. callID=%s \n",
		   fn, (char*) CallID2String(appHandle->callID,callIdStr)));
		goto _return;
	} 

	if((origin == TRUE) && (callHandle1->flags & FL_CALL_FAX))
	{
		/* Cisco bug - sends sid = 1 in olcAck for t38 fax */
		int i;
		for(i = MAX_LOGICAL_CHAN-1;i>=0;--i)
		{
			if(H323outChan(callHandle1)[i].hsChan == hsChan)
			{
				sid = i;
				break;
			}
		}
	}

	confHandle = CacheGet(confCache,appHandle->confID);

	if(confHandle == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG2,
		   ("%s Did not find ConfHandle. confID=%s \n",
		   fn, (char*) CallID2String(appHandle->confID,callIdStr)));

		goto _return;
	} 
	
	NETDEBUG(MH323, NETLOG_DEBUG4,
	   ("%s Channel for hsCall = %p \n",fn,H323hsCall(callHandle1)));

	DEBUG(MH323, NETLOG_DEBUG1, (" %s %s/%d origin %d -- \n",
		fn, (char*) ULIPtostring(ip), port, origin));

	GkInitChannelAddress(callHandle1, origin,
		ip, port,sid);
#if 0 // don't drectly interact with other leg
if (origin == TRUE)
	{
		getPeerCallID(appHandle->confID,callHandle1->callID, callID2);
		callHandle2 = CacheGet(callCache,callID2);

		if(callHandle2 == NULL)
		{
			NETDEBUG(MH323, NETLOG_DEBUG4,
				   ("%s Did not find Call handle for other leg \n",fn));
			goto _return;
		} 

		if (callHandle2->handleType == SCC_eH323CallHandle)
		{
			DEBUG(MH323, NETLOG_DEBUG4,
				("%s calling GkSetChannelAddress for peer\n", fn));

			GkSetChannelAddress(callHandle2, ip, port);
		}
	}
#endif

_return:
	CacheReleaseLocks(callCache);
	CacheReleaseLocks(confCache);

	return 0;
}

int CALLCONV cmEvChannelSetRTCPAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32              ip,
		IN      UINT16              port)
{
	char fn[] = "cmEvChannelSetRTCPAddress():";
   	BOOL	origin;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haChan;
	CallHandle 				*callHandle;
	char 					callIdStr[CALL_ID_LEN];
	int						sid;

	ip = ntohl(ip);
    NETDEBUG(MH323,NETLOG_DEBUG4,("%s ip/port = %s/%d\n",fn,
		    (char*) ULIPtostring(ip), port));

    /* Clarent problem - uses 2 different rtcp addresses
    *  we will always use rtp+1 as rtcp
    */

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No Application Handle found\n", fn));
		return -1;
	}

	if (cmChannelGetOrigin(hsChan, &origin) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("%s cmCallGetOrigin:: returned error\n", fn));
	}

	sid = cmChannelSessionId(hsChan) - 1;
	DEBUG(MH323, NETLOG_DEBUG1, (" %s %s/%d origin %d sid = %d -- \n",
		fn, (char*) ULIPtostring(ip), port, origin,sid));

	if (sid <0 || sid >= MAX_LOGICAL_CHAN)
	{
		NETERROR(MH323, 
			("%s sessionId out of range = %d. hschan = %p origin = %d port = %d\n",
			fn,sid,hsChan,origin,port));
		return -1;
	}

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);

	if(!(callHandle = CacheGet(callCache, appHandle->callID)))
	{
		NETDEBUG(MH323, NETLOG_DEBUG2,
		   ("%s Did not find CallHandle. callID=%s \n",
		   fn, (char*) CallID2String(appHandle->callID,callIdStr)));
		goto _return;
	} 

	if((origin == TRUE) && (callHandle->flags & FL_CALL_FAX))
	{
		/* Cisco bug - sends sid = 1 in olcAck for t38 fax */
		int i;
		for(i = MAX_LOGICAL_CHAN-1;i>=0;--i)
		{
			if(H323outChan(callHandle)[i].hsChan == hsChan)
			{
				sid = i;
				break;
			}
		}
	}

	GkInitChannelRTCPAddress(callHandle,origin,ip,port,sid);
	//if(origin == 0)
	{
		GkInitChannelHandle(callHandle, origin, hsChan,sid);
	}

_return:
	CacheReleaseLocks(callCache);
    return 0;
}

int CALLCONV cmEvChannelParameters(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      char*               channelName,
		IN      HAPPCHAN            haChanSameSession,
		IN      HCHAN               hsChanSameSession,
		IN      HAPPCHAN            haChanAssociated,
		IN      HCHAN               hsChanAssociated,
		IN      UINT32              rate)
{
	char 					fn[] = "cmEvChannelParameters():";
	BOOL    				origin = 0;
	UH323CallAppHandle 		*appHandle = (UH323CallAppHandle *)haChan;
	CallHandle 				*callHandle;
	char 					callIdStr[CALL_ID_LEN];
	int						codecType;
	int						sid;

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No Application Handle found\n", fn));
		return -1;
	}

	if (cmChannelGetOrigin(hsChan, &origin) < 0)
	{
		DEBUG(MH323, NETLOG_DEBUG1, 
			("%s cmCallGetOrigin:: returned error\n", fn));
	}


	sid = cmChannelSessionId(hsChan) - 1;
    DEBUG(MH323, NETLOG_DEBUG1,
        (" -- cmEvChannelParameters (%s origin = %d rate = %d) sid = %d-- \n",
        channelName ? channelName : "", origin,rate,sid));

	if (sid <0 || sid >= MAX_LOGICAL_CHAN)
	{
		NETERROR(MH323, 
			("%s sessionId out of range = %d. hschan = %p, origin = %d\n",
			fn,sid,hsChan,origin));
		return -1;
	}

    if (channelName)
    {
        codecType = ChannelCodec(channelName);
        if (codecType < 0)
        {
            // This is probably a codec which we dont support

			return -1;
        }

		CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
		if((callHandle = CacheGet(callCache, appHandle->callID)) == 0 )
		{
			NETDEBUG(MH323, NETLOG_DEBUG2,
			   ("%s Did not find ConfHandle. confID=%s \n",
			   fn, (char*) CallID2String(appHandle->confID,callIdStr)));
			CacheReleaseLocks(callCache);
			return 0;
		} 

		if((origin == TRUE)  && (callHandle->flags & FL_CALL_FAX))
		{
			/* Cisco bug - sends sid = 1 in olcAck for t38 fax */
			int i;
			for(i = MAX_LOGICAL_CHAN-1;i>=0;--i)
			{
				if(H323outChan(callHandle)[i].hsChan == hsChan)
				{
					sid = i;
					break;
				}
			}
		}

		if(origin == TRUE)
		{
			H323outChan(callHandle)[sid].codec = codecType;
		}
		else 
		{
			H323inChan(callHandle)[sid].codec = codecType;
		}
		CacheReleaseLocks(callCache);
	}
	return 0;
}

int CALLCONV cmEvChannelRTPDynamicPayloadType(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      INT8                dynamicPayloadType)
{

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelRTPDynamicPayloadType type = %d -- \n",dynamicPayloadType));
	return 0;
}

int CALLCONV cmEvChannelVideoFastUpdatePicture(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelVideoFastUpdatePicture -- \n"));
	return 0;
}

int CALLCONV cmEvChannelVideoFastUpdateGOB(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 firstGOB,
		IN      int                 numberOfGOBs)
{
   	BOOL	origin;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelVideoFastUpdateGOB -- \n"));
	return 0;
}

int CALLCONV cmEvChannelVideoFastUpdateMB(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 firstGOB,
		IN      int                 firstMB,
		IN      int                 numberOfMBs)
{
   BOOL	origin;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelVideoFastUpdateMB -- \n"));
	return 0;
}

int CALLCONV cmEvChannelHandle(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      int                 dataTypeHandle,
		IN      cmCapDataType       dataType)
{
	char fn[] = "cmEvChannelHandle():";
   	BOOL origin;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haChan;
	CallHandle *callHandle1;
	SCC_EventBlock	sccEvt = {0};
	int		codecType,param,flags = 0;
	H323EventData	*pH323Data;
	int						sid;

	/* We have the channel data type, if we have the RTCP address, 
	 * we can originate a channel to the peer. We do this only
	 * if we are not the originator of the channel.
	 */

#if 0
	if (dataType != cmCapAudio)
	{
		/* Close the channel */
		cmChannelDrop(hsChan);
		return -1;
	}
#endif

	if (appHandle == NULL)
	{
		NETERROR(MH323, ("%s No Application Handle found\n", fn));
		return -1;
	}

	if (cmChannelGetOrigin(hsChan, &origin) < 0)
	{
		NETERROR(MH323, 
			("%s cmChannelGetOrigin returned error\n", fn));
		return -1;
	}

	sid = cmChannelSessionId(hsChan) - 1;
	if (sid <0 || sid >= MAX_LOGICAL_CHAN)
	{
		NETERROR(MH323, 
			("%s sessionId out of range = %d. hschan = %p origin = %d\n",
			fn,sid,hsChan,origin));
		return -1;
	}

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	callHandle1 = CacheGet(callCache, appHandle->callID);

	if (callHandle1 == NULL)
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
		goto _return;
	} 

	/* We have all parameters of this channel now. Now
	 * if we have to open the channel for peer, we can do
	 * it 
	 */
	
	if (dataType == cmCapAudio)
	{
		getCodecParam(dataTypeHandle,&codecType,&param,&flags);
	}

	NETDEBUG(MH323, NETLOG_DEBUG4,
   	("%s Channel for leg %d handle = %d codec %d/%d doOlc = %d flag = 0x%x\n",
		fn,callHandle1->leg,dataTypeHandle,codecType,param,
		H323doOlc(callHandle1),flags));

	GkInitChannelDataType(callHandle1, origin, dataTypeHandle,
		dataType,param,sid,flags);

	if (origin == FALSE)
	{
		if(H323doOlc(callHandle1))
		{ 
			sccUh323InitEventBlock(&sccEvt,appHandle);
			sccEvt.event = SCC_eNetworkOLC;
			pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
			memset(pH323Data,0,sizeof(H323EventData));
			pH323Data->sid = sid;
			sccEvt.data = pH323Data;
			SCC_DelegateEvent(&sccEvt);
		}
		else
		{
			NETDEBUG(MSCC, NETLOG_DEBUG4, ("%s Ignoring OLC, origin = %d\n", fn, origin));
		}
	}

_return:
	CacheReleaseLocks(callCache);

   	return TRUE;
}

int CALLCONV cmEvChannelGetRTCPAddress(
		IN      HAPPCHAN            haChan,
		IN      HCHAN               hsChan,
		IN      UINT32*             ip,
		IN      UINT16*             port)
{
	DEBUG(MH323, NETLOG_DEBUG1, 
		(" -- cmEvChannelGetRTCPAddress -- \n"));
	return 0;
}

int CALLCONV cmEvChannelRequestCloseStatus(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      cmRequestCloseStatus  status)
{
	static char 	fn[] = "cmEvChannelRequestCloseStatus";
	int				origin;
	SCC_EventBlock	sccEvt = {0};
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haChan;
	H323EventData	*pH323Data;
	int				sid;

	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelRequestCloseStatus -- \n"));
	if (cmChannelGetOrigin(hsChan, &origin) < 0)
	{
		NETERROR(MH323, ("%s cmChannelGetOrigin returned error\n", fn));
		return -1;
	}

	if (status==cmRequestCloseRequest)
	   {
            cmChannelAnswer(hsChan);
            cmChannelDrop(hsChan);
			if(origin == TRUE)
			{
				sid = cmChannelSessionId(hsChan) - 1;
				if (sid <0 || sid >= MAX_LOGICAL_CHAN)
				{
					NETERROR(MH323, 
						("%s sessionId out of range = %d. hschan = %p\n",
						fn,sid,hsChan));
					return -1;
				}
				sccUh323InitEventBlock(&sccEvt,appHandle);
				sccEvt.event = SCC_eNetworkCLC;
				sccEvt.data = pH323Data = (H323EventData *) malloc(sizeof(H323EventData));
				memset(pH323Data,0,sizeof(H323EventData));
				pH323Data->sid  = sid;
				SCC_DelegateEvent(&sccEvt);
			}
	   }
	return 0;
}

int CALLCONV cmEvChannelTSTO(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      INT8                  isCommand,
		IN      INT8                  tradeoffValue)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelTSTO -- \n"));
	return 0;
}

int CALLCONV cmEvChannelMediaLoopStatus(
		IN      HAPPCHAN              haChan,
		IN      HCHAN                 hsChan,
		IN      cmMediaLoopStatus     status)
{
	return 0;
}

int CALLCONV cmEvChannelReplace(
		IN 	HAPPCHAN	      haChan,
		IN 	HCHAN	 	      hsChan,
		IN 	HAPPCHAN 	      haReplacedChannel,
		IN 	HCHAN	 	      hsReplacedChannel)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelReplace -- \n"));
	return TRUE;
}

int CALLCONV cmEvChannelFlowControlToZero(
		IN 	HAPPCHAN	      haChan,
		IN      HCHAN                 hsChan)
{
	DEBUG(MH323, NETLOG_DEBUG1, (" -- cmEvChannelFlowControlToZero -- \n"));
	return TRUE;
}

#endif

#if 0
char * fastStartChannelToStr(cmFastStartMessage *fsMsg)
{
	static char 			str[256];
	int 					i;
	cmPartnerChannel 		*partner;
	cmAlternativeChannels	*altChannel;
	char 					*p = str;

	for(i = 0; i<fsMsg.parterChannelsNum; ++i)
	{
		partner = fsMsg.partnerChannels[i];
		altChannel = partner.transmit;
		p+=sprintf(p,"type = %d\t Transmit Channels = %d\n",partner.type,altChannelNumber);
		p+=sprintf(p,"rtp/rtcp/dataTypeHandle/channelName/index\n");
		for(j=0;j<altChannel.altChannelNumber; ++j)
		{
			p+=sprintf(p,"= %d\t",partner.type);
		}
}
#endif
			
			

int
LogCalledLeg(
	IN      HAPPCALL            haCall,
	IN      HCALL               hsCall,
	char	*fn)
{
	CallHandle callHandleBlk1, callHandleBlk2;
	UH323CallAppHandle *appHandle = (UH323CallAppHandle *)haCall;
	ConfHandle confHandleBlk;
	char *callID2;
	HCALL               hsCall2;
	INT32 ip, port;
	
	if (!(NetLogStatus[MH323] & NETLOG_DEBUG1))
	{
		// dont have to log anything
		return 0;
	}

	if(CacheFind(callCache,appHandle->callID,&callHandleBlk1,sizeof(CallHandle))<0 )
	{
		NETDEBUG(MH323, NETLOG_DEBUG4,
			   ("%s Did not find Call handle \n",fn));
		return 0;
	} 

	NETDEBUG(MH323, NETLOG_DEBUG4,
		   ("%s  Received message on Leg %d \n",fn,callHandleBlk1.leg));
	
	if(CacheFind(confCache,appHandle->confID,&confHandleBlk,sizeof(ConfHandle))<0)

	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,
			   ("%s Did not find Found Conf handle \n",fn));
	   	return 0;
	} 
		
	
	if(!memcmp(callHandleBlk1.callID,confHandleBlk.callID[0],CALL_ID_LEN))
	{
			callID2 = confHandleBlk.callID[1];
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received message on call 0\n",fn));
	}
	else if(!memcmp(callHandleBlk1.callID,confHandleBlk.callID[1],CALL_ID_LEN))
	{
			callID2 = confHandleBlk.callID[0];
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received message on call 1\n",fn));
	}
	else {
			NETDEBUG(MH323, NETLOG_DEBUG4,
				("%s  Received message on unknown leg\n",fn));
			return 0;
	}
	return 0;
}

int     CALLCONV cmEvRegEvent(
     IN      HAPP                hApp,
     IN      cmRegState          regState,
     IN      cmRegEvent          regEvent,
     IN      int                 regEventHandle)
{
	NETDEBUG(MH323, NETLOG_DEBUG4, ("not supported\n"));
     return 0;
}


int getAppIp( void *p)
{
	HCALL hsCall = (HCALL)p;
	int ip;
	UH323CallAppHandle *appHandle;
	cmCallGetHandle(hsCall,(void *)&appHandle);
	ip = appHandle->localIP;
	return ip;
}

/******************************************************************************************
 * buildFacilityFastStartOpenChannels
 *
 * Purpose:  This API function enables the caller to supply a structure with data about
 *           the offered logical channels for fast start procedure. The structure includes
 *           all offered channels, both incoming and outgoing, arranged by their type, i.e.
 *           Audio channels, Video channels, etc.
 *
 * Input:    hsCall - A handle to the call whose setup message shall carry the
 *                    fast start offer.
 *
 *           fsMessage - A pointer to the structure containing the channels data.
 *
 * Reurned Value: TRUE or RVERROR.
 *
 ****************************************************************************************/
RVAPI int RVCALLCONV
buildFacilityFastStartOpenChannels(
							IN      HAPPCALL            	haCall,
							IN 		cmFastStartMessage		*fsMessage)
{
	static char 		fn[] = "buildFacilityFastStartOpenChannels";
	HPVT 				hVal = cmGetValTree(UH323Globals()->hApp);
	int 				dataTypeHandle;
	int 				i = 0,j;
	H323RTPSet			*pRtpSet;
	int					dataHandle;
	HPST 				hSyn = cmGetSynTreeByRootName(UH323Globals()->hApp, 
					"capData");
	int nodeId;
	int					rx,tx;
	cmFastStartChannel	*fsChannel;
	UH323CallAppHandle 	*appHandle = (UH323CallAppHandle *)haCall;
	CallHandle 			*callHandle = NULL;
	char 				callIdStr[CALL_ID_LEN];

    if (!fsMessage) return RVERROR;
    memset(fsMessage,0,sizeof(cmFastStartMessage));


	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	callHandle = CacheGet(callCache,appHandle->callID);
    if (!callHandle)
    {
    	NETERROR(MH323, ("%s NULL call handle %s\n",
                fn, CallID2String(appHandle->callID,callIdStr)));
            CacheReleaseLocks(callCache);
		return RVERROR;
    }
	CacheReleaseLocks(callCache); // Release here or in the end  ???
			
	/* One audio channels */
	fsMessage->partnerChannelsNum = 1;

	/* Just one possiblity for an audio channel */
	fsMessage->partnerChannels[0].type = cmCapAudio;

	/* Transmit */
	for(i = 0,j=0, pRtpSet = H323remoteSet(callHandle),
			fsChannel = fsMessage->partnerChannels[0].transmit.channels;
		j < H323nremoteset(callHandle); 
		++j,++pRtpSet)
	{
#ifndef _cisco
		if((dataHandle = createDataHandle(pRtpSet))<0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4, 
				("%s CreateDataHandle Failed\n",fn));
			continue;
		}
#endif /* _cisco */
		fsChannel[i].rtp.port = 0;
		fsChannel[i].rtcp.ip = 0;
		fsChannel[i].rtcp.port = pRtpSet->rtpport+1;
		fsChannel[i].rtcp.ip = htonl(pRtpSet->rtpaddr);
#ifndef _cisco
		fsChannel[i].dataTypeHandle = dataHandle;
		fsChannel[i].channelName = NULL;
#else
		fsChannel[i].dataTypeHandle = -1;
		fsChannel[i].channelName = ChannelName(pRtpSet->codecType);
#endif /* _cisco */
		i++;
	}

	tx = i;
	fsMessage->partnerChannels[0].transmit.altChannelNumber = i;

	/* Receive */
	for(i = 0, j = 0, pRtpSet = H323remoteSet(callHandle), 
			fsChannel = fsMessage->partnerChannels[0].receive.channels;
		j < H323nremoteset(callHandle); ++j,++pRtpSet)
	{
#ifndef _cisco 
		if((dataHandle = createDataHandle(pRtpSet))<0)
		{
			NETDEBUG(MH323,NETLOG_DEBUG4, 
				("%s CreateDataHandle Failed\n",fn));
			continue;
		}
#endif /* _cisco */

		fsChannel[i].rtp.ip = htonl(pRtpSet->rtpaddr);
		fsChannel[i].rtp.port = pRtpSet->rtpport;
		fsChannel[i].rtcp.ip = htonl(pRtpSet->rtpaddr);
		fsChannel[i].rtcp.port = pRtpSet->rtpport +1;
#ifndef _cisco
		fsChannel[i].dataTypeHandle = dataHandle;
		fsChannel[i].channelName = NULL;
#else 
		fsChannel[i].dataTypeHandle = -1;
		fsChannel[i].channelName = ChannelName(pRtpSet->codecType);
#endif /* _cisco */
		i++;
	}

	rx = i;
	fsMessage->partnerChannels[0].receive.altChannelNumber = i;


#if 0
#ifndef _cisco
	cmMeiEnter(UH323Globals()->hApp);
	for(i = 0, fsChannel = fsMessage->partnerChannels[0].receive.channels;
		i<rx; ++i)
	{
		pvtDelete(hVal,fsChannel[i].dataTypeHandle);
	}

	for(i = 0, fsChannel = fsMessage->partnerChannels[0].transmit.channels;
		i<tx; ++i)
	{
		pvtDelete(hVal,fsChannel[i].dataTypeHandle);
	}
	cmMeiExit(UH323Globals()->hApp);
#endif		
#endif		
    return  TRUE;
}

RVAPI int RVCALLCONV
buildFacilityFastStartCloseChannels(
							IN      HAPPCALL            haCall,
							IN 		cmFastStartMessage		*fsMessage)
{
	static char 				fn[] = "buildFacilityFastStartCloseChannels";
	int							nCodecs = 1;
	int 						i = 0;

    if (!fsMessage) return RVERROR;
	memset( fsMessage, (int32_t) 0, sizeof(cmFastStartMessage) );

	//
 	// One audio channels 
 	//

	fsMessage->partnerChannelsNum = 1;
	
	//
	// Just one possiblity for an audio channel 
	//
	
	fsMessage->partnerChannels[0].type = cmCapEmpty;

	//
	// Transmit 
	//

	i = 0;

	while (i < nCodecs)
	{
		cmFastStartChannel *fschannel = 
			&fsMessage->partnerChannels[0].transmit.channels[i];

		fschannel->rtp.ip = 0;
		fschannel->rtp.port = 0;
		fschannel->rtcp.ip = 0;
		fschannel->rtcp.port = 0;

		fschannel->dataTypeHandle = -1;
		//fschannel->channelName = codecs[i];

		i++;
	}

	fsMessage->partnerChannels[0].transmit.altChannelNumber = nCodecs;

	//
 	// Receive 
 	//

	i = 0;

	while (i < nCodecs)
	{
		cmFastStartChannel *fschannel = 
			&(fsMessage->partnerChannels[0].receive.channels[i]);

		fschannel->rtp.ip = 0;
		fschannel->rtp.port = 0;
		fschannel->rtcp.ip = 0;
		fschannel->rtcp.port = 0;

		fschannel->dataTypeHandle = -1;
		//fschannel->channelName = codecs[i];
		
		i++;
	}

	fsMessage->partnerChannels[0].receive.altChannelNumber = nCodecs;

	return TRUE;
}


RVAPI
int RVCALLCONV sendFacilityFastStartOpenChannel(
						IN      HAPPCALL            		haCall,
                        IN   	HCALL       				hsCall,
						IN		INT16						lcn)	
{
	static char 				fn[] = "sendFacilityFastStartOpenChannels";
    cmFastStartMessage  		fsMessage;
	if (buildFacilityFastStartOpenChannels(haCall, &fsMessage) < 0)
	{
			fprintf(stderr, "%s Failed to build Fast Start\n",fn);
    		return RVERROR;
	}
	else
	{
		if (sendFacilityFastStart(haCall, hsCall, &fsMessage, lcn) < 0)
		{
			fprintf(stderr, "%s Failed to build Fast Start\n",fn);
    		return RVERROR;
		}
	}
    return 0;
}

RVAPI
int RVCALLCONV sendFacilityFastStartCloseChannel(
						IN      HAPPCALL            		haCall,
                        IN   	HCALL       				hsCall,
						IN		INT16						lcn)	
{
	static char 				fn[] = "sendFacilityFastStartCloseChannels";
    cmFastStartMessage  		fsMessage;
	
	if (buildFacilityFastStartCloseChannels(haCall, &fsMessage) < 0)
	{
			fprintf(stderr, "%s Failed to build Fast Start\n",fn);
    		return RVERROR;
	}
	else
	{
		if (sendFacilityFastStart(haCall, hsCall, &fsMessage, lcn) < 0)
		{
			fprintf(stderr, "%s Failed to build Fast Start\n",fn);
    		return RVERROR;
		}
	}
    return 0;
}
