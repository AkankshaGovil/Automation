/**************************************************************
** FUNCTION:
**			Contains prototypes for Timer Related Functions
***************************************************************
** FILENAME:
** siptxntimer.h
**
** DESCRIPTION:
**
** DATE			NAME			REFERENCE		REASON
** ----			----			---------		------
** 07/02/2002	Mohan KR		    -			Initial Creation
**
** Copyright 2002, Hughes Software Systems, Ltd.
**************************************************************/

#ifndef __TXN_TIMER_H__
#define __TXN_TIMER_H__

#include <stdlib.h>
#include "sipcommon.h"
#include "portlayer.h"
#include "siptxnstruct.h"

/* Ensure Names are not Mangled by C++ Compilers */
#ifdef __cplusplus
extern "C" {
#endif

#define TXN_CREAT	1
#define TXN_REMOVE	2


/***************************************************************************
* FUNCTION: fast_startTxnTimer
* Parameters
*
* dDuration(IN)		:	Duration of the Timer
* timeOutFuncPtr(IN):	TimeOut function to be invoked on Timer Expiry
* pData(IN)			:	Buffer to be stored
* ppHandle(OUT)		:	Handle to the timer, to be used with
*							fast_stopTxnTimer
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool fast_startTxnTimer(SIP_U32bit dDuration,\
	TimeoutFuncPtr timeOutFuncPtr, SIP_Pvoid pData, SIP_Pvoid* ppHandle,\
	SipError *pErr);

/***************************************************************************
* FUNCTION: fast_stopTxnTimer
* Parameters
*
* pInkey(IN)		:	Handle to the Timer that is to be stoped
* ppBuff(OUT)		:	Buffer that was stored during the fast_startTxnTimer
*							will be returned
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool fast_stopTxnTimer(SIP_Pvoid pInkey,SIP_Pvoid* ppBuff,\
	SipError* pErr);

/***************************************************************************
* FUNCTION: sip_cbk_fetchTxn
* Parameters
*
* ppTxnBuffer(OUT)	:	Txn Buffer pointer will be returned with
*						this variable
* pTxnKey(IN)		:	Txn Key for fetching the Txn Element
* pContext(IN)		:	Extra value being stored
* dOpt(IN)			:	When set to O_CREAT, new Txn Element will be
* 						created if it can't find one
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool sip_cbk_fetchTxn(SIP_Pvoid* ppTxnBuffer,SIP_Pvoid pTxnKey, \
			SIP_Pvoid pContext,SIP_S8bit dOpt,SipError *pErr);

/***************************************************************************
* FUNCTION: sip_cbk_releaseTxn
* Parameters
*
* pTxnKey(IN)		:	Key to the Element
* ppTxnKey(OUT)		:	Key of the Element being released
* ppTxnBuffer(OUT)	:	Buffer of the Txn Element being released
* dOpt(IN)			:	When set to O_REMOVE, Txn Element will be deleted
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool sip_cbk_releaseTxn(SIP_Pvoid pTxnKey, SIP_Pvoid* ppTxnKey,\
			SIP_Pvoid *ppTxnBuffer, SIP_Pvoid pContextData, SIP_S8bit dOpt,\
			SipError *pErr);

/***************************************************************************
* FUNCTION: sip_freeTimerHandle
* Parameters
*
* pTimerHandle(OUT)			:	SIP_Pvoid timer handle
****************************************************************************/
extern void sip_freeTimerHandle(SIP_Pvoid pTimerHandle);


#ifdef __cplusplus
}
#endif

#endif
