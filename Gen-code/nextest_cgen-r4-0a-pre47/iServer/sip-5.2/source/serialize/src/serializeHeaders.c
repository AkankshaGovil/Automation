#ifdef __cplusplus
extern "C"
{
#endif
/**********************************************************************
 ** FUNCTION:
 **            Handles serialization of SipHeader
 *********************************************************************
 ** FILENAME:
 ** serializeHeaders.c
 **
 ** DESCRIPTION: This file contains the code for handling serialization/
 **				 deserialization of SipHeader.
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 **
 ** 17/05/2002  R Kamath 	                               Initial Version
 **
 **
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *********************************************************************/
 
#include "serialize.h"
#include "sipcommon.h"
#include "sipstruct.h" 
#include "serializeHeaders.h"


tyDeSerFn	glbDeserializeArray[HEADERTYPENUM];
tyDeSerFn	glbSerializeArray[HEADERTYPENUM];

/***************************************************************************
**** FUNCTION:sip_initSerializationArray
****
****
**** DESCRIPTION:Initializes the serialization/deserialization arrays
******************************************************************************/
SipBool sip_initSerializationArray(SipError *pError)
{
	int i,j;
	for (i=0;i<HEADERTYPENUM;i++)
	{
		glbSerializeArray[i]=SIP_NULL;
		glbDeserializeArray[i]=SIP_NULL;
	}
	
	pError=SIP_NULL;

	for(j=0;j<HEADERTYPENUM;j++)
	{
		switch(j)
		{
			case SipHdrTypeFrom:
					glbSerializeArray[SipHdrTypeFrom] =  (tyDeSerFn)\
							sip_serializeSipFromHeader;
					glbDeserializeArray[SipHdrTypeFrom] =  (tyDeSerFn)\
						(tyDeSerFn) sip_deserializeSipFromHeader ;
					break;

			case SipHdrTypeAuthenticationInfo:
					glbSerializeArray[SipHdrTypeAuthenticationInfo] =  \
						(tyDeSerFn) sip_serializeSipAuthenticationInfoHeader;
					glbDeserializeArray[SipHdrTypeAuthenticationInfo] =  \
						(tyDeSerFn) sip_deserializeSipAuthenticationInfoHeader;
					break;

			case SipHdrTypeTo:
					glbSerializeArray[SipHdrTypeTo] =  (tyDeSerFn)\
							sip_serializeSipToHeader;
					glbDeserializeArray[SipHdrTypeTo] =  (tyDeSerFn)\
							sip_deserializeSipToHeader;
					break;

			case SipHdrTypeContentLength:
					glbSerializeArray[SipHdrTypeContentLength] =  (tyDeSerFn)\
							sip_serializeSipContentLengthHeader;
					glbDeserializeArray[SipHdrTypeContentLength] =  (tyDeSerFn)\
							sip_deserializeSipContentLengthHeader;
					break;
			
			case SipHdrTypeReplyTo:
					glbSerializeArray[SipHdrTypeReplyTo] =  (tyDeSerFn)\
							sip_serializeSipReplyToHeader;
					glbDeserializeArray[SipHdrTypeReplyTo] =  (tyDeSerFn)\
							sip_deserializeSipReplyToHeader;
					break;

			case SipHdrTypeReplaces:
					glbSerializeArray[SipHdrTypeReplaces] =  (tyDeSerFn)\
							sip_serializeSipReplacesHeader;
					glbDeserializeArray[SipHdrTypeReplaces] =  (tyDeSerFn)\
							sip_deserializeSipReplacesHeader;
					break;
			
			case SipHdrTypeMinExpires:
					glbSerializeArray[SipHdrTypeMinExpires] =  (tyDeSerFn)\
							sip_serializeSipMinExpiresHeader;
					glbDeserializeArray[SipHdrTypeMinExpires] =  (tyDeSerFn)\
							sip_serializeSipMinExpiresHeader;
					break;
					
			case SipHdrTypeContentType:
					glbSerializeArray[SipHdrTypeContentType] =  (tyDeSerFn)\
							sip_serializeSipContentTypeHeader;
					glbDeserializeArray[SipHdrTypeContentType] =  (tyDeSerFn)\
							sip_deserializeSipContentTypeHeader;
					break;

			case SipHdrTypeContentEncoding:
					glbSerializeArray[SipHdrTypeContentEncoding] =  (tyDeSerFn)\
							sip_serializeSipContentEncodingHeader;
					glbDeserializeArray[SipHdrTypeContentEncoding] =  \
						(tyDeSerFn) sip_deserializeSipContentEncodingHeader;
					break;

			case SipHdrTypeAccept:
					glbSerializeArray[SipHdrTypeAccept] =  (tyDeSerFn)\
							sip_serializeSipAcceptHeader;
					glbDeserializeArray[SipHdrTypeAccept] =  (tyDeSerFn)\
							sip_deserializeSipAcceptHeader;
					break;

			case SipHdrTypeAcceptEncoding:
					glbSerializeArray[SipHdrTypeAcceptEncoding] =  (tyDeSerFn)\
							sip_serializeSipAcceptEncodingHeader;
					glbDeserializeArray[SipHdrTypeAcceptEncoding] = \
						(tyDeSerFn) sip_deserializeSipAcceptEncodingHeader;
					break;

			case SipHdrTypeAcceptLanguage:
					glbSerializeArray[SipHdrTypeAcceptLanguage] =  (tyDeSerFn)\
							sip_serializeSipAcceptLangHeader;
					glbDeserializeArray[SipHdrTypeAcceptLanguage] = \
						(tyDeSerFn) sip_deserializeSipAcceptLangHeader;
					break;

			case SipHdrTypeCallId:
					glbSerializeArray[SipHdrTypeCallId] =  (tyDeSerFn)\
							sip_serializeSipCallIdHeader;
					glbDeserializeArray[SipHdrTypeCallId] =  (tyDeSerFn)\
							sip_deserializeSipCallIdHeader;
					break;

			case SipHdrTypeContactNormal:
					glbSerializeArray[SipHdrTypeContactNormal] =  (tyDeSerFn)\
							sip_serializeSipContactHeader;
					glbDeserializeArray[SipHdrTypeContactNormal] =  (tyDeSerFn)\
							sip_deserializeSipContactHeader;
					break;
					
			case SipHdrTypeContactWildCard:
					glbSerializeArray[SipHdrTypeContactWildCard] =  (tyDeSerFn)\
							sip_serializeSipContactHeader;
					glbDeserializeArray[SipHdrTypeContactWildCard] = \
						(tyDeSerFn) sip_deserializeSipContactHeader;
					break;
					
			case SipHdrTypeContactAny:
					glbSerializeArray[SipHdrTypeContactAny] =  (tyDeSerFn)\
							sip_serializeSipContactHeader;
					glbDeserializeArray[SipHdrTypeContactAny] =  (tyDeSerFn)\
							sip_deserializeSipContactHeader;
					break;
					
			case SipHdrTypeCseq:
					glbSerializeArray[SipHdrTypeCseq] =  (tyDeSerFn)\
							sip_serializeSipCseqHeader;
					glbDeserializeArray[SipHdrTypeCseq] =  (tyDeSerFn)\
							sip_deserializeSipCseqHeader;
					break;

			case SipHdrTypeDate:
					glbSerializeArray[SipHdrTypeDate] =  (tyDeSerFn)\
							sip_serializeSipDateHeader;
					glbDeserializeArray[SipHdrTypeDate] =  (tyDeSerFn)\
							sip_deserializeSipDateHeader;
					break;

			case SipHdrTypeEncryption:
					glbSerializeArray[SipHdrTypeEncryption] =  (tyDeSerFn)\
							sip_serializeSipEncryptionHeader;
					glbDeserializeArray[SipHdrTypeEncryption] =  (tyDeSerFn)\
							sip_deserializeSipEncryptionHeader;
					break;

			case SipHdrTypeExpiresDate:
					glbSerializeArray[SipHdrTypeExpiresDate] =  (tyDeSerFn)\
							sip_serializeSipExpiresHeader;
					glbDeserializeArray[SipHdrTypeExpiresDate] =  (tyDeSerFn)\
							sip_deserializeSipExpiresHeader;
					break;

			case SipHdrTypeExpiresAny:
					glbSerializeArray[SipHdrTypeExpiresAny] =  (tyDeSerFn)\
							sip_serializeSipExpiresHeader;
					glbDeserializeArray[SipHdrTypeExpiresAny] =  (tyDeSerFn)\
							sip_deserializeSipExpiresHeader;
					break;

			case SipHdrTypeExpiresSec:
					glbSerializeArray[SipHdrTypeExpiresSec] =  (tyDeSerFn)\
							sip_serializeSipExpiresHeader;
					glbDeserializeArray[SipHdrTypeExpiresSec] =  (tyDeSerFn)\
							sip_deserializeSipExpiresHeader;
					break;


#ifdef SIP_SESSIONTIMER
			case SipHdrTypeSessionExpires:
					glbSerializeArray[SipHdrTypeSessionExpires] =  (tyDeSerFn)\
							sip_serializeSipSessionExpiresHeader;
					glbDeserializeArray[SipHdrTypeSessionExpires] = \
						(tyDeSerFn) sip_deserializeSipSessionExpiresHeader;
					break;

			case SipHdrTypeMinSE:
					glbSerializeArray[SipHdrTypeMinSE] =  (tyDeSerFn)\
							sip_serializeSipMinSEHeader;
					glbDeserializeArray[SipHdrTypeMinSE] =  (tyDeSerFn)\
							sip_deserializeSipMinSEHeader;
					break;

#endif


			case SipHdrTypeRecordRoute:
					glbSerializeArray[SipHdrTypeRecordRoute] =  (tyDeSerFn)\
							sip_serializeSipRecordRouteHeader;
					glbDeserializeArray[SipHdrTypeRecordRoute] =  (tyDeSerFn)\
							sip_deserializeSipRecordRouteHeader;
					break;

			case SipHdrTypeTimestamp:
					glbSerializeArray[SipHdrTypeTimestamp] =  (tyDeSerFn)\
							sip_serializeSipTimeStampHeader;
					glbDeserializeArray[SipHdrTypeTimestamp] =  (tyDeSerFn)\
							sip_deserializeSipTimeStampHeader;
					break;

			case SipHdrTypeVia:
					glbSerializeArray[SipHdrTypeVia] =  (tyDeSerFn)\
							sip_serializeSipViaHeader;
					glbDeserializeArray[SipHdrTypeVia] =  (tyDeSerFn)\
							sip_deserializeSipViaHeader;
					break;

			case SipHdrTypeRequire:
					glbSerializeArray[SipHdrTypeRequire] =  (tyDeSerFn)\
							sip_serializeSipRequireHeader;
					glbDeserializeArray[SipHdrTypeRequire] =  (tyDeSerFn)\
							sip_deserializeSipRequireHeader;
					break;

			case SipHdrTypeOrganization:
					glbSerializeArray[SipHdrTypeOrganization] =  (tyDeSerFn)\
							sip_serializeSipOrganizationHeader;
					glbDeserializeArray[SipHdrTypeOrganization] =  (tyDeSerFn)\
							sip_deserializeSipOrganizationHeader;
					break;

			case SipHdrTypeContentDisposition:
					glbSerializeArray[SipHdrTypeContentDisposition] = \
						(tyDeSerFn)sip_serializeSipContentDispositionHeader;
					glbDeserializeArray[SipHdrTypeContentDisposition] = \
						(tyDeSerFn)sip_deserializeSipContentDispositionHeader;
					break;

			case SipHdrTypeContentLanguage:
					glbSerializeArray[SipHdrTypeContentLanguage] =  (tyDeSerFn)\
							sip_serializeSipContentLanguageHeader;
					glbDeserializeArray[SipHdrTypeContentLanguage] = \
						(tyDeSerFn) sip_deserializeSipContentLanguageHeader;
					break;

			case SipHdrTypeCallInfo:
					glbSerializeArray[SipHdrTypeCallInfo] =  (tyDeSerFn)\
							sip_serializeSipCallInfoHeader;
					glbDeserializeArray[SipHdrTypeCallInfo] =  (tyDeSerFn)\
							sip_deserializeSipCallInfoHeader;
					break;

			case SipHdrTypeAllow:
					glbSerializeArray[SipHdrTypeAllow] =  (tyDeSerFn)\
							sip_serializeSipAllowHeader;
					glbDeserializeArray[SipHdrTypeAllow] =  (tyDeSerFn)\
							sip_deserializeSipAllowHeader;
					break;

			case SipHdrTypeUserAgent:
					glbSerializeArray[SipHdrTypeUserAgent] =  (tyDeSerFn)\
							sip_serializeSipUserAgentHeader;
					glbDeserializeArray[SipHdrTypeUserAgent] =  (tyDeSerFn)\
							sip_deserializeSipUserAgentHeader;
					break;

#ifdef SIP_IMPP
			case SipHdrTypeAllowEvents:
					glbSerializeArray[SipHdrTypeAllowEvents] =  (tyDeSerFn)\
							sip_serializeSipAllowEventsHeader;
					glbDeserializeArray[SipHdrTypeAllowEvents] =  (tyDeSerFn)\
							sip_deserializeSipAllowEventsHeader;
					break;

			case SipHdrTypeEvent:
					glbSerializeArray[SipHdrTypeEvent] =  (tyDeSerFn)\
							sip_serializeSipEventHeader;
					glbDeserializeArray[SipHdrTypeEvent] =  (tyDeSerFn)\
							sip_deserializeSipEventHeader;
					break;

			case SipHdrTypeSubscriptionState:
					glbSerializeArray[SipHdrTypeSubscriptionState] =  \
						(tyDeSerFn) sip_serializeSipSubscriptionStateHeader;
					glbDeserializeArray[SipHdrTypeSubscriptionState] =  \
						(tyDeSerFn) sip_deserializeSipSubscriptionStateHeader;
					break;

#endif


			case SipHdrTypeUnknown:
					glbSerializeArray[SipHdrTypeUnknown] =  (tyDeSerFn)\
							sip_serializeSipUnknownHeader;
					glbDeserializeArray[SipHdrTypeUnknown] =  (tyDeSerFn)\
							sip_deserializeSipUnknownHeader;
					break;

			case SipHdrTypeMimeVersion:
					glbSerializeArray[SipHdrTypeMimeVersion] =  (tyDeSerFn)\
							sip_serializeSipMimeVersionHeader;
					glbDeserializeArray[SipHdrTypeMimeVersion] =  (tyDeSerFn)\
							sip_deserializeSipMimeVersionHeader;
					break;

			case SipHdrTypeSupported:
					glbSerializeArray[SipHdrTypeSupported] =  (tyDeSerFn)\
							sip_serializeSipSupportedHeader;
					glbDeserializeArray[SipHdrTypeSupported] =  (tyDeSerFn)\
							sip_deserializeSipSupportedHeader;
					break;

#ifdef SIP_DCS
			case SipHdrTypeDcsMediaAuthorization:
					glbSerializeArray[SipHdrTypeDcsMediaAuthorization] =\
						(tyDeSerFn)sip_serializeSipDcsMediaAuthorizationHeader;
					glbDeserializeArray[SipHdrTypeDcsMediaAuthorization]=\
						(tyDeSerFn)\
						sip_deserializeSipDcsMediaAuthorizationHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsAnonymity:
					glbSerializeArray[SipHdrTypeDcsAnonymity] =  (tyDeSerFn)\
							sip_serializeSipDcsAnonymityHeader;
					glbDeserializeArray[SipHdrTypeDcsAnonymity] =  (tyDeSerFn)\
							sip_deserializeSipDcsAnonymityHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsRpidPrivacy:
					glbSerializeArray[SipHdrTypeDcsRpidPrivacy] =  (tyDeSerFn)\
							sip_serializeSipDcsRpidPrivacyHeader;
					glbDeserializeArray[SipHdrTypeDcsRpidPrivacy] = \
						(tyDeSerFn) sip_deserializeSipDcsRpidPrivacyHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsState:
					glbSerializeArray[SipHdrTypeDcsState] =  (tyDeSerFn)\
							sip_serializeSipDcsStateHeader;
					glbDeserializeArray[SipHdrTypeDcsState] =  (tyDeSerFn)\
							sip_deserializeSipDcsStateHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsBillingId:
					glbSerializeArray[SipHdrTypeDcsBillingId] =  (tyDeSerFn)\
							sip_serializeSipDcsBillingIdHeader;
					glbDeserializeArray[SipHdrTypeDcsBillingId] =  (tyDeSerFn)\
							sip_deserializeSipDcsBillingIdHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsBillingInfo:
					glbSerializeArray[SipHdrTypeDcsBillingInfo] =  (tyDeSerFn)\
							sip_serializeSipDcsBillingInfoHeader;
					glbDeserializeArray[SipHdrTypeDcsBillingInfo] = \
						(tyDeSerFn) sip_deserializeSipDcsBillingInfoHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsOsps:
					glbSerializeArray[SipHdrTypeDcsOsps] =  (tyDeSerFn)\
							sip_serializeSipDcsOspsHeader;
					glbDeserializeArray[SipHdrTypeDcsOsps] =  (tyDeSerFn)\
							sip_deserializeSipDcsOspsHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeSession:
					glbSerializeArray[SipHdrTypeSession] =  (tyDeSerFn)\
							sip_serializeSipSessionHeader;
					glbDeserializeArray[SipHdrTypeSession] =  (tyDeSerFn)\
							sip_deserializeSipSessionHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsGate:
					glbSerializeArray[SipHdrTypeDcsGate] =  (tyDeSerFn)\
							sip_serializeSipDcsGateHeader;
					glbDeserializeArray[SipHdrTypeDcsGate] =  (tyDeSerFn)\
							sip_deserializeSipDcsGateHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsLaes:
					glbSerializeArray[SipHdrTypeDcsLaes] =  (tyDeSerFn)\
							sip_serializeSipDcsLaesHeader;
					glbDeserializeArray[SipHdrTypeDcsLaes] =  (tyDeSerFn)\
							sip_deserializeSipDcsLaesHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsRemotePartyId:
					glbSerializeArray[SipHdrTypeDcsRemotePartyId] = \
						(tyDeSerFn) sip_serializeSipDcsRemotePartyIdHeader;
					glbDeserializeArray[SipHdrTypeDcsRemotePartyId] = \
						(tyDeSerFn) sip_deserializeSipDcsRemotePartyIdHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsTracePartyId:
					glbSerializeArray[SipHdrTypeDcsTracePartyId] = \
						(tyDeSerFn) sip_serializeSipDcsTracePartyIdHeader;
					glbDeserializeArray[SipHdrTypeDcsTracePartyId] =\
						(tyDeSerFn) sip_deserializeSipDcsTracePartyIdHeader;
					break;

#endif


#ifdef SIP_DCS
			case SipHdrTypeDcsRedirect:
					glbSerializeArray[SipHdrTypeDcsRedirect] =  (tyDeSerFn)\
							sip_serializeSipDcsRedirectHeader;
					glbDeserializeArray[SipHdrTypeDcsRedirect] =  (tyDeSerFn)\
							sip_deserializeSipDcsRedirectHeader;
					break;

#endif



			case SipHdrTypeAuthorization:
					glbSerializeArray[SipHdrTypeAuthorization] =  (tyDeSerFn)\
							sip_serializeSipAuthorizationHeader;
					glbDeserializeArray[SipHdrTypeAuthorization] =  (tyDeSerFn)\
							sip_deserializeSipAuthorizationHeader;
					break;

			case SipHdrTypeHide:
					glbSerializeArray[SipHdrTypeHide] =  (tyDeSerFn)\
							sip_serializeSipHideHeader;
					glbDeserializeArray[SipHdrTypeHide] =  (tyDeSerFn)\
							sip_deserializeSipHideHeader;
					break;

			case SipHdrTypeMaxforwards:
					glbSerializeArray[SipHdrTypeMaxforwards] =  (tyDeSerFn)\
							sip_serializeSipMaxForwardsHeader;
					glbDeserializeArray[SipHdrTypeMaxforwards] =  (tyDeSerFn)\
							sip_deserializeSipMaxForwardsHeader;
					break;

			case SipHdrTypePriority:
					glbSerializeArray[SipHdrTypePriority] =  (tyDeSerFn)\
							sip_serializeSipPriorityHeader;
					glbDeserializeArray[SipHdrTypePriority] =  (tyDeSerFn)\
							sip_deserializeSipPriorityHeader;
					break;

			case SipHdrTypeProxyauthorization:
					glbSerializeArray[SipHdrTypeProxyauthorization] = \
						(tyDeSerFn) sip_serializeSipProxyAuthorizationHeader;
					glbDeserializeArray[SipHdrTypeProxyauthorization] = \
						(tyDeSerFn) sip_deserializeSipProxyAuthorizationHeader;
					break;

			case SipHdrTypeProxyRequire:
					glbSerializeArray[SipHdrTypeProxyRequire] =  (tyDeSerFn)\
							sip_serializeSipProxyRequireHeader;
					glbDeserializeArray[SipHdrTypeProxyRequire] =  (tyDeSerFn)\
							sip_deserializeSipProxyRequireHeader;
					break;

			case SipHdrTypeRoute:
					glbSerializeArray[SipHdrTypeRoute] =  (tyDeSerFn)\
							sip_serializeSipRouteHeader;
					glbDeserializeArray[SipHdrTypeRoute] =  (tyDeSerFn)\
							sip_deserializeSipRouteHeader;
					break;
#ifdef SIP_SECURITY
			case SipHdrTypeSecurityClient:
                                        glbSerializeArray[SipHdrTypeSecurityClient] =  (tyDeSerFn)\
                                                        sip_serializeSipSecurityClientHeader;
                                        glbDeserializeArray[SipHdrTypeSecurityClient] =  (tyDeSerFn)\
                                                        sip_deserializeSipSecurityClientHeader;
					break;

			case SipHdrTypeSecurityServer:
                                        glbSerializeArray[SipHdrTypeSecurityServer] =  (tyDeSerFn)\
							sip_serializeSipSecurityServerHeader;
                                        glbDeserializeArray[SipHdrTypeSecurityServer] =  (tyDeSerFn)\
							sip_deserializeSipSecurityServerHeader;
                                        break;
			
			case SipHdrTypeSecurityVerify:
                                        glbSerializeArray[SipHdrTypeSecurityVerify] =  (tyDeSerFn)\
							sip_serializeSipSecurityVerifyHeader;
                                        glbDeserializeArray[SipHdrTypeSecurityVerify] =  (tyDeSerFn)\
							sip_deserializeSipSecurityVerifyHeader;
                                        break;
#endif

			case SipHdrTypeResponseKey:
					glbSerializeArray[SipHdrTypeResponseKey] =  (tyDeSerFn)\
							sip_serializeSipRespKeyHeader;
					glbDeserializeArray[SipHdrTypeResponseKey] =  (tyDeSerFn)\
							sip_deserializeSipRespKeyHeader;
					break;

			case SipHdrTypeSubject:
					glbSerializeArray[SipHdrTypeSubject] =  (tyDeSerFn)\
							sip_serializeSipSubjectHeader;
					glbDeserializeArray[SipHdrTypeSubject] =  (tyDeSerFn)\
							sip_deserializeSipSubjectHeader;
					break;

			case SipHdrTypeRAck:
					glbSerializeArray[SipHdrTypeRAck] =  (tyDeSerFn)\
							sip_serializeSipRackHeader;
					glbDeserializeArray[SipHdrTypeRAck] =  (tyDeSerFn)\
							sip_deserializeSipRackHeader;
					break;

#ifdef SIP_CCP
			case SipHdrTypeAcceptContact:
					glbSerializeArray[SipHdrTypeAcceptContact] =  (tyDeSerFn)\
							sip_serializeSipAcceptContactHeader;
					glbDeserializeArray[SipHdrTypeAcceptContact] =  (tyDeSerFn)\
							sip_deserializeSipAcceptContactHeader;
					break;

			case SipHdrTypeRejectContact:
					glbSerializeArray[SipHdrTypeRejectContact] =  (tyDeSerFn)\
							sip_serializeSipRejectContactHeader;
					glbDeserializeArray[SipHdrTypeRejectContact] =  (tyDeSerFn)\
							sip_deserializeSipRejectContactHeader;
					break;

#endif


			case SipHdrTypeAlertInfo:
					glbSerializeArray[SipHdrTypeAlertInfo] =  (tyDeSerFn)\
							sip_serializeSipAlertInfoHeader;
					glbDeserializeArray[SipHdrTypeAlertInfo] =  (tyDeSerFn)\
							sip_deserializeSipAlertInfoHeader;
					break;

#ifdef SIP_CCP
			case SipHdrTypeRequestDisposition:
					glbSerializeArray[SipHdrTypeRequestDisposition] = \
						(tyDeSerFn) sip_serializeSipRequestDispositionHeader;
					glbDeserializeArray[SipHdrTypeRequestDisposition] =  \
						(tyDeSerFn)sip_deserializeSipRequestDispositionHeader;
					break;

#endif


			case SipHdrTypeInReplyTo:
					glbSerializeArray[SipHdrTypeInReplyTo] =  (tyDeSerFn)\
							sip_serializeSipInReplyToHeader;
					glbDeserializeArray[SipHdrTypeInReplyTo] =  (tyDeSerFn)\
							sip_deserializeSipInReplyToHeader;
					break;

			case SipHdrTypeReferTo:
					glbSerializeArray[SipHdrTypeReferTo] =  (tyDeSerFn)\
							sip_serializeSipReferToHeader;
					glbDeserializeArray[SipHdrTypeReferTo] =  (tyDeSerFn)\
							sip_deserializeSipReferToHeader;
					break;

			case SipHdrTypeReferredBy:
					glbSerializeArray[SipHdrTypeReferredBy] =  (tyDeSerFn)\
							sip_serializeSipReferredByHeader;
					glbDeserializeArray[SipHdrTypeReferredBy] =  (tyDeSerFn)\
							sip_deserializeSipReferredByHeader;
					break;

			case SipHdrTypeAlso:
					glbSerializeArray[SipHdrTypeAlso] =  (tyDeSerFn)\
							sip_serializeSipAlsoHeader;
					glbDeserializeArray[SipHdrTypeAlso] =  (tyDeSerFn)\
							sip_deserializeSipAlsoHeader;
					break;

			case SipHdrTypeProxyAuthenticate:
					glbSerializeArray[SipHdrTypeProxyAuthenticate] = \
						(tyDeSerFn) sip_serializeSipProxyAuthenticateHeader;
					glbDeserializeArray[SipHdrTypeProxyAuthenticate] =  \
						(tyDeSerFn) sip_deserializeSipProxyAuthenticateHeader;
					break;

			case SipHdrTypeRetryAfterAny:
					glbSerializeArray[SipHdrTypeRetryAfterAny] =  (tyDeSerFn)\
							sip_serializeSipRetryAfterHeader;
					glbDeserializeArray[SipHdrTypeRetryAfterAny] =  (tyDeSerFn)\
							sip_deserializeSipRetryAfterHeader;
					break;

			case SipHdrTypeServer:
					glbSerializeArray[SipHdrTypeServer] =  (tyDeSerFn)\
							sip_serializeSipServerHeader;
					glbDeserializeArray[SipHdrTypeServer] =  (tyDeSerFn)\
							sip_deserializeSipServerHeader;
					break;

			case SipHdrTypeUnsupported:
					glbSerializeArray[SipHdrTypeUnsupported] =  (tyDeSerFn)\
							sip_serializeSipUnsupportedHeader;
					glbDeserializeArray[SipHdrTypeUnsupported] =  (tyDeSerFn)\
							sip_deserializeSipUnsupportedHeader;
					break;

			case SipHdrTypeWarning:
					glbSerializeArray[SipHdrTypeWarning] =  (tyDeSerFn)\
							sip_serializeSipWarningHeader;
					glbDeserializeArray[SipHdrTypeWarning] =  (tyDeSerFn)\
							sip_deserializeSipWarningHeader;
					break;

			case SipHdrTypeWwwAuthenticate:
					glbSerializeArray[SipHdrTypeWwwAuthenticate] =  (tyDeSerFn)\
							sip_serializeSipWwwAuthenticateHeader;
					glbDeserializeArray[SipHdrTypeWwwAuthenticate] = \
						(tyDeSerFn) sip_deserializeSipWwwAuthenticateHeader;
					break;

			case SipHdrTypeRSeq:
					glbSerializeArray[SipHdrTypeRSeq] =  (tyDeSerFn)\
							sip_serializeSipRseqHeader;
					glbDeserializeArray[SipHdrTypeRSeq] =  (tyDeSerFn)\
							sip_deserializeSipRseqHeader;
					break;

			case SipHdrTypeErrorInfo:
					glbSerializeArray[SipHdrTypeErrorInfo] =  (tyDeSerFn)\
							sip_serializeSipErrorInfoHeader;
					glbDeserializeArray[SipHdrTypeErrorInfo] =  (tyDeSerFn)\
							sip_deserializeSipErrorInfoHeader;
					break;

#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId:
					glbSerializeArray[SipHdrTypePAssertId] =  (tyDeSerFn)\
							sip_serializeSipPAssertIdHeader;
					glbDeserializeArray[SipHdrTypePAssertId] =  (tyDeSerFn)\
							sip_deserializeSipPAssertIdHeader;
					break;

			case SipHdrTypePPreferredId:
					glbSerializeArray[SipHdrTypePPreferredId] =  (tyDeSerFn)\
							sip_serializeSipPPreferredIdHeader;
					glbDeserializeArray[SipHdrTypePPreferredId] =  (tyDeSerFn)\
							sip_deserializeSipPPreferredIdHeader;
					break;

			case SipHdrTypePrivacy:
					glbSerializeArray[SipHdrTypePrivacy] =  (tyDeSerFn)\
							sip_serializeSipPrivacyHeader;
					glbDeserializeArray[SipHdrTypePrivacy] =  (tyDeSerFn)\
							sip_deserializeSipPrivacyHeader;
					break;

#endif /*#ifdef SIP_PRIVAC */

#ifdef SIP_3GPP
			case SipHdrTypeServiceRoute:
					glbSerializeArray[SipHdrTypeServiceRoute] =  (tyDeSerFn)\
							sip_serializeSipServiceRouteHeader;
					glbDeserializeArray[SipHdrTypeServiceRoute] =  (tyDeSerFn)\
							sip_deserializeSipServiceRouteHeader;
					break;
			case SipHdrTypePath:
					glbSerializeArray[SipHdrTypePath] =  (tyDeSerFn)\
							sip_serializeSipPathHeader;
					glbDeserializeArray[SipHdrTypePath] =  (tyDeSerFn)\
							sip_deserializeSipPathHeader;
					break;
			case SipHdrTypePanInfo:
					glbSerializeArray[SipHdrTypePanInfo] =  (tyDeSerFn)\
							sip_serializeSipPanInfoHeader;
					glbDeserializeArray[SipHdrTypePanInfo] =  (tyDeSerFn)\
							sip_deserializeSipPanInfoHeader;
					break;
            case SipHdrTypePcVector:
					glbSerializeArray[SipHdrTypePcVector] =  (tyDeSerFn)\
							sip_serializeSipPcVectorHeader;
					glbDeserializeArray[SipHdrTypePcVector] =  (tyDeSerFn)\
							sip_deserializeSipPcVectorHeader;
					break;
		

				

#endif /*#ifdef SIP_3GPP */
/*
			case SipHdrTypeRetryAfterDate:
					glbSerializeArray[SipHdrTypeRetryAfterDate] = (tyDeSerFn)\
							sip_serializeSipRetryAfterDateHeader;
					glbDeserializeArray[SipHdrTypeRetryAfterDate] = (tyDeSerFn)\
							sip_deserializeSipRetryAfterDateHeader;
					break ;

*/

			case SipHdrTypeReason:
					glbSerializeArray[SipHdrTypeReason] =  (tyDeSerFn)\
							sip_serializeSipReasonHeader;
					glbDeserializeArray[SipHdrTypeReason] =  (tyDeSerFn)\
							sip_deserializeSipReasonHeader;
					break;
#ifdef SIP_3GPP
			case SipHdrTypePAssociatedUri:
					glbSerializeArray[SipHdrTypePAssociatedUri] =  (tyDeSerFn)\
							sip_serializeSipPAssociatedUriHeader;
					glbDeserializeArray[SipHdrTypePAssociatedUri] = (tyDeSerFn)\
							sip_deserializeSipPAssociatedUriHeader;
					break;
			case SipHdrTypePCalledPartyId:
					glbSerializeArray[SipHdrTypePCalledPartyId] =  (tyDeSerFn)\
							sip_serializeSipPCalledPartyIdHeader;
					glbDeserializeArray[SipHdrTypePCalledPartyId] = (tyDeSerFn)\
							sip_deserializeSipPCalledPartyIdHeader;
					break;
			case SipHdrTypePVisitedNetworkId:
					glbSerializeArray[SipHdrTypePVisitedNetworkId] =  (tyDeSerFn)\
							sip_serializeSipPVisitedNetworkIdHeader;
					glbDeserializeArray[SipHdrTypePVisitedNetworkId] = (tyDeSerFn)\
							sip_deserializeSipPVisitedNetworkIdHeader;
					break;
   			case SipHdrTypePcfAddr:
					glbSerializeArray[SipHdrTypePcfAddr] =  (tyDeSerFn)\
							sip_serializeSipPcfAddrHeader;
					glbDeserializeArray[SipHdrTypePcfAddr] = (tyDeSerFn)\
							sip_deserializeSipPcfAddrHeader;
					break;

#endif /*#ifdef SIP_3GPP */
#ifdef SIP_CONF
			case SipHdrTypeJoin:
					glbSerializeArray[SipHdrTypeJoin] =  (tyDeSerFn)\
							sip_serializeSipJoinHeader;
					glbDeserializeArray[SipHdrTypeJoin] = (tyDeSerFn)\
							sip_deserializeSipJoinHeader;
					break;
#endif                    
#ifdef SIP_CONGEST
			case SipHdrTypeRsrcPriority:
					glbSerializeArray[SipHdrTypeRsrcPriority] =  (tyDeSerFn)\
							sip_serializeSipRsrcPriorityHeader;
					glbDeserializeArray[SipHdrTypeRsrcPriority] = (tyDeSerFn)\
							sip_deserializeSipRsrcPriorityHeader;
					break;
			case SipHdrTypeAcceptRsrcPriority:
					glbSerializeArray[SipHdrTypeAcceptRsrcPriority] =  (tyDeSerFn)\
							sip_serializeSipAcceptRsrcPriorityHeader;
					glbDeserializeArray[SipHdrTypeAcceptRsrcPriority] = (tyDeSerFn)\
							sip_deserializeSipAcceptRsrcPriorityHeader;
					break;
                    
#endif                      
			default:	
					/*ssdsdsd*/
					break;

		}
	}
	return SipSuccess;
}

/***************************************************************************
**** FUNCTION:sip_serializeSipHeader
****
****
**** DESCRIPTION:Serializes the SipHeader structure
******************************************************************************/
SIP_U32bit sip_serializeSipHeader
#ifdef ANSI_PROTO
 (SipHeader *pSipHeader,
 SIP_S8bit *buffer,
 SIP_U32bit position,
 SipError	*pError)
#else
 (pSipHeader,buffer,position, pError)
 SipHeader *pSipHeader;
 SIP_S8bit *buffer;
 SIP_U32bit position;
 SipError *pError;
#endif
{
	SIP_U16bit id ;
	SIP_U16bit idElement=0;
	SIP_U32bit lengthElement=0;
	SIP_U32bit objectPos=0;
	SIP_U32bit	sLength=0,headerSize=0;

	SIP_U32bit bytesWritten=0;

 	*pError = E_NO_ERROR;
	lengthElement = 0;
 	sLength=position;

	/* storing the ID field of SipHeader */
 	id=SIPHEADER_ID;
 	idElement =SIP_htons(id);
 	memcpy(&buffer[position],&idElement,sizeof(SIP_U16bit));
 	position=position + sizeof(SIP_U16bit);
	
	/*In case the SipHeader itself is NULL then set the length to zero*/
	if (pSipHeader==SIP_NULL)
	{
		lengthElement=0;
 		memcpy(&buffer[position],&lengthElement,sizeof(SIP_U32bit));
		return(sizeof(SIP_U32bit)+sizeof(SIP_U16bit));
	}
	
 	/* preserve the position and skip 4 bytes to store LENGTH field
	of SipHeader later */
 	objectPos = position;
 	position=position + sizeof(SIP_U32bit);
	
	/*Now serialize the enum that is stored in the SipHeader*/
	
	/*Storing the id of the enum itself*/
	idElement =SIP_htons(HEADERTYPEENUMID);
 	memcpy(&buffer[position],&idElement,sizeof(SIP_U16bit));
 	position=position + sizeof(SIP_U16bit);
	
 	/* Storing Length  of SipHeader type */
 	lengthElement=sizeof(en_HeaderType);
	lengthElement = SIP_htonl(lengthElement);	
 	memcpy(&buffer[position],&lengthElement,sizeof(SIP_U32bit));
 	position=position + sizeof(SIP_U32bit);

	/*Storing the enum itself*/
	lengthElement=pSipHeader->dType;
    lengthElement = SIP_htonl(lengthElement);
 	memcpy(&buffer[position],&lengthElement,sizeof(en_HeaderType));
 	position=position + sizeof(en_HeaderType);

	/*Now call the relevant serialization function*/
	if (glbSerializeArray[pSipHeader->dType]!=SIP_NULL)
	{	
		bytesWritten=(*glbSerializeArray[pSipHeader->dType])\
			(pSipHeader->pHeader,buffer,position,pError);
		if (bytesWritten==0)
			return 0;
	}
	else
	{
		return 0;
	}

	position=position+bytesWritten;		
	
	/*How many bytes were written for SipHeader??*/
	headerSize=(position-sLength);
	headerSize=SIP_htonl(headerSize);
	
	memcpy(&buffer[objectPos],&headerSize,sizeof(SIP_U32bit));
	headerSize=SIP_ntohl(headerSize);
	return(headerSize);
}

/***************************************************************************
**** FUNCTION:sip_deserializeSipHeader
****
****
**** DESCRIPTION:DeSerializes the SipHeader structure
******************************************************************************/
SIP_U32bit sip_deserializeSipHeader
#ifdef ANSI_PROTO
 (SipHeader **ppSipHeader,
 SIP_S8bit *buffer,
 SIP_U32bit position,
 SipError	*pError)
#else
 (ppSipHeader,buffer,position, pError)
 SipHeader **ppSipHeader;
 SIP_S8bit *buffer;
 SIP_U32bit position;
 SipError *pError;
#endif
{
 	SIP_U16bit objectid=0;
 	SIP_U32bit objectlength=0;

 	SIP_U16bit idElement=0;
 	SIP_U32bit lengthElement=0;
 	SIP_U32bit lengthSipHeader=0;
 	SIP_U32bit dsLength;
	SIP_U32bit	enumLength;
	en_HeaderType	dType;
	SIP_U32bit	noDeserializedBytes=0;
	SipHeader	*pReturnedHdr=SIP_NULL;
	SipBool		dResult;
 
	/* preserve the position to calculate the deSerialized length */ 
	dsLength=position;

 	/* retrieving the ID field of object */ 
 	memcpy(&idElement, &buffer[position],sizeof(SIP_U16bit));
 	position = position + sizeof(SIP_U16bit);
 	objectid = SIP_ntohs(idElement);
	/*Confirm if it is indeed the SipHeader serialized*/
 	if (objectid !=SIPHEADER_ID) 		 return(0); 

 	/* retrieving the LENGTH field of object */ 
 	memcpy(&lengthElement, &buffer[position],sizeof(SIP_U32bit));
 	position = position + sizeof(SIP_U32bit);
 	objectlength = SIP_ntohl(lengthElement);
	lengthSipHeader = objectlength;
	/*If the Length is zero then it means that an empty SipHeader
	  had been serialized*/
 	if(objectlength==0) 
	{
		*ppSipHeader=SIP_NULL;
 		*pError = E_NO_ERROR;
		return(sizeof(SIP_U16bit)+sizeof(SIP_U32bit));
	}	
	
 	*pError = E_NO_ERROR;

	/*get the dType enum stored in the SipHeader*/
	memcpy(&idElement, &buffer[position],sizeof(SIP_U16bit));
 	position = position + sizeof(SIP_U16bit);
 	idElement = SIP_ntohs(idElement);
 	if (idElement !=HEADERTYPEENUMID) return(0);

	memcpy(&lengthElement, &buffer[position],sizeof(SIP_U32bit));
 	position = position + sizeof(SIP_U32bit);
 	enumLength = SIP_ntohl(lengthElement);

 	memcpy(&lengthElement, &buffer[position],sizeof(SIP_U32bit));
 	position = position + sizeof(en_SipMessageType);
 	dType = (en_HeaderType)SIP_ntohl(lengthElement);

	/*Now that we have the type of the header enclosed inside the pdata
		we can call the deserialize of the header*/
		/*Now call the relevant serialization function*/
	if (glbDeserializeArray[dType]!=SIP_NULL)
	{
		dResult=sip_initSipHeader(&pReturnedHdr,SipHdrTypeAny,pError);
		if (dResult==SipFail)
		{
			return 0;
		}
		pReturnedHdr->pHeader=SIP_NULL;
		pReturnedHdr->dType=dType;
		noDeserializedBytes=(*glbDeserializeArray[dType])\
			((SIP_Pvoid *)(&(pReturnedHdr->pHeader)),buffer,position,pError);
		if (noDeserializedBytes==0)
			return 0;
	}
	else
	{
		return 0;
	}

	position=position+noDeserializedBytes;
	noDeserializedBytes=position-dsLength;
	*ppSipHeader=pReturnedHdr;	
	return(noDeserializedBytes);
}
#ifdef __cplusplus
}
#endif
