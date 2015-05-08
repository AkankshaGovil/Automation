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

#include <signal.h>

/* Primitives for controlling thread execution */
#define USE_QUEUE_LOCKS

static inline void l_restart(spthread_descr th)
{
#ifdef USE_QUEUE_LOCKS
	WakeupPid(th->p_pid);
#else	
  kill(th->p_pid, __spthread_sig_restart);
#endif
}

static inline void l_suspend(spthread_descr self)
{
  sigset_t mask;

#ifdef USE_QUEUE_LOCKS
	WaitForMsg(self->p_pid);
#else
  sigprocmask(SIG_SETMASK, NULL, &mask); /* Get current signal mask */
  sigdelset(&mask, __spthread_sig_restart); /* Unblock the restart signal */
  do {
    self->p_signal = 0;
    sigsuspend(&mask);                   /* Wait for signal */
  } while (self->p_signal !=__spthread_sig_restart );
#endif
}
