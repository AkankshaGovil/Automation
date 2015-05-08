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
  copybits.h

  */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _COPYBITS_
#define _COPYBITS_

#include <rvinternal.h>

#define getBit(ptr, bit) \
    ((unsigned int)(((BYTE *)(ptr))[((UINT32)(bit))>>3] & (0x80 >> ((UINT32)(bit) & 7))))

void setBit(BYTE *ptr, UINT32 bit, unsigned int value);

/* copy bits from src to dest */
void memcpyb(OUT BYTE *dest,
         IN  UINT32 destBitPos,
         IN  BYTE *src,
         IN  UINT32 srcBitPos,
         IN  UINT32 numOfBits);

UINT32 get1st0BitNumber(BYTE*ptr,UINT32 startBit, UINT32 stopBit);

#endif

#ifdef __cplusplus
}
#endif



