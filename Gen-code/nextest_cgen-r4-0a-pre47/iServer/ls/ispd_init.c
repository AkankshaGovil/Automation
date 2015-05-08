// System include files

#define STRSZ			128

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>

#if SOLARIS_REL == 7
	#include <addrinfo.h>
#endif

#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sched.h>
#include <rpc/rpc.h>
#include <libgen.h>

// iServer include files

#include <spversion.h>
#include <generic.h>
#include <bits.h>
#include <ipc.h>
#include <srvrlog.h>
#include <lsconfig.h>
#include <serverdb.h>
#include <key.h>
#include <protos.h>
#include <serverp.h>
#include <pids.h>
#include <ifs.h>
#include <db.h>
#include <timer.h>
#include <connapi.h>
#include <sconfig.h>
#include <xmltags.h>
#include <malloc.h>
#include <firewallcontrol.h>
#include "gis.h"
#include "nxosd.h"
#include "nxioctl.h"

pthread_t	gis_rpcsvc_thread;

extern	char	fceConfigFwName[ 128 ];
extern int IserverVipStatus(int);

typedef struct _popen_list
{
	int fd;
	pid_t child_pid;
	struct _popen_list *next;
} popen_list_t;

pthread_mutex_t			popen_lock;

extern	void *	gis_rpcsvc_init( void * args );
//extern	void	tweak_ipfilter_ipaddr(	char *primary_vip,
//										char *secondary_vip );

static popen_list_t *	popen_chain;

static void		lock_mutex( pthread_mutex_t * lock );
static void		unlock_mutex( pthread_mutex_t * lock );
static void		millisleep( uint32_t milliseconds );
static void		gis_callback(	int status,
								char *primary_vip,
								char *secondary_vip );
static void		conf_inetd( void );
static void		ping_local_ispd( void );

#define write_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLK, F_WRLCK, offset, whence, len )

#define un_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLK, F_UNLCK, offset, whence, len )

//
//	Function :
//		lock_reg()
//
//  Arguments       :
//		fd			file descriptor to which locking operation should
//					be applied.
//
//		cmd			F_SETLK or F_SETLKW
//
//		type		F_RDLCK, F_WRLCK or F_UNLCK
//
//		offset		starting offset relative to whence setting
//
//		whence		SEEK_SET (absolute), SEEK_CUR (relative to current) or
//					SEEK_END (relative to end of file)
//
//		len			length of locking operation
//
//	Description :
//		File locking and unlocking primitive
//
//	Return Values:
//		 fcntl() return code
//
static int
lock_reg(	int		fd,
			int		cmd,
			int		type,
			off_t	offset,
			int		whence,
			off_t	len )
{

	struct	flock	lock;

	lock.l_type = type;			// F_RDLCK, F_WRLCK, F_UNLCK
	lock.l_start = offset;		// byte offset, relative to l_whence
	lock.l_whence = whence;		// SEEK_SET, SEEK_CUR, SEEK_END
	lock.l_len = len;			// # of bytes ( 0 means to EOF )

	return( fcntl( fd, cmd, &lock ) );
}

//
//	Function :
//		start_ispd()
//
//  Arguments       :
//		None
//
//
//	Description :
//		This function checks on ispd configuration and starts
//		the ispd on the local host if it is configured in the
//		server.cfg file.
//
//	Return Values:
//		char **		pointer to an argument list array for use
//					with execv.
//
void
start_ispd( void )
{
	char			command[128];
	int				cmdfd;
	pthread_attr_t	thread_attr;
	int32_t			status;

	// Is an ispd configuration defined in the server.cfg ?

	if ( iserver == NULL || iserver->ispd.location.type == CONFIG_LOCATION_NONE || ispd_type == ISPD_TYPE_DISABLED )
	{
        GisExecuteCmd(GIS_REALM_UP_ALL_CMDID);
		return;
    }

	//
	// code to prevent multiple copies of ispd on one host.
	// Uses advisory locking scheme to insure locking
	// ala Stevens - Advanced Unix Programming - Page 372
	//
	// In this case we want to insure we dont call nullproc
	// on a server that already has an ispd running.
	//

	{
		char	pidfile[MAXPATHLEN];
		char	pid_buf[10];
		int		fd_val;
		int		ispd_fd;

		// Make sure ispd is not already running

		sprintf( pidfile, "%s/%s", "/var/run", ISPD_PID_FILE );

		if ( ( ispd_fd = open( pidfile, O_WRONLY | O_CREAT, 0644 )) < 0 )
		{
			NETDEBUG(	MDEF, NETLOG_DEBUG4,
						("INIT : Error opening pid file, %s\n",
						pidfile ));
		}
		else
		{
			// Try and look the file to see if an active ispd is here on
			// localhost

			if ( write_lock( ispd_fd, 0, SEEK_SET, 0 ) < 0 )
			{
				NETDEBUG(	MDEF,	NETLOG_DEBUG2,
							("INIT : ispd already running exiting\n" ));
				//
				// Startup a thread as an RPC server for the ISPD_PROG 
				// program number and GIS_VERS version number. The client
				// of the service is the local ispd daemon. The service will
				// be called when ever the status of the box changes.
				// A callback routine gis_callback() local to this c file
				// is registered to handle data passed to it. It is defined below.
				//

				pthread_attr_init( &thread_attr );

				pthread_attr_setdetachstate( &thread_attr,
											 PTHREAD_CREATE_DETACHED );

				pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );

				if ( (status = pthread_create(	&gis_rpcsvc_thread,
												&thread_attr,
												gis_rpcsvc_init,
												(void*) gis_callback ) ) != 0 )
				{
					NETERROR(	MDEF,
								("start_ispd() : error failed to start "
								"gis_rpcsvc_init() - status %d\n",
								status ));
				}
				return;
			}
			else
			{
				un_lock( ispd_fd, 0, SEEK_SET, 0 );
			}

			close( ispd_fd );
		}
	}

	sys_utils_init();			// Initialize stuff for sys_popen()

	// Add stuff to

	conf_inetd();

	//
	// Startup a thread as an RPC server for the ISPD_PROG program number
	// and GIS_VERS version number. The client of the service is the
	// local ispd daemon. The service will be called when ever the status
	// of the box changes. A callback routine gis_callback() local to this
	// c file is registered to handle data passed to it. It is defined below.
	//

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
	pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );

	if ( (status = pthread_create(	&gis_rpcsvc_thread,
							&thread_attr,
							gis_rpcsvc_init,
							(void*) gis_callback ) ) != 0 )
	{
		NETERROR(	MDEF,
					("start_ispd() : error failed to start "
						"gis_rpcsvc_init() - status %d\n",
					status ));
	}

	// Build command to ping nullproc of local rpc service ispd
	// to bring up local ispd

	ping_local_ispd();

	return;
}

//
//	Function :
//		restart_ispd()
//
//  Arguments       :
//		None
//
//
//	Description :
//		This function checks on ispd configuration and starts
//		the ispd on the local host if it is configured in the
//		server.cfg file.
//
//	Return Values:
//		None
//
void
restart_ispd( void )
{
	char			command[128];
	int				cmdfd;
	pthread_attr_t	thread_attr;
	int32_t			status;

	// Is an ispd configuration defined in the server.cfg ?

	if ( iserver == NULL )
		return;

	if ( iserver->ispd.location.type == CONFIG_LOCATION_NONE)
		return;		// No, simply return - do nothing

	//
	// code to prevent multiple copies of ispd on one host.
	// Uses advisory locking scheme to insure locking
	// ala Stevens - Advanced Unix Programming - Page 372
	//
	// In this case we want to insure we dont call nullproc
	// on a server that already has an ispd running.
	//

	{
		char	pidfile[MAXPATHLEN];
		char	pid_buf[10];
		int		fd_val;
		int		ispd_fd;

		// Make sure ispd is not already running

		sprintf( pidfile, "%s/%s", PIDS_DIRECTORY, ISPD_PID_FILE );

		if ( ( ispd_fd = open( pidfile, O_WRONLY | O_CREAT, 0644 )) < 0 )
		{
			NETERROR(	MDEF,
						("INIT : Error opening pid file, %s\n",
						pidfile ));
		}

		// Is another ispd running?

		if ( write_lock( ispd_fd, 0, SEEK_SET, 0 ) < 0 )
		{
			// Yes, it has the lock

			if ( errno == EACCES || errno == EAGAIN )
			{
				// Send it a HUP to make it restart!!

				sprintf(	command,
							"/bin/pkill -HUP ispd" );

				// Issue command to restart ispd

				NETERROR(	MDEF,
							("INIT : restart_ispd() - restarting ispd\n" ));

				if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 )
				{
					sys_pclose( cmdfd );
				}
				return;
			}
			else
			{
				NETERROR(	MDEF,
							("INIT : write_lock() error on file, %s\n",
							pidfile ));
				return;
			}
		}

		close( ispd_fd );
	}

	// If the ispd is disabled non of the stuff below needs to be done

	if ( ispd_type == ISPD_TYPE_DISABLED )
		return;		// No, simply return - do nothing

	//
	// Startup a thread as an RPC server for the ISPD_PROG program number
	// and GIS_VERS version number. The client of the service is the
	// local ispd daemon. The service will be called when ever the status
	// of the box changes. A callback routine gis_callback() local to this
	// c file is registered to handle data passed to it. It is defined below.
	//

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
	pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );

	if ( (status = pthread_create(	&gis_rpcsvc_thread,
							&thread_attr,
							gis_rpcsvc_init,
							(void*) gis_callback ) ) != 0 )
	{
		NETERROR(	MDEF,
					("start_ispd() : error failed to start "
						"gis_rpcsvc_init() - status %d\n",
					status ));
	}

	// Build command to ping nullproc of local rpc service ispd
	// to bring up local ispd

	ping_local_ispd();

	return;
}

static	int	first_time = 1;
static	int	previous_local_vip_status = -1;		// Starts as unknown (-1)

//
//	Function :
//		gis_callback()
//
//  Arguments       :
//		new_local_vip_status	integer containing status of vip(s) on localhost
//								0   "means"  INACTIVE
//								1   "means"  ACTIVE
//
//      primary_vip				character string containing ip address of
//								primary vip. Primary vip equates to public
//								interface.
//
//      secondary_vip			character string containing ip address of
//								secondary vip. Secondary vip equates to private
//								interface.
//
//	Description :
//		This function is a registered callback which get called
//		by local ispd to reflect changes to the status of the machine
//		regarding whether it is an active iserver or not and which
//		vips it is servicing.
//
//	Return Values:
//		None
//
static void	
gis_callback(	int new_local_vip_status,
				char *primary_vip,
				char *secondary_vip )
{
	static	int	gis_status_counter;

	// Has local vip status changed ?

	if ( previous_local_vip_status != new_local_vip_status )
	{
		// Yes, Report local vip status change

		//	NETDEBUG(	MDEF, NETLOG_DEBUG3,
		//				("     local vip status is %-9s (%d)\n",
		//				( status == 0 )? "INACTIVE" : "ACTIVE",
		//				gis_status_counter++ ));

		NETERROR(	MDEF,
					("     local vip status is %-9s (%d)\n",
					( new_local_vip_status == 0 )? "INACTIVE" : "ACTIVE" ,
					gis_status_counter++ ));

		// indicate to the FCE layer that some switchover has occurred

		if (new_local_vip_status) {
			FCEServerActive();
		} else {
			FCEServerInactive();
		}

		// Indicate next INACTIVE call should call BridgeEndCalls()

		first_time = 0;

		previous_local_vip_status = new_local_vip_status;
		IserverVipStatus((new_local_vip_status == 0)?0:1);
	}

	return;
}

//
//	Function		:
//		lock_mutex()
//
//	Arguments		:
//		lock			a pointer to an initialized mutex
//						to lock.
//
//	Description		:
//		Locks a mutex for a thread.
//
//	Return value	:
//		None
//
static void
lock_mutex( pthread_mutex_t * lock )
{
	int	status;

	for (;;)
	{
		if ( (status = pthread_mutex_trylock( lock )) == 0 )
			break;
		else 
		if ( status != EBUSY )
		{
			NETERROR( 	MISPD,
						("ispd_init.c : mutex lock error\n" ));
		}

		millisleep( 500 );
	}
}

//
//	Function		:
//		unlock_mutex()
//
//	Arguments		:
//		lock			a pointer to an initialized mutex
//						which is locked.
//
//	Description		:
//		Unlocks a locked mutex for a thread.
//
//	Return value	:
//		None
//
static void
unlock_mutex( pthread_mutex_t * lock )
{
	int	status;

	if ( ( status = pthread_mutex_unlock( lock ) ) != 0 )
	{
		NETERROR( 	MISPD,
					("ispd_init.c : mutex unlock error\n" ));
	}
}

//
//	Function		:
//		millisleep()
//
//	Arguments		:
//
//		milliseconds	# of milliseconds to sleep
//
//	Purpose			:
//		Use nanosleep() to sleep for a number of 
//		milliseconds.
//
//	Description		:
//		This function is given an integer containing
//		the number of milliseconds to sleep. 
//
//	Return value	:
//		None
//
static void
millisleep( uint32_t milliseconds )
{
	struct	timespec	delay;
	struct	timespec	remaining;
	int32_t				retval;
	extern	int			errno;

	if ( milliseconds )
	{
		delay.tv_sec = milliseconds/1000;
		delay.tv_nsec = (milliseconds%1000)*1000000;
	}
	else
		return;

	// Increment timespec_t by # of milliseconds

	while ( (retval = nanosleep( &delay, &remaining )) )
	{
		if ( errno == EINTR )
		{
			delay = remaining;
			continue;
		}

		if ( errno == EINVAL )
		{
			break;
		}
	}
}

//
//	Function :
//		conf_inetd()
//
//  Arguments       :
//		None
//
//
//	Description :
//		This function adds the appropriate entries to
//		inetd.conf and rpc in the safest way.
//
//	Return Values:
//		None
//
static void
conf_inetd( void )
{
	FILE *			cmdfile;

	// The following command appends appropriate lines to
	// inetd.conf and rpc in etc if they are not already there
	// and tells inetd to reread the input files. only works
	// properly with the real popen.

	char *	command = 
		"/bin/sh -c \'"
			"PATH=/usr/bin:/sbin:/bin;"
			"grep \"^ispd\" /etc/inetd.conf >/dev/null 2>&1;"
			"if [ $? != 0 ]; then"
			"	echo \"ispd/1\ttli\trpc/tcp\twait\troot"
			"\t/usr/local/nextone/bin/ispd\tispd\"	>>/etc/inetd.conf;"
			"fi;"
			"grep \"^ispd\" /etc/rpc >/dev/null 2>&1;"
			"if [ $? != 0 ]; then"
			"	echo \"ispd\t540000001\" >>/etc/rpc;"
			"fi;"
			"pkill -HUP -x inetd;\'";

	//
	// Issue command to configure inetd for ispd and
	// tell inetd to re-read configuration files.
	//

	if ( ( cmdfile = popen( command, "r" ) ) >= 0 )
	{
		pclose( cmdfile );
	}

	millisleep(1000);
}

static void
ping_local_ispd( void )
{
	CLIENT *		cl;
	char			buffer[512];
	char            description[255];
	char *          error;
	struct timeval  timeOut;
	struct rpcent	rentry;		
	struct rpcent*	rptr;		
	int				rc;
	struct timeval	timeout = { 3, 0 };

	// Set default timeout for client handle, cl, to 4 seconds

	memset( &timeOut, (int) 0, sizeof( struct timeval ) );

	timeOut.tv_sec = 4;

	if ( (rptr = nx_getrpcbyname_r(	"ispd",
									&rentry,
									buffer,
									512 )) == (struct rpcent*) 0 )
	{
		NETERROR(	MISPD,
					("RPC  : ispd rpc service not found\n" ));
		return;
	}

	//
	// Create client "handle" used for calling
	// MESSAGEPROG on the serverdesignated on
	// command line. We tell RPC package
	// to use the "tcp" protocol when contacting
	// the server.
	//

	if ( ( cl = clnt_create_timed(  "localhost",
									rptr->r_number,
									1,
									"tcp",
									&timeOut ) ) == NULL )
	{
		// Couldn't establish connection with the server

		if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
		{
			error = clnt_spcreateerror( "localhost" );
			NETERROR(	MISPD,
						("RPC  : Client : %s\n", error ));
		}
		return;
	}

	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
	{
		sprintf(	description,
					"Registered with RPC Server, \"%s\"",
					"localhost" );

		NETERROR(	MISPD,
					("RPC  : Client : %s\n", description ));
	}


	if ( clnt_control( cl, CLSET_TIMEOUT, (char*) &timeOut ) == FALSE )
	{
		sprintf(    description,
					"can't set timeout value to 4 seconds\n" );
		NETERROR(	MISPD,
					("RPC  : Client : %s\n", description ));
	}

	// Call the remote procedure on the server

	if ( (rc = (int) clnt_call( cl,
   								NULLPROC,
								(xdrproc_t) xdr_void,
								(caddr_t) NULL,
								(xdrproc_t) xdr_void,
								(caddr_t) NULL,
								timeout )) != RPC_SUCCESS )
	{
		NETERROR(	MISPD,
					("RPCE : failed nullproc to \"%s\" - rc %d\n",
					"localhost",
					rc ));
	}

	if ( cl != ( CLIENT* ) NULL )
		clnt_destroy( cl );

	return;
}
