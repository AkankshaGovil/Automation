/**************************************************************************
** FUNCTION:
** This has prototypes for creating Mid-Way Transactions 
**
***************************************************************************
**
** FILENAME:
**  txnmidway.h
**
** DESCRIPTION
**
**
**  DATE           NAME                       REFERENCE
** 18Feb2002  	sasidhar P V K			 			Initial
**
**
** Copyright 2002, Hughes Software Systems, Ltd.
***************************************************************************/

#ifndef TXN_MIDWAY_H
#define TXN_MIDWAY_H

#include "siptxnstruct.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif 

/***************************************************************************
**** FUNCTION:sip_txn_createMidwayTxn
**** Description:The application will use this API to create a Txn by giving
****		 the SipMessage.This API does not  perform the check of whether
****		dependent Txn is present or not.This will immediately Create
****		 New Txn If there is Nothing. 
******************************************************************************/
SipBool sip_txn_createMidwayTxn(SipMessage *pMessage, SipTranspAddr *pAddr,\
	SIP_S8bit dTranspType, SipTxnContext *pTxnContext,\
	en_SipTxnAPICalled dAPICalled, SipTxnKey **ppTxnKey, SipError *pError);
/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif 

#endif
