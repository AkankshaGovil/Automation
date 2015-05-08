
/***********************************************************************
 ** FUNCTION:
 **             Has Free Functions For all bcpt Structures

 *********************************************************************
 **
 ** FILENAME:
 ** sipbcptfree.c
 **
 ** DESCRIPTION:
 ** This file contains dCodeNum to free all bcpt structures
 **
 ** DATE        NAME                    REFERENCE               REASON
 ** ----        ----                    ---------              --------
 ** 09/02/00   B. Borthakur       		                    Initial Creation
 **
 **
 **     Copyright 1999, Hughes Software Systems, Ltd.
 *********************************************************************/


#include <stdlib.h>
#include "sipstruct.h"
#include "sipfree.h"
#include "sipbcptfree.h"
#include "portlayer.h"


/* These are the functions that a USER will call to free structures.
They are strongly typecasted 
*/
/*****************************************************************
** FUNCTION:sip_bcpt_freeIsupMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_bcpt_freeIsupMessage
#ifdef ANSI_PROTO
	(IsupMessage * m)
#else
	(m)
	IsupMessage *m;
#endif
{
	if (m==SIP_NULL) return;

	HSS_LOCKREF(m->dRefCount);HSS_DECREF(m->dRefCount);
	if(HSS_CHECKREF(m->dRefCount))
	{
		HSS_FREE(m->pBody);
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
** FUNCTION:sip_bcpt_freeQsigMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_bcpt_freeQsigMessage
#ifdef ANSI_PROTO
	(QsigMessage * ppM)
#else
	(ppM)
	QsigMessage *ppM;
#endif
{
	if (ppM==SIP_NULL)
		return;

	HSS_LOCKREF(ppM->dRefCount);
	HSS_DECREF(ppM->dRefCount);
	if(HSS_CHECKREF(ppM->dRefCount))
	{
		HSS_FREE(ppM->pBody);
		HSS_UNLOCKREF(ppM->dRefCount);
		HSS_DELETEREF(ppM->dRefCount);
		HSS_FREE(ppM);
	}
	else
	{
		HSS_UNLOCKREF(ppM->dRefCount);
	}
}

/*****************************************************************
** FUNCTION:sip_bcpt_freeSipMimeVersionHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_bcpt_freeSipMimeVersionHeader
#ifdef ANSI_PROTO
	(SipMimeVersionHeader * h)
#else
	(h)
	SipMimeVersionHeader *h;
#endif
{
	if (h==SIP_NULL) return;
	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount); 
	if(HSS_CHECKREF(h->dRefCount))
	{
		HSS_FREE(h->pVersion);
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
** FUNCTION:sip_bcpt_freeMimeMessage
**
**
** DESCRIPTION:
*******************************************************************/

void sip_bcpt_freeMimeMessage
#ifdef ANSI_PROTO
	(MimeMessage * m)
#else
	(m)
	MimeMessage *m;
#endif
{
	SipError temp_err;

	if (m==SIP_NULL) return;
	HSS_LOCKREF(m->dRefCount);HSS_DECREF(m->dRefCount);
	if(HSS_CHECKREF(m->dRefCount))
	{
		sip_listDeleteAll(&(m->slRecmimeBody), &temp_err);
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
** FUNCTION:sip_bcpt_freeSipMimeHeader
**
**
** DESCRIPTION:
*******************************************************************/

void sip_bcpt_freeSipMimeHeader
#ifdef ANSI_PROTO
	(SipMimeHeader * h)
#else
	(h)
	SipMimeHeader *h;
#endif
{
	SipError temp_err;

	if (h==SIP_NULL) return;
	HSS_LOCKREF(h->dRefCount);HSS_DECREF(h->dRefCount);
	if(HSS_CHECKREF(h->dRefCount))
	{
		sip_listDeleteAll(&(h->slAdditionalMimeHeaders), &temp_err);
		HSS_FREE( h->pContentTransEncoding);
		HSS_FREE( h->pContentId );
		HSS_FREE( h->pContentDescription );
		sip_freeSipContentTypeHeader( h->pContentType);
		sip_freeSipContentDispositionHeader( h->pContentDisposition);
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
** FUNCTION:__sip_bcpt_freeMimeMessage
**
**
** DESCRIPTION:
*******************************************************************/

void __sip_bcpt_freeMimeMessage
#ifdef ANSI_PROTO
	(SIP_Pvoid  m)
#else
	(m)
	SIP_Pvoid m;
#endif
{
	sip_bcpt_freeMimeMessage((MimeMessage*)m);
}

