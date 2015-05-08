/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/*
  intUtils.h

  Ron S.
  2 May 1996

  int conversion functions.

  */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _INTUTILS_
#define _INTUTILS_

#include <rvinternal.h>

BOOL /* true if platform is using big endian integers */
isPlatformBigEndian(void);

UINT16 /* num representation in the opposite endian type */
intEndianConvert16(IN  UINT16 num);

UINT32 /* num representation in the opposite endian type */
intEndianConvert(IN  UINT32 num);

int
int32EndianChange( /* change content of buffer (32 bits integer) to other endian format */
          INOUT char *buffer);



/* convert integer to little endian */
UINT8 *int2LittleE(IN  UINT32 num,
           IN  int numOfBytes, /* to represent num in buffer */
           OUT UINT8  *buf);

/* convert integer to big endian */
UINT8 *int2BigE(IN  UINT32 num,
        IN  int numOfBytes, /* to represent num in buffer */
        OUT UINT8 *buf);

/* convert big endian to integer */
int bigE2Int(IN  UINT8 *buf,
         IN  int numOfBytes, /* in buffer */
         OUT UINT32 *num);

BOOL isNumber(IN char *data); /* null terminated string */


/* returns 2 to the power of n. n must be positive */
#define ipow2(n) (1<<(n))


#ifndef WIN32
char * CDECL itoa(int i, char *a, int size);
#endif

BOOL isPrime(int n);

int /* first prime number avobe n */
intFirstPrime(int n);


typedef unsigned int RvRandomType;
	
typedef struct {
	RvRandomType state;
} RvRandomGenerator;

RVAPI RvRandomGenerator* UTILS_RandomGeneratorConstruct(RvRandomGenerator* r,
                                                  RvRandomType seed);
#define UTILS_RandomGeneratorDestruct(r)
#define UTILS_RandomGeneratorGetMax(r)			(RAND_MAX)
#define UTILS_RandomGeneratorGetInRange(r, n)	(rvRandomGeneratorGetValue(r) % (n))
RVAPI unsigned int UTILS_RandomGeneratorGetValue(RvRandomGenerator* r);

#endif
#ifdef __cplusplus
}
#endif
