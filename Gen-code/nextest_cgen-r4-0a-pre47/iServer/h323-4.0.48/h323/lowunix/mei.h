#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced IN  any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef __MEI_H
#define __MEI_H

#include <rvinternal.h>

/* Handle of a mutual exclusion instance = lock */
DECLARE_OPAQUE(HMEI);

#ifndef NOTHREADS


/************************************************************************
 * meiGlobalInit
 * purpose: Initialize mutual exclusion module. This function should be
 *          called at the begining of execution
 * input  : none
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int meiGlobalInit(void);


/************************************************************************
 * meiGlobalEnd
 * purpose: Deinitialize mutual exclusion module.
 * input  : none
 * output : none
 * return : none
 ************************************************************************/
void meiGlobalEnd(void);


/************************************************************************
 * meiGetSize
 * purpose: Returns the size of a lock. We use this function to call
 *          meiInitFrom, which will reduce amount of malloc()s in the
 *          code while running.
 * input  : none
 * output : none
 * return : Size of a lock (OS dependant)
 ************************************************************************/
int meiGetSize(void);


/************************************************************************
 * meiInitFrom
 * purpose: Initialize a lock from a given place in memory. This allows
 *          reducing the amount of malloc()s during execution.
 *          This kind of locks have to be deallocated using meiEndFrom()
 *          and NOT by meiEnd().
 * input  : ptr - Place in memory for the handle
 * output : none
 * return : Lock handle on success
 *          NULL on failure
 ************************************************************************/
HMEI meiInitFrom(IN void* ptr);


/************************************************************************
 * meiEndFrom
 * purpose: Deallocate a lock that was initialized by meiInitFrom().
 * input  : hMEI    - lock handle to use
 * output : none
 * return : Negative value on error
 ************************************************************************/
int meiEndFrom(IN HMEI hMEI);


/************************************************************************
 * meiInit
 * purpose: Initialize a mutex.
 *          This will cause a dynamic allocation to be invoked
 * input  : none
 * output : none
 * return : Mutex handle
 ************************************************************************/
HMEI meiInit(void);


/************************************************************************
 * meiEnter
 * purpose: Lock a mutex
 *          This will cause the mutex to lock for use in the current
 *          thread. It will wait for another thread to unlock it if it
 *          was previously locked by the other thread.
 *          The mutex is reentrant - the same thread can lock the mutex
 *          several times without unlocking it.
 * input  : hMEI    - Mutex handle to lock
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int meiEnter(IN HMEI hMEI);


/************************************************************************
 * meiExit
 * purpose: Unlock a mutex
 *          This will cause the mutex to unlock after meiEnter().
 *          The mutex is reentrant - the same thread can lock the mutex
 *          several times without unlocking it.
 * input  : hMEI    - Mutex handle to unlock
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int meiExit(IN HMEI hMEI);


/************************************************************************
 * meiEnd
 * purpose: Deallocate a lock that was initialized by meiInit().
 * input  : hMEI    - lock handle to use
 * output : none
 * return : Negative value on error
 ************************************************************************/
int meiEnd(IN HMEI hMEI);


#else /* NOTHREADS */

#define meiGlobalInit() 0
#define meiGlobalEnd() 0
#define meiGetSize() 0
#define meiInitFrom(ptr) ((HMEI)(1))
#define meiEndFrom(hMEI) if (hMEI);
#define meiInit() ((HMEI)(1))
#define meiEnter(hMEI) if (hMEI);
#define meiExit(hMEI)  if (hMEI);
#define meiEnd(hMEI)   if (hMEI);

#endif /* NOTHREADS */
/* In case we are working with threads*/

#ifndef RTP_NOLOCKS

/********************************************************************************************
* mutexInitFrom
* purpose : This routine initializes a critical object in supplied space
* input   : ptr - pointer to the space to initialize critical object.
* output  : none
* return  : HMEI - handle of a mutex.
*           NULL on failure.
********************************************************************************************/
HMEI mutexInitFrom(void *);

/********************************************************************************************
* mutexGetAllocationSize
* purpose : This routine define the size of "mutex".
* input   : none.
* output  : none
* return  : int the size of "mutex".
********************************************************************************************/
int mutexGetAllocationSize(void);

/********************************************************************************************
* mutexEnd
* purpose : This routine free a memory and releases all resources used by an unowned
*           critical section object. 
* input   : mutexHandle - A mutex handle.
* output  : none
* return  : none.
********************************************************************************************/
void mutexEnd(HMEI);


/*********************************************************************************************
* mutexLock
* purpose : Lock a mutex
* input   : mutexHandle    - The mutex handle.
* output  : none.
* return  : return 0.
*********************************************************************************************/
int mutexLock(HMEI);


/*********************************************************************************************
* mutexUnlock
* purpose : Unlock a locked mutex
* input   : mutexHandle    - The handle to the mutex
* output  : none
* return  : return 0.
*********************************************************************************************/
int mutexUnlock(HMEI);

#else
#define mutexInit() ((HMEI )-1)
#define mutexInitFrom(a) ((HMEI )-1)
#define mutexGetAllocationSize() 0
#define mutexEnd(HMEI)
#define mutexLock(HMEI) 0
#define mutexUnlock(HMEI) 0

#endif

#endif /* __MEI_H */
#ifdef __cplusplus
}
#endif
