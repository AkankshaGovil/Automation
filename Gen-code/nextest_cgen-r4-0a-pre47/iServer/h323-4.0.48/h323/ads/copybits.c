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
  copybits.c

  Sasha F.
  */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <copybits.h>



void
setBit(BYTE *ptr, UINT32 bit, unsigned int value)
{
    if (value)
        ptr[bit>>3] |=  (0x80 >> (bit & 7));
    else
        ptr[bit>>3] &= ~(0x80 >> (bit & 7));
}

void
memcpyb(BYTE *dest, UINT32 destBitPos, unsigned char  *src, UINT32 srcBitPos, UINT32 numOfBits)
{
    UINT32 i;

    if ((srcBitPos&7) == 0 && (destBitPos&7) == 0 && numOfBits>=8)
    {
        for (i=numOfBits-(numOfBits&7); i<numOfBits; i++)
        {
            setBit(dest, destBitPos+i, getBit(src, srcBitPos+i));
        }
        memmove(dest + (destBitPos>>3), src + (srcBitPos>>3), (int)(numOfBits>>3));
    }
    else {
        /* optimization: looping down to 0 is faster than looping up to
           numOfBits on most machines */
        for (i = numOfBits; i > 0; i--)
        {
            setBit(dest, destBitPos++, getBit(src, srcBitPos));
            srcBitPos++;
        }
    }
}

static const char the1st0InByte[]=
{
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/*0*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*1*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*2*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*3*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*4*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*5*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*6*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*7*/   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*8*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*9*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*A*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*B*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*C*/   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*D*/   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*E*/   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
/*F*/   4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,
};


UINT32
get1st0BitNumber(BYTE *ptr,UINT32 startBit, UINT32 stopBit)
{
    UINT32 i,bit;
    BYTE ext = (BYTE)(startBit&7);

    if (ext)
    {
        BYTE c=ptr[startBit>>3];
        c|=~((0x80>>(ext-1))-1);
        if (c!=0xff)
        {
            bit=the1st0InByte[c];
            return bit+(startBit-ext);
        }
        startBit += (8-ext);
    }
    for (i=startBit>>3;i<(stopBit+8)>>3;i++)
        if (ptr[i]!=0xff)
            break;
    if (i<((stopBit+8)>>3))
    {
        int bit=the1st0InByte[ptr[i]]+(i<<3);
        return bit;
    }
    return stopBit+1;
}




#ifdef __cplusplus
}
#endif
