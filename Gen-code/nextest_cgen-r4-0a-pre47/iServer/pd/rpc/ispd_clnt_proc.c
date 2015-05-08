//
//	File:
//
//		ispd_clnt_proc.c
//
//	Description:
//
//		    This file contains the client interface functions
//		called by the ispd daemon. The functions are wrappers
//		for client-rpc procedure procedure calls made by the ispd,
//		( iServer Peering Daemon ).
//			These functions are called by the higher
//		level ispd code to make client calls to the rpc servers
//		in other ispd daemons running on other hosts.
//			The function descriptions describe which ispd rpc
//		version number they are affiliated with.
//

#include <ispd.h>
#include <rpc/rpc.h>

#include "ispd_rpc.h"

static void dump_args( iserver_info *	iserver_info );

//
// Function :
//		ispd_client_close()
//
// Version number affiliation:
//		ISPD_VERS, GIS_VERS and DBSYNC_VERS
//
// Arguments:
//		cl		client handle to be closed
//
// Return Values:
//		None
//
void
ispd_client_close( CLIENT * cl )
{
	if ( cl != (CLIENT *) NULL )
		clnt_destroy( cl );
}

//
// Function :
//		ispd_client_init()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		server			pointer to character string identifying
//						server to initialize client connection with.
// Description :
//		This routine is called to establish a client handle
//		with the specified ispd daemon on a remote host.
//
// Return Values:
//		 NULL	if not successful 
//		 cl     if successful returns new client handle
//
static CLIENT *
ispd_client_init( char *server )
{
	CLIENT *		cl;
	char *			error;
	char			description[255];
	struct timeval	timeOut;
	rpcvers_t		returned_vers;

	// Set default timeout for client handle, cl, to 2 seconds

	memset( &timeOut, (int) 0, sizeof( struct timeval ) );
	memset( &cl, (int) 0, sizeof( CLIENT * ) );

	timeOut.tv_sec = 2;

	//
	// Create client "handle" used for calling
	// MESSAGEPROG on the serverdesignated on
	// command line. We tell RPC package
	// to use the "tcp" protocol when contacting
	// the server.
	//

	if ( ( cl = clnt_create( server, 
							 ISPD_PROG,
							 ISPD_VERS,
							 "tcp" ) ) == NULL )  
	{
		// Couldn't establish connection with the server

		if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG4 )
    	{
			error = clnt_spcreateerror( server );
			trc_error( MISPD,  "RPC  : Client : %s\n", error );
		}

		return( (CLIENT *) NULL );
	}

	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
    {
		sprintf(	description,
					"Registered with RPC Server, \"%s\"",
					server );

		trc_error( MISPD,  "RPC  : Client : %s\n", description );
	}


	if ( clnt_control( cl, CLSET_TIMEOUT, (char*) &timeOut ) == FALSE )
	{
		sprintf(	description, 
					"can't set timeout value to 2 seconds\n" );
		trc_error( MISPD,  "RPC  : Client : %s\n", description );
	}

	return( cl );
}

//
// Function :
//		get_iserver_info()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		peer_info_ptr	pointer to peer_info_t structure containing
//						the address of the peer isever we want data
//						from.
// Description :
//		This routine calls the pull_iserver_1() rpc call to
//		call another ispd and retrieve peer configuration 
//		information. If successful the information retreived
//		is used to fill in the peer_info_t structure pointed
//		to by the input argument.
//
// Return Values:
//		 0	if successful
//		-1	if not successful
//
int
get_iserver_info( peer_info_t * peer_info_ptr )
{
	void *			args;
	iserver_info	*result;
	int				rc;

	lock_mutex( &peer_info_ptr->lock );

	if ( peer_info_ptr->ispd_clnt_handle == NULL )
	{
		if ( ( peer_info_ptr->ispd_clnt_handle =
				ispd_client_init( peer_info_ptr->ctl_ip ) ) == NULL )
		{
			peer_info_ptr->ispd_vers_client = FALSE;
			rc = -1;
		}
		else
		{
			peer_info_ptr->ispd_vers_client = TRUE;
		}
	}

	if ( peer_info_ptr->ispd_clnt_handle != NULL )
	{
		// Call the remote procedure readdir on the server

		if (result = pull_iserver_info_1(&args, peer_info_ptr->ispd_clnt_handle )) 
		{
			// Is peer disabled ?

			if ( result->server_type == ISPD_TYPE_DISABLED )
			{
				// Yes,

				trc_debug(	MISPD, NETLOG_DEBUG1,
							"RPC  : peer info pulled from, \"%s\" - DISABLED\n",
							peer_info_ptr->ctl_ip );

				peer_info_ptr->ispd_vers_client = FALSE;
				ispd_client_close( peer_info_ptr->ispd_clnt_handle );
				peer_info_ptr->ispd_clnt_handle = (CLIENT*) NULL;

				rc = -1;
			}
			else
			{

				peer_info_ptr->server_type = result->server_type;
				peer_info_ptr->iserver_status = result->iserver_status;

				// Primary interface info

				strcpy( peer_info_ptr->primary_interface,
						result->primary_interface );

				strcpy( peer_info_ptr->primary_ip,
						result->primary_ip );

				strcpy( peer_info_ptr->primary_router,
						result->primary_router );

				strcpy( peer_info_ptr->primary_vip,
						result->primary_vip );

				peer_info_ptr->primary_interface_status =
						result->primary_interface_status;

				peer_info_ptr->primary_link_status =
						result->primary_link_status;

				peer_info_ptr->primary_vip_status =
						result->primary_vip_status;

				peer_info_ptr->primary_router_echo =
						result->primary_router_echo;

				// Secondary interface info

				strcpy( peer_info_ptr->secondary_interface,
						result->secondary_interface );

				strcpy( peer_info_ptr->secondary_ip,
						result->secondary_ip );

				strcpy( peer_info_ptr->secondary_router,
						result->secondary_router );

				strcpy( peer_info_ptr->secondary_vip,
						result->secondary_vip );

				peer_info_ptr->secondary_interface_status =
						result->secondary_interface_status;

				peer_info_ptr->secondary_link_status =
						result->secondary_link_status;

				peer_info_ptr->secondary_vip_status =
						result->secondary_vip_status;

				peer_info_ptr->secondary_router_echo =
						result->secondary_router_echo;

				// Control interface info

				//strcpy( peer_info_ptr->ctl_ip,
				//		result.ctl_ip );

				peer_info_ptr->ctl_interface_status =
						result->ctl_interface_status;

				peer_info_ptr->ctl_link_status =
						result->ctl_link_status;

				rc = 0;


				trc_debug(	MISPD,
							NETLOG_DEBUG2,
							"RPC  : peer info pulled from, \"%s\"\n",
							peer_info_ptr->ctl_ip );
			}
		}
		else
		{
			trc_debug(	MISPD,
						NETLOG_DEBUG1,
						"RPCC : pull status call failed "
						"to ispd on \"%s\" - connection closed\n",
						peer_info_ptr->ctl_ip );

			peer_info_ptr->ispd_vers_client = FALSE;
			ispd_client_close( peer_info_ptr->ispd_clnt_handle );
			peer_info_ptr->ispd_clnt_handle = (CLIENT*) NULL;

			rc = -1;
		}

	}

	unlock_mutex( &peer_info_ptr->lock );

	return( rc );
}

//
// Function :
//		ispd_nullproc()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		peer_info_ptr	pointer to peer_info_t structure containing
//						the address of the peer isever we want to
//						ping.
//
// Description :
//		This routine makes a NULLPROC client call to the remote
//		ispd on another host. NULLPROC is essentially a ping of
//		the remote host. no information is exchanged.
//
// Return Values:
//		 0	if successful
//		-1	if not successful
//
int
ispd_nullproc( peer_info_t * peer_info_ptr )
{
	int						rc = 0;
	struct timeval			timeout = { 2, 0 };

	lock_mutex( &peer_info_ptr->lock );

	if ( peer_info_ptr->ispd_clnt_handle == NULL )
	{
		if ( ( peer_info_ptr->ispd_clnt_handle =
				ispd_client_init( peer_info_ptr->ctl_ip )) == NULL )
		{
			peer_info_ptr->ispd_vers_client = FALSE;
			rc = -1;
		}
		else
		{
			peer_info_ptr->ispd_vers_client = TRUE;
		}
	}

	if ( peer_info_ptr->ispd_clnt_handle != NULL )
	{
		// Call the remote procedure on the server

		if ( (rc = (int) clnt_call(	peer_info_ptr->ispd_clnt_handle,
									NULLPROC,
        							(xdrproc_t) xdr_void,
									(caddr_t) NULL,
        							(xdrproc_t) xdr_void,
									(caddr_t) NULL,
									timeout )) != RPC_SUCCESS )
		{
			trc_debug(	MISPD,
						NETLOG_DEBUG1,
						"RPCC : nullproc call failed to ispd "
						"on \"%s\" - connection closed\n",
						peer_info_ptr->ctl_ip );

			peer_info_ptr->ispd_vers_client = FALSE;
			ispd_client_close( peer_info_ptr->ispd_clnt_handle );
			peer_info_ptr->ispd_clnt_handle = (CLIENT*) NULL;

			rc = -1;
		}
	}

	unlock_mutex( &peer_info_ptr->lock );

	return( rc );
}

//
// Function :
//		ispd_nullproc_localhost()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//		None
//
// Description :
//		This routine makes a NULLPROC client call to the local
//		ispd. It is used during restart by ispd.
//
// Return Values:
//		 None
//
void
ispd_nullproc_localhost( void )
{
	struct timeval			timeout = { 2, 0 };
	CLIENT *				cl;

	cl = ispd_client_init( "localhost" );

	if ( cl != NULL )
	{
		// Call the remote procedure on the server

		if ( (int) clnt_call(	cl,
								NULLPROC,
        						(xdrproc_t) xdr_void,
								(caddr_t) NULL,
        						(xdrproc_t) xdr_void,
								(caddr_t) NULL,
								timeout ) != RPC_SUCCESS )
		{
			trc_error(	MISPD,
						"restart_ispd() : failed nullproc to "
						"\"localhost\" - connection closed\n" );
		}

		ispd_client_close( cl );
	}

	return;
}

//
// Function :
//		send_iserver_info()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		None.
//
// Description :
//		This routine calls the push_iserver_1() rpc call to
//		call each of the known peer ispds to tell them about
//		changes in local ispd status.
//
// Return Values:
//		 0	if successful pushing to all peers
//		-1	if not successful with at least 1 peer
//
int
send_iserver_info( void )
{
	int				rc = 0;
	iserver_info	args;
	int				i;
	peer_info_t *	peer_info_ptr;
	char			args_buffer[ 9 * STRSZ ];
	int				updated_hosts = 0;
	int				hosts_to_update = 0;

	for ( i = 0; i < peer_count; i++ )
		if ( local_info.peer_status[ i ].peer_needs_update == TRUE )
			hosts_to_update++;

	if ( hosts_to_update != 0 )
	{
		memset( &args, (int) 0, sizeof( struct iserver_info ));
		memset( &args_buffer, (int) 0, (9 * STRSZ) );

		args.primary_interface = &args_buffer[0];
		args.primary_ip = &args_buffer[ STRSZ ];
		args.primary_router = &args_buffer[ (STRSZ * 2) ];
		args.primary_vip = &args_buffer[ (STRSZ * 3) ];

		args.secondary_interface = &args_buffer[ (STRSZ * 4) ];
		args.secondary_ip = &args_buffer[ (STRSZ * 5) ];
		args.secondary_router = &args_buffer[ (STRSZ * 6) ];
		args.secondary_vip = &args_buffer[ (STRSZ * 7) ];

		args.ctl_ip = &args_buffer[ (STRSZ * 8) ];

		// Fill in local information which has changed

		args.server_type = local_info.server_type;
		args.iserver_status = local_info.iserver_status;

		// Primary interface info

		strcpy( args.primary_interface,
				local_info.primary_interface );

		strcpy( args.primary_ip,
				local_info.primary_ip );

		strcpy( args.primary_router,
				local_info.primary_router );

		strcpy( args.primary_vip,
				local_info.primary_vip );

		args.primary_interface_status =
			local_info.primary_interface_status;

		args.primary_link_status =
			local_info.primary_link_status;

		args.primary_vip_status =
			local_info.primary_vip_status;

		args.primary_router_echo =
			local_info.primary_router_echo;

		// Secondary interface info

		strcpy( args.secondary_interface,
				local_info.secondary_interface );

		strcpy( args.secondary_ip,
				local_info.secondary_ip );

		strcpy( args.secondary_router,
				local_info.secondary_router );

		strcpy( args.secondary_vip,
				local_info.secondary_vip );

		args.secondary_interface_status =
			local_info.secondary_interface_status;

		args.secondary_link_status =
			local_info.secondary_link_status;

		args.secondary_vip_status =
			local_info.secondary_vip_status;

		args.secondary_router_echo =
			local_info.secondary_router_echo;

		// Control interface info

		strcpy( args.ctl_ip,
				local_info.ctl_ip );

		args.ctl_interface_status =
			local_info.ctl_interface_status;

		args.ctl_link_status =
			local_info.ctl_link_status;

		dump_args( &args );

		// Push local info out to all peers that have not received it.

		for ( i = 0; i < peer_count; i++ )
		{
			if ( local_info.peer_status[ i ].peer_needs_update == FALSE )
				continue;

			peer_info_ptr = &peer_info[ i ];

			lock_mutex( &peer_info_ptr->lock );

			if ( peer_info_ptr->ispd_clnt_handle == NULL )
			{
				if ( ( peer_info_ptr->ispd_clnt_handle =
						ispd_client_init( peer_info_ptr->ctl_ip ) ) == NULL )
				{
					peer_info_ptr->ispd_vers_client = FALSE;
					rc = -1;
				}
				else
				{
					peer_info_ptr->ispd_vers_client = TRUE;
				}
			}

			if ( peer_info_ptr->ispd_clnt_handle != NULL )
			{
				// Call the remote procedure on the server to push updates
				// to server.

				if ( !push_iserver_info_1(	&args,
											peer_info_ptr->ispd_clnt_handle ))
				{
					trc_debug(	MISPD,
								NETLOG_DEBUG1,
								"RPCC : push status info call failed to "
								"ispd on \"%s\" - connection closed\n",
								peer_info_ptr->ctl_ip );

					peer_info_ptr->ispd_vers_client = FALSE;
					ispd_client_close( peer_info_ptr->ispd_clnt_handle );
					peer_info_ptr->ispd_clnt_handle = (CLIENT*) NULL;
					
					rc = -1;
				}
				else
				{
					trc_debug(	MISPD, NETLOG_DEBUG2,
								"RPCO : local data pushed to \"%s\"\n",
								peer_info_ptr->ctl_ip );
					updated_hosts++;
					local_info.peer_status[ i ].peer_needs_update = FALSE;
				}
			}

			unlock_mutex( &peer_info_ptr->lock );
		}
	}

	return( rc );
}

//
// Function :
//		dump_args()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//		None
//
// Description :
//		Utility function to dump the data from
//		a particular iserver_info structure.
//
// Return Values:
//		None.
//
static void
dump_args( iserver_info *	iserver_info )
{
	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG4 )
	{

		if ( iserver_info->server_type == ISPD_TYPE_ACTIVE )
		{
			trc_error(	MISPD,
						"RPCO : ispd_type       \"active\"\n" );
		}
		else
		if ( iserver_info->server_type == ISPD_TYPE_STANDBY )
		{
			trc_error(	MISPD,
						"RPCO : ispd_type       \"standby\"\n" );
		}
		else
		if ( iserver_info->server_type == ISPD_TYPE_DISABLED )
		{
			trc_error(	MISPD,
						"RPCO : ispd_type       \"disabled\"\n" );
		}

		if ( iserver_info->primary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{

			// Primary interface debugging

			trc_error(	MISPD,
						"RPCO : primary_interface:\n" );

			if ( strlen( iserver_info->primary_interface ) )
			{
				trc_error(	MISPD,
							"RPCO :      name       \"%s\"\n",
							iserver_info->primary_interface );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      name       NA\n" );
			}

			if ( iserver_info->primary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      if status  UP\n" );
			}
			else
			if ( iserver_info->primary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      if status  DOWN\n" );
			}

			if ( iserver_info->primary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      link state UP\n" );
			}
			else
			if ( iserver_info->primary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      link state DOWN\n" );
			}

			if ( strlen( iserver_info->primary_ip ) )
			{
				trc_error(	MISPD,
							"RPCO :      ip         \"%s\"\n",
							iserver_info->primary_ip );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      ip         NA\n" );
			}

			if ( strlen( iserver_info->primary_router ) )
			{
				trc_error(	MISPD,
							"RPCO :      router     \"%s\"\n",
							iserver_info->primary_router );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      router     NA\n" );
			}

			if ( strlen( iserver_info->primary_vip ) )
			{
				trc_error(	MISPD,
							"RPCO :      vip        \"%s\"\n",
							iserver_info->primary_vip );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      vip        NA\n" );
			}

			if ( iserver_info->primary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      vip status UP\n" );
			}
			else
			if ( iserver_info->primary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      vip status DOWN\n" );
			}

			if ( iserver_info->primary_router_echo == ECHO_STATUS_REACHABLE )
			{
				trc_error(	MISPD,
							"RPCO :      router reachable\n" );
			}
			else
			if ( iserver_info->primary_router_echo == ECHO_STATUS_UNREACHABLE )
			{
				trc_error(	MISPD,
							"RPCO :      router unreachable\n" );
			}
			else
			if ( iserver_info->primary_router_echo == ECHO_STATUS_UNKNOWN )
			{
				trc_error(	MISPD,
							"RPCO :      router unknown\n" );
			}
		}

		// Secondary interface debugging

		if ( iserver_info->secondary_interface_status != ISPD_IFACE_STATUS_UNKNOWN )
		{
			trc_error(	MISPD,
						"RPCO : secondary_interface:\n" );

			if ( strlen( iserver_info->secondary_interface ) )
			{
				trc_error(	MISPD,
							"RPCO :      name       \"%s\"\n",
							iserver_info->secondary_interface );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      name       NA\n" );
			}

			if ( iserver_info->secondary_interface_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      if status  UP\n" );
			}
			else
			if ( iserver_info->secondary_interface_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      if status  DOWN\n" );
			}

			if ( iserver_info->secondary_link_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      link state UP\n" );
			}
			else
			if ( iserver_info->secondary_link_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      link state DOWN\n" );
			}

			if ( strlen( iserver_info->secondary_ip ) )
			{
				trc_error(	MISPD,
							"RPCO :      ip         \"%s\"\n",
							iserver_info->secondary_ip );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      ip         NA\n" );
			}

			if ( strlen( iserver_info->secondary_router ) )
			{
				trc_error(	MISPD,
							"RPCO :      router     \"%s\"\n",
							iserver_info->secondary_router );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      router     NA\n" );
			}

			if ( strlen( iserver_info->secondary_vip ) )
			{
				trc_error(	MISPD,
							"RPCO :      vip        \"%s\"\n",
							iserver_info->secondary_vip );
			}
			else
			{
				trc_error(	MISPD,
							"RPCO :      vip        NA\n" );
			}

			if ( iserver_info->secondary_vip_status == ISPD_IFACE_STATUS_UP )
			{
				trc_error(	MISPD,
							"RPCO :      vip status UP\n" );
			}
			else
			if ( iserver_info->secondary_vip_status == ISPD_IFACE_STATUS_DOWN )
			{
				trc_error(	MISPD,
							"RPCO :      vip status DOWN\n" );
			}

			if ( iserver_info->secondary_router_echo == ECHO_STATUS_REACHABLE )
			{
				trc_error(	MISPD,
							"RPCO :      router reachable\n" );
			}
			else
			if ( iserver_info->secondary_router_echo == ECHO_STATUS_UNREACHABLE )
			{
				trc_error(	MISPD,
							"RPCO :      router unreachable\n" );
			}
			else
			if ( iserver_info->secondary_router_echo == ECHO_STATUS_UNKNOWN )
			{
				trc_error(	MISPD,
							"RPCO :      router unreachable\n" );
			}
		}

		// Control interface debugging

		trc_error(	MISPD,
					"RPCO : control_interface:\n" );

		if ( strlen( iserver_info->ctl_ip ) )
		{
			trc_error(	MISPD,
						"RPCO :      ip         \"%s\"\n",
						iserver_info->ctl_ip );
		}
		else
		{
			trc_error(	MISPD,
						"RPCO :      ip         NA\n" );
		}

		if ( iserver_info->ctl_interface_status == ISPD_IFACE_STATUS_UP )
		{
			trc_error(	MISPD,
						"RPCO :      if status  UP\n" );
		}
		else
		if ( iserver_info->ctl_interface_status == ISPD_IFACE_STATUS_DOWN )
		{
			trc_error(	MISPD,
						"RPCO :      if status  DOWN\n" );
		}

		if ( iserver_info->ctl_link_status == ISPD_IFACE_STATUS_UP )
		{
			trc_error(	MISPD,
						"RPCO :      link state UP\n" );
		}
		else
		if ( iserver_info->ctl_link_status == ISPD_IFACE_STATUS_DOWN )
		{
			trc_error(	MISPD,
						"RPCO :      link state DOWN\n" );
		}
	}
}

//
// Function :
//		select_vip_owner()
//
// Version number affiliation:
//		ISPD_VERS
//
// Arguments       :
//
//		peer_info_ptr	pointer to peer info structure for peer to
//						be called.
//
//		vip_id			which vip is to be selected
//
// Description :
//		This routine calls the select_vip_owner_1() rpc
//		call on the peer that shares the vip on one of
//		its network interfaces. The server routine on
//		the other host chooses which host will get
//		ownership of the vip and returns the value to
//		the calling host.
//
// Return Values:
//		 PEERHOST_UNREACHABLE
//		 PEERHOST_OWNS_VIP
//		 LOCALHOST_OWNS_VIP
//
selection_result_t
select_vip_owner(	peer_info_t *	peer_info_ptr,
					vip_id_t		vip_id )
{
	selection_result_t	rc = PEERHOST_UNREACHABLE;
	vip_info			args;
	char				args_buffer[ STRSZ ];
	int					*result;

	memset( &args, (int) 0, sizeof( struct vip_info ));
	memset( &args_buffer, (int) 0, STRSZ );

	lock_mutex( &peer_info_ptr->lock );

	if ( peer_info_ptr->ispd_clnt_handle == NULL )
	{
		if ( ( peer_info_ptr->ispd_clnt_handle =
				ispd_client_init( peer_info_ptr->ctl_ip )) == NULL )
		{
			peer_info_ptr->ispd_vers_client = FALSE;
		}
		else
		{
			peer_info_ptr->ispd_vers_client = TRUE;
		}
	}

	if ( peer_info_ptr->ispd_clnt_handle != NULL )
	{
		// Call the remote procedure on the server

		args.vip = &args_buffer[0];

		switch ( vip_id )
		{
		case PRIMARY_VIP:
			strcpy( args.vip,
					peer_info_ptr->primary_vip );
			break;
		case SECONDARY_VIP:
			strcpy( args.vip,
					peer_info_ptr->secondary_vip );
			break;
		}

		args.vip_id = vip_id;

		args.host_key = local_info.host_key;

		if ( !(result = select_vip_owner_1(	&args,
									peer_info_ptr->ispd_clnt_handle )))
		{
			trc_debug(	MISPD,
						NETLOG_DEBUG1,
						"RPCC : select vip call failed to "
						"ispd on \"%s\" - connection closed\n",
						peer_info_ptr->ctl_ip );

			peer_info_ptr->ispd_vers_client = FALSE;
			ispd_client_close( peer_info_ptr->ispd_clnt_handle );
			peer_info_ptr->ispd_clnt_handle = (CLIENT*) NULL;
		}
		else
		{
			switch( (selection_result_t) (*result) )
			{
			case PEERHOST_UNREACHABLE:
				trc_debug(	MISPD, NETLOG_DEBUG2,
					"CLNT : select_vip_owner() returns PEERHOST_UNREACHABLE\n");
				break;
			case PEERHOST_OWNS_VIP:
				trc_debug(	MISPD, NETLOG_DEBUG2,
					"CLNT : select_vip_owner() returns PEERHOST_OWNS_VIP\n");
				break;
			case LOCALHOST_OWNS_VIP:
				trc_debug(	MISPD, NETLOG_DEBUG2,
					"CLNT : select_vip_owner() returns LOCALHOST_OWNS_VIP\n");
				break;
			}

			rc = (selection_result_t) (*result);
		}
	}

	unlock_mutex( &peer_info_ptr->lock );

	return( rc );
}

//
// Function :
//		gis_clnt_init()
//
// Version number affiliation:
//		GIS_VERS
//
// Arguments       :
//		None
//
// Description :
//		This routine is called to establish a client handle
//		connected to the local gis daemon. The local gis daemon
//      starts an rpc server for the GIS_VERS version number
//      at the same time that it starts the ispd daemon.
//
// Return Values:
//		 NULL	if not successful 
//		 cl     if successful returns new client handle
//
static CLIENT *
gis_client_init( void )
{
	CLIENT *		cl;
	char *			error;
	char			description[255];
	struct timeval	timeOut;
	char *			server = "localhost";
	enum clnt_stat	call_rc;
	int				rc;

	// Set default timeout for client handle, cl, to 1 seconds

	memset( &timeOut, (int) 0, sizeof( struct timeval ) );
	memset( &cl, (int) 0, sizeof( CLIENT * ) );

	timeOut.tv_sec = 1;

	//
	// Create client "handle" used for calling
	// MESSAGEPROG on the serverdesignated on
	// command line. We tell RPC package
	// to use the "tcp" protocol when contacting
	// the server.
	//
	if ( ( cl = clnt_create (	server,
									ISPD_PROG,
									GIS_VERS,
									"tcp") ) == NULL )
	{
		// Couldn't establish connection with the server

		rc = rpc_createerr.cf_stat;
		error = clnt_spcreateerror( server );
		trc_debug(	MISPD, NETLOG_DEBUG4,
					"RPC  : Client : GIS_VERS - NULL HANDLE - %s\n",
					error,
					rc );

	}
	else
	{

		if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
		{
			sprintf(	description,
						"Registered with GIS_VERS RPC Server on \"%s\"",
						server );

			trc_error(	MISPD,
						"RPC  : Client : %s\n", description );
		}


		if ( clnt_control( cl, CLSET_TIMEOUT, (char*) &timeOut ) == FALSE )
		{
			sprintf(	description, 
						"can't set timeout value to 1 seconds\n" );
			trc_error( MISPD,  "RPC  : Client : %s\n", description );
		}

		if ( ( call_rc = clnt_call( cl, 
									NULLPROC,
									(xdrproc_t) xdr_void,
									(caddr_t) NULL,
									(xdrproc_t) xdr_void,
									(caddr_t) NULL,
									timeOut ) ) != RPC_SUCCESS )
		{
			// Couldn't establish connection with the server

			rc = rpc_createerr.cf_stat;
			error = clnt_spcreateerror( server );
			trc_debug(	MISPD, NETLOG_DEBUG3,
						"RPC  : Client : GIS_VERS - CALL FAILED - %s (%d)\n",
						error,
						rc );

			(void) clnt_destroy( cl );
			cl = (CLIENT * ) NULL;
		}
	}

	return( cl );
}

//
// Function :
//		dbsync_clnt_init()
//
// Version number affiliation:
//		DBSYNC_VERS
//
// Arguments       :
//		None
//
// Description :
//		This routine is called to establish a client handle
//		connected to the local dbsync daemon. The local dbsync
//		daemon starts an rpc server for the DBSYNC_VERS version
//		number after it is initialized.
//
// Return Values:
//		 NULL	if not successful 
//		 cl     if successful returns new client handle
//
static CLIENT *
dbsync_client_init( void )
{
	CLIENT *		cl;
	char *			error;
	char			description[255];
	struct timeval	timeOut;
	char *			server = "localhost";
	rpcvers_t		returned_vers;

	// Set default timeout for client handle, cl, to 2 seconds

	memset( &timeOut, (int) 0, sizeof( struct timeval ) );
	memset( &cl, (int) 0, sizeof( CLIENT * ) );

	timeOut.tv_sec = 2;

	//
	// Create client "handle" used for calling
	// MESSAGEPROG on the serverdesignated on
	// command line. We tell RPC package
	// to use the "tcp" protocol when contacting
	// the server.
	//

	if ( ( cl = clnt_create( server, 
							 ISPD_PROG,
							 DBSYNC_VERS,
							 "tcp" ) ) == NULL )  
	{
		// Couldn't establish connection with the server

		if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
    	{
			error = clnt_spcreateerror( server );
			trc_error( MISPD,  "RPC  : Client : DBSYNC_VERS - %s\n", error );
		}

		return( (CLIENT *) NULL );
	}

	if ( NetLogStatus[ MISPD ] & NETLOG_DEBUG2 )
    {
		sprintf(	description,
					"Registered with DBSYNC_VERS RPC Server on \"%s\"",
					server );

		trc_error( MISPD,  "RPC  : Client : %s\n", description );
	}


	if ( clnt_control( cl, CLSET_TIMEOUT, (char*) &timeOut ) == FALSE )
	{
		sprintf(	description, 
					"can't set timeout value to 2 seconds\n" );
		trc_error( MISPD,  "RPC  : Client : %s\n", description );
	}

	return( cl );
}

//
// Function :
//		send_status_change()
//
// Version number affiliation:
//		GIS_VERS, DBSYNC_VERS
//
// Arguments       :
//		None
//
// Description :
//		This routine is called to reflect vip interface status 
//		changes to the gis daemon. It passes the status - active or
//      inactive and the actual vips of the primary and secondary
//      vip interfaces to the gis.
//
// Return Values:
//		 0      if successful
//		-1      if unsuccessful
//
int32_t
send_status_change( void )
{
	status_change		args;
	char				args_buffer[ 2 * STRSZ ];
	int					gis_rc = -1;
	int					dbsync_rc = -1;
	int					status;

	trc_debug(	MISPD, NETLOG_DEBUG4,
				"GISV : send_status_change() entered\n" );

	if ( local_info.send_status_gis == TRUE )
	{
		// Establish client handle with local gis
		// Local gis is server for GIS_VERS

		if ( local_info.gis_clnt_handle == NULL )
		{
			if ( ( local_info.gis_clnt_handle = gis_client_init() ) == NULL )
			{
				local_info.gis_vers_client = FALSE;
			}
			else
			{
				local_info.gis_vers_client = TRUE;
			}
		}

		// If handle is established send status update
		// to Local gis

		if ( local_info.gis_clnt_handle != NULL )
		{
			memset( &args, (int) 0, sizeof( struct status_change ));
			memset( &args_buffer, (int) 0, (2 * STRSZ) );

			args.status = (*p_iserver_state == ISPD_ISERVER_UP)?1:0;

			args.primary_vip = &args_buffer[0];
			args.secondary_vip = &args_buffer[ STRSZ ];

			strcpy( args.primary_vip,
					local_info.primary_vip );

			strcpy( args.secondary_vip,
					local_info.secondary_vip );

			trc_debug(	MISPD, NETLOG_DEBUG2,
						"GISV : calling gis_status_change_2() - status %d\n",
						status );

			if ( !gis_status_change_2(	&args,
										local_info.gis_clnt_handle ))
			{
				trc_debug(	MISPD,
							NETLOG_DEBUG1,
							"RPCC : push status call failed to gis on "
							"localhost - connection closed\n" );

				local_info.gis_vers_client = FALSE;
				ispd_client_close( local_info.gis_clnt_handle );
				local_info.gis_clnt_handle = (CLIENT*) NULL;
			}
			else
			{
				local_info.send_status_gis = FALSE;
				gis_rc = 0;
			}
		}
		else
		{
			trc_debug(	MISPD, NETLOG_DEBUG2,
						"GISV : gis_clnt_handle == NULL - not sending\n" );
		}
	}
	else
	{
		trc_debug(	MISPD, NETLOG_DEBUG2,
					"GISV : send_status_gis == FALSE - not sending\n" );
		gis_rc = 0;
	}

	if ( local_info.send_status_dbsync == TRUE )
	{

		// Establish client handle with local dbsync
		// Local dbsync is server for DBSYNC_VERS

		if ( local_info.dbsync_clnt_handle == NULL )
		{
			if ( ( local_info.dbsync_clnt_handle =
						dbsync_client_init() ) == NULL )
			{
				local_info.dbsync_vers_client = FALSE;
			}
			else
			{
				local_info.dbsync_vers_client = TRUE;
			}
		}

		// If handle is established with local dbsync,
		// send status update to Local dbsync

		if ( local_info.dbsync_clnt_handle != NULL )
		{
			memset( &args, (int) 0, sizeof( struct status_change ));
			memset( &args_buffer, (int) 0, (2 * STRSZ) );

			args.status = (*p_iserver_state == ISPD_ISERVER_UP)?1:0;

			args.primary_vip = &args_buffer[0];
			args.secondary_vip = &args_buffer[ STRSZ ];

			strcpy( args.primary_vip,
					local_info.primary_vip );

			strcpy( args.secondary_vip,
					local_info.secondary_vip );

			if ( !dbsync_status_change_3(&args,
										local_info.dbsync_clnt_handle )
							!= RPC_SUCCESS )
			{
				trc_debug(	MISPD, 
							NETLOG_DEBUG1,
							"RPCC : push status call failed to "
							"dbsync on localhost"
							" - connection closed\n" );

				local_info.dbsync_vers_client = FALSE;
				ispd_client_close( local_info.dbsync_clnt_handle );
				local_info.dbsync_clnt_handle = (CLIENT*) NULL;
			}
			else
			{
				local_info.send_status_dbsync = FALSE;
				dbsync_rc = 0;
			}
		}
	}
	else
		dbsync_rc = 0;

	if ( dbsync_rc == 0 && gis_rc == 0 )
		return( 0 );
	else
		return( -1 );
}
