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
#include <string.h>
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
         )
{
  int i;

  if (!option || strlen(option) ==0) return FALSE;

  for (i=1; i<argc; i++)
    if (*argv[i] == '-' && !strcmp(argv[i]+1, option))
      if (argc > i) {
    if (optionArgument) *optionArgument = argv[i+1];
    return TRUE;
      }

  if (optionArgument) *optionArgument = NULL;
  return FALSE;
}



BOOL /* TRUE if option found FALSE otherwise. */
argGetFile(
       /* Search the corresponding argv[] file that match the postfix
          Format: path/foo.postix
          Usage: "foo.c" <-- argGetOption(argc, argv, "c", &ptr) */
       IN  int argc,      /* process argc */
       IN  char *argv[],  /* process argv */
       IN  char *postfix, /* filename postfix */
       OUT char **argument  /* argument string if found */
       )
{
  int i;
  char *pfix;

  if (!postfix || strlen(postfix)==0) return FALSE;

  for (i=1; i<argc; i++) {
    pfix = strrchr(argv[i], '.');
    if (pfix && !strcmp(pfix+1, postfix)) {
      if (argument) *argument = argv[i];
      return TRUE;
    }
  }

  if (argument) *argument = NULL;
  return FALSE;
}


BOOL /* TRUE if option found FALSE otherwise. */
argGetParamNext(
       /* Search the corresponding argv[] file that match the postfix
          Format: path/foo.postix
          Usage: "foo.c" <-- argGetOption(argc, argv, "c", &ptr) */
       IN  int argc,      /* process argc */
       IN  char *argv[],  /* process argv */
       IN  char *postfix, /* filename postfix */
       OUT char **argument  /* argument string if found */
       )
{
  int i;
  char *pfix;
  static int cur = 1;

  if (!postfix || strlen(postfix)==0) return FALSE;

  for (i=cur; i<argc; i++) {
    pfix = strrchr(argv[i], '.');
    if (pfix && !strcmp(pfix+1, postfix)) {
      if (argument) *argument = argv[i];
      cur = i+1;
      return TRUE;
    }
  }

  if (argument) *argument = NULL;
  return FALSE;
}


#ifdef __cplusplus
}
#endif



