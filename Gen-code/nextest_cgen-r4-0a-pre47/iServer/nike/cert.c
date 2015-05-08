/* 
 * $Id: cert.c,v 1.1 1999/04/27 19:03:44 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/cert.c,v $
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

/*
 * cert.c -- placeholder for eventual processing and construction of
 *	certificate payloads.
 */

int process_cert (SA *sa, unsigned char *payload, unsigned long mess_id)
{
    cert_payload cert;
    unsigned char *cert_data;

    /* Here we process the certificate, and install it... */
    LOG((DEB,"processing ISA_CERT"));
    bcopy((char *)payload, (char *)&cert, sizeof(cert_payload));
    ntoh_cert(&cert);
    cert_data = (unsigned char *) malloc (cert.payload_len);
    if(cert_data == NULL){
	LOG((CRIT,"Out Of Memory"));
	return(1);
    }
    bcopy((char *)(payload + cert.payload_len), (char *)cert_data, 
		cert.payload_len);
    
    /* Copy this.. */
    printf("Saving obtained certificate for later use...\n");
    sa->hisdsacert = cert_data;
    sa->hisdsacertlen = cert.payload_len;

    /* We should verify this certificate, if we have a public key
     * installed, from the Server.
     * It should be written inside the certificate, who the issuer
     * is, ie., the IP Address at least 4 now, and we will try and 
     * locate this in our server list, and use the unsigned certificate
     * we have from that server (LS/VPNS).
     * The application will supply the library with a linked list of trusted
     * certificates. The library will check a signed certificate against
     * one of these trusted certificates. RIght now, for simplicity, we
     * must have an unsigned certificate from the same issuer (containing
     * its pk basically), in order to verify.
     */

    printf("have to implement certificate verification\n");

    sa->state_mask |= PROC_AUTH;

    return(0);
}

int construct_cert (SA *sa, unsigned char nextp, int *pos)
{
    cert_payload cert;
    int nbytes;

    /* Here we send our DSS certificate to this other guy.. */
    LOG((DEB,"constructing ISA_CERT"));
    nbytes = sizeof(cert_payload) + mydsacertlen;

    bzero((char *)&cert, sizeof(cert_payload));
    cert.next_payload = nextp;
    cert.payload_len = nbytes;
    hton_cert(&cert);

    expand_packet(sa, *pos, nbytes);
    bcopy((char *)&cert, (char *)(sa->packet + *pos), 
			sizeof(cert_payload));
    *pos += sizeof(cert_payload);
    
    bcopy((char *)mydsacert, (char *)(sa->packet + *pos), mydsacertlen);
    *pos += mydsacertlen;

    sa->state_mask |= CONST_AUTH;

    return(0);
}

