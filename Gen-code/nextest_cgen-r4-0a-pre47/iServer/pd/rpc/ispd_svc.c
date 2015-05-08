//
//	File:
//
//		ispd_svc.c
//
//	Description:
//
//		This file contains the server initialization
//		functions for the ISPD_PROG and ISPD_VERS.
//      These functions are used to initialize the
//      rpc server within a running ispd,
//		( iServer Peering Daemon ).
//

#include "ispd_rpc.h"
#include <ispd.h>
#include <rpc/pmap_clnt.h>		/* for pmap_unset */
#include <memory.h>
#include <sys/resource.h>		/* rlimit */
#include <signal.h>
#include <rpc/rpc.h>
#include "nxosd.h"

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

extern void ispd_prog_1(struct svc_req *rqstp, register SVCXPRT * transp);

static SVCXPRT *	transp;

pthread_mutex_t 			_svcstate_lock;	// lock for _rpcsvcstate, _rpcsvccount

//
//	Function :
//		ispd_rpcsvc_init()
//
//  Arguments       :
//
//		args			pointer to a thread_args_t structure
//						used by the threads.h framework routines.
//
//	Description :
//			This routine is started as a thread at ispd
//		initialization time. ispd_rpcsvc_init() initializes
//		RPC services for the ispd process. After RPC services
//		are initialized the thread drops into an eventloop
//		in svc_run() where it spends the rest of its life
//		servicing RPC calls from peer ispd daemons on other
//		iServer hosts.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
void *
ispd_rpcsvc_init( void * args )
{
	thread_descr_t *thread_descr;

	thread_descr = thread_register( args );

	pthread_mutex_init(&_svcstate_lock, NULL);

	(void) sigset( SIGPIPE, SIG_IGN );

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

		_rpcpmstart = 1;

		trc_error( MISPD, "RPCS : registering ISPD_VERS - TLI\n" );

		if ( (netid = getenv("NLSPROVIDER")) == NULL )
		{
			// started from inetd 
		}
		else
		{
			// not started via inetd

			if ((nconf = getnetconfigent(netid)) == NULL)
			{
				trc_error(	MISPD,
					"RPCS : Error - cannot get transport info\n" );
			}
		}

		if ((transp = svc_tli_create( 0, nconf, NULL, 0, 0 ) ) == NULL )
		{
			trc_error(	MISPD,
						"RPCS : Error - cannot create server handle\n");

			thread_exit(thread_descr->transient);
			return( (void*) NULL );
		}

		if (nconf)
			freenetconfigent(nconf);

		if ( !svc_reg( transp, ISPD_PROG, ISPD_VERS, ispd_prog_1, 0 ) )
		{
			trc_error(	MISPD,
						"RPCS : Error - unable to register "
						"( ISPD_PROG, ISPD_VERS )\n" );

			thread_exit(thread_descr->transient);
			return( (void*) NULL );
		}

		svc_run();

		trc_error(	MISPD,
					"RPCS : ISPD_VERS service exiting\n");

		//svc_unreg( ISPD_PROG, ISPD_VERS );

		thread_exit(thread_descr->transient);
		return( (void*) NULL );

		// NOTREACHED 
	}
	else
#endif /* NETOID_LINUX */
	{
		trc_error( MISPD, "RPCS : registering ISPD_VERS - normal\n" );

		if(!nx_svcudp_create(ispd_prog_1, ISPD_PROG, ISPD_VERS))
		{
			trc_error(	MISPD,
						"RPCS : Error - unable to create "
						"( ISPD_PROG, ISPD_VERS ) for udp\n");
			thread_exit(thread_descr->transient);
			return( (void*) NULL );
		}
	

		svc_run();

		trc_error(	MISPD,
					"RPCS : Error - ISPD_VERS service : svc_run returned\n");

		thread_exit(thread_descr->transient);
		return( (void*) NULL );

		// NOTREACHED 

	}
}

//
//	Function :
//		ispd_rpcsvc_shutdown()
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
ispd_rpcsvc_shutdown( void )
{
	//svc_exit();
	//millisleep( 500 );
	return;
}
