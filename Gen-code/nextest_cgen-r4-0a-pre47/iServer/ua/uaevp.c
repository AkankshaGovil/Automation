#include "gis.h"
#include <malloc.h>
#include "ua.h"
#include "sipcallactions.h"

extern int idaemon;

int *sipinPool;
int *sipinClass;
extern int nCallIdThreads;

/* UA Event processor code */
int
SipUAProcessEventWorker(SipEventHandle *evHandle)
{
	char fn[] = "SipUAProcessEventWorker():";
	SIP_StateMachineEntry *smEntry = NULL;
	CallHandle *callHandle = NULL;
	SipAppMsgHandle *appMsgHandle = NULL;
	int state = Sip_sIdle;

	appMsgHandle = SipEventAppHandle(evHandle);

	/* See if we can find the call handle corresponding
	 * to the callid mentioned here
	 */
	if (SipUACheckStateForCallID(appMsgHandle->callID, &state) < 0)
	{
		NETERROR(MSIP, ("%s No SIP State found for call handle\n",
			fn));

		return -1;
	}

	if ((state < 0) &&
		(evHandle->event == Sip_eNetworkInvite))
	{
		/* We dont have an application call handle for this call */
		CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

		SipInitAppCallHandleForEvent(evHandle);

		CacheReleaseLocks(callCache);

		state = Sip_sIdle;
	}

	// If state is still undefined, we should quit
	if (state < 0)
	{
		NETDEBUG(MSIP, NETLOG_DEBUG4,
			("%s Event %s with no previous call state\n", fn,
			GetSipEvent(evHandle->type, evHandle->event)));

		SipHandleNetworkError(evHandle);

		return -1;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s State %s Event %s\n", fn, 
			GetSipState(state),
			GetSipEvent(evHandle->type, evHandle->event)));

	/* Based on what kind of event it is we
	 * will trigger the right s/m
	 */
	switch (evHandle->type)
	{
	case Sip_eNetworkEvent:
		smEntry = &SipNetworkSM[state][evHandle->event];
		break;
	case Sip_eBridgeEvent:
		smEntry = &SipBridgeSM[state][evHandle->event];
		break;
	default:
		NETERROR(MSIP, ("%s Invalid Event Type %d\n", 
			fn, evHandle->type));
		break;
	}

	if (smEntry == NULL)
	{
		NETERROR(MSIP, ("%s No State Machine Entry found\n", fn));

		SipHandleNetworkError(evHandle);
		goto _release_locks;
	}

	/* change state */
	SipUAChangeStateForCallID(evHandle, appMsgHandle->callID);

	/* Call the action routine */
	if (smEntry->action)
	{
		if ((smEntry->action)(evHandle) < 0)
		{
			/* Action routine failed */
			NETDEBUG(MSIP, NETLOG_DEBUG1, 
				("%s Action routine failed\n", fn));
			goto _error;
		}
	}
	else
	{
		// No action routine ?? Event must be freed
		SipHandleNetworkError(evHandle);
	}

	return 0;

_release_locks:

_error:
	NETDEBUG(MSIP, NETLOG_DEBUG1, ("%s Returning Error\n", fn));
	return -1;
}

int
SipUAProcessEvent(SipEventHandle *evHandle)
{
	char fn[] = "SipUAProcessEvent():";

	//SipQueueEvent(evHandle, SipUAProcessEventWorker);
	SipQueueEvent(evHandle, (void* (*) (void*))SipUAProcessEventWorker);

	return 0;
}

int
SipUACheckStateForCallID(char *callID, int *state)
{
	CallHandle *callHandle = NULL;

	*state = -1;

	CacheGetLocks(callCache,LOCK_READ,LOCK_BLOCK);
	
	callHandle = CacheGet(callCache, callID);
	if (callHandle)
	{
		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			CacheReleaseLocks(callCache);
			return -1;
		}

		*state = callHandle->state;
	}
	
	CacheReleaseLocks(callCache);
	
	return 1;
}

int
SipUAChangeStateForCallID(SipEventHandle *evHandle, char *callID)
{
	CallHandle *callHandle = NULL;
	int rc = -1;

	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
	
	callHandle = CacheGet(callCache, callID);
	rc = SipUAChangeState(evHandle, callHandle);
	
	CacheReleaseLocks(callCache);
	
	return rc;
}

/* Locks must be acquired around this routine, 
 * as callHandle is unprotected otherwise
 */
int
SipUAChangeState(SipEventHandle *evHandle, CallHandle *callHandle)
{
	char fn[] = "SipUAChangeState():";
	SIP_StateMachineEntry *smEntry = NULL;

	if (callHandle == NULL)
	{
		NETERROR(MSIP, ("%s callHandle is NULL\n", fn));
		return -1;
	}

	/* Based on what kind of event it is we
	 * will trigger the right s/m
	 */
	switch (evHandle->type)
	{
	case Sip_eNetworkEvent:
		smEntry = &SipNetworkSM[callHandle->state][evHandle->event];
		break;
	case Sip_eBridgeEvent:
		smEntry = &SipBridgeSM[callHandle->state][evHandle->event];
		break;
	default:
		NETERROR(MSIP, ("%s Invalid Event Type %d\n", 	
			fn, evHandle->type));
		break;
	}

	if (smEntry == NULL)
	{
		NETERROR(MSIP, ("%s No State Machine Entry found\n", fn));
		goto _error;
	}

	NETDEBUG(MSIP, NETLOG_DEBUG4,
		("%s State %s Event %s New State %s\n", fn, 
			GetSipState(callHandle->state),
			GetSipEvent(evHandle->type, evHandle->event),
			GetSipState(smEntry->nextState)));

	callHandle->state = smEntry->nextState;

	return callHandle->state;

_error:
	return -1;
}

int
SipUAEnd()
{
	int i;

	for (i = 0; i<nCallIdThreads; i++)
	{
		ThreadPoolEnd(sipinPool[i]);
	}

	ThreadPoolEnd(poolid);

	return 0;
}

int
SipUAInit()
{
	int i;

	// Initialize the queue for events coming
	// in for SIP call handles
	sipinPool = (int *)malloc(nCallIdThreads*sizeof(int));
	sipinClass = (int *)malloc(nCallIdThreads*sizeof(int));

	for (i = 0; i<nCallIdThreads; i++)
	{
		sipinPool[i] = ThreadPoolInit("ua", 1, PTHREAD_SCOPE_PROCESS,
							0, 0);
		sipinClass[i] = ThreadAddPoolClass("uaclass", sipinPool[i], 0, 100000000);
		ThreadPoolStart(sipinPool[i]);
	}

	// Initialize the TSM threads
	poolid = ThreadPoolInit("tsm", xthreads, PTHREAD_SCOPE_PROCESS,  0, 0);
	hpcid = ThreadAddPoolClass("tsm-net", poolid, 0, class1deadline);
	lpcid = ThreadAddPoolClass("tsm-sm", poolid, 0, class2deadline);
	ThreadPoolStart(poolid);

	// Also register the callback with the icmp thread
	return(0);
}

int
SipQueueEvent(SipEventHandle *evHandle, void* (*cfn)(void *))
{
	char fn[] = "SipQueueEvent():";
	unsigned short index = 0;
	SipAppMsgHandle *appMsgHandle = NULL;

	appMsgHandle = SipEventAppHandle(evHandle);

	if (appMsgHandle == NULL)
	{
		NETERROR(MSIP, ("%s Null appMsgHandle\n", fn))
		return 0;
	}

	memcpy(&index, appMsgHandle->callID+14, 2);
	index %= nCallIdThreads;

	if (ThreadDispatch(sipinPool[index], sipinClass[0],
			cfn, evHandle,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MSIP, ("%s Error in dispatching\n", fn));
	}

	return 0;
}
