//
//
//
//

#include <ispd.h>
#include <rpc/rpc.h>

#include <trace.h>

// Global variables

int32_t			trace_disk  = 1;
FILE *			trace_fdesc;
char			trace_log[ 256 ]    = "/var/adm/ispd";

char 			progname[ MAXPATHLEN ];

trace_table_t   _trace_tbl;

void ispd_client_close( CLIENT * cl );

int
main(	int		argc,
		char *	argv[] )
{
	CLIENT *	cl;
	peer_info_t	peer_info;
	char *		tempname;

	tempname = basename( argv[0] );
	sprintf( progname, "%s[%d]", tempname, (int) getpid() );

	trc_init();

	if ( !get_iserver_info( &peer_info ) )
	{

		printf( "server_type	= %d\n", peer_info.server_type );
		printf( "iserver_status	= %d\n\n", peer_info.iserver_status );

		// Primary interface

		printf( "primary_interface     \"%s\"\n",
				peer_info.primary_interface );

		printf( "primary_ip            \"%s\"\n",
				peer_info.primary_ip );

		printf( "primary_router        \"%s\"\n",
				peer_info.primary_router );

		printf( "primary_vip           \"%s\"\n",
				peer_info.primary_vip );

		printf( "primary_iface_status   %d\n",
				peer_info.primary_interface_status );

		printf( "primary_vip_status     %d\n\n",
				peer_info.primary_vip_status );

		// Secondary interface

		printf( "secondary_interface   \"%s\"\n",
				peer_info.secondary_interface );

		printf( "secondary_ip          \"%s\"\n",
				peer_info.secondary_ip );

		printf( "secondary_router      \"%s\"\n",
				peer_info.secondary_router );

		printf( "secondary_vip         \"%s\"\n",
				peer_info.secondary_vip );

		printf( "secondary_iface_status %d\n",
				peer_info.secondary_interface_status );

		printf( "secondary_vip_status   %d\n\n",
				peer_info.secondary_vip_status );

		// Control interface

		printf( "ctl_ip                \"%s\"\n",
				peer_info.ctl_ip );

	}
	
	ispd_client_close( cl );

	return( 0 );
}

