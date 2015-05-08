/* The sip_init Procedures added for following structures 
*	SipOptions
*	SipTrace
*	SipTranspAddr
*	SipRequestDispositionHeader
**************************************************************/
#include "sipstruct.h"
#include "portlayer.h"
#include "ccpstruct.h"
#include "sipinit.h"
#include "sipfree.h"

SipBool sip_initSipOptions
#ifdef ANSI_PROTO
	(SipOptions **ppOptions, SipError *pErr)
#else
	(ppOptions, pErr)
	SipOptions **ppOptions;
	SipError	*pErr;
#endif
{
	*ppOptions = (SipOptions *)fast_memget(0, sizeof(SipOptions), pErr);
	if (*ppOptions == SIP_NULL)
		return SipFail;
	(*ppOptions)->dOption = 0;
	*pErr = E_NO_ERROR;
	return SipSuccess;
}

SipBool sip_initSipTrace
#ifdef ANSI_PROTO
	(SipTrace **ppTrace, SipError *pErr)
#else
	(ppTrace, pErr)
	SipTrace **ppTrace;
	SipError	*pErr;
#endif
{
	*ppTrace = (SipTrace *)fast_memget(0, sizeof(SipTrace), pErr);
	if (*ppTrace == SIP_NULL)
		return SipFail;
	/*No pointers so no init necessary */
return SipSuccess;
}


SipBool sip_initSipTranspAddr
#ifdef ANSI_PROTO
	(SipTranspAddr **ppAddr, SipError *pErr)
#else
	(ppAddr, pErr)
	SipTranspAddr **ppAddr;
	SipError	*pErr;
#endif
{
	*ppAddr = (SipTranspAddr *)fast_memget(0, sizeof(SipTranspAddr), pErr);
	if (*ppAddr == SIP_NULL)
		return SipFail;
	
	INIT((*ppAddr)->pHost);
	return SipSuccess;
}

/* Modified sip_initSipMessage suitable for the marshal functions */
SipBool sip_msgif_initSipMessage
#ifdef ANSI_PROTO
	(SipMessage **s,en_SipMessageType dType,SipError *err)
#else
	(s,dType, err)
	SipMessage **s;
	en_SipMessageType dType;
	SipError *err;
#endif
{
	*s= (SipMessage *) fast_memget(0, sizeof(SipMessage),err);
	if (*s==SIP_NULL)
		return SipFail;
	(*s)->dType=dType;
	INIT((*s)->pGeneralHdr);
	if ((*s)->dType==SipMessageRequest)
		INIT((*s)->u.pRequest);
	else if((*s)->dType==SipMessageResponse)
		INIT((*s)->u.pResponse);
	
	if ( sip_listInit( &((*s)->slOrderInfo), __sip_freeSipHeaderOrderInfo,\
		err) == SipFail)
		return SipFail;

	if ( sip_listInit( &((*s)->slMessageBody), __sip_freeSipMsgBody, err)\
		 == SipFail)
		return SipFail;
	HSS_INITREF((*s)->dRefCount);
	return SipSuccess;
}

SipBool sip_msgif_initSipReqMessage
#ifdef ANSI_PROTO
	(SipReqMessage **r,SipError *err)
#else
	(r,err)
	SipReqMessage **r;
	SipError *err;
#endif
{
	*r= (SipReqMessage *) fast_memget(0, sizeof(SipReqMessage),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pRequestLine);
	INIT((*r)->pRequestHdr);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

SipBool sip_msgif_initSipRespMessage
#ifdef ANSI_PROTO
	(SipRespMessage **r,SipError *err)
#else
	(r,err)
	SipRespMessage **r;
	SipError *err;
#endif
{
	*r= (SipRespMessage *) fast_memget(0, sizeof(SipRespMessage),err);
	if (*r==SIP_NULL)
		return SipFail;
	INIT((*r)->pStatusLine);
	INIT((*r)->pResponseHdr);
	HSS_INITREF((*r)->dRefCount);
	return SipSuccess;
}

