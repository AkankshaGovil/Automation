/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprfree.c
**
** DESCRIPTION:
**  	
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 23Feb2000   	S.Luthra    	Original
**
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/

#include "rprfree.h"
#include "portlayer.h"

void sip_rpr_freeSipRAckHeader
#ifdef ANSI_PROTO
	(SipRackHeader *pHdr)
#else
	(pHdr)
	SipRackHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		sip_freeString(pHdr->pMethod);
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE(pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}


void sip_rpr_freeSipRSeqHeader
#ifdef ANSI_PROTO
	(SipRseqHeader *pHdr)
#else
	(pHdr)
	SipRseqHeader *pHdr;
#endif
{
	if (pHdr == SIP_NULL) return;
	HSS_LOCKREF(pHdr->dRefCount);HSS_DECREF(pHdr->dRefCount);
	if(HSS_CHECKREF(pHdr->dRefCount))
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
		HSS_DELETEREF(pHdr->dRefCount);
		HSS_FREE (pHdr);
	}
	else
	{
		HSS_UNLOCKREF(pHdr->dRefCount);
	}
}



