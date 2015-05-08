#ifdef __cplusplus
extern "C" {
#endif

/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  ti.h

  Ron S. 29 Nov. 1995
  Revised 18 Aug. 1996 by ron.

  Low level timing mechanism.
  Machine dependant.

  Handle ONE timer.
  Repeated called to timerSet will override the previous calls.
  timeout interval is in mili-seconds.

*/

#ifdef __VXWORKS__
/* --------------------------- vxWorks ------------------------- */
#include <vxWorks.h>
#include <sys/ioctl.h>
#include <inetLib.h>
#include <ioLib.h>
#include <selectLib.h>
#include <taskLib.h>
#include <time.h>
#include <tickLib.h>


#elif RV_OS_OSE
/* -------------------------- OSE ------------------------- */
#include <ose.h>
#include <inet.h>

#elif __PSOSPNA__
/* -------------------------- pSOS ------------------------- */
#include <configs.h>
#include <time.h>
#include <pna.h>
#define __Iunistd



#elif UNDER_CE
/* ----------------------- Windows CE ------------------------- */
#pragma warning (disable : 4201 4214)
#include <windows.h>
#include <winsock.h>



#else
/* ---------------------- Unix ----------------------- */
#include <sys/time.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>

#endif  /* __VXWORKS__ / __PSOSPNA__ / UNDER_CE */


#include <seli.h>
#include <tls.h>
#include <ts.h>
#include <mti.h>


static int seliMAX_TIMERSETS = 16;


static UINT32 mtiCur = 0;
static UINT32 mtiMax = 0;

static RVHLOGMGR    timerLogMgr = NULL;


typedef struct
{
    int                     initialized;
    int                     msaTime;
    HTSTRUCT                timerTH;
} THREAD_MtiLocalStorage;



int seliPreempt(IN RvH323ThreadHandle threadId);




HTSTRUCT mtiGetTimersHandle(void)
{
    THREAD_MtiLocalStorage* mtiTls;

    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));
    if (mtiTls == NULL)
        return NULL;

    return mtiTls->timerTH;
}




RVAPI HSTIMER RVCALLCONV mtimerInit(int maxTimers, HAPPTIMER appHndl)
{
    THREAD_MtiLocalStorage* mtiTls;
    HSTIMER timer;
    tlsLockAll();

    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if (!mtiTls->initialized)
    {
        timerLogMgr = logInitialize();
        mtiTls->timerTH = tsConstruct(seliMAX_TIMERSETS, timerLogMgr);
        mtiTls->msaTime = msaRegister(0, "TIMER", "Select Timer Interface");
    }
    mtiTls->initialized++;

    timer = (HSTIMER)tsAdd(mtiTls->timerTH, maxTimers, (HTSAPPTIMER)appHndl);
    if (timer == NULL)
    {
        msaPrintFormat(mtiTls->msaTime, "timerInit: timer construct failed.");
        return NULL;
    }

    tlsUnlockAll();
    return timer;
}


RVAPI int RVCALLCONV mtimerEnd(HSTIMER timer)
{
    THREAD_MtiLocalStorage* mtiTls;
    tlsLockAll();
    mtiTls = (THREAD_MtiLocalStorage *)
        THREAD_GetLocalStorage(tsGetThreadId(tsGetStruct((HTSTIMER)timer)), tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if (mtiTls->initialized)
    {
        tsDelete((HTSTIMER)timer);

        mtiTls->initialized--;
        if (!mtiTls->initialized)
        {
            tsDestruct(mtiTls->timerTH);
            mtiTls->timerTH = NULL;
            msaUnregister(mtiTls->msaTime);
            logEnd(timerLogMgr);
            timerLogMgr = NULL;
        }
    }

    tlsUnlockAll();
    return 0;
}


RVAPI int RVCALLCONV mtimerEndByHandle(HAPPTIMER appHndl)
{
    int result;
    HSTIMER timer;
    THREAD_MtiLocalStorage* mtiTls;
    tlsLockAll();

    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if ( !mtiTls->timerTH ) return RVERROR;

    timer = (HSTIMER)tsFind(mtiTls->timerTH, (HTSAPPTIMER)appHndl);
    result = mtimerEnd(timer);

    tlsUnlockAll();
    return result;
}


/* This procedure should be called at following sequence: */
/*          1. seliInit */
/*      .........   */
/*      2. timerSet */

RVAPI HTI RVCALLCONV mtimerSet(
     /* Desc: set time interval in mili-seconds for this task.
        Note: Max timeout shall be less then 0x7fffffff (2147483647) msec ~ 24.8 days
     */
     HSTIMER timer,
     LPMTIMEREVENTHANDLER eventHandler,
     void* context,
     UINT32 timeOut  /* 1 msec units */
     )
{
    HTI loc;
    RvH323ThreadHandle threadId;
    THREAD_MtiLocalStorage* mtiTls;

    threadId = tsGetThreadId(tsGetStruct((HTSTIMER)timer));
    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if (timeOut > 1000000000)
    {
        logPrint((RVHLOG)mtiTls->msaTime, RV_ERROR,
            ((RVHLOG)mtiTls->msaTime, RV_ERROR, "mtimerSet: timeout out of range"));
        return (HTI)RVERROR;
    }

    if (!timer)
        return (HTI)RVERROR;
    if (timeOut == 0)
        timeOut = 1;

    msaPrintFormat(mtiTls->msaTime, "timerSet(Before)");

    loc = (HTI)tsSet((HTSTIMER)timer, eventHandler, context, timeOut);

    msaPrintFormat(mtiTls->msaTime, "timerSet(After): loc=%d callback=0x%p, timeOut=%d, context=0x%p.",
        loc, eventHandler, timeOut, context);

    if ((int)loc >= 0)
    {
        mtiCur++;
        if (mtiCur > mtiMax)
            mtiMax = mtiCur;
    }

#ifndef NOTHREADS
    seliPreempt(threadId);
#endif

    return (loc);
}


RVAPI HTI RVCALLCONV mtimerSetByHandle(
         /* Desc: set time interval in mili-seconds for this task.
            Note: Max timeout shall be less then 0x7fffffff (2147483647) msec ~ 24.8 days
         */
         HAPPTIMER appHndl,
         LPMTIMEREVENTHANDLER eventHandler,
         void* context,
         UINT32 timeOut  /* 1 msec units */
         )
{
    HSTIMER timer;
    THREAD_MtiLocalStorage* mtiTls;
    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if ( !mtiTls->timerTH ) return (HTI)RVERROR;

    timer = (HSTIMER)tsFind(mtiTls->timerTH, (HTSAPPTIMER)appHndl);

    return ( mtimerSet(timer, eventHandler, context, timeOut));
}



RVAPI int RVCALLCONV mtimerResetByValue(
          /* Delete timer from this task timer list */
          HSTIMER timer,
          LPMTIMEREVENTHANDLER eventHandler,
          void* context
          )
{
    THREAD_MtiLocalStorage* mtiTls;
    RvH323ThreadHandle threadId;

    threadId = tsGetThreadId(tsGetStruct((HTSTIMER)timer));
    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if (mtiTls->timerTH == NULL)
    {
        msaPrintFormat(mtiTls->msaTime, "timerResetByValue: Cannot reset timer on callback=0x%x, key=0x%x.",
            eventHandler, context);
        return RVERROR;
    }

    msaPrintFormat(mtiTls->msaTime, "timerResetByValue(Before): minT=%d callback=0x%p, context=0x%p.",
        tsGetMinTime(mtiTls->timerTH), eventHandler, context);

    if (tsResetByValue((HTSTIMER)timer, eventHandler, context) < 0)
    {
        msaPrintFormat(mtiTls->msaTime, "timerResetByValue: Cannot reset timer on callback=0x%x, key=0x%x.",
            eventHandler, context);
        return RVERROR;
    }

    mtiCur--;
    msaPrintFormat(mtiTls->msaTime, "timerResetByValue(After): minT=%d callback=0x%p, context=0x%p.",
        tsGetMinTime(mtiTls->timerTH), eventHandler, context);
    msaPrintFormat(mtiTls->msaTime, "timerResetByValue: callback=0x%p, context=0x%p.", eventHandler, context);

#ifndef NOTHREADS
    seliPreempt(threadId);
#endif

    return TRUE;
}


RVAPI int RVCALLCONV mtimerReset(
       HSTIMER timer,
       HTI tElem
       )
{
    RvH323ThreadHandle threadId;
    THREAD_MtiLocalStorage* mtiTls;

    if (!timer)
         return RVERROR;

    threadId = tsGetThreadId(tsGetStruct((HTSTIMER)timer));
    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(threadId, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if (!mtiTls->timerTH)
         return RVERROR;

    msaPrintFormat(mtiTls->msaTime, "timerReset(Before): minT=%d loc=%d", tsGetMinTime(mtiTls->timerTH), (int)tElem);

    if (tsReset((HTSTIMER)timer, (HT)tElem) < 0)
        return RVERROR;

    mtiCur--;
    msaPrintFormat(mtiTls->msaTime, "timerReset(After): minT=%d loc=%d", tsGetMinTime(mtiTls->timerTH), (int)tElem);

#ifndef NOTHREADS
    seliPreempt(threadId);
#endif
    return TRUE;
}


RVAPI int RVCALLCONV mtimerResetByHandle(
            HAPPTIMER  appHndl,
            HTI  tElem
          )
{
    HSTIMER timer;
    THREAD_MtiLocalStorage* mtiTls;
    mtiTls = (THREAD_MtiLocalStorage *)THREAD_GetLocalStorage(NULL, tlsIntrMti, sizeof(THREAD_MtiLocalStorage));

    if ( !mtiTls->timerTH ) return RVERROR;

    timer = (HSTIMER)tsFind(mtiTls->timerTH, (HTSAPPTIMER)appHndl);

    return (mtimerReset( timer, tElem));
}




/*__________________________Get_time_reference_____________________________*/

/*
  RETURNS reference of system time in miliseconds.
  The time is relative and has no date meaning.
  Uses the gettimeofday() system call.
  */
UINT32 timerGetTimeInMilliseconds(void)
{
#ifdef __VXWORKS__
    /* VXWORKS */
    UINT32 nTicks, nTicksPerSec;
    UINT32 t;
    static UINT32 offset=0;
    static UINT32 lastTime=0;

    nTicks = tickGet();

    nTicksPerSec = sysClkRateGet();

    t = nTicks / nTicksPerSec * 1000 + (nTicks % nTicksPerSec) * 1000 / nTicksPerSec + offset;

    if (t<lastTime)
    {
       /* the "t" has passed 0xFFFF.FFFF mark */
       offset = lastTime;     
    }

    lastTime = t;


    return t;


#elif RV_OS_OSE
  /* OSE ... */
    UINT32 nTicks, nMsPerTick;
    UINT32 t;

    /* This fix has to do with the fact that after enough time the multiplying with 1000
       operation which is done first, might cause an overflow. */
    nTicks = get_ticks();
    nMsPerTick = system_tick();
    t = nTicks * nMsPerTick / 1000;/* + (nTicks % (1000 / nMsPerTick)) * nMsPerTick;*/

    return (t==0xffffffff)?0:t;

#elif __PSOSPNA__
  /* pSOS */
   ULONG date;
   ULONG time;
   ULONG ticks;
   ULONG day;
   ULONG month;

   ULONG cur_time;

   static ULONG start_time = 0l;

   tm_get(&date, &time, &ticks);
   month= ((date>>8 )&0x000000FF);
   day  = ((date    )&0x000000FF);
   if (month == 0)
       month =1;
   if (day == 0)
       day = 1;

   if ( !start_time)
    start_time = ((time  >> 16)&0x0000FFFF)*3600 + ((time>>8)&0x000000FF)*60 + (time&0x000000FF);

   cur_time = ((time  >> 16)&0x0000FFFF)*3600 + ((time>>8)&0x000000FF)*60 + (time&0x000000FF)+
                    (day-1)*86400 + (month-1)*2678400;

   ticks = ticks / CLOCKS_PER_SEC * 1000 + (ticks % CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC;
   return (cur_time - start_time)*1000 + ticks;         /* pSOS */

#elif UNDER_CE
    /* Windows CE */
    return GetTickCount();


#else
   /* Unix */
  UINT32 t;
  static UINT32 sSeconds=0;
  struct timeval tv; /* timeval structure */

  gettimeofday(&tv, NULL);
  /* printf("%d   %d\n", tv.tv_sec, tv.tv_usec); */
  if (!sSeconds)
    sSeconds=(UINT32)tv.tv_sec;

  t=(UINT32)( (tv.tv_sec - sSeconds)*1000 + (tv.tv_usec/1000) );
  return (t==0xffffffff)?0:t;
#endif
}


UINT32 timerGetTimeInSeconds(void)
{
  return (time(NULL))&0x7fffffff; /* force positive time */
}



/************************************************************************
 * mtimerSetMaxTimerSets
 * purpose: To change the maximum number of timer sets (mtimerInit calls)
 *          allowed per thread (the default is 16)
 * input  : timerSets   - the maximum number of timer sets
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV mtimerSetMaxTimerSets(int timerSets)
{
    seliMAX_TIMERSETS=timerSets;
}



RVAPI UINT32 RVCALLCONV mtimerGetCurTimers(void)
{
    return mtiCur;
}


RVAPI UINT32 RVCALLCONV mtimerGetMaxTimers(void)
{
    return mtiMax;
}



#ifdef __cplusplus
}
#endif
