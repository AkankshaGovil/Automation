#ifdef __cplusplus
extern "C" {
#endif


/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

#include <mei.h>

#if ( !defined( RTP_NOLOCKS ) || !defined( NOTHREADS ) )

#include <pthread.h>
#include <Threads_API.h>

/* Determine which mutex solution to use by checking the OS */

#define RV_MUTEX_GENERIC

#if ((defined(IS_PLATFORM_SOLARIS) || defined(IS_PLATFORM_SOLARISPC)) && defined(PTHREAD_MUTEX_RECURSIVE)) /* NexTone */
/* The RV_MUTEX_SOLARIS is much more efficient than
 * RV_MUTEX_GENERIC.  However, Solaris 2.6 can not use
 * RV_MUTEX_SOLARIS, and Solaris 2.7 and 2.8 require
 * patches.  RV_MUTEX_GENERIC should be used with 
 * Solaris 2.6 or with Solaris 2.7 or 2.8 when the 
 * patches are not applied.  By default the code assumes 
 * Solaris 2.7 and 2.8 have the patches applied.
*/
#define RV_MUTEX_SOLARIS
#undef RV_MUTEX_GENERIC

typedef pthread_mutex_t mei_t;
#endif

#ifdef __REDHAT__
#define RV_MUTEX_REDHAT
#undef RV_MUTEX_GENERIC

typedef pthread_mutex_t mei_t;
#endif


#ifdef RV_MUTEX_GENERIC
#include <semaphore.h>

typedef struct
{
    sem_t               lock;
    sem_t               handle;
    int                 count;
    RvH323ThreadHandle  owner;
    int                 waiters;
} mei_t;

#endif



#include <errno.h>
#include <ms.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


#ifndef NOTHREADS
static int meiInitialized = 0;
static int msa = 0;
#endif



#ifdef RV_MUTEX_GENERIC
/* This function makes sure we're waiting on a semaphore without any signals interrupting us
   in the middle */
static int lock_LockMutexBySem(IN sem_t *sem)
{
    do
    {
        if (sem_wait(sem) == 0)
        {
            /* Succeed to lock semaphore */
            errno = 0;
            return 0;
        }
    } while (errno == EINTR);
  
    /* Error occured, semaphore value remains unchanged */
    return RVERROR;
}
#endif




void meiInitInternal(IN mei_t* mei)
{
#ifdef RV_MUTEX_SOLARIS
	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mei, &ma);
	pthread_mutexattr_destroy(&ma);
#endif

#ifdef RV_MUTEX_REDHAT
	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setkind_np(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(mei, &ma);
	pthread_mutexattr_destroy(&ma);
#endif

#ifdef RV_MUTEX_GENERIC
    mei->owner = NULL;
    mei->count = 0;
    mei->waiters = 0;
    sem_init(&(mei->lock), 0, 1);
    sem_init(&(mei->handle), 0, 0);
#endif
}


void meiEndInternal(IN mei_t* mei)
{
#if (defined(RV_MUTEX_SOLARIS) || defined (RV_MUTEX_REDHAT))
    pthread_mutex_destroy(mei);
#endif

#ifdef RV_MUTEX_GENERIC
    sem_destroy(&(mei->lock));
    sem_destroy(&(mei->handle));
#endif
}


int meiEnterInternal( mei_t * mei )
{
#if (defined(RV_MUTEX_SOLARIS) || defined (RV_MUTEX_REDHAT))
    return pthread_mutex_lock(mei);
#endif

#ifdef RV_MUTEX_GENERIC
    RvH323ThreadHandle curThreadId = RvH323ThreadGetHandle();
    BOOL decreaseWaiters = FALSE;

    do
    {
        if (lock_LockMutexBySem(&(mei->lock)) < 0)
        {
            /* Error has occured. */
#ifndef NOTHREADS
            msaPrintFormat(msa,"meiEnter: failed trying to lock thread=0x%p, mutex=0x%p, count=%d, errno=%d",
                curThreadId, mei, mei->count, errno);
#endif
            return -1;
        }

        if ((mei->owner == NULL) || (mei->owner == curThreadId))
        {
            /* Seems like there's no owner for this mutex, or we are the rightful owners */
            mei->owner = curThreadId;
            mei->count++;
            if (decreaseWaiters)
                mei->waiters--;

            /* Mutex is now locked */
	        sem_post(&(mei->lock));
	        return 0;
        }

        /* We should wait on this mutex. Someone else has it locked */
        decreaseWaiters = TRUE;
        sem_post(&(mei->lock));
        if (lock_LockMutexBySem(&(mei->handle)) < 0)
        {
            /* Error has occured. */
#ifndef NOTHREADS
            msaPrintFormat(msa,"meiEnter: failed trying to lock thread=0x%p, mutex=0x%p, count=%d, errno=%d",
                curThreadId, mei, mei->count, errno);
#endif
            return -1;
        }

    } while (1);
#endif  /* RV_MUTEX_GENERIC */
}


int meiExitInternal(IN mei_t * mei)
{
#if (defined(RV_MUTEX_SOLARIS) || defined (RV_MUTEX_REDHAT))
    return pthread_mutex_unlock(mei);
#endif

#ifdef RV_MUTEX_GENERIC
    RvH323ThreadHandle curThreadId = RvH323ThreadGetHandle();

    if (lock_LockMutexBySem(&(mei->lock)) < 0)
    {
        /* Error occured, semaphore remains unchanged */
#ifndef NOTHREADS
        msaPrintFormat(msa,"meiEnter: failed trying to unlock thread=0x%p, mutex=0x%p, count=%d, errno=%d",
            curThreadId, mei, mei->count, errno);
#endif
        return -1;
    }

    /* Make sure unlocking thread is owner and count is not zero */
    if (mei->owner != curThreadId)
    {
#ifndef NOTHREADS
        msaPrintFormat(msa,"meiExit: Thread not owner! mei=0x%p, thread=0x%p", mei, curThreadId);
#endif

        /* sem_wait succeed, semaphore should be freed */
        sem_post(&(mei->lock));
        return -1;
    }

    if (mei->count == 0)
    {
#ifndef NOTHREADS
        msaPrintFormat(msa,"meiExit: Mutex not locked! mei=0x%p", mei);
#endif

        /* sem_wait succeed, semaphore should be freed */
        sem_post(&(mei->lock));
        return -1;
    }
  
    mei->count--;
    if (mei->count == 0)
    {
        mei->owner = NULL;
        if (mei->waiters > 0)
        {
            /* Someone is waiting for this mutex to be freed. Let's help him.. */
	        sem_post(&(mei->handle));
        }
    }

    /* We're done here */
    sem_post(&(mei->lock));

    return 0;
#endif  /* RV_MUTEX_GENERIC */
}


#else

int RV_dummyMEI = 0;

#endif  /* RTP_NOLOCKS / NOTHREADS */





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
int meiGlobalInit(void)
{
    meiInitialized++;
    if (meiInitialized == 1)
        msa = msaRegister(0,"MEI","Mutual Exclusions");

    return 0;
}


/************************************************************************
 * meiGlobalEnd
 * purpose: Deinitialize mutual exclusion module.
 * input  : none
 * output : none
 * return : none
 ************************************************************************/
void meiGlobalEnd(void)
{
    meiInitialized--;
    if (meiInitialized == 0)
        msaUnregister(msa);
}


/************************************************************************
 * meiGetSize
 * purpose: Returns the size of a lock. We use this function to call
 *          meiInitFrom, which will reduce amount of malloc()s in the
 *          code while running.
 * input  : none
 * output : none
 * return : Size of a lock (OS dependant)
 ************************************************************************/
int meiGetSize(void)
{
    return sizeof(mei_t);
}


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
HMEI meiInitFrom(IN void* ptr)
{
    mei_t* mei = (mei_t *)ptr;

    if (mei != NULL)
        meiInitInternal(mei);

    return (HMEI)mei;
}


/************************************************************************
 * meiEndFrom
 * purpose: Deallocate a lock that was initialized by meiInitFrom().
 * input  : hMEI    - lock handle to use
 * output : none
 * return : Negative value on error
 ************************************************************************/
int meiEndFrom(IN HMEI hMEI)
{
    mei_t* mei = (mei_t *)hMEI;

    if (mei != NULL)
        meiEndInternal(mei);

    return 0;
}


/************************************************************************
 * meiInit
 * purpose: Initialize a mutex.
 *          This will cause a dynamic allocation to be invoked
 * input  : none
 * output : none
 * return : Mutex handle
 ************************************************************************/
HMEI meiInit(void)
{
    mei_t* mei = (mei_t *)malloc(sizeof(mei_t));
    return (meiInitFrom(mei));
}


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
int meiEnter(IN HMEI hMEI)
{
    mei_t* mei = (mei_t *)hMEI;

    if (hMEI == NULL)
        return RVERROR;

    msaPrintFormat(msa, "meiEnter(0x%p)", mei);

    meiEnterInternal(mei);

    return 0;
}


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
int meiExit(IN HMEI hMEI)
{
    mei_t* mei = (mei_t *)hMEI;

    if (hMEI == NULL)
        return RVERROR;

    msaPrintFormat(msa, "meiExit(0x%p)", mei);

    meiExitInternal(mei);

    return 0;
}


/************************************************************************
 * meiEnd
 * purpose: Deallocate a lock that was initialized by meiInit().
 * input  : hMEI    - lock handle to use
 * output : none
 * return : Negative value on error
 ************************************************************************/
int meiEnd(IN HMEI hMEI)
{
    if (meiEndFrom(hMEI) >= 0)
    {
        free((void *)hMEI);
        return 0;
    }
    return RVERROR;
}


#endif  /* NOTHREADS */



#ifndef RTP_NOLOCKS
/********************************************************************************************
* mutexGetAllocationSize
* purpose : This routine define the size of "mutex".
* input   : none.
* output  : none
* return  : int the size of "mutex".
********************************************************************************************/
int mutexGetAllocationSize(void)
{
    return sizeof(mei_t);
}


/********************************************************************************************
* mutexInitFrom
* purpose : This routine initializes a critical object in supplied space
* input   : ptr - pointer to the space to initialize critical object.
* output  : none
* return  : HMEI - handle of a mutex.
*******************************************************************************************/
HMEI mutexInitFrom(void *ptr)
{
    mei_t* mei = (mei_t *)ptr;

    if (mei != NULL)
        meiInitInternal(mei);

    return (HMEI)ptr;
}



/********************************************************************************************
* mutexEnd
* purpose : This routine releases all resources used by an unowned
*           critical section object. 
* input   : mutexHandle - A mutex handle.
* output  : none.
* return  : none.
********************************************************************************************/
void mutexEnd(HMEI hMutex)
{
    mei_t* mei = (mei_t *)hMutex;

    if (mei != NULL)
        meiEndInternal(mei);

    return;
}


/*********************************************************************************************
* mutexLock
* purpose : Lock a mutex
* input   : mutexHandle    - The mutex handle.
* output  : none.
* return  : return 0.
*********************************************************************************************/
int mutexLock(HMEI hMutex)
{
    if (hMutex == NULL)
        return 0;

    meiEnterInternal((mei_t*)hMutex);

    return 0;
}


/*********************************************************************************************
* mutexUnlock
* purpose : Unlock a locked mutex
* input   : mutexHandle    - The handle to the mutex
* output  : none
* return  : return 0.
*********************************************************************************************/
int mutexUnlock(HMEI  hMutex)
{
    if (hMutex == NULL)
        return 0;

    meiExitInternal((mei_t*)hMutex);

    return 0;
}
#endif  /* RTP_NOLOCKS */


#ifdef __cplusplus
}
#endif
