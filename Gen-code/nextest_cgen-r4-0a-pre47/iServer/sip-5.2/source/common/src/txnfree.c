/************************************************************************
 ** FUNCTION:
 **             This file has all the SIP Txn Related free Functions

 ************************************************************************
 **
 ** FILENAME:
 ** txnfree.c
 **
 ** DESCRIPTION:
 **
 ** DATE                NAME                    REFERENCE               REASON
 ** ----                ----                    --------                ------
 ** 09-02-2002			Kamath										Creation
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *************************************************************************/

#include "siptxnstruct.h"
#include "siptxntimer.h"
#include "rprfree.h"
#include "portlayer.h"
#include "txnfree.h"


#define FREE_TIMER_HANDLE(a)	sip_freeTimerHandle(a)

/*******************************************************************************
**** FUNCTION:sip_freeSipTxnKey
****
****
**** DESCRIPTION:This fn frees up the Txn Key
**************************************************************************/
void  sip_freeSipTxnKey
#ifdef ANSI_PROTO
	(SipTxnKey *pKey)
#else
	(pKey)
	SipTxnKey *pKey;
#endif
{
	if(pKey == SIP_NULL) return;
 	HSS_LOCKREF(pKey->dRefCount);
	HSS_DECREF(pKey->dRefCount);
    if(HSS_CHECKREF(pKey->dRefCount))
    {
		HSS_FREE(pKey->pMethod);
		HSS_FREE(pKey->pToTag);
		HSS_FREE(pKey->pFromTag);
		
		HSS_FREE(pKey->pViaBranch);
		HSS_FREE(pKey->pCallid);
		if(pKey->pRackHdr)
		sip_rpr_freeSipRAckHeader(pKey->pRackHdr);
		HSS_DELETEREF(pKey->dRefCount);
		HSS_FREE(pKey);
	}
	else
	{
		HSS_UNLOCKREF(pKey->dRefCount);
	}
}

/*******************************************************************************
**** FUNCTION:sip_freeSipTxnBuffer
****
****
**** DESCRIPTION:This fn frees up the TxnBuffer
******************************************************************************/
void sip_freeSipTxnBuffer
#ifdef ANSI_PROTO
	(SipTxnBuffer *pBuffer)
#else
	(pBuffer)
	SipTxnBuffer *pBuffer;
#endif
{
	SipError err;
	if(pBuffer == SIP_NULL) return;
 	HSS_LOCKREF(pBuffer->dRefCount);HSS_DECREF(pBuffer->dRefCount);
    if(HSS_CHECKREF(pBuffer->dRefCount))
    {
		if (!pBuffer->dDirectOption)
			HSS_FREE(pBuffer->pBuffer);
			
		sip_listDeleteAll(pBuffer->pslMesgQ,&err);
		HSS_FREE(pBuffer->pslMesgQ);
		sip_listDeleteAll(pBuffer->pslRPRTxnKeyQ,&err);
		HSS_FREE(pBuffer->pslRPRTxnKeyQ);
		
		if (pBuffer->pTimerHandle!=SIP_NULL)
			FREE_TIMER_HANDLE(pBuffer->pTimerHandle);
			
		if(pBuffer->pAddr)
		{
			if (pBuffer->pAddr->pHost)
				HSS_FREE(pBuffer->pAddr->pHost);
			HSS_FREE(pBuffer->pAddr);			
		}
		HSS_DELETEREF(pBuffer->dRefCount);
		HSS_FREE(pBuffer);
	}
	else
	{		
		HSS_UNLOCKREF(pBuffer->dRefCount);
	}	
}
/*******************************************************************************
**** FUNCTION:sip_freeSipTxnTimeoutData
****
****
**** DESCRIPTION:This function frees up the SipTxnTimeoutData internally
****			it frees up the SipTxnKey	
*******************************************************************************/
void sip_freeSipTxnTimeoutData
#ifdef ANSI_PROTO
        (SipTxnTimeoutData *pTxnTimeoutData)
#else
        (pTxnTimeoutData)
	SipTxnTimeoutData *pTxnTimeoutData;
#endif
{
    if (pTxnTimeoutData == SIP_NULL) return;
	sip_freeSipTxnKey(pTxnTimeoutData->pKey);
	HSS_FREE(pTxnTimeoutData);
}


/*******************************************************************************
**** FUNCTION:sip_txn_freeSipRAckHeader
****
****
**** DESCRIPTION:This function frees up the RAck Header
*******************************************************************************/
void sip_txn_freeSipRAckHeader
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
void __sip_freeSipTxnKey(SIP_Pvoid pTxnKey)
{
	sip_freeSipTxnKey((SipTxnKey*)pTxnKey);
}

