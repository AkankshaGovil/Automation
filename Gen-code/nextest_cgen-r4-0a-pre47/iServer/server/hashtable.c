#include <pthread.h>
#include <stdio.h>
#include "hashtable.h"
#include "srvrlog.h"
#include <malloc.h>

/**
 * given the root block (i.e., first block - block number 0) and the desired block number,
 * return that block
 *
 * @param root the first block of the table
 * @param blockNumber the desired block number
 */
static ListEntry**
GetBlock (ListEntry **root, int blockNumber)
{
  int i;
  ListEntry **ptr;

  for (i = 0, ptr = root; i < blockNumber; ptr = (ListEntry **)ptr[0], i++);
  return ptr;
}


/**
 * given the bucket number, return the pointer to that bucket from the correct block
 *
 * @param root the first block of the table
 * @param bucketNumber the desired bucket number (0 to numBuckets-1)
 */
static ListEntry*
GetBucket (ListEntry **root, int bucketNumber)
{
  ListEntry *ptr;
  ListEntry **block;

  block = GetBlock(root, bucketNumber/(HASHTABLE_BLOCK_SIZE-1));
  bucketNumber %= (HASHTABLE_BLOCK_SIZE-1);
  bucketNumber++;
  return block[bucketNumber];  
}


/**
 * given the number of buckets, return the number of blocks of size HASHTABLE_BLOCK_SIZE
 * is needed to accomodate those buckets
 *
 * @param numBuckets the desired number of buckets
 */
static int
GetNumBlocks (int numBuckets)
{
  int i, j, numBlocks, leftover;

  numBlocks = leftover = 0;
  i = numBuckets;
  while (i != 0)
  {
    j = i/HASHTABLE_BLOCK_SIZE;
    leftover += (i%HASHTABLE_BLOCK_SIZE);
    numBlocks += j;
    i = j;

    if (i == 0 && leftover > HASHTABLE_BLOCK_SIZE)
    {
      i = leftover;
      leftover = 0;
    }
  }
  if (leftover > 0)
    numBlocks++;

  return numBlocks;
}

/**
 * create a hashtable structure with the given number of buckets
 *
 * @param numBuckets the number of buckets in the hashtable
 * @param offset the offset to use in the Listg macros
 * @param allocFn the pointer to the memory allocation function
 * @param freeFn the pointer to the memory freeing function
 */
void*
HashCreate (int numBuckets, int offset, void *(*allocFn)(size_t), void (*freeFn)(void*))
{
  ListEntry **ptr = NULL, **tmp, **prev;
  int i, j, numBlocks, count;

  /* calculate the number of blocks to be allocated */
  numBlocks = GetNumBlocks(numBuckets);
  NETDEBUG(MCACHE, NETLOG_DEBUG4, ("HashCreate: Using %d blocks for %d buckets\n", numBlocks, numBuckets));

  /* allocate the buckets in the table */
  for (i = count = 0; i < numBlocks; i++)
  {
    tmp = (ListEntry **)(*allocFn)(HASHTABLE_BLOCK_SIZE*sizeof(ListEntry *));
    if (tmp == NULL)
    {
      NETERROR(MCACHE, ("HashCreate: Cannot allocate block #%d\n", i));
      break;
    }

    tmp[0] = NULL; // the first element points to NULL in the beginning
    /* put a dummy ListEntry element in each of the other buckets */
    for (j = 1; j < HASHTABLE_BLOCK_SIZE; j++, count++)
    {
      /* only put the dummy element, if we are going to use it */
      if (count < numBuckets)
      {
	tmp[j] = (ListEntry *)(*allocFn)(offset + sizeof(ListEntry));
	if (tmp[j] == NULL)
	{
	  NETERROR(MCACHE, ("HashCreate: Cannot allocate element %d of block %d\n", j, i));
	  break;
	}
	ListgInitElem(tmp[j], offset);
      }
      else
      {
	tmp[j] = (ListEntry *)0xdeadbeef;
      }
    }
    if (j != HASHTABLE_BLOCK_SIZE)
    {
      // some malloc failed
      for (j--; j >= 1; (*freeFn)(tmp[j--]));
      (*freeFn)(tmp);
      break;
    }

    if (i == 0)
      ptr = tmp;
    else
    {
      prev = GetBlock(ptr, i-1);
      prev[0] = (ListEntry *)tmp;  // the first element points to the next block
    }
  }
  if (i != numBlocks)
  {
    NETERROR(MCACHE, ("HashCreate: malloc failed, free'ing previously allocated blocks\n"));
    // some malloc failed
    for (i--; i >= 0; i--)
    {
      tmp = GetBlock(ptr, i);
      for (j = HASHTABLE_BLOCK_SIZE-1; j >= 1; (*freeFn)(tmp[j--]));
      (*freeFn)(tmp);
    }
    ptr = NULL;
  }

  NETDEBUG(MCACHE, NETLOG_DEBUG4, ("HashCreate: returning hashtable %p\n", ptr));
  return (void *)ptr;
}


/**
 * release the resources held by the given hashtable
 *
 * @param table the hashtable
 * @param numBuckets the number of buckets in the hashtable
 * @param freeFn the pointer to the memory freeing function
 */
void
HashDestroy (void *table, int numBuckets, void (*freeFn)(void*))
{
  int i, count;
  ListEntry **tmp, **ptr = (ListEntry **)table;

  count = 0;
  do
  {
    tmp = ptr;
    ptr = (ListEntry **)ptr[0];

    for (i = 1; i < HASHTABLE_BLOCK_SIZE && count < numBuckets; count++, (*freeFn)(tmp[i++]));
    (*freeFn)(tmp);
  } while ((ptr != NULL) && (count < numBuckets));
}


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
void*
HashFind (void *table, void *key, int (*hash)(void*), void* (*data2key)(void*), int (*keycmp)(void*, void*), int offset)
{
  ListEntry *entryList, *ptr;

  entryList = GetBucket((ListEntry **)table, (*hash)(key));
  if (ListgIsSingle(entryList, offset))
    return NULL;

  ptr = Listg(entryList, offset)->next;
  do
  {
    int result = (*keycmp)((*data2key)(ptr), key);
    if (result == 0)
      return ptr;
    else if (result > 0)
      break;  // nothing found
    ptr = Listg(ptr, offset)->next;
  } while (ptr != entryList);

  return NULL;  // end of list
}


/**
 * get the first available entry in the hashtable
 *
 * @param table the hashtable
 * @param numBuckets the number of buckets in the table
 * @param offset the offset to use in the Listg macros
 */
void*
HashGetFirst (void *table, int numBuckets, int offset)
{
  int i, count;
  ListEntry **tmp, **ptr = (ListEntry **)table;

  count = 0;
  do
  {
    tmp = ptr;
    ptr = (ListEntry **)ptr[0];

    for (i = 1; i < HASHTABLE_BLOCK_SIZE && count < numBuckets; count++, i++)
    {
      if (!ListgIsSingle(tmp[i], offset))  // skip the dummy element
        return Listg(tmp[i], offset)->next;  // return the first element after the dummy
    }
  } while ((ptr != NULL) && (count < numBuckets));

  return NULL;
}


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
void
HashInsert (void *table, void *data, int (*hash)(void*), void* (*data2key)(void*), int (*keycmp)(void*, void*), int offset)
{
  ListEntry *entryList, *ptr, *prev;
  void *key;

  // get the key of the entry
  key = (*data2key)(data);

  ListgInitElem(data, offset);

  entryList = GetBucket((ListEntry **)table, (*hash)(key));
  ptr = Listg(entryList, offset)->next;
  prev = entryList;
  if (!ListgIsSingle(entryList, offset))
  {
    do
    {
      if ((*keycmp)((*data2key)(ptr), key) > 0)
	break;
      prev = ptr;
      ptr = Listg(ptr, offset)->next;
    } while (ptr != entryList);
  }

  ListgInsert(prev, data, offset);
}


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
void*
HashDelete (void *table, void *key, int (*hash)(void*), void* (*data2key)(void*), int (*keycmp)(void*, void*), int offset)
{
  ListEntry *first, *last, *ptr, *entryList;

  entryList = GetBucket((ListEntry **)table, (*hash)(key));
  first = last = NULL;

  if (!ListgIsSingle(entryList, offset))
  {
    ptr = Listg(entryList, offset)->next;
    do
    {
      int result = (*keycmp)((*data2key)(ptr), key);
      if (result == 0)
      {
	if (first == NULL)  // found the first match
	  first = last = ptr;
	else
	  last = ptr;  // next match
      }
      else if (result > 0)
	break;  // no more matches
      ptr = Listg(ptr, offset)->next;
    } while (ptr != entryList);

    if (first != NULL)
    {
      /* unlink the nodes between 'first' and 'last' */
      Listg((Listg(first, offset)->prev), offset)->next = Listg(last, offset)->next;
      Listg((Listg(last, offset)->next), offset)->prev = Listg(first, offset)->prev;

      /* rechain the unlinked nodes */
      Listg(first, offset)->prev = last;
      Listg(last, offset)->next = first;
    }
  }

  return first;
}


/**
 * count the number of items on this list (the list is usually the one returned by HashDelete)
 *
 * @param list the linked list
 * @param offset the offset to use in the Listg macros
 */
int
HashCountList (void *list, int offset)
{
  int count = 0;
  ListEntry *ptr;

  if (list != NULL)
  {
    for (count = 1, ptr = Listg(list, offset)->next; ptr != list; ptr = Listg(ptr, offset)->next, count++);
  }

  return count;
}

