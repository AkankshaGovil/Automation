#include <sys/types.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>

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
#include <sched.h>
#include "license.h"
#include "licenseIf.h"
#include "poll.h"

#include "gis.h"
#include "ls.h"
#include "common.h"
#include "tsm.h"

#define SERVER_DBG
#undef SERVER_DBG

#define USE_FORK 
#undef USE_FORK

/* Define SERVER_DBG to debug this mammoth... */
#ifndef SERVER_DBG
#define USE_DAEMONIZE
#endif

#include "spversion.h"

#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"

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

#include "gis.h"
#include <malloc.h>

#include "common.h"
#include "thutils.h"
#include "tsm.h"
	

char NexToneShellBanner[] = "\nNextone iServer \n(c) NexTone Communications Inc, 1999-2001\n";

char NextonePrompt[] = "iServer >";
unsigned long histid1, histid2, orig_size, current_size;

#define COMMAND_PREFIX_MAX	24

enum
{
	COMMAND_GET = 1,
	COMMAND_SET,
	COMMAND_RESET,
	COMMAND_BYE, 
	COMMAND_CLI,
	COMMAND_MEMSTART,
	COMMAND_MEMSTOP,

	COMMAND_NONE,
}; 

typedef struct
{
	int type;
	char prefix[256];
} command_type;

command_type
command_types[COMMAND_PREFIX_MAX] = 
{
	{ COMMAND_GET, "get" },
	{ COMMAND_SET, "set" },
	{ COMMAND_RESET, "reset" },
	{ COMMAND_BYE, "bye" },
	{ COMMAND_BYE, "quit" },
	{ COMMAND_BYE, "done" },
	{ COMMAND_BYE, "exit" },
	{ COMMAND_CLI, "cli" },
	{ COMMAND_MEMSTART, "mem_start" },
	{ COMMAND_MEMSTOP, "mem_stop" },
};

char 
ourin[254] = {0}, ourout[254] = {0}, ourerr[254] = {0};
pthread_t       cons_thread;
int ns = -1;
	
void *
CliDReqThread(void *args)
{
	char fn[] = "CliDReqThread():";
	int	ns = *(int *)args;
	FILE *tmpfile;
	struct sched_param param = { 0 };

	// decrease priority
	param.sched_priority=40;
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);

	// Make 0, 1 point the new socket, ns. This will close
	// 0, 1 if necessary and reopen it.
	dup2(ns, 0);
	dup2(ns, 1);
//	dup2(ns, 2);

	// Open a file so that we can do a setbuffer on it
	tmpfile = fdopen(ns, "ab+");
	if (tmpfile == NULL)
	{
		write(ns, "error ", 6);
		write(ns, (char *)&errno, 4);
		goto _return;
	}

	setbuffer(tmpfile, NULL, 0);

	/* Call the CliDMain function here */
	CliDMain();
	
_return:

	// shutdown the new connection and make 0, 1 point to their old values
	shutdown(ns, 2);

	close(ns);	

	ns = -1;

	free(args);
	fclose(tmpfile);

	CFG_ReserveFds();

	printf("CliDReqThread closing\n");

	return((void*) NULL);
}

int
CliDReceiveReq(int csock, FD_MODE rw, void *data)
{
	char fn[] = "CliDReceiveReq():";
	pthread_attr_t  detached;
	int status, *arg;
	struct sockaddr_in sn = { 0 };
	socklen_t foo;

	if (ns > 0)
	{
		/* Close down the previous call */
		shutdown(ns, 0);
	}

	foo = sizeof(sn);

	ns = accept(csock, (struct sockaddr *)&sn, &foo);

	if (ns < 0)
	{
		NETERROR(MINIT, ("%s accept returned error %d\n", fn, errno));
		return -1;
	}
	
	/* Launch a thread which will give the caller a cli */
	pthread_attr_init( &detached );
	pthread_attr_setdetachstate( &detached, PTHREAD_CREATE_DETACHED );
	
	arg = (int *)malloc(sizeof(int));
	if (arg == NULL)
	{
		printf("%s malloc failed error no %d\n", fn, errno);
		return -1;
	}

	*arg = ns;

	status = pthread_create( &cons_thread, &detached,
				CliDReqThread, (void *)arg);

#ifdef use_fun_call
	CliDReqThread((void *)&ns);
#endif

	return(1);
}

int
CliDInit(uint16_t port)
{
	char fn[] = "CliDInit():";
	int i = 1;
	struct sockaddr_in addr = { 0 };
	int s, rc;

	/* Open a socket and bind to it */

	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0)
	{
		NETERROR(MINIT, ("%s socket return error %d\n", fn, errno));
	}

	/* Set some options */
	i = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&i, sizeof(i));

	/* diable nagle */
	i = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));

	/* oob should be inline */
	i = 1;
	setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (void *)&i, sizeof(i));

	/* disable async i/o */
	i = 0;
	ioctl(s, FIOASYNC, (void *)&i);

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(port);

	rc = bind (s, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

	if (rc < 0)
	{
		NETERROR(MINIT, ("%s bind returned error %d\n", fn, errno));
	}

	if (listen(s, 5) < 0)
	{
		NETERROR(MINIT, ("%s listen returned error %d\n", fn, errno));
	}

	NetFdsAdd(&lsnetfds, s, FD_READ, CliDReceiveReq, (NetFn) 0, NULL, NULL);

#ifdef _DMALLOC_
#ifdef NETOID_LINUX
	{
		extern char *dmalloc_logpath;
		dmalloc_logpath = "./malloc.inuse";
	}
#else
	{
		extern char *dmalloc_logpath;
		dmalloc_logpath = "./gis_malloc.inuse";
	}
#endif

#endif /* _DBMALLOC_  */

	return(0);

}

int
CliDMain()
{
	printf("%s\n", NexToneShellBanner);

	PrintCommandPrompt(stdout);

	while (ProcessCommandLine(stdin))
	{
		PrintCommandPrompt(stdout);
	}
	return(0);
}

int
ProcessCommandLine(FILE *in)
{
	int rc;
	char commandbuffer[1024];
	int flag;

	rc = read_command(in, commandbuffer, 1024);

	if (rc)
	{
		rc = handle_command(commandbuffer);
	}

	return rc;
}

int
PrintCommandPrompt(FILE *out)
{
	int error, len = sizeof(int);

	/* check error on socket */
	if (getsockopt(ns, SOL_SOCKET, SO_ERROR,
		(char *)&error, &len) < 0)
	{
		printf("error on socket. error no %d\n", error);
	}

	fprintf(out, "%s", NextonePrompt);
	fflush(out);
	return(0);
}

/* REserve 0,1,2 to be used by the clid */
int
CFG_ReserveFds(void)
{
	FILE *tmpfile;

	// flush stdin, if needed and make it point to /dev/null
//	if (tmpfile = freopen("/dev/null", "r", stdin))
//	{
//		setbuffer(tmpfile, NULL, 0);
//		//dup2(fileno(tmpfile), 0);
//	}

	// flush stdout, and make it point to iserverout.log
	if (tmpfile = freopen("/var/log/iserverout.log", "ab+", stdout))
	{
		setbuffer(tmpfile, NULL, 0);
		//dup2(fileno(tmpfile), 1);
	}

	// leave stderr going to /var/log/iservererr.log
//	if (tmpfile = freopen("/var/log/iservererr.log", "ab+", stderr))
//	{
//		setbuffer(tmpfile, NULL, 0);
//		dup2(fileno(tmpfile), 2);
//	}
	return(0);
}

int
read_command(FILE *in, char *buffer, int buflen)
{
	char fn[] = "read_command():";
	int error;
	char *retval;

	retval = fgets(buffer, buflen, in);

	if (retval == NULL)
	{
		/* check for errors */
		if (errno != 0)
		{
			fprintf(stderr, "%s encountered error %d\n", fn, errno);
			return 0;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}

int
handle_command(char *buffer)
{
	int rc = 1;
	char *op;
	int cmdtype;
	static unsigned long mark;

	if (ExtractArg(buffer, &op, &buffer) < 0)
	{
		return -1;
	}

	cmdtype = type_command(op);

	switch (cmdtype)
	{
		case COMMAND_GET:
			handle_get_command(buffer);
			break;
		case COMMAND_SET:
			handle_set_command(buffer);
			break;
		case COMMAND_RESET:
			handle_reset_command(buffer);
			break;
		case COMMAND_BYE:
			handle_bye_command(buffer);
			rc = 0;
			break;
		case COMMAND_CLI:
			handle_cli_command(buffer);
			break;
		case COMMAND_MEMSTART:
#ifdef _DMALLOC_
		dmalloc_message("starting new log");
		mark = dmalloc_mark();
#elif _DBMALLOC_ 
			if(malloc_chain_check(0)!=0)
			{
				int fd;
                fd = open("/usr/local/nextone/logs/malloc.inuse", O_CREAT|O_RDWR
);
                malloc_dump(fd );
                close(fd);

			}
			orig_size = malloc_inuse(&histid1);
#endif
            break;	
		case COMMAND_MEMSTOP:
#ifdef _DMALLOC_
		dmalloc_log_changed(mark, 1, 0, 1);
		dmalloc_message("end of log");
#elif _DBMALLOC_ 
			current_size = malloc_inuse(&histid2);

            if(current_size != orig_size)
            {
				int fd;

                fd = open("/usr/local/nextone/logs/malloc.inuse", O_CREAT|O_RDWR
);
                malloc_list(fd, histid1, histid2);
                close(fd);
            }
#endif
            break;

		case COMMAND_NONE:
		default:
			break;
	}

	return rc;
}	

int
handle_cli_command(char *buffer)
{
	// First we form arguments out of this buffer
	int argc = 0;
	char* argv[100] = { 0 };

	while (ExtractArg(buffer, &argv[argc++], &buffer) >= 0);

#if 0
	ProcessCommand(argc, argv, 0x0002);
#endif

	return(0);
}

int
handle_reset_command(char *cmd)
{
	char *param;
	char fn[] = "handle_reset_command():";

	if (ExtractArg(cmd, &param, &cmd) < 0)
	{
		return -1;
	}

	if (!strcmp(param, "stats"))
	{
		return ThreadStatsReset();
	}

	if ( param != (char*) NULL )
	{
		NETERROR(MINIT, ("%s invalid command %s specified\n", fn, param ));
	}
	else
	{
		NETERROR(MINIT, ("%s no command specified\n", fn ));
	}

	return(-1);
}

int
handle_set_command(char *cmd)
{
#if 0
	ConfigCommandStruct *cmdstruct;
	ConfigCommandFn fn;
	char *param;

	if (ExtractArg(cmd, &param, &cmd) < 0)
	{
		return -1;
	}

	/* lookup the command */
	cmdstruct = lookupconfigcmd(param);

	if (cmdstruct == NULL)
	{
		return -1;
	}

	fn = cmdstruct->set_func;
	
	if (fn == NULL)
	{
		return -1;
	}

	/* call the function */
	fn(param, cmdstruct->cmd, cmd, CONFIG_VERBOSE, &config_change[0]);
#endif
	return(0);
}

int
handle_timers_command(char *cmd)
{
	struct timeval now, tout;
	int msec = -1;
	struct Timer *t;

	timersPrint(&localConfig.timerPrivate);

	gettimeofday(&now, NULL);

	printf("Time now [%ld,%ld]\n", now.tv_sec, now.tv_usec);

	if (localConfig.timerPrivate.timerCache->minData)
	{
		t = (struct Timer*)localConfig.timerPrivate.timerCache->minData;
		printf("min timer %s[%d]cb(%p,%p)expire[%ld,%ld]value[%ld,%ld]int[%ld,%ld] \n", t->name, t->ntimes,	
					t->cb, t->data, t->expire.tv_sec, t->expire.tv_usec,
					t->itime.it_value.tv_sec, t->itime.it_value.tv_usec,
					t->itime.it_interval.tv_sec, t->itime.it_interval.tv_usec);
		tout.tv_sec = t->expire.tv_sec - now.tv_sec;
		tout.tv_usec = t->expire.tv_usec - now.tv_usec;
		LusAdjustTimeout(&tout, &msec);
		printf("time remaining %d msecs\n", msec);
	}
	return(0);
}

int
handle_stats_command(char *cmd)
{
	// print the number of pending items for thread pool
	ThreadStats();

	// print no of TSM entries
	TimerStats(&localConfig.timerPrivate);
	return(0);
}

int
handle_tsm_command(char *cmd)
{
	TsmLogCache(0, 0, 0);
	return(0);
}

int
handle_get_command(char *cmd)
{
	char *param;
	char fn[] = "handle_get_command():";

	if (ExtractArg(cmd, &param, &cmd) < 0)
	{
		return -1;
	}

	if (!strcmp(param, "timers"))
	{
		return handle_timers_command(cmd);
	}

	if (!strcmp(param, "stats"))
	{
		return handle_stats_command(cmd);
	}

	if (!strcmp(param, "tsm"))
	{
		return handle_tsm_command(cmd);
	}

	if ( param != (char*) NULL )
	{
		NETERROR(MINIT, ("%s invalid command %s specified\n", fn, param ));
	}
	else
	{
		NETERROR(MINIT, ("%s no command specified\n", fn ));
	}

	return(-1);
}

int
handle_bye_command(char *cmd)
{
	/* Do syslog ?? */

	printf("NexTone Rules!!\n");
	return(0);
}

int
type_command(char *cmd)
{
	int i;

	for (i=0; i<COMMAND_PREFIX_MAX; i++)
	{
		if (!strcmp(cmd, command_types[i].prefix))
		{
			return command_types[i].type;
		}
	}

	return COMMAND_NONE;
}

