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

#ifndef MLIST_H
#define MLIST_H

#include <rlist.h>

/*
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
    IN const char*  name);

/*
 *Description: free memory acquired by mlist
 *Parameters : hlist - mlist handle
 */
void    mlistDestruct   (HLIST hlist);


/*
 * Description: Set the compare function to use
 * Parameters : hlist   - Handle of the MLIST object
 *              func    - Compare function to use
 */
void mlistSetCompareFunc(IN HLIST hlist, IN ECompare func);


/*
 *Description: add empty list to mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : location of the new list in mlist
 */
int mlistAddList    (HLIST hlist);

/*
 *Description: delete list from mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : TRUE id sucess, otherwise FALSE
 */
BOOL    mlistDeleteList (HLIST hlist, int list);

/*
 *Description: define number of elements in the list
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : number of elements in the list
 */
int     mlistCurSize    (HLIST hlist, int list);

/*h.e unused!!!
int     mlistListChanged(HLIST hlist, int list);
*/

/*
 *Description: define 1st (head) element in the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *Return     : 1st (head) element in the list
 */
int     mlistHead       (HLIST hlist, int list);

/*h.e unused!!!
int     mlistTail       (HLIST hlist, int list);
*/

/*
 *Description: define 'next'element in the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to get 'next' element
 *             location - location after which to get 'next' element
 *Return     : 'next' element in the list
 */
int     mlistNext       (HLIST hlist, int list, int location);

/*h.e unused!!!
int     mlistPrev       (HLIST hlist, int list, int location);
*/

/*
 *Description: add an element to the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to add element
 *             location - location where to add element
 *             elem - element to add
 *Return     : position of the new element in the list
 */
int     mlistAdd        (HLIST hlist, int list, int location, Element elem);

/*
 *Description: add 1st element (head) to the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to add element
 *             elem - element to add
 *Return     : position of the new element in the list
 */
int     mlistAddHead    (HLIST hlist, int list, Element elem);

/*h.e Unused!!!
int     mlistAddTail    (HLIST hlist, int list, Element elem);
*/

/*
 *Description: delete an element located at 'location' from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete element from
 *             location - element to delete
 *Return     : TRUE if success, otherwise FALSE
 */
BOOL    mlistDelete     (HLIST hlist, int list, int location);

/*
 *Description: delete 1st element from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete a 'head' - 1st element
 *Return     : TRUE if success, otherwise FALSE
 */
BOOL    mlistDeleteHead (HLIST hlist, int list);

/*h.e unused!!!
BOOL    mlistDeleteTail (HLIST hlist, int list);
*/

/*
 *Description: delete all elements from the list in mlist
 *Parameters : hlist - mlist handle
 *             list  - list number to delete all elements
 *Return     : TRUE always
 */
BOOL    mlistDeleteAll  (HLIST hlist, int list);

/*h.e unused!!!
int     mlistCompare    (HLIST hlist, int list, int location, void *param, ECompare compare);
*/

/*h.e unused!!!
int mlistFind   (HLIST hlist, int list, int location, void *param);
*/

/*
 *Description: get an element from mlist
 *Parameters : hlist - mlist handle
 *             location - location of the element to fetch
 *Return     : fetched element
 */
Element mlistGetElem    (HLIST hlist, int list, int location);

/*
 *Description: get an element from mlist by Pointer
 *Parameters : hlist - mlist handle
 *             prt - pointer to element in the mlist
 *Return     : fetched element
 */
int     mlistGetByPtr   (HLIST hlist, int list, void *ptr);

/*h.e unused!!!
 * mlist max usage
int
mlistMaxUsage(HLIST hlist);
*/


#endif
#ifdef __cplusplus
}
#endif



