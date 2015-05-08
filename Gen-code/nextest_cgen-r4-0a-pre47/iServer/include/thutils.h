#ifndef _thutils_h_
#define _thutils_h_
#include "nxosdtypes.h"
#include "fdsets.h"


typedef void *(*PFVP)(void *);

// return classid
int
ThreadAddPoolClass(char *name, int poolid, int maxitems, longlong_t deadline);

// start the threads
int
ThreadPoolStart(int poolid);

int
ThreadPoolEnd(int poolid);

// return poolid
int
ThreadPoolInit(char *name, int max, int scope, int priority, int policy);

int
ThreadDispatch(int poolid, int classid, void *(*cfn)(void*), void *arg, int force, int scope, int policy, int priority);

int
ThreadLaunch2(void *(*fn)(void*), void *arg, int force, 
				int scope, int policy, int priority, int detachstate, 
				pthread_t *tid);

int
ThreadLaunch(void *(*fn)(void*), void *arg, int force);

int
ThreadSetPriority(pthread_t thread, int policy, int priority);

void *
ThreadInitProgCPU(int poolid);

int 
ThreadHandleNotify(int fd, FD_MODE rw, void *data);

int 
ThreadPoolNotify(int poolid);

int 
ThreadNotificationFd(int poolid);

int 
ThreadPoolInitNotify(int poolid);

int
ThreadSetRT(void);

int
ThreadInitRT(void);

extern int getMyThreadIndex (void);

extern int ThreadPoolGetPending (int poolid, int classid);

int ThreadStatsReset(void);
int ThreadStats(void);
int ThreadDispatchAction( int poolid, int classid, void *(*cfn)(void*), void *arg, int force, int scope, int policy, int priority, int where );

	
	


#endif /* _thutils_h_ */
