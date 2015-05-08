/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced IN  any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/****************************************************************************

  ci.h  --  Configuration Interface Header

  Module Author: Oz Solomonovich
  This Comment:  04-Feb-1997

****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef __CI_H
#define __CI_H


#include <rvcommon.h>

DECLARE_OPAQUE(HCFG);

typedef enum
{
     ERR_CI_NOERROR          = 0,     /* no error */

     ERR_CI_GENERALERROR     = -1,    /* general error */
     ERR_CI_BUFFERTOOSMALL   = -2,    /* return buffer was too small */
     ERR_CI_NOTFOUND         = -3,    /* the value was not found */
     ERR_CI_INTERFACEUNKNOWN = -4,    /* can't ID input/output source */
     ERR_CI_ALLOCERROR       = -5,    /* couldn't allocate memory */

     ERR_CI_REGOPENKEY       = -10,   /* couldn't open registry key */
     ERR_CI_REGCREATE        = -11,   /* couldn't create registry key */

     ERR_CI_FILEOPEN         = -20,   /* couldn't open file */
     ERR_CI_FILEREAD         = -21,   /* error reading from file */
     ERR_CI_FILEWRITE        = -22    /* error writing to file */
} ci_errors;

typedef enum
{
    ci_str_not               = 0,     /* not a string */
    ci_str_regular           = 1,     /* a regular string */
    ci_str_bit               = 2      /* a bit string */
} ci_str_type;


RVAPI
HCFG RVCALLCONV ciConstruct(
        IN  const char * source      /* configuration source identifier */
        );


RVAPI
HCFG RVCALLCONV ciConstructEx(
        IN  const char * source,     /* configuration source identifier */
        IN  int          extraData,  /* maximum amount of data growth */
        IN  int          extraNodes  /* maximum number of nodes growth */
        );

RVAPI
ci_errors RVCALLCONV ciDestruct(
        IN  HCFG hCfg
        );


RVAPI
ci_errors RVCALLCONV ciSave(
        IN  HCFG hCfg,
        IN  char *target
        );


RVAPI
ci_errors RVCALLCONV ciGetValue(
        IN   HCFG         hCfg,
        IN   const char * path,      /* full path to node, i.e. "a.b.c" */
        OUT  BOOL *       isString,
        OUT  INT32 *      value      /* data for ints, length for strings */
        );


RVAPI
ci_errors RVCALLCONV ciGetValueExt(
        IN   HCFG         hCfg,
        IN   const char  *path,      /* full path to node, i.e. "a.b.c" */
        OUT  ci_str_type *strType,
        OUT  INT32 *      value      /* data for ints, length for strings */
        );


RVAPI
ci_errors RVCALLCONV ciGetString(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node, i.e. "a.b.c" */
        OUT char *       str,
        IN  UINT32       maxStrLen   /* length of output string buffer */
        );


RVAPI
ci_errors RVCALLCONV ciGetBitString(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node, i.e. "a.b.c" */
        OUT char *       str,
        IN  UINT32       maxStrLen,  /* length of output string buffer */
        OUT UINT32 *     bits
        );


RVAPI      /* returns ERR_CI_NOTFOUND when there is no 'next' value */
ci_errors RVCALLCONV ciNext(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node, or NULL to start
                                        at first node */
        OUT char *       nextPath,   /* buffer for next path IN cfg */
        IN  UINT32       maxPathLen  /* length of output buffer */
        );


RVAPI
ci_errors RVCALLCONV ciSetValue(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node */
        IN  BOOL         isString,
        IN  INT32        value,      /* data for ints, length for strings */
        IN  const char * str         /* null for ints, data for strings */
        );


RVAPI
ci_errors RVCALLCONV ciSetBitString(
        IN  HCFG         hCfg,
        IN  const char * path,       /* full path to node */
        IN  INT32        bits,       /* number of bits in the string */
        IN  const char * str         /* null for ints, data for strings */
        );


RVAPI
ci_errors RVCALLCONV ciDeleteValue(
        IN  HCFG hCfg,
        IN  const char * path
        );



#endif /* __CI_H */
#ifdef __cplusplus
}
#endif



