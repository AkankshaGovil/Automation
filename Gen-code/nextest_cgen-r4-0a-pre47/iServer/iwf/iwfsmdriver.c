#include "gis.h"
#include "iwfsm.h"
#include "sipcall.h"
#include "mealeysm.h"
#include "uh323inc.h"
#include "usipinc.h"
#include "serverp.h"
#include <malloc.h>
#include "iwfutils.h"
#include "log.h"
#include "gk.h"
#include "bridge.h"
#include "iwfutils.h"

int *iwfinPool;
int *iwfinClass;
extern int nConfIdThreads;

// For Each sm - Analogous to EventProcessors
void iwfSipChangeState(void *smInstance, SCC_EventBlock *evtptr);

void usipChangeState(void *smInstance, SCC_EventBlock *evtptr);
int usipNetworkSendEvent(SCC_EventBlock *);
int usipBridgeSendEvent(SCC_EventBlock *);

/* 
	Handles Sip Events coming in from Bridge(sip) going out to H323
	Find callHandle
	Create and initialize if not there
	Change state 
	Execute the action routine
*/

int iwfSipEventProcessorWorker(SCC_EventBlock *evtPtr)
{
	static char fn[] = "iwfSipEventProcessorWorker";
	CallHandle 			callHandleBlk;
	ConfHandle			*confHandle;
	char 				str[EVENT_STRLEN];
	SipCallHandle		*pSipData;
	MLSM_StateMachineEntry *sm;
	char				stateString1[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				confIDStr[CONF_ID_LEN];
	int					event;

	if(!evtPtr)
	{
		NETERROR(MIWF,("%s Null EventPtr \n",fn));
		return -1;
	}

	if(!IWF_IsSipEvent(evtPtr->event))
	{
		NETERROR(MIWF,("%s Invalid Event(%s)\n",
			fn,sipEvent2Str(evtPtr->event,str)));
		return -1;
	}

	event = evtPtr->event;

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if(!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		CacheReleaseLocks(confCache);
		NETERROR(MIWF,("%s Event = %s Failed to find ConfId %s\n",
			fn,sipEvent2Str(evtPtr->event,str), 
			(char*) ConfID2String(evtPtr->confID,confIDStr)));
	    SipFreeAppCallHandle(evtPtr->data);
    	free(evtPtr);
		return -1;
	}	

 	// Determinesm
	sm = &IWF_smSip2H323[confHandle->state][evtPtr->event];

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,sipEvent2Str(evtPtr->event,str),
		iwfState2Str(confHandle->state,stateString1),
		iwfState2Str(sm->nextState,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));

	// Change State for conf Handle
	confHandle->state = sm->nextState;
	CacheReleaseLocks(confCache);

	if(sm->action && (sm->action(evtPtr) !=0))
	{
		NETERROR(MIWF,("%s Action routine %p failed.\n",
			fn,sm->action));
		return 0;
	}

	return 0;
}

/* 
	Handles H323 Events coming in from Bridge(H323) going out to Sip 
	Find callHandle
	Create and initialize if not there
	Change state 
	Execute the action routine
*/

int iwfH323EventProcessorWorker(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "iwfH323EventProcessorWorker";
	ConfHandle			*confHandle;
	CallHandle 			callHandleBlk;
	char				callIDStr[CALL_ID_LEN];
	char 				str[EVENT_STRLEN];
	MLSM_StateMachineEntry *sm;
	char				stateString1[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				confIDStr[CONF_ID_LEN];
	char				evtStr[80];
	int					event;

	if(!evtPtr)
	{
		NETERROR(MIWF,("%s Null EventPtr \n",fn));
		return -1;
	}

	event = evtPtr->event;

	evtPtr->event -= SCC_eBridgeEventsMin;
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if(!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		CacheReleaseLocks(confCache);
		NETERROR(MIWF,("%s Event = %s Failed to find ConfId %s\n",
			fn,SCC_EventToStr(event,str),
			(char*) ConfID2String(evtPtr->confID,confIDStr)));
		H323FreeEvData(evtPtr->data);
    	free(evtPtr);
		return -1;
	}	

 	// Determinesm
	sm = &IWF_smH3232Sip[confHandle->state][evtPtr->event];

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
		iwfState2Str(confHandle->state,stateString1),
		iwfState2Str(sm->nextState,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));

	// Change State for conf Handle
	confHandle->state = sm->nextState;
	CacheReleaseLocks(confCache);

	if(sm->action && (sm->action(evtPtr) !=0))
	{
		NETERROR(MIWF,("%s Action routine %p failed.\n",
			fn,sm->action));
		return 0;
	}

	return 0;
}

int iwfSipEventProcessor(SCC_EventBlock *evtPtr)
{
	static char fn[] = "iwfSipEventProcessor";
	CallHandle 			callHandleBlk;
	ConfHandle			*confHandle;
	char 				str[EVENT_STRLEN];
	SipCallHandle		*pSipData;
	MLSM_StateMachineEntry *sm;
	char				stateString1[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				confIDStr[CONF_ID_LEN];
	int					event;

	if(!evtPtr)
	{
		NETERROR(MIWF,("%s Null EventPtr \n",fn));
		return -1;
	}

	if(!IWF_IsSipEvent(evtPtr->event))
	{
		NETERROR(MIWF,("%s Invalid Event(%s)\n",
			fn,sipEvent2Str(evtPtr->event,str)));
		return -1;
	}

	evtPtr->evtProcessor = iwfSipEventProcessorWorker;
	return iwfQueueEvent(evtPtr);

	event = evtPtr->event;

	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if(!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		CacheReleaseLocks(confCache);
		NETERROR(MIWF,("%s. Event = %s.Failed to find ConfId %s\n",
			fn, sipEvent2Str(evtPtr->event,str),
			(char*) ConfID2String(evtPtr->confID,confIDStr)));
		return -1;
	}	

 	// Determinesm
	sm = &IWF_smSip2H323[confHandle->state][evtPtr->event];

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,sipEvent2Str(evtPtr->event,str),
		iwfState2Str(confHandle->state,stateString1),
		iwfState2Str(sm->nextState,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));

	// Change State for conf Handle
	confHandle->state = sm->nextState;
	CacheReleaseLocks(confCache);

	if(sm->action && (sm->action(evtPtr) !=0))
	{
		NETERROR(MIWF,("%s Action routine %p failed.\n",
			fn,sm->action));
		return 0;
	}

	return 0;
}

/* 
	Handles H323 Events coming in from Bridge(H323) going out to Sip 
	Find callHandle
	Create and initialize if not there
	Change state 
	Execute the action routine
*/

int iwfH323EventProcessor(SCC_EventBlock *evtPtr)
{
	static char 		fn[] = "iwfH323EventProcessor";
	ConfHandle			*confHandle;
	CallHandle 			callHandleBlk;
	char				callIDStr[CALL_ID_LEN];
	char 				str[EVENT_STRLEN];
	MLSM_StateMachineEntry *sm;
	char				stateString1[IWF_STATE_STR],stateString2[IWF_STATE_STR];
	char				confIDStr[CONF_ID_LEN];
	char				evtStr[80];
	int					event;

	if(!evtPtr)
	{
		NETERROR(MIWF,("%s Null EventPtr \n",fn));
		return -1;
	}

	evtPtr->evtProcessor = iwfH323EventProcessorWorker;
	return iwfQueueEvent(evtPtr);

	event = evtPtr->event;

	evtPtr->event -= SCC_eBridgeEventsMin;
	CacheGetLocks(confCache,LOCK_READ,LOCK_BLOCK);
	if(!(confHandle = CacheGet(confCache,evtPtr->confID)))
	{
		CacheReleaseLocks(confCache);
		NETERROR(MIWF,("%s Failed to find ConfId %s. confID = %s\n",
			fn,SCC_EventToStr(event,str),
			(char*) ConfID2String(evtPtr->confID,confIDStr)));
		return -1;
	}	

 	// Determinesm
	sm = &IWF_smH3232Sip[confHandle->state][evtPtr->event];

	NETDEBUG(MIWF,NETLOG_DEBUG4, 
		("%s %s/%s--> %s confID = %s\n",
		fn,SCC_BridgeEventToStr(evtPtr->event,evtStr),
		iwfState2Str(confHandle->state,stateString1),
		iwfState2Str(sm->nextState,stateString2),
		(char*) ConfID2String(evtPtr->confID,confIDStr)));

	// Change State for conf Handle
	confHandle->state = sm->nextState;
	CacheReleaseLocks(confCache);

	if(sm->action && (sm->action(evtPtr) !=0))
	{
		NETERROR(MIWF,("%s Action routine %p failed.\n",
			fn,sm->action));
		return 0;
	}

	return 0;
}


/*
*	Provides an easy interface to send events to H323  state machine
*/
int iwfSendH323Event(SCC_EventBlock *evtPtr)
{
	static char	fn[] = "iwfSendH323Event";
	CallHandle 			callHandleBlk = {0};
	char	str[256];

    if(CacheFind(callCache,evtPtr->callID,&callHandleBlk,sizeof(CallHandle)) <0)
    {
		NETERROR(MIWF,("%s CallHandle Not found %s\n",
			fn,sccEventBlockToStr(evtPtr, str)));
		goto _error;
    }

	// Initialize the H.323 stack - no need as the SCC is in it own thread
	// uh323StackLock();

	if(callHandleBlk.bridgeEventProcessor(evtPtr) <0 )
	{
		NETDEBUG(MIWF, NETLOG_DEBUG2,
			("%s bridgeEventProcessor returned error\n",fn));

		uh323StackUnlock();
		goto _error;
	}

	// uh323StackUnlock();

	return 0;

_error:
	H323FreeEvData((H323EventData *) evtPtr->data);
	free(evtPtr);
	return -1;
}

void
iwfEnd()
{
	int i;

	for (i = 0; i<nConfIdThreads; i++)
	{
		ThreadPoolEnd(iwfinPool[i]);
	}
}

int
iwfInit()
{
	int i;

	// Initialize the queue for events coming
	// in for iwf
	iwfinPool = (int *)malloc(nConfIdThreads*sizeof(int));
	iwfinClass = (int *)malloc(nConfIdThreads*sizeof(int));

	for (i = 0; i<nConfIdThreads; i++)
	{
		iwfinPool[i] = ThreadPoolInit("iwf", 1, PTHREAD_SCOPE_PROCESS,
							0, 0);
		iwfinClass[i] = ThreadAddPoolClass("iwfclass", iwfinPool[i], 0, 100000000);
		ThreadPoolStart(iwfinPool[i]);
	}
	return(0);
}

int
iwfQueueEvent(SCC_EventBlock *evtPtr)
{
	char fn[] = "iwfQueueEvent():";

	if (evtPtr->evtProcessor == NULL)
	{
		NETERROR(MIWF, ("%s Error: Null function\n", fn));

		return -1;
	}

	if (ThreadDispatch(iwfinPool[0], iwfinClass[0],
			(PFVP)evtPtr->evtProcessor, evtPtr,
			1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MIWF, ("%s Error in dispatching\n", fn));
	}

	return 0;
}
