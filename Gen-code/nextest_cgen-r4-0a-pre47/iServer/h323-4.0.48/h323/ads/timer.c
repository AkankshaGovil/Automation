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
  timer.c

  */

#include <stdio.h>
#include <timer.h>
#include <ms.h>
#include <ti.h>


typedef struct {
  TimerHandler callback;
  timerKey key;
  UINT32 interval;
  UINT32 absTime;
} timerNode;

typedef struct {
  HLIST  tlist; /* timers list */
  UINT32 NextExpiration;
} timerStruct;

/*______________________________time_list___________________________________*/
BOOL timerNodeCompare(Element e1, Element e2)
{
  timerNode *n1 = (timerNode *)e1;
  timerNode *n2 = (timerNode *)e2;

  if (n1->key == n2->key &&
      n1->callback == n2->callback)
    return TRUE;
  else
    return FALSE;
}


/*__________________________Timer_Register_Function________________________*/

/* Desc: Init timer. Return handler to timers.
   Input: size- maximum number of timers.
   Returns: Handler for timers.
*/
HTIMER
timerConstruct(int maxTimers, RVHLOGMGR logMgr)
{
  timerStruct *timer;

  if (maxTimers < 1) return NULL;

  timer = (timerStruct *)calloc(sizeof(timerStruct), 1);
  timer->tlist = listConstruct((int)sizeof(timerNode), maxTimers, logMgr, "Timer nodes");
  listSetCompareFunc(timer->tlist, timerNodeCompare);

  timer->NextExpiration = (UINT32)RVERROR;

  return (HTIMER )timer;
}

int
timerDestruct(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;

  if (!timer) return RVERROR;
  listDestruct(tm->tlist);
  free(tm);
  timer=NULL;
  return TRUE;
}

int
timerClear(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;

  if (!timer) return RVERROR;
  listClear(tm->tlist);
  tm->NextExpiration = (UINT32)RVERROR;
  return TRUE;
}


/*
  Desc: set elapse time to minimum of all timers intervals.
*/

static void
timerSetNextExpiration(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;
  timerNode *elem;
  int loc;
  UINT32 t;
  int refresh;

  if (!timer) return; /* NULL function */
  t = timerGetTimeInMilliseconds();
  if ( ((loc=listHead(tm->tlist)))>=0) {
    elem = (timerNode *)listGetElem(tm->tlist, loc);
    refresh = elem->absTime-t;

    for (; loc>=0; loc=listNext(tm->tlist, loc)) {
      elem = (timerNode *)listGetElem(tm->tlist, loc);
      refresh = min(refresh, (int)(elem->absTime-t));
    }
    tm->NextExpiration=t+refresh;
  }
  else
    tm->NextExpiration = (UINT32)0x7fffffff+t; /* empty list */
}



/* Desc: Register time out callback function for select()ing.
   Set Next elapse time.
   Note: negative timeout ==> callback is deleted from list.
   Input: callback- timer callback. called when expired.
          key- timer descriptors.
          timeOut- in msec.
  */
HTIMERELEM
timerAdd(HTIMER timer, TimerHandler callback, timerKey key, UINT32 timeOut)
{
  timerStruct *tm = (timerStruct *)timer;
  timerNode node;
  int loc;

  if (!timer) return NULL; /* NULL function */

  node.callback = callback;
  node.key = key;
  node.interval = timeOut;
  node.absTime = timerGetTimeInMilliseconds() + timeOut;

  if ( (loc = listFind(tm->tlist, listHead(tm->tlist), (void *)&node)) >=0)
    listDelete(tm->tlist, loc);

  loc = listAddHead(tm->tlist, (Element)&node);
  timerSetNextExpiration(timer);
  return ((HTIMERELEM)loc);
}



/* Desc: Delete time out timer.
   Set Next elapse time.
   Input: callback- timer callback. called when expired.
          key- timer descriptors.
  */
int
timerDelete(HTIMER timer, HTIMERELEM tNode)
{
  timerStruct *tm = (timerStruct *)timer;
  int location = (int)tNode;

  if (!timer) return RVERROR; /* NULL function */
  if (! (listGetElem(tm->tlist, location) ))
    return RVERROR;

  if ( ! listDelete(tm->tlist, location) )
    return RVERROR;
  timerSetNextExpiration(timer);
  return TRUE;
}

HTIMERELEM
timerFind(HTIMER timer, TimerHandler callback, timerKey key)
{
  timerStruct *tm = (timerStruct *)timer;
  timerNode node;
  int loc=0;

  if (!timer) return (HTIMERELEM)RVERROR; /* NULL function */

  node.callback = callback;
  node.key = key;

  if ( (loc = listFind(tm->tlist, listHead(tm->tlist), (void *)&node))<0 )
    return (HTIMERELEM)RVERROR;
  else
    return ((HTIMERELEM)loc);
}

int
timerDeleteByValue(HTIMER timer, TimerHandler callback, timerKey key)
{
  int loc=0;

  if (!timer) return RVERROR; /* NULL function */

  if ( (HTIMERELEM)(loc = (int)timerFind(timer,callback, key)) == (HTIMERELEM)RVERROR )
    return RVERROR;

  return ( timerDelete(timer, (HTIMERELEM)loc));
}


/*
  Desc: check if one of the time outs expired, and call it if so.
  Called after NextElapseTime msec from the last call.
*/
int
timerCheck(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;
  timerNode *elem;
  int loc, t;

  if (!timer) return RVERROR;
  t = timerGetTimeInMilliseconds();
  for (loc=listHead(tm->tlist); loc>=0; ) {
    elem=(timerNode *)listGetElem(tm->tlist, loc);
    loc=listNext(tm->tlist, loc);
    if ((int)(t - elem->absTime) >= 0 ) {  /* time expired */
      elem->absTime += elem->interval;
      if (elem->callback) elem->callback(elem->key);
    }
  }
  timerSetNextExpiration(timer);
  return TRUE;
}

/*
  Desc: return next expiration time in msec.
*/
UINT32
timerGetNextExpiration(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;

  if (!timer) return (UINT32)RVERROR;
  return tm->NextExpiration;
}

int
timerGetCount(HTIMER timer)
{
  timerStruct *tm = (timerStruct *)timer;

  if (!timer) return (UINT32)RVERROR;
  return listCurSize(tm->tlist);
}


/*
  Desc: return next elapse time in msec.
*/
#ifndef NOLOGSUPPORT

int
timerDisplay(
         HTIMER timer,
         int msa
         )
{
  timerStruct *tm = (timerStruct *)timer;

  if (!timer) return RVERROR;
  listPrint(tm->tlist, (void*)(INTPTR)msa);
  return TRUE;
}


#endif
