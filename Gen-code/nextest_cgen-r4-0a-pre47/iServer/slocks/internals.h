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

#ifndef _INTERNALS_H_
#define _INTERNALS_H_

/* Internal data structures */

/* Includes */

#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "pt-machine.h"

#ifndef THREAD_GETMEM
# define THREAD_GETMEM(descr, member) descr->member
#endif
#ifndef THREAD_GETMEM_NC
# define THREAD_GETMEM_NC(descr, member) descr->member
#endif
#ifndef THREAD_SETMEM
# define THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif
#ifndef THREAD_SETMEM_NC
# define THREAD_SETMEM_NC(descr, member, value) descr->member = (value)
#endif

/* The type of thread descriptors */

typedef struct _spthread_descr_struct * spthread_descr;

struct _spthread_descr_struct {
  spthread_descr p_nextlive, p_prevlive;
                                /* Double chaining of active threads */
  spthread_descr p_nextwaiting;  /* Next element in the queue holding the thr */
  spthread_descr p_nextlock;	/* can be on a queue and waiting on a lock */
  int p_pid;                    /* PID of Unix process */
  int p_priority;               /* Thread priority (== 0 if not realtime) */
  int p_signal;                 /* last signal received */
  char p_exited;                /* true if the assoc. process terminated */
  int * p_errnop;               /* pointer to used errno variable */
  int p_errno;                  /* error returned by last system call */
  int * p_h_errnop;             /* pointer to used h_errno variable */
  int p_h_errno;                /* error returned by last netdb function */
  char * p_in_sighandler;       /* stack address of sighandler, or NULL */
  char p_sigwaiting;            /* true if a sigwait() is in progress */
  spthread_descr p_self;		/* Pointer to this structure */
} __attribute__ ((aligned(32))); /* We need to align the structure so that
				    doubles are aligned properly.  This is 8
				    bytes on MIPS and 16 bytes on MIPS64.
				    32 bytes might give better cache
				    utilization.  */

/* Signals used for suspend/restart and for cancellation notification.  */

extern int __spthread_sig_restart;

/* Descriptor of the main thread */

extern spthread_descr __spthread_main_thread;

/* Pending request for a process-wide exit */

extern int __spthread_exit_requested, __spthread_exit_code;

/* Set to 1 by gdb if we're debugging */

extern volatile int __spthread_threads_debug;

/* Return the handle corresponding to a thread id */

/* Validate a thread handle. Must have acquired h->h_spinlock before. */

/* Fill in defaults left unspecified by pt-machine.h.  */

/* The max size of the thread stack segments.  If the default
   THREAD_SELF implementation is used, this must be a power of two and
   a multiple of PAGE_SIZE.  */
#ifndef STACK_SIZE
#define STACK_SIZE  (2 * 1024 * 1024)
#endif

/* Recover thread descriptor for the current thread */

static inline spthread_descr sthread_self (void) __attribute__ ((const));
static inline spthread_descr sthread_self (void)
{
    return __spthread_main_thread;
}

/* Max number of times we must spin on a spinlock calling sched_yield().
   After MAX_SPIN_COUNT iterations, we put the calling thread to sleep. */

#ifndef MAX_SPIN_COUNT
#define MAX_SPIN_COUNT 50
#endif

/* Duration of sleep (in nanoseconds) when we can't acquire a spinlock
   after MAX_SPIN_COUNT iterations of sched_yield().
   With the 2.0 and 2.1 kernels, this MUST BE > 2ms.
   (Otherwise the kernel does busy-waiting for realtime threads,
    giving other threads no chance to run.) */

#ifndef SPIN_SLEEP_DURATION
#define SPIN_SLEEP_DURATION 2000001
#endif

/* Debugging */

#ifdef DEBUG
#include <assert.h>
#define ASSERT assert
#define MSG __spthread_message
#else
#define ASSERT(x)
#define MSG(msg,arg...)
#endif

#endif /* _INTERNALS_H_ */
