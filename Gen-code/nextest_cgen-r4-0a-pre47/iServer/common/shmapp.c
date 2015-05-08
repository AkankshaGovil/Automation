#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <syslog.h>
#include <stdio.h>
#include <spthread.h>
#include <string.h>
#include "db.h"
#include "shm.h"
#include "shmapp.h"
#include "ipcutils.h"
#include <malloc.h>
#include "nxosd.h"

int checkMapReady = 1;

shm_struct_t shmStruct[] = {
	{
		ISERVER_SHM_KEY,
		(void *) 0 /* ISERVER_SHM_STARTADDR */,
		0x100000,	/* To change this default size from 1 meg, use server.cfg */
		5,
		1000,
	},
};

PageDescriptor **ShmPages = NULL;

/* Must be called before any shared memory can be
 * created/attached to. We dont need the slocks library
 * here, as this function is totally process internal.
 */
int
SHM_Init(int index)
{
	pthread_mutexattr_t mattr;
	//setpshared call is now availaible on linux too. So removed the ifdef SUNOS condition
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&shmStruct[index].mutex, &mattr);


	SHM_InitBlockSizes();

	return 0;
}

int
SHM_Create(int index, void **map)
{
	char fn[] = "SHM_Create():";

	if (index >= CACHE_INDEX_MAX)
	{
		printf("%s Invalid index %d\n", fn, index);
		return -1;
	}

	*map = SHMAPP_Init(shmStruct[index].key, 
		&shmStruct[index].startAddr, 
		shmStruct[index].segSize, 
		shmStruct[index].msegs, 
		shmStruct[index].mpages,
		shmStruct[index].segaddrtype);

	if (*map == NULL)
	{
		printf("%s SHMAPP_Init failed!\n", fn);
		return -1;
	}
	
	SHMAPP_AllocMap(*map);	

	if (spthread_init() < 0)
	{
		printf("%s spthread_init failed!\n", fn);
		return -1;
	}

	/* Assign in local memory */
	shmStruct[index].map = *map;
	SHM_AddAttached(index);

	// Do a test here
	SHM_MallocTest();

	return index;
}

int
SHM_Attach(int index, void **map)
{ 
	char fn[] = "SHM_Attach():";
	int nsegs, npages; 
	void *startAddr;
	int rc = index;
	int status;

	if (index >= CACHE_INDEX_MAX)
	{
		printf("%s Invalid index %d\n", fn, index );
		return -1;
	}

	if ((status = pthread_mutex_lock(&shmStruct[index].mutex)) != 0)
	{
		printf("shm mutex lock failed errno %d\n", status);
		return -1;
	}

	if (SHM_AddAttached(index))
	{
		/* Already attached */
		rc = index;
		goto _return;
	}

	startAddr = shmStruct[index].startAddr;

	*map = SHMAPP_UseMap(
		shmStruct[index].key,
		&startAddr,
		shmStruct[index].segSize,
		&nsegs, &npages);

	if (*map == NULL)
	{
		SHM_DelAttached(index);
		rc = -1;
		goto _return;
	}

	if (spthread_init() < 0)
	{
		printf("%s spthread_init failed!\n", fn);
		SHM_DelAttached(index);
		rc = -1;
		goto _return;
	}

	/* Assign in local memory */
	shmStruct[index].map = *map;

_return:
	if ((status = pthread_mutex_unlock(&shmStruct[index].mutex)) != 0)
	{
		printf("shm mutex unlock failed errno %d\n", status);
		return -1;
	}

	return rc;
}

int
SHM_Detach(int index, MemoryMap *map)
{
	MemoryMap *lmap;

	if (pthread_mutex_lock(&shmStruct[index].mutex) < 0)
	{
		printf("shm mutex lock failed errno\n");
		return -1;
	}

	if (SHM_DelAttached(index))
	{
		/* Not fully detached */
		goto _return;
	}

	spthread_exit();

/*	lmap = (MemoryMap *)malloc(map->len);
	memcpy(lmap, (char *)map, map->len); */
#if     0
        lmap->segs = ( SegMap * )(lmap+1);
#endif
        SHMAPP_DetachMap(map);
        /* free(lmap); */

	/* Assign in local memory */
	shmStruct[index].map = NULL;

_return:
	if (pthread_mutex_unlock(&shmStruct[index].mutex) < 0)
	{
		printf("shm mutex lock failed errno\n");
		return -1;
	}

	return index;
}

int
SHM_Destroy(int index, MemoryMap *map)
{
    MemoryMap *lmap;

	/* Destroy the semaphore and queue we created.
	 * This is not utmost necessary, but for completeness.
	 * If this step is not executed, the iserver will keep
	 * re-using them, as key is always constant.
	 */
	if (sm_delete(map->semid) < 0)
	{
		printf("error deleting semaphore %d\n",
			map->semid);
	}

	if (q_vdelete(map->qid) < 0)
	{
		printf("error deleting queue %d\n",
			map->qid);
	}

    lmap = (MemoryMap *)malloc(map->len);

    memcpy(lmap, (char *)map, map->len);
#if 0
    lmap->segs = ( SegMap * )(lmap+1);
#endif
    spthread_exit();
    SHMAPP_ReleaseMap(lmap);
    free(lmap);

	/* Assign in local memory */
	shmStruct[index].map = NULL;
	shmStruct[index].nattached = 0;

	/* Delete the semaphores associated w/ the databases */
	DbDeleteLocks();
	return(0);
}

int
SHM_IsAttached(int index)
{
	return (shmStruct[index].map != NULL);
}

int
SHM_AddAttached(int index)
{
	return shmStruct[index].nattached++;
}

int
SHM_DelAttached(int index)
{
	return --shmStruct[index].nattached;
}

int
SHM_IsReady(int index)
{
	MemoryMap *map;

	map = shmStruct[index].map;
	return (map && map->ready);
}

int
SHM_MakeReady(int index)
{
	MemoryMap *map;

	map = shmStruct[index].map;

	if (map)
	{
		map->ready = 1;
	}

	return map->ready;
}

int
WakeupPid(pid_t pid)
{
	char fn[] = "WakeupPid():";

	if (memoryMap == NULL)
	{
		fprintf(stderr, "%s No Shared Memory\n", fn);
		return -1;
	}

	if (q_vsend(memoryMap->qid, &pid, sizeof(pid), 0) < 0)
	{
		fprintf(stderr, "%s q_vsend failed!!!\n", fn);
	}

	return 0;
}

int
WaitForMsg(pid_t pid)
{
	char fn[] = "WaitForMsg():";
	pid_t msg;
	ssize_t len;

	if (memoryMap == NULL)
	{
		fprintf(stderr, "%s No Shared Memory\n", fn);
		return -1;
	}

	if (q_vreceive(memoryMap->qid, &msg, sizeof(msg), pid, 0, &len) < 0)
	{
		fprintf(stderr, "%s q_vreceive failed!!!\n", fn);
		return -1;
	}

	return 0;
}

MemoryMap *
SHMAPP_Init(key_t key, void **addr, int size, int msegs, int mpages, int segaddrtype)
{
	char fn[] = "SHMAPP_Init():";
	MemoryMap *map;
	int needed, i;
	int shmid;

	needed = sizeof(MemoryMap) + msegs*sizeof(SegMap);

	/* Do a sanity check on size */
	if (size < needed)
	{
		printf("size (%d) for Map is NOT ENOUGH need (%d)\n",
			size, needed);
		return NULL;
	}

	shmid = SHM_AttachSegment(key, size, addr, DEFAULT_SHMFLAGS|IPC_CREAT);
	if (shmid < 0)
	{
		return NULL;
	}

	map = *addr;
	
	memset(map, 0, needed);
	
	map->len = needed;
	map->msegs = msegs;
	map->mpages = mpages;
	/*	map->segs = ( SegMap * )(map+1); */

	/* Fill up the config for this segment that we have used up */
	/* Do not use first segment for allocations, it only 
	 * contains addresses of other segments.
	 */

	map->segs[0].segSize = size;
	map->segs[0].segFlags = DEFAULT_SHMFLAGS;
	map->segs[0].segKey = key;
	map->segs[0].shmId = shmid;
	map->segs[0].mapAddr = *addr;

	/* Round size used to the next page size */
	map->segs[0].sizeUsed = size;

	/* Initialize the semaphore */
	if (sm_create(key, 1, 0, &map->semid) < 0)
	{
		printf("%s sm_create failed\n", fn); fflush(stdout);
		syslog(LOG_ALERT|LOG_LOCAL1, 
			"%s sm_create failed\n", fn);
		return NULL;
	}

	/* Initialize the queue for mutex locks  - linux*/
	if (q_vcreate(key, 0, 1000, 4, &map->qid) < 0)
	{
		printf("%s q_vcreate failed\n", fn); fflush(stdout);
		syslog(LOG_ALERT|LOG_LOCAL1, 
			"%s q_vcreate failed\n", fn);
		return NULL;
	}

	/* CReate the database locks */
	DbCreateLocks();

	memcpy(map->sizes, sizesLocal, NSIZES*sizeof(sizesLocal[0]));
	sizes = map->sizes;
	memoryMap = map;

	for (i= 1; i < map->msegs; i++)
	{
		map->segs[i].segKey = IPC_PRIVATE;
		map->segs[i].shmId = -1;
		if (segaddrtype == 0)
		{
			map->segs[i].mapAddr = (void *)0;
		}
		else
		{
			map->segs[i].mapAddr =
			(void *)(((long)((char *)*addr+i*size+PAGE_SIZE))&PAGE_MASK); 
		}
		map->segs[i].segSize = size;
		map->segs[i].segFlags = DEFAULT_SHMFLAGS;
		map->segs[i].sizeUsed = 0;
	}

	return map;
}

/* Use the map in the specified segment, to attach the whole memory */
MemoryMap *
SHMAPP_UseMap(key_t key, void **addr, int size, int *nsegs, int *npages)
{
	char fn[] = "SHMAPP_UseMap():";
	MemoryMap *map;
	SegMap *seg;
	int needed, i, j;
	int shmid;

	shmid = SHM_AttachSegment(key, size, addr, DEFAULT_SHMFLAGS);
	if (shmid < 0)
	{
		return NULL;
	}
	map = *addr;
	
	// Check to see if the map is ready
	if (checkMapReady && (map->ready == 0))
	{
		printf("shared memory not ready yet\n");
		// The Map is not ready yet
		if (shmdt((void *)map) != 0)
		{
			perror("shmdt");
		}
		
		return NULL;
	}

	*nsegs = map->nsegs;
	*npages = map->npages;
	needed = sizeof(MemoryMap) + map->msegs*sizeof(SegMap);

	sizes = map->sizes;
	memoryMap = map;

	/* Walk over the map and do the shmat */

	for (i=1; i<map->nsegs; i++)
	{
		seg = &map->segs[i];
		if (seg->shmId >= 0)
		{
			if (shmat(seg->shmId, seg->mapAddr, seg->segFlags) == (caddr_t)-1)
			{
				printf("%d. %s Unable to Attach to segment %d at address %p flags are 0x%x pid is %lu errno is %d\n",
					i, fn, seg->shmId, seg->mapAddr, seg->segFlags, ULONG_FMT(getpid()), errno); fflush(stdout);
				syslog(LOG_ALERT|LOG_LOCAL1, 
					"%s Unable to Attach to segment %d at address %p flags are 0x%x pid is %lu errno is %d\n",
					fn, seg->shmId, seg->mapAddr, seg->segFlags, ULONG_FMT(getpid()), errno);
				/* We must handle this error condition, by
				 * detaching from all the other segments
				 */
				for (j=1; j<i; j++)
				{
					seg = &map->segs[j];
					if (seg->shmId >= 0)
					{
						if (shmdt(seg->mapAddr) != 0)
						{
				syslog(LOG_ALERT|LOG_LOCAL1, 
					"%s Unable to detach from segment %d at address %p pid is %d errno is %d\n",
					fn, seg->shmId, seg->mapAddr, getpid(), errno);
						}
					}
				}

				return NULL;
			}
		}
	}

	/* Initialize the free pages in the library */
	ShmPages = map->pmap.freePages;

	return map;
}

int
SHMAPP_AllocMap(MemoryMap *map)
{
	int i, n = 0, m = 0;
	SegMap *seg;
	
	for (i= 0; (i < map->msegs) && (n < map->mpages); i++)
	{
		seg = &map->segs[i];

		m ++;

		if (seg->shmId != -1)
		{
			/* Check to see if the segment is completely used up */
			if (seg->sizeUsed < seg->segSize)
			{
				/* Paginate */
				n+= SHM_PaginateSegment(((char *)seg->mapAddr+seg->sizeUsed), 
					seg->segSize-seg->sizeUsed, &map->pmap.freePages[0], 
					PAGE_SIZE);
			}
			continue;
		}

		if ((seg->shmId = 
			SHM_AttachSegment(seg->segKey, 
				(seg->segSize?seg->segSize:DEFAULT_SEG_SIZE), 
				&seg->mapAddr, seg->segFlags|IPC_CREAT)) < 0)
		{
			printf("could not allocate shared segment\n");
			continue;
		}

		seg->sizeUsed = seg->segSize;

		n += SHM_PaginateSegment(seg->mapAddr, seg->segSize, 
			&map->pmap.freePages[0], PAGE_SIZE);
	}

	if (n < map->npages)
	{
		printf("Increase the number of segments. Fell short by %d pages\n",
			map->mpages-n);
	}
	
	/* Initialize the free pages in the library */
	ShmPages = map->pmap.freePages;

	map->pmap.nPages[0] = n;
	map->npages = n;
	map->nsegs = m;

	return n;
}

/* Note that map cannot be part of shared memory when this is called.
 * Copy map into a local structure.
 */
int
SHMAPP_ReleaseMap(MemoryMap *map)
{
	int i;
	SegMap *seg;
	struct shmid_ds shmidDs;

	/* Walk over the map and do the shctls */

	/* Free the lock */
	sm_delete(map->semid);
	q_vdelete(map->qid);

	for (i=0; i<map->nsegs; i++)
	{
		seg = &map->segs[i];
		shmctl(seg->shmId, IPC_RMID, &shmidDs);
	}

	/* Delete the semaphores associated w/ the databases */
	DbDeleteLocks();
	return(0);
}

int
SHMAPP_DetachMap(MemoryMap *map)
{
	int i;
	SegMap *seg;

	/* Walk over the map and do the shctls */

	for (i=1; i<map->nsegs; i++)
	{
		seg = &map->segs[i];
		if (seg->shmId >= 0)
		{
			if (shmdt(seg->mapAddr) != 0)
			{
				perror("shmdt");
			}
		}
	}
	if (shmdt ((void *)map) != 0) 
	{
		perror ("shmdt");
	}
	return( 0 );
}

int
SHMAPP_AttachMap(MemoryMap *map)
{
	char fn[] = "SHMAPP_AttachMap():";
	int i;
	SegMap *seg;

	/* Walk over the map and do the shctls */

	for (i=1; i<map->nsegs; i++)
	{
		seg = &map->segs[i];
		if (seg->shmId >= 0)
		{
			if (shmat(seg->shmId, seg->mapAddr, seg->segFlags) == (caddr_t)-1)
			{
				printf("%s Unable to Attach to segment %d at address %p pid is %lu errno is %d\n",
					fn, seg->shmId, seg->mapAddr, ULONG_FMT(getpid()), errno); fflush(stdout);
				syslog(LOG_ALERT|LOG_LOCAL1, 
					"%s Unable to Attach to segment %d at address %p pid is %lu errno is %d\n",
					fn, seg->shmId, seg->mapAddr, ULONG_FMT(getpid()), errno);
			}
		}
	}

	return( 0 );
}

void
PrintBlock(char *ptr, int sz, int format)
{
	int i;

	printf("block %p size %d:\n", ptr, sz);
	for (i=0; i<sz; i++)
	{
		if (i%8 == 0) printf("\n");
		printf("%x ", *(char *)(ptr+i));
	}	
	printf("\n");
}

void
PrintBlocks(MemoryMap *map, int order)
{
	struct SizeDescriptor *bucket = NULL;
	struct PageDescriptor *p;
	struct BlockHeader *bptr;
	int j = 0, i, sz, n = 0;
	SegMap *seg;
	void *ptr;
	
	for (j= 1; (j < map->nsegs) && (n < map->npages); j++)
	{
		seg = &map->segs[j];

		for (ptr = seg->mapAddr; ptr < seg->mapAddr+seg->sizeUsed; 
			ptr += PAGE_SIZE )
		{
			p = (PageDescriptor *)ptr;

			if (p->order != order)
			{
				continue;
			}

			n++;
			printf("\tPage %d order %d nFree %d\n", 
				n, p->order, p->nFree);
		
			// print the allocated portion of page p
			bucket = sizes + p->order;
			i = bucket->nBlocks - 1;
			bptr = BH(p + 1);
			sz = BLOCKSIZE(p->order);
			while (i > 0) {
				i--;
				if (bptr->bhFlags != MF_FREE)
				{
					PrintBlock((char *)(bptr+1), sz-sizeof(struct BlockHeader), 0);
				}
				bptr = BH(((long) bptr) + sz);
			}
			/* Last block: */
			if (bptr->bhFlags != MF_FREE)
			{
				PrintBlock((char *)(bptr+1), sz, 0);
			}
		}
	}
}

int
PrintMap(MemoryMap *map)
{
	struct SizeDescriptor *bucket = sizes;
	struct PageDescriptor *p;
	int i, j = 0, nbytes = 0, nunused = 0, total, totalPages = 0;

	printf("size of map %d\n", map->len);
	printf("no of segs %d\n", map->nsegs);
	printf("no of pages %d\n", map->npages);
	printf("no of pages left %d\n", map->pmap.nPages[0]);
	printf("Trying shared memory semaphore:");
	
	if (sm_p(map->semid, 0, 0) < 0)
	{
		printf("error.\n");
	}
	else
	{
		sm_v(map->semid);
		printf("success.\n");
	}
	printf("Page Size %d Kbytes\n", PAGE_SIZE/1024);

	/* Print the page map... */
	for (i=0; i<NSIZES-1; i++)
	{
#if 0
		printf("block size %d\n", blockSize[i]);	
		printf("\tnpages %d \n", bucket->nPages);
		printf("\tnBytesMalloced %d\n", bucket->nBytesMalloced);
		printf("\tnFrees %d\n", bucket->nFrees);
		printf("\tnMallocs %d\n", bucket->nMallocs);
		printf("\tnBlocks %d\n", bucket->nBlocks);
#endif
		nunused += ((bucket->nPages * PAGE_SIZE) - bucket->nBytesMalloced);
		totalPages += bucket->nPages;

		printf("\tblock %d, %d pages used, %d bytes unused, %d/%d bytes/blocks used, %d frees, %d mallocs\n",
			blockSize[i], bucket->nPages, 
			(bucket->nPages * PAGE_SIZE) - bucket->nBytesMalloced,
			bucket->nBytesMalloced, bucket->nBytesMalloced/blockSize[i],
			bucket->nFrees, bucket->nMallocs);
		p = bucket->firstFree;
		while (p)
		{
			j++;
#if 0
			printf("\tPage %d order %d nFree %d\n", 
				j, p->order, p->nFree);
#endif
			p = p->next;
		}
		bucket ++;
		nbytes += bucket->nBytesMalloced;
	}
	
	total = map->segs[0].segSize*map->msegs;
	printf("total memory: %d Mbytes\n", total/(1024*1024));
	printf("total   used: %d Mbytes, %2.2f%% of total\n", nbytes/(1024*1024), ((float)nbytes*100)/total);
	printf("total formatted memory unused: %d Mbytes, %2.2f%% of allocated pages\n", nunused/(1024*1024), ((float)nunused*100)/(totalPages*PAGE_SIZE));

	return( 0 );
}

/* Auxiliary functions */

int
WriteBuffToFile(char *file, char *buff, int len)
{
	int fd;
	char *map;

	fd = open(file, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXO|S_IRWXG);

	if (fd < 0)
	{
		return -1;
	}

	map = (char *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	memcpy(map, buff, len);

	munmap(map, len);

	close(fd);
	return( 0 );
}

int
ReadBuffFromFile(char *file, char *buff, int len)
{
	int fd;
	char *map;

	fd = open(file, O_RDWR|O_CREAT|O_SYNC, S_IRWXU|S_IRWXO|S_IRWXG);

	if (fd < 0)
	{
		return -1;
	}

	map = (char *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	memcpy(buff, map, len);

	munmap(map, len);

	close(fd);
	return( 0 );
}

