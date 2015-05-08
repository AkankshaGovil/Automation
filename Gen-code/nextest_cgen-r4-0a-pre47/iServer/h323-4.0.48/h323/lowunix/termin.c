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
  terminate.c

  Ron S.
  25 July 1996

  Program termination routine
  */

#include <stdio.h>

void terminateProgram(void)
{
  /*static void (*f)()=(void *)0x3;
  static char *p=(void *)0x3;*/

  fprintf(stderr, "Virtual termination...\n");
  /*  *p=0;
  f();*/
}
#ifdef __cplusplus
}
#endif



