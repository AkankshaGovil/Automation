
 /******************************************************************************
 ** FUNCTION:
 **	 This file has the source for the free functions for the
 **	 Instant Messaging and Presence related headers
 **
 ******************************************************************************
 **
 ** FILENAME:
 ** 		imppfree.c
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

#include "imppfree.h"

/*****************************************************************
** FUNCTION:sip_impp_freeSipEventHeader
**
** DESCRIPTION:
*******************************************************************/
void sip_impp_freeSipEventHeader
#ifdef ANSI_PROTO
	(SipEventHeader *e)
#else
	(e)
	SipEventHeader *e;
#endif
{
	SipError err;

	if (e==SIP_NULL) return;
	
	HSS_LOCKREF(e->dRefCount);HSS_DECREF(e->dRefCount);
	if(HSS_CHECKREF(e->dRefCount))
	{ 
		HSS_FREE(e->pEventType);
		sip_listDeleteAll(&(e->slParams),&err);
		HSS_UNLOCKREF(e->dRefCount);
		HSS_DELETEREF(e->dRefCount);
		HSS_FREE(e);
	}
	else
	{
		HSS_UNLOCKREF(e->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_impp_freeSipAllowEventsHeader
**
** DESCRIPTION:
*******************************************************************/

void sip_impp_freeSipAllowEventsHeader
#ifdef ANSI_PROTO
	(SipAllowEventsHeader *a)
#else
	(a)
	SipAllowEventsHeader *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{ 
		HSS_FREE(a->pEventType);
		HSS_UNLOCKREF(a->dRefCount);
		HSS_DELETEREF(a->dRefCount);
		HSS_FREE(a);
	}
	else
	{
		HSS_UNLOCKREF(a->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:__sip_impp_freeSipEventHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_impp_freeSipEventHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid e)
#else
	(e)
	SIP_Pvoid e;
#endif
{
	sip_impp_freeSipEventHeader((SipEventHeader*) e);
}

/*****************************************************************
** FUNCTION:__sip_impp_freeSipAllowEventsHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_impp_freeSipAllowEventsHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_impp_freeSipAllowEventsHeader((SipAllowEventsHeader*) a);
}

/*****************************************************************
** FUNCTION:sip_impp_freeSipSubscriptionStateHeader
**
** DESCRIPTION:
*******************************************************************/
void sip_impp_freeSipSubscriptionStateHeader
#ifdef ANSI_PROTO
	(SipSubscriptionStateHeader *pS)
#else
	(pS)
	SipSubscriptionStateHeader *pS;
#endif
{
	SipError err;

	if (pS==SIP_NULL) return;
	HSS_LOCKREF(pS->dRefCount);HSS_DECREF(pS->dRefCount);
	if(HSS_CHECKREF(pS->dRefCount))
	{ 
		HSS_FREE(pS->pSubState);
		if(sip_listDeleteAll(&(pS->slParams),&err)==SipFail)
			return;
		HSS_UNLOCKREF(pS->dRefCount);
		HSS_DELETEREF(pS->dRefCount);
		HSS_FREE(pS);
	}
	else
	{
		HSS_UNLOCKREF(pS->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:__sip_impp_freeSipSubscriptionStateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_impp_freeSipSubscriptionStateHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_impp_freeSipSubscriptionStateHeader((\
	SipSubscriptionStateHeader*) s);
}



/*****************************************************************
** FUNCTION:sip_freeImUrl
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeImUrl 
#ifdef ANSI_PROTO
	(ImUrl *pUrl)
#else
 	(pUrl)
	ImUrl *pUrl;
#endif
{
	SipError dErr;

	if (pUrl == SIP_NULL) 
		return;
	HSS_LOCKREF(pUrl->dRefCount);
	HSS_DECREF(pUrl->dRefCount);
	if(HSS_CHECKREF(pUrl->dRefCount))
	{

		HSS_FREE(pUrl->pDispName);
		HSS_FREE(pUrl->pUser);
		HSS_FREE(pUrl->pHost);
		if (pUrl->slRoute.size != 0)
			if(sip_listDeleteAll ( &(pUrl->slRoute), &dErr)==SipFail)
				return;
		if (pUrl->slParams.size != 0)
			if(sip_listDeleteAll ( &(pUrl->slParams), &dErr)==SipFail)
				return;
		HSS_UNLOCKREF(pUrl->dRefCount);
		HSS_DELETEREF(pUrl->dRefCount);
		HSS_FREE(pUrl);
	}
	else
	{
		HSS_UNLOCKREF(pUrl->dRefCount);
	}
}

