/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SIPTHREADMGR_H__
#define __SIPTHREADMGR_H__
/******************************************************************************
 ** FUNCTION:
 **		Has the hashtable implementation used in the SipStack
 ******************************************************************************
 **
 ** FILENAME:		sipthreadmgr.h
 **
 ** DESCRIPTION:  	This file contains the definitions for implementation
 **					of the thread
 **                 manager  used
 **					within the SipStack.
 **
 ** DATE     	NAME          REFERENCE     REASON
 ** ----      	----          ---------     ------
 ** 12/04/01	K. Binu, 	  siphash.h		Creation
 **				Siddharth
 ** 11/12/02    Jyoti                        Changes for porting 
 **                                          to windows
 ******************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 ******************************************************************************/

#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#include <pthread.h>
#endif

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"

#define THREADMGR_THREADID_ANY	0

typedef struct ThreadMgr_struct
{
	SIP_U8bit nWorkerThreads;
	SIP_U32bit maxEventQSize;

	/* The following fields are all arrays of size nWorkerThreads */
	thread_id_t *workerThreadId;

	void *(**eventQ); 		/* an array of void* arrays */
	SIP_U32bit *eventQHead;
	SIP_U32bit *eventQTail;

	synch_id_t *QLock;
	synch_id_t *CondEventLock;
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
	thread_cond_t *CondQVar;
	thread_cond_t *CondEventVar;
#else
	synch_id_t *SemQVar;
	synch_id_t *SemEventVar;
#endif

} TThreadMgr;

/******************************************************************************
 ** FUNCTION: threadMgrInit
 **
 ** DESCRIPTION: Init function for the Thread Manager
 **
 ** PARAMETERS:
 ** thrMgr(IN):	Reference to the thread manager
 ** nWorkerThreads(IN):	Number of worker threads
 ** MaxEventQsize(IN): The size of the Events Q
 ******************************************************************************/
SipBool threadMgrInit(
	TThreadMgr *thrMgr,
	SIP_U8bit nWorkerThreads,
	SIP_U32bit MaxEventQsize);

/******************************************************************************
 ** FUNCTION: threadMgrSpawnThreads
 **
 ** DESCRIPTION: Function used to spawn threads within the Thread Manager
 **
 ** PARAMETERS:
 ** thrMgr(IN):	Reference to the thread manager
 ** attr(IN):	The attribute for the threads that will be spawned
 ** master_routine(IN): The function ptr to the master_routine
 ** worker_routine(IN): The function ptr to the worker_routine
 ******************************************************************************/
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SipBool threadMgrSpawnThreads(
	TThreadMgr *thrMgr, const thread_attr_t *attr,
	void *(*master_routine)(void *),
	void *(*worker_routine)(void *));
#endif
/******************************************************************************
 ** FUNCTION: threadMgrPostEvent
 **
 ** DESCRIPTION: Function used to post an event to the Thread Manager
 **
 ** PARAMETERS:
 ** thrMgr(IN):	Reference to the thread manager
 ** threadId(IN):	The threadId of the thread posting the event
 ** event(IN): The event that is getting posted
 ******************************************************************************/
SipBool threadMgrPostEvent(
	TThreadMgr *thrMgr,
	SIP_U8bit	threadId,
	void *event);

/******************************************************************************
 ** FUNCTION: threadMgrGetEventFromMaster
 **
 ** DESCRIPTION: Function used to receive an event from the Thread Mgr
 **
 ** PARAMETERS:
 ** thrMgr(IN):	Reference to the thread manager
 ** threadId(IN):	The threadId of the thread posting the event
 ** event(OUT): The event obtained.
 ******************************************************************************/
SipBool threadMgrGetEventFromMaster(
	TThreadMgr *thrMgr,
	SIP_U8bit	threadId,
	void **event);

/******************************************************************************
 ** FUNCTION: threadMgrFree
 **
 ** DESCRIPTION: Function used to free the memory allocated by Thread Mgr
******************************************************************************/
SipBool threadMgrFree(TThreadMgr *thrMgr);

#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif
