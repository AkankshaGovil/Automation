/* 
 * $Id: sig.c,v 1.2 1999/11/04 23:35:17 releasor Exp $
 * $Source: /export/home/cm/repos/nike/sig.c,v $
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
#include <arpa/inet.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "sha.h"

/*
 * sig.c - routines to handle the processing and construction of sig
 *	payloads.
 */

#define HIS 1
#define MINE 2

/*
 * DSS is tied to SHA so don't let negotiation of MD5 for HMAC reasons
 * screw up authentication. We want to do SHA regardless for this.
 */

static int process_dss_sig (SA *sa, DSA *dsa,
			unsigned char *payload, unsigned long mess_id)
{
     sig_payload sig;
     int nbytes, nsig;
     char digest[16], /* will be the message digest */
	  signature[256];
     int rc;

    /* Compute a hash of the payload first, the signature will be
     * posted right under where we are, so we will compute the hash of 
     * the packet starting from below the header, upto whatever we have
     * till now, and then we will create the digest below it. We will
     * sign this digest, into local memory, and copy the signature on
     * it
     */
     bcopy((char *)payload, (char *)&sig, sizeof(sig_payload));
     ntoh_sig(&sig);

     SHA1(sa->packet + sizeof(isakmp_hdr), 
	 (payload - sa->packet - sizeof(isakmp_hdr)),
	 (unsigned char *)digest);
    
     rc = DSA_verify(0, (unsigned char *)digest, 16, (unsigned char *)signature, 
		sig.payload_len - sizeof(sig_payload), 
		dsa);

     if (rc == -1)
     {
	  printf("DSS signature verification error\n");
	  return -1;
     }
     
     if (rc == 0)
     {
	  printf("DSS signature failed verification\n");
	  return -1;
     }

     printf("DSS signature verified...\n");

     return(0);
}

int process_sig (SA *sa, unsigned char *payload, unsigned long mess_id)
{
     
     if(sa->state_mask & PROC_SIG)
	  return(0);
     
     LOG((DEB,"processing ISA_SIG"));
     
     switch(sa->auth_alg){
     case DSS_SIG:
	  process_dss_sig(sa, mydsa, payload, mess_id);
	  break;
     default:
	  LOG((WARN, "unknown AUTH method!"));
	  return(1);
     }
     
     sa->state_mask |= PROC_SIG;
     
     return(0);
}

static int construct_dss_sig (SA *sa, DSA *dsa, 
			      unsigned char nextp, int *pos)
{
    sig_payload sig;
    int nbytes, nsig;
    char digest[16], /* will be the message digest */
	 signature[256];

    /* Compute a hash of the payload first, the signature will be
     * posted right under where we are, so we will compute the hash of 
     * the packet starting from below the header, upto whatever we have
     * till now, and then we will create the digest below it. We will
     * sign this digest, into local memory, and copy the signature on
     * it
     */

    SHA1(sa->packet + sizeof(isakmp_hdr), 
	 (*pos - sizeof(isakmp_hdr)),
	 (unsigned char *)digest);
    
    DSA_sign(0, (unsigned char *)digest, 16, (unsigned char *)signature, (unsigned int *)&nsig, dsa);

    nbytes = sizeof(sig_payload) + nsig;
    sig.next_payload = nextp;
    sig.payload_len = nbytes;

    hton_sig(&sig);

    expand_packet(sa, *pos, nbytes);
    bcopy((char *)&sig, (char *)(sa->packet + *pos), 
			sizeof(sig_payload));
    *pos += sizeof(sig_payload);

    bcopy(signature, (char *)(sa->packet + *pos), nsig);
    *pos += nsig;

    return(0);
}

int construct_sig (SA *sa, unsigned char nextp, int *pos)
{ 
     switch(sa->auth_alg){
     case DSS_SIG:
	  construct_dss_sig(sa, mydsa, nextp, pos);
	  break;
     default:
	  LOG((WARN, "unknown AUTH method!"));
	  return(1);
     }
     
    sa->state_mask |= CONST_SIG;

    return(0);
}

