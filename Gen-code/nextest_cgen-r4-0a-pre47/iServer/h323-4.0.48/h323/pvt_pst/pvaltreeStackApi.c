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
  pvaltreeStackApi

  This file contains functions which are available for the use of the stack modules,
  but are not part of the API provided to the users

  */

#include <pvaltreeDef.h>
#include <pvaltreeStackApi.h>



/************************************************************************
 *
 *                          Private declarations
 *
 ************************************************************************/


/************************************************************************
 * pvtCompareContext
 * Context to use for pvtCompareFunction.
 * This contexts includes the following information:
 * hVal     - Value tree to use
 * nodeId   - Value node id to compare
 *            This node will be compare with a syntax value node
 ************************************************************************/
typedef struct
{
    HPVT    hVal;
    int     nodeId;
} pvtCompareContext;




/************************************************************************
 *
 *                          Private functions
 *
 ************************************************************************/


static
BOOL
pvtCompareTwo(
         /* Compare two vt nodes */
         RTElement element1,
         RTElement element2,
         void *param /* syntax handle */
         )
{
    vtNode *node1 = (vtNode*)element1;
    vtNode *node2 = (vtNode*)element2;
    vtStruct* vt  = (vtStruct *)param; /*((vtStruct**)param)[0];*/
    HPST hSyn    = node1->hSyn; /*((HPST*)param)[1];*/
    int  synField;

    /* -- structure */
    if (/* Fix for 230 */VTN_SYN_FIELD(node1) >= 0 &&
      VTN_SYN_FIELD(node1) != VTN_SYN_FIELD(node2)) return FALSE;
    if (VTN_SYN_NODE(node1) != VTN_SYN_NODE(node2)) return FALSE;
    synField = VTN_SYN_NODE(node1);
    if (pstIsNodeComplex(hSyn, synField)) return TRUE;

    /* -- value */
    if (node1->value != node2->value) return FALSE;
    if ( (node1->string && !node2->string) ||
       (!node1->string && node2->string)) return FALSE;
    if (node1->string &&
        rpoolCompareInternal(vt->sPool,node1->string, node2->string, node1->value)) return FALSE;

    return TRUE;
}


/************************************************************************
 * pvtCompare
 * purpose: Comparision function used when searching for a specific object
 *          in an association table.
 * input  : hSyn        - Syntax tree used
 *          sNodeId     - Syntax value node to compare
 *          context     - Context to use for comparison
 * output : none
 * return : Negative if the key is lower than elem in dictionary comparison
 *          0 if the key and the element are equal
 *          Positive if the key is higher than elem in dictionary comparison
 ************************************************************************/
static int pvtCompare(IN HPST hSyn, IN int sNodeId, IN void* context)
{
    pvtCompareContext* key = (pvtCompareContext *)context;

    int i,cmp,vSynNodeId=-1,vtNodeId,stNodeId=-1;
	INT32 vValue=0;
    INTPTR vFieldId=-1;
    pstValueTreeStruct sValTreeStruct;
    pstValueNodeStruct sValNodeStruct;
    char vString[2048];

    vtNodeId = key->nodeId;

    pvtGet(key->hVal,key->nodeId,NULL,&vSynNodeId,NULL,NULL);

    pstGetValueTreeStruct(hSyn, sNodeId, &sValTreeStruct);

    if(sValTreeStruct.isString != pstValueSubTree)
    {
        if(pvtGetByIndex(key->hVal,key->nodeId,1,NULL)>=0)
        return TRUE;

        pvtGet(key->hVal,key->nodeId,NULL,NULL,&vValue,NULL);
        pvtGetString(key->hVal,key->nodeId,vValue,vString);

        if(sValTreeStruct.isString == pstValueString)
        {
            return(strcmp(vString,pstGetFieldNamePtr(hSyn,sValTreeStruct.value)));
        }
        else
        {
            if(vValue>sValTreeStruct.value) return TRUE;
            else if(vValue<sValTreeStruct.value) return RVERROR;
            else return FALSE;
        }
    }

    for(i=2,stNodeId=pstGetChild(hSyn,sNodeId,1,NULL),vtNodeId=pvtChild(key->hVal,vtNodeId);
        vtNodeId>=0 && stNodeId>=0;
        vtNodeId=pvtBrother(key->hVal,vtNodeId),stNodeId=pstGetChild(hSyn,sNodeId,i++,NULL))
    {
        pvtGet(key->hVal,vtNodeId,&vFieldId,NULL,NULL,NULL);
        pstGetValueNodeStruct(hSyn, stNodeId, &sValNodeStruct);
        if(vFieldId>sValNodeStruct.fieldName)
            return TRUE;
        else
            if(vFieldId<sValNodeStruct.fieldName)
                return RVERROR;
        else
        {
            pvtCompareContext newContext;
            newContext.hVal = key->hVal;
            newContext.nodeId = vtNodeId;
            if ((cmp=pvtCompare(hSyn, stNodeId, &newContext)))
                return cmp;
        }
    }

    return FALSE;
}




/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


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
    IN  int     val2RootId)
{
    vtStruct *vt1 = (vtStruct *)val1H;
    vtStruct *vt2 = (vtStruct *)val2H;
    HMEI    lock1, lock2;
    int     result;

    if (!val1H || !val2H) return RVERROR;

    /* Make sure we lock an an orderly fashion to diminish dead-locking possibilities */
    if (val1H > val2H)
    {
        lock1 = vt2->lock;
        lock2 = vt1->lock;
    }
    else
    {
        lock1 = vt1->lock;
        lock2 = vt2->lock;
    }

    meiEnter(lock1);
    meiEnter(lock2);
    result = rtCompareTrees(vt1->vTree, val1RootId, vt2->vTree, val2RootId, pvtCompareTwo,
        (void *)val1H);
    meiExit(lock2);
    meiExit(lock1);

    return result;
}


RVAPI int RVCALLCONV
pvtFindObject(
         IN  HPVT   hVal,
         IN  int    nodeId,
         IN  HPST   hSyn,
         IN  int    stNodeIdOfAT,
         OUT int*   objectId)
{
    vtStruct* vt = (vtStruct *)hVal;
    pvtCompareContext   key;

    if (vt == NULL) return RVERROR;
    meiEnter(vt->lock);

    key.hVal = hVal;
    key.nodeId = nodeId;

    *objectId = pstFindObjectInAT(hSyn, stNodeIdOfAT, pvtCompare, &key);

    meiExit(vt->lock);
    return (*objectId);
}


RVAPI int RVCALLCONV
pvtAddChildsIfDiffer(
                        IN  HPVT destH,
                        IN  int destParentId,
                        IN  HPVT srcH,
                        IN  int srcParentId,
                        IN  BOOL move
                        )
{
    int destCur, srcCur, leftBrother=RVERROR;
    BOOL notFound;

    if (!destH || !srcH) return RVERROR;

    destCur = pvtChild(destH, destParentId);
    if (destCur<0)
        return pvtAddChilds(destH, destParentId, srcH, srcParentId);

    for(; destCur >=0; destCur = pvtBrother(destH, destCur))
    leftBrother = destCur;

    for(srcCur = pvtChild(srcH, srcParentId); srcCur >= 0; srcCur = pvtBrother(srcH, srcCur))
    {
        notFound = TRUE;
        for(destCur = pvtChild(destH, destParentId);
            destCur >= 0;
            destCur = pvtBrother(destH, destCur))
            if (pvtCompareTree(destH, destCur, srcH, srcCur) == TRUE)
            {
                notFound = FALSE;
                break;
            }

        if (notFound) /* srcCur child not found in dest children */
        {
            if (move)
            {
                pvtAdoptChild(destH, srcCur, destParentId, leftBrother);
                leftBrother = pvtBrother(destH, leftBrother);
            }
            else
                pvtAddTree(destH, destParentId, srcH, srcCur);
        }
    }
    return TRUE;

}




#ifdef __cplusplus
}
#endif



