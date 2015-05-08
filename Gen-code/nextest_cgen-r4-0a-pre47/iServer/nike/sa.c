/* 
 * $Id: sa.c,v 1.1 1999/04/27 19:03:47 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/sa.c,v $
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
#include "sha.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

int process_sa (SA *sa, unsigned char *payload, unsigned long mess_id)
{
    unsigned char *ptr, pseudo_rand[SHA_DIGEST_LENGTH], transform;
    sa_offer offer, *node;
    proposal_payload prop;
    int acceptable = 0, len, seq_num;
    sa_payload sa_sa;

    LOG((DEB,"checking ISA_SA"));

    bcopy((char *)payload, (char *)&sa_sa, sizeof(sa_payload));
    ntoh_sa(&sa_sa);
    if(sa_sa.doi != IPSEC_DOI)
    {
	LOG((ERR, "Unknown DOI of %ld!", sa_sa.doi));
	construct_header(sa, ISAKMP_INFO, 0, ISA_NOTIFY);
	len = sizeof(isakmp_hdr);
	return(1);
    }
    ptr = payload + sizeof(sa_payload);
	/*
	 * note that optional situation fields (e.g. secrecy level) are
	 * not supported. If they exist in the situation we will fail,
	 * which, I guess, is a good thing since we don't do any of that.
	 */
    bzero((char *)&offer, sizeof(sa_offer));
    if(mess_id)
    {
	acceptable = check_proposal(ptr, &offer, mess_id);
    } 
    else 
    {
	offer.next = NULL;
	bcopy((char *)ptr, (char *)&prop, sizeof(proposal_payload));
	ntoh_proposal(&prop);
	ptr += sizeof(proposal_payload);
	if(prop.protocol_id != PROTO_ISAKMP){
	    LOG((ERR, "protocol not ISAKMP in SA offer!"));
	    return(1);
	}
	acceptable = check_prop(ptr, PROTO_ISAKMP, prop.num_transforms,
				&transform, &offer.atts, &offer.natts);

	/* Set this to 1 for now... please... */
	acceptable = 1;
    }
    if(acceptable)
    {
	if(mess_id)
	{
	} 
	else 
	{
		/*
		 * if we're the responder then record the entire offer(s)
		 * sent by the initiator.
		 */
	    if(sa->init_or_resp == RESPONDER){
		sa->sap_len = sa_sa.payload_len - sizeof(sa_payload);
		sa->sap_offered = (unsigned char *) malloc (sa->sap_len);
		bcopy((char *)(payload + sizeof(sa_payload)), 
			(char *)sa->sap_offered, sa->sap_len);
	    }
	    sa->state_mask |= PROC_SA;
	    LOG((DEB,"Oakley proposal is acceptable"));
	}
    } else {
	construct_header(sa, ISAKMP_INFO, 0, ISA_NOTIFY);
	len = sizeof(isakmp_hdr);
    }
	/*
	 * even if unacceptable, one of an AND proposal might've been OK.
	 */ 
    delete_sa_offer(&offer);
    return(acceptable ? 0 : -1);
}

int construct_isakmp_sa (SA *sa, unsigned char nextp, int *pos)
{
    sa_payload sa_sa;
    int save_pos, fixpos, nbytes;

#if 0
    if(sa->state_mask & CONST_SA)
	return(0);
#endif
    LOG((DEB,"constructing ISA_SA for isakmp"));

    bzero((char *)&sa_sa, sizeof(sa_payload));
    save_pos = *pos;
    expand_packet(sa, save_pos, sizeof(sa_payload));
	/*
	 * we don't yet know the ultimate size of the sa offer so just
	 * leave some blank room and then add the proposal(s) and 
	 * transform(s). Fill in afterwards.
	 */
    *pos += sizeof(sa_payload);
    fixpos = *pos;
    nbytes = set_proposal(sa, fixpos, pos, PROTO_ISAKMP, 
			  NULL, 1);

    sa_sa.next_payload = nextp;
    sa_sa.payload_len = nbytes + sizeof(sa_payload);
    sa_sa.doi = IPSEC_DOI;
    sa_sa.situation = 1;
    hton_sa(&sa_sa);

    bcopy((char *)&sa_sa, (char *)(sa->packet + save_pos), 
						sizeof(sa_payload));
	/*
	 * if we're the initiator, record the entire offer before we send
	 * it off because if we offer more than one proposal the responder
	 * will select one and send it back (we don't want to do it above).
	 */
    if(sa->init_or_resp == INITIATOR){
	sa->sap_offered = malloc(nbytes);
	sa->sap_len = nbytes;
	if(sa->sap_offered == NULL){
	    LOG((ERR, "Out Of Memory"));
	    return(1);
	}
	bcopy((char *)(sa->packet + save_pos + sizeof(sa_payload)),
		sa->sap_offered, nbytes);
    }
    sa->state_mask |= CONST_SA;
    return(0);
}

int construct_ipsec_sa (SA *sa, sa_list *node, int *pos)
{
    sa_payload sa_sa;
    int save_pos, fixpos, nbytes;

    LOG((DEB,"constructing ISA_SA for ipsec"));

    bzero((char *)&sa_sa, sizeof(sa_payload));
    save_pos = *pos;
    expand_packet(sa, save_pos, sizeof(sa_payload));

    *pos += sizeof(sa_payload);
    fixpos = *pos;
    nbytes = set_ipsec_proposals(sa, fixpos, pos, node);
	/*
	 * determine whether all the ipsec_sas in the list were consumed
	 * in this single sa_payload. As long as the next payload has a
	 * flag set it will have already been taken care of in 
	 * set_ipsec_proposals. If no more ipsec_sas then the next payload
	 * is a NONCE, if there are more then the next one is an SA.
	 */
    while((node->next) && (node->next->flags)){
	node = node->next;
    }
    sa_sa.next_payload = (node->next == NULL) ? ISA_NONCE : ISA_SA;
    sa_sa.payload_len = nbytes + sizeof(sa_payload);
    sa_sa.doi = IPSEC_DOI;
    sa_sa.situation = 1;
    hton_sa(&sa_sa);

    bcopy((char *)&sa_sa, (char *)(sa->packet + save_pos), 
						sizeof(sa_payload));
    sa->state_mask |= CONST_NEG;
    return(0);
}

