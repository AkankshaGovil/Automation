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
 * This API provides minimal handling for threads, including the following:
 * 1. Thread IDs
 * 2. Thread Local Storage
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

#ifndef NOTHREADS
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <Threads.h>
#include <memfunc.h>
#include <tls.h>
#include <ti.h>


/* Static declaration of process handle that we are using  */


#ifndef NOTHREADS

/* Number of opened threads */
static int tlsNumThreads = 0;


/* Index associated with new attached threads */
int tlsIndex = 0;


/********************************************************************************************
 * ThreadLocalStorage key that the stack uses for its own purposes.
 * This key is allocated per process that uses the DLL. It is actually a key index to a process's
 * location in which pointers to the process's threads local storage is kept. When a running thread
 * wants to allocate memory for its "thread local storage" it uses a simple malloc to allocate
 * memory an dthen using the process key set it in the process location.
 * When a running thread wants an access to its "thread local storage" it can ask the OS using the process
 * key to get a pointer to its TLS.
 ********************************************************************************************/
pthread_key_t tlsKey;


#endif  /* NOTHREADS */



#ifndef NOTHREADS
/********************************************************************************************
 * thread_CheckExit
 * purpose : Make sure thread is exited only after TIMER_End is called for that thread
 *           This function is only a double-checker which returns RV_ERROR on the log
 * input   : ptr        - Pointer to the thread's local storage (RA element id)
 * output  : none
 * return  : none
 ********************************************************************************************/
static void thread_CheckExit(IN void* ptr)
{
    THREAD_LocalStorage*    tls;
    UINT32                  i;

    /* Call all the exit functions for this thread and kill it nicely */
    tls = (THREAD_LocalStorage *)ptr;
    if (tls != NULL)
    {
        i = 0;

        for (i = tls->numFunctions; i > 0; i--)
            tls->exitFunctions[i-1].func(tls->exitFunctions[i-1].context);

        /* Deallocate the Thread-Local-Storage information we've got */
        free(tls);

        pthread_setspecific(tlsKey, NULL);
    }

    tlsNumThreads--;
    if (tlsNumThreads == 0)
        pthread_key_delete(tlsKey);
}


#endif


/********************************************************************************************
 * RvH323ThreadGetIndex
 * purpose : Returns the index of the running thread
 * input   : none
 * output  : none
 * return  : Index of the running thread
 ********************************************************************************************/
int RVAPI RVCALLCONV RvH323ThreadGetIndex(void)
{
    THREAD_LocalStorage* tls = (THREAD_LocalStorage *)RvH323ThreadGetHandle();
    return tls->index;
}


/********************************************************************************************
 * RvH323ThreadGetThreadId
 * purpose : Returns the thread ID of the given thread handle.
 *           The returned value is the OS specific ID
 * input   : none
 * output  : none
 * return  : OS specific thread ID
 ********************************************************************************************/
int RVAPI RVCALLCONV RvH323ThreadGetThreadId(IN RvH323ThreadHandle threadId)
{
    THREAD_LocalStorage* tls = (THREAD_LocalStorage *)threadId;
    return tls->osThreadId;
}


/********************************************************************************************
 * RvH323ThreadGetHandle
 * purpose : This routine is used by the running thread to get a pointer to its TLS ( Thread local
 *           stotage ).
 *           When a new thread calls this routine at the first time ( The returned pointer to the TLS is NULL )
 *           this routine is resposible to allocate memory for the thread local storage and to initialize the
 *           partameters in it.
 * input   : none
 * output  : none
 * return  : A handle to the thread local storage.
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RvH323ThreadHandle RVAPI RVCALLCONV RvH323ThreadGetHandle(void)
{
#ifdef NOTHREADS
    static THREAD_LocalStorage* tls = NULL;
    if (tls == NULL)
    {
        tls = (THREAD_LocalStorage *)memfAllocNoLog(sizeof(THREAD_LocalStorage));
        memfReport(tls, sizeof(THREAD_LocalStorage), __FILE__, __LINE__);
        memset(tls, 0, sizeof(THREAD_LocalStorage));
    }

#else
    THREAD_LocalStorage*    tls;

    if (tlsNumThreads == 0)
    {
        /* Allocate a Thread-Local-Storage key */
        if (pthread_key_create(&tlsKey, thread_CheckExit) != 0)
            return NULL;
    }

    /* The handle we return is actualy the thread-local-storage pointer we use */
    tls = (THREAD_LocalStorage *)pthread_getspecific(tlsKey);
    if (tls == NULL)
    {
        /* We have to create such allocation for the current thread */
        tls = (THREAD_LocalStorage *)memfAllocNoLog(sizeof(THREAD_LocalStorage));
        memset(tls, 0, sizeof(THREAD_LocalStorage));
        tls->index = tlsIndex++;
        tls->osThreadId = (int)pthread_self();

        if (pthread_setspecific(tlsKey, (void *)tls) != 0)
        {
            /* Error setting TLS information */
            return NULL;
        }

        tlsNumThreads++;
        memfReport(tls, sizeof(THREAD_LocalStorage), __FILE__, __LINE__);

        /* for random function, set a new seed for the current thread */
#ifndef _MTS_
        srand(timerGetTimeInMilliseconds());
#endif
    }
#endif  /* NOTHREADS */

    return (RvH323ThreadHandle)tls;
}





#ifdef __cplusplus
}
#endif


