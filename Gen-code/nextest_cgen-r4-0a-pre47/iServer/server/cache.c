#include "cache.h"
#include "mem.h"
#include "srvrlog.h"
#include "stdlib.h"
#include <malloc.h>
#include <stdio.h>

/* strings for operation types */
char 
cache_operations_strings[CACHEOP_MAX][50] = {
	"CACHEOP_INSERT",
	"CACHEOP_DELETE",
	"CACHEOP_GET",
	"CACHEOP_GETFIRST",
	"CACHEOP_GETNEXT",
	"CACHEOP_LOCK",
	"CACHEOP_UNLOCK",
};

void *(*CacheMalloc[2])(size_t) = { SHM_Malloc, malloc };
void (*CacheFree[2])(void *) = { SHM_Free, free };

void
node_free_shm(void *data, void *param)
{
	SHM_Free(data);
}

void 
node_free(void *data, void *param)
{
	free(data);
}

void (*avl_node_freefn[2])(void *, void *) = 
		{ node_free_shm, node_free };


// TAVL requires functions to  create data and memory. 
//  We already do our own memory mangagement
// so we just create 2 do nothing functions.
void *
make_item(const void *data){
      return (void *)data;
}
void *
copy_item(void *dest,void *src){
	//do nothing
        return src;
}
// Standard Cache functions.
cache_t 
CacheCreate(int memfntype)
{
	cache_t cache;

	/* cache = _malloc(sizeof(struct cache_t)); */
	cache = CacheMalloc[memfntype](sizeof(struct cache_t));

	if (cache != NULL)
	{
		memset(cache, 0, sizeof(struct cache_t));
		cache->dt = CACHE_DT_AVLT;
		cache->root = NULL;
		cache->nitems = 0;
		cache->malloc = memfntype;
		cache->free = memfntype;
		cache->name = CMalloc(cache)(CACHE_NAME_LEN);
		memset(cache->name, 0, CACHE_NAME_LEN);
	}

	return cache;
}

int
CacheInstantiate(cache_t cache)
{
	if (cache == NULL)
	{
		return -1;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		cache->root = avlt_create(cache->cachecmp, cache->cacheinscmp, cache);
		break;
	case CACHE_DT_AVLTR:
		cache->root = avltr_create(cache->cachecmp, cache->cacheinscmp, cache);
		break;
	case CACHE_DT_AVL:
		cache->root = avl_create(cache->cachecmp, cache->cacheinscmp, cache);
		break;
	case CACHE_DT_RB:
		cache->root = rb_create(cache->cachecmp, cache->cacheinscmp, cache);
		break;
	case CACHE_DT_TST:
		cache->root = tst_init(cache->nodeLineWidth);
		break;
	case CACHE_DT_HASH:
		cache->root = HashCreate(cache->_numBuckets, cache->_hashlistOffset, CMalloc(cache), CFree(cache));
		break;
	case CACHE_DT_CLIST:
		cache->root = NULL;
		// This is enough for clist
		return 1;
	case CACHE_DT_TAVL:
		cache->root = tavl_init(CachecmpArray[cache->cachecmp], CacheinscmpArray[cache->cacheinscmp], make_item,CacheFree[cache->free],copy_item,CacheMalloc[cache->malloc],CacheFree[cache->free],&cache);
		break;
	}

	return (cache->root != NULL);
}

void
CacheSetName(cache_t cache, char *name)
{
	if (cache == NULL)
	{
		return;
	}

	strncpy(cache->name, name, CACHE_NAME_LEN);
	cache->name[CACHE_NAME_LEN-1] = '\0';
}

int 
CacheDestroyKeys(cache_t cache)
{
	if (cache == NULL)
	{
		return -1;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		avlt_destroy(cache->root, NULL);
		break;
	case CACHE_DT_AVLTR:
		avltr_destroy(cache->root, NULL);
		break;
	case CACHE_DT_AVL:
		avl_destroy(cache->root, NULL);
		break;
	case CACHE_DT_RB:
		rb_destroy(cache->root, NULL);
		break;
	case CACHE_DT_TST:
		tst_cleanup(cache->root);
		break;
	case CACHE_DT_HASH:
		break;
	case CACHE_DT_CLIST:
		list_reset((ListEntry **)&cache->root, cache->listoffset);
		break;
	case CACHE_DT_TAVL:
		tavl_delete_all(cache->root);
		break;
	}

	// no keys means nothing in the cache
	cache->nitems = 0;
	
	return 0;
}

int 
CacheDestroyData(cache_t cache)
{
	if (cache == NULL)
	{
		return -1;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		avlt_destroy(cache->root, avl_node_freefn[cache->free]);
		break;
	case CACHE_DT_AVLTR:
		avltr_destroy(cache->root, avl_node_freefn[cache->free]);
		break;
	case CACHE_DT_AVL:
		avl_destroy(cache->root, avl_node_freefn[cache->free]);
		break;
	case CACHE_DT_RB:
		rb_destroy(cache->root, avl_node_freefn[cache->free]);
		break;
	case CACHE_DT_TST:
		break;
	case CACHE_DT_HASH:
		HashDestroy(cache->root, cache->_numBuckets, CFree(cache));
		break;
	case CACHE_DT_CLIST:
		break;
	case CACHE_DT_TAVL:
		tavl_delete_all(cache->root);
		break;
	}

	// since the data is not destroyed
	cache->nitems = 0;
	
	return 0;
}

int 
CacheDestroy(cache_t cache)
{
	if (cache == NULL)
	{
		return -1;
	}

	CacheDestroyData(cache);

	CFree(cache)(cache->name);
	CFree(cache)(cache);

	return 0;
}

int
CacheGetLocks(cache_t cache, int mode, int block)
{
	char fn[] = "CacheGetLocks():";
	int error, result;
	pthread_t selfid;

	if (cache == NULL)
	{
		return -1;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_LOCK, NULL, 0)))
	{
		return -1;
	}
	
	result = LockGetLock(cache->lock, mode, block);

_return:
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_LOCK, NULL, 0);
	}
	
	return result;
}

int
CacheReleaseLocks(cache_t cache)
{
	char fn[] = "CacheReleaseLocks():";
	int error, count;

	if (cache == NULL)
	{
		return -1;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_UNLOCK, NULL, 0)))
	{
		return -1;
	}
	
	error = LockReleaseLock(cache->lock);

_return:
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_UNLOCK, NULL, 0);
	}
	
	return error;
}

int
CacheFind(cache_t cache, void *data, void *copy, size_t size)
{
	void *entry = NULL;

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	entry = CacheGet(cache, data);

	if (entry && copy)
	{
		/* Copy this entry */
		memcpy(copy, entry, size);
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if (entry)
	{
		return 1;
	}
	
	return -1;
}

/* For all get functions, locks muct be acquired beforehand */
void *
CacheGet(cache_t cache, void *data)
{
	void *item = NULL, **itemp = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GET, data, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		itemp = avlt_find(cache->root, data);
		if (itemp) item = *itemp;
		break;
	case CACHE_DT_AVLTR:
		itemp = avltr_find(cache->root, data);
		if (itemp) item = *itemp;
		break;
	case CACHE_DT_AVL:
		item = avl_find(cache->root, data);
		break;
	case CACHE_DT_RB:
		item = rb_find(cache->root, data);
		break;
	case CACHE_DT_TST:
		if (((char *)data)[0] == '\0')
		{
			item = tst_search("%NULL%", cache->root);
		}
		else
		{
			item = tst_search(data, cache->root);
		}
		break;
	case CACHE_DT_HASH:
		item = HashFind(cache->root, data, (int (*)(void*))cache->_hash, (void* (*)(void*))cache->_entry2key, (int (*)(void*, void*))cache->_keycmp, cache->_hashlistOffset);
		break;
	case CACHE_DT_CLIST:
		item = list_get(cache->root, cache->listoffset, data, cache->cachecmp);
		break;
	case CACHE_DT_TAVL:
		item = ((TAVL_nodeptr)tavl_find(cache->root, data));
		if(item != NULL) item =((TAVL_nodeptr) item)->dataptr;	
		break;
	}

	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GET, item, 0);
	}
	
	return item;
}

/* For all get functions, locks muct be acquired beforehand */
void **
CacheGetFast(cache_t cache, void *data)
{
	void **item = NULL;
	void *itemp;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GET, data, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_find(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_find(cache->root, data);
		break;
	case CACHE_DT_AVL:
		item = NULL;
		break;
	case CACHE_DT_RB:
		item = NULL;
		break;
	case CACHE_DT_HASH:
		item = NULL;
		break;
	case CACHE_DT_CLIST:
		item = NULL;
		break;
	case CACHE_DT_TAVL:
		itemp = (TAVL_nodeptr)tavl_find(cache->root, data);
		if(itemp) item = (void **)&(((TAVL_nodeptr)(itemp))->dataptr);
		break;
	}

	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GET, item, 0);
	}
	
	return item;
}

int
CacheFindFirst(cache_t cache, void *copy, size_t size)
{
	void *entry = NULL;

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	entry = CacheGetFirst(cache);

	if (entry)
	{
		/* Copy this entry */
		memcpy(copy, entry, size);
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if (entry)
	{
		return 1;
	}
	
	return -1;
}

void *
CacheGetFirst(cache_t cache)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETFIRST, NULL, 0)))
	{
		return NULL;
	}
	
	// reset the iterator
	memset(&cache->iterator, 0, sizeof(cache->iterator));

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_traverse(cache->root, &cache->iterator.avlt);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_traverse(cache->root, &cache->iterator.avltr);
		break;
	case CACHE_DT_AVL:
		item = avl_traverse(cache->root, &cache->iterator.avl);
		break;
	case CACHE_DT_RB:
		item = rb_traverse(cache->root, &cache->iterator.rb);
		break;
	case CACHE_DT_HASH:
		item = HashGetFirst(cache->root, cache->_numBuckets, cache->_hashlistOffset);
		break;
	case CACHE_DT_CLIST:
		item = cache->root;
		break;
	case CACHE_DT_TAVL:
		cache->iterator.tavl_iterator_node = tavl_succ(tavl_reset(cache->root));
	        item = (TAVL_nodeptr)(cache->iterator.tavl_iterator_node)->dataptr;	
		break;
	}
	
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETFIRST, item, 0);
	}
	
	return item;
}

int
CacheFindNext(cache_t cache, void *data, void *copy, size_t size)
{
	void *entry = NULL;

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	entry = CacheGetNext(cache, data);

	if (entry)
	{
		/* Copy this entry */
		memcpy(copy, entry, size);
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if (entry)
	{
		return 1;
	}
	
	return -1;
}

void *
CacheGetNext(cache_t cache, void *data)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETNEXT, data, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_traverse(cache->root, &cache->iterator.avlt);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_traverse(cache->root, &cache->iterator.avltr);
		break;
	case CACHE_DT_AVL:
		item = avl_traverse(cache->root, &cache->iterator.avl);
		break;
	case CACHE_DT_RB:
		item = rb_traverse(cache->root, &cache->iterator.rb);
		break;
	case CACHE_DT_HASH:
		item = NULL;
		break;
	case CACHE_DT_CLIST:
		if ((item = Listg(data, cache->listoffset)->next) == cache->root)
		{
			item = NULL;
		}

		break;
	case CACHE_DT_TAVL:
		cache->iterator.tavl_iterator_node = tavl_succ(cache->iterator.tavl_iterator_node);
	        if((cache->iterator.tavl_iterator_node) !=NULL)
	        	item = (TAVL_nodeptr)(cache->iterator.tavl_iterator_node)->dataptr;	
		else item =NULL;	
		break;
	}

	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETNEXT, item, 0);
	}
	
	return item;
}

void *
CacheGetPrev(cache_t cache, void *data)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		break;
	case CACHE_DT_AVLTR:
		break;
	case CACHE_DT_AVL:
		break;
	case CACHE_DT_RB:
		break;
	case CACHE_DT_HASH:
		break;
	case CACHE_DT_CLIST:
		if (data != cache->root)
		{
			item = Listg(data, cache->listoffset)->prev;
		}

		break;
	case CACHE_DT_TAVL:
		break;
	}

	return item;
}

void *
CacheGetLast(cache_t cache)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	// reset the iterator
	memset(&cache->iterator, 0, sizeof(cache->iterator));

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		break;
	case CACHE_DT_AVLTR:
		break;
	case CACHE_DT_AVL:
		break;
	case CACHE_DT_RB:
		break;
	case CACHE_DT_HASH:
		break;
	case CACHE_DT_CLIST:
		if (cache->root)
		{	
			item = Listg(cache->root, cache->listoffset)->prev;
		}

		break;
	case CACHE_DT_TAVL:
		break;
	}
	
	return item;
}

// does not use iterator
void **
CacheGetNextFast(cache_t cache, void **data)
{
	void **item = NULL;
	TAVL_nodeptr ptr;
	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETNEXT, data, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_next(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_next(cache->root, data);
		break;
	case CACHE_DT_AVL:
		// dont allow usage, as will lead to bugs
		// item = avl_traverse(cache->root, &cache->iterator.avl);
		item = NULL;
		break;
	case CACHE_DT_RB:
		// item = rb_traverse(cache->root, &cache->iterator.rb);
		item = NULL;
		break;
	case CACHE_DT_HASH:
		item = NULL;
		break;
	case CACHE_DT_CLIST:
		item = NULL;
		break;
 	case CACHE_DT_TAVL:
 		ptr = (TAVL_nodeptr)((char *)data - offsetof(TAVL_NODE,dataptr));
 		ptr = tavl_succ(ptr);
 		if(ptr !=NULL) {
 			item = (void **) &(ptr->dataptr);
  		}
 		else item = NULL;
 		break;
	}

	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETNEXT, item, 0);
	}
	
	return item;
}

// does not use iterator
void **
CacheGetPrevFast(cache_t cache, void **data)
{
	void **item = NULL;
	TAVL_nodeptr ptr;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETNEXT, data, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_prev(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
	//	item = avltr_next(cache->root, data);
		item = NULL;
		break;
	case CACHE_DT_AVL:
		// dont allow usage, as will lead to bugs
		// item = avl_traverse(cache->root, &cache->iterator.avl);
		item = NULL;
		break;
	case CACHE_DT_RB:
		// item = rb_traverse(cache->root, &cache->iterator.rb);
		item = NULL;
		break;
	case CACHE_DT_HASH:
		item = NULL;
		break;
	case CACHE_DT_CLIST:
		item = NULL;
		break;
 	case CACHE_DT_TAVL:
 		ptr = (TAVL_nodeptr)((char *)data - offsetof(TAVL_NODE,dataptr));
 		ptr = tavl_pred(ptr);
 		if(ptr !=NULL) {
 			item = (void **) &(ptr->dataptr);
  		}
 		else item = NULL;
 		break;
	}

	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETNEXT, item, 0);
	}
	
	return item;
}

void *
CacheGetMin(cache_t cache)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETFIRST, NULL, 0)))
	{
		return NULL;
	}
	
	if (cache->flags & CACHE_MAINT_MIN)
	{
		// optimized O(1) lkup
		if ((item = cache->minData) == NULL)
		{
			item = cache->minData = CacheGetFirst(cache);
		}
	}
	else
	{
		item = CacheGetFirst(cache);
	}
	
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETFIRST, item, 0);
	}
	
	return item;
}

int
CachePut(cache_t cache, void *copy, size_t size)
{
	void *entry = NULL;

	if (cache == NULL)
	{
		return -1;
	}

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	entry = CMalloc(cache)(size);

	if (entry)
	{
		/* Copy this entry */
		memcpy(entry, copy, size);
	}

	if (CacheInsert(cache, entry) < 0)
	{
		/* Free the allocated unit */
		CFree(cache)(entry);
		entry = NULL;
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if (entry)
	{
		return 1;
	}
	
	return -1;
}

int
CacheOverwrite(cache_t cache, void *copy, size_t size, void *key)
{
	void *entry = NULL;

	if (cache == NULL)
	{
		return -1;
	}

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	entry = CacheGet(cache, key);

	if (entry)
	{
		/* Copy this entry */
		memcpy(entry, copy, size);
		// min cant change here, as min is position based
		// and position did not change here
	}
	else if ((entry = CMalloc(cache)(size)))
	{
		/* Copy this entry */
		memcpy(entry, copy, size);
		CacheInsert(cache, entry);
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if (entry)
	{
		return 1;
	}
	
	return -1;
}

/* return -1 if failure */
int 
CacheInsert(cache_t cache, void *data)
{
	int result = -1;
	void *item = NULL;

	if (cache == NULL)
	{
		return -1;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_INSERT, data, 0)))
	{
		return -1;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_insert(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_insert(cache->root, data);
		break;
	case CACHE_DT_AVL:
		item = avl_insert(cache->root, data);
		break;
	case CACHE_DT_RB:
		item = rb_insert(cache->root, data);
		break;
	case CACHE_DT_TST:
		if (tst_insert(CacheInsData2KeyArray[cache->cachedatains2Key](data), data, cache->root, 0, &item) != TST_OK)
		{
			item = (void *)0xdeadbeef;
		}
		break;
	case CACHE_DT_HASH:
		HashInsert(cache->root, data, (int (*)(void*))cache->_hash, (void* (*)(void*))cache->_entry2key, (int (*)(void*, void*))cache->_keycmp, cache->_hashlistOffset);
		break;
	case CACHE_DT_CLIST:
		item = list_insert((ListEntry **)&cache->root, cache->listoffset, data, cache->cacheinscmp);
		break;
	case CACHE_DT_TAVL:
		item = tavl_insert(cache->root,data,0);
		break;
	}

	if (item == 0)
	{
		result = 1;
		cache->nitems ++;
		if (cache->flags & CACHE_MAINT_MIN)
		{
			if (cache->minData == NULL)
			{
				if (cache->nitems==1)
					cache->minData = data;
			}
			else if (CacheinscmpArray[cache->cacheinscmp] (data, 
						cache->minData, NULL) < 0)
			{
				cache->minData = data;
			}
		}
	}
	else
	{
		result = -1;
	}

_return:
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_INSERT, &result, 0);
	}
	
	return result;
}

int 
CacheProbeInsert(cache_t cache, void *data, void **existData)
{
	void *item = NULL;
	int result = -1;

	*existData = NULL;

	if (cache == NULL)
	{
		return -1;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_INSERT, data, 0)))
	{
		return -1;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_insert(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_insert(cache->root, data);
		break;
	case CACHE_DT_AVL:
		item = avl_insert(cache->root, data);
		break;
	case CACHE_DT_RB:
		item = rb_insert(cache->root, data);
		break;
	case CACHE_DT_TST:
		result = tst_insert(CacheInsData2KeyArray[cache->cachedatains2Key](data), data, cache->root, 0, &item);
		if ((result != TST_OK) && (result != TST_DUPLICATE_KEY))
		{
			result = -1;
			item = NULL;

			goto _return;
		}
		break;
	case CACHE_DT_HASH:
		HashInsert(cache->root, data, (int (*)(void*))cache->_hash, (void* (*)(void*))cache->_entry2key, (int (*)(void*, void*))cache->_keycmp, cache->_hashlistOffset);
		break;
	case CACHE_DT_CLIST:
		item = list_insert((ListEntry **)&cache->root, cache->listoffset, data, cache->cacheinscmp);
		break;
	case CACHE_DT_TAVL:
		item = tavl_insert(cache->root,data,0);
		break;
	}

	if (item == 0)
	{
		result = 1;
		cache->nitems ++;
		if (cache->flags & CACHE_MAINT_MIN)
		{
			if (cache->minData == NULL)
			{
				if (cache->nitems==1)
					cache->minData = data;
			}
			else if (CacheinscmpArray[cache->cacheinscmp] (data, 
						cache->minData, NULL) < 0)
			{
				cache->minData = data;
			}
		}
	}
	else
	{
		result = -1;
		*existData = item;
	}

_return:
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_INSERT, &result, 0);
	}
	
	return result;
}

/* cache entry is freed */
void *
CacheRemove(cache_t cache, void *entry)
{
	void *tmp = NULL;

	/* Lock the cache */
	CacheGetLocks(cache, LOCK_READ, LOCK_BLOCK);

	if (entry)
	{
		tmp = CacheDelete(cache, entry);
	}

	/* Release the cache */
	CacheReleaseLocks(cache);
	
	if(tmp)
	{
		CFree(cache)(tmp);
	}
	else {
		NETERROR(MCACHE,("CacheRemove: Attempted to delete non existent entry"));
	}

	return NULL;
}

void *
CacheDelete(cache_t cache, void *data)
{
	void *item = NULL;
	// For TAVL so it returns the data instead of removing it.
	TAVL_NODE ret;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_DELETE, data, 0)))
	{
		return NULL;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		item = avlt_delete(cache->root, data);
		break;
	case CACHE_DT_AVLTR:
		item = avltr_delete(cache->root, data);
		break;
	case CACHE_DT_AVL:
		item = avl_delete(cache->root, data);
		break;
	case CACHE_DT_RB:
		item = rb_delete(cache->root, data);
		break;
	case CACHE_DT_TST:
		item = tst_delete(CacheData2KeyArray[cache->cachedata2Key](data), cache->root);
		break;
	case CACHE_DT_HASH:
		item = HashDelete(cache->root, data, (int (*)(void*))cache->_hash, (void* (*)(void*))cache->_entry2key, (int (*)(void*, void*))cache->_keycmp, cache->_hashlistOffset);
		break;
	case CACHE_DT_CLIST:
		item = list_delete((ListEntry **)&cache->root, cache->listoffset, data, cache->cachecmp);
		break;
	case CACHE_DT_TAVL:
		item = tavl_delete(cache->root,data,&ret);
		//item = ret.dataptr;
		break;
	}

	if (item)
	{
		if (cache->dt == CACHE_DT_HASH)
		  cache->nitems -= HashCountList(item, cache->_hashlistOffset);
		else
		  cache->nitems --;

		if (cache->flags & CACHE_MAINT_MIN)
		{
			if (cache->minData == item)
			{
				cache->minData = NULL;
			}
		}
	}
	
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_DELETE, item, 0);
	}
	
	return item;
}

void *
CacheInitIterator(cache_t cache)
{
	if (cache == NULL)
	{
		return NULL;
	}

	memset(&cache->iterator, 0, sizeof(cache_iterator));
	
	// return the start address
	return &cache->iterator.avl;
}

int CacheFreeIterator(cache_t cache)
{
	return(0);
}

/*
 * CacheApply -- cache is traversed, function fn is called with
 * arguments arg and the data portion of each node.  if fn returns stopflag,
 * the traversal is cut short, otherwise it continues.  Do not use -6 as
 * a stopflag, as this is what is used to indicate the traversal ran out
 * of nodes. - from our avl implementation
 *
 * FOR TAVL no Implementation provided. TAVL do not provide this functionality.
 * 
 */
int
CacheApply(cache_t cache, avl_node_func fn, void *arg, int stopflag, int type)
{
	if (cache == NULL)
	{
		return -1;
	}

	switch (cache->dt)
	{
	case CACHE_DT_AVLT:
		avlt_walk(cache->root, fn, arg);
		break;
	case CACHE_DT_AVLTR:
		avltr_walk(cache->root, fn, arg);
		break;
	case CACHE_DT_AVL:
		avl_walk(cache->root, fn, arg);
		break;
	case CACHE_DT_RB:
		rb_walk(cache->root, fn, arg);
		break;
	case CACHE_DT_CLIST:
		break;
	}

	return 0;
}

void *
CacheMatchFirstPrefix(cache_t cache, unsigned char *key)
{
	void *item = NULL;

	if (cache == NULL)
	{
		return NULL;
	}

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETFIRST, NULL, 0)))
	{
		return NULL;
	}
	
	// reset the iterator
	tst_iterator_init(&cache->iterator.tst, cache->root);

	switch (cache->dt)
	{
	case CACHE_DT_TST:
		item = tst_searchall(&cache->iterator.tst, key);
		break;
	}
	
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETFIRST, item, 0);
	}
	
	return item;
}

void *
CacheMatchNextPrefix(cache_t cache, unsigned char *key)
{
	void *item = NULL;

	if ((cache->pre_cond) &&
		((CachePreCbArray[cache->pre_cond])(cache, CACHEOP_GETFIRST, NULL, 0)))
	{
		return NULL;
	}
	
	switch (cache->dt)
	{
	case CACHE_DT_TST:
		item = tst_searchall(&cache->iterator.tst, key);
		break;
	}
	
	if (cache->post_cond)
	{
		(CachePostCbArray[cache->post_cond])(cache, CACHEOP_GETFIRST, item, 0);
	}
	
	return item;
}

void
libavl_init(int x)
{
	avl_init(x);
	avlt_init(x);
	avltr_init(x);
	rb_init(x);
}

char *
SHM_Strdup(char *str)
{
	char *copy;

	copy = (char *)SHM_Malloc(strlen(str)+1);
	if (copy)
	{
		strcpy(copy, str);
	}

	return copy;
}

char *
CStrdup(cache_t cache, char *str)
{
	char *copy = NULL;

	if (str)
	{
		copy = (char *)CMalloc(cache)(strlen(str)+1);
	}

	if (copy)
	{
		strcpy(copy, str);
	}

	return copy;	
}

#if 0
char *
MStrdup(int mallocfn, char *str)
{
	char *copy;

	copy = (char *)MMalloc(mallocfn, strlen(str)+1);
	if (copy)
	{
		strcpy(copy, str);
	}

	return copy;
}
#endif

