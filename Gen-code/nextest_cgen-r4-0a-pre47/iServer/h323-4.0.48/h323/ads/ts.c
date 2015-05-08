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
  ts.c

  */

#include <rvinternal.h>
#include <rlist.h>
#include <ra.h>
#include <ms.h>
#include <etimer.h>
#include <mei.h>
#include <ts.h>


typedef struct
{
    HLIST               tl;
    UINT32              min;
    UINT                count;
    BOOL                tsWasSet; /* boolian stating if someone called tsSetTimer() */
    RvH323ThreadHandle  threadId;
} tHandle;


typedef struct
{
    tHandle*            tsHandle;
    HETIMER             tHndl;
    HTSAPPTIMER         appHndl;
    HMEI                mutex;
} tsNode;



/*________________________________ Array functions ________________________________________ */
BOOL tsElementCompare( Element e1, Element e2 )
{
  tsNode *el1 = (tsNode *)e1;
  tsNode *el2 = (tsNode *)e2;

  if( el1->appHndl == el2->appHndl )
    return TRUE;
  else
    return FALSE;
}


/*_______________________________________timer section______________________________________*/

HTSTRUCT
tsConstruct(int count, RVHLOGMGR logMgr)
{

  tHandle *tH;

  if (count < 1) return NULL;

  if ( !(tH = (tHandle *)calloc(sizeof(tHandle),1)) )
    return NULL;
  if ( !(tH->tl = listConstruct((int)sizeof(tsNode), count, logMgr, "TS list")) )
    return NULL;
  listSetCompareFunc(tH->tl, tsElementCompare);
  tH->min = (UINT32)-1;
  tH->count = 0;
  tH->tsWasSet = FALSE;
  tH->threadId = RvH323ThreadGetHandle();

  return (HTSTRUCT)tH;

}

int
tsDestruct(HTSTRUCT tH)
{
  tHandle *t = (tHandle *)tH;

  if (!tH) return RVERROR;
  listDestruct((HLIST)t->tl);
  free(t);
  tH=NULL;
  return TRUE;
}

int
tsClear(HTSTRUCT tH)
{
  tHandle *t = (tHandle *)tH;

  if (!tH) return RVERROR;
  listClear(t->tl);
  t->min = (UINT32)-1;
  t->count =  0;
  t->tsWasSet =  FALSE;
  return TRUE;
}


HTSTIMER
tsAdd(HTSTRUCT tH, int maxTimers, HTSAPPTIMER appHndl)
{
  tHandle *tm = (tHandle *)tH;
  tsNode node;
  int loc;

  if (!tH) return NULL;

  node.appHndl = appHndl;

  /* return NULL if appHndl already exist at list */
  if( appHndl &&
      ( (loc = listFind(tm->tl, listHead(tm->tl), (void *)&node))>=0))
    return NULL;

  if ( !(node.tHndl = etimerConstruct(maxTimers))) return NULL;
  node.tsHandle = tm;
  node.mutex = meiInit();

  if ( (loc = listAddHead((HLIST)(tm->tl), (Element)&node))<0)
  {
      etimerDestruct(node.tHndl);
      meiEnd(node.mutex);
      return NULL;
  }

  return ( (HTSTIMER)listGetElem((HLIST)(tm->tl), loc ) );

}


HTSTIMER
tsFind(HTSTRUCT tH, HTSAPPTIMER appHndl)
{
  tHandle *tm = (tHandle *)tH;
  int loc;
  tsNode node;

  if (! tH ) return NULL;

  node.appHndl = appHndl;
  if( (loc = listFind(tm->tl, listHead(tm->tl), (void *)&node))<0)
    return NULL;

  return ( (HTSTIMER)listGetElem((HLIST)(tm->tl), loc ) );
}


int
tsDelete(HTSTIMER timer)
{
  tHandle *t;
  int location;
  tsNode *node = (tsNode *)timer;

  if ( ! timer ) return RVERROR;
  t = node->tsHandle;

  if ( (location = listGetByPtr((HLIST)(t->tl), (void *)timer )) < 0 )
    return RVERROR;

  etimerDestruct( (HETIMER)(node->tHndl) ) ;
  meiEnd(node->mutex) ;

  return (listDelete((HLIST)(t->tl), location));

}

HT
tsSet(
      HTSTIMER timer,
      TimerCallBack eventHandler,
      void* context,
      UINT32 timeOut  /* 1 msec units */
      )
{
  int loc;
  tsNode *node = (tsNode *)timer;

  if ( ! timer || ! timeOut ) return NULL;
  meiEnter(node->mutex);

  if( (loc = (int)etimerAdd( (HETIMER)(node->tHndl), eventHandler, context, timeOut) ) < 0 )
  {
        meiExit(node->mutex);
        return NULL;
  }
  tsUpdateMinTime((HTSTRUCT)node->tsHandle);

  meiExit(node->mutex);
  return ((HT)loc);

}




int
tsReset(
    HTSTIMER timer,
    HT tElement
    )
{
  tsNode *node = (tsNode *)timer;
  int ret;

  if ( !timer ) return RVERROR;
  meiEnter(node->mutex);

  ret = etimerDelete( (HETIMER)(node->tHndl), (HETIMERELEM)tElement );
  tsUpdateMinTime( (HTSTRUCT)node->tsHandle );

  meiExit(node->mutex);
  return ( ret );
}

int
tsResetByValue(
           HTSTIMER timer,
           TimerCallBack eventHandler,
           void* context
           )
{
  int tElement;
  tsNode *node = (tsNode *)timer;
  int ret=0;

  if ( !timer ) return RVERROR;
  meiEnter(node->mutex);

  tElement = (int)etimerFind((HETIMER)(node->tHndl), eventHandler, context);
  if ( tElement >=0 )
    ret=etimerDelete( (HETIMER)(node->tHndl), (HETIMERELEM)tElement );
  tsUpdateMinTime((HTSTRUCT)node->tsHandle);

  meiExit(node->mutex);
  return (ret);
}

int
tsUpdateMinTime(
        HTSTRUCT tH
        )
{
    tHandle *tm = (tHandle *)tH;
    int loc;
    int first=TRUE;
    tsNode *node;
    
    if (! tH ) return RVERROR;

    for( loc=listHead(tm->tl) ;loc>=0; loc=listNext(tm->tl,loc) )
    {
        if ( !(node = (tsNode *)listGetElem((HLIST)(tm->tl), loc)))
            return RVERROR;
        if ( etimerGetCount((HETIMER)(node->tHndl) ) > 0 )
        {
            if (first)
            {
                tm->min = etimerGetNextExpiration((HETIMER)(node->tHndl));
                first=FALSE;
                tm->tsWasSet=TRUE;
            }
            else
                if ((tm->min > etimerGetNextExpiration((HETIMER)(node->tHndl))) > 0)
                    tm->min = etimerGetNextExpiration((HETIMER)(node->tHndl));
        }
        
    }
    
    if (first==TRUE) tm->min=(UINT32)-1;
    return OK;
}

int
tsGetMinTime(
         HTSTRUCT tH
         )
{
  tHandle *tm = (tHandle *)tH;

  if (! tH ) return RVERROR;

#ifndef NOLOGSUPPORT
  {
    tsNode *node;
    int loc;

    for( loc=listHead(tm->tl) ;loc>=0; loc=listNext(tm->tl,loc) ){
      if ( !(node = (tsNode *)listGetElem((HLIST)(tm->tl), loc)))
    continue;
      etimerPrintMinElement((HETIMER)(node->tHndl) );
    }

  }
#endif
  return (tm->min);
}

int
tsCheck(
    HTSTRUCT tH
    )
{
    tHandle *tm = (tHandle *)tH;
    int loc;
    tsNode *node;
    int first=TRUE;
    
    if (! tH ) return RVERROR;
    
    tm->tsWasSet = FALSE;

    for( loc=listHead(tm->tl) ;loc>=0; loc=listNext(tm->tl,loc) )
    {
        if ( !(node = (tsNode *)listGetElem((HLIST)(tm->tl), loc)))
            return RVERROR;
        meiEnter(node->mutex);

        if(tm->tsWasSet && first) first = FALSE;

        if ( etimerGetCount((HETIMER)(node->tHndl) ) > 0 )
        {
            if (first)
            {
                tm->min = etimerGetNextExpiration((HETIMER)(node->tHndl));
                first=FALSE;
            }
            else
			{
                UINT32 nextExp=etimerGetNextExpiration((HETIMER)(node->tHndl));
                BOOL tmin_rollOver=FALSE;
                BOOL nextExp_rollOver = FALSE;
                UINT32 diff;

                if (tm->min > nextExp)
                  diff = tm->min > nextExp;
                else
                  diff = nextExp - tm->min;
				
                if ((tm->min < nextExp) && (diff > 0x80000000)) 
                {
                   tmin_rollOver = TRUE;
                }

                if ((nextExp < tm->min) && (diff > 0x80000000)) 
                {
                   nextExp_rollOver = TRUE;
                }

                if ((tm->min > nextExp) && (!nextExp_rollOver)) 
                {
                    tm->min = nextExp;
                } 
                if ((tm->min < nextExp) && (tmin_rollOver))    
                {
                    /*tm->min has passed 0xFFFFFFFF mark and actually is bigger than nextExp*/
                    tm->min = nextExp;
    	            printf("tsCheck: (tm->min < nextExp) && (tmin_rollOver) tm->min=%u \n", tm->min);
                } 
            }     
        }

        etimerCheck((HETIMER)(node->tHndl), node->mutex);

        meiExit(node->mutex);
    }

    if (first==TRUE) tm->min=(UINT32)-1;

    return OK;
}

int
tsGetCount(
       HTSTRUCT tH
       )
{
  tHandle *tm = (tHandle *)tH;
  int loc;
  tsNode *node;

  if (! tH ) return RVERROR;

  tm->count = 0;
  for( loc=listHead(tm->tl) ;loc>=0; loc=listNext(tm->tl,loc) ){
    if ( !(node = (tsNode *)listGetElem((HLIST)(tm->tl), loc)))
      return RVERROR;
    tm->count += etimerGetCount( node->tHndl );
  }

  return (tm->count);
}


int
tsGetParams(
        IN HTSTIMER timer,
        IN HT tElement,
        OUT TimerCallBack *eventHandler,
        OUT void** context
        )
{
  tsNode *node = (tsNode *)timer;

  if (  ! timer ) return RVERROR;

  return ( etimerGetParams( (HETIMER)(node->tHndl), (HETIMERELEM)tElement, eventHandler, context ));
}


HTSTRUCT tsGetStruct(IN HTSTIMER timer)
{
    return (HTSTRUCT)((tsNode *)timer)->tsHandle;
}


RvH323ThreadHandle tsGetThreadId(IN HTSTRUCT tH)
{
    return ((tHandle *)tH)->threadId;
}


#ifdef __cplusplus
}
#endif



