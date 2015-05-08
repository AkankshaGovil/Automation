//
// This file contains NSF firewall specific api. These APIs act as a glue
// between FC layer and the NSF drivers.
//
// Note :
//    vi settings!!
//    	:set ts=4 sw=4
//

#define _NSFGLUE_SYS_INCLUDE_

#include "nsfglue.h"
#include "poolalloc.h"
#include "execd.h"
#include "nxosd.h"
#include "fcemacro.h"  /* For IS_HKNIFE_FW */
#include "hknife.h" /* For hknife_flush() */

int  poolsXmlCkSum ;
static int msgQueueId;			// the message queue to execd
static pid_t mypid;			


extern int getSignaledInterfaces(	int		numEnt,
									int *	returned_entries,
									char	(*ifs)[IFI_NAME] );

extern unsigned int ckSumPoolFile(unsigned long *cksum);

// mutex to lock on, while accessing shared resources 

static pthread_mutex_t mutex;

typedef struct _ipf_nat
{
	IPNAT_T data;
	char rdr_string_1[128];
	char rdr_string_2[128];
	char rdr_string_3[128];
	int32_t rdr_defined;
    TejaCPDP_legkey_t legkey;

} ipf_nat_t;

typedef struct _ipf_pmhole
{
	fr_pm_entry_t	data;
	uint16_t		port;
	int32_t			hole_defined;
} ipf_pmhole_t;

typedef struct _IpHole
{
	struct _IpHole *	holeIdPrev,
	               *	holeIdNext;
	struct _IpHole *	bundleIdPrev,
	               *	bundleIdNext;

	uint32_t 				assignedId;
	uint32_t 				bundleId;

	uint32_t				ingressPoolId;
	uint32_t				ingressIp;
	uint16_t				ingressPort;

	uint32_t				egressPoolId;
	uint32_t				egressIp;
	uint16_t				egressPort;

	uint32_t				dstIp;
	uint16_t				dstPort;

	ipf_nat_t				rtp_nat;
	ipf_nat_t				rtcp_nat;

	ipf_pmhole_t			ipf_pmhole_signaling;

	uint32_t				flags;

  uint32_t        ingress_vlan;
  uint32_t        egress_vlan;

#define	HOLE_ALLOCATE       (0x1)
#define DNAT_RELATE_PENDING (0x2)

} IpHole_t;

static char *	fwconfig_path = "/usr/local/nextone/bin/fwconfig";
static char *	iptables_path = "/usr/sbin/iptables";
int				ActiveRedirects;

#define HOLEID_LIST_OFFSET      (0)
#define BUNDLEID_LIST_OFFSET   (2*sizeof(IpHole_t *))


// Hashtable cache for IpHole_t lookup by HoleId

static cache_t HoleIdCache;

// Hashtable cache for IpHole_t lookup by bundleId

static cache_t BundleIdCache;

static int lastAssignedId = 0;

// APIs related to local cache of holes

static int32_t
		InitCaches(		void );

static IpHole_t *
		IpHoleAlloc(	uint32_t	bundleId, uint32_t holeId );

static void
		DeleteIpHole(	IpHole_t *	ptr );

static void
	DeleteIpHoleForBundle( uint32_t	bundleId );

static IpHole_t *
	GetIpHoleBlockForBundleId(uint32_t bundleId);

static IpHole_t *
	GetIpHoleBlockForHoleId(uint32_t assignedId);

// get a new holeid
uint32_t
	newHoleId(void);

// APIs related to closing/opening holes using NSF driver

// opens a holes for rtp and rtcp for one leg of call

static int	OpenMediaHole(	uint32_t		bundleId,
						 	uint32_t		dstIpAddress,
						 	uint16_t		dstPort,
						 	uint32_t		ingressPoolId,
						 	uint32_t		egressPoolId,
						 	uint32_t		peerResourceId,
						 	uint32_t *		returnedNatSrcIpAddress,
						 	uint16_t *		returnedNatSrcPort,
						 	uint32_t *		returnedNatDestIpAddress,
						 	uint16_t *		returnedNatDestPort,
						 	uint32_t *		returnedResourceId,
							int			    dstSym,
							int				flag);

static int ModifyMediaHole(
							uint32_t		bundleId,
							uint32_t		resourceId,
						 	uint32_t		dstIpAddress,
						 	uint16_t		dstPort,
							uint32_t		ingressPoolId,
						 	uint32_t		egressPoolId,
							uint32_t		peerResourceId,
						 	uint32_t *		returnedNatSrcIpAddress,
						 	uint16_t *		returnedNatSrcPort,
						 	uint32_t *		returnedNatDestIpAddress,
						 	uint16_t *		returnedNatDestPort,
							int			    dstSym,
							int 			flag
						);

// add a nat translation

static int	AddRedirect(	char *			iface_in,
					   		char *			iface_out,
					   		int32_t			holeId,
					   		int32_t			orig_dst_ip,
					   		uint16_t		orig_dst_port,
					   		int32_t			new_dst_ip,
					   		uint16_t		new_dst_port,
					   		char *			rdr_type,
					   		ipf_nat_t * 	rdr_ptr,
					   		int32_t			map_src_ip,
					   		uint16_t		map_src_port,
					   		int32_t			mapflag,
							int				nop,
							ipf_nat_t *		other_rdr_ptr,
							IpHole_t *		other_hp,
							int				dstSym,
							uint32_t		peerResourceId,
              int ingressVlanId,
              int egressVlanId );

static int	ModifyRedirect(	char *			iface_in,
					   		char *			iface_out,
					   		int32_t			holeId,
					   		int32_t			orig_dst_ip,
					   		uint16_t		orig_dst_port,
					   		int32_t			new_dst_ip,
					   		uint16_t		new_dst_port,
					   		char *			rdr_type,
					   		ipf_nat_t * 	rdr_ptr,
					   		int32_t			map_src_ip,
					   		uint16_t		map_src_port,
					   		int32_t			mapflag,
							int				nop,
							ipf_nat_t *		other_rdr_ptr,
							IpHole_t *		other_hp,
							int				dstSym,
							uint32_t		peerResourceId );

// adds the nat translation to the local hashtable

static uint32_t
			AddMediaHole(	uint32_t		bundleId,
							uint32_t		holeId,
							ipf_nat_t *		rtp_nat_ptr,
							ipf_nat_t *		rtcp_nat_ptr,
							uint32_t		ingressPoolId,
							uint32_t		ingressIp,
							uint16_t		ingressPort,
              uint32_t    ingressVlanId,
							uint32_t		egressPoolId,
							uint32_t		egressIp,
							uint16_t		egressPort,
              uint32_t    egressVlanId,
							uint32_t		dstIp,
							uint16_t		dstPort,
							int				allocateOnly,
							IpHole_t **		returned_ptr );

// removes the nat translation

static void	RemoveRedirect(	uint32_t		bundleId,
							uint32_t		holeId,
							void *			ipnat_ptr,
						 	char *			rule_string_1,
							char *			rule_string_2,
							char *			rule_string_3,
							char *			ctype,
              uint32_t    ingressVlan,
							int				nop);

// opens a signaling hole

static int 
		OpenSignalingHole(	uint32_t		bundleId,
							char *			signalingInterface,
							uint32_t		signalingIpAddress,
							uint16_t		signalingPort,
							char *			protocol,
							uint32_t *		returnedResourceId );

// adds a signaling hole

static int AddPMHole(		char *			interface,
							int32_t			ip_addr,
							int32_t			holeId,
							uint32_t		bundleId,
							uint16_t		port,
							int32_t			protocol,
							ipf_pmhole_t *	pmh_ptr);

// saves signaling hole into local hashtable
// for later removal

static uint32_t
		AddSignalingHole(	uint32_t		bundleId,
							uint32_t		holeId,
							ipf_pmhole_t *	ipf_pmhole_signaling_ptr );

// removes the signaling hole

static int RemovePMHole(	int32_t			holeId,
							uint32_t		bundleId,
							ipf_pmhole_t *	pmh_ptr );

// primitive api
static void 
UnplumbMediaHole(
				uint32_t	bundleId,
				uint32_t	holeId,
				ipf_nat_t	*rtp_nat_p,
				ipf_nat_t	*rtcp_nat_p,
        uint32_t  ingress_vlan,
				int			nop
				);


// removes the hole from the local hashtable

static void CloseHole(			IpHole_t *	idp );

static void RebuildRules( char * args );

static void Hexdump(uchar_t * input, int32_t nBytes, uint32_t firstByte);


// Ward off warnings from the compiler about
// unsued routines

void *dummy_hAndle1 = (void *) Hexdump;

// >*****************************************
// >* Locally used Trace facility globals
// >* definitions. static inline trace 
// >* functions defined in trace.h file
// >*****************************************

int32_t trace_disk = 1;
int32_t trace_initialized;
FILE *trace_fdesc;
int32_t trace_fd;
int32_t nsf_reconfig;

trace_table_t iserv_trace_tbl;

char trace_log[256] = TRACE_NSF_TRACE_FILENAME;
char pools_cfg_file[256] = POOLS_CFG_FILE;

//
//	Function 	:
//		nsfGlueInit()
//
//	Purpose		:
//
//		This function should be called once at iserver startup.
//		It gets configuration information from the pools XML file
//		and opens the local devices needed to setup and destroy
//		pinholes for firewalling using nsf. It also sets up the
//		media interfaces using either the Teja api if hotknife is
//		used or the nsf ioctl interface if the local box is used 
//		for media.
//
//	Return		:
//		None
//
void
nsfGlueInit(void)
{
	fr_clearbm_entry_t	fr_cb;
	fr_interface_type_t	fr_it;
	int					i, num;
	char 				signaling_ifs[MAX_POOL_IDS][INTF_NAME_LEN] = { {0} };
	char 				media_ifs[MAX_POOL_IDS][INTF_NAME_LEN] = { {0} };
	char				argsbuf[512];
	int					args_inited = 0;
	unsigned long		csum=0;
	uint32_t			hk_addr;

	// use the SERPLEXPATH env var for the location of the pools.xml file
	
	char *path = getenv("SERPLEXPATH");

	trc_init();

	memset( signaling_ifs, (int) 0, (MAX_POOL_IDS * INTF_NAME_LEN) );

	mypid = getpid();

	// Perform hotknife initialization
	if( fceFirewallAddresses[0] != 0 )
	{
		hk_addr = ntohl( *(uint32_t *)&fceFirewallAddresses[0] ); 
	}
	else
	{
		inet_pton( AF_INET, HKNIFE_DEF_MGMT_IP, &hk_addr );
	}
	if ( hknife_connect( inet_ntoa( *(struct in_addr *)&hk_addr ) ) != 0 )
	{	
		NETERROR(	MFCE,
					( "MFCE : Unable to connect to radisys device\n" ));
 		return;
	}
	hknife_flush();
	ActiveRedirects = 0;

	if (path) {
	  strcpy(pools_cfg_file,path);
	  strcat(pools_cfg_file,POOLS_CFG_FILE);
	} else {
	  strcpy(pools_cfg_file,"/usr/local/nextone/bin/");
	  strcat(pools_cfg_file,POOLS_CFG_FILE);
	}
	
	// first thing to do is to intialize the interfaces
	// from pool XML file. Local interfaces or hotknife
	// media interfaces are plumbed in the following call.

	initPoolAllocation(pools_cfg_file);

	if(ckSumPoolFile(&csum))
	{
		NETERROR( MFCE, ("MFCE : Unable to compute initial checksum on pools.xml\n" ));
		return;
	}
	poolsXmlCkSum = csum;

	// open a message queue to execd

	if (( msgQueueId = open_execd() ) < 0 ) {
		NETERROR( MFCE, ("MFCE : Unable to open_execd\n" ));
		return;
	}


	// Instantiate caches used for bundleId and holeId lookup

	if (InitCaches() < 0) {
		NETERROR(	MFCE,
					("MFCE : Unable to allocate cache(s) for "
					 "ipfilter lookup\n"));
		return;
	}

	// initialize the mutex 

	if (initMutex(&mutex, NULL)) {
		return;
	}

	// Process signaling interfaces

	sprintf( argsbuf, "hknife ");

	if ( (num = getPooledInterfaces( POOL_INTF_SIG, signaling_ifs) ) ) {
		sprintf( argsbuf, "-s " );

		// Clear call bitmaps and set type for each
		// signaling interface

		for (i = 0; i < num; i++) {
			memset( &fr_it, 0, sizeof( fr_interface_type_t ) );
			strcpy( fr_it.ifname, signaling_ifs[i] );

			fr_it.itype = IPF_ITYPE_PUBLIC;

			memset(&fr_cb, 0, sizeof(fr_clearbm_entry_t));
			strcpy(fr_cb.ifname, signaling_ifs[i]);

			// Builds the nsfconfig command for a firewalled
			// configuration
			//     Format example :
			//        nsfconfig -s e1000g0,e1000g1

			if ( !args_inited ) {
				sprintf( argsbuf, "%s %s", argsbuf, signaling_ifs[i] );
				args_inited = 1;
			}
			else
				sprintf( argsbuf, "%s,%s", argsbuf, signaling_ifs[i] );

			NETINFOMSG(	MFCE,
						("     physical signaling interface ,\"%s\", initialized\n",
						signaling_ifs[i] ));
		}
	} else {
		NETERROR(	MFCE,
					("No physical signaling interfaces defined for this server" ));

		// Builds the nsfconfig command for an open
		// configuration
		//     Format example :
		//        nsfconfig -d

		sprintf( argsbuf, "-d" );
		args_inited = 1;
	}

	// Issue nsfconfig command with built arguments

	if ( args_inited )
		RebuildRules( argsbuf );
	
	// Process media interfaces

		hknife_purge_redirects();

	num = getPooledInterfaces(POOL_INTF_MEDIA,  media_ifs );


	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("Successfully initialized with ipfilter firewall\n"));
}

//
//	Function 	:
//		nsfGlueShutdown()
//
//	Purpose		:
//
//		This function closes all BM holes on signaling interfaces,
//		deletes all redirects, clears and deletes the local cache
//		used to keep track of kernel resources used and destroys
//		the mutex used to protect the cache. 
//
//	Return		:
//		1 - if no problem with shutting down
//
int
nsfGlueShutdown(void)
{
	IpHole_t *			ptr;
	fr_clearbm_entry_t	fr_cb;
	int					i, num;
	char 				signaling_ifs[MAX_POOL_IDS][INTF_NAME_LEN] = { {0} };

	trc_error(	MFCE,
				"SHUTD  : called\n" );

	lockMutex(&mutex);

	memset( signaling_ifs, (int) 0, (MAX_POOL_IDS * INTF_NAME_LEN) );

	if (HoleIdCache != NULL)
	{
		// Close all holes that have been opened.

		while ((ptr = CacheGetFirst(HoleIdCache)) != NULL)
		{
			CloseHole(ptr);

			// Remove holeId from BundleIdCache and HoleIdCache

			DeleteIpHole(ptr);
		}

		CacheDestroy(HoleIdCache);
		HoleIdCache = NULL;
	}

	if (BundleIdCache != NULL)
	{
		CacheDestroy(BundleIdCache);
		BundleIdCache = NULL;
	}

	// Process signaling interfaces

	if ( getSignaledInterfaces( MAX_POOL_IDS, &num, signaling_ifs ) == 0 )
	{
		// Clear call bitmaps and set type for each
		// signaling interface

		for (i = 0; i < num; i++)
		{
			memset(&fr_cb, 0, sizeof(fr_clearbm_entry_t));
			strcpy(fr_cb.ifname, signaling_ifs[i]);

		}
	}

	// Process media interfaces
	//    Clear any nat entries on media interfaces


	hknife_flush();
	ActiveRedirects = 0;

	// Close hotknife connection
	hknife_close();


	// unlock the mutex

	unlockMutex(&mutex);

	// destroy the mutex 

	destroyMutex(&mutex, NULL);

	trc_error(MFCE, "SHUTD  : exiting\n");

	trc_close();

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("Successfully shutdown connection with ipfilter firewall\n"));

	return(1);
}

//
//	Function 	:
//		nsfGlueReconfig()
//
//	Arguments	:
//		None
//
//	Purpose		:
//
//		This function is called when configuration changes.
//
//	Note:
//		for now any changes to nsf.cfg or ipfilter.conf files, require cycling
//		of the gis (start/stop) to take effect.
//
// return		:
//
//		 0 - if changes in the file does not warrant a MSW restart
//		-1 - if the changes in the file requires a MSW restart
//
int
nsfGlueReconfig(void)
{
	int		rc = 0;
	int		config_changed, nsf_disabled;
	char 	signaling_ifs[MAX_POOL_IDS][INTF_NAME_LEN] = { {0} };
	char	argsbuf[512];
	int		args_inited = 0;
	int		num = 0, i;

	trc_error(MFCE, "RECFG  : called\n");

	config_changed = poolConfigChanged(pools_cfg_file);

	// Has nsf been disabled ?

//	nsf_disabled = !(IS_NSF_FW);
	nsf_disabled = IS_FW_DISABLED; //Always true in case of hotknife

	if ( config_changed || nsf_disabled )
	{
		nsf_reconfig = 1;			// indicate we are in reconfig

    	// Yes, reinit nsf and rebuild 

		memset( signaling_ifs, (int) 0, (MAX_POOL_IDS * INTF_NAME_LEN) );

		if ( getSignaledInterfaces( MAX_POOL_IDS, &num, signaling_ifs ) == 0 )
		{
			if ( nsf_disabled || num == 0 )
			{
				// Builds the nsfconfig command for an open
				// configuration
				//     Format example :
				//        nsfconfig -d

				sprintf( argsbuf, "-d" );
				args_inited = 1;
			}
			else
			{

				sprintf( argsbuf, "-s " );

				// Builds the nsfconfig command for a firewalled
				// configuration
				//     Format example :
				//        nsfconfig -s e1000g0,e1000g1

				for (i = 0; i < num; i++)
				{
					if ( !args_inited )
					{
						sprintf( argsbuf, "%s %s", argsbuf, signaling_ifs[i] );
						args_inited = 1;
					}
					else
						sprintf( argsbuf, "%s,%s", argsbuf, signaling_ifs[i] );
				}
			}
		}

		// Shutdown nsf 
		//   flushes loaded BM holes and redirects

		nsfGlueShutdown();

		// Rebuild static rules based on signaling interface changes

		if ( args_inited )
			RebuildRules( argsbuf );

		// Re-read the pool configuration

		rc = reconfigPoolAllocation(pools_cfg_file);

		// Restart nsf and reload new filter rules

		if (!nsf_disabled)
			nsfGlueInit();
		nsf_reconfig = 0;			// indicate reconfig is done
	}

	trc_error(MFCE, "RECFG  : exiting\n");

	return( rc );
}

//
//	Function 	:
//		nsfGlueOpenResource()
//
//	Arguments	:
//		None
//
//	Purpose		:
//
//		This function is called to open a hole/resource. It is
//		called to setup both media legs between two endpoints or
//		signaling holes for communication between a remote endpoint
//		and the local MSW.
//
//		For media holes, the routine creates a NAT translation to
//		allow traffic to flow through the MSW as follows :
//
//		   ANY--->returnedIpAddress/returnedPort from ingressPoolId
//		   Ip/Port from egressPoolId--->dstIpAddress/dstPort
//
//	Note:
//		for now any changes to nsf.cfg or ipfilter.conf files, require cycling
//		of the gis (start/stop) to take effect.
//
// return		:
//
//		 0 - if changes in the file does not warrant a MSW restart
//		-1 - if the changes in the file requires a MSW restart
//
//		bundleId		-	the bundle this hole will belong to
//
//		dstIpAddress	-	the IP address where the traffic will terminate
//
//								For media allocations this will be the ultimate
//								destination ip address of the media
//
//								For signaling allocations, this will be the
//								local ip address of an interface on the
//								MSW the the signaling traffic will be sent
//								to by the remote endpoint
//
//		dstPort			-	the port where the traffic will terminate
//
//								For media allocations, this will be the ultimate
//								destination port of the media
//
//								For signaling allocations, this will be a
//								local port on the local interface allocated
//								by the caller.
//
//		ingressPoolId	-	the pool from which the ingress side of the hole
//							will be allocated
//
//								Only used for media allocations
//
//		egressPoolId	-	the pool from which the egress side of the hole
//							will be allocated
//
//								Only used for media allocations
//
//		protocol		-	null terminated character string representing the
//							protocol of the traffic, "TCP", "UDP" or "RTP"
//							(for RTP, both port and port+1 will be used)
//
//		returnedIpAddress -
//							For media allocations, the IP address that was
//							allocated from the ingressPoolId
//
//							For signaling allocations, the input dstIpAddress
//
//		returnedPort	-	
//							For media allocations, the port that was allocated
//							from the ingressPoolId
//
//		returnedResourceId -
//							the id associated with this resource/hole
//
//		dstSym          - flag indicating that the leg of the call being
//		                  created will use data nat traversal. If value
//		                  is non-zero, data nat traversal will be used 
//		                  for the leg being created.
//
int
nsfGlueOpenResource(	uint32_t	bundleId,
						uint32_t	dstIpAddress,
						uint16_t	dstPort,
						uint32_t	ingressPoolId,
						uint32_t	egressPoolId,
						char *		protocol,
						uint32_t	peerResourceId,
						uint32_t *	returnedNatSrcIpAddress,
						uint16_t *	returnedNatSrcPort,
						uint32_t *	returnedNatDestIpAddress,
						uint16_t *	returnedNatDestPort,
						uint32_t *	returnedResourceId,
						int		    dstSym,
						int		    allocateCreate)

{
	int 		rc = -1;
	char 		signalingInterface[INTF_NAME_LEN];

	if ( !strncasecmp( protocol, "rtp", 3) )
	{
		rc = OpenMediaHole(	bundleId,
				   	dstIpAddress,
				   	dstPort,
				   	ingressPoolId,
				   	egressPoolId,
					peerResourceId,
				   	returnedNatSrcIpAddress,
					returnedNatSrcPort,
				   	returnedNatDestIpAddress,
					returnedNatDestPort,
					returnedResourceId, 
					dstSym,
					allocateCreate);
	}
	else
	{
		if (getInterfaceForPool(egressPoolId, signalingInterface) == NULL)
		{
			trc_error(MFCE,
					  "ERROR  : nsfGlueOpenResource() - allocating "
					  "signaling interface on pool %d\n",
					  egressPoolId );
		}
		else
		{
			rc = OpenSignalingHole(	bundleId,
									signalingInterface,
									dstIpAddress,
									dstPort,
									protocol,
									returnedResourceId );

			*returnedNatDestIpAddress 	= dstIpAddress;
			*returnedNatDestPort 		= dstPort;
		}
	}

	return( rc );
}

static void 
UnplumbMediaHole(
				uint32_t	bundleId,
				uint32_t	holeId,
				ipf_nat_t	*rtp_nat_p,
				ipf_nat_t	*rtcp_nat_p,
        uint32_t  ingress_vlan,
				int			nop
				)
{
	// Does a nat redirect exist for the hole ?
	if (rtp_nat_p->rdr_defined == TRUE)
	{
		// Yes, remove associated nat translations

		// Remove rtp nat translation
		RemoveRedirect(	bundleId,
				   	holeId,
				   	(void *) &rtp_nat_p->data,
				   	rtp_nat_p->rdr_string_1,
				   	rtp_nat_p->rdr_string_2,
				   	rtp_nat_p->rdr_string_3,
				   	"RTP",
					ingress_vlan,
					 nop);

		ActiveRedirects--;

		rtp_nat_p->rdr_defined = FALSE;
	}

	
}

static int
PlumbMediaHole(			
				uint32_t	holeId,
				char 		*ingressInterface,
				char 		*egressInterface,
			 	uint32_t	ingressIp,
			 	uint16_t	ingressPort,
			 	uint32_t	egressIp,
			 	uint16_t	egressPort,
			 	uint32_t	dstIpAddress,
			 	uint16_t	dstPort,
				int		ingressVlanId,
				int		egressVlanId,
				ipf_nat_t	*rtp_nat_p,
				ipf_nat_t	*rtcp_nat_p,
				int 		nop,
				IpHole_t *	other_hp,
				int         dstSym,
				uint32_t	peerResourceId )
{
	// Add new redirect for the rtp

	if (AddRedirect(	ingressInterface,
						egressInterface,
						holeId,
						ingressIp,
						ingressPort,
						dstIpAddress,
						dstPort,
						"rtp",
						rtp_nat_p,
						egressIp,
						egressPort,
						1,
						nop,
						((other_hp)?&other_hp->rtp_nat:NULL),
						other_hp,
						dstSym,
						peerResourceId,
						ingressVlanId,
						egressVlanId ) < 0 )
	{
		trc_error(	MFCE,
				  	"       : failed to add rtp  Redirect for hole %d\n",
				  	holeId);
		return -1;
	}

	ActiveRedirects++;

	

	if (other_hp && other_hp->flags & DNAT_RELATE_PENDING ) {
		// Yes, turn off flag in other leg and indicate as much
		other_hp->flags &= ~DNAT_RELATE_PENDING;
	}

	return 0;
}

static int
ModifyMediaHole(			
				uint32_t	bundleId,
				uint32_t	holeId,
			 	uint32_t	dstIpAddress,
			 	uint16_t	dstPort,
				uint32_t	ingressPoolId,
				uint32_t	egressPoolId,
				uint32_t	peerResourceId,
				uint32_t *	returnedNatSrcIpAddress,
			 	uint16_t *	returnedNatSrcPort,
				uint32_t *	returnedNatDestIpAddress,
				uint16_t *	returnedNatDestPort,
				int		    dstSym,
				int 		flag
				)
{
	uint16_t		ingressPort, inboundPort, egressPort, outboundPort;
	int			ingressVlanId, egressVlanId;

	char			ingressInterface[INTF_NAME_LEN], egressInterface[INTF_NAME_LEN];

	uint32_t		ingressIp, egressIp;

	ipf_nat_t		rtp_nat, rtcp_nat;
	IPNAT_T			*nat_rtp_p;
	IpHole_t		*idp, *hp;

	int				nop = (flag == ALLOCATE_ONLY), rc = 0, modified = 0;
	char 			*typestr = "MMH", c1_addr[32], c2_addr[32];

	ingressPort = egressPort = inboundPort = outboundPort = 0;
	ingressIp = egressIp = 0;
	
	memset(&rtp_nat, (uint32_t) 0, sizeof(ipf_nat_t));
	memset(&rtcp_nat, (uint32_t) 0, sizeof(ipf_nat_t));

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("ModifyMediaHole: entering \n") );

	lockMutex(&mutex);

	trc_debug(	MFCE,
			NETLOG_DEBUG2,
			"%s -> : bundle %d, hole %d\n",
			typestr, bundleId, holeId);

	if ((idp = GetIpHoleBlockForHoleId(holeId)) == NULL) {
		trc_error( 	MFCE,
				"ERROR	: ModifyMediaHole(): - can not locate"
				"hole %d\n", holeId);
		goto errorExit;
	}
	
	if ((peerResourceId != 0) && (hp = GetIpHoleBlockForHoleId(peerResourceId)) == NULL) {
		trc_error( 	MFCE,
				"ERROR	: ModifyMediaHole(): - can not locate"
				"hole %d allocated by peer\n", peerResourceId);
		goto errorExit;
	}	
	
	if (bundleId == BUNDLE_ANY) {
		bundleId = idp->bundleId;	// Overwrite the bundleId from idp, in case we don't have it
	}

	if ( (peerResourceId && (ingressPoolId == hp->egressPoolId)) ||
		 ((ingressPoolId != POOL_ANY) && (ingressPoolId != idp->ingressPoolId)) || 
		 ((*returnedNatDestIpAddress != 0) && (*returnedNatDestIpAddress != idp->ingressIp)) || 
		 ((*returnedNatDestPort != 0) && (*returnedNatDestPort != idp->ingressPort)) ) {
		// If the peer already has an allocated address on his egressPool use that
		if ( (peerResourceId && (ingressPoolId == hp->egressPoolId)) ) {
			inboundPort = hp->egressPort;
			ingressIp = hp->egressIp;
		}
		
		// free the previously allocated port		
		freePool(idp->ingressPoolId, idp->ingressPort, idp->ingressIp, rx);
	
		// Allocate the specified addresses
		if (allocatePool(ingressPoolId,
                     returnedNatDestPort,
                     returnedNatDestIpAddress,
                     ingressInterface, 
                     &ingressVlanId,
                     rx) )
		{
			trc_error(	MFCE,
				  	"ERROR  : ModifyMediaHole() - allocating "
				  	"MR ports on pool %d\n",
					ingressPoolId );
		        NETDEBUG(	MFCE, NETLOG_DEBUG4, ("ModifyMediaHole: will try to allocate a new port"));
			goto errorExit;
		}
		else {
			ingressPort = (uint16_t) *returnedNatDestPort;
		}

		modified = 1;
	}
	else {
		// use all the old params except dstIp and dstPort
		ingressPoolId = idp->ingressPoolId;
		ingressPort = idp->ingressPort;
		ingressIp = idp->ingressIp;
		nat_rtp_p = (IPNAT_T *) &idp->rtp_nat.data;
		nx_strlcpy(ingressInterface, nat_rtp_p->in_ifname, INTF_NAME_LEN);
	}

	if ( (peerResourceId && (egressPoolId == hp->ingressPoolId)) ||
		 ((egressPoolId != POOL_ANY) && (egressPoolId != idp->egressPoolId)) ||
		 ((*returnedNatSrcIpAddress != 0) && (*returnedNatSrcIpAddress != idp->egressIp)) || 
		 ((*returnedNatSrcPort != 0) && (*returnedNatSrcPort != idp->egressPort)) ) {
		// If the peer already has an allocated address on his egressPool use that
		if ( (peerResourceId && (egressPoolId == hp->ingressPoolId)) ) {
			outboundPort = hp->ingressPort;
			egressIp = hp->ingressIp;
		}	

		// free the previously allocated port		
		freePool(idp->egressPoolId, idp->egressPort, idp->egressIp, tx);

		// Allocate the specified addresses
		if (allocatePool(egressPoolId, 
                     returnedNatSrcPort, 
                     returnedNatSrcIpAddress,
                     egressInterface, 
                     &egressVlanId,
                     tx) )
		{
			trc_error(	MFCE,
				  	"ERROR  : ModifyMediaHole() - allocating "
				  	"MR ports on pool %d\n",
					egressPoolId );
		        NETDEBUG(	MFCE, NETLOG_DEBUG4, ("ModifyMediaHole: will try to allocate a new port"));
			goto errorExit;
		}
		else {
			egressPort = (uint16_t) *returnedNatSrcPort;
		}

		modified = 1;
	}
	else {
		// use all the old params except dstIp and dstPort
		egressPoolId = idp->egressPoolId;
		egressPort = idp->egressPort;
		egressIp = idp->egressIp;
		nat_rtp_p = (IPNAT_T *) &idp->rtp_nat.data;
		nx_strlcpy(egressInterface, nat_rtp_p->in_ifname_out, INTF_NAME_LEN);
	}

	if ((dstIpAddress != idp->dstIp) || (dstPort != idp->dstPort)) {
		if ((dstIpAddress == IP_ANY) || (dstPort == PORT_ANY)) {
			dstIpAddress = idp->dstIp;
			dstPort = idp->dstPort;
		}
		else {
			modified = 1;
		}
	}

	if (modified) {

		NETDEBUG(	MFCE, NETLOG_DEBUG4,
				("ModifyMediaHole: BundleId = %d, %s -> ingress = %s:%d@%s egress = %s:%d@%s\n",
				 bundleId, typestr,
				 FormatIpAddress(*returnedNatDestIpAddress, c1_addr), ingressPort, ingressInterface, 
				 FormatIpAddress(*returnedNatSrcIpAddress, c2_addr), egressPort, egressInterface));

			if (ModifyRedirect(	ingressInterface,
						egressInterface,
						holeId,
						ingressIp,
						ingressPort,
						dstIpAddress,
						dstPort,
						"rtp",
						&rtp_nat,
						egressIp,
						egressPort,
						1,
						nop,
						((hp)?&hp->rtp_nat:NULL),
						hp,
						dstSym,
						peerResourceId ) < 0 )
				goto errorExit;
	}

	unlockMutex(&mutex);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"%s <- : hole %d, bundle %d\n",
				typestr,
				holeId,
				bundleId);

	NETDEBUG(	MFCE,
		 		NETLOG_DEBUG4,
		 		("ModifyMediaHole: exiting - hole ID %d, bundle Id %d\n",
		  		holeId,
				bundleId));

	return( rc );

  errorExit:

	// Was a media nat redirect created ?
	UnplumbMediaHole(bundleId, -1, &rtp_nat, &rtcp_nat, ingressVlanId, nop);

	trc_error(	MFCE,
				"%s <- : bundle %d - Error\n",
				typestr,
				bundleId );

	NETERROR(	MFCE,
				("ModifyMediaHole: unable to modify hole %d, bundle %d\n",
				holeId,
				bundleId ));

	unlockMutex(&mutex);

	return( -1 );
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
//		bundleId
//
//		dstIpAddress		
//							-	remote destination ip address
//
//								This value is the ip address of the
//								ultimate media destination for this hole.
//								Packets associated with this hole will be
//								redirected to this ip address.
//								(host order)
//
//		dstPort			
//							-	remote destination port
//
//								This value identifies the first port of
//								a port pair on the remote endpoint identified
//								by dstIpAddress. Two consecutive holes will
//								be opened for RTP/RTCP.
//								(host order)
//
//
//		ingressPoolId			
//							-	poolid # of ingress pool
//
//		egressPoolId			
//							-	poolid # of egress pool
//
//		peerResourceId
//							- 	BundleId of the peer call-leg
//
//		returnedIpAddress
//							-	return value of ip address to which
//								media should be sent
//
//		returnedPort
//							-	return value of port to which media
//								should be sent
//
//		returnedResourceId
//							-	if the value is 1, the ports returned could be
//									re-used in IpfilterReopenHole call
//
//		dstSym              - flag indicating that the leg of the call being
//		                  	  created will use data nat traversal. If value
//		                      is non-zero, data nat traversal will be used 
//		                      for the leg being created.
//
//		flag		
//							-   0 - allocate and Create
//							    1 - allcoate only
//								2 - create and allocate hole
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
static int
OpenMediaHole(  uint32_t    bundleId,
                uint32_t    dstIpAddress,
                uint16_t    dstPort,
                uint32_t    ingressPoolId,
                uint32_t    egressPoolId,
                uint32_t    peerResourceId,
                uint32_t *  returnedNatSrcIpAddress,
                uint16_t *  returnedNatSrcPort,
                uint32_t *  returnedNatDestIpAddress,
                uint16_t *  returnedNatDestPort,
                uint32_t *  returnedResourceId,
                int         dstSym,
                int         flag)
{
	uint16_t		ingressPort;
	uint16_t		egressPort;

	char			ingressInterface[INTF_NAME_LEN];
	char			egressInterface[INTF_NAME_LEN];

	uint32_t		ingressIp;
	uint32_t		egressIp;

	int			ingressVlanId;
	int			egressVlanId;

	uint32_t		holeId;

	ipf_nat_t		rtp_nat, rtcp_nat;
	IpHole_t *		returned_hole_ptr;
	IpHole_t *		other_hp = (IpHole_t*) NULL;

	int				nop = (flag == ALLOCATE_ONLY);
	char 			* typestr = ( nop ? "AMH" : "OMH");
	char 			* prstr = ( nop ? "- Allocate" : "- Open");

	int				rc = 1;
	char			c1_addr[32], c2_addr[32];


	ingressIp = egressIp = 0;
	ingressPort = egressPort = 0;

	
	memset(&rtp_nat, (uint32_t) 0, sizeof(ipf_nat_t));
	memset(&rtcp_nat, (uint32_t) 0, sizeof(ipf_nat_t));

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("OpenMediaHole: entering %s\n", prstr) );

	lockMutex(&mutex);

	trc_debug(	MFCE,
			NETLOG_DEBUG2,
			"%s -> : ingressPool %d, egressPool %d, bundle %d\n",
			typestr,
			ingressPoolId,
			egressPoolId,
			bundleId );

	holeId = newHoleId();

	// Does the peer leg of the call exist yet, meaning this is the second leg
	// of the call ?

	if (peerResourceId != 0) {
		// Yes, lookup the entry for the other leg. The other leg will be used
		// will be used in PlumbMediaHole() for dnat relate functions if needed.
		if ((other_hp = GetIpHoleBlockForHoleId(peerResourceId)) == NULL) {
			trc_error( 	MFCE,
						"ERROR	: OpenMediaHole(): - can not locate"
						"hole %d allocated by peer\n", peerResourceId);
			NETDEBUG(MFCE, NETLOG_DEBUG4,("OpenMediaHole(): - can not locate hole %d allocated by peer\n", peerResourceId));
			goto errorExit;
		}
	}

	NETDEBUG(MFCE, NETLOG_DEBUG4,("value for flag 0x%8x\n", rtp_nat.data.in_flags));

	// Allocate port on the ingress side
	if (allocatePool(	ingressPoolId,
				returnedNatDestPort,
				returnedNatDestIpAddress,
				ingressInterface,
				&ingressVlanId,
				rx) ) {
		trc_error(	MFCE,
			  	"ERROR  : OpenMediaHole() - allocating "
			  	"MR ports on pool %d\n",
				ingressPoolId );
		goto errorExit;
	}

	// fill ingressPort only if allocation is succesful otherwise let it be 0

	ingressPort = (uint16_t) *returnedNatDestPort;

	// Allocate port on the egress side
	if (allocatePool(	egressPoolId,
				returnedNatSrcPort,
				returnedNatSrcIpAddress,
				egressInterface,
				&egressVlanId,
				tx)) {
		trc_error(MFCE,
			  "ERROR  : OpenMediaHole() - allocating "
			  "MR ports on pool %d\n",
			  egressPoolId );
		goto errorExit;
	}

	// fill egressPort only if allocation is succesful otherwise let it be 0
	
	egressPort = (uint16_t) *returnedNatSrcPort;

	NETDEBUG(	MFCE, NETLOG_DEBUG4,
			("OpenMediaHole: BundleId = %d, peerResourceId = %d, %s -> ingress = "
			 "%s:%d@%s egress = %s:%d@%s\n",
				 bundleId, peerResourceId,
				 typestr,
				 FormatIpAddress(*returnedNatDestIpAddress, c1_addr),
				 ingressPort,
				 ingressInterface, 
				 FormatIpAddress(*returnedNatSrcIpAddress, c2_addr),
				 egressPort,
				 egressInterface));

	// Make call to add redirects via AddRedirect() subroutine call
	// via either hknife or NSF

	if (PlumbMediaHole(	holeId,
				ingressInterface,
				egressInterface,
				*returnedNatDestIpAddress,
				ingressPort,
				*returnedNatSrcIpAddress,
				egressPort,
				dstIpAddress,
				dstPort,
				ingressVlanId,
				egressVlanId,
				&rtp_nat,
				&rtcp_nat,
				nop, 
				other_hp,
				dstSym,
				peerResourceId ) < 0)
		goto errorExit;

	// Allocate memory resources to store redirect info in cache for redirects
	//      Note : this better be the same hole id we calculated above

	rc = AddMediaHole(	bundleId,
						holeId,
					  	&rtp_nat,
					  	&rtcp_nat,
					  	ingressPoolId,
					  	*returnedNatDestIpAddress,
						ingressPort,
						ingressVlanId,
						egressPoolId,
						*returnedNatSrcIpAddress,
						egressPort,
						egressVlanId,
						dstIpAddress,
						dstPort,
						nop,
						&returned_hole_ptr );

	if (!rc)
	{
		NETERROR(	MFCE,
					("OpenMediaHole: unable to add media hole, "
					"%d to %d, bundle %d\n",
					ingressPoolId,
					egressPoolId,
					bundleId));
		goto errorExit;
	}

	*returnedResourceId = holeId;

	// If this is the first leg of a call with data nat specified
	// then set the DNAT_RELATE_PENDING flag

	if ( dstSym && returned_hole_ptr )
		returned_hole_ptr->flags |= DNAT_RELATE_PENDING;

	unlockMutex(&mutex);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"%s <- : hole %d, bundle %d, dstSym %d\n",
				typestr,
				holeId,
				bundleId,
				dstSym );

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("OpenMediaHole: exiting - hole ID %d, bundle Id %d\n",
			  	holeId,
				bundleId));

	return( 0 );

  errorExit:

	UnplumbMediaHole(bundleId, -1, &rtp_nat, &rtcp_nat, ingressVlanId, nop);

	if (ingressPort != 0) {
		freePool(	ingressPoolId, ingressPort, *returnedNatDestIpAddress, rx);
	}

	if (egressPort != 0) {
		freePool(	egressPoolId, egressPort, *returnedNatSrcIpAddress, tx);
	}

	trc_error(	MFCE,
				"%s <- : bundle %d - Error\n",
				typestr,
				bundleId );

	NETERROR(	MFCE,
				("OpenMediaHole: unable to open hole, %d to %d, bundle %d\n",
				ingressPoolId,
				egressPoolId,
				bundleId ));

	unlockMutex(&mutex);

	return( -1 );
}

//
//	Function 	:
//		AddMediaHole()
//
//	Purpose		:
//
//		This function adds a new media IpHole_t
//		to the chain of IpHole_t structures and
//		fills it in with the data passed in as
//		arguments. It returns a uniquely assigned
//		integer that can be used to reference the
//		IpHole_t for update or deletion purposes.
//
//	Arguments	:
//
//		bundleId				- bundle id for the new pinhole
//
//		holeId					- hole id for the new pinhole
//
//		rtp_nat_ptr				- a pointer to a rtp ipf_nat_t for redirect
//
//		rtcp_nat_ptr			- a pointer to a rtcp ipf_nat_t for redirect
//
//		ingressPoolId			- poolid # of incoming pool
//
//		ingressIp				- local ip address to which media will be
//									sent
//
//		ingressPort				- first local port of port pair to which media
//									will be sent
//
//		egressPoolId			- poolid # of outbound pool
//
//		egressIp				- local ip address from which media will
//									appear to be coming at final
//									destination
//									(source ip address after nat)
//
//		egressPort				- first port of port pair from which media
//									will appear to be coming at final
//									destination
//									(source l4 port after nat)
//
//		dstIp				    - real ip address to which media will be sent
//									after nat operation.
//									(dest ip address after nat)
//
//		dstPort				    - real l4 port to to which media will be sent
//									after nat operation.
//									(dst l4 port after nat)
//									
//
//	Return Value :
//
//		assigned Id of hole			on success
//		0							on failure - if one cannot be allocated
//
static uint32_t
AddMediaHole(	uint32_t	bundleId,
				uint32_t	holeId,
			 	ipf_nat_t *	rtp_nat_ptr,
			 	ipf_nat_t *	rtcp_nat_ptr,
			 	uint32_t	ingressPoolId,
			 	uint32_t	ingressIp,
			 	uint16_t	ingressPort,
				uint32_t  ingressVlanId,
			 	uint32_t	egressPoolId,
				uint32_t	egressIp,
				uint16_t	egressPort,
				uint32_t  egressVlanId,
				uint32_t	dstIp,
				uint16_t	dstPort,
				int			allocateOnly, 
				IpHole_t **	returned_hole_ptr )
{
	IpHole_t *ptr;
	int allocated = 0;

	*returned_hole_ptr = (IpHole_t *) NULL; // assume failure

	if (!holeId) {
		return 0;
	}

	// Check if an entry already exists
	ptr = GetIpHoleBlockForHoleId(holeId);

	if (ptr) {
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("will update IpHole entry for hole %d\n", holeId));
	}
	else {
		ptr = IpHoleAlloc(bundleId, holeId);
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("will create new IpHole entry for hole %d\n", holeId));
		allocated = 1;
	}

	if (ptr == NULL)
		return( 0 );

	*returned_hole_ptr = ptr;

	if (allocateOnly) {
		ptr->flags = HOLE_ALLOCATE;
	}

	if (rtp_nat_ptr->rdr_defined)
		memcpy(&ptr->rtp_nat, rtp_nat_ptr, sizeof(ipf_nat_t));

	if (rtcp_nat_ptr->rdr_defined)
		memcpy(&ptr->rtcp_nat, rtcp_nat_ptr, sizeof(ipf_nat_t));

	ptr->ingressPoolId = ingressPoolId;
	ptr->ingressIp = ingressIp;
	ptr->ingressPort = ingressPort;
	ptr->egressPoolId = egressPoolId;
	ptr->egressIp = egressIp;
	ptr->egressPort = egressPort;
	ptr->dstIp = dstIp;
	ptr->dstPort = dstPort;
	ptr->ingress_vlan = ingressVlanId;
	ptr->egress_vlan = egressVlanId;
	
	if (allocated) {
		// Add the new block to the HoleId hash table cache
		CacheInsert(HoleIdCache, ptr);

		// Add the new block to the BundleId hash table cache
		CacheInsert(BundleIdCache, ptr);
	}

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
//		bundleId
//
//		signalingInterface	-
//								char string representing the interface
//								on which packets will enter and exit
//								the MSW for signaling traffic associated
//								with the hole being opened.
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
//							-	local port number used for signaling
//								traffic
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
//		protocol				
//							-	protocol used for signaling traffic
//								through this hole being opened
//
//								valid values are one of the following
//								character strings :
//									"tcp" or "udp"
//
//		returnedIpAddress
//							-	returned ip address to be used
//								by endpoint to converse with MSW
//
//		returnedMediaPort
//							-	returned value of port to be used
//								by endpoint to converse with MSW
//
//	Return Value :
//
//			 0		on Success
//			-1		on Failure
//
static int
OpenSignalingHole(	uint32_t	bundleId,
					char *		signalingInterface,
				  	uint32_t	signalingIpAddress,
				  	uint16_t	signalingPort,
				  	char *		protocol,
					uint32_t *	returnedResourceId )
{
	uint32_t holeId;
	char 	c_addr[32];
	int		rc;

	ipf_pmhole_t ipf_pmhole_signaling;


	memset(&ipf_pmhole_signaling, (int) 0, sizeof(ipf_pmhole_t));

	trc_debug(	MFCE,
				NETLOG_DEBUG2, "OSH -> : \n" );

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("OpenSignalingHole: entering\n") );

	lockMutex(&mutex);

	holeId = newHoleId();

	// call nsf via ioctl to open specified signaling pinhole

	if (	AddPMHole(	signalingInterface,
				  		signalingIpAddress,
				  		holeId,
						bundleId,
						signalingPort,
						(( !strncasecmp( protocol, "udp", 3 ) ) ? IPPROTO_UDP :
						 										  IPPROTO_TCP ),
						&ipf_pmhole_signaling ) < 0 )
	{
		trc_error(	MFCE,
					"       : Error - failed to open local signaling hole\n");
		trc_error(	MFCE,
				  	"       :   %s:%d %s on %s\n",
				  	FormatIpAddress(
						signalingIpAddress,
						c_addr),
				  	signalingPort,
					(( !strncasecmp( protocol, "udp", 3 ) ) ? "UDP" : "TCP" ),
					signalingInterface );
		goto errorExit;
	}

	ipf_pmhole_signaling.port = signalingPort;
	ipf_pmhole_signaling.hole_defined = TRUE;

	// Allocate and save hole information for later removal

	if ( ( rc = AddSignalingHole(	bundleId,
									holeId,
									&ipf_pmhole_signaling) ) == 0 )
	{
		trc_error(	MFCE,
					"       : Error - failed to open local signaling hole\n");
		trc_error(	MFCE,
				  	"       :   %s:%d %s on %s\n",
				  	FormatIpAddress(
						signalingIpAddress,
						c_addr),
				  	signalingPort,
					(( !strncasecmp( protocol, "udp", 3 ) ) ? "UDP" : "TCP" ),
					signalingInterface );
		goto errorExit;
	}

	*returnedResourceId = holeId;

	unlockMutex(&mutex);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"OSH <- : hole %d, bundle %d\n",
				holeId,
				bundleId );

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("OpenSignalingHole: exiting - hole ID %d, bundle Id %d\n",
			  	holeId,
				bundleId ) );

	return( 0 );

  errorExit:

	// Does a portmap pinhole exist for signaling ?

	if (ipf_pmhole_signaling.hole_defined)
	{
		// Yes, remove the port map hole
		RemovePMHole(	-1,
						bundleId,
						&ipf_pmhole_signaling );
	}

	trc_error(	MFCE,
				"OSH <- : bundle %d - Error\n",
				bundleId);

	unlockMutex(&mutex);

	return( -1 );
}

static uint32_t
AddSignalingHole(	uint32_t		bundleId,
					uint32_t		holeId,
					ipf_pmhole_t *	ipf_pmhole_signaling_ptr )
{
	IpHole_t *ptr;

	ptr = IpHoleAlloc(bundleId, holeId);

	if (ptr == NULL)
		return( 0 );

	if (ipf_pmhole_signaling_ptr->hole_defined)
	{
		memcpy(	&ptr->ipf_pmhole_signaling,
				ipf_pmhole_signaling_ptr,
				sizeof(ipf_pmhole_t) );
	}

	// Add the new block to the HoleId hash table cache

	CacheInsert(HoleIdCache, ptr);

	// Add the new block to the BundleId hash table cache

	CacheInsert(BundleIdCache, ptr);

	return( ptr->assignedId );
}

/**
 * This method is used to modify a resource that has already been created
 * using the OpenResource call.
 *
 * This modifies the original NAT translations such that the traffic flow becomes:
 *    ANY--->returnedIpAddress/returnedPort from ingressPoolId
 *    Ip/Port from newEgressPoolId--->newDstIpAddress/newDstPort
 * where,
 *   the returnedIpAddress/returnedPort and ingressPoolId are from the original
 * OpenResource call.
 *
 * Parameters:
 *  resourceId - the resourceId that was returned in the original OpenResource call
 *  newDstIpAddress - the IP address where the traffic will terminate
 *  newDstPort - the port where the traffic will terminate
 *  newEgressPoolId - the pool from which the egress side of the hole will be allocated
 *
 *	dstSym              - flag indicating that the leg of the call being
 *	                	  created will use data nat traversal. If value
 *		                  is non-zero, data nat traversal will be used 
 *		                  for the leg being created.
 *
 * Returns:
 *   0 - on sucess
 *  -1 - on failure
 */
int
nsfGlueModifyResource(	uint32_t	bundleId,
			uint32_t	resourceId,
			uint32_t	newDstIpAddress,
			uint16_t	newDstPort,
			uint32_t	newIngressPoolId,
			uint32_t	newEgressPoolId,
			uint32_t	peerResourceId,
			uint32_t *	returnedNatSrcIpAddress,
			uint16_t *	returnedNatSrcPort,
			uint32_t *	returnedNatDestIpAddress,
			uint16_t *	returnedNatDestPort,
			int		    dstSym,
			int		    allocateCreate)
{
	uint32_t *resId;
	int rc = -1;
	//uint32_t retIpAddr = 0;
	//uint16_t retPort = 0;
	
	resId = (uint32_t *)malloc(sizeof(uint32_t));
	*resId = resourceId;
	
	if (allocateCreate & CREATE_ONLY) {
		rc = nsfGlueOpenResource(	bundleId,
						newDstIpAddress,
						newDstPort,
						0,
						0,
						"rtp",
						0,
						returnedNatSrcIpAddress,
						returnedNatSrcPort,
						returnedNatDestIpAddress,
						returnedNatDestPort,
						resId,
						dstSym,
						allocateCreate);
	}
	else {
		rc = ModifyMediaHole(bundleId,
						resourceId,	
						newDstIpAddress,
						newDstPort,
						newIngressPoolId,
						newEgressPoolId,
						peerResourceId,
						returnedNatSrcIpAddress,
						returnedNatSrcPort,
						returnedNatDestIpAddress,
						returnedNatDestPort,
						dstSym,
						allocateCreate
				);
	}

	free(resId);
	return rc;
}

/**
 * This method is called to close a single hole/resource that was opened.
 *
 * Parameters:
 *  resourceId - the resourceId that was returned in the original OpenResource call
 *
 * Returns:
 *   0 - on sucess
 *  -1 - on failure
 */
int
nsfGlueCloseResource(uint32_t resourceId)
{
	IpHole_t *	idp;
	uint32_t	bundleId;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"DH  -> : resourceId %d\n",
				resourceId );

	NETDEBUG(MFCE, NETLOG_DEBUG4, ("nsfGlueCloseResource: entering\n"));

	lockMutex(&mutex);

	// get the assigned id block

	idp = GetIpHoleBlockForHoleId(resourceId);

	if (idp == NULL)
	{
		NETERROR(MFCE,
				 ("IpfilterCloseHole: Cannot find assigned ID %d in list\n",
				  resourceId));

		trc_error(	MFCE,
					"DH  <- : Error - resourceId %d not found\n",
					resourceId );

		unlockMutex(&mutex);
		return( -1 );
	}

	bundleId = idp->bundleId;

	CloseHole(idp);

	DeleteIpHole(idp);

	trc_debug(	MFCE,
			  	NETLOG_DEBUG2,
				"DH  <- : resourceId %d, bundle %d\n",
				resourceId,
				bundleId);

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("nsfGlueCloseResource: exiting - hole ID %d, bundle Id %d\n",
			 	resourceId,
			 	bundleId));

	unlockMutex(&mutex);

	return( 0 );
}

//
//	Function 	:
//		nsfGlueCloseBundle()
//
//	Purpose		:
//		This helper function is called to close all resources that are
//		associated with the specified bundle.
//
//	Arguments	:
//		bundleId	-  the bundle id whose resources need freeing
//
//	Return Value :
//		0 - on sucess
//
int
nsfGlueCloseBundle(uint32_t bundleId)
{
	lockMutex(&mutex);

	DeleteIpHoleForBundle(bundleId);

	NETDEBUG(	MFCE,
				NETLOG_DEBUG4,
				("Successfully closed bundle %d\n",
				bundleId));

	unlockMutex(&mutex);

	return( 0 );
}

// >*****************************************
// >*
// >* Definitions of Local entry points
// >*
// >*****************************************

// 
// Function :
// 		RebuildRules()
// 
// Purpose :
// 
// 		This function calls the nsfconfig script via a 
// 		call to execd passing the argument string sent
// 		by the caller.
// 
// Arguments :
// 		args	a character string containing the arguments
// 				to be given to the nsfconfig script
// 
// None.
// 
// Return Value :
// 
// None.
// 
static void
RebuildRules( char * args )
{
	struct stat	statbuf;
	int			selfid, rc;
	char 		cmd[512];
	char		msg[MAX_MSGLEN] = {0};

	if ( stat( fwconfig_path, (struct stat *) &statbuf ) < 0 ) // does script exist ?
	{
		// No, log error and return

		NETERROR(	MFCE, 
					("RebuildRules : file, \"%s\", not found.\n",
               		fwconfig_path ));
		return;
   	}
	else
	{
		if ( !( statbuf.st_mode & S_IXUSR ) ) // is script executable ?
		{
			// No, log error and return

       		NETERROR(	MFCE,
               			("RebuildRules : file, \"%s\", not executable.\n",
						fwconfig_path ));
			return;
		}
	}

	sprintf(	cmd,
				"%s %s\n",
				fwconfig_path,
				args );

	selfid = (	((mypid & 0xffff) << 16) | 
				 (pthread_self() & 0xffff) );

	if ((rc = sys_execd(	msgQueueId,
							selfid,
							SRVR_MSG_TYP,
							(1<<REQ_BIT)|(1<<OUT_BIT),
							cmd,
							msg,
							MAX_MSGLEN)) < 0)
	{
   		NETERROR(	MFCE,
					("RebuildRules : unable to sys_execd "
					"(%d): %s\n\t%s\n",
					rc,
					cmd,
					msg));
	}

	return;
}

//
//	Function 	:
//		CloseHole()
//
//	Purpose		:
//		This helper function issues the nessesary ioctls to
//		actually close the hole given a pointer to the hole block
//		describing it.
//
//	Arguments	:
//		idp	-  IpHole_t * of hole to be closed
//
//	Return Value :
//		None.
//
static void
CloseHole(	IpHole_t * idp )
{
	int 	nop = (idp->flags & HOLE_ALLOCATE);

	// Does a nat redirect exist for the hole ?
	if (idp->rtp_nat.rdr_defined == TRUE)
	{
		// Yes, remove associated nat translations

		// Remove rtp nat translation
		RemoveRedirect(	idp->bundleId,
					   	idp->assignedId,
					   	(void *) &idp->rtp_nat.data,
					   	idp->rtp_nat.rdr_string_1,
					   	idp->rtp_nat.rdr_string_2,
					   	idp->rtp_nat.rdr_string_3,
					   	"RTP",
						idp->ingress_vlan,
						nop );

		ActiveRedirects--;

		idp->rtp_nat.rdr_defined = FALSE;
	}

	// Does a portmap pinhole exist for signaling ?
	if (idp->ipf_pmhole_signaling.hole_defined)
	{
		// Yes, remove the port map hole
		RemovePMHole(	idp->assignedId,
						idp->bundleId,
						&idp->ipf_pmhole_signaling);
	}

	// Was a port pair allocated for this port ?

	if (idp->ingressIp != 0)
		freePool(	idp->ingressPoolId,
					idp->ingressPort,
					idp->ingressIp, rx );

	if (idp->egressIp != 0)
		freePool(	idp->egressPoolId,
					idp->egressPort,
					idp->egressIp, tx );

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("closed hole %d, bundle %d\n",
				idp->assignedId,
				idp->bundleId));

	return;
}

// 
// Function :
// RemoveRedirect()
// 
// Purpose :
// 
// This helper function issues the nessesary ioctls to
// actually remove a redirect associated with a pinhole
// given a pointer to the specified information
// 
// Arguments :
// 
// bundleId - bundleId in which rule is contained
// 
// holeId - holeId of redirect
// 
// ipnat_ptr - pointer to ipnat_t or ipnat64_t for
// redirect
// 
// rule_string_1 - pointer to character string containing
// first line of redirect description
// 
// rule_string_2 - pointer to character string containing
// second line of redirect description
// 
// ctype - character string containing redirect type
// "RTP" or "RTCP"
// 
// Return Value :
// 
// None.
// 
// 
static void
RemoveRedirect(uint32_t bundleId,
			   uint32_t holeId,
			   void *ipnat_ptr,
			   char *rdr_string_1,
			   char *rdr_string_2,
			   char *rdr_string_3,
			   char *ctype,
				uint32_t ingressVlan,
			   int nop)
{
	uint32_t lerrno;

	IPNAT_T *nat_entry_ptr = (IPNAT_T *) ipnat_ptr;

	if (nop) {
		return;
	}

	// Remove redirect

		if( hknife_delete_redirect( 
               getPortnumFromIfacename(nat_entry_ptr->in_ifname), /* ingress ethport */
               ingressVlan,                                       /* vlan id */
               ntohl( nat_entry_ptr->in_out[0].s_addr ),          /* ip_daddr */
               ntohs( nat_entry_ptr->in_pmin ),                   /* l4_dport */
               nat_entry_ptr->in_p                                /* l4_proto */ ) < 0 ) {
			lerrno = errno;

			trc_error(MFCE,
					  "       : ERROR hk_delete_redirect(%s) - hole %d, err : %s\n",
					  ctype, holeId, strerror(lerrno));

			NETERROR(MFCE,
					 ("RemoveRedirect() : Error removing "
					  "nat translation - holeId %d - (%s) err : %s\n",
						  holeId, ctype, strerror(lerrno)));
			return;
		}

	NETDEBUG(MFCE,
			 NETLOG_DEBUG4, ("       : removed - NAT media redirect translation\n"));

	trc_debug(MFCE,
			NETLOG_DEBUG2, "       : removed - NAT media redirect translation\n");

	trc_debug(MFCE, NETLOG_DEBUG2, "       :   %s\n", rdr_string_1);

	trc_debug(MFCE, NETLOG_DEBUG2, "       :   %s\n", rdr_string_2);

	trc_debug(MFCE, NETLOG_DEBUG2, "       :   %s\n", rdr_string_3);

	return;
}

//
//	Function 	:
//		IpHoleAlloc()
//
//	Purpose		:
//
//		This function allocates free storage for a new
//		hole.
//
//	Arguments	:
//
//		bundleId		bundleId assosiated with hole being allocated
//
//	Return Value :
//
//		NULL				on failure - free storage exahauted
//		IpHole_t *			on success
//
static IpHole_t *
IpHoleAlloc(	uint32_t bundleId , uint32_t holeId)
{
	IpHole_t *	ptr;

	ptr = (IpHole_t *) malloc(sizeof(IpHole_t));

	if (ptr == NULL)
	{
		NETERROR(MFCE, ("Cannot allocate IDBlock\n"));
		return( NULL );
	}

	memset(ptr, (int32_t) 0, sizeof(IpHole_t));

	ptr->assignedId = holeId;
	ptr->bundleId = bundleId;

	return( ptr );
}

// 
// Function :
// 		GetIpHoleBlockForHoleId()
// 
// Arguments :
//		assignedId	-	holeId associated with IpHole_t to
//						be found.
// 
// Purpose :
// 
//		Finds and returns the IpHole_t associated with
//		this specified HoleId, NULL if not present
// 
// Return Value:
// 		ipHoleBlock_t * 	IpHole_t is found
// 		0					if IpHole_t not found
// 
static IpHole_t *
GetIpHoleBlockForHoleId(uint32_t assignedId)
{
	IpHole_t *	ptr;
	char *		fn = "GetIpHoleBlockForHoleId():";

	if ((ptr = CacheGet(HoleIdCache, &assignedId)) == NULL)
	{
		NETDEBUG(	MFCE, NETLOG_DEBUG4,
				 	("%s could not find cache entry for hole id %d\n",
					fn,
					assignedId));
	}

	return( ptr );
}

// 
// Function :
// 		GetIpHoleBlockForBundleId()
// 
// Arguments :
//		bundleId	-	bundleId associated with IpHole_t to
//						be found.
// 
// Purpose :
// 
//		Finds and returns the IpHole_t associated with
//		this assigned bundleId, NULL if not present
// 
// Return Value:
// 		ipHoleBlock_t * 	IpHole_t is found
// 		0					if IpHole_t not found
// 
// 
static IpHole_t *
GetIpHoleBlockForBundleId(uint32_t bundleId)
{
	IpHole_t *	ptr;

	ptr = CacheGet(BundleIdCache, &bundleId);

	return( ptr );
}

// 
// Function :
// 		DeleteIpHole()
// 
// Arguments :
//		ptr		- 	pointer to IpHole_t representing
//					hole to be deleted.
// 
// Purpose :
// 
//		Deletes the given IpHole_t from the internal list
// 
// Return Value:
//		None.
// 
static void
DeleteIpHole(	IpHole_t * ptr )
{
	IpHole_t *retHole;
	char *fn = "DeleteIpHole():";

	// CacheDelete returns a chain of elements with the same key

	if ((retHole = CacheDelete(HoleIdCache, &ptr->assignedId)) == NULL)
	{
		NETERROR(MFCE,
				 ("%s Cannot find assigned hole id %d in holeid cache\n",
				  fn, ptr->assignedId));
	}

	ListgDelete(ptr, BUNDLEID_LIST_OFFSET);
	BundleIdCache->nitems--;

	free(ptr);
}

// 
// Function :
// 		DeleteIpHoleForBundle()
// 
// Arguments :
//		bundleId	-	assigned bundleId associated with hole
// 
// Purpose :
// 
//			Deletes all the IpHole_t structures associated with
//		this bundle id
// 
// Return Value:
//
//		None.
// 
static void
DeleteIpHoleForBundle(	uint32_t bundleId )
{
	IpHole_t *idp;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"DS  -> : bundle %d\n",
				bundleId);

	while ((idp = GetIpHoleBlockForBundleId(bundleId)) != NULL)
	{
		CloseHole( idp );
		DeleteIpHole( idp );
	}

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"DS  <- : bundle %d\n", 
				bundleId);
}

static void
Hexdump(uchar_t * input, int32_t nBytes, uint32_t firstByte)
{
	char output[80];			// Accumulates the formated output
	int32_t nHex;				// Indexes the hex digits in "output"
	int32_t nAlpha;				// Indexes the alpha characters in "output"
	int32_t i;					// Temporary storage
	char outbuf[1024];

	memset(outbuf, (int32_t) 0, 1024);

	if (!input)
		return;

	while (nBytes > 0)
	{
		if (firstByte < 0)
			sprintf(output, "%08x:", (uint32_t) input);
		else
			sprintf(output, "%08x:", firstByte);

		for (i = 9; i < 80; ++i)
			output[i] = ' ';
		nHex = 11;
		nAlpha = 48;
		for (i = 1; (i <= 16) && (nBytes > 0); ++i)
		{
			// Convert a byte to hex...
			output[nHex] = "0123456789ABCDEF"[*input / 16];
			output[nHex + 1] = "0123456789ABCDEF"[*input % 16];
			/*
			 * and translate it to alpha 
			 */
			output[nAlpha] = ((*input >= 0x20) && (*input < 0x7f)) ? *input : '.';
			++input;
			--nBytes;
			++nAlpha;
			nHex += 2;

			// Insert a blank every 5th byte
			if ((i % 4) == 0)
				++nHex;
		}

		output[nAlpha] = '\0';	/* End of line (16 bytes) */

		printf("%s\n", output);

		if (firstByte >= 0)
			firstByte += 16;
	}
}

//
//	Function 	:
//		AddRedirect()
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
//      nop             -	if non-zero dont actually setup redirect
//
//		other_rdr_ptr	-	pointer to other ipf_nat_t * if it exists or NULL
//
//		other_hp	    -	pointer to other IpHole_t * if it exists or NULL
//
//		dstSym          - flag indicating that the leg of the call being
//		                  created will use data nat traversal. If value
//		                  is non-zero, data nat traversal will be used 
//		                  for the leg being created.
//
//		peerResourceId	-	holeid of other hole 
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
AddRedirect(	char *		iface_in,
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
				int32_t		mapflag,
				int			nop,
				ipf_nat_t *	other_rdr_ptr,
				IpHole_t *	other_hp,
				int			dstSym,
				uint32_t	peerResourceId,
        int ingressVlanId,
        int egressVlanId )
{
	char *			c_protocol = "UDP";
	char				c_localaddr[32];
	char				c_localaddr2[32];
	char				c_localaddr3[32];
	IPNAT_T *			nat_rdr_ptr = (IPNAT_T *) &rdr_ptr->data;
	IPNAT_T *			other_nat_rdr_ptr = (IPNAT_T *) &other_rdr_ptr->data;
	uint32_t			nat_sym = 0;
    int             	relate_leg1_leg2 = 0, relate_leg2_leg1 = 0;

	// Are we ready to issue any needed dnat relate operations for a call if
	// needed ? This can only be done after we have a pair of unidirectional 
	// related media legs. Do we have a peer leg yet ?

	if (other_hp) {
		// Yes, This is the second leg. get information about the peer resourceId
		// - i.e. - the first leg

		nat_rdr_ptr->in_legkey = other_nat_rdr_ptr->in_legkey;

		// Does the other leg need a data nat operation ?

		if( other_hp->flags & DNAT_RELATE_PENDING ) {
			// Yes, indicate as much
			relate_leg2_leg1 = 1;
		}

		// Does this leg need a data nat operation ?

		if(dstSym)
			relate_leg1_leg2 = 1; // Yes, indicate as much
	}

	// Specify its nat type as NAT_REDIRECT

	nat_rdr_ptr->in_redir = NAT_REDIRECT;

	// Specify inbound interface

	strcpy(nat_rdr_ptr->in_ifname, iface_in);

	// Specify outbound interface

	strcpy(nat_rdr_ptr->in_ifname_out, iface_out);

	// Specify what destination ip should look like when coming into
	// the specified interface from the outside world inorder to
	// be redirected

	nat_rdr_ptr->in_outip = htonl(orig_dst_ip);
	nat_rdr_ptr->in_outmsk = -1;
	nat_rdr_ptr->in_p = (char)17; /* UDP */

	// Specify the port from which the packet should come

	nat_rdr_ptr->in_pmin = nat_rdr_ptr->in_pmax = htons(orig_dst_port);

	// Specify the ip to which the packet will be
	// redirected on the internal network

	nat_rdr_ptr->in_inip = htonl(new_dst_ip);
	nat_rdr_ptr->in_inmsk = -1;

	if (mapflag)
	{
		nat_rdr_ptr->in_map_srcip.s_addr = htonl(map_src_ip);
		nat_rdr_ptr->in_map_port = htons(map_src_port);
		nat_rdr_ptr->in_flags = IPN_XMAP;
	}

	// Specify the port to which the packet will be
	// redirected

	nat_rdr_ptr->in_pnext = htons(new_dst_port);

	// Specify the protocol

	nat_rdr_ptr->in_flags |= IPN_UDP;

	// specify data NAT symmetry

	nat_rdr_ptr->in_flags |= nat_sym;

	memset(rdr_ptr->rdr_string_1, (int32_t) 0, 128);
	memset(rdr_ptr->rdr_string_2, (int32_t) 0, 128);
	memset(rdr_ptr->rdr_string_3, (int32_t) 0, 128);

	if (mapflag)
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress(orig_dst_ip, c_localaddr), orig_dst_port);

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d",
				iface_out,
				FormatIpAddress(new_dst_ip, c_localaddr2), new_dst_port);

		sprintf(rdr_ptr->rdr_string_3,
				"    proto %s - xmap src %s, port %d",
				c_protocol,
				FormatIpAddress(map_src_ip, c_localaddr3), map_src_port);
	}
	else
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress(orig_dst_ip, c_localaddr), orig_dst_port);

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d",
				iface_out,
				FormatIpAddress(new_dst_ip, c_localaddr2), new_dst_port);

		sprintf(rdr_ptr->rdr_string_3, "    proto %s", c_protocol);
	}

	if (nop) {
		return(0);
	}

		if ( mapflag ) {
			if( hknife_add_redirect( getPortnumFromIfacename(iface_in),
                               ingressVlanId,
                               orig_dst_ip,
                               orig_dst_port,
                               17,
                               map_src_ip,
                               map_src_port,
                               new_dst_ip,
                               new_dst_port,
                               getPortnumFromIfacename(iface_out),
                               egressVlanId,
                               &rdr_ptr->legkey ) != 0 ) {
				trc_error(	MFCE,
					"       : ERROR hknife_add_redirect() - (%s) hole %d\n",
					rdr_type,
					holeId );
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
					( "Error creating local translation (%s)\n",
					rdr_type ));
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

			// relate the resources for discovery

			NETDEBUG(MFCE, NETLOG_DEBUG4,
					("AddRedirect(): relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1) );
			trc_debug(MFCE, NETLOG_DEBUG2,
					"       : relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1 );

			if(relate_leg1_leg2) {
				if ( hknife_dnat_relate(	rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr,
											other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("AddRedirect(): relatedn leg_1_2 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_1_2 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr  );
				}
			}

			if(relate_leg2_leg1) {
				if ( hknife_dnat_relate(	other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr,
											rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("AddRedirect(): relatedn leg_2_1 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_2_1 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr  );
				}
			}
		}
    else
    {
			if( hknife_add_redirect( getPortnumFromIfacename(iface_in),
                               ingressVlanId,
                                     orig_dst_ip,
                                     orig_dst_port,
                                     17,
			                               0,
                                     0,
                                     new_dst_ip,
                                     new_dst_port,
									                   getPortnumFromIfacename(iface_out),
                                     egressVlanId,
                                     &rdr_ptr->legkey ) != 0 ) {
				trc_error(	MFCE,
					"       : ERROR hknife_add_redirect() - (%s) hole %d\n",
					rdr_type,
					holeId );
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
					( "Error creating local translation (%s)\n",
					rdr_type ));
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

			// relate the resources for discovery

			NETDEBUG(MFCE, NETLOG_DEBUG4,
					("AddRedirect(): relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1) );
			trc_debug(MFCE, NETLOG_DEBUG2,
					"       : relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1 );

			if(relate_leg1_leg2) {
				if ( hknife_dnat_relate(	rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr,
											other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("AddRedirect(): relatedn leg_1_2 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_1_2 failed hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr  );
				}
			}

			if(relate_leg2_leg1) {
				if ( hknife_dnat_relate(	other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr,
											rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("AddRedirect(): relatedn leg_2_1 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_2_1 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr  );
				}
			}
  }
  
	rdr_ptr->rdr_defined = TRUE;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       : added - NAT redirect translation (%s) \n",
				rdr_type);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_1);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_2);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_3);
		trc_debug( MFCE,
                   NETLOG_DEBUG2,
				   "       :   returned legkey: hv %d, redir_addr 0x%08x\n",
				   rdr_ptr->legkey.hv, rdr_ptr->legkey.redir_addr );

	if(relate_leg1_leg2) {
			trc_debug(	MFCE, 
						NETLOG_DEBUG2,
						"       :   hknife_dnat_relate() - leg_1_2 for hid %d, phid %d\n",
						holeId,
						peerResourceId );
			trc_debug(	MFCE,
						NETLOG_DEBUG2,
						"             dnat leg  key %u 0x%8x\n",
				        rdr_ptr->legkey.hv,
						(uint32_t) rdr_ptr->legkey.redir_addr );
			trc_debug(	MFCE,
						NETLOG_DEBUG2,
						"             other leg key %u 0x%8x\n",
				        other_rdr_ptr->legkey.hv,
						(uint32_t) other_rdr_ptr->legkey.redir_addr );
	}

	if(relate_leg2_leg1) {
			trc_debug(	MFCE, 
						NETLOG_DEBUG2,
						"       :   hknife_dnat_relate() - leg_2_1 for hid %d, phid %d\n",
						peerResourceId,
						holeId );
			trc_debug(	MFCE,
						NETLOG_DEBUG2,
						"             dnat leg  key %u 0x%8x\n",
				        other_rdr_ptr->legkey.hv,
						(uint32_t) other_rdr_ptr->legkey.redir_addr );
			trc_debug(	MFCE,
						NETLOG_DEBUG2,
						"             other leg key %u 0x%8x\n",
				        rdr_ptr->legkey.hv,
						(uint32_t) rdr_ptr->legkey.redir_addr );
	}

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("Created Translation: (ANY -> "
			  	"%s:%d :: %s:%d -> %s:%d) - proto %s - %s\n",
			  	FormatIpAddress(orig_dst_ip, c_localaddr),
			  	orig_dst_port,
			  	FormatIpAddress(map_src_ip, c_localaddr2),
			  	map_src_port,
				FormatIpAddress(new_dst_ip, c_localaddr3),
			  	new_dst_port,
				c_protocol,
				rdr_type));
		NETDEBUG(MFCE, NETLOG_DEBUG4, ("key returned %u 0x%8x", rdr_ptr->legkey.hv, (uint32_t)rdr_ptr->legkey.redir_addr));

	return( 0 );
}

//
//	Function 	:
//		ModifyRedirect()
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
//      nop             -	if non-zero dont actually setup redirect
//
//		other_rdr_ptr	-	pointer to other ipf_nat_t * if it exists or NULL
//
//		other_hp	    -	pointer to other IpHole_t * if it exists or NULL
//
//		dstSym          - flag indicating that the leg of the call being
//		                  created will use data nat traversal. If value
//		                  is non-zero, data nat traversal will be used 
//		                  for the leg being created.
//
//		peerResourceId	-	holeid of other hole 
//
//	Purpose		:
//
//		This function is called to add a NAT redirect (rdr).
//		This operation is currently only implemented in HotKnife.
//
//	Return Value:
//		 0		on success
//		-1		on failure
//
static int
ModifyRedirect(	char *		iface_in,
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
				int32_t		mapflag,
				int			nop,
				ipf_nat_t *	other_rdr_ptr,
				IpHole_t *	other_hp,
				int			dstSym,
				uint32_t	peerResourceId )
{
	char *				c_protocol = "UDP";
	char				c_localaddr[32];
	char				c_localaddr2[32];
	char				c_localaddr3[32];
	IPNAT_T *			nat_rdr_ptr = (IPNAT_T *) &rdr_ptr->data;
	IPNAT_T *			other_nat_rdr_ptr = (IPNAT_T *) &other_rdr_ptr->data;
	uint32_t			nat_sym = 0;
    int             	relate_leg1_leg2 = 0, relate_leg2_leg1 = 0;

	// Are we ready to issue any needed dnat relate operations for a call if
	// needed ? This can only be done after we have a pair of unidirectional 
	// related media legs. Do we have a peer leg yet ?

	if (other_hp) {
		// Yes, This is the second leg. get information about the peer resourceId
		// - i.e. - the first leg

		nat_rdr_ptr->in_legkey = other_nat_rdr_ptr->in_legkey;

		// Does the other leg need a data nat operation ?

		if( other_hp->flags & DNAT_RELATE_PENDING ) {
			// Yes, indicate as much
			relate_leg2_leg1 = 1;
		}

		// Does this leg need a data nat operation ?

		if(dstSym)
			relate_leg1_leg2 = 1; // Yes, indicate as much
	}

	// Specify its nat type as NAT_REDIRECT

	nat_rdr_ptr->in_redir = NAT_REDIRECT;

	// Specify inbound interface

	strcpy(nat_rdr_ptr->in_ifname, iface_in);

	// Specify outbound interface

	strcpy(nat_rdr_ptr->in_ifname_out, iface_out);

	// Specify what destination ip should look like when coming into
	// the specified interface from the outside world inorder to
	// be redirected

	nat_rdr_ptr->in_outip = htonl(orig_dst_ip);
	nat_rdr_ptr->in_outmsk = -1;
	nat_rdr_ptr->in_p = (char)17; /* UDP */

	// Specify the port from which the packet should come

	nat_rdr_ptr->in_pmin = nat_rdr_ptr->in_pmax = htons(orig_dst_port);

	// Specify the ip to which the packet will be
	// redirected on the internal network

	nat_rdr_ptr->in_inip = htonl(new_dst_ip);
	nat_rdr_ptr->in_inmsk = -1;

	if (mapflag)
	{
		nat_rdr_ptr->in_map_srcip.s_addr = htonl(map_src_ip);
		nat_rdr_ptr->in_map_port = htons(map_src_port);
		nat_rdr_ptr->in_flags = IPN_XMAP;
	}

	// Specify the port to which the packet will be
	// redirected

	nat_rdr_ptr->in_pnext = htons(new_dst_port);

	// Specify the protocol

	nat_rdr_ptr->in_flags |= IPN_UDP;

	// specify data NAT symmetry

	nat_rdr_ptr->in_flags |= nat_sym;

	memset(rdr_ptr->rdr_string_1, (int32_t) 0, 128);
	memset(rdr_ptr->rdr_string_2, (int32_t) 0, 128);
	memset(rdr_ptr->rdr_string_3, (int32_t) 0, 128);

	if (mapflag)
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress(orig_dst_ip, c_localaddr), orig_dst_port);

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d",
				iface_out,
				FormatIpAddress(new_dst_ip, c_localaddr2), new_dst_port);

		sprintf(rdr_ptr->rdr_string_3,
				"    proto %s - xmap src %s, port %d",
				c_protocol,
				FormatIpAddress(map_src_ip, c_localaddr3), map_src_port);
	}
	else
	{
		sprintf(rdr_ptr->rdr_string_1,
				"rdr %s %s port %5d ->",
				iface_in,
				FormatIpAddress(orig_dst_ip, c_localaddr), orig_dst_port);

		sprintf(rdr_ptr->rdr_string_2,
				"    %s %s port %5d",
				iface_out,
				FormatIpAddress(new_dst_ip, c_localaddr2), new_dst_port);

		sprintf(rdr_ptr->rdr_string_3, "    proto %s", c_protocol);
	}

	if (nop) {
		return(0);
	}

		if ( mapflag ) {
			if( hknife_update_redirect( getPortnumFromIfacename(iface_in),
                                     orig_dst_ip,
                                     orig_dst_port,
                                     17,
                                     map_src_ip,
                                     map_src_port,
                                     new_dst_ip,
                                     new_dst_port,
									 getPortnumFromIfacename(iface_out)) != 0 ) {
				trc_error(	MFCE,
					"       : ERROR hknife_update_redirect() - (%s) hole %d\n",
					rdr_type,
					holeId );
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
					( "Error updating translation (%s)\n",
					rdr_type ));
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

			// relate the resources for discovery

			NETDEBUG(MFCE, NETLOG_DEBUG4,
					("UpdateRedirect(): relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1) );
			trc_debug(MFCE, NETLOG_DEBUG2,
					"       : relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1 );

			if(relate_leg1_leg2) {
				if ( hknife_dnat_relate(	rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr,
											other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("UpdateRedirect(): relatedn leg_1_2 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_1_2 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr  );
				}
			}

			if(relate_leg2_leg1) {
				if ( hknife_dnat_relate(	other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr,
											rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("UpdateRedirect(): relatedn leg_2_1 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_2_1 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr  );
				}
			}
		} else {
			if( hknife_update_redirect( getPortnumFromIfacename(iface_in),
                                     orig_dst_ip,
                                     orig_dst_port,
                                     17,
			                         0,
                                     0,
                                     new_dst_ip,
                                     new_dst_port,
									 getPortnumFromIfacename(iface_out) ) != 0 ) {
				trc_error(	MFCE,
					"       : ERROR hknife_update_redirect() - (%s) hole %d\n",
					rdr_type,
					holeId );
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
					( "Error updating translation (%s)\n",
					rdr_type ));
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

			// relate the resources for discovery

			NETDEBUG(MFCE, NETLOG_DEBUG4,
					("UpdateRedirect(): relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1) );
			trc_debug(MFCE, NETLOG_DEBUG2,
					"       : relate 1/2 %d relate 2/1 %d\n",
					 relate_leg1_leg2,
					 relate_leg2_leg1 );

			if(relate_leg1_leg2) {
				if ( hknife_dnat_relate(	rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr,
											other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("UpdateRedirect(): relatedn leg_1_2 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_1_2 failed hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr  );
				}
			}

			if(relate_leg2_leg1) {
				if ( hknife_dnat_relate(	other_rdr_ptr->legkey.hv,
											other_rdr_ptr->legkey.redir_addr,
											rdr_ptr->legkey.hv,
											rdr_ptr->legkey.redir_addr ) != 0 ) {
					NETERROR(MFCE, ("UpdateRedirect(): relatedn leg_2_1 failed - hole ids "
									"%d %d keys %u 0x%8x %u 0x%8x\n",
									holeId,
									peerResourceId,
									other_rdr_ptr->legkey.hv,
									(uint32_t) other_rdr_ptr->legkey.redir_addr,
									rdr_ptr->legkey.hv,
									(uint32_t) rdr_ptr->legkey.redir_addr ) );
					trc_error(	MFCE, 
								"       : ERROR hknife_dnat_relate() - leg_2_1 failed - hole ids "
								"%d %d keys %u 0x%8x %u 0x%8x\n",
								holeId,
								peerResourceId,
								other_rdr_ptr->legkey.hv,
								(uint32_t) other_rdr_ptr->legkey.redir_addr,
								rdr_ptr->legkey.hv,
								(uint32_t) rdr_ptr->legkey.redir_addr  );
				}
			}

	}

	rdr_ptr->rdr_defined = TRUE;

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       : updated - NAT redirect translation (%s) \n",
				rdr_type);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_1);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_2);

	trc_debug(	MFCE,
				NETLOG_DEBUG2,
				"       :   %s\n",
				rdr_ptr->rdr_string_3);
		trc_debug( MFCE,
                   NETLOG_DEBUG2,
				   "       :   existing legkey: hv %d, redir_addr 0x%08x\n",
				   rdr_ptr->legkey.hv, rdr_ptr->legkey.redir_addr );

	if(relate_leg1_leg2) {
		trc_debug(	MFCE, 
					NETLOG_DEBUG2,
					"       :   hknife_dnat_relate() - leg_1_2 for hid %d, phid %d\n",
					holeId,
					peerResourceId );
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"             dnat leg  key %u 0x%8x\n",
			        rdr_ptr->legkey.hv,
					(uint32_t) rdr_ptr->legkey.redir_addr );
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"             other leg key %u 0x%8x\n",
			        other_rdr_ptr->legkey.hv,
					(uint32_t) other_rdr_ptr->legkey.redir_addr );
	}

	if(relate_leg2_leg1) {
		trc_debug(	MFCE, 
					NETLOG_DEBUG2,
					"       :   hknife_dnat_relate() - leg_2_1 for hid %d, phid %d\n",
					peerResourceId,
					holeId );
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"             dnat leg  key %u 0x%8x\n",
			        other_rdr_ptr->legkey.hv,
					(uint32_t) other_rdr_ptr->legkey.redir_addr );
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"             other leg key %u 0x%8x\n",
			        rdr_ptr->legkey.hv,
					(uint32_t) rdr_ptr->legkey.redir_addr );
	}

	NETDEBUG(	MFCE,
			 	NETLOG_DEBUG4,
			 	("Updated Translation: (ANY -> "
			  	"%s:%d :: %s:%d -> %s:%d) - proto %s - %s\n",
			  	FormatIpAddress(orig_dst_ip, c_localaddr),
			  	orig_dst_port,
			  	FormatIpAddress(map_src_ip, c_localaddr2),
			  	map_src_port,
				FormatIpAddress(new_dst_ip, c_localaddr3),
			  	new_dst_port,
				c_protocol,
				rdr_type));
	NETDEBUG(MFCE, NETLOG_DEBUG4, ("existing key %u 0x%8x", rdr_ptr->legkey.hv, (uint32_t)rdr_ptr->legkey.redir_addr));

	return( 0 );
}

// 
// Function :
// 		AddPMHole()
// 
// Arguments :
//		interface	-	physical interface name associated wiht hole
// 
//		ip_addr		-	ipv4 destination ip address for incoming
//						signaling packets inbound to gis.
//						For DMR this will be the ip address of a
//						logical interface on the physical interface.
//						( host order )
// 
//		holeId		-	assigned holeId associated with hole
// 
//		bundleId	-	assigned bundleId associated with hole
// 
//		port		-	local port number being opened on lif
//						(host order)
// 
//		protocol	-	protocol type used for signaling traffic
//						through hole - IPPROTO_UDP or IPPROTO_TCP
// 
//		pmh_ptr		-	pointer to structure to be filled in as portmap
//						hole is constructed. When pmhole is removed,
//						structure is used to guide removal.
// 
// Purpose :
// 
//			This function is called to poke pinholes in the specified
//		network interface with the specified criterion. It puts out
//		logging messages and fills in a data structure for keeping
//		track of the hole so it can be removed.
// 
// Return Value:
//
//		 0 on success
//		-1 on failure
// 
static int
AddPMHole(	char *			interface,
		  	int32_t			ip_addr,
			int32_t			holeId,
			uint32_t		bundleId,
			uint16_t		port,
			int32_t			protocol,
			ipf_pmhole_t *	pmh_ptr )
{
	char *c_proto;
	char c_addr[32];
	int			selfid, rc;
	char 		cmd[512];
	char		msg[MAX_MSGLEN] = {0};

	if (protocol == IPPROTO_TCP)
		c_proto = "TCP";
	else if (protocol == IPPROTO_UDP)
		c_proto = "UDP";
	else
		c_proto = "Unknown";

	if (protocol == IPPROTO_TCP)
	{
		strcpy(pmh_ptr->data.ifname, interface);

		if (protocol == IPPROTO_TCP)
			pmh_ptr->data.proto = IPPROTO_TCP;
		else if (protocol == IPPROTO_UDP)
			pmh_ptr->data.proto = IPPROTO_UDP;

		pmh_ptr->data.lif_ip = htonl(ip_addr);
		pmh_ptr->data.port = ntohs(port);

		selfid = (	((mypid & 0xffff) << 16) | 
				 (pthread_self() & 0xffff) );

		FormatIpAddress(ip_addr, c_addr);

		snprintf(cmd, 512, "%s -A sigholes -p tcp --dst %s --dport %d -j ACCEPT", iptables_path, c_addr, port);

		if ((rc = sys_execd(	msgQueueId,
								selfid,
								SRVR_MSG_TYP,
								(1<<REQ_BIT)|(1<<OUT_BIT),
								cmd,
								msg,
								MAX_MSGLEN)) < 0)
		{
   			NETERROR(	MFCE,
						("AddPMHole : hole %d, unable to sys_execd "
						"(%d): %s\n\t%s\n",
						holeId,
						rc,
						cmd,
						msg));
			return (-1);					
		}
		else
		{
			trc_debug(	MFCE,
						NETLOG_DEBUG1,
						"       : adding local signaling hole\n");

			trc_debug(	MFCE,
					  	NETLOG_DEBUG1,
						"       :   %s:%d %s on %s\n",
						c_addr,
					  	port,
						((protocol == IPPROTO_UDP) ? "UDP" : "TCP"),
						interface);

			trc_debug(	MFCE,
						NETLOG_DEBUG1,
						"       :       holeid %d, bundleId %d\n",
						holeId,
						bundleId);
		}
	}
	else {
			trc_error(	MFCE,
					  	"AddPMHole : invalid protocol %s, "
						"hole : %d - SIGNALING\n",
						c_proto,
						holeId);

			return(-1);
	}

	return( 0 );
}

//
//	Function 	:
//		RemovePMHole()
//
//	Arguments	:
//
//		holeId			-	holeId for which rule is associated.
//
//		bundleId		-	bundleId associated with hole
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
RemovePMHole(	int32_t			holeId,
				uint32_t		bundleId,
				ipf_pmhole_t *	pmh_ptr )
{
	char *		c_proto = "";
	uint16_t	port;
	char		c_addr[32];
	int			selfid, rc;
	char 		cmd[512];
	char		msg[MAX_MSGLEN] = {0};

	if (pmh_ptr->data.proto == (uint8_t) IPPROTO_TCP)
		c_proto = "TCP";
	else if (pmh_ptr->data.proto == (uint8_t) IPPROTO_UDP)
		c_proto = "UDP";

		selfid = (	((mypid & 0xffff) << 16) | 
				 (pthread_self() & 0xffff) );

	FormatIpAddress( ntohl(pmh_ptr->data.lif_ip), c_addr);
	port = ntohs(pmh_ptr->data.port),

	snprintf(cmd, 512, "%s -D sigholes -p tcp --dst %s --dport %d -j ACCEPT", iptables_path, c_addr, port);

	if ((rc = sys_execd(	msgQueueId,
							selfid,
							SRVR_MSG_TYP,
							(1<<REQ_BIT)|(1<<OUT_BIT),
							cmd,
							msg,
							MAX_MSGLEN)) < 0)
	{
   		NETERROR(	MFCE,
					("RemovePMHole : hole %d, unable to sys_execd "
					"(%d): %s\n\t%s\n",
					holeId,
					rc,
					cmd,
					msg));
		return (-1);					
	}
	else
	{
		trc_debug(	MFCE,
					NETLOG_DEBUG2,
					"       : removing local signaling hole\n");

		trc_debug(	MFCE,
				  	NETLOG_DEBUG2,
				  	"       :   %s:%d %s\n",
					FormatIpAddress(
						ntohl(pmh_ptr->data.lif_ip),
						c_addr),
					ntohs(pmh_ptr->data.port),
					c_proto );

		trc_debug(	MFCE,
				  	NETLOG_DEBUG2,
				  	"       :                 holeid %d, bundleId %d\n",
					holeId,
					bundleId );
	}

	return( 0 );
}

// *
// * Hash table functions for lookup of IpHole_t's by
// * either bundleId or holeId
// *

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
	int32_t buckets = 1;

	if (lsMem->maxCalls <= 0)
	{
		buckets = 1024;
	}
	else
	{
		while ((buckets < 16384) && (buckets < lsMem->maxCalls))
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
commonHash(void *key)
{
	int *val = (int *) key;

	if (HoleIdCache->_numBuckets == 1)
		return (0);
	else
		return (*val % HoleIdCache->_numBuckets);
	// Note : HoleIdCache->_numBuckets == bundleIdCache->_numBuckets

}


static int
commonKeyCmp(void *key1, void *key2)
{
	int *val1 = (int *) key1;
	int *val2 = (int *) key2;

	return (*val1 - *val2);
}


static void *
holeIdData2Key(void *entry)
{
	return( &((IpHole_t *) entry)->assignedId );
}


static void *
bundleIdData2Key(void *entry)
{
	return( &((IpHole_t *) entry)->bundleId );
}


//
//	Function    :
//		InitCaches()
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
InitCaches(void)
{
	extern void		CacheSetName(cache_t cache, char *name);
	int32_t 		BucketCount = 2 * getHashtableBucketCount();
	char *			fn = "InitCaches(): ";

	// initialize holeId cache

	HoleIdCache = CacheCreate(CACHE_MALLOC_LOCAL);
	HoleIdCache->dt = CACHE_DT_HASH;
	HoleIdCache->_numBuckets = BucketCount;
	HoleIdCache->_hashlistOffset = HOLEID_LIST_OFFSET;
	HoleIdCache->_hash = (int) commonHash;
	HoleIdCache->_entry2key = (int) holeIdData2Key;
	HoleIdCache->_keycmp = (int) commonKeyCmp;
	HoleIdCache->lock = NULL;

	CacheSetName(HoleIdCache, "NSF holeId");

	if (!CacheInstantiate(HoleIdCache))
	{
		NETERROR(	MFCE,
					("%s Cannot initialize HoleId cache\n",
					fn));
		return( -1 );
	}

	// initialize holeId cache

	BundleIdCache = CacheCreate(CACHE_MALLOC_LOCAL);
	BundleIdCache->dt = CACHE_DT_HASH;
	BundleIdCache->_numBuckets = BucketCount;
	BundleIdCache->_hashlistOffset = BUNDLEID_LIST_OFFSET;
	BundleIdCache->_hash = (int) commonHash;
	BundleIdCache->_entry2key = (int) bundleIdData2Key;
	BundleIdCache->_keycmp = (int) commonKeyCmp;
	BundleIdCache->lock = NULL;

	CacheSetName(BundleIdCache, "NSF bundleId");

	if (!CacheInstantiate(BundleIdCache))
	{
		NETERROR(	MFCE,
					("%s Cannot initialize BundleId cache\n",
					fn));
		return( -1 );
	}

	return( 0 );
}

// newHoleId() creates a new HoleId. 
// HoleId is always greater  than 0. 
uint32_t
newHoleId(void)
{
	if (lastAssignedId == -1) 
		lastAssignedId = 1;
	else
		lastAssignedId++;
	
	return lastAssignedId; 
}
