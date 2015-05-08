#ifndef _u_h323_h_
#define _u_h323_h_

#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "cm.h"
#include "key.h"
#include "Threads_API.h"

#define GK_DISCOVERY_ADDRSTR	"224.0.1.41"
#define LRQ_MAX_PHONES	10
#define MAX_TCS_ENTRIES 20
#define MAX_ENCODEDECODE_BUFFRSIZE 2056

/* Well defined instance ids for H323 multiple instancing case */
#define SGK_MINSTANCE_ID		(nh323Instances - 3)
#define ARQ_MINSTANCE_ID		(nh323Instances - 2)
#define TMP_MINSTANCE_ID		(nh323Instances - 1)
/* H323 instancing models */
#define H323_MULTI_INST 	(nh323Instances > 1)
#define H323_SINGLE_INST	(nh323Instances == 1)

#define SrcARQEnabled()		allowAuthArq
#define IsInstanceSGK()		((nh323Instances > 1) && (UH323Globals()->instance == SGK_MINSTANCE_ID))

#define MAX_ENCODEDECODE_BUFFERSIZE 1024
#define COUNTRY_CODE_181 			181
#define MANUFACTURER_CISCO 			18

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef abs
#define abs(a) ( ((a)>(0))?(a):(-(a)) )
#endif

/* User H.323 include file */
typedef struct
{
	HAPP    hApp;
	HAPP    peerhApp;	// peer's stack instance

	int 	maxCalls;
	int		nCalls;

	int		instance;	// just the instance no.
	int		setupRate;	// Rate per second for incoming calls
	int		arqRate;	// Rate per second for incoming ARQs

	int		selims;		// milli secs spent in seli loop
	int		bridgems;	// milli secs spent in bridge loop

	time_t		arqLast;
	int		narqs;

	time_t		setupLast;
	int		nsetups;
	
	int		reserved[10];	// reserved space

	cmTransportAddress sigAddr;
	cmTransportAddress rasAddr;
  	RvH323ThreadHandle threadId;
} UH323_tGlobals;

typedef struct 
{
	unsigned char	IEIdentifier;
	unsigned char	IELength;
	union
	{
		struct	
		{
			unsigned char	location:4;
			unsigned char	spare:1;
			unsigned char	codingstandard:2;
			unsigned char	ext:1;
		}data;	
		unsigned char char_data;
	}byte_3rd;
	union 
	{
		struct	
		{
			unsigned char	causeCode:7;
			unsigned char	ext:1;
		}data;	
		unsigned char char_data;
	}byte_4th;
}CauseIE;

extern UH323_tGlobals *uh323Globals;
NetoidSNKey sgkSN;

char *liIpToString(UINT32 ipAddr,char* buf);

int UH323Init(void);

//#define UH323Globals()		(&(localConfig.uh323Globals))
UH323_tGlobals * UH323Globals(void);

#define H323Enabled()		(h323AdminStatus)

#include "sconfig.h"

extern	HPST hSynAlerting;
extern	HPST hSynVideo;
extern	HPST hSynData;
extern  HPST hSynBearerCap; 
extern  HPST hPstSetup; 
extern 	int	 PINodeId;
void GkEnd(void *);
void GwEnd(void *);

#define localTCSId	(*(_localTCSId()))
#define emptyTCSId	(*(_emptyTCSId()))
#define gcpma		_gcpma()
#define gcpmu		_gcpmu()
#define g729a		_g729a()
#define g729		_g729()
#define g723		_g723()
#define g723SS		_g723SS()
#define g728		_g728()
#define g729b		_g729b()
#define g729awb		_g729awb()
#define dtmfDataHandle (*(_dtmfDataHandle()))
#define dtmfStringDataHandle (*(_dtmfStringDataHandle()))
#define faxDataHandle (*(_faxDataHandle()))
#define mswNonStandard		_mswNonStandard()
#define faxLucentDataHandle (*(_faxLucentDataHandle()))

#define rfc2833DataHandle (*(_rfc2833DataHandle()))

extern int *_localTCSId();
extern int *_emptyTCSId();
extern int *_gcpma();
extern int *_gcpmu();
extern int *_g729a();
extern int *_g729();
extern int *_g723();
extern int *_g723SS();
extern int *_g728();
extern int *_g729b();
extern int *_g729awb();
extern int *_dtmfDataHandle();
extern int *_dtmfStringDataHandle();
extern int *_faxDataHandle();
extern int *_mswNonStandard();
extern int *_faxLucentDataHandle();
extern int *_rfc2833DataHandle();

int freeNodeTree(HAPP hApp, int nodeId, int hasroot);

#define GKNOCAUSE		0
#define GKNO225REASON	0
#define GKNORASREASON	0

int GkCallDropReason(HCALL hsCall, int callerror, int h225Reason, 
						int rasReason, int cause);

#define GkCallDropReasonNone(hsCall, callerror) \
		GkCallDropReason(hsCall, callerror, GKNO225REASON, GKNORASREASON, GKNOCAUSE)

#define GkCallDropLRJReason(hsCall, callerror) \
		(GkCallDropReasonNone(hsCall, callerror)>>16)

#define GkCallDropARJReason(hsCall, callerror) \
		(GkCallDropReasonNone(hsCall, callerror)&0xff)

#define GkCallDropReasonRAS(hsCall, callerror, rasReason) \
		GkCallDropReason(hsCall, callerror, GKNO225REASON, rasReason, GKNOCAUSE)

#define GkCallDropReasonSig(hsCall, callerror, h225Reason, cause) \
		GkCallDropReason(hsCall, callerror, h225Reason, GKNORASREASON, cause)

extern int uh323ready;

int GkMapCallErrorToCauseCode( int callerror );
int GkMapCauseCode(	int cause );

void GkMapCallErrorToH225nRasReason( int callerror, int *h225Reason, int *rasReason );
int UH323GlobalsInitKey(void);
int UH323GlobalsInit(HAPP hApp, int instance);
int UH323AllocVarsKeys(void);
int UH323InitVarsKeys(void);
int uh323InitVars();
int uh323InitPI();
int UH323GlobalsAttach(int instance);
int UH323InitInstance(char *valfile, int instance);
int UH323LoggingInit(void);
int UH323InitRealms(void);
int UH323CBInit(int instance);
int UH323DetermineBestSigAddr(cmTransportAddress *dstSignalAddr);


typedef struct
{
	int	t35CountryCode;
	int	t35Extension;
	int	manufacturerCode;	
} h221VendorInfo;

int GetVendorFromH221Info(h221VendorInfo *ns);
int uh323GetLocalCaps();
int UH323UpdateStats(time_t *timeLast, time_t *now, int *ntotal);
int uh323StackLock(void);
int uh323StackUnlock(void);
void generateOutgoingH323CallId (char *callID, int isDestSgk);
int iServerUpdateH323Allocations(struct Timer* t);
int UH323Reconfig(void);
int UH323InitCont(void);
int UH323InitThread(int outid);
int UH323Ready(int id);
extern int UH323RealmReconfig (unsigned long rsa);	

#endif /* _u_h323_h_ */
