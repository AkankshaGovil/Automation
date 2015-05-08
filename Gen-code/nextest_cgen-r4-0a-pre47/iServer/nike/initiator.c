/* 
 * $Id: initiator.c,v 1.1 1999/04/27 19:03:45 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/initiator.c,v $
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
#include <sys/time.h>
#include "bn.h"
#include "dh.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "toolkit.h"
#include "ipc.h"

/*
 * initiator.c -- routines specific to the initiator portion of ISAKMP
 *	protocol. Since it proposes and the responder accepts (or rejects)
 *	these are initiator-specific.
 */

int 
initiate_preregistration_mode (SA *sa)
{
     int pos;
     
     /*
      * these algs should, ideally, come from the PF_KEY, and they should
      * have some implied preference and logical construction. For now just
      * stick 'em here in plain serial, logical OR.
      */
    sa->hash_alg = HASH_SHA;
    sa->encr_alg = ET_DES_CBC;
    sa->auth_alg = PRESHRD;

    sa->group_desc = 1;		/* the default Oakley group */

    /* Nike : We use the Aggressive mode */
    construct_header(sa, NIKE_PREREGISTRATION, 0, ISA_SA);
    pos = sizeof(isakmp_hdr);
    construct_isakmp_sa(sa, ISA_KE, &pos);
    construct_ke(sa, ISA_SIGREQ, &pos);
    construct_sig_req(sa, 0, &pos);
#if 0
    switch (sa->auth_alg)
    {
	case PRESHRD:
		construct_id(sa, ISA_HASH, &pos);
		construct_hash(sa, 0, &pos);
		break;
        default:
		break;
    }
#endif
    return(0);
}

/* This function is specially for netoid end boxes */
SA *
initiate_preregistration( 
	struct sockaddr_in *to_addr, 
	struct sockaddr_in *src
)
{
    SA *security_assoc;

    LOG((DEB,"INITIATING PREREGISTRATION...."));

    security_assoc = isadb_create_entry(src, INITIATOR, PKT_NIKE);
    security_assoc->state = OAK_PREREG_INIT_EXCH;

    /* Our cookie is our registration number... for prereg */
    
    sprintf(security_assoc->my_cookie, "%ld", myserialno);

    bcopy((char *)to_addr, (char *)&security_assoc->dst, 
			sizeof(struct sockaddr_in));

    if(initiate_preregistration_mode(security_assoc))
    {
	LOG((CRIT, "Unable to initiate protocol!"));
	return(NULL);
    }

    security_assoc->send_dir = BIDIR;
    security_assoc->init_or_resp == INITIATOR;

    return(security_assoc);
}

int 
initiate_certcomm_mode (SA *sa)
{
     int pos;
     
     /*
      * these algs should, ideally, come from the PF_KEY, and they should
      * have some implied preference and logical construction. For now just
      * stick 'em here in plain serial, logical OR.
      */

    sa->hash_alg = HASH_SHA;
    sa->encr_alg = ET_DES_CBC;
    sa->auth_alg = DSS_SIG;

    sa->group_desc = 1;		/* the default Oakley group */

    /* Nike : We use the Aggressive mode */
    construct_header(sa, NIKE_CERTCOMM, 0, ISA_SA);
    pos = sizeof(isakmp_hdr);
    construct_isakmp_sa(sa, ISA_KE, &pos);
    construct_ke(sa, ISA_CERT, &pos);
    construct_cert(sa, ISA_SIG, &pos);
    construct_sig(sa, 0, &pos);
#if 0
    switch (sa->auth_alg)
    {
	case PRESHRD:
		construct_id(sa, ISA_HASH, &pos);
		construct_hash(sa, 0, &pos);
		break;
        default:
		break;
    }
#endif
    return(0);
}

/* This function is specially for netoid end boxes */
SA *
initiate_certcomm( 
	struct sockaddr_in *to_addr, 
	struct sockaddr_in *src
)
{
    SA *security_assoc;

    LOG((DEB,"INITIATING CERTIFIED COMMUNICATION...."));

    security_assoc = isadb_create_entry(src, INITIATOR, PKT_NIKE);
    security_assoc->state = OAK_CERTCOMM_INIT_EXCH;

    /* Our cookie is our registration number... for prereg */
    
    sprintf(security_assoc->my_cookie, "%ld", myserialno);

    bcopy((char *)to_addr, (char *)&security_assoc->dst, 
			sizeof(struct sockaddr_in));

    if(initiate_certcomm_mode(security_assoc))
    {
	LOG((CRIT, "Unable to initiate protocol!"));
	return(NULL);
    }

    security_assoc->send_dir = BIDIR;
    security_assoc->init_or_resp == INITIATOR;

    return(security_assoc);
}
