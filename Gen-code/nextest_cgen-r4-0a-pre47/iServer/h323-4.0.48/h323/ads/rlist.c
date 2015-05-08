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
  rlist.c

  list implementation by fixed size array.

  Ron Shpilman
  5 Sep 1995
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rvinternal.h>

#include <rlist.h>
#include <ra.h>
#include <ms.h>



typedef struct {
  int prev;
  int next;
  char *data;
} listElem;

/*__________________________________allocators______________________________*/

/*
 * listConstruct
 * description: Create an RLIST object
 * parameters : elemSize            - Size of elements in the RLIST in bytes
 *              maxNumOfElements    - Number of elements in RLIST
 *              logMgr              - Log manager to use
 *              name                - Name of RLIST (used in log messages)
 * return     : Handle to RLIST constructed on success
 *              NULL on failure
 */
HLIST listConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN RVHLOGMGR    logMsg,
    IN const char*  name)
{
  HRA raH;

  raH = raConstruct((int)(elemSize + sizeof(listElem) - sizeof(char*)),
                    maxNumOfElements,
                    logMsg,
                    name);

  if (raH == NULL) return NULL;

  raSetFirst(raH, RVERROR);
  raSetLast(raH, RVERROR);
  return (HLIST)raH;
}


/*
 *listDestruct
 *Description: free memory acquired by list
 *Parameters : hlist - list handle
 */
void
listDestruct(HLIST hlist)
{
  raDestruct((HRA)hlist);
}


/*
 *listClear
 *Description: Clean the list from all elements.
 *Parameters : hlist - list handle
 */
void
listClear(HLIST hlist)
{
  HRA raH = (HRA)hlist;

  raClear(raH);
  raSetFirst(raH, RVERROR);
  raSetLast(raH, RVERROR);
}


/*
 * listSetCompareFunc
 * description: Set the compare function to use
 * parameters : hlist   - Handle of the RLIST object
 *              func    - Compare function to use
 * return     : none
 */
void listSetCompareFunc(IN HLIST hlist, IN ECompare func)
{
    raSetCompareFunc((HRA)hlist, func);
}



/*________________________________________Information_________________________________*/

/*
 * listElemIsVacant
 * description : Give status of node location
 * parameters  : hlist    - handle of the RLIST object
 *               location - location of element in list
 * return      : TRUE, FALSE or RVERROR
 */
int
listElemIsVacant(HLIST hlist,
                 int location)
{ return raElemIsVacant((HRA)hlist, location); }


/*
 *listCurSize
 *Description: define number of elements in the list
 *Parameters : hlist - list handle
 *Return     : number of elements in the list
 */
int listCurSize(HLIST hlist)
{  return raCurSize((HRA)hlist); }


/*
 *listMaxSize
 *Description: define number of elements in the list
 *Parameters : hlist - list handle
 *Return     : maximum number of elements in the list
 */
int     listMaxSize    (HLIST hlist)
{   return raMaxSize((HRA)hlist); }



/*
 *listHead
 *Description: define 1st (head) element in the list
 *Parameters : hlist - list handle
 *Return     : 1st (head) element in the list
 */
int listHead(HLIST hlist)
{ return raFirst((HRA)hlist); }


/*
 *listTail
 *Description: define last (tail) element in the list
 *Parameters : hlist - list handle
 *Return     : last (tail) element in the list
 */
int listTail(HLIST hlist)
{ return raLast((HRA)hlist); }


/*
 *listNext
 *Description: define 'next'element in the list
 *Parameters : hlist - list handle
 *             location - location after which to get 'next' element
 *Return     : 'next' element in the list
 */
int listNext(HLIST hlist, int location)
{
  HRA raH = (HRA)hlist;
  listElem *elem;

  if (!(elem = (listElem *)raGet(raH, location)))
    return RVERROR;
  else
    return elem->next;
}

/*
 *listPrev
 *Description: define 'previous' element in the list
 *Parameters : hlist - list handle
 *             location - location before which to get 'previous' element
 *Return     : 'previous' element in the list
 */
int listPrev(HLIST hlist, int location)
{
  HRA raH = (HRA)hlist;
  listElem *elem;

  if (!(elem = (listElem *)raGet(raH, location)))
    return RVERROR;
  else
    return elem->prev;
}


/*
 *listMaxUsage
 *Description: get list max usage
 *Parameters : hlist - list handle
 *             location - location before which to get 'previous' element
 *Return     : list max usage
 */
int
listMaxUsage(HLIST hlist)
{  return raMaxUsage((HRA)hlist); }




/*______________________________search_________________________________*/

/*
 *listGetElem
 *Description: get an element from the list
 *Parameters : hlist - list handle
 *             location - location of the element to fetch
 *Return     : fetched element
 */

Element
listGetElem(HLIST hlist, int location)
{
  listElem *elem;

  if (!(elem = (listElem *)raGet((HRA)hlist, location)))
    return NULL;
  else
    return &(elem->data);
}


/*
 *listGetByPtr
 *Description: get the location of node at ptr
 *Parameters : hlist - list handle
 *             prt - pointer to element in the list
 *Return     : the location of the element
 */
int
listGetByPtr(HLIST hlist, void *ptr)
{
  return raGetByPtr((HRA)hlist, (char *)ptr - 2*sizeof(int));
}




/*
  Desc: Search from location for element matching param.
  Note: Does not change current.
  Returns: location of matching element.
  */
int
listCompare(HLIST hlist, int location, void *param, ECompare compare)
{
  Element *elem;
  int cur;

  for (cur=location; cur != RVERROR; cur=listNext(hlist, cur)) {
    if (!(elem = (Element*)listGetElem(hlist, cur))) return RVERROR; /* invalid element */
    if ( (compare && compare(elem, param)) || elem == param)
      return cur;
  }

  return RVERROR;
}


/*
 *listFind
 *Description: find a node in the list starting from the location
               does not change current
 *Parameters : hlist - list handle
 *             location - starting location
 *             param - parameter to compare function
 *Return     : the location of the element
 */
int
listFind(HLIST hlist, int location, void *param)
{
  return listCompare(hlist, location, param, (ECompare)raFCompare((HRA)hlist));
}


/*__________________________________update_____________________________________*/

/* Add element to a vacant place in list array
   Copy element data and return position in element array.
*/
static int
listAddNode(HLIST hlist, Element elem)
{
  HRA raH = (HRA)hlist;
  int loc;
  listElem *elemList;

  if ( (loc = raAddExt(raH, (void **)&elemList)) < 0) return RVERROR;

  if (elem)
      memcpy((void *)&(elemList->data), elem, raElemSize(raH)-sizeof(listElem)+sizeof(char*));
  else
      memset((void *)&(elemList->data), 0, raElemSize(raH)-sizeof(listElem)+sizeof(char*));

  elemList->prev = elemList->next = RVERROR;
  return loc;
}


/*
  Add to location:

  Before: || A || <--> || C ||

  After:  || A || <--> || B || <--> || C ||

  Scenario: Add new node B after location A.

  Algorithm:
  - B.prev = A
  - B.next = C
  - if (C) C.prev = B
    else Tail = B
  - if (A) A.next = B
    else Head = B
 */
/*
  Desc: Add element after location.
  Returns: location of the new node.
  Note: if location error or RVERROR than node is added to head of list.
  */
int
listAdd(HLIST hlist, int location, Element elem)
{
  HRA raH = (HRA)hlist;
  int Apos, Bpos, Cpos=RVERROR;
  listElem *A, *B, *C;

  if ( (Bpos = listAddNode(hlist, elem)) < 0)
    return RVERROR;
  (location<0)?(Apos=RVERROR):(Apos=location);

  B = (listElem *)raGet(raH, Bpos);
  A = (listElem *)raGet(raH, Apos);
  (A)?(Cpos=A->next):(Cpos=raFirst(raH), Apos=RVERROR);
  C = (listElem *)raGet(raH, Cpos);

  B->prev = Apos;
  B->next = Cpos;
  if (C) C->prev = Bpos;
  else raSetLast(raH, Bpos);
  if (A) A->next = Bpos;
  else raSetFirst(raH, Bpos);

  return Bpos;
}

/* Description: add elem as a head of the list*/
int
listAddHead(HLIST hlist, Element elem)
{
  return listAdd(hlist, RVERROR, elem);
}

/* Description: add elem as a tail of the list*/
int
listAddTail(HLIST hlist, Element elem)
{
  return listAdd(hlist, listTail(hlist), elem);
}


/*
  Delete location:

  Before:  || A || <--> || B || <--> || C ||
  After:   || A || <--> || C ||

  if (!C)  Tail=A;
  if (!A)  Head=C;

  Description: Delete node B defined by location between A and C.

 */
BOOL
listDelete(HLIST hlist, int location)
{
  HRA raH = (HRA)hlist;
  int Apos, Cpos;
  listElem *A, *B, *C;

  if (raElemIsVacant(raH, location) == TRUE)
    return FALSE;
  B = (listElem *)raGet(raH, location);
  if (!B) return FALSE;

  Apos = B->prev;
  Cpos = B->next;
  A = (listElem *)raGet(raH, Apos);
  C = (listElem *)raGet(raH, Cpos);

  if (C) C->prev = Apos;
  else raSetLast(raH, Apos);
  if (A) A->next = Cpos;
  else raSetFirst(raH, Cpos);

  raDeleteLocation(raH, location);
  return TRUE;
}

/*
  Delete location:

  Before:  || A || <--> || B || <--> || C ||
  After:                || B || <--> || C ||

  Description: Delete Head of the List.
 */
BOOL
listDeleteHead(HLIST hlist)
{
  return listDelete(hlist, listHead(hlist));
}

/*
  Delete location:

  Before:  || A || <--> || B || <--> || C ||
  After:   || A || <--> || B ||

  Description: Delete Tail of the List.
 */
BOOL
listDeleteTail(HLIST hlist)
{
  return listDelete(hlist, listTail(hlist));
}

/*
 *  Description: Delete all elements from the List.
 */
BOOL
listDeleteAll(HLIST hlist)
{
  while (listDeleteHead(hlist));
  return TRUE;
}

/*__________________________________display_____________________________________*/
void
listPrint(HLIST hlist, void *param)
{
/* todo: leave this?*/
  EFunc print=raGetPrintFunc((HRA)hlist);
  int cur;

  for (cur=listHead(hlist); cur>=0; cur=listNext(hlist, cur)) {
    msaPrintFormat( -1, "[%d]", cur);
    print(listGetElem(hlist, cur), param);
  }

}


#ifdef __cplusplus
}
#endif



