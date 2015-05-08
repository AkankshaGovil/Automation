
#ifndef _SIP_SERIALIZE_H
#define _SIP_SERIALIZE_H

#include <portlayer.h>
#include <siplist.h>
#include <sipinit.h>
#include <sipfree.h>
#include <rprinit.h>
#include <rprfree.h>
#ifdef SIP_CCP
#include <ccpinit.h>
#include <ccpfree.h>
#endif
#ifdef SIP_PRES
#include <presstruct.h>
#include <pres.h>
#endif
#include <sipbcptinit.h>
#include <sipbcptfree.h>
#ifdef SIP_IMPP
#include <imppinit.h>
#include <imppfree.h>
#endif
#ifdef SIP_TEL
#include <telinit.h>
#include <telfree.h>
#endif
#include <sipcommon.h>
#ifdef SIP_DCS
#include <dcsinit.h>
#include <dcsfree.h>
#endif

#include <byteordering.h>

#ifdef SIP_TXN_LAYER
#include <siptxnstruct.h>
#include <txninit.h>
#include <txnfree.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* The sip_init Procedures prototype definations 
*	SipRequestDispositionHeader
**************************************************************/
#ifdef SIP_CCP
extern SipBool sip_initSipRequestDispositionHeader (SipRequestDispositionHeader **ppHdr, SipError *pErr);
#endif 

SIP_U32bit sip_serializeSipReqLine _ARGS_((SipReqLine *pSipReqLine,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReqLine _ARGS_((SipReqLine **pSipReqLine,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPanInfoHeader _ARGS_((SipPanInfoHeader *pSipPanInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPanInfoHeader _ARGS_((SipPanInfoHeader **pSipPanInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipStatusLine _ARGS_((SipStatusLine *pSipStatusLine,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipStatusLine _ARGS_((SipStatusLine **pSipStatusLine,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_CCP_VERSION10
SIP_U32bit sip_serializeSipAcceptContactHeader _ARGS_((SipAcceptContactHeader *pSipAcceptContactHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION10

SIP_U32bit sip_deserializeSipAcceptContactHeader _ARGS_((SipAcceptContactHeader **pSipAcceptContactHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CCP_VERSION06
SIP_U32bit sip_serializeSipAcceptContactHeader _ARGS_((SipAcceptContactHeader *pSipAcceptContactHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION06

SIP_U32bit sip_deserializeSipAcceptContactHeader _ARGS_((SipAcceptContactHeader **pSipAcceptContactHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipFromHeader _ARGS_((SipFromHeader *pSipFromHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipFromHeader _ARGS_((SipFromHeader **pSipFromHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReplyToHeader _ARGS_((SipReplyToHeader *pSipReplyToHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReplyToHeader _ARGS_((SipReplyToHeader **pSipReplyToHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReplacesHeader _ARGS_((SipReplacesHeader *pSipReplacesHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReplacesHeader _ARGS_((SipReplacesHeader **pSipReplacesHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAuthenticationInfoHeader _ARGS_((SipAuthenticationInfoHeader *pSipAuthenticationInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAuthenticationInfoHeader _ARGS_((SipAuthenticationInfoHeader **pSipAuthenticationInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipNameValuePair _ARGS_((SipNameValuePair *pSipNameValuePair,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipNameValuePair _ARGS_((SipNameValuePair **pSipNameValuePair,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipToHeader _ARGS_((SipToHeader *pSipToHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipToHeader _ARGS_((SipToHeader **pSipToHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContentLengthHeader _ARGS_((SipContentLengthHeader *pSipContentLengthHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContentLengthHeader _ARGS_((SipContentLengthHeader **pSipContentLengthHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContentTypeHeader _ARGS_((SipContentTypeHeader *pSipContentTypeHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContentTypeHeader _ARGS_((SipContentTypeHeader **pSipContentTypeHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContentEncodingHeader _ARGS_((SipContentEncodingHeader *pSipContentEncodingHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContentEncodingHeader _ARGS_((SipContentEncodingHeader **pSipContentEncodingHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAcceptHeader _ARGS_((SipAcceptHeader *pSipAcceptHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAcceptHeader _ARGS_((SipAcceptHeader **pSipAcceptHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAcceptEncodingHeader _ARGS_((SipAcceptEncodingHeader *pSipAcceptEncodingHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAcceptEncodingHeader _ARGS_((SipAcceptEncodingHeader **pSipAcceptEncodingHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAcceptLangHeader _ARGS_((SipAcceptLangHeader *pSipAcceptLangHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAcceptLangHeader _ARGS_((SipAcceptLangHeader **pSipAcceptLangHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipCallIdHeader _ARGS_((SipCallIdHeader *pSipCallIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipCallIdHeader _ARGS_((SipCallIdHeader **pSipCallIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContactHeader _ARGS_((SipContactHeader *pSipContactHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContactHeader _ARGS_((SipContactHeader **pSipContactHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipCseqHeader _ARGS_((SipCseqHeader *pSipCseqHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipCseqHeader _ARGS_((SipCseqHeader **pSipCseqHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipDateHeader _ARGS_((SipDateHeader *pSipDateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipDateHeader _ARGS_((SipDateHeader **pSipDateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipEncryptionHeader _ARGS_((SipEncryptionHeader *pSipEncryptionHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipEncryptionHeader _ARGS_((SipEncryptionHeader **pSipEncryptionHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipExpiresHeader _ARGS_((SipExpiresHeader *pSipExpiresHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipExpiresHeader _ARGS_((SipExpiresHeader **pSipExpiresHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_SESSIONTIMER
SIP_U32bit sip_serializeSipSessionExpiresHeader _ARGS_((SipSessionExpiresHeader *pSipSessionExpiresHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_SESSIONTIMER

SIP_U32bit sip_deserializeSipSessionExpiresHeader _ARGS_((SipSessionExpiresHeader **pSipSessionExpiresHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_SESSIONTIMER
SIP_U32bit sip_serializeSipMinSEHeader _ARGS_((SipMinSEHeader *pSipMinSEHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_SESSIONTIMER

SIP_U32bit sip_deserializeSipMinSEHeader _ARGS_((SipMinSEHeader **pSipMinSEHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipRecordRouteHeader _ARGS_((SipRecordRouteHeader *pSipRecordRouteHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRecordRouteHeader _ARGS_((SipRecordRouteHeader **pSipRecordRouteHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipTimeStampHeader _ARGS_((SipTimeStampHeader *pSipTimeStampHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipTimeStampHeader _ARGS_((SipTimeStampHeader **pSipTimeStampHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipViaHeader _ARGS_((SipViaHeader *pSipViaHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipViaHeader _ARGS_((SipViaHeader **pSipViaHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRequireHeader _ARGS_((SipRequireHeader *pSipRequireHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRequireHeader _ARGS_((SipRequireHeader **pSipRequireHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipOrganizationHeader _ARGS_((SipOrganizationHeader *pSipOrganizationHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipOrganizationHeader _ARGS_((SipOrganizationHeader **pSipOrganizationHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContentDispositionHeader _ARGS_((SipContentDispositionHeader *pSipContentDispositionHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContentDispositionHeader _ARGS_((SipContentDispositionHeader **pSipContentDispositionHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContentLanguageHeader _ARGS_((SipContentLanguageHeader *pSipContentLanguageHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContentLanguageHeader _ARGS_((SipContentLanguageHeader **pSipContentLanguageHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipCallInfoHeader _ARGS_((SipCallInfoHeader *pSipCallInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipCallInfoHeader _ARGS_((SipCallInfoHeader **pSipCallInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAllowHeader _ARGS_((SipAllowHeader *pSipAllowHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAllowHeader _ARGS_((SipAllowHeader **pSipAllowHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipUserAgentHeader _ARGS_((SipUserAgentHeader *pSipUserAgentHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipUserAgentHeader _ARGS_((SipUserAgentHeader **pSipUserAgentHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_IMPP
SIP_U32bit sip_serializeSipAllowEventsHeader _ARGS_((SipAllowEventsHeader *pSipAllowEventsHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_IMPP

SIP_U32bit sip_deserializeSipAllowEventsHeader _ARGS_((SipAllowEventsHeader **pSipAllowEventsHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_IMPP
SIP_U32bit sip_serializeSipEventHeader _ARGS_((SipEventHeader *pSipEventHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_IMPP

SIP_U32bit sip_deserializeSipEventHeader _ARGS_((SipEventHeader **pSipEventHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_IMPP
SIP_U32bit sip_serializeSipSubscriptionStateHeader _ARGS_((SipSubscriptionStateHeader *pSipSubscriptionStateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_IMPP

SIP_U32bit sip_deserializeSipSubscriptionStateHeader _ARGS_((SipSubscriptionStateHeader **pSipSubscriptionStateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipUnknownHeader _ARGS_((SipUnknownHeader *pSipUnknownHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipUnknownHeader _ARGS_((SipUnknownHeader **pSipUnknownHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipMimeVersionHeader _ARGS_((SipMimeVersionHeader *pSipMimeVersionHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMimeVersionHeader _ARGS_((SipMimeVersionHeader **pSipMimeVersionHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipSupportedHeader _ARGS_((SipSupportedHeader *pSipSupportedHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipSupportedHeader _ARGS_((SipSupportedHeader **pSipSupportedHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsMediaAuthorizationHeader _ARGS_((SipDcsMediaAuthorizationHeader *pSipDcsMediaAuthorizationHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsMediaAuthorizationHeader _ARGS_((SipDcsMediaAuthorizationHeader **pSipDcsMediaAuthorizationHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsAnonymityHeader _ARGS_((SipDcsAnonymityHeader *pSipDcsAnonymityHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsAnonymityHeader _ARGS_((SipDcsAnonymityHeader **pSipDcsAnonymityHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsRpidPrivacyHeader _ARGS_((SipDcsRpidPrivacyHeader *pSipDcsRpidPrivacyHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsRpidPrivacyHeader _ARGS_((SipDcsRpidPrivacyHeader **pSipDcsRpidPrivacyHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsStateHeader _ARGS_((SipDcsStateHeader *pSipDcsStateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsStateHeader _ARGS_((SipDcsStateHeader **pSipDcsStateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsBillingIdHeader _ARGS_((SipDcsBillingIdHeader *pSipDcsBillingIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsBillingIdHeader _ARGS_((SipDcsBillingIdHeader **pSipDcsBillingIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsBillingInfoHeader _ARGS_((SipDcsBillingInfoHeader *pSipDcsBillingInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsBillingInfoHeader _ARGS_((SipDcsBillingInfoHeader **pSipDcsBillingInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsOspsHeader _ARGS_((SipDcsOspsHeader *pSipDcsOspsHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsOspsHeader _ARGS_((SipDcsOspsHeader **pSipDcsOspsHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipSessionHeader _ARGS_((SipSessionHeader *pSipSessionHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipSessionHeader _ARGS_((SipSessionHeader **pSipSessionHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsGateHeader _ARGS_((SipDcsGateHeader *pSipDcsGateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsGateHeader _ARGS_((SipDcsGateHeader **pSipDcsGateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsLaesHeader _ARGS_((SipDcsLaesHeader *pSipDcsLaesHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsLaesHeader _ARGS_((SipDcsLaesHeader **pSipDcsLaesHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsRemotePartyIdHeader _ARGS_((SipDcsRemotePartyIdHeader *pSipDcsRemotePartyIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsRemotePartyIdHeader _ARGS_((SipDcsRemotePartyIdHeader **pSipDcsRemotePartyIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsTracePartyIdHeader _ARGS_((SipDcsTracePartyIdHeader *pSipDcsTracePartyIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsTracePartyIdHeader _ARGS_((SipDcsTracePartyIdHeader **pSipDcsTracePartyIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsRedirectHeader _ARGS_((SipDcsRedirectHeader *pSipDcsRedirectHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsRedirectHeader _ARGS_((SipDcsRedirectHeader **pSipDcsRedirectHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_DCS
SIP_U32bit sip_serializeSipDcsAcctEntry _ARGS_((SipDcsAcctEntry *pSipDcsAcctEntry,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_DCS

SIP_U32bit sip_deserializeSipDcsAcctEntry _ARGS_((SipDcsAcctEntry **pSipDcsAcctEntry,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipAuthorizationHeader _ARGS_((SipAuthorizationHeader *pSipAuthorizationHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAuthorizationHeader _ARGS_((SipAuthorizationHeader **pSipAuthorizationHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipHideHeader _ARGS_((SipHideHeader *pSipHideHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipHideHeader _ARGS_((SipHideHeader **pSipHideHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipMaxForwardsHeader _ARGS_((SipMaxForwardsHeader *pSipMaxForwardsHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMaxForwardsHeader _ARGS_((SipMaxForwardsHeader **pSipMaxForwardsHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipPriorityHeader _ARGS_((SipPriorityHeader *pSipPriorityHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipPriorityHeader _ARGS_((SipPriorityHeader **pSipPriorityHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipProxyAuthorizationHeader _ARGS_((SipProxyAuthorizationHeader *pSipProxyAuthorizationHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipProxyAuthorizationHeader _ARGS_((SipProxyAuthorizationHeader **pSipProxyAuthorizationHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipProxyRequireHeader _ARGS_((SipProxyRequireHeader *pSipProxyRequireHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipProxyRequireHeader _ARGS_((SipProxyRequireHeader **pSipProxyRequireHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRouteHeader _ARGS_((SipRouteHeader *pSipRouteHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRouteHeader _ARGS_((SipRouteHeader **pSipRouteHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_SECURITY
SIP_U32bit sip_serializeSipSecurityClientHeader _ARGS_((SipSecurityClientHeader *pSipSecurityClientHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_SECURITY

SIP_U32bit sip_deserializeSipSecurityClientHeader _ARGS_((SipSecurityClientHeader **pSipSecurityClientHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_SECURITY
SIP_U32bit sip_serializeSipSecurityVerifyHeader _ARGS_((SipSecurityVerifyHeader *pSipSecurityVerifyHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_SECURITY

SIP_U32bit sip_deserializeSipSecurityVerifyHeader _ARGS_((SipSecurityVerifyHeader **pSipSecurityVerifyHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipRespKeyHeader _ARGS_((SipRespKeyHeader *pSipRespKeyHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRespKeyHeader _ARGS_((SipRespKeyHeader **pSipRespKeyHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipSubjectHeader _ARGS_((SipSubjectHeader *pSipSubjectHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipSubjectHeader _ARGS_((SipSubjectHeader **pSipSubjectHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRackHeader _ARGS_((SipRackHeader *pSipRackHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRackHeader _ARGS_((SipRackHeader **pSipRackHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_CCP_VERSION10
SIP_U32bit sip_serializeSipRejectContactHeader _ARGS_((SipRejectContactHeader *pSipRejectContactHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION10

SIP_U32bit sip_deserializeSipRejectContactHeader _ARGS_((SipRejectContactHeader **pSipRejectContactHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CCP_VERSION06
SIP_U32bit sip_serializeSipRejectContactHeader _ARGS_((SipRejectContactHeader *pSipRejectContactHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION06

SIP_U32bit sip_deserializeSipRejectContactHeader _ARGS_((SipRejectContactHeader **pSipRejectContactHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipAlertInfoHeader _ARGS_((SipAlertInfoHeader *pSipAlertInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAlertInfoHeader _ARGS_((SipAlertInfoHeader **pSipAlertInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_CCP
SIP_U32bit sip_serializeSipRequestDispositionHeader _ARGS_((SipRequestDispositionHeader *pSipRequestDispositionHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP

SIP_U32bit sip_deserializeSipRequestDispositionHeader _ARGS_((SipRequestDispositionHeader **pSipRequestDispositionHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipInReplyToHeader _ARGS_((SipInReplyToHeader *pSipInReplyToHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipInReplyToHeader _ARGS_((SipInReplyToHeader **pSipInReplyToHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReferToHeader _ARGS_((SipReferToHeader *pSipReferToHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReferToHeader _ARGS_((SipReferToHeader **pSipReferToHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReferredByHeader _ARGS_((SipReferredByHeader *pSipReferredByHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReferredByHeader _ARGS_((SipReferredByHeader **pSipReferredByHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_PRIVACY
SIP_U32bit sip_serializeSipPAssertIdHeader _ARGS_((SipPAssertIdHeader *pSipPAssertIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_PRIVACY

SIP_U32bit sip_deserializeSipPAssertIdHeader _ARGS_((SipPAssertIdHeader **pSipPAssertIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_PRIVACY
SIP_U32bit sip_serializeSipPPreferredIdHeader _ARGS_((SipPPreferredIdHeader *pSipPPreferredIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_PRIVACY

SIP_U32bit sip_deserializeSipPPreferredIdHeader _ARGS_((SipPPreferredIdHeader **pSipPPreferredIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipAlsoHeader _ARGS_((SipAlsoHeader *pSipAlsoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAlsoHeader _ARGS_((SipAlsoHeader **pSipAlsoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipProxyAuthenticateHeader _ARGS_((SipProxyAuthenticateHeader *pSipProxyAuthenticateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipProxyAuthenticateHeader _ARGS_((SipProxyAuthenticateHeader **pSipProxyAuthenticateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRetryAfterHeader _ARGS_((SipRetryAfterHeader *pSipRetryAfterHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRetryAfterHeader _ARGS_((SipRetryAfterHeader **pSipRetryAfterHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipServerHeader _ARGS_((SipServerHeader *pSipServerHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipServerHeader _ARGS_((SipServerHeader **pSipServerHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipUnsupportedHeader _ARGS_((SipUnsupportedHeader *pSipUnsupportedHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipUnsupportedHeader _ARGS_((SipUnsupportedHeader **pSipUnsupportedHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipWarningHeader _ARGS_((SipWarningHeader *pSipWarningHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipWarningHeader _ARGS_((SipWarningHeader **pSipWarningHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipWwwAuthenticateHeader _ARGS_((SipWwwAuthenticateHeader *pSipWwwAuthenticateHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipWwwAuthenticateHeader _ARGS_((SipWwwAuthenticateHeader **pSipWwwAuthenticateHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRseqHeader _ARGS_((SipRseqHeader *pSipRseqHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRseqHeader _ARGS_((SipRseqHeader **pSipRseqHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipErrorInfoHeader _ARGS_((SipErrorInfoHeader *pSipErrorInfoHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipErrorInfoHeader _ARGS_((SipErrorInfoHeader **pSipErrorInfoHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_SECURITY
SIP_U32bit sip_serializeSipSecurityServerHeader _ARGS_((SipSecurityServerHeader *pSipSecurityServerHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_SECURITY

SIP_U32bit sip_deserializeSipSecurityServerHeader _ARGS_((SipSecurityServerHeader **pSipSecurityServerHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipMsgBody _ARGS_((SipMsgBody *pSipMsgBody,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMsgBody _ARGS_((SipMsgBody **pSipMsgBody,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_MWI
SIP_U32bit sip_serializeMesgSummaryMessage _ARGS_((MesgSummaryMessage *pMesgSummaryMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_MWI

SIP_U32bit sip_deserializeMesgSummaryMessage _ARGS_((MesgSummaryMessage **pMesgSummaryMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_MWI
SIP_U32bit sip_serializeSummaryLine _ARGS_((SummaryLine *pSummaryLine,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_MWI

SIP_U32bit sip_deserializeSummaryLine _ARGS_((SummaryLine **pSummaryLine,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipMessage _ARGS_((SipMessage *pSipMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMessage _ARGS_((SipMessage **pSipMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_TXN_LAYER
SIP_U32bit sip_serializeSipTxnKey _ARGS_((SipTxnKey *pSipTxnKey,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_TXN_LAYER

SIP_U32bit sip_deserializeSipTxnKey _ARGS_((SipTxnKey **pSipTxnKey,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipTimerKey _ARGS_((SipTimerKey *pSipTimerKey,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipTimerKey _ARGS_((SipTimerKey **pSipTimerKey,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipParam _ARGS_((SipParam *pSipParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipParam _ARGS_((SipParam **pSipParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_TEL
SIP_U32bit sip_serializeTelUrl _ARGS_((TelUrl *pTelUrl,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_TEL

SIP_U32bit sip_deserializeTelUrl _ARGS_((TelUrl **pTelUrl,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_TEL
SIP_U32bit sip_serializeTelGlobalNum _ARGS_((TelGlobalNum *pTelGlobalNum,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_TEL

SIP_U32bit sip_deserializeTelGlobalNum _ARGS_((TelGlobalNum **pTelGlobalNum,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_TEL
SIP_U32bit sip_serializeTelLocalNum _ARGS_((TelLocalNum *pTelLocalNum,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_TEL

SIP_U32bit sip_deserializeTelLocalNum _ARGS_((TelLocalNum **pTelLocalNum,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_IMPP
SIP_U32bit sip_serializeImUrl _ARGS_((ImUrl *pImUrl,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_IMPP

SIP_U32bit sip_deserializeImUrl _ARGS_((ImUrl **pImUrl,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_PRES
SIP_U32bit sip_serializePresUrl _ARGS_((PresUrl *pPresUrl,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_PRES

SIP_U32bit sip_deserializePresUrl _ARGS_((PresUrl **pPresUrl,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipUrl _ARGS_((SipUrl *pSipUrl,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipUrl _ARGS_((SipUrl **pSipUrl,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipAddrSpec _ARGS_((SipAddrSpec *pSipAddrSpec,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipAddrSpec _ARGS_((SipAddrSpec **pSipAddrSpec,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipGenericChallenge _ARGS_((SipGenericChallenge *pSipGenericChallenge,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipGenericChallenge _ARGS_((SipGenericChallenge **pSipGenericChallenge,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipGenericCredential _ARGS_((SipGenericCredential *pSipGenericCredential,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipGenericCredential _ARGS_((SipGenericCredential **pSipGenericCredential,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipDateFormat _ARGS_((SipDateFormat *pSipDateFormat,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipDateFormat _ARGS_((SipDateFormat **pSipDateFormat,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipTimeFormat _ARGS_((SipTimeFormat *pSipTimeFormat,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipTimeFormat _ARGS_((SipTimeFormat **pSipTimeFormat,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipDateStruct _ARGS_((SipDateStruct *pSipDateStruct,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipDateStruct _ARGS_((SipDateStruct **pSipDateStruct,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipExpiresStruct _ARGS_((SipExpiresStruct *pSipExpiresStruct,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipExpiresStruct _ARGS_((SipExpiresStruct **pSipExpiresStruct,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipContactParam _ARGS_((SipContactParam *pSipContactParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipContactParam _ARGS_((SipContactParam **pSipContactParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_CCP_VERSION06
SIP_U32bit sip_serializeSipAcceptContactParam _ARGS_((SipAcceptContactParam *pSipAcceptContactParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION06

SIP_U32bit sip_deserializeSipAcceptContactParam _ARGS_((SipAcceptContactParam **pSipAcceptContactParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CCP_VERSION10
SIP_U32bit sip_serializeSipAcceptContactParam _ARGS_((SipAcceptContactParam *pSipAcceptContactParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION10

SIP_U32bit sip_deserializeSipAcceptContactParam _ARGS_((SipAcceptContactParam **pSipAcceptContactParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CCP_VERSION06
SIP_U32bit sip_serializeSipRejectContactParam _ARGS_((SipRejectContactParam *pSipRejectContactParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION06

SIP_U32bit sip_deserializeSipRejectContactParam _ARGS_((SipRejectContactParam **pSipRejectContactParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CCP_VERSION10
SIP_U32bit sip_serializeSipRejectContactParam _ARGS_((SipRejectContactParam *pSipRejectContactParam,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CCP_VERSION10

SIP_U32bit sip_deserializeSipRejectContactParam _ARGS_((SipRejectContactParam **pSipRejectContactParam,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeIsupMessage _ARGS_((IsupMessage *pIsupMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeIsupMessage _ARGS_((IsupMessage **pIsupMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeQsigMessage _ARGS_((QsigMessage *pQsigMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeQsigMessage _ARGS_((QsigMessage **pQsigMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipUnknownMessage _ARGS_((SipUnknownMessage *pSipUnknownMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipUnknownMessage _ARGS_((SipUnknownMessage **pSipUnknownMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipHeaderOrderInfo _ARGS_((SipHeaderOrderInfo *pSipHeaderOrderInfo,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipHeaderOrderInfo _ARGS_((SipHeaderOrderInfo **pSipHeaderOrderInfo,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReqHeader _ARGS_((SipReqHeader *pSipReqHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReqHeader _ARGS_((SipReqHeader **pSipReqHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipReqMessage _ARGS_((SipReqMessage *pSipReqMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReqMessage _ARGS_((SipReqMessage **pSipReqMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRespHeader _ARGS_((SipRespHeader *pSipRespHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRespHeader _ARGS_((SipRespHeader **pSipRespHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipRespMessage _ARGS_((SipRespMessage *pSipRespMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipRespMessage _ARGS_((SipRespMessage **pSipRespMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipGeneralHeader _ARGS_((SipGeneralHeader *pSipGeneralHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipGeneralHeader _ARGS_((SipGeneralHeader **pSipGeneralHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipMimeHeader _ARGS_((SipMimeHeader *pSipMimeHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMimeHeader _ARGS_((SipMimeHeader **pSipMimeHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpOrigin _ARGS_((SdpOrigin *pSdpOrigin,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpOrigin _ARGS_((SdpOrigin **pSdpOrigin,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpConnection _ARGS_((SdpConnection *pSdpConnection,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpConnection _ARGS_((SdpConnection **pSdpConnection,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpTime _ARGS_((SdpTime *pSdpTime,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpTime _ARGS_((SdpTime **pSdpTime,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpAttr _ARGS_((SdpAttr *pSdpAttr,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpAttr _ARGS_((SdpAttr **pSdpAttr,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpMedia _ARGS_((SdpMedia *pSdpMedia,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpMedia _ARGS_((SdpMedia **pSdpMedia,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSdpMessage _ARGS_((SdpMessage *pSdpMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSdpMessage _ARGS_((SdpMessage **pSdpMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeMimeMessage _ARGS_((MimeMessage *pMimeMessage,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeMimeMessage _ARGS_((MimeMessage **pMimeMessage,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

SIP_U32bit sip_serializeSipMinExpiresHeader _ARGS_((SipMinExpiresHeader *pSipMinExpiresHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipMinExpiresHeader _ARGS_((SipMinExpiresHeader **pSipMinExpiresHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_PRIVACY
SIP_U32bit sip_serializeSipPrivacyHeader _ARGS_((SipPrivacyHeader *pSipPrivacyHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_PRIVACY

SIP_U32bit sip_deserializeSipPrivacyHeader _ARGS_((SipPrivacyHeader **pSipPrivacyHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipServiceRouteHeader _ARGS_((SipServiceRouteHeader *pSipServiceRouteHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipServiceRouteHeader _ARGS_((SipServiceRouteHeader **pSipServiceRouteHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPathHeader _ARGS_((SipPathHeader *pSipPathHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPathHeader _ARGS_((SipPathHeader **pSipPathHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


SIP_U32bit sip_serializeSipReasonHeader _ARGS_((SipReasonHeader *pSipReasonHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));

SIP_U32bit sip_deserializeSipReasonHeader _ARGS_((SipReasonHeader **pSipReasonHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPAssociatedUriHeader _ARGS_((SipPAssociatedUriHeader *pSipPAssociatedUriHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPAssociatedUriHeader _ARGS_((SipPAssociatedUriHeader **pSipPAssociatedUriHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPCalledPartyIdHeader _ARGS_((SipPCalledPartyIdHeader *pSipPCalledPartyIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPCalledPartyIdHeader _ARGS_((SipPCalledPartyIdHeader **pSipPCalledPartyIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPVisitedNetworkIdHeader _ARGS_((SipPVisitedNetworkIdHeader *pSipPVisitedNetworkIdHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPVisitedNetworkIdHeader _ARGS_((SipPVisitedNetworkIdHeader **pSipPVisitedNetworkIdHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPcfAddrHeader _ARGS_((SipPcfAddrHeader *pSipPcfAddrHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPcfAddrHeader _ARGS_((SipPcfAddrHeader **pSipPcfAddrHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CONF
SIP_U32bit sip_serializeSipJoinHeader _ARGS_((SipJoinHeader *pSipJoinHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CONF

SIP_U32bit sip_deserializeSipJoinHeader _ARGS_((SipJoinHeader **pSipJoinHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CONGEST
SIP_U32bit sip_serializeSipRsrcPriorityHeader _ARGS_((SipRsrcPriorityHeader *pSipRsrcPriorityHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CONGEST

SIP_U32bit sip_deserializeSipRsrcPriorityHeader _ARGS_((SipRsrcPriorityHeader **pSipRsrcPriorityHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_CONGEST
SIP_U32bit sip_serializeSipAcceptRsrcPriorityHeader _ARGS_((SipAcceptRsrcPriorityHeader *pSipAcceptRsrcPriorityHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_CONGEST

SIP_U32bit sip_deserializeSipAcceptRsrcPriorityHeader _ARGS_((SipAcceptRsrcPriorityHeader **pSipAcceptRsrcPriorityHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef SIP_3GPP
SIP_U32bit sip_serializeSipPcVectorHeader _ARGS_((SipPcVectorHeader *pSipPcVectorHeader,  SIP_S8bit *buffer,  SIP_U32bit position,  SipError	*pError));
#endif


#ifdef SIP_3GPP

SIP_U32bit sip_deserializeSipPcVectorHeader _ARGS_((SipPcVectorHeader **pSipPcVectorHeader,SIP_S8bit *buffer,SIP_U32bit position, SipError *pError ));

#endif


#ifdef __cplusplus
}
#endif

#endif /* _SIP_SERIALIZE_H */
