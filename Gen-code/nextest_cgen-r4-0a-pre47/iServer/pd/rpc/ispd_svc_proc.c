//
//	File:
//		ispd_svc_proc.c
//
//	Description:
//		This file contains the server functions called by
//		the rpcgen-generated rpc server portion of the
//		ispd, ( iServer Peering Daemon ). The only server
//		that runs in an ispd daemon is affiliated with
//		the ISPD_VERS version number. The only functions
//		that should be called are affiliated with ISPD_VERS.
//		Dummy functions for GIS_VERS and DBSYNC_VERS are
//		included for linkage editting purposes.
//

#include <ispd.h>

#include <rpc/rpc.h>

#include "ispd_rpc.h"

extern peer_info_t	local_info;
extern peer_info_t	peer_info[ MAX_ISPD_PEERS ];
extern int			peer_count;

static void dump_peer_status( peer_info_t *	peer_info );

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
// Description :
//		This routine is called by the ISPD_PROG/ISPD_VERS
//		rpc server as the result of client call made by another
//		ispd daemon. The remote caller is asking for status
//		information about this ispd. The res structure is filled
//		in with local status information and returned.
//
// Return Values:
//		 TRUE   indicating success
//
struct iserver_info *
pull_iserver_info_1_svc(	void *					none,
							struct svc_req *		req )
{
	static struct iserver_info res;

	trc_debug(	MISPD, NETLOG_DEBUG4,
				"CALL : entering pull_iserver_info_1_svc()\n" );

	res.server_type = local_info.server_type;
	res.iserver_status = local_info.iserver_status;
	
	res.primary_interface = strdup( ispd_primary.name );
	res.primary_ip = strdup( ispd_primary.ip );
	res.primary_router = strdup( ispd_primary.router );
	res.primary_vip = strdup( ispd_primary.vip );

	res.primary_interface_status = local_info.primary_interface_status;
	res.primary_link_status = local_info.primary_link_status;
	res.primary_vip_status = local_info.primary_vip_status;
	res.primary_router_echo = local_info.primary_router_echo;

	res.secondary_interface = strdup( ispd_secondary.name );
	res.secondary_ip = strdup( ispd_secondary.ip );
	res.secondary_router = strdup( ispd_secondary.router );
	res.secondary_vip = strdup( ispd_secondary.vip );

	res.secondary_interface_status = local_info.secondary_interface_status;
	res.secondary_link_status = local_info.secondary_link_status;
	res.secondary_vip_status = local_info.secondary_vip_status;
	res.secondary_router_echo = local_info.secondary_router_echo;

	res.ctl_ip = strdup( ispd_ctl.ip );

	res.ctl_interface_status = local_info.ctl_interface_status;
	res.ctl_link_status = local_info.ctl_link_status;

	trc_debug(	MISPD, NETLOG_DEBUG4,
				"CALL : exiting pull_iserver_info_1_svc()\n" );

	return( &res );			// Send reply
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
// Description :
//		This routine is called by the ISPD_PROG/ISPD_VERS
//		rpc server as the result of client call made by another
//		ispd daemon. The remote caller is sending status information
//		changes to this ispd. The peer ispd to which the changes apply
//		is looked up and its data updated locally.
//
// Return Values:
//		 TRUE   indicating success
//
void *
push_iserver_info_1_svc(	iserver_info *		arg,
							struct svc_req *	req )
{
	int i, found = 0;
	static char c;

	trc_debug(	MISPD, NETLOG_DEBUG4,
				"CALL : entering push_iserver_info_1_svc()\n" );

	// Find the peer that is pushing this update to us in our
	// peer list

	for ( i = 0; i < peer_count; i++ )
	{
		if ( !strcmp( arg->ctl_ip, peer_info[ i ].ctl_ip ) )
		{
			peer_info[ i ].server_type = arg->server_type;
			peer_info[ i ].iserver_status = arg->iserver_status;
			
			peer_info[ i ].primary_interface_status =
				arg->primary_interface_status;

			peer_info[ i ].primary_link_status = arg->primary_link_status;
			peer_info[ i ].primary_vip_status = arg->primary_vip_status;

			peer_info[ i ].primary_router_echo =
				arg->primary_router_echo;

			peer_info[ i ].secondary_interface_status =
				arg->secondary_interface_status;

			peer_info[ i ].secondary_link_status = arg->secondary_link_status;
			peer_info[ i ].secondary_vip_status = arg->secondary_vip_status;

			peer_info[ i ].secondary_router_echo =
				arg->secondary_router_echo;

			peer_info[ i ].ctl_interface_status = arg->ctl_interface_status;
			peer_info[ i ].ctl_link_status = arg->ctl_link_status;

			lock_mutex( &local_info.lock );

			local_info.peer_status[ i ].peer_status_changed = TRUE;

			unlock_mutex( &local_info.lock );

			found = 1;
		}

		dump_peer_status( &peer_info[ i ] );
	}

	if ( !found )
	{
		trc_error(	MISPD,
					"RPCE : push_iserver_info_1_svc(): Unknown peer, \"%s\"\n",
					arg->ctl_ip );
	}
	else
	{
		trc_debug(	MISPD, NETLOG_DEBUG2,
					"RPC  : peer info pushed from, \"%s\"\n",
					arg->ctl_ip );
	}

	trc_debug(	MISPD, NETLOG_DEBUG4,
				"CALL : exiting  push_iserver_info_1_svc()\n" );

	return( (void *)&c );			// Send reply
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
// Description :
//		This routine is called by the ISPD_PROG/ISPD_VERS
//		rpc server as the result of client call made by another
//		ispd daemon. The purpose of the call is to decide which
//		of the ispd hosts owns the vip. Only one server can own
//		the vip or all hell will break loose when the vip is
//		brought up on multiple hosts. The health of the localhost
//		and the key value of the local and remote host are used
//		in this routine to make a decision on which server will
//		get the vip and the result is passed back to the caller
//		in res.
//
// Return Values:
//		 TRUE   indicating success
//
int *
select_vip_owner_1_svc(	struct vip_info *		arg,
						struct svc_req  *		req )
{
	extern void				recycle_virtifs( ispd_route_action_t action, int call );
	static	int				res;
	char *					choice;
	selection_result_t *	selres = (selection_result_t*) &res;
	vip_id_t				vip_id = arg->vip_id;

	// Are we healthy enough to be the master?
	if ( check_local_health() == SYSTEM_HEALTHY  && *p_iserver_state == ISPD_ISERVER_UP)
	{
		*selres = PEERHOST_OWNS_VIP;	// we get vip ownership -
		// We become the Active iServer.
		// Calling client becomes the Standby iServer.
	}
	else
	{
		*selres = LOCALHOST_OWNS_VIP;	
	}

	choice = ( ( *selres == LOCALHOST_OWNS_VIP )? "REMOTE" : "LOCAL" );

	if ( *selres == LOCALHOST_OWNS_VIP ) {

		// Caller is going to bring up vip so we need to bring it down
		// prior to returning so we dont have an IP conflict !!!!

		if(*p_iserver_state == ISPD_ISERVER_UP)
		{
			recycle_virtifs(ISPD_VIP_DOWN, 21);
		}

		*p_iserver_state = ISPD_ISERVER_DOWN;
	}

	trc_debug(	MISPD, NETLOG_DEBUG2,
				"SVC  : vip owner %s - lkey %d, rkey %d\n",
				choice,
				local_info.host_key,
				arg->host_key );

	return( &res );			// Send reply
}

//
// Function :
//		dump_peer_status()
//
// Version number affiliation:
//		ISPD_VERS
//
//
// Arguments       :
//		None
//
// Description :
//		Utility function to dump the data from
//		a particular peer_info_t structure.
//
// Return Values:
//		None.
//
static void
dump_peer_status( peer_info_t *	peer_info )
{
	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG3 )
	{
		// Control interface debugging

		trc_error(	MISPD,
					"RPCI : Update received from \"%s\"\n",
					peer_info->ctl_ip );

		if ( peer_info->primary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{
			// Primary interface debugging

			trc_error(	MISPD,
						"RPCI :   primary_interface:\n" );

			if ( peer_info->primary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     if status  UP\n" );
			}
			else
			if ( peer_info->primary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     if status  DOWN\n" );
			}

			if ( peer_info->primary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     link state UP\n" );
			}
			else
			if ( peer_info->primary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     link state DOWN\n" );
			}

			if ( peer_info->primary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     vip status UP\n" );
			}
			else
			if ( peer_info->primary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     vip status DOWN\n" );
			}

			if ( peer_info->primary_router_echo == ECHO_STATUS_REACHABLE )
			{
				trc_error(	MISPD,
							"RPCI :     router reachable\n" );
			}
			else
			if ( peer_info->primary_router_echo == ECHO_STATUS_UNREACHABLE )
			{
				trc_error(	MISPD,
							"RPCI :     router unreachable\n" );
			}
			else
			if ( peer_info->primary_router_echo == ECHO_STATUS_UNKNOWN )
			{
				trc_error(	MISPD,
							"RPCI :     router unknown\n" );
			}
		}

		// Secondary interface debugging

		if ( peer_info->secondary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{
			trc_error(	MISPD,
						"RPCI :   secondary_interface:\n" );

			if ( peer_info->secondary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     if status  UP\n" );
			}
			else
			if ( peer_info->secondary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     if status  DOWN\n" );
			}

			if ( peer_info->secondary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     link state UP\n" );
			}
			else
			if ( peer_info->secondary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     link state DOWN\n" );
			}

			if ( peer_info->secondary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCI :     vip status UP\n" );
			}
			else
			if ( peer_info->secondary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCI :     vip status DOWN\n" );
			}

			if ( peer_info->secondary_router_echo == ECHO_STATUS_REACHABLE )
			{
				trc_error(	MISPD,
							"RPCI :     router reachable\n" );
			}
			else
			if ( peer_info->secondary_router_echo == ECHO_STATUS_UNREACHABLE )
			{
				trc_error(	MISPD,
							"RPCI :     router unreachable\n" );
			}
			else
			if ( peer_info->secondary_router_echo == ECHO_STATUS_UNKNOWN )
			{
				trc_error(	MISPD,
							"RPCI :     router unknown\n" );
			}
		}
	}
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
//		This function is called to free up the results
//		of an svc service call.
//
//	Return Values:
//		TRUE		indicates success
//
int
ispd_prog_1_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	(void) xdr_free( xdr_result, result );
	return ( TRUE );
}

//
//	Function :
//		gis_status_change_2_svc()
//
// Version number affiliation:
//		GIS_VERS
//
//  Arguments       :
//		arg			pointer to status_change structure
//					passed by caller.
//
//		none		void * for null result
//
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the ispd daemon is not registered
//		for the GIS_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
void *
gis_status_change_2_svc(	status_change *			arg,
							struct svc_req *		req )
{
	return ( FALSE );
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
//		req			service request handle
//
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the ispd daemon is not registered
//		for the GIS_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int
ispd_prog_2_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	return ( FALSE );
}

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
//	Description :
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the ispd daemon is not registered
//		for the DBSYNC_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
void *
dbsync_status_change_3_svc(	status_change *			arg,
							struct svc_req *		req )
{
	return ( FALSE );
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
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the ispd daemon is not registered
//		for the DBSYNC_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int *
dbsync_health_check_3_svc(	void *					none,
							struct svc_req *		req )
{
	return ( NULL );
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
//		This function is a dummy routine included 
//		for linkage editting purposes. It should not
//		be called since the ispd daemon is not registered
//		for the DBSYNC_VERS version number.
//
//	Return Values:
//		FALSE		indicates failure
//
int
ispd_prog_3_freeresult(	SVCXPRT *	transp,
						xdrproc_t	xdr_result,
						caddr_t		result		)
{
	return ( FALSE );
}
