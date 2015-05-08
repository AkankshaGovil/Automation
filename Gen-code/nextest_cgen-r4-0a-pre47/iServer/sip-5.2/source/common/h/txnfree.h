/************************************************************************
 ** FUNCTION:
 **             This file has all the SIP Txn Related Freeing Functions

 ************************************************************************
 **
 ** FILENAME:
 ** siptxnfree.h
 **
 ** DESCRIPTION:This has structures related to Transaction Layer
 **
 ** DATE                NAME                    REFERENCE               REASON
 ** ----                ----                    --------                ------
 **13-Feb-2002			P.V.K.Sasidhar								Initial Crn
 **
 **     Copyright 2002, Hughes Software Systems, Ltd.
 *************************************************************************/


#ifndef __SIPTXNFREE_H__
#define __SIPTXNFREE_H__

#include "sipstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
**FUNCTION: sip_freeSipTxnKey
**
**DESCRIPTION: This function frees up the SipTxnKey
**PARAMETERS: pTxnKey(IN): This is the reference to the SipTxnKey 
**				freed
**
***********************************************************************/
void sip_freeSipTxnKey(SipTxnKey* pTxnKey);

/***********************************************************************
**FUNCTION: sip_freeSipTxnTimeoutData
**
**DESCRIPTION: This function frees the wrapper of the SipTxnKey, internally
**				it calls the sip_freeSipTxnKey() API
**
**PARAMETERS: pTxnTimeoutData(IN): The SipTxnTimeoutData reference that needs to be freed
**
***********************************************************************/
void sip_freeSipTxnTimeoutData(SipTxnTimeoutData* pTxnTimeoutData);

/***********************************************************************
**FUNCTION: sip_freeSipTxnBuffer
**
**DESCRIPTION: This fn frees up the TxnBuffer
**
**PARAMETERS: pTxnBuffer(IN):This frees up the SipTxnBuffer
**
***********************************************************************/
void sip_freeSipTxnBuffer(SipTxnBuffer* pTxnBuffer);

/***********************************************************************
**FUNCTION: sip_txn_freeSipRAckHeader
**
**DESCRIPTION:This fn frees up the SiprackHeader
**PARAMETERS: pHeader(IN):The reference to the RAck header that needs to be
**				freed.
**
***********************************************************************/
void sip_txn_freeSipRAckHeader(SipRackHeader* pHeader);


extern void __sip_freeSipTxnKey(SIP_Pvoid pTxnKey);

#ifdef __cplusplus
}
#endif

#endif
