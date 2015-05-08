/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/********************************************************************************************
 *                                Threads.h
 *
 * This module provides an API for threads handling with a common interface for all operating
 * systems.
 * It handles the concept of thread local storage as implemented in the stack.
 * Each thread holds a structure with the necessary thread-specific information.
 * When compiled with NOTHREADS, it gives the notion of the use of threads altough it doesn't
 * use them at all.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  25-Sep-2000
 *
 ********************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "memfunc.h"
#include "mei.h"
#include "tls.h"


/********************************************************************************************
 *
 *                                  Private variables
 *
 ********************************************************************************************/

static HMEI globalMutex = NULL;


#ifdef _NUCLEUS
extern RVAPI THREAD_LocalStorage* RVCALLCONV RV_THREAD_Storage(IN RvH323ThreadHandle threadId);
#else
#define RV_THREAD_Storage(a) (THREAD_LocalStorage*)((a==NULL) ? RvH323ThreadGetHandle() : (a))
#endif




/********************************************************************************************
 *
 *                                  Public functions
 *
 ********************************************************************************************/


/********************************************************************************************
 * H323ThreadSetExitFunction
 * purpose : Set a thread's exit function.
 *           Each exit routine that will be register ( to a given thread ) will be called when the thread
 *           will be closed. Normally such an exist routine should include code that free resources.
 *           Pointers to the exist routines and their parameters are kept on the thread local storage.
 *           This can be called several times. All of the exit functions will be called in
 *           the order of their setting for the specific thread on its exit.
 *           In the current implementation both the LI and the Timer routines register exist routines.
 * input   : threadId   - Id of the thread we want to use
 *           callback   - Pointer to the exist function that is register.
 *           context    - Context to use when calling this function ( parameter that will be used to call
 *                        the exist function
 * output  : none
 * return  : Non-negative on success
 *           RVERROR for a thread handle which is invalid (NULL)
 *           RESOURCES_PROBLEM on failure (too many exit functions for the thread)
 ********************************************************************************************/
int H323ThreadSetExitFunction(
    IN RvH323ThreadHandle   threadId,
    IN THREAD_ExitCallback  callback,
    IN void*                context)
{
    THREAD_LocalStorage*    tls;

    if (threadId == NULL)
        return RVERROR;

    tls = (THREAD_LocalStorage *)RV_THREAD_Storage(threadId);

    if (tls->numFunctions == THREAD_MAX_EXIT_FUNCS)
        return RESOURCES_PROBLEM;

    tls->exitFunctions[tls->numFunctions].func = callback;
    tls->exitFunctions[tls->numFunctions].context = context;

    tls->numFunctions++;
    return 0;
}


/********************************************************************************************
 * THREAD_Exit
 * purpose : Function that is called to notify that a thread was closed and is not available
 *           anymore. It is called automatically by the Threads package in low.
 *           This funcition is responsible for calling all the exit functions that are set
 *           for the given thread.
 * input   : threadId   - Handle of the thread to exit
 * output  : none
 * return  : none
 ********************************************************************************************/
void THREAD_Exit(IN RvH323ThreadHandle threadId)
{
    THREAD_LocalStorage* tls;
    INT32 i;

    tls = (THREAD_LocalStorage *)RV_THREAD_Storage(threadId);
    if (tls == NULL) return;

    for (i = tls->numFunctions-1; i >= 0; i--)
        tls->exitFunctions[i].func(tls->exitFunctions[i].context);
}



/********************************************************************************************
 * THREAD_GetLogLocalStorage
 * purpose : Get LOG specific local storage information
 *           The returned struct can also be used for setting values.
 * input   : threadId   - Handle of the thread to use
 * output  : none
 * return  : LOG specific information
 *           NULL on failure
 ********************************************************************************************/
THREAD_LogLocalStorage* THREAD_GetLogLocalStorage(IN RvH323ThreadHandle threadId)
{
    THREAD_LocalStorage*    tls;
    tls = RV_THREAD_Storage(threadId);
    /*

    if (threadId == NULL)
        tls = (THREAD_LocalStorage *)RvH323ThreadGetHandle();
    else
        tls = (THREAD_LocalStorage *)threadId;
    */
    if (tls != NULL)
        return &(tls->log);
    else
        return NULL;
}



char* THREAD_GetMSABuffer(IN RvH323ThreadHandle threadId)
{
    THREAD_LocalStorage*    tls;
    tls = RV_THREAD_Storage(threadId);
    /*
    if (threadId == NULL)
        tls = (THREAD_LocalStorage *)RvH323ThreadGetHandle();
    else
        tls = (THREAD_LocalStorage *)threadId;
    */
    if (tls != NULL)
        return tls->msaBuf;
    else
        return NULL;
}

/********************************************************************************************
 * threadFreeThreadBuffer
 * An exit callback of a specific thread for CODER.
 * This function frees the allocation of the encode/decode buffer
 * context  - The pointer to the buffer to free
 ********************************************************************************************/
void RVCALLCONV threadFreeThreadBuffer(IN void*   context)
{
    free(context);
}


/********************************************************************************************
 * THREAD_GetLocalStorage
 * purpose : Get specific local storage information
 *           The returned struct can also be used for setting values.
 * input   : threadId   - Handle of the thread to use
 *           type       - Type of local storage to get
 *           size       - Size of allocation to create if there's none
 * output  : none
 * return  : Specific information struct on success
 *           NULL on failure
 ********************************************************************************************/
void* THREAD_GetLocalStorage(
    IN RvH323ThreadHandle   threadId,
    IN tlsInterface         type,
    IN UINT32               size)
{
    THREAD_LocalStorage*    tls;
    tls = RV_THREAD_Storage(threadId);
    /*if (threadId == NULL)
        tls = (THREAD_LocalStorage *)RvH323ThreadGetHandle();
    else
        tls = (THREAD_LocalStorage *)threadId;
    */
    if (tls != NULL)
    {
        if ((tls->interfaces[type] == NULL) && (size > 0))
        {
            /* Allocate the interface on demand */
            tls->interfaces[type] = (void *)memfAllocNoLog((int)size);
            memset(tls->interfaces[type], 0, size);
            memfReport(tls->interfaces[type], (int)size, __FILE__, __LINE__);
            threadId = (!threadId)? RvH323ThreadGetHandle():threadId;
            H323ThreadSetExitFunction(threadId, threadFreeThreadBuffer, tls->interfaces[type]);
        }
        return tls->interfaces[type];
    }
    else
        return NULL;
}




/********************************************************************************************
 * tlsStartUp
 * purpose : Initialize a global mutex for use later on
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsStartUp(void)
{
    if (globalMutex == NULL)
    {
        /* Create a mutex */
        globalMutex = meiInit();

        /* Make sure we've got a key in the tls mechanism of the OS */
        RvH323ThreadGetHandle();
    }
}

/********************************************************************************************
 * tlsShutDown
 * purpose : Frees a global mutex for use later on
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsShutDown(void)
{
    if (globalMutex != NULL)
    {
        /* Create a mutex */
        meiEnd(globalMutex);
    }
}

/********************************************************************************************
 * tlsLockAll
 * purpose : Lock the global mutex.
 *           This function should be used when we need to initialize static parameters that
 *           are shared by several stack instances.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsLockAll(void)
{
    if (globalMutex != NULL)
        meiEnter(globalMutex);
}


/********************************************************************************************
 * tlsUnlockAll
 * purpose : Unlock the global mutex.
 *           This function should be used when we need to initialize static parameters that
 *           are shared by several stack instances.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsUnlockAll(void)
{
    if (globalMutex != NULL)
        meiExit(globalMutex);
}




#ifdef __cplusplus
}
#endif

