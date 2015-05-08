/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/********************************************************************************************
 *                                Threads_API.h
 *
 * This module provides an API for threads handling with a common interface for all operating
 * systems.
 * It includes a single function which returns the handle of the current running thread.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  25-Sep-2000
 *
 ********************************************************************************************/

#ifndef _H323_THREADS_API_H
#define _H323_THREADS_API_H

#ifdef __cplusplus
extern "C" {
#endif


#include <rvcommon.h>



/* A thread handle - used to identify a thread */
DECLARE_OPAQUE(RvH323ThreadHandle);




/********************************************************************************************
 * RvH323ThreadGetHandle
 * purpose : Returns the handle of the running thread
 * input   : none
 * output  : none
 * return  : A thread handle
 *           NULL on failure (unrecognized thread)
 ********************************************************************************************/
RvH323ThreadHandle RVAPI RVCALLCONV RvH323ThreadGetHandle(void);


/********************************************************************************************
 * RvH323ThreadGetIndex
 * purpose : Returns the index of the running thread
 * input   : none
 * output  : none
 * return  : Index of the running thread
 ********************************************************************************************/
int RVAPI RVCALLCONV RvH323ThreadGetIndex(void);


/********************************************************************************************
 * RvH323ThreadGetThreadId
 * purpose : Returns the thread ID of the given thread handle.
 *           The returned value is the OS specific ID
 * input   : none
 * output  : none
 * return  : OS specific thread ID
 ********************************************************************************************/
int RVAPI RVCALLCONV RvH323ThreadGetThreadId(IN RvH323ThreadHandle threadId);





#ifdef __cplusplus
}
#endif

#endif

