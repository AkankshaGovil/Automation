#include <signal.h>
#define _GNU_SOURCE
#include <string.h>
#include "osdconfig.h"
#include "nxosd.h"
#define INVALID_SIGSTR "Unknown signal"

#if HAVE_SIG2STR
int nx_sig2str(int signum, char *str, size_t buflen) 
{
  // ignore buflen 
  return sig2str( signum, str);
}
#elif HAVE_STRSIGNAL
/* 
 get signal description using strsignal.
 Copy the description to the buffer provided and avoid 
 overflow by using nx_strlcpy
*/
int nx_sig2str(int signum, char *str, size_t buflen)
{
  char *sigstr;
  int ret;

  sigstr = strsignal(signum);

  // On linux, strsignal returns Unknown signal <signum> 
  //if signum is not valid
   if ( sigstr != NULL && 
       strncmp( sigstr, INVALID_SIGSTR, strlen( INVALID_SIGSTR)) != 0
      )
    {
      // valid signal
      ret = 0;
      // copy signal description to buffer
      nx_strlcpy( str, sigstr, buflen); 
         
    }
    else
    {
      // invalid signal
      ret = -1;
    }

  return ret;

}
#else
#error no fallback implementation of sig2str found
#endif
