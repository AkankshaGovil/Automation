/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


/************************************************************************
 *
 * rvinternal.h
 *
 *
 * Common definitions for RADVISION project
 * This file should be included by RADVISION code, and should not be used
 * by customer's applications.
 *
 ************************************************************************/

#ifndef __RVINTERNAL_H
#define __RVINTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif


#include <rvcommon.h>
#include <rv_defs.h>


#ifdef _DEBUG
#define RV_DEBUG_ALL
#endif


#define PROTOCOL_IDENTIFIER "itu-t recommendation h(8) 2250 0 4"

/************************************************************************
 * Global macro definitions
 ************************************************************************/
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef abs
#define abs(a) ( ((a)>(0))?(a):(-(a)) )
#endif



/************************************************************************
 * memory handling functions definitions
 ************************************************************************/
#define malloc(size)  memfAlloc((int)(size), __FILE__, __LINE__)
#define calloc(size, elemSize)  memfCalloc(size, (elemSize), __FILE__, __LINE__)
#define halloc(size, elemSize)  memfCalloc(size, (elemSize), __FILE__, __LINE__)
#define free(p)  memfFree((p))
#define hfree(p)  memfFree((p))

/* See memfunc.h for the implementation of the functions below */
void* memfAlloc(IN int size, IN const char *name, IN int lineno);
void* memfCalloc(IN int size, IN int elemSize, IN const char *name, IN int lineno);
void* memfAllocNoLog(IN int size);
void memfFree(IN void *ptr);

#undef realloc

#ifdef NOLOGSUPPORT
#define memfReport(ptr, size, name, lineno)
#define memfAlloc(size, name, lineno) memfAllocNoLog(size)
#endif  /* NOLOGSUPPORT */



/************************************************************************
 * resources struct
 * cur      - Current used elements
 * maxUsage - Maximum number of elements used since initialization
 * max      - Maximum number of elements that can be used
 ************************************************************************/
typedef struct
{
    UINT32      cur;
    UINT32      maxUsage;
    UINT32      max;
} rvStatistics;



#ifdef __cplusplus
}
#endif

#endif  /* __RVINTERNAL_H */

