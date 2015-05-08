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
  semi.c

  Low Level Binary Semaphore Interface

  VxWorks (Gateway) version - Ron S., 26 Nov. 1996
  Unix version              - Oz S.,  22 Jun. 1997

  */

#ifdef __VXWORKS__
#include <semLib.h>
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif

#ifndef SEM_A  /* not defined in LINUX */
#define SEM_A   0200
#define SEM_R   0400
#endif

#include <semi.h>


int
semiInit(void)
{
  return TRUE;
}


int
semiEnd(void)
{
  return TRUE;
}


HSEMI /* id or NULL */
semiConstruct(void)
{
#ifdef __VXWORKS__
  /* create a binary semaphore that is initially full */
  return (HSEMI)semBCreate(SEM_Q_PRIORITY, SEM_FULL);
#else
  int semID;
  union semun sem;
  semID = semget(IPC_PRIVATE, 1, SEM_A + SEM_R);
  if (semID >= 0)
    {
      sem.val=1;
      semctl(semID, 0, SETVAL, sem);
    }
  return (HSEMI)(semID + 1);
#endif
}

int
semiDestruct(
         HSEMI semiH
         )
{
#ifdef __VXWORKS__
  return semDelete((SEM_ID)semiH);
#else
  union semun sem;
  sem.val=0;
  return semctl(((int)semiH) - 1, 0, IPC_RMID, sem);
#endif
}

int
semiGive(
     HSEMI semiH
     )
{
#ifdef __VXWORKS__
  return semGive((SEM_ID)semiH);
#else
 struct sembuf op;
  op.sem_num = (ushort)0;
  op.sem_op = (short)1;
  op.sem_flg = (short) SEM_UNDO;
  return semop(((int)semiH) - 1, &op, 1);
#endif
}

int
semiTake(
     HSEMI semiH
     )
{
#ifdef __VXWORKS__
  return semTake((SEM_ID)semiH, WAIT_FOREVER);
#else
  struct sembuf op;
  op.sem_num = (ushort)0;
  op.sem_op = (short)-1;
  op.sem_flg = (short) SEM_UNDO;

  return semop(((int)semiH) - 1, &op, 1);
#endif
}
#ifdef __cplusplus
}
#endif



