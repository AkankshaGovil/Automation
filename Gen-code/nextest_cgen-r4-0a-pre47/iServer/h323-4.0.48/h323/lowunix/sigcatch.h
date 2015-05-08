#ifdef __cplusplus
extern "C" {
#endif



/*
  sigcatch.h
  Ron S.
  2 Feb. 1997

  Catching signals in UNIX systems.
  Avoiding crashes on special signals s.a. SIG_PIPE.
  */


#ifndef _SIGCATCH_
#define _SIGCATCH_

int sigCatch(void);

#endif
#ifdef __cplusplus
}              
#endif



