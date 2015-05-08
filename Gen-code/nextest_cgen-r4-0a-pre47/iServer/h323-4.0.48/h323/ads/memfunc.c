#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  memFunc.c


  memory functions
*/


#include <log.h>
#include <rvinternal.h>
#include <memfunc.h>

#if defined (_NUCLEUS)
#include <rvmem.h>

#elif defined(RV_OS_OSE)
#include <heapapi.h>
#endif

static RVHLOGMGR    logMgr = NULL;
static RVHLOG       localLog = NULL;

static int totalSize = 0;
static int totalBlocks = 0;
#define STATIC_VALUE (0xaaaaaaaa)


typedef struct
{
    const char* name;
    int         lineno;
    size_t      size;
    int         id;
    UINT32      debug;
} memfHeader;



/*________________________memory allocation debugging______________________*/


/* Make sure we don't recourse our own macros */
#undef malloc
#undef calloc
#undef realloc
#undef free



int
memfInit(IN RVHLOGMGR hLogMgr)
{
    logMgr = hLogMgr;
    localLog = logRegister(hLogMgr, "MEMORY", "Memory allocator");
    return 0;
}


void
memfEnd(void)
{
    localLog = NULL;
}



#ifdef NOLOGSUPPORT

void*
memfAllocNoLog(IN int size)
{
    UINT32* ptr;

    if (size == -1)
    {
        /* When malloc(-1) is called, we should just printout current amount of allocated
           memory */
        printf("Total stack allocation: %d bytes in %d blocks\n", totalSize, totalBlocks);
        return NULL;
    }

#ifdef RV_OS_OSE
    ptr = heap_alloc_shared(size + sizeof(void*), __FILE__, __LINE__);
#elif defined (_NUCLEUS)
    ptr = (memfHeader *)rvMemAlloc(size + sizeof(void*));
#else
    ptr = malloc(size + sizeof(void*));
#endif

    if (ptr != NULL)
    {
        totalSize += size;
        totalBlocks++;
        *ptr = size;
        return (void *)((char*)ptr + sizeof(void*));
    }

    return NULL;
}


void
memfFree(IN void *ptr)
{
    UINT32*     p;

    p = (UINT32 *)((char*)ptr - sizeof(void*));

    totalSize -= (int)*p;
    totalBlocks--;
#if defined (RV_OS_OSE)
    heap_free_shared(p);
#elif defined (_NUCLEUS)
    rvMemFree(p);
#else
    free(p);
#endif
}





#else  /* NOLOGSUPPORT */


static int theId = 0;



void*
memfAllocNoLog(IN int size)
{
    memfHeader* ptr;
    UINT32      allocationSize;
    UINT32*     blockEnd;

    /* Make sure allocation is 4-octet-aligned */
    allocationSize = RV_ALIGN(size);

#if defined (RV_OS_OSE)
    ptr = (memfHeader *)heap_alloc_shared(allocationSize + sizeof(memfHeader) + sizeof(UINT32), __FILE__, __LINE__);
#elif defined (_NUCLEUS)
    ptr = (memfHeader *)rvMemAlloc(allocationSize + sizeof(memfHeader) + sizeof(UINT32));
#else
    ptr = (memfHeader *)malloc(allocationSize + sizeof(memfHeader) + sizeof(UINT32));
#endif

    if (ptr != NULL)
    {
        totalSize += size;
        totalBlocks++;

        blockEnd = (UINT32 *)( (char *)(ptr + 1) + allocationSize );
        ptr->size = size;
        ptr->name = NULL;
        ptr->lineno = -1;
        ptr->id=theId++;
        ptr->debug = STATIC_VALUE;
        *blockEnd = STATIC_VALUE;

        return (void *)(ptr + 1);
    }
    return NULL;
}


void
memfReport(IN void* ptr, IN int size, IN const char *name, IN int lineno)
{
    if (ptr != NULL)
    {
        memfHeader* p;

        p = (memfHeader *)ptr - 1;
        logPrint(localLog, RV_DEBUG, (localLog, RV_DEBUG,
                 "memfAlloc(%d): %s:%d allocated %d bytes at 0x%p. Total is %d (%d allocations)",
                 p->id, nprn(name), lineno, size, ptr, totalSize, totalBlocks));

        p->name = name;
        p->lineno = lineno;
    }
    else
    {
        logPrint(localLog, RV_ERROR, (localLog, RV_ERROR,
                 "memfAlloc: Cannot allocate memory in size: %d (%s:%d)",
                 size, nprn(name), lineno));
        perror("malloc");
    }
}


void *
memfAlloc(IN int size, IN const char *name, IN int lineno)
{
    void* ptr;
    ptr = memfAllocNoLog(size);
    memfReport(ptr, size, name, lineno);

    return ptr;
}


void
memfFree(IN void *ptr)
{
    UINT32      allocationSize;
    memfHeader* p;
    UINT32*     blockEnd;

    p = ((memfHeader *)ptr) - 1;
    allocationSize = (UINT32)RV_ALIGN(p->size);
    blockEnd = (UINT32 *)( (char *)(p+1) + allocationSize );

    totalSize -= (int)p->size;
    totalBlocks--;

    if (p->lineno != -1)
    {
        /* We should only start logging if we logged the malloc() */
        logPrint(localLog, RV_DEBUG, (localLog, RV_DEBUG,
                 "memfFree(%d): Deallocating 0x%p (%d bytes allocated by %s:%d). Total is %d (%d allocations)",
                 p->id,ptr, p->size, nprn(p->name), p->lineno, totalSize, totalBlocks));

        if (p->debug != STATIC_VALUE)
        {
            logPrint(localLog, RV_EXCEP, (localLog, RV_EXCEP,
                     "memfFree: Trashed allocation beginning (%s:%d - 0x%p)",
                     p->name, p->lineno, p+1));
        }
        if (*blockEnd != STATIC_VALUE)
        {
            logPrint(localLog, RV_EXCEP, (localLog, RV_EXCEP,
                     "memfFree: Trashed allocation ending (%s:%d - 0x%p)",
                     p->name, p->lineno, p+1));
        }
    }

#ifdef RV_OS_OSE
    heap_free_shared(p);
#elif defined (_NUCLEUS)
    rvMemFree(p);
#else
    free(p);
#endif

    /* Make sure we're not freeing the log... */
    if ((void *)logMgr == ptr)
    {
        logMgr = NULL;
        localLog = NULL;
    }
}


#endif  /* NOLOGSUPPORT */



void *
memfCalloc(IN int size, IN int elemSize, IN const char *name, IN int lineno)
{
    void *ptr = NULL;
    if (lineno || name);

    if (! (ptr = memfAlloc(size*elemSize, name, lineno)))
        return NULL;

    memset(ptr, 0, (size_t)size*elemSize);
    return ptr;
}




#ifdef __cplusplus
}
#endif



