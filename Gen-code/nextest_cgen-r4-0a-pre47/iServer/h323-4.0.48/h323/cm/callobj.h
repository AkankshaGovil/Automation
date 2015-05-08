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
#ifndef CALLOBJ_H
#define CALLOBJ_H


#include <rvinternal.h>

typedef enum { /* call source */
  Incoming = 0x4,
  Outgoing = 0x8

} callSrcEnum;


/************************************************************************
 * callObjectCreate
 * purpose: Create a CAT struct for a given call by the call's object.
 * input  : app     - Stack's instance
 *          hsCall  - Stack's handle of the call
 *          call    - Type of call - Incoming or Outgoing
 * output : callObj - CAT struct for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int
callObjectCreate(
          IN  cmElem        *app,
          IN  HCALL         hsCall,
          IN  callSrcEnum   call,
          OUT catStruct     *callObj);


int
callObjectCreateFromMessage(
          IN  cmElem *app,
          IN  int message,
          OUT catStruct *callObj);


#endif

#ifdef __cplusplus
}
#endif



