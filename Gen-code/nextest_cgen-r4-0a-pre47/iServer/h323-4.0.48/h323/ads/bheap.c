#ifdef __cplusplus
extern "C" {
#endif



/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVISION LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVISION LTD..                  *
*                                                                               *
* RADVISION LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVISION 1998.                                                     *
*********************************************************************************
*/

/*********************************************************
*                P A C K A G E    B O D Y
*=========================================================
* TITLE:  bheap
*---------------------------------------------------------
* PROJECT:  GK
*---------------------------------------------------------
* $Workfile$
*---------------------------------------------------------
* $Author: sshetty $ Nathaniel Leibowitz
*---------------------------------------------------------
* $Date: 2004/03/24 17:51:56 $
*---------------------------------------------------------
* $Revision: 1.1 $
* $SKIP START$
*********************************************************/




/********************************************************/
/*                     INTERFACE DECLARATIONS           */
/********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rvinternal.h>

#include <copybits.h>
#include "bheap.h"

/********************************************************/
/*                     LOCAL DECLARATIONS               */
/********************************************************/

/*-----------------*/
/* LOCAL CONSTANTS */
/*-----------------*/

/*-----------------*/
/*  LOCAL MACROS   */
/*-----------------*/

#define p(i) ((INT32)((i-1)/2))
#define l(i) (((i+1)<<1) - 1)
#define r(i) ((i+1)<<1)
#define NODE(i,h) ((char *)(h->heap + i*sizeof(BHeapNode)))
/*-----------------*/
/*   LOCAL TYPES   */
/*-----------------*/

typedef struct
{
  char* heap;
  INT32 capacity;
  INT32 size;
  BOOL isAllocated;
  void *param;
  Fcompare compare;
  Fupdate  update;
} bheapStruct;

/*-----------------*/
/* LOCAL VARIABLES */
/*-----------------*/

/*------------------------------------------------------*/
/*            PROTOTYPES OF LOCAL FUNCTIONS             */
/*------------------------------------------------------*/

static void heapify(
            IN bheapStruct* h,
            IN UINT32 i
            );

static void swap(
         IN bheapStruct* h,
         IN UINT32 i,
         IN UINT32 j
         );

static void move(/*removes node from place src to place dest*/
         IN bheapStruct *h,
         IN UINT32 dest,
         IN UINT32 src
         );

static void set(/*sets the node in place i and calls the callback*/
        IN bheapStruct* h,
        IN BHeapNode *node,
        IN UINT32 i
        );

static bheapStruct *build(
              IN UINT32 heapCapacity,
              IN char *buff,
              IN Fcompare compare,
              IN Fupdate  update,
              IN void *param
              );
/*------------------------------------------------------*/
/*                  PUBLIC  FUNCTIONS                   */
/*------------------------------------------------------*/

int bheapGetAllocationSize(
                IN int maxCapacity
                )
{
  return (
      sizeof(bheapStruct) +
      maxCapacity * sizeof(BHeapNode)
      );
}


HBHEAP bheapConstruct(/*allocates and resets the memory and sets the callback functions.
               Upon success, handle to the bheap is returned.Else, NULL returned*/
              IN int heapCapacity,
              IN Fcompare compare,
              IN Fupdate  update,
              IN void *param
              )
{
  bheapStruct *h;
  char *ptr;

  if (( heapCapacity < 0 ) || (! compare ))
    return NULL;
  if( !(ptr=(char*)calloc(bheapGetAllocationSize(heapCapacity), 1)))
    return NULL;
  h = build( heapCapacity, ptr, compare, update, param);
  h->isAllocated= TRUE;
  return ((HBHEAP)h);
}


HBHEAP bheapConstructFrom(
             IN int heapCapacity,
             IN char *buff,
             IN int  buffSize,
             IN Fcompare compare,
             IN Fupdate  update,
             IN void *param
             )
{
  bheapStruct* h;

  if (heapCapacity<=0 || !compare || ! buff || buffSize < bheapGetAllocationSize(heapCapacity))
    return NULL;

  h=build(heapCapacity, buff, compare, update, param);
  h->isAllocated=FALSE;
  return (HBHEAP)h;
}



UINT32 bheapSize(/*returns number of nodes currently stored in the heap.Time : const*/
        IN HBHEAP handle
        )
{
  return ((bheapStruct*)handle)->size;
}

int bheapInsert(/*if heap is full, node is not added and FALSE returned.Time : log(n)*/
                IN HBHEAP handle,
                IN BHeapNode node
        )
{
  int i;
  bheapStruct* h = (bheapStruct *)handle;
  BHeapNode parent;

  if((! h->compare ) || (h->size == h->capacity))
    return RVERROR;
  i = h->size;
  set(h,&node,i);

  while (i > 0 )
    {
      memcpy(&parent, NODE(p(i),h), sizeof(BHeapNode));
      if (  h->compare(parent,node,h->param) == -1)
    break;
      swap(h,p(i),i);
      i = p(i);
    }
  h->size++;
  return OK;
}

BHeapNode bheapExtractTop(/*extracts and returns top of heap and maintains the heap invariant.
              if heap is already empty,NULL is returned.Time : log(n)*/
            IN HBHEAP handle
            )
{
  bheapStruct* h = (bheapStruct *)handle;
  BHeapNode top;

  if (h->size <= 0)
    return NULL;

  memcpy( &top, NODE(0,h) , sizeof(BHeapNode));
  move(h,0,h->size-1);
  h->size--;
  heapify(h,0);
  return top;
}


BHeapNode bheapTop(/*returns top of heap without extracting it.
           if heap is already empty,NULL is returned.Time : const*/
           IN HBHEAP handle
           )
{
  bheapStruct* h = (bheapStruct *)handle;
  BHeapNode top;

  if (h->size <= 0)
    return (BHeapNode)RVERROR;
  memcpy( &top, NODE(0,h) , sizeof(BHeapNode));

  return (top);
}

BHeapNode bheapDeleteNode(
              IN HBHEAP handle,
              IN BHeapNode* node)
{
  bheapStruct* h = (bheapStruct *)handle;
  int i = (int)(((char *)node - h->heap)/sizeof(BHeapNode));
  BHeapNode hNode;

  if (i < 0 || i >= h->size)
    return NULL;

  memcpy( &hNode, NODE(i,h) , sizeof(BHeapNode));
  while (i > 0)
    {
      move(h,i,p(i));
      i = p(i);
    }
  bheapExtractTop(handle);
  return (hNode);
}

int bheapUpdateNode(/*If an node's key was changed, Calling this will reshuffle the heap
             to restore the heap invariant. Should be called before any other action
             is done to heap or any other node's key is changed.FALSE on error,Time : log(n)*/
           IN HBHEAP handle,
           IN BHeapNode* node)
{
  bheapStruct* h = (bheapStruct *)handle;
  int  i = (int)(((char*)node - h->heap)/sizeof(BHeapNode));
  BHeapNode parent, hNode;

  if (i < 0 || i >= h->size)
    return RVERROR;

  memcpy(&hNode, NODE(i, h), sizeof(BHeapNode));

  while (i > 0 ) {
    memcpy(&parent, NODE(p(i),h), sizeof(BHeapNode));
    if (  h->compare(parent,hNode,h->param) == -1)
      break;
    swap(h,p(i),i);
    i = p(i);
    hNode = parent;
  }

  heapify(h, i);

  return OK;
}


int bheapUpdateTop(
          /* Special case of heapUpdateNode */
          /* Implemented due to improve performance */
          IN HBHEAP handle
          )
{
  bheapStruct* h = (bheapStruct *)handle;

  heapify( h, 0 );
  return OK;
}


void bheapDestruct(
           IN HBHEAP handle
           )
{
  bheapStruct* h = (bheapStruct *)handle;
  if ( h->isAllocated )
    free(h);
}


/*------------------------------------------------------*/
/*                  LOCAL  FUNCTIONS                    */
/*------------------------------------------------------*/

static  bheapStruct *build(
               IN UINT32 heapCapacity,
               IN char *buff,
               IN Fcompare compare,
               IN Fupdate  update,
               IN void *param
               )
{
  bheapStruct* h;

  memset(buff, 0, bheapGetAllocationSize( (int)heapCapacity ));

  h = (bheapStruct *)buff;

  h->heap = (char *)h + sizeof(bheapStruct);
  h->compare = compare;
  h->update = update;
  h->size = 0;
  h->capacity = heapCapacity;
  h->isAllocated = FALSE;
  h->param = param;
  return (h);
}


static void heapify(
            IN bheapStruct* h,
            IN UINT32 i)
{
  UINT32 smallest;
  BHeapNode node ,smallestNode, left, right;


  memcpy(&node, NODE(i,h) , sizeof(BHeapNode));
  smallestNode = node;

  while ((INT32)l(i) < h->size || (INT32)r(i) < h->size) {
    memcpy(&left, NODE(l(i), h),  sizeof(BHeapNode));
    memcpy(&right, NODE(r(i), h), sizeof(BHeapNode));

    if ( (INT32)l(i) < h->size  &&  h->compare( left, node,h->param) == -1 ) {
      smallestNode = left;
      smallest = l(i);
    }
    else{
      smallest = i;
      smallestNode = node;
    }

    if ((INT32)r(i) < h->size && h->compare(right,smallestNode,h->param) == -1){
      smallest = r(i);
      smallestNode = right;
    }

    if ( smallest == i )
      return ;

    swap(h,i,smallest);
    i = smallest;
  }
}

static void swap(
         IN bheapStruct* h,
         IN UINT32 i,
         IN UINT32 j)
{
  BHeapNode tmp ;

  memcpy(&tmp, NODE(i,h), sizeof(BHeapNode));
  set(h,(BHeapNode *)NODE(j,h),i);
  set(h,&tmp,j);
}

static void move(/*removes node from place src to place dest*/
         IN bheapStruct *h,
         IN UINT32 dest,
         IN UINT32 src)
{
  if (src == dest)
    return;
  set(h,(BHeapNode *)NODE(src,h),dest);
  memset(h->heap + src*sizeof(BHeapNode), 0, sizeof(BHeapNode));
}

static void set(/*sets the node in place i and calls the callback*/
        IN bheapStruct* h,
        IN BHeapNode *node,
        IN UINT32 i)
{

  memcpy((h->heap + i*sizeof(BHeapNode)), node, sizeof(BHeapNode));
  if (! h->update) return ;

  h->update( (BHeapNode *)(h->heap + i*sizeof(BHeapNode) ), h->param );

}

#ifdef __cplusplus
}
#endif



