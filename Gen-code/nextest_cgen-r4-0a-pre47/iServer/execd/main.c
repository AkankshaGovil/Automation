#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execd.h"
#include "lsconfig.h"
#include "pids.h"
#include "pmpoll.h"
#include "spversion.h"
#include "server.h"
#include "nxosd.h"
#include "sysutils.h"

#define	MAXFD	64
#define	MAX_EXECD_THREADS	10

#define USE_SYS_POPEN

/* Private Routines */
void 			parse_opt(int argc, char *argv[]);
static	void	SignalInit(void);
static	void	SyncSigHandler(int signo);
static	void	*AsyncSigHandlerThread(void * args);
static	void 	HandleINT(int sig);
static	void 	HandleCHLD(int sig);
static	void 	HandleTERM(int sig);
static	void 	HandleHUP(int sig);
static	int	ConfigureExecD(void);
static	void	*SendPMPoll(void *arg);
static	int 	CmdWorker(q_msg *m);
static	void 	SendResp(q_msg *m, int rc);
static	int 	SendFailMsg(q_msg *m);
static	int 	System(const char *cmdstr, char *buf, int len);

/* Global Variables */
sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

char pname[MAX_LINE];
int  execd_daemonize = 1, execd_conf = 1, debug_level = 0;
char config_file[MAX_LINE] = CONFIG_FILENAME;
char pidfile[MAX_LINE];
LsMemStruct *lsMem;
int	msgqid = -1;
int nthreads = MAX_EXECD_THREADS;
char	pidf[MAX_LINE];
int		execdpid;

int
main(int argc, char *argv[], char *envp[])
{
	int i, poolid, classid;
	ssize_t len;
	key_t	key;
	char *msg;

	//	Parse the options and configure appropriate values. 
	parse_opt(argc, argv);

	//	Read the configuration
	setConfigFile();
	myConfigServerType = CONFIG_SERPLEX_EXECD;
	DoConfig(ConfigureExecD);

	//	Daemonize
	if (execd_daemonize) {
		daemonize();
		(void) freopen( "/dev/null", "r", stdin );
		(void) freopen( "/var/log/iserverout.log", "a", stdout );
		(void) freopen( "/var/log/iservererr.log", "a", stderr ); 
		for (i = 3; i < MAXFD; i++) {
			close(i);	
		}
	}

	//	Start the signal handler thread.
	SignalInit();

	NetSyslogOpen(pname, NETLOG_ASYNC);
	sprintf(pidf, "%s/%s", PIDS_DIRECTORY, EXECD_PID_FILE); 
	execdpid = ReadPid(pidf);
 
	if ( execdpid > 0 ) { 
		if ((kill(execdpid, 0) == 0) || (errno != ESRCH)) { 
			NETERROR(MRSD, ("%s seems to be running already - exiting\n", basename(argv[0])));
			exit(0);
		} 
		else  /* Get rid of the leftover file */ 
       		UnlinkPid(pidf);
    }

	StorePid(pidf);

	NETINFOMSG(MEXECD, ("*** NexTone Cmd Execution Server started ***\n"));

	// 	Start the thread which sends message to pm
	Initpoll(EXECD_ID, SERPLEX_GID);
	Sendpoll(0); 
	ThreadLaunch((PFVP)SendPMPoll, NULL, 1);

	// 	Get the msg_q id. If it does not exist then create it
	if ((key = ftok(ISERVER_FTOK_PATH, ISERVER_EXECD_Q)) < 0) {
		NETERROR(MEXECD, ("ftok: %s\n", strerror(errno)));
		exit(0);
	}
	
	if (q_vget(key, 0, MAX_NUM_MSG, MAX_MSGLEN, &msgqid) < 0) {
		NETERROR(MEXECD, ("q_vget: %s\n", strerror(errno)));
		exit(0);
	}

#ifdef USE_SYS_POPEN	
	sys_utils_init();
#endif

	// Start a threadpool of worker threads
	poolid = ThreadPoolInit("execcmd", nthreads, PTHREAD_SCOPE_PROCESS, -1, 0);
	classid = ThreadAddPoolClass("execcmd", poolid, 0, 0);
	ThreadPoolStart(poolid);

	// msgqid has to be valid if we reached here
	for( ; ; ) {
		if (!(msg = malloc(MAX_MSGLEN))) {
			NETERROR(MEXECD, ("malloc: %s\n", strerror(errno)));
			sleep(5);   // sleep and try again
			continue;
		}
		if (q_vreceive(msgqid, msg, MAX_MSGLEN, SRVR_MSG_TYP, 0, &len) < 0) {
			NETERROR(MEXECD, ("q_vreceive: %s\n", strerror(errno)));
			// Check error types
			if (errno == EIDRM) {
				printf("execd: stop messing with my message queue\n");
				// somebody deleted the queue
				if (q_vget(key, 0, MAX_NUM_MSG, MAX_MSGLEN, &msgqid) < 0) {
					NETERROR(MEXECD, ("q_vget: %s\n", strerror(errno)));
					exit(0);
				}
			}
			free(msg);
			continue;
		}
		if (ThreadDispatch(poolid, classid, (PFVP)CmdWorker, msg, 1, 
			PTHREAD_SCOPE_PROCESS, SCHED_FIFO, 59) < 0) {
			NETERROR(MRSD, ("ThreadDispatch: No free thread\n"));
			// Send response in a new thread
			ThreadLaunch((PFVP)SendFailMsg, msg, 1);
		}
	}
}

int
CmdWorker(q_msg *m)
{
	char *c = &(m->cmd[0]);	
	char s[MAX_MSGLEN];
	int rc; 

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("To: %x From: %x Flag: %x Cmd: %s\n",
		m->self_id, m->peer_id, m->flag, c));
	
	if (ISSET_BIT(m, OUT_BIT)) {
		rc = System(c, s, MAX_CMDLEN);	
	}
	else {
		rc = System(c, NULL, 0);
		s[0]='\0';
	}

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("Cmd returned %d\n", rc));

	if (ISSET_BIT(m, REQ_BIT)) {
		strncpy(c, s, MAX_CMDLEN);
		SendResp(m, rc);
	}

	free(m);

	return(rc);
}

int
SendFailMsg(q_msg *m)
{
	SendResp(m, -1);
	free(m);
	return(0);
}

void 
SendResp(q_msg *m, int rc) 
{
	m->self_id = m->peer_id;
	m->peer_id = SRVR_MSG_TYP;
	m->flag = rc;
	if (q_vsend(msgqid, (void *)m, Q_MSG_HDRLEN + strlen(&(m->cmd[0])) + 1, 0)	< 0) {
		NETERROR(MEXECD, ("q_vsend: %s\n", strerror(errno)));
	}
}

int
System(const char *cmdstr, char *buf, int buflen)
{
	int status;
	int rval = -128;

#ifdef USE_SYS_POPEN
	int cmdfd;
	int nbytes;

	if ( (cmdfd = sys_popen(cmdstr, 1)) >= 0) {
		if (buf) {
			if ((nbytes = read(cmdfd, buf, buflen-1)) >=0) {
				*(buf+nbytes) = '\0';	//	terminate the string
				NETDEBUG(MEXECD, NETLOG_DEBUG4, ("Cmd Output: %s", buf));
			}
			else {
				NETERROR(MEXECD, ("read: %s, for output of - %s\n", strerror(errno),
					cmdstr));
				status = 1;
			}
		}
		status = sys_pclose(cmdfd);
	}
	else {
		NETERROR(MEXECD, ("sys_popen: failed for cmd %s, error: %s\n", cmdstr,
			strerror(errno)));
		status = 1;
	}
#else
	status = system(cmdstr);	
#endif

	if (WIFEXITED(status)) {
		NETDEBUG(MEXECD, NETLOG_DEBUG3, ("normal termination, exit status = %d\n",
			WEXITSTATUS(status)));
		rval = (WEXITSTATUS(status)) - ( ((WEXITSTATUS(status)) & 0x80) ? 0x100 : 0 );
	}
	else if (WIFSIGNALED(status)) {
		NETDEBUG(MEXECD, NETLOG_DEBUG3, ("abnormal termination, signal number = %d%s\n", 
			WTERMSIG(status), WCOREDUMP(status) ? " (core file generated)" : ""));
		rval = -128;		
	}
	else if (WIFSTOPPED(status)) {
		NETDEBUG(MEXECD, NETLOG_DEBUG3, ("child stopped. signal number = %d\n", 
			WSTOPSIG(status)));
		rval = -128;
	}

	return(rval);
}

void
parse_opt(int argc, char *argv[]) 
{
	int c, errflg = 0;
	char usage[] = 	"Usage: execd [-d] [-c] [-v]\n"
					"\t-d does not daemonize\n"
					"\t-c does not read configuration file\n";
					"\t-v display version number\n";

	snprintf(pname, MAX_LINE, "%s", basename(argv[0]));

	while((c = getopt(argc, argv, "cdhl:v")) != EOF) {
		switch (c) {
			case 'c':
				execd_conf = 0;
				break;
			case 'd':
				execd_daemonize = 0;
				break;
			case 'l':
				debug_level = atoi(optarg);
				break;
			case 'v':
				fprintf (stdout, "\n");
				fprintf (stdout, "%s %s.%s, %s\n%s\n",
					EXCD_NAME,
					EXCD_VERSION,
					EXCD_MINOR,
					EXCD_BUILDDATE,
					EXCD_COPYRIGHT);
				fprintf (stdout, "\n");
				/* NOTE that this exits */
				exit (0);
				break;
			case 'h':
			default:
				errflg++;
				break;
		}
	}

	if (errflg) {
		printf("%s\n", usage);
		exit(1);
	}
}

void *
SendPMPoll(void *arg)
{
	char fn[] = "SendPMPoll(): ";
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	// Loop Forever
	for ( ; ; ) {
		Sendpoll(0);
		sleep(POLL_TIME_OUT);
	}
}

// Process Configuration, read from the config file
int
ConfigureExecD(void)
{
	int match = -1;

	match = FindServerConfig();

	if (debug_level) {
		NETLOG_SETLEVEL(MEXECD, NETLOG_DEBUG4);
	}

	if (!execd_conf) {
		return(-1);
	}

	if (match == -1) {
		printf("%s: Not configured to run in %s...\n", pname, config_file);
		exit(0);
	}

	if (serplexes[match].location.type == CONFIG_LOCATION_NONE) {
		NETERROR(MEXECD, ("Not Configured to run in server configuration file %s\n", config_file));
	}

	ServerSetLogging(pname, &serplexes[match]);

	return(0);
}

//
//	Function :
//		SignalInit()
//
//	Description :
//		Setup process wide signal handling, adhered to
//		by all threads via sigaction(2). This routine
//		should be called very early in the startup
//		process after daemonizing and prior to starting
//		any threads. In this way the blocked signal mask
//		will be inheritted by any threads.
//
static void
SignalInit(void)
{
	// Setup up alternate signal stack to be 
	// used by syncronous signal handler

	s.ss_sp = malloc( SIGSTKSZ );
	s.ss_size = SIGSTKSZ;
	s.ss_flags = 0;

	if ( sigaltstack( &s, 0 ) == -1 )
		perror( "sigaltstack" );

	// Create mask for blocking asyncronous signals
	// They must be blocked for sigwait() to work!!

	sigemptyset( &async_signal_mask );

	sigaddset( &async_signal_mask, SIGTERM );
	sigaddset( &async_signal_mask, SIGPOLL );
	sigaddset( &async_signal_mask, SIGCHLD );
	sigaddset( &async_signal_mask, SIGINT );
	sigaddset( &async_signal_mask, SIGHUP );
	sigaddset( &async_signal_mask, SIGALRM );

	// Setup SIGCHLD special case

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags = (SA_RESTART|SA_NOCLDSTOP|SA_ONSTACK);
	sigact.sa_handler = SIG_IGN;

	// sigaction( SIGCHLD, &sigact, NULL );

	// sigset( SIGCHLD, HandleCHLD );

	// Setup async signals to be ignored

	sigact.sa_flags = (SA_RESTART|SA_ONSTACK);

	sigaction( SIGPIPE, &sigact, NULL );
	sigaction( SIGWINCH, &sigact, NULL );
	sigaction( SIGTTIN,	&sigact, NULL );
	sigaction( SIGTTOU, &sigact, NULL );

	// Setup Signal handler to be called for
	// syncronous signals (fatal traps)

	sigact.sa_handler = SyncSigHandler;
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigaction(SIGBUS,	&sigact, NULL );
	/* No SIGEMT on Linux*/
#ifndef NETOID_LINUX
	sigaction(SIGEMT,	&sigact, NULL );
#endif
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );

	// Setup block mask for asyncronous signals to be
	// processed

	pthread_sigmask( SIG_BLOCK, &async_signal_mask, NULL );

	// Launch thread to process asyncronous signals

	ThreadLaunch( AsyncSigHandlerThread, NULL, 1);
}

//
//	Function :
//		AsyncSigHandlerThread()
//
//	Description :
//		This routine contains logic for a thread
//		to handle asyncronous signals in a syncronous
//		fashion for the rsd process. The thread is 
//		spawned at initialization time. sigwait(2)
//		is used to process the signals correctly.
//		The routine is not a signal handler so any
//		function call may be called from it.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char			c_sig[256];
	int				signo = 0, err;
	extern	void	restart_ispd(void);

	sigdelset( &async_signal_mask, SIGCHLD);
	//sigfillset( &async_signal_mask);

	for (;;) {

		memset( c_sig, 0, 256 );
		
#ifdef _POSIX_PTHREAD_SEMANTICS
		err = sigwait( &async_signal_mask, &signo );
		if (err < 0)
			continue;
#else
		signo = sigwait( &async_signal_mask);
#endif

		if (nx_sig2str( signo, c_sig, sizeof(c_sig) ) < 0) {
			sprintf(c_sig, "INVALID SIGNAL");
		}

		printf("%s received signal %d\n", pname, signo);

		switch (signo) {

		case SIGINT:
			HandleINT(signo);
			break;

		case SIGTERM:
			HandleTERM(signo);
			break;

		case SIGCHLD:
			HandleCHLD(signo);
			break;

		case SIGHUP:
			HandleHUP(signo);
			break;

		default:
			NETERROR(	MEXECD,
						("Caught %s(%d) signal - ignoring.\n",
						c_sig, signo));
			break;
		}
	}
}

//
//	Function :
//		SyncSigHandler()
//
//	Description :
//		This routine is a signal catcher used for
//		Synchronous signals. (fatal traps)
//
static void
SyncSigHandler(int signo)
{
	int32_t thread = pthread_self();
	char	c_sig[32];

	nx_sig2str( signo, c_sig, sizeof(c_sig));

	switch (signo) {
		case SIGBUS:
#ifndef NETOID_LINUX
		case SIGEMT:
#endif
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
			abort();
			break;
		default:
			NETERROR(	MEXECD,
					("Caught %s signal in LWP %d\n",
					c_sig,
					thread ));
			break;
	}
}

static void
HandleINT(int sig)
{
	char fn[] = "HandleINT";

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("%s: Signal SIGINT caught - stopping process\n", fn));
	NETINFOMSG(MEXECD, ("*** NexTone Cmd Execution Server shutdown: started ***\n"));
	q_vdelete(msgqid);
	exit(0);
}

static void
HandleTERM(int sig)
{
	char fn[] = "HandleTERM";

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("%s: Signal SIGTERM caught - stopping process\n", fn));
	NETINFOMSG(MEXECD, ("*** NexTone Cmd Execution Server shutdown: started ***\n"));
	q_vdelete(msgqid);
	exit(0);
}

static void
HandleHUP(int sig)
{
	char fn[] = "HandleHUP";

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("%s: Signal SIGHUP caught - Reconfiguring\n",
		fn));
	NetLogClose();
	DoConfig(ConfigureExecD);
//	exit(0);
	return;
}

static void
HandleCHLD(int sig)
{
	char fn[] = "HandleCHLD";
	pid_t		pid;
	int 		stat;

	NETDEBUG(MEXECD, NETLOG_DEBUG2, ("%s: Signal SIGCHLD caught - waiting for child\n", fn));

	while ((pid = waitpid((pid_t)-1, &stat, WNOHANG)) > 0)
		fprintf(stderr, "Child terminated\n");
	return;
}

