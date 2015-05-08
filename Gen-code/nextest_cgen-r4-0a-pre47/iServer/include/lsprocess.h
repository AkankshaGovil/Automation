#ifndef _ls_process_h_
#define _ls_process_h_

#include "fdsets.h"

int
HandleCentrex(int fd, FD_MODE rw, void *data);

int
RedundsPacketReceive(int csock, FD_MODE rw, void *data);

int RedundsConnReceive(int csock, FD_MODE rw, void *data);

int ProcessRegister (int sockfd, Pkt * data_pkt,  Pkt * reply_pkt,
		     void *opaque, int opaquelen, int (*writecb)());
int ProcessFind (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		     void *opaque, int opaquelen, int (*writecb)());     
int ProcessRedirect (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		     void *opaque, int opaquelen, int (*writecb)());
int ProcessDND (int sockfd, Pkt * data_pkt, Pkt * reply_pkt,
		void *opaque, int opaquelen, int (*writecb)());
int ProcessStatus (int sockfd, Pkt * data_pkt,  Pkt * reply_pkt,
		   void *opaque, int opaquelen, int (*writecb)());
int ProcessUnregister (int sockfd, Pkt * data_pkt,  Pkt * reply_pkt,
		       void *opaque, int opaquelen, int (*writecb)());

int RedundsUpdateVpnDelete(int fd, VpnEntry *vpnEntry);
int RedundsUpdateVpnGDelete(int fd, VpnGroupEntry *vpnGroupEntry);
int RedundsUpdateNetoidDelete(int fd, InfoEntry *entry);
int RedundsPushVpnEntry(int fd, VpnEntry *vpnEntry);
int RedundsPushVpnGEntry(int fd, VpnGroupEntry *vpnGroupEntry);
int RedundsPushNetoidEntry(int fd, InfoEntry *entry);
int HandleRemoteClient(int fd, FD_MODE rw, void *data);

extern int isLusReq, isVpnsReq;

extern NetFds 		lsnetfds;
extern NetFds		*lsh323fds;
extern NetFds 		lstimerfds;

// Will be the main thread id
extern pthread_t	lsThread, timerThread, *h323Threads;
extern int *h323inPool, *h323stackinClass;
extern int h323initsem, h323waitsem;
extern int maxfds;

int IpcH323MainLoop (void *);

void *IpcHandleTimers(void *);
int IServerH323Notify(void *);
int HandleH323Notify(int fd, FD_MODE rw, void *data);

int IServerNotify(void *);
int HandleNotify(int fd, FD_MODE rw, void *data);

extern int iserverPrimary;
#define IServerIsPrimary()		(iserverPrimary)
#define IServerIsSecondary()	(!iserverPrimary)

#endif	/* _ls_process_h_ */
