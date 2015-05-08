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
  pi.h

  Pipe Interface library.

  Ron S. 14 Jan. 1996
*/

#ifndef __PI__
#define __PI__

#include <rvinternal.h>

typedef enum {
  piEvRead=0x01,
  piEvWrite=0x02,
  piEvClose=0x04
} piEvents;

typedef void (*piCallback) (int handle, piEvents event, BOOL error, void* context);

int  piInit(void);
int  piEnd(void);

int  piCallOn(int handle, piEvents lEvent, piCallback callback, void* context);
void piDisplay(void);
void piCloseAll(void);

int piOpen(char *name, int msgSize, int maxMsgs);
int piClose(int handle);

int piRead(int handle, UINT8 *buf, int len);
int piWrite(int handle, UINT8 *buf, int len);

int piBytesAvailable(int sockId,int *bytesAvailable);

int
piUnblock(
      /*  Desc: Make socket non-blocked.  */
      int handle
      );

int
piBlock(
    /* Desc: Make socket blocked.  */
    int handle
    );


#endif


#ifdef __cplusplus
}
#endif



