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

#ifndef _PVALTREE_DEF_H
#define _PVALTREE_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rvinternal.h>
#include <rlist.h>
#include <rtree.h>
#include <rpool.h>
#include <mei.h>
#include <psyntreeDb.h>
#include <psyntreeStackApi.h>
#include <pvaltree.h>



#define UNDEFINED_TYPE -1905


#ifndef NOLOGSUPPORT
/************************************************************************
 * pvtPrintDegree enum
 * Used to define what will be printed for each node of a tree
 * pvtPrintDegreeSyntax     - Display syntax information
 * pvtPrintDegreeNodeId     - Display node ID for each node
 * pvtPrintDegreeValue      - Always print the node's value
 * pvtPrintDegreeSyntaxId   - Display syntax node ID for each node
 *
 * pvtPrintDegreeNode       - Print nothing
 * pvtPrintDegreeAll        - Print everything
 ************************************************************************/
typedef enum {
  pvtPrintDegreeSyntax      = 0x1, /* display syntax information */
  pvtPrintDegreeNodeId      = 0x2,
  pvtPrintDegreeValue       = 0x4, /* always print node value */
  pvtPrintDegreeSyntaxId    = 0x8,

  pvtPrintDegreeNone        = 0x0,
  pvtPrintDegreeAll         = 0xffff
} pvtPrintDegree;
#endif


/************************************************************************
 * vtStruct
 * PVT handle contents
 * vTree        - Value tree handle to use
 *                Should be first field for performance reasons
 * sPool        - Strings pool. Holds all the strings for the nodes
 * log          - Log handle to use for PVT related messages
 * unregLog     - Log handle used for PVT messages by user on no-log handle
 * lock         - Mutex to use for locking
 *
 * Tree printing related variables
 * pFunc            - Print function to use for each node printed
 * pFuncParam       - The context to pass to the print function on each printed line
 * printOnlyRoots   - Indicates if debug printing of all nodes will print only the
 *                    roots or the whole trees
 * degree           - Details degree of the printing
 ************************************************************************/
typedef struct
{
    HRTREE  vTree;
    HRPOOL  sPool;
    RVHLOG  log;
    RVHLOG  unregLog;
    HMEI    lock;

#ifndef NOLOGSUPPORT
    pvtPrintFuncP   pFunc;
    int             pFuncParam;
    BOOL            printOnlyRoots;
    pvtPrintDegree  degree;
#endif
} vtStruct;


/************************************************************************
 * vtNode
 * PVT node contents
 * hSyn             - Syntax tree of the node
 *                    Make sure it's first (optimized pointer access)
 * synFieldAndNode  - Field and node in syntax tree
 * string           - String stored inside node
 *                    NULL if it doesn't exist
 * value            - Value of node for non-string values
 *                    Length of string for string values
 ************************************************************************/
typedef struct {
  HPST      hSyn;
  UINT32    synFieldAndNode;
  char*     string;
  INT32     value;
} vtNode;





/************************************************************************
 *
 *                          Private macros
 *
 ************************************************************************/



#define VTN_SYN_FIELD(vtnode) ((INT16)((vtnode)->synFieldAndNode & 0xFFFF))
#define VTN_SYN_NODE(vtnode)  ((INT16)((vtnode)->synFieldAndNode >> 16))

#define VTN_SET_SYN_INFO(vtnode, node, field) \
    ((vtnode)->synFieldAndNode = ((unsigned)node << 16) + (UINT16)field)
#define VTN_SET_SYN_FIELD(vtnode, field) \
    ((vtnode)->synFieldAndNode = ((vtnode)->synFieldAndNode & 0xFFFF0000) + (unsigned)field);
#define VTN_SET_SYN_NODE(vtnode, node) \
    ((vtnode)->synFieldAndNode = ((vtnode)->synFieldAndNode & 0xFFFF) + ((unsigned)node << 16))


#define GET_NODE(vt, nodeId) \
    ((vtNode *)rtGetByPath(((vtStruct *)(vt))->vTree, (nodeId)))




#ifdef __cplusplus
}
#endif


#endif  /* _PVALTREE_DEF_H */

