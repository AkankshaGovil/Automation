#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#include <srvrlog.h>
#include <string.h>
#include <unistd.h>
#ifndef NETOID_LINUX
#include <sys/lwp.h>
#include <sys/corectl.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/processor.h>
#include <sys/procset.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "nxosd.h"
#include "lock.h"
#include "list.h"
#include "thutils.h"
#include "lsconfig.h"
#include "common.h"

#ifdef NOTHRDLOG
#include "nodebuglog.h"
#endif

#include <malloc.h>

extern int xthreads;
int threadstack = 1024;

/* priocntl and processor_bind related structures and calls not to be used on 
Linux.
*/

#ifndef NETOID_LINUX
// for getting the cid
pcinfo_t tpcinfo;

// for doing PC_GETPARMS/SET
pcparms_t tpcparms;
#endif

static int setrtInitialized = 0;

typedef struct
{
	int			poolid;
	int 		id;

	pthread_t	tid;

	int 		status;		// 0 = asleep, 1 = awake

} ProgCPUStruct;

#define THREAD_POOLCLASS_MAX	5

typedef struct
{
	List 		progs;

	// max items allowable in this class, set to <=0 if max is infinite
	int 		maxitems;
	
	// all items in this queue must have the same deadline
	longlong_t	deadline;

	// stats
	int			missed;		// items which missed deadlines
	int			processed;	// total processed
	int			peakitems;	// peak number

	char		name[32];
} ThreadPoolClass;

#define THREAD_POOL_MAX	50

typedef struct
{

	ThreadPoolClass	pclass[THREAD_POOLCLASS_MAX];
	int				nclasses;

	cvtuple 		progcv;

	ProgCPUStruct 	*progCPU;	
	int 			progCPUSize;

	void			(*threadInitFn)();
	void			*threadInitFnArg;

	int 			notifyPipe[2];
	unsigned int 	wnotify;
	unsigned int	rnotify;

	char			name[32];

	int 			policy;
	int 			priority;

} ThreadPool;

static ThreadPool thread_pool[THREAD_POOL_MAX];
static int allocated_pools = 0;                // thread pools allocated

pthread_key_t isDispatchThread;
pthread_key_t threadIndex;

typedef struct
{
	pthread_attr_t 	attr;
	void 			*(*fn)(void*);
	void 			*arg;

	int 			policy;
	int 			priority;

	hrtime_t 		tstamp;		// deadline

} ThreadArg;

int
getMyThreadIndex ()
{
	int tid = pthread_self ();
	int i, j;
	int threadId;

	threadId = NX_POINTER_TO_INT(pthread_getspecific (threadIndex));
	NETDEBUG (MSIP, NETLOG_DEBUG4, ("Thread id : %d\n", 
					threadId));
	return threadId;
}

static int
_mainThread(void)
{
	return 0;
}

int (*identMain)() = _mainThread;

ThreadArg *
ThreadNewArg(void)
{
	ThreadArg *targ;

	targ = (ThreadArg *)malloc(sizeof(ThreadArg));
	memset(targ, 0, sizeof(ThreadArg));

	return targ;
}

#define ThreadFreeArg(_x_)	(free(_x_))
/*
 Real time priotity scheduling functions not to be used on Linux
 */
#ifndef NETOID_LINUX
// Sets to default RT priority + prio
int
ThreadSetRTPrio(int prio)
{
	char fn[] = "ThreadSetRT():";
	pcparms_t mytpcparms = { 0 };
	rtparms_t   *myrtparmsp = (struct rtparms *) mytpcparms.pc_clparms;
	rtinfo_t    *rtinfop = (struct rtinfo *) tpcinfo.pc_clinfo;

	if ((realTimeEnable == 0) || (setrtInitialized == 0))
	{
		return 0;
	}

	// set up the tpcparms for everyone to use
	memset( &mytpcparms, 0, sizeof(pcparms_t) );

	mytpcparms.pc_cid = tpcinfo.pc_cid;
	myrtparmsp->rt_pri = rtinfop->rt_maxpri/2+prio;
	myrtparmsp->rt_tqsecs = 0;
	myrtparmsp->rt_tqnsecs = RT_NOCHANGE;

	// Check to see if the initialization routine has been called,
	// as only then we will make threads realtime
	if (priocntl(P_LWPID, P_MYID, PC_SETPARMS, (caddr_t)&mytpcparms) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
	}

#if 1
	mytpcparms.pc_cid = tpcparms.pc_cid;

	if (priocntl(P_LWPID, P_MYID, PC_GETPARMS, (caddr_t)&mytpcparms) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
	}

	// print these out
	NETDEBUG(MINIT, NETLOG_DEBUG4,
		(" RT: cid %lu, pri %d, ts=%d/%d\n", 
			ULONG_FMT(mytpcparms.pc_cid),
			myrtparmsp->rt_pri,
			myrtparmsp->rt_tqsecs,
			myrtparmsp->rt_tqnsecs));
#endif

	return(0);
}

// Sets to default RT priority
int
ThreadSetRT(void)
{
	char fn[] = "ThreadSetRT():";
	pcparms_t mytpcparms = { 0 };
	rtparms_t   *myrtparmsp = (struct rtparms *) mytpcparms.pc_clparms;

	if ((realTimeEnable == 0) || (setrtInitialized == 0))
	{
		return 0;
	}

	// Check to see if the initialization routine has been called,
	// as only then we will make threads realtime

	if (priocntl(P_LWPID, P_MYID, PC_SETPARMS, (caddr_t)&tpcparms) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
	}

#if 1
	mytpcparms.pc_cid = tpcparms.pc_cid;
	if (priocntl(P_LWPID, P_MYID, PC_GETPARMS, (caddr_t)&mytpcparms) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
	}

	// print these out
	NETDEBUG(MINIT, NETLOG_DEBUG4,
		(" RT: cid %lu, pri %d, ts=%d/%d\n", 
			ULONG_FMT(mytpcparms.pc_cid),
			myrtparmsp->rt_pri,
			myrtparmsp->rt_tqsecs,
			myrtparmsp->rt_tqnsecs));
#endif

	return(0);
}

int
ThreadInitRT(void)
{
	char fn[] = "ThreadInitRT():";
	rtparms_t   *rtparmsp = (struct rtparms *) tpcparms.pc_clparms;
	rtinfo_t    *rtinfop = (struct rtinfo *) tpcinfo.pc_clinfo;

	// initialize the class id for thread class
	strcpy(tpcinfo.pc_clname, "RT");

	if (priocntl(0, 0, PC_GETCID, (caddr_t)&tpcinfo) < 0)
	{
		NETERROR(MINIT, ("%s priocntl error %d\n", fn, errno));
		return -1;
	}

	// set up the tpcparms for everyone to use
	memset( &tpcparms, 0, sizeof(pcparms_t) );

	tpcparms.pc_cid = tpcinfo.pc_cid;
	rtparmsp->rt_pri = rtinfop->rt_maxpri/2;
	rtparmsp->rt_tqsecs = 0;
	rtparmsp->rt_tqnsecs = RT_NOCHANGE;

	setrtInitialized = 1;

	return 0;
}
#endif
// New Thread
void *
ThreadFn(void *arg1)
{
	char fn[] = "ThreadFn():";
	ThreadArg *targ = (ThreadArg *)arg1;
	void *res;

	// free memory not needed
	pthread_attr_destroy(&targ->attr);
	
	// call actual function
	res = (targ->fn)(targ->arg);

	// free our specific arg
	free(targ);

	return res;
}

// Launch a thread only if we are not in main
// return 0 if success
// If force is set, TRY to spawn a thread, and if that does not work,
// call the fn directly. Else, call a fn if we are already in a thread
int
ThreadLaunch(  void *(*fn)(void*),
               void *arg,
               int force )
{
	pthread_t mon_thread;
	int status;
	ThreadArg *targ;

	targ = ThreadNewArg();
	pthread_attr_init( &targ->attr );
	targ->arg = arg;
	targ->fn = fn;

	if (force || identMain())
	{
		// Launch a thread
		pthread_attr_setdetachstate( &targ->attr, PTHREAD_CREATE_DETACHED );
		pthread_attr_setscope( &targ->attr, PTHREAD_SCOPE_SYSTEM );
//		pthread_attr_setguardsize( &targ->attr, (size_t) (2*getpagesize()) );
		pthread_attr_setstacksize( &targ->attr, threadstack*1024 );

		status = pthread_create(&mon_thread, &targ->attr, ThreadFn, targ);
		if (status != 0)
		{
	  		NETERROR(MINIT, ("Could not launch thread\n"));
			if (force)
			{
				// call the fn directly
				ThreadFn(targ);
				return 0;
			}
			else
			{
				ThreadFreeArg(targ);
			}
		}

		return status;
	}
	else
	{
		// Dont have to spawn a thread
		NETDEBUG(MINIT, NETLOG_DEBUG4, ("No need to spawn thread\n"));

		// call the fn directly
		ThreadFn(targ);
		return 0;
	}
}

int
ThreadSetPriority( pthread_t thread,
                   int policy,
                   int priority )
{
	struct sched_param param = { 0 };
	int rc = 0;

//	rc = pthread_setschedparam(thread, policy, &param);

	return rc;
}

// thread id is filled up only if set to non-null
int
ThreadLaunch2(  void *(*fn)(void*),
                void *arg,
                int force, 
				int scope,
                int policy,
                int priority, 
				int	detachstate,
				pthread_t *tid )
{
	pthread_t mon_thread;
	int status;
	ThreadArg *targ;
	struct sched_param param = { 0 };

	targ = ThreadNewArg();
	pthread_attr_init( &targ->attr );
	targ->arg = arg;
	targ->fn = fn;

	if (force || identMain())
	{
		// Launch a thread
		pthread_attr_setdetachstate( &targ->attr, detachstate );
		pthread_attr_setscope( &targ->attr, PTHREAD_SCOPE_SYSTEM );
//		pthread_attr_setguardsize( &targ->attr, (size_t) (2*getpagesize()) );
		pthread_attr_setstacksize( &targ->attr, threadstack*1024 );
//		pthread_attr_setschedpolicy(&targ->attr, policy);
		param.sched_priority=priority;
		pthread_attr_setschedparam(&targ->attr, &param);

		status = pthread_create(&mon_thread, &targ->attr, ThreadFn, targ);
		if (status != 0)
		{
	  		NETERROR(MINIT, ("Could not launch thread\n"));
			if (force)
			{
				// call the fn directly
				ThreadFn(targ);
				return 0;
			}
			else
			{
				ThreadFreeArg(targ);
			}
		}

		if (tid)
		{
			*tid = mon_thread;
		}

		return status;
	}
	else
	{
		// Dont have to spawn a thread
		NETDEBUG(MINIT, NETLOG_DEBUG4, ("No need to spawn thread\n"));

		// call the fn directly
		ThreadFn(targ);
		return 0;
	}
}

int
ThreadFindClassAvailable(int poolid)
{
	ThreadPool *pool;
	int i;
	hrtime_t minstamp = 0, tstamp;
	int classid = -1;
     
	pool = &thread_pool[poolid];

	for (i=0; i<pool->nclasses; i++)
	{
		if (pool->pclass[i].progs->head->nitems > 0)
		{
    		tstamp = 
			  ((ThreadArg *)pool->pclass[i].progs->head->begin->next->item)->tstamp;

			if (minstamp)
			{
				if (minstamp > tstamp)
				{
					minstamp = tstamp;
					classid = i;
				}
			}
			else
			{
				minstamp = tstamp;
				classid = i;
			}
		}
	}

	return classid;
}

// New Thread
void *
ProgCPU(void *arg1)
{
	char fn[] = "ProgCPU():";
	ThreadArg *arg = (ThreadArg *)arg1;
	void *res = 0;
	int priority, policy;
	int i, status, avail;
	int *isDispatchThreadVal;
	ProgCPUStruct   *myprogCPU;
	List progs;
	int classid;
	cvtuple *progcv;
	ThreadPool *pool;
	hrtime_t now;

	// remember our parameters
	priority = arg->priority;
	policy = arg->policy;
	myprogCPU = (ProgCPUStruct *)arg->arg;
	pool = &thread_pool[myprogCPU->poolid];
	progcv = &pool->progcv;

	// re-write our thread id
	myprogCPU->tid = pthread_self();

	// free our specific arg
	free(arg);

	// set up our globals
	isDispatchThreadVal = (int *)malloc(sizeof(int));
	*isDispatchThreadVal = 1;
	if (status = pthread_setspecific(isDispatchThread, isDispatchThreadVal))
	{
		NETERROR(MINIT, ("%s pthread_setspecific error %d\n", 
			fn, status));
	}

	if (status = pthread_setspecific (threadIndex, NX_INT_TO_POINTER(myprogCPU->id)))
	{
		NETERROR(MINIT, ("%s pthread_setspecific error %d\n",
				 fn, status));
	}


	// Call the initialization function
	if (pool->threadInitFn)
	{
		(pool->threadInitFn)(pool->threadInitFnArg);
	}
#ifndef NETOID_LINUX
	if (pool->priority >= 0)
	{
		ThreadSetRTPrio(pool->priority);
	}
#endif
	while (1)
	{
//		ThreadSetPriority(pthread_self(), policy, priority);

		if ((status = pthread_mutex_lock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_lock error  %d\n", 
				fn, status));
			break;
		}

		progs = NULL;
		classid = -1;

		while (!progs)
		{
			if ((classid = ThreadFindClassAvailable(myprogCPU->poolid)) >= 0)
			{
				progs = pool->pclass[classid].progs;
				break;
			}

			myprogCPU->status = 0;
			status = pthread_cond_wait(&(progcv->cond), &(progcv->mutex));
			if (status == 0)
			{
				//
                // This may be a spurious wakeup. Let the
                // condition test decide
                // 
				continue;
			}

			NETERROR(MINIT, ("%s pthread_cond_wait error %d\n", fn, status));
			break;
		}

		myprogCPU->status = 1;

		// We are here because we won the bid
		arg = (ThreadArg *)listGetFirstItem(progs);
		listDeleteItem(progs, arg);

		if ((status = pthread_mutex_unlock(&(progcv->mutex))) != 0)
		{
			 NETERROR(MINIT, 
				("%s pthread_mutex_unlock error %d\n", fn, status));
			break;
		}

		now = nx_gethrtime();

		pool->pclass[classid].processed++;

		// see if we missed any deadline
		if (now > arg->tstamp)
		{
			pool->pclass[classid].missed++;
		}
	
//		ThreadSetPriority(pthread_self(), arg->policy, arg->priority);

		// now execute the function we have
		res = (arg->fn)(arg->arg);

		// free the arg
		free(arg);
	}

	return res;
}

int
ThreadSetInitFunction(  int poolid,
                        void (*threadInitFn)(),
                        void *arg )
{
	char fn[] = "ThreadSetInitFunction():";
	ThreadPool *pool;
	int classid;

	if (poolid >= allocated_pools)
	{
		NETERROR(MINIT, ("%s Invalid pool id %d\n", fn, poolid));
		return -1;
	}

	pool = &thread_pool[poolid];

	// Initialize a pool of threads
	pool->threadInitFn = threadInitFn;
	pool->threadInitFnArg = arg;
	
	return 0;
}

void *
ThreadInitProgCPU(int poolid)
{
	ProgCPUStruct *progCPU;

	progCPU = (ProgCPUStruct *)malloc(sizeof(ProgCPUStruct));
	memset(progCPU, 0, sizeof(ProgCPUStruct));

	progCPU->id = 0;
	progCPU->poolid = poolid;

	return progCPU;
}

int
ThreadHandleNotify( int fd,
                    FD_MODE rw,
                    void *data )
{
	char fn[] = "ThreadHandleNotify():";
	ThreadPool *pool;
	ProgCPUStruct   *myprogCPU = (ProgCPUStruct *)data;
	ThreadArg *arg = NULL;
	cvtuple *progcv;
	List progs;
	int classid;
	char buff[100];
	int res, status;
	hrtime_t now;

	pool = &thread_pool[myprogCPU->poolid];
	progcv = &pool->progcv;

	res = pool->wnotify - pool->rnotify;
	res = (res>100)?100:res;
	res = read(fd, buff, res);	
	pool->rnotify += res;
	NETDEBUG(MINIT, NETLOG_DEBUG4, ("%s Read %d notifications\n", fn, res));

	while (1)
	{
		if ((status = pthread_mutex_lock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_lock error  %d\n", 
				fn, status));
			break;
		}

		progs = NULL;
		classid = -1;

		if ((classid = ThreadFindClassAvailable(myprogCPU->poolid)) >= 0)
		{
			progs = pool->pclass[classid].progs;
		}

		if (progs == NULL)
		{
			pthread_mutex_unlock(&(progcv->mutex));
			NETDEBUG(MINIT, NETLOG_DEBUG4, ("%s progs is NULL\n", fn));
			break;
		}

		myprogCPU->status = 1;

		// We are here because we won the bid
		arg = (ThreadArg *)listGetFirstItem(progs);
		listDeleteItem(progs, arg);

		if ((status = pthread_mutex_unlock(&(progcv->mutex))) != 0)
		{
			 NETERROR(MINIT, 
				("%s pthread_mutex_unlock error %d\n", fn, status));
			break;
		}

		now = nx_gethrtime();

		pool->pclass[classid].processed++;

		// see if we missed any deadline
		if (now > arg->tstamp)
		{
			pool->pclass[classid].missed++;
		}

		NETDEBUG(MINIT, NETLOG_DEBUG4, ("%s processing queue item\n", fn));
	
		// now execute the function we have
		(arg->fn)(arg->arg);

		// free the arg
		free(arg);
	}

	return 0;
}

int
ThreadNotificationFd(int poolid)
{
	char fn[] = "ThreadNotificationFd():";
	ThreadPool *pool;

	if (poolid >= allocated_pools)
	{
		NETERROR(MINIT, ("%s Invalid pool id %d\n", fn, poolid));
		return -1;
	}

	pool = &thread_pool[poolid];

	return pool->notifyPipe[NOTIFY_READ];
}

// so that dbmalloc is ok
void
tfree(void *ptr)
{
	free(ptr);
}

//
//	Function:
//		ThreadPoolInit()
//
//	Description:
//		Allocate a thread pool from the statically defined
//		array of thread_pool[]'s and define the characteristics
//		of the threads used in the pool.
//
//	Arguments:
//
//		name            a character string naming the pool.
//
//		max             the maximum number of threads in the pool
//
//		scope           the pthreads scope value to be used when
//                      creating threads in the pool.
//                       PTHREAD_SCOPE_PROCESS or PTHREAD_SCOPE_SYSTEM
//                       argument does not seem to be used here.
//
//		priority        the priority of threads in the pool
//
//		policy          the process policy of the threads in the pool
//
//	Return Value:
//		the index of the pool within the static thread_pool[] array
//		or -1 on failure to allocate, which unchecked by caller
//		would be catastrophic in the near future.
//
int
ThreadPoolInit( char *name,
                int max,
                int scope,
                int priority,
                int policy )
{
	char fn[] = "ThreadPoolInit():";
	int i, poolid;
	int status;
	ThreadArg *targ;
	ThreadPool *pool;

	if (allocated_pools == THREAD_POOL_MAX)
	{
		NETERROR(MINIT, ("%s No more pools\n", fn));
		return -1;
	}

	poolid = allocated_pools++;
	pool = &thread_pool[poolid];
	
	memset(pool, 0, sizeof(ThreadPool));

	nx_strlcpy(pool->name, name, 32);

	pool->priority = priority;
	pool->policy = policy;

	// Initialize the cv
	CVInit(&pool->progcv);

	pool->progCPUSize = max;
	if (max > 0)
	{
		pool->progCPU = (ProgCPUStruct *)malloc(max*sizeof(ProgCPUStruct)); 
		memset(pool->progCPU, 0, max*sizeof(ProgCPUStruct));
	}
	
	// Create the globals for these threads, as we want to know
	// when to optimize dispatching by just calling the fn directly
	// and use the force arg

	if (status = pthread_key_create(&isDispatchThread, tfree))
	{
		NETERROR(MINIT, ("%s pthread_key_create error %d\n",
			fn, status));

		return -1;
	}
	if (status = pthread_key_create (&threadIndex, NULL))
	{
		NETERROR (MINIT, ("%s pthread_key_create error %d\n",
				  fn, status));
		return -1;
	}

	return poolid;
}

// return class id
int
ThreadAddPoolClass( char *name,
                    int poolid,
                    int maxitems,
                    longlong_t deadline )
{
	char fn[] = "ThreadPoolClassInit():";
	ThreadPool *pool;
	int classid;

	if (poolid >= allocated_pools)
	{
		NETERROR(MINIT, ("%s Invalid pool id %d\n", fn, poolid));
		return -1;
	}

	pool = &thread_pool[poolid];

	if (pool->nclasses == THREAD_POOLCLASS_MAX)
	{
		NETERROR(MINIT, ("%s No more classes\n", fn));
		return -1;
	}

	classid = pool->nclasses++;

	// Initialize a pool of threads
	pool->pclass[classid].progs = listInit();
	pool->pclass[classid].maxitems = maxitems;
	pool->pclass[classid].deadline = deadline;

	nx_strlcpy(pool->pclass[classid].name, name, 32);
	
	return classid;
}

int
ThreadPoolStart(int poolid)
{
	char fn[] = "ThreadPoolStart():";
	ThreadArg *targ;
	ThreadPool *pool;
	int i;

	pool = &thread_pool[poolid];

	// now launch the threads
	for (i=0; i<pool->progCPUSize; i++)
	{
		pool->progCPU[i].id = i;
		pool->progCPU[i].poolid = poolid;

		targ = ThreadNewArg();
		targ->priority = pool->priority;
		targ->policy = pool->policy;
		targ->arg = &pool->progCPU[i];

		ThreadLaunch2(ProgCPU, targ, 1, PTHREAD_SCOPE_SYSTEM,
			SCHED_RR, 50, PTHREAD_CREATE_DETACHED, &pool->progCPU[i].tid);
	}

	return(0);
}

int
ThreadPoolEnd(int poolid)
{
	char fn[] = "ThreadPoolEnd():";
	ThreadArg *targ;
	ThreadPool *pool;
	int i;

	pool = &thread_pool[poolid];

	// now kill the threads
	for (i=0; i<pool->progCPUSize; i++)
	{
		pthread_kill(pool->progCPU[i].tid, SIGKILL);
	}

	// Free the memory

	return(0);
}

int
ThreadPoolNotify(int poolid)
{
	char fn[] = "ThreadPoolNotify():";
	ThreadPool *pool;

	if (poolid >= allocated_pools)
	{
		NETERROR(MINIT, ("%s Invalid pool id %d\n", fn, poolid));
		return -1;
	}

	pool = &thread_pool[poolid];

	if (pool->wnotify != pool->rnotify)
	{
		// already notification in queue
		NETDEBUG(MINIT, 4, ("%s Already Notified r=%d w=%d\n",
			fn, pool->rnotify, pool->wnotify));
		return 0;
	}

	if (write(pool->notifyPipe[NOTIFY_WRITE]," ",1) < 0)
	{
		// Check the error
		if (errno == EAGAIN)
		{
			NETERROR(MINIT, ("Blocking error in notification\n"));
		}
	}
	else
	{
		pool->wnotify ++;
		NETDEBUG(MINIT, 4, ("%s Succesfully Notified r=%d w=%d\n",
			fn, pool->rnotify, pool->wnotify));
	}

	return(0);
}

// We will use the notification fd instead of
// condition variables
int
ThreadPoolInitNotify(int poolid)
{
	char fn[] = "ThreadPoolInitNotify():";
	ThreadPool *pool;
	int classid;
	int flags;

	if (poolid >= allocated_pools)
	{
		NETERROR(MINIT, ("%s Invalid pool id %d\n", fn, poolid));
		return -1;
	}

	pool = &thread_pool[poolid];

	/* Open notification pipe */
	if (pipe(pool->notifyPipe) < 0)
	{
		perror("Unable to open pipe");
		return -1;
	}

	if((flags = fcntl(pool->notifyPipe[NOTIFY_READ],F_GETFL,0)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(pool->notifyPipe[NOTIFY_READ],F_SETFL,flags)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	if((flags = fcntl(pool->notifyPipe[NOTIFY_WRITE],F_GETFL,0)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(pool->notifyPipe[NOTIFY_WRITE],F_SETFL,flags)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	return(0);
}

int
ThreadPoolGetPending(int poolid, int classid)
{
	char fn[] = "ThreadPoolGetPending():";
	List progs;
	ThreadPool *pool;

	pool = &thread_pool[poolid];
	progs = pool->pclass[classid].progs;

	return progs->head->nitems;
}

int
ThreadDispatchAction( int poolid,
                int classid,
                void *(*cfn)(void*),
                void *arg,
                int force, 
				int scope,
                int policy,
                int priority,
				int where )
{
	char fn[] = "ThreadDispatchAction():";
	int status;
	ThreadArg *targ;
	struct sched_param param = { 0 };
	int *isDispatchThreadVal;
	List progs;
	ProgCPUStruct *myprogCPU;
	ThreadPool *pool;
	cvtuple *progcv;
	int rc = 0;

	pool = &thread_pool[poolid];
	myprogCPU = pool->progCPU;
	progcv = &pool->progcv;

	if (xthreads == 0)
	{
		return ThreadLaunch2(cfn, arg, force, scope, policy, priority, 
			PTHREAD_CREATE_DETACHED, NULL);
	}

	if ((status = pthread_mutex_lock(&(progcv->mutex))) != 0)
	{
		NETERROR(MINIT, ("%s pthread_mutex_lock error %d\n", fn, status));
		return -1;
	}

	progs = pool->pclass[classid].progs;

	if ((pool->pclass[classid].maxitems) &&
		(progs->head->nitems == pool->pclass[classid].maxitems))
	{
		// Cannot add any more items in this class
		rc = -1;
		goto _return;
	}

	targ = ThreadNewArg();
	targ->arg = arg;
	targ->fn = cfn;
	targ->policy = policy;
	targ->priority = priority;
	targ->tstamp = nx_gethrtime() + pool->pclass[classid].deadline;

	if (where == 0)
	{
		listAddItem(progs, targ);
	}
	else
	{
		listAddItemInFront(progs, targ);
	}

	if (progs->head->nitems > pool->pclass[classid].peakitems)
	{
		pool->pclass[classid].peakitems = progs->head->nitems;
	}

	if (pool->notifyPipe[NOTIFY_WRITE] > 0)
	{
		ThreadPoolNotify(poolid);
	}

	// signal the workers
	if ((status = pthread_cond_signal(&(progcv->cond))) != 0)
	{
		NETERROR(MINIT, ("%s pthread_mutex_unlock error %d\n", fn, status));
	}

_return:
	if ((status = pthread_mutex_unlock(&(progcv->mutex))) != 0)
	{
		NETERROR(MINIT, ("%s pthread_mutex_unlock error %d\n", fn, status));
	}

#if 0
	if (isDispatchThreadVal = pthread_getspecific(isDispatchThread))
	{
		// this is a dispatch thread
		if (force == 0)
		{
			// We can optimize this
			cfn(arg);
			return 0;
		}
	}
#endif
	return rc;
}

int
ThreadDispatch( int poolid,
                int classid,
                void *(*cfn)(void*),
                void *arg,
                int force, 
				int scope,
                int policy,
                int priority )
{
	return ThreadDispatchAction(poolid, classid, cfn, arg, force, scope,
				policy, priority, 0);
}

int
ThreadStatsReset(void)
{
	char fn[] = "ThreadStats():";
	ThreadArg *arg;
	ListHead * lh;
	ListStruct * ptr;
	int status, i, n=0, poolid, classid;
	List progs;
	ProgCPUStruct *myprogCPU;
	ThreadPool *pool;
	cvtuple *progcv;

	for ( poolid = 0; poolid < allocated_pools; poolid++ )
	{
		pool = &thread_pool[poolid];
		myprogCPU = pool->progCPU;
		progcv = &pool->progcv;

		if ((status = pthread_mutex_lock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_lock error %d\n", fn, status));
			return -1;
		}

		for (classid = 0; classid<pool->nclasses; classid++)
		{
			progs = pool->pclass[classid].progs;

			pool->pclass[classid].peakitems = 0;
			pool->pclass[classid].missed = 0;
			pool->pclass[classid].processed = 0;
		}

		if ((status = pthread_mutex_unlock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_unlock error %d\n", fn, status));
			return -1;
		}
	}
	return(0);
}

int
ThreadStats(void)
{
	char fn[] = "ThreadStats():";
	ThreadArg *arg;
	ListHead * lh;
	ListStruct * ptr;
	int status, i, n=0, poolid, classid;
	List progs;
	ProgCPUStruct *myprogCPU;
	ThreadPool *pool;
	cvtuple *progcv;

	for ( poolid = 0; poolid < allocated_pools; poolid++ )
	{
		pool = &thread_pool[poolid];
		myprogCPU = pool->progCPU;
		progcv = &pool->progcv;

		if ((status = pthread_mutex_lock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_lock error %d\n", fn, status));
			return -1;
		}

		n = 0;

		for (i=0; i<pool->progCPUSize; i++)
		{
			n+=pool->progCPU[i].status;
		}

		for (classid = 0; classid<pool->nclasses; classid++)
		{
			progs = pool->pclass[classid].progs;

			printf( "pool %d(%s), class %d(%s), pending items %d,"
					" pending items peak %d, deadlines missed %d/%d, "
                    "active threads=%d/%d\n", 
					poolid,
					pool->name,
					classid,
					pool->pclass[classid].name,
					progs->head->nitems,
					pool->pclass[classid].peakitems,
					pool->pclass[classid].missed,
					pool->pclass[classid].processed,
					n,
					pool->progCPUSize);

		}

		if ((status = pthread_mutex_unlock(&(progcv->mutex))) != 0)
		{
			NETERROR(MINIT, ("%s pthread_mutex_unlock error %d\n", fn, status));
			return -1;
		}
	}
	return(0);
}
/*
  function is not required on linux
 */
#ifndef NETOID_LINUX
// Sets the whole process w/ threads to use ONE processor
int
ThreadSetUniprocessorMode(void)
{
	char fn[] = "ThreadSetUniprocessorMode():";
	processorid_t myprocessor = (processorid_t) 0;
	int rc;
	
	if ((rc = processor_bind(P_PID, P_MYID, myprocessor, 
				(processorid_t)NULL )) < 0)
	{
		rc = errno;
		NETERROR(MINIT, ("%s Could not initialize errno =%d\n", fn, rc));
	}
	return(0);
}
#endif
