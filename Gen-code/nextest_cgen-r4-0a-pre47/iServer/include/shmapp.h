#ifndef _shm_app_h_
#define _shm_app_h_

#include <pthread.h>


/* Storage of the memory map:
 * shmid, size, flags, n pages
 * free page list
 */

#define MAX_PAGE_ORDERS	6

typedef struct
{
	int	segSize;
	int	segFlags;
	int	segKey;
	int 	shmId;
	void	*mapAddr;
	int	sizeUsed;
} SegMap;

typedef struct
{
	int	nPages[MAX_PAGE_ORDERS];
	PageDescriptor *freePages[MAX_PAGE_ORDERS];
} PageMap;

typedef struct
{
	int	len;	/* length of this whole structure */
	int	msegs;	/* Max no of segments which may be used */
	int 	mpages; /* Max number of pages which we need */
	int	nsegs;	/* no of segments which are used */
	int 	npages; /* number of pages which are used */
	struct SizeDescriptor sizes[NSIZES];
	PageMap	pmap;	

	int 	semid;	/* Serialize access to this structure */
	int	qid;

	/* Application stuff */
	void	*appStruct;

	int	ready;	/* Is this structure ready ?? */
	SegMap	segs[1];
} MemoryMap;

/* Initialize a memory map */
MemoryMap *
SHMAPP_Init(key_t key, void **addr, int size, int nsegs, int npages, int segaddrtype);

/* Create the memory map specified in the segment configs */
int
SHMAPP_AllocMap(MemoryMap *map);

MemoryMap *
SHMAPP_UseMap(key_t key, void **addr, int size, int *nsegs, int *npages);

#define DEFAULT_SEG_SIZE	0x100000

#define SHM_SEMAPHORE 		0x1
#define SHM_QUEUE		0x1

extern MemoryMap *memoryMap;

typedef struct _shm_app_struct
{
	key_t	key;
	void 	*startAddr;
	size_t	segSize;
	int		msegs;
	int		mpages;
	void 	*map;
	int		nattached;
	int		segaddrtype;
	pthread_mutex_t 	mutex;	
					/* Serialize access to this structure. Note that
					 * this serialization is for threads in the same
					 * process, as this is a local structure.
					 */ 
	int		unused[4];
} shm_struct_t;
	
typedef enum
{
	ISERVER_CACHE_INDEX = 0,

	CACHE_INDEX_MAX,
} shm_cache_index_t;

extern shm_struct_t shmStruct[];

#define SHM_Config(_x_)	(&shmStruct[_x_])

#define ISERVER_SHM_KEY 	1
/* #define ISERVER_SHM_STARTADDR	0x60ae8000*/
#define ISERVER_SHM_STARTADDR    0xB0000000

extern int SHM_Create (int index, void **map);
extern int SHM_Attach (int index, void **map);
extern int SHM_Detach (int index, MemoryMap *map);
extern int SHM_IsAttached (int index);
extern int SHM_AddAttached (int index);
extern int SHM_DelAttached (int index);
extern int SHM_MakeReady (int index);
extern int SHMAPP_ReleaseMap (MemoryMap *map);
extern int SHMAPP_DetachMap (MemoryMap *map);
extern void PrintBlocks (MemoryMap *map, int order);
extern int PrintMap (MemoryMap *map);
extern int SHM_Init (int index);

#endif /* _shm_app_h_ */
