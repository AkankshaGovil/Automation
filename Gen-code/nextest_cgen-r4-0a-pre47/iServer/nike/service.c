/* 
 * $Id: service.c,v 1.1 1999/04/27 19:03:48 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/service.c,v $
 */
/*
 *  Copyright Cisco Systems, Incorporated
 *
 *  Cisco Systems grants permission for redistribution and use in source 
 *  and binary forms, with or without modification, provided that the 
 *  following conditions are met:
 *     1. Redistribution of source code must retain the above copyright
 *        notice, this list of conditions, and the following disclaimer
 *        in all source files.
 *     2. Redistribution in binary form must retain the above copyright
 *        notice, this list of conditions, and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *
 *  "DISCLAIMER OF LIABILITY
 *  
 *  THIS SOFTWARE IS PROVIDED BY CISCO SYSTEMS, INC. ("CISCO")  ``AS IS'' 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CISCO BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE."
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include "service.h"

/*
 * service.c -- the bare bones (bb) service model. Input multiplexing and 
 * 	timer setting. Wow!
 */

#define TICK 1000000

static int next_timer = 0;

void add_time (struct timeval *t1, struct timeval *t2)
{
    t1->tv_sec += t2->tv_sec;
    t1->tv_usec += t2->tv_usec;

    if(t1->tv_usec > TICK){
	t1->tv_sec += (t1->tv_usec/TICK);
	t1->tv_usec %= TICK;
    }
}

void sub_time (struct timeval *t1, struct timeval *t2)
{
    t1->tv_sec -= t2->tv_sec;
    t1->tv_usec -= t2->tv_usec;

    if(t1->tv_usec < 0){
	t1->tv_sec--;
	t1->tv_usec += TICK;
    }

    if(t1->tv_sec < 0){
	t1->tv_sec = 0;
	t1->tv_usec = 0;
    }
}

/*
 * compare times, 0 is the highest.
 */
int cmp_time (struct timeval *t1, struct timeval *t2)
{
    if(t1->tv_sec > t2->tv_sec){
	if(t2->tv_sec)
	    return(1);
	else
	    return(-1);
    }
    if(t1->tv_sec < t2->tv_sec){
	if(t1->tv_sec)
	    return(-1);
	else 
	    return(1);
    }

    if(t1->tv_usec > t2->tv_usec){
	if(t2->tv_usec)
	    return(1);
	else
	    return(-1);
    }
    if(t1->tv_usec < t2->tv_usec){
	if(t1->tv_usec)
	    return(-1);
	else
	    return(1);
    }

    return(0);
}

static int cmp_timers (struct timer *t1, struct timer *t2)
{
    return(cmp_time(&(t1->to), &(t2->to)));
}

int bb_add_timeout (service_context context, int usec, 
		timercb proc, caddr_t data)
{
    struct timeval right_now;
    struct timezone tz;
    int id;

    context->timers[context->ntimers].to.tv_sec = usec/TICK;
    context->timers[context->ntimers].to.tv_usec = usec - ((usec/TICK)*TICK);
    id = context->timers[context->ntimers].id = ++next_timer;
    context->timers[context->ntimers].proc = proc;
    context->timers[context->ntimers].data = data;
    next_timer %= 32760;
    gettimeofday(&right_now, &tz);
    add_time(&(context->timers[context->ntimers].to), &right_now);
    (context->ntimers)++;
    if(context->ntimers > 1)
	qsort(context->timers, context->ntimers, sizeof(struct timer), 
		(int (*)())cmp_timers);
    return(id);
}

int bb_rem_timeout (service_context context, int id)
{
    int i;

    if(id < 1)
	return(0);
    for(i=0; i<FD_SETSIZE; i++){
	if(context->timers[i].id == id){
	    context->timers[i].id = 0;
	    context->timers[i].to.tv_sec = context->timers[i].to.tv_usec = 0;
	    if(context->ntimers > 1){
		qsort(context->timers, context->ntimers, sizeof(struct timer),
			(int (*)())cmp_timers);
	    }
	    (context->ntimers)--;
	    break;
	}
    }
    return(i < FD_SETSIZE);
}

void bb_add_input (service_context context, int fd, caddr_t data, inputcb proc)
{
    int i;

    for(i=0; i<context->ninputs; i++)
	if(context->inputs[i].fd == 0)
	    break;
    FD_SET(fd, &context->readfds);
    context->inputs[i].fd = fd;
    context->inputs[i].proc = proc;
    context->inputs[i].data = data;
    context->ninputs++;
    return;
}

void bb_rem_input (service_context context, int fd)
{
    int i;

    for(i=0; i<context->ninputs; i++){
	if(context->inputs[i].fd == fd){
	    context->inputs[i].fd = 0;
	    context->ninputs--;
	    FD_CLR(fd, &context->readfds);
	    return;
	}
    }
    return;
}

static void check_timers (service_context sc)
{
    struct timezone tz;
    struct timeval right_now, tdiff;
    int i = 0;

	/*
	 * the service context time out is since epoch. Find diff from
	 * now and assign to global timer. (timer 0 is the next to spring).
	 */
    if(sc->ntimers){
	/*
	 * first check to see if any sprung while we were off doing
	 * other things. If so then call their callback, and resort
	 */
	do {
	    gettimeofday(&right_now, &tz);
	    tdiff = sc->timers[i].to;
	    if(cmp_time(&tdiff, &right_now) < 1){
		(*sc->timers[i].proc)(sc->timers[i].id, sc->timers[i].data);
		sc->timers[i].id = 0;
		sc->timers[i].to.tv_sec = sc->timers[i].to.tv_usec = 0;
		i++;
	    }
	} while(cmp_time(&tdiff, &right_now) < 1);
	if(i){
	    if((sc->ntimers - i) > 1)
		qsort(sc->timers, sc->ntimers, sizeof(struct timer), 
				(int (*)())cmp_timers);
	    sc->ntimers -= i;
	}
	/*
	 * now the zero'th timer is the one that'll go off next
	 */
	if(sc->ntimers){
	    sub_time(&tdiff, &right_now);
	    sc->gbl_timer = tdiff;
	} else {
	    sc->gbl_timer.tv_sec = 1000;
	    sc->gbl_timer.tv_usec = 0;
	}
    } else {
	sc->gbl_timer.tv_sec = 1000;
	sc->gbl_timer.tv_usec = 0;
    }
    return;
}
	

void bbMainLoop(service_context sc)
{
    fd_set rfds;
    int i, active;
    while(1){
	check_timers(sc);
	bcopy((char *)&sc->readfds, (char *)&rfds,  sizeof(fd_set));
	if(sc->ninputs)
	  select(FD_SETSIZE, &rfds, NULL, NULL, &sc->gbl_timer);
	else
	  select(0, NULL, NULL, NULL, &sc->gbl_timer);
	active = 0;
	for(i=0; i<sc->ninputs; i++){
	    if(FD_ISSET(sc->inputs[i].fd, &rfds)){
		(*sc->inputs[i].proc)(sc->inputs[i].fd, sc->inputs[i].data);
		FD_CLR(sc->inputs[i].fd, &rfds);
		active = 1;
	    }
	}
	if(!active && sc->ntimers){
	    (*sc->timers[0].proc)(sc->timers[0].id, sc->timers[0].data);
	    sc->timers[0].id = 0;
	    sc->timers[0].to.tv_sec = sc->timers[0].to.tv_usec = 0;
	    if(sc->ntimers > 1)
		qsort(sc->timers, sc->ntimers, sizeof(struct timer),
			(int (*)())cmp_timers);
	    (sc->ntimers)--;
	}
    }
}

service_context bbCreateContext(void)
{
    service_context blah;

    blah = (service_context) malloc (sizeof(struct _servcxt));
    FD_ZERO(&blah->readfds);
    bzero((char *)blah->timers, (FD_SETSIZE * sizeof(struct timer)));
    bzero((char *)blah->inputs, (FD_SETSIZE * sizeof(struct source)));
    blah->gbl_timer.tv_sec = 1000;
    blah->gbl_timer.tv_usec = 0;

    return(blah);
}

