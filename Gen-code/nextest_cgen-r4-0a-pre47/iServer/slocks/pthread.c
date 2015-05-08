/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Thread creation, initialization, and basic low-level routines */

#ifdef USE_SLOCKS

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "internals.h"
#include "spthread.h"
#include "spinlock.h"
#include "restart.h"

/* Originally, this is the initial thread, but this changes after fork() */

spthread_descr __spthread_main_thread = NULL;

/* Limit between the stack of the initial thread (above) and the
   stacks of other threads (below). Aligned on a STACK_SIZE boundary. */

/* File descriptor for sending requests to the thread manager. */
/* Initially -1, meaning that the thread manager is not running. */

int __spthread_manager_request = -1;

/* Other end of the pipe for sending requests to the thread manager. */

int __spthread_manager_reader;

/* Limits of the thread manager stack */

char *__spthread_manager_thread_bos = NULL;
char *__spthread_manager_thread_tos = NULL;

/* For process-wide exit() */

int __spthread_exit_requested = 0;
int __spthread_exit_code = 0;

/* Forward declarations */

/* Signal numbers used for the communication.
   In these variables we keep track of the used variables.  If the
   platform does not support any real-time signals we will define the
   values to some unreasonable value which will signal failing of all
   the functions below.  */
int __spthread_sig_restart = SIGUNUSED;

#endif
