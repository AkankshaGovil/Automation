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
 * The Windows version contains the DLLMain routine , This routine is called whenever a new process
 * or a new thread is attached to the core dll.
 *
 * The thread module allocates Thread local storage per each thread that uses the core. In the thread
 * local storage we keep information that is kept per thread for the LI ( networking and timer modules(.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  25-Sep-2000
 *
 ********************************************************************************************/

#ifndef THREADS_H
#define THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Threads_API.h"



#ifdef __cplusplus
}
#endif

#endif

