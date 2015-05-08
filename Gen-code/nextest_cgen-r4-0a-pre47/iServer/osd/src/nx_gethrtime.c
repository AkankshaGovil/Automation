#include <sys/time.h>
#include "nxosd.h"
#define  NSECS_IN_SEC 1000000000LL
#define  NSECS_IN_USEC 1000LL

#if HAVE_GETHRTIME
hrtime_t nx_gethrtime()
{
  return gethrtime();
}
#else
/*
 * Fallback implementation of gethrtime(). 
 * Warning : The function does not guarantee unique values if two calls
 * are sufficiently proximate.The implementation can at a maximum provide resolution 
 * of one microsecond.The time is also subject to drifting and change 
 * if settimeofday is used  on the target machine to change the time.
 * */
hrtime_t nx_gethrtime()
{
  struct timeval tval;
  hrtime_t hrt;
  gettimeofday(&tval,NULL);
  hrt = NSECS_IN_SEC* tval.tv_sec + NSECS_IN_USEC*tval.tv_usec;
  return hrt;
}
#endif
