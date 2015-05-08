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
  token.h

  Ron S. 22 Nov. 1995

  General token definitions for ASN compiler, encoder and users.

*/

#ifndef _TOKENH_
#define _TOKENH_

#include <string.h>
#include <rvinternal.h>


typedef struct {
  char *name;
  int ID;
} tokenT;


/* Returns the ID of str as found in the token table or -1 */
int   GetTokenID(tokenT *ar, char *str);

char *GetTokenName(tokenT *ar, int ID);
int   GetTokenInstance(tokenT *ar, char *str, BOOL *match);
int   GetTokenSize(tokenT *ar);

#endif
#ifdef __cplusplus
}
#endif



