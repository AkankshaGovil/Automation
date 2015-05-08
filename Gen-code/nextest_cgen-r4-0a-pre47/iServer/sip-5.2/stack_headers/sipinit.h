/***********************************************************************
 ** FUNCTION:
 **             Has Init Functions For all Structures

 *********************************************************************
 **
 ** FILENAME:
 ** sipinit.h
 **
 ** DESCRIPTION:
 ** This file contains code to init all structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 8/12/99   R Preethy       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __SIP_INIT_H_
#define __SIP_INIT_H_


#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


extern SIP_S8bit* sip_getPart _ARGS_((void ));

extern SIP_S8bit* sip_getVersion _ARGS_((void ));

extern SipBool sip_initSipTimerKey _ARGS_((SipTimerKey **k,SipError *err));

extern SipBool sip_initSipTimerBuffer _ARGS_((SipTimerBuffer **b,SipError *err));

extern SipBool sip_initSdpOrigin _ARGS_((SdpOrigin **s,SipError *err));

extern SipBool sip_initSdpMedia _ARGS_((SdpMedia **m,SipError *err));

extern SipBool sip_initSdpAttr _ARGS_((SdpAttr **a,SipError *err));

extern SipBool sip_initSdpTime _ARGS_((SdpTime **t,SipError *err));

extern SipBool sip_initSdpConnection _ARGS_((SdpConnection **c,SipError *err));

extern SipBool sip_initSdpMessage _ARGS_((SdpMessage **m,SipError *err));

extern SipBool sip_initSipGenericChallenge _ARGS_((SipGenericChallenge **c,SipError *err));

extern SipBool sip_initSipWwwAuthenticateHeader _ARGS_((SipWwwAuthenticateHeader **h,SipError *err));

extern SipBool sip_initSipProxyAuthenticateHeader _ARGS_((SipProxyAuthenticateHeader **p,SipError *err));

/*extern SipBool sip_initSipAuthParam _ARGS_((SipAuthParam **a,SipError *err));(*/

extern SipBool sip_initSipWarningHeader _ARGS_((SipWarningHeader **w,SipError *err));

extern SipBool sip_initSipDateFormat _ARGS_((SipDateFormat **d,SipError *err));

extern SipBool sip_initSipTimeFormat _ARGS_((SipTimeFormat **t,SipError *err));

extern SipBool sip_initSipDateStruct _ARGS_((SipDateStruct **d,SipError *err));

extern SipBool sip_initSipDateHeader _ARGS_((SipDateHeader **d,SipError *err));

extern SipBool sip_initSipAllowHeader _ARGS_((SipAllowHeader **a,SipError *err));

extern SipBool sip_initSipRetryAfterHeader _ARGS_((SipRetryAfterHeader **r,en_ExpiresType type,SipError *err));

extern SipBool sip_initSipGenericCredential _ARGS_((SipGenericCredential **c,en_CredentialType type,SipError *err));

extern SipBool sip_initSipAuthorizationHeader _ARGS_((SipAuthorizationHeader **a,SipError *err));

extern SipBool sip_initSipRespHeader _ARGS_((SipRespHeader **h,SipError *err));

extern SipBool sip_initSipRespKeyHeader _ARGS_((SipRespKeyHeader **r,SipError *err));

extern SipBool sip_initSipUserAgentHeader _ARGS_((SipUserAgentHeader **u,SipError *err));

extern SipBool sip_initSipSubjectHeader _ARGS_((SipSubjectHeader **s,SipError *err));

extern SipBool sip_initSipProxyRequireHeader _ARGS_((SipProxyRequireHeader **p,SipError *err));

extern SipBool sip_initSipProxyAuthorizationHeader _ARGS_((SipProxyAuthorizationHeader **a,SipError *err));

extern SipBool sip_initSipPriorityHeader _ARGS_((SipPriorityHeader **p,SipError *err));

extern SipBool sip_initSipOrganizationHeader _ARGS_((SipOrganizationHeader **o,SipError *err));

extern SipBool sip_initSipContentTypeHeader _ARGS_((SipContentTypeHeader **c,SipError *err));

#ifdef SIP_MWI
extern SipBool sip_mwi_initMesgSummaryMessage _ARGS_((MesgSummaryMessage **m,SipError *err));

extern SipBool sip_mwi_initSummaryLine _ARGS_((SummaryLine **c,SipError *err));

#endif
extern SipBool sip_initSipNameValuePair _ARGS_((SipNameValuePair **c,SipError *err));

extern SipBool sip_initSipContentEncodingHeader _ARGS_((SipContentEncodingHeader **c,SipError *err));

extern SipBool sip_initSipHideHeader _ARGS_((SipHideHeader **h,SipError *err));

extern SipBool sip_initSipMaxForwardsHeader _ARGS_((SipMaxForwardsHeader **h,SipError *err));

/* extern SipBool sip_initSipViaParam _ARGS_((SipViaParam **v,en_ViaParamType type,SipError *err)); */

extern SipBool sip_initSipContentLengthHeader _ARGS_((SipContentLengthHeader **c,SipError *err));

extern SipBool sip_initSipViaHeader _ARGS_((SipViaHeader **v,SipError *err));

/*extern SipBool sip_initSipUrlParam _ARGS_((SipUrlParam **u,en_UrlParamType type,SipError *err));*/

extern SipBool sip_initSipUrl _ARGS_((SipUrl **u,SipError *err));

extern SipBool sip_initSipAddrSpec _ARGS_((SipAddrSpec **a,en_AddrType type,SipError *err));

extern SipBool sip_initSipToHeader _ARGS_((SipToHeader **t,SipError *err));

extern SipBool sip_initSipTimeStampHeader _ARGS_((SipTimeStampHeader **t,SipError *err));

extern SipBool sip_initSipRouteHeader _ARGS_((SipRouteHeader **r,SipError *err));

extern SipBool sip_initSipRecordRouteHeader _ARGS_((SipRecordRouteHeader **r,SipError *err));

extern SipBool sip_initSipRequireHeader _ARGS_((SipRequireHeader **r,SipError *err));

extern SipBool sip_initSipFromHeader _ARGS_((SipFromHeader **t,SipError *err));

extern SipBool sip_initSipExpiresStruct _ARGS_((SipExpiresStruct **e,en_ExpiresType type,SipError *err));

extern SipBool sip_initSipExpiresHeader _ARGS_((SipExpiresHeader **e,en_ExpiresType type,SipError *err));

extern SipBool sip_initSipEncryptionHeader _ARGS_((SipEncryptionHeader **e,SipError *err));

extern SipBool sip_initSipContactParam _ARGS_((SipContactParam **c,en_ContactParamsType type,SipError *err));

extern SipBool sip_initSipContactHeader _ARGS_((SipContactHeader **c,en_ContactType type,SipError *err));

extern SipBool sip_initSipCseqHeader _ARGS_((SipCseqHeader **c,SipError *err));

extern SipBool sip_initSipCallIdHeader _ARGS_((SipCallIdHeader **c,SipError *err));

extern SipBool sip_initSipAcceptHeader _ARGS_((SipAcceptHeader **a,SipError *err));

extern SipBool sip_initSipAcceptEncodingHeader _ARGS_((SipAcceptEncodingHeader **a,SipError *err));

extern SipBool sip_initSipAcceptLangHeader _ARGS_((SipAcceptLangHeader **a,SipError *err));

extern SipBool sip_initSipReqHeader _ARGS_((SipReqHeader **s,SipError *err));

extern SipBool sip_initSipGeneralHeader _ARGS_((SipGeneralHeader **g,SipError *err));

extern SipBool sip_initSipReqLine _ARGS_((SipReqLine **r,SipError *err));

extern SipBool sip_initSipReqMessage _ARGS_((SipReqMessage **r,SipError *err));

extern SipBool sip_initSipStatusLine _ARGS_((SipStatusLine **s,SipError *err));

extern SipBool sip_initSipRespMessage _ARGS_((SipRespMessage **r,SipError *err));

extern SipBool sip_initSipUnknownMessage _ARGS_((SipUnknownMessage **s,SipError *err));

extern SipBool sip_initSipMsgBody _ARGS_((SipMsgBody **s,en_SipMsgBodyType type,SipError *err));

extern SipBool sip_initSipMessage _ARGS_((SipMessage **s,en_SipMessageType type,SipError *err));

extern SipBool sip_initSipHeader _ARGS_((SipHeader **h,en_HeaderType type,SipError *err));

extern SipBool sip_initSipUnknownHeader _ARGS_((SipUnknownHeader **h,SipError *err));

extern SipBool sip_initSipHeaderOrderInfo _ARGS_((SipHeaderOrderInfo **h,SipError *err));

extern SipBool sip_initSipParam _ARGS_((SipParam **p,SipError *err));

extern SipBool sip_initSipServerHeader _ARGS_((SipServerHeader **s,SipError *err));

extern SipBool sip_initSipUnsupportedHeader _ARGS_((SipUnsupportedHeader **u,SipError *err));

extern SipBool sip_initSipSupportedHeader _ARGS_ ((SipSupportedHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipAlertInfoHeader _ARGS_ ((SipAlertInfoHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipReferToHeader _ARGS_ ((SipReferToHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipReferredByHeader _ARGS_ ((SipReferredByHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipInReplyToHeader _ARGS_ ((SipInReplyToHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipCallInfoHeader _ARGS_ ((SipCallInfoHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipContentDispositionHeader _ARGS_ ((SipContentDispositionHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipReasonHeader _ARGS_ ((SipReasonHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipContentLanguageHeader _ARGS_ ((SipContentLanguageHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipErrorInfoHeader _ARGS_ ((SipErrorInfoHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipMinExpiresHeader _ARGS_ ((SipMinExpiresHeader **ppHdr, SipError *ppErr));

extern SipBool sip_initSipAlsoHeader _ARGS_ ((SipAlsoHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipReplacesHeader _ARGS_ ((SipReplacesHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipReplyToHeader _ARGS_ ((SipReplyToHeader **ppHdr, SipError *pErr));

extern SipBool sip_initSipAuthenticationInfoHeader _ARGS_((SipAuthenticationInfoHeader **ppHdr,SipError *pErr));

#ifdef SIP_SESSIONTIMER
extern SipBool sip_initSipMinSEHeader _ARGS_ ((SipMinSEHeader **ppHdr, SipError *ppErr));

extern SipBool sip_initSipSessionExpiresHeader _ARGS_((SipSessionExpiresHeader **e,SipError *err));
#endif

extern SipBool sip_initSipBadHeader _ARGS_((SipBadHeader **e,\
	en_HeaderType dType,SipError *err));

#ifdef SIP_PRIVACY
extern SipBool sip_initSipPAssertIdHeader _ARGS_((SipPAssertIdHeader **pHdr,\
					SipError *pErr));
extern SipBool sip_initSipPPreferredIdHeader _ARGS_((SipPPreferredIdHeader 
					**pHdr,SipError *pErr));
extern SipBool sip_initSipPrivacyHeader _ARGS_((SipPrivacyHeader **pHdr,\
	SipError *perr));
#endif

# ifdef SIP_3GPP
extern SipBool sip_initSipServiceRouteHeader _ARGS_((SipServiceRouteHeader **pHdr,\
					SipError *err));
extern SipBool sip_initSipPathHeader _ARGS_((SipPathHeader **pHdr,\
					SipError *err));
extern SipBool sip_initSipPanInfoHeader _ARGS_((SipPanInfoHeader **pHdr,\
                    SipError *err));
extern SipBool sip_initSipPcVectorHeader _ARGS_((SipPcVectorHeader **pHdr,\
                    SipError *err));

# endif

#ifdef SIP_CONGEST
extern SipBool sip_initSipRsrcPriorityHeader _ARGS_((SipRsrcPriorityHeader
                                                     **ppHdr, SipError
                                                     *pErr));
extern SipBool sip_initSipAcceptRsrcPriorityHeader _ARGS_((SipAcceptRsrcPriorityHeader **ppHdr, SipError *pErr));

# endif

#ifdef SIP_CONF
extern SipBool sip_initSipJoinHeader _ARGS_ ((SipJoinHeader **ppHdr, SipError *pErr));
#endif

# ifdef SIP_3GPP
/*P-Associated-URI header */
extern SipBool sip_initSipPAssociatedUriHeader _ARGS_((SipPAssociatedUriHeader **r,SipError *err));

/*P-Visited-Network-Id header */
extern SipBool sip_initSipPVisitedNetworkIdHeader _ARGS_((SipPVisitedNetworkIdHeader **r,SipError *err));

/*P-Called-Party-Id header */
extern SipBool sip_initSipPCalledPartyIdHeader _ARGS_((SipPCalledPartyIdHeader **r,SipError *err));

/*P-Charging-Function-Addresses header */
extern SipBool sip_initSipPcfAddrHeader _ARGS_((SipPcfAddrHeader **r,SipError *err));
# endif

#ifdef SIP_SECURITY
extern SipBool sip_initSipSecurityClientHeader _ARGS_((SipSecurityClientHeader **v, SipError *err));
extern SipBool sip_initSipSecurityServerHeader _ARGS_((SipSecurityServerHeader **v, SipError *err));
extern SipBool sip_initSipSecurityVerifyHeader _ARGS_((SipSecurityVerifyHeader **v, SipError *err));
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
