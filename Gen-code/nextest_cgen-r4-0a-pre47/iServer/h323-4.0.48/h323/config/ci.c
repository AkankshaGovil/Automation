/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  ci.c  --  Configuration Interface

  Module Author: Oz Solomonovich
  This Comment:  26-Dec-1996

  Abstract:      Low level configuration interface for the H323 Stack.

                 Configuration information can reside in a number of
                 different sources.  CI converts the data from the
                 configuration source into an internal R-Tree, with the
                 aid of the helper functions found in cisupp.c.

  Platforms:     All.

  Known Bugs:    None.

***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <rvinternal.h>
#include <rtree.h>
#include <rpool.h>

#include "ci.h"
#include "cisupp.h"

/* local functions */
static int ciGetNodeID(IN HCFG hCfg, IN const char *path);


#define POOL_OVERHEAD  4


RVAPI
HCFG RVCALLCONV ciConstruct(IN const char *source)
{
    return ciConstructEx(source, 0, 0);
}

RVAPI
HCFG RVCALLCONV ciConstructEx(
        IN  const char * source,     /* configuration source identifier */
        IN  int          extraData,  /* maximum amount of data growth */
        IN  int          extraNodes  /* maximum number of nodes growth */
        )
{
    int nodes, data, size;

    cfgHandle *hCfg = (cfgHandle*)malloc(sizeof(cfgHandle));


    if (!hCfg)
        return NULL;

    hCfg->pool = 0;
    hCfg->tree = 0;

    if (source != NULL)
    {
        ciEstimateCfgSize(source, &nodes, &data);
        if (nodes < 0  ||  data < 0)
        {
            ciDestruct((HCFG)hCfg);
            return NULL;
        }
    }
    else
    {
        data = 0;
        nodes = 0;
    }

    data  += extraData;
    nodes += extraNodes;

    size = (nodes + 1) * 2 * POOL_OVERHEAD + data + 100;
    size = (size + CONFIG_RPOOL_BLOCK_SIZE) / CONFIG_RPOOL_BLOCK_SIZE;   /* set to chuncks of 8 bytes */

    hCfg->pool = rpoolConstruct(CONFIG_RPOOL_BLOCK_SIZE, size, (RVHLOGMGR)1, "CONFIG");
    if ((hCfg->tree = ciBuildRTree(source, nodes, hCfg->pool, (RVHLOGMGR)1)) == NULL)
    {
        ciDestruct((HCFG)hCfg);
        return NULL;
    }

    return (HCFG)hCfg;
}

RVAPI
ci_errors RVCALLCONV ciDestruct(IN HCFG hCfg)
{
    int nodeID,rootID;
    if (!hCfg)
        return ERR_CI_GENERALERROR;

    rootID=nodeID=rtRoot(__hCfg->tree);
    if (__hCfg->tree)  rtDestruct  (__hCfg->tree);
    if (__hCfg->pool)  rpoolDestruct(__hCfg->pool);

    free((void *)hCfg);

    return ERR_CI_NOERROR;

}

RVAPI
ci_errors RVCALLCONV ciSave(IN HCFG hCfg,
                          IN char *target)
{

    return (ci_errors)ciOutputRTree(target, __hCfg->tree, __hCfg->pool);

}

static int ciGetNodeID(IN HCFG hCfg, IN const char *path)
{
    int nodeID;
    int len;
    char *dot;
    pcfgValue cfgVal;

    if (!path || !*path)
        return ERR_CI_GENERALERROR;

    if ((nodeID = rtHead(__hCfg->tree, rtRoot(__hCfg->tree))) < 0)
        return ERR_CI_GENERALERROR;

    while (*path)
    {
        dot = strchr(path, '.');
        len = dot? dot - path : strlen(path);
        cfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);

        while (rpoolChunkSize(__hCfg->pool, cfgVal->name) != len  ||
               rpoolCompareExternal(__hCfg->pool, cfgVal->name, (void*)path, len))
        {
            nodeID = rtBrother(__hCfg->tree, nodeID);
            if (nodeID < 0)
                return ERR_CI_NOTFOUND;
            cfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);
        }

        if (!dot)
            break;

        nodeID = rtHead(__hCfg->tree, nodeID);
        if (nodeID < 0)
            return ERR_CI_NOTFOUND;

        path = dot + 1;
    }

    return nodeID;
}


RVAPI
ci_errors RVCALLCONV ciGetValue(
        IN  HCFG         hCfg,
        IN  const char * path,      /* full path to nodeID, i.e. "a.b.c" */
        OUT BOOL  *      isString,
        OUT INT32 *      value      /* data for ints, length for strings */
        )
{
    ci_str_type strtype = ci_str_not;
    ci_errors err;
    err = ciGetValueExt(hCfg, path, &strtype, value);
    if (isString) *isString = (strtype != ci_str_not);
    return err;
}


RVAPI
ci_errors RVCALLCONV ciGetValueExt(
        IN  HCFG         hCfg,
        IN  const char  *path,      /* full path to nodeID, i.e. "a.b.c" */
        OUT ci_str_type *strType,
        OUT INT32 *      value      /* data for ints, length for strings */
        )
{
    int bits, nodeID = ciGetNodeID(hCfg, (char *)path);
    pcfgValue cfgVal;
    ci_str_type dummy;

    if (nodeID < 0)
        return (ci_errors)nodeID;

    cfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);

    if (value) *value = cfgVal->value;

    if (!strType) strType = &dummy;

    if (cfgVal->isString)
    {
        char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];
        rpoolCopyToExternal(__hCfg->pool, (void *)buff, cfgVal->str, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
        if ((bits = ciIsBitString(buff, cfgVal->value)) >= 0)
        {
            *strType = ci_str_bit;
            if (value) *value = (bits + 7) / 8;
        }
        else
        {
            *strType = ci_str_regular;
        }
    }
    else
        *strType = ci_str_not;
    
    return ERR_CI_NOERROR;
}


static
ci_errors RVCALLCONV ciGetStringInternal(
                                         IN   HCFG         hCfg,
                                         IN   const char * path,      /* full path to nodeID, i.e. "a.b.c" */
                                         OUT  char *       str,
                                         IN   UINT32       maxStrLen, /* length of output string buffer */
                                         OUT  UINT32 *     outbits
                                         )
{
    int bits, length, nodeID = ciGetNodeID(hCfg, (char *)path);
    pcfgValue cfgVal;
    const char *srcstr;
    char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];
    
    if (nodeID < 0)
        return (ci_errors)nodeID;

    cfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);

    if (!cfgVal->isString)
        return ERR_CI_GENERALERROR;

    rpoolCopyToExternal(__hCfg->pool, (void *)buff, cfgVal->str, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
    if ((bits = ciIsBitString(buff, cfgVal->value)) >= 0)
    {
        length = (bits + 7) / 8;
        srcstr = ciGetBitStringData(buff);
    }
    else
    {
        length = cfgVal->value;
        srcstr = buff;
    }

    if (maxStrLen < ((UINT32)length))
        return ERR_CI_BUFFERTOOSMALL;

    memcpy(str, srcstr, length);
    if (maxStrLen > ((UINT32)length))
        str[(int)length] = '\0';

    if (outbits) *outbits = bits;

    return ERR_CI_NOERROR;
}

RVAPI
ci_errors RVCALLCONV ciGetString(
        IN   HCFG         hCfg,
        IN   const char * path,      /* full path to nodeID, i.e. "a.b.c" */
        OUT  char *       str,
        IN   UINT32       maxStrLen  /* length of output string buffer */
        )
{
    return ciGetStringInternal(hCfg, path, str, maxStrLen, NULL);
}

RVAPI
ci_errors RVCALLCONV ciGetBitString(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node, i.e. "a.b.c" */
        OUT char *       str,
        IN  UINT32       maxStrLen,  /* length of output string buffer */
        OUT UINT32 *     bits
        )
{
    return ciGetStringInternal(hCfg, path, str, maxStrLen, bits);
}


RVAPI
ci_errors RVCALLCONV ciNext(
        IN   HCFG         hCfg,
        IN   const char * path,       /* full path to nodeID */
        OUT  char       * nextPath,   /* buffer for next path in cfg */
        IN   UINT32       maxPathLen  /* length of output buffer */
        )
{
    int len, firstTime = 1;
    int root = rtRoot(__hCfg->tree);
    int nodeID = path? ciGetNodeID(hCfg, path) : root;
    pcfgValue cfgVal;

    if (root < 0)
        return ERR_CI_GENERALERROR;
    if (nodeID < 0)
        return ERR_CI_NOTFOUND;

    do
    {
        nodeID = rtNext(__hCfg->tree, root, nodeID);
        if (nodeID < 0)
            return ERR_CI_NOTFOUND;
    } while (rtParent(__hCfg->tree, nodeID) == root); /* skip first level */

    /* build full path */

    maxPathLen--;

    do
    {
        cfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);

        len = rpoolChunkSize(__hCfg->pool, cfgVal->name) + 1;
        if ((UINT32)len > maxPathLen)
            return ERR_CI_BUFFERTOOSMALL;

        if (firstTime)
        {
            rpoolCopyToExternal(__hCfg->pool, nextPath, cfgVal->name, 0, len);
            nextPath[len - 1] = '\0';
            firstTime = 0;
        }
        else
        {
            memmove(nextPath + len, nextPath, strlen(nextPath) + 1);
            rpoolCopyToExternal(__hCfg->pool, nextPath, cfgVal->name, 0, len);
            nextPath[len - 1] = '.';
        }

        maxPathLen -= len;
        nodeID = rtParent(__hCfg->tree, nodeID);
    } while (nodeID != root);


    return ERR_CI_NOERROR;

}

RVAPI
ci_errors RVCALLCONV ciSetValue(
        IN  HCFG         hCfg,
        IN  const char * path,      /* full path to nodeID */
        IN  BOOL         isString,
        IN  INT32        value,     /* data for ints, length for strings */
        IN  const char * str        /* null for ints, data for strings */
        )
{
    int nodeID = ciGetNodeID(hCfg, path), newNodeID;
    int len;
    char *dot;
    cfgValue cfgVal;
    pcfgValue pCfgVal;

    if (!path  ||  !path[0])
        return ERR_CI_GENERALERROR;

    if (nodeID < 0)  /* no previous value, make a new node */
    {
        /* values for dummy nodes */
        cfgVal.isString  = FALSE;
        cfgVal.value     = 0;
        cfgVal.str       = NULL;

        if ((nodeID = rtHead(__hCfg->tree, rtRoot(__hCfg->tree))) < 0)
        {
            /* a special case - an empty configuration */            
            dot = strchr(path, '.');
            len = dot? dot - path : strlen(path);

            cfgVal.name = rpoolAllocCopyExternal(__hCfg->pool, path, len);

            newNodeID = rtAddTail(__hCfg->tree, rtRoot(__hCfg->tree), &cfgVal);
            nodeID = newNodeID;
        }

        while (*path)
        {
            dot = strchr(path, '.');
            len = dot? dot - path : strlen(path);
            pCfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);

            while (rpoolChunkSize(__hCfg->pool, pCfgVal->name) != len  ||
                   rpoolCompareExternal(__hCfg->pool, pCfgVal->name, (void*)path, len))
            {
                newNodeID = rtBrother(__hCfg->tree, nodeID);
                if (newNodeID < 0)
                {
                    cfgVal.name = rpoolAllocCopyExternal(__hCfg->pool, path, len);

                    newNodeID = rtAddBrother(__hCfg->tree, nodeID, &cfgVal);
                    nodeID = newNodeID;
                    break;
                }
                nodeID = newNodeID;
                pCfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);
            }

            newNodeID = rtHead(__hCfg->tree, nodeID);
            path = dot + 1;

            /* if not found, insert a dummy value */
            if (newNodeID < 0)
            {
                if (!dot)
                    break;

                dot = strchr(path, '.');
                len = dot? dot - path : strlen(path);
                
                cfgVal.name = rpoolAllocCopyExternal(__hCfg->pool, path, len);

                newNodeID = rtAddTail(__hCfg->tree, nodeID, &cfgVal);
            }

            nodeID = newNodeID;

            if (!dot)
                break;
        }
    }

    pCfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);
    if (pCfgVal == NULL)
        return ERR_CI_GENERALERROR;

    /* free, resize, or leave alone the old string buffer */
    if (pCfgVal->isString)
    {
        if (!isString)
        {
            rpoolFree(__hCfg->pool, pCfgVal->str);
            pCfgVal->str = NULL;
        }
        else
        {
            if (pCfgVal->value != value)
            {
                rpoolFree(__hCfg->pool, pCfgVal->str);
                pCfgVal->str = rpoolAlloc(__hCfg->pool, value);
            }
        }
    }
    else
    {
        if (isString)
        {
            pCfgVal->str = rpoolAlloc(__hCfg->pool, value);
        }
    }

    pCfgVal->isString  = isString;
    pCfgVal->value     = value;

    if (isString)
        rpoolCopyFromExternal(__hCfg->pool, pCfgVal->str, (void *)str, 0, value);

    return ERR_CI_NOERROR;

}

RVAPI
ci_errors RVCALLCONV ciSetBitString(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node */
        IN  INT32        bits,       /* number of bits in the string */
        IN  const char * str         /* null for ints, data for strings */
        )
{
    char buf[1024];
    return
        ciSetValue(hCfg, path, TRUE, ciBuildBitString(str, bits, buf), buf);
}


RVAPI
ci_errors RVCALLCONV ciDeleteValue(
        IN  HCFG         hCfg,
        IN  const char * path
        )
{
    int nodeID = ciGetNodeID(hCfg, path);
    pcfgValue pCfgVal;

    if (nodeID < 0)
          return ERR_CI_NOTFOUND;

    pCfgVal = (pcfgValue)rtGetByPath(__hCfg->tree, nodeID);
    if (pCfgVal && pCfgVal->str)
        rpoolFree(__hCfg->pool, pCfgVal->str);
    if (pCfgVal&& pCfgVal->name)
        rpoolFree(__hCfg->pool, pCfgVal->name);
    if (rtDelete(__hCfg->tree, nodeID, 0, 0) < 0)
        return ERR_CI_GENERALERROR;


    return ERR_CI_NOERROR;

}


#ifdef __cplusplus
}
#endif




