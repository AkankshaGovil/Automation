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
  bb.c

  bit buffer handling

  Ron S.
  11 May 1996
  */

#include <stdio.h>

#include <bb.h>
#include <copybits.h>



/*
   Desc: Set correct number of bytes in length of bits.
   */
unsigned int
bbSetByte(UINT32 bitLength)
{
  unsigned int bytes = (int)(bitLength/8);

  if (bitLength%8) bytes++; /* complete the last byte. */
  return bytes;
}

/*_____________________________________________________________________*/
/*_________________________________MODULE______________________________*/
/*_____________________________________________________________________*/

int
bbGetAllocationSize(int maxOctets)
{
  return sizeof(bbStruct) + maxOctets;
}


static bbStruct *
bbBuild(IN char *buffer,
    IN int maxOctets)
{
  bbStruct *bb;

  memset(buffer, 0, bbGetAllocationSize(maxOctets));
  bb = (bbStruct *)buffer;

  bb->isAllocated = FALSE;
  bb->maxOctets = maxOctets;
  bb->bitsInUse = 0;
  bb->isAligned = TRUE;
  bb->octets = (UINT8 *)bb + sizeof(bbStruct);
  bb->isOverflowOfBuffer = FALSE;

  return bb;
}


HBB
bbConstruct(IN int maxOctets) /* size of buffer in octetes */
{
  bbStruct *bb;
  char *buffer;

  if (!(buffer = (char *)calloc(bbGetAllocationSize(maxOctets), 1))) return NULL;
  bb = bbBuild(buffer, maxOctets);
  bb->isAllocated = TRUE;
  return (HBB)bb;
}


/*
  Desc: Construct bb from user specified memory location.
  Note: user memory size should be at least the size of bb structure!
  */
HBB
bbConstructFrom(IN int maxOctets, /* size of buffer in octetes */
        IN char *buffer,
        IN int bufferSize)
{
  bbStruct *bb;

  if (!buffer || bufferSize < bbGetAllocationSize(maxOctets)) return NULL;
  bb = bbBuild(buffer, maxOctets);
  bb->isAllocated = FALSE;
  return (HBB)bb;
}


/*
   Desc: Set the octets to be octetBuffer.
   Usage: for user allocated buffer.
   */
int
bbSetOctets(IN  HBB bbH,
        IN  int maxOctets, /* size of buffer in octets */
        IN  INT32 bitsInUse, /* number of bits already in use */
        IN  UINT8 *octetBuffer) /* octet memory */
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb || !octetBuffer) return RVERROR;
  bb->maxOctets = maxOctets;
  bb->bitsInUse = bitsInUse;
  bb->isAligned = TRUE;
  bb->octets = octetBuffer;
  return TRUE;
}


int
bbDestruct(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  if (bb->isAllocated) free(bb);
  return TRUE;
}


/* set buffer to zeros */
int
bbClear(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;

  bb->bitsInUse = 0;
  bb->isAligned = TRUE;
  memset(bb->octets, 0, bb->maxOctets);
  return TRUE;
}

/*_____________________________________________________________________*/
/*_________________________________STATUS______________________________*/
/*_____________________________________________________________________*/

/* returns pointer to the octet array */
UINT8 *
bbOctets(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return NULL;
  return bb->octets;
}


/* TRUE if buffer is aligned */
BOOL
bbIsAligned(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return FALSE;
  return bb->isAligned;
}


/* return number of alignment bits (modulu 8) */
int
bbAlignBits(HBB bbH,
        IN INT32 location)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  return (8 - (int)location%8)%8;
}


/* set buffer alignment */
int
bbSetAligned(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  bb->isAligned = TRUE;
  return TRUE;
}

int
bbSetUnaligned(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  bb->isAligned = FALSE;
  return TRUE;
}


/*
  Desc: Return number of BITS left free in buffer. Meaning you may add
  up to this number of bits to buffer.
*/
INT32
bbFreeBits(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  return (int)(bb->maxOctets*8l - bb->bitsInUse);

}

/*
  Desc: Return number of BYTES left free in buffer. Meaning you may add
  up to this number of bytes to buffer. rounded up.
*/
int
bbFreeBytes(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  return (bb->maxOctets - bbSetByte(bb->bitsInUse));
}


/*
  Desc: Return number of BITS currently in buffer.
  */
UINT32
bbBitsInUse(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return (unsigned)RVERROR;
  return bb->bitsInUse;
}


/*
  Desc: Return number of BYTES currently in buffer.
  */
unsigned
bbBytesInUse(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return (unsigned)RVERROR;
  return bbSetByte(bb->bitsInUse);
}


/*_____________________________________________________________________*/
/*_________________________________UPDATE______________________________*/
/*_____________________________________________________________________*/

/* concatate src to buffer */
int
bbAddTail(HBB bbH,
      IN UINT8 *src,
      IN UINT32 srcBitsLength,
      IN BOOL isAligned) /* true if src is aligned */
{
  bbStruct *bb = (bbStruct *)bbH;
  INT32 bbPos, align;
  UINT8 ch=0;

  if (!bb || (!src && srcBitsLength!=0)) return RVERROR;

  bbPos = bb->bitsInUse;
  if (isAligned)  {  /* align to octet boundery. 0 padding */
    align = bbAlignBits(bbH, bbPos);
    memcpyb(bb->octets, bbPos, &ch, 0, align);
    bbPos += align;
  }

  if (bbPos + srcBitsLength > bb->maxOctets*8l)
    {
      bb->isOverflowOfBuffer=TRUE;
      return RVERROR; /* no place */
    }
  memcpyb(bb->octets, bbPos, src, 0, srcBitsLength);
  bb->bitsInUse = bbPos + srcBitsLength;
  return TRUE;
}



/* concatate src to buffer */
int
bbAddTailFrom(HBB bbH,
          IN  UINT8 *src,
          IN  UINT32 srcFrom, /* offset for beginning of data in src, in bits */
          IN  UINT32 srcBitsLength,
          IN  BOOL isAligned) /* true if src is aligned */
{
  bbStruct *bb = (bbStruct *)bbH;
  INT32 bbPos, align;
  UINT8 ch=0;

  if (!bb || !src) return RVERROR;

  bbPos = bb->bitsInUse;
  if (isAligned)  {  /* align to octet boundery. 0 padding */
    align = bbAlignBits(bbH, bbPos);
    memcpyb(bb->octets, bbPos, &ch, 0, align);
    bbPos += align;
  }

  if (bbPos + srcBitsLength > bb->maxOctets*8l)
    {
      bb->isOverflowOfBuffer=TRUE;
      return RVERROR; /* no place */
    }
  memcpyb(bb->octets, bbPos, src, srcFrom, srcBitsLength);
  bb->bitsInUse = bbPos + srcBitsLength;
  return TRUE;
}



/* move bits within buffer
   bitLength bits starting at fromOffset shall be moved to position starting at toOffset */
int
bbMove(HBB bbH,
       UINT32 fromOffset,
       UINT32 bitLength,
       UINT32 toOffset)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  if (fromOffset + bitLength > bbBitsInUse(bbH) ||
      toOffset + bitLength > bb->maxOctets*8l)
    {  /* no place to hold bits */
      bb->isOverflowOfBuffer=TRUE;
      return RVERROR;
    }
  memcpyb(bb->octets, toOffset, bb->octets, fromOffset, bitLength);

  /* update current size */
  bb->bitsInUse = max(bb->bitsInUse, toOffset+bitLength);

  return TRUE;
}




/*
   Desc: Set bits within buffer.
   */
int
bbSet(HBB bbH,
      IN  UINT32 fromOffset,
      IN  UINT32 bitLength,
      IN  UINT8 *src)
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb || !src) return RVERROR;
  if (fromOffset + bitLength > bb->maxOctets*8l)
    {  /* no place to hold bits */
      bb->isOverflowOfBuffer=TRUE;
      return RVERROR;
    }
  memcpyb(bb->octets, fromOffset, src, 0, bitLength);

  /* update current size */
  bb->bitsInUse = max(bb->bitsInUse, fromOffset+bitLength);

  return TRUE;
}





int
bbDelTail(HBB bbH,
      IN UINT32 numOfBits) /* to delete from tail of buffer */
{
  bbStruct *bb = (bbStruct *)bbH;
  int bytes;
  INT32 newLength, bits;

  if (!bb) return RVERROR;
  if (bb->bitsInUse < numOfBits) return RVERROR;
  if (numOfBits == 0) return TRUE;

  newLength = bb->bitsInUse - numOfBits;
  bytes = bbSetByte(newLength);
  bits = (8-newLength%8)%8;

  memset(bb->octets+bytes, 0, bb->maxOctets - bytes);
  bb->octets[bytes-1] = (UINT8)(bb->octets[bytes-1] >> bits);
  bb->octets[bytes-1] = (UINT8)(bb->octets[bytes-1] << bits);
  bb->bitsInUse = newLength;

  return TRUE;
}

/*
  Desc: delete bits from head of buffer.
  Note: copies, than deletes tail of buffer.
  */
int
bbDelHead(HBB bbH,
      IN UINT32 numOfBits) /* to delete from HEAD of buffer */
{
  bbStruct *bb = (bbStruct *)bbH;

  if (!bb) return RVERROR;
  if (bb->bitsInUse < numOfBits) return RVERROR;

  memcpyb(bb->octets, 0, bb->octets, numOfBits, bb->bitsInUse-numOfBits);
  return bbDelTail(bbH, numOfBits);
}
#ifndef NOLOGSUPPORT
#ifndef NOFILESYSTEM
/*_____________________________________________________________________*/
/*_________________________________DISPLAY______________________________*/
/*_____________________________________________________________________*/

int
bbDisplay(HBB bbH)
{
  bbStruct *bb = (bbStruct *)bbH;
  unsigned int i, j;

  if (!bb) {
    printf ("Buffer not allocated.\n");
    return RVERROR;
  }

  printf("Size=%d  Bytes=%d  Bits=%d  ",
     bb->maxOctets, bbBytesInUse(bbH), bbBitsInUse(bbH));
  (bb->isAligned)?(printf("Aligned ")):(printf("Not Aligned "));
  if (!bb->octets) {
      printf("Buffer octets field not allocated\n");
      return RVERROR;
  }

  printf("  Data: ");
  for (i=0; i < bbSetByte(bb->bitsInUse); i++) {
    for (j=0; j<8; j++) {
      if ( (bb->octets[i] & (1<<(7-j))) )
    printf("1");
      else
    printf("0");
    }
    printf(" ");
  }
  printf("\n");

  return TRUE;
}
#endif
#endif
#ifdef __cplusplus
}
#endif



