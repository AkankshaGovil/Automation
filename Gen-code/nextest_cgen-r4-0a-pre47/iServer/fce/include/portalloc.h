#ifndef __PORTALLOC_H
#define __PORTALLOC_H

typedef struct _PortAlloc {
  int inited;       // allocation structure is intialized
  int low;          // the low port number of the range to be allocated
  int high;         // the high port number of the range to be allocated
  int size;
  unsigned char *bArray;   // the bits that hold the allocation information
  int curIndex;     // current pointer inside the bit array
} PortAlloc;

typedef enum {
	rx = 1,
	tx = 2
} dirxn;

#define allocRxPort(p) 				allocPort((p), 0, rx)
#define allocTxPort(p) 				allocPort((p), 0, tx)
#define allocRxGivenPort(p, port) 	allocPort((p), (port), rx)
#define allocTxGivenPort(p, port) 	allocPort((p), (port), tx)

extern PortAlloc* initPortAllocation (int, int);
extern void clearPortAllocation (PortAlloc*);
extern int allocPort (PortAlloc*, int port, dirxn d);
extern int allocGivenPort (PortAlloc *ptr, int port, dirxn d);
extern void freePort (PortAlloc*, int port, dirxn d);

#endif
