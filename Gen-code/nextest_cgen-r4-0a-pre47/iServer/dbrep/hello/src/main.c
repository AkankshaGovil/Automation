#include "unp.h"
#include "rs.h"
#include "fdsets.h"
#include "thutils.h"
#include "timer.h"
#include "hello.h"
#include "srvrlog.h"
#include "list.h"
#include "queue.h"
#include "ipcutils.h"
#include "ipckey.h"
#include <net/if.h>

#include "dummy.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

//pthread_t	timerThread = (pthread_t)-1;
//NetFds		Hellotimerfds;
//int 		rnotify = 0, wnotify = 0;
//int 		notifyPipe[2];
//HelloConfp	hcp;

//extern int RunHelloProtocol(int argc, char **argv);
sharvar host_dbrev = {PTHREAD_MUTEX_INITIALIZER, 0};

/* typedef void *(*PFVP)(void *); */

int
main(int argc, char **argv)
{
	NetLogInit();
	NetSyslogOpen("hello", NETLOG_TERMINAL);
	NETLOG_SETLEVEL(MRSD, NETLOG_DEBUG2);
//	NETLOG_SETLEVEL(MTMR, NETLOG_DEBUG4);
	NETLOG_SETLEVELE(MRSD, NETLOG_DEBUGMASK);
	NETLOG_SETLEVELE(MRSD, NETLOG_ERRORMASK);

	ThreadLaunch((PFVP)RunHelloProtocol, NULL, 1);
	for(;;) {
	}
	
	return(0);
}
