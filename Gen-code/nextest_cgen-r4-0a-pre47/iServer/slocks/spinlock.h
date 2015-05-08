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

static inline int l_compare_and_swap(long * ptr, long oldval, long newval,
                                   int * spinlock)
{
    return __l_compare_and_swap(ptr, oldval, newval);
}

/* Internal locks */

extern void __spthread_lock(struct _spthread_fastlock * lock,
					     spthread_descr self);
extern void __spthread_unlock(struct _spthread_fastlock *lock);

static inline void __spthread_init_lock(struct _spthread_fastlock * lock)
{
  lock->__status = 0;
  lock->__spinlock = 0;
}

static inline int __spthread_trylock (struct _spthread_fastlock * lock)
{
  long oldstatus;

  do {
    oldstatus = lock->__status;
    if (oldstatus != 0) return EBUSY;
  } while(! l_compare_and_swap(&lock->__status, 0, 1, &lock->__spinlock));
  return 0;
}

#define LOCK_INITIALIZER {0, 0}
