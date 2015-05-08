/*
 * $Id: oakley.c,v 1.1 1999/04/27 19:03:46 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/oakley.c,v $
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
#ifndef MULTINET
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#else
#include <types.h>
#endif /* MULTINET */
#include <netinet/in.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

/*
 * oakley.c - as defined by draft-ietf-ipsec-isakmp-oakley-01.txt
 */

static void load_all_sas(SA *sa, unsigned long mess_id)
{
    conn_entry *centry;
    sa_list *node;
    unsigned char val = 0;

    centry = get_centry_by_mess_id(mess_id);
    if(centry == NULL){
	LOG((ERR,"Cannot Store <NULL> security associations!"));
	return;
    }
    node = centry->sa_s;
	/*
	 * generate keys for all negotiated SAs. "val" is a running counter
	 * that in incremented by gen_qm_key to guarantee uniqueness.
	 */
    while(node != NULL){
	if(node->spi && node->other_spi){
#if 0
	    gen_qm_key(sa, centry->nonce_I, centry->nonce_I_len, 
			   centry->nonce_R, centry->nonce_R_len, &val, node);
	    load_sa(sa, node, centry, INITIATOR);
	    load_sa(sa, node, centry, RESPONDER);
#endif
	}
	node = node->next;
    }
}

/* The state machine for pre-registration
 * behaves as follows: No state -> ini-exchange -> Authenticated.
 * The terminology we use is Initiator and Responder, and both
 * move thru the same set of states. The following routine will be
 * invoked only as a response to some packet received.
 */
int 
oakley_process_preregistration_mode (isakmp_hdr *hdr, SA *sa)
{
     unsigned char *pptr;
     unsigned long mess_id;
     unsigned short expected_state;
     int pos;
     
     pptr = sa->payload + sizeof(isakmp_hdr);
     mess_id = hdr->mess_id;

     switch(sa->state){
	  
     case OAK_PREREG_NOSTATE:
	  /* Only the Responder can be in this state at this time */
	  if(sa->init_or_resp == INITIATOR)
	  {
	       /* How can we be the initiator, and havbe no state... 
		* There is obviously something wrong
		*/
	       printf("Initiator received unexpected packet\n");
	       return 1;
	  }
	  
	  /* This is a fresh message from an initiator */
	  if(process(hdr->next_payload, sa, 
		     (sa->payload + sizeof(isakmp_hdr)), mess_id))
	  {
	       return(1);
	  }

	  expected_state = PROC_SA | PROC_KE | PROC_SIGREQ;

	  if((sa->state_mask & expected_state) != expected_state)
	  {
	       return(1);
	  }

	  /* Seems that we obtained whatever we needed,
	   * An SA, A DH Key from the initiator, a DSS public
	   * Key from the initiator.
	   * Now we will send a reply back.
	   * The reply should consist of an SA, 
	   * KE containing our DH key, our DSS public key,
	   * and the signed DSS public key of the netoid.
	   */
	  construct_header(sa, NIKE_PREREGISTRATION, 0, ISA_SA);
	  pos = sizeof(isakmp_hdr);
	  construct_isakmp_sa(sa, ISA_KE, &pos);
	  construct_ke(sa, ISA_SIGREP, &pos);
	  construct_sig_rep(sa, ISA_CERT, &pos);
	  construct_cert(sa, 0, &pos);
#if 0
	  switch(sa->auth_alg) {
	  case DSS_SIG:
	  case RSA_SIG:
	       construct_id(sa, ISA_SIG, &pos);
	       construct_sig(sa, 0, &pos);
	       break;
	  case RSA_ENC:
	  case PRESHRD:
	       construct_id(sa, ISA_HASH, &pos);
	       construct_hash(sa, 0, &pos);
	       break;
	  }
#endif
	  sa->state = OAK_PREREG_INIT_EXCH;
	  sa->send_dir = BIDIR;
	  sa->init_or_resp = RESPONDER;

	  break;
     case OAK_PREREG_INIT_EXCH:
	  /* We may be the responder or the initiator. The expected
	   * messages are going to be different though.
	   * The initiator needs a KE, SIGREP, AUTH,
	   * while a responder, already has all that information.
	   * He just needs an SA, probably some ID.
	   */
	  if(sa->init_or_resp == INITIATOR)
	  {
	       expected_state = PROC_SA | PROC_KE | PROC_SIGREP | PROC_AUTH;
	  }
	  else
	  {
	       expected_state = PROC_SA;
	  }
	  
	  if(process(hdr->next_payload, sa, 
		     (sa->payload + sizeof(isakmp_hdr)), mess_id))
	  {
	       return(1);
	  }


	  if((sa->state_mask & expected_state) != expected_state)
	  {
	       return(1);
	  }

	  sa->state = OAK_PREREG_AUTH;

	  /* Also , if we are the initiator, we must send back some response
           * encrypted with the old key for now, otherwise, we install the 
	   * new key
	   */

	  if(sa->init_or_resp == INITIATOR)
	  {
	       construct_header(sa, NIKE_PREREGISTRATION, 0, ISA_SA);
	       pos = sizeof(isakmp_hdr);
	       construct_isakmp_sa(sa, 0, &pos);
	  }
	  else
	  {
	  	/* Install the new secret key. */
	  	if (sa->newdhsecretlen > 0)
	  	{
	       		/* Now we can install the shared key */
	       		if (sa->dhsecret)
	       		{
			    free (sa->dhsecret);
	       		}

	       		sa->dhsecret = (char *)malloc(sa->newdhsecretlen);
	       		memcpy(sa->dhsecret, sa->newdhsecret, sa->newdhsecretlen);
	       		sa->dhsecretlen = sa->newdhsecretlen;
	       		free(sa->newdhsecret);
	       		sa->newdhsecret = 0;
	       		sa->newdhsecretlen = 0;
	       		printf("Copied the new dh secret...\n");
	   
#if 0
	       		BF_set_key(&sa->symmkey, sa->dhsecretlen, sa->dhsecret);
#endif
	       		RC4_set_key(&sa->symmkey, sa->dhsecretlen, sa->dhsecret);
	  	}
	  }

	  break;

     case OAK_PREREG_AUTH:
	  /* We are already authenticated. So why are we still getting messages...
	   * Ignore...
	   */
	  break;
     }
     return(0);
}

/* The state machine for certcomm
 * behaves as follows: No state -> init-exchange -> Authenticated.
 * The terminology we use is Initiator and Responder, and both
 * move thru the same set of states. The following routine will be
 * invoked only as a response to some packet received.
 */
int 
oakley_process_certcomm_mode (isakmp_hdr *hdr, SA *sa)
{
     unsigned char *pptr;
     unsigned long mess_id;
     unsigned short expected_state;
     int pos;
     
     pptr = sa->payload + sizeof(isakmp_hdr);
     mess_id = hdr->mess_id;

     switch(sa->state){
	  
     case OAK_CERTCOMM_NOSTATE:
	  /* Only the Responder can be in this state at this time */
	  if(sa->init_or_resp == INITIATOR)
	  {
	       /* How can we be the initiator, and havbe no state... 
		* There is obviously something wrong
		*/
	       printf("Initiator received unexpected packet\n");
	       return 1;
	  }
	  
	  /* This is a fresh message from an initiator */
	  if(process(hdr->next_payload, sa, 
		     (sa->payload + sizeof(isakmp_hdr)), mess_id))
	  {
	       return(1);
	  }

	  expected_state = PROC_SA | PROC_KE | PROC_AUTH | PROC_SIG;

	  if((sa->state_mask & expected_state) != expected_state)
	  {
	       return(1);
	  }

	  /* Seems that we obtained whatever we needed,
	   * An SA, A DH Key from the initiator, a DSS public
	   * Key from the initiator.
	   * Now we will send a reply back.
	   * The reply should consist of an SA, 
	   * KE containing our DH key, our DSS public key,
	   * and the signed DSS public key of the netoid.
	   */
	  construct_header(sa, NIKE_CERTCOMM, 0, ISA_SA);
	  pos = sizeof(isakmp_hdr);
	  construct_isakmp_sa(sa, ISA_KE, &pos);
	  construct_ke(sa, ISA_CERT, &pos);
	  construct_cert(sa, ISA_SIG, &pos);
	  construct_sig(sa, 0, &pos);
#if 0
	  switch(sa->auth_alg) {
	  case DSS_SIG:
	  case RSA_SIG:
	       construct_id(sa, ISA_SIG, &pos);
	       construct_sig(sa, 0, &pos);
	       break;
	  case RSA_ENC:
	  case PRESHRD:
	       construct_id(sa, ISA_HASH, &pos);
	       construct_hash(sa, 0, &pos);
	       break;
	  }
#endif
	  sa->state = OAK_CERTCOMM_INIT_EXCH;
	  sa->send_dir = BIDIR;
	  sa->init_or_resp = RESPONDER;

	  break;
     case OAK_CERTCOMM_INIT_EXCH:
	  /* We may be the responder or the initiator. The expected
	   * messages are going to be different though.
	   * The initiator needs a KE, SIGREP, AUTH,
	   * while a responder, already has all that information.
	   * He just needs an SA, probably some ID.
	   */
	  if(sa->init_or_resp == INITIATOR)
	  {
	       expected_state = PROC_SA | PROC_KE | PROC_AUTH | PROC_SIG;
	  }
	  else
	  {
	       expected_state = PROC_SA;
	  }
	  
	  if(process(hdr->next_payload, sa, 
		     (sa->payload + sizeof(isakmp_hdr)), mess_id))
	  {
	       return(1);
	  }


	  if((sa->state_mask & expected_state) != expected_state)
	  {
	       return(1);
	  }

	  sa->state = OAK_CERTCOMM_AUTH;

	  /* Also , if we are the initiator, we must send back some response
           * encrypted with the old key for now, otherwise, we install the 
	   * new key
	   */

	  if(sa->init_or_resp == INITIATOR)
	  {
	       construct_header(sa, NIKE_CERTCOMM, 0, ISA_SA);
	       pos = sizeof(isakmp_hdr);
	       construct_isakmp_sa(sa, 0, &pos);
	  }
	  else
	  {
	  	/* Install the new secret key. */
	  	if (sa->newdhsecretlen > 0)
	  	{
	       		/* Now we can install the shared key */
	       		if (sa->dhsecret)
	       		{
			    free (sa->dhsecret);
	       		}

	       		sa->dhsecret = (char *)malloc(sa->newdhsecretlen);
	       		memcpy(sa->dhsecret, sa->newdhsecret, sa->newdhsecretlen);
	       		sa->dhsecretlen = sa->newdhsecretlen;
	       		free(sa->newdhsecret);
	       		sa->newdhsecret = 0;
	       		sa->newdhsecretlen = 0;
	       		printf("Copied the new dh secret...\n");
	   
#if 0
	       		BF_set_key(&sa->symmkey, sa->dhsecretlen, sa->dhsecret);
#endif
	       		RC4_set_key(&sa->symmkey, sa->dhsecretlen, sa->dhsecret);
	  	}
	  }

	  break;

     case OAK_CERTCOMM_AUTH:
	  /* We are already authenticated. So why are we still getting messages...
	   * Ignore...
	   */
	  break;
     }
     return(0);
}
