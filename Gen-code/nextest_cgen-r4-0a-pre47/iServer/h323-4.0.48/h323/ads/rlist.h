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
   rlist.h

   RADVISION list header file.
   Array implementation of a list.

   Ron Shpilman
   5 Sep 1995

   Version 1: Sasha: for windows.

   Version 2: Ron: 9 April 1996.
   - listDestruct() change name.

   */


#ifndef _RLIST_
#define _RLIST_

#include <rvinternal.h>
#include <log.h>

DECLARE_OPAQUE(HLIST); /* list handler */

/*********** element prototypes ************/
typedef void* Element;
typedef BOOL  (*ECompare)(Element element1, void *param);
typedef void* (*EFunc)(Element elem, void *param);


/*********** list functions **************/
/* All (int) functions Returns location or RVERROR if not successfull */

/************************************************************************
 * listConstruct
 * purpose: Create an RLIST object
 * input  : elemSize            - Size of elements in the RLIST in bytes
 *          maxNumOfElements    - Number of elements in RLIST
 *          logMgr              - Log manager to use
 *          name                - Name of RLIST (used in log messages)
 * output : none
 * return : Handle to RLIST constructed on success
 *          NULL on failure
 ************************************************************************/
HLIST listConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN RVHLOGMGR    logMsg,
    IN const char*  name);



/*
 *listDestruct
 *Description: free memory acquired by list
 *Parameters : hlist - list handle
 */
void    listDestruct   (HLIST hlist);


/*
 *listClear
 *Description: Clean the list from all elements.
 *Parameters : hlist - list handle
 */
void    listClear      (HLIST hlist);


/*
 * listSetCompareFunc
 * description: Set the compare function to use
 * parameters : hlist   - Handle of the RLIST object
 *              func    - Compare function to use
 * return     : none
 */
void listSetCompareFunc(IN HLIST hlist, IN ECompare func);


/*
 *listCurSize
 *Description: define number of elements in the list
 *Parameters : hlist - list handle
 *Return     : number of elements in the list
 */
int     listCurSize    (HLIST hlist);

/*
 *listMaxSize
 *Description: define number of elements in the list
 *Parameters : hlist - list handle
 *Return     : maximum number of elements in the list
 */
int     listMaxSize    (HLIST hlist);

/*
 *listHead
 *Description: define 1st (head) element in the list
 *Parameters : hlist - list handle
 *Return     : 1st (head) element in the list
 */
int     listHead       (HLIST hlist);

/*
 *listTail
 *Description: define last (tail) element in the list
 *Parameters : hlist - list handle
 *Return     : last (tail) element in the list
 */
int     listTail       (HLIST hlist);

/*
 *listNext
 *Description: define 'next'element in the list
 *Parameters : hlist - list handle
 *             location - location after which to get 'next' element
 *Return     : 'next' element in the list
 */
int     listNext       (HLIST hlist, int location);

/*
 *listPrev
 *Description: define 'previous' element in the list
 *Parameters : hlist - list handle
 *             location - location before which to get 'previous' element
 *Return     : 'previous' element in the list
 */
int     listPrev       (HLIST hlist, int location);

/*
 *listMaxUsage
 *Description: get list max usage
 *Parameters : hlist - list handle
 *             location - location before which to get 'previous' element
 *Return     : list max usage
 */
int
listMaxUsage(HLIST hlist);


/*
 *listFind
 *Description: find a node in the list starting from the location
               does not change current
 *Parameters : hlist - list handle
 *             location - starting location
 *             param - parameter to compare function
 *Return     : the location of the element
 */
int     listFind       (HLIST hlist, int location, void *param);

/*
  Desc: Search from location for element matching param.
  Note: Does not change current.
  Returns: location of matching element.
  */
int     listCompare    (HLIST hlist, int location, void *param, ECompare compare);

/*
 *listGetElem
 *Description: get an element from the list
 *Parameters : hlist - list handle
 *             location - location of the element to fetch
 *Return     : fetched element
 */
Element listGetElem    (HLIST hlist, int location);

/*
 *listGetByPtr
 *Description: get the location of node at ptr
 *Parameters : hlist - list handle
 *             prt - pointer to element in the list
 *Return     : the location of the element
 */
int     listGetByPtr   (HLIST hlist, void *ptr);

/*
  Desc: Add element after location.
  Returns: location of the new node.
  Note: if location error or RVERROR than node is added to head of list.
  */
int     listAdd        (HLIST hlist, int location, Element elem); /* after location */

/* Description: add elem as a head of the list*/
int     listAddHead    (HLIST hlist, Element elem);

/* Description: add elem as a tail of the list*/
int     listAddTail    (HLIST hlist, Element elem);

/* Description: Delete a node defined by location in the list.*/
BOOL    listDelete     (HLIST hlist, int location);

/* Description: Delete a 1st element (head) of the list.*/
BOOL    listDeleteHead (HLIST hlist);

/* Description: Delete a last element (tail) of the list.*/
BOOL    listDeleteTail (HLIST hlist);

/* Description: Delete all elements from the list.*/
BOOL    listDeleteAll  (HLIST hlist);

void    listPrint      (HLIST hlist, void *param);


/* Description: Give status of node location
 * Return     : TRUE, FALSE or RVERROR */
int
listElemIsVacant(
         HLIST hlist,
         int location  /* location of element in list */
         );

#endif
#ifdef __cplusplus
}
#endif



