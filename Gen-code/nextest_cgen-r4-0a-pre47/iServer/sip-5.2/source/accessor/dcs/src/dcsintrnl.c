/******************************************************************************
** FUNCTION:
** 	This header file contains the source code of all DCS SIP Structure  
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	dcsintrnl.c
**
** DESCRIPTION:
**  	THIS FILE IS USED INTERNALLY BY THE STACK
**
** DATE    	 NAME           REFERENCE      REASON
** ----    	 ----           ---------      ------
** 16Nov00	S. Luthra			Creation
**					
** Copyrights 2000, Hughes Software Systems, Ltd.
*******************************************************************************/

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"	
#include "dcsintrnl.h"
#include "dcsclone.h"
#include "dcsfree.h"
#include "dcsinit.h"
#include "sipinternal.h"



SipBool sip_dcs_cloneDcsHeaders 
#ifdef ANSI_PROTO
	(SipHeader *pDest, SipHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipHeader *pDest;
	SipHeader *pSource;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering sip_dcs_cloneDcsHeaders");
	switch (pSource->dType)
	{
		case 	SipHdrTypeDcsRemotePartyId	:
			if (__sip_dcs_cloneSipDcsRemotePartyIdHeader( \
				(SipDcsRemotePartyIdHeader *)(pDest->pHeader), \
				(SipDcsRemotePartyIdHeader *)(pSource->pHeader), \
				pErr) == SipFail)
				return SipFail;
			break;
		case	SipHdrTypeDcsRpidPrivacy	:
			if (__sip_dcs_cloneSipDcsRpidPrivacyHeader(\
				(SipDcsRpidPrivacyHeader *)(pDest->pHeader), \
				(SipDcsRpidPrivacyHeader *)(pSource->pHeader), \
				pErr) == SipFail)
				return SipFail;
			break;

		case	SipHdrTypeDcsTracePartyId	:if (__sip_dcs_cloneSipDcsTracePartyIdHeader( (SipDcsTracePartyIdHeader *)(pDest->pHeader), (SipDcsTracePartyIdHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsAnonymity		:if (__sip_dcs_cloneSipDcsAnonymityHeader( (SipDcsAnonymityHeader *)(pDest->pHeader), (SipDcsAnonymityHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsMediaAuthorization	:if (__sip_dcs_cloneSipDcsMediaAuthorizationHeader( (SipDcsMediaAuthorizationHeader *)(pDest->pHeader), (SipDcsMediaAuthorizationHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsGate		:if (__sip_dcs_cloneSipDcsGateHeader( (SipDcsGateHeader *)(pDest->pHeader), (SipDcsGateHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsBillingId		:if (__sip_dcs_cloneSipDcsBillingIdHeader( (SipDcsBillingIdHeader *)(pDest->pHeader), (SipDcsBillingIdHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsRedirect		:if (__sip_dcs_cloneSipDcsRedirectHeader( (SipDcsRedirectHeader *)(pDest->pHeader), (SipDcsRedirectHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsState		:if (__sip_dcs_cloneSipDcsStateHeader( (SipDcsStateHeader *)(pDest->pHeader), (SipDcsStateHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;
	
		case	SipHdrTypeDcsLaes		:if (__sip_dcs_cloneSipDcsLaesHeader( (SipDcsLaesHeader *)(pDest->pHeader), (SipDcsLaesHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;
	
		case	SipHdrTypeSession		:if (__sip_dcs_cloneSipSessionHeader( (SipSessionHeader *)(pDest->pHeader), (SipSessionHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsOsps		:if (__sip_dcs_cloneSipDcsOspsHeader( (SipDcsOspsHeader *)(pDest->pHeader), (SipDcsOspsHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		case	SipHdrTypeDcsBillingInfo		:if (__sip_dcs_cloneSipDcsBillingInfoHeader( (SipDcsBillingInfoHeader *)(pDest->pHeader), (SipDcsBillingInfoHeader *)(pSource->pHeader), pErr) == SipFail)
								return SipFail;
							break;

		default					:*pErr = E_INV_TYPE;
							return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting sip_dcs_cloneDcsHeaders");
	return SipSuccess;
}



SipBool sip_dcs_deleteAllDcsGeneralHeaders 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipGeneralHeader *pDest;
	SipGeneralHeader *pSource;
	SipError *pErr;
#endif
{
	SipGeneralHeader *dummy;
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	dummy=pSource;
	SIPDEBUGFN("Entering function sip_dcs_deletaAllDcsGeneralHeader");
	if (sip_listDeleteAll(&(pDest->slDcsStateHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slDcsAnonymityHdr), pErr) == SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slDcsRpidPrivacyHdr),pErr)== SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slDcsRemotePartyIdHdr),pErr)== SipFail)
		return SipFail;
	if (sip_listDeleteAll(&(pDest->slDcsMediaAuthorizationHdr),pErr)== SipFail)
		return SipFail;
	sip_dcs_freeSipDcsGateHeader (pDest->pDcsGateHdr);
	sip_dcs_freeSipDcsBillingIdHeader (pDest->pDcsBillingIdHdr);
	sip_dcs_freeSipDcsLaesHeader (pDest->pDcsLaesHdr);
	sip_dcs_freeSipDcsBillingInfoHeader (pDest->pDcsBillingInfoHdr);

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deletaAllDcsGeneralHeader");
	return SipSuccess;
}


SipBool sip_dcs_cloneAllDcsGeneralHeaders 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipGeneralHeader *pDest;
	SipGeneralHeader *pSource;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pDest == SIP_NULL)||(pSource == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function sip_dcs_cloneAllDcsGeneralHeaders");
	if (__sip_dcs_cloneSipDcsAnonymityHeaderList (&(pDest->slDcsAnonymityHdr), &(pSource->slDcsAnonymityHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsRpidPrivacyHeaderList(\
		&(pDest->slDcsRpidPrivacyHdr), &(pSource->slDcsRpidPrivacyHdr),\
		pErr)== SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsRemotePartyIdHeaderList(\
		&(pDest->slDcsRemotePartyIdHdr), &(pSource->slDcsRemotePartyIdHdr),\
		pErr)== SipFail)
		return SipFail;
		
	if (__sip_dcs_cloneSipDcsStateHeaderList (&(pDest->slDcsStateHdr), &(pSource->slDcsStateHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsMediaAuthorizationHeaderList (&(pDest->slDcsMediaAuthorizationHdr), &(pSource->slDcsMediaAuthorizationHdr), pErr) == SipFail)
		return SipFail;
	if(pSource->pDcsGateHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsGateHeader (&(pDest->pDcsGateHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsGateHeader (pDest->pDcsGateHdr, pSource->pDcsGateHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pDcsBillingIdHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsBillingIdHeader (&(pDest->pDcsBillingIdHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsBillingIdHeader (pDest->pDcsBillingIdHdr, pSource->pDcsBillingIdHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pDcsLaesHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsLaesHeader (&(pDest->pDcsLaesHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsLaesHeader (pDest->pDcsLaesHdr, pSource->pDcsLaesHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pDcsBillingInfoHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsBillingInfoHeader (&(pDest->pDcsBillingInfoHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsBillingInfoHeader (pDest->pDcsBillingInfoHdr, pSource->pDcsBillingInfoHdr, pErr) == SipFail)
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_cloneAllDcsGeneralHeaders");
	return SipSuccess;
}


SipBool sip_dcs_cloneAllDcsRequestHeaders 
#ifdef ANSI_PROTO
	(SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReqHeader *pDest;
	SipReqHeader *pSource;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function sip_dcs_cloneAllDcsRequestHeaders");
	if(pSource->pDcsTracePartyIdHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsTracePartyIdHeader (&(pDest->pDcsTracePartyIdHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsTracePartyIdHeader (pDest->pDcsTracePartyIdHdr, pSource->pDcsTracePartyIdHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pDcsOspsHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsOspsHeader (&(pDest->pDcsOspsHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsOspsHeader (pDest->pDcsOspsHdr, pSource->pDcsOspsHdr, pErr) == SipFail)
		return SipFail;
	}
	if(pSource->pDcsRedirectHdr != SIP_NULL)
	{
	if (sip_dcs_initSipDcsRedirectHeader (&(pDest->pDcsRedirectHdr), pErr) == SipFail)
		return SipFail;
	if (__sip_dcs_cloneSipDcsRedirectHeader (pDest->pDcsRedirectHdr, pSource->pDcsRedirectHdr, pErr) == SipFail)
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_cloneAllDcsRequestHeaders");
	return SipSuccess;
}


SipBool sip_dcs_cloneAllDcsResponseHeaders 
#ifdef ANSI_PROTO
	(SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRespHeader *pDest;
	SipRespHeader *pSource;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function sip_dcs_cloneAllDcsResponseHeaders");
	if (__sip_dcs_cloneSipSessionHeaderList (&(pDest->slSessionHdr), &(pSource->slSessionHdr), pErr) == SipFail)
		return SipFail;

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_cloneAllDcsResponseHeaders");
	return SipSuccess;
}


SipBool sip_dcs_deleteAllDcsRequestHeaders 
#ifdef ANSI_PROTO
	(SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipReqHeader *pDest;
	SipReqHeader *pSource;
	SipError *pErr;
#endif
{
	SipReqHeader *dummy;
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	dummy= pSource;
	SIPDEBUGFN("Entering function sip_dcs_deleteAllDcsRequestHeaders");
	sip_dcs_freeSipDcsTracePartyIdHeader (pDest->pDcsTracePartyIdHdr);
	sip_dcs_freeSipDcsOspsHeader (pDest->pDcsOspsHdr);
	sip_dcs_freeSipDcsRedirectHeader (pDest->pDcsRedirectHdr);

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteAllDcsRequestHeaders");
	return SipSuccess;
}


SipBool sip_dcs_deleteAllDcsResponseHeaders 
#ifdef ANSI_PROTO
	(SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRespHeader *pDest;
	SipRespHeader *pSource;
	SipError *pErr;
#endif
{
	SipRespHeader *dummy; 
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pSource == SIP_NULL)||(pDest == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	dummy= pSource;
	SIPDEBUGFN("Entering function sip_dcs_deleteAllDcsResponseHeaders");
	if (sip_listDeleteAll(&(pDest->slSessionHdr), pErr) == SipFail)
		return SipFail;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteAllDcsResponseHeaders");
	return SipSuccess;
}


en_SipBoolean sip_dcs_validateDcsHeaderType
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN ("Entering function sip_dcs_validateDcsHeaderType");
	switch (dType)
	{
	 	case 	SipHdrTypeDcsRemotePartyId:
		case	SipHdrTypeDcsRpidPrivacy: 
	 	case	SipHdrTypeDcsTracePartyId: 
		case	SipHdrTypeDcsAnonymity: 
		case 	SipHdrTypeDcsMediaAuthorization:   	
		case 	SipHdrTypeDcsGate :
		case 	SipHdrTypeDcsRedirect:
		case	SipHdrTypeDcsState:
		case 	SipHdrTypeDcsLaes :
		case 	SipHdrTypeSession:
		case    SipHdrTypeDcsOsps:
		case	SipHdrTypeDcsBillingId:
		case	SipHdrTypeDcsBillingInfo:
				break;
		default:
				return SipFalse;
	}
	SIPDEBUGFN("Exiting function sip_dcs_validateDcsHeaderType");
	return SipTrue;
}


en_SipBoolean sip_dcs_isSipDcsGeneralHeader 
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_isSipDcsGeneralHeader");
	switch (dType)
	{
		case	SipHdrTypeDcsState:
		case	SipHdrTypeDcsBillingId:
		case	SipHdrTypeDcsMediaAuthorization:
		case	SipHdrTypeDcsRemotePartyId:
		case	SipHdrTypeDcsRpidPrivacy:
		case	SipHdrTypeDcsGate:
		case	SipHdrTypeDcsAnonymity:
		case	SipHdrTypeDcsLaes:
		case	SipHdrTypeDcsBillingInfo:
					 break;
		default:
			return SipFalse;
	}	
	SIPDEBUGFN("Exiting function sip_dcs_isSipDcsGeneralHeader");
	return SipTrue;
}


en_SipBoolean sip_dcs_isSipDcsRequestHeader 
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_isSipDcsRequestHeader");
	switch (dType)
	{
		case	SipHdrTypeDcsOsps:
		case	SipHdrTypeDcsRedirect:
		case	SipHdrTypeDcsTracePartyId:
						 break;
		default:
			return SipFalse;
	}	
	SIPDEBUGFN("Exiting function sip_dcs_isSipDcsRequestHeader");
	return SipTrue;
}


en_SipBoolean sip_dcs_isSipDcsResponseHeader
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_isSipDcsResponseHeader");
	if (dType != SipHdrTypeSession)
		return SipFalse;	
	SIPDEBUGFN("Exiting function sip_dcs_isSipDcsResponseHeader");
	return SipTrue;
}


SipBool sip_dcs_deleteAllDcsGeneralHeaderByType
#ifdef ANSI_PROTO
	(SipGeneralHeader *pHdr, en_HeaderType dType, SipError *pErr)
#else
	(pHdr, dType, pErr)
	SipGeneralHeader *pHdr;
	en_HeaderType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteAllDcsGeneralHeader");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
		case	SipHdrTypeDcsState	:if (sip_listDeleteAll(&(pHdr->slDcsStateHdr), pErr) == SipFail)
				return SipFail;
						break;

		case	SipHdrTypeDcsBillingId	:if (pHdr->pDcsBillingIdHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsBillingIdHeader (pHdr->pDcsBillingIdHdr);
							pHdr->pDcsBillingIdHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsMediaAuthorization:
				if (sip_listDeleteAll(&(pHdr->slDcsMediaAuthorizationHdr), \
					pErr) == SipFail)
				return SipFail;
				break;

		case	SipHdrTypeDcsRemotePartyId:
				if (sip_listDeleteAll(&(pHdr->slDcsRemotePartyIdHdr), \
					pErr) == SipFail)
				return SipFail;
				break;
						
		case	SipHdrTypeDcsRpidPrivacy:
				if (sip_listDeleteAll(&(pHdr->slDcsRpidPrivacyHdr), \
					pErr) == SipFail)
				return SipFail;
				break;

		case	SipHdrTypeDcsGate	:if (pHdr->pDcsGateHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsGateHeader (pHdr->pDcsGateHdr);
							pHdr->pDcsGateHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsAnonymity	:if (sip_listDeleteAll(&(pHdr->slDcsAnonymityHdr), pErr) == SipFail)
				return SipFail;
						break;

		case	SipHdrTypeDcsLaes	:if (pHdr->pDcsLaesHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsLaesHeader (pHdr->pDcsLaesHdr);
							pHdr->pDcsLaesHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsBillingInfo	:if (pHdr->pDcsBillingInfoHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsBillingInfoHeader (pHdr->pDcsBillingInfoHdr);
							pHdr->pDcsBillingInfoHdr = SIP_NULL;
						}
						break;

		default	: *pErr = E_INV_TYPE;
			return SipFail;
	} 

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteAllDcsGeneralHeader");
	return SipSuccess;
}


SipBool sip_dcs_deleteAllDcsRequestHeaderByType 
#ifdef ANSI_PROTO
	(SipReqHeader *pHdr, en_HeaderType dType, SipError *pErr)
#else
	(pHdr, dType, pErr)
	SipReqHeader *pHdr;
	en_HeaderType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteAllDcsRequestHeader");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	switch (dType)
	{
		case	SipHdrTypeDcsOsps	:if (pHdr->pDcsOspsHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsOspsHeader (pHdr->pDcsOspsHdr);
							pHdr->pDcsOspsHdr = SIP_NULL;
						}
						break;

		case 	SipHdrTypeDcsTracePartyId:if (pHdr->pDcsTracePartyIdHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsTracePartyIdHeader (pHdr->pDcsTracePartyIdHdr);
							pHdr->pDcsTracePartyIdHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsRedirect	:if (pHdr->pDcsRedirectHdr != SIP_NULL)
						{
							sip_dcs_freeSipDcsRedirectHeader (pHdr->pDcsRedirectHdr);
							pHdr->pDcsRedirectHdr = SIP_NULL;
						}
						break;

		default: *pErr = E_INV_TYPE;
			return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteAllDcsRequestHeader");
	return SipSuccess;
}


SipBool sip_dcs_deleteAllDcsResponseHeaderByType
#ifdef ANSI_PROTO
	(SipRespHeader *pHdr, en_HeaderType dType, SipError *pErr)
#else
	(pHdr, dType, pErr)
	SipRespHeader *pHdr;
	en_HeaderType dType;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteAllDcsReponseHeaderByType");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif

	if (dType == SipHdrTypeSession)
	{
		if (sip_listDeleteAll(&(pHdr->slSessionHdr), pErr) == SipFail)
				return SipFail;
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteAllDcsResponseHeaderByType");
	return SipSuccess;
}


SipBool sip_dcs_getDcsGeneralHeaderCount 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, dType, pCount, pErr)
	SipGeneralHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIP_U32bit dTempCount;
	SIPDEBUGFN("Entering function sip_dcs_getDcsGeneralHeaderCount");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL)||(pCount == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
		case	SipHdrTypeDcsState	:if (sip_listSizeOf(&(pHdr->slDcsStateHdr), &dTempCount, pErr) == SipFail)
							return SipFail;
						 *pCount = dTempCount;
						 break;

		case	SipHdrTypeDcsBillingId	:if (pHdr->pDcsBillingIdHdr != SIP_NULL)
								*pCount = 1;
							 else
								*pCount = 0; 
							 break;

		case	SipHdrTypeDcsMediaAuthorization:
			if (sip_listSizeOf(&(pHdr->slDcsMediaAuthorizationHdr), &dTempCount, pErr) == SipFail)
				return SipFail;
			*pCount = dTempCount;	
			break;

		case	SipHdrTypeDcsGate	:if (pHdr->pDcsGateHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;

		case	SipHdrTypeDcsAnonymity	:if (sip_listSizeOf(&(pHdr->slDcsAnonymityHdr), &dTempCount, pErr) == SipFail)
							return SipFail;
						 *pCount = dTempCount;
						 break;
						 
		case	SipHdrTypeDcsRemotePartyId:
				if (sip_listSizeOf(&(pHdr->slDcsRemotePartyIdHdr), \
					&dTempCount, pErr) == SipFail)
					return SipFail;
				 *pCount = dTempCount;
				 break;
				 
		case	SipHdrTypeDcsRpidPrivacy	:
				if (sip_listSizeOf(&(pHdr->slDcsRpidPrivacyHdr), \
					&dTempCount, pErr) == SipFail)
					return SipFail;
				 *pCount = dTempCount;
				 break;

		case	SipHdrTypeDcsLaes	:if (pHdr->pDcsLaesHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;	

		case	SipHdrTypeDcsBillingInfo	:if (pHdr->pDcsBillingInfoHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;

		default				: *pErr = E_INV_TYPE;
						return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsGeneralHeaderCount");
	return SipSuccess;
}


SipBool sip_dcs_getDcsRequestHeaderCount 
#ifdef ANSI_PROTO
	(SipReqHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, dType, pCount, pErr)
	SipReqHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL)||(pCount == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function sip_dcs_getDcsRequestHeaderCount");
	switch (dType)
	{
		case	SipHdrTypeDcsOsps	:if (pHdr->pDcsOspsHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;

		case	SipHdrTypeDcsTracePartyId:if (pHdr->pDcsTracePartyIdHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;

		case	SipHdrTypeDcsRedirect	:if (pHdr->pDcsRedirectHdr != SIP_NULL)
							*pCount = 1;
						 else
							*pCount = 0; 
						 break;

		default				: *pErr = E_INV_TYPE;
						 return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsRequestHeaderCount");
	return SipSuccess;
}


SipBool sip_dcs_getDcsResponseHeaderCount 
#ifdef ANSI_PROTO
	(SipRespHeader *pHdr, en_HeaderType dType, SIP_U32bit *pCount, SipError *pErr)
#else
	(pHdr, dType, pCount, pErr)
	SipRespHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit *pCount;
	SipError *pErr;
#endif
{
	SIP_U32bit dTempCount;
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL)||(pCount == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	SIPDEBUGFN("Entering function sip_dcs_getDcsResponseHeaderCount");
	if (dType == SipHdrTypeSession)
	{
	if (sip_listSizeOf(&(pHdr->slSessionHdr), &dTempCount, pErr) == SipFail)
		return SipFail;
	 *pCount = dTempCount;
	}
	else
	{
		*pErr = E_INV_TYPE;
	
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsResponseHeaderCount");
	return SipSuccess;
}


SipBool sip_dcs_deleteDcsGeneralHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dType, dIndex, pErr)
	SipGeneralHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteDcsGeneralHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
			case	SipHdrTypeDcsState	:if (sip_listDeleteAt (&(pHdr->slDcsStateHdr), dIndex, pErr) == SipFail)
							return SipFail;

		case	SipHdrTypeDcsBillingId	:if (pHdr->pDcsBillingIdHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsBillingIdHeader (pHdr->pDcsBillingIdHdr);
			 				pHdr->pDcsBillingIdHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsMediaAuthorization:
			if (sip_listDeleteAt (&(pHdr->slDcsMediaAuthorizationHdr), \
				dIndex, pErr) == SipFail)
				return SipFail;

		case	SipHdrTypeDcsGate	:if (pHdr->pDcsGateHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsGateHeader (pHdr->pDcsGateHdr);
			 				pHdr->pDcsGateHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsAnonymity	:if (sip_listDeleteAt (&(pHdr->slDcsAnonymityHdr), dIndex, pErr) == SipFail)
							return SipFail;
			break;
		case	SipHdrTypeDcsRemotePartyId:
				if (sip_listDeleteAt (&(pHdr->slDcsRemotePartyIdHdr), \
					dIndex, pErr) == SipFail)
					return SipFail;
				break;
		case	SipHdrTypeDcsRpidPrivacy :
				if (sip_listDeleteAt (&(pHdr->slDcsRpidPrivacyHdr), \
					dIndex, pErr) == SipFail)
					return SipFail;
				break;

		case	SipHdrTypeDcsLaes	:if (pHdr->pDcsLaesHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsLaesHeader (pHdr->pDcsLaesHdr);
			 				pHdr->pDcsLaesHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsBillingInfo	:if (pHdr->pDcsBillingInfoHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsBillingInfoHeader (pHdr->pDcsBillingInfoHdr);
			 				pHdr->pDcsBillingInfoHdr = SIP_NULL;
						}
						break;

		default				: *pErr = E_INV_TYPE;
						return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteDcsGeneralHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_deleteDcsRequestHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipReqHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dType, dIndex, pErr)
	SipReqHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteDcsRequestHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
		case	SipHdrTypeDcsOsps	:if (pHdr->pDcsOspsHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsOspsHeader (pHdr->pDcsOspsHdr);
			 				pHdr->pDcsOspsHdr = SIP_NULL;
						}
						break;

		case 	SipHdrTypeDcsRedirect	:if (pHdr->pDcsRedirectHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsRedirectHeader (pHdr->pDcsRedirectHdr);
			 				pHdr->pDcsRedirectHdr = SIP_NULL;
						}
						break;

		case	SipHdrTypeDcsTracePartyId:if (pHdr->pDcsTracePartyIdHdr == SIP_NULL)
						{
							*pErr = E_NO_EXIST;
							return SipFail;
						 }
						 else
			 			{
						 	if (dIndex > 0)
						 	{
								*pErr = E_INV_INDEX;
								return SipFail;
						 	}
							sip_dcs_freeSipDcsTracePartyIdHeader (pHdr->pDcsTracePartyIdHdr);
			 				pHdr->pDcsTracePartyIdHdr = SIP_NULL;
						}
						break;		

		default				: *pErr = E_INV_TYPE;
						return SipFail;	
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteDcsRequestHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_deleteDcsResponseHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipRespHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pHdr, dType, dIndex, pErr)
	SipRespHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_deleteDcsResponseHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if(pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (dType == SipHdrTypeSession)
	{
		if (sip_listDeleteAt (&(pHdr->slSessionHdr), dIndex, pErr) == SipFail)
			return SipFail;
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_deleteDcsResponseHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_getDcsGeneralHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pGeneralHdr, SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pGeneralHdr, pHdr, dType, dIndex, pErr)
	SipGeneralHeader *pGeneralHdr;
	SipHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIP_Pvoid pTemp;
	SIPDEBUGFN("Entering function sip_dcs_getDcsGeneralHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL)||(pGeneralHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
		case	SipHdrTypeDcsState:
			if (sip_listGetAt(&(pGeneralHdr->slDcsStateHdr), dIndex, &pTemp, pErr)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE				
			 ((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipDcsStateHeader *) ((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipDcsStateHeader ((SipDcsStateHeader *)(pHdr->pHeader),\
				 (SipDcsStateHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
			break;

		case	SipHdrTypeDcsBillingId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsBillingIdHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((pHdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pDcsBillingIdHdr;
					HSS_LOCKEDINCREF(((SipDcsBillingIdHeader *) ((pHdr)->pHeader))->dRefCount);
#else
					if (__sip_dcs_cloneSipDcsBillingIdHeader( (SipDcsBillingIdHeader *)(pHdr->pHeader)\
						, pGeneralHdr->pDcsBillingIdHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			}
			break;

		case	SipHdrTypeDcsMediaAuthorization:
			if (sip_listGetAt(&(pGeneralHdr->slDcsMediaAuthorizationHdr), dIndex, &pTemp, pErr)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE				
			((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipDcsMediaAuthorizationHeader *) ((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipDcsMediaAuthorizationHeader ((SipDcsMediaAuthorizationHeader *)(pHdr->pHeader),\
				 (SipDcsMediaAuthorizationHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
			break;

		case	SipHdrTypeDcsGate:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsGateHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((pHdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pDcsGateHdr;
					HSS_LOCKEDINCREF(((SipDcsGateHeader *) ((pHdr)->pHeader))->dRefCount);
#else
					if (__sip_dcs_cloneSipDcsGateHeader( (SipDcsGateHeader *)(pHdr->pHeader)\
						, pGeneralHdr->pDcsGateHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			}
			break;

		case	SipHdrTypeDcsAnonymity:
			if (sip_listGetAt(&(pGeneralHdr->slDcsAnonymityHdr), dIndex, &pTemp, pErr)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE				
			 ((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipDcsAnonymityHeader *) ((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipDcsAnonymityHeader ((SipDcsAnonymityHeader *)(pHdr->pHeader),\
				 (SipDcsAnonymityHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
			break;
		case	SipHdrTypeDcsRemotePartyId:
			if (sip_listGetAt(&(pGeneralHdr->slDcsRemotePartyIdHdr), 
				dIndex, &pTemp, pErr) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE				
			 ((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipDcsRemotePartyIdHeader *) \
			((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipDcsRemotePartyIdHeader \
				((SipDcsRemotePartyIdHeader *)(pHdr->pHeader), \
				(SipDcsRemotePartyIdHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
			break;
			
		case	SipHdrTypeDcsRpidPrivacy:
			if (sip_listGetAt(&(pGeneralHdr->slDcsRpidPrivacyHdr), 
				dIndex, &pTemp, pErr) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE				
			 ((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipDcsRpidPrivacyHeader *) \
			((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipDcsRpidPrivacyHeader \
				((SipDcsRpidPrivacyHeader *)(pHdr->pHeader), \
				(SipDcsRpidPrivacyHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
			break;

		case	SipHdrTypeDcsLaes:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsLaesHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((pHdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pDcsLaesHdr;
					HSS_LOCKEDINCREF(((SipDcsLaesHeader *) ((pHdr)->pHeader))->dRefCount);
#else
					if (__sip_dcs_cloneSipDcsLaesHeader( (SipDcsLaesHeader *)(pHdr->pHeader)\
						, pGeneralHdr->pDcsLaesHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			}
			break;

		case	SipHdrTypeDcsBillingInfo:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsBillingInfoHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((pHdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pDcsBillingInfoHdr;
					HSS_LOCKEDINCREF(((SipDcsBillingInfoHeader *) ((pHdr)->pHeader))->dRefCount);
#else
					if (__sip_dcs_cloneSipDcsBillingInfoHeader( (SipDcsBillingInfoHeader *)(pHdr->pHeader)\
						, pGeneralHdr->pDcsBillingInfoHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			}
			break;
					 
		default:*pErr = E_INV_TYPE;
			return SipFail;
	}	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsGeneralHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_getDcsRequestHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr,  SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pRequestHdr, pHdr, dType, dIndex, pErr)
	SipReqHeader *pRequestHdr;
	SipHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_getDcsRequestHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pRequestHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (dType)
	{
		case	SipHdrTypeDcsOsps	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsOspsHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((pHdr)->pHeader) = (SIP_Pvoid)  pRequestHdr->pDcsOspsHdr;
					HSS_LOCKEDINCREF(((SipDcsOspsHeader *) ((pHdr)->pHeader))->dRefCount);
#else					
					if (__sip_dcs_cloneSipDcsOspsHeader(\
						(SipDcsOspsHeader *)(pHdr->pHeader),\
						pRequestHdr->pDcsOspsHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			 }
			 break;

		case	SipHdrTypeDcsRedirect	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsRedirectHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((pHdr)->pHeader) = (SIP_Pvoid)  pRequestHdr->pDcsRedirectHdr;
					HSS_LOCKEDINCREF(((SipDcsRedirectHeader *) ((pHdr)->pHeader))->dRefCount);
#else					
					if (__sip_dcs_cloneSipDcsRedirectHeader(\
						(SipDcsRedirectHeader *)(pHdr->pHeader),\
						pRequestHdr->pDcsRedirectHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			 }
			 break;

		case	SipHdrTypeDcsTracePartyId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsTracePartyIdHdr == SIP_NULL)
				{
					*pErr = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((pHdr)->pHeader) = (SIP_Pvoid)  pRequestHdr->pDcsTracePartyIdHdr;
					HSS_LOCKEDINCREF(((SipDcsTracePartyIdHeader *) ((pHdr)->pHeader))->dRefCount);
#else					
					if (__sip_dcs_cloneSipDcsTracePartyIdHeader(\
						(SipDcsTracePartyIdHeader *)(pHdr->pHeader),\
						pRequestHdr->pDcsTracePartyIdHdr, pErr) == SipFail)
						return SipFail;
#endif						
				}
			 }
			 break;

		default				: *pErr = E_INV_TYPE;
						return SipFail;
	}	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsRequestHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_getDcsResponseHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipRespHeader *pResponseHdr,  SipHeader *pHdr, en_HeaderType dType, SIP_U32bit dIndex, SipError *pErr)
#else
	(pResponseHdr,pHdr, dType, dIndex, pErr)
	SipRespHeader *pResponseHdr;
	SipHeader *pHdr;
	en_HeaderType dType;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIP_Pvoid pTemp;
	SIPDEBUGFN("Entering function sip_dcs_getDcsResponseHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pResponseHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (dType == SipHdrTypeSession)
	{
			if (sip_listGetAt(&(pResponseHdr->slSessionHdr), dIndex,\
				&pTemp, pErr) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((pHdr)->pHeader) = (SIP_Pvoid) pTemp;
			HSS_LOCKEDINCREF(((SipSessionHeader *) ((pHdr)->pHeader))->dRefCount);
#else
			if (__sip_dcs_cloneSipSessionHeader (\
				(SipSessionHeader *)(pHdr->pHeader),\
				(SipSessionHeader *)pTemp, pErr) == SipFail)
				return SipFail;
#endif				
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_getDcsResponseHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_setDcsGeneralHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pGeneralHdr, SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pGeneralHdr, pHdr, dIndex, pErr)
	SipGeneralHeader *pGeneralHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipDcsStateHeader *pDcsState;
	SipDcsMediaAuthorizationHeader *pDcsMedia;
	SipDcsAnonymityHeader *pDcsAnonymity;
	SipDcsRpidPrivacyHeader *pDcsRpidPrivacy;
	SipDcsRemotePartyIdHeader *pDcsRemotePartyId;
	en_SipBoolean z=SipFalse;
#endif
	SIPDEBUGFN("Entering function sip_dcs_setDcsGeneralHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pGeneralHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (pHdr->dType)
	{
		case	SipHdrTypeDcsState:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slDcsStateHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsStateHeader *) (pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsStateHeader (&pDcsState, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsStateHeader(pDcsState, (SipDcsStateHeader *)\
				(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsStateHeader(pDcsState);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slDcsStateHdr),dIndex,\
				(SIP_Pvoid)pDcsState, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsStateHeader(pDcsState);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsBillingId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pDcsBillingIdHdr!=SIP_NULL)
					sip_dcs_freeSipDcsBillingIdHeader(pGeneralHdr->pDcsBillingIdHdr);
				pGeneralHdr->pDcsBillingIdHdr=(SipDcsBillingIdHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsBillingIdHeader *) ((pHdr)->pHeader))->dRefCount); 
#else				
				if (pGeneralHdr->pDcsBillingIdHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsBillingIdHeader(&(pGeneralHdr->pDcsBillingIdHdr), \
					pErr) == SipFail)
					return SipFail;
					z = SipTrue;
				}
				if (__sip_dcs_cloneSipDcsBillingIdHeader(pGeneralHdr->pDcsBillingIdHdr, \
					(SipDcsBillingIdHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsBillingIdHeader(pGeneralHdr->pDcsBillingIdHdr);
					return SipFail;
				}
#endif				
			}
			break;

		case	SipHdrTypeDcsMediaAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slDcsMediaAuthorizationHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsMediaAuthorizationHeader *) (pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsMediaAuthorizationHeader (&pDcsMedia, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsMediaAuthorizationHeader (pDcsMedia,\
                                                                           (SipDcsMediaAuthorizationHeader *)\
                                                                           (pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsMediaAuthorizationHeader(pDcsMedia);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slDcsMediaAuthorizationHdr),dIndex,\
				(SIP_Pvoid)pDcsMedia, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsMediaAuthorizationHeader(pDcsMedia);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsGate:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pDcsGateHdr!=SIP_NULL)
					sip_dcs_freeSipDcsGateHeader(pGeneralHdr->pDcsGateHdr);
				pGeneralHdr->pDcsGateHdr=(SipDcsGateHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsGateHeader *) ((pHdr)->pHeader))->dRefCount); 
#else				
				if (pGeneralHdr->pDcsGateHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsGateHeader(&(pGeneralHdr->pDcsGateHdr), \
					pErr) == SipFail)
					return SipFail;
					z = SipTrue;
				}
				if (__sip_dcs_cloneSipDcsGateHeader(pGeneralHdr->pDcsGateHdr, \
					(SipDcsGateHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsGateHeader(pGeneralHdr->pDcsGateHdr);
					return SipFail;
				}
#endif				
			}
			break;

		case	SipHdrTypeDcsAnonymity:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slDcsAnonymityHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsAnonymityHeader *) (pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsAnonymityHeader (&pDcsAnonymity, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsAnonymityHeader(pDcsAnonymity, (SipDcsAnonymityHeader *)\
				(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsAnonymityHeader(pDcsAnonymity);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slDcsAnonymityHdr),dIndex,\
				(SIP_Pvoid)pDcsAnonymity, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsAnonymityHeader(pDcsAnonymity);
				return SipFail;
			}
#endif			
			break;
		case	SipHdrTypeDcsRemotePartyId:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slDcsRemotePartyIdHdr), \
				dIndex, (SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsRemotePartyIdHeader *) \
				(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsRemotePartyIdHeader (&pDcsRemotePartyId,\
				pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsRemotePartyIdHeader(pDcsRemotePartyId,\
				(SipDcsRemotePartyIdHeader *)(pHdr->pHeader), pErr) \
				== SipFail)
			{
				sip_dcs_freeSipDcsRemotePartyIdHeader(pDcsRemotePartyId);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slDcsRemotePartyIdHdr),\
				dIndex, (SIP_Pvoid)pDcsRemotePartyId, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsRemotePartyIdHeader(pDcsRemotePartyId);
				return SipFail;
			}
#endif			
			break;
			
		case	SipHdrTypeDcsRpidPrivacy:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slDcsRpidPrivacyHdr), \
				dIndex, (SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsRpidPrivacyHeader *) \
				(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsRpidPrivacyHeader (&pDcsRpidPrivacy,\
				pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsRpidPrivacyHeader(pDcsRpidPrivacy,\
				(SipDcsRpidPrivacyHeader *)(pHdr->pHeader), pErr) \
				== SipFail)
			{
				sip_dcs_freeSipDcsRpidPrivacyHeader(pDcsRpidPrivacy);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slDcsRpidPrivacyHdr),\
				dIndex, (SIP_Pvoid)pDcsRpidPrivacy, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsRpidPrivacyHeader(pDcsRpidPrivacy);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsLaes:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pDcsLaesHdr!=SIP_NULL)
					sip_dcs_freeSipDcsLaesHeader(pGeneralHdr->pDcsLaesHdr);
				pGeneralHdr->pDcsLaesHdr=(SipDcsLaesHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsLaesHeader *) ((pHdr)->pHeader))->dRefCount); 
#else				
				if (pGeneralHdr->pDcsLaesHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsLaesHeader(&(pGeneralHdr->pDcsLaesHdr), \
					pErr) == SipFail)
					return SipFail;
					z = SipTrue;
				}
				if (__sip_dcs_cloneSipDcsLaesHeader(pGeneralHdr->pDcsLaesHdr, \
					(SipDcsLaesHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsLaesHeader(pGeneralHdr->pDcsLaesHdr);
					return SipFail;
				}
#endif				
			}
			break;

		case	SipHdrTypeDcsBillingInfo:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pDcsBillingInfoHdr!=SIP_NULL)
					sip_dcs_freeSipDcsBillingInfoHeader(pGeneralHdr->pDcsBillingInfoHdr);
				pGeneralHdr->pDcsBillingInfoHdr=(SipDcsBillingInfoHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsBillingInfoHeader *) ((pHdr)->pHeader))->dRefCount); 
#else				
				if (pGeneralHdr->pDcsBillingInfoHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsBillingInfoHeader(&(pGeneralHdr->pDcsBillingInfoHdr), \
					pErr) == SipFail)
					return SipFail;
					z = SipTrue;
				}
				if (__sip_dcs_cloneSipDcsBillingInfoHeader(pGeneralHdr->pDcsBillingInfoHdr, \
					(SipDcsBillingInfoHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsBillingInfoHeader(pGeneralHdr->pDcsBillingInfoHdr);
					return SipFail;
				}
#endif				
			}
			break;
					 
		default:*pErr = E_INV_TYPE;
			return SipFail;
	}
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_setDcsGeneralHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_setDcsRequestHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr, SipHeader *pHdr,   SIP_U32bit dIndex, SipError *pErr)
#else
	(pRequestHdr, pHdr, dIndex, pErr)
	SipReqHeader *pRequestHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	en_SipBoolean z=SipFalse;
#endif
	SIPDEBUGFN("Entering function sip_dcs_setDcsRequestHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pRequestHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
		switch (pHdr->dType)
	{
		case	SipHdrTypeDcsOsps	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pDcsOspsHdr != SIP_NULL)
					sip_dcs_freeSipDcsOspsHeader(pRequestHdr->\
						pDcsOspsHdr);
				pRequestHdr->pDcsOspsHdr=(SipDcsOspsHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsOspsHeader *)(pHdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pDcsOspsHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsOspsHeader(\
						&(pRequestHdr->pDcsOspsHdr), \
						pErr) == SipFail)
						return SipFail;
					z = SipTrue;	
				}
				if (__sip_dcs_cloneSipDcsOspsHeader(\
					pRequestHdr->pDcsOspsHdr, \
					(SipDcsOspsHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsOspsHeader(\
							pRequestHdr->pDcsOspsHdr);
					return SipFail;
				}
#endif				
			 }
			 break;

		case	SipHdrTypeDcsRedirect	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pDcsRedirectHdr != SIP_NULL)
					sip_dcs_freeSipDcsRedirectHeader(pRequestHdr->\
						pDcsRedirectHdr);
				pRequestHdr->pDcsRedirectHdr=(SipDcsRedirectHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsRedirectHeader *)(pHdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pDcsRedirectHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsRedirectHeader(\
						&(pRequestHdr->pDcsRedirectHdr), \
						pErr) == SipFail)
						return SipFail;
					z = SipTrue;	
				}
				if (__sip_dcs_cloneSipDcsRedirectHeader(\
					pRequestHdr->pDcsRedirectHdr, \
					(SipDcsRedirectHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsRedirectHeader(\
							pRequestHdr->pDcsRedirectHdr);
					return SipFail;
				}
#endif				
			 }
			 break;

		case	SipHdrTypeDcsTracePartyId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pDcsTracePartyIdHdr != SIP_NULL)
					sip_dcs_freeSipDcsTracePartyIdHeader(pRequestHdr->\
						pDcsTracePartyIdHdr);
				pRequestHdr->pDcsTracePartyIdHdr=(SipDcsTracePartyIdHeader *)(pHdr->pHeader);
				HSS_LOCKEDINCREF(((SipDcsTracePartyIdHeader *)(pHdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pDcsTracePartyIdHdr == SIP_NULL)
				{
					if (sip_dcs_initSipDcsTracePartyIdHeader(\
						&(pRequestHdr->pDcsTracePartyIdHdr), \
						pErr) == SipFail)
						return SipFail;
					z = SipTrue;	
				}
				if (__sip_dcs_cloneSipDcsTracePartyIdHeader(\
					pRequestHdr->pDcsTracePartyIdHdr, \
					(SipDcsTracePartyIdHeader *)(pHdr->pHeader), pErr) == SipFail)
				{
					if (z == SipTrue)
						sip_dcs_freeSipDcsTracePartyIdHeader(\
							pRequestHdr->pDcsTracePartyIdHdr);
					return SipFail;
				}
#endif				
			 }
			 break;

		default				: *pErr = E_INV_TYPE;
						return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_setDcsRequestHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_setDcsResponseHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipRespHeader *pResponseHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pResponseHdr,pHdr, dIndex,pErr)
	SipRespHeader *pResponseHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipSessionHeader *pSession;
#endif
	SIPDEBUGFN("Entering function sip_dcs_setDcsResponseHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pResponseHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pHdr->dType == SipHdrTypeSession)
	{
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slSessionHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipSessionHeader *) (pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipSessionHeader (&pSession, pErr)\
				== SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipSessionHeader(pSession,\
				(SipSessionHeader *)(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipSessionHeader(pSession);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slSessionHdr),dIndex,\
				(SIP_Pvoid)pSession, pErr) == SipFail)
			{
				sip_dcs_freeSipSessionHeader(pSession);
				return SipFail;
			}
#endif
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_setDcsResponseHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_insertDcsGeneralHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipGeneralHeader *pGeneralHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pGeneralHdr, pHdr, dIndex, pErr)
	SipGeneralHeader *pGeneralHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipDcsStateHeader *pDcsState;
	SipDcsMediaAuthorizationHeader *pDcsMedia;
	SipDcsAnonymityHeader *pDcsAnonymity;
	SipDcsRpidPrivacyHeader *pDcsRpidPrivacy;
	SipDcsRemotePartyIdHeader *pDcsRemotePartyId;
#endif
	SIPDEBUGFN("Entering function sip_dcs_insertDcsGeneralHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pGeneralHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (pHdr->dType)
	{
		case	SipHdrTypeDcsState:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slDcsStateHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsStateHeader *)(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsStateHeader (&pDcsState, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsStateHeader(pDcsState, \
				(SipDcsStateHeader *)(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsStateHeader(pDcsState);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slDcsStateHdr),dIndex,\
				(SIP_Pvoid)pDcsState, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsStateHeader(pDcsState);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsBillingId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsBillingIdHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pDcsBillingIdHdr=(SipDcsBillingIdHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsBillingIdHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsBillingIdHeader(&(pGeneralHdr->pDcsBillingIdHdr),\
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsBillingIdHeader(pGeneralHdr->pDcsBillingIdHdr,\
						(SipDcsBillingIdHeader *)(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsBillingIdHeader(pGeneralHdr->pDcsBillingIdHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		case	SipHdrTypeDcsMediaAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slDcsMediaAuthorizationHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsMediaAuthorizationHeader *)(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsMediaAuthorizationHeader (&pDcsMedia, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsMediaAuthorizationHeader(pDcsMedia, \
				(SipDcsMediaAuthorizationHeader *)(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsMediaAuthorizationHeader(pDcsMedia);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slDcsMediaAuthorizationHdr),dIndex,\
				(SIP_Pvoid)pDcsMedia, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsMediaAuthorizationHeader(pDcsMedia);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsGate:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsGateHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pDcsGateHdr=(SipDcsGateHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsGateHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsGateHeader(&(pGeneralHdr->pDcsGateHdr),\
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsGateHeader(pGeneralHdr->pDcsGateHdr,\
						(SipDcsGateHeader *)(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsGateHeader(pGeneralHdr->pDcsGateHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		case	SipHdrTypeDcsAnonymity:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slDcsAnonymityHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsAnonymityHeader *)(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsAnonymityHeader (&pDcsAnonymity, pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsAnonymityHeader(pDcsAnonymity, \
				(SipDcsAnonymityHeader *)(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipDcsAnonymityHeader(pDcsAnonymity);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slDcsAnonymityHdr),dIndex,\
				(SIP_Pvoid)pDcsAnonymity, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsAnonymityHeader(pDcsAnonymity);
				return SipFail;
			}
#endif			
			break;
		case	SipHdrTypeDcsRemotePartyId:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slDcsRemotePartyIdHdr),\
				dIndex, (SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsRemotePartyIdHeader *)\
			(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsRemotePartyIdHeader (&pDcsRemotePartyId,\
				pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsRemotePartyIdHeader(pDcsRemotePartyId,\
				(SipDcsRemotePartyIdHeader *)(pHdr->pHeader), pErr) \
				== SipFail)
			{
				sip_dcs_freeSipDcsRemotePartyIdHeader(pDcsRemotePartyId);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slDcsRemotePartyIdHdr),\
				dIndex, (SIP_Pvoid)pDcsRemotePartyId, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsRemotePartyIdHeader(pDcsRemotePartyId);
				return SipFail;
			}
#endif			
			break;
			
		case	SipHdrTypeDcsRpidPrivacy:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slDcsRpidPrivacyHdr),\
				dIndex, (SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipDcsRpidPrivacyHeader *)\
			(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipDcsRpidPrivacyHeader (&pDcsRpidPrivacy,\
				pErr) == SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipDcsRpidPrivacyHeader(pDcsRpidPrivacy,\
				(SipDcsRpidPrivacyHeader *)(pHdr->pHeader), pErr) \
				== SipFail)
			{
				sip_dcs_freeSipDcsRpidPrivacyHeader(pDcsRpidPrivacy);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slDcsRpidPrivacyHdr),\
				dIndex, (SIP_Pvoid)pDcsRpidPrivacy, pErr) == SipFail)
			{
				sip_dcs_freeSipDcsRpidPrivacyHeader(pDcsRpidPrivacy);
				return SipFail;
			}
#endif			
			break;

		case	SipHdrTypeDcsLaes:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsLaesHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pDcsLaesHdr=(SipDcsLaesHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsLaesHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsLaesHeader(&(pGeneralHdr->pDcsLaesHdr),\
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsLaesHeader(pGeneralHdr->pDcsLaesHdr,\
						(SipDcsLaesHeader *)(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsLaesHeader(pGeneralHdr->pDcsLaesHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		case	SipHdrTypeDcsBillingInfo:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDcsBillingInfoHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pDcsBillingInfoHdr=(SipDcsBillingInfoHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsBillingInfoHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsBillingInfoHeader(&(pGeneralHdr->pDcsBillingInfoHdr),\
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsBillingInfoHeader(pGeneralHdr->pDcsBillingInfoHdr,\
						(SipDcsBillingInfoHeader *)(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsBillingInfoHeader(pGeneralHdr->pDcsBillingInfoHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;
					 
		default:*pErr = E_INV_TYPE;
			return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_insertDcsGeneralHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_insertDcsRequestHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pRequestHdr, pHdr, dIndex, pErr)
	SipReqHeader *pRequestHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_dcs_insertDcsRequestHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pRequestHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	switch (pHdr->dType)
	{
		case	SipHdrTypeDcsOsps	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsOspsHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE						
					pRequestHdr->pDcsOspsHdr=(SipDcsOspsHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsOspsHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsOspsHeader(\
						&(pRequestHdr->pDcsOspsHdr), \
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsOspsHeader(\
						pRequestHdr->pDcsOspsHdr, (SipDcsOspsHeader *)\
						(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsOspsHeader(\
							pRequestHdr->pDcsOspsHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		case	SipHdrTypeDcsRedirect	:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsRedirectHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE						
					pRequestHdr->pDcsRedirectHdr=(SipDcsRedirectHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsRedirectHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsRedirectHeader(\
						&(pRequestHdr->pDcsRedirectHdr), \
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsRedirectHeader(\
						pRequestHdr->pDcsRedirectHdr, (SipDcsRedirectHeader *)\
						(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsRedirectHeader(\
							pRequestHdr->pDcsRedirectHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		case	SipHdrTypeDcsTracePartyId:
			if (dIndex > 0)
			{
				*pErr = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pDcsTracePartyIdHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE						
					pRequestHdr->pDcsTracePartyIdHdr=(SipDcsTracePartyIdHeader *)(pHdr->pHeader);
					HSS_LOCKEDINCREF(((SipDcsTracePartyIdHeader *)(pHdr->pHeader))->dRefCount);
#else					
					if (sip_dcs_initSipDcsTracePartyIdHeader(\
						&(pRequestHdr->pDcsTracePartyIdHdr), \
						pErr) == SipFail)
						return SipFail;
					if (__sip_dcs_cloneSipDcsTracePartyIdHeader(\
						pRequestHdr->pDcsTracePartyIdHdr, (SipDcsTracePartyIdHeader *)\
						(pHdr->pHeader), pErr) == SipFail)
					{
						sip_dcs_freeSipDcsTracePartyIdHeader(\
							pRequestHdr->pDcsTracePartyIdHdr);
						return SipFail;
					}
#endif					
				}
				else
				{
					*pErr = E_INV_INDEX;
					return SipFail;
				}
			 }
			 break;

		default				: *pErr = E_INV_TYPE;
						return SipFail;
	}

	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_insertDcsRequestHeaderAtIndex");
	return SipSuccess;
}


SipBool sip_dcs_insertDcsResponseHeaderAtIndex 
#ifdef ANSI_PROTO
	(SipRespHeader *pResponseHdr,  SipHeader *pHdr, SIP_U32bit dIndex, SipError *pErr)
#else
	(pResponseHdr, pHdr, dIndex, pErr)
	SipRespHeader *pResponseHdr;
	SipHeader *pHdr;
	SIP_U32bit dIndex;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipSessionHeader *pSession;
#endif
	SIPDEBUGFN("Entering function sip_dcs_insertDcsResponseHeaderAtIndex");
#ifndef SIP_NO_CHECK
	if(pErr == SIP_NULL)
	{
		return SipFail;
	}
	if((pHdr == SIP_NULL) || (pResponseHdr == SIP_NULL))
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
#endif
	if (pHdr->dType == SipHdrTypeSession)
	{
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slSessionHdr),dIndex,\
				(SIP_Pvoid)(pHdr->pHeader), pErr) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipSessionHeader *)(pHdr->pHeader))->dRefCount);
#else			
			if (sip_dcs_initSipSessionHeader (&pSession, pErr) \
				== SipFail)
				return SipFail;
			if (__sip_dcs_cloneSipSessionHeader(pSession,\
				(SipSessionHeader *)(pHdr->pHeader), pErr) == SipFail)
			{
				sip_dcs_freeSipSessionHeader(pSession);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slSessionHdr),\
				dIndex, (SIP_Pvoid)pSession, pErr) == SipFail)
			{
				sip_dcs_freeSipSessionHeader(pSession);
				return SipFail;
			}
#endif
	}
	else
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_dcs_insertDcsResponseHeaderAtIndex");
	return SipSuccess;
}

SipBool sip_dcs_validateDcsHeaderString
#ifdef ANSI_PROTO
(SIP_S8bit *pStr, en_HeaderType dType, SipError *pErr)
#else
(pStr, dType, pErr)
SIP_S8bit *pStr;
en_HeaderType dType;
SipError *pErr;
#endif
{
	SipBool retval;
	SipError *dummy;
	dummy = pErr;
	switch (dType)
	{
		case SipHdrTypeSession:
			if((strcasecmp("Session:",pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsTracePartyId:
			if ((strcasecmp("Dcs-Trace-Party-Id:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsRedirect:
			if ((strcasecmp("Dcs-Redirect:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsOsps:
			if ((strcasecmp("Dcs-Osps:",pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsBillingInfo:
			if ((strcasecmp("Dcs-Billing-Info:",pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsLaes:
			if ((strcasecmp("Dcs-LAES:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsAnonymity:
			if ((strcasecmp("Anonymity:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsRpidPrivacy:
			if ((strcasecmp("RPID-Privacy:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsGate:
			if ((strcasecmp("Dcs-Gate:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsRemotePartyId:
			if ((strcasecmp("Remote-Party-Id:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsMediaAuthorization:
			if ((strcasecmp("P-Media-Authorization:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsBillingId:
			if ((strcasecmp("Dcs-Billing-Id:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
		case SipHdrTypeDcsState:
			if ((strcasecmp("State:", pStr))==0)
				retval=SipSuccess;
			else retval=SipFail;
			break;
			
		default:
			retval=SipFail;
	}
	return retval;
}
