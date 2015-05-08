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
  pvaltree.h

  Tree manipulation.
  Holds message values according to syntax structure.
  Support multiple messages (trees) within a single instance.

  Ron S.
  27 Oct. 1996

  Global behavior:
  All functions return RVERROR upon illegal paramenters or any other RVERROR situation.
  Some functions return TRUE on normal conditions.
  OUT pointer parameters may be NULL if there is no need for them.

  Handlers:
   - HPVT: value tree instance handler.
   - nodeId: single message (tree) identifier.

  Function parameters:
   - parentId: Id of parent node. Unique node identifier.
   - fieldId: enumeration of field as in the message.
   - value: integer value of node.
   - index: of child under parent. >=1
   - path: string description by dotted notation of node location in tree.
          e.g. root.colors.white
  */


#ifndef _PVALTREE_
#define _PVALTREE_


#include <rvcommon.h>
#include <psyntree.h>


/* handler */
DECLARE_OPAQUE(HPVT);

/* print function prototype */
typedef void (*pvtPrintFuncP)(IN int type, IN const char*line, ...);





/************************************************************************
 *
 *                      Field ID macros
 *
 ************************************************************************/

/* Macros for nice access to paths by field ids */
#ifndef LAST_TOKEN
#define LAST_TOKEN (-5) /* Last token in fieldPath parameters */
#endif

#ifndef MAX_TOKEN
#define MAX_TOKEN  100 /* maximal elements in a path */
#endif

/* Macros used to specify any value of the given choice - they are the equivalent of '*' in
   their string functions */
#define _anyField (-1),
#define __anyField (-1)

/* Used to specify the index of the value inside a SEQUENCE OF */
#define _nul(a) -(a)-100,
#define __nul(a) -(a)-100


/* Macros that allow easier access to the fields through the pvtXXXByFieldIds */
#define __pvtGetByFieldIds(res, valH, nodeId, path, fieldId, value, isString) \
    { static INT16 fieldPath[] = path; \
      res = pvtGetByFieldIds(valH, nodeId, fieldPath, fieldId, value, isString); \
    }
#define __pvtGetNodeIdByFieldIds(res, valH, nodeId, path) \
    { static INT16 fieldPath[] = path; \
      res = pvtGetNodeIdByFieldIds(valH, nodeId, fieldPath); \
    }
#define __pvtBuildByFieldIds(res, valH, rootNodeId, path, value, data) \
    { static INT16 fieldPath[] = path; \
      res = pvtBuildByFieldIds(valH, rootNodeId, fieldPath, value, data); \
    }






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
        IN  int numOfNodes);


/************************************************************************
 * pvtDestruct
 * purpose: Destruct a pool of PVT nodes.
 * input  : valH    - The PVT handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDestruct(IN  HPVT valH);



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
       IN  HPVT valH,
       IN  int nodeId);


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
          IN  HPVT valH,
          IN  int nodeId);


/************************************************************************
 * pvtCurSize
 * purpose: Gets the number of nodes currently used in the Value Tree forest
 * input  : valH    - The PVT handle
 * output : none
 * return : The number of nodes in the Value Tree forest on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtCurSize(IN HPVT valH);


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
pvtMaxUsage(IN HPVT valH);


/************************************************************************
 * pvtMaxSize
 * purpose: Gets the highest number of nodes that cab be used in the Value Tree forest
 * input  : valH    - The PVT handle
 * output : none
 * return : The maximum number of nodes in the Value Tree forest on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtMaxSize(IN HPVT valH);



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
          IN  HPVT   valH,
          OUT INT32* poolSize,
          OUT INT32* availableSpace,
          OUT INT32* maxFreeChunk,
          OUT INT32* numOfChunks);


/************************************************************************
 * pvtTreeSize
 * purpose: Returns the number of nodes included in a Value Tree root
 * input  : valH        - The PVT handle
 *          parentId    - The ID of any node
 * output : none
 * return : The number of nodes included in a Value Tree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtTreeSize(
        IN  HPVT    valH,
        IN  int     parentId);


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
         IN  HPVT   valH,
         IN  int    parentId);


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
      IN  HPVT  valH,
      IN  int   nodeId);


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
       IN  HPVT valH,
       IN  int  nodeId);


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
       IN  HPVT valH,
       IN  int  nodeId);


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
     IN  HPVT   valH,
     IN  int    parentId);


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
    IN  HPVT    valH,
    IN  int     rootId,
    IN  int     location);



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
    IN  const char* string);


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
    IN  char*   string);


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
    IN  HPVT        valH,
    IN  int         parentId,
    IN  INTPTR      fieldId,
    IN  INT32       value,
    IN  const char* string,
    OUT int*        index);


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
    IN  int     srcNodeId);


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
    IN  int     srcNodeId);


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
      IN  HPVT  valH,
      IN  int   subTreeRootId);


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
        IN  HPVT    valH,
        IN  int     subTreeRootId);


/************************************************************************
 * pvtDeleteAll
 * purpose: Deletes all the nodes in a Value Tree.
 * input  : valH            - PVT handle to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
pvtDeleteAll(IN  HPVT valH);


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
    IN  int     srcNodeId);


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
    IN  int     srcRootId);


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
    IN  int     srcRootId);


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
    IN  HPVT    valH,
    IN  int     adoptedChildId,
    IN  int     newParentId,
    IN  int     newBrotherId);


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
    IN  HPVT    valH,
    IN  int     nodeId,
    OUT INTPTR* fieldId,
    OUT int*    synNodeId,
    OUT INT32*  value,
    OUT BOOL*   isString);


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
    IN  HPVT    valH,
    IN  int     nodeId,
    IN  INT32   stringLength,
    OUT char*   string);


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
    IN  HPVT    valH,
    IN  int     nodeId,
    IN  INT32   stringLength,
    OUT char*   string);


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
    IN  HPVT        valH,
    IN  int         nodeId,
    IN  INTPTR      fieldId,
    IN  INT32       value,
    IN  const char* string);


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
    IN  HPVT    valH,
    IN  int     parentId,
    IN  INTPTR  fieldId,
    OUT int*    childNodeId);


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
    IN  HPVT    valH,
    IN  int     nodeId,
    IN  INTPTR  fieldId,
    OUT INT32*  value,
    OUT BOOL*   isString);


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
    IN  HPVT    valH,
    IN  int     parentId,
    IN  INT32   index,
    OUT int*    childNodeId);


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
    IN  HPVT    valH,
    IN  int     nodeId);


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
    IN  BOOL    checkLeaves);





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
    IN  int             pFuncParam);


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
    IN  int     logHandle);






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
    IN  HPVT        valH,
    IN  int         searchRootNodeId,
    IN  const char* path);


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
    IN  HPVT        valH,
    IN  int         rootId,
    IN  const char* path,
    IN  INT32       value,
    IN  const char* string);


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
    IN  HPVT        valH,
    IN  int         rootId,
    IN  const char* path,
    IN  INT32       value,
    IN  const char* string);


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
    IN  HPVT        valH,
    IN  int         nodeId,
    IN  const char* path,
    IN  int         relation);


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
    IN  HPVT        valH,
    IN  int         nodeId,
    IN  const char* path,
    OUT INTPTR*     fieldId,
    OUT INT32*      value,
    OUT BOOL*       isString);


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
 *          nodeId      - The start of the val sub tree.
 *          path        - An array of fieldIds, ending with LAST_TOKEN
 *                        from search root. format: {a,b,c,LAST_TOKEN}
 *
 *
 * Return value: The found node id or RVERROR
 **************************************************************************************/
RVAPI int RVCALLCONV
pvtGetNodeIdByFieldIds(
    IN  HPVT            valH,
    IN  int             searchRootNodeId,
    IN  const INT16*    path);


/**************************************************************************************
 * pvtGetByFieldIds
 *
 * Purpose: Get the data of a node pointed by a given path of field ids
 *
 * Input:   valH        - vtStruct of val tree
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
    IN  HPVT    valH,
    IN  int     nodeId,
    IN  INT16*  path,
    OUT INTPTR* fieldId,
    OUT INT32*  value,
    OUT BOOL*   isString);


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
    IN  HPVT    valH,
    IN  int     rootId,
    IN  INT16*  path,
    IN  INT32   value,
    IN  char*   string);


/* todo: What about these? */
#define pvtGetChild2(hVal,nodeId,fieldId1,fieldId2) \
    pvtGetChild((hVal),(pvtGetChild((hVal),(nodeId),(fieldId1),NULL)),(fieldId2),NULL)

#define pvtGetChildValue pvtGetChildByFieldId
#define pvtGetChildValue2(hVal,nodeId,fieldId1,fieldId2,value,svalue) \
    pvtGet((hVal),pvtGetChild2((hVal),(nodeId),(fieldId1),(fieldId2)),NULL,NULL,(value),(svalue))

#define pvtAddBranch(hVal,nodeId,fieldId) \
    pvtAdd((hVal),(nodeId),(fieldId),0,NULL,NULL)
#define pvtAddBranch2(hVal,nodeId,fieldId1,fieldId2) \
    pvtAddBranch((hVal),(pvtAddBranch((hVal),(nodeId),(fieldId1))),(fieldId2))




/************************************************************************
 * This part is here for debugging purposes only.
 * The function declarations here are not part of the API and might not
 * be supported in future versions.
 * These functions should not be called directly.
 ************************************************************************/
#ifndef NOLOGSUPPORT
#define pvtAddRoot(hVal,hSyn,value,string)                  pvtAddRootFromInt(hVal,hSyn,-1,value,(char *)string,__FILE__,__LINE__)
#define pvtAddRootByPath(hVal,hSyn,syntaxPath,value,string) pvtAddRootFromInt(hVal,hSyn,pstGetNodeIdByPath(hSyn,(const char*)syntaxPath),value,string,__FILE__,__LINE__)
RVAPI int RVCALLCONV pvtAddRootFromInt(IN HPVT hVal, IN HPST hSyn, IN int nodePath, IN INT32 value, IN char* string, IN const char* fileName, IN int lineno);
RVAPI void RVCALLCONV pvtPrintRootNodes(IN HPVT hVal, IN BOOL printOnlyRoot, IN int msa);
#endif


#endif
#ifdef __cplusplus
}
#endif



