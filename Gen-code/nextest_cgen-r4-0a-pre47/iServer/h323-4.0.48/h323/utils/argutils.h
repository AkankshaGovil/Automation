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

#ifndef _ARGUTILS_
#define _ARGUTILS_

#include <rvinternal.h>


BOOL /* TRUE if option found FALSE otherwise.*/
argGetOption(
         /* Search the corresponding argv[] option string.
        Format: -o option
        Usage: "option" <-- argGetOption(argc, argv, "o", &ptr)  */
         IN  int argc,      /* process argc */
         IN  char *argv[],  /* process argv */
         IN  char *option,  /* option name (without '-' sign) */
         OUT char **optionArgument  /* argument after option if found */
         );


BOOL /* TRUE if option found FALSE otherwise. */
argGetFile(
       /* Search the corresponding argv[] file that match the postfix
          Format: path/foo.postix
          Usage: "foo.c" <-- argGetOption(argc, argv, "c", &ptr) */
       IN  int argc,      /* process argc */
       IN  char *argv[],  /* process argv */
       IN  char *postfix, /* filename postfix */
       OUT char **argument  /* argument string if found */
       );

BOOL /* TRUE if option found FALSE otherwise. */
argGetParamNext(
       /* Search the corresponding argv[] file that match the postfix
          Format: path/foo.postix
          Usage: "foo.c" <-- argGetOption(argc, argv, "c", &ptr) */
       IN  int argc,      /* process argc */
       IN  char *argv[],  /* process argv */
       IN  char *postfix, /* filename postfix */
       OUT char **argument  /* argument string if found */
       );
#endif
#ifdef __cplusplus
}
#endif



