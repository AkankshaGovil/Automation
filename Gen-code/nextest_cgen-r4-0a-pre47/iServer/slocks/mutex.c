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

/* Mutexes */

#ifndef USE_SLOCKS

int
spthread_init()
{
	return 0;
}

int
spthread_exit()
{
	return 0;
}

#else

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include "internals.h"
#include "spthread.h"
#include "spinlock.h"
#include "restart.h"

static void spthread_handle_sigrestart(int sig)
{
  	spthread_descr self;
  	self = sthread_self();
  	THREAD_SETMEM(self, p_signal, sig);
}

int
spthread_init()
{
  	struct sigaction sa;
  	sigset_t mask;

	__spthread_main_thread = 
		(spthread_descr)SHM_Malloc(sizeof(struct _spthread_descr_struct));

	if ( __spthread_main_thread == NULL)
	{
		return -1;
	}

	/* Initialize it */
	memset(__spthread_main_thread, 0, sizeof(struct _spthread_descr_struct));
	__spthread_main_thread->p_nextlive = __spthread_main_thread->p_prevlive = 
		__spthread_main_thread->p_self = __spthread_main_thread;

	__spthread_main_thread->p_pid = getpid();
	__spthread_main_thread->p_errnop = &errno;

#ifdef dangerous_if_spthread_handle_sigrestart_is_sigusr1_or_sigusr2

  	sa.sa_handler = (__sighandler_t) spthread_handle_sigrestart;
  	sigemptyset(&sa.sa_mask);
  	sa.sa_flags = 0;
  	sigaction(__spthread_sig_restart, &sa, NULL);


  	/* Initially, block __spthread_sig_restart. Will be unblocked on demand. */
  	sigemptyset(&mask);
  	sigaddset(&mask, __spthread_sig_restart);
  	sigprocmask(SIG_BLOCK, &mask, NULL);
#endif

	return 0;
}

int
spthread_exit()
{
	if (__spthread_main_thread != NULL)
	{
		SHM_Free(__spthread_main_thread);
		__spthread_main_thread = NULL;
	}

	return 0;
}

int spthread_mutex_init(spthread_mutex_t * mutex,
                       const spthread_mutexattr_t * mutex_attr)
{
  __spthread_init_lock(&mutex->__m_lock);
  mutex->__m_kind =
    mutex_attr == NULL ? PTHREAD_MUTEX_FAST_NP : mutex_attr->__mutexkind;
  mutex->__m_count = 0;
  mutex->__m_owner = NULL;
  return 0;
}

int spthread_mutex_destroy(spthread_mutex_t * mutex)
{
  if (mutex->__m_lock.__status != 0) return EBUSY;
  return 0;
}

int spthread_mutex_trylock(spthread_mutex_t * mutex)
{
  spthread_descr self;
  int retcode;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    retcode = __spthread_trylock(&mutex->__m_lock);
    return retcode;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = sthread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    retcode = __spthread_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = self;
      mutex->__m_count = 0;
    }
    return retcode;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    retcode = __spthread_trylock(&mutex->__m_lock);
    if (retcode == 0) {
      mutex->__m_owner = sthread_self();
    }
    return retcode;
  default:
    return EINVAL;
  }
}

int spthread_mutex_lock(spthread_mutex_t * mutex)
{
  spthread_descr self;

  switch(mutex->__m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    __spthread_lock(&mutex->__m_lock, NULL);
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    self = sthread_self();
    if (mutex->__m_owner == self) {
      mutex->__m_count++;
      return 0;
    }
    __spthread_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    mutex->__m_count = 0;
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    self = sthread_self();
    if (mutex->__m_owner == self) return EDEADLK;
    __spthread_lock(&mutex->__m_lock, self);
    mutex->__m_owner = self;
    return 0;
  default:
    return EINVAL;
  }
}

int spthread_mutex_unlock(spthread_mutex_t * mutex)
{
  switch (mutex->__m_kind) {
  case PTHREAD_MUTEX_FAST_NP:
    __spthread_unlock(&mutex->__m_lock);
    return 0;
  case PTHREAD_MUTEX_RECURSIVE_NP:
    if (mutex->__m_count > 0) {
      mutex->__m_count--;
      return 0;
    }
    mutex->__m_owner = NULL;
    __spthread_unlock(&mutex->__m_lock);
    return 0;
  case PTHREAD_MUTEX_ERRORCHECK_NP:
    if (mutex->__m_owner != sthread_self() || mutex->__m_lock.__status == 0)
      return EPERM;
    mutex->__m_owner = NULL;
    __spthread_unlock(&mutex->__m_lock);
    return 0;
  default:
    return EINVAL;
  }
}

int __spthread_mutexattr_init(spthread_mutexattr_t *attr)
{
  attr->__mutexkind = PTHREAD_MUTEX_FAST_NP;
  return 0;
}

#endif
