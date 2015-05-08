#ifndef _timer_h_
#define _timer_h_

//
// Whoever deletes the timer, owns the timer callback data
// Timer handle WILL ALWAYS be freed by the application.
// 	free will be auto when timerDelete is called and must
//	be deleted by the application, when timer callback is called,
//	for a value timer. Interval timers MUST never be freed.

#define USECS_IN_SEC	1000000

#include <sys/types.h>
#ifdef NETOID_LINUX
#include <sys/time.h>
#else /* NETOID_LINUX */
#ifndef SUNOS
#include <pna.h>


int 
gettimeofday(struct timeval *tv, struct timezone *tz);

struct itimerval {
	struct timeval it_interval;
	struct timeval it_value;
};
#endif
#include <time.h>
#define ITIMER_REAL	0
#define ITIMER_VIRTUAL	1
#define ITIMER_PROF	2

#endif /* NETOID_LINUX */

#define PSOS_TIMER_ABS		1
#define PSOS_TIMER_REL		2
#define TMR_NAME_LEN		12

/* Definition of a timer id */
#ifdef NETOID_LINUX
typedef u_quad_t tid;
#else /* NETOID_LINUX */
typedef u_longlong_t tid;
#endif /* NETOID_LINUX */


#include "cache.h"

typedef enum
{
	TIMER_PERM = 0,
	TIMER_ONE_SHOT,
	TIMER_TWO_SHOT,
} TimerDuration;

/*
 * Works with real time.
 */
typedef struct Timer
{
     tid id;			        /* My id */
     char	name[TMR_NAME_LEN];		/* Short name for timer */
     struct itimerval    itime;	        /* The duration of the timer 
					 * Only the interval will have a 
					 * valid value when the 
					 * timer expires... NOTE
					 */
     struct timeval      expire;        /* When this timer is going to 
					 * expire - absolute time 
					 */ 
     int	ntimes;			/* PERM, ONE_SHOT */
     time_t	when;			/* When it was set */
     int (*cb)(struct Timer*);			/* Callback to call on expiration */
     void *data;                        /* Some data */
     
     struct Timer * next;
} Timer;

typedef	int	(*TimerFn)(struct Timer *t);

/* This structure provides the timer private vaiables for each task
 * which wants to use the timer library. Must be initialized by the
 * task
 */
typedef struct TimerPrivate {
	cache_t	timerCache;
	cache_t	timerHandleCache;	/* Cache of handles */
	unsigned short xTimers;		/* Max # timers */
	unsigned short nTimers;		/* No of timer entries in the array */
	unsigned long taskTid;		/* Task Id of user task */
//	void (*asHandler)(void);	/* Signal handler */
	struct timeval when;		/* If any timer is running,
					 * when is it going to expire!!
					 */
	int (*notify)();			/* Notification function when a timer
								 * is manipulated, such that app
								 * needs to be notified
								 */
	void *notifycbdata;			/* data passed to the write notify */
} TimerPrivate;

/* Library Initialization Function Must be invoked once */
extern int timerLibInit(void);

extern int timerInit (TimerPrivate *, int, void (*asHandler)(void));

char *timerGetName(TimerPrivate *, tid);

tid timerAddToList(TimerPrivate *, struct itimerval *, 
		unsigned short, unsigned short,
		char [], TimerFn, void *);

int timerDeleteFromList (TimerPrivate *, tid, void **);

#define timerGetCbData(_tid_)	((_tid_)->data)

extern void timerServiceSignal(TimerPrivate *pvt);

#define timerFreeHandle(_timer_)	free(_timer_)

void timerComputeOffset(struct Timer*, struct timeval *);
int serviceTimers(TimerPrivate *);
long timerDiff(struct timeval *, struct timeval *);
int timerFront(TimerPrivate *, struct Timer* );
int timerDeleteFromListOnCheck(TimerPrivate *pvt, 
	int (*fn)(tid, void *), void *data);

int adjustTimersByOffset(TimerPrivate *pvt, struct timeval *offset, int pol);

#define timerNonZero(timer) (((timer)->tv_sec != 0) || ((timer)->tv_usec != 0))

/* Signal Normally used by calling task */
#define ITIMER_AS_ALARM	1

/* Queue on which setitimer sends message */
#define ITIMER_TIMER_QUEUE	"timQ"

/* Queue Message exchanged between the library and the
 * timer daemon. This is internal to the library, and
 * embedded inside the setitimer call
 */
typedef struct ItimerQueueMsg {
	unsigned long tid;		/* Task Id of calling task */
	unsigned long signals;		/* Signals (Async) which calling
					 * task wants to get when the timer
					 * expires
					 */
	struct timeval when;		/* Absolute time (derived from 
					 * gettimeofday call), when this timer
					 * should expire
					 */
} ItimerQueueMsg;

// General data structure used by
// the application for notification callbacks
// when a timer is added/deleted
// Note that this is generally used on the notification
// callback fd.

typedef struct
{
	// Used for optimizing the no of notofications
	int wnotify;
	int rnotify;

	// pipe used for notofications
	int notifyPipe[2];
} TimerNotifyCBData;

TimerNotifyCBData * TimerNotifyCBDataAlloc();

int setTimerNotifyCb(TimerPrivate *pvt, int (*notify)(), void *data);

#endif /* _timer_h_ */
