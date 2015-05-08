#include "gis.h"
#include <malloc.h>
#include "ua.h"
#include "bridge.h"
#include "sipcallactions.h"
int
SipUASendToBridge(SipEventHandle *evb)
{
	char fn[] = "SipUASendToBridge():";
	sigset_t sset;
	SipAppMsgHandle *appMsgHandle = NULL;
	CallHandle 		*callHandle = NULL;
	ConfHandle		*confHandle = NULL;
	char callID[CALL_ID_LEN];
	SCC_EventBlock *evPtr;
	int i, shouldPause = 0, sig;

	// Check if this event can be locally handle
	appMsgHandle = SipEventAppHandle(evb);
	SipEventAppHandle(evb) = NULL;

	/* Allocate an SCC Event Block and fill it with data */
	evPtr = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memset(evPtr, 0, sizeof(SCC_EventBlock));

	evPtr->event = evb->event;
	evPtr->data = appMsgHandle;
	memcpy(&evPtr->callDetails, &evb->callDetails, sizeof(CallDetails));

	SipFreeEventHandle(evb);

	memcpy(evPtr->callID, appMsgHandle->callID, CALL_ID_LEN);
	memcpy(evPtr->confID, appMsgHandle->confID, CONF_ID_LEN);

	memcpy(callID, appMsgHandle->callID, CALL_ID_LEN);
	if (bridgeSipEventProcessor(evPtr) != 0)
	{
		NETERROR(MSIP, ("%s bridgeSipEventProcessor error\n", fn));

		// Free the call handle and return an error for now
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		callHandle = CacheGet(callCache, callID);

		if (callHandle == NULL)
		{
			NETDEBUG(MSIP, NETLOG_DEBUG4,
				("%s Call Handle already deleted?\n", fn));
		
			goto _release_locks;
		}

		SipCloseCall(callHandle, 1);

		// We dont know what event to send back to bridge,
		// Error would be most appropriate, but iwf does not
		// handle that event

	_release_locks:
		CacheReleaseLocks(callCache);

		return -1;
	}

	return 1;
}

int
sipBridgeEventProcessor(SCC_EventBlock *evPtr)
{
	char fn[] = "sipBridgeEventProcessor():";
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;

	if (evPtr == NULL)
	{
		NETERROR(MSIP, ("%s evPtr is NULL\n", fn));
		return -1;
	}

	appMsgHandle = (SipAppMsgHandle *)evPtr->data;

	if (!appMsgHandle)
	{
		// Allocate an app msg handle if one is not around
		evPtr->data = appMsgHandle = SipAllocAppMsgHandle();
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s App Msg Data is not present, allocating\n", fn));
	}

	// if Conf ID is not filled up in the ingress leg.. 
	memcpy(appMsgHandle->confID, evPtr->confID, CONF_ID_LEN);

	evHandle = SipAllocEventHandle();

	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = evPtr->event;
	evHandle->handle = appMsgHandle;
	memcpy(&evHandle->callDetails, &evPtr->callDetails, 
			sizeof(CallDetails));

	free(evPtr);

	if (SipValidateSipEventHandle(evHandle) < 0)
	{
		return -1;
	}

	return SipUAProcessEvent(evHandle);		
}

