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

#ifndef _PSYNTREE_STACKAPI_H
#define _PSYNTREE_STACKAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <psyntree.h>


/************************************************************************
 *
 *                          Public structures
 *
 ************************************************************************/


typedef enum {
    pstNotSpecial=0,        /* 000 */
    pstDepending=3,         /* 011 */
    pstDependent=5,         /* 101 */
    pstDependingDependent=7 /* 111 */
} pstFieldSpeciality;


typedef enum {
    pstValueInteger = 0,
    pstValueString  = 1,
    pstValueSubTree = 2
} pstValueType;


/************************************************************************
 * pstChildExt
 * Extended information about a child node inside the syntax tree
 * index            - Index of the child, 1-based
 *                    Negative value if invalid
 * nodeId           - Child's node ID
 * fieldId          - Child's field ID
 * isOptional       - TRUE if the child is an OPTIONAL field in a SEQUENCE
 * isDefault        - TRUE if the child is a DEFAULT field in a SEQUENCE
 * isExtended       - TRUE if the child is an extended field (after ...)
 * speciality       - Dependency between fields
 * enumerationValue - Value if it's an ENUMERATED value field type
 ************************************************************************/
typedef struct
{
    int                 index;
    int                 nodeId;
    INT32               fieldId;
    BOOL                isOptional;
    BOOL                isDefault;
    BOOL                isExtended;
    pstFieldSpeciality  speciality;
    int                 enumerationValue;
} pstChildExt;


/* --- value tree root --- */
typedef struct {
    int             typeReference;
    int             value;/* value of typeReference (integer or string). */
    pstValueType    isString; /* TRUE if value is string */
} pstValueTreeStruct;


/* --- value node --- */
typedef struct {
    int             fieldName;
    int             value; /* value of field (integer or string). */
    pstValueType    isString; /* TRUE if value is string */
} pstValueNodeStruct;


typedef struct
{
    INT32   objectSetId;  /* object set nodeid  -  MUST be referenced */
    INT32   fieldInClassNumber; /* field in class number */
} pstTypeFromConstraint;


typedef struct
{
    INT32   parentDepth;
    INT32   fieldId;
    INT32   relativeIndex;
} pstConstrainingField;


typedef struct
{
    INT32   fieldInClassNumber;
    INT32   settingId;
} pstFieldOfObjectReference;




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
    OUT pstChildExt*    child);


int
pstGetField(
        IN  HPST      synH,
        IN  int       nodeId,      /* node id of parent */
        IN  int       fieldId,  /* index of child */
        OUT pstChild *child);

int
pstGetFieldExt(
            IN  HPST      synH,
            IN  int       nodeId,      /* node id of parent */
            IN  int       fieldId,  /* index of child */
            OUT pstChildExt *child
           );

int
pstChildIsExtended(
      IN  HPST        synH,
      IN  int         nodeId,      /* node id of parent */
      IN  int         childIndex    /* index of child */
      );

int
pstGetFirstExtendedChild(
         IN  HPST    synH,
         IN  int     nodeId);

int
pstGetNumberOfOptionalBeforeExtension(
         IN  HPST    synH,
         IN  int     nodeId);

INT32 /* Field index or RVERROR */
pstGetFieldIndex(
      /* convert field name to internal id */
      IN  HPST synH,
      IN  int synNodeId,
      IN  int fieldId
      );


char* /* null terminated field name */
pstGetFieldNamePtr(
        /* convert field internal id to field name */
        IN  HPST hSyn,
        IN  INT32 fieldId
        );


int /* type of node */
pstGetNodeRangeExt(
      IN  HPST    hSyn,
      IN  int     nodeId,
      OUT int *   from,
      OUT int *   to,
      OUT BOOL*   fromAbsent,
      OUT BOOL*   toAbsent);


int
pstChildIsSpecial(
  IN  HPST        synH,
  IN  int         nodeId,      /* node id of parent */
  IN  int         childIndex   /* index of child */
  );


int pstGetTypeFromConstraint(
            IN  HPST                    synH,
            IN  int                     specialTypeNodeId,
            OUT pstTypeFromConstraint*  specialType);

int pstGetConstrainingField(
            IN  HPST                    synH,
            IN  int                     specialTypeNodeId,
            OUT pstConstrainingField*   constrainingField);

int pstGetFieldOfObjectReference(
            IN  HPST                        synH,
            IN  int                         objectNodeId,
            IN  int                         index,
            OUT pstFieldOfObjectReference*  fieldOfObject);

int pstGetValueTreeStruct(
            IN  HPST                        synH,
            IN  int                         vtStructNodeId,
            OUT pstValueTreeStruct*         valueTreeStruct);

int pstGetValueNodeStruct(
            IN  HPST                synH,
            IN  int                 vtNodeNodeId,
            OUT pstValueNodeStruct* valueTreeNode);

/************************************************************************
 * pstCompareFunction
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
typedef int (*pstCompareFunction)(IN HPST hSyn, IN int sNodeId, IN void* context);

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
    IN void*                context);

/* TRUE if nodes have the same structure */
BOOL pstAreNodesCongruent(
            IN HPST hSyn1,
            IN int  synNodeId1,
            IN HPST hSyn2,
            IN int  synNodeId2);

/* true if node is of complex type */
/* asnChoice, asnSequence, asnSet, asnSequenceOf, asnSetOf */
BOOL pstIsNodeComplex(
            IN  HPST    hSyn,
            IN  int     nodeId);

char* /* actual length of the fromString or RVERROR */
pstGetFROMStringPtr(
         /* Get the character constraints of the syntax node */
         IN  HPST hSyn,
         IN  int nodeId,
         OUT int*actualLength);

/* Get root nodeId */
int pstGetRoot(IN HPST hSyn);


#ifdef __cplusplus
}
#endif

#endif  /* _PSYNTREE_STACKAPI_H */



