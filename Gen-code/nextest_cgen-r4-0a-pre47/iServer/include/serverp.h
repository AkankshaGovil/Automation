#ifndef _serverp_h_
#define _serverp_h_

#include <sys/resource.h>
#include "mem.h"
#include "db.h"
#include "avl.h"
#include "license.h"
#include "thutils.h"
#include "callstats.h"
#include "srvrlog.h"
#include "lsconfig.h"

/* Default config file name */
#define CONFIG_FILENAME "./server.cfg"
#define MAX_LS_ALARM 15
#define LS_ALARM_INTERVAL 300  // seconds before registering the next license alarm

#define MAX_RSD_RECORDS  32
#define MAX_H323INSTANCES	10

#define N_SCM_STAT_RECORDS	100

void sig_int(int signo);

void sig_chld(int signo);

void sig_hup(int signo);

int DoConfig(int (*processConfig)(void));

int FindServerConfig(void);

extern char config_file[];

/* function to restart signal once its has been handled */
void (*Signal (int signo, void(*disp)(int)))(int);

typedef struct
{
	int xprotocols;
	int	nprotocols;
	int xprocs;
	int nprocs;
	int xtpktchans;
	int ntpktchans;
	int xmessages;
	int nmessages;
	int vtnodecount;
	int xudpchans;
	int nudpchans;
	int	xevents;
	int	nevents;
	int	xchannels;
	int nchannels;
	int	xchandescs;
	int nchandescs;
	int	xtimers;
	int	ntimers;

	int *nCalls;
	int *arqRate;
	int *setupRate;
	int *selims;
	int *bridgems;

} AllocStats;

typedef struct
{
	int invs;		// Invite server
	int invsr;		// Invite server retransmissions
	int invc;		// Invite client
	int	invcr;		// Invite client retransmissions
	int	invsfr;		// Invite server final response retransmissions
	int	invcfr;		// Invite client final response retransmissions
	int byes;		// bye server
	int byesr;		// bye server retransmissions
	int byec;		// bye clients
	int byecr;		// bye client retransmissions
	int cs;			// cancel server
	int csr;		// cancel server retransmissions
	int cc;			// cancel client
	int ccr;		// cancel client retransmissions
	int notrans;	// no call/transaction
	
	int			ntrans;	// total no of TSMs processed
	hrtime_t	ptime;	// total time for TSM processing

} SipStats;

typedef struct
{
	int allowSrcAll;
	int allowDestAll;
	int cpRoutingPolicy;
	int cacheTimeout;
	int routeDebug;
	int nh323Instances;
	int maxHunts;
	int crids;
	int	HistDbSize;
	int	RSDConfig;
	int allowHairPin;
	int globalTime;
	int rejectTime;
	
} CfgParms;

typedef struct
{
	int nelems;
	hrtime_t transportLatency[N_SCM_STAT_RECORDS];

	int pendingStates;
	int	errorStates;
	int successStates;

	cache_t scmCallCache;

} SCMInfo;

typedef struct
{
  unsigned int ipaddr;   // ip address of the iserver
  int          status;   // status of the iserver (primary/secondary)
  unsigned int port;	 // rs_port
} RSDRecord;

typedef struct
{
  int          count;                    // count of valid entries
  RSDRecord    records[MAX_RSD_RECORDS]; // records of cluster information
} RSDInfo;


// This structure is what every process will be able to see
// when it attaches to the shared memory
// It is the application data associated with the main
// memory map
typedef struct
{
	int 			nlic;
	int 			usedlic;
	time_t			expiry_time;
	char			macstr[MAC_ADDR_LEN];
	pthread_mutex_t	usedlicmutex;

	/* Iedge Information */
	Lock			iedgemutex;
	cache_t			regCache;
	cache_t			regidCache;
	cache_t			phoneCache;
	cache_t			vpnPhoneCache;
//	cache_t			emailCache;		- eliminated for 2.1
	cache_t			gwCache;
	cache_t			ipCache;
	cache_t			gkCache;
	cache_t			h323idCache;
	cache_t			uriCache;
	cache_t			gwcpCache;
	cache_t			subnetCache;
	cache_t			cridCache;
	cache_t			tgCache;
	cache_t			dtgCache;

	CacheEntry		updateList;
	pthread_mutex_t updatemutex;
	
	/* List of vpns */
	cache_t 		vpnCache;
	Lock			vpnmutex;

	/* List of vpn groups */
	cache_t			vpnGCache;
	Lock			vpngmutex;
	
	/* Call Plan/Route cache */
	cache_t			cpCache;
	cache_t			cpdestCache;
	cache_t			cpsrcCache;
	cache_t			cptransitCache;
	cache_t			cporigDNISCache;
	cache_t			cporigANICache;	
	Lock			cpmutex;

	/* Call Cache */
	cache_t			callCache;
	cache_t			sipCallCache;
	cache_t			guidCache;
	cache_t			tipCache;
	Lock			callmutex;

	/* Conf Cache */
	cache_t			confCache;
	Lock			confmutex;

	/* Call Plan Binding cache */
	cache_t			cpbCache;
	cache_t			cpbcrCache;
	cache_t			cpbcpCache;
	cache_t			lruRoutesCache;
	Lock			cpbmutex;

	cache_t			transCache;
	cache_t			transDestCache;
	Lock			transmutex;

	/* Trigger cache */
	cache_t			triggerCache;
	Lock			triggermutex;

	/* Realm cache */
	cache_t			realmCache;
	cache_t			rsaCache;
	cache_t			rsapubnetsCache;
	CacheRealmEntry *defaultRealm;
	Lock			realmmutex;

	/* igrp cache */
	cache_t			igrpCache;
	Lock			igrpmutex;

	AllocStats 		*allocStats[MAX_H323INSTANCES];
	int				maxCalls;
	int				features;
	int				maxMRCalls;
	int				nMRCalls;

	SipStats		*sipStats;
	CfgParms		*cfgParms;
	SCMInfo			*scm;

	/* license alarm */
	time_t			*lsVportAlarm;
	time_t			*lsMRVportAlarm;
	Lock			alarmmutex;

	/* firewal config structure */
	void			*fwParams;
	Lock			*fwLock;

	/* Call Statistics */
	CallStats		*callStats;

	/* status flag */
	int				statusFlag;

	/* rsd status information */
	Lock			rsdmutex;
	RSDInfo			*rsdInfo;

	cache_t			sipregCache;
	Lock			sipregmutex;

	time_t			initTime;
	time_t			termTime;
	int				maxRealms;		/* Do not access directly, use nlm_getMaxRealms() */
	int				maxRoutes;		/* Do not access directly, use nlm_getMaxRoutes() */
	int				maxDynamicEP;	/* Do not access directly, use nlm_getMaxDynamicEP() */

	cache_t			vnetCache;
	Lock			vnetmutex;

} LsMemStruct;

/* the n-th bit in statusFlag */
#define STATUS_ALL_INIT     1      /* complete initialization done */

#define SET_STATUS(lsMem, status)   ((lsMem)->statusFlag |= (1 << (status)))
#define CLEAR_STATUS(lsMem, status) ((lsMem)->statusFlag &= ~(1 << (status)))
#define CHECK_STATUS(lsMem, status) ((lsMem)->statusFlag & (1 << (status)))


int
LsMemStructInit(LsMemStruct *m);

extern MemoryMap *map;
extern LsMemStruct *lsMem;

int IedgeCachePopulate(LsMemStruct *, DB, unsigned short);
int VpnPopulate(LsMemStruct *, DB);
int VpnGPopulate(LsMemStruct *, DB);
int CPPopulate(LsMemStruct *, DB);
int CPBPopulate(LsMemStruct *, DB);


int ServerSetLogging(char *, serplex_config *);

void setConfigFile(void);

int UH323ResetLogging(void);

int SipResetLogging(void);

extern int notifyPipe[];

extern int (*identMain)();
 
extern int poolid, hpcid, lpcid;

#define sglobalTime	(lsMem->cfgParms->globalTime)
#define srejectTime	(lsMem->cfgParms->rejectTime)



#endif /* _serverp_h_ */
