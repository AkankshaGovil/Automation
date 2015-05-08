/******************************************************************************
 ** FUNCTION:
 **		Has the hashtable implementation used in the SipStack.
 ******************************************************************************
 **
 ** FILENAME: 		siphash.c
 **
 ** DESCRIPTION:	This file contains the implementation of hash table
 **					used within the SIP Stack.
 **
 ** DATE     	NAME          REFERENCE     REASON
 ** ----      	----          ---------     ------
 ** 12/04/01	K. Binu, 	  siphash.c		Creation
 **				Siddharth
 **
 ******************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 ******************************************************************************/
#include "sipinternal.h"
#include "sipcommon.h"
#include "siphash.h"
#include "portlayer.h"


/******************************************************************************
 ** FUNCTION: 		sip_hashInit
 **
 ** DESCRIPTION: 	This is the function to initialize
 **					a new hash table.
 **
 ******************************************************************************/
SIP_S8bit sip_hashInit
#ifdef ANSI_PROTO
	(SipHash *pHash, sip_hashFunc fpHashFunc,\
	sip_hashKeyCompareFunc fpCompareFunc, \
	sip_hashElementFreeFunc fpElemFreeFunc, \
	sip_hashKeyFreeFunc fpKeyFreeFunc, \
	SIP_U32bit numBuckets, SIP_U32bit maxElements, \
	SipError *pErr)
#else
	(pHash, fpHashFunc, fpCompareFunc, fpElemFreeFunc, \
		fpKeyFreeFunc, numBuckets, maxElements)
	SipHash *pHash;
	sip_hashFunc fpHashFunc; /* function to calculate hash value */
	sip_hashKeyCompareFunc fpCompareFunc; /* function to compare keys of 2 elements */
	sip_hashElementFreeFunc fpElemFreeFunc; /* function to free the data being stored */
	sip_hashKeyFreeFunc fpKeyFreeFunc; /* function to free the key given for an element */
	SIP_U32bit numBuckets;
	SIP_U32bit maxElements;
	SipError *pErr;
#endif
{
	SIP_U32bit i;
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashInit");

	/* Initialize structure variables */
	pHash->fpHashFunc = fpHashFunc;
	pHash->fpCompareFunc = fpCompareFunc;
	pHash->numberOfBuckets = numBuckets;
	pHash->numberOfElements = 0;
	pHash->maxNumberOfElements = maxElements;
	pHash->fpElementFreeFunc = fpElemFreeFunc;
	pHash->fpKeyFreeFunc = fpKeyFreeFunc;

	/* Allocate space for buckets */
	pHash->ppHashChains = (SipHashElement **) \
		fast_memget(0, sizeof(SipHashElement*)*numBuckets,\
			SIP_NULL);
	if (pHash->ppHashChains == NULL)
	{
		*pErr= E_NO_MEM;
		return SipFail;
	}

#ifdef SIP_THREAD_SAFE
	/* Allocate space for the mutexes */
	pHash->pBucketMutex = (synch_id_t *) \
		fast_memget(0, sizeof(synch_id_t)*numBuckets, \
		SIP_NULL);
	if (pHash->pBucketMutex == NULL)
	{
		fast_memfree(0, (SIP_Pvoid )(pHash->ppHashChains), \
			SIP_NULL);
		*pErr = E_NO_MEM;
		return SipFail;
	}
#endif

	/* Initialize the buckets. Also init a mutex for each bucket */
	for(i=0; i<numBuckets; i++)
	{
		pHash->ppHashChains[i] = SIP_NULL;
#ifdef SIP_THREAD_SAFE
		fast_init_synch(&(pHash->pBucketMutex[i]));
#endif
	}

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashInit");
	return SipSuccess;
}


/******************************************************************************
 ** FUNCTION: 		sip_hashFree
 **
 ** DESCRIPTION: 	This is the function to free members from the hash
 **					table. It does not free the hash elements, but frees
 **					other member variables malloced at the time of Init
 **					of the hash table
 **
 **					NOTE :
 **					=====
 **					This function *must not* be invoked when there are
 **					threads working on the hash table.
 **
 ******************************************************************************/
SIP_S8bit sip_hashFree
#ifdef ANSI_PROTO
	(SipHash *pHash, SipError *pErr)
#else
	(pHash, pErr)
	SipHash *pHash;
	SipError pErr;
#endif
{
#ifdef SIP_THREAD_SAFE
	SIP_U32bit i;
#endif

	fast_memfree(0, (SIP_Pvoid )(pHash->ppHashChains), pErr);
#ifdef SIP_THREAD_SAFE
	for(i=0; i<pHash->numberOfBuckets; i++)
	{
		 fast_free_synch(&(pHash->pBucketMutex[i]));
	}
	fast_memfree(0, (SIP_Pvoid )(pHash->pBucketMutex), pErr);
#endif
	return SipSuccess;
}


/******************************************************************************
 ** FUNCTION: 		sip_hashAdd
 **
 ** DESCRIPTION: 	This is the function to add an entry
 **					into the hash table.
 **
 ******************************************************************************/
SIP_S8bit sip_hashAdd
#ifdef ANSI_PROTO
	(SipHash *pHash, SIP_Pvoid pElement, SIP_Pvoid pKey)
#else
	(pHash, pElement, pKey)
	SipHash *pHash;
	SIP_Pvoid pElement;
	SIP_Pvoid pKey;
#endif
{
	SIP_U32bit hashKey;
	SIP_U32bit bucket;
	SipHashElement* pNewElement;
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashAdd");

	if ((pHash == SIP_NULL) || (pKey == SIP_NULL))
		return SipFail;

	/* Check if the max number of Elements is getting exceeded */
	if (pHash->numberOfElements == (pHash->maxNumberOfElements))
		return SipFail;

	/* Compute hash for the element */
	hashKey = pHash->fpHashFunc(pKey);

	/* Locate the bucket */
	bucket = hashKey % pHash->numberOfBuckets;

	/* Allocate and initialize element holder */
	pNewElement = (SipHashElement *)\
		fast_memget(0, sizeof(SipHashElement), SIP_NULL);

	if (pNewElement == SIP_NULL)
		return SipFail;

	pNewElement->pElement = pElement;
	pNewElement->pKey = pKey;
	pNewElement->dRemove = SipFail;
	HSS_INITREF(pNewElement->dRefCount);

#ifdef SIP_THREAD_SAFE
	/* Grab lock for the bucket in which
	   the operation is to be done */
	 fast_lock_synch(0, &(pHash->pBucketMutex[bucket]), 0);
#endif

	/* Push element into the bucket */
	pNewElement->pNext = pHash->ppHashChains[bucket];
	pHash->ppHashChains[bucket] = pNewElement;
	pHash->numberOfElements++;

#ifdef SIP_THREAD_SAFE
	/* Unlock the mutex for the bucket now */
	fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashAdd");
	return SipSuccess;
}


/******************************************************************************
 ** FUNCTION: 		sip_hashFetch
 **
 ** DESCRIPTION: 	This is the function to fetch an entry
 **					from the hash table.
 **
 ******************************************************************************/
SIP_Pvoid sip_hashFetch
#ifdef ANSI_PROTO
	(SipHash *pHash, SIP_Pvoid pKey)
#else
	(pHash, pKey)
	SipHash *pHash;
	SIP_Pvoid pKey;
#endif
{
	SIP_U32bit hashKey;
	SIP_U32bit bucket;
	SipHashElement* pIterator;
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashFetch");

	if ((pKey == SIP_NULL) || (pHash == SIP_NULL))
		return SIP_NULL;

	/* Compute hash for the element */
	hashKey = pHash->fpHashFunc(pKey);

	/* Locate the bucket */
	bucket = hashKey % pHash->numberOfBuckets;

#ifdef SIP_THREAD_SAFE
	/* Grab lock for the bucket in which
	   the operation is to be done */
	 fast_lock_synch(0, &(pHash->pBucketMutex[bucket]), 0);
#endif

	/* Go through chain */
	pIterator = pHash->ppHashChains[bucket];

	while(pIterator != SIP_NULL)
	{
		if(pHash->fpCompareFunc(pIterator->pKey, pKey) == 0)
		{
			/* Increment reference count */
			HSS_LOCKEDINCREF(pIterator->dRefCount);
			break;
		}
		pIterator = pIterator->pNext;
	}

#ifdef SIP_THREAD_SAFE
	/* Unlock the mutex for the bucket now */
	fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashFetch");

	if (pIterator == SIP_NULL)
	 	return SIP_NULL;
	else
		return pIterator->pElement;
}


/******************************************************************************
 ** FUNCTION: 		sip_hashRemove
 **
 ** DESCRIPTION: 	This is the function to remove an entry
 **					from the hash table.
 **
 **					Note:
 **					-----
 **					This function should not be invoked if memory does not
 **					have to be freed at time of invocation.
 **					sip_hashRelease( ) should be called instead.
 **
 ******************************************************************************/
SIP_S8bit sip_hashRemove
#ifdef ANSI_PROTO
	(SipHash *pHash, SIP_Pvoid pKey)
#else
	(pHash, pKey)
	SipHash *pHash;
	SIP_Pvoid pKey;
#endif
{
	SipHashElement *pTempElement;
	SIP_U32bit hashKey;
	SIP_U32bit bucket;
	SipHashElement **ppIterator;

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashRemove");

	if ((pKey == SIP_NULL) || (pHash == SIP_NULL))
		return SipFail;

	/* Compute hash for the element */
	hashKey = pHash->fpHashFunc(pKey);

	/* Locate the bucket */
	bucket = hashKey % pHash->numberOfBuckets;

#ifdef SIP_THREAD_SAFE
	/* Grab lock for the bucket in which
	   the operation is to be done */
	 fast_lock_synch(0, &(pHash->pBucketMutex[bucket]), 0);
#endif

	/* Go through chain */
	ppIterator = &(pHash->ppHashChains[bucket]);

	while(*ppIterator != SIP_NULL)
	{
		if(pHash->fpCompareFunc((*ppIterator)->pKey, pKey) == 0)
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	if(*ppIterator == SIP_NULL)
	{
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif
		return SipFail;
	}

	/* 	check if this hash entry is in use.
		If so just set the remove flag and return */
	HSS_LOCKREF((*ppIterator)->dRefCount);
	HSS_DECREF((*ppIterator)->dRefCount);
	if (HSS_CHECKREF((*ppIterator)->dRefCount))
	{
		pTempElement = *ppIterator;
		*ppIterator = (*ppIterator)->pNext;
		if (pHash->fpElementFreeFunc != NULL)
			pHash->fpElementFreeFunc(pTempElement->pElement);
		if (pHash->fpKeyFreeFunc != NULL)
			pHash->fpKeyFreeFunc(pTempElement->pKey);
		HSS_UNLOCKREF(pTempElement->dRefCount);
		HSS_DELETEREF(pTempElement->dRefCount);
		fast_memfree(0,(SIP_Pvoid )pTempElement,SIP_NULL);
		pHash->numberOfElements--;
	}
	else
	{
		/* 	After decrementing refcount also, it is
			not yet zero. This means the element
			was checked out recently. When its checked
			in the memory will be freed. Set the remove
			flag to true for the moment */
		(*ppIterator)->dRemove = SipSuccess;
		HSS_INCREF((*ppIterator)->dRefCount);
		HSS_UNLOCKREF((*ppIterator)->dRefCount);
	}


#ifdef SIP_THREAD_SAFE
	/* Unlock the mutex for the bucket now */
	fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashRemove");
	return SipSuccess;
}


/******************************************************************************
 ** FUNCTION: 		sip_hashRelease
 **
 ** DESCRIPTION:	This function should be invoked to "check in" an element
 **					that was obtained from the hash table. Normally, it would
 **					just decrement reference count for the element. In case
 **					that the element has its remove flag set, this function
 **					frees the memory too.
 **
 ******************************************************************************/
void sip_hashRelease
#ifdef ANSI_PROTO
(SipHash *pHash, SIP_Pvoid pKey)
#else
(pHash, pKey)
SipHash *pHash;
SIP_Pvoid pKey;
#endif
{
	SIP_U32bit hashKey;
	SIP_U32bit bucket;
	SipHashElement *pTempElement;
	SipHashElement **ppIterator;

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashRelease");

	if ((pKey == SIP_NULL) || (pHash == SIP_NULL))
		return;

	/* Compute hash for the element */
	hashKey = pHash->fpHashFunc(pKey);

	/* Locate the bucket */
	bucket = hashKey % pHash->numberOfBuckets;

#ifdef SIP_THREAD_SAFE
	/* Grab lock for the bucket in which
	   the operation is to be done */
	 fast_lock_synch(0, &(pHash->pBucketMutex[bucket]), 0);
#endif

	/* Go through chain */
	ppIterator = &(pHash->ppHashChains[bucket]);
	while(*ppIterator != SIP_NULL)
	{
		if(pHash->fpCompareFunc((*ppIterator)->pKey, pKey) == 0)
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	if(*ppIterator == SIP_NULL)
	{
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif
		return;
	}

	/* decrement refcount for the hash element */
	HSS_LOCKREF((*ppIterator)->dRefCount);
	HSS_DECREF((*ppIterator)->dRefCount);
	HSS_DECREF((*ppIterator)->dRefCount);

	/* 	check if the remove flag has been set
		before the "check in" of the element. Also
		free the memory if the refCount has already
		become zero*/
	if (((*ppIterator)->dRemove == SipSuccess)&& \
		(HSS_CHECKREF((*ppIterator)->dRefCount)))
	{
		pTempElement = *ppIterator;
		*ppIterator = (*ppIterator)->pNext;
		if (pHash->fpElementFreeFunc != NULL)
			pHash->fpElementFreeFunc(pTempElement->pElement);
		if (pHash->fpKeyFreeFunc != NULL)
			pHash->fpKeyFreeFunc(pTempElement->pKey);
		HSS_UNLOCKREF(pTempElement->dRefCount);
		HSS_DELETEREF(pTempElement->dRefCount);
		fast_memfree(0,(SIP_Pvoid )pTempElement,SIP_NULL);
		pHash->numberOfElements--;
	}
	else
	{
		HSS_INCREF((*ppIterator)->dRefCount);
		HSS_UNLOCKREF((*ppIterator)->dRefCount);
	}
#ifdef SIP_THREAD_SAFE
	/* Unlock the mutex for the bucket now */
	fast_unlock_synch(0, &(pHash->pBucketMutex[bucket]));
#endif

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashRelease");
}


/******************************************************************************
 ** FUNCTION: 		sip_hashForeach
 **
 ** DESCRIPTION: 	This is the function to iterate through all the elements
 **					in the hash table. Passed function is invoked for each key
 **					and element in the list.
 **
 **					Note:
 **					----
 **					* Function must return 1 if iteration must continue to
 **					  the next element.
 **					* Returning 0 stops iterations.
 **
 ******************************************************************************/
void sip_hashForeach
#ifdef ANSI_PROTO
	(SipHash *pHash, sip_hashIteratorFunc fpIteratorFunc)
#else
	(pHash, fpIteratorFunc)
	SipHash *pHash;
	sip_hashIteratorFunc fpIteratorFunc;
#endif
{
	SIP_U32bit index;
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashForeach");

	if (pHash == SIP_NULL)
		return;

	/* Iterate through all buckets */
	for(index = 0; index < pHash->numberOfBuckets; index++)
	{
		SipHashElement *pIterator;
#ifdef SIP_THREAD_SAFE
		/* 	Grab lock for the bucket in which
	  		the operation is to be done */
		 fast_lock_synch(0, &(pHash->pBucketMutex[index]), 0);
#endif

		/* Iterate through elements in the bucket */
		pIterator=pHash->ppHashChains[index];
		while(pIterator != SIP_NULL)
		{
			if(fpIteratorFunc(pIterator->pKey, \
				pIterator->pElement) == 0)
			{
				SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting "
					"sip_hashForeach");
#ifdef SIP_THREAD_SAFE
				fast_unlock_synch(0, &(pHash->pBucketMutex[index]));
#endif
				return;
			}
			pIterator = pIterator->pNext;
		}

#ifdef SIP_THREAD_SAFE
		/* Unlock the mutex for the bucket now */
		fast_unlock_synch(0, &(pHash->pBucketMutex[index]));
#endif
	}
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashForeach");
}


/******************************************************************************
 ** FUNCTION: 		sip_hashNext
 **
 ** DESCRIPTION: 	This is the function to get the element
 **					next to an iterator in the hash table
 **
 ******************************************************************************/
void sip_hashNext
#ifdef ANSI_PROTO
(SipHash *pHash, SipHashIterator *pIterator)
#else
(pHash, pIterator)
SipHash *pHash;
SipHashIterator *pIterator;
#endif
{
	SipHashElement *nextElem, *pTempElement, **ppElement;
	SIP_Pvoid pKey;

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_hashNext");

	if ((pHash == SIP_NULL) || (pIterator == SIP_NULL))
		return;

#ifdef SIP_THREAD_SAFE
	/* Grab mutex for the bucket in which the iterator is currently */
	 fast_lock_synch(0, &(pHash->\
		pBucketMutex[pIterator->currentBucket]), 0);
#endif

	/* 	If current element in the iterator points to a
		null node - keep it null else make it point to
		next element in the chain. */
	nextElem = (pIterator->pCurrentElement == SIP_NULL)\
		? SIP_NULL : pIterator->pCurrentElement->pNext;

	/*
	 * Check if the current element has to be removed.
	 * If so, do it here before fetching the next element
	 */
	if (pIterator->pCurrentElement != SIP_NULL)
	{
		HSS_LOCKREF(pIterator->pCurrentElement->dRefCount);
		HSS_DECREF(pIterator->pCurrentElement->dRefCount);
		HSS_DECREF(pIterator->pCurrentElement->dRefCount);
		if (HSS_CHECKREF(pIterator->pCurrentElement->dRefCount) && \
			(pIterator->pCurrentElement->dRemove == SipSuccess))
		{
			pKey = pIterator->pCurrentElement->pKey;
			ppElement = &(pHash->ppHashChains[pIterator->currentBucket]);

			while(*ppElement != SIP_NULL)
			{
				if(pHash->fpCompareFunc((*ppElement)->pKey, pKey) == 0)
					break;
				ppElement = &((*ppElement)->pNext);
			}
			if(*ppElement != SIP_NULL)
			{
				pTempElement = *ppElement;
				*ppElement = (*ppElement)->pNext;
				if (pHash->fpElementFreeFunc != NULL)
					pHash->fpElementFreeFunc(pTempElement->pElement);
				if (pHash->fpKeyFreeFunc != NULL)
					pHash->fpKeyFreeFunc(pTempElement->pKey);
				HSS_UNLOCKREF(pTempElement->dRefCount);
				HSS_DELETEREF(pTempElement->dRefCount);
				fast_memfree(0,(SIP_Pvoid )pTempElement, \
					SIP_NULL);
				pHash->numberOfElements--;
			}
		}
		else
		{
			HSS_INCREF(pIterator->pCurrentElement->dRefCount);
			HSS_UNLOCKREF(pIterator->pCurrentElement->dRefCount);
		}
	}

	if (nextElem != SIP_NULL)
	{
		/* 	Still in the middle of iteration through a chain.
			We have already taken the next element.
			Return now */
		pIterator->pCurrentElement = nextElem;
		HSS_LOCKEDINCREF(pIterator->pCurrentElement->dRefCount);
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]));
#endif
		return ;
	}

	/*	Since the next element is Null, we have reached the end
		of the chain. Check if its the end of the last chain.
		If so, return */
	if((nextElem == SIP_NULL) &&\
		(pIterator->currentBucket == pHash->numberOfBuckets-1))
	{
		pIterator->pCurrentElement = nextElem;
#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]));
#endif
		return;
	}

#ifdef SIP_THREAD_SAFE
	/* Unlock the current bucket now */
	fast_unlock_synch(0, &(pHash->\
		pBucketMutex[pIterator->currentBucket]));
#endif

	/* 	Find the next non-empty chain and
		make the iterator point to that */
	pIterator->currentBucket++;
	while (pIterator->currentBucket != pHash->numberOfBuckets-1)
	{
#ifdef SIP_THREAD_SAFE
		 fast_lock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]), 0);
#endif

		if (pHash->ppHashChains[pIterator->currentBucket]!=SIP_NULL)
			break;

#ifdef SIP_THREAD_SAFE
		fast_unlock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]));
#endif

		pIterator->currentBucket++;
	}

	/* If its the final bucket grab the lock now */
	if (pIterator->currentBucket == pHash->numberOfBuckets-1)
	{
#ifdef SIP_THREAD_SAFE
		 fast_lock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]), 0);
#endif
	}

	pIterator->pCurrentElement = \
		pHash->ppHashChains[pIterator->currentBucket];

	/* 	Increment reference count of the element being
		returned unless we reached the end of the final
		bucket */
	if (pIterator->pCurrentElement != SIP_NULL)
		HSS_LOCKEDINCREF(pIterator->pCurrentElement->dRefCount);

#ifdef SIP_THREAD_SAFE
	/* Free the lock that was grabbed above */
	fast_unlock_synch(0, &(pHash->\
		pBucketMutex[pIterator->currentBucket]));
#endif
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_hashNext");
}


/******************************************************************************
 ** FUNCTION: 		sip_hashInitIterator
 **
 ** DESCRIPTION:	Sets the iterator to the first element of the hashtable
 **
 ******************************************************************************/
void sip_hashInitIterator
#ifdef ANSI_PROTO
(SipHash *pHash, SipHashIterator *pIterator)
#else
(pHash, pIterator)
SipHash *pHash;
SipHashIterator *pIterator;
#endif
{
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering "
		"sip_hashInitIterator");

	if ((pHash == SIP_NULL) || (pIterator == SIP_NULL))
		return;

	pIterator->currentBucket = 0;
	while(pIterator->currentBucket <= pHash->numberOfBuckets-1)
	{
#ifdef SIP_THREAD_SAFE
		/* Grab mutex for the bucket in which the iterator is currently */
		fast_lock_synch(0, &(pHash->\
			pBucketMutex[pIterator->currentBucket]),0);
#endif
		if(pHash->ppHashChains[pIterator->currentBucket] == SIP_NULL)
		{
#ifdef SIP_THREAD_SAFE
			/* Unlock the current bucket now */
			fast_unlock_synch(0, &(pHash->\
				pBucketMutex[pIterator->currentBucket]));
#endif
			pIterator->currentBucket++;
			continue;
		}
		else
		{
			pIterator->pCurrentElement = pHash->ppHashChains\
				[pIterator->currentBucket];
			HSS_LOCKEDINCREF(pIterator->pCurrentElement->dRefCount);
#ifdef SIP_THREAD_SAFE
			/* Unlock the current bucket now */
			fast_unlock_synch(0, &(pHash->\
				pBucketMutex[pIterator->currentBucket]));
#endif
			return;
		}
	}
	pIterator->currentBucket--;
	pIterator->pCurrentElement = SIP_NULL;

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting "
		"sip_hashInitIterator");
}


/******************************************************************************
 ** FUNCTION: 		sip_elfHash
 **
 ** DESCRIPTION: 	ELF hash function
 ** 				Algorithm from Dr. Dobb's Journal
 **
 ** http://www.ddj.com/articles/1996/9604/9604b/9604b.htm?topic=algorithms
 **
 ******************************************************************************/
SIP_U32bit sip_elfHash
#ifdef ANSI_PROTO
(SIP_Pvoid pName)
#else
(pName)
SIP_Pvoid pName;
#endif
{
	SIP_U32bit h = 0, g;
	SIP_S8bit *name;
	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Entering sip_elfHash");
	if (pName == NULL)
		return (SIP_U32bit)0;

	name = (SIP_S8bit *)pName;
	while ( *name )
	{
		h = ( h << 4 ) + *name++;
		if ( (g = h & 0xF0000000) )
			h ^= g >> 24;
		h &= ~g;
	}

	SIPDEBUG((SIP_S8bit*)"SIP_DEBUG - Exiting sip_elfHash");
	return h;
}
