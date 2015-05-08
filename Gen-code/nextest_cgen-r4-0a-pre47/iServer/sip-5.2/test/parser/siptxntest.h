/**************************************************************
** FUNCTION:
**			Contains prototypes for Timer Related Functions
***************************************************************
** FILENAME:
** siptxntest.h
**
** DESCRIPTION:
**
** DATE			NAME			REFERENCE		REASON
** ----			----			---------		------
** 25/09/2002	Kamath and Jyoti		    -			Initial Creation
**
** Copyright 2002, Hughes Software Systems, Ltd.
**************************************************************/
/* Ensure Names are not Mangled by C++ Compilers */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TXN_TEST_H__
#define __TXN_TEST_H__

#include <stdlib.h>
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
#include <pthread.h>
#endif

#include "sipcommon.h"
#include "portlayer.h"
#include "siptxnstruct.h"
#include "txnfree.h"

#if defined(SIP_VXWORKS) || defined(SIP_OSE)
#define NUM_BUCKET  100
#else
#define NUM_BUCKET	1000
#endif
#define GRANULARITY 100
#define MAX_TIMEOUT 32000
#define TXN_CREAT	1
#define TXN_REMOVE	2

typedef SipBool (*CompareKeyFuncPtr)(void*,void*,SipError*);

typedef struct SipTimerHandle
{
    SIP_U32bit dHandle;
    SIP_U32bit dIndex;
}SipTimerHandle;

typedef struct SipTimerElem
{
    struct SipTimerElem* pRight;
    struct SipTimerElem* pLeft;
    SIP_U32bit dHandle;
    SIP_U32bit dDuration;
    SIP_U32bit dRevol;
    SIP_S8bit dIsHead;
/*  SIP_S8bit dToBeDeleted; */
	SipTimerHandle *pTimerHandle;
    TimeoutFuncPtr timeoutfunc;
    SIP_Pvoid pBuffer;
}SipTimerElem;

typedef struct SipTimerWheel
{
    SipTimerElem* pWheel;
    SIP_U32bit dGranularity;  /* number of msecs per tick */
    SIP_U32bit dCurrentIndex; /* index to the current entry in the wheel */
    SIP_U32bit dWheelSize;	   /* Total number of entries in the wheel */
    SIP_U32bit dHandle;       /* Increasing handle value */
#ifdef SIP_THREAD_SAFE
	synch_id_t *pTimerWheelMutex;
    synch_id_t HandleMutex;
#endif	
}SipTimerWheel;

typedef struct SipTxnHashElem
{
	SIP_Pvoid pBuffer; /* Stores the Actual Data*/
	SIP_Pvoid pKey;    /* Stores the Key for Txn HashTbl */
	SIP_Pvoid pContext;
	struct SipTxnHashElem* pRight;
	struct SipTxnHashElem* pLeft;
}SipTxnHashElem;

typedef struct SipTxnHashTbl
{
	SipTxnHashElem* pHashtbl;

	TimeoutFuncPtr hashcalc; /* To Calculate the Hashvalue */
	CompareKeyFuncPtr keycmpr;  /* To compare the keys */
	SIP_U32bit dTxnElemCount; /* Counter for number of Txn Elements */
#ifdef SIP_THREAD_SAFE	
	synch_id_t CounterMutex;
	synch_id_t HashMutex[NUM_BUCKET+1];
#endif	
}SipTxnHashTbl;
/***************************************************************************
* FUNCTION: sip_initHashTbl
* Parameters
*
* keycmpr(IN):		Function pointer to KeyCompare function
****************************************************************************/
extern SipBool sip_initHashTbl( CompareKeyFuncPtr keycmpr );
/***************************************************************************
* FUNCTION: sip_initTimerWheel
* Parameters
*
* dGranuality(IN) :		Granuality of the Timer Wheel(in msec)
* dMaxTimeout(IN) :		Max TimeOut Value(in msec)
****************************************************************************/
extern SipBool sip_initTimerWheel( SIP_U32bit dGranuality,\
	SIP_U32bit dMaxTimeout);
/***************************************************************************
* FUNCTION: __sip_removeTimerElem
* Parameters
*
* pTimerElem(IN):		Timer Element to be removed
****************************************************************************/
SIP_S32bit __sip_removeTimerElem( SipTimerElem* pTimerElem);
#if !defined(SIP_VXWORKS) && !defined(SIP_WINDOWS)
SIP_Pvoid sip_timerThread(SIP_Pvoid pTWheel);
#endif

/***************************************************************************
* FUNCTION: __sip_sleep
* Parameters
*
* dMicrosecs(IN)	:	Sleep duration in MicroSeconds
****************************************************************************/
void __sip_sleep(SIP_U32bit dMicrosecs);
/***************************************************************************
* FUNCTION: __sip_flushTimer
* Parameters
*
* None
****************************************************************************/
void __sip_flushTimer(void);
/***************************************************************************
* FUNCTION: __sip_flushHash
* Parameters
*
* None
****************************************************************************/
void __sip_flushHash(void);
/***************************************************************************
* FUNCTION: __sip_getHandle
* Parameters
*
* pHandle(OUT)		:	Handle will be returned with this variable
****************************************************************************/
SIP_S32bit __sip_getHandle(SIP_U32bit* pHandle);
/***************************************************************************
* FUNCTION: sip_freeTimerWheel
* Parameters
*
* None
****************************************************************************/
extern void sip_freeTimerWheel(void);
/***************************************************************************
* FUNCTION: sip_freeHashTable
* Parameters
*
* None
****************************************************************************/
extern void sip_freeHashTable(void);

# if 0
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
# endif /* if 0*/

/***************************************************************************
* FUNCTION: __sip_hashCalc
* Parameters
*
* pKey(IN)			:	Key used for calculating the Hash Value
****************************************************************************/
SIP_U32bit __sip_hashCalc(SIP_Pvoid pKey);


/***************************************************************************
* FUNCTION: sip_getAllTxnElem
* Parameters
*
* ppTxnKeys(OUT)	:	Keys of all the Txn Elements will be returned
* pCount(OUT)		:	Count of Txn Elements
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool sip_getAllKeysOfTxnElem(SipTxnKey** ppTxnKeys,\
		SIP_U32bit* pCount,SipError* pErr);

/***************************************************************************
* FUNCTION: sip_getBranchFromViaHdr
* Parameters
*
* pViaHeader(IN)	:	Via Header from which branch is extracted
* ppBranch(OUT)		:	Branch from Via Hdr
* pErr(OUT)			:	Error variable
****************************************************************************/
extern SipBool sip_getBranchFromViaHdr _ARGS_((SipViaHeader *pViaHeader, \
	SIP_S8bit **ppBranch, SipError *pErr));

#endif

#ifdef __cplusplus
}
#endif
