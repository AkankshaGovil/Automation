/**
 * This file contains structs and prototypes for a generic hashtable. The elements
 * in the hashtable are assumed to follow a linked list as in clist.h. There could
 * be duplicate elements with the same hash code. They will be stored in the same bucket
 * as a linked list.
 *
 * During initialization, each bucket in the hashtable contains a ListEntry type element.
 * These dummy elements remain as the head of the linked list in each bucket of the hashtable
 * until the hashtable is destroyed. They are free'd when the HashDestory is called.
 *
 * To create a cache using this hashtable, follow these steps:
 *   cache_t cache = CacheCreate();
 *   cache->dt = CACHE_DT_HASH;
 *   cache->_numBuckets = 1024;  // the desired buckets
 *   cache->_hashlistOffset = 0;  // the offset to use in Listg macros
 *   cache->_hash = myHashFunction;
 *   cache->_entry2key = myEntry2KeyConvertFunction;
 *   cache->_keycmp = myKeyComparisonFunction;
 *   cache->lock = LockInit(&myLock, 0, 0);  // optional lock
 *   cache->name = myName;  // optional cache name
 *   if (CacheInstantiate(cache))
 *     goto _error;
 *
 * myHashFunction - given the key, do a hash and return an integer in the range of 0 to
 *                  numBuckets
 * myEntry2KeyConvertFunction - given the entry, return a pointer to the key of that entry
 * myKeyComparisonFunction - given key1 and key2, return < 0 if key1 < key2, return 0 if
 *                           key1 = key2, and return > 0 if key1 > key2
 *
 * Use these Cache API, when using hashtable:
 *   CacheGet, CacheFind, CacheOverwrite, CacheInsert, CachePut, CacheProbeInsert, 
 *   CacheRemove, CacheDelete
 *
 * When done with the hashtable:
 *   CacheDestroy(cache);
 */
#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#include "clist.h"
#include "lock.h"

/* the following fields from the cache structure will be used for the hashtable
typedef struct cache_t
{
  int dt;  // type of the cache, i.e., CACHE_DT_HASH

  void *root;  // the root pointer to the hashtable

  int nodeLineWidth;	// the offset to use with Listg operations (clist.h)
  int	xitems;  // the bucket size
  int	nitems;  // number of items currently in the table in the table
	
  int	cachedatains2Key;  // given the entry, return the key of that entry void* (*)(void *entry)
  int	cachedata2Key;	   // given the key, return the hash int (*)(void *key)
  int cachecmp;  // compare the given two keys int (*)(void *key1, void *key2)

  // following are generic cache related fields
  Lock	*lock;
  int	malloc;
  int free;
  char 	*name;
  int	pre_cond;
  int post_cond;
}
*/

/* the block size of the hash table, if a size greater than this is requested, internally
   multiple blocks of this size will be allocated */
#define HASHTABLE_BLOCK_SIZE      4090


/**
 * create a hashtable structure with the given number of buckets
 *
 * @param numBuckets the number of buckets in the hashtable
 * @param offset the offset to use in the Listg macros
 * @param allocFn the pointer to the memory allocation function
 * @param freeFn the pointer to the memory freeing function
 */
extern void* HashCreate (int numBuckets,
			 int offset,
			 void *(*allocFn)(size_t),
			 void (*freeFn)(void*));

/**
 * release the resources held by the given hashtable
 *
 * @param table the hashtable
 * @param numBuckets the number of buckets in the hashtable
 * @param freeFn the pointer to the memory freeing function
 */
extern void HashDestroy (void *table,
			 int numBuckets,
			 void (*freeFn)(void*));

/**
 * find the entry for the given key (find the first entry in the table with that key)
 *
 * @param table the hashtable
 * @param key pointer to the key to search for
 * @param hash pointer to the hash function
 * @param data2key pointer to the function that returns the key for the given data
 * @param keycmp pointer to the function that compares given keys
 * @param offset the offset to use in the Listg macros
 */
extern void* HashFind (void *table,
		       void *key,
		       int (*hash)(void*),
		       void* (*data2key)(void*),
		       int (*keycmp)(void*, void*),
		       int offset);


/**
 * get the first available entry in the hashtable
 *
 * @param table the hashtable
 * @param numBuckets the number of buckets in the table
 * @param offset the offset to use in the Listg macros
 */
extern void* HashGetFirst (void *table,
                           int numBuckets,
                           int offset);


/**
 * insert the given entry into the table 
 *
 * @param table the hashtable
 * @param data the entry to insert
 * @param hash pointer to the hash function
 * @param data2key pointer to the function that returns the key for the given data
 * @param keycmp pointer to the function that compares given keys
 * @param offset the offset to use in the Listg macros
 */
extern void HashInsert (void *table,
			void *data,
			int (*hash)(void*),
			void* (*data2key)(void*),
			int (*keycmp)(void*, void*),
			int offset);

/**
 * delete the given key from the table and returns the deleted key
 * (for duplicate keys, unlinks the chain and returns the first element of the chain)
 *
 * @param table the hashtable
 * @param key the key to delete
 * @param hash pointer to the hash function
 * @param data2key pointer to the function that returns the key for the given data
 * @param keycmp pointer to the function that compares given keys
 * @param offset the offset to use in the Listg macros
 */
extern void* HashDelete (void *table,
			 void *key,
			 int (*hash)(void*),
			 void* (*data2key)(void*),
			 int (*keycmp)(void*, void*),
			 int offset);

/**
 * count the number of items on this list (the list is usually the one returned by HashDelete)
 *
 * @param list the linked list
 * @param offset the offset to use in the Listg macros
 */
extern int HashCountList (void *list, int offset);

#endif /* __HASHTABLE_H */
