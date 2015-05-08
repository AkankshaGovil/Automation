#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/

/*
  rpool.h


  Rpool of memory handling.
  Allocates contineous memory for each chunk.

  - malloc
  - calloc
  - realloc
  - free
  - copy
  - move
  - compare

  */


#ifndef _RPOOLH_
#define _RPOOLH_


#include <rvinternal.h>
#include <rlist.h>

DECLARE_OPAQUE(HRPOOL);
#define COMPARE_ERROR 0x80000000



/*** construct ***/

/************************************************************************
 * rpoolConstruct
 * purpose: Create an RPOOL object
 * input  : elemSize        - Size of elements in the RPOOL in bytes
 *          maxNumOfBlocks  - Number of blocks in RPOOL
 *          logMgr          - Log manager to use
 *          name            - Name of RPOOL (used in log messages)
 * output : none
 * return : Handle to RPOOL constructed on success
 *          NULL on failure
 ************************************************************************/
HRPOOL rpoolConstruct(
    IN int          elemSize,
    IN int          maxNumOfBlocks,
    IN RVHLOGMGR    logMgr,
    IN const char*  name);


/************************************************************************
 * rpoolClear
 * purpose: Clear the RPOOL from any allocations
 * input  : pool    - RPOOL handle
 * output : none
 * return : none
 ************************************************************************/
void rpoolClear(IN HRPOOL pool);


/************************************************************************
 * rpoolDestruct
 * purpose: Deallocate an RPOOL object
 * input  : pool    - RPOOL handle
 * output : none
 * return : none
 ************************************************************************/
void rpoolDestruct(IN HRPOOL pool);




/*** allocation handling ***/

/************************************************************************
 * rpoolAlloc
 * purpose: Allocate a chunk of memory from RPOOL.
 *          The allocation is automatically set to zero in all of its bytes
 * input  : pool    - RPOOL handle
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolAlloc(
      IN HRPOOL pool,
      IN int    size);


/************************************************************************
 * rpoolAllocCopyExternal
 * purpose: Allocate a chunk of memory from RPOOL and set it to a specified
 *          value from a buffer in memory
 * input  : pool    - RPOOL handle
 *          src     - Source pointer of the external buffer set
 *                    If NULL, then the memory allocated is set to zero
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolAllocCopyExternal(
       IN HRPOOL        pool,
       IN const void*   src,
       IN int           size);


/************************************************************************
 * rpoolAllocCopyInternal
 * purpose: Allocate a chunk of memory from RPOOL and duplicate its value
 *          from another allocation in RPOOL
 * input  : destPool- Destination RPOOL handle, where the new buffer will
 *                    be allocated
 *          srcPool - Source RPOOL handle, where the buffer we copy from
 *                    resides
 *          src     - Source pointer of the internal buffer set
 *                    It is actually an RPOOL allocation handle returned
 *                    by rpoolAlloc() and other functions in RPOOL
 *                    If NULL, then the memory allocated is set to zero
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolAllocCopyInternal(
       IN HRPOOL        destPool,
       IN HRPOOL        srcPool,
       IN const void*   src,
       IN int           size);


/************************************************************************
 * rpoolRealloc
 * purpose: Reallocate chunk of memory, leaving any old bytes with the
 *          same value they had previously and setting new allocated
 *          bytes to zero.
 * input  : pool    - RPOOL handle
 *          ptr     - Element in RPOOL to reallocate
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolRealloc(
       IN HRPOOL    pool,
       IN void*     ptr,
       IN int       size);


/************************************************************************
 * rpoolFree
 * purpose: Free an element allocation in RPOOL
 * input  : pool    - RPOOL handle
 *          ptr     - Element in RPOOL
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rpoolFree(
     IN HRPOOL  pool,
     IN void*   ptr);




/* Internal Operations */


/************************************************************************
 * rpoolCopyFromExternal
 * purpose: Copy an external memory buffer into an RPOOL element
 * input  : pool    - RPOOL handle
 *          dest    - Element in RPOOL to copy to
 *          src     - Source buffer in memory
 *          shift   - Offset in RPOOL block to copy to
 *          size    - Size of buffer to copy
 * output : none
 * return : Destination element on success
 *          NULL on failure
 ************************************************************************/
void* rpoolCopyFromExternal(
    IN HRPOOL       pool,
    IN void*        dest,
    IN const void*  src,
    IN int          shift,
    IN int          size);


/************************************************************************
 * rpoolCopyToExternal
 * purpose: Copy information from an RPOOL element to a memory buffer
 * input  : pool    - RPOOL handle
 *          dest    - Destination buffer in memory
 *          src     - Element in RPOOL to copy from
 *          shift   - Offset in RPOOL block to copy from
 *          size    - Size of buffer to copy
 * output : none
 * return : Destination memory buffer on success
 *          NULL on failure
 ************************************************************************/
void* rpoolCopyToExternal(
    IN HRPOOL       pool,
    IN void*        dest,
    IN void*        src,
    IN int          shift,
    IN int          size);


/************************************************************************
 * rpoolCopyInternal
 * purpose: Copy information from one RPOOL element to another
 * input  : pool    - RPOOL handle
 *          dest    - Element in RPOOL to copy to
 *          src     - Element in RPOOL to copy from
 *          size    - Size of buffer to copy
 * output : none
 * return : Destination memory buffer on success
 *          NULL on failure
 ************************************************************************/
void* rpoolCopyInternal(
    IN HRPOOL       pool,
    IN void*        dest,
    IN const void*  src,
    IN int          size);


void * /* dest pointer */
rpoolSet(
    /* set 'size' bytes to value */
     HRPOOL pool,
     void *dest, /* should be a valid pool chunk */
     char value,
     int size
     );

int /* as memcmp returns */
rpoolCompareExternal(
             /* Compare dest to src */
             HRPOOL pool,
             void *dest,
             void *src,
             int size
             );


/* Compares two allocated elements from pool */
/* Returns int that is less , equal greater then 0 */
/* if ptr1 is less, equal or greater than ptr2*/
int
rpoolCompareInternal(
             IN HRPOOL pool,
             IN void *ptr1,
             IN void *ptr2,
             IN int size
             );

void *
rpoolMove(
      IN HRPOOL pool,
      IN void *ptr1,
      OUT void **ptr2
      );

/*** status ***/


void
rpoolPrint(
      /* print pool allocated and free chunks into stdout. */
      HRPOOL pool
      );

int /* TRUE or RVERROR */
rpoolDisplay(
        /* Display pool as matrix busy/free */
        HRPOOL pool
        );

int
rpoolStatistics(
           /* Get pool statistics (space is in bytes) */
           IN  HRPOOL pool,
           OUT INT32* poolSize, /* max size of pool */
           OUT INT32* availableSpace, /* current available space */
           OUT INT32* allocatedSpace  /* currently allocated space */
          );

int rpoolChunkSize(
           IN HRPOOL pool,
           IN void *ptr
           );


void *
rpoolCopyPoolToPool(
            IN HRPOOL poolSrc,
            IN HRPOOL poolDest,
            IN void *ptrSrc,
            IN void *ptrDest,
            IN int size
            );
int
rpoolComparePoolToPool(
               IN HRPOOL poolSrc,
               IN HRPOOL poolDest,
               IN void *ptrSrc,
               IN void *ptrDest,
               IN int size
               );

/***************************************************************************************
 * rpoolGetPtr
 *
 * purpose: To get an drpool message and offset and return a real pointer to that location
 *          and the length until the end of the contigous part.
 *          If the offset reaches or exceeds the end of the element the length is returned as -1
 *
 * Input:  pool - the pool in question
 *         element - the element on which we are working
 *         offset - the offset in bytes into the element
 *         totalLength - the length of the whole message
 *
 * Output: pointer - A real pointer to the place calculated by the offset.
 *
 * returned value: Length - The size of the contigous area from the pointer to the end
 *                          of the current segment of the element.
 *                 -1     - If the offset is beyound the end of the message.
 ****************************************************************************************/
int
rpoolGetPtr(IN  HRPOOL  pool,
            IN  void    *element,
            IN  int     offset,
            OUT void    **pointer,
            IN  int     totalLength);



#endif
#ifdef __cplusplus
}
#endif



