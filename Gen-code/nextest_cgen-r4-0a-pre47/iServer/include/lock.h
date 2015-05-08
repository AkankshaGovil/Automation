#ifndef _lock_h_
#define _lock_h_

typedef struct 
{
	pthread_mutex_t lock;
	pthread_mutex_t lockint;

	int 			lockcount;
	pthread_t		threadid;

	int				pid;
} Lock;

Lock * LockAlloc(void);
void LockFree(Lock *lock);
int LockInit(Lock * l, int pshared);
int LockGetLock(Lock * l, int mode, int block);
int LockReleaseLock(Lock *l);
void LockDestroy(Lock *l);

typedef struct
{
	pthread_mutex_t mutex;
	pthread_cond_t 	cond;
	int				value;
} cvtuple;

int CVInit(cvtuple *cv);

#endif /* _lock_h_ */
