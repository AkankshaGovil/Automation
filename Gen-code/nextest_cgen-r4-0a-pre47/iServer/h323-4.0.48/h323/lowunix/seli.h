/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced IN  any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SELI_H
#define __SELI_H

#include <rvcommon.h>
#include <Threads_API.h>

typedef enum {
  seliEvRead=0x01,
  seliEvWrite=0x02,
  seliEvExept=0x04
} seliEvents;

/*  Desc: Init SELI module.  */
RVAPI int RVCALLCONV seliInit(void);

/*  Desc: End module operation.  */
RVAPI int RVCALLCONV seliEnd(void);

RVAPI
int RVCALLCONV seliSelect(void);

typedef void (RVCALLCONV *seliCallback)(int fd, seliEvents sEvent, BOOL error);

RVAPI
int RVCALLCONV seliCallOn(int fd, seliEvents sEvent, seliCallback _callback);

RVAPI
int RVCALLCONV seliCallOnThread(int fd, seliEvents sEvent, seliCallback _callback, RvH323ThreadHandle threadId);



/* The following functions are only relevant for systems supporting the select() interface */

#ifdef fd_set

int
seliSelectEventsRegistration(
                 IN  int fdSetLen,
                 OUT int *num,
                 OUT fd_set *rdSet,
                 OUT fd_set *wrSet,
                 OUT fd_set *exSet,
                 OUT UINT32 *timeOut);

int
seliSelectEventsHandling(
            IN fd_set *rdSet,
            IN fd_set *wrSet,
            IN fd_set *exSet,
            IN int num,
            IN int numEvents);

#endif  /* fd_set */



#endif /* __SELI_H */

#ifdef __cplusplus
}
#endif
