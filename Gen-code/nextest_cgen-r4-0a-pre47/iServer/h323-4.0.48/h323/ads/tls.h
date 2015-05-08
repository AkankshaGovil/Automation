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
 * Services:
 * 1. Thread-Local-Storage for the stack's modules.
 * 2. Global locking facilities
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  25-Sep-2000
 *
 ********************************************************************************************/

#ifndef TLS_H
#define TLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rvinternal.h>
#include <Threads_API.h>




/* Maximum number of exit callbacks for a running thread on the system */
#define THREAD_MAX_EXIT_FUNCS (18)


/********************************************************************************************
 * THREAD_ExitCallback
 * An exit callback of a specific thread.
 * context  - The context given in H323ThreadSetExitFunction()
 ********************************************************************************************/
typedef void (RVCALLCONV *THREAD_ExitCallback) (IN void*   context);


/********************************************************************************************
 * THREAD_ExitFuncInfo struct
 * Holds information about a single exit callback function
 * func     - Function to execute when the thread exists
 * context  - Context to use for function
 ********************************************************************************************/
typedef struct
{
    THREAD_ExitCallback func;
    void*               context;
} THREAD_ExitFuncInfo;


/********************************************************************************************
 * Description of THREAD_LogLocalStorage
 * This structure holds thread related information for LOG
 * printedMessage   - Buffer holding a message being formatted for logging
 ********************************************************************************************/
typedef struct
{
    char    printedMessage[1024];
} THREAD_LogLocalStorage;



typedef enum
{
    tlsIntrSeli,
    tlsIntrLi,
    tlsIntrMti,
    tlsIntrCoder,
    tlsIntrTrans,
    tlsIntrCm,
    tlsIntrCid,
    tlsIntrLast
} tlsInterface;


/********************************************************************************************
 * THREAD_LocalStorage struct
 * Holds information about a single thread
 * index            - Index of this thread
 * osThreadId       - OS specific thread id
 * log              - LOG interface storage
 * msaBuf           - Old logger interface storage
 *
 * interfaces       - General interfaces for local storage
 *                    This is used for interfaces with variable size or OS specific
 *
 * numFunctions     - Number of exit functions for this thread
 * exitFunctions    - Exit functions for this thread
 ********************************************************************************************/
typedef struct
{
    int                         index;
    int                         osThreadId;
    THREAD_LogLocalStorage      log;
    char                        msaBuf[1024]; /* todo: remove when msaPrintFormat() is removed from code */

    void*                       interfaces[tlsIntrLast];
    char                        reserved[100];
    INT32                       numFunctions;
    THREAD_ExitFuncInfo         exitFunctions[THREAD_MAX_EXIT_FUNCS];
} THREAD_LocalStorage;




/********************************************************************************************
 *
 *                                  THREAD LOCAL STORAGE
 *
 ********************************************************************************************/


/********************************************************************************************
 * H323ThreadSetExitFunction
 * purpose : Set a thread's exit function.
 *           This can be called several times. All of the exit functions will be called in
 *           the order of their setting for the specific thread on its exit.
 *           Each module that need to do something when the thread exist will register its exit function.
 *           Currently both LI and Timers register their own function for thread exist.
 * input   : threadId   - Id of the thread we want to use
 *           callback   - Callback function to set
 *           context    - Context to use when calling this function
 * output  : none
 * return  : Non-negative on success
 *           RVERROR for a thread handle which is invalid (NULL)
 *           RESOURCES_PROBLEM on failure (too many exit functions for the thread)
 ********************************************************************************************/
int H323ThreadSetExitFunction(
    IN RvH323ThreadHandle   threadId,
    IN THREAD_ExitCallback  callback,
    IN void*                context);



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
void THREAD_Exit(IN RvH323ThreadHandle threadId);


/********************************************************************************************
 * THREAD_GetLogLocalStorage
 * purpose : Get LOG specific local storage information
 *           The returned struct can also be used for setting values.
 * input   : threadId   - Handle of the thread to use
 * output  : none
 * return  : LOG specific information
 *           NULL on failure
 ********************************************************************************************/
THREAD_LogLocalStorage* THREAD_GetLogLocalStorage(IN RvH323ThreadHandle threadId);


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
    IN UINT32               size);



char* THREAD_GetMSABuffer(IN RvH323ThreadHandle threadId);





/********************************************************************************************
 *
 *                                  GLOBAL LOCKING
 *
 ********************************************************************************************/


/********************************************************************************************
 * tlsStartUp
 * purpose : Initialize a global mutex for use later on
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsStartUp(void);


/********************************************************************************************
 * tlsShutDown
 * purpose : Frees a global mutex for use later on
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsShutDown(void);

/********************************************************************************************
 * tlsLockAll
 * purpose : Lock the global mutex.
 *           This function should be used when we need to initialize static parameters that
 *           are shared by several stack instances.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsLockAll(void);


/********************************************************************************************
 * tlsUnlockAll
 * purpose : Unlock the global mutex.
 *           This function should be used when we need to initialize static parameters that
 *           are shared by several stack instances.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void tlsUnlockAll(void);



#ifdef __cplusplus
}
#endif

#endif

