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
  perSimple.h

  Simple type encoding/decoding
  ASN.1 PER basic-aligned variant.
  */

#ifndef _PERSIMPLE_
#define _PERSIMPLE_

#include <rvinternal.h>
#include <bb.h>

typedef enum {
  perLenTypeCONSTRAINED = 1,
  perLenTypeNORMALLY_SMALL,
  perLenTypeUNCONSTRAINED
} perLenType;


int perEncodeInt(IN  UINT32 n,
         IN  UINT32 lb,
         IN  UINT32 ub,
                 IN  BOOL isFromAbsent,
                 IN  BOOL isToAbsent,
         IN  BOOL isExtension,
         OUT HBB bbH); /* Should be initialized with sufficient size.
                  It has to have at least 4 bytes size. */

int /* TRUE or RVERROR */
perDecodeInt(
         /* Decodes an integer from a bit buffer. */
         OUT UINT32 *n, /* decoded number */
         IN  UINT32 lb,
         IN  UINT32 ub,
         IN  BOOL isFromAbsent,
         IN  BOOL isToAbsent,
         IN  BOOL isExtension,
         IN  HPER hPer,
         IN  UINT32 from, /* position in buffer */
         OUT UINT32 *decoded,
         IN  char* desc /* short description (optional) */
         );

int perEncodeLen(IN  perLenType type,  /* CONSTRAINED, NORMALLY_SMALL, UNCONSTRAINED */
         IN  UINT32 n,  /* the length */
         IN  UINT32 lb,  /* only for constrained type */
         IN  UINT32 ub,  /* only for constrained type */
         OUT HBB bbH); /* Should be initialized with sufficient size.
                  It has to have at least 4 bytes size. */

int perDecodeLen(IN  perLenType type, /* CONSTRAINED, NORMALLY_SMALL, UNCONSTRAINED */
         OUT UINT32 *n, /* the length */
         IN  UINT32 lb,
         IN  UINT32 ub,
         OUT HPER hPer,
         IN  UINT32 from, /* position in buffer */
         OUT UINT32 *decoded);

int perEncodeBool(IN  BOOL n,
          OUT HBB bbH); /* Should be initialized with sufficient size.
                   It has to have at least 1 byte size. */
int perDecodeBool(OUT BOOL *n,
          IN  HPER hPer,
          IN  UINT32 from,
          OUT UINT32 *decoded);

int perEncodeNull(HBB bbH);
int perDecodeNull(HBB bbH);


int perEncodeNormallySmallInt(IN  UINT32 n, /* the number */
                  OUT HBB bbH);

int perDecodeNormallySmallInt(OUT UINT32 *n, /* the number */
                  IN  HPER hPer,
                  IN  UINT32 from,
                  OUT UINT32 *dec);

int perEncodeNumber(IN  UINT32 n,
            IN  UINT32 b,  /* number of bits to hold the encoding */
            OUT HBB bbH); /* Should be initialized with sufficient size. */

int perDecodeNumber(OUT UINT32 *n, /* decoded number */
            IN  UINT32 b,  /* number of bits to hold the encoding */
            IN  HPER hPer,
            IN  UINT32 from, /* position in buffer */
            OUT UINT32 *decoded);


#endif
#ifdef __cplusplus
}
#endif



