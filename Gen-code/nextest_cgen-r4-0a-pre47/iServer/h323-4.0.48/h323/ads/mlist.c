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

#include <ms.h>
#include <ra.h>
#include <mlist.h>

typedef struct
{
  int   tail;
  int   count;
  int   align; /* for DEC */
} mListDesc;


/*
 * Function   : mlistConstruct
 * Description: Create an MLIST object
 * Parameters : elemSize            - Size of elements in the MLIST in bytes
 *              maxNumOfElements    - Number of elements in MLIST
 *              logMgr              - Log manager to use
 *              name                - Name of MLIST (used in log messages)
 * return     : handle to MLIST constructed on success
 *              NULL on failure
 */
HLIST mlistConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN RVHLOGMGR    logMsg,
    IN const char*  name)
{
    return listConstruct(max(elemSize, (int)sizeof(mListDesc)),
                         maxNumOfElements,
                         logMsg,
                         name);
}

/*
 *Function   : mlistDestruct
 *Description: free memory acquired by mlist
 *Parameters : hlist - mlist handle
 *Return     :
 */
void    mlistDestruct   (HLIST hlist)
{
    listDestruct(hlist);
}


/************************************************************************
 * Function   : mlistSetCompareFunc
 * Description: Set the compare function to use
 * Parameters : hlist   - Handle of the MLIST object
 *          func    - Compare function to use
 * return : none
 ************************************************************************/
void mlistSetCompareFunc(IN HLIST hlist, IN ECompare func)
{
    listSetCompareFunc(hlist, func);
}


/*
 *Function   : mlistAddList
 *Description: add empty list to mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : location of the new list in mlist
 */
int mlistAddList(HLIST hlist)
{
    mListDesc newList;
        newList.tail = RVERROR;
        newList.count = 0;
        /*h.e unused!!!
        newList.changed = 0;
        */
        newList.align = 0;
    return listAddHead(hlist, &newList);
}

/*
 *Function   : mlistDeleteList
 *Description: delete list from mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : TRUE id sucess, otherwise FALSE
 */
BOOL mlistDeleteList(HLIST hlist, int list)
{
    /*first - delete all elements of the list,
      than  - delete reference to the list in mlist
     */
    mlistDeleteAll(hlist, list);
    return listDelete(hlist, list);
}

/*
 *Function   : mlistCurSize
 *Description: define number of elements in the list
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : number of elements in the list
 */
int     mlistCurSize    (HLIST hlist, int list)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return 0;
    return mlist->count;
}

/*h.e unused!!!
int     mlistListChanged(HLIST hlist, int list)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return 0;
    return mlist->changed;
}
*/

/*
 *Function   : mlistHead
 *Description: define 1st (head) element in the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : 1st (head) element in the list
 */
int     mlistHead       (HLIST hlist, int list)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    if (!mlist->count) return RVERROR;
    return listNext(hlist, list);
}

/*h.e unused!!!
int     mlistTail       (HLIST hlist, int list)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    return mlist->tail;
}
*/

/*
 *Function   : mlistNext
 *Description: define 'next'element in the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *             location - location after which to get 'next' element
 *Return     : 'next' element in the list
 */
int     mlistNext       (HLIST hlist, int list, int location)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    if (location==mlist->tail) return RVERROR;
    return listNext(hlist, location);
}

/*h.e unused!!!
int     mlistPrev       (HLIST hlist, int list, int location)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    if (location==listNext(hlist, list)) return RVERROR;
    return listPrev(hlist, location);
}
*/

/*
 *Function   : mlistAdd
 *Description: add an element to the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to add element
 *             location - location where to add element
 *             elem - element to add
 *Return     : position of the new element in the list
 */
int     mlistAdd        (HLIST hlist, int list, int location, Element elem)
{
    int pos;
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    if (location<0) location=list;
    pos=listAdd(hlist, location, elem);
    if (pos>=0)
    {
        if (!mlist->count || mlist->tail==location)
            mlist->tail=pos;
        /*h.e unused!!!
          mlist->changed++;
         */
        mlist->count++;
    }
    return pos;
}

/*
 *Function   : mlistAddHead
 *Description: add 1st element (head) to the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to add element
 *             elem - element to add
 *Return     : position of the new element in the list
 */
int     mlistAddHead    (HLIST hlist, int list, Element elem)
{
    return mlistAdd(hlist, list, RVERROR, elem);
}

/*h.e Unused!!!
int     mlistAddTail    (HLIST hlist, int list, Element elem)
{   mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return RVERROR;
    return mlistAdd(hlist, list, ((mlist->tail<0)?RVERROR:mlist->tail), elem);
}
*/

/*
 *Function   : mlistDelete
 *Description: delete an element located at 'location' from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete element from
 *             location - element to delete
 *Return     : TRUE if success, otherwise FALSE
 */
BOOL    mlistDelete     (HLIST hlist, int list, int location)
{
    mListDesc *mlist=(mListDesc*)listGetElem(hlist, list);
    if (!mlist) return FALSE;
    if (!mlist->count) return FALSE;
    if (mlist->tail==location) mlist->tail=listPrev(hlist,location);
    if (mlist->tail==list) mlist->tail=RVERROR;
    if (!listDelete(hlist, location)) return FALSE;
    mlist->count--;
    return TRUE;
}

/*
 *Function   : mlistDeleteHead
 *Description: delete 1st element from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete a 'head' - 1st element
 *Return     : TRUE if success, otherwise FALSE
 */
BOOL    mlistDeleteHead (HLIST hlist, int list)
{
    int pos=listNext(hlist, list);
    return mlistDelete(hlist, list, pos);
}

/*h.e unused!!!
BOOL    mlistDeleteTail (HLIST hlist, int list)
{
    return mlistDelete(hlist, list, ((mListDesc *)listGetElem(hlist, list))->tail);
}
*/

/*
 *Function   : mlistDeleteAll
 *Description: delete all elements from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete all elements
 *Return     : TRUE always
 */
BOOL    mlistDeleteAll  (HLIST hlist, int list)
{
    while(mlistDeleteHead(hlist, list));
    return TRUE;
}

/*h.e unused!!!
int     mlistCompare    (HLIST hlist, int list, int location, void *param, ECompare compare)
{
    Element *elem;
    int cur;
    list=list;
    for (cur=location; cur >= 0; cur=mlistNext(hlist, list, cur)) {
        if (!(elem = (Element*)listGetElem(hlist, cur))) return RVERROR;
        if ( (compare && compare(elem, param)) || elem == param)
        return cur;
    }
    return RVERROR;
}
*/

/*h.e unused!!!
int mlistFind(HLIST hlist, int list, int location, void *param)
{
  return mlistCompare(hlist, list, location, param, (ECompare)raFCompare((HRA)hlist));
}
*/

/*
 *Function   : mlistGetElem
 *Description: get an element from mlist
 *Parameters : hlist - mlist handle
 *             location - location of the element to fetch
 *Return     : fetched element
 */
Element mlistGetElem    (HLIST hlist, int list, int location)
{
    if (list);

    return listGetElem(hlist, location);
}

/*
 *Function   : mlistGetByPtr
 *Description: get an element from mlist by Pointer
 *Parameters : hlist - mlist handle
 *             prt - pointer to element in the mlist
 *Return     : fetched element
 */
int     mlistGetByPtr   (HLIST hlist, int list, void *ptr)
{
    if (list);

    return listGetByPtr(hlist, ptr);
}



/* mlist max usage */
/*h.e unused!!!
int
mlistMaxUsage(HLIST hlist)
{  return listMaxUsage(hlist); }
*/

#ifdef __cplusplus
}
#endif



