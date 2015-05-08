%{ 
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ipc.h"
#include "key.h"
#include "srvrlog.h"
#include "lsconfig.h"
#include "age.h"
#include "server.h"
#include "cdr.h"
#include "codecs.h"
#include "ispd_common.h"
#include "rsd_common.h"
#include "hello_common.h"
#include "serverdb.h"
#include "calldefs.h"
#include "shm.h"
#include "shmapp.h"
#include "codemap.h"
#include "nxosd.h"
#include "sconfig.h"


#include "serverp.h"

#define TRUE 1
#define FALSE 0

int sconfiglex(void);
// Variable declarations and initializations
static int InitServers();
static int InitServer(serplex_config *server, int type);

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
       "modirq",
       "modicmp",
	   "modexecd",
	   "modscm",
	   "modscmrpc"
 };

char  CustomLogs[CUSLOGMAX][64] =
{ 
  "TPKTCHAN",
  "UDPCHAN",
  "PERERR",
  "CM",
  "CMAPICB",
  "CMAPI",
  "CMERR",
  "LI",
  "LIINFO"
};

serplex_config *serplexes = 0, *server;

int 	max_servers = CONFIG_SERPLEX_MAX;
int 	max_endpts = 5000;
int		max_segs = 5;
int		max_segsize = 0x100000;
int		segaddrtype = 0;
void            *segaddr = NULL;
int 	segowner = CONFIG_SERPLEX_JSERVER;
int 	max_gws = 50;
int		max_ccalls = 100;
int 	realTimeEnable = 0;

char	gkid[GKID_LEN] = { 0 };
char	sipservername[SIPURL_LEN] = {0};
int	sipport = 5060;
char	mswname[MSWNAME_LEN] = {0};

int		doScm = 0;

unsigned int		rrqttl = RRQTTL_DEFAULT;
unsigned int		callttl = CALLTTL_DEFAULT;
unsigned int		routeH245 = H245_DEFAULT;

int		forceh245 = 0;

int		rrqtimer = 30;
int		updateAllocations = 1;
int		routecall = 1;
int		recordroute = 0;
int 	sharecall = 0;
int 	inserver = 0; /* running index on servers */
int		nopolicy = 0;
int		nocallstate = 0;
char	sipdomain[SIPDOMAIN_LEN] = { 0 };
int		sipauth = 0;
char	sipauthpassword[SIPAUTHPASSWORD_LEN] = { 0 };
int		sipservertype;
int		sipmaxforwards = 70;

int		siphold3264 = 0;

char	rad_server_addr[MAX_NUM_RAD_ENTRIES][RADSERVERADDR_LEN] = { 0, 0 };
char	secret[MAX_NUM_RAD_ENTRIES][SECRET_LEN] = { 0, 0 };
int		rad_timeout = 5;
int		rad_retries = 4;
int		rad_deadtime = 0;
int		rad_acct = FALSE;
int		rad_acct_session_id_overloaded = FALSE;
int		rad_failed_timeout = 60;
char 	rad_dirname[CDRDIRNAMELEN] = ".";

char	prefix[PHONE_NUM_LEN] = { 0 };
char	enumdomain[ENUMDOMAIN_LEN] = { 0 };
char	fax_dir_pathname[128] = "/tmp/faxdir";
char	fax_user[20] = "faxuser";
char	fax_password[20] = "";

char    jsLogLevel[128] = "normal";
char 	jsLogFile[512] = "/tmp/jserverlogfile";
char 	readPass[20] = "";
char 	writePass[20] = "";
int     jsCompression = 0;

char 	cdrdirname[CDRDIRNAMELEN] = ".";
int		cdrtype = CDRMINDCTIFIXED;
int		cdrformat = CDRFORMAT_MIND;
int		cdrtimer = 60;

char 	ctrdirname[CDRDIRNAMELEN] = ".";
int		ctrtype = CDRMINDCTIFIXED;
int		ctrformat = CDRFORMAT_MIND;
int		ctrtimer = 60;

unsigned int	cdrevents = CDREND1;
int 	moduleid;
int 	h323AdminStatus = 1, sipAdminStatus = 1;

int 	clwp = 15;

int		doFastStart = 1;
int		localProceeding = 0;
int		h245Tunneling = 1;
int		always2833 = 0;
int		mapisdncc = 0;
int		maplrjreason = 0;
int		fsInConnect=0;
int		getAniFromAcf=0;
int		uccProto = CPROTO_UCC;
int		billingType = BILLING_POSTPAID;
char	first_auth_username[128] = "";
char	first_auth_password[128] = "";
char	second_auth_username[128] = "";
char	second_auth_password[128] = "";
int		rolloverTime = 20;
int		recoveryOnTimerExpiry=0;
int		localReInviteNoSdp = 0;

/* this array stores the cause codes that are configurable.
** they are stored as (Call error code, H323 code, SIP code) and initialized to their
** default values
*/

int		causeCodes[][3] = {
	SCC_errorNone, -1, 500,						// no-error
	SCC_errorBusy, Cause_eUserBusy, 486,				// busy
	SCC_errorAbandoned, Cause_eNormalCallClearing, 500,		// abandoned
	SCC_errorInvalidPhone, Cause_eInvalidNumberFormat, 484,		// invalid-phone
	SCC_errorBlockedUser, Cause_eCallRejected, 403,			// user-blocked
	SCC_errorNetwork, Cause_eNoCircuitAvailable, 503,		// network-error
	SCC_errorNoRoute, Cause_eNoCircuitAvailable, 404,		// no-route
	SCC_errorNoPorts, Cause_eNoCircuitAvailable, 503,		// no-ports
	SCC_errorGeneral, Cause_eNormalCallClearing, 503,		// general-error
	SCC_errorMaxCallDuration, 0, 500,				// max-call-duration
	SCC_errorResourceUnavailable, Cause_eNoResource, 480,		// resource-unavailable
	SCC_errorDestinationUnreachable, Cause_eDestinationOutOfOrder, 480,	// dest-unreach
	SCC_errorUndefinedReason, Cause_eNormalUnspecified, 503,	// undefined
	SCC_errorInadequateBandwidth, Cause_eNoCircuitAvailable, 503,	// no-bandwidth
	SCC_errorH245Incomplete, Cause_eInterworking, 503,		// h245-error
	SCC_errorIncompleteAddress, Cause_eInvalidNumberFormat, 484,	// incomp-addr
	SCC_errorLocalDisconnect, Cause_eNormalCallClearing, 503,		// local-disconnect
	SCC_errorH323Internal, Cause_eTemporaryFailure, 503,		// h323-internal
	SCC_errorH323Protocol, Cause_eTemporaryFailure, 503,		// h323-proto
	SCC_errorNoCallHandle, Cause_eTemporaryFailure, 481,		// no-call-handle
	SCC_errorGatewayResourceUnavailable, Cause_eNoResource, 503,	// gw-resource-unavailable
	SCC_errorFCECallSetup, Cause_eNoCircuitAvailable, 500,		// fce-error-setup
	SCC_errorFCE, Cause_eNoCircuitAvailable, 500,			// fce-error
	SCC_errorNoVports, Cause_eNoCircuitAvailable, 503,		// no-vports
	SCC_errorHairPin, Cause_eInvalidTransit, 500,			// hairpin
	SCC_errorShutdown, Cause_eNormalUnspecified, 500,		// shutdown
	SCC_errorDisconnectUnreachable, Cause_eRecoveryOnExpiresTimeout, 480,	// disconnect-ureach
	SCC_errorTemporarilyUnavailable, Cause_eUserBusy, 480,		// temporarily-unavailable
	SCC_errorSwitchover, Cause_eNormalUnspecified, 500,		// switchover
	SCC_errorDestRelComp, Cause_eNormalUnspecified, 500,		// dest-rel-comp
	SCC_errorNoFCEVports, Cause_eNoCircuitAvailable, 500,		// fce-no-vports
	SCC_errorH323MaxCalls, Cause_eNoCircuitAvailable, 503,		// h323-maxcalls
	SCC_errorMswInvalidEpId, Cause_eNormalUnspecified, 503,		// msw-invalid-epid
	SCC_errorMswRouteCallToGk, Cause_eNormalUnspecified, 503,		// msw-routecallgk
	SCC_errorMswCallerNotRegistered, Cause_eNormalUnspecified, 503,	// msw-notreg
	SCC_errorDestBlockedUser, Cause_eCallRejected, 403,		// user-blocked-at-dest
	SCC_errorDestNoRoute, Cause_eNoRouteDest, 404,			// no-route-at-dest
	SCC_errorDestTimeout, Cause_eSwitchingEquipmentCongestion, 503,	// dest-timeout
	SCC_errorDestGone, Cause_eNumberChanged, 500,			// dest-gone
	SCC_errorRejectRoute, Cause_eNoCircuitAvailable, 404,		// reject-route
	SCC_errorSipRedirect, Cause_eNormalUnspecified, 500,		// sip-redirect
	SCC_errorSipAuthRequired, Cause_eNormalUnspecified, 401,	// sip-auth-req
	SCC_errorSipForbidden, Cause_eNormalUnspecified, 403,		// sip-auth-req
	SCC_errorSipProxyAuthRequired, Cause_eNormalUnspecified, 407,	// sip-proxy-auth-req
	SCC_errorSipInternalError, Cause_eNormalUnspecified, 500,	// sip-int-error
	SCC_errorSipNotImplemented, Cause_eNormalUnspecified, 501,	// sip-not-impl
	SCC_errorSipServiceUnavailable, Cause_eNormalUnspecified, 503,	// sip-service-unavailable
	SCC_errorNoNATTLicense, Cause_eNoCircuitAvailable, 500,		// no-nat-t-license
};

/* use binary search to get the array index corresponding to the internal error code */
int causeCodeLkupArrayMax = sizeof(causeCodes)/(3*sizeof(int));
int getCauseCodeIndex(int callErrorCode)
{
	int top = causeCodeLkupArrayMax - 1;
	int bottom = 0;
	int mid;

	while(top >= bottom)
	{
		mid = (top + bottom)/2;
		if(causeCodes[mid][0] == callErrorCode)
			return mid;

		if(callErrorCode < causeCodes[mid][0])
			top = mid - 1;
		else
			bottom = mid + 1;
	}
	return -1;
}
/* get the SIP error code corresponding to the internal error code */
int getSipCode(int callErrorCode)
{
	int index = getCauseCodeIndex(callErrorCode);
	if(index >= 0)
	{
		return causeCodes[index][2];
	}
	return 500;
}
/* get the H323 error code corresponding to the internal error code */
int getH323Code(int callErrorCode)
{
	int index = getCauseCodeIndex(callErrorCode);
	if(index >= 0)
	{
		return causeCodes[index][1];
	}
	return Cause_eNormalUnspecified;
}
/* set the SIP error code using the string to lookup the internal error code */
void setSipCode(char *errStr, int value)
{
	int internalErrCode = errorStr2CallError(errStr);
	if(internalErrCode > 0)
	{
		int index = getCauseCodeIndex(internalErrCode);
		if(index > 0)
		{
			causeCodes[index][2] = value;
		}
	}
}

/* set the H323 error code using the string to lookup the internal error code */
void setH323Code(char *errStr, int value)
{
	int internalErrCode = errorStr2CallError(errStr);
	if(internalErrCode > 0)
	{
		int index = getCauseCodeIndex(internalErrCode);
		if(index > 0)
		{
			causeCodes[index][1] = value;
		}
	}
}

extern 	char *sconfigtext;

int		(*_msAdd)(char *) = NULL, (*_msDelete)(char *) = NULL, (*_msSetDebugLevel)(int) = NULL, (*_msDeleteAll)(void) = NULL;
int 	(*_sipSetTraceLevel)(int) = NULL;

int 	g711Ulaw64Duration = 20;
int 	g711Alaw64Duration = 20;
int 	g729Frames = 2;
int 	g7231Frames = 1;
CodecType	defaultCodec = CodecGPCMU;

/* Maximum Calls Per Second for H323 */
int  	h323Cps	= 200;
int  	h323QLen = 1000;

char 	e911loc[128] = { 0 };
char 	tapcall[128] = { 0 };
char 	untapcall[128] = { 0 };
int 	useXConnId = 0;

char	fceConfigFwName[128] = {0};
unsigned int  fceFirewallAddresses[10];
int     fceFirewallNumber = 0;
#if FCE_REMOVED
int 	fceDefaultPublic = TRUE;
int fceH245PinholeEnabled = FALSE;
unsigned int fceConfigOurIpAddr = 0;
int defaultMediaRouting = 1;
int defaultHideAddressChange = 1;
#endif


ispd_server_type_t		ispd_type;		// ispd server type :
										// "active", "standby" or "disabled"

ispd_interface_t		ispd_primary;	// definition of primary lan interface
ispd_interface_t		ispd_secondary;	// definition of secondary lan interface
ispd_ctl_interface_t	ispd_ctl;		// definition of control lan interface

/* Start RSD Related stuff */
char			rs_ifname[RS_LINELEN];
char			rs_mcast_addr[RS_LINELEN]; 
char			rs_port[RS_LINELEN];
char			rs_tmp_dir[RS_LINELEN];
char			rs_cp_cmd_str[RS_LINELEN];
int				rs_host_prio = RS_DEF_HOST_PRIO;
int				rs_ssn_int = RS_SEND_SEQNUM_INT;		/* Interval at which seq num
														is broadcasted by the master */
int 			histdb_size = CLI_MAX_HIST;

	/* Some flags which are used for config file generation */
int				rs_tmp_dir_fl = 0;
int				rs_cp_cmd_str_fl = 0;
int				rs_ssn_int_fl = 0;

int				RSDConfig = 0;

int				hello_interval = HELLO_INTERVAL;
char			hello_mcast_addr[80] = HELLO_MCAST_ADDR;
char			hello_port[80] = HELLO_PORT_STR;
unsigned short	hello_group = HELLO_DEFAULT_GROUP;
unsigned short	hello_priority = HELLO_DEFAULT_PRIORITY;
char			hello_ifname[80] = HELLO_IFNAME;
int				hello_dead_factor = DEAD_FACTOR;

/* End RSD Related stuff */

/* Start ExecD Related Stuff */
int 			ExecDConfig = CONFIG_LOCATION_LOCAL;
/* End ExecD Related Stuff */

int 		bridgeH323QSize = 1000;

int 		allowDestAll = 0;
int 		allowDestArq = 0;
int 		allowAuthArq = 0;
int 		allowSrcAll = 0;
int			forwardSrcAddr = 0;
int			cpRoutingPolicy = CP_ROUTE_UTILZ;
int			nprocs = 2;
int			routeDebug = 0;
int			cacheTimeout;
int			allowHairPin = 0;
int			allowRtpAll = 1;

// Out Bound Proxy Config
int			obpEnabled = 0;
int			allowInternalCalling = 0;
int			allowDynamicEndpoints = 0;
int			enableNatDetection = 0;

unsigned long gisrip = 0;

int 		logCdr = 0;
int 		logIntCdr = 0;

// thread defaults			// gk just has one thread
int 		nBridgeThreads = 10;	// no of threads in bridge
int 		nCallIdThreads = 2;		// no of threads in sip ua
int 		nConfIdThreads = 1;		// no of threads in iwf
int 		nH323Threads = 1;		// no of threads for H.323
int 		xthreads = 2;			// no of threads in sip tsm
int 		nRadiusThreads = 10;	// no of threads in radius client
int 		nh323Instances = 1;		// no of instances for h.323
int 		nh323CfgInstances = 1;	// no of instances for h.323 - configured
int 		h323maxCalls = 200;		// default maxCalls
int 		h323maxCallsSgk = 100;	// default maxCalls for sgk
int 		h323RasMaxBuffSize = 2048;		// default h323RasMaxBuffSize
int			h323infoTransCap = INFO_TRANSCAP_PASS;	//default infoTransCap for h.323
int 		h323maxCallsPadFixed = H323MAXCALLSPADF;
int			h323maxCallsPadVariable = H323MAXCALLSPADV;
int			h323RemoveTcs2833 = 0;
int			h323RemoveTcsT38 = 0;
int 		maxHunts = 1;
int 		iservercrId = 1;
int 		crids = 0;

#define		MAX_MAX_CALL_DURATION 31536000		// 1 year in seconds
int			max_call_duration = 0;
int			max_hunt_allowable_duration = 0;

/* usecs */
int 		siptimerT1 = 500000, siptimerT2 = 4000000;
int 		sipMaxInvRqtRetran = 7;

// seconds
int			siptimerC = 180;

int			cdrcallidlen = 64;

/* nsecs */
longlong_t 	class1deadline = 50000000, class2deadline = 200000000;
int     trackVports = 0, trackMaxCalls = 0;

List 		altGkList = NULL;

int			h323GracefulShutdown = 0;	// Send Unregistrations at shutdown

int			dns_recovery_timeout = 120;

int			nlruRoutes = 500;

int		sipminSE = 600;
int		sipsessionexpiry = 3600;

int			allowANIPortSel = 0, allowDNISPortSel = 0;

int			huntAllCauseCodes = 0;

char		codemapfilename[256] = CODEMAP_SHORTFILE;
char		codemaptemplate[128] = CODEMAP_SHORT;

char		mgmtInterfaceIp[128];

#define CONFIG_ASSERT(x)	if (!(x)) {sconfigerror("check failed"); YYERROR;}

%}

// define semantics
%union
{
	void				*ptr;
	unsigned int 		val;
	int					sval;
	struct sockaddr_in 	ipaddr;
	char 				*str;	// will be a pointer to yytext
	List				list;
}

%token ENABLE TMOUT PORT FORCE
%token SERVERS ENDPTS GWS GIS LUS VPNS BCS JSERVER FAXS NONE LOCAL DBTOKEN CLI
%token AGE PM FCE ISPD SCM MGMT_INTERFACE
%token SEGS SEGSIZE DAEMONIZE SEGOWNER THREADS LWPS PROCS STACK IWF BRIDGE TSM UA
%token FAXDIR FAXUSER FAXPASSWORD MAXRETRANSCOUNT
%token DDEBUG DLOG DTERMINAL DSYSLOG PRIO DLOGFILE DLOGLEVEL
%token HDEBUG LEVEL NO HLOG
%token PERMISSION_READ PERMISSION_WRITE
%token POLICY CALLSTATE NONE GKID RRQTTL CALLTTL H245 CALL ROUTE SHARE
%token SIPDOMAIN SEGADDR SEGADDRTYPE DYNAMIC FIXED
%token SIPAUTH SIPAUTHPASSWORD
%token SIPSERVER 
%token MSWNAME 
%token SIPHOLD3264 
%token SIPPORT 
%token CDR DIR MINDCTI FIXED DAILY SEQ TIME CDRTIMER SLEEP
%token SIPDOMAIN SDEBUG SLOG
%token FASTSTART ON OFF UCC PREFIX ALWAYS2833 MAPISDNCC PROCEEDING TUNNELING GETANIFROMACF 
%token MAPLRJREASON
%token FSINCONNECT
%token ROLLOVERTIME ENUMDOMAIN H323CPS H323QLEN
%token BILLING RECORDROUTE
%token FIRST_AUTH SECOND_AUTH
%token CODEC G711ULAW64 G711ALAW64 G7231 G729
%token FWNAME FWIPADDR FWPRIVATE FWPUBLIC DEFAULT FWCONNECTADDR
%token MGMT_IP
%token UPDATE ALLOCATIONS E911LOC TAPCALL UNTAPCALL
%token RRQTIMER XCONNID IPPROTO XML SIPTIMER ALTGK GK T1 T2 C
%token H323QSIZE REALTIME ALLOW DEST DESTARQ AUTHARQ ALL SRC CP INSTANCE LRU UTILZ
%token RADIUS RADIUS_SERVER SHARED_SECRET RADIUS_ACCT ACCT_SESSION_ID OVERLOADED
%token RADIUS_TIMEOUT RADIUS_RETRIES RADIUS_DEADTIME
%token HAIRPIN
%token OBP INTERNAL DYNAMIC_ENDPOINTS NAT_DETECTION
%token ISPD_TYPE
%token ISPD_INTERFACE_NAME
%token ISPD_ROUTER
%token ISPD_VIP
%token PRIMARY_INTERFACE
%token SECONDARY_INTERFACE
%token CONTROL_INTERFACE
%token ISPD_PEER_ISERVER
%token RSD RS_PORT RS_HOST_PRIO RS_MCAST_ADDR HISTDB_SIZE
%token RS_SSN_INT RS_IFNAME RS_CP_CMD_STR RS_TMP_DIR
%token EXECD
%token MEDIAROUTING MIDCALLMEDIACHANGE
%token MAXCALLS SGK MAXRASBUFFSIZE
%token INFOTRANSCAP
%token MAXHUNTS CRID MASK HIDE FORWARD ADDR SIPTRANS INVITEC REINVITENOSDP
%token SIPMAXFORWARDS RTP SHUTDOWN GRACEFUL COMPRESSION PAD
%token MAXCALLDUR MAXHUNTALLOWDUR
%token DNS_REC_TIMEOUT
%token CALLIDLEN
%token SIP_SESSION_TIMER_MINSE SIP_SESSION_TIMER_EXPIRY
%token REALM
%token REMOVETCS2833 REMOVETCST38 DISABLE
%token CALLIDLEN LOCAL_ISDN_CC
%token ANI DNIS PORTSEL
%token HUNTALLCC TIMEREXPIRYCC USECODEMAP
%token <val> 	INTEGER MODULE SERVERTYPE UCC_PROTO BILLTYPE CDREVENT
%token <str> 	STRING CAUSESTR

%type <str> 	string
%type <val> 	integer
%type <val> 	location port ipaddr timeout 
%type <val> 	module set_mod level_mod level log_dump
%type <val> 	cdr_format cdr_attrs
%type <ipaddr> 	ipaddr_port
%type <list> 	gk_list
%type <ptr>		gk
%type <val>		status

%start iserver_configs

%%

iserver_configs	: iserver_config iserver_configs
		| /* empty */
		;

iserver_config	: global_config
		| pm_configs
		| gis_configs
		| bcs_configs
		| jserver_configs
		| rsd_configs
		| faxs_configs
		| execd_configs
		| mgmt_interface_configs
		| error iserver_config 
			{
				yyerrok;
			}
		;

global_config	: SERVERS integer
			{
				max_servers = $2;
				InitServers();
			}
		| FAXDIR string
			{
				strcpy(fax_dir_pathname, $2);
				free($2);
			}
		| FAXUSER string
			{
				strcpy(fax_user, $2);
				free($2);
			}
		| FAXPASSWORD string
			{
				strcpy(fax_password, $2);
				free($2);
			}
		| mem_configs
		| sip_configs
		| UCC UCC_PROTO
			{
				uccProto = $2;
			}
		| NO UCC_PROTO
			{
				switch ($2)
				{
				case CPROTO_SIP:
					/* Sip is disabled */
					sipAdminStatus = 0;
					break;
				case CPROTO_H323:
					/* H.323 is disabled */
					h323AdminStatus = 0;
					break;
				default:
					break;
				}
				
			}
		| ENUMDOMAIN string
			{
				strcpy(enumdomain, $2);
				free ($2);
			}
		| E911LOC string
			{
				strcpy(e911loc, $2);
				free($2);
			}
		| TAPCALL string
			{
				strcpy(tapcall, $2);
				free($2);
			}
		| UNTAPCALL string
			{
				strcpy(untapcall, $2);
				free($2);
			}
		| CODEC	G711ULAW64 integer
			{
					g711Ulaw64Duration = $3;
			}
		| CODEC	G711ALAW64 integer
			{
					g711Alaw64Duration = $3;
			}
		| CODEC	G729 integer
			{
					g729Frames = $3;
			}
		| CODEC	G7231 integer
			{
					g7231Frames = $3;
			}
		| DEFAULT CODEC G711ULAW64
			{
					defaultCodec = CodecGPCMU;	
			}
		| DEFAULT CODEC G711ALAW64
			{
					defaultCodec = CodecGPCMA;	
			}
		| DEFAULT CODEC G729
			{
					defaultCodec = CodecG729;	
			}
		| DEFAULT CODEC G7231
			{
					defaultCodec = CodecG7231;	
			}
		| PREFIX string
			{
				strcpy(prefix, $2);
				free($2);
			}
		| REALTIME 	
			{
				realTimeEnable = 1;
			}
		| MAXCALLDUR integer
			{
				if($2 <= MAX_MAX_CALL_DURATION)
					max_call_duration = $2;
				else
					max_call_duration = MAX_MAX_CALL_DURATION;
			}
		| MAXHUNTALLOWDUR integer
			{
				max_hunt_allowable_duration = $2;
			}
		| DNS_REC_TIMEOUT integer
			{
				dns_recovery_timeout = $2;
			}
		| DEFAULT REALM status
			{
			}
		;

mgmt_interface_configs: MGMT_INTERFACE '{' mgmt_interface_config_list '}'
			{
			}
;

mgmt_interface_config_list:
			mgmt_interface_config mgmt_interface_config_list
			{
			}
			|	/* epsilon transition */ 
			{
			}
;

mgmt_interface_config: MGMT_IP string
                        {
				strcpy(mgmtInterfaceIp, $2);
				free( $2 );
			}
;
		
sip_configs: SIPDOMAIN string
			{
				/* Add this to the list of local domains we own */
				strcpy(sipdomain, $2);
				free($2);
			}
		| SIPAUTH LOCAL
			{
				sipauth = 1;
			}
		| SIPAUTH RADIUS
			{
				sipauth = 2;
			}
		| NO SIPAUTH
			{
				sipauth = 0;
			}
		| SIPAUTHPASSWORD string 
			{
				strcpy(sipauthpassword, $2);
				free($2);
			}
		| RECORDROUTE
			{
				recordroute = 1;
			}
		| RECORDROUTE ENABLE
			{
				recordroute = 1;
			}
		| NO RECORDROUTE 
			{
				recordroute = 0;
			}
		| SIPSERVER SERVERTYPE
			{
				sipservertype = $2;
			}
		| SIPSERVER string
			{
				if (strlen($2))
				{
					strcpy(sipservername, $2);
				}
				free($2);
			}
		| MSWNAME string
			{
				if(strlen($2))
				{
					strcpy(mswname, $2);
				}
				free($2);
			}
		| SIPTIMER T1 INTEGER
			{
				siptimerT1 = $3;
			}
		| SIPTIMER T2 INTEGER
			{
				siptimerT2 = $3;
			}
		| SIPTIMER C INTEGER
			{
				siptimerC = $3;
			}
		| SIPTRANS INVITEC INTEGER
			{
				sipMaxInvRqtRetran = $3;
			}
		| XCONNID ENABLE
			{
				useXConnId = 1;
			}
		| SIPMAXFORWARDS INTEGER
			{
				sipmaxforwards = $2;
			}
		| SIP_SESSION_TIMER_MINSE  INTEGER
			{
				sipminSE = $2;
			}
		| SIP_SESSION_TIMER_EXPIRY INTEGER
			{
				sipsessionexpiry = $2;
			}
		| SIPHOLD3264 ON
			{
				siphold3264 = 1;
			}
		| SIPHOLD3264 OFF
			{
				siphold3264 = 0;
			}
		| SIPPORT INTEGER
			{
				sipport = $2;
			}
		;

gis_configs	: gis_config 
		| lus_config 
		| vpns_config 
		;

pm_configs	: PM
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_PM;
				InitServer(server, CONFIG_SERPLEX_PM);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

gis_config	: GIS 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_GIS;
				InitServer(server, CONFIG_SERPLEX_GIS);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

lus_config	: LUS 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_LUS;
				InitServer(server, CONFIG_SERPLEX_LUS);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

vpns_config	: VPNS 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_VPNS;
				InitServer(server, CONFIG_SERPLEX_VPNS);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

bcs_configs	: BCS 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_BCS;
				InitServer(server, CONFIG_SERPLEX_BCS);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

jserver_configs	: JSERVER 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_JSERVER;
				InitServer(server, CONFIG_SERPLEX_JSERVER);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}
			
faxs_configs	: FAXS 
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_FAXS;
				InitServer(server, CONFIG_SERPLEX_FAXS);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
			}

rsd_configs :	RSD
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_RSD;
				InitServer(server, CONFIG_SERPLEX_RSD);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
				RSDConfig = server->location.type;
			}

execd_configs :	EXECD
			{
				CONFIG_ASSERT(inserver < max_servers);
				server = &serplexes[inserver++];
				server->type = CONFIG_SERPLEX_EXECD;
				InitServer(server, CONFIG_SERPLEX_EXECD);
			}
		location '{' config_list '}'
			{
				memcpy(&server->debconfigs.netLogStatus[0],
					&NetLogStatus[0], MNETLOGMAX);
				server->debconfigs.flags = netLogStruct.flags;
				ExecDConfig = server->location.type;
			}

location	: NONE
			{
				server->location.type = CONFIG_LOCATION_NONE;
				server->location.address.sin_addr.s_addr = 0;
			}
		| LOCAL 
			{
				server->location.type = CONFIG_LOCATION_LOCAL;
				server->location.address.sin_addr.s_addr = 
					inet_addr("127.0.0.1");
			}
		| LOCAL '/' port
			{
				server->location.type = CONFIG_LOCATION_LOCAL;
				server->location.address.sin_addr.s_addr = 
					inet_addr("127.0.0.1");
				server->location.address.sin_port = htons($3);
			}
		| ipaddr 
			{
				server->location.type = CONFIG_LOCATION_INADDR;
				server->location.address.sin_addr.s_addr = 
					htonl($1);
			}
		| ipaddr_port
			{
				server->location.type = CONFIG_LOCATION_INADDR;
				server->location.address.sin_addr.s_addr = 
					htonl($1.sin_addr.s_addr);
				server->location.address.sin_port = htons($1.sin_port);
			}
		;

ipaddr	: integer '.' integer '.' integer '.' integer
			{
				$$ = ($1 << 24) | ($3 << 16) | ($5 << 8) | $7;
			}

port	: integer

ipaddr_port	: ipaddr '/' port
			{
				memset(&$$, 0, sizeof(struct sockaddr_in));
				$$.sin_addr.s_addr = $1;
				$$.sin_port = $3;
			}

config_list	: configs config_list
		| /* empty */
		;

configs		: db_configs
		| age_configs
		| fce_configs
		| ispd_configs
		| rsd_configs
		| execd_configs
		| fax_configs
		| debug_configs
		| sys_configs
		| h323_configs
       	| jserver_configs
		| policy_configs
		| cdr_configs
		| billing_configs
		| radius_configs
		| cp_policy
		| obp_policy
		;

db_configs	: DBTOKEN '{' '}'

age_configs	: AGE 
			{
				server->age.location.type = 
					server->location.type;
				server->age.location.address.sin_addr.s_addr =
					server->location.address.sin_addr.s_addr;
			}
		'{' age_config_parms_list '}'		


fce_configs	: FCE
		'{' fce_config_parms_list '}'		

ispd_configs	: ISPD
			{
				server->ispd.location.type = 
					server->location.type;
				ispd_type = ISPD_TYPE_ACTIVE;
			}
		'{' ispd_config_parms_list '}'		


fax_configs	: TMOUT timeout
			{
				server->fax.dispatch_timeout = $2;
			}
		| MAXRETRANSCOUNT integer
			{
				server->fax.max_retransmit_count = $2;
			}
	
mem_configs : SEGS integer
			{
				max_segs = $2;
			}
		| SEGSIZE integer
			{
				max_segsize = $2;
			}
		| SEGADDR integer
			{
					segaddr = (void *)$2;
			}
	 	| SEGADDRTYPE DYNAMIC
			{
					segaddrtype = 0;
			}
	 	| SEGADDRTYPE FIXED
			{
				segaddrtype = 1;
			}
		| SEGOWNER GIS
			{
				segowner = CONFIG_SERPLEX_GIS;
				segaddrtype = 1;
				segaddr = (void *)ISERVER_SHM_STARTADDR;
			}
		| SEGOWNER JSERVER
			{
				segowner = CONFIG_SERPLEX_JSERVER;
			}
		| SEGOWNER CLI
			{
				segowner = CONFIG_SERPLEX_CLI;
			}
		;

sys_configs	: PRIO integer
			{
				server->prio = $2;
			}
		| ENDPTS integer
			{
				server->max_endpts = $2;
			}
		| GWS integer
			{
				server->max_gws = $2;
			}

		| DAEMONIZE
			{
				server->daemonize = 1;
			}
		| NO DAEMONIZE
			{
				server->daemonize = 0;
			}
		| LWPS integer
			{
				clwp = $2;
			}
		| thread_configs
		| H323QSIZE integer
			{
				bridgeH323QSize = $2;
			}
		| proc_configs
		| UCC_PROTO INSTANCE integer
			{
				nh323Instances = $3;
				nh323CfgInstances = $3;
				if (nh323Instances > 1)
				{
					nh323Instances += 3;
					nH323Threads = nh323Instances-1;
				}
			}
		| UCC_PROTO MAXCALLS integer
			{
				if ($1 == CPROTO_H323)
				{
					h323maxCalls = $3;
				}
			}
		| UCC_PROTO MAXCALLS SGK integer
			{
				if ($1 == CPROTO_H323)
				{
					h323maxCallsSgk = $4;
				}
			}
		| UCC_PROTO MAXRASBUFFSIZE integer
			{
				if ($1 == CPROTO_H323)
				{
					if ($3 < 2048 || $3 > 4096 )
					{
						h323RasMaxBuffSize = 2048;
					}
  					else
					{
						h323RasMaxBuffSize = $3;
					}
				}
			}
		| UCC_PROTO INFOTRANSCAP string
			{
				if ($1 == CPROTO_H323)
				{
					h323infoTransCap = str2enum(infoTransCapOptions, $3);
					if (h323infoTransCap < 0)
					{
						h323infoTransCap = INFO_TRANSCAP_PASS;
					}
				}
				free($3);
			}
		| UCC_PROTO REMOVETCS2833 ENABLE
			{
				if ($1 == CPROTO_H323)
				{
					h323RemoveTcs2833 = 1;
				}
			}
		| UCC_PROTO REMOVETCS2833 DISABLE
			{
				if ($1 == CPROTO_H323)
				{
					h323RemoveTcs2833 = 0;
				}
			}
		| UCC_PROTO REMOVETCST38 ENABLE
			{
				if ($1 == CPROTO_H323)
				{
					h323RemoveTcsT38 = 1;
				}
			}
		| UCC_PROTO REMOVETCST38 DISABLE
			{
				if ($1 == CPROTO_H323)
				{
					h323RemoveTcsT38 = 0;
				}
			}
		| UCC_PROTO MAXCALLS PAD integer integer
			{
				if ($1 == CPROTO_H323)
				{
					h323maxCallsPadFixed = $4;
					h323maxCallsPadVariable = $5;
				}
			}
                | ALLOW DESTARQ ALL
                        {
			        allowDestArq = 1;
		        }

		| ALLOW AUTHARQ ALL
			{
				allowAuthArq = 1;
			}
		;

proc_configs: PROCS integer
			{
				nprocs = $2;
			}

thread_configs: THREADS STACK integer
			{
				server->threadstack = $3;
			}
		| THREADS IWF integer 
			{
				nConfIdThreads = $3;
			}
		| THREADS TSM integer 
			{
				server->threads = $3;
				xthreads = $3;
			}
		| THREADS UA integer 
			{
				nCallIdThreads = $3;
			}
		| THREADS BRIDGE integer 
			{
				nBridgeThreads = $3;
			}
		| THREADS RADIUS integer 
			{
				nRadiusThreads = $3;
			}
		| THREADS UCC_PROTO integer 
			{
				if ($2 == CPROTO_H323)
				{
					if (nh323Instances == 1)
					{	
						nH323Threads = $3;
					}
				}
			}
		;

h323_configs : GKID string
			{
				strcpy(gkid, $2);
				free($2);
			}
		| RRQTIMER integer
			{
				rrqtimer = $2;
				if (rrqtimer == (unsigned int)-1)
				{
					 rrqtimer = 30;
				}
			}
		| RRQTTL integer
			{
				rrqttl = $2;
				if (rrqttl == (unsigned int)-1)
				{
					 rrqttl = 0x0fffffff;
				}
			}
		| CALLTTL integer
			{
				callttl = $2;
				if (callttl == (unsigned int)-1)
				{
					 callttl = 0x0fffffff;
				}
			}
		| ROUTE H245
			{
				routeH245 = 1;
			}
		| FORCE H245
			{
				// forceh245 => routeh245, but not vice versa
				forceh245 = 1;
				routeH245 = 1;
			}
		| NO FORCE H245
			{
				forceh245 = 0;
			}
		| NO ROUTE H245
			{
				if (forceh245 != 1)
				{
					routeH245 = 0;
				}
			}
		| ROUTE CALL
			{
				routecall = 1;
			}
		| NO ROUTE CALL
			{
				routecall = 0;
			}
		| SHARE CALL
			{
				sharecall = 1;
			}
		| NO SHARE CALL
			{
				sharecall = 0;
			}
		| FASTSTART ON
			{
				doFastStart = 1;
			}
		| FASTSTART OFF
			{
				doFastStart = 0;
			}
		| H245 TUNNELING
			{
				h245Tunneling = 1;
			}
		| NO H245 TUNNELING
			{
				h245Tunneling = 0;
			}
		| LOCAL PROCEEDING 
			{
				localProceeding = 1;
			}
		| NO LOCAL PROCEEDING 
			{
				localProceeding = 0;
			}
		| ALWAYS2833 ON
			{
				always2833 = 1;
			}
		| ALWAYS2833 OFF
			{
				always2833 = 0;
			}
		| MAPISDNCC ENABLE
			{
				mapisdncc = 1;
			}
		| MAPISDNCC DISABLE
			{
				mapisdncc = 0;
			}
		| MAPLRJREASON ENABLE
			{
				maplrjreason = 1;
			}
		| MAPLRJREASON DISABLE
			{
				maplrjreason = 0;
			}
		| FSINCONNECT ENABLE 
			{
				fsInConnect = 1;
			}
		| FSINCONNECT DISABLE 
			{
				fsInConnect = 0;
			}
		| GETANIFROMACF ENABLE 
			{
				getAniFromAcf = 1;
			}
		| GETANIFROMACF DISABLE 
			{
				getAniFromAcf = 0;
			}
		| ROLLOVERTIME integer
			{
				rolloverTime = $2;
			}
		| UPDATE ALLOCATIONS
			{
				updateAllocations = 1;
			}
		| NO UPDATE ALLOCATIONS
			{
				updateAllocations = 0;
			}
		| HAIRPIN 
			{
				allowHairPin= 1;
			}
		| NO HAIRPIN
			{
				allowHairPin= 0;
			}
		| MAXHUNTS integer
			{
			        int sval = $2;
				if (sval < 0)
				{
					maxHunts = -1;
				}
				else if (sval >= SYSTEM_MAX_HUNTS)
				{
					maxHunts = SYSTEM_MAX_HUNTS;
				}
				else
				{
					maxHunts = sval;
				}
			}
		| TIMEREXPIRYCC  integer
			{
				recoveryOnTimerExpiry = $2;
			}
		| CRID integer
			{
				iservercrId = $2;
			}
		| CRID ON
			{
				crids = 1;
			}
		| CRID OFF
			{
				crids = 0;
			}
		| H323CPS integer
			{
				h323Cps = $2;
			}
		| LOCAL_ISDN_CC '{' isdn_cc_list '}'
			{
			}
		| H323QLEN integer
			{
				h323QLen = $2;
			}
		| SHUTDOWN UCC_PROTO GRACEFUL
			{
				h323GracefulShutdown = 1;
			}
		| altgk_list
		;

altgk_list : ALTGK '{' gk_list '}'
			{
				if (altGkList)
				{
					listDestroy(altGkList);
					altGkList = NULL;
				}

				altGkList = $3;
			}
		;

gk_list : gk_list gk
			{
				$$ = $1;
				listAddItem($$, $2);
			}
		| gk
			{
				$$ = listInit();
				listAddItem($$, $1);
			}
		;
 
gk : GK ipaddr_port
			{
				struct sockaddr_in *addr;

				// address is in host format
				addr = (struct sockaddr_in *)
							malloc(sizeof(struct sockaddr_in));

				memcpy(addr, &($2), sizeof(struct sockaddr_in));
				$$ = addr;
			}
		;

isdn_cc_list : isdn_cc_list isdn_cc
			{
			}
		|  isdn_cc
			{
			}
		;

isdn_cc : CAUSESTR integer
			{
				setH323Code($1, $2);
			}
	| CAUSESTR integer UCC_PROTO integer
			{
				setH323Code($1, $2);
				setSipCode($1, $4);
			}
		;

billing_configs : BILLING billtype billing_auth_config

billtype: BILLTYPE
				{
						billingType = $1;
				}
		| NONE
				{
						billingType = BILLING_NONE;
				}
		;

billing_auth_config : '{' billing_auth_config_list '}'
		| /* empty */
		;

billing_auth_config_list: billing_auth billing_auth_config_list
		| /* empty */
		;

billing_auth: FIRST_AUTH string string
			{
				nx_strlcpy(first_auth_username, $2, sizeof(first_auth_username));
				nx_strlcpy(first_auth_password, $3, sizeof(first_auth_password));
				free($2); free($3);
                                
			}
		| SECOND_AUTH string string
			{
				nx_strlcpy(second_auth_username, $2, sizeof(second_auth_username));
				nx_strlcpy(second_auth_password, $3, sizeof(second_auth_password));
				free($2); free($3);
			}
		;

radius_configs : RADIUS
		'{' radius_config_parms_list '}'		
		;

radius_config_parms_list	: radius_config_parm radius_config_parms_list
		| /* empty */
		;

radius_config_parm	: RADIUS_SERVER string
			{
				int i;

				for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
				{
					if(strlen(rad_server_addr[i]) == 0)
					{
						nx_strlcpy(rad_server_addr[i], $2, RADSERVERADDR_LEN);
						break;
					}
				}

				free($2);
			}
		| SHARED_SECRET string
			{
				int i;

				for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
				{
					if(strlen(secret[i]) == 0)
					{
						nx_strlcpy(secret[i], $2, SECRET_LEN);
						break;
					}
				}

				free($2);
			}
		| RADIUS_TIMEOUT integer
			{
				rad_timeout = $2;
			}
		| RADIUS_RETRIES integer
			{
				rad_retries = $2;
			}
		| RADIUS_DEADTIME integer
			{
				rad_deadtime = $2;
			}
		| RADIUS_ACCT ON
			{
				rad_acct = TRUE;
			}
		| RADIUS_ACCT OFF
			{
				rad_acct = FALSE;
			}
		| ACCT_SESSION_ID OVERLOADED
			{
				rad_acct_session_id_overloaded = TRUE;
			}
		| DIR string
			{
				nx_strlcpy(rad_dirname, $2, CDRDIRNAMELEN);

				free($2);
			}
		;

fce_config_parms_list	: fce_config_parm fce_config_parms_list
		| /* empty */
		;


fce_config_parm	: FWNAME string
			{
			   strcpy(fceConfigFwName, $2);
			   free($2);
			}
                | FWIPADDR ipaddr
                        {
			  fceFirewallAddresses[fceFirewallNumber++] = $2;
			}
		| DEFAULT FWPRIVATE
			{
                          /*			   fceDefaultPublic = FALSE;*/
			}
		| DEFAULT FWPUBLIC
			{
                          /*			   fceDefaultPublic = TRUE;*/
			}
		| FWPRIVATE string
			{
                          /*			   FCENetworkListConfig($2, FALSE);*/
			   free($2);
			}
		| FWPUBLIC string
			{
                          /*			   FCENetworkListConfig($2, TRUE);*/
			   free($2);
			}
		| FWCONNECTADDR ipaddr
			{
                          /*			   fceConfigOurIpAddr = $2;*/
			}
		| H245 ON
			{
			  /*                       fceH245PinholeEnabled = TRUE;*/
			}
		| H245 OFF
			{
			  /* 			   fceH245PinholeEnabled = FALSE;*/
			}
		| DEFAULT MEDIAROUTING ON
			{
                          /*			   defaultMediaRouting = 1;*/
			}
		| DEFAULT MEDIAROUTING OFF
			{
                          /*			   defaultMediaRouting = 0;*/
			}
		| DEFAULT MIDCALLMEDIACHANGE ON
			{
                          /*			   defaultHideAddressChange = 1;*/
			}
		| DEFAULT MIDCALLMEDIACHANGE OFF
			{
                          /*			   defaultHideAddressChange = 0;*/
			}
/*                        FCE_REMOVED */
		;

primary_interface_configs	: PRIMARY_INTERFACE
			{
			}
		'{' primary_interface_parms_list '}'		

primary_interface_parms_list : primary_interface_parm primary_interface_parms_list
		| /* empty */
		;

primary_interface_parm :
		  ISPD_INTERFACE_NAME string
			{
				free($2);
			}
		| ISPD_ROUTER string
			{
				free($2);
			}
		| ISPD_VIP string
			{
				free($2);
			}
		;

secondary_interface_configs	: SECONDARY_INTERFACE
			{
			}
		'{' secondary_interface_parms_list '}'		

secondary_interface_parms_list : secondary_interface_parm secondary_interface_parms_list
		| /* empty */
		;

secondary_interface_parm :
		  ISPD_INTERFACE_NAME string
			{
				free($2);
			}
		| ISPD_ROUTER string
			{
				free($2);
			}
		| ISPD_VIP string
			{
				free($2);
			}
		;

control_interface_configs	: CONTROL_INTERFACE
			{
				ispd_ctl.defined = 1;
			}
		'{' control_interface_parms_list '}'		

control_interface_parms_list : control_interface_parm control_interface_parms_list
		| /* empty */
		;

control_interface_parm :
		  ISPD_INTERFACE_NAME string
			{
				strcpy( ispd_ctl.name, $2 );
				free($2);
			}
		| ISPD_PEER_ISERVER string
			{
				if ( ispd_ctl.peer_count < MAX_ISPD_PEERS )
				{
					strcpy( &ispd_ctl.peer_iservers[ispd_ctl.peer_count][0], $2 );
					free($2);
					ispd_ctl.peer_count++;
				}
			}
		;

ispd_config_parms_list	: 	ispd_config_parm ispd_config_parms_list
		| /* empty */
		;

ispd_config_parm	:
		  ISPD_TYPE string
			{
				if ( !strcmp( $2, "standby" ) )
				{
					ispd_type = ISPD_TYPE_STANDBY;
				}
				else
				if ( !strcmp( $2, "active" ) )
				{
					ispd_type = ISPD_TYPE_ACTIVE;
				}
				else
				{
					ispd_type = ISPD_TYPE_DISABLED;
				}
			
				free($2);
			}
		| primary_interface_configs
		| secondary_interface_configs
		| control_interface_configs
		| SCM ON
			{
				doScm = 1;
			}
		| SCM OFF
			{
				doScm = 0;
			}
		;

rsd_configs	:
			RS_IFNAME		string
			{
				strcpy( rs_ifname, $2 );
				free($2);
			}
		|	RS_MCAST_ADDR	string
			{
				strcpy( rs_mcast_addr, $2 );
				free($2);
			}
		|	RS_PORT	string
			{
				strcpy( rs_port, $2 );
				free($2);
			}
		|	RS_CP_CMD_STR	string
			{
				strcpy( rs_cp_cmd_str, $2 );
				rs_cp_cmd_str_fl = 1;
				free($2);
			}
		|	RS_TMP_DIR		string
			{
				strcpy( rs_tmp_dir, $2 );
				rs_tmp_dir_fl = 1;
				free($2);
			}
		|	RS_HOST_PRIO	integer
			{
				rs_host_prio = $2;
			}
		|	RS_SSN_INT	integer
			{
				rs_ssn_int = $2;
				rs_ssn_int_fl = 1;
			}
		|	HISTDB_SIZE	integer
			{
				histdb_size = $2;
			}
		;

age_config_parms_list	: age_config_parm age_config_parms_list
		| /* empty */
		;

age_config_parm	: TMOUT timeout
			{
				server->age.cache_timeout = $2;
			}
		| PORT port
			{
				server->age.location.address.sin_port = 
						htons($2);
			}
		| PRIO integer
			{
				server->age.prio = $2;
			}
		| SLEEP integer
			{
				server->age.sleep_time = $2;
			}
		;

timeout	: integer
	;

debug_configs	: DDEBUG module set_mod level_mod level
			{
				SetDebugLevel($2, $3, $4, $5);
			}	
		| DLOG log_dump
			{
				if ($2 == DSYSLOG)
				{
					netLogStruct.flags |=
						NETLOG_ASYNC;
				}
			}
		| HDEBUG LEVEL integer
			{
				server->cusconfigs.level = $3;
				if (_msSetDebugLevel)
				{
					_msSetDebugLevel($3);
				}
			}
		| HDEBUG string
			{
				SetCustomDebug($2);
				if (_msAdd)
				{
					_msAdd($2);
				}
				free($2);
			}
		| NO HDEBUG string
			{
				if (_msDelete)
				{
					_msDelete($3);
				}
				free($3);
			}
		| HLOG log_dump
			{
				if ($2 == DTERMINAL)
				{
					netLogStruct.flags |=
						NETLOG_H323TERMINAL;
				}
			}
		| SDEBUG LEVEL integer
			{
				server->cusconfigs.slevel = $3;
				if (_sipSetTraceLevel)
				{
						_sipSetTraceLevel($3);
				}
			}
		| SLOG log_dump
			{
				if ($2 == DTERMINAL)
				{
					netLogStruct.flags |=
						NETLOG_SIPTERMINAL;
				}
				else if ($2 == DSYSLOG)
				{
					netLogStruct.flags |=
						NETLOG_SIPSYSLOG;
				}
			}
		;

jserver_configs	: jserver_perms

jserver_perms	: PERMISSION_READ string
			{
				strcpy(readPass, $2);
				free($2);
			}
                | PERMISSION_WRITE string
			{
				strcpy(writePass, $2);
				free($2);
			}
		| DLOGFILE string
			{
				strcpy(jsLogFile, $2);
				free($2);
			}
		| DLOGLEVEL string
			{
			    strcpy(jsLogLevel, $2);
				free($2);
			}
		| COMPRESSION ON
			{
			    jsCompression = 1;
			}
		| COMPRESSION OFF
			{
			    jsCompression = 0;
			}
                ;

policy_configs : POLICY policy_description
			{
			}
        | CALLSTATE callstate_description
			{
			}
        | LOCAL REINVITENOSDP
			{
				localReInviteNoSdp = 1;
			}
        | NO LOCAL REINVITENOSDP
			{
				localReInviteNoSdp = 0;
			}
        ;

policy_description : '{' '}'
		| NONE
			{
				nopolicy = 1;
			}
		;

cp_policy : CP '{' cp_config_listdef '}'
	;

cp_config_listdef : cp_config_list
	| /* empty */
	;

cp_config_list : cp_config_list cp_config
	| cp_config
	;

cp_config : ALLOW DEST ALL
		{
			allowDestAll = 1;
		}
	| ALLOW DEST NONE
		{
			allowDestAll = 0;
		}
      	| ALLOW SRC ALL
		{
			allowSrcAll = 1;
		}
	| ALLOW SRC NONE
		{
			allowSrcAll = 0;
		}
	| HIDE SRC ADDR
		{
			forwardSrcAddr = 0;
		}
	| FORWARD SRC ADDR
		{
			forwardSrcAddr = 1;
		}
	| GWS LRU
		{
			cpRoutingPolicy = CP_ROUTE_LRU;
		}
	| GWS UTILZ
		{
			cpRoutingPolicy = CP_ROUTE_UTILZ;
		}
	| ALLOW RTP ALL
		{
			allowRtpAll = 1;
		}
	| NO ALLOW RTP ALL
		{
			allowRtpAll = 0;
		}
	| LRU ROUTE integer
		{
			nlruRoutes = $3;
		}
	| ALLOW ANI PORTSEL
		{
			allowANIPortSel = 1;
		}
	| ALLOW DNIS PORTSEL
		{
			allowDNISPortSel = 1;
		}
	| HUNTALLCC
		{
			huntAllCauseCodes = 1;
			//strcpy(codemaptemplate, CODEMAP_LONG);
			//strcpy(codemapfilename, CODEMAP_LONGFILE);
		}
	| USECODEMAP string
		{
			strcpy(codemaptemplate, $2);
			strcpy(codemapfilename, "codemap_");
			strcat(codemapfilename, $2);
			strcat(codemapfilename, ".dat");
			free($2);
		}
	;

obp_policy : OBP '{' obp_config_listdef '}'
		{
			obpEnabled = 1;
		}
	;

obp_config_listdef : obp_config_list
	| /* empty */
	;

obp_config_list : obp_config_list obp_config
	| obp_config
	;

obp_config : ALLOW INTERNAL
		{
			allowInternalCalling = 1;
		}
	| ALLOW DYNAMIC_ENDPOINTS
		{
			allowDynamicEndpoints = 1;
		}
	| ENABLE NAT_DETECTION
		{
			enableNatDetection = 1;
		}
	;

cdr_configs : CDR cdr_spec cdr_descriptions 
		;

cdr_spec : cdr_format cdr_attrs
		{
			cdrformat = $1;
			cdrtype = $2;
		}
	| /* empty */
		{
		//	cdrformat = CDRFORMAT_UNKNOWN;
		}
	;

cdr_format: XML
			{
				$$ = CDRFORMAT_XML;
			}
		| MINDCTI
			{
				$$ = CDRFORMAT_MIND;
			}
		| DSYSLOG
			{
				$$ = CDRFORMAT_SYSLOG;
			}
		;

cdr_attrs: FIXED
			{
				$$ = CDRMINDCTIFIXED;
			}
		| DAILY
			{
				$$ = CDRMINDCTIDAILY;
			}
		| SEQ
			{
				$$ = CDRMINDCTISEQ;
			}
        | TIME
            {
                $$ = CDRNEXTONETIME;
            }
		| IPPROTO
			{
				$$ = CDRMINDCTIFIXED;
			}
		| /* empty */
			{
				$$ = CDRMINDCTIFIXED;
			}
		;

cdr_descriptions: '{' cdr_description_list '}'
		| /* empty */
		;

cdr_description_list: cdr_description cdr_description_list
		| /* empty */
		;

cdr_config: cdr_event cdr_spec cdr_descriptions

cdr_description: DIR string
			{
				strcpy(cdrdirname, $2);
				free($2);
                                
			}
		| CDRTIMER integer
			{
			    cdrtimer = $2;
			}
		| CALLIDLEN INTEGER
			{
				if (($2<256) && ($2>0))
				{
					cdrcallidlen = $2;
				}
			}
		| cdr_config
		;

cdr_event: CDREVENT
		{
			cdrevents |= $1;
		}
	;

callstate_description : '{' '}'
		| NONE
			{
				nocallstate = 1;
			}
		;

module	: MODULE
			{
				$$ = $1;
			}
	;

set_mod	: '!'
			{ 
				$$ = 0; 
			}
	| /* empty */
			{ 
				$$ = 1; 
			}
	;

level_mod	: '=' 
			{ 
				$$ = 0; 
			}
		| /* empty */
			{ 
				$$ = 1; 
			}
		;

level	: integer
	;

integer	: INTEGER
			{
				$$ = $1;
			}

string	: STRING
			{
				$$ = $1;
			}

status : ENABLE
			{
				$$ = 1;
			}
		| DISABLE
			{
				$$ = 0;
			}
		;

log_dump	: DTERMINAL
			{
				$$ = DTERMINAL;
			}
		| DSYSLOG
			{
				$$ = DSYSLOG;
			}
		;

%%

/* Called before parse_config */
int
InitVars()
{
	/* ALL NOT-MANDATORY arguments which assume defaults
	 * should be re-initialized here
	 * as a re-config, has to take care of going back to 
	 * defaults, if user takes out configs
	 */

	int i;

	max_servers = CONFIG_SERPLEX_MAX;
	max_ccalls = 100;
	inserver = 0; /* running index on servers */
	doScm = 0;
	rrqttl = RRQTTL_DEFAULT;
	rrqtimer = 30;
	routeH245 = H245_DEFAULT;
	forceh245 = 0;
	routecall = 1;
	recordroute = 0;
	doFastStart = 1;
	localProceeding = 0;
	always2833 = 0;
	mapisdncc = 0;
	maplrjreason = 0;
	fsInConnect=0;
	siphold3264 = 0;
	updateAllocations = 1;
	billingType = BILLING_POSTPAID;
	/* DO NOT INITIALIZE sharecall here. It is
	 * a ONE time initialization, and requires a
	 * an iserver restart.
	 */
	memset(gkid, 0, GKID_LEN);
	gethostname(mswname, MSWNAME_LEN);
	strcpy(sipservername, mswname);
	strcpy(cdrdirname, ".");
	cdrtype = CDRMINDCTIFIXED;
	cdrformat = CDRFORMAT_MIND;
	cdrtimer = 60;
	cdrevents = CDREND1;

	memset(prefix, 0, PHONE_NUM_LEN);
	memset(sipdomain, 0, SIPDOMAIN_LEN);
	memset(sipauthpassword, 0, SIPAUTHPASSWORD_LEN);

	memset(enumdomain, 0, ENUMDOMAIN_LEN);
	memset(e911loc, 0, 128);
	memset(tapcall, 0, 128);
	memset(untapcall, 0, 128);

	sipauth = 0;
	sipservertype = SERVER_PROXYSTATEFULL;

	useXConnId = 0;

	uccProto = CPROTO_UCC;

	g711Ulaw64Duration = 20;
	g711Alaw64Duration = 20;
	g729Frames = 2;
	g7231Frames = 1;

	/* initialize fce related params */
	strcpy(fceConfigFwName, "none");
#if FCE_REMOVED
	fceConfigOurIpAddr = 0;
	FCENetworkListConfig(NULL, TRUE);
	fceH245PinholeEnabled = FALSE;
	fceDefaultPublic = TRUE;
	defaultMediaRouting = 1;
	defaultHideAddressChange = 1;
#endif
	/* initialize ispd related params */

	memset( &ispd_type, 0,  sizeof( ispd_server_type_t ) );
        ispd_type = ISPD_TYPE_DISABLED;
	memset( &ispd_primary, 0,  sizeof( ispd_interface_t ) );
	memset( &ispd_secondary, 0,  sizeof( ispd_interface_t ) );
	memset( &ispd_ctl, 0,  sizeof( ispd_ctl_interface_t ) );

	/* initialize rsd related params */
	memset(rs_ifname, 0, sizeof(rs_ifname));
	nx_strlcpy(rs_mcast_addr, RS_DEF_MCAST_ADDR, sizeof(rs_mcast_addr));
	nx_strlcpy(rs_port, RS_DEF_PORT, sizeof(rs_port));
	nx_strlcpy(rs_tmp_dir, RS_DEF_TMP_DIR, sizeof(rs_tmp_dir));
	nx_strlcpy(rs_cp_cmd_str, RS_DEF_CP_CMD_STR, sizeof(rs_cp_cmd_str));
	rs_host_prio = RS_DEF_HOST_PRIO;
	rs_ssn_int = RS_SEND_SEQNUM_INT;
	histdb_size = CLI_MAX_HIST;

	/* Initialize all the rsd variable related flags */
	rs_tmp_dir_fl = 0; 
	rs_cp_cmd_str_fl = 0;
	rs_ssn_int_fl = 0; 

	RSDConfig = 0;

	clwp = 10;

	siptimerT1 = 500000; 
	siptimerT2 = 4000000;
	siptimerC = 180;
	sipMaxInvRqtRetran = 7;

	class1deadline = 50000; 
	class2deadline = 200000;

	if (altGkList)
	{
		listDestroy(altGkList);
		altGkList = NULL;
	}

	allowDestAll = 0;
	allowDestArq = 0;
	allowAuthArq = 0;
	allowSrcAll = 0;
	forwardSrcAddr = 0;
	allowRtpAll = 1;

	huntAllCauseCodes = 0;

    strcpy(codemaptemplate, CODEMAP_SHORT);
    strcpy(codemapfilename, CODEMAP_SHORTFILE);

	routeDebug = 0;

	max_segs = 6;
	max_segsize = 0x100000;
	segaddrtype = 0;
	segowner = CONFIG_SERPLEX_JSERVER;
	allowHairPin = 1;

	for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
	{
		memset(rad_server_addr[i], 0, RADSERVERADDR_LEN);
		memset(secret[i], 0, SECRET_LEN);
	}

	rad_timeout = 5;
	rad_retries = 4;
	rad_deadtime = 0;
	rad_acct = FALSE;
	strcpy(rad_dirname, ".");
	rad_acct_session_id_overloaded = FALSE;

	rad_failed_timeout = 60;

	memset(first_auth_username, 0, sizeof(first_auth_username));
	memset(first_auth_password, 0, sizeof(first_auth_password));
	memset(second_auth_username, 0, sizeof(second_auth_username));
	memset(second_auth_password, 0, sizeof(second_auth_password));

	obpEnabled = 0;
	allowInternalCalling = 0;
	allowDynamicEndpoints = 0;
	enableNatDetection = 0;

	maxHunts = 1;

	max_call_duration = 0;
	max_hunt_allowable_duration = 0;

	iservercrId = 1;
	crids = 0;

	h323GracefulShutdown = 0;

        strcpy(jsLogLevel, "normal");
        strcpy(jsLogFile, "/tmp/jserverlogfile");
        memset(readPass, 0, sizeof(readPass));
        memset(writePass, 0, sizeof(writePass));
        jsCompression = 0;

	h323maxCallsPadFixed = H323MAXCALLSPADF;
	h323maxCallsPadVariable = H323MAXCALLSPADV;

	dns_recovery_timeout = 120;

	cdrcallidlen = 64;

	allowANIPortSel = 0;
	allowDNISPortSel = 0;
	recoveryOnTimerExpiry=0;

	localReInviteNoSdp = 0;

	strcpy(mgmtInterfaceIp, "");

	return 0;
}

/* called on a servers statement and also just before parse_config */
static int
InitServers()
{
	int i;

	if (serplexes != NULL)
	{
		free(serplexes);
	}

	serplexes = (serplex_config *)malloc(max_servers*sizeof(serplex_config));

	for (i=0; i < max_servers; i++)
	{
		serplexes[i].type = CONFIG_SERPLEX_NONE;
		serplexes[i].location.type = CONFIG_LOCATION_NONE;
		serplexes[i].prio = 0;
		serplexes[i].max_endpts = 0;
		serplexes[i].max_gws = 0;
		serplexes[i].daemonize = 1;
		serplexes[i].threads = 5;
		serplexes[i].threadstack = 256;
	}
	return(0);
}

static int
InitServer(serplex_config *server, int type)
{
	int i=0;
	/* Initialize logging */
	netLogStruct.flags = 0;
	memset(&NetLogStatus[0], 0, MNETLOGMAX);

	server->type = type;
	server->location.type = CONFIG_LOCATION_NONE;

	server->location.address.sin_family = AF_INET;
	server->location.address.sin_addr.s_addr = 
		inet_addr("127.0.0.1");

	memset(&server->age.location.address, 0, 
		sizeof(struct sockaddr_in));
	server->age.location.address.sin_family = AF_INET;
	server->age.location.type = CONFIG_LOCATION_NONE;
	server->age.location.address.sin_addr.s_addr =
		inet_addr("127.0.0.1");

	server->ispd.location.type = CONFIG_LOCATION_NONE;

	switch (type)
	{
	case CONFIG_SERPLEX_GIS:
		server->location.address.sin_port = 
			htons(ALOID_LOOKUP_PORT_NUMBER);
		server->age.location.address.sin_port = 
			htons(ALOID_AGING_PORT_NUMBER);
		break;
	case CONFIG_SERPLEX_LUS:
		server->location.address.sin_port = 
			htons(ALOID_LOOKUP_PORT_NUMBER);
		server->age.location.address.sin_port = 
			htons(ALOID_AGING_PORT_NUMBER);
		break;
	case CONFIG_SERPLEX_VPNS:
		server->location.address.sin_port = 
			htons(VPNS_LOOKUP_PORT_NUMBER);
		server->age.location.address.sin_port = 
			htons(VPNS_AGING_PORT_NUMBER);
		break;
	case CONFIG_SERPLEX_BCS:
		server->location.address.sin_port = 
			htons(BCS_PORT_NUMBER);
		break;
	case CONFIG_SERPLEX_FAXS:
		server->fax.dispatch_timeout = 300;
		break;
	case CONFIG_SERPLEX_NONE:
	default:
		break;
	}

	server->age.cache_timeout = CACHE_TIMEOUT_DEFAULT;

	server->flags = 0;

	/* Initialize debugging */
	memset(&server->debconfigs.netLogStatus[0], 0,
		MNETLOGMAX);
	server->debconfigs.flags = 0;	

	/* Initialize custom logging */
	server->cusconfigs.level	=	0;
	server->cusconfigs.slevel	=	0;
	for(i=0; i < CUSLOGMAX; i++){
		server->cusconfigs.customLogStatus[i]	=	0;
	}

	return(0);
}

int
InitConfig()
{
	nopolicy = 0;
	nocallstate = 0;

	InitServers();

	NetLogInit();
	NetLogOpen(NULL, 0, NETLOG_TERMINAL);
	return(0);
}

int
SetDebugLevel(int moduleid, int set, int all, int level)
{
	if ((moduleid < 0) || (moduleid >= MNETLOGMAX))
	{
		/* Error */
		printf("invalid module id %d\n", moduleid);
		return -1;
	}

	switch (level)                                                          
    {                                                                       
		case 1:                                                         
       		level = NETLOG_DEBUG1;                                  
            break;                                                  
        case 2:                                                         
            level = NETLOG_DEBUG2;                                  
            break;                                                  
        case 3:                                                         
            level = NETLOG_DEBUG3;                                  
            break;                                                  
        case 4:                                                         
            level = NETLOG_DEBUG4;                                  
            break;                                                  
        default:                                                        
            level = 0;                                              
            printf("Invalid level %d\n", level);                    
            break;                                                  
    }                                            

	(set == 1)                                                              
                ? ( (all == 1)                                                  
                        ? NETLOG_SETLEVEL(moduleid, level)                      
                        : NETLOG_SETLEVELE(moduleid, level)                     
                  )                                                             
                : ( (all == 1)                                                  
                        ? NETLOG_RESETLEVEL(moduleid, level)                    
                        : NETLOG_RESETLEVELE(moduleid, level)                   
                  );                                                            
                                                                                
        if (set == 1)                                                           
                NETLOG_SETLEVELE(moduleid, NETLOG_DEBUGMASK);                   
                                                                                
        return 1;                            	
}
void
SetCustomDebug(char* debug)
{
	if(strcmp(CustomLogs[TPKTCHAN],debug)	==	0)
		server->cusconfigs.customLogStatus[TPKTCHAN]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[UDPCHAN],debug)	==	0)
		server->cusconfigs.customLogStatus[UDPCHAN]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[PERERR],debug)	==	0)
		server->cusconfigs.customLogStatus[PERERR]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[CM],debug)	==	0)
		server->cusconfigs.customLogStatus[CM]	=	server->cusconfigs.level;

	if(strcmp(CustomLogs[CMAPICB],debug)	==	0)
		server->cusconfigs.customLogStatus[CMAPICB]	=	server->cusconfigs.level;

	if(strcmp(CustomLogs[CMAPI],debug)	==	0)
		server->cusconfigs.customLogStatus[CMAPI]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[CMERR],debug)	==	0)
		server->cusconfigs.customLogStatus[CMERR]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[LI],debug)	==	0)
		server->cusconfigs.customLogStatus[LI]	=	server->cusconfigs.level;
	if(strcmp(CustomLogs[LIINFO],debug)	==	0)
		server->cusconfigs.customLogStatus[LIINFO]	=	server->cusconfigs.level;


}
void PrintSerplex (serplex_config *serplex) {
  FILE *fp = stdout;

  fprintf(fp, "server type: %d\n", serplex->type);
  fprintf(fp, "server location type: %d\n", serplex->location.type);
  fprintf(fp, "server location address: 0x%x\n", serplex->location.address.sin_addr.s_addr);
  fprintf(fp, "priority: %d\n", serplex->prio);
  fprintf(fp, "max_endpts: %d\n", serplex->max_endpts);
  fprintf(fp, "max_gws: %d\n", serplex->max_gws);
  fprintf(fp, "daemonize: %d\n", serplex->daemonize);
  fprintf(fp, "threads: %d\n", serplex->threads);
  fprintf(fp, "threadstack: %d\n", serplex->threadstack);
  fflush(fp);
}

char *
FormatIpAddress(unsigned int ipAddress, char s[])
{
     sprintf(s, "%u.%u.%u.%u", 
	     (ipAddress & 0xff000000) >> 24,
	     (ipAddress & 0x00ff0000) >> 16,
	     (ipAddress & 0x0000ff00) >> 8,
	     (ipAddress & 0x000000ff));
     return s;
}

#if FCE_REMOVED
static int
PrintFcePrivateNet (int addr, int mask, int isPublic, void *passAlong1, void *passAlong2) {
  char addr1[32], addr2[32];
  FILE *fp = (FILE *)passAlong1;

  fprintf(fp, "\t\t%s \"%s/%s\"\n", (isPublic == TRUE)?"public":"private", FormatIpAddress(addr, addr1), FormatIpAddress(mask, addr2));
  return 0;
}
#endif

void
setProcessDebugLevels(FILE* fp, unsigned char* debugLevels){
  int i=0;
  int debug;

  for (i=0; i < MNETLOGMAX; i++) {
    debug = debugLevels[i];
    if (debug & NETLOG_DEBUG4)
		{
      fprintf(fp, "\tdebug %s 4\n", ModuleNames[i]);
		}
		else if (debug & NETLOG_DEBUG3)
		{
      fprintf(fp, "\tdebug %s 3\n", ModuleNames[i]);
		}
		else if (debug & NETLOG_DEBUG2)
		{
      fprintf(fp, "\tdebug %s 2\n", ModuleNames[i]);
		}
		else if (debug & NETLOG_DEBUG1)
		{
      fprintf(fp, "\tdebug %s 1\n", ModuleNames[i]);
		}
  }
}

int
GenerateCfgFile(char *progname, char *outf, char *hdr)
{
	FILE *fp;
	char addr[32], fwaddr[32];
	char command[512];
	int match = -1;
	int prevConfigServerType = myConfigServerType;
	serplex_config *serplexPtr = NULL;
	char *pStr;
	int i;

	myConfigServerType = CONFIG_SERPLEX_GIS;
	match = FindServerConfig();
	if (match != -1)
	  serplexPtr = &serplexes[match];
	myConfigServerType = prevConfigServerType;
/*
	if (serplexPtr != NULL) {
	  fprintf(stdout, "GIS serplex while generating cfg file:\n");
	  PrintSerplex(serplexPtr);
	}
*/
	sprintf(command, "cp -fp %s %s.prev", outf, outf);
	if (system(command))
	{
	    fprintf(stderr, "%s: could not create backup copy of %s\n", progname, outf);
	}

	if (!(fp = fopen(outf, "w")))
	{
		fprintf(stderr, "%s: could not open outfile %s\n", progname, outf);
		return -1;
	}

	fprintf(fp, "# Serplex Configuration File\n");
	if (hdr != NULL) {
	  fprintf(fp, hdr);
	  fprintf(fp, "\n");
	}
	fprintf(fp, "servers %d\n\n", max_servers);

#ifdef OLD_STYLE_FAX_
	fprintf(fp, "faxdirectory \"%s\"\n", fax_dir_pathname);
	fprintf(fp, "faxuser \"%s\"\n", fax_user);
	fprintf(fp, "faxpassword \"%s\"\n\n", fax_password);
#endif

	
	if (realTimeEnable)
	{
		fprintf(fp, "realtime\n");
	}

	// this line will get generated by default during initial install,
	// and as a result ./iserver all start will not work
	//	if (segowner == CONFIG_SERPLEX_GIS)
	//	{
	//		fprintf(fp, "segowner gis\n");
	//	}

	fprintf(fp, "\ndnsrecoverytimeout %d\n", dns_recovery_timeout);

	if (uccProto == CPROTO_SIP)
	{
		fprintf(fp, "uccproto sip\n");
	}
	else if (uccProto == CPROTO_H323)
	{
		fprintf(fp, "uccproto h323\n");
	}

	if (strlen(enumdomain))
	{
		fprintf(fp, "enumdomain \"%s\"\n\n", enumdomain);
	}

	if (g711Ulaw64Duration != 20)
	   fprintf(fp, "codec g711ulaw64k %d\n", g711Ulaw64Duration);
	if (g711Alaw64Duration != 20)
	   fprintf(fp, "codec g711alaw64k %d\n", g711Alaw64Duration);
	if (g729Frames != 2)
	   fprintf(fp, "codec g729 %d\n", g729Frames);
	if (g7231Frames != 1)
	   fprintf(fp, "codec g723.1 %d\n", g7231Frames);

  if( defaultCodec ==  CodecGPCMU)
    fprintf(fp,"default codec g711ulaw64k\n");
  else if(defaultCodec ==  CodecGPCMA)
    fprintf(fp,"default codec g711alaw64k\n");
  else if(defaultCodec ==  CodecG7231)
    fprintf(fp,"default codec g723.1\n");
  else if(defaultCodec ==  CodecG729)
    fprintf(fp,"default codec g729\n");


	fprintf(fp, "\n");

	if (max_segs != 6)
	  fprintf(fp, "segs %d\n", max_segs);
	if (max_segsize != 0x100000)
	  fprintf(fp, "segsize 0x%x\n\n", max_segsize);

	if (segaddrtype == 1) 
		fprintf (fp, "segaddrtype fixed\n");
	if (segaddr != NULL)
		fprintf (fp, "segaddr %u\n", (unsigned int)segaddr);

#ifdef _sipdomain
	if (strlen(sipdomain))
	{
		fprintf(fp, "sipdomain \"%s\"\n", sipdomain);
	}
#endif

	fprintf(fp, "mswname \"%s\"\n", mswname);

	fprintf(fp, "sipserver \"%s\"\n", sipservername);

	fprintf(fp, "sipport %d\n", sipport);

	if (sipservertype==SERVER_REDIRECT)
	{
		fprintf(fp, "sipserver redirect\n");
	}
	else if (sipservertype==SERVER_PROXY)
	{
		fprintf(fp, "sipserver proxy\n");
	}
	else 
	{
		fprintf(fp, "sipserver proxystateful\n");
	}

	if (sipauth == 1)
	{
		fprintf(fp, "sipauth local\n");
		fprintf(fp, "sipauthpassword \"%s\"\n", sipauthpassword);
	}
	else if (sipauth == 2)
	{
		fprintf(fp, "sipauth radius\n");
	}

	if (recordroute)
	{
	    fprintf(fp, "record-route enable\n");
	}

	fprintf(fp, "siptimer T1 %d\n", siptimerT1);
	fprintf(fp, "siptimer T2 %d\n", siptimerT2);
	fprintf(fp, "siptimer C %d\n", siptimerC);
	fprintf(fp, "sipminse %d\n", sipminSE);
	fprintf(fp, "sipsess  %d\n", sipsessionexpiry);
	fprintf(fp, "siptrans invitec %d\n", sipMaxInvRqtRetran);

	fprintf(fp, "sipmaxforwards %d\n", sipmaxforwards);

	if(siphold3264)
		fprintf(fp, "siphold3264 on\n");

	if(max_call_duration != 0)
	{
		fprintf(fp, "\nmaxcallduration %d\n", max_call_duration);
	}

	if(max_hunt_allowable_duration != 0)
	{
		fprintf(fp, "\nmaxhuntallowdur %d\n", max_hunt_allowable_duration);
	}

	if (strlen(prefix))
	{
	    fprintf(fp, "\nprefix \"%s\"\n", prefix);
	}
	fprintf(fp, "\n");

	/* h323 / sip */
	if (h323AdminStatus == 0)
	{
		fprintf(fp, "no h323\n");
	}

	if (sipAdminStatus == 0)
	{
		fprintf(fp, "no sip\n");
	}

//	fprintf(fp, "pm local {\n\tlog syslog\n#\tdebug modpmgr 4\n}\n\n");
	fprintf(fp, "pm local {\n\tlog syslog\n");

	myConfigServerType = CONFIG_SERPLEX_PM;
	match = FindServerConfig();

	if (match != -1)
	{
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}

	fprintf(fp,"}\n\n");

	myConfigServerType = prevConfigServerType;

	myConfigServerType = CONFIG_SERPLEX_GIS;

	match = FindServerConfig();

	fprintf(fp, "gis local {\n");

	fprintf(fp, "\tage {\n");

	if ((match != -1) &&
		(serplexes[match].age.cache_timeout != CACHE_TIMEOUT_DEFAULT))
	{
		fprintf(fp, "\t\ttimeout %d\n", serplexes[match].age.cache_timeout);
	}

	fprintf(fp, "\t}\n");

	fprintf(fp, "\tfce {\n");
	fprintf(fp, "# firewall name, \"NSF\" or \"none\" or \"MFCP\"\n");
	fprintf(fp, "\t\tfwname \"%s\"\n", (strcmp(fceConfigFwName, "ipfilter") == 0)?"NSF":fceConfigFwName);

       if (fceFirewallAddresses[0] == 0) {
		inet_pton(AF_INET, HKADDR, &fceFirewallAddresses[0]);
		fceFirewallAddresses[0] = ntohl( fceFirewallAddresses[0] );
	}
	FormatIpAddress(fceFirewallAddresses[0], fwaddr);
        fprintf(fp, "\t\tfwaddress %s\n", fwaddr);

#if FCE_REMOVED
	fprintf(fp, "# the ip address to receive the incoming signalling traffic\n");

	FormatIpAddress(fceConfigOurIpAddr, addr);

	fprintf(fp, "\t\tfwconnectaddr %s\n", addr);
	fprintf(fp, "# open dynamic pinholes for H.245 signaling, \"on\", or \"off\"\n");
	fprintf(fp, "\t\th245 %s\n", (fceH245PinholeEnabled == TRUE)?"on":"off");
	fprintf(fp, "# by default all addresses will be considered \"public\" or \"private\"\n");
	fprintf(fp, "\t\tdefault %s\n", (fceDefaultPublic == TRUE)?"public":"private");

	fprintf(fp, "# networks declared \"public\" or \"private\"\n"
				"# if an address matches a \"private\" space, no FCE actions will be taken\n"
				"#\t\tprivate \"10.0.0.0/255.255.0.0\"\n"
				"#\t\tpublic \"10.0.1.0/255.255.255.0\"\n" );

	FCEListNetworkList( PrintFcePrivateNet, fp, NULL );
	fprintf(fp, "# media routing for endpoints not in the database\n\t\tdefault mediarouting %s\n", defaultMediaRouting?"on":"off");
	fprintf(fp, "# mid call media address change for endpoints not in the database\n\t\tdefault midcallmediachange %s\n", defaultHideAddressChange?"on":"off");
#endif
	fprintf(fp, "\t}\n");

		fprintf( fp,	"\n"
						"\tpeering_config {\n" );

		if ( ispd_type == ISPD_TYPE_ACTIVE )
		{
			fprintf( fp,	"\t\tserver_type \"active\"\n" );
		}
		else
		if ( ispd_type == ISPD_TYPE_STANDBY )
		{
			fprintf( fp,	"\t\tserver_type \"standby\"\n" );
		}
		else
		{
			fprintf( fp,	"\t\tserver_type \"disabled\"\n" );
		}

		if ( ispd_ctl.defined )
		{
			int i;

			fprintf( fp,	"\t\tcontrol_interface {\n" );

			fprintf( fp,	"\t\t\tinterface_name      \"%s\"\n",
							ispd_ctl.name );

			for ( i = 0; i < ispd_ctl.peer_count; i++ )
			{
				fprintf( fp,	"\t\t\tpeer_iserver        \"%s\"\n",
								&ispd_ctl.peer_iservers[i][0] );
			}

			fprintf( fp,	"\t\t}\n" );
		}

		fprintf(fp, "\tscm %s\n", doScm?"on":"off");

		fprintf( fp,	"\t}\n"
						"\n" );

//	fprintf(fp, "\tpolicy none\n\tslog syslog\n"
//				"\tlog syslog\n#\tdebug modfce 4\n"
//				"#\tdebug modreg 4\n"
//				"#\tdebug modfind 4\n"
//				"#\tdebug modredund 4\n"
//				"%s\tdebug modcdr 4\n"
//				"#\tdebug modh323 4\n"
//				"#\tdebug modscc 4\n"
//				"#\tdebug modsip 4\n"
//				"#\tdebug modarq 4\n"
//				"#\tdebug modrrq 4\n"
//				"#\tdebug modlrq 4\n"
//				"\tdebug modiwf 4\n"
//				"\tdebug modbridge 4\n"
//				"\tdebug modfce 4\n",
//				(logCdr?"":"#"));

	fprintf(fp,	"\tpolicy none\n"
				"\tslog syslog\n"
				"\tlog syslog\n" );

	if (match != -1)
	{
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
		if(updateAllocations == 0)
			fprintf(fp,"\tno update allocations \n");

		if(	serplexes[match].cusconfigs.level >= 3){
			fprintf(fp,"\thdebug level %d\n",serplexes[match].cusconfigs.level);
			if(serplexes[match].cusconfigs.customLogStatus[TPKTCHAN] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[TPKTCHAN]);
			if(serplexes[match].cusconfigs.customLogStatus[UDPCHAN] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[UDPCHAN]);
			if(serplexes[match].cusconfigs.customLogStatus[PERERR] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[PERERR]);
			if(serplexes[match].cusconfigs.customLogStatus[CM] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[CM]);
			if(serplexes[match].cusconfigs.customLogStatus[CMAPICB] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[CMAPICB]);
			if(serplexes[match].cusconfigs.customLogStatus[CMAPI] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[CMAPI]);
			if(serplexes[match].cusconfigs.customLogStatus[CMERR] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[CMERR]);
			if(serplexes[match].cusconfigs.customLogStatus[LI] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[LI]);
			if(serplexes[match].cusconfigs.customLogStatus[LIINFO] >0)
				fprintf(fp,"\thdebug \"%s\"\n",CustomLogs[LIINFO]);

		}
		if(serplexes[match].cusconfigs.slevel >= 3) {
		   fprintf(fp,"\tsdebug level %d\n", serplexes[match].cusconfigs.slevel);
                }
	}

	myConfigServerType = prevConfigServerType;	
	
	if (serplexPtr != NULL)
	{
		fprintf(fp, "\tpriority %d\n", serplexPtr->prio);

		if (serplexPtr->threads != 5)
		{
			fprintf(fp, "\tthreads tsm %d\n", serplexPtr->threads);
		}
	}
	else
	{
		fprintf(fp, "\tpriority -10\n");
	}

	if (nBridgeThreads != 10)
	{
		fprintf(fp, "\tthreads bridge %d\n", nBridgeThreads);
	}

	if (nCallIdThreads != 2)
	{
		fprintf(fp, "\tthreads ua %d\n", nCallIdThreads);
	}

	if (nConfIdThreads != 1)
	{
		fprintf(fp, "\tthreads iwf %d\n", nConfIdThreads);
	}

	if (nRadiusThreads != 1)
	{
		fprintf(fp, "\tthreads radius %d\n", nRadiusThreads);
	}

	if (billingType == BILLING_PREPAID)
	{
		fprintf(fp, "\tbilling prepaid\n");
	}
	else if (billingType == BILLING_POSTPAID)
	{
		fprintf(fp, "\tbilling postpaid\n");
	}
	else if (billingType == BILLING_CISCOPREPAID)
	{
		fprintf(fp, "\tbilling ciscoprepaid {\n");
		fprintf(fp, "\t\tfirstauth \"%s\" \"%s\"\n", first_auth_username, first_auth_password);
		fprintf(fp, "\t\tsecondauth \"%s\" \"%s\"\n", second_auth_username, second_auth_password);
		fprintf(fp, "\t}\n");
	}
	else if (billingType == BILLING_NONE)
	{
		fprintf(fp, "\tbilling none\n");
	}

	switch(cdrformat)
	{
		case CDRFORMAT_XML:
			fprintf(fp, "\tcdr %s", "xml");
			break;

		case CDRFORMAT_MIND:
			fprintf(fp, "\tcdr %s", "mindcti");
			break;

		case CDRFORMAT_SYSLOG:
			fprintf(fp, "\tcdr %s", "syslog");
			break;
	}

	switch (cdrtype)
	{
	case CDRMINDCTIFIXED:
		fprintf(fp, " fixed");
		break;
	case CDRMINDCTIDAILY:
		fprintf(fp, " daily");
		break;
	case CDRNEXTONETIME:
		fprintf(fp, " time");
		break;
	case CDRMINDCTISEQ:
		fprintf(fp, " seq");
		break;
	}

	fprintf(fp, " {\n");
	fprintf(fp, "\t\tdir \"%s\"\n", cdrdirname);
               fprintf(fp, "\t\tcdrtimer %d\n",cdrtimer);
	if (cdrevents & ~CDREND1)
	{
		fprintf(fp, "\t\t");
		fprintf(fp, "%s", cdrevents&CDRSTART1?"start1 ":"");
		fprintf(fp, "%s", cdrevents&CDRSTART2?"start2 ":"");
		fprintf(fp, "%s", cdrevents&CDREND2?"end2 ":"");
		fprintf(fp, "%s", cdrevents&CDRHUNT?"hunt ":"");
		fprintf(fp, "\n");
	}

    fprintf(fp,"\t}\n");

	if (allowDestArq)
	{
		fprintf (fp, "\tallow destarq all\n");
	}
	if (allowAuthArq)
	{
		fprintf (fp, "\tallow autharq all\n");
	}

	fprintf(fp, "\tcp {\n");
	if (allowRtpAll)
	{
		fprintf(fp, "\t\tallow rtp all\n");
	}
	else
	{
		fprintf(fp, "\t\tno allow rtp all\n");
	}
	if (allowSrcAll)
	{
		fprintf(fp, "\t\tallow src all\n");
	}
	if (allowDestAll)
	{
		fprintf(fp, "\t\tallow dest all\n");
	}
	if (forwardSrcAddr)
	{
		fprintf(fp, "\t\tforward src addr\n");
	}
	if (nlruRoutes != 500)
	{
		fprintf(fp, "\t\tlru route %d\n", nlruRoutes);
	}
	if (allowANIPortSel)
	{
		fprintf(fp, "\t\tallow ani portsel\n");
	}

	if (allowDNISPortSel)
	{
		fprintf(fp, "\t\tallow dnis portsel\n");
	}

   	fprintf(fp, "\t\tusecodemap \"%s\"\n", codemaptemplate);

	fprintf(fp, "\t}\n");

	if (obpEnabled)
	{
		fprintf(fp, "\tobp {\n");

		if (allowInternalCalling)
		{
			fprintf(fp, "\t\tallow internalcalling\n");
		}
		if (allowDynamicEndpoints)
		{
			fprintf(fp, "\t\tallow dynamicendpoints\n");
		}
		if (enableNatDetection)
		{
			fprintf(fp, "\t\tenable natdetection\n");
		}

		fprintf(fp, "\t}\n");
	}

	if (strlen(gkid))
	    fprintf(fp, "\tgkid \"%s\"\n", gkid);
	if (rrqtimer != 30)
	    fprintf(fp, "\trrqtimer %d\n", rrqtimer);
	if (recoveryOnTimerExpiry)
	    fprintf(fp, "\ttcpUnreachCC %d\n", recoveryOnTimerExpiry);
#if 0	// all timeouts will be based on the cacheTimeout
	if (rrqttl != RRQTTL_DEFAULT)
	    fprintf(fp, "\trrqttl %d\n", rrqttl);
#endif
	//fprintf(fp, "\t%sforce h245\n", forceh245?"":"no ");
	fprintf(fp, "\t%sroute h245\n", routeH245?"":"no ");
	fprintf(fp, "\t%sroute call\n", routecall?"":"no ");
	fprintf(fp, "\tfaststart %s\n", doFastStart?"on":"off");
	fprintf(fp, "\t%sh245 tunneling\n", h245Tunneling?"":"no ");
	if(always2833)
		fprintf(fp, "\talways2833 on\n");
	if(mapisdncc)
		fprintf(fp, "\tmapisdncc enable\n");
	if(maplrjreason)
		fprintf(fp, "\tmaplrjreason enable\n");
	if(fsInConnect)
    	fprintf(fp, "\tfsinconnect enable\n");
	fprintf(fp, "\tgetanifromacf %s\n", getAniFromAcf?"enable":"disable");
	fprintf(fp, "\t%slocal proceeding\n", localProceeding?"":"no ");
	fprintf(fp, "\th323 instance %d\n", nh323CfgInstances);
	fprintf(fp, "\th323 maxcalls %d\n", h323maxCalls);
	fprintf(fp, "\th323 maxcalls sgk %d\n", h323maxCallsSgk);
	fprintf(fp, "\th323 maxrasbuffsize %d\n", h323RasMaxBuffSize);
	pStr = enum2str(infoTransCapOptions, h323infoTransCap);
	fprintf(fp, "\th323 infoTransCap \"%s\"\n", pStr == NULL ? "pass" : pStr );
	fprintf(fp, "\th323 removetcst38 %s\n", h323RemoveTcsT38?"enable":"disable");
	fprintf(fp, "\th323 removetcs2833 %s\n", h323RemoveTcs2833?"enable":"disable");

	if ((h323maxCallsPadFixed != H323MAXCALLSPADF) || 
			(h323maxCallsPadVariable != H323MAXCALLSPADV))
	{
		fprintf(fp, "\th323 maxcalls pad %d %d\n", h323maxCallsPadFixed, h323maxCallsPadVariable);
	}

	fprintf(fp, "\th323cps %d\n", h323Cps);
	fprintf(fp, "\tlocal-isdn-cc {\n\t\tno-route %d sip %d\n", getH323Code(SCC_errorNoRoute), getSipCode(SCC_errorNoRoute));
	fprintf(fp, "\t\tno-ports %d sip %d\n\t}\n", getH323Code(SCC_errorNoPorts), getSipCode(SCC_errorNoPorts));
	if(!allowHairPin)
	{
		fprintf(fp, "\tno hairpin\n");
	}

	fprintf(fp, "\trollovertime %d\n", rolloverTime);
	fprintf(fp, "\tcallstate none\n");

	fprintf(fp, "\tmaxhunts %d\n", maxHunts);

	fprintf(fp, "\tcrid %d\n", iservercrId);
	fprintf(fp, "\tcrid %s\n", crids?"on":"off");

	if (!localReInviteNoSdp) 
		fprintf(fp, "\tno ");
	fprintf(fp, "local reinvite-no-sdp\n");

	// print the altgk list
	if (altGkList)
	{
		struct sockaddr_in *saddr;

		fprintf(fp, "\taltgk {\n ");
		for (saddr = listGetFirstItem(altGkList);
				saddr; saddr = listGetNextItem(altGkList, saddr))
		{
			fprintf(fp, "\t\tgk %s/%d\n", 
				FormatIpAddress(saddr->sin_addr.s_addr, addr),
				saddr->sin_port);
		}
		fprintf(fp, "\t}\n");
	}
	
	if (strlen(rad_server_addr[0]) > 0)
	{
		fprintf(fp, "\tradius {\n");
		for(i = 0; i < MAX_NUM_RAD_ENTRIES; ++i)
		{
			if(strlen(rad_server_addr[i]))
			{
				fprintf(fp, "\t\tradserver \"%s\"\n", rad_server_addr[i]);
			}
			if(strlen(secret[i]))
			{
				fprintf(fp, "\t\tradsecret \"%s\"\n", secret[i]);
			}
		}

		fprintf(fp, "\t\tradtimeout %d\n", rad_timeout);
		fprintf(fp, "\t\tradretries %d\n", rad_retries);
		fprintf(fp, "\t\traddeadtime %d\n", rad_deadtime);

		fprintf(fp, "\t\tradacct %s\n", rad_acct ? "on" : "off");
		fprintf(fp, "\t\tdir \"%s\"\n", rad_dirname);
		if(rad_acct_session_id_overloaded)
		{
			fprintf(fp, "\t\tacctsessionid overloaded\n");
		}
		fprintf(fp, "\t}\n");
	}

	fprintf(fp, "}\n\n");

	if (gisrip)
	{
		struct in_addr inaddr;
                char buf[INET_ADDRSTRLEN];

		inaddr.s_addr = gisrip;
		fprintf(fp, "#Redundant Server Configuration\n");
//		fprintf(fp, "gis %s {\n\tage {\n\t}\n\tpriority -10\n\tlog syslog\n#\tdebug modreg 4\n#\tdebug modfind 4\n#\tdebug modredund 4\n#\tdebug modcdr 4\n}\n\n", inet_ntoa(inaddr));
                fprintf(fp, "gis %s {\n\tage {\n\t}\n\tlog syslog\n", inet_ntop(AF_INET, &inaddr, buf, INET_ADDRSTRLEN));
                myConfigServerType = CONFIG_SERPLEX_GIS;
                match = FindServerConfig();
                if (match != -1){
                  setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
                }
                fprintf(fp,"}\n\n");
                myConfigServerType = prevConfigServerType;
	}

//	fprintf(fp, "bcs local {\n\tlog syslog\n%s\tdebug modcdr 4\n}\n\n", (logIntCdr?"":"#"));
	fprintf(fp, "bcs local {\n\tlog syslog\n");
	myConfigServerType = CONFIG_SERPLEX_BCS;
	match = FindServerConfig();
	if (match != -1){
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}
	fprintf(fp,"}\n\n");
	myConfigServerType = prevConfigServerType;

	fprintf(fp, "jserver local {\n\tlog syslog\n\tjserverlogfile \"%s\"\n\tjserverloglevel \"%s\"\n\tread_permission \"%s\"\n\twrite_permission \"%s\"\n\tcompression %s\n", jsLogFile, jsLogLevel, readPass, writePass, jsCompression?"on":"off");
	myConfigServerType = CONFIG_SERPLEX_JSERVER;
	match = FindServerConfig();
	if (match != -1){
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}
	fprintf(fp,"}\n\n");
	myConfigServerType = prevConfigServerType;


//	fprintf(fp, "faxserver local {\n\ttimeout 300\n\tmaxretransmitcount 10\n\n\tlog syslog\n#\tdebug modfaxp 4\n#\tdebug modinit 4\n}\n\n");
	fprintf(fp, "faxserver local {\n\ttimeout 300\n\tmaxretransmitcount 10\n\n\tlog syslog\n");
	myConfigServerType = CONFIG_SERPLEX_FAXS;
	match = FindServerConfig();
	if (match != -1){
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}
	fprintf(fp,"}\n\n");
	myConfigServerType = prevConfigServerType;

/* RSD related output */
	fprintf( fp,	"\n" "rsd ");

	if (RSDConfig == CONFIG_LOCATION_NONE) 
		fprintf( fp,	"none");
	else
		fprintf( fp,	"local");
		
	fprintf( fp,	" {\n" );

	fprintf( fp,	"\trs_ifname\t\t\"%s\"\n", rs_ifname );

	fprintf( fp,	"\trs_mcast_addr\t\"%s\"\n", rs_mcast_addr );

	fprintf( fp,	"\trs_port\t\t\t\"%s\"\n", rs_port );

	fprintf( fp,	"\trs_host_prio\t %d\n", rs_host_prio );

	fprintf( fp,	"\thistdb_size\t %d\n", histdb_size );

	if (rs_cp_cmd_str_fl)
		fprintf( fp,	"\trs_cp_cmd_str\t %s\n", rs_cp_cmd_str );

	if (rs_tmp_dir_fl)
		fprintf( fp,	"\trs_tmp_dir\t\t %s\n", rs_tmp_dir );

	if (rs_ssn_int_fl)
		fprintf( fp,	"\trs_ssn_int\t\t %d\n", rs_ssn_int );

	fprintf( fp,    "\tlog syslog\n");



	myConfigServerType = CONFIG_SERPLEX_RSD;
	match = FindServerConfig();
	if (match != -1) {
		if (serplexes[match].debconfigs.flags & NETLOG_ASYNC) 
			fprintf( fp,	"\tlog\t\t\t\tsyslog\n");
		if (serplexes[match].debconfigs.flags & NETLOG_TERMINAL) 
			fprintf( fp,	"\tlog\t\t\t\tterminal\n");
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}
	myConfigServerType = prevConfigServerType;

	fprintf( fp,	"}\n\n");

/* ExecD related output */
	fprintf( fp,	"\n" "execd ");

	if (ExecDConfig == CONFIG_LOCATION_NONE) 
		fprintf( fp,	"none");
	else
		fprintf( fp,	"local");
		
	fprintf( fp,	" {\n" );

	myConfigServerType = CONFIG_SERPLEX_EXECD;
	match = FindServerConfig();
	if (match != -1) {
		if (serplexes[match].debconfigs.flags & NETLOG_ASYNC) 
			fprintf( fp,	"\tlog\t\t\t\tsyslog\n");
		if (serplexes[match].debconfigs.flags & NETLOG_TERMINAL) 
			fprintf( fp,	"\tlog\t\t\t\tterminal\n");
		setProcessDebugLevels(fp,serplexes[match].debconfigs.netLogStatus);
	}
	myConfigServerType = prevConfigServerType;

	fprintf( fp,	"}\n\n");

/* management interface related */
	if( strlen(mgmtInterfaceIp) )
	{
		fprintf( fp, "\nmgmt_interface {\n" );
		fprintf( fp, "\tmgmt_ip\t\"%s\"\n", mgmtInterfaceIp );
		fprintf( fp, "}\n" );
	}

	fflush(fp);
	fclose(fp);

	return 0;
}

/* string and enum mapping array */
/* has to ended with { NULL, .. } */
enumString infoTransCapOptions[] = {
	{ "speech", INFO_TRANSCAP_SPEECH },
	{ "unrestricted", INFO_TRANSCAP_UNRESTRICTED },
	{ "restricted", INFO_TRANSCAP_RESTRICTED },
	{ "audio", INFO_TRANSCAP_AUDIO },
	{ "unrestrictedtones", INFO_TRANSCAP_UNRESTRICTEDTONES },
	{ "video", INFO_TRANSCAP_VIDEO },
	{ "pass", INFO_TRANSCAP_PASS },
	{ "default", INFO_TRANSCAP_DEFAULT },
	{ NULL, -1 }	/* number here is for invalid number */
};

/*
str2enum() -- to convert string to enum from enumString[] array 
 returns invalid enum, if not found. the string is NOT casesenstive 
*/
int
str2enum( enumString *enumStrArray,  char *str )
{
	int i;
	for (i=0; enumStrArray[i].str != NULL; i++)
	{
		if ( !strcasecmp( enumStrArray[i].str, str ) )
		{
			return enumStrArray[i].number;
		}
	}
	return enumStrArray[i].number;	/* the last one indicates "not found" */
}

/* enum2str() -- to convert enum to string from enumStrin[] array
	returns NULL is not found.
*/
char * 
enum2str( enumString *enumStrArray,  int number )
{
	int i;
	for (i=0; enumStrArray[i].str != NULL; i++)
	{
		if ( enumStrArray[i].number ==  number )
		{
			return enumStrArray[i].str;
		}
	}
	return NULL;
}

int errorStr2CallError(char *errStr)
{
	if(strcmp(errStr, "abandoned") == 0)
	{
		return SCC_errorAbandoned;
	}
	else if(strcmp(errStr, "user-blocked") == 0)
	{
		return SCC_errorBlockedUser;
	}
	else if(strcmp(errStr, "user-blocked-at-dest") == 0)
	{
		return SCC_errorDestBlockedUser;
	}
	else if(strcmp(errStr, "no-route") == 0)
	{
		return SCC_errorNoRoute;
	}
	else if(strcmp(errStr, "no-route-at-dest") == 0)
	{
		return SCC_errorDestNoRoute;
	}
	else if(strcmp(errStr, "busy") == 0)
	{
		return SCC_errorBusy;
	}
	else if(strcmp(errStr, "resource-unavailable") == 0)
	{
		return SCC_errorResourceUnavailable;
	}
	else if(strcmp(errStr, "invalid-phone") == 0)
	{
		return SCC_errorInvalidPhone;
	}
	else if(strcmp(errStr, "network-error") == 0)
	{
		return SCC_errorNetwork;
	}
	else if(strcmp(errStr, "no-ports") == 0)
	{
		return SCC_errorNoPorts;
	}
	else if(strcmp(errStr, "general-error") == 0)
	{
		return SCC_errorGeneral;
	}
	else if(strcmp(errStr, "dest-unreach") == 0)
	{
		return SCC_errorDestinationUnreachable;
	}
	else if(strcmp(errStr, "undefined") == 0)
	{
		return SCC_errorUndefinedReason;
	}
	else if(strcmp(errStr, "no-bandwidth") == 0)
	{
		return SCC_errorInadequateBandwidth;
	}
	else if(strcmp(errStr, "h245-error") == 0)
	{
		return SCC_errorH245Incomplete;
	}
	else if(strcmp(errStr, "incomp-addr") == 0)
	{
		return SCC_errorIncompleteAddress;
	}
	else if(strcmp(errStr, "local-disconnect") == 0)
	{
		return SCC_errorLocalDisconnect;
	}
	else if(strcmp(errStr, "h323-internal") == 0)
	{
		return SCC_errorH323Internal;
	}
	else if(strcmp(errStr, "h323-proto") == 0)
	{
		return SCC_errorH323Protocol;
	}
	else if(strcmp(errStr, "no-call-handle") == 0)
	{
		return SCC_errorNoCallHandle;
	}
	else if(strcmp(errStr, "gw-resource-unavailable") == 0)
	{
		return SCC_errorGatewayResourceUnavailable;
	}
	else if(strcmp(errStr, "fce-error-setup") == 0)
	{
		return SCC_errorFCECallSetup;
	}
	else if(strcmp(errStr, "fce-error") == 0)
	{
		return SCC_errorFCE;
	}
	else if(strcmp(errStr, "fce-no-vports") == 0)
	{
		return SCC_errorNoFCEVports;
	}
	else if(strcmp(errStr, "no-vports") == 0)
	{
		return SCC_errorNoVports;
	}
	else if(strcmp(errStr, "h323-maxcalls") == 0)
	{
		return SCC_errorH323MaxCalls;
	}
	else if(strcmp(errStr, "msw-invalid-epid") == 0)
	{
		return SCC_errorMswInvalidEpId;
	}
	else if(strcmp(errStr, "msw-routecallgk") == 0)
	{
		return SCC_errorMswRouteCallToGk;
	}
	else if(strcmp(errStr, "msw-notreg") == 0)
	{
		return SCC_errorMswCallerNotRegistered;
	}
	else if(strcmp(errStr, "hairpin") == 0)
	{
		return SCC_errorHairPin;
	}
	else if(strcmp(errStr, "shutdown") == 0)
	{
		return SCC_errorShutdown;
	}
	else if(strcmp(errStr, "switchover") == 0)
	{
		return SCC_errorSwitchover;
	}
	else if(strcmp(errStr, "disconnnect-unreach") == 0)
	{
		return SCC_errorDisconnectUnreachable;
	}
	else if(strcmp(errStr, "dest-relcomp") == 0)
	{
		return SCC_errorDestRelComp;
	}
	else if(strcmp(errStr, "temporarily-unavailable") == 0)
	{
		return SCC_errorTemporarilyUnavailable;
	}
	else if(strcmp(errStr, "dest-gone") == 0)
	{
		return SCC_errorDestGone;
	}
	else if(strcmp(errStr, "dest-timeout") == 0)
	{
		return SCC_errorDestTimeout;
	}
	else if(strcmp(errStr, "reject-route") == 0)
	{
		return SCC_errorRejectRoute;
	}
	else if(strcmp(errStr, "no-nat-t-license") == 0)
	{
		return SCC_errorNoNATTLicense;
	}
	return -1;
}

