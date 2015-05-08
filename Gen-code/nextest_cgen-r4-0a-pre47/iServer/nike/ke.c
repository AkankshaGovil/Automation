
/* 
 * $Id: ke.c,v 1.1 1999/04/27 19:03:46 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/ke.c,v $
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
#include "bn.h"
#include "dh.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "toolkit.h"

/*
 * ke.c -- routines to construct and process key exchange payloads 
 */
int process_ke (SA *sa, unsigned char *payload, unsigned long mess_id)
{
     key_x_payload ke;
     char *pubkey;
     
     if(sa->state_mask & PROC_KE)
     {
	  return(0);
     }
     /*
      * TODO: allow PFS for QM 
      */
     if(mess_id){
	  return(1);
     }
     LOG((DEB,"processing ISA_KE"));
     bcopy((char *)payload, (char *)&ke, sizeof(key_x_payload));
     ntoh_key(&ke);
     
     pubkey = (char *)malloc(ke.payload_len - sizeof(key_x_payload));
     
     bcopy((char *)(payload + sizeof(key_x_payload)), 
	   (char *)pubkey, 
	   (ke.payload_len - sizeof(key_x_payload)));
     
     /* Convert to BIGNUM format for storing this... */
     sa->hispub = BN_new();
     BN_bin2bn(pubkey, 
	       (ke.payload_len - sizeof(key_x_payload)),
	       sa->hispub);

#ifdef DONT_DO_HANDSHAKE
     /* If we are the initiator, we should store this into the new -- variables
      * and then when we get the reply bit set, we will move new into present
      */
     if (sa->dhsecret)
     {
	  free(sa->dhsecret);
     }

     sa->dhsecret = (char *)malloc(256);
     
     sa->dhsecretlen = DH_compute_key(sa->dhsecret, sa->hispub, mydh);
     
     printf("Generated DH shared key... len is %d\n", sa->dhsecretlen);

     /* Now generate the blowfish key also */
     BF_set_key(&sa->symmkey, sa->dhsecretlen, sa->dhsecret);

     sa->state_mask &= ~CRYPTO_ACTIVE;
#endif

     if (sa->newdhsecret)
     {
	  free(sa->newdhsecret);
     }

     sa->newdhsecret = (char *)malloc(256);
     
     sa->newdhsecretlen = DH_compute_key(sa->newdhsecret, sa->hispub, mydh);
     
     printf("Generated DH shared key... len is %d\n", sa->newdhsecretlen);

{int i; for (i=0; i< sa->newdhsecretlen; i++) printf("%x", sa->newdhsecret[i]);
printf("\n");}
     sa->state_mask |= PROC_KE;

     return(0);
}

int construct_ke (SA *sa, unsigned char nextp, int *pos)
{
    key_x_payload ke;
    int nbytes;

    if(sa->state_mask & CONST_KE)
	return(0);

    LOG((DEB,"constructing ISA_KE"));
    nbytes = sizeof(key_x_payload) + mydhpublen;

    bzero((char *)&ke, sizeof(key_x_payload));
    ke.next_payload = nextp;
    ke.payload_len = nbytes;
    hton_key(&ke);

    expand_packet(sa, *pos, nbytes);
    bcopy((char *)&ke,(char *)(sa->packet + *pos),sizeof(key_x_payload));
    *pos += sizeof(key_x_payload);
	/*
	 * we've gotta hton the key before we put it on the wire
	 */

    bcopy((char *)mydhpubstr, (char *)(sa->packet + *pos), mydhpublen);
    *pos += mydhpublen;

    sa->state_mask |= CONST_KE;
    return(0);
}

int construct_sig_req (SA *sa, unsigned char nextp, int *pos)
{
     sig_req_payload req; 
     int nbytes;
     
     if(sa->state_mask & CONST_SIGREQ)
     {
	  return(0);
     }
     
     LOG((DEB,"constructing ISA_SIGREQ"));
     nbytes = sizeof(sig_req_payload) + mydsacertlen;

     bzero((char *)&req, sizeof(sig_req_payload));
     req.next_payload = nextp;
     req.payload_len = nbytes;
     hton_sig_req(&req);

     expand_packet(sa, *pos, nbytes);
     bcopy((char *)&req,(char *)(sa->packet + *pos),sizeof(sig_req_payload));
     *pos += sizeof(sig_req_payload);
     
     /*
      * we've gotta hton the key before we put it on the wire
      */
     
     bcopy((char *)mydsacert, (char *)(sa->packet + *pos), mydsacertlen);
     *pos += mydsacertlen;
     
     sa->state_mask |= CONST_SIGREQ;
     return(0);
}

int process_sig_req (SA *sa, unsigned char *payload, unsigned long mess_id)
{
     sig_req_payload req;
     char *pubkey;
     N500Cert *cert;

     /* We have received a request to obtain a signed copy
      * of someone's public key certificate.
      * the server.
      */

     if(sa->state_mask & PROC_SIGREQ)
     {
	  return(0);
     }
     
     /*
      * TODO: allow PFS for QM 
      */
     if(mess_id)
     {
	  return(1);
     }
     
     LOG((DEB,"processing ISA_SIGREQ"));

     bcopy((char *)payload, (char *)&req, sizeof(sig_req_payload));
     ntoh_sig_req(&req);

     /* Add some bytes to compute the signature here.. */
     cert = (N500Cert *)malloc(req.payload_len - sizeof(sig_req_payload)
			       + 512); 
     memcpy(cert, (char *)(payload + sizeof(sig_req_payload)), 
	    (req.payload_len - sizeof(sig_req_payload)));

     printf("Received SIGREQ, dsa publen is %d\n", cert->pubkeylen);

     DSA_sign(0, (char *)cert, 
	      req.payload_len - sizeof(sig_req_payload),
	      (char *)(cert) + (req.payload_len - sizeof(sig_req_payload)),
	      &sa->hisdsacertlen, mydsa);

     sa->hisdsacertlen += (req.payload_len - sizeof(sig_req_payload));

     /* Now the payload length has increased by the the siglen */
     
     printf("Signed the DSA Certificate with my own public key...\n");
     
     sa->hisdsacert = cert;

     sa->state_mask |= PROC_SIGREQ;
     return(0);
}

int construct_sig_rep (SA *sa, unsigned char nextp, int *pos)
{
     sig_rep_payload rep; 
     int nbytes;
     
     if(sa->state_mask & CONST_SIGREP)
     {
	  return(0);
     }
     
     LOG((DEB,"constructing ISA_SIGREP"));
     nbytes = sizeof(sig_rep_payload) + sa->hisdsacertlen;

     bzero((char *)&rep, sizeof(sig_rep_payload));
     rep.next_payload = nextp;
     rep.payload_len = nbytes;
     hton_sig_rep(&rep);

     expand_packet(sa, *pos, nbytes);
     bcopy((char *)&rep,(char *)(sa->packet + *pos),sizeof(sig_rep_payload));
     *pos += sizeof(sig_rep_payload);
     
     
     bcopy((char *)sa->hisdsacert, 
	   (char *)(sa->packet + *pos), sa->hisdsacertlen);
     *pos += sa->hisdsacertlen;
     
     sa->state_mask |= CONST_SIGREP;
     return(0);
}

int process_sig_rep (SA *sa, unsigned char *payload, unsigned long mess_id)
{
     sig_rep_payload rep;
     char *pubkey;
     N500Cert *cert;

     /* We have received a signed copy of our certificate from
      * the server.
      */
     if(sa->state_mask & PROC_SIGREP)
     {
	  return(0);
     }
     
     /*
      * TODO: allow PFS for QM 
      */
     if(mess_id)
     {
	  return(1);
     }
     
     LOG((DEB,"processing ISA_SIGREP"));

     bcopy((char *)payload, (char *)&rep, sizeof(sig_rep_payload));
     ntoh_sig_rep(&rep);

     /* Here we must verify that the public key sent in the certificate
      * is ours.
      */
     cert = (N500Cert *)malloc(rep.payload_len - sizeof(sig_rep_payload));
     
     /* Now copy it... */
     memcpy(cert, (char *)(payload + sizeof(sig_rep_payload)), 
	    (rep.payload_len - sizeof(sig_rep_payload)));

     if (cert->pubkeylen != mydsapublen)
     {
	  printf("Pub key lengths do not match... \n");
	  return 1;
     }
     
     if (memcmp(cert->subject, mydsacert->subject, 
		strlen(mydsacert->subject)) != 0)
     {
	  printf("Subjects dont match...\n");
	  return 1;
     }
     
     if (memcmp(cert + 1, mydsacert + 1, mydsacert->pubkeylen) != 0)
     {
	  printf("Public Keys done match...\n");
	  return 1;
     }
     
     /* Now we should install this certificate... 
      */
     free(mydsacert);
     mydsacert = cert;
     
     printf("Installed DSA Certificate...\n");
     
     sa->state_mask |= PROC_SIGREP;
     return(0);
}

