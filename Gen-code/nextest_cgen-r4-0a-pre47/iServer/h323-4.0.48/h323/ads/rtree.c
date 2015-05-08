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
  rtree.c

  Ron S. 29 Jan. 1996

  Tree implementation over array.

  node structure:

                      --------
                ----> | node |
               /      --------
              /
  -------------------------------------------                  --------
  | Child | Parent |     Data     | Brother |  --------------> | node | ------> NULL
  -------------------------------------------                  --------
     |
     |
     |
  --------     --------          --------
  | node |---> | node |   . . .->| node | -> NULL
  --------     --------          --------



  Child:   points to first child or NULL.
  Parent:  points to parent or NULL (if root).
  Brother: points to next brother or NULL (if last brother).
  Data:    contains node information.


  Schema of a tree:

      NULL
       |
     -----
 1  |  1  |--> NULL
     -----
       |
     -----                                   -----
 2  |  2  |-------------------------------->|  3  |--> NULL
     -----       ^         ^                 -----
       |         |         |                   |
     -----     -----     -----               -----
 3  |  4  |-->|  5  |-->|  6  |--> NULL     |  7  |--> NULL
     -----     -----     -----               -----
                 |
               -----
 4            |  8  |--> NULL
               -----

  Node 1: Root node of the tree. Points to <Node 2> as its Child.
  Node 2: Parent is <node 1>. Borther is <node 3>.
          It has 3 children, from them, it points to <node 4> as its Child.
  Node 3: Parent is <node 1>. Borther is NULL. Points to <node 7> as its child.
  Node 4: Parent is <node 2>. Brother is <node 5>. Child is NULL.
  Node 5: Parent is <node 2>. Brother is <node 6>. Child is <node 8>.
  Node 6: Parent is <node 2>. Brother is NULL. Child is NULL.
  Node 7: Parent is <node 3>. Brother is NULL. Child is NULL.
  Node 8: Parent is <node 5>. Brother is NULL, Child is NULL.


  Array Fields Description
  ------------------------

  The following parameters are used for reserving traveling knowlage. The tree
  travel is PREORDER.
  FirstNode: Root node of travel.
  CurNode: Current node in travel. This is the last node being traveled.


  Tree Description:
  -----------------

  A path is a unique identifier of a node in the tree.

  A node can also be identified as a child of parent. However, the key for
  a child might not be unique and so an *index* must be used to identify
  multiple appearences of a key in a child list.


  Parameters:
  -----------
  parent: path of a node. Serves as the root of the operation.
  elem: data content of an element in list. (node data).
  index: of key in the child list. index >= 1. index=1 is the first child of
  parent that match the key.

  */

#include <stdio.h>
#include <rtree.h>
#include <ms.h>


/************************************************************************
 * rtNode
 * Node structure, holding the informatino for a tree node.
 * child    -
 * parent   - Parent of the current node in the tree.
 *            Negative value if the node is the root of a tree
 * brother  -
 * data     - Actual node information, stored here by the user of RTREE
 ************************************************************************/
typedef struct {
  int   child;
  int   parent;
  int   brother;
  void* data;
} rtNode;


/* Check if the given node is a valid one */
#define rtVALID_NODE(cur)  ((cur) >= 0)


/* RTREE and RPOOL can live in the same memory space. This is the way things are done
   in the PVT.
   To allow this, and to know which of the two are occupying a specified node, the
   first 4 bytes in each node are slightly different in RTREE and RPOOL.
   An RPOOL's node always has 0x40000000 used.
   An RTREE node never has this bit on.
   We don't use 0x80000000 as this bit to allow 0xffffffff to be used in RTREE as an invalid
   child node.
 */
#define MUCH_BIGGER_THEN_VALID_RT_LOCATION 0x1ffffffU
#define SANITY_CHECK(nodeId) ((UINT32)(rtHead(rtH,nodeId)- RVERROR)>MUCH_BIGGER_THEN_VALID_RT_LOCATION)
/* SANITY_CHECK makes sure that the node's child is an RTREE node by making sure that
   the nodes' value is small enough (meaning bit 0x40000000 is not used), and taking into
   account that it's value can be RVERROR (-1)
 */
    /* if there is no children this gives false    :: (RVERROR - RVERROR)>MUCH_BIGGER_THEN_VALID_RT_LOCATION */
    /* if there is valid child this gives false too:: (VALID_LOCATION - RVERROR)==VALID_LOCATION+1 >MUCH_BIGGER_THEN_VALID_RT_LOCATION */
    /* only if the child value is really invalid (the rpool NODE) this is TRUE  */






/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * rtAddNode
 * purpose: Add a node into RTREE
 * input  : rtH     - Handle to RTREE
 *          elem    - Element data to add
 *                    If NULL, then an empty element is added
 *          parent  - Added node's parent
 *                    Negative value if it's an added root
 *          brother - Added node's brother
 *                    Negative value if there's no brother to the right
 *                    of the current node
 * output : none
 * return : Added node's ID on success
 *          Negative value on failure
 ************************************************************************/
static int rtAddNode(
    IN HRTREE       rtH,
    IN RTElement    elem,
    IN int          parent,
    IN int          brother)
{
    HRA raH = (HRA)rtH;
    rtNode* node;
    int location;
    int size = (int)(raElemSize(raH) - sizeof(rtNode) + sizeof(void*));

    /* Add the node */
    location = raAddExt(raH, (void **)&node);
    if (location < 0) return location;

    /* Set all of the family :-) */
    node->child     = RVERROR; /* no children */
    node->parent    = parent;
    node->brother   = brother;

    /* Set the data itself */
    if (elem != NULL)
        memcpy((void *)&(node->data), (void *)elem, size);
    else
        memset((void *)&(node->data), 0, size);

    return location;
}


/************************************************************************
 * rtSetChild
 * purpose: Set a node as the first child of another node
 *          This function doesn't fix anything in the tree. It will unlink
 *          the previous child of the given parent node
 * input  : rtH     - Handle to RTREE
 *          parent  - Parent ID whose child we're going to set
 *          child   - Child ID to set
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
static int rtSetChild(
    IN  HRTREE  rtH,
    IN  int     parent,
    IN  int     child)
{
    rtNode* node;

    /* Get the parent */
    node = (rtNode *)raGet((HRA)rtH, parent);
    if (!node) return RVERROR;

    node->child = child;
    return 0;
}


/************************************************************************
 * rtSetBrother
 * purpose: Set a node as the right brother of another node
 *          This function doesn't fix anything in the tree. It will unlink
 *          the previous brother of the given node
 * input  : rtH         - Handle to RTREE
 *          location    - Node ID whose brother we're changing
 *          brother     - New brother ID to set
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
static int rtSetBrother(
    IN  HRTREE  rtH,
    IN  int     location,
    IN  int     brother)
{
    rtNode* node;

    /* Get the node */
    node = (rtNode *)raGet((HRA)rtH, location);
    if (!node) return RVERROR;

    node->brother = brother;
    return 0;
}


/************************************************************************
 * rtAddTree
 * purpose: Iterative copy of a source subtree under a destination parent
 *          node as its last child.
 *          - The source and destination RTREE elements must have the same size
 *          - There should be enough room in the destination for the source
 *            subtree
 * input  : destH           - Handle to destination RTREE
 *          destParentId    - Destination parent node. We're going to add
 *                            a new child to it (a last child)
 *          srcH            - Handle to source RTREE
 *          srcRootId       - The node ID of the source tree we're adding
 *                            under the destination parent node.
 *          fadd            - User's add function - called for every
 *                            added node in the tree.
 *                            If this function returns NULL, it is considered
 *                            an error in the execution of rtAddTree.
 *                            can be set to NULL
 *          param           - Context parameter to use on calls to fadd
 *          check           - Indicate if we want some sanity checks or not
 * output : none
 * return : Node ID of the new subtree in the destination tree on success
 *          Negative value on failure
 ************************************************************************/
static int rtAddTree(
    IN  HRTREE  destH,
    IN  int     destParentId,
    IN  HRTREE  srcH,
    IN  int     srcRootId,
    IN  RTEFunc fadd, /* user add node function */
    IN  void*   param,
    IN  BOOL    check /* false==> no checks */
      )
{
    HRA destRH = (HRA)destH;
    HRA srcRH = (HRA)srcH;
    int destParent, destNewSubTree, srcNode;

    if (check)
    {
        /* Make sure we've got everything right */
        if ((destH == NULL) || (srcH == NULL)) return RVERROR;
        if ((destParentId < 0) || (srcRootId < 0)) return RVERROR;

        /* Make sure element sizes are the same in both RTREEs */
        if (raElemSize(destRH) != raElemSize(srcRH)) return RVERROR;

        /* Make sure we've got enough room in the destination RTREE */
        if (raFreeSize(destRH) < rtTreeSize(srcH, srcRootId)) return RVERROR;
    }

    /* Add the new subtree as the last child of the given destination node */
    destNewSubTree = rtAddTail(destH, destParentId, rtGetByPath(srcH, srcRootId));

    /* See if we're done already */
    if (rtHead(srcH, srcRootId) < 0)
    {
        /* The source root node is a leaf in the tree */
        if (fadd)
            if (!(param = fadd(destH, destNewSubTree, param))) return RVERROR;
        return destNewSubTree;
    }


    /* Let's prepare for an iterative copy */
    srcNode = srcRootId;
    destParent = destNewSubTree;

    /* Copy all nodes in PRE-ORDER */
    while ((srcNode >= 0) && (destParent >= 0))
    {
        /* Call the add function of the user on the current destination parent */
        if (fadd)
            if (!(param = fadd(destH, destParent, param))) return RVERROR;

        /* 1. CHILD - copy any first born and go on through the first-borns first */
        if (rtHead(srcH, srcNode) >= 0)
        {
            /* Add the child to the destination */
            srcNode = rtHead(srcH, srcNode);
            destParent = rtAddTail(destH, destParent, rtGetByPath(srcH, srcNode));

            /* Skip everything and check the child first */
            continue;
        }

        /* 2. BROTHER - copy any close brother and check the brother for first-borns first */
        if (rtBrother(srcH, srcNode) >= 0)
        {
            /* Add the brother to the destination */
            srcNode = rtBrother(srcH, srcNode);
            destParent = rtAddTail(destH, rtParent(destH, destParent), rtGetByPath(srcH, srcNode));

            /* Skip everything and check the brother first */
            continue;
        }

        /* 3. If we're here, then we're now going up the tree up to its source root,
              each time covering all of the brothers of each level and their sub-trees
         */

        for (srcNode = rtParent(srcH, srcNode), destParent = rtParent(destH, destParent);
             (srcNode >= 0) && (srcNode != srcRootId);
             srcNode = rtParent(srcH, srcNode), destParent = rtParent(destH, destParent))
        {
            /* We'll stop on the first node that has a brother, since we haven't
               dealt with that brother yet */
            if (rtBrother(srcH, srcNode) >= 0)
            {
                /* Stop on this brother */
                srcNode = rtBrother(srcH, srcNode);
                break;
            }
        }

        /* See if we're done */
        if ((srcNode < 0) || (srcNode == srcRootId))
            break; /* completed */

        /* Add the node to the destination */
        destParent = rtAddTail(destH, rtParent(destH, destParent), rtGetByPath(srcH, srcNode));

    } /* while */

    return destNewSubTree;
}


/************************************************************************
 * rtAddTree
 * purpose: Get the next node in post order for a given sub-tree.
 * input  : rtH         - Handle of RTREE
 *          root        - Root node ID of the sub-tree to traverse
 *          location    - Current location in sub-tree
 * output : none
 * return : Next node ID on success
 *          Negative value when done or on failure
 ************************************************************************/
static int rtNextPostorder(
    IN HRTREE   rtH,
    IN int      root,
    IN int      location)
{
    int cur, tmp;

    /*
    Algorithm:
        if (child) return child;
        while (parent != root) {
          if (parent->brother) return parent->brother;
          parent = parent->parent;
        }
        return Completed!
     */

    if (!rtH || (location < 0)) return RVERROR;

    /* If we've got any child, then we return the child */
    if ((tmp = rtHead(rtH, location)) >= 0) return tmp;

    /* Go up the tree until we find a brother for one of the nodes */
    for (cur = location; (cur >= 0) && (cur != root); cur = rtParent(rtH, cur))
        if ((tmp = rtBrother(rtH, cur)) >= 0)
            return tmp;

    /* Looks like we're done */
    return RVERROR;
}


static int /* node id of the leftmost deepest child of parent */
rtLeftMostChild(
        HRTREE rtH,
        int parent)
{
  int cur, prev;

  if (!rtH || parent<0) return RVERROR;
  /*for (cur=parent; rtHead(rtH, cur) >=0; cur = rtHead(rtH, cur)); return cur;*/

  prev=cur=parent;
  while (cur>=0) {
    prev=cur;
    cur=rtHead(rtH, cur);
  }

  return prev;
}


static int /* Next node in preorder or RVERROR if travel is completed. */
rtNextPreorder(
           /*
         PRE ORDER traveling. Iterative implementation using 'location' as current pointer.
         */
           IN  HRTREE rtH,
           IN  int root, /* root of sub-tree */
           IN  int location)
{
  int tmp;

  if (location == root) return RVERROR;
  if ((tmp=rtBrother(rtH, location)) <0) return rtParent(rtH, location);

  return rtLeftMostChild(rtH, tmp);
}


static int /* TRUE or RVERROR */
rtMove2Other(
         /* move sub-tree to another sub-tree, when not on the same tree. */
         IN  HRTREE rtH,
         IN  int destNodeId,
         IN  int srcRootId,
         IN  BOOL keepSrcRoot, /* true==> srcRoot node not deleted */
         IN  RTEFunc fdelete,  /* user delete node function */
         IN  void *param       /* for add and delete user functions */
         )
{
  HRA raH = (HRA)rtH;
  rtNode *destNode, *srcNode;
  int cur;

  if (!rtH) return RVERROR;
  if (destNodeId<0 || srcRootId <0) return RVERROR;
  /*if (raFreeSize(raH) < rtTreeSize(rtH, srcRootId)) return RVERROR;*/
  if (rtGetRoot(rtH, destNodeId) == rtGetRoot(rtH, srcRootId)) return RVERROR;

  /* -- delete dest childs */
  while (rtDelete(rtH, rtHead(rtH, destNodeId), fdelete, param) == TRUE);

  /* -- move src root to dest root (+pointers) */
  srcNode = (rtNode *)raGet(raH, srcRootId);
  destNode = (rtNode *)raGet(raH, destNodeId);
  destNode->child = srcNode->child;

  memcpy((void *)&(destNode->data), (void *)&(srcNode->data),
     raElemSize(raH) -sizeof(rtNode)+sizeof(void*));

  /* -- update childrens parent reference */
  for (cur=destNode->child; cur >=0; cur=rtBrother(rtH, cur)) {
    rtNode *childNode = (rtNode *)raGet(raH, cur);
    childNode->parent = destNodeId;
  }

  /* -- delete src root node */
  srcNode->child = RVERROR;
  /*srcNode->brother = RVERROR;*/
  if (!keepSrcRoot) rtDeleteNode(rtH, srcRootId, NULL, NULL);

  return TRUE;
}










/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * rtConstruct
 * purpose: Create an RTREE object
 * input  : elemSize            - Size of elements in the RTREE in bytes
 *          maxNumOfElements    - Number of elements in RTREE
 *          logMgr              - Log manager to use
 *          name                - Name of RTREE (used in log messages)
 * output : none
 * return : Handle to RTREE constructed on success
 *          NULL on failure
 ************************************************************************/
HRTREE rtConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN RVHLOGMGR    logMgr,
    IN const char*  name)
{
    HRA raH;

    /* Create the RA pool used by RTREE */
    raH = raConstruct((int)(elemSize + sizeof(rtNode) - sizeof(void*)), maxNumOfElements, logMgr, name);

    return (HRTREE)raH;
}


/************************************************************************
 * rtDestruct
 * purpose: Deallocates an RTREE object
 * input  : rtH - RTREE handle
 * output : none
 * return : none
 ************************************************************************/
void rtDestruct(IN HRTREE rtH)
{
    raDestruct((HRA)rtH);
}


/************************************************************************
 * rtClear
 * purpose: Clear an RTREE object for any allocated nodes
 * input  : rtH - RTREE handle
 * output : none
 * return : none
 ************************************************************************/
void rtClear(IN HRTREE rtH)
{
    raClear((HRA)rtH);
}


/************************************************************************
 * rtSetCompareFunc
 * purpose: Set the compare function to use
 * input  : rtH     - Handle of the RTREE object
 *          func    - Compare function to use
 * output : none
 * return : none
 ************************************************************************/
void rtSetCompareFunc(IN HRTREE rtH, IN RTECompare func)
{
    raSetCompareFunc((HRA)rtH, (RAECompare)func);
}


/************************************************************************
 * rtSetPrintFunc
 * purpose: Set the print function to use
 * input  : rtH     - Handle of the RTREE object
 *          func    - Print function to use
 * output : none
 * return : none
 ************************************************************************/
void rtSetPrintFunc(IN HRTREE rtH, IN RTEPFunc func)
{
    raSetPrintFunc((HRA)rtH, (RAEFunc)func);
}


/************************************************************************
 * rtGetByPath
 * purpose: Return the element stored in a node ID
 * input  : rtH     - Handle of the RTREE object
 *          path    - Node ID to get from
 * output : none
 * return : element on success
 *          NULL on failure
 ************************************************************************/
RTElement rtGetByPath(IN HRTREE rtH, IN int path)
{
    rtNode* pNode;

    if ( (pNode = (rtNode *)raGet((HRA)rtH, path)) != NULL)
        return (RTElement)(&(pNode->data));
    else
        return NULL;
}


/************************************************************************
 * rtGetRoot
 * purpose: Get the root node of the tree for the given node
 * input  : rtH     - Handle of the RTREE object
 *          nodeId  - Node ID whose root we're looking for
 * output : none
 * return : Root node ID on success
 *          Negative value on failure
 * This function returns RVERROR if the given nodeId is the root itself
 ************************************************************************/
int rtGetRoot(
    IN  HRTREE  rtH,
    IN  int     nodeId)
{
    int curNode = nodeId;
    int prevNode = RVERROR;

    if (rtH == NULL) return RVERROR;

    /* Search the root until we find it */
    while (curNode >= 0)
    {
        prevNode = curNode;

        /* Get the current node's parent */
        curNode = rtParent(rtH, curNode);
    }

    return prevNode;
}


/************************************************************************
 * rtGetByIndex
 * purpose: Find one of the children of a parent node by its index
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Parent's node ID
 *          index   - Index of searched child (1-based)
 * output : none
 * return : Child's Node id on success
 *          Negative value on failure
 ************************************************************************/
int rtGetByIndex(
    IN HRTREE   rtH,
    IN int      parent,
    IN int      index)
{
    HRA raH = (HRA)rtH;
    rtNode* pNode;
    int cur, i;

    if (index < 1) return RVERROR;

    /* Get the parent */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    for (i = 1, cur = pNode->child; (i < index) && rtVALID_NODE(cur); i++)
    {
        pNode = (rtNode *)raGet(raH, cur);
        cur = pNode->brother;
    }

    return cur;
}


/************************************************************************
 * rtParent
 * purpose: Get the parent node of the given node
 * input  : rtH     - Handle of the RTREE object
 *          node    - Node ID whose parent we're looking for
 * output : none
 * return : Parent node ID on success
 *          Negative value on failure
 ************************************************************************/
int rtParent(IN HRTREE rtH, IN int node)
{
    rtNode* pNode;

    pNode = (rtNode *)raGet((HRA)rtH, node);
    if (!pNode) return RVERROR;

    return pNode->parent;
}


/************************************************************************
 * rtBrother
 * purpose: Get the next node in the same level of the current node.
 *          This is referred to as the brother of the current node.
 * input  : rtH     - Handle of the RTREE object
 *          node    - Node ID whose brother we're looking for
 * output : none
 * return : Brother's node ID on success
 *          Negative value on failure
 ************************************************************************/
int rtBrother(IN HRTREE rtH, IN int node)
{
    rtNode* pNode;

    pNode = (rtNode *)raGet((HRA)rtH, node);
    if (!pNode) return RVERROR;

    return pNode->brother;
}


/************************************************************************
 * rtHead
 * purpose: Get the first child node of the current parent node
 *          If we want to get all of the child nodes, we can call
 *          rtBrother() from hear on until we get an error
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Node ID whose first child we're looking for
 * output : none
 * return : First child's node ID on success
 *          Negative value on failure
 ************************************************************************/
int rtHead(IN HRTREE rtH, IN int parent)
{
    rtNode* node;

    node = (rtNode *)raGet((HRA)rtH, parent);
    if (!node) return RVERROR;

    return node->child;
}


/************************************************************************
 * rtTail
 * purpose: Get the last child node of the current parent node
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Node ID whose last child we're looking for
 * output : none
 * return : Last child's node ID on success
 *          Negative value on failure
 ************************************************************************/
int rtTail(IN HRTREE rtH, IN int parent)
{
    HRA raH = (HRA)rtH;
    rtNode* pNode;
    int cur;
    int lastCur = RVERROR;

    /* Get the parent's node before we begin */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    /* Travel through the nodes by the brothers until we're done */
    for (cur = pNode->child; rtVALID_NODE(cur); cur = pNode->brother)
    {
        lastCur = cur;

        /* Check the next node for its brother */
        pNode = (rtNode *)raGet(raH, cur);
    }

    return lastCur;
}


/************************************************************************
 * rtIndex
 * purpose: Get the index of the given child under the parent node
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Node ID of the parent
 *          child   - Child's node ID
 * output : none
 * return : Index of the child inside the parent (1-based)
 *          Negative value on failure
 ************************************************************************/
int rtIndex(IN HRTREE rtH, IN int parent, IN int child)
{
    HRA raH = (HRA)rtH;
    rtNode* pNode;
    int cur;
    int index = 1;

    /* Get the parent */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    /* Search through the children until we find the one matching the one we're looking for */
    for (cur = pNode->child; (cur != child) && rtVALID_NODE(cur); cur = pNode->brother)
    {
        pNode = (rtNode *)raGet(raH, cur);
        index++;
    }

    if (cur == child)
        return index;
    else
        return RVERROR;
}


/************************************************************************
 * rtNumChilds
 * purpose: Get the number of child nodes under a given node
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Node ID of the parent
 * output : none
 * return : Number of child nodes under the parent
 *          Negative value on failure
 ************************************************************************/
int rtNumChilds(IN HRTREE rtH, IN int parent)
{
    HRA raH = (HRA)rtH;
    rtNode* pNode;
    int numChilds = 0;
    int cur=0; /* current location of child node */

    /* Get the parent */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    /* scan child list and search for child */
    for (cur = pNode->child; rtVALID_NODE(cur); cur = pNode->brother)
    {
        pNode = (rtNode *)raGet(raH, cur);
        numChilds++;
    }

    return numChilds;
}


/************************************************************************
 * rtNext
 * purpose: Get the next node for a given node ID
 *          This function can be used to traverse the tree, each time
 *          returning another node ID inside the tree.
 *          This function uses POST-ORDER traversal of the tree
 * input  : rtH         - Handle of the RTREE object
 *          root        - Root node ID of the sub-tree we're traversing
 *          location    - Current node ID
 * output : none
 * return : Next node ID in the tree on success
 *          Negative value when done or on failure
 ************************************************************************/
int rtNext(
    IN  HRTREE  rtH,
    IN  int     root,
    IN  int     location)
{
    return rtNextPostorder(rtH, root, location);
}


/************************************************************************
 * rtAddRoot
 * purpose: Add a new root node as a tree to RTREE
 * input  : rtH     - Handle of the RTREE object
 *          elem    - Element to add as root
 *                    If given as NULL, an empty element is added
 * output : none
 * return : Node ID of the new root on success
 *          Negative value on failure
 ************************************************************************/
int rtAddRoot(
    IN  HRTREE      rtH,
    IN  RTElement   elem)
{
    return rtAddNode(rtH, elem, RVERROR, RVERROR);
}


/************************************************************************
 * rtAddHead
 * purpose: Add a new node as the first child, before any other of a given
 *          parent node
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Parent's node ID
 *          elem    - Element to add
 *                    If given as NULL, an empty element is added
 * output : none
 * return : Node ID of the added node on success
 *          Negative value on failure
 ************************************************************************/
int rtAddHead(
    IN  HRTREE      rtH,
    IN  int         parent,
    IN  RTElement   elem)
{
    rtNode* pNode;
    int location;
    HRA raH = (HRA)rtH;

    /* Get the parent */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    /* Add the element as the first child */
    location = rtAddNode(rtH, elem, parent, pNode->child);
    if (location < 0) return location;

    /* Make sure the parent knows it's first born */
    pNode->child = location;

    return location;
}


/************************************************************************
 * rtAddTail
 * purpose: Add a new node as the last child of a parent
 * input  : rtH     - Handle of the RTREE object
 *          parent  - Parent's node ID
 *          elem    - Element to add
 *                    If given as NULL, an empty element is added
 * output : none
 * return : Node ID of the added node on success
 *          Negative value on failure
 ************************************************************************/
int rtAddTail(
    IN  HRTREE      rtH,
    IN  int         parent,
    IN  RTElement   elem)
{
    rtNode* pNode;
    rtNode* lastChild;
    int location;
    HRA raH = (HRA)rtH;

    /* Get the parent */
    pNode = (rtNode *)raGet(raH, parent);
    if (!pNode) return RVERROR;

    /* Find the last child of this parent */
    lastChild = (rtNode *)raGet(raH, rtTail(rtH, parent));

    /* Add the node after the last child */
    location = rtAddNode(rtH, elem, parent, RVERROR);
    if (location < 0) return location;

    if (lastChild != NULL)
    {
        /* There's a last child - make sure it's connects the new one as it's brother */
        lastChild->brother = location;
    }
    else
    {
        /* There are no children to this parent - it's the first born */
        pNode->child = location;
    }

    return location;
}



/************************************************************************
 * rtAddBrother
 * purpose: Add a new node as the closest (right) brother of a given node
 * input  : rtH     - Handle of the RTREE object
 *          brother - Brother node we're joining to the right
 *          elem    - Element to add
 *                    If given as NULL, an empty element is added
 * output : none
 * return : Node ID of the added node on success
 *          Negative value on failure
 ************************************************************************/
int rtAddBrother(
    IN  HRTREE      rtH,
    IN  int         brother,
    IN  RTElement   elem)
{
    rtNode* pNode;
    int location;
    HRA raH = (HRA)rtH;

    /* Get the brother */
    pNode = (rtNode *)raGet(raH, brother);
    if (!pNode) return RVERROR;

    /* Add this element after our given bro' */
    location = rtAddNode(rtH, elem, pNode->parent, pNode->brother);
    if (location < 0) return location;

    /* Make sure the older brother knows of this new brother */
    pNode->brother = location;

    return location;
}


/************************************************************************
 * rtAdd
 * purpose: Iterative copy of a source subtree under a destination parent
 *          node as its last child.
 *          - The source and destination RTREE elements must have the same size
 *          - There should be enough room in the destination for the source
 *            subtree
 * input  : destH           - Handle to destination RTREE
 *          destParentId    - Destination parent node. We're going to add
 *                            a new child to it (a last child)
 *          srcH            - Handle to source RTREE
 *          srcRootId       - The node ID of the source tree we're adding
 *                            under the destination parent node.
 *          fadd            - User's add function - called for every
 *                            added node in the tree.
 *                            If this function returns NULL, it is considered
 *                            an error in the execution of rtAdd.
 *                            can be set to NULL
 *          param           - Context parameter to use on calls to fadd
 * output : none
 * return : Node ID of the added node on success
 *          Negative value on failure
 ************************************************************************/
int rtAdd(
    IN  HRTREE  destH,
    IN  int     destParentId,
    IN  HRTREE  srcH,
    IN  int     srcRootId,
    IN  RTEFunc fadd,
    IN  void*   param)
{
    return rtAddTree(destH, destParentId, srcH, srcRootId, fadd, param, TRUE);
}


/************************************************************************
 * rtAdd
 * purpose: Iterative copy of all the children of a source node under a
 *          destination parent node as its last children.
 *          - The source and destination RTREE elements must have the same size
 *          - There should be enough room in the destination for the source
 *            subtree
 * input  : destH           - Handle to destination RTREE
 *          destParentId    - Destination parent node. We're going to add
 *                            a new child to it (a last child)
 *          srcH            - Handle to source RTREE
 *          srcRootId       - The node ID of the source tree whose children
 *                            we're adding under the destination parent node.
 *          fadd            - User's add function - called for every
 *                            added node in the tree.
 *                            If this function returns NULL, it is considered
 *                            an error in the execution of rtAddChilds.
 *                            can be set to NULL
 *          param           - Context parameter to use on calls to fadd
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rtAddChilds(
    IN  HRTREE  destH,
    IN  int     destParentId,
    IN  HRTREE  srcH,
    IN  int     srcRootId,
    IN  RTEFunc fadd,
    IN  void*   param)
{
    int cur, result;

    if ((destParentId < 0) || (srcRootId < 0)) return RVERROR;

    /* Go throught the source root's children and copy them one by one */
    for (cur = rtHead(srcH, srcRootId); cur >= 0; cur = rtBrother(srcH, cur))
    {
        result = rtAdd(destH, destParentId, srcH, cur, fadd, param);
        if (result < 0) return result;
    }

    return 0;
}


/************************************************************************
 * rtSet
 * purpose: Copy a source sub-tree onto destParentId, deleting any previous
 *          content and nodes under destParentId
 *          - The source and destination RTREE elements must have the same size
 *          - There should be enough room in the destination for the source
 *            subtree
 * input  : destH           - Handle to destination RTREE
 *          destParentId    - Destination parent node. We're going to delete
 *                            it and copy srcRootId onto it
 *          srcH            - Handle to source RTREE
 *          srcRootId       - The node ID of the source tree we'll copy
 *          fadd            - User's add function - called for every
 *                            added node in the tree.
 *                            If this function returns NULL, it is considered
 *                            an error in the execution of rtAddChilds.
 *                            can be set to NULL
 *          fdelete         - User's delete function - caled for every
 *                            deleted node under destParentId, including destParentId
 *          param           - Context parameter to use on calls to fadd and fdelete
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rtSet(
    IN  HRTREE  destH,
    IN  int     destParentId,
    IN  HRTREE  srcH,
    IN  int     srcRootId,
    IN  RTEFunc fadd,
    IN  RTEFunc fdelete,
    IN  void*   param)
{
    HRA destRH = (HRA)destH;
    HRA srcRH = (HRA)srcH;
    int cur;
    rtNode* destNode;
    rtNode* srcNode;

    if (!destH || !srcH) return RVERROR;
    if (destParentId < 0 || srcRootId < 0) return RVERROR;
    if (raElemSize(destRH) != raElemSize(srcRH)) return RVERROR;
/*  if (raFreeSize(destRH) < rtTreeSize(srcH, srcRootId)) return RVERROR;*/

    /* -- delete all children sub-trees */
    while (rtDelete(destH, rtHead(destH, destParentId), fdelete, param) >= 0);

    /* -- delete destination root node */
    if (fdelete)
        if (!(param = fdelete(destH, destParentId, param))) return RVERROR;

    /* -- set dest parent */
    srcNode = (rtNode *)raGet(srcRH, srcRootId);
    destNode = (rtNode *)raGet(destRH, destParentId);
    if (!destNode || !srcNode) return RVERROR;

    memcpy((void *)&(destNode->data), (void *)&(srcNode->data),
    raElemSize(destRH) -sizeof(rtNode)+sizeof(void*));

    if (fadd)
        if (!(param = fadd(destH, destParentId, param))) return RVERROR;

    /* -- add all src childs under dest parent */
    for (cur = rtHead(srcH, srcRootId); cur >= 0; cur = rtBrother(srcH, cur))
        if (rtAddTree(destH, destParentId, srcH, cur, fadd, param, FALSE) < 0)
            return RVERROR;

    return 0;
}


/* move sub-tree to another sub-tree, NO restriction */
int rtMove(
    IN  HRTREE  rtH,
    IN  int     destNodeId,
    IN  int     srcRootId,
    IN  BOOL    keepSrcRoot, /* true==> srcRoot node not deleted */
    IN  RTEFunc fdelete,  /* user delete node function */
    IN  void*   param       /* for add and delete user functions */
    )
  /* using temporary node (R) */
{
  HRA raH = (HRA)rtH;
  int R;

  if (!rtH) return RVERROR;
  if (destNodeId<0 || srcRootId <0) return RVERROR;
  if (raFreeSize(raH) < 1 /*rtTreeSize(rtH, srcRootId)+1*/) return RVERROR;

  R = rtAddRoot(rtH, NULL);
  rtMove2Other(rtH, R, srcRootId, keepSrcRoot, fdelete, param);
  rtMove2Other(rtH, destNodeId, R, FALSE, fdelete, param);
  return 0;
}









int /* number of nodes in sub-tree */
rtTreeSize(
       /* calc. num of nodes in subtree */
       IN  HRTREE rtH,
       IN  int rootId
       )
{
  int cur, count;

  if (!rtH) return RVERROR;

  for (count=0, cur=rtHead(rtH, rootId); cur>=0; cur = rtNext(rtH, rootId, cur))
    count++;
  return count+1;
}








/*_________________________________two trees operations________________________________*/



int /* TRUE or RVERROR */
rtCompareTrees(
           /* Compare between two trees.
          The trees must be structure identical.
          The compare function check node content.
          */
           IN  HRTREE destH,
           IN  int destRootId,
           IN  HRTREE srcH,
           IN  int srcRootId,
           IN  RTECompareTwo fcompare, /* compare two nodes */
           IN  void *param /* for compare function */
           )
{
  HRA destRH = (HRA)destH;
  HRA srcRH = (HRA)srcH;
  int destCur, srcCur;

  if (!destH || !srcH) return RVERROR;
  if (destRootId<0 || srcRootId <0) return RVERROR;
  if (raElemSize(destRH) != raElemSize(srcRH)) return RVERROR;
  if (rtTreeSize(destH, destRootId) != rtTreeSize(srcH, srcRootId)) return RVERROR;

  for (destCur = destRootId, srcCur = srcRootId;
       destCur >=0 && srcCur >=0;
       destCur = rtNext(destH, destRootId, destCur), srcCur = rtNext(srcH, srcRootId, srcCur)) {
    if (rtNumChilds(destH, destCur) != rtNumChilds(srcH, srcCur)) return RVERROR;
    if (fcompare) {
      if (!fcompare(rtGetByPath(destH, destCur), rtGetByPath(srcH, srcCur), param))
    return RVERROR;
    }
    else {
      if (memcmp(rtGetByPath(destH, destCur), rtGetByPath(srcH, srcCur),
         raElemSize(destRH)-sizeof(rtNode)+sizeof(void*)))
    return RVERROR;
    }
  }

  return TRUE;
}



/*___________________________________move________________________________*/

/*___________________________________Family Relocations________________________________*/
int /* TRUE or RVERROR */
rtAdoptChild(
         /* Child is adopted by its new family. new brother is a child of new parent. */
         IN  HRTREE rtH,
         IN  int adoptedChildId, /* child to be adopted by the new family */
         IN  int newParentId, /* parent of adopted child. -1: become root */
         IN  int newBrotherId /* previously born child (left brother). -1: first born */
         )
{
  HRA raH = (HRA)rtH;
  rtNode *adoptedNode;
  int parent; /* adoptedChild current parent */


  if (!rtH) return RVERROR;
  if (newParentId<0) newParentId=RVERROR;
  if (newBrotherId<0) newBrotherId=RVERROR;
  if ( (adoptedNode = (rtNode *)raGet(raH, adoptedChildId)) == NULL) return RVERROR;

  /* -- check legitimacy of new parent and brother and relations */
  if (newParentId >=0) {
    if (raGet(raH, newParentId) == NULL) return RVERROR;
    if (newBrotherId >=0) {
      if ( raGet(raH, newBrotherId) == NULL) return RVERROR;
      if ( rtParent(rtH, newBrotherId) != newParentId) return RVERROR;
    }
  }

  /* -- remove former relations */
  parent = rtParent(rtH, adoptedChildId);
  if (parent >=0)  { /* update parent links */
    if (rtHead(rtH, parent) == adoptedChildId) /* leftmost child */
      rtSetChild(rtH, parent, rtBrother(rtH, adoptedChildId));
    else { /* has left brother */
      int cur;
      for (cur=rtHead(rtH, parent); rtBrother(rtH, cur) != adoptedChildId; cur=rtBrother(rtH, cur));
      rtSetBrother(rtH, cur, rtBrother(rtH, adoptedChildId));
    }
  }

  /* -- join your new family */
  adoptedNode->parent = newParentId;
  adoptedNode->brother = rtBrother(rtH, newBrotherId); /* apart your younger brother */

  if (newParentId<0) return TRUE; /* become head of the family. I.e. root */

  if (newBrotherId<0) { /* become first born child */
    adoptedNode->brother = rtHead(rtH, newParentId);
    rtSetChild(rtH, newParentId, adoptedChildId);
    return TRUE;
  }

  rtSetBrother(rtH, newBrotherId, adoptedChildId);
  return TRUE;
}



/*___________________________________Delete________________________________*/

int /* TRUE or RVERROR */
rtDeleteNode(
         /* delete this node and update links (parent or brother) */
         /* Must be a leaf (no childs) */
         IN  HRTREE rtH,
         IN  int location,  /* this node id */
         IN  RTEFunc fdelete, /* delete function */
         IN  void *param
         )
{
  int parent;

  if (!rtGetByPath(rtH, location))
    return RVERROR; /* delete root node */


  if (rtHead(rtH, location) >=0)  /* no childs allowed */
    return RVERROR; /* delete root node */


  /* -- user delete node */
  if (fdelete)
    if (!(param = fdelete(rtH, location, param)))
      return RVERROR; /* delete root node */


  parent = rtParent(rtH, location);
  if (parent >=0)  { /* update parent links */
    if (rtHead(rtH, parent) == location) /* leftmost child */
      rtSetChild(rtH, parent, rtBrother(rtH, location));
    else { /* has left brother */
      int cur;
      for (cur=rtHead(rtH, parent); rtBrother(rtH, cur) != location; cur=rtBrother(rtH, cur));
      rtSetBrother(rtH, cur, rtBrother(rtH, location));
    }
  }

  return raDeleteLocation((HRA)rtH, location);
}


int /* TRUE or RVERROR */
rtDelete(
     /* delete subtree from rootId. Iterative preorder deleting */
     IN  HRTREE rtH,
     IN  int rootId, /* subtree root id */
     IN  RTEFunc fdelete, /* delete function */
     IN  void *param
     )
{
  int cur, nextCur;

  if (rootId <0) return RVERROR;
  if (SANITY_CHECK(rootId)) return RVERROR;
  for (cur=rtLeftMostChild(rtH, rootId); cur>=0 && cur != rootId; ) {
    if (SANITY_CHECK(cur)) return RVERROR;
    nextCur = rtNextPreorder(rtH, rootId, cur);
    /*if (rtDeleteNode(rtH, cur, fdelete, param) <0) return RVERROR;*/
    if (fdelete) if (!(param = fdelete(rtH, cur, param)))
      return RVERROR;  /*user delete function*/

    if (raDeleteLocation((HRA)rtH, cur) <0)
      return RVERROR;


    cur = nextCur;
  }

  if (!raElemIsVacant((HRA)rtH, rootId)) rtSetChild(rtH, rootId, RVERROR);
  if (rtDeleteNode(rtH, rootId, fdelete, param) <0)
    return RVERROR; /* delete root node */

  return TRUE;
}


int /* TRUE or RVERROR */
rtDeleteChilds(
           /* delete all child-sub-trees of rootId. Iterative preorder deleting */
           IN  HRTREE rtH,
           IN  int rootId, /* subtree root id */
           IN  RTEFunc fdelete, /* delete function */
           IN  void *param
           )
{
  if (rootId <0) return RVERROR;
  while (rtDelete(rtH, rtHead(rtH, rootId), fdelete, param) >0);
  return TRUE;
}


/*___________________________________Search________________________________*/

/*
  Desc: get path of child by key and index.
  Returns: path to child or RVERROR.
  Note: index >=1

  -> finds the index'th child matching the criteria
  */
int
rtGetChild(HRTREE rtH, int parent, void *param, int index)
{
    HRA raH = (HRA)rtH;
    rtNode *pNode;
    int cur, lastCur=0;
    int found = 0;
    RTECompare compare = (RTECompare)raFCompare(raH);

    if (index <1) return RVERROR;

    /* scan child list and search for child */
    if ( (pNode = (rtNode *)raGet(raH, parent)) == NULL)
        return RVERROR;

    cur = pNode->child;
    found = 0;

    if (compare)
    {
        for (; ( (found < index) && rtVALID_NODE(cur) );
             cur = pNode->brother)
        {
            pNode = (rtNode *)raGet(raH, cur);
			if (pNode == NULL)
				break;
            lastCur = cur; /* save this location */
            if ( (compare(&(pNode->data), param)) )
                found++;
        }
    }
    else
    {
        for (; ( (found < index) && rtVALID_NODE(cur) );
             cur = pNode->brother)
        {
            pNode = (rtNode *)raGet(raH, cur);
			if (pNode == NULL)
				break;
            lastCur = cur; /* save this location */
            if ( pNode->data == param )
                found++;
        }
    }

    /* found! */
    if (found == index)
    {
        return lastCur;
    }

    /* not found... */
    return RVERROR;
}




int
rtFind(HRTREE rtH, int subTreeRoot, void *param, int index)
{
  HRA raH = (HRA)rtH;
  RTECompare compare = (RTECompare)raFCompare(raH);
  rtNode *pNode;
  int curNode, lastCur=0;
  int hits = 0; /* number of successfull find operations */

  for (curNode = subTreeRoot;
       curNode>=0 && hits<index;
       curNode = rtNext(rtH, subTreeRoot, curNode)) {
    lastCur = curNode;
    pNode = (rtNode *)raGet(raH, curNode);
    if ((compare && compare(&(pNode->data), param)) ||
    pNode->data == param) hits++;
  }

  if (hits == index) return lastCur;
  else return RVERROR;
}

int
rtCompare(HRTREE rtH, int subTreeRoot, void *param, int index, RTECompare compare)
{
  HRA raH = (HRA)rtH;
  rtNode *pNode;
  int curNode, lastCur=0;
  int hits = 0; /* number of successfull find operations */

  for (curNode = subTreeRoot;
       curNode>=0 && hits<index;
       curNode = rtNext(rtH, subTreeRoot, curNode)) {
    lastCur = curNode;
    pNode = (rtNode *)raGet(raH, curNode);
    if ((compare && compare(&(pNode->data), param)) ||
    pNode->data == param) hits++;
  }

  if (hits == index) return lastCur;
  else return RVERROR;
}












/*_____________________________________overall__________________________________*/


/************************************************************************
 * rtDoAll
 * purpose: Execute a given function on all of the roots inside RTREE.
 *          Note that this function will only be called on the roots and
 *          not on all the allocated nodes.
 * input  : rtH         - Handle to RTREE
 *          operation   - Function to call on all roots
 *          param       - Context parameter to use on calls
 * output : none
 * return : none
 ************************************************************************/
void rtDoAll(
    IN HRTREE   rtH,
    IN RTEFunc  operation,
    IN void*    param)
{
    int i;
    void* context = param;

    if ((!rtH) || (operation == NULL)) return;

    /* Pass through all the elements, executing the functions on those
       which are used roots */
    for (i = 0; i < raMaxSize((HRA)rtH); i++)
        if (!raElemIsVacant((HRA)rtH, i))
        {
            /* The sanity check is here to make sure the node is RTREE, since
               RTREE and RPOOL reside on the same memory space in the PVT */
            if (!SANITY_CHECK(i) && (rtParent(rtH, i) < 0)) /* only roots */
            {
                /* We change the context by the return value of the function */
                context = operation(rtH, i, context);
            }
        }
}




/*_____________________________________Display__________________________________*/


static int
rtFuncPrintLayer(
         HRTREE rtH,
         int parent,
         int maxLevel, /* -1=infinite */
         void *param,
         int msa,
         RTEPFunc fpr,
         unsigned int layer
         )
{
  int cur;

  if (parent <0) return TRUE;

  fpr(rtH, parent, (int)layer, param);
  if (maxLevel>=0 && (int)layer>=maxLevel) return TRUE;

  for (cur = rtHead(rtH, parent);
       cur >=0;
       cur = rtBrother(rtH, cur))
    rtFuncPrintLayer(rtH, cur, maxLevel, param, msa, fpr, layer+1);

  return TRUE;
}


int
rtPrint(
          /* Recursive print of tree. */
          IN  HRTREE rtH,
          IN  int parent,
          IN  unsigned int layer, /* start printing layer */
          IN  int maxLevel, /* -1=infinite, 0==>parent node */
          IN  void *param,
          IN  int msa)
{
    return rtFuncPrintLayer(rtH, parent, maxLevel, param, msa, (RTEPFunc)raGetPrintFunc((HRA)rtH), layer);
}



/*
  Return: path of first root node
  */
int
rtRoot(HRTREE rtH)
{
    HRA raH = (HRA)rtH;
    int hits, cur;

    for (hits=0, cur=0; hits<1 && cur<raMaxSize(raH); cur++)
        if (raElemIsVacant(raH, cur) == FALSE && rtParent(rtH, cur) <0) hits++;

    return (hits==1)?(cur-1):(RVERROR);
}


int /* current number of elements */
rtCurSize(HRTREE rtH)
{
  return raCurSize((HRA)rtH);
}

int /* returns maximum number of elements. */
rtMaxSize(HRTREE rtH)
{
  return raMaxSize((HRA)rtH);
}

int /* tree max usage */
rtMaxUsage(HRTREE rtH)
{  return raMaxUsage((HRA)rtH); }



#ifdef __cplusplus
}
#endif



