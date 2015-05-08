/*
 * Provide the timer task in the psos kernel, which
 * sends the SIGALRM to the task which has a timer
 * running. It uses the psos kernel call to send
 * and event to itself when the first timer in its queue
 * is about to expire. The task which owns this timer
 * is the one who receives the SIGALRM signal.
 */
#include <psos.h>

/* Some of these definitions may belong to a global file,
 * like the queue names used up here, to avoid
 * duplicate usage. These may be moved out of this file.
 *
#define TIMER_KERNEL_QUEUE "t1kq"
#define TIMER_TIMER_QUEUE "t2tq"

static unsigned long t1kq;
static unsigned long t2tq;
static unsigned long timer_events = 0;

static void
timer_init(void)
{
	char fn[] = "timer_init():";

	/* start the timer queues. We have one queue
	 * to receive events from the kernel. One
	 * to receive events from all the other tasks
	 */

	if ((q_create(TIMER_KERNEL_QUEUE, 0, Q_NOLIMIT, &t1kq) != 0) ||
		(q_create(TIMER_KERNEL_QUEUE, 0, Q_NOLIMIT, &t1kq) != 0))
	{
		log(LOG_DEBUG, 0, "%s q_create failed\n",
			fn);		
		return -1;
	}

	return 0;	
}

static void
timer_main(void)
{
     char fn[] = "timer_main():";
     unsigned long timer_events_received;

     /* Main loop for the timer task. Here we wait for the events
      * we have set in the event bit mask. The bit mask will always
      * have the timer queue event set. The system timer event may
      * or may not be set. We will wait for an OR of the events.
      * However the timer processing is always given higher priority.
      * We should check the timers even when processing the timer
      * queue events
      */
 _main_loop:

     timer_events_received = 0;

     if (ev_receive(timer_events, EV_WAIT|EV_ANY, 0, 
		timer_events_received) != 0)
     {
	  log(LOG_DEBUG, 0, "%s ev_receive returned error\n", 
	      fn);
     }

     if (BIT_SET(timer_events_received, TIMER_EVENT_SYSQ))
     {
	  /* The kernel gave us a timer event */
	  timer_timerHandler();
     }

     if (BIT_SET(timer_events_received, TIMER_EVENT_APPQ))
     {
	  timer_appHandler();
     }

     goto _main_loop;
}

timer_timerExpireCallback(tid t)
{
     /* Here we send an async signal to the task which
      * installed this timer
      */
}

static void
timer_appHandler()
{
     /* An event from a task has arrived. We must
      * read the app queue now and process the timer 
      * action - add/delete. Also, the call back functions the
      * service routine uses, have to be installed.
      * After that we must check whether we need to modify
      * the waiting time. If we do, whether we need to cancel an
      * existing system timer handler, and do another tm_wkwhen call.
      */
}


static void
timer_timerHandler()
{
     /*
      * some timer MAY have expired. Look at the first element in the
      * list and expire the timer.
      * If there are more elemnts in the list, restart the timer.
      */
     
     serviceTimers();
}




