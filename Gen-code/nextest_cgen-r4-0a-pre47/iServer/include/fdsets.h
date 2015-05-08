#ifndef _fdsets_h_
#define _fdsets_h_

#include "list.h"
#include "netloopP.h"

int NetFdsInit (NetFds *netfds);

int NetFdsCleanup (NetFds *netfds);

int NetFdsAdd (NetFds *netfds, int fd, int rw, NetFn readfn, 
				NetFn writefn, void * cbdata,
				void (*destroy)(void *));

int NetFdsSetupPoll (NetFds *netfds, int module, int level);

int NetFdsProcessPoll(NetFds *netfds, int module, int level);

int NetFdsRemove (NetFds *netfds, int fd, int rw);

int NetFdsUpdate (NetFds *netfds, int fd, int rw, NetFn fn);

int NetFdsDeactivate(NetFds *netfds, int fd, int rw);

int NetFdsSetup (NetFds *netfds, int module, int level);

int NetFdsProcess(NetFds *netfds, int module, int level);

int NetFdsStats(NetFds *netfds, int module, int level );
int NetFdsSetMax(int n);
int NetFdsSetCbData (NetFds *netfds, int fd, void * cbdata, void (*destroy)(void *));
	
	

#define NOTIFY_READ 	0
#define NOTIFY_WRITE	1

#endif /* _fdsets_h_ */
