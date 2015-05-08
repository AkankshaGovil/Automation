/* 
 * $Id: hash.c,v 1.1 1999/04/27 19:03:44 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/hash.c,v $
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
#include "md5.h"

/*
 * hash.c - process and construct hash payloads.
 */

#define HIS 1
#define MINE 2

int compute_hash(SA *sa, unsigned char *hash, int mine_or_his)
{
    return(0);
}

/*
 * handle hash payloads used for authentication in phase 1 processing
 */
int process_hash (SA *sa, unsigned char *payload, 
		unsigned long message_id)
{
    return(0);
}

int construct_hash (SA *sa, unsigned char nextp, int *pos)
{
    hash_payload hash;
    unsigned char hash_data[SHA_DIGEST_LENGTH];
    int nbytes, hash_len = 0;
 
    nbytes = sizeof(hash_payload);
    switch(sa->hash_alg){
	case HASH_SHA:
	    hash_len = SHA_DIGEST_LENGTH;
	    break;
	case HASH_MD5:
	    hash_len = MD5_DIGEST_LENGTH;
	    break;
    }
    nbytes += hash_len;
    bzero((char *)hash_data, hash_len);
    bzero((char *)&hash, sizeof(hash_payload));
    if(compute_hash(sa, hash_data, MINE)){
	fprintf(stderr,"Unable to compute hash!\n");
	return(1);
    }
    hash.next_payload = nextp;
    hash.payload_len = nbytes;
    hton_hash(&hash);

    expand_packet(sa, *pos, nbytes);
    bcopy((char *)&hash, (char *)(sa->packet + *pos),
		sizeof(hash_payload));
    *pos += sizeof(hash_payload);

    bcopy((char *)hash_data, (char *)(sa->packet + *pos), hash_len);
    *pos += hash_len;

    return(0);
}

int construct_blank_hash (SA *sa, unsigned char nextp, int *pos)
{
#if 0
    hash_payload hash;
    int nbytes, hash_len = 0;
 
    switch(sa->hash_alg){
	case HASH_SHA:
	    hash_len = SHA_LENGTH;
	    break;
	case HASH_MD5:
	    hash_len = MD5_LEN;
	    break;
    }
    nbytes = sizeof(hash_payload) + hash_len;
    bzero((char *)&hash, sizeof(hash_payload));

    hash.next_payload = nextp;
    hash.payload_len = nbytes;
    hton_hash(&hash);

    expand_packet(sa, *pos, nbytes);
    bcopy((char *)&hash, (char *)(sa->packet + *pos),
		sizeof(hash_payload));
    *pos += sizeof(hash_payload);
    *pos += hash_len;
#endif
    return(0);
}

