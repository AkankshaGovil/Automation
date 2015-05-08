/* 
 * $Id: cookie.c,v 1.1 1999/04/27 19:03:44 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/cookie.c,v $
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
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "toolkit.h"
#include "isadb.h"
#include "protocol.h"

/*
 * cookie.c -- cookie generation routine and "random" number supplier.
 *	"random" numbers generated using the truerand() function.
 */

void get_rand(unsigned char *rand, int entropy)
{
}

void gen_cookie(struct sockaddr_in *addr, unsigned char *cookie)
{
}

