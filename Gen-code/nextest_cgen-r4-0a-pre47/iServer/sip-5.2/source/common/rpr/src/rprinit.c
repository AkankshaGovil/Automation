/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprinit.c
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

#include "rprinit.h"
#include "portlayer.h"


SipBool sip_rpr_initSipRAckHeader
#ifdef ANSI_PROTO
	(SipRackHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipRackHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipRackHeader *)fast_memget (0, sizeof(SipRackHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	(*ppHdr)->dRespNum = 0;
	(*ppHdr)->dCseqNum = 0;
	INIT ( (*ppHdr)->pMethod);
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}


SipBool sip_rpr_initSipRSeqHeader
#ifdef ANSI_PROTO
	(SipRseqHeader **ppHdr, SipError *pErr)
#else
	(ppHdr, pErr)
	SipRseqHeader **ppHdr;
	SipError *pErr;
#endif
{
	*ppHdr = (SipRseqHeader *)fast_memget(0, sizeof(SipRseqHeader), pErr);
	if (*ppHdr == SIP_NULL)
		return SipFail;
	(*ppHdr)->dRespNum = 0;
	HSS_INITREF((*ppHdr)->dRefCount);
	*pErr = E_NO_ERROR;
	return SipSuccess;
}





