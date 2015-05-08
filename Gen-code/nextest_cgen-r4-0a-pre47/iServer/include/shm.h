#ifndef _shm_h_
#define _shm_h_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

//#define PAGE_SIZE	4096	// Original Page Size
#define PAGE_SIZE	16384
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define SHMLBA_MASK	(~(SHMLBA-1))

#ifdef SUNOS
#define DEFAULT_SHMFLAGS	(SHM_R|SHM_W|SHM_RND)
#else
#define DEFAULT_SHMFLAGS	(SHM_R|SHM_W|SHM_RND)
#endif

#define MF_USED 0xffaa0055
#define MF_DMA  0xff00aa55
#define MF_FREE 0x0055ffaa

/*
 * A block header. This is in front of every malloc-block, whether free or not.
 */
typedef struct BlockHeader {
        unsigned long bhFlags;
        union {
                unsigned long ubhLength;
                struct BlockHeader *fbhNext;
        } vp;
} BlockHeader;


#define bhLength 	vp.ubhLength
#define bhNext   	vp.fbhNext
#define BH(p) 		((struct BlockHeader *)(p))

typedef struct PageDescriptor
{
	struct PageDescriptor *next;	
	struct BlockHeader *firstFree;
	int order;
	int nFree;
} PageDescriptor;

#define PAGE_DESC(p) ((PageDescriptor *)(((unsigned long)(p)) & PAGE_MASK))

/*
 * A size descriptor describes a specific class of malloc sizes.
 * Each class of sizes has its own freelist.
 */
typedef struct SizeDescriptor {
        struct PageDescriptor *firstFree;
        int nBlocks;

        int nMallocs;
        int nFrees;
        int nBytesMalloced;
        int nPages;
        unsigned long gfporder; /* number of pages in the area required */
} SizeDescriptor;

#define NSIZES	14

extern unsigned int blockSize[NSIZES]; 

extern struct SizeDescriptor *sizes;

extern struct SizeDescriptor sizesLocal[NSIZES];

#define NBLOCKS(order)          (sizes[order].nBlocks)
#define BLOCKSIZE(order)        (blockSize[order])
#define AREASIZE(order)         (PAGE_SIZE<<(sizes[order].gfporder))

/* Only entry 0 in the following array will be valid for now */
extern PageDescriptor **ShmPages;

void * 
SHM_Malloc(size_t size);

void 
SHM_Free(void *__ptr);

void *
SHM_Realloc(void *__ptr, size_t nsize);

extern void SHM_InitBlockSizes (void);
extern int SHM_AttachSegment (key_t key, int totalSize, void **addr, int shmFlag);
extern int SHM_PaginateSegment (void *addr, int segSize, PageDescriptor **pageListPtr, int pageSize);
extern long int SHM_MallocTest (void);
extern int SHM_MallocBlockTest (void);


#endif /* _shm_h_ */
