/******************************************************************************
 ** FUNCTION:
 **		Has the hashtable implementation used in the Sip Stack.
 ******************************************************************************
 **
 ** FILENAME:		siphash.h
 **
 ** DESCRIPTION:  	This file contains the implementation of hashtable used
 **					within the Sip Stack.
 **
 ** DATE     	NAME          REFERENCE     REASON
 ** ----      	----          ---------     ------
 ** 12/04/01	K. Binu, 	  siphash.h		Creation
 **				Siddharth
 **
 ******************************************************************************
 ** 			Copyrights 2001, Hughes Software Systems, Ltd.
 ******************************************************************************/

#ifndef __SIP_STACK_HASH_H__
#define __SIP_STACK_HASH_H__

#include "sipcommon.h"
#include "sipstruct.h"
#include "portlayer.h"

/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * The function to be used to calculate the hash value for the key passed.
 */
typedef SIP_U32bit (*sip_hashFunc) (void *pData);

/*
 * The function to be used to compare keys of 2 hash elements.
 */
typedef SIP_U8bit (*sip_hashKeyCompareFunc) (void *pKey1,\
					void *pKey2);

/*
 * The function to be given when wanting to iterate through all the
 * elements of the hash table.
 */
typedef int (*sip_hashIteratorFunc) (void *pKey, void *pElement);

/*
 * The function to free the data stored in the hash element.
 */
typedef void (*sip_hashElementFreeFunc) (void *pElement);

/*
 * The function to free the key stored in the hash element.
 */
typedef void (*sip_hashKeyFreeFunc) (void *pKey);


/*
 * Hash element - The hash table contains chains of this
 * 				  structure.
 */
typedef struct SipHashElement
{
	/*
	 * Data to be stored in the hash table.
	 */
	void*				pElement;

	/*
	 * Key to be used to retrieve this data later.
	 */
	void*				pKey;

	/*
	 * The next element in the hash chain. Will be NULL is the current
	 * element is the lat element of the chain.
	 */
	struct SipHashElement*	pNext;

	/*
	 * Flag that will be set to true if the API sip_hashRemove is
	 * unable to free the memory for the hash element at time of
	 * invocation. As soon as the structures refCount reduces to zero, the
	 * memory will be freed if this flag is set.
	 */
	SipBool			dRemove;

	/*
	 * Keep track of how many check-outs of this data have happened. If the
	 * refCount is greater than 1 and sip_hashRemove is invoked,
	 * memory will not be freed and the dRemove flag will be set.
	 */
	SIP_RefCount		dRefCount;
} SipHashElement;

/*
 * Iterator needed to operate on all the hash elements. User will need to
 * do sip_hashNext() to get the next element from the hash table.
 */
typedef struct
{
	SipHashElement 		*pCurrentElement;
	SIP_U32bit			currentBucket;
} SipHashIterator;

/*
 * The actual hash table data structure.
 */
typedef struct
{
	/* function to calculate hash value */
	sip_hashFunc				fpHashFunc;

	/* function to compare keys of 2 elements */
	sip_hashKeyCompareFunc	fpCompareFunc;

	/* function to free the data being stored */
	sip_hashElementFreeFunc	fpElementFreeFunc;

	/* function to free the key given for an element */
	sip_hashKeyFreeFunc		fpKeyFreeFunc;

	/* Number of buckets and number of elements in the hash */
	SIP_U32bit 				numberOfBuckets;
	SIP_U32bit				numberOfElements;
	SIP_U32bit				maxNumberOfElements;

	/* Resizable array that holds the hash lists */
	SipHashElement			**ppHashChains;

#ifdef SIP_THREAD_SAFE
	/* Mutex for each bucket of the hash table */
	synch_id_t				*pBucketMutex;
#endif
} SipHash;


/******************************************************************************
 ** FUNCTION: 		sip_hashInit
 **
 ** DESCRIPTION: 	This is the function to initialize a new
 **  				hash table.
 **
 ** PARAMETERS:
 **	 pHash 			(IN/OUT)	: Hash table to be initialized.
 **  fpHashFunc 	(IN)		: Function to be used to hash the key.
 **  fpCompareFunc 	(IN)		: Function to compare the hash keys of entries
 **								  at time of doing a fetch. If the comparison
 **								  function :
 **								  returns 0 - the keys that were compared match
 **								  returns 1 - the keys don't match
 **  fpElemFreeFunc (IN)		: Function to invoke to free the
 **								  element data when the hash entry
 **								  has be deleted.
 **  fpKeyFreeFunc	(IN)		: Function to invoke to free the
 **								  element key when the hash entry
 **								  has be deleted.
 **  numBuckets		(IN)		: number of chains in the hash table.
 **  maxElements	(IN)		: maximum number of elements to be allowed
 **								  in the hash table.
 **	 pErr			(IN/OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
SIP_S8bit sip_hashInit _ARGS_((SipHash *pHash, \
		sip_hashFunc fpHashFunc,\
		sip_hashKeyCompareFunc fpCompareFunc, \
		sip_hashElementFreeFunc fpElemFreeFunc, \
		sip_hashKeyFreeFunc fpKeyFreeFunc, \
		SIP_U32bit numBuckets, SIP_U32bit maxElements, \
		SipError *pErr));


/******************************************************************************
 ** FUNCTION: 		sip_hashFree
 **
 ** DESCRIPTION: 	This is the function to free members from the hash table.
 **					It does not free the hash elements, but frees other	member
 **					variables malloced at the time of Init of the hash table
 **
 ** PARAMETERS:
 **	 pHash 	(IN)	: Hash table to be freed.
 **  pErr	(OUT)	: Error variable returned in case of failure.
 **
 ******************************************************************************/
SIP_S8bit sip_hashFree _ARGS_((SipHash *pHash, \
					SipError *pErr));


/******************************************************************************
 ** FUNCTION: 		sip_hashAdd
 **
 ** DESCRIPTION: 	This is the function to add an entry into the hash table.
 **
 ** PARAMETERS:
 **	 pHash 	(IN)	: Hash table to which the entry has
 **					  to be added.
 **  pErr	(OUT)	: Error variable returned in case
 **					  of failure.
 **
 ******************************************************************************/
SIP_S8bit sip_hashAdd _ARGS_((SipHash *pHash, \
		void *pElement, void* pKey));


/******************************************************************************
 ** FUNCTION: 		sip_hashFetch
 **
 ** DESCRIPTION: 	This is the function to fetch an entry from the hash table.
 **					null is returned in case the hash table does not
 **					contain any entries corresponding to the key passed.
 **
 ** PARAMETERS:
 **	 pHash 	(IN)	: Hash table from which the entry has to be
 **					  extracted.
 **  pKey	(IN)	: Key corresponding to the element to be
 **					  fetched from the hash table.
 **
 ******************************************************************************/
void* sip_hashFetch _ARGS_((SipHash *pHash, \
								void *pKey));


/******************************************************************************
 ** FUNCTION: 		sip_hashRelease
 **
 ** DESCRIPTION:	This function should be invoked to
 **					"check in" an element that was obtained
 **					from the hash table. Normally, it would
 **					just decrement reference count for the
 **					element. In case that the element has its
 **					remove flag set, this function frees
 **					the memory too.
 **
 ** PARAMETERS:
 **	 pHash 	(IN)	: Hash table from which the entry has to
 **					  be released.
 **  pKey	(IN)	: Key corresponding to the element to be
 **					  released.
 **
 ******************************************************************************/
void sip_hashRelease _ARGS_((SipHash *pHash, \
								void *pKey));


/******************************************************************************
 ** FUNCTION: sip_hashRemove
 **
 ** DESCRIPTION: 	This is the function to remove an entry from the hash
 **					table. If the element is in use at the time of the remove
 **					request, then it is marked for removal and memory actually
 **					gets freed only when the other usage releases the entry.
 **
 ** PARAMETERS:
 **	 pHash 	(IN)	: Hash table from which the entry has to be released.
 **  pKey	(IN)	: Key corresponding to the element to be removed.
 **
 ******************************************************************************/
SIP_S8bit sip_hashRemove _ARGS_((SipHash *pHash, void *pKey));


/******************************************************************************
 ** FUNCTION: sip_hashForeach
 **
 ** DESCRIPTION: 	This is the function to iterate through
 **					all the elements in the hash table.
 **					Passed function is invoked for each key
 **					and element in the list.
 **
 **					Note:
 **					----
 **					* Function must return 1 if iteration
 **					  must continue to the next element.
 **					* Returning 0 stops iterations.
 **
 ** PARAMETERS:
 **	 pHash 			(IN)	: Hash table for which all the entries
 **							  have to be traversed.
 **  fpIteratorFunc	(IN)	: Iterator function to be invoked for each entry.
 **
 ******************************************************************************/
void sip_hashForeach _ARGS_((SipHash *pHash, \
				sip_hashIteratorFunc fpIteratorFunc));


/******************************************************************************
 ** FUNCTION: 		sip_hashNext
 **
 ** DESCRIPTION: 	This is the function to get the element next to an
 **					iterator in the hash table
 **
 ** PARAMETERS:
 **	 pHash 		(IN)		: Hash table from which the next entry
 **							  has to be retrieved.
 **  pIterator	(IN/OUT)	: Hash iterator for which the next
 **							  element has to be retrieved.
 **
 ******************************************************************************/
void sip_hashNext _ARGS_((SipHash *pHash, \
				SipHashIterator *pIterator));


/******************************************************************************
 ** FUNCTION: sip_hashInitIterator
 **
 ** DESCRIPTION:	Sets the iterator to the first element of the hashtable
 **
 ** PARAMETERS:
 **	 pHash 		(IN)		: Hash table from which the next entry
 **							  has to be retrieved.
 **  pIterator	(IN/OUT)	: Hash iterator to be inited.
 **
 ******************************************************************************/
void sip_hashInitIterator _ARGS_((SipHash *pHash, \
					SipHashIterator *pIterator));


/* ELF hash function */
SIP_U32bit sip_elfHash _ARGS_((void* name));


/* Ensure Names are not mangled by C++ compilers */
#ifdef __cplusplus
}
#endif

#endif
