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
  rid.h

  id package.

  Ron S.
  21 Nov. 1996
  */

#ifndef _RID_
#define _RID_

#include <rvinternal.h>

DECLARE_OPAQUE(HRID);

#define ridFreeSave(a,b) ridFree(a,b)
#define ridSetMinFree(a,b) 

HRID
ridConstruct(
         IN  INT32 from, /* min. id */
         IN  INT32 to /* max. id */
         );

int
ridAllocationSize(
          IN  INT32 from, /* min. id */
          IN  INT32 to /* max. id */
          );

HRID
ridConstructFrom(
         IN  INT32 from, /* min. id */
         IN  INT32 to, /* max. id */
         IN  char* buffer, /* allocated buffer */
         IN  int bufferSize
         );

int
ridDestruct(
        IN  HRID ridH
        );

INT32 /* new id or RVERROR */
ridNew(
       /* get new id */
       HRID  ridH
       );

int /* RVERROR if id is free or Free() failed */
ridFree(
    /* free id */
    IN  HRID ridH,
    IN  INT32 id /* non-free id */
    );


BOOL /* true if given id is free */
ridIsFree(
      /* determine status of id */
      IN  HRID ridH,
      IN  INT32 id /* non-free id */
      );

int
ridFreeAll(
       /* free all ids */
       IN  HRID ridH
       );

int
ridSetAll(
      /* set all ids */
      IN  HRID ridH
      );


INT32
ridFrom(
        /* gets id's from or RVERROR */
        IN  HRID ridH
        );
INT32
ridTo(
      /* gets id's to or RVERROR */
      IN  HRID ridH
      );

int
ridCopy(
    /* Copy source to destination. Sizes must match */
    OUT HRID destH,
    IN  HRID srcH
    );

#endif
#ifdef __cplusplus
}
#endif



