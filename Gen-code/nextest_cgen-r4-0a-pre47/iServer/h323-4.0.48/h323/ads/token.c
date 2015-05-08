#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  token.c

  Ron S. 22 Nov. 1995

  Utils functions for handling static map.
  The map is in the form:
  - string
  - id

  Converting enumerations into strings for displaying purpose.

  */


#include <stdio.h>
#include <token.h>


/*______________________________get_token_______________________________*/
char *
GetTokenName(tokenT *ar, int ID)
{
  int i=0;

  if (!ar) return NULL;

  while (ar[i].name) {
    if (ar[i].ID == ID)
      return ar[i].name;
    i++;
  }

  return (char*)"NO TOKEN";
}


int
GetTokenSize(tokenT *ar)
{
  int i=0;
  if (!ar) return RVERROR;
  while (ar[i++].name);
  return i-1;
}




/* Returns the ID of str as found in the token table or -1 */
int
GetTokenID(tokenT *ar, char *str)
{
  int pos=0;

  if (!ar) return -1;

  while (ar[pos].name) {
    if (!strcmp(str, ar[pos].name))
      return ar[pos].ID;
    pos++;
  }

  return -1;
}


#if 0
static int /* true if prefix of str */
strPrefix(
      IN  char *str,
      IN  char *prefix,
      OUT int *count /* number of match characters */
      )
{
  int i, _count=0;
  int ret=TRUE;

  for (i=0; *(str+i); i++) {
    if (!*(prefix+i)) break;
    if ( *(str+i) == *(prefix+i)) _count++;
    else {ret=RVERROR; break; }
  }

  if (*(prefix+i)) ret=RVERROR; /* str smaller than prefix */
  if (*count) *count = _count;
  return ret;
}
#endif



/* Returns the number of instances where str apears in 'ar' as
   the prefix of the token
   OUT match: TRUE if str is complete match to a single token.
   E.g: ar = "..", "...".
        str = "."
    ==> result = 2

   */
int
GetTokenInstance(tokenT *ar, char *str, BOOL *match)
{
  int pos=0;
  int count=0;

  if (!ar || !str || !match) return -1;
  if (strlen(str) == 0) return 0;
  *match = FALSE;

  /*
  while (ar[pos].name) {
    if (strPrefix(ar[pos].name, str, NULL) == TRUE) {
      count++;
      if (!strcmp(ar[pos].name, str)) *match=TRUE;
    }
    pos++;
  }
  */


  while (ar[pos].name) {
    if (strstr(ar[pos].name, str) == ar[pos].name) {
      count++;
      if (!strcmp(ar[pos].name, str)) *match=TRUE;
    }
    pos++;
  }

  return count;
}

#ifdef __cplusplus
}
#endif



