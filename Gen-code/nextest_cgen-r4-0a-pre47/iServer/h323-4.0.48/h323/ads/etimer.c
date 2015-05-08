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
  etimer.c

  */

#include <stdio.h>
#include <ms.h>
#include <mti.h>
#include <ti.h>
#include <rlist.h>
#include <bheap.h>
#include <etimer.h>
#include <ms.h>

typedef struct {
  ETimerHandler callback;
  void *param;
  UINT32 interval;
  UINT32 absTime;
  void *heapRef;
} etimerNode;


typedef struct {
  HLIST  timer;
  HBHEAP heap;
} etimerStruct;

static int msaTimer=0;


static BOOL etimerCompareCB(void *e1, void *e2);
static int etimerheapCompareCB( void *elem1, void *elem2, void *param);
static int etimerheapUpdateCB( void **elem, void *param );

static etimerNode *etimerGetMinElement(HETIMER timer );
static int etimerUpdateMinElem( HETIMER timer );

/***************************************************************************/
/*                          PUBLIC FUNCTIONS                               */
/***************************************************************************/
HETIMER
etimerConstruct(int maxTimers)
{
  etimerStruct *tm;

  if (maxTimers < 1) return NULL;

  if (!msaTimer  )
    msaTimer = msaRegister(0, "ETIMER", "ADS ETIMER INTERFACE");

  if ( !( tm = (etimerStruct *)calloc(sizeof(etimerStruct), 1) ))
    return NULL;
  tm->timer =
      listConstruct((int)sizeof(etimerNode), maxTimers, (RVHLOGMGR)1, "ETIMER");
  listSetCompareFunc(tm->timer, etimerCompareCB);

  tm->heap = bheapConstruct( maxTimers,
                 etimerheapCompareCB,
                 etimerheapUpdateCB,
                 (void *)(tm->timer));

  return (HETIMER )tm;
}

int
etimerDestruct(HETIMER timer)
{
  etimerStruct *tm = (etimerStruct *)timer;

  if (!timer) return RVERROR;
  listDestruct(tm->timer);
  bheapDestruct(tm->heap);
  free(tm);
  timer=NULL;
  return TRUE;
}


HETIMERELEM
etimerAdd(HETIMER timer, ETimerHandler callback, void *param, UINT32 timeOut)
{
  etimerStruct *tm = (etimerStruct *)timer;
  etimerNode node;
  int loc;

  if (!timer) return (HETIMERELEM)RVERROR; /* NULL function */

  memset(&node, 0, sizeof(node));
  node.callback = callback;
  node.param = param;
  node.interval = timeOut;
  node.absTime = timerGetTimeInMilliseconds() + timeOut;

 /* if ( (loc = listFind(tm->timer, listHead(tm->timer), (void *)&node)) >=0)
    etimerDelete(timer, (HETIMERELEM)loc);*/

  if (( loc = listAddHead(tm->timer, (void *)&node) ) < 0)
    return (HETIMERELEM)RVERROR;

  if( bheapInsert(tm->heap, (BHeapNode)(loc)) == RVERROR )
    return (HETIMERELEM)RVERROR;

  return ((HETIMERELEM)loc);
}



int
etimerDelete(HETIMER timer, HETIMERELEM tNode)
{
  etimerStruct *tm = (etimerStruct *)timer;
  int location = (int)tNode;
  etimerNode *node;
  int loc;

  if (!timer) return RVERROR; /* NULL function */
  if (! (node = (etimerNode*)listGetElem(tm->timer, location) ))
    return RVERROR;
  loc = (int)(bheapDeleteNode(tm->heap, (BHeapNode*)node->heapRef ));

    if (loc!=location)
    {
        loc=location;
    }

  listDelete(tm->timer, location);
  return TRUE;
}

HETIMERELEM
etimerFind(HETIMER timer, ETimerHandler callback,void *param)
{
  etimerStruct *tm = (etimerStruct *)timer;
  etimerNode node;
  int loc=0;

  if (!timer) return (HETIMERELEM)RVERROR; /* NULL function */

  node.callback = callback;
  node.param = param;

  if ( (loc = listFind(tm->timer, listHead(tm->timer), (void *)&node))<0 )
    return (HETIMERELEM)RVERROR;
  else
    return ((HETIMERELEM)loc);
}

int
etimerGetParams(
        IN HETIMER timer,
        IN HETIMERELEM tNode,
        OUT ETimerHandler *callback,
        OUT void **param
        )
{
  etimerStruct *tm = (etimerStruct *)timer;
  int location = (int)tNode;
  etimerNode *node;


  if (!timer) return RVERROR; /* NULL function */
  if (! (node = (etimerNode*)listGetElem(tm->timer, location) ))
    return RVERROR;

  *callback = node->callback;
  *param = node->param;

  return OK;
}


int
etimerDeleteByValue(HETIMER timer, ETimerHandler callback, void *param)
{
  int loc=0;

  if (!timer) return RVERROR; /* NULL function */

  if ( (loc = (int)etimerFind(timer,callback, param)) == RVERROR )
    return RVERROR;

  return ( etimerDelete(timer, (HETIMERELEM)loc));
}


/*
  Desc: check if one of the time outs expired, and call it if so.
  Called after NextElapseTime msec from the last call.
*/
int
etimerCheck(HETIMER timer, HMEI mutex)
{
  etimerNode *elem;
  UINT32 t, nextTimer;

  if (!timer) return RVERROR;
  t = timerGetTimeInMilliseconds();
  while( (nextTimer = etimerGetNextExpiration(timer)) != RVERROR ) 
  { 
    BOOL t_rollOver=FALSE;
    BOOL next_rollOver = FALSE; 
    BOOL callTimer=FALSE;
	UINT32 diff;

	if (t>nextTimer)
      diff=t-nextTimer;
	else diff=nextTimer-t;

    if ((t<nextTimer) && (diff>0x80000000))
    {
       t_rollOver = TRUE;
    }
    if ((nextTimer<t) && (diff>0x80000000))
    {
       next_rollOver = TRUE;
    }

    if ((t>= nextTimer) && (!next_rollOver))
      callTimer = TRUE;
 
    if ((t < nextTimer) && (t_rollOver))
    {
      /* t has passed 0xFFFFFFFF mark. Actually t is bigger then next */
      callTimer = TRUE; 
    }

    if (!callTimer)
    {
      /* leave while loop*/
      break;
    }
    
    if ( !(elem = etimerGetMinElement( timer ) ) )
        continue;
    elem->absTime += elem->interval;
    if (elem->callback)
    {
        void * param = elem->param;
        ETimerHandler callback = elem->callback;
        msaPrintFormat(msaTimer, "etimerCheck: callback=0x%p param=0x%p interval=%d abstime=%d",
             elem->callback, elem->param, elem->interval, elem->absTime);
        meiExit(mutex);
        callback(param);
        meiEnter(mutex);
    }

      etimerUpdateMinElem( timer );
  }

  return TRUE;
}


UINT32 etimerGetNextExpiration(
                   HETIMER timer
                   )
{
  etimerStruct *tm = (etimerStruct *)timer;
  etimerNode *node;
  int loc=0;

  if ( !timer ) return (UINT32)RVERROR;

  if (( loc = (int)bheapTop(tm->heap) ) < 0 )
    return (UINT32)RVERROR;

  if ( ! (node = (etimerNode *)listGetElem(tm->timer, loc) ) )
    return (UINT32)RVERROR;

  return ( node->absTime );

}

/*
  Desc: return next elapse time in msec.
*/

#ifndef NOLOGSUPPORT
int
etimerDisplay(
         HETIMER timer,
         int msa
         )
{
  etimerStruct *tm = (etimerStruct *)timer;
  if(msa);

  if (!timer) return RVERROR;
  listPrint(tm->timer, (void*)(INTPTR)msaTimer);
  return TRUE;
}
#endif
int etimerGetCount(
          HETIMER timer
          )
{
  etimerStruct *tm = (etimerStruct *)timer;

  if (!timer) return RVERROR;

  return ( listCurSize(tm->timer ) );
}

void etimerPrintMinElement(
               HETIMER timer
               )
{
  etimerNode *node;

  if (!( node = etimerGetMinElement(timer) ))
    return;
  if ( etimerGetCount(timer) > 0)
  {
    msaPrintFormat(msaTimer,"etimerPrintMinElement: HETIMER=%x absTime=%u interval=%d callback=0x%p(0x%p)",
           timer, node->absTime, node->interval, node->callback, node->param);
  }
  else
    msaPrintFormat(msaTimer,"etimerPrintMinElement: HETIMER=%x None of the timers are set !!!!!",timer);

}

/*********************************************************************************/
/*                                 LOCAL FUNCTIONS                               */
/*********************************************************************************/


static BOOL etimerCompareCB(void *e1, void *e2)
{
  etimerNode *n1 = (etimerNode *)e1;
  etimerNode *n2 = (etimerNode *)e2;

  if (n1->param == n2->param &&
      n1->callback == n2->callback)
    return TRUE;
  else
    return FALSE;
}

static int etimerheapCompareCB( void *elem1, void *elem2, void *param)
     /* Note: if the arguments are not valid 0 will be returned */
{
  int loc1 = (int)elem1;
  int loc2 = (int)elem2;
  HLIST raH = (HLIST)param;
  etimerNode *e1, *e2;
  UINT32 diff;

  if ( loc1<0 || loc2<0 )
    return 0;

  if( !( e1 = (etimerNode *)listGetElem(raH, loc1) ) )
    return 0;

  if (! ( e2 = (etimerNode *)listGetElem(raH, loc2)))
    return 0;

  if ( e1->absTime == e2->absTime )
    return 0;

/* if e2 is smaller it will be inserted later on the first place.
  Now the "1" will be returned.
  The problem arise when e2 is smaller because of time rollover.
  The rollover event can be identified if the difference between 2 elements
  is bigger 0x80000000 which is about 24 days*/
  if (e1->absTime  > e2->absTime)
	diff = e1->absTime - e2->absTime;  
  else 
	diff = e2->absTime - e1->absTime;  

  if (( e1->absTime > e2->absTime) && (diff<0x80000000))
  {
    /* examples
	   e1=100,     e2=80       result e2 is smaller
	   e1=F0000100 e2=F000080  result e2 is smaller
	   e1=F0000100 e2=0000080  result e2 is NOT smaller, because diff > 0x80000000 
	 */
    return 1;
  }
  else
  if (( e1->absTime < e2->absTime) && (diff>0x80000000))
  { 
    /* examples
	   e1= 100 e2=F000080 result e2 is SMALLER, 
	                      because diff > 0x80000000 which means e1 after rollover
	 */
    return 1;
  } 
  return -1;

}

static int etimerheapUpdateCB( void **elem, void *param )
{
  int loc;
  HLIST raH = (HLIST)param;
  etimerNode *e;

  if ( !elem )
    return RVERROR;

  loc = (int)(*elem);
  if ( !(e = (etimerNode *)listGetElem(raH, loc)))
    return RVERROR;
  e->heapRef = elem;
  return OK;

}



static etimerNode *etimerGetMinElement(
                    HETIMER timer
                    )
{
  etimerStruct *tm = (etimerStruct *)timer;
  int loc=0;

  if ( !timer ) return NULL;

  if (( loc = (int)bheapTop(tm->heap) ) < 0 )
    return NULL;

  return ( (etimerNode *)listGetElem(tm->timer, loc));
}

static int etimerUpdateMinElem(
                   HETIMER timer
                   )
{
  etimerStruct *tm = (etimerStruct *)timer;

  if ( !timer ) return RVERROR;

  return (bheapUpdateTop(tm->heap) );
}

#ifdef __cplusplus
}
#endif



