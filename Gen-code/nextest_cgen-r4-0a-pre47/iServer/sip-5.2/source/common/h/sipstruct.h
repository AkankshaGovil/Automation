/*******************************************************************
 ** FUNCTION:
 **	 	This file has all the SIP Related Structures
 *******************************************************************
 **
 ** FILENAME:
 ** sipglb.h
 **
 ** DESCRIPTION:
 **	All primary structures and wrappers for pBasic datatypes are 
 ** stored here. It is expected that all programs use datatypes 
 ** defined here
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 4/12/99	        Arjun RC						Initial
 **
 **	Copyright 1999, Hughes Software Systems, Ltd.
 *******************************************************************/

#ifndef __SIPSTRUCT_H__
#define __SIPSTRUCT_H__

#include "sipcommon.h"
#include "siplist.h"
#include "portlayer.h"
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif


#define SIP_TCP (0x1)
#define SIP_UDP (0x2)
#define SIP_NORETRANS (0x4)

#define SIP_SCTP (0x8)
#define SIP_TLS (0x10)

/* constants for forming/sending a message */

#define SIP_OPT_FULLFORM 		(0x1)
#define SIP_OPT_SHORTFORM		(0x2)
#define SIP_OPT_COMMASEPARATED	(0x4)
#define SIP_OPT_SINGLE			(0x8)
#define SIP_OPT_REORDERHOP		(0x10)
#define SIP_OPT_AUTHCANONICAL	(0x20)
#define SIP_OPT_CLEN			(0x40)
#define SIP_OPT_PERMSGRETRANS	(0x80)
#define SIP_OPT_NOSTARTLINE   	(0x100)
#define SIP_OPT_RETRANSCALLBACK	(0x200)
#define SIP_OPT_PERMSGRETRANSCOUNT (0x400)
#define SIP_OPT_DIRECTBUFFER 	(0x800)
#define SIP_OPT_MAXBUFSIZE 	(0x1000)


/* constants for decoding a message */
#define SIP_OPT_NOPARSEBODY		(0x1)
#define SIP_OPT_NOPARSESDP		(0x2)
#define SIP_OPT_DBLNULLBUFFER	(0x4)
#define SIP_OPT_SELECTIVE		(0x8)
#define SIP_OPT_NOPARSEAPPSIP	(0x10)
#define SIP_OPT_NOTIMER			(0x20)
#define SIP_OPT_PARTIAL			(0x40)
#define SIP_OPT_BADMESSAGE		(0x80)

#ifdef SIP_LOCKEDREFCOUNT
typedef struct
{
	SIP_U32bit			ref;
	synch_id_t			lock;
} SIP_RefCount;
#else
typedef		SIP_U32bit			SIP_RefCount;
#endif

#ifdef SIP_TEL
#include "telstruct.h"
#endif

#ifdef SIP_IMPP
#include "imppstruct.h"
#endif

#ifdef SIP_MIB
typedef struct
{
	SIP_U32bit recv;
	SIP_U32bit sendInitial;
	SIP_U32bit sendRetry;
	SipBool	dGiveCallback;
}SipStatParam;
#endif

typedef enum
{
	SipMessageRequest=0,
	SipMessageResponse,
	SipMessageAny				/* Required for timer pKey matching */
} en_SipMessageType;


/* This enumeration is used to define various timers defined in the bis-05 for the SIP state machine.*/
typedef enum
{
	SipTimerA_B,	/* For Starting TimerA along With TimerB */
	SipTimerB,
	
	/*
		There are two timer Cs. The reason for having two timers is
		slightly involved. Timer C is started as soon as a PROXY sends
		an INVITE. This timer is reset(restarted) whenever a provisional
		response is received(other than 100 Trying). However when it actually
		fires an indication needs to be given to the application so that
		the application can embark on cancelling the INVITE txn. However the
		txn shud not be removed. Instead another timer Cr is started during
		which the INVITE txn is allowed to complete by way of receipt of a
		487.During the Timer Cr is running any provisional response that is
		received DOES NOT restart the Timer C , these are returned as stray
		The value for both these  timers will be independently configurable. 
		However receipt of timeout callbacks will NOT be independently 
		configurable. In case an application is interested in receiving 
		timeout for TimerC , the appln will *also* receive timeouts for
		Timer Cr as well.
	*/
	SipTimerC,
	SipTimerCr,
	
	SipTimerD,
	SipTimerE_F,	/* For Starting TimerE along With TimerF */
	SipTimerF,
	SipTimerG_H,	/* For Starting TimerG along With TimerH */
	SipTimerH,
	SipTimerI,
	SipTimerJ,
	SipTimerK,
	
	SipTimerForCancelledState,  /*This timer is the timer that is started when
									a txn is cancelled*/
	
	/*
		When a proxy UAS forwards a 2xx the txn needs to be terminated. However 
		this is not done immediately since receipt of retransmissions of INVITEs
		is an issue. We do not want each of these retransmissions to end up
		creating a new txn. For this reason the txn is not immediately removed 
		but retained for a fixed period which shud be sufficient enough to
		buffer up retransmissions of INVITE. For all purposes this value
		shud be same as that of the remote end's TimerB. As far as the stack
		is concerned no separate value will be defined for this timer, its value
		will be the same as the application's configured value of Timer B. 
		Receipt of timeouts of this timer will *not* be configurable. The
		stack will not throw callbacks whenever this timer expires.
	*/
	SipTimerForProxyUAS2xxFwded,

	/*
	 * This timer only serves an an indicator for the last enum.
	 * It is not used anywhere in the stack
	 */
	SipTimerLast
}en_SipTimerType;

#ifdef SIP_MIB
	typedef enum
	{
		SipCallbkForResponseSent=0,
		SipCallbkForResponseRecvd
	} en_SipMibCallbackType;
#endif

typedef enum
{
	SipHdrTypeAccept	= 0, 	/* Tokensltoken */
	SipHdrTypeAcceptEncoding,	/* Tokens */
	SipHdrTypeAcceptLanguage,	/* Tokensltoken */
	SipHdrTypeCallId,			/* Contact */
	SipHdrTypeCseq,				/* Tokens */
	SipHdrTypeDate,				/* Date */
	SipHdrTypeEncryption,		/* Key */
	SipHdrTypeExpiresDate,		/* Date */
	SipHdrTypeExpiresSec,		/* Date */
	SipHdrTypeExpiresAny,		/* Date */
	SipHdrTypeFrom,				/* Fromto */
	SipHdrTypeRecordRoute,		/* Fromto */
	SipHdrTypeTimestamp,		/* Date */
	SipHdrTypeTo,				/* Fromto */
	SipHdrTypeVia,				/* Via */
	SipHdrTypeContentEncoding,	/* Tokens */
	SipHdrTypeContentLength,	/* Tokens */
	SipHdrTypeContentType,		/* Tokensltoken */
	SipHdrTypeAuthorization,	/* Pgp */
	SipHdrTypeContactNormal,	/* Contact */
	SipHdrTypeContactWildCard,	/* Contact */
	SipHdrTypeContactAny,		/* Contact */
	SipHdrTypeHide,				/* Tokens */
	SipHdrTypeMaxforwards,		/* Tokens */
	SipHdrTypeOrganization,		/* Utf8 */
	SipHdrTypePriority,			/* Tokens */
	SipHdrTypeProxyauthorization, /* Pgp */
	SipHdrTypeProxyRequire,		/* Tokens */
	SipHdrTypeRoute,			/* Fromto */
	SipHdrTypeRequire,			/* Tokens */
	SipHdrTypeResponseKey,		/* Key */
	SipHdrTypeSubject,			/* Utf8 */
	SipHdrTypeUserAgent,		/* Tokencomment */
	SipHdrTypeAllow,			/* Tokens */
	SipHdrTypeProxyAuthenticate,/* Pgp */
	SipHdrTypeRetryAfterDate,	/* Date */
	SipHdrTypeRetryAfterSec,	/* Date */
	SipHdrTypeRetryAfterAny,	/* Date */
	SipHdrTypeServer,			/* Tokencomment */
	SipHdrTypeUnsupported,		/* Tokens */
	SipHdrTypeWarning,			/* Tokensltoken */
	SipHdrTypeWwwAuthenticate,	/* Pgp */
	SipHdrTypeAuthenticationInfo,	/* Pgp */
	SipHdrTypeUnknown,			/* NULL */
	SipHdrTypeMimeVersion,  	/* Tokensltoken */
#ifdef SIP_CCP
	SipHdrTypeAcceptContact, 	/* AcceptContact */
	SipHdrTypeRejectContact, 	/* RejectContact */
	SipHdrTypeRequestDisposition, /* Tokens */
#endif
	SipHdrTypeRAck, 			/* RprTokens */
	SipHdrTypeRSeq,	 			/* RprTokens */
	SipHdrTypeSupported,		/* Tokens */
	SipHdrTypeAlertInfo,		/* Tokensltoken */
	SipHdrTypeInReplyTo,		/* Tokensltoken */
	SipHdrTypeCallInfo,			/* Tokensltoken */
	SipHdrTypeContentLanguage,	/* Key */
	SipHdrTypeErrorInfo,		/* Tokensltoken */
	SipHdrTypeContentDisposition, /* Tokensltoken */
	SipHdrTypeReferTo,	    	/* Fromto */
	SipHdrTypeAlso,				/* Contact */
	SipHdrTypeReferredBy, 		/* Fromto */
	SipHdrTypeReplaces,			/* Contact */
	SipHdrTypeReplyTo,			/* Contact */
#ifdef SIP_IMPP
	SipHdrTypeEvent,			/* Tokensltoken */
	SipHdrTypeAllowEvents,			/* Tokens */
	SipHdrTypeSubscriptionState,	/* Datetime */
#endif
#ifdef SIP_DCS
	SipHdrTypeDcsRemotePartyId,   	/* Dcs */
	SipHdrTypeDcsRpidPrivacy,   	/* Dcs */
	SipHdrTypeDcsTracePartyId,   	/* Dcs */
	SipHdrTypeDcsAnonymity,   	/* Dcs */
	SipHdrTypeDcsMediaAuthorization,   	/* Dcs */
	SipHdrTypeDcsGate,   	/* Dcs */
	SipHdrTypeDcsRedirect,   	/* Dcs */
	SipHdrTypeDcsState,   	/* Dcs */
	SipHdrTypeDcsLaes,   	/* Dcs */
	SipHdrTypeSession,   	/* Dcs */
	SipHdrTypeDcsOsps,
	SipHdrTypeDcsBillingId,   	/* Dcs */
	SipHdrTypeDcsBillingInfo,
#endif   	/* Dcs */
#ifdef SIP_SESSIONTIMER
	SipHdrTypeMinSE,				/*Date */
	SipHdrTypeSessionExpires,		/* Date */
#endif
	SipHdrTypeMinExpires,		/* Datetime */
#ifdef SIP_PRIVACY
	SipHdrTypePAssertId , /* P-Asserted-Identity */
	SipHdrTypePPreferredId , /* P-Preferred-Identity */
	SipHdrTypePrivacy,	/* tokens */
# endif
#ifdef SIP_3GPP
	SipHdrTypePath,		/*	Fromto	*/
	SipHdrTypePanInfo,   /* Fromto */ 
	SipHdrTypePcVector,  /* Fromto */
#endif
	SipHdrTypeReason,	/* Tokensltoken */
#ifdef SIP_CONGEST
    SipHdrTypeRsrcPriority,	/* Tokens*/
    SipHdrTypeAcceptRsrcPriority,	/* Tokens*/
#endif
#ifdef SIP_CONF
	SipHdrTypeJoin,			/* Contact */
#endif
#ifdef SIP_3GPP
    SipHdrTypePAssociatedUri, /*3gpp */
    SipHdrTypePCalledPartyId, /*3gpp */
    SipHdrTypePVisitedNetworkId, /*3gpp */
    SipHdrTypePcfAddr, /*3gpp */
#endif
#ifdef SIP_SECURITY
    SipHdrTypeSecurityClient,	/*Via */
    SipHdrTypeSecurityServer,   /*Via */
    SipHdrTypeSecurityVerify,   /*Via */
#endif 
#ifdef SIP_3GPP
    SipHdrTypeServiceRoute, /*3GPP */
#endif
   SipHdrTypeAny				/* NULL */
}en_HeaderType;

#define HEADERTYPENUM ((int)SipHdrTypeAny + 1)

typedef enum
{
	SipModeJoin=0,
	SipModeNew,
	SipModeNone
}en_AdditionMode;

typedef enum
{
	SipFormFull = 0,
	SipFormShort
}en_HeaderForm;

typedef enum
{
	SipCParamQvalue=0,
	SipCParamExpires,
	SipCParamExtension,
	SipCParamFeatureParam,
	SipCParamAny
}en_ContactParamsType;


typedef enum
{	SipTranspTcp=0,
	SipTranspUdp
}en_TransportParam;

typedef enum
{
	SipUserParamPhone=0,
	SipUserParamIp
} en_UserParam;

typedef enum
{
	SipMethodInvite=0,
	SipMethodAck,
	SipMethodCancel,
	SipMethodBye,
	SipMethodOptions,
	SipMethodRegister,
	SipMethodRefer,
	SipMethodPublish,
	SipMethodUnknown
}en_Method;

typedef enum
{	SipDayMon=0,
	SipDayTue,
	SipDayWed,
	SipDayThu,
	SipDayFri,
	SipDaySat,
	SipDaySun,
	SipDayNone
}en_DayOfWeek;

typedef enum
{
	SipCredBasic=0,
	SipCredAuth,
	SipCredAny
}en_CredentialType;

typedef enum
{
	SipMonthJan=0,
	SipMonthFeb,
	SipMonthMar,
	SipMonthApr,
	SipMonthMay,
	SipMonthJun,
	SipMonthJul,
	SipMonthAug,
	SipMonthSep,
	SipMonthOct,
	SipMonthNov,
	SipMonthDec
} en_Month;

typedef enum
{
	SipAddrSipUri=0,
	SipAddrSipSUri,
	SipAddrReqUri,
	SipAddrAny
}en_AddrType;

typedef enum
{
	SipExpDate=0,
	SipExpSeconds,
	SipExpAny
}en_ExpiresType	;

typedef enum
{
	SipHideRoute=0,
	SipHideHop,
	SipHideOther
}en_HideType;


typedef enum
{
	SipPriorityEmergency=0,
	SipPriorityUrgent,
	SipPriorityNormal,
	SipPriorityNonUrgent,
	SipPriorityOther
}en_Priority;


/* Note:
	we suggest the user uses
	sip_setStatusCodeNumInStatusLine(...) and
	sip_getStatusCodeNumFromStatusLine(...)

	which set/get the code number as integers and not enums as below.

	For all practical purposes, please ignore the list below and do
	not use it in your application code.
*/

typedef enum
{
	SipStatusTrying 			= 100,
	SipStatusRinging 			= 180,
	SipStatusForwarded 			= 181,
	SipStatusQueued				= 182,
	SipStatusSessionProgress	= 183,
	SipStatusOk					= 200,
	SipStatusMultiChoice		= 300,
	SipStatusMovedPerm			= 301,
	SipStatusMovedTemp			= 302,
	SipStatusSeeOther			= 303,
	SipStatusUseProxy			= 305,
	SipStatusAltService			= 380,
	SipStatusBadRequest			= 400,
	SipStatusUnauthorized		= 401,
	SipStatusPaymentReq			= 402,
	SipStatusForbidden			= 403,
	SipStatusNotFound			= 404,
	SipStatusMethNotAllowed		= 405,
	SipStatusNotAcceptable		= 406,
	SipStatusProxyAuthReq		= 407,
	SipStatusRequestTimeout		= 408,
	SipStatusConflict			= 409,
	SipStatusGone				= 410,
	SipStatusLengthReq			= 411,
	SipStatusReqEntTooLarge		= 413,
	SipStatusReqUriTooLarge		= 414,
	SipStatusUnsuppMediaType	= 415,
	SipStatusBadExtension		= 420,
	SipStatusTempNotAvail		= 480,
	SipStatusCallNotExist		= 481,
	SipStatusLoopDetected		= 482,
	SipStatusTooManyHops		= 483,
	SipStatusAddressIncomp		= 484,
	SipStatusAmbiguous			= 485,
	SipStatusBusyHere			= 486,
	SipStatusRequestTerminated	= 487,
	SipStatusInternalSrvErr		= 500,
	SipStatusNotImplemented		= 501,
	SipStatusBadGateway			= 502,
	SipStatusServiceUnavail		= 503,
	SipStatusGwTimeout			= 504,
	SipStatusSipVerNotSupp		= 505,
	SipStatusBusyEveryWhere		= 600,
	SipStatusDecline			= 603,
	SipStatusDoesNotExist		= 604,
	SipStatusGlobNotAcceptable	= 606,
	SipUnknownStatus			= 2000

}en_StatusCode;

typedef enum
{
	SipContactWildCard = 0,
	SipContactNormal,
	SipContactAny
} en_ContactType;

typedef enum
{
    SipSdpBody = 0,
	SipIsupBody,
	SipMultipartMimeBody,
	SipAppSipBody,
	SipUnknownBody,
#ifdef SIP_MWI
	SipMessageSummaryBody,
#endif
	SipQsigBody,
	SipBodyAny
}en_SipMsgBodyType;

/*
** Structure Definitions Follow
*/

typedef struct
{
	SipBool enable[HEADERTYPENUM];
} SipHdrTypeList;

typedef struct
{
	SIP_S8bit 		*pName;
	SipList			slValue; 			/* of SIP_S8bit * */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipParam;

typedef struct
{
	SIP_S8bit dIpv4[16];
	/* New host field added. User not limited to numeric IPV4 address now. */
	/* Used only for C++ wrappers */
	SIP_S8bit *pHost;
	SIP_U16bit dPort;
	SIP_S32bit dSockFd;
	/* New void pointer field. Used to pass contextual data from
	   sendmessage to sendToNetwork */
	SIP_Pvoid	pData;
} SipTranspAddr;

typedef struct
{
	SipTraceLevel	dLevel;
	SipTraceType 	dType;
} SipTrace;

typedef struct
{
	SIP_U32bit dOption;
} SipOptions;

typedef struct
{
	en_HeaderType	dType;
	SIP_Pvoid		pHeader;
} SipHeader;

typedef struct
{
	/* Header type list - used to pass list of headers with the selective
	   parsing option to sip_decodeMessage */
	SipHdrTypeList	*pList;
	SipTranspAddr	*pTranspAddr;
	SIP_S8bit		*pDirectBuffer;
	SIP_Pvoid 		pData;
	SIP_U32bit		dRemainingLength;
	SIP_U32bit		dNextMessageLength;
	SipOptions      dOptions;
#ifndef SIP_TXN_LAYER
	SIP_U32bit 		dRetransT1;
	SIP_U32bit 		dRetransT2;
	SIP_U16bit 		dMaxRetransCount;
	SIP_U16bit 		dMaxInviteRetransCount;
#endif
} SipEventContext;

/* SDP Structures */

typedef struct
{
	SIP_S8bit*		pUser;
	SIP_S8bit*		pSessionid;
	SIP_S8bit*		pVersion;
	SIP_S8bit*		pNetType;
	SIP_S8bit*		pAddrType;
	SIP_S8bit		*dAddr;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SdpOrigin;

typedef struct
{
	SIP_S8bit*		pInformation;
	SipList			slConnection;		/* of Connection */
	SipList			slBandwidth;
	SIP_S8bit*		pKey;
	SIP_S8bit*		pFormat;
	SIP_U16bit		dPort;
	SIP_U32bit*		pPortNum;
	SIP_S8bit*		pProtocol;
	SipList			slAttr;   			/* of SdpAttr */
	SIP_S8bit*		pMediaValue;
	SipList			slIncorrectLines;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
#ifdef SIP_ATM
	SIP_S8bit*		pVirtualCID;
	SipList			slProtofmt; /* of SipNameValuePair */
#endif
}SdpMedia ;

typedef struct
{
	SIP_S8bit*		pName;
	SIP_S8bit*		pValue;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
}SipNameValuePair;

typedef struct
{
	SipList slNameValue;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
}SipAuthenticationInfoHeader;

typedef struct
{
	SIP_S8bit*		pName;
	SIP_S8bit*		pValue;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SdpAttr;

typedef struct
{
	SIP_S8bit*		pStart;
	SIP_S8bit*		pStop;
	SipList			slRepeat;			/* of char *	*/
	SIP_S8bit*		pZone;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SdpTime;

typedef struct
{
	SIP_S8bit*		pNetType;
	SIP_S8bit*		pAddrType;
	SIP_S8bit*		dAddr;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SdpConnection;

typedef struct
{
	SIP_S8bit*		pVersion;
	SdpOrigin*		pOrigin;
	SIP_S8bit*		pSession;
	SIP_S8bit*		pInformation;
	SIP_S8bit*		pUri;
	SipList			slEmail;			/* of char *		*/
	SipList			slPhone;			/* of char *		*/
	SdpConnection*	slConnection;
	SipList			pBandwidth;			/* of char *		*/
	SipList			slTime;				/* of SdpTime		*/
	SIP_S8bit*		pKey;
	SipList			slAttr;				/* of Attr		*/
	SipList			slMedia;			/* of Media		*/
	SipList			slIncorrectLines;   /* incorrect SDP lines */
	SIP_RefCount	dRefCount;			/* Reference Count for structures in new version */
} SdpMessage;

/*
We have a generic Challenge structure this has PGP + any other
*/


typedef struct
{
	SIP_S8bit*		pScheme;					/* note Realm is a part of authparam */
	/* SipList		slAuthorizationParam;		  of SipAuthParam		*/
	SipList			slParam; /* of SipParam */
	SIP_RefCount	dRefCount;					/* Reference Count for structures in new version */
} SipGenericChallenge;


/* Response Structures */

typedef struct
{
	SipGenericChallenge		*pChallenge;		/* of SipGenericChallenge */
	SIP_RefCount			dRefCount;		/* Reference Count for structures in new version */
} SipWwwAuthenticateHeader;

typedef struct
{
	SipGenericChallenge		*pChallenge;		/* of SipGenericChallenge */
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
} SipProxyAuthenticateHeader;

typedef struct
{
	SIP_U16bit		dCodeNum;
	SIP_S8bit*		pAgent;
	SIP_S8bit*		pText;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipWarningHeader ;

typedef struct
{
	SIP_S8bit		dDay;
	en_Month		dMonth;
	SIP_U16bit		dYear;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipDateFormat;

typedef struct
{
	SIP_S8bit		dHour;
	SIP_S8bit		dMin;
	SIP_S8bit		dSec;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipTimeFormat;

typedef struct
{
	en_DayOfWeek		dDow;
	SipDateFormat		*pDate;
	SipTimeFormat		*pTime;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipDateStruct;

typedef struct
{
	en_DayOfWeek		dDow;
	SipDateFormat		*pDate;
	SipTimeFormat		*pTime;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipDateHeader;


typedef struct
{
	SIP_S8bit*		pMethod; 	/* CHANGED */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipAllowHeader;


typedef struct
{
	en_ExpiresType	dType;
	union
	{
		SipDateStruct	*pDate;
		SIP_U32bit		dSec;
	} u;
	SIP_S8bit *		pComment;
	/* SIP_U32bit*		pDuration;*/
	SipList 		slParams;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipRetryAfterHeader ;

typedef struct
{
	en_CredentialType	dType;
	union
	{
		SIP_S8bit*			pBasic;
		SipGenericChallenge	*pChallenge;
	}u;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipGenericCredential;

typedef struct
{
	SipGenericCredential			*pCredential;	/*  of  SipGenericCredential */
	SIP_RefCount					dRefCount;	/* Reference Count for structures in new version */
} SipAuthorizationHeader;

typedef struct
{
	SIP_U32bit dRespNum;
	SIP_RefCount dRefCount;	/* Reference Count for structures in new version */
}SipRseqHeader;   									/* Retrans */

typedef struct
{
	SIP_S8bit *pOption;
	SIP_RefCount dRefCount;	/* Reference Count for structures in new version */
} SipUnsupportedHeader;

typedef struct
{
	SIP_S8bit *pValue;
	SIP_RefCount dRefCount;	/* Reference Count for structures in new version */
} SipServerHeader;

typedef struct
{
	SIP_U32bit dSec;
	SIP_RefCount dRefCount;
} SipMinExpiresHeader;

typedef struct
{
	SipList					slProxyAuthenticateHdr;	/*  of SipGenericChallenge	*/
	SipServerHeader*		pServerHdr;
	SipList					slUnsupportedHdr;		/* of SipUnsupportedHeader	*/
	SipList					slWarningHeader;		/* of WarningHeader	*/
	SipList					slWwwAuthenticateHdr;	/* of WwwAuthenticateHdr;	*/
	SipList					slAuthorizationHdr;
	SipList					slAuthenticationInfoHdr;/* of SipAuthenticationInfoHdr */
	SipRseqHeader*			pRSeqHdr;  				/* Retrans */
	SipList					slErrorInfoHdr; 	/* Error-Info Header */
	SipMinExpiresHeader*	pMinExpiresHdr;		/* MinExpires header */
#ifdef SIP_DCS
	SipList					slSessionHdr;
#endif
#ifdef SIP_CONGEST
    SipList slAcceptRsrcPriorityHdr;
#endif
#ifdef SIP_3GPP
	SipList  slPAssociatedUriHdr; /*P-Associated-URI header */
#endif
#ifdef SIP_SECURITY
SipList  slSecurityServerHdr; /*Security-Server header */
#endif
    SIP_RefCount			dRefCount;				/* Reference Count for structures in new version */
} SipRespHeader ;

typedef struct
{
	SIP_S8bit*	pKeyScheme;
    SipList        slParam;     /*of SipParam *  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipRespKeyHeader;

typedef struct _UserAgentHeader
{
	SIP_S8bit* 	pValue;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipUserAgentHeader;

typedef struct
{
	SIP_S8bit*	pSubject;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipSubjectHeader;

typedef struct
{
	SIP_S8bit* 	pToken;		/*  of char * 	*/
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipProxyRequireHeader ;

typedef struct
{
	SipGenericCredential *pCredentials;
	SIP_RefCount		 dRefCount;	/* Reference Count for structures in new version */
} SipProxyAuthorizationHeader ;

typedef struct
{
	SIP_S8bit*		pPriority;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipPriorityHeader;

typedef struct
{
	SIP_S8bit*	pOrganization;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipOrganizationHeader;

typedef struct
{
	SIP_S8bit*	pMediaType;
	SipList 	slParams;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipContentTypeHeader;

typedef struct
{
	SIP_S8bit*	pEncoding;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipContentEncodingHeader;

typedef struct
{
	SIP_S8bit*		pType;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipHideHeader ;

typedef struct
{
	SIP_U32bit		dHops;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipMaxForwardsHeader;

/* Replaced by SipParam - PARAMCHANGE
typedef struct
{
	en_ViaParamType dType;
	union
	{
		SIP_S8bit*	pHidden;
		SIP_U8bit	dTtl;
		SIP_S8bit*	pMaddr;
		SIP_S8bit*	pViaReceived;
		SIP_S8bit*	pViaBranch;
		SIP_S8bit*	pExtParam;		 of char * "pName=vale"
	}u;
	SIP_RefCount	dRefCount;	 Reference Count for structures in new version
} SipViaParam;
*/
typedef struct
{
      SIP_S8bit *pLangTag;
      SIP_RefCount    dRefCount;      /* Reference Count for structures in new version */

}SipContentLanguageHeader;



typedef struct
{
	SIP_U32bit	dLength;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipContentLengthHeader ;

typedef struct
{
	SIP_S8bit*	 	pSentProtocol;
	SIP_S8bit*		pSentBy;
	/* SipList			slViaParam;	 of ViaParams		*/
	SipList			slParam;	/* of SipParam	*/
	SIP_S8bit*		pComment;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipViaHeader;

/* Now replaced by SipParam - PARAMCHANGE
typedef struct
{
	en_UrlParamType	dType;
	union
	{
		en_TransportParam	dTranspParam;
		en_UserParam		dUserParam;
		SIP_U8bit			dTtl;
		SIP_S8bit*			pMethod;
		SIP_S8bit*			pMaddr;
		SIP_S8bit*			pOtherParam;

	} u;
} SipUrlParam;
*/

typedef struct
{
	SIP_S8bit*		pUser;
	SIP_S8bit*		pPassword;
	SIP_S8bit*		pHost;
	SIP_U16bit*		dPort;
	SipList			slParam; 		/*  of SipParam	PARAMCHANGE*/
	SIP_S8bit*		pHeader;
	SIP_RefCount    dRefCount;  		/* Reference Count for structures in new version */
} SipUrl;

typedef struct
{
	en_AddrType	dType;
	union
	{
		SIP_S8bit*	pUri;
		SipUrl*		pSipUrl;
	} u;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipAddrSpec;

typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList			slTag;		/* of char *		*/
	SipList			slParam;	/*  of char * pName=pValue  Changed to SipParam PARAMCHANGE */
	/* SipList			pExtParam;	  of char * pName=pValue */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipToHeader;

typedef struct
{
	SIP_S8bit*		pTime;
	SIP_S8bit*		delay;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipTimeStampHeader;

typedef struct
{
	SIP_S8bit*			pDispName;
	SipAddrSpec*		pAddrSpec;
	SipList 		slParams; 	/* siplist of siparam */
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipRouteHeader;


typedef struct
{
	SIP_S8bit*		pCallid;
	SIP_S8bit*		pFromTag;
	SIP_S8bit*		pToTag;
	SipList			slParams;	/* SipList of SipParam */
	SIP_RefCount	dRefCount;
}SipReplacesHeader;


typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList			slParams; 	/* SipList of SipParam */
	SIP_RefCount	dRefCount;
}SipReplyToHeader;

typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList			slParams;	/* siplist of Sipparam */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
}SipRecordRouteHeader;

#ifdef SIP_3GPP
typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList			slParams;	/* siplist of Sipparam */
	SIP_RefCount	dRefCount;	/* Reference Count for structures */
}SipPathHeader;

typedef SipPathHeader SipServiceRouteHeader ;

typedef struct
{
	SIP_S8bit*     pAccessType;
	SipList        slParams;
	SIP_RefCount   dRefCount;
}SipPanInfoHeader;	

typedef struct
{
	SIP_S8bit*     pAccessType;
	SIP_S8bit*     pAccessValue;
	SipList        slParams;
	SIP_RefCount   dRefCount;
}SipPcVectorHeader;	

#endif


typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*    pAddrSpec;
	SipList	slParams;	/* List of SipParam structures */
	SIP_RefCount    dRefCount;/* Reference Count for structures in new version */
} SipReferToHeader ;

typedef struct
{
	SIP_S8bit*	pDispName;
	SipAddrSpec*    pAddrSpecReferrer;
	SipAddrSpec*    pAddrSpecReferenced;
	SipList 		slParams; 	/* siplist of Sipparam */
	SIP_S8bit*		pMsgId;
	SIP_RefCount    dRefCount;/* Reference Count for structures in new version */
} SipReferredByHeader ;

typedef struct
{
	SIP_S8bit*		pToken;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipRequireHeader;

typedef struct
{
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList			slTag;		/*  of char*		*/
	SipList			slParam;	/*  of char * pName=pValue  Changed to SipParam PARAMCHANGE */
	/* SipList			pExtParam;	  of char * pName=pValue */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipFromHeader;

typedef struct
{
	en_ExpiresType	dType;
	union
	{
		SipDateStruct	*pDate;
		SIP_U32bit		dSec;
	} u;
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
} SipExpiresStruct;

typedef struct
{
	en_ExpiresType	dType;
	union
	{
		SipDateStruct	*pDate;
		SIP_U32bit		dSec;
	} u;
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
} SipExpiresHeader;




typedef struct
{
	SIP_S8bit*		pScheme;
	SipList			slParam;		/* of SipParam *	*/
	SIP_RefCount	dRefCount;		/* Reference Count for structures in new version */
} SipEncryptionHeader;

typedef struct
{
	en_ContactParamsType	dType;
	union
	{
		SIP_S8bit*			pQValue;
		SipExpiresStruct*	pExpire;
		SIP_S8bit*			pExtensionAttr;
		SipParam* pParam ;
	} u;
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
} SipContactParam;


typedef struct
{
	en_ContactType		dType;
	SIP_S8bit*			pDispName;
	SipAddrSpec*		pAddrSpec;
	SipList				slContactParam; 		/*  of ContactParams;		*/
	SIP_RefCount		dRefCount;				/* Reference Count for structures in new version */
} SipContactHeader;

typedef struct
{
	SIP_U32bit		dSeqNum;
	SIP_S8bit*		pMethod;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipCseqHeader;

typedef struct
{
	SIP_S8bit*		pValue;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipCallIdHeader;

typedef struct
{
	SIP_S8bit *		pMediaRange;
	SIP_S8bit*		pAcceptRange;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipAcceptHeader;

typedef struct
{
	SIP_S8bit* 		pCoding;
	SIP_S8bit*		pQValue;
	SipList			slParam;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipAcceptEncodingHeader;


typedef struct
{
	SIP_S8bit*		pLangRange;
	SIP_S8bit*		pQValue;
	SipList			slParam;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipAcceptLangHeader;

typedef struct
{
	SIP_U32bit 		dRespNum;
	SIP_U32bit 		dCseqNum;
	SIP_S8bit* 		pMethod;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipRackHeader;         /* Retrans */

typedef struct
{
	SIP_S8bit* pUri;
	SipList    slParam;		/* Of SipParams*  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
}SipAlertInfoHeader;

typedef struct
{
	SIP_S8bit* pUri;
	SipList    slParam;             /* Of SipParams*  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */

}SipCallInfoHeader;

typedef struct
{
	SIP_S8bit* pUri;
	SipList    slParam;             /* Of SipParams*  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */

}SipErrorInfoHeader;

typedef struct
{
	SIP_S8bit* pDispType;
	SipList    slParam;             /* Of SipParams*  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */

}SipContentDispositionHeader;

typedef struct
{
	SIP_S8bit* pDispType;
	SipList    slParam;             /* Of SipParams*  */
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */

}SipReasonHeader;

typedef struct
{
	SIP_S8bit*    pCallId;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
}SipInReplyToHeader;

typedef struct
{
	SIP_S8bit*	pDispName;
	SipAddrSpec*	pAddrSpec;
	SIP_RefCount	dRefCount; /* Reference Count for structures*/
}SipAlsoHeader;


#ifdef SIP_PRIVACY

typedef struct
{
	SIP_S8bit    * pDispName ; /* of Name-Addr */
	SipAddrSpec  * pAddrSpec ; /* of Addr-Spec */
	SIP_RefCount dRefCount ;
} SipPAssertIdHeader;

typedef struct
{
	SIP_S8bit    * pDispName ; /* of Name-Addr */
	SipAddrSpec  * pAddrSpec ; /* of Addr-Spec */
	SIP_RefCount dRefCount ;
} SipPPreferredIdHeader ;

typedef struct
{
	SipList		slPrivacyValue;	/* Privacy value */
 	SIP_RefCount	dRefCount;	/* Reference count for the structure */
}SipPrivacyHeader;

#endif

#ifdef SIP_DCS

typedef struct
{
	SIP_S8bit 	*pDispName;
	SipAddrSpec 	*pAddrSpec;
	SipList 	slParams;
/*	SipList         slRpiAuths; */
	SIP_RefCount	dRefCount;
} SipDcsRemotePartyIdHeader;

typedef struct
{
	SipList			slParams;
	SIP_RefCount	dRefCount;
} SipDcsRpidPrivacyHeader;

typedef struct
{
	SipAddrSpec 	*pAddrSpec;
	SIP_RefCount	dRefCount;
} SipDcsTracePartyIdHeader;

typedef struct
{
	SIP_S8bit 	*pTag;
	SIP_RefCount	dRefCount;
} SipDcsAnonymityHeader;

typedef struct
{
	SIP_S8bit 	*pAuth;
	SIP_RefCount	dRefCount;
} SipDcsMediaAuthorizationHeader;

typedef struct
{
	SIP_S8bit 	*pHost;
	SIP_U16bit 	*pPort;
	SIP_S8bit 	*pId;
	SIP_S8bit 	*pKey;
	SIP_S8bit 	*pCipherSuite;
	SIP_S8bit	*pStrength;
	SIP_RefCount	dRefCount;
} SipDcsGateHeader;

typedef struct
{
	SIP_S8bit 	*pHost;
	SipList 	slParams;
	SIP_RefCount	dRefCount;
} SipDcsStateHeader;

typedef struct
{
	SIP_S8bit 	*pTag;
	SIP_RefCount	dRefCount;
} SipDcsOspsHeader;

typedef struct
{
	SIP_S8bit 	*pId;
	SIP_S8bit 	*pFEId;
	SIP_RefCount	dRefCount;
} SipDcsBillingIdHeader;

typedef struct
{
	SIP_S8bit*	pChargeNum;
	SIP_S8bit*	pCallingNum;
	SIP_S8bit*	pCalledNum;
	SIP_S8bit*	pRoutingNum;
	SIP_S8bit*	pLocationRoutingNum;
	SIP_RefCount dRefCount;
} SipDcsAcctEntry;

typedef struct
{
	SIP_S8bit*	pHost;
	SIP_U16bit*	pPort;
	SipList		slAcctEntry;		/* of SipDcsAcctEntry */
	SIP_RefCount dRefCount;
} SipDcsBillingInfoHeader;

typedef struct
{
	SIP_S8bit  	*pSignatureHost;
	SIP_U16bit  	*pSignaturePort;
	SIP_S8bit 	*pContentHost;
	SIP_U16bit 	*pContentPort;
	SIP_S8bit 	*pKey;
	SIP_RefCount	dRefCount;
} SipDcsLaesHeader;

typedef struct
{
	SipUrl 		*pCalledId;
	SipUrl 		*pRedirector;
	SIP_U32bit 	dNum;
	SIP_RefCount	dRefCount;
} SipDcsRedirectHeader;

typedef struct
{
	SIP_S8bit 	*pTag;
	SIP_RefCount	dRefCount;
} SipSessionHeader;
#endif


#ifdef SIP_SESSIONTIMER
typedef struct
{
	SIP_U32bit    dSec;
	SipList slNameValue;
	SIP_RefCount  dRefCount; /* Reference Count */
}SipMinSEHeader;

typedef struct
{
	SIP_U32bit	dSec;
	SipList slNameValue;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipSessionExpiresHeader;
#endif

#ifdef SIP_CONGEST
typedef struct
{
    SIP_S8bit* pNamespace;
    SIP_S8bit* pPriority;
    SIP_RefCount dRefCount;
}SipRsrcPriorityHeader;

typedef SipRsrcPriorityHeader SipAcceptRsrcPriorityHeader;

#endif

#ifdef SIP_CONF
typedef struct
{
	SIP_S8bit*		pCallid;
	SIP_S8bit*		pFromTag;
	SIP_S8bit*		pToTag;
	SipList			slParams;	/* SipList of SipParam */
	SIP_RefCount	dRefCount;
}SipJoinHeader;
#endif

#ifdef SIP_3GPP
typedef struct 
{	
	SIP_S8bit*		pDispName;
	SipAddrSpec*	pAddrSpec;
	SipList		    slParams;	
	SIP_RefCount	dRefCount;	
} SipPAssociatedUriHeader;

/* P-Called-Party-Id header */
typedef SipPAssociatedUriHeader SipPCalledPartyIdHeader;

/* P-Visited-Network-Id header */
typedef struct
{	
	SIP_S8bit*		pVNetworkSpec;
	SipList		    slParams;	
	SIP_RefCount	dRefCount;	
} SipPVisitedNetworkIdHeader;

/* P-Charging-Function-Addresses header */
typedef struct
{	
	SipList		    slParams;	
	SIP_RefCount	dRefCount;	
} SipPcfAddrHeader;
#endif

typedef struct
{
	SipList							slAuthorizationHdr;
	SipHideHeader*					pHideHdr;
	SipReplacesHeader*				pReplacesHdr;
	SipMaxForwardsHeader*			pMaxForwardsHdr;
	SipPriorityHeader*				pPriorityHdr;
	SipList							slProxyAuthorizationHdr; /* list of */
												/*SipProxyAuthorizationHeader */
	SipList							slProxyRequireHdr;			/* of SipProxyRequireHeader */
	SipList							slRouteHdr;					/* of RouteHeader		*/
	SipRespKeyHeader*				pRespKeyHdr;
	SipSubjectHeader*				pSubjectHdr;
	SipReferToHeader*				pReferToHdr;				/* present along wiht the REFER method in a Request Message*/
	SipReferredByHeader*			pReferredByHdr;
	SipList							slWwwAuthenticateHdr;	 	/* of slWwwAuthenticateHdr*/
#ifdef SIP_CCP
	SipList 						slAcceptContactHdr; 		/* CCP */
	SipList							slRejectContactHdr;			/* CCP */
	SipList							slRequestDispositionHdr; 	/* CCP */
#endif
	SipRackHeader*					pRackHdr;   				/* Retrans */
    SipList							slAlertInfoHdr;				/* of Alert Info */

	SipList							slInReplyToHdr;                /* of In Reply To */
	SipList							slAlsoHdr;
#ifdef SIP_IMPP
	SipSubscriptionStateHeader*	pSubscriptionStateHdr;
#endif

	/*DCS*/
#ifdef SIP_DCS
	SipDcsTracePartyIdHeader   *pDcsTracePartyIdHdr;
	SipDcsOspsHeader*			pDcsOspsHdr;
	SipDcsRedirectHeader*		pDcsRedirectHdr;
#endif
#ifdef SIP_IMPP
	SipEventHeader*			pEventHdr;
#endif
#ifdef SIP_CONGEST
    SipList slRsrcPriorityHdr;
#endif
#ifdef SIP_CONF    
	SipJoinHeader*				pJoinHdr;
#endif
#ifdef SIP_SECURITY
	SipList  slSecurityClientHdr;
	SipList  slSecurityVerifyHdr;
#endif

#ifdef SIP_3GPP
    SipPCalledPartyIdHeader  *pPCalledPartyIdHdr;
    SipList  slPVisitedNetworkIdHdr;
#endif
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipReqHeader ;

/***************** BCP-T Extension Specific *********************/
typedef struct
{
		SIP_U32bit 	dLength;
        SIP_S8bit	*pBody;
		SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
}IsupMessage;

typedef struct
{
		SIP_U32bit 	dLength;
        SIP_S8bit	*pBody;
		SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
}QsigMessage;

typedef struct
{
    SIP_S8bit		*pVersion;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipMimeVersionHeader;

typedef struct
{
	SipList			slRecmimeBody;  /* of SipMsgBody */
	SIP_RefCount	dRefCount;		/* Reference Count for structures in new version */
} MimeMessage;

typedef struct
{
	SipContentTypeHeader		*pContentType;
	SipContentDispositionHeader *pContentDisposition;
	SIP_S8bit 					*pContentTransEncoding;
	SIP_S8bit					*pContentId;
	SIP_S8bit					*pContentDescription;
	SipList						slAdditionalMimeHeaders;  /* of char * */
	SIP_RefCount				dRefCount;	/* Reference Count for structures in new version */
}SipMimeHeader;

/***************** BCP-T Extension Specific END *********************/

typedef struct
{
	SipList					slAcceptHdr;  			/* of AcceptHeader		*/
	SipList					slAcceptEncoding;		/* of AcceptEncodingHeader	*/
	/*SipAcceptEncodingHeader*	pAcceptEncodingHdr;*/
	SipList					slAcceptLang;			/* of AcceptLangHeader		*/
	SipCallIdHeader*		pCallidHdr;
	SipList					slContactHdr;			/* of ContactHeader		*/
	SipCseqHeader*			pCseqHdr;
	SipDateHeader*			pDateHdr;
	SipEncryptionHeader*	pEncryptionHdr;
	SipExpiresHeader*		pExpiresHdr;
	SipFromHeader*			pFromHeader;
	SipList					slRecordRouteHdr;		/* of RecordRouteHeader		*/
	SipTimeStampHeader*		pTimeStampHdr;
	SipToHeader*			pToHdr;
	SipList					slViaHdr;				/* of ViaHeader			*/
	SipContentLengthHeader*	pContentLengthHdr;
	SipContentTypeHeader*	pContentTypeHdr;
	SipList					slContentEncodingHdr;	/* of ContentEncodingHeader;	*/
	SipList					slUnknownHdr; 			/* of SipUnknownHeader */
	SipMimeVersionHeader*	pMimeVersionHdr;  		/* bcpt ext */
	SipList 				slSupportedHdr; 		/* BIS extension for RPR */
	SipList					slAllowHdr;				/*  of Allow/Sip		*/
	SipList					slCallInfoHdr;          /* of CallInfoHeader */
	SipList					slContentDispositionHdr;/* of ContentDispHeader */
	SipList					slReasonHdr;			/* of Reason */
	SipList					slContentLanguageHdr;   /* of ContentLanguage */
	SipOrganizationHeader*	pOrganizationHdr;
	SipList					slRequireHdr;			/* of RequireHeader		*/
	SipUserAgentHeader*		pUserAgentHdr;
	SipRetryAfterHeader*	pRetryAfterHdr;
	SipReplyToHeader*		pReplyToHdr;
#ifdef SIP_SESSIONTIMER
	SipSessionExpiresHeader* pSessionExpiresHdr;
	SipMinSEHeader*			pMinSEHdr;
#endif
	SipList					slBadHdr;				/* of SipBadHeader */
#ifdef SIP_IMPP
	SipList					slAllowEventsHdr;		/* of Allow-Events header */
#endif
#ifdef SIP_DCS	/*DCS*/
	SipList							slDcsRemotePartyIdHdr;
	SipList							slDcsRpidPrivacyHdr;
	SipList							slDcsAnonymityHdr;
	SipList							slDcsMediaAuthorizationHdr;
	SipDcsGateHeader*				pDcsGateHdr;
	SipList							slDcsStateHdr;
	SipDcsBillingIdHeader*			pDcsBillingIdHdr;
	SipDcsBillingInfoHeader*		pDcsBillingInfoHdr;
	SipDcsLaesHeader*				pDcsLaesHdr;
#endif
	/*DCS*/
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
#ifdef SIP_PRIVACY
		SipList slPAssertIdHdr ;
		SipList slPPreferredIdHdr ;
		SipPrivacyHeader*	pPrivacyHdr;
#endif
#ifdef SIP_3GPP
	SipList	slPathHdr;	/* Of Path Header	*/	
    SipPcfAddrHeader  *pPcfAddrHdr;/* Of P-Charging-Function-Addresses */
	SipPanInfoHeader*        pPanInfoHdr;  /* Of P-Access-Network-Info */
	SipPcVectorHeader*      pPcVectorHdr; /* Of P-Charging-Vector */ 
	SipList	slServiceRouteHdr;	/* Of ServiceRoute Header	*/	
#endif
} SipGeneralHeader;


#ifdef SIP_MWI

typedef enum
{
	SipMsgWaitingNo=0,
	SipMsgWaitingYes
} en_StatusType;

typedef struct
{
	SIP_S8bit*	 pMedia;		/* Media Type */
	SIP_U32bit       newMessages;		/* no of new Messages */
	SIP_U32bit       oldMessages;		/* no of old Messages */
	SIP_U32bit       newUrgentMessages;	/* no of new Urgent Messages */
	SIP_U32bit       oldUrgentMessages;	/* no of old Urgent Messages */
	SIP_RefCount	 dRefCount;
} SummaryLine;


typedef struct
{
	en_StatusType 	dStatus;
	SipList		slSummaryLine;	/* List of SummaryLine structures */
	SipList		slNameValue;	/* List of SipNameValuePair structures */
	SipAddrSpec * pAddrSpec ; /* Message-Account */
	SIP_RefCount	dRefCount;
} MesgSummaryMessage;
#endif
typedef struct
{
	SIP_S8bit * pOption;
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
} SipSupportedHeader;

typedef struct
{
	SIP_S8bit*		pName;
	SIP_S8bit*		pBody;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipUnknownHeader;

typedef struct
{
	SIP_S8bit*		pMethod;
	SipAddrSpec*	pRequestUri;
	SIP_S8bit*		pVersion;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipReqLine;

typedef struct
{
	SipReqLine 			*pRequestLine;
	SipReqHeader		*pRequestHdr;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipReqMessage;


typedef struct
{
	SIP_S8bit*		pVersion;
	en_StatusCode	code;
	SIP_U16bit		dCodeNum;
	/* This has dCodeNum number for above - make sure both are in sync */
	SIP_S8bit*		pReason;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipStatusLine;

typedef struct
{
	SipStatusLine	 	*pStatusLine;
	SipRespHeader		*pResponseHdr;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipRespMessage;

typedef struct
{

	en_HeaderType		dType;
	en_HeaderForm		dTextType;
	SIP_U32bit			dNum;       /* number of dType headers in that line */
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipHeaderOrderInfo;

typedef struct
{
	SIP_U32bit		dLength;
	SIP_S8bit		*pBuffer;
	SIP_RefCount	dRefCount;	/* Reference Count for structures in new version */
} SipUnknownMessage;

typedef struct
{
	en_HeaderType dType;
	SIP_S8bit *pName;
	SIP_S8bit *pBody;
	SIP_RefCount dRefCount;
} SipBadHeader;

typedef struct
{
	en_SipMessageType 	dType;
	SipList				slOrderInfo;   	/* of SipHeaderOrderInfo */
	SipList				slMessageBody; 	  /* of SipMsgBody	*/
	SipGeneralHeader 	*pGeneralHdr;
	union
	{
		SipReqMessage  *pRequest;
		SipRespMessage *pResponse;
	} u;
	SIP_U32bit			dIncorrectHdrsCount;
	SIP_U32bit			dEntityErrorCount;
	SIP_RefCount		dRefCount;	/* Reference Count for structures in new version */
} SipMessage;

typedef struct
{
	en_SipMsgBodyType	dType;
	SipMimeHeader		*pMimeHeader;	/* bcpt ext */
	union
	{
		SdpMessage			*pSdpMessage;
		SipUnknownMessage	*pUnknownMessage;
		IsupMessage			*pIsupMessage; 		/* bcpt ext */
		MimeMessage			*pMimeMessage;		/* bcpt ext */
		SipMessage			*pAppSipMessage;	/* REFER draft extension */
#ifdef SIP_MWI
		MesgSummaryMessage		*pSummaryMessage;    /* message waiting extension */
#endif
		QsigMessage			*pQsigMessage; 		/* bcpt ext */
	}u;
	SIP_RefCount			dRefCount;	/* Reference Count for structures in new version */
}SipMsgBody;

typedef struct
{
	en_SipMessageType		dMatchType;
	SIP_U8bit 				dMatchFlag;
	SIP_S8bit*				dCallid;
	SIP_S8bit*				pMethod;
	SIP_U32bit				dCseq;
	SIP_U32bit 				dRseq; 		/* rpr */
	SipRackHeader*			pRackHdr; 	/* rpr */
	SipFromHeader*			pFromHdr;
	SipToHeader*			pToHdr;
	SIP_U16bit				dCodeNum;
	SIP_S8bit*				pViaBranch;
	/* The event context in this structure is used to pass the context
	   from sip_decodeMessage to fast_stopTimer */
	SipEventContext*		pContext;
	SIP_RefCount			dRefCount;
} SipTimerKey;


#ifdef SIP_TXN_LAYER
/*The prototype for start/stop timer*/
typedef SIP_U32bit (*TimeoutFuncPtr)(void*,...);
#define sip_txn_timeOutFuncPtr TimeoutFuncPtr
#endif

typedef SipBool (*sip_timeoutFuncPtr)(SipTimerKey *key, SIP_Pvoid buf);


typedef struct
{
	SIP_U8bit		dRetransCount;
	SIP_U32bit		dDuration;
	SipBool			dInfoReceived;
	SipTranspAddr	dAddr;
	SIP_S8bit 		dTranspType;
	SIP_S8bit		*pBuffer;
	SIP_U32bit		dBufferLength;
	SipEventContext *pContext;
	SIP_U32bit		dRetransT1;
	SIP_U32bit		dRetransT2;
	SIP_U16bit 		dMaxRetransCount;
	SIP_U16bit 		dMaxInviteRetransCount;
	SipBool			enableRetransCallback;
	SIP_RefCount			dRefCount;
} SipTimerBuffer;

/*typedef struct
{
	SipMessage *pSipMessage;
	SipList	slGC;
	SipError *pError;
#ifdef SIP_TEL
	TelUrl *pTelStr;
#endif
} SipParserParams;*/

typedef struct
{
	SipMessage *pSipMessage;
	SipList	   *pGCList;
	SipError *pError;
} SipHeaderParserParam;


typedef struct
{
	 SipError    *pError;
	 SipList     *pGCList;
	 SdpMessage  *pSdpMessage;
	 SdpMedia    *pMedia;
}SipSdpParserParam;

typedef struct
{
	 SipError    *pError;
	 SipList     *pGCList;
	 SipList     *pHeaderList;
}SipHeaderSplitParserParam;
#ifdef SIP_MWI
typedef struct
{
	SipError     		*pError;
	SipList      		*pGCList;
	MesgSummaryMessage 	*pMesgSummaryMessage;
} SipMesgSummaryParserParam;
#endif

#ifdef SIP_SECURITY
typedef struct
{	
	SIP_S8bit*	pMechanismName;
	SipList		slParams;	
	SIP_RefCount	dRefCount;	
} SipSecurityClientHeader;

typedef SipSecurityClientHeader SipSecurityServerHeader;
typedef SipSecurityClientHeader SipSecurityVerifyHeader;
#endif


typedef struct
{
	 SipError    	*pError;
	 SipList	*pGCList;
	 SipMimeHeader  *pMimeHeader;
}SipMimeParserParam;

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
