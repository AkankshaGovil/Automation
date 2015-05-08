#include <stdio.h>
#include <dlfcn.h>
#include <setjmp.h>
#include<sys/types.h>
#include <sys/reg.h>
#include <sys/frame.h>

#include "srvrlog.h"

#if defined(sparc) || defined(__sparc)
#define FLUSHWIN() asm("ta 3");
#define FRAME_PTR_INDEX 1
#define SKIP_FRAMES 0
#endif

#if defined(i386) || defined(__i386)
#define FLUSHWIN() 
#define FRAME_PTR_INDEX 3
#define SKIP_FRAMES 1
#endif

#if defined(ppc) || defined(__ppc)
#define FLUSHWIN() 
#define FRAME_PTR_INDEX 0
#define SKIP_FRAMES 2
#endif

#define printf(fmt, ...)  NETINFOMSG(MDEF, (fmt, ## __VA_ARGS__)) 

/*
  this function walks up call stack, calling user-supplied
  function once for each stack frame, passing the pc and the user-supplied
  usrarg as the argument.  
  */

int cs_operate(int (*func)(void *, void *), void * usrarg)
{
 struct frame *sp;
 jmp_buf env;
 int i;
 int * iptr;

 FLUSHWIN();

 setjmp(env);
 iptr = (int*) env;

 sp = (struct frame *) iptr[FRAME_PTR_INDEX];

 for(i=0;i<SKIP_FRAMES && sp;i++)
     sp = (struct frame *) sp->fr_savfp;

 i = 0;

 while(sp && sp->fr_savpc && ++i && (*func)((void*)sp->fr_savpc, usrarg)) {
  sp =  (struct frame *) sp->fr_savfp;
 }

 return(i);
}

print_stack_trace()
{
  int print_address(void *, void *);
  printf("STACK TRACE BEGIN -->\n");
  cs_operate(print_address, NULL);
  printf("STACK TRACE END   <--\n");
}

int print_address(void *pc, void * usrarg)
{
  Dl_info info;
  char * func;
  char * lib;

  if(dladdr(pc, & info) == 0) {
   func = "??";
   lib = "??";
  }
  else {
   lib =   (char *) info.dli_fname;
   func =  (char *) info.dli_sname;
  }

  printf("STACK TRACE - %p:%s:%s+0x%td\n", 
  pc,
  lib, 
  func,
  pc - info.dli_saddr);

  return(1);
}
