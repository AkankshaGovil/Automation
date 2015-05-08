
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
  ra.h

  RADVISION static array implementation.

  The RA module provides basic memory allocation services to the rest of the
  stack moduels.
  RA is designed to be fast. Most validity checks are not done in release modes
  and on errors they send exception messages to the log.


  Ron Shpilman 28 Jan. 1996
  Documented: Tsahi Levent-Levi

*/


#ifndef _RA_
#define _RA_

#ifdef __cplusplus
extern "C" {
#endif


#include <rvinternal.h>
#include <log.h>


/* Handle to an RA object */
DECLARE_OPAQUE(HRA);


/* RA element pointer declaration */
typedef void *RAElement;


/************************************************************************
 * RAECompare
 * purpose: Definition of compare function for RA elements
 * input  : element1    - The element to compare
 *          param       - Parameter given to raCompare function as context
 * output : none
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
typedef BOOL (*RAECompare)(RAElement element1, void *param);


/************************************************************************
 * RAEFunc
 * purpose: Definition of a general function on an RA element
 * input  : elem    - Element to function on
 *          param   - Context to use for it
 * output : none
 * return : Pointer for the context to use on the next call to this
 *          RAEFunc.
 ************************************************************************/
typedef void*(*RAEFunc)(RAElement elem, void *param);



/************************************************************************
 * raHeader struct
 * This struct holds an RA object's main information.
 * This struct is declared in the header file for the macro support.
 * name                 - Name of allocation
 * arrayLocation        - Pointer to the location of the actual data elements
 * firstVacantElement   - First vacant element in the array's free list
 * lastVacantElement    - Last vacant element in the array's free list
 * maxElems             - Maximum number of elements
 * curElems             - Current number of elements
 * maxUsage             - Peak value of curElems
 * sizeofElement        - Size of a single element inside RA
 * log                  - Log handle to use for printouts
 * compare              - Compare function used by raFind()
 * print                - Print function used by raPrint()
 *
 * The following 2 fields are only here for RLIST's uses:
 * firstNodeLocation    - First node in list
 * lastNodeLocation     - Last node in list
 ************************************************************************/
typedef struct {
    char            name[32];
    char*           arrayLocation;
    int             firstVacantElement;
    int             lastVacantElement;
    int             maxNumOfElements;
    int             curNumOfElements;
    unsigned int    sizeofElement;
    int             maxUsage;
    RVHLOG          log;
    RAECompare      compare;
    RAEFunc         print;

    int             firstNodeLocation;
    int             lastNodeLocation;
} raHeader;





/*********** array functions **************/

#define ELEM_DATA(ra, i) \
    ((char *) (((raHeader*)(ra))->arrayLocation + (i)*(((raHeader*)(ra))->sizeofElement)))






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
    IN const char*  name);


/************************************************************************
 * raDestruct
 * purpose: Free an RA object, deallocating all of its used memory
 * input  : raH     - Handle of the RA object
 * output : none
 * return : none
 ************************************************************************/
void raDestruct(IN HRA raH);


/************************************************************************
 * raClear
 * purpose: Clean an RA object from any used elements, bringing it back
 *          to the point it was when raConstruct() was called.
 * input  : raH     - Handle of the RA object
 * output : none
 * return : none
 ************************************************************************/
void raClear(IN HRA raH);


/************************************************************************
 * raSetCompareFunc
 * purpose: Set the compare function to use in raFind()
 * input  : raH     - Handle of the RA object
 *          func    - Compare function to use
 * output : none
 * return : none
 ************************************************************************/
void raSetCompareFunc(IN HRA raH, IN RAECompare func);


/************************************************************************
 * raSetPrintFunc
 * purpose: Set the print function to use in raPrint()
 * input  : raH     - Handle of the RA object
 *          func    - Print function to use
 * output : none
 * return : none
 ************************************************************************/
void raSetPrintFunc(IN HRA raH, IN RAEFunc func);


/************************************************************************
 * raGetPrintFunc
 * purpose: Set the print function to use in raPrint()
 * input  : raH     - Handle of the RA object
 * output : none
 * return : Print function used by RA (given by raSetPrintFunc)
 ************************************************************************/
RAEFunc raGetPrintFunc(IN HRA raH);


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
int raAdd(IN HRA raH, IN RAElement elem);


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
int raAddExt(IN HRA raH, OUT RAElement *pOutElem);


/************************************************************************
 * raDeleteLocation
 * purpose: Delete an element from RA by its location
 * input  : raH      - Handle of the RA object
 * output : location - Location of the element in RA
 * return : RVERROR on failure
 ************************************************************************/
int raDeleteLocation(IN HRA raH, IN int location);


/************************************************************************
 * raGet
 * purpose: Get the pointer to an RA element by its location
 * input  : raH      - Handle of the RA object
 * output : location - Location of the element in RA
 * return : Pointer to the RA element
 *          In release mode, no checks are done for the validity or the
 *          vacancy of the location.
 ************************************************************************/
#ifdef RV_RA_DEBUG
RAElement raGet(IN HRA raH, IN int location);
#else
#define raGet(raH, location) \
    (RAElement)(((unsigned)(location) >= (unsigned)(((raHeader*)raH)->maxNumOfElements)) ?  \
        NULL : ELEM_DATA((raH), (location)))
#endif


/************************************************************************
 * raGetByPtr
 * purpose: Get the location of an RA element by its element pointer
 * input  : raH     - Handle of the RA object
 * output : ptr     - Pointer to the RA element's value
 * return : Location of the element on success
 *          RVERROR on failure
 ************************************************************************/
int raGetByPtr(IN HRA raH, IN void *ptr);


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
    IN int location);


/************************************************************************
 * raGetName
 * purpose: Get the name of the RA object
 * input  : raH         - Handle of the RA object
 * output : none
 * return : Name of the RA
 ************************************************************************/
const char* raGetName(IN HRA raH);


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
    IN int  cur);


/************************************************************************
 * raGetStatistics
 * purpose: Get statistics information about RA.
 * input  : raH         - Handle of the RA object
 * output : stats       - Statistics information
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int raGetStatistics(IN HRA raH, OUT rvStatistics* stats);



int        raCurSize       (HRA raH); /* returns current number of elements */
int        raFreeSize      (HRA raH); /* returns number of elements that can be added to array. */
int        raMaxSize       (HRA raH); /* returns maximum number of elements */
int        raElemSize      (HRA raH); /* returns size of element */
int        raMaxUsage      (HRA raH); /* Maximum usage of array */

RAECompare raFCompare      (HRA raH);

int       raFind           (HRA raH, void *param);
int       raCompare        (HRA raH, RAECompare compare, void *param);


void      raPrint          (HRA raH, void *param);

int       raFirst          (HRA raH);
int       raLast           (HRA raH);

int       raSetFirst       (HRA raH, int location);
int       raSetLast        (HRA raH, int location);






#ifdef __cplusplus
}
#endif

#endif  /* _RA_ */


