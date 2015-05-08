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
  ra.c

  fixed size array implementation.

  Ron Shpilman 28 Jan. 1996

  Format:

  +----------+------------+----------------+
  |  header  | bit vector | array of data  |
  +----------+------------+----------------+
  (raHeader)               (sizeofElement*maxNodes)


  Version 1: Ron S. 21 Aug. 1996. Improving vacant element search to O(1) by
  linking all vacant nodes (link is inside the data). array of elements removed.
  no need for vacant and data pointer (saving a lot of space).

  Note: element size >=4.
  Bit vector: bit i=0 iff element i is free.

  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rvinternal.h>

#include <copybits.h>
#include <ra.h>



#define ALIGN sizeof(void*)

#define BIT_VECTOR(ra)      ((BYTE *)(ra) + sizeof(raHeader) )

#define BIT_VECTOR_SIZE(n)   ( (n+7)/8 + (ALIGN - ((n+7)/8)%ALIGN)%ALIGN)



#define NEXT_VACANT(ra, i)  (((vacantNode *)ELEM_DATA(ra, i))->nextVacant)

typedef struct {
  int nextVacant;
} vacantNode;



/************************************************************************
 *
 *                           Private functions
 *
 ************************************************************************/


/************************************************************************
 * raGetAllocationSize
 * purpose: Return the allocation size of an RA object
 * input  : elemSize            - Size of elements in the RA in bytes
 *          maxNumOfElements    - Number of elements in RA
 * output : none
 * return : Allocation size
 ************************************************************************/
static int
raGetAllocationSize(int elemSize, int maxNumOfElements)
{
  return
    sizeof(raHeader) +
    BIT_VECTOR_SIZE(maxNumOfElements) +
    (maxNumOfElements * max(4, elemSize));
}


/************************************************************************
 * raBuild
 * purpose: Build the actual RA from a given allocated memory block
 * input  : buffer              - Buffer of allocated RA memory block
 *          elemSize            - Size of elements in the RA in bytes
 *          maxNumOfElements    - Number of elements in RA
 *          logMgr              - Log manager to use
 *          name                - Name of RA (used in log messages)
 * output : none
 * return : Pointer to the modified header
 ************************************************************************/
static raHeader *
raBuild(
    IN char*        buffer,
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN const char*  name)
{
    raHeader *ra;

    /* Clear the buffer */
    memset(buffer, 0, raGetAllocationSize(elemSize, maxNumOfElements));

    /* Set the header information */
    ra = (raHeader *)buffer;

    ra->maxNumOfElements = maxNumOfElements;
    ra->arrayLocation = (char*)ra + BIT_VECTOR_SIZE(maxNumOfElements) + sizeof(raHeader);
    ra->curNumOfElements = 0;
    ra->sizeofElement    = elemSize;
    ra->maxUsage         = 0;
    strncpy(ra->name, name, sizeof(ra->name));

    raClear((HRA)ra);   /* init vacant list and pointers */

    return ra;
}


/************************************************************************
 * raElemSet
 * purpose: Set the value of an RA element
 * input  : raH         - Handle of the RA object
 *          location    - Location of RA element to check
 *          src         - The element to copy from
 *                        If set to NULL, the element will be cleared
 * output : none
 * return : none
 ************************************************************************/
static void
raElemSet(
      IN HRA raH,
      IN int location,
      IN void* src)
{
  if (src)
    memcpy(ELEM_DATA(raH, location), (void *)src, ((raHeader *)raH)->sizeofElement);
  else
    memset(ELEM_DATA(raH, location), 0, ((raHeader *)raH)->sizeofElement);
}







/************************************************************************
 *
 *                           Public functions
 *
 ************************************************************************/


/************************************************************************
 * raConstruct
 * purpose: Create an RA object
 * input  : elemSize            - Size of elements in the RA in bytes
 *          maxNumOfElements    - Number of elements in RA
 *          logMgr              - Log manager to use
 *          name                - Name of RA (used in log messages)
 * output : none
 * return : Handle to RA constructed on success
 *          NULL on failure
 ************************************************************************/
HRA raConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN RVHLOGMGR    logMgr,
    IN const char*  name)
{
    raHeader*   ra;
    int         size;
    void*       ptr=NULL;

    if (logMgr);

    /* Make sure the size is at least 4 bytes per element and aligned on 32bits */
    if (elemSize < 4)
        size = 4;
    else
        size = elemSize;
    size = RV_ALIGN(size);

    /* Allocate the amount of memory necessary */
    if (! (ptr = malloc(raGetAllocationSize(size, maxNumOfElements))))
        return NULL;

    /* Build the RA header and elements properly */
    ra = raBuild((char *)ptr, size, maxNumOfElements, name);

    ra->log = logRegister(logMgr, "RA", "R Array");

#ifdef RV_RA_DEBUG
    logPrint(ra->log, RV_DEBUG, (ra->log, RV_DEBUG, "raConstruct (%s): %d elements of size %d (total of %d)", name, maxNumOfElements, size, maxNumOfElements*size));
#endif

    return (HRA)ra;
}


/************************************************************************
 * raDestruct
 * purpose: Free an RA object, deallocating all of its used memory
 * input  : raH     - Handle of the RA object
 * output : none
 * return : none
 ************************************************************************/
void raDestruct(HRA raH)
{
    if (raH != NULL) free((raHeader*)raH);
}


/************************************************************************
 * raClear
 * purpose: Clean an RA object from any used elements, bringing it back
 *          to the point it was when raConstruct() was called.
 * input  : raH     - Handle of the RA object
 * output : none
 * return : none
 ************************************************************************/
void
raClear(HRA raH)
{
  raHeader *ra = (raHeader *)raH;
  int i;

  if (!ra) return;

  ra->curNumOfElements = 0;
  ra->firstNodeLocation = ra->lastNodeLocation = -1;
  ra->maxUsage = 0;

  /* -- free list */
  if(ra->maxNumOfElements > 0)
      ra->firstVacantElement = 0;
  else
      ra->firstVacantElement = -1;
  ra->lastVacantElement = ra->maxNumOfElements-1;
  for (i=0; i<ra->maxNumOfElements; i++)
    ((vacantNode *)ELEM_DATA(ra, i))->nextVacant = i+1;

  if (ra->maxNumOfElements)
  {
      ((vacantNode *)ELEM_DATA(ra, ra->maxNumOfElements-1))->nextVacant = RVERROR;
      /* -- bit vector */
      memset(BIT_VECTOR(ra), 0, BIT_VECTOR_SIZE(ra->maxNumOfElements)); /* make all free */
  }
}


/************************************************************************
 * raSetCompareFunc
 * purpose: Set the compare function to use in raFind()
 * input  : raH     - Handle of the RA object
 *          func    - Compare function to use
 * output : none
 * return : none
 ************************************************************************/
void raSetCompareFunc(IN HRA raH, IN RAECompare func)
{
    raHeader *ra = (raHeader *)raH;
    ra->compare = func;
}


/************************************************************************
 * raSetPrintFunc
 * purpose: Set the print function to use in raPrint()
 * input  : raH     - Handle of the RA object
 *          func    - Print function to use
 * output : none
 * return : none
 ************************************************************************/
void raSetPrintFunc(IN HRA raH, IN RAEFunc func)
{
    raHeader *ra = (raHeader *)raH;
    ra->print = func;
}


/************************************************************************
 * raGetPrintFunc
 * purpose: Set the print function to use in raPrint()
 * input  : raH     - Handle of the RA object
 * output : none
 * return : Print function used by RA (given by raSetPrintFunc)
 ************************************************************************/
RAEFunc raGetPrintFunc(IN HRA raH)
{
    return ((raHeader *)raH)->print;
}


/************************************************************************
 * raAdd
 * purpose: Allocate an element in RA for use, setting its value to
 *          the given pointer
 * input  : raH     - Handle of the RA object
 *          elem    - Value to set in the new allocated element
 *                    If this pointer is set to NULL, the element in RA
 *                    is filled with zeros
 * output : none
 * return : RVERROR on failure
 *          Non-negative value representing the location of the added
 *          element.
 ************************************************************************/
int raAdd(IN HRA raH, IN RAElement elem)
{
    raHeader *ra = (raHeader *)raH;
    int vLocation;

    /* See if there's any place in this RA */
    if ( (vLocation = ra->firstVacantElement) == RVERROR)
    {
        logPrint(ra->log, RV_ERROR,
                 (ra->log, RV_ERROR,
                 "raAdd (%s): Array full (%d elements)", ra->name, ra->maxNumOfElements));
        printf("raAdd (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        return RVERROR;
    }

    /* Get the element from list of vacant elements and fix that list */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == RVERROR) ra->lastVacantElement = RVERROR;
    setBit(BIT_VECTOR(ra), vLocation, TRUE); /* make it occupied */

    /* Make sure we set the element's value properly */
    raElemSet(raH, vLocation, elem);

    /* Set statistical information */
    ra->curNumOfElements++;
    if (ra->curNumOfElements > ra->maxUsage) ra->maxUsage=ra->curNumOfElements;

    /* Return the location */
    return vLocation;
}


/************************************************************************
 * raAddExt
 * purpose: Allocate an element in RA for use, without initializing its
 *          value.
 * input  : raH         - Handle of the RA object
 * output : pOutElem    - Pointer to the element added.
 *                        If given as NULL, it will not be set
 * return : RVERROR on failure
 *          Non-negative value representing the location of the added
 *          element.
 ************************************************************************/
int raAddExt(IN HRA raH, OUT RAElement *pOutElem)
{
    raHeader *ra = (raHeader *)raH;
    int vLocation;

    /* See if there's any place in this RA */
    if ( (vLocation = ra->firstVacantElement) == RVERROR)
    {
        logPrint(ra->log, RV_ERROR,
                 (ra->log, RV_ERROR,
                 "raAddExt (%s): Array full (%d elements)", ra->name, ra->maxNumOfElements));
        printf("raAddExt (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        if (pOutElem != NULL) *pOutElem = NULL;
        return RVERROR;
    }

    /* Get the element from list of vacant elements and fix that list */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == RVERROR) ra->lastVacantElement = RVERROR;
    setBit(BIT_VECTOR(ra), vLocation, TRUE); /* make it occupied */

    /* Set statistical information */
    ra->curNumOfElements++;
    if (ra->curNumOfElements > ra->maxUsage) ra->maxUsage=ra->curNumOfElements;

    /* Return the location */
    if (pOutElem != NULL) *pOutElem = (RAElement)ELEM_DATA(raH, vLocation);
    return vLocation;
}


/************************************************************************
 * raDeleteLocation
 * purpose: Delete an element from RA by its location
 * input  : raH      - Handle of the RA object
 * output : location - Location of the element in RA
 * return : RVERROR on failure
 ************************************************************************/
int raDeleteLocation(IN HRA raH, IN int location)
{
    raHeader *ra = (raHeader *)raH;

#if 0
#ifdef RV_RA_DEBUG
    /* todo: if we're going to change rtGet() to accept only valid parameters */
    /* Some validity checks for debug mode */
    if (ra == NULL) return RVERROR;
    if (location < 0 || location > ra->maxNumOfElements)
    {
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raDeleteLocation (%s): Bad location %d [0-%d]",
                 ra->name, location, ra->maxNumOfElements));
        return RVERROR;
    }

    if (raElemIsVacant(raH, location) == TRUE)
    {
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raDeleteLocation (%s): Element %d already vacant",
                 ra->name, location));
        return RVERROR;
    }
#endif
#endif
    if (location < 0 || location > ra->maxNumOfElements) return RVERROR;
    if (raElemIsVacant(raH, location) == TRUE) return RVERROR;

    /* Add the element to the vacant list */
    ra->curNumOfElements--;
    ((vacantNode *)ELEM_DATA(ra, location))->nextVacant = RVERROR;
    if (ra->lastVacantElement != RVERROR)
        ((vacantNode *)ELEM_DATA(ra, ra->lastVacantElement))->nextVacant = location;
    else
    {
        /* ra->lastVacantElement==RVERROR means that ra->firstVacantElement is also
           equal to RVERROR, so after this delete location just only one element
           will be in this RA, in such situation both ra->firstVacantElement &
           ra->lastVacantElement should point to the deleted element
        */
        ra->firstVacantElement = location;
    }

    ra->lastVacantElement = location;
    setBit(BIT_VECTOR(ra), location, FALSE); /* make it free */

    return 0;
}


#ifdef RV_RA_DEBUG
/************************************************************************
 * raGet
 * purpose: Get the pointer to an RA element by its location
 * input  : raH      - Handle of the RA object
 * output : location - Location of the element in RA
 * return : Pointer to the RA element
 *          In release mode, no checks are done for the validity or the
 *          vacancy of the location.
 ************************************************************************/
RAElement raGet(IN HRA raH, IN int location)
{
    raHeader* ra = (raHeader *)raH;

    if (ra == NULL) return NULL;

    if ((location < 0) || (unsigned)location >= (unsigned)(ra->maxNumOfElements))
    {
#if 0
      /* todo: if we're going to change rtGet() to accept only valid parameters */
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raGet (%s): Bad location %d [0-%d]",
                 ra->name, location, ra->maxNumOfElements));
#endif
        return NULL;
    }

    if (raElemIsVacant(raH, location) == TRUE)
    {
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raGet (%s): Element %d is vacant",
                 ra->name, location));
        return NULL;
    }

    return (RAElement)ELEM_DATA(raH, location);
}
#endif  /* RV_RA_DEBUG */


/************************************************************************
 * raGetByPtr
 * purpose: Get the location of an RA element by its element pointer
 * input  : raH     - Handle of the RA object
 * output : ptr     - Pointer to the RA element's value
 * return : Location of the element on success
 *          RVERROR on failure
 ************************************************************************/
int raGetByPtr(IN HRA raH, IN void *ptr)
{
    raHeader *ra = (raHeader *)raH;
    int location;
    int position;

#ifdef RV_RA_DEBUG
    /* Make sure the given pointer is a valid element */
    if (!ra) return RVERROR;

    if (((char *)ptr < (char *)(ra->arrayLocation)) ||
        ((char *)ptr > (char *)(ra->arrayLocation) + ra->maxNumOfElements*(ra->sizeofElement)))
    {
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raGetByPtr (%s): Pointer 0x%x out of bounds [0x%x-0x%x]",
                 ra->name, (UINT32)ptr, (UINT32)ra->arrayLocation,
                 (UINT32)((char *)(ra->arrayLocation) + ra->maxNumOfElements*(ra->sizeofElement))));
        return RVERROR;
    }
#endif

    /* Calculate the location of the element */
    location = (int)((char *)ptr - ra->arrayLocation);

#ifdef RV_RA_DEBUG
    /* Make sure the pointer is aligned properly */
    if (location % ra->sizeofElement != 0)
    {   /* alignment */
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raGetByPtr (%s): Pointer 0x%x not aligned",
                 ra->name, (UINT32)ptr));

        return RVERROR;
    }
#endif

    position = location/(ra->sizeofElement);

#ifdef RV_RA_DEBUG
    /* Make sure element is not vacant */
    if (raElemIsVacant(raH, position))
    {
        logPrint(ra->log, RV_EXCEP,
                 (ra->log, RV_EXCEP, "raGetByPtr (%s): Element %d is vacant",
                 ra->name, position));

        return RVERROR;
    }
#endif

    return position;
}



/************************************************************************
 * raElemIsVacant
 * purpose: Check if an element is vacant inside RA or not
 * input  : raH         - Handle of the RA object
 *          location    - Location of RA element to check
 * output : none
 * return : TRUE if element is vacant
 *          FALSE if element is used
 *          RVERROR on failure
 ************************************************************************/
int raElemIsVacant(
    IN HRA raH,
    IN int location)
{
  raHeader *ra = (raHeader *)raH;

  if (!ra || location<0 || location>ra->maxNumOfElements) return RVERROR;
  return ((getBit((BIT_VECTOR(ra)), (location))) != 0) ? (FALSE):(TRUE);
}



/************************************************************************
 * raGetName
 * purpose: Get the name of the RA object
 * input  : raH         - Handle of the RA object
 * output : none
 * return : Name of the RA
 ************************************************************************/
const char* raGetName(IN HRA raH)
{
    raHeader *ra = (raHeader *)raH;

    return (const char *)(ra->name);
}



/************************************************************************
 * raGetNext
 * purpose: Get the next used element in RA.
 *          This function can be used to implement search or "doall"
 *          functions on RA.
 * input  : raH - Handle of the RA object
 *          cur - Current RA location whose next we're looking for
 *                If negative, then emaGetNext() will return the first
 *                used element.
 * output : none
 * return : Location of the next used element on success
 *          Negative value when no more used elements are left
 ************************************************************************/
int raGetNext(
    IN HRA  raH,
    IN int  cur)
{
    raHeader* ra = (raHeader *)raH;
    int i;
    if (cur < 0)
        i = 0;
    else
        i = cur + 1;
    if (i >= ra->maxNumOfElements) return RVERROR; /* out of bounds */

    while ((i < ra->maxNumOfElements) && raElemIsVacant(raH, i))
        i++;

    if (i < ra->maxNumOfElements)
        return i;
    else
        return RVERROR;
}


/************************************************************************
 * raGetStatistics
 * purpose: Get statistics information about RA.
 * input  : raH         - Handle of the RA object
 * output : stats       - Statistics information
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int raGetStatistics(IN HRA raH, OUT rvStatistics* stats)
{
    raHeader* ra = (raHeader *)raH;
    if (ra == NULL) return RVERROR;

    stats->cur      = ra->curNumOfElements;
    stats->maxUsage = ra->maxUsage;
    stats->max      = ra->maxNumOfElements;

    return 0;
}


/* todo: continue from here */




/*
  Desc: Returns location of first element.
  */
int
raFirst(HRA raH)
{
  raHeader *ra = (raHeader *)raH;
  if (!raH) return RVERROR;
  return ra->firstNodeLocation;
}

/*
  Desc: Returns location of last element.
  */
int
raLast(HRA raH)
{
  raHeader *ra = (raHeader *)raH;
  if (!raH) return RVERROR;
  return ra->lastNodeLocation;
}

/*
  Desc: Set location of first element
  */
int
raSetFirst(HRA raH, int location)
{
  raHeader *ra = (raHeader *)raH;
  if (!raH) return RVERROR;
  ra->firstNodeLocation = location;
  return location;
}

/*
  Desc: Set location of last element
  */
int
raSetLast(HRA raH, int location)
{
  raHeader *ra = (raHeader *)raH;
  if (!raH) return RVERROR;
  ra->lastNodeLocation = location;
  return location;
}

/* Desc: returns current number of elements.
 */
int
raCurSize(HRA raH)
{
  raHeader *ra = (raHeader *)raH;
  if (!raH) return RVERROR;
  return ra->curNumOfElements;
}


/* Desc: returns number of elements that can be added to array.
 */
int
raFreeSize(HRA raH)
{
  if (!raH) return RVERROR;
  return ((raHeader *)raH)->maxNumOfElements -
         ((raHeader *)raH)->curNumOfElements;
}

/* Desc: returns maximum number of elements.
 */
int
raMaxSize(HRA raH)
{
  if (!raH) return RVERROR;
  return ((raHeader *)raH)->maxNumOfElements;
}




int /* Maximum usage of array */
raMaxUsage(HRA raH)
{
  if (!raH) return RVERROR;
  return ((raHeader *)raH)->maxUsage;
}





/* returns size of element */
int
raElemSize(HRA raH)
{
  if (!raH) return RVERROR;
  return ((raHeader *)raH)->sizeofElement;
}


RAECompare
raFCompare(HRA raH)
{
  if (!raH) return NULL;
  return ((raHeader *)raH)->compare;
}





/* Desc: find element by key.
   Returns: location of element or RVERROR.
   */
int
raFind(HRA raH, void *param)
{
  raHeader *ra = (raHeader *)raH;
  int i;

  if (!ra) return RVERROR;

  for (i=0; i<ra->maxNumOfElements; i++)
    if (!raElemIsVacant(raH, i)) {
      if ((ra->compare && ra->compare(ELEM_DATA(ra, i), param)) ||
      ELEM_DATA(ra, i) == param) /* address comparison if no compare fuction */
    return i;
    }
  return RVERROR;
}


int
raCompare(HRA raH, RAECompare compare, void *param)
{
  raHeader *ra = (raHeader *)raH;
  int i;

  /*if (!ra) return RVERROR;*/

  for (i=0; i<ra->maxNumOfElements; i++)
    if (!raElemIsVacant(raH, i)) {
      if ((compare && compare(ELEM_DATA(ra, i), param)) ||
      ELEM_DATA(ra, i) == param) /* address comparison if no compare fuction */
    return i;
    }
  return RVERROR;
}

#if 0

void
raPrint(HRA raH, void *param)
{
  raHeader *ra = (raHeader *)raH;
  int i;

  if (!ra) return;
  for (i=0; i<ra->maxNumOfElements; i++)
    if (!raElemIsVacant(raH, i) && ra->print)
      ra->print(ELEM_DATA(ra, i), param);
}

#endif





#ifdef __cplusplus
}
#endif



