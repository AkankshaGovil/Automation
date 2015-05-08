/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 ** FUNCTION:
 **			The HSS SIP Stack performance testing application.
 **
 *****************************************************************************
 **
 ** FILENAME:		sipthreadmgr.c
 **
 ** DESCRIPTION:	This file contains generic thread manager utiltity functions
 **					that manage several worker threads waiting for events from
 **					a single master thread.
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			---------		------
 ** 10-Nov-01		Ramu K							Creation
 ** 11-Dec-02       Jyoti                           Updation for win-5.0 rel
 *****************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 *****************************************************************************/

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "sipcommon.h"
#include "sipstruct.h"
#include "sipthreadmgr.h"


/******************************************************************************
 ** FUNCTION: threadMgrInit
 **
 ** DESCRIPTION: Init function for the Thread Manager
 **
 ** PARAMETERS:
 ******************************************************************************/
SipBool
threadMgrInit(
	TThreadMgr *thrMgr,
	SIP_U8bit nWorkerThreads,
	SIP_U32bit maxEventQSize)
{
	SIP_U8bit i;
	SIP_U32bit j;

	thrMgr->nWorkerThreads = nWorkerThreads;
	thrMgr->maxEventQSize = maxEventQSize;


	/* Initialize all array pointer to NULL and Allocate all the arrays */

	thrMgr->workerThreadId = NULL;
	thrMgr->eventQHead = NULL;
	thrMgr->eventQTail = NULL;
	thrMgr->eventQ = NULL;
	thrMgr->QLock = NULL;
	thrMgr->CondEventLock = NULL;

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	thrMgr->CondQVar = NULL;
	thrMgr->CondEventVar = NULL;
#else
	thrMgr->SemQVar = NULL;
	thrMgr->SemEventVar = NULL;
#endif
	thrMgr->workerThreadId = (thread_id_t *) fast_memget(0, \
		sizeof(thread_id_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->workerThreadId == NULL)
		goto threadMgrInit_exceptionHndlr;


	thrMgr->eventQHead = (SIP_U32bit *) fast_memget(0,\
		sizeof(SIP_U32bit) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->eventQHead == NULL)
		goto threadMgrInit_exceptionHndlr;
	for (i=0; i< (SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
		thrMgr->eventQHead[i] = 0;

	thrMgr->eventQTail = (SIP_U32bit *) fast_memget(0,\
		sizeof(SIP_U32bit) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->eventQTail == NULL)
		goto threadMgrInit_exceptionHndlr;
	for (i=0; i< (SIP_U8bit) (thrMgr->nWorkerThreads+1); ++i)
		thrMgr->eventQTail[i] = 0;


	thrMgr->eventQ = (void ***) fast_memget(0,\
		sizeof(void **) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->eventQ == (void ***)NULL)
		goto threadMgrInit_exceptionHndlr;
	for (i=0; i< (SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
		thrMgr->eventQ[i] = (void **)NULL;
	for (i=1; i<(SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
	{
		thrMgr->eventQ[i] = (void **) fast_memget(0,\
			maxEventQSize * sizeof(void *),SIP_NULL);
		if (thrMgr->eventQ[i] == NULL)
			goto threadMgrInit_exceptionHndlr;
		for (j=0; j<maxEventQSize; ++j)
			*(thrMgr->eventQ[i] + j) = (void *) NULL;
	}


	thrMgr->QLock = (synch_id_t *) fast_memget(0,\
		sizeof(synch_id_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->QLock == NULL)
		goto threadMgrInit_exceptionHndlr;

	thrMgr->CondEventLock = (synch_id_t *) fast_memget(0,\
		sizeof(synch_id_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->CondEventLock == NULL)
		goto threadMgrInit_exceptionHndlr;

#if!defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	thrMgr->CondQVar = (thread_cond_t *) fast_memget(0,\
		sizeof(thread_cond_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->CondQVar == NULL)
		goto threadMgrInit_exceptionHndlr;

	thrMgr->CondEventVar = (thread_cond_t *) fast_memget(0,\
		sizeof(thread_cond_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->CondEventVar == NULL)
		goto threadMgrInit_exceptionHndlr;
#else
	/*Allocate memory for Semaphores*/
	thrMgr->SemQVar = (synch_id_t *) fast_memget(0,\
		sizeof(synch_id_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->SemQVar == NULL)
		goto threadMgrInit_exceptionHndlr;

	thrMgr->SemEventVar = (synch_id_t *) fast_memget(0,\
		sizeof(synch_id_t) * (nWorkerThreads+1),SIP_NULL);
	if (thrMgr->SemEventVar == NULL)
		goto threadMgrInit_exceptionHndlr;
#endif

	return SipSuccess;

threadMgrInit_exceptionHndlr:
	printf("Reached thread manager exception handler\n");

	if (thrMgr->workerThreadId != NULL)
		fast_memfree(0,thrMgr->workerThreadId,SIP_NULL);

	if (thrMgr->eventQHead != NULL)
		fast_memfree(0,thrMgr->eventQHead,SIP_NULL);

	if (thrMgr->eventQTail != NULL)
		fast_memfree(0,thrMgr->eventQTail,SIP_NULL);

	if (thrMgr->eventQ != NULL)
	{
		for (i=0; i< (SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
		{
			if (thrMgr->eventQ[i] != NULL)
				fast_memfree(0,thrMgr->eventQ[i],SIP_NULL);
		}
		fast_memfree(0,thrMgr->eventQ,SIP_NULL);
	}

	if (thrMgr->QLock != NULL)
		fast_memfree(0,thrMgr->QLock,SIP_NULL);

	if (thrMgr->CondEventLock != NULL)
		fast_memfree(0,thrMgr->CondEventLock,SIP_NULL);

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	if (thrMgr->CondQVar != NULL)
		fast_memfree(0,thrMgr->CondQVar,SIP_NULL);

	if (thrMgr->CondEventVar != NULL)
		fast_memfree(0,thrMgr->CondEventLock,SIP_NULL);
#else
	if (thrMgr->SemQVar != NULL)
		fast_memfree(0,thrMgr->SemQVar,SIP_NULL);

	if (thrMgr->SemEventVar != NULL)
		fast_memfree(0,thrMgr->CondEventLock,SIP_NULL);
#endif

	return SipFail;
}

/******************************************************************************
 ** FUNCTION: threadMgrSpawnThreads
 **
 ** DESCRIPTION: Function used to spawn threads within the Thread Manager
 **
 ** PARAMETERS:
 ******************************************************************************/
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SipBool
threadMgrSpawnThreads(
	TThreadMgr *thrMgr, const thread_attr_t *attr,
	void *(*masterRoutine)(void *),
	void *(*workerRoutine)(void *))
#endif
{
	SIP_U8bit i;
	SIP_U8bit *threadIndex;
	thread_id_t threadId;
	for (i=1; i<(SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
		fast_init_synch(&thrMgr->QLock[i]);

	for (i=1; i<(SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
	{
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
		if (pthread_cond_init(&thrMgr->CondQVar[i], NULL) != 0)
			return SipFail;
#endif
	}
	for (i=1; i<(SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
		fast_init_synch(&thrMgr->CondEventLock[i]);

	for (i=1; i<(SIP_U8bit)(thrMgr->nWorkerThreads+1); ++i)
	{
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
		if (pthread_cond_init(&thrMgr->CondEventVar[i], NULL) != 0)
			return SipFail;
#endif
	}

	if (masterRoutine != NULL)
	{
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
		if (pthread_create(&threadId, attr, masterRoutine, NULL)
	    	!= 0)
		{
			return SipFail;
		}
#endif
	}

	if (workerRoutine != NULL)
	{
		for (i=1; i<=thrMgr->nWorkerThreads; ++i)
		{
			threadIndex = (SIP_U8bit *) fast_memget(0,\
				sizeof(SIP_U8bit),SIP_NULL);
			*threadIndex = i;

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
			if (pthread_create(&threadId, attr, workerRoutine, \
				(void *)threadIndex) != 0)
			{
				return SipFail;
			}
#endif
			thrMgr->workerThreadId[i] = threadId;
		}
	}

	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: threadMgrFree
 **
 ** DESCRIPTION: Function used to free the memory allocated by the Thread Manager
 **
 ** PARAMETERS:
 ******************************************************************************/

SipBool threadMgrFree(TThreadMgr *thrMgr) 
{
	SIP_U8bit i;
	SIP_U8bit nWorkerThreads=0;

	/* Initialize all array pointers to SIP_NULL; Allocate all the arrays */

	sip_memfree(0, (SIP_Pvoid*)&thrMgr->workerThreadId,SIP_NULL);
	sip_memfree(0, (SIP_Pvoid*)&thrMgr->eventQHead,SIP_NULL);
	sip_memfree(0, (SIP_Pvoid*)&thrMgr->eventQTail,SIP_NULL);
	nWorkerThreads=thrMgr->nWorkerThreads;

	for (i=1; i<(SIP_U8bit)(nWorkerThreads+1); ++i)
	{
		sip_memfree(0, (SIP_Pvoid*)&thrMgr->eventQ[i],SIP_NULL);
	}

	sip_memfree(0, (SIP_Pvoid*)&thrMgr->eventQ,SIP_NULL);

	sip_memfree(0, (SIP_Pvoid*)&thrMgr->QLock,SIP_NULL);
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	sip_memfree(0, (SIP_Pvoid*)&thrMgr->CondQVar,SIP_NULL);
#endif

#ifndef SDF_WINDOWS
	sip_memfree(0, (SIP_Pvoid*)&thrMgr->CondEventLock,SIP_NULL);
#endif

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	sip_memfree(0, (SIP_Pvoid*)&thrMgr->CondEventVar,SIP_NULL);
#endif

	thrMgr->workerThreadId = SIP_NULL;
	thrMgr->eventQHead = SIP_NULL;
	thrMgr->eventQTail = SIP_NULL;
	thrMgr->eventQ = SIP_NULL;
	thrMgr->QLock = SIP_NULL;
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	thrMgr->CondQVar = SIP_NULL;
#endif

	
#ifndef SDF_WINDOWS
	thrMgr->CondEventLock = SIP_NULL;
#endif
	
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	thrMgr->CondEventVar = SIP_NULL;
#endif
	return SipSuccess;
} 

 
/******************************************************************************
 ** FUNCTION: threadMgrPostEvent
 **
 ** DESCRIPTION: Function used to post an event to the Thread Manager
 **
 ** PARAMETERS:
 ******************************************************************************/
SipBool
threadMgrPostEvent(
	TThreadMgr *thrMgr,
	SIP_U8bit	threadId,
	void *event)
{
	static SIP_U8bit threadIdRoundRobin = 1;

	if (threadId == THREADMGR_THREADID_ANY)
	{
		threadId = threadIdRoundRobin++;
		if (threadIdRoundRobin > thrMgr->nWorkerThreads)
			threadIdRoundRobin = 1;
	}

	/* Add the event to event Q; if the Q is full, wait for some worker thread
	** to clear up an event from the queue
	*/

	fast_lock_synch(0,&thrMgr->QLock[threadId],0);

	while (((thrMgr->eventQTail[threadId] + 1) % thrMgr->maxEventQSize)
	     == thrMgr->eventQHead[threadId])
	{
		/* Q full */
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
		if (pthread_cond_wait(&thrMgr->CondQVar[threadId], \
			&thrMgr->QLock[threadId]) != 0)
		{
			printf("ThreadMgr :: Failed conditional wait on CondQVar\n");
			fast_unlock_synch(0,&thrMgr->QLock[threadId]);
			return SipFail;
		}
#endif
	}

	/* Add event to Q */
	*(thrMgr->eventQ[threadId] + thrMgr->eventQTail[threadId]) = event;
	thrMgr->eventQTail[threadId] = (thrMgr->eventQTail[threadId]+1) % \
		thrMgr->maxEventQSize;
	fast_unlock_synch(0,&thrMgr->QLock[threadId]);

	/* Singal the worker threads that are waiting for events */
	fast_lock_synch(0,&thrMgr->CondEventLock[threadId],0);
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	if (pthread_cond_signal(&thrMgr->CondEventVar[threadId]) != 0)
	{
		printf("ThreadMgr :: Failed to signal CondEventVar\n");
		fast_unlock_synch(0,&thrMgr->CondEventLock[threadId]);
		return SipFail;
	}
#endif

	fast_unlock_synch(0,&thrMgr->CondEventLock[threadId]);

	return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: threadMgrGetEventFromMaster
 **
 ** DESCRIPTION: Function used to receive an event from the Thread Mgr
 **
 ** PARAMETERS:
 ******************************************************************************/
SipBool
threadMgrGetEventFromMaster(
	TThreadMgr *thrMgr,
	SIP_U8bit	threadId,
	void **event)
{
	*event = NULL;

	/* if Q is empty wait for the master to post an event ;
	** else go ahead and extract an event
	*/

	while (1)
	{
		fast_lock_synch(0,&thrMgr->QLock[threadId],0);

		if (thrMgr->eventQTail[threadId] != thrMgr->eventQHead[threadId])
			break;

		/* Q empty */

		fast_unlock_synch(0,&thrMgr->QLock[threadId]);

		fast_lock_synch(0,&thrMgr->CondEventLock[threadId],0);

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
		if (pthread_cond_wait(&thrMgr->CondEventVar[threadId], \
			&thrMgr->CondEventLock[threadId]) != 0)
		{
			printf("ThreadMgr :: Failed to signal CondEventVar\n");
			fast_unlock_synch(0,&thrMgr->CondEventLock[threadId]);
			return SipFail;
		}
#endif

		fast_unlock_synch(0,&thrMgr->CondEventLock[threadId]);
	}

	/* Q not empty */

	*event = *(thrMgr->eventQ[threadId] + thrMgr->eventQHead[threadId]);
	thrMgr->eventQHead[threadId] = (thrMgr->eventQHead[threadId]+1) % \
		thrMgr->maxEventQSize;

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	if (pthread_cond_signal(&thrMgr->CondQVar[threadId]) != 0)
	{
		printf("ThreadMgr :: Failed to signal CondQVar\n");
		fast_unlock_synch(0,&thrMgr->QLock[threadId]);
		return SipFail;
	}
#endif

	fast_unlock_synch(0,&thrMgr->QLock[threadId]);

	return SipSuccess;
}
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif
