#include <string.h>
#include "osdconfig.h"
#include "nxosd.h"


#ifdef HAVE_STRLCPY
 size_t nx_strlcpy(char *d, const char *s, size_t bufsize)
{
  return strlcpy( d, s, bufsize);
}
#else
/*
 * Implementation of strlcpy.
 * like strncpy but does not 0 fill the buffer and always null 
 * terminates. bufsize is the size of the destination buffer 
 * */
  
 size_t nx_strlcpy(char *d, const char *s, size_t bufsize)
{
	size_t len = strlen(s);
	size_t ret = len;
	if (len >= bufsize) len = bufsize-1;
	memcpy(d, s, len);
	d[len] = '\0';
	return ret;
}
#endif
