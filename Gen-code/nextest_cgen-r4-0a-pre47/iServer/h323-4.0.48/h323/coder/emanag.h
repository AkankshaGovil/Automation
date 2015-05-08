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

#ifndef EMANAG_H
#define EMANAG_H

#include <pvaltree.h>

typedef enum {
  emPER,
  emQ931,
  emBER
} emPrivateTags;


typedef struct
{
    int (*Encode)(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    encoded);
    int (*Decode)(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  INT32   filedId,
    OUT BYTE*   buffer,
    IN  int     length,
    OUT int*    decoded);
} emTypeOfEncoding;


int emSetEncoding(
    IN  int                 privateTag,
    IN  emTypeOfEncoding*   encoding);

int emEncode(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    encoded);

int emEncodeExt(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    encoded);

int emDecode(
    IN  HPVT    valH,
    IN  int     vNodeId,
    OUT BYTE*   buffer,
    IN  int     length,
    OUT int*    decoded);

int emDecodeExt(
    IN  HPVT    valH,
    IN  int     vNodeId,
    IN  INT32   fieldId,
    IN  BYTE*   buffer,
    IN  int     length,
    OUT int*    decoded);

#endif
#ifdef __cplusplus
}
#endif



