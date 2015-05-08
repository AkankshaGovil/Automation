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
    rpool.c

     memory allocation mechanism, implementation based on fixed size array

    */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rvinternal.h>

#include <rpool.h>
#include <ra.h>
#include <ms.h>


/*
  An RPOOL node always has bit 0x40000000 used.
  This is the case since RPOOL and RTREE are used in the same allocation space in the PVT,
  and this bit is used to see which type of node is used in each case.
  Each RPOOL node holds in its first parameter the location of the next block.
  The last element node holds 0x60000000, ORd with the size used in the last block.
 */

/* MACRO Definitions */
#define RPOOL_LOCATION(p)       ((int)p)
#define RETURNED_VALUE(p)       (void *)(p|0x40000000)
#define NODE_HANDLE(p)          (p|0x40000000)
#define RPOOL_NODE_ID(p)        ((int)p&~0x60000000)
#define IS_LAST_NODE(p)         ((p)& 0x20000000)
#define GET_LAST_NODE_SIZE(p)   ((p)&~0x20000000)
#define SET_LAST_NODE(p)        ((p) |0x20000000)
#define rpoolGetNode(raH,location)  ((rpoolElem *)raGet((raH),RPOOL_NODE_ID(location)))
#define INVALID_LOCATION(raH,location) (!((location) & 0x60000000) || (raElemIsVacant((HRA)(raH), RPOOL_NODE_ID(location))))

#define RPOOL_NODE_SIZE(elemSize) ((int)((elemSize) + sizeof(int)))
#define RPOOL_ELEM_SIZE(nodeSize) ((int)((nodeSize) - sizeof(int)))
#define RPOOL_BLOCKS(size,elemSize)  ((size)/(elemSize) + ( ((size)%(elemSize)) ? 1 : 0 ))



/************************************************************************
 * rpoolElem
 * Header of an element inside rpool
 * nextBlock    - Location of the next block for this allocation
 * data         - User's data information stored
 ************************************************************************/
typedef struct
{
    int     nextBlock;
    char    data;
} rpoolElem;







/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


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
    IN const char*  name)
{
    HRA raH;

    if ((!elemSize) || (!maxNumOfBlocks)) return NULL;
    if (!(raH = raConstruct(RPOOL_NODE_SIZE(elemSize), maxNumOfBlocks, logMgr, name)) )
        return NULL;

    rpoolClear((HRPOOL)raH);
    return (HRPOOL)raH;
}


/************************************************************************
 * rpoolClear
 * purpose: Clear the RPOOL from any allocations
 * input  : pool    - RPOOL handle
 * output : none
 * return : none
 ************************************************************************/
void
rpoolClear(IN HRPOOL pool)
{
    if (pool != NULL)
        raClear((HRA)pool);
}


/************************************************************************
 * rpoolDestruct
 * purpose: Deallocate an RPOOL object
 * input  : pool    - RPOOL handle
 * output : none
 * return : none
 ************************************************************************/
void rpoolDestruct(IN HRPOOL pool)
{
    if (pool != NULL)
        raDestruct((HRA)pool);
}


/************************************************************************
 * rpoolAlloc
 * purpose: Allocate a chunk of memory from RPOOL
 *          The allocation is automatically set to zero in all of its bytes
 * input  : pool    - RPOOL handle
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolAlloc(
      IN HRPOOL pool,
      IN int    size)
{
    return rpoolAllocCopyExternal(pool, NULL, size);
}


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
       IN int           size)
{
    HRA raH = (HRA)pool;
    rpoolElem *elem=NULL, *newElem;
    int numOfBlocks,numOfFreeBlocks;
    int elemSize;
    int i, cur, first=0;

    if (( ! pool ) || ( ! size )) return NULL;

    /* Calculate the number of blocks we're dealing with */
    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
    numOfBlocks = RPOOL_BLOCKS(size,elemSize);
    numOfFreeBlocks = raFreeSize(raH);

    /* Make sure we've got enough room for it */
    if ( (!size) || (numOfFreeBlocks < numOfBlocks )) return NULL;

    /* Start adding blocks */
    for(i = 0; i < numOfBlocks; i++)
    {
        /* Allocate another block */
        if ( (cur = raAddExt(raH, (void **)&newElem)) < 0 )
        {
            /* This shouldn't happen ! */
            if (elem != NULL)
            {
                /* Make sure this rpool element is valid even though it's an error */
                elem->nextBlock = SET_LAST_NODE(0);
            }
            return NULL;
        }

        /* Link the last handle to the new one */
        if (elem != NULL)
            elem->nextBlock = NODE_HANDLE(cur);
        else
            first = cur;

        elem = newElem; /* rpoolGetNode(raH,cur);    */
        elem->nextBlock = 0;

        /* Fill/Clear the allocated memory chunk */
        if (src != NULL)
        {
            /* We need to set from given external memory buffer */
            if ((i == numOfBlocks-1) && (size % elemSize))
            {
                /* Last element - copy only the bytes that are left in it */
                memcpy(&(elem->data), (char*)src+i*elemSize, size % elemSize);
            }
            else
            {
                /* Something in the middle. Copy it all */
                memcpy(&(elem->data), (char*)src+i*elemSize, elemSize);
            }
        }
        else
        {
            /* Clear the allocation */
            memset(&(elem->data), 0, elemSize);
        }
    }

    /* Set the last block as the last block */
    elem->nextBlock = SET_LAST_NODE ( (size%elemSize) ? size%elemSize : elemSize );

    /* Return the allocation */
    return (RETURNED_VALUE(first));
}


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
       IN int           size)
{
    HRA destRa = (HRA)destPool;
    HRA srcRa = (HRA)srcPool;
    rpoolElem *elem=NULL, *elemSrc=NULL, *newElem;
    int numOfBlocks,numOfFreeBlocks;
    int elemSize;
    int i, cur, first=0;

    if ((srcPool == NULL) || (destPool == NULL) || (size <= 0))
        return NULL;

    /* Get the source element */
    if (src != NULL)
    {
        int location = RPOOL_LOCATION(src);
        if (INVALID_LOCATION(srcRa,location))
            return NULL;
        elemSrc=rpoolGetNode(srcRa,location);
    }

    /* Calculate the number of blocks we're dealing with */
    elemSize = RPOOL_ELEM_SIZE(raElemSize(srcRa));
    numOfBlocks = RPOOL_BLOCKS(size,elemSize);
    numOfFreeBlocks = raFreeSize(destRa);

    /* Make sure we've got enough room for it */
    if ( (!size) || (numOfFreeBlocks < numOfBlocks )) return NULL;

    /* Start adding blocks */
    for(i=0; i<numOfBlocks; i++)
    {
        /* Add an RPOOL block */
        if ( (cur = raAddExt(destRa, (void **)&newElem)) < 0 )
        {
            if (elem != NULL)
            {
                /* Make sure this rpool element is valid */
                elem->nextBlock = SET_LAST_NODE(0);
            }
            return NULL;
        }

        /* Link previous block to the new one */
        if (elem)
            elem->nextBlock = NODE_HANDLE(cur);
        else
            first=cur;

        elem = newElem; /*rpoolGetNode(raH,cur);    */
        elem->nextBlock = 0;

        /* Copy/Clear the allocated block */
        if (src != NULL)
        {
            memcpy(&(elem->data), &(elemSrc->data), elemSize);
            if (IS_LAST_NODE(elemSrc->nextBlock))
                src=NULL;
            else
                elemSrc=rpoolGetNode(srcRa,elemSrc->nextBlock);
        }
        else
        {
            memset(&(elem->data), 0, elemSize);
        }
    }

    elem->nextBlock = SET_LAST_NODE ( (size%elemSize) ? size%elemSize : elemSize );
    return (RETURNED_VALUE(first));
}


/************************************************************************
 * rpoolRealloc
 * purpose: Reallocate chunk of memory, leaving any old bytes with the
 *          same value they had previously and setting new allocated
 *          bytes to zero.
 * input  : pool    - RPOOL handle
 *          src     - Element in RPOOL ot reallocate
 *          size    - Size of allocation in bytes
 * output : none
 * return : Pointer to memory chunk on success
 *          NULL on failure
 ************************************************************************/
void* rpoolRealloc(
       IN HRPOOL    pool,
       IN void*     ptr,
       IN int       size)
{
    HRA raH = (HRA)pool;
    int numOfBlocks;
    int elemSize;
    int count=0;
    int contLocation, location;
    rpoolElem *elem=NULL;

    if(( ! pool ) || (! size))  return NULL;

    location = RPOOL_LOCATION( ptr);

    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
    numOfBlocks = RPOOL_BLOCKS(size,elemSize);

    /* Count the nodes in RPOOL that we're going to leave as is.
       We might end up holding all of them if the reallocation is bigger than the
       original size */
    while (( !IS_LAST_NODE(location) )&&( count < numOfBlocks ))
    {
        elem = rpoolGetNode(raH,location);
        location = elem->nextBlock;
        count ++;
    }

    /* New size is smaller than at previous allocation */
    if ( count == numOfBlocks )
    {
        /* Fix the next block of the new last element */
        elem->nextBlock = SET_LAST_NODE ( (size%elemSize) ? size%elemSize :  elemSize );

        /* Free the tail and we're done */
        rpoolFree( pool, (void *)location);
        return (ptr);
    }

    /* New size is bigger than at previous allocation */
    if ( IS_LAST_NODE(location) )
    {
        /* Allocate the additional space needed */
        if ((contLocation = (int)rpoolAlloc(pool, (size - count*elemSize)) ) == 0)
            return NULL;

        /* Link the new allocation with the old one */
        elem->nextBlock = contLocation;
        return (ptr);
    }

    return (NULL);
}


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
     IN void*   ptr)
{
    HRA raH = (HRA)pool;
    int next, location;
    rpoolElem *elem;

    if ( ! pool ) return RVERROR;

    location = RPOOL_LOCATION(ptr);
    if (INVALID_LOCATION(raH,location)) return RVERROR;

    while (!IS_LAST_NODE(location))
    {
        location=RPOOL_NODE_ID(location);
        elem = rpoolGetNode(raH,location);
        if (elem != NULL)
            next = elem->nextBlock;
        else
            next = -1;
        raDeleteLocation(raH, location);
        location = next;
    }

    return TRUE;
}






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
    IN int          size)
{
    HRA raH = (HRA)pool;
    int elemSize, tmpSize;
    int location, i, shiftBlocks;
    rpoolElem *elem;
    int srcShift=0;

    if(( !pool ) || ( !src )  || ( !size ))return NULL;

    location = RPOOL_LOCATION(dest);

    if (INVALID_LOCATION(raH,location)) return NULL;

    /* Calculate the number of blocks we want to skip */
    tmpSize = 0;
    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
    shiftBlocks = RPOOL_BLOCKS(shift,elemSize);

    for ( i=0;!IS_LAST_NODE(location) && size>0; i++,location = elem->nextBlock)
    {
        elem=rpoolGetNode(raH,location);
        if (i==shiftBlocks-1 && shift%elemSize)
        {
            /* First block we copy to */
            tmpSize=min(elemSize-shift%elemSize, size);
            memcpy( (char *)&(elem->data)+(shift%elemSize), (char *)src, tmpSize);
            size-=tmpSize;
        }
        if (i>shiftBlocks-1)
        {
            /* Copy full blocks */
            tmpSize=min(elemSize, size);
            memcpy( (char *)&(elem->data), (char *)src+srcShift, tmpSize);
            size-=tmpSize;
        }

        srcShift+=tmpSize;
    }

    return (dest);
}


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
    IN int          size)
{
    HRA raH = (HRA)pool;
    int elemSize, tmpSize;
    int location, i, shiftBlocks;
    rpoolElem *elem;
    int destShift=0;

    if(( !pool ) || ( !dest )  || ( !size ))return NULL;

    location = RPOOL_LOCATION(src);

    if (INVALID_LOCATION(raH,location)) return NULL;

    /* Calculate the amount of blocks to skip before copying */
    tmpSize = 0;
    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
    shiftBlocks = RPOOL_BLOCKS(shift,elemSize);

    for ( i=0;!IS_LAST_NODE(location) && size>0; i++,location = elem->nextBlock)
    {
        elem=rpoolGetNode(raH,location);
        if (i==shiftBlocks-1 && shift%elemSize)
        {
            /* First block we're copying */
            tmpSize=min(elemSize-shift%elemSize, size);
            memcpy( (char *)dest, (char *)&(elem->data)+(shift%elemSize), tmpSize);
            size-=tmpSize;
        }
        if (i>shiftBlocks-1)
        {
            /* Copy the block */
            tmpSize=min(elemSize, size);
            memcpy( (char *)dest+destShift, (char *)&(elem->data),  tmpSize);
            size-=tmpSize;
        }
        destShift+=tmpSize;
    }

    return (dest);
}


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
    IN int          size)
{
    HRA raH = (HRA)pool;
    int elemSize;
    int numOfBlocks;
    int destLocation, srcLocation, i;
    rpoolElem *srcElem, *destElem;

    if ((! pool ) || ( ! size ))
        return NULL;

    srcLocation = RPOOL_LOCATION(src);
    destLocation = RPOOL_LOCATION(dest);

    if (INVALID_LOCATION(raH,srcLocation)) return NULL;
    if (INVALID_LOCATION(raH,destLocation)) return NULL;

    /* Calculate the number of blocks to copy */
    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
    numOfBlocks = RPOOL_BLOCKS(size,elemSize);

    /* Go through the blocks until we're done or until we don't have any more nodes in dest or src */
    for(i = 0; (!IS_LAST_NODE(destLocation) && !IS_LAST_NODE(srcLocation) && (i < numOfBlocks)); i++)
    {
        /* Get the pointer to the actual data */
        srcElem = rpoolGetNode(raH, srcLocation);
        destElem = rpoolGetNode(raH, destLocation);

        /* Copy */
        memcpy((void *)&(destElem->data), (void *)&(srcElem->data), elemSize);

        /* Go on the next block */
        destLocation = destElem->nextBlock;
        srcLocation = srcElem->nextBlock;
    }

    return (dest);
}


    /* Sets pool location pointed by ptr to value 'value' */
    /* Returns pointer to ptr or NULL */
    void *
        rpoolSet(
        IN HRPOOL pool,
        void *ptr,
        char value,
        int size
        )
    {
        HRA raH = (HRA)pool;
        int location;
        int elemSize;
        int numOfBlocks;
        int i;
        rpoolElem *elem;
        int iValue = value;

        if(( !pool ) || (! size ))
            return NULL;

        location = RPOOL_LOCATION( ptr);

        if (INVALID_LOCATION(raH,location)) return NULL;

        elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
        numOfBlocks = RPOOL_BLOCKS(size,elemSize);

        for(i=0;(!IS_LAST_NODE(location)&&(i<numOfBlocks));i++) {
            elem=rpoolGetNode(raH,location);
            memset((void *)&(elem->data), iValue, elemSize);
            location=elem->nextBlock;
        }

        return (ptr);
    }

    /* Compares allocated element from pool with external block of memory */
    /* Returns  the same output as memcmp */
    int
        rpoolCompareExternal(
        IN HRPOOL pool,
        IN void *dest,
        IN void *src,
        IN int size
        )
    {
        HRA raH = (HRA)pool;
        int elemSize, numOfBlocks;
        int i, location, res=RVERROR;
        rpoolElem *elem;


        if ( (! pool ) || ( ! size) || (! src ))
            return RVERROR;

        location = RPOOL_LOCATION(dest);

        if (INVALID_LOCATION(raH,location)) return COMPARE_ERROR;

        elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
        numOfBlocks = RPOOL_BLOCKS(size,elemSize);

        for(i=0;(!IS_LAST_NODE(location)&&(i<(numOfBlocks-1)));i++) {
            elem=rpoolGetNode(raH,location);
            if  ((res =  memcmp((void *)&(elem->data),(char*)src+i*elemSize , elemSize))) return(res);
            location=elem->nextBlock;
        }

        if(!IS_LAST_NODE(location) && ( i == ( numOfBlocks-1))){
            elem=rpoolGetNode(raH,location);
            if  ((res =  memcmp((void *)&(elem->data),(char*)src+i*elemSize ,
                (size%elemSize ? size%elemSize : elemSize))
                ))
                return(res);
        }


        if (IS_LAST_NODE(location)&&((size-i*elemSize)>0)) return (-1);

        return (res);
    }

    /* Compares two allocated elements from pool */
    /* Returns int that is less , equal greater then 0 */
    /* if ptr1 is less, equal or greater than ptr2*/
    int
        rpoolCompareInternal(
        IN HRPOOL pool,
        IN void *ptr1,
        IN void *ptr2,
        IN int size
        )
    {
        HRA raH=(HRA)pool;
        int elemSize, numOfBlocks;
        int i, location1, location2, res=RVERROR;
        rpoolElem *elem1,*elem2;

        if ( (! pool ) || (! size) ) return RVERROR;

        location1=RPOOL_LOCATION(ptr1);
        location2=RPOOL_LOCATION(ptr2);

        if (INVALID_LOCATION(raH,location1)) return COMPARE_ERROR;
        if (INVALID_LOCATION(raH,location2)) return COMPARE_ERROR;

        elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));
        numOfBlocks = RPOOL_BLOCKS(size,elemSize);

        for (i=0; (i<numOfBlocks)&&!IS_LAST_NODE(location1)&&!IS_LAST_NODE(location2) ; i++){
            elem1=rpoolGetNode(raH,location1);
            elem2=rpoolGetNode(raH,location2);
            if (( res = memcmp( (void *)&(elem1->data), (void *)&(elem2->data), min(size,elemSize))))
                return (res);
            location1=elem1->nextBlock;
            location2=elem2->nextBlock;
            size-=elemSize;
        }


        if ((!IS_LAST_NODE(location1) &&!IS_LAST_NODE(location2)) ||
            ( IS_LAST_NODE(location1) && IS_LAST_NODE(location2)) ||
            ( i==numOfBlocks) )
            return (res);

        if( IS_LAST_NODE(location1))
            return (-1);
        else
            return (1);
    }

    /* Moves from ptr1 to ptr2 */
    /* Returns pointer to new ptr2 location */
    void *
        rpoolMove(
        IN HRPOOL pool,
        IN void *ptr1,
        OUT void **ptr2
        )
    {
        int location;

        if ( ! pool ) return NULL;

        location = RPOOL_LOCATION( ptr1);

        if (INVALID_LOCATION(pool,location)) return NULL;

        (*ptr2) = (void*)RETURNED_VALUE(location);
        return (*ptr2);
    }


#ifndef NOLOGSUPPORT
    /*         Display Functions */
    void
        rpoolPrint(
        IN HRPOOL pool
        )
    {
        HRA raH = (HRA)pool;
        int location;
        int maxNumOfElem;
        int length;
        rpoolElem *elem;
        int elemSize;
        char tmp[128];

        strcpy(tmp, "");

        if( ! pool ) return ;

        maxNumOfElem=raMaxSize(raH);
        elemSize = (int)(raElemSize(raH) - sizeof(rpoolElem) + sizeof(char));
        for(location=0;location<maxNumOfElem;location++) {
            elem = rpoolGetNode(raH,location);
            length = IS_LAST_NODE(elem->nextBlock) ? (GET_LAST_NODE_SIZE(elem->nextBlock)) : elemSize ;
            if ( elem->data  ){
                memcpy(tmp, (char *)&(elem->data), elemSize);
                tmp[length]='\0';
            }
            printf("\n%d: alloc[%d] next[%d] data[%s]",
                location, raElemIsVacant(raH, location),
                elem->nextBlock, nprn(tmp));
            strcpy(tmp, "");


        }
    }
#endif
    int
        rpoolStatistics(
           /* Get pool statistics (space is in bytes) */
           IN  HRPOOL pool,
           OUT INT32* poolSize, /* max size of pool */
           OUT INT32* availableSpace, /* current available space */
           OUT INT32* allocatedSpace  /* currently allocated space */
           )
    {
        HRA raH = (HRA)pool;
        int maxNumOfBlocks;
        int elemSize;

        if (( ! pool ) || ( ! poolSize ) || (! availableSpace) || ( ! allocatedSpace))
            return RVERROR;

        maxNumOfBlocks=raMaxSize(raH);
        elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));

        *poolSize = elemSize * maxNumOfBlocks;
        *availableSpace = raFreeSize(raH)*elemSize;
        *allocatedSpace = raCurSize(raH)*elemSize;

        return TRUE;
    }

    int rpoolChunkSize(
        IN HRPOOL pool,
        IN void *ptr
        )
    {
        HRA raH = (HRA)pool;
        rpoolElem *elem;
        int elemSize;
        int location;
        int size = 0;

        if( ! pool ) return RVERROR;

        location = RPOOL_LOCATION( ptr);
        if (INVALID_LOCATION(raH,location)) return RVERROR;

        elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));

        while(!IS_LAST_NODE(location) ){
            elem=rpoolGetNode(raH,location);
            size += (IS_LAST_NODE(elem->nextBlock) ? (GET_LAST_NODE_SIZE(elem->nextBlock)) : elemSize);
            location = elem->nextBlock;
        }

        return (size);
    }

    /*   Inter RPOOLs operations */

    /* Makes copy from poolSrc-ptrSsrc  to poolDest-ptrDest only if element sizes are equal */
    /* Otherwise return NULL */
    /* Returns pointer to dest */
    void *
        rpoolCopyPoolToPool(
        IN HRPOOL poolSrc,
        IN HRPOOL poolDest,
        IN void *ptrSrc,
        IN void *ptrDest,
        IN int size
        )
    {
        int src, dest, srcSize, destSize, elemSize;
        HRA raSrc = (HRA)poolSrc;
        HRA raDest = (HRA)poolDest;
        int totalSize = 0;
        rpoolElem *elemSrc, *elemDest;

        if (( ! poolSrc ) || ( ! poolDest) || ( ! size))
            return NULL;

        srcSize = raElemSize(raSrc);
        destSize = raElemSize(raDest);

        if( srcSize != destSize ) return NULL;

        elemSize = RPOOL_ELEM_SIZE(srcSize);

        src = RPOOL_LOCATION(ptrSrc);
        dest = RPOOL_LOCATION(ptrSrc);

        if (INVALID_LOCATION(raSrc,src)) return NULL;
        if (INVALID_LOCATION(raDest,dest)) return NULL;

        while (!IS_LAST_NODE(src) && !IS_LAST_NODE(dest) && (totalSize < size )) {
            elemSrc = rpoolGetNode(raSrc, src);
            elemDest = rpoolGetNode(raDest, dest);
            memcpy((void *)&(elemDest->data), (void *)&(elemSrc->data), elemSize);
            dest = elemDest->nextBlock;
            src = elemSrc->nextBlock;
            totalSize += elemSize;
        }
        return (ptrDest);

    }

    /* Compares two locations at different pools , only if element size at both are equal*/
    /* Returns the value less, equal , more than 0 if Src smaller , equal , greater than */
    /* Dest respectevly */
    int
        rpoolComparePoolToPool(
        IN HRPOOL poolSrc,
        IN HRPOOL poolDest,
        IN void *ptrSrc,
        IN void *ptrDest,
        IN int size
        )
    {
        int src, dest, srcSize, destSize, elemSize, res=RVERROR;
        HRA raSrc = (HRA)poolSrc;
        HRA raDest = (HRA)poolDest;
        int totalSize = 0;
        rpoolElem *elemSrc, *elemDest;
        if(ptrDest);

        if (( ! poolSrc ) || ( ! poolDest) || ( ! size))
            return RVERROR;

        srcSize = raElemSize(raSrc);
        destSize = raElemSize(raDest);

        if( srcSize != destSize ) return RVERROR;

        elemSize = RPOOL_ELEM_SIZE(srcSize);

        src = RPOOL_LOCATION(ptrSrc);
        dest = RPOOL_LOCATION(ptrSrc);

        if (INVALID_LOCATION(raSrc,src)) return COMPARE_ERROR;
        if (INVALID_LOCATION(raDest,dest)) return COMPARE_ERROR;

        while ( !IS_LAST_NODE(src) && !IS_LAST_NODE(dest) && (totalSize < size )) {
            elemSrc = rpoolGetNode(raSrc, src);
            elemDest = rpoolGetNode(raSrc, dest);
            if (( res = memcmp((void *)&(elemSrc->data), (void *)&(elemDest->data), elemSize) ))
                return (res);
            dest = elemDest->nextBlock;
            src = elemSrc->nextBlock;
            totalSize += elemSize;
        }

        if ((!IS_LAST_NODE(src) &&!IS_LAST_NODE(dest)) ||
            ( IS_LAST_NODE(src) && IS_LAST_NODE(dest)) ||
            ( totalSize==size) )
            return (res);

        if( IS_LAST_NODE(src) )
            return (-1);
        else
            return (1);
    }


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
            IN  int     totalLength)
{
    HRA raH = (HRA)pool;
    int elemSize;
    int length = -1;
    int location, i, shiftBlocks, lastBlock;
    rpoolElem *elem;
    BOOL        found = FALSE;

    /* check that the inputs were legal */
    if(( !pool ) || ( !element ))
        return -1;

    location = RPOOL_LOCATION(element);

    if (INVALID_LOCATION(raH,location))
        return -1;

    /* calculate the size of each part of the element */
    elemSize = RPOOL_ELEM_SIZE(raElemSize(raH));

    /* calculate the part in which the offset falls */
    shiftBlocks = RPOOL_BLOCKS(offset+1,elemSize);
    lastBlock   = RPOOL_BLOCKS(totalLength, elemSize);

    /* go over the parts until we reach the part in which the offset is */
    for ( i=1; (!found) && (i <= shiftBlocks); i++, location = elem->nextBlock)
    {
        /* get the current part entry */
        elem=rpoolGetNode(raH,location);

        /* if we reached the required part calculate the pointer and length */
        if (i==shiftBlocks)
        {
            /* Assume the message occupies all the block and maybe more */
            int lastByte = elemSize;

            if (i == lastBlock)
            {
                /* last chunk of the message, we need only the tail of the message */
                if ((totalLength % elemSize) != 0)
                    lastByte = totalLength % elemSize;
            }

            /* the contigous area is from the offset in this block to the last byte */
            length = lastByte - offset%elemSize;

            /* The pointer to the contigous area is the beginning of the block plus
               the offset within the block */
            if (pointer)
                *pointer = (void *)( (char *)&(elem->data) + (offset%elemSize) );

            found = TRUE;
        }
    }

    return length;
}


#ifdef __cplusplus
}
#endif



