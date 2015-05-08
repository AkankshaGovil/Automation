/* 
 * $Id: crypto.c,v 1.1 1999/04/27 19:03:44 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/crypto.c,v $
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
#include <sys/time.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

/*
 * crypto.c -- routines to encrypt and decrypt payloads 
 */
#include "rc4.h"

int decrypt_payload (SA *sa, unsigned char *payload, int len)
{
    int i = 0;

    unsigned char exp_key[128];

#if 0
    for (i = 0; i < len; i += 8)
    	BF_ecb_encrypt(payload+i, payload+i, &sa->symmkey, BF_DECRYPT);
#endif
    RC4(&sa->symmkey, len , payload, payload);
    return(0);
}

int encrypt_payload (SA *sa, unsigned char **payload, short offset, 
			unsigned  short *len)
{
     unsigned char *ptr, exp_key[128];
     int ncrypt = *len - offset;
     int oldlen = *len;
    int i = 0;
     
     if ((sa->dhsecret) && (sa->dhsecretlen > 0))
     {
	  /* Now we need to encrypt with whatever shared key is installed */
	  if (ncrypt % CYPHER_BLOCKLEN)
	  {
	       /* We must pad now */
	       ncrypt = (((ncrypt / CYPHER_BLOCKLEN) + 1) * CYPHER_BLOCKLEN);
	       *len = ncrypt + offset;
printf("FATAL ---------------- we have to realloc\n");
	       ptr = (unsigned char *) realloc (*payload, *len);
	       if(ptr == NULL)
	       {
		    LOG((CRIT,"Out Of Memory"));
		    return(-1);
	       }
	       memset(ptr + oldlen, 0, *len - oldlen);
	       *payload = ptr;
	  }
     }

     ptr = *payload + offset;
#if 0
    for (i = 0; i < ncrypt; i += 8)
    BF_ecb_encrypt(ptr+i, ptr+i, &sa->symmkey, BF_ENCRYPT);
#endif  
    RC4(&sa->symmkey, ncrypt, ptr, ptr);
    return(0);
}

