#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#include <stdio.h>
#include <rvinternal.h>
#include <emanag.h>

#define MAX_E_SYSTEMS 3
static emTypeOfEncoding eSystems[MAX_E_SYSTEMS];


int emSetEncoding(
    IN  int                 privateTag,
    IN  emTypeOfEncoding*   encoding)
{
    eSystems[privateTag]=*encoding;
    return 0;
}


int emEncode(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    encoded)
{
    int eSystem;
    int nodeId,tag;
    pstTagClass tagClass;
    HPST synH=pvtGetSynTree(valH, vNodeId);
    pvtGet(valH,vNodeId,NULL,&nodeId,NULL,NULL);
    tag=pstGetTag(synH,nodeId, &tagClass);
    eSystem=(int)(tagClass==pstTagPrivate)?tag:0;
    if (eSystems[eSystem].Encode)
        return eSystems[eSystem].Encode(valH,vNodeId,buffer,length,encoded);
    return 0;
}

int emEncodeExt(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    encoded)
{
    return emEncode(valH,vNodeId,buffer,length,encoded);
}

int emDecode(
    IN  HPVT    valH,
    IN  int     vNodeId,
    OUT BYTE*   buffer,
    IN  int     length,
    OUT int*    decoded)
{
    int ret;
    if ((ret=emDecodeExt(valH,vNodeId,-1,buffer,length,decoded))<0)
        return ret;
    return TRUE;
}

int emDecodeExt(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  INT32   fieldId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    decoded)
{
    int eSystem;
    int nodeId,tag;
    pstTagClass tagClass;
    HPST synH=pvtGetSynTree(valH, vNodeId);

    if (synH == NULL)
    {
        /* No syntax information in the given node id! */
        return RVERROR;
    }

    /* Find out which decoding system we have to use */
    pvtGet(valH,vNodeId,NULL,&nodeId,NULL,NULL);
    tag=pstGetTag(synH,nodeId, &tagClass);
    eSystem=(int)(tagClass==pstTagPrivate)?tag:0;

    if (eSystems[eSystem].Decode)
    {
        /* Decode it */
        return eSystems[eSystem].Decode(valH,vNodeId,fieldId,buffer,length,decoded);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif



