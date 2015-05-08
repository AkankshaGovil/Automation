#include "lsconfig.h"

// Configuration variables
int		max_segs 			= CDEF_Val(max_segs);
int		max_segsize 		= CDEF_Val(max_segsize);
int		segaddrtype 		= CDEF_Val(segaddrtype);
int 	segowner 			= CDEF_Val(segowner);
int 	realTimeEnable 		= CDEF_Val(realTimeEnable);

char	gkid[GKID_LEN] 		= CDEF_Val(gkid);
char	sipservername[SIPURL_LEN] 
							= CDEF_Val(sipservername);
char	mswname[MSWNAME_LEN] = CDEF_Val(mswname);

int		doScm				= CDEF_Val(doScm);

unsigned int rrqttl 		= CDEF_Val(rrqttl);
unsigned int routeH245 		= CDEF_Val(routeH245);

int		forceh245 			= CDEF_Val(forceh245);

int		rrqtimer 			= CDEF_Val(rrqtimer);
int		updateAllocations 	= CDEF_Val(updateAllocations);
int		routecall 			= CDEF_Val(routecall);
int		recordroute 		= CDEF_Val(recordroute);
int 	sharecall 			= CDEF_Val(sharecall);
int 	defaultrealm 		= CDEF_Val(defaultrealm);
char	sipdomain[SIPDOMAIN_LEN] 
							= CDEF_Val(sipdomain);
int		sipauth 			= CDEF_Val(sipauth);
char	sipauthpassword[SIPAUTHPASSWORD_LEN] 
							= CDEF_Val(sipauthpassword);
int		sipservertype 		= CDEF_Val(sipservertype);
int		sipmaxforwards		= CDEF_Val(sipmaxforwards);

char	rad_server_addr[RADSERVERADDR_LEN] 
							= CDEF_Val(rad_server_addr);
char	secret[SECRET_LEN] 	= CDEF_Val(secret);

char	enumdomain[ENUMDOMAIN_LEN] 
							= CDEF_Val(enumdomain);

char    jsLogLevel[128] 	= CDEF_Val(jsLogLevel);
char 	jsLogFile[512] 		= CDEF_Val(jsLogFile);
char 	readPass[20] 		= CDEF_Val(readPass);
char 	writePass[20] 		= CDEF_Val(writePass);

char 	cdrdirname[CDRDIRNAMELEN] 
							= CDEF_Val(cdrdirname);
int		cdrtype 			= CDEF_Val(cdrtype);
int		cdrformat 			= CDEF_Val(cdrformat);
int		cdrtimer 			= CDEF_Val(cdrtimer);

char 	ctrdirname[CDRDIRNAMELEN] = CDEF_Val(ctrdirname);
int		ctrtype = CDEF_Val(ctrtype);
int		ctrformat = CDEF_Val(ctrformat);
int		ctrtimer = CDEF_Val(ctrtimer);

int 	moduleid;
int 	h323AdminStatus 	= CDEF_Val(h323AdminStatus),
		sipAdminStatus 		= CDEF_Val(sipAdminStatus);

int		doFastStart 		= CDEF_Val(doFastStart);
int		uccProto 			= CDEF_Val(uccProto);
int		billingType 		= CDEF_Val(billingType);
char	first_auth_username[128] 
							= CDEF_Val(first_auth_username);
char	first_auth_password[128] 
							= CDEF_Val(first_auth_password);
char	second_auth_username[128] 
							= CDEF_Val(second_auth_username);
char	second_auth_password[128] 
							= CDEF_Val(second_auth_password);
int		rolloverTime 		= CDEF_Val(rolloverTime);

int 	g711Ulaw64Duration 	= CDEF_Val(g711Ulaw64Duration);
int 	g711Alaw64Duration 	= CDEF_Val(g711Alaw64Duration);
int 	g729Frames 			= CDEF_Val(g729Frames);
int 	g7231Frames 		= CDEF_Val(g7231Frames);
CodecType	defaultCodec	= CDEF_Val(defaultCodec);

int		defaultcodec		= CDEF_Val(defaultcodec);

/* Maximum Calls Per Second for H323 */
int  	h323Cps				= CDEF_Val(h323Cps);

char 	e911loc[128] 		= CDEF_Val(e911loc);
char 	tapcall[128] 		= CDEF_Val(tapcall);
char 	untapcall[128] 		= CDEF_Val(untapcall);
int 	useXConnId 			= CDEF_Val(useXConnId);

char	fceConfigFwName[128] 
							= CDEF_Val(fceConfigFwName);
#if FCE_REMOVED
int 	fceDefaultPublic 	= CDEF_Val(fceDefaultPublic);
unsigned int fceConfigOurIpAddr 
							= CDEF_Val(fceConfigOurIpAddr);
int 	fceH245PinholeEnabled 	
							= CDEF_Val(fceH245PinholeEnabled);
int 	defaultPublicMediaRouting = CDEF_Val(defaultPublicMediaRouting);
int 	defaultPrivateMediaRouting = CDEF_Val(defaultPrivateMediaRouting);
int 	defaultHideAddressChange 
							= CDEF_Val(defaultHideAddressChange);
#endif

int 	allowDestAll 		= CDEF_Val(allowDestAll);
int     allowDestArq            = CDEF_Val(allowDestArq);
int 	allowSrcAll 		= CDEF_Val(allowSrcAll);
int		forwardSrcAddr 		= CDEF_Val(forwardSrcAddr);
int		cpRoutingPolicy;
int		nprocs 				= CDEF_Val(nprocs);
int		routeDebug;
int		cacheTimeout;
int		allowHairPin 		= CDEF_Val(allowHairPin);

// Out Bound Proxy Config
int		obpEnabled 			= CDEF_Val(obpEnabled);
int		allowInternalCalling 
							= CDEF_Val(allowInternalCalling);
int		allowDynamicEndpoints 
							= CDEF_Val(allowDynamicEndpoints);

unsigned long gisrip;

// thread defaults			// gk just has one thread
int 	nBridgeThreads 		= CDEF_Val(nBridgeThreads);
int 	nCallIdThreads 		= CDEF_Val(nCallIdThreads);
int 	nConfIdThreads 		= CDEF_Val(nConfIdThreads);
int 	xthreads 			= CDEF_Val(xthreads);
int 	nRadiusThreads 		= CDEF_Val(nRadiusThreads);
int 	nh323Instances;
int 	nh323CfgInstances 	= CDEF_Val(nh323CfgInstances);
int 	h323maxCalls 		= CDEF_Val(h323maxCalls);
int 	h323maxCallsSgk 	= CDEF_Val(h323maxCallsSgk);
int 	h323infoTransCap 		= CDEF_Val(h323infoTransCap);

int 	maxHunts 			= CDEF_Val(maxHunts);
int 	iservercrId 		= CDEF_Val(iservercrId);
int 	crids 				= CDEF_Val(crids);

/* usecs */
int 	siptimerT1 			= CDEF_Val(siptimerT1),
		siptimerT2 			= CDEF_Val(siptimerT2);
int 	sipMaxInvRqtRetran 	= CDEF_Val(sipMaxInvRqtRetran);

int		hello_interval 		= CDEF_Val(hello_interval);
char	hello_mcast_addr[80] 
							= CDEF_Val(hello_mcast_addr);
char	hello_port[80] 		= CDEF_Val(hello_port);
unsigned short	hello_group 	
							= CDEF_Val(hello_group);
unsigned short	hello_priority 	
							= CDEF_Val(hello_priority);
char	hello_ifname[80] 	= CDEF_Val(hello_ifname);
int		hello_dead_factor 	= CDEF_Val(hello_dead_factor);

char	rs_ifname[RS_LINELEN];
char	rs_mcast_addr[RS_LINELEN]; 
char	rs_port[RS_LINELEN];
char	rs_tmp_dir[RS_LINELEN];
char	rs_cp_cmd_str[RS_LINELEN];

int		rs_host_prio 		= CDEF_Val(rs_host_prio);
int		rs_ssn_int 			= CDEF_Val(rs_ssn_int);

int		max_call_duration	= CDEF_Val(max_call_duration);


// Other Variable declarations and initializations
char ModuleNames[MNETLOGMAX][20] = {
       "moddef",
       "modreg",
       "modfind",
       "modage",
       "modcache",
       "modinit",
       "modsel",
       "modpkt",
       "moddb",
       "modshm",
       "modcdr",
       "modfaxp",
       "modconn",
       "modtmr",
       "modredund",
       "modxml",
       "modcli",
       "modlmgr",
       "modpmgr",
       "modh323",
       "modlrq",
       "modrrq",
       "modarq",
       "modsip",
       "modq931",
       "modscc",
       "modiwf",
       "modbridge",
       "modfce",
       "modradc",
       "modispd",
       "modrsd",
       "moddlic",
 };

char  CustomLogs[CUSLOGMAX][64] =
{ 
  "TPKTCHAN",
  "UDPCHAN",
  "PERERR",
  "CM",
  "CMAPICB",
  "CMAPI"
  "CMERR",
  "LI",
  "LIINFO"
};

serplex_config *serplexes = 0, *server;
int 	inserver = 0; /* running index on servers */
int		nopolicy = 0;
extern 	char *sconfigtext;

int		(*_msAdd)(char *) = NULL, (*_msDelete)(char *) = NULL, (*_msSetDebugLevel)(int) = NULL, (*_msDeleteAll)(void) = NULL;
int 	(*_sipSetTraceLevel)(int) = NULL;
int  	h323QLen = 1000;

//  cluster id to identify the cluster
char	cluster_id[256] = CDEF_Val(cluster_id);

ispd_server_type_t		ispd_type;		// ispd server type :
										// "active", "standby" or "disabled"

ispd_interface_t		ispd_primary;	// definition of primary lan interface
ispd_interface_t		ispd_secondary;	// definition of secondary lan interface
ispd_ctl_interface_t	ispd_ctl;		// definition of control lan interface


int 	max_servers 		= CONFIG_SERPLEX_MAX;
/* Start RSD Related stuff */

	/* Some flags which are used for config file generation */
int				rs_tmp_dir_fl = 0;
int				rs_cp_cmd_str_fl = 0;
int				rs_ssn_int_fl = 0;

int				RSDConfig = 0;

/* End RSD Related stuff */

/* Start ExecD Related stuff */
int				ExecDConfig = CONFIG_LOCATION_NONE;
/* End ExecD Related stuff */

int 		bridgeH323QSize = 1000;

int 		nH323Threads 		= 1;		// no of threads for H.323

/* nsecs */
longlong_t 	class1deadline = 50000000, class2deadline = 200000000;

List 		altGkList = NULL;
int		compactVersion = 0;
int		trackVports = 0, trackMaxCalls = 0;	
