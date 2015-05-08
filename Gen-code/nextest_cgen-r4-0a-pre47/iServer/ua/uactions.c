#include <sys/types.h>
#include <time.h>
#include "gis.h"
#include "sipcallactions.h"
#include "firewallcontrol.h"
#include "scm_call.h"
#include <malloc.h>
#include "nxosd.h"
#include "ua.h"
#include "callutils.h"
#include "bridge.h"
#include "radclient.h"
#include "log.h"
#include "gk.h"
#include "tsm.h"

/* Action routines for both Bridge and network sides */
int 
SipQueueNetworkInvite(SipEventHandle *evb)
{
	char fn[] = "SipQueueNetworkInvite():";
	
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));
	return(0);
}

int 
SipQueueBridgeInvite(SipEventHandle *evb)
{
	char fn[] = "SipQueueBridgeInvite():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	
	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// We will queue this invite, if we are in a state
	// which allows queueing. However, we must check, if 
	// Bridge already has an invite pending

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->uaflags & UAF_PENDBINVITE)
	{
		// error
		NETERROR(MSIP,
			("%s Bridge already has pending INVITE, dropping\n",
			fn));
		goto _error;
	}

	if (sipCallHandle->pendBInvite)
	{
		// error
		NETERROR(MSIP,
			("%s Bridge has pending INVITE event block, dropping\n",
			fn));
		goto _error;
	}
	
	// no error, also check for already existing event
	sipCallHandle->pendBInvite = evb;

	CacheReleaseLocks(callCache);

	return 0;

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeInvite(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeInvite():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	char*           replaces = NULL;
	char*           alert_info = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	// store the replaces header
	if(appMsgHandle->msgHandle && appMsgHandle->msgHandle->replaces)
	{
		replaces = strdup(appMsgHandle->msgHandle->replaces);
	}

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	/* Do sanity checks and insert the call handle */
	if ((callHandle = SipBridgeCreateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeCreateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}

	SipPrintState(callHandle);

	callHandle->lastEvent = SCC_evtSipInviteTx;

	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->inviteOrigin = 1;
	callHandle->callSource=1;
	timedef_cur(&callHandle->callStartTime);

	if (sipCallHandle->successfulInvites == 0)
	{
		callHandle->leg = SCC_CallLeg2;
	}

	callHandle->conf_id = CStrdup(callCache, sipCallHandle->callLeg.callid);

	// Check if it the call is a fax
	if(callHandle->handle.sipCallHandle.remoteSet.remoteSet_val &&
		callHandle->handle.sipCallHandle.remoteSet.remoteSet_val[0].codecType == T38Fax)
	{
		callHandle->flags |= FL_CALL_FAX;
	}

	callHandle->callError = SCC_errorNone;
	BillCall(callHandle, CDR_CALLSETUP);


	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if((replaces == NULL) && msgHandle && msgHandle->replaces)
	{
		replaces = strdup(msgHandle->replaces);
	}

	if(msgHandle && msgHandle->alert_info) 
	{
		alert_info = strdup(msgHandle->alert_info);
	}

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle, 
						"INVITE", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	msgHandle->alert_info = alert_info;

	appMsgHandle->timerSupported = 1;
	appMsgHandle->minSE = sipminSE;
	appMsgHandle->sessionExpires = sipsessionexpiry;
	appMsgHandle->refresher = SESSION_REFRESHER_UAC;


	msgHandle->replaces = replaces;

	// We must start timer C here 
	SipStartTimerC(callHandle);

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeReInvite(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeReInvite():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	int i;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	callHandle->lastEvent = SCC_evtSipReInviteTx;
	SipPrintState(callHandle);

	/* Incerement the cseq no */
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->localCSeqNo ++;

	appMsgHandle->timerSupported = 1;
	appMsgHandle->minSE = sipminSE;
	appMsgHandle->sessionExpires = sipsessionexpiry;
	appMsgHandle->refresher = SESSION_REFRESHER_UAC;

	// Check if it the call is a fax
	if(callHandle->handle.sipCallHandle.remoteSet.remoteSet_val &&
			callHandle->handle.sipCallHandle.remoteSet.remoteSet_val[0].codecType == T38Fax)
	{
		callHandle->flags |= FL_CALL_FAX;
	}

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle, 
							"INVITE", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if ((appMsgHandle->localSet == NULL) &&
		sipCallHandle->remoteSet.remoteSet_val &&
		(appMsgHandle->flags & SIPAPP_USELOCALSDP))
	{
		// There is no SDP, see if the flag is set
		appMsgHandle->nlocalset = sipCallHandle->remoteSet.remoteSet_len;

		appMsgHandle->localSet = 
			malloc(sipCallHandle->remoteSet.remoteSet_len*sizeof(RTPSet));

		memcpy(appMsgHandle->localSet, sipCallHandle->remoteSet.remoteSet_val,
			appMsgHandle->nlocalset*sizeof(RTPSet));

		appMsgHandle->attr_count = sipCallHandle->remoteAttr.remoteAttr_len;
		if (sipCallHandle->remoteAttr.remoteAttr_len)
		{
			appMsgHandle->attr = 
				malloc (sipCallHandle->remoteAttr.remoteAttr_len*sizeof (SDPAttr));
			for (i=0;i<sipCallHandle->remoteAttr.remoteAttr_len;i++)
			{
				SipDupAttrib(&appMsgHandle->attr[i], &sipCallHandle->remoteAttr.remoteAttr_val[i]);
			}
		}
	}

	// Mark this as locally originated if needed
	if (appMsgHandle->flags & SIPAPP_LOCALINVITE)
	{
		sipCallHandle->uaflags |= UAF_LOCALINVITE;
	}

	// We must start timer C here
	SipStopTimerC(callHandle);
	SipStartTimerC(callHandle);

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeAlerting(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeAlerting():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	int				responseCode;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	callHandle->lastEvent = SCC_evtSip100xTx;
	
	if (timedef_iszero(&callHandle->callRingbackTime))
	{
		timedef_cur(&callHandle->callRingbackTime);
	}

	SipPrintState(callHandle);

	/* We need to generate a tag if we havent already done so */
	SipCreateTags(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if(msgHandle)
	{
		responseCode = msgHandle->responseCode;
	}
	else if (appMsgHandle->responseCode)
	{
		responseCode = appMsgHandle->responseCode;
	}
	else
	{
		responseCode = 180;
	}

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle, 
							"INVITE", SIPMSG_RESPONSE, responseCode)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeConnect(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeConnect():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle *sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	// Time stamp the connect
	if(timedef_iszero(&callHandle->callConnectTime))
	{
		timedef_cur(&callHandle->callConnectTime);
	}

	callHandle->lastEvent = SCC_evtSip200Tx;
	sipCallHandle = SipCallHandle(callHandle);

	// sipCallHandle should be non null because BridgeUpdate worked
	sipCallHandle->successfulInvites ++;

	SipPrintState(callHandle);

	/* We need to generate a tag if we havent already done so */
	SipCreateTags(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"INVITE", SIPMSG_RESPONSE, 200)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	/* Set session timers if remote endpoint in this leg supports */
	if (callHandle->timerSupported)
	{
		appMsgHandle->timerSupported = 1;
		appMsgHandle->minSE = callHandle->minSE;
		appMsgHandle->sessionExpires = callHandle->sessionExpires;
		appMsgHandle->refresher = callHandle->refresher;
	}
	else
	{
		appMsgHandle->timerSupported = 0;
		appMsgHandle->sessionExpires = 0;
		appMsgHandle->refresher = SESSION_REFRESHER_NONE;
	}


	SipEventMsgHandle(evb) = msgHandle;

	if ((appMsgHandle->localSet == NULL) &&
		sipCallHandle->remoteSet.remoteSet_val &&
		(appMsgHandle->flags & SIPAPP_USELOCALSDP))
	{
		// There is no SDP, see if the flag is set
		appMsgHandle->nlocalset = sipCallHandle->remoteSet.remoteSet_len;
		appMsgHandle->sdpVersion = sipCallHandle->rsdpVersion;

		appMsgHandle->localSet = 
			malloc(sipCallHandle->remoteSet.remoteSet_len*sizeof(RTPSet));

		memcpy(appMsgHandle->localSet, sipCallHandle->remoteSet.remoteSet_val,
			appMsgHandle->nlocalset*sizeof(RTPSet));
	}

	// Start Session timer only on successful call setup and if the Source
	// can handle session timers(set in NetworkInvite)
	if (sipCallHandle->successfulInvites == 1 && callHandle->timerSupported)
	{
		SipStopSessionTimer(callHandle);
		if (callHandle->refresher == SESSION_REFRESHER_UAS)
		{
			SipStartSessionTimer(callHandle);
		}
		else
		{
			if (callHandle->refresher == SESSION_REFRESHER_UAC)
			{
				NETDEBUG ( MSIP, NETLOG_DEBUG4, ("%s we are not the refresher %d", fn,
							 callHandle->refresher));
			}
		}
	}

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipSimulateNetworkAck(SipEventHandle *evb)
{
	char fn[] = "SipSimulateNetworkAck():";

	/* Based on the event handle, construct a fresh
	 * handle to send the ack out
	 */
	return(0);
}

int 
SipHandleBridgeAck(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeAck():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle	*callHandle = NULL;
	SipCallHandle	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	callHandle->lastEvent = SCC_evtSipAckTx;
	sipCallHandle = SipCallHandle(callHandle);
	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"ACK", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (sipCallHandle->uaflags & UAF_LOCALINVITE)
	{
		appMsgHandle->nlocalset = 0;
		sipCallHandle->uaflags &= ~UAF_LOCALINVITE;
	}

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SCMCALL_Replicate(callHandle);

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeCancel(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeCancel():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipEventHandle *evHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"CANCEL", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	GkCallSetRemoteErrorCodes(callHandle, &evb->callDetails);
	memcpy(&callHandle->callDetails2, &evb->callDetails, 
		sizeof(CallDetails));

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SipCloseCall(callHandle, 1);

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	//SipEventAppHandle(evb) = NULL;
	//SipFreeEventHandle(evb);

	//return SipTransSendMsgHandle(appMsgHandle)?-1:1;
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeInfo(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeInfo():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipEventHandle *evHandle;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	/* evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);*/

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->localCSeqNo ++;

	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"INFO", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	if (appMsgHandle->realmInfo == NULL) {
		appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}
	SipEventMsgHandle(evb) = msgHandle;


	CacheReleaseLocks(callCache);

	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleBridgeInfoResponse (SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeInfoResponse():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	int				responseCode;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if(msgHandle)
	{
		responseCode = msgHandle->responseCode;
	}
	else if (appMsgHandle->responseCode)
	{
		responseCode = appMsgHandle->responseCode;
	}
	else
	{
		responseCode = 200;
	}

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle, 
							"INFO", SIPMSG_RESPONSE, responseCode)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeBye(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeBye():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipEventHandle *evHandle;
	char *also = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

#if 0
	// We should not kill the INVITE transaction
	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);
#endif

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	SipPrintState(callHandle);

	// If the To-tag is not initialized yet, we 
	// cannot send any BYE...

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if(msgHandle && msgHandle->also)
	{
		also = strdup(msgHandle->also);
	}

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	/* Incerement the cseq no */
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->localCSeqNo ++;
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"BYE", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	msgHandle->also = also;

	SipEventMsgHandle(evb) = msgHandle;

	GkCallSetRemoteErrorCodes(callHandle, &evb->callDetails);
	memcpy(&callHandle->callDetails2, &evb->callDetails, 
		sizeof(CallDetails));

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SipCloseCall(callHandle, 1);

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	//SipEventAppHandle(evb) = NULL;
	//SipFreeEventHandle(evb);

	//return SipTransSendMsgHandle(appMsgHandle)?-1:1;
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

// Is not an actual action routine, but must follow
// all the rules
int 
SipHandleBridgeHunting(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeHunting():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	char			callID2[CALL_ID_LEN] = {0};
	NetoidSNKey		tmpkey;
	unsigned short 	tmpcap = 0;	
	header_url 			*newrequri = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	// Do all the cleanup needed first
	if (callHandle->flags & FL_CALL_TAP)
	{
		CallTap(callHandle, 0);
	}

	// Timestamp the call
	timedef_cur(&callHandle->callEndTime);

	// Log the call if needed
	// if (!(evb->callDetails.flags&REDIRECT)) - Log All invites going out
	{
		BillCall(callHandle, CDR_CALLHUNT);
	}

	if (CallFceBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), CallFceBundleId(callHandle));
		CallFceBundleId(callHandle) = 0;
	}

	memset(&callHandle->callDetails2, 0, sizeof(CallDetails));

	// Push the current dest into the callHandle list
	GwAddPhoNodeToRejectList(&callHandle->rfphonode, callHandle->crname,
		&callHandle->destRejectList, CMalloc(callCache));

	// Reset the dest information in the phonode
	memcpy(&tmpkey, &callHandle->rfphonode, sizeof(NetoidSNKey));
	BIT_COPY(tmpcap, CAP_IGATEWAY, callHandle->rfphonode.cap, CAP_IGATEWAY);
	memset(&callHandle->rfphonode, 0, sizeof(PhoNode));
	memcpy(&callHandle->rfphonode, &tmpkey, sizeof(NetoidSNKey));
	callHandle->rfphonode.cap = tmpcap;

	if(evb->callDetails.flags & REDIRECT && msgHandle->remotecontact)
	{
		newrequri = UrlDup(msgHandle->remotecontact, sipCallCache->malloc);
		nx_strlcpy(callHandle->rfphonode.phone, SVal(msgHandle->remotecontact->name), sizeof(callHandle->rfphonode.phone));

	}
	else if(callHandle->remotecontact_list)
	{
		newrequri = SipPopUrlFromContactList(&callHandle->remotecontact_list, sipCallCache->free);
		nx_strlcpy(callHandle->rfphonode.phone, newrequri->name, sizeof(callHandle->rfphonode.phone));
	}
	else
	{
		// The original input number is stored here
		strcpy(callHandle->rfphonode.phone, CallInputNumber(callHandle));
	}

	SipStripUriParams(callHandle->rfphonode.phone);
	BIT_SET(callHandle->rfphonode.sflags, ISSET_PHONE);

	// Finished with  msgHandle
	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	// Delete call handle for leg2 from conf handle
	if (getPeerCallID(appMsgHandle->confID,appMsgHandle->callID,callID2) < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s could not get peer call id\n", fn));
	}
	else
	{
		GisDeleteCallFromConf(callID2, appMsgHandle->confID);
	}

	// Re-Create...
	callHandle->state = Sip_sWORRing;
	GkCallResetRemoteErrorCodes(callHandle);
	callHandle->callError = 0;
	callHandle->cause = 0;

	evb->event = Sip_eNetworkInvite;
	evb->type = Sip_eNetworkEvent;

	// Reset the app msg handle
	SipFreeAppMsgHandle(appMsgHandle);
	SipEventAppHandle(evb) = NULL;

	if ((appMsgHandle = SipCreateAppMsgHandleForCall(callHandle, newrequri)) == NULL)
	{
		NETERROR(MSIP, ("%s SipCreateAppMsgHandleForCall failed\n", fn))
		goto _release_locks;
	}

	CacheReleaseLocks(callCache);

	appMsgHandle->maxForwards = callHandle->handle.sipCallHandle.maxForwards;

        // Re-populate the appMsgHandleStructure with the privacy headers.
        
        if( callHandle->handle.sipCallHandle.pAssertedID_Sip )
        {
                appMsgHandle->pAssertedID_Sip = 
                        UrlDup( callHandle->handle.sipCallHandle.pAssertedID_Sip, MEM_LOCAL);
                
        }
        if( callHandle->handle.sipCallHandle.pAssertedID_Tel )
        {
                appMsgHandle->pAssertedID_Tel =
                        strdup( callHandle->handle.sipCallHandle.pAssertedID_Tel );
        }
        if( callHandle->handle.sipCallHandle.original_from_hdr )
        {
                appMsgHandle->original_from_hdr = 
                        UrlDup( callHandle->handle.sipCallHandle.original_from_hdr, MEM_LOCAL );
                
        }
        if( callHandle->handle.sipCallHandle.rpid_hdr )
        {
                appMsgHandle->rpid_hdr = 
                        UrlDup( callHandle->handle.sipCallHandle.rpid_hdr, MEM_LOCAL );
                
        }
        if( callHandle->handle.sipCallHandle.rpid_url )
        {
                appMsgHandle->rpid_url =
                        strdup( callHandle->handle.sipCallHandle.rpid_url );
        }
        if( callHandle->handle.sipCallHandle.priv_value )
        {
                appMsgHandle->priv_value =
                        strdup( callHandle->handle.sipCallHandle.priv_value );
        }
        if( callHandle->handle.sipCallHandle.proxy_req_hdr )
        {
            appMsgHandle->proxy_req_hdr =
                        strdup( callHandle->handle.sipCallHandle.proxy_req_hdr );
        }    

        appMsgHandle->incomingPrivType = callHandle->handle.sipCallHandle.incomingPrivType ; 
        appMsgHandle->privLevel        = callHandle->handle.sipCallHandle.privLevel        ;

	// We can assign the message handle only after the
	// app msg handle is assigned
	SipEventAppHandle(evb) = appMsgHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s Sending Call again to bridge\n", fn));


	UrlFree(newrequri, sipCallCache->free);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	UrlFree(newrequri, sipCallCache->free);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	UrlFree(newrequri, sipCallCache->free);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipSimulateBridgeFinalResponse(SipEventHandle *evb, int bridgeError)
{
	char fn[] = "SipSimulateBridgeFinalResponse():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	header_url_list *list_entry, *new_list_entry = NULL;
	header_url *contact_url = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	GkCallSetRemoteErrorCodes(callHandle, &evb->callDetails);

	if(appMsgHandle->msgHandle && appMsgHandle->msgHandle->remotecontact_list)
	{
		list_entry = appMsgHandle->msgHandle->remotecontact_list->prev;

		do
		{
			contact_url = list_entry->url;

			new_list_entry = MMalloc (sipCallCache->malloc, sizeof(header_url_list));
			new_list_entry->url = UrlDup(contact_url, sipCallCache->malloc);

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

			list_entry = list_entry->prev;
		}
		while(list_entry != appMsgHandle->msgHandle->remotecontact_list->prev);
	}

	callHandle->callError = evb->callDetails.callError;
	memcpy(&callHandle->callDetails2, &evb->callDetails, 
		sizeof(CallDetails));
	callHandle->responseCode = appMsgHandle->responseCode;

	if (callHandle->callSource == 0 &&
		sipCallHandle->successfulInvites == 0 &&
		(max_hunt_allowable_duration == 0 ||
			difftime(time(NULL), timedef_sec(&callHandle->callStartTime)) < max_hunt_allowable_duration) &&
		((callHandle->remotecontact_list && evb->callDetails.responseCode != 486)
        || ((evb->callDetails.flags & HUNT_TRIGGER ||
        h323HuntError(evb->callDetails.callError, evb->callDetails.cause - 1)) &&
		(evb->callDetails.flags & REDIRECT ||
		(callHandle->nhunts < callHandle->maxHunts && BIT_TEST(callHandle->rfphonode.cap, CAP_IGATEWAY))))))
	{
		CacheReleaseLocks(callCache);
		return SipHandleBridgeHunting(evb);
	}


	/* We need to generate a tag if we havent already done so */
	SipCreateTags(callHandle);

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	// Since we are going to be sending the final response,
	// we must make sure that its the correct one. Get the
	// right response code from prev cdr if needed.
	if (!BillCallIsCdrValid(callHandle))
	{
		BillCallSetFromPrevCdr(callHandle);
		appMsgHandle->responseCode = callHandle->responseCode;
	}

	// If the final response is one which we can hunt on,
	// we will not send it on

	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"INVITE", SIPMSG_RESPONSE,
						appMsgHandle->responseCode)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (bridgeError || 
		(appMsgHandle->responseCode == 481) ||
		(appMsgHandle->responseCode == 408))
	{
		SipCloseCall(callHandle, 1);
	}
	else
	{
		if ((appMsgHandle->responseCode == 401) ||
			(appMsgHandle->responseCode == 407))
		{
			/* start timer */
			SipStartTimerC(callHandle);
		}
		else
		{
			SipCloseCall(callHandle, 0);
		}
	}

	CacheReleaseLocks(callCache);

	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}


int 
SipHandleBridgeFinalResponse(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeFinalResponse():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	return SipSimulateBridgeFinalResponse(evb, 0);
}

int 
SipHandleNetworkInvite(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkInvite():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	CacheTableInfo scacheInfoEntry = {0}, *srcCacheInfo = NULL;
	SipEventHandle 	*evHandle = NULL;
	int 			finalResponse = 503;
	PhoNode			*phonodep = NULL;
	char 			s1[24];
	unsigned long		ipaddr;
	int 			rcSrc = -1;
	struct itimerval timerval;
	char *callid;
	SipTransKey siptrankey = {0};
	SipTrans *siptran = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	/* Check for To tag to determine whether this is a new Invite or a
	 * reinvite. This is already done in TSM but is repeated here to 
     * handle the following case:
	 * Under load conditions for H323-SIP fax calls, it was observed that 
     * the call handle for the H323 leg is there when TSM gets a reinvite 
     * on leg 2 for an active call. It determines that this is a reinvite 
     * based on the presence of a To tag and forwards it to UA. But by 
     * the time the event reaches UA, the call handles have been 
     * deleted because of a RelComp from the H323 leg. The UA 
     * therefore treats this as a new Invite and forwards it as a 
     * Setup event to Bridge. This causes a core in IWFInitiateSetup(). 
	 * A call handle is created for such an Invite before this function
     * is called. But no CDR will be written since this is technically
     * not a new call. The original call has been released and 
     * the CDR for it has already been written.
	 */
	if (msgHandle && msgHandle->local && msgHandle->local->tag)
	{
		NETERROR(MSIP, ("%s Ignoring incoming new Invite with To tag %s\n", 
			fn, msgHandle->local->tag));

		// Form the key for the pending UAS Invite transaction
		SipFormTranKey(SIPTRAN_UAS, SipMsgFrom(msgHandle), SipMsgTo(msgHandle),
			SipMsgCallID(msgHandle), SipMsgCallCSeqNo(msgHandle),
			SipMsgCallCseqMethod(msgHandle), &siptrankey);

		// Remove the pending UAS Invite transaction
		CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
		if((siptran = CacheGet(transCache, &siptrankey)) != NULL)
		{
			SipRemoveTSM(siptran);
			TranFreeQueue(siptran);
			MFree(transCache->free, siptran);
		}
		CacheReleaseLocks(transCache);

		// Delete the call handle created by SipUAEventProcessorWorker()
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
		if ((callHandle = CacheGet(callCache, appMsgHandle->callID)) != NULL)
		{
			CacheDelete(callCache, appMsgHandle->callID); 
			GisFreeCallHandle(callHandle);
		}
		CacheReleaseLocks(callCache);

		// Free event handle
		SipFreeEventHandle(evb);

		return -1;
	}

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	/* Do sanity checks and insert the call handle */
	if ((callHandle = SipNetworkCreateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipNetworkCreateCallHandle failed\n", fn));
		goto _error;
	}

	// Check if it the call is a fax
	if(callHandle->handle.sipCallHandle.localSet.localSet_val &&
			callHandle->handle.sipCallHandle.localSet.localSet_val[0].codecType == T38Fax)
	{
		callHandle->flags |= FL_CALL_FAX;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(appMsgHandle->natip && appMsgHandle->natport)
	{
		callHandle->natip = appMsgHandle->natip;
		callHandle->natport = appMsgHandle->natport;
	}

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->successfulInvites == 0)
	{
		callHandle->leg = SCC_CallLeg1;
	}

	// time stamp the call start time
	timedef_cur(&callHandle->callStartTime);

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSipInviteRx;
	SipPrintState(callHandle);

	/* Process session timer parameters here .*/
	callHandle->timerSupported = appMsgHandle->timerSupported;
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Session Timer %s", fn, 
						callHandle->timerSupported ? "enabled" : "disabled"));
	
	if (appMsgHandle->timerSupported)
	{
		if (appMsgHandle->minSE < sipminSE ) 
		{
			callHandle->minSE = sipminSE;
		} 
		else
		{
			callHandle->minSE = appMsgHandle->minSE;
		}

		if (appMsgHandle->sessionExpires < sipminSE)
		{
			/* error send back a 422 response */
			finalResponse = 422;
			callHandle->callError = SCC_errorGeneral;
			goto _drop_call;
		} 

		if (appMsgHandle->sessionExpires > sipsessionexpiry )
		{
			callHandle->sessionExpires = sipsessionexpiry;
		} else
		{
			callHandle->sessionExpires = appMsgHandle->sessionExpires;
		}
		if (appMsgHandle->refresher == SESSION_REFRESHER_NONE)
		{
			appMsgHandle->refresher = SESSION_REFRESHER_UAS;
		}
		callHandle->refresher = appMsgHandle->refresher;
	}
	else
	{
		callHandle->refresher = SESSION_REFRESHER_NONE;
		callHandle->sessionExpires = 0;
	}


	/* Now we need to properly adjust the event handle */
	appMsgHandle->calledpn = UrlDup(msgHandle->requri, MEM_LOCAL);

	// Set Realm of this URI as -1, as this is something we have to figure out
	appMsgHandle->calledpn->realmId = -1;
	appMsgHandle->requri->realmId = -1;

	SipStripUriParams(appMsgHandle->calledpn->name);
	
	// Privacy related changes
	if(appMsgHandle->incomingPrivType == privacyTypeNone) {
	  appMsgHandle->callingpn = UrlDup(SipMsgFrom(msgHandle), MEM_LOCAL);
	}
	else {
	  if(appMsgHandle->incomingPrivType == privacyTypeRFC3325) {
	    appMsgHandle->callingpn         = UrlDup(appMsgHandle->pAssertedID_Sip, MEM_LOCAL);
	  }
	  else if(appMsgHandle->incomingPrivType == privacyTypeDraft01) {
	    appMsgHandle->callingpn         = UrlDup(appMsgHandle->rpid_hdr,MEM_LOCAL);
	  }
	}

        // Modify DNIS here.
	// Do Source identification and port allocation

	phonodep = &callHandle->phonode;
	srcCacheInfo = &scacheInfoEntry;

	rcSrc = SipFindSrc(appMsgHandle->callingpn, msgHandle->srchost, 
				phonodep->ipaddress.l, callHandle->tg, "", appMsgHandle->calledpn->name, callHandle->realmInfo, srcCacheInfo);

	if (rcSrc < 0)
	{
		if (!allowSrcAll)
		{
			callHandle->callError = SCC_errorBlockedUser;
			finalResponse = 403;

			goto _drop_call;
		}

		callHandle->vpnName = strdup("");
		callHandle->zone = strdup("");
		callHandle->cpname = strdup("");
	}	
	else 
	{
		strcpy(phonodep->regid, srcCacheInfo->data.regid);
		phonodep->uport = srcCacheInfo->data.uport;
		callHandle->vendor = srcCacheInfo->data.vendor;
		callHandle->cap = srcCacheInfo->data.cap;
		callHandle->ecaps1 = srcCacheInfo->data.ecaps1;
		callHandle->vpnName = strdup(srcCacheInfo->data.vpnName);
		callHandle->zone = strdup(srcCacheInfo->data.zone);
		callHandle->cpname = strdup(srcCacheInfo->data.cpname);
		if (BIT_TEST(srcCacheInfo->data.sflags, ISSET_PHONE))
		{
			strcpy(callHandle->phonode.phone, srcCacheInfo->data.phone);
			BIT_SET(callHandle->phonode.sflags, ISSET_PHONE);
		}

		h323RemoveTcs2833 ? (callHandle->ecaps1 |= ECAPS1_DELTCSRFC2833) : (callHandle->ecaps1 &= ~ECAPS1_DELTCSRFC2833);
		h323RemoveTcsT38 ? (callHandle->ecaps1 |= ECAPS1_DELTCST38) : (callHandle->ecaps1 &= ~ECAPS1_DELTCST38);

		if( !(callHandle->ecaps1 & ECAPS1_NATDETECT) )
		{
			if(BIT_TEST(srcCacheInfo->data.sflags, ISSET_NATIP))
			{
				callHandle->natip = srcCacheInfo->data.natIp;
				if(BIT_TEST(srcCacheInfo->data.sflags, ISSET_NATPORT))
				{
					callHandle->natport = srcCacheInfo->data.natPort;
				}
			}
			else
			{
				callHandle->natip = 0;
				callHandle->natport = 0;
			}
		}

		NETDEBUG(MFCE, NETLOG_DEBUG4,
			("%s Source: %s:%u \n", fn, FormatIpAddress(callHandle->natip, s1), callHandle->natport));

		if (GwPortAllocCall(&callHandle->phonode,1, 0) < 0)
		{
			callHandle->callError = SCC_errorInadequateBandwidth;

			goto _drop_call;
		}

		if (srcCacheInfo->data.custID[0] != '\0')
		{
			callHandle->custID = 
				CStrdup(callCache, srcCacheInfo->data.custID);
		}

		if (srcCacheInfo->data.srcIngressTG[0] != '\0')
		{
			/* now replace tg with srcIngressTG */
			if ( callHandle->tg )
			{
				/* free it first, it might too short for strcpy */
				(CFree(callCache))(callHandle->tg);
			}
			callHandle->tg = CStrdup(callCache,
						srcCacheInfo->data.srcIngressTG);
		}

		if (srcCacheInfo->data.srcEgressTG[0] != '\0')
		{
			/* now replace tg with srcEgressTG */
			if ( callHandle->destTg )
			{
				/* free it first, it might too short for strcpy */
				(CFree(callCache))(callHandle->destTg);
			}
			callHandle->destTg = CStrdup(callCache,
						srcCacheInfo->data.srcEgressTG);
		}

		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Source: %s/%lu %s %s\n", 
			fn, srcCacheInfo->data.regid,
			srcCacheInfo->data.uport, srcCacheInfo->data.phone,
			FormatIpAddress(srcCacheInfo->data.ipaddress.l, s1)));

		callHandle->maxHunts = srcCacheInfo->data.maxHunts;

		if (callHandle->maxHunts == 0)
		{
			callHandle->maxHunts = maxHunts;
		}
		else if (callHandle->maxHunts > SYSTEM_MAX_HUNTS)
		{
			callHandle->maxHunts = SYSTEM_MAX_HUNTS;
		}

		callHandle->callError = SCC_errorNone;
		BillCall(callHandle, CDR_CALLSETUP);

                if( (srcCacheInfo->data.cidblock == 1) && 
		    appMsgHandle->generate_cid != cid_unblock)
                {
                        appMsgHandle->generate_cid = cid_block;
                }                
	}


	if(GisAddCallToConf(callHandle) != 0)
	{
		NETERROR(MSIP, ("%s GisAddCallToConf Failed\n", fn));

		// mark the call as having been billed
		callHandle->callError = SCC_errorNoVports;

		goto _drop_call;
	}

	if(!allowHairPin)
	{
		// Hairpin is NOT allowed, add src into reject list.
		// If src is not found, add the src ip addr
		GwAddPhoNodeToRejectList(&callHandle->phonode, NULL,
			&callHandle->destRejectList, CMalloc(callCache));
	}

	/* Start max call duration timer */
	if(max_call_duration > 0)
	{
		if((callid = malloc(CALL_ID_LEN)))
		{
			memcpy(callid, callHandle->callID, CALL_ID_LEN);

			bzero(&timerval, sizeof(struct itimerval));

			timerval.it_value.tv_sec = max_call_duration;

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
			NETERROR(MSIP, ("%s unable to malloc callid\n", fn));
		}
	}

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	if(billingType == BILLING_CISCOPREPAID)
	{
		/* Authenticate user with Radius Server */
		return SipCiscoRadiusAuthenticate(evb);
	}
	else
	{
		/* Send the event to bridge */
		return SipUASendToBridge(evb);
	}

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_drop_call:
	// Prepare to send a final resp
	evHandle = SipEventHandleDup(evb);
	SipSendNetworkFinalResponse(evHandle, finalResponse);

	// Free the call handle, bill the call 	
	BillCall(callHandle, CDR_CALLDROPPED);

	CallDelete(callCache, callHandle->callID);
	CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);
	GisFreeCallHandle(callHandle);

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkReInvite(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkReInvite():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipCallHandle 	*sipCallHandle = NULL;
	int				finalResponse = 500;
	int				generateLocalResponse = 0;
	SipEventHandle	*evHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipNetworkUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipNetworkUpdateCallHandle failed\n", fn));
		goto _error;
	}

	sipCallHandle = SipCallHandle(callHandle);
	
	// Check if it the call is a fax
	if(callHandle->handle.sipCallHandle.localSet.localSet_val &&
			callHandle->handle.sipCallHandle.localSet.localSet_val[0].codecType == T38Fax)
	{
		callHandle->flags |= FL_CALL_FAX;
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSipReInviteRx;
	SipPrintState(callHandle);
	/* 401/407 scenario. Endpoint sends back ReInvite with creds */
	SipStopTimerC(callHandle);

	// If Invite is coming in from a mirror proxy we need to copy calledpn and callingpn
	if( !appMsgHandle->calledpn )
	{
		appMsgHandle->calledpn = UrlDup(msgHandle->requri, MEM_LOCAL);
	}
	SipStripUriParams(appMsgHandle->calledpn->name);
	
	if( !appMsgHandle->callingpn )
	{
		// Privacy related changes
		if(appMsgHandle->incomingPrivType == privacyTypeNone) {
			appMsgHandle->callingpn = UrlDup(SipMsgFrom(msgHandle), MEM_LOCAL);
		}
		else {
			if(appMsgHandle->incomingPrivType == privacyTypeRFC3325) {
				appMsgHandle->callingpn         = UrlDup(appMsgHandle->pAssertedID_Sip, 
									 MEM_LOCAL);
			}
			else if(appMsgHandle->incomingPrivType == privacyTypeDraft01) {
				appMsgHandle->callingpn         = UrlDup(appMsgHandle->rpid_hdr,MEM_LOCAL);
			}
		}
	}

	// Check to see if SDP was changed. If there is no change, generate
	// A 200 OK back locally, dont send the event to the bridge
	if ((appMsgHandle->nlocalset || localReInviteNoSdp) &&
		(appMsgHandle->mediaChanged == 0) &&
		sipCallHandle->remoteSet.remoteSet_len)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s New Invite has same SDP, regenerating local UA response\n", fn));
		generateLocalResponse = 1;
		sipCallHandle->uaflags |= UAF_NETIDINVITE;
	}

	if (generateLocalResponse)
	{
		// Dup the event handle
		evHandle = SipEventHandleDup(evb);
		appMsgHandle = SipEventAppHandle(evHandle);
		// Use local sdp next time
		appMsgHandle->flags |= SIPAPP_USELOCALSDP;

		// set the event to a bridge connect
		evHandle->event = Sip_eBridge200;
		evHandle->type = Sip_eBridgeEvent;
	
		// Event may not be immediately scheduled
		SipUAProcessEvent(evHandle);

		goto _release_locks;
	}

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	evHandle = SipEventHandleDup(evb);
	SipSendNetworkFinalResponse(evHandle, finalResponse);

	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);

	return -1;
}

int 
SipHandleNetworkAlerting(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkAlerting():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipNetworkUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipNetworkUpdateCallHandle failed\n", fn));
		goto _error;
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSip100xRx;

	if (timedef_iszero(&callHandle->callRingbackTime))
	{
		timedef_cur(&callHandle->callRingbackTime);
	}

	SipPrintState(callHandle);

	// Reset TimerC
	SipStopTimerC(callHandle);
	SipStartTimerC(callHandle);

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkConnect(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkConnect():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle *sipCallHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle 	*pendEvb = NULL;
	int				localInvite = 0;

	SipSimulateNetworkAck(evb);

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipNetworkUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipNetworkUpdateCallHandle failed\n", fn));
		goto _error;
	}

	if(timedef_iszero(&callHandle->callConnectTime))
	{
		timedef_cur(&callHandle->callConnectTime);
	}

	callHandle->lastEvent = SCC_evtSip200Rx;

	sipCallHandle = SipCallHandle(callHandle);

	// sipCallHandle should be non null because NetworkUpdate worked
	sipCallHandle->successfulInvites ++;

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	/* Check the tags to see if they match or need
	 * to be initialized
	 */
	if (SipInitializeTags(callHandle, msgHandle) < 0)
	{
		NETERROR(MSIP, ("%s SipInitializeTags failed\n", fn));
		goto _error;
	}

	SipPrintState(callHandle);

	if (sipCallHandle->pendBInvite)
	{
		pendEvb = sipCallHandle->pendBInvite;

		sipCallHandle->pendBInvite = NULL;
		sipCallHandle->uaflags &= ~UAF_PENDBINVITE;
	}

	// get rid of timer C
	SipStopTimerC(callHandle);

	// If this is the First 200OK check Session timer negotiation result
	// Session-Timer can be turned off by peer during mid-dialog
	if (sipCallHandle->successfulInvites > 0)
	{
		if (appMsgHandle->timerSupported)
		{
			if (appMsgHandle->sessionExpires > sipsessionexpiry )
			{
				callHandle->sessionExpires = sipsessionexpiry;
			} else
			{
				callHandle->sessionExpires = appMsgHandle->sessionExpires;
			}
			callHandle->timerSupported = 1;
			if (appMsgHandle->refresher == SESSION_REFRESHER_NONE)
				appMsgHandle->refresher = SESSION_REFRESHER_UAC;
		}
		else
		{
			callHandle->timerSupported = 0;
			appMsgHandle->refresher = SESSION_REFRESHER_UAC;
		}
		callHandle->refresher = appMsgHandle->refresher;
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Session Timer %s", fn, 
							callHandle->timerSupported ? "enabled" : "disabled"));

	}

	// Either this was the first 200 OK, or 
	// response to a ReInvite sent on Session-timer expiry handler
	if ((sipCallHandle->uaflags & UAF_LOCALINVITE) || callHandle->timerSupported)
	{
		SipStopSessionTimer(callHandle);
		if (callHandle->refresher == SESSION_REFRESHER_UAC)
		{
			SipStartSessionTimer(callHandle);
		}
		else
		{
			NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s We are not refresher %d %d", 
						fn, callHandle->refresher,
						appMsgHandle->refresher));
		}
	}

	if (sipCallHandle->uaflags & UAF_LOCALINVITE)
	{
		localInvite = 1;
	}

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	// Event must be immediately scheduled
	if (pendEvb)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Processing Pending Invite\n", fn));

		SipUAProcessEventWorker(pendEvb);	
	}

	// The event will be sent to bridge only if the INVITE was
	// not locally generated

	if (localInvite)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Response was for local Invite, holding in UA\n",
			fn));
		/* We should be sending ACK back here */
		evb->event = Sip_eBridgeAck;
		evb->type = Sip_eBridgeEvent;
		appMsgHandle->maxForwards = sipmaxforwards;
		return SipUAProcessEvent (evb);
	}
	else
	{
		/* Send the event to bridge */
		return SipUASendToBridge(evb);
	}

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkAck(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkAck():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle	*sipCallHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	int				forwardAck = 1;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	sipCallHandle = SipCallHandle(callHandle);
	if (sipCallHandle->uaflags & UAF_NETIDINVITE)
	{
		forwardAck = 0;
		sipCallHandle->uaflags &= ~UAF_NETIDINVITE;
	}

	if ((callHandle = SipNetworkUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipNetworkUpdateCallHandle failed\n", fn));
		goto _error;
	}

	// No longer need dest reject list
	GwFreeRejectList(callHandle->destRejectList, CFree(callCache));
	callHandle->destRejectList = NULL;

	// No longer need remotecontact list
	SipFreeRemotecontactList(callHandle->remotecontact_list);
	callHandle->remotecontact_list = NULL;

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSipAckRx;
	SipPrintState(callHandle);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	if (((appMsgHandle->mediaChanged) && sipCallHandle->remoteSet.remoteSet_len) && (!forwardAck)) {
		forwardAck = 1;
		sipCallHandle->uaflags |= UAF_NETIDINVITE;
	}

	if (forwardAck)
	{
		SCMCALL_Replicate(callHandle);

		CacheReleaseLocks(callCache);

		/* Send the event to bridge */
		return SipUASendToBridge(evb);
	}
	else
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Holding Ack in UA\n", fn));

		CacheReleaseLocks(callCache);

		SipFreeEventHandle(evb);

		return 1;
	}

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkCancel(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkCancel():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	/*
	 * We must generate a 487 on the network side also
	 */
	evHandle = SipEventHandleDup(evb);

	appMsgHandle = SipEventAppHandle(evHandle);
	SipSendNetworkFinalResponse(evHandle, 487);

	// Now get on with our regular stuff
	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	SipPrintState(callHandle);

	evb->callDetails.callError = callHandle->callError = SCC_errorAbandoned;
	evb->callDetails.lastEvent = callHandle->lastEvent;

	SipCloseCall(callHandle, 1);

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkBye(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkBye():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

#if 0
	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);
#endif

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	SipPrintState(callHandle);

	evb->callDetails.callError = callHandle->callError = SCC_errorNone;
	evb->callDetails.lastEvent = callHandle->lastEvent;

	SipCloseCall(callHandle, 1);

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleNetworkInfo (SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkInfo():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey   leg = { 0 } ;
	SipEventHandle *evHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

    /* Update Cseq number */
    callHandle->handle.sipCallHandle.remoteCSeqNo = msgHandle->cseqno;

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	if (appMsgHandle->realmInfo == NULL ) {
		appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SipPrintState(callHandle);

/*
	evb->callDetails.callError = callHandle->callError = SCC_errorNone;
	evb->callDetails.lastEvent = callHandle->lastEvent;
*/

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleNetworkInfoResponse (SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkInfoResponse():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey   leg = { 0 } ;
	SipEventHandle *evHandle;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	if (appMsgHandle->realmInfo == NULL ) {
		appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

#if 0
int
SipHandleNetworkInfo1xx (SipEventHandle *evb)
{
	return (SipHandleNetworkInfoResponse (evb));
}

int
SipHandleNetworkInfo200 (SipEventHandle *evb)
{
	return (SipHandleNetworkInfoResponse (evb));
}

int
SipHandleNetworkInfoFinalResponse (SipEventHandle *evb)
{
	return (SipHandleNetworkInfoResponse (evb));
}
#endif

int 
SipHandleNetworkFinalResponse(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkFinalResponse():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle 	*pendEvb = NULL;
	int				localInvite = 0;
	int 			rc=1;
	CacheTableInfo *info = NULL;
	char *auth;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle->responseCode == 491)
	{
		// This is special scenario, we have to wait
		// and send out the invite again later on
		// If we are the origin, wait between [2.4-4]
		// else [0-2].
		return SipHandleNetwork491(evb);
	}

	if (msgHandle->responseCode == 422 )
	{
		return SipHandleNetwork422 (evb);
	}

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	if(msgHandle->responseCode == 401 || msgHandle->responseCode == 407)
	{
		info = CacheGet(regCache, &callHandle->rfphonode);

		callHandle->lastEvent =  SCC_evtSipAuthReqRx;
		if(info && (info->data.stateFlags & CL_UAREG) )
		{
			evb->event = Sip_eBridgeInvite;

			auth = createAuth(&info->data, msgHandle->method, appMsgHandle->hdrProxyauthenticate);

			appMsgHandle->hdrProxyauthorization = auth;

			appMsgHandle->maxForwards = sipmaxforwards;

			CacheReleaseLocks(callCache);

			return SipHandleBridgeReInvite(evb);
		}
	}

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	evb->callDetails.responseCode = callHandle->responseCode = 
		appMsgHandle->responseCode = msgHandle->responseCode;

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	evb->callDetails.callError = callHandle->callError = 
		SipDetermineCallError(appMsgHandle->responseCode);

	if (evb->callDetails.flags & REDIRECT)
	{
		// special processing
		evb->callDetails.lastEvent = callHandle->lastEvent = SCC_evtSip3xxRx;
	}
	else
	{
		evb->callDetails.lastEvent = callHandle->lastEvent;
	}

	if (sipCallHandle->pendBInvite)
	{
		pendEvb = sipCallHandle->pendBInvite;

		sipCallHandle->pendBInvite = NULL;
		sipCallHandle->uaflags &= ~UAF_PENDBINVITE;
	}

	// get rid of timer C
	SipStopTimerC(callHandle);

    if(sipHuntError(appMsgHandle->responseCode))
    {
        evb->callDetails.flags |= HUNT_TRIGGER;
    }


	if ((msgHandle->responseCode == 481) ||
		(msgHandle->responseCode == 408))
	{
		rc = SipCloseCall(callHandle, 1);
	}
	else
	{
		if ((msgHandle->responseCode != 401) && 
			(msgHandle->responseCode != 407))
		{
			rc = SipCloseCall(callHandle, 0);
		}
	}

	if (!rc)
	{
		if ((sipCallHandle->uaflags & UAF_LOCALINVITE) ||
			(sipCallHandle->successfulInvites == 1))
		{
			NETDEBUG(MSIP, NETLOG_DEBUG1,
				("%s %d Received for Session Timer Invite!\n",
				fn, appMsgHandle->responseCode));

			// We should disconnect the call ?? -
			// Only for 401/481. - If problems
			// are found, move this code up
			SipStopSessionTimer(callHandle);
			SipStartSessionTimer(callHandle);
		}

		if (sipCallHandle->uaflags & UAF_LOCALINVITE)
		{
			localInvite = 1;
			sipCallHandle->uaflags &= ~UAF_LOCALINVITE;
		}
	}

	/* local SDP must be initialized. 
	 * But that is already in the event handle
	 */ 

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	if (pendEvb)
	{
		if (!rc)
		{
			// Event must be immediately scheduled
			SipUAProcessEventWorker(pendEvb);	
		}
		else
		{
			// free the pending event block
			SipFreeEventHandle(pendEvb);
		}
	}

	if (localInvite)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Response was for local Invite, holding in UA\n",
			fn));
		SipFreeEventHandle(evb);
		return 1;
	}
	else
	{
		/* Send the event to bridge */
		return SipUASendToBridge(evb);
	}

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

void
SipStopSessionTimer(CallHandle *callHandle)
{
	char fn[] = "SipStopSessionTimer():";
	SipCallHandle 	*sipCallHandle = NULL;
	void *timerdata = NULL;

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->sessionTimer)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Attempting to stop session timer\n", fn));

		callHandle->sessionTimerActive = 0;

		if (timerDeleteFromList (&localConfig.timerPrivate, 
				sipCallHandle->sessionTimer, &timerdata))
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s session timer stopped\n", fn));

			// Free the event pointer
			SipFreeEventHandle((SipEventHandle *)timerdata);
			sipCallHandle->sessionTimer = 0;
		}
	}
}

int
SipStartSessionTimer(CallHandle *callHandle)
{
	char fn[] = "SipStartSessionTimer():";
	struct itimerval sessionTimer;
	SipEventHandle *evHandle = 0;
	SipAppMsgHandle *appMsgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	memset(&sessionTimer, 0, sizeof(struct itimerval));

	appMsgHandle = SipAllocAppMsgHandle();
	evHandle = SipAllocEventHandle();

	evHandle->handle = appMsgHandle;
	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);
	appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);

	// half of session expiry period.
	sessionTimer.it_value.tv_sec = callHandle->sessionExpires / 2;

	// Adjust the offset if necessary
	if (callHandle->callSource)
	{
//		sessionTimer.it_value.tv_sec += 15;
	}

	callHandle->sessionTimerActive = 1;
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->sessionTimer = timerAddToList(&localConfig.timerPrivate, 
							&sessionTimer, 0,PSOS_TIMER_REL, "sessTimer",
							SipHandleSessionTimer, evHandle);

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s started the session timer\n", fn));

	return 1;
}

int
SipStart491Timer(int callSource, void *data)
{
    struct itimerval timer491;
	double			randwait;

	memset(&timer491, 0, sizeof(struct itimerval));

	// If we are the origin, wait between [2.4-4]
	// else [0-2].
	if (callSource)
	{
		randwait = 1.6*drand48();
	}
	else
	{
		randwait = 2.0*drand48();
	}

	timer491.it_value.tv_sec = randwait;
	timer491.it_value.tv_usec = 
			(randwait - (double)timer491.it_value.tv_sec)*1000000;

	// Adjust the offset if necessary
	if (callSource)
	{
		timer491.it_value.tv_sec += 2;
	}

	timerAddToList(&localConfig.timerPrivate, 
		&timer491, 0,PSOS_TIMER_REL, "timer491", 
		SipHandle491Timer, data);

	return 1;
}

int
SipHandleNetwork422 (SipEventHandle *evb)
{
	char fn[] = "SipHandleNetwork422()";
	SipAppMsgHandle *appMsgHandle= NULL;
	SipMsgHandle *msgHandle = NULL;

	CallHandle *callHandle = NULL;

	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s entering\n", fn));

	if (evb == NULL)
	{
		NETERROR (MSIP, ("%s NULL event handle\n", fn));
		return -1;
	}


	appMsgHandle  = SipEventAppHandle (evb);
	msgHandle = SipEventMsgHandle (evb);

	CacheGetLocks (callCache, LOCK_WRITE, LOCK_BLOCK);
	callHandle = CacheGet (callCache, appMsgHandle->callID);
	if (callHandle == NULL) 
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4,
			  ("%s call handle not found", fn));
		goto _error;
	}
	SipPrintState (callHandle);
	appMsgHandle->flags |= SIPAPP_USELOCALSDP;
	appMsgHandle->maxForwards = sipmaxforwards;

	callHandle->minSE = callHandle->sessionExpires = 
		appMsgHandle->sessionExpires = appMsgHandle->minSE;
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s setting se to %d minse to %d",
					fn, callHandle->sessionExpires, callHandle->minSE));

	evb->event = Sip_eBridgeInvite;
	evb->type = Sip_eBridgeEvent;

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks (callCache);
	// Send to UA
	SipUAProcessEvent (evb);
	//	SipFreeEventHandle (evb);
	return 1;
 _error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks (callCache);
	return -1;
}


int 
SipHandleNetwork491(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetwork491():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	// Dup the event handle
	evHandle = SipEventHandleDup(evb);
	appMsgHandle = SipEventAppHandle(evHandle);
	// Use local sdp next time
	appMsgHandle->flags |= SIPAPP_USELOCALSDP;

	// We can essentially just cook up the event block
	// to send the INVITE again
	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	sipCallHandle->uaflags |= UAF_TIMEDINVITE;

	callHandle->lastEvent = SCC_evtSip491Rx;

	SipStart491Timer(callHandle->callSource, evHandle);

	// Dont have to forward/do any tsm activity

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetwork3xx(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetwork3xx():";

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	// Convert event to a final response and force hunting
	evb->event = Sip_eNetworkFinalResponse;
	evb->callDetails.flags |= HUNT_TRIGGER | REDIRECT;

	return SipHandleNetworkFinalResponse(evb);
}
				 
int
SipNetworkCloseCallEvb(SipEventHandle *evb, int forceDelete)
{
	char fn[] = "SipNetworkCloseCallEvb():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	int rc = 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _error;
	}

	// mark the call as having been billed
	evb->callDetails.callError = callHandle->callError = 
			SipDetermineCallError(appMsgHandle->responseCode);
	evb->callDetails.lastEvent = callHandle->lastEvent;
	evb->callDetails.responseCode = callHandle->responseCode = 
			appMsgHandle->responseCode;

	rc = SipCloseCall(callHandle, forceDelete);

	CacheReleaseLocks(callCache);
	return rc;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipBridgeCloseCallEvb(SipEventHandle *evb, int forceDelete)
{
	char fn[] = "SipBridgeCloseCallEvb():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	int rc = 0;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _error;
	}

	GkCallSetRemoteErrorCodes(callHandle, &evb->callDetails);
	memcpy(&callHandle->callDetails2, &evb->callDetails, 
		sizeof(CallDetails));

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	rc = SipCloseCall(callHandle, forceDelete);

	CacheReleaseLocks(callCache);
	return rc;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

// Release resources associated with the sip call
// also see if we can delete the call
// Will return 1 if the call was deleted
// and 0 if it wasnt, -1 for error
int
SipCloseCall(CallHandle *callHandle, int forceDelete)
{
	char fn[] = "SipCloseCall():";
	SipCallHandle 	*sipCallHandle = NULL;
	int rc = 0;
	char	callID[CALL_ID_LEN];
	char	callID2[CALL_ID_LEN];
	CallHandle *callHandle2;
	int freePort = 1;

	NETDEBUG(MSIP, NETLOG_DEBUG4, 
		("%s Entering - callID %s\n", 
		fn, CallID2String(callHandle->callID, callID)));

	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null call handle\n", fn));
		return -1;
	}

	sipCallHandle = SipCallHandle(callHandle);

	SipPrintState(callHandle);

	if (CallFceBundleId(callHandle) != 0)
	{
		FCECloseBundle(CallFceSession(callHandle), CallFceBundleId(callHandle));
		CallFceBundleId(callHandle) = 0;
	}

	if (callHandle->flags & FL_CALL_TAP)
	{
		CallTap(callHandle, 0);
	}

	// get rid of the timer
	SipStopTimerC(callHandle);
	SipStopSessionTimer(callHandle);

	SCMCALL_Delete(callHandle);

	if (forceDelete ||
		(sipCallHandle->successfulInvites == 0))
	{
		rc = 1;

		getPeerCallID(callHandle->confID, callHandle->callID, callID2);

		callHandle2 = CacheGet(callCache, callID2);

		if(callHandle2 && !callHandle2->callSource && (callHandle2->remotecontact_list || callHandle->lastEvent == SCC_evtSip3xxRx))
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
		GisDeleteCallFromConf(callHandle->callID, callHandle->confID);

		CallDelete(callCache, callHandle->callID);
		CacheDelete(sipCallCache, &SipCallHandle(callHandle)->callLeg);

		// Timestamp the call
		timedef_cur(&callHandle->callEndTime);

		BillCall(callHandle, CDR_CALLDROPPED);

		/* Delete the call Handle */
		GisFreeCallHandle(callHandle);
	}

	return rc;

_error:
	return -1;
}

int
SipBridgeErrorCancel(SipEventHandle *evb)
{
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	evHandle = SipEventHandleDup(evb);

	SipSendNetworkCancel(evHandle);

	evHandle = SipEventHandleDup(evb);

	SipTerminateTrans(evHandle);

	SipBridgeCloseCallEvb(evb, 1);

	SipFreeEventHandle(evb);

	return 1;
}

int
SipSendNetworkCancel(SipEventHandle *evb)
{
	char fn[] = "SipSendNetworkCancel():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	
	// Generate a Msg on the Network Side
	// We dont have to allocate any new handles,
	// just keep using whatever was passed to us.

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"CANCEL", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	//SipEventAppHandle(evb) = NULL;
	//SipFreeEventHandle(evb);

	//return SipTransSendMsgHandle(appMsgHandle)?-1:1;
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipBridgeErrorBye(SipEventHandle *evb)
{
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	evHandle = SipEventHandleDup(evb);
	SipSendNetworkBye(evHandle);

	SipBridgeCloseCallEvb(evb, 1);

	SipFreeEventHandle(evb);

	return 1;
}

int
SipSendNetworkBye(SipEventHandle *evb)
{
	char fn[] = "SipSendNetworkBye():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	
	// Generate a Msg on the Network Side
	// We dont have to allocate any new handles,
	// just keep using whatever was passed to us.

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	/* Incerement the cseq no */
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->localCSeqNo ++;
	appMsgHandle->maxForwards = sipmaxforwards;

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"BYE", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _error;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	//SipEventAppHandle(evb) = NULL;
	//SipFreeEventHandle(evb);

	//return SipTransSendMsgHandle(appMsgHandle)?-1:1;
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipBridgeError408(SipEventHandle *evb)
{
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	appMsgHandle = SipEventAppHandle(evb);
	
	if (appMsgHandle && appMsgHandle->responseCode == 0)
	{
		appMsgHandle->responseCode = 408;
	}

	return SipSimulateBridgeFinalResponse(evb, 1);
}

int
SipBridgeError500(SipEventHandle *evb)
{
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipEventHandle *evHandle;

	evHandle = SipEventHandleDup(evb);

	if((appMsgHandle = SipEventAppHandle(evb)))
	{
		appMsgHandle->responseCode = 500;
	}

	SipSimulateBridgeFinalResponse(evb, 1);

	return SipHandleBridgeBye(evHandle);
}

int
SipSendNetwork500(SipEventHandle *evb)
{
	return SipSendNetworkFinalResponse(evb, 500);
}

int
SipSendNetworkFinalResponse(SipEventHandle *evb, int response)
{
	char fn[] = "SipSendNetworkFinalResponse():";
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	// Generate a Msg on the Network Side
	// We dont have to allocate any new handles,
	// just keep using whatever was passed to us.

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	appMsgHandle->responseCode = response;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	/* We need to generate a tag if we havent already done so */
	SipCreateTags(callHandle);

	callHandle->responseCode = appMsgHandle->responseCode;

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"INVITE", SIPMSG_RESPONSE, 
						appMsgHandle->responseCode)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	//SipEventAppHandle(evb) = NULL;
	//SipFreeEventHandle(evb);

	//return SipTransSendMsgHandle(appMsgHandle)?-1:1;
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleNetworkError500(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkError500():";
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	/*
	 * We must generate a 500 on the network side also
	 */
	evHandle = SipEventHandleDup(evb);

	appMsgHandle = SipEventAppHandle(evHandle);
	SipSendNetworkFinalResponse(evHandle, 500);

	/* Delete the call */
	SipNetworkCloseCallEvb(evb, 1);
	
	evb->event = Sip_eBridgeCancel;

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	/* Send the event to bridge */
	return SipUASendToBridge(evb);
}

int
SipHandleNetworkErrorCancel(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkErrorCancel():";
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));
#ifdef _SEND_CANCEL_HERE
	evHandle = SipEventHandleDup(evb);

	appMsgHandle = SipEventAppHandle(evHandle);
	SipSendNetworkCancel(evHandle);
#endif
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	// Terminate the transaction
	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	// Check to send a networkBye
	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->callLeg.remote->tag)
	{
		// Issue a BYE out on both sides
		evHandle = SipEventHandleDup(evb);
		SipSendNetworkBye(evHandle);

		evb->event = Sip_eBridgeBye;
	}
	else
	{
		// No need to send Cancel out
		// A timed out request is equivalent to a 503
		// final response
		evb->event = Sip_eBridgeFinalResponse;
		appMsgHandle = SipEventAppHandle(evb);
		appMsgHandle->responseCode = 503;
	}

	// Will be used only when successful invites was 0,
	// But may have applications in future
	evb->callDetails.flags |= HUNT_TRIGGER;

	/* Delete the call */
	SipNetworkCloseCallEvb(evb, 1);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	CacheReleaseLocks(callCache);

	/* Send the event to bridge */
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleNetworkErrorBye(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkErrorBye():";
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	evHandle = SipEventHandleDup(evb);
	SipSendNetworkBye(evHandle);

	appMsgHandle = SipEventAppHandle(evb);

	/* Delete the call */
	SipNetworkCloseCallEvb(evb, 1);
	
	evb->event = Sip_eBridgeBye;

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	/* Send the event to bridge */
	return SipUASendToBridge(evb);
}

int
SipHandleNetworkNoResponseError(SipEventHandle *evb)
{
	char fn[] = "SipHandleNetworkNoResponseError():";
	SipEventHandle *evHandle;
	SipAppMsgHandle *appMsgHandle, *inAppMsgHandle;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	/* Delete the call */
	SipNetworkCloseCallEvb(evb, 1);
	
	evb->event = Sip_eBridgeNoResponseError;
	evb->callDetails.flags |= HUNT_TRIGGER;

	/* Send the event to bridge */
	return SipUASendToBridge(evb);
}

int
SipSendNetwork491(SipEventHandle *evb)
{
	char fn[] = "SipSendNetwork491():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipCallHandle 	*sipCallHandle = NULL;
	int 			finalResponse = 500;
	int 			generateLocalResponse = 0;
	SipEventHandle	*evHandle = NULL;

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s No call handle exists\n", fn));
		goto _error;
	}

	sipCallHandle = SipCallHandle(callHandle);
	
	// Check CSeq
	if (sipCallHandle->remoteCSeqNo >= msgHandle->cseqno)
	{
		// Invalid
		NETERROR(MSIP, ("%s Invalid CSeq number %d against %d\n", 
			fn, msgHandle->cseqno, sipCallHandle->remoteCSeqNo));
		goto _error;
	}

	/* Initialize Cseq numbers */
	sipCallHandle->remoteCSeqNo = msgHandle->cseqno;

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSipReInviteRx;
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	return SipSendNetworkFinalResponse(evb, 491);

_error:
	evHandle = SipEventHandleDup(evb);
	SipSendNetworkFinalResponse(evHandle, finalResponse);

	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;
}

int
SipSendNetwork488(SipEventHandle *evb)
{
	char fn[] = "SipSendNetwork488():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };
	SipCallHandle 	*sipCallHandle = NULL;
	int 			finalResponse = 500;
	int 			generateLocalResponse = 0;
	SipEventHandle	*evHandle = NULL;

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle = SipEventMsgHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s No call handle exists\n", fn));
		goto _error;
	}

	sipCallHandle = SipCallHandle(callHandle);
	
	// Check CSeq
	if (sipCallHandle->remoteCSeqNo >= msgHandle->cseqno)
	{
		// Invalid
		NETERROR(MSIP, ("%s Invalid CSeq number %d against %d\n", 
			fn, msgHandle->cseqno, sipCallHandle->remoteCSeqNo));
		goto _error;
	}

	/* Initialize Cseq numbers */
	sipCallHandle->remoteCSeqNo = msgHandle->cseqno;

	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);

	callHandle->lastEvent = SCC_evtSipReInviteRx;
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	return SipSendNetworkFinalResponse(evb, 488);

_error:
	evHandle = SipEventHandleDup(evb);
	SipSendNetworkFinalResponse(evHandle, finalResponse);

	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;
}

int
SipTerminateTrans2(SipEventHandle *evb, SipCallHandle *sipCallHandle, char *method)
{
	char fn[] = "SipTerminateTrans2():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	
	// Generate a Msg on the Network Side
	// We dont have to allocate any new handles,
	// just keep using whatever was passed to us.

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	appMsgHandle->csmError = csmError_KILL_TRAN;

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle2(sipCallHandle,
						method, 
						SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int
SipTerminateTrans(SipEventHandle *evb)
{
	char fn[] = "SipTerminateTrans():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	
	// Generate a Msg on the Network Side
	// We dont have to allocate any new handles,
	// just keep using whatever was passed to us.

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	sipCallHandle = SipCallHandle(callHandle);

	appMsgHandle->csmError = csmError_KILL_TRAN;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						"INVITE", 
						(sipCallHandle->inviteOrigin)?SIPMSG_REQUEST:SIPMSG_RESPONSE, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int
SipHandleNetworkError(SipEventHandle *evb)
{
	SipFreeEventHandle(evb);

	return -1;
}

int
SipHandleBridgeError(SipEventHandle *evb)
{
	SipFreeEventHandle(evb);

	return -1;
}

int
SipHandleNetworkNoOp(SipEventHandle *evb)
{
	SipFreeEventHandle(evb);

	return 1;
}

int
SipHandleBridgeNoOp(SipEventHandle *evb)
{
	SipFreeEventHandle(evb);

	return 1;
}


int
SipDetermineCallError(int responseCode)
{
	int callError;

	switch(responseCode)
	{
	case 300:
	case 301:
	case 302:
		callError = SCC_errorSipRedirect;
		break;

	case 401:
		callError = SCC_errorSipAuthRequired;
		break;

	case 403:
		callError = SCC_errorSipForbidden;
		break;

	case 404:
		callError = SCC_errorDestNoRoute;
		break;

	case 407:
		callError = SCC_errorSipProxyAuthRequired;
		break;

	case 408:
		callError = SCC_errorDestinationUnreachable;
		break;

	case 480:
		callError = SCC_errorTemporarilyUnavailable;
		break;

	case 481:
		callError = SCC_errorNoCallHandle;
		break;

	case 486:
		callError = SCC_errorBusy;
		break;

	case 488:
		callError = SCC_errorDestinationUnreachable;
		break;

	case 500:
		callError = SCC_errorSipInternalError;
		break;

	case 501:
		callError = SCC_errorSipNotImplemented;
		break;

	case 503:
		callError = SCC_errorSipServiceUnavailable;
		break;

	case 504:
		callError = SCC_errorDestinationUnreachable;
		break;

	case 600:
		callError = SCC_errorBusy;
		break;

	default:
		callError = SCC_errorUndefinedReason;
		break;
	}

	return(callError);
}


int
SipHandleSessionTimer(struct Timer *t)
{
	char fn[] = "SipHandleSessionTimer():";
	SipEventHandle *evb = (SipEventHandle *)(t->data);
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// Free up the timer handle, we already have the data
	timerFreeHandle(t);

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		goto _error;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s Refresher : %d", fn, callHandle->refresher));

	sipCallHandle = SipCallHandle(callHandle);
	SipPrintState(callHandle);

	sipCallHandle->sessionTimer = 0;

	appMsgHandle->sdpVersion = sipCallHandle->rsdpVersion;

	// Dont have to forward/do any tsm activity

	CacheReleaseLocks(callCache);

	// set the event to a bridge re-invite
	evb->event = Sip_eBridgeSessionExpired;
	evb->type = Sip_eBridgeEvent;
	
	// Event must be immediately scheduled
	return SipUAProcessEventWorker(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipBridgeRestartSessionTimer(SipEventHandle *evb)
{
	char fn[] = "SipBridgeRestartSessionTimer():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// Check to see if the call still exists,
	// and if it does, restart the timer

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	SipStartSessionTimer(callHandle);

	// Dont have to forward/do any tsm activity

	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);

	return 1;

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeSessionExpired(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeSessionExpired():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipEventHandle 	*evHandle;

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		goto _error;
	}

	// Send a Re-Invite
	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks (callCache, LOCK_WRITE, LOCK_BLOCK);
	callHandle = CacheGet (callCache, appMsgHandle->callID);
	if (callHandle == NULL)
	{
		NETDEBUG (MSIP, NETLOG_DEBUG4, ("%s call handle not found", fn));
		goto _release_locks;
	}
	msgHandle = SipEventMsgHandle(evb);
	appMsgHandle->refresher = callHandle->refresher;
	appMsgHandle->minSE = callHandle->minSE;
	appMsgHandle->sessionExpires = callHandle->sessionExpires;
	CacheReleaseLocks (callCache);

	appMsgHandle->flags |= SIPAPP_USELOCALSDP;
	appMsgHandle->flags |= SIPAPP_LOCALINVITE;
	appMsgHandle->maxForwards = sipmaxforwards;

	// set the event to a bridge re-invite
	evb->event = Sip_eBridgeInvite;
	evb->type = Sip_eBridgeEvent;
	
	// Event must be immediately scheduled
	return SipUAProcessEventWorker(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

// Here we are not sure whether we can just send
// a re-invite or re-start the timer again
// for some reason
int
SipHandle491Timer(struct Timer *t)
{
	char fn[] = "SipHandle491Timer():";
	SipEventHandle *evb = (SipEventHandle *)(t->data);
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// Free up the timer handle, we already have the data
	timerFreeHandle(t);

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		goto _error;
	}

	// set the event to a bridge re-invite
	evb->event = Sip_eBridge491Expired;
	evb->type = Sip_eBridgeEvent;
	
	// Event must be immediately scheduled
	return SipUAProcessEventWorker(evb);

_release_locks:
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipBridgeRestart491Timer(SipEventHandle *evb)
{
	char fn[] = "SipBridgeRestart491Timer():";
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallLegKey 	leg = { 0 };

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// Check to see if the call still exists,
	// and if it does, restart the timer

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	SipPrintState(callHandle);

	SipStart491Timer(callHandle->callSource, evb);

	// Dont have to forward/do any tsm activity

	CacheReleaseLocks(callCache);

	return 1;

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridge491Expired(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridge491Expired():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	// Treat this like a bridge re-invite
	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	// reset the timed invite flag
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->uaflags &= ~UAF_TIMEDINVITE;

	callHandle->lastEvent = SCC_evtSip491TimerExp;
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	// set the event to a bridge re-invite
	evb->event = Sip_eBridgeInvite;
	evb->type = Sip_eBridgeEvent;
	
	// Event must be immediately scheduled
	return SipUAProcessEventWorker(evb);

_release_locks:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return 1;

_error:
	SipFreeEventHandle(evb);
	CacheReleaseLocks(callCache);
	return -1;
}

void
SipStopTimerC(CallHandle *callHandle)
{
	char fn[] = "SipStopTimerC():";
	SipCallHandle 	*sipCallHandle = NULL;
	SipEventHandle *timerdata = NULL;

	sipCallHandle = SipCallHandle(callHandle);

	if (sipCallHandle->timerC)
	{
		if (timerDeleteFromList (&localConfig.timerPrivate, 
				sipCallHandle->timerC, (void*)&timerdata))
		{
			// Free the event pointer
			SipFreeEventHandle(timerdata);
			sipCallHandle->timerC = 0;
			if (callHandle->lastEvent == SCC_evtSipAuthReqRx)
			{
				callHandle->lastEvent = SCC_evtSipAuthReqTmout;
			}
		}
	}
}

int
SipStartTimerC(CallHandle *callHandle)
{
	char fn[] = "SipStartTimerC():";
    struct itimerval timerC;
	SipEventHandle *evHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	if(siptimerC == 0)
	{
		return 1;
	}

	memset(&timerC, 0, sizeof(struct itimerval));

	appMsgHandle = SipAllocAppMsgHandle();
	evHandle = SipAllocEventHandle();

	evHandle->handle = appMsgHandle;
	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);
	appMsgHandle->maxForwards = sipmaxforwards;

	// 3 mts
	timerC.it_value.tv_sec = siptimerC;

	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->timerC = timerAddToList(&localConfig.timerPrivate, 
							&timerC, 0,PSOS_TIMER_REL, "timerC", 
							SipHandleTimerC, evHandle);

	return 1;
}

int
SipHandleTimerC(struct Timer *timer)
{
	char fn[] = "SipHandleTimerC():";
	SipEventHandle *evb = (SipEventHandle *)(timer->data);
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	// Free up the timer handle, we already have the data
	timerFreeHandle(timer);

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		goto _error;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(callCache, appMsgHandle->callID);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Call Handle not found\n", fn));
		
		goto _release_locks;
	}

	sipCallHandle = SipCallHandle(callHandle);
	SipPrintState(callHandle);

	sipCallHandle->timerC = 0;

	CacheReleaseLocks(callCache);

	// set the event to a bridge re-invite
	evb->event = Sip_eBridgeCExpired;
	evb->type = Sip_eBridgeEvent;
	
	// Event must be immediately scheduled
	return SipUAProcessEventWorker(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipHandleBridgeCExpired(SipEventHandle *evb)
{
	char fn[] = "SipHandleBridgeCExpired():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipEventHandle 	*evHandle;
	int				send_cancel;

	// See if the call exists, and if it does, send
	// the event in as a re-invite from the bridge
	// We check to see if the call exists, so that
	// we dont end up creating a new call
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		goto _error;
	}

	// Treat this like a network error is treated
	// However, we will generate a final response with
	// code of 504
	// We must also terminate the transaction, 
	// send a cancel out

	appMsgHandle = SipEventAppHandle(evb);
	appMsgHandle->responseCode = 504;

	evb->callDetails.flags |= HUNT_TRIGGER;

	// set the event to a bridge re-invite
	evb->event = Sip_eBridgeFinalResponse;
	evb->type = Sip_eBridgeEvent;
	
	// Terminate the transaction
	evHandle = SipEventHandleDup(evb);
	SipTerminateTrans(evHandle);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if((callHandle = CacheGet(callCache, appMsgHandle->callID)))
	{
		sipCallHandle = SipCallHandle(callHandle);
		if(sipCallHandle->successfulInvites == 0)
		{
			send_cancel = 1;
		}
		else
		{
			send_cancel = 0;
		}
	}

	CacheReleaseLocks(callCache);

	evHandle = SipEventHandleDup(evb);
	if(send_cancel)
	{
		// Send a CANCEL out
		SipSendNetworkCancel(evHandle);
	}
	else
	{
		// Send a BYE out
		SipSendNetworkBye(evHandle);
	}

	/* Delete the call */
	SipNetworkCloseCallEvb(evb, 1);

	// Event must be immediately scheduled
	return SipUASendToBridge(evb);

_release_locks:
	SipFreeEventHandle(evb);
	return 1;

_error:
	SipFreeEventHandle(evb);
	return -1;
}

int 
SipBridgeError504(SipEventHandle *evb)
{
	char fn[] = "SipBridgeError504():";
	SipAppMsgHandle *appMsgHandle = NULL;
	SipMsgHandle	*msgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	SipCallHandle 	*sipCallHandle = NULL;
	SipEventHandle 	*evHandle;

	appMsgHandle = SipEventAppHandle(evb);

	if (appMsgHandle && appMsgHandle->responseCode == 0)
	{
		appMsgHandle->responseCode = 504;
	}

	return SipSimulateBridgeFinalResponse(evb, 1);
}

int
SipHandleNetworkRefer(SipEventHandle* evb)
{

	char*            fn  = "SipHandleNetworkRefer():";
	SipAppMsgHandle* appMsgHandle = NULL;
	SipMsgHandle*    msgHandle = NULL;
	CallHandle*      callHandle = NULL;
	SipCallLegKey    leg = { 0 } ;
	SipEventHandle*  evHandle;
	SipCallHandle*   sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle    = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local  = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle    = CacheGet(sipCallCache, &leg);
	sipCallHandle = SipCallHandle(callHandle);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Call Handle not found\n", fn));		
		goto _error;
	}
	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	/* Update Cseq number */
	callHandle->handle.sipCallHandle.remoteCSeqNo = msgHandle->cseqno;
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;
	
	evb->event = Sip_eBridgeRefer;
	evb->type  = Sip_eBridgeEvent;

	/* Send the event to bridge */
	return SipUASendToBridge(evb);
	
 _error:
	CacheReleaseLocks(callCache);
	return -1;
}
int
SipHandleBridgeRefer(SipEventHandle* evb)
{
	char*            fn            = "SipHandleBridgeRefer():";
	SipAppMsgHandle* appMsgHandle  = NULL;
	SipMsgHandle*    msgHandle     = NULL;
	CallHandle*      callHandle    = NULL;
	SipCallHandle*   sipCallHandle = NULL;
	header_url*      tmp_referto   = NULL;
	header_url*      tmp_referby   = NULL;
	char 		peercallid [CALL_ID_LEN];
	SipCallLegKey   callleg = {0};
	char*           ptr     = NULL;
	char*           begin    = NULL;
	char*           temp     = NULL;
	char            buffer[CALL_ID_LEN];
	SipCallHandle*  shandle;
	CallHandle*     chandle;
	CallHandle*     chandle2;
	char	callID2[CALL_ID_LEN];
	char            buf[1024];
	char callid[CALL_ID_LEN],fromtag[CALL_ID_LEN],totag[CALL_ID_LEN];
	header_url lc = {0};
	header_url rm = {0};

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	callHandle->lastEvent = SCC_evtSipReferTx;
	sipCallHandle = SipCallHandle(callHandle);
	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if(msgHandle && msgHandle->referto)
	{
		tmp_referto = UrlDup(msgHandle->referto,MEM_LOCAL);
	}
	if(msgHandle && msgHandle->referby)
	{
		tmp_referby = UrlDup(msgHandle->referby,MEM_LOCAL);
	}

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}

	/* Incerement the cseq no */
	sipCallHandle = SipCallHandle(callHandle);
	sipCallHandle->localCSeqNo ++;	
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,"REFER", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn));
		goto _release_locks;
	}

	msgHandle->referto = tmp_referto;
	msgHandle->referby = tmp_referby;


	// Replaces header treatment in Refer-To header.
	if(msgHandle->referto->header && strlen(msgHandle->referto->header))
	{

		bzero(buffer,CALL_ID_LEN);
		bzero(buf,1024);

		// Preprocess the header

		if(processreplaces(msgHandle->referto->header,buf) < 0 )
		{
			begin = msgHandle->referto->header;
		}
		else 
		{
			begin = buf;
		}

		bzero(buffer,CALL_ID_LEN);
		ptr = strtok_r(begin,";",&temp);
		while(ptr != NULL)
		{			
			strcpy(buffer,ptr);
			begin = ptr;
			parsereplaces(buffer,callid,fromtag,totag);			
			ptr = strtok_r(NULL,";",&temp);
			bzero(buffer,CALL_ID_LEN);
		}
		strcpy(buffer,begin);
		parsereplaces(buffer,callid,fromtag,totag);

		lc.tag = fromtag;
		rm.tag = totag;

		NETDEBUG(MSIP,NETLOG_DEBUG4,("%s looking up cid = %s from tag = %s to tag = %s",fn,callid,fromtag,totag));

		if(strlen(callid))
		{
			callleg.callid = callid;
			callleg.local  = &rm;
			callleg.remote = &lc;			
		}


		bzero(buffer,CALL_ID_LEN);

		CacheGetLocks(sipCallCache,LOCK_READ,LOCK_BLOCK);
		chandle = CacheGet(sipCallCache, &callleg);

		// Reform the replaces header.
		if(chandle)
		{

			getPeerCallID(chandle->confID, chandle->callID, callID2);

			chandle2 = CacheGet(callCache, callID2);
			
			if(chandle2) 
			{
				sprintf(buffer,"Replaces=");					
				shandle = SipCallHandle(chandle2);
				strcat(buffer,shandle->callLeg.callid);
				if(shandle->callLeg.local->tag && strlen(shandle->callLeg.local->tag))
				{
					strcat(buffer,"%3Bfrom-tag%3D");
					strcat(buffer,shandle->callLeg.local->tag);
				}
				if(shandle->callLeg.remote->tag && strlen(shandle->callLeg.remote->tag))
				{
					strcat(buffer,"%3Bto-tag%3D");
					strcat(buffer,shandle->callLeg.remote->tag);						 
				}							
				SipCheckFree(msgHandle->referto->header);
				msgHandle->referto->header = strdup(buffer);
			}
		}
		else
		{
			NETDEBUG(MSIP,NETLOG_DEBUG4,("%s Unable to lookup call referenced in Replaces header",fn));
		}
		
		CacheReleaseLocks(sipCallCache);	

	}
	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SCMCALL_Replicate(callHandle);

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

 _release_locks:
	CacheReleaseLocks(callCache);
	return 1;

 _error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetwork202(SipEventHandle* evb)
{
	char*            fn  = "SipHandleNetwork202():";
	SipAppMsgHandle* appMsgHandle = NULL;
	SipMsgHandle*    msgHandle = NULL;
	CallHandle*      callHandle = NULL;
	SipCallLegKey    leg = { 0 } ;
	SipEventHandle*  evHandle;
	SipCallHandle*   sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle    = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local  = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle    = CacheGet(sipCallCache, &leg);
	sipCallHandle = SipCallHandle(callHandle);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Call Handle not found\n", fn));		
		goto _error;
	}
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	evb->event = Sip_eBridge202;
	evb->type  = Sip_eBridgeEvent;
	
	return SipUASendToBridge(evb);

 _error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridge202(SipEventHandle* evb)
{
	char*             fn            = "SipHandleBridge202():";
	SipAppMsgHandle*  appMsgHandle  = NULL;
	SipMsgHandle*     msgHandle     = NULL;
	CallHandle*       callHandle    = NULL;
	SipCallHandle*    sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}

	callHandle->lastEvent = SCC_evtSip202Rx;
	sipCallHandle = SipCallHandle(callHandle);

	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						  "REFER", SIPMSG_RESPONSE, 202)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}


	SipEventMsgHandle(evb) = msgHandle;


	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo, 
									  MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;

}

int
SipHandleNetworkNotify(SipEventHandle* evb)
{
	char*            fn  = "SipHandleNetworkNotify():";
	SipAppMsgHandle* appMsgHandle = NULL;
	SipMsgHandle*    msgHandle = NULL;
	CallHandle*      callHandle = NULL;
	SipCallLegKey    leg = { 0 } ;
	SipEventHandle*  evHandle;
	SipCallHandle 	*sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle    = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local  = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = CacheGet(sipCallCache, &leg);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Call Handle not found\n", fn));		
		goto _error;
	}

	sipCallHandle = SipCallHandle(callHandle);
	memcpy(appMsgHandle->confID, callHandle->confID, CONF_ID_LEN);
	/* Update Cseq number */
	callHandle->handle.sipCallHandle.remoteCSeqNo = msgHandle->cseqno;
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;
	
	//evb->event = Sip_eBridgeNotify;
	//evb->type  = Sip_eBridgeEvent;

	/* Send the event to bridge */
	return SipUASendToBridge(evb);
	
 _error:
	CacheReleaseLocks(callCache);
	return -1;
}


int
SipHandleBridgeNotify(SipEventHandle* evb)
{
	
	char*            fn            = "SipHandleBridgeNotify():";
	SipAppMsgHandle* appMsgHandle  = NULL;
	SipMsgHandle*    msgHandle     = NULL;
	CallHandle*      callHandle    = NULL;
	header_url*      tmp_referto   = NULL;
	header_url*      tmp_referby   = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}
	//callHandle->lastEvent = SCC_evtSipReferTx;
	callHandle->lastEvent = SCC_evtSipNotifyTx;
	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);


	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						  "NOTIFY", SIPMSG_REQUEST, 0)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
			goto _release_locks;
	}

	SipEventMsgHandle(evb) = msgHandle;

	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *)RealmInfoDup (callHandle->realmInfo, MEM_LOCAL);
	}

	SCMCALL_Replicate(callHandle);

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

 _release_locks:
	CacheReleaseLocks(callCache);
	return 1;

 _error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleNetworkNotifyResponse(SipEventHandle* evb)
{

	char*            fn  = "SipHandleNetworkNotifyResponse():";
	SipAppMsgHandle* appMsgHandle = NULL;
	SipMsgHandle*    msgHandle = NULL;
	CallHandle*      callHandle = NULL;
	SipCallLegKey    leg = { 0 } ;
	SipEventHandle*  evHandle;
	SipCallHandle*   sipCallHandle = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	/* Initialize the event handle */
	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);
	msgHandle    = SipEventMsgHandle(evb);

	leg.callid = msgHandle->callid;
	leg.local  = msgHandle->local;
	leg.remote = msgHandle->remote;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle    = CacheGet(sipCallCache, &leg);
	sipCallHandle = SipCallHandle(callHandle);

	if (callHandle == NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Call Handle not found\n", fn));		
		goto _error;
	}
	SipPrintState(callHandle);

	CacheReleaseLocks(callCache);

	appMsgHandle->natip = 0;
	appMsgHandle->natport = 0;

	evb->event = Sip_eBridgeNotifyResponse;
	evb->type  = Sip_eBridgeEvent;
	
	return SipUASendToBridge(evb);

 _error:
	CacheReleaseLocks(callCache);
	return -1;
}

int 
SipHandleBridgeNotifyResponse(SipEventHandle* evb)
{
	char*             fn            = "SipHandleBridgeNotifyResponse():";
	SipAppMsgHandle*  appMsgHandle  = NULL;
	SipMsgHandle*     msgHandle     = NULL;
	CallHandle*       callHandle    = NULL;

	NETDEBUG(MSIP, NETLOG_DEBUG4, ("%s Entering\n", fn));

	if (evb == NULL)
	{
		NETERROR(MSIP, ("%s Null event handle\n", fn));
		return -1;
	}

	appMsgHandle = SipEventAppHandle(evb);

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	if ((callHandle = SipBridgeUpdateCallHandle(evb)) == NULL)
	{
		/* Cannot allow this call to go through */
		NETERROR(MSIP, ("%s SipBridgeUpdateCallHandle failed\n", fn));
		goto _release_locks;
	}

	// store nat-ip/ nat-port if they exist in the app msg handle
	if(callHandle->natip && callHandle->natport)
	{
		appMsgHandle->natip = callHandle->natip;
		appMsgHandle->natport = callHandle->natport;
	}

	callHandle->lastEvent = SCC_evtSip200Rx;
	SipPrintState(callHandle);

	/* We have a valid callHandle, now we
	 * need to initialize the message handle so we can
	 * send the message out.
	 */
	msgHandle = SipEventMsgHandle(evb);

	if (msgHandle != NULL)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,("%s Freeing previous message handle\n", fn));

		SipFreeSipMsgHandle(msgHandle);
		SipEventMsgHandle(evb) = NULL;
	}
		
	if ((msgHandle = SipBridgeCreateMsgHandle(callHandle,
						  "NOTIFY", SIPMSG_RESPONSE, 200)) == NULL)
	{
		NETERROR(MSIP, ("%s SipBridgeCreateMsgHandle failed\n", fn))
		goto _release_locks;
	}


	SipEventMsgHandle(evb) = msgHandle;


	if (!appMsgHandle->realmInfo)
	{
		appMsgHandle->realmInfo = (CallRealmInfo *) RealmInfoDup (callHandle->realmInfo,MEM_LOCAL);
	}

	CacheReleaseLocks(callCache);

	/* Send the message using the TSM */
	return SipUASendToTSM(evb);

_release_locks:
	CacheReleaseLocks(callCache);
	return 1;

_error:
	CacheReleaseLocks(callCache);
	return -1;
}

void
parsereplaces(char* buffer, char* callid, char* fromtag, char* totag)
{
 
	char* fn  = "parsereplces";
	char* ptr1 = NULL;
	char* ptr2 = NULL;

	NETDEBUG(MSIP,NETLOG_DEBUG4, ("%s entering ",fn));

	if(strncmp(buffer,"Replaces",strlen("Replaces")) == 0 )
	{
		if( (ptr1 = strstr(buffer,"=")) != NULL)
		{
			if ( (ptr1+strlen("=")) < (buffer+strlen(buffer)) )
			{
				ptr1 += strlen("=");
				strncpy(callid, ptr1,CALL_ID_LEN);
				NETDEBUG(MSIP,NETLOG_DEBUG4, ("%s callid is %s",fn,callid));
			}
		}		
	}
	else if(strncmp(buffer,"from-tag",strlen("from-tag"))==0)
	{
		if( (ptr1 = strstr(buffer,"=")) != NULL)
		{
			if ( (ptr1+strlen("=")) < (buffer+strlen(buffer)) )
			{
				ptr1 += strlen("=");
				strncpy(fromtag, ptr1,CALL_ID_LEN);
				NETDEBUG(MSIP,NETLOG_DEBUG4, ("%s from_tag is %s",fn,fromtag));
			}
		}		
	}
	else if(strncmp(buffer,"to-tag",strlen("to-tag")) == 0 )
	{
		if( (ptr1 = strstr(buffer,"=")) != NULL)
		{
			if ( (ptr1+strlen("=")) < (buffer+strlen(buffer)) )
			{
				ptr1 += strlen("=");
				strncpy(totag, ptr1,CALL_ID_LEN);
				NETDEBUG(MSIP,NETLOG_DEBUG4, ("%s to_tag is %s",fn,totag));
			}
		}
	}

	return;
}

int
processreplaces(char* incoming_replaces, char* buffer)
{

	char* ptr   = NULL;
	char* begin = NULL;
	int   len;
	char  tgt[1024];
	char* ptr1 = NULL;

	if( !tgt || !buffer)
	{
		return -1;
	}
	
	strncpy(tgt,incoming_replaces,1024);
	len = strlen(tgt);

	ptr = strtok_r(tgt,"%",&ptr1);

	if(ptr == NULL)
	{
		return -1;
	}

	while(ptr != NULL)
	{		
		strcat(buffer,ptr);
		begin = ptr+strlen(ptr);
		
		begin++;
		ptr = NULL;

		// Ensure that we don't drift out of target string
		if( begin  < (tgt+len))
		{
			if(strncasecmp(begin,"2D",2) == 0 )
			{
				strcat(buffer,"-");
				begin += 2;
			}
			else if(strncasecmp(begin,"2E",2) == 0 )
			{
				strncat(buffer,".",2);
				begin +=2;
			}
			else if(strncasecmp(begin,"3B",2)==0)
			{
				strcat(buffer,";");
				begin += 2;
			}
			else if(strncasecmp(begin,"3D",2)==0)
			{
				strcat(buffer,"=");
				begin += 2;
			}
			else if(strncasecmp(begin,"40",2) == 0 )
			{
				strncat(buffer,"@",2);
				begin +=2;
			}
			else
			{
				strcat(buffer,"%");				
			}
			ptr = strtok_r(begin,"%",&ptr1);
		}
	}	
	return 1;
}

/* 
 * ExtractContactHost 
 *  finds the string between begining of the string OR @
 *  and end of the string or :
 * tested inputs
 * char	contact1[]="100@1.1.1.1:5060";
 * char	contact2[]="@1.1.1.1:5060";
 * char	contact3[]="100@1.1.1.1";
 * char	contact4[]="1.1.1.1:5060";
 * char	contact5[]="1.1.1.1";
 * char	contact6[]="100@www.yahoo.com:"; 
 * outputs 
 * 100@1.1.1.1:5060 has contactip 1.1.1.1
 * @1.1.1.1:5060 has contactip 1.1.1.1
 * 100@1.1.1.1 has contactip 1.1.1.1
 * 1.1.1.1:5060 has contactip 1.1.1.1
 * 1.1.1.1 has contactip 1.1.1.1
 * 100@www.yahoo.com: has contactip www.yahoo.com
 */

int
ExtractContactHost(char *entry_contact, char *contacthostname, int* port)
{
	char tmpcontact[255];
	char *p = NULL, *q=NULL;
	unsigned int  len;	
	char portnum[16];

	if ((entry_contact == NULL) || strlen(entry_contact)==0)
	{
		return 0;
	}

	nx_strlcpy(tmpcontact, entry_contact, sizeof(tmpcontact));
	p = strchr(tmpcontact, '@');

	if (p==NULL)
	{
		p = tmpcontact - 1;
	}

	q = strchr(tmpcontact, ':');
	if (q)
	{
		len = q - p;
		q++;
		if( q < (tmpcontact + strlen(tmpcontact)))
		{
			bzero(portnum,16);
			nx_strlcpy(portnum,q,sizeof(portnum));
			*port = atoi(portnum);
		}
	}
	else
	{
		len = strlen(entry_contact) - (unsigned int)(p - tmpcontact);
	}
	memset(contacthostname, 0, SIPURL_LEN);
	memcpy (contacthostname, p+1, len -1);
	contacthostname[len -1 ] = '\0';

	return 0;
}

