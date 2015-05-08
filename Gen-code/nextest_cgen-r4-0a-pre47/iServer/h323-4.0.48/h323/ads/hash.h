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

#ifndef _HASH_H
#define _HASH_H

#ifdef __cplusplus
extern "C" {
#endif



#include <rvinternal.h>
#include <log.h>

/* Handle to a HASH object */
DECLARE_OPAQUE(HHASH);


/************************************************************************
 * Hash function type
 * input  : param       - Pointer to the element to hash
 *          paramSize   - Size of the element to hash in bytes
 *          hashSize    - Size of the hash table used for the hash
 *                        function's return value
 * return : Hash result
 ************************************************************************/
typedef UINT32 (*hashFunc) (IN void *param, int paramSize, int hashSize);


/************************************************************************
 * Comparison function for elements
 * input  : key1, key2  - Keys to compare
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
typedef BOOL (*hashCompareFunc) (IN void *key1, IN void* key2, IN UINT32 keySize);


/************************************************************************
 * HASHEFunc
 * purpose: Definition of a general function on an RA element
 * input  : hHash   - Hash object
 *          elem    - Element to function on
 *          param   - Context to use for it
 * output : none
 * return : Pointer for the context to use on the next call to this
 *          HASHEFunc.
 ************************************************************************/
typedef void*(*HASHEFunc)(IN HHASH hHash, IN void* elem, void *param);



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
    IN int      hashSize);


/************************************************************************
 * Default comparison function. Checks byte-by-byte.
 * input  : key1, key2  - Keys to compare
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
BOOL hashDefaultCompare(IN void *key1, IN void* key2, IN UINT32 keySize);



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
    IN  const char*     name);


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
    IN  HHASH hHash);


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
     IN  BOOL   searchBeforeInsert);


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
    IN  void* key);


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
    IN  void* location);


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
    IN  void* location);


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
    IN  void*   userElem);


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
    IN  void* location);


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
    IN  void*   location);


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
    IN void*        param);


#ifdef __cplusplus
}
#endif

#endif  /* _HASH_H */

