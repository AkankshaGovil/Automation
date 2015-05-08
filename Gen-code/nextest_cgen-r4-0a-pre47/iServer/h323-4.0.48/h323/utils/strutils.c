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

  strutils.c  --  String Utilities

  Module Authors: Oz Solomonovich, Sasha Ruditsky
  This Comment:   1-Jan-1997

  Abstract:       Various string manipulation utilities.

  Platforms:      All.

  Known Bugs:     None.

****************************************************************************/


#include <ctype.h>

#include <rvinternal.h>

#include "strutils.h"


char *
strtok_mts(char *s, const char *delim, char **lasts)
{
        char *spanp;
        int c, sc;
        char *tok;

        if (s == NULL && (s = *lasts) == NULL)
                return (NULL);

        /*
         * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
         */
cont:
        c = *s++;
        for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
                if (c == sc)
                        goto cont;
        }

        if (c == 0) {           /* no non-delimiter characters */
                *lasts = NULL;
                return (NULL);
        }
        tok = s - 1;

        /*
         * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
         * Note that delim must have one NUL; we stop if we see that, too.
         */
        for (;;) {
                c = *s++;
                spanp = (char *)delim;
                do {
                        if ((sc = *spanp++) == c) {
                                if (c == 0)
                                        s = NULL;
                                else
                                        s[-1] = 0;
                                *lasts = s;
                                return (tok);
                        }
                } while (sc != 0);
        }
        /* NOTREACHED */
}
int chrn2bmp(char *str, int maxStrLen, BYTE* bmpStr)
{
    int i, i2;

    for (i = 0, i2 = 0; i < maxStrLen; i++, i2 += 2)
    {
      bmpStr[i2 + 0] = 0;
      bmpStr[i2 + 1] = str[i];
    }

    return i2;
}

int chr2bmp(char *str, BYTE* bmpStr)
{
    return chrn2bmp(str, (int)strlen(str), bmpStr);
}

int bmp2chr(char *str, BYTE* bmpStr, int bmpLen)
{
    int i, i2;

    for (i = 0, i2 = 0; i < bmpLen/2; i++, i2 += 2)
    {
      if (bmpStr[i2])
      {
        str[i] = 0;
        return RVERROR;
      }
      str[i] = bmpStr[i2 + 1];
    }

    str[i] = 0;
    return 0;
}

/* case insensitive strcmp */
int strcmp_ci(const char *dst, const char *src)
{
  int f, l;

  do
  {
    f = tolower(*dst);
    l = tolower(*src);
    dst++;
    src++;
  } while (f && f == l);

  return(f - l);
}


/* case insensitive strncmp */
int strncmp_ci(const char *dst, const char *src, unsigned int count)
{
  int f, l;
  int result = 0;

  if (count)
  {
    do
    {
      f = tolower(*dst);
      l = tolower(*src);
      dst++;
      src++;
    } while (--count && f && l && f == l);

    result = f - l;
  }
  return(result);
}


#ifdef __cplusplus
}
#endif



