#ifndef _cache_h_
#define _cache_h_

#include <sys/types.h>
#include <inttypes.h>
#include "avl.h"
#include "avlt.h"
#include "avltr.h"
#include "rb.h"
#include "tst.h"
#include "hashtable.h"
#include "tavltree.h"

#include "lock.h"

#include "spthread.h"

// flags
#define CACHE_MAINT_MIN	0x00000001

#define CACHE_NAME_LEN	16

// datatype values
#define CACHE_DT_AVLT	0	// avl_tree
#define CACHE_DT_AVLTR	1	// avltr_tree
#define CACHE_DT_AVL	2	// avl_tree
#define CACHE_DT_RB		3	// rb_tree
#define CACHE_DT_TST	4	// TST Tries
#define CACHE_DT_HASH   5       // hashtable
#define CACHE_DT_CLIST  6       // CLIST API
#define CACHE_DT_TAVL	7	// TAVL TREE

typedef enum {
	CACHEOP_INSERT = 0,
	CACHEOP_DELETE,
	CACHEOP_GET,
	CACHEOP_GETFIRST,
	CACHEOP_GETNEXT,
	CACHEOP_LOCK,
	CACHEOP_UNLOCK,

	CACHEOP_MAX,
} cache_operations;

extern char cache_operations_strings[CACHEOP_MAX][50];

/* Cache Malloc functions */
#define CACHE_MALLOC_SHARED    0
#define CACHE_MALLOC_LOCAL     1
#define MEM_SHARED	0
#define MEM_LOCAL	1

extern void *((*CacheMalloc[2]))(size_t);
extern void ((*CacheFree[2]))(void *);

char *SHM_Strdup(char *);

#if defined(_DBMALLOC_) || defined(_DMALLOC_)
#define MMalloc(_typ_, _sz_)	((_typ_==MEM_LOCAL)?malloc(_sz_):\
										(CacheMalloc[_typ_])(_sz_))
#define MFree(_typ_, _ptr_)		do if (_ptr_) ((_typ_==MEM_LOCAL)?free(_ptr_):\
										(CacheFree[_typ_])(_ptr_)); while (0)
#else
#define MMalloc(_typ_, _sz_)		((CacheMalloc[_typ_])(_sz_))
#define MFree(_typ_, _ptr_)		((CacheFree[_typ_])(_ptr_))
#endif

#define CMalloc(_cache_)	(CacheMalloc[(_cache_)->malloc])
#define CFree(_cache_)		(CacheFree[(_cache_)->free])
#define MStrdup(_typ_, _ptr_)	((_typ_==MEM_LOCAL)?strdup(_ptr_):SHM_Strdup(_ptr_))

typedef union
{
	avl_traverser avl;
	avlt_traverser avlt;
	avltr_traverser avltr;
	rb_traverser rb;
	tst_iterator tst;
	TAVL_nodeptr tavl_iterator_node;

} cache_iterator;

typedef struct cache_t
{
	/* Root of the cache */
	int dt;

	// can be Avlnode, avl_tree, avlt_tree, avltr_tree, rb_tree, hashtable
	void *root;	

	void *minData;	// try to maintain min

#define _numBuckets xitems  // for hashtable, store the number of buckets here
	int	xitems;
	int	nitems;

#define _hashlistOffset nodeLineWidth  // for hashtable, store the linked list offset here
	int nodeLineWidth;	// for TST
#define _entry2key cachedatains2Key  // for hashtable, function pointer
	uintptr_t	cachedatains2Key;
#define _hash cachedata2Key   // for hashtable, hash function pointer
	uintptr_t	cachedata2Key;	

	unsigned long	flags;	

	/* Callback functions need to be initialized by the appl */
	int	cacheinscmp;	/* Insert cmp function */
#define _keycmp cachecmp   // for hashtable, function pointer to compare keys
	uintptr_t	cachecmp;	/* cmp function for other ops using keys */
	int	cachedup;	/* dup error fn */

	// This lock may be shared with another cache,
	// if the data contained in the cache is shared
	Lock	*lock;

	/* allocation functions, if used */
	int	malloc;
	int free;

	char 	*name;

	/* Callback functions, to be called at various
	 * operations. These are called only if defined.
	 */
	int	pre_cond;
	int post_cond;

	/* iterator for traversal */
	cache_iterator iterator;
	
	int listoffset;
} *cache_t;

/* Initialize the libavl package to use SHMEM or not */
void libavl_init(int);
#define AVL_NOT_SHMEM (TRUE)
#define AVL_USE_SHMEM (FALSE)


/* Basic Cache routines */
cache_t CacheCreate(int);
int CacheDestroy(cache_t cache);
int CacheInstantiate(cache_t cache);

/* Acquire/Release locks on the cache */
int CacheGetLocks(cache_t cache, int mode, int block);
int CacheReleaseLocks(cache_t cache);

/* Get a copy of the entry from the cache */
int CacheFind(cache_t cache, void *data, void *copy, size_t size);
int CacheFindFirst(cache_t cache, void *copy, size_t size);
int CacheFindNext(cache_t cache, void *data, void *copy, size_t size);

/* Get an entry from the cache: This will be a locked entry */
void *CacheGet(cache_t cache, void *data);
void *CacheGetFirst(cache_t cache);
void *CacheGetNext(cache_t cache, void *data);
void * CacheGetPrev(cache_t cache, void *data);
void * CacheGetMin(cache_t cache);
void ** CacheGetFast(cache_t cache, void *data);
void ** CacheGetNextFast(cache_t cache, void **data);
void ** CacheGetPrevFast(cache_t cache, void **data);
void *CacheInitIterator(cache_t cache);
int CacheFreeIterator(cache_t cache);
void * CacheGetLast(cache_t cache);

// Walk APIs
void * CacheMatchFirstPrefix(cache_t cache, unsigned char *key);
void * CacheMatchNextPrefix(cache_t cache, unsigned char *key);

/* Insertion routines */
int CacheInsert(cache_t cache, void *data);
int CachePut(cache_t cache, void *copy, size_t size);
int CacheOverwrite(cache_t cache, void *copy, size_t size, void *key);

/* Deletion routines */
void *CacheDelete(cache_t cache, void *data);
void *CacheRemove(cache_t cache, void *entry);

/* Application, post/pre/in order */
int CacheApply(cache_t cache, avl_node_func fn, void *arg, int stopflag, int type);

extern avl_comparison_func CacheinscmpArray[];
extern avl_comparison_func CachecmpArray[];
extern avl_comparison_func CachedupArray[];

/* Cache Callbacks are called at the entry and exit points from the cache */
typedef	int (*CacheCallbacks)(struct cache_t *cache, int op, void *data, size_t size);
extern CacheCallbacks CachePreCbArray[];
extern CacheCallbacks CachePostCbArray[];

int CacheIedgeRegInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeRegidInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgePhoneInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeVpnPhoneInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeEmailInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeIpInsCmp(const void *v1, const void *v2, void *param);
int CacheCallInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeUriInsCmp(const void *v1, const void *v2, void *param);
int CacheConfInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeH323IdInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeSubnetInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeCrIdInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeTGInsCmp(const void *v1, const void *v2, void *param);
int CacheGuidInsCmp(const void *v1, const void *v2, void *param);
int CacheTriggerInsCmp(const void *v1, const void *v2, void *param);
int CacheRealmInsCmp(const void *v1, const void *v2, void *param);
int CacheRsaInsCmp(const void *v1, const void *v2, void *param);
int CacheTipInsCmp(const void *v1, const void *v2, void *param);
int CacheIgrpInsCmp(const void *v1, const void *v2, void *param);
int CacheVnetInsCmp(const void *v1, const void *v2, void *param);
int CacheRsapubnetsInsCmp(const void *v1, const void *v2, void *param);
int CacheSCMCallInsCmp(const void *v1, const void *v2, void *param);

int CacheIedgeRegCmp(const void *v1, const void *v2, void *param);
int CacheIedgeRegidCmp(const void *v1, const void *v2, void *param);
int CacheIedgePhoneCmp(const void *v1, const void *v2, void *param);
int CacheIedgeVpnPhoneCmp(const void *v1, const void *v2, void *param);
int CacheIedgeEmailCmp(const void *v1, const void *v2, void *param);
int CacheIedgeIpCmp(const void *v1, const void *v2, void *param);
int CacheCPCmp(const void *v1, const void *v2, void *param);
int CacheCPInsCmp(const void *v1, const void *v2, void *param);
int CacheVpnCmp(const void *v1, const void *v2, void *param);
int CacheVpnGCmp(const void *v1, const void *v2, void *param);
int CacheCallCmp(const void *v1, const void *v2, void *param);
int CacheIedgeUriCmp(const void *v1, const void *v2, void *param);
int CacheConfCmp(const void *v1, const void *v2, void *param);
int CacheCPBCmp(const void *v1, const void *v2, void *param);
int CacheCPBInsCmp(const void *v1, const void *v2, void *param);
int CacheIedgeGkCmp(const void *v1, const void *v2, void *param);
int CacheIedgeH323IdCmp(const void *v1, const void *v2, void *param);
int CacheIedgeSubnetCmp(const void *v1, const void *v2, void *param);
int CacheIedgeCrIdCmp(const void *v1, const void *v2, void *param);
int CacheIedgeTGCmp(const void *v1, const void *v2, void *param);

int CacheGuidCmp(const void *v1, const void *v2, void *param);

int CacheTriggerCmp(const void *v1, const void *v2, void *param);
int CacheRealmCmp(const void *v1, const void *v2, void *param);
int CacheRsaCmp(const void *v1, const void *v2, void *param);
int CacheTipCmp(const void *v1, const void *v2, void *param);
int CacheIgrpCmp(const void *v1, const void *v2, void *param);
int CacheVnetCmp(const void *v1, const void *v2, void *param);
int CacheRsapubnetsCmp(const void *v1, const void *v2, void *param);
int CacheSCMCallCmp(const void *v1, const void *v2, void *param);

int CacheTransCmp(const void *v1, const void *v2, void *param);
int CacheTransInsCmp(const void *v1, const void *v2, void *param);
int CacheTransDestCmp(const void *v1, const void *v2, void *param);
int CacheTransDestInsCmp(const void *v1, const void *v2, void *param);
int CacheSipCallCmp(const void *v1, const void *v2, void *param);
int CacheSipCallInsCmp(const void *v1, const void *v2, void *param);
int CacheRoutesLRUInsCmp(const void *v1, const void *v2, void *param);
int timerCompare(const void *e1, const void *e2, void *param);
int timerHandleCompare(const void *e1, const void *e2, void *param);
int timerHandleInsCompare(const void *e1, const void *e2, void *param);

int CacheSipRegCmp(const void *v1, const void *v2, void *param);
int CacheSipRegInsCmp(const void *v1, const void *v2, void *param);

int CacheIedgeCachePreCb(struct cache_t *cache, int op, void *data, size_t size);
int CacheVpnCachePreCb(struct cache_t *cache, int op, void *data, size_t size);
int CacheVpnGCachePreCb(struct cache_t *cache, int op, void *data, size_t size);
int CacheCallCachePreCb(struct cache_t *cache, int op, void *data, size_t size);

int CacheIedgeCachePostCb(struct cache_t *cache, int op, void *data, size_t size);
int CacheVpnCachePostCb(struct cache_t *cache, int op, void *data, size_t size);
int CacheVpnGCachePostCb(struct cache_t *cache, int op, void *data, size_t size);

int TimerPreCbPrint(struct cache_t *cache, int op, void *data, size_t size);
int TimerPostCbPrint(struct cache_t *cache, int op, void *data, size_t size);
int TimerHandlePreCbPrint(struct cache_t *cache, int op, void *data, size_t size);
int TimerHandlePostCbPrint(struct cache_t *cache, int op, void *data, size_t size);

int LruRoutesPostCbCheck(struct cache_t *cache, int op, void *data, size_t size);

typedef void * ( * data2keyfn)(void *arg);
extern data2keyfn CacheData2KeyArray[];
extern data2keyfn CacheInsData2KeyArray[];

void *CacheIedgePhoneData2Key(void *);
void *CacheIedgePhoneInsData2Key(void *);
void *CacheCPData2Key(void *);
void *CacheCPInsData2Key(void *);
void *CacheCPBCRData2Key(void *);
void *CacheCPBCRInsData2Key(void *);
void *CacheIedgeGwCPData2Key(void *);
void *CacheIedgeGwCPInsData2Key(void *);
void *CacheCPBCPData2Key(void *);
void *CacheCPBCPInsData2Key(void *);
void *CacheCPSrcInsData2Key(void *);
void CacheSetName (cache_t cache, char *name);
int CacheDestroyKeys (cache_t cache);
int CacheDestroyData (cache_t cache);
int CacheProbeInsert (cache_t cache, void *data, void **existData);

#endif /* _cache_h_ */	
