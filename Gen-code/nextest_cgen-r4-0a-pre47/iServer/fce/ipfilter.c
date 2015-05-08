//
// this file contains ipfilter (NSF) firewall specific api
//

#define _IPFILTER_SYS_INCLUDE_

#include "ipfilter.h"
#include "portalloc.h"

#define PM_HOLE_TYPE_MEDIA		10
#define PM_HOLE_TYPE_SIGNALING	20

//>*****************************************
//>* local ipfilter (NSF) firewall parameter
//>* storage and datatype definitions
//>*****************************************

static char			PublicInterface[256];
static uint32_t		PublicIp;
static PortAlloc *	PublicInterface_PortAlloc;

static char			PrivateInterface[256];
static uint32_t		PrivateIp = 0;
static PortAlloc *	PrivateInterface_PortAlloc;

// Public Media Routing Interface Resources

static uint32_t		PublicMR_count;
static char			PublicMR_interface[MAX_MEDIA_IFACE][256];
static PortAlloc *	PublicMR_PortAlloc[MAX_MEDIA_IFACE];
static uint32_t		PublicMR_ipaddr[MAX_MEDIA_IFACE];
static uint32_t		PublicMR_current;

// Private Media Routing Interface Resources

static uint32_t		PrivateMR_count;
static char			PrivateMR_interface[MAX_MEDIA_IFACE][256];
static PortAlloc *	PrivateMR_PortAlloc[MAX_MEDIA_IFACE];
static uint32_t		PrivateMR_ipaddr[MAX_MEDIA_IFACE];
static uint32_t		PrivateMR_current;

// Physical Interface Resources

static uint32_t		phys_iface_count;
static char			phys_ifaces[MAX_INTERFACES][256];
static uint32_t		in_group[MAX_INTERFACES];
static uint32_t		out_group[MAX_INTERFACES];	

static uint32_t		LocalNatEnabled;
static uint32_t		RemoteNatEnabled;

static int			ipnat_fd;		// open fd for local /dev/ipnat device
static int			ipf_fd;			// open fd for local /dev/ipf device

static uint16_t		largest_anon_port = 59999;

// Structure to hold data read from config file

typedef	struct _ipf_config
{
	char		PublicInterface[256];
	char		PrivateInterface[256];
	char		PublicMR_interface[MAX_MEDIA_IFACE][256];
	uint32_t	PublicMR_count;
	char		PrivateMR_interface[MAX_MEDIA_IFACE][256];
	uint32_t	PrivateMR_count;
//	char		MRInterface[MAX_MEDIA_IFACE][256];	// Media routing interfaces
//	uint32_t	MRInterface_count;					// # of media routing
	uint32_t	LocalNatEnabled;
	uint32_t	RemoteNatEnabled;
} ipf_config_t;

//
// mutex to lock on, while accessing shared resources 
//

static pthread_mutex_t			mutex;

typedef struct	_ipf_nat
{
	IPNAT_T			data;
	char			rdr_string_1[128];
	char			rdr_string_2[128];
	char			rdr_string_3[128];
	int32_t			rdr_defined;

}	ipf_nat_t;

typedef struct	_ipf_rule
{
	FRENTRY_T		data;
	char			rule_string_1[128];
	char			rule_string_2[128];
	char			rule_string_3[128];
	int32_t			rule_defined;

}	ipf_rule_t;

typedef struct	_ipf_pmhole
{
	fr_pm_entry_t	data;
	uint16_t		port;
	int32_t			hole_defined;
}	ipf_pmhole_t;

typedef	struct IpHoleBlock	IpHoleBlock_t;

struct IpHoleBlock
{
	struct	IpHoleBlock		*holeIdPrev, 	*holeIdNext;
	struct	IpHoleBlock		*sessionIdPrev, *sessionIdNext;

	uint32_t				assignedId;
	uint32_t				sessionId;
	PortAlloc *				PortAlloc_ptr;
	uint16_t				allocated_port_pair;
	PortAlloc *				PortAlloc_ptr2;
	uint16_t				allocated_port_pair2;

	ipf_nat_t				rtp_nat;
	ipf_nat_t				rtcp_nat;

	ipf_pmhole_t			ipf_pmhole_signaling;
};

#define	HOLEID_LIST_OFFSET		(0)
#define	SESSIONID_LIST_OFFSET	(2*sizeof(IpHoleBlock_t *))

typedef struct	_forward_media_t
{
	int32_t				recv_sock;
	int32_t				send_sock;
	uint32_t			dest_ip;
	uint16_t			dest_port;
	pthread_attr_t		thread_attr;
}	forward_media_t;

typedef struct	_finish_reopen_t
{
	uint32_t			mediaIpAddress;		// ip address for changing leg of call
	uint16_t			mediaPort;			// port for changing leg of call

	uint32_t			protocol;

	uint32_t			sessionId;
	uint32_t			sessionId2;
	pthread_attr_t		thread_attr;
}	finish_reopen_t;

// Hashtable cache for IpHoleBlock_t lookup by HoleId

static	cache_t			HoleIdCache;

// Hashtable cache for IpHoleBlock_t lookup by sessionId

static	cache_t			SessionIdCache;

static int				lastAssignedId = 0;

void					tweak_ipfilter_ipaddr(	char *	primary_vip,
												char *	secondary_vip );

//>*****************************************
//>*
//>* Declarations of local entry points
//>*
//>*****************************************

static int32_t	_IpFilter_initcaches(
													void);

static IpHoleBlock_t *	
				GetIpHoleBlockForSessionId( 
									uint32_t 		sessionId );

static void		_CloseHole(			IpHoleBlock_t *
													idp );

static int 		_RemovePMHole(		int32_t			holeId,
									uint32_t		sessionId,
									ipf_pmhole_t *	pmh_ptr );

static void		_RemoveRedirect(	uint32_t		sessionId,
									uint32_t		holeId,
									void *			ipnat_ptr,
									char *			rule_string_1,
									char *			rule_string_2,
									char *			rule_string_3,
									char *			ctype );

static int 		_OpenPinHole(		char *			interface,
									int32_t			ip_addr,
									int32_t			holeId,
									uint32_t		sessionId,
									uint16_t		port,
									int32_t			protocol,
									ipf_pmhole_t *	pmh_ptr );

static int 		_AddRedirect( 		char *			iface_in,
									char *			iface_out,
									int32_t			holeId,
									int32_t			orig_dst_ip,
									uint16_t		orig_dst_port,
									int32_t			new_dst_ip,
									uint16_t		new_dst_port,
									char *			rdr_type,
									ipf_nat_t *		rdr_ptr,
									int32_t			map_src_ip,
									uint16_t		map_src_port,
									int32_t			mapflag );

static IpHoleBlock_t * 	
				IpHoleBlockAlloc( 	uint32_t 		sessionId );

static int		OpenMediaHole(		uint32_t		mediaIpAddress,
	   								uint16_t		mediaPort,
									eFCETrafficDirection	
													direction,
	   								uint32_t		sessionId,
	   								uint32_t		sessionIdLeg2,
									int				numTranslations,
	   								uint32_t *		returnedMediaIpAddress,
	   								uint16_t *		returnedMediaPort,
									int				allocatePrivatePorts );

static uint32_t AddMediaHole(		uint32_t		sessionId,
									ipf_nat_t *		rtp_nat_ptr,
									ipf_nat_t *		rtcp_nat_ptr,
									PortAlloc *		PortAlloc_ptr,
									uint16_t		new_local_port,
									PortAlloc *		PortAlloc_ptr2,
									uint16_t		new_local_port2 );

static int		OpenSignalingHole(	uint32_t		protocol,
		   							uint32_t		signalingIpAddress,
		   							uint16_t		signalingPort,
		   							uint32_t		sessionId,
		   							uint32_t		sessionIdLeg2,
									int				numTranslations,
		   							uint32_t *		returnedMediaIpAddress,
		   							uint16_t *		returnedMediaPort );

static uint32_t AddSignalingHole(	uint32_t		sessionId,
									ipf_pmhole_t *	ipf_pmhole_signaling_ptr );

static IpHoleBlock_t *	
				GetIpHoleBlockForHoleId( 
									uint32_t		assignedId );

static void		DeleteIpHoleBlock(	IpHoleBlock_t * ptr );

static void		DeleteIpHoleBlockForSession( 
									uint32_t		sessionId );

static int		opendevice(			char *			devname,
									int *			fdp );

static void		ReadVars( 							void );

static void		ConfigInit( 						void );

static void		hexdump(			uchar_t *		input,
									int32_t			nBytes,
									uint32_t		firstByte);

static int32_t	IpfilterReadConfig( ipf_config_t *	config_data );


static uint16_t	_getnext_port(		PortAlloc *		PortAlloc_ptr );

static void		_free_ports(		PortAlloc *		PortAlloc_ptr,
									uint16_t		port );

static void		_IpFilter_reload(					void );

// Ward off warnings from the compiler about
// unsued routines

void *dummy_hAndle1 = (void*) hexdump;

//>*****************************************
//>* Locally used Trace facility globals
//>* definitions. static inline trace 
//>* functions defined in trace.h file
//>*****************************************

int32_t	trace_disk = 1;
int32_t	trace_initialized;
FILE*	trace_fdesc;
int32_t	trace_fd;

trace_table_t	iserv_trace_tbl;

char			trace_log[256] = TRACE_NSF_TRACE_FILENAME;

//>*****************************************
//>*
//>* Definitions of exported entry points
//>*
//>*****************************************

//
//	Function 	:
//		IpfilterInit()
//
//	Purpose		:
//
//		This function should be called at iserver startup time.
//		it reads in the configuration file and opens the devices
//		needed to setup and destroy pinholes.
//
void
IpfilterInit(void)
{
	char *				cmd = "/usr/sbin/ndd -get /dev/udp udp_largest_anon_port";
	char 				buf[256];
	FILE *				ptr;
	fr_clearbm_entry_t	fr_cb;
	int					i;

	static int			largest_anon_port_gotten = 0;

	if ( opendevice( IPNAT_DEVICE, &ipnat_fd ) < 0 )
	{
		NETERROR(	MFCE,
					( "MFCE : Unable to open /dev/ipnat device\n" ));
		return;
	}

	if ( opendevice( IPF_DEVICE, &ipf_fd ) < 0 )
	{
		NETERROR(	MFCE,
					( "MFCE : Unable to open /dev/ipf device\n" ));
		return;
	}

	if ( !largest_anon_port_gotten )
	{
		// Get largest_anon_port value set in /etc/init.d/nettune

		if ((ptr = popen(cmd, "r")) != NULL)
		{
			if ( fgets(buf, 256, ptr) != NULL )
			{
				largest_anon_port = (uint16_t) atoi( buf );
				NETDEBUG(	MFCE,
							NETLOG_DEBUG2,
							("largest_anon_port %d\n",
							largest_anon_port ));
			}
			pclose( ptr );
		}
	}

	// read the variables configured 

	ReadVars();

	// Reload ipfilter static rules and clear any dynamic rules
	// associated with the driver

	_IpFilter_reload();

	// Instantiate caches used for sessionId and holeId lookup

	if ( _IpFilter_initcaches() < 0 )
	{
		NETERROR(	MFCE,
					( "MFCE : Unable to allocate cache(s) for "
					  "ipfilter lookup\n" ));
		return;
	}

	// initialize the mutex 

	if ( initMutex( &mutex, NULL ) )
	{
		ConfigInit();
		return;
	}

	trc_init();

	// Close bit map holes for all three interface types,
	//   ( Public, Private and Media Route )

	if ( strlen( PublicInterface ) != 0 )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, PublicInterface );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Public interface %s\n",
						PublicInterface ));
		}

	}

	if ( strlen( PrivateInterface ) != 0 )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, PrivateInterface );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Private interface %s\n",
						PrivateInterface ));
		}
	}

	for ( i = 0; i < PublicMR_count; i++ )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, &PublicMR_interface[i][0] );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Public MR interface %s\n",
						&PublicMR_interface[i][0] ));
		}
	}

	for ( i = 0; i < PrivateMR_count; i++ )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, &PrivateMR_interface[i][0] );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Private MR interface %s\n",
						&PrivateMR_interface[i][0] ));
		}
	}

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
			 	("Successfully initialized with ipfilter firewall\n" ));
}

//
//	Function 	:
//		IpfilterShutdown()
//
//	Purpose		:
//
//		This function should be called at iserver shutdown time.
//		It removes all holes and closes the ipfilter devices.
//
//
int
IpfilterShutdown(void)
{
	IpHoleBlock_t *		ptr;
	fr_clearbm_entry_t	fr_cb;
	int					i;

	trc_error(	MFCE,
				"SHUTD  : called\n" );

	lockMutex( &mutex );
	
	if ( HoleIdCache != NULL )
	{
		// Close all holes that have been opened.

		while( (ptr = CacheGetFirst( HoleIdCache )) != NULL )
		{
			_CloseHole( ptr );

			// Remove holeId from SessionIdCache and HoleIdCache

			DeleteIpHoleBlock(ptr);
		}
	}

	if ( HoleIdCache != NULL )
	{
		CacheDestroy( HoleIdCache );
		HoleIdCache = NULL;
	}

	if ( SessionIdCache != NULL )
	{
		CacheDestroy( SessionIdCache );
		SessionIdCache = NULL;
	}

	// Close bit map holes for all three interface types,
	//   ( Public, Private and Media Route )

	if ( strlen( PublicInterface ) != 0 )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, PublicInterface );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Public interface %s\n",
						PublicInterface ));
		}

	}

	if ( strlen( PrivateInterface ) != 0 )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, PrivateInterface );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Private interface %s\n",
						PrivateInterface ));
		}
	}

	for ( i = 0; i < PublicMR_count; i++ )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, &PublicMR_interface[i][0] );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Public MR interface %s\n",
						&PublicMR_interface[i][0] ));
		}
	}

	for ( i = 0; i < PrivateMR_count; i++ )
	{
		memset( &fr_cb, 0, sizeof( fr_clearbm_entry_t ) );
		strcpy( fr_cb.ifname, &PrivateMR_interface[i][0] );

		if ( ioctl( ipf_fd, SIOCCLRBM, &fr_cb ) < 0 )
		{
			NETERROR(	MFCE,
				 		("Error clearing bitmap for Private MR interface %s\n",
						&PrivateMR_interface[i][0] ));
		}
	}

	// Clear any nat entries in kernel

	{
		int	n = 0;

		if ( ioctl( ipnat_fd, SIOCIPFFL, &n ) == -1 )
			NETERROR(	MFCE,
						("Error flushing nat table - error %d\n",
						errno ));

		n = 1;
		if ( ioctl( ipnat_fd, SIOCIPFFL, &n ) == -1 )
			NETERROR(	MFCE, 
						("Error clearing nat table - error %d\n",
						errno ));
	}

	close( ipnat_fd );
	close( ipf_fd );

	// unlock the mutex

	unlockMutex( &mutex );

	// destroy the mutex 

	destroyMutex( &mutex, NULL );

	trc_error(	MFCE,
				"SHUTD  : exiting\n" );

	trc_close();

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
			 	("Successfully shutdown connection with ipfilter firewall\n" ));

	return( TRUE );
}

//
//	Function 	:
//		IpfilterReconfig()
//
//	Purpose		:
//
//		This function should be called after the configuration
//		file, nsf.cfg or ipfilter.cfg, has changed.
//
//	Note:
//		for now any changes to nsf.cfg or ipfilter.conf files, require cycling
//		of the gis (start/stop) to take effect.
//
// return Value :
//
//		 0 if the previous config equals the current config,
//      -1 if there are any changes
//
int
IpfilterReconfig(void)
{
	ipf_config_t	config_data;
	int32_t			i, j, found;
	int32_t			rc = 0;

	trc_error(	MFCE,
				"RECFG  : called\n" );

	// Were we able to read the config file successfully ?

	if ( IpfilterReadConfig( &config_data ) == 0 )
	{
		// Yes, Check if anything changed

		int	config_changed = 0;

		if ( strcmp( config_data.PublicInterface, PublicInterface ) != 0 )
			config_changed = 1;

		if ( !config_changed )
			if ( strcmp( config_data.PrivateInterface, PrivateInterface ) != 0 )
			config_changed = 1;

		if ( !config_changed )
			if ( ( PublicMR_count != config_data.PublicMR_count ) )
				config_changed = 1;

		if ( !config_changed )
			if ( ( PrivateMR_count != config_data.PrivateMR_count ) )
				config_changed = 1;

		if ( !config_changed )
		{
			for ( i = 0; i < config_data.PublicMR_count; i++ )
			{
				found = 0;

				for ( j = 0; j < PublicMR_count; j++ ) 
				{
					if ( strcmp( config_data.PublicMR_interface[ i ],
					             PublicMR_interface[ j ] ) == 0 )
						found = 1;
				}

				if ( found == 0 )
				{
					config_changed = 1;
					break;
				}
			}
		}

		if ( !config_changed )
		{
			for ( i = 0; i < config_data.PrivateMR_count; i++ )
			{
				found = 0;

				for ( j = 0; j < PrivateMR_count; j++ ) 
				{
					if ( strcmp( config_data.PrivateMR_interface[ i ],
					             PrivateMR_interface[ j ] ) == 0 )
						found = 1;
				}

				if ( found == 0 )
				{
					config_changed = 1;
					break;
				}
			}
		}

		if ( !config_changed )
			if ( config_data.RemoteNatEnabled != RemoteNatEnabled )
				config_changed = 1;

		if ( !config_changed )
			if ( config_data.LocalNatEnabled != LocalNatEnabled )
				config_changed = 1;

		// Did anything change ?

		if ( config_changed )
		{
			// Yes, reinit ipfilter and rebuild from nsf.cfg

			char *			path = "/usr/local/nextone/bin/nsfconfig";
			char *			cmd = "/usr/local/nextone/bin/nsfconfig -v";
			char 			buf[256];
			FILE *			ptr;
			struct stat		statbuf;

			rc = 1;			// Indicate configuration has changed

			// Shutdown ipfilter and flush loaded rules

			IpfilterShutdown();

			// Rebuild static rules based on nsf.cfg changes

			if ( stat( path, 
						(struct stat *) &statbuf ) < 0 ) // does script exist ?
			{
				// No, log error and return

				NETERROR( 	MFCE, 
		   			("Ipfilter_reconfig : file, \"%s\", not found.\n",
					path ));
			}
			else
			{
				if ( !( statbuf.st_mode & S_IXUSR ) ) // is script executable ?
				{
					// No, log error and return

					NETERROR(	MFCE,
		   				("Ipfilter_reconfig : file, \"%s\", not executable.\n",
						path ));
				}
				else
				if ((ptr = popen(cmd, "r")) != NULL)
				{
   					while (fgets(buf, 256, ptr) != NULL);
					pclose( ptr );
				}

			}

			// Restart ipfilter and reload new filter rules

			IpfilterInit();
		}
	}

	// Has nsf been disabled ?

	if ( strcasecmp( fceConfigFwName, "nsf") != 0 && 
		 strcasecmp( fceConfigFwName, "ipfilter" ) != 0 )
	{
		// Yes, disable nsf

		char *			path = "/usr/local/nextone/bin/nsfconfig";
		char *			cmd = "/usr/local/nextone/bin/nsfconfig -d";
		char 			buf[256];
		FILE *			ptr;
		struct stat		statbuf;

		rc = 1;			// Indicate configuration has changed

		// Shutdown ipfilter and flush loaded rules

		IpfilterShutdown();

		// disable ipfilter.

		if ( stat(	path, 
					(struct stat *) &statbuf ) < 0 ) // does script exist ?
		{
			// No, log error and return

			NETERROR( 	MFCE, 
	   					("Ipfilter_reconfig : file, \"%s\", not found.\n",
						path ));
		}
		else
		{
			if ( !( statbuf.st_mode & S_IXUSR ) ) // is script executable ?
			{
				// No, log error and return

				NETERROR(	MFCE,
	   				("Ipfilter_reconfig : file, \"%s\", not executable.\n",
					path ));
			}
			else
			if ((ptr = popen(cmd, "r")) != NULL)
			{
   				while (fgets(buf, 256, ptr) != NULL);
				pclose( ptr );
			}
		}
	}

	trc_error(	MFCE,
				"RECFG  : exiting\n" );

	return( rc );
}

//
//	Function 	:
//		IpfilterServerInactive()
//
//	Purpose		:
//
//		This function is called when the redundant iserver
//		has becomes active.
//
//	Arguments	:
//		None	
//
//	Return Value :
//		None	
//
void
IpfilterServerActive(void)
{
	return;
}

//
//	Function 	:
//		IpfilterServerInactive()
//
//	Purpose		:
//
//		This function is called when the redundant iserver
//		has become inactive. Clear all the holes/redirects.
//
//	Arguments	:
//		None	
//
//	Return Value :
//		None	
//
void
IpfilterServerInactive(void)
{
	return;
}

//
//	Function 	:
//		IpfilterOpenHole()
//
//	Purpose		:
//
//		This function opens a pinhole in the firewall and sets up
//		a nat translation entry in the firewall.
//
//	Arguments	:
//
//		protocol			
//							-	one of :
//								IPPROTO_TCP, IPPROTO_UDP, or IPPROTO_RAW
//
//		mediaIpAddress		
//							-	ip address of endpoint that receives the packets
//								( ultimate destination of packets associated
//								  with hole )
//
//								for signaling holes traffic terminates on the
//								MSW so ip address will be the ip address of
//								a local physical interface or a logical
//								interface associated with a local physical
//								interface.
//
//								for media holes this is the ip address of the
//								ultimate media destination for this hole.
//								Packets associated with this hole will travel
//								to this ip address.
//
//		mediaPort			
//							-	destination port
//
//								for signaling holes this will be a port
//								on a local physical or logical interface
//
//								for media holes the value is a remote port
//								on the endpoint identified by mediaIpAddress.
//								RTP packets will flow to this port. Two
//								consecutive holes will be opened in this
//								case. (RTP/RTCP)
//
//		direction			-	defines the traffic direction through the
//								firewall. Only used for media holes since
//								signaling holes terminate, locally. For media
//								holes on an MSW routing between two networks
//								on two physical network interfaces, direction,
//								tells us how to build the redirects.
//
//								one of :
//									FCE_eInt2Int -
//										  Packet traffic from a private endpoint
//										travels to another private endpoint.
//										Data is reflected back out of the
//										incoming local private interface.
//										  We allocate a port pair from a local
//										private interface ip address and return
//										the allocated resources to endpoint
//										sending the media traffic for this leg
//										and set up the appropriate reflective
//										redirects - RTP/RTCP
//													
//									FCE_eInt2Ext -
//										Packet traffic from a private endpoint
//										travels to a public endpoint.
//										  We allocate a port pair from a
//										private local interface ip address
//										and return the allocated resources to
//										the endpoint sending the media traffic
//										and set up appropriate dual interface
//										redirections - RTP/RTCP
//
//									FCE_eExt2Int -
//										Packet traffic from a public endpoint
//										travels to a private endpoint.
//										  We allocate a port pair from a
//										public local interface ip address
//										and return the allocated resources
//										to the endpoint sending the media
//										traffic and set up appropriate dual
//										interface redirections - RTP/RTCP
//
//									FCE_eExt2Ext -
//										  Packet traffic from a public endpoint
//										travels to another public endpoint.
//										Data is reflected back out of the
//										incoming local public interface.
//										  We allocate a port pair from a local
//										public interface ip address and return
//										the allocated resources to endpoint
//										sending the media traffic for this leg
//										and set up the appropriate reflective
//										redirects - RTP/RTCP
//
//		sessionId			
//							-	an optional session id that can be used
//								to tear down connections in the future
//
//		sessionIdLeg2		-	the session id of the other call leg
//
//		numTranslations		-	the # of TCP/RTP connections to allow,
//								to be used in the future
//
//		returnedMediaIpAddress
//							-	return value of media ip address
//
//		returnedMediaPort
//							-	return value of media port
//
//		allocatePrivatePorts
//							-	if the value is 1, the ports returned could be
//									re-used in IpfilterReopenHole call
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
int
IpfilterOpenHole(	uint32_t				protocol,
			   		uint32_t				mediaIpAddress,
			   		uint16_t				mediaPort,
					eFCETrafficDirection	direction,
			   		uint32_t				sessionId,
			   		uint32_t				sessionIdLeg2,
					int						numTranslations,
			   		uint32_t *				returnedMediaIpAddress,
			   		uint16_t *				returnedMediaPort,
					int						allocatePrivatePorts )
{
	int		rc;

	if ( protocol == IPPROTO_RAW )
	{
		rc = OpenMediaHole(		mediaIpAddress,
								mediaPort,
								direction,
								sessionId,
								sessionIdLeg2,
								numTranslations,
								returnedMediaIpAddress,
								returnedMediaPort,
								allocatePrivatePorts );
	}
	else
	{
		rc =OpenSignalingHole(	protocol,
		   						mediaIpAddress,
								mediaPort,
								sessionId,
								sessionIdLeg2,
								numTranslations,
								returnedMediaIpAddress,
								returnedMediaPort );
	}

	return( rc );
}

//
//	Function 	:
//		OpenMediaHole()
//
//	Purpose		:
//
//		This function opens a pinholes for rtp and rtcp media
//		to pass from one media endpoint through the nsf firewall
//		to another endpoint.
//
//	Arguments	:
//
//		mediaIpAddress		
//							-	remote destination ip address
//
//								This value is the ip address of the
//								ultimate media destination for this hole.
//								Packets associated with this hole will travel
//								to this ip address.
//
//		mediaPort			
//							-	remote destination port
//
//								This value is a remote port
//								on the endpoint identified by mediaIpAddress.
//								RTP packets will flow to this port. Two
//								consecutive holes will be opened in this
//								case. (RTP/RTCP)
//								(host order)
//
//		direction			-	defines the traffic direction through the
//								firewall. Only used for media holes since
//								signaling holes terminate, locally. For media
//								holes on an MSW routing between two networks
//								on two physical network interfaces, direction,
//								tells us how to build the redirects.
//
//								one of :
//									FCE_eInt2Int -
//										  Packet traffic from a private
//										endpoint travels to another private
//										endpoint. Data is reflected back out
//										of the incoming local private
//										interface.
//										  We allocate a port pair from a local
//										private interface ip address and return
//										the allocated resources to endpoint
//										sending the media traffic for this leg
//										and set up the appropriate reflective
//										redirects - RTP/RTCP
//													
//									FCE_eInt2Ext -
//										Packet traffic from a private endpoint
//										travels to a public endpoint.
//										  We allocate a port pair from a
//										private local interface ip address
//										and return the allocated resources to
//										the endpoint sending the media traffic
//										and set up appropriate dual interface
//										redirections - RTP/RTCP
//
//									FCE_eExt2Int -
//										Packet traffic from a public endpoint
//										travels to a private endpoint.
//										  We allocate a port pair from a
//										public local interface ip address
//										and return the allocated resources
//										to the endpoint sending the media
//										traffic and set up appropriate dual
//										interface redirections - RTP/RTCP
//
//									FCE_eExt2Ext -
//										  Packet traffic from a public endpoint
//										travels to another public endpoint.
//										Data is reflected back out of the
//										incoming local public interface.
//										  We allocate a port pair from a local
//										public interface ip address and return
//										the allocated resources to endpoint
//										sending the media traffic for this leg
//										and set up the appropriate reflective
//										redirects - RTP/RTCP
//
//		direction			-	defines the traffic direction through the firewall
//								one of :
//								FCE_eInt2Int - nothing to do for now
//								FCE_eInt2Ext - traffic from inside the firewall
//									to get out (pass out rule)
//								FCE_eExt2Int - traffic from outside the firewall
//									to get out (pass in rule)
//								FCE_eExt2Ext - to be used in the future
//
//		sessionId			
//							-	an optional session id that can be used
//								to tear down connections in the future
//
//		sessionIdLeg2		-	the session id of the other call leg
//
//		numTranslations		-	the number of TCP/RTP connections to allow,
//								to be used in the future
//
//		returnedMediaIpAddress
//							-	return value of media ip address
//
//		returnedMediaPort
//							-	return value of media port
//
//		allocatePrivatePorts
//							-	if the value is 1, the ports returned could be
//									re-used in IpfilterReopenHole call
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
static int
OpenMediaHole(	uint32_t				mediaIpAddress,
	   			uint16_t				mediaPort,
				eFCETrafficDirection	direction,
	   			uint32_t				sessionId,
	   			uint32_t				sessionIdLeg2,
				int						numTranslations,
	   			uint32_t *				returnedMediaIpAddress,
	   			uint16_t *				returnedMediaPort,
				int						allocatePrivatePorts )
{
	uint16_t		new_local_port = mediaPort;
	uint16_t		new_local_port2 = 0;
	int32_t			ports_allocated = 0;
	uint32_t		holeId;

	ipf_nat_t		rtp_nat;
	ipf_nat_t		rtcp_nat;

	char *			media_gw_interface;
	int32_t			media_gw_ip;

	PortAlloc *		PortAlloc_ptr = NULL;
	PortAlloc *		PortAlloc_ptr2 = NULL;

	char *			PublicMRInterface = NULL;
	char *			PrivateMRInterface = NULL;

	uint32_t		PublicMRIp = 0;
	uint32_t		PrivateMRIp = 0;

	memset( &rtp_nat, (uint32_t) 0, sizeof( ipf_nat_t ) );
	memset( &rtcp_nat, (uint32_t) 0, sizeof( ipf_nat_t ) );

	*returnedMediaIpAddress = mediaIpAddress;
	*returnedMediaPort = mediaPort;

	if ( direction == FCE_eExt2Int )
	{
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"OMH -> : Ext2Int\n" );
	}
	else
	if ( direction == FCE_eInt2Ext )
	{
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"OMH -> : Int2Ext\n" );
	}
	else
	if ( direction == FCE_eInt2Int )
	{
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"OMH -> : Int2Int\n" );
	}
	else
	if ( direction == FCE_eExt2Ext )
	{
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"OMH -> : Ext2Ext\n" );
	}

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "OpenMediaHole: entering\n" ));

	lockMutex( &mutex );

	holeId = lastAssignedId + 1;

	// if NAT enabled, set up REDIRECT translations

	if ( (	LocalNatEnabled == TRUE		&& direction		== FCE_eExt2Int ) )
	{
		// Check feasability 

		if ( PublicMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  :  Unable to Media Route"
						" Private to Public calls\n" );
			trc_error(	MFCE,
						"       :  No Public MR interfaces configured\n" );
			goto errorExit;
		}

		if ( PrivateMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  :  Unable to Media Route"
						" Private to Public calls\n" );
			trc_error(	MFCE,
						"       :  No Private MR interfaces configured\n" );
			goto errorExit;
		}

		// Allocate public interface resources

		PublicMRInterface =
			&PublicMR_interface[ PublicMR_current ][0];

		PublicMRIp =
			PublicMR_ipaddr[ PublicMR_current ];

		PortAlloc_ptr =
			PublicMR_PortAlloc[ PublicMR_current ];

		if ( PublicMR_current < ( PublicMR_count - 1 ) )
			PublicMR_current++;
		else
			PublicMR_current = 0;

		// Allocate private interface resources

		PrivateMRInterface =
			&PrivateMR_interface[ PrivateMR_current ][0];

		PrivateMRIp =
			PrivateMR_ipaddr[ PrivateMR_current ];

		PortAlloc_ptr2 =
			PrivateMR_PortAlloc[ PrivateMR_current ];

		if ( PrivateMR_current < ( PrivateMR_count - 1 ) )
			PrivateMR_current++;
		else
			PrivateMR_current = 0;

		//
		// Add a redirect for rtp of the following format :
		//	rdr {public_if} {gw_public_ip} port {allocated_port} udp ->
		//					{private_endpoint_ip} port {private_endpoint_port}
		//
		// Example:
		//  rdr znb0 204.180.228.217/32 port 60000 ->
		//			 192.168.228.2 port 9446 udp
		//

		//
		//	Get 2 new port pairs to be used by external endpoint
		// on the public side to reach internal endpoint on the
		// private side.
		//	The first port pair is allocated from a public
		// interface. The external endpoint will send its rtp
		// and rtcp data to these two ports, respectively.
		//	The second port pair is allocated from a private
		// interface. On the way out of the private interface
		// the source information will be altered to specify 
		// the private interface address and this second set 
		// of ports.
		//

		if ( (new_local_port = 
				_getnext_port( PortAlloc_ptr )) ==  (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating public "
						"MR ports on %s - portalloc_ptr 0x%08x\n",
						PublicMRInterface,
						PortAlloc_ptr );
			goto errorExit;
		}

		if ( (new_local_port2 = 
				_getnext_port( PortAlloc_ptr2 )) ==  (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating private "
						"MR ports on %s - portalloc_ptr 0x%08x\n",
						PrivateMRInterface,
						PortAlloc_ptr2 );	//xxx
			goto errorExit;
		}

		ports_allocated = 1;

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	PublicMRInterface,
							PrivateMRInterface,
							holeId,
							PublicMRIp,
							new_local_port,
							mediaIpAddress,
							mediaPort,
							"rtp",
							&rtp_nat,
							PrivateMRIp,
							new_local_port2,
							1 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 1 failed for Ext2Int UDP\n" );
			goto errorExit;
		}

		*returnedMediaIpAddress = PublicMRIp;
		*returnedMediaPort = new_local_port;

		rtp_nat.rdr_defined = TRUE;

		// Build a redirect for rtcp of the following format :
		//   rdr {public_if} {gw_public_ip} port {allocated_port} ->
		//					 {local_endpoint_ip} port {local_port} udp
		//
		// Example:
		//  rdr znb0 204.180.228.217/32 port 60001 ->
		//			 192.168.228.2 port 9446 udp

		// Add new redirect and fill in local structure 

		// Specify its nat type as NAT_REDIRECT

		if ( _AddRedirect(	PublicMRInterface,
							PrivateMRInterface,
							holeId,
							PublicMRIp,
							( new_local_port + 1 ),
							mediaIpAddress,
							(mediaPort + 1),
							"rtcp",
							&rtcp_nat,
							PrivateMRIp,
							( new_local_port2 + 1 ),
							1 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 2 failed for Ext2Int UDP\n" );
			goto errorExit;
		}

		rtcp_nat.rdr_defined = TRUE;
	}
	else
	if ( (	RemoteNatEnabled == TRUE && direction		== FCE_eInt2Ext ) )
	{
		// Check feasability 

		if ( PublicMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  :  Unable to Media Route"
						" Private to Public calls\n" );
			trc_error(	MFCE,
						"       :  No Public MR interfaces configured\n" );
			goto errorExit;
		}

		if ( PrivateMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  :  Unable to Media Route"
						" Private to Public calls\n" );
			trc_error(	MFCE,
						"       :  No Private MR interfaces configured\n" );
			goto errorExit;
		}

		// Allocate private interface resources

		PrivateMRInterface =
			&PrivateMR_interface[ PrivateMR_current ][0];

		PrivateMRIp =
			PrivateMR_ipaddr[ PrivateMR_current ];

		PortAlloc_ptr =
			PrivateMR_PortAlloc[ PrivateMR_current ];

		if ( PrivateMR_current < ( PrivateMR_count - 1 ) )
			PrivateMR_current++;
		else
			PrivateMR_current = 0;

		// Allocate public interface resources

		PublicMRInterface =
			&PublicMR_interface[ PublicMR_current ][0];

		PublicMRIp =
			PublicMR_ipaddr[ PublicMR_current ];

		PortAlloc_ptr2 =
			PublicMR_PortAlloc[ PublicMR_current ];

		if ( PublicMR_current < ( PublicMR_count - 1 ) )
			PublicMR_current++;
		else
			PublicMR_current = 0;

		//
		// Build a redirect of the following format :
		//	rdr {private_if} {gw_private_ip} port {allocated_port} ->
		//		 {public_endpoint_ip} port {public_endpoint_port} udp
		//
		// Example:
		//  rdr znb1 192.168.0.1/32 port 60000 ->
		//			 136.180.0.150 port 9696 udp
		//
		// This redirect is for media
		//

		//
		//	Get 2 new port pairs to be used by internal endpoint
		// on the private side to reach external endpoint on the
		// public side.
		//	The first port pair is allocated from a private
		// interface. The internal endpoint will send its rtp
		// and RTCP data to these two ports, respectively.
		//	The second port pair is allocated from a public
		// interface. On the way out of the public interface
		// the source information will be altered to specify 
		// the public interface address and this second set 
		// of ports.
		//

		if ( (new_local_port = 
				_getnext_port( PortAlloc_ptr )) ==  (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating private "
						"MR ports on %s - portalloc_ptr 0x%08x\n",
						PrivateMRInterface,
						PortAlloc_ptr );	//xxx
			goto errorExit;
		}

		if ( (new_local_port2 = 
				_getnext_port( PortAlloc_ptr2 )) ==  (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating public MR ports "
						"on %s - portalloc_ptr 0x%08x\n",
						PublicMRInterface,
						PortAlloc_ptr2 );
			goto errorExit;
		}

		ports_allocated = 1;

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	PrivateMRInterface,
							PublicMRInterface,
							holeId,
							PrivateMRIp,
							new_local_port,
							mediaIpAddress,
							mediaPort,
							"rtp",
							&rtp_nat,
							PublicMRIp,
							new_local_port2,
							1 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 1 failed for Int2Ext UDP\n" );
			goto errorExit;
		}

		*returnedMediaIpAddress = PrivateMRIp;
		*returnedMediaPort = new_local_port;

		rtp_nat.rdr_defined = TRUE;

		//
		// Add a redirect for rtcp of the following format :
		//	rdr {private_if} {gw_private_ip} port {allocated_port} ->
		//		{private_endpoint_ip} port {private_endpoint_port} udp
		//
		// Example:
		//  rdr znb1 192.168.228.1/32 port 60001 ->
		//			 192.168.228.2 port 9696 udp

		if ( _AddRedirect(	PrivateMRInterface,
							PublicMRInterface,
							holeId,
							PrivateMRIp,
							( new_local_port + 1 ),
							mediaIpAddress,
							(mediaPort + 1),
							"rtcp",
							&rtcp_nat,
							PublicMRIp,
							( new_local_port2 + 1 ),
							1 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 2 failed for Int2Ext UDP\n" );
			goto errorExit;
		}

		rtcp_nat.rdr_defined = TRUE;
	}
	else
	if ( (	RemoteNatEnabled == TRUE && direction == FCE_eExt2Ext ) )
	{
		if ( PublicMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - Unable to Media Route"
						" Public to Public calls\n" );	//xxx
			trc_error(	MFCE,
						"       :  No Public MR interfaces configured\n" );
			goto errorExit;
		}

		media_gw_interface =
			&PublicMR_interface[ PublicMR_current ][0];

		media_gw_ip =
			PublicMR_ipaddr[ PublicMR_current ];

		PortAlloc_ptr = PublicMR_PortAlloc[ PublicMR_current ];

		if ( PublicMR_current < ( PublicMR_count - 1 ) )
			PublicMR_current++;
		else
			PublicMR_current = 0;

		//
		// Reflect external media and rtcp data streams from two 
		// external endpoints.
		//
		// The redirects defined here change the destination address
		// and port of the incoming packets from the specified
		// iserver address and port to that of the actual enpoint
		// to which they are destined.
		//

		//
		// Build a redirect of the following format :
		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
		//					{mediaIpAddress} port {mediaPort} udp
		//
		// Example:
		//  rdr znb0 204.180.228.217/32 port 60000 ->
		//			 207.13.113.75 port 9522 udp
		//

		//
		// This redirect is for media
		//

		//
		// Get new port to be used by an external endpoint
		// to reach another external endpoint.
		//

		if ( ( new_local_port = 
				_getnext_port( PortAlloc_ptr ) ) == (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating public "
						"MR ports on %s - portalloc_ptr 0x%08x\n",
						media_gw_interface,
						PortAlloc_ptr );	//xxx
			goto errorExit;
		}

		ports_allocated = 1;

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	media_gw_interface,
							media_gw_interface,		//xxx
							holeId,
							media_gw_ip,
							new_local_port,
							mediaIpAddress,
							mediaPort,
							"rtp",
							&rtp_nat,
							0,
							0,
							0 ) < 0 )
		
		{
    		trc_error(  MFCE,
                		"       : Redirect 1 failed for Ext2Ext UDP\n" );
			goto errorExit;
		}

		*returnedMediaIpAddress = media_gw_ip;
		*returnedMediaPort = new_local_port;

		rtp_nat.rdr_defined = TRUE;

		//
		// Build a redirect of the following format :
		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
		//					{rtcpIpAddress} port {rtcpPort} udp
		//
		// Example:
		//  rdr znb0 204.180.228.217/32 port 60001 ->
		//			 207.13.113.75 port 9523 udp
		//

		//
		// This redirect is for rtcp
		//

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	media_gw_interface,
							media_gw_interface,
							holeId,
							media_gw_ip,
							(new_local_port + 1),
							mediaIpAddress,
							(mediaPort + 1),
							"rtcp",
							&rtcp_nat,
							0,
							0,
							0 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 2 failed for Ext2Ext UDP\n" );
			goto errorExit;
		}

		rtcp_nat.rdr_defined = TRUE;
	}
	else
	if ( (	RemoteNatEnabled == TRUE && direction == FCE_eInt2Int ) )
	{
		if ( PrivateMR_count == 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - Unable to Media Route"
						" Private to Private calls\n" );	//xxx
			trc_error(	MFCE,
						"       :  No Private MR interfaces configured\n" );
			goto errorExit;
		}

		media_gw_interface =
			&PrivateMR_interface[ PrivateMR_current ][0];

		media_gw_ip =
			PrivateMR_ipaddr[ PrivateMR_current ];

		PortAlloc_ptr = PrivateMR_PortAlloc[ PrivateMR_current ];

		if ( PrivateMR_current < ( PrivateMR_count - 1 ) )
			PrivateMR_current++;
		else
			PrivateMR_current = 0;

		//
		// Reflect internal media and rtcp data streams from two 
		// internal endpoints.
		//
		// The redirects defined here change the destination address
		// and port of the incoming packets from the specified
		// iserver address and port to that of the actual enpoint
		// to which they are destined.
		//

		//
		// Build a redirect of the following format :
		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
		//					{mediaIpAddress} port {mediaPort} udp
		//
		// Example:
		//  rdr znb0 192.168.0.1/32 port 60000 ->
		//			 192.168.0.75 port 9522 udp
		//

		//
		// This redirect is for media
		//

		//
		// Get new port to be used by an internal endpoint
		// to reach another internal endpoint.
		//

		if ( ( new_local_port = 
				_getnext_port( PortAlloc_ptr ) ) == (uint16_t) 0 )
		{
			trc_error(	MFCE,
						"ERROR  : OpenMediaHole() - allocating private "
						"MR ports on %s - portalloc_ptr 0x%08x\n",
						media_gw_interface,
						PortAlloc_ptr );
			goto errorExit;
		}

		ports_allocated = 1;

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	media_gw_interface,
							media_gw_interface,
							holeId,
							media_gw_ip,
							new_local_port,
							mediaIpAddress,
							mediaPort,
							"rtp",
							&rtp_nat,
							0,
							0,
							0 ) < 0 )
		{
    		trc_error(  MFCE,
                		"       : Redirect 1 failed for Int2Int UDP\n" );
			goto errorExit;
		}

		*returnedMediaIpAddress = media_gw_ip;
		*returnedMediaPort = new_local_port;

		rtp_nat.rdr_defined = TRUE;

		// Build a redirect of the following format :
		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
		//					{rtcpIpAddress} port {rtcpPort} udp
		//
		// Example:
		//  rdr znb0 192.168.0.1/32 port 60001 ->
		//			 192.168.0.75 port 9522 udp
		//

		//
		// This redirect is for rtcp
		//

		// Add new redirect and fill in local structure 

		if ( _AddRedirect(	media_gw_interface,
							media_gw_interface,
							holeId,
							media_gw_ip,
							(new_local_port + 1),
							mediaIpAddress,
							(mediaPort + 1),
							"rtcp",
							&rtcp_nat,
							0,
							0,
							0 ) < 0 )

		{
    		trc_error(  MFCE,
                		"       : Redirect 2 failed for Int2Int UDP\n" );
			goto errorExit;
		}

		rtcp_nat.rdr_defined = TRUE;
	}

	holeId = AddMediaHole(	sessionId,
	       					&rtp_nat,
		       				&rtcp_nat,
							PortAlloc_ptr,
							new_local_port,
							PortAlloc_ptr2,
							new_local_port2 );

	unlockMutex( &mutex );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"OMH <- : hole %d, session %d\n",
				holeId,
				sessionId );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "nsfOpenMediaHole: exiting - hole ID %d, session Id %d\n",
				holeId,
				sessionId ));

	return( 0 );

errorExit:

	// Was a media nat redirect created ?

	if ( rtp_nat.rdr_defined == TRUE )
	{
		// Yes, Remove media nat translation for failed hole

		_RemoveRedirect( sessionId,
						-1,
						(void*) &rtp_nat.data,
						rtp_nat.rdr_string_1,
						rtp_nat.rdr_string_2,
						rtp_nat.rdr_string_3,
						"RTP" );
	}

	// Was a rtcp nat redirect created ?

	if ( rtcp_nat.rdr_defined == TRUE )
	{
		// Yes, Remove rtcp nat translation

		_RemoveRedirect(	sessionId,
							-1,
						(void*) &rtcp_nat.data,
						rtcp_nat.rdr_string_1,
						rtcp_nat.rdr_string_2,
						rtcp_nat.rdr_string_3,
						"RTCP" );
	}

	if ( ports_allocated )
	{
		if ( PortAlloc_ptr != NULL )
		{
			_free_ports(	PortAlloc_ptr,
							new_local_port );
		}

		if ( PortAlloc_ptr2 != NULL )
		{
			_free_ports(	PortAlloc_ptr2,
							new_local_port2 );
		}
	}

    trc_error(  MFCE,
                "OH  <- : session %d - Error\n",
                sessionId );

    unlockMutex( &mutex );
    return( -1 );
}

//
//	Function 	:
//		AddMediaHole()
//
//	Purpose		:
//
//		This function adds a new media IpHoleBlock_t
//		to the chain of IpHoleBlock structures and
//		fills it in with the data passed in as
//		arguments. It returns a uniquely assigned
//		integer that can be used to reference the
//		IpHoleBlock_t for update or deletion purposes.
//
//	Arguments	:
//
//		sessionId				- session id for the new pinhole
//
//		rtp_nat_ptr				- a pointer to a rtp ipf_nat_t for redirect
//
//		rtcp_nat_ptr			- a pointer to a rtcp ipf_nat_t for redirect
//
//		ipf_rule_ptr_1			- a pointer to a frentry_t for first rule
//
//		ipf_rule_ptr_2			- a pointer to a frentry_t for second rule
//
//		PortAlloc_ptr			- pointer to port allocation structure for
//									interface from which local ephemeral ports
//									were allocated.
//	
//		new_local_port			- first port of port pair allocated for hole
//
//		PortAlloc_ptr2			- pointer to port allocation structure for
//									second interface from which local ephemeral
//									ports were allocated. May not be allocated.
//	
//		new_local_port2			- first port of port pair allocated for second
//									interface
//									
//
//	Return Value :
//
//		assigned Id of hole			on success
//		0							on failure - if one cannot be allocated
//
static uint32_t
AddMediaHole(	uint32_t		sessionId,
				ipf_nat_t *		rtp_nat_ptr,
				ipf_nat_t *		rtcp_nat_ptr,
				PortAlloc *		PortAlloc_ptr,
				uint16_t		new_local_port,
				PortAlloc *		PortAlloc_ptr2,
				uint16_t		new_local_port2 )
{
	IpHoleBlock_t *ptr;

	ptr = IpHoleBlockAlloc( sessionId );

	if ( ptr == NULL )
		return( 0 );

	if ( rtp_nat_ptr->rdr_defined )
	{
		memcpy( &ptr->rtp_nat,
				rtp_nat_ptr,
				sizeof( ipf_nat_t ) );
	}

	if ( rtcp_nat_ptr->rdr_defined )
	{
		memcpy( &ptr->rtcp_nat,
				rtcp_nat_ptr,
				sizeof( ipf_nat_t ) );
	}

	ptr->PortAlloc_ptr = PortAlloc_ptr;
	ptr->allocated_port_pair = new_local_port;
	ptr->PortAlloc_ptr2 = PortAlloc_ptr2;
	ptr->allocated_port_pair2 = new_local_port2;

	// Add the new block to the HoleId hash table cache

	CacheInsert( HoleIdCache, ptr );

	// Add the new block to the SessionId hash table cache

	CacheInsert( SessionIdCache, ptr );

	return( ptr->assignedId );
}

//
//	Function 	:
//		OpenSignalingHole()
//
//	Purpose		:
//
//		This function opens a pinhole in the firewall and sets up
//		a nat translation entry in the firewall.
//
//	Arguments	:
//
//		protocol				
//							-	protocol used for signaling traffic
//								through this hole being opened
//
//								valid values are one of the following
//								enumerated types :
//									IPPROTO_TCP or IPPROTO_UDP
//
//		signalingIpAddress		
//							-	local ipv4 addr of hole
//
//								the remote endpoint will specify this
//								ip address as the destination ip in all
//								packets being sent to the MSW, associated
//								with this new hole.
//									For DMR, this will be the ip address
//								of a Logical interface servicing the
//								realm on which, the MSW listens for
//								signaling traffic
//									(host order)
//
//		signalingPort			
//							-	port number of hole
//
//								the remote endpoint will specify this 
//								port as the destination port in all
//								packets being sent to the MSW, associated
//								with this new hole.
//									For DMR, this will be a port allocated
//								on a Logical interface servicing the realm
//								on which, the MSW listens for signaling
//								traffic
//									(host order)
//
//		sessionId			
//							-	an optional session id that can be used
//								to tear down connections in the future
//
//		sessionIdLeg2		-	the session id of the other call leg
//
//		numTranslations		-	the number of TCP/RTP connections to allow,
//								to be used in the future
//
//		returnedMediaIpAddress
//							-	return value of media ip address
//
//		returnedMediaPort
//							-	return value of media port
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
static int
OpenSignalingHole(	uint32_t				protocol,
		   			uint32_t				signalingIpAddress,
		   			uint16_t				signalingPort,
		   			uint32_t				sessionId,
		   			uint32_t				sessionIdLeg2,
					int						numTranslations,
		   			uint32_t *				returnedMediaIpAddress,
		   			uint16_t *				returnedMediaPort )
{
	uint32_t	holeId;
	char		c_addr[32];

	ipf_pmhole_t	ipf_pmhole_signaling;

	memset(	&ipf_pmhole_signaling,
			(int) 0,
			sizeof( ipf_pmhole_t ) );

	*returnedMediaIpAddress = signalingIpAddress;
	*returnedMediaPort = signalingPort;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"OSH -> : \n" );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "OpenSignalingHole: entering\n" ));

	lockMutex( &mutex );

	holeId = lastAssignedId + 1;

	//
	// call nsf via ioctl to open specified signaling pinhole
	//

	if ( _OpenPinHole(	PublicInterface,
		   				signalingIpAddress,
						holeId,
						sessionId,
						signalingPort,
						protocol,
						&ipf_pmhole_signaling ) < 0 )
	{

		trc_error(  MFCE,
					"       : Error - failed to open local signaling hole\n" );
		trc_error(  MFCE,
					"       :   %s:%d %s on %s\n",
					FormatIpAddress(	signalingIpAddress,
										c_addr ),
					signalingPort,
					(( protocol == IPPROTO_UDP )? "UDP" : "TCP"),
					PublicInterface );
		goto errorExit; 
	}

	ipf_pmhole_signaling.port = signalingPort;
	ipf_pmhole_signaling.hole_defined = TRUE;

	// Allocate and save hole information for later removal

	holeId = AddSignalingHole(	sessionId,
				       			&ipf_pmhole_signaling );

	unlockMutex( &mutex );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"OSH <- : hole %d, session %d\n",
				holeId,
				sessionId );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "OpenSignalingHole: exiting - hole ID %d, session Id %d\n",
				holeId,
				sessionId ));

	return( 0 );

errorExit:

	// Does a portmap pinhole exist for signaling ?

	if ( ipf_pmhole_signaling.hole_defined )
	{
		// Yes, remove the port map hole

		_RemovePMHole(	-1,
						sessionId,
						&ipf_pmhole_signaling );
	}

    trc_error(  MFCE,
                "OSH <- : session %d - Error\n",
                sessionId );

    unlockMutex( &mutex );
    return( -1 );
}

//
//	Function 	:
//		AddSignalingHole()
//
//	Purpose		:
//
//		This function adds a new media IpHoleBlock_t
//		to the chain of IpHoleBlock structures and
//		fills it in with the data passed in as
//		arguments. It returns a uniquely assigned
//		integer that can be used to reference the
//		IpHoleBlock_t for update or deletion purposes.
//
//	Arguments	:
//
//		sessionId				- session id for the new pinhole
//
//		ipf_pmhole_signaling_ptr- a pointer to a ipf_pmhole_t specifying
//									an rtp hole in an interface
//
//	Return Value :
//
//		assigned Id of hole			on success
//		0							on failure - if one cannot be allocated
//
static uint32_t
AddSignalingHole(	uint32_t		sessionId,
					ipf_pmhole_t *	ipf_pmhole_signaling_ptr )
{
	IpHoleBlock_t *ptr;

	ptr = IpHoleBlockAlloc( sessionId );

	if ( ptr == NULL )
		return( 0 );

	if ( ipf_pmhole_signaling_ptr->hole_defined )
	{
		memcpy( &ptr->ipf_pmhole_signaling,
				ipf_pmhole_signaling_ptr,
				sizeof( ipf_pmhole_t ) );
	}

	// Add the new block to the HoleId hash table cache

	CacheInsert( HoleIdCache, ptr );

	// Add the new block to the SessionId hash table cache

	CacheInsert( SessionIdCache, ptr );

	return( ptr->assignedId );
}

//
//	Function 	:
//		IpfilterReopenHole()
//
//	Purpose		:
//
//		This function reopens a pinhole in the firewall and sets up
//		a nat translation entry in the firewall. It closes the hole
//		given by prevHoleId, and open a new pinhole, with the
//		nat translation giving the same port number as the closed hole.
//
//	Arguments	:
//
//		protocol			
//							-	one of :
//								IPPROTO_TCP, IPPROTO_UDP, or IPPROTO_RAW
//
//		mediaIpAddress		
//							-	ip address of endpoint that receives the packets
//
//		mediaPort			
//							-	port number of endpoint that receives the packets
//
//		direction			-	defines the traffic direction through the firewall
//								one of :
//								FCE_eInt2Int - nothing to do for now
//								FCE_eInt2Ext - traffic from inside the firewall
//									to get out (pass out rule)
//								FCE_eExt2Int - traffic from outside the firewall
//									to get out (pass in rule)
//								FCE_eExt2Ext - to be used in the future
//
//		sessionId			-	the session id of the call leg which is
//								changing.
//
//		sessionIdLeg2		-	the session id of the other call leg whose
//								media ip and port will not change.
//
//		numTranslations		-	the number of TCP/RTP connections to allow,
//								to be used in the future
//
//		returnedMediaIpAddress
//							-	return value of media ip address
//
//		returnedMediaPort
//							-	return value of media port
//
//		allocatePrivatePorts
//							-	if the value is 1, the ports returned could be
//									re-used in IpfilterReopenHole call
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
int
IpfilterReopenHole(	uint32_t				protocol,
			   		uint32_t				mediaIpAddress,
			   		uint16_t				mediaPort,
					eFCETrafficDirection	direction,
			   		uint32_t				sessionId,
			   		uint32_t				sessionIdLeg2,
					int						numTranslations,
			   		uint32_t *				returnedMediaIpAddress,
			   		uint16_t *				returnedMediaPort,
					int		       			allocatePrivatePorts )
{
//	IpHoleBlock_t *	idp, * idp2;
//	char			errorString[100];
//
//	uint32_t		holeId1, holeId2, mediaIpAddress2, mapip, mapip2;
//	uint16_t		original_local_port, original_local_port2, mediaPort2,
//					mapport, mapport2;
//	char *			media_gw_interface, * media_gw_interface2;
//	int32_t			media_gw_ip, media_gw_ip2, mapflag, mapflag2;
//
//	ipf_nat_t		rtp_nat,	rtcp_nat;
//	ipf_nat_t		rtp_nat2,	rtcp_nat2;
//
//	lockMutex( &mutex );
//
//	if ( direction == FCE_eExt2Int )
//	{
//		trc_error(	MFCE,
//					"ROH -> : Ext2Int\n" );
//		sprintf(	errorString,
//					"only valid for Ext2Ext call" );
//		goto errorExit_ROH;
//	}
//	else
//	if ( direction == FCE_eInt2Ext )
//	{
//		trc_error(	MFCE,
//					"ROH -> : Int2Ext\n" );
//		sprintf(	errorString,
//					"only valid for Ext2Ext call" );
//		goto errorExit_ROH;
//	}
//	else
//	if ( direction == FCE_eInt2Int )
//	{
//		trc_error(	MFCE,
//					"ROH -> : Int2Int\n" );
//		sprintf(	errorString,
//					"only valid for Ext2Ext call" );
//		goto errorExit_ROH;
//	}
//	else
//	if ( direction == FCE_eExt2Ext )
//	{
//		trc_debug(	MFCE,
//					NETLOG_DEBUG2,
//					"ROH -> : Ext2Ext\n" );
//	}
//
//	memset( &rtp_nat, (uint32_t) 0, sizeof( ipf_nat_t ) );
//	memset( &rtcp_nat, (uint32_t) 0, sizeof( ipf_nat_t ) );
//	memset( &rtp_nat2, (uint32_t) 0, sizeof( ipf_nat_t ) );
//	memset( &rtcp_nat2, (uint32_t) 0, sizeof( ipf_nat_t ) );
//
//	if ( (idp2 = GetIpHoleBlockForSessionId( sessionIdLeg2 ) ) == NULL )
//	{
//		holeId1 = -1;
//		sprintf( errorString, "no hole block found for sessionIdLeg2" );
//		goto errorExit_ROH;
//	}
//
//  	// Remove in rule and redirects for sessionId and add them 
//  	// again for the specified media ip and port using the same
//	// local ports
//
//	if ( (idp = GetIpHoleBlockForSessionId( sessionId ) ) != NULL )
//	{
//		holeId1 = idp->assignedId;
//		holeId2 = idp2->assignedId;
//
//		// Does a nat redirect exist for Leg 2 hole ?
//
//		if ( idp2->rtp_nat.rdr_defined == TRUE )
//		{
//			// Yes, remove associated nat translations
//
//			// Remove media nat translation
//
//			_RemoveRedirect(idp2->sessionId,
//							holeId2,
//							(void*) &idp2->rtp_nat.data,
//							idp2->rtp_nat.rdr_string_1,
//							idp2->rtp_nat.rdr_string_2,
//							idp2->rtp_nat.rdr_string_3,
//							"RTP" );
//
//			// Remove rtcp nat translation
//
//			_RemoveRedirect(idp2->sessionId,
//							idp2->assignedId,
//							(void*) &idp2->rtcp_nat.data,
//							idp2->rtcp_nat.rdr_string_1,
//							idp2->rtcp_nat.rdr_string_2,
//							idp2->rtcp_nat.rdr_string_3,
//							"RTCP" );
//		}
//		else
//		{
//			sprintf( 	errorString, 
//						"Leg 2 - no redirect defined" );
//			goto errorExit_ROH;
//		}
//
//		// Does a nat redirect exist for Target hole ?
//
//		if ( idp->rtp_nat.rdr_defined == TRUE )
//		{
//			// Yes, remove associated nat translations
//
//			// Remove media nat translation
//
//			_RemoveRedirect(idp->sessionId,
//							holeId1,
//							(void*) &idp->rtp_nat.data,
//							idp->rtp_nat.rdr_string_1,
//							idp->rtp_nat.rdr_string_2,
//							idp->rtp_nat.rdr_string_3,
//							"RTP" );
//
//			// Remove rtcp nat translation
//
//			_RemoveRedirect(idp->sessionId,
//							holeId1,
//							(void*) &idp->rtcp_nat.data,
//							idp->rtcp_nat.rdr_string_1,
//							idp->rtcp_nat.rdr_string_2,
//							idp->rtcp_nat.rdr_string_3,
//							"RTCP" );
//		}
//		else
//		{
//			sprintf(	errorString,
//						"Target Leg - no redirect defined" );
//			goto errorExit_ROH;
//		}
//
//		// Delay Re-adding of legs for at least 4 seconds
//		// to allow active redirects to age out in kernel
//
//		//millisleep( 5000 );
//
//		// Use same media interface as original rule and redirects
//
//		media_gw_interface = idp->rtp_nat.data.in_ifname;
//		media_gw_ip = ntohl( idp->rtp_nat.data.in_outip );
//		original_local_port = ntohs( idp->rtp_nat.data.in_pmin );
//		mapip = ntohl( idp->rtp_nat.data.in_map_srcip.s_addr );
//		mapport = ntohs( idp->rtp_nat.data.in_map_port );
//		mapflag = ( idp->rtp_nat.data.in_flags & IPN_XMAP )? 1 : 0 ;
//
//		media_gw_interface2 = idp2->rtp_nat.data.in_ifname;
//		media_gw_ip2 = ntohl( idp2->rtp_nat.data.in_outip );
//		original_local_port2 = ntohs( idp2->rtp_nat.data.in_pmin );
//		mediaPort2 = ntohs( idp2->rtp_nat.data.in_pnext );
//		mediaIpAddress2 = ntohl( idp2->rtp_nat.data.in_inip );
//		mapip2 = ntohl( idp2->rtp_nat.data.in_map_srcip.s_addr );
//		mapport2 = ntohs( idp2->rtp_nat.data.in_map_port );
//		mapflag2 = ( idp2->rtp_nat.data.in_flags & IPN_XMAP )? 1 : 0 ;
//
//		// Do leg 2 first
//		//
//		// Reflect external media and rtcp data streams from two 
//		// external endpoints.
//		//
//		// The redirects defined here change the destination address
//		// and port of the incoming packets from the specified
//		// iserver address and port to that of the actual enpoint
//		// to which they are destined.
//		//
//
//		//
//		// Build a redirect of the following format :
//		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
//		//					{mediaIpAddress} port {mediaPort} {protocol}
//		//
//		// Example:
//		//  rdr znb0 204.180.228.217/32 port 60000 ->
//		//			 207.13.113.75 port 9522 udp
//		//
//
//		//
//		// This redirect is for media
//		//
//
//		//
//		// Get new port to be used by an external endpoint
//		// to reach another external endpoint.
//		//
//
//		// modify existing redirect and tweaking local structure 
//
//		if ( _AddRedirect(	media_gw_interface2,
//							holeId2,
//							media_gw_ip2,
//							original_local_port2,
//							mediaIpAddress2,
//							mediaPort2,
//							"rtp",
//							&rtp_nat2,
//							mapip2,
//							mapport2,
//							mapflag2 ) < 0 )
//		{
//			sprintf(	errorString,
//						"AddRedirect() failed for sessionIdLeg2 (media)" );
//			goto errorExit_ROH;
//		}
//
//		rtp_nat2.rdr_defined = TRUE;
//
//		//
//		// Build a redirect of the following format :
//		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
//		//					{rtcpIpAddress} port {rtcpPort} {protocol}
//		//
//		// Example:
//		//  rdr znb0 204.180.228.217/32 port 60001 ->
//		//			 207.13.113.75 port 9523 udp
//		//
//
//		//
//		// This redirect is for rtcp
//		//
//
//		// Add redirect and fill in local structure 
//
//		if ( _AddRedirect(	media_gw_interface2,
//							holeId2,
//							media_gw_ip2,
//							(original_local_port2 + 1),
//							mediaIpAddress2,
//							(mediaPort2 + 1),
//							"rtcp",
//							&rtcp_nat2,
//							mapip2,
//							( mapport2 + 1 ),
//							mapflag2 ) < 0 )
//		{
//			sprintf(	errorString,
//						"AddRedirect() failed for sessionIdLeg2 (rtcp)" );
//			goto errorExit_ROH;
//		}
//
//		rtcp_nat2.rdr_defined = TRUE;
//
//		//
//		// Now setup target leg
//		//
//		// Reflect external media and rtcp data streams from two 
//		// external endpoints.
//		//
//		// The redirects defined here change the destination address
//		// and port of the incoming packets from the specified
//		// iserver address and port to that of the actual enpoint
//		// to which they are destined.
//		//
//
//		//
//		// Build a redirect of the following format :
//		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
//		//					{mediaIpAddress} port {mediaPort} {protocol}
//		//
//		// Example:
//		//  rdr znb0 204.180.228.217/32 port 60000 ->
//		//			 207.13.113.75 port 9522 udp
//		//
//
//		//
//		// This redirect is for media
//		//
//
//		//
//		// Get new port to be used by an external endpoint
//		// to reach another external endpoint.
//		//
//
//		// Add new redirect and fill in local structure 
//
//		if ( _AddRedirect(	media_gw_interface,
//							holeId1,
//							media_gw_ip,
//							original_local_port,
//							mediaIpAddress,
//							mediaPort,
//							"rtp",
//							&rtp_nat,
//							mapip,
//							mapport,
//							mapflag ) < 0 )
//		{
//			sprintf(	errorString,
//						"AddRedirect() failed for target Leg (media)" );
//			goto errorExit_ROH;
//		}
//
//		*returnedMediaIpAddress = media_gw_ip;
//		*returnedMediaPort = original_local_port;
//
//		rtp_nat.rdr_defined = TRUE;
//
//		//
//		// Build a redirect of the following format :
//		//	rdr {media_gw_if} {media_gw_ip} port {allocated_port} ->
//		//					{rtcpIpAddress} port {rtcpPort} {protocol}
//		//
//		// Example:
//		//  rdr znb0 204.180.228.217/32 port 60001 ->
//		//			 207.13.113.75 port 9523 udp
//		//
//
//		//
//		// This redirect is for rtcp
//		//
//
//		// Add new redirect and fill in local structure 
//
//		if ( _AddRedirect(	media_gw_interface,
//							holeId1,
//							media_gw_ip,
//							(original_local_port + 1),
//							mediaIpAddress,
//							(mediaPort + 1),
//							"rtcp",
//							&rtcp_nat,
//							mapip,
//							( mapport + 1 ),
//							mapflag ) < 0 )
//		{
//			sprintf(	errorString,
//						"AddRedirect() failed for target Leg (media)" );
//			goto errorExit_ROH;
//		}
//
//		rtcp_nat.rdr_defined = TRUE;
//
//		// Copy new definitions into existing session for Target Leg
//
//		memcpy( &idp->rtp_nat,
//				&rtp_nat,
//				sizeof( ipf_nat_t ) );
//
//		memcpy( &idp->rtcp_nat,
//				&rtcp_nat,
//				sizeof( ipf_nat_t ) );
//
//		// Copy new definitions into existing session for Leg 2
//
//		memcpy( &idp2->rtp_nat,
//				&rtp_nat2,
//				sizeof( ipf_nat_t ) );
//
//		memcpy( &idp2->rtcp_nat,
//				&rtcp_nat2,
//				sizeof( ipf_nat_t ) );
//	}
//	else
//	{
//		sprintf( errorString, "no hole block found for session" );
//		holeId1 = -1;
//		goto errorExit_ROH;
//	}
//
//	unlockMutex( &mutex );
//
//	trc_debug(	MFCE,
//				NETLOG_DEBUG2,
//				"ROH <- : hole %d, session %d\n",
//				holeId1,
//				sessionId );

	return(0);

//errorExit_ROH:
//
//	unlockMutex( &mutex );
//
//    trc_debug(  MFCE,
//                NETLOG_DEBUG2,
//                "ROH <- : session %d - Error - %s\n",
//                sessionId,
//				errorString );
//
//    return( -1 );
}

//
//	Function 	:
//		IpfilterCloseHole()
//
//	Purpose		:
//
//		This function closes a pinhole in the firewall
//		given the assigned holeId number.
//
//	Arguments	:
//
//		holeId	-  assigned holeId returned from IpfilterOpenHole()
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
int
IpfilterCloseHole( uint32_t holeId )
{
	IpHoleBlock_t *	idp;
	uint32_t		sessionId;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"CH  -> :\n" );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "IpfilterCloseHole: entering\n" ));

	lockMutex( &mutex );

	// get the assigned id block

	idp = GetIpHoleBlockForHoleId( holeId );

	if ( idp == NULL )
	{
		NETERROR(	MFCE,
				 	("IpfilterCloseHole: Cannot find assigned ID %d in list\n",
				  	holeId ) );
		unlockMutex( &mutex );
		return( -1 );
	}

	sessionId = idp->sessionId;

	_CloseHole( idp );

	DeleteIpHoleBlock( idp );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"CH  <- : hole %d, session %d\n",
				holeId,
				sessionId );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "IpfilterCloseHole: exiting - hole ID %d, session Id %d\n",
			  	holeId,
				sessionId ));

	unlockMutex( &mutex );

	return( 0 );
}

//
//	Function 	:
//		IpfilterCloseSession()
//
//	Purpose		:
//
//		This function closes all pinholes associated with
//		the specified sessionId.
//
//	Arguments	:
//
//		sessionId	-  sessionId of holes to be deleted.
//
//	Return Value :
//
//			 0		always
//
int
IpfilterCloseSession( uint32_t sessionId )
{
	char	c_addr[32];

	lockMutex( &mutex );

	DeleteIpHoleBlockForSession( sessionId );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
			 	("Successfully closed session %d on firewall %s\n",
				sessionId,
				FormatIpAddress(	PrivateIp,
										c_addr ) ) );

	unlockMutex( &mutex );

	return( 0 );
}

//
//	Function 	:
//		tweak_ipfilter_ipaddr()
//
//	Arguments	:
//		primary_vip		character string containing primary vip. primary vip
//						is the ip address which should be used for the public
//						address in rule construction in a redundant
//						configuration.
//
//		secondary_vip	character string containing secondary vip. secondary vip
//						is the ip address which should be used for the private
//						address in rule construction in a redundant
//						configuration.
//
//	Purpose		:
//
//		This function is called if we have a redundant ipfilter
//		configuration. The ip address pulled off the interface
//		will not work in this case. When the local iserver is
//		notified that it is the active iserver the vips that it
//		services are updated here.
//
//	Return Value:
//		none
//
void
tweak_ipfilter_ipaddr(	char *	primary_vip,
						char *	secondary_vip )
{
	ulong_t             addr;
	struct hostent      he;
	struct hostent *    hp = &he;
	char                buffer[256];
	char **             p;
	int                 error = 0;
	struct in_addr      in;
        char                addrbuf[INET_ADDRSTRLEN];
	char                outbuf[256];
	fr_interface_vip_t	fr_vip;

	// Got a primary vip ?

	if ( 	strlen( primary_vip ) > 0 )
	{
		// Yes, Is primary_vip an inet address ?

		if ( (int)( addr = inet_addr( primary_vip ) ) == -1 )
		{
			// No, Let gethostbyname_r() get the address for us
			// if it can be resolved.

			gethostbyname_r(    primary_vip,
								hp,
								buffer,
								256,
								&error );

			if ( error != 0 )
			{
				NETERROR(	MFCE,
							("tweak_ipfilter_ipaddr() : error 1 "
							": unable to resolve ip address for primary vip %s\n",
							primary_vip ));
				return;
			}

			if (hp == NULL)
			{
				NETERROR(	MFCE,
							("tweak_ipfilter_ipaddr() : error 2 "
							": unable to resolve ip address for primary vip %s\n",
							primary_vip ));
				return;
			}

			p = hp->h_addr_list;

			(void) memcpy( &in.s_addr, *p, sizeof( in.s_addr ) );

			sprintf(	outbuf,
						" FCE : tweak_ipfilter_ipaddr() : Primary vip   %s\n",
						inet_ntop( AF_INET, &in , addrbuf, INET_ADDRSTRLEN));

			NETERROR(	MFCE,
						("%s\n", outbuf ));

			memset( &fr_vip, 0, sizeof( fr_interface_vip_t ) );

			strcpy( fr_vip.ifname, PublicInterface );
			fr_vip.vip = (uint32_t) in.s_addr;

			if ( ioctl( ipf_fd, SIOCSETIVIP, &fr_vip ) < 0 )
			{
				NETERROR(	MFCE,
					("Error setting interface vip for public interface %s\n",
					PublicInterface ));
			}

			PublicIp = ntohl( in.s_addr );
		}
		else
		{
			// Yes, primary_vip is an ip address - use FormatIpAddress() to convert
			// address to ascii.

			sprintf(	outbuf,
						" FCE : tweak_ipfilter_ipaddr() : Primary vip     %s\n",
						FormatIpAddress(	htonl( addr ), buffer ) );
			NETERROR(	MFCE,
						("%s\n", outbuf ));

			memset( &fr_vip, 0, sizeof( fr_interface_vip_t ) );

			strcpy( fr_vip.ifname, PublicInterface );
			fr_vip.vip = (uint32_t) addr;

			if ( ioctl( ipf_fd, SIOCSETIVIP, &fr_vip ) < 0 )
			{
				NETERROR(	MFCE,
					("Error setting interface vip for public interface %s\n",
					PublicInterface ));
			}

			PublicIp = ntohl( addr );
		}
	}

	// Got a secondary vip ?

	if ( 	strlen( secondary_vip ) > 0 )
	{
		// Yes, Is secondary_vip an inet address ?

		if ( (int)( addr = inet_addr( secondary_vip ) ) == -1 )
		{
			// No, Let gethostbyname_r() get the address for us
			// if it can be resolved.

			gethostbyname_r(    secondary_vip,
								hp,
								buffer,
								256,
								&error );

			if ( error != 0 )
			{
				NETERROR(	MFCE,
							("tweak_ipfilter_ipaddr() : error 1 "
							": unable to resolve ip address for secondary vip %s\n",
							secondary_vip ));
				return;
			}

			if (hp == NULL)
			{
				NETERROR(	MFCE,
							("tweak_ipfilter_ipaddr() : error 2 "
							": unable to resolve ip address for secondary vip %s\n",
							secondary_vip ));
				return;
			}

			p = hp->h_addr_list;

			(void) memcpy( &in.s_addr, *p, sizeof( in.s_addr ) );

			sprintf(	outbuf,
						" FCE : tweak_ipfilter_ipaddr() : Secondary vip   %s\n",
						inet_ntop( AF_INET, & in , addrbuf, INET_ADDRSTRLEN));

			NETERROR(	MFCE,
						("%s\n", outbuf ));

			memset( &fr_vip, 0, sizeof( fr_interface_vip_t ) );

			strcpy( fr_vip.ifname, PrivateInterface );
			fr_vip.vip = (uint32_t) in.s_addr;

			if ( ioctl( ipf_fd, SIOCSETIVIP, &fr_vip ) < 0 )
			{
				NETERROR(	MFCE,
					("Error setting interface vip for public interface %s\n",
					PrivateInterface ));
			}

			PrivateIp = ntohl( in.s_addr );
		}
		else
		{
			// Yes, secondary_vip is an ip address - use FormatIpAddress() to convert
			// address to ascii.

			sprintf(	outbuf,
						" FCE : tweak_ipfilter_ipaddr() : Secondary vip   %s\n",
						FormatIpAddress(	htonl( addr ), buffer ) );
			NETERROR(	MFCE,
						("%s\n", outbuf ));

			memset( &fr_vip, 0, sizeof( fr_interface_vip_t ) );

			strcpy( fr_vip.ifname, PublicInterface );
			fr_vip.vip = (uint32_t) addr;

			if ( ioctl( ipf_fd, SIOCSETIVIP, &fr_vip ) < 0 )
			{
				NETERROR(	MFCE,
					("Error setting interface vip for public interface %s\n",
					PublicInterface ));
			}

			PrivateIp = ntohl( addr );
		}
	}

	return;
}

//
//	Function 	:
//		IpfilterGetPublicAddress()
//
//	Purpose		:
//
//		This function returns the public ip address
//		if local nat is on, otherwise returns ip addr
//		passed in. Since the address is passed in in
//		network order if nat is enabled we must return
//		our address in network order!! This simplifies
//		the caller's logic
//
//	Arguments	:
//
//		addr	-  an ip address passed by caller.
//
//	Return Value :
//
//		fceConfigOurIpAddr	if local nat is enabled
//		addr			if local nat is not enabled
//
uint32_t
IpfilterGetPublicAddress( uint32_t addr )
{
	if ( LocalNatEnabled == TRUE )
		return( htonl( fceConfigOurIpAddr?fceConfigOurIpAddr:PublicIp ) );

	return( addr );
}

//
//	Function 	:
//		IpfilterGetPrivateAddress()
//
//	Purpose		:
//
//		This function returns the private ip address
//		PrivateIP value is returned, caller must check
//		whether it is a valid value.
//
//	Arguments	:
//		none
//
//	Return Value :
//		private IP address
//
//
uint32_t
IpfilterGetPrivateAddress(void)
{
	return( htonl(PrivateIp) );
}

//>*****************************************
//>*
//>* Definitions of Local entry points
//>*
//>*****************************************

//
//	Function 	:
//		_IpFilter_reload()
//
//	Purpose		:
//
//		This function reloads the virgin ipf 
//		configuration from closing any extraneous
//		holes and getting rid of any nat translations.
//		This is done on iserver startup and shutdown.
//
//	Arguments	:
//
//		None.
//
//	Return Value :
//
//		None.
//
static void
_IpFilter_reload(void)
{
	char *			path = "/etc/init.d/ipfboot";
	char *			cmd = "/etc/init.d/ipfboot reload";
	char 			buf[256];
	FILE *			ptr;
	struct stat		statbuf;

	if ( stat( path, (struct stat *) &statbuf ) < 0 ) // does script exist ?
	{
		// No, log error and return
		NETERROR( 	MFCE, 
		   			("Ipfilter_reload : file, \"%s\", not found "
					"- no reload performed\n",
					path ));
		return;
	}

	if ( !( statbuf.st_mode & S_IXUSR ) )	// is script executable ?
	{
		// No, log error and return

		NETERROR(	MFCE,
		   			("Ipfilter_reload : file, \"%s\", not executable "
					"- no reload performed\n",
					path ));
		return;
	}

	if ((ptr = popen(cmd, "r")) != NULL)
	{
   		while (fgets(buf, 256, ptr) != NULL)
		{
			NETINFOMSG( MFCE,
						("Ipfilter_reload : %s\n", buf ));
		}
		pclose( ptr );
	}

	return;
}

//
//	Function 	:
//		_CloseHole()
//
//	Purpose		:
//
//		This helper function issues the nessesary ioctls to
//		actually close the hole given a pointer to the hole block
//		describing it.
//
//	Arguments	:
//
//		idp	-  IpHoleBlock_t * of hole to be closed
//
//	Return Value :
//
//		None.
//
static void
_CloseHole(	IpHoleBlock_t *	idp )
{
	// Does a nat redirect exist for the hole ?

	if ( idp->rtp_nat.rdr_defined == TRUE )
	{
		// Yes, remove associated nat translations

		// Remove media nat translation

		_RemoveRedirect(idp->sessionId,
						idp->assignedId,
						(void*) &idp->rtp_nat.data,
						idp->rtp_nat.rdr_string_1,
						idp->rtp_nat.rdr_string_2,
						idp->rtp_nat.rdr_string_3,
						"RTP" );

	}

	if ( idp->rtcp_nat.rdr_defined == TRUE )
	{
		// Remove rtcp nat translation

		_RemoveRedirect(idp->sessionId,
						idp->assignedId,
						(void*) &idp->rtcp_nat.data,
						idp->rtcp_nat.rdr_string_1,
						idp->rtcp_nat.rdr_string_2,
						idp->rtcp_nat.rdr_string_3,
						"RTCP" );
	}

	// Does a portmap pinhole exist for signaling ?

	if ( idp->ipf_pmhole_signaling.hole_defined )
	{
		// Yes, remove the port map hole


		_RemovePMHole(	idp->assignedId,
						idp->sessionId,
						&idp->ipf_pmhole_signaling );
	}

	// Was a port pair allocated for this port ?

	if (	idp->allocated_port_pair > largest_anon_port 
			&& idp->PortAlloc_ptr != (PortAlloc*) NULL )
	{
		// Yes, Call _free_ports to remove the port pair for the
		// interface identified bye idp->PortAlloc_ptr

		_free_ports(	idp->PortAlloc_ptr,
						idp->allocated_port_pair );
	}

	if (	idp->allocated_port_pair2 > largest_anon_port 
			&& idp->PortAlloc_ptr2 != (PortAlloc*) NULL )
	{
		// Yes, Call _free_ports to remove the port pair for the
		// interface identified bye idp->PortAlloc_ptr2

		_free_ports(	idp->PortAlloc_ptr2,
						idp->allocated_port_pair2 );
	}

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("closed hole %d, session %d\n",
				idp->assignedId,
				idp->sessionId ));

	return;
}

//
//	Function 	:
//		_RemoveRedirect()
//
//	Purpose		:
//
//		This helper function issues the nessesary ioctls to
//		actually remove a redirect associated with a pinhole
//		given a pointer to the specified information
//
//	Arguments	:
//
//		sessionId		-	sessionId in which rule is contained
//
//		holeId			-	holeId of redirect
//
//		ipnat_ptr		-	pointer to ipnat_t or ipnat64_t for
//							redirect
//
//		rule_string_1	-	pointer to character string containing
//							first line of redirect description
//
//		rule_string_2	-	pointer to character string containing
//							second line of redirect description
//
//		ctype			-	character string containing redirect type
//							"RTP" or "RTCP"
//
//	Return Value :
//
//		None.
//
//
static void
_RemoveRedirect(	uint32_t	sessionId,
					uint32_t	holeId,
					void *		ipnat_ptr,
					char *		rdr_string_1,
					char *		rdr_string_2,
					char *		rdr_string_3,
					char *		ctype )
{
	uint32_t		lerrno;

	IPNAT_T *		nat_entry_ptr = (IPNAT_T*) ipnat_ptr;
	
	// Remove redirect

	if ( ioctl( ipnat_fd, SIOCRMNAT, &nat_entry_ptr ) < 0 )
	{
		lerrno = errno;

		trc_error(	MFCE,
			"       : ERROR ioctl RMNAT (%s) - hole %d, err : %s\n",
			ctype,
			holeId,
			strerror( lerrno ) );

		NETERROR(	MFCE,
					( "RemoveRedirect() : Error removing "
					"nat translation - holeId %d - (%s) err : %s\n",
					holeId,
					ctype,
					strerror( lerrno ) ));
	}
	else
	{
		NETDEBUG(	MFCE,
					NETLOG_DEBUG4,
					( "       : removed - NAT media redirect translation\n" ));

		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"       : removed - NAT media redirect translation\n" );

		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"       :   %s\n", 
					rdr_string_1 );

		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"       :   %s\n", 
					rdr_string_2 );

		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"       :   %s\n", 
					rdr_string_3 );
	}

	return;
}


//
//	Function 	:
//		IpHoleBlockAlloc()
//
//	Purpose		:
//
//		This function allocates free storage for a new
//		hole.
//
//	Arguments	:
//
//		SessionId		sessionId assosiated with hole being allocated
//
//	Return Value :
//
//		NULL				on failure - free storage exahauted
//		IpHoleBlock_t *		on success
//
static IpHoleBlock_t *
IpHoleBlockAlloc( uint32_t	sessionId )
{
	IpHoleBlock_t *	ptr;

	ptr = (IpHoleBlock_t *) malloc(sizeof(IpHoleBlock_t));

	if (ptr == NULL)
	{
		NETERROR(	MFCE,
					("Cannot allocate IDBlock\n"));
		return( NULL );
	}

	memset( ptr, (int32_t) 0, sizeof( IpHoleBlock_t ) );

	lastAssignedId++;

	if (lastAssignedId == -1)
		lastAssignedId = 1;

	ptr->assignedId = lastAssignedId;
	ptr->sessionId = sessionId;

	return( ptr );
}

//
//	Function:
//		_getnext_port()
//
//	Purpose		:
//
//		This function is called to allocate a pair of ports for
//		one leg of a call.
//
//	Arguments	:
//		PortAlloc_ptr	Pointer to port allocation structure for
//						interface from which ports are to be allocated.
//
//	Return value :
//
//		port value		integer specifying the first port in a 
//						consequtive pair of ports has been allocated.
//
//		0				on error condition when unable to allocate a
//						port pair
//
static uint16_t			
_getnext_port(	PortAlloc *	PortAlloc_ptr )
{
	uint16_t	return_value = 0;
	int			allocated_pair;

	if ( PortAlloc_ptr == (PortAlloc*) NULL )
	{
		trc_error(	MFCE,
					"ERROR : _getnext_port() called with NULL PortAlloc_ptr\n" );
		return( return_value );
	}

	if ( ( allocated_pair =
			allocPort( PortAlloc_ptr )) == -1 )
	{
		return( return_value );
	}

	return_value = (uint16_t) allocated_pair;

	return( return_value );
}


//
//	Function:
//		_free_ports()
//
//	Purpose		:
//
//		This function is called at the end of a call to free up
//		the port pair used by one leg of a call. It is called once
//		for each leg of the call to free up the RTP and RTCP port
//		associated with the call.
//
//	Arguments	:
//
//		PortAlloc_ptr	Pointer to port allocation structure for
//						interface from which ports are to be free'd.
//
//		port			first port in pair allocated via _getnext_port()
//
static void				
_free_ports(	PortAlloc * PortAlloc_ptr,
				uint16_t	port )
{
	int		allocated_pair = port;

	if ( PortAlloc_ptr == (PortAlloc*) NULL )
	{
		trc_error(	MFCE,
					"ERROR : _free_ports() called with NULL PortAlloc_ptr "
					"for port pair %d\n",
					port );
		NETERROR( 	MFCE,
					("ERROR : _free_ports() called with NULL PortAlloc_ptr "
					"for port pair %d\n",
					port) );
		return;
	}

	if ( allocated_pair != 0 )
	{
		freePort(	PortAlloc_ptr,
					allocated_pair );
	}
}

//
// returns the IpHoleBlock given its assigned holeid, NULL if not present
//
static IpHoleBlock_t *
GetIpHoleBlockForHoleId( uint32_t assignedId )
{
	IpHoleBlock_t *	ptr;
	char *			fn = "GetIpHoleBlockForHoleId():";

	if ( ( ptr = CacheGet( HoleIdCache, &assignedId ) ) == NULL )
	{
		NETERROR(	MFCE,
					( "%s error finding cache entry for hole id %d\n",
					fn,
					assignedId ));
	}

	return( ptr );
}

//
// returns the IpHoleBlock associated with this assigned id, NULL if not present
//
static IpHoleBlock_t *
GetIpHoleBlockForSessionId( uint32_t sessionId )
{
	IpHoleBlock_t *	ptr;

	ptr = CacheGet( SessionIdCache, &sessionId );

	return( ptr );
}

//
// deletes the given IpHoleBlock from the internal list
//
static void
DeleteIpHoleBlock( IpHoleBlock_t * ptr )
{
	IpHoleBlock_t *	retHole;
	char *			fn = "DeleteIpHoleBlock():";

	// CacheDelete returns a chain of elements with the same key

	if ( ( retHole = CacheDelete( HoleIdCache, &ptr->assignedId ) ) == NULL )
	{
		NETERROR(	MFCE, 
					( "%s Cannot find assigned hole id %d in holeid cache\n",
					fn,
					ptr->assignedId ));
	}

	ListgDelete( ptr, SESSIONID_LIST_OFFSET );
	SessionIdCache->nitems--;

	free( ptr );
}

//
// deletes all the IpHoleBlock structures associated with this session id
//
static void
DeleteIpHoleBlockForSession( uint32_t sessionId )
{
	IpHoleBlock_t *	idp;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"DS  -> : session %d\n",
				sessionId );

	while ( (idp = GetIpHoleBlockForSessionId( sessionId ) ) != NULL )
	{
		_CloseHole( idp );
		DeleteIpHoleBlock( idp );
	}

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"DS  <- : session %d\n",
				sessionId );
}

//
// initialize the local config parameters to a known state
//
static void
ConfigInit(void)
{
	PrivateIp = 0;
	PublicIp = 0;
	PublicMR_count = 0;
	PrivateMR_count = 0;
	phys_iface_count = 0;
	memset( PrivateInterface, (int32_t) 0, 256 );
	memset( PublicInterface, (int32_t) 0, 256 );
	LocalNatEnabled = FALSE;
	RemoteNatEnabled = FALSE;
}

//
//	Function 	:
//		ReadVars()
//
//	Arguments	:
//
//		None.
//
//	Purpose		:
//
//		This function is called to read and parse the contents of 
//		the nsf.cfg or ipfilter.cfg file in /usr/local/nextone/bin.
//		The values are assigned to global static variables in the
//		ipfilter.c file.
//
//	Return Value:
//		None.
//
static void
ReadVars( void )
{
	char				c_addr[32];
	struct ifi_info *	ifi_ptr;
	char *				curname = NULL;
	int					i;
	fr_interface_type_t	fr_it;
	ipf_config_t		config_data;

	#define NUM_KEYS	5
	#define VALUE_LEN	256

	// init all values to a known original 

	ConfigInit();

	// Build list of physical interfaces

	for ( i = 0; i < MAX_INTERFACES; i++ )
	{
		if ( ( curname = GetNextIfname( ifihead, curname ) ) == NULL )
			break;

		if ( !strncmp( curname, "lo", 2 ) )
			continue;

		if ( (ifi_ptr =
			findIfByIfname( ifihead, curname )) == (struct ifi_info *) NULL )
		{
			NETERROR(	MFCE,
				 ("Error getting ifi_ptr for interface %s\n",
						curname ));
		}

		strcpy( &phys_ifaces[ phys_iface_count ][ 0], curname );

		in_group[ phys_iface_count ] = ifi_ptr->ifi_index;
		out_group[ phys_iface_count ] = ifi_ptr->ifi_index * 10;

		phys_iface_count++;
	}

	if ( NetLogStatus[ MFCE ] & NETLOG_DEBUG2 )
	{
		NETINFOMSG(	MFCE,
					("Ipfilter Network Interfaces:\n"));

		for ( i = 0; i < phys_iface_count; i++ )
		{
			NETINFOMSG(	MFCE,
					("     interface %1d: \"%s\", in_group %3d, out_group %3d\n",
						i,
						&phys_ifaces[ i ][ 0 ],
						in_group[ i ],
						out_group[ i ] ));
		}
	}

	if ( IpfilterReadConfig( &config_data ) < 0 )
		return;

	// store the config into the local variables 

	if ( strlen( config_data.PrivateInterface ) )
	{
		if ( ( ifi_ptr = 
			findIfByIfname( ifihead, config_data.PrivateInterface ) )
							!= (struct ifi_info *) NULL )
		{
			memset( &fr_it, 0, sizeof( fr_interface_type_t ) );
			strcpy( fr_it.ifname, config_data.PrivateInterface );
			fr_it.itype = IPF_ITYPE_PRIVATE;

			if ( ioctl( ipf_fd, SIOCSETITYPE, &fr_it ) < 0 )
			{
				NETERROR(	MFCE,
							("Error setting interface type for private interface, %s, ignoring\n",
							config_data.PrivateInterface ));
			}
			else
			{
				PrivateInterface_PortAlloc = initPortAllocation( (largest_anon_port + 1),
															 65534 );

				if ( PrivateInterface_PortAlloc == (PortAlloc*) NULL )
				{
					NETERROR(	MFCE,
								("Error allocating ports for private "
								"interface, %s - initPortAllocation() - ignoring\n",
								config_data.PrivateInterface ));
				}
				else
				{
					PrivateIp = ntohl( ifi_ptr->ifi_addr->sin_addr.s_addr );
					strcpy( PrivateInterface, config_data.PrivateInterface );
				}
			}
		}
	}

	if ( strlen( config_data.PublicInterface ) )
	{
		if ( ( ifi_ptr = 
			findIfByIfname( ifihead, config_data.PublicInterface ) )
							!= (struct ifi_info *) NULL )
		{
			memset( &fr_it, 0, sizeof( fr_interface_type_t ) );
			strcpy( fr_it.ifname, config_data.PublicInterface );
			fr_it.itype = IPF_ITYPE_PUBLIC;

			if ( ioctl( ipf_fd, SIOCSETITYPE, &fr_it ) < 0 )
			{
				NETERROR(	MFCE,
							("Error setting interface type for public interface %s\n",
							config_data.PublicInterface ));
			}
			else
			{
				PublicInterface_PortAlloc = initPortAllocation( (largest_anon_port + 1),
																65534 );

				if ( PublicInterface_PortAlloc == (PortAlloc*) NULL )
				{
					NETERROR(	MFCE,
								("Error allocating ports for public "
								"interface, %s - initPortAllocation() - ignoring\n",
								config_data.PublicInterface ));
				}
				else
				{
					PublicIp = ntohl( ifi_ptr->ifi_addr->sin_addr.s_addr );
					strcpy( PublicInterface, config_data.PublicInterface );
				}
			}
		}
	}

	LocalNatEnabled = config_data.LocalNatEnabled;

	RemoteNatEnabled = config_data.RemoteNatEnabled;

	for ( i = 0; i < config_data.PublicMR_count; i++ )
	{
		if ( ( ifi_ptr = 
			findIfByIfname( ifihead, &config_data.PublicMR_interface[ i ][0] ) )
							!= (struct ifi_info *) NULL )
		{
			memset( &fr_it, 0, sizeof( fr_interface_type_t ) );
			strcpy( fr_it.ifname, &config_data.PublicMR_interface[ i ][0] );
			fr_it.itype = IPF_ITYPE_MEDIA_ROUTE;
			fr_it.largest_anon_udp_port = (uint16_t) largest_anon_port;

			if ( ioctl( ipf_fd, SIOCSETITYPE, &fr_it ) < 0 )
			{
				NETERROR(	MFCE,
							("Error setting interface type for Public MR interface %s - ignoring\n",
							&config_data.PublicMR_interface[ i ][0] ));
				continue;
			}

			PublicMR_ipaddr[ PublicMR_count ] =
				ntohl( ifi_ptr->ifi_addr->sin_addr.s_addr );

			strcpy( &PublicMR_interface[ PublicMR_count ][0],
					&config_data.PublicMR_interface[ i ][0] );

			PublicMR_PortAlloc[ PublicMR_count ] =
							initPortAllocation( (largest_anon_port + 1),
												65534 );

			if ( PublicMR_PortAlloc[ PublicMR_count ] == (PortAlloc*) NULL )
			{
				NETERROR(	MFCE,
				("Error allocating ports for Public MR "
				"interface, %s - initPortAllocation() - ignoring\n",
				&config_data.PublicMR_interface[ i ][0] ));
				continue;
			}

			PublicMR_count++;
		}
	}

	for ( i = 0; i < config_data.PrivateMR_count; i++ )
	{
		if ( ( ifi_ptr = 
			findIfByIfname( ifihead,
							&config_data.PrivateMR_interface[ i ][0] ) )
							!= (struct ifi_info *) NULL )
		{
			memset( &fr_it, 0, sizeof( fr_interface_type_t ) );
			strcpy( fr_it.ifname, &config_data.PrivateMR_interface[ i ][0] );
			fr_it.itype = IPF_ITYPE_MEDIA_ROUTE;

			if ( ioctl( ipf_fd, SIOCSETITYPE, &fr_it ) < 0 )
			{
				NETERROR(	MFCE,
							("Error setting interface type for "
							 "Private MR interface %s - ignoring\n",
							&config_data.PrivateMR_interface[ i ][0] ));
				continue;
			}

			PrivateMR_ipaddr[ PrivateMR_count ] =
				ntohl( ifi_ptr->ifi_addr->sin_addr.s_addr );

			strcpy( &PrivateMR_interface[ PrivateMR_count ][0],
					&config_data.PrivateMR_interface[ i ][0] );

			PrivateMR_PortAlloc[ PrivateMR_count ] =
							initPortAllocation( (largest_anon_port + 1),
												65534 );

			if ( PrivateMR_PortAlloc[ PrivateMR_count ] == (PortAlloc*) NULL )
			{
				NETERROR(	MFCE,
				("Error allocating ports for Private MR "
				"interface, %s - initPortAllocation() - ignoring\n",
				&config_data.PrivateMR_interface[ i ][0] ));
				continue;
			}

			PrivateMR_count++;
		}
	}

	NETINFOMSG(	MFCE,
				("Ipfilter Physical Interface Configuration:\n" ));

	NETINFOMSG(	MFCE,
				(" Public Interface   %s, ipaddr %-15s, remotenat %s\n", 
				PublicInterface,
				FormatIpAddress(	PublicIp,
									c_addr ),
				(RemoteNatEnabled == TRUE)? "YES": "NO" ) );

	if ( PrivateIp != 0 )
	{
		NETINFOMSG(	MFCE,
				(" Private Interface  %s, ipaddr %-15s, localnat  %s\n", 
					PrivateInterface,
					FormatIpAddress(	PrivateIp,
										c_addr ),
					(LocalNatEnabled == TRUE)? "YES": "NO" ) );
	}

	for ( i = 0; i < PublicMR_count; i++ )
	{
		NETINFOMSG(	MFCE,
				(" Public  Media Routing Interface %1d  %s, ipaddr %-15s\n",
				i,
				&PublicMR_interface[i][0],
				FormatIpAddress(	PublicMR_ipaddr[i],
									c_addr ) ));
	}

	for ( i = 0; i < PrivateMR_count; i++ )
	{
		NETINFOMSG(	MFCE,
				(" Private Media Routing Interface %1d  %s, ipaddr %-15s\n",
				i,
				&PrivateMR_interface[i][0],
				FormatIpAddress(	PrivateMR_ipaddr[i],
									c_addr ) ));
	}
}

//
//	Function 	:
//		IpfilterReadConfig()
//
//	Arguments	:
//
//		config_data		pointer to ipf_config_t structure to
//						be filled in by routine.
//
//	Purpose		:
//
//		This function is called to read and parse the contents of 
//		the nsf.cfg or ipfilter.cfg file in /usr/local/nextone/bin.
//		The values are assigned to the config_data structure passed
//		into the function and returned to caller.
//
//	Return Value:
//		 0 	on success
//		-1	on error
//
static int32_t
IpfilterReadConfig( ipf_config_t *	config_data )
{
	struct ifi_info *	ifi_ptr;
	char *				token;
	char *				placeholder;
	struct sockaddr_in	public_ip_address;
	struct sockaddr_in	public_ip_netmask;
	struct sockaddr_in	private_ip_address;
	struct sockaddr_in	private_ip_netmask;

	#define NUM_KEYS	5
	#define VALUE_LEN	256

	char *keys[NUM_KEYS] = 
	{	"fwPrivateInterface",
		"fwPublicInterface",
		"localNatEnabled",
		"remoteNatEnabled",
		"mediaRouteInterfaces",
	};

	char 			values[NUM_KEYS][VALUE_LEN];
	char			media_interface[256];	// Media routing interfaces

	memset( config_data, (int) 0, sizeof( ipf_config_t ) );

	// read the configuration 

	NETDEBUG(	MFCE,
				NETLOG_DEBUG3,
				("reading configuration file, nsf.cfg\n"));

	if ( ReadConfigFile(	"/usr/local/nextone/bin/nsf.cfg",
							NUM_KEYS,
							keys,
							values,
		 					VALUE_LEN ) == FALSE )
	{
		NETDEBUG(	MFCE,
					NETLOG_DEBUG3,
					("cannot read config file, nsf.cfg\n"));

		return(-1);
	}

	// store the config into the local variables 

	if ( strlen( values[0] ) )
	{
		if ( (ifi_ptr =
				findIfByIfname( ifihead, values[0] )) == (struct ifi_info *) NULL )
		{
			NETERROR(	MFCE,
						("Error - Unknown private interface, %s, specified in config file - ignoring\n",
							values[0] ));
		}
		else
		{
			strcpy( config_data->PrivateInterface, values[0] );
			memcpy( &private_ip_address,
					ifi_ptr->ifi_addr,
					sizeof( struct sockaddr_in ) );
			memcpy( &private_ip_netmask,
					ifi_ptr->ifi_netmask,
					sizeof( struct sockaddr_in ) );
		}
	}

	if ( strlen( values[1] ) )
	{
		if ( ( ifi_ptr =
				findIfByIfname( ifihead, values[1] )) == (struct ifi_info *) NULL )
		{
			NETERROR(	MFCE,
						("Error - Unknown public interface, %s, specified in config file - ignoring\n",
						values[1] ));
		}
		else
		{
			strcpy( config_data->PublicInterface, values[1] );
			memcpy( &public_ip_address,
					ifi_ptr->ifi_addr,
					sizeof( struct sockaddr_in ) );
			memcpy( &public_ip_netmask,
					ifi_ptr->ifi_netmask,
					sizeof( struct sockaddr_in ) );
		}
	}

	config_data->LocalNatEnabled = ( strcmp( values[2], "yes") == 0 ) ? TRUE : FALSE;

	config_data->RemoteNatEnabled = ( strcmp( values[3], "yes") == 0 ) ? TRUE : FALSE;

	strcpy( media_interface, values[4] );

	placeholder = media_interface;

	token = strtok_r( media_interface, " ,", &placeholder );

	while ( token != NULL  )
	{
		if ( (ifi_ptr =
			findIfByIfname( ifihead, token )) == (struct ifi_info *) NULL )
		{
			NETERROR(	MFCE,
						("Error - Unknown media interface, %s, "
						 "specified in config file - ignoring\n",
						token ));
			token = strtok_r( (char*) NULL, " ,", &placeholder );
			continue;
		}

		// Is this interface a public media routing interface ?

		if (	( ifi_ptr->ifi_addr->sin_addr.s_addr &
				  ifi_ptr->ifi_netmask->sin_addr.s_addr ) ==
				( public_ip_address.sin_addr.s_addr &
				  public_ip_netmask.sin_addr.s_addr ) )
		{
			// Yes,	Add it to the list of public MR interfaces

			strcpy( &config_data->PublicMR_interface[
						config_data->PublicMR_count++][0], token );

		}	// Is it a private media routing interface ?
		else if (	( ifi_ptr->ifi_addr->sin_addr.s_addr &
				  ifi_ptr->ifi_netmask->sin_addr.s_addr ) ==
				( private_ip_address.sin_addr.s_addr &
				  private_ip_netmask.sin_addr.s_addr ) )
		{
			// Yes,	Add it to the list of private MR interfaces

			strcpy( &config_data->PrivateMR_interface[
						config_data->PrivateMR_count++][0], token );
		}
		else
		{
			NETERROR(	MFCE,
						("Error - unable location of media interface, %s, "
						 "specified in config file - ignoring\n",
						token ));
		}

		token = strtok_r( (char*) NULL, " ,", &placeholder );
	}

	return(0);

}

static int
opendevice(		char * devname,
				int *  fdp )
{
	if ( ( *fdp = open( devname, O_RDWR ) ) < 0 ) 
		return( -1 );

	return( 0 );
}

static void
hexdump(	uchar_t *	input,
			int32_t		nBytes,
			uint32_t	firstByte )
{
	char	output[80];		// Accumulates the formated output
	int32_t	nHex;			// Indexes the hex digits in "output"
	int32_t	nAlpha;			// Indexes the alpha characters in "output"
	int32_t	i;				// Temporary storage
	char	outbuf[1024];

	memset(	outbuf, (int32_t) 0, 1024) ;

	if (!input)
		return;

	while (nBytes >  0) 
	{
		if (firstByte < 0)
			sprintf( output, "%08x:", (uint32_t) input );
		else
			sprintf( output, "%08x:", firstByte );

		for (i=9; i < 80; ++i) output[i] = ' ';
		nHex   = 11;
		nAlpha = 48;
		for (i=1; (i <= 16) && (nBytes > 0); ++i) 
		{
			// Convert a byte to hex...

			output[nHex]   = "0123456789ABCDEF"[*input/16];
			output[nHex+1] = "0123456789ABCDEF"[*input%16];
						/* and translate it to alpha */
			output[nAlpha] = ((*input >= 0x20) && 
					(*input < 0x7f)) ? *input : '.';
			++input;
			--nBytes;
			++nAlpha;
			nHex += 2;

			// Insert a blank every 5th byte

			if ((i % 4) == 0)	
				++nHex;
		}

		output[nAlpha] = '\0';		/* End of line (16 bytes) */

		printf( "%s\n", output );

		if (firstByte >= 0)
			firstByte += 16;
	}
}

//
//	Function 	:
//		_AddRedirect()
//
//	Arguments	:
//
//		iface_in		-	name of inbound interface
//
//		iface_out		-	name of outbound interface
//
//		holeId			-	holeId for which rule is associated
//
//		orig_dst_ip		-	destination ip address sought in incoming packet to
//							which this redirect will be applied
//							(orig_ip/orig_port pairs will be redirected to
//							new_ip/new_port pairs for the specified
//							protocol and interface.)
//
//		orig_dst_port	-	destination port number sought in incoming packet to
//							which this redirect will be applied
//
//		new_dst_ip		-	new destination ip address of packet
//							(orig_ip/orig_port pairs will be redirected to
//							new_ip/new_port pairs for the specified
//							protocol and interface.)
//
//		new_dst_port	-	new destination port number of incoming packet
//							on specified interface
//
//		rdr_type		-	character string describing use of redirect.
//							Redirection of "media" or "rtcp" data.
//
//		rdr_ptr			-	pointer to structure to be filled in as redirect is
//							constructed. When redirect is removed structure is
//							used to guide removal.
//
//		map_src_ip		-	special map ip address specified for redirects
//							which will redirect to other network interfaces
//
//		map_src_port	-	special map port specified for redirects
//							which will redirect to other network interfaces
//
//		mapflag			-	flag indicating map_src_ip and map_src_port
//							are in use for redirect.
//
//	Purpose		:
//
//		This function is called to add a NAT redirect (rdr)
//		via ipfilter ioctl calls and fill in the passed structure.
//
//	Return Value:
//		 0		on success
//		-1		on failure
//
static int
_AddRedirect(	char *		iface_in,
				char *		iface_out,
				int32_t		holeId,
				int32_t		orig_dst_ip,
				uint16_t	orig_dst_port,
				int32_t		new_dst_ip,
				uint16_t	new_dst_port,
				char *		rdr_type,
				ipf_nat_t *	rdr_ptr,
				int32_t		map_src_ip,
				uint16_t	map_src_port,
				int32_t		mapflag )
{
	char*		c_protocol = "UDP";
    char        c_localaddr[32], c_localaddr2[32], c_localaddr3[32];
	uint32_t	lerrno;
	IPNAT_T *	nat_rdr_ptr = (IPNAT_T*) &rdr_ptr->data;

	// Specify its nat type as NAT_REDIRECT

	nat_rdr_ptr->in_redir = NAT_REDIRECT;

	// Specify inbound interface

	strcpy(	nat_rdr_ptr->in_ifname, iface_in );

	// Specify outbound interface

	strcpy(	nat_rdr_ptr->in_ifname_out, iface_out );

	// Specify what destination ip should look like when coming into
	// the specified interface from the outside world inorder to
	// be redirected

	nat_rdr_ptr->in_outip = htonl( orig_dst_ip );
	nat_rdr_ptr->in_outmsk = -1;

	// Specify the port from which the packet should come

	nat_rdr_ptr->in_pmin = nat_rdr_ptr->in_pmax = htons( orig_dst_port );

	// Specify the ip to which the packet will be
	// redirected on the internal network

	nat_rdr_ptr->in_inip = htonl( new_dst_ip );
	nat_rdr_ptr->in_inmsk = -1;

	if ( mapflag )
	{
		nat_rdr_ptr->in_map_srcip.s_addr = htonl( map_src_ip );
		nat_rdr_ptr->in_map_port = htons( map_src_port );
		nat_rdr_ptr->in_flags = IPN_XMAP;
	}

	// Specify the port to which the packet will be
	// redirected

	nat_rdr_ptr->in_pnext = htons( new_dst_port );

	// Specify the protocol

	nat_rdr_ptr->in_flags |= IPN_UDP;

	memset( rdr_ptr->rdr_string_1, (int32_t) 0, 128 );
	memset( rdr_ptr->rdr_string_2, (int32_t) 0, 128 );
	memset( rdr_ptr->rdr_string_3, (int32_t) 0, 128 );

	if ( mapflag )
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress( orig_dst_ip, c_localaddr ),
				orig_dst_port );

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d",
				iface_out,
				FormatIpAddress( new_dst_ip, c_localaddr2 ),
				new_dst_port );

		sprintf(rdr_ptr->rdr_string_3,
				"    proto %s - xmap src %s, port %d", 
				c_protocol,
				FormatIpAddress( map_src_ip, c_localaddr3 ),
				map_src_port );
	}
	else
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress( orig_dst_ip, c_localaddr ),
				orig_dst_port );

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d", 
				iface_out,
				FormatIpAddress( new_dst_ip, c_localaddr2 ),
				new_dst_port );

		sprintf(rdr_ptr->rdr_string_3,
				"    proto %s", 
				c_protocol );
	}

	if ( ioctl( ipnat_fd, SIOCADNAT, &nat_rdr_ptr ) < 0 )
	{
		lerrno = errno;

		trc_error(	MFCE,
			"       : ERROR ioctl ADNAT - (%s) hole %d, err : %s\n",
			rdr_type,
			holeId,
			strerror( lerrno ) );

		trc_error(	MFCE,
					"       : failed - NAT redirect translation (%s) \n",
					rdr_type );

		trc_error(	MFCE,
					"       :   %s\n", 
					rdr_ptr->rdr_string_1 );

		trc_error(	MFCE,
					"       :   %s\n", 
					rdr_ptr->rdr_string_2 );

		trc_error(	MFCE,
					"       :   %s\n", 
					rdr_ptr->rdr_string_3 );

		NETERROR(	MFCE,
					( "Error creating local translation (%s) - err : %s\n",
					rdr_type,
					strerror( lerrno ) ));

		NETERROR(	MFCE,
					("       : failed - NAT redirect translation (%s) \n",
					rdr_type ));

		NETERROR(	MFCE,
					("       :   %s\n", 
					rdr_ptr->rdr_string_1 ));

		NETERROR(	MFCE,
					("       :   %s\n", 
					rdr_ptr->rdr_string_2 ));

		NETERROR(	MFCE,
					("       :   %s\n", 
					rdr_ptr->rdr_string_3 ));

		return(-1);
	}

	rdr_ptr->rdr_defined = TRUE;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       : added - NAT redirect translation (%s) \n",
				rdr_type );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n", 
				rdr_ptr->rdr_string_1 );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n", 
				rdr_ptr->rdr_string_2 );

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n", 
				rdr_ptr->rdr_string_3 );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				( "Successfully created local translation: ANY -> "
					"%s:%d (%s:%d) - proto %s - %s\n",
				FormatIpAddress(	orig_dst_ip,
									c_localaddr ),
				orig_dst_port,
				FormatIpAddress( 	new_dst_ip,
									c_localaddr2 ),
				new_dst_port,
				c_protocol,
				rdr_type ));

	return(0);
}

//
//	Function 	:
//		_OpenPinHole()
//
//	Arguments	:
//
//		interface		-	physical interface name associated wiht hole
//
//		ip_addr			-	ipv4 destination ip address for incoming
//							signaling packets inbound to gis
//							For DMR this will be the ip address of a
//							logical interface on the physical interface
//							( host order )
//
//		holeId			-	assigned holeId associated with hole
//
//		sessionId		-	assigned sessionId associated with hole
//
//		port			-	local port number being opened on lif
//							(host order)
//
//		protocol		-	protocol type used for signaling traffic
//							through hole
//
//		pmh_ptr	    	-	pointer to structure to be filled in as portmap
//							hole is constructed. When pmhole is removed,
//							structure is used to guide removal.
//
//	Purpose		:
//
//		This function is called to poke pinholes in the specified
//		network interface with the specified criterion. It puts out
//		logging messages and fills in a data structure for keeping
//		track of the hole so it can be removed.
//
//	Return Value:
//		 0		on success
//		-1		on failure
//
static int 		
_OpenPinHole(	char *			interface,
				int32_t			ip_addr,
				int32_t			holeId,
				uint32_t		sessionId,
				uint16_t		port,
				int32_t			protocol,
				ipf_pmhole_t *	pmh_ptr )
{
	char *		c_proto;
	uint32_t	lerrno;
	char		c_addr[32];

	if ( protocol == IPPROTO_TCP )
		c_proto = "TCP";
	else if ( protocol == IPPROTO_UDP )
		c_proto = "UDP";
	else
		c_proto = "Unknown";

	if ( protocol == IPPROTO_UDP || protocol == IPPROTO_TCP )
	{
		strcpy( pmh_ptr->data.ifname, interface );

		if ( protocol == IPPROTO_TCP )
			pmh_ptr->data.proto = IPPROTO_TCP;
		else if ( protocol == IPPROTO_UDP )
			pmh_ptr->data.proto = IPPROTO_UDP;

		pmh_ptr->data.lif_ip = htonl( ip_addr );
		pmh_ptr->data.port = ntohs( port );

		if ( ioctl( ipf_fd, SIOCADDPME, &pmh_ptr->data ) < 0 )
		{
			lerrno = errno;

			trc_error(	MFCE,
			"       : ERROR ioctl ADDPME - hole %d, err : %s - SIGNALING\n",
			holeId,
			strerror( lerrno ) );
			return( -1 );
		}
		else
		{
			trc_debug(  MFCE,
						NETLOG_DEBUG1,
						"       : adding local signaling hole\n" );

			trc_debug(  MFCE,
						NETLOG_DEBUG1,
						"       :   %s:%d %s on %s\n",
						FormatIpAddress(ip_addr,
										c_addr ),
						port,
						(( protocol == IPPROTO_UDP )? "UDP" : "TCP"),
						PublicInterface );

			trc_debug(  MFCE,
						NETLOG_DEBUG1,
						"       :       holeid %d, sessionId %d\n",
						holeId,
						sessionId );
		}
	}

	return(0);
}

//
//	Function 	:
//		_RemovePMHole()
//
//	Arguments	:
//
//		holeId			-	holeId for which rule is associated.
//
//		sessionId		-	sessionId associated with hole
//
//		pmh_ptr		    -	pointer to structure to be filled in as portmap
//							hole is constructed. When pmhole is removed,
//							structure is used to guide removal.
//
//	Purpose		:
//
//		This function is called to remove a pinhole in the specified
//		network interface with the specified criterion.
//
//	Return Value:
//		 0		on success
//		-1		on failure
//
static int 		
_RemovePMHole(	int32_t			holeId,
				uint32_t		sessionId,
				ipf_pmhole_t *	pmh_ptr )
{
	char *		c_proto;
	uint32_t	lerrno;
	char		c_addr[32];

	if ( pmh_ptr->data.proto == (uint8_t) IPPROTO_TCP )
		c_proto = "TCP";
	else if ( pmh_ptr->data.proto == (uint8_t) IPPROTO_UDP )
		c_proto = "UDP";

	if ( ioctl( ipf_fd, SIOCRMPME, &pmh_ptr->data ) < 0 )
	{
		lerrno = errno;

		trc_error(	MFCE,
			"       : ERROR ioctl RMPME - hole %d, err : %s\n",
			holeId,
			strerror( lerrno ) );
		return( -1 );
	}
	else
	{
		trc_debug(  MFCE,
					NETLOG_DEBUG2,
					"       : removing local signaling hole\n" );

		trc_debug(  MFCE,
					NETLOG_DEBUG2,
					"       :   %s:%d %s on %s\n",
					FormatIpAddress(	ntohl(pmh_ptr->data.lif_ip),
										c_addr ),
					ntohs(pmh_ptr->data.port),
					c_proto,
					PublicInterface );

		trc_debug(  MFCE,
					NETLOG_DEBUG2,
					"       :                 holeid %d, sessionId %d\n",
					holeId,
					sessionId );
	}

	return(0);
}

//*
//*	Hash table functions for lookup of IpHoleBlock_t's by
//* either sessionId or holeId
//*

//
//	Function    :
//		getHashtableBucketCount()
//	
//	Arguments:
//		None
//
//	Purpose		:
//		This function calculates the number of hash buckets that will be
//		needed for optimal sizing of the caches used for sessionId and holeId
//		lookup. The calculation is based on the maxCalls value specified for
//		the iServer configuration. Value has a maximum of 16384 or tracks the
//		max licenses in powers of 2 for the range lower than that
//
//	Return values:
//		 BucketCount 	value
//
static int32_t
getHashtableBucketCount(void)
{
	int32_t	buckets = 1;

	if (lsMem->maxCalls <= 0)
	{
		buckets = 1024;
	}
	else
	{
    	while ( (buckets < 16384) && (buckets < lsMem->maxCalls) )
    	{
      		buckets <<= 1;
    	}
	}

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("getHashtableBucketCount: using bucket count %d\n",
				buckets));

	return( buckets );
}

static int
commonHash( void *key )
{
	int	*val = (int *) key;

	if ( HoleIdCache->_numBuckets == 1 )
		return(0);
	else
		return( *val % HoleIdCache->_numBuckets );	
			// Note :	HoleIdCache->_numBuckets == sessionIdCache->_numBuckets

}

static int
commonKeyCmp( void*	key1, void *key2 )
{
	int *val1 = (int*) key1;
	int *val2 = (int*) key2;

	return( *val1 - *val2 );
}

static void*
holeIdData2Key( void * entry )
{
	return( & ((IpHoleBlock_t *) entry)->assignedId ); 
}

static void*
sessionIdData2Key( void * entry )
{
	return( & ((IpHoleBlock_t *) entry)->sessionId ); 
}

//
//	Function    :
//		_IpFilter_initcaches()
//	
//	Arguments:
//		None
//
//	Purpose		:
//		This function sets up caches for keeping track of
//		
//
//	Return values:
//		 0		on success
//		-1		on failure
//
static int32_t
_IpFilter_initcaches(void)
{
	extern int CacheSetName( cache_t cache, char *name );
	int32_t	BucketCount = 2 * getHashtableBucketCount();
	char *	fn = "_IpFilter_initcaches(): ";

	// initialize holeId cache

	HoleIdCache = CacheCreate( CACHE_MALLOC_LOCAL );
	HoleIdCache->dt = CACHE_DT_HASH;
	HoleIdCache->_numBuckets = BucketCount;
	HoleIdCache->_hashlistOffset = HOLEID_LIST_OFFSET;
	HoleIdCache->_hash = (int) commonHash;
	HoleIdCache->_entry2key = (int) holeIdData2Key;
	HoleIdCache->_keycmp = (int) commonKeyCmp;
	HoleIdCache->lock = NULL;

	CacheSetName( HoleIdCache, "IPF holeId" );

	if ( !CacheInstantiate( HoleIdCache ) )
	{
		NETERROR(	MFCE,
					( "%s Cannot initialize HoleId cache\n",
					fn ));
		return(-1);
	}

	// initialize holeId cache

	SessionIdCache = CacheCreate( CACHE_MALLOC_LOCAL );
	SessionIdCache->dt = CACHE_DT_HASH;
	SessionIdCache->_numBuckets = BucketCount;
	SessionIdCache->_hashlistOffset = SESSIONID_LIST_OFFSET;
	SessionIdCache->_hash = (int) commonHash;
	SessionIdCache->_entry2key = (int) sessionIdData2Key;
	SessionIdCache->_keycmp = (int) commonKeyCmp;
	SessionIdCache->lock = NULL;

	CacheSetName( SessionIdCache, "IPF sessionId" );

	if ( !CacheInstantiate( SessionIdCache ) )
	{
		NETERROR(	MFCE,
					( "%s Cannot initialize SessionId cache\n",
					fn ));
		return(-1);
	}

	return( 0 );
}
