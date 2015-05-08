
/*
** THIS FILE IS INTERNALLY USED BY THE STACK
** IT SHOULD NOT BE DIRECTLY CALLED BY THE USER
*/

/******************************************************************************
** FUNCTION:
** 	This header file contains the prototypes off all INTERNAL SIP Structure
**      duplicating/cloning APIs.
**
*******************************************************************************
**
** FILENAME:
** 	sipclone.h
**
** DESCRIPTION:
**
**
** DATE    			  NAME           REFERENCE      REASON
** ----    			  ----           ---------      ------
** 25Jul00		S.Luthra							Intial Creation	
**
** Copyrights 1999, Hughes Software Systems, Ltd.
*******************************************************************************/

#ifndef __SIP_CLONE_INTRNL_H_
#define __SIP_CLONE_INTRNL_H_

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


SipBool __sip_cloneSipCallIdHeader _ARGS_ ((SipCallIdHeader *dest, SipCallIdHeader *source, SipError *err));
SipBool __sip_cloneSipContentLanguageHeader _ARGS_ ((SipContentLanguageHeader *dest, SipContentLanguageHeader *source, SipError *err));
 SipBool __sip_cloneSipCseqHeader _ARGS_ ((SipCseqHeader *dest, SipCseqHeader *source, SipError *err));
 SipBool __sip_cloneSipInReplyToHeader _ARGS_ ((SipInReplyToHeader *dest, SipInReplyToHeader *source, SipError *err));
 SipBool __sip_cloneSipAcceptHeader _ARGS_ ((SipAcceptHeader *dest, SipAcceptHeader *source, SipError *err));
 SipBool __sip_cloneSipAcceptEncodingHeader _ARGS_ ((SipAcceptEncodingHeader *dest, \
 	SipAcceptEncodingHeader *source, SipError *err));
 SipBool __sip_cloneSipAcceptLangHeader _ARGS_ ((SipAcceptLangHeader *dest, SipAcceptLangHeader *source, SipError *err));
 SipBool __sip_cloneSipTimeStampHeader _ARGS_ ((SipTimeStampHeader *dest, SipTimeStampHeader *source, SipError *err));
 SipBool	__sip_cloneSipContentLengthHeader _ARGS_ ((SipContentLengthHeader *dest, \
 	SipContentLengthHeader *source, SipError *err));
 SipBool __sip_cloneSipContentTypeHeader _ARGS_ ((SipContentTypeHeader *dest, SipContentTypeHeader *source, SipError *err));
 SipBool __sip_cloneSipContentEncodingHeader _ARGS_ ((SipContentEncodingHeader *dest, SipContentEncodingHeader *source, SipError *err));
 SipBool __sip_cloneSipUnknownHeader _ARGS_ ((SipUnknownHeader *dest, SipUnknownHeader *source, SipError *err));
 SipBool __sip_cloneSipEncryptionHeader _ARGS_ ((SipEncryptionHeader *dest, SipEncryptionHeader *source, SipError *err));
#ifdef SIP_SESSIONTIMER
 SipBool __sip_cloneSipMinSEHeader _ARGS_ ((SipMinSEHeader *dest, SipMinSEHeader *source, SipError *err));
 SipBool __sip_cloneSipSessionExpiresHeader _ARGS_ ((SipSessionExpiresHeader *dest, SipSessionExpiresHeader *source, SipError *err));
#endif
 SipBool __sip_cloneSipExpiresHeader _ARGS_ ((SipExpiresHeader *dest, SipExpiresHeader *source, SipError *err));
 SipBool __sip_cloneSipFromHeader _ARGS_ ((SipFromHeader *dest, SipFromHeader *source, SipError *err));
 SipBool __sip_cloneSipToHeader _ARGS_ ((SipToHeader *dest, SipToHeader *source, SipError *err));
 SipBool __sip_cloneSipViaHeader _ARGS_ ((SipViaHeader *dest, SipViaHeader *source, SipError *err));
 SipBool __sip_cloneSipContactHeader _ARGS_ ((SipContactHeader *dest, SipContactHeader *source, SipError *err));
 SipBool __sip_cloneSipRecordRouteHeader _ARGS_ ((SipRecordRouteHeader *dest, SipRecordRouteHeader *source, SipError *err));
 #ifdef SIP_3GPP
 SipBool __sip_cloneSipServiceRouteHeader _ARGS_ ((SipServiceRouteHeader *dest, SipServiceRouteHeader *source, SipError *err));
 SipBool __sip_cloneSipPathHeader _ARGS_ ((SipPathHeader *dest, SipPathHeader *source, SipError *err));
 SipBool __sip_cloneSipPanInfoHeader _ARGS_ ((SipPanInfoHeader *dest, SipPanInfoHeader *source, SipError *err));
SipBool __sip_cloneSipPcVectorHeader _ARGS_ ((SipPcVectorHeader *dest, SipPcVectorHeader *source, SipError *err));
 #endif
 #ifdef SIP_PRIVACY
 SipBool __sip_cloneSipPAssertIdHeader _ARGS_ ((SipPAssertIdHeader *pDest, SipPAssertIdHeader *pSource, SipError *err));
 SipBool __sip_cloneSipPPreferredIdHeader _ARGS_ ((SipPPreferredIdHeader *pDest, SipPPreferredIdHeader *pSource, SipError *err));
 #endif
 SipBool __sip_cloneSipStatusLine _ARGS_ ((SipStatusLine *dest, SipStatusLine *source, SipError *err));
 SipBool __sip_cloneSipReqLine _ARGS_ ((SipReqLine *dest, SipReqLine *source, SipError *err));
 SipBool __sip_cloneSipAllowHeader _ARGS_ ((SipAllowHeader *dest, SipAllowHeader *source, SipError *err));
 SipBool __sip_cloneSipWarningHeader _ARGS_ ((SipWarningHeader *dest, SipWarningHeader *source, SipError *err));
#ifdef SIP_PRIVACY
 SipBool __sip_cloneSipPrivacyHeader _ARGS_((SipPrivacyHeader *dest, SipPrivacyHeader *source, SipError *err));
#endif
 SipBool __sip_cloneSipRetryAfterHeader _ARGS_ ((SipRetryAfterHeader *dest, SipRetryAfterHeader *source, SipError *err));
 SipBool __sip_cloneSipProxyAuthenticateHeader _ARGS_ ((SipProxyAuthenticateHeader *dest, SipProxyAuthenticateHeader *source, SipError *err));
 SipBool __sip_cloneSipWwwAuthenticateHeader _ARGS_ ((SipWwwAuthenticateHeader *dest, SipWwwAuthenticateHeader *source, SipError *err));
 SipBool __sip_cloneSipAuthenticationInfoHeader _ARGS_ ((SipAuthenticationInfoHeader *pDest, SipAuthenticationInfoHeader *pSource, SipError *pErr));
 SipBool __sip_cloneSipReferToHeader _ARGS_ ((SipReferToHeader *pDest, SipReferToHeader *pSource, SipError *pErr));
 SipBool __sip_cloneSipReferredByHeader _ARGS_ ((SipReferredByHeader *pDest, SipReferredByHeader *pSource, SipError *pErr));
 SipBool __sip_cloneSipAuthorizationHeader _ARGS_((SipAuthorizationHeader *dest,SipAuthorizationHeader *source,SipError *err));
 SipBool __sip_cloneSipHideHeader _ARGS_((SipHideHeader *to,SipHideHeader *from,SipError *err));
 SipBool __sip_cloneSipMaxForwardsHeader _ARGS_((SipMaxForwardsHeader *to,SipMaxForwardsHeader *from,SipError *err));
 SipBool __sip_cloneOrganizationHeader  _ARGS_((SipOrganizationHeader *to, SipOrganizationHeader *from,SipError *err));
 SipBool __sip_cloneSipPriorityHeader _ARGS_((SipPriorityHeader *to,SipPriorityHeader *from,SipError *err));
 SipBool __sip_cloneSipProxyAuthorizationHeader _ARGS_((SipProxyAuthorizationHeader *dest, SipProxyAuthorizationHeader *source, SipError *err));
 SipBool __sip_cloneSipProxyRequireHeader _ARGS_((SipProxyRequireHeader *to,SipProxyRequireHeader *from,SipError *err));
 SipBool __sip_cloneSipRouteHeader _ARGS_((SipRouteHeader *to,SipRouteHeader *from,SipError *err));
 SipBool __sip_cloneSipAlsoHeader _ARGS_((SipAlsoHeader *to,SipAlsoHeader *from,SipError *err));
 SipBool __sip_cloneSipRequireHeader _ARGS_((SipRequireHeader *to,SipRequireHeader *from,SipError *err));
 SipBool __sip_cloneSipRespKeyHeader _ARGS_((SipRespKeyHeader *to,SipRespKeyHeader *from,SipError *err));
 SipBool __sip_cloneSipSubjectHeader _ARGS_((SipSubjectHeader *to,SipSubjectHeader *from,SipError *err));
 SipBool __sip_cloneSipUserAgentHeader _ARGS_((SipUserAgentHeader *to,SipUserAgentHeader *from,SipError *err));
 SipBool __sip_cloneSipCallInfoHeader _ARGS_((SipCallInfoHeader *to,SipCallInfoHeader *from,SipError *err));
 SipBool __sip_cloneSipContentDispositionHeader _ARGS_((SipContentDispositionHeader *to,SipContentDispositionHeader *from,SipError *err));
 SipBool __sip_cloneSipReasonHeader _ARGS_((SipReasonHeader *pTo,SipReasonHeader *pFrom,SipError *err));
 SipBool __sip_cloneSipAlertInfoHeader _ARGS_((SipAlertInfoHeader *to,SipAlertInfoHeader *from,SipError *err));
 SipBool __sip_cloneSipErrorInfoHeader _ARGS_((SipErrorInfoHeader *to,SipErrorInfoHeader *from,SipError *err));
 SipBool __sip_cloneSipAddrSpec _ARGS_ ( (SipAddrSpec *dest, SipAddrSpec *source, SipError *err));
 SipBool __sip_cloneSipViaParam _ARGS_ ( (SipParam *dest, SipParam *source, SipError *err));
 SipBool __sip_cloneSipContactParam _ARGS_ ( (SipContactParam *dest, SipContactParam *source, SipError *err));
 SipBool __sip_cloneSipExpiresStruct _ARGS_ ( (SipExpiresStruct *dest, SipExpiresStruct *source, SipError *err));
 SipBool __sip_cloneSipStringList _ARGS_ ( (SipList *dest, SipList *source, SipError *err));
 SipBool __sip_cloneChallenge _ARGS_((SipGenericChallenge *dest,SipGenericChallenge *src,SipError *err));
 SipBool __sip_cloneCredential _ARGS_((SipGenericCredential *to,SipGenericCredential *from,SipError *err));
 SipBool __sip_cloneSipUrlParam _ARGS_((SipParam *to,SipParam *from, SipError *err));
 SipBool __sip_cloneSipUrl _ARGS_((SipUrl *to,SipUrl *from,SipError *err));
 SipBool __sip_cloneAddrSpec _ARGS_((SipAddrSpec *to,SipAddrSpec *from,SipError *err));
 SipBool __sip_cloneDateFormat _ARGS_((SipDateFormat *dest, SipDateFormat *src,SipError *err));
 SipBool __sip_cloneTimeFormat _ARGS_((SipTimeFormat * dest, SipTimeFormat *src,SipError *err));
 SipBool __sip_cloneDateStruct _ARGS_((SipDateStruct * dest, SipDateStruct *src,SipError *err));
 SipBool __sip_cloneExpires _ARGS_((SipExpiresStruct *to,SipExpiresStruct *from,SipError *err));
 SipBool __sip_cloneSipDateFormat _ARGS_((SipDateFormat * dest, SipDateFormat *src, SipError * err));
 SipBool __sip_cloneSipTimeFormat _ARGS_((SipTimeFormat * dest, SipTimeFormat *src, SipError * err));
 SipBool __sip_cloneSipDateStruct _ARGS_((SipDateStruct * dest, SipDateStruct *src, SipError * err));
 SipBool __sip_cloneSipChallenge _ARGS_((SipGenericChallenge * dest, SipGenericChallenge *src,SipError *err));
 SipBool __sip_cloneSdpTime _ARGS_(( SdpTime 	*dest, SdpTime 	*src, SipError 		*err));
 SipBool __sip_cloneSdpMedia _ARGS_(( SdpMedia 	*dest, SdpMedia 	*src, SipError 		*err)) ;
 SipBool __sip_cloneSdpMessage _ARGS_((SdpMessage * dest, SdpMessage *src, SipError *err));
 SipBool __sip_cloneSdpOrigin _ARGS_((SdpOrigin * dest, SdpOrigin *src, SipError *err));
 SipBool __sip_cloneSdpConnection _ARGS_((SdpConnection * dest, SdpConnection *src, SipError *err)) ;
 SipBool __sip_cloneSdpAttr _ARGS_((SdpAttr * dest, SdpAttr *src, SipError *err));
SipBool __sip_cloneSipUnknownMessage _ARGS_((SipUnknownMessage *dest, SipUnknownMessage *src, SipError *err));
SipBool __sip_cloneSipMsgBody _ARGS_((SipMsgBody *dest, SipMsgBody *src, SipError *err));
SipBool __sip_cloneSipParam _ARGS_((SipParam *pDest, SipParam *pSource, SipError *pErr));
SipBool __sip_cloneSipDateHeader _ARGS_((SipDateHeader *dest, SipDateHeader *source, SipError *err));
SipBool __sip_cloneSipOrganizationHeader _ARGS_((SipOrganizationHeader *to, SipOrganizationHeader *from, SipError *err));
SipBool __sip_cloneSipParamList _ARGS_((SipList *dest, SipList *source, SipError *err));
SipBool __sip_cloneSipServerHeader _ARGS_ ((SipServerHeader *pDest, SipServerHeader *pSource, SipError *pErr));
SipBool __sip_cloneSipUnsupportedHeader _ARGS_ ((SipUnsupportedHeader *pDest, SipUnsupportedHeader *pSource, SipError *pErr));
SipBool __sip_cloneSipSupportedHeader _ARGS_((SipSupportedHeader *pDest, SipSupportedHeader *pSource, SipError *pErr));
SipBool __sip_cloneSipMessage _ARGS_ ((SipMessage *pDest, SipMessage *pSource, SipError *pErr));
SipBool __sip_cloneSipOrderInfoList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipMessageBodyList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
/* GENERAL HEADERS */
SipBool __sip_cloneSipAcceptHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipContentLanguageHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAcceptEncodingHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAcceptLangList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAcceptLangHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipContactHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipRecordRouteHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
#ifdef SIP_3GPP
SipBool __sip_cloneSipPathHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipServiceRouteHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
#endif
SipBool __sip_cloneSipViaHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipContentEncodingHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipUnknownHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipBadHeaderList _ARGS_ ((SipList *pDest, SipList *pSource,\
SipError *pErr));
SipBool __sip_cloneSipSupportedHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipCallInfoHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipContentDispositionHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipReasonHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
/* REQUEST HEADERS */
SipBool __sip_cloneSipProxyRequireHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipProxyAuthorizationHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipRouteHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAlsoHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipRequireHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipWwwAuthenticateHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAuthenticationInfoHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAcceptContactHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipInReplyToHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipRejectContactHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipRequestDispositionHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipAlertInfoHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
#ifdef SIP_SECURITY
SipBool __sip_cloneSipSecurityClientHeaderList _ARGS_ ((SipList *pDest, SipList *pSource,SipError *pErr)); 
SipBool __sip_cloneSipSecurityVerifyHeaderList _ARGS_ ((SipList *pDest, SipList *pSource,SipError *pErr));
SipBool __sip_cloneSipSecurityServerHeaderList _ARGS_ ((SipList *pDest, SipList *pSource,SipError *pErr));
#endif
/* RESPONSE HEADER */
SipBool __sip_cloneSipAllowHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipProxyAuthenticateHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipUnsupportedHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipWarningHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipErrorInfoHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
/* COPYING ALL HEADERS */
SipBool __sip_cloneSipGeneralHeader _ARGS_ ((SipGeneralHeader *pDest, SipGeneralHeader *pSource, SipError *pErr));
SipBool __sip_cloneSipReqHeader _ARGS_ ((SipReqHeader *pDest, SipReqHeader *pSource, SipError *pErr));
SipBool __sip_cloneSipRespHeader _ARGS_ ((SipRespHeader *pDest, SipRespHeader *pSource, SipError *pErr));
/* COPYING MESSAGES */
SipBool __sip_cloneSipReqMessage _ARGS_ ((SipReqMessage *pDest, SipReqMessage *pSource, SipError *pErr));
SipBool __sip_cloneSipRespMessage _ARGS_ ((SipRespMessage *pDest, SipRespMessage *pSource, SipError *pErr));

#ifdef SIP_MWI
SipBool __sip_mwi_cloneMesgSummaryMessage _ARGS_ ((MesgSummaryMessage *pDest, MesgSummaryMessage *pSource, SipError *pErr));
SipBool __sip_mwi_cloneSummaryLine _ARGS_ ((SummaryLine *pDest, SummaryLine *pSource, SipError *pErr));

#endif
SipBool __sip_cloneSipNameValuePair _ARGS_ ((SipNameValuePair *pDest, SipNameValuePair *pSource, SipError *pErr));
SipBool __sip_cloneSipHeaderOrderInfo _ARGS_ ((SipHeaderOrderInfo *pDest, SipHeaderOrderInfo *pSource, SipError *pErr));
SipBool __sip_cloneSipAuthorizationHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));

#ifdef SIP_PRIVACY
SipBool __sip_cloneSipPAssertIdHeader _ARGS_ ((SipPAssertIdHeader *dest, SipPAssertIdHeader *source, SipError *err));
SipBool __sip_cloneSipPAssertIdHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
SipBool __sip_cloneSipPPreferredIdHeader _ARGS_ ((SipPPreferredIdHeader *dest, SipPPreferredIdHeader *source, SipError *err));
SipBool __sip_cloneSipPPreferredIdHeaderList _ARGS_ ((SipList *pDest, SipList *pSource, SipError *pErr));
#endif /* # ifdef SIP_PRIVACY */

#ifdef SIP_CONF
/******************************************************************
**
** FUNCTION:  __sip_cloneSipJoinHeader
**
** DESCRIPTION: This function makes a deep copy of the fileds from
**	the Join Header structures "source" to "dest".
**
******************************************************************/
SipBool __sip_cloneSipJoinHeader _ARGS_( (SipJoinHeader *dest,\
				SipJoinHeader *source, SipError *err));
#endif

#ifdef SIP_SECURITY
SipBool __sip_cloneSipSecurityClientHeader _ARGS_((SipSecurityClientHeader *dest, SipSecurityClientHeader *source, SipError *err));
SipBool __sip_cloneSipSecurityVerifyHeader _ARGS_((SipSecurityVerifyHeader *dest, SipSecurityVerifyHeader *source, SipError *err)); 
SipBool __sip_cloneSipSecurityServerHeader _ARGS_((SipSecurityServerHeader *dest, SipSecurityServerHeader *source, SipError *err));
#endif

#ifdef SIP_3GPP
SipBool __sip_cloneSipPAssociatedUriHeaderList _ARGS_((SipList *pDest, SipList *pSource, SipError *pErr));

SipBool __sip_cloneSipPCalledPartyIdHeader _ARGS_((SipPCalledPartyIdHeader *to,SipPCalledPartyIdHeader *from,SipError *err));

SipBool __sip_cloneSipPAssociatedUriHeader _ARGS_((SipPAssociatedUriHeader *to,SipPAssociatedUriHeader *from,SipError *err));

SipBool __sip_cloneSipPVisitedNetworkIdHeader _ARGS_((SipPVisitedNetworkIdHeader *to,SipPVisitedNetworkIdHeader *from,SipError *err));

SipBool __sip_cloneSipPcfAddrHeader _ARGS_((SipPcfAddrHeader *to,SipPcfAddrHeader *from,SipError *err));

SipBool __sip_cloneSipPVisitedNetworkIdHeaderList _ARGS_((SipList *pDest, SipList *pSource, SipError *pErr));
#endif

#ifdef SIP_CONGEST
SipBool __sip_cloneSipRsrcPriorityHeader _ARGS_ ((SipRsrcPriorityHeader *to, SipRsrcPriorityHeader *from,SipError *err));

SipBool __sip_cloneSipAcceptRsrcPriorityHeader _ARGS_ ((SipAcceptRsrcPriorityHeader *to, SipAcceptRsrcPriorityHeader *from,SipError *err));

SipBool __sip_cloneSipRsrcPriorityHeaderList _ARGS_((SipList *pDest, SipList *pSource, SipError *pErr));  

SipBool __sip_cloneSipAcceptRsrcPriorityHeaderList _ARGS_((SipList *pDest, SipList *pSource, SipError *pErr));                                                       
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
