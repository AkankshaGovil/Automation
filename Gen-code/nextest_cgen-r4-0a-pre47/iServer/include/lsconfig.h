#ifndef _ls_config_h_
#define _ls_config_h_

#include "list.h"
#include "ipc.h"
#include "ispd_common.h"
#include "rsd_common.h"
#ifndef  NO_AVL_NEEDED
#include "hello_common.h"
#endif
#include "codecs.h"
#include "nxosdtypes.h"

typedef struct
{
	int type;
#define CONFIG_LOCATION_NONE	0
#define CONFIG_LOCATION_LOCAL	1
#define CONFIG_LOCATION_INADDR	2

	struct sockaddr_in address;
} location_config;

#define HKADDR  "169.254.0.2" /* IP Addr to talk to HK card */

typedef struct
{
	location_config location;
	int cache_timeout;
	int prio;
	int sleep_time;
} age_config;

typedef struct
{
	location_config location;
} ispd_config;

typedef struct
{
	location_config location;
} rsd_config;

typedef struct
{
	int dispatch_timeout;
	int max_retransmit_count;
} fax_config;

typedef struct
{
	unsigned char netLogStatus[MNETLOGMAX];
	unsigned short          flags;
} debug_configs;

typedef struct
{
	unsigned short customLogStatus[CUSLOGMAX];
	unsigned short level;
        unsigned short slevel;
} custom_configs;

typedef struct
{
	int type;
#define CONFIG_SERPLEX_NONE	0
#define CONFIG_SERPLEX_LUS	1
#define CONFIG_SERPLEX_VPNS	2
#define CONFIG_SERPLEX_BCS	3
#define CONFIG_SERPLEX_JSERVER	4
#define CONFIG_SERPLEX_FAXS	5
#define CONFIG_SERPLEX_GIS	7
#define	CONFIG_SERPLEX_PM	8
#define	CONFIG_SERPLEX_CLI	9
#define	CONFIG_SERPLEX_RSD	10
#define CONFIG_SERPLEX_EXECD 11

// one more than the last config
#define CONFIG_SERPLEX_MAXTYPES	12

	/* Linked list of config items follows,
	 * --- will add later on... 
	 */
	location_config location;

	age_config age;
	ispd_config ispd;
	fax_config fax;

	unsigned short flags; 
#define CONFIG_SERPLEX_REDUNDF	0x0001

	int prio;
	
	int max_endpts;
	int max_gws;
	int daemonize;
	int threads;
	int threadstack;

	debug_configs debconfigs;
  custom_configs cusconfigs; 
} serplex_config;

#define CONFIG_SERPLEX_MAX 10
extern serplex_config *serplexes;
extern serplex_config *redunds;
extern serplex_config *iserver;
extern int max_servers;
extern int max_endpts;
extern int max_gws;
extern char gkid[];
extern char codemapfilename[];
extern char codemaptemplate[];
extern char sipservername[];
extern char mswname[];
extern int sipport;
extern unsigned int rrqttl, callttl;
extern int rrqtimer;
extern unsigned int routeH245;
extern int localProceeding;
extern int h245Tunneling;
extern int routecall;
extern int recordroute;
extern int sharecall;
extern int defaultrealm;
extern int allowHairPin;
extern	char	fax_dir_pathname[];
extern	char	fax_user[];
extern	char	fax_password[];
extern	char	sipdomain[];
extern 	char	prefix[];

extern struct ifi_info *ifihead;
extern int myConfigServerType;
extern int inserver;
extern int max_servers;

extern char jsLogFile[];
extern char jsLogLevel[];
extern char readPass[];
extern char writePass[];
extern int jsCompression;

extern int nopolicy;
extern int nocallstate;
extern int max_segs;
extern int max_segsize;
extern int segaddrtype;
extern void * segaddr;
extern int segowner;

extern char 	cdrdirname[];
extern int		cdrtype;
extern int		cdrformat;
extern int		cdrtimer;
extern unsigned int	cdrevents;

extern int clwp;

extern int	doScm;

extern int 	doFastStart;

extern int 	always2833;

extern int 	mapisdncc;

extern int 	maplrjreason;

extern int  fsInConnect;

extern int  getAniFromAcf;

extern int 	siphold3264;

extern int	max_ccalls;

extern int  billingType;

extern char first_auth_username[];
extern char first_auth_password[];
extern char second_auth_username[];
extern char second_auth_password[];

extern int  rolloverTime;

extern int     	(*_msAdd)(char *), (*_msDelete)(char *), 
				(*_msSetDebugLevel)(int), (*_msDeleteAll)(void);
extern int		(*_sipSetTraceLevel)(int);

extern int		(_dummysipSetTraceLevel)(int);
extern int     	(_dummymsAdd)(char *), (_dummymsSetDebugLevel)(int);

#define CPROTO_UCC	0
#define CPROTO_H323	1
#define CPROTO_SIP	2
extern int uccProto;
extern int h323AdminStatus, sipAdminStatus;

extern int sipauth;
extern char sipauthpassword[];
extern int	sipservertype;
extern int	sipmaxforwards;
extern int   sipminSE;
extern int   sipsessionexpiry;


#define MAX_NUM_RAD_ENTRIES 2
extern char rad_server_addr[MAX_NUM_RAD_ENTRIES][RADSERVERADDR_LEN];
extern char secret[MAX_NUM_RAD_ENTRIES][SECRET_LEN];
extern int rad_timeout;
extern int rad_retries;
extern int rad_deadtime;
extern int rad_acct;
extern int rad_acct_session_id_overloaded;
extern int rad_failed_timeout;
extern char rad_dirname[];

extern char enumdomain[];
extern char e911loc[];
extern char tapcall[];
extern char untapcall[];

#define SERVER_PROXY	0
#define SERVER_REDIRECT	1
#define SERVER_PROXYSTATEFULL	2

extern int idaemon;
extern int xthreads, nthreads, threadstack;
extern int		updateAllocations;
extern int useXConnId;

extern char fceConfigFwName[];
extern unsigned int fceFirewallAddresses[];

extern ispd_server_type_t	ispd_type;
extern ispd_interface_t		ispd_primary;
extern ispd_interface_t		ispd_secondary;
extern ispd_ctl_interface_t	ispd_ctl;

extern int g711Ulaw64Duration;
extern int g711Alaw64Duration;
extern int g729Frames;
extern int g7231Frames;
extern CodecType defaultCodec;


extern int GenerateCfgFile (char*, char*, char*);
extern unsigned long gisrip;
extern int PeeringConfig;
extern int logCdr;
extern int logIntCdr;

extern int siptimerT1, siptimerT2, siptimerC;
extern int sipMaxInvRqtRetran;

extern longlong_t class1deadline, class2deadline;

extern char* FormatIpAddress(unsigned int, char*);
extern List altGkList;
extern int 	bridgeH323QSize;

extern int 	realTimeEnable;

extern int 	allowDestAll;
extern int  allowDestArq;
extern int  allowAuthArq;
extern int 	allowSrcAll;
extern int	cpRoutingPolicy;
extern int	allowRtpAll;

extern int	obpEnabled;
extern int	allowInternalCalling;
extern int	allowDynamicEndpoints;
extern int	enableNatDetection;

extern int	nprocs;

#define H245_DEFAULT	1	

extern int cacheTimeout;
extern int routeDebug;
extern int forceh245;
extern int recoveryOnTimerExpiry;

extern int defaultMediaRouting;
extern int defaultHideAddressChange;

extern int nRadiusThreads;
extern int nBridgeThreads;
extern int nCallIdThreads;
extern int nConfIdThreads;
extern int nH323Threads;
extern int nh323Instances;
extern int nh323CfgInstances;
extern int h323Cps;
extern int h323QLen;
extern int h323maxCalls, h323maxCallsSgk, h323RasMaxBuffSize;
extern int 	h323infoTransCap;
#define	H323MAXCALLSPADF	200
#define H323MAXCALLSPADV	80
extern int h323maxCallsPadFixed, h323maxCallsPadVariable;
extern int cdrcallidlen;
extern int h323RemoveTcs2833;
extern int h323RemoveTcsT38;

// cause code related section
int errorStr2CallError(char *errStr);
int getSipCode(int callErrorCode);
int getH323Code(int callErrorCode);
extern int causeCodes[][];

// We must have a system level max, even though there
// is no resource level limitations. This is to prevent
// the user from killing the box
#define SYSTEM_MAX_HUNTS		50
extern int maxHunts;	// Default system wide max provisioned by the user
extern int iservercrId;
extern int crids;

extern int forwardSrcAddr;
extern int     trackVports, trackMaxCalls;
extern int	h323GracefulShutdown;	// Send Unregistrations at shutdown
extern int	nlruRoutes;

extern int max_call_duration;

extern int max_hunt_allowable_duration;

extern int dns_recovery_timeout;

extern int allowANIPortSel, allowDNISPortSel;

extern int huntAllCauseCodes;

extern int localReInviteNoSdp;

extern char mgmtInterfaceIp[];

#endif /* _ls_config_h_ */
