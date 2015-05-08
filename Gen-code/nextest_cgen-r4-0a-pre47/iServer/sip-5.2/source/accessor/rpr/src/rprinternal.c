/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rprinternal.c
**
** DESCRIPTION:
**  	
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 23Feb2000   	S.Luthra    	Original
** 28Feb2000	B.Borthakur						Included Supported heeader
** 28Feb2000	B.Borthakur						Included make functions
**
** 24Jul00	S.Luthra	Moved Supported header clone and make APIs to sipclone.c & sipstring.c
**
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/

#include "rprinternal.h"
#include "portlayer.h"
#include "sipdecodeintrnl.h"
#include "sipdecode.h"
#include "sipparserinc.h"


/****************************************************************
**
** FUNCTION: __sip_rpr_cloneSipRAckHeader
**
** DESCRIPTION: This function makes a deep copy of the pSource RAck 
** pHeader structure to pDest RAck pHeader.
**
*****************************************************************/
SipBool __sip_rpr_cloneSipRAckHeader
#ifdef ANSI_PROTO
	(SipRackHeader *pDest, SipRackHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRackHeader *pDest;
	SipRackHeader *pSource;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTemp;
	SIPDEBUGFN("Entering function __sip_rpr_cloneSipRAckHeader");

	/* freeig up existing field values of the destination RAck 
	structure */
	if (pDest->pMethod != SIP_NULL)
	{
		sip_freeString(pDest->pMethod);
		pDest->pMethod = SIP_NULL;
	}
	/* copying values from source to destination */
	pDest->dRespNum = pSource->dRespNum;
	pDest->dCseqNum = pSource->dCseqNum;
	if (pSource->pMethod == SIP_NULL)
		pTemp = SIP_NULL;
	else
	{
		pTemp = (SIP_S8bit *)STRDUPACCESSOR(pSource->pMethod);
		if (pTemp == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}
	pDest->pMethod = pTemp;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_rpr_cloneSipRAckHeader");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: __sip_rpr_cloneSipRSeqHeader
**
** DESCRIPTION: This function makes a deep copy of the pSource 
** Rseq pHeader to pDest RSeq pHeader structure. 
**
*****************************************************************/
SipBool __sip_rpr_cloneSipRSeqHeader
#ifdef ANSI_PROTO
	(SipRseqHeader *pDest, SipRseqHeader *pSource, SipError *pErr)
#else
	(pDest, pSource, pErr)
	SipRseqHeader *pDest;
	SipRseqHeader *pSource;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function __sip_rpr_cloneSipRSeqHeader");
	pDest->dRespNum = pSource->dRespNum;
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exitting function __sip_rpr_cloneSipRSeqHeader");
	return SipSuccess;
}

