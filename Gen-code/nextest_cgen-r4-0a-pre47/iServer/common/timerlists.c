/*
 * No timer should be deleted in a callback
 * function. The way to get rid of interval
 * timers this way would be to set the ntimes
 * and interval value in the timer to be 0, whereby
 * they would get deleted in the next callback.
 */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//#include "cache.h"
#include "nxosd.h"
#include "mem.h"
#include "srvrlog.h"
#include "list.h"
#include "timer.h"
#include "pthread.h"
#include <malloc.h>

#ifdef NOTMRLOG
#include "nodebuglog.h"
#endif

#define MAX(x,y)	(((x)>(y))?x:y)

Lock timermutex;

static pthread_mutex_t magic_mutex = PTHREAD_MUTEX_INITIALIZER;
static long magic = 0;

int
timerLibInit(void)
{
	return( 0 );
}

int 
timerInit (TimerPrivate *pvt, int xTimers, void (*asHandler)(void))
{
	memset(pvt, 0, sizeof(TimerPrivate));

	LockInit(&timermutex, 1);

	pvt->timerCache = CacheCreate(1);	// local mem
	pvt->timerCache->dt = CACHE_DT_AVLTR;
	pvt->timerCache->flags |= CACHE_MAINT_MIN;
	CacheSetName(pvt->timerCache, "TimerCache");
	pvt->timerCache->cachecmp = TimerCmp;
	pvt->timerCache->cacheinscmp = TimerCmp;
	pvt->timerCache->lock = &timermutex;
	//pvt->timerCache->pre_cond = TimerPreCb;
	//pvt->timerCache->post_cond = TimerPostCb;
	CacheInstantiate(pvt->timerCache);

	pvt->timerHandleCache = CacheCreate(1);	// local mem
	pvt->timerHandleCache->dt = CACHE_DT_AVL;
	CacheSetName(pvt->timerHandleCache, "TmrHCache");
	pvt->timerHandleCache->cachecmp = TimerHandleCmp;
	pvt->timerHandleCache->cacheinscmp = TimerHandleInsCmp;
	pvt->timerHandleCache->lock = &timermutex;
	//pvt->timerHandleCache->pre_cond = TimerPreCb;
	//pvt->timerHandleCache->post_cond = TimerPostCb;
	CacheInstantiate(pvt->timerHandleCache);

	pvt->nTimers = 0;

	return( 0 );
}

int
setTimerNotifyCb(TimerPrivate *pvt, int (*notify)(), void *data)
{
	pvt->notify = notify;
	pvt->notifycbdata = data;
	return( 0 );
}

/* Compute the offset of the timer with respect to now
 * Note that we store this offset in "t" itself
 */
void
timerComputeOffset(struct Timer *timer, struct timeval *res)
{
	struct timeval now;

	gettimeofday(&now, 0);

	/* Based on expiration time */
	res->tv_sec = timer->expire.tv_sec - now.tv_sec;
	res->tv_usec = timer->expire.tv_usec - now.tv_usec;
     
	if (res->tv_usec < 0)
	{
		res->tv_sec --;
		res->tv_usec += USECS_IN_SEC;
    }
}

/* if there are timers, return the first guy in the list
 */
int 
timerFront(TimerPrivate *pvt, struct Timer *front)
{
	int rc;
	struct Timer *tmp = NULL;

	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

	memset(front, 0, sizeof(struct Timer ));

    if (pvt->nTimers > 0)
    {
		if (tmp = CacheGetMin(pvt->timerCache))
		{
			memcpy(front, tmp, sizeof(struct Timer));
		}
		else
		{
			NETERROR(MTMR, ("timerFront: Timer Cache is empty\n"));
		}

		rc = 1;
		goto _return;
    }
    	 
    if (pvt->nTimers == 0)
   	{ 
		rc = 0;
		goto _return;
   	}

_return:
	CacheReleaseLocks(pvt->timerCache);

	return rc;
}
     
/* BEWARE: this function will not function properly,
 * when enetered with unsigned values which are high
 * enough to have negative signed values...
 * See the fix_this_to_be_correct thing below
 */
/* return microseconds */
long
timerDiff(struct timeval *t1, struct timeval *t2)
{
	long diff, udiff;

    diff = t1->tv_sec - t2->tv_sec;
    udiff = t1->tv_usec - t2->tv_usec;

    if (diff < 0)
    {
		return -1;
    }

    return ((diff * USECS_IN_SEC) + udiff);
}
	  
int
timerCompare(const void *e1, const void *e2, void *param)
{
	char fn[] = "timerCompare():";
	struct Timer *timer1 = (struct Timer*)e1, *timer2 = (struct Timer*)e2;
	long diff, udiff;

    if (!timer1 || !timer2)
    {
		NETERROR(MTMR, ("%s: Null entry \n", fn));
		return 0;
	}

    diff = timer1->expire.tv_sec - timer2->expire.tv_sec;
    udiff = timer1->expire.tv_usec - timer2->expire.tv_usec;

	return (diff<0)?-1:
		((diff>0)?1:
			((udiff<0)?-1:
				((udiff>0)?1:(e1-e2))));
}

int
timerHandleCompare(const void *e1, const void *e2, void *param)
{
	char fn[] = "timerHandleCompare():";
	tid id = *(tid*)e1;
	struct Timer *timer = (struct Timer*)e2;

    if (id == 0 || timer == NULL)
    {
		NETERROR(MTMR, ("%s: Null entry \n", fn));
		return 0;
	}

	if(id < timer->id)
	{
		return 1;
	}
	else if(id > timer->id)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
timerHandleInsCompare(const void *e1, const void *e2, void *param)
{
	char fn[] = "timerHandleCompare():";
	struct Timer *timer1 = (struct Timer*)e1, *timer2 = (struct Timer*)e2;

    if (timer1 == NULL || timer2 == NULL)
    {
		NETERROR(MTMR, ("%s: Null entry \n", fn));
		return 0;
	}

	if(timer1->id < timer2->id)
	{
		return 1;
	}
	else if(timer1->id > timer2->id)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int
timerAbsolute(struct timeval *t)
{
	if (t->tv_usec >= USECS_IN_SEC)
	{
		t->tv_sec += (t->tv_usec/USECS_IN_SEC);
		t->tv_usec %= USECS_IN_SEC;
	}
	return( 0 );
}

int
timerCompareValue(struct timeval *t1, struct timeval *t2)
{
	char fn[] = "timerCompareValue():";
	long diff, udiff;
	int rc = 0;

    if (!t1 || !t2)
    {
		NETERROR(MTMR, ("%s: Null entry \n", fn));
		return 0;
	}

    diff = t1->tv_sec - t2->tv_sec;
    udiff = t1->tv_usec - t2->tv_usec;

	return (diff<0)?-1:
		((diff>0)?1:
			((udiff<0)?-1:
				((udiff>0)?1:0)));

}

int
timerCheckExpiration(struct timeval *t1, struct timeval *t2, 
						unsigned int maxsecs)
{
	long diff;
	int rc;

	rc = timerCompareValue(t1, t2);

	if (rc > 0)
	{
    	diff = t1->tv_sec - t2->tv_sec;
		if (diff > maxsecs) 
		{
			NETERROR(MTMR, 
				("timer fired, expiration due in %lds, expected in %ds\n",
				diff, maxsecs));
			rc = -1;
		}
	}

	return rc;
}

int
timersPrint(TimerPrivate *pvt)
{
    int n = 0;
	struct Timer *timer;

	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

	for (timer=CacheGetFirst(pvt->timerCache);timer;timer=CacheGetNext(pvt->timerCache,timer))
	{
		printf("timer %p %s[%d]cb(%p,%p)expire[%ld,%ld]value[%ld,%ld]int[%ld,%ld] \n",
					timer, timer->name, timer->ntimes,	
					timer->cb, timer->data, timer->expire.tv_sec, timer->expire.tv_usec,
					timer->itime.it_value.tv_sec, timer->itime.it_value.tv_usec,
					timer->itime.it_interval.tv_sec, timer->itime.it_interval.tv_usec);
	}

	CacheReleaseLocks(pvt->timerCache);

	return( 0 );
}

/* Must be lock protected when called */
struct Timer *
timerAddEntry(TimerPrivate *pvt, struct Timer *timer)
{
	if (CacheInsert(pvt->timerCache, timer) < 0)
	{
		NETERROR(MTMR, ("timerAddEntry error adding into cache\n"));
		return NULL;
	}
	else if (CacheInsert(pvt->timerHandleCache, timer) < 0)
	{
		CacheDelete(pvt->timerCache, timer);
		return NULL;
	}
	else
	{
    	(pvt->nTimers)++;
	}

_return:
    return (void *)0xdeadbeef;
}

static tid createTid()
{
	tid id;
	time_t now;

	time(&now);

	pthread_mutex_lock(&magic_mutex);

	id = ((long long)now << 32) | ++magic;

	pthread_mutex_unlock(&magic_mutex);

	return id;
}


char *
timerGetName(TimerPrivate *pvt, tid id)
{
	struct Timer *timer;
	char *name;

	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

	if((timer = CacheGet(pvt->timerHandleCache, &id)))
	{
		name = strdup(timer->name);
	}
	else
	{
		name = strdup("");
	}

	CacheReleaseLocks(pvt->timerCache);

	return name;
}


tid 
timerAddToList (TimerPrivate *pvt, struct itimerval *itime, 
	unsigned short ntimes, unsigned short type, char name[], 
	TimerFn cb, void *data)
{
	static char fn[]="timerAddToList():";

	struct Timer *timer;

	if (!name)
	{
		NETERROR (MDEF, ("%s No name supplied, timer NOT added\n",fn));
		return 0;
	}
    if ((timer = malloc(sizeof(struct Timer))) == NULL)
    {
		DEBUG(MDEF, NETLOG_DEBUG4, ("%s malloc failed....\n",fn));

		return 0;
    }

    memset(timer, 0, sizeof(struct Timer));

    timer->id = createTid();

    nx_strlcpy (timer->name, name,TMR_NAME_LEN -1);
    timer->name[TMR_NAME_LEN-1] = '\0';

    timer->ntimes = ntimes;
    timer->data = data;
    timer->cb = cb;

    memcpy(&timer->itime, itime, sizeof(struct itimerval));

	if (type == PSOS_TIMER_ABS)
	{
		timer->expire.tv_sec = itime->it_value.tv_sec;
		timer->expire.tv_usec = itime->it_value.tv_usec;
	}
	else
	{
     	gettimeofday(&timer->expire, 0);

		NETDEBUG(MTMR, NETLOG_DEBUG1, ("%s tmr %p %s added to expire, now=[%ld,%ld]\n",fn,
			timer, timer->name, timer->expire.tv_sec, timer->expire.tv_usec));
		/* If value is set, we treat it as an offset, otherwise
		* we use the interval 
		*/	
		if (itime->it_value.tv_sec ||
			itime->it_value.tv_usec)
		{
     			timer->expire.tv_sec += itime->it_value.tv_sec;
     			timer->expire.tv_usec += itime->it_value.tv_usec;
		}
		else
		{
     			timer->expire.tv_sec += itime->it_interval.tv_sec;
     			timer->expire.tv_usec += itime->it_interval.tv_usec;
		}

		timerAbsolute(&timer->expire);

		NETDEBUG(MTMR, NETLOG_DEBUG1, ("%s tmr %s expires at [%ld,%ld]\n",fn,
			timer->name, timer->expire.tv_sec, timer->expire.tv_usec));
	}
    
	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

    if (timerAddEntry(pvt, timer) == 0)
    {
		NETERROR(MTMR, ("%s Failed to Add Timer\n",fn));

	  	free (timer);
		timer = NULL;
    }
	else
	{
		/* Call the notify function */
		if (pvt->notify)
		{
			NETDEBUG(MTMR, NETLOG_DEBUG4, ("%s Notifying application\n",fn));
			pvt->notify(pvt->notifycbdata);
		}
	}

	CacheReleaseLocks(pvt->timerCache);

	return (timer ? timer->id : 0);
}


// return 1 if timer was successfully deleted and 0 otherwise
int
timerDeleteFromList (TimerPrivate *pvt, tid id, void **data)
{
	char fn[] = "timerDeleteFromList():";
	struct Timer *timer;
    int n, m, rc = 1;

	*data = NULL;

	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

    if (pvt->nTimers == 0)
    {
	  	NETERROR(MTMR, ("%s FATAL: There are no timers in  the list\n", fn));
		rc = 0;
	  	goto _return;
    }

	if ((timer = CacheDelete(pvt->timerHandleCache, &id)) == 0)
	{
		NETDEBUG(MTMR, NETLOG_DEBUG1,
			("%s Error in deleting timer %p\n", fn, timer));
		rc = 0;
	}
	else if (CacheDelete(pvt->timerCache, timer) == 0)
	{
		NETERROR(MTMR, ("%s Error in deleting timer %s\n", fn, timer->name));
		rc = 0;
	}
	else
	{
		pvt->nTimers --;

		*data = timer->data;
		free(timer);
		rc = 1;
	}
     
	/* Call the notify function */
	if (rc && (pvt->notify))
	{
		NETDEBUG(MTMR, NETLOG_DEBUG4, ("Notifying application\n"));
		pvt->notify(pvt->notifycbdata);
	}

 _return:

	CacheReleaseLocks(pvt->timerCache);

	/* Success in deleting timer from local list */
    return rc;
}


int
timerSum(struct timeval *sum, struct timeval *t1, struct timeval *t2)
{
	sum->tv_sec = t1->tv_sec + t2->tv_sec;
	sum->tv_usec = t1->tv_usec + t2->tv_usec;

	if (sum->tv_usec >= 1000000)
	{
		sum->tv_usec -= 1000000;
		sum->tv_sec ++;

		return 1;
	}
	
	return 0;
}

/* Runs in the main loop */
int
serviceTimers(TimerPrivate *pvt)
{
	struct Timer *timer;
    static struct timeval now;
    long diff;

	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

    /* service all the expired timers */
    while (pvt->nTimers > 0)
    {
	  	/* Doing gettimeofday here, will expire the timers more
	   	* accurately.
	   	*/
		gettimeofday(&now, NULL);

		NETDEBUG(MTMR, NETLOG_DEBUG4, 
			("Time now [%ld,%ld]\n", now.tv_sec, now.tv_usec));

		timer = CacheGetMin(pvt->timerCache);

		if (timer == NULL)
		{
			NETERROR(MTMR, ("serviceTimer: Timer cache is empty\n"));
			break;
		}

		NETDEBUG(MTMR, NETLOG_DEBUG4, ("Timer Base %p\n", timer));

	  	if ((diff = timerCheckExpiration(&timer->expire, &now, 
						MAX(timer->itime.it_value.tv_sec, 
							timer->itime.it_interval.tv_sec)+1)) > 0)
	  	{
			NETDEBUG(MTMR, NETLOG_DEBUG4, 
				("Timer %s still is left to expire\n", timer->name));

			break;
	  	}
	  
		NETDEBUG(MTMR, NETLOG_DEBUG4, ("Timer Base %p\n", timer));
	  	/* If this is an interval timer, we need to put it back into the list
	   	* again. Interval of 0 is not allowed.
	   	*/
		DEBUG(MTMR, NETLOG_DEBUG4, ("Timer %p\n", timer));
		DEBUG(MTMR, NETLOG_DEBUG1, ("Timer %s (%p) expiring at [%ld,%ld]\n", timer->name, timer, now.tv_sec, now.tv_usec));
		NETDEBUG(MTMR, NETLOG_DEBUG1, ("tmr %s was scheduled at [%ld,%ld]\n",
			timer->name, timer->expire.tv_sec, timer->expire.tv_usec));

		if (CacheDelete(pvt->timerHandleCache, &timer->id) == 0)
		{
			NETERROR(MTMR, ("serviceTimers: Error in deleting timer\n"));
			continue;
		}
		else if (CacheDelete(pvt->timerCache, timer) == 0)
		{
			NETERROR(MTMR, ("serviceTimers: Error in deleting timer\n"));
			continue;
		}
		else
		{
	  		--(pvt->nTimers);
		}
	  
		// At this point the timer is successfully deleted
	  	if (timer->ntimes) 
	       	timer->ntimes --;
	  
	  	if (timer->ntimes || 
	      	timer->itime.it_interval.tv_sec ||
	      	timer->itime.it_interval.tv_usec)
	  	{
	       	/* Add this guy back into the list */
	       
			DEBUG(MTMR, NETLOG_DEBUG1, ("Timer needs to be added back\n"));

	       	/* Set the expiry time */
	       	/* offset by interval secs this time, instead of offset
			* secs the first time
			*/
			if (timer->cb == NULL)
			{
				NETERROR(MTMR, ("Interval Timer Callback was NULL\n"));
			}
			else
			{

	       		timer->expire.tv_sec = now.tv_sec + timer->itime.it_interval.tv_sec;
	       		timer->expire.tv_usec = now.tv_usec + timer->itime.it_interval.tv_usec;
	       
	       		timerAddEntry(pvt, timer);
	       
				// Release locks at this point, as we are done for now
				CacheReleaseLocks(pvt->timerCache);

	       		/* Everything is done, we can safely call the callback */
	       		timer->cb(timer);

				CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);
			}
	  	}
	  	else
	  	{
	       	/* Everything is done, we can safely call the callback */
			DEBUG(MTMR, NETLOG_DEBUG2, ("Timer Callback\n"));

			if (timer->cb)
			{
				CacheReleaseLocks(pvt->timerCache);

	       		timer->cb(timer);

				CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);
			}
			else
			{
				NETERROR(MTMR, ("Value Timer Callback was NULL\n"));
			}

			// Timer is Always FREED by the calling application.
			// Either in an explicit timerDelete or
			// by timerFreeHandle
	       	//free(t);

	  	}
	}

	CacheReleaseLocks(pvt->timerCache);

	return( 0 );
}

int 
TimerPreCbPrint(struct cache_t *cache, int op, void *data, size_t size)
{
	struct Timer *timer = (struct Timer*)data;

	NETDEBUG(MTMR, NETLOG_DEBUG4,
		("op %s begin on timer %s\n", 
		cache_operations_strings[op], timer?timer->name:"X"));
		
	return 0;
}

int 
TimerHandlePreCbPrint(struct cache_t *cache, int op, void *data, size_t size)
{
	struct Timer *timer = CacheGet(cache, data);

	NETDEBUG(MTMR, NETLOG_DEBUG4,
		("op %s begin on timer %s\n", 
		cache_operations_strings[op], timer?timer->name:"X"));
		
	return 0;
}

int 
TimerPostCbPrint(struct cache_t *cache, int op, void *data, size_t size)
{
	int result;
 
	if (data)	
		result =  *(int *)data;
	else
		result = 0;

	NETDEBUG(MTMR, NETLOG_DEBUG4,
		("op %s end result=%d\n", 
		cache_operations_strings[op], result));
		
	return 0;
}

int 
TimerHandlePostCbPrint(struct cache_t *cache, int op, void *data, size_t size)
{
	int result;
 
	if (data)	
		result =  *(int *)data;
	else
		result = 0;

	NETDEBUG(MTMR, NETLOG_DEBUG4,
		("op %s end result=%d\n", 
		cache_operations_strings[op], result));
		
	return 0;
}

int
TimerStats(TimerPrivate *pvt)
{
	CacheGetLocks(pvt->timerCache,LOCK_READ,LOCK_BLOCK);

	printf("no of timers=%d\n", pvt->timerCache->nitems);

	CacheReleaseLocks(pvt->timerCache);
	return( 0 );
}

