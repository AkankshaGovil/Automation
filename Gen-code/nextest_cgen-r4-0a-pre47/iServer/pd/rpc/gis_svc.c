//
//	File:
//
//		gis_svc.c
//
//	Description:
//
//		This file contains the server initialization
//		functions for the ISPD_PROG and GIS_VERS.
//      These functions are used to initialize the
//      rpc server within a running gis daemon.
//

#include "ispd_rpc.h"
#include <ispd.h>
#include <rpc/pmap_clnt.h>		/* for pmap_unset */
#include <memory.h>
#include <sys/resource.h>		/* rlimit */
#include <signal.h>
#include <rpc/rpc.h>
#include "nxosd.h"

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

extern void (*gis_rpc_cb)( int status, char *primary_vip, char* secondary_vip );

extern void ispd_prog_2(struct svc_req *rqstp, register SVCXPRT * transp);

static SVCXPRT *	transp;

pthread_mutex_t 			_svcstate_lock; // lock for _rpcsvcstate, _rpcsvccount

static void
_msgout(char *msg)
{
	NETERROR( MISPD, ( "RPC  : GIS ISPD Server : %s\n", msg ) );
}

//
//	Function :
//		gis_rpcsvc_init()
//
//  Arguments       :
//
//		args			Unused
//
//	Description :
//			This routine is started as a thread at gis
//		initialization time. gis_rpcsvc_init() initializes
//		RPC services for the ISPD_PROG, GIS_VERS service within
//      the gis process. After RPC services	are initialized the
//      thread drops into an eventloop in svc_run() where it
//      spends the rest of its life	servicing RPC calls from the
//      local ispd daemon on this host.
//
//	Return Values:
//		void *		not used. Exiting of the thread is actually
//					done in thread_exit() routine.
//
void *
gis_rpcsvc_init( void * args )
{
	struct netconfig *	nconf = NULL;
	void *	handlep = (void*) NULL;

	gis_rpc_cb = args;

	pthread_mutex_init(&_svcstate_lock, NULL);

	(void) sigset(SIGPIPE, SIG_IGN);

	handlep = setnetconfig();


	if ( (nconf = getnetconfigent("tcp")) == NULL)
	{
		_msgout("gis_rpcsvc_init(): cannot get transport info");
		return( (void*) NULL );
	}

	if ( ( transp = nx_svcudp_create(	ispd_prog_2,
									ISPD_PROG,
									GIS_VERS					)) == (SVCXPRT *)	NULL )
	{
		_msgout("gis_rpcsvc_init(): unable to create ( ISPD_PROG, GIS_VERS )");
		return( (void*) NULL );
	}

	freenetconfigent(nconf);

	if ( endnetconfig(handlep) != 0 )
	{
		_msgout("gis_rpcsvc_init(): endnetconfig() call failed");
		return( (void*) NULL );
	}

	if (!svc_reg( transp, ISPD_PROG, GIS_VERS, ispd_prog_2, 0))
	{
		_msgout("gis_rpcsvc_init(): unable to register ( ISPD_PROG, GIS_VERS ).");

		return( (void*) NULL );
	}

	svc_run();

	_msgout("daemon : svc_run returned");

	return( (void*) NULL );
}

//
//	Function :
//		gis_rpcsvc_shutdown()
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
gis_rpcsvc_shutdown( void )
{
	return;
}
