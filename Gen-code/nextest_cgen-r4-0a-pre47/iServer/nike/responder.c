/* 
 * $Id: responder.c,v 1.1 1999/04/27 19:03:47 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/responder.c,v $
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
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

/*
 * responder.c -- routines unique to the responder portion of the ISAKMP
 *	protocol.
 */

SA *responder (isakmp_hdr *hdr, struct sockaddr_in *from, 
			unsigned char *payload)
{
    SA *security_assoc;
    struct hostent *me;
    struct utsname myself;

    /*
     * create an isadb entry. Type is zero because the responder 
     * doesn't know yet.
     */

    security_assoc = isadb_create_entry(from, RESPONDER, 0);
    gen_cookie(from, security_assoc->my_cookie);
    bcopy((char *)hdr->init_cookie, (char *)security_assoc->his_cookie,
	sizeof(security_assoc->his_cookie));
    security_assoc->payload = (unsigned char *) malloc 
				(hdr->len * sizeof(unsigned char));

    if(security_assoc->payload == NULL){
	LOG((CRIT,"Out Of Memory"));
	return(NULL);
    }

    security_assoc->payload_len = hdr->len;
    bcopy((char *)payload, (char *)security_assoc->payload, hdr->len);
    security_assoc->send_dir = BIDIR;

    return(security_assoc);
}









