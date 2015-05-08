/* 
 * $Id: utils.c,v 1.1 1999/04/27 19:03:49 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/utils.c,v $
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
#include <unistd.h>
#include <errno.h>
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"

/*
 * list of weak and semi-weak DES keys 
 *    from Applied Cryptography: Protocols, Algorithms, and Source Code in C,
 *		first edition (yea, I've still got the 1st edition)
 *	   by Bruce Schneier
 */
#define NUM_WEAK_KEYS 16
unsigned char weak_keys[][DES_KEYLEN] = {
		/* the weak keys */
	{ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
	{ 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe },
	{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f },
	{ 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0 },
		/* the semi-weak keys */
	{ 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe },
	{ 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0 },
	{ 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0 }, 
	{ 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe }, 
	{ 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f },
	{ 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe },
	{ 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01 },
	{ 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f },
	{ 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01 },
	{ 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f },
	{ 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01 },
	{ 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0 }
};

/*
 * utils.c -- generic utilities. hton- and ntoh- routines for payloads,
 *	ISAKMP packet growth routine and DES is-/make- odd parity routines.
 */

#ifdef i386
/*
 * host <==> network routines for header
 */
void hton_hdr (isakmp_hdr *hdr)
{
    hdr->len = htonl(hdr->len);
    hdr->mess_id = htonl(hdr->mess_id);
}
void ntoh_hdr (isakmp_hdr *hdr)
{
    hdr->len = ntohl(hdr->len);
    hdr->mess_id = ntohl(hdr->mess_id);
}

/*
 * host <==> network routines for sa_payload
 */
void hton_sa (sa_payload *sa_sa)
{
    sa_sa->payload_len = htons(sa_sa->payload_len);
    sa_sa->doi = htonl(sa_sa->doi);
    sa_sa->situation = htonl(sa_sa->situation);
}
void ntoh_sa (sa_payload *sa_sa)
{
    sa_sa->payload_len = ntohs(sa_sa->payload_len);
    sa_sa->doi = ntohl(sa_sa->doi);
    sa_sa->situation = ntohl(sa_sa->situation);
}

void hton_dummy (generic_payload *dummy)
{
    dummy->payload_len = htons(dummy->payload_len);
}
void ntoh_dummy (generic_payload *dummy)
{
    dummy->payload_len = ntohs(dummy->payload_len);
}

/*
 * host <==> network routines for key exchange payload
 */
void hton_key (key_x_payload *key)
{
    key->payload_len = htons(key->payload_len);
}
void ntoh_key (key_x_payload *key)
{
    key->payload_len = ntohs(key->payload_len);
}

void ntoh_cert(
	cert_payload *cert
)
{
     cert->payload_len = ntohs(cert->payload_len);
}

void hton_cert(
	cert_payload *cert
)
{
     cert->payload_len = htons(cert->payload_len);
}

void hton_sig_req (sig_req_payload *key)
{
    key->payload_len = htons(key->payload_len);
}
void ntoh_sig_req (sig_req_payload *key)
{
    key->payload_len = ntohs(key->payload_len);
}

void hton_sig_rep (sig_rep_payload *key)
{
    key->payload_len = htons(key->payload_len);
}
void ntoh_sig_rep (sig_rep_payload *key)
{
    key->payload_len = ntohs(key->payload_len);
}

void hton_sig (sig_payload *sig)
{
    sig->payload_len = htons(sig->payload_len);
}
void ntoh_sig (sig_payload *sig)
{
    sig->payload_len = ntohs(sig->payload_len);
}

void hton_nonce (nonce_payload *nonce)
{
    nonce->payload_len = htons(nonce->payload_len);
}
void ntoh_nonce (nonce_payload *nonce)
{
    nonce->payload_len = ntohs(nonce->payload_len);
}

void hton_hash (hash_payload *hash)
{ 
    hash->payload_len = htons(hash->payload_len);
}
void ntoh_hash (hash_payload *hash)
{
    hash->payload_len = ntohs(hash->payload_len);
}

void hton_id (id_payload *id)
{
    id->payload_len = htons(id->payload_len);
}
void ntoh_id (id_payload *id)
{
    id->payload_len = ntohs(id->payload_len);
}

void hton_notify (notify_payload *notify)
{
    notify->payload_len = htons(notify->payload_len);
    notify->notify_message = htons(notify->notify_message);
    notify->spi[0] = htonl(notify->spi[0]);
    notify->spi[1] = htonl(notify->spi[1]);
}
void ntoh_notify (notify_payload *notify)
{
    notify->payload_len = ntohs(notify->payload_len);
    notify->notify_message = ntohs(notify->notify_message);
    notify->spi[0] = ntohl(notify->spi[0]);
    notify->spi[1] = ntohl(notify->spi[1]);
}

void hton_delete (delete_payload *del)
{
    del->payload_len = htons(del->payload_len);
}

void ntoh_delete (delete_payload *del)
{
    del->payload_len = ntohs(del->payload_len);
}

/*
 * host <==> network routines for proposals
 */
void hton_proposal (proposal_payload *prop)
{
    prop->payload_len = htons(prop->payload_len);
    prop->num_transforms = htons(prop->num_transforms);
    prop->spi = htonl(prop->spi);
}
void ntoh_proposal (proposal_payload *prop)
{
    prop->payload_len = ntohs(prop->payload_len);
    prop->num_transforms = ntohs(prop->num_transforms);
    prop->spi = ntohl(prop->spi);
}

void hton_transform (trans_payload *trans)
{
    trans->payload_len = htons(trans->payload_len);
}

void ntoh_transform (trans_payload *trans)
{
    trans->payload_len = ntohs(trans->payload_len);
}

/*
 * host <==> network routines for attributes
 */
void hton_att (sa_attribute *att)
{
    ushort blah;

    bcopy((char *)att, (char *)&blah, sizeof(ushort));
    blah = htons(blah);
    bcopy((char *)&blah, (char *)att, sizeof(ushort));
    att->att_type.basic_value = htons(att->att_type.basic_value);

}

void ntoh_att (sa_attribute *att)
{
    ushort blah;

    bcopy((char *)att, (char *)&blah, sizeof(ushort));
    blah = ntohs(blah);
    bcopy((char *)&blah, (char *)att, sizeof(ushort));
    att->att_type.basic_value = ntohs(att->att_type.basic_value);
}

void hton_payload (char *p, int len)
{
    int i;
    char temp;

    for(i=0; i<len; i+=2){
	temp = p[i];
	p[i] = p[i+1];
	p[i+1] = temp;
    }
}

#endif

/*
 * this grows the output packet of the SA as we create more piggybacked
 * payloads.
 */
void expand_packet (SA *sa, int position, int numbytes)
{
    int n;
    unsigned char *temp;

    if(sa->packet_len > (position + numbytes))
	return;
    n = (position + numbytes) - sa->packet_len;
    n = ROUNDUP(n);
    if(sa->packet_len == 0){
	sa->packet_len = n;
#if 0
	sa->packet = (unsigned char *) malloc (sa->packet_len);
#endif
    } else {
	sa->packet_len += n;
#if 0
	temp = (unsigned char *) realloc (sa->packet, sa->packet_len);
	sa->packet = temp;
#endif
    }
    if (sa->packet_len > sa->pmax_len)
    {
	printf("VERY FATAL NIKE ERROR: INCREASE the pmax_len inside the SA...\n");
	return;
    }
    bzero((char *)(sa->packet + position), (sa->packet_len - position));
}

int adjust_enc_payload(SA *sa, short offset, short mul)
{
	int old_len = sa->packet_len;

	if ((sa->packet_len - offset) % mul)
	{
		/* We need to re - adjust */
		sa->packet_len = ((sa->packet_len - offset)/mul + 1)*mul + offset;
		if (sa->packet_len > sa->pmax_len)
		{
			printf("FATAL ERROR in NIKE: INCREASE THE BUFFER\n");
			return;
		}
		/* bzero it */
		memset(sa->packet + old_len, 0,
				sa->packet_len - old_len);
		return 1;
	}
	
	return 0;
}	
/*
 * key[i] = odd_parity[key[i])] would've been preferable but
 * since the odd_parity array has been copyrighted (thanks alot!)
 * we'll do if(is_odd(key[i]) make_odd(&key[i]);
 */
static void make_odd (unsigned char *component)
{
    if((*component)%2)
	*component &= 0xfe;
    else
	*component |= 0x01;
}

static int is_odd (unsigned char component)
{
    int i=0;

    while(component){
	if(component%2)
	    i++;
	component = component>>1;
    }
    return(i%2);
}

#if 0
/*
 * take 1st DES_KEYLEN bytes of keybits which aren't in the weak key list
 */

static int copy_des_key(unsigned char *keybits, int keybitlen, 
			unsigned char *key)
{
    unsigned char des_key[DES_KEYLEN];
    int i,j, status = 1;

    if((keybits == NULL) || (key == NULL) || (keybitlen < DES_KEYLEN))
	return(status);

    for(i=0; i<(keybitlen - DES_KEYLEN); i++){
	status = 0;
	bcopy((char *)(keybits + i), des_key, DES_KEYLEN);
	for(j=0; j<DES_KEYLEN; j++)
	    if(!is_odd(des_key[j]))
		make_odd(&des_key[j]);
	for(j=0; j<NUM_WEAK_KEYS; j++)
	    if(bcmp(des_key, weak_keys[j], DES_KEYLEN) == 0){
		status = 1;
		break;
	    }
	if(status == 0){
	    bcopy(des_key, key, DES_KEYLEN);
	    break;
	}
    }
    return(status);
}

/*
 * generate SKEYID as defined in Oakley 
 */
void gen_skeyid (SA *sa)
{
    hmac_CTX context;
    unsigned char val, iv_stuff[SHA_LENGTH];

printf("generating skeyid\n");
    if(sa->hash_alg == HASH_MD5){
	sa->skeyid_len = MD5_LEN;
	sa->skeyid_e = (unsigned char *) malloc (MD5_LEN);
	if(sa->skeyid_e == NULL){
	    LOG((CRIT, "Out Of Memory"));
	    return;
	}
	sa->skeyid_a = (unsigned char *) malloc (MD5_LEN);
	if(sa->skeyid_a == NULL){
	    LOG((CRIT, "Out Of Memory"));
	    return;
	}
    } else {
	sa->skeyid_len = SHA_LENGTH;
	sa->skeyid_e = (unsigned char *) malloc (SHA_LENGTH);
	if(sa->skeyid_e == NULL){
	    LOG((CRIT, "Out Of Memory"));
	    return;
	}
	sa->skeyid_a = (unsigned char *) malloc (SHA_LENGTH);
	if(sa->skeyid_a == NULL){
	    LOG((CRIT, "Out Of Memory"));
	    return;
	}
    }
    sa->Inithmac(&context, sa->g_to_xy, sa->dh_len);
    if(sa->init_or_resp){
	sa->Updhmac(&context, sa->my_cookie, COOKIE_LEN);
	sa->Updhmac(&context, sa->his_cookie, COOKIE_LEN);
    } else {
	sa->Updhmac(&context, sa->his_cookie, COOKIE_LEN);
	sa->Updhmac(&context, sa->my_cookie, COOKIE_LEN);
    }
    sa->Updhmac(&context, sa->nonce_I, sa->nonce_I_len);
    sa->Updhmac(&context, sa->nonce_R, sa->nonce_R_len);
    val = 1;
    sa->Updhmac(&context, &val, sizeof(unsigned char));
    sa->Finhmac(sa->skeyid_a, &context);
    sa->Inithmac(&context, sa->g_to_xy, sa->dh_len);
    if(sa->init_or_resp){
	sa->Updhmac(&context, sa->my_cookie, COOKIE_LEN);
	sa->Updhmac(&context, sa->his_cookie, COOKIE_LEN);
    } else {
	sa->Updhmac(&context, sa->his_cookie, COOKIE_LEN);
	sa->Updhmac(&context, sa->my_cookie, COOKIE_LEN);
    }
    sa->Updhmac(&context, sa->nonce_I, sa->nonce_I_len);
    sa->Updhmac(&context, sa->nonce_R, sa->nonce_R_len);
    val = 0;
    sa->Updhmac(&context, &val, sizeof(unsigned char));
    sa->Finhmac(sa->skeyid_e, &context);
    switch(sa->encr_alg){
	case ET_DES_CBC:
	    copy_des_key(sa->skeyid_e, sa->skeyid_len, sa->crypt_key);
	    sa->crypt_iv_len = DES_KEYLEN;
	    sa->crypt_iv = (unsigned char *) malloc (DES_KEYLEN);
	    sa->InitHash(&context);
	    if(sa->init_or_resp == INITIATOR){
		sa->UpdHash(&context, sa->DH_pub_val, sa->dh_len);
		sa->UpdHash(&context, sa->his_DH_pub_val, sa->dh_len);
	    } else {
		sa->UpdHash(&context, sa->his_DH_pub_val, sa->dh_len);
		sa->UpdHash(&context, sa->DH_pub_val, sa->dh_len);
	    }
	    sa->FinHash(iv_stuff, &context);
	    bcopy((char *)iv_stuff, (char *)sa->crypt_iv, DES_KEYLEN);
	    break;
	default:
	    LOG((ERR,"Unknown crypto-alg"));
	    return;
    }
    return;
}

/*
 * generate the key the results from a Quick Mode exchange
 */
void gen_qm_key (SA *sa, unsigned char *nonce_I, unsigned short nonce_I_len,
		unsigned char *nonce_R, unsigned short nonce_R_len, 
		unsigned char *val, sa_list *node)
{
    hmac_CTX context;
    unsigned char digest[SHA_LENGTH];
    int hashlen;

    if(sa->hash_alg == HASH_MD5){
	hashlen = MD5_LEN;
    } else {
	hashlen = SHA_LENGTH;
    } 
    sa->Inithmac(&context, sa->skeyid_e, sa->skeyid_len);
    sa->Updhmac(&context, nonce_I, nonce_I_len);
    sa->Updhmac(&context, nonce_R, nonce_R_len);
    sa->Updhmac(&context, val, sizeof(unsigned char));
    sa->Finhmac(digest, &context);
    switch(node->other_sa.type){
	case PROTO_IPSEC_AH:
	    node->crypt_key = (unsigned char *) malloc (hashlen);
	    if(node->crypt_key == NULL){
		LOG((CRIT, "Out Of Memory"));
		return; }
	    node->crypt_keylen = hashlen;
	    bcopy((char *)digest, (char *)node->crypt_key, hashlen);
	    break;
	case PROTO_IPSEC_ESP:
	    switch(node->other_sa.esp.transform){
		case ESP_DES_CBC_TUNNEL:
		case ESP_DES_CBC_TRANSPORT:
		case ESP_DES_CBC_HMAC_REPLAY:
		    node->crypt_key = (unsigned char *) malloc (DES_KEYLEN);
		    node->crypt_keylen = DES_KEYLEN;
		    if(node->crypt_key == NULL){
			LOG((CRIT,"Out Of Memory"));
			return;
		    }
		    copy_des_key(digest, hashlen, node->crypt_key);
		    break;
		default:
		/*
		 * when adding crypto algs remember to inc val if need be
		 */
		    LOG((ERR,"Unknown crypto-alg"));
		    break;
	    }
	    break;
	default:
	    LOG((ERR, "Unknown protocol %d\n", node->other_sa.type));
    }
    return;
}
#endif
