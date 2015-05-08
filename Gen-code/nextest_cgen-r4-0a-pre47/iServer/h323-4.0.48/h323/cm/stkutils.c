#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/***************************************************************************

  stkutils.c  --  Exported Stack Utilities

  Module Author:  Oz Solomonovich
  This Comment:   20-Jul-1997

  Abstract:       This is an wrapper module for the various stack utilities
                  that we export.

  Platforms:      All.

  Known Bugs:     None.

***************************************************************************/


#include <stkutils.h>
#include <oidutils.h>
#include <strutils.h>


                        /* == OID Manipulation == */

RVAPI
int RVCALLCONV utlEncodeOID(
            OUT   int           oidSize,
            OUT   char *        oid,
            IN    const char *  buff
            )
{
    return oidEncodeOID(oidSize, oid, (char *)buff);
}

RVAPI
int RVCALLCONV utlDecodeOID(
        IN   int            oidSize,
        IN   const char *   oid,
        OUT  int            buffSize,
        OUT  char *         buff,
            IN   OID_form       f
        )
{
    return oidDecodeOID(oidSize, (char *)oid, buffSize, buff, (form)f);
}


                     /* == BMP String Manipulation == */

/* ascii->bmp string - returns bytes written to target buffer */
RVAPI
int RVCALLCONV utlChr2Bmp(
            IN   const char *   str,
            OUT  BYTE *         bmpStr
            )
{
    return chr2bmp((char *)str, bmpStr);
}

/* same as chr2bmp, with maximum length */
RVAPI
int RVCALLCONV utlChrn2Bmp(
            IN   const char *   str,
            IN   int            maxStrLen,
            OUT  BYTE *         bmpStr
            )
{
    return chrn2bmp((char *)str, maxStrLen, bmpStr);
}

/* bmp->ascii string - returns RVERROR if conversion was unsuccessful */
RVAPI
int RVCALLCONV utlBmp2Chr(
            OUT  char *         str,
            IN   const BYTE *   bmpStr,
            IN   int            bmpLen
            )
{
    return bmp2chr(str, (BYTE *)bmpStr, bmpLen);
}
#ifdef __cplusplus
}
#endif



