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

/**********************************************************
            PUBLIC  DECLARATIONS
*=========================================================
* TITLE: heap
*---------------------------------------------------------
* PROJECT:
*---------------------------------------------------------
* $Workfile$
*---------------------------------------------------------
* $Author: sshetty $ Nathaniel Leibowitz
*---------------------------------------------------------
* $Date: 2004/03/24 17:51:56 $
*---------------------------------------------------------
* $Revision: 1.1 $
**********************************************************/


#ifndef _BHEAP_
#define _BHEAP_

#include "rvinternal.h"

/*------------------*/
/* PUBLIC CONSTANTS */
/*------------------*/

/*-------------*/
/* PUBLIC TYPE */
/*-------------*/
DECLARE_OPAQUE(HBHEAP);
typedef void *BHeapNode;

typedef int (*Fcompare)(BHeapNode hNodeA, BHeapNode hNodeB, void *param);
typedef int (*Fupdate)(BHeapNode* newAddress, void *param);
typedef int (*Fprint)(BHeapNode hNode, void *param );


/*------------------*/
/* PUBLIC VARIABLES */
/*------------------*/

/*------------------*/
/* PUBLIC FUNCTIONS */
/*------------------*/

int bheapGetAllocatioonSize(
                 IN int heapCapacity
                 );

HBHEAP bheapConstruct(
              IN int heapCapacity,
              IN Fcompare compare,
              IN Fupdate  update,
              IN void *param
              );

HBHEAP bheapConstructFrom(
              IN int heapCapacity,
              IN char *buff,
              IN int  buffSize,
              IN Fcompare compare,
              IN Fupdate  update,
              IN void *param
              );

UINT32 bheapSize(
         IN HBHEAP handle
         );

int bheapInsert(
                IN HBHEAP handle,
                IN BHeapNode node
        );

BHeapNode bheapExtractTop(
              IN HBHEAP handle
              );

BHeapNode bheapTop(
           IN HBHEAP handle
           );

BHeapNode bheapDeleteNode(
              IN HBHEAP handle,
              IN BHeapNode* node
              );


int bheapUpdateNode(
            IN HBHEAP handle,
            IN BHeapNode* node
            );

int bheapUpdateTop(
          IN HBHEAP handle
          );

void bheapDestruct(
           IN HBHEAP handle
           );

#endif
#ifdef __cplusplus
}
#endif



