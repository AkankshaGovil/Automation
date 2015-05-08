#ifdef __cplusplus
extern "C" {
#endif



/*

 NOTICE:
 This document contains information that is proprietary to RADVISION LTD..
 No part of this publication may be reproduced in any form whatsoever without
 written prior approval by RADVISION LTD..

  RADVISION LTD. reserves the right to revise this publication and make changes
  without obligation to notify any person of such revisions or changes.

    */
#ifndef _CMIQ931_
#define _CMIQ931_


typedef struct
{
    HTI        timer;
    HTI        timerSE;
    BOOL       origin;
    callStateE callState;
    int        t301;
    int        t302;
    int        t303;
    int        t304;
    int        t310;
    int        t322;
} q931Elem;

#endif
#ifdef __cplusplus
}
#endif
