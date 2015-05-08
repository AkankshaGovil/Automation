/*
 * $Id: header.c,v 1.1 1999/04/27 19:03:44 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/header.c,v $
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "ipc.h"

/*
 * put a header in the SA's outgoing packet buffer. This is incomplete
 * since we don't know the real packet length yet. throw() will fill
 * in the real size (and byte swap if necessary).
 */
void construct_header(SA *sa, unsigned char exch, unsigned long message_id,
			unsigned char nextp)
{
    isakmp_hdr hdr;

    /* The total length we will remember to fill on
     * when we are about to send this packet
     */

    bzero((char *)&hdr, sizeof(isakmp_hdr));
    if(sa->in_or_out == OUTBOUND){
	bcopy((char *)sa->my_cookie, (char *)&hdr.init_cookie, COOKIE_LEN);
	bcopy((char *)sa->his_cookie, (char *)&hdr.resp_cookie, COOKIE_LEN);
    } else {
	bcopy((char *)sa->his_cookie, (char *)&hdr.init_cookie, COOKIE_LEN);
	bcopy((char *)sa->my_cookie, (char *)&hdr.resp_cookie, COOKIE_LEN);
    }
    hdr.majver = ISAKMP_MAJOR_VERSION;
    hdr.minver = ISAKMP_MINOR_VERSION;
    hdr.exch = exch;
    hdr.len = sizeof(isakmp_hdr);
    hdr.next_payload = nextp;
    hdr.mess_id = message_id;
    expand_packet(sa, 0, sizeof(isakmp_hdr));
    bcopy((char *)&hdr, (sa->packet), sizeof(isakmp_hdr));
}



