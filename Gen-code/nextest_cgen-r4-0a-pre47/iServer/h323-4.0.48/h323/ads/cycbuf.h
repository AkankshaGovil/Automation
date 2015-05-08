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
  CycBuf.h

  Cyclic buffer implementation headers.

  Supports reading and writing of arbitarary number of bytes up to some top
  limit.

  Ron S.
  15 Oct 1995
  */

#ifndef _CYCBUF_
#define _CYCBUF_

#include <stdio.h>

#include <rvinternal.h>


typedef struct {
  int size;         /* size of buffer */
  char *buffer;     /* buffer holding the data */
  int toRead;       /* position of first byte to read from */
  int toWrite;      /* position of first byte to write to */
} cycBuf;


cycBuf *cbInit(int size);
int cbDistruct(cycBuf *cb);
int cbClear(cycBuf *cb);

int cbDataSize(cycBuf *cb);
int cbFreeSize(cycBuf *cb);

int cbPeek(cycBuf *cb, char *buf, int size);
int cbRead(cycBuf *cb, char *buf, int size);
int cbWrite(cycBuf *cb, char *buf, int size);
int cbDel(cycBuf *cb, int size);

#endif
#ifdef __cplusplus
}
#endif



