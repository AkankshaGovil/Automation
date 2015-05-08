#include "pmpoll.h"
#include "gis.h"
#include "lsprocess.h"
#include "nxosd.h"
#include <malloc.h>
#include <poll.h>
#include "seli.h"
#include "gk.h"
#include "ls.h"
#include "uh323.h"


// Main Loop Processing for gis

#define MAX(x,y)	(((x)>(y))?x:y)

time_t lastTime, timeNow;

void
IpcH323LaunchLoops(void)
{
	int i, *iptr;

	for (i=1; i<nH323Threads; i++)
	{
		iptr = (int *)malloc(sizeof(int));
		*iptr = i;
		ThreadLaunch2((void *(*)(void*))IpcH323MainLoop, iptr, 1, 
						PTHREAD_SCOPE_SYSTEM, SCHED_RR, 50, 
						PTHREAD_CREATE_DETACHED, &h323Threads[i]);
	}
}

// Main H.323 Loop for all scenarios:
// single instance, multiple instance, multiple threads
extern int processus[3],ioctlus[3],devNumEvents[3],devfdsUsed[3];
int
IpcH323MainLoop (void *arg)
{
	char fn[] = "IpcH323MainLoop():";
	int	retval = 0;
	sigset_t o_signal_mask, p_signal_mask;
	int msec  = -1, timermsec = -1;
	int rc = 0, numus = 0;
	int outid;
	TimerNotifyCBData *tcbdata;
	hrtime_t hrstart, hrend, hrdiff,hr1,hr2,hrloop1,hrloop2;


	lastTime= time(NULL);
	outid = *(int *)arg;

	// Sanity check
	if ((outid < 0) || (outid >= nH323Threads))
	{
		NETERROR(MH323, ("%s Bad Id %d\n", fn, outid));
		return -1;
	}

	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	// Store our id
	h323Threads[outid] = pthread_self();

	pthread_cleanup_push(GkEnd, arg);

	/* Open a notification pipe */
	NetFdsAdd(&lsh323fds[outid], 
		ThreadNotificationFd(h323inPool[outid]), FD_READ, 
		(NetFn) ThreadHandleNotify, (NetFn) NULL,
		ThreadInitProgCPU(h323inPool[outid]), NULL);

	tcbdata = TimerNotifyCBDataAlloc();
	InitPipe(tcbdata->notifyPipe);

	/* Add the callback */
	setTimerNotifyCb(&h323timerPrivate[outid], IServerH323Notify, tcbdata);

	NETDEBUG(MSEL, NETLOG_DEBUG4, ("tcbdata notify read fd  = %d write = %d\n",
	tcbdata->notifyPipe[NOTIFY_READ],
	tcbdata->notifyPipe[NOTIFY_WRITE]));
	/* Open a notification pipe */
	NetFdsAdd(&lsh323fds[outid], tcbdata->notifyPipe[NOTIFY_READ], FD_READ, 
		(NetFn) HandleH323Notify, (NetFn) NULL,
		tcbdata, NULL);

	UH323InitThread(outid);

	// Now we are ready to initialize the callbacks
	UH323Ready(outid);

	uh323ready = 1;

	/* Loop forever */
	for (; ;)
	{
		pthread_testcancel();

		rc = 0;
		timeNow = time(NULL);

		retval = LusSetupTimeoutInMsec(&h323timerPrivate[outid], &timermsec);

		if (retval < 0)
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("h323 main loop timeout already outid = %d\n",outid));

			serviceTimers(&h323timerPrivate[outid]);
			continue;
		}

		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s %d ready for us\n", 
			fn, timermsec));

		// Find out how many entries we have used up first
		numus = NetFdsSetupPoll(&lsh323fds[outid], MSEL, NETLOG_DEBUG4);
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%d ready for gis\n", numus));

		// if no timer fires for msec - then go call seliSelect 

		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s %d/%d ready in all\n", 
			fn, numus, msec));

		retval = poll(lsh323fds[outid].pollA, numus, 0);

		pthread_testcancel();
		switch (retval)
		{
		case -1:
			NETDEBUG(MSEL, NETLOG_DEBUG1, ("poll failure %d", errno));
			break;
		case 0:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll timeout"));
			serviceTimers(&h323timerPrivate[outid]);
			break;
		default:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll process"));

			// Moved this line up from below, rc = 0 always at this point

			hrstart = nx_gethrtime();
			rc = NetFdsProcessPoll(&lsh323fds[outid], MSEL, NETLOG_DEBUG4);
			hrend = nx_gethrtime();
			hrdiff = hrend - hrstart;
			uh323Globals[outid].bridgems = hrdiff/1000000;

			if (rc < 0)
			{
				NETERROR(MSEL,
					("Found a bad fd %d, deactivating it!\n", -rc));
				NetFdsDeactivate(&lsh323fds[outid], -rc, FD_RW);

				break;
			}

			break;
		}
		hrstart = nx_gethrtime();
		seliSelect();
		hrend = nx_gethrtime();
		uh323Globals[outid].selims = (hrend -hrstart)/1000000;
		if(timeNow - lastTime >= 60)
		{
			lastTime = timeNow;
			CleanHalfClose();
		}
	}

	pthread_cleanup_pop(0);

	return 0;
}

// poll
int
IpcMainLoop (void)
{
	char fn[] = "IpcMainLoop():";
	int	retval = 0;
	sigset_t o_signal_mask, p_signal_mask;
	static struct timeval tout;
	int num = 0, rc = 0, numus = 0;
	int msec = 100;

	// Assign our thread id
	lsThread = pthread_self();
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	/* Loop forever */
	for (; ;)
	{
		// Find out how many entries we have used up first
		numus = NetFdsSetupPoll(&lsnetfds, MSEL, NETLOG_DEBUG4);
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s %d ready for gis\n", fn, numus));

		retval = poll(lsnetfds.pollA, num+numus, msec);

		switch (retval)
		{
		case -1:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s poll failure %d", fn, errno));
		
			break;
		case 0:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s poll timeout", fn));

			break;
		default:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("%s poll process", fn));
			retval = NetFdsProcessPoll(&lsnetfds, MSEL, NETLOG_DEBUG4);

			if (retval < 0)
			{
				NETERROR(MSEL,
					("%s Found a bad fd %d, deactivating it!\n", fn, -retval));
				NetFdsDeactivate(&lsnetfds, -retval, FD_RW);
			}

			break;
		}
	}

	return 0;
}
	
int
LusAdjustTimeout(struct timeval *tout, int *msec)
{
	int m1;

	// Find min
	if ((tout->tv_sec == (unsigned long)-1) &&
		(tout->tv_usec == (unsigned long)-1))
	{
		NETERROR(MTMR, ("LusAdjustTimeout, bad arg\n"));
		return -1;
	}

	m1 = tout->tv_sec*1000 + tout->tv_usec/1000;	

	if ((*msec == -1) || (m1 < *msec))
	{
		*msec = m1;
		return 1;
	}

	return 0;
}

int
LusSetupTimeout(TimerPrivate *timerPrivate, struct timeval *tout)
{
	static struct timeval now;
	static struct Timer front;
	int retval = 0;
	long secs, usecs, expecteds;

	tout->tv_sec = 0xfffffff;
	tout->tv_usec = 0xfffffff;

	/* Set up the timeout based on our own timer list */
	if (timerFront(timerPrivate, &front) != 0)
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

		if ((secs < 0) || (!secs && !usecs))
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, 
				("LusSetupTimeout:: secs is negative\n"));
			return -1;
		}

		expecteds = MAX(front.itime.it_value.tv_sec, 
							front.itime.it_interval.tv_sec)+1;

		if (secs > expecteds)
		{
			NETERROR(MSEL, 
				("LusSetupTimeout:: expected time of expiry %lds is greater than %lds\n", secs, expecteds));
			tout->tv_sec = expecteds;
			tout->tv_usec = 0;
		}
		else
		{
			tout->tv_sec = secs;
			tout->tv_usec = usecs;
		}

		retval = 1;
	}

	/* we are done... */
	return retval;
}

// Find the minimum of what is there in msec1 and msec2
// and return it
// A value of -1 in msec stands for infinite
int
LusAdjustTimeoutInMsec(int msec1, int msec2)
{
	int m1;

	if (msec1 == -1) return msec2;
	if (msec2 == -1) return msec1;

	if (msec1 < msec2) return msec1;
	else return msec2;
}

// Return 0 if there is nothing in the timer list
// 1 if there is still time to expite (and set the msec properly)
// and -1 if we find stuff in the list which is already expired
int
LusSetupTimeoutInMsec(TimerPrivate *timerPrivate, int *msec)
{
	char fn[] = "LusSetupTimeoutInMsec():";
	static struct timeval now;
	static struct Timer front;
	long secs, usecs, expecteds;

	*msec = -1;

	/* Set up the timeout based on our own timer list */
	if (timerFront(timerPrivate, &front) != 0)
	{
		gettimeofday(&now, NULL);

		NETDEBUG(MSEL, NETLOG_DEBUG4,
			("%s front=%ld.%ld, now=%ld.%ld\n",
			fn, front.expire.tv_sec,  front.expire.tv_usec,
			now.tv_sec, now.tv_usec));

		/* compute front - now, to return the delta */
		secs = front.expire.tv_sec - now.tv_sec;
		usecs = front.expire.tv_usec - now.tv_usec;

		if (usecs < 0)
		{
			secs --;
			usecs += 1000000;
		}

		if ((secs < 0) || (!secs && !usecs))
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, 
				("%s:: secs is negative\n",fn));
			return -1;
		}

		expecteds = MAX(front.itime.it_value.tv_sec, 
							front.itime.it_interval.tv_sec)+1;

		if (secs > expecteds)
		{
			*msec = expecteds*1000;
		}
		else
		{
			*msec = secs*1000 + usecs/1000;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

// For H.323, handle all the timers and
// all new calls, as there is no mechanism
// to separate the two. Note: This must be a
// separate thread than the main Thread, otherwise
// we end up polling the same fds in two places for
// H.323
void *
IpcHandleTimers(void *arg)
{
	char fn[] = "IpcHandleTimers():";
	int	retval = 0;
	sigset_t o_signal_mask, p_signal_mask;
	static struct timeval tout;
	int msec  = -1;
	int num = 0, rc = 0, numus = 0;

	timerThread = pthread_self();
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	/* Loop forever */
	for (; ;)
	{
		retval = LusSetupTimeout(&localConfig.timerPrivate, &tout);

		if (retval < 0)
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("timeout already\n"));
			serviceTimers(&localConfig.timerPrivate);
			continue;
		}

		msec  = -1;
		if (retval)
        {
              // Find the timeout
              LusAdjustTimeout(&tout, &msec);
        }

		// Find out how many entries we have used up first
		numus = NetFdsSetupPoll(&lstimerfds, MSEL, NETLOG_DEBUG4);
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%d ready for gis\n", numus));
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%d/%d ready in all\n", num+numus, 
				msec));

		retval = poll(lstimerfds.pollA, num+numus, msec);

		switch (retval)
		{
		case -1:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll failure %d", errno));
		
			break;
		case 0:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll timeout"));
			serviceTimers(&localConfig.timerPrivate);

			break;
		default:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll process"));
			retval = NetFdsProcessPoll(&lstimerfds, MSEL, NETLOG_DEBUG4);

			if (retval < 0)
			{
				NETERROR(MSEL,
					("Found a bad fd %d, deactivating it!\n", -retval));
				NetFdsDeactivate(&lstimerfds, -retval, FD_RW);
			}

			break;
		}
	}

	return 0;
}

int
HandleH323Notify(int fd, FD_MODE rw, void *data)
{
	char buff[100];
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;
	int res;

	if (data == NULL)
	{
		NETERROR(MINIT, ("tcbdata is NULL\n"));
	}
	else
	{
		tcbdata->rnotify++;
	}

	res = read(fd, buff, 1);	

	NETDEBUG(MINIT, NETLOG_DEBUG4,
		("Read %d notifications\n", res));

	return 0;
}

int
IServerH323Notify(void *data)
{
	char fn[] = "IServerNotify():";
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;

	if (tcbdata->wnotify != tcbdata->rnotify)
	{
		// already notification in queue
		NETDEBUG(MINIT, 4, ("%s Already Notified r=%x w=%x\n",
			fn, tcbdata->rnotify, tcbdata->wnotify));
		return 0;
	}

	if (write(tcbdata->notifyPipe[NOTIFY_WRITE]," ",1) < 0)
	{
		// Check the error
		if (errno == EAGAIN)
		{
			NETERROR(MINIT, ("Blocking error in notification\n"));
		}
	}
	else
	{
		tcbdata->wnotify ++;
	}

	return(0);
}

int
HandleNotify(int fd, FD_MODE rw, void *data)
{
	char buff[100];
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;
	int res;

	tcbdata->rnotify++;
	res = read(fd, buff, 1);	

	NETDEBUG(MINIT, NETLOG_DEBUG4,
		("Read %d notifications\n", res));

	return 0;
}

int
IServerNotify(void *data)
{
	char fn[] = "IServerNotify():";
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;

	if (tcbdata->wnotify != tcbdata->rnotify)
	{
		// already notification in queue
		NETDEBUG(MINIT, 4, ("%s Already Notified r=%x w=%x\n",
			fn, tcbdata->rnotify, tcbdata->wnotify));
		return 0;
	}

	if (write(tcbdata->notifyPipe[NOTIFY_WRITE]," ",1) < 0)
	{
		// Check the error
		if (errno == EAGAIN)
		{
			NETERROR(MINIT, ("Blocking error in notification\n"));
		}
	}
	else
	{
		tcbdata->wnotify ++;
	}

	return(0);
}

void *
HandlePollPM(void *arg)
{
	char fn[] = "HandlePollPM():";
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	/* Loop forever */
	for (;;)
	{
		iServerPoll(0);

		sleep(POLL_TIME_OUT);
	}

	return 0;
}

