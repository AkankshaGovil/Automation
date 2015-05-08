
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
  ema.h

  RADVISION static array implementation.

  The EMA module provides the Enhanced Memory Allocation services that RA provides, with
  several additional features:

  1. Knowledge about the type of handle in EMA.
     An EMA element can be checked for its type using emaGetType()
  2. Reference counts
     Each element has a reference count that can be accessed through emaIncRefCount() and
     emaDefRefCount().
  3. Auto-lock
     EMA will automatically lock itself on functions that need thread-safety (such as
     emaAdd).
  4. Explicit element locks

  EMA can also be used to link elements from different types to the same locks are reference
  counts. It is done in CM to link the channels of the call to the call, allowing easier
  locking and reference counting mechanisms.

  Written by Tsahi Levent-Levi

*/


#ifndef _EMA_H
#define _EMA_H

#ifdef __cplusplus
extern "C" {
#endif


#include <rvinternal.h>
#include <log.h>


/* Handle to an EMA object */
DECLARE_OPAQUE(HEMA);


/* EMA element pointer declaration */
typedef void *EMAElement;


/************************************************************************
 * emaStatistics struct
 * elems        - Statistics about the elements inside EMA
 * numMarked    - Number of items that are marked and deleted, not released
 *                yet.
 ************************************************************************/
typedef struct
{
    rvStatistics    elems;
    UINT32          numMarked;
} emaStatistics;


/************************************************************************
 * emaLockType enum
 * emaNoLocks       - EMA is not using locks at all. It does use referece
 *                    counts. It will only use a general lock to lock when
 *                    adding and deleting elements
 * emaNormalLocks   - EMA uses locks and reference counts in a "normal" way
 * emaLinkedLocks   - EMA's locks and reference counts are linked to another
 *                    EMA object, which holds different types of information.
 ************************************************************************/
typedef enum
{
    emaNoLocks,
    emaNormalLocks,
    emaLinkedLocks
} emaLockType;


/************************************************************************
 * EMAFunc
 * purpose: Definition of a general function on an EMA element
 * input  : elem    - Element to function on
 *          param   - Context to use for it
 * output : none
 * return : Pointer for the context to use on the next call to this
 *          EMAFunc.
 ************************************************************************/
typedef void*(*EMAFunc)(EMAElement elem, void *param);




/************************************************************************
 * emaConstruct
 * purpose: Create an EMA object
 * input  : elemSize            - Size of elements in the EMA in bytes
 *          maxNumOfElements    - Number of elements in EMA
 *          lockType            - Type of locking mechanism to use
 *          logMgr              - Log manager to use
 *          name                - Name of EMA (used in log messages)
 *          type                - Integer representing the type of objects
 *                                stored in this EMA.
 *          userData            - User related information associated with
 *                                this EMA object.
 *          instance            - Instance associated with this EMA object.
 * output : none
 * return : Handle to RA constructed on success
 *          NULL on failure
 ************************************************************************/
HEMA emaConstruct(
    IN int          elemSize,
    IN int          maxNumOfElements,
    IN emaLockType  lockType,
    IN RVHLOGMGR    logMgr,
    IN const char*  name,
    IN UINT32       type,
    IN void*        userData,
    IN void const*  instance);


/************************************************************************
 * emaDestruct
 * purpose: Free an EMA object, deallocating all of its used memory
 * input  : emaH   - Handle of the EMA object
 * output : none
 * return : none
 ************************************************************************/
void emaDestruct(IN HEMA emaH);


/************************************************************************
 * emaAdd
 * purpose: Allocate an element in EMA for use, without initializing its
 *          value.
 *          This automatically locks the EMA object.
 * input  : emaH        - Handle of the EMA object
 *          appHandle   - Application's handle for the element
 * output : none
 * return : Pointer to element added on success
 *          NULL on failure
 ************************************************************************/
EMAElement emaAdd(IN HEMA emaH, IN void* appHandle);


/************************************************************************
 * emaDelete
 * purpose: Delete an element from EMA
 * input  : elem    - Element to delete
 * return : RVERROR on failure
 ************************************************************************/
int emaDelete(IN EMAElement elem);


/************************************************************************
 * emaLinkToElement
 * purpose: Link an EMA element to another element, from a different
 *          EMA construct. This function should be used when the EMA we're
 *          dealing with was created with emaLinkedLocks. This function
 *          allows the element to use a different element's lock.
 *          This function will only work if the element has no reference
 *          count at present.
 * input  : elem        - Element to link
 *          otherElem   - Element we're linking to. Should be constructed
 *                        with emaNormalLocks or linked to such element.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int emaLinkToElement(IN EMAElement elem, IN EMAElement otherElem);


/************************************************************************
 * emaLock
 * purpose: Lock an element in EMA for use from the executing thread only
 *          This function will succeed only if the element exists
 * input  : elem    - Element to lock
 * output : none
 * return : TRUE    - When the element exists and was locked
 *          FALSE   - When the element doesn't exist (NULL are was deleted)
 *                    In this case, there's no need to call emaUnlock().
 ************************************************************************/
BOOL emaLock(IN EMAElement elem);


/************************************************************************
 * emaUnlock
 * purpose: Unlock an element in EMA that were previously locked by
 *          emaLock() from the same thread
 * input  : elem    - Element to unlock
 * output : none
 * return : TRUE    if element still exists
 *          FALSE   if element was deleted and is not valid anymore
 *          RVERROR on failure
 ************************************************************************/
int emaUnlock(IN EMAElement elem);


/************************************************************************
 * emaMark
 * purpose: Mark an element in EMA for use, not letting anyone delete
 *          this element until it is release.
 *          This automatically locks the EMA object.
 * input  : elem    - Element to mark
 * output : none
 * return : RVERROR on failure
 ************************************************************************/
int emaMark(IN EMAElement elem);


/************************************************************************
 * emaRelease
 * purpose: Release an element in EMA after it was marked using
 *          emaMark(), returning an indication if this element
 *          still exists.
 *          This automatically locks the EMA object.
 * input  : elem    - Element to mark
 * output : none
 * return : TRUE    if element still exists
 *          FALSE   if element was deleted and is not valid anymore
 *          RVERROR on failure
 ************************************************************************/
int emaRelease(IN EMAElement elem);


/************************************************************************
 * emaWasDeleted
 * purpose: Check if an element in EMA was deleted after a call to
 *          emaMark().
 * input  : elem    - Element to mark
 * output : none
 * return : TRUE    if element was deleted
 *          FALSE   if element still exists
 ************************************************************************/
BOOL emaWasDeleted(IN EMAElement elem);


/************************************************************************
 * emaPrepareForCallback
 * purpose: Prepare an element in EMA for use in a callback to the app.
 *          This function will make sure the element is unlocked the necessary
 *          number of times, and then marked once (so the app won't delete
 *          this element).
 *          emaReturnFromCallback() should be called after the callback,
 *          with the return value of this function.
 * input  : elem    - Element to prepare
 * output : none
 * return : Number of times the element was locked on success
 *          RVERROR on failure
 ************************************************************************/
int emaPrepareForCallback(IN EMAElement elem);


/************************************************************************
 * emaReturnFromCallback
 * purpose: Make sure the EMA element knows it has returned from a
 *          callback. This function will ensure that the element is
 *          locked again with the specified number of times. It will also
 *          release the element, and if timersLocked==0, and the element
 *          was deleted by the app in the callback, the element will also
 *          be permanently deleted.
 * input  : elem    - Element to prepare
 * output : none
 * return : TRUE    if element still exists
 *          FALSE   if element was deleted and is not valid anymore
 *          RVERROR on failure
 ************************************************************************/
int emaReturnFromCallback(IN EMAElement elem, IN int timesLocked);


/************************************************************************
 * emaSetApplicationHandle
 * purpose: Set the application handle of an element in EMA
 * input  : elem        - Element in EMA
 *          appHandle   - Application's handle for the element
 * output : none
 * return : RVERROR on failure
 ************************************************************************/
int emaSetApplicationHandle(IN EMAElement elem, IN void* appHandle);


 /************************************************************************
 * emaGetApplicationHandle
 * purpose: Get the application's handle of an element in EMA
 * input  : elem        - Element in EMA
 * output : appHandle   - Application's handle for the element
 * return : Pointer to the application handle
 *          NULL on failure
 ************************************************************************/
void* emaGetApplicationHandle(IN EMAElement elem);


/************************************************************************
 * emaGetType
 * purpose: Return the type of the element inside the EMA object.
 *          This is the type given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's type on success
 *          0 on failure
 ************************************************************************/
UINT32 emaGetType(IN EMAElement elem);


/************************************************************************
 * emaGetUserData
 * purpose: Return the user related data of the element inside the EMA
 *          object. This is the userData given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's user data pointer on success
 *          NULL on failure
 ************************************************************************/
void* emaGetUserData(IN EMAElement elem);

/************************************************************************
 * emaGetUserDataByInstance
 * purpose: Return the user related data inside the EMA object, by the
 *          EMA instance returned by emaConstruct().
 *          This is the userData given in emaConstruct().
 * input  : emaH   - handle to the EMA
 * output : none
 * return : The user data pointer on success
 *          NULL on failure
 ************************************************************************/
void* emaGetUserDataByInstance(IN HEMA emaH);

/************************************************************************
 * emaGetInstance
 * purpose: Return the instance of this EMA element.
 *          This is the instance given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's instance on success
 *          NULL on failure
 ************************************************************************/
void const* emaGetInstance(IN EMAElement elem);


/************************************************************************
 * emaGetObject
 * purpose: Return the EMA object this element is in
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's EMA object on success
 *          NULL on failure
 ************************************************************************/
HEMA emaGetObject(IN EMAElement elem);


/************************************************************************
 * emaDoAll
 * purpose: Call a function on all used elements in EMA
 * input  : emaH        - Handle of the EMA object
 *          func        - Function to execute on all elements
 *          param       - Context to use when executing the function
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int emaDoAll(
    IN HEMA     emaH,
    IN EMAFunc  func,
    IN void*    param);


/************************************************************************
 * emaGetNext
 * purpose: Get the next used element in EMA.
 *          This function can be used to implement search or "doall"
 *          functions on EMA.
 * input  : emaH        - Handle of the EMA object
 *          cur         - Current EMA element whose next we're looking for
 *                        If NULL, then emaGetNext() will return the first
 *                        used element.
 * output : none
 * return : Pointer to the next used element on success
 *          NULL when no more used elements are left
 ************************************************************************/
EMAElement emaGetNext(
    IN HEMA         emaH,
    IN EMAElement   cur);


/************************************************************************
 * emaGetIndex
 * purpose: Returns the index of the element in the ema
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's index on success
 *          RVERROR on failure
 ************************************************************************/
int emaGetIndex(IN EMAElement elem);


/************************************************************************
 * emaGetStatistics
 * purpose: Get statistics information about EMA.
 *          The number of used items also includes those deleted, but still
 *          marked.
 * input  : emaH        - Handle of the EMA object
 * output : stats       - Statistics information
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int emaGetStatistics(IN HEMA emaH, OUT emaStatistics* stats);


/************************************************************************
 * emaIsVacant
 * purpose: Returns whether the given object is free for allocation
 * input  : elem    - Element in EMA
 * output : none
 * return : TRUE  - if the elemnt is not allocated
 *			FALSE - otherwise
 ************************************************************************/
int emaIsVacant(IN EMAElement elem);



#ifdef RV_EMA_DEBUG

#define emaLock(elem) emaLockDebug(elem, __FILE__, __LINE__)
#define emaUnlock(elem) emaUnlockDebug(elem, __FILE__, __LINE__)
#define emaMark(elem) emaMarkDebug(elem, __FILE__, __LINE__)
#define emaRelease(elem) emaReleaseDebug(elem, __FILE__, __LINE__)
#define emaPrepareForCallback(elem) emaPrepareForCallbackDebug(elem, __FILE__, __LINE__)
#define emaReturnFromCallback(elem, timesLocked) emaReturnFromCallbackDebug(elem, timesLocked, __FILE__, __LINE__)

BOOL emaLockDebug(IN EMAElement elem, IN const char* filename, IN int lineno);
int emaUnlockDebug(IN EMAElement elem, IN const char* filename, IN int lineno);
int emaMarkDebug(IN EMAElement elem, IN const char* filename, IN int lineno);
int emaReleaseDebug(IN EMAElement elem, IN const char* filename, IN int lineno);
int emaPrepareForCallbackDebug(IN EMAElement elem, IN const char* filename, IN int lineno);
int emaReturnFromCallbackDebug(IN EMAElement elem, IN int timesLocked, IN const char* filename, IN int lineno);

#endif  /* RV_EMA_DEBUG */




#ifdef __cplusplus
}
#endif

#endif  /* _EMA_H */


