/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  cisupp.c  --  CI helper functions interface

  Module Author: Oz Solomonovich
  This Comment:  26-Dec-1996

  Abstract:      Builds the configuration tree from a configuration source.

  Platforms:     File and buffer interfaces are supported for all platforms.
                 Registry support is added for WIN32.
                 See indevidual interface source files for more detailed
                 platform notes.

  Known Bugs:    None.

***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#include <rvinternal.h>
#include <rtree.h>
#include <rpool.h>
#include <ms.h>

#include "ci.h"
#include "cisupp.h"

typedef struct
{
  ciIDFunc       *idFunc;
  ciEstimateFunc *estimateFunc;
  ciBuildFunc    *buildFunc;
  ciOutputFunc   *outputFunc;
} interfaceFuncs;


                      /* == Interface Definitions == */

#undef DECLARE_INTERFACE

#define DECLARE_INTERFACE(n)                                                \
    int ciID##n             (const char *source);                           \
    int ciEstimateCfgSize##n(const char *source, int *nodes, int *data);    \
    int ciBuildFrom##n      (const char *source, HRTREE tree, HRPOOL pool); \
    int ciOutputTo##n       (const char *source, HRTREE tree, HRPOOL pool);

#define DEFINE_INTERFACE(name)                                             \
    { ciID##name, ciEstimateCfgSize##name,                                 \
      ciBuildFrom##name, ciOutputTo##name }                                \

/* declare external functions */
#ifndef UNDER_CE
DECLARE_INTERFACE(Registry)
#endif
DECLARE_INTERFACE(Buffer)
DECLARE_INTERFACE(File)

/* cisupp will try to use the interfaces in the order they appear here: */
interfaceFuncs interfaces[] =
{
#ifndef NOFILESYSTEM
#ifdef WIN32
#ifndef UNDER_CE
  /* Registry interface */  DEFINE_INTERFACE(Registry),
#endif
#endif
#endif
  /* Buffer interface   */  DEFINE_INTERFACE(Buffer),
#ifndef NOFILESYSTEM
  /* File interface     */  DEFINE_INTERFACE(File)
#endif
};


static int getInterface(const char *source)
{
    int i;

    for (i = 0; i < (int)(sizeof(interfaces) / sizeof(interfaceFuncs)); i++)
    {
        if (interfaces[i].idFunc(source))
            return i;
    }

    return ERR_CI_INTERFACEUNKNOWN;
}


void ciEstimateCfgSize(const char *source, int *nodes, int *data)
{
    int iface;

    *nodes = ERR_CI_GENERALERROR;
    *data  = ERR_CI_GENERALERROR;

    if ((iface = getInterface(source)) < 0) return;

    interfaces[iface].estimateFunc(source, nodes, data);
    return;
}


HRTREE ciBuildRTree(const char *source, int nodes, HRPOOL pool, RVHLOGMGR logMgr)
{
    HRTREE tree;
    int root, iface;

    tree = rtConstruct(sizeof(cfgValue), nodes + 1, logMgr, "CI tree");
    root = rtAddRoot(tree, NULL);  /* a root for all configs */

    if (source != NULL)
    {
        if ((iface = getInterface(source)) < 0)
            return NULL;

        if (interfaces[iface].buildFunc(source, tree, pool) <0)
            return NULL;
    }

    return tree;
}


int ciOutputRTree(const char *target, HRTREE tree, HRPOOL pool)
{
    int iface;

    if ((iface = getInterface(target)) < 0)
        return iface;

    return interfaces[iface].outputFunc(target, tree, pool);
}


/* == bit string support */

int
ciBuildBitString(const char *str, int bits, OUT char *bitstr)
{
    int len = (bits + 7) / 8;

    if (bitstr)
    {
        memcpy(bitstr, CI_BITSTR_ID, CI_BITSTR_ID_LEN);
        bitstr[CI_BITSTR_ID_LEN + 0] = (char)((bits & 0xFF00) >> 8);
        bitstr[CI_BITSTR_ID_LEN + 1] = (char)(bits & 0xFF);
        memcpy(bitstr + CI_BITSTR_ID_LEN + 2, str, len);
    }

    return len + CI_BITSTR_ID_LEN + 2;
}

int
ciIsBitString(const char *str, int length)
{
    if (length < (CI_BITSTR_ID_LEN + 2))
        return -1;

    if (memcmp(str, CI_BITSTR_ID, CI_BITSTR_ID_LEN) != 0)
        return -1;

    return (((int)str[CI_BITSTR_ID_LEN]) << 8) + str[CI_BITSTR_ID_LEN + 1];
}

/* returns the data section inside the bit string buffer */
const char *ciGetBitStringData(const char *str)
{
    return str + CI_BITSTR_ID_LEN + 2;
}



#ifdef __cplusplus
}
#endif



