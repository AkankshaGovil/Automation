/* 
 * $Id: misc.c,v 1.1 1999/04/27 19:03:46 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/misc.c,v $
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

int valid_payload(int *, int, unsigned char *, unsigned char *);

/*
 * misc.c -- routines which have no other place to go. Verify header check,
 *	payload validation, and no-op procedure calls.
 */

int verify_hdr (isakmp_hdr *hdr, SA *sa)
{
    if(sa->in_or_out == OUTBOUND){
	if(bcmp((char *)hdr->init_cookie, sa->my_cookie, COOKIE_LEN)){
	    return(-1);
	}
	if((hdr->next_payload == ISA_SA) && 
	  ((hdr->exch == OAK_MM) || (hdr->exch == OAK_AG) ||
	       (hdr->exch == NIKE_PREREGISTRATION) )){
	    bcopy((char *)hdr->resp_cookie, sa->his_cookie, COOKIE_LEN);
	    return(0);
	}
	if(bcmp((char *)hdr->resp_cookie, sa->his_cookie, COOKIE_LEN)){
	    return(-1);
	}
    } else {
	if(bcmp((char *)hdr->resp_cookie, sa->my_cookie, COOKIE_LEN)){
	    return(-1);
	}
	if(bcmp((char *)hdr->init_cookie, sa->his_cookie, COOKIE_LEN)){
	    return(-1);
	}
    }
    return(0);
}

static int valid_transform (unsigned short *tlen, unsigned char **payload)
{
    trans_payload trans;
    sa_attribute att;
    unsigned char *p;
    int len = 0;

    bcopy((char *)*payload, (char *)&trans, sizeof(trans_payload));
    ntoh_transform(&trans);
    if(trans.payload_len > *tlen)
	return(0);
	/*
	 * after checking transform length against total length allowed,
	 * quickly run through all attributes to see if any overrun the
	 * proposal length or would screw up the parser (e.g. vpi of 
	 * length zero!)
	 */
    len = sizeof(trans_payload);
    while(len < trans.payload_len){
	p = *payload + len;
	bcopy((char *)p, (char *)&att, sizeof(sa_attribute));
	ntoh_att(&att);
	if(att.type == 0){
	    if(att.att_type.vpi_length == 0)
		return(0);
	    len += att.att_type.vpi_length;
	}
	len += sizeof(sa_attribute);
    }
    *payload += trans.payload_len;
    return(len == trans.payload_len);
}

static int valid_sa (int *running_len, int total_len, unsigned char *payload)
{
    int i, len = 0;
    sa_payload sa;
    proposal_payload prop;
    unsigned char *ptr;

    bcopy((char *)payload, (char *)&sa, sizeof(sa_payload));
    ntoh_sa(&sa);
printf("a sa of length %d\n", sa.payload_len);
    if(((*running_len + sa.payload_len) > total_len) ||
	(sa.payload_len == 0))
	return(0);
	/*
	 * after a quick sanity check run through all proposals in this
	 * SA payload. Make sure they're all OK individually, then make 
	 * sure their sum doesn't overrun
	 */
    len = sizeof(sa_payload);
    while(len < sa.payload_len){
	ptr = payload + len;
	bcopy((char *)ptr, (char *)&prop, sizeof(proposal_payload));
	ntoh_proposal(&prop);
	if((prop.payload_len == 0) || (prop.num_transforms == 0))
	    return(0);
	ptr += sizeof(proposal_payload);
	for(i=0; i<prop.num_transforms; i++){
	    if(valid_transform(&prop.payload_len, &ptr) == 0)
		return(0);
	}
	len += prop.payload_len;
    }
    if(len != sa.payload_len)
	return(0);
    *running_len += sa.payload_len;
    return(1);
}

int valid_payload (int *running_len, int total_len, unsigned char *ptype,
		unsigned char *payload)
{
    generic_payload dummy;

    bcopy((char *)payload, (char *)&dummy, sizeof(generic_payload));
    ntoh_dummy(&dummy);
    if(dummy.payload_len < 1)
	return(0);
	/*
	 * SA payload is a special case and require unique checking.
	 */
    if(*ptype == ISA_SA){
	if(valid_sa(running_len, total_len, payload) == 0)
	    return(0);
    } else {
	*running_len += dummy.payload_len;
    }
printf("it's %d\n", dummy.payload_len);
    *ptype = dummy.next_payload;
    return(*running_len <= total_len);
}

int validate_payload (int total_length, unsigned char payload_type,
		unsigned char *payload, int *actual_length)
{
    unsigned char *p = payload;
	/*
	 * Run a sanity check on the payload. make sure that no interior 
	 * payloads have lengths that would exceed the payload's total length,
	 * also make sure that the assumptions the payload parsers make
	 * will be valid (e.g. incrememt a pointer by payload_length. If the
	 * length is zero we'll loop forever).
	 */
    *actual_length = 0;
    while(payload_type){
printf("checking payload %d, at %d of %d...", payload_type, *actual_length, total_length);
	if(valid_payload(actual_length, total_length, &payload_type, p) == 0)
	    return(0);
	p = payload + *actual_length;
    }
    return(!(*actual_length > total_length));/* allow length < total length */
}					     /* since there might be padding */

int process_err (SA *sa, unsigned char *payload, unsigned long mess_id)
{
    LOG((ERR,"Asked to process a 'no next payload'!"));
    return(1);
}

int construct_err (SA *sa, unsigned char nextp, int *pos)
{
    LOG((ERR,"Asked to construct a 'no next payload' with next payload %d", nextp));
    return(1);
}

int process_noop (SA *sa, unsigned char *payload, unsigned long mess_id)
{
    return(0);
}

