#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <limits.h>
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "license.h"
#include "licenseIf.h"
#include "poll.h"

#include "spversion.h"

#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"
#include "protos.h"
#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "sconfig.h"
#include <malloc.h>

NetFds 		lsnetfds;
Config 		localConfig;
char 		pidfile[256];
int 		shmId;
MemoryMap 	*map;
LsMemStruct 	*lsMem = 0;

char 		config_file[60] = CONFIG_FILENAME; 	

int ncalls = 0;
extern int xthreads;
extern int nthreads;
