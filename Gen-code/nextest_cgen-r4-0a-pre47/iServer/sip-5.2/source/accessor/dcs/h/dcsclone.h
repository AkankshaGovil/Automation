/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all DCS SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcsclone.h
**
** DESCRIPTION:
**  THIS FILE IS USED INTERNALLY BY THE STACK
**	IT SHOULD NOT BE INCLUDED DIRECTLY BY THE USER
**
** DATE    	 NAME           REFERENCE      REASON
** ----    	 ----           ---------      ------
** 16Nov00	S. Luthra			Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/


#ifndef __DCSCLONE_H__
#define __DCSCLONE_H__

#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

SipBool __sip_dcs_cloneSipDcsRemotePartyIdHeader _ARGS_\
	((SipDcsRemotePartyIdHeader *pDest, SipDcsRemotePartyIdHeader *pSource,\
	SipError *pErr));

SipBool __sip_dcs_cloneSipDcsRpidPrivacyHeader _ARGS_\
	((SipDcsRpidPrivacyHeader *pDest, SipDcsRpidPrivacyHeader *pSource,\
	SipError *pErr));

SipBool __sip_dcs_cloneSipDcsTracePartyIdHeader _ARGS_ ((SipDcsTracePartyIdHeader *pDest, SipDcsTracePartyIdHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsAnonymityHeader _ARGS_ ((SipDcsAnonymityHeader *pDest, SipDcsAnonymityHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsMediaAuthorizationHeader _ARGS_ ((SipDcsMediaAuthorizationHeader *pDest, SipDcsMediaAuthorizationHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsGateHeader _ARGS_ ((SipDcsGateHeader *pDest, SipDcsGateHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsStateHeader _ARGS_ ((SipDcsStateHeader *pDest, SipDcsStateHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsOspsHeader _ARGS_ ((SipDcsOspsHeader *pDest, SipDcsOspsHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsBillingIdHeader _ARGS_ ((SipDcsBillingIdHeader *pDest, SipDcsBillingIdHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsLaesHeader _ARGS_ ((SipDcsLaesHeader *pDest, SipDcsLaesHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsRedirectHeader _ARGS_ ((SipDcsRedirectHeader *pDest, SipDcsRedirectHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipSessionHeader _ARGS_ ((SipSessionHeader *pDest, SipSessionHeader *pSource, SipError *pErr));

SipBool __sip_cloneSipGenericChallengeList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsAnonymityHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsRemotePartyIdHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsRpidPrivacyHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsMediaAuthorizationHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsStateHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipSessionHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool sip_dcs_cloneSipDcsAcctEntry _ARGS_ ((SipDcsAcctEntry *pDest, SipDcsAcctEntry *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsAcctEntry _ARGS_ ((SipDcsAcctEntry *pDest, SipDcsAcctEntry *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsBillingInfoHeader _ARGS_ ((SipDcsBillingInfoHeader *pDest, SipDcsBillingInfoHeader *pSource, SipError *pErr));

SipBool __sip_dcs_cloneSipDcsAcctEntryList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
