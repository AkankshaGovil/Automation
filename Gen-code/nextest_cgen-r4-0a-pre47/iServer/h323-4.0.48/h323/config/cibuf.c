/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  cibuf.c  --  Configuration load/save functions - buffer version

  Module Author:  Oz Solomonovich
  This Comment:   27-May-1997

  Abstract:       CI routines for loading and saving the configuration to
                  and from buffers.

                  The module supplies routines for creating an managing
                  these buffers, as exported by cibuf.h.

  Platforms:      Supports all platforms.  Integer data is always output
                  in network order.

  Known Bugs:     None.

***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>

#include <rvinternal.h>
#include <rtree.h>
#include <rpool.h>
#include <netutl.h>

#include "ci.h"
#include "cisupp.h"
#include "cibuf.h"


static void do_ntohl(UINT32 *netlong);
static void do_htonl(UINT32 *hostlong);


/* the buffer header - DONT USE A STRUCT because byte allignment
   could give wrong offset calculations:

     0               4               8
   +---+---+---+---+---+---+---+---+---+---+---+---+
   | ID STRING                     | SIZE OF DATA  |
   +---+---+---+---+---+---+---+---+---+---+---+---+

    12              16              20
   +---+---+---+---+---+---+---+---+---+---+---+---+ ...
   | # NODES       | STR DATA SIZE | DATA
   +---+---+---+---+---+---+---+---+---+---+---+---+ ...

   all int values in network order.
*/

/* CI_BUF_XXX_OFFSET macros define the offset of field XXX in the buffer */
#define CI_BUF_SIZE_OFFSET       8                             /* UINT32 */
#define CI_BUF_NODES_OFFSET      (CI_BUF_SIZE_OFFSET    + 04)  /* UINT32 */
#define CI_BUF_STRSIZE_OFFSET    (CI_BUF_NODES_OFFSET   + 04)  /* UINT32 */
#define CI_BUF_DATA_OFFSET       (CI_BUF_STRSIZE_OFFSET + 04)  /* UINT32 */

/* CI_BUF_XXX() macros etract the XXX field from a buffer */
#define CI_BUF_SIZE(buf)         (*(UINT32 *)(buf + CI_BUF_SIZE_OFFSET))
#define CI_BUF_NODES(buf)        (*(UINT32 *)(buf + CI_BUF_NODES_OFFSET))
#define CI_BUF_STRSIZE(buf)      (*(UINT32 *)(buf + CI_BUF_STRSIZE_OFFSET))
#define CI_BUF_DATA(buffer)      (buffer + CI_BUF_DATA_OFFSET)



/* the data - formatted as below.  The structure is NEVER read or written
   as a whole in order to overcome compiler allignment calculations:
    INT32  level;
    BOOL   isString;
    INT32  value;
    UINT32 nameLen;
    name data
    string data (optional)
*/
typedef struct
{
  INT32  level;
  INT8   isString;
  INT32  value;
  UINT32 nameLen;
} bufInfoBlock;



static void buildFromBuffer(const char **pos, const char *end, HRPOOL pool,
                            HRTREE tree, int nodeID, int level);

static int outputToBuffer(char **pos, char *end, HRTREE tree, int nodeID,
                          int level, UINT32 *nodes, UINT32 *dataSize,
                          int estimateOnly, HRPOOL pool);



                  /* == Exported Functions (cibuf.h) == */

RVAPI
INT32 RVCALLCONV ciTargetBufferSize(IN HCFG hCfg)
{
    int a=0, b=0, retVal;
    char *pos = 0;

    if ((retVal = outputToBuffer(&pos, 0, __hCfg->tree, rtRoot(__hCfg->tree),
        0, (UINT32*)&a, (UINT32*)&b, 1, __hCfg->pool)) < 0)
        return ERR_CI_GENERALERROR;

    return (INT32)(pos - (char *)0) + CI_BUF_DATA_OFFSET;
}

RVAPI
int RVCALLCONV ciPrepareTargetBuffer(IN OUT   void *  buffer,
                                   IN       INT32   bufferSize)
{
    if (bufferSize < (INT32)(CI_BUF_DATA_OFFSET + sizeof(bufInfoBlock)))
        return ERR_CI_BUFFERTOOSMALL;

    strcpy((char*)buffer, CI_BUF_ID_STRING);
    CI_BUF_SIZE((char *)buffer) = rv_htonl(bufferSize);

    return 0;
}

RVAPI
int RVCALLCONV ciAllocateTargetBuffer(IN  HCFG    hCfg,
                                    OUT void ** buffer,
                                    OUT int *   bufferSize
                                    )
{
    int size = ciTargetBufferSize(hCfg), err;

    *buffer = 0;

    if (bufferSize)
    {
        *bufferSize = 0;
    }

    if (size < 0)
        return size;

    if (!(*buffer = (void *)malloc(size)))
        return ERR_CI_ALLOCERROR;

    if ((err = ciPrepareTargetBuffer(*buffer, size)) < 0)
    {
        free(*buffer);
        *buffer = 0;
        return err;
    }

    *bufferSize = size;

    return 0;
}

RVAPI
int RVCALLCONV ciFreeTargetBuffer(IN  HCFG    hCfg,
                                IN  void *  buffer
                                )
{
  if(hCfg);
    free(buffer);

    return 0;
}

                      /* == CI Interface Functions == */

int ciIDBuffer(const char *source)
{
    return (!(strncmp(source, CI_BUF_ID_STRING, CI_BUF_ID_LEN)));
}


int ciEstimateCfgSizeBuffer(const char *source, int *nodes, int *data)
{
    *nodes = (int)(3*rv_ntohl(CI_BUF_NODES(source)));
    *data  = (int)(3*rv_ntohl(CI_BUF_STRSIZE(source)));

    return 0;
}

int ciBuildFromBuffer(const char *source, HRTREE tree, HRPOOL pool)
{
    UINT32 bufSize;
    const char *end;
    const char *pos;
    UINT32 temp;

    /* The next 2 statements together is supposed to be equivalent to the
       original statement bufSize  = rv_ntohl(CI_BUF_SIZE(source)), but
       without the integer alignment problem. In the original statement,
       if source is not aligned with an integer. then parameter
       CI_BUF_SIZE(source) may be an unexpected value to rv_ntohl.
    */
    memcpy(&temp, source + CI_BUF_SIZE_OFFSET, sizeof(UINT32));
    bufSize = rv_ntohl(temp);

    end = source + bufSize;
    pos = CI_BUF_DATA(source);

    buildFromBuffer(&pos, end, pool, tree, rtRoot(tree), 0);

    return 0;
}


int ciOutputToBuffer(const char *target, HRTREE tree, HRPOOL pool)
{
    UINT32 *bufSize = &CI_BUF_SIZE(target);
    char *end       = (char *)target + rv_ntohl(*bufSize);
    char *pos       = CI_BUF_DATA((char *)target);
    INT32 retVal;

    if ((end - pos) < CI_BUF_DATA_OFFSET)
        return ERR_CI_BUFFERTOOSMALL;

    CI_BUF_NODES  (target) = 0;
    CI_BUF_STRSIZE(target) = 0;

    if ((retVal = outputToBuffer(&pos, end, tree, rtRoot(tree), 0,
        &CI_BUF_NODES(target), &CI_BUF_STRSIZE(target), 0, pool)) < 0)
        return retVal;

    do_htonl(&CI_BUF_NODES  (target));
    do_htonl(&CI_BUF_STRSIZE(target));

    *bufSize = rv_htonl((UINT32)((char *)pos - (char *)target));

    return 0;
}

static void bufferReadRpool(const char **bufPtr, HRPOOL rpool, void *dest, int size)
{
    rpoolCopyFromExternal(rpool, dest, (void *)*bufPtr, 0, size);
    *bufPtr += size;
}

static void bufferRead(const char **bufPtr, char *dest, int size)
{
    memcpy(dest, *bufPtr, size);
    *bufPtr += size;
}

static int bufferWrite(char **bufPtr, char *end, char *src, int size)
{
    if (*bufPtr + size - 1 > end)
        return ERR_CI_BUFFERTOOSMALL;

    memcpy(*bufPtr, src, size);
    *bufPtr += size;

    return 0;
}

static void buildFromBuffer(const char **pos, const char *end, HRPOOL pool,
                            HRTREE tree, int nodeID, int level)
{
    bufInfoBlock info;
    int newNodeID;
    cfgValue cfgVal;
    const char *lastPos;

    while (*pos < end)
    {
        lastPos = *pos;

        bufferRead(pos, (char *)&info.level, sizeof(info.level));
        do_ntohl((UINT32*)&info.level);

        if (info.level < level)
        {
            /* a higher level, we must exit */
            *pos = lastPos;
            return;
        }

        bufferRead(pos, (char *)&info.isString, sizeof(info.isString));
        bufferRead(pos, (char *)&info.value,    sizeof(info.value));
        bufferRead(pos, (char *)&info.nameLen,  sizeof(info.nameLen));
        do_ntohl((UINT32*)&info.value);
        do_ntohl(&info.nameLen);

        cfgVal.isString = info.isString;
        cfgVal.value    = info.value;
        cfgVal.name     = rpoolAlloc(pool, info.nameLen);
        bufferReadRpool(pos, pool, cfgVal.name, (int)(info.nameLen));
        if (cfgVal.isString)
        {
            cfgVal.str = rpoolAlloc(pool, cfgVal.value);
            bufferReadRpool(pos, pool, cfgVal.str, cfgVal.value);
        }
        else
            cfgVal.str = NULL;

        if (level == info.level)
        {
            newNodeID = rtAddBrother(tree, nodeID, &cfgVal);
            nodeID = newNodeID;
        }
        else
            newNodeID = rtAddTail(tree, nodeID, &cfgVal);

        if (nodeID != newNodeID)  /* nest to next level */
            buildFromBuffer(pos, end, pool, tree, newNodeID, level + 1);
    }
}

#define WRITE_AND_CHECK(dest, size) \
  { \
    if (estimateOnly) \
      (*pos) += size; \
    else \
      if ((retVal = bufferWrite(pos, end, dest, size)) < 0) \
        return retVal; \
  }

static int outputToBuffer(char **pos, char *end, HRTREE tree, int nodeID,
                          int level, UINT32 *nodes, UINT32 *dataSize,
                          int estimateOnly, HRPOOL pool)
{
    pcfgValue cfgVal;
    bufInfoBlock info;
    int child, retVal;
    char buff[MAX_CONFIG_TEMP_BUFFER_SIZE];

    for (;;)
    {
        if (level != 0)
        {
            cfgVal = (pcfgValue)rtGetByPath(tree, nodeID);

            info.level    = rv_htonl(level);
            info.isString = (char)(cfgVal->isString != 0);
            info.value    = rv_htonl(cfgVal->value);
            info.nameLen  = rv_htonl(rpoolChunkSize(pool, cfgVal->name));

            WRITE_AND_CHECK((char *)&info.level,    sizeof(info.level));
            WRITE_AND_CHECK((char *)&info.isString, sizeof(info.isString));
            WRITE_AND_CHECK((char *)&info.value,    sizeof(info.value));
            WRITE_AND_CHECK((char *)&info.nameLen,  sizeof(info.nameLen));
            rpoolCopyToExternal(pool, (void*)buff, cfgVal->name, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
            buff[rpoolChunkSize(pool, cfgVal->name)] = 0;
            WRITE_AND_CHECK(buff, (int)rv_ntohl(info.nameLen));

            if (info.isString)
            {
                rpoolCopyToExternal(pool, (void*)buff, cfgVal->str, 0, MAX_CONFIG_TEMP_BUFFER_SIZE);
                buff[rpoolChunkSize(pool, cfgVal->str)] = 0;
                WRITE_AND_CHECK(buff, cfgVal->value);
                *dataSize += cfgVal->value;
            }

            (*nodes)++;
            *dataSize += rv_ntohl(info.nameLen) + 1;
        }

        /* process children */
        child = rtHead(tree, nodeID);
        if (child >= 0)
        {
            retVal = outputToBuffer(pos, end, tree, child, level + 1, nodes,
                dataSize, estimateOnly, pool);
            if (retVal < 0)
                return retVal;
        }

        /* move to brother */
        nodeID = rtBrother(tree, nodeID);
        if (nodeID < 0)
            return 0;
    }
}


static void do_ntohl(UINT32 *netlong)
{
    *netlong = rv_ntohl(*netlong);
}

static void do_htonl(UINT32 *hostlong)
{
    *hostlong = rv_htonl(*hostlong);
}
#ifdef __cplusplus
}
#endif



