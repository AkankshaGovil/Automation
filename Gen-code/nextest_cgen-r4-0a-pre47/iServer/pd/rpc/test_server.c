//
//
//
//

#include <ispd.h>

#include <trace.h>
#include <threads.h>
#include "nxosd.h"

// Global variables

int32_t         trace_disk  = 1;
FILE *          trace_fdesc;
char            trace_log[ 256 ]    = "/var/adm/ispd";

char			progname[ MAXPATHLEN ];

LsMemStruct *		lsMem;
char				pidfile[ MAXPATHLEN ];
char				config_file[ MAXPATHLEN ];

trace_table_t   _trace_tbl;

thread_descr_t  temporary_threads[ MAX_TEMPORARY_THREADS ];
thread_descr_t  permanant_threads[ MAX_PERMANANT_THREADS ];

cv_t            init_thread_cv =
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, THRD_INIT };


static	pthread_t	rpc_server_thread;

pthread_t		signal_thread;

extern void *	ispd_rpcsvc_init( void * args );
extern void		ispd_rpcsvc_shutdown(void );

// Local routine prototypes

static void		SignalInit(void);

static void *	AsyncSigHandlerThread( void * args );

static void		SyncSigHandler( int signo );

int
main(	int		argc,
		char *	argv[] )
{
	char	description[ MAX_DESCR_SIZE ];
	char *	tempname;

	// Initialization stuff

	tempname = basename( argv[0] );

	sprintf( progname, "%s[%d]", tempname, (int) getpid() );

	threads_init();
	trc_init();

	SignalInit();

	strcpy( ispd_primary.name, "znb0" );
	strcpy( ispd_secondary.name, "znb3" );

	strcpy( ispd_primary.router, "204.180.228.14" );
	strcpy( ispd_secondary.router, "192.168.0.14" );

	strcpy( ispd_primary.vip, "204.180.228.217" );
	strcpy( ispd_secondary.vip, "192.168.0.217" );

	strcpy( ispd_ctl.name, "znb1" );

	strcpy( &ispd_ctl.peer_iservers[0][0], "192.168.228.1" );
	strcpy( &ispd_ctl.peer_iservers[1][0], "192.168.228.2" );
	strcpy( &ispd_ctl.peer_iservers[2][0], "192.168.228.3" );

	strcpy( ispd_ctl.ip, "192.168.228.1" );

	ispd_ctl.peer_count = 3;

	// Start up a thread to service rpc server requests

	sprintf( description, "rpc_server_thread()" );

	thread_create(	&rpc_server_thread,
					ispd_rpcsvc_init,
					(void*) NULL,
					description,
					0 );

	for(;;) 
		pause(); 

	return( 0 );
}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

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

	char	description[ MAX_DESCR_SIZE ];

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
	
	// Setup async signals to be ignored

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigact.sa_handler = SIG_IGN;

	sigaction( SIGPIPE, &sigact, NULL );
	sigaction( SIGWINCH, &sigact, NULL );
	sigaction( SIGTTIN,	&sigact, NULL );
	sigaction( SIGTTOU, &sigact, NULL );

	// Setup Signal handler to be called for
	// syncronous signals (fatal traps)

	sigact.sa_handler = SyncSigHandler;
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigaction(SIGBUS,	&sigact, NULL );
#ifndef NETOID_LINUX
	sigaction(SIGEMT,	&sigact, NULL );
#endif
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );

	// Setup block mask for asyncronous signals to be
	// processed

	pthread_sigmask( SIG_BLOCK, &async_signal_mask, NULL );

	// Startup thread to handle asyncronous signals

	sprintf( description, "AsyncSigHandlerThread()" );

	thread_create(  &signal_thread,
					AsyncSigHandlerThread,
					(void*) NULL,
					description,
					0 );
}

//
//	Function :
//		AsyncSigHandlerThread()
//
//	Description :
//		This routine is contains logic for a thread
//		to handle asyncronous signals in a syncronous
//		fashion for the gis process. The thread is 
//		spawned at initialization time. sigwait(2)
//		is used to process the signals correctly.
//		The routine is not a signal handler so any
//		function call may be called from it.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char	c_sig[256];
	int		signo;
	thread_descr_t* thread_descr;

	thread_descr = thread_register(args);

	memset( c_sig, (int) 0, 256 );

	for (;;)
	{

		sigwait( &async_signal_mask, &signo );

		nx_sig2str( signo, c_sig, sizeof(c_sig) );

		switch (signo)
		{
		case SIGINT:
			NETERROR(	MISPD,
						("Caught INT signal - terminating ispd\n" ));

			ispd_rpcsvc_shutdown();

			UnlinkPid(pidfile);
			exit(0);
			break;

		case SIGTERM:
			NETERROR(	MISPD,
						("Caught TERM signal - terminating ispd\n"));

			ispd_rpcsvc_shutdown();
			UnlinkPid(pidfile);
			exit(0);
			break;

		case SIGCHLD:
			sig_chld(signo);
			break;

		case SIGHUP:
			sig_hup(signo);
			break;

		default:
			NETERROR(	MISPD,
						("Caught %s signal - ignoring\n",
						c_sig ));
			break;
		}
	}

	thread_exit(thread_descr->transient);
	return((void*) NULL );
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
	int		pid;

	nx_sig2str( signo, c_sig, sizeof(c_sig) );

	switch (signo)
	{
	case SIGBUS:
#ifndef NETOID_LINUX
	case SIGEMT:
#endif
	case SIGFPE:
	case SIGILL:
	case SIGSEGV:

		if ((pid = fork()) == 0)
		{
			NETERROR(	MDEF,
						("Caught fatal %s signal in LWP %d\n",
						c_sig,
						thread ));
			exit(0);
		}
		else
			abort();
		break;
	default:
		NETERROR(	MDEF,
					("Caught %s signal in LWP %d\n",
					c_sig,
					thread ));
		break;
	}
}

