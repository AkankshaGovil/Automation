/******************************************************************************
** FUNCTION:
**
**
*******************************************************************************
**
** FILENAME:
** 	sipinternal.c
**
** DESCRIPTION:
**
**
** DATE      NAME          REFERENCE      REASON
** ----      ----          ---------      ------
** 13Dec99   S.Luthra	   Creation
**	         B.Borthakur
**	         Preethy
**
** Copyrights 1999, Hughes Software Systems, Ltd.
******************************************************************************/

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"
#include "sipinit.h"
#include "sipbcptinit.h"
#ifdef SIP_CCP
#include "ccpinit.h"
#include "ccpfree.h"
#include "ccpinternal.h"
#endif /* of ccp */

#ifdef SIP_IMPP
#include "imppinit.h"
#include "imppfree.h"
#include "imppinternal.h"
#endif

#include "rprinit.h"
#include "sipfree.h"
#include "sipbcptfree.h"
#include "rprfree.h"
#include "siplist.h"
#include "sipclone.h"
#include "sipinternal.h"
#include "rprinternal.h"
#include "sipbcptinternal.h"
#include "sipvalidate.h"
#include "sdp.h"
#include "sipdecode.h"
#include "sipdecodeintrnl.h"
#include "sipclone.h"
#ifdef SIP_DCS
#include "dcsintrnl.h"
#endif
#include "sipparserinc.h"


/******************************************************************
**
** FUNCTION:  getRequestHdrCount
**
** DESCRIPTION: This function returns the number of request headers
**	of a particular en_HeaderType "dType".
**
******************************************************************/
SipBool getRequestHdrCount
#ifdef ANSI_PROTO
	(SipReqHeader *hdr, en_HeaderType dType, SIP_U32bit *count, SipError *err)
#else
	(hdr, dType, count, err)
	SipReqHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIP_U32bit temp_count;
	SIPDEBUGFN("Entering function getRequestHdrCount");
	switch (dType)
	{

		case SipHdrTypeAuthorization:
				if (sip_listSizeOf(&(hdr->slAuthorizationHdr),&temp_count,\
					err) == SipFail)
					return SipFail;
				*count = temp_count;
				break;

		case SipHdrTypeHide:
				if (hdr->pHideHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;

		case SipHdrTypeReplaces:
				if (hdr->pReplacesHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;

				
		case SipHdrTypeMaxforwards:
				if (hdr->pMaxForwardsHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
#ifdef SIP_IMPP
		case	SipHdrTypeEvent	:
			if (hdr->pEventHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#endif

		case SipHdrTypePriority:
				if (hdr->pPriorityHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
		case SipHdrTypeProxyauthorization:
				if (sip_listSizeOf(&(hdr->slProxyAuthorizationHdr),&temp_count,\
					err) == SipFail)
					return SipFail;
				*count = temp_count;
				break;
		/* call-transfer */
		case SipHdrTypeReferTo:
				if (hdr->pReferToHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
		case SipHdrTypeReferredBy:
				if (hdr->pReferredByHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break; /* call-tarnsfer */
		case SipHdrTypeProxyRequire:
				if (sip_listSizeOf(&(hdr->slProxyRequireHdr), &temp_count, \
					err) == SipFail)
					return SipFail;
				*count = temp_count;
				break;
		case SipHdrTypeRoute:
				if (sip_listSizeOf(&(hdr->slRouteHdr), &temp_count, err)\
					 == SipFail)
					return SipFail;
				*count = temp_count;
				break;
		case SipHdrTypeAlso:
				if (sip_listSizeOf(&(hdr->slAlsoHdr), &temp_count, err)\
					 == SipFail)
					return SipFail;
				*count = temp_count;
				break;
		case SipHdrTypeResponseKey:
				if (hdr->pRespKeyHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
		case SipHdrTypeSubject:
				if (hdr->pSubjectHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
			/* Retrans */
		case SipHdrTypeRAck:
				if (hdr->pRackHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;
		case SipHdrTypeWwwAuthenticate:
				if (sip_listSizeOf(&(hdr->slWwwAuthenticateHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				break;
#ifdef SIP_CCP				
		case SipHdrTypeAcceptContact:
				if (sip_listSizeOf(&(hdr->slAcceptContactHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				break; /* CCP case */
		case SipHdrTypeRejectContact:
				if (sip_listSizeOf(&(hdr->slRejectContactHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				break; /* CCP case */
		case SipHdrTypeRequestDisposition:
				if (sip_listSizeOf(&(hdr->slRequestDispositionHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				break; /* CCP case */
#endif				
		case SipHdrTypeAlertInfo:
				if (sip_listSizeOf(&(hdr->slAlertInfoHdr),\
					 &temp_count, err) == SipFail)
					return SipFail;
				*count = temp_count;
				break; /* CCP case */
		case SipHdrTypeInReplyTo:
				if (sip_listSizeOf(&(hdr->slInReplyToHdr),\
					 &temp_count, err) == SipFail)
					return SipFail;
				*count = temp_count;
				break; /* CCP case */
#ifdef SIP_IMPP
		case SipHdrTypeSubscriptionState:
			if (hdr->pSubscriptionStateHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#endif
#ifdef SIP_CONF            
		/*Join header*/
        case SipHdrTypeJoin:
			if (hdr->pJoinHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#endif
		/*Security-Client Header */
#ifdef SIP_SECURITY
	case SipHdrTypeSecurityClient:
			if (sip_listSizeOf(&(hdr->slSecurityClientHdr), &temp_count, err)== SipFail)
				return SipFail;
			*count = temp_count;
			break;
	case SipHdrTypeSecurityVerify:
                        if (sip_listSizeOf(&(hdr->slSecurityVerifyHdr), &temp_count, err)== SipFail)
                                return SipFail;
                        *count = temp_count;
                        break;
#endif
#ifdef SIP_3GPP
            /*P-Called-Party-Id header */
		case SipHdrTypePCalledPartyId:
			if (hdr->pPCalledPartyIdHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
            /*P-Visited-Network-Id header */
  		case SipHdrTypePVisitedNetworkId:
				if (sip_listSizeOf(&(hdr->slPVisitedNetworkIdHdr),\
					 &temp_count, err) == SipFail)
					return SipFail;
				*count = temp_count;
				break;
#endif
#ifdef SIP_CONGEST
            /*RsrcPriority */
		case SipHdrTypeRsrcPriority:
			if (sip_listSizeOf(&(hdr->slRsrcPriorityHdr),\
				 &temp_count, err) == SipFail)
				return SipFail;
			
            *count = temp_count;
			
            break;
#endif                

		default:
#ifdef SIP_DCS
				if (sip_dcs_getDcsRequestHeaderCount (hdr, dType, count, err) == SipSuccess)
					break;
				if (*err != E_INV_TYPE)
					return SipFail;
#endif
				*err = E_INV_TYPE;
				return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getRequestHdrCount");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  getResponseHdrCount
**
** DESCRIPTION:  This function returns the number of response headers
**	of a particular en_HeaderType "dType".
**
******************************************************************/
SipBool getResponseHdrCount
#ifdef ANSI_PROTO
	(SipRespHeader *hdr, en_HeaderType dType, SIP_U32bit *count, SipError *err)
#else
	(hdr, dType, count, err)
	SipRespHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIP_U32bit temp_count;
	SIPDEBUGFN("Entering function getResponseHdrCount");
	switch (dType)
	{
			case SipHdrTypeProxyAuthenticate:
			if (sip_listSizeOf(&(hdr->slProxyAuthenticateHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
		case SipHdrTypeAuthenticationInfo:
			if (sip_listSizeOf(&(hdr->slAuthenticationInfoHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
	 	case SipHdrTypeMinExpires:
			if (hdr->pMinExpiresHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break; 
		case SipHdrTypeServer:
			/* if (sip_listSizeOf(&(hdr->slServerHdr), &temp_count, err)\
				 == SipFail)
				return SipFail;
			*count = temp_count; */
			if (hdr->pServerHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
		case SipHdrTypeUnsupported:
			if (sip_listSizeOf(&(hdr->slUnsupportedHdr), &temp_count, err)\
				 == SipFail)
				return SipFail;
			*count = temp_count;
			break;
		case SipHdrTypeWarning:
			if (sip_listSizeOf(&(hdr->slWarningHeader), &temp_count, err)\
				 == SipFail)
				return SipFail;
			*count = temp_count;
			break;
		case SipHdrTypeWwwAuthenticate:
			if (sip_listSizeOf(&(hdr->slWwwAuthenticateHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
		case SipHdrTypeErrorInfo:
			if (sip_listSizeOf(&(hdr->slErrorInfoHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
		case SipHdrTypeAuthorization:
			if (sip_listSizeOf(&(hdr->slAuthorizationHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
	/* Retrans */
		case SipHdrTypeRSeq:
			if (hdr->pRSeqHdr != SIP_NULL)
				*count = 1;
			else
			 	*count = 0;
			break;
#ifdef SIP_3GPP
            /*P-Associated-Uri header */
		case SipHdrTypePAssociatedUri:
            if (sip_listSizeOf(&(hdr->slPAssociatedUriHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
#endif
#ifdef SIP_CONGEST
        /*AcceptRsrcPriority */
		case SipHdrTypeAcceptRsrcPriority:
        if (sip_listSizeOf(&(hdr->slAcceptRsrcPriorityHdr), &temp_count,\
    		 err) == SipFail)
			return SipFail;

        *count = temp_count;
            
        break;
#endif 
#ifdef SIP_SECURITY
	case SipHdrTypeSecurityServer:
                        if (sip_listSizeOf(&(hdr->slSecurityServerHdr), &temp_count, err)== SipFail)
                                return SipFail;
                        *count = temp_count;
                        break;
#endif 
            
		default:
#ifdef SIP_DCS
				if (sip_dcs_getDcsResponseHeaderCount (hdr, dType, count, err) == SipSuccess)
					break;
				if (*err != E_INV_TYPE)
					return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getResponseHdrCount");
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  getGeneralHdrCount
**
** DESCRIPTION: This function returns the number of general headers of en_HeaderType "dType" present in SipMessage "msg". The pValue is returned in the variable "count". For SipHeaders in which the dType "Any is possible" the tupe passed must be "Any" - otherwise E_INV_TYPE is returned.
**
*********************************************************************/
SipBool getGeneralHdrCount
#ifdef ANSI_PROTO
	(SipGeneralHeader *hdr, en_HeaderType dType, SIP_U32bit *count, SipError *err)
#else
	(hdr, dType, count, err)
	SipGeneralHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit *count;
	SipError *err;
#endif
{
	SIP_U32bit temp_count;
	SIPDEBUGFN("Entering function getGeneralHdrCount");
	switch (dType)
	{

		case	SipHdrTypeAccept		:if (sip_listSizeOf(&(hdr->slAcceptHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case	SipHdrTypeAcceptEncoding	:if (sip_listSizeOf(&(hdr->slAcceptEncoding), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case 	SipHdrTypeAcceptLanguage	:if (sip_listSizeOf(&(hdr->slAcceptLang), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case 	SipHdrTypeCallId		:if (hdr->pCallidHdr != SIP_NULL)
								*count = 1;
							 else
								 *count = 0;
							 break;
		case	SipHdrTypeCseq			:if (hdr->pCseqHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeDate			:if (hdr->pDateHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeEncryption		:if (hdr->pEncryptionHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
		case	SipHdrTypeReplyTo		:
					if (hdr->pReplyToHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
					 
		case	SipHdrTypeExpiresAny		:if (hdr->pExpiresHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
#ifdef SIP_SESSIONTIMER

		/*NEW-CHANGE*/
		case 	SipHdrTypeMinSE:
								if (hdr->pMinSEHdr != SIP_NULL)
									*count = 1;
							 	else
									*count = 0;
							 	break;


		case	SipHdrTypeSessionExpires		:if (hdr->pSessionExpiresHdr\
													!= SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
#endif


		case	SipHdrTypeFrom			:if (hdr->pFromHeader != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeRecordRoute		:if (sip_listSizeOf(&(hdr->slRecordRouteHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;
#ifdef SIP_3GPP
		case	SipHdrTypePath	:
							if (sip_listSizeOf(&(hdr->slPathHdr), \
								&temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;
		case	SipHdrTypeServiceRoute	:
							if (sip_listSizeOf(&(hdr->slServiceRouteHdr), \
								&temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;
		case	SipHdrTypePanInfo		:if (hdr->pPanInfoHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
		case	SipHdrTypePcVector		:if (hdr->pPcVectorHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
					 
#endif
		case	SipHdrTypeTimestamp		:if (hdr->pTimeStampHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeTo			:if (hdr->pToHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeVia			:if (sip_listSizeOf(&(hdr->slViaHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case	SipHdrTypeContentEncoding	:if (sip_listSizeOf(&(hdr->slContentEncodingHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;
	/* RPR */
		case	SipHdrTypeSupported	:if (sip_listSizeOf(&(hdr->slSupportedHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case	SipHdrTypeContentLength		:if (hdr->pContentLengthHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;

		case	SipHdrTypeContentType		:if (hdr->pContentTypeHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
/* bcpt ext */
		case	SipHdrTypeMimeVersion		:if (hdr->pMimeVersionHdr != SIP_NULL)
								*count = 1;
							 else
								*count = 0;
							 break;
/* bcpt ext ends */
		case	SipHdrTypeUnknown		:if (sip_listSizeOf(&(hdr->slUnknownHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;

		case	SipHdrTypeContactAny		:if (sip_listSizeOf(&(hdr->slContactHdr), &temp_count, err) == SipFail)
								return SipFail;
							 *count = temp_count;
							 break;
		case SipHdrTypeUserAgent:
				if (hdr->pUserAgentHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;

		case SipHdrTypeOrganization:
				if (hdr->pOrganizationHdr != SIP_NULL)
					*count = 1;
				else
					*count = 0;
				break;

		case SipHdrTypeRequire:
				if (sip_listSizeOf(&(hdr->slRequireHdr), &temp_count, err)\
					 == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeCallInfo:
				if (sip_listSizeOf(&(hdr->slCallInfoHdr), &temp_count, err)\
					 == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeAllow:
				if (sip_listSizeOf(&(hdr->slAllowHdr), &temp_count, err)\
					 == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeContentLanguage:
				if (sip_listSizeOf(&(hdr->slContentLanguageHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeContentDisposition:
				if (sip_listSizeOf(&(hdr->slContentDispositionHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeReason:
				if (sip_listSizeOf(&(hdr->slReasonHdr), &temp_count,\
					 err) == SipFail)
					return SipFail;
				*count = temp_count;
				 break;
		case SipHdrTypeRetryAfterAny:
			if (hdr->pRetryAfterHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId :
			if (sip_listSizeOf(&(hdr->slPAssertIdHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break ;
		case SipHdrTypePPreferredId :
			if (sip_listSizeOf(&(hdr->slPPreferredIdHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
                        break ;
		case SipHdrTypePrivacy:
			if(hdr->pPrivacyHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#endif 
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
			if (sip_listSizeOf(&(hdr->slAllowEventsHdr), &temp_count,\
				 err) == SipFail)
				return SipFail;
			*count = temp_count;
			break;
#endif
#ifdef SIP_3GPP
        /*P-Charging-Function-Addresses header */
		case SipHdrTypePcfAddr:
			if (hdr->pPcfAddrHdr != SIP_NULL)
				*count = 1;
			else
				*count = 0;
			break;
#endif
		default	:
#ifdef SIP_DCS
				if (sip_dcs_getDcsGeneralHeaderCount (hdr, dType, count, err) == SipSuccess)
					break;
				if (*err != E_INV_TYPE)
					return SipFail;
#endif
				*err = E_INV_TYPE;
				 return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getGeneralHdrCount");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  isSipGeneralHeader
**
** DESCRIPTION: This function returns "SipSuccess" if "dType" is one
**	among the defined general headers.
**
******************************************************************/
en_SipBoolean isSipGeneralHeader
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function isSipGeneralHeader");
	switch(dType)
	{
		case SipHdrTypeAccept:
		case SipHdrTypeAllow:
		case SipHdrTypeAcceptEncoding:
		case SipHdrTypeAcceptLanguage:
		case SipHdrTypeCallId:
		case SipHdrTypeCseq:
		case SipHdrTypeDate:
		case SipHdrTypeEncryption:
		case SipHdrTypeReplyTo:
		case SipHdrTypeExpiresDate:
		case SipHdrTypeExpiresSec:
		case SipHdrTypeExpiresAny:
		case SipHdrTypeFrom:
		case SipHdrTypeRecordRoute:
#ifdef SIP_3GPP
		case SipHdrTypeServiceRoute:
		case SipHdrTypePath:
		case SipHdrTypePanInfo:
		case SipHdrTypePcVector:
#endif
		case SipHdrTypeTimestamp:
		case SipHdrTypeTo:
		case SipHdrTypeVia:
		case SipHdrTypeContentEncoding:
		case SipHdrTypeContentLength:
		case SipHdrTypeContentLanguage:
		case SipHdrTypeContentType:
		case SipHdrTypeContactWildCard:
		case SipHdrTypeContactNormal:
		case SipHdrTypeContactAny:
		case SipHdrTypeMimeVersion: /* bcpt ext */
		case SipHdrTypeSupported: /* RPR */
		case SipHdrTypeOrganization:
		case SipHdrTypeRequire:
		case SipHdrTypeContentDisposition:
		case SipHdrTypeReason:
		case SipHdrTypeCallInfo:
		case SipHdrTypeUserAgent:
		case SipHdrTypeRetryAfterDate:
		case SipHdrTypeRetryAfterSec:
		case SipHdrTypeRetryAfterAny:
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
#endif
#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId:
		case SipHdrTypePPreferredId:
		case SipHdrTypePrivacy:
#endif
		case SipHdrTypeUnknown:

#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE: /*NEW-CHANGE*/
		case SipHdrTypeSessionExpires:
#endif
#ifdef SIP_3GPP
		case SipHdrTypePcfAddr:
#endif
			break;
		default:
#ifdef SIP_DCS
				if (sip_dcs_isSipDcsGeneralHeader(dType) == SipTrue)
					break;
#endif
			return SipFalse;
	}
	SIPDEBUGFN("Exiting function isSipGeneralHeader");
	return SipTrue;
}

/******************************************************************
**
** FUNCTION:  isSipReqHeader
**
** DESCRIPTION:  This function returns "SipSuccess" if "dType" is one
**	among the defined request headers.
**
******************************************************************/
en_SipBoolean isSipReqHeader
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function isSipReqHeader");
	switch (dType)
	{
		case SipHdrTypeAuthorization:
		case SipHdrTypeHide:
		case SipHdrTypeReplaces:
		case SipHdrTypeMaxforwards:
		case SipHdrTypePriority:
		case SipHdrTypeProxyauthorization:
		case SipHdrTypeProxyRequire:
		case SipHdrTypeReferTo: /* call-transfer */
		case SipHdrTypeReferredBy: /* call-transfer */
		case SipHdrTypeRoute:
		case SipHdrTypeAlso:
		case SipHdrTypeResponseKey:
		case SipHdrTypeSubject:
		case SipHdrTypeWwwAuthenticate:
		case SipHdrTypeRAck:/* Retrans */
#ifdef SIP_CCP		
		case SipHdrTypeAcceptContact:/* CCP */
		case SipHdrTypeRejectContact:/* CCP */
		case SipHdrTypeRequestDisposition: /* CCP */
#endif		
		case SipHdrTypeAlertInfo:
		case SipHdrTypeInReplyTo:
#ifdef SIP_IMPP
		case SipHdrTypeSubscriptionState:
		case SipHdrTypeEvent:
#endif
#ifdef SIP_CONF            
		case SipHdrTypeJoin:   /* Join */
#endif
#ifdef SIP_SECURITY
		case SipHdrTypeSecurityClient: /* Security-Client */
		case SipHdrTypeSecurityVerify: /* Security-Verify */
#endif
#ifdef SIP_3GPP
		case SipHdrTypePCalledPartyId:
		case SipHdrTypePVisitedNetworkId:
#endif
#ifdef SIP_CONGEST
		case SipHdrTypeRsrcPriority:
#endif
			break;

		default:
#ifdef SIP_DCS
				if (sip_dcs_isSipDcsRequestHeader(dType) == SipTrue)
					break;
#endif

			return SipFalse;
	}
	SIPDEBUGFN("Exiting function isSipReqHeader");
	return SipTrue;
}

/******************************************************************
**
** FUNCTION:  isSipRespHeader
**
** DESCRIPTION:  This function returns "SipSuccess" if "dType" is one
**	among the defined response headers.
**
******************************************************************/
en_SipBoolean isSipRespHeader
#ifdef ANSI_PROTO
	(en_HeaderType dType)
#else
	(dType)
	en_HeaderType dType;
#endif
{
	SIPDEBUGFN("Entering function isSipRespHeader");
	switch(dType)
	{
		case SipHdrTypeProxyAuthenticate:
		case SipHdrTypeServer:
		case SipHdrTypeUnsupported:
		case SipHdrTypeWarning:
		case SipHdrTypeWwwAuthenticate:
		case SipHdrTypeAuthenticationInfo:
		case SipHdrTypeAuthorization:
		case SipHdrTypeRSeq: /*Retrans */
		case SipHdrTypeErrorInfo:
		case SipHdrTypeMinExpires:
#ifdef SIP_3GPP
		case SipHdrTypePAssociatedUri:
#endif
#ifdef SIP_CONGEST
		case SipHdrTypeAcceptRsrcPriority:
#endif
#ifdef SIP_SECURITY
		case SipHdrTypeSecurityServer: /* Security-Server */
#endif
            
			break;
		default:
#ifdef SIP_DCS
				if (sip_dcs_isSipDcsResponseHeader(dType) == SipTrue)
					break;
#endif

			return SipFalse;
	}
	SIPDEBUGFN("Exiting function isSipRespHeader");
	return SipTrue;
}

/******************************************************************
**
** FUNCTION:  getGeneralHeaderAtIndex
**
** DESCRIPTION: This function returns the general pHeader of
**	en_HeaderType "dType" at the index  specified by "index".
**
******************************************************************/
SipBool getGeneralHeaderAtIndex
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipGeneralHeader *pGeneralHdr, en_HeaderType dType, SipHeader *hdr,
	 SIP_U32bit index, SipError *err)
#else
	(SipGeneralHeader *pGeneralHdr, en_HeaderType dType, SipHeader *hdr,
	 SIP_U32bit index, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pGeneralHdr, dType, hdr, index, err)
	SipGeneralHeader *pGeneralHdr;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#else
	(pGeneralHdr, dType, hdr, index, err)
	SipGeneralHeader *pGeneralHdr;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIP_Pvoid temp;
	SIPDEBUGFN("Entering function getGeneralHeaderAtIndex");

	switch (dType)
	{
		case SipHdrTypeAllow:
			if (sip_listGetAt(&(pGeneralHdr->slAllowHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAllowHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAllowHeader ((SipAllowHeader *)(hdr->pHeader),\
				 (SipAllowHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeContentDisposition:
			if (sip_listGetAt(&(pGeneralHdr->slContentDispositionHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipContentDispositionHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipContentDispositionHeader ((SipContentDispositionHeader *)(hdr->pHeader),\
				 (SipContentDispositionHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeReason:
			if (sip_listGetAt(&(pGeneralHdr->slReasonHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipReasonHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipReasonHeader ((SipReasonHeader *)(hdr->pHeader),\
				 (SipReasonHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeContentLanguage:
			if (sip_listGetAt(&(pGeneralHdr->slContentLanguageHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipContentLanguageHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipContentLanguageHeader ((SipContentLanguageHeader *)(hdr->pHeader),\
				 (SipContentLanguageHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;


		case SipHdrTypeAccept:
			if (sip_listGetAt(&(pGeneralHdr->slAcceptHdr), index, &temp, \
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			(hdr)->pHeader = (SIP_Pvoid)temp;
			HSS_LOCKEDINCREF(((SipAcceptHeader *)((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAcceptHeader ((SipAcceptHeader *)(hdr->pHeader),\
				 (SipAcceptHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeAcceptEncoding:
			if (sip_listGetAt(&(pGeneralHdr->slAcceptEncoding), index, &temp,\
				 err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid )temp;
			HSS_LOCKEDINCREF(((SipAcceptEncodingHeader *)	((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAcceptEncodingHeader ((SipAcceptEncodingHeader *)\
				(hdr->pHeader), (SipAcceptEncodingHeader *)temp, err) == \
				SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeAcceptLanguage:
			if (sip_listGetAt(&(pGeneralHdr->slAcceptLang), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid)temp;
			HSS_LOCKEDINCREF(((SipAcceptLangHeader *)((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAcceptLangHeader ((SipAcceptLangHeader *)\
				(hdr->pHeader), (SipAcceptLangHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeCallId:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pCallidHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid ) pGeneralHdr->pCallidHdr;
					HSS_LOCKEDINCREF(((SipCallIdHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipCallIdHeader( (SipCallIdHeader *)\
						(hdr->pHeader), pGeneralHdr->pCallidHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			 }
			 break;

		case SipHdrTypeCseq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pCseqHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pCseqHdr;
					HSS_LOCKEDINCREF(((SipCseqHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipCseqHeader( (SipCseqHeader *)(hdr->pHeader)\
						, pGeneralHdr->pCseqHdr, err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDateHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid ) pGeneralHdr->pDateHdr;
					HSS_LOCKEDINCREF(((SipDateHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipDateHeader( (SipDateHeader *)\
						(hdr->pHeader), pGeneralHdr->pDateHdr, err)\
						 == SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeEncryption:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pEncryptionHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					(hdr)->pHeader = (SIP_Pvoid)   pGeneralHdr->pEncryptionHdr;
					HSS_LOCKEDINCREF(((SipEncryptionHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipEncryptionHeader( (SipEncryptionHeader *)\
						(hdr->pHeader), pGeneralHdr->pEncryptionHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;

	case SipHdrTypeReplyTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pReplyToHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					(hdr)->pHeader = (SIP_Pvoid)   pGeneralHdr->pReplyToHdr;
					HSS_LOCKEDINCREF(((SipReplyToHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipReplyToHeader( (SipReplyToHeader *)\
						(hdr->pHeader), pGeneralHdr->pReplyToHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;

		case SipHdrTypeExpiresAny:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pExpiresHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pExpiresHdr;
					HSS_LOCKEDINCREF(((SipExpiresHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipExpiresHeader( (SipExpiresHeader *)\
						(hdr->pHeader), pGeneralHdr->pExpiresHdr, err)\
						 == SipFail)
						return SipFail;
#endif
				}
			}
			break;
#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pMinSEHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid ) pGeneralHdr->pMinSEHdr;
					HSS_LOCKEDINCREF(((SipMinSEHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipMinSEHeader( (SipMinSEHeader *)\
						(hdr->pHeader), pGeneralHdr->pMinSEHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			 }
			 break;

		case SipHdrTypeSessionExpires:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pSessionExpiresHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pSessionExpiresHdr;
					HSS_LOCKEDINCREF(((SipSessionExpiresHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipSessionExpiresHeader( (SipSessionExpiresHeader *)\
						(hdr->pHeader), pGeneralHdr->pSessionExpiresHdr, err)\
						 == SipFail)
						return SipFail;
#endif
				}
			}
			break;
#endif



		case SipHdrTypeFrom:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pFromHeader == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pFromHeader;
					HSS_LOCKEDINCREF(((SipFromHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipFromHeader((SipFromHeader *)(hdr->pHeader),\
						 pGeneralHdr->pFromHeader, err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeRecordRoute:
			if (sip_listGetAt(&(pGeneralHdr->slRecordRouteHdr), index, \
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRecordRouteHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipRecordRouteHeader ((SipRecordRouteHeader *)\
				(hdr->pHeader), (SipRecordRouteHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeTimestamp:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pTimeStampHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					(hdr)->pHeader = (SIP_Pvoid) pGeneralHdr->pTimeStampHdr;
					HSS_LOCKEDINCREF(((SipTimeStampHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipTimeStampHeader( (SipTimeStampHeader *)\
						(hdr->pHeader), pGeneralHdr->pTimeStampHdr, err)\
						 == SipFail)
						return SipFail;
#endif
				}
			}
			break;
#ifdef SIP_3GPP
		case SipHdrTypePath:
			if (sip_listGetAt(&(pGeneralHdr->slPathHdr), index, \
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipPathHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipPathHeader ((SipPathHeader *)\
				(hdr->pHeader), (SipPathHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeServiceRoute:
			{
					if (sip_listGetAt(&(pGeneralHdr->slServiceRouteHdr), index, \
											&temp, err) == SipFail)
							return SipFail;
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) temp;
					HSS_LOCKEDINCREF(((SipServiceRouteHeader *) \
											((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipServiceRouteHeader ((SipServiceRouteHeader *)\
											(hdr->pHeader),
											(SipServiceRouteHeader *)temp, err) == SipFail)
							return SipFail;
#endif
					break;
			}

		case SipHdrTypePanInfo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPanInfoHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pPanInfoHdr;
					HSS_LOCKEDINCREF(((SipPanInfoHeader *) ((hdr)->pHeader))->\
					dRefCount);
#else
					if (__sip_cloneSipPanInfoHeader((SipPanInfoHeader *)(hdr->\
						pHeader),\
						 pGeneralHdr->pPanInfoHdr, err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
    case SipHdrTypePcVector:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPcVectorHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
				 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pPcVectorHdr;
					HSS_LOCKEDINCREF(((SipPcVectorHeader *) ((hdr)->pHeader))\
					->dRefCount);
#else
					if (__sip_cloneSipPcVectorHeader((SipPcVectorHeader *)(hdr\
						->pHeader),\
						 pGeneralHdr->pPcVectorHdr, err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
	
#endif
		case SipHdrTypeTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pToHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pToHdr;
					HSS_LOCKEDINCREF(((SipToHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipToHeader( (SipToHeader *)(hdr->pHeader),\
						 pGeneralHdr->pToHdr, err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeVia:
			if (sip_listGetAt(&(pGeneralHdr->slViaHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid)  temp;
			HSS_LOCKEDINCREF(((SipViaHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipViaHeader ((SipViaHeader *)(hdr->pHeader),\
				(SipViaHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId:
			if (sip_listGetAt(&(pGeneralHdr->slPAssertIdHdr), index, &temp, \
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid)  temp;
		    HSS_LOCKEDINCREF((\
				(SipPAssertIdHeader*)((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipPAssertIdHeader ((SipPAssertIdHeader *)\
					(hdr->pHeader),(SipPAssertIdHeader *)temp, err) \
					== SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypePPreferredId:
			if (sip_listGetAt(&(pGeneralHdr->slPPreferredIdHdr), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid)  temp;
		    HSS_LOCKEDINCREF((\
				(SipPPreferredIdHeader*)((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipPPreferredIdHeader ((SipPPreferredIdHeader *)\
					(hdr->pHeader),(SipPPreferredIdHeader *)temp, err) \
					== SipFail)
				return SipFail;
#endif
			break ;		

		case SipHdrTypePrivacy:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPrivacyHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pPrivacyHdr;
					HSS_LOCKEDINCREF(((SipPrivacyHeader *)\
									 ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipPrivacyHeader( (SipPrivacyHeader *)\
						(hdr->pHeader),pGeneralHdr->pPrivacyHdr,err) == SipFail)
						return SipFail;
#endif
				}
			}
			break;
#endif /* #ifdef SIP_PRIVACY */

		case SipHdrTypeContentEncoding:
			if (sip_listGetAt(&(pGeneralHdr->slContentEncodingHdr), index,\
				 &temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid)  temp;
			HSS_LOCKEDINCREF(((SipContentEncodingHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipContentEncodingHeader ((SipContentEncodingHeader *)\
				(hdr->pHeader), (SipContentEncodingHeader *)temp, err) == \
				SipFail)
				return SipFail;
#endif
			break;
	/* RPR */
		case SipHdrTypeSupported:
			if (sip_listGetAt(&(pGeneralHdr->slSupportedHdr), index, &temp,\
				 err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipSupportedHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipSupportedHeader ((SipSupportedHeader *)\
				(hdr->pHeader), (SipSupportedHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeContentLength:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pContentLengthHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pContentLengthHdr;
					HSS_LOCKEDINCREF(((SipContentLengthHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipContentLengthHeader( \
						(SipContentLengthHeader *)(hdr->pHeader),\
						pGeneralHdr->pContentLengthHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		case SipHdrTypeContentType:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pContentTypeHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pContentTypeHdr;
					HSS_LOCKEDINCREF(((SipContentTypeHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipContentTypeHeader( (SipContentTypeHeader *)\
						(hdr->pHeader), pGeneralHdr->pContentTypeHdr, err) \
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;
	/* bcpt ext */
		case SipHdrTypeMimeVersion:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
				if (pGeneralHdr->pMimeVersionHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pMimeVersionHdr;
					HSS_LOCKEDINCREF(((SipMimeVersionHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_bcpt_cloneSipMimeVersionHeader( \
						(SipMimeVersionHeader *)(hdr->pHeader), \
						pGeneralHdr->pMimeVersionHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
/* bcpt ext ends */
		case SipHdrTypeOrganization:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pOrganizationHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pOrganizationHdr;
					HSS_LOCKEDINCREF(((SipOrganizationHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipOrganizationHeader(\
						(SipOrganizationHeader *)(hdr->pHeader),\
						pGeneralHdr->pOrganizationHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		case SipHdrTypeContactAny:
			if (sip_listGetAt(&(pGeneralHdr->slContactHdr), index, &temp, \
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipContactHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipContactHeader ((SipContactHeader *)(hdr->pHeader),\
				(SipContactHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeRequire:
			if (sip_listGetAt(&(pGeneralHdr->slRequireHdr), index, &temp, \
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRequireHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipRequireHeader ((SipRequireHeader *)(hdr->pHeader),\
				(SipRequireHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeCallInfo:
			if (sip_listGetAt(&(pGeneralHdr->slCallInfoHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipCallInfoHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipCallInfoHeader ((SipCallInfoHeader *) (hdr->pHeader),\
			 (SipCallInfoHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeUserAgent:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pUserAgentHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pUserAgentHdr;
					HSS_LOCKEDINCREF(((SipUserAgentHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipUserAgentHeader( (SipUserAgentHeader *)\
						(hdr->pHeader), pGeneralHdr->pUserAgentHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
	case SipHdrTypeRetryAfterAny:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pRetryAfterHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pRetryAfterHdr;
					HSS_LOCKEDINCREF(((SipRetryAfterHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipRetryAfterHeader( (SipRetryAfterHeader *)\
						(hdr->pHeader), pGeneralHdr->pRetryAfterHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeUnknown:
			if (sip_listGetAt(&(pGeneralHdr->slUnknownHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			(hdr)->pHeader = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipUnknownHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipUnknownHeader ((SipUnknownHeader *)\
				(hdr->pHeader), (SipUnknownHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
			if (sip_listGetAt(&(pGeneralHdr->slAllowEventsHdr), index, \
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAllowEventsHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_impp_cloneSipAllowEventsHeader ((SipAllowEventsHeader *)\
				(hdr->pHeader),(SipAllowEventsHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
#endif

#ifdef SIP_3GPP
	    /* PcfAddr header */
        case SipHdrTypePcfAddr:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPcfAddrHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pGeneralHdr->pPcfAddrHdr;
					HSS_LOCKEDINCREF(((SipPcfAddrHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipPcfAddrHeader((SipPcfAddrHeader *)(hdr->pHeader),\
						pGeneralHdr->pPcfAddrHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;/* for PcfAddr header */
#endif


		default:
#ifdef SIP_DCS
			if (sip_dcs_getDcsGeneralHeaderAtIndex (pGeneralHdr, hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getGeneralHeaderAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  getRequestHeaderAtIndex
**
** DESCRIPTION:  This function returns the request pHeader of
**	en_HeaderType "dType" at the index specified by "index".
**
******************************************************************/
SipBool getRequestHeaderAtIndex
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr, en_HeaderType dType, SipHeader *hdr,\
	 SIP_U32bit index, SipError *err)
#else
	(pRequestHdr, dType, hdr, index, err)
	SipReqHeader *pRequestHdr;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIP_Pvoid temp;
	SIPDEBUGFN("Entering function getRequestHeaderAtIndex");
	switch(dType)
	{
		case SipHdrTypeAuthorization:
			if (sip_listGetAt(&(pRequestHdr->slAuthorizationHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAuthorizationHeader (\
				(SipAuthorizationHeader *)\
				(hdr->pHeader), (SipAuthorizationHeader *)temp, err)\
				== SipFail)
				return SipFail;
#endif
			 break;

/* call-transfer */
		case SipHdrTypeReferTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReferToHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid)  pRequestHdr->pReferToHdr;
					HSS_LOCKEDINCREF(((SipReferToHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipReferToHeader(\
						(SipReferToHeader *)(hdr->pHeader),\
						pRequestHdr->pReferToHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;

		case SipHdrTypeReferredBy:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReferredByHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid)  pRequestHdr->pReferredByHdr;
					HSS_LOCKEDINCREF(((SipReferredByHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipReferredByHeader(\
						(SipReferredByHeader *)(hdr->pHeader),\
						pRequestHdr->pReferredByHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;	/* call-transfer */

		case SipHdrTypeHide:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pHideHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pHideHdr;
					HSS_LOCKEDINCREF(((SipHideHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipHideHeader((SipHideHeader *)(hdr->pHeader),\
						pRequestHdr->pHideHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;

	case SipHdrTypeReplaces:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReplacesHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pReplacesHdr;
					HSS_LOCKEDINCREF(((SipReplacesHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipReplacesHeader((SipReplacesHeader *)(hdr->pHeader),\
						pRequestHdr->pReplacesHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;

		case SipHdrTypeMaxforwards:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pMaxForwardsHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pMaxForwardsHdr;
					HSS_LOCKEDINCREF(((SipMaxForwardsHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipMaxForwardsHeader( (SipMaxForwardsHeader *)\
						(hdr->pHeader), pRequestHdr->pMaxForwardsHdr, \
						err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		case SipHdrTypePriority:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pPriorityHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pPriorityHdr;
					HSS_LOCKEDINCREF(((SipPriorityHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipPriorityHeader( (SipPriorityHeader *)\
						(hdr->pHeader), pRequestHdr->pPriorityHdr, err) \
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeProxyauthorization:
			if (sip_listGetAt(&(pRequestHdr->slProxyAuthorizationHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipProxyAuthorizationHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipProxyAuthorizationHeader (\
				(SipProxyAuthorizationHeader *)\
				(hdr->pHeader), (SipProxyAuthorizationHeader *)temp, err)\
				== SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeProxyRequire:
			if (sip_listGetAt(&(pRequestHdr->slProxyRequireHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipProxyRequireHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipProxyRequireHeader ((SipProxyRequireHeader *)\
				(hdr->pHeader), (SipProxyRequireHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeRoute:
			if (sip_listGetAt(&(pRequestHdr->slRouteHdr), index, &temp, err)\
				== SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRouteHeader *) ((hdr)->pHeader))->dRefCount);
#else
			 if (__sip_cloneSipRouteHeader ((SipRouteHeader *)(hdr->pHeader),\
				(SipRouteHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			 break;
		case SipHdrTypeAlso:
			if (sip_listGetAt(&(pRequestHdr->slAlsoHdr), index, &temp, err)\
				== SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAlsoHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAlsoHeader ((SipAlsoHeader *)(hdr->pHeader),\
				(SipAlsoHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeResponseKey:
			if (index > 0)
			 {
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
				if (pRequestHdr->pRespKeyHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pRespKeyHdr;
					HSS_LOCKEDINCREF(((SipRespKeyHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipRespKeyHeader( (SipRespKeyHeader *)\
						(hdr->pHeader), pRequestHdr->pRespKeyHdr, err) ==\
						SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		case SipHdrTypeSubject:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pSubjectHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pSubjectHdr;
					HSS_LOCKEDINCREF(((SipSubjectHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipSubjectHeader( (SipSubjectHeader *)\
						(hdr->pHeader), pRequestHdr->pSubjectHdr, err) \
						== SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
	/* Retrans */
		case SipHdrTypeRAck:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pRackHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pRackHdr;
					HSS_LOCKEDINCREF(((SipRackHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_rpr_cloneSipRAckHeader( (SipRackHeader *)\
						(hdr->pHeader), pRequestHdr->pRackHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		/* Retrans ends */
		case SipHdrTypeWwwAuthenticate:
			if (sip_listGetAt(&(pRequestHdr->slWwwAuthenticateHdr), index, \
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipWwwAuthenticateHeader ((SipWwwAuthenticateHeader *)\
				(hdr->pHeader), (SipWwwAuthenticateHeader *)temp, err) \
				== SipFail)
				return SipFail;
#endif
			break;
#ifdef SIP_CCP
		case SipHdrTypeAcceptContact:
			if (sip_listGetAt(&(pRequestHdr->slAcceptContactHdr), index, \
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAcceptContactHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_ccp_cloneSipAcceptContactHeader ((SipAcceptContactHeader *)\
				(hdr->pHeader), (SipAcceptContactHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break; /* CCP case */
		case SipHdrTypeRejectContact:
			if (sip_listGetAt(&(pRequestHdr->slRejectContactHdr), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRejectContactHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_ccp_cloneSipRejectContactHeader ((SipRejectContactHeader *)\
				(hdr->pHeader), (SipRejectContactHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break; /* CCP case */
		case SipHdrTypeRequestDisposition:
			if (sip_listGetAt(&(pRequestHdr->slRequestDispositionHdr), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRequestDispositionHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_ccp_cloneSipRequestDispositionHeader ((SipRequestDispositionHeader *)\
				(hdr->pHeader), (SipRequestDispositionHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break; /* CCP case */
			
#endif /* of CCP */

		case SipHdrTypeAlertInfo:
			if (sip_listGetAt(&(pRequestHdr->slAlertInfoHdr), index, &temp,\
				 err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAlertInfoHeader *) ((hdr)->pHeader))->dRefCount);
#else
			 if (__sip_cloneSipAlertInfoHeader ((SipAlertInfoHeader *)\
				(hdr->pHeader), (SipAlertInfoHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			 break;

		case SipHdrTypeInReplyTo:
			if (sip_listGetAt(&(pRequestHdr->slInReplyToHdr), index, &temp,\
				 err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipInReplyToHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipInReplyToHeader ((SipInReplyToHeader *)\
				(hdr->pHeader), (SipInReplyToHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			 break;

#ifdef SIP_IMPP
		case SipHdrTypeEvent:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pEventHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pEventHdr;
					HSS_LOCKEDINCREF(((SipEventHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_impp_cloneSipEventHeader( (SipEventHeader *)\
						(hdr->pHeader), pRequestHdr->pEventHdr, err) \
						== SipFail)
						return SipFail;
#endif
				}
			}
			break;
		case SipHdrTypeSubscriptionState:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pSubscriptionStateHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid)  pRequestHdr-> \
					 	pSubscriptionStateHdr;
					HSS_LOCKEDINCREF(((SipSubscriptionStateHeader *) \
						((hdr)->pHeader))->dRefCount);
#else
					if (__sip_impp_cloneSipSubscriptionStateHeader(\
						(SipSubscriptionStateHeader *)(hdr->pHeader),\
						pRequestHdr->pSubscriptionStateHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
#endif	/* of IMPP */

#ifdef SIP_CONF             
	    /* Join header */
        case SipHdrTypeJoin:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pJoinHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pJoinHdr;
					HSS_LOCKEDINCREF(((SipJoinHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipJoinHeader((SipJoinHeader *)(hdr->pHeader),\
						pRequestHdr->pJoinHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;/* for Join header */
#endif

#ifdef SIP_3GPP
	    /* PCalledPartyId header */
        case SipHdrTypePCalledPartyId:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pPCalledPartyIdHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pRequestHdr->pPCalledPartyIdHdr;
					HSS_LOCKEDINCREF(((SipPCalledPartyIdHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipPCalledPartyIdHeader((SipPCalledPartyIdHeader *)(hdr->pHeader),\
						pRequestHdr->pPCalledPartyIdHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;/* for PCalledPartyId header */

	    /* PVisitednetworkId header */
        case SipHdrTypePVisitedNetworkId:
			if (sip_listGetAt(&(pRequestHdr->slPVisitedNetworkIdHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipPVisitedNetworkIdHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipPVisitedNetworkIdHeader (\
				(SipPVisitedNetworkIdHeader *)(hdr->pHeader),\
				(SipPVisitedNetworkIdHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
             
#endif /* #ifdef SIP_3GPP */

#ifdef SIP_SECURITY
        case SipHdrTypeSecurityClient:
                       if (sip_listGetAt(&(pRequestHdr->slSecurityClientHdr), index, &temp, err)\
                                  == SipFail)
                               return SipFail;
#ifdef SIP_BY_REFERENCE
                        ((hdr)->pHeader) = (SIP_Pvoid) temp;
                         HSS_LOCKEDINCREF(((SipSecurityClientHeader *) ((hdr)->pHeader))->dRefCount);
#else
                          if (__sip_cloneSipSecurityClientHeader ((SipSecurityClientHeader *)(hdr->pHeader),\
                                 (SipSecurityClientHeader *)temp, err) == SipFail)
                                  return SipFail;
#endif
                         break;/* for Security-Client header */

	case SipHdrTypeSecurityVerify:
                       if (sip_listGetAt(&(pRequestHdr->slSecurityVerifyHdr), index, &temp, err)\
                                  == SipFail)
                               return SipFail;
#ifdef SIP_BY_REFERENCE
                        ((hdr)->pHeader) = (SIP_Pvoid) temp;
                         HSS_LOCKEDINCREF(((SipSecurityVerifyHeader *) ((hdr)->pHeader))->dRefCount);
#else
                          if (__sip_cloneSipSecurityVerifyHeader ((SipSecurityVerifyHeader *)(hdr->pHeader),\
                                 (SipSecurityVerifyHeader *)temp, err) == SipFail)
                                  return SipFail;
#endif
                         break;/* for Security-Verify header */
#endif /* end of #ifdef SIP_SECURITY */

#ifdef SIP_CONGEST
		case SipHdrTypeRsrcPriority:
			if (sip_listGetAt(&(pRequestHdr->slRsrcPriorityHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipRsrcPriorityHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipRsrcPriorityHeader (\
				(SipRsrcPriorityHeader *)\
				(hdr->pHeader), (SipRsrcPriorityHeader *)temp, err)\
				== SipFail)
				return SipFail;
#endif
			 break;

#endif

		default:

#ifdef SIP_DCS
			if (sip_dcs_getDcsRequestHeaderAtIndex (pRequestHdr, hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;

	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getRequestHeaderAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  getResponseHeaderAtIndex
**
** DESCRIPTION:  This function returns the response pHeader of
**	en_HeaderType "dType" at the index specified by "index".
**
******************************************************************/
SipBool getResponseHeaderAtIndex
#ifdef ANSI_PROTO
#ifdef SIP_BY_REFERENCE
	(SipRespHeader *pResponseHdr, en_HeaderType dType, SipHeader *hdr,\
	 SIP_U32bit index, SipError *err)
#else
	(SipRespHeader *pResponseHdr, en_HeaderType dType, SipHeader *hdr,\
	 SIP_U32bit index, SipError *err)
#endif
#else
#ifdef SIP_BY_REFERENCE
	(pResponseHdr, dType, hdr, index, err)
	SipRespHeader *pResponseHdr;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#else
	(pResponseHdr, dType, hdr, index, err)
	SipRespHeader *pResponseHdr;
	en_HeaderType dType;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
#endif
{
	SIP_Pvoid temp;
	SIPDEBUGFN("Entering function getResponseHeaderAtIndex");
	switch(dType)
	{
		case SipHdrTypeProxyAuthenticate:
			if (sip_listGetAt(&(pResponseHdr->slProxyAuthenticateHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipProxyAuthenticateHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipProxyAuthenticateHeader (\
				(SipProxyAuthenticateHeader *)(hdr->pHeader),\
				(SipProxyAuthenticateHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeAuthenticationInfo:
			if (sip_listGetAt(&(pResponseHdr->slAuthenticationInfoHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAuthenticationInfoHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAuthenticationInfoHeader (\
				(SipAuthenticationInfoHeader *)(hdr->pHeader),\
				(SipAuthenticationInfoHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeMinExpires:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pMinExpiresHdr== SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pResponseHdr->\
					 pMinExpiresHdr;
					HSS_LOCKEDINCREF(((SipMinExpiresHeader *) ((hdr)->pHeader))\
					->dRefCount);
#else
					if (__sip_cloneSipMinExpiresHeader( (SipMinExpiresHeader *)\
						(hdr->pHeader), pResponseHdr->pMinExpiresHdr, err)\
						== SipFail)
						return SipFail;
#endif
				}
			}
			break; 
		/* Retrans */
		case SipHdrTypeRSeq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pRSeqHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					 ((hdr)->pHeader) = (SIP_Pvoid) pResponseHdr->pRSeqHdr;
					HSS_LOCKEDINCREF(((SipRseqHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_rpr_cloneSipRSeqHeader( (SipRseqHeader *)\
						(hdr->pHeader), pResponseHdr->pRSeqHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		/* Retrans ends */
		case SipHdrTypeServer:
			/* if (sip_listGetAt(&(pResponseHdr->slServerHdr), index, &temp, err)\
				 == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			(hdr)->pHeader = (SIP_Pvoid)temp;
#else
			 temp1 = (SIP_S8bit *)STRDUPACCESSOR((SIP_S8bit *)temp);
			 if (temp1 == SIP_NULL)
			 {
				*err = E_NO_MEM;
				return SipFail;
			 }
			 hdr->pHeader = (SIP_Pvoid)temp1;
#endif	*/
/* 			if (sip_listGetAt(&(pResponseHdr->slServerHeader), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			(SipServerHeader *) ((hdr)->pHeader) = (SipServerHeader *)temp;
			HSS_LOCKEDINCREF(((SipServerHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipServerHeader ((SipServerHeader *)(hdr->pHeader),\
				(SipServerHeader *)temp, err) == SipFail)
				return SipFail;
#endif */
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pServerHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
#ifdef SIP_BY_REFERENCE
					((hdr)->pHeader) = (SIP_Pvoid) pResponseHdr->pServerHdr;
					HSS_LOCKEDINCREF(((SipServerHeader *) ((hdr)->pHeader))->dRefCount);
#else
					if (__sip_cloneSipServerHeader( (SipServerHeader *)\
						(hdr->pHeader), pResponseHdr->pServerHdr, err) == SipFail)
						return SipFail;
#endif
				}
			 }
			 break;
		case SipHdrTypeUnsupported:
			/* if (sip_listGetAt(&(pResponseHdr->slUnsupportedHdr), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			(hdr)->pHeader = (SIP_Pvoid)temp;
#else
			temp1 = (SIP_S8bit *)STRDUPACCESSOR((SIP_S8bit *)temp);
			if (temp1 == SIP_NULL)
			{
				*err = E_NO_MEM;
				return SipFail;
			}
			hdr->pHeader = (SIP_Pvoid)temp1;
#endif */
			if (sip_listGetAt(&(pResponseHdr->slUnsupportedHdr), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipUnsupportedHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipUnsupportedHeader ((SipUnsupportedHeader *)(hdr->pHeader),\
				(SipUnsupportedHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;

		case SipHdrTypeWarning:
			if (sip_listGetAt(&(pResponseHdr->slWarningHeader), index, &temp,\
				err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipWarningHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipWarningHeader ((SipWarningHeader *)(hdr->pHeader),\
				(SipWarningHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
		case SipHdrTypeWwwAuthenticate:
			if (sip_listGetAt(&(pResponseHdr->slWwwAuthenticateHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipWwwAuthenticateHeader ((SipWwwAuthenticateHeader *)\
				(hdr->pHeader), (SipWwwAuthenticateHeader *)temp, err)\
				== SipFail)
			return SipFail;
#endif
			break;
		case SipHdrTypeAuthorization:
			if (sip_listGetAt(&(pResponseHdr->slAuthorizationHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			 ((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAuthorizationHeader (\
				(SipAuthorizationHeader *)\
				(hdr->pHeader), (SipAuthorizationHeader *)temp, err)\
				== SipFail)
				return SipFail;
#endif
			 break;
		case SipHdrTypeErrorInfo:
			if (sip_listGetAt(&(pResponseHdr->slErrorInfoHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipErrorInfoHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipErrorInfoHeader (\
				(SipErrorInfoHeader *)(hdr->pHeader),\
				(SipErrorInfoHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
#ifdef SIP_3GPP
        case SipHdrTypePAssociatedUri:
			if (sip_listGetAt(&(pResponseHdr->slPAssociatedUriHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipPAssociatedUriHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipPAssociatedUriHeader (\
				(SipPAssociatedUriHeader *)(hdr->pHeader),\
				(SipPAssociatedUriHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
#endif

#ifdef SIP_CONGEST
        case SipHdrTypeAcceptRsrcPriority:
			if (sip_listGetAt(&(pResponseHdr->slAcceptRsrcPriorityHdr), index,\
				&temp, err) == SipFail)
				return SipFail;
#ifdef SIP_BY_REFERENCE
			((hdr)->pHeader) = (SIP_Pvoid) temp;
			HSS_LOCKEDINCREF(((SipAcceptRsrcPriorityHeader *) ((hdr)->pHeader))->dRefCount);
#else
			if (__sip_cloneSipAcceptRsrcPriorityHeader (\
				(SipAcceptRsrcPriorityHeader *)(hdr->pHeader),\
				(SipAcceptRsrcPriorityHeader *)temp, err) == SipFail)
				return SipFail;
#endif
			break;
#endif

#ifdef SIP_SECURITY
	case SipHdrTypeSecurityServer:
                       if (sip_listGetAt(&(pResponseHdr->slSecurityServerHdr), index, &temp, err)\
                                  == SipFail)
                               return SipFail;
#ifdef SIP_BY_REFERENCE
                        ((hdr)->pHeader) = (SIP_Pvoid) temp;
                         HSS_LOCKEDINCREF(((SipSecurityServerHeader *) ((hdr)->pHeader))->dRefCount);
#else
                          if (__sip_cloneSipSecurityServerHeader ((SipSecurityServerHeader *)(hdr->pHeader),\
                                 (SipSecurityServerHeader *)temp, err) == SipFail)
                                  return SipFail;
#endif
                         break;/* for Security-Server header */
#endif /*end of #ifdef SIP_SECURITY */            
		default:
#ifdef SIP_DCS
			if (sip_dcs_getDcsResponseHeaderAtIndex (pResponseHdr, hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function getResponseHeaderAtIndex");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  setGeneralHeaderAtIndex
**
** DESCRIPTION: This function sets a general pHeader "hdr" at the
**	index specified by "index".
**
******************************************************************/
SipBool setGeneralHeaderAtIndex
#ifdef ANSI_PROTO
	(SipGeneralHeader *pGeneralHdr, SipHeader *hdr, SIP_U32bit index,\
	 SipError *err)
#else
	(pGeneralHdr, hdr, index, err)
	SipGeneralHeader *pGeneralHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAcceptHeader *accep;
	SipAllowHeader *allow;
#ifdef SIP_IMPP
	SipAllowEventsHeader *allowevents;
#endif
	SipContentDispositionHeader *contentdisposition;
	SipReasonHeader *pReason;
	SipContentLanguageHeader *contentlang;
	SipAcceptEncodingHeader *accep_enc;
	SipAcceptLangHeader *accep_lang;
	SipRecordRouteHeader *record;
	SipViaHeader *via;
#ifdef SIP_PRIVACY
	SipPAssertIdHeader *pPAssertId =SIP_NULL ;
	SipPPreferredIdHeader *pPPreferredId =SIP_NULL ;
#endif 
	SipContentEncodingHeader *pEncoding;
	SipUnknownHeader *unknown;
	SipContactHeader *contact;
	SipSupportedHeader * pSupp;
	SipRequireHeader *require;
#ifdef SIP_3GPP
	SipPathHeader *pPath;
	SipServiceRouteHeader *pService=SIP_NULL;
#endif
	SipCallInfoHeader *callinfo;
#endif
	en_SipBoolean z;
	SIPDEBUGFN("Entering function setGeneralHeaderAtIndex");
	z = SipFalse;
	switch(hdr->dType)
	{
		case SipHdrTypeAllow:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slAllowHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAllowHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAllowHeader (&allow, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAllowHeader(allow, (SipAllowHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAllowHeader(allow);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slAllowHdr),index,\
				(SIP_Pvoid)allow, err) == SipFail)
			{
				sip_freeSipAllowHeader(allow);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeContentDisposition:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slContentDispositionHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentDispositionHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentDispositionHeader (&contentdisposition, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentDispositionHeader(contentdisposition, (SipContentDispositionHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentDispositionHeader(contentdisposition);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slContentDispositionHdr),index,\
				(SIP_Pvoid)contentdisposition, err) == SipFail)
			{
				sip_freeSipContentDispositionHeader(contentdisposition);
				return SipFail;
			}
#endif
			break;
			
		case SipHdrTypeReason:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slReasonHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipReasonHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipReasonHeader (&pReason, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipReasonHeader(pReason, (SipReasonHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipReasonHeader(pReason);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slReasonHdr),index,\
				(SIP_Pvoid)pReason, err) == SipFail)
			{
				sip_freeSipReasonHeader(pReason);
				return SipFail;
			}
#endif
			break;
			
		case SipHdrTypeContentLanguage:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slContentLanguageHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentLanguageHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentLanguageHeader (&contentlang, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentLanguageHeader(contentlang, (SipContentLanguageHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentLanguageHeader(contentlang);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slContentLanguageHdr),index,\
				(SIP_Pvoid)contentlang, err) == SipFail)
			{
				sip_freeSipContentLanguageHeader(contentlang);
				return SipFail;
			}
#endif
			break;



		case SipHdrTypeAccept:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slAcceptHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptHeader (&accep, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptHeader(accep, (SipAcceptHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptHeader(accep);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slAcceptHdr),index, \
				(SIP_Pvoid)accep, err) == SipFail)
			{
				sip_freeSipAcceptHeader(accep);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypeAcceptEncoding:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slAcceptEncoding),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptEncodingHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptEncodingHeader (&accep_enc, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptEncodingHeader(accep_enc, \
				(SipAcceptEncodingHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptEncodingHeader(accep_enc);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slAcceptEncoding),index, \
				(SIP_Pvoid)accep_enc, err) == SipFail)
			{
				sip_freeSipAcceptEncodingHeader(accep_enc);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAcceptLanguage:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slAcceptLang),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptLangHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptLangHeader (&accep_lang, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptLangHeader(accep_lang, \
				(SipAcceptLangHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptLangHeader(accep_lang);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slAcceptLang),index, \
				(SIP_Pvoid)accep_lang, err) == SipFail)
			{
				sip_freeSipAcceptLangHeader(accep_lang);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeCallId:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pCallidHdr != SIP_NULL)
					sip_freeSipCallIdHeader(pGeneralHdr->pCallidHdr);
				pGeneralHdr->pCallidHdr=(SipCallIdHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipCallIdHeader *) (hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pCallidHdr == SIP_NULL)
				{
					if (sip_initSipCallIdHeader(&(pGeneralHdr->pCallidHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipCallIdHeader(pGeneralHdr->pCallidHdr, \
					(SipCallIdHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipCallIdHeader(pGeneralHdr->pCallidHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeCseq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pCseqHdr!=SIP_NULL)
					sip_freeSipCseqHeader(pGeneralHdr->pCseqHdr);
				pGeneralHdr->pCseqHdr=(SipCseqHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipCseqHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pCseqHdr == SIP_NULL)
				{
					if (sip_initSipCseqHeader(&(pGeneralHdr->pCseqHdr), \
					err) == SipFail)
					return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipCseqHeader(pGeneralHdr->pCseqHdr, \
					(SipCseqHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipCseqHeader(pGeneralHdr->pCseqHdr);
					return SipFail;
				}
#endif
			}
			break;
		case SipHdrTypeDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pDateHdr!=SIP_NULL)
					sip_freeSipDateHeader(pGeneralHdr->pDateHdr);
				pGeneralHdr->pDateHdr=(SipDateHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipDateHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pDateHdr == SIP_NULL)
				{
					if (sip_initSipDateHeader(&(pGeneralHdr->pDateHdr), err)\
						== SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipDateHeader(pGeneralHdr->pDateHdr, \
					(SipDateHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipDateHeader(pGeneralHdr->pDateHdr);
					return SipFail;
				}
#endif
			}
			break;
		case SipHdrTypeEncryption:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pEncryptionHdr != SIP_NULL)
					sip_freeSipEncryptionHeader(pGeneralHdr->pEncryptionHdr);
				pGeneralHdr->pEncryptionHdr=(SipEncryptionHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipEncryptionHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pEncryptionHdr == SIP_NULL)
				{
					if (sip_initSipEncryptionHeader(&(pGeneralHdr->\
						pEncryptionHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
			 	if (__sip_cloneSipEncryptionHeader(pGeneralHdr->pEncryptionHdr,\
					(SipEncryptionHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipEncryptionHeader(\
							pGeneralHdr->pEncryptionHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeReplyTo:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pReplyToHdr != SIP_NULL)
					sip_freeSipReplyToHeader(pGeneralHdr->pReplyToHdr);
				pGeneralHdr->pReplyToHdr=(SipReplyToHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipReplyToHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pReplyToHdr == SIP_NULL)
				{
					if (sip_initSipReplyToHeader(&(pGeneralHdr->\
						pReplyToHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipReplyToHeader(pGeneralHdr->pReplyToHdr,\
					(SipReplyToHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipReplyToHeader(\
							pGeneralHdr->pReplyToHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeExpiresDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pExpiresHdr!=SIP_NULL)
					sip_freeSipExpiresHeader(pGeneralHdr->pExpiresHdr);
				pGeneralHdr->pExpiresHdr=(SipExpiresHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipExpiresHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pExpiresHdr == SIP_NULL)
				{
					if (sip_initSipExpiresHeader(&(pGeneralHdr->pExpiresHdr),\
						SipExpDate, err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipExpiresHeader(pGeneralHdr->pExpiresHdr,\
					(SipExpiresHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipExpiresHeader (pGeneralHdr->pExpiresHdr);
					return SipFail;
				}
#endif
			}
		 	break;
		case SipHdrTypeExpiresSec:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pExpiresHdr!=SIP_NULL)
					sip_freeSipExpiresHeader(pGeneralHdr->pExpiresHdr);
				pGeneralHdr->pExpiresHdr=(SipExpiresHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipExpiresHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pExpiresHdr == SIP_NULL)
				{
					if (sip_initSipExpiresHeader(&(pGeneralHdr->pExpiresHdr),\
						SipExpSeconds, err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipExpiresHeader(pGeneralHdr->pExpiresHdr,\
					(SipExpiresHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipExpiresHeader(pGeneralHdr->pExpiresHdr);
					return SipFail;
				}
#endif
			 }
			 break;

#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pMinSEHdr != SIP_NULL)
					sip_freeSipMinSEHeader(pGeneralHdr->pMinSEHdr);
				pGeneralHdr->pMinSEHdr=(SipMinSEHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipMinSEHeader *) (hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pMinSEHdr == SIP_NULL)
				{
					if (sip_initSipMinSEHeader(&(pGeneralHdr->pMinSEHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipMinSEHeader(pGeneralHdr->pMinSEHdr, \
					(SipMinSEHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipMinSEHeader(pGeneralHdr->pMinSEHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeSessionExpires:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			 }
			 else
			 {
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pSessionExpiresHdr!=SIP_NULL)
					sip_freeSipSessionExpiresHeader(pGeneralHdr->pSessionExpiresHdr);
				pGeneralHdr->pSessionExpiresHdr=(SipSessionExpiresHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipSessionExpiresHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pSessionExpiresHdr == SIP_NULL)
				{
					if (sip_initSipSessionExpiresHeader(&(pGeneralHdr->pSessionExpiresHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipSessionExpiresHeader(pGeneralHdr->pSessionExpiresHdr,\
					(SipSessionExpiresHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipSessionExpiresHeader(pGeneralHdr->pSessionExpiresHdr);
					return SipFail;
				}
#endif
			 }
			 break;
#endif

		case SipHdrTypeFrom:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pFromHeader!=SIP_NULL)
					sip_freeSipFromHeader(pGeneralHdr->pFromHeader);
				pGeneralHdr->pFromHeader=(SipFromHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipFromHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pFromHeader == SIP_NULL)
				{
					if (sip_initSipFromHeader(&(pGeneralHdr->pFromHeader),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipFromHeader(pGeneralHdr->pFromHeader, \
					(SipFromHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipFromHeader(pGeneralHdr->pFromHeader);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeRecordRoute:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slRecordRouteHdr),index, \
				(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRecordRouteHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRecordRouteHeader (&record, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRecordRouteHeader(record, \
				(SipRecordRouteHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRecordRouteHeader(record);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slRecordRouteHdr),index, \
				(SIP_Pvoid)record,err) == SipFail)
			{
				sip_freeSipRecordRouteHeader(record);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_3GPP
		case SipHdrTypePath:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slPathHdr),index, \
				(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPathHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPathHeader (&pPath, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPathHeader(pPath, \
				(SipPathHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPathHeader(pPath);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slPathHdr),index, \
				(SIP_Pvoid)pPath,err) == SipFail)
			{
				sip_freeSipPathHeader(pPath);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeServiceRoute:
			{
#ifdef SIP_BY_REFERENCE
					if (sip_listSetAt(&(pGeneralHdr->slServiceRouteHdr),index, \
											(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
							return SipFail;
					HSS_LOCKEDINCREF(((SipServiceRouteHeader*)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipServiceRouteHeader (&pService, err) == SipFail)
							return SipFail;
					if (__sip_cloneSipServiceRouteHeader(pService, \
										(SipServiceRouteHeader *)(hdr->pHeader), err) == SipFail)
					{
							sip_freeSipServiceRouteHeader(pService);
							return SipFail;
					}
					if (sip_listSetAt(&(pGeneralHdr->slServiceRouteHdr),index, \
											(SIP_Pvoid)pService,err) == SipFail)
					{
							sip_freeSipServiceRouteHeader(pService);
							return SipFail;
					}
#endif
					break;
			}


        case SipHdrTypePanInfo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pPanInfoHdr!=SIP_NULL)
					sip_freeSipPanInfoHeader(pGeneralHdr->pPanInfoHdr);
				pGeneralHdr->pPanInfoHdr=(SipPanInfoHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipPanInfoHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pPanInfoHdr == SIP_NULL)
				{
					if (sip_initSipPanInfoHeader(&(pGeneralHdr->pPanInfoHdr)\
						
						,err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPanInfoHeader(pGeneralHdr->pPanInfoHdr, \
					(SipPanInfoHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipPanInfoHeader(pGeneralHdr->pPanInfoHdr);
					return SipFail;
				}
#endif
			 }
			 break;
	    case SipHdrTypePcVector:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pPcVectorHdr!=SIP_NULL)
					sip_freeSipPcVectorHeader(pGeneralHdr->pPcVectorHdr);
				pGeneralHdr->pPcVectorHdr=(SipPcVectorHeader *)(hdr->\
				pHeader);
				HSS_LOCKEDINCREF(((SipPcVectorHeader *) ((hdr)->pHeader))->\
				dRefCount);
#else
				if (pGeneralHdr->pPcVectorHdr == SIP_NULL)
				{
					if (sip_initSipPcVectorHeader(&(pGeneralHdr->pPcVectorHdr)\
						,err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPcVectorHeader(pGeneralHdr->pPcVectorHdr, \
					(SipPcVectorHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipPcVectorHeader(pGeneralHdr->pPcVectorHdr);
					return SipFail;
				}
#endif
			 }
			 break;
 
			
#endif
		case SipHdrTypeTimestamp:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pTimeStampHdr!=SIP_NULL)
					sip_freeSipTimeStampHeader(pGeneralHdr->pTimeStampHdr);
				pGeneralHdr->pTimeStampHdr=(SipTimeStampHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipTimeStampHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pTimeStampHdr == SIP_NULL)
				{
					if (sip_initSipTimeStampHeader\
						(&(pGeneralHdr->pTimeStampHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipTimeStampHeader(pGeneralHdr->pTimeStampHdr,\
					(SipTimeStampHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipTimeStampHeader(pGeneralHdr->pTimeStampHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pToHdr != SIP_NULL)
					sip_freeSipToHeader(pGeneralHdr->pToHdr);
				pGeneralHdr->pToHdr=(SipToHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipToHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pToHdr == SIP_NULL)
				{
					if (sip_initSipToHeader(&(pGeneralHdr->pToHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipToHeader(pGeneralHdr->pToHdr, \
					(SipToHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipToHeader(pGeneralHdr->pToHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeVia:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slViaHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipViaHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipViaHeader (&via, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipViaHeader(via, (SipViaHeader *)(hdr->pHeader),\
				err) == SipFail)
			{
				sip_freeSipViaHeader(via);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slViaHdr),index, \
				(SIP_Pvoid)via, err) == SipFail)
			{
				sip_freeSipViaHeader(via);
				return SipFail;
			}
#endif
			break;

#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slPAssertIdHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPAssertIdHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPAssertIdHeader (&pPAssertId, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPAssertIdHeader(pPAssertId, \
				(SipPAssertIdHeader *)(hdr->pHeader),err) == SipFail)
			{
				sip_freeSipPAssertIdHeader(pPAssertId);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slPAssertIdHdr),index,\
				 (SIP_Pvoid)pPAssertId, err) == SipFail)
			{
				sip_freeSipPAssertIdHeader(pPAssertId);
				return SipFail;
			}

#endif
			break;
		case SipHdrTypePPreferredId:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slPPreferredIdHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPPreferredIdHeader *)
					(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPPreferredIdHeader (&pPPreferredId, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPPreferredIdHeader(pPPreferredId, \
				(SipPPreferredIdHeader *)(hdr->pHeader),err) == SipFail)
			{
				sip_freeSipPPreferredIdHeader(pPPreferredId);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slPPreferredIdHdr),index,\
				 (SIP_Pvoid)pPPreferredId, err) == SipFail)
			{
				sip_freeSipPPreferredIdHeader(pPPreferredId);
				return SipFail;
			}
#endif
			break ;
		case SipHdrTypePrivacy:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pPrivacyHdr != SIP_NULL)
					sip_freeSipPrivacyHeader(pGeneralHdr->pPrivacyHdr);
				pGeneralHdr->pPrivacyHdr=\
										(SipPrivacyHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipPrivacyHeader *)\
											 ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pPrivacyHdr == SIP_NULL)
				{
					if (sip_initSipPrivacyHeader(&(pGeneralHdr->pPrivacyHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPrivacyHeader(pGeneralHdr->pPrivacyHdr, \
					(SipPrivacyHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipPrivacyHeader(pGeneralHdr->pPrivacyHdr);
					return SipFail;
				}
#endif
			 }
			 break;
#endif /* # ifdef SIP_PRIVACY */

		case SipHdrTypeContentEncoding:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slContentEncodingHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentEncodingHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentEncodingHeader (&pEncoding, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentEncodingHeader(pEncoding, \
				(SipContentEncodingHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentEncodingHeader(pEncoding);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slContentEncodingHdr),index,\
				 (SIP_Pvoid)pEncoding, err) == SipFail)
			{
				sip_freeSipContentEncodingHeader(pEncoding);
				return SipFail;
			}
#endif
			break;
	/* RPR */
		case SipHdrTypeSupported:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slSupportedHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipSupportedHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipSupportedHeader (&pSupp, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipSupportedHeader(pSupp, \
				(SipSupportedHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipSupportedHeader(pSupp);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slSupportedHdr),index, \
				(SIP_Pvoid)pSupp, err) == SipFail)
			{
				sip_freeSipSupportedHeader(pSupp);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeRequire:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slRequireHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRequireHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRequireHeader (&require, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRequireHeader(require, (SipRequireHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRequireHeader(require);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slRequireHdr),index,\
				(SIP_Pvoid)require, err) == SipFail)
			{
				sip_freeSipRequireHeader(require);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeContentLength:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pContentLengthHdr!=SIP_NULL)
					sip_freeSipContentLengthHeader(pGeneralHdr->\
						pContentLengthHdr);
				pGeneralHdr->pContentLengthHdr=(SipContentLengthHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipContentLengthHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pContentLengthHdr == SIP_NULL)
				{
					if (sip_initSipContentLengthHeader(&(pGeneralHdr->\
						pContentLengthHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}

				if (__sip_cloneSipContentLengthHeader(pGeneralHdr->\
					pContentLengthHdr, (SipContentLengthHeader *)(hdr->pHeader), err)\
					== SipFail)
				{
					if (z == SipTrue)
						sip_freeSipContentLengthHeader(pGeneralHdr->\
						pContentLengthHdr);
					return SipFail;
				}
#endif
			}
			break;
		case SipHdrTypeContentType:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pContentTypeHdr!=SIP_NULL)
					sip_freeSipContentTypeHeader(pGeneralHdr->pContentTypeHdr);
				pGeneralHdr->pContentTypeHdr=(SipContentTypeHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipContentTypeHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pContentTypeHdr == SIP_NULL)
				{
					if (sip_initSipContentTypeHeader(&(pGeneralHdr->\
						pContentTypeHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if(__sip_cloneSipContentTypeHeader(pGeneralHdr->pContentTypeHdr\
					,(SipContentTypeHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipContentTypeHeader(pGeneralHdr->\
							pContentTypeHdr);
					return SipFail;
				}
#endif
			 }
			 break;
	/* bcpt ext */
		case SipHdrTypeMimeVersion:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pMimeVersionHdr!=SIP_NULL)
					sip_bcpt_freeSipMimeVersionHeader(pGeneralHdr->\
						pMimeVersionHdr);
				pGeneralHdr->pMimeVersionHdr=(SipMimeVersionHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipMimeVersionHeader *) ((hdr)->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pMimeVersionHdr == SIP_NULL)
				{
					if (sip_bcpt_initSipMimeVersionHeader(&(pGeneralHdr->\
						pMimeVersionHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}

				if (__sip_bcpt_cloneSipMimeVersionHeader(pGeneralHdr->\
					pMimeVersionHdr, (SipMimeVersionHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_bcpt_freeSipMimeVersionHeader(pGeneralHdr->\
							pMimeVersionHdr);
					return SipFail;
				}
#endif
			 }
			 break;
	/* bcpt ext ends */
		case SipHdrTypeOrganization:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pOrganizationHdr!=SIP_NULL)
					sip_freeSipOrganizationHeader(pGeneralHdr->\
						pOrganizationHdr);
				pGeneralHdr->pOrganizationHdr=(SipOrganizationHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipOrganizationHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pOrganizationHdr == SIP_NULL)
				{
					if (sip_initSipOrganizationHeader(&(pGeneralHdr->\
						pOrganizationHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipOrganizationHeader(pGeneralHdr->\
					pOrganizationHdr, (SipOrganizationHeader *)(hdr->pHeader),\
					err) == SipFail)
				{
					sip_freeSipOrganizationHeader(\
						pGeneralHdr->pOrganizationHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeContactWildCard:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContactHeader (&contact, SipContactWildCard, err)\
				== SipFail)
				return SipFail;
			if (__sip_cloneSipContactHeader(contact, (SipContactHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)contact, err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeContactNormal:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slContactHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContactHeader (&contact, SipContactNormal, err)\
				== SipFail)
				return SipFail;
			if (__sip_cloneSipContactHeader(contact, (SipContactHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slContactHdr),index, \
				(SIP_Pvoid)contact, err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeCallInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slCallInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipCallInfoHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipCallInfoHeader (&callinfo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipCallInfoHeader(callinfo, (SipCallInfoHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipCallInfoHeader(callinfo);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slCallInfoHdr),index,\
				(SIP_Pvoid)callinfo, err) == SipFail)
			{
				sip_freeSipCallInfoHeader(callinfo);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeUserAgent:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pUserAgentHdr!=SIP_NULL)
					sip_freeSipUserAgentHeader(pGeneralHdr->pUserAgentHdr);
				pGeneralHdr->pUserAgentHdr=(SipUserAgentHeader *) (hdr->pHeader);
				HSS_LOCKEDINCREF(((SipUserAgentHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pUserAgentHdr == SIP_NULL)
				{
					if (sip_initSipUserAgentHeader(\
						&(pGeneralHdr->pUserAgentHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipUserAgentHeader(pGeneralHdr->pUserAgentHdr,\
					(SipUserAgentHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipUserAgentHeader(pGeneralHdr->pUserAgentHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeRetryAfterDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pRetryAfterHdr!=SIP_NULL)
					sip_freeSipRetryAfterHeader(pGeneralHdr->pRetryAfterHdr);
				pGeneralHdr->pRetryAfterHdr=(SipRetryAfterHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipRetryAfterHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pRetryAfterHdr == SIP_NULL)
				{
					if (sip_initSipRetryAfterHeader(\
						&(pGeneralHdr->pRetryAfterHdr), SipExpDate, err)\
						== SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipRetryAfterHeader(pGeneralHdr->pRetryAfterHdr,\
					(SipRetryAfterHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipRetryAfterHeader(\
							pGeneralHdr->pRetryAfterHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeRetryAfterSec:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pRetryAfterHdr!=SIP_NULL)
					sip_freeSipRetryAfterHeader(pGeneralHdr->pRetryAfterHdr);
				pGeneralHdr->pRetryAfterHdr=(SipRetryAfterHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipRetryAfterHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pRetryAfterHdr == SIP_NULL)
				{
					if (sip_initSipRetryAfterHeader(&\
						(pGeneralHdr->pRetryAfterHdr), SipExpSeconds, err)\
						== SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipRetryAfterHeader(pGeneralHdr->pRetryAfterHdr,\
					(SipRetryAfterHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipRetryAfterHeader(\
						pGeneralHdr->pRetryAfterHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeUnknown:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slUnknownHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipUnknownHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipUnknownHeader (&unknown, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipUnknownHeader(unknown, (SipUnknownHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipUnknownHeader(unknown);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slUnknownHdr),index, \
				(SIP_Pvoid)unknown, err) == SipFail)
			{
				sip_freeSipUnknownHeader(unknown);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pGeneralHdr->slAllowEventsHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAllowEventsHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_impp_initSipAllowEventsHeader (&allowevents, err) == SipFail)
				return SipFail;
			if (__sip_impp_cloneSipAllowEventsHeader(allowevents, \
				(SipAllowEventsHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_impp_freeSipAllowEventsHeader(allowevents);
				return SipFail;
			}
			if (sip_listSetAt(&(pGeneralHdr->slAllowEventsHdr),index,\
				(SIP_Pvoid)allowevents, err) == SipFail)
			{
				sip_impp_freeSipAllowEventsHeader(allowevents);
				return SipFail;
			}
#endif
			break;
#endif

#ifdef SIP_3GPP
		case SipHdrTypePcfAddr:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pGeneralHdr->pPcfAddrHdr!=SIP_NULL)
					sip_freeSipPcfAddrHeader(pGeneralHdr->\
						pPcfAddrHdr);
				pGeneralHdr->pPcfAddrHdr=(SipPcfAddrHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipPcfAddrHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pGeneralHdr->pPcfAddrHdr == SIP_NULL)
				{
					if (sip_initSipPcfAddrHeader(&(pGeneralHdr->\
						pPcfAddrHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPcfAddrHeader(pGeneralHdr->\
					pPcfAddrHdr, (SipPcfAddrHeader *)(hdr->pHeader),\
					err) == SipFail)
				{
					sip_freeSipPcfAddrHeader(\
						pGeneralHdr->pPcfAddrHdr);
					return SipFail;
				}
#endif
			 }
			 break;

#endif
		default:
#ifdef SIP_DCS
			if (sip_dcs_setDcsGeneralHeaderAtIndex (pGeneralHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function setGeneralHeaderAtIndex");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  insertGeneralHeaderAtIndex
**
** DESCRIPTION: This function inserts a general pHeader "hdr" at the
**	index specified by "index".
**
******************************************************************/
SipBool insertGeneralHeaderAtIndex
#ifdef ANSI_PROTO
	(SipGeneralHeader *pGeneralHdr, SipHeader *hdr, SIP_U32bit index,
	 SipError *err)
#else
	(pGeneralHdr, hdr, index, err)
	SipGeneralHeader *pGeneralHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipAllowHeader *allow;
#ifdef SIP_IMPP
	SipAllowEventsHeader *allowevents;
#endif
	SipContentDispositionHeader *contentdisposition;
	SipReasonHeader *pReason;
	SipContentLanguageHeader *contentlang;
	SipAcceptHeader *accep;
	SipAcceptEncodingHeader *accep_enc;
	SipAcceptLangHeader *accep_lang;
	SipRecordRouteHeader *record;
#ifdef SIP_3GPP
	SipPathHeader *pPath;
	SipServiceRouteHeader *pService=SIP_NULL;
#endif
	SipViaHeader *via;
#ifdef SIP_PRIVACY
	SipPAssertIdHeader *pPAssertId =SIP_NULL ;
	SipPPreferredIdHeader *pPPreferredId =SIP_NULL ;
#endif
	SipContentEncodingHeader *pEncoding;
	SipUnknownHeader *unknown;
	SipContactHeader *contact;
	SipSupportedHeader *pSupp;
	SipRequireHeader *require;
	SipCallInfoHeader *callinfo;
#endif

    SipBool dSecondInsertionFlag = SipFail;

	SIPDEBUGFN("Entering function insertGeneralHeaderAtIndex");

	switch(hdr->dType)
	{
		case SipHdrTypeAllow:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slAllowHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAllowHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAllowHeader (&allow, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAllowHeader(allow, \
				(SipAllowHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAllowHeader(allow);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slAllowHdr),index,\
				(SIP_Pvoid)allow, err) == SipFail)
			{
				sip_freeSipAllowHeader(allow);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeContentDisposition:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slContentDispositionHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentDispositionHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentDispositionHeader (&contentdisposition, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentDispositionHeader(contentdisposition, \
				(SipContentDispositionHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentDispositionHeader(contentdisposition);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slContentDispositionHdr),index,\
				(SIP_Pvoid)contentdisposition, err) == SipFail)
			{
				sip_freeSipContentDispositionHeader(contentdisposition);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeReason:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slReasonHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipReasonHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipReasonHeader (&pReason, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipReasonHeader(pReason, \
				(SipReasonHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipReasonHeader(pReason);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slReasonHdr),index,\
				(SIP_Pvoid)pReason, err) == SipFail)
			{
				sip_freeSipReasonHeader(pReason);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeContentLanguage:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slContentLanguageHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentLanguageHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentLanguageHeader (&contentlang, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentLanguageHeader(contentlang, \
				(SipContentLanguageHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentLanguageHeader(contentlang);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slContentLanguageHdr),index,\
				(SIP_Pvoid)contentlang, err) == SipFail)
			{
				sip_freeSipContentLanguageHeader(contentlang);
				return SipFail;
			}
#endif
			break;




		case SipHdrTypeAccept:
#ifdef SIP_BY_REFERENCE
			 if (sip_listInsertAt(&(pGeneralHdr->slAcceptHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptHeader (&accep, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptHeader(accep, (SipAcceptHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptHeader(accep);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pGeneralHdr->slAcceptHdr),index, \
				(SIP_Pvoid)accep, err) == SipFail)
			 {
				sip_freeSipAcceptHeader(accep);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypeAcceptEncoding:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slAcceptEncoding),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptEncodingHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptEncodingHeader (&accep_enc, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptEncodingHeader(accep_enc, \
				(SipAcceptEncodingHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptEncodingHeader(accep_enc);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slAcceptEncoding),index, \
				(SIP_Pvoid)accep_enc, err) == SipFail)
			{
				sip_freeSipAcceptEncodingHeader(accep_enc);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAcceptLanguage:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slAcceptLang),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptLangHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptLangHeader (&accep_lang, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptLangHeader(accep_lang, \
				(SipAcceptLangHeader *)(hdr->pHeader), err) == SipFail)
			 {
				sip_freeSipAcceptLangHeader(accep_lang);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pGeneralHdr->slAcceptLang),index,\
				(SIP_Pvoid)accep_lang, err) == SipFail)
			 {
				sip_freeSipAcceptLangHeader(accep_lang);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypeCallId:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pCallidHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pCallidHdr=(SipCallIdHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipCallIdHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipCallIdHeader(&(pGeneralHdr->pCallidHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipCallIdHeader(pGeneralHdr->pCallidHdr,\
						(SipCallIdHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipCallIdHeader(pGeneralHdr->pCallidHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypeCseq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pCseqHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pCseqHdr=(SipCseqHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipCseqHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipCseqHeader(&(pGeneralHdr->pCseqHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipCseqHeader(pGeneralHdr->pCseqHdr,\
						(SipCseqHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipCseqHeader(pGeneralHdr->pCseqHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pDateHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pDateHdr=(SipDateHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipDateHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipDateHeader(&(pGeneralHdr->pDateHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipDateHeader(pGeneralHdr->pDateHdr,\
						(SipDateHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipDateHeader(pGeneralHdr->pDateHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeEncryption:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pEncryptionHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pEncryptionHdr=(SipEncryptionHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipEncryptionHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipEncryptionHeader(&\
						(pGeneralHdr->pEncryptionHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipEncryptionHeader(\
						pGeneralHdr->pEncryptionHdr, (SipEncryptionHeader *)(hdr->pHeader),\
						err) == SipFail)
					{
						sip_freeSipEncryptionHeader\
							(pGeneralHdr->pEncryptionHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

	case SipHdrTypeReplyTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pReplyToHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pReplyToHdr=(SipReplyToHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipReplyToHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipReplyToHeader(&\
						(pGeneralHdr->pReplyToHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipReplyToHeader(\
						pGeneralHdr->pReplyToHdr, (SipReplyToHeader *)(hdr->pHeader),\
						err) == SipFail)
					{
						sip_freeSipReplyToHeader\
							(pGeneralHdr->pReplyToHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypeExpiresDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pExpiresHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pExpiresHdr=(SipExpiresHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipExpiresHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipExpiresHeader(&(pGeneralHdr->pExpiresHdr), \
						SipExpDate, err) == SipFail)
						return SipFail;
					if (__sip_cloneSipExpiresHeader(pGeneralHdr->pExpiresHdr, \
						(SipExpiresHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipExpiresHeader(pGeneralHdr->pExpiresHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeExpiresSec:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pExpiresHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pExpiresHdr=(SipExpiresHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipExpiresHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipExpiresHeader(&(pGeneralHdr->pExpiresHdr),\
						SipExpSeconds, err) == SipFail)
						return SipFail;
					if (__sip_cloneSipExpiresHeader(pGeneralHdr->pExpiresHdr,\
						(SipExpiresHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipExpiresHeader(pGeneralHdr->pExpiresHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;

#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pMinSEHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pMinSEHdr=(SipMinSEHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipMinSEHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipMinSEHeader(&(pGeneralHdr->pMinSEHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipMinSEHeader(pGeneralHdr->pMinSEHdr,\
						(SipMinSEHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipMinSEHeader(pGeneralHdr->pMinSEHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

			case SipHdrTypeSessionExpires:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pSessionExpiresHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pSessionExpiresHdr=(SipSessionExpiresHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipSessionExpiresHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipSessionExpiresHeader(&(pGeneralHdr->pSessionExpiresHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipSessionExpiresHeader(pGeneralHdr->pSessionExpiresHdr,\
						(SipSessionExpiresHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipSessionExpiresHeader(pGeneralHdr->pSessionExpiresHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
#endif

		case SipHdrTypeFrom:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pFromHeader == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pFromHeader=(SipFromHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipFromHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipFromHeader(&(pGeneralHdr->pFromHeader),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipFromHeader(pGeneralHdr->pFromHeader,\
						(SipFromHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipFromHeader(pGeneralHdr->pFromHeader);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeRecordRoute:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slRecordRouteHdr),index,\
				(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRecordRouteHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRecordRouteHeader(&record, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRecordRouteHeader(record, (SipRecordRouteHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRecordRouteHeader(record);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slRecordRouteHdr),index,\
				(SIP_Pvoid)record,err) == SipFail)
			{
				sip_freeSipRecordRouteHeader(record);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_3GPP
		case SipHdrTypePath:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slPathHdr),index,\
				(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPathHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPathHeader(&pPath, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPathHeader(pPath, (SipPathHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPathHeader(pPath);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slPathHdr),index,\
				(SIP_Pvoid)pPath,err) == SipFail)
			{
				sip_freeSipPathHeader(pPath);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeServiceRoute:
			{
#ifdef SIP_BY_REFERENCE
					if (sip_listInsertAt(&(pGeneralHdr->slServiceRouteHdr),index,\
											(SIP_Pvoid)(hdr->pHeader),err) == SipFail)
							return SipFail;
					HSS_LOCKEDINCREF(((SipServiceRouteHeader*)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipServiceRouteHeader(&pService, err) == SipFail)
							return SipFail;
					if (__sip_cloneSipServiceRouteHeader(pService,\
							(SipServiceRouteHeader *) (hdr->pHeader), err) == SipFail)
					{
							sip_freeSipServiceRouteHeader(pService);
							return SipFail;
					}
					if (sip_listInsertAt(&(pGeneralHdr->slServiceRouteHdr),index,\
											(SIP_Pvoid)pService,err) == SipFail)
					{
							sip_freeSipServiceRouteHeader(pService);
							return SipFail;
					}
#endif
					break;
			}

		case SipHdrTypePanInfo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPanInfoHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
			pGeneralHdr->pPanInfoHdr=(SipPanInfoHeader *)(hdr->pHeader);
			HSS_LOCKEDINCREF(((SipPanInfoHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipPanInfoHeader(&(pGeneralHdr->pPanInfoHdr)\
						,err) == SipFail)
						return SipFail;
					if (__sip_cloneSipPanInfoHeader(pGeneralHdr->pPanInfoHdr\
					  ,(SipPanInfoHeader *)(hdr->pHeader), err) == SipFail)
					{
					 sip_freeSipPanInfoHeader(pGeneralHdr->pPanInfoHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
	    case SipHdrTypePcVector:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPcVectorHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
			pGeneralHdr->pPcVectorHdr=(SipPcVectorHeader *)(hdr->pHeader);
			HSS_LOCKEDINCREF(((SipPcVectorHeader *)(hdr->pHeader))->dRefCount);
#else
				if (sip_initSipPcVectorHeader(&(pGeneralHdr->pPcVectorHdr)\
						,err) == SipFail)
						return SipFail;
				if (__sip_cloneSipPcVectorHeader(pGeneralHdr->pPcVectorHdr\
					  ,(SipPcVectorHeader *)(hdr->pHeader), err) == SipFail)
					{
					 sip_freeSipPcVectorHeader(pGeneralHdr->pPcVectorHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
	
#endif
		case SipHdrTypeTimestamp:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pTimeStampHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pTimeStampHdr=(SipTimeStampHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipTimeStampHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipTimeStampHeader(&(pGeneralHdr->\
						pTimeStampHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipTimeStampHeader(pGeneralHdr->\
						pTimeStampHdr, (SipTimeStampHeader *)(hdr->pHeader),\
						err) == SipFail)
					{
						sip_freeSipTimeStampHeader(pGeneralHdr->pTimeStampHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pToHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pToHdr=(SipToHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipToHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipToHeader(&(pGeneralHdr->pToHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipToHeader(pGeneralHdr->pToHdr,\
						(SipToHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipToHeader(pGeneralHdr->pToHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeVia:
#ifdef SIP_BY_REFERENCE
			 if (sip_listInsertAt(&(pGeneralHdr->slViaHdr),index, (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipViaHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipViaHeader (&via, err) == SipFail)
				return SipFail;
		 	if (__sip_cloneSipViaHeader(via, (SipViaHeader *)(hdr->pHeader),\
				err) == SipFail)
			{
				sip_freeSipViaHeader(via);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pGeneralHdr->slViaHdr),index, (SIP_Pvoid)via, err) == SipFail)
			 {
				sip_freeSipViaHeader(via);
				return SipFail;
			 }
#endif
			 break;
#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId:
#ifdef SIP_BY_REFERENCE
			 if (sip_listInsertAt(&(pGeneralHdr->slPAssertIdHdr),
                  index, (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF((\
				(SipPAssertIdHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPAssertIdHeader (&pPAssertId, err) == SipFail)
				return SipFail;
		 	if (__sip_cloneSipPAssertIdHeader(pPAssertId, \
					(SipPAssertIdHeader *)(hdr->pHeader),err) == SipFail)
			{
				sip_freeSipPAssertIdHeader(pPAssertId);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pGeneralHdr->slPAssertIdHdr),index, \
			 	  (SIP_Pvoid)pPAssertId, err) == SipFail)
			 {
				sip_freeSipPAssertIdHeader(pPAssertId);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypePPreferredId:
#ifdef SIP_BY_REFERENCE
			 if (sip_listInsertAt(&(pGeneralHdr->slPPreferredIdHdr),
                  index, (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF((\
				(SipPPreferredIdHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPPreferredIdHeader (&pPPreferredId, err) == SipFail)
				return SipFail;
		 	if (__sip_cloneSipPPreferredIdHeader(pPPreferredId, \
					(SipPPreferredIdHeader *)(hdr->pHeader),err) == SipFail)
			{
				sip_freeSipPPreferredIdHeader(pPPreferredId);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pGeneralHdr->slPPreferredIdHdr),index, \
			 	  (SIP_Pvoid)pPPreferredId, err) == SipFail)
			 {
				sip_freeSipPPreferredIdHeader(pPPreferredId);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypePrivacy :
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPrivacyHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pPrivacyHdr=(SipPrivacyHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipPrivacyHeader *)\
										(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipPrivacyHeader(&(pGeneralHdr->pPrivacyHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipPrivacyHeader(pGeneralHdr->pPrivacyHdr,\
						(SipPrivacyHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipPrivacyHeader(pGeneralHdr->pPrivacyHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
			
#endif /* # ifdef SIP_PRIVACY */

		case SipHdrTypeContentEncoding:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slContentEncodingHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContentEncodingHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContentEncodingHeader (&pEncoding, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContentEncodingHeader(pEncoding, \
				(SipContentEncodingHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContentEncodingHeader(pEncoding);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slContentEncodingHdr),index, \
				(SIP_Pvoid)pEncoding, err) == SipFail)
			{
				sip_freeSipContentEncodingHeader(pEncoding);
				return SipFail;
			}
#endif
			break;
	/* RPR */
		case SipHdrTypeSupported:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slSupportedHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipSupportedHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipSupportedHeader(&pSupp, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipSupportedHeader(pSupp, (SipSupportedHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipSupportedHeader(pSupp);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slSupportedHdr),index,\
				(SIP_Pvoid)pSupp, err) == SipFail)
			{
				sip_freeSipSupportedHeader(pSupp);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeRequire:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slRequireHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRequireHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRequireHeader (&require, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRequireHeader(require, (SipRequireHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRequireHeader(require);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slRequireHdr),index,\
				(SIP_Pvoid)require, err) == SipFail)
			{
				sip_freeSipRequireHeader(require);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeContentLength:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pContentLengthHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pContentLengthHdr=(SipContentLengthHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipContentLengthHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipContentLengthHeader(\
						&(pGeneralHdr->pContentLengthHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipContentLengthHeader(\
						pGeneralHdr->pContentLengthHdr,\
						(SipContentLengthHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipContentLengthHeader(\
							pGeneralHdr->pContentLengthHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeContentType:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			 {
				if (pGeneralHdr->pContentTypeHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pContentTypeHdr=(SipContentTypeHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipContentTypeHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipContentTypeHeader(\
						&(pGeneralHdr->pContentTypeHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipContentTypeHeader(\
						pGeneralHdr->pContentTypeHdr, \
						(SipContentTypeHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipContentTypeHeader(\
							pGeneralHdr->pContentTypeHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
	/* bcpt ext */
		case SipHdrTypeMimeVersion:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pMimeVersionHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pMimeVersionHdr=(SipMimeVersionHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipMimeVersionHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_bcpt_initSipMimeVersionHeader(&(pGeneralHdr->\
						pMimeVersionHdr), err) == SipFail)
						return SipFail;
					if (__sip_bcpt_cloneSipMimeVersionHeader(pGeneralHdr->\
						pMimeVersionHdr, (SipMimeVersionHeader *)(hdr->pHeader),\
						err) == SipFail)
					{
						sip_bcpt_freeSipMimeVersionHeader(pGeneralHdr->\
							pMimeVersionHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
/* bcpt ext ends */
		case SipHdrTypeOrganization:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pOrganizationHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pOrganizationHdr=(SipOrganizationHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipOrganizationHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipOrganizationHeader(\
						&(pGeneralHdr->pOrganizationHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipOrganizationHeader(\
						pGeneralHdr->pOrganizationHdr,\
						(SipOrganizationHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipOrganizationHeader(\
							pGeneralHdr->pOrganizationHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeContactWildCard:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContactHeader (&contact, SipContactWildCard, \
				err) == SipFail)
				return SipFail;
			if (__sip_cloneSipContactHeader(contact, \
				(SipContactHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)contact, err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeContactNormal:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipContactHeader (&contact, SipContactNormal, err)\
				 == SipFail)
				return SipFail;
			if (__sip_cloneSipContactHeader(contact, (SipContactHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slContactHdr),index,\
				(SIP_Pvoid)contact, err) == SipFail)
			{
				sip_freeSipContactHeader(contact);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeCallInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slCallInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipCallInfoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipCallInfoHeader (&callinfo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipCallInfoHeader(callinfo, \
				(SipCallInfoHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipCallInfoHeader(callinfo);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slCallInfoHdr),index,\
				(SIP_Pvoid)callinfo, err) == SipFail)
			{
				sip_freeSipCallInfoHeader(callinfo);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeUserAgent:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pUserAgentHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pUserAgentHdr=(SipUserAgentHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipUserAgentHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipUserAgentHeader(\
						&(pGeneralHdr->pUserAgentHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipUserAgentHeader(pGeneralHdr->pUserAgentHdr,\
						(SipUserAgentHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipUserAgentHeader(pGeneralHdr->pUserAgentHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeRetryAfterDate:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pRetryAfterHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pRetryAfterHdr=(SipRetryAfterHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipRetryAfterHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipRetryAfterHeader(&(pGeneralHdr->\
						pRetryAfterHdr), SipExpDate, err) == SipFail)
						return SipFail;
					if (__sip_cloneSipRetryAfterHeader(pGeneralHdr->\
						pRetryAfterHdr, (SipRetryAfterHeader *)(hdr->pHeader), err)\
						== SipFail)
					{
						sip_freeSipRetryAfterHeader(\
							pGeneralHdr->pRetryAfterHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
		case SipHdrTypeRetryAfterSec:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pRetryAfterHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pRetryAfterHdr=(SipRetryAfterHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipRetryAfterHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipRetryAfterHeader(&(pGeneralHdr->\
						pRetryAfterHdr), SipExpSeconds, err) == SipFail)
						return SipFail;
					if (__sip_cloneSipRetryAfterHeader(pGeneralHdr->\
						pRetryAfterHdr, (SipRetryAfterHeader *)(hdr->pHeader), \
						err) == SipFail)
					{
						sip_freeSipRetryAfterHeader(\
							pGeneralHdr->pRetryAfterHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeUnknown:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slUnknownHdr),index, \
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipUnknownHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipUnknownHeader (&unknown, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipUnknownHeader(unknown, (SipUnknownHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipUnknownHeader(unknown);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slUnknownHdr),index, \
				(SIP_Pvoid)unknown, err) == SipFail)
			{
				sip_freeSipUnknownHeader(unknown);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pGeneralHdr->slAllowEventsHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAllowEventsHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_impp_initSipAllowEventsHeader (&allowevents, err) == SipFail)
				return SipFail;
			if (__sip_impp_cloneSipAllowEventsHeader(allowevents, \
				(SipAllowEventsHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_impp_freeSipAllowEventsHeader(allowevents);
				return SipFail;
			}
			if (sip_listInsertAt(&(pGeneralHdr->slAllowEventsHdr),index,\
				(SIP_Pvoid)allowevents, err) == SipFail)
			{
				sip_impp_freeSipAllowEventsHeader(allowevents);
				return SipFail;
			}
#endif
			break;
#endif

#ifdef SIP_3GPP
		case SipHdrTypePcfAddr:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pGeneralHdr->pPcfAddrHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pGeneralHdr->pPcfAddrHdr=(SipPcfAddrHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipPcfAddrHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipPcfAddrHeader(&(pGeneralHdr->\
						pPcfAddrHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipPcfAddrHeader(pGeneralHdr->\
						pPcfAddrHdr, (SipPcfAddrHeader *)(hdr->pHeader), \
						err) == SipFail)
					{
						sip_freeSipPcfAddrHeader(\
							pGeneralHdr->pPcfAddrHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
#endif            

		default:
#ifdef SIP_DCS
			if (sip_dcs_insertDcsGeneralHeaderAtIndex (pGeneralHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

    if(dSecondInsertionFlag == SipSuccess)
    {
        if (setGeneralHeaderAtIndex(pGeneralHdr,hdr,0,err)== SipFail)
        {
            return SipFail ;
        }
        else
        {
            /* actually there is no error.
               We wanted to pass additional info without adding 
               extra parameters iso we make use of err variable*/

            *err = E_SECND_INSERTION_SINGLE_INST_HDR ;
            return SipSuccess ;
        }
    }

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function insertGeneralHeaderAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  setRequestHeaderAtIndex
**
** DESCRIPTION: This function sets a request pHeader "hdr" at the
**	index specified by "index".
**
******************************************************************/
SipBool setRequestHeaderAtIndex
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr, SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(pRequestHdr, hdr, index, err)
	SipReqHeader *pRequestHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipProxyRequireHeader *proxy_req;
	SipProxyAuthorizationHeader *proxy_auth;
	SipRouteHeader *route;
	SipAlsoHeader *also;
	SipWwwAuthenticateHeader *wwwauth;
	SipAuthorizationHeader *auth;
#ifdef SIP_SECURITY
	SipSecurityClientHeader *sec_cli;
	SipSecurityVerifyHeader *sec_ver;
#endif

#ifdef SIP_CCP
	SipAcceptContactHeader *pAccContact;
	SipRejectContactHeader *pRejContact;
	SipRequestDispositionHeader *pReqDisp;
#endif
	SipAlertInfoHeader *pAlertInfo;
	SipInReplyToHeader *pInReplyTo;
#ifdef SIP_3GPP
	SipPVisitedNetworkIdHeader *pPVisitedNetworkId;
#endif
#ifdef SIP_CONGEST
	SipRsrcPriorityHeader *pRsrcPriority;
#endif

#endif
	en_SipBoolean z;
	SIPDEBUGFN("Entering function setRequestHeaderAtIndex");
	z = SipTrue;

	switch(hdr->dType)
	{
		case SipHdrTypeAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthorizationHeader (&auth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAuthorizationHeader(auth, \
				(SipAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)auth, err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
#endif
			 break;
		case SipHdrTypeHide:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pHideHdr != SIP_NULL)
					sip_freeSipHideHeader(pRequestHdr->pHideHdr);
				pRequestHdr->pHideHdr=(SipHideHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipHideHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pHideHdr == SIP_NULL)
				{
					if (sip_initSipHideHeader(&(pRequestHdr->pHideHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipHideHeader(pRequestHdr->pHideHdr,\
					(SipHideHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipHideHeader(pRequestHdr->pHideHdr);
					return SipFail;
				}
#endif
			 }
			 break;

	case SipHdrTypeReplaces:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pReplacesHdr != SIP_NULL)
					sip_freeSipReplacesHeader(pRequestHdr->pReplacesHdr);
				pRequestHdr->pReplacesHdr=(SipReplacesHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipReplacesHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pReplacesHdr == SIP_NULL)
				{
					if (sip_initSipReplacesHeader(&(pRequestHdr->pReplacesHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipReplacesHeader(pRequestHdr->pReplacesHdr,\
					(SipReplacesHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipReplacesHeader(pRequestHdr->pReplacesHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeMaxforwards:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pMaxForwardsHdr!=SIP_NULL)
					sip_freeSipMaxForwardsHeader(pRequestHdr->pMaxForwardsHdr);
				pRequestHdr->pMaxForwardsHdr=(SipMaxForwardsHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipMaxForwardsHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pMaxForwardsHdr == SIP_NULL)
				{
					if (sip_initSipMaxForwardsHeader(\
						&(pRequestHdr->pMaxForwardsHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipMaxForwardsHeader(pRequestHdr->pMaxForwardsHdr,\
					(SipMaxForwardsHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipMaxForwardsHeader(\
						pRequestHdr->pMaxForwardsHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypePriority:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pPriorityHdr!=SIP_NULL)
					sip_freeSipPriorityHeader(pRequestHdr->pPriorityHdr);
				pRequestHdr->pPriorityHdr=(SipPriorityHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipPriorityHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pPriorityHdr == SIP_NULL)
				{
					if (sip_initSipPriorityHeader(\
						&(pRequestHdr->pPriorityHdr), err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPriorityHeader(pRequestHdr->pPriorityHdr,\
					(SipPriorityHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipPriorityHeader(pRequestHdr->pPriorityHdr);
					return SipFail;
				}
#endif
			}
			break;
		case SipHdrTypeProxyauthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slProxyAuthorizationHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipProxyAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyAuthorizationHeader (&proxy_auth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipProxyAuthorizationHeader(proxy_auth, \
				(SipProxyAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipProxyAuthorizationHeader(proxy_auth);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slProxyAuthorizationHdr),index,\
				 (SIP_Pvoid)proxy_auth, err) == SipFail)
			{
				sip_freeSipProxyAuthorizationHeader(proxy_auth);
				return SipFail;
			}
#endif
			break;


		case SipHdrTypeProxyRequire:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slProxyRequireHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipProxyRequireHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyRequireHeader (&proxy_req, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipProxyRequireHeader(proxy_req, \
				(SipProxyRequireHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipProxyRequireHeader(proxy_req);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slProxyRequireHdr),index,\
				 (SIP_Pvoid)proxy_req, err) == SipFail)
			{
				sip_freeSipProxyRequireHeader(proxy_req);
				return SipFail;
			}
#endif
			break;

/* call-transfer */
		case SipHdrTypeReferTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pReferToHdr != SIP_NULL)
					sip_freeSipReferToHeader(pRequestHdr->\
						pReferToHdr);
				pRequestHdr->pReferToHdr=(SipReferToHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipReferToHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pReferToHdr == SIP_NULL)
				{
					if (sip_initSipReferToHeader(\
						&(pRequestHdr->pReferToHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipReferToHeader(\
					pRequestHdr->pReferToHdr, \
					(SipReferToHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipReferToHeader(\
							pRequestHdr->pReferToHdr);
					return SipFail;
				}
#endif
			 }
			 break;

		case SipHdrTypeReferredBy:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pReferredByHdr != SIP_NULL)
					sip_freeSipReferredByHeader(pRequestHdr->\
						pReferredByHdr);
				pRequestHdr->pReferredByHdr=(SipReferredByHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipReferredByHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pReferredByHdr == SIP_NULL)
				{
					if (sip_initSipReferredByHeader(\
						&(pRequestHdr->pReferredByHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipReferredByHeader(\
					pRequestHdr->pReferredByHdr, \
					(SipReferredByHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipReferredByHeader(\
							pRequestHdr->pReferredByHdr);
					return SipFail;
				}
#endif
			 }
			 break; /* call-transfer */

#ifdef SIP_IMPP
		case SipHdrTypeEvent:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pEventHdr != SIP_NULL)
					sip_impp_freeSipEventHeader(pRequestHdr->pEventHdr);
				pRequestHdr->pEventHdr=(SipEventHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipEventHeader *) (hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pEventHdr == SIP_NULL)
				{
					if (sip_impp_initSipEventHeader(&(pRequestHdr->pEventHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_impp_cloneSipEventHeader(pRequestHdr->pEventHdr, \
					(SipEventHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_impp_freeSipEventHeader(pRequestHdr->pEventHdr);
					return SipFail;
				}
#endif
			 }
			 break;
		case SipHdrTypeSubscriptionState:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pSubscriptionStateHdr!=SIP_NULL)
					sip_impp_freeSipSubscriptionStateHeader(pRequestHdr-> \
					pSubscriptionStateHdr);
				pRequestHdr->pSubscriptionStateHdr= \
				(SipSubscriptionStateHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipSubscriptionStateHeader *) ((hdr)-> \
				pHeader))->dRefCount);
#else
				if (pRequestHdr->pSubscriptionStateHdr == SIP_NULL)
				{
					if (sip_impp_initSipSubscriptionStateHeader(& \
						(pRequestHdr->pSubscriptionStateHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_impp_cloneSipSubscriptionStateHeader(pRequestHdr->\
					pSubscriptionStateHdr,(SipSubscriptionStateHeader *) \
					(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_impp_freeSipSubscriptionStateHeader ( \
						pRequestHdr->pSubscriptionStateHdr);
					return SipFail;
				}
#endif
			}
		 	break;
#endif /* IMPP */
			

		case SipHdrTypeRoute:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slRouteHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRouteHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRouteHeader (&route, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRouteHeader(route, (SipRouteHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRouteHeader(route);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slRouteHdr),index,\
				(SIP_Pvoid)route, err) == SipFail)
			{
				sip_freeSipRouteHeader(route);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAlso:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slAlsoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAlsoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAlsoHeader (&also, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAlsoHeader(also, (SipAlsoHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAlsoHeader(also);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slAlsoHdr), index,\
				(SIP_Pvoid)also, err) == SipFail)
			{
				sip_freeSipAlsoHeader(also);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeResponseKey:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pRespKeyHdr!=SIP_NULL)
					sip_freeSipRespKeyHeader(pRequestHdr->pRespKeyHdr);
				pRequestHdr->pRespKeyHdr=(SipRespKeyHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipRespKeyHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pRespKeyHdr == SIP_NULL)
				{
					if (sip_initSipRespKeyHeader(&(pRequestHdr->pRespKeyHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipRespKeyHeader(pRequestHdr->pRespKeyHdr,\
					(SipRespKeyHeader *) (hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipRespKeyHeader(pRequestHdr->pRespKeyHdr);
					return SipFail;
				}
#endif
			}
			break;
		case SipHdrTypeSubject:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pSubjectHdr!=SIP_NULL)
					sip_freeSipSubjectHeader(pRequestHdr->pSubjectHdr);
				pRequestHdr->pSubjectHdr=(SipSubjectHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipSubjectHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pSubjectHdr == SIP_NULL)
				{
					if (sip_initSipSubjectHeader(&(pRequestHdr->pSubjectHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipSubjectHeader(pRequestHdr->pSubjectHdr,\
					(SipSubjectHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipSubjectHeader(pRequestHdr->pSubjectHdr);
					return SipFail;
				}
#endif
			}
			break;
	/* Retrans */
		case SipHdrTypeRAck:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pRackHdr!=SIP_NULL)
					sip_rpr_freeSipRAckHeader(pRequestHdr->pRackHdr);
				pRequestHdr->pRackHdr=(SipRackHeader *) (hdr->pHeader);
				HSS_LOCKEDINCREF(((SipRackHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pRackHdr == SIP_NULL)
				{
					if (sip_rpr_initSipRAckHeader(&(pRequestHdr->pRackHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_rpr_cloneSipRAckHeader(pRequestHdr->pRackHdr,\
					(SipRackHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_rpr_freeSipRAckHeader(pRequestHdr->pRackHdr);
					return SipFail;
				}
#endif
			 }
			 break;
	/* Retrans ends */
		case SipHdrTypeWwwAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWwwAuthenticateHeader (&wwwauth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWwwAuthenticateHeader(wwwauth, \
				(SipWwwAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)wwwauth, err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_CCP
		case SipHdrTypeAcceptContact:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slAcceptContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipAcceptContactHeader(&pAccContact, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipAcceptContactHeader(pAccContact, \
				(SipAcceptContactHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_ccp_freeSipAcceptContactHeader(pAccContact);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slAcceptContactHdr),index,\
				(SIP_Pvoid)pAccContact, err) == SipFail)
			{
				sip_ccp_freeSipAcceptContactHeader(pAccContact);
				return SipFail;
			}
#endif
			break; /* CCP case	*/
		case SipHdrTypeRejectContact:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slRejectContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRejectContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipRejectContactHeader(&pRejContact, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipRejectContactHeader(pRejContact,\
				(SipRejectContactHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_ccp_freeSipRejectContactHeader(pRejContact);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slRejectContactHdr),index,\
				(SIP_Pvoid)pRejContact, err) == SipFail)
			{
				sip_ccp_freeSipRejectContactHeader(pRejContact);
				return SipFail;
			}
#endif
			break; /* CCP case	*/
		case SipHdrTypeRequestDisposition:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slRequestDispositionHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRequestDispositionHeader *)\
				(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipRequestDispositionHeader(&pReqDisp, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipRequestDispositionHeader(pReqDisp, \
				(SipRequestDispositionHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_ccp_freeSipRequestDispositionHeader(pReqDisp);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slRequestDispositionHdr),index,\
				(SIP_Pvoid)pReqDisp, err) == SipFail)
			{
				sip_ccp_freeSipRequestDispositionHeader(pReqDisp);
				return SipFail;
			}
#endif
			break; /* CCP case	*/
#endif /* of CCP */

		case SipHdrTypeAlertInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slAlertInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAlertInfoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAlertInfoHeader (&pAlertInfo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAlertInfoHeader(pAlertInfo,(SipAlertInfoHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAlertInfoHeader(pAlertInfo);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slAlertInfoHdr),index,\
				(SIP_Pvoid)pAlertInfo, err) == SipFail)
			{
				sip_freeSipAlertInfoHeader(pAlertInfo);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeInReplyTo:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slInReplyToHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipInReplyToHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipInReplyToHeader (&pInReplyTo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipInReplyToHeader(pInReplyTo, (SipInReplyToHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipInReplyToHeader(pInReplyTo);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slInReplyToHdr),index,\
				(SIP_Pvoid)pInReplyTo, err) == SipFail)
			{
				sip_freeSipInReplyToHeader(pInReplyTo);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_CONF            
	    /* Join header */
        case SipHdrTypeJoin:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pJoinHdr != SIP_NULL)
					sip_freeSipJoinHeader(pRequestHdr->pJoinHdr);
				pRequestHdr->pJoinHdr=(SipJoinHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipJoinHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pJoinHdr == SIP_NULL)
				{
					if (sip_initSipJoinHeader(&(pRequestHdr->pJoinHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipJoinHeader(pRequestHdr->pJoinHdr,\
					(SipJoinHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipJoinHeader(pRequestHdr->pJoinHdr);
					return SipFail;
				}
#endif
			 }
			 break;/* for Join header */
#endif

#ifdef SIP_SECURITY
	case SipHdrTypeSecurityClient:
#ifdef SIP_BY_REFERENCE
                        if (sip_listSetAt(&(pRequestHdr->slSecurityClientHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityClientHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityClientHeader (&sec_cli, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityClientHeader(sec_cli, (SipSecurityClientHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityClientHeader(sec_cli);
                                return SipFail;
                        }
                        if (sip_listSetAt(&(pRequestHdr->slSecurityClientHdr),index,\
                                (SIP_Pvoid)sec_cli, err) == SipFail)
                        {
                                sip_freeSipSecurityClientHeader(sec_cli);
                                return SipFail;
                        }
#endif
                        break; /* for Security-Client */
	      
	case SipHdrTypeSecurityVerify:
#ifdef SIP_BY_REFERENCE
                        if (sip_listSetAt(&(pRequestHdr->slSecurityVerifyHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityVerifyHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityVerifyHeader (&sec_ver, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityVerifyHeader(sec_ver, (SipSecurityVerifyHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityVerifyHeader(sec_ver);
                                return SipFail;
                        }
                        if (sip_listSetAt(&(pRequestHdr->slSecurityVerifyHdr),index,\
                                (SIP_Pvoid)sec_ver, err) == SipFail)
                        {
                                sip_freeSipSecurityVerifyHeader(sec_ver);
                                return SipFail;
                        }
#endif
                        break; /* for Security-Verify */
#endif /* end of #ifdef SIP_SECURITY */

#ifdef SIP_3GPP
	    /* PCalledPartyId header */
        case SipHdrTypePCalledPartyId:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pRequestHdr->pPCalledPartyIdHdr != SIP_NULL)
					sip_freeSipPCalledPartyIdHeader(pRequestHdr->pPCalledPartyIdHdr);
				pRequestHdr->pPCalledPartyIdHdr=(SipPCalledPartyIdHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipPCalledPartyIdHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pRequestHdr->pPCalledPartyIdHdr == SIP_NULL)
				{
					if (sip_initSipPCalledPartyIdHeader(&(pRequestHdr->pPCalledPartyIdHdr), \
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipPCalledPartyIdHeader(pRequestHdr->pPCalledPartyIdHdr,\
					(SipPCalledPartyIdHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipPCalledPartyIdHeader(pRequestHdr->pPCalledPartyIdHdr);
					return SipFail;
				}
#endif
			 }
			 break;/* for PCalledPartyId header */

	    /* PVisitedNetworkId header */
		case SipHdrTypePVisitedNetworkId:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slPVisitedNetworkIdHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPVisitedNetworkIdHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPVisitedNetworkIdHeader (&pPVisitedNetworkId, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPVisitedNetworkIdHeader(pPVisitedNetworkId, (SipPVisitedNetworkIdHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPVisitedNetworkIdHeader(pPVisitedNetworkId);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slPVisitedNetworkIdHdr),index,\
				(SIP_Pvoid)pPVisitedNetworkId, err) == SipFail)
			{
				sip_freeSipPVisitedNetworkIdHeader(pPVisitedNetworkId);
				return SipFail;
			}
#endif
			break;
             
#endif

#ifdef SIP_CONGEST
		case SipHdrTypeRsrcPriority:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pRequestHdr->slRsrcPriorityHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRsrcPriorityHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRsrcPriorityHeader (&pRsrcPriority, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRsrcPriorityHeader(pRsrcPriority, (SipRsrcPriorityHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRsrcPriorityHeader(pRsrcPriority);
				return SipFail;
			}
			if (sip_listSetAt(&(pRequestHdr->slRsrcPriorityHdr),index,\
				(SIP_Pvoid)pRsrcPriority, err) == SipFail)
			{
				sip_freeSipRsrcPriorityHeader(pRsrcPriority);
				return SipFail;
			}
#endif
			break;
#endif
            
             
		default:
#ifdef SIP_DCS
			if (sip_dcs_setDcsRequestHeaderAtIndex (pRequestHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function setRequestHeaderAtIndex");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  insertRequestHeaderAtIndex
**
** DESCRIPTION:  This function inserts a request pHeader "hdr" at the
**	index specified by "index".
**
******************************************************************/
SipBool insertRequestHeaderAtIndex
#ifdef ANSI_PROTO
	(SipReqHeader *pRequestHdr, SipHeader *hdr, SIP_U32bit index, SipError *err)
#else
	(pRequestHdr, hdr, index, err)
	SipReqHeader *pRequestHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipProxyRequireHeader *proxy_req;
	SipProxyAuthorizationHeader *proxy_auth;
	SipRouteHeader *route;
	SipAlsoHeader *also;
	SipWwwAuthenticateHeader *wwwauth;
	SipAuthorizationHeader *auth;
#ifdef SIP_SECURITY
	SipSecurityClientHeader	*sec_cli;
	SipSecurityVerifyHeader *sec_ver;
#endif
#ifdef SIP_CCP
	SipAcceptContactHeader *pAccContact;
	SipRejectContactHeader *pRejContact;
	SipRequestDispositionHeader *pReqDisp;
#endif
	SipAlertInfoHeader *pAlertInfo;
	SipInReplyToHeader *pInReplyTo;
#ifdef SIP_3GPP
	SipPVisitedNetworkIdHeader *pPVisitedNetworkId;
#endif
#ifdef SIP_CONGEST
	SipRsrcPriorityHeader *pRsrcPriority;
#endif

#endif
    
    SipBool dSecondInsertionFlag = SipFail;

	SIPDEBUGFN("Entering function insertRequestHeaderAtIndex");

	switch(hdr->dType)
	{
		case SipHdrTypeAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthorizationHeader (&auth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAuthorizationHeader(auth, \
				(SipAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)auth, err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
#endif
			 break;


		/* call-transfer */
		case SipHdrTypeReferTo:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReferToHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pReferToHdr=(SipReferToHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipReferToHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipReferToHeader(\
						&(pRequestHdr->pReferToHdr), \
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipReferToHeader(\
						pRequestHdr->pReferToHdr, (SipReferToHeader *)\
						(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipReferToHeader(\
							pRequestHdr->pReferToHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;

		case SipHdrTypeReferredBy:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReferredByHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pReferredByHdr=(SipReferredByHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipReferredByHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipReferredByHeader(\
						&(pRequestHdr->pReferredByHdr), \
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipReferredByHeader(\
						pRequestHdr->pReferredByHdr, (SipReferredByHeader *)\
						(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipReferredByHeader(\
							pRequestHdr->pReferredByHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;/* call-transfer */

#ifdef SIP_IMPP
		case SipHdrTypeEvent:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pEventHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pEventHdr=(SipEventHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipEventHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_impp_initSipEventHeader(&(pRequestHdr->pEventHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_impp_cloneSipEventHeader(pRequestHdr->pEventHdr,\
						(SipEventHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_impp_freeSipEventHeader(pRequestHdr->pEventHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypeSubscriptionState:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pSubscriptionStateHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pSubscriptionStateHdr=( \
					SipSubscriptionStateHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipSubscriptionStateHeader *)( \
					hdr->pHeader))->dRefCount);
#else
					if (sip_impp_initSipSubscriptionStateHeader(& \
						(pRequestHdr->pSubscriptionStateHdr), \
						err) == SipFail)
						return SipFail;
					if (__sip_impp_cloneSipSubscriptionStateHeader(pRequestHdr->pSubscriptionStateHdr, \
						(SipSubscriptionStateHeader *)(hdr->pHeader), \
						err) == SipFail)
					{
						sip_impp_freeSipSubscriptionStateHeader( \
						pRequestHdr->pSubscriptionStateHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
#endif /* of IMPP */

		case SipHdrTypeHide:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pHideHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pHideHdr=(SipHideHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipHideHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipHideHeader(&(pRequestHdr->pHideHdr), err)\
						== SipFail)
						return SipFail;
					if (__sip_cloneSipHideHeader(pRequestHdr->pHideHdr,\
						(SipHideHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipHideHeader(pRequestHdr->pHideHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

	case SipHdrTypeReplaces:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pReplacesHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pReplacesHdr=(SipReplacesHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipReplacesHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipReplacesHeader(&(pRequestHdr->pReplacesHdr), err)\
						== SipFail)
						return SipFail;
					if (__sip_cloneSipReplacesHeader(pRequestHdr->pReplacesHdr,\
						(SipReplacesHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipReplacesHeader(pRequestHdr->pReplacesHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypeMaxforwards:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pMaxForwardsHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pMaxForwardsHdr=(SipMaxForwardsHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipMaxForwardsHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipMaxForwardsHeader(\
						&(pRequestHdr->pMaxForwardsHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipMaxForwardsHeader(\
						pRequestHdr->pMaxForwardsHdr,\
						(SipMaxForwardsHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipMaxForwardsHeader(\
							pRequestHdr->pMaxForwardsHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypePriority:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pPriorityHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pPriorityHdr=(SipPriorityHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipPriorityHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipPriorityHeader(&(pRequestHdr->pPriorityHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipPriorityHeader(pRequestHdr->pPriorityHdr,\
						(SipPriorityHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipPriorityHeader(pRequestHdr->pPriorityHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeProxyauthorization:
#ifdef SIP_BY_REFERENCE
			 if(sip_listInsertAt(&(pRequestHdr->slProxyAuthorizationHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			 HSS_LOCKEDINCREF(((SipProxyAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyAuthorizationHeader (&proxy_auth, err)==SipFail)
				return SipFail;
			if (__sip_cloneSipProxyAuthorizationHeader(proxy_auth,\
				(SipProxyAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			 {
				sip_freeSipProxyAuthorizationHeader(proxy_auth);
				return SipFail;
			 }
			 if(sip_listInsertAt(&(pRequestHdr->slProxyAuthorizationHdr),index,\
				(SIP_Pvoid)proxy_auth, err) == SipFail)
			 {
				sip_freeSipProxyAuthorizationHeader(proxy_auth);
				return SipFail;
			 }
#endif
			 break;

		case SipHdrTypeProxyRequire:
#ifdef SIP_BY_REFERENCE
			 if (sip_listInsertAt(&(pRequestHdr->slProxyRequireHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			 HSS_LOCKEDINCREF(((SipProxyAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyRequireHeader (&proxy_req, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipProxyRequireHeader(proxy_req,\
				(SipProxyRequireHeader *)(hdr->pHeader), err) == SipFail)
			 {
				sip_freeSipProxyRequireHeader(proxy_req);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pRequestHdr->slProxyRequireHdr),index,\
				(SIP_Pvoid)proxy_req, err) == SipFail)
			 {
				sip_freeSipProxyRequireHeader(proxy_req);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypeRoute:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slRouteHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRouteHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRouteHeader (&route, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRouteHeader(route, (SipRouteHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRouteHeader(route);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slRouteHdr),index,\
				(SIP_Pvoid)route, err) == SipFail)
			{
				sip_freeSipRouteHeader(route);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAlso:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slAlsoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAlsoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAlsoHeader (&also, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAlsoHeader(also, (SipAlsoHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAlsoHeader(also);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slAlsoHdr),index,\
				(SIP_Pvoid)also, err) == SipFail)
			{
				sip_freeSipAlsoHeader(also);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeResponseKey:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pRespKeyHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pRespKeyHdr=(SipRespKeyHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipRespKeyHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipRespKeyHeader(&(pRequestHdr->pRespKeyHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipRespKeyHeader(pRequestHdr->pRespKeyHdr,\
						(SipRespKeyHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipRespKeyHeader(pRequestHdr->pRespKeyHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
		case SipHdrTypeSubject:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pSubjectHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pSubjectHdr=(SipSubjectHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipSubjectHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipSubjectHeader(&(pRequestHdr->pSubjectHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipSubjectHeader(pRequestHdr->pSubjectHdr,\
						(SipSubjectHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipSubjectHeader(pRequestHdr->pSubjectHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			 }
			 break;
	/* Retrans */
		case SipHdrTypeRAck:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pRackHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pRackHdr=(SipRackHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipRackHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_rpr_initSipRAckHeader(&(pRequestHdr->pRackHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_rpr_cloneSipRAckHeader(pRequestHdr->pRackHdr,\
						(SipRackHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_rpr_freeSipRAckHeader(pRequestHdr->pRackHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
	/* Retrans ends */
		case SipHdrTypeWwwAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWwwAuthenticateHeader (&wwwauth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWwwAuthenticateHeader(wwwauth,\
				(SipWwwAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)wwwauth, err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_CCP
		case SipHdrTypeAcceptContact:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slAcceptContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipAcceptContactHeader(&pAccContact, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipAcceptContactHeader(pAccContact,\
				(SipAcceptContactHeader *)(hdr->pHeader), err) == SipFail)
			 {
				sip_ccp_freeSipAcceptContactHeader(pAccContact);
				return SipFail;
			 }
			 if (sip_listInsertAt(&(pRequestHdr->slAcceptContactHdr),index,\
				(SIP_Pvoid)pAccContact, err) == SipFail)
			 {
				sip_ccp_freeSipAcceptContactHeader(pAccContact);
				return SipFail;
			 }
#endif
			 break; /* CCP case	*/
		case SipHdrTypeRejectContact:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slRejectContactHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRejectContactHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipRejectContactHeader(&pRejContact, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipRejectContactHeader(pRejContact,\
				(SipRejectContactHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_ccp_freeSipRejectContactHeader(pRejContact);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slRejectContactHdr),index,\
				(SIP_Pvoid)pRejContact, err) == SipFail)
			{
				sip_ccp_freeSipRejectContactHeader(pRejContact);
				return SipFail;
			}
#endif
			 break; /* CCP case	*/
		case SipHdrTypeRequestDisposition:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slRequestDispositionHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRequestDispositionHeader *)\
				(hdr->pHeader))->dRefCount);
#else
			if (sip_ccp_initSipRequestDispositionHeader(&pReqDisp, err)\
				== SipFail)
				return SipFail;
			if (__sip_ccp_cloneSipRequestDispositionHeader(pReqDisp, \
				(SipRequestDispositionHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_ccp_freeSipRequestDispositionHeader(pReqDisp);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slRequestDispositionHdr),index,\
				(SIP_Pvoid)pReqDisp, err) == SipFail)
			{
				sip_ccp_freeSipRequestDispositionHeader(pReqDisp);
				return SipFail;
			}
#endif			
			 break; /* CCP case	*/
#endif /* CCP */
		case SipHdrTypeAlertInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slAlertInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAlertInfoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAlertInfoHeader (&pAlertInfo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAlertInfoHeader(pAlertInfo, (SipAlertInfoHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAlertInfoHeader(pAlertInfo);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slAlertInfoHdr),index,\
				(SIP_Pvoid)pAlertInfo, err) == SipFail)
			{
				sip_freeSipAlertInfoHeader(pAlertInfo);
				return SipFail;
			}
#endif
			break;

		case SipHdrTypeInReplyTo:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slInReplyToHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipInReplyToHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipInReplyToHeader (&pInReplyTo, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipInReplyToHeader(pInReplyTo,(SipInReplyToHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipInReplyToHeader(pInReplyTo);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slInReplyToHdr),index,\
				(SIP_Pvoid)pInReplyTo, err) == SipFail)
			{
				sip_freeSipInReplyToHeader(pInReplyTo);
				return SipFail;
			}
#endif
			break;
#ifdef SIP_CONF            
	    /* Join header */
        case SipHdrTypeJoin:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pJoinHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pJoinHdr=(SipJoinHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipJoinHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipJoinHeader(&(pRequestHdr->pJoinHdr), err)\
						== SipFail)
						return SipFail;
					if (__sip_cloneSipJoinHeader(pRequestHdr->pJoinHdr,\
						(SipJoinHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipJoinHeader(pRequestHdr->pJoinHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
#endif
#ifdef SIP_3GPP
	    /* PCalledPartyId header */
        case SipHdrTypePCalledPartyId:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pRequestHdr->pPCalledPartyIdHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pRequestHdr->pPCalledPartyIdHdr=(SipPCalledPartyIdHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipPCalledPartyIdHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipPCalledPartyIdHeader(&(pRequestHdr->pPCalledPartyIdHdr), err)\
						== SipFail)
						return SipFail;
					if (__sip_cloneSipPCalledPartyIdHeader(pRequestHdr->pPCalledPartyIdHdr,\
						(SipPCalledPartyIdHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipPCalledPartyIdHeader(pRequestHdr->pPCalledPartyIdHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypePVisitedNetworkId:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slPVisitedNetworkIdHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPVisitedNetworkIdHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPVisitedNetworkIdHeader (&pPVisitedNetworkId, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipPVisitedNetworkIdHeader(pPVisitedNetworkId,(SipPVisitedNetworkIdHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPVisitedNetworkIdHeader(pPVisitedNetworkId);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slPVisitedNetworkIdHdr),index,\
				(SIP_Pvoid)pPVisitedNetworkId, err) == SipFail)
			{
				sip_freeSipPVisitedNetworkIdHeader(pPVisitedNetworkId);
				return SipFail;
			}
#endif
			break;
            
#endif
#ifdef SIP_SECURITY
                case SipHdrTypeSecurityClient:
#ifdef SIP_BY_REFERENCE
                        if (sip_listInsertAt(&(pRequestHdr->slSecurityClientHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityClientHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityClientHeader (&sec_cli, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityClientHeader(sec_cli, (SipSecurityClientHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityClientHeader(sec_cli);
                                return SipFail;
                        }
                        if (sip_listInsertAt(&(pRequestHdr->slSecurityClientHdr),index,\
                                (SIP_Pvoid)sec_cli, err) == SipFail)
                        {
                                sip_freeSipSecurityClientHeader(sec_cli);
                                return SipFail;
                        }
#endif
                        break; /* Security-Client */

		case SipHdrTypeSecurityVerify:
#ifdef SIP_BY_REFERENCE
                        if (sip_listInsertAt(&(pRequestHdr->slSecurityVerifyHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityVerifyHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityVerifyHeader (&sec_ver, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityVerifyHeader(sec_ver, (SipSecurityVerifyHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityVerifyHeader(sec_ver);
                                return SipFail;
                        }
                        if (sip_listInsertAt(&(pRequestHdr->slSecurityVerifyHdr),index,\
                                (SIP_Pvoid)sec_ver, err) == SipFail)
                        {
                                sip_freeSipSecurityVerifyHeader(sec_ver);
                                return SipFail;
                        }
#endif
                        break; /* Security-Verify */
#endif /* end of #ifdef SIP_SECURITY */ 

     
#ifdef SIP_CONGEST
		case SipHdrTypeRsrcPriority:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pRequestHdr->slRsrcPriorityHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipRsrcPriorityHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipRsrcPriorityHeader (&pRsrcPriority, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipRsrcPriorityHeader(pRsrcPriority,(SipRsrcPriorityHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipRsrcPriorityHeader(pRsrcPriority);
				return SipFail;
			}
			if (sip_listInsertAt(&(pRequestHdr->slRsrcPriorityHdr),index,\
				(SIP_Pvoid)pRsrcPriority, err) == SipFail)
			{
				sip_freeSipRsrcPriorityHeader(pRsrcPriority);
				return SipFail;
			}
#endif
			break;
            
#endif
            
		default:
#ifdef SIP_DCS
			if (sip_dcs_insertDcsRequestHeaderAtIndex (pRequestHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	} /* switch statement ends */

    if(dSecondInsertionFlag == SipSuccess)
    {
        if (setRequestHeaderAtIndex(pRequestHdr,hdr,0,err)== SipFail)
        {
            return SipFail ;
        }
        else
        {
            /* actually there is no error.
               We wanted to pass additional info without adding 
               extra parameters iso we make use of err variable*/

            *err = E_SECND_INSERTION_SINGLE_INST_HDR ;
            return SipSuccess ;
        }
    }

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function insertRequestHeaderAtIndex");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  setResponseHeaderAtIndex
**
** DESCRIPTION:  This function sets a response pHeader "hdr" at
**	the index specified by "index".
**
******************************************************************/
SipBool setResponseHeaderAtIndex
#ifdef ANSI_PROTO
	(SipRespHeader *pResponseHdr, SipHeader *hdr, SIP_U32bit index,
	 SipError *err)
#else
	(pResponseHdr, hdr, index, err)
	SipRespHeader *pResponseHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipWwwAuthenticateHeader *wwwauth;
	SipWarningHeader *warning;
	SipUnsupportedHeader *unsupp;
	SipProxyAuthenticateHeader *proxy_auth;
	SipErrorInfoHeader *errorinfo;
	SipAuthorizationHeader *auth;
	SipAuthenticationInfoHeader *auth_info;
#ifdef SIP_3GPP
	SipPAssociatedUriHeader *aUri;
#endif
#ifdef SIP_CONGEST
	SipAcceptRsrcPriorityHeader *pAcceptRsrcPriority;
#endif  
#ifdef SIP_SECURITY
	SipSecurityServerHeader *sec_ser;
#endif  
#endif
	en_SipBoolean z;
	z = SipFalse;
	SIPDEBUGFN("Entering function setResponseHeaderAtIndex");

	switch(hdr->dType)
	{
		case SipHdrTypeProxyAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slProxyAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipProxyAuthenticateHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyAuthenticateHeader (&proxy_auth, err)\
				== SipFail)
				return SipFail;
			if (__sip_cloneSipProxyAuthenticateHeader(proxy_auth,\
				(SipProxyAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipProxyAuthenticateHeader(proxy_auth);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slProxyAuthenticateHdr),index,\
				(SIP_Pvoid)proxy_auth, err) == SipFail)
			{
				sip_freeSipProxyAuthenticateHeader(proxy_auth);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAuthenticationInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slAuthenticationInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthenticationInfoHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthenticationInfoHeader (&auth_info, err)\
				== SipFail)
				return SipFail;
			if (__sip_cloneSipAuthenticationInfoHeader(auth_info,\
				(SipAuthenticationInfoHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthenticationInfoHeader(auth_info);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slAuthenticationInfoHdr),index,\
				(SIP_Pvoid)auth_info, err) == SipFail)
			{
				sip_freeSipAuthenticationInfoHeader(auth_info);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeMinExpires:
			if (index > 0)
		 	{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pResponseHdr->pMinExpiresHdr!=SIP_NULL)
					sip_freeSipMinExpiresHeader(pResponseHdr->pMinExpiresHdr);
				pResponseHdr->pMinExpiresHdr=(SipMinExpiresHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipMinExpiresHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pResponseHdr->pMinExpiresHdr == SIP_NULL)
				{
					if (sip_initSipMinExpiresHeader(&\
						(pResponseHdr->pMinExpiresHdr), err)\
						== SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipMinExpiresHeader(pResponseHdr->pMinExpiresHdr,\
					(SipMinExpiresHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipMinExpiresHeader(\
						pResponseHdr->pMinExpiresHdr);
					return SipFail;
				}
#endif
			 }
			 break;
	/* Retrans */
		case SipHdrTypeRSeq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pResponseHdr->pRSeqHdr != SIP_NULL)
					sip_rpr_freeSipRSeqHeader(pResponseHdr->pRSeqHdr);
				pResponseHdr->pRSeqHdr=(SipRseqHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipRseqHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pResponseHdr->pRSeqHdr == SIP_NULL)
				{
					if (sip_rpr_initSipRSeqHeader(&(pResponseHdr->pRSeqHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_rpr_cloneSipRSeqHeader(pResponseHdr->pRSeqHdr,\
					(SipRseqHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_rpr_freeSipRSeqHeader(pResponseHdr->pRSeqHdr);
					return SipFail;
				}
#endif
			}
			break;
	/* Retrans ends */
		case SipHdrTypeServer:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
#ifdef SIP_BY_REFERENCE
				if(pResponseHdr->pServerHdr!=SIP_NULL)
					sip_freeSipServerHeader(pResponseHdr->pServerHdr);
				pResponseHdr->pServerHdr=(SipServerHeader *)(hdr->pHeader);
				HSS_LOCKEDINCREF(((SipServerHeader *)(hdr->pHeader))->dRefCount);
#else
				if (pResponseHdr->pServerHdr == SIP_NULL)
				{
					if (sip_initSipServerHeader(&(pResponseHdr->pServerHdr),\
						err) == SipFail)
						return SipFail;
					z = SipTrue;
				}
				if (__sip_cloneSipServerHeader(pResponseHdr->pServerHdr,\
					(SipServerHeader *)(hdr->pHeader), err) == SipFail)
				{
					if (z == SipTrue)
						sip_freeSipServerHeader(pResponseHdr->pServerHdr);
					return SipFail;
				}
#endif
			}
			break;

		case SipHdrTypeUnsupported:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slUnsupportedHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipUnsupportedHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipUnsupportedHeader (&unsupp, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipUnsupportedHeader(unsupp, (SipUnsupportedHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipUnsupportedHeader(unsupp);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slUnsupportedHdr),index,\
				(SIP_Pvoid)unsupp, err) == SipFail)
			{
				sip_freeSipUnsupportedHeader(unsupp);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeWarning:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slWarningHeader),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWarningHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWarningHeader (&warning, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWarningHeader(warning, (SipWarningHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWarningHeader(warning);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slWarningHeader),index,\
				(SIP_Pvoid)warning, err) == SipFail)
			{
				sip_freeSipWarningHeader(warning);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeWwwAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWwwAuthenticateHeader (&wwwauth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWwwAuthenticateHeader(wwwauth, \
				(SipWwwAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)wwwauth, err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthorizationHeader (&auth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAuthorizationHeader(auth, \
				(SipAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)auth, err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeErrorInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slErrorInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipErrorInfoHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipErrorInfoHeader (&errorinfo, err)==SipFail)
				return SipFail;
			if (__sip_cloneSipErrorInfoHeader(errorinfo,\
				(SipErrorInfoHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipErrorInfoHeader(errorinfo);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slErrorInfoHdr),index,\
				(SIP_Pvoid)errorinfo, err) == SipFail)
			{
				sip_freeSipErrorInfoHeader(errorinfo);
				return SipFail;
			}
#endif
			break;

#ifdef SIP_3GPP
		case SipHdrTypePAssociatedUri:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slPAssociatedUriHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPAssociatedUriHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPAssociatedUriHeader (&aUri, err)==SipFail)
				return SipFail;
			if (__sip_cloneSipPAssociatedUriHeader(aUri,\
				(SipPAssociatedUriHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPAssociatedUriHeader(aUri);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slPAssociatedUriHdr),index,\
				(SIP_Pvoid)aUri, err) == SipFail)
			{
				sip_freeSipPAssociatedUriHeader(aUri);
				return SipFail;
			}
#endif
			break;
#endif

#ifdef SIP_CONGEST
		case SipHdrTypeAcceptRsrcPriority:
#ifdef SIP_BY_REFERENCE
			if (sip_listSetAt(&(pResponseHdr->slAcceptRsrcPriorityHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptRsrcPriorityHeader *) (hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptRsrcPriorityHeader (&pAcceptRsrcPriority, err)==SipFail)
				return SipFail;
			if (__sip_cloneSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority,\
				(SipAcceptRsrcPriorityHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority);
				return SipFail;
			}
			if (sip_listSetAt(&(pResponseHdr->slAcceptRsrcPriorityHdr),index,\
				(SIP_Pvoid)pAcceptRsrcPriority, err) == SipFail)
			{
				sip_freeSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority);
				return SipFail;
			}
#endif
			break;
#endif
#ifdef SIP_SECURITY
        case SipHdrTypeSecurityServer:
#ifdef SIP_BY_REFERENCE
                        if (sip_listSetAt(&(pResponseHdr->slSecurityServerHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityServerHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityServerHeader (&sec_ser, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityServerHeader(sec_ser, (SipSecurityServerHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityServerHeader(sec_ser);
                                return SipFail;
                        }
                        if (sip_listSetAt(&(pResponseHdr->slSecurityServerHdr),index,\
                                (SIP_Pvoid)sec_ser, err) == SipFail)
                        {
                                sip_freeSipSecurityServerHeader(sec_ser);
                                return SipFail;
                        }
#endif
                        break; /* for Security-Server */
#endif

		default:
#ifdef SIP_DCS
			if (sip_dcs_setDcsResponseHeaderAtIndex (pResponseHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function setResponseHeaderAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  insertResponseHeaderAtIndex
**
** DESCRIPTION: This function inserts a response pHeader "hdr" at the
**	index specified by "index".
**
******************************************************************/
SipBool insertResponseHeaderAtIndex
#ifdef ANSI_PROTO
	(SipRespHeader *pResponseHdr, SipHeader *hdr, SIP_U32bit index,\
	 SipError *err)
#else
	(pResponseHdr, hdr, index, err)
	SipRespHeader *pResponseHdr;
	SipHeader *hdr;
	SIP_U32bit index;
	SipError *err;
#endif
{
#ifndef SIP_BY_REFERENCE
	SipWwwAuthenticateHeader *wwwauth;
	SipWarningHeader *warning;
	SipUnsupportedHeader *unsupp;
	SipProxyAuthenticateHeader *proxy_auth;
	SipErrorInfoHeader *errorinfo;
	SipAuthorizationHeader *auth;
	SipAuthenticationInfoHeader *auth_info;
#ifdef SIP_3GPP
	SipPAssociatedUriHeader *aUri;
#endif
#ifdef SIP_CONGEST
	SipAcceptRsrcPriorityHeader *pAcceptRsrcPriority;
#endif  
#ifdef SIP_SECURITY
	SipSecurityServerHeader *sec_ser;
#endif  
#endif

    SipBool dSecondInsertionFlag = SipFail;

	SIPDEBUGFN("Entering function insertResponseHeaderAtIndex");

	switch(hdr->dType)
	{
		case SipHdrTypeProxyAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slProxyAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipProxyAuthenticateHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipProxyAuthenticateHeader (&proxy_auth, err) \
				== SipFail)
				return SipFail;
			if (__sip_cloneSipProxyAuthenticateHeader(proxy_auth,\
				(SipProxyAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipProxyAuthenticateHeader(proxy_auth);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slProxyAuthenticateHdr),\
				index, (SIP_Pvoid)proxy_auth, err) == SipFail)
			{
				sip_freeSipProxyAuthenticateHeader(proxy_auth);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeAuthenticationInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slAuthenticationInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthenticationInfoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthenticationInfoHeader (&auth_info, err) \
				== SipFail)
				return SipFail;
			if (__sip_cloneSipAuthenticationInfoHeader(auth_info,\
				(SipAuthenticationInfoHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthenticationInfoHeader(auth_info);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slAuthenticationInfoHdr),\
				index, (SIP_Pvoid)auth_info, err) == SipFail)
			{
				sip_freeSipAuthenticationInfoHeader(auth_info);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeMinExpires:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pMinExpiresHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pResponseHdr->pMinExpiresHdr=(SipMinExpiresHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipMinExpiresHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipMinExpiresHeader(&(pResponseHdr->\
						pMinExpiresHdr), err) == SipFail)
						return SipFail;
					if (__sip_cloneSipMinExpiresHeader(pResponseHdr->\
						pMinExpiresHdr, (SipMinExpiresHeader *)(hdr->pHeader), \
						err) == SipFail)
					{
						sip_freeSipMinExpiresHeader(\
							pResponseHdr->pMinExpiresHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
	/* Retrans */
		case SipHdrTypeRSeq:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pRSeqHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pResponseHdr->pRSeqHdr=(SipRseqHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipRseqHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_rpr_initSipRSeqHeader(&(pResponseHdr->pRSeqHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_rpr_cloneSipRSeqHeader(pResponseHdr->pRSeqHdr,\
						(SipRseqHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_rpr_freeSipRSeqHeader(pResponseHdr->pRSeqHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;
	/* Retrans ends */
		case SipHdrTypeServer:
			if (index > 0)
			{
				*err = E_INV_INDEX;
				return SipFail;
			}
			else
			{
				if (pResponseHdr->pServerHdr == SIP_NULL)
				{
#ifdef SIP_BY_REFERENCE
					pResponseHdr->pServerHdr=(SipServerHeader *)(hdr->pHeader);
					HSS_LOCKEDINCREF(((SipServerHeader *)(hdr->pHeader))->dRefCount);
#else
					if (sip_initSipServerHeader(&(pResponseHdr->pServerHdr),\
						err) == SipFail)
						return SipFail;
					if (__sip_cloneSipServerHeader(pResponseHdr->pServerHdr,\
						(SipServerHeader *)(hdr->pHeader), err) == SipFail)
					{
						sip_freeSipServerHeader(pResponseHdr->pServerHdr);
						return SipFail;
					}
#endif
				}
				else
				{
                    dSecondInsertionFlag = SipSuccess;
				}
			}
			break;

		case SipHdrTypeUnsupported:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slUnsupportedHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipUnsupportedHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipUnsupportedHeader (&unsupp, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipUnsupportedHeader(unsupp, (SipUnsupportedHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipUnsupportedHeader(unsupp);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slUnsupportedHdr),index,\
				(SIP_Pvoid)unsupp, err) == SipFail)
			{
				sip_freeSipUnsupportedHeader(unsupp);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeWarning:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slWarningHeader),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWarningHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWarningHeader (&warning, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWarningHeader(warning, (SipWarningHeader *)\
				(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWarningHeader(warning);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slWarningHeader),index,\
				(SIP_Pvoid)warning, err) == SipFail)
			{
				sip_freeSipWarningHeader(warning);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeWwwAuthenticate:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipWwwAuthenticateHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipWwwAuthenticateHeader (&wwwauth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipWwwAuthenticateHeader(wwwauth, \
				(SipWwwAuthenticateHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slWwwAuthenticateHdr),index,\
				(SIP_Pvoid)wwwauth, err) == SipFail)
			 {
				sip_freeSipWwwAuthenticateHeader(wwwauth);
				return SipFail;
			 }
#endif
			 break;
		case SipHdrTypeAuthorization:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAuthorizationHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAuthorizationHeader (&auth, err) == SipFail)
				return SipFail;
			if (__sip_cloneSipAuthorizationHeader(auth, \
				(SipAuthorizationHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slAuthorizationHdr),index,\
				 (SIP_Pvoid)auth, err) == SipFail)
			{
				sip_freeSipAuthorizationHeader(auth);
				return SipFail;
			}
#endif
			break;
		case SipHdrTypeErrorInfo:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slErrorInfoHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipErrorInfoHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipErrorInfoHeader (&errorinfo, err)==SipFail)
				return SipFail;
			if (__sip_cloneSipErrorInfoHeader(errorinfo,\
				(SipErrorInfoHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipErrorInfoHeader(errorinfo);
				return SipFail;
			}
			if (sip_listInsertAt(&(pResponseHdr->slErrorInfoHdr),\
				index, (SIP_Pvoid)errorinfo, err) == SipFail)
			{
				sip_freeSipErrorInfoHeader(errorinfo);
				return SipFail;
			}
#endif
			break;

#ifdef SIP_3GPP
		case SipHdrTypePAssociatedUri:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slPAssociatedUriHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipPAssociatedUriHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipPAssociatedUriHeader (&aUri, err)==SipFail)
				return SipFail;

            if (__sip_cloneSipPAssociatedUriHeader(aUri,\
				(SipPAssociatedUriHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipPAssociatedUriHeader(aUri);
				return SipFail;
			}

			if (sip_listInsertAt(&(pResponseHdr->slPAssociatedUriHdr),\
				index, (SIP_Pvoid)aUri, err) == SipFail)
			{
				sip_freeSipPAssociatedUriHeader(aUri);
				return SipFail;
			}
#endif
			break;
#endif

#ifdef SIP_CONGEST
		case SipHdrTypeAcceptRsrcPriority:
#ifdef SIP_BY_REFERENCE
			if (sip_listInsertAt(&(pResponseHdr->slAcceptRsrcPriorityHdr),index,\
				(SIP_Pvoid)(hdr->pHeader), err) == SipFail)
				return SipFail;
			HSS_LOCKEDINCREF(((SipAcceptRsrcPriorityHeader *)(hdr->pHeader))->dRefCount);
#else
			if (sip_initSipAcceptRsrcPriorityHeader (&pAcceptRsrcPriority, err)==SipFail)
				return SipFail;

            if (__sip_cloneSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority,\
				(SipAcceptRsrcPriorityHeader *)(hdr->pHeader), err) == SipFail)
			{
				sip_freeSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority);
				return SipFail;
			}

			if (sip_listInsertAt(&(pResponseHdr->slAcceptRsrcPriorityHdr),\
				index, (SIP_Pvoid)pAcceptRsrcPriority, err) == SipFail)
			{
				sip_freeSipAcceptRsrcPriorityHeader(pAcceptRsrcPriority);
				return SipFail;
			}
#endif
			break;
#endif
#ifdef SIP_SECURITY
                case SipHdrTypeSecurityServer:
#ifdef SIP_BY_REFERENCE
                        if (sip_listInsertAt(&(pResponseHdr->slSecurityServerHdr),index,\
                                (SIP_Pvoid)(hdr->pHeader), err) == SipFail)
                                return SipFail;
                        HSS_LOCKEDINCREF(((SipSecurityServerHeader *)(hdr->pHeader))->dRefCount);
#else
                        if (sip_initSipSecurityServerHeader (&sec_ser, err) == SipFail)
                                return SipFail;
                        if (__sip_cloneSipSecurityServerHeader(sec_ser, (SipSecurityServerHeader *)\
                                (hdr->pHeader), err) == SipFail)
                        {
                                sip_freeSipSecurityServerHeader(sec_ser);
                                return SipFail;
                        }
                        if (sip_listInsertAt(&(pResponseHdr->slSecurityServerHdr),index,\
                                (SIP_Pvoid)sec_ser, err) == SipFail)
                        {
                                sip_freeSipSecurityServerHeader(sec_ser);
                                return SipFail;
                        }
#endif
                        break; /* Security-Server */
#endif

		default:
#ifdef SIP_DCS
			if (sip_dcs_insertDcsResponseHeaderAtIndex (pResponseHdr, hdr, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif

			*err = E_INV_TYPE;
			return SipFail;
	}

    if(dSecondInsertionFlag == SipSuccess)
    {
        if (setResponseHeaderAtIndex(pResponseHdr,hdr,0,err)== SipFail)
        {
            return SipFail ;
        }
        else
        {
            /* actually there is no error.
               We wanted to pass additional info without adding 
               extra parameters iso we make use of err variable*/

            *err = E_SECND_INSERTION_SINGLE_INST_HDR ;
            return SipSuccess ;
        }
    }

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function insertResponseHeaderAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteAllGeneralHdr
**
** DESCRIPTION: This function deletes all general headers of
**	en_HeaderType "dType".
**
******************************************************************/
SipBool deleteAllGeneralHdr
#ifdef ANSI_PROTO
	(SipGeneralHeader *hdr, en_HeaderType dType, SipError *err)
#else
	(hdr, dType, err)
	SipGeneralHeader *hdr;
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIP_Pvoid contact;
	SIP_U32bit count, index;
	SIPDEBUGFN("Entering function deleteAllGeneralHdr");

	switch (dType)
	{
		case SipHdrTypeAllow:
			if (sip_listDeleteAll(&(hdr->slAllowHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeContentDisposition:
			if (sip_listDeleteAll(&(hdr->slContentDispositionHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeReason:
			if (sip_listDeleteAll(&(hdr->slReasonHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeContentLanguage:
			if (sip_listDeleteAll(&(hdr->slContentLanguageHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeAccept:
			if (sip_listDeleteAll(&(hdr->slAcceptHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAcceptEncoding:
			if (sip_listDeleteAll(&(hdr->slAcceptEncoding), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAcceptLanguage:
			if (sip_listDeleteAll(&(hdr->slAcceptLang), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeCallId:
			if (hdr->pCallidHdr != SIP_NULL)
			{
				sip_freeSipCallIdHeader (hdr->pCallidHdr);
				hdr->pCallidHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeCseq:
			if (hdr->pCseqHdr != SIP_NULL)
			{
				sip_freeSipCseqHeader (hdr->pCseqHdr);
				hdr->pCseqHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeDate:
			if (hdr->pDateHdr != SIP_NULL)
			{
				sip_freeSipDateHeader (hdr->pDateHdr);
				hdr->pDateHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeEncryption:
			if (hdr->pEncryptionHdr != SIP_NULL)
			{
				sip_freeSipEncryptionHeader (hdr->pEncryptionHdr);
				hdr->pEncryptionHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeReplyTo:
			if (hdr->pReplyToHdr != SIP_NULL)
			{
				sip_freeSipReplyToHeader (hdr->pReplyToHdr);
				hdr->pReplyToHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeExpiresSec:
			if (hdr->pExpiresHdr != SIP_NULL)
			{
				if ( ((hdr->pExpiresHdr)->dType) == SipExpSeconds )
				{
					sip_freeSipExpiresHeader(hdr->pExpiresHdr);
					hdr->pExpiresHdr = SIP_NULL;
				}
			 }
			 break;
		case SipHdrTypeExpiresDate:
			if (hdr->pExpiresHdr != SIP_NULL)
			{
				if ( ((hdr->pExpiresHdr)->dType) == SipExpDate )
				{
					sip_freeSipExpiresHeader(hdr->pExpiresHdr);
					hdr->pExpiresHdr = SIP_NULL;
				}
			}
			break;
		case SipHdrTypeExpiresAny:
			if (hdr->pExpiresHdr != SIP_NULL)
			{
				sip_freeSipExpiresHeader (hdr->pExpiresHdr);
				hdr->pExpiresHdr = SIP_NULL;
			}
			break;
#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE:
			if (hdr->pMinSEHdr != SIP_NULL)
			{
				sip_freeSipMinSEHeader (hdr->pMinSEHdr);
				hdr->pMinSEHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeSessionExpires:
			if (hdr->pSessionExpiresHdr != SIP_NULL)
			{
				sip_freeSipSessionExpiresHeader(hdr->pSessionExpiresHdr);
				hdr->pSessionExpiresHdr = SIP_NULL;
			}
			break;
#endif

		case SipHdrTypeFrom:
			if (hdr->pFromHeader != SIP_NULL)
			{
				sip_freeSipFromHeader (hdr->pFromHeader);
				hdr->pFromHeader = SIP_NULL;
			}
			break;
		case SipHdrTypeRecordRoute:
			if (sip_listDeleteAll(&(hdr->slRecordRouteHdr), err) == SipFail)
				return SipFail;
			break;
#ifdef SIP_3GPP
		case SipHdrTypeServiceRoute:
			if (sip_listDeleteAll(&(hdr->slServiceRouteHdr), err) == SipFail)
					return SipFail;
			break;
		case SipHdrTypePath:
			if (sip_listDeleteAll(&(hdr->slPathHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypePanInfo:
			if (hdr->pPanInfoHdr != SIP_NULL)
			{
				sip_freeSipPanInfoHeader (hdr->pPanInfoHdr);
				hdr->pPanInfoHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePcVector:
			if (hdr->pPcVectorHdr != SIP_NULL)
			{
				sip_freeSipPcVectorHeader (hdr->pPcVectorHdr);
				hdr->pPcVectorHdr = SIP_NULL;
			}
			break;
			
#endif
		case SipHdrTypeTimestamp:
			if (hdr->pTimeStampHdr != SIP_NULL)
			{
				sip_freeSipTimeStampHeader (hdr->pTimeStampHdr);
				hdr->pTimeStampHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeTo:
			if (hdr->pToHdr != SIP_NULL)
			{
				sip_freeSipToHeader (hdr->pToHdr);
				hdr->pToHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeVia:
			if (sip_listDeleteAll(&(hdr->slViaHdr), err) == SipFail)
				return SipFail;
			 break;
		case SipHdrTypeContentEncoding:
			if (sip_listDeleteAll(&(hdr->slContentEncodingHdr), err) == SipFail)
				return SipFail;
			break;
	/* RPR */
		case SipHdrTypeSupported:
			if (sip_listDeleteAll(&(hdr->slSupportedHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeContentLength:
			if (hdr->pContentLengthHdr != SIP_NULL)
			{
				sip_freeSipContentLengthHeader (hdr->pContentLengthHdr);
				hdr->pContentLengthHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeContentType:
			if (hdr->pContentTypeHdr != SIP_NULL)
			{
				sip_freeSipContentTypeHeader (hdr->pContentTypeHdr);
				hdr->pContentTypeHdr = SIP_NULL;
			}
			break;
/* bcpt ext */
		case SipHdrTypeMimeVersion:
			if (hdr->pMimeVersionHdr != SIP_NULL)
			{
				sip_bcpt_freeSipMimeVersionHeader (hdr->pMimeVersionHdr);
				hdr->pMimeVersionHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeOrganization:
			if (hdr->pOrganizationHdr != SIP_NULL)
			{
				sip_freeSipOrganizationHeader(hdr->pOrganizationHdr);
				hdr->pOrganizationHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeContactWildCard:
			if (sip_listSizeOf(&(hdr->slContactHdr), &count, err) == SipFail)
				return SipFail;
			for (index=0; index<count; index++)
			{
				if (sip_listGetAt(&(hdr->slContactHdr), index, &contact, err)\
					== SipFail)
					return SipFail;
				if ( ((SipContactHeader *)contact)->dType == \
					SipContactWildCard)
				{
					if (sip_listDeleteAt(&(hdr->slContactHdr), index, err)\
						== SipFail)
				    {
						contact = SIP_NULL;
						return SipFail;
					}
				}

			}
			break;
		case SipHdrTypeContactNormal:
			if (sip_listSizeOf(&(hdr->slContactHdr), &count, err) == SipFail)
				return SipFail;
			for (index=0; index<count; index++)
			{
				if (sip_listGetAt(&(hdr->slContactHdr), index, &contact, err)\
					== SipFail)
					return SipFail;
				if (((SipContactHeader *)contact)->dType == \
					SipContactNormal)
				{
					if (sip_listDeleteAt(&(hdr->slContactHdr), index, err)\
						== SipFail)
				    {
						contact = SIP_NULL;
						return SipFail;
					}
				}
			}
			break;

		case SipHdrTypeContactAny:
			if (sip_listDeleteAll(&(hdr->slContactHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeRequire:
			if (sip_listDeleteAll(&(hdr->slRequireHdr), err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeCallInfo:
			if (sip_listDeleteAll(&(hdr->slCallInfoHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeUserAgent:
			if (hdr->pUserAgentHdr != SIP_NULL)
			{
				sip_freeSipUserAgentHeader(hdr->pUserAgentHdr);
				hdr->pUserAgentHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeRetryAfterDate:
			if (hdr->pRetryAfterHdr != SIP_NULL)
			{
				if ((hdr->pRetryAfterHdr)->dType == SipExpDate)
				{
					sip_freeSipRetryAfterHeader(hdr->pRetryAfterHdr);
					hdr->pRetryAfterHdr = SIP_NULL;
				}
			}
			break;
		case SipHdrTypeRetryAfterSec:
			if (hdr->pRetryAfterHdr != SIP_NULL)
			{
				if ((hdr->pRetryAfterHdr)->dType == SipExpSeconds)
				{
					sip_freeSipRetryAfterHeader(hdr->pRetryAfterHdr);
					hdr->pRetryAfterHdr = SIP_NULL;
				}
			}
			break;
#ifdef SIP_PRIVACY
		case SipHdrTypePrivacy:
			if (hdr->pPrivacyHdr != SIP_NULL)
			{
				sip_freeSipPrivacyHeader(hdr->pPrivacyHdr);
				hdr->pPrivacyHdr = SIP_NULL;
			}
		    break;
#endif

		case SipHdrTypeRetryAfterAny:
			if (hdr->pRetryAfterHdr != SIP_NULL)
			{
				sip_freeSipRetryAfterHeader(hdr->pRetryAfterHdr);
				hdr->pRetryAfterHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeUnknown:
			if (sip_listDeleteAll(&(hdr->slUnknownHdr), err) == SipFail)
				return SipFail;
			break;
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
			if (sip_listDeleteAll(&(hdr->slAllowEventsHdr), err) == SipFail)
				return SipFail;
			break;
#endif

#ifdef SIP_3GPP
		case SipHdrTypePcfAddr:
			if (hdr->pPcfAddrHdr != SIP_NULL)
			{
				sip_freeSipPcfAddrHeader(hdr->pPcfAddrHdr);
				hdr->pPcfAddrHdr = SIP_NULL;
			}
		    break;
#endif

            
		default:
#ifdef SIP_DCS
			if (sip_dcs_deleteAllDcsGeneralHeaderByType (hdr, dType, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteAllGeneralHdr");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteAllRequestHdr
**
** DESCRIPTION:  This function deletes all request headers of
**	en_HeaderType "dType".
**
******************************************************************/
SipBool deleteAllRequestHdr
#ifdef ANSI_PROTO
	(SipReqHeader *hdr, en_HeaderType dType, SipError *err)
#else
	(hdr, dType, err)
	SipReqHeader *hdr;
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function deleteAllRequestHdr");

	switch (dType)
	{
		case SipHdrTypeAuthorization:
			if (sip_listDeleteAll(&(hdr->slAuthorizationHdr),err)==SipFail)
				return SipFail;
			break;

			/* call-transfer */
		case SipHdrTypeReferTo:
			if (hdr->pReferToHdr != SIP_NULL)
			{
				sip_freeSipReferToHeader(hdr->pReferToHdr);
				hdr->pReferToHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeReferredBy:
			if (hdr->pReferredByHdr != SIP_NULL)
			{
				sip_freeSipReferredByHeader(hdr->pReferredByHdr);
				hdr->pReferredByHdr = SIP_NULL;
			}
			break;	/* call-transfer */
#ifdef SIP_IMPP
		case SipHdrTypeEvent:
			if (hdr->pEventHdr != SIP_NULL)
			{
				sip_impp_freeSipEventHeader (hdr->pEventHdr);
				hdr->pEventHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeSubscriptionState:
			if (hdr->pSubscriptionStateHdr != SIP_NULL)
			{
				sip_impp_freeSipSubscriptionStateHeader(hdr-> \
				pSubscriptionStateHdr);
				hdr->pSubscriptionStateHdr = SIP_NULL;
			}
			break;
#endif
		case SipHdrTypeHide:
			if (hdr->pHideHdr != SIP_NULL)
			{
				sip_freeSipHideHeader(hdr->pHideHdr);
				hdr->pHideHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeReplaces:
			if (hdr->pReplacesHdr != SIP_NULL)
			{
				sip_freeSipReplacesHeader(hdr->pReplacesHdr);
				hdr->pReplacesHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeMaxforwards:
			if (hdr->pMaxForwardsHdr != SIP_NULL)
			{
				sip_freeSipMaxForwardsHeader(hdr->pMaxForwardsHdr);
				hdr->pMaxForwardsHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePriority:
			if (hdr->pPriorityHdr != SIP_NULL)
			{
				sip_freeSipPriorityHeader(hdr->pPriorityHdr);
				hdr->pPriorityHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeProxyauthorization:
			if (sip_listDeleteAll(&(hdr->slProxyAuthorizationHdr),err)==SipFail)
				return SipFail;
			break;
		case SipHdrTypeProxyRequire:
			if (sip_listDeleteAll(&(hdr->slProxyRequireHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeRoute:
			if (sip_listDeleteAll(&(hdr->slRouteHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAlso:
			if (sip_listDeleteAll(&(hdr->slAlsoHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeResponseKey:
			if (hdr->pRespKeyHdr != SIP_NULL)
			{
				sip_freeSipRespKeyHeader(hdr->pRespKeyHdr);
				hdr->pRespKeyHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeSubject:
			if (hdr->pSubjectHdr != SIP_NULL)
			{
				sip_freeSipSubjectHeader(hdr->pSubjectHdr);
				hdr->pSubjectHdr = SIP_NULL;
			}
			break;
	/* Retrans */
		case SipHdrTypeRAck:
			if (hdr->pRackHdr != SIP_NULL)
			{
				sip_rpr_freeSipRAckHeader(hdr->pRackHdr);
				hdr->pRackHdr = SIP_NULL;
			}
			break;
	/* Retrans ends */
		case SipHdrTypeWwwAuthenticate:
			if (sip_listDeleteAll(&(hdr->slWwwAuthenticateHdr), err) == SipFail)
				return SipFail;
			break;
#ifdef 	SIP_CCP	
		case SipHdrTypeAcceptContact:
			if (sip_listDeleteAll(&(hdr->slAcceptContactHdr), err) == SipFail)
				return SipFail;
			break; /* CCP case */
		case SipHdrTypeRejectContact:
			if (sip_listDeleteAll(&(hdr->slRejectContactHdr), err) == SipFail)
				return SipFail;
			break; /* CCP case */
		case SipHdrTypeRequestDisposition:
			if (sip_listDeleteAll(&(hdr->slRequestDispositionHdr), err) == SipFail)
				return SipFail;
			break; /* CCP case */
#endif			
		case SipHdrTypeAlertInfo:
			if (sip_listDeleteAll(&(hdr->slAlertInfoHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeInReplyTo:
			if (sip_listDeleteAll(&(hdr->slInReplyToHdr), err) == SipFail)
				return SipFail;
			break;
#ifdef SIP_CONF            
		/* Join header */
        case SipHdrTypeJoin:
			if (hdr->pJoinHdr != SIP_NULL)
			{
				sip_freeSipJoinHeader(hdr->pJoinHdr);
				hdr->pJoinHdr = SIP_NULL;
			}
			break;
#endif
#ifdef SIP_SECURITY
		case SipHdrTypeSecurityClient:
			if (sip_listDeleteAll(&(hdr->slSecurityClientHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeSecurityVerify:
                        if (sip_listDeleteAll(&(hdr->slSecurityVerifyHdr), err) == SipFail)
                                return SipFail;
                        break;
#endif

#ifdef SIP_3GPP
       	/* PCalledPartyId header */
        case SipHdrTypePCalledPartyId:
			if (hdr->pPCalledPartyIdHdr != SIP_NULL)
			{
				sip_freeSipPCalledPartyIdHeader(hdr->pPCalledPartyIdHdr);
				hdr->pPCalledPartyIdHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePVisitedNetworkId:
			if (sip_listDeleteAll(&(hdr->slPVisitedNetworkIdHdr), err) == SipFail)
				return SipFail;
			break;
            
#endif

#ifdef SIP_CONGEST
       	/* RsrcPriority header */
        case SipHdrTypeRsrcPriority:
			if (sip_listDeleteAll(&(hdr->slRsrcPriorityHdr), err) == SipFail)
				return SipFail;
			break;
#endif
            
		default:
#ifdef SIP_DCS
			if (sip_dcs_deleteAllDcsRequestHeaderByType (hdr, dType, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteAllRequestHdr");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteAllResponseHdr
**
** DESCRIPTION: This function deletes all response headers of en_HeaderType "dType".
**
******************************************************************/
SipBool deleteAllResponseHdr
#ifdef ANSI_PROTO
	(SipRespHeader *hdr, en_HeaderType dType, SipError *err)
#else
	(hdr, dType, err)
	SipRespHeader *hdr;
	en_HeaderType dType;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function deleteAllResponseHdr");
	switch(dType)
	{
		case SipHdrTypeProxyAuthenticate:
			if (sip_listDeleteAll(&(hdr->slProxyAuthenticateHdr), err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeAuthenticationInfo:
			if (sip_listDeleteAll(&(hdr->slAuthenticationInfoHdr), err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeMinExpires:
			if (hdr->pMinExpiresHdr != SIP_NULL)
			{
				sip_freeSipMinExpiresHeader(hdr->pMinExpiresHdr);
				hdr->pMinExpiresHdr = SIP_NULL;
			}
			break;
		/* Retrans */
		case SipHdrTypeRSeq:
			if (hdr->pRSeqHdr != SIP_NULL)
			{
					sip_rpr_freeSipRSeqHeader(hdr->pRSeqHdr);
					hdr->pRSeqHdr = SIP_NULL;
			}
			break;
		/* retrans ends */
		case SipHdrTypeServer:
			/* if (sip_listDeleteAll(&(hdr->slServerHdr), err) == SipFail)
				return SipFail; */
			if (hdr->pServerHdr != SIP_NULL)
			{
					sip_freeSipServerHeader(hdr->pServerHdr);
					hdr->pServerHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeUnsupported:
			if (sip_listDeleteAll(&(hdr->slUnsupportedHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeWarning:
			if (sip_listDeleteAll(&(hdr->slWarningHeader), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeWwwAuthenticate:
			if (sip_listDeleteAll(&(hdr->slWwwAuthenticateHdr), err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAuthorization:
			if (sip_listDeleteAll(&(hdr->slAuthorizationHdr), err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeErrorInfo:
			if (sip_listDeleteAll(&(hdr->slErrorInfoHdr), err)\
				== SipFail)
				return SipFail;
			break;

#ifdef SIP_3GPP
   		case SipHdrTypePAssociatedUri:
			if (sip_listDeleteAll(&(hdr->slPAssociatedUriHdr), err)\
				== SipFail)
				return SipFail;
			break;
#endif

#ifdef SIP_CONGEST
   		case SipHdrTypeAcceptRsrcPriority:
			if (sip_listDeleteAll(&(hdr->slAcceptRsrcPriorityHdr), err)\
				== SipFail)
				return SipFail;
			break;
#endif
            
#ifdef SIP_SECURITY
                case SipHdrTypeSecurityServer:
                        if (sip_listDeleteAll(&(hdr->slSecurityServerHdr), err) == SipFail)
                                return SipFail;
                        break;
#endif

		default:
#ifdef SIP_DCS
			if ( sip_dcs_deleteAllDcsResponseHeaderByType (hdr, dType, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteAllResponseHdr");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteGeneralHdrAtIndex
**
** DESCRIPTION: This function deletes a General pHeader of dType "dType"
**	at the index "index".
**
******************************************************************/
SipBool deleteGeneralHdrAtIndex
#ifdef ANSI_PROTO
	(SipGeneralHeader *hdr, en_HeaderType dType, SIP_U32bit index, \
	 SipError *err)
#else
	(hdr, dType, index, err)
	SipGeneralHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function deleteGeneralHdrAtIndex");
	switch(dType)
	{
		case SipHdrTypeAllow:
			if (sip_listDeleteAt (&(hdr->slAllowHdr), index, err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeContentDisposition:
			if (sip_listDeleteAt (&(hdr->slContentDispositionHdr), index, err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeReason:
			if (sip_listDeleteAt (&(hdr->slReasonHdr), index, err) == SipFail)
				return SipFail;
			break;

		case SipHdrTypeContentLanguage:
			if (sip_listDeleteAt (&(hdr->slContentLanguageHdr), index, err) == SipFail)
				return SipFail;
			break;


		case SipHdrTypeAccept:
			if (sip_listDeleteAt (&(hdr->slAcceptHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAcceptEncoding:
			if (sip_listDeleteAt (&(hdr->slAcceptEncoding), index, err) \
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeAcceptLanguage:
			if (sip_listDeleteAt (&(hdr->slAcceptLang), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeCallId:
			if (hdr->pCallidHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			 }
			 else
			 {
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipCallIdHeader (hdr->pCallidHdr);
			 	hdr->pCallidHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeCseq:
			if (hdr->pCseqHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipCseqHeader (hdr->pCseqHdr);
			 	hdr->pCseqHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeDate:
			if (hdr->pDateHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipDateHeader (hdr->pDateHdr);
			 	hdr->pDateHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeEncryption:
			if (hdr->pEncryptionHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipEncryptionHeader (hdr->pEncryptionHdr);
			 	hdr->pEncryptionHdr = SIP_NULL;
			}
			break;
			
		case SipHdrTypeReplyTo:
			if (hdr->pReplyToHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipReplyToHeader (hdr->pReplyToHdr);
			 	hdr->pReplyToHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeExpiresAny:
			if (hdr->pExpiresHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipExpiresHeader (hdr->pExpiresHdr);
			 	hdr->pExpiresHdr = SIP_NULL;
			}
			break;

#ifdef SIP_SESSIONTIMER
		case SipHdrTypeMinSE:
			if (hdr->pMinSEHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			 }
			 else
			 {
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipMinSEHeader (hdr->pMinSEHdr);
			 	hdr->pMinSEHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeSessionExpires:
			if (hdr->pSessionExpiresHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipSessionExpiresHeader (hdr->pSessionExpiresHdr);
			 	hdr->pSessionExpiresHdr = SIP_NULL;
			}
			break;
#endif

		case SipHdrTypeFrom:
			if (hdr->pFromHeader == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipFromHeader (hdr->pFromHeader);
			 	hdr->pFromHeader = SIP_NULL;
			}
			break;
		case SipHdrTypeRecordRoute:
			if (sip_listDeleteAt (&(hdr->slRecordRouteHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;
#ifdef SIP_3GPP
		case SipHdrTypePath:
			if (sip_listDeleteAt (&(hdr->slPathHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;

		case SipHdrTypeServiceRoute:
			if (sip_listDeleteAt (&(hdr->slServiceRouteHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;

		case SipHdrTypePanInfo:
			if (hdr->pPanInfoHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPanInfoHeader (hdr->pPanInfoHdr);
			 	hdr->pPanInfoHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePcVector:
			if (hdr->pPcVectorHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPcVectorHeader (hdr->pPcVectorHdr);
			 	hdr->pPcVectorHdr = SIP_NULL;
			}
			break;
	
#endif
		case SipHdrTypeTimestamp:
			if (hdr->pTimeStampHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipTimeStampHeader (hdr->pTimeStampHdr);
			 	hdr->pTimeStampHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeTo:
			if (hdr->pToHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipToHeader (hdr->pToHdr);
			 	hdr->pToHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeVia:
			if (sip_listDeleteAt (&(hdr->slViaHdr), index, err) == SipFail)
				return SipFail;
			break;

#ifdef SIP_PRIVACY
		case SipHdrTypePAssertId:
			if (sip_listDeleteAt (&(hdr->slPAssertIdHdr), index, err) \
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypePPreferredId:
			if (sip_listDeleteAt (&(hdr->slPPreferredIdHdr),index, err) \
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypePrivacy:
			if(hdr->pPrivacyHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if(index > 0)
				{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPrivacyHeader (hdr->pPrivacyHdr);
			 	hdr->pPrivacyHdr = SIP_NULL;
			}
		   break;

				
#endif

		case SipHdrTypeContentEncoding:
			if (sip_listDeleteAt (&(hdr->slContentEncodingHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
	/* RPR */
		case SipHdrTypeSupported:
			if (sip_listDeleteAt (&(hdr->slSupportedHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeContentLength:
			if (hdr->pContentLengthHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipContentLengthHeader (hdr->pContentLengthHdr);
			 	hdr->pContentLengthHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeContentType:
			if (hdr->pContentTypeHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipContentTypeHeader (hdr->pContentTypeHdr);
			 	hdr->pContentTypeHdr = SIP_NULL;
			}
			break;
/* bcpt ext */
		case SipHdrTypeMimeVersion:
			if (hdr->pMimeVersionHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_bcpt_freeSipMimeVersionHeader (hdr->pMimeVersionHdr);
			 	hdr->pMimeVersionHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeContactAny:
			if (sip_listDeleteAt (&(hdr->slContactHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeOrganization:
			if (hdr->pOrganizationHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipOrganizationHeader (hdr->pOrganizationHdr);
			 	hdr->pOrganizationHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeRequire:
			if (sip_listDeleteAt (&(hdr->slRequireHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeCallInfo:
			if (sip_listDeleteAt (&(hdr->slCallInfoHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeUserAgent:
			if (hdr->pUserAgentHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipUserAgentHeader (hdr->pUserAgentHdr);
			 	hdr->pUserAgentHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeRetryAfterAny:
			if (hdr->pRetryAfterHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipRetryAfterHeader (hdr->pRetryAfterHdr);
			 	hdr->pRetryAfterHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeUnknown:
			if (sip_listDeleteAt (&(hdr->slUnknownHdr), index, err) == SipFail)
				return SipFail;
			break;
#ifdef SIP_IMPP
		case SipHdrTypeAllowEvents:
			if (sip_listDeleteAt (&(hdr->slAllowEventsHdr), index, err) \
				== SipFail)
				return SipFail;
			break;
#endif
#ifdef SIP_3GPP
		case SipHdrTypePcfAddr:
			if (hdr->pPcfAddrHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPcfAddrHeader (hdr->pPcfAddrHdr);
			 	hdr->pPcfAddrHdr = SIP_NULL;
			}
			break;

#endif

		default:
#ifdef SIP_DCS
			if (sip_dcs_deleteDcsGeneralHeaderAtIndex (hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteGeneralHdrAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteRequestHdrAtIndex
**
** DESCRIPTION: This function deletes a request pHeader of dType
**	"dType" at the index "index".
**
******************************************************************/
SipBool deleteRequestHdrAtIndex
#ifdef ANSI_PROTO
	(SipReqHeader *hdr, en_HeaderType dType, SIP_U32bit index, SipError *err)
#else
	(hdr, dType, index, err)
	SipReqHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function deleteRequestHdrAtIndex");

	switch(dType)
	{
		case SipHdrTypeAuthorization:
			if (sip_listDeleteAt (&(hdr->slAuthorizationHdr), index, err)\
				== SipFail)
				return SipFail;
			break;

/* call-transfer */

		case SipHdrTypeReferTo:
			if (hdr->pReferToHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipReferToHeader (hdr->pReferToHdr);
			 	hdr->pReferToHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeReferredBy:
			if (hdr->pReferredByHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipReferredByHeader (hdr->pReferredByHdr);
			 	hdr->pReferredByHdr = SIP_NULL;
			}
			break;
#ifdef SIP_IMPP
		case SipHdrTypeEvent:
			if (hdr->pEventHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			 }
			 else
			 {
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_impp_freeSipEventHeader (hdr->pEventHdr);
			 	hdr->pEventHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeSubscriptionState:
			if (hdr->pSubscriptionStateHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_impp_freeSipSubscriptionStateHeader (hdr->\
				pSubscriptionStateHdr);
			 	hdr->pSubscriptionStateHdr = SIP_NULL;
			}
			break;
#endif
		case SipHdrTypeHide:
			if (hdr->pHideHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipHideHeader (hdr->pHideHdr);
			 	hdr->pHideHdr = SIP_NULL;
			}
			break;

	case SipHdrTypeReplaces:
			if (hdr->pReplacesHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipReplacesHeader (hdr->pReplacesHdr);
			 	hdr->pReplacesHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeMaxforwards:
			if (hdr->pMaxForwardsHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipMaxForwardsHeader (hdr->pMaxForwardsHdr);
			 	hdr->pMaxForwardsHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePriority:
			if (hdr->pPriorityHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPriorityHeader (hdr->pPriorityHdr);
			 	hdr->pPriorityHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeProxyauthorization:
			if (sip_listDeleteAt (&(hdr->slProxyAuthorizationHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeRoute:
			if (sip_listDeleteAt (&(hdr->slRouteHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeAlso:
			if (sip_listDeleteAt (&(hdr->slAlsoHdr), index, err) == SipFail)
				return SipFail;
			break;
		case SipHdrTypeResponseKey:
			if (hdr->pRespKeyHdr == SIP_NULL)
		 	{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipRespKeyHeader (hdr->pRespKeyHdr);
			 	hdr->pRespKeyHdr = SIP_NULL;
			}
			break;
		case SipHdrTypeSubject:
			if (hdr->pSubjectHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipSubjectHeader (hdr->pSubjectHdr);
			 	hdr->pSubjectHdr = SIP_NULL;
			 }
			 break;
	/* Retrans */
		case SipHdrTypeRAck:
			if (hdr->pRackHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_rpr_freeSipRAckHeader (hdr->pRackHdr);
			 	hdr->pRackHdr = SIP_NULL;
			}
			break;
	/* Retrans ends */
		case SipHdrTypeWwwAuthenticate:
			if (sip_listDeleteAt (&(hdr->slWwwAuthenticateHdr), index, err)\
				== SipFail)
				return SipFail;
			 break;
		case SipHdrTypeProxyRequire:
			if (sip_listDeleteAt (&(hdr->slProxyRequireHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
#ifdef SIP_CCP			
		case SipHdrTypeAcceptContact:
			if (sip_listDeleteAt (&(hdr->slAcceptContactHdr), index, err)\
				== SipFail)
				return SipFail;
			break; /* CCP case */
		case SipHdrTypeRejectContact:
			if (sip_listDeleteAt (&(hdr->slRejectContactHdr), index, err)\
				== SipFail)
				return SipFail;
			break; /* CCP case */
		case SipHdrTypeRequestDisposition:
			if (sip_listDeleteAt (&(hdr->slRequestDispositionHdr), index, err)\
				== SipFail)
				return SipFail;
			break; /* CCP case */
#endif			
		case SipHdrTypeAlertInfo:
			if (sip_listDeleteAt (&(hdr->slAlertInfoHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;
		case SipHdrTypeInReplyTo:
			if (sip_listDeleteAt (&(hdr->slInReplyToHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;
#ifdef SIP_CONF            
	/* Join header*/
        case SipHdrTypeJoin:
			if (hdr->pJoinHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipJoinHeader (hdr->pJoinHdr);
			 	hdr->pJoinHdr = SIP_NULL;
			}
			break;
#endif
#ifdef SIP_SECURITY
	case SipHdrTypeSecurityClient:
                        if (sip_listDeleteAt (&(hdr->slSecurityClientHdr), index, err) == SipFail)
                                return SipFail;
                        break;

	case SipHdrTypeSecurityVerify:
                        if (sip_listDeleteAt (&(hdr->slSecurityVerifyHdr), index, err) == SipFail)
                                return SipFail;
                        break;
#endif

#ifdef SIP_3GPP
	/* PCalledPartyId header*/
        case SipHdrTypePCalledPartyId:
			if (hdr->pPCalledPartyIdHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipPCalledPartyIdHeader (hdr->pPCalledPartyIdHdr);
			 	hdr->pPCalledPartyIdHdr = SIP_NULL;
			}
			break;
		case SipHdrTypePVisitedNetworkId:
			if (sip_listDeleteAt (&(hdr->slPVisitedNetworkIdHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;            
#endif

#ifdef SIP_CONGEST
		case SipHdrTypeRsrcPriority:
			if (sip_listDeleteAt (&(hdr->slRsrcPriorityHdr), index, err)\
				 == SipFail)
				return SipFail;
			break;
#endif
            
		default:
#ifdef SIP_DCS
			if (sip_dcs_deleteDcsRequestHeaderAtIndex (hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
		 	return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteRequestHdrAtIndex");
	return SipSuccess;
}

/******************************************************************
**
** FUNCTION:  deleteResponseHdrAtIndex
**
** DESCRIPTION:  This function deletes a response Header of Type
**	"dType" at the index "index".
**
******************************************************************/
SipBool deleteResponseHdrAtIndex
#ifdef ANSI_PROTO
	(SipRespHeader *hdr, en_HeaderType dType, SIP_U32bit index, SipError *err)
#else
	(hdr, dType, index, err)
	SipRespHeader *hdr;
	en_HeaderType dType;
	SIP_U32bit index;
	SipError *err;
#endif
{
	SIPDEBUGFN("Entering function deleteResponseHdrAtIndex");

	switch(dType)
	{
		case SipHdrTypeProxyAuthenticate:
			if (sip_listDeleteAt (&(hdr->slProxyAuthenticateHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeAuthenticationInfo:
			if (sip_listDeleteAt (&(hdr->slAuthenticationInfoHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeMinExpires:
			if (hdr->pMinExpiresHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipMinExpiresHeader (hdr->pMinExpiresHdr);
			 	hdr->pMinExpiresHdr = SIP_NULL;
			}
			break; 
	/* Retrans */
		case SipHdrTypeRSeq:
			if (hdr->pRSeqHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_rpr_freeSipRSeqHeader (hdr->pRSeqHdr);
			 	hdr->pRSeqHdr = SIP_NULL;
			}
			break;
		/* Retrans ends */
		case SipHdrTypeServer:
			/* if (sip_listDeleteAt (&(hdr->slServerHdr), index, err) == SipFail)
				return SipFail; */
			if (hdr->pServerHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
			 	if (index > 0)
			 	{
					*err = E_INV_INDEX;
					return SipFail;
			 	}
				sip_freeSipServerHeader (hdr->pServerHdr);
			 	hdr->pServerHdr = SIP_NULL;
			}
			break;

		case SipHdrTypeUnsupported:
			if (sip_listDeleteAt (&(hdr->slUnsupportedHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeWarning:
			if (sip_listDeleteAt (&(hdr->slWarningHeader), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeWwwAuthenticate:
			if (sip_listDeleteAt (&(hdr->slWwwAuthenticateHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeAuthorization:
			if (sip_listDeleteAt (&(hdr->slAuthorizationHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
		case SipHdrTypeErrorInfo:
			if (sip_listDeleteAt (&(hdr->slErrorInfoHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
#ifdef SIP_3GPP
   		case SipHdrTypePAssociatedUri:
			if (sip_listDeleteAt (&(hdr->slPAssociatedUriHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
#endif

#ifdef SIP_CONGEST
   		case SipHdrTypeAcceptRsrcPriority:
			if (sip_listDeleteAt (&(hdr->slAcceptRsrcPriorityHdr), index, err)\
				== SipFail)
				return SipFail;
			break;
#endif
        
#ifdef SIP_SECURITY
        	case SipHdrTypeSecurityServer:
                        if (sip_listDeleteAt (&(hdr->slSecurityServerHdr), index, err) == SipFail)
                                return SipFail;
                        break;
#endif
     
		default:
#ifdef SIP_DCS
			if (sip_dcs_deleteDcsResponseHeaderAtIndex (hdr, dType, index, err) == SipSuccess)
				break;
			if (*err != E_INV_TYPE)
				return SipFail;
#endif
			*err = E_INV_TYPE;
			return SipFail;
	}

	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function deleteResponseHdrAtIndex");
	return SipSuccess;
}




/*****************************************************************************
** FUNCTION:sip_equateTypeInSipHeader
**
** DESCRIPTION: Makes the type of the header structure consistent with
** with the type of the encapsulated header structure.
**
******************************************************************************/
void sip_equateTypeInSipHeader
#ifdef ANSI_PROTO
	(SipHeader *hdr)
#else
	(hdr)
	SipHeader *hdr;
#endif
{
	switch(hdr->dType)
	{
		case SipHdrTypeContactAny:
			if(((SipContactHeader *)(hdr->pHeader))->dType == SipContactNormal)
				hdr->dType = SipHdrTypeContactNormal;
			else if(((SipContactHeader *)(hdr->pHeader))->dType\
					== SipContactWildCard)
					hdr->dType = SipHdrTypeContactWildCard;
			break;
		case SipHdrTypeRetryAfterAny:
			if(((SipRetryAfterHeader *)(hdr->pHeader))->dType == SipExpDate)
				hdr->dType = SipHdrTypeRetryAfterDate;
			else if(((SipRetryAfterHeader *)(hdr->pHeader))->dType\
					== SipExpSeconds)
					hdr->dType = SipHdrTypeRetryAfterSec;
			break;
		case SipHdrTypeExpiresAny:
			if(((SipExpiresHeader *)(hdr->pHeader))->dType == SipExpDate)
				hdr->dType = SipHdrTypeExpiresDate;
			else if(((SipExpiresHeader *)(hdr->pHeader))->dType\
				== SipExpSeconds)
				hdr->dType = SipHdrTypeExpiresSec;
			break;
		default:
			break;
	}
}


/******************************************************************
**
** FUNCTION:  __sip_deleteHeaderAtIndex
**
** DESCRIPTION: Internal function to delete a pHeader at Index
**
******************************************************************/
SipBool __sip_deleteHeaderAtIndex
#ifdef ANSI_PROTO
	(SipMessage *msg, en_HeaderType dType, SIP_U32bit index, SipError *err)
#else
	(msg, dType, index, err)
	SipMessage *msg;
	en_HeaderType dType;
	SIP_U32bit index;
	SipError *err;
#endif
{

	SIPDEBUGFN("Entering function __sip_deleteHeaderAt");

	if (validateSipHeaderType (dType, err) == SipFail)
		return SipFail;

	switch (msg->dType)
	{
		case SipMessageRequest:
			if ((msg->u).pRequest == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			/*  No general header in request header
			if (isSipGeneralHeader(dType) == SipTrue)
			{
				if (((msg->u).pRequest)->pGeneralHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
					if (deleteGeneralHdrAtIndex(((msg->u).pRequest)->\
						pGeneralHdr, dType, index, err) == SipFail)
					return SipFail;
				}
			}
			*/
			if (isSipReqHeader(dType) == SipTrue)
			{
				if (((msg->u).pRequest)->pRequestHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
					return SipFail;
				}
				else
				{
					if (deleteRequestHdrAtIndex(((msg->u).pRequest)->\
						pRequestHdr, dType, index, err) == SipFail)
						return SipFail;
					}
			}
			else
		  	{
				*err = E_INV_TYPE;
				return SipFail;
			}
			break;
		case SipMessageResponse:
			if ((msg->u).pResponse == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			/* Response header does not have general header anymore
			if (isSipGeneralHeader(dType) == SipTrue)
			{
				if (((msg->u).pResponse)->pGeneralHdr == SIP_NULL)
				{
					*err = E_NO_EXIST;
				return SipFail;
				}
				else
				{
					if (deleteGeneralHdrAtIndex(((msg->u).pResponse)->pGeneralHdr,\
						dType, index, err) == SipFail)
						return SipFail;
				}
			}
		*/
		if (isSipRespHeader(dType) == SipTrue)
		{
			if (((msg->u).pResponse)->pResponseHdr == SIP_NULL)
			{
				*err = E_NO_EXIST;
				return SipFail;
			}
			else
			{
				if (deleteResponseHdrAtIndex(((msg->u).pResponse)->\
						pResponseHdr, dType, index, err) == SipFail)
					return SipFail;
			}
		}
		else
	  	{
			*err = E_INV_TYPE;
			return SipFail;
		}
		break;
		default:
			*err = E_INV_TYPE;
			return SipFail;
	}
	*err = E_NO_ERROR;
	SIPDEBUGFN("Exiting function __sip_deleteHeaderAt");
	return SipSuccess;
}


/******************************************************************
**
** FUNCTION:  sip_verifyTypeAny
**
** DESCRIPTION: Internal functionto verify if the pHeader has an ANY
**		dType
**
******************************************************************/
SipBool sip_verifyTypeAny
#ifdef ANSI_PROTO
        (
          en_HeaderType         dType,
          SipError              *err )
#else
        ( dType, err )
          en_HeaderType         dType;
          SipError              *err;
#endif
{
	switch(dType)
	{
		case SipHdrTypeExpiresDate:
		case SipHdrTypeExpiresSec:
		case SipHdrTypeContactNormal:
		case SipHdrTypeContactWildCard:
		case SipHdrTypeRetryAfterDate:
		case SipHdrTypeRetryAfterSec:
			*err = E_INV_TYPE;
			return SipFail;
		default :
			*err = E_NO_ERROR;
			return SipSuccess;
	}
}


/******************************************************************
**
** FUNCTION:  sip_changeTypeAny
**
** DESCRIPTION: This function changes the Type to ANY dType if it exists
**
******************************************************************/
SipBool sip_changeTypeAny
#ifdef ANSI_PROTO
        (
          en_HeaderType         *dType,
          SipError              *err )
#else
        ( dType, err )
          en_HeaderType         *dType;
          SipError              *err;
#endif
{
	switch(*dType)
	{
		case SipHdrTypeExpiresDate:
		case SipHdrTypeExpiresSec:
				*dType = SipHdrTypeExpiresAny;
				break;
		case SipHdrTypeContactNormal:
		case SipHdrTypeContactWildCard:
				*dType = SipHdrTypeContactAny;
				break;
		case SipHdrTypeRetryAfterDate:
		case SipHdrTypeRetryAfterSec:
				*dType = SipHdrTypeRetryAfterAny;
				break;
		default :
				break;
	}
	*err = E_NO_ERROR;
	return SipSuccess;
}

/********************************************************************
**
** FUNCTION:  sip_makeStatusLine (internal function)
**
** DESCRIPTION: This function parses a Status Line given as
**				a string and fills up a SipStatusLine Structure.
**
*********************************************************************/
SipBool sip_makeStatusLine
#ifdef ANSI_PROTO
(SipStatusLine *pSLine, SIP_S8bit *pStr, SipError *pErr)
#else
	( pSLine,pStr,pErr)
	SipStatusLine *pSLine;
	SIP_S8bit *pStr;
	SipError *pErr;
#endif
{
	SIP_U32bit len;
	SIP_S8bit *pParserBuffer;
	SipHeaderParserParam dHeaderParserParam;

	len = strlen(pStr);
	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, pStr);
	pParserBuffer[len+1]='\0';
	dHeaderParserParam.pError=(SipError*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipError),pErr);
	if(dHeaderParserParam.pError==SIP_NULL)
		return SipFail;


	*(dHeaderParserParam.pError) = E_NO_ERROR;

	dHeaderParserParam.pSipMessage = ( SipMessage * ) \
		fast_memget(ACCESSOR_MEM_ID, sizeof (SipMessage) , pErr);
	if ( dHeaderParserParam.pSipMessage == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	dHeaderParserParam.pSipMessage->dType = SipMessageResponse;

	dHeaderParserParam.pSipMessage->u.pResponse = (SipRespMessage *) \
		fast_memget(ACCESSOR_MEM_ID, sizeof( SipRespMessage), pErr);
	if ( dHeaderParserParam.pSipMessage->u.pResponse == SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	dHeaderParserParam.pSipMessage->u.pResponse->pStatusLine = SIP_NULL;

	dHeaderParserParam.pGCList=(SipList*)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList),pErr);
    if(dHeaderParserParam.pGCList==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	if(sip_listInit((dHeaderParserParam.pGCList), sip_freeVoid, pErr)\
		==SipFail)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}

	if (sip_lex_Statusline_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, pErr);
		sip_listDeleteAll((dHeaderParserParam.pGCList), pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	glbSipParserStatuslineparse((void*)&dHeaderParserParam);
	sip_lex_Statusline_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	sip_listDeleteAll((dHeaderParserParam.pGCList), pErr);
	fast_memfree(ACCESSOR_MEM_ID,dHeaderParserParam.pGCList,SIP_NULL);

	if ( *(dHeaderParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSipStatusLine(dHeaderParserParam.pSipMessage->u.pResponse->pStatusLine);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pResponse, pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,pErr);

		*pErr = *(dHeaderParserParam.pError);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, pErr);
		return SipFail;
	}

	/* Clone the parsed Status Line structure into the parameter input */

	if ((sip_cloneSipStatusLine(pSLine, dHeaderParserParam.pSipMessage->u.pResponse->pStatusLine,\
									pErr)) == SipFail)
	{
     	sip_freeSipStatusLine(dHeaderParserParam.pSipMessage->u.pResponse->pStatusLine);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pResponse, SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, SIP_NULL);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, SIP_NULL);

		return SipFail;
	}

	sip_freeSipStatusLine(dHeaderParserParam.pSipMessage->u.pResponse->pStatusLine);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pResponse, pErr);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, pErr);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, pErr);

	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/********************************************************************
**
** FUNCTION:  sip_makeReqLine (internal function)
**
** DESCRIPTION: This function parses a Status Line given as
**				a string and fills up a SipStatusLine Structure.
**
*********************************************************************/
SipBool sip_makeReqLine
#ifdef ANSI_PROTO
	(SipReqLine *pSipReqLine, SIP_S8bit *line, SipError *pErr)
#else
	(pSipReqLine, line, pErr)
	SipReqLine *pSipReqLine;
	SIP_S8bit *line;
	SipError *pErr;
#endif
{
	SIP_U32bit len;
	SipError temp_err;
	SIP_S8bit *pParserBuffer;
	SipHeaderParserParam dHeaderParserParam;

	len = strlen(line);
	dHeaderParserParam.pError=(SipError *)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipError), pErr);
	dHeaderParserParam.pGCList=(SipList *)fast_memget(ACCESSOR_MEM_ID,\
		sizeof(SipList), pErr);

	pParserBuffer = (SIP_S8bit *) fast_memget(ACCESSOR_MEM_ID, len+2, pErr);
	if(pParserBuffer == SIP_NULL)
		return SipFail;
	strcpy(pParserBuffer, line);
	pParserBuffer[len+1]='\0';

	*(dHeaderParserParam.pError) = E_NO_ERROR;


		dHeaderParserParam.pSipMessage = ( SipMessage * )fast_memget(ACCESSOR_MEM_ID, sizeof(SipMessage) , pErr);
	if ( dHeaderParserParam.pSipMessage == SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		*pErr = E_NO_MEM;
		return SipFail;
	}

	dHeaderParserParam.pSipMessage->dType = SipMessageRequest;
	dHeaderParserParam.pSipMessage->u.pRequest = (SipReqMessage *) \
		fast_memget(ACCESSOR_MEM_ID, sizeof( SipReqMessage), pErr);
	if ( dHeaderParserParam.pSipMessage->u.pRequest == SIP_NULL)
	{
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		*pErr = E_NO_MEM;
		return SipFail;
	}
	dHeaderParserParam.pSipMessage->u.pRequest->pRequestLine  = SIP_NULL;

	if(sip_listInit((dHeaderParserParam.pGCList), sip_freeVoid, pErr)\
		==SipFail)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,
			&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		*pErr = E_NO_MEM;
		return SipFail;
	}

	/*glbSipParserAddrSpec = SIP_NULL;*/
	if (sip_lex_Reqline_scan_buffer(pParserBuffer, len+2) != 0)
	{
		sip_listDeleteAll( &(dHeaderParserParam.pSipMessage->slOrderInfo),\
			pErr);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->\
			pGeneralHdr, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,
			&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		sip_listDeleteAll((dHeaderParserParam.pGCList), pErr);
		*pErr = E_NO_MEM;
		return SipFail;
	}

    sip_lex_Reqline_reset_state();
	glbSipParserReqlineparse((void *)&dHeaderParserParam);
	sip_lex_Reqline_release_buffer();
	fast_memfree(ACCESSOR_MEM_ID,pParserBuffer,SIP_NULL);

	sip_listDeleteAll((dHeaderParserParam.pGCList), pErr);

	if ( *(dHeaderParserParam.pError) != E_NO_ERROR)
	{
		sip_freeSipReqLine(dHeaderParserParam.pSipMessage->u.pRequest->pRequestLine);
		/*if (glbSipParserAddrSpec != SIP_NULL)
			sip_freeSipAddrSpec(glbSipParserAddrSpec);
		glbSipParserAddrSpec = SIP_NULL;*/
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pRequest, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList,&temp_err);
		*pErr = *(dHeaderParserParam.pError);

		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
		return SipFail;
	}

	if (sip_cloneSipReqLine(pSipReqLine,dHeaderParserParam.pSipMessage->u.pRequest->pRequestLine,\
		pErr) == SipFail)
	{
        sip_freeSipReqLine(dHeaderParserParam.pSipMessage->u.pRequest->pRequestLine);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pRequest, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,&temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
		fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);

		return SipFail;
	}



	sip_freeSipReqLine(dHeaderParserParam.pSipMessage->u.pRequest->pRequestLine);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage->u.pRequest, &temp_err);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pSipMessage,&temp_err);

	*pErr = E_NO_ERROR;
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pGCList, &temp_err);
	fast_memfree(ACCESSOR_MEM_ID, dHeaderParserParam.pError, &temp_err);
	return SipSuccess;


}
/*****************************************************************
** FUNCTION: sip_formSipList
** DESCRIPTION: Converts a list of String to text
** Parameters:
** 		pOut(OUT)    - output buffer
**		pList(IN)       -  the string list to be converted
** 		pSeperator(IN)	- each element to be sepearated by
**		dLeadingsep(IN)	- leading seperator
**		pErr (OUT)		- Possible Error value (see API ref doc)
*****************************************************************/

SipBool sip_formSipList
#ifdef ANSI_PROTO
	(SIP_S8bit 	*pOut, 
	SipList 	*pList, 
	SIP_S8bit 	*pSeparator,
	SIP_U8bit	dLeadingsep,
	SipError 	*pErr)
#else
	(pOut, pList, pSeparator, dLeadingsep, pErr)
	SIP_S8bit *pOut;
	SipList *pList;
	SIP_S8bit *pSeparator;
	SIP_U8bit dLeadingsep;
	SipError *pErr;
#endif
{
		SIP_U32bit listSize,listIter;
		sip_listSizeOf( pList, &listSize, pErr);
		listIter = 0;
		while (listIter < listSize)
		{
			SIP_S8bit *pString;
			sip_listGetAt (pList, listIter, (SIP_Pvoid *) &pString, pErr);
			if((listIter!=0)&&(dLeadingsep!=0))
				STRCAT ((char *)pOut, pSeparator);
			STRCAT(pOut,pString);
			listIter++;
		} /* while */
		if(listSize!=0)
			STRCAT(pOut,":");
			
		return SipSuccess;
}


