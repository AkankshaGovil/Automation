#include "unp.h"
#include "rs.h"
#include "nxosd.h"

pthread_t		dbsync_rpcsvc_thread;

extern	void *	dbsync_rpcsvc_init( void * args );

void	dbsync_status_change_cb(	int status );
int		dbsync_health_check_cb(	void );

//
//  Function :
//      RPC_Init()
//
//  Arguments       :
//      None
//
//
//  Description :
//      This function starts up a thread as an RPC server in
//      the rsd daemon. Client calls will be made to it from
//		its parent, the ispd daemon.
//
//  Return Values:
//		None
//
void
RPC_Init( void )
{
	pthread_attr_t	thread_attr;
	int				status;

#ifndef NETOID_LINUX  
    //
    // Startup a thread as an RPC server for the ISPD_PROG program number
    // and DBSYNC_VERS version number. The client of the service is the
    // local ispd daemon. The service will be called when ever the status
    // of the box changes. A callback routine dbsync_callback() local to this
    // c file is registered to handle data passed to it. It is defined below.
    // 

    pthread_attr_init( &thread_attr );
    pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
    pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );
                                
    if ( (status = pthread_create(  &dbsync_rpcsvc_thread,
                            &thread_attr,
                            dbsync_rpcsvc_init,
                            (void*) NULL ) ) != 0 )
    {   
        NETERROR(   MRSD,
                    ("RPC_init() : error failed to start "
                        "dbsync_rpcsvc_init() - status %d\n",
                    status ));
    }
#endif // NETOID_LINUX

    return;
}

//
//	Function :
//		dbsync_status_change_cb()
//
//  Arguments       :
//		status			integer containing status of local iserver
//							0   "means"  INACTIVE
//							1   "means"  ACTIVE
//
//	Description :
//		This function is a registered callback which gets called
//		by local ispd to reflect changes to the status of the machine
//		regarding whether it is an active iserver or not.
//
//	Return Values:
//		None
//
void	
dbsync_status_change_cb(	int status )
{

	NETERROR(	MDEF,
				("     status is %s\n",
				( status == 0 )? "INACTIVE" : "ACTIVE" ));

	return;
}

//
//	Function :
//		dbsync_health_check_cb()
//
//  Arguments       :
//		None
//
//	Description :
//		This function is a registered callback which gets called
//		by local ispd to ask about its health.
//
//	Return Values:
//		status			integer containing status of local rsd
//							0   "means"  UNHEALTHY
//							1   "means"  HEALTHY
//
int
dbsync_health_check_cb( void )
{

	NETERROR(	MDEF,
				("     health check called\n" ));

	return(1);
}
