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

#ifndef _TIH_
#define _TIH_

#include <rvinternal.h>

UINT32 timerGetTimeInMilliseconds(void);
UINT32 timerGetTimeInSeconds(void);

#endif
#ifdef __cplusplus
}
#endif



