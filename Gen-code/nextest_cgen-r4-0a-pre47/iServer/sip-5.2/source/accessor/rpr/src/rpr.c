/******************************************************************************
** FUNCTION:
** 	
**
*******************************************************************************
**
** FILENAME:
** 	rpr.c
**
** DESCRIPTION:
**  	
**
** DATE      	NAME        	REFERENCE      	REASON
** ----      	----        	---------      	------
** 23Feb2000   	S.Luthra    	Original
** 28Feb2000	B.Borthakur						Included Supported heeader
** 26Jul2000	S.Luthra	Added SIP_NO_CHECK support
**
** Copyrights 1999, Hughes Software Systems, Ltd.
**
******************************************************************************/

#include "rpr.h"
#include "portlayer.h"

/****************************************************************
**
** FUNCTION: sip_rpr_getRespNumFromRSeqHdr
**
** DESCRIPTION: This function retrives the RespNum field from the
** RSeq pHeader 
**
*****************************************************************/
SipBool sip_rpr_getRespNumFromRSeqHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pRespNum, SipError *pErr)
#else
	(pHdr, pRespNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pRespNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_getRespNumFromRSeqHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pRespNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRSeq)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	/* assigning the dRespNum pValue to be returned */
	*pRespNum = ( (SipRseqHeader *)(pHdr->pHeader) )->dRespNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_getRespNumFromRSeqHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_rpr_setRespNumInRSeqHdr
**
** DESCRIPTION: This function sets the RespNum field in the RSeq
** pHeader
**
*****************************************************************/
SipBool sip_rpr_setRespNumInRSeqHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dRespNum, SipError *pErr)
#else
	(pHdr, dRespNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit dRespNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_setRespNumInRSeqHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRSeq)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	/* setting the dRespNum pValue in the structure */
	((SipRseqHeader *)(pHdr->pHeader))->dRespNum = dRespNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_setRespNumInRSeqHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_rpr_getRespNumFromRAckHdr
**
** DESCRIPTION: This function retrives the ResNum field from the 
** RAck pHeader.
**
*****************************************************************/
SipBool sip_rpr_getRespNumFromRAckHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pRespNum, SipError *pErr)
#else
	(pHdr, pRespNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pRespNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_getRespNumFromRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pRespNum == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	/* Setting the dRespNum pValue to be returned */
	*pRespNum = ( (SipRackHeader *)(pHdr->pHeader) )->dRespNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_getRespNumFromRAckHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_rpr_setRespNumInRAckHdr
**
** DESCRIPTION: This function sets the RespNum field in the RAck
** pHeader.
**
*****************************************************************/
SipBool sip_rpr_setRespNumInRAckHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit dRespNum, SipError *pErr)
#else
	(pHdr, dRespNum, pErr)
	SipHeader *pHdr;
	SIP_U32bit dRespNum;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_setRespNumInRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* setting the dRespNum pValue in the structure */
	((SipRackHeader *)(pHdr->pHeader))->dRespNum = dRespNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_setRespNumInRAckHdr");
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_rpr_getCseqNumFromRAckHdr
**
** DESCRIPTION: This function retrives the CSeq field from the
** RAck pHeader. 
**
*****************************************************************/
SipBool sip_rpr_getCseqNumFromRAckHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit *pCseq, SipError *pErr)
#else
	(pHdr, pCseq, pErr)
	SipHeader *pHdr;
	SIP_U32bit *pCseq;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_getCseqNumFromRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pCseq == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif	
	/* setting the Cseq pValue to be returned */
	*pCseq = ( (SipRackHeader *)(pHdr->pHeader) )->dCseqNum; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_getCseqNumFromRAckHdr");
	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_rpr_setCseqNumInRAckHdr
**
** DESCRIPTION: This function sets the Cseq field in the RAck pHeader
**
*****************************************************************/
SipBool sip_rpr_setCseqNumInRAckHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_U32bit cSeq, SipError *pErr)
#else
	(pHdr, cSeq, pErr)
	SipHeader *pHdr;
	SIP_U32bit cSeq;
	SipError *pErr;
#endif
{
	SIPDEBUGFN("Entering function sip_rpr_setCseqNumInRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* setting the Cseq pValue in the structure */
	((SipRackHeader *)(pHdr->pHeader))->dCseqNum = cSeq; 
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_setCseqNumNumInRAckHdr");
	return SipSuccess;
}


/****************************************************************
**
** FUNCTION: sip_rpr_getMethodFromRAckHdr
**
** DESCRIPTION: This function retrives the pMethod field from the 
** RAck pHeader.
**
*****************************************************************/
SipBool sip_rpr_getMethodFromRAckHdr 
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit **ppMethod, SipError *pErr)
#else
	(pHdr, ppMethod, pErr)
	SipHeader *pHdr;
	SIP_S8bit **ppMethod;
	SipError *pErr;
#endif
{
	SIP_S8bit *pTempMethod;
	SIPDEBUGFN("Entering function sip_rpr_getMethodFromRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( ppMethod == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif
	/* the pValue in the structure */
 	pTempMethod = ((SipRackHeader *)(pHdr->pHeader))->pMethod;
 	if (pTempMethod == SIP_NULL)
 	{
 		*pErr = E_NO_EXIST;
 		return SipFail;
 	}
 	
#ifdef SIP_BY_REFERENCE
	*ppMethod = pTempMethod;
#else
	/* duplicating */
 	*ppMethod = (SIP_S8bit *) STRDUPACCESSOR (pTempMethod);
	if (*ppMethod == SIP_NULL)
	{
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif
	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_getMethodFromRAckHdr");
 	return SipSuccess;
}

/****************************************************************
**
** FUNCTION: sip_rpr_setMethodInRAckHdr
**
** DESCRIPTION: This function sets the pMethod field in the RAck pHeader.
**
*****************************************************************/
SipBool sip_rpr_setMethodInRAckHdr
#ifdef ANSI_PROTO
	(SipHeader *pHdr, SIP_S8bit *pMethod, SipError *pErr)
#else
	(pHdr, pMethod, pErr)
	SipHeader *pHdr;
	SIP_S8bit *pMethod;
	SipError *pErr;
#endif
{
#ifndef SIP_BY_REFERENCE
	SIP_S8bit *pTempMethod;
#endif
	SIP_S8bit *pMthd;
    SIPDEBUGFN("Entering function sip_rpr_setMethodInRAckHdr");
#ifndef SIP_NO_CHECK
	if( pErr == SIP_NULL)
		return SipFail;
	if( pHdr == SIP_NULL)
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	if( pHdr->dType != SipHdrTypeRAck)
	{
		*pErr = E_INV_TYPE;
		return SipFail;
	}
	if( pHdr->pHeader == SIP_NULL)
	{
		*pErr = E_INV_HEADER;
		return SipFail;
	}
#endif        
    pMthd = ((SipRackHeader *)(pHdr->pHeader))->pMethod;
                
	/* freeing original pValue of it exists */
	if ( pMthd != SIP_NULL)
	{
		if (sip_memfree(ACCESSOR_MEM_ID, (SIP_Pvoid*)(&pMthd), pErr) == SipFail)
			return SipFail;
	}
    
#ifdef SIP_BY_REFERENCE
	((SipRackHeader *)(pHdr->pHeader))->pMethod = pMethod;
#else
    if( pMethod == SIP_NULL)
                pTempMethod = SIP_NULL;
    else
    {
		pTempMethod = (SIP_S8bit *) STRDUPACCESSOR(pMethod);
		if (pTempMethod == SIP_NULL)
		{
			*pErr = E_NO_MEM;
			return SipFail;
		}
	}	
        
    
	/* assigning the new pValue to the structure field */
	((SipRackHeader *)(pHdr->pHeader))->pMethod = pTempMethod;
#endif	
	*pErr = E_NO_ERROR;
	SIPDEBUGFN("Exiting function sip_rpr_setMethodInRAckHdr");
    return SipSuccess;
}



