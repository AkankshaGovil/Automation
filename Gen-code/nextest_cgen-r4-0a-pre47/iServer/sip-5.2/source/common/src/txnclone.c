/************************************************************************
 ** FUNCTION:
 **             This file has all the SIP Txn Related clone Functions

 ************************************************************************
 **
 ** FILENAME:
 ** txnclone.c
 **
 ** DESCRIPTION:This file comprises of the clone functions necessary for the
 **				transaction layer.
 **
 ** DATE                NAME                    REFERENCE               REASON
 ** ----                ----                    --------                ------
 ** 11-Feb-2002			Sasidhar PVK	
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *************************************************************************/

#include "siptxnstruct.h"
#include "portlayer.h"
#include "txnclone.h"
#include "txnfree.h"
#include "rprinit.h"
#include "sipfree.h"

#define CLONE(x,y) \
{ \
	if(y != SIP_NULL) \
 	{\
	if(x != SIP_NULL)\
        {\
            fast_memfree(ACCESSOR_MEM_ID, x,NULL);\
            x = SIP_NULL;\
        }\
	x = (SIP_S8bit *)STRDUPACCESSOR(y);\
	if(x == SIP_NULL)\
	{\
		mem_flag = 1;break;\
	}\
	}\
	if(y == SIP_NULL)\
	{\
		x = SIP_NULL;\
	}\
} 



/***********************************************************************
**FUNCTION: sip_txn_rprcloneSipRAckHeader
**
**DESCRIPTION: This function achieves the cloning of the RACk header.
**
***********************************************************************/
SipBool sip_txn_rprcloneSipRAckHeader
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
        SIPDEBUGFN("Entering function sip_txn_rprcloneSipRAckHeader");
		
		/*Check for the validity of the i/p params*/
		if (pErr==SIP_NULL)
		{
			return SipFail;
		}

		if ((pDest==SIP_NULL)||(pSource==SIP_NULL))
		{
			*pErr=E_INV_PARAM;
			return SipFail;
		}
		
		*pErr=E_NO_ERROR;
		
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
        SIPDEBUGFN("Exiting function sip_txn_rprcloneSipRAckHeader");
        return SipSuccess;
}


/*******************************************************************
**** FUNCTION:sip_cloneSipTxnKey
****
****
**** DESCRIPTION:This function clones the TxnKey
****
********************************************************************/
SipBool sip_cloneSipTxnKey
#ifdef ANSI_PROTO
	(SipTxnKey *pDestKey, SipTxnKey *pSrcKey, SipError *pErr)
#else
	(pDestKey,pSrcKey,pErr)
	SipTxnKey *pDestKey;
	SipTxnKey *pSrcKey;
	SipError *pErr;
#endif
{
	SIP_S8bit mem_flag = 0;

	/*Check for the validity of the i/p params*/
	if (pErr==SIP_NULL)
	{
		return SipFail;
	}

	if ((pDestKey==SIP_NULL)||(pSrcKey==SIP_NULL))
	{
		*pErr=E_INV_PARAM;
		return SipFail;
	}
		
	*pErr=E_NO_ERROR;
	
	pDestKey->dCSeq = pSrcKey->dCSeq;
	pDestKey->dRseq = pSrcKey->dRseq;
	pDestKey->dTagCheck = pSrcKey->dTagCheck;
	pDestKey->dViaBranchCheck = pSrcKey->dViaBranchCheck;
	pDestKey->dType = pSrcKey->dType;
	pDestKey->dTxnClass = pSrcKey->dTxnClass;
	pDestKey->dViaHdrCount=pSrcKey->dViaHdrCount;
	pDestKey->dRespCodeNum = pSrcKey->dRespCodeNum;
	
	do
	{
		CLONE(pDestKey->pCallid,pSrcKey->pCallid)
		CLONE(pDestKey->pMethod,pSrcKey->pMethod)
		CLONE(pDestKey->pToTag,pSrcKey->pToTag)
		CLONE(pDestKey->pFromTag,pSrcKey->pFromTag)
		CLONE(pDestKey->pViaBranch,pSrcKey->pViaBranch)

		/* clone RAck Header */
		if(pSrcKey->pRackHdr)
		{
			if(sip_rpr_initSipRAckHeader(&((pDestKey)->pRackHdr),\
                                 pErr)==SipFail)
                		return SipFail;
			if(sip_txn_rprcloneSipRAckHeader(pDestKey->pRackHdr, \
			pSrcKey->pRackHdr,pErr) == SipFail)
			{
				mem_flag = 1;break;
			}
		}
		else
		{
			pDestKey->pRackHdr = SIP_NULL;	
		}
	}while(0);

	if(mem_flag)
	{
		sip_freeSipTxnKey(pDestKey);
		*pErr = E_NO_MEM;	
		return SipFail;
	}	
	return SipSuccess;
}
