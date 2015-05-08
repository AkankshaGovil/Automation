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
#include <ti.h>
#include <rvinternal.h>
#include <cmintr.h>
#include <Threads_API.h>
#include <tls.h>
#include <intutils.h>

typedef struct
{
    BYTE macAddress[6];
    BYTE time[8];
    BOOL clock_seq_change;
} uCID;

typedef struct
{
     UINT32 startTime;
     UINT32 startMilliTime;
     UINT32 oldMilliTime;
     BYTE lastTime[8];
     BOOL notStart;
     BOOL generated;
     uCID u;
     BYTE cid[16];
     BYTE oldCid[16];
     UINT16 clock_seq;
} cidLocalStorage;

void getTime(BYTE*theTime, BOOL *clockSeqChange)
{
    UINT32 milliTime;
    UINT32 secTime;



    cidLocalStorage*   cidTls;
    RvH323ThreadHandle threadId;

    threadId = RvH323ThreadGetHandle();
    cidTls = (cidLocalStorage *)THREAD_GetLocalStorage( threadId,
                                                        tlsIntrCid,
                                                        sizeof(cidLocalStorage));
    if (cidTls == NULL)
        return;

    if (!cidTls->startTime)
    {
        cidTls->startTime=timerGetTimeInSeconds()/3*5+0xb21dd213u;
        cidTls->startMilliTime=timerGetTimeInMilliseconds();
    }

    milliTime=timerGetTimeInMilliseconds()-cidTls->startMilliTime;

    *clockSeqChange  = (!cidTls->notStart) ||  /* first time */
                       ((INT32)milliTime<0) || /* after reboot or overflow */
                       ((cidTls->oldMilliTime - milliTime) == 0); /* in the same millisec as before */
    cidTls->notStart = TRUE;
    cidTls->oldMilliTime = milliTime;

    secTime=milliTime/600+cidTls->startTime;

    milliTime%=600;
    milliTime*=27777;

    theTime[0]=2;
    theTime[1]=(BYTE)(secTime>>24);
    theTime[2]=(BYTE)(secTime>>16);
    theTime[3]=(BYTE)(secTime>>8);
    theTime[4]=(BYTE)(secTime);
    theTime[5]=(BYTE)(milliTime>>16);
    theTime[6]=(BYTE)(milliTime>>8);
    theTime[7]=(BYTE)(milliTime);
}

void getMACaddress(RvRandomGenerator*seed,BYTE*theMAC)
{
    int i;

    for (i = 0; i < 6; i++)
        theMAC[i] = (BYTE)UTILS_RandomGeneratorGetValue(seed);
    theMAC[5] |= 0x01;
}

char* getCID(RvRandomGenerator*seed)
{

  cidLocalStorage*   cidTls;
  RvH323ThreadHandle threadId;

  threadId = RvH323ThreadGetHandle();
  cidTls = (cidLocalStorage *)THREAD_GetLocalStorage(   threadId,
                                                        tlsIntrCid,
                                                        sizeof(cidLocalStorage));
  if (cidTls == NULL)
      return NULL;

  if (!cidTls->clock_seq)
      cidTls->clock_seq=(UINT16)UTILS_RandomGeneratorGetValue(seed);/* init to a random number */

  /* set time_low , time_mid, time_hi_and_version fields - 0-7 octets of cid */
  getTime(cidTls->u.time, &cidTls->u.clock_seq_change);
  memcpy(cidTls->cid,cidTls->u.time,8);

  /* 4 most significant bits of 6-7 bytes set to version number - '0001' */
  *((UINT16*)(cidTls->cid+6)) &= 0x1fff;
  *((UINT16*)(cidTls->cid+6)) |= 0x1000;


  /* set clock_seq_hi_and_reserved and clock_seq_low  fields - 8,9 octets of cid */

  if (cidTls->u.clock_seq_change)
  {
    cidTls->clock_seq++;
    cidTls->clock_seq %= 16384; /* 16384 - '0100000000000000' */
  }

  cidTls->cid[9] = (BYTE) cidTls->clock_seq; /* set clock_seq_low to the 8 LSBs*/
  cidTls->cid[8] = (BYTE) (cidTls->clock_seq>>8); /* set clock_seq_hi */
  cidTls->cid[8] &= 0xbf; /* set 2 MSBs to '10' */

  /* set node  - 10 -15 octets of cid */
  if (!cidTls->generated)
    {
      cidTls->generated=TRUE;
      getMACaddress(seed, cidTls->u.macAddress);
    }

  memcpy(cidTls->cid+10,cidTls->u.macAddress,6);

  if (memcmp(cidTls->cid, cidTls->oldCid, 16) == 0)
      printf("identical cids on thread %x!!!\n",(unsigned)threadId);
  memcpy(cidTls->oldCid, cidTls->cid, 16);

  return (char*)cidTls->cid;

}
#ifdef __cplusplus
}
#endif



