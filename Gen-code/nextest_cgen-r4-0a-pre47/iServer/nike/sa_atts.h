/* 
 * $Id: sa_atts.h,v 1.1 1999/04/27 19:06:08 medhavi Exp $
 * $Source: /export/home/cm/repos/nike/sa_atts.h,v $
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
/* 
 *  Protection Suites for negotiation security association
 * ----------------------------------------------------------
 * list attributes that must be in a proposal to be accepted.
 * Proposal is {transform, number-of-additional-attributes} followed
 * by the attributes as { ATT_CLASS, ATT_TYPE }. Multiple protection
 * suites may be specified and their order implicitly specifies preference. 
 * Each suite is a necessary, but not necessarily complete set of
 * attributes and their values. The type of an attribute can be a
 * wildcard (a 0) in which case any value is accepted. VPI attributes
 * attribute types can only be wildcards at this point, although it should
 * be straightforward for you the reader to modify sa_utils.c to allow
 * for VPIs to specify a minimum (or maximum) acceptible value, etc.
 *
 * NOTE: failure to properly specify attributes will result in catastrophe
 */

static unsigned char oakley_pref[][2] = {
	{ PROTO_ISAKMP, 4 },		/* transform and # of addtional atts */
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_MD5 },		/* most preferred suite */
	{ OAK_AUTH_METHOD, PRESHRD },
	{ OAK_GROUP_DESC, 1 },

	{ PROTO_ISAKMP, 4 },
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_SHA },		/* alternate acceptible suite */
	{ OAK_AUTH_METHOD, PRESHRD },
	{ OAK_GROUP_DESC, 1 },

#ifdef NON_COMMERCIAL_AND_RSAREF
	{ PROTO_ISAKMP, 4 },		/* transform and # of addtional atts */
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_SHA },
	{ OAK_AUTH_METHOD, RSA_ENC },
	{ OAK_GROUP_DESC, 1 },

	{ PROTO_ISAKMP, 4 },
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_MD5 },
	{ OAK_AUTH_METHOD, RSA_ENC },
	{ OAK_GROUP_DESC, 1 },

	{ PROTO_ISAKMP, 4 },
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_SHA },	
	{ OAK_AUTH_METHOD, RSA_SIG },
	{ OAK_GROUP_DESC, 1 },

	{ PROTO_ISAKMP, 4 },
	{ OAK_ENCR_ALG, ET_DES_CBC },
	{ OAK_HASH_ALG, HASH_MD5 },
	{ OAK_AUTH_METHOD, RSA_SIG },
	{ OAK_GROUP_DESC, 1 },
#endif
	{ 0, 0 }
};

/* 
 *  Protection Suites for AH and ESP Security Associations
 */
static unsigned char ah_pref[][2] = { 
	{ AH_SHA_HMAC_REPLAY, 0 },	/* transform and # of additional atts */
	{ AH_MD5_HMAC_REPLAY, 0 },
	{ AH_MD5_KPDK, 0 },
	{ 0, 0 }
};

static unsigned char esp_pref[][2] = { 
	{ ESP_DES_CBC_TUNNEL, 0 },   /* transform and # of additional atts */
	{ ESP_DES_CBC_TRANSPORT, 0 },/* transform and # of additional atts */
	{ ESP_DES_CBC_HMAC_REPLAY, 0 },
	{ 0, 0 }
};

