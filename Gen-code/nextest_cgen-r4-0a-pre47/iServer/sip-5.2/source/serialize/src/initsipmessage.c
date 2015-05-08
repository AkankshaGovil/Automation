#include <stdio.h>
#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"
#include "portlayer.h"
#include "sipinit.h"
#include "byteordering.h"

SipBool sip_init_deserializeSipMessage
#ifdef ANSI_PROTO
(SipMessage **s,en_SipMessageType dType,SipError *err)
#else
(s,dType,err)
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
		INIT((*s)->u.pRequest);
	
	if ( sip_listInit( &((*s)->slOrderInfo), __sip_freeSipHeaderOrderInfo,\
		 err) == SipFail)
		return SipFail;

	if ( sip_listInit( &((*s)->slMessageBody), __sip_freeSipMsgBody, err) ==\
		 SipFail)
		return SipFail;
	(*s)->dIncorrectHdrsCount=0;
	HSS_INITREF((*s)->dRefCount);

	return SipSuccess;
	
		



	}



SipBool sip_init_deserializeSipReqMessage
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

SipBool sip_init_deserializeSipRespMessage
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





