/***********************************************************************
 ** FUNCTION:
 **             Has Init Function prototypes For all DCS Structures
 *********************************************************************
 **
 ** FILENAME:
 ** 		dcsinit.h
 **
 ** DESCRIPTION:
 ** 		This file contains code to init all DCS structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 13Nv00   S. Luthra			Initial Creation
 **
 **
 **     Copyright 2000, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __DCSINIT_H__
#define __DCSINIT_H__

#ifdef SIP_DCS
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


extern SipBool sip_dcs_initHeader _ARGS_ ((SipHeader **ppHdr, en_HeaderType dType, SipError *pErr));

extern SipBool sip_dcs_initDcsGeneralHeaders _ARGS_ ((SipGeneralHeader **g, SipError *err));

extern SipBool sip_dcs_initDcsReqHeaders _ARGS_ ((SipReqHeader **s, SipError *err));

extern SipBool sip_dcs_initDcsRespHeaders _ARGS_ ((SipRespHeader **h, SipError *err));


extern SipBool sip_dcs_initSipDcsTracePartyIdHeader _ARGS_ ((SipDcsTracePartyIdHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsAnonymityHeader _ARGS_ ((SipDcsAnonymityHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsRemotePartyIdHeader _ARGS_ ((SipDcsRemotePartyIdHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsRpidPrivacyHeader _ARGS_ ((SipDcsRpidPrivacyHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsMediaAuthorizationHeader _ARGS_ ((SipDcsMediaAuthorizationHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsGateHeader _ARGS_ ((SipDcsGateHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsStateHeader _ARGS_ ((SipDcsStateHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsOspsHeader _ARGS_ ((SipDcsOspsHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsBillingIdHeader _ARGS_ ((SipDcsBillingIdHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsLaesHeader _ARGS_ ((SipDcsLaesHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsRedirectHeader _ARGS_ ((SipDcsRedirectHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipSessionHeader _ARGS_ ((SipSessionHeader **ppHdr, SipError *pErr));

extern SipBool sip_dcs_initSipDcsBillingInfoHeader _ARGS_ ((SipDcsBillingInfoHeader **ppHdr,SipError *pErr));

extern SipBool sip_dcs_initSipDcsAcctEntry _ARGS_ ((SipDcsAcctEntry **ppEntry, SipError *pErr));
#endif



/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
