
/***********************************************************************
 ** FUNCTION:
 **             Has Free Functions For all Structures

 *********************************************************************
 **
 ** FILENAME:
 ** sipfree.c
 **
 ** DESCRIPTION:
 ** This file contains dCodeNum to free all structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 6/12/99   Arjun RC       		        Initial Creation
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"
#include "sipbcptfree.h"
#include "portlayer.h"
#ifdef SIP_CCP
#include "ccpstruct.h" /* CCP */
#include "ccpfree.h"
#endif

#ifdef SIP_IMPP
#include "imppstruct.h" /* CCP */
#include "imppfree.h"
#endif

#include "rprfree.h" /* RPR */

#ifdef SIP_DCS
#include "dcsfree.h"
#endif



/* These are the functions that a USER will call to free structures.
They are strongly typecasted
*/

/*****************************************************************
** FUNCTION:sip_freeVoid
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeVoid
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	HSS_FREE(s);
}

/*****************************************************************
** FUNCTION:sip_freeString
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeString
#ifdef ANSI_PROTO
	(SIP_S8bit *s)
#else
	(s)
	SIP_S8bit *s;
#endif
{
	HSS_FREE(s);
}

/*****************************************************************
** FUNCTION:sip_freeSipTimerKey
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipTimerKey
#ifdef ANSI_PROTO
	(SipTimerKey *k)
#else
	(k)
	SipTimerKey *k;
#endif
{
	if (k==SIP_NULL) return;
	HSS_LOCKREF(k->dRefCount);HSS_DECREF(k->dRefCount);
	if(HSS_CHECKREF(k->dRefCount))
	{
		HSS_FREE(k->dCallid);
		HSS_FREE(k->pMethod);
		HSS_FREE(k->pViaBranch);
		if(k->pFromHdr != SIP_NULL)
			sip_freeSipFromHeader(k->pFromHdr);
		if(k->pToHdr != SIP_NULL)
			sip_freeSipToHeader(k->pToHdr);
		if(k->pRackHdr != SIP_NULL)
			sip_rpr_freeSipRAckHeader(k->pRackHdr);
		HSS_UNLOCKREF(k->dRefCount);
		HSS_DELETEREF(k->dRefCount);
		HSS_FREE(k);
	}
	else
	{
		HSS_UNLOCKREF(k->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipTimerBuffer
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipTimerBuffer
#ifdef ANSI_PROTO
	(SipTimerBuffer *b)
#else
	(b)
	SipTimerBuffer *b;
#endif
{
	if (b==SIP_NULL) return;
	HSS_LOCKREF(b->dRefCount);HSS_DECREF(b->dRefCount);
	if(HSS_CHECKREF(b->dRefCount))
	{
      	if(b->pContext != SIP_NULL)
       	{
         	if( (b->pContext->dOptions.dOption & SIP_OPT_DIRECTBUFFER)\
				!= SIP_OPT_DIRECTBUFFER)
         	 {
               	  if(b->pBuffer != SIP_NULL)
                   	  HSS_FREE(b->pBuffer);
        	}
		}
        else
        {
        	   if(b->pBuffer != SIP_NULL)
                  HSS_FREE(b->pBuffer);
        }

		if(b->pContext != SIP_NULL)
			sip_freeEventContext(b->pContext);
		HSS_UNLOCKREF(b->dRefCount);
		HSS_DELETEREF(b->dRefCount);
		HSS_FREE(b);
	}
	else
	{
		HSS_UNLOCKREF(b->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSdpOrigin
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpOrigin
#ifdef ANSI_PROTO
	(SdpOrigin *s)
#else
	(s)
	SdpOrigin *s;
#endif
{
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		HSS_FREE(s->pUser);
        HSS_FREE(s->pSessionid);
		HSS_FREE(s->pVersion);
		HSS_FREE(s->pNetType);
		HSS_FREE(s->pAddrType);
		HSS_FREE(s->dAddr);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSdpMedia
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpMedia
#ifdef ANSI_PROTO
	(SdpMedia *m)
#else
	(m)
	SdpMedia *m;
#endif
{
	SipError err;
	if (m==SIP_NULL) return;
	HSS_LOCKREF(m->dRefCount);HSS_DECREF(m->dRefCount);
	if(HSS_CHECKREF(m->dRefCount))
	{
		HSS_FREE(m->pInformation);
		sip_listDeleteAll (&(m->slConnection), &err);
		sip_listDeleteAll ( &(m->slBandwidth), &err);
		HSS_FREE(m->pKey);
		HSS_FREE(m->pFormat);
		HSS_FREE(m->pPortNum);
		HSS_FREE(m->pProtocol);

#ifdef SIP_ATM
        HSS_FREE(m->pVirtualCID);
        sip_listDeleteAll ( &(m->slProtofmt), &err);
#endif

		sip_listDeleteAll ( &(m->slAttr), &err);
		HSS_FREE (m->pMediaValue);
		sip_listDeleteAll ( &(m->slIncorrectLines), &err);
		HSS_UNLOCKREF(m->dRefCount);
		HSS_DELETEREF(m->dRefCount);
		HSS_FREE(m);
	}
	else
	{
		HSS_UNLOCKREF(m->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipNameValuePair
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipNameValuePair
#ifdef ANSI_PROTO
        (SipNameValuePair *n)
#else
        (n)
        SipNameValuePair *n;
#endif
{
        if (n==SIP_NULL) return;
        HSS_LOCKREF(n->dRefCount);HSS_DECREF(n->dRefCount);
        if(HSS_CHECKREF(n->dRefCount))
        {
                HSS_FREE(n->pName);
                HSS_FREE(n->pValue);
                HSS_UNLOCKREF(n->dRefCount);
                HSS_DELETEREF(n->dRefCount);
                HSS_FREE(n);
        }
        else
        {
                HSS_UNLOCKREF(n->dRefCount);
        }
}
#ifdef SIP_MWI
/*****************************************************************
** FUNCTION:sip_mwi_freeSummaryLine
**
**
** DESCRIPTION:
*******************************************************************/
void sip_mwi_freeSummaryLine
#ifdef ANSI_PROTO
        (SummaryLine *s)
#else
        (s)
        SummaryLine *s;
#endif
{
        if (s==SIP_NULL) return;
        HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
        if(HSS_CHECKREF(s->dRefCount))
        {
                HSS_FREE(s->pMedia);
                HSS_UNLOCKREF(s->dRefCount);
                HSS_DELETEREF(s->dRefCount);
                HSS_FREE(s);
        }
        else
        {
                HSS_UNLOCKREF(s->dRefCount);
        }
}


#endif
/*****************************************************************
** FUNCTION:sip_freeSdpAttr
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpAttr
#ifdef ANSI_PROTO
	(SdpAttr *a)
#else
	(a)
	SdpAttr *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		HSS_FREE(a->pName);
		HSS_FREE(a->pValue);
		HSS_UNLOCKREF(a->dRefCount);
		HSS_DELETEREF(a->dRefCount);
		HSS_FREE (a);
	}
	else
	{
		HSS_UNLOCKREF(a->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSdpTime
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpTime
#ifdef ANSI_PROTO
	(SdpTime *t)
#else
	(t)
	SdpTime *t;
#endif
{
	SipError err;
	if (t==SIP_NULL) return;
	HSS_LOCKREF(t->dRefCount);HSS_DECREF(t->dRefCount);
	if(HSS_CHECKREF(t->dRefCount))
	{
		HSS_FREE(t->pStart);
		HSS_FREE(t->pStop);
		sip_listDeleteAll ( &(t->slRepeat), &err);
		HSS_FREE(t->pZone);
		HSS_UNLOCKREF(t->dRefCount);
		HSS_DELETEREF(t->dRefCount);
		HSS_FREE (t);
	}
	else
	{
		HSS_UNLOCKREF(t->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSdpConnection
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpConnection
#ifdef ANSI_PROTO
	( SdpConnection *c)
#else
	(c)
 	SdpConnection *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pNetType);
		HSS_FREE(c->pAddrType);
		HSS_FREE(c->dAddr);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}


#ifdef SIP_MWI
/*****************************************************************
** FUNCTION:sip_mwi_freeMesgSummaryMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_mwi_freeMesgSummaryMessage
#ifdef ANSI_PROTO
	(MesgSummaryMessage *m)
#else
	(m)
	MesgSummaryMessage *m;
#endif
{
 	SipError err;
        if (m==SIP_NULL) return;
        HSS_LOCKREF(m->dRefCount);HSS_DECREF(m->dRefCount);
        if(HSS_CHECKREF(m->dRefCount))
        {
                sip_listDeleteAll (&(m->slSummaryLine), &err);
                sip_listDeleteAll (&(m->slNameValue), &err);
                sip_freeSipAddrSpec(m->pAddrSpec) ;
                HSS_UNLOCKREF(m->dRefCount);
                HSS_DELETEREF(m->dRefCount);
                HSS_FREE(m);
        }
        else
        {
                HSS_UNLOCKREF(m->dRefCount);
        }
}

#endif

/*****************************************************************
** FUNCTION:sip_freeSdpMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSdpMessage
#ifdef ANSI_PROTO
	(SdpMessage *m)
#else
	(m)
	SdpMessage *m;
#endif
{
	SipError err;
	if (m==SIP_NULL) return;
	HSS_LOCKREF(m->dRefCount);HSS_DECREF(m->dRefCount);
	if(HSS_CHECKREF(m->dRefCount))
	{
		HSS_FREE(m->pVersion);
		sip_freeSdpOrigin(m->pOrigin);
		HSS_FREE(m->pSession);
		HSS_FREE(m->pInformation);
		HSS_FREE(m->pUri);
		sip_listDeleteAll ( &(m->slEmail), &err);
		sip_listDeleteAll ( &(m->slPhone), &err);
		sip_freeSdpConnection (m->slConnection);
		sip_listDeleteAll ( &(m->pBandwidth), &err);
		sip_listDeleteAll ( &(m->slTime), &err);
		HSS_FREE(m->pKey);
		sip_listDeleteAll ( &(m->slAttr), &err);
		sip_listDeleteAll ( &(m->slMedia), &err);
		sip_listDeleteAll ( &(m->slIncorrectLines), &err);
		HSS_UNLOCKREF(m->dRefCount);
		HSS_DELETEREF(m->dRefCount);
		HSS_FREE(m);
	}
	else
	{
		HSS_UNLOCKREF(m->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipParam
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipParam
#ifdef ANSI_PROTO
	(SipParam *p)
#else
	(p)
	SipParam *p;
#endif
{
	SipError err;
	if (p==SIP_NULL) return;
	HSS_LOCKREF(p->dRefCount);HSS_DECREF(p->dRefCount);
	if(HSS_CHECKREF(p->dRefCount))
	{
		HSS_FREE(p->pName);
		sip_listDeleteAll ( &(p->slValue), &err);
		HSS_UNLOCKREF(p->dRefCount);
		HSS_DELETEREF(p->dRefCount);
		HSS_FREE(p);
	}
	else
	{
		HSS_UNLOCKREF(p->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipGenericChallenge
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipGenericChallenge
#ifdef ANSI_PROTO
	(SipGenericChallenge *c)
#else
	(c)
	SipGenericChallenge *c;
#endif
{
	SipError err;
	if (c==SIP_NULL) return;

	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pScheme);
		sip_listDeleteAll ( &(c->slParam), &err);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipWwwAuthenticateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipWwwAuthenticateHeader
#ifdef ANSI_PROTO
	(SipWwwAuthenticateHeader *h)
#else
	(h)
	SipWwwAuthenticateHeader *h;
#endif
{
	if (h==SIP_NULL) return;

	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount);
	if(HSS_CHECKREF(h->dRefCount))
	{
		__sip_freeSipGenericChallenge(h->pChallenge);
		HSS_UNLOCKREF(h->dRefCount);
		HSS_DELETEREF(h->dRefCount);
		HSS_FREE(h);
	}
	else
	{
		HSS_UNLOCKREF(h->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipProxyAuthenticateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipProxyAuthenticateHeader
#ifdef ANSI_PROTO
	(SipProxyAuthenticateHeader *p)
#else
	(p)
	SipProxyAuthenticateHeader *p;
#endif
{
	if (p==SIP_NULL) return;
	HSS_LOCKREF(p->dRefCount);HSS_DECREF(p->dRefCount);
	if(HSS_CHECKREF(p->dRefCount))
	{
		__sip_freeSipGenericChallenge(p->pChallenge);
		HSS_UNLOCKREF(p->dRefCount);
		HSS_DELETEREF(p->dRefCount);
		HSS_FREE(p);
	}
	else
	{
		HSS_UNLOCKREF(p->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipWarningHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipWarningHeader
#ifdef ANSI_PROTO
	(SipWarningHeader *w)
#else
	(w)
	SipWarningHeader *w;
#endif
{
	if (w==SIP_NULL) return;
	HSS_LOCKREF(w->dRefCount);HSS_DECREF(w->dRefCount);
	if(HSS_CHECKREF(w->dRefCount))
	{
		HSS_FREE(w->pAgent);
		HSS_FREE(w->pText);
		HSS_UNLOCKREF(w->dRefCount);
		HSS_DELETEREF(w->dRefCount);
		HSS_FREE(w);
	}
	else
	{
		HSS_UNLOCKREF(w->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipDateFormat
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipDateFormat
#ifdef ANSI_PROTO
	(SipDateFormat *d)
#else
	(d)
	SipDateFormat *d;
#endif
{
	if (d==SIP_NULL) return;
	HSS_LOCKREF(d->dRefCount);HSS_DECREF(d->dRefCount);
	if(HSS_CHECKREF(d->dRefCount))
	{
		HSS_UNLOCKREF(d->dRefCount);
		HSS_DELETEREF(d->dRefCount);
		HSS_FREE(d);
	}
	else
	{
		HSS_UNLOCKREF(d->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipTimeFormat
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipTimeFormat
#ifdef ANSI_PROTO
	(SipTimeFormat *t)
#else
	(t)
	SipTimeFormat *t;
#endif
{
	if (t==SIP_NULL) return;
	HSS_LOCKREF(t->dRefCount);HSS_DECREF(t->dRefCount);
	if(HSS_CHECKREF(t->dRefCount))
	{
		HSS_UNLOCKREF(t->dRefCount);
		HSS_DELETEREF(t->dRefCount);
		HSS_FREE(t);
	}
	else
	{
		HSS_UNLOCKREF(t->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipDateStruct
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipDateStruct
#ifdef ANSI_PROTO
	(SipDateStruct *d)
#else
	(d)
	SipDateStruct *d;
#endif
{
	if (d==SIP_NULL) return;
	HSS_LOCKREF(d->dRefCount);HSS_DECREF(d->dRefCount);
	if(HSS_CHECKREF(d->dRefCount))
	{
		__sip_freeSipDateFormat(d->pDate);
		__sip_freeSipTimeFormat(d->pTime);
		HSS_UNLOCKREF(d->dRefCount);
		HSS_DELETEREF(d->dRefCount);
		HSS_FREE(d);
	}
	else
	{
		HSS_UNLOCKREF(d->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipDateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipDateHeader
#ifdef ANSI_PROTO
	(SipDateHeader *d)
#else
	(d)
	SipDateHeader *d;
#endif
{
	if (d==SIP_NULL) return;
	HSS_LOCKREF(d->dRefCount);HSS_DECREF(d->dRefCount);
	if(HSS_CHECKREF(d->dRefCount))
	{
		__sip_freeSipDateFormat(d->pDate);
		__sip_freeSipTimeFormat(d->pTime);
		HSS_UNLOCKREF(d->dRefCount);
		HSS_DELETEREF(d->dRefCount);
		HSS_FREE(d);
	}
	else
	{
		HSS_UNLOCKREF(d->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipAllowHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAllowHeader
#ifdef ANSI_PROTO
	(SipAllowHeader *a)
#else
	(a)
	SipAllowHeader *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		HSS_FREE(a->pMethod);
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
** FUNCTION:sip_freeSipRetryAfterHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRetryAfterHeader
#ifdef ANSI_PROTO
	(SipRetryAfterHeader *r)
#else
	(r)
	SipRetryAfterHeader *r;
#endif
{
	SipError err;

	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		if(r->dType == SipExpDate)
			__sip_freeSipDateStruct(r->u.pDate);
		HSS_FREE(r->pComment);
		/* HSS_FREE(r->pDuration); */
		sip_listDeleteAll( &(r->slParams), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipGenericCredential
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipGenericCredential
#ifdef ANSI_PROTO
	(SipGenericCredential *c)
#else
	(c)
	SipGenericCredential *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		if (c->dType==SipCredAuth)
		{
			__sip_freeSipGenericChallenge(c->u.pChallenge);
		}
		else
		if (c->dType==SipCredBasic)
		{
			HSS_FREE(c->u.pBasic);
		}
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipAuthorizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAuthorizationHeader
#ifdef ANSI_PROTO
	(SipAuthorizationHeader *a)
#else
	(a)
	SipAuthorizationHeader *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		__sip_freeSipGenericCredential(a->pCredential);
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
** FUNCTION:sip_freeSipRespHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRespHeader
#ifdef ANSI_PROTO
	(SipRespHeader *h)
#else
	(h)
	SipRespHeader *h;
#endif
{
	SipError err;
	if (h==SIP_NULL) return;
	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount);
	if(HSS_CHECKREF(h->dRefCount))
	{
		sip_listDeleteAll( &(h->slProxyAuthenticateHdr), &err);
		sip_listDeleteAll( &(h->slUnsupportedHdr), &err);
		sip_listDeleteAll( &(h->slWarningHeader), &err);
		sip_listDeleteAll( &(h->slWwwAuthenticateHdr), &err);
		sip_listDeleteAll( &(h->slErrorInfoHdr), &err);
		sip_listDeleteAll( &(h->slAuthorizationHdr), &err);
		sip_listDeleteAll( &(h->slAuthenticationInfoHdr), &err);
		sip_rpr_freeSipRSeqHeader(h->pRSeqHdr); /* Retrans */
		sip_freeSipServerHeader(h->pServerHdr);
		sip_freeSipMinExpiresHeader(h->pMinExpiresHdr);
#ifdef SIP_DCS
		sip_listDeleteAll( &(h->slSessionHdr), &err);
#endif
#ifdef SIP_3GPP
		sip_listDeleteAll( &(h->slPAssociatedUriHdr), &err);
#endif
#ifdef SIP_CONGEST
		sip_listDeleteAll( &(h->slAcceptRsrcPriorityHdr), &err);
#endif      
#ifdef SIP_SECURITY
		sip_listDeleteAll( &(h->slSecurityServerHdr), &err);
#endif  
		HSS_UNLOCKREF(h->dRefCount);
		HSS_DELETEREF(h->dRefCount);
		HSS_FREE(h);
	}
	else
	{
		HSS_UNLOCKREF(h->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipRespKeyHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRespKeyHeader
#ifdef ANSI_PROTO
	( SipRespKeyHeader* r)
#else
	( r)
 	SipRespKeyHeader* r;
#endif
{
	SipError err;
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pKeyScheme);
		sip_listDeleteAll( &(r->slParam), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipUserAgentHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipUserAgentHeader
#ifdef ANSI_PROTO
	(SipUserAgentHeader *u)
#else
	(u)
	SipUserAgentHeader *u;
#endif
{
	if (u==SIP_NULL) return;
	HSS_LOCKREF(u->dRefCount);HSS_DECREF(u->dRefCount);
	if(HSS_CHECKREF(u->dRefCount))
	{
		HSS_FREE(u->pValue);
		HSS_UNLOCKREF(u->dRefCount);
		HSS_DELETEREF(u->dRefCount);
		HSS_FREE(u);
	}
	else
	{
		HSS_UNLOCKREF(u->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipSubjectHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipSubjectHeader
#ifdef ANSI_PROTO
	(SipSubjectHeader *s)
#else
	(s)
	SipSubjectHeader *s;
#endif
{
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		HSS_FREE(s->pSubject);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipProxyRequireHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipProxyRequireHeader
#ifdef ANSI_PROTO
	(SipProxyRequireHeader *p)
#else
	(p)
	SipProxyRequireHeader *p;
#endif
{
	if (p==SIP_NULL) return;
	HSS_LOCKREF(p->dRefCount);HSS_DECREF(p->dRefCount);
	if(HSS_CHECKREF(p->dRefCount))
	{
		HSS_FREE(p->pToken);
		HSS_UNLOCKREF(p->dRefCount);
		HSS_DELETEREF(p->dRefCount);
		HSS_FREE(p);
	}
	else
	{
		HSS_UNLOCKREF(p->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipProxyAuthorizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipProxyAuthorizationHeader
#ifdef ANSI_PROTO
	(SipProxyAuthorizationHeader *a)
#else
	(a)
	SipProxyAuthorizationHeader *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		__sip_freeSipGenericCredential(a->pCredentials);
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
** FUNCTION:sip_freeSipPriorityHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipPriorityHeader
#ifdef ANSI_PROTO
	(SipPriorityHeader *p)
#else
	(p)
	SipPriorityHeader *p;
#endif
{
	if (p==SIP_NULL) return;
	HSS_LOCKREF(p->dRefCount);HSS_DECREF(p->dRefCount);
	if(HSS_CHECKREF(p->dRefCount))
	{
		HSS_FREE(p->pPriority);
		HSS_UNLOCKREF(p->dRefCount);
		HSS_DELETEREF(p->dRefCount);
		HSS_FREE(p);
	}
	else
	{
		HSS_UNLOCKREF(p->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipOrganizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipOrganizationHeader
#ifdef ANSI_PROTO
	(SipOrganizationHeader *o)
#else
	(o)
	SipOrganizationHeader *o;
#endif
{
	if (o==SIP_NULL) return;
	HSS_LOCKREF(o->dRefCount);HSS_DECREF(o->dRefCount);
	if(HSS_CHECKREF(o->dRefCount))
	{
		HSS_FREE(o->pOrganization);
		HSS_UNLOCKREF(o->dRefCount);
		HSS_DELETEREF(o->dRefCount);
		HSS_FREE(o);
	}
	else
	{
		HSS_UNLOCKREF(o->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipContentTypeHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipContentTypeHeader
#ifdef ANSI_PROTO
	(SipContentTypeHeader *c)
#else
	(c)
	SipContentTypeHeader *c;
#endif
{
	SipError err;

	if (c==SIP_NULL) return;

	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pMediaType);
		sip_listDeleteAll(&(c->slParams),&err);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipContentEncodingHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipContentEncodingHeader
#ifdef ANSI_PROTO
	(SipContentEncodingHeader *c)
#else
	(c)
	SipContentEncodingHeader *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pEncoding);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipHideHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipHideHeader
#ifdef ANSI_PROTO
	(SipHideHeader *h)
#else
	(h)
	SipHideHeader *h;
#endif
{
	if (h==SIP_NULL) return;
	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount);
	if(HSS_CHECKREF(h->dRefCount))
	{
		HSS_FREE(h->pType);
		HSS_UNLOCKREF(h->dRefCount);
		HSS_DELETEREF(h->dRefCount);
		HSS_FREE(h);
	}
	else
	{
		HSS_UNLOCKREF(h->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipMaxForwardsHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipMaxForwardsHeader
#ifdef ANSI_PROTO
	(SipMaxForwardsHeader *h)
#else
	(h)
	SipMaxForwardsHeader *h;
#endif
{
	if (h==SIP_NULL) return;
	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount);
	if(HSS_CHECKREF(h->dRefCount))
	{
		HSS_UNLOCKREF(h->dRefCount);
		HSS_DELETEREF(h->dRefCount);
		HSS_FREE(h);
	}
	else
	{
		HSS_UNLOCKREF(h->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipViaParam
**
**
** DESCRIPTION:
*******************************************************************/

/* PARAMCHANGE
void sip_freeSipViaParam
#ifdef ANSI_PROTO
	(SipViaParam *v)
#else
	(v)
	SipViaParam *v;
#endif
{
	if (v==SIP_NULL) return;
	switch (v->dType)
	{
		case SipViaParamHidden:
			HSS_FREE(v->u.pHidden);
			break;
		case SipViaParamMaddr:
			HSS_FREE(v->u.pMaddr);
			break;
		case SipViaParamReceived:
			HSS_FREE(v->u.pViaReceived);
			break;
		case SipViaParamBranch:
			HSS_FREE(v->u.pViaBranch);
			break;
		case SipViaParamExtension:
			HSS_FREE(v->u.pExtParam);
			break;
		case SipViaParamTtl:
		case SipViaParamAny:
			break;
	}  switch
	HSS_FREE(v);
}
*/

/*****************************************************************
** FUNCTION:sip_freeSipContentLengthHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipContentLengthHeader
#ifdef ANSI_PROTO
	(SipContentLengthHeader *c)
#else
	(c)
	SipContentLengthHeader *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipViaHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipViaHeader
#ifdef ANSI_PROTO
	(SipViaHeader *v)
#else
	(v)
	SipViaHeader *v;
#endif
{
	SipError err;
	if (v==SIP_NULL) return;
	HSS_LOCKREF(v->dRefCount);HSS_DECREF(v->dRefCount);
	if(HSS_CHECKREF(v->dRefCount))
	{
		HSS_FREE(v->pSentProtocol);
		HSS_FREE(v->pSentBy);
		sip_listDeleteAll( &(v->slParam), &err);
		HSS_FREE(v->pComment);
		HSS_UNLOCKREF(v->dRefCount);
		HSS_DELETEREF(v->dRefCount);
		HSS_FREE(v);
	}
	else
	{
		HSS_UNLOCKREF(v->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipUrlParam
**
**
** DESCRIPTION:
*******************************************************************/

/* PARAMCHANGE
void sip_freeSipUrlParam
#ifdef ANSI_PROTO
	(SipUrlParam *u)
#else
	(u)
	SipUrlParam *u;
#endif
{
	if (u==SIP_NULL) return;
	switch(u->dType)
	{
		case SipUrlParamMethod:
			HSS_FREE(u->u.pMethod);
			break;
		case SipUrlParamMaddr:
			HSS_FREE(u->u.pMaddr);
			break;
		case SipUrlParamOther:
			HSS_FREE(u->u.pOtherParam);
			break;
		case SipUrlParamTransport:
		case SipUrlParamUser:
		case SipUrlParamTtl:
		case SipUrlParamAny:
			break;
	} switch
	HSS_FREE(u);
}
*/

/*****************************************************************
** FUNCTION:sip_freeSipUrl
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipUrl
#ifdef ANSI_PROTO
	(SipUrl *u)
#else
	(u)
	SipUrl *u;
#endif
{
	SipError err;
	if (u==SIP_NULL) return;
	HSS_LOCKREF(u->dRefCount);HSS_DECREF(u->dRefCount);
	if(HSS_CHECKREF(u->dRefCount))
	{
		HSS_FREE(u->pUser);
		HSS_FREE(u->pPassword);
		HSS_FREE(u->pHost);
		HSS_FREE(u->dPort);
		sip_listDeleteAll( &(u->slParam), &err);
		HSS_FREE(u->pHeader);
		HSS_UNLOCKREF(u->dRefCount);
		HSS_DELETEREF(u->dRefCount);
		HSS_FREE(u);
	}
	else
	{
		HSS_UNLOCKREF(u->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipAddrSpec
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAddrSpec
#ifdef ANSI_PROTO
	(SipAddrSpec *a)
#else
	(a)
	SipAddrSpec *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		if (a->dType == SipAddrReqUri)
			HSS_FREE(a->u.pUri);
		else
		if ((a->dType == SipAddrSipUri) || (a->dType == SipAddrSipSUri))
		{
			sip_freeSipUrl(a->u.pSipUrl);
		}
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
** FUNCTION:sip_freeSipToHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipToHeader
#ifdef ANSI_PROTO
	(SipToHeader *t)
#else
	(t)
	SipToHeader *t;
#endif
{
	SipError err;
	if (t==SIP_NULL) return;
	HSS_LOCKREF(t->dRefCount);HSS_DECREF(t->dRefCount);
	if(HSS_CHECKREF(t->dRefCount))
	{
		HSS_FREE(t->pDispName);
		sip_freeSipAddrSpec(t->pAddrSpec);
		sip_listDeleteAll( &(t->slTag), &err);
		sip_listDeleteAll( &(t->slParam), &err);
		HSS_UNLOCKREF(t->dRefCount);
		HSS_DELETEREF(t->dRefCount);
		HSS_FREE(t);
	}
	else
	{
		HSS_UNLOCKREF(t->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipTimeStampHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipTimeStampHeader
#ifdef ANSI_PROTO
	(SipTimeStampHeader *t)
#else
	(t)
	SipTimeStampHeader *t;
#endif
{
	if (t==SIP_NULL) return;
	HSS_LOCKREF(t->dRefCount);HSS_DECREF(t->dRefCount);
	if(HSS_CHECKREF(t->dRefCount))
	{
		HSS_FREE(t->pTime);
		HSS_FREE(t->delay);
		HSS_UNLOCKREF(t->dRefCount);
		HSS_DELETEREF(t->dRefCount);
		HSS_FREE(t);
	}
	else
	{
		HSS_UNLOCKREF(t->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipRouteHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRouteHeader
#ifdef ANSI_PROTO
	(SipRouteHeader *r)
#else
	(r)
	SipRouteHeader *r;
#endif
{
	SipError err;
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pDispName);
		sip_freeSipAddrSpec(r->pAddrSpec);
		sip_listDeleteAll( &(r->slParams), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipRecordRouteHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRecordRouteHeader
#ifdef ANSI_PROTO
	(SipRecordRouteHeader *r)
#else
	(r)
	SipRecordRouteHeader *r;
#endif
{
	SipError err;
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pDispName);
		sip_freeSipAddrSpec(r->pAddrSpec);
		sip_listDeleteAll( &(r->slParams), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}
#ifdef SIP_3GPP
/*****************************************************************
** FUNCTION:sip_freeSipPathHeader
**
**
** DESCRIPTION: 
*******************************************************************/

void sip_freeSipPathHeader
#ifdef ANSI_PROTO
	(SipPathHeader *pPath)
#else
	(pPath)
	SipPathHeader *pPath;
#endif
{
	SipError err;
	if (pPath==SIP_NULL) return;
	HSS_LOCKREF(pPath->dRefCount);
	HSS_DECREF(pPath->dRefCount);
	if(HSS_CHECKREF(pPath->dRefCount))
	{
		HSS_FREE(pPath->pDispName);
		sip_freeSipAddrSpec(pPath->pAddrSpec);
		sip_listDeleteAll( &(pPath->slParams), &err);
		HSS_UNLOCKREF(pPath->dRefCount);
		HSS_DELETEREF(pPath->dRefCount);
		HSS_FREE(pPath);
	}
	else
	{
		HSS_UNLOCKREF(pPath->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipServiceRouteHeader
**
**
** DESCRIPTION: Frees the service route header
*******************************************************************/

void sip_freeSipServiceRouteHeader
#ifdef ANSI_PROTO
	(SipServiceRouteHeader *pService)
#else
	(pService)
	SipServiceRouteHeader *pService;
#endif
{
	SipError dErr=E_NO_ERROR;
	if (pService==SIP_NULL) return;
	HSS_LOCKREF(pService->dRefCount);
	HSS_DECREF(pService->dRefCount);
	if(HSS_CHECKREF(pService->dRefCount))
	{
		HSS_FREE(pService->pDispName);
		sip_freeSipAddrSpec(pService->pAddrSpec);
		sip_listDeleteAll( &(pService->slParams), &dErr);
		HSS_UNLOCKREF(pService->dRefCount);
		HSS_DELETEREF(pService->dRefCount);
		HSS_FREE(pService);
	}
	else
	{
		HSS_UNLOCKREF(pService->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipPanInfoHeader
**
**
** DESCRIPTION: 
*******************************************************************/

void sip_freeSipPanInfoHeader
#ifdef ANSI_PROTO
	(SipPanInfoHeader *pHdr)
#else
	(pHdr)
	SipPanInfoHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr==SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pAccessType);
		sip_listDeleteAll( &(pHdr->slParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipPcVectorHeader
**
**
** DESCRIPTION: 
*******************************************************************/

void sip_freeSipPcVectorHeader
#ifdef ANSI_PROTO
	(SipPcVectorHeader *pHdr)
#else
	(pHdr)
	SipPcVectorHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr==SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pAccessType);
		HSS_FREE(pHdr->pAccessValue);
		sip_listDeleteAll( &(pHdr->slParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}
   
#endif
/*****************************************************************
** FUNCTION:sip_freeSipRequireHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRequireHeader
#ifdef ANSI_PROTO
	(SipRequireHeader *r)
#else
	(r)
	SipRequireHeader *r;
#endif
{
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pToken);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:sip_freeSipFromHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipFromHeader
#ifdef ANSI_PROTO
	(SipFromHeader *t)
#else
	(t)
	SipFromHeader *t;
#endif
{
	SipError err;
	if (t==SIP_NULL) return;
	HSS_LOCKREF(t->dRefCount);HSS_DECREF(t->dRefCount);
	if(HSS_CHECKREF(t->dRefCount))
	{
		HSS_FREE(t->pDispName);
		sip_freeSipAddrSpec(t->pAddrSpec);
		sip_listDeleteAll( &(t->slTag), &err);
		sip_listDeleteAll( &(t->slParam), &err);
		HSS_UNLOCKREF(t->dRefCount);
		HSS_DELETEREF(t->dRefCount);
		HSS_FREE(t);
	}
	else
	{
		HSS_UNLOCKREF(t->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipExpiresStruct
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipExpiresStruct
#ifdef ANSI_PROTO
	(SipExpiresStruct *e)
#else
	(e)
	SipExpiresStruct *e;
#endif
{
	if (e==SIP_NULL) return;
	HSS_LOCKREF(e->dRefCount);HSS_DECREF(e->dRefCount);
	if(HSS_CHECKREF(e->dRefCount))
	{
		if(e->dType == SipExpDate)
			__sip_freeSipDateStruct(e->u.pDate);
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
** FUNCTION:sip_freeSipExpiresHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipExpiresHeader
#ifdef ANSI_PROTO
	(SipExpiresHeader *e)
#else
	(e)
	SipExpiresHeader *e;
#endif
{
	if (e==SIP_NULL) return;
	HSS_LOCKREF(e->dRefCount);HSS_DECREF(e->dRefCount);
	if(HSS_CHECKREF(e->dRefCount))
	{
		if(e->dType == SipExpDate)
			__sip_freeSipDateStruct(e->u.pDate);
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
** FUNCTION:sip_freeSipEncryptionHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipEncryptionHeader
#ifdef ANSI_PROTO
	(SipEncryptionHeader *e)
#else
	(e)
	SipEncryptionHeader *e;
#endif
{
	SipError err;
	if (e==SIP_NULL) return;
	HSS_LOCKREF(e->dRefCount);HSS_DECREF(e->dRefCount);
	if(HSS_CHECKREF(e->dRefCount))
	{
		HSS_FREE(e->pScheme);
		sip_listDeleteAll( &(e->slParam), &err);
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
** FUNCTION:sip_freeSipContactParam
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipContactParam
#ifdef ANSI_PROTO
	(SipContactParam *c)
#else
	(c)
	SipContactParam *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		switch(c->dType)
		{
			case SipCParamQvalue:
				HSS_FREE(c->u.pQValue);
				break;
			case SipCParamExpires:
				sip_freeSipExpiresStruct(c->u.pExpire);
				break;
			case SipCParamExtension:
				HSS_FREE(c->u.pExtensionAttr);
				break;
			case SipCParamFeatureParam :
			  sip_freeSipParam(c->u.pParam) ;
				break;
			case SipCParamAny:
				break;
		} /*switch*/
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipContactHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipContactHeader
#ifdef ANSI_PROTO
	(SipContactHeader *c)
#else
	(c)
	SipContactHeader *c;
#endif
{
	SipError err;
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pDispName);
		sip_freeSipAddrSpec(c->pAddrSpec);
		sip_listDeleteAll( &(c->slContactParam), &err);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipCseqHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipCseqHeader
#ifdef ANSI_PROTO
	(SipCseqHeader *c)
#else
	(c)
	SipCseqHeader *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pMethod);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipCallIdHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipCallIdHeader
#ifdef ANSI_PROTO
	(SipCallIdHeader *c)
#else
	(c)
	SipCallIdHeader *c;
#endif
{
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pValue);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}
/*****************************************************************
** FUNCTION:sip_freeSipReplacesHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipReplacesHeader
#ifdef ANSI_PROTO
	(SipReplacesHeader *c)
#else
	(c)
	SipReplacesHeader *c;
#endif
{
	SipError dError;
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pCallid);
		HSS_FREE(c->pFromTag);
		HSS_FREE(c->pToTag);
		sip_listDeleteAll( &(c->slParams), &dError);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipReplyToHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipReplyToHeader
#ifdef ANSI_PROTO
	(SipReplyToHeader *c)
#else
	(c)
	SipReplyToHeader *c;
#endif
{
	SipError dError;
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pDispName);
		sip_freeSipAddrSpec(c->pAddrSpec);
		sip_listDeleteAll( &(c->slParams), &dError);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipAcceptHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAcceptHeader
#ifdef ANSI_PROTO
	(SipAcceptHeader *a)
#else
	(a)
	SipAcceptHeader *a;
#endif
{
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		HSS_FREE(a->pAcceptRange);
		HSS_FREE(a->pMediaRange);
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
** FUNCTION:sip_freeSipAcceptEncodingHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAcceptEncodingHeader
#ifdef ANSI_PROTO
	(SipAcceptEncodingHeader *a)
#else
	(a)
	SipAcceptEncodingHeader *a;
#endif
{
	SipError dError;
	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		HSS_FREE(a->pCoding);
		HSS_FREE(a->pQValue);
		sip_listDeleteAll( &(a->slParam), &dError);

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
** FUNCTION:sip_freeSipAcceptLangHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAcceptLangHeader
#ifdef ANSI_PROTO
	(SipAcceptLangHeader *a)
#else
	(a)
	SipAcceptLangHeader *a;
#endif
{
	SipError dError;

	if (a==SIP_NULL) return;
	HSS_LOCKREF(a->dRefCount);HSS_DECREF(a->dRefCount);
	if(HSS_CHECKREF(a->dRefCount))
	{
		HSS_FREE(a->pLangRange);
		HSS_FREE(a->pQValue);
		sip_listDeleteAll( &(a->slParam), &dError);
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
** FUNCTION:sip_freeSipUnsupportedHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipUnsupportedHeader
#ifdef ANSI_PROTO
	(SipUnsupportedHeader *u)
#else
	(u)
	SipUnsupportedHeader *u;
#endif
{
	if (u==SIP_NULL) return;
	HSS_LOCKREF(u->dRefCount);HSS_DECREF(u->dRefCount);
	if(HSS_CHECKREF(u->dRefCount))
	{
		HSS_FREE(u->pOption);
		HSS_UNLOCKREF(u->dRefCount);
		HSS_DELETEREF(u->dRefCount);
		HSS_FREE(u);
	}
	else
	{
		HSS_UNLOCKREF(u->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipServerHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipServerHeader
#ifdef ANSI_PROTO
	(SipServerHeader *s)
#else
	(s)
	SipServerHeader *s;
#endif
{
	if (s==SIP_NULL) return;
		HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		HSS_FREE(s->pValue);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipReqHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipReqHeader
#ifdef ANSI_PROTO
	(SipReqHeader *s)
#else
	(s)
	SipReqHeader *s;
#endif
{
	SipError err;
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		sip_listDeleteAll( &(s->slAuthorizationHdr), &err);
		sip_freeSipHideHeader(s->pHideHdr);
		sip_freeSipMaxForwardsHeader(s->pMaxForwardsHdr);
		sip_freeSipPriorityHeader(s->pPriorityHdr);
		sip_freeSipRespKeyHeader(s->pRespKeyHdr);
		sip_freeSipSubjectHeader(s->pSubjectHdr);
		sip_freeSipReferToHeader(s->pReferToHdr);
		sip_freeSipReplacesHeader(s->pReplacesHdr);
		sip_freeSipReferredByHeader(s->pReferredByHdr);
		sip_rpr_freeSipRAckHeader(s->pRackHdr);   /* Retrans */
		sip_listDeleteAll( &(s->slProxyAuthorizationHdr), &err);
		sip_listDeleteAll( &(s->slProxyRequireHdr), &err);
		sip_listDeleteAll( &(s->slRouteHdr), &err);
		sip_listDeleteAll( &(s->slWwwAuthenticateHdr), &err);
#ifdef SIP_IMPP
		sip_impp_freeSipEventHeader(s->pEventHdr);
		sip_impp_freeSipSubscriptionStateHeader( s->pSubscriptionStateHdr);
#endif
#ifdef SIP_CCP
		sip_listDeleteAll( &(s->slAcceptContactHdr), &err); /* CCP */
		sip_listDeleteAll( &(s->slRejectContactHdr), &err); /* CCP */
		sip_listDeleteAll( &(s->slRequestDispositionHdr), &err); /* CCP */
#endif
		sip_listDeleteAll( &(s->slAlertInfoHdr), &err);
		sip_listDeleteAll( &(s->slInReplyToHdr), &err);
		sip_listDeleteAll( &(s->slAlsoHdr), &err);
#ifdef SIP_DCS
		sip_dcs_freeSipDcsTracePartyIdHeader(s->pDcsTracePartyIdHdr);
		sip_dcs_freeSipDcsOspsHeader(s->pDcsOspsHdr);
		sip_dcs_freeSipDcsRedirectHeader(s->pDcsRedirectHdr);
#endif
#ifdef SIP_CONGEST
		sip_listDeleteAll( &(s->slRsrcPriorityHdr), &err);
#endif
#ifdef SIP_CONF        
		sip_freeSipJoinHeader(s->pJoinHdr);
#endif

#ifdef SIP_SECURITY
		sip_listDeleteAll( &(s->slSecurityClientHdr), &err);
		sip_listDeleteAll( &(s->slSecurityVerifyHdr), &err);
#endif
#ifdef SIP_3GPP
		sip_freeSipPCalledPartyIdHeader(s->pPCalledPartyIdHdr);
		sip_listDeleteAll( &(s->slPVisitedNetworkIdHdr), &err);
#endif

		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}
/*****************************************************************
** FUNCTION:sip_freeSipUnknownHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipUnknownHeader
#ifdef ANSI_PROTO
	(SipUnknownHeader *u)
#else
	(u)
	SipUnknownHeader *u;
#endif
{
	HSS_LOCKREF(u->dRefCount);HSS_DECREF(u->dRefCount);
	if(HSS_CHECKREF(u->dRefCount))
	{
		HSS_FREE(u->pName);
		HSS_FREE(u->pBody);
		HSS_UNLOCKREF(u->dRefCount);
		HSS_DELETEREF(u->dRefCount);
		HSS_FREE(u);
	}
	else
	{
		HSS_UNLOCKREF(u->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipBadHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipBadHeader
#ifdef ANSI_PROTO
	(SipBadHeader *u)
#else
	(u)
	SipBadHeader *u;
#endif
{
	HSS_LOCKREF(u->dRefCount);HSS_DECREF(u->dRefCount);
	if(HSS_CHECKREF(u->dRefCount))
	{
		HSS_FREE(u->pName);
		HSS_FREE(u->pBody);
		HSS_UNLOCKREF(u->dRefCount);
		HSS_DELETEREF(u->dRefCount);
		HSS_FREE(u);
	}
	else
	{
		HSS_UNLOCKREF(u->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipGeneralHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipGeneralHeader
#ifdef ANSI_PROTO
	(SipGeneralHeader *g)
#else
	(g)
	SipGeneralHeader *g;
#endif
{
	SipError err;
	if (g==SIP_NULL) return;
	HSS_LOCKREF(g->dRefCount);HSS_DECREF(g->dRefCount);
	if(HSS_CHECKREF(g->dRefCount))
	{
		sip_listDeleteAll( &(g->slAcceptHdr), &err);
		sip_listDeleteAll( &(g->slAllowHdr), &err);
		sip_listDeleteAll( &(g->slAcceptEncoding), &err);
		sip_listDeleteAll( &(g->slAcceptLang), &err);
		sip_listDeleteAll( &(g->slContactHdr), &err);
		sip_listDeleteAll( &(g->slRecordRouteHdr), &err);
#ifdef SIP_3GPP
		sip_listDeleteAll( &(g->slPathHdr), &err);
#endif
		sip_listDeleteAll( &(g->slViaHdr), &err);
		sip_listDeleteAll( &(g->slContentEncodingHdr), &err);
		sip_listDeleteAll( &(g->slSupportedHdr), &err);
		sip_listDeleteAll( &(g->slUnknownHdr), &err);
		sip_listDeleteAll( &(g->slBadHdr), &err);
		sip_listDeleteAll( &(g->slCallInfoHdr), &err);
		sip_listDeleteAll( &(g->slContentDispositionHdr), &err);
		sip_listDeleteAll( &(g->slReasonHdr), &err);
		sip_listDeleteAll( &(g->slRequireHdr), &err);
		sip_listDeleteAll( &(g->slContentLanguageHdr), &err);
#ifdef SIP_PRIVACY
		sip_listDeleteAll( &(g->slPAssertIdHdr), &err);
		sip_listDeleteAll( &(g->slPPreferredIdHdr), &err);
#endif /* # ifdef SIP_PRIVACY */
		sip_freeSipCallIdHeader(g->pCallidHdr);
		sip_freeSipReplyToHeader(g->pReplyToHdr);
		sip_freeSipCseqHeader(g->pCseqHdr);
		sip_freeSipDateHeader(g->pDateHdr);
		sip_freeSipEncryptionHeader(g->pEncryptionHdr);
		sip_freeSipExpiresHeader(g->pExpiresHdr);
		sip_freeSipFromHeader(g->pFromHeader);
		sip_freeSipTimeStampHeader(g->pTimeStampHdr);
		sip_freeSipToHeader(g->pToHdr);
		sip_freeSipContentLengthHeader(g->pContentLengthHdr);
		sip_freeSipContentTypeHeader(g->pContentTypeHdr);
		sip_bcpt_freeSipMimeVersionHeader(g->pMimeVersionHdr);  /* bcpt ext */
		sip_freeSipOrganizationHeader(g->pOrganizationHdr);
		sip_freeSipUserAgentHeader(g->pUserAgentHdr);
		sip_freeSipRetryAfterHeader(g->pRetryAfterHdr);
#ifdef SIP_PRIVACY
		sip_freeSipPrivacyHeader(g->pPrivacyHdr);
#endif
#ifdef SIP_SESSIONTIMER
        sip_freeSipMinSEHeader(g->pMinSEHdr);
		sip_freeSipSessionExpiresHeader(g->pSessionExpiresHdr);
#endif
#ifdef SIP_IMPP
		sip_listDeleteAll( &(g->slAllowEventsHdr), &err);
#endif
#ifdef SIP_DCS
		sip_listDeleteAll( &(g->slDcsStateHdr), &err);
		sip_listDeleteAll( &(g->slDcsAnonymityHdr), &err);
		sip_listDeleteAll( &(g->slDcsRpidPrivacyHdr), &err);
		sip_listDeleteAll( &(g->slDcsRemotePartyIdHdr), &err);
		sip_listDeleteAll( &(g->slDcsMediaAuthorizationHdr), &err);
		sip_dcs_freeSipDcsGateHeader(g->pDcsGateHdr);
		sip_dcs_freeSipDcsBillingIdHeader(g->pDcsBillingIdHdr);
		sip_dcs_freeSipDcsBillingInfoHeader(g->pDcsBillingInfoHdr);
		sip_dcs_freeSipDcsLaesHeader(g->pDcsLaesHdr);
#endif

#ifdef SIP_3GPP
        sip_freeSipPanInfoHeader(g->pPanInfoHdr);
        sip_freeSipPcVectorHeader(g->pPcVectorHdr);
        sip_freeSipPcfAddrHeader(g->pPcfAddrHdr);
				sip_listDeleteAll( &(g->slServiceRouteHdr), &err);
#endif
		HSS_UNLOCKREF(g->dRefCount);
		HSS_DELETEREF(g->dRefCount);
		HSS_FREE(g);
		}
	else
	{
		HSS_UNLOCKREF(g->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipReqLine
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipReqLine
#ifdef ANSI_PROTO
	(SipReqLine *r)
#else
	(r)
	SipReqLine *r;
#endif
{
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pMethod);
		sip_freeSipAddrSpec(r->pRequestUri);
		HSS_FREE(r->pVersion);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipReqMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipReqMessage
#ifdef ANSI_PROTO
	(SipReqMessage *r)
#else
	(r)
	SipReqMessage *r;
#endif
{
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		sip_freeSipReqLine(r->pRequestLine);
		sip_freeSipReqHeader(r->pRequestHdr);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipStatusLine
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipStatusLine
#ifdef ANSI_PROTO
	(SipStatusLine *s)
#else
	(s)
	SipStatusLine *s;
#endif
{
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		HSS_FREE(s->pVersion);
		HSS_FREE(s->pReason);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipRespMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipRespMessage
#ifdef ANSI_PROTO
	(SipRespMessage *r)
#else
	(r)
	SipRespMessage *r;
#endif
{
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		sip_freeSipStatusLine(r->pStatusLine);
		sip_freeSipRespHeader(r->pResponseHdr);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipUnknownMessage
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipUnknownMessage
#ifdef ANSI_PROTO
	(SipUnknownMessage *s)
#else
	(s)
	SipUnknownMessage *s;
#endif
{
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		if (s->pBuffer != SIP_NULL)
			HSS_FREE(s->pBuffer);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipMesgBody
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipMsgBody
#ifdef ANSI_PROTO
	(SipMsgBody	*s)
#else
	(s)
	SipMsgBody 	*s;
#endif
{
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		switch ( s->dType )
		{
			case SipSdpBody:
				sip_freeSdpMessage(s->u.pSdpMessage);
				break;
			case SipUnknownBody:
				sip_freeSipUnknownMessage(s->u.pUnknownMessage);
				break;
			case SipIsupBody :  /* bcpt ext */
				sip_bcpt_freeIsupMessage(s->u.pIsupMessage);
				break;
			case SipQsigBody :  /* bcpt ext */
				sip_bcpt_freeQsigMessage(s->u.pQsigMessage);
				break;
			case SipMultipartMimeBody : /* bcpt ext */
				sip_bcpt_freeMimeMessage(s->u.pMimeMessage);
				break;
#ifdef SIP_MWI
			case SipMessageSummaryBody:
				sip_mwi_freeMesgSummaryMessage(s->u.pSummaryMessage);
				break;
#endif
			case SipAppSipBody:
				sip_freeSipMessage(s->u.pAppSipMessage);
			default :
				break;
		}
		sip_bcpt_freeSipMimeHeader(s->pMimeHeader); /* bcpt ext */
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}
/*****************************************************************
** FUNCTION:sip_freeSipMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipMessage
#ifdef ANSI_PROTO
	(SipMessage *s)
#else
	(s)
	SipMessage *s;
#endif
{
	SipError err;

	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		sip_freeSipGeneralHeader(s->pGeneralHdr);
		if (s->dType==SipMessageRequest)
			sip_freeSipReqMessage(s->u.pRequest);
		else
		if (s->dType==SipMessageResponse)
		{
			sip_freeSipRespMessage(s->u.pResponse);
		}
		s->dIncorrectHdrsCount=0;
		sip_listDeleteAll( &(s->slOrderInfo), &err);
		sip_listDeleteAll( &(s->slMessageBody), &err);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_freeSipHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipHeader
#ifdef ANSI_PROTO
	(SipHeader *h)
#else
	(h)
	SipHeader *h;
#endif
{
	if (h==SIP_NULL) return;
		switch(h->dType)
		{
			case SipHdrTypeAccept:
				sip_freeSipAcceptHeader((SipAcceptHeader *)(h->pHeader));
				break;
			case SipHdrTypeAcceptEncoding:
				sip_freeSipAcceptEncodingHeader((SipAcceptEncodingHeader*)(h->pHeader));
				break;

			case SipHdrTypeReplyTo:
				 sip_freeSipReplyToHeader((SipReplyToHeader*)h->pHeader);
				 break;

			case SipHdrTypeReplaces:
				 sip_freeSipReplacesHeader((SipReplacesHeader*)h->pHeader);
				 break;

			case SipHdrTypeAcceptLanguage:
				sip_freeSipAcceptLangHeader((SipAcceptLangHeader *)(h->pHeader));
				break;

			case SipHdrTypeCallId:
				sip_freeSipCallIdHeader((SipCallIdHeader *)(h->pHeader));
				break;
			case SipHdrTypeCseq:
				sip_freeSipCseqHeader((SipCseqHeader *)(h->pHeader));
				break;
			case SipHdrTypeCallInfo:
				sip_freeSipCallInfoHeader((SipCallInfoHeader *)(h->pHeader));
				break;
			case SipHdrTypeErrorInfo:
				sip_freeSipErrorInfoHeader((SipErrorInfoHeader *)(h->pHeader));
				break;
            case SipHdrTypeMinExpires:
                 sip_freeSipMinExpiresHeader((SipMinExpiresHeader *)\
				 (h->pHeader));
                 break;
			case SipHdrTypeContentDisposition:
					sip_freeSipContentDispositionHeader((\
					SipContentDispositionHeader *)(h->pHeader));
				break;
			case SipHdrTypeReason:
				sip_freeSipReasonHeader((SipReasonHeader *)(h->pHeader));
				break;
			case SipHdrTypeDate:
				sip_freeSipDateHeader((SipDateHeader *)(h->pHeader));
				break;
			case SipHdrTypeEncryption:
				sip_freeSipEncryptionHeader((SipEncryptionHeader *)(h->pHeader));
				break;
			case SipHdrTypeExpiresSec:
			case SipHdrTypeExpiresDate:
			case SipHdrTypeExpiresAny:
				sip_freeSipExpiresHeader((SipExpiresHeader *)(h->pHeader));
				break;
			case SipHdrTypeFrom:
				sip_freeSipFromHeader((SipFromHeader *)(h->pHeader));
				break;
			case SipHdrTypeRecordRoute:
				sip_freeSipRecordRouteHeader((SipRecordRouteHeader *)(h->pHeader));
				break;
			case SipHdrTypeTimestamp:
				sip_freeSipTimeStampHeader((SipTimeStampHeader *)(h->pHeader));
				break;
			case SipHdrTypeTo:
				sip_freeSipToHeader((SipToHeader *)(h->pHeader));
				break;
			case SipHdrTypeVia:
				sip_freeSipViaHeader((SipViaHeader *)(h->pHeader));
				break;
			case SipHdrTypeContentEncoding:
				sip_freeSipContentEncodingHeader((SipContentEncodingHeader *)(h->pHeader));
				break;
			case SipHdrTypeContentLength:
				sip_freeSipContentLengthHeader((SipContentLengthHeader *)(h->pHeader));
				break;
			case SipHdrTypeContentType:
				sip_freeSipContentTypeHeader((SipContentTypeHeader *)(h->pHeader));
				break;
			/* bcpt ext */
			case SipHdrTypeMimeVersion:
				sip_bcpt_freeSipMimeVersionHeader((SipMimeVersionHeader *)(h->pHeader));
				break;
		/* bcpt ext ends */
		/* Retrans */
			case SipHdrTypeRSeq:
				sip_rpr_freeSipRSeqHeader((SipRseqHeader *)(h->pHeader));
				break;
			case SipHdrTypeRAck:
				sip_rpr_freeSipRAckHeader((SipRackHeader *)(h->pHeader));
				break;
			case SipHdrTypeSupported:
				sip_freeSipSupportedHeader((SipSupportedHeader *)(h->pHeader));
				break;
		/* Retrans */
			case SipHdrTypeAuthorization:
				sip_freeSipAuthorizationHeader((SipAuthorizationHeader *)(h->pHeader));
				break;
			case SipHdrTypeContactNormal:
			case SipHdrTypeContactWildCard:
			case SipHdrTypeContactAny:
				sip_freeSipContactHeader((SipContactHeader *)(h->pHeader));
				break;
			case SipHdrTypeHide:
				sip_freeSipHideHeader((SipHideHeader *)(h->pHeader));
				break;
			case SipHdrTypeMaxforwards:
				sip_freeSipMaxForwardsHeader((SipMaxForwardsHeader *)(h->pHeader));
				break;
			case SipHdrTypeOrganization:
				sip_freeSipOrganizationHeader((SipOrganizationHeader *)(h->pHeader));
				break;
			case SipHdrTypePriority:
				sip_freeSipPriorityHeader((SipPriorityHeader *)(h->pHeader));
				break;
			case SipHdrTypeProxyauthorization:
				sip_freeSipProxyAuthorizationHeader((SipProxyAuthorizationHeader *)(h->pHeader));
				break;
			case SipHdrTypeProxyRequire:
				sip_freeSipProxyRequireHeader((SipProxyRequireHeader *)(h->pHeader));
				break;
			case SipHdrTypeRoute:
				sip_freeSipRouteHeader((SipRouteHeader *)(h->pHeader));
				break;
			case SipHdrTypeRequire:
				sip_freeSipRequireHeader((SipRequireHeader *)(h->pHeader));
				break;
			case SipHdrTypeResponseKey:
				sip_freeSipRespKeyHeader((SipRespKeyHeader *)(h->pHeader));
				break;
			case SipHdrTypeSubject:
				sip_freeSipSubjectHeader((SipSubjectHeader *)(h->pHeader));
				break;
			case SipHdrTypeUserAgent:
				sip_freeSipUserAgentHeader((SipUserAgentHeader *)(h->pHeader));
				break;
			case SipHdrTypeAllow:
				sip_freeSipAllowHeader((SipAllowHeader *)(h->pHeader));
				break;
			case SipHdrTypeProxyAuthenticate:
				sip_freeSipProxyAuthenticateHeader((SipProxyAuthenticateHeader *)(h->pHeader));
				break;
			case SipHdrTypeAuthenticationInfo:
				sip_freeSipAuthenticationInfoHeader((SipAuthenticationInfoHeader *)(h->pHeader));
				break;
			case SipHdrTypeRetryAfterSec:
			case SipHdrTypeRetryAfterDate:
			case SipHdrTypeRetryAfterAny:
				sip_freeSipRetryAfterHeader((SipRetryAfterHeader *)(h->pHeader));
				break;
#ifdef SIP_IMPP
			case SipHdrTypeEvent:
				sip_impp_freeSipEventHeader((SipEventHeader *)(h->pHeader));
				break;
			case SipHdrTypeAllowEvents:
				sip_impp_freeSipAllowEventsHeader((SipAllowEventsHeader *)(h->pHeader));
				break;
			case SipHdrTypeSubscriptionState:
				sip_impp_freeSipSubscriptionStateHeader((SipSubscriptionStateHeader *)(h->pHeader));
				break;
#endif
			case SipHdrTypeServer:
				sip_freeSipServerHeader((SipServerHeader *) (h->pHeader));
				break;
			case SipHdrTypeUnsupported:
				sip_freeSipUnsupportedHeader( (SipUnsupportedHeader *) (h->pHeader));
				break;
			case SipHdrTypeUnknown:
				sip_freeSipUnknownHeader ((SipUnknownHeader *)(h->pHeader));
				break;
			case SipHdrTypeWarning:
				sip_freeSipWarningHeader((SipWarningHeader *)(h->pHeader));
				break;
			case SipHdrTypeWwwAuthenticate:
				sip_freeSipWwwAuthenticateHeader((SipWwwAuthenticateHeader *)(h->pHeader));
				break;
			case SipHdrTypeAlertInfo:
				sip_freeSipAlertInfoHeader((SipAlertInfoHeader *)(h->pHeader));
				break;
			case SipHdrTypeInReplyTo:
				sip_freeSipInReplyToHeader((SipInReplyToHeader *)(h->pHeader));
				break;
			case SipHdrTypeAlso:
				sip_freeSipAlsoHeader((SipAlsoHeader *)(h->pHeader));
				break;

			case SipHdrTypeReferTo:
				sip_freeSipReferToHeader((SipReferToHeader *)(h->pHeader));
				break;

			case SipHdrTypeReferredBy:
				sip_freeSipReferredByHeader((SipReferredByHeader *)(h->pHeader));
				break;

			case SipHdrTypeContentLanguage:
				sip_freeSipContentLanguageHeader((SipContentLanguageHeader *)(h->pHeader));
				break;
#ifdef SIP_CCP
			case SipHdrTypeAcceptContact:
				sip_ccp_freeSipAcceptContactHeader((SipAcceptContactHeader *)(h->pHeader));
				break; /* CCP */
			case SipHdrTypeRejectContact:
				sip_ccp_freeSipRejectContactHeader((SipRejectContactHeader *)(h->pHeader));
				break;/* CCP case */
			case SipHdrTypeRequestDisposition:
				sip_ccp_freeSipRequestDispositionHeader((SipRequestDispositionHeader *)(h->pHeader));
				break;/* CCP case */
#endif /* ccp*/


#ifdef SIP_DCS
				case SipHdrTypeDcsRemotePartyId:
					sip_dcs_freeSipDcsRemotePartyIdHeader \
						((SipDcsRemotePartyIdHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsRpidPrivacy:
					sip_dcs_freeSipDcsRpidPrivacyHeader \
						((SipDcsRpidPrivacyHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsTracePartyId:
					sip_dcs_freeSipDcsTracePartyIdHeader ((SipDcsTracePartyIdHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsAnonymity:
					sip_dcs_freeSipDcsAnonymityHeader ((SipDcsAnonymityHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsMediaAuthorization:
					sip_dcs_freeSipDcsMediaAuthorizationHeader ((SipDcsMediaAuthorizationHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsGate:
					sip_dcs_freeSipDcsGateHeader ((SipDcsGateHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsRedirect:
					sip_dcs_freeSipDcsRedirectHeader ((SipDcsRedirectHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsState:
					sip_dcs_freeSipDcsStateHeader ((SipDcsStateHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsLaes:
					sip_dcs_freeSipDcsLaesHeader ((SipDcsLaesHeader *)(h->pHeader));
					break;
				case SipHdrTypeSession:
				 	sip_dcs_freeSipSessionHeader ((SipSessionHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsOsps:
					sip_dcs_freeSipDcsOspsHeader ((SipDcsOspsHeader *)(h->pHeader));
					break;
				case SipHdrTypeDcsBillingId:
					sip_dcs_freeSipDcsBillingIdHeader ((SipDcsBillingIdHeader *)(h->pHeader));
					break;
			        case SipHdrTypeDcsBillingInfo:
					sip_dcs_freeSipDcsBillingInfoHeader ((SipDcsBillingInfoHeader *)(h->pHeader));
					break;
#endif
#ifdef SIP_SESSIONTIMER
              case SipHdrTypeMinSE:
                   sip_freeSipMinSEHeader((SipMinSEHeader *)(h->pHeader));
                    break;
			  case SipHdrTypeSessionExpires:
					sip_freeSipSessionExpiresHeader((\
						SipSessionExpiresHeader *)(h->pHeader));
					break;
#endif

#ifdef SIP_PRIVACY
			case SipHdrTypePAssertId:
				sip_freeSipPAssertIdHeader((SipPAssertIdHeader *)(h->pHeader)) ;
				break ;
			case SipHdrTypePPreferredId:
				sip_freeSipPPreferredIdHeader(
						(SipPPreferredIdHeader *)(h->pHeader)) ;
				break ;
			case SipHdrTypePrivacy:
				sip_freeSipPrivacyHeader((SipPrivacyHeader *)(h->pHeader));
				break;
#endif /* # ifdef SIP_PRIVACY */
#ifdef SIP_3GPP
			case SipHdrTypePath:
				sip_freeSipPathHeader((SipPathHeader *)(h->pHeader));
				break;
			case SipHdrTypePanInfo:
				sip_freeSipPanInfoHeader((SipPanInfoHeader *)(h->pHeader));
				break;
			case SipHdrTypePcVector:
				sip_freeSipPcVectorHeader((SipPcVectorHeader *)(h->pHeader));
				break;
			case SipHdrTypeServiceRoute:
				sip_freeSipServiceRouteHeader((SipServiceRouteHeader *)(h->pHeader));
				break;
	
#endif

#ifdef SIP_CONGEST
			case SipHdrTypeRsrcPriority:
				sip_freeSipRsrcPriorityHeader((SipRsrcPriorityHeader *)(h->pHeader));
				break;
   			case SipHdrTypeAcceptRsrcPriority:
				sip_freeSipAcceptRsrcPriorityHeader((SipAcceptRsrcPriorityHeader *)(h->pHeader));
				break;

#endif

#ifdef SIP_CONF
			case SipHdrTypeJoin:
				 sip_freeSipJoinHeader((SipJoinHeader*)h->pHeader);
				 break;
#endif
/* 3GPP headers */
#ifdef SIP_3GPP
			case SipHdrTypePAssociatedUri:
				sip_freeSipPAssociatedUriHeader((SipPAssociatedUriHeader *)(h->pHeader));
				break;
			case SipHdrTypePCalledPartyId:
				sip_freeSipPCalledPartyIdHeader((SipPCalledPartyIdHeader *)(h->pHeader));
				break;
			case SipHdrTypePVisitedNetworkId:
				sip_freeSipPVisitedNetworkIdHeader((SipPVisitedNetworkIdHeader *)(h->pHeader));
				break;
			case SipHdrTypePcfAddr:
				sip_freeSipPcfAddrHeader((SipPcfAddrHeader *)(h->pHeader));
				break;
#endif

#ifdef SIP_SECURITY			
			case SipHdrTypeSecurityClient:
				sip_freeSipSecurityClientHeader((SipSecurityClientHeader *)(h->pHeader));
				break;	
			case SipHdrTypeSecurityServer:
                                sip_freeSipSecurityServerHeader((SipSecurityServerHeader *)(h->pHeader));
                                break;
			case SipHdrTypeSecurityVerify:
                                sip_freeSipSecurityVerifyHeader((SipSecurityVerifyHeader *)(h->pHeader));
                                break;
#endif
	
			case SipHdrTypeAny:;


		} /* switch */
#ifndef SIP_BY_REFERENCE
		HSS_FREE(h);
#else
	h->dType=SipHdrTypeAny;
#endif

} /* freeSipHeader */

#ifdef SIP_SESSIONTIMER
/*****************************************************************
** FUNCTION:sip_freeSipSessionExpiresHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipSessionExpiresHeader
#ifdef ANSI_PROTO
	(SipSessionExpiresHeader *e)
#else
	(e)
	SipSessionExpiresHeader *e;
#endif
{
	SipError err;
	if (e==SIP_NULL) return;
	HSS_LOCKREF(e->dRefCount);HSS_DECREF(e->dRefCount);
	if(HSS_CHECKREF(e->dRefCount))
	{
		sip_listDeleteAll( &(e->slNameValue), &err);
		HSS_UNLOCKREF(e->dRefCount);
		HSS_DELETEREF(e->dRefCount);
		HSS_FREE(e);
	}
	else
	{
		HSS_UNLOCKREF(e->dRefCount);
	}
}

#endif




/* These are the functions that will be called INTERNALLY
to associate with sip lists which expect void *.
These are NOT exposed to the pUser
*/

/*****************************************************************
** FUNCTION:__sip_freeString
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeString
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeString((SIP_S8bit *)s);

}
/*****************************************************************
** FUNCTION:__sip_freeSipHeaderOrderInfo
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipHeaderOrderInfo
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	SipHeaderOrderInfo *pOrderInfo=SIP_NULL;

	if (s==SIP_NULL) return;
	pOrderInfo=(SipHeaderOrderInfo *)s;

    HSS_LOCKREF(pOrderInfo->dRefCount);HSS_DECREF(pOrderInfo->dRefCount);
	if(HSS_CHECKREF(pOrderInfo->dRefCount))
	{
		HSS_UNLOCKREF(pOrderInfo->dRefCount);
		HSS_DELETEREF(pOrderInfo->dRefCount);
		HSS_FREE(pOrderInfo);
	}
	else
	{
		HSS_UNLOCKREF(pOrderInfo->dRefCount);
	}
}


/*****************************************************************
** FUNCTION:__sip_freeSipTimerKey
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipTimerKey
#ifdef ANSI_PROTO
	(SIP_Pvoid k)
#else
	(k)
	SIP_Pvoid k;
#endif
{
	sip_freeSipTimerKey ((SipTimerKey *) k);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpOrigin
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpOrigin
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeSdpOrigin((SdpOrigin*) s);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpMedia
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpMedia
#ifdef ANSI_PROTO
	(SIP_Pvoid m)
#else
	(m)
	SIP_Pvoid m;
#endif
{
	sip_freeSdpMedia((SdpMedia*) m);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpAttr
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpAttr
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSdpAttr((SdpAttr*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpTime
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpTime
#ifdef ANSI_PROTO
	(SIP_Pvoid t)
#else
	(t)
	SIP_Pvoid t;
#endif
{
	sip_freeSdpTime ((SdpTime *) t);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpConnection
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpConnection
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
 	SIP_Pvoid c;
#endif
{
	sip_freeSdpConnection((SdpConnection*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSdpMessage
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSdpMessage
#ifdef ANSI_PROTO
	(SIP_Pvoid m)
#else
	(m)
	SIP_Pvoid m;
#endif
{
	sip_freeSdpMessage ((SdpMessage *) m);
}

#ifdef SIP_MWI
/*****************************************************************
** FUNCTION:__sip_freeSummaryLine
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_mwi_freeSummaryLine
#ifdef ANSI_PROTO
        (SIP_Pvoid s)
#else
        (s)
        SIP_Pvoid s;
#endif
{
        sip_mwi_freeSummaryLine((SummaryLine*) s);
}

#endif
/*****************************************************************
** FUNCTION:__sip_freeSipNameValuePair
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipNameValuePair
#ifdef ANSI_PROTO
        (SIP_Pvoid n)
#else
        (n)
        SIP_Pvoid n;
#endif
{
        sip_freeSipNameValuePair((SipNameValuePair*) n);
}

/*****************************************************************
** FUNCTION:__sip_freeSipParam
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipParam
#ifdef ANSI_PROTO
	(SIP_Pvoid p)
#else
	(p)
	SIP_Pvoid p;
#endif
{
	sip_freeSipParam((SipParam *) p);
}



/*****************************************************************
** FUNCTION:__sip_freeSipGenericChallenge
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipGenericChallenge
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipGenericChallenge ((SipGenericChallenge *) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipWwwAuthenticateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipWwwAuthenticateHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipWwwAuthenticateHeader ((SipWwwAuthenticateHeader *) h);
}


/*****************************************************************
** FUNCTION:__sip_freeSipProxyAuthenticateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipProxyAuthenticateHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid p)
#else
	(p)
	SIP_Pvoid p;
#endif
{
	sip_freeSipProxyAuthenticateHeader((SipProxyAuthenticateHeader*) p);
}


/*****************************************************************
** FUNCTION:__sip_freeSipAuthParam
**
**
** DESCRIPTION:
*******************************************************************/
/*
void __sip_freeSipAuthParam
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAuthParam((SipAuthParam*) a);
}
*/


/*****************************************************************
** FUNCTION:__sip_freeSipWarningHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipWarningHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid w)
#else
	(w)
	SIP_Pvoid w;
#endif
{
	sip_freeSipWarningHeader((SipWarningHeader*) w);
}


/*****************************************************************
** FUNCTION:__sip_freeSipDateFormat
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipDateFormat
#ifdef ANSI_PROTO
	(SIP_Pvoid d)
#else
	(d)
	SIP_Pvoid d;
#endif
{
	sip_freeSipDateFormat((SipDateFormat*) d);
}


/*****************************************************************
** FUNCTION:__sip_freeSipTimeFormat
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipTimeFormat
#ifdef ANSI_PROTO
	(SIP_Pvoid t)
#else
	(t)
	SIP_Pvoid t;
#endif
{
	sip_freeSipTimeFormat((SipTimeFormat*) t);
}


/*****************************************************************
** FUNCTION:__sip_freeSipDateStruct
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipDateStruct
#ifdef ANSI_PROTO
	(SIP_Pvoid d)
#else
	(d)
	SIP_Pvoid d;
#endif
{
	sip_freeSipDateStruct((SipDateStruct*) d);
}


/*****************************************************************
** FUNCTION:__sip_freeSipDateHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipDateHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid d)
#else
	(d)
	SIP_Pvoid d;
#endif
{
	sip_freeSipDateHeader((SipDateHeader*) d);
}


/*****************************************************************
** FUNCTION:__sip_freeSipAllowHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAllowHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAllowHeader((SipAllowHeader*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSipRetryAfterHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRetryAfterHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipRetryAfterHeader((SipRetryAfterHeader*) r);
}

/*****************************************************************
** FUNCTION:__sip_freeSipGenericCredential
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipGenericCredential
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipGenericCredential((SipGenericCredential*) c);
}

/*****************************************************************
** FUNCTION:__sip_freeSipAuthorizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAuthorizationHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAuthorizationHeader((SipAuthorizationHeader*) a);
}

/*****************************************************************
** FUNCTION:__sip_freeSipRespHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRespHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipRespHeader((SipRespHeader*) h);
}



/*****************************************************************
** FUNCTION:__sip_freeSipRespKeyHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRespKeyHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	( r)
 	SIP_Pvoid  r;
#endif
{
	sip_freeSipRespKeyHeader( (SipRespKeyHeader*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipUserAgentHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipUserAgentHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid u)
#else
	(u)
	SIP_Pvoid u;
#endif
{
	sip_freeSipUserAgentHeader((SipUserAgentHeader*) u);
}


/*****************************************************************
** FUNCTION:__sip_freeSipSubjectHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipSubjectHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeSipSubjectHeader((SipSubjectHeader*) s);
}


/*****************************************************************
** FUNCTION:__sip_freeSipProxyRequireHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipProxyRequireHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid p)
#else
	(p)
	SIP_Pvoid p;
#endif
{
	sip_freeSipProxyRequireHeader((SipProxyRequireHeader*) p);
}


/*****************************************************************
** FUNCTION:__sip_freeSipProxyAuthorizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipProxyAuthorizationHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipProxyAuthorizationHeader((SipProxyAuthorizationHeader*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSipPriorityHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipPriorityHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid p)
#else
	(p)
	SIP_Pvoid p;
#endif
{
	sip_freeSipPriorityHeader((SipPriorityHeader*) p);
}


/*****************************************************************
** FUNCTION:__sip_freeSipOrganizationHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipOrganizationHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid o)
#else
	(o)
	SIP_Pvoid o;
#endif
{
	sip_freeSipOrganizationHeader((SipOrganizationHeader*) o);
}


/*****************************************************************
** FUNCTION:__sip_freeSipContentTypeHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipContentTypeHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipContentTypeHeader((SipContentTypeHeader*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipContentEncodingHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipContentEncodingHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipContentEncodingHeader((SipContentEncodingHeader*) c);
}



/*****************************************************************
** FUNCTION:__sip_freeSipHideHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipHideHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipHideHeader((SipHideHeader*) h);
}


/*****************************************************************
** FUNCTION:__sip_freeSipMaxForwardsHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipMaxForwardsHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipMaxForwardsHeader((SipMaxForwardsHeader*) h);
}


/*****************************************************************
** FUNCTION:__sip_freeSipViaParam
**
**
** DESCRIPTION:
*******************************************************************/
/*
void __sip_freeSipViaParam
#ifdef ANSI_PROTO
	(SIP_Pvoid v)
#else
	(v)
	SIP_Pvoid v;
#endif
{
	sip_freeSipViaParam((SipViaParam*) v);
}
*/

/*****************************************************************
** FUNCTION:__sip_freeSipContentLengthHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipContentLengthHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipContentLengthHeader((SipContentLengthHeader*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipViaHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipViaHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid v)
#else
	(v)
	SIP_Pvoid v;
#endif
{
	sip_freeSipViaHeader((SipViaHeader*) v);
}


/*****************************************************************
** FUNCTION:__sip_freeSipUrlParam
**
**
** DESCRIPTION:
*******************************************************************/
/*
void __sip_freeSipUrlParam
#ifdef ANSI_PROTO
	(SIP_Pvoid u)
#else
	(u)
	SIP_Pvoid u;
#endif
{
	sip_freeSipUrlParam((SipUrlParam*) u);
}
*/

/*****************************************************************
** FUNCTION:__sip_freeSipUrl
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipUrl
#ifdef ANSI_PROTO
	(SIP_Pvoid u)
#else
	(u)
	SIP_Pvoid u;
#endif
{
	sip_freeSipUrl((SipUrl*) u);
}


/*****************************************************************
** FUNCTION:__sip_freeSipAddrSpec
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAddrSpec
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAddrSpec((SipAddrSpec*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSipToHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipToHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid t)
#else
	(t)
	SIP_Pvoid t;
#endif
{
	sip_freeSipToHeader((SipToHeader*) t);
}


/*****************************************************************
** FUNCTION:__sip_freeSipTimeStampHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipTimeStampHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid t)
#else
	(t)
	SIP_Pvoid t;
#endif
{
	sip_freeSipTimeStampHeader((SipTimeStampHeader*) t);
}


/*****************************************************************
** FUNCTION:__sip_freeSipRouteHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRouteHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipRouteHeader((SipRouteHeader*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipRecordRouteHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRecordRouteHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipRecordRouteHeader((SipRecordRouteHeader*) r);
}
#ifdef SIP_3GPP
/***********************************************************************
** FUNCTION:__sip_freeSipPathHeader
**
**
** DESCRIPTION: This release the memory allocated for the SipPathHeader
***********************************************************************/
void __sip_freeSipPathHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipPathHeader((SipPathHeader*) r);
}

/***********************************************************************
** FUNCTION:__sip_freeSipServiceRouteHeader
**
**
** DESCRIPTION: This release the memory allocated for the 
** SipServiceRoute header.
***********************************************************************/
void __sip_freeSipServiceRouteHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipServiceRouteHeader((SipServiceRouteHeader*) r);
}

/***********************************************************************
** FUNCTION:__sip_freeSipPanInfoHeader
**
**
** DESCRIPTION: This release the memory allocated for the SipPanInfoHeader
***********************************************************************/


void __sip_freeSipPanInfoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipPanInfoHeader((SipPanInfoHeader*) pHdr);
}
/***********************************************************************
** FUNCTION:__sip_freeSipPcVectorHeader
**
**
** DESCRIPTION: This release the memory allocated for the SipPcVectorHeader
***********************************************************************/

void __sip_freeSipPcVectorHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipPcVectorHeader((SipPcVectorHeader*) pHdr);
}

#endif

/*****************************************************************
** FUNCTION:__sip_freeSipRequireHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRequireHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipRequireHeader((SipRequireHeader*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipFromHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipFromHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid t)
#else
	(t)
	SIP_Pvoid t;
#endif
{
	sip_freeSipFromHeader((SipFromHeader*) t);
}


/*****************************************************************
** FUNCTION:__sip_freeSipExpiresStruct
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipExpiresStruct
#ifdef ANSI_PROTO
	(SIP_Pvoid e)
#else
	(e)
	SIP_Pvoid e;
#endif
{
	sip_freeSipExpiresStruct((SipExpiresStruct*) e);
}



/*****************************************************************
** FUNCTION:__sip_freeSipExpiresHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipExpiresHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid e)
#else
	(e)
	SIP_Pvoid e;
#endif
{
	sip_freeSipExpiresHeader((SipExpiresHeader*) e);
}


/*****************************************************************
** FUNCTION:__sip_freeSipEncryptionHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipEncryptionHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid e)
#else
	(e)
	SIP_Pvoid e;
#endif
{
	sip_freeSipEncryptionHeader((SipEncryptionHeader*) e);
}


/*****************************************************************
** FUNCTION:__sip_freeSipContactParam
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipContactParam
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipContactParam((SipContactParam*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipContactHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipContactHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipContactHeader((SipContactHeader*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipCseqHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipCseqHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipCseqHeader((SipCseqHeader*) c);
}


/*****************************************************************
** FUNCTION:__sip_freeSipCallIdHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipCallIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipCallIdHeader((SipCallIdHeader*) c);
}
/*****************************************************************
** FUNCTION:__sip_freeSipReplacesHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipReplacesHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipReplacesHeader((SipReplacesHeader*) c);
}
/*****************************************************************
** FUNCTION:__sip_freeSipReplyToHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipReplyToHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipReplyToHeader((SipReplyToHeader*) c);
}

/*****************************************************************
** FUNCTION:__sip_freeSipAcceptHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAcceptHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAcceptHeader((SipAcceptHeader*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSipAcceptEncodingHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAcceptEncodingHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAcceptEncodingHeader((SipAcceptEncodingHeader*) a);
}


/*****************************************************************
** FUNCTION:__sip_freeSipAcceptLangHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipAcceptLangHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipAcceptLangHeader((SipAcceptLangHeader*) a);
}

/*****************************************************************
** FUNCTION:__sip_freeSipUnsupportedHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipUnsupportedHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipUnsupportedHeader((SipUnsupportedHeader*) a);
}

/*****************************************************************
** FUNCTION:__sip_freeSipServerHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipServerHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid a)
#else
	(a)
	SIP_Pvoid a;
#endif
{
	sip_freeSipServerHeader((SipServerHeader*) a);
}

/*****************************************************************
** FUNCTION:__sip_freeSipReqHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipReqHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeSipReqHeader((SipReqHeader*) s);
}

/*****************************************************************
** FUNCTION:__sip_freeSipUnknownHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipUnknownHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid u)
#else
	(u)
	SIP_Pvoid u;
#endif
{
	sip_freeSipUnknownHeader((SipUnknownHeader*) u);
}

/*****************************************************************
** FUNCTION:__sip_freeSipBadHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipBadHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid u)
#else
	(u)
	SIP_Pvoid u;
#endif
{
	sip_freeSipBadHeader((SipBadHeader *) u);
}

/*****************************************************************
** FUNCTION:__sip_freeSipGeneralHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipGeneralHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid g)
#else
	(g)
	SIP_Pvoid g;
#endif
{
	sip_freeSipGeneralHeader((SipGeneralHeader*) g);
}


/*****************************************************************
** FUNCTION:__sip_freeSipReqLine
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipReqLine
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipReqLine((SipReqLine*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipReqMessage
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipReqMessage
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipReqMessage((SipReqMessage*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipStatusLine
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipStatusLine
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeSipStatusLine((SipStatusLine*) s);
}


/*****************************************************************
** FUNCTION:__sip_freeSipRespMessage
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipRespMessage
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipRespMessage((SipRespMessage*) r);
}


/*****************************************************************
** FUNCTION:__sip_freeSipMessage
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipMessage
#ifdef ANSI_PROTO
	(SIP_Pvoid s)
#else
	(s)
	SIP_Pvoid s;
#endif
{
	sip_freeSipMessage((SipMessage*) s);
}


/*****************************************************************
** FUNCTION:__sip_freeSipHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipHeader((SipHeader*) h);
}
/* freeSipHeader */

void __sip_freeSipMsgBody
#ifdef ANSI_PROTO
	(SIP_Pvoid h)
#else
	(h)
	SIP_Pvoid h;
#endif
{
	sip_freeSipMsgBody((SipMsgBody*) h);
}

void sip_freeSipSupportedHeader
#ifdef ANSI_PROTO
	(SipSupportedHeader *pHdr)
#else
	(pHdr)
	SipSupportedHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_freeString(pHdr->pOption);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipSupportedHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipSupportedHeader((SipSupportedHeader *)pHdr);
}


/*****************************************************************
** FUNCTION:sip_freeAlertInfoHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipAlertInfoHeader
#ifdef ANSI_PROTO
	(SipAlertInfoHeader *pHdr)
#else
	(pHdr)
	SipAlertInfoHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pUri);
		sip_listDeleteAll( &(pHdr->slParam), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipAlertInfoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipAlertInfoHeader((SipAlertInfoHeader *)pHdr);
}


/*****************************************************************
** FUNCTION:sip_freeSipReferToHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipReferToHeader
#ifdef ANSI_PROTO
	(SipReferToHeader *pHdr)
#else
	(pHdr)
	SipReferToHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		sip_listDeleteAll( &(pHdr->slParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipReferToHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipReferToHeader((SipReferToHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipReferredByHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipReferredByHeader
#ifdef ANSI_PROTO
	(SipReferredByHeader *pHdr)
#else
	(pHdr)
	SipReferredByHeader *pHdr;
#endif
{
	SipError err;

	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		HSS_FREE(pHdr->pMsgId);
		sip_freeSipAddrSpec(pHdr->pAddrSpecReferrer);
		sip_freeSipAddrSpec(pHdr->pAddrSpecReferenced);
		sip_listDeleteAll(&(pHdr->slParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipReferredByHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipReferredByHeader((SipReferredByHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipInReplyToHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipInReplyToHeader
#ifdef ANSI_PROTO
	(SipInReplyToHeader *pHdr)
#else
	(pHdr)
	SipInReplyToHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pCallId);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipInReplyToHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipInReplyToHeader((SipInReplyToHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipAlsoHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipAlsoHeader
#ifdef ANSI_PROTO
	(SipAlsoHeader *pHdr)
#else
	(pHdr)
	SipAlsoHeader *pHdr;
#endif
{
	if(pHdr == SIP_NULL)
		return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipAlsoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipAlsoHeader((SipAlsoHeader*)pHdr);
}


/*****************************************************************
** FUNCTION:sip_freeSipCallInfoHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipCallInfoHeader
#ifdef ANSI_PROTO
	(SipCallInfoHeader *pHdr)
#else
	(pHdr)
	SipCallInfoHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pUri);
		sip_listDeleteAll( &(pHdr->slParam), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipCallInfoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipCallInfoHeader((SipCallInfoHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeContentDispositionHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipContentDispositionHeader
#ifdef ANSI_PROTO
	(SipContentDispositionHeader *pHdr)
#else
	(pHdr)
	SipContentDispositionHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispType);
		sip_listDeleteAll( &(pHdr->slParam), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipContentDispositionHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipContentDispositionHeader((SipContentDispositionHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeReasonHeader
**
**
** DESCRIPTION: This releases the memory allocated to the members of
**					the SipReasonHeader and the Structure itself.
*******************************************************************/
void sip_freeSipReasonHeader
#ifdef ANSI_PROTO
	(SipReasonHeader *pHdr)
#else
	(pHdr)
	SipReasonHeader *pHdr;
#endif
{
	SipError dErr;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispType);
		sip_listDeleteAll( &(pHdr->slParam), &dErr);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipReasonHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipReasonHeader((SipReasonHeader *)pHdr);
}


/*****************************************************************
** FUNCTION:sip_freeContentLanguageHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipContentLanguageHeader
#ifdef ANSI_PROTO
	(SipContentLanguageHeader *pHdr)
#else
	(pHdr)
	SipContentLanguageHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pLangTag);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipContentLanguageHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipContentLanguageHeader((SipContentLanguageHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipErrorInfoHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipErrorInfoHeader
#ifdef ANSI_PROTO
	(SipErrorInfoHeader *pHdr)
#else
	(pHdr)
	SipErrorInfoHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr == SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pUri);
		sip_listDeleteAll( &(pHdr->slParam), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipErrorInfoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipErrorInfoHeader((SipErrorInfoHeader *)pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipMinExpiresHeader
**
**
** DESCRIPTION: Free the MinExpires header if the ref count
**									has become zero.
*******************************************************************/
void sip_freeSipMinExpiresHeader
#ifdef ANSI_PROTO
       (SipMinExpiresHeader *pHdr)
#else
       (pHdr)
        SipMinExpiresHeader *pHdr;
#endif
{
/* NEXTONE - comment out checking of SIP_NO_CHECK */ 
/* #ifndef SIP_NO_CHECK */
        if(pHdr == SIP_NULL)
                return;
/* #endif */
        HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
        if(HSS_CHECKREF(pHdr->dRefCount))
        {
                HSS_UNLOCKREF(pHdr->dRefCount);
                HSS_DELETEREF(pHdr->dRefCount);
                HSS_FREE(pHdr);
        }
        else
        {
                HSS_UNLOCKREF(pHdr->dRefCount);
        }
}

/*****************************************************************
** FUNCTION:__sip_freeSipMinExpiresHeader
**
**
** DESCRIPTION:Free the MinExpires header if the ref count
**									has become zero.
*******************************************************************/
void __sip_freeSipMinExpiresHeader
#ifdef ANSI_PROTO
        (SIP_Pvoid pHdr)
#else
        (pHdr)
        SIP_Pvoid pHdr;
#endif
{
        sip_freeSipMinExpiresHeader((SipMinExpiresHeader*)pHdr);
}

#ifdef SIP_SESSIONTIMER
/*****************************************************************
** FUNCTION:sip_freeSipMinSEHeader
**
**
** DESCRIPTION: Free the MinSE header if the ref count
**									has become zero.
*******************************************************************/
void sip_freeSipMinSEHeader
#ifdef ANSI_PROTO
       (SipMinSEHeader *pHdr)
#else
       (pHdr)
        SipMinSEHeader *pHdr;
#endif
{
		SipError err;

/* NEXTONE - comment out SIP_NO_CHECK */
/* #ifndef SIP_NO_CHECK */
        if(pHdr == SIP_NULL)
                return;
/* #endif */

        HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
        if(HSS_CHECKREF(pHdr->dRefCount))
        {
				sip_listDeleteAll( &(pHdr->slNameValue), &err);
                HSS_UNLOCKREF(pHdr->dRefCount);
                HSS_DELETEREF(pHdr->dRefCount);
				HSS_FREE(pHdr);
        }
        else
        {
                HSS_UNLOCKREF(pHdr->dRefCount);
        }
}

/*****************************************************************
** FUNCTION:__sip_freeSipMinSEHeader
**
**
** DESCRIPTION:Free the MinSE header if the ref count
**									has become zero.
*******************************************************************/
void __sip_freeSipMinSEHeader
#ifdef ANSI_PROTO
        (SIP_Pvoid pHdr)
#else
        (pHdr)
        SIP_Pvoid pHdr;
#endif
{
        sip_freeSipMinSEHeader((SipMinSEHeader*)pHdr);
}

/*****************************************************************
** FUNCTION:__sip_freeSipSessionExpiresHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipSessionExpiresHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid e)
#else
	(e)
	SIP_Pvoid e;
#endif
{
	sip_freeSipSessionExpiresHeader((SipSessionExpiresHeader*) e);
}
#endif


/*****************************************************************
** FUNCTION:sip_freeSipAuthenticationInfoHeader
**
**
** DESCRIPTION:
*******************************************************************/
void sip_freeSipAuthenticationInfoHeader
#ifdef ANSI_PROTO
	(SipAuthenticationInfoHeader *pHdr)
#else
	(pHdr)
	SipAuthenticationInfoHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr==SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_listDeleteAll( &(pHdr->slNameValue), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

void __sip_freeSipAuthenticationInfoHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipAuthenticationInfoHeader ((SipAuthenticationInfoHeader *) pHdr);
}

#ifdef SIP_PRIVACY
/*****************************************************************
** FUNCTION:sip_freeSipPAssertIdHeader
**
**
** DESCRIPTION:This function will free the SipPAssertIdHeader 
** structure.
*******************************************************************/
void sip_freeSipPAssertIdHeader
#ifdef ANSI_PROTO
	(SipPAssertIdHeader *pHdr)
#else
	(pHdr)
	SipPAssertIdHeader *pHdr;
#endif
{
	if (pHdr==SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:__sip_freeSipPAssertIdHeader
**
**
** DESCRIPTION:This function will free the SipPAssertIdHeader 
** structure.
*******************************************************************/
void __sip_freeSipPAssertIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipPAssertIdHeader((SipPAssertIdHeader*) pHdr);
}

/*****************************************************************
** FUNCTION:sip_freeSipPPreferredIdHeader
**
**
** DESCRIPTION:This function will free the SipPPreferredIdHeader 
** structure.
*******************************************************************/
void sip_freeSipPPreferredIdHeader
#ifdef ANSI_PROTO
	(SipPPreferredIdHeader *pHdr)
#else
	(pHdr)
	SipPPreferredIdHeader *pHdr;
#endif
{
	if (pHdr==SIP_NULL) return;

	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:__sip_freeSipPPreferredIdHeader
**
**
** DESCRIPTION:This function will free the SipPPreferredIdHeader 
** structure.
*******************************************************************/
void __sip_freeSipPPreferredIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid pHdr)
#else
	(pHdr)
	SIP_Pvoid pHdr;
#endif
{
	sip_freeSipPPreferredIdHeader((SipPPreferredIdHeader*) pHdr);
}
/***********************************************************************
** FUNCTION : sip_freeSipPrivacyHeader
**
** DESCRIPTION : 
**
***********************************************************************/
void sip_freeSipPrivacyHeader
#ifdef ANSI_PROTO
(SipPrivacyHeader *pHdr)
#else
(pHdr)
SipPrivacyHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr==SIP_NULL) return;

 	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_listDeleteAll( &(pHdr->slPrivacyValue), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
	return;
}

void __sip_freeSipPrivacyHeader
#ifdef ANSI_PROTO
(SIP_Pvoid pHdr)
#else
(pHdr)
SIP_Pvoid pHdr;
#endif
{
	sip_freeSipPrivacyHeader ((SipPrivacyHeader *) pHdr);
}
# endif /* ifdef SIP_PRIVACY */

#ifdef SIP_CONGEST
/***********************************************************************
** FUNCTION : sip_freeSipRsrcPriorityHeader
**
** DESCRIPTION : 
**
***********************************************************************/
void sip_freeSipRsrcPriorityHeader
#ifdef ANSI_PROTO
(SipRsrcPriorityHeader *pHdr)
#else
(pHdr)
SipRsrcPriorityHeader *pHdr;
#endif
{
	if (pHdr==SIP_NULL) return;

 	HSS_LOCKREF(pHdr->dRefCount);
	HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pNamespace);
		HSS_FREE(pHdr->pPriority);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
	return;
}

void __sip_freeSipRsrcPriorityHeader
#ifdef ANSI_PROTO
(SIP_Pvoid pHdr)
#else
(pHdr)
SIP_Pvoid pHdr;
#endif
{
	sip_freeSipRsrcPriorityHeader ((SipRsrcPriorityHeader *) pHdr);
}

void __sip_freeSipAcceptRsrcPriorityHeader
#ifdef ANSI_PROTO
(SIP_Pvoid pHdr)
#else
(pHdr)
SIP_Pvoid pHdr;
#endif
{
	sip_freeSipRsrcPriorityHeader ((SipRsrcPriorityHeader *) pHdr);
}

void sip_freeSipAcceptRsrcPriorityHeader
#ifdef ANSI_PROTO
(SipAcceptRsrcPriorityHeader *pHdr)
#else
(pHdr)
SipAcceptRsrcPriorityHeader *pHdr;
#endif
{
   	sip_freeSipRsrcPriorityHeader ((SipRsrcPriorityHeader *) pHdr);

}

# endif /* ifdef SIP_CONGEST */

#ifdef SIP_CONF
/*****************************************************************
** FUNCTION:sip_freeSipJoinHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipJoinHeader
#ifdef ANSI_PROTO
	(SipJoinHeader *c)
#else
	(c)
	SipJoinHeader *c;
#endif
{
	SipError dError;
	if (c==SIP_NULL) return;
	HSS_LOCKREF(c->dRefCount);HSS_DECREF(c->dRefCount);
	if(HSS_CHECKREF(c->dRefCount))
	{
		HSS_FREE(c->pCallid);
		HSS_FREE(c->pFromTag);
		HSS_FREE(c->pToTag);
		sip_listDeleteAll( &(c->slParams), &dError);
		HSS_UNLOCKREF(c->dRefCount);
		HSS_DELETEREF(c->dRefCount);
		HSS_FREE(c);
	}
	else
	{
		HSS_UNLOCKREF(c->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:__sip_freeSipJoinHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipJoinHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid c)
#else
	(c)
	SIP_Pvoid c;
#endif
{
	sip_freeSipJoinHeader((SipJoinHeader*) c);
}

#endif

#ifdef SIP_3GPP
/*******************************************************************************
** FUNCTION:sip_freeSipPAssociatedUriHeader
**
**
** DESCRIPTION: Checks the reference count of the P-Associated-URI header structure. If it is 0 frees the structure, else just decrements the reference count.
*******************************************************************************/

void sip_freeSipPAssociatedUriHeader
#ifdef ANSI_PROTO
	(SipPAssociatedUriHeader *pHdr)
#else
	(pHdr)
	SipPAssociatedUriHeader *pHdr;
#endif
{
	SipError err;
	if (pHdr==SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_FREE(pHdr->pDispName);
		sip_freeSipAddrSpec(pHdr->pAddrSpec);
		sip_listDeleteAll( &(pHdr->slParams), &err);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:__sip_freeSipPAssociatedUriHeader
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_freeSipPAssociatedUriHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
	sip_freeSipPAssociatedUriHeader((SipPAssociatedUriHeader*) r);
}

/*******************************************************************************
** FUNCTION:sip_freeSipPCalledPartyIdHeader
**
**
** DESCRIPTION:Since the structure type is the same as the PAssociatedUriHeader structure, just calls the function for freeing the PAssociatedUriHeader structure
*******************************************************************************/

void sip_freeSipPCalledPartyIdHeader
#ifdef ANSI_PROTO
	(SipPCalledPartyIdHeader *r)
#else
	(r)
	SipPCalledPartyIdHeader *r;
#endif
{
    sip_freeSipPAssociatedUriHeader((SipPAssociatedUriHeader *)r);

}

/*******************************************************************************
** FUNCTION:__sip_freeSipPCalledPartyIdHeader
**
**
** DESCRIPTION:Since the structure type is the same as the PAssociatedUriHeader structure, just calls the function for freeing the PAssociatedUriHeader structure
*******************************************************************************/

void __sip_freeSipPCalledPartyIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
    sip_freeSipPAssociatedUriHeader((SipPAssociatedUriHeader *)r);
}

/*******************************************************************************
** FUNCTION:sip_freeSipPVisitedNetworkIdHeader
**
**
** DESCRIPTION: This function will free the PVisitedNetworkId structure.
******************************************************************************/

void sip_freeSipPVisitedNetworkIdHeader
#ifdef ANSI_PROTO
	(SipPVisitedNetworkIdHeader *r)
#else
	(r)
	SipPVisitedNetworkIdHeader *r;
#endif
{
	SipError err;
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		HSS_FREE(r->pVNetworkSpec);
		sip_listDeleteAll( &(r->slParams), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}

}

/*******************************************************************************
** FUNCTION:__sip_freeSipPVisitedNetworkIdHeader
**
**
** DESCRIPTION: This function will free the PVisitedNetworkId structure.
*******************************************************************************/

void __sip_freeSipPVisitedNetworkIdHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
    sip_freeSipPVisitedNetworkIdHeader((SipPVisitedNetworkIdHeader *)r);
}

/*******************************************************************************
** FUNCTION:sip_freeSipPcfAddrHeader
**
**
** DESCRIPTION: This function will free the PcfAddr header structure.
*******************************************************************************/

void sip_freeSipPcfAddrHeader
#ifdef ANSI_PROTO
	(SipPcfAddrHeader *r)
#else
	(r)
	SipPcfAddrHeader *r;
#endif
{
	SipError err;
	if (r==SIP_NULL) return;
	HSS_LOCKREF(r->dRefCount);HSS_DECREF(r->dRefCount);
	if(HSS_CHECKREF(r->dRefCount))
	{
		sip_listDeleteAll( &(r->slParams), &err);
		HSS_UNLOCKREF(r->dRefCount);
		HSS_DELETEREF(r->dRefCount);
		HSS_FREE(r);
	}
	else
	{
		HSS_UNLOCKREF(r->dRefCount);
	}
}

/*******************************************************************************
** FUNCTION:__sip_freeSipPcfAddrHeader
**
**
** DESCRIPTION:
*******************************************************************************/

void __sip_freeSipPcfAddrHeader
#ifdef ANSI_PROTO
	(SIP_Pvoid r)
#else
	(r)
	SIP_Pvoid r;
#endif
{
    sip_freeSipPcfAddrHeader((SipPcfAddrHeader *)r);
}

#endif

#ifdef SIP_SECURITY
/*****************************************************************
** FUNCTION:sip_freeSipSecurityClientHeader
**
**
** DESCRIPTION:
*******************************************************************/
  
void sip_freeSipSecurityClientHeader
	(SipSecurityClientHeader *s)
{
	SipError err;
	if (s==SIP_NULL) return;
	HSS_LOCKREF(s->dRefCount);HSS_DECREF(s->dRefCount);
	if(HSS_CHECKREF(s->dRefCount))
	{
		HSS_FREE(s->pMechanismName);
		sip_listDeleteAll( &(s->slParams), &err);
		HSS_UNLOCKREF(s->dRefCount);
		HSS_DELETEREF(s->dRefCount);
		HSS_FREE(s);
	}
	else
	{
		HSS_UNLOCKREF(s->dRefCount);
	}
}
    

/*****************************************************************
** FUNCTION:__sip_freeSipSecurityClientHeader
**
**
** DESCRIPTION:
*******************************************************************************/

void __sip_freeSipSecurityClientHeader
	(SIP_Pvoid s)
{
	sip_freeSipSecurityClientHeader((SipSecurityClientHeader*) s);
}

/*****************************************************************
** FUNCTION:sip_freeSipSecurityServerHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipSecurityServerHeader
        (SipSecurityServerHeader *s)
{
       sip_freeSipSecurityClientHeader((SipSecurityClientHeader *)s);
}
/*****************************************************************
** FUNCTION:__sip_freeSipSecurityServerHeader
**
**
** DESCRIPTION:
*******************************************************************************/

void __sip_freeSipSecurityServerHeader
        (SIP_Pvoid s)
{
        sip_freeSipSecurityServerHeader((SipSecurityServerHeader*) s);
}

/*****************************************************************
** FUNCTION:sip_freeSipSecurityVerifyHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_freeSipSecurityVerifyHeader
        (SipSecurityVerifyHeader *s)
{
       sip_freeSipSecurityClientHeader((SipSecurityClientHeader *)s);
}
/*****************************************************************
** FUNCTION:__sip_freeSipSecurityVerifyHeader
**
**
** DESCRIPTION:
*******************************************************************************/

void __sip_freeSipSecurityVerifyHeader
        (SIP_Pvoid s)
{
        sip_freeSipSecurityVerifyHeader((SipSecurityVerifyHeader*) s);
}

#endif /* end of #ifdef SECURITY */

