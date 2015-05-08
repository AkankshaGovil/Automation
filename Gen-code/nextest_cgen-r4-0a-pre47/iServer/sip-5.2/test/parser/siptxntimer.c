/***************************************************************
** FUNCTION:
**			Manages Timer and Transaction Elements
**
****************************************************************
** FILENAME:
** siptxntimer.c
**
** Description	:Implementation for All the Timer Related
**					Functions
**
** DATE			NAME		REFERENCE		REASON
** ----			----		---------		------
** 07/02/2002	Mohan KR	    -			Initial Creation
**
** Copyright 2002, Hughes Software Systems, Ltd.
***************************************************************/
/* Ensure Names are not Mangled by C++ Compilers */
#ifdef __cplusplus
extern "C" {
#endif

#include "siptxntimer.h"
#include "siptxntest.h"
#include "txnclone.h"
#include "portlayer.h"
#include "txnfree.h"



SipTimerWheel glbtimerWheel;
SipTxnHashTbl glbhashTable;


/*******************************************************
*  FUNCTION    : sip_initTimerWheel
*
*  DESCRIPTION : Initialization of Timer Wheel is done
*                here.
********************************************************/
SipBool sip_initTimerWheel
#ifdef ANSI_PROTO
    (
    SIP_U32bit dGranuality,
    SIP_U32bit dMaxTimeout
    )
#else
    (dGranuality,dMaxTimeout)
    SIP_U32bit dGranuality;
    SIP_U32bit dMaxTimeout;
#endif
{
    SIP_U32bit dI;
	SipError err;

    if ( dMaxTimeout != 0 && dGranuality != 0 )
    {
        glbtimerWheel.dWheelSize = ( dMaxTimeout / dGranuality) + \
		    ((dMaxTimeout%dGranuality)?1:0);
        glbtimerWheel.dGranularity = dGranuality;
    }
    else
    {
		glbtimerWheel.dWheelSize = ( MAX_TIMEOUT / GRANULARITY )+1;
		glbtimerWheel.dGranularity = GRANULARITY;
    }

    glbtimerWheel.dCurrentIndex = 0;
	glbtimerWheel.dHandle = 0;

    glbtimerWheel.pWheel = (SipTimerElem*)fast_memget(TIMER_MEM_ID,
		sizeof(SipTimerElem) * glbtimerWheel.dWheelSize,&err);
    if (glbtimerWheel.pWheel == SIP_NULL)
	{
		return SipFail;
	}

	glbtimerWheel.pTimerWheelMutex = (synch_id_t*)fast_memget(\
		TIMER_MEM_ID,sizeof(synch_id_t)*glbtimerWheel.dWheelSize,\
		&err);
	if ( glbtimerWheel.pTimerWheelMutex == SIP_NULL )
	{
		fast_memfree(TIMER_MEM_ID,glbtimerWheel.pWheel,NULL);
		return SipFail;
	}
    /* intialize all wheel chains */
    for (dI=0; dI < (glbtimerWheel.dWheelSize); dI++)
    {
		glbtimerWheel.pWheel[dI].pRight = &glbtimerWheel.pWheel[dI];
        glbtimerWheel.pWheel[dI].pLeft = &glbtimerWheel.pWheel[dI];
		glbtimerWheel.pWheel[dI].dIsHead = 1;
    }

    /* new - initialize all the mutexes */
    for (dI=0; dI < (glbtimerWheel.dWheelSize); dI++)
    {
		fast_init_synch(&glbtimerWheel.pTimerWheelMutex[dI]);
    }

    fast_init_synch(&glbtimerWheel.HandleMutex);

    return SipSuccess;
}

/*******************************************************
*  FUNCTION    : sip_initHashTbl
*
*  DESCRIPTION : Initialization of Hash Table is done
*                here.
********************************************************/
SipBool sip_initHashTbl
#ifdef ANSI_PROTO
    (
	CompareKeyFuncPtr keycmpr
    )
#else
    (keycmpr)
    CompareKeyFuncPtr keycmpr;
#endif
{
    SIP_U32bit dI;
	SipError err;

    glbhashTable.pHashtbl = (SipTxnHashElem*)fast_memget(TIMER_MEM_ID,
		sizeof(SipTxnHashElem) * (NUM_BUCKET+1),&err);
    if (glbhashTable.pHashtbl == SIP_NULL)
		return SipFail;

	glbhashTable.keycmpr = keycmpr;
	glbhashTable.dTxnElemCount = 0;
	fast_init_synch(&glbhashTable.CounterMutex);

    /* intialize all Hash chains */
    for (dI=0; dI < (NUM_BUCKET+1); dI++)
    {
		glbhashTable.pHashtbl[dI].pRight = &glbhashTable.pHashtbl[dI];
        glbhashTable.pHashtbl[dI].pLeft = &glbhashTable.pHashtbl[dI];
		glbhashTable.pHashtbl[dI].pKey = SIP_NULL;
		glbhashTable.pHashtbl[dI].pBuffer = SIP_NULL;
	}

    /* new - initialize all the mutexes */
    for (dI=0; dI < (NUM_BUCKET+1); dI++)
    {
		fast_init_synch(&glbhashTable.HashMutex[dI]);
    }

    return SipSuccess;
}
/*******************************************************
*  FUNCTION    : __sip_removeTimerElem
*
*  DESCRIPTION : Remove the Timer Element from the
*				 Timer Wheel.
********************************************************/

SIP_S32bit __sip_removeTimerElem
#ifdef ANSI_PROTO
    ( SipTimerElem* pElem )
#else
    (pElem)
    SipTimerElem* pElem;
#endif
{
    pElem->pRight->pLeft = pElem->pLeft;
    pElem->pLeft->pRight = pElem->pRight;
    fast_memfree(TIMER_MEM_ID,pElem,SIP_NULL);

    return 0;
}
/*******************************************************
*  FUNCTION    : fast_startTxnTimer
*
*  DESCRIPTION : Start the Timer.
*
********************************************************/

SipBool fast_startTxnTimer
#ifdef ANSI_PROTO
    (
    SIP_U32bit dDuration,
    TimeoutFuncPtr funcptr,
    SIP_Pvoid pData,
    SIP_Pvoid* ppHandle,
    SipError *pErr
    )
#else
    (dDuration,funcptr,pData,ppHandle,pErr)
    SIP_U32bit dDuration;
    TimeoutFuncPtr funcptr;
    SIP_Pvoid pData;
    SIP_Pvoid* ppHandle;
    SipError *pErr;
#endif
{
    SipTimerElem *pTlastElem=SIP_NULL;
	SipTimerElem *pElemTimer=SIP_NULL;
    SIP_U32bit dOffset,dIndex;
    SIP_U32bit dHandle;
    SipTimerHandle *pTimerHandle=SIP_NULL;
	SIP_S8bit traceBuf[200];

	HSS_SPRINTF (traceBuf,"####### Timer Duration: %d #######\n",dDuration);
	SIPDEBUG(traceBuf);

    /* This memory will be released in the fast_stopTxnTimer or in
       the processTimeout function */
    pTimerHandle = (SipTimerHandle *) fast_memget(TIMER_MEM_ID,\
				sizeof(SipTimerHandle),pErr);
    if ( pTimerHandle == SIP_NULL )
    {
		*ppHandle = SIP_NULL;
        *pErr = E_NO_MEM;
        return SipFail;
    }

    dOffset = (dDuration / glbtimerWheel.dGranularity)\
	    + ((dDuration % glbtimerWheel.dGranularity)?1:0);
    dIndex = ( glbtimerWheel.dCurrentIndex + dOffset) % \
	    glbtimerWheel.dWheelSize;

    pElemTimer = (SipTimerElem*) fast_memget(TIMER_MEM_ID,\
				sizeof (SipTimerElem),pErr);
    if (pElemTimer == SIP_NULL)
    {
		fast_memfree(TIMER_MEM_ID,pTimerHandle,SIP_NULL);
		*ppHandle = SIP_NULL;
		*pErr = E_NO_MEM;
		return SipFail;
    }
    pElemTimer->dIsHead = 0;

    pElemTimer->pBuffer = pData;
    __sip_getHandle(&dHandle);
    pElemTimer->dHandle = dHandle;
    pElemTimer->timeoutfunc = funcptr;
    pElemTimer->dDuration = dDuration;
    pElemTimer->dRevol = dOffset / glbtimerWheel.dWheelSize;
	pElemTimer->pTimerHandle = pTimerHandle;


    pTimerHandle->dHandle = dHandle;
    /*pTimerHandle->pTimerElem = pElemTimer;*/
    pTimerHandle->dIndex = dIndex;
    *ppHandle = pTimerHandle;

    fast_lock_synch(0,&glbtimerWheel.pTimerWheelMutex[dIndex],0);

    pTlastElem = glbtimerWheel.pWheel[dIndex].pLeft;
    pElemTimer->pRight = pTlastElem->pRight;
    pElemTimer->pLeft = pTlastElem;
    pTlastElem->pRight->pLeft = pElemTimer;
    pTlastElem->pRight = pElemTimer;

    fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex[dIndex]);

    *pErr = E_NO_ERROR;
    return SipSuccess;
}
/*******************************************************
*  FUNCTION    : fast_stopTxnTimer
*
*  DESCRIPTION : Stop the Timer.
*
********************************************************/

SipBool fast_stopTxnTimer
#ifdef ANSI_PROTO
    ( SIP_Pvoid pHandle,SIP_Pvoid* ppBuff,SipError* pErr )
#else
    (pHandle,ppBuff,pErr)
    SIP_Pvoid pHandle;
    SIP_Pvoid* ppBuff;
    SipError* pErr;
#endif
{
    SIP_U32bit dInWheelIndex;
    SipTimerElem *pHead=SIP_NULL;
	SipTimerElem *pCur=SIP_NULL;
	SipTimerElem *pTimerElem = SIP_NULL;

	/* If the Handle is NULL there is no timer to be stopped */
	if ( pHandle == SIP_NULL )
	{
		return SipSuccess;
	}

    dInWheelIndex = ((SipTimerHandle*)pHandle)->dIndex;

    fast_lock_synch(0,&glbtimerWheel.pTimerWheelMutex[dInWheelIndex],0);
    pHead = &glbtimerWheel.pWheel[dInWheelIndex];
    pCur = pHead->pRight;
    while ( pCur->dIsHead != 1 )
    {
		if ( pCur->dHandle == ((SipTimerHandle*)pHandle)->dHandle )
		{
		    pTimerElem = pCur;
	    	break;
		}
		pCur = pCur->pRight;
    }
    if(pTimerElem != SIP_NULL)
    {
		*ppBuff = pTimerElem->pBuffer;
		/* remove from the timer wheel */
		__sip_removeTimerElem(pTimerElem);
		fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex[dInWheelIndex]);
		fast_memfree(TIMER_MEM_ID,pHandle,SIP_NULL);
		*pErr = E_NO_ERROR;
		return SipSuccess;
    }
    fast_memfree(TIMER_MEM_ID,pHandle,SIP_NULL);
    fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex[dInWheelIndex]);
    *pErr = E_INV_PARAM;
    return SipFail;
}

/*******************************************************
*  FUNCTION    : sip_timerThread
*
*  DESCRIPTION : Timer thread Removes the TimedOut
*                Elements from the Timer.
********************************************************/
SIP_Pvoid sip_timerThread

#ifdef ANSI_PROTO
    (SIP_Pvoid pTWheel)
#else
    (pTWheel)
    SIP_Pvoid pTWheel;
#endif

{
    struct timeval tv1, tv2;
	if ( pTWheel != SIP_NULL )
		pTWheel = SIP_NULL;
    while(1)
    {
		SIP_U32bit dElapsed_time;

        SipTimerElem *pTimerWheelElem=SIP_NULL;
		SipTimerElem *pHead=SIP_NULL;
		SipTimerElem *pRef=SIP_NULL;
        gettimeofday(&tv1, SIP_NULL);

        /* if any timers are due on current time */
        pHead = &glbtimerWheel.pWheel[glbtimerWheel.dCurrentIndex];
        pRef = pHead;
        while (1)
        {
		    TimeoutFuncPtr tempfunc;
			SIP_Pvoid pTempbuffer;

		    fast_lock_synch(0,&glbtimerWheel.pTimerWheelMutex\
				[glbtimerWheel.dCurrentIndex],0);

            pTimerWheelElem = pHead->pRight;
		    if ( pTimerWheelElem->dIsHead == 1 )
            {
				/* End of wheel chain */
                fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex\
		    		[glbtimerWheel.dCurrentIndex]);
                break;
		    }
            if ( pTimerWheelElem->dRevol >= 1 )
            {
				fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex\
		    		[glbtimerWheel.dCurrentIndex]);
				pTimerWheelElem->dRevol--;
            	pHead = pHead->pRight;
                continue;
	    	}

		    tempfunc = pTimerWheelElem->timeoutfunc;
	    	pTempbuffer = pTimerWheelElem->pBuffer;
			fast_memfree(TIMER_MEM_ID,pTimerWheelElem->pTimerHandle,SIP_NULL);
	    	__sip_removeTimerElem(pTimerWheelElem);
            pHead = pHead->pRight;
		    fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex\
				[glbtimerWheel.dCurrentIndex]);

			tempfunc(pTempbuffer);

		}/* end of inner while loop */

		gettimeofday(&tv2, SIP_NULL);

		/* Get time elapsed */
		dElapsed_time = (((tv2.tv_sec )*1000+(tv2.tv_usec/1000))
			-((tv1.tv_sec )*1000+(tv1.tv_usec/1000)));
		if(dElapsed_time < glbtimerWheel.dGranularity)
	    	 __sip_sleep((glbtimerWheel.dGranularity-dElapsed_time)*1000);

		/* increment the currenttime pointer*/
		glbtimerWheel.dCurrentIndex = (glbtimerWheel.dCurrentIndex+1) \
			% glbtimerWheel.dWheelSize;
    }/* end of outer while loop*/
    return SIP_NULL;
}

/*******************************************************
*
*  FUNCTION    : __sip_sleep
*
********************************************************/

void __sip_sleep
#ifdef ANSI_PROTO
    (SIP_U32bit dMicrosecs)
#else
    (dMicrosecs)
    SIP_U32bit dMicrosecs;
#endif
{
    struct timeval timeout;

    timeout.tv_sec = dMicrosecs /  1000000;
    timeout.tv_usec = dMicrosecs % 1000000;
    if(dMicrosecs != 0)
		select(64,SIP_NULL,SIP_NULL,SIP_NULL, &timeout);
}

/*******************************************************
*  FUNCTION    : __sip_flushTimer
*
*  DESCRIPTION : Removes all the Elements
*                from the Timer.
********************************************************/
void __sip_flushTimer(void)
{
    SipTimerElem *pTimerElem=SIP_NULL;
	SipTimerElem *pCur=SIP_NULL;
	SipTimerElem *pHead=SIP_NULL;
    SIP_U32bit dI;

    /* Flush the Timer */
    for (dI=0; dI<glbtimerWheel.dWheelSize; dI++)
    {
    	fast_lock_synch(0,&glbtimerWheel.pTimerWheelMutex[dI],0);

		pHead = &glbtimerWheel.pWheel[dI];
		pCur = pHead->pRight;
		while (pCur != pHead)
		{
	    	pTimerElem = pCur;
		    pCur = pCur->pRight;
		    sip_freeSipTxnTimeoutData((SipTxnTimeoutData *)pTimerElem->pBuffer);
		    __sip_removeTimerElem(pTimerElem);
		}

		fast_unlock_synch(0,&glbtimerWheel.pTimerWheelMutex[dI]);
    }
}
/*******************************************************
*  FUNCTION    : __sip_flushHash
*
*  DESCRIPTION : Removes all the Elements
*                from the Hash.
********************************************************/

void __sip_flushHash(void)
{
    SipTxnHashElem *pHashElem=SIP_NULL;
	SipTxnHashElem *pCur=SIP_NULL;
	SipTxnHashElem *pHead=SIP_NULL;
    SIP_U32bit dI;

    /* Flush the Hash */
    for (dI=0; dI<(NUM_BUCKET+1); dI++)
    {
    	fast_lock_synch(0,&glbhashTable.HashMutex[dI],0);

		pHead = &glbhashTable.pHashtbl[dI];
		pCur = pHead->pRight;
		while (pCur != pHead)
		{
	    	pHashElem = pCur;
		    pCur = pCur->pRight;
			pHashElem->pRight->pLeft = pHashElem->pLeft;
			pHashElem->pLeft->pRight = pHashElem->pRight;
			sip_freeSipTxnBuffer((SipTxnBuffer *)pHashElem->pBuffer);
			sip_freeSipTxnKey((SipTxnKey *)pHashElem->pKey);
			fast_memfree(TIMER_MEM_ID,pHashElem,SIP_NULL);

			/* Decrement the Txn Element Counter */
			fast_lock_synch(0,&glbhashTable.CounterMutex,0);
				glbhashTable.dTxnElemCount--;
			fast_unlock_synch(0,&glbhashTable.CounterMutex);
		}

		fast_unlock_synch(0,&glbhashTable.HashMutex[dI]);
    }
}
/*******************************************************
*  FUNCTION    : __sip_getHandle
*
*  DESCRIPTION : Generates the handle that is to be
*                returned by the fast_startTxnTimer
********************************************************/
SIP_S32bit __sip_getHandle
#ifdef ANSI_PROTO
    (SIP_U32bit* pHandle)
#else
    (pHandle)
    SIP_U32bit* pHandle;
#endif
{
    fast_lock_synch(0,&glbtimerWheel.HandleMutex,0);

    *pHandle = glbtimerWheel.dHandle;
    glbtimerWheel.dHandle++;

    fast_unlock_synch(0,&glbtimerWheel.HandleMutex);
    return 0;
}

/*******************************************************
*  FUNCTION    : sip_freeTimerWheel
*
*  DESCRIPTION : Releases the memory allocated to the
*					Timer Wheel.
********************************************************/
void sip_freeTimerWheel(void)
{
    fast_memfree(TIMER_MEM_ID,glbtimerWheel.pWheel,SIP_NULL);
    fast_memfree(TIMER_MEM_ID,glbtimerWheel.pTimerWheelMutex,SIP_NULL);
    return;
}
/*******************************************************
*  FUNCTION    : sip_freeHashTable
*
*  DESCRIPTION : Releases the memory allocated to the
*					Hash Table.
********************************************************/
void sip_freeHashTable(void)
{
    fast_memfree(TIMER_MEM_ID,glbhashTable.pHashtbl,SIP_NULL);
    return;
}
/*******************************************************
*  FUNCTION    : sip_cbk_fetchTxn
*
*  DESCRIPTION : Fetches the Transaction Element
*
********************************************************/
SipBool sip_cbk_fetchTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid* ppTxnBuffer,SIP_Pvoid pTxnKey,SIP_Pvoid pContext,\
		SIP_S8bit dOpt,SipError *pErr)
#else
	(ppTxnBuffer,pTxnKey,pContext,dOpt,pErr)
	SIP_Pvoid* ppTxnBuffer;
	SIP_Pvoid pTxnKey;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	SIP_U32bit dHash;
	SIP_U32bit dHashIndex;
	SipTxnHashElem* pTempElem=SIP_NULL;
	SipTxnHashElem *pLastElem=SIP_NULL;
	SipTxnHashElem *pHead=SIP_NULL;
	SipTxnHashElem *pCur=SIP_NULL;
	dHash = __sip_hashCalc(pTxnKey);
	dHashIndex = dHash % NUM_BUCKET;

	fast_lock_synch(0,&glbhashTable.HashMutex[dHashIndex],0);

	pHead = &glbhashTable.pHashtbl[dHashIndex];
	if ( pHead != SIP_NULL )
		pCur = pHead->pRight;
	else
	{
		*pErr = E_INV_PARAM;
		return SipFail;
	}
	while ( pCur != pHead )
	{
		if ( glbhashTable.keycmpr(pCur->pKey,pTxnKey,pErr) == SipSuccess )
		{
			pTempElem = pCur;
			break;
		}
		pCur = pCur->pRight;
	}
	if ( pTempElem != SIP_NULL )
	{
		*ppTxnBuffer = pTempElem->pBuffer;
		return SipSuccess;
	}

	if ( TXN_CREAT == dOpt )
	{
		/* New Element has to be added */
		pTempElem = (SipTxnHashElem*)fast_memget(TIMER_MEM_ID,
					sizeof(SipTxnHashElem),pErr);
		if ( pTempElem == SIP_NULL )
		{
			*pErr = E_NO_MEM;
			fast_unlock_synch(0,&glbhashTable.HashMutex[dHashIndex]);
			return SipFail;
		}
		pTempElem->pKey = pTxnKey;
		pTempElem->pContext = pContext;
		/* Increment the Txn Element Counter */
		fast_lock_synch(0,&glbhashTable.CounterMutex,0);
			glbhashTable.dTxnElemCount++;
		fast_unlock_synch(0,&glbhashTable.CounterMutex);

		pTempElem->pBuffer = *ppTxnBuffer;
		pLastElem = pHead->pLeft;
		pTempElem->pRight = pLastElem->pRight;
		pTempElem->pLeft = pLastElem;
		pLastElem->pRight->pLeft = pTempElem;
		pLastElem->pRight = pTempElem;
		return SipSuccess;
	}
	else
	{
		fast_unlock_synch(0,&glbhashTable.HashMutex[dHashIndex]);
		return SipFail;
	}
}

/*******************************************************
*  FUNCTION    : sip_cbk_releaseTxn
*
*  DESCRIPTION : Releases the Transaction Element
*
********************************************************/
SipBool sip_cbk_releaseTxn
#ifdef ANSI_PROTO
	(SIP_Pvoid pTxnKey, SIP_Pvoid* ppTxnKey,SIP_Pvoid *ppTxnBuffer, \
		SIP_Pvoid pContext,SIP_S8bit dOpt, SipError *pErr)
#else
	(pTxnKey, ppTxnKey,ppTxnBuffer,pContext,dOpt,pErr)
	SIP_Pvoid pTxnKey;
	SIP_Pvoid* ppTxnKey;
	SIP_Pvoid *ppTxnBuffer;
	SIP_Pvoid pContext;
	SIP_S8bit dOpt;
	SipError *pErr;
#endif
{
	SIP_U32bit dHash;
	SIP_U32bit dHashIndex;
	SipTxnHashElem* pTempElem=SIP_NULL;
	SipTxnHashElem *pHead=SIP_NULL;
	SipTxnHashElem *pCur=SIP_NULL;
	dHash = __sip_hashCalc(pTxnKey);
	dHashIndex = dHash % NUM_BUCKET;

	pHead = &glbhashTable.pHashtbl[dHashIndex];
	pCur = pHead->pRight;
	(void)pContext;

	while ( pCur != pHead )
	{
		if ( glbhashTable.keycmpr(pCur->pKey,pTxnKey,pErr) == SipSuccess )
		{
			pTempElem = pCur;
			break;
		}
		pCur = pCur->pRight;
	}
	if ( pTempElem != SIP_NULL )
	{
		if ( TXN_REMOVE == dOpt )
		{
			pTempElem->pLeft->pRight = pTempElem->pRight;
			pTempElem->pRight->pLeft = pTempElem->pLeft;
			*ppTxnBuffer = pTempElem->pBuffer;
			*ppTxnKey = pTempElem->pKey;
			fast_memfree(TIMER_MEM_ID,pTempElem,SIP_NULL);
			fast_unlock_synch(0,&glbhashTable.HashMutex[dHashIndex]);

			/* Decrement the Txn Element Counter */
			fast_lock_synch(0,&glbhashTable.CounterMutex,0);
				glbhashTable.dTxnElemCount--;
			fast_unlock_synch(0,&glbhashTable.CounterMutex);
			return SipSuccess;
		}
		else
		{
			fast_unlock_synch(0,&glbhashTable.HashMutex[dHashIndex]);
			*pErr = E_NO_ERROR;
			return SipSuccess;
		}
	}

	return SipFail;
}

/*******************************************************
*  FUNCTION    : __sip_hashCalc
*
*  DESCRIPTION : Calculates the Hash
*
********************************************************/
SIP_U32bit __sip_hashCalc
#ifdef ANSI_PROTO
	(SIP_Pvoid pKey)
#else
	(pKey)
	SIP_Pvoid pKey;
#endif
{
	SIP_U32bit dH=0, dG;
	SipTxnKey* pTempKey = SIP_NULL;
	SIP_S8bit* pName = SIP_NULL;

	pTempKey = (SipTxnKey*)pKey;
	if ( pTempKey->pCallid != SIP_NULL )
		pName = (SIP_S8bit*)pTempKey->pCallid;
	else
		return 0;

	while( *pName )
	{
		dH = ( dH << 4 ) + *pName++;
		if ( (dG = dH & 0xf0000000) )
			dH ^= dG >> 24;
		dH &= ~dG;
	}
	return dH;
}

/*******************************************************
*  FUNCTION    : sip_getAllKeysOfTxnElem
*
*  DESCRIPTION : Returns all the Txn Elements in
*				 the Hash Table.
********************************************************/
SipBool sip_getAllKeysOfTxnElem
#ifdef ANSI_PROTO
	(SipTxnKey** ppTxnKeys,SIP_U32bit* pCount,SipError* pErr)
#else
	(ppTxnKeys,pCount,pErr)
	SipTxnKey** ppTxnKeys;
	SIP_U32bit* pCount;
	SipError* pErr;
#endif
{
    SipTxnHashElem *pHashElem=SIP_NULL;
	SipTxnHashElem *pCur=SIP_NULL;
	SipTxnHashElem *pHead=SIP_NULL;
    SIP_U32bit dI;
	SIP_U32bit dCount=0;
	SipTxnKey* pTxnKeys = SIP_NULL;

	/* Get the Txn Element Count */
	fast_lock_synch(0,&glbhashTable.CounterMutex,0);
	*pCount = glbhashTable.dTxnElemCount;
	if ( 0 == *pCount  )
	{
		fast_unlock_synch(0,&glbhashTable.CounterMutex);
		return SipSuccess;
	}

	/* Allocate the memory for Storing the Txn Keys */
	/* This has to be released in the Application */

	pTxnKeys = (SipTxnKey*)fast_memget(TIMER_MEM_ID,\
		sizeof(SipTxnKey)*(glbhashTable.dTxnElemCount+1),pErr);
	if ( pTxnKeys == SIP_NULL )
	{
		*pErr = E_NO_MEM;
		fast_unlock_synch(0,&glbhashTable.CounterMutex);
		return SipFail;
	}

    for (dI=0; dI<(NUM_BUCKET+1); dI++)
    {
    	fast_lock_synch(0,&glbhashTable.HashMutex[dI],0);

		pHead = &glbhashTable.pHashtbl[dI];
		pCur = pHead->pRight;
		while (pCur != pHead)
		{
	    	pHashElem = pCur;
		    pCur = pCur->pRight;

			INIT((pTxnKeys[dCount]).pMethod);
			INIT((pTxnKeys[dCount]).pToTag);
			INIT((pTxnKeys[dCount]).pFromTag);
			INIT((pTxnKeys[dCount]).pViaBranch);
			INIT((pTxnKeys[dCount]).pCallid);
			INIT((pTxnKeys[dCount]).pRackHdr);

			(pTxnKeys[dCount]).dCSeq	= 0;
			(pTxnKeys[dCount]).dRseq	= 0;
			(pTxnKeys[dCount]).dTagCheck = SipFail;
			HSS_INITREF((pTxnKeys[dCount]).dRefCount);

			if ( sip_cloneSipTxnKey(&pTxnKeys[dCount],\
				(SipTxnKey*)(pHashElem->pKey),pErr) != \
				SipFail )
			{
				dCount++;
			}
		}

		fast_unlock_synch(0,&glbhashTable.HashMutex[dI]);
    }


	fast_unlock_synch(0,&glbhashTable.CounterMutex);
	*ppTxnKeys = pTxnKeys;
	return SipSuccess;
}

#if 0		// also in libsipcore.a
/*******************************************************
*  FUNCTION    : sip_getBranchFromViaHdr
*
*  DESCRIPTION : Returns the Via Branch
********************************************************/
SipBool sip_getBranchFromViaHdr
#ifdef ANSI_PROTO
	(SipViaHeader *pViaHeader,\
	 SIP_S8bit **ppBranch,\
	 SipError *pErr)
#else
	(pViaHeader,ppBranch,pErr)
	SipViaHeader *pViaHeader;
	SIP_S8bit **ppBranch;
	SipError *pErr;
#endif
{
	SIP_U32bit index, size;

	*pErr = E_NO_ERROR;

	if(sip_listSizeOf(&(pViaHeader->slParam), &size, pErr) == SipFail)
		return SipFail;
	index = 0;
	while (index < size)
	{
		SipParam *pParam;

		if(sip_listGetAt(&(pViaHeader->slParam), index, ((SIP_Pvoid *) &pParam)\
			, pErr) == SipFail)
			return SipFail;
		if(pParam->pName != SIP_NULL)
		{
			if(strcmp(pParam->pName,"branch")==0)
			{
				SIP_S8bit *pBranch;

				if(sip_listGetAt(&(pParam->slValue),0,((SIP_Pvoid *) &pBranch),\
					pErr) != SipFail)
				{
					*ppBranch = pBranch;
					return SipSuccess;
				}
			}
		}
		index++;
	}
	return SipFail;
}
#endif

#ifdef __cplusplus
}
#endif