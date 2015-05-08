/*
 * Right now this code is not MT - safe.
 * to make this MT - safe. We must
 * make the critical section code protected
 * by a lock
 */
/*
 * Timers are ordered in the list by the time left to
 * expire. Any item in the list <=> A timer is running.
 * First element is being added => start system timer.
 * Last element being deleted => stop system timer.
 * Timer expired => set the time to expire next by looking
 * at the first element in the list. If timer expired is an
 * interval timer, re-insert it intol the list appropriately.
 */

#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>

#include "timer.h"
#include "log.h"
#include <malloc.h>

#define MAX_ACTIVE_TIMERS 1000
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

tid activeTimers[MAX_ACTIVE_TIMERS];

tid *activeTimersBase;
#define activeTimersEnd  (activeTimers + (MAX_ACTIVE_TIMERS - 1))

int nTimers = 0; /* No of timers */

static struct itimerval _itimer;
static struct itimerval _otimer;
static void alarmHandler (int sig);

static int timerCritical = 0;
static int checkTimer();
static int _alarmPending = 0;

#define TIMER_CRITICAL_ENTER()  (timerCritical = 1)
#define TIMER_CRITICAL_EXIT()  {timerCritical = 0; checkTimer();}
#define IS_TIMER_SAFE() (timerCritical == 0)

int 
timerInit (void)
{
     int	retval = 0;

     /* Initialize the base and end pointers */
     activeTimersBase = activeTimers + MAX_ACTIVE_TIMERS;
     nTimers = 0;

     return 0;
}

int
timerComputeOffset(tid t)
{
     struct timeval now;

     gettimeofday(&now, 0);

     /* Based on expiration time */
     t->itime.it_value.tv_sec = t->expire.tv_sec - now.tv_sec;
     t->itime.it_value.tv_usec = t->expire.tv_usec - now.tv_usec;
     
     if (t->itime.it_value.tv_usec < 0)
     {
	  t->itime.it_value.tv_sec --;
	  t->itime.it_value.tv_usec += 1000000;
     }
}

int
timerSystemSet(void)
{
     char fn[] = "timerSystemSet():";

     if (nTimers > 0)
     {
	  /* We need to compute the offset of the first element */
	  timerComputeOffset(activeTimersBase[0]);

	  log(LOG_DEBUG, 0, "%s Head of list is %s\n", fn, activeTimersBase[0]->name);

	  _itimer.it_value.tv_sec = activeTimersBase[0]->itime.it_value.tv_sec;
	  _itimer.it_value.tv_usec = activeTimersBase[0]->itime.it_value.tv_usec;
	  _itimer.it_interval.tv_sec = 0;
	  _itimer.it_interval.tv_usec = 0;

	  log(LOG_DEBUG, 0, "%s Starting time for (%d, %d)\n", 
	      fn, _itimer.it_value.tv_sec, _itimer.it_value.tv_usec);

	  /* Set the ALARM */
	  signal (SIGALRM, alarmHandler);
	  
	  if (setitimer (ITIMER_REAL, &_itimer, &_otimer) < 0)
	       perror ("setitimer: ");
     }
     
     if (nTimers == 0)
     {
	  /* We need to stop the timers */
	  _itimer.it_value.tv_sec = 0;
	  _itimer.it_value.tv_usec = 0;
	  _itimer.it_interval.tv_sec = 0;
	  _itimer.it_interval.tv_usec = 0;
	  
	  if (setitimer (ITIMER_REAL, &_itimer, &_otimer) < 0)
	       perror ("setitimer: ");
	  
	  signal (SIGALRM, SIG_DFL);
     }
     
     return 0;
}

/* return microseconds */
long
timerDiff(struct timeval *t1, struct timeval *t2)
{
     long diff, udiff;

     diff = t1->tv_sec - t2->tv_sec;
     udiff = t1->tv_usec - t2->tv_usec;

     return (diff * 1000000 + udiff);
}
	  
int 
timerCompare(const void *e1, const void *e2)
{
     char fn[] = "timerCompare():";
     tid t1 = *(tid *)e1, t2 = *(tid *)e2;

     if (!t1 || !t2)
     {
	  log(LOG_DEBUG, 0, "%s: FATAL. NULL Entry found in array !!\n", fn);
	  return 0;
     }
     return timerDiff(&t1->expire, &t2->expire);
}

int
timersPrint()
{
     int n = 0;

     for (n = 0; n < nTimers; n++)
     {
	  fprintf(stderr, "%d %s\n", n, activeTimersBase[n]->name);
     }
}

tid *
timerAddEntry(tid t)
{
     char fn[] = "timerAddEntry()";
     tid *tp;

     TIMER_CRITICAL_ENTER();

     /* Get the pointer where to insert */
     if (activeTimersBase == activeTimers)
     {
	  tp = (activeTimersBase + nTimers);
     }
     else
     {
	  tp = --activeTimersBase;
     }
     
     nTimers++;

     if (!tp)
     {
	  log(LOG_DEBUG, 0, "%s Timers Not Initialized!\n");
	  return 0;
     }

     *tp = t;

     /* Sort the list */
     qsort(&activeTimersBase[0], nTimers, sizeof(tid), timerCompare);
     
     timerSystemSet();
     
     TIMER_CRITICAL_EXIT();

     return tp; 
}

tid 
timerAdd (struct itimerval *itime, int ntimes, char *name, TimerFn cb, void *data)
{
     char fn[] = "timerAdd():";
     tid t;
     
     t = (tid)malloc(sizeof(struct Timer));
     bzero(t, sizeof(struct Timer));

     t->id = t;
     strcpy (t->name, name);
     t->cb = cb;
     t->ntimes = ntimes;
     t->data = data;

     /* Structure copy */
     t->itime = *itime;
     gettimeofday(&t->expire, 0);

     t->expire.tv_sec += itime->it_value.tv_sec;
     t->expire.tv_usec += itime->it_value.tv_usec;
    
     if (timerAddEntry(t))
     {
	  return ((tid) t);
     }
     else
     {
	  free (t);
	  return 0;
     }
}

int 
timerDelete (tid t)
{
     char fn[] = "timerDelete():";
     int n;

     if (nTimers == 0)
     {
	  log(LOG_DEBUG, 0, "%s FATAL: There are no timers in  the list\n", fn);
	  return 0;
     }

     TIMER_CRITICAL_ENTER();

     /* First find this guy in the array */
     for (n = 0; n < nTimers; n++)
     {
	  if (activeTimersBase[n] == t)
	  {
	       break;
	  }
     }

     if (n == nTimers)
     {
	  log(LOG_DEBUG, 0, "%s Unable to find timer in list\n", fn);
	  goto _return;
     }

     free(t);

     if (n == 0)
     {
	  /* This probably knocked off the base timer.
	   * We dont have to do anything !
	   */
	  activeTimersBase ++;
	  nTimers --;
     }
     else if (n == (nTimers - 1))
     {
	  /* We deleted the last timer, this does not affect anything */
	  nTimers --;
	  goto _return;
     }
     else
     {
	  nTimers --;

	  /* This automatically means that there is at least one timer left */
	  if (nTimers > 1)
	  {
	       /* place whatever is the last timer here */
	       activeTimersBase[n] = activeTimersBase[nTimers];
	       
	       /* Sort the list again */
	       qsort(&activeTimersBase[0], nTimers, sizeof(tid), timerCompare);     
	  }
     }	  
     
     timerSystemSet();

 _return:
     TIMER_CRITICAL_EXIT();
     return 0;
}

int
timerStart (tid t)
{
}

int 
timerStatus (tid t)
{
}

int 
timerRemaining (tid t)
{
}

static int 
checkTimer()
{
     if (_alarmPending)
     {
	  alarmHandler(SIGALRM);
	  _alarmPending = 0;
     }
     return 0;
}

static void 
alarmHandler (int sig)
{
     char fn[] = "alarmHandler():";
     int	debugmode = 1;
     int expireNext = FALSE;
     tid t;
     
     printf ("Signal [%d] \n", sig);
     
     switch (sig)
     {
     case SIGALRM:
     {
	  if (!(IS_TIMER_SAFE()))
	  {
	       _alarmPending = 1;
	       return;
	  }

	  /* service all the expired timers */
	  while (nTimers > 0)
	  {
	       expireNext = FALSE;

	       /* Do this check befor we expire the timer, as the callback is supposed to 
		* free the timer
		*/
	       if (nTimers == 1)
	       {
		    /* This is a special case */
		    goto _expire;
	       }
	       
	       if (timerDiff(&activeTimersBase[1]->expire, &activeTimersBase[0]->expire) <= 0)
	       {
		    /* The next one also expires right now */
		    expireNext = TRUE;
	       }
	       else
	       {
		    /* after calling the callback we will break,
		     * So we can probably reset the
		     * it_val of this guy. We WILL NOT touch its expire time, 
		     * as that's its true EXPIRE TIME. NO ONE in the list should be using
		     * the guy's itime
		     */
		    /* Set up the interval in which the next guy will expire 
		     * as the time difference between the expiry timestamps
		     * of these guys
		     */
	       }
	       
	  _expire:
	       /* If this is an interval timer, we need to put it back into the list
		* again. Interval of 0 is not allowed, so this guy really cannot alter
		* the expireNext logic
		*/
	       t = activeTimersBase[0];
	       
	       ++activeTimersBase, --nTimers;
	       
	       if (t->ntimes) 
		    t->ntimes --;
	       
	       if (t->ntimes || 
		   t->itime.it_interval.tv_sec ||
		   t->itime.it_interval.tv_usec)
	       {
		    /* Add this guy back into the list */
		    
		    /* Set the expiry time */
		    gettimeofday(&t->expire, 0);
		    
		    /* offset by interval secs this time, instead of offset
		     * secs the first time
		     */
		    t->expire.tv_sec += t->itime.it_interval.tv_sec;
		    t->expire.tv_usec += t->itime.it_interval.tv_usec;
		    
		    timerAddEntry(t);

		    /* Everything is done, we can safely call the callback */
	       	    t->cb(t);
	       }
	       else
	       {
		    /* Everything is done, we can safely call the callback */
	            t->cb(t);
		    free(t);
	       }
	       
	       if (!expireNext)
	       {
		    break;
	       }
	  }
     }		  
     break;
     }
     
     /* Now we have to initialize the first timer in the list, so that
      * it refers to NOW
      */
     
     timerSystemSet();
}

#if 0
int
mycb(tid t)
{
     char fn[] = "mycb():";

     log(LOG_DEBUG, 0, "%s %s\n", fn, t->name);
}

int debug = 4;
main()
{
     struct itimerval t1, t2, t3, t4, t5;

     bzero(&t1, sizeof(struct itimerval));
     bzero(&t2, sizeof(struct itimerval));
     bzero(&t3, sizeof(struct itimerval));
     bzero(&t4, sizeof(struct itimerval));
     bzero(&t5, sizeof(struct itimerval));

     timerInit();

     t1.it_value.tv_sec = 1;
     t1.it_value.tv_usec = 1;

     t2.it_value.tv_sec = 2;
     t2.it_value.tv_usec = 1;

     t3.it_value.tv_sec = 3;
     t3.it_value.tv_usec = 1;
     
     t3.it_interval.tv_sec = 10;
     t3.it_interval.tv_usec = 1;

     t4.it_value.tv_sec = 4;
     t4.it_value.tv_usec = 1;

     t5.it_value.tv_sec = 5;
     t5.it_value.tv_usec = 1;

     timerAdd(&t1, 1, "test1", mycb);
     timerAdd(&t2, 1, "test2", mycb);
     timerAdd(&t3, 1, "test3", mycb);
     timerAdd(&t4, 1, "test4", mycb);
     timerAdd(&t5, 1, "test5", mycb);

     for (;;);
     
}
#endif

