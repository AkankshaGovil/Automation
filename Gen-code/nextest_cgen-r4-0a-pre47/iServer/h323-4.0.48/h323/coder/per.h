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
  perComplex.h

  Ron S. 7 Nov. 1995
*/


#ifndef _PER_
#define _PER_

#include <rvinternal.h>
#include <pvaltree.h>
#include <bb.h>

DECLARE_OPAQUE(HPER);


INT32 bbGet2Right(HPER hPer,
        IN UINT32 fromBitPosition, /* in buffer */
        IN UINT32 numOfBitsToGet,
        OUT UINT8 *dest); /* destination buffer */

INT32 bbGet2Left(HPER hPer,
           IN UINT32 fromBitPosition, /* in buffer */
           IN UINT32 numOfBitsToGet,
           OUT UINT8 *dest); /* destination buffer */



int perGetMsa(void);

/************************************************************************
 * perConstruct
 * purpose: Construct PER information needed by the encode/decode manager.
 * input  : maxBufSize  - Maximum size of buffer supported (messages larger
 *                        than this size in bytes cannot be decoded/encoded).
 * output : none
 * return : none
 ************************************************************************/
void perConstruct(IN int maxBufSize);



void perDestruct(void);

int perEncode(
          IN  HPVT  valH,            /* encoding from value tree */
          IN  int    valNodeId,       /* root of encoding */
          OUT UINT8  *buffer,         /* encoding to this buffer */
          IN  int    bufferLength,    /* in bytes */
          OUT int*   encoded);        /* BYTES encoded to buffer */

int perDecode(
          OUT HPVT valH,         /* decoding to value tree */
          IN  int   valNodeId,    /* root of encoding */
          IN  INT32 fieldId,      /* root field Id */
          IN  UINT8*buffer,       /* decoding from this buffer */
          IN  int   bufferLength, /* in bytes */
          OUT int*  decoded);     /* number of BYTES successfully decoded from buffer */

#endif
#ifdef __cplusplus
}
#endif



