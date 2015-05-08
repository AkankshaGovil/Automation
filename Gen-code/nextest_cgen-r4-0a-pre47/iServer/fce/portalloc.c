/**
 * contains methods to do port allocation. Allocates even numbered ports within the
 * specified range. This does not do any locking to prevent concurrent accesses. The
 * caller need to take care of it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bits.h"
#include "srvrlog.h"
#include "portalloc.h"

#define RANGE(low, high)	((high - low)/2)
#define FREEPORT ((unsigned char) 0)

#define GET_RX_CNT(c) 		((c) & 0x01)   
#define GET_TX_CNT(c) 		((c) >> 1)   
#define SET_RX_CNT(c, v) 	((c) = (((c) & ~(0x01)) | (0x01 & (v))))   
#define SET_TX_CNT(c, v) 	((c) = (((c) & 0x01) | ((v) << 1)))   
#define MAX_TX_CNT			(GET_TX_CNT((unsigned char)~(0x01)))

const char *
dirxn2str(dirxn d) {
	switch (d) {
		case rx:
			return "rx";
			break;
		case tx:
			return "tx";
			break;
		default:
			return "unknown";
	};
}

/** 
 * cyclical increment of index within the given range
 */
#define incrementIndex(ptr) ( (ptr)->curIndex = ( ( (ptr)->curIndex + 1) % ( (ptr)->size)))

/**
 * initialize a port allocation scheme for the given range
 *
 * @param low the lower port number of the range
 * @param low the higher port number of the range
 *
 * @return the pointer to the PortAlloc structure to be used in allocPort and freePort calls
 */
PortAlloc*
initPortAllocation (int low, int high)
{
	PortAlloc *ptr;
	int lowRange = (low % 2)?low+1:low;			 // eliminate any odd numbers
	int highRange = (high % 2)?high-1:high;  // eliminate any odd numbers
	int size;

	/* validate range */
	if (lowRange >= highRange)
	{
		NETERROR(MFCE, ("initPortAllocation: invalid range %d - %d\n", low, high));
		return NULL;
	}

	/* size of the bit array for the given range */
	/* one byte per port are getting allocated. 1 for ingress 7 for egress */
	size = (RANGE(lowRange, highRange) + 1);

	/* allocate the PortAlloc struct */
	ptr = (PortAlloc *)malloc(sizeof(PortAlloc));
	if (ptr == NULL)
	{
		NETERROR(MFCE, ("initPortAllocation: cannot allocate PortAlloc\n"));
		return NULL;
	}

	ptr->bArray = (char *)malloc(size);
	if (ptr->bArray == NULL)
	{
		free(ptr);
		NETERROR(MFCE, ("initPortAllocation: cannot allocate bitArray\n"));
		return NULL;
	}
	
	memset(ptr->bArray, 0, size);
	ptr->size = size;
	ptr->inited = 1;
	ptr->low = lowRange;
	ptr->high = highRange;
	ptr->curIndex = 0;

	return ptr;
}


/**
 * frees resources held by the PortAlloc structure
 */
void
clearPortAllocation (PortAlloc *ptr)
{
	if (!ptr->inited)
	{
		NETERROR(MFCE, ("clearPortAllocation: attempt to clear port allocation before initing\n"));
		return;
	}

	ptr->inited = 0;
	free(ptr->bArray);
	ptr->bArray = (char *)0xDEADBEEF;
	ptr->size = 0;
	ptr->low = 0;
	ptr->high = 0;
	ptr->curIndex = 0;
	free(ptr);
}

/**
 * makes the index pointer point to the next available port bit, returns -1 if all ports
 * in the range have been allocated
 */
static int
nextIndex (PortAlloc *ptr)
{
	int startIndex = ptr->curIndex;

	do
	{
		if (ptr->bArray[ptr->curIndex] == FREEPORT)
			return ptr->curIndex;
		incrementIndex(ptr);
	} while (ptr->curIndex != startIndex);

	return -1;
}


/**
 * returns the next available port in the range, returns -1 if no ports are available
 * if reqport is not 0 then assigns the requested port
 */
int
allocPort (PortAlloc *ptr, int reqport, dirxn d)
{
	int port = -1;
	int count, index = 0;

	NETDEBUG(MFCE, NETLOG_DEBUG4, ("allocPort: request to allocate port %d for %s\n", reqport, dirxn2str(d)));

	if (!ptr->inited)
	{
		NETERROR(MFCE, ("allocPort: trying to allocate port before initialization\n"));
		return -1;
	}

	if (reqport) {
		index = (reqport - ptr->low)/2;
	}
	else {
		if ((index = nextIndex(ptr)) == -1) {
			NETERROR(MFCE, ("allocPort: cannot allocate port, no more ports available in the range %d - %d\n", ptr->low, ptr->high));
			return -1;	// no ports available in the port range
		}
		
		incrementIndex(ptr);	/* get ready for the next request */

		reqport = (index * 2) + ptr->low;
	}

	/* set the current port as used */
	if (d == rx) {
		if (GET_RX_CNT(ptr->bArray[index])) {
			/* port is already allocated */
			NETERROR(MFCE, ("allocPort: can not allocate rx port on %d.\n", reqport));
			return port;
		}
		SET_RX_CNT(ptr->bArray[index], 1);
	}
	else if (d == tx) {	
		count = GET_TX_CNT(ptr->bArray[index]);
		count++;
		if (count <= MAX_TX_CNT) {
			SET_TX_CNT(ptr->bArray[index], count);
		}
		else {
			NETERROR(MFCE, ("allocPort: can not allocate %d tx ports on %d.\n", count, reqport));
			return port;	// no ports available in the port range
		}
	}
	else {
		NETERROR(MFCE, ("allocPort: can not allocate port for %s(%d).\n", dirxn2str(d), d));
		return port;
	}

	port = reqport;

	NETDEBUG(MFCE, NETLOG_DEBUG4, ("allocPort: allocating port %d for %s\n", port, dirxn2str(d)));
	return port;
}

/**
 * frees the given port back to the allocation pool
 */
void
freePort (PortAlloc *ptr, int port, dirxn d)
{
	int index; 
	int count;

	NETDEBUG(MFCE, NETLOG_DEBUG4, ("freePort: freeing port %d for %s\n", port, dirxn2str(d)));

	if (port < ptr->low || port > ptr->high)
	{
		NETERROR(MFCE, ("freePort: attempt to free port %d outside of range %d - %d\n", port, ptr->low, ptr->high));
		return;
	}
	else if (port%2)
	{
		NETERROR(MFCE, ("freePort: attempting to free odd port %d, freeing %d instead\n", port, (port-1)));
		port--;
	}

	/* (port - ptr->low)/2 is the index for this port */
	index = (port - ptr->low)/2;

	if (d == rx) {
		count = GET_RX_CNT(ptr->bArray[index]);
		count--;
		if (count < 0) {
			NETERROR(MFCE, ("freePort: attempt to free port %d for %s, which is already free\n", port, dirxn2str(d)));
		}
		else {
			SET_RX_CNT(ptr->bArray[index], 0);
		}
	}
	else if (d == tx) {	
		count = GET_TX_CNT(ptr->bArray[index]);
		count--;
		if (count < 0) {
			NETERROR(MFCE, ("freePort: attempt to free port %d for %s, which is already free\n", port, dirxn2str(d)));
		}
		else {
			SET_TX_CNT(ptr->bArray[index], count);
		}
	}
	else {
		NETERROR(MFCE, ("freePort: attempt to free port for %s(%d).\n", dirxn2str(d), d));
		return;
	}
}


#ifdef TEST_PORT_ALLOC
/* These two methods were used for testing the port allocation scheme */

void
printPorts (PortAlloc *ptr)
{
	int i;
	int range = RANGE(ptr->low, ptr->high);

	printf("rx:tx\t");
	for (i = 0; i <= range; i++) {
		printf("%d:%d\t", GET_RX_CNT(ptr->bArray[i]), GET_TX_CNT(ptr->bArray[i]));
		if (i != 0 && i%8 == 0)
			printf("\n");
	}
	printf("\n");
}


int
main (int argc, char **argv)
{
	int times = (argc > 1)?atoi(argv[1]):100;
	int i, random, port=0, index=0, numPorts = 0;
	struct timeval tv;
	dirxn d;
	PortAlloc *pa;
	int *portList;

	gettimeofday(&tv, NULL);
	srand48(tv.tv_sec);

	portList = calloc(times, sizeof(int));

	NetLogInit();
	NetLogOpen((struct sockaddr_in *)NULL, 0, NETLOG_TERMINAL);
	NETLOG_SETLEVEL(MFCE, NETLOG_DEBUG4);
	NETLOG_SETLEVELE(MFCE, NETLOG_DEBUGMASK|NETLOG_ERRORMASK);

	pa = initPortAllocation(2, 10);

	for (i = 0; i < times; i++) {
		random = (rand() % 100);
		d = ((rand() % 100) < 40) ? rx : tx ; 
		if (numPorts)
			index = rand() % numPorts;

		if (random < 70) {
			if ((random < 50) || (numPorts == 0)) {
				// get a new port
				port = 0;
			}
			else if (random < 70) {
				// choose an allocated port
				port = portList[index];	
			}

//			printf("allocating: port %d for %s\n", port, dirxn2str(d));
			if ((port = allocPort(pa, port, d)) > 0) {
				portList[numPorts] = port;
				numPorts++;
			}
//			else {
//				printf("allocPort failed - %s port %d\n", dirxn2str(d), port);
//			}
		} else if (numPorts) {
			// free this port
//			printf("freeing: port %d for %s\n", portList[index], dirxn2str(d));
			freePort(pa, portList[index], d);
		}

		printPorts(pa);
	}
	
	return 0;
}
#endif /* TEST_PORT_ALLOC */
