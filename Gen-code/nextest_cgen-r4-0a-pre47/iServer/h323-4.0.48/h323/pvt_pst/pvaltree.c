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
  pvaltree.c

  Ron S.
  31 Oct. 1996

  */

#include <stdlib.h>
#include <rvinternal.h>
#include <mei.h>
#include <psyntreeDb.h>
#include <psyntreeStackApi.h>
#include <pvaltreeDef.h>
#include <pvaltreeStackApi.h>
#include <pvaltree.h>


/*
  valTree.c

  Holds message values to be used by coding systems and message compilers.

  Ron S.
  5 May 1996

  Parameters:
   - parentId: Id of parent node. Unique node identifier.
   - fieldEnum: enumeration of field as in the message.
   - value: integer value of node.
   - index: of child under parent. >=1

  Version 1: Ron S. 18 July 1996. Add syntax tree association.
  Now any value tree construction should be associated with a syntax tree. Each value node
  holds reference to the correlated syntax tree node. So you may get syntax node directly
  from the vtGet() function.


   */

#include <stdio.h>
#include <string.h>

#include <rlist.h>
#include <rtree.h>
#include <rpool.h>
#include <intutils.h>
#include <ms.h>
#include <oidutils.h>
#include <strutils.h>
#include <bb.h>



/* Definition of functions we'll need later */
static int pvtSetSyn(IN HPVT hVal, IN HPST hSyn, IN int nodeId, IN INTPTR fieldEnum, IN INT32 value, IN const char *string);



typedef struct
{
    vtStruct*   dest;
    vtStruct*   src;
} vtStructs;




/************************************************************************
 *
 *                          Private functions
 *
 ************************************************************************/


/************************************************************************
 * vtPrintToLog
 * purpose: Callback function called when a node has been parsed into
 *          a string and should be sent to the log.
 * input  : type    - Type of log handle to use
 *          line    - Line information
 *                    The additional parameters are discarded
 * output : none
 * return : none
 ************************************************************************/
#ifndef NOLOGSUPPORT
void vtPrintToLog(int type, const char* line, ...)
{
    logPrintFormat((RVHLOG)type, RV_DEBUG, "%s", line);
}
#endif


/************************************************************************
 * vtPrintFunc
 * purpose: Print a tree from a given node.
 *          This function is the function used by all of the public
 *          print functions.
 * input  : hVal        - Value Tree handle
 *          parentId    - node ID to start printing from
 *          pFunc       - The function called for the printing itself
 *          pFuncParam  - The context to pass to the print function
 *                        on each printed line
 *          degree      - Detail degree to use when printing the message
 *                        tree
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
#ifndef NOLOGSUPPORT
static int vtPrintFunc(
    IN  HPVT            hVal,
    IN  int             parentId,
    IN  pvtPrintFuncP   pFunc,
    IN  int             pFuncParam,
    IN  pvtPrintDegree  degree)
{
    vtStruct *vt = (vtStruct *)hVal;
    int result;

    meiEnter(vt->lock);

    /* Set printing parameters before we start printing */
    vt->pFunc       = pFunc;
    vt->pFuncParam  = pFuncParam;
    vt->degree      = degree;

    /* Print the message */
    result = rtPrint(vt->vTree, parentId, 0, -1, (void *)hVal, pFuncParam);

    meiExit(vt->lock);
    return result;
}
#endif


/************************************************************************
 * vtNodePrint
 * purpose: Callback function used to print a single node using vt's
 *          print parameters
 * input  : rtH     - RTree handle used
 *          nodeId  - Node to print
 *          layer   - Level of node in tree
 *          param   - vtStruct of the node
 * output : none
 * return : vtStruct of the node (i.e. - param)
 ************************************************************************/
#ifndef NOLOGSUPPORT
void *vtNodePrint(HRTREE rtH, int nodeId, int layer, void *param)
{
    const char* hexCh = (char*)"0123456789abcdef";
    const char* levels = (char*)". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ";
    vtNode*     n1 = (vtNode *)rtGetByPath(rtH, nodeId);
    vtStruct*   vt = (vtStruct *)param;
    char*       name = NULL;
    char        string[1024];
    char*       ptr;
    HPST        hSyn;
    char        buff[128];
    int         synNodeId = -1;
    int         tmpLen, value;
    pstNodeType type;

    /* Make sure we've got something to print */
    if ((n1 == NULL) || (param == NULL)) return param;
    ptr = string;
    hSyn = pvtGetSynTree((HPVT)vt, nodeId);

    /* 0. Layer */
    ptr += sprintf(ptr, "%2d> ", layer);
    if (layer > 32)
    {
        strcpy(ptr, levels);
        ptr += 64;
    }
    else
    {
        memcpy(ptr, levels, layer * 2);
        ptr += (layer * 2);
    }

    /* 1. NODE ID */
    if ((vt->degree & pvtPrintDegreeNodeId) != 0)
    {
        ptr += sprintf(ptr, "<%d> ", nodeId);
    }


    /* 2. FIELD NAME */
    if (hSyn != NULL)
    {
        int         sNodeId = -1;
        pstNodeType type;

        /* Get the parent's node type */
        if (pvtGet((HPVT)vt, pvtParent((HPVT)vt, nodeId), NULL, &sNodeId, NULL, NULL) >= 0)
            type = pstGetNodeType(hSyn, sNodeId);
        else
            type = pstNull;
        if (type == pstSequenceOf || type == pstSetOf)
        {
            /* SEQUENCE OF - no actual field name */
            name = (char*)"*";
        }
        else
        {
            if (layer == 0)
            {
                /* First node - get its type */
                int synNodeId;
                pvtGet((HPVT)vt, nodeId, NULL, &synNodeId, NULL, NULL);
                name = stGetNameByNodeId(hSyn, synNodeId);
            }
            else
            {
                /* Not first node - get the field's name */
                int synField = VTN_SYN_FIELD(n1);
                name = pstGetFieldNamePtr(hSyn, synField);
            }
        }
    }
    if (name != NULL)
    {
        /* Add the name of field */
        strcpy(ptr, name);
        ptr += strlen(ptr);
        *ptr = ' '; ptr++;
    }
    else
    {
        /* Didn't find the name - print the value of the node */
        ptr += sprintf(ptr, "$fid=%ld$ ", (long int)VTN_SYN_FIELD(n1));
        name = buff;
    }


    /* 3. VALUE */

    /* Check the type of the node we're about to print */
    pvtGet((HPVT)vt, nodeId, NULL, &synNodeId, NULL, NULL);
    type = pstGetNodeType(hSyn, synNodeId);

    if (type != pstBitString)
    {
        value = n1->value;
        tmpLen = min(n1->value, (int)sizeof(buff));
    }
    else
    {
        BYTE bitsNotInUse;

        /* Calculate the length of the bitstring in bits */
        rpoolCopyToExternal(vt->sPool, &bitsNotInUse, n1->string, n1->value - 1, 1);
        value = (n1->value - 1) * 8 - bitsNotInUse;
        tmpLen = min(n1->value - 1, (int)sizeof(buff));
    }

    if ((vt->degree & pvtPrintDegreeValue) != 0)
    {
        /* We always print the value */
        ptr += sprintf(ptr, "= (%u) ", value);
    }
    else
    {
        /* Remove values from non leaf nodes */
        if ((pvtChild((HPVT)vt, nodeId) < 0) || (n1->string != NULL))
        {
            /* leaf */
            ptr += sprintf(ptr, "= %d ", value);
        }
    }

    /* Get the string's value if we have a string */
    if (n1->string != NULL)
    {
        /* -- show strings */
        if (type == pstObjectIdentifier)
        {
            int resLen;

            /* We have to decode the object identifier and only then add it in */
            rpoolCopyToExternal(vt->sPool, buff, n1->string, 0, tmpLen);

            ptr[0] = '{';
            ptr[1] = ' ';
            resLen = oidDecodeOID((int)tmpLen, buff, sizeof(buff), ptr + 2, nameForm);
            if (resLen < 0) resLen = 0;
            ptr[resLen + 2] = '}';
            ptr += (resLen + 3);
        }
        else
        {
            /* Other string types - just print them */
            int     i;
            char*   hexPtr;
            char    ip[4];

            /* Put the string in place */
            rpoolCopyToExternal(vt->sPool, ptr + 1, n1->string, 0, tmpLen);
            if (tmpLen == 4)
            {
                /* We might need the original string later on for IP address */
                memcpy(ip, ptr + 1, tmpLen);
            }

            /* Fix unprintable characters and print the hex string */
            *ptr = '\''; ptr++;
            strcpy(ptr + tmpLen, "' =0x");
            hexPtr = ptr + tmpLen + 5;
            for (i = 0; i < tmpLen; i++)
            {
                *hexPtr = hexCh[((BYTE)*ptr) >> 4]; hexPtr++;
                *hexPtr = hexCh[((BYTE)*ptr) & 0xf]; hexPtr++;
                if ((BYTE)*ptr < ' ') *ptr = '.';
                ptr++;
            }

            ptr = hexPtr;

            /* Deal with IP address */
            if ((type == pstOctetString) && (n1->value == 4))
            {
                /* might be an IP address */
                ptr += sprintf(ptr, " <%u.%u.%u.%u> ", (UINT8)ip[0], (UINT8)ip[1], (UINT8)ip[2], (UINT8)ip[3]);
            }
            else
            {
                *ptr = ' ';
                ptr++;
            }
        }
    }

    /* 4. DEPENDENCY */
    ptr[0] = '.';
    ptr[1] = ' ';
    ptr[2] = ' ';
    ptr += 3;

    {
        pstChildExt child;
        vtNode* n2 = (vtNode *)rtGetByPath(rtH, rtParent(rtH, nodeId));

        /* Check if we've got a child... */
        if (hSyn && n2 != NULL)
        {
            int synField2 = VTN_SYN_NODE(n2);
            int synField1 = VTN_SYN_FIELD(n1);
            if (pstGetFieldExt(hSyn, synField2, synField1, &child) >= 0)
                if (child.speciality == pstDepending)
                {
                    int objectId;
                    if (pvtFindObject((HPVT)vt, nodeId, hSyn, child.enumerationValue, &objectId) >= 0)
                        ptr += sprintf(ptr, "[%s] ", stGetNameByNodeId(hSyn, objectId));
                }
        }
    }

    /* 5. GENERAL TYPE */
    if (((vt->degree & pvtPrintDegreeSyntax) != 0) && (hSyn != NULL))
    {
        int synField = VTN_SYN_NODE(n1);
        if ((vt->degree & pvtPrintDegreeSyntaxId) != 0)
            ptr += sprintf(ptr, "<%d> ", synField);

        ptr += pstPrintNode(hSyn, synField, ptr, sizeof(string) - (ptr-string));
    }

    /* Print the line we parsed */
    ptr[0] = '\n';
    ptr[1] = '\0';
    vt->pFunc((int)vt->pFuncParam, string);

    return param;
}


/************************************************************************
 * vtRootPrint
 * purpose: Callback function used to print a single root for debugging
 *          purposes only
 * input  : rtH     - RTree handle used
 *          nodeId  - Node to print
 *          param   - vtStruct of the node
 * output : none
 * return : vtStruct of the node (i.e. - param)
 ************************************************************************/
static void* vtRootPrint(IN HRTREE rtH, IN int nodeId, IN void* param)
{
    vtStruct*   vt = (vtStruct *)param;

    if (vt->printOnlyRoots)
        return vtNodePrint(rtH, nodeId, 0, param);
    else
    {
        rtPrint(vt->vTree, nodeId, 0, -1, param, vt->pFuncParam);
        return param;
    }
}


/* used for RVERROR reporting */
static const char *getSafeFieldName(vtNode *pNode)
{
    int synField = VTN_SYN_FIELD(pNode);
    return pstGetFieldNamePtr(pNode->hSyn, synField);
}


#else
#define vtNodePrint NULL
#endif  /* NOLOGSUPPORT */




/************************************************************************
 * vtSearchPath
 * purpose: Searches for a specified path, value or both in a specified tree
 *          This function is called by pvtSearchPath. It does less sanity
 *          checks and has no locking mechanisms in it
 * input  : vtDest      - PVT handle of the search tree
 *          rootNodeId  - Root ID of the search tree
 *          vtSrc       - PVT handle of path to be searched in
 *          srcNodeId   - Node ID of the beginning of the path to be searched in
 *          checkLeaves - If TRUE, Compare the values in the lowest level nodes (leaves)
 *                        Use FALSE if the tree syntax contains choices.
 * output : none
 * return : TRUE if found, FALSE if not
 *          Negative value on failure
 ************************************************************************/
static int
vtSearchPath(
    IN  vtStruct*   vtDest,
    IN  int         rootNodeId,
    IN  vtStruct*   vtSrc,
    IN  int         srcNodeId,
    IN  BOOL        checkLeaves)
{
    vtNode *vtNodeDest, *vtNodeSrc;
    int destCur, srcCur;
    int  destSynNodeId,  srcSynNodeId;
    HPST hSyn;
    /*  INTPTR destFieldId, srcFieldId;
    char *destString, *srcString;
    = pvtGetSynTree(destH, rootNodeId); */

    /*vtPrint(destH, rootNodeId, ErrVt); */
    vtNodeDest = GET_NODE(vtDest, rootNodeId);
    vtNodeSrc  = GET_NODE(vtSrc,  srcNodeId);
    hSyn       = vtNodeDest->hSyn;

    /*  vtGet(srcH, srcNodeId, &srcFieldId, &srcSynNodeId, &srcValue, &srcString);
    vtGet(destH, rootNodeId, &destFieldId, &destSynNodeId, &destValue, &destString);
    */
    destSynNodeId = VTN_SYN_NODE(vtNodeDest);
    srcSynNodeId  = VTN_SYN_NODE(vtNodeSrc);

    if (!pstAreNodesCongruent(
        hSyn,             destSynNodeId,
        vtNodeSrc->hSyn,  srcSynNodeId))
    {
        return RVERROR;
    }

    for (destCur = rootNodeId, srcCur = srcNodeId;
         destCur >=0 && srcCur >=0; )
    {
        destSynNodeId = VTN_SYN_NODE(vtNodeDest);
        srcSynNodeId  = VTN_SYN_NODE(vtNodeSrc);

        if (destSynNodeId != srcSynNodeId)
        {
            logPrint(vtSrc->log, RV_ERROR,
                     (vtSrc->log, RV_ERROR,
                     "vtSearchPath: %s Different synNodeId [%d <> %d].",
                     getSafeFieldName(vtNodeSrc),
                     destSynNodeId, srcSynNodeId));
            return FALSE;
        }

        switch (pstGetNodeType(vtNodeDest->hSyn, destSynNodeId))
        {
            case pstSet:
            case pstSequence:  /* investigate all children */
                if (pvtNumChilds((HPVT)vtSrc, srcCur) >1)
                {
                    int i;
                    INTPTR fieldId=-1;
                    int srcChild = pvtChild((HPVT)vtSrc, srcCur), destChild;
                    int children = pvtNumChilds((HPVT)vtSrc, srcCur);

                    for (i = 0; i < children; i++)
                    {
                        pvtGet((HPVT)vtSrc, srcChild, &fieldId, NULL, NULL, NULL);
                        destChild = pvtGetChild((HPVT)vtDest, destCur, fieldId, NULL);

                        if (destChild <0)
                        {
                            logPrint(vtSrc->log, RV_ERROR,
                                     (vtSrc->log, RV_ERROR,
                                     "vtSearchPath:Sequence: %s not in destination.",
                                     pstGetFieldNamePtr(hSyn, fieldId)));
                            return FALSE;
                        }
                        if (pvtSearchPath((HPVT)vtDest, destChild, (HPVT)vtSrc, srcChild, checkLeaves) != TRUE)
                            return FALSE;
                        srcChild = pvtBrother((HPVT)vtSrc, srcChild);
                    }
                    return TRUE;
                }
                break;

            case pstChoice:
                break;

            case pstSetOf:
            case pstSequenceOf:
                {
                    int j;
                    int children = pvtNumChilds((HPVT)vtDest, destCur);
                    int child    = pvtChild((HPVT)vtDest, destCur);
                    int srcChild = pvtChild((HPVT)vtSrc, srcCur);

                    pvtSetSyn((HPVT)vtSrc, hSyn, srcCur, -1, -1, NULL);

                    for (j = 0; j < children; j++)
                    {
                        if (pvtSearchPath((HPVT)vtDest, child, (HPVT)vtSrc, srcChild, checkLeaves) == TRUE)
                        {
                            logPrint(vtSrc->log, RV_DEBUG,
                                     (vtSrc->log, RV_DEBUG,
                                     "vtSearchPath:SetOf: [%d] %s found.",
                                     j + 1, getSafeFieldName(vtNodeSrc)));
                            return pvtSetSyn((HPVT)vtSrc, hSyn, srcCur, -1, j + 1, NULL);
                        }
                        child = pvtBrother((HPVT)vtDest, child);
                    }
                    logPrint(vtSrc->log, RV_ERROR,
                             (vtSrc->log, RV_ERROR,
                             "vtSearchPath:SetOf: subtree of %s not found.",
                             getSafeFieldName(vtNodeSrc)));
                    return FALSE;
                }

            default:
            {
                /* leaf holds value */

                if (!checkLeaves)
                {
                    /* not checking leaves - no need for comparison */
                    return TRUE;
                }

                logPrint(vtSrc->log, RV_DEBUG,
                         (vtSrc->log, RV_DEBUG,
                         "pvtSearchPath:Leaf:%s: %d <-> %d.",
                         getSafeFieldName(vtNodeSrc),
                         vtNodeDest->value, vtNodeSrc->value));

                /* compare values: */
                if (vtNodeDest->value != vtNodeSrc->value)
                    return FALSE;

                /* see if both had/don't have strings: */
                if ((vtNodeDest->string && !vtNodeSrc->string) ||
                    (!vtNodeDest->string && vtNodeSrc->string))
                    return FALSE;

                /* compare strings */
                if (vtNodeDest->string)
                {
                    if (rpoolCompareInternal(vtDest->sPool, vtNodeDest->string,
                        vtNodeSrc->string, (int)vtNodeDest->value))
                    {
                        return FALSE;
                    }
                }

                /* matched */
                return TRUE;
            }
        }

        {
            int synField;
            srcCur = pvtChild((HPVT)vtSrc, srcCur);
            vtNodeSrc = GET_NODE(vtSrc,  srcCur);
            synField = VTN_SYN_FIELD(vtNodeSrc);
            destCur = pvtGetChild((HPVT)vtDest, destCur, synField, NULL);
            vtNodeDest = GET_NODE(vtDest, destCur);
        }
    }

    return (srcCur < 0)? TRUE : FALSE;
}




BOOL pvtNodeCompare(RTElement e1, void *param)
{
    return ( (INTPTR)VTN_SYN_FIELD((vtNode *)e1) == (INTPTR)param );
}


static BOOL /* TRUE -if node is a bit string /FALSE - not a bit string */
pvtIsBitString(

          IN HPST hSyn,
          IN int nodeId)
{
    return (pstGetNodeType(hSyn, nodeId) == pstBitString)?TRUE:FALSE;
}






/*____________________________interface_______________________________________*/

static int /* syntax node id or RVERROR */
pvtResolveSynNodeId(
           IN  HPVT hVal,
           IN  HPST hSyn,
           IN  int valParentId,
           IN  INTPTR fieldEnum,
           OUT int *index /* of child in parent */
           )
{
    int parentSynNodeId=-1,parentId,childNodeId,objectId;
    pstChildExt child,field;
    pstTypeFromConstraint specialType;
    pstConstrainingField constrainingField;
    pstFieldOfObjectReference fieldOfObject;



    INTPTR parentFieldId;
    int _index,i;
    pstNodeType type;
    if (index) *index = RVERROR;

    /* -- resolve syntax node id */
    if (pvtGet(hVal, valParentId, &parentFieldId, &parentSynNodeId, NULL, NULL) <0)
        return RVERROR;

    /* -- SEQUENCE OF */
    if ((int)(type = pstGetNodeType(hSyn, parentSynNodeId)) != RVERROR)
        if (type == pstSequenceOf || type == pstSetOf)
            return pstGetNodeOfId(hSyn,parentSynNodeId);

    if ( (_index = pstGetFieldExt(hSyn, parentSynNodeId, (int)fieldEnum, &child)) <0)
    {
        logPrint(((vtStruct *)hVal)->log, RV_WARN,
                 (((vtStruct *)hVal)->log, RV_WARN,
                "pvtResolveSynNodeId: child '%s' does not exist under '%s'.",
                pstGetFieldNamePtr(hSyn, fieldEnum),
                pstGetFieldNamePtr(hSyn, parentFieldId)));
        return RVERROR;
    }
    if (index) *index = _index;

    if (child.speciality!=pstDependingDependent &&
        child.speciality!=pstDependent)
        return child.nodeId;


    /* getting special type */
    pstGetTypeFromConstraint(hSyn, child.nodeId, &specialType);

    /* getting constraining field */
    pstGetConstrainingField(hSyn, child.nodeId, &constrainingField);

    /* getting field itself(node) */
    /* here very intimate knowledge of the internal database is used*/
    /* it is known that fields are placed at nodeId+fieldIndex position */

    pstGetChildExt(hSyn, constrainingField.fieldId, 0, &field);

    for(i=0,parentId=valParentId;i<constrainingField.parentDepth;i++) /* going to parent */
        parentId=pvtParent(hVal,parentId);

    if((childNodeId=pvtGetChild(hVal,parentId,field.fieldId,&childNodeId))<0) /* going to depending field */
        return RVERROR;

    if(pvtFindObject(hVal,childNodeId,hSyn,specialType.objectSetId,&objectId)==RVERROR) /* finding object */
        return(UNDEFINED_TYPE);

    for(i=1;;i++)                                                           /* finding field of object */
    {
        if (pstGetFieldOfObjectReference(hSyn, objectId, i, &fieldOfObject) < 0)
            return RVERROR;
        if (fieldOfObject.fieldInClassNumber == specialType.fieldInClassNumber)
            break;
    }

    return fieldOfObject.settingId;
}


/*________________________________________add_________________________________*/


/************************************************************************
 * pvtSetNode
 * purpose: Set a node's value
 * input  : hVal        - PVT handle to use
 *          hSyn        - Syntax tree to use
 *                        Can be set to -1 if it will be supplied later (by pvtSetTree)
 *          nodePath    - Node Id to set
 *          nodeSynId   - Syntax node Id of the node or RVERROR if unknown
 *          parentId    - Parent's node Id or RVERROR if a root node
 *          fieldEnum   - Syntax tree field inside a type, or RVERROR if a root node
 *          value       - Value of the root node, or length of the value if
 *                        value is a string
 *          string      - String value of the node, or NULL if not a string
 * output : index       - Index of the new node (1-based)
 * return : Node id of the added root on success
 *          Negative value on failure
 ************************************************************************/
static int
pvtSetNode(
      IN  HPVT          hVal,
      IN  HPST          hSyn,
      IN  int           nodePath,
      IN  int           nodeSynId,
      IN  int           parentId,
      IN  INTPTR        fieldEnum,
      IN  INT32         value,
      IN  const char*   string,
      OUT int*          index)
{
    vtStruct *vt = (vtStruct *)hVal;
    vtNode *node;
    BOOL isBitStr;
    char bitsNotInUse;
    int synField;

    if (!vt) return RVERROR;

    if ( (node = (vtNode *)rtGetByPath (vt->vTree, nodePath)) == NULL)
    return RVERROR;

    node->hSyn = hSyn;
    if (nodeSynId == RVERROR)
        nodeSynId = pvtResolveSynNodeId(hVal, hSyn, parentId, fieldEnum, NULL);
    VTN_SET_SYN_INFO(node, nodeSynId, fieldEnum);

    if (index)
    *index = (parentId == RVERROR)?(1):(rtIndex(vt->vTree, parentId, nodePath));

    /* Irina * Changes for Bit String recognition */
    synField = VTN_SYN_NODE(node);
    if ((isBitStr = pvtIsBitString(hSyn, synField)))
    {
        if (value < 0)
        {
            isBitStr = 0;  /* node in temporary building state */
        }
        else
        {
            bitsNotInUse =  (char)((8 - value%8)%8);
            value = (INT32) bbSetByte(value) + 1;
        }
    }

    /* If we have a string with the same length already we'll use that string's allocation
       instead of reallocating the new one */
    if (node->string && (node->value != value || !string))
    {
        /* Not good - free the old one first... */
        rpoolFree(vt->sPool, node->string);
        node->string = NULL;
    }

    /* Set the value of this node */
    node->value = (value-isBitStr);

    synField = VTN_SYN_NODE(node);
    if ((pstIsStringNode(hSyn, synField) || string) && (value > 0))
    {
        /* We've got a string to think of here... */
        if (node->string == NULL)
        {
            /* No string - allocate a new one */
            node->string = (char*)rpoolAllocCopyExternal(vt->sPool, string, (int)value);
        }
        else
        {
            /* We've got a previous string - reuse it */
            if (rpoolCopyFromExternal(vt->sPool, node->string, string, 0, (int)value) == NULL)
                return RVERROR;
        }

        if (node->string == NULL)
            return RVERROR;

        if (isBitStr)/***POOL***/
        {
            /* Bit string - we should update the last byte in allocation with the amount
               of unused bits in the last actual byte */
            if (rpoolCopyFromExternal(vt->sPool, node->string, &bitsNotInUse, value-1, 1) == NULL)
                return RVERROR;
        }
    }

    return nodePath;
}

static
int /* new node id or RVERROR */
pvtAddSyn(
     /*
        Add child node under parentId.
        The new node is placed in its relative position according to syntax tree
        indexing of SEQUENCE fields of structure.
        */
     IN  HPVT hVal,
     IN  HPST hSyn, /* -1 ==> copy from parent node */
     IN  int parentId,
     IN  INTPTR fieldEnum, /* field inside SEQUENCE or CHOICE, or -1 if not applicable */
     IN  INT32 value, /* if string exists ==> size of string ,
                  if Bit String - length in bit*/
     IN  const char *string, /* NULL if no string */
     OUT int *newNodeId, /* id for the new node */
     OUT int *index) /* index of new node */
{
    vtStruct *vt = (vtStruct *)hVal;
    int path=RVERROR, childIndex, cur, pre, childCount;
    INTPTR curFieldId;
    int synParentNodeId, synNodeId;
    pstNodeType type;
    vtNode *parentNode;

    if (!vt || parentId<0) return RVERROR;

    /* -- Check choice brothers and delete if exist */
    parentNode = (vtNode *)rtGetByPath(vt->vTree, parentId);
    if (!parentNode)
        return RVERROR;
    synParentNodeId = VTN_SYN_NODE(parentNode);
    if (hSyn == (HPST)-1)
        hSyn = parentNode->hSyn;

    type=pstGetNodeType(hSyn, synParentNodeId);
    if (type == RVERROR) return RVERROR;

    if (type == pstChoice)
    {
        pvtDelete(hVal, pvtChild(hVal, parentId));
    }

    /* -- copy from parent */
#if 0
todo: remove
    if (fieldEnum ==-1)
    {
        if ((path = rtAddHead(vt->vTree, parentId, NULL)) <0)
        {
            logPrint(vt->log, RV_ERROR,
                     (vt->log, RV_ERROR,
                    "pvtAddSyn: rtAddHead failed for %s (%d).",
                    getSafeFieldName(parentNode), fieldEnum));
            return RESOURCES_PROBLEM;
        }
        if (index) *index = 1;
        if (newNodeId) *newNodeId = path;
        return pvtSetNode(
            hVal, parentNode->hSyn, path, synParentNodeId, parentId,
            VTN_SYN_FIELD(parentNode), value, string, NULL);
    }
#endif

    synNodeId = pvtResolveSynNodeId(hVal, hSyn, parentId, fieldEnum, &childIndex);
    if (synNodeId == RVERROR)
    {
        logPrint(vt->log, RV_WARN,
                 (vt->log, RV_WARN,
                 "pvtAddSyn: pvtResolveSynNodeId failed for %s (%d).",
                 pstGetFieldNamePtr(hSyn, fieldEnum), fieldEnum));
        return RVERROR;
    }
    if (childIndex <0) { /* SEQUENCE OF */
        if ((path = rtAddTail(vt->vTree, parentId, NULL)) <0)
        {
        logPrint(vt->log, RV_ERROR,
                 (vt->log, RV_ERROR,
                 "pvtAddSyn: rtAddTail failed for %s (%d).",
                 pstGetFieldNamePtr(hSyn, fieldEnum), fieldEnum));
            return RESOURCES_PROBLEM;
        }
    }
    else
    {
        /* Find the left brother of the new node (or none) so we know
           where to insert in the child list. */

        /* pre: the previous node - the left brother */
        pre = RVERROR;  /* RVERROR -> add as leftmost */
        childCount = pstGetNumberOfChildren(hSyn, synParentNodeId);
        if (childCount > 0)
        {
            cur = pvtChild(hVal, parentId);
            if (cur >= 0)
            {
                pstChildExt st_child;
                int stChildIndex = 1;
                vtNode *node;

                node = (vtNode *)rtGetByPath(vt->vTree, cur);
                if (node != NULL)
                    curFieldId = VTN_SYN_FIELD(node);
                else
                    curFieldId = RVERROR;

                for (;;)
                {
                    /* this is just a sanity check: */
                    if (stChildIndex > childCount)
                    {
                        /* should never reach here */
                        logPrint(vt->log, RV_ERROR,
                                 (vt->log, RV_ERROR,
                                 "pvtAddSyn: probable integrity failiure in vt %x.", hVal));
                        pre = -1; /* allow leftmost insertion */
                        break;
                    }

                    if (pstGetChildExt(hSyn, synParentNodeId,
                        stChildIndex, &st_child) < 0)
                    {
                        pre = -1;
                        break;
                    }
                    if (st_child.fieldId == fieldEnum)
                        break;
                    if (st_child.fieldId != curFieldId)
                    {
                        stChildIndex++;
                        continue;
                    }
                    pre = cur;
                    cur = rtBrother(vt->vTree, cur);
                    if (cur < 0)
                    {
                        break;
                    }

                    node = (vtNode *)rtGetByPath(vt->vTree, cur);
                    if (node != NULL)
                        curFieldId = VTN_SYN_FIELD(node);
                    else
                        curFieldId = RVERROR;
                }

                if (cur>=0 && curFieldId==fieldEnum)
                    path=cur;
            }
        }
        if (path<0)
        {
            if (pre < 0)
            {
                if ((path = rtAddHead(vt->vTree, parentId, NULL)) <0)
                {
                    logPrint(vt->log, RV_ERROR,
                             (vt->log, RV_ERROR,
                             "pvtAddSyn: rtAddHead (left brother) failed for %s (%d).",
                             pstGetFieldNamePtr(hSyn, fieldEnum), fieldEnum));
                    return RESOURCES_PROBLEM;
                }
            }
            else
            {
                if ((path = rtAddBrother(vt->vTree, pre, NULL)) <0)
                {
                    logPrint(vt->log, RV_ERROR,
                             (vt->log, RV_ERROR,
                             "pvtAddSyn: rtAddBrother failed for %s (%d).",
                             pstGetFieldNamePtr(hSyn, fieldEnum), fieldEnum));
                    return RESOURCES_PROBLEM;
                }
            }
        }
    }

    if (newNodeId) *newNodeId = path;
    return pvtSetNode(hVal, hSyn, path, synNodeId, parentId, fieldEnum, value, string, index);
}


static void *
pvtAddTreeFunc(HRTREE rtH, int nodeId, void *vts)
{
    vtNode *node = (vtNode *)rtGetByPath(rtH, nodeId);
    vtStructs* v = (vtStructs *)vts;
    char *string;

    if (!vts || !node) return NULL;
    string = node->string;
    if (string)
    {
        if (!(node->string = (char*)rpoolAllocCopyInternal(v->dest->sPool, v->src->sPool, string, (int)node->value)))
        {
            logPrint(v->dest->log, RV_ERROR,
                     (v->dest->log, RV_ERROR,
                     "pvtAddTreeFunc: String allocation failed for '%d' (%d).",
                     string, (int)node->value));
            return NULL;
        }
    }

    return vts;
}

/*________________________________________delete_________________________________*/
static void *
pvtDeleteFunc(HRTREE rtH, int nodeId, void *hVal)
{
    vtNode *node = (vtNode *)rtGetByPath(rtH, nodeId);
    vtStruct *vt = *(vtStruct **)hVal;

    if (!hVal || !node) return NULL;

    if (node->string)
    {
        if (rpoolFree(vt->sPool, node->string) <0)
        {
            logPrint(vt->log, RV_ERROR,
                     (vt->log, RV_ERROR,
                     "pvtDeleteFunc:%d: Failed to free string from pool [0x%x].",
                     nodeId, node->string));
        }
    }

    return hVal;
}





/*________________________________________get&set node_________________________________*/

static int
pvtSetSyn(
     /* Desc:Set values of existed node    */
     IN  HPVT hVal,
     IN  HPST hSyn, /* -1 ==> don't change */
     IN  int nodeId,
     IN  INTPTR fieldEnum, /* <0 not changing current value */
     IN  INT32 value,      /* if bit string - length in bits */
     IN  const char *string)
{
    vtStruct *vt = (vtStruct *)hVal;
    vtNode *node;
    char bitsNotInUse;
    BOOL isBitStr;
    int synField;

    if (!vt) return RVERROR;
    if (! (node = (vtNode *)rtGetByPath(vt->vTree, nodeId)) ) return RVERROR;

    if (hSyn == (HPST)-1)
    {
        hSyn = node->hSyn;
    }
    else
    {
        node->hSyn = hSyn;
    }

    if ( fieldEnum >= 0 )
    {
        if (fieldEnum != (INTPTR)VTN_SYN_FIELD(node) && /* changed */
          pvtParent(hVal, nodeId) >=0) /* not root */
        {
            int synNodeId = pvtResolveSynNodeId(hVal, hSyn, pvtParent(hVal, nodeId), fieldEnum, NULL);
            VTN_SET_SYN_NODE(node, synNodeId);
        }
        VTN_SET_SYN_FIELD(node, fieldEnum);
    }

    /* Irina * changes for Bit String  */
    synField = VTN_SYN_NODE(node);
    if ((isBitStr = pvtIsBitString(hSyn, synField)))
    {
        bitsNotInUse = (char)((8 - value%8)%8);
        value = (INT32) bbSetByte(value) + 1;
    }
    /***/

    if (node->string && (node->value != value || !string))
    {
        rpoolFree(vt->sPool, node->string);
        node->string=NULL;
    }

    synField = VTN_SYN_NODE(node);
    if ((pstIsStringNode(hSyn, synField) || string) && value>0)
    {
        if (node->value != value || node->string==0)
        {
            if (!(node->string = (char*)rpoolAllocCopyExternal(vt->sPool, string, (int)value)))
                return RVERROR;
        }
        else
            if (string) rpoolCopyFromExternal(vt->sPool, node->string, string, 0, (int)value);

        /* Irina * changes for Bit String recognition */
        /*******POOL**********************/
        if (isBitStr)
            rpoolCopyFromExternal(vt->sPool, node->string, &bitsNotInUse, value-1, 1);
        /***/
    }

    node->value = (value-isBitStr);

    return TRUE;
}




/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


/************************************************************************
 * pvtConstruct
 * purpose: Construct a pool of PVT nodes.
 * input  : stringBufferSize    - Not used
 *          numOfNodes          - The maximum number of nodes to be allowed
 *                                in the tree
 * output : none
 * return : Handle to PVT constructed on success
 *          NULL on failure
 ************************************************************************/
RVAPI HPVT RVCALLCONV
pvtConstruct(
        IN  int stringBufferSize,
        IN  int numOfNodes)
{
    vtStruct *vt;
    if(stringBufferSize);

    if (!(vt = (vtStruct *)calloc(sizeof(vtStruct), 1)))
        return NULL;

    vt->log = logRegister((RVHLOGMGR)1, "VT", "Value Tree");
    vt->unregLog = logRegister((RVHLOGMGR)1, "UNREG", "Any non-registered user log");

    vt->vTree = rtConstruct(sizeof(vtNode), numOfNodes, (RVHLOGMGR)1, "VT tree");
    vt->sPool = (HRPOOL)vt->vTree;/*rpoolConstruct(stringBufferSize);*/

    vt->lock = meiInit();

    if (!vt->vTree || !vt->sPool)
    {
        pvtDestruct((HPVT)vt);
        return NULL;
    }

    rtSetCompareFunc(vt->vTree, pvtNodeCompare);
    rtSetPrintFunc(vt->vTree, vtNodePrint);

    return (HPVT)vt;
}


/************************************************************************
 * pvtDestruct
 * purpose: Destruct a pool of PVT nodes.
 * input  : valH    - The PVT handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDestruct(IN  HPVT hVal)
{
    vtStruct *vt = (vtStruct *)hVal;

    if (!vt) return RVERROR;

    rtDestruct(vt->vTree);
    meiEnd(vt->lock);

    free(vt);
    return TRUE;
}


/************************************************************************
 * pvtGetRoot
 * purpose: Returns the Node ID of the root of a Value Tree to which a
 *          specified node belongs
 * input  : valH    - The PVT handle
 *          nodeId  - The ID of the node inside a tree
 * output : none
 * return : Root node ID of the given node ID on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetRoot(
       IN  HPVT hVal,
       IN  int nodeId)
{
    int result;
    vtStruct* vt = (vtStruct *)hVal;

    if (!hVal) return RVERROR;

    meiEnter(vt->lock);
    result = rtGetRoot(vt->vTree, nodeId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtGetSynTree
 * purpose: Returns a handle to the structure (Syntax Tree) definition of
 *          a specified node
 * input  : valH    - The PVT handle
 *          nodeId  - The ID of the node the syntax of which is returned
 * output : none
 * return : A handle to the structure (Syntax Tree) definition on success
 *          NULL on failure
 ************************************************************************/
RVAPI HPST RVCALLCONV
pvtGetSynTree(
          IN  HPVT hVal,
          IN  int nodeId)
{
    HPST        result = NULL;
    vtNode*     node;
    vtStruct*   vt = (vtStruct *)hVal;

    if (!hVal) return NULL;

    meiEnter(vt->lock);
    if ( (node = (vtNode *)rtGetByPath(vt->vTree, nodeId)) )
        result = node->hSyn;
    meiExit(vt->lock);

    return result;
}



/*** Statistics ***/

/************************************************************************
 * pvtCurSize
 * purpose: Gets the number of nodes currently used in the Value Tree forest
 * input  : valH    - The PVT handle
 * output : none
 * return : The number of nodes in the Value Tree forest on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtCurSize(IN HPVT hVal)
{
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;
    return rtCurSize(vt->vTree);
}


/************************************************************************
 * pvtMaxUsage
 * purpose: Gets the highest number of nodes used in the Value Tree forest
 *          since the cmInitialize() function was called.
 * input  : valH    - The PVT handle
 * output : none
 * return : The maximum number of nodes used in the Value Tree forest on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtMaxUsage(IN HPVT hVal)
{
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;
    return rtMaxUsage(vt->vTree);
}


/************************************************************************
 * pvtMaxSize
 * purpose: Gets the highest number of nodes that cab be used in the Value Tree forest
 * input  : valH    - The PVT handle
 * output : none
 * return : The maximum number of nodes in the Value Tree forest on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtMaxSize(IN HPVT hVal)
{
    if (!hVal) return RVERROR;
    return rtMaxSize(((vtStruct *)hVal)->vTree);
}


/************************************************************************
 * pvtPoolStatistics
 * purpose: Get pool statistics (space is in bytes)
 * input  : valH            - The PVT handle
 * output : poolSize        - Maximum size of pool
 *          availableSpace  - Current available space
 *          maxFreeChunk    - Always returned as 0
 *          numOfChunks     - Always returned as 0
 *          If any output parameter is set to NULL, it will be discarded
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtPoolStatistics(
          IN  HPVT   hVal,
          OUT INT32* poolSize,
          OUT INT32* availableSpace,
          OUT INT32* maxFreeChunk,
          OUT INT32* numOfChunks)
{
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;
    if (maxFreeChunk) *maxFreeChunk=0;
    if (numOfChunks)  *numOfChunks=0;
    return rpoolStatistics(vt->sPool, poolSize, availableSpace, NULL);
}



/************************************************************************
 * pvtTreeSize
 * purpose: Returns the number of nodes included in a Value Tree root
 * input  : valH        - The PVT handle
 *          parentId    - The ID of any node. The function returns the
 *                        number of nodes located under the specified parent.
 * output : none
 * return : The number of nodes included in a Value Tree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtTreeSize(
        IN  HPVT    hVal,
        IN  int     parentId)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtTreeSize(vt->vTree, parentId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtNumChilds
 * purpose: Returns the number of the dependents (children) of a parent tree
 * input  : valH        - The PVT handle
 *          parentId    - The ID of any node
 * output : none
 * return : The number of immediate nodes under the given node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtNumChilds(
         IN  HPVT   hVal,
         IN  int    parentId)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtNumChilds(vt->vTree, parentId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtParent
 * purpose: Returns the ID of the parent node of a specified node
 * input  : valH        - The PVT handle
 *          nodeId      - The ID of any node
 * output : none
 * return : The parent ID of the given node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtParent(
      IN  HPVT  hVal,
      IN  int   nodeId)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtParent(vt->vTree, nodeId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtBrother
 * purpose: Returns the Node ID of a specified node's brother (right).
 * input  : valH        - The PVT handle
 *          nodeId      - The ID of any node
 * output : none
 * return : The node ID of the given node's brother on success
 *          Negative value on failure
 * The function returns the next child (rightward). Use pvtChild() to get
 * the first dependent, and then use pvtBrother() to get to the next brother.
 ************************************************************************/
RVAPI int RVCALLCONV
pvtBrother(
       IN  HPVT hVal,
       IN  int  nodeId)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtBrother(vt->vTree, nodeId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtLBrother
 * purpose: Gets the ID of the node before (left) a particular node
 * input  : valH        - The PVT handle
 *          nodeId      - The ID of any node
 * output : none
 * return : The node ID of the previous node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtLBrother(
       IN  HPVT hVal,
       IN  int  nodeId)
{
    vtStruct* vt = (vtStruct *)hVal;
    int parentId;
    int lbrotherId = RVERROR;
    int currId;

    meiEnter(vt->lock);
    parentId = pvtParent(hVal, nodeId);
    if (parentId < 0)
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    currId = pvtChild(hVal, parentId);

    while (currId != nodeId)
    {
        lbrotherId = currId;
        currId = pvtBrother(hVal, currId);
    }

    meiExit(vt->lock);
    return lbrotherId;
}


/************************************************************************
 * pvtChild
 * purpose: Returns the Node ID of the first (leftmost) child of a parent.
 * input  : valH        - The PVT handle
 *          parentId    - The ID of any node
 * output : none
 * return : The Node ID of the first (leftmost) child of a parent on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtChild(
     IN  HPVT   hVal,
     IN  int    parentId)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtHead(vt->vTree, parentId);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtNext
 * purpose: Returns the ID of a node located after a specified node.
 * input  : valH    - The PVT handle
 *          rootId  - The ID of the root node of the current Value Tree
 *          location- A Node ID inside the given root
 * output : none
 * return : The Node ID of the next node in the tree on success - this
 *          value should be given as the location on the next call.
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtNext(
    IN  HPVT    hVal,
    IN  int     rootId,
    IN  int     location)
{
    int result;
    vtStruct *vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = rtNext(vt->vTree, rootId, location);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtAddRootFromInt
 * purpose: Add a new root to PVT. This function should not be used
 *          directly by user applications. It is called internally through
 *          macros or other stack functions.
 * input  : hVal    - PVT handle to use
 *          hSyn    - Syntax tree to use
 *                    Can be set to -1 if it will be supplied later (by pvtSetTree)
 *          nodePath- Syntax node Id to use for the root.
 *                    Can be set to -1 if it will be supplied later or if
 *                    it should be the root of the syntax tree created.
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 *          fileName- Filename that called this function
 *                    NULL if unknown
 *          lineno  - Line number in the filename that called this function
 *                    0 if unknown
 * output : none
 * return : Node id of the added root on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAddRootFromInt(
           IN  HPVT         hVal,
           IN  HPST         hSyn,
           IN  int          nodePath,
           IN  INT32        value,
           IN  char*		string,
           IN  const char*  fileName,
           IN  int          lineno)
{
    vtStruct *vt = (vtStruct *)hVal;
    int path, result;
    if (lineno || fileName);

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    /* Try and add the new root */
    if ( (path = rtAddRoot(vt->vTree, NULL)) <0)
    {
        logPrint(vt->log, RV_ERROR,
                 (vt->log, RV_ERROR,
                 "pvtAddRootFromInt: (@%s:%d) Cannot add root [size=%d]",
                 nprn(fileName), lineno, rtCurSize(vt->vTree)));
        meiExit(vt->lock);
        return RESOURCES_PROBLEM;
    }

    logPrint(vt->log, RV_DEBUG,
             (vt->log, RV_DEBUG,
             "pvtAddRootFromInt: (@%s:%d) hVal=0x%x. Added root=%d",
             nprn(fileName), lineno, hVal, path));

    /* Make sure we know the syntax node Id of the added node */
    if (nodePath == RVERROR)
        nodePath = pstGetRoot(hSyn);

    /* Set the value of the root node */
    result = pvtSetNode(hVal, hSyn, path, nodePath, RVERROR, RVERROR, value, string, NULL);

    meiExit(vt->lock);
    return result;
}


/* Remove definitions of macros for these functions. The definitions are there to allow
   printing information about these functions */
#if defined(pvtAddRoot) || defined(pvtAddRootByPath)
#undef pvtAddRoot
#undef pvtAddRootByPath
#endif


/************************************************************************
 * pvtAddRoot
 * purpose: Adds a new node that constitutes the root of a new tree.
 * input  : valH    - PVT handle to use
 *          synH    - Syntax tree to use
 *                    Can be set to -1 if it will be supplied later (by pvtSetTree)
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 * output : none
 * return : Node id of the added root on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAddRoot(
    IN  HPVT        valH,
    IN  HPST        synH,
    IN  INT32       value,
    IN  const char* string)
{
    return pvtAddRootFromInt(valH,synH,RVERROR,value,(char *)string,NULL,0);
}



/************************************************************************
 * pvtAddRootByPath
 * purpose: Adds a new node that constitutes the root of a new tree.
 * input  : valH        - PVT handle to use
 *          synH        - Syntax tree to use
 *                        Can be set to -1 if it will be supplied later (by pvtSetTree)
 *          syntaxPath  - A path separated by periods (for example, "a.b.c") that
 *                        identifies the node of the Value Tree's syntax node.
 *                        The path starts from the syntax handle root node
 *          value       - Value of the root node, or length of the value if
 *                        value is a string
 *          string      - String value of the node, or NULL if not a string
 * output : none
 * return : Node id of the added root on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAddRootByPath(
    IN  HPVT    valH,
    IN  HPST    synH,
    IN  char*   syntaxPath,
    IN  INT32   value,
    IN  char*   string)
{
    return pvtAddRootFromInt(valH,synH,pstGetNodeIdByPath(synH, syntaxPath),value,string,NULL,0);
}




/************************************************************************
 * pvtAdd
 * purpose: Add child node under parentId.
 *          The new node is placed in its relative position according to syntax tree
 *          indexing of SEQUENCE fields of structure.
 * input  : valH        - PVT handle to use
 *          parentId    - The Node ID of the immediate parent of the new node.
 *          fieldId     - The field ID returned by pstGetFieldId().
 *                        If set to -1, it will be copied from the parent node.
 *          value       - Value of the root node, or length of the value if
 *                        value is a string
 *          string      - String value of the node, or NULL if not a string
 * output : index       - The index of the added child (1-based)
 * return : Node id of the added node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAdd(
    IN  HPVT        hVal,
    IN  int         parentId,
    IN  INTPTR      fieldId,
    IN  INT32       value,
    IN  const char* string,
    OUT int*        index)
{
    int result;
    vtStruct* vt = (vtStruct *)hVal;

    meiEnter(vt->lock);
    result = pvtAddSyn(hVal, (HPST)-1, parentId, fieldId, value, string, NULL, index);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtAddTree
 * purpose: Copies a sub-tree and places it under a specified parent in
 *          another sub-tree.
 * input  : destH       - The handle returned by cmGetValTree() for the
 *                        parent (destination).
 *          destNodeId  - The Node ID of the parent (destination).
 *                        The new sub-tree is placed under this node.
 *          srcH        - The handle returned by cmGetValTree() for the
 *                        source (copied sub-tree).
 *          srcNodeId   - The ID of the root node of the source sub-tree.
 * output : none
 * return : Node id of the added node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAddTree(
    IN  HPVT    destH,
    IN  int     destNodeId,
    IN  HPVT    srcH,
    IN  int     srcNodeId)
{
    vtStructs   v;
    HMEI        lock1, lock2;
    int         result;

    if (!destH || !srcH) return RVERROR;

    v.dest = (vtStruct *)destH;
    v.src = (vtStruct *)srcH;

    /* Make sure we lock an an orderly fashion to diminish dead-locking possibilities */
    if (destH > srcH)
    {
        lock1 = v.src->lock;
        lock2 = v.dest->lock;
    }
    else
    {
        lock1 = v.dest->lock;
        lock2 = v.src->lock;
    }

    meiEnter(lock1);
    meiEnter(lock2);
    result = rtAdd(v.dest->vTree, destNodeId, v.src->vTree, srcNodeId, pvtAddTreeFunc, (void *)&v);
    meiExit(lock2);
    meiExit(lock1);

    return result;
}


/************************************************************************
 * pvtAddChilds
 * purpose: Copies a sub-tree, excluding its root node, and places it under
 *          a specified parent.
 * input  : destH       - The handle returned by cmGetValTree() for the
 *                        parent (destination).
 *          destNodeId  - The Node ID of the parent (destination).
 *                        The new sub-tree is placed under this node.
 *          srcH        - The handle returned by cmGetValTree() for the
 *                        source (copied sub-tree).
 *          srcNodeId   - The ID of the root node of the source sub-tree.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAddChilds(
    IN  HPVT    destH,
    IN  int     destNodeId,
    IN  HPVT    srcH,
    IN  int     srcNodeId)
{
    vtStructs   v;
    HMEI        lock1, lock2;
    int         result;

    if (!destH || !srcH) return RVERROR;

    v.dest = (vtStruct *)destH;
    v.src = (vtStruct *)srcH;

    /* Make sure we lock an an orderly fashion to diminish dead-locking possibilities */
    if (destH > srcH)
    {
        lock1 = v.src->lock;
        lock2 = v.dest->lock;
    }
    else
    {
        lock1 = v.dest->lock;
        lock2 = v.src->lock;
    }

    meiEnter(lock1);
    meiEnter(lock2);
    result = rtAddChilds(v.dest->vTree, destNodeId, v.src->vTree, srcNodeId, pvtAddTreeFunc, (void *)&v);
    meiExit(lock2);
    meiExit(lock1);

    return result;
}


/************************************************************************
 * pvtDelete
 * purpose: Deletes a sub-tree
 * input  : valH            - PVT handle to use
 *          subTreeRootId   - The ID of the root node to delete
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDelete(
      IN  HPVT  hVal,
      IN  int   subTreeRootId)
{
    vtStruct *vt = (vtStruct *)hVal;
    int result;

    if (subTreeRootId < 0) return RVERROR;

    meiEnter(vt->lock);

#ifndef NOLOGSUPPORT
    /* If it's a root node, we notify that it's being deleted */
    if (rtParent(vt->vTree, subTreeRootId) < 0)
    {
        logPrint(vt->log, RV_DEBUG,
                 (vt->log, RV_DEBUG,
                 "pvtDelete: hVal=0x%x. Deleted root=%d", hVal, subTreeRootId));
    }
#endif

    result = rtDelete(vt->vTree, subTreeRootId, pvtDeleteFunc, (void *)&hVal);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtDeleteChilds
 * purpose: Deletes the children of root node.
 * input  : valH            - PVT handle to use
 *          subTreeRootId   - The ID of the root node to delete
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDeleteChilds(
        IN  HPVT    hVal,
        IN  int     subTreeRootId)
{
    vtStruct *vt = (vtStruct *)hVal;
    int result;

    if (subTreeRootId < 0) return RVERROR;

    meiEnter(vt->lock);
    result = rtDeleteChilds(vt->vTree, subTreeRootId, pvtDeleteFunc, (void *)&hVal);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtDeleteAll
 * purpose: Deletes all the nodes in a Value Tree.
 * input  : valH            - PVT handle to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDeleteAll(IN  HPVT hVal)
{
    vtStruct *vt = (vtStruct *)hVal;

    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    rtClear(vt->vTree);
    rpoolClear(vt->sPool);

    logPrint(vt->log, RV_DEBUG,
             (vt->log, RV_DEBUG,
             "pvtDeleteAll: Called for hVal=0x%x", hVal));
    meiExit(vt->lock);

    return TRUE;
}


/************************************************************************
 * pvtSetTree
 * purpose: Copies a sub-tree from one Value Tree to another,
 *          overwriting the destination's tree
 * input  : destH       - The handle returned by cmGetValTree() for the
 *                        parent (destination).
 *          destNodeId  - The Node ID of the destination sub-tree.
 *                        The copied sub-tree overwrites this one
 *          srcH        - The handle returned by cmGetValTree() for the
 *                        source (copied sub-tree).
 *          srcNodeId   - The ID of the root node of the source sub-tree.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtSetTree(
    IN  HPVT    destH,
    IN  int     destNodeId,
    IN  HPVT    srcH,
    IN  int     srcNodeId)
{
    vtStructs   v;
    UINT32      destSynFieldAndNode;
    HPST        destSynH;
    vtNode*     node;
    HMEI        lock1, lock2;
    int         result;

    if (!destH || !srcH || destNodeId<0 || srcNodeId<0) return RVERROR;
    if ((destH == srcH) && (destNodeId == srcNodeId)) return TRUE;

    v.dest = (vtStruct *)destH;
    v.src = (vtStruct *)srcH;

    /* Make sure we lock in an orderly fashion to diminish dead-locking possibilities */
    if (destH > srcH)
    {
        lock1 = v.src->lock;
        lock2 = v.dest->lock;
    }
    else
    {
        lock1 = v.dest->lock;
        lock2 = v.src->lock;
    }

    meiEnter(lock1);
    meiEnter(lock2);

    if (!(node = (vtNode *)rtGetByPath(v.dest->vTree, destNodeId)))
        result = RVERROR;
    else
    {
        destSynFieldAndNode = node->synFieldAndNode;
        destSynH = node->hSyn;

        result = rtSet(v.dest->vTree, destNodeId, v.src->vTree, srcNodeId,
            pvtAddTreeFunc, pvtDeleteFunc, (void*)&v);
        if (result >= 0)
        {
            if (destSynH)
                node->synFieldAndNode = destSynFieldAndNode;
            result = TRUE;
        }
    }

    meiExit(lock2);
    meiExit(lock1);

    return result;
}


/************************************************************************
 * pvtMoveTree
 * purpose: Moves a sub-tree to another location within the same Value Tree
 * input  : destH       - The handle returned by cmGetValTree() for the
 *                        parent (destination).
 *          destRootId  - The ID of the root node to which the sub-tree is
 *                        moved (destination root) - this node is overwritted.
 *          srcRootId   - The ID of the root node to which the sub-tree
 *                        belongs (source node).
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtMoveTree(
    IN  HPVT    destH,
    IN  int     destRootId,
    IN  int     srcRootId)
{
    vtStruct *vtDest = (vtStruct *)destH;
    UINT32 destSynFieldAndNode;
    HPST destSynH;
    vtNode *node;

    if (!destH) return RVERROR;
    meiEnter(vtDest->lock);

    if (!(node = (vtNode *)rtGetByPath(vtDest->vTree, destRootId)))
    {
        meiExit(vtDest->lock);
        return RVERROR;
    }
    destSynFieldAndNode = node->synFieldAndNode;
    destSynH = node->hSyn;

#ifndef NOLOGSUPPORT
    /* If it's a root node, we notify that it's being moved */
    if (rtParent(vtDest->vTree, srcRootId) < 0)
    {
        logPrint(vtDest->log, RV_DEBUG,
                 (vtDest->log, RV_DEBUG,
                 "pvtMoveTree: hVal=0x%x. root=%d is being moved as a sub-tree",
                 destH, srcRootId));
    }
#endif

    if (rtMove(vtDest->vTree, destRootId, srcRootId, FALSE,
        pvtDeleteFunc, (void *)&destH) < 0)
    {
        meiExit(vtDest->lock);
        return RVERROR;
    }

    /* Make sure the syntax in the destination node is the one used */
    if (destSynH)
        node->synFieldAndNode = destSynFieldAndNode;

    meiExit(vtDest->lock);
    return TRUE;
}


/************************************************************************
 * pvtShiftTree
 * purpose: Moves a sub-tree to another location within the same Value Tree,
 *          without changing the value of the source handle.
 * input  : destH       - The handle returned by cmGetValTree() for the
 *                        parent (destination).
 *          destRootId  - The ID of the root node to which the sub-tree
 *                        is moved (destination root).
 *          srcRootId   - The ID of the root node to which the sub-tree
 *                        belongs (source node).
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtShiftTree(
    IN  HPVT    destH,
    IN  int     destRootId,
    IN  int     srcRootId)
{
    vtStruct *vtDest = (vtStruct *)destH;
    UINT32 destSynFieldAndNode;
    HPST destSynH;
    vtNode *node;

    if (!destH) return RVERROR;
    meiEnter(vtDest->lock);

    if (!(node = (vtNode *)rtGetByPath(vtDest->vTree, destRootId)))
    {
        meiExit(vtDest->lock);
        return RVERROR;
    }
    destSynFieldAndNode = node->synFieldAndNode;
    destSynH = node->hSyn;

    if (rtMove(vtDest->vTree, destRootId, srcRootId, TRUE,
        pvtDeleteFunc, (void *)&destH) < 0)
    {
        meiExit(vtDest->lock);
        return RVERROR;
    }

    /* Make sure the syntax in the destination node is the one used */
    if (destSynH)
        node->synFieldAndNode = destSynFieldAndNode;

    meiExit(vtDest->lock);
    return TRUE;
}



/************************************************************************
 * pvtAdoptChild
 * purpose: Moves the child of a specific tree to below a specific node in
 *          a different tree. This process is referred to as adopting a child
 * input  : valH            - PVT handle to use
 *          adoptedChildId  - The child you want to move
 *          newParentId     - The node that you want to move the child under.
 *                            This node becomes the root. If this is set to -1,
 *                            then the node becomes a root
 *          newBrotherId    - The node below the new root that you want the new
 *                            child to follow. If -1, then the node is the first
 *                            born.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtAdoptChild(
    IN  HPVT    hVal,
    IN  int     adoptedChildId,
    IN  int     newParentId,
    IN  int     newBrotherId)
{
    int result;
    vtStruct* vt = (vtStruct *)hVal;
    if (vt == NULL) return RVERROR;

    meiEnter(vt->lock);
    result = rtAdoptChild(vt->vTree, adoptedChildId, newParentId, newBrotherId);
    if ((result >= 0) && (newParentId < 0))
    {
        logPrint(vt->log, RV_DEBUG,
                 (vt->log, RV_DEBUG,
                 "pvtAdoptChild: hVal=0x%x. Changed to a root=%d", hVal, adoptedChildId));
    }
    meiExit(vt->lock);

    return result;
}



/************************************************************************
 * pvtGet
 * purpose: Returns the value stored in a node (integer or a string) or an
 *          indication as to the type of the value
 * input  : valH        - PVT handle to use
 *          nodeId      - The ID of the node to check
 * output : fieldId     - The returned field ID of the node. You can then
 *                        use the pstGetFieldName() function to obtain the field name
 *          synNodeId   - The ID of the node in the Syntax Tree
 *          value       - Value of the root node, or length of the value if
 *                        value is a string
 *          isString    - TRUE if node contains a string - see pvtGetString()
 * return : The node Id value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGet(
    IN  HPVT    hVal,
    IN  int     nodeId,
    OUT INTPTR* fieldId,
    OUT int*    synNodeId,
    OUT INT32*  value,
    OUT BOOL*   isString)
{
    vtNode *node;
    vtStruct* vt = (vtStruct *)hVal;
    int synField;

    if (!hVal) return RVERROR;
    meiEnter(vt->lock);

    if (!(node = (vtNode *)rtGetByPath(((vtStruct *)hVal)->vTree, nodeId)) )
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    if (fieldId)    *fieldId = VTN_SYN_FIELD(node);
    if (synNodeId)  *synNodeId = VTN_SYN_NODE(node);
    if (value)      *value = node->value;

    synField = VTN_SYN_NODE(node);
    if (isString)
        *isString = (node->string != NULL ||
                    pstIsStringNode(pvtGetSynTree(hVal, nodeId), synField))?
                                    TRUE:FALSE;

    meiExit(vt->lock);
    return nodeId;
}


/************************************************************************
 * pvtGetString
 * purpose: Returns the value stored in the node, if the value is of string type.
 * input  : valH            - PVT handle to use
 *          nodeId          - The ID of the node to check
 *          stringLength    - The size of the buffer that will hold the returned value
 * output : string          - The return string value. This is a buffer allocated
 *                            by the user to hold the string
 * return : Actual string's length on success
 *          Negative value on failure
 ************************************************************************/
RVAPI INT32 RVCALLCONV
pvtGetString(
    IN  HPVT    hVal,
    IN  int     nodeId,
    IN  INT32   stringLength,
    OUT char*   string)
{
    vtStruct *vt = (vtStruct *)hVal;
    vtNode *node;
    INT32 result;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    if (! (node = (vtNode *)rtGetByPath(vt->vTree, nodeId)) )
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    if (string && stringLength>0) string[0]=0;

    if (!node->string)
    {
        /* no string */
        meiExit(vt->lock);
        return RVERROR;
    }

    if (string && stringLength>0)
    rpoolCopyToExternal(vt->sPool, string, node->string,0, (int)stringLength);
    result = node->value;

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtGetBitString
 * purpose: Returns the value stored in the node, if the value is of a bit string type.
 * input  : valH            - PVT handle to use
 *          nodeId          - The ID of the node to check
 *          stringLength    - The size of the buffer that will hold the returned value
 *                            The length is gien in bytes
 * output : string          - The return string value. This is a buffer allocated
 *                            by the user to hold the string
 * return : Actual string's length in bits on success
 *          Negative value on failure
 ************************************************************************/
RVAPI INT32 RVCALLCONV
pvtGetBitString(
    IN  HPVT    hVal,
    IN  int     nodeId,
    IN  INT32   stringLength,
    OUT char*   string)
{
    vtStruct *vt = (vtStruct*)hVal;
    vtNode *node;
    char bitsNotInUse=0;
    int lengthInBits;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    if (! (node = (vtNode *)rtGetByPath(vt->vTree, nodeId)) )
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    if (string && stringLength>0) string[0]=0;

    if (!node->string)
    {
        /* no string */
        meiExit(vt->lock);
        return RVERROR;
    }

    if (string && stringLength>0)
        rpoolCopyToExternal(vt->sPool,&bitsNotInUse ,node->string ,
               node->value-1, 1);

    lengthInBits = node->value*8 - bitsNotInUse;

    if (string && stringLength>0)
    rpoolCopyToExternal(vt->sPool, string, node->string,0, (int)stringLength);

    meiExit(vt->lock);
    return lengthInBits;
}


/************************************************************************
 * pvtSet
 * purpose: Modifies values in a node
 * input  : valH    - PVT handle to use
 *          nodeId  - The ID of the node to modify
 *          fieldId - The new field ID of the node.
 *                    A negative value means no change in fieldId.
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 *                    The string is allocated and stored in the PVT.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtSet(
    IN  HPVT        hVal,
    IN  int         nodeId,
    IN  INTPTR      fieldId,
    IN  INT32       value,
    IN  const char* string)
{
    int result;
    vtStruct* vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result = pvtSetSyn(hVal, (HPST)-1, nodeId, fieldId, value, string);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtGetChild
 * purpose: Returns the ID of a child node based on its field ID
 * input  : valH        - PVT handle to use
 *          parentId    - The Node ID of the immediate parent
 *          fieldId     - The field ID of the node that the function is
 *                        expected to locate
 * output : childNodeId - The ID of the node that is found, or negative value on failure
 *                        If set to NULL, it will be discarded
 * return : Child's node id on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetChild(
    IN  HPVT    hVal,
    IN  int     parentId,
    IN  INTPTR  fieldId,
    OUT int*    childNodeId)
{
    vtStruct *vt = (vtStruct *)hVal;
    int path;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    if ((path = rtGetChild(vt->vTree, parentId, (void *)fieldId, 1)) <0)
    {
        if (childNodeId) *childNodeId = RVERROR;
    }
    else
    {
        if (childNodeId) *childNodeId = path;
    }

    meiExit(vt->lock);
    return path;

}


/************************************************************************
 * pvtGetChildByFieldId
 * purpose: Returns the child's value based on its field ID
 * input  : valH        - PVT handle to use
 *          parentId    - The Node ID of the immediate parent
 *          fieldId     - The field ID of the node that the function is
 *                        expected to locate
 * output : value       - Value of the root node, or length of the value if
 *                        value is a string
 *          isString    - TRUE if node contains a string - see pvtGetString()
 * return : Child's node id on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetChildByFieldId(
    IN  HPVT    hVal,
    IN  int     nodeId,
    IN  INTPTR  fieldId,
    OUT INT32*  value,
    OUT BOOL*   isString)
{
    int result;
    vtStruct* vt = (vtStruct *)hVal;
    if (!vt) return RVERROR;

    meiEnter(vt->lock);
    result =
        pvtGet(hVal, pvtGetChild(hVal, nodeId, fieldId, NULL), NULL, NULL, value, isString);
    meiExit(vt->lock);

    return result;
}


/************************************************************************
 * pvtGetByIndex
 * purpose: Returns the ID of a child node based on the child's index
 * input  : valH        - PVT handle to use
 *          parentId    - The Node ID of the immediate parent
 *          index       - The index of the child, as determined by pvtAdd().
 *                        This index is 1-based.
 * output : childNodeId - The ID of the node that is found, or negative value on failure
 *                        If set to NULL, it will be discarded
 * return : Child's node id on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetByIndex(
    IN  HPVT    hVal,
    IN  int     parentId,
    IN  INT32   index,
    OUT int*    childNodeId)
{
    vtStruct *vt = (vtStruct *)hVal;
    int path;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    if ( (path = rtGetByIndex(vt->vTree, parentId, (int)index)) >= 0)
    {
        if (childNodeId) *childNodeId = path;
    }

    meiExit(vt->lock);
    return path;

}


/************************************************************************
 * pvtGetSyntaxIndex
 * purpose: Gets the index of the node in the parent syntax structure.
 * input  : valH    - PVT handle to use
 *          nodeId  - The PVT Node ID
 * output : none
 * return : The index of the specified node in the parent syntax
 *          structure on success (this is 1-based)
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetSyntaxIndex(
    IN  HPVT    hVal,
    IN  int     nodeId)
{
    int result;
    int parentSynNodeId=-1;
    INTPTR fieldEnum=-1;
    vtStruct* vt = (vtStruct *)hVal;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    pvtGet(hVal, pvtParent(hVal, nodeId), NULL, &parentSynNodeId, NULL, NULL);
    pvtGet(hVal, nodeId, &fieldEnum, NULL, NULL, NULL);
    result = pstGetField(pvtGetSynTree(hVal, nodeId), parentSynNodeId, (int)fieldEnum, NULL);

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtSearchPath
 * purpose: Searches for a specified path, value or both in a specified tree
 * input  : destH       - PVT handle of the search tree
 *          rootNodeId  - Root ID of the search tree
 *          srcH        - PVT handle of path to be searched in
 *          srcNodeId   - Node ID of the beginning of the path to be searched in
 *          checkLeaves - If TRUE, Compare the values in the lowest level nodes (leaves)
 *                        Use FALSE if the tree syntax contains choices.
 * output : none
 * return : TRUE if found, FALSE if not
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtSearchPath(
    IN  HPVT    destH,
    IN  int     rootNodeId,
    IN  HPVT    srcH,
    IN  int     srcNodeId,
    IN  BOOL    checkLeaves)
{
    vtStruct *vtDest =(vtStruct *)destH;
    vtStruct *vtSrc =(vtStruct *)srcH;
    HMEI    lock1, lock2;
    int     result;

    if (!destH || !srcH) return RVERROR;

    /* Make sure we lock an an orderly fashion to diminish dead-locking possibilities */
    if (destH > srcH)
    {
        lock1 = vtSrc->lock;
        lock2 = vtDest->lock;
    }
    else
    {
        lock1 = vtDest->lock;
        lock2 = vtSrc->lock;
    }

    meiEnter(lock1);
    meiEnter(lock2);
    result = vtSearchPath(vtDest, rootNodeId, vtSrc, srcNodeId, checkLeaves);
    meiExit(lock2);
    meiExit(lock1);

    return result;
}



/*---------------------- by path operations ------------------------ */


/************************************************************************
 * pvtGetNodeIdByPath
 * purpose: Returns the ID of a node based on a path that starts from a
 *          specified root
 * input  : valH                - Value Tree handle
 *          searchRootNodeId    - The ID of the node from which the search starts from
 *          path                - The path to the searched node. format: "a.b.c"
 * output : none
 * return : Node ID we've reached on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetNodeIdByPath(
    IN  HPVT        hVal,
    IN  int         nodeId,
    IN  const char* path)
{
    vtStruct *vt = (vtStruct *)hVal;
    char name[256];
    const char *ptr;
    char *nameptr;
    INTPTR fieldEnum;
    HPST hSyn;

    if (!hVal || nodeId<0  || !path) return RVERROR;
    meiEnter(vt->lock);
    hSyn = pvtGetSynTree(hVal, nodeId);

    ptr = path;

    do
    {
        nameptr = name;
        while (*path)
        {
            if (*path == '.')
            {
                path++;
                break;
            }
            *nameptr++ = *path++;
        }

        if (nameptr != name)
        {
            *nameptr = '\0';

            if (isNumber(name))
            {   /* sequence of index */
                nodeId = rtGetByIndex(vt->vTree, nodeId, atoi(name));
            }
            else  {  /* other */

                /* -- '*': semantic for 'next child ' */
                if (name[0] == '*'  &&  name[1] == '\0')
                {  /* go to child */
                    nodeId = rtGetByIndex(vt->vTree, nodeId, 1);
                }
                else
                {
                    if ((fieldEnum=pstGetFieldId(hSyn, name)) == RVERROR)
                    {
                        nodeId = RVERROR;
                        break;
                    }
                    nodeId = rtGetChild(vt->vTree, nodeId, (void *)fieldEnum, 1);
                }
            }

            /* RVERROR? */
            if (nodeId < 0) break;
        }
    } while (*path);

    meiExit(vt->lock);
    return nodeId;
}


/************************************************************************
 * pvtSetByPath
 * purpose: Modifies the value stored in a node. The function first locates
 *          the node based on a path. The function will fail if "path" doesn't exist
 * input  : valH    - Value Tree handle
 *          rootId  - The ID of the node from which the search starts from
 *          path    - The path to the searched node. format: "a.b.c"
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 *                    The string is allocated and stored in the PVT.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtSetByPath(
    IN  HPVT        hVal,
    IN  int         nodeId,
    IN  const char* path,
    IN  INT32       value,
    IN  const char* string)
{
    vtStruct *vt = (vtStruct *)hVal;
    int vNodeId=nodeId;
    int result;

    if (!vt) return RVERROR;
    meiEnter(vt->lock);

    if ((vNodeId = pvtGetNodeIdByPath(hVal, nodeId, path)) >= 0)
        result = pvtSet(hVal, vNodeId, -1, value, string);
    else
        result = RVERROR;

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtBuildByPath
 * purpose: Modifies the value stored in a node. The function first builds
 *          the path to the node, if the path does not exist.
 * input  : valH    - Value Tree handle
 *          rootId  - The ID of the node from which the search starts from
 *          path    - The path to the searched node. format: "a.b.c"
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 *                    The string is allocated and stored in the PVT.
 * output : none
 * return : Last built node ID on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtBuildByPath(
    IN  HPVT        hVal,
    IN  int         rootNodeId,
    IN  const char* path,
    IN  INT32       value,
    IN  const char* data)
{
    vtStruct* vt = (vtStruct *)hVal;
    char name[300];
    char *nameptr;
    int vtNodeId = rootNodeId;
    int vtChildId =0;
    INTPTR fieldId=-800;
    int index;
    int result = 0;
    HPST hSyn;

    if (!hVal || !path || rootNodeId<0) return RVERROR;
    meiEnter(vt->lock);

    hSyn = pvtGetSynTree(hVal, rootNodeId);

    /* -- generate path */
    do
    {
        nameptr = name;
        while (*path)
        {
            if (*path == '.')
            {
                path++;
                break;
            }
            *nameptr++ = *path++;
        }

        if (nameptr != name)
        {
            *nameptr = '\0';

            /* -- sequence of */
            if (isNumber(name))
            {
                index = atoi(name);
                if (index < 0 || index > pvtNumChilds(hVal, vtNodeId)+1)
                    continue;

                if (pvtGetByIndex(hVal, vtNodeId, index, &vtChildId) < 0)
                {
                    /* does not exist */
                    fieldId=-800;
                    if (pvtAddSyn(hVal, hSyn, vtNodeId, fieldId, -555, NULL, &vtChildId, NULL) <0) /* create node */
                    {
                        result = RVERROR;
                        break;
                    }
                }
            }
            else {
                /* -- other */
                if (name[0] == '*'  &&  name[1] == '\0')
                {
                    /* go to child */
                    if ((vtChildId = pvtChild(hVal, vtNodeId)) <0)
                    {
                        result = RVERROR;
                        break;
                    }
                }
                else
                {
                    if ((fieldId = pstGetFieldId(hSyn, name)) < 0)
                    {
                        result = RVERROR;
                        break;
                    }
                    if ((vtChildId=pvtGetChild(hVal, vtNodeId, fieldId, NULL)) <0)
                    {
                        /* does not exist */
                        if (pvtAddSyn(hVal, hSyn, vtNodeId, fieldId, -555, NULL, &vtChildId, NULL) <0) /* create node */
                        {
                            result = RVERROR;
                            break;
                        }
                    }
                }
            }

            vtNodeId = vtChildId;
        }
    } while (*path);

    if (result >= 0)
    {
        if (pvtSetSyn(hVal, hSyn, vtNodeId, fieldId, value, data) >= 0)
           result = vtNodeId;
        else
           result = RVERROR;
    }

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtGetChildTagByPath
 * purpose: Gets the tag value of the node specified in the path format
 * input  : valH    - Value Tree handle
 *          nodeId  - The ID of the node
 *          path    - The path of the child node from the node specified in
 *                    the nodeId parameter
 *          relation- The number of levels down the tree from the node specified
 *                    in the nodeId parameter
 * output : none
 * return : The tag value of the specified node on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetChildTagByPath(
    IN  HPVT        hVal,
    IN  int         nodeId,
    IN  const char* path,
    IN  int         relation)
{
    vtStruct* vt = (vtStruct *)hVal;
    HPST hSyn;
    INTPTR fieldId=-1;
    int sNodeId=-1;
    int i, vParentNodeId;
    pstChild child;
    int result;

    if (vt == NULL) return RVERROR;
    meiEnter(vt->lock);

    if ((nodeId=pvtGetNodeIdByPath(hVal, nodeId, path)) <0)
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    vParentNodeId = nodeId; /* <- to avoid warning */

    for (i = 0; i < relation; i++)
    {
        vParentNodeId = nodeId;
        if ( (nodeId = pvtChild(hVal, vParentNodeId)) < 0)
        {
            meiExit(vt->lock);
             return RVERROR;
        }
    }

    pvtGet(hVal, vParentNodeId, NULL, &sNodeId, NULL, NULL);
    pvtGet(hVal, nodeId, &fieldId, NULL, NULL, NULL);

    hSyn = pvtGetSynTree(hVal, nodeId);

    if (pstGetField(hSyn, sNodeId, fieldId, &child) <0)
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    result = pstGetTag(hSyn, child.nodeId, NULL);

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtGetByPath
 * purpose: This function returns the value in a node of the Value Tree.
 *          As input to the function, you need to provide the starting point
 *          Node ID and the path to the node.
 * input  : valH    - Value Tree handle
 *          nodeId  - The ID of the node
 *          path    - The path of the child node from the node specified in
 *                    the nodeId parameter
 * output : fieldId     - The returned field ID of the node. You can then
 *                        use the pstGetFieldName() function to obtain the field name
 *          value       - Value of the root node, or length of the value if
 *                        value is a string
 *          isString    - TRUE if node contains a string - see pvtGetString()
 * return : Node ID we've reached on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtGetByPath(
    IN  HPVT        hVal,
    IN  int         nodeId,
    IN  const char* path,
    OUT INTPTR*     fieldId,
    OUT INT32*      value,
    OUT BOOL*       isString)
{
    vtStruct* vt = (vtStruct *)hVal;
    int result;

    if (vt == NULL) return RVERROR;
    meiEnter(vt->lock);

    if ((nodeId=pvtGetNodeIdByPath(hVal, nodeId, path)) >= 0)
        result = pvtGet(hVal, nodeId, fieldId, NULL, value, isString);
    else
        result = RVERROR;

    meiExit(vt->lock);
    return result;
}


/**************************************************************************************
 * pvtGetNodeIdByFieldIds
 *
 * Purpose: find node id by traversing both value and syntax trees according to a path
 *          of filed Ids.
 *              - Field Ids in the path should be existant Field Ids.
 *              - Child indexes should be negetive values starting from -101.
 *              - for '*' -1 is used
 *              - to add element to SEQUENCE OF -100 may be used or the last element index + 1
 *              - The list should end with LAST_TOKEN value
 *
 * Input:   val H       - vtStruct of val tree
 *          nodeId     - The start of the val sub tree.
 *          path        - An array of fieldIds, ending with LAST_TOKEN
 *
 *
 * Return value: The found node id or RVERROR
 **************************************************************************************/
RVAPI int RVCALLCONV
pvtGetNodeIdByFieldIds(
    IN  HPVT            hVal,
    IN  int             nodeId,
    IN  const INT16*    path)
{
    vtStruct *vt = (vtStruct *)hVal;
    INTPTR fieldEnum;
    HPST   hSyn;
    int  index;
    INTPTR token;

    if (!vt || nodeId<0 || !path) return RVERROR;
    meiEnter(vt->lock);

    hSyn = pvtGetSynTree(hVal, nodeId);

    /* Go over each field id in the path */
    for ( token = 0; (path[token] != LAST_TOKEN) && (token < MAX_TOKEN); token++)
    {

          /* a number less than -100 indicates a SEQUENCE OF element's index */
        if (path[token] <= -100)
        {
            /* translate it to index (-101 -> 1; -102 -> 2 etc.) */
            index = -(path[token] + 100);
            /* Get the node id of the given element */
            if ((nodeId=pvtGetByIndex(hVal, nodeId, index, NULL)) <0)
            {
                meiExit(vt->lock);
                return RVERROR;
            }
        }
        else
        {  /* other (not SEQUENCE OF) */

            /* -1 ('*') is a semantic symbol for 'next child ' */
            if (path[token] == -1)
            {
                /* get the first child's node id */
                if ((nodeId=pvtChild(hVal, nodeId)) <0)
                {
                    meiExit(vt->lock);
                    return RVERROR;
                }
            }
            else
            {
                /* Search for the child with that id amongst the child elements */
                fieldEnum=path[token];
                if ((nodeId=pvtGetChild(hVal, nodeId, fieldEnum, NULL)) <0)
                {
                    meiExit(vt->lock);
                    return RVERROR;
                }
            }
        }
    }

    /* sanity check */
    meiExit(vt->lock);
    if (token >= MAX_TOKEN) return RVERROR;

    return nodeId;
}


/**************************************************************************************
 * pvtGetByFieldIds
 *
 * Purpose: Get the data of a node pointed by a given path of field ids
 *
 * Input:   hVal        - vtStruct of val tree
 *          nodeId      - The root of the value sub tree
 *          path        - An array of fieldIds, ending with LAST_TOKEN
 *
 * Output:  fieldId     - field id of the element
 *          value       - The value of the element (or string length if any)
 *          isSstring   - whether value string or not
 *
 * Return value: RVERROR or node id
 **************************************************************************************/
RVAPI int RVCALLCONV
pvtGetByFieldIds(
    IN  HPVT    hVal,
    IN  int     nodeId,
    IN  INT16*  path,
    OUT INTPTR* fieldId,
    OUT INT32*  value,
    OUT BOOL*   isString)
{
    vtStruct* vt = (vtStruct *)hVal;
    int result;

    if (vt == NULL) return RVERROR;

    meiEnter(vt->lock);

    /* Get the node id by the given path */
    if ((nodeId = pvtGetNodeIdByFieldIds(hVal, nodeId, path)) >= 0)
    {
        /* Get the data of the node by its node id */
        result = pvtGet(hVal, nodeId, fieldId, NULL, value, isString);
    }
    else
        result = RVERROR;

    meiExit(vt->lock);
    return result;
}


/************************************************************************
 * pvtBuildByFieldIds
 * purpose: Modifies the value stored in a node. The function first builds
 *          the path to the node, if the path does not exist.
 *          This function works faster than pvtBuildByPath
 * input  : valH    - Value Tree handle
 *          rootId  - The ID of the node from which the search starts from
 *          path    - The path to the searched node. format: {a,b,c,LAST_TOKEN}
 *          value   - Value of the root node, or length of the value if
 *                    value is a string
 *          string  - String value of the node, or NULL if not a string
 *                    The string is allocated and stored in the PVT.
 * output : none
 * return : Last built node ID on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtBuildByFieldIds(
    IN  HPVT    hVal,
    IN  int     rootNodeId,
    IN  INT16*  path,
    IN  INT32   value,
    IN  char*   data)
{
    vtStruct* vt = (vtStruct *)hVal;
    INTPTR token;
    int vtNodeId = rootNodeId;
    int vtChildId =0;
    INTPTR fieldId=-800;
    int index;
    HPST hSyn;

    if (!hVal || !path || rootNodeId<0) return RVERROR;
    meiEnter(vt->lock);
    hSyn = pvtGetSynTree(hVal, rootNodeId);

    /* Go over each field id in the path */
    for ( token = 0; (path[token] != LAST_TOKEN) && (token < MAX_TOKEN); token++) {

    /* if the number is negetive less than 100, that is an element position in a SEQUENCE OF field */
    if (path[token] <= -100)  {
      /* translate it to index (-101 -> 1; -102 -> 2 etc.) */
      index = -(path[token] + 100);
      if (index <0 || index > pvtNumChilds(hVal, vtNodeId)+1) continue;

      /* Get the given element's node id within the val tree */
      if ((vtChildId=pvtGetByIndex(hVal, vtNodeId, index,NULL)) <0)  {
        /* element doesn't exists, create it */
        fieldId=-800;
        if (pvtAddSyn(hVal, hSyn, vtNodeId, fieldId, -555, NULL, &vtChildId, NULL) <0)
        {
            meiExit(vt->lock);
            return RVERROR;
        }
      }
    }
    else {
        /* Other (not SEQUENCE OF) */

      /* if first child is requested (the -1 is equivalent to * sign) get its node id */
      if (path[token] == -1)  {
        fieldId = -800;
        if ((vtChildId=pvtChild(hVal,vtNodeId)) <0)
        {
            meiExit(vt->lock);
            return RVERROR;
        }
      }
      else {
        /* a specific field Id was given */
        fieldId = path[token];

        /* Get the given field's node id within the val tree */
        if ((vtChildId=pvtGetChild(hVal, vtNodeId, fieldId, NULL)) <0)
        {
            /* field doesn't exists, create it */
            if (pvtAddSyn(hVal, hSyn, vtNodeId, fieldId, -555, NULL, &vtChildId, NULL) <0)
            {
                meiExit(vt->lock);
                return RVERROR;
            }
        }
      }
    }

    /* we have the node id (new or old) */
    vtNodeId = vtChildId;
    }

    /* Sanity check */
    if (token >= MAX_TOKEN)
    {
        meiExit(vt->lock);
        return RVERROR;
    }

    /* set the given values to the node */
    if (pvtSetSyn(hVal, hSyn, vtNodeId, fieldId, value, data) <0)
        vtNodeId = RVERROR;

    meiExit(vt->lock);
    return vtNodeId;
}





/************************************************************************
 * pvtPrint
 * purpose: Print a tree from a given node, using a specified printing
 *          function
 * input  : valH        - Value Tree handle
 *          parentId    - node ID to start printing from
 *          pFunc       - The function called for the printing itself
 *          pFuncParam  - The context to pass to the print function
 *                        on each printed line
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV pvtPrint(
    IN  HPVT            valH,
    IN  int             parentId,
    IN  pvtPrintFuncP   pFunc,
    IN  int             pFuncParam)
{
#ifndef NOLOGSUPPORT
    return vtPrintFunc(valH, parentId, pFunc, pFuncParam, pvtPrintDegreeAll);
#else
    if (valH || parentId || pFunc || pFuncParam);
    return 0;
#endif
}


/************************************************************************
 * pvtPrintStd
 * purpose: Print a tree from a given node
 * input  : valH        - Value Tree handle
 *          parentId    - node ID to start printing from
 *          logHandle   - Log source to print to
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV pvtPrintStd(
    IN  HPVT    valH,
    IN  int     parentId,
    IN  int     logHandle)
{
#ifndef NOLOGSUPPORT
    int             level;
    pvtPrintDegree  degree;
    vtStruct*       vt = (vtStruct *)valH;
    if (vt == NULL) return RVERROR;

    /* See if we can print a message to this log or not */
    level = logGetDebugLevel((RVHLOG)logHandle);
    if ((level < 2) || !logIsSelected((RVHLOG)logHandle, RV_DEBUG))
        return 0;

    if (logHandle == -1)
    {
        /* No specific log - use UNREG... */
        logHandle = (int)vt->unregLog;
    }


    if (logGetDebugLevel(vt->log) >= 3)
        degree = pvtPrintDegreeAll;
    else
        degree = (pvtPrintDegree)(pvtPrintDegreeSyntax | pvtPrintDegreeNodeId);

    return vtPrintFunc(valH, parentId, vtPrintToLog, logHandle, degree);
#else
    if (valH || parentId || logHandle);
    return 0;
#endif
}


#ifndef NOLOGSUPPORT

/************************************************************************
 * pvtPrintRootNodes
 * purpose: Prints all the root nodes currently allocated inside the
 *          PVT. It is only here for debugging purposes and should not
 *          be used in any other type of implementation.
 * input  : valH            - Value Tree handle
 *          printOnlyRoot   - If TRUE, only the root nodes are printed
 *                            If FALSE, the whole tree nodes are printed
 *          msa             - Log source to print to
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI void RVCALLCONV pvtPrintRootNodes(IN HPVT hVal, IN BOOL printOnlyRoot, IN int msa)
{
    vtStruct* vt = (vtStruct *)hVal;

    if ((vt == NULL) || (!logIsSelected((RVHLOG)msa, RV_DEBUG))) return;

    meiEnter(vt->lock);
    vt->pFuncParam = msa;
    vt->printOnlyRoots = printOnlyRoot;
    vt->degree = pvtPrintDegreeAll;

    logPrintFormat((RVHLOG)msa, RV_DEBUG, "PVT Nodes (0x%x):", hVal);
    rtDoAll(vt->vTree, vtRootPrint, vt);
    meiExit(vt->lock);
}


#endif  /* NOLOGSUPPORT */





#ifdef __cplusplus
}
#endif



