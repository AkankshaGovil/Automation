#include <string.h>
#include "osdconfig.h"
#include "nxosd.h"

#ifdef HAVE_STRLCAT
/*
  nx_strlcat simply wraps around strlcat
*/
size_t nx_strlcat(char *d, const char *s, size_t bufsize)
{
  return strlcat( d, s, bufsize);
}

#else
/* like strncat but does not 0 fill the buffer and always null 
   terminates. bufsize is the length of the buffer, which should
   be one more than the maximum resulting string length */
 size_t nx_strlcat(char *d, const char *s, size_t bufsize)
{
	size_t len1 = strlen(d);
	size_t len2 = strlen(s);
	size_t ret = len1 + len2;

	if (len1+len2 >= bufsize) {
		len2 = bufsize - (len1+1);
	}
	if (len2 > 0) {
		memcpy(d+len1, s, len2);
		d[len1+len2] = '\0';
	}
	return ret;
}
#endif
