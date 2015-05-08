#ifndef _ISPD_H_
#define _ISPD_H_

//  "$Id: ispd.h,v 1.11.2.11 2004/10/14 00:21:10 amar Exp $"

// System include files

#define STRSZ			128

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
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
#include <string.h>
#include <stropts.h>
#include "nxosd.h"

#if SOLARIS_REL == 7
	#include <addrinfo.h>
#endif

#ifdef _DBMALLOC_
#define _DEBUG_MALLOC_
#endif

#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sched.h>
#include <rpc/rpc.h>
#include <libgen.h>

#ifndef _DEBUG_MALLOC_
	#define malloc_enter(func)
	#define malloc_leave(func)
	#define malloc_chain_check()
	#define malloc_dump(fd)
	#define malloc_list(a,b,c)
	#define malloc_inuse(hist)    (*(hist) = 0, 0)
#else
	#include "/usr/local/debug_include/malloc.h"
#endif

// iServer include files

#include <unix_ifi.h>
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

#include <trace.h>
#include <threads.h>
#include <ispd_queue.h>
#include <ispd_common.h>
#include <ispd_rpc.h>
#include <icmp_echo_utils.h>
#include "sysutils.h"

#define DEFAULT_CONFIG_FILE	"/usr/local/nextone/bin/server.cfg"

//
// Information about this host - read from server.cfg
//

extern	ispd_server_type_t		ispd_type;
extern	ispd_interface_t		ispd_primary;
extern	ispd_interface_t		ispd_secondary;
extern	ispd_ctl_interface_t	ispd_ctl;

extern	char					primary_vip_interface[ STRSZ ];
extern	char					secondary_vip_interface[ STRSZ ];

extern	int			ServerSetLogging(char *name, serplex_config *s);

extern	uint32_t	ispd_event_qid;

extern	cv_t		event_signal_cv;
extern	cv_t		monitor_signal_cv;

typedef enum _ispd_event_type
{
	ISPD_EVENT_PRIMARY_ROUTER_ECHO_STATUS   =  1,
	ISPD_EVENT_PRIMARY_IFACE_STATUS         =  2,
	ISPD_EVENT_PRIMARY_IFACE_LINK_STATUS    =  3,
	ISPD_EVENT_PRIMARY_VIP_STATUS           =  4,
	ISPD_EVENT_PRIMARY_VIP_ECHO_STATUS      =  5,

	ISPD_EVENT_SECONDARY_ROUTER_ECHO_STATUS =  6,
	ISPD_EVENT_SECONDARY_IFACE_STATUS       =  7,
	ISPD_EVENT_SECONDARY_VIP_STATUS         =  8,
	ISPD_EVENT_SECONDARY_IFACE_LINK_STATUS  =  9,
	ISPD_EVENT_SECONDARY_VIP_ECHO_STATUS    = 10,

	ISPD_EVENT_CTL_IFACE_ECHO_STATUS        = 11,
	ISPD_EVENT_CTL_IFACE_STATUS             = 12,
	ISPD_EVENT_CTL_IFACE_LINK_STATUS        = 13,

	ISPD_EVENT_ISERVER_STATUS           	= 14,
} ispd_event_type_t;

typedef enum _ispd_route_action
{
	ISPD_VIP_UP   							=  1,
	ISPD_VIP_DOWN         					=  2,
} ispd_route_action_t;

typedef struct _ispd_event_entry
{
	ispd_event_type_t	type;
	int					value;
	char				host[ STRSZ ];
} ispd_event_entry_t;

typedef enum _vip_id
{
	PRIMARY_VIP                             = 1,
	SECONDARY_VIP                           = 2,
} vip_id_t;

typedef enum _selection_result
{
	PEERHOST_UNREACHABLE                    = 1,
	PEERHOST_OWNS_VIP                       = 2,
	LOCALHOST_OWNS_VIP                      = 3,
} selection_result_t;

typedef enum _ispd_health
{
	SYSTEM_NOTREADY                    = 1,
	SYSTEM_HEALTHY                     = 2,
} ispd_health_t;

typedef enum _iserver_status
{
	ISPD_ISERVER_UNKNOWN                    = 0,
	ISPD_ISERVER_DOWN                       = 1,
	ISPD_ISERVER_UP                         = 2,
} iserver_status_t;

typedef enum _interface_status
{
	ISPD_IFACE_STATUS_UNKNOWN               = 0,
	ISPD_IFACE_STATUS_DOWN                  = 1,
	ISPD_IFACE_STATUS_UP                    = 2,
} interface_status_t;

typedef	struct _peer_status
{
	bool_t				peer_status_changed;	// flag indicating peer status
												// has changed.

	bool_t				ctl_reachable;			// peer reachable via ping
												// on control interface

	bool_t				peer_needs_update;		// flag indicating local info
												// needs to be pushed to peer
} peer_status_t;

typedef	struct _peer_info
{
	char				ctl_devname[ STRSZ ];
	int					ctl_instance;
	char				ctl_interface[ STRSZ ];
	char				ctl_ip[ STRSZ ];

	int32_t				host_key;				// Random key used to select
												// vip owner

	interface_status_t	ctl_interface_status;	// Status of interface :
												//  UNKNOWN, UP, DOWN

	interface_status_t	ctl_link_status;		// Status of hardware link :
												//  UNKNOWN, UP, DOWN

	CLIENT *			ispd_clnt_handle;		// rpc handle for peer ispd

	CLIENT *			gis_clnt_handle;		// rpc handle for local gis daemon

	CLIENT *			dbsync_clnt_handle;		// rpc handle for local dbsync daemon

	pthread_mutex_t		lock;

	int					ispd_vers_client;		// boolean :
                                                //  TRUE  "means" we have
                                                //         established client 
                                                //         communications
                                                //         with peer's ispd_vers
												//		   rpc server
                                                //  FALSE  "means" we have not
                                                //         established client 
                                                //         communications
                                                //         with peer's ispd_vers
												//		   rpc server

	int					gis_vers_client;		// boolean :
                                                //  TRUE  "means" we have
                                                //         established client 
                                                //         communications
                                                //         with local gis_vers
												//		   rpc server on gis
                                                //  FALSE  "means" we have not
                                                //         established client 
                                                //         communications
                                                //         with local gis_vers
												//		   rpc server on gis

	int					dbsync_vers_client;		// boolean :
                                                //  TRUE  "means" we have
                                                //         established client 
                                                //         communications
                                                //         with local dbsync_vers
												//		   rpc server on dbsync
                                                //  FALSE  "means" we have not
                                                //         established client 
                                                //         communications
                                                //         with local dbsync_vers
												//		   rpc server on dbsync

	int					send_status_gis;		// boolean :
                                                //  TRUE  "means" we should send
                                                //         status change message
                                                //         to local gis_vers
                                                //         rpc server on gis
                                                //  FALSE  "means" we should not
												//         send status change
												//         message to local
												//         gis_vers rpc server
												//         on gis

	int					send_status_dbsync;		// boolean :
                                                //  TRUE  "means" we should send
                                                //         status change message
                                                //         to local dbsync_vers
                                                //         rpc server on dbsync
                                                //  FALSE  "means" we should not
												//         send status change
												//         message to local
												//         dbsync_vers rpc server
												//         on dbsync

	int					icmp_echo_registered;   // boolean :
                                                //  TRUE  "means" registered ctl_ip
                                                //         for icmp echo testing
                                                //  FALSE "means" have not
                                                //         registered ctl_ip for
                                                //         icmp echo testing

	// Can clear structures beyond this point

	ispd_server_type_t	server_type;			// Server type: 
												//    ISPD_TYPE_ACTIVE
												//    ISPD_TYPE_DISABLED
												//    ISPD_TYPE_STANDBY

	iserver_status_t	iserver_status;			// Status of iserver processes
												//   gis, jvm, etc.

	char				primary_devname[ STRSZ ];
	int					primary_instance;
	char				primary_interface[ STRSZ ];
	char				primary_ip[ STRSZ ];
	char				primary_router[ STRSZ ];
	char				primary_vip[ STRSZ ];

	struct	sockaddr_in	primary_ip_addr;
	struct	sockaddr_in	primary_netmask_addr;

	interface_status_t	primary_interface_status;	// Status of interface :
													//  UNKNOWN, UP, DOWN

	interface_status_t	primary_link_status;		// Status of hardware link :
													//  UNKNOWN, UP, DOWN

	interface_status_t	primary_vip_status;			// Status of virtual ip :
													//  UNKNOWN, UP, DOWN

	echo_status_t		primary_router_echo;	    // primary router reachable
													// via ping :
													//    ECHO_STATUS_UNKNOWN
													//    ECHO_STATUS_UNREACHABLE
													//    ECHO_STATUS_REACHABLE

	echo_status_t		primary_vip_echo;	        // primary vip reachable
													// via ping :
													//    ECHO_STATUS_UNKNOWN
													//    ECHO_STATUS_UNREACHABLE
													//    ECHO_STATUS_REACHABLE

	char				secondary_devname[ STRSZ ];
	int					secondary_instance;
	char				secondary_interface[ STRSZ ];
	char				secondary_ip[ STRSZ ];
	char				secondary_router[ STRSZ ];
	char				secondary_vip[ STRSZ ];

	struct	sockaddr_in	secondary_ip_addr;
	struct	sockaddr_in	secondary_netmask_addr;

	interface_status_t	secondary_interface_status;	// Status of interface :
													//  UNKNOWN, UP, DOWN

	interface_status_t	secondary_link_status;		// Status of hardware link :
													//  UNKNOWN, UP, DOWN

	interface_status_t	secondary_vip_status;		// Status of virtual ip :
													//  UNKNOWN, UP, DOWN

	echo_status_t		secondary_router_echo;		// secondary router reachable
													// via ping :
													//    ECHO_STATUS_UNKNOWN
													//    ECHO_STATUS_UNREACHABLE
													//    ECHO_STATUS_REACHABLE

	echo_status_t		secondary_vip_echo;			// secondary vip reachable
													// via ping :
													//    ECHO_STATUS_UNKNOWN
													//    ECHO_STATUS_UNREACHABLE
													//    ECHO_STATUS_REACHABLE

	bool_t				local_status_changed;		// Flag indicating that
													// local status has changed

	peer_status_t		peer_status[ MAX_ISPD_PEERS ];

} peer_info_t;

extern peer_info_t		peer_info[ MAX_ISPD_PEERS ];
extern int				peer_count;
extern peer_info_t		local_info;

#include <ispd_rpc_common.h>

#define	ISPD_WEIGHTING	1.0
#define	ISPD_SCALE		65536

extern double drand48();
extern double pow();

//
//	Function		:
//		rand_init()
//
//	Arguments		:
//		None.
//
//	Description		:
//		Seeds random number generator.
//
//	Return value	:
//		None		on success
//		Abort		process on any failure - no bugs allowed
//
static inline void
rand_init( void )
{
	time_t	curtime;

	srand48( ( time( &curtime ) + getpid() ) );
}

//
//	Function		:
//		ispd_random()
//
//	Arguments		:
//		None.
//
//	Description		:
//		Returns a random integer between 0 and	65535
//
//	Return value	:
//		None		on success
//		Abort		process on any failure - no bugs allowed
//
static inline uint32_t
ispd_random( void )
{
	return( (uint32_t) ( ISPD_SCALE * pow( drand48(), ISPD_WEIGHTING ) ) );
}

//
//	Function		:
//		ifconfig_cycle()
//
//	Arguments		:
//
//		ifname		name of interface to bring down
//
//	Description		:
//		Brings up and down the specified network interface
//
//	Return value	:
//		 0			on success - command was run
//		-1			on failure
//
static inline int32_t
ifconfig_cycle(	char * 	ifname )
{
    char    command[128];
    int    	cmdfd;

	sprintf(    command,
				"/usr/sbin/ifconfig %s down",
				ifname );

	if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {

		sys_pclose( cmdfd );

		trc_debug(	MISPD, NETLOG_DEBUG4,
					"ACTN : ifconfig %-12s down - OK  - ifconfig_cycle()\n",
					ifname );

		sprintf(command,
				"/usr/sbin/ifconfig %s up",
				ifname );

		if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
			sys_pclose( cmdfd );

			trc_debug(	MISPD, NETLOG_DEBUG4,
						"ACTN : ifconfig %-12s up   - OK  - ifconfig_cycle()\n",
						ifname );

			return( 0 );
		} else {
			trc_error(	MISPD,
						"ACTN : ifconfig %-12s up   - ERR - ifconfig_cycle()\n",
						ifname );
		}
	} else {
		trc_error(	MISPD,
					"ACTN : ifconfig %-12s down - ERR - ifconfig_cycle()\n",
					ifname );
	}

	return( -1 );
}

//
//	Function		:
//		ifconfig_up()
//
//	Arguments		:
//		if_name			name of interface to bring up
//
//		if_status		address of interface_status_t to be updated
//
//	Description		:
//		Brings up the specified network interface
//
//	Return value	:
//		 None
//
static inline void
ifconfig_up(	char * 					if_name,
				interface_status_t	*	if_status )
{
    char    command[128];
    int    	cmdfd;

	sprintf(    command,
				"/usr/sbin/ifconfig %s up",
				if_name );

	if ( ( cmdfd = sys_popen( command, 0 )) > 0 ) {
		sys_pclose( cmdfd );

		trc_debug(	MISPD, NETLOG_DEBUG1,
					"ACTN : ifconfig %-12s up   - OK  - ifconfig_up()\n",
					if_name );

		*if_status = ISPD_IFACE_STATUS_UP;
	} else {
		trc_error(	MISPD,
					"ACTN : ifconfig %-12s up   - ERR - ifconfig_up()\n",
					if_name );
	}

	return;
}

//
//	Function		:
//		netroute_add()
//
//	Arguments		:
//
//		ifi_ptr		pointer to interface structure
//					containing route data
//
//	Description		:
//		Adds the network route associated with the physical interface
//
//	Return value	:
//		 None
//
static inline void
netroute_add(	struct ifi_info *	ifi_ptr )
{
    char    		command[128];
    int    			cmdfd;
    char    		c_network[128];
    char    		c_netmask[128];
    char    		c_ip[128];
    struct sockaddr_in	network;
    char buf[INET_ADDRSTRLEN];


	network.sin_addr.s_addr =
		ifi_ptr->ifi_addr->sin_addr.s_addr & ifi_ptr->ifi_netmask->sin_addr.s_addr;

	strcpy( c_ip, inet_ntop( AF_INET, &ifi_ptr->ifi_addr->sin_addr , buf, INET_ADDRSTRLEN) );
	strcpy( c_netmask, inet_ntop( AF_INET, &ifi_ptr->ifi_netmask->sin_addr , buf, INET_ADDRSTRLEN) );
	strcpy( c_network, inet_ntop( AF_INET, &network.sin_addr , buf, INET_ADDRSTRLEN));

	sprintf(    command,
				"/usr/sbin/route add -net %s -netmask %s %s -interface",
				c_network, c_netmask, c_ip );

	if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
			sys_pclose( cmdfd );

		trc_debug(	MISPD, NETLOG_DEBUG1,
					"ACTN : %s - route_add()\n",
					command );
	} else {
		trc_error(	MISPD,
					"ACTN : %s - ERR - route_add()\n",
					command );
	}

	return;
}

//
//	Function		:
//		ifconfig_down()
//
//	Arguments		:
//
//		if_name		name of interface to bring down
//
//		if_status	address of interface_status_t to be updated
//
//	Description		:
//		Brings down the specified network interface
//
//	Return value	:
//		 None
//
static inline void
ifconfig_down(	char * 					if_name,
				interface_status_t	*	if_status )
{
    char    command[128];
    int    	cmdfd;

	sprintf(    command,
				"/usr/sbin/ifconfig %s down",
				if_name );

	if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
			sys_pclose( cmdfd );

		trc_debug(	MISPD, NETLOG_DEBUG1,
					"ACTN : ifconfig %-12s down - OK  - ifconfig_down()\n",
					if_name );
		*if_status = ISPD_IFACE_STATUS_DOWN;
	} else {
		trc_error(	MISPD,
					"ACTN : ifconfig %-12s down - ERR - ifconfig_down()\n",
					if_name );
	}

	return;
}

//
//	Function		:
//		netroute_del()
//
//	Arguments		:
//
//		ifi_ptr		pointer to interface structure
//					containing route data
//
//	Description		:
//		Removes the network route associated with the physical interface
//
//	Return value	:
//		 None
//
static inline void
netroute_del(	struct ifi_info *	ifi_ptr )
{
    char    		command[128];
    int    			cmdfd;
    char    		c_network[128];
    char    		c_netmask[128];
    char    		c_ip[128];
    struct sockaddr_in	network;
    char buf[INET_ADDRSTRLEN];

	network.sin_addr.s_addr =
		ifi_ptr->ifi_addr->sin_addr.s_addr & ifi_ptr->ifi_netmask->sin_addr.s_addr;

	strcpy( c_ip, inet_ntop( AF_INET, &ifi_ptr->ifi_addr->sin_addr , buf, INET_ADDRSTRLEN) );
	strcpy( c_netmask, inet_ntop( AF_INET, &ifi_ptr->ifi_netmask->sin_addr , buf, INET_ADDRSTRLEN) );
	strcpy( c_network, inet_ntop( AF_INET, &network.sin_addr , buf, INET_ADDRSTRLEN));

	sprintf(    command,
				"/usr/sbin/route delete -net %s -netmask %s %s",
				c_network, c_netmask, c_ip );

	if ( ( cmdfd = sys_popen( command, 0 ) ) >= 0 ) {
			sys_pclose( cmdfd );

		trc_debug(	MISPD, NETLOG_DEBUG1,
					"ACTN : %s - netroute_del()\n",
					command );
	} else {
		trc_error(	MISPD,
					"ACTN : %s - ERR - netroute_del()\n",
					command );
	}

	return;
}

//
//	Function :
//		check_local_health()
//
//  Arguments       :
//		None.
//
//	Description :
//		This routine is called evaluate the readyness of
//		the local iserver. It makes sure that all conditions
//		for assuming ownership of a vip are met. If they
//		are it returns SYSTEM_HEALTHY, otherwise it returns
//		SYSTEM_NOTREADY.
//
//	Return Values:
//		ispd_health_t	returns an ispd_health_t enum value.
//						SYSTEM_HEALTHY or SYSTEM_NOTREADY
//
static inline ispd_health_t
check_local_health(	void )
{
	ispd_health_t	status = SYSTEM_NOTREADY;		// Default value - SYSTEM_NOTREADY
	peer_info_t *	lip = &local_info;

	// Is iserver operational ?

	if ( lip->iserver_status == ISPD_ISERVER_UP ) {

		status = SYSTEM_HEALTHY;
	}

	return( status );
}

//
//	Function :
//		validate_hostent()
//
//  Arguments       :
//		input	address of character string containing
//				name or ip address (in character form)
//				of host in system hostent database.
//
//		output	address of character string in which to place
//				ip address of input
//
//	Description :
//		This routine validates a host or ip address
//
//	Return Values:
//		 0		if input is ok
//		-1		if input is bogus
//
static inline int
validate_hostent(	const char *	input,
					char *			output )
{
	ulong_t				addr;
	struct hostent		he;
	struct hostent *	hp = &he;
	char				buffer[256];
	char **				p;
	int					error = 0;
	struct in_addr 		in;
        char                            buf[INET_ADDRSTRLEN];

	if ( (int)( addr = inet_addr( input ) ) == -1 )
	{
		nx_gethostbyname_r(	input,
							hp,
							buffer,
							256,
							&error );

		if ( error != 0 )
		{
			// host information not found for input
			return(-1);
		}

		if (hp == NULL)
		{
			// host information not found for input
			return(-1);
		}

		p = hp->h_addr_list;

		(void) memcpy( &in.s_addr, *p, sizeof( in.s_addr ) );
		(void) sprintf( output, "%s", inet_ntop( AF_INET, &in , buf, INET_ADDRSTRLEN) );
	}
	else
	{
		// Yes, let FormatIpAddress() convert the address to ascii for us

		FormatIpAddress( htonl( addr), output );
	}

	return(0);
}

//
//	Function :
//		findIfLogical()
//
//  Arguments       :
//		ifihead		pointer to head of list of struct ifi_info 
//
//		ipaddr		ip address of logical interface.
//
//	Description :
//		This routine validates a host or ip address
//
//	Return Values:
//		 0					if not found
//		
//		struct ifi_info *	pointer to logical interface	
//
static inline struct ifi_info *
findIfLogical(	struct ifi_info *ifihead,
    			unsigned long ipaddr )
{
    struct ifi_info *ifi, *ifinext;

    // Go through the list of interfaces given and match
    // the right one
    //

    for ( ifi = ifihead; ifi != NULL; ifi = ifinext )
    {
        if (( ifi->ifi_addr != NULL 	) && 
       		( ifi->ifi_netmask != NULL	) && 
			( ifi->ifi_myflags & IFI_ALIAS ) )
        {
			if (( ipaddr & ifi->ifi_netmask->sin_addr.s_addr ) ==
                ( ifi->ifi_addr->sin_addr.s_addr &
					ifi->ifi_netmask->sin_addr.s_addr) )
            {
                return ifi;
            }

        }
        ifinext = ifi->ifi_next;
    }

    return 0;
}

//
//	Function :
//		findIfPhys()
//
//  Arguments       :
//		ifihead		pointer to start of list of struct ifi_info 
//					to search
//
//		ipaddr		ip address of interface to look for
//
//		netmask		netmask of interface to look for
//
//		anyflag		flag indicating any physical interface
//					should be found without regard to ipaddr
//					and netmask information
//
//	Description :
//		This routine searches for physical interfaces that match the
//		network specified by the ipaddr and netmask argument. If 
//		found a pointer to the ifi_info structure is returned.
//		Otherwise null is returned.
//
//	Return Values:
//		NULL 				if not found
//		
//		struct ifi_info *	pointer to logical interface	
//
static inline struct ifi_info *
findIfPhys(	struct ifi_info *	ifihead,
			unsigned long		ipaddr,
    		unsigned long 		netmask,
    		uint32_t			anyflag )
{
    struct ifi_info *ifi, *ifinext;

	// Go through the list of interfaces given and match
	// the right one
	//

	for ( ifi = ifihead; ifi != NULL; ifi = ifinext )
	{
		if (( ifi->ifi_addr != NULL 	) && 
			( ifi->ifi_netmask != NULL	) &&
			( ! ( ifi->ifi_myflags & IFI_ALIAS ) ) )
		{

			if ( anyflag == 0 )
			{
				if (( ipaddr & netmask ) ==
					(	ifi->ifi_addr->sin_addr.s_addr & netmask ) )
				{
					char output[32];

					trc_debug(	MISPD,
								NETLOG_DEBUG2,
								"ACTN : found interface %s, ip %s - findIfPhys()\n",
								ifi->ifi_name,
								FormatIpAddress( htonl(ifi->ifi_addr->sin_addr.s_addr),
												 output ) );
								
					return( ifi );
				}
			}
			else
			{
				// If its not a loopback return it to caller

				if ( strncmp( ifi->ifi_name, "lo", 2 ) )
				{
					char output[32];

					trc_debug(	MISPD,
								NETLOG_DEBUG2,
								"found interface %s, ip %s - findIfPhys()\n",
								ifi->ifi_name,
								FormatIpAddress( htonl(ifi->ifi_addr->sin_addr.s_addr),
											 	output ) );
					return( ifi );
				}
			}
		}
		ifinext = ifi->ifi_next;
	}
    return( (struct ifi_info *) NULL );
}

#define read_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLK, F_RDLCK, offset, whence, len )

#define readw_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLKW, F_RDLCK, offset, whence, len )

#define write_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLK, F_WRLCK, offset, whence, len )

#define writew_lock( fd, offset, whence, len ) 				\
	lock_reg( fd, F_SETLKW, F_WRLCK, offset, whence, len )

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
static inline int
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


#endif // _ISPD_H_
