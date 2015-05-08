#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/****************************************************************************

  strutils.h  --  String utilities interface

  Module Authors: Oz Solomonovich, Sasha Ruditsky
  This Comment:   1-Jan-1996

  Abstract:       Various string manipulation utilities.

  Platforms:      All,

  Known Bugs:     None.

****************************************************************************/


#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <rvinternal.h>

/* multi thread safe strtok version */
char *
strtok_mts(char *s, const char *delim, char **lasts);

/* ascii->bmp string - returns bytes written to target buffer */
int chr2bmp(IN char *str, OUT BYTE *bmpStr);

/* same as chr2bmp, with maximum length */
int chrn2bmp(IN char *str, IN int maxStrLen, OUT BYTE *bmpStr);

/* bmp->ascii string - returns RVERROR if conversion was unsuccessful */
int bmp2chr(OUT char *str, IN BYTE *bmpStr, IN int bmpLen);

/* case insensitive strcmp */
int strcmp_ci(const char *dst, const char *src);

/* case insensitive strncmp */
int strncmp_ci(const char *dst, const char *src, unsigned int count);


#endif  /* __STRUTILS_H */

#ifdef __cplusplus
}
#endif



