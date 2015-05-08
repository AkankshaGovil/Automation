#ifndef _GW_H_
#define _GW_H_

#include "handles.h"
#include "ipcerror.h"

struct gw_match_set
{
	struct gw_match_set *prev, *next;

	// Option 1.
	char regid[REG_ID_LEN];
	unsigned long uport;

	// Option 2.
	unsigned long ipaddr;
	
	int zoneVal;
	int vpnVal;
	int cpVal;
	int routePriority;
	int gwPriority;
	float utilization;
	time_t cTime;			// lower value means higher preference
	long gwFlags;

	 // Keep as last entry for easy initialization
	 VpnRouteEntry route;
};

int
GwLookup(ResolveHandle *rhandle, RouteLogFn logfn);

//debug version - for exhaustive lookup
int
GwLookup_debug(char *srcVpnGrp, char *srcVpnId, int checkVpnGrp,
	char *srcZone, int checkZone, int isVpnCall,
	PhoNode *dest, PhoNode *phonode, CacheTableInfo *, unsigned short *sigport,
	int primary, int reservePort, int phoneChange,
	RouteLogFn logfn);

int
ApplyNetworkPolicy(
	char *vpnName,
	char *cpname,
	char *sphone,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int type,
	int *matchVal,
	char *crname,
	RouteLogFn logfn
);

int
ApplyVpnGPolicy(
	char *vpng,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int *matchVal,
	RouteLogFn logfn
);

int
ApplyVpnPolicy(
	VpnEntry *vpn,
	char *phone,
	char *newphone,
	unsigned long crflags,
	int type,
	int *matchVal,
	RouteLogFn logfn
);

int
ApplyCPPolicy(
	char *cpname,
	char *sphone,
	char *phone,
	char *newphone,
	unsigned long crflags,
	unsigned long type, /* PREFIX/DEST */
	int *matchVal,
	char *crname,
	RouteLogFn logfn
);

int
ApplyRoutePrefix(char *crname, char *phone, 
	char *newphone, int *xlen);
int
CRApplyPrefix(VpnRouteEntry *routeEntry, char *phone, 
	char *newphone, int *xlen);

int
ApplyRouteDest(char *crname, char *phone, char *newphone);

int
CRApplyDest(
	VpnRouteEntry *routeEntry, 
	char *phone, 
	char *newphone,
	int *matchLen,
	int *hasRejectAllElse
);

int
CRApplyTime(
	CallPlanBindEntry *cpbEntry
);

/* Initialize set to { -1,-1,-1... }
 * CpCompare returs when comparison is either > or <.
 * It proceeds to the next level if its ==
 */
typedef struct _cp_match_set
{
	 int cpVal;
	 int srcVal;
	 VpnRouteEntry route;
} cp_match_set;

typedef struct 
{
	int routePriority;
	int matchLen;
	CacheRouteEntry *cacheRouteEntry;
} route_match_set;

int
GwAddPhoNodeToRejectList(PhoNode *phonode, char *crname, ListEntry **rejectList, 
	void *(*mallocfn)(size_t));

int
GwCheckRejectList(CacheTableInfo *gwCacheEntry, ListEntry *rejectList, char *crname);

int
GwFreeRejectList(ListEntry *head, void (*freefn)(void *));

int
FillSourceCacheForCallerIdForceLkup(PhoNode *phonodep, char *h323id, char *tg, 
	char *cic, char *dnis,
	CacheTableInfo *srcCacheInfo, int force);

#define FillSourceCacheForCallerId(phonode, h323id, tg, cic, dnis, srcCacheInfo) \
	FillSourceCacheForCallerIdForceLkup(phonode, h323id, tg, cic, dnis, \
						srcCacheInfo, 0)

// decrement the no. of Active Calls on the port 
// Only serial no. and uport need to be set in PhoNode
// callSource = Is MSW call source
void GwPortFreeCall(PhoNode *phonode, int ncalls, int callSource);
int GwPortAllocCall(PhoNode *phonode, int ncalls, int callSource);
int ctInfoAllocCall(CacheTableInfo *ctInfo,int ncalls, int callSource);

CacheTableInfo * GwRouteLookup(
	cache_t cache, 
	int offset,
	char *pattern, 
	char *regid, 
	struct gw_match_set *gwset, 
	struct gw_match_set *gwsetin, 
	RouteLogFn logfn);

/* lkup.c */
int SetPhonodeFromDb (PhoNode *phonode, InfoEntry *pentry);
/* lkup.c */

/* routematch.c */ 
extern int ApplyANIPlan (char *cpname, char *phone, char *newphone, long unsigned int crflags, long
		unsigned int type, int *matchVal, RouteLogFn logfn);
/* routematch.c */ 

#endif /* _GW_H_ */
