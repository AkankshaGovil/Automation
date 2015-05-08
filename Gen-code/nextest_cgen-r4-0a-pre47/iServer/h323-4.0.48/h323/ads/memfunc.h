#ifdef __cplusplus
extern "C" {
#endif




/*
  memFunc.h
  
  Memory Abstraction Layer
  Ron S.

  */


#ifndef _MEMFUNC_
#define _MEMFUNC_


#include <stdio.h>
#include <stdlib.h>
#include <log.h>


int
memfInit(IN RVHLOGMGR hLogMgr);

void
memfEnd(void);

void *
memfCalloc(IN int size, IN int elemSize, IN const char *name, IN int lineno);

void
memfFree(IN void *ptr);

void*
memfAllocNoLog(IN int size);


#ifndef NOLOGSUPPORT

void*
memfAlloc(IN int size, IN const char *name, IN int lineno);

void
memfReport(IN void* ptr, IN int size, IN const char *name, IN int lineno);

#endif



#endif  /* _MEMFUNC_ */


#ifdef __cplusplus
}              
#endif



