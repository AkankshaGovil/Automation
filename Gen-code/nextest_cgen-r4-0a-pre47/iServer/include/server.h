#ifndef _server_h_
#define _server_h_

#include "list.h"
#include "ipc.h"

typedef struct
{
	int fd;
	unsigned long ipAddress;
} SNode;

extern SNode	*Parent;
extern List 	DirectChildren;
extern List	OtherChildren;
extern List	ConfigSNodes;
int htonPkt(int type, Pkt *pktP);
int daemonize (void);
#endif /* _server_h_ */
