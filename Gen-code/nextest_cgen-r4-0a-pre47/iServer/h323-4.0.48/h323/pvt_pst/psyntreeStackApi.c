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
  psyntreeStackApi

  Internal Syntax-Tree functions used as API for other stack modules.

  */

#include <stdlib.h>
#include <psyntreeDef.h>
#include <psyntreeDb.h>
#include <psyntreeStackApi.h>


/************************************************************************
 *
 *                          Private constants
 *
 ************************************************************************/


/************************************************************************
 * ones array
 * This array holds the amount of '1' bits in binary representation for
 * the index value from 0 to 31. We use this to check how many optional
 * fields a type is holding. By checking the number of bits we actually
 * know the position of the optional field inside the type, since it
 * may vary.
 * The comments indicate which kind of field we're looking for in each
 * of the values.
 ************************************************************************/
unsigned char ones[32] =
{
    /* Bits: to, from, fromID, toID, tag */
    0,  /* 00000 */ /* Should never happen */

    /* tag existance */
    1,  /* 00001 */

    /* ofId for arrays */
    1,  /* 00010 */
    2,  /* 00011 */

    /* fromId for strings */
    1,  /* 00100 */
    2,  /* 00101 */
    2,  /* 00110 */
    3,  /* 00111 */

    /* from constraint */
    1,  /* 01000 */
    2,  /* 01001 */
    2,  /* 01010 */
    3,  /* 01011 */
    2,  /* 01100 */
    3,  /* 01101 */
    3,  /* 01110 */
    4,  /* 01111 */

    /* to constraint */
    1,  /* 10000 */
    2,  /* 10001 */
    2,  /* 10010 */
    3,  /* 10011 */
    2,  /* 10100 */
    3,  /* 10101 */
    3,  /* 10110 */
    4,  /* 10111 */
    2,  /* 11000 */
    3,  /* 11001 */
    3,  /* 11010 */
    4,  /* 11011 */
    3,  /* 11100 */
    4,  /* 11101 */
    4,  /* 11110 */
    5   /* 11111 */
};


/************************************************************************
 * stATSearchStruct
 * Association table search struct.
 * Used on calls to bsearch().
 * hSyn         - Syntax tree used
 * compareFunc  - Comparison function to call
 * context      - Context used
 ************************************************************************/
typedef struct
{
    HPST                hSyn;
    pstCompareFunction  compareFunc;
    void*               context;
} stATSearchStruct;






/************************************************************************
 *
 *                          Private functions
 *
 ************************************************************************/


/************************************************************************
 * pstSearch
 * purpose: Prepare parameters when trying to search an object inside
 *          an association table (=object set).
 * input  : eKey    - Key element we're searching for
 *          elem    - Element we're currently comparing with
 * output : none
 * return : Negative if the key is lower than elem in dictionary comparison
 *          0 if the key and the element are equal
 *          Positive if the key is higher than elem in dictionary comparison
 ************************************************************************/
int pstSearch(IN const void *eKey, IN const void *elem)
{
    stATSearchStruct*           key = (stATSearchStruct *)eKey;
    stAssociationTableValue*    atNode = (stAssociationTableValue *)elem;

    return key->compareFunc(key->hSyn, (int)m_ATValueValue(atNode), key->context);
}








/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


/************************************************************************
 * pstGetChildExt
 * purpose: Return extended information about one of the child nodes of
 *          a parent node
 * input  : hSyn        - Syntax tree used
 *          nodeId      - Parent's node ID
 *          childIndex  - Index of the child (1-based)
 * output : child       - Extended child node information
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int pstGetChildExt(
    IN  HPST            hSyn,
    IN  int             nodeId,
    IN  int             childIndex,
    OUT pstChildExt*    child)
{
    stChildExt* fieldInfo;
    int err;

    if (child)
        child->index = RVERROR;  /* assume the worst */

    /* Get the information */
    err = stGetChildByIndex(hSyn, nodeId, childIndex, &fieldInfo);
    if (err < 0) return err;

    /* Convert it to something readable */
    child->index            = childIndex;
    child->nodeId           = m_structId(fieldInfo);
    child->fieldId          = m_fieldId(fieldInfo);
    child->isOptional       = m_isOptional(fieldInfo);
    child->isDefault        = m_isDefault(fieldInfo);
    child->isExtended       = m_isExtended(fieldInfo);
    child->speciality       = (pstFieldSpeciality)m_isSpecial(fieldInfo);
    child->enumerationValue = m_enumerationValue(fieldInfo);

    return err;
}


int
pstGetField(
            IN  HPST      hSyn,
            IN  int       nodeId,      /* node id of parent */
            IN  int       fieldId,  /* index of child */
            OUT pstChild *child
           )
{
    int index=pstGetFieldIndex(hSyn,nodeId,fieldId);
    if (child)
        pstGetChild(hSyn, nodeId, index, child);
   return index;
}

int
pstGetFieldExt(
            IN  HPST      hSyn,
            IN  int       nodeId,      /* node id of parent */
            IN  int       fieldId,  /* index of child */
            OUT pstChildExt *child
           )
{
    int index;
    if (nodeId<0)
        return RVERROR;
    index=pstGetFieldIndex(hSyn,nodeId,fieldId);
    if (child)
        pstGetChildExt(hSyn, nodeId, index, child);
    return index;
}


int
pstChildIsExtended(
      IN  HPST        hSyn,
      IN  int         nodeId,      /* node id of parent */
      IN  int         childIndex   /* index of child */
      )
{
    stChildExt *fieldInfo;
    int err;

    err = stGetChildByIndex(hSyn, nodeId, childIndex, &fieldInfo);
    if (err < 0) return err;

    return m_isExtended(fieldInfo);
}


int
pstGetFirstExtendedChild(
         IN  HPST    hSyn,
         IN  int     nodeId)
{
    stNodeExt* node;

    node = stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return RVERROR;

    return m_childsBeforeExt(node) + 1;
}


int
pstGetNumberOfOptionalBeforeExtension(
         IN  HPST    hSyn,
         IN  int     nodeId)
{
    stNodeExt* node;

    node = stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return RVERROR;

    return m_numOptBeforeExt(node);
}


INT32 /* Field index or RVERROR */
pstGetFieldIndex(
          /* convert field name to internal id */
          IN  HPST hSyn,
          IN  int nodeId,
          IN  int fieldId
          )
{
    /* todo: make it faster by removing this function */
    stNodeExt* elementBuffer;

    if(hSyn==NULL || fieldId<0)
        return RVERROR;

    elementBuffer = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);

    return stGetChildByNodeAndFieldName(hSyn, nodeId, (int)m_numOfChilds(elementBuffer), fieldId, NULL);
}


/* convert field internal id to field name */
/* null terminated field name */
char* pstGetFieldNamePtr(
        IN  HPST hSyn,
        IN  INT32 fieldId
        )
{
    char* string = stGetNameByNameId(hSyn, fieldId, NULL);
    return (string != NULL) ? string : (char *)"";
}


int /* type of node */
pstGetNodeRangeExt(
      IN  HPST    hSyn,
      IN  int     nodeId,
      OUT int *   from,
      OUT int *   to,
      OUT BOOL*   _fromAbsent,
      OUT BOOL*   _toAbsent)
{
    stNodeExt* node;

    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return RVERROR;

    if (from)  *from = m_from(node);
    if (to)    *to   = m_to(node);

    if (fromAbsent) *_fromAbsent = m_flags(node) & fromAbsent;
    if (toAbsent) *_toAbsent = m_flags(node) & toAbsent;

    return 0;
}


int
pstChildIsSpecial(
      IN  HPST        hSyn,
      IN  int         nodeId,      /* node id of parent */
      IN  int         childIndex   /* index of child */
      )
{
    stChildExt *fieldInfo;
    int err;

    err = stGetChildByIndex(hSyn, nodeId, childIndex, &fieldInfo);
    if (err < 0) return err;

    return m_isSpecial(fieldInfo);
}




int pstGetTypeFromConstraint(
            IN  HPST                    hSyn,
            IN  int                     specialTypeNodeId,
            OUT pstTypeFromConstraint*  specialType)
{
    synStruct *syn = (synStruct *)hSyn;
    BYTE * node;

    if (!syn  ||  specialTypeNodeId < 0 || !specialType) return RVERROR;

    node = (BYTE *)stGetNodeDataByNodeId(hSyn, specialTypeNodeId);
    *specialType=*(pstTypeFromConstraint*)node;
    return  TRUE;
}


int pstGetConstrainingField(
            IN  HPST                    hSyn,
            IN  int                     specialTypeNodeId,
            OUT pstConstrainingField*   constrainingField)
{
    synStruct *syn = (synStruct *)hSyn;
    BYTE * node;

    if (!syn  ||  specialTypeNodeId < 0 || !constrainingField) return RVERROR;

    node = (BYTE *)stGetNodeDataByNodeId(hSyn, specialTypeNodeId + 3);
    *constrainingField=*(pstConstrainingField*)node;
    return  TRUE;
}


int pstGetFieldOfObjectReference(
            IN  HPST                        hSyn,
            IN  int                         objectNodeId,
            IN  int                         index,
            OUT pstFieldOfObjectReference*  fieldOfObject)
{
    int childNodeId;
    UINT32* node;

    if (!hSyn  ||  objectNodeId < 0 || !fieldOfObject) return RVERROR;

    childNodeId = stGetChildByIndex(hSyn, objectNodeId, index, &node);
    if (childNodeId < 0) return childNodeId;

    *fieldOfObject=*(pstFieldOfObjectReference*)node;
    return  TRUE;
}


int pstGetValueTreeStruct(
            IN  HPST                        hSyn,
            IN  int                         vtStructNodeId,
            OUT pstValueTreeStruct*         valueTreeStruct)
{
    stValueTree* node;

    if (!hSyn  ||  vtStructNodeId < 0 || !valueTreeStruct) return RVERROR;

    node = (stValueTree *)stGetNodeDataByNodeId(hSyn, vtStructNodeId);

    valueTreeStruct->typeReference  = m_valueTreeTypeReference(node);
    valueTreeStruct->isString       = (pstValueType)m_valueTreeIsString(node);
    valueTreeStruct->value          = m_valueTreeValue(node);

    return 0;
}


int pstGetValueNodeStruct(
            IN  HPST                hSyn,
            IN  int                 vtNodeNodeId,
            OUT pstValueNodeStruct* valueTreeNode)
{
    stValueNode* node;

    if (!hSyn  ||  vtNodeNodeId < 0 || !valueTreeNode) return RVERROR;

    node = (stValueNode *)stGetNodeDataByNodeId(hSyn, vtNodeNodeId);

    valueTreeNode->fieldName    = m_valueNodeFieldName(node);
    valueTreeNode->isString     = (pstValueType)m_valueNodeIsString(node);
    valueTreeNode->value        = m_valueNodeValue(node);

    return 0;
}


/************************************************************************
 * pstFindObjectInAT
 * purpose: Find the type of field from the association table that matches
 *          a given tree node. This is used when trying to encode/decode
 *          messages with object sets.
 * input  : hSyn        - Syntax tree used
 *          atNodeId    - Association table node ID in the syntax tree
 *          compareFunc - Comparison functio to use for nodes
 *          context     - Context to use for comparison function
 * output : none
 * return : Node ID of the matching syntax value on success
 *          Negative value on failure
 ************************************************************************/
int pstFindObjectInAT(
    IN HPST                 hSyn,
    IN int                  atNodeId,
    IN pstCompareFunction   compareFunc,
    IN void*                context)
{
    stAssociationTable*         atNode;
    int                         numObjects;
    stAssociationTableValue*    base;
    stAssociationTableValue*    node;
    stATSearchStruct            key;

    /* Find out how many items are then in this association table */
    atNode = (stAssociationTable *)stGetNodeDataByNodeId(hSyn, atNodeId);
    numObjects = m_ATNumObjects(atNode);

    /* Find the position of the first object in the association table */
    base = (stAssociationTableValue *)m_ATTable(atNode);

    /* Set the key to search for */
    key.hSyn = hSyn;
    key.compareFunc = compareFunc;
    key.context = context;

    /* Use binary search to look for the specific object id that matches */
    node = (stAssociationTableValue *)
        bsearch(&key, base, numObjects, m_ATObjectSize(atNode), pstSearch);

    /* Make sure we've got something */
    if (node == NULL) return RVERROR;

    /* Found! get the syntax tree for this object */
    return m_ATValueObject(node);
}


/* TRUE if nodes have the same structure */
BOOL pstAreNodesCongruent(
            IN HPST hSyn1,
            IN int  synNodeId1,
            IN HPST hSyn2,
            IN int  synNodeId2)
{
    synStruct *syn1 = (synStruct *)hSyn1;
    synStruct *syn2 = (synStruct *)hSyn2;
    stNodeExt *node1=NULL;
    stNodeExt *node2=NULL;
    pstChildExt child1;
    pstChildExt child2;
    unsigned i;

    if (syn1==NULL || syn2==NULL ||
       (syn1->syntax != syn2->syntax)) return FALSE;

    if (!synNodeId1) synNodeId1=pstGetRoot(hSyn1);
    if (!synNodeId2) synNodeId2=pstGetRoot(hSyn2);

    node1 = (stNodeExt *)stGetNodeDataByNodeId(hSyn1, synNodeId1);
    node2 = (stNodeExt *)stGetNodeDataByNodeId(hSyn2, synNodeId2);
    if (!node1 || !node2) return FALSE;

    if (m_isExtension(node1) != m_isExtension(node2)) return FALSE;
    if (m_numOfChilds(node1) != m_numOfChilds(node2)) return FALSE;
    if (m_from(node1) != m_from(node2)) return FALSE;
    if (m_to(node1) != m_to(node2)) return FALSE;
    if (m_ofId(node1) != m_ofId(node2)) return FALSE;
    if (m_fromId(node1) != m_fromId(node2)) return FALSE;

    /* check childs */
    for (i=1; i<=m_numOfChilds(node1); i++)
    {
        pstGetChildExt(hSyn1, synNodeId1, (int)i, &child1);
        pstGetChildExt(hSyn2, synNodeId2, (int)i, &child2);
        /*if (child1->fieldId != child2->fieldId) return FALSE;*/
        if (child1.nodeId != child2.nodeId) return FALSE;
        if (child1.isExtended != child2.isExtended) return FALSE;
        if (child1.isOptional != child2.isOptional) return FALSE;
    }

    return TRUE;
}


BOOL pstIsNodeComplex(
            IN  HPST    hSyn,
            IN  int     nodeId)
{
    pstNodeType type = pstGetNodeType(hSyn, nodeId);
    if (((int)type) < 0)
        return FALSE;

    switch (type)
    {
        case pstChoice:
        case pstSequence:
        case pstSet:
        case pstSequenceOf:
        case pstSetOf:      return TRUE;
        default:            break;
    }
    return FALSE;
}


char* /* actual length of the fromString or RVERROR */
pstGetFROMStringPtr(
         /* Get the character constraints of the syntax node */
         IN  HPST hSyn,
         IN  int nodeId,
         OUT int*actualLength)
{
    stNodeExt* node=NULL;
    char* string=NULL;

    *actualLength = 0;
    node = (stNodeExt *)stGetNodeDataByNodeId(hSyn, nodeId);
    if (node == NULL) return NULL;

    if(m_fromId(node)<0) return NULL;

    string = stGetNameByNameId(hSyn, m_fromId(node), actualLength);

    return string;
}


int pstGetRoot(IN HPST hSyn)
{
    synStruct *syn = (synStruct *)hSyn;

    if (!syn) return RVERROR;
    return syn->rootNodeId;
}

#ifdef __cplusplus
}
#endif



