#ifdef RPC_HDR
%#include "ipc.h"
%#include "key.h"
%#include "serverdb.h"
#endif

/* All Database storage entries are defined in this file so that
 * RPC and rest of the cli have same concept for these structures.
 * All Future changes to these structures should be made in this file only.
 */

/*
 * CACHE COMMAND HANDLER Specification
 * Cache will be part of gis process and cli has to write into 
 * Cache for making changes known to a running system
 */

/* Info entries in the database must be stored in network
 * byte order. In the cache these entries are stored in network
 * byte order also.
 */
enum Vendor_e{
		Vendor_eGeneric,
		Vendor_eClarent,
		Vendor_eSonusGSX,
		Vendor_eBroadsoft,
		Vendor_eCisco,
		Vendor_eVocalTec,
		Vendor_eLucentTnt,
		Vendor_eExcel
} ;
typedef enum  Vendor_e	Vendor;
		
/* Database storage data structure for iedge db. */
struct info_entry {
     	/* Global information about the netoid */
     	char              	regid[REG_ID_LEN];		/* leave as first two entries */
     	unsigned long     	uport;

     	IPaddr            	ipaddress;

		/* PHONE of this iedge (derived from vpnPhone, vpn) */
     	char              	phone[PHONE_NUM_LEN]; 

		/* VPN this iedge belongs to */
		char				vpnName[VPNS_ATTR_LEN];

     	char 				vpnPhone[VPN_LEN]; 	
     	unsigned long 		vpnExtLen;

     	unsigned short     	sflags;
     	unsigned short     	dflags;  

     	long               	stateFlags;
     
     	/* Redirect information */
     	char               	nphone[PHONE_NUM_LEN];
     	char 			nvpnPhone[VPN_LEN]; 	
     	unsigned long 		nvpnExtLen;
     	int	       		protocol;

     	unsigned short     	nsflags;
     	unsigned short		ispcorgw;

     	/* Various timing information */
     	time_t  		iTime;          /* Inception Time */
     	time_t  		mTime;          /* Last Management Timestamp */
     	time_t  		cTime;          /* Last Connection Time */
     	time_t  		rTime;          /* Last Refresh Time (Aging) */
		time_t			iaTime;			/* Last time the endpt became inactive */
		time_t			attTime;		/* Last call attempt (candidate) */
		time_t			dndTime;		/* Last time the endpt reported dnd/ran
										 * out of ports, when we were routing calls
										 * to it
										 */
     
     	char 			zone[ZONE_LEN];
     
     	char 			cpname[CALLPLAN_ATTR_LEN];

     	unsigned short 		cap;	/* Capabilities of the terminal */

	unsigned short		rasport;
	unsigned long		rasip;

	unsigned long		ports[PORT_ARRAY_LEN];  

	unsigned short		nports;		/* Used only in IP address cache */

	unsigned long		cfgports[PORT_ARRAY_LEN];

	unsigned short		ncfgports;		/* Used only in reg id cache */

	unsigned short		callsigport;	/* Q.931 port */
	
    char 			email[EMAIL_LEN];

	/* h.323 Id */
	/* This will contain what is registered by this endpoint, 
	* or whatis registered by us to this endpoint, in case this
	* endpoint is a gatekeeper to whom we do ARQs.
	*/
	char			h323id[H323ID_LEN];
	char			pgkid[GKID_LEN];/* GKID for peer gatekeeper */

	char			uri[SIPURL_LEN];/* Full SIP URI */
	char			contact[SIPURL_LEN];

	/* macros defined below to access these fields */
	short			ncallsIn;	/* Current no of ingress calls active */
	short			ncallsOut;	/* Current no of egress calls active */
	short			ncalls;		/* sum of ncallsIn and ncallsOut, maintained here
								 * to optmimize on the no of adds done at runtime
								 */
	short			xcalls;		/* Max active calls allowed */
	short			xcallsIn;	/* Max active calls ingress */
	short			xcallsOut;	/* Max active calls egress */

	int				priority;	/* priority of choosing this gateway */
	char			techprefix[PHONE_NUM_LEN];

	/* for the time, keep the subnet ip and mask separate from ip */
	unsigned long		subnetip;
	unsigned long		subnetmask;
	int					vendor; /* Endpoint vendor */
	char			passwd[PASSWD_LEN]; /* password for H.235 */
	
	int				maxHunts;	/* Endpoint level max hunting */
	unsigned int	ecaps1;  /* Extended capabilities */

	unsigned int	crId;		/* if endpoint, then sgatekeeper's id */
								/* if sgatekeeper, then index for the entry */
	char			custID[CLIENT_ATTR_LEN];
	unsigned char	q931IE[Q931IE_LEN]; /* currently only byte 0 getting used  */
	unsigned char	bcap[BCAP_LEN]; /* Bearer Capability  */

	/* Trunk group to be used for ingress/egress calls on this port */
	/* Not a prefix */
	char			tg[PHONE_NUM_LEN];
	char			srcIngressTG[PHONE_NUM_LEN]; /* new source ingress trunk group ID */

	/* Outgoing prefix to be attached on all calls using this gw */
	char			ogprefix[PHONE_NUM_LEN];
    time_t  		rejectTime;          /* Used for managing reject routes */
	short			infoTransCap;		/* Q.391 Information Transfer Capability */

	unsigned char	realmName[REALM_NAME_LEN]; /* User configurable */
	unsigned int	realmId;  /* Assigned */

	unsigned char	igrpName[IGRP_NAME_LEN];
	char			qval[4];			/* For SIP registration */
	char			srcEgressTG[PHONE_NUM_LEN];
	char			dtg[PHONE_NUM_LEN];

} ;
typedef struct info_entry 		InfoEntry;
typedef InfoEntry 				NetoidInfoEntry;

/* Database storage data structure for vpn db. */
struct vpn_entry {
	char 		vpnName[VPNS_ATTR_LEN];

	char 		vpnId[VPN_LEN]; /* This is kind of index */
	unsigned int vpnExtLen;
	char 		cpname[CALLPLAN_ATTR_LEN];

	/* Prefix is used to define both the internal and 
	 * external route of the vpn. The internal route is
	 * implicitly ""(extLen)/prefix, and the external route
	 * is vpnId(extLen)/prefix. The external route is 
	 * associated with the vpn group. If this prefix is
	 * changed, the phone numbers of all iedges will also
	 * have to be changed.
	 */
	char 		prefix[PHONE_NUM_LEN];
	char		sipdomain[SIPURL_LEN];

	unsigned int nUsers; /* No of users */
	char 		vpnGroup[VPN_GROUP_LEN];
	char 		vpnLoc[VPNS_ATTR_LEN];
	char 		vpnContact[VPNS_ATTR_LEN];

	time_t  	mTime; /* Last Management Timestamp */
};
typedef struct vpn_entry		VpnEntry;

struct vpn_group_entry {
	char vpnGroup[VPN_GROUP_LEN];
	char cpname[CALLPLAN_ATTR_LEN];

	time_t  mTime; /* Last Management Timestamp */
};
typedef struct vpn_group_entry		VpnGroupEntry;

struct vpn_route_entry {
	char		crname[CALLPLAN_ATTR_LEN];

	char 		src[VPN_LEN];
	int			srclen;	/* len=0 means infinite */
	char 		srcprefix[PHONE_NUM_LEN];
	char		tmpsrcprefix[PHONE_NUM_LEN]; /* internal for storing last assignments */
	char 		dest[VPN_LEN];
	int			destlen;	/* len=0 means infinite */
	char 		prefix[PHONE_NUM_LEN];
	time_t		startTime;
	time_t 		endTime;	/* Applicable time */
	unsigned int crflags;
		
	time_t  	mTime;		/* Last Management Timestamp */

	time_t		rTime;		/* Last Use Time */

	char    	cpname[CALLPLAN_ATTR_LEN];	/* option plan name to have a binding */
	char		trname[TRIGGER_ATTR_LEN];	/* any trigger name associated with this route */

};
typedef struct vpn_route_entry	 VpnRouteEntry;
typedef struct vpn_route_entry	 CallPlanRouteEntry; 
typedef struct vpn_route_entry	 RouteEntry;


struct call_plan_entry {
	char		cpname[CALLPLAN_ATTR_LEN];
	char		pcpname[CALLPLAN_ATTR_LEN];	/* parent cp */
	char		vpnGroup[VPN_GROUP_LEN];

	time_t  	mTime;		/* Last Management Timestamp */
};
typedef struct	call_plan_entry		CallPlanEntry;

struct call_plan_bind_entry {
	char		cpname[CALLPLAN_ATTR_LEN];
	char		crname[CALLPLAN_ATTR_LEN];

	struct tm	sTime;
	struct tm 	fTime;	/* Interval during which the route applies */
	int			priority;		/* Priority of the route in the cp */
	unsigned int crflags;

	time_t		mTime;

};
typedef struct call_plan_bind_entry		CallPlanBindEntry;

struct trigger_entry {
	char		name[TRIGGER_ATTR_LEN];
	
	/* event information */
	int			event;
	int			srcvendor;
	int			dstvendor;

	/* action information */
	int			action;
	char		actiondata[TRIGGER_ATTR_LEN];
	int			actionflags;

};
typedef struct	trigger_entry		TriggerEntry;

enum  realm_address_enum
{
    ENDPOINT_PUBLIC_ADDRESS,
    ENDPOINT_PRIVATE_ADDRESS
  
};

typedef	enum realm_address_enum		Address_e;

struct realm_entry
{
    unsigned long   realmId;  /* The cache key */
    unsigned char   realmName[REALM_NAME_LEN]; 
    unsigned long   rsa;      /* Realm Signalling IP Address */
    unsigned long   mask;      /* Realm Signalling Net Mask */
    unsigned int    sigPoolId;
    unsigned int    medPoolId;
    unsigned int    adminStatus;
	unsigned int	operStatus;
    Address_e       addrType;
    unsigned short 	interRealm_mr;
    unsigned short 	intraRealm_mr;
    char           	vipName[IFI_NAME]; 
	unsigned long	flags;
	unsigned int authFlags;
    unsigned char   vnetName[VNET_NAME_LEN]; 

};
typedef	struct	realm_entry		RealmEntry;

struct igrp_info
{
	unsigned char	igrpName[IGRP_NAME_LEN];
	/* Configurable fields */
	unsigned short	nMaxCallsIn; 	/* max ingress calls */
	unsigned short	nMaxCallsOut; 	/* max egress calls */
	unsigned short	nMaxCallsTotal; /* Total ingress + egress calls */

	/* Accounting fields */
	unsigned short	nCallsIn; 		/* ingress calls */
	unsigned short	nCallsOut; 		/* egress calls */
	unsigned short	nCallsTotal; 	/* nCallsIn + nCallsOut */

	time_t			dndTime; /* Last time we ran out of calls */
};
typedef  struct igrp_info 		IgrpInfo;

struct vnet_entry
{
	unsigned char   vnetName[VNET_NAME_LEN];
	char           	ifName[IFI_NAME];
	unsigned short  vlanid;   /* VLAN Identifier */
	unsigned short  rtgTblId; /* Routing table ID */
	unsigned long   gateway;  /* Gateway IP Address */
};
typedef struct vnet_entry VnetEntry;


/* Cache Structure definitions */

struct _cache_table_info {
	 /* holds gws in the same calling plan */
     struct _cache_table_info *prev;
	 struct _cache_table_info *next;/* THIS SHOULD BE FIRST */
	 /*void 					*head; */	/* This is pointing the CacheTableEntry structure */
	 int					head; /* changed for RPC. Don't know if its used anymore */
	 unsigned long 			eval;      /* Evaluate the Info entry... ;-) Use Hash on Key */
	 InfoEntry 				data;
	 unsigned long iserver_addr;
	 unsigned long state;
} ;
typedef  struct _cache_table_info	CacheTableInfo;

struct cache_table_entry{
     CacheTableInfo *info;     /* chain of info entries, which is a linked list */
     long index;               /* index of this entry */
     long numInfo;             /* Number of info entries chained up */
  
     pthread_mutex_t mutex;     /* locking */
} ;
typedef struct cache_table_entry 	CacheTableEntry;

struct cache_vpn_entry{
	VpnEntry vpnEntry;	/* MUST be FIRST, see the VpnCmp fn */	
	unsigned long iserver_addr;
	unsigned long state;
} ;
typedef  struct cache_vpn_entry CacheVpnEntry;

struct cache_vpng_entry{
	VpnGroupEntry vpnGroupEntry;	/* MUST be FIRST, above */	
	unsigned long iserver_addr;
	unsigned long state;
} ;
typedef struct cache_vpng_entry  CacheVpnGEntry;

/* CP's dont reside in cache, but this entry is needed
 * for redundancy purposes, where the updates reside
 * in the cache
 */
struct cache_cp_entry {
	CallPlanEntry cpEntry;
	unsigned long iserver_addr;
	unsigned long state;
} ;
typedef  struct cache_cp_entry CacheCPEntry;

struct _CacheRouteEntry {
    struct _CacheRouteEntry *prev; /* holds routes with same dest */
    struct _CacheRouteEntry *next; /* holds routes with same dest */
    struct _CacheRouteEntry *sprev;/* holds routes with same src */
    struct _CacheRouteEntry *snext; /* holds routes with same src */
    struct _CacheRouteEntry *tprev; /* holds routes in LRU order of triggers */
    struct _CacheRouteEntry *tnext; /* holds routes in LRU order of triggers */
	VpnRouteEntry routeEntry;
	unsigned long iserver_addr;
	unsigned long state;
} ;
typedef	struct _CacheRouteEntry CacheVpnRouteEntry;
typedef	struct _CacheRouteEntry CacheRouteEntry;

struct _CacheCPBEntry {
    struct _CacheCPBEntry *crprev; /* holds bindings for the same route */
    struct _CacheCPBEntry *crnext; /* holds bindings for the same route */
    struct _CacheCPBEntry *cpprev; /* holds bindings for the same plan */
    struct _CacheCPBEntry *cpnext; /* holds bindings for the same plan */
	CallPlanBindEntry cpbEntry;
	unsigned long iserver_addr;
	unsigned long state;
} ;
typedef  struct _CacheCPBEntry   CacheCPBEntry;


struct cache_trigger_entry{
	TriggerEntry trigger;
	int			ntriggers;	/* no of installed routes for this trigger */
} ;
typedef struct cache_trigger_entry	 CacheTriggerEntry;

struct cache_realm_entry{
	 RealmEntry					realm;
	 int						socketId;
} ;
typedef cache_realm_entry	 CacheRealmEntry;

struct cache_igrp_info {
	IgrpInfo	igrp;
} ;
typedef  struct	cache_igrp_info CacheIgrpInfo;

struct cache_vnet_entry {
	VnetEntry	vnet;
} ;
typedef  struct	cache_vnet_entry CacheVnetEntry;


struct gk_info
{
    char              	regid[REG_ID_LEN];
    unsigned long     	uport;

	int 	regttl;		/* Registration TTL assigned by the GK */
	char    endpointIDString[ENDPOINTID_LEN];
	int		endpointIDLen;
	char    gkIDString[GKID_LEN];
	int		regState;	/* of is with the Gk */
	time_t	nextEvent;	/* Time when the next event is scheduled */
	unsigned int crId;	/* controller id */

	int		flags;		/* GKCAP flags defined above */
} ;

typedef	gk_info			GkInfo;
typedef gk_info			CacheGkInfo;
/* Cache Keys */

/* Netoid Database Keys:
 * Combo of serial number abd port number
 * ip address and port number (network byte order... )
 */
/*** NOT 4 BYTE aligned ***/
struct netoid_snkey{
	char regid[REG_ID_LEN];
	unsigned int uport;
} ;
typedef  netoid_snkey	NetoidSNKey; 

struct netoid_phone_key{
	char phone[PHONE_NUM_LEN]; 
} ;
typedef netoid_phone_key		NetoidPhoneKey;

struct netoid_email_key{
	char email[EMAIL_LEN]; 
} ;
typedef struct netoid_email_key  NetoidEmailKey;

struct netoid_h323id_key{
	char			h323id[H323ID_LEN];
} ;
typedef struct netoid_h323id_key  NetoidH323IdKey;

struct netoid_tg_key{
	char			tg[PHONE_NUM_LEN];
} ;
typedef struct netoid_tg_key  NetoidTgKey;

struct netoid_sipuri_key{
	char			uri[SIPURL_LEN];
} ;
typedef struct netoid_sipuri_key  NetoidUriKey;

struct netoid_crid_key{
	unsigned int	crId;		/* if endpoint, then sgatekeeper's id */
} ;
typedef	struct netoid_crid_key NetoidCrIdKey;

/* ipCache lookup Key */
struct realm_ip_key
{
	unsigned long	ipaddress;	/* IP Address */
	unsigned long	realmId;	/* realmid */
} ;
typedef struct realm_ip_key RealmIP;

/* subnet caches lookup key */
struct subnet_ip_key
{
	unsigned long	subnetip;	/* Subnet Id */
	unsigned long	mask;		/* IP mask */
} ;
typedef struct subnet_ip_key Subnet;

struct vpn_key {
	char vpnName[VPNS_ATTR_LEN];
} ;
typedef  struct	vpn_key VpnKey;

struct vpn_group_key{
	char vpnGroup[VPN_GROUP_LEN];
} ;
typedef struct vpn_group_key 	VpnGroupKey;

/* triggerCache key */
struct	trigger_key{
	char		name[TRIGGER_ATTR_LEN];
} ;
typedef	struct trigger_key	TriggerKey;

struct igrp_key{
	char		igrpNameKey[IGRP_NAME_LEN];
} ;

typedef struct igrp_key  IgrpKey;

struct realm_subnet_key {
	unsigned long	subnetip;	/* Subnet Id */
	unsigned long	mask;		/* IP mask */
	unsigned long	realmId;	/* realmid */
} ;

typedef struct realm_subnet_key RealmSubnet;

struct realm_id_key{
	unsigned long	realmId;
} ;
typedef	struct realm_id_key	RealmIdKey;

struct rsa_id_key{
	unsigned long	rsa;
} ;
typedef	struct rsa_id_key	RsaKey;

struct crname_key{
	char		crname[CALLPLAN_ATTR_LEN];
} ;
typedef struct crname_key	VpnRouteKey;
typedef struct crname_key	CallPlanRouteKey;
typedef struct crname_key	RouteKey;

struct call_plan_key{
	char    	cpname[CALLPLAN_ATTR_LEN];
} ;
typedef  call_plan_key	 CallPlanKey;

struct lru_key{
	time_t		rTime;		/* Last Use Time */
} ;
typedef	struct lru_key		LruKey;

struct cpb_key{
	char		cpname[CALLPLAN_ATTR_LEN];
	char		crname[CALLPLAN_ATTR_LEN];
} ;	
typedef struct cpb_key	CallPlanBindKey;

struct realm_key
{
	char		realmNameKey[REALM_NAME_LEN];
} ;
typedef struct realm_key	RealmKey;

struct vnet_key
{
	char		vnetNameKey[VNET_NAME_LEN];
} ;
typedef struct vnet_key	VnetKey;


const	REG_CACHE		=1 ;
const	IP_CACHE		=2 ;
const	PHONE_CACHE		=3;
const	EMAIL_CACHE		=4;
const	GK_CACHE		=5;
const	URI_CACHE		=6;
const	H323ID_CACHE	=7;
const	SUBNET_CACHE	=8;
const	CRID_CACHE		=9;
const	TG_CACHE		=10;
const	GW_CACHE		=11;
const	VPN_PHONE_CACHE =12;
const	REGID_CACHE 	=13;
const	REALM_CACHE 	=14;
const	RSA_CACHE 		=15;
const	IGRP_CACHE 		=16;
const	GWCP_CACHE		=17;
const	CPDEST_CACHE	=18;
const	TRIGGER_CACHE	=19;

const	VPN_CACHE		=20;
const   VPNG_CACHE		=21;
const	CP_CACHE		=22;
const	CPB_CACHE		=23;
const	CPBCR_CACHE		=24;
const	CPBCP_CACHE		=25;
const	RSAPUBNETS_CACHE=26;
const	DTG_CACHE		=27;
const	VNET_CACHE 		=16;
const	CACHE_IGNORE	= 0xffffffff;

union cache_key	switch (int	cache)
{
	case REG_CACHE:
		NetoidSNKey		keyInfoEntry;

	case IP_CACHE:
		RealmIP			keyRealmIP;

	case VPN_PHONE_CACHE:
	case PHONE_CACHE:
		NetoidPhoneKey	keyPhone;

	case EMAIL_CACHE:
		NetoidEmailKey	keyEmail;

	case GK_CACHE:
		NetoidSNKey		keyGk;
 
	case URI_CACHE:
		NetoidUriKey	keyUri;

	case H323ID_CACHE:
		NetoidH323IdKey	keyh323Id;

	case SUBNET_CACHE:
		RealmSubnet		keyRealmSubnet;

	case CRID_CACHE:
		NetoidCrIdKey	keyCrId;
	
	case TG_CACHE:
	case DTG_CACHE:
		NetoidTgKey		keyTg;

	case TRIGGER_CACHE:
		TriggerEntry	keyTrigger;

	case REALM_CACHE:
	case RSA_CACHE:
		unsigned long	keyRealm;

	case IGRP_CACHE:
		IgrpKey			keyIgrp;
		
	case VPN_CACHE:
		VpnKey			keyVpn;

	case VPNG_CACHE:
		VpnGroupKey		keyVpnG;

	case CP_CACHE:
		CallPlanRouteKey	keyCp;	

	case CPB_CACHE:
	case CPBCR_CACHE:
	case CPBCP_CACHE:
		CallPlanBindKey 	keyCpb;

	case VNET_CACHE:
		VnetKey			keyVnet;
};

typedef	cache_key		CacheKey;		


union cache_entry	switch (int	cache)
{
	case REG_CACHE:
	case IP_CACHE:
	case VPN_PHONE_CACHE:
	case PHONE_CACHE:
	case EMAIL_CACHE:
	case URI_CACHE:
	case H323ID_CACHE:
	case SUBNET_CACHE:
	case CRID_CACHE:
	case TG_CACHE:
		CacheTableInfo	cacheInfoEntry;

	case REALM_CACHE:
	case RSA_CACHE:
		CacheRealmEntry	cacheRealmEntry;

	case IGRP_CACHE:
		CacheIgrpInfo	cacheIgrpEntry;

	case GK_CACHE:
		CacheGkInfo		cacheGkEntry;

	case TRIGGER_CACHE:
		CacheTriggerEntry	cacheTriggerEntry;

	case VPN_CACHE:
		CacheVpnEntry	cacheVpnEntry;

	case VPNG_CACHE:
		CacheVpnGEntry	cacheVpnGEntry;

	case CP_CACHE:
		CacheCPEntry	cacheCPEntry;
		
	case CPB_CACHE:
	case CPBCR_CACHE:
	case CPBCP_CACHE:
		CacheCPBEntry	cacheCPBEntry;

	case CACHE_IGNORE: /* used for error scenarios - NULL pointer equiv */
		unsigned int	foo;

	case VNET_CACHE:
		CacheVnetEntry	cacheVnetEntry;
};

typedef  cache_entry		CacheEntryGeneric;

/* Miscellaneous data types */

struct netoid_var_a{
	NetoidInfoEntry		infoEntryVar_a<>;
} ;

typedef struct netoid_var_a		NetoidVarArr;

typedef	CacheTableInfo	CacheTableInfo_va<>;

typedef	CacheGkInfo		CacheGkInfo_va<>;

typedef CacheTriggerEntry	CacheTriggerEntry_va<>;

typedef CacheRealmEntry		CacheRealmEntry_va<>;

typedef CacheIgrpInfo		CacheIgrpInfo_va<>;

typedef CacheVpnEntry		CacheVpnEntry_va<>;

typedef CacheVpnGEntry		CacheVpnGEntry_va<>;

typedef CacheCPBEntry		CacheCPBEntry_va<>;

typedef CacheRouteEntry		CacheCREntry_va<>;

typedef CacheVnetEntry		CacheVnetEntry_va<>;

typedef	char				char_va<128>;
typedef char_va				str_va<128>;

/* 
typedef  char	cmdnodename<32>;

struct argv_argc
{
int				argc;
cmdnodename		argv<>;
};
*/

program CLI_CACHE_HANDLER_PROG{
		version CLI_CACHE_HANDLER_VERS{

		/* Low level functions on any cache */
		CacheEntryGeneric		CLI_CACHE_GET(int cacheType, CacheKey data) = 100;
		CacheEntryGeneric		CLI_CACHE_GETNEXT(int cacheType, CacheKey data) = 101;
		CacheEntryGeneric		CLI_CACHE_GETFIRST(int cacheType) = 102;
		int						CLI_CACHE_PURGE(string cacheName)= 103;
		int	CLI_CACHE_INSERT(int cacheType, CacheEntryGeneric data)=104;
		int	CLI_CACHE_GET_NUMITEMS(int cacheType) = 105;
		CacheEntryGeneric  CLI_CACHE_DELETE(int cacheType, CacheEntryGeneric data)=106;
		int	CLI_CACHE_REMOVE(int cacheType, CacheEntryGeneric data)=107;
		str_va 	CLI_HANDLE_TEST_CACHE(string argv0)=108;


		/* IEDGE related functions */
		CacheTableInfo 	CLI_CACHE_HANDLE_IEDGE(NetoidInfoEntry *netInfo, int cacheType, int op) 	= 1;
		CacheTableInfo_va  	CLI_HANDLE_NETOID_GET_BULK(int cacheType) =  2;
		CacheTableInfo 	CLI_HANDLE_GET_IEDGE_LONGEST_MATCH(RealmSubnet *rmsub) =3;
		int	 CLI_HANDLE_MEM_AGE_IEDGES_IN_VPNGS(string vpng1, string vpng2)=4;
		


		/* GK gkCache */
		CacheGkInfo_va		CLI_HANDLE_GK_GET_BULK() = 5;

		int   CLI_CACHE_HANDLE_VPN(VpnEntry *vpnEntry, int op) 	= 6;
		int   CLI_CACHE_HANDLE_VPNG (VpnGroupEntry *vpnGroupEntry, int op) 	= 7;
		int   CLI_CACHE_HANDLE_CR(VpnRouteEntry *routeEntry, int op) 	= 8;
		int   CLI_CACHE_HANDLE_CP(CallPlanEntry *cpEntry, int op) 	= 9;
		int   CLI_CACHE_HANDLE_CPB(CallPlanBindEntry *cpbEntry, int op)	= 10;
		int   CLI_CACHE_HANDLE_IGRP(IgrpInfo *igrpEntry, int op)	= 11;
		int   CLI_CACHE_HANDLE_REALM(RealmEntry *rmEntry, int op)	= 12;
		int   CLI_CACHE_HANDLE_TRIGGER(TriggerEntry *tgEntry, int op) = 13;

		/* Populate functions */
		int	CLI_HANDLE_IEDGE_CACHE_POP(int flags)=14;
		int	CLI_HANDLE_VPN_CACHE_POP(int flags)=15;
		int	CLI_HANDLE_VPNG_CACHE_POP(int flags)=16;
		int	CLI_HANDLE_CP_CACHE_POP(int flags)=17;
		int	CLI_HANDLE_CR_CACHE_POP(int flags)=18;
		int	CLI_HANDLE_CPB_CACHE_POP(int flags)=19;
		int	CLI_HANDLE_TRIGGER_CACHE_POP(int flags)=20;
		int	CLI_HANDLE_REALM_CACHE_POP(int flags)=21;
		int	CLI_HANDLE_IGRP_CACHE_POP(int flags)=22;

		/* License */
		int		CLI_LICENSE_INIT()= 23;
		int		CLI_LICENSE_ALLOCATE(int n) = 24;
		int		CLI_LICENSE_RELEASE(int n) = 25;
	
		/* license stuff coming out of lsMem */
		int		CLI_GET_LSMEM_USEDLIC()= 26;
		int		CLI_GET_LSMEM_NUMLIC()=27;
		int		CLI_GET_LSMEM_MAXCALLS()=28;
		int		CLI_GET_LSMEM_MAXMRCALLS()=29;

		/* shared memory related routines called from java */
		int	CLI_NLM_INIT_CONFIG_PORT()= 30;
		int	CLI_NLM_GET_CONFIG_PORT()= 31;
		int	CLI_NLM_SET_CONFIG_PORT(int n)= 32;

		int	CLI_NLM_GET_VPORT() = 33;
		int	CLI_NLM_GET_USEDVPORT() = 34;
		int	CLI_NLM_GET_USEDVPORT_NOLOCK() = 35;
	
		int CLI_NLM_GET_MRVPORT()=36;
		int CLI_NLM_GET_USEDMRVPORT_NOLOCK()=38;
		int CLI_NLM_FREE_MRVPORT()=39;

		/* Features */
		string	CLI_NLM_GET_FEATURELIST()= 40;

		int	CLI_SIP_ENABLED()= 41;
		int	CLI_FCE_ENABLED()= 42;
		int	CLI_RADIUS_ENABLED()= 43;
		int	CLI_H323_ENABLED()= 44;
		int	CLI_GEN_ENABLED()= 45;

		/* Realm specific */
		CacheRealmEntry_va	CLI_HANDLE_REALM_GET_BULK() = 46;
		/* not coded this one. its just one cacheget */
		int		CLI_HANDLE_CHECK_REALMID_DUP(unsigned long realmid) =47;
		int		CLI_HANDLE_REALM_ENABLE_SIG(unsigned long rsa)=48;
		int		CLI_HANDLE_REALM_DISABLE_SIG(unsigned long rsa)=49;
		string	CLI_HANDLE_CHECK_RSA_DUP(RealmEntry *new, RealmEntry *old) =50;
				
		/* Igrp specific */
		CacheIgrpInfo_va	CLI_HANDLE_IGRP_GET_BULK() = 51;
		int		CLI_HANDLE_IGRP_ADD_CALLS(string igrpname, unsigned int incalls, 
											unsigned int outcalls, unsigned int totalcalls) =52;
		int		CLI_HANDLE_IGRP_DELETE_CALLS(string igrpname, unsigned int incalls, 
											unsigned int outcalls, unsigned int totalcalls) =53;

		/* trigger specific */
		CacheTriggerEntry_va CLI_HANDLE_TRIGGER_GET_BULK() =54;

		/* xxxxCacheInstantiate and xxxxDestroyData functions */
		int CLI_HANDLE_CACHE_REINSTANTIATE(int cacheType) =55;

		/* VPN specific */
		CacheVpnEntry_va CLI_HANDLE_VPN_GET_BULK() = 56;
		CacheVpnGEntry_va CLI_HANDLE_VPNG_GET_BULK() = 57;
		int		CLI_HANDLE_VPN_SIPDOMAIN_CHANGE(VpnEntry *vpn) =58;
		int		CLI_HANDLE_VPN_PREFIX_CHANGE(VpnEntry *vpn) = 59;

		/* CP specific */
		CacheCPBEntry_va	CLI_HANDLE_CP_GET_BULK() = 60;
		CacheCREntry_va		CLI_HANDLE_CR_GET_BULK() = 61;
		CacheRouteEntry		CLI_HANDLE_CACHE_ROUTE_ENTRY_GET_NEXTPTR(CacheRouteEntry *entry) =62;
		CacheCPBEntry		CLI_HANDLE_CACHE_CPB_ENTRY_GET_CRNEXTPTR(CacheCPBEntry *entry) =63;
		CacheTableInfo		CLI_HANDLE_CACHE_IEDGE_ENTRY_GET_NEXTPTR(CacheTableInfo *entry) =64;
	} = 1;

} = 0x2000babe;

