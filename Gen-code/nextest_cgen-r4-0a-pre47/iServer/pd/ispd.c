static char const rcsid[] = "$Id: ispd.c,v 1.28.2.31 2004/10/22 22:06:46 amar Exp $";
//
//	Module:
//		Iserver Peering Daemon (ispd)
//
//	Description:
//		The ispd monitors the health of iserver hosts in a multi
//		iserver host environment. One ispd runs on each host.
//
//
//

#include <ispd.h>
#ifndef NETOID_LINUX
#include <sys/priocntl.h>
#include <sys/tspriocntl.h>
#include <sys/rtpriocntl.h>
#endif
#include <icmp_echo_utils.h>
#include "nxosd.h"

// Global variables

char			pidfile[MAXPATHLEN];
char			config_file[MAXPATHLEN] = DEFAULT_CONFIG_FILE;
LsMemStruct *	lsMem;
char 			progname[MAXPATHLEN];
char *			arg_0;
int				ispd_fd;

struct ifi_info *ifihead;

int32_t			trace_disk  = 1;
int32_t			trace_initialized;
char			trace_log[ 256 ]    = "/var/log/ispd.log";

FILE *			trace_fdesc;
int32_t			trace_fd;

trace_table_t   _trace_tbl;


thread_descr_t  temporary_threads[ MAX_TEMPORARY_THREADS ];
thread_descr_t  permanant_threads[ MAX_PERMANANT_THREADS ];

cv_t            init_thread_cv =
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, THRD_INIT };

cv_t			event_signal_cv = 
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, THRD_INIT };

cv_t			monitor_signal_cv = 
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, THRD_INIT };

cv_t			ispd_main_cv =
{ PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, THRD_INIT };

uint32_t		ispd_event_qid;

#ifdef _DEBUG_MALLOC_
	unsigned long histid1, histid2, orig_size, current_size;
#endif

//
// Handles to local threads created
//

pthread_t		rpc_server_thread;
pthread_t		signal_thread;
pthread_t		get_peerinfo_thread;
pthread_t		check_localhost_thread;

pthread_t		ispd_eventloop_thread;
pthread_t		ispd_monitor_thread;

//
// Information compliled about other hosts from their ispd daemons
//

peer_info_t		peer_info[ MAX_ISPD_PEERS ];	// peer information

int				peer_count;						// number of peers


peer_info_t		local_info;

char			primary_vip_interface[ STRSZ ];
char			secondary_vip_interface[ STRSZ ];

//
// Global state of this host as discerned via evaluation of
// network interfaces and iServer software.
//

// Local routine prototypes

static	int		ispd_initialize(	int		argc,
									char *	argv[] );

static void		ispd_initialize_exit( void );

static 	int		Config_Callback( 	void );

static	int		ParseCommandLine(	int argc,
									char * argv[] );

static void		SignalInit(void);

static void *	AsyncSigHandlerThread( void * args );

static void		SyncSigHandler( int signo );

static void		dump_server_cfg( void );

static void		dump_peer_status( peer_info_t * peer_info_ptr );

static void *	get_peerinfo(	void * args );

static void *	ispd_monitor(	void * args );

static void *	ispd_eventloop(	void * args );

static void *	check_localhost(	void * args );

static void		check_network_interfaces( void );

static interface_status_t get_link_status(	char * devname,
                 							int    instance );

static void		checkIserver( void );

static void		check_peers( void );
static int		check_peer_iservers( void );
static void		checkRsd( void );

static void		restart_ispd( void );


static void		ispd_routing_change(ispd_route_action_t	action,
									ispd_interface_t *	iface_desc );

void			recycle_virtifs( ispd_route_action_t	action, int call );

void *			dummy_handle = check_peer_iservers;
void *			dummy_handle1 = ispd_routing_change;
void *			dummy_handle2 = checkRsd;
static int		reported;
int			init_state();
int			*p_iserver_state = NULL;

int
main(	int 	argc,
		char *	argv[] )
{
	char	description[ MAX_DESCR_SIZE ];

	// Set up as RT SCHED_RR queue thread with quantum
	// of 100 ms.

	nx_thread_set_rt( 100000000 );


	// Call function to do ispd initialization tasks.
	
	if ( ispd_initialize( argc, argv ) == 0 )
	{

		// Start up a thread to service rpc server requests

		sprintf( description, "rpc_server_thread()" );

		thread_create(  &rpc_server_thread,
						ispd_rpcsvc_init,
						(void*) NULL,
						description,
						0 );


		// Start up a thread to retreive peer information

		sprintf( description, "get_peerinfo_thread()" );

		thread_create(  &get_peerinfo_thread,
						get_peerinfo,
						(void*) NULL,
						description,
						0 );

		// Start up a thread to monitor the health of this
		// system and our active peers if we are a standby
		// host

		sprintf( description, "ispd_eventloop()" );

		thread_create(  &ispd_eventloop_thread,
						ispd_eventloop,
						(void*) NULL,
						description,
						0 );

		cv_wait( &ispd_main_cv, THRD_DONE );
	}

	return(0);
}

//
//	Function :
//		ispd_initialize()
//
//  Arguments       :
//
//		argc			argc argument passed to main().
//
//		argv			argv argument passed to main().
//
//	Description :
//		This routine is called by main() to do basic
//		initialization tasks. These include :
//
//			1)	Parse command line input.
//
//			2)	checking to make sure another ispd
//				is not already running on this host.
//				exit process if already running.
//
//			3)	Parsing the server.cfg file to obtain
//				ispd-related configuration. exit process
//				if not configured.
//
//			4)	Setting up signal handling for ispd
//				process.
//
//			5)	Setting up threading environment.
//
//			6)	Setting up logging facilities.
//
//	Return Values:
//		-1	on error terminal condition
//		 0  initialization OK
//
static int
ispd_initialize(	int		argc,
					char *	argv[] )
{
	struct ifi_info	*	ifi_ptr;
        char                    buf[INET_ADDRSTRLEN];
	char *				tempname;
	char *				cptr;

	#ifdef _DEBUG_MALLOC_
		union dbmalloptarg m;
	#endif

	#ifdef _DEBUG_MALLOC_

		dbmalloc_init();

		m.i = ( M_HANDLE_CORE | M_HANDLE_DUMP );

		dbmallopt( MALLOC_WARN, &m );

		m.str = "malloc.log";
		dbmallopt( MALLOC_ERRFILE, &m );

		orig_size = malloc_inuse( &histid1 );
		
	#endif

	arg_0 = argv[ 0 ];

	tempname = basename( arg_0 );

	sprintf( progname, "%s[%d]", tempname, (int) getpid() );

	// Init system utilities in sysutils.c
	
	sys_utils_init();

	// Initialize random number generator

	rand_init();

	// Parse command line arguments

	ParseCommandLine( argc, argv ); 

	// Read the configuration file

	myConfigServerType = CONFIG_SERPLEX_GIS;
	DoConfig( Config_Callback );

	threads_init();

	if( init_state() < 0)
	{
		ispd_initialize_exit();
		return(-1);
	}

	local_info.server_type = ispd_type;

	//
	// Daemon start-up code to prevent multiple copies of ispd
	// on one host. Uses advisory locking scheme to insure locking
	// ala Stevens - Advanced Unix Programming - Page 372
	//

	{
		char	pid_buf[10];
		int		fd_val;

		// Make sure ispd is not already running

		sprintf( pidfile, "%s/%s", "/var/run", ISPD_PID_FILE );

		if ( ( ispd_fd = open( pidfile, (O_WRONLY | O_CREAT), 0644 )) < 0 )
		{
			trc_error(	MISPD,
						"INIT : Error opening pid file, %s\n",
						pidfile );
			return(-1);
		}

		if ( write_lock( ispd_fd, 0, SEEK_SET, 0 ) < 0 )
		{
			if ( errno == EACCES || errno == EAGAIN )
			{
				trc_error(	MISPD,
							"INIT : ispd already running exiting\n" );

				ispd_initialize_exit();

				return(-1);
			}
			else
			{
				trc_error(	MISPD,
							"INIT : write_lock() error on file, %s\n",
							pidfile );

				ispd_initialize_exit();

				return(-1);
			}
		}

		// Truncate file to zero length prior to writing our pid

		if ( ftruncate( ispd_fd, 0 ) < 0 )
		{
			trc_error(	MISPD,
						"INIT : ftruncate() error on file, %s\n",
						pidfile );

			ispd_initialize_exit();

			return(-1);
		}

		memset( pid_buf, (int) 0, 10 );

		sprintf( pid_buf, "%d", (int) getpid() );
		
		if ( write( ispd_fd, pid_buf, strlen( pid_buf ) ) != strlen( pid_buf ) ) 
		{
			trc_error(	MISPD,
						"INIT : write() error on file, %s\n",
						pidfile );

			ispd_initialize_exit();

			return(-1);
		}

		// set close-on-exec flag for descriptor

		if ( ( fd_val = fcntl( ispd_fd, F_GETFD, 0 )) < 0 )
		{
			trc_error(	MISPD,
						"INIT : fcntl() F_GETFD error on file, %s\n",
						pidfile );

			ispd_initialize_exit();

			return(-1);
		}

		fd_val |= FD_CLOEXEC;

		if ( fcntl( ispd_fd, F_SETFD, fd_val ) < 0 )
		{
			trc_error(	MISPD,
						"INIT : fcntl() F_SETFD error on file, %s\n",
						pidfile );

			ispd_initialize_exit();

			return(-1);
		}
	}


	// Setup process wide signal handling

	SignalInit();

	// Collect network interface information including aliases

	ifihead = get_iface_info( AF_INET, 1 );

	// Grab key for	vip selection process

	local_info.host_key = ispd_random();

	// Issue cli command to plumb remaining virtual interfaces

	{
		char    command[128];
		int    	cmdfd;


		sprintf(    command,
   					"/usr/local/nextone/bin/cli realm open all" );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
			sys_pclose( cmdfd );
		}
	}

	// Start with primary and secondary vip interfaces in down state

	recycle_virtifs(ISPD_VIP_DOWN, 1);

	if ( ispd_ctl.defined )
	{
		int i;

		if ( ( ifi_ptr = findIfByIfname(	ifihead,
											ispd_ctl.name ) ) != NULL )
		{
			// Note : only works for IP4

			strcpy( ispd_ctl.ip,
					inet_ntop( AF_INET, &ifi_ptr->ifi_addr->sin_addr , buf, INET_ADDRSTRLEN) );

			strcpy( local_info.ctl_interface, ispd_ctl.name );
			strcpy( local_info.ctl_ip, ispd_ctl.ip );

			strcpy( local_info.ctl_devname, "/dev/" );
			strcat( local_info.ctl_devname, ispd_ctl.name );

			cptr = local_info.ctl_devname
					+ strlen( local_info.ctl_devname ) - 1;

			if ( isdigit( (int) *cptr ) )
			{
				local_info.ctl_instance = atoi( cptr );
			}
			else
			{
				trc_error(	MISPD, 
							"CFGE : Control interface, \"%s\", "
							"does not end in integer - invalid - exiting\n",
							ispd_ctl.name );

				ispd_initialize_exit();

				return(-1);
			}
			
			*cptr = 0;
		}

		if ( pthread_mutex_init(&local_info.lock,
								NULL ) )
		{
			trc_error(	MISPD,
						"INIT : pthread_mutex_init() failed for local_info\n");
		}

		for ( i = 0; i < MAX_ISPD_PEERS; i++ )
		{
			if ( pthread_mutex_init(&peer_info[ i ].lock,
									NULL ) )
			{
				trc_error(	MISPD,
							"INIT : pthread_mutex_init() failed for peer %d\n",
							i );
			}
		}

		peer_count = ispd_ctl.peer_count;

		for ( i = 0; i < peer_count; i++ )
		{
			if ( validate_hostent(	&ispd_ctl.peer_iservers[ i ][ 0 ],
									peer_info[ i ].ctl_ip ) < 0 )
			{
				trc_error(	MISPD,
							"CFGE : peer_iserver entry %d in server.cfg, \"%s\", "
							"has no system host entry - exiting\n",
							i, 
							&ispd_ctl.peer_iservers[ i ][ 0 ] );

				ispd_initialize_exit();

				return(-1);
			}
		}
	}

	if ( ispd_type == ISPD_TYPE_DISABLED )
	{
		trc_debug(	MISPD, NETLOG_DEBUG1,
					"INIT : ispd_type       \"disabled\"\n" );

		ispd_initialize_exit();

		return(-1);
	}

	trc_error( MISPD, "INIT : Starting ispd\n" );

	// Get initial status of interfaces

	check_network_interfaces();

	// Print out debugging data if requested

	dump_server_cfg();

	return(0);
}

//
//	Function :
//		ParseCommandLine()
//
//  Arguments       :
//
//		argc			argc argument passed to main().
//
//		argv			argv argument passed to main().
//
//	Description :
//		Routine to parse command line arguments to ispd.
//
//	Return Values:
//		None
//
static int
ParseCommandLine(	int argc,
					char * argv[] )
{
	char *	tempname;

	tempname = basename( arg_0 );

	sprintf( progname, "%s[%d]", tempname, (int) getpid() );

	argv ++;
	argc --;

	while (argc > 0)
	{
		if (argv[0][0] == '-')
		{
			switch (argv[0][1])
			{
			case 'v':
				fprintf( stdout, "\n");
				fprintf( stdout, "%s %s.%s, %s\n%s\n",
					"NexTone ISPD (Iserver Peering Daemon) ",
					GIS_VERSION,
					GIS_MINOR,
					GIS_BUILDDATE,
					GIS_COPYRIGHT);
				fprintf (stdout, "\n");

				/* NOTE that this exits */
				exit( 0 );

				break;
			
			case 'f':
				strcpy(config_file, argv[1]);
		
				argc --;
				argv ++;
				
				break;
			}
		}

		argc --;
		argv ++;
	}

	return 0;
}

static int
Config_Callback( void )
{
	int match = -1;

	// Process Configuration, read from the config file

	match = FindServerConfig();

	if (match == -1)
	{
		fprintf(stderr, "Not Configured to run...\n");
		exit(0);
	}

	if (serplexes[match].ispd.location.type == CONFIG_LOCATION_NONE)
	{
		fprintf( stderr,
				 "%s : Not Configured to run in server configuration file\n",
				 progname );
		exit(0);
	}

	// Setup logging

	ServerSetLogging( progname, &serplexes[ match ] );


	return(0);
}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

//
//	Function :
//		SignalInit()
//
//  Arguments       :
//		None.
//
//	Description :
//		Setup process wide signal handling, adhered to
//		by all threads via sigaction(2). This routine
//		should be called very early in the startup
//		process after daemonizing and prior to starting
//		any threads. In this way the blocked signal mask
//		will be inheritted by any threads.
//
//	Return Values:
//		None.
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
	sigaddset( &async_signal_mask, SIGUSR1 );
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
//  Arguments       :
//		args			pointer to a thread_args_t structure
//						used by the threads.h framework routines.
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
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char	c_sig[256];
	int		signo;
	thread_descr_t* thread_descr;
	#ifdef _DEBUG_MALLOC_
		int fd;
	#endif


	thread_descr = thread_register(args);

	memset( c_sig, (int) 0, 256 );

	for (;;)
	{

		sigwait( &async_signal_mask, &signo );

		nx_sig2str( signo, c_sig, sizeof(c_sig) );

		switch (signo)
		{
		case SIGINT:
			trc_error(	MISPD,
						"SGNL : Caught INT signal - terminating ispd\n" );

			//UnlinkPid(pidfile);

			// bring primary and secondary vip interfaces down if up

			recycle_virtifs(ISPD_VIP_DOWN, 2);

			millisleep( 1000 );

			svc_exit();

			millisleep( 2000 );

			#ifdef _DEBUG_MALLOC_
				current_size = malloc_inuse( &histid2 );
				fd = open( "malloc.inuse", O_CREAT | O_RDWR );
				malloc_list( fd, histid1, histid2 );
				close( fd );
			#endif

			cv_signal( &ispd_main_cv, THRD_DONE );

			break;

		case SIGTERM:
			trc_error(	MISPD,
						"SGNL : Caught TERM signal - terminating ispd\n");

			// bring primary and secondary vip interfaces down if up

			recycle_virtifs(ISPD_VIP_DOWN, 3);

			millisleep( 1000 );

			svc_exit();

			millisleep( 2000 );

			#ifdef _DEBUG_MALLOC_
				current_size = malloc_inuse( &histid2 );
				fd = open( "malloc.inuse", O_CREAT | O_RDWR );
				malloc_list( fd, histid1, histid2 );
				close( fd );
			#endif

			cv_signal( &ispd_main_cv, THRD_DONE );

			break;

		case SIGCHLD:
			trc_error(	MISPD,
						"SGNL : Caught %s signal - ignoring\n",
						c_sig );
			sig_chld(signo);
			break;

		case SIGUSR1:
			{
				char *new_log_setting;

				if ( NetLogStatus[ MISPD ] == 0 )
				{
					NetLogStatus[ MISPD ] = NETLOG_DEBUG1;
					new_log_setting = "NETLOG_DEBUG1";
				}
				else
				if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG1 )
				{
					NetLogStatus[ MISPD ] = NETLOG_DEBUG2;
					new_log_setting = "NETLOG_DEBUG2";
				}
				else
				if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
				{
					NetLogStatus[ MISPD ] = NETLOG_DEBUG3;
					new_log_setting = "NETLOG_DEBUG3";
				}
				else
				if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG3 )
				{
					NetLogStatus[ MISPD ] = NETLOG_DEBUG4;
					new_log_setting = "NETLOG_DEBUG4";
				}
				else
				{
					NetLogStatus[ MISPD ] = 0;
					new_log_setting = "OFF";
				}

				trc_error(	MISPD,
							"SGNL : Caught %s signal - changing logging to %s\n",
							c_sig,
							new_log_setting );
			}
			break;

		case SIGHUP:
			{
				trc_error(	MISPD,
							"SGNL : Caught %s signal - reconfiguring\n",
							c_sig );

				millisleep( 1000 );

				svc_exit();

				millisleep( 2000 );

				restart_ispd();

				#ifdef _DEBUG_MALLOC_
					current_size = malloc_inuse( &histid2 );
					fd = open( "malloc.inuse", O_CREAT | O_RDWR );
					malloc_list( fd, histid1, histid2 );
					close( fd );
				#endif

				cv_signal( &ispd_main_cv, THRD_DONE );
			}

			break;

		default:
			trc_error(	MISPD,
						"SGNL : Caught %s signal - ignoring\n",
						c_sig );
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
//  Arguments       :
//		signo	signal number passed to signal handler.
//
//	Description :
//		This routine is a signal catcher used for
//		Synchronous signals. (fatal traps)
//
//	Return Value:
//		None.
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
			trc_error(	MISPD,
						"SGNL : Caught fatal %s signal in LWP %d\n",
						c_sig,
						thread );
			exit(0);
		}
		else
			abort();
		break;
	default:
		trc_error(	MISPD,
					"SGNL : Caught %s signal in LWP %d\n",
					c_sig,
					thread );
		break;
	}
}

//
//	Function :
//		dump_server_cfg()
//
//  Arguments       :
//		None
//
//	Description :
//		Utility function to dump the data read from
//		the server.cfg
//
//	Return Values:
//		None.
//
static void
dump_server_cfg( void )
{
	int		i;

	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG1 )
	{
		if ( ispd_type == ISPD_TYPE_ACTIVE )
		{
			trc_error(	MISPD,
						"CFG  : ispd_type       \"active\"\n" );
		}
		else
		if ( ispd_type == ISPD_TYPE_STANDBY )
		{
			trc_error(	MISPD,
						"CFG  : ispd_type       \"standby\"\n" );
		}
		else
		if ( ispd_type == ISPD_TYPE_DISABLED )
		{
			trc_error(	MISPD,
						"CFG  : ispd_type       \"disabled\"\n" );
		}

		trc_error(	MISPD,
					"CFG  : host_key        %d\n",
					local_info.host_key );

		// Control interface debugging

		if ( ispd_ctl.defined )
		{
			trc_error(	MISPD,
						"CFG  : control_interface:\n" );

			if ( strlen( ispd_ctl.name ) )
			{
				trc_error(	MISPD,
							"CFG  :      name       \"%s\"\n",
							ispd_ctl.name );
			}
			else
			{
				trc_error(	MISPD,
							"CFG  :      name       NA\n" );
			}

			if ( strlen( ispd_ctl.ip ) )
			{
				trc_error(	MISPD,
							"CFG  :      ip         \"%s\"\n",
							ispd_ctl.ip );
			}
			else
			{
				trc_error(	MISPD,
							"CFG  :      ip         NA\n" );
			}

			for ( i = 0; i < ispd_ctl.peer_count; i++ )
			{
				trc_error(	MISPD,
							"CFG  :      peer[%1d]    \"%s\"\n",
							i,
							peer_info[ i ].ctl_ip );
			}
		}
	}
}

//
//	Function :
//		dump_peer_status()
//
//  Arguments       :
//		None
//
//	Description :
//		Utility function to dump server status information
//
//	Return Values:
//		None.
//
static void
dump_peer_status( peer_info_t * peer_info_ptr )
{

	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
	{
		bool_t	islocal = FALSE;

		if ( !strcmp( local_info.ctl_ip, peer_info_ptr->ctl_ip ) )
		{
			trc_error(	MISPD,
						"STAT : Local Status changed %s:\n",
						peer_info_ptr->ctl_ip );
			islocal = TRUE;
		}
		else
		{
			trc_error(	MISPD,
						"STAT : Peer Status changed %s:\n",
						peer_info_ptr->ctl_ip );
			islocal = FALSE;
		}

		// iserver status

		switch ( peer_info_ptr->iserver_status )
		{
		case ISPD_ISERVER_UP:

			trc_error(	MISPD,
						"STAT :    iserver          UP\n" );

			break;

		case ISPD_ISERVER_DOWN:

			trc_error(	MISPD,
						"STAT :    iserver          DOWN\n" );

			break;

		case ISPD_ISERVER_UNKNOWN:

			trc_error(	MISPD,
						"STAT :    iserver          Unknown\n" );

			break;
		}

		// Primary interface debugging

		if ( peer_info->primary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{
			trc_error(	MISPD,
						"STAT :    primary_interface:\n" );

			if ( peer_info_ptr->primary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      interface      UP\n" );
			}
			else
			if ( peer_info_ptr->primary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      interface      DOWN\n" );
			}

			if ( peer_info_ptr->primary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      Cable link     CONNECTED\n" );
			}
			else
			if ( peer_info_ptr->primary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      Cable link     NO_CARRIER\n" );
			}

			if ( peer_info_ptr->primary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      vip interface  UP\n" );
			}
			else
			if ( peer_info_ptr->primary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      vip interface  DOWN\n" );
			}
		}

		// Secondary interface debugging

		if ( peer_info->secondary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{
			trc_error(	MISPD,
						"STAT :    secondary_interface:\n" );

			if ( peer_info_ptr->secondary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      interface      UP\n" );
			}
			else
			if ( peer_info_ptr->secondary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      interface      DOWN\n" );
			}

			if ( peer_info_ptr->secondary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      Cable link     CONNECTED\n" );
			}
			else
			if ( peer_info_ptr->secondary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      Cable link     NO_CARRIER\n" );
			}

			if ( peer_info_ptr->secondary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"STAT :      vip interface  UP\n" );
			}
			else
			if ( peer_info_ptr->secondary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"STAT :      vip interface  DOWN\n" );
			}
		}

		// Control interface debugging

		trc_error(	MISPD,
					"STAT :    control_interface:\n" );

		if ( peer_info_ptr->ctl_interface_status == ISPD_IFACE_STATUS_UP )
		{
			trc_error(	MISPD,
						"STAT :      interface      UP\n" );
		}
		else
		if ( peer_info_ptr->ctl_interface_status == ISPD_IFACE_STATUS_DOWN )
		{
			trc_error(	MISPD,
						"STAT :      interface      DOWN\n" );
		}

		if ( peer_info_ptr->ctl_link_status == ISPD_IFACE_STATUS_UP )
		{
			trc_error(	MISPD,
						"STAT :      Cable link     CONNECTED\n" );
		}
		else
		if ( peer_info_ptr->ctl_link_status == ISPD_IFACE_STATUS_DOWN )
		{
			trc_error(	MISPD,
						"STAT :      Cable link     NO_CARRIER\n" );
		}

	}
}

//
//	Function :
//		get_peerinfo()
//
//  Arguments       :
//
//		args			pointer to a thread_args_t structure
//						used by the threads.h framework routines.
//
//	Description :
//			This routine is started as a thread to get
//		information from each of its peer ispd services,
//		sequentially.
//			For each peer, a client rpc handle to the peer
//		iserver's ispd is obtained. This peer handle is saved
//		for future use. A client call is then made to the ispd
//		service on the peer to get configuration information
//		from that peer.
//			If we are unable to get a client handle for a peer
//		we skip that host coming back to it later.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
static void *
get_peerinfo(	void * args )
{
	thread_descr_t	*	thread_descr;
	int					i;
	int					retreived = 0;
	int					rc;

	thread_descr = thread_register( args );


	peer_count = ispd_ctl.peer_count;

	while ( retreived < peer_count )
	{
		for ( i = 0; i < peer_count; i++ )
		{
			rc = get_iserver_info( &peer_info[ i ] );

			if ( rc == 0 )
			{
				local_info.peer_status[ i ].ctl_reachable = TRUE;
				retreived++;
			}
			else
			{
				local_info.peer_status[ i ].ctl_reachable = FALSE;
			}
		}

		millisleep( 3000 );
	}

	thread_exit( thread_descr->transient );
	return( (void*) NULL );
}

//
//	Function :
//		ispd_monitor()
//
//  Arguments       :
//		args			pointer to a thread_args_t structure
//						used by the threads.h framework routines.
//
//	Description :
//		This routine is started as a thread to analyse
//		and evaluate the status infomation collected
//		in the eventloop(), periodically, and then take
//		action based on the status.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
static void *
ispd_monitor(	void * args )
{
	thread_descr_t	*	thread_descr;
	int					i;
	peer_info_t *		peer_info_ptr = NULL;
	struct timespec		delay;
	int32_t				timeout;
	static selection_result_t	previous;
	static selection_result_t	current;
	int 		local_host_winner;

	thread_descr = thread_register( args );

	//
	// Wait about 2 seconds to allow current local status of pinged
	// hosts information to settle before interpreting.
	//

	millisleep( 2000 );

	send_status_change();

	millisleep( 2000 );

	cv_signal( &monitor_signal_cv, THRD_DONE );

	for (;;) {
		lock_mutex( &local_info.lock );

#if 0
#endif

		if ( check_local_health() == SYSTEM_HEALTHY && *p_iserver_state != ISPD_ISERVER_UP )
		{
			local_host_winner = 1;

			for ( i = 0; i < ispd_ctl.peer_count; i++ )
			{
				current = select_vip_owner(&peer_info[ i ], 0);
				switch( current )
				{
					case LOCALHOST_OWNS_VIP:
						break;

					case PEERHOST_UNREACHABLE:
						break;

					case PEERHOST_OWNS_VIP:
					{
						local_host_winner = 0;
						break;
					}
					default:
						break;
				}
			}
			if(local_host_winner)
			{
				local_info.local_status_changed = TRUE;
				*p_iserver_state = ISPD_ISERVER_UP;

				recycle_virtifs(ISPD_VIP_UP, 4);
			}
		}

		// Send updated iserver information to peers

		send_iserver_info();

		// Send status change information to gis and dbsync if needed

		send_status_change();

		if ( local_info.local_status_changed == TRUE ) {
			dump_peer_status( &local_info );
			local_info.local_status_changed = FALSE;
		}

		for ( i = 0; i < peer_count; i++ ) {
			if ( local_info.peer_status[ i ].peer_status_changed == TRUE ) {
				dump_peer_status( &peer_info[ i ] );
				local_info.peer_status[ i ].peer_status_changed = FALSE;
			}
		}

		unlock_mutex( &local_info.lock );

		clock_gettime( CLOCK_REALTIME, &delay );
		incr_timespec( &delay, 10, 0 );

		switch(  cv_wait_timed( 	&monitor_signal_cv,
									( THRD_ABORT | THRD_CONT ),
									&delay,
									&timeout ) )
		{
		case THRD_ABORT:
			trc_debug(	MISPD, NETLOG_DEBUG2,
						"MON  : THRD_ABORT signaled for check_localhost()\n" );
			cv_signal( &monitor_signal_cv, THRD_DONE );
			break;
		case THRD_CONT:
			trc_debug(	MISPD, NETLOG_DEBUG2,
						"MON  : THRD_CONT signaled for check_localhost()\n" );
			cv_signal( &monitor_signal_cv, THRD_DONE );
			break;
		default:
			if ( timeout )
			{
/*
				trc_debug(	MISPD, NETLOG_DEBUG2,
						"MON  : THRD_TIMEOUT signaled for check_localhost()\n" );
*/
			}
			break;
		}
	}

	thread_exit( thread_descr->transient );
	return( (void*) NULL );
}


//
//	Function :
//		ispd_eventloop()
//
//  Arguments       :
//		args	pointer to a thread_args_t structure
//				used by the threads.h framework routines.
//
//	Description :
//		This routine is started as a thread to initialize
//		eventloop processing and handle incoming events.
//		It initialized threads which will deliver events
//		to it and also starts up the ispd_monitor() thread
//		which will analyze status information delivered via
//		events periodically.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
static void *
ispd_eventloop(	void * args )
{
	thread_descr_t	*	thread_descr;
	char				description[ MAX_DESCR_SIZE ];
	ispd_event_entry_t	event;
	int					local_status_change;
	int					remote_status_change;
	int					i;

	thread_descr = thread_register( args );

	// Initialize ispd event queue

	if ( ( ispd_event_qid = ispd_q_create(	sizeof( ispd_event_entry_t ),
											1024 )) < 0 )
	{
		trc_error(	MISPD,
					"DEAD : Unable to create ispd event queue - exiting\n" );
		cv_signal( &ispd_main_cv, THRD_DONE );
	}

    // Start up a thread to retreive peer information

    sprintf( description, "check_localhost_thread()" );

	thread_create(  &check_localhost_thread,
					check_localhost,
					(void*) NULL,
					description,
					0 );

    // Start up a thread to monitor events and take action

    sprintf( description, "ispd_monitor()" );

	thread_create(  &ispd_monitor_thread,
					ispd_monitor,
					(void*) NULL,
					description,
					0 );

	//
	// Drop into ispd event queue loop for the rest of ispd processes
	// existence
	//

	while ( ispd_q_recv( ispd_event_qid, &event ) == 0 )
	{
		local_status_change = 0;
		remote_status_change = 0;

		switch( event.type )
		{
		case ISPD_EVENT_PRIMARY_IFACE_STATUS:

			// Did somebody use an ifconfig command to down the
			// ppa interface for the primary ?

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				// Yes, indicate as much and bring down the vip

				lock_mutex( &local_info.lock );

				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary \"%s\" DOWN\n",
							local_info.primary_interface );

				local_info.primary_interface_status = ISPD_IFACE_STATUS_DOWN;
				recycle_virtifs(ISPD_VIP_DOWN, 12);

				unlock_mutex( &local_info.lock );
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				lock_mutex( &local_info.lock );

				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary \"%s\" UP\n",
							local_info.primary_interface );
				local_info.primary_interface_status = ISPD_IFACE_STATUS_UP;

				unlock_mutex( &local_info.lock );
			}

			local_status_change = 1;

			break;

		case ISPD_EVENT_PRIMARY_IFACE_LINK_STATUS:
			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_error(	MISPD,
							"EVNT : Cable disconnected - primary \"%s\"\n",
							local_info.primary_interface );

				lock_mutex( &local_info.lock );

				local_info.primary_link_status = ISPD_IFACE_STATUS_DOWN;

				unlock_mutex( &local_info.lock );

				recycle_virtifs(ISPD_VIP_DOWN, 13);

				local_status_change = 1;
				reported = 0;

			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_error(	MISPD,
							"EVNT : Cable connected    - primary \"%s\"\n",
							local_info.primary_interface );

				lock_mutex( &local_info.lock );
				local_info.primary_link_status = ISPD_IFACE_STATUS_UP;
				unlock_mutex( &local_info.lock );

				local_status_change = 1;
			}
			break;

		case ISPD_EVENT_PRIMARY_VIP_STATUS:

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary vip \"%s\" DOWN\n",
							primary_vip_interface );
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary vip \"%s\" UP\n",
							primary_vip_interface );
			}

			local_status_change = 1;

			break;

		case ISPD_EVENT_SECONDARY_IFACE_STATUS:

			// Did somebody use an ifconfig command to down the
			// ppa interface for the secondary ?

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				// Yes, indicate as much and bring down the vip

				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - secondary \"%s\" DOWN\n",
							local_info.secondary_interface );

				lock_mutex( &local_info.lock );

				local_info.secondary_interface_status = ISPD_IFACE_STATUS_DOWN;

				unlock_mutex( &local_info.lock );

				recycle_virtifs(ISPD_VIP_DOWN, 18);

			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - secondary \"%s\" UP\n",
							local_info.secondary_interface );

				lock_mutex( &local_info.lock );
				local_info.secondary_interface_status = ISPD_IFACE_STATUS_UP;
				unlock_mutex( &local_info.lock );
			}

			local_status_change = 1;

			break;

		case ISPD_EVENT_SECONDARY_IFACE_LINK_STATUS:

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_error(	MISPD,
							"EVNT : Cable disconnected - secondary \"%s\"\n",
							local_info.secondary_interface );

				lock_mutex( &local_info.lock );

				local_info.secondary_link_status = ISPD_IFACE_STATUS_DOWN;

				unlock_mutex( &local_info.lock );
				recycle_virtifs(ISPD_VIP_DOWN, 19);

				local_status_change = 1;
				reported = 0;
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_error(	MISPD,
							"EVNT : Cable connected    - secondary \"%s\"\n",
							local_info.secondary_interface );

				lock_mutex( &local_info.lock );
				local_info.secondary_link_status = ISPD_IFACE_STATUS_UP;
				unlock_mutex( &local_info.lock );

				local_status_change = 1;
			}

			break;

		case ISPD_EVENT_SECONDARY_VIP_STATUS:

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary vip \"%s\" DOWN\n",
							secondary_vip_interface );
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_debug(	MISPD, NETLOG_DEBUG1,
							"EVNT : ifconfig - primary vip \"%s\" UP\n",
							secondary_vip_interface );
			}

			local_status_change = 1;

			break;

		case ISPD_EVENT_CTL_IFACE_STATUS:

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_error(	MISPD,
							"EVNT : ifconfig - control \"%s\" DOWN\n",
							local_info.ctl_interface );

				lock_mutex( &local_info.lock );
				local_info.ctl_interface_status = ISPD_IFACE_STATUS_DOWN;
				unlock_mutex( &local_info.lock );
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_error(	MISPD,
							"EVNT : ifconfig - control \"%s\" UP\n",
							local_info.ctl_interface );
				lock_mutex( &local_info.lock );
				local_info.ctl_interface_status = ISPD_IFACE_STATUS_UP;
				unlock_mutex( &local_info.lock );
			}

			break;

		case ISPD_EVENT_CTL_IFACE_LINK_STATUS:

			if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_DOWN ) {
				trc_error(	MISPD,
							"EVNT : Cable disconnected - control \"%s\"\n",
							local_info.ctl_interface );
				lock_mutex( &local_info.lock );
				local_info.ctl_link_status = ISPD_IFACE_STATUS_DOWN;
				unlock_mutex( &local_info.lock );
			} else if ( (interface_status_t) event.value == ISPD_IFACE_STATUS_UP ) {
				trc_error(	MISPD,
							"EVNT : Cable connected    - control \"%s\"\n",
							local_info.ctl_interface );
				lock_mutex( &local_info.lock );
				local_info.ctl_link_status = ISPD_IFACE_STATUS_UP;
				unlock_mutex( &local_info.lock );
			}

			break;

		case ISPD_EVENT_ISERVER_STATUS:

			switch(local_info.iserver_status)
			{
			case ISPD_ISERVER_UP:

				trc_error(	MISPD,
							"EVNT : local iserver      - UP\n" );
				reported = 0;
				break;

			case ISPD_ISERVER_DOWN:

				trc_error(	MISPD,
							"EVNT : local iserver      - DOWN\n" );
				recycle_virtifs(ISPD_VIP_DOWN, 20);
				reported = 0;
				break;

			case ISPD_ISERVER_UNKNOWN:

				trc_error(	MISPD,
							"EVNT : local iserver      - UNKNOWN\n" );

				break;
			}

			local_status_change = 1;

			break;
		default:
			break;
		}

		if ( local_status_change ) {
			// Indicate that local information change should
			// be pushed to peers

			lock_mutex( &local_info.lock );

			local_info.local_status_changed = TRUE;

			for ( i = 0; i < ispd_ctl.peer_count; i++ )
				local_info.peer_status[ i ].peer_needs_update = TRUE;

			unlock_mutex( &local_info.lock );

			cv_signal( &monitor_signal_cv, THRD_CONT );
		}

		memset( &event, (int32_t) 0, sizeof(ispd_event_entry_t));
	}

	thread_exit( thread_descr->transient );
	return( (void*) NULL );
}

//
//	Function :
//		check_localhost()
//
//  Arguments       :
//		args			pointer to a thread_args_t structure
//						used by the threads.h framework routines.
//
//	Description :
//		This routine is started as a thread to monitor
//		the health of the iserver on the local host and the
//		network interfaces on the localhost.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
static void *
check_localhost(	void * args )
{
	thread_descr_t	*	thread_descr;
	struct timespec		delay;
	int32_t				timeout;

	thread_descr = thread_register( args );

	for(;;)
	{
		// Check status of gis (iServer), reporting changes in status

		checkIserver();

		// Check network interfaces

		check_network_interfaces();

		// send nullproc call to peers

		check_peers();

		// Setup a delay of 2 seconds

		clock_gettime( CLOCK_REALTIME, &delay );
		incr_timespec( &delay, 2, 0 );


		// Wait for 5 seconds or a cv signal 

		switch(  cv_wait_timed( 	&event_signal_cv,
									( THRD_ABORT | THRD_CONT ),
									&delay,
									&timeout ) )
		{
		case THRD_ABORT:
			trc_debug(	MISPD, NETLOG_DEBUG3,
						"CKEV : THRD_ABORT signaled for check_localhost()\n" );
			cv_signal( &event_signal_cv, THRD_DONE );
			goto check_localhost_exit;
			break;
		case THRD_CONT:
			trc_debug(	MISPD, NETLOG_DEBUG3,
						"CKEV : THRD_CONT signaled for check_localhost()\n" );
			cv_signal( &event_signal_cv, THRD_DONE );
			break;
		default:
			if ( timeout )
			{
/*
				trc_debug(	MISPD, NETLOG_DEBUG3,
						"CKEV : THRD_TIMEOUT signaled for check_localhost()\n" );
*/
			}
			break;
		}
	}

check_localhost_exit:

	thread_exit( thread_descr->transient );
	return( (void*) NULL );
}

//
//	Function :
//		check_peer_iservers()
//
//  Arguments       :
//		None
//
//	Description :
//		This routine checks to see if any of the peer iserver's
//		have an iserver up and ready to rock.
//
//	Return Values:
//		1		if at least one peer has an iserver up
//		0		if no peers are ready for switchover
//
static int
check_peer_iservers( void )
{
	int	i;

	for ( i = 0; i < ispd_ctl.peer_count; i++ )
	{
		if ( peer_info[ i ].iserver_status == ISPD_ISERVER_UP )
			return( 1 );
	}

	return( 0 );
}

//
//	Function :
//		checkIserver()
//
//  Arguments       :
//		None
//
//	Description :
//		This routine called by the check_localhost() thread
//		to determine the health of the iserver on the local
//		host. Changes in health deliver an event to the 
//		ispd_eventloop() thread reflecting the change.
//
//	Return Values:
//		None
//
static void
checkIserver( void )
{
	static char					gispidfile[MAXPATHLEN];
	static int					inited;
	static ispd_event_entry_t	event;
	static int					running_pid = -1;

	// Is this the first time we are being called ?

	if ( !inited )
	{
		// Yes, set up gispidfile static to point to gis pid file. 
		// initialize for subsequent calls.

		sprintf( gispidfile, "%s/%s", PIDS_DIRECTORY, GIS_PID_FILE );

		memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
		event.type = ISPD_EVENT_ISERVER_STATUS;
		inited = 1;
	}

	if ( running_pid < 0 )
		running_pid = ReadPid( gispidfile );

	if ( running_pid > 0 ) {
		// Ping gis pid via kill to see if its there 

		if ( kill( running_pid, 0 ) == 0 )
		{
			switch ( local_info.iserver_status )
			{
			case ISPD_ISERVER_UNKNOWN:
			case ISPD_ISERVER_DOWN:

				local_info.iserver_status = ISPD_ISERVER_UP;
				ispd_q_send( ispd_event_qid, &event );
				break;

			default:
				break;
			}
		}
		else
		{
			switch ( local_info.iserver_status )
			{
			case ISPD_ISERVER_UNKNOWN:
			case ISPD_ISERVER_UP:

				local_info.iserver_status = ISPD_ISERVER_DOWN;
				*p_iserver_state = ISPD_ISERVER_DOWN;
				ispd_q_send( ispd_event_qid, &event );
				break;

			default:
				break;
			}

			running_pid = -1;	// set running pid to -1 so we re-read
								// the pid file to get the new pid.
		}
	}
	else
	{
		switch ( local_info.iserver_status )
		{
		case ISPD_ISERVER_UNKNOWN:
		case ISPD_ISERVER_UP:

			local_info.iserver_status = ISPD_ISERVER_DOWN;
			*p_iserver_state = ISPD_ISERVER_DOWN;
			ispd_q_send( ispd_event_qid, &event );

			break;

		default:
			break;
		}

		running_pid = -1;	// set running pid to -1 so we re-read
							// the pid file to get the new pid.
	}

	return;
}

//
//	Function :
//		checkRsd()
//
//  Arguments       :
//		None
//
//	Description :
//		This routine is called by the check_localhost() thread
//		to determine the health of the rsd daemon on the local
//		host. Changes in health deliver an event to the 
//		ispd_eventloop() thread reflecting the change.
//
//	Return Values:
//		None
//
static void
checkRsd( void )
{
	static char					gispidfile[MAXPATHLEN];
	static int					running_pid = -1;
	static int					inited;
	static ispd_event_entry_t	event;

	// Is this the first time we are being called ?

	if ( !inited )
	{
		// Yes, set up gispidfile static to point to gis pid file. 
		// initialize for subsequent calls.

		sprintf( gispidfile, "%s/%s", PIDS_DIRECTORY, GIS_PID_FILE );

		memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
		event.type = ISPD_EVENT_ISERVER_STATUS;
		inited = 1;
	}

	if ( running_pid < 0 )
		running_pid = ReadPid( gispidfile );

	if ( running_pid > 0 )
	{
		// Ping gis pid via kill to see if its there 

		if ( kill( running_pid, 0 ) == 0 )
		{
			switch ( local_info.iserver_status )
			{
			case ISPD_ISERVER_UNKNOWN:
			case ISPD_ISERVER_DOWN:

				local_info.iserver_status = ISPD_ISERVER_UP;
				ispd_q_send( ispd_event_qid, &event );
				break;

			default:
				break;
			}
		}
		else
		{
			switch ( local_info.iserver_status )
			{
			case ISPD_ISERVER_UNKNOWN:
			case ISPD_ISERVER_UP:

				local_info.iserver_status = ISPD_ISERVER_DOWN;
				ispd_q_send( ispd_event_qid, &event );
				break;

			default:
				break;
			}

			running_pid = -1;	// set running pid to -1 so we re-read
								// the pid file to get the new pid nextime
								// through
		}
	}
	else
	{
		switch ( local_info.iserver_status )
		{
		case ISPD_ISERVER_UNKNOWN:
		case ISPD_ISERVER_UP:

			local_info.iserver_status = ISPD_ISERVER_DOWN;
			ispd_q_send( ispd_event_qid, &event );

			break;

		default:
			break;
		}

		running_pid = -1;	// set running pid to -1 so we re-read
							// the pid file to get the new pid nextime
							// through
	}

	return;
}

//
//	Function :
//		check_network_interfaces()
//
//  Arguments       :
//		None
//
//	Description :
//		This routine is called periodically to check the
//		status of local interfaces on the host on which the
//		caller resides.
//
//	Return Values:
//		None.
//
static void
check_network_interfaces( void )
{
	struct ifi_info	*	ifi_ptr;
	ispd_event_entry_t	event;
	interface_status_t	link_status;

	lock_mutex( &local_info.lock );

	if ( ifihead != NULL )
	{
		free_iface_info( ifihead );
	}

	ifihead = get_iface_info( AF_INET, 1 );

	unlock_mutex( &local_info.lock );

	if ( ispd_ctl.defined )
	{
		if ( ( ifi_ptr = findIfByIfname(	ifihead,
											ispd_ctl.name ) ) != NULL )
		{
			switch( local_info.ctl_interface_status )
			{
			case ISPD_IFACE_STATUS_UNKNOWN:

				// Initial setting

				if ( ifi_ptr->ifi_flags & IFF_UP )
				{
					local_info.ctl_interface_status = ISPD_IFACE_STATUS_UP;
				}
				else
				{
					local_info.ctl_interface_status = ISPD_IFACE_STATUS_DOWN;
				}
				break;
			case ISPD_IFACE_STATUS_DOWN:

				// Has status of control interface status changed ?

				if ( ifi_ptr->ifi_flags & IFF_UP )
				{
					// Yes, send an event to ispd_eventloop()

					memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
					event.type = ISPD_EVENT_CTL_IFACE_STATUS;
					(interface_status_t) event.value = ISPD_IFACE_STATUS_UP;
					ispd_q_send( ispd_event_qid, &event );
				}
				break;
			case ISPD_IFACE_STATUS_UP:

				// Has status of control interface status changed ?

				if ( !(ifi_ptr->ifi_flags & IFF_UP) )
				{
					// Yes, send an event to ispd_eventloop()

					memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
					event.type = ISPD_EVENT_CTL_IFACE_STATUS;
					(interface_status_t) event.value = ISPD_IFACE_STATUS_DOWN;
					ispd_q_send( ispd_event_qid, &event );
				}
				break;
			}

			link_status = get_link_status(	local_info.ctl_devname,
							 				local_info.ctl_instance );

			switch( local_info.ctl_link_status )
			{
			case ISPD_IFACE_STATUS_UNKNOWN:

				// Initial setting

				if ( link_status == ISPD_IFACE_STATUS_UP )
				{
					local_info.ctl_link_status = ISPD_IFACE_STATUS_UP;
				}
				else
				if ( link_status == ISPD_IFACE_STATUS_DOWN )
				{
					local_info.ctl_link_status = ISPD_IFACE_STATUS_DOWN;
				}
				break;
			case ISPD_IFACE_STATUS_DOWN:

				// Has status of control link status changed ?

				if ( link_status == ISPD_IFACE_STATUS_UP )
				{
					// Yes, send an event to ispd_eventloop()

					memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
					event.type = ISPD_EVENT_CTL_IFACE_LINK_STATUS;
					(interface_status_t) event.value = ISPD_IFACE_STATUS_UP;
					ispd_q_send( ispd_event_qid, &event );
				}
				break;
			case ISPD_IFACE_STATUS_UP:

				// Has status of control link status changed ?

				if ( link_status == ISPD_IFACE_STATUS_DOWN )
				{
					// Yes, send an event to ispd_eventloop()

					memset( &event, (int32_t) 0, sizeof( ispd_event_entry_t ) );
					event.type = ISPD_EVENT_CTL_IFACE_LINK_STATUS;
					(interface_status_t) event.value = ISPD_IFACE_STATUS_DOWN;
					ispd_q_send( ispd_event_qid, &event );
				}
				break;
			}
		}
		else
		{
			trc_error(	MISPD, 
						"CFGE : Control interface, \"%s\", "
						"not defined - config error - exiting\n",
						ispd_primary.name );
			cv_signal( &ispd_main_cv, THRD_DONE );
		}
	}
}

//
//	Function :
//		get_link_status()
//
//  Arguments       :
//
//		devname		device name of global clone device
//
//		instance	instance of global clone device we
//					want the link status for
//
//	Description :
//		This routine is called periodically to get the link
//		status of local interfaces on the host on which the
//		caller resides.
//
//	Return Values:
//		interface_status_t value :
//
//			ISPD_IFACE_STATUS_UP	-  link status UP - cable connected
//			ISPD_IFACE_STATUS_DOWN	-  link status DOWN - no cable
//
static interface_status_t
get_link_status( char * devname,
                 int    instance )
{
    char    		command[128];
    char    		buf[ 128 ];
    char    		temp_devname[ 128 ];
	struct	stat	stat_buf;
    int    			cmdfd;
	int				value;
	int				rc;

	if ( !strcmp( devname, "/dev/e1000g" ) )
	{
#define E1000G_LINK_STATUS  _IOWR( 'r', 101, struct e1000g_link_status * )	

		typedef struct  e1000g_link_status
		{
			int32_t     instance;
			int32_t     link_status;
		} e1000g_link_status_t;

		struct e1000g_link_status	gls_struct;
		struct strioctl				ioc_desc;
		static	int32_t				e1000gfd = -1;



		memset( &ioc_desc,
				(int32_t) 0,
				sizeof( struct strioctl ) );
		memset( &gls_struct,
				(int32_t) 0,
				sizeof( e1000g_link_status_t ) );

		ioc_desc.ic_cmd = E1000G_LINK_STATUS;
		ioc_desc.ic_len = sizeof( e1000g_link_status_t );
		ioc_desc.ic_dp = (void*) &gls_struct;

		gls_struct.instance = instance;

		if ( e1000gfd < 0 )
		{
			if ( ( e1000gfd = open( "/dev/e1000g", O_RDWR )) < 0 )
			{
				trc_error(	MISPD,
							"LNK :  unable to open \"%s\" device\n",
							"/dev/e1000g" );
				return( ISPD_IFACE_STATUS_DOWN );
			}
		}

		if ( ioctl( e1000gfd, I_STR, &ioc_desc ) < 0 )
		{
			int32_t	kerrno = errno;

			trc_error(	MISPD,
						"LNK :  ioctl failed for \"%s\" device, instance %d - errno %d\n",
						"/dev/e1000g",
						instance,
						kerrno );
			return( ISPD_IFACE_STATUS_DOWN );
		}
		else
		{
			if ( gls_struct.link_status )
				return( ISPD_IFACE_STATUS_UP );
			else
				return( ISPD_IFACE_STATUS_DOWN );
		}
	}
	else
	{

		// Construct "/dev/{devname}{instance}" name for file
		// to see if this device uses real device instances for
		// sub devices. The dmfe device on Netra X1s is of this
		// ilk. It must be treated differently when using ndd to
		// discern the link_status.

		sprintf( temp_devname, "%s%1d", devname, instance );

		// Is this a special type of device - ala - dmfe ilk ?

		if ( 	( ( stat( temp_devname, &stat_buf )) == 0 )	&&
				S_ISCHR( stat_buf.st_mode ) )
		{
			// Yes, simply issue the link status command against
			// the sub device name.

			sprintf(    command,
						"/usr/sbin/ndd %s link_status",
						temp_devname );

			trc_debug(	MISPD, NETLOG_DEBUG2,
						"LNK :  %s\n", command );
		}
		else
		{
			// No, set the instance number for which we want link
			// status to the instance number of the sub device via
			// the global clone name of the device.

			sprintf(    command,
						"/usr/sbin/ndd -set %s instance %d",
						devname,
						instance );

			if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
			{
				sys_pclose( cmdfd );
			}

			// Now, issue the link status command for the global
			// clone name of the device.

			sprintf(    command,
						"/usr/sbin/ndd %s link_status",
						devname );

			trc_debug(	MISPD, NETLOG_DEBUG2,
						"LNK :  %s\n", command );
		}

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
		{
			rc = read( cmdfd, buf, 128 );
			sys_pclose( cmdfd );
		}

		return( (value = atoi( buf ))?
					ISPD_IFACE_STATUS_UP : ISPD_IFACE_STATUS_DOWN );
	}
}

//
//	Function :
//		check_peers()
//
//  Arguments       :
//		None
//
//	Description :
//			This routine is called from check_localhost()
//		to ping peers via a nullproc rpc call if we do
//		not already have a connection to a peer.
//
//	Return Values:
//		None
//
static void
check_peers( void )
{
	int					i, rc;

	for ( i = 0; i < ispd_ctl.peer_count; i++ )
	{
		rc = ispd_nullproc( &peer_info[ i ] );
	}

	return;
}

//
//	Function :
//		ispd_initialize_exit()
//
//  Arguments       :
//		None
//
//	Description :
//			This routine is called to gracefully exit ispd
//		during initialization.
//
//	Return Values:
//		None
//
static void
ispd_initialize_exit( void )
{
	char	description[ MAX_DESCR_SIZE ];

	#ifdef _DEBUG_MALLOC_
		int		fd;
	#endif

	// Start up a thread to service rpc server request that
	// started us

	sprintf( description, "rpc_server_thread()" );

	thread_create(  &rpc_server_thread,
					ispd_rpcsvc_init,
					(void*) NULL,
					description,
					0 );


	millisleep( 5000 );

	svc_exit();

	millisleep( 1000 );

	#ifdef _DEBUG_MALLOC_
		current_size = malloc_inuse( &histid2 );
		fd = open( "malloc.inuse", O_CREAT | O_RDWR );
		malloc_list( fd, histid1, histid2 );
		close( fd );
	#endif

	trc_error(	MISPD,
				"INIT : exiting\n" );

}

//
//	Function :
//		restart_ispd()
//
//  Arguments       :
//		None
//
//	Description :
//			This routine is called to restart the local ispd
//		daemon
//
//	Return Values:
//		None
//
static void
restart_ispd( void )
{
	pid_t	pid;

	if ( ( pid = fork() ) < 0 )
	{
		perror( "restart_ispd() : Error in forking:" );
		return;
	}
	else if ( pid != 0 )
	{
		// Parent

		trc_error(	MISPD,
					"restart_ispd() : started pid %d for restart\n",
					pid );
		return;	// Parent returns
	}

	trc_error( MISPD, "restart_ispd() : child pid %d - sleeping...\n", getpid() );

	// Child chugs on ...

	setpgrp();

	setsid();

	umask(0);

	// Setup process wide signal handling

	SignalInit();

	trc_error( MISPD, "restart_ispd() : sleeping...\n" );

	millisleep( 2000 );

	trc_error( MISPD, "restart_ispd() : starting ispd...\n" );

	ispd_nullproc_localhost();

	trc_error( MISPD, "restart_ispd() : exiting\n" );

	return;
}


static	int32_t prio_inited = 0;



//
//  Function    :
//      ispd_routing_change()
//
//  Arguments   :
//      action  	enum defining situation for which
//                  change_loopback_route() is being
//                  called. Either :
//						ISPD_VIP_UP		- vip being brought up
//						ISPD_VIP_DOWN	- vip being brought down
//
//  Description :
//
//      This routine is called to make routing changes
//      routing changes for a specified network
//		interface. Different actions are take place depending
//      on the action specified.
//
//  Return values:
//      None.
//
static void
ispd_routing_change(	ispd_route_action_t	action,
						ispd_interface_t *	iface_desc )
{
	char    command[128];
	int    	cmdfd;

	switch ( action )
	{
	case	ISPD_VIP_DOWN:

		// Remove route for primary vip to loopback

		sprintf(    command,
					"/usr/sbin/route delete %s 127.0.0.1 0",
					iface_desc->vip );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
		{
			sys_pclose( cmdfd );
		}

		// Add route for primary interface to loopback

		sprintf(    command,
					"/usr/sbin/route add %s 127.0.0.1 0",
					iface_desc->ip );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
		{
			sys_pclose( cmdfd );
		}
		break;

	case	ISPD_VIP_UP:

		// Remove route for primary interface to loopback

		sprintf(    command,
					"/usr/sbin/route delete %s 127.0.0.1 0",
					iface_desc->ip );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
		{
			sys_pclose( cmdfd );
		}

		// Add route for primary vip to loopback

		sprintf(    command,
					"/usr/sbin/route add %s 127.0.0.1 0",
					iface_desc->vip );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
		{
			sys_pclose( cmdfd );
		}
		break;
	}
}

void
recycle_virtifs( ispd_route_action_t	action, int call )
{
	trc_error(	MISPD,
				"ACTN :  recycle_virtifs(%2d) actions %s\n",
				call,
				( action == ISPD_VIP_UP )?"VIPS_UP  ":"VIPS_DOWN" );

	// Do what needs to be done to vips based on action input

	if ( action == ISPD_VIP_DOWN ) { 

		local_info.send_status_gis = TRUE;
		local_info.send_status_dbsync = FALSE;

		// Issue command to down remaining logical interfaces

		{
			char    command[128];
			int    	cmdfd;


			sprintf(    command,
						"/usr/local/nextone/bin/ifmgmt down" );

			if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
				sys_pclose( cmdfd );
			}
		}
	} else if ( action == ISPD_VIP_UP ) { 

		local_info.send_status_gis = TRUE;
		local_info.send_status_dbsync = FALSE;

		// Issue command to up remaining virtual interfaces

		{
			char    command[128];
			int    	cmdfd;


			sprintf(    command,
						"/usr/local/nextone/bin/ifmgmt up" );

			if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
				sys_pclose( cmdfd );
			}
		}
	}
}

/*
   initialize a shared memory variable to maintain the state
*/
int init_state()
{
	int shmId;
	shmId = shmget(ISERVER_STATE_SHM_KEY, sizeof(int), SHM_R | SHM_W | IPC_CREAT);
	if(shmId < 0)
	{
		trc_error(MISPD, "shmget() failed. error code %d\n", errno );
		return -1;
	}
	p_iserver_state = shmat(shmId, (void *)0, SHM_R | SHM_W | SHM_RND);
	if( p_iserver_state <= 0)
	{
		trc_error(MISPD, "shmat() failed. error code %d\n", errno );
		return -1;
	}
	*p_iserver_state = ISPD_ISERVER_UNKNOWN;

	return 0;
}
