/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
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

#ifdef USE_SLOCKS

/* Internal locks */

#include <errno.h>
#include <sched.h>
#include <time.h>
#include "internals.h"
#include "spthread.h"
#include "spinlock.h"
#include "restart.h"

/* The status field of a fastlock has the following meaning:
     0: fastlock is free
     1: fastlock is taken, no thread is waiting on it
  ADDR: fastlock is taken, ADDR is address of thread descriptor for
        first waiting thread, other waiting threads are linked via
        their p_nextlock field.
   The waiting list is not sorted by priority order.
   Actually, we always insert at top of list (sole insertion mode
   that can be performed without locking).
   For __spthread_unlock, we perform a linear search in the list
   to find the highest-priority, oldest waiting thread.
   This is safe because there are no concurrent __spthread_unlock
   operations -- only the thread that locked the mutex can unlock it. */

void  __spthread_lock(struct _spthread_fastlock * lock,
				      spthread_descr self)
{
  long oldstatus, newstatus;

  do {
    oldstatus = lock->__status;
    if (oldstatus == 0) {
      newstatus = 1;
    } else {
      if (self == NULL)
	self = sthread_self();
      newstatus = (long) self;
    }
    if (self != NULL) {
      ASSERT(self->p_nextlock == NULL);
      THREAD_SETMEM(self, p_nextlock, (spthread_descr) oldstatus);
    }
  } while(! l_compare_and_swap(&lock->__status, oldstatus, newstatus,
                             &lock->__spinlock));
  if (oldstatus != 0) l_suspend(self);
}

void  __spthread_unlock(struct _spthread_fastlock * lock)
{
  long oldstatus;
  spthread_descr thr, * ptr, * maxptr;
  int maxprio;

again:
  oldstatus = lock->__status;
  if (oldstatus == 0 || oldstatus == 1) {
    /* No threads are waiting for this lock.  Please note that we also
       enter this case if the lock is not taken at all.  If this wouldn't
       be done here we would crash further down.  */
    if (! l_compare_and_swap(&lock->__status, oldstatus, 0, &lock->__spinlock))
      goto again;
    return;
  }
  /* Find thread in waiting queue with maximal priority */
  ptr = (spthread_descr *) &lock->__status;
  thr = (spthread_descr) oldstatus;
  maxprio = 0;
  maxptr = ptr;
  while (thr != (spthread_descr) 1) {
    if (thr->p_priority >= maxprio) {
      maxptr = ptr;
      maxprio = thr->p_priority;
    }
    ptr = &(thr->p_nextlock);
    thr = *ptr;
  }
  /* Remove max prio thread from waiting list. */
  if (maxptr == (spthread_descr *) &lock->__status) {
    /* If max prio thread is at head, remove it with compare-and-swap
       to guard against concurrent lock operation */
    thr = (spthread_descr) oldstatus;
    if (! l_compare_and_swap(&lock->__status,
                           oldstatus, (long)(thr->p_nextlock),
                           &lock->__spinlock))
      goto again;
  } else {
    /* No risk of concurrent access, remove max prio thread normally */
    thr = *maxptr;
    *maxptr = thr->p_nextlock;
  }
  /* Wake up the selected waiting thread */
  thr->p_nextlock = NULL;
  l_restart(thr);
}

#endif
