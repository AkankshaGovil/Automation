#ifdef __cplusplus
extern "C" {
#endif



/*
  sigcatch.c
  Ron S.
  2 Feb. 1997

  Catching signals in UNIX systems.
  Avoiding crashes on special signals s.a. SIG_PIPE.
  */


#include <signal.h>
#include <rvinternal.h>

static void handleSignal(int signo);

int
sigCatch(void)
{
#if 0 /* nextone - do not overwrite MSW's signal handler */
  if (signal(SIGPIPE, handleSignal) == SIG_ERR)
    fprintf(stderr, "Can't catch SIGPIPE signal\n");
#endif

  return TRUE;
}


static void
handleSignal(int signo)
{
#if 0 /* nextone - do not overwrite MSW's signal handler */
  signo=0;
  sigCatch();
#endif
  fprintf(stderr, "Received SIGPIPE signal\n");
}
#ifdef __cplusplus
}
#endif



