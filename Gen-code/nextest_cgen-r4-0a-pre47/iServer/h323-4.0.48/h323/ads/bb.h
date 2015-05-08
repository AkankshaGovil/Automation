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
  bb.h

  bit buffer handling

  Ron S.
  11 May 1996
  */

#ifndef _BB_
#define _BB_

#include <rvinternal.h>

DECLARE_OPAQUE(HBB); /* handler */

typedef struct {
  BOOL isAllocated;
  unsigned int maxOctets;
  UINT32 bitsInUse;
  BOOL isAligned;
  UINT8 *octets; /* buffer */
  BOOL isOverflowOfBuffer;
} bbStruct;




/* returns number of bytes needed to contain the bitLength */
unsigned int bbSetByte(IN UINT32 bitLength);

int bbGetAllocationSize(int maxOctets);

HBB bbConstruct(IN int maxOctets); /* size of buffer in octets */

HBB bbConstructFrom(IN int maxOctets, /* size of buffer in octetes */
            IN char *buffer,
            IN int bufferSize);

int bbSetOctets(IN  HBB bbH,
        IN  int maxOctets, /* size of buffer in octets */
        IN  INT32 bitsInUse, /* number of bits already in use */
        IN  UINT8 *octetBuffer); /* octet memory */

int bbDestruct(HBB bbH);

/* set buffer to zeros */
int bbClear(HBB bbH);

/* returns pointer to the octet array */
UINT8 *bbOctets(HBB bbH);

/* TRUE if buffer is aligned */
BOOL bbIsAligned(HBB bbH);

/* return number of alignment bits (modulu 8) */
int bbAlignBits(HBB bbH,
        IN INT32 location);

/* set buffer alignment */
int bbSetAligned(HBB bbH);
int bbSetUnaligned(HBB bbH);

INT32 bbFreeBits      (HBB bbH);
int bbFreeBytes     (HBB bbH);
UINT32 bbBitsInUse     (HBB bbH);
unsigned bbBytesInUse    (HBB bbH);


/* concatate src to buffer */
int bbAddTail(HBB bbH,
          IN UINT8 *src,
          IN UINT32 srcBitsLength,
          IN BOOL isAligned); /* true if src is aligned */

int bbAddTailFrom(HBB bbH,
          IN  UINT8 *src,
          IN  UINT32 srcFrom, /* offset for beginning of data in src, in bits */
          IN  UINT32 srcBitsLength,
          IN  BOOL isAligned); /* true if src is aligned */


/* move bits within buffer
   bitLength bits starting at fromOffset shall be moved to position starting at toOffset */
int bbMove(HBB bbH,
       IN  UINT32 fromOffset,
       IN  UINT32 bitLength,
       IN  UINT32 toOffset);

/* Desc: Set bits within buffer. */
int bbSet(HBB bbH,
      IN  UINT32 fromOffset,
      IN  UINT32 bitLength,
      IN  UINT8 *src);



int bbDelTail(HBB bbH,
          IN UINT32 numOfBits); /* to delete from tail of buffer */

int bbDelHead(HBB bbH,
          IN UINT32 numOfBits); /* to delete from HEAD of buffer */

int bbDisplay(HBB bbH);


#endif
#ifdef __cplusplus
}
#endif



