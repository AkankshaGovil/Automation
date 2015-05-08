#ifndef NX_MKDIRP_H
#define NX_MKDIR_H
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*!
   \param p Target string.
   \param s Source string.
   \desc
   Works like strcat, accept inlined, and if you have a faster memmove then
   it might work better with really large strings.
*/

# ifndef addtext
#  define addtext(p,s) { int plen=strlen (p), slen=strlen (s); memmove (p+plen, s, slen); p[plen+slen]='\0'; }
# endif

/*!
   \param t The type to allocate.
   \desc
   Type calloc.
*/
# ifndef tcalloc
#  define tcalloc(t) (t *)calloc (1, sizeof(t))
# endif

/*!
   \param t The type to allocate.
   \param n The number of elements in the array.
   \desc
   Type array calloc.
*/
# ifndef tncalloc
#  define tncalloc(t,n) (t *)calloc (n, sizeof(t))
# endif
/*!
   \param len Length of the string to allocate.
   \desc
   String calloc.
*/
# ifndef scalloc
#  define scalloc(len) (char *)calloc (len, sizeof(char))
# endif

# ifndef TRUE
#  define TRUE			(1 == 1)
# endif
/*!
   Universal False.
*/
# ifndef FALSE
#  define FALSE			(1 == 0)
# endif

#endif /*NX_MKDIRP*/
