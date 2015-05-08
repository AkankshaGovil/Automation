/* 
 * $Id: ikmpd.c,v 1.1.16.1 2004/08/03 20:33:06 amar Exp $
 * $Source: /export/home/cm/repos/nike/ikmpd.c,v $
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
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "service.h"
#include "cryptoparams.h"
#include "ipc.h"
#include "nike.h"

/*
 * isakmp.c -- main driver for ikmpd.
 */
int is, ks;
pid_t my_pid;
static int send_request (SA *secass);
service_context context;

/*
 * fire off the next volley to the other guy
 */
void throw (SA *secassoc)
{
     isakmp_hdr hdr;
	unsigned char *ptr;
     
     if(secassoc->packet_len){
	  if(secassoc->state_mask & CRYPTO_ACTIVE){
	       
	       bcopy((char *)secassoc->packet, 
		     (char *)&hdr, sizeof(isakmp_hdr));
	       print_hdr(&hdr);
		adjust_enc_payload(secassoc, sizeof(isakmp_hdr), 8);
		ptr = secassoc->packet;
	       if(encrypt_payload(secassoc, &ptr, 
				  sizeof(isakmp_hdr), &secassoc->packet_len)){
		    LOG((WARN,"Unable to encrypt payload!"));
		    return;
	       }
	  }
	  /* 
	   * the payload length may have changed since the header was
	   * written so make sure the correct length is there. I don't
	   * like it any more than you but it's better than remembering
	   * what the first payload was, malloc'ing space and building 
	   * the new payload = header+old_payload here.
	   */
	  bcopy((char *)&hdr, (char *)secassoc->packet, 
		sizeof(isakmp_hdr));
	  hdr.len = secassoc->packet_len;
	  if(secassoc->state_mask & CRYPTO_ACTIVE)
	  {
	       hdr.flags |= ISAKMP_HDR_ENCR_BIT;
	       printf("Setting the encryption bit in header\n");
	  }
	  hton_hdr(&hdr);
	  bcopy((char *)&hdr, (char *)secassoc->packet, 
		sizeof(isakmp_hdr));
	  
     }
}

/*
 * handlers for receiving a packet from the other guy. We catch and then
 * throw.
 */
	  
/* sin is the socket on which the packet might have come in
 * if it makes sense :)
 * otherwise we can always do a lookup based on from
 */
SA *
catcher (int sin, struct sockaddr_in *from, unsigned char *pkt)
{
     isakmp_hdr hdr;
     SA *secassoc, blank_sa;
     unsigned char *buffer = pkt;
     int fromlen, len, pos; 
     
     bcopy((char *)buffer, (char *)&hdr, sizeof(isakmp_hdr));
     
     ntoh_hdr(&hdr);

#ifdef DEBUG
     print_hdr(&hdr);
     
     print_vpi("--------------", (buffer + sizeof(isakmp_hdr)), 
	       hdr.len - sizeof(isakmp_hdr));
#endif
     
     secassoc = isadb_get_entry_by_addr2(from);
     
     if(secassoc == NULL)
     {
	  /*
	   * there is no db entry yet, it's a SA payload, and an exchange
	   * that's acceptable at this point, so let's respond, shall we?
	   * Unless, of course, the payload is invalid then throw it away.
	   */
	  if(hdr.flags & ISAKMP_HDR_ENCR_BIT)
	  {
	       /* We must first decrypt the packet
		* then validate the payload
		*/
	       secassoc = responder(&hdr, from, buffer);
	       
	       if((secassoc->state_mask & CRYPTO_ACTIVE) == 0)
	       {
		    LOG((WARN, 
			 "Send an encrypted packet when not crypto active!"));
		    return;
	       }
	       if(decrypt_payload(secassoc, (buffer+sizeof(isakmp_hdr)),
				  (hdr.len - sizeof(isakmp_hdr))))
	       {
		    LOG((WARN,"Unable to decrypt payload!\n"));
		    return;
	       }
	       
	       /* Copy the payload back into the sa - hehehehe */
	       bcopy((char *)buffer, (char *)secassoc->payload, hdr.len);
	  }
	  else
	  {
	       secassoc = responder(&hdr, from, buffer);
	  }

	  if(secassoc == NULL)
	  {
	       return;
	  }

	  if (validate_payload((hdr.len - sizeof(isakmp_hdr)), 
			       hdr.next_payload,
			       (buffer + sizeof(isakmp_hdr)), &len) == 0)
	  {
	       LOG((WARN, "Invalid payload. Possible overrun attack!"));
	       return;
	  }
	  
	  switch(hdr.exch)
	  {
	  case NIKE_PREREGISTRATION:
	       if(oakley_process_preregistration_mode(&hdr, secassoc) == 0)
	       {
		    throw(secassoc);
	       }
	       break;
	  case NIKE_CERTCOMM:
	       if(oakley_process_certcomm_mode(&hdr, secassoc) == 0)
	       {
		    throw(secassoc);
	       }
	       break;
	  }
     } 
     else 
     {	
	  /* Security Association exists... */
	  bb_rem_timeout(context, secassoc->to_id);
	 
	  if(secassoc->packet_len)
	  {
#if 0
	       free(secassoc->packet);
#endif
	       secassoc->packet_len = 0;
	  }
	  
	  if(hdr.flags & ISAKMP_HDR_ENCR_BIT)
	  {
	       /* We must first decrypt the packet
		* then validate the payload
		*/
	       if((secassoc->state_mask & CRYPTO_ACTIVE) == 0)
	       {
		    LOG((WARN, 
			 "Send an encrypted packet when not crypto active!"));
		    return;
	       }
	       
	       if(decrypt_payload(secassoc, (buffer+sizeof(isakmp_hdr)),
				  (hdr.len - sizeof(isakmp_hdr))))
	      {
		   LOG((WARN,"Unable to decrypt payload!\n"));
		   return;
	      }
	  }
	  
	  if(verify_hdr(&hdr, secassoc))
	  {
	       LOG((WARN,"Header invalid (verified)! (msg=%d)",
		    hdr.next_payload));
	       return;
	  }
	  
	  /*
	   * make a sanity check on this payload. Later parsing assumes 
	   * well formed payloads. Make sure that no payloads in here have
	   * bogus sizes that would cause us to lose our mind later on.
	   */
	  if(validate_payload((hdr.len - sizeof(isakmp_hdr)), 
			      hdr.next_payload,
			      (buffer + sizeof(isakmp_hdr)), &len) == 0)
	  {
	       LOG((WARN, "Invalid payload. Possible overrun attack!"));
	       print_hdr(&hdr);
	       return;
	  }
	  /*
	   * We've identified the SA in question and the payload looks good.
	   * Process it according to the exchange specified.
	   */
	  secassoc->payload = (unsigned char *) malloc 
	       (hdr.len * sizeof(unsigned char));
	  if(secassoc->payload == NULL)
	  {
	       LOG((CRIT,"Out Of Memory"));
	       return;
	  }
	  /*
	   * copy over the entire damned thing (just because, that's why)
	   * but note the actual length so QM hash checking will work right
	   */
	  secassoc->payload_len = hdr.len;
	  bcopy((char *)buffer, (char *)secassoc->payload, hdr.len);
	  if((len + sizeof(isakmp_hdr)) < hdr.len){
	       printf("hdr.len is %d but it's really %d\n", hdr.len, 
		      (len + sizeof(isakmp_hdr)));
	       hdr.len = len + sizeof(isakmp_hdr);
	  }
	  switch(hdr.exch)
	  {
	  case NIKE_PREREGISTRATION:
	       if (oakley_process_preregistration_mode(&hdr, secassoc))
	       {
		    LOG((CRIT, "Prereg processing failed"));
	       }
	       break;
	  case NIKE_CERTCOMM:
	       if (oakley_process_certcomm_mode(&hdr, secassoc))
	       {
		    LOG((CRIT, "certcomm failed"));
	       }
	       break;
	  default:
	       LOG((ERR, "Unknown exchange type, %d", hdr.exch));
	       return;
	  }
	  throw(secassoc);
     }

     return secassoc;
}

/* Our pitcher is the state machine rather than the kernel */
/* sin may be a socket over which communication might
 * be going on 
 */
SA *
pitcher (struct sockaddr_in *src,
	 struct sockaddr_in *dst,
	 unsigned char exch)
{
     char buff[1000], *ptr;
     SA *secassoc = 0;
     
     switch (exch)
     {
     case NIKE_PREREGISTRATION:
	  secassoc = initiate_preregistration(dst, 
					      src);
	  break;
     case NIKE_CERTCOMM:
	  secassoc = initiate_certcomm(dst, 
				       src);
	  break;
     default:
	  printf("Unknowm mode of operation...\n");
	  break;
     }
     
     if(secassoc != NULL)
     {
	  throw(secassoc);
     }
     
     return secassoc;
}


/* Put these globals into a common struct or something... */
struct in_addr myaddress;
unsigned long myserialno;

DH *mydh;
char *mydhpubstr;
int mydhpublen;
DSA *mydsa;
char *mydsapubstr;
int mydsapublen;
N500Cert *mydsacert;
int mydsacertlen;

int 
nike_init (unsigned long ipaddress, unsigned long serialno)
{
	FILE *tmpf;
	N500Cert *cert;

	/* Set myaddress here */
	myaddress.s_addr = ipaddress;
	myserialno = serialno;

	printf("generating my global diffie hellman key \n");
	/* Init my diffie hellman values */
	mydh = DH_new();

#if 0
	tmpf = fopen("/export/home/medhavi/netoids/src/dh.tmp",
				"r");
	d2i_DHparams_fp(tmpf, &mydh);
	fclose(tmpf);
     printf("p %s\n", BN_bn2hex(mydh->p));
     printf("\n");
     printf("g %s\n", BN_bn2hex(mydh->g));
#endif
#if 1
	N500DH_SetParams(mydh);
#endif
	
	DH_generate_key(mydh);

	mydhpublen = BN_num_bytes(mydh->pub_key);
	mydhpubstr = (char *)malloc(mydhpublen);
	mydhpublen = BN_bn2bin(mydh->pub_key, mydhpubstr);

	printf("Computed the DH keys. publen is %d\n", mydhpublen);

	/* Also generate my DSA public key */
	mydsa = DSA_new();
#if 0	
	/* Read it from some file, initialize it... */
	tmpf = fopen("/export/home/medhavi/netoids/src/dsa.tmp",
				"r");
	d2i_DSAparams_fp(tmpf, &mydsa);
	fclose(tmpf);
#endif
	N500DSA_SetParams(mydsa);

	DSA_generate_key(mydsa);
	
	mydsapublen = i2d_DSAPublicKey(mydsa, 0);
	mydsapubstr = (char *)malloc(mydsapublen);
	mydsapublen = i2d_DSAPublicKey(mydsa, (unsigned char **)&mydsapubstr);

	printf("Computed the DSA keys, publen is %d\n", mydsapublen);
	
	/* Now put this key inside a certificate, which is not yet 
	 * signed, because we will get it signed when we send it to the
	 * server
	 */
	
	mydsacertlen = mydsapublen + sizeof(N500Cert);

	printf("Computed the DSA cert, certlen is %d\n", mydsacertlen);

	mydsacert = (N500Cert *)malloc(mydsacertlen);

	cert = (N500Cert *)mydsacert;
	cert->serialno = serialno;
	sprintf(cert->subject, "%ld", serialno);
	cert->sigalgorithm = DSS_SIG;
	cert->algorithm = 0; /* We will start filling this later */
	cert->pubkeylen = mydsapublen;
	memcpy(cert + 1, mydsapubstr, mydsapublen);

	/* Also invoke the signing setup functions as an aid to procomputation
	 */
	{
	  BN_CTX *ctx_in = BN_CTX_new();
	  BIGNUM *kinv = 0, *r = 0;

	  DSA_sign_setup(mydsa, ctx_in, &kinv, &r);
	}
}
 
