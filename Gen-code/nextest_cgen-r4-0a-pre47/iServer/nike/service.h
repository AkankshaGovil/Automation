/* 
 * $Id: service.h,v 1.1 1999/04/27 19:06:08 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/service.h,v $
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
#ifndef _SERVICE_H_
#define _SERVICE_H_

	/*
	 * definitions to use the bb service model. 
	 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

typedef void (*inputcb)(int fd, caddr_t data);
typedef void (*timercb)(int id, caddr_t data);

struct timer {
    struct timeval to;
    timercb proc;
    int id;
    caddr_t data;
};

struct source {
    int fd;
    inputcb proc;
    caddr_t data;
};

typedef struct _servcxt {
    fd_set readfds;
    struct timeval gbl_timer;
    int ntimers;
    struct timer timers[FD_SETSIZE];
    int ninputs;
    struct source inputs[FD_SETSIZE];
} servcxt;

typedef struct _servcxt *service_context;

void add_time(
	struct timeval *t1, 
	struct timeval *t2
);

void sub_time(
	struct timeval *t1, 
	struct timeval *t2
);

int cmp_time(
	struct timeval *t1, 
	struct timeval *t2
);

int bb_add_timeout(
	service_context context, 
	int usec, 
	timercb proc, 
	caddr_t data
);

int bb_rem_timeout(
	service_context context, 
	int id
);

void bb_add_input(
	service_context context, 
	int fd, 
	caddr_t data, 
	inputcb proc
);

void bb_rem_input(
	service_context context, 
	int fd
);

void bbMainLoop(
	service_context sc
);

service_context bbCreateContext(
	void
);

#endif

