#include <stdio.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <string.h>
#include "srvrlog.h"
#include "timer.h"

int
setupTimeout(TimerPrivate *pTimerPrivate,struct timeval *tout)
{
        static struct timeval now;
        static struct Timer front;
        int retval = 0;
        long secs, usecs;

        tout->tv_sec = 0xfffffff;
        tout->tv_usec = 0xfffffff;

        /* Set up the timeout based on our own timer list */
        if (timerFront(pTimerPrivate, &front) != 0)
        {
                gettimeofday(&now, NULL);

                /* compute front - now, to return the delta */
                secs = front.expire.tv_sec - now.tv_sec;
                usecs = front.expire.tv_usec - now.tv_usec;

                if (usecs < 0)

                {
                        secs --;
                        usecs += 1000000;
                }

                if (secs < 0)
                {
                        DEBUG(MTMR, NETLOG_DEBUG4,
                                ("SetupTimeout:: secs is negative\n"));
			DEBUG(MTMR,NETLOG_DEBUG4,("front secs = %ld front usec = %ld\n",front.expire.tv_sec,front.expire.tv_usec));
			DEBUG(MTMR,NETLOG_DEBUG4,("now sec = %ld now usec = %ld\n",now.tv_sec,now.tv_usec));
                        return -1;
                }

                tout->tv_sec = secs;
                tout->tv_usec = usecs;

                retval = 1;
        }

        /* we are done... */
        return retval;
}

TimerNotifyCBData *
TimerNotifyCBDataAlloc()
{
	TimerNotifyCBData *tcbdata;

	tcbdata = (TimerNotifyCBData *)malloc(sizeof(TimerNotifyCBData));
	memset(tcbdata, 0, sizeof(TimerNotifyCBData));

	return tcbdata;
}
