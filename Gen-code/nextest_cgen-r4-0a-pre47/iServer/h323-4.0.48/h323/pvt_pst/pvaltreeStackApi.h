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
  pvaltreeStackApi

  This file contains functions which are available for the use of the stack modules,
  but are not part of the API provided to the users

  */

#ifndef _PVALTREE_STACKAPI_H
#define _PVALTREE_STACKAPI_H

#ifdef __cplusplus
extern "C" {
#endif


#include <pvaltreeDef.h>


/************************************************************************
 * pvtCompareTree
 * purpose: Compare between two trees
 *          The trees must be structure identical, the compare function only
 *          checks that the values are identical.
 * input  : val1H       - PVT handle of the 1st tree
 *          val1RootId  - Root ID of the 1st tree
 *          val2H       - PVT handle of the 2nd tree
 *          val2RootId  - Root ID of the 2nd tree
 * output : none
 * return : Non-negative value if trees are identical
 *          Negative value on failure
 ************************************************************************/
int pvtCompareTree(
    IN  HPVT    val1H,
    IN  int     val1RootId,
    IN  HPVT    val2H,
    IN  int     val2RootId);



RVAPI int RVCALLCONV
pvtAddChildsIfDiffer(
                        IN  HPVT destH,
                        IN  int destParentId,
                        IN  HPVT srcH,
                        IN  int srcParentId,
                        IN  BOOL move
                        );

RVAPI int RVCALLCONV
pvtFindObject(
         IN HPVT valH,
         IN int nodeId,
         IN HPST synH,
         IN int stNodeIdOfAT,
         OUT int *objectId
         );



#ifdef __cplusplus
}
#endif

#endif  /* _PVALTREE_STACKAPI_H */


