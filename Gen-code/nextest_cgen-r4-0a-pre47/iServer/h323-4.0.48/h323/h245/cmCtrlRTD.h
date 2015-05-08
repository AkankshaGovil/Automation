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



#include <stdlib.h>

#include <rvinternal.h>
#include <cmintr.h>


/************************************************************************
 * rtdInit
 * purpose: Initialize the round trip delay process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
 int rtdInit(controlElem* ctrl);

/************************************************************************
 * rtdEnd
 * purpose: Stop the round trip delay process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void rtdEnd(IN controlElem* ctrl);

int roundTripDelayRequest(controlElem* ctrl, int message);
int roundTripDelayResponse(controlElem* ctrl, int message);

#ifdef __cplusplus
}
#endif

