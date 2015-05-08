
#ifndef _netloop_h_
#define _netloop_h_


typedef enum
{
	FD_READ,
	FD_WRITE,
	FD_RW,
} FD_MODE;


typedef	int	(*NetFn) (int fd, FD_MODE mode, void * cbdata);



int netInit (void);
int netDestroy (void);
void netMainLoop (void);
int netAddFd (int fd, int rw, NetFn readfn, NetFn writefn, void * cbdata, void (*destroy)(void *));
int netSetupFds (void);
int netSetupTimeout (struct timeval * tout, int selimsecs);
int netProcessFds (void);
int netProcessTimeout (void);

#endif	/* _netloop_h_ */
