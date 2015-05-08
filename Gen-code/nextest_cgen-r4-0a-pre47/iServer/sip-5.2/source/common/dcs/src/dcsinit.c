/*******************************************************************************
 **** FUNCTION:
 ****           Has Init Function code For all DCS Structures

 ******************************************************************************
 ****
 **** FILENAME:
 **** 		dcsinit.c
 ****
 **** DESCRIPTION:
 **** 		This file contains Code to init all DCS structures
 ****
 **** DATE        NAME                    REFERENCE             REASON
 **** ----        ----                    ---------             --------
 **** 13Nov00	S.Luthra					Creation
 ****
 ****     Copyright 2000, Hughes Software Systems, Ltd.
 *****************************************************************************/

#include "dcsinit.h"
#include "dcsfree.h"
#include "portlayer.h"
#include "sipcommon.h"
#include "sipfree.h"
#include "siplist.h"


SipBool sip_dcs_initSipDcsRemotePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsRemotePartyIdHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsRemotePartyIdHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsRemotePartyIdHeader *) fast_memget (0, sizeof(SipDcsRemotePartyIdHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pDispName);
	INIT ((*ppHdr)->pAddrSpec);
	/*
	sip_listInit (&((*ppHdr)->slRpiAuths), __sip_freeSipGenericChallenge, pErr);
	*/
	sip_listInit (&((*ppHdr)->slParams), __sip_freeSipParam, pErr); 
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsRpidPrivacyHeader 
#ifdef ANSI_PROTO
	(SipDcsRpidPrivacyHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsRpidPrivacyHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsRpidPrivacyHeader *) fast_memget (0, \
		sizeof(SipDcsRpidPrivacyHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	sip_listInit (&((*ppHdr)->slParams), __sip_freeSipParam, pErr); 
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsTracePartyIdHeader 
#ifdef ANSI_PROTO
	(SipDcsTracePartyIdHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsTracePartyIdHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsTracePartyIdHeader *) fast_memget (0, sizeof(SipDcsTracePartyIdHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pAddrSpec);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsAnonymityHeader 
#ifdef ANSI_PROTO
	(SipDcsAnonymityHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsAnonymityHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsAnonymityHeader *) fast_memget (0, sizeof(SipDcsAnonymityHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pTag);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}

	
SipBool sip_dcs_initSipDcsMediaAuthorizationHeader 
#ifdef ANSI_PROTO
	(SipDcsMediaAuthorizationHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsMediaAuthorizationHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsMediaAuthorizationHeader *) fast_memget (0, sizeof(SipDcsMediaAuthorizationHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pAuth);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsGateHeader 
#ifdef ANSI_PROTO
	(SipDcsGateHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsGateHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsGateHeader *)fast_memget (0, sizeof(SipDcsGateHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pHost);
	INIT ((*ppHdr)->pPort);
	INIT ((*ppHdr)->pId);
	INIT ((*ppHdr)->pKey);
	INIT ((*ppHdr)->pCipherSuite);
	INIT ((*ppHdr)->pStrength);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsStateHeader 
#ifdef ANSI_PROTO
	(SipDcsStateHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsStateHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsStateHeader *) fast_memget (0, sizeof(SipDcsStateHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pHost);
 	sip_listInit (&((*ppHdr)->slParams), __sip_freeSipParam, pErr); 

	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsOspsHeader 
#ifdef ANSI_PROTO
	(SipDcsOspsHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsOspsHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsOspsHeader *) fast_memget (0, sizeof(SipDcsOspsHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pTag);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
} 


SipBool sip_dcs_initSipDcsBillingIdHeader 
#ifdef ANSI_PROTO
	(SipDcsBillingIdHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsBillingIdHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsBillingIdHeader *) fast_memget (0, sizeof(SipDcsBillingIdHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pId);
	INIT ((*ppHdr)->pFEId);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}   


SipBool sip_dcs_initSipDcsLaesHeader 
#ifdef ANSI_PROTO
	(SipDcsLaesHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsLaesHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsLaesHeader *) fast_memget (0, sizeof(SipDcsLaesHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pSignatureHost);
	INIT ((*ppHdr)->pContentHost);
	INIT ((*ppHdr)->pSignaturePort);
	INIT ((*ppHdr)->pContentPort);
	INIT ((*ppHdr)->pKey);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipDcsRedirectHeader 
#ifdef ANSI_PROTO
	(SipDcsRedirectHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipDcsRedirectHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsRedirectHeader *) fast_memget (0, sizeof(SipDcsRedirectHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pCalledId);
	INIT ((*ppHdr)->pRedirector);
	(*ppHdr)->dNum	= 0;
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}


SipBool sip_dcs_initSipSessionHeader 
#ifdef ANSI_PROTO
	(SipSessionHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipSessionHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipSessionHeader *) fast_memget (0, sizeof(SipSessionHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pTag);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}

/********************************************************************
**** FUNCTION:sip_dcs_initSipDcsBillingInfoHeader
****
****
**** DESCRIPTION:
*********************************************************************/

SipBool sip_dcs_initSipDcsBillingInfoHeader
#ifdef ANSI_PROTO
	(SipDcsBillingInfoHeader **ppHdr,SipError *pErr)
#else
	(ppHdr,pErr)
	SipDcsBillingInfoHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipDcsBillingInfoHeader *) fast_memget (0, sizeof(SipDcsBillingInfoHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ((*ppHdr)->pHost);
	INIT ((*ppHdr)->pPort);
	sip_listInit (&((*ppHdr)->slAcctEntry), __sip_dcs_freeSipDcsAcctEntry, pErr);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}

/********************************************************************
**** FUNCTION:sip_dcs_initSipDcsAcctEntry
****
****
**** DESCRIPTION:
*********************************************************************/

SipBool sip_dcs_initSipDcsAcctEntry
#ifdef ANSI_PROTO
	(SipDcsAcctEntry **ppEntry, SipError *pErr)
#else
	(ppEntry,pErr)
	SipDcsAcctEntry **ppEntry;
	SipError *pErr;
#endif
{
	*ppEntry = (SipDcsAcctEntry *) fast_memget (0, sizeof(SipDcsAcctEntry), pErr);
	if (*ppEntry == SIP_NULL)
		return SipFail;	
	INIT ((*ppEntry)->pChargeNum);
	INIT ((*ppEntry)->pCallingNum);
	INIT ((*ppEntry)->pCalledNum);
	INIT ((*ppEntry)->pRoutingNum);
	INIT ((*ppEntry)->pLocationRoutingNum);
	HSS_INITREF((*ppEntry)->dRefCount);
	return SipSuccess;
}



SipBool sip_dcs_initHeader
#ifdef ANSI_PROTO
	(SipHeader **ppHdr, en_HeaderType dType, SipError *pErr)
#else
	(ppHdr, dType, pErr)
	SipHeader **ppHdr;
	en_HeaderType dType;
	SipError *pErr;
#endif
{
	switch (dType)
	{
		case SipHdrTypeDcsRemotePartyId:
			sip_dcs_initSipDcsRemotePartyIdHeader((SipDcsRemotePartyIdHeader **) \
								&((*ppHdr)->pHeader), pErr);
				break;
		case SipHdrTypeDcsRpidPrivacy:
			sip_dcs_initSipDcsRpidPrivacyHeader((SipDcsRpidPrivacyHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;			
		case SipHdrTypeDcsTracePartyId:
			sip_dcs_initSipDcsTracePartyIdHeader((SipDcsTracePartyIdHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsAnonymity:
			sip_dcs_initSipDcsAnonymityHeader((SipDcsAnonymityHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;			
		case SipHdrTypeDcsMediaAuthorization:
			sip_dcs_initSipDcsMediaAuthorizationHeader((SipDcsMediaAuthorizationHeader **)\
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsGate:
			sip_dcs_initSipDcsGateHeader((SipDcsGateHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsRedirect:
			sip_dcs_initSipDcsRedirectHeader((SipDcsRedirectHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsState:
			sip_dcs_initSipDcsStateHeader((SipDcsStateHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsLaes:
			sip_dcs_initSipDcsLaesHeader((SipDcsLaesHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;					
		case SipHdrTypeSession:
			sip_dcs_initSipSessionHeader((SipSessionHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;					
		case SipHdrTypeDcsOsps:
			sip_dcs_initSipDcsOspsHeader((SipDcsOspsHeader **) \
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsBillingId:
			sip_dcs_initSipDcsBillingIdHeader((SipDcsBillingIdHeader **)\
								&((*ppHdr)->pHeader), pErr);
			break;
		case SipHdrTypeDcsBillingInfo:
			sip_dcs_initSipDcsBillingInfoHeader((SipDcsBillingInfoHeader **)\
								&((*ppHdr)->pHeader), pErr);
			break;					
		default: *pErr = E_INV_TYPE;
			return SipFail;
	}
	return SipSuccess;
}


SipBool sip_dcs_initDcsGeneralHeaders
#ifdef ANSI_PROTO
(SipGeneralHeader **g, SipError *err)
#else
(g, err)
SipGeneralHeader **g;
SipError *err;
#endif
{
	sip_listInit(&((*g)->slDcsAnonymityHdr),__sip_dcs_freeSipDcsAnonymityHeader, err);
	sip_listInit(&((*g)->slDcsStateHdr),__sip_dcs_freeSipDcsStateHeader, err);
	sip_listInit(&((*g)->slDcsRpidPrivacyHdr),__sip_dcs_freeSipDcsRpidPrivacyHeader, err);
	sip_listInit(&((*g)->slDcsRemotePartyIdHdr),__sip_dcs_freeSipDcsRemotePartyIdHeader, err);
	sip_listInit(&((*g)->slDcsMediaAuthorizationHdr),__sip_dcs_freeSipDcsMediaAuthorizationHeader, err);

	INIT((*g)->pDcsLaesHdr);
	INIT((*g)->pDcsGateHdr);
	INIT((*g)->pDcsBillingIdHdr);
	INIT((*g)->pDcsBillingInfoHdr);
	return SipSuccess;
}

SipBool sip_dcs_initDcsReqHeaders
#ifdef ANSI_PROTO
(SipReqHeader **s, SipError *err)
#else
(s, err)
SipReqHeader **s;
SipError *err;
#endif
{
	SipError *dummy;
	dummy = err;
	INIT((*s)->pDcsOspsHdr);
	INIT((*s)->pDcsRedirectHdr);
	INIT((*s)->pDcsTracePartyIdHdr);
	return SipSuccess;
}


SipBool sip_dcs_initDcsRespHeaders
#ifdef ANSI_PROTO
(SipRespHeader **h, SipError *err)
#else
(h, err)
SipRespHeader **h;
SipError *err;
#endif
{
	sip_listInit(&((*h)->slSessionHdr),__sip_dcs_freeSipSessionHeader, err);
	return SipSuccess;
}
