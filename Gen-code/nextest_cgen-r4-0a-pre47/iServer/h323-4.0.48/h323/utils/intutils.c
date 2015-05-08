#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/*
  intUtils.c

  Ron S.
  2 May 1996

  int conversion functions.

  */



#include <stdio.h>
#include <stdlib.h>
#include <intutils.h>
#include <ctype.h>
#include <ti.h>


BOOL /* true if platform is using big endian integers */
isPlatformBigEndian(void)
{
  static UINT32 check=1;
  return (*(char*)&check==0)?TRUE:FALSE;
}


UINT16 /* num representation in the opposite endian type */
intEndianConvert16(IN  UINT16 num)
{
  UINT16 flip=0;

  ((char*)&flip)[1]=((char*)&num)[0];
  ((char*)&flip)[0]=((char*)&num)[1];

  return flip;
}

UINT32 /* num representation in the opposite endian type */
intEndianConvert(IN  UINT32 num)
{
  UINT32 flip=0;

  ((char*)&flip)[3]=((char*)&num)[0];
  ((char*)&flip)[2]=((char*)&num)[1];
  ((char*)&flip)[1]=((char*)&num)[2];
  ((char*)&flip)[0]=((char*)&num)[3];

  return flip;
}


int
int32EndianChange( /* change content of buffer (32 bits integer) to other endian format */
          INOUT char* buffer)
{
  char tmp[4];
  int i;

  memcpy(tmp, buffer, 4);
  for (i=0; i<4; i++) buffer[i]=tmp[3-i];
  return TRUE;
}



/*
   Desc: convert integer to little endian
   Returns: buf or NULL.
*/
UINT8 *
int2LittleE(IN  UINT32 num,
        IN  int numOfBytes, /* to represent num in buffer */
        OUT UINT8 *buf)
{
  int pos=numOfBytes-1;

  if (!buf || numOfBytes <0) return NULL;

  if (numOfBytes >= 4) {
    buf[pos--] = (UINT8)(num>>24); /* n / 2^24 */
    num -= buf[pos+1]<<24;
  }

  if (numOfBytes >=3) {
    buf[pos--] = (UINT8)(num>>16); /* n / 2^16 */
    num -= buf[pos+1]<<16;
  }

  if (numOfBytes >= 2) {
    buf[pos--] = (UINT8)(num>>8); /* n / 2^8 */
    num -= buf[pos+1]<<8;
  }

  if (numOfBytes >= 1)
    buf[pos--] = (UINT8)num;

  return buf;
}



/*
   Desc: convert integer to big endian
   Returns: buf or NULL.
*/
UINT8 *
int2BigE(IN  UINT32 num,
     IN  int numOfBytes, /* to represent num in buffer */
     OUT UINT8 *buf)
{
  int pos=0;

  if (!buf || numOfBytes <0) return NULL;

  if (numOfBytes >= 4) {
    buf[pos++] = (UINT8)(num>>24); /* n / 2^24 */
    num -= buf[pos-1]<<24;
  }

  if (numOfBytes >=3) {
    buf[pos++] = (UINT8)(num>>16); /* n / 2^16 */
    num -= buf[pos-1]<<16;
  }

  if (numOfBytes >= 2) {
    buf[pos++] = (UINT8)(num>>8); /* n / 2^8 */
    num -= buf[pos-1]<<8;
  }

  if (numOfBytes >= 1)
    buf[pos++] = (UINT8)num;

  return buf;
}



/*
  Desc: convert big endian to integer
  Returns: num or RVERROR.
  */
/*int
bigE2Int(IN  UINT8 *buf,
     IN  int numOfBytes,
     OUT UINT32 *num)
{
  int pos=0;

  if (!buf || !num || numOfBytes<0) return RVERROR;

  *num=0;
  if (numOfBytes >= 4) *num+= ((UINT32)buf[pos++])<<24l;
  if (numOfBytes >= 3) *num+= ((UINT32)buf[pos++])<<16l;
  if (numOfBytes >= 2) *num+= ((UINT32)buf[pos++])<<8l;
  if (numOfBytes >= 1) *num+= buf[pos++];

  return 0;
}*/

int
bigE2Int(IN  UINT8 *buf,
     IN  int numOfBytes, /* in buffer */
     OUT UINT32 *num)
{
  static int platform;
  typedef enum {LITTLE=1,BIG=2} platformType;

  if (!buf || !num) return RVERROR;

  *num=0;
  if (platform==LITTLE)
  {
      if (numOfBytes == 4)
      {
          ((char*)num)[3]=buf[0];
          ((char*)num)[2]=buf[1];
          ((char*)num)[1]=buf[2];
          ((char*)num)[0]=buf[3];
      }
      else if (numOfBytes == 2)
      {
          ((char*)num)[1]=buf[0];
          ((char*)num)[0]=buf[1];
      }
      else if (numOfBytes == 1)
      {
          ((char*)num)[0]=buf[0];
      }
      else
      {
          ((char*)num)[2]=buf[0];
          ((char*)num)[1]=buf[1];
          ((char*)num)[0]=buf[2];
      }
  }
  else
  if (platform==BIG)
    {
      if (numOfBytes == 4)
      {
          ((char*)num)[0]=buf[0];
          ((char*)num)[1]=buf[1];
          ((char*)num)[2]=buf[2];
          ((char*)num)[3]=buf[3];
      }
      else if (numOfBytes == 2)
      {
          ((char*)num)[2]=buf[0];
          ((char*)num)[3]=buf[1];
      }
      else if (numOfBytes == 1)
      {
          ((char*)num)[3]=buf[0];
      }
      else
      {
          ((char*)num)[1]=buf[0];
          ((char*)num)[2]=buf[1];
          ((char*)num)[3]=buf[2];
      }
    }
  else
  {
      if (isPlatformBigEndian())
          platform=BIG;
      else
          platform=LITTLE;
      return bigE2Int(buf,numOfBytes,num);
  }

  return 0;
}


/*
  Desc: Check if data is a number.
  */
BOOL
isNumber(IN char *data) /* null terminated string. */
{
  char *p = data;

  if (!data) return FALSE;

  while (*p)
  {
    if (!isdigit((int)*p)) return FALSE;
    p++;
  }
  return TRUE;
}



#ifndef WIN32
char *
itoa(int i, char *a, int size)
{
  size=0;
  if (!a) return NULL;

  sprintf(a, "%d", i);
  return a;
}
#endif

/* prime numbers__________________________________________________________________________________*/

BOOL
isPrime(int n)
{
  int i;
  for (i=2; i<n/i/*sqrt(n)*/+1; i++)
    if (!(n%i)) return FALSE;
  return TRUE;
}

int /* first prime number avobe n */
intFirstPrime(int n)
{
  int i=n;

  if ((i & 1) == 0) i++;
  while (!isPrime(i)) i+=2;
  return i;
}


RVAPI RvRandomGenerator* UTILS_RandomGeneratorConstruct(RvRandomGenerator* r,
                                                  RvRandomType seed) 
{
	r->state = seed;  
	return r;
}

RVAPI unsigned int UTILS_RandomGeneratorGetValue(RvRandomGenerator* r) {
	return ((r->state = r->state * 1103515245 + 12345) & RAND_MAX);
}



#ifdef __cplusplus
}
#endif

