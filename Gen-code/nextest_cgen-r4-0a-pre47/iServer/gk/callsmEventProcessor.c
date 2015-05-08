#include "gis.h"
#include "callsm.h"
#include "uh323.h"
#include "uh323cb.h"
#include "sccdefs.h"
#include "uh323inc.h"
#include <malloc.h>
#include "gk.h"
#include "log.h"

int *h323inPool;
int *h323inClass, *h323inhpClass;

extern pthread_t *h323Threads;

// Always frees up eventBlockPtr
int
SCC_EventProcessorWorker(SCC_EventBlock *eventBlockPtr)
{
	char fn[] = "SCC_EventProcessorWorker():";
	char str[256] = {0};

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s Processing event %s\n", 
		fn,sccEventBlockToStr(eventBlockPtr, str)));

#if 0
	if (SCC_IsNetworkEvent(eventBlockPtr->event))
	{
		SCC_DelegateEventWorker(eventBlockPtr);
	}
	else if (SCC_IsBridgeEvent(eventBlockPtr->event))
#endif

	if(SCC_BridgeEventWorker(eventBlockPtr)!=0)
	{
		H323FreeEvent(eventBlockPtr);
	}

	return 0;
}

/* Event Processor  - Shallow copy of eventData */
/* The caller should not try to access or free event data */
int 
SCC_DelegateEvent(SCC_EventBlock *eventBlockPtr)
{
	char fn[] = "SCC_DelegateEvent():";
	SCC_EventBlock *tmpEvt;
	SCC_EventBlock	*pNextEvt;
	H323EventData	*pH323Data;

	// Allocate memory for eventBlockPtr
	pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memcpy(pNextEvt,eventBlockPtr,sizeof(SCC_EventBlock));
#if 0
	pNextEvt->event = eventBlockPtr->event;
	pNextEvt->cause = eventBlockPtr->cause;
	memcpy(pNextEvt->callID,eventBlockPtr->callID,CALL_ID_LEN);
	memcpy(pNextEvt->confID,eventBlockPtr->confID,CONF_ID_LEN);
#endif

#if 1
	if (pNextEvt->evtProcessor != NULL)
	{
		// Set it to the sm processor
		eventBlockPtr->evtProcessor(pNextEvt);
	}
	else if (SCC_DelegateEventWorker(pNextEvt) < 0)
	{
		NETERROR(MSCC, ("%s SCC_DelegateEventWorker returned error\n", fn));
	}
#endif

	//SCC_QueueEvent(pNextEvt);

	return 0;
}

// returns 0 on success negative value on error
// Always free the evBlockPtr
int 
SCC_DelegateEventWorker(SCC_EventBlock *eventBlockPtr)
{
	char fn[] = "SCC_DelegateEventWorker():";
	SCC_EventBlock	*pNextEvt;
    ConfHandle 	confHandleBlk,*confHandle = &confHandleBlk;
    CallHandle callHandleBlk1,callHandleBlk2;
	int peerCall; 
	int	rv;
	char confIdStr[CONF_ID_LEN],callIdStr1[CALL_ID_LEN],callIdStr2[CALL_ID_LEN];
	char str[256] = {0};
	H323EventData	*pH323Data;

	if(!eventBlockPtr)
	{
     	NETERROR(MSCC,("%s Null eventBlock",fn));
		return -1;
    }
	if(CacheFind(callCache,eventBlockPtr->callID,&callHandleBlk1,sizeof(CallHandle)) <0)
	{
			NETERROR(MSCC,("%s CallHandle Not found %s\n",
				fn,sccEventBlockToStr(eventBlockPtr, str)));
			H323FreeEvent(eventBlockPtr);
			return -1;
	}	

#ifdef _delegate_evt_
	// If you define this - the watchout for memory leak!!
	pNextEvt = (SCC_EventBlock *)malloc(sizeof(SCC_EventBlock));
	memcpy(pNextEvt,eventBlockPtr,sizeof(SCC_EventBlock));

	pH323Data = (H323EventData *)malloc(sizeof(H323EventData));
    memset(pH323Data,0,sizeof(H323EventData));

    pNextEvt->data = (void *) pH323Data;
    pNextEvt->event = eventBlockPtr->event;

    memcpy(pH323Data->callID,eventBlockPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->callID,eventBlockPtr->callID,CALL_ID_LEN);
    memcpy(pNextEvt->confID,eventBlockPtr->confID,CONF_ID_LEN);
#else
	pNextEvt = eventBlockPtr;
	pH323Data = (H323EventData *)pNextEvt->data;
#endif

	if(callHandleBlk1.networkEventProcessor)
	{
		rv = (callHandleBlk1.networkEventProcessor)(pNextEvt);
	}
	else {
		NETERROR(MSCC,("%s networkEventProcessor null %s \n",
				fn,sccEventBlockToStr(eventBlockPtr, str)));
		H323FreeEvent(eventBlockPtr);
		return -1;
	}
    return 0;
}



// 0 if OK -1 if error
// Always free inEventPtr
int 
SCC_H323Leg1NetworkEventProcessor(SCC_EventBlock *inEventPtr)
{
    char fn[] = "SCC_Leg1NetworkProcessEvent():";
    CallHandle 	callHandleBlk,*callHandle = &callHandleBlk;
    SCC_StateMachineEntry *sm;
	char str[256] = {0},str2[256] = {0},evtStr[256] = {0};
	char callIDStr[CALL_ID_STRLEN] = {0};
	char confIDStr[CONF_ID_STRLEN] = {0};


    if(!inEventPtr)
    {
			NETERROR(MSCC,("%s Null EventPtr\n",fn));
			return -1;
    }

    if(!SCC_IsNetworkEvent(inEventPtr->event))
    {
			NETERROR(MSCC,("%s Invalid Event %d\n"
				,fn, inEventPtr->event));
			H323FreeEvent(inEventPtr);
			return -1;
    }

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s callID = %s confID = %s\n",
		fn,CallID2String(inEventPtr->callID,callIDStr),
		ConfID2String(inEventPtr->confID,confIDStr)));

    CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
    if(!(callHandle = CacheGet(callCache,inEventPtr->callID)))
    {
		DEBUG(MSCC, NETLOG_DEBUG4, ("%s CacheGet failed for %s\n",
			fn,sccEventBlockToStr(inEventPtr, str)));
		CacheReleaseLocks(callCache);
		H323FreeEvent(inEventPtr);
		return -1;
    }
   
    sm = &SCC_smH323Leg1Network[callHandle->state][inEventPtr->event];

    NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg1Network %s/%s-->%s\n",
		fn,SCC_EventToStr(inEventPtr->event,evtStr),
		SCC_CallStateToStr(callHandle->state,str),
		SCC_CallStateToStr(sm->nextState,str2)));

    // Move to next state 
    callHandle->state = sm->nextState;

    CacheReleaseLocks(callCache);



    if(sm->action)
    {
	   uh323StackLock();

       if ((sm->action) (inEventPtr) != 0)
       {
			/* One of the action routines has failed */
			DEBUG(MSCC,NETLOG_DEBUG2,("%s Action Routine failed\n", fn));

	   		uh323StackUnlock();

			return 0;
		}
		else {
				DEBUG(MSCC, NETLOG_DEBUG4,
					("%s Action Routine was successful\n",fn));
      		}
	   	uh323StackUnlock();
     }
	 else 
	 {
		NETDEBUG(MSCC, NETLOG_DEBUG4,("%s Action Routine NULL\n",fn));
		H323FreeEvent(inEventPtr);
	 }
    return 0;
}


// returns 0 on success negative value on error
// 0 if OK -1 if error
int 
SCC_H323Leg2BridgeEventProcessor(SCC_EventBlock *inEventPtr)
{
    char fn[] = "SCC_Leg2BridgeProcessEvent():";
    CallHandle 	callHandleBlk,*callHandle = &callHandleBlk;
    SCC_StateMachineEntry *sm;
	char str[256] = {0},str2[256] = {0},evtStr[256] = {0};
	char callIDStr[CALL_ID_STRLEN] = {0};
	char confIDStr[CONF_ID_STRLEN] = {0};

	inEventPtr->evtProcessor = SCC_EventProcessorWorker;
	return SCC_QueueEvent(inEventPtr);

    if(!inEventPtr || !SCC_IsBridgeEvent(inEventPtr->event))
    {
		NETERROR(MSCC,("%s Invalid Event",fn));
		return -1;
    }


	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s callID = %s confID = %s\n",
		fn,CallID2String(inEventPtr->callID,callIDStr),
		ConfID2String(inEventPtr->confID,confIDStr)));

    // Locate CallHandle 
    // CHECK Allocate Handle if Setup
    
    // Move to next state 
    CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
    if(!(callHandle = CacheGet(callCache,inEventPtr->callID)))
    {
		DEBUG(MSCC, NETLOG_DEBUG4, ("%s CacheGet failed for %s\n",
			fn,sccEventBlockToStr(inEventPtr, str)));
		CacheReleaseLocks(callCache);
		return -1;
    }

	sm = &SCC_smH323Leg2Bridge[callHandle->state]
		[inEventPtr->event - SCC_eBridgeEventsMin];

    NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg2Bridge %s/%s-->%s\n",
		fn,SCC_EventToStr(inEventPtr->event,evtStr),
		SCC_CallStateToStr(callHandle->state,str),
		SCC_CallStateToStr(sm->nextState,str2)));

    callHandle->state = sm->nextState;
    CacheReleaseLocks(callCache);

    if(sm->action)
    {
									
	   uh323StackLock();

       if ((sm->action) (inEventPtr) != 0)
       {
			uh323StackUnlock();

	    	/* One of the action routines has failed */
	    	NETDEBUG (MSCC, NETLOG_DEBUG2,
				("%s Action Routine failed\n",fn));
			return 0;
       }
       else {
				NETDEBUG(MSCC, NETLOG_DEBUG4,
					("%s Action Routine was successful\n",fn));
           }

	   uh323StackUnlock();
     }
                                                                                
    return 0;                                                             
}




// returns 0 on success negative value on error
// 0 if OK -1 if error
int 
SCC_H323Leg2NetworkEventProcessor(SCC_EventBlock *inEventPtr)
{
    char fn[] = "SCC_Leg2NetworkProcessEvent():";
    CallHandle 	callHandleBlk,*callHandle = &callHandleBlk;
    SCC_StateMachineEntry *sm;
	char str[256] = {0},str2[256] = {0},evtStr[256] = {0};
	char callIDStr[CALL_ID_STRLEN] = {0};
	char confIDStr[CONF_ID_STRLEN] = {0};

    if(!inEventPtr)
    {
			NETERROR(MSCC,("%s Null EventPtr\n",fn));
			return -1;
    }

    if(!SCC_IsNetworkEvent(inEventPtr->event))
    {
			NETERROR(MSCC,("%s Invalid Event %d\n"
				,fn, inEventPtr->event));
			H323FreeEvent(inEventPtr);
			return -1;
    }

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s callID = %s confID = %s\n",
		fn,CallID2String(inEventPtr->callID,callIDStr),
		ConfID2String(inEventPtr->confID,confIDStr)));

    // Move to next state 
    CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
    if(!(callHandle = CacheGet(callCache,inEventPtr->callID)))
    {
		DEBUG(MSCC, NETLOG_DEBUG4, ("%s CacheGet failed for %s\n",
			fn,sccEventBlockToStr(inEventPtr, str)));
		CacheReleaseLocks(callCache);
		H323FreeEvent(inEventPtr);
		return -1;
    }


    sm = &SCC_smH323Leg2Network[callHandle->state][inEventPtr->event];

    NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg2Network %s/%s-->%s\n",
		fn,SCC_EventToStr(inEventPtr->event,evtStr),
		SCC_CallStateToStr(callHandle->state,str),
		SCC_CallStateToStr(sm->nextState,str2)));

    callHandle->state = sm->nextState;
    CacheReleaseLocks(callCache);

    if(sm->action)
    {
									
	   uh323StackLock();

		if ((sm->action) (inEventPtr) != 0)
		{
			/* One of the action routines has failed */
	    	DEBUG(MSCC,NETLOG_DEBUG2,("%s Action Routine failed\n",fn));

	   		uh323StackUnlock();

			return 0;
		}
		else {
			DEBUG(MSCC, NETLOG_DEBUG4,
                        ("%s Action Routine was successful\n",fn));
	   	}

	   	uh323StackUnlock();
	}
	else
	{
		NETDEBUG(MSCC, NETLOG_DEBUG4,("%s Action Routine NULL\n",fn));
		H323FreeEvent(inEventPtr);
	}
    return 0;
}



// returns 0 on success negative value on error
// 0 if OK -1 if error
int 
SCC_H323Leg1BridgeEventProcessor(SCC_EventBlock *inEventPtr)
{
    char fn[] = "SCC_Leg1BridgeProcessEvent():";
    CallHandle 	callHandleBlk,*callHandle;
    SCC_StateMachineEntry *sm;
	char str[256] = {0},str2[256] = {0},evtStr[256] = {0};
	char callIDStr[CALL_ID_STRLEN] = {0};
	char confIDStr[CONF_ID_STRLEN] = {0};

	inEventPtr->evtProcessor = SCC_EventProcessorWorker;
	return SCC_QueueEvent(inEventPtr);

    if(!inEventPtr || !SCC_IsBridgeEvent(inEventPtr->event))
    {
		NETERROR(MSCC,("%s Invalid Event",fn));
		return -1;
    }

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s callID = %s confID = %s\n",
		fn,CallID2String(inEventPtr->callID,callIDStr),
		ConfID2String(inEventPtr->confID,confIDStr)));

    CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
    if(!(callHandle = CacheGet(callCache,inEventPtr->callID)))
    {
		DEBUG(MSCC, NETLOG_DEBUG4, ("%s CacheGet failed for %s\n",
			fn,sccEventBlockToStr(inEventPtr, str)));
		CacheReleaseLocks(callCache);
		return -1;
    }


    sm = &SCC_smH323Leg1Bridge[callHandle->state]
		[inEventPtr->event-SCC_eBridgeEventsMin];

    NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg1Bridge %s/%s-->%s\n",
		fn,SCC_EventToStr(inEventPtr->event,evtStr),
		SCC_CallStateToStr(callHandle->state,str),
		SCC_CallStateToStr(sm->nextState,str2)));

    // Move to next state 
    callHandle->state = sm->nextState;
    CacheReleaseLocks(callCache);

	// Action routines are not supposed to fail
    if(sm->action)
    {
	   uh323StackLock();

       if ((sm->action) (inEventPtr) != 0)
       {
			uh323StackUnlock();
	    	DEBUG(MSCC,NETLOG_DEBUG2, ("%s Action Routine failed\n",fn));
			return 0;
       }
	   else {
			DEBUG(MSCC, NETLOG_DEBUG4,
				("%s Action Routine was successful\n",fn));
		}

	   uh323StackUnlock();
     }
                                                                                
    return 0;                                                             
}

// Consumes evtPtr on success
int
SCC_BridgeEventWorker(SCC_EventBlock *evtPtr)
{
    char fn[] = "SCC_BridgeEventWorker():";
    CallHandle 	callHandleBlk,*callHandle;
    SCC_StateMachineEntry *sm;
	char str[256] = {0},str2[256] = {0},evtStr[256] = {0};
	char callIDStr[CALL_ID_STRLEN] = {0};
	char confIDStr[CONF_ID_STRLEN] = {0};
	SCC_CallLeg leg;

    if(!evtPtr || !SCC_IsBridgeEvent(evtPtr->event))
    {
		NETERROR(MSCC,("%s Invalid Event",fn));
		return -1;
    }

    CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);
    if(!(callHandle = CacheGet(callCache,evtPtr->callID)))
    {
		DEBUG(MSCC, NETLOG_DEBUG4, ("%s CacheGet failed for %s\n",
			fn,sccEventBlockToStr(evtPtr, str)));
		CacheReleaseLocks(callCache);
		return -1;
    }

	leg = callHandle->leg;

	NETDEBUG(MSCC, NETLOG_DEBUG4,
		("%s callID = %s confID = %s\n",
		fn,CallID2String(evtPtr->callID,callIDStr),
		ConfID2String(evtPtr->confID,confIDStr)));

	if (leg == SCC_CallLeg1)
	{
    	sm = &SCC_smH323Leg1Bridge[callHandle->state]
			[evtPtr->event-SCC_eBridgeEventsMin];

		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg1Bridge %s/%s-->%s\n",
			fn,SCC_EventToStr(evtPtr->event,evtStr),
			SCC_CallStateToStr(callHandle->state,str),
			SCC_CallStateToStr(sm->nextState,str2)));
	}
	else
	{
		sm = &SCC_smH323Leg2Bridge[callHandle->state]
			[evtPtr->event - SCC_eBridgeEventsMin];

		NETDEBUG(MSCC, NETLOG_DEBUG4,
			("%s sm = SCC_smH323Leg2Bridge %s/%s-->%s\n",
			fn,SCC_EventToStr(evtPtr->event,evtStr),
			SCC_CallStateToStr(callHandle->state,str),
			SCC_CallStateToStr(sm->nextState,str2)));
	}


    // Move to next state 
    callHandle->state = sm->nextState;
    CacheReleaseLocks(callCache);

	// Action routines are not supposed to fail
    if(sm->action)
    {
#ifdef H323GLOBALLOCKS
		if (cmMeiEnter(UH323Globals()->hApp) < 0)
		{
			NETERROR(MH323, ("uh323Stack locks failed!!!\n"));
		}
#endif

       if ((sm->action) (evtPtr) != 0)
       {
	    	DEBUG(MSCC,NETLOG_DEBUG2, ("%s Action Routine failed\n",fn));
       }
	   else {
			DEBUG(MSCC, NETLOG_DEBUG4,
				("%s Action Routine was successful\n",fn));
		}

#ifdef H323GLOBALLOCKS
		cmMeiExit(UH323Globals()->hApp);
#endif
     }
                                                                                
    return 0;                                                             
}

int
SCC_Init()
{
	int i;

	// no threads
	h323inPool = (int *)malloc(nH323Threads*sizeof(int));
	h323inClass = (int *)malloc(nH323Threads*sizeof(int));
//	h323inhpClass = (int *)malloc(nH323Threads*sizeof(int));

	for (i = 0; i<nH323Threads; i++)
	{
		h323inPool[i] = ThreadPoolInit("h.323", 0, PTHREAD_SCOPE_PROCESS, 0, 2);
		h323inClass[i] = ThreadAddPoolClass("h.323-sm", h323inPool[i], 0, 100000000);
		//ThreadClassSetPfactor(h323inPool[i], h323inClass[i], 3);
		//ThreadClassSetPfactor(h323inPool[i], h323inClass[i], 30);

//		h323inhpClass[i] = ThreadAddPoolClass("h.323-smhp", h323inPool[i], 0, 20000000);
		//ThreadClassSetPfactor(h323inPool[i], h323inhpClass[i], 50);
		//ThreadClassSetPfactor(h323inPool[i], h323inhpClass[i], 30);
		
		ThreadPoolInitNotify(h323inPool[i]);

		ThreadPoolStart(h323inPool[i]);
	}

	// i1
//	ThreadClassSetPfactor(h323inPool[0], h323inClass[0], 2);
//	ThreadClassSetPfactor(h323inPool[0], h323inhpClass[0], 10);
	// o
//	ThreadClassSetPfactor(h323inPool[1], h323inClass[1], 3);
//	ThreadClassSetPfactor(h323inPool[1], h323inhpClass[1], 10);
	// i2
//	ThreadClassSetPfactor(h323inPool[1], h323inClass[2], 2);
//	ThreadClassSetPfactor(h323inPool[1], h323inhpClass[2], 10);
	return(0);
}

//Always consumes eventBlockPtr
int
SCC_QueueEvent(SCC_EventBlock *eventBlockPtr)
{
	char fn[] = "SCC_QueueEvent():";
	pthread_t callThreadId;
	unsigned short index = 0;
//	static int p = (nH323Threads-1)/2;

	// set up the event block processor
	if (eventBlockPtr->evtProcessor == NULL)
	{
		// Set it to the sm processor
		eventBlockPtr->evtProcessor = SCC_EventProcessorWorker;
	}

	callThreadId = *(long *)(eventBlockPtr->callID + 4);

	if (nh323Instances > 1)
	{
		// Route the call back to the originating thread
		while (index < nH323Threads)
		{
			if (pthread_equal(callThreadId, h323Threads[index]))
			{
				break;
			}
			index++;
		}

		if (index == nH323Threads)
		{
			NETERROR(MH323,
				("Unknown instance for call\n"));
#if 0
			// We must choose an outgoing instance
			// Based on the callid, one which is the same always
			// They are 1, 3, 5, 7....
			memcpy(&index, evtPtr->callID+14, 2);
			index %= p;
			index = 2(index) + 1;
#endif
			H323FreeEvent(eventBlockPtr);
			return 0;
		}
	}
	else if (nH323Threads > 1)
	{
		index = 1;
	}

	NETDEBUG(MH323, NETLOG_DEBUG4,
		("%s Queueing call thread id = %lu on instance %d\n",
		fn, ULONG_FMT(callThreadId), index));


	if (ThreadDispatch(h323inPool[index], 
		h323inClass[index],
		(PFVP)eventBlockPtr->evtProcessor, eventBlockPtr,
		1, PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59))
	{
		NETERROR(MSCC, ("%s Error in dispatching\n", fn));
	}

	return 0;
}
