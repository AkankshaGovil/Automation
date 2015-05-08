#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "srvrlog.h"
#include "lock.h"
#include <malloc.h>
#include "sys/types.h"
#include "unistd.h"
#include "stdio.h"
#include "nxosd.h"

static pid_t MyPid=0;

Lock *
LockAlloc(void)
{
	return (Lock *)malloc(sizeof(Lock));
}

void
LockFree(Lock *lock)
{
	if (lock) free(lock);
}

int
LockInit(Lock *lock, int pshared)
{
	pthread_mutexattr_t mattr;
	
	MyPid = getpid();

	memset(lock, 0, sizeof(Lock));

	pthread_mutexattr_init(&mattr);
	if (pshared)
	{
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	}
	pthread_mutex_init(&lock->lock, &mattr);	
	pthread_mutex_init(&lock->lockint, &mattr);	

	pthread_mutexattr_destroy(&mattr);

	return 0;
}

void
LockDestroy (Lock *lock)
{
  pthread_mutex_destroy(&lock->lock);
  pthread_mutex_destroy(&lock->lockint);
  memset(lock, 0, sizeof(Lock));
}

int
LockGetLock(Lock * l, int mode, int block)
{
	char fn[] = "LockGetLock():";
	int error, ownlock = 0;
	pthread_t selfid;

	selfid = pthread_self();
	if (MyPid == 0)		// In case we (cli, iview, etc.) didn't init the lock
	{
		MyPid = getpid();
	}

	// Try to acquire the lock

	if (( error = pthread_mutex_trylock(&l->lock)) == EBUSY )
	{
		// lock the lock's internal access mutex

		pthread_mutex_lock(&l->lockint);

		// Is the lock owned by the same thread in the
		// same process ?

		if (pthread_equal(l->threadid, selfid) && (l->pid == MyPid))
		{
			// Yes, we already own the lock increment
			// the lock reference counter

			l->lockcount++;
			ownlock = 1;
		}

		// unlock the lock's internal access mutex

		pthread_mutex_unlock(&l->lockint);

		if (!ownlock) {
			// We don't own the lock, so wait for it to be freed
			pthread_mutex_lock(&l->lock);
		}
	}
	else if ( error != 0 )
	{
        NETERROR(   MDEF,
                    ("%s mutex_trylock(lock) - Error - errno %d - %s\n",
                    fn,
                    error,
                    strerror(error) ));
	}
	
	// We have the lock
	pthread_mutex_lock(&l->lockint);
	l->pid = MyPid;
	l->threadid = selfid;
	if (ownlock == 0) {
	   // this call did the l->lock locking
	   l->lockcount = 1;
	}
	pthread_mutex_unlock(&l->lockint);

	return 0;
}

int
LockReleaseLock(Lock *l)
{
	char fn[] = "LockReleaseLock():";
	int error, count,result;
	pthread_t selfid;

	selfid = pthread_self();

	if (!(pthread_equal(l->threadid, selfid) && (l->pid == MyPid)))
	{
		NETERROR(MINIT,
			("%s Lock does not belong to this thread, won't Free!!"
			"owner =  %lu/%lu self = %lu/%lu \n",
			fn,ULONG_FMT(l->threadid),ULONG_FMT(l->pid),ULONG_FMT(selfid),ULONG_FMT(MyPid)));
#ifndef NETOID_LINUX
		print_stack_trace();
#endif
		return 0;
	}

	pthread_mutex_lock(&l->lockint);

	if (l->lockcount > 0)
	{
		l->lockcount --;
	}

	if ((count = l->lockcount) == 0)
	{
		l->pid = 0;
		l->threadid = 0;
	}

	pthread_mutex_unlock(&l->lockint);

	if (count > 0)
	{
		goto _return;
	}

	pthread_mutex_unlock(&l->lock);

_return:
	
	return 0;
}

int
CVInit(cvtuple *cv)
{
	pthread_mutex_init(&cv->mutex, NULL);
	pthread_cond_init(&cv->cond, NULL);
	cv->value = 0;
	return(0);
}

int
CVSignal(cvtuple *cv)
{
	char fn[] = "CVSignal():";
	int status;

	if ((status = pthread_cond_signal(&cv->cond)) != 0)
	{
		NETERROR(MINIT, ("%s pthread_cond_signal error %d\n", fn, status));
	}

	return 0;
}

// dummy wait - no condition
int
CVWait(cvtuple *cv)
{
	char fn[] = "CVWait():";
	int status, rc;

	if ((rc = pthread_mutex_lock(&cv->mutex)) != 0)
	{
		NETERROR(MINIT, ("%s pthread_mutex_lock error %d\n", 
			fn, rc));
		return -1;
	}

	status = pthread_cond_wait(&cv->cond, &cv->mutex);

	if (status == 0)
	{
		//
        	// This may be a spurious wakeup. Let the
        	// condition test decide
        	// 

	}

	if ((rc = pthread_mutex_unlock(&cv->mutex)) != 0)
	{
		NETERROR(MINIT, 
			("%s pthread_mutex_unlock error %d\n", fn, status));
		return -1;
	}

	return status;
}
