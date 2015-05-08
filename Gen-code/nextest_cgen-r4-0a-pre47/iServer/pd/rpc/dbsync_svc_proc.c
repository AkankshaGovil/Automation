//
//	File:
//		dbsync_svc_proc.c
//
//	Description:
//		This file contains the server functions called by
//		the rpcgen-generated rpc server portion of the
//		dbsync daemon. The only rpc server that runs in
//		an dbsync daemon is affiliated with the DBSYNC_VERS version
//		number. The only functions that should be called are
//		affiliated with DBSYNC_VERS. Dummy functions for ISPD_VERS
//		and GIS_VERS are included for linkage editting purposes.
//

#include <ispd.h>

#include <rpc/rpc.h>

#include "ispd_rpc.h"

//
// Place holder for callback routine
// Initialized when ISPD_PROG/DBSYNC_VERS service 
// is started.
//

extern	void	dbsync_status_change_cb( int status );
extern	int		dbsync_health_check_cb( void );

extern peer_info_t	local_info;
extern peer_info_t	peer_info[ MAX_ISPD_PEERS ];
extern int			peer_count;

//
// Dummy functions for ISPD_VERS not handled by this service
//

//
// Function :
//		pull_iserver_info_1_svc()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		none		void input argument
//
//		res			pointer to struct iserver_info to be filled
//					in to contain local ispd server state.
//
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the gis daemon is not registered
//		for the ISPD_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
struct iserver_info *
pull_iserver_info_1_svc(	void *					none,
							struct svc_req *		req )
{
	return( NULL );
}

//
// Function :
//		push_iserver_info_1_svc()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		arg			pointer to struct iserver_info passed by
//					remote ispd and containing the remote ispd's
//					status information
//
//		none		void output argument
//
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the gis daemon is not registered
//		for the ISPD_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
void *
push_iserver_info_1_svc(	iserver_info *		arg,
							struct svc_req *	req )
{
	return( NULL );
}

//
// Function :
//		select_vip_owner_1_svc()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		arg			pointer to struct vip_info passed by
//					remote ispd and containing the remote ispd's
//					vip information and a ispd key value
//
//		res			an integer indicating which server owns the
//					vip. Possible return values are :
//								LOCALHOST_OWNS_VIP
//								PEERHOST_OWNS_VIP
//
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the gis daemon is not registered
//		for the ISPD_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int *
select_vip_owner_1_svc(	struct vip_info *		arg,
						struct svc_req  *		req )
{
	return( NULL );
}
	
//
//	Function :
//		ispd_prog_1_freeresult()
//
// Version number affiliation:
//		ISPD_VERS
//
//  Arguments       :
//		transp		pointer to SVCXPRT for service whose
//					result is to be free'ed
//
//		xdr_result	type identifier for type to be free'ed
//
//		result		address of result to be free'ed
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the gis daemon is not registered
//		for the ISPD_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int
ispd_prog_1_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	return ( FALSE );
}

//
// Dummy functions for GIS_VERS not handled by this service
//

//
// Function :
//		gis_status_change_2_svc()
//
// Version number affiliation:
//		GIS_VERS
//
// Arguments       :
//
//		arg			pointer to struct status_change passed by
//					local ispd and containing the change of 
//					state information
//
//		none		void output argument
//
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the dbsync daemon is not registered
//		for the GIS_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
void *
gis_status_change_2_svc(	status_change *			arg,
							struct svc_req *		req )
{
	return ( NULL );
}

//
//	Function :
//		ispd_prog_2_freeresult()
//
// Version number affiliation:
//		GIS_VERS
//
//  Arguments       :
//		transp		pointer to SVCXPRT for service whose
//					result is to be free'ed
//
//		xdr_result	type identifier for type to be free'ed
//
//		result		address of result to be free'ed
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the gis daemon is not registered
//		for the ISPD_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int
ispd_prog_2_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	(void) xdr_free( xdr_result, result );
	return ( TRUE );
}

//
// functions for DBSYNC_VERS
//

static int32_t	dbsync_svccall_count;

//
//	Function :
//		dbsync_status_change_3_svc()
//
// Version number affiliation:
//		DBSYNC_VERS
//
//  Arguments       :
//		arg			pointer to status_change structure
//					passed by caller.
//
//		none		void * for null result
//
//		req			service request handle
//
// Description :
//		This routine is called by the ISPD_PROG/DBSYNC_VERS
//		rpc server in the dbsync daemon as the result of
//		client call made by the local ispd daemon. The ispd
//		caller is passing status change information to the dbsync.
//		The dbsync daemon takes appropriate action based on the
//		state change and returns.
//
// Return Values:
//		 TRUE   indicating success
//
void *
dbsync_status_change_3_svc(	status_change *			arg,
							struct svc_req *		req )
{
	static char c;
	NETERROR(	MDEF, 
				("dbsync_status_change_3_svc() called in dbsync - count %d\n",
				dbsync_svccall_count++ ));

	// Call the registered callback routine

    dbsync_status_change_cb( arg->status );
	return ( &c );
}

//
//	Function :
//		dbsync_health_check_3_svc()
//
// Version number affiliation:
//		DBSYNC_VERS
//
//  Arguments       :
//		none		void * for null arg
//
//		res			integer result of from health check callback
//
//		req			service request handle
//
// Description :
//		This routine is called by the ISPD_PROG/DBSYNC_VERS
//		rpc server in the dbsync daemon as the result of
//		client call made by the local ispd daemon. The callback
//		routine in rsd is called and returns an integer which is
//		returned to ispd indicating the health of the rsd daemon.
//
// Return Values:
//		 TRUE   indicating success
//
int *
dbsync_health_check_3_svc(	void *					none,
							struct svc_req *		req )
{
	static int res;
	NETERROR(	MDEF, 
				("dbsync_health_check_3_svc() called in dbsync - count %d\n",
				dbsync_svccall_count++ ));

	// Call the registered callback routine

    res = dbsync_health_check_cb();
	return ( &res );
}

//
//	Function :
//		ispd_prog_3_freeresult()
//
// Version number affiliation:
//		DBSYNC_VERS
//
//  Arguments       :
//		transp		pointer to SVCXPRT for service whose
//					result is to be free'ed
//
//		xdr_result	type identifier for type to be free'ed
//
//		result		address of result to be free'ed
//
//	Description :
//		This function is called to free up the results
//		of an svc service call.
//
//	Return Values:
//		TRUE		indicates success
//
int
ispd_prog_3_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	(void) xdr_free( xdr_result, result );
	return ( TRUE );
}
