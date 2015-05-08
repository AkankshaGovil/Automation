/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _CLI_RPC_H_RPCGEN
#define	_CLI_RPC_H_RPCGEN

#include <rpc/rpc.h>
#include "ipc.h"
#include "key.h"
#include "serverdb.h"

enum Vendor_e {
	Vendor_eGeneric = 0,
	Vendor_eClarent = 1,
	Vendor_eSonusGSX = 2,
	Vendor_eBroadsoft = 3,
	Vendor_eCisco = 4,
	Vendor_eVocalTec = 5,
	Vendor_eLucentTnt = 6,
	Vendor_eExcel = 7
};
typedef enum Vendor_e Vendor_e;

typedef Vendor_e Vendor;

struct info_entry {
	char regid[REG_ID_LEN];
	u_long uport;
	IPaddr ipaddress;
	char phone[PHONE_NUM_LEN];
	char vpnName[VPNS_ATTR_LEN];
	char vpnPhone[VPN_LEN];
	u_long vpnExtLen;
	u_short sflags;
	u_short dflags;
	long stateFlags;
	char nphone[PHONE_NUM_LEN];
	char nvpnPhone[VPN_LEN];
	u_long nvpnExtLen;
	int protocol;
	u_short nsflags;
	u_short ispcorgw;
	time_t iTime;
	time_t mTime;
	time_t cTime;
	time_t rTime;
	time_t iaTime;
	time_t attTime;
	time_t dndTime;
	char zone[ZONE_LEN];
	char cpname[CALLPLAN_ATTR_LEN];
	u_short cap;
	u_short rasport;
	u_long rasip;
	u_long ports[PORT_ARRAY_LEN];
	u_short nports;
	u_long cfgports[PORT_ARRAY_LEN];
	u_short ncfgports;
	u_short callsigport;
	char email[EMAIL_LEN];
	char h323id[H323ID_LEN];
	char pgkid[GKID_LEN];
	char uri[SIPURL_LEN];
	char contact[SIPURL_LEN];
	short ncallsIn;
	short ncallsOut;
	short ncalls;
	short xcalls;
	short xcallsIn;
	short xcallsOut;
	int priority;
	char techprefix[PHONE_NUM_LEN];
	u_long subnetip;
	u_long subnetmask;
	int vendor;
	char passwd[PASSWD_LEN];
	int maxHunts;
	u_int ecaps1;
	u_int crId;
	char custID[CLIENT_ATTR_LEN];
	u_char q931IE[Q931IE_LEN];
	u_char bcap[BCAP_LEN];
	char tg[PHONE_NUM_LEN];
	char srcIngressTG[PHONE_NUM_LEN];
	char ogprefix[PHONE_NUM_LEN];
	time_t rejectTime;
	short infoTransCap;
	u_char realmName[REALM_NAME_LEN];
	u_int realmId;
	u_char igrpName[IGRP_NAME_LEN];
	char qval[4];
	char srcEgressTG[PHONE_NUM_LEN];
	char dtg[PHONE_NUM_LEN];
        u_short cidblk;        
};
typedef struct info_entry info_entry;

typedef info_entry InfoEntry;

typedef InfoEntry NetoidInfoEntry;

struct vpn_entry {
	char vpnName[VPNS_ATTR_LEN];
	char vpnId[VPN_LEN];
	u_int vpnExtLen;
	char cpname[CALLPLAN_ATTR_LEN];
	char prefix[PHONE_NUM_LEN];
	char sipdomain[SIPURL_LEN];
	u_int nUsers;
	char vpnGroup[VPN_GROUP_LEN];
	char vpnLoc[VPNS_ATTR_LEN];
	char vpnContact[VPNS_ATTR_LEN];
	time_t mTime;
  
};
typedef struct vpn_entry vpn_entry;

typedef vpn_entry VpnEntry;

struct vpn_group_entry {
	char vpnGroup[VPN_GROUP_LEN];
	char cpname[CALLPLAN_ATTR_LEN];
	time_t mTime;
};
typedef struct vpn_group_entry vpn_group_entry;

typedef vpn_group_entry VpnGroupEntry;

struct vpn_route_entry {
	char crname[CALLPLAN_ATTR_LEN];
	char src[VPN_LEN];
	int srclen;
	char srcprefix[PHONE_NUM_LEN];
	char tmpsrcprefix[PHONE_NUM_LEN];
	char dest[VPN_LEN];
	int destlen;
	char prefix[PHONE_NUM_LEN];
	time_t startTime;
	time_t endTime;
	u_int crflags;
	time_t mTime;
	time_t rTime;
	char cpname[CALLPLAN_ATTR_LEN];
	char trname[TRIGGER_ATTR_LEN];
};
typedef struct vpn_route_entry vpn_route_entry;

typedef vpn_route_entry VpnRouteEntry;

typedef vpn_route_entry CallPlanRouteEntry;

typedef vpn_route_entry RouteEntry;

struct call_plan_entry {
	char cpname[CALLPLAN_ATTR_LEN];
	char pcpname[CALLPLAN_ATTR_LEN];
	char vpnGroup[VPN_GROUP_LEN];
	time_t mTime;
};
typedef struct call_plan_entry call_plan_entry;

typedef call_plan_entry CallPlanEntry;

struct call_plan_bind_entry {
	char cpname[CALLPLAN_ATTR_LEN];
	char crname[CALLPLAN_ATTR_LEN];
	struct tm sTime;
	struct tm fTime;
	int priority;
	u_int crflags;
	time_t mTime;
};
typedef struct call_plan_bind_entry call_plan_bind_entry;

typedef call_plan_bind_entry CallPlanBindEntry;

struct trigger_entry {
	char name[TRIGGER_ATTR_LEN];
	int event;
	int srcvendor;
	int dstvendor;
	int action;
	char actiondata[TRIGGER_ATTR_LEN];
	int actionflags;
};
typedef struct trigger_entry trigger_entry;

typedef trigger_entry TriggerEntry;

enum realm_address_enum {
	ENDPOINT_PUBLIC_ADDRESS = 0,
	ENDPOINT_PRIVATE_ADDRESS = 1
};
typedef enum realm_address_enum realm_address_enum;

typedef realm_address_enum Address_e;

struct realm_entry {
	u_long realmId;
	u_char realmName[REALM_NAME_LEN];
	u_long rsa;
	u_long mask;
	u_int sigPoolId;
	u_int medPoolId;
	u_int adminStatus;
	u_int operStatus;
	Address_e addrType;
	u_short interRealm_mr;
	u_short intraRealm_mr;
	char ifName[IFI_NAME];
	char vipName[IFI_NAME];
	u_long flags;  
        u_long mirrorProxy;
};
typedef struct realm_entry realm_entry;

typedef realm_entry RealmEntry;

struct igrp_info {
	u_char igrpName[IGRP_NAME_LEN];
	u_short nMaxCallsIn;
	u_short nMaxCallsOut;
	u_short nMaxCallsTotal;
	u_short nCallsIn;
	u_short nCallsOut;
	u_short nCallsTotal;
	time_t dndTime;
};
typedef struct igrp_info igrp_info;

typedef igrp_info IgrpInfo;

struct _cache_table_info {
	struct _cache_table_info *prev;
	struct _cache_table_info *next;
	int head;
	u_long eval;
	InfoEntry data;
	u_long iserver_addr;
	u_long state;
};
typedef struct _cache_table_info _cache_table_info;

typedef _cache_table_info CacheTableInfo;

struct cache_table_entry {
	CacheTableInfo *info;
	long index;
	long numInfo;
	pthread_mutex_t mutex;
};
typedef struct cache_table_entry cache_table_entry;

typedef cache_table_entry CacheTableEntry;

struct cache_vpn_entry {
	VpnEntry vpnEntry;
	u_long iserver_addr;
	u_long state;
};
typedef struct cache_vpn_entry cache_vpn_entry;

typedef cache_vpn_entry CacheVpnEntry;

struct cache_vpng_entry {
	VpnGroupEntry vpnGroupEntry;
	u_long iserver_addr;
	u_long state;
};
typedef struct cache_vpng_entry cache_vpng_entry;

typedef cache_vpng_entry CacheVpnGEntry;

struct cache_cp_entry {
	CallPlanEntry cpEntry;
	u_long iserver_addr;
	u_long state;
};
typedef struct cache_cp_entry cache_cp_entry;

typedef cache_cp_entry CacheCPEntry;

struct _CacheRouteEntry {
	struct _CacheRouteEntry *prev;
	struct _CacheRouteEntry *next;
	struct _CacheRouteEntry *sprev;
	struct _CacheRouteEntry *snext;
	struct _CacheRouteEntry *tprev;
	struct _CacheRouteEntry *tnext;
	VpnRouteEntry routeEntry;
	u_long iserver_addr;
	u_long state;
};
typedef struct _CacheRouteEntry _CacheRouteEntry;

typedef _CacheRouteEntry CacheVpnRouteEntry;

typedef _CacheRouteEntry CacheRouteEntry;

struct _CacheCPBEntry {
	struct _CacheCPBEntry *crprev;
	struct _CacheCPBEntry *crnext;
	struct _CacheCPBEntry *cpprev;
	struct _CacheCPBEntry *cpnext;
	CallPlanBindEntry cpbEntry;
	u_long iserver_addr;
	u_long state;
};
typedef struct _CacheCPBEntry _CacheCPBEntry;

typedef _CacheCPBEntry CacheCPBEntry;

struct cache_trigger_entry {
	TriggerEntry trigger;
	int ntriggers;
};
typedef struct cache_trigger_entry cache_trigger_entry;

typedef cache_trigger_entry CacheTriggerEntry;

struct cache_realm_entry {
	RealmEntry realm;
	int socketId;
};
typedef struct cache_realm_entry cache_realm_entry;

typedef cache_realm_entry CacheRealmEntry;

struct cache_igrp_info {
	IgrpInfo igrp;
};
typedef struct cache_igrp_info cache_igrp_info;

typedef cache_igrp_info CacheIgrpInfo;

struct gk_info {
	char regid[REG_ID_LEN];
	u_long uport;
	int regttl;
	char endpointIDString[ENDPOINTID_LEN];
	int endpointIDLen;
	char gkIDString[GKID_LEN];
	int regState;
	time_t nextEvent;
	u_int crId;
	int flags;
};
typedef struct gk_info gk_info;

typedef gk_info GkInfo;

typedef gk_info CacheGkInfo;

struct netoid_snkey {
	char regid[REG_ID_LEN];
	u_int uport;
};
typedef struct netoid_snkey netoid_snkey;

typedef netoid_snkey NetoidSNKey;

struct netoid_phone_key {
	char phone[PHONE_NUM_LEN];
};
typedef struct netoid_phone_key netoid_phone_key;

typedef netoid_phone_key NetoidPhoneKey;

struct netoid_email_key {
	char email[EMAIL_LEN];
};
typedef struct netoid_email_key netoid_email_key;

typedef netoid_email_key NetoidEmailKey;

struct netoid_h323id_key {
	char h323id[H323ID_LEN];
};
typedef struct netoid_h323id_key netoid_h323id_key;

typedef netoid_h323id_key NetoidH323IdKey;

struct netoid_tg_key {
	char tg[PHONE_NUM_LEN];
};
typedef struct netoid_tg_key netoid_tg_key;

typedef netoid_tg_key NetoidTgKey;

struct netoid_sipuri_key {
	char uri[SIPURL_LEN];
};
typedef struct netoid_sipuri_key netoid_sipuri_key;

typedef netoid_sipuri_key NetoidUriKey;

struct netoid_crid_key {
	u_int crId;
};
typedef struct netoid_crid_key netoid_crid_key;

typedef netoid_crid_key NetoidCrIdKey;

struct realm_ip_key {
	u_long ipaddress;
	u_long realmId;
};
typedef struct realm_ip_key realm_ip_key;

typedef realm_ip_key RealmIP;

struct subnet_ip_key {
	u_long subnetip;
	u_long mask;
};
typedef struct subnet_ip_key subnet_ip_key;

typedef subnet_ip_key Subnet;

struct vpn_key {
	char vpnName[VPNS_ATTR_LEN];
};
typedef struct vpn_key vpn_key;

typedef vpn_key VpnKey;

struct vpn_group_key {
	char vpnGroup[VPN_GROUP_LEN];
};
typedef struct vpn_group_key vpn_group_key;

typedef vpn_group_key VpnGroupKey;

struct trigger_key {
	char name[TRIGGER_ATTR_LEN];
};
typedef struct trigger_key trigger_key;

typedef trigger_key TriggerKey;

struct igrp_key {
	char igrpNameKey[IGRP_NAME_LEN];
};
typedef struct igrp_key igrp_key;

typedef igrp_key IgrpKey;

struct realm_subnet_key {
	u_long subnetip;
	u_long mask;
	u_long realmId;
};
typedef struct realm_subnet_key realm_subnet_key;

typedef realm_subnet_key RealmSubnet;

struct realm_id_key {
	u_long realmId;
};
typedef struct realm_id_key realm_id_key;

typedef realm_id_key RealmIdKey;

struct rsa_id_key {
	u_long rsa;
};
typedef struct rsa_id_key rsa_id_key;

typedef rsa_id_key RsaKey;

struct crname_key {
	char crname[CALLPLAN_ATTR_LEN];
};
typedef struct crname_key crname_key;

typedef crname_key VpnRouteKey;

typedef crname_key CallPlanRouteKey;

typedef crname_key RouteKey;

struct call_plan_key {
	char cpname[CALLPLAN_ATTR_LEN];
};
typedef struct call_plan_key call_plan_key;

typedef call_plan_key CallPlanKey;

struct lru_key {
	time_t rTime;
};
typedef struct lru_key lru_key;

typedef lru_key LruKey;

struct cpb_key {
	char cpname[CALLPLAN_ATTR_LEN];
	char crname[CALLPLAN_ATTR_LEN];
};
typedef struct cpb_key cpb_key;

typedef cpb_key CallPlanBindKey;

struct realm_key {
	char realmNameKey[REALM_NAME_LEN];
};
typedef struct realm_key realm_key;

typedef realm_key RealmKey;
#define	REG_CACHE 1
#define	IP_CACHE 2
#define	PHONE_CACHE 3
#define	EMAIL_CACHE 4
#define	GK_CACHE 5
#define	URI_CACHE 6
#define	H323ID_CACHE 7
#define	SUBNET_CACHE 8
#define	CRID_CACHE 9
#define	TG_CACHE 10
#define	GW_CACHE 11
#define	VPN_PHONE_CACHE 12
#define	REGID_CACHE 13
#define	REALM_CACHE 14
#define	RSA_CACHE 15
#define	IGRP_CACHE 16
#define	GWCP_CACHE 17
#define	CPDEST_CACHE 18
#define	TRIGGER_CACHE 19
#define	VPN_CACHE 20
#define	VPNG_CACHE 21
#define	CP_CACHE 22
#define	CPB_CACHE 23
#define	CPBCR_CACHE 24
#define	CPBCP_CACHE 25
#define	RSAPUBNETS_CACHE 26
#define	DTG_CACHE 27
#define	CACHE_IGNORE 0xffffffff

struct cache_key {
	int cache;
	union {
		NetoidSNKey keyInfoEntry;
		RealmIP keyRealmIP;
		NetoidPhoneKey keyPhone;
		NetoidEmailKey keyEmail;
		NetoidSNKey keyGk;
		NetoidUriKey keyUri;
		NetoidH323IdKey keyh323Id;
		RealmSubnet keyRealmSubnet;
		NetoidCrIdKey keyCrId;
		NetoidTgKey keyTg;
		TriggerEntry keyTrigger;
		u_long keyRealm;
		IgrpKey keyIgrp;
		VpnKey keyVpn;
		VpnGroupKey keyVpnG;
		CallPlanRouteKey keyCp;
		CallPlanBindKey keyCpb;
	} cache_key_u;
};
typedef struct cache_key cache_key;

typedef cache_key CacheKey;

struct cache_entry {
	int cache;
	union {
		CacheTableInfo cacheInfoEntry;
		CacheRealmEntry cacheRealmEntry;
		CacheIgrpInfo cacheIgrpEntry;
		CacheGkInfo cacheGkEntry;
		CacheTriggerEntry cacheTriggerEntry;
		CacheVpnEntry cacheVpnEntry;
		CacheVpnGEntry cacheVpnGEntry;
		CacheCPEntry cacheCPEntry;
		CacheCPBEntry cacheCPBEntry;
		u_int foo;
	} cache_entry_u;
};
typedef struct cache_entry cache_entry;

typedef cache_entry CacheEntryGeneric;

struct netoid_var_a {
	struct {
		u_int infoEntryVar_a_len;
		NetoidInfoEntry *infoEntryVar_a_val;
	} infoEntryVar_a;
};
typedef struct netoid_var_a netoid_var_a;

typedef netoid_var_a NetoidVarArr;

typedef struct {
	u_int CacheTableInfo_va_len;
	CacheTableInfo *CacheTableInfo_va_val;
} CacheTableInfo_va;

typedef struct {
	u_int CacheGkInfo_va_len;
	CacheGkInfo *CacheGkInfo_va_val;
} CacheGkInfo_va;

typedef struct {
	u_int CacheTriggerEntry_va_len;
	CacheTriggerEntry *CacheTriggerEntry_va_val;
} CacheTriggerEntry_va;

typedef struct {
	u_int CacheRealmEntry_va_len;
	CacheRealmEntry *CacheRealmEntry_va_val;
} CacheRealmEntry_va;

typedef struct {
	u_int CacheIgrpInfo_va_len;
	CacheIgrpInfo *CacheIgrpInfo_va_val;
} CacheIgrpInfo_va;

typedef struct {
	u_int CacheVpnEntry_va_len;
	CacheVpnEntry *CacheVpnEntry_va_val;
} CacheVpnEntry_va;

typedef struct {
	u_int CacheVpnGEntry_va_len;
	CacheVpnGEntry *CacheVpnGEntry_va_val;
} CacheVpnGEntry_va;

typedef struct {
	u_int CacheCPBEntry_va_len;
	CacheCPBEntry *CacheCPBEntry_va_val;
} CacheCPBEntry_va;

typedef struct {
	u_int CacheCREntry_va_len;
	CacheRouteEntry *CacheCREntry_va_val;
} CacheCREntry_va;

typedef struct {
	u_int char_va_len;
	char *char_va_val;
} char_va;

typedef struct {
	u_int str_va_len;
	char_va *str_va_val;
} str_va;

struct cli_cache_get_1_argument {
	int cacheType;
	CacheKey data;
};
typedef struct cli_cache_get_1_argument cli_cache_get_1_argument;

struct cli_cache_getnext_1_argument {
	int cacheType;
	CacheKey data;
};
typedef struct cli_cache_getnext_1_argument cli_cache_getnext_1_argument;

struct cli_cache_insert_1_argument {
	int cacheType;
	CacheEntryGeneric data;
};
typedef struct cli_cache_insert_1_argument cli_cache_insert_1_argument;

struct cli_cache_delete_1_argument {
	int cacheType;
	CacheEntryGeneric data;
};
typedef struct cli_cache_delete_1_argument cli_cache_delete_1_argument;

struct cli_cache_remove_1_argument {
	int cacheType;
	CacheEntryGeneric data;
};
typedef struct cli_cache_remove_1_argument cli_cache_remove_1_argument;

struct cli_cache_handle_iedge_1_argument {
	NetoidInfoEntry *netInfo;
	int cacheType;
	int op;
};
typedef struct cli_cache_handle_iedge_1_argument cli_cache_handle_iedge_1_argument;

struct cli_handle_mem_age_iedges_in_vpngs_1_argument {
	char *vpng1;
	char *vpng2;
};
typedef struct cli_handle_mem_age_iedges_in_vpngs_1_argument cli_handle_mem_age_iedges_in_vpngs_1_argument;

struct cli_cache_handle_vpn_1_argument {
	VpnEntry *vpnEntry;
	int op;
};
typedef struct cli_cache_handle_vpn_1_argument cli_cache_handle_vpn_1_argument;

struct cli_cache_handle_vpng_1_argument {
	VpnGroupEntry *vpnGroupEntry;
	int op;
};
typedef struct cli_cache_handle_vpng_1_argument cli_cache_handle_vpng_1_argument;

struct cli_cache_handle_cr_1_argument {
	VpnRouteEntry *routeEntry;
	int op;
};
typedef struct cli_cache_handle_cr_1_argument cli_cache_handle_cr_1_argument;

struct cli_cache_handle_cp_1_argument {
	CallPlanEntry *cpEntry;
	int op;
};
typedef struct cli_cache_handle_cp_1_argument cli_cache_handle_cp_1_argument;

struct cli_cache_handle_cpb_1_argument {
	CallPlanBindEntry *cpbEntry;
	int op;
};
typedef struct cli_cache_handle_cpb_1_argument cli_cache_handle_cpb_1_argument;

struct cli_cache_handle_igrp_1_argument {
	IgrpInfo *igrpEntry;
	int op;
};
typedef struct cli_cache_handle_igrp_1_argument cli_cache_handle_igrp_1_argument;

struct cli_cache_handle_realm_1_argument {
	RealmEntry *rmEntry;
	int op;
};
typedef struct cli_cache_handle_realm_1_argument cli_cache_handle_realm_1_argument;

struct cli_cache_handle_trigger_1_argument {
	TriggerEntry *tgEntry;
	int op;
};
typedef struct cli_cache_handle_trigger_1_argument cli_cache_handle_trigger_1_argument;

struct cli_handle_check_rsa_dup_1_argument {
	RealmEntry *new;
	RealmEntry *old;
};
typedef struct cli_handle_check_rsa_dup_1_argument cli_handle_check_rsa_dup_1_argument;

struct cli_handle_igrp_add_calls_1_argument {
	char *igrpname;
	u_int incalls;
	u_int outcalls;
	u_int totalcalls;
};
typedef struct cli_handle_igrp_add_calls_1_argument cli_handle_igrp_add_calls_1_argument;

struct cli_handle_igrp_delete_calls_1_argument {
	char *igrpname;
	u_int incalls;
	u_int outcalls;
	u_int totalcalls;
};
typedef struct cli_handle_igrp_delete_calls_1_argument cli_handle_igrp_delete_calls_1_argument;

#define	CLI_CACHE_HANDLER_PROG	0x2000babe
#define	CLI_CACHE_HANDLER_VERS	1
#define	CLI_CACHE_GET	100
extern  CacheEntryGeneric * cli_cache_get_1();
#define	CLI_CACHE_GETNEXT	101
extern  CacheEntryGeneric * cli_cache_getnext_1();
#define	CLI_CACHE_GETFIRST	102
extern  CacheEntryGeneric * cli_cache_getfirst_1();
#define	CLI_CACHE_PURGE	103
extern  int * cli_cache_purge_1();
#define	CLI_CACHE_INSERT	104
extern  int * cli_cache_insert_1();
#define	CLI_CACHE_GET_NUMITEMS	105
extern  int * cli_cache_get_numitems_1();
#define	CLI_CACHE_DELETE	106
extern  CacheEntryGeneric * cli_cache_delete_1();
#define	CLI_CACHE_REMOVE	107
extern  int * cli_cache_remove_1();
#define	CLI_HANDLE_TEST_CACHE	108
extern  str_va * cli_handle_test_cache_1();
#define	CLI_CACHE_HANDLE_IEDGE	1
extern  CacheTableInfo * cli_cache_handle_iedge_1();
#define	CLI_HANDLE_NETOID_GET_BULK	2
extern  CacheTableInfo_va * cli_handle_netoid_get_bulk_1();
#define	CLI_HANDLE_GET_IEDGE_LONGEST_MATCH	3
extern  CacheTableInfo * cli_handle_get_iedge_longest_match_1();
#define	CLI_HANDLE_MEM_AGE_IEDGES_IN_VPNGS	4
extern  int * cli_handle_mem_age_iedges_in_vpngs_1();
#define	CLI_HANDLE_GK_GET_BULK	5
extern  CacheGkInfo_va * cli_handle_gk_get_bulk_1();
#define	CLI_CACHE_HANDLE_VPN	6
extern  int * cli_cache_handle_vpn_1();
#define	CLI_CACHE_HANDLE_VPNG	7
extern  int * cli_cache_handle_vpng_1();
#define	CLI_CACHE_HANDLE_CR	8
extern  int * cli_cache_handle_cr_1();
#define	CLI_CACHE_HANDLE_CP	9
extern  int * cli_cache_handle_cp_1();
#define	CLI_CACHE_HANDLE_CPB	10
extern  int * cli_cache_handle_cpb_1();
#define	CLI_CACHE_HANDLE_IGRP	11
extern  int * cli_cache_handle_igrp_1();
#define	CLI_CACHE_HANDLE_REALM	12
extern  int * cli_cache_handle_realm_1();
#define	CLI_CACHE_HANDLE_TRIGGER	13
extern  int * cli_cache_handle_trigger_1();
#define	CLI_HANDLE_IEDGE_CACHE_POP	14
extern  int * cli_handle_iedge_cache_pop_1();
#define	CLI_HANDLE_VPN_CACHE_POP	15
extern  int * cli_handle_vpn_cache_pop_1();
#define	CLI_HANDLE_VPNG_CACHE_POP	16
extern  int * cli_handle_vpng_cache_pop_1();
#define	CLI_HANDLE_CP_CACHE_POP	17
extern  int * cli_handle_cp_cache_pop_1();
#define	CLI_HANDLE_CR_CACHE_POP	18
extern  int * cli_handle_cr_cache_pop_1();
#define	CLI_HANDLE_CPB_CACHE_POP	19
extern  int * cli_handle_cpb_cache_pop_1();
#define	CLI_HANDLE_TRIGGER_CACHE_POP	20
extern  int * cli_handle_trigger_cache_pop_1();
#define	CLI_HANDLE_REALM_CACHE_POP	21
extern  int * cli_handle_realm_cache_pop_1();
#define	CLI_HANDLE_IGRP_CACHE_POP	22
extern  int * cli_handle_igrp_cache_pop_1();
#define	CLI_LICENSE_INIT	23
extern  int * cli_license_init_1();
#define	CLI_LICENSE_ALLOCATE	24
extern  int * cli_license_allocate_1();
#define	CLI_LICENSE_RELEASE	25
extern  int * cli_license_release_1();
#define	CLI_GET_LSMEM_USEDLIC	26
extern  int * cli_get_lsmem_usedlic_1();
#define	CLI_GET_LSMEM_NUMLIC	27
extern  int * cli_get_lsmem_numlic_1();
#define	CLI_GET_LSMEM_MAXCALLS	28
extern  int * cli_get_lsmem_maxcalls_1();
#define	CLI_GET_LSMEM_MAXMRCALLS	29
extern  int * cli_get_lsmem_maxmrcalls_1();
#define	CLI_NLM_INIT_CONFIG_PORT	30
extern  int * cli_nlm_init_config_port_1();
#define	CLI_NLM_GET_CONFIG_PORT	31
extern  int * cli_nlm_get_config_port_1();
#define	CLI_NLM_SET_CONFIG_PORT	32
extern  int * cli_nlm_set_config_port_1();
#define	CLI_NLM_GET_VPORT	33
extern  int * cli_nlm_get_vport_1();
#define	CLI_NLM_GET_USEDVPORT	34
extern  int * cli_nlm_get_usedvport_1();
#define	CLI_NLM_GET_USEDVPORT_NOLOCK	35
extern  int * cli_nlm_get_usedvport_nolock_1();
#define	CLI_NLM_GET_MRVPORT	36
extern  int * cli_nlm_get_mrvport_1();
#define	CLI_NLM_GET_USEDMRVPORT_NOLOCK	38
extern  int * cli_nlm_get_usedmrvport_nolock_1();
#define	CLI_NLM_FREE_MRVPORT	39
extern  int * cli_nlm_free_mrvport_1();
#define	CLI_NLM_GET_FEATURELIST	40
extern  char ** cli_nlm_get_featurelist_1();
#define	CLI_SIP_ENABLED	41
extern  int * cli_sip_enabled_1();
#define	CLI_FCE_ENABLED	42
extern  int * cli_fce_enabled_1();
#define	CLI_RADIUS_ENABLED	43
extern  int * cli_radius_enabled_1();
#define	CLI_H323_ENABLED	44
extern  int * cli_h323_enabled_1();
#define	CLI_GEN_ENABLED	45
extern  int * cli_gen_enabled_1();
#define	CLI_HANDLE_REALM_GET_BULK	46
extern  CacheRealmEntry_va * cli_handle_realm_get_bulk_1();
#define	CLI_HANDLE_CHECK_REALMID_DUP	47
extern  int * cli_handle_check_realmid_dup_1();
#define	CLI_HANDLE_REALM_ENABLE_SIG	48
extern  int * cli_handle_realm_enable_sig_1();
#define	CLI_HANDLE_REALM_DISABLE_SIG	49
extern  int * cli_handle_realm_disable_sig_1();
#define	CLI_HANDLE_CHECK_RSA_DUP	50
extern  char ** cli_handle_check_rsa_dup_1();
#define	CLI_HANDLE_IGRP_GET_BULK	51
extern  CacheIgrpInfo_va * cli_handle_igrp_get_bulk_1();
#define	CLI_HANDLE_IGRP_ADD_CALLS	52
extern  int * cli_handle_igrp_add_calls_1();
#define	CLI_HANDLE_IGRP_DELETE_CALLS	53
extern  int * cli_handle_igrp_delete_calls_1();
#define	CLI_HANDLE_TRIGGER_GET_BULK	54
extern  CacheTriggerEntry_va * cli_handle_trigger_get_bulk_1();
#define	CLI_HANDLE_CACHE_REINSTANTIATE	55
extern  int * cli_handle_cache_reinstantiate_1();
#define	CLI_HANDLE_VPN_GET_BULK	56
extern  CacheVpnEntry_va * cli_handle_vpn_get_bulk_1();
#define	CLI_HANDLE_VPNG_GET_BULK	57
extern  CacheVpnGEntry_va * cli_handle_vpng_get_bulk_1();
#define	CLI_HANDLE_VPN_SIPDOMAIN_CHANGE	58
extern  int * cli_handle_vpn_sipdomain_change_1();
#define	CLI_HANDLE_VPN_PREFIX_CHANGE	59
extern  int * cli_handle_vpn_prefix_change_1();
#define	CLI_HANDLE_CP_GET_BULK	60
extern  CacheCPBEntry_va * cli_handle_cp_get_bulk_1();
#define	CLI_HANDLE_CR_GET_BULK	61
extern  CacheCREntry_va * cli_handle_cr_get_bulk_1();
#define	CLI_HANDLE_CACHE_ROUTE_ENTRY_GET_NEXTPTR	62
extern  CacheRouteEntry * cli_handle_cache_route_entry_get_nextptr_1();
#define	CLI_HANDLE_CACHE_CPB_ENTRY_GET_CRNEXTPTR	63
extern  CacheCPBEntry * cli_handle_cache_cpb_entry_get_crnextptr_1();
#define	CLI_HANDLE_CACHE_IEDGE_ENTRY_GET_NEXTPTR	64
extern  CacheTableInfo * cli_handle_cache_iedge_entry_get_nextptr_1();
extern int cli_cache_handler_prog_1_freeresult();

/* the xdr functions */
extern bool_t xdr_Vendor_e();
extern bool_t xdr_Vendor();
extern bool_t xdr_info_entry();
extern bool_t xdr_InfoEntry();
extern bool_t xdr_NetoidInfoEntry();
extern bool_t xdr_vpn_entry();
extern bool_t xdr_VpnEntry();
extern bool_t xdr_vpn_group_entry();
extern bool_t xdr_VpnGroupEntry();
extern bool_t xdr_vpn_route_entry();
extern bool_t xdr_VpnRouteEntry();
extern bool_t xdr_CallPlanRouteEntry();
extern bool_t xdr_RouteEntry();
extern bool_t xdr_call_plan_entry();
extern bool_t xdr_CallPlanEntry();
extern bool_t xdr_call_plan_bind_entry();
extern bool_t xdr_CallPlanBindEntry();
extern bool_t xdr_trigger_entry();
extern bool_t xdr_TriggerEntry();
extern bool_t xdr_realm_address_enum();
extern bool_t xdr_Address_e();
extern bool_t xdr_realm_entry();
extern bool_t xdr_RealmEntry();
extern bool_t xdr_igrp_info();
extern bool_t xdr_IgrpInfo();
extern bool_t xdr__cache_table_info();
extern bool_t xdr_CacheTableInfo();
extern bool_t xdr_cache_table_entry();
extern bool_t xdr_CacheTableEntry();
extern bool_t xdr_cache_vpn_entry();
extern bool_t xdr_CacheVpnEntry();
extern bool_t xdr_cache_vpng_entry();
extern bool_t xdr_CacheVpnGEntry();
extern bool_t xdr_cache_cp_entry();
extern bool_t xdr_CacheCPEntry();
extern bool_t xdr__CacheRouteEntry();
extern bool_t xdr_CacheVpnRouteEntry();
extern bool_t xdr_CacheRouteEntry();
extern bool_t xdr__CacheCPBEntry();
extern bool_t xdr_CacheCPBEntry();
extern bool_t xdr_cache_trigger_entry();
extern bool_t xdr_CacheTriggerEntry();
extern bool_t xdr_cache_realm_entry();
extern bool_t xdr_CacheRealmEntry();
extern bool_t xdr_cache_igrp_info();
extern bool_t xdr_CacheIgrpInfo();
extern bool_t xdr_gk_info();
extern bool_t xdr_GkInfo();
extern bool_t xdr_CacheGkInfo();
extern bool_t xdr_netoid_snkey();
extern bool_t xdr_NetoidSNKey();
extern bool_t xdr_netoid_phone_key();
extern bool_t xdr_NetoidPhoneKey();
extern bool_t xdr_netoid_email_key();
extern bool_t xdr_NetoidEmailKey();
extern bool_t xdr_netoid_h323id_key();
extern bool_t xdr_NetoidH323IdKey();
extern bool_t xdr_netoid_tg_key();
extern bool_t xdr_NetoidTgKey();
extern bool_t xdr_netoid_sipuri_key();
extern bool_t xdr_NetoidUriKey();
extern bool_t xdr_netoid_crid_key();
extern bool_t xdr_NetoidCrIdKey();
extern bool_t xdr_realm_ip_key();
extern bool_t xdr_RealmIP();
extern bool_t xdr_subnet_ip_key();
extern bool_t xdr_Subnet();
extern bool_t xdr_vpn_key();
extern bool_t xdr_VpnKey();
extern bool_t xdr_vpn_group_key();
extern bool_t xdr_VpnGroupKey();
extern bool_t xdr_trigger_key();
extern bool_t xdr_TriggerKey();
extern bool_t xdr_igrp_key();
extern bool_t xdr_IgrpKey();
extern bool_t xdr_realm_subnet_key();
extern bool_t xdr_RealmSubnet();
extern bool_t xdr_realm_id_key();
extern bool_t xdr_RealmIdKey();
extern bool_t xdr_rsa_id_key();
extern bool_t xdr_RsaKey();
extern bool_t xdr_crname_key();
extern bool_t xdr_VpnRouteKey();
extern bool_t xdr_CallPlanRouteKey();
extern bool_t xdr_RouteKey();
extern bool_t xdr_call_plan_key();
extern bool_t xdr_CallPlanKey();
extern bool_t xdr_lru_key();
extern bool_t xdr_LruKey();
extern bool_t xdr_cpb_key();
extern bool_t xdr_CallPlanBindKey();
extern bool_t xdr_realm_key();
extern bool_t xdr_RealmKey();
extern bool_t xdr_cache_key();
extern bool_t xdr_CacheKey();
extern bool_t xdr_cache_entry();
extern bool_t xdr_CacheEntryGeneric();
extern bool_t xdr_netoid_var_a();
extern bool_t xdr_NetoidVarArr();
extern bool_t xdr_CacheTableInfo_va();
extern bool_t xdr_CacheGkInfo_va();
extern bool_t xdr_CacheTriggerEntry_va();
extern bool_t xdr_CacheRealmEntry_va();
extern bool_t xdr_CacheIgrpInfo_va();
extern bool_t xdr_CacheVpnEntry_va();
extern bool_t xdr_CacheVpnGEntry_va();
extern bool_t xdr_CacheCPBEntry_va();
extern bool_t xdr_CacheCREntry_va();
extern bool_t xdr_char_va();
extern bool_t xdr_str_va();
extern bool_t xdr_cli_cache_get_1_argument();
extern bool_t xdr_cli_cache_getnext_1_argument();
extern bool_t xdr_cli_cache_insert_1_argument();
extern bool_t xdr_cli_cache_delete_1_argument();
extern bool_t xdr_cli_cache_remove_1_argument();
extern bool_t xdr_cli_cache_handle_iedge_1_argument();
extern bool_t xdr_cli_handle_mem_age_iedges_in_vpngs_1_argument();
extern bool_t xdr_cli_cache_handle_vpn_1_argument();
extern bool_t xdr_cli_cache_handle_vpng_1_argument();
extern bool_t xdr_cli_cache_handle_cr_1_argument();
extern bool_t xdr_cli_cache_handle_cp_1_argument();
extern bool_t xdr_cli_cache_handle_cpb_1_argument();
extern bool_t xdr_cli_cache_handle_igrp_1_argument();
extern bool_t xdr_cli_cache_handle_realm_1_argument();
extern bool_t xdr_cli_cache_handle_trigger_1_argument();
extern bool_t xdr_cli_handle_check_rsa_dup_1_argument();
extern bool_t xdr_cli_handle_igrp_add_calls_1_argument();
extern bool_t xdr_cli_handle_igrp_delete_calls_1_argument();

#endif /* !_CLI_RPC_H_RPCGEN */
