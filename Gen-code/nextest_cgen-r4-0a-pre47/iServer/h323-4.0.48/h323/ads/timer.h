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
  timer.h

  Ron S. 28 Nov. 1995


*/

#ifndef __TIMER__
#define __TIMER__

#include <rvinternal.h>
#include <rlist.h>


typedef void *timerKey;
typedef void(*TimerHandler)(timerKey key);


DECLARE_OPAQUE(HTIMER);
DECLARE_OPAQUE(HTIMERELEM);

HTIMER timerConstruct(int maxTimers, RVHLOGMGR logMgr);
int    timerDestruct(HTIMER timer);
int    timerClear(HTIMER timer);

HTIMERELEM timerAdd(HTIMER timer, TimerHandler eventHandler, timerKey key, UINT32 timeOut);
int        timerDelete(HTIMER timer, HTIMERELEM tNode);
HTIMERELEM timerFind(HTIMER timer, TimerHandler callback, timerKey key);
int        timerDeleteByValue(HTIMER timer, TimerHandler eventHandler, timerKey key);

int    timerCheck(HTIMER timer); /* check expiration */
UINT32 timerGetNextExpiration(HTIMER timer);
int    timerGetCount(HTIMER timer);

int    timerDisplay(HTIMER timer,int msa);

#endif
#ifdef __cplusplus
}
#endif



