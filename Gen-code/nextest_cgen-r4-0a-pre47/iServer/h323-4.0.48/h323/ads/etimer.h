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
  etimer.h

  Ron S. 28 Nov. 1995


*/

#ifndef __ETIMER__
#define __ETIMER__

#include <rvinternal.h>
#include <mei.h>
    

typedef void(RVCALLCONV*ETimerHandler)(void *param);


DECLARE_OPAQUE(HETIMER);
DECLARE_OPAQUE(HETIMERELEM);

HETIMER     etimerConstruct(int maxTimers);
int        etimerDestruct(HETIMER timer);

HETIMERELEM etimerAdd(HETIMER timer, ETimerHandler eventHandler, void *param, UINT32 timeOut);
int        etimerDelete(HETIMER timer, HETIMERELEM tNode);
HETIMERELEM etimerFind(HETIMER timer, ETimerHandler callback, void *param);
int        etimerDeleteByValue(HETIMER timer, ETimerHandler eventHandler, void *param);

int    etimerCheck(HETIMER timer, HMEI mutex); /* check expiration */
UINT32 etimerGetNextExpiration(HETIMER timer);
int    etimerGetCount(HETIMER timer);
int    etimerDisplay(HETIMER timer,int msa);
int    etimerGetParams(
               IN HETIMER timer,
               IN HETIMERELEM tNode,
               OUT ETimerHandler *callback,
               OUT void **param
               );
void etimerPrintMinElement( HETIMER timer);
#endif
#ifdef __cplusplus
}
#endif



