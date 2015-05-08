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
  rid.c

  id package.

  Use a bit buffer to save status of ids.
  Time complexity: O(n)
  Space complexity: (to-from+1)/8 bytes


  int bit array: 0 means free, 1 means taken.

  Ron S.
  21 Nov. 1996
  */

#include <rid.h>
#include <copybits.h>

#define RID_BIT_ARRAY_SIZE(n)   ( (n+7)/8 + (4 - ((n+7)/8)%4)%4)
#define RID_BIT_VECTOR(rid)      ((BYTE *) (rid) + sizeof(ridStruct) )

#define RID_NAME "rid"

typedef char* rHandler;

int /* RVERROR if id is not free or set failed */
ridSet(
       /* set a free id */
    IN  HRID ridH,
    IN  INT32 id /* free id */
    );

typedef struct {
  char* hName;
  BOOL  isAllocated;
  INT32 from;
  INT32 to;
  INT32 minFreeId; /* minimal free id */
} ridStruct;



int
ridAllocationSize(
          IN  INT32 from, /* min. id */
          IN  INT32 to /* max. id */
          )
{
  if (to<from) return RVERROR;

  return sizeof(ridStruct)+
    (int)RID_BIT_ARRAY_SIZE(to-from+1);
}


HRID
ridConstructFrom(
         IN  INT32 from, /* min. id */
         IN  INT32 to, /* max. id */
         IN  char* buffer, /* allocated buffer */
         IN  int bufferSize
         )
{
  ridStruct* rid;

  if (to<from) return NULL;
  if (!buffer || bufferSize<ridAllocationSize(from, to)) return NULL;
  memset(buffer, 0, ridAllocationSize(from, to));

  rid=(ridStruct*)buffer;

  rid->hName = (char*)RID_NAME;
  rid->isAllocated = FALSE;
  rid->from = from;
  rid->to = to;
  rid->minFreeId = from;
  return (HRID)rid;
}


HRID
ridConstruct(
         IN  INT32 from, /* min. id */
         IN  INT32 to /* max. id */
         )
{
  ridStruct* rid;

  if (to<from) return NULL;
  if ( (rid = (ridStruct*)calloc(ridAllocationSize(from, to), 1)) == NULL)
    return NULL;

  rid->hName = (char*)RID_NAME;
  rid->isAllocated = TRUE;
  rid->from = from;
  rid->to = to;
  rid->minFreeId = from;
  return (HRID)rid;
}

int
ridDestruct(
        IN  HRID ridH
        )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (rid && rid->isAllocated) free(rid);
  return TRUE;
}

INT32 /* new id or RVERROR */
ridNew(
       /* get new id */
       HRID  ridH
       )
{
  ridStruct* rid = (ridStruct*)ridH;
  INT32 i;
 
  if (!rid) return RVERROR;

  for (i=rid->minFreeId; i<=rid->to; i++)
    if (ridIsFree(ridH, i)) {
      ridSet(ridH, i);
      rid->minFreeId = i+1;
      if (rid->minFreeId >= rid->to)
          rid->minFreeId = rid->from;
      return i;
    }

  return RVERROR;
}

int /* RVERROR if id is free or Free() failed */
ridFree(
    /* free id */
    IN  HRID ridH,
    IN  INT32 id /* non-free id */
    )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;
  if (id<rid->from || id>rid->to) return FALSE;

  if (ridIsFree(ridH, id)) return RVERROR;

  setBit(RID_BIT_VECTOR(rid), id-rid->from, FALSE);

  return TRUE;
}


BOOL /* true if given id is free */
ridIsFree(
      /* determine status of id */
      IN  HRID ridH,
      IN  INT32 id /* non-free id */
      )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return FALSE;
  if (id<rid->from || id>rid->to) return FALSE;

  return ((getBit(RID_BIT_VECTOR(rid), id-rid->from)) != 0) ? (FALSE):(TRUE);
}


int /* RVERROR if id is not free or set failed */
ridSet(
       /* set a free id */
    IN  HRID ridH,
    IN  INT32 id /* free id */
    )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;
  if (id<rid->from || id>rid->to) return FALSE;
  if (!ridIsFree(ridH, id)) return RVERROR;

  setBit(RID_BIT_VECTOR(rid), id-rid->from, TRUE);
  return TRUE;
}


int
ridFreeAll(
       /* free all ids */
       IN  HRID ridH
       )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;

  memset(RID_BIT_VECTOR(rid), 0, (int)RID_BIT_ARRAY_SIZE(rid->to -rid->from +1));
  return TRUE;
}

int
ridSetAll(
      /* set all ids */
      IN  HRID ridH
      )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;

  memset(RID_BIT_VECTOR(rid), -1, (int)RID_BIT_ARRAY_SIZE(rid->to -rid->from +1));
  return TRUE;
}


INT32
ridFrom(
        /* gets id's from or RVERROR */
        IN  HRID ridH
        )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;
  return rid->from;
}



INT32
ridTo(
      /* gets id's to or RVERROR */
      IN  HRID ridH
      )
{
  ridStruct* rid = (ridStruct*)ridH;

  if (!rid) return RVERROR;
  return rid->to;
}



int
ridCopy(
    /* Copy source to destination. Sizes must match */
    OUT HRID destH,
    IN  HRID srcH
    )
{
  ridStruct* dest = (ridStruct*)destH;
  ridStruct* src = (ridStruct*)srcH;

  if (!dest || !src) return RVERROR;
  if (dest->to - dest->from != src->to - src->from) return RVERROR; /* different sizes */

  memcpy(RID_BIT_VECTOR(dest), RID_BIT_VECTOR(src), (int)RID_BIT_ARRAY_SIZE(src->to - src->from +1));
  dest->to = src->to;
  dest->from = src->from;
  dest->minFreeId=src->minFreeId;

  return TRUE;
}


#ifdef _RID_TEST_

int main(int argc, char* argv[])
{
  HRID ridH;
  int i, id;
  int from=1, to=65535;
  BOOL ret;
  char buffer[50];

  if (argc>1) from = atoi(argv[1]);
  if (argc>2) to = atoi(argv[2]);

  /*ridH = ridConstruct(from, to);*/
  ridH = ridConstructFrom(from, to, buffer, sizeof(buffer));
  printf("rid allocation size: %d.\n", ridAllocationSize(from, to));

  /* ridSetAll(ridH); */

  for (i=0; i<to-from+3; i++) {
    id = ridNew(ridH);
    printf("[%d] got new id: %d.\n", i, id);
  }

  /* ridFreeAll(ridH); */

  for (i=from-1; i<to+2; i++) {
    ret = ridFree(ridH, i);
    printf("[%d] free id result: %d.\n", i, ret);
  }

  ridDestruct(ridH);

  return 0;
}

#endif
#ifdef __cplusplus
}
#endif



