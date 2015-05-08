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

#ifndef _SPTHREAD_H
#define _SPTHREAD_H	1

/* Whatever is applicable to both cases, lives up here */
#define spthread_mutexattr_init(x) pthread_mutexattr_init(x)
#define spthread_mutexattr_setpshared(x,y) pthread_mutexattr_setpshared(x,y)

#ifdef USE_SLOCKS
#include <features.h>

#include <sched.h>
#include <time.h>

#define __need_sigset_t

#include <signal.h>
#include "internals.h"
#include "spthreadtypes.h"

#else	/* USE_SLOCKS */
#include <pthread.h>
#define spthread_mutex_init(x,y) pthread_mutex_init(x,y)
#define spthread_mutex_destroy(x) pthread_mutex_destroy(x)
#define spthread_mutex_trylock(x) pthread_mutex_trylock(x)
#define spthread_mutex_lock(x) pthread_mutex_lock(x)
#define spthread_mutex_unlock(x) pthread_mutex_unlock(x)
typedef pthread_mutex_t spthread_mutex_t;
typedef pthread_mutexattr_t spthread_mutexattr_t;


#endif	/* USE_SLOCKS */

#endif	/* pthread.h */
