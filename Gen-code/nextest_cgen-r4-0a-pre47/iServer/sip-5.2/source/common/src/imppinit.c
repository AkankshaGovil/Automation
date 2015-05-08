
 /******************************************************************************
 ** FUNCTION:
 **	 This file has all the source for init functions for Instant Messaging 
 **	 and Presence Related Headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppinit.c
 **
 ** DESCRIPTION:
 **	 
 **
 ** DATE		NAME				REFERENCE	REASON
 ** ----		----				--------	------
 ** 16/4/2001	Subhash Nayak U.	Original
 **
 **
 **	Copyright 2001, Hughes Software Systems, Ltd. 
 ******************************************************************************/

#include "imppinit.h"


/**********************************************************************************************************************************
**** FUNCTION:sip_impp_initSipEventHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_impp_initSipEventHeader
#ifdef ANSI_PROTO
	(SipEventHeader **e,SipError *err)
#else
	(e,err)
	SipEventHeader **e;
	SipError *err;
#endif
{
	*e= (SipEventHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipEventHeader),err);
	if (*e==SIP_NULL)
		return SipFail;
	INIT((*e)->pEventType);
	sip_listInit(&((*e)->slParams),__sip_freeSipParam,err);
	HSS_INITREF((*e)->dRefCount);
	return SipSuccess;
}

/**********************************************************************************************************************************
**** FUNCTION:sip_impp_initSipAllowEventsHeader
****
****
**** DESCRIPTION:
**************************************************************************************************************************************/

SipBool sip_impp_initSipAllowEventsHeader
#ifdef ANSI_PROTO
	(SipAllowEventsHeader **a,SipError *err)
#else
	(a,err)
	SipAllowEventsHeader **a;
	SipError *err;
#endif
{
	*a= (SipAllowEventsHeader *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(SipAllowEventsHeader),err);
	if (*a==SIP_NULL)
		return SipFail;
	INIT((*a)->pEventType);
	HSS_INITREF((*a)->dRefCount);
	return SipSuccess;
}

/*******************************************************************************
**** FUNCTION:sip_impp_initSipSubscriptionStateHeader
****
****
**** DESCRIPTION:
*******************************************************************************/

SipBool sip_impp_initSipSubscriptionStateHeader
#ifdef ANSI_PROTO
	(SipSubscriptionStateHeader **ppS,SipError *pErr)
#else
	(ppS,pErr)
	SipSubscriptionStateHeader **ppS;
	SipError *pErr;
#endif
{
	*ppS= (SipSubscriptionStateHeader *) fast_memget(NON_SPECIFIC_MEM_ID, \
		sizeof(SipSubscriptionStateHeader),pErr);
	if (*ppS==SIP_NULL)
		return SipFail;
	INIT((*ppS)->pSubState);
	sip_listInit(&((*ppS)->slParams),__sip_freeSipParam,pErr);
	HSS_INITREF((*ppS)->dRefCount);
	return SipSuccess;
}

/******************************************************************************
**** FUNCTION:	sip_initImUrl
****
****
**** DESCRIPTION:
*******************************************************************************/
SipBool sip_initImUrl
#ifdef ANSI_PROTO
	(ImUrl **ppIm,SipError *pErr)
#else
	(ppIm,pErr)
	ImUrl **ppIm;
	SipError *pErr;
#endif
{
	*ppIm = (ImUrl *) fast_memget(NON_SPECIFIC_MEM_ID, sizeof(ImUrl),pErr);
	if (*ppIm==SIP_NULL)
		return SipFail;
	INIT((*ppIm)->pDispName);
	INIT((*ppIm)->pUser);
	INIT((*ppIm)->pHost);
	sip_listInit(& ((*ppIm)->slRoute),__sip_freeString, pErr);
	sip_listInit(& ((*ppIm)->slParams),__sip_freeSipParam,pErr);
	HSS_INITREF((*ppIm)->dRefCount);
	return SipSuccess;
}

