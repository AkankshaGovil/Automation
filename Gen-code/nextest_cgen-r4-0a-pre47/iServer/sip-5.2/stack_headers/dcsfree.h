/***********************************************************************
 ** FUNCTION:
 **             Has Free Function Declarations For all DCS Structures

 *********************************************************************
 **
 ** FILENAME:
 ** 		dcsfree.h
 **
 ** DESCRIPTION:
 ** 		This file contains code to free all DCS structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 13Nov00	S.Luthra                Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#ifndef __DCSFREE_H__
#define __DCSFREE_H__

#ifdef SIP_DCS
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

extern void sip_dcs_freeSipDcsTracePartyIdHeader _ARGS_ ((SipDcsTracePartyIdHeader *pHdr));

extern void sip_dcs_freeSipDcsAnonymityHeader _ARGS_ ((SipDcsAnonymityHeader *pHdr));

extern void sip_dcs_freeSipDcsRemotePartyIdHeader _ARGS_ ((SipDcsRemotePartyIdHeader *pHdr));

extern void sip_dcs_freeSipDcsRpidPrivacyHeader _ARGS_ ((SipDcsRpidPrivacyHeader *pHdr));

extern void sip_dcs_freeSipDcsMediaAuthorizationHeader _ARGS_ ((SipDcsMediaAuthorizationHeader *pHdr));

extern void sip_dcs_freeSipDcsGateHeader _ARGS_ ((SipDcsGateHeader *pHdr));

extern void sip_dcs_freeSipDcsStateHeader _ARGS_ ((SipDcsStateHeader *pHdr));

extern void sip_dcs_freeSipDcsOspsHeader _ARGS_ ((SipDcsOspsHeader *pHdr));

extern void sip_dcs_freeSipDcsBillingIdHeader _ARGS_ ((SipDcsBillingIdHeader *pHdr));

extern void sip_dcs_freeSipDcsBillingInfoHeader _ARGS_ ((SipDcsBillingInfoHeader *pHdr));

extern void sip_dcs_freeSipDcsAcctEntry _ARGS_ ((SipDcsAcctEntry *pEntry));

extern void sip_dcs_freeSipDcsLaesHeader _ARGS_ ((SipDcsLaesHeader *pHdr));

extern void sip_dcs_freeSipDcsRedirectHeader _ARGS_ ((SipDcsRedirectHeader *pHdr));

extern void sip_dcs_freeSipSessionHeader _ARGS_ ((SipSessionHeader *pHdr));

/* void * prototypes */

extern void __sip_dcs_freeSipDcsRemotePartyIdHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsTracePartyIdHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsAnonymityHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsRpidPrivacyHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsMediaAuthorizationHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsGateHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsStateHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsOspsHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsBillingIdHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsLaesHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsRedirectHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipSessionHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsBillingInfoHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_dcs_freeSipDcsAcctEntry _ARGS_ ((SIP_Pvoid pEntry));

#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
