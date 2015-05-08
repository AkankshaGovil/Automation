/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  cisupp.h  --  CI helper functions interface

  Module Author: Oz Solomonovich
  This Comment:  18-Dec-1996

****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __CISUPP_H
#define __CISUPP_H

#include <rtree.h>
#include <rpool.h>

#define CI_BITSTR_ID        "!CIBITSTR!"
#define CI_BITSTR_ID_LEN    10

typedef struct
{
  void *name;                       /* this is an RPOOL pointer */
  BOOL isString;
  INT32 value;                      /* length of 'str' or integer value */
  void *str;                        /* this is an RPOOL pointer */
} cfgValue, *pcfgValue;


typedef struct
{
	HRTREE tree;
	HRPOOL pool;
} cfgHandle;

#define __hCfg  ((cfgHandle *)(hCfg))


#define CONFIG_RPOOL_BLOCK_SIZE 32
#define MAX_CONFIG_TEMP_BUFFER_SIZE 512


/* helper functions to indentify a data source (i.e. file, registry) */
typedef int (ciIDFunc)      (const char *source);
/* helper functions to estimate the size of the data in the data source  */
typedef int (ciEstimateFunc)(const char *source, int *nodes, int *data);
/* helper functions to load the data */
typedef int (ciBuildFunc)   (const char *source, HRTREE tree, HRPOOL pool);
/* helper functions to save the data */
typedef int (ciOutputFunc)  (const char *target, HRTREE tree, HRPOOL pool);


                /* == interface to load/save functions == */

/* estimate minimum amouts of: */
void
ciEstimateCfgSize(const char *source,  /* config source identifier */
                  int *nodes,          /* number of nodes */
                  int *data            /* size of all data */
                  );

/* bulid a configuration R-Tree from the source */
HRTREE
ciBuildRTree(const char *source,        /* config source identifier */
             int nodes,                 /* number of nodes from estimate */
             HRPOOL pool,               /* preallcated pool for data */
             RVHLOGMGR logMgr);         /* log manager to use */

/* output the configuration to the target */
int
ciOutputRTree(const char *target,      /* target for tree */
              HRTREE tree,             /* configuration tree*/
              HRPOOL pool              /* preallcated pool for data */
              );


/* == bit string support == */

/* builds an internal representation of a bit string */
int /* returns: the length of the output string */
ciBuildBitString(const char *str,      /* the bit string buffer */
                 int bits,             /* the number of bits in the buffer */
                 OUT char *bitstr      /* output buffer - can be NULL */
                 );

/* checks if a string is a bit string buffer */
int /* returns: <0 - not bit string, otherwise number of bits */
ciIsBitString(const char *str,         /* string to check */
              int length               /* length of string to check */
              );

/* returns the data section inside the bit string buffer */
const char *ciGetBitStringData(const char *str);


#endif /* __CISUPP_H */
#ifdef __cplusplus
}
#endif



