#include <sys/types.h>
#include "shm.h"
#include "shmapp.h"
#include <malloc.h>
#include <ipcutils.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "nxosd.h"

#ifdef _go_back_to_4K_page_size_

unsigned int blockSize[NSIZES] = {
        32,
        64,
        128,
        252,
        508,
        1020,
        2040,
        4096 - 16,
        8192 - 16,
        16384 - 16,
        32768 - 16,
        65536 - 16,
        131072 - 16,
        0
};

struct SizeDescriptor sizesLocal[NSIZES] =
{
        {NULL, 127, 0, 0, 0, 0, 0},
        {NULL, 63, 0, 0, 0, 0, 0},
        {NULL, 31, 0, 0, 0, 0, 0},
        {NULL, 16, 0, 0, 0, 0, 0},
        {NULL, 8, 0, 0, 0, 0, 0},
        {NULL, 4, 0, 0, 0, 0, 0},
        {NULL, 2, 0, 0, 0, 0, 0},
        {NULL, 1, 0, 0, 0, 0, 0},
        {NULL, 1, 0, 0, 0, 0, 1},
        {NULL, 1, 0, 0, 0, 0, 2},
        {NULL, 1, 0, 0, 0, 0, 3},
        {NULL, 1, 0, 0, 0, 0, 4},
        {NULL, 1, 0, 0, 0, 0, 5},
        {NULL, 0, 0, 0, 0, 0, 0}
};

int SHM_InitBlockSizes() { return 0; };

#else /* _go_back_to_4K_page_size_ */

unsigned int blockSize[NSIZES];
struct SizeDescriptor sizesLocal[NSIZES];

#define MIN_BLOCK_SIZE	32

void
SHM_InitBlockSizes()
{
	int i, curb;

	curb = MIN_BLOCK_SIZE;

	for (i=0; i<NSIZES-1; i++)
	{
		// Initialize the sizes array
		memset(&sizesLocal[i], 0, sizeof(SizeDescriptor));

		blockSize[i] = curb;
		if (blockSize[i] >= PAGE_SIZE)
		{
			blockSize[i] -= sizeof(PageDescriptor);
			sizesLocal[i].nBlocks = 1;
		}
		else
		{
			sizesLocal[i].nBlocks = PAGE_SIZE/blockSize[i] - 1;
		}

		curb = curb << 1;

		sizesLocal[i].gfporder = blockSize[i]/PAGE_SIZE;
	}

	blockSize[NSIZES-1] = 0;

	// Initialize the sizes array
	memset(&sizesLocal[NSIZES-1], 0, sizeof(SizeDescriptor));
}

#endif

#ifdef _example_for_32k_page_size_

// for page size of 32 K
const unsigned int blockSize[NSIZES] = {
        32,
        64,
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768 - 16,
        65536 - 16,
        131072 - 16,
        0
};

struct SizeDescriptor sizesLocal[NSIZES] =
{
        {NULL, 1023, 0, 0, 0, 0, 0},
        {NULL, 511, 0, 0, 0, 0, 0},
        {NULL, 255, 0, 0, 0, 0, 0},
        {NULL, 127, 0, 0, 0, 0, 0},
        {NULL, 63, 0, 0, 0, 0, 0},
        {NULL, 31, 0, 0, 0, 0, 0},
        {NULL, 15, 0, 0, 0, 0, 0},
        {NULL, 7, 0, 0, 0, 0, 0},
        {NULL, 3, 0, 0, 0, 0, 0},
        {NULL, 1, 0, 0, 0, 0, 0},
        {NULL, 1, 0, 0, 0, 0, 0},
        {NULL, 1, 0, 0, 0, 0, 1},
        {NULL, 1, 0, 0, 0, 0, 2},
        {NULL, 0, 0, 0, 0, 0, 0}
};
#endif

struct SizeDescriptor *sizes;

/* Initialized by application */
MemoryMap *memoryMap;

/* Define these to appropriate values if you want to debug */
#define ERROR(x,y)	
#define MSHM 0

/* Get a shared memory segment.
 * Key, totalsize, and flags are specified
 * return shmid.
 */
int
SHM_AllocateSegment(
	key_t key, 
	int totalSize, 
	int shmFlag
)
{
	int shmId;

    /* Allocate the segment */
    shmId = shmget(key, totalSize, DEFAULT_SHMFLAGS|shmFlag|IPC_CREAT);
	if (shmId == -1)
   	{
		int	lerrno = errno;

		ERROR(MSHM, ("shmget returned errno %d - %s\n",
				lerrno,
				strerror(lerrno) ));
          	return -1;
     }

     	return shmId;
}

/* Attach the memory at a specified virtual memory address 
 * return shmid
 */

int
SHM_AttachSegment(
	key_t key, 
	int totalSize, 
	void **addr,
	int shmFlag
)
{
	int shmId;

	shmId = shmget(key, totalSize, shmFlag);
	if (shmId == -1)
	{
		int	lerrno = errno;

		ERROR(MSHM, ("shmget returned errno %d - %s \n",
				lerrno,
				strerror(lerrno) ));
		return -1;
	}
	
	*addr  = shmat(shmId, (char *)*addr, shmFlag);
	
	if (*addr == (caddr_t)-1)
	{
		int	lerrno = errno;

		ERROR(MSHM, ("shmat returned errno %d - %s\n",
				lerrno,
				strerror(lerrno) ));
		return -1;
	}

	return shmId;
}

/* Given a start address and sizeof segment starting from
 * the start address, divide the segment into pages, of
 * size page_size, and add them into a list, provided
 * as an argument to the function.
 * Start address should be at a long boundary.
 * After this we have a segment, split up into pages, each of size
 * pageSize, and inside each page, the firstBlockPtr set to the
 * offset from the page of the header (pageDescriptor). The link list
 * of pages is maintained inside the headers.
 * The pages will be divided into blocks, when their order is
 * known. The order will be known, at the time of a malloc.
 */
int
SHM_PaginateSegment(
	void 		*addr,
	int 		segSize,
	PageDescriptor 	**pageListPtr,
	int 		pageSize
)
{
	int i = 0;
	int sizeUsed = 0;
	PageDescriptor *pageDescriptor = (PageDescriptor *)*pageListPtr;
	void *lastAddr = *pageListPtr;

	pageDescriptor = (PageDescriptor *)addr;

	while ((sizeUsed + pageSize) < segSize)
	{
		i++;
		pageDescriptor->next = lastAddr;
		lastAddr = pageDescriptor;

		/* Initialize what will later be used for blocks */
		pageDescriptor->order = 0;
		pageDescriptor->firstFree = 
			(BlockHeader *)(pageDescriptor + 1);

		pageDescriptor = (PageDescriptor *)
					((caddr_t)pageDescriptor + pageSize);
		sizeUsed += pageSize;
	}	
	
	*pageListPtr = lastAddr;
	
	return i;
}

/* Go through the list and get a page */
PageDescriptor *
SHM_GetFreePage(int gpforder)
{
	PageDescriptor *page;

	page = memoryMap->pmap.freePages[gpforder];

	if (page)
	{
		memoryMap->pmap.freePages[gpforder] = page->next;
		memoryMap->pmap.nPages[gpforder] --;
	}
	return page;
}

int
SHM_FreePage(PageDescriptor *page, int gpforder)
{
	page->next = memoryMap->pmap.freePages[gpforder];
	memoryMap->pmap.freePages[gpforder] = page;
	memoryMap->pmap.nPages[gpforder] ++;
	return(0);
}

long 
SHM_MallocTest(void)
{
	int order;

	/*
 	* Check the static info array. Things will blow up terribly if it's
 	* incorrect. This is a late "compile time" check.....
 	*/

	for (order = 0; BLOCKSIZE(order); order++) 
	{
		if ((NBLOCKS(order) * BLOCKSIZE(order) + 
			sizeof(struct PageDescriptor)) > AREASIZE(order)) 
		{
			printf("Cannot use %d bytes out of %d in order = %d block mallocs\n",
			       (int) (NBLOCKS(order) * BLOCKSIZE(order) +
				      sizeof(struct PageDescriptor)),
			        (int) AREASIZE(order),
			       BLOCKSIZE(order));
			printf("ERROR in SHM\n");
		}
	}

	return 0;
}

int
SHM_MallocBlockTest(void)
{
	int order, i;
	unsigned int realsize;
	void *ptr[100];
	int success;

	// Try allocating 100 blocks for each of the NBLOCKS

	/*
	* Check if we can allocate around 100 blocks and free them up
 	*/

	for (order = 0; (BLOCKSIZE(order) && !sizes[order].gfporder); order++) 
	{
		success = 1;

		for (i = 0; i < 100; i++)
		{
			ptr[i] = NULL;
		}

		for (i = 0; i < 100; i++)
		{
			realsize = BLOCKSIZE(order) - sizeof(struct BlockHeader);

			if (ptr[i] = SHM_Malloc(realsize))
			{
				// success
			}
			else
			{
				// failure
				success = 0;
				break;
			}
		}

		for (i = 0; i < 100; i++)
		{
			if (ptr[i])
			{
				SHM_Free(ptr[i]);
			}
			else
			{
				// failure
			}
		}

		if (!success)
		{
			printf("error in allocating blocks of size %d\n", realsize);
		}
		else
		{
			printf("block %d OK\n", realsize);
		}
	}

	return 0;
}

/*
 * Ugh, this is ugly, but we want the default case to run
 * straight through, which is why we have the ugly goto's
 */
void * 
SHM_Malloc(size_t size)
{
	unsigned long flags;
	unsigned long type;
	int order;
	struct BlockHeader *p;
	struct PageDescriptor *page, **pg;
	struct SizeDescriptor *bucket = sizes;

	if ((size <= 0) || !memoryMap)
	{
		return NULL;
	}

	/* Get order */
	order = 0;
	{
		unsigned int realsize = size + sizeof(struct BlockHeader);
		for (;;) {
			int ordersize = BLOCKSIZE(order);
			if (realsize <= ordersize)
				break;
			order++;
			bucket++;
			if (ordersize)
				continue;
			printf("malloc of too large a block (%d bytes).\n", (int) size);
			return NULL;
		}
	}

	/* Acquire Locks */
	sm_p(memoryMap->semid, 0, 0);

	type = MF_USED;
	pg = &bucket->firstFree;

	page = *pg;
	if (!page)
	{
		goto no_bucket_page;
	}

	p = page->firstFree;
	if (p->bhFlags != MF_FREE)
	{
		goto not_free_on_freelist;
	}

found_it:
	page->firstFree = p->bhNext;
	page->nFree--;
	if (!page->nFree)
	{
		*pg = page->next;
	}
	bucket->nMallocs++;
	bucket->nBytesMalloced += size;
	p->bhFlags = type;	/* As of now this block is officially in use */
	p->bhLength = size;
#ifdef SADISTIC_KMALLOC
	memset(p+1, 0xf0, size);
#endif
	sm_v(memoryMap->semid);

	return p + 1;		/* Pointer arithmetic: increments past header */


no_bucket_page:
	/*
	 * If we didn't find a page already allocated for this
	 * bucket size, we need to get one..
	 *
	 */

	{
		int i, sz;
		
		/* sz is the size of the blocks we're dealing with */
		sz = BLOCKSIZE(order);

		page = SHM_GetFreePage(bucket->gfporder);
		if (!page)
		{
			goto no_free_page;
		}
found_cached_page:

		bucket->nPages++;

		page->order = order;
		/* Loop for all but last block: */
		i = (page->nFree = bucket->nBlocks) - 1;
		p = BH(page + 1);
		while (i > 0) {
			i--;
			p->bhFlags = MF_FREE;
			p->bhNext = BH(((long) p) + sz);
			p = p->bhNext;
		}
		/* Last block: */
		p->bhFlags = MF_FREE;
		p->bhNext = NULL;

		p = BH(page+1);
	}

	/*
	 * Now we're going to muck with the "global" freelist
	 * for this size: this should be uninterruptible
	 */
	page->next = *pg;
	*pg = page;
	goto found_it;

no_free_page:
	printf("Problem: no free page\n");
	sm_v(memoryMap->semid);
	return NULL;

not_free_on_freelist:
	printf("Problem: block on freelist at %08lx isn't free.\n", (long) p);
	sm_v(memoryMap->semid);
	return NULL;
}

void *
SHM_Realloc(void *__ptr, size_t nsize)
{
	size_t 	csize;
	void 	*nptr;
	struct BlockHeader *bptr;

	/* Figure out the size, 
	 * Allocate new block,
	 * Make copy,
	 * Free the original
	 */
	
	if ((nsize == 0) && __ptr)
	{
		/* Behave like free */
		SHM_Free(__ptr);
		return NULL;
	}

	if (!__ptr && (nsize > 0))
	{
		/* Behave like malloc */
		return SHM_Malloc(nsize);
	}

	/* sanity check */
	if (!__ptr)
	{
		return NULL;
	}

	bptr = (struct BlockHeader *)__ptr;
	bptr = bptr - 1;

	csize = bptr->bhLength;
	if (csize > nsize)
	{
		csize = nsize;
	}

	nptr = SHM_Malloc(nsize);
	if (nptr)
	{
		memcpy(nptr, __ptr, csize);
	}

	SHM_Free(__ptr);

	return nptr;
}

void 
SHM_Free(void *__ptr)
{
	unsigned long flags;
	unsigned int order;
	struct PageDescriptor *page, **pg;
	struct SizeDescriptor *bucket;

	if (!__ptr)
		goto null_kfree;

	/* Acquire Locks */
	sm_p(memoryMap->semid, 0, 0);

#define ptr ((struct BlockHeader *) __ptr)
	page = PAGE_DESC(ptr);
	__ptr = ptr - 1;
	if (~SHMLBA_MASK & (unsigned long)page->next)
	{
		goto bad_order;
	}
	order = page->order;
	if (order >= /* sizeof(sizes) / sizeof(sizes[0]) */NSIZES)
	{
		goto bad_order;
	}
	bucket = sizes + order;
	pg = &bucket->firstFree;
	if (ptr->bhFlags != MF_USED)
	{
		goto bad_order;
	}
	ptr->bhFlags = MF_FREE;	/* As of now this block is officially free */
#ifdef SADISTIC_KMALLOC
	memset(ptr+1, 0xe0, ptr->bhLength);
#endif

	bucket->nFrees++;
	bucket->nBytesMalloced -= ptr->bhLength;

	ptr->bhNext = page->firstFree;
	page->firstFree = ptr;
	if (!page->nFree++) 
	{
/* Page went from full to one free block: put it on the freelist. */
		if (bucket->nBlocks == 1)
		{
			goto free_page;
		}
		page->next = *pg;
		*pg = page;
	}
/* If page is completely free, free it */
	if (page->nFree == bucket->nBlocks) {
		for (;;) {
			struct PageDescriptor *tmp = *pg;
			if (!tmp)
				goto not_on_freelist;
			if (tmp == page)
				break;
			pg = &tmp->next;
		}
		*pg = page->next;
free_page:
		bucket->nPages--;
		SHM_FreePage(page, bucket->gfporder);
	}

	sm_v(memoryMap->semid);

null_kfree:
	return;

bad_order:
	sm_v(memoryMap->semid);

	printf("SHM_Free of non-malloced memory[%lu]: %p, next= %p, order=%d\n",
		ULONG_FMT(getpid()),
	       ptr+1, page->next, page->order);
	return;

not_on_freelist:
	sm_v(memoryMap->semid);

	printf("Ooops. page %p doesn't show on freelist.\n", page);
}
