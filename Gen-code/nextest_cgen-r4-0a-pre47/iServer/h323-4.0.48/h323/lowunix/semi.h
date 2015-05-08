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
  semi.h

  Semaphore Low Level Interface

  Currently supported for VxWorks Binary semaphores for mutual exclusion.

  Ron S.
  26 Nov. 1996

  */


#ifndef _SEMI_
#define _SEMI_

#include <rvinternal.h>

DECLARE_OPAQUE(HSEMI);

#ifdef IS_PLATFORM_SOLARIS
union semun {
               int val;
               struct semid_ds *buf;
               ushort *array;
          };
#endif

#ifdef IS_PLATFORM_SOLARISPC
union semun {
               int val;
               struct semid_ds *buf;
               ushort *array;
          };
#endif

#ifdef IS_PLATFORM_HPUX
union semun {
               int val;
               struct semid_ds *buf;
               ushort *array;
          };
#endif

#ifdef UNIXWARE
union semun {
               int val;
               struct semid_ds *buf;
               ushort *array;
          };
#endif

#ifdef _ALPHA_
union semun {
               int val;
               struct semid_ds *buf;
               ushort *array;
          };
#endif

#ifdef __REDHAT__

union semun {
               int val;                    /* value for SETVAL */
               struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
               unsigned short int *array;  /* array for GETALL, SETALL */
               struct seminfo *__buf;      /* buffer for IPC_INFO */
       };

#endif

int
semiInit(void);

int
semiEnd(void);

HSEMI
semiConstruct(void);

int
semiDestruct(
         HSEMI semiH
         );

int
semiGive(
     HSEMI semiH
     );

int
semiTake(
     HSEMI semiH
     );




#endif
#ifdef __cplusplus
}
#endif



