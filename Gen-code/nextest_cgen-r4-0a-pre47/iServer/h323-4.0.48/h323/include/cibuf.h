#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  cibuf.h  --  Configuration buffer services interface

  Module Author:  Oz Solomonovich
  This Comment:   27-May-1997

***************************************************************************/


#ifndef __CIBUF_H
#define __CIBUF_H


#include <rvcommon.h>


#define CI_BUF_ID_STRING   "**BUF**"    /* with an appended asciiz */
#define CI_BUF_ID_LEN      8


RVAPI
INT32 RVCALLCONV ciTargetBufferSize(
        IN  HCFG  hCfg
        );


RVAPI
int RVCALLCONV ciPrepareTargetBuffer(
        IN OUT  void *  buffer,
        IN      INT32   bufferSize
        );


RVAPI
int RVCALLCONV ciAllocateTargetBuffer(
        IN   HCFG     hCfg,
        OUT  void **  buffer,
        OUT  int *    bufferSize
        );


RVAPI
int RVCALLCONV ciFreeTargetBuffer(
        IN  HCFG    hCfg,
        IN  void *  buffer
        );



#endif /* __CIBUF_H */
#ifdef __cplusplus
}
#endif



