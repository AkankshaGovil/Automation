/* Include file for cache management
 */

#ifndef _mem_h_
#define _mem_h_

#include <sys/ipc.h>
#include <sys/shm.h>
#include "spthread.h"

#include "ipc.h"
#include "key.h"
#include "serverdb.h"
#include "alerror.h"
#include "shm.h"
#include "shmapp.h"
#include "cache.h"
#include "clist.h"
#include "call_realm_info.h"

#define CACHE_NEEDS_UPDATE	0x0001
#define CACHE_NEEDS_DELETE	0x0002
#define CACHE_PUSH_ENTRY	0x0004
#define CACHE_DIRTY		0x0008

typedef struct _cache_table_info {
	 // holds gws in the same calling plan
     struct _cache_table_info *prev, *next;   	/* THIS SHOULD BE FIRST */
     struct _cache_table_info *dtgprev, *dtgnext;   	/* THIS SHOULD BE FIRST */
     void *head;               			/* This is pointing the CacheTableEntry structure */
     InfoEntry data;
} CacheTableInfo;

typedef struct {
     CacheTableInfo *info;     /* chain of info entries, which is a linked list */
     long index;               /* index of this entry */
     long numInfo;             /* Number of info entries chained up */
  
     pthread_mutex_t mutex;     /* locking */
} CacheTableEntry;

typedef struct {
	VpnEntry vpnEntry;	/* MUST be FIRST, see the VpnCmp fn */	
} CacheVpnEntry;

typedef struct {
	VpnGroupEntry vpnGroupEntry;	/* MUST be FIRST, above */	
} CacheVpnGEntry;

/* CP's dont reside in cache, but this entry is needed
 * for redundancy purposes, where the updates reside
 * in the cache
 */
typedef struct {
	CallPlanEntry cpEntry;
} CacheCPEntry;
	
#define LRUROUTESOFFSET		2*sizeof(ListEntry)

typedef struct _CacheRouteEntry {
    struct _CacheRouteEntry *prev, *next; // holds routes with same dest
    struct _CacheRouteEntry *sprev, *snext; // holds routes with same src
    struct _CacheRouteEntry *tprev, *tnext; // holds routes in LRU order of triggers
	VpnRouteEntry routeEntry;
} CacheVpnRouteEntry, CacheRouteEntry;

typedef struct _CacheCPBEntry {
    struct _CacheCPBEntry *crprev, *crnext; // holds bindings for the same route
    struct _CacheCPBEntry *cpprev, *cpnext; // holds bindings for the same plan
	CallPlanBindEntry cpbEntry;
} CacheCPBEntry;
	
typedef struct {
	TriggerEntry trigger;
	int			ntriggers;	// no of installed routes for this trigger
} CacheTriggerEntry;


typedef struct {
	 RealmEntry					realm;
	 int						socketId;
} CacheRealmEntry;

typedef struct {
	IgrpInfo	igrp;
} CacheIgrpInfo;

typedef struct {
	VnetEntry	vnet;
} CacheVnetEntry;

#if 0 /* moved to ls/rpc/call_realm_info.h */
// Realm information which will be useful in doing call source lookups,
// like realmId, passing to FC code, like pool ids, inter/intra realm mr
// usually a subset of the realm entry in the database
typedef struct {
    unsigned long   realmId;  

	// all other information can be looked up
    unsigned long   rsa;     

    unsigned int    sPoolId;
    unsigned int    mPoolId;
    Address_e       addrType;

    unsigned short interRealm_mr;
    unsigned short intraRealm_mr;

	char		   *sipdomain;
} CallRealmInfo;
#endif

CallRealmInfo * getRealmInfo (int realmId, int mallocfn);

#define CACHE_INFO_ENTRY	1
#define CACHE_VPN_ENTRY		2
#define CACHE_VPNG_ENTRY	3
#define CACHE_ROUTE_ENTRY	4
#define CACHE_CP_ENTRY		5
#define CACHE_CPB_ENTRY		6

/* Generic Cache Entry  - used for chaining also
 * where the current prev and next ptrs are useless
 */
typedef struct _cache_entry_ {
     	struct _cache_entry_ *prev, *next;   	/* THIS SHOULD BE FIRST */
	int type;
	void *entry;
} CacheEntry;

/* Linear state sequence, of registration states.
 * Progression is cyclical
 */
typedef enum
{
	/* Note that the ordering of this enum governs the ordering of
	 * entries in the cache
	 */
	GKREG_UNKNOWN = 0,		/* Nothing has been attempted so far */

	GKREG_REGISTERED = 1,
	GKREG_REGISTERING,
	GKREG_DISCOVERED,
	GKREG_DISCOVERING,
	GKREG_IDLE,				/* Registration has been attempted, nothing achieved */
	GKREG_TRYALT,	/* We will try the alternate of this gk */

	GKREG_MAX,
} GkRegStates;

#define GKREG_LEN	25
extern char gkRegState[][GKREG_LEN];

#define GKCAP_LTWTREG	0x00000001

typedef struct
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
} GkInfo, CacheGkInfo;

extern CacheEntry *updateList;
extern cache_t 
	regCache, phoneCache, vpnPhoneCache, emailCache, gwCache, h323idCache,
	ipCache, regidCache, gkCache, vpnCache,
	vpnGCache, cpCache, callCache, confCache, cpbCache, uriCache,
	transCache, sipCallCache, cpdestCache, cpbcrCache, gwcpCache,
	cpbcpCache, subnetCache, cpsrcCache, cridCache, transDestCache,
    tgCache, guidCache, triggerCache, cptransitCache, lruRoutesCache,
    realmCache, rsaCache, tipCache, cporigDNISCache, cporigANICache, 
	igrpCache, rsapubnetsCache, sipregCache, dtgCache, vnetCache;

// Also protected by the realm cache locks
extern CacheRealmEntry **defaultRealm;

CacheTableInfo *GetFreeCacheInfo();

CacheVpnEntry *GetFreeVpnEntry();

CacheVpnGEntry *GetFreeVpnGEntry();

CacheVpnRouteEntry *GetFreeVpnRouteEntry();

CacheCPEntry *GetFreeCPEntry();
CacheCPBEntry *GetFreeCPBEntry();

CacheTriggerEntry * GetFreeTriggerEntry();
CacheRealmEntry *   GetFreeRealmEntry();
CacheIgrpInfo * GetFreeIgrpInfo();
CacheVnetEntry *   GetFreeVnetEntry();
int AddVnetEntry(char *name, char *errMsg);
int DeleteVnetEntry(char *name, char *errMsg);

int
InitCacheInfo(CacheTableInfo *cacheInfo, InfoEntry *info);

typedef pthread_mutex_t MemRwLock;

#define LOCK_READ  1
#define LOCK_WRITE 2

#define LOCK_BLOCK 1
#define LOCK_TRY   2
 
AlStatus
MemGetRwLock(MemRwLock *lock, uint mode, uint flags);

AlStatus
MemReleaseRwLock(MemRwLock *lock);

#define CACHE_POPULATE_ALL	0x1
#define CACHE_SAVE_DYNAMIC	0x2
#define CACHE_PURGE_OLD		0x4
#define CACHE_INIT_STAGE	0x8

CacheTableInfo *
CacheDupInfoEntry(NetoidInfoEntry *info);

CacheVpnEntry *
CacheDupVpnEntry(VpnEntry *in);

CacheVpnGEntry *
CacheDupVpnGEntry(VpnGroupEntry *in);

CacheVpnRouteEntry *
CacheDupVpnRouteEntry(VpnRouteEntry *);

CacheCPEntry *
CacheDupCPEntry(CallPlanEntry *in);

CacheCPBEntry *
CacheDupCPBEntry(CallPlanBindEntry *in);

CacheTriggerEntry *
CacheDupTriggerEntry(TriggerEntry *in);

CacheRealmEntry *
CacheDupRealmEntry(RealmEntry *in);

CacheVnetEntry *
CacheDupVnetEntry(VnetEntry *in);

CacheVpnGEntry *
CacheDeleteVpnG(VpnGroupEntry *vpnGroupEntry, 
	int (*testfn)(CacheVpnGEntry *cacheVpnGEntry, void *), void *);

CacheVpnGEntry *
CacheStoreVpnGEntry(VpnGroupEntry *vpnGroupEntry);

CacheVpnEntry *
CacheDeleteVpn(VpnEntry *vpnEntry, 
	int (*testfn)(CacheVpnEntry *cacheVpnEntry, void *), void *);

CacheVpnEntry *
CacheStoreVpnEntry(VpnEntry *vpnEntry);

CacheIgrpInfo*
CacheDupIgrpInfo(IgrpInfo *in);

CacheTableInfo *
GetIedge(InfoEntry *netInfo);

CacheTableInfo *
GetDuplicateIedge(InfoEntry *netInfo);

int
FindIedge(InfoEntry *netInfo, void *copy, size_t size);

CacheTableInfo *
DeleteIedge(InfoEntry *netInfo);

int
AddIedge(CacheTableInfo *cacheInfo);

char *
GuessVpnPhone(InfoEntry *entry, char *inphone, char *outphone, unsigned long *);

int
MemAgeIedgesInVpnGs(char *vpng1, char *vpng2);

int
BranchProxyPhone(InfoEntry *pentry, CacheTableInfo *cacheInfo, char *);

int
BranchFwdPhone(InfoEntry *pentry, CacheTableInfo *cacheInfo, char *);

int
FindIedgeVpn(char *vpnName, VpnEntry *vpn);

int
GetIedgeVpn(char *vpnName, VpnEntry *vpn);

InfoEntry *
SearchNetoidDatabase (char *key, int keylen);

InfoEntry *
FindIedgeVpnGroupInDatabase(char *key, int keylen, char *vpnGroup);

int
AddIedgeIpAddr(cache_t ipCache, CacheTableInfo *cacheInfo);

int
DeleteIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo);

CacheTableInfo *
GetDuplicateIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo);

CacheTableInfo *
CheckDuplicateIedgeIpAddr(cache_t ipCache, NetoidInfoEntry *netInfo, 
						  unsigned long ipaddr);

CacheTableInfo *DeleteGw(InfoEntry *entry);

CacheTableInfo * GetIedgeLongestMatch(RealmSubnet *RealmSubnetEntry);

int DeleteRealm(unsigned long *rmId);

int AddRealm(CacheRealmEntry *entry);

int DeleteIgrp(unsigned char *igrpname);

int AddIgrp(CacheIgrpInfo *entry);

int DeleteVnet(unsigned char *vnetName);

int AddVnet(CacheVnetEntry *entry);

int ExtractIedge(char *key, void *copy, size_t size);

#define IedgeRegInsCmp		1
#define IedgeRegidInsCmp	2
#define IedgePhoneInsCmp	3
#define IedgeVpnPhoneInsCmp	4
#define IedgeEmailInsCmp	5
#define IedgeIpInsCmp		6
#define VpnInsCmp			7
#define VpnGInsCmp			8
#define CPInsCmp			9
#define CallInsCmp			10
#define IedgeUriInsCmp		11
#define ConfInsCmp			12
#define CPBInsCmp			13
#define IedgeGkInsCmp		14
#define IedgeH323IdInsCmp   15
#define TransInsCmp			16
#define SipCallInsCmp		17
#define TimerInsCmp			18
#define TimerHandleInsCmp	19
#define IedgeSubnetInsCmp	20
#define LicConnTableInsCmp	21
#define IedgeCrIdInsCmp		22
#define TransDestInsCmp		23
#define IedgeTGInsCmp   	24
#define GuidInsCmp			25	
#define TriggerInsCmp		26
#define RoutesLRUInsCmp		27
#define RealmInsCmpFnId		28
#define RsaInsCmpFnId		29
#define TipInsCmp			30
#define IgrpInsCmpFnId		31
#define RsapubnetsInsCmpFnId	32	
#define SipRegInsCmp		33
#define SCMCallInsCmp		34
#define VnetInsCmpFnId		35

#define IedgeRegCmp			1
#define IedgeRegidCmp		2
#define IedgePhoneCmp		3
#define IedgeVpnPhoneCmp	4
#define IedgeEmailCmp		5
#define IedgeIpCmp			6
#define VpnCmp				7
#define VpnGCmp				8
#define CPCmp				9
#define CallCmp				10
#define IedgeUriCmp			11
#define ConfCmp				12
#define CPBCmp				13
#define IedgeGkCmp			14
#define IedgeH323IdCmp      15
#define TransCmp			16
#define SipCallCmp			17
#define TimerCmp			18
#define TimerHandleCmp		19
#define IedgeSubnetCmp		20
#define LicConnTableCmp		21
#define IedgeCrIdCmp		22
#define TransDestCmp		23
#define IedgeTGCmp      	24
#define GuidCmp				25	
#define TriggerCmp			26
#define RealmCmpFnId		27
#define RsaCmpFnId			28
#define TipCmp				29
#define IgrpCmpFnId			30
#define RsapubnetsCmpFnId		31
#define SipRegCmp			32
#define SCMCallCmp		33
#define VnetCmpFnId			34

#define IedgePhoneData2Key	1
#define CPData2Key			2
#define CPBCRData2Key		3
#define IedgeGwCPData2Key	4
#define CPBCPData2Key		5

#define IedgePhoneInsData2Key	1
#define CPInsData2Key		2
#define CPBCRInsData2Key	3
#define IedgeGwCPInsData2Key	4
#define CPBCPInsData2Key	5
#define CPSrcInsData2Key	6

#define IedgeRegDup			1
#define IedgeRegidDup		2
#define IedgePhoneDup		3
#define IedgeVpnPhoneDup	4
#define IedgeEmailDup		5
#define IedgeIpDup			6
#define VpnDup				7
#define VpnGDup				8
#define CPDup				9
#define CallDup				10
#define IedgeUriDup			11
#define ConfDup				12
#define CPBDup				13
#define IedgeGkDup			14
#define IedgeH323IdDup     	15
#define TransDup			16
#define SipCallDup			17
#define TimerDup			18
#define TimerHandleDup		19
#define IedgeSubnetDup		20
#define LicConnTableDup		21
#define IedgeCrIdDup		22
#define TransDestDup		23
#define IedgeTGDup     		24
#define TriggerDup     		25
#define SipRegDup			26

#define IedgeCachePreCb		1
#define VpnCachePreCb		2
#define VpnGCachePreCb		3
#define TimerPreCb			4
#define TimerHandlePreCb	5
#define CallCachePreCb		6

#define IedgeCachePostCb	1
#define VpnCachePostCb		2
#define VpnGCachePostCb		3
#define TimerPostCb			4
#define TimerHandlePostCb	5
#define LruRoutesPostCb		6

int ShmAttached();

/* Status of cache lookups */
#define CACHE_FOUND			0		/* Successful */
#define CACHE_NOTFOUND		1		/* Failed */
#define CACHE_INPROG		2		/* In Progeress. LRQ etc */
#define CACHE_ERR			-1		/* Error */

char *
CStrdup(cache_t cache, char *str);

#define ISERVER_FTOK_PATH	"/usr/local/nextone/bin"
#define ISERVER_FTOK_PROJ_BRIDGEH323Q	9

#define memswap(x, y, sz)	do { char tmp[sz]; memcpy(tmp, y, sz); memcpy(y, x, sz); memcpy(x, tmp, sz); } while (0)

void * GetSubnetLongestMatch(cache_t cache, Subnet *sub);

int ShmSetReady (void);

/* memdb.c */
int DeleteNetoidFromDatabase (NetoidInfoEntry *netInfo);
int DeleteCRFromDatabase (VpnRouteEntry *routeEntry);
int UpdateNetoidDatabase (NetoidInfoEntry *netInfo);
int UpdateNetoidAttrDatabase (NetoidSNKey *snkey, ClientAttribs *clAttribs);
int DeleteIedgeRegid (cache_t regidCache, NetoidInfoEntry *netInfo);
int HandleVpnPrefixChange (VpnEntry *vpnEntry);
int HandleVpnPrefixChangeDb (VpnEntry *vpnEntry);
int AssignIedgePhone (InfoEntry *entry, VpnEntry *vpnEntry);
int HandleVpnSipDomainChange (VpnEntry *vpnEntry);
int HandleVpnSipDomainChangeDb (VpnEntry *vpnEntry);
int PropagateIedgeIpAddr (char *regid, long unsigned int ipaddr);
int AddRoute (CacheRouteEntry *entry);
int DeleteRoute (RouteEntry *entry);
int AddCPB (CacheCPBEntry *entry);
int DeleteCPBFromDatabase (CallPlanBindEntry *cpbEntry);
int DeleteCPB (CallPlanBindEntry *entry);
int AddTrigger (CacheTriggerEntry *entry);
int DeleteTrigger (TriggerEntry *tgEntry);
int AddGw (CacheTableInfo *entry);
int CPBTestTime (CallPlanBindEntry *cpbEntry);

/* memdb.c */
#endif 
