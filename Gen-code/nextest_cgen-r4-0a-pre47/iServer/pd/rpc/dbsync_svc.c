//
//	File:
//
//		dbsync_svc.c
//
//	Description:
//
//		This file contains the server initialization
//		functions for the ISPD_PROG and DBSYNC_VERS.
//      These functions are used to initialize the
//      rpc server within a running database
//		sycronization daemon.
//

#include "ispd_rpc.h"
#include <ispd.h>
#include <rpc/pmap_clnt.h>		/* for pmap_unset */
#include <memory.h>
#include <sys/resource.h>		/* rlimit */
#include <pthread.h>
#include <signal.h>
#include <rpc/rpc.h>
#include "nxosd.h"

int32_t			trace_disk  = 1;
int32_t			trace_initialized;
FILE *			trace_fdesc;
int32_t			trace_fd;
char			trace_log[ 256 ];

trace_table_t   _trace_tbl;
char 			progname[MAXPATHLEN];

#ifndef SIG_PF
	#define	SIG_PF void(*)(int)
#endif

#ifdef DEBUG
	#define	RPC_SVC_FG
#endif

#define	_RPCSVC_CLOSEDOWN 120
int _rpcpmstart;			/* Started by a port monitor ? */

//
// States a server can be in wrt request 
//

#define	_IDLE 0
#define	_SERVED 1

extern void ispd_prog_3(struct svc_req *rqstp, register SVCXPRT * transp);

static SVCXPRT *	transp;

pthread_mutex_t 			_svcstate_lock;	// lock for _rpcsvcstate, _rpcsvccount

static void
_msgout(char *msg)
{
	NETERROR( MISPD, ( "RPC  : GIS ISPD Server : %s\n", msg ) );
}

//
//	Function :
//		dbsync_rpcsvc_init()
//
//  Arguments       :
//
//		args			address of callback routine to be called
//						on status changes
//
//	Description :
//			This routine is started as a thread at gis
//		initialization time. dbsync_rpcsvc_init() initializes
//		RPC services for the ISPD_PROG, DBSYNC_VERS service within
//      the dbsync process. After RPC services	are initialized the
//      thread drops into an eventloop in svc_run() where it
//      spends the rest of its life	servicing RPC calls from the
//      local ispd daemon on this host.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
void *
dbsync_rpcsvc_init( void * args )
{

	pthread_mutex_init(&_svcstate_lock, NULL);

	(void) sigset(SIGPIPE, SIG_IGN);

	//
	// If stdin looks like a TLI endpoint, we assume
	// that we were started by a port monitor. If
	// t_getstate fails with TBADF, this is not a
	// TLI endpoint.
	//

#ifndef NETOID_LINUX
	if (t_getstate(0) != -1 || t_errno != TBADF)
	{
		char *				netid;
		struct netconfig *	nconf = NULL;
		int					pmclose;

		_rpcpmstart = 1;

		if ((netid = getenv("NLSPROVIDER")) == NULL)
		{
			// started from inetd 

			pmclose = 1;
		}
		else
		{
			if ((nconf = getnetconfigent(netid)) == NULL)
				_msgout("cannot get transport info");

			pmclose = (t_getstate(0) != T_DATAXFER);
		}

		if ((transp = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL)
		{
			_msgout("cannot create server handle");

			return( (void*) NULL );
		}

		if (nconf)
			freenetconfigent(nconf);

		if (!svc_reg( transp, ISPD_PROG, DBSYNC_VERS, ispd_prog_3, 0))
		{
			_msgout("unable to register ( ISPD_PROG, DBSYNC_VERS ).");

			return( (void*) NULL );
		}

		svc_run();

		_msgout("service : svc_run returned");

		return( (void*) NULL );

		// NOTREACHED 
	}
	else
#endif /* NETOID_LINUX */
	{
		if(!nx_svcudp_create(ispd_prog_3, ISPD_PROG, DBSYNC_VERS))
		{
			_msgout("unable to create (ISPD_PROG, DBSYNC_VERS) for udp.");
			exit(1);
		}
		
		svc_run();

		_msgout("daemon : svc_run returned");

		return( (void*) NULL );

		// NOTREACHED 

	}
}

//
//	Function :
//		dbsync_rpcsvc_shutdown()
//
//  Arguments       :
//
//		None.
//
//	Description :
//			Called to shutdown rpc services. Remains to
//		be implemented cleanly.
//
//	Return Values:
//		None.
//
void
dbsync_rpcsvc_shutdown( void )
{
	return;
}
