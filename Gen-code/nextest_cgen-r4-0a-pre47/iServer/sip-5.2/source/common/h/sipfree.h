/***********************************************************************
 ** FUNCTION:
 **             Has Free Function Declarations For all Structures

 *********************************************************************
 **
 ** FILENAME:
 ** sipfree.h
 **
 ** DESCRIPTION:
 ** This file contains code to free all structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 6/12/99   Arjun RC       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/

#ifndef __SIP_FREE_H_
#define __SIP_FREE_H_

#include <stdlib.h>
#include "sipstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


extern void sip_freeVoid _ARGS_((SIP_Pvoid s));

extern void sip_freeString _ARGS_((SIP_S8bit *s));

extern void sip_freeSipTimerKey _ARGS_((SipTimerKey *k));

extern void sip_freeSipTimerBuffer _ARGS_((SipTimerBuffer *b));

extern void sip_freeSdpOrigin _ARGS_((SdpOrigin *s));

extern void sip_freeSdpMedia _ARGS_((SdpMedia *m));

extern void sip_freeSdpAttr _ARGS_((SdpAttr *a));

extern void sip_freeSdpTime _ARGS_((SdpTime *t));

extern void sip_freeSdpConnection _ARGS_(( SdpConnection *c));

extern void sip_freeSdpMessage _ARGS_((SdpMessage *m));

extern void sip_freeSipParam _ARGS_((SipParam *c));

extern void sip_freeSipGenericChallenge _ARGS_((SipGenericChallenge *c));

extern void sip_freeSipWwwAuthenticateHeader _ARGS_((SipWwwAuthenticateHeader *h));

extern void sip_freeSipAuthenticationInfoHeader _ARGS_((SipAuthenticationInfoHeader *pHdr));

extern void sip_freeSipProxyAuthenticateHeader _ARGS_((SipProxyAuthenticateHeader *p));

extern void sip_freeSipNameValuePair _ARGS_((SipNameValuePair *n));
#ifdef SIP_MWI

extern void sip_mwi_freeSummaryLine _ARGS_((SummaryLine *s));

extern void sip_mwi_freeMesgSummaryMessage _ARGS_((MesgSummaryMessage *m));
#endif

extern void sip_freeSipWarningHeader _ARGS_((SipWarningHeader *w));

extern void sip_freeSipDateFormat _ARGS_((SipDateFormat *d));

extern void sip_freeSipTimeFormat _ARGS_((SipTimeFormat *t));

extern void sip_freeSipDateStruct _ARGS_((SipDateStruct *d));

extern void sip_freeSipDateHeader _ARGS_((SipDateHeader *d));

extern void sip_freeSipAllowHeader _ARGS_((SipAllowHeader *a));

extern void sip_freeSipRetryAfterHeader _ARGS_((SipRetryAfterHeader *r));

extern void sip_freeSipGenericCredential _ARGS_((SipGenericCredential *c));

extern void sip_freeSipAuthorizationHeader _ARGS_((SipAuthorizationHeader *a));

extern void sip_freeSipReferToHeader _ARGS_((SipReferToHeader *r));

extern void sip_freeSipReferredByHeader _ARGS_((SipReferredByHeader *r));

extern void sip_freeSipRespHeader _ARGS_((SipRespHeader *h));

extern void sip_freeSipRespKeyHeader _ARGS_(( SipRespKeyHeader* r));

extern void sip_freeSipUserAgentHeader _ARGS_((SipUserAgentHeader *u));

extern void sip_freeSipSubjectHeader _ARGS_((SipSubjectHeader *s));

extern void sip_freeSipProxyRequireHeader _ARGS_((SipProxyRequireHeader *p));

extern void sip_freeSipProxyAuthorizationHeader _ARGS_((SipProxyAuthorizationHeader *a));

extern void sip_freeSipPriorityHeader _ARGS_((SipPriorityHeader *p));

extern void sip_freeSipOrganizationHeader _ARGS_((SipOrganizationHeader *o));

extern void sip_freeSipContentTypeHeader _ARGS_((SipContentTypeHeader *c));

extern void sip_freeSipContentEncodingHeader _ARGS_((SipContentEncodingHeader *c));

extern void sip_freeSipHideHeader _ARGS_((SipHideHeader *h));

extern void sip_freeSipMaxForwardsHeader _ARGS_((SipMaxForwardsHeader *h));

extern void sip_freeSipContentLengthHeader _ARGS_((SipContentLengthHeader *c));

extern void sip_freeSipViaHeader _ARGS_((SipViaHeader *v));

extern void sip_freeSipUrl _ARGS_((SipUrl *u));

extern void sip_freeSipAddrSpec _ARGS_((SipAddrSpec *a));

extern void sip_freeSipToHeader _ARGS_((SipToHeader *t));

extern void sip_freeSipTimeStampHeader _ARGS_((SipTimeStampHeader *t));

extern void sip_freeSipRouteHeader _ARGS_((SipRouteHeader *r));

extern void sip_freeSipRecordRouteHeader _ARGS_((SipRecordRouteHeader *r));

extern void sip_freeSipRequireHeader _ARGS_((SipRequireHeader *r));

extern void sip_freeSipFromHeader _ARGS_((SipFromHeader *t));

extern void sip_freeSipExpiresStruct _ARGS_((SipExpiresStruct *e));

extern void sip_freeSipExpiresHeader _ARGS_((SipExpiresHeader *e));

extern void sip_freeSipEncryptionHeader _ARGS_((SipEncryptionHeader *e));

extern void sip_freeSipContactParam _ARGS_((SipContactParam *c));

extern void sip_freeSipContactHeader _ARGS_((SipContactHeader *c));

extern void sip_freeSipReplacesHeader _ARGS_((SipReplacesHeader *c));

extern void sip_freeSipReplyToHeader _ARGS_((SipReplyToHeader *c));

extern void sip_freeSipCseqHeader _ARGS_((SipCseqHeader *c));

extern void sip_freeSipCallIdHeader _ARGS_((SipCallIdHeader *c));

extern void sip_freeSipAcceptHeader _ARGS_((SipAcceptHeader *a));

extern void sip_freeSipAcceptEncodingHeader _ARGS_((SipAcceptEncodingHeader *a));

extern void sip_freeSipAcceptLangHeader _ARGS_((SipAcceptLangHeader *a));

extern void sip_freeSipUnsupportedHeader _ARGS_((SipUnsupportedHeader *a));

extern void sip_freeSipServerHeader _ARGS_((SipServerHeader *a));

extern void sip_freeSipReqHeader _ARGS_((SipReqHeader *s));

extern void sip_freeSipUnknownHeader _ARGS_((SipUnknownHeader *u));

extern void sip_freeSipBadHeader _ARGS_((SipBadHeader *u));

extern void sip_freeSipGeneralHeader _ARGS_((SipGeneralHeader *g));

extern void sip_freeSipReqLine _ARGS_((SipReqLine *r));

extern void sip_freeSipReqMessage _ARGS_((SipReqMessage *r));

extern void sip_freeSipStatusLine _ARGS_((SipStatusLine *s));

extern void sip_freeSipRespMessage _ARGS_((SipRespMessage *r));

extern void sip_freeSipMessage _ARGS_((SipMessage *s));

extern void sip_freeSipUnknownMessage _ARGS_((SipUnknownMessage *s));

extern void sip_freeSipMsgBody _ARGS_((SipMsgBody *s));

extern void sip_freeSipHeader _ARGS_((SipHeader *h));

extern void sip_freeSipAlertInfoHeader _ARGS_ ((SipAlertInfoHeader* pHdr));

extern void sip_freeSipInReplyToHeader _ARGS_ ((SipInReplyToHeader* pHdr));

extern void sip_freeSipAlsoHeader _ARGS_ ((SipAlsoHeader* pHdr));

extern void sip_freeSipCallInfoHeader _ARGS_ ((SipCallInfoHeader* pHdr));

extern void sip_freeSipErrorInfoHeader _ARGS_ ((SipErrorInfoHeader* pHdr));

extern void sip_freeSipMinExpiresHeader _ARGS_ ((SipMinExpiresHeader* pHdr));

extern void sip_freeSipContentDispositionHeader _ARGS_ ((SipContentDispositionHeader* pHdr));

extern void sip_freeSipReasonHeader _ARGS_ ((SipReasonHeader* pHdr));

extern void sip_freeSipContentLanguageHeader _ARGS_ ((SipContentLanguageHeader* pHdr));

#ifdef SIP_SESSIONTIMER
extern void sip_freeSipMinSEHeader _ARGS_ ((SipMinSEHeader* pHdr));
extern void sip_freeSipSessionExpiresHeader _ARGS_((SipSessionExpiresHeader *e));
#endif


/* These are declarations for the INTERNAL siplist functions */

extern void __sip_freeString _ARGS_((SIP_Pvoid s));

extern void __sip_freeSipTimerKey _ARGS_((SIP_Pvoid k));

extern void __sip_freeSdpOrigin _ARGS_((SIP_Pvoid s));

extern void __sip_freeSdpMedia _ARGS_((SIP_Pvoid m));

extern void __sip_freeSdpAttr _ARGS_((SIP_Pvoid a));

extern void __sip_freeSdpTime _ARGS_((SIP_Pvoid t));

extern void __sip_freeSdpConnection _ARGS_((SIP_Pvoid c));

extern void __sip_freeSdpMessage _ARGS_((SIP_Pvoid m));

extern void __sip_freeSipParam _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipGenericChallenge _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipNameValuePair _ARGS_((SIP_Pvoid n));
#ifdef SIP_MWI
extern void __sip_mwi_freeSummaryLine _ARGS_((SIP_Pvoid s));

#endif
extern void __sip_freeSipWwwAuthenticateHeader _ARGS_((SIP_Pvoid h));

extern void __sip_freeSipProxyAuthenticateHeader _ARGS_((SIP_Pvoid p));

/* extern void __sip_freeSipAuthParam _ARGS_((SIP_Pvoid a)); */

extern void __sip_freeSipWarningHeader _ARGS_((SIP_Pvoid w));

extern void __sip_freeSipDateFormat _ARGS_((SIP_Pvoid d));

extern void __sip_freeSipTimeFormat _ARGS_((SIP_Pvoid t));

extern void __sip_freeSipDateStruct _ARGS_((SIP_Pvoid d));

extern void __sip_freeSipDateHeader _ARGS_((SIP_Pvoid d));

extern void __sip_freeSipAllowHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipRetryAfterHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipGenericCredential _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipAuthorizationHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipReferToHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipReferredByHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipRespHeader _ARGS_((SIP_Pvoid h));

extern void __sip_freeSipRespKeyHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipUserAgentHeader _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipReplacesHeader _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipReplyToHeader _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipSubjectHeader _ARGS_((SIP_Pvoid s));

extern void __sip_freeSipProxyRequireHeader _ARGS_((SIP_Pvoid p));

extern void __sip_freeSipProxyAuthorizationHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipPriorityHeader _ARGS_((SIP_Pvoid p));

extern void __sip_freeSipOrganizationHeader _ARGS_((SIP_Pvoid o));

extern void __sip_freeSipContentTypeHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipContentEncodingHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipHideHeader _ARGS_((SIP_Pvoid h));

extern void __sip_freeSipMaxForwardsHeader _ARGS_((SIP_Pvoid h));


extern void __sip_freeSipContentLengthHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipViaHeader _ARGS_((SIP_Pvoid v));


extern void __sip_freeSipUrl _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipAddrSpec _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipToHeader _ARGS_((SIP_Pvoid t));

extern void __sip_freeSipTimeStampHeader _ARGS_((SIP_Pvoid t));

extern void __sip_freeSipRouteHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipRecordRouteHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipRequireHeader _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipFromHeader _ARGS_((SIP_Pvoid t));

extern void __sip_freeSipExpiresStruct _ARGS_((SIP_Pvoid e));

extern void __sip_freeSipExpiresHeader _ARGS_((SIP_Pvoid e));

extern void __sip_freeSipEncryptionHeader _ARGS_((SIP_Pvoid e));

extern void __sip_freeSipContactParam _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipContactHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipCseqHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipCallIdHeader _ARGS_((SIP_Pvoid c));

extern void __sip_freeSipAcceptHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipAcceptEncodingHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipAcceptLangHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipUnsupportedHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipServerHeader _ARGS_((SIP_Pvoid a));

extern void __sip_freeSipReqHeader _ARGS_((SIP_Pvoid s));

extern void __sip_freeSipUnknownHeader _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipBadHeader _ARGS_((SIP_Pvoid u));

extern void __sip_freeSipGeneralHeader _ARGS_((SIP_Pvoid g));

extern void __sip_freeSipReqLine _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipReqMessage _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipStatusLine _ARGS_((SIP_Pvoid s));

extern void __sip_freeSipRespMessage _ARGS_((SIP_Pvoid r));

extern void __sip_freeSipMessage _ARGS_((SIP_Pvoid s));

extern void __sip_freeSipHeader _ARGS_((SIP_Pvoid h));

extern void __sip_freeSipHeaderOrderInfo _ARGS_((SIP_Pvoid h));

extern void __sip_freeSipMsgBody _ARGS_((SIP_Pvoid s));

extern void sip_freeSipSupportedHeader _ARGS_ ((SipSupportedHeader *pHdr));

extern void __sip_freeSipSupportedHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipAlertInfoHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipInReplyToHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipCallInfoHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipErrorInfoHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipMinExpiresHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipContentDispositionHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipReasonHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipContentLanguageHeader _ARGS_ ((SIP_Pvoid pHdr));

extern void __sip_freeSipAuthenticationInfoHeader _ARGS_((SIP_Pvoid pHdr));

extern void __sip_freeSipAlsoHeader _ARGS_ ((SIP_Pvoid pHdr));

#ifdef SIP_SESSIONTIMER
extern void __sip_freeSipMinSEHeader _ARGS_ ((SIP_Pvoid pHdr));
extern void __sip_freeSipSessionExpiresHeader _ARGS_((SIP_Pvoid e));
#endif

extern void sip_freeEventContext(SipEventContext *pContext);

#ifdef SIP_PRIVACY
extern void sip_freeSipPAssertIdHeader _ARGS_ ((SipPAssertIdHeader *pPAssertId)) ;
extern void __sip_freeSipPAssertIdHeader _ARGS_ ((SIP_Pvoid pHdr)) ;
extern void sip_freeSipPPreferredIdHeader _ARGS_ ((SipPPreferredIdHeader *pPId)) ;
extern void __sip_freeSipPPreferredIdHeader _ARGS_ ((SIP_Pvoid pHdr)) ;
extern void sip_freeSipPrivacyHeader _ARGS_((SipPrivacyHeader *pHdr));
extern void __sip_freeSipPrivacyHeader _ARGS_((SIP_Pvoid pHdr));
#endif /* # ifdef SIP_PRIVACY */

# ifdef SIP_3GPP
extern void sip_freeSipServiceRouteHeader _ARGS_((SipServiceRouteHeader *pPath));
extern void __sip_freeSipServiceRouteHeader _ARGS_((SIP_Pvoid pHdr));
extern void sip_freeSipPathHeader _ARGS_((SipPathHeader *pPath));
extern void __sip_freeSipPathHeader _ARGS_((SIP_Pvoid pHdr));
extern void sip_freeSipPanInfoHeader _ARGS_((SipPanInfoHeader *pHdr));
extern void __sip_freeSipPanInfoHeader _ARGS_((SIP_Pvoid pHdr));

extern void sip_freeSipPcVectorHeader _ARGS_((SipPcVectorHeader *pHdr));
extern void __sip_freeSipPcVectorHeader _ARGS_((SIP_Pvoid pHdr));
# endif

#ifdef SIP_CONGEST
extern void sip_freeSipRsrcPriorityHeader _ARGS_((SipRsrcPriorityHeader
                                                  *pHdr));
extern void __sip_freeSipRsrcPriorityHeader _ARGS_((SIP_Pvoid pHdr));

extern void sip_freeSipAcceptRsrcPriorityHeader _ARGS_((SipAcceptRsrcPriorityHeader  *pHdr));
extern void __sip_freeSipAcceptRsrcPriorityHeader _ARGS_((SIP_Pvoid pHdr));

# endif


#ifdef SIP_CONF
/*Join Header*/
extern void sip_freeSipJoinHeader _ARGS_((SipJoinHeader *c));
extern void __sip_freeSipJoinHeader _ARGS_((SIP_Pvoid u));
# endif

#ifdef SIP_3GPP
/*P-Associated-URI header */
extern void sip_freeSipPAssociatedUriHeader _ARGS_((SipPAssociatedUriHeader *pHdr));
extern void __sip_freeSipPAssociatedUriHeader _ARGS_((SIP_Pvoid r));

/*P-Called-Party-Id header */
extern void sip_freeSipPCalledPartyIdHeader _ARGS_((SipPCalledPartyIdHeader *r));
extern void __sip_freeSipPCalledPartyIdHeader _ARGS_((SIP_Pvoid r));

/*P-Visited-Network-Id header */
extern void sip_freeSipPVisitedNetworkIdHeader _ARGS_((SipPVisitedNetworkIdHeader *r));
extern void __sip_freeSipPVisitedNetworkIdHeader _ARGS_((SIP_Pvoid r));

/*P-Charging-Function-Addresses header */
extern void sip_freeSipPcfAddrHeader _ARGS_((SipPcfAddrHeader *r));
extern void __sip_freeSipPcfAddrHeader _ARGS_((SIP_Pvoid r));
#endif

#ifdef SIP_SECURITY
extern void sip_freeSipSecurityClientHeader _ARGS_((SipSecurityClientHeader *s));
extern void __sip_freeSipSecurityClientHeader _ARGS_((SIP_Pvoid s));

extern void sip_freeSipSecurityServerHeader _ARGS_((SipSecurityServerHeader *s));
extern void __sip_freeSipSecurityServerHeader _ARGS_((SIP_Pvoid s));

extern void sip_freeSipSecurityVerifyHeader _ARGS_((SipSecurityVerifyHeader *s));
extern void __sip_freeSipSecurityVerifyHeader _ARGS_((SIP_Pvoid s));
#endif

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif

