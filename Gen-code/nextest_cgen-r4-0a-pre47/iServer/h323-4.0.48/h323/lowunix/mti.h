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
  mti.h

  Ron S. 29 Nov. 1995
  Revised 18 Aug. 1996 by ron.

  Low level timing mechanism.
  Machine dependant.

  Handle ONE timer.
  Repeated called to timerSet will override the previous calls.
  timeout interval is in mili-seconds.

*/

#ifndef _MTIH_
#define _MTIH_

#include <rvcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_OPAQUE(HTI);
DECLARE_OPAQUE(HSTIMER);
DECLARE_OPAQUE(HAPPTIMER);
typedef void(RVCALLCONV*LPMTIMEREVENTHANDLER)(void* context);


RVAPI HSTIMER RVCALLCONV
mtimerInit(int maxTimers, HAPPTIMER appHndl);

RVAPI int RVCALLCONV
mtimerEnd(HSTIMER timer);


RVAPI int RVCALLCONV
mtimerEndByHandle(HAPPTIMER appHndl);


RVAPI HTI RVCALLCONV
mtimerSet(
     HSTIMER timer,
     LPMTIMEREVENTHANDLER eventHandler,
     void* context,
     UINT32 timeOut  /* 1 msec units */
     );

RVAPI HTI RVCALLCONV
mtimerSetByHandle(
         HAPPTIMER appHndl,
         LPMTIMEREVENTHANDLER eventHandler,
         void* context,
         UINT32 timeOut
         );
RVAPI int RVCALLCONV
mtimerResetByValue(
          HSTIMER timer,
          LPMTIMEREVENTHANDLER eventHandler,
          void* context
          );


RVAPI int RVCALLCONV
mtimerResetByHandle(
          HAPPTIMER  appHndl,
          HTI tElem
          );
RVAPI int RVCALLCONV
mtimerReset(
       HSTIMER timer,
       HTI tElem
       );

/************************************************************************
 * mtimerSetMaxTimerSets
 * purpose: To change the maximum number of timer sets (mtimerInit calls)
 *          allowed per thread (the default is 16)
 * input  : timerSets   - the maximum number of timer sets
 * output : none
 * return : none
 ************************************************************************/
RVAPI void RVCALLCONV mtimerSetMaxTimerSets(int timerSets);

#ifdef __cplusplus
}
#endif

#endif
