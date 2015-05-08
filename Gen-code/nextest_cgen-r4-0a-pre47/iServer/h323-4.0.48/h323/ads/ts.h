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
  ts.h



*/

#ifndef __TS__
#define __TS__

#include <rvinternal.h>
#include <Threads_API.h>
#include <etimer.h>
#include <log.h>


typedef void(RVCALLCONV*TimerCallBack)(void *context);
/*typedef void **HTSTIMER;*/
DECLARE_OPAQUE(HT);
DECLARE_OPAQUE(HTSTIMER);
DECLARE_OPAQUE(HTSTRUCT);
DECLARE_OPAQUE(HTSAPPTIMER);

HTSTRUCT   tsConstruct(int count, RVHLOGMGR logMgr);
int        tsDestruct(HTSTRUCT tH);
int        tsClear(HTSTRUCT tH);
HTSTIMER    tsAdd(HTSTRUCT tH, int maxTimers,HTSAPPTIMER appHndl);
HTSTIMER    tsFind(HTSTRUCT tH, HTSAPPTIMER appHndl);
int        tsDelete(HTSTIMER timer);
HT         tsSet(HTSTIMER timer, TimerCallBack eventHandler,void* context,UINT32 timeOut );
int        tsReset(HTSTIMER timer, HT tElement);
int        tsResetByValue(HTSTIMER timer, TimerCallBack eventHandler,void* context);
int        tsUpdateMinTime(HTSTRUCT tH);
int        tsGetMinTime(HTSTRUCT tH);
int        tsCheck(HTSTRUCT tH);
int        tsGetCount(HTSTRUCT tH);
int        tsGetParams(HTSTIMER timer,HT tElement, TimerCallBack *eventHandler,void** context);
HTSTRUCT            tsGetStruct(IN HTSTIMER timer);
RvH323ThreadHandle  tsGetThreadId(IN HTSTRUCT tH);

#endif
#ifdef __cplusplus
}
#endif



