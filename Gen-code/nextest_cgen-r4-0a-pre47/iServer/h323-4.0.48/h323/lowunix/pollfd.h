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
  seli.h

  Select Interface Header file.

  Ron S. 14 Jan 1996.
  */


#ifndef _POLLFD_
#define _POLLFD_


#include <rvcommon.h>
#include <poll.h>

int
seliPollEventsRegistration(
               IN  int len,
               OUT struct pollfd *pollFdSet,
               OUT int *num,
               OUT UINT32 *timeOut
               );


int
seliPollEventsHandling(
               IN struct pollfd *pollFdSet,
               IN int  num,
               IN int numEvents
               );


#endif
#ifdef __cplusplus
}
#endif



