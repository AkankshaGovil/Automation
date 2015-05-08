/*******************************************************************************
 **** FUNCTION:
 ****             Has Init Functions For all Structures

 ******************************************************************************
 ****
 **** FILENAME:
 **** sipinit.c
 ****
 **** DESCRIPTION:
 **** This file contains dCodeNum to init all structures
 ****
 **** DATE        NAME                    REFERENCE               REASON
 **** ----        ----                    ---------              --------
 **** 7/12/99   R Preethy       		                    Initial Creation
 ****
 ****     Copyright 1999, Hughes Software Systems, Ltd.
 *****************************************************************************/


#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"
#include "portlayer.h"
#include "sipinit.h"
#ifdef SIP_CCP
#include "ccpinit.h"
#include "ccpfree.h"
#endif

#ifdef SIP_IMPP
#include "imppinit.h"
#include "imppfree.h"
#endif

#include "sipbcptinit.h"
#include "sipbcptfree.h"
#include "rprinit.h"
#include "rprfree.h"

#ifdef SIP_DCS
#include "dcsinit.h"
#endif

SIP_S8bit* hss_sip_part_id = (SIP_S8bit *) SIP_PART_ID;
SIP_S8bit* hss_sip_version_id = (SIP_S8bit *) SIP_STACK_VERSION_ID;

/*************************************************************************************
**** FUNCTION:sip_initString
****
****
**** DESCRIPTION:
*************************************************************************************/
/*
SipBool sip_initString
#ifdef ANSI_PROTO
	(SIP_S8bit **s,SipError *err)
#else
	(s)
	SIP_S8bit **s;
#endif
{
	INIT(*s);
	return SipSuccess;
}*/
/***********************************************************************************
**** FUNCTION:sip_initSipTimerKey
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_initSipTimerKey
#ifdef ANSI_PROTO
	(SipTimerKey **k,SipError *err)
#else
	(k,err)
	SipTimerKey **k;
	SipError *err;
#endif
{
	*k= ( SipTimerKey * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipTimerKey), err);
	if(*k==SIP_NULL)
		return SipFail;
	INIT((*k)->dCallid);
	INIT((*k)->pMethod);
	INIT((*k)->pRackHdr);
	INIT((*k)->pFromHdr);
	INIT((*k)->pToHdr);
	INIT((*k)->pViaBranch);
	(*k)->dCseq = 0;
	(*k)->dRseq = 0; 		/* rpr */
	(*k)->dMatchFlag = TIMER_NONE;
	HSS_INITREF((*k)->dRefCount);
	return SipSuccess;
}

/***********************************************************************************
**** FUNCTION:sip_initSipTimerBuffer
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_initSipTimerBuffer
#ifdef ANSI_PROTO
	(SipTimerBuffer **b,SipError *err)
#else
	(b,err)
	SipTimerBuffer **b;
	SipError *err;
#endif
{
	*b= ( SipTimerBuffer * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipTimerBuffer), err);
	if(*b==SIP_NULL)
		return SipFail;
	INIT((*b)->pBuffer);
	INIT((*b)->pContext);
	HSS_INITREF((*b)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpOrigin
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpOrigin
#ifdef ANSI_PROTO
	(SdpOrigin **s,SipError *err)
#else
	(s,err)
	SdpOrigin **s;
	SipError *err;
#endif
{
	*s= (SdpOrigin *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SdpOrigin),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT((*s)->pUser);
        INIT((*s)->pSessionid);
	INIT((*s)->pVersion);
	INIT((*s)->pNetType);
	INIT((*s)->pAddrType);
	INIT((*s)->dAddr);
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpMedia
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpMedia
#ifdef ANSI_PROTO
	(SdpMedia **m,SipError *err)
#else
	(m,err)
	SdpMedia **m;
	SipError *err;
#endif
{
	*m= (SdpMedia *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SdpMedia),err);
	if (*m==SIP_NULL)
		return SipFail;
	INIT((*m)->pInformation);
	sip_listInit(& ((*m)->slConnection ),__sip_freeSdpConnection,err);
	sip_listInit(& ((*m)->slBandwidth ),__sip_freeString,err);
	INIT((*m)->pKey);
	INIT((*m)->pFormat);
	INIT((*m)->pPortNum);
	INIT((*m)->pProtocol);
	sip_listInit(& ((*m)->slAttr ),__sip_freeSdpAttr,err);
	INIT ((*m)->pMediaValue);
	sip_listInit(& ((*m)->slIncorrectLines ),__sip_freeString,err);

#ifdef SIP_ATM
	INIT((*m)->pVirtualCID);
	sip_listInit(& ((*m)->slProtofmt ),__sip_freeSipNameValuePair,err);
#endif

	HSS_INITREF((*m)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpAttr
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpAttr
#ifdef ANSI_PROTO
	(SdpAttr **a,SipError *err)
#else
	(a,err)
	SdpAttr **a;
	SipError *err;
#endif
{
	*a= (SdpAttr *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SdpAttr),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pName);
	INIT((*a)->pValue);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpTime
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpTime
#ifdef ANSI_PROTO
	(SdpTime **t,SipError *err)
#else
	(t,err)
	SdpTime **t;
	SipError *err;
#endif
{
	*t= (SdpTime *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SdpTime),err);
	if (*t==SIP_NULL)
		return SipFail;
	INIT((*t)->pStart);
	INIT((*t)->pStop);
	sip_listInit(& ((*t)->slRepeat),__sip_freeString,err);
	INIT((*t)->pZone);
	HSS_INITREF((*t)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpConnection
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpConnection
#ifdef ANSI_PROTO
	( SdpConnection **c,SipError *err)
#else
	(c,err)
 	SdpConnection **c;
	SipError *err;
#endif
{
	*c= ( SdpConnection *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof( SdpConnection),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pNetType);
	INIT((*c)->pAddrType);
	INIT((*c)->dAddr);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSdpMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSdpMessage
#ifdef ANSI_PROTO
	(SdpMessage **m,SipError *err)
#else
	(m,err)
	SdpMessage **m;
	SipError *err;
#endif
{
	*m= (SdpMessage *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SdpMessage),err);
	if (*m==SIP_NULL)
		return SipFail;
	INIT((*m)->pVersion);
	INIT((*m)->pOrigin);
	INIT((*m)->pSession);
	INIT((*m)->pInformation);
	INIT((*m)->pUri);
	sip_listInit(& ((*m)->slEmail ),__sip_freeString,err);
	sip_listInit(& ((*m)->slPhone ),__sip_freeString,err);
	INIT((*m)->slConnection);
	sip_listInit(& ((*m)->pBandwidth ),__sip_freeString,err);
	sip_listInit(& ((*m)->slTime ),__sip_freeSdpTime,err);
	INIT((*m)->pKey);
	sip_listInit(& ((*m)->slAttr ),__sip_freeSdpAttr,err);
	sip_listInit(& ((*m)->slMedia ),__sip_freeSdpMedia,err);
	sip_listInit(& ((*m)->slIncorrectLines ),__sip_freeString,err);

	HSS_INITREF((*m)->dRefCount);


	return SipSuccess;
}

/***********************************************************************************
**** FUNCTION:sip_initSipParam
****
****
**** DESCRIPTION:
***********************************************************************************/

SipBool sip_initSipParam
#ifdef ANSI_PROTO
	(SipParam **p,SipError *err)
#else
	(p,err)
	SipParam **p;
	SipError *err;
#endif
{
	*p= ( SipParam * ) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipParam), err);
	if(*p==SIP_NULL)
		return SipFail;
	INIT((*p)->pName);
	sip_listInit(&((*p)->slValue),&__sip_freeString,err);
	HSS_INITREF((*p)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipGenericChallenge
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipGenericChallenge
#ifdef ANSI_PROTO
	(SipGenericChallenge **c,SipError *err)
#else
	(c,err)
	SipGenericChallenge **c;
	SipError *err;
#endif
{
	*c= (SipGenericChallenge *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipGenericChallenge),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pScheme);
	sip_listInit(& ((*c)->slParam ),__sip_freeSipParam,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipWwwAuthenticateHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipWwwAuthenticateHeader
#ifdef ANSI_PROTO
	(SipWwwAuthenticateHeader **h,SipError *err)
#else
	(h,err)
	SipWwwAuthenticateHeader **h;
	SipError *err;
#endif
{
	*h= (SipWwwAuthenticateHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipWwwAuthenticateHeader),err);
	if (*h==SIP_NULL)
		return SipFail;
	INIT((*h)->pChallenge);
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipProxyAuthenticateHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipProxyAuthenticateHeader
#ifdef ANSI_PROTO
	(SipProxyAuthenticateHeader **p,SipError *err)
#else
	(p,err)
	SipProxyAuthenticateHeader **p;
	SipError *err;
#endif
{
	*p= (SipProxyAuthenticateHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipProxyAuthenticateHeader),err);
	if (*p==SIP_NULL)
		return SipFail;

	INIT((*p)->pChallenge);
	HSS_INITREF((*p)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAuthParam
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/
/*
SipBool sip_initSipAuthParam
#ifdef ANSI_PROTO
	(SipAuthParam **a,SipError *err)
#else
	(a,err)
	SipAuthParam **a;
	SipError *err;
#endif
{
	*a= (SipAuthParam *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAuthParam),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pToken);
	INIT((*a)->pValue);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}
*/
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipWarningHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipWarningHeader
#ifdef ANSI_PROTO
	(SipWarningHeader **w,SipError *err)
#else
	(w,err)
	SipWarningHeader **w;
	SipError *err;
#endif
{
	*w= (SipWarningHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipWarningHeader),err);
	if (*w==SIP_NULL)
		return SipFail;
	INIT((*w)->pAgent);
	INIT((*w)->pText);
	HSS_INITREF((*w)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipDateFormat
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat **d,SipError *err)
#else
	(d,err)
	SipDateFormat **d;
	SipError *err;
#endif
{
	*d= (SipDateFormat *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipDateFormat),err);
	if (*d==SIP_NULL)
		return SipFail;
	HSS_INITREF((*d)->dRefCount);
	/** No pointers so no init necessary **/
return SipSuccess;}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipTimeFormat
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat **t,SipError *err)
#else
	(t,err)
	SipTimeFormat **t;
	SipError *err;
#endif
{
	*t= (SipTimeFormat *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipTimeFormat),err);
	if (*t==SIP_NULL)
		return SipFail;
	/** No pointers so no init necessary **/
	HSS_INITREF((*t)->dRefCount);
return SipSuccess;}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipDateStruct
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct **d,SipError *err)
#else
	(d,err)
	SipDateStruct **d;
	SipError *err;
#endif
{
	*d= (SipDateStruct *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipDateStruct),err);
	if (*d==SIP_NULL)
		return SipFail;
	INIT((*d)->pDate);
	INIT((*d)->pTime);
	HSS_INITREF((*d)->dRefCount);
return SipSuccess;}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipDateHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipDateHeader
#ifdef ANSI_PROTO
	(SipDateHeader **d,SipError *err)
#else
	(d,err)
	SipDateHeader **d;
	SipError *err;
#endif
{
	*d= (SipDateHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipDateHeader),err);
	if (*d==SIP_NULL)
		return SipFail;
	INIT((*d)->pDate);
	INIT((*d)->pTime);
	HSS_INITREF((*d)->dRefCount);
return SipSuccess;}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAllowHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAllowHeader
#ifdef ANSI_PROTO
	(SipAllowHeader **a,SipError *err)
#else
	(a,err)
	SipAllowHeader **a;
	SipError *err;
#endif
{
	*a= (SipAllowHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAllowHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pMethod);
	HSS_INITREF((*a)->dRefCount);
return SipSuccess;}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRetryAfterHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRetryAfterHeader
#ifdef ANSI_PROTO
	(SipRetryAfterHeader **r,en_ExpiresType dType,SipError *err)
#else
	(r,dType,err)
	SipRetryAfterHeader **r;
	en_ExpiresType dType;
	SipError *err;
#endif
{
	*r= (SipRetryAfterHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRetryAfterHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	(*r)->dType=dType; /**got to validate**/
	if(dType == SipExpDate)
		INIT((*r)->u.pDate);
	INIT((*r)->pComment);
	sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);

	/* INIT((*r)->pDuration); */
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipGenericCredential
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipGenericCredential
#ifdef ANSI_PROTO
	(SipGenericCredential **c,en_CredentialType dType,SipError *err)
#else
	(c,dType,err)
	SipGenericCredential **c;
	en_CredentialType dType;
	SipError *err;
#endif
{
	*c= (SipGenericCredential *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipGenericCredential),err);
	if (*c==SIP_NULL)
		return SipFail;
	(*c)->dType=dType;
	if ((*c)->dType==SipCredAuth)
	{
		INIT((*c)->u.pChallenge);
	}
	else if((*c)->dType==SipCredBasic)
	{
		INIT((*c)->u.pBasic);
	}
	else if((*c)->dType != SipCredAny)
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAuthorizationHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAuthorizationHeader
#ifdef ANSI_PROTO
	(SipAuthorizationHeader **a,SipError *err)
#else
	(a,err)
	SipAuthorizationHeader **a;
	SipError *err;
#endif
{
	*a= (SipAuthorizationHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAuthorizationHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pCredential);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReferToHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReferToHeader
#ifdef ANSI_PROTO
	(SipReferToHeader **a,SipError *err)
#else
	(a,err)
	SipReferToHeader **a;
	SipError *err;
#endif
{
	*a= (SipReferToHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReferToHeader),err);
	if (*a==SIP_NULL)
		return SipFail;

	INIT((*a)->pDispName);
	INIT((*a)->pAddrSpec);
	sip_listInit(&((*a)->slParams),__sip_freeSipParam,err);

	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReferredByHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReferredByHeader
#ifdef ANSI_PROTO
	(SipReferredByHeader **a,SipError *err)
#else
	(a,err)
	SipReferredByHeader **a;
	SipError *err;
#endif
{
	*a= (SipReferredByHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReferredByHeader),err);
	if (*a==SIP_NULL)
		return SipFail;

	INIT((*a)->pDispName);
	INIT((*a)->pMsgId);
	INIT((*a)->pAddrSpecReferrer);
	INIT((*a)->pAddrSpecReferenced);
	sip_listInit(&((*a)->slParams),__sip_freeSipParam,err);

	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRespHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRespHeader
#ifdef ANSI_PROTO
	(SipRespHeader **h,SipError *err)
#else
	(h,err)
	SipRespHeader **h;
	SipError *err;
#endif
{
	*h= (SipRespHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRespHeader),err);
	if (*h==SIP_NULL)
		return SipFail;
	sip_listInit(& ((*h)->slProxyAuthenticateHdr),__sip_freeSipProxyAuthenticateHeader,err);
	sip_listInit(& ((*h)->slUnsupportedHdr),__sip_freeSipUnsupportedHeader,err);
	sip_listInit(& ((*h)->slWarningHeader),__sip_freeSipWarningHeader,err);
	sip_listInit(& ((*h)->slWwwAuthenticateHdr),__sip_freeSipWwwAuthenticateHeader,err);
	sip_listInit(& ((*h)->slAuthenticationInfoHdr),__sip_freeSipAuthenticationInfoHeader,err);
	sip_listInit(& ((*h)->slErrorInfoHdr),__sip_freeSipErrorInfoHeader,err);

	INIT((*h)->pServerHdr);

	sip_listInit(& ((*h)->slAuthorizationHdr),\
		__sip_freeSipAuthorizationHeader,err);

	INIT((*h)->pRSeqHdr); /* Retrans */
	INIT((*h)->pMinExpiresHdr);
#ifdef SIP_DCS
	sip_dcs_initDcsRespHeaders(h, err);
#endif
	
#ifdef SIP_3GPP
    sip_listInit(& ((*h)->slPAssociatedUriHdr),__sip_freeSipPAssociatedUriHeader,err);
#endif
#ifdef SIP_CONGEST
    sip_listInit(& ((*h)->slAcceptRsrcPriorityHdr),__sip_freeSipAcceptRsrcPriorityHeader,err);
#endif

#ifdef SIP_SECURITY
    sip_listInit(& ((*h)->slSecurityServerHdr),__sip_freeSipSecurityServerHeader,err);
#endif
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRespKeyHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRespKeyHeader
#ifdef ANSI_PROTO
	( SipRespKeyHeader** r,SipError *err)
#else
	( r,err)
 	SipRespKeyHeader** r;
	SipError *err;
#endif
{
	* r= ( SipRespKeyHeader*) fast_memget(NON_SPECIFIC_MEM_ID, sizeof( SipRespKeyHeader),err);
	if (* r==SIP_NULL)
		return SipFail;
	INIT((*r)->pKeyScheme);
	sip_listInit(&((*r)->slParam),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUserAgentHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipUserAgentHeader
#ifdef ANSI_PROTO
	(SipUserAgentHeader **u,SipError *err)
#else
	(u,err)
	SipUserAgentHeader **u;
	SipError *err;
#endif
{
	*u= (SipUserAgentHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUserAgentHeader),err);
	if (*u==SIP_NULL)
		return SipFail;
	INIT((*u)->pValue);
	HSS_INITREF((*u)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipSubjectHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipSubjectHeader
#ifdef ANSI_PROTO
	(SipSubjectHeader **s,SipError *err)
#else
	(s,err)
	SipSubjectHeader **s;
	SipError *err;
#endif
{
	*s= (SipSubjectHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipSubjectHeader),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT((*s)->pSubject);
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipProxyRequireHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipProxyRequireHeader
#ifdef ANSI_PROTO
	(SipProxyRequireHeader **p,SipError *err)
#else
	(p,err)
	SipProxyRequireHeader **p;
	SipError *err;
#endif
{
	*p= (SipProxyRequireHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipProxyRequireHeader),err);
	if (*p==SIP_NULL)
		return SipFail;
	INIT((*p)->pToken);
	HSS_INITREF((*p)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipProxyAuthorizationHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipProxyAuthorizationHeader
#ifdef ANSI_PROTO
	(SipProxyAuthorizationHeader **a,SipError *err)
#else
	(a,err)
	SipProxyAuthorizationHeader **a;
	SipError *err;
#endif
{
	*a= (SipProxyAuthorizationHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipProxyAuthorizationHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pCredentials);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPriorityHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipPriorityHeader
#ifdef ANSI_PROTO
	(SipPriorityHeader **p,SipError *err)
#else
	(p,err)
	SipPriorityHeader **p;
	SipError *err;
#endif
{
	*p= (SipPriorityHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPriorityHeader),err);
	if (*p==SIP_NULL)
		return SipFail;
	INIT((*p)->pPriority);
	HSS_INITREF((*p)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipOrganizationHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipOrganizationHeader
#ifdef ANSI_PROTO
	(SipOrganizationHeader **o,SipError *err)
#else
	(o,err)
	SipOrganizationHeader **o;
	SipError *err;
#endif
{
	*o= (SipOrganizationHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipOrganizationHeader),err);
	if (*o==SIP_NULL)
		return SipFail;
	INIT((*o)->pOrganization);
	HSS_INITREF((*o)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContentTypeHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipContentTypeHeader
#ifdef ANSI_PROTO
	(SipContentTypeHeader **c,SipError *err)
#else
	(c,err)
	SipContentTypeHeader **c;
	SipError *err;
#endif
{
	*c= (SipContentTypeHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipContentTypeHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pMediaType);
	sip_listInit(&((*c)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContentEncodingHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipContentEncodingHeader
#ifdef ANSI_PROTO
	(SipContentEncodingHeader **c,SipError *err)
#else
	(c,err)
	SipContentEncodingHeader **c;
	SipError *err;
#endif
{
	*c= (SipContentEncodingHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipContentEncodingHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pEncoding);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipHideHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipHideHeader
#ifdef ANSI_PROTO
	(SipHideHeader **h,SipError *err)
#else
	(h,err)
	SipHideHeader **h;
	SipError *err;
#endif
{
	*h= (SipHideHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipHideHeader),err);
	if (*h==SIP_NULL)
		return SipFail;
	INIT((*h)->pType);
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipMaxForwardsHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipMaxForwardsHeader
#ifdef ANSI_PROTO
	(SipMaxForwardsHeader **h,SipError *err)
#else
	(h,err)
	SipMaxForwardsHeader **h;
	SipError *err;
#endif
{
	*h= (SipMaxForwardsHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipMaxForwardsHeader),err);
	if (*h==SIP_NULL)
		return SipFail;
	/** No pointers so no init necessary **/
	HSS_INITREF((*h)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipViaParam
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/
/*
SipBool sip_initSipViaParam
#ifdef ANSI_PROTO
	(SipViaParam **v,en_ViaParamType dType,SipError *err)
#else
	(v,dType, err)
	SipViaParam **v;
	en_ViaParamType dType;
	SipError *err;
#endif
{
	*v= (SipViaParam *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipViaParam),err);
	if (*v==SIP_NULL)
		return SipFail;
	(*v)->dType=dType;
	switch ((*v)->dType)
	{
		case SipViaParamHidden:
			INIT((*v)->u.pHidden);
			break;
		case SipViaParamMaddr:
			INIT((*v)->u.pMaddr);
			break;
		case SipViaParamReceived:
			INIT((*v)->u.pViaReceived);
			break;
		case SipViaParamBranch:
			INIT((*v)->u.pViaBranch);
			break;
		case SipViaParamExtension:
			INIT((*v)->u.pExtParam);
			break;
		case SipViaParamTtl:
		case SipViaParamAny:
			break;
	}
	HSS_INITREF((*v)->dRefCount);
	return SipSuccess;
}
*/

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContentLengthHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipContentLengthHeader
#ifdef ANSI_PROTO
	(SipContentLengthHeader **c,SipError *err)
#else
	(c,err)
	SipContentLengthHeader **c;
	SipError *err;
#endif
{
	*c= (SipContentLengthHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipContentLengthHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	/** No pointers so no init necessary **/
	HSS_INITREF((*c)->dRefCount);
	(*c)->dLength = 0;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipViaHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipViaHeader
#ifdef ANSI_PROTO
	(SipViaHeader **v,SipError *err)
#else
	(v,err)
	SipViaHeader **v;
	SipError *err;
#endif
{
	*v= (SipViaHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipViaHeader),err);
	if (*v==SIP_NULL)
		return SipFail;
	INIT((*v)->pSentProtocol);
	INIT((*v)->pSentBy);
	sip_listInit(& ((*v)->slParam),__sip_freeSipParam,err); /*changed slViaParam to slParam and SipViaParam to SipParam */
	INIT((*v)->pComment);
	HSS_INITREF((*v)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUrlParam
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/
/* changed to SipParam
SipBool sip_initSipUrlParam
#ifdef ANSI_PROTO
	(SipUrlParam **u,en_UrlParamType dType,SipError *err)
#else
	(u,dType,err)
	SipUrlParam **u;
	en_UrlParamType dType;
	SipError *err;
#endif
{
	*u= (SipUrlParam *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUrlParam),err);
	if (*u==SIP_NULL)
		return SipFail;
	(*u)->dType=dType;
	switch((*u)->dType)
	{
		case SipUrlParamMethod:
			INIT((*u)->u.pMethod);
			break;
		case SipUrlParamMaddr:
			INIT((*u)->u.pMaddr);
			break;
		case SipUrlParamOther:
			INIT((*u)->u.pOtherParam);
			break;
		case SipUrlParamTransport:
		case SipUrlParamUser:
		case SipUrlParamTtl:
		case SipUrlParamAny:
			break;
	}
	HSS_INITREF((*u)->dRefCount);
	return SipSuccess;
}
*/

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUrl
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipUrl
#ifdef ANSI_PROTO
	(SipUrl **u,SipError *err)
#else
	(u,err)
	SipUrl **u;
	SipError *err;
#endif
{
	*u= (SipUrl *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUrl),err);
	if (*u==SIP_NULL)
		return SipFail;
	INIT((*u)->pUser);
	INIT((*u)->pPassword);
	INIT((*u)->pHost);
	INIT((*u)->dPort);
	sip_listInit(& ((*u)->slParam),__sip_freeSipParam,err); /* changed from __freeSipUrlParam */
	INIT((*u)->pHeader);
	HSS_INITREF((*u)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAddrSpec
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec **a,en_AddrType dType,SipError *err)
#else
	(a,dType,err)
	SipAddrSpec **a;
	en_AddrType dType;
	SipError *err;
#endif
{
	*a= (SipAddrSpec *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAddrSpec),err);
	if (*a==SIP_NULL)
		return SipFail;
	(*a)->dType=dType;/**got to validate**/
	if ((*a)->dType == SipAddrReqUri)
		INIT((*a)->u.pUri);
	else if (((*a)->dType == SipAddrSipUri) || ((*a)->dType == SipAddrSipSUri))
		INIT((*a)->u.pSipUrl);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipToHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipToHeader
#ifdef ANSI_PROTO
	(SipToHeader **t,SipError *err)
#else
	(t,err)
	SipToHeader **t;
	SipError *err;
#endif
{
	*t= (SipToHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipToHeader),err);
	if (*t==SIP_NULL)
		return SipFail;
	INIT((*t)->pDispName);
	INIT((*t)->pAddrSpec);
	sip_listInit(& ((*t)->slTag),__sip_freeString,err);
	sip_listInit(& ((*t)->slParam),__sip_freeSipParam,err);
	HSS_INITREF((*t)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipTimeStampHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipTimeStampHeader
#ifdef ANSI_PROTO
	(SipTimeStampHeader **t,SipError *err)
#else
	(t,err)
	SipTimeStampHeader **t;
	SipError *err;
#endif
{
	*t= (SipTimeStampHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipTimeStampHeader),err);
	if (*t==SIP_NULL)
		return SipFail;
	INIT((*t)->pTime);
	INIT((*t)->delay);
	HSS_INITREF((*t)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:Sip_initSipRouteHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRouteHeader
#ifdef ANSI_PROTO
	(SipRouteHeader **r,SipError *err)
#else
	(r,err)
	SipRouteHeader **r;
	SipError *err;
#endif
{
	*r= (SipRouteHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRouteHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pDispName);
	INIT((*r)->pAddrSpec);
	sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRecordRouteHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRecordRouteHeader
#ifdef ANSI_PROTO
	(SipRecordRouteHeader **r,SipError *err)
#else
	(r,err)
	SipRecordRouteHeader **r;
	SipError *err;
#endif
{
	*r= (SipRecordRouteHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRecordRouteHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pDispName);
	INIT((*r)->pAddrSpec);
	sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

#ifdef SIP_3GPP
/*******************************************************************
**** FUNCTION:sip_initSipPathHeader
****
****
**** DESCRIPTION: Initilizes the SipPathHeader Structure
********************************************************************/
SipBool sip_initSipPathHeader
#ifdef ANSI_PROTO
	(SipPathHeader **ppPath,SipError *pErr)
#else
	(ppPath,pErr)
	SipPathHeader **ppPath;
	SipError *pErr;
#endif
{
	*ppPath=(SipPathHeader*)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipPathHeader),pErr);
	if (*ppPath==SIP_NULL)
		return SipFail;
	INIT((*ppPath)->pDispName);
	INIT((*ppPath)->pAddrSpec);
	sip_listInit(&((*ppPath)->slParams),__sip_freeSipParam,pErr);
	HSS_INITREF((*ppPath)->dRefCount);
	return SipSuccess;
}

/*******************************************************************
** FUNCTION:sip_initSipServiceRouteHeader
**
** DESCRIPTION: Initilizes the SipServiceRouteHeader Structure
********************************************************************/
SipBool sip_initSipServiceRouteHeader
#ifdef ANSI_PROTO
	(SipServiceRouteHeader **ppService,SipError *pErr)
#else
	(ppService,pErr)
	SipServiceRouteHeader **ppService;
	SipError *pErr;
#endif
{
	*ppService=(SipServiceRouteHeader*)fast_memget(NON_SPECIFIC_MEM_ID,
					sizeof(SipServiceRouteHeader),pErr);
	if (*ppService==SIP_NULL)
		return SipFail;
	INIT((*ppService)->pDispName);
	INIT((*ppService)->pAddrSpec);
	sip_listInit(&((*ppService)->slParams),__sip_freeSipParam,pErr);
	HSS_INITREF((*ppService)->dRefCount);
	return SipSuccess;
}

/*******************************************************************
**** FUNCTION:sip_initSipPanInfoHeader
****
****
**** DESCRIPTION: Initilizes the SipPanInfoHeader Structure
********************************************************************/
SipBool sip_initSipPanInfoHeader
#ifdef ANSI_PROTO
	(SipPanInfoHeader **pHdr,SipError *pErr)
#else
	(pHdr,pErr)
	SipPanInfoHeader **pHdr;
	SipError *pErr;
#endif
{
	*pHdr=(SipPanInfoHeader*)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipPanInfoHeader),pErr);
	if (*pHdr==SIP_NULL)
		return SipFail;
	INIT((*pHdr)->pAccessType);
	sip_listInit(&((*pHdr)->slParams),__sip_freeSipNameValuePair,pErr);
	HSS_INITREF((*pHdr)->dRefCount);
	return SipSuccess;
}
/*******************************************************************
**** FUNCTION:sip_initSipPcVectorHeader
****
****
**** DESCRIPTION: Initilizes the SipPcVectorHeader Structure
********************************************************************/
SipBool sip_initSipPcVectorHeader
#ifdef ANSI_PROTO
	(SipPcVectorHeader **pHdr,SipError *pErr)
#else
	(pHdr,pErr)
	SipPcVectorHeader **pHdr;
	SipError *pErr;
#endif
{
	*pHdr=(SipPcVectorHeader*)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipPcVectorHeader),pErr);
	if (*pHdr==SIP_NULL)
		return SipFail;
	INIT((*pHdr)->pAccessType);
	INIT((*pHdr)->pAccessValue);
	sip_listInit(&((*pHdr)->slParams),__sip_freeSipNameValuePair,pErr);
	HSS_INITREF((*pHdr)->dRefCount);
	return SipSuccess;
}

#endif
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRequireHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRequireHeader
#ifdef ANSI_PROTO
	(SipRequireHeader **r,SipError *err)
#else
	(r,err)
	SipRequireHeader **r;
	SipError *err;
#endif
{
	*r= (SipRequireHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRequireHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pToken);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipFromHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipFromHeader
#ifdef ANSI_PROTO
	(SipFromHeader **t,SipError *err)
#else
	(t,err)
	SipFromHeader **t;
	SipError *err;
#endif
{
	*t= (SipFromHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipFromHeader),err);
	if (*t==SIP_NULL)
		return SipFail;
	INIT((*t)->pDispName);
	INIT((*t)->pAddrSpec);
	sip_listInit(& ((*t)->slTag),__sip_freeString,err);
	sip_listInit(& ((*t)->slParam),__sip_freeSipParam,err);
	HSS_INITREF((*t)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipExpiresStruct
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipExpiresStruct
#ifdef ANSI_PROTO
	(SipExpiresStruct **e,en_ExpiresType dType,SipError *err)
#else
	(e,dType,err)
	SipExpiresStruct **e;
	en_ExpiresType dType;
	SipError *err;
#endif
{
	*e= (SipExpiresStruct *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipExpiresStruct),err);
	if (*e==SIP_NULL)
		return SipFail;
	(*e)->dType=dType;
	if(dType == SipExpDate)
		INIT((*e)->u.pDate);
	else
		(*e)->u.pDate=SIP_NULL;
	HSS_INITREF((*e)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipExpiresHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipExpiresHeader
#ifdef ANSI_PROTO
	(SipExpiresHeader **e,en_ExpiresType dType,SipError *err)
#else
	(e,dType,err)
	SipExpiresHeader **e;
	en_ExpiresType dType;
	SipError *err;
#endif
{
	*e= (SipExpiresHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipExpiresHeader),err);
	if (*e==SIP_NULL)
		return SipFail;
	(*e)->dType=dType;
	if(dType == SipExpDate)
		INIT((*e)->u.pDate);
	HSS_INITREF((*e)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipEncryptionHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipEncryptionHeader
#ifdef ANSI_PROTO
	(SipEncryptionHeader **e,SipError *err)
#else
	(e,err)
	SipEncryptionHeader **e;
	SipError *err;
#endif
{
	*e= (SipEncryptionHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipEncryptionHeader),err);
	if (*e==SIP_NULL)
		return SipFail;
	INIT((*e)->pScheme);
	sip_listInit(& ((*e)->slParam),__sip_freeSipParam,err);
	HSS_INITREF((*e)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContactParam
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipContactParam
#ifdef ANSI_PROTO
	(SipContactParam **c,en_ContactParamsType dType,SipError *err)
#else
	(c,dType,err)
	SipContactParam **c;
	en_ContactParamsType dType;
	SipError *err;
#endif
{
	*c= (SipContactParam *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipContactParam),err);
	if (*c==SIP_NULL)
		return SipFail;
	(*c)->dType=dType;
	switch((*c)->dType)
	{
		case SipCParamQvalue:
			INIT((*c)->u.pQValue);
			break;
		case SipCParamExpires:
			INIT((*c)->u.pExpire);
			break;
		case SipCParamExtension:
			INIT((*c)->u.pExtensionAttr);
			break;
		case SipCParamFeatureParam :
			INIT((*c)->u.pParam);
			break;
		case SipCParamAny:
			break;
	} /**switch**/
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContactHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipContactHeader
#ifdef ANSI_PROTO
	(SipContactHeader **c,en_ContactType dType, SipError *err)
#else
	(c,dType, err)
	SipContactHeader **c;
	en_ContactType dType;
	SipError *err;
#endif
{
	*c= (SipContactHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipContactHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	(*c)->dType = dType;
	INIT((*c)->pDispName);
	INIT((*c)->pAddrSpec);
	sip_listInit(& ((*c)->slContactParam),__sip_freeSipContactParam,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipCseqHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipCseqHeader
#ifdef ANSI_PROTO
	(SipCseqHeader **c,SipError *err)
#else
	(c,err)
	SipCseqHeader **c;
	SipError *err;
#endif
{
	*c= (SipCseqHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipCseqHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pMethod);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipCallIdHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipCallIdHeader
#ifdef ANSI_PROTO
	(SipCallIdHeader **c,SipError *err)
#else
	(c,err)
	SipCallIdHeader **c;
	SipError *err;
#endif
{
	*c= (SipCallIdHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipCallIdHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pValue);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReplacesHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReplacesHeader
#ifdef ANSI_PROTO
	(SipReplacesHeader **c,SipError *err)
#else
	(c,err)
	SipReplacesHeader **c;
	SipError *err;
#endif
{
	*c= (SipReplacesHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReplacesHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pCallid);
	INIT((*c)->pFromTag);
	INIT((*c)->pToTag);
	sip_listInit(&((*c)->slParams),__sip_freeSipNameValuePair,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReplyToHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReplyToHeader
#ifdef ANSI_PROTO
	(SipReplyToHeader **c,SipError *err)
#else
	(c,err)
	SipReplyToHeader **c;
	SipError *err;
#endif
{
	*c= (SipReplyToHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReplyToHeader),err);
	if (*c==SIP_NULL)
		return SipFail;
	INIT((*c)->pDispName);
	INIT((*c)->pAddrSpec);
	sip_listInit(&((*c)->slParams),__sip_freeSipNameValuePair,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAcceptHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAcceptHeader
#ifdef ANSI_PROTO
	(SipAcceptHeader **a,SipError *err)
#else
	(a,err)
	SipAcceptHeader **a;
	SipError *err;
#endif
{
	*a= (SipAcceptHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAcceptHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pAcceptRange);
	INIT((*a)->pMediaRange);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAcceptEncodingHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAcceptEncodingHeader
#ifdef ANSI_PROTO
	(SipAcceptEncodingHeader **a,SipError *err)
#else
	(a,err)
	SipAcceptEncodingHeader **a;
	SipError *err;
#endif
{
	*a= (SipAcceptEncodingHeader *)\
		fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAcceptEncodingHeader),err);
	if (*a==SIP_NULL)
		return SipFail;

	sip_listInit(& ((*a)->slParam),__sip_freeSipNameValuePair,err);

	INIT((*a)->pCoding);
	INIT((*a)->pQValue);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAcceptLangHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipAcceptLangHeader
#ifdef ANSI_PROTO
	(SipAcceptLangHeader **a,SipError *err)
#else
	(a,err)
	SipAcceptLangHeader **a;
	SipError *err;
#endif
{
	*a= (SipAcceptLangHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAcceptLangHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pLangRange);
	INIT((*a)->pQValue);
	sip_listInit(& ((*a)->slParam),__sip_freeSipNameValuePair,err);

	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipServerHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipServerHeader
#ifdef ANSI_PROTO
	(SipServerHeader **s,SipError *err)
#else
	(s,err)
	SipServerHeader **s;
	SipError *err;
#endif
{
	*s= (SipServerHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipServerHeader),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT((*s)->pValue);
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

#ifdef SIP_SECURITY
/*************************************************************************************************
***********
**** FUNCTION:sip_initSipSecurityClientHeader
****
****
**** DESCRIPTION:
**************************************************************************************************
************************************/

SipBool sip_initSipSecurityClientHeader
        (SipSecurityClientHeader **v,
	SipError *err)
{
        *v= (SipSecurityClientHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipSecurityClientHeader),err);
        if (*v==SIP_NULL)
                return SipFail;
        INIT((*v)->pMechanismName);
        sip_listInit(& ((*v)->slParams),__sip_freeSipParam,err);
        HSS_INITREF((*v)->dRefCount);
        return SipSuccess;
}

/*******************************************************************************************
*****************
**** FUNCTION:sip_initSipSecurityServerHeader
****
****
**** DESCRIPTION:
********************************************************************************************
******************************************/

SipBool sip_initSipSecurityServerHeader
        (SipSecurityServerHeader **v,
	SipError *err)
{
        return sip_initSipSecurityClientHeader((SipSecurityClientHeader **)v, err );
}

/*******************************************************************************************
*****************
**** FUNCTION:sip_initSipSecurityVerifyHeader
****
****
**** DESCRIPTION:
********************************************************************************************
******************************************/

SipBool sip_initSipSecurityVerifyHeader
        (SipSecurityVerifyHeader **v,
	SipError *err)
{
        return sip_initSipSecurityClientHeader((SipSecurityClientHeader **)v, err );
}

#endif

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUnsupportedHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipUnsupportedHeader
#ifdef ANSI_PROTO
	(SipUnsupportedHeader **s,SipError *err)
#else
	(s,err)
	SipUnsupportedHeader **s;
	SipError *err;
#endif
{
	*s= (SipUnsupportedHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUnsupportedHeader),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT((*s)->pOption);
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUnknownHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipUnknownHeader
#ifdef ANSI_PROTO
	(SipUnknownHeader **u, SipError *err)
#else
	(u,err)
	SipUnknownHeader **u;
	SipError *err;
#endif
{
	*u= (SipUnknownHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUnknownHeader),err);
	if (*u==SIP_NULL)
		return SipFail;
	INIT((*u)->pName);
	INIT((*u)->pBody);
	HSS_INITREF((*u)->dRefCount);
	return SipSuccess;
}
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReqHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReqHeader
#ifdef ANSI_PROTO
	(SipReqHeader **s,SipError *err)
#else
	(s,err)
	SipReqHeader **s;
	SipError *err;
#endif
{
	*s= (SipReqHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReqHeader),err);
	if (*s==SIP_NULL)
		return SipFail;
	sip_listInit(& ((*s)->slAuthorizationHdr),\
		__sip_freeSipAuthorizationHeader,err);

	INIT((*s)->pHideHdr);
	INIT((*s)->pMaxForwardsHdr);
	INIT((*s)->pPriorityHdr);
	INIT((*s)->pRespKeyHdr);
	INIT((*s)->pReplacesHdr);
#ifdef SIP_IMPP
	INIT((*s)->pEventHdr);
#endif
	INIT((*s)->pSubjectHdr);
	INIT((*s)->pRackHdr); /* Retrans */
	INIT((*s)->pReferToHdr);
	INIT((*s)->pReferredByHdr);
#ifdef SIP_IMPP
	INIT((*s)->pSubscriptionStateHdr);
#endif

	sip_listInit(& ((*s)->slProxyAuthorizationHdr),\
		__sip_freeSipProxyAuthorizationHeader,err);
	sip_listInit(& ((*s)->slProxyRequireHdr),\
		__sip_freeSipProxyRequireHeader,err);
	sip_listInit(& ((*s)->slAlertInfoHdr),__sip_freeSipAlertInfoHeader,err);

	sip_listInit(& ((*s)->slInReplyToHdr),__sip_freeSipInReplyToHeader,err);
	sip_listInit(& ((*s)->slAlsoHdr),__sip_freeSipAlsoHeader, err);
	sip_listInit(& ((*s)->slRouteHdr),__sip_freeSipRouteHeader,err);


/*	sip_listInit(& ((*s)->slAlertInfoHdr),__sip_freeSipAlertInfoHeader,err);*/

	sip_listInit(& ((*s)->slWwwAuthenticateHdr),__sip_freeSipWwwAuthenticateHeader,err);
#ifdef SIP_CCP
	sip_listInit(& ((*s)->slAcceptContactHdr), __sip_ccp_freeSipAcceptContactHeader, err); /* CCP */
	sip_listInit(& ((*s)->slRejectContactHdr), __sip_ccp_freeSipRejectContactHeader, err); /* CCP */
	sip_listInit(& ((*s)->slRequestDispositionHdr), __sip_ccp_freeSipRequestDispositionHeader, err); /* CCP */
#endif /*cpp */

#ifdef SIP_DCS
	sip_dcs_initDcsReqHeaders(s, err);
#endif

#ifdef SIP_CONGEST
	sip_listInit(& ((*s)->slRsrcPriorityHdr),__sip_freeSipRsrcPriorityHeader,err);
#endif
#ifdef SIP_CONF    
	INIT((*s)->pJoinHdr);
#endif

#ifdef SIP_SECURITY
	sip_listInit(& ((*s)->slSecurityClientHdr),__sip_freeSipSecurityClientHeader,err);
	sip_listInit(& ((*s)->slSecurityVerifyHdr),__sip_freeSipSecurityVerifyHeader,err);
#endif
#ifdef SIP_3GPP
	INIT((*s)->pPCalledPartyIdHdr);
	sip_listInit(& ((*s)->slPVisitedNetworkIdHdr),__sip_freeSipPVisitedNetworkIdHeader,err);
#endif

	HSS_INITREF((*s)->dRefCount);

	return SipSuccess;
}
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipGeneralHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipGeneralHeader
#ifdef ANSI_PROTO
	(SipGeneralHeader **g,SipError *err)
#else
	(g,err)
	SipGeneralHeader **g;
	SipError *err;
#endif
{
	*g=(SipGeneralHeader *)fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipGeneralHeader),err);
	if(*g==SIP_NULL)
		return SipFail;
	sip_listInit(& ((*g)->slAcceptHdr),__sip_freeSipAcceptHeader,err);
	sip_listInit(& ((*g)->slAllowHdr),__sip_freeSipAllowHeader,err);
	sip_listInit(& ((*g)->slAcceptEncoding),__sip_freeSipAcceptEncodingHeader,err);
	sip_listInit(& ((*g)->slAcceptLang),__sip_freeSipAcceptLangHeader,err);
	sip_listInit(& ((*g)->slContactHdr),__sip_freeSipContactHeader,err);
	sip_listInit(& ((*g)->slRecordRouteHdr),__sip_freeSipRecordRouteHeader,err);
#ifdef SIP_3GPP
	sip_listInit(&((*g)->slPathHdr),__sip_freeSipPathHeader,err);
	sip_listInit(&((*g)->slServiceRouteHdr),__sip_freeSipServiceRouteHeader,err);
#endif
	sip_listInit(& ((*g)->slViaHdr),__sip_freeSipViaHeader,err);
	sip_listInit(& ((*g)->slContentEncodingHdr),__sip_freeSipContentEncodingHeader,err);
	sip_listInit(& ((*g)->slSupportedHdr),__sip_freeSipSupportedHeader,err);
	sip_listInit(& ((*g)->slCallInfoHdr),__sip_freeSipCallInfoHeader,err);
	sip_listInit(& ((*g)->slContentDispositionHdr),__sip_freeSipContentDispositionHeader,err);
	sip_listInit(& ((*g)->slReasonHdr),__sip_freeSipReasonHeader,err);
	sip_listInit(& ((*g)->slContentLanguageHdr),__sip_freeSipContentLanguageHeader,err);
	sip_listInit(& ((*g)->slRequireHdr),__sip_freeSipRequireHeader,err);
#ifdef SIP_IMPP
	sip_listInit(& ((*g)->slAllowEventsHdr),__sip_impp_freeSipAllowEventsHeader,err);
#endif
	sip_listInit(& ((*g)->slUnknownHdr),__sip_freeSipUnknownHeader,err);
	sip_listInit(& ((*g)->slBadHdr),__sip_freeSipBadHeader,err);
#ifdef SIP_PRIVACY
	sip_listInit(& ((*g)->slPAssertIdHdr),__sip_freeSipPAssertIdHeader,err);
	sip_listInit(& ((*g)->slPPreferredIdHdr),__sip_freeSipPPreferredIdHeader,err);
#endif /*  # ifdef SIP_PRIVACY */

	INIT((*g)->pCallidHdr);
	INIT((*g)->pReplyToHdr);
	INIT((*g)->pCseqHdr);
	INIT((*g)->pDateHdr);
	INIT((*g)->pEncryptionHdr);
	INIT((*g)->pExpiresHdr);
	INIT((*g)->pFromHeader);
	INIT((*g)->pTimeStampHdr);
	INIT((*g)->pToHdr);
	INIT((*g)->pContentLengthHdr);
	INIT((*g)->pContentTypeHdr);
	INIT((*g)->pMimeVersionHdr);   /* bcpt ext */
	INIT((*g)->pOrganizationHdr);
	INIT((*g)->pUserAgentHdr);
	INIT((*g)->pRetryAfterHdr);
#ifdef SIP_PRIVACY
	INIT((*g)->pPrivacyHdr);
#endif
#ifdef SIP_SESSIONTIMER
	INIT((*g)->pMinSEHdr);
	INIT((*g)->pSessionExpiresHdr);
#endif

#ifdef SIP_DCS
	sip_dcs_initDcsGeneralHeaders(g, err);
#endif
#ifdef SIP_3GPP
    INIT((*g)->pPanInfoHdr);
    INIT((*g)->pPcVectorHdr);
	INIT((*g)->pPcfAddrHdr);
#endif

	HSS_INITREF((*g)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReqLine
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReqLine
#ifdef ANSI_PROTO
	(SipReqLine **r,SipError *err)
#else
	(r,err)
	SipReqLine **r;
	SipError *err;
#endif
{
	*r= (SipReqLine *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReqLine),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pMethod);
	INIT((*r)->pRequestUri);
	INIT((*r)->pVersion);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipReqMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipReqMessage
#ifdef ANSI_PROTO
	(SipReqMessage **r,SipError *err)
#else
	(r,err)
	SipReqMessage **r;
	SipError *err;
#endif
{
	*r= (SipReqMessage *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipReqMessage),err);
	if (*r==SIP_NULL)
		return SipFail;
	sip_initSipReqLine(&(*r)->pRequestLine,err);
	sip_initSipReqHeader(&(*r)->pRequestHdr,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipStatusLine
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine **s,SipError *err)
#else
	(s,err)
	SipStatusLine **s;
	SipError *err;
#endif
{
	*s= (SipStatusLine *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipStatusLine),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT((*s)->pVersion);
	INIT((*s)->pReason);
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRespMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipRespMessage
#ifdef ANSI_PROTO
	(SipRespMessage **r,SipError *err)
#else
	(r,err)
	SipRespMessage **r;
	SipError *err;
#endif
{
	*r= (SipRespMessage *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipRespMessage),err);
	if (*r==SIP_NULL)
		return SipFail;
	sip_initSipStatusLine(&(*r)->pStatusLine,err);
	sip_initSipRespHeader(&(*r)->pResponseHdr,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipHeaderOrderInfo
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipHeaderOrderInfo
#ifdef ANSI_PROTO
	(SipHeaderOrderInfo **order,SipError *err)
#else
	(order, err)
	SipHeaderOrderInfo **order;
	SipError *err;
#endif
{
	*order= (SipHeaderOrderInfo *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipHeaderOrderInfo),err);
	if (*order==SIP_NULL)
		return SipFail;

	*err = E_NO_ERROR;
	HSS_INITREF((*order)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipUnknownMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipUnknownMessage
#ifdef ANSI_PROTO
	(SipUnknownMessage **s,SipError *err)
#else
	(s,err)
	SipUnknownMessage **s;
	SipError *err;
#endif
{
	*s= (SipUnknownMessage *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipUnknownMessage),err);
	if (*s==SIP_NULL)
		return SipFail;
	INIT( (*s)->pBuffer );
	(*s)->dLength = 0;
	HSS_INITREF((*s)->dRefCount);

	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipMsgBody
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipMsgBody
#ifdef ANSI_PROTO
	(SipMsgBody **s,en_SipMsgBodyType dType,SipError *err)
#else
	(s,dType, err)
	SipMsgBody **s;
	en_SipMsgBodyType dType;
	SipError *err;
#endif
{
	*s= (SipMsgBody *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipMsgBody),err);
	if (*s==SIP_NULL)
		return SipFail;
	(*s)->dType=dType;
	switch ( dType )
	{
		case SipSdpBody :
			INIT( (*s)->u.pSdpMessage );
			break;
		case SipUnknownBody :
			INIT( (*s)->u.pUnknownMessage);
			break;
		/***  bcpt ext ***/
		case SipIsupBody :
			INIT( (*s)->u.pIsupMessage);
			break;
		case SipQsigBody :
			INIT( (*s)->u.pQsigMessage);
			break;
		case SipMultipartMimeBody :
			INIT( (*s)->u.pMimeMessage);
			break;
		/***  bcpt ext ends ***/
		case SipAppSipBody:
			INIT((*s)->u.pAppSipMessage);
			break;
		case SipBodyAny :
			INIT( (*s)->u.pMimeMessage);
			break;
#ifdef SIP_MWI
		case SipMessageSummaryBody :
			INIT( (*s)->u.pSummaryMessage);
			break;
#endif
		default :
			*err = E_INV_TYPE;
			return SipFail;
	}

	INIT( (*s)->pMimeHeader);  /***  bcpt ext ends ***/
	*err = E_NO_ERROR;
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipMessage
#ifdef ANSI_PROTO
	(SipMessage **s,en_SipMessageType dType,SipError *err)
#else
	(s,dType, err)
	SipMessage **s;
	en_SipMessageType dType;
	SipError *err;
#endif
{
	*s= (SipMessage *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipMessage),err);
	if (*s==SIP_NULL)
		return SipFail;
	(*s)->dType=dType;
	sip_initSipGeneralHeader(&(*s)->pGeneralHdr,err);
	if ((*s)->dType==SipMessageRequest)
		sip_initSipReqMessage(&(*s)->u.pRequest,err);
	else if((*s)->dType==SipMessageResponse)
		sip_initSipRespMessage(&(*s)->u.pResponse,err);

	if ( sip_listInit( &((*s)->slOrderInfo), __sip_freeSipHeaderOrderInfo,\
		 err) == SipFail)
		return SipFail;

	if ( sip_listInit( &((*s)->slMessageBody), __sip_freeSipMsgBody, err) ==\
		 SipFail)
		return SipFail;
	(*s)->dIncorrectHdrsCount=0;
	(*s)->dEntityErrorCount=0;
	HSS_INITREF((*s)->dRefCount);

	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipHeader
#ifdef ANSI_PROTO
	(SipHeader **h,en_HeaderType dType,SipError *err)
#else
	(h,dType,err)
	SipHeader **h;
	en_HeaderType dType;
	SipError *err;
#endif
{
	if ((err == SIP_NULL) || ( h == SIP_NULL))
	{
		*err = E_INV_PARAM;
		return SipFail;
	}
	*h= (SipHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipHeader),err);
	if (*h==SIP_NULL)
	{
		*err = E_NO_MEM;
		return SipFail;
	}
	(*h)->dType=dType;/**got to validate**/
	/* Now init  pHeader - we need to do this as the lower dLevel pHeader APIs are never invoked directly
	by the pUser. If it were invoked, the yyySetxxxxx fns would automatically allocate space
	*/
	switch(dType)
	{
			case SipHdrTypeAny:
					break;
			case SipHdrTypeAccept:
					if(sip_initSipAcceptHeader( (SipAcceptHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAcceptEncoding:
					if(sip_initSipAcceptEncodingHeader (( \
						SipAcceptEncodingHeader **) &(*h)->pHeader, err)== \
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAcceptLanguage:
					if(sip_initSipAcceptLangHeader ((SipAcceptLangHeader **) & \
						(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeCallId:
					if(sip_initSipCallIdHeader ((SipCallIdHeader **) &(*h)-> \
						pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeReplyTo:
					if(sip_initSipReplyToHeader ((SipReplyToHeader **) &(*h)-> \
								pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeReplaces:
					if(sip_initSipReplacesHeader ((SipReplacesHeader **) &(*h)-> \
								pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeCseq:
					if(sip_initSipCseqHeader ((SipCseqHeader **) &(*h)-> \
						pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeDate:
					if(sip_initSipDateHeader ((SipDateHeader **) &(*h)-> \
						pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeEncryption:
					if(sip_initSipEncryptionHeader ((SipEncryptionHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeExpiresAny:
					if(sip_initSipExpiresHeader ((SipExpiresHeader **) \
						&(*h)->pHeader, SipExpAny, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeExpiresDate:
					if(sip_initSipExpiresHeader ((SipExpiresHeader **) \
						&(*h)->pHeader, SipExpDate, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeExpiresSec:
					if(sip_initSipExpiresHeader ((SipExpiresHeader **) \
						&(*h)->pHeader, SipExpSeconds, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeFrom:
					if(sip_initSipFromHeader ((SipFromHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRecordRoute:
					if(sip_initSipRecordRouteHeader ((SipRecordRouteHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#ifdef SIP_3GPP
			case SipHdrTypeServiceRoute:
					if(sip_initSipServiceRouteHeader ((SipServiceRouteHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePath:
					if(sip_initSipPathHeader ((SipPathHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePanInfo:
					if(sip_initSipPanInfoHeader ((SipPanInfoHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
		   case SipHdrTypePcVector:
					if(sip_initSipPcVectorHeader ((SipPcVectorHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
 
#endif
			case SipHdrTypeTimestamp:
					if(sip_initSipTimeStampHeader ((SipTimeStampHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeTo:
					if(sip_initSipToHeader ((SipToHeader **) &(*h)->pHeader, \
						err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeVia:
					if(sip_initSipViaHeader ((SipViaHeader **) &(*h)->pHeader, \
						err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContentEncoding:
					if(sip_initSipContentEncodingHeader (( \
						SipContentEncodingHeader **) &(*h)->pHeader, err)== \
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContentLength:
					if(sip_initSipContentLengthHeader (( \
						SipContentLengthHeader **) &(*h)->pHeader, err)== \
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContentType:
					if(sip_initSipContentTypeHeader ((SipContentTypeHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAlertInfo:
					if(sip_initSipAlertInfoHeader ((SipAlertInfoHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeReferTo:
					if(sip_initSipReferToHeader ((SipReferToHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeReferredBy:
					if(sip_initSipReferredByHeader ((SipReferredByHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeInReplyTo:
					if(sip_initSipInReplyToHeader ((SipInReplyToHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAlso:
					if(sip_initSipAlsoHeader ((SipAlsoHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeCallInfo:
					if(sip_initSipCallInfoHeader ((SipCallInfoHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeErrorInfo:
					if(sip_initSipErrorInfoHeader ((SipErrorInfoHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeMinExpires:
                    if(sip_initSipMinExpiresHeader ((SipMinExpiresHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
                    break;
			case SipHdrTypeContentDisposition:
					if(sip_initSipContentDispositionHeader (( \
						SipContentDispositionHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeReason:
					if(sip_initSipReasonHeader (( \
						SipReasonHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContentLanguage:
					if(sip_initSipContentLanguageHeader ((\
						SipContentLanguageHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
		/* bcpt ext */
			case SipHdrTypeMimeVersion:
					if(sip_bcpt_initSipMimeVersionHeader ((\
						SipMimeVersionHeader **) &(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
		/* bcpt ext ends */

		/* Retrans ends */
			case SipHdrTypeRAck:
					if(sip_rpr_initSipRAckHeader((SipRackHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRSeq:
					if(sip_rpr_initSipRSeqHeader((SipRseqHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeSupported:
					if(sip_initSipSupportedHeader((SipSupportedHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
		/* Retrans ends */
			case SipHdrTypeAuthorization:
					if(sip_initSipAuthorizationHeader ((\
						SipAuthorizationHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContactNormal:
					if(sip_initSipContactHeader ((SipContactHeader **) \
						&(*h)->pHeader, SipContactNormal, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;

			case SipHdrTypeContactWildCard:
					if(sip_initSipContactHeader ((SipContactHeader **) \
						&(*h)->pHeader, SipContactWildCard, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeContactAny:
					if(sip_initSipContactHeader ((SipContactHeader **) \
						&(*h)->pHeader, SipContactAny, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeHide:
					if(sip_initSipHideHeader ((SipHideHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeMaxforwards:
					if(sip_initSipMaxForwardsHeader ((SipMaxForwardsHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeOrganization:
					if(sip_initSipOrganizationHeader (( \
						SipOrganizationHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePriority:
					if(sip_initSipPriorityHeader ((SipPriorityHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeProxyauthorization:
					if(sip_initSipProxyAuthorizationHeader ((\
						SipProxyAuthorizationHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeProxyRequire:
					if(sip_initSipProxyRequireHeader ((\
						SipProxyRequireHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRoute:
					if(sip_initSipRouteHeader ((SipRouteHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRequire:
					if(sip_initSipRequireHeader ((SipRequireHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeResponseKey:
					if(sip_initSipRespKeyHeader ((SipRespKeyHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeSubject:
					if(sip_initSipSubjectHeader ((SipSubjectHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeUserAgent:
					if(sip_initSipUserAgentHeader ((SipUserAgentHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAllow:
					if(sip_initSipAllowHeader ((SipAllowHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeProxyAuthenticate:
					if(sip_initSipProxyAuthenticateHeader ((\
						SipProxyAuthenticateHeader **) &(*h)->pHeader, err)
									==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAuthenticationInfo:
					if(sip_initSipAuthenticationInfoHeader ((\
						SipAuthenticationInfoHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRetryAfterDate:
					if(sip_initSipRetryAfterHeader ((SipRetryAfterHeader **) \
						&(*h)->pHeader, SipExpDate, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRetryAfterAny:
					if(sip_initSipRetryAfterHeader ((SipRetryAfterHeader **) \
						&(*h)->pHeader, SipExpAny, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeRetryAfterSec:
					if(sip_initSipRetryAfterHeader ((SipRetryAfterHeader **) \
						&(*h)->pHeader, SipExpSeconds, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#ifdef SIP_IMPP
			case SipHdrTypeEvent:
					if(sip_impp_initSipEventHeader ((SipEventHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAllowEvents:
					if(sip_impp_initSipAllowEventsHeader ((\
						SipAllowEventsHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeSubscriptionState:
					if(sip_impp_initSipSubscriptionStateHeader ((\
						SipSubscriptionStateHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#endif
			case SipHdrTypeServer:
					if(sip_initSipServerHeader((SipServerHeader **)\
						&((*h)->pHeader),err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeUnsupported:
					if(sip_initSipUnsupportedHeader((SipUnsupportedHeader **)\
						&((*h)->pHeader),err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeWarning:
					if(sip_initSipWarningHeader ((SipWarningHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeWwwAuthenticate:
					if(sip_initSipWwwAuthenticateHeader ((\
						SipWwwAuthenticateHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#ifdef SIP_CCP
			case SipHdrTypeAcceptContact:
					if(sip_ccp_initSipAcceptContactHeader((\
						SipAcceptContactHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break; /* CCP case */
			case SipHdrTypeRejectContact:
					if(sip_ccp_initSipRejectContactHeader((\
						SipRejectContactHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break; /* CCP case */
			case SipHdrTypeRequestDisposition:
					if(sip_ccp_initSipRequestDispositionHeader((\
						SipRequestDispositionHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break; /* CCP case */
#endif /* ccp*/

#ifdef SIP_SESSIONTIMER
			case SipHdrTypeMinSE:
                    if(sip_initSipMinSEHeader ((SipMinSEHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
                    break;
			case SipHdrTypeSessionExpires:
					if(sip_initSipSessionExpiresHeader ((\
						SipSessionExpiresHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#endif
#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId :
					if(sip_initSipPAssertIdHeader ((\
						SipPAssertIdHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePPreferredId :
					if(sip_initSipPPreferredIdHeader ((\
						SipPPreferredIdHeader **) &(*h)->pHeader, err)==\
						SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePrivacy:
 					if(sip_initSipPrivacyHeader((SipPrivacyHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#endif /*  ifdef SIP_PRIVACY */
#ifdef SIP_CONGEST
			case SipHdrTypeRsrcPriority:
					if(sip_initSipRsrcPriorityHeader( (SipRsrcPriorityHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypeAcceptRsrcPriority:
					if(sip_initSipAcceptRsrcPriorityHeader( (SipAcceptRsrcPriorityHeader **) \
						&(*h)->pHeader,err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
                    
#endif /*  ifdef SIP_CONGEST */
#ifdef SIP_CONF			
            case SipHdrTypeJoin:
					if(sip_initSipJoinHeader ((SipJoinHeader **) &(*h)-> \
								pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
#endif /*  ifdef SIP_CONF */

#ifdef SIP_3GPP
			case SipHdrTypePAssociatedUri:
					if(sip_initSipPAssociatedUriHeader ((SipPAssociatedUriHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePCalledPartyId:
					if(sip_initSipPCalledPartyIdHeader ((SipPCalledPartyIdHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePVisitedNetworkId:
					if(sip_initSipPVisitedNetworkIdHeader ((SipPVisitedNetworkIdHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			case SipHdrTypePcfAddr:
					if(sip_initSipPcfAddrHeader ((SipPcfAddrHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;

#endif
#ifdef SIP_SECURITY
			case SipHdrTypeSecurityClient:
    					if(sip_initSipSecurityClientHeader ((SipSecurityClientHeader **) &(*h)->pHeader, \
              					err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
        					return SipFail;
					}
					break;
        		case SipHdrTypeSecurityServer:
                                        if(sip_initSipSecurityServerHeader ((SipSecurityServerHeader **) &(*h)->pHeader, \
                                                err)==SipFail)
                                        {
                                                fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
                                                return SipFail;
                                        }
                                        break;

			case SipHdrTypeSecurityVerify:
                                        if(sip_initSipSecurityVerifyHeader ((SipSecurityVerifyHeader **) &(*h)->pHeader, \
                                                err)==SipFail)
                                        {
                                                fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
                                                return SipFail;
                                        }
                                        break;
#endif


			case SipHdrTypeUnknown:
					if(sip_initSipUnknownHeader ((SipUnknownHeader **) \
						&(*h)->pHeader, err)==SipFail)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
					break;
			default 	:
#ifdef SIP_DCS
					if ((sip_dcs_initHeader(h, dType, err)) == SipSuccess)
						break;
					if (*err != E_INV_TYPE)
					{
						fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
						return SipFail;
					}
#endif
					*err = E_INV_TYPE;
					fast_memfree(NON_SPECIFIC_MEM_ID,*h,SIP_NULL);
					return SipFail;
	} /* of switch */
	*err = E_NO_ERROR;
	return SipSuccess;
 /** initSipHeader **/
}


/***********************************************************************************
**** FUNCTION:sip_getVersion
****
****
**** DESCRIPTION: This returns a string containing the version id. The string is a 
**** global string and statically allocated.
***********************************************************************************/

SIP_S8bit *sip_getVersion()
{
	return hss_sip_version_id;
}


/***********************************************************************************
**** FUNCTION:sip_getPart
****
****
**** DESCRIPTION:
***********************************************************************************/

SIP_S8bit * sip_getPart()
{
	return hss_sip_part_id;
}

/**********************************************************************************
**** FUNCTION:sip_initSipSupportedHeader
****
****
**** DESCRIPTION:
***********************************************************************************/


SipBool sip_initSipSupportedHeader
#ifdef ANSI_PROTO
	(SipSupportedHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipSupportedHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipSupportedHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipSupportedHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	INIT ( (*ppHdr)->pOption);
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAlertInfoHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipAlertInfoHeader
#ifdef ANSI_PROTO
	(SipAlertInfoHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipAlertInfoHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipAlertInfoHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipAlertInfoHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pUri);
	sip_listInit(& ((*ppHdr)->slParam),__sip_freeSipParam,pErr);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipInReplyToHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipInReplyToHeader
#ifdef ANSI_PROTO
	(SipInReplyToHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipInReplyToHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipInReplyToHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipInReplyToHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pCallId);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAlsoHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipAlsoHeader
#ifdef ANSI_PROTO
	(SipAlsoHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipAlsoHeader **ppHdr;
	SipError	*pErr;
#endif
{
	*ppHdr = (SipAlsoHeader*)fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAlsoHeader), pErr);
	if(*ppHdr == SIP_NULL)
		return SipFail;
	INIT((*ppHdr)->pDispName);
	INIT((*ppHdr)->pAddrSpec);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipCallInfoHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipCallInfoHeader
#ifdef ANSI_PROTO
	(SipCallInfoHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipCallInfoHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipCallInfoHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipCallInfoHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pUri);
	sip_listInit(& ((*ppHdr)->slParam),__sip_freeSipParam,pErr);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipErrorInfoHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipErrorInfoHeader
#ifdef ANSI_PROTO
	(SipErrorInfoHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipErrorInfoHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipErrorInfoHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipErrorInfoHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pUri);
	sip_listInit(& ((*ppHdr)->slParam),__sip_freeSipParam,pErr);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipMinExpiresHeader
****
****
**** DESCRIPTION: Initialize the MinSE Header and it's members
**************************************************************************************************************************************/
SipBool sip_initSipMinExpiresHeader
#ifdef ANSI_PROTO
        (SipMinExpiresHeader **ppHdr, SipError *pErr)
#else
        (ppHdr,pErr)
        SipMinExpiresHeader **ppHdr;
        SipError *pErr;
#endif
{
        *ppHdr = (SipMinExpiresHeader*) fast_memget(NON_SPECIFIC_MEM_ID,\
				sizeof(SipMinExpiresHeader),pErr);
        if ( *ppHdr == SIP_NULL )
                return SipFail;
        HSS_INITREF((*ppHdr)->dRefCount);
        return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContentDispositionHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipContentDispositionHeader
#ifdef ANSI_PROTO
	(SipContentDispositionHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipContentDispositionHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipContentDispositionHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipContentDispositionHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pDispType);
	sip_listInit(& ((*ppHdr)->slParam),__sip_freeSipParam,pErr);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/******************************************************************************
**** FUNCTION:sip_initSipReasonHeader
****
****
**** DESCRIPTION: Initializes the SipReasonHeader structure
******************************************************************************/


SipBool sip_initSipReasonHeader
#ifdef ANSI_PROTO
	(SipReasonHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipReasonHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipReasonHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipReasonHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pDispType);
	sip_listInit(& ((*ppHdr)->slParam),__sip_freeSipParam,pErr);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


/**********************************************************************************************************************************
**** FUNCTION:sip_initSipContentLanguageHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/


SipBool sip_initSipContentLanguageHeader
#ifdef ANSI_PROTO
	(SipContentLanguageHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipContentLanguageHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipContentLanguageHeader *)fast_memget (NON_SPECIFIC_MEM_ID, sizeof(SipContentLanguageHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;

	INIT((*ppHdr)->pLangTag);

	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

/* Function to init the reentrant structure*/

/*SipBool sip_initSipParserStruct
#ifdef ANSI_PROTO
	(SipHeaderParserParam **s,en_SipMessageType dType,SipError *err)
#else
	(s,dType, err)
	SipHeaderParserParam **s;
	en_SipMessageType dType;
	SipError *err;
#endif
{
	*((*s)->pError)=E_NO_ERROR;
	sip_initSipMessage(&((*s)->pSipMessage),dType,err);
	if ((*err)!=E_NO_ERROR)
		return SipFail;
	sip_listInit(((*s)->pGCList),__sip_freeSipParam,err);
	if (*err!=E_NO_ERROR)
		return SipFail;
	return SipSuccess;
}*/

/***************************************************************************************************************
*******************
**** FUNCTION:sip_initSipNameValuePair
****
****
**** DESCRIPTION:
****************************************************************************************************************
**********************/

SipBool sip_initSipNameValuePair
#ifdef ANSI_PROTO
        ( SipNameValuePair **c,SipError *err)
#else
        (c,err)
        SipNameValuePair **c;
        SipError *err;
#endif
{
        *c= ( SipNameValuePair *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof( SipNameValuePair),err);
        if (*c==SIP_NULL)
                return SipFail;
        INIT((*c)->pName);
        INIT((*c)->pValue);
        HSS_INITREF((*c)->dRefCount);
        return SipSuccess;
}

#ifdef SIP_MWI
/***************************************************************************************************************
*******************
**** FUNCTION:sip_mwi_initSummaryLine
****
****
**** DESCRIPTION:
****************************************************************************************************************
**********************/

SipBool sip_mwi_initSummaryLine
#ifdef ANSI_PROTO
        ( SummaryLine **c,SipError *err)
#else
        (c,err)
        SummaryLine **c;
        SipError *err;
#endif
{
        *c= ( SummaryLine *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof( SummaryLine),err);
        if (*c==SIP_NULL)
                return SipFail;
        INIT((*c)->pMedia);
        HSS_INITREF((*c)->dRefCount);
		(*c)->newMessages=0;
		(*c)->oldMessages=0;
		(*c)->newUrgentMessages=0;
		(*c)->oldUrgentMessages=0;
        return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_mwi_initMesgSummaryMessage
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/
SipBool sip_mwi_initMesgSummaryMessage
   #ifdef ANSI_PROTO
         (MesgSummaryMessage  **ppParam, SipError *err)
   #else
           (ppParam,err)
           MesgSummaryMessage  **ppParam;
           SipError *err;
   #endif
{
	*ppParam = (MesgSummaryMessage *)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(MesgSummaryMessage),err);
        if ( *ppParam == SIP_NULL )
                   return SipFail;
 	sip_listInit(& ((*ppParam)->slSummaryLine ),__sip_mwi_freeSummaryLine,err);
 	sip_listInit(& ((*ppParam)->slNameValue ),__sip_freeSipNameValuePair,err);
	/* (*ppParam)->pAddrSpec = SIP_NULL ; */
	INIT((*ppParam)->pAddrSpec);
  HSS_INITREF((*ppParam)->dRefCount);
  return SipSuccess;
}

#endif
#ifdef SIP_SESSIONTIMER
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipMinSEHeader
****
****
**** DESCRIPTION: Initialize the MinSE Header and it's members
**************************************************************************************************************************************/
SipBool sip_initSipMinSEHeader
#ifdef ANSI_PROTO
        (SipMinSEHeader **ppHdr, SipError *pErr)
#else
        (ppHdr,pErr)
        SipMinSEHeader **ppHdr;
        SipError *pErr;
#endif
{
        *ppHdr = (SipMinSEHeader*) fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipMinSEHeader),pErr);
        if ( *ppHdr == SIP_NULL )
                return SipFail;
		(*ppHdr)->dSec = 0;	/*	Member initialized for Zero Seconds	*/		                
		sip_listInit(& ((*ppHdr)->slNameValue),__sip_freeSipNameValuePair,pErr);
        HSS_INITREF((*ppHdr)->dRefCount);
        return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipSessionExpiresHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipSessionExpiresHeader
#ifdef ANSI_PROTO
	(SipSessionExpiresHeader **e,SipError *err)
#else
	(e,dType,err)
	SipSessionExpiresHeader **e;
	en_ExpiresType dType;
	SipError *err;
#endif
{
	*e= (SipSessionExpiresHeader *) fast_memget(NON_SPECIFIC_MEM_ID, \
		sizeof(SipSessionExpiresHeader),err);
	if (*e==SIP_NULL)
		return SipFail;
	sip_listInit(& ((*e)->slNameValue),__sip_freeSipNameValuePair,err);
	HSS_INITREF((*e)->dRefCount);
	return SipSuccess;
}
#endif

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipBadHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipBadHeader
#ifdef ANSI_PROTO
	(SipBadHeader **e,en_HeaderType dType,SipError *err)
#else
	(e,dType,err)
	SipBadHeader **e;
	en_HeaderType dType;
	SipError *err;
#endif
{
	*e= (SipBadHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipBadHeader),err);
	if (*e==SIP_NULL)
		return SipFail;
	INIT((*e)->pName);
	INIT((*e)->pBody);
	 (*e)->dType=dType;
	 HSS_INITREF((*e)->dRefCount); /* Initializing the reference count to 1 on initialization */
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAuthenticationInfoHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/
SipBool sip_initSipAuthenticationInfoHeader
#ifdef ANSI_PROTO
	(SipAuthenticationInfoHeader **pHdr,SipError *pErr)
#else
	(pHdr,pErr)
	SipAuthenticationInfoHeader **pHdr;
	SipError *pErr;
#endif
{
	*pHdr= (SipAuthenticationInfoHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAuthenticationInfoHeader),pErr);
	if (*pHdr==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	sip_listInit(& ((*pHdr)->slNameValue),__sip_freeSipNameValuePair,pErr);
	HSS_INITREF((*pHdr)->dRefCount);
	return SipSuccess;
}

#ifdef SIP_PRIVACY
/**************************************************************************************
**** FUNCTION:sip_initSipPAssertIdHeader
****
****
**** DESCRIPTION: This function will initialise SipPAssertIdHeader structure.
****************************************************************************************/
SipBool sip_initSipPAssertIdHeader
#ifdef ANSI_PROTO
	(SipPAssertIdHeader **pHdr,SipError *pErr)
#else
	(pHdr,pErr)
	SipPAssertIdHeader **pHdr;
	SipError *pErr;
#endif
{
	*pHdr= (SipPAssertIdHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPAssertIdHeader),pErr);
	if (*pHdr==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	INIT((*pHdr)->pDispName) ;
	INIT((*pHdr)->pAddrSpec);
	HSS_INITREF((*pHdr)->dRefCount);
  return SipSuccess;
}

/**************************************************************************************
**** FUNCTION:sip_initSipPPreferredIdHeader
****
****
**** DESCRIPTION: This function will initialise SipPPreferredIdHeader structure.
****************************************************************************************/
SipBool sip_initSipPPreferredIdHeader
#ifdef ANSI_PROTO
	(SipPPreferredIdHeader **pHdr,SipError *pErr)
#else
	(pHdr,pErr)
	SipPPreferredIdHeader **pHdr;
	SipError *pErr;
#endif
{
	*pHdr= (SipPPreferredIdHeader *) fast_memget(NON_SPECIFIC_MEM_ID, 
					sizeof(SipPPreferredIdHeader),pErr);
	if (*pHdr==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	INIT((*pHdr)->pDispName) ;
	INIT((*pHdr)->pAddrSpec);
	HSS_INITREF((*pHdr)->dRefCount);
  return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPrivacyHeader
****
****
**** DESCRIPTION: This function will initialise the SipPrivacy Header structure
**************************************************************************************************************************************/
SipBool sip_initSipPrivacyHeader
#ifdef ANSI_PROTO
(SipPrivacyHeader **ppHdr,SipError *pErr)
#else
(ppHdr,pErr)
SipPrivacyHeader **ppHdr;
SipError *pErr;
#endif
{
	*ppHdr= (SipPrivacyHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPrivacyHeader),pErr);
	if (*ppHdr==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}

	sip_listInit(& ((*ppHdr)->slPrivacyValue),__sip_freeSipNameValuePair,pErr);
	HSS_INITREF((*ppHdr)->dRefCount);
	return SipSuccess;
}
#endif /* # ifdef SIP_PRIVACY */

#ifdef SIP_CONGEST
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipRsrcPriorityHeader 
****
****
**** DESCRIPTION: This function will initialise the SipRsrcPriority Header structure
**************************************************************************************************************************************/
SipBool sip_initSipRsrcPriorityHeader
#ifdef ANSI_PROTO
(SipRsrcPriorityHeader **ppHdr, SipError *pErr)

#else
(ppHdr, pErr)
SipRsrcPriorityHeader **ppHdr;
SipError *pErr;

#endif
{
	*ppHdr = ( SipRsrcPriorityHeader*) fast_memget(0, sizeof(SipRsrcPriorityHeader),pErr);
	if (*ppHdr==SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
	INIT((*ppHdr)->pNamespace);
	INIT((*ppHdr)->pPriority);
	HSS_INITREF((*ppHdr)->dRefCount);

    return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipAcceptRsrcPriorityHeader 
****
****
**** DESCRIPTION: This function will initialise the SipAcceptRsrcPriority Header structure
**************************************************************************************************************************************/
SipBool sip_initSipAcceptRsrcPriorityHeader
#ifdef ANSI_PROTO
(SipAcceptRsrcPriorityHeader **ppHdr, SipError *pErr)

#else
(ppHdr, pErr)
SipAcceptRsrcPriorityHeader **ppHdr;
SipError *pErr;

#endif
{

    SipBool dResult;

    dResult = sip_initSipRsrcPriorityHeader((SipRsrcPriorityHeader **)ppHdr, pErr);
    return dResult;

}
# endif /* #ifdef SIP_CONGEST*/

#ifdef SIP_CONF
/**********************************************************************************************************************************
**** FUNCTION:sip_initSipJoinHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_initSipJoinHeader
#ifdef ANSI_PROTO
	(SipJoinHeader **c,SipError *err)
#else
	(c,err)
	SipJoinHeader **c;
	SipError *err;
#endif
{
	*c= (SipJoinHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipJoinHeader),err);
	
    if (*c==SIP_NULL)
    {
		*err = E_NO_MEM;
		return SipFail;
    }
    
	INIT((*c)->pCallid);
	INIT((*c)->pFromTag);
	INIT((*c)->pToTag);
	sip_listInit(&((*c)->slParams),__sip_freeSipNameValuePair,err);
	HSS_INITREF((*c)->dRefCount);
	return SipSuccess;
}
#endif /*  ifdef SIP_CONF */

# ifdef SIP_3GPP

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPAssociatedUriHeader
****
****
**** DESCRIPTION: Inits the P-Associated-URI header structure
**************************************************************************************************************************************/

SipBool sip_initSipPAssociatedUriHeader
#ifdef ANSI_PROTO
	(SipPAssociatedUriHeader **r,SipError *err)
#else
	(r,err)
	SipPAssociatedUriHeader **r;
	SipError *err;
#endif
{
	*r= (SipPAssociatedUriHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPAssociatedUriHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pDispName);
	INIT((*r)->pAddrSpec);
	sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPCalledPartyIdHeader
****
****
**** DESCRIPTION: Inits the PCalledPartyId header structure
**************************************************************************************************************************************/

SipBool sip_initSipPCalledPartyIdHeader
#ifdef ANSI_PROTO
	(SipPCalledPartyIdHeader **r,SipError *err)
#else
	(r,err)
	SipPCalledPartyIdHeader **r;
	SipError *err;
#endif
{
    if( (sip_initSipPAssociatedUriHeader((SipPAssociatedUriHeader**)r, err)) == SipFail )
        return SipFail;

    return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPVisitedNetworkIdHeader
****
****
**** DESCRIPTION: Inits the PVisitedNetworkId header structure
**************************************************************************************************************************************/

SipBool sip_initSipPVisitedNetworkIdHeader
#ifdef ANSI_PROTO
	(SipPVisitedNetworkIdHeader **r,SipError *err)
#else
	(r,err)
	SipPVisitedNetworkIdHeader **r;
	SipError *err;
#endif
{
	*r= (SipPVisitedNetworkIdHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPVisitedNetworkIdHeader),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pVNetworkSpec);
	sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);

    return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_initSipPcfAddrHeader
****
****
**** DESCRIPTION: Inits the PcfAddr header structure
**************************************************************************************************************************************/

SipBool sip_initSipPcfAddrHeader
#ifdef ANSI_PROTO
	(SipPcfAddrHeader **r,SipError *err)
#else
	(r,err)
	SipPcfAddrHeader **r;
	SipError *err;
#endif
{
	*r= (SipPcfAddrHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipPcfAddrHeader),err);
	
    if (*r==SIP_NULL)
		return SipFail;
	
    sip_listInit(& ((*r)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*r)->dRefCount);

    return SipSuccess;
}


#endif
