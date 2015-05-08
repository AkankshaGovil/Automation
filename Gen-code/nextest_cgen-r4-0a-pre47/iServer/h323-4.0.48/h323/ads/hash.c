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
  ohash.c

  Irina S.
  20 Jan. 1998

  Opened hash table implementation


  Notes:
  Hash function is provided by the user.
  Hash function parameters may be duplicated in the hash table.
  Each table element holds list id. New collided parameter is
  positioned in the list corresponding to parameter key.
  Duplicated parameters following each other in the corresponding list.
  The hash size is the first prime number above the supplied hashSize.

  */


#include <string.h>
#include <ra.h>
#include <hash.h>
#include <intutils.h>


/************************************************************************
 * hashStruct structure
 * Structure holding a HASH object
 * hash             - Hash function used
 * compare          - Comparison function used
 * numKeys          - Size of the hash table (number of keys supported)
 * userKeySize      - Size of the keys inside the hash table
 * userElemSize     - Size of the elements inside the hash table
 * alignedKeySize   - Size of the keys inside the hash table, aligned to 32bits
 * numElements      - Number of elements that can be stored inside the hash
 * curSize          - Current number of occupied elements
 * keys             - List of keys (the hash table)
 *                    This array holds pointers to the elements
 * elements         - Table of stored elements (RA)
 *                    See hashListElement to see the value itself
 ************************************************************************/
typedef struct
{
    hashFunc        hash;
    hashCompareFunc compare;
    UINT32          numKeys;
    UINT32          userKeySize;
    UINT32          userElemSize;
    UINT32          alignedKeySize;
    UINT32          numElements;
    UINT32          curSize;
    RAElement*      keys;
    HRA             elements;
} hashStruct;


/************************************************************************
 * hashListElement struct
 * This structure will be included in any user element and serve for
 * pointing to the next and prev elements in the list of elements that
 * were mapped to the same entry (collision/key)
 * next     - Pointer to the next user element with the same key value
 * prev     - Pointer to the previous user element with the same key value
 * entryNum - The entry location of the key value in the hash table
 *
 * Note: Inside the elements RA, the information stored in each element
 *       is the list element, the key and the data in their written order.
 ************************************************************************/
typedef struct hashListElement_tag hashListElement;
struct hashListElement_tag
{
    hashListElement*    next;
    hashListElement*    prev;
    int                 entryNum;
};




/************************************************************************
 * Default hashing function.
 * input  : param       - Pointer to the element to hash
 *          paramSize   - Size of the element to hash in bytes
 *                        When this value <=0, then 'param' is considered
 *                        to be NULL terminated.
 *          hashSize    - Size of the hash table used for the hash
 *                        function's return value
 * return : Hash result
 ************************************************************************/
UINT32 hashstr(
    IN void*    param,
    IN int      paramSize,
    IN int      hashSize)
{
    UINT32 hash=0;
    char* ptr = (char *)param;

    if (paramSize <= 0)
    {
        /* string is null terminated */
        while (*ptr) hash = hash << 1 ^ *ptr++;
    }
    else
    {
        while (paramSize-- > 0 && *ptr) hash = hash << 1 ^ *ptr++;
    }

    return (hash % hashSize);
}


/************************************************************************
 * Default comparison function. Checks byte-by-byte.
 * input  : key1, key2  - Keys to compare
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
BOOL hashDefaultCompare(IN void *key1, IN void* key2, IN UINT32 keySize)
{
    return (memcmp(key1, key2, keySize) == 0);
}



/************************************************************************
 * hashConstruct
 * purpose: Create a HASH object, holding a hash table of keys and the
 *          actual data in an array
 * input  : numOfKeys       - Size of hash table.
 *                            This is the amount of keys available for use
 *                            It should be greater than the number of
 *                            elements in the table
 *          numOfElems      - Number of elements that will be stored
 *                            in the hash table
 *          hashFunc        - Hash function used on the data
 *          compareFunc     - Comparison function used on the keys
 *          keySize         - Size of the keys
 *          elemSize        - Size of the elements
 *          logMgr          - Log manager to use
 *          name            - Name of HASH (used in log messages)
 * output : none
 * return : Handle to RA constructed on success
 *          NULL on failure
 ************************************************************************/
HHASH
hashConstruct(
    IN  int             numOfKeys,
    IN  int             numOfElems,
    IN  hashFunc        hashFunc,
    IN  hashCompareFunc compareFunc,
    IN  int             keySize,
    IN  int             elemSize,
    IN  RVHLOGMGR       logMgr,
    IN  const char*     name)
{
    hashStruct* hash;
    UINT32      actualKeySize;

    /* Calculate the actual size of the hash table = number of keys */
    actualKeySize = intFirstPrime(numOfKeys);

    /* Allocate the HASH object and the hash table itself
     * We make sure to leave it empty to the hash table will be filled with NULL
     * pointers.
     */
    hash = (hashStruct *)calloc((int)(sizeof(hashStruct) + actualKeySize * sizeof(RAElement)), 1);

    /* Set the HASH object struct */
    hash->hash = hashFunc;
    hash->compare = compareFunc;
    hash->numKeys = actualKeySize;
    hash->userKeySize = keySize;
    hash->userElemSize = elemSize;
    hash->alignedKeySize = RV_ALIGN(keySize);
    hash->numElements = numOfElems;
    hash->curSize = 0;
    hash->keys = (RAElement*)((BYTE*)hash + sizeof(hashStruct));

    /* Construct the elements RA */
    hash->elements =
        raConstruct((int)(elemSize + hash->alignedKeySize + sizeof(hashListElement)), numOfElems, logMgr, name);
    if (hash->elements == NULL)
    {
        free(hash);
        return NULL;
    }

    return (HHASH)hash;
}


/************************************************************************
 * hashDestruct
 * purpose: Delete a HASH object, freeing all of the taken memory
 * input  : hHash   - HASH object handle
 * output : none
 * return : Non negative value on succes
 *          Negative value on failure
 ************************************************************************/
int
hashDestruct(
    IN  HHASH hHash)
{
    hashStruct* hash = (hashStruct *)hHash;

    if (!hash) return RVERROR;
    raDestruct(hash->elements);
    free(hash);

    return 0;
}


/************************************************************************
 * hashAdd
 * purpose: Add a new element to the hash table.
 *          This function will not add the element if an element with the
 *          same key already exists if asked
 * input  : hHash               - HASH object handle
 *          key                 - Pointer to the key
 *          userElem            - Pointer to the element to store
 *          searchBeforeInsert  - Check for the same key inside the HASH or not
 * output : none
 * return : Pointer to the element's location in the hash table on success
 *          NULL on failure
 ************************************************************************/
void*
hashAdd(
     IN  HHASH  hHash,
     IN  void*  key,
     IN  void*  userElem,
     IN  BOOL   searchBeforeInsert)
{
    hashStruct* hash = (hashStruct *)hHash;
    hashListElement* newElem;
    UINT32 keyValue;

    /* See if such an element exists */
    if (searchBeforeInsert)
    {
        newElem = (hashListElement*)hashFind(hHash, key);
        if (newElem != NULL) return newElem;
    }

    /* Try to allocate a new element in the hash */
    if (raAddExt(hash->elements, (RAElement*)&newElem) < 0)
    {
        /* Not found... */
        return NULL;
    }

    /* Calculate the key's hash value */
    keyValue = hash->hash(key, (int)(hash->userKeySize), (int)(hash->numKeys)) % hash->numKeys;

    /* Update new element's information (this is the first in the list) */
    newElem->prev = NULL;
    newElem->entryNum = keyValue;
    newElem->next = (hashListElement *)hash->keys[keyValue];

    /* Fix the next one in list if there is one */
    if (newElem->next != NULL) newElem->next->prev = newElem;

    /* Fill in with the element */
    memcpy((char*)newElem + sizeof(hashListElement), key, hash->userKeySize);
    memcpy((char*)newElem + sizeof(hashListElement) + hash->alignedKeySize, userElem, hash->userElemSize);
    hash->keys[keyValue] = (RAElement)newElem;

    return newElem;
}


/************************************************************************
 * hashFind
 * purpose: Find the location of an element by its key
 * input  : hHash       - HASH object handle
 *          key         - Pointer to the key
 * output : none
 * return : Pointer to the element's location in the hash table on success
 *          NULL on failure or if the element wasn't found
 ************************************************************************/
void*
hashFind(
    IN  HHASH hHash,
    IN  void* key)
{
    hashStruct* hash = (hashStruct *)hHash;
    UINT32 keyValue;
    hashListElement* hashElem;

    if (hHash == NULL) return NULL;

    /* Calculate the key's hash value */
    keyValue = hash->hash(key, (int)(hash->userKeySize), (int)(hash->numKeys)) % hash->numKeys;

    /* See if there are any elements in this key value at all */
    if (hash->keys[keyValue] == NULL)
        return NULL;

    /* Start looking for the element */
    hashElem = (hashListElement *)hash->keys[keyValue];

    /* Search for this element */
    while (hashElem != NULL)
    {
        if (hash->compare(key, (char*)hashElem + sizeof(hashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }

    /* No such element was found */
    return NULL;
}


/************************************************************************
 * hashFindNext
 * purpose: Find the location of the next element with the same key
 * input  : hHash       - HASH object handle
 *          key         - Pointer to the key
 *          location    - Location given in the last call to hashFindNext()
 *                        or hashFind().
 * output : none
 * return : Pointer to the element's location in the hash table on success
 *          NULL on failure or if the element wasn't found
 ************************************************************************/
void*
hashFindNext(
    IN  HHASH hHash,
    IN  void* key,
    IN  void* location)
{
    hashStruct* hash = (hashStruct *)hHash;
    hashListElement* hashElem;

    if (location == NULL) return NULL;

    /* Start looking for the element */
    hashElem = (hashListElement *)location;

    /* First we skip the one we already know is ok */
    hashElem = hashElem->next;

    /* Search for this element */
    while (hashElem != NULL)
    {
        if (hash->compare(key, (char*)hashElem + sizeof(hashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }

    /* No such element was found */
    return NULL;
}


/************************************************************************
 * hashGetElement
 * purpose: Get the element's data by its location (given by hashFind()).
 * input  : hHash       - HASH object handle
 *          location    - Pointer to the element in hash
 *                        (given by hashAdd)
 * output : none
 * return : Pointer to the element's date in the hash table on success
 *          NULL on failure or if the element wasn't found
 ************************************************************************/
void*
hashGetElement(
    IN  HHASH hHash,
    IN  void* location)
{
    hashStruct* hash = (hashStruct *)hHash;

    if (location == NULL) return NULL;

    return ((char*)location + sizeof(hashListElement) + hash->alignedKeySize);
}


/************************************************************************
 * hashSetElement
 * purpose: Set the element's data by its location (given by hashFind()).
 * input  : hHash       - HASH object handle
 *          location    - Pointer to the element in hash
 *                        (given by hashAdd)
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int
hashSetElement(
    IN  HHASH   hHash,
    IN  void*   location,
    IN  void*   userElem)
{
    hashStruct* hash = (hashStruct *)hHash;

    if (hash == NULL) return RVERROR;

    memcpy((char*)location + sizeof(hashListElement) + hash->alignedKeySize, userElem, hash->userElemSize);
    return 0;
}


/************************************************************************
 * hashGetKey
 * purpose: Get the element's key by its location (given by hashFind()).
 * input  : hHash       - HASH object handle
 *          location    - Pointer to the element in hash
 *                        (given by hashAdd)
 * output : none
 * return : Pointer to the element's key in the hash table on success
 *          NULL on failure or if the element wasn't found
 ************************************************************************/
void*
hashGetKey(
    IN  HHASH hHash,
    IN  void* location)
{
    if (location == NULL) return NULL;
    if (hHash);
    return ((char*)location + sizeof(hashListElement));
}


/************************************************************************
 * hashDelete
 * purpose: Delete an element from the HASH
 * input  : hHash       - HASH object handle
 *          location    - Pointer to the element in hash
 *                        (given by hashAdd)
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int
hashDelete(
    IN  HHASH   hHash,
    IN  void*   location)
{
    hashStruct* hash = (hashStruct *)hHash;
    hashListElement* userElem;

    if (hash == NULL) return RVERROR;
    userElem = (hashListElement *)location;

    /* Remove linkages to this element, making sure to update the key if
       we have to */
    if (userElem->prev == NULL)
    {
        /* First element - update keys table */
        hash->keys[userElem->entryNum] = userElem->next;
    }
    else
        userElem->prev->next = userElem->next;

    if (userElem->next != NULL)
        userElem->next->prev = userElem->prev;

    /* Remove this element from RA */
    return raDeleteLocation(hash->elements, raGetByPtr(hash->elements, userElem));
}


/************************************************************************
 * hashDoAll
 * purpose: Call a function on all used elements stored in HASH
 * input  : hHash       - HASH object handle
 *          func        - Function to execute on all elements
 *          param       - Context to use when executing the function
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int hashDoAll(
    IN HHASH        hHash,
    IN HASHEFunc    func,
    IN void*        param)
{
    hashStruct* hash = (hashStruct *)hHash;
    int cur;
    if (hash == NULL) return RVERROR;

    cur = -1;
    while ((cur = raGetNext(hash->elements, cur)) >= 0)
    {
        param = func(hHash, raGet(hash->elements, cur), param);
    }
    return 0;
}


#ifdef __cplusplus
}
#endif



