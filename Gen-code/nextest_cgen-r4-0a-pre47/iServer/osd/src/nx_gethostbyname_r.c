#include "osdconfig.h"
#include "nxosd.h"
#if STDC_HEADERS 
#include <stddef.h>
#endif
#if HAVE_FUNC_GETHOSTNAMEBYNAME_R_6
#include <stdio.h>
#endif



#ifdef HAVE_FUNC_GETHOSTBYNAME_R_6
/* 
 * Implementation of nx_gethostbyname_r using gethostbyname_r 
 * which uses six arguments (Linux). 
 * 
 * */
struct hostent* nx_gethostbyname_r (const char *name, struct hostent *result, 
char *buffer, int buflen, int *h_errnop)
{
  struct hostent* res;
  gethostbyname_r( name, result, buffer, buflen, &res, h_errnop);
  return res;
}

#elif HAVE_FUNC_GETHOSTBYNAME_R_5
struct hostent* nx_gethostbyname_r (const char *name, struct hostent *result, 
char *buffer, int buflen, int *h_errnop)
{
  
  return gethostbyname_r( name, result, buffer, buflen, h_errnop);
 
}
#else
#error re-entrant gethostbyname not found
#endif


