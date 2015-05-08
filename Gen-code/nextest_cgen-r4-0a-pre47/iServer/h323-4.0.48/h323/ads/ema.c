
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

#ifdef __cplusplus
extern "C" {
#endif


#include <mei.h>
#include <ra.h>
#include <ema.h>
#include <syslog.h>


/************************************************************************
 * emaObject struct
 * Holds the information of an emaObject
 * ra           - Handle to RA we're using
 * log          - Log to use for messages
 * lock         - Lock we're using
 * lockType     - Type of locking mechanism to use
 * elemSize     - Size of each element inside EMA
 * type         - Integer representing the type of objects stored in EMA
 * userData     - User related information associated with this EMA object
 * instance     - Instance associated with this EMA object
 * markCount    - Number of items currently deleted and marked
 ************************************************************************/
typedef struct
{
    HRA         ra;
    RVHLOG      log;
    HMEI        lock;
    emaLockType lockType;
    UINT32      elemSize;
    UINT32      type;
    void*       userData;
    void const* instance;
    UINT32      markCount;
} emaObject;


/************************************************************************
 * emaElem struct
 * Holds the information of an ema element
 * debug1       - Debugging bytes
 * flags        - Reference count and locks count
 *                It also holds a bit indicating if element was deleted
 *                This MUST be the first element when not used RA_EMA_DEBUG!
 *                Otherwise, the function emaLock() won't work properly.
 * ema          - Pointer to the EMA object
 *                When this pointer is NULL, it indicates that the element
 *                was deleted. We use this for emaLock().
 * appHandle    - Application's handle of the EMA object
 * debug2       - Debugging bytes
 *
 * Note: The mutex for the element is held in the end of the allocation
 *       if using emaNormalLocks. If emaLinkedLocks is used, then in the
 *       end of the allocation we'll have to pointer to the linked element
 *       in another EMA construct.
 ************************************************************************/
typedef struct
{
#ifdef RV_EMA_DEBUG
    UINT32      debug1;
#endif
    UINT32      flags;
    emaObject*  ema;
    void*       appHandle;
#ifdef RV_EMA_DEBUG
    UINT32      debug2;
#endif
} emaElem;



/* Bytes used when debuging memory allocations */
#define EMA_DEBUG_BYTES (0xdeadbeef)

/* Indication in the reference count that this element was deleted */
#define EMA_ALREADY_DELETED     0x80000000

/* Indication of the reference count's actual value */
#define EMA_GET_REFERENCE(elem)         ((elem)->flags & 0x0000ffff)
#define EMA_INC_REFERENCE(elem,count)   ((elem)->flags += (UINT16)(count))
#define EMA_DEC_REFERENCE(elem,count)   ((elem)->flags -= (UINT16)(count))

/* Indication of the locks count's actual value */
#define EMA_GET_LOCKS(elem)             (((elem)->flags & 0x7fff0000) >> 16)
#define EMA_INC_LOCKS(elem,count)       ((elem)->flags += (((UINT32)(count)) << 16))
#define EMA_DEC_LOCKS(elem,count)       ((elem)->flags -= (((UINT32)(count)) << 16))



/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * emaDeleteElement
 * purpose: Delete an element used by ema.
 * input  : ema         - EMA object used
 *          rElem       - Element to delete
 *          location    - Location of element deleted
 *          decCount    - TRUE if we need to decrease count of marked/deleted elements
 *          functionName- Name of function that called this one - used for log
 * output : none
 * return : none
 ************************************************************************/
static void emaDeleteElement(
    IN emaObject*   ema,
    IN emaElem*     rElem,
    IN int          location,
    IN BOOL         decCount,
    IN const char*  functionName)
{
    BOOL locked = TRUE;
    EMAElement parent = NULL;
    if (functionName);

    /* First of all, let's lock the element and remove the reference to ema on it.
       Doing this, allows us to stay blocked on an emaLock() call while trying to
       delete the element */
    switch (ema->lockType)
    {
        case emaNoLocks:        break;
        case emaNormalLocks:    meiEnter((HMEI)((char*)rElem + sizeof(emaElem) + ema->elemSize)); break;
        case emaLinkedLocks:
        {
            parent = *((EMAElement*)((char*)rElem + sizeof(emaElem) + ema->elemSize));
            if (parent != NULL)
                locked = emaLock(parent);
            else
                locked = FALSE;
            break;
        }
    }
    rElem->ema = NULL;
    if (locked)
    {
        switch (ema->lockType)
        {
            case emaNoLocks:        break;
            case emaNormalLocks:    meiExit((HMEI)((char*)rElem + sizeof(emaElem) + ema->elemSize)); break;
            case emaLinkedLocks:    emaUnlock(parent); break;
        }
    }

    /* Do all the rest here */
    meiEnter(ema->lock);

    if (decCount)
        ema->markCount--;

    raDeleteLocation(ema->ra, location);

    meiExit(ema->lock);

    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "%s (%s): 0x%p deleted (location=%d)",
             functionName, raGetName(ema->ra), rElem, location));
}






/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


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
    IN void const*  instance)
{
    emaObject* ema;
    int allocSize = 0;

    /* Allocate the object */
    ema = (emaObject *)malloc(sizeof(emaObject));
    if (ema == NULL) return NULL;

    /* Remember the type of elements stored */
    ema->type = type;
    ema->userData = userData;
    ema->instance = instance;

    /* Create a log handle for our use */
    ema->log = logRegister(logMgr, "EMA", "Enhanced Memory Allocator");

    /* Calculate the size of each element (32bit-aligned) */
    ema->elemSize = RV_ALIGN(elemSize);

    /* See if we need a lock for each element */
    switch (lockType)
    {
        case emaNoLocks:        allocSize = (int)(ema->elemSize + sizeof(emaElem)); break;
        case emaNormalLocks:    allocSize = (int)(ema->elemSize + sizeof(emaElem) + meiGetSize()); break;
        case emaLinkedLocks:    allocSize = (int)(ema->elemSize + sizeof(emaElem) + sizeof(EMAElement*)); break;
    }

    /* Create the RA */
    ema->ra = raConstruct(allocSize, maxNumOfElements, logMgr, name);

    /* Create the mutex */
    ema->lock = meiInit();
    ema->lockType = lockType;

#ifndef NOTHREADS
    if (lockType == emaNormalLocks)
    {
        int i;

        /* Initialize all the mutexes we need */
        for (i = 0; i < maxNumOfElements; i++)
        {
            meiInitFrom(ELEM_DATA(ema->ra, i) + sizeof(emaElem) + ema->elemSize);
        }
    }
#endif

    ema->markCount = 0;

    return (HEMA)ema;
}


/************************************************************************
 * emaDestruct
 * purpose: Free an EMA object, deallocating all of its used memory
 * input  : emaH   - Handle of the EMA object
 * output : none
 * return : none
 ************************************************************************/
void emaDestruct(IN HEMA emaH)
{
    emaObject* ema = (emaObject *)emaH;
    int numElems;

    if (ema == NULL) return;

    numElems = raCurSize(ema->ra);
    if (numElems > 0)
    {
        logPrint(ema->log, RV_WARN,
                 (ema->log, RV_WARN,
                 "emaDestruct (%s): %d elements not deleted",
                 raGetName(ema->ra), numElems));
    }

    /* Remove lock */
    if (ema->lock != NULL) meiEnd(ema->lock);

#ifndef NOTHREADS
    if (ema->lockType == emaNormalLocks)
    {
        int i;

        /* End all the mutexes we initialized */
        for (i = 0; i < raMaxSize(ema->ra); i++)
        {
            meiEndFrom((HMEI)(ELEM_DATA(ema->ra, i) + sizeof(emaElem) + ema->elemSize));
        }
    }
#endif

    /* Free any used memory and RA */
    raDestruct(ema->ra);
    free(ema);
}


/************************************************************************
 * emaAdd
 * purpose: Allocate an element in EMA for use, without initializing its
 *          value.
 *          This automatically locks the EMA object.
 * input  : emaH       - Handle of the EMA object
 *          appHandle   - Application's handle for the EMA object
 * output : none
 * return : Pointer to element added on success
 *          NULL on failure
 ************************************************************************/
EMAElement emaAdd(IN HEMA emaH, IN void* appHandle)
{
    emaObject* ema = (emaObject *)emaH;
    emaElem* elem;
    char* ptr;
    int location;

    /* Use RA for our purposes */
    meiEnter(ema->lock);
    location = raAddExt(ema->ra, (RAElement*)&elem);
    meiExit(ema->lock);

    /* Make sure it was allocated */
    if (location < 0)
    {
        logPrint(ema->log, RV_ERROR,
                 (ema->log, RV_ERROR,
                 "emaAdd (%s): Out of resources", raGetName(ema->ra)));
        return NULL;
    }

    /* Set the element's OO information */
    elem->ema = ema;
    elem->flags = 0;
    elem->appHandle = appHandle;
#ifdef RV_EMA_DEBUG
    elem->debug1 = EMA_DEBUG_BYTES;
    elem->debug2 = EMA_DEBUG_BYTES;
#endif

    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaAdd (%s): Got 0x%p (location=%d)",
             raGetName(ema->ra), elem, location));

    ptr = (char*)elem + sizeof(emaElem);

    /* Allocate a mutex if necessary */
    switch (ema->lockType)
    {
        case emaLinkedLocks: memset(ptr + ema->elemSize, 0, sizeof(EMAElement*)); break;
        default:             break;
    }

    /* Calculate and return the position of the true element */
    return (EMAElement*)ptr;
}


/************************************************************************
 * emaDelete
 * purpose: Delete an element from EMA
 * input  : elem    - Element to delete
 * return : RVERROR on failure
 ************************************************************************/
int emaDelete(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;
    int location;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Find the location */
    location = raGetByPtr(ema->ra, rElem);

    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaDelete (%s): Deleting %d,0x%p (refCount=%d)",
             raGetName(ema->ra), location, rElem,
             EMA_GET_REFERENCE(rElem)));

#ifdef RV_EMA_DEBUG
    if ((rElem->debug1 != EMA_DEBUG_BYTES) || (rElem->debug2 != EMA_DEBUG_BYTES))
    {
        logPrint(ema->log, RV_EXCEP,
                 (ema->log, RV_EXCEP, "emaDelete (%s): Someone is messing with memory 0x%p",
                 raGetName(ema->ra), rElem));
    }
#endif

    /* Check the reference count */
    if (rElem->flags == 0)
    {
        /* No one is looking for this guy - we can delete it */
        emaDeleteElement(ema, rElem, location, FALSE, "emaDelete");
    }
    else
    {
        meiEnter(ema->lock);

#ifdef RV_EMA_DEBUG
        if ((rElem->flags & EMA_ALREADY_DELETED) != 0)
        {
            logPrint(ema->log, RV_EXCEP,
                     (ema->log, RV_EXCEP, "emaDelete (%s): Deleting an element 0x%p for the second time",
                     raGetName(ema->ra), rElem));
        }
#endif
        ema->markCount++;
        rElem->flags |= EMA_ALREADY_DELETED;

        meiExit(ema->lock);
    }


    return 0;
}


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
int emaLinkToElement(IN EMAElement elem, IN EMAElement otherElem)
{
    emaObject*  ema;
    emaElem*    rElem;
    EMAElement* parent;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Make sure we've got the element */
    if (ema == NULL) return RVERROR;

    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaLinkToElement (%s): Linking 0x%p to 0x%p",
             raGetName(ema->ra), rElem, otherElem));

#ifdef RV_EMA_DEBUG
    if (ema->lockType != emaLinkedLocks)
    {
        logPrint(ema->log, RV_EXCEP,
                 (ema->log, RV_EXCEP, "emaLinkToElement (%s): This EMA cannot be linked",
                 raGetName(ema->ra)));
        return RVERROR;
    }
#endif

    /* Find place of parent */
    parent = (EMAElement*)((char*)elem + ema->elemSize);
    if (EMA_GET_REFERENCE(rElem) > 0)
    {
        logPrint(ema->log, RV_ERROR,
                 (ema->log, RV_ERROR, "emaLinkToElement (%s): Cannot link 0x%p - has a positive reference count",
                 raGetName(ema->ra), rElem));
        return RVERROR;
    }

    *parent = otherElem;
    return 0;
}


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
#ifdef RV_EMA_DEBUG
BOOL emaLockDebug(IN EMAElement elem, IN const char* filename, IN int lineno)
#else
BOOL emaLock(IN EMAElement elem)
#endif
{
    emaObject*  ema;
    emaElem*    rElem;
    BOOL        status = TRUE;
    EMAElement  parent = NULL;

    if (elem == NULL) return FALSE;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Make sure we've got the element */
    if (ema == NULL) return FALSE;

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaLock (%s): Locking 0x%p (%s:%d)",
             raGetName(ema->ra), rElem, filename, lineno));
#endif

    switch (ema->lockType)
    {
#ifdef RV_EMA_DEBUG
        case emaNoLocks:
            logPrint(ema->log, RV_EXCEP,
                     (ema->log, RV_EXCEP, "emaLock (%s): This EMA cannot be locked",
                     raGetName(ema->ra)));
            break;
#endif
        case emaNormalLocks:
            /* We lock it */
            meiEnter((HMEI)((char*)elem + ema->elemSize));

            /* Now that it's locked, see if the element still exists */
            if (rElem->ema == NULL)
            {
                /* Seems like someone has deleted this element when we were trying to lock it */
#ifdef RV_EMA_DEBUG
                logPrint(ema->log, RV_DEBUG,
                         (ema->log, RV_DEBUG, "emaLock (%s): Unlocking deleted element 0x%p",
                         raGetName(ema->ra), rElem));
#endif
                status = FALSE;

                /* Release the lock - we shouldn't go on with it */
                meiExit((HMEI)((char*)elem + ema->elemSize));
            }
            break;

        case emaLinkedLocks:
        {
            /* We lock the parent */
            parent = *((EMAElement*)((char*)elem + ema->elemSize));
            if (parent != NULL)
                status = emaLock(parent);
            else
                status = FALSE;

            if (status == FALSE)
            {
                logPrint(ema->log, RV_WARN,
                         (ema->log, RV_WARN, "emaLock (%s): Couldn't lock parent=0x%p of 0x%p for some reason",
                         raGetName(ema->ra), parent, elem));
            }
            break;
        }
        default:
            break;
    }

    /* Make sure we increment the reference count on this one */
    if (status == TRUE)
    {
        EMA_INC_REFERENCE(rElem,1);
        EMA_INC_LOCKS(rElem,1);
    }

    return status;
}


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
#ifdef RV_EMA_DEBUG
int emaUnlockDebug(IN EMAElement elem, IN const char* filename, IN int lineno)
#else
int emaUnlock(IN EMAElement elem)
#endif
{
    emaObject* ema;
    emaElem* rElem;
    int elemExists;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaUnlock (%s): Unlocking 0x%p (%s:%d)",
             raGetName(ema->ra), rElem, filename, lineno));
#endif

    EMA_DEC_REFERENCE(rElem,1);
    EMA_DEC_LOCKS(rElem,1);
    elemExists = ((rElem->flags & EMA_ALREADY_DELETED) == 0);

    /* Delete element if we're done with it */
    if (rElem->flags == EMA_ALREADY_DELETED)
    {
        int location = raGetByPtr(ema->ra, rElem);
        emaDeleteElement(ema, rElem, location, TRUE, "emaUnlock");
    }

    switch (ema->lockType)
    {
#ifdef RV_EMA_DEBUG
        case emaNoLocks:
            logPrint(ema->log, RV_EXCEP,
                     (ema->log, RV_EXCEP, "emaUnlock (%s): This EMA cannot be unlocked",
                     raGetName(ema->ra)));
            break;
#endif
        case emaNormalLocks:
            /* We lock it */
            meiExit((HMEI)((char*)elem + ema->elemSize));
            break;
        case emaLinkedLocks:
        {
            /* We lock the parent */
            int result = RVERROR;
            EMAElement  parent = *((EMAElement*)((char*)elem + ema->elemSize));
            if (parent != NULL)
                result = emaUnlock(parent);

            if (result < 0)
            {
                logPrint(ema->log, RV_WARN,
                         (ema->log, RV_WARN, "emaUnlock (%s): Couldn't unlock parent=0x%p of 0x%p for some reason",
                         raGetName(ema->ra), parent, elem));
                elemExists = result;
            }
            break;
        }
        default:
            break;
    }

    return elemExists;
}


/************************************************************************
 * emaMark
 * purpose: Mark an element in EMA for use, not letting anyone delete
 *          this element until it is release.
 *          This automatically locks the EMA object.
 * input  : elem    - Element to mark
 * output : none
 * return : RVERROR on failure
 ************************************************************************/
#ifdef RV_EMA_DEBUG
int emaMarkDebug(IN EMAElement elem, IN const char* filename, IN int lineno)
#else
int emaMark(IN EMAElement elem)
#endif
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaMark (%s): Marking 0x%p, refCount=%d (%s:%d)",
             raGetName(ema->ra), rElem, EMA_GET_REFERENCE(rElem), filename, lineno));

    if ((rElem->debug1 != EMA_DEBUG_BYTES) || (rElem->debug2 != EMA_DEBUG_BYTES))
    {
        logPrint(ema->log, RV_EXCEP,
                 (ema->log, RV_EXCEP, "emaMark (%s): Someone is messing with memory 0x%p",
                 raGetName(ema->ra), rElem));
    }
#else
    logPrint(ema->log, RV_DEBUG,
        (ema->log, RV_DEBUG, "emaMark (%s): Marking 0x%p, refCount=%d",
        raGetName(ema->ra), rElem, EMA_GET_REFERENCE(rElem)));
#endif

    /* Increase the reference count */
    switch (ema->lockType)
    {
        case emaNoLocks:        break;
        case emaNormalLocks:    meiEnter((HMEI)((char*)elem + ema->elemSize)); break;
        case emaLinkedLocks:
            emaLock(*((EMAElement*)((char*)elem + ema->elemSize)));
            emaMark(*((EMAElement*)((char*)elem + ema->elemSize)));
            break;
    }

    EMA_INC_REFERENCE(rElem,1);

    switch (ema->lockType)
    {
        case emaNoLocks:        break;
        case emaNormalLocks:    meiExit((HMEI)((char*)elem + ema->elemSize)); break;
        case emaLinkedLocks:    emaUnlock(*((EMAElement*)((char*)elem + ema->elemSize))); break;
    }

    return 0;
}


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
#ifdef RV_EMA_DEBUG
int emaReleaseDebug(IN EMAElement elem, IN const char* filename, IN int lineno)
#else
int emaRelease(IN EMAElement elem)
#endif
{
    emaObject* ema;
    emaElem* rElem;
    int elemExists;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaRelease (%s): Releasing 0x%p, refCount=%d (%s:%d)",
             raGetName(ema->ra), rElem, EMA_GET_REFERENCE(rElem), filename, lineno));

    if ((rElem->debug1 != EMA_DEBUG_BYTES) || (rElem->debug2 != EMA_DEBUG_BYTES))
    {
        logPrint(ema->log, RV_EXCEP,
                 (ema->log, RV_EXCEP, "emaRelease (%s): Someone is messing with memory 0x%p",
                 raGetName(ema->ra), rElem));
    }
#else
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaRelease (%s): Releasing 0x%p, refCount=%d",
             raGetName(ema->ra), rElem, EMA_GET_REFERENCE(rElem)));
#endif

    /* Decrease the reference count */
    switch (ema->lockType)
    {
        case emaNoLocks:        break;
        case emaNormalLocks:    meiEnter((HMEI)((char*)elem + ema->elemSize)); break;
        case emaLinkedLocks:
            emaLock(*((EMAElement*)((char*)elem + ema->elemSize)));
            emaRelease(*((EMAElement*)((char*)elem + ema->elemSize)));
            break;
    }

    EMA_DEC_REFERENCE(rElem,1);
    elemExists = ((rElem->flags & EMA_ALREADY_DELETED) == 0);

    /* Delete element if we're done with it */
    if (rElem->flags == EMA_ALREADY_DELETED)
    {
        int location = raGetByPtr(ema->ra, rElem);
        emaDeleteElement(ema, rElem, location, TRUE, "emaRelease");
    }

    switch (ema->lockType)
    {
        case emaNoLocks:        break;
        case emaNormalLocks:    meiExit((HMEI)((char*)elem + ema->elemSize)); break;
        case emaLinkedLocks:
            emaUnlock(*((EMAElement*)((char*)elem + ema->elemSize)));
            break;
    }

    return elemExists;
}


/************************************************************************
 * emaWasDeleted
 * purpose: Check if an element in EMA was deleted after a call to
 *          emaMark().
 * input  : elem    - Element to mark
 * output : none
 * return : TRUE    if element was deleted
 *          FALSE   if element still exists
 ************************************************************************/
BOOL emaWasDeleted(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

	/* NexTone: added check for NULL ema */
	if (ema != NULL)
	{
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaWasDeleted (%s): Checking 0x%p (refCount=%d)",
             raGetName(ema->ra), rElem, EMA_GET_REFERENCE(rElem)));

#ifdef RV_EMA_DEBUG
    if ((rElem->debug1 != EMA_DEBUG_BYTES) || (rElem->debug2 != EMA_DEBUG_BYTES))
    {
        logPrint(ema->log, RV_EXCEP,
                 (ema->log, RV_EXCEP, "emaWasDeleted (%s): Someone is messing with memory 0x%p",
                 raGetName(ema->ra), rElem));
    }
#endif
	}

    /* Check if element was deleted */
    return ((rElem->flags & EMA_ALREADY_DELETED) != 0);
}


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
#ifdef RV_EMA_DEBUG
int emaPrepareForCallbackDebug(IN EMAElement elem, IN const char* filename, IN int lineno)
#else
int emaPrepareForCallback(IN EMAElement elem)
#endif
{
    emaObject*  ema;
    emaElem*    rElem;
    int         numLocks;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Make sure we've got an element */
    if (ema == NULL) return RVERROR;


    /* Increase the reference count */
    EMA_INC_REFERENCE(rElem,1);

    /* See where the do we have to unlock */
    switch (ema->lockType)
    {
        case emaNormalLocks:
        {
            /* We unlock the element */
            int i;
            numLocks = EMA_GET_LOCKS(rElem);

            /* First we decrease the number of locks, and then we actually exit them... */
            EMA_DEC_LOCKS(rElem,numLocks);
            for (i = 0; i < numLocks; i++)
            {
                meiExit((HMEI)((char*)elem + ema->elemSize));
            }
            break;
        }
        case emaLinkedLocks:
        {
            /* We must prepare the parent */
            EMAElement  parent = *((EMAElement*)((char*)elem + ema->elemSize));
            if (parent != NULL)
                numLocks = emaPrepareForCallback(parent);
            else
                numLocks = RVERROR;

            if (numLocks < 0)
            {
                logPrint(ema->log, RV_WARN,
                         (ema->log, RV_WARN, "emaPrepareForCallback (%s): Couldn't prepare parent=0x%p of 0x%p for some reason",
                         raGetName(ema->ra), parent, elem));
            }
            break;
        }
        default:
            numLocks = 0;
            break;
    }

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaPrepareForCallback (%s): on 0x%p - locked %d times (%s:%d)",
             raGetName(ema->ra), rElem, numLocks, filename, lineno));
#endif
    return numLocks;
}


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
#ifdef RV_EMA_DEBUG
int emaReturnFromCallbackDebug(IN EMAElement elem, IN int timesLocked, IN const char* filename, IN int lineno)
#else
int emaReturnFromCallback(IN EMAElement elem, IN int timesLocked)
#endif
{
    emaObject*  ema;
    emaElem*    rElem;
    int         status = RVERROR;
    BOOL        elemExists;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Make sure we've got an element */
    if (ema == NULL) return RVERROR;

#ifdef RV_EMA_DEBUG
    logPrint(ema->log, RV_DEBUG,
             (ema->log, RV_DEBUG, "emaReturnFromCallback (%s): on 0x%p, %d times (%s:%d)",
             raGetName(ema->ra), rElem, timesLocked, filename, lineno));
#endif

    /* See where the do we have to unlock */
    switch (ema->lockType)
    {
        case emaNormalLocks:
        {
            /* We unlock the element */
            int i;

            /* First we should lock, and only then: increase the number of locks */
            for (i = 0; i < timesLocked; i++)
            {
                meiEnter((HMEI)((char*)elem + ema->elemSize));
            }
            EMA_INC_LOCKS(rElem,timesLocked);
            status = 0;
            break;
        }
        case emaLinkedLocks:
        {
            /* We must prepare the parent */
            EMAElement  parent = *((EMAElement*)((char*)elem + ema->elemSize));
            if (parent != NULL)
                status = emaReturnFromCallback(parent, timesLocked);
            else
                status = RVERROR;

            if (status < 0)
            {
                logPrint(ema->log, RV_WARN,
                         (ema->log, RV_WARN, "emaReturnFromCallback (%s): Couldn't return parent=0x%p of 0x%p for some reason",
                         raGetName(ema->ra), parent, elem));
            }
            break;
        }
        default:
            break;
    }

    /* Decrease the reference count */
    EMA_DEC_REFERENCE(rElem,1);
    elemExists = ((rElem->flags & EMA_ALREADY_DELETED) == 0);

    /* Delete element if we're done with it */
    if (rElem->flags == EMA_ALREADY_DELETED)
    {
        int location = raGetByPtr(ema->ra, rElem);
        emaDeleteElement(ema, rElem, location, TRUE, "emaReturnFromCallback");
    }
    else
    {
        /* We didn't delete this element... */
        status = TRUE;
    }

    return status;
}


/************************************************************************
 * emaSetApplicationHandle
 * purpose: Set the application handle of an element in EMA
 * input  : elem        - Element in EMA
 *          appHandle   - Application's handle for the element
 * output : none
 * return : RVERROR on failure
 ************************************************************************/
int emaSetApplicationHandle(IN EMAElement elem, IN void* appHandle)
{
    emaElem* rElem;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    rElem->appHandle = appHandle;

#ifdef RV_EMA_DEBUG
    {
        /* Make sure element is not vacant */
        emaObject* ema;
        int location;

        ema = rElem->ema;

		/* NexTone: added check for NULL ema */
		if (ema != NULL)
		{
        location = raGetByPtr(ema->ra, rElem);

        if (raElemIsVacant(ema->ra, location))
        {
            logPrint(ema->log, RV_EXCEP,
                     (ema->log, RV_EXCEP,
                     "emaSetApplicationHandle (%s): Element %d,0x%p is vacant",
                     raGetName(ema->ra), location, rElem));
        }
		}
    }
#endif

    return 0;
}


/************************************************************************
 * emaGetApplicationHandle
 * purpose: Get the application's handle of an element in EMA
 * input  : elem        - Element in EMA
 * output : appHandle   - Application's handle for the element
 * return : Pointer to the application handle
 *          NULL on failure
 ************************************************************************/
void* emaGetApplicationHandle(IN EMAElement elem)
{
    emaElem* rElem;

    if (elem == NULL) return NULL;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));

#ifdef RV_EMA_DEBUG
    {
        /* Make sure element is not vacant */
        emaObject* ema;
        int location;

        ema = rElem->ema;

		/* NexTone: added check for NULL ema */
		if (ema != NULL)
		{
        location = raGetByPtr(ema->ra, rElem);

        if (raElemIsVacant(ema->ra, location))
        {
            logPrint(ema->log, RV_EXCEP,
                     (ema->log, RV_EXCEP,
                     "emaGetApplicationHandle (%s): Element %d,0x%p is vacant",
                     raGetName(ema->ra), location, rElem));
        }
		}
    }
#endif

    return rElem->appHandle;
}


/************************************************************************
 * emaGetType
 * purpose: Return the type of the element inside the EMA object.
 *          This is the type given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's type on success
 *          0 on failure
 ************************************************************************/
UINT32 emaGetType(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return 0;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));

	/* change from radvision for 2804 nextone */
	if ((rElem->flags & EMA_ALREADY_DELETED) != 0)
	{
/*SH-		printf("Problem in EMA!!!\n"); */
	    syslog(LOG_LOCAL3|LOG_DEBUG, "problem in EMA!!!\n");
		return 0;
	}
	/* end of change */

    ema = rElem->ema;
 
    /* NexTone: added check for NULL ema */
    if (ema == NULL) return 0;

    return ema->type;
}


/************************************************************************
 * emaGetUserData
 * purpose: Return the user related data of the element inside the EMA
 *          object. This is the userData given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's user data pointer on success
 *          NULL on failure
 ************************************************************************/
void* emaGetUserData(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return NULL;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* NexTone: added check for NULL ema */
    if (ema == NULL) return NULL;

    return ema->userData;
}

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
void* emaGetUserDataByInstance(IN HEMA emaH)
{
    emaObject* ema = (emaObject *)emaH;

    if (emaH == NULL) return NULL;

    /* Find out our information */
    return ema->userData;
}

/************************************************************************
 * emaGetInstance
 * purpose: Return the instance of this EMA element.
 *          This is the instance given in emaConstruct().
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's instance on success
 *          NULL on failure
 ************************************************************************/
void const* emaGetInstance(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return NULL;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    if (ema)
        return ema->instance;
    else
        return NULL;
}


/************************************************************************
 * emaGetObject
 * purpose: Return the EMA object this element is in
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's EMA object on success
 *          NULL on failure
 ************************************************************************/
HEMA emaGetObject(IN EMAElement elem)
{
    emaElem* rElem;

    if (elem == NULL) return NULL;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));

    return (HEMA)rElem->ema;
}


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
    IN void*    param)
{
    emaObject* ema = (emaObject *)emaH;
    int        i;

    /* Pass through all the elements, executing the functions on those
       which are used */
    for (i = 0; i < raMaxSize(ema->ra); i++)
    if (!raElemIsVacant(ema->ra, i))
    {
        char *elem  = (char *)raGet(ema->ra, i);

        /* We change the context by the return value of the function */
        param = func(elem + sizeof(emaElem), param);
    }

    return 0;
}


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
    IN EMAElement   cur)
{
    emaObject* ema = (emaObject *)emaH;
    emaElem* rElem;
    int location;

    /* Find out our element information */
    if (cur != NULL)
        rElem = (emaElem *)((char*)cur - sizeof(emaElem));
    else
        rElem = NULL;

    /* Find the location */
    location = raGetNext(ema->ra, raGetByPtr(ema->ra, rElem));

    if (location >= 0)
        return (EMAElement)((char*)raGet(ema->ra, location) + sizeof(emaElem));
    else
        return NULL;
}


/************************************************************************
 * emaGetIndex
 * purpose: Returns the index of the element in the ema
 * input  : elem    - Element in EMA
 * output : none
 * return : The element's index on success
 *          RVERROR on failure
 ************************************************************************/
int emaGetIndex(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return RVERROR;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Find the location */
    return raGetByPtr(ema->ra, rElem);
}


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
int emaGetStatistics(IN HEMA emaH, OUT emaStatistics* stats)
{
    emaObject* ema = (emaObject *)emaH;
    if (ema == NULL) return RVERROR;

    stats->numMarked = ema->markCount;
    return raGetStatistics(ema->ra, &stats->elems);
}

/************************************************************************
 * emaIsVacant
 * purpose: Returns whether the given object is free for allocation
 * input  : elem    - Element in EMA
 * output : none
 * return : TRUE  - if the elemnt is not allocated
 *          FALSE - otherwise
 ************************************************************************/
int emaIsVacant(IN EMAElement elem)
{
    emaObject* ema;
    emaElem* rElem;

    if (elem == NULL) return FALSE;

    /* Find out our element information */
    rElem = (emaElem *)((char*)elem - sizeof(emaElem));
    ema = rElem->ema;

    /* Find the location */
    return (!ema);
}

#ifdef __cplusplus
}
#endif

