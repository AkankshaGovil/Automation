#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include "cgen.h"
#include "mgen.h"
#include "serverp.h"
#include "clist.h"
#include "nxosd.h"
#include "expect.h"
#include "ixia.h"
#include <sys/types.h>
#include <sys/stat.h>
//#include <scmrpc_common.h>
#include "q931asn1.h"
//#include "ls.h"

#include "bits.h"
#include "ipc.h"

#include "srvrlog.h"
#include "fdsets.h"
#include "timer.h"
#include "connapi.h"
#include "lsconfig.h"
#include "sconfig.h"
#include "mem.h"
#include "ipc.h"

#include "uh323.h"
//#include "uh323cb.h"

#include "gis.h"
#include "uh323inc.h"
#include "callsm.h"
#include "h323realm.h"
#include "nxosd.h"
//#include "evt_mgmt.h"
#include <malloc.h>
#include "gk.h"

#include "log.h"
#include "stkutils.h"
#include "bridge.h"
#include "callid.h"
//#include "ls.h"
#include "ipstring.h"
//#include "h323stack.h"
#include "q931asn1.h"

extern UH323_tGlobals *uh323Globals;
#define MAX_ENCODEDECODE_BUFFERSIZE 1024
#define COUNTRY_CODE_181                        181
#define MANUFACTURER_CISCO                      18
#define GTD_SERVICECONTROLFILE "/tmp/gtd.cnf"
#define GTD_STRING "GTD"
#define GTD_LEN    3

extern errno;
extern long hostid;
HAPP	hApp = NULL;
HCFG	hCfg = NULL;

int
encodeACFNonStandardInfo(
			IN	HRAS	hsRas,
			IN      int     contentNodeId);

int g_isServiceCtrl = 1;
int
attachServiceControl(
			IN OUT	HPVT   hVal,
			IN	int    altNodeId,
			IN      int    isACF);

static char version[] = "$Id: cgen.c,v 1.38.2.47.22.4.10.1 2007/04/20 11:02:49 ychathley Exp $";

/* START: for ASN.1 printing */
char    autoReg = 0;
int level; /* Counts the depth of the recursion */
void dumpMessageSyntax(FILE *dumpFile, int nodeId, BOOL inMsg);

EpInfoStruct           ep_list[10];
SerCtrlInfoStruct      srCtrl_list[1000];
GtdCollectionStruct    gtd_list[20];

void read_ep_list_cfg_file(int *ep_count);
int  g_sr_count;
void read_serviceControl_list_cfg_file();
void getGTDData();

int                     g_hairPin=1;
char        		g_destGWAddr[256];
unsigned long		g_destGWAddrLong = 0;

int  getAliasType(char *aliasStr, int ep_id);
char gk_cfg_file[256] = "h323_ep_cfg_list";

char serviceCtrl_cfg_file[256] = "serviceCtrl_cfg";

char gkMode = 0;
#define RAS_RETURN_PATH(path) \
{                                                                       \
    static INT16 fieldPath[] = path;                                    \
    return fieldPath;                                                   \
}

/* END: for ASN.1 printing */

#define printf

// Used for debugging

/* PROTOTYPES of RAS CALLBACKS */
int cmEvRASRequest(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmRASTransaction	transaction,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall) 
{
    int rc = -1;

    switch (transaction)
    {
        case cmRASLocation:
            genHandleLRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
            cmRASConfirm(hsRas);
            cmRASClose(hsRas);
            return 1;

        case cmRASAdmission:

            rc = genHandleARQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	    //fprintf(stdout, "After genHandleARQ [%d]\n",rc);

	    if(rc <= 0)
	    {
	        cmRASReject(hsRas, rc);
	        fprintf(stdout, "Sent ARJ\n");
	    }
	    else
	    {
                cmRASConfirm(hsRas);
	        fprintf(stdout, "Sent ACF\n");
	    }

            cmRASClose(hsRas);
	    fprintf(stdout, "After cmRASClose\n");
            return 1;

        case cmRASRegistration:
            genHandleRRQ(hsRas, hsCall, lphaRas, srcAddress, haCall);
	    fprintf(stdout, "After genHandleRRQ\n");
            if(cmRASConfirm(hsRas) < 0)
	    {
	        fprintf(stdout, "Error in sending RCF\n");
	    }
	    else
	    {
	        fprintf(stdout, "After sending RCF\n");
	    }
            cmRASClose(hsRas);
	    //fprintf(stdout, "After cmRASClose\n");
            return 1;

        default :
            return -1;
    }
}

SCMRASEVENT cmRASEvent = { cmEvRASRequest, 0, 0, 0 };

//
//	Channel structures used to keep
//  track of channels associated with
//	calls.
//

typedef struct
{
	HCHAN					hsChan;			// Stack Channel Handle
	HAPPCALL				haCall;			// Current call's application handle
	int						ip;				// RTP ip value for the channel
	int						port;			// RTP port value for the channel
	int						chNum;
	cmChannelState_e		state;
	cmChannelStateMode_e	stateMode;
	int						rate;
	int						handle;
	int						dynaPayloadType;
	int						MediaLoop;
	char					channelName[50];
	int 					mgenFd;
	int						pendingMode;
} Channel;

typedef struct _call
{
	struct _call		*prev, *next;
	HCALL				hsCall;				// Stack Call handle
	int					cNum;
	cmCallState_e		state;
	cmCallStateMode_e	stateMode;
	cmControlState		controlState;
	cmControlStateMode	controlStateMode;
	int					fastStart;
	Channel				chIn[1];
	Channel				chOut[1];
	int					success;

	time_t				tsOriginate;	// TCP connection was originated
	time_t				tsUse;			// Call Entry was used up
	time_t				tsSetup;		// Setup was originated
	time_t				tsProceeding;
	time_t				tsRingBack;
	time_t				tsConnect;
	time_t				tsDrop;			// Call was dropped
	time_t				tsIdle;			// Call went to idle state

	int					ntimes;		// no of times call make was called
	int					dofax;

	time_t				timeNextEvt;		// Duration timer
	hrtime_t			hrOriginate;
	hrtime_t			hrSetup;
	hrtime_t			hrProceeding;
	hrtime_t			hrRingBack;
	hrtime_t			hrConnect;

	int 				nulltcs;		// Received NULL TCS
} Call;

//int dns_recovery_timeout = 120;
struct pollfd*      h323_pollfd_array;
int32_t             h323_fds_inuse;

int                 idaemon;
cmTransportAddress  trAddr;
int                 h323Initialized;
int                 reportAllDisconnects;

Call*				Calls;
Call*				CallsOut;

char 				startCallingPn[256], startCalledPn[256];
char 				callingpn[256],	calledpn[256];
char**				callingpna;
char**				calledpna;

int					fax;

int					incCallingPn;
int					incCalledPn;
int					multipleCalls;
int					shutEmptyConnection = 1;

int					CurrentCalls;
int					TotalCalls;

int					CurrentCallsOut;
int					TotalCallsOut;

int		 			nChannelsIn;
int					nChannelsOut;

int					nErrors;
int					nRelComps;
int					nFailedSetups;
int					nFailedProceeding;
int					nFailedRingBack;
int					nFailedFaststarts;

int					burst;				// Number of calls in a burst
int					burstInterval = 5;	// milliseconds between bursts

int                                     readcallingpnfromfile=0;
FILE                            *srcfp; //file pointer to read src nums file
int                                     readcalledpnfromfile=0;
FILE                            *dstfp; //file pointer to read dst nums file


uint32_t			tdMonitor;
uint32_t			tdSetup;
uint32_t			tdProceeding;
uint32_t			tdRingBack;
uint32_t			tdConnect;
uint32_t			tdCallDuration;
uint32_t			tdIdle;

char				codecs[5][25] = { "g711Ulaw64k", "g7231", "g729", "g729AnnexA", "g711Alaw64k" };
int					ncodecs = 2;

LsMemStruct 	 	*lsMem;

int					mode = MODE_RECEIVE;
int                 automode;   
int					fastStart = 1;
int					doh245 = 1;		// Default is to do fast start and H.245
int					asrMode = 0;
int                             	maxCallsMode = 0;
double                          	iprobability;   // probability of dropping setups
int					h245TunnFlag = 0;

int					dropISDN;

int					isdnCode = 16;
int					nCalls = 25;
int                             	totalMaxCalls;
int					xCalls;
static					int callCounterTx = 0;

char				Destination[256];

char				userUserStr[] = "NexTone Test Application";
char				displayStr[] = "NexTone Test Application";

int					debug;
int					rvdebug;
int					maxfds = 1024;
int					rvmaxfds;			// Radvision Maximum Fds
int					stats;

int					mgenServerFd = -1;
int					mgenFd = -1;
MgenStats			mgenStats;
char				*mstatArg =NULL;

static int32_t		startPort = 2326;

unsigned long		chanLocalAddr;
unsigned short		chanLocalPort = 49200;
unsigned short		mediaStartPort = 49200;
unsigned short		mgenPort = 49160;

char        		gwAddr[256];
unsigned long		gwAddrLong;
int					gwAddrFlag = 0;
unsigned short		gwPort = 1720;

char 				otg[256];	// orig trunk group
char 				dtg[256];	// dest trunk group

unsigned char 		h323Id[256] = "change-me";	// H323 ID

extern char 		buildDate[];
Call				*timerHead = NULL;

#define NOTIFY_READ		0
#define NOTIFY_WRITE	1
int 				notifyPipe[2];
int					wave;
int					errorScenario = Scene_eNone;

// Mgen options
char 			mgenPayloadLen[8] = "30";	
char 			mgenPayloadInterval[32] = "30000000"; 
char			mgenNumThreads[8] = "1";
char			mgenTxRxSamePort[4];
char			mgenLoopback[4];

//
// Counters used to keep track of
// call state changes and callbacks
//

callback_counters_t	cb_counters;

int					cmCallNew_counter;
int					cmCallNewFailed_counter;
int					cmCallMake_counter;
int					cmCallMakeFailed_counter;
int					cmFailedSetups;
int					cmFailedFastStarts;

FILE*				trace_desc,*stats_desc;

cv_t				exit_cgen_cv =
						{	PTHREAD_MUTEX_INITIALIZER,
							PTHREAD_COND_INITIALIZER,
						THRD_INIT };

cv_t				main_loop_cv =
						{	PTHREAD_MUTEX_INITIALIZER,
							PTHREAD_COND_INITIALIZER,
							THRD_INIT };

cv_t				callgen_complete_cv =
						{	PTHREAD_MUTEX_INITIALIZER,
							PTHREAD_COND_INITIALIZER,
							THRD_INIT };

int32_t				exit_main_loop;
int32_t				shutdown_calls;
int32_t				relcomp;
pthread_t			mainloop_thread;
pthread_t			callgen_thread;
//pthread_t			callmon_thread;
pthread_t			stdinLoop_thread;
pthread_t			mgen_thread;
int32_t				no_more_status;
int32_t				max_nready;

sigset_t			threads_signal_mask;

int32_t				module_count = 0;
char 				modules[24][256];

trace_table_t		h323_trace_tbl;
FILE*				h323_fdesc;

#ifndef NETOID_LINUX // RT not to be ported to linux
pcinfo_t			rt_pcinfo;
#endif //NETOID_LINUX

char				config_file[256] = "config.val";
char				config_template_file[256] = "config-template.val";

char				mgen_file[256] = "mgen";

int					maxCalls;
int					maxChannels;
int					channels;
int					messages;
int					vtNodeCount;
int					protocols;
int					maxProcs;
int					tpktChans;

int					call_count = 0;
float				call_rate = 0;
int                 sendFaxOLC = 0;
char                e164Id[256] = "oops";
unsigned long       localIp;
char                localIpStr[5];
char                localIpFlag = 0;
int                 localRasPort = 1719;
int                 localQ931Port = 1720;
char                localPortFlag = 0;
char                gwIpStr[5];
char                h323AliasCmdLine = 0;
char                e164AliasCmdLine = 0;
int                 maxCalls = 0;

hrtime_t			callTimePeriod, hrdiff = 0, hrtimeLast = 0;

ixiaInfo            ixiaGlobal;

char				ixia_testScriptName[IXIA_TESTSCRIPT_NAME_LEN];
char				xferTgtNum[256];
int					xferMode = 0, setModeTransfer= 0 , startTransfer = 0, waitToResume = 0;

#ifdef _DEBUG_MALLOC_INC

	union	dbmalloptarg m;
	extern	int	malloc_preamble;

#endif

// Forward Function Declarations

void		ParseArguments( int argc, char **argv );
int 		ReadInput( void );
void		PrintCallbackCounters( void );
int			PrintStatus( int debug );
int			PrintCallStats( Call *call, int stats);
int			getNextPhone( char *phone );
int			ResetCall( Call * call );
char*		ULIPtostring(unsigned long ipaddress);
int			NetSetMaxFds( int32_t numfds );
// void     thread_set_rt( int32_t ns_quantum, int16_t priority );
void*		pollArrayInit( int32_t poll_array_size );

void*		callgenThread( void *arg );
void*		launchMainLoop( void *arg );
// void*        callMonitorThread(void *arg);
void*		stdinLoop(void	*	data);
void*		mgenServerThread(void *arg);

void		sigdummy_handler( int signo );

void		SpawnOutgoingCall( int i );

static void SignalInit(void);

static void	PrintUsage( void );
static void     GenerateSeed();
static void 	PrintErrorTime();
static void 	PrintCallSummary();
static void 	PrintInteractiveCmds();
static void 	PrintMgenOptions();
static int	SendUserInput(HCALL hsCall, int nodeId);
static int	SendSignalDtmf(Call *call, char *signalType, int duration);
static int	SendNewTCS(Call *call, char *input, int media_ip, int media_port);
static int	SendAlphaDtmf(Call *call, char *alphanumeric);
static int	IsCallConnected(Call *call);
static pid_t	StartMgen();
static int GetH323IDFromConfig();
static int	PrintMgenStats();
int			getNextPhoneFromFile(char *,FILE *);
int SendMgenDtmf(char *signalType, int duration, int volume);

/* compilation */
MemoryMap *map;
char pidfile[128];

int
HandleIdleChannelState(HCHAN hsChannel, Channel *channel)
{
	BOOL origin;

	cb_counters.channel_state.StateIdle++;

	if ( cmChannelGetOrigin(hsChannel, &origin) < 0 )
	{

        PrintErrorTime();
        fprintf(stderr, "HandleIdleChannelState(): ");
        fprintf(stderr, "cmChannelGetOrigin() failed\n");
	}

	if ( cmChannelClose(hsChannel) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "HandleIdleChannelState(): ");
        fprintf(stderr, "cmChannelClose() failed\n");
	}

	cmChannelSetHandle(hsChannel, NULL);

	if (channel)
	{
		channel->dynaPayloadType = 0;
		channel->hsChan = NULL;
		channel->state = cmChannelStateIdle;

		if (origin)
			nChannelsOut--;
		else
			nChannelsIn--;
	}

	return 0;
}

int
TerminateCall(Call *call, int origin)
{
	if (call)
	{
		if (origin == 1)
		{
			// Delete items from the call list
			CallDeleteFromTimerList(call);
		}

		ResetCall( call );

		time( &call->tsIdle );

		mgenInform(call, 0);

		if (origin == 0)
			CurrentCalls--;
		else
			CurrentCallsOut--;

	}
}

/*
// One single step to kill a call
int
EndCall(Call *call, int origin)
{
	mgenInform(call, 0);
	time( &call->tsIdle );

	if (call->chIn[0].hsChan)
	{
		nChannelsIn--;
	}
	
	if (call->chOut[0].hsChan)
	{
		nChannelsOut--;
	}

	if (origin == 0)
		CurrentCalls--;
	else
		CurrentCallsOut--;

	cmCallSetHandle(call->hsCall, NULL);
	cmChannelSetHandle(call->chIn[0].hsChan, NULL);
	cmChannelSetHandle(call->chOut[0].hsChan, NULL);
	call->chIn[0].hsChan = NULL;
	call->chOut[0].hsChan = NULL;
	call->chIn[0].state = cmChannelStateIdle;
	call->chOut[0].state = cmChannelStateIdle;
}
*/

int
HandleIdleCallState(HCALL hsCall, Call *call)
{
	int32_t	retval;
	BOOL 	origin;

	if ((retval = cmCallGetOrigin( hsCall, NULL) ) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "HandleIdleCallState(): ");
        fprintf(stderr, "cmCallGetOrigin() failed for call # %d\n",
                call->cNum);
	}

	origin = (BOOL) retval;

	cb_counters.call_state.StateIdle++;

	PrintCallStats(call,stats);

	cmCallSetHandle(hsCall, NULL);

	if (cmCallClose(hsCall) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "HandleIdleCallState(): ");
        fprintf(stderr, "cmCallClose() failed for call# %d\n",
                call->cNum);
	}

	TerminateCall(call, origin);

	// Add the call back to the timer list
	if (origin && call && tdCallDuration)
	{
		time(&call->timeNextEvt);
		call->timeNextEvt += tdIdle;
		//CallInsertIntoTimerList(call);
	}

	return 0;
}

int
HangupCalls()
{
	Call *call;
	time_t now, delta = 0;
	int i, nrelcomps = 0;

	for (i = 0; i < nCalls; i++)
	{
		call= &CallsOut[i];

		if (call->hsCall)
		{
			time(&call->tsDrop);

			// If in FAX mode, before tearing down calls, revert to voice mode.
                if (fax)
                {

                        cmReqModeEntry  entry = { "g711Ulaw64k", -1 };
                        cmReqModeEntry  *desc[] = { &entry, 0 };
                        cmReqModeEntry  **modes[] = { desc, 0 };
                        int modeFaxId;

                        // Send a mode status change request
                        modeFaxId = cmRequestModeBuild(hApp, modes);
                        if (modeFaxId < 0)
                        {
                                PrintErrorTime();
                                fprintf(stderr, "cmRequestModeBuild() failed\n");
                        }
                        if (cmCallRequestMode(call->hsCall, modeFaxId) < 0)
                        {
                                PrintErrorTime();
                                fprintf(stderr, "cmCallRequestMode() failed\n");
                        }
                }

			cmCallDrop( call->hsCall );
			millisleep( callTimePeriod/1000000);
			nrelcomps ++;
		}
	}

	for (i = 0; i < nCalls; i++)
	{
		call= &Calls[i];

		if (call->hsCall)
		{
			time(&call->tsDrop);

			// If in FAX mode, before tearing down calls, revert to voice mode.
                if (fax)
                {

                        cmReqModeEntry  entry = { "g711Ulaw64k", -1 };
                        cmReqModeEntry  *desc[] = { &entry, 0 };
                        cmReqModeEntry  **modes[] = { desc, 0 };
                        int modeFaxId;

                        // Send a mode status change request
                        modeFaxId = cmRequestModeBuild(hApp, modes);
                        if (modeFaxId < 0)
                        {
                                PrintErrorTime();
                                fprintf(stderr, "cmRequestModeBuild() failed\n");
                        }
                        if (cmCallRequestMode(call->hsCall, modeFaxId) < 0)
                        {
                                PrintErrorTime();
                                fprintf(stderr, "cmCallRequestMode() failed\n");
                        }
                }

			cmCallDrop( call->hsCall );
			millisleep( callTimePeriod/1000000);
			nrelcomps ++;
		}
	}

	return 1;
}

/*
// Not being used currently
void *
callMonitorThread(void *arg)
{

	Call *call;
	time_t now, delta = 0;
	char timestr[120];
	int i, newCalls, n, cted, dropCall;

//	cv_wait( &main_loop_cv, THRD_READY );
//	millisleep(1000);

	if ( shutdown_calls )
		return( (void*) NULL );

//	cv_wait( &callgen_complete_cv, THRD_DONE );

	if ( shutdown_calls )
		return( (void*) NULL );

//	thread_set_rt( 50000000, 30 );

_start:

//	sleep(tdMonitor);

	if ( shutdown_calls )
		return( (void*) NULL );

	time( &now );

	if (mode & MODE_TRANSMIT)
	{
		newCalls = nCalls - CurrentCallsOut;
	}
	else
	{
		newCalls = nCalls - CurrentCalls;
	}

//	fprintf(stdout, "Monitor: %d calls left\n", newCalls);

	// At most newCalls will get spawned out
	n = 0;
	cted = 0;

	// loop around the calls and monitor timestamps

	for (i = 0; i < nCalls; i++)
	{
		call= &CallsOut[i];
		dropCall = 0;

		if (call->tsUse && call->tsOriginate && !call->tsDrop)
		{
			// This call is not in the idle state,
			// setup connection has been originated for this call

			delta = now - call->tsOriginate;
			if (delta >= tdConnect)
			{
				// This call needs to be disconnected
				dropCall = 1;	
			}
		}

		switch (call->state)
		{
		case cmCallStateDialtone:

			if (call->tsSetup == 0)
			{
                fprintf(stderr, "call not stamped in setup state\n");
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			delta = now - call->tsSetup;

			//if (dropCall || ( delta > tdSetup ))
			if (dropCall)
			{
				nErrors++;

				nFailedSetups++;
				cmFailedSetups++;

				cftime(timestr,"%Y,%m,%d %T",&now);
                fprintf(stderr, "abandon error(%s): call#%d(%d), or %d su %d, pr %d al %d co %d\n", 
					timestr,
					call->cNum, 
					call->ntimes,
					call->tsOriginate?now - call->tsOriginate:-1, 
					call->tsSetup?now - call->tsSetup:-1, 
					call->tsProceeding?now - call->tsProceeding:-1, 
					call->tsRingBack?now - call->tsRingBack:-1,
					call->tsConnect?now - call->tsConnect:-1);
				time(&call->tsDrop);
				cmCallDrop( call->hsCall );

			}

			break;

		case cmCallStateProceeding:
			if (call->tsProceeding == 0)
			{
                fprintf(stderr, "call not stamped in proceeding state\n");
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			delta = now - call->tsProceeding;

			//if (dropCall || ( delta > tdProceeding ))
			if (dropCall)
			{
				nErrors++;

				nFailedProceeding++;

				cftime(timestr,"%Y,%m,%d %T",&now);
                fprintf(stderr, "abandon error(ts=%s): call#%d(%d), or %d su %d, pr %d al %d co %d\n", 
					timestr,
					call->cNum, 
					call->ntimes,
					call->tsOriginate?now - call->tsOriginate:-1, 
					call->tsSetup?now - call->tsSetup:-1, 
					call->tsProceeding?now - call->tsProceeding:-1, 
					call->tsRingBack?now - call->tsRingBack:-1,
					call->tsConnect?now - call->tsConnect:-1);

				time(&call->tsDrop);
				cmCallDrop( call->hsCall );
			}

			break;

		case cmCallStateRingBack:

			if (call->tsRingBack == 0)
			{
                fprintf(stderr, "call not stamped in ringback state\n");
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			delta = now - call->tsRingBack;

			//if (dropCall || ( delta > tdRingBack ))
			if (dropCall)
			{
				nErrors++;

				nFailedRingBack++;
                fprintf(stderr, "abandon error(ts=%d): call#%d(%d), or %d su %d, pr %d al %d co %d\n", 
                        now,
                        call->cNum, 
                        call->ntimes,
                        call->tsOriginate?now - call->tsOriginate:-1, 
                        call->tsSetup?now - call->tsSetup:-1, 
                        call->tsProceeding?now - call->tsProceeding:-1, 
                        call->tsRingBack?now - call->tsRingBack:-1,
                        call->tsConnect?now - call->tsConnect:-1);

				time(&call->tsDrop);
				cmCallDrop( call->hsCall );
			}

			break;

		case cmCallStateConnected:

			if (call->tsDrop)
			{
				break;
			}

			cted ++;
			if ( call->tsConnect )
				delta = now - call->tsConnect;

			if (dropCall || ( delta > tdConnect ))
			{
				if (fastStart != call->fastStart)
				{
					nErrors++;
					nFailedFaststarts++;
					cmFailedFastStarts++;
				}
				cftime(timestr,"%Y,%m,%d %T",&now);
                fprintf(stderr, "abandon error(ts=%s): call#%d(%d), or %d su %d, pr %d al %d co %d\n", 
					timestr,
					call->cNum, 
					call->ntimes,
					call->tsOriginate?now - call->tsOriginate:-1, 
					call->tsSetup?now - call->tsSetup:-1, 
					call->tsProceeding?now - call->tsProceeding:-1, 
					call->tsRingBack?now - call->tsRingBack:-1,
					call->tsConnect?now - call->tsConnect:-1);

				time(&call->tsDrop);
				cmCallDrop(call->hsCall);
			}
			break;

		case cmCallStateIdle:

			if (call->tsUse)
			{
				break;
			}

			if ( call->tsIdle )
				delta =  now - call->tsIdle;

			if (( delta > tdIdle ) && 
				(!call->tsDrop) &&
				(n < newCalls) &&
				(call->chIn[0].state == cmChannelStateIdle) && 
				(call->chOut[0].state == cmChannelStateIdle))
			{
				// Initiate the call again
		
				// We should Reset the whole call handle at this point
				// as we are done using any previous state (tsIdle for e.g.)
				ResetCall( call );

				SpawnOutgoingCall(i);
				n++;
			}
			break;

		default:
			break;
		}

		if ( shutdown_calls )
			return( (void*) NULL );
	}

//	fprintf(stderr, "Monitor: %d calls placed\n", n);

//	goto _start;
}
*/

//*
//* Radvision H323 Callback Structures and
//* Function Declarations
//*

#ifdef _WIN32
	BOOL CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND hwnd;
#endif

int CALLCONV cmEvNewCall(IN HAPP hApp, IN HCALL hsCall, IN LPHAPPCALL lphaCall);

int CALLCONV cmEvRegEvent(IN HAPP hApp,
						  IN cmRegState regState,
						  IN cmRegEvent regEvent, IN int regEventHandle);

//
// Structure for registration of General Events
//

SCMEVENT cmEvent =
{	cmEvNewCall,
	cmEvRegEvent
};

int CALLCONV cmEvCallStateChanged(	IN HAPPCALL haCall,
								  	IN HCALL hsCall,
									IN UINT32 state,
									IN UINT32 stateMode);

int CALLCONV cmEvCallNewRate(	IN HAPPCALL haCall,
								IN HCALL hsCall,
								IN UINT32 rate);

int CALLCONV cmEvCallInfo(	IN HAPPCALL haCall,
						  	IN HCALL hsCall,
						  	IN char *display,
							IN char *userUser,
							IN int userUserSize);

int CALLCONV cmCallNonStandardParam(	IN HAPPCALL haCall,
										IN HCALL hsCall,
										IN char *data,
										IN int dataSize);

int CALLCONV cmEvCallFacility(	IN HAPPCALL haCall,
							  	IN HCALL hsCall,
								IN int handle,
								OUT IN BOOL * proceed);

int CALLCONV cmEvCallFastStartSetup(	IN HAPPCALL haCall,
										IN HCALL hsCall,
										OUT IN cmFastStartMessage * fsMessage);

int CALLCONV cmEvCallStatus(
    IN      HAPPCALL haCall,
    IN      HCALL hsCall,
    OUT IN  cmCallStatusMessage *callStatusMsg);

//
// Structure for registration of Call Events
//

SCMCALLEVENT cmCallEvent =
{	
	cmEvCallStateChanged,
	cmEvCallNewRate,
	cmEvCallInfo,
	cmCallNonStandardParam,
	cmEvCallFacility,
	cmEvCallFastStartSetup,
};

int CALLCONV cmEvCallCapabilities(IN HAPPCALL haCall,
								  IN HCALL hsCall, IN cmCapStruct * capabilities[]);

int CALLCONV cmEvCallCapabilitiesExt(IN HAPPCALL haCall,
									 IN HCALL hsCall, IN cmCapStruct *** capabilities[]);

int CALLCONV cmEvCallNewChannel(IN HAPPCALL haCall,
								IN HCALL hsCall,
								IN HCHAN hsChan, OUT LPHAPPCHAN lphaChan);

int CALLCONV cmEvCallCapabilitiesResponse(IN HAPPCALL haCall,
										  IN HCALL hsCall, IN UINT32 status);

int CALLCONV cmEvCallMasterSlaveStatus(IN HAPPCALL haCall,
									   IN HCALL hsCall, IN UINT32 status);

int CALLCONV cmEvCallRoundTripDelay(IN HAPPCALL haCall, IN HCALL hsCall, IN INT32 delay);

int CALLCONV cmEvCallUserInput(IN HAPPCALL haCall,
							   IN HCALL hsCall, IN INT32 userInputId);

int CALLCONV cmEvCallRequestMode(IN HAPPCALL haCall,
								 IN HCALL hsCall,
								 IN cmReqModeStatus status, IN INT32 nodeId);

int CALLCONV cmEvCallMiscStatus(IN HAPPCALL haCall,
								IN HCALL hsCall, IN cmMiscStatus status);

int CALLCONV cmEvCallControlStateChanged(	IN HAPPCALL haCall,
										 	IN HCALL hsCall,
										 	IN cmControlState state,
											IN cmControlStateMode stateMode);

int CALLCONV cmEvCallMasterSlave(IN HAPPCALL haCall,
								 IN HCALL hsCall,
								 IN UINT32 terminalType,
								 IN UINT32 statusDeterminationNumber);

//
// Structure for registration of Control Events
//

SCMCONTROLEVENT cmControlEvent =
{		cmEvCallCapabilities,
		cmEvCallCapabilitiesExt,
		cmEvCallNewChannel,
		cmEvCallCapabilitiesResponse,
		cmEvCallMasterSlaveStatus,
		cmEvCallRoundTripDelay,
		cmEvCallUserInput,
		cmEvCallRequestMode,
		cmEvCallMiscStatus,
		cmEvCallControlStateChanged,
		cmEvCallMasterSlave
};


int CALLCONV cmEvChannelStateChanged(	IN HAPPCHAN haChannel,
										IN HCHAN hsChannel, 
										IN cmChannelState_e state, 
										IN cmChannelStateMode_e stateMode);

int CALLCONV cmEvChannelNewRate(IN HAPPCHAN haChan, IN HCHAN hsChan, IN UINT32 rate);

int CALLCONV cmEvChannelMaxSkew(IN HAPPCHAN haChan1,
								IN HCHAN hsChan1,
								IN HAPPCHAN haChan2, IN HCHAN hsChan2, IN UINT32 skew);

int CALLCONV cmEvChannelSetAddress(IN HAPPCHAN haChan,
								   IN HCHAN hsChan, IN UINT32 ip, IN UINT16 port);

int CALLCONV cmEvChannelSetRTCPAddress(IN HAPPCHAN haChan,
									   IN HCHAN hsChan, IN UINT32 ip, IN UINT16 port);

int CALLCONV cmEvChannelParameters(IN HAPPCHAN haChan,
								   IN HCHAN hsChan,
								   IN char *channelName,
								   IN HAPPCHAN haChanSameSession,
								   IN HCHAN hsChanSameSession,
								   IN HAPPCHAN haChanAssociated,
								   IN HCHAN hsChanAssociated, IN UINT32 rate);

int CALLCONV cmEvChannelRTPDynamicPayloadType(IN HAPPCHAN haChan,
											  IN HCHAN hsChan,
											  IN INT8 dynamicPayloadType);

int CALLCONV cmEvChannelVideoFastUpdatePicture(IN HAPPCHAN haChan, IN HCHAN hsChan);

int CALLCONV cmEvChannelVideoFastUpdateGOB(IN HAPPCHAN haChan,
										   IN HCHAN hsChan,
										   IN int firstGOB, IN int numberOfGOBs);

int CALLCONV cmEvChannelVideoFastUpdateMB(IN HAPPCHAN haChan,
										  IN HCHAN hsChan,
										  IN int firstGOB,
										  IN int firstMB, IN int numberOfMBs);

int CALLCONV cmEvChannelHandle(IN HAPPCHAN haChan,
							   IN HCHAN hsChan,
							   IN int dataTypeHandle, IN cmCapDataType dataType);


int CALLCONV cmEvChannelGetRTCPAddress(IN HAPPCHAN haChan,
									   IN HCHAN hsChan,
									   IN UINT32 * ip, IN UINT16 * port);

int CALLCONV cmEvChannelRequestCloseStatus(IN HAPPCHAN haChan,
										   IN HCHAN hsChan,
										   IN cmRequestCloseStatus status);

int CALLCONV cmEvChannelTSTO(IN HAPPCHAN haChan,
							 IN HCHAN hsChan, IN INT8 isCommand, IN INT8 tradeoffValue);

int CALLCONV cmEvChannelMediaLoopStatus(IN HAPPCHAN haChan,
										IN HCHAN hsChan, IN cmMediaLoopStatus status);


int CALLCONV cmEvChannelReplace(IN HAPPCHAN haChan,
								IN HCHAN hsChan,
								IN HAPPCHAN haReplacedChannel,
								IN HCHAN hsReplacedChannel);

int CALLCONV cmEvChannelFlowControlToZero(IN HAPPCHAN haChan, IN HCHAN hsChan);

//
// Structure for registration of Channel Events
//

SCMCHANEVENT cmChannelEvent =
{	
	cmEvChannelStateChanged,
	cmEvChannelNewRate,
	cmEvChannelMaxSkew,
	cmEvChannelSetAddress,
	cmEvChannelSetRTCPAddress,
	cmEvChannelParameters,
	cmEvChannelRTPDynamicPayloadType,
	cmEvChannelVideoFastUpdatePicture,
	cmEvChannelVideoFastUpdateGOB,
	cmEvChannelVideoFastUpdateMB,
	cmEvChannelHandle,
	cmEvChannelGetRTCPAddress,
	cmEvChannelRequestCloseStatus,
	cmEvChannelTSTO,
	cmEvChannelMediaLoopStatus,
	cmEvChannelReplace,
	cmEvChannelFlowControlToZero
};

BOOL CALLCONV cmHookListen(		/*Before listen */
							  IN HPROTCONN hConn, IN int addr);


int CALLCONV cmHookListening(	/*After listen */
								IN HPROTCONN hConn, IN int addr, IN BOOL error);

int CALLCONV cmHookConnecting(	/*Before connect */
								 IN HPROTCONN hConn, IN int addr);

int CALLCONV cmHookInConn(		/*Incomming connect */
							 IN HPROTCONN hConn, IN int addrFrom, IN int addrTo);

int CALLCONV cmHookOutConn(		/*Outgoing connect */
							  IN HPROTCONN hConn,
							  IN int addrFrom, IN int addrTo, IN BOOL error);

BOOL CALLCONV cmHookSend(IN HPROTCONN hConn, IN int nodeId, IN BOOL error);
static int SendGtd( IN HAPP hApp,int nodeId,char* gtd, int gtdLen);

BOOL CALLCONV cmHookRecv(IN HPROTCONN hConn, IN int nodeId, IN BOOL error);

BOOL CALLCONV cmHookSendTo(IN HPROTCONN hConn,
						   IN int nodeId, IN int addrTo, IN BOOL error);

BOOL CALLCONV cmHookRecvFrom(IN HPROTCONN hConn,
							 IN int nodeId,
							 IN int addrFrom, IN BOOL multicast, IN BOOL error);

void CALLCONV cmHookClose(IN HPROTCONN hConn);

//
// Structure for registration of Protocol Events
//

SCMPROTOCOLEVENT cmProtocolEvent =
{	cmHookListen,
	cmHookListening,
	cmHookConnecting,
	cmHookInConn,
	cmHookOutConn,
	cmHookSend,
	cmHookRecv,
	cmHookSendTo,
	cmHookRecvFrom,
	cmHookClose
};

/* Additional helper functions */
static char *
liIpToString(UINT32 ipAddr, char *buf)
{
	BYTE *ip = (char *) &ipAddr;
	sprintf(buf, "%d.%d.%d.%d", (int) ip[0], (int) ip[1], (int) ip[2], (int) ip[3]);
	return buf;
}

int
getFreeCall()
{
	int i;

	for (i = 0; i < nCalls; i++)
		if (!Calls[i].hsCall)
			return i;
	return -1;
}

int
getFreeInChan(HAPPCALL haCall, HCHAN hsChannel)
{
	int i;

	for (i = 0; i < 1; i++)
		if ( !((Call *) haCall)->chIn[i].hsChan ||
			(((Call *) haCall)->chIn[i].hsChan == hsChannel))
			return i;
	return -1;
}

int
getFreeOutChan(HAPPCALL haCall, HCHAN hsChannel)
{
	int i;

	for (i = 0; i < 1; i++)
		if ( !((Call *) haCall)->chOut[i].hsChan ||
			(((Call *) haCall)->chOut[i].hsChan == hsChannel))
			return i;
	return -1;
}

#ifdef _WIN32
	unsigned __stdcall
	WMain(void *p)
	{

		static char szAppName[] = "CONSOLEAPPTEST";
		MSG msg;
		WNDCLASS wndclass;
		HANDLE hInstance = GetModuleHandle(NULL);


		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = "";
		wndclass.lpszClassName = szAppName;

		RegisterClass(&wndclass);

		hwnd = CreateWindow(szAppName,	// window class name
							"Test",	// window caption
							WS_OVERLAPPEDWINDOW,	// window style
							CW_USEDEFAULT,	// initial x position
							CW_USEDEFAULT,	// initial y position
							CW_USEDEFAULT,	// initial x size
							CW_USEDEFAULT,	// initial y size
							NULL,	// parent window handle
							NULL,	// window menu handle
							hInstance,	// program instance handle
							NULL);	// creation parameters

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return 0;
	}

	BOOL CALLBACK
	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		cmTransportAddress trAddr;
		char ipStr[80];

		switch (message)
		{
		case WM_CREATE:
			{
				int i,
					j;
				int error;

				for (i = 0; i < 100; i++)	// Initialize call/channel structures
				{
					Calls[i].cNum = i;
					Calls[i].state = cmCallStateIdle;
					for (j = 0; j < 10; j++)
					{
						Calls[i].chOut[j].state = cmChannelStateIdle;
						Calls[i].chOut[j].chNum = j;
						Calls[i].chOut[j].haCall = (HAPPCALL) & Calls[i];
						Calls[i].chIn[j].state = cmChannelStateIdle;
						Calls[i].chIn[j].chNum = j;
						Calls[i].chIn[j].haCall = (HAPPCALL) & Calls[i];
					}
				}

				// Initialize Radvision Syntax Tree

				error = cmInitialize(config_file, &hApp);

				if (error < 0)
				{
					MessageBox(NULL, "Cannot cmInit", "Appl", MB_OK);
					PostQuitMessage(0);
					break;
				}

				#ifdef  USE_RTP
					rtpTestInit();
				#endif

				cmSetGenEventHandler(hApp, &cmEvent, sizeof(cmEvent));
				cmSetCallEventHandler(hApp, &cmCallEvent, sizeof(cmCallEvent));
				cmSetControlEventHandler(hApp, &cmControlEvent, sizeof(cmControlEvent));
				cmSetChannelEventHandler(hApp, &cmChannelEvent, sizeof(cmChannelEvent));

				cmSetProtocolEventHandler(	hApp,
												&cmProtocolEvent,
												sizeof(cmProtocolEvent));

				error = cmGetLocalCallSignalAddress(hApp, &trAddr);

				if (!error)
                {
					fprintf(stdout, 
                            "Local Call Signaling IP/Port = %s/%d\n\n",
							liIpToString(trAddr.ip, ipStr), trAddr.port);
                }

				break;
			}

		case WM_USER:
			switch (wParam)
			{
			case 0:
				cmEnd(hApp);
				break;

			case 1:
				{
					cmAlias myAlias;
					int index,
						rc;
					int tlen;

					cmCallNew(hApp, (HAPPCALL) & Calls[0].hsCall, &Calls[0].hsCall);
					for (index = 0; index < 10; index++)
					{
						tlen = sizeof(myAlias);
						rc = cmCallGetParam(Calls[0].hsCall, cmParamSourceAddress, index,
											&tlen, (char *) &myAlias);


						if (rc >= 0 && myAlias.string)
							printf("My Alias #%d = %s\n", index, myAlias.string);
						else
						{
                            PrintErrorTime();
                            fprintf(stderr, "WndProc(): ");
                            fprintf(stderr, "cmCallGetParam() failed "
                                    "for cmParamSourceAddress\n");
							break;
						}
					}				/*  for  */


                    if (cmCallSetParam(Calls[0].hsCall,
                                       cmParamIsMultiplexed,
                                       0,
                                       multipleCalls,
                                       NULL) < 0)
                    {
                        PrintErrorTime();
                        fprintf(stderr, "WndProc(): ");
                        fprintf(stderr, "cmCallSetParam() failed "
                                "for multiplex calls\n");
                    }


					cmCallMake(	Calls[0].hsCall,
								64000,
								0,
								Destination,
								"NAME:TestC",
								displayStr,
								userUserStr,
								strlen(userUserStr) );

				}					/*  case  */
				break;
			}						/*  switch wParam */
			break;
		}							/*  switch  */

		return TRUE;
	}
#endif // _WIN32

//*===
//* Callback routines for General Events
//*===

int CALLCONV
cmEvRegEvent(	IN HAPP hApp,
				IN cmRegState regState,
				IN cmRegEvent regEvent,
				IN int regEventHandle)
{
	cb_counters.gen.EvRegEvent_counter++;

	switch (regState)
	{
	case cmRegistered:
	{
		cmTransportAddress tr;
		char buf[100];
		printf("Registered");
		fprintf(stdout, "Registration confirmed\n");
		fprintf(stdout, ">\n");

		if ( cmGetGKCallSignalAddress(hApp, &tr) < 0 )
		{
        	     PrintErrorTime(); 
		     fprintf(stderr, "cmEvRegEvent(): "); 
		     fprintf(stderr, "cmGetGKCallSignalAddress() failed\n");
		}
		else
		{
			char tel[30];

			sprintf(tel, "%d.%d.%d.%d:%d",
					(int) ((unsigned char *) &(tr.ip))[0],
					(int) ((unsigned char *) &(tr.ip))[1],
					(int) ((unsigned char *) &(tr.ip))[2],
					(int) ((unsigned char *) &(tr.ip))[3], tr.port);
			sprintf(buf, "GK CS Address is %s\n", tel);
			fprintf(stdout,buf);
		}

		if ( cmGetGKRASAddress(hApp, &tr) < 0 )
		{
        	PrintErrorTime();
            fprintf(stderr, "cmEvRegEvent(): ");
            fprintf(stderr, "cmGetGKRASAddress() failed\n");
		}

		{
			char tel[30];
			sprintf(tel, "%d.%d.%d.%d:%d",
					(int) ((unsigned char *) &(tr.ip))[0],
					(int) ((unsigned char *) &(tr.ip))[1],
					(int) ((unsigned char *) &(tr.ip))[2],
					(int) ((unsigned char *) &(tr.ip))[3], tr.port);

			sprintf(buf, "GK RAS Address is %s\n", tel);
			printf(buf);
		}
			break;
	}
	case cmDiscovered:
		printf("cmEvRegEvent: cmDiscovered\n\n");
		break;
	case cmIdle:
		printf("cmEvRegEvent: cmIdle\n\n");
		if (regEvent != cmRegistrationReject)
		{
    		PrintErrorTime();
    		fprintf(stderr, "cmEvRegEvent(): ");
    		fprintf(stderr, "Registration timed out\n");
		}
		break;
	}

	if (regEvent == cmRegistrationReject)
	{
		HPVT	hVal;
		int		retval = (int) cmGetValTree(hApp);
		int		handle;

    	PrintErrorTime();
    	fprintf(stderr, "cmEvRegEvent(): ");
    	fprintf(stderr, "Registration rejected");

		if ( retval != RVERROR )
		{
			hVal = (HPVT) retval;
			if ( (handle = pvtChild(hVal, regEventHandle)) != RVERROR )
			{
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.invalidAlias") >= 0)
					fprintf(stderr, "...invalidAlias");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.duplicateAlias") >= 0)
					fprintf(stderr, "...duplicateAlias");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.invalidTerminalType") >= 0)
					fprintf(stderr, "...invalidTerminalType");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.invalidCallSignalAddress") >= 0)
					fprintf(stderr, "...invalidCallSignalAddress");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.invalidRASAddress") >= 0)
					fprintf(stderr, "...invalidRASAddress");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.transportNotSupported") >= 0)
					fprintf(stderr, "...transportNotSupported");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.invalidRevision") >= 0)
					fprintf(stderr, "...invalidRevision");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.discoveryRequired") >= 0)
					fprintf(stderr, "...discoveryRequired");
				if (pvtGetNodeIdByPath(hVal, handle, "rejectReason.undefinedReason") >= 0)
					fprintf(stderr, "...undefinedReason");
			}
		}
    	fprintf(stderr, "\n");
	}

	return 0;
}

int CALLCONV
cmEvNewCall(	IN HAPP hApp,
		IN HCALL hsCall,
		IN LPHAPPCALL lphaCall)
{
	UINT32 ipOrig;
	INT32 tlen;
	cmTransportAddress addr;
	cmRASAlias number;
	char ipStr[20];
	char destination[80];
	int rc;
	int freeCall;

        printf("Entered cmEvNewCall\n");
	cb_counters.gen.EvNewCall_counter++;

	//
	// get call info 
	//

	tlen = sizeof(addr);

	rc = cmCallGetParam( hsCall, cmParamSrcCallSignalAddress, 0, &tlen, (char *) &addr );
	if (rc >= 0)
	{
		ipOrig = addr.ip;
		printf("Source Signal Address = %s:%d\n", liIpToString(addr.ip, ipStr),
			   addr.port);
	}
	else
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvNewCall(): ");
        fprintf(stderr,
                "cmCallGetParam() failed for cmParamSrcCallSignalAddress\n");

        ipOrig = addr.ip;

        fprintf(stdout, "Source Signal Address = %s:%d\n",
            liIpToString(addr.ip, ipStr), addr.port);
	}

	number.string = destination;
	number.length = 80;
	tlen = sizeof(number);

	if (cmCallSetParam(hsCall,
			 cmParamIsMultiplexed,
			 0,
			 1,
			 NULL) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvNewCall(): ");
        fprintf(stderr, "cmCallSetParam() failed for multiplex calls\n");
	}

	if (cmCallSetParam(hsCall,
			 cmParamShutdownEmptyConnection,
			 0,
			 shutEmptyConnection,
			 NULL) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvNewCall(): ");
        fprintf(stderr,
                "cmCallSetParam() failed for cmParamShutdownEmptyConnection\n");
	}

	rc = cmCallGetParam(hsCall, cmParamDestinationAddress, 0, &tlen, (char *) &number);

	if (rc >= 0)
	{
		printf("Destination Address: ");
		switch (number.type)
		{
		case cmAliasTypeE164:
			printf("E164 = ");
			break;
		case cmAliasTypeH323ID:
			printf("H323 ID = ");
			break;
		case cmAliasTypeEndpointID:
			printf("Endpoint ID = ");
			break;
		case cmAliasTypeGatekeeperID:
			printf("Gatekeeper ID = ");
			break;
		case cmAliasTypeURLID:
			printf("URL ID = ");
			break;
		case cmAliasTypeEMailID:
			printf("Email ID = ");
			break;
		case cmAliasTypeTransportAddress:
			printf("Transport Address = ");
			break;
		case cmAliasTypePartyNumber:
			printf("Party Number = ");
			break;
		}
		printf("%s\n", number.string);
	}							/* if  */
	else
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvNewCall(): ");
        fprintf(stderr, 
                "cmCallGetParam() failed for cmParamDestinationAddress\n");

		ipOrig = addr.ip;

        fprintf(stdout, "Source Signal Address = %s:%d\n",
                liIpToString(addr.ip, ipStr), addr.port);
	}

	freeCall = getFreeCall();

	if (freeCall < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvNewCall(): ");
        fprintf(stderr,
                "getFreeCall() failed for call # %d - no more calls\n",
                cb_counters.gen.EvNewCall_counter);

        fprintf(    trace_desc,
					"cmEvCallNewCall() : "
						"getFreeCall() failed - call#%d : no more calls\n",
					cb_counters.gen.EvNewCall_counter);
		fflush( trace_desc );

		if ( cmCallDrop(hsCall) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvNewCall(): ");
            fprintf(stderr,
                    "cmCallDrop() failed - getFreeCall() returned -1\n");
		}

		return -1;
	}

	CurrentCalls++;
	TotalCalls++;

	Calls[freeCall].hsCall = hsCall;

	Calls[freeCall].nulltcs = 0;

	*lphaCall = (HAPPCALL) & Calls[freeCall];

	return 0;
}

//*===
//* Callback routines for Call Events
//*===

int CALLCONV
cmEvCallStateChanged(	IN HAPPCALL haCall,
					  	IN HCALL hsCall,
						IN UINT32 state,
						IN UINT32 stateMode)
{
	int32_t	retval;
	BOOL 	origin;
	Call*	call = (Call *) haCall;
	char	callIDStr[32] = {0};
	char 	callID[32];
	int len;
	time_t now;
	char timestr[120];
	hrtime_t hrtime = nx_gethrtime();


    if (call == NULL)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallStateChanged(): ");
        fprintf(stderr, "NULL call handle\n");
		if (state == cmCallStateIdle)
		{
			printf(" -- cmEvCallStateChanged: cmCallStateIdle -- \n");

			HandleIdleCallState(hsCall, call);
		}
		return 0;
	}

	time(&now);

	call->state = (int) state;
	call->stateMode = (int) stateMode;

	cb_counters.call.EvCallStateChanged_counter++;

	switch( call->stateMode )
	{
	case cmCallStateModeDisconnectedBusy:
		cb_counters.call_state_mode.DisconnectedBusy++;
		break;
	case cmCallStateModeDisconnectedNormal:
		cb_counters.call_state_mode.DisconnectedNormal++;
		break;
	case cmCallStateModeDisconnectedReject:
		cb_counters.call_state_mode.DisconnectedReject++;
		break;
	case cmCallStateModeDisconnectedUnreachable:
		cb_counters.call_state_mode.DisconnectedUnreachable++;
		break;
	case cmCallStateModeDisconnectedUnknown:
		cb_counters.call_state_mode.DisconnectedUnknown++;
		break;
	case cmCallStateModeDisconnectedLocal:
		cb_counters.call_state_mode.DisconnectedLocal++;
		break;
	case cmCallStateModeConnectedControl:
		cb_counters.call_state_mode.ConnectedControl++;
		break;
	case cmCallStateModeConnectedCallSetup:
		cb_counters.call_state_mode.ConnectedCallSetup++;
		break;
	case cmCallStateModeConnectedCall:
		cb_counters.call_state_mode.ConnectedCall++;
		break;
	case cmCallStateModeConnectedConference:
		cb_counters.call_state_mode.ConnectedConference++;
		break;
	case cmCallStateModeOfferingCreate:
		cb_counters.call_state_mode.OfferingCreate++;
		break;
	case cmCallStateModeOfferingInvite:
		cb_counters.call_state_mode.OfferingInvite++;
		break;
	case cmCallStateModeOfferingJoin:
		cb_counters.call_state_mode.OfferingJoin++;
		break;
	case cmCallStateModeOfferingCapabilityNegotiation:
		cb_counters.call_state_mode.OfferingCapabilityNegotiation++;
		break;
	case cmCallStateModeOfferingCallIndependentSupplementaryService:
		cb_counters.call_state_mode.OfferingCallIndependentSupplementaryService++;
		break;
	case cmCallStateModeDisconnectedIncompleteAddress:
		cb_counters.call_state_mode.DisconnectedIncompleteAddress++;
		break;
	default:
		cb_counters.call_state_mode.Unknown++;
		break;
	}

	if ( (retval = cmCallGetOrigin( hsCall, NULL) ) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallStateChanged(): ");
        fprintf(stderr, "cmCallGetOrigin() failed for call # %d\n",
                call->cNum);
	}

	origin = (BOOL) retval;

	switch (state)
	{
	case cmCallStateDialtone:
		printf(" -- cmEvCallStateChanged: cmCallStateDialtone -- \n");

		time( &call->tsSetup );

		if (errorScenario == Scene_eDiscAfterARQ)
		{
			// Drop this call
			cmCallDrop(hsCall);
			return 0;
		}

		TotalCallsOut++;

		cb_counters.call_state.StateDialtone++;

		// Fast Start the call

		if (fastStart)
		{
			cmFastStartMessage fsMessage;
			int i;

			memset( &fsMessage, (int32_t) 0, sizeof(cmFastStartMessage) );

			//
		 	// One audio channels 
		 	//

			fsMessage.partnerChannelsNum = 1;

			//
		 	// Just one possiblity for an audio channel 
		 	//

			fsMessage.partnerChannels[0].type = cmCapAudio;

			//
		 	// Transmit 
		 	//

			i = 0;

			while (i < ncodecs)
			{
				cmFastStartChannel *fschannel = 
					&fsMessage.partnerChannels[0].transmit.channels[i];

				fschannel->rtp.ip = 0;
				fschannel->rtp.port = 0;
				fschannel->rtcp.ip = htonl(call->chIn[0].ip);
				//fschannel->rtcp.port = call->chIn[0].port + 1 +nCalls+i;
				fschannel->rtcp.port = call->chIn[0].port + 1;

				fschannel->dataTypeHandle = -1;
				fschannel->channelName = codecs[i];

				i++;
			}

			fsMessage.partnerChannels[0].transmit.altChannelNumber = ncodecs;

			//
		 	// Receive 
		 	//

			i = 0;

			while (i < ncodecs)
			{
				cmFastStartChannel *fschannel = 
					&fsMessage.partnerChannels[0].receive.channels[i];

				fschannel->rtp.ip = htonl(call->chIn[0].ip);
				//fschannel->rtp.port = call->chIn[0].port +nCalls +i;
				fschannel->rtp.port = call->chIn[0].port;

				fschannel->rtcp.ip = htonl(call->chIn[0].ip);
				//fschannel->rtcp.port = call->chIn[0].port + 1 +nCalls+i;
				fschannel->rtcp.port = call->chIn[0].port + 1;

				fschannel->dataTypeHandle = -1;
				fschannel->channelName = codecs[i];

				i++;
			}

			fsMessage.partnerChannels[0].receive.altChannelNumber = ncodecs;

			if (cmFastStartOpenChannels(hsCall, &fsMessage) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallStateChanged(): ");
                fprintf(stderr,
                        "cmFastStartOpenChannels() failed for call # %d\n",
                        call->cNum );
			}
		}
		call->hrSetup = hrtime;
		break;

	case cmCallStateProceeding:
		printf(" -- cmEvCallStateChanged: cmCallStateProceeding -- \n");
		cb_counters.call_state.StateProceeding++;
		time( &call->tsProceeding );
		call->hrProceeding = hrtime;

		if (errorScenario == Scene_eDiscAfterProceeding)
		{
			cmCallDrop(hsCall);
			return 0;
		}

		break;

	case cmCallStateRingBack:
		printf(" -- cmEvCallStateChanged: cmCallStateRingBack -- \n");
		cb_counters.call_state.StateRingBack++;
		time( &call->tsRingBack );
		call->hrRingBack = hrtime;

		if (errorScenario == Scene_eDiscAfterAlerting)
		{
			cmCallDrop(hsCall);
			return 0;
		}

		break;

	case cmCallStateConnected:
		printf(" -- cmEvCallStateChanged: cmCallStateConnected -- \n");

		cb_counters.call_state.StateConnected++;

        if (	doh245 &&
            	(stateMode == cmCallStateModeCallSetupConnected))
        {
        	if (cmCallConnectControl(hsCall) < 0)
            {
            	PrintErrorTime();
                fprintf(stderr, "cmEvCallStateChanged(): ");
                fprintf(stderr, "cmCallConnectControl() failed\n");
            }
        }
		if (fax && call->fastStart && (stateMode == cmCallStateModeCallConnected))
		{
			cmReqModeEntry  entry = { "t38fax", -1 };
    		cmReqModeEntry  *desc[] = { &entry, 0 };
    		cmReqModeEntry  **modes[] = { desc, 0 };
			int modeId;

			// Send a mode status change request
			modeId = cmRequestModeBuild(hApp, modes);
			if (modeId < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmRequestModeBuild() failed\n");
			}
			if (cmCallRequestMode(hsCall, modeId) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmCallRequestMode() failed\n");
			}
		}
		time(&call->tsConnect);
		call->hrConnect = hrtime;

		break;

	case cmCallStateDisconnected:
		printf(" -- cmEvCallStateChanged: cmCallStateDisconnecetd -- \n");

		if (origin && (call->tsDrop == 0))
		{
			if (!automode)
			{
				nErrors++;
			}
			nRelComps++;
			if(startTransfer)
			{	
				HangupCalls();
			}

			if (!automode)
			{
				// cftime(timestr,"%Y,%m,%d %T",&now); replacing this with POSIX strftime
				strftime(timestr, sizeof(timestr), "%Y,%m,%d %T", localtime(&now));
                PrintErrorTime();
                fprintf(stderr, "cmEvCallStateChanged(): ");
                fprintf(stderr, "rc error(ts=%s): mode=%d, call#%d(%d),pn=%s, or %d su %d, pr %d al %d co %d\n", 
						timestr,
						stateMode, call->cNum, 
						call->ntimes,
						calledpna[call->cNum],
						call->tsOriginate?now - call->tsOriginate:-1, 
						call->tsSetup?now - call->tsSetup:-1, 
						call->tsProceeding?now - call->tsProceeding:-1, 
						call->tsRingBack?now - call->tsRingBack:-1,
						call->tsConnect?now - call->tsConnect:-1);
			}

			fsync(3);
		}

		cb_counters.call_state.StateDisconnected++;
		break;

	case cmCallStateIdle:
		printf(" -- cmEvCallStateChanged: cmCallStateIdle -- \n");
		HandleIdleCallState(hsCall, call);
		break;

	case cmCallStateOffering:
		printf(" -- cmEvCallStateChanged: cmCallStateOffering -- \n");
		cb_counters.call_state.StateOffering++;

		if (asrMode)
                {

                        // Calculate the random probability for dropping the INVITE.
                        // If probability <= iprobability, process the INVITE,
                        // else drop it.
                        if (drand48() > iprobability)
                        {
                                isdnCode = 17;
                                dropISDN = 1;
                                nFailedSetups++;
                        }
                }


		if(dropISDN)
		{
			if (cmCallSetParam(hsCall, cmParamReleaseCompleteCause, 0,
				isdnCode, NULL) < 0)
			{
        		PrintErrorTime();
            	fprintf(stderr, "cmEvCallStateChanged(): ");
            	fprintf(stderr, 
						"Failed to set ISDN drop code %d for call # %d\n",
						call->cNum);
			}
			cmCallDrop(hsCall);
			if (asrMode)
                        {
                                dropISDN = 0;
                                //isdnCode = 16;
                        }

			return 0;
		}
		if (errorScenario == Scene_eNoAutoAnswer)
		{
			cmCallAccept(hsCall);
			//sleep(2);
		}
		else
		{

		if ((errorScenario == Scene_eDiscAfterARQ) ||
			(errorScenario == Scene_eDiscAfterSetup) ||
			(errorScenario == Scene_eDiscAfterProceeding) ||
			(errorScenario == Scene_eDiscAfterAlerting))
		{
			cmCallDrop(hsCall);
			return 0;
		}

		if (cmCallAnswer(hsCall) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvCallStateChanged(): ");
            fprintf(stderr, "cmCallAnswer() failed for call # %d\n",
                        call->cNum);
		}
		}
		break;

	case cmCallStateTransfering:
		printf(" -- cmEvCallStateChanged: cmCallStateTransfering -- \n");
		cb_counters.call_state.StateTransfering++;
		break;

	case cmCallStateIncompleteAddress:
		cb_counters.call_state.StateIncompleteAddress++;
		break;

	case cmCallStateWaitAddressAck:
		cb_counters.call_state.StateWaitAddressAck++;
		break;

	default:
        PrintErrorTime();
        fprintf(stderr, "cmEvCallStateChanged(): ");
        fprintf(stderr, "Unknown call state %d for call # %d\n",
                state, call->cNum);
		cb_counters.call_state.Unknown++;
		break;
	}
	return 0;
}

int CALLCONV
cmEvCallNewRate(	IN HAPPCALL haCall,
					IN HCALL hsCall,
					IN UINT32 rate)
{
	cb_counters.call.EvCallNewRate_counter++;

	printf(" -- cmEvCallNewRate -- \n");
	printf("New Call Rate = %u\n", rate);
	return 0;
}

int CALLCONV
cmEvCallInfo(	IN HAPPCALL haCall,
			 	IN HCALL hsCall,
				IN char *display,
				IN char *userUser,
				IN int userUserSize)
{
	cb_counters.call.EvCallInfo_counter++;

	printf(" -- cmEvCallInfo -- \n");
	return 0;
}

int CALLCONV
cmCallNonStandardParam(	IN HAPPCALL haCall,
					   	IN HCALL hsCall,
						IN char *data,
						IN int dataSize)
{
	cb_counters.call.CallNonStandardParam_counter++;

	printf(" -- cmEvCallNonStandardParam -- \n");
	return 0;
}

int CALLCONV
cmEvCallFacility(	IN HAPPCALL haCall,
				 	IN HCALL hsCall,
					IN int handle,
					OUT IN BOOL * proceed)
{
	cb_counters.call.EvCallFacility_counter++;

	*proceed = TRUE;

	printf(" -- cmEvCallFacility -- \n");

	return 0;
}

int
uh323ProcessIncomingFastStartAudio(	IN HAPPCALL haCall,
								   	IN HCALL hsCall,
								   	cmFastStartMessage * fsMessage,
									int i,
									int *tx,
									int *rx)
{
	Call*	call = (Call *) haCall;
	int j, nt, nr;
	int jt = -1, jr = -1;				/* Final accepted indices */
	int tmpCodec, codecType;

	cmFastStartChannel *txC, *rxC;
	cmTransportAddress rtp, rtcp;

	//
	// If haCall structure given to us by radvision
	// is null fail the call rather than core dumping.
	//

	if ( (Call*) call == (Call*) NULL )
		return( -1 );

	rtp.ip = htonl(call->chIn[0].ip);

	rtp.port = call->chIn[0].port;

	rtcp.ip = htonl(call->chIn[0].ip);
	rtcp.port = call->chIn[0].port + 1;

	nt = fsMessage->partnerChannels[i].transmit.altChannelNumber;
	nr = fsMessage->partnerChannels[i].receive.altChannelNumber;
	txC = &fsMessage->partnerChannels[i].transmit.channels[0];
	rxC = &fsMessage->partnerChannels[i].receive.channels[0];

	for (j = 0; j < nt; j++)
	{
		if (txC[j].channelName)
		{
			printf("Outgoing channel for TX %s\n", txC[j].channelName);
		}
		else
		{
            PrintErrorTime();
            fprintf(stderr, "uh323ProcessIncomingFastStartAudio(): ");
            fprintf(stderr, "No channel name for TX channel %d\n", j);
			break;
		}

		if (	!strcmp(txC[j].channelName, "g711Ulaw64k") )
		{
			jt = j;

			break;
		}
	}

	for (j = 0; j < nr; j++)
	{
		if (rxC[j].channelName)
		{
			printf("Incoming channel for RX %s\n", rxC[j].channelName);
		}
		else
		{
            PrintErrorTime();
            fprintf(stderr, "uh323ProcessIncomingFastStartAudio(): ");
            fprintf(stderr, "No channel name for RX channel %d\n", j);
			break;
		}

		if ( !strcmp(rxC[j].channelName, "g711Ulaw64k" ))
		{
			jr = j;

			break;
		}
	}

	if ((jt >= 0) && (jr >= 0))
	{
		if (cmFastStartChannelsAckIndex(hsCall, txC[jt].index, &rtcp, &rtp) >= 0)
		{
			*tx = 0;

			printf("Accepted TX channel %s.\n", txC[jt].channelName);
		}
		else
		{
            PrintErrorTime();
            fprintf(stderr, "uh323ProcessIncomingFastStartAudio(): ");
            fprintf(stderr, "TX Ack failed for %s\n", txC[jt].channelName);
		}

		if (cmFastStartChannelsAckIndex(hsCall, rxC[jr].index, &rtcp, &rtp) >= 0)
		{
			*rx = 0;

			printf("Accepted RX channel %s.\n", rxC[jr].channelName);
		}
		else
		{
            PrintErrorTime();
            fprintf(stderr, "uh323ProcessIncomingFastStartAudio(): ");
            fprintf(stderr, "RX Ack failed for %s\n", rxC[jr].channelName);
		}

		if (*tx || *rx)
		{
			//
		 	// Some error happened 
		 	//

			return -1;
		}
	}
	else
	{
        PrintErrorTime();
        fprintf(stderr, "uh323ProcessIncomingFastStartAudio(): ");
        fprintf(stderr, "Common fast channel absent\n");

		return -1;
	}

	return 1;
}

int CALLCONV
cmEvCallFastStartSetup(	IN HAPPCALL haCall,
						IN HCALL hsCall,
						OUT IN cmFastStartMessage * fsMessage)
{
	Call *call = (Call *) haCall;
	int i,
	    rc = -1,
	    tx = 1,
	    rx = 1;

	cb_counters.call.EvCallFastStartSetup_counter++;

	printf(" -- cmEvCallFastStartSetup -- \n");

	if (!fastStart)
	{
		// ignore fast start
		return(0);
	}

	// Fast start call has arrived
	// See if there is a g711 channel in fast start

	for (i = 0; i < fsMessage->partnerChannelsNum; i++)
	{
		switch (fsMessage->partnerChannels[i].type)
		{
		case cmCapAudio:
			rc = uh323ProcessIncomingFastStartAudio(	haCall,
														hsCall,
														fsMessage,
														i,
														&tx,
														&rx);
			break;
		default:
			break;
		}
	}

	if (rc < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallFastStartSetup(): ");
        fprintf(stderr, "uh323ProcessIncomingFastStartAudio() failed\n");
		// this can happen with a failed incoming call,
		// which is rejected due to no space.
	}
	else
	{
		if (cmFastStartChannelsReady(hsCall) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvCallFastStartSetup(): ");
            fprintf(stderr,
                    "cmFastStartChannelsReady() failed for call # %d\n",
                    call->cNum );
		}
		else
		{
			call->fastStart = 1;
		}
	}

	return 0;
}

//*===
//* Callback routines for control Events
//*===

int CALLCONV
cmEvCallCapabilities(	IN HAPPCALL haCall,
						IN HCALL hsCall,
						IN cmCapStruct * capabilities[])
{
	int i;
	Call *call = (Call *) haCall;

	cb_counters.control.EvCallCapabilities_counter++;

	printf(" -- cmEvCallCapabilities -- \n");
	printf("Capabilities reported by remote:\n");

	if (capabilities[0] == NULL)
	{
		fprintf(stdout, "Received NULL TerminalCapabilitySet for call %d\n",
				call->cNum);
		fprintf(stdout, ">\n");
		call->nulltcs = 1;	
	}
	else
	{
		if (call->nulltcs)
		{
			fprintf(stdout, "Received TerminalCapabilitySet for call %d\n",
					call->cNum);
			fprintf(stdout, ">\n");
			call->nulltcs = 0;	
		}
	}

	for (i = 0; capabilities[i]; i++)
	{
		printf("Capability Name: %s\n", capabilities[i]->name);
	}

	return 0;
}

int CALLCONV
cmEvCallCapabilitiesExt(	IN HAPPCALL haCall,
							IN HCALL hsCall,
							IN cmCapStruct *** capabilities[])
{
	int i,
	    j,
	    k;

	cb_counters.control.EvCallCapabilitiesExt_counter++;

	printf(" -- cmEvCallCapabilitiesExt -- \n");
	printf("Capability Descriptors: \n\n");

	for (i = 0; capabilities[i]; i++)
	{
		printf("Simultaneous capabilities:\n");

		for (j = 0; capabilities[i][j]; j++)
		{
			printf("  Alternative Capabilities:\n");

			for (k = 0; capabilities[i][j][k]; k++)
			{
				printf("    Capability Name: %s\n", capabilities[i][j][k]->name);
			}

			printf("\n");
		}
		printf("---------------------------------------\n");
	}
	return TRUE;
}

int CALLCONV
cmEvCallNewChannel(	IN HAPPCALL haCall,
					IN HCALL hsCall,
					IN HCHAN hsChannel,
					OUT LPHAPPCHAN lphaChannel)
{
	BOOL origin;
	Call *call = (Call *) haCall;
	int sNum;
	HCALL hsCall2;
	Call *call2;

	cb_counters.control.EvCallNewChannel_counter++;

	printf(" -- cmEvCallNewChannel -- \n");

	if (hsCall != call->hsCall)
	{
		lphaChannel = 0;
		return -1;
	}

	// Find out who originated the channel

	if ( cmChannelGetOrigin(	hsChannel,	&origin	) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallNewChannel(): ");
        fprintf(stderr, "cmChannelGetOrigin() failed for call # %d\n",
                call->cNum );
	}

	if (origin)					// This station originated the channel
	{
		sNum = getFreeOutChan(haCall, hsChannel);
		if (sNum < 0)
		{   
            PrintErrorTime();
            fprintf(stderr, "cmEvCallNewChannel(): ");
            fprintf(stderr,
                    "getFreeOutChan() failed for call # %d in state %d "
                    "with oldchannel %x - no more channels\n",
                    call->cNum, call->state, call->chOut[0].hsChan);

			fprintf(	trace_desc,
						"cmEvCallNewChannel() : "
							"getFreeOutChan() failed - call#%d : no more channels\n",
						call->cNum );

			fflush( trace_desc );

			if ( cmChannelDrop(hsChannel) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallNewChannel(): ");
                fprintf(stderr, "cmChannelDrop() failed for channel out "
                        "for call # %d\n", call->cNum);
			}

			return -1;
		}

		call->chOut[sNum].hsChan = hsChannel;
		*lphaChannel = (HAPPCHAN) & (call->chOut[sNum]);

		#ifdef USE_RTP
				call->chOut[sNum].port = rtpTestOpen(	call->cNum * 10 + sNum,
														"testChannel",
														startPort);
		#endif

		nChannelsOut++;
	}
	else
	{							// Channel was originated elsewhere

		sNum = getFreeInChan(haCall, hsChannel);

		if (sNum < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvCallNewChannel(): ");
            fprintf(stderr,
                    "getFreeInChan() failed for call # %d in state %d - "
                    "no more channels\n",
                    call->cNum, call->state);

			fprintf(	trace_desc,
						"cmEvCallNewChannel() : "
							"getFreeInChan() failed - call#%d : no more channels\n",
						call->cNum );

			fflush( trace_desc );

			if ( cmChannelDrop(hsChannel) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallNewChannel(): ");
                fprintf(stderr, "cmChannelDrop() failed for channel in "
                        "for call # d\n", call->cNum);
			}
			return -1;
		}

		call->chIn[sNum].hsChan = hsChannel;
		*lphaChannel = (HAPPCHAN) & (call->chIn[sNum]);

		#ifdef USE_RTP
				call->chIn[sNum].port =
					rtpTestOpen(call->cNum * 10 + sNum, "testChannel", startPort);
		#endif

		nChannelsIn++;
	}

	return 0;
}

int CALLCONV
cmEvCallCapabilitiesResponse(	IN HAPPCALL haCall,
								IN HCALL hsCall,
								IN UINT32 status)
{
	cb_counters.control.EvCallCapabilitiesResponse_counter++;

	printf(" -- cmEvCallCapabilitiesResponse -- \n");
	if (status == cmCapAccept)
		printf("Capabilities Accepted By Remote Station\n");
	else if (status == cmCapReject)
		printf("Capabilities Rejected By Remote Station\n");
	else
    {
        PrintErrorTime();
        fprintf(stderr, "cmEvCallCapabilitiesResponse(): ");
        fprintf(stderr, "Unknown Status %d in Capabilities Response\n",
                status);
    }
	return 0;
}

int CALLCONV
cmEvCallMasterSlaveStatus(	IN HAPPCALL haCall,
							IN HCALL hsCall,
							IN UINT32 status)
{
	BOOL origin;
	cb_counters.control.EvCallMasterSlaveStatus_counter++;

	printf(" -- cmEvCallMasterSlaveStatus -- \n");
	if (status == cmMSMaster)
		printf("Remote Station Is Master\n");
	else if (status == cmMSSlave)
		printf("Remote Station Is Slave\n");
	else if (status == cmMSError)
    {
        PrintErrorTime();
        fprintf(stderr, "cmEvCallMasterSlaveStatus(): ");
        fprintf(stderr, "Error in Master/Slave Determination\n");
    }
	else
    {
        PrintErrorTime();
        fprintf(stderr, "cmEvCallMasterSlaveStatus(): ");
        fprintf(stderr, "Unknown Status %d in Master/Slave Determination\n",
                status);
    }
	return 0;
}

int CALLCONV
cmEvCallRoundTripDelay(	IN HAPPCALL haCall,
						IN HCALL hsCall,
						IN INT32 delay)
{
	cb_counters.control.EvCallRoundTripDelay_counter++;

	printf(" -- cmEvCallRoundTripDelay -- \n");
	return 0;
}

int CALLCONV
cmEvCallUserInput(	IN HAPPCALL haCall,
					IN HCALL hsCall,
					IN INT32 userInputId)
{
	int nodeId = -1;
	cmUserInputIndication userInputIndication; 
	cmUserInputSignalStruct userInputSignalStruct;
    cmNonStandardIdentifier nsId;
    char udata[256];
    cmUserInputData userData = {udata, sizeof(udata)};
    char data[256];
    INT32 len = sizeof(data);
	Call *call = (Call *) haCall;

	cb_counters.control.EvCallUserInput_counter++;

	nodeId = cmUserInputGetDetail(hApp, userInputId, &userInputIndication);

	if (nodeId < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallUserInput(): ");
        fprintf(stderr, "User input failure\n");
		return 0;
	}

	switch (userInputIndication)
	{
	case cmUserInputSignal:
		if (cmUserInputGetSignal(hApp, nodeId, &userInputSignalStruct) < 0)
		{
        	fprintf(stdout, "Received signal DTMF for call %d\n", call->cNum);
        	fprintf(stdout, ">\n");
			break;
		}
        fprintf(stdout, "Received signal DTMF for call %d : '%c', %dms\n", 
				call->cNum, userInputSignalStruct.signalType, 
				userInputSignalStruct.duration);
       	fprintf(stdout, ">\n");
		break;
	case cmUserInputSignalUpdate:
		if (cmUserInputGetSignalUpdate(hApp, nodeId, &userInputSignalStruct) < 0)
		{
        	fprintf(stdout, "Received signal DTMF update for call %d\n", 
					call->cNum);
       		fprintf(stdout, ">\n");
			break;
		}
        fprintf(stdout, "Received signal DTMF update for call %d : %dms\n", 
				call->cNum, userInputSignalStruct.duration);
       	fprintf(stdout, ">\n");
		break;
	case cmUserInputAlphanumeric:
        if (cmUserInputGet(hApp, userInputId, &nsId, data, &len, &userData) < 0)
        {   
       		fprintf(stdout, "Received alphanumeric DTMF for call %d\n",
					call->cNum);
       		fprintf(stdout, ">\n");
			break;
		}
   		fprintf(stdout, "Received alphanumeric DTMF for call %d : '%c'\n", 
				call->cNum, userData.data[0]);
       	fprintf(stdout, ">\n");
		break;
	default:
        PrintErrorTime();
        fprintf(stderr, "cmEvCallUserInput(): ");
        fprintf(stderr, "Unknown user input received for call %d\n", 
				call->cNum);
		break;
	}

	printf(" -- cmEvCallUserInput -- \n");
	return 0;
}

int CALLCONV
cmEvCallRequestMode(	IN HAPPCALL haCall,
			IN HCALL hsCall,
			IN cmReqModeStatus status,
			IN INT32 nodeId)
{
	Call *call = (Call *) haCall;
	cmReqModeEntry      entry;
    cmReqModeEntry      *ep[1];
    cmReqModeEntry      **epp = NULL;
    void                *ptrs[10];
    cmReqModeEntry      ***modes = NULL;
    int                 modeId;

	cb_counters.control.EvCallRequestMode_counter++;

	printf(" -- cmEvCallRequestMode -- \n");

	if (status == cmReqModeRequest)
	{
		ep[0] = &entry;
        epp = ep;
        if (cmRequestModeStructBuild(hApp,
                    nodeId, epp, 1, ptrs, 10, &modes) != TRUE)
        {
                PrintErrorTime();
                fprintf(stderr, "cmEvCallRequestMode(): ");
                fprintf(stderr, "cmRequestModeStructBuild() failed\n");
                return 0;
        }

        if (modes == NULL)
        {
                PrintErrorTime();
                fprintf(stderr, "cmEvCallRequestMode(): ");
                fprintf(stderr, "Modes not found\n");
                return 0;
        }

		if ((strcmp(modes[0][0]->name, "t38fax") == 0))
		{
			// Send an ACK
			cmCallRequestModeAck(hsCall, "willTransmitMostPreferredMode");
			//fax = 1;
	
			if (call->controlState != cmControlStateConnected)
			{
				call->dofax = 1;
				return 0;
			}

			// Close our logical channel and open a fax one
			cmChannelDrop(call->chOut[0].hsChan);

			// We have a pending mode request on it
			call->chOut[0].pendingMode = 1;

		}
		if ((strcmp(modes[0][0]->name, "g711Ulaw64k") == 0))
		{
			//fprintf(stdout, "In revert voice mode.");
		}
	}
	else if (status == cmReqModeAccept)
	{
		if (call->controlState != cmControlStateConnected)
		{
			call->dofax = 1;
			return 0;
		}

		// Close our logical channel and open a fax one
		cmChannelDrop(call->chOut[0].hsChan);

		call->chOut[0].pendingMode = 1;

	}

	return 0;
}

int CALLCONV
cmEvCallMiscStatus(	IN HAPPCALL haCall,
					IN HCALL hsCall,
					IN cmMiscStatus status)
{
	cb_counters.control.EvCallMiscStatus_counter++;

	printf(" -- cmEvCallMiscStatus -- \n");
	return 0;
}

int CALLCONV
cmEvCallControlStateChanged(	IN HAPPCALL haCall,
							 	IN HCALL hsCall,
							 	IN cmControlState state,
								IN cmControlStateMode stateMode)
{
	Call *call = (Call *) haCall;
	BOOL origin;
	time_t now;

	cb_counters.control.EvCallControlStateChanged_counter++;

	call->controlState = state;
	call->controlStateMode = stateMode;

	if (cmCallGetOrigin( hsCall, &origin) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvCallControlStateChanged(): ");
        fprintf(stderr, "cmCallGetOrigin() failed for call # %d\n",
                call->cNum);
	}

	time(&now);

	printf(" -- cmEvCallControlStateChanged -- ");
	switch (state)
	{
	case cmControlStateTransportConnected:
		printf(" -- cmEvCallControlStateChanged: cmControlStateTransportConnected -- \n");
		cb_counters.control_state.StateTransportConnected++;
		break;

	case cmControlStateTransportDisconnected:
		printf( " -- cmEvCallControlStateChanged: cmControlStateTransportDisconnected "
				"-- \n");
		cb_counters.control_state.StateTransportDisconnected++;
		if ( reportAllDisconnects )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvCallControlStateChanged(): ");
            fprintf(stderr, "cs error: mode=%d, call#%d, su %d, pr %d al %d co %d\n", stateMode, call->cNum, 
				call->tsOriginate?now - call->tsOriginate:-1, 
				call->tsSetup?now - call->tsSetup:-1, 
				call->tsProceeding?now - call->tsProceeding:-1, 
				call->tsRingBack?now - call->tsRingBack:-1,
				call->tsConnect?now - call->tsConnect:-1);
			fsync(3);
		}
        sendFaxOLC = 0;

		break;

	case cmControlStateConnected:

		printf(" -- cmEvCallControlStateChanged: cmControlStateConnected -- \n");

		cb_counters.control_state.StateConnected++;

		if (fastStart != call->fastStart)
		{
			nErrors++;
			nFailedFaststarts++;
			cmFailedFastStarts++;
		}

		// We can open OLCs now
		if (!call->fastStart)
		{
			int sNum;

			sNum = getFreeOutChan(haCall, NULL);

			if (sNum < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "getFreeOutChan() failed for call # %d - "
                        "no more channels\n", call->cNum );

				fprintf(	trace_desc,
							"cmEvCallControlStateChanged() : "
								"getFreeOutChan() failed - call#%d : no more channels\n",
							call->cNum );

				fflush( trace_desc );

				return -1;
			}

			if (cmChannelNew(	hsCall,
								(HAPPCHAN) & (call->chOut[sNum]),
								&call->chOut[sNum].hsChan) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmChannelNew() failed for call # %d\n",
                        call->cNum );
			}

			if (cmChannelSetRTCPAddress(	call->chOut[sNum].hsChan,
				htonl(call->chIn[sNum].ip),
				call->chIn[sNum].port + 1) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, 
                        "cmChannelSetRTCPAddress() failed for call # %d\n",
                        call->cNum );
			}

			if (cmChannelOpen(	call->chOut[sNum].hsChan,
								"g711Ulaw64k",
								NULL,
								NULL,
								0) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmChannelOpen() failed for call # %d\n",
                        call->cNum );
			}

			nChannelsOut++;
		}
		break;

	case cmControlStateConference:
		printf(" -- cmEvCallControlStateChanged: cmControlStateConference -- \n");
		cb_counters.control_state.StateConference++;
		break;

	case cmControlStateFastStart:
		printf(" -- cmEvCallControlStateChanged: cmControlStateFastSetup -- \n");
		cb_counters.control_state.StateFastStart++;

		call->fastStart = 1;

		if (doh245 && 0)
		{
			// user wants to do h.245 also
			if (cmCallConnectControl(hsCall) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr,
                        "cmCallConnectControl() failed for call # %d\n",
                        call->cNum);
			}
		}
		break;

	default:
        PrintErrorTime();
        fprintf(stderr, "cmEvCallControlStateChanged(): ");        
        fprintf(stderr, "Unknown call state %d for call # %d\n",
                state, call->cNum);
		cb_counters.control_state.Unknown++;
		break;
	}

	return 0;
}

int CALLCONV
cmEvCallMasterSlave(	IN HAPPCALL haCall,
						IN HCALL hsCall,
						IN UINT32 terminalType,
						IN UINT32 statusDeterminationNumber)
{
	printf(" -- cmEvCallMasterSlave -- \n");

	cb_counters.control.EvCallMasterSlave_counter++;

	printf("Remote Side Terminal Type = %d, StatusDeterminationNumber = %d\n",
		   terminalType, statusDeterminationNumber);
	return 0;
}

//*===
//* Callback routines for channel Events
//*===

int CALLCONV
cmEvChannelStateChanged(	IN HAPPCHAN haChannel,
							IN HCHAN hsChannel, 
							IN cmChannelState_e state, 
							IN cmChannelStateMode_e stateMode)
{
	Channel *channel = (Channel *) haChannel;
	Call *call;
	BOOL origin;
	int rtpport;

	cb_counters.channel.EvChannelStateChanged_counter++;

	if (channel == NULL)
	{
		if (state == cmChannelStateIdle)
		{
			HandleIdleChannelState(hsChannel, channel);
		}

		return 0;
	}

	call = (Call *) channel->haCall;
	if ( cmChannelGetOrigin(hsChannel, &origin) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelStateChanged(): ");
        fprintf(stderr, "cmChannelGetOrigin() failed for call # %d\n",
                call->cNum );
	}

	channel->state = (int) state;
	channel->stateMode = (int) stateMode;

	switch( channel->stateMode )
	{
	case cmChannelStateModeOn:
		cb_counters.channel_state_mode.On++;
		break;
	case cmChannelStateModeOff:
		cb_counters.channel_state_mode.Off++;
		break;
	case cmChannelStateModeDisconnectedLocal:
		cb_counters.channel_state_mode.DisconnectedLocal++;
		break;
	case cmChannelStateModeDisconnectedRemote:
		cb_counters.channel_state_mode.DisconnectedRemote++;
		break;
	case cmChannelStateModeDisconnectedMasterSlaveConflict:
		cb_counters.channel_state_mode.DisconnectedMasterSlaveConflict++;
		break;
	case cmChannelStateModeDuplex:
		cb_counters.channel_state_mode.Duplex++;
		break;
	case cmChannelStateModeDisconnectedReasonUnknown:
		cb_counters.channel_state_mode.DisconnectedReasonUnknown++;
		break;
	case cmChannelStateModeDisconnectedReasonReopen:
		cb_counters.channel_state_mode.DisconnectedReasonReopen++;
		break;
	case cmChannelStateModeDisconnectedReasonReservationFailure:
		cb_counters.channel_state_mode.DisconnectedReasonReservationFailure++;
		break;
	default:
		cb_counters.channel_state_mode.Unknown++;
		break;
	}

	switch (state)
	{
	case cmChannelStateDialtone:
		printf(" -- cmEvChannelStateChanged: cmChannelStateDialtone -- \n");
		cb_counters.channel_state.StateDialtone++;
		break;

	case cmChannelStateRingBack:
		printf(" -- cmEvChannelStateChanged: cmChannelStateRingBack -- \n");
		cb_counters.channel_state.StateRingBack++;
		break;

	case cmChannelStateConnected:
		printf(" -- cmEvChannelStateChanged: cmChannelStateConnected -- \n");
		cb_counters.channel_state.StateConnected++;
		if (origin)
		{
			if(mode & MODE_TRANSMIT)
			{
				CallsOut[call->cNum].chOut[0].ip = call->chOut[0].ip;
			}
			else
			{
				Calls[call->cNum].chOut[0].ip = call->chOut[0].ip;
			}
			if(!startTransfer)
				mgenInform(call, 1);
		}
        sendFaxOLC++;
		if (fax && (sendFaxOLC == 2) && !call->fastStart && (channel->stateMode == cmChannelStateModeOn))
		{
			cmReqModeEntry  entry = { "t38fax", -1 };
    		cmReqModeEntry  *desc[] = { &entry, 0 };
    		cmReqModeEntry  **modes[] = { desc, 0 };
			int modeId;

			// Send a mode status change request
			modeId = cmRequestModeBuild(hApp, modes);
			if (modeId < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmRequestModeBuild() failed\n");
			}
			if (cmCallRequestMode(call->hsCall, modeId) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvCallControlStateChanged(): ");
                fprintf(stderr, "cmCallRequestMode() failed\n");
			}
		}
		break;

	case cmChannelStateDisconnected:
		printf(" -- cmEvChannelStateChanged: cmChannelStateDisconnected -- \n");
		cb_counters.channel_state.StateDisconnected++;
		break;

	case cmChannelStateIdle:
		printf(" -- cmEvChannelStateChanged: cmChannelStateIdle -- \n");
		HandleIdleChannelState(hsChannel, channel);
		if (channel->pendingMode)
		{
			channel->pendingMode = 0;

			// Open a fax channel
			if (cmChannelNew(call->hsCall, (HAPPCHAN) &(call->chOut[0]),
				&call->chOut[0].hsChan) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");        
                fprintf(stderr, "cmChannelNew() failed\n");
			}

			if (cmChannelSetRTCPAddress(call->chOut[0].hsChan,
				htonl(call->chIn[0].ip),
				call->chIn[0].port + 1) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr, "cmChannelSetRTCPAddress() failed\n");
			}

			if (cmChannelOpen(call->chOut[0].hsChan, "t38fax", NULL, NULL, 144) < 0)
            {
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr, "cmChannelOpen() failed\n");
			}
			nChannelsOut++;
		}

		break;

	case cmChannelStateOffering:

		printf(" -- cmEvChannelStateChanged: cmChannelStateOffering -- \n");
		cb_counters.channel_state.StateOffering++;

/*
		if (!doh245)
		{
			// this should be regarded as an error
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelStateChanged(): ");
            fprintf(stderr, "Incoming H.245 channel\n");
		}
*/

		if (stateMode != cmChannelStateModeDuplex)
		{
			if ( cmChannelSetAddress(	hsChannel,
										0,
										(UINT16) channel->port) < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr,
                        "cmChannelSetAddress() failed for call # %d\n",
                        call->cNum );
			}
		}
		else
		{
			cmTransportAddress ta;
			ta.length = 0;
			ta.ip = 0;
			ta.port = (UINT16) channel->port;
			ta.type = cmTransportTypeIP;

			if ( cmChannelSetDuplexAddress(	hsChannel,
											ta,
											1,
											"2",
											FALSE) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr, 
                        "cmChannelSetDuplexAddress() failed for call # %d\n",
                         call->cNum );
			}

		}

		if ( cmChannelSetFlowControlToZero(	hsChannel, TRUE	) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelStateChanged(): ");
            fprintf(stderr, 
                    "cmChannelSetFlowControlToZero() failed for call # %d\n",
                    call->cNum);
		}                     
		if ( cmChannelAnswer(hsChannel) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelStateChanged(): ");
            fprintf(stderr, "cmChannelAnswer() failed for call # %d\n",
                    call->cNum);
		}

		if (	!call->fastStart &&
				!call->chOut[channel->chNum].hsChan &&
				!cmChannelIsDuplex(call->chIn[channel->chNum].hsChan))
		{
			#ifdef USE_RTP

				// Open Channel in reverse direction

				rtpport = rtpTestOpen(	call->cNum * 10 + channel->chNum,
										"testChannel",
										startPort);

			#endif

			if ( cmChannelNew(	call->hsCall,
								(HAPPCHAN) &(call->chOut[channel->chNum]),
						 		&(call->chOut[channel->chNum].hsChan)) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr, "cmChannelNew() failed for call # %d\n",
                        call->cNum );
			}

			if ( cmChannelSetRTCPAddress(	call->chOut[channel->chNum].hsChan,
											0,
											(UINT16) (call->chIn[0].port + 1)) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr,
                        "cmChannelSetRTCPAddress() failed for call # %d\n",
                        call->cNum );
			}

			strcpy(	call->chOut[channel->chNum].channelName,
				   	call->chIn[channel->chNum].channelName);

			if (call->chIn[channel->chNum].dynaPayloadType)
			{
				if ( cmChannelSetDynamicRTPPayloadType(
												call->chOut[channel->chNum].hsChan,
												call->chIn[channel->chNum].
												dynaPayloadType) < 0 )
				{
                    PrintErrorTime();
                    fprintf(stderr, "cmEvChannelStateChanged(): ");
                    fprintf(stderr, 
                            "cmChannelSetDynamicRTPPayloadType() failed "
                            "for call # %d\n", call->cNum );
				}
			}

			if ( cmChannelOpenDynamic(	call->chOut[channel->chNum].hsChan,
								 		pvtParent(	cmGetValTree(hApp),
													call->chIn[channel->chNum].handle),
										NULL,
								 		NULL,
										FALSE) < 0 )
			{
                PrintErrorTime();
                fprintf(stderr, "cmEvChannelStateChanged(): ");
                fprintf(stderr, 
                        "cmChannelOpenDynamic() failed for call # %d\n",
                        call->cNum );
			}

			nChannelsOut++;
		}

		break;
	default:
		cb_counters.channel_state.Unknown++;
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelStateChanged(): ");
        fprintf(stderr, "Unknown call state %d for call # %d\n",
                state, call->cNum );
		break;
	}

	return 0;
}

int CALLCONV
cmEvChannelNewRate(	IN HAPPCHAN haChan,
					IN HCHAN hsChan,
					IN UINT32 rate)
{
	Channel *channel = (Channel *) haChan;
	Call *call = (Call *) channel->haCall;
	BOOL origin;

	cb_counters.channel.EvChannelNewRate_counter++;

	printf(" -- cmEvChannelNewRate -- \n");
	printf("New Channel Rate = %ud\n", rate);

	if ( cmChannelGetOrigin(hsChan, &origin) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelNewRate(): ");
        fprintf(stderr, 
                "cmChannelGetOrigin() failed for call # %d\n",
                call->cNum );
	}

	channel->rate = (int) rate;

	if (origin && call->chIn[channel->chNum].hsChan)
	{
		if ( cmChannelFlowControl(call->chIn[channel->chNum].hsChan, rate) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelNewRate(): ");
            fprintf(stderr, 
                    "cmChannelFlowControl() failed for call # %d, rate %d\n",
                     call->cNum, rate);
		}
	}
	return 0;
}

int CALLCONV
cmEvChannelMaxSkew(	IN HAPPCHAN haChan1,
					IN HCHAN hsChan1,
					IN HAPPCHAN haChan2,
					IN HCHAN hsChan2,
					IN UINT32 skew)
{
	cb_counters.channel.EvChannelMaxSkew_counter++;

	printf(" -- cmEvChannelMaxSkew -- \n");
	return 0;
}

int CALLCONV
cmEvChannelSetAddress(	IN HAPPCHAN haChan,
						IN HCHAN hsChan,
						IN UINT32 ip,
						IN UINT16 port)
{
	Channel *channel = (Channel *) haChan;
	BOOL origin;

	if ( cmChannelGetOrigin(	hsChan,	&origin	) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelSetAddress(): ");
        fprintf(stderr, "cmChannelGetOrigin() failed\n");
	}

	cb_counters.channel.EvChannelSetAddress_counter++;

	printf(" -- cmEvChannelSetAddress -- \n");

	#ifdef USE_RTP
		rtpTestSetRtp(	((Call *) (channel->haCall))->cNum * 10 + channel->chNum,
						ip, port);
	#endif

	if (channel && origin)
	{
		channel->ip = ntohl(ip);
		channel->port = port;
	}

	return 0;
}

int CALLCONV
cmEvChannelSetRTCPAddress(	IN HAPPCHAN haChan,
							IN HCHAN hsChan,
							IN UINT32 ip,
							IN UINT16 port)
{
	Channel *channel = (Channel *) haChan;
	BOOL origin;

	if ( cmChannelGetOrigin(	hsChan,	&origin	) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelSetRTCPAddress(): ");
        fprintf(stderr, "cmChannelGetOrigin() failed\n");
	}


	cb_counters.channel.EvChannelSetAddress_counter++;

	printf(" -- cmEvChannelSetRTCPAddress -- \n");

	#ifdef USE_RTP
		rtpTestSetRtcp( ((Call *) (channel->haCall))->cNum * 10 + channel->chNum,
						ip, port);
	#endif

	if (channel && origin)
	{
		channel->ip = ntohl(ip);
		channel->port = port-1;
	}

	return 0;
}

int CALLCONV
cmEvChannelParameters(	IN HAPPCHAN haChan,
						IN HCHAN hsChan,
						IN char *channelName,
						IN HAPPCHAN haChanSameSession,
						IN HCHAN hsChanSameSession,
						IN HAPPCHAN haChanAssociated,
						IN HCHAN hsChanAssociated,
						IN UINT32 rate)
{
	Channel *channel = (Channel *) haChan;

	cb_counters.channel.EvChannelParameters_counter++;

	printf(" -- cmEvChannelParameters -- \n");

	if (channel && channelName)
		strcpy( channel->channelName, channelName );

	return 0;
}

int CALLCONV
cmEvChannelRTPDynamicPayloadType(	IN HAPPCHAN haChan,
									IN HCHAN hsChan,
									IN INT8 dynamicPayloadType)
{
	Channel *channel = (Channel *) haChan;

	cb_counters.channel.EvChannelRTPDynamicPayloadType_counter++;

	printf(" -- cmEvChannelRTPDynamicPayloadType -- \n");
	channel->dynaPayloadType = dynamicPayloadType;
	return 0;
}

int CALLCONV
cmEvChannelVideoFastUpdatePicture(	IN HAPPCHAN haChan,
									IN HCHAN hsChan)
{
	Channel *channel = (Channel *) haChan;
	BOOL origin;

	cb_counters.channel.EvChannelVideoFastUpdatePicture_counter++;

	printf(" -- cmEvChannelVideoFastUpdatePicture -- \n");
	if ( cmChannelGetOrigin(hsChan, &origin) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "cmEvChannelVideoFastUpdatePicture(): ");
        fprintf(stderr, "cmChannelGetOrigin failed\n");
	}

	if (origin && ((Call *) channel->haCall)->chIn[channel->chNum].hsChan)
		cmChannelVideoFastUpdatePicture(	
							((Call *) channel->haCall)->chIn[channel->chNum].hsChan);
	return 0;
}

int CALLCONV
cmEvChannelVideoFastUpdateGOB(	IN HAPPCHAN haChan,
								IN HCHAN hsChan,
								IN int firstGOB,
								IN int numberOfGOBs)
{
	Channel *channel = (Channel *) haChan;
	BOOL origin = FALSE;

	cb_counters.channel.EvChannelVideoFastUpdateGOB_counter++;

	printf(" -- cmEvChannelVideoFastUpdateGOB -- \n");

	if (origin && ((Call *) channel->haCall)->chIn[channel->chNum].hsChan)
		cmChannelVideoFastUpdateGOB(((Call *) channel->haCall)->chIn[channel->chNum].
									hsChan, firstGOB, numberOfGOBs);
	return 0;
}

int CALLCONV
cmEvChannelVideoFastUpdateMB(	IN HAPPCHAN haChan,
								IN HCHAN hsChan,
								IN int firstGOB,
								IN int firstMB,
								IN int numberOfMBs)
{
	Channel *channel = (Channel *) haChan;
	BOOL origin = FALSE;

	cb_counters.channel.EvChannelVideoFastUpdateMB_counter++;

	printf(" -- cmEvChannelVideoFastUpdateMB -- \n");
	if (origin && ((Call *) channel->haCall)->chIn[channel->chNum].hsChan)
		cmChannelVideoFastUpdateMB(((Call *) channel->haCall)->chIn[channel->chNum].
								   hsChan, firstGOB, firstMB, numberOfMBs);
	return 0;
}

int CALLCONV
cmEvChannelHandle(	IN HAPPCHAN haChan,
					IN HCHAN hsChan,
					IN int dataTypeHandle,
					IN cmCapDataType dataType)
{
	HPVT valH = cmGetValTree(hApp);
	Channel *channel = (Channel *) haChan;

	cb_counters.channel.EvChannelHandle_counter++;

	printf(" -- cmEvChannelHandle -- \n");
	if (channel)
	{
		channel->handle = dataTypeHandle;
	}

	return TRUE;
}

int CALLCONV
cmEvChannelGetRTCPAddress(	IN HAPPCHAN haChan,
							IN HCHAN hsChan,
							IN UINT32 * ip,
							IN UINT16 * port)
{
	cb_counters.channel.EvChannelGetRTCPAddress_counter++;

	printf(" -- cmEvChannelGetRTCPAddress -- \n");
	return 0;
}

int CALLCONV
cmEvChannelRequestCloseStatus(	IN HAPPCHAN haChan,
								IN HCHAN hsChan,
								IN cmRequestCloseStatus status)
{
	cb_counters.channel.EvChannelRequestCloseStatus_counter++;

	printf(" -- cmEvChannelRequestCloseStatus -- \n");
	if (status == cmRequestCloseRequest)
	{
		if ( cmChannelAnswer(hsChan) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelRequestCloseStatus(): ");
            fprintf(stderr, "cmChannelAnswer failed\n");
		}
		if ( cmChannelDrop(hsChan) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelRequestCloseStatus(): ");
            fprintf(stderr, "cmChannelDrop() failed\n");
		}
        sendFaxOLC = 0;
	}
	return 0;
}

int CALLCONV
cmEvChannelTSTO(	IN HAPPCHAN haChan,
					IN HCHAN hsChan,
					IN INT8 isCommand,
					IN INT8 tradeoffValue)
{
	cb_counters.channel.EvChannelTSTO_counter++;

	printf(" -- cmEvChannelTSTO -- \n");
	return 0;
}

int CALLCONV
cmEvChannelMediaLoopStatus(	IN HAPPCHAN haChan,
							IN HCHAN hsChan,
							IN cmMediaLoopStatus status)
{
	Channel *channel = (Channel *) haChan;

	cb_counters.channel.EvChannelMediaLoopStatus_counter++;

	printf(" -- cmEvChannelMediaLoopStatus -- \n");
	switch (status)
	{
	case cmMediaLoopRequest:

		if ( cmChannelMediaLoopConfirm(hsChan) < 0 )
		{
            PrintErrorTime();
            fprintf(stderr, "cmEvChannelMediaLoopStatus(): ");
            fprintf(stderr, "cmChannelMediaLoopConfirm() failed\n");
		}

		channel->MediaLoop = TRUE;
		break;

	case cmMediaLoopOff:
		channel->MediaLoop = FALSE;
		break;
	default:
		break;
	}
	return 0;
}

int CALLCONV
cmEvChannelReplace(	IN HAPPCHAN haChan,
				   	IN HCHAN hsChan,
					IN HAPPCHAN haReplacedChannel,
					IN HCHAN hsReplacedChannel)
{
	cb_counters.channel.EvChannelReplace_counter++;

	printf(" -- cmEvChannelReplace -- \n");
	return TRUE;
}

int CALLCONV
cmEvChannelFlowControlToZero(IN HAPPCHAN haChan, IN HCHAN hsChan)
{
	cb_counters.channel.EvChannelFlowControlToZero_counter++;

	printf(" -- cmEvChannelFlowControlToZero -- \n");
	return TRUE;
}

//*===
//* Callback routines for Protocol Events
//*===

//Before listen

BOOL CALLCONV
cmHookListen(	IN HPROTCONN hConn,
				IN int addr)
{
	cb_counters.protocol.HookListen_counter++;

	printf(" -- cmHookListen -- \n");
	return 0;
}

//After listen

int CALLCONV
cmHookListening(	IN HPROTCONN hConn,
					IN int addr,
					IN BOOL error)
{
	cb_counters.protocol.HookListening_counter++;

	printf(" -- cmHookListening -- \n");
	return 0;
}

//Before connect

int CALLCONV
cmHookConnecting(	IN HPROTCONN hConn,
					IN int addr)
{
	cb_counters.protocol.HookConnecting_counter++;

	printf(" -- cmHookConnecting -- \n");

	return 0;
}

//Incoming connect

int CALLCONV
cmHookInConn(	IN HPROTCONN hConn,
				IN int addrFrom,
				IN int addrTo)
{
	cb_counters.protocol.HookInConn_counter++;

	printf(" -- cmHookConn -- \n");
	return 0;
}

//Outgoing connect

int CALLCONV
cmHookOutConn(	IN HPROTCONN hConn,
				IN int addrFrom,
				IN int addrTo,
				IN BOOL error)
{
	cb_counters.protocol.HookOutConn_counter++;

	printf(" -- cmHookOutConn -- \n");
	return 0;
}

BOOL CALLCONV
cmHookSend(IN HPROTCONN hConn, IN int nodeId, IN BOOL error)
{
	char				msgname[80];
	int 				paramNodeId, facnodeId;
	HPVT	hVal;
	INT32 				ip, port;
	//char *gtd = "GTDTEST";
	//int   gtdLen = 7;
	int offset = 0;

	cb_counters.protocol.HookSend_counter++;

	strcpy(msgname,cmGetProtocolMessageName(hApp, nodeId));
	printf(" -- cmHookSend -- %s \n", msgname);
	//fprintf(stdout,"Entering cmHookSend [%s]\n",msgname);

	hVal		=  cmGetValTree(hApp);

	//
	offset = getGTDMsgOffset(msgname);
	//fprintf(stdout,"Offset for [%s] is [%d]\n",msgname,offset);
	//fprintf(stdout,"Sending [%s] message\n",msgname);

	if(SendGtd(hApp,
	        nodeId, 
		gtd_list[offset].gtd,
		gtd_list[offset].gtd_len) < 0)
		//strlen(gtd_list[offset].gtd)) < 0)
	{
	      fprintf(stdout,"Error is attaching GTD [%s]\n",msgname);
	}
	//

	if (!strcmp(msgname, "connect"))
	{
		// remove h.245 address from setup if its there
		if ((paramNodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.connect.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.connect.h245Address")) > 0)
		{
			// remove this node id
			printf("removed h.245 address from connect\n");
			pvtDelete(hVal, paramNodeId);
		}
	}
  	else if (!strcmp(msgname, "setup"))
    {
		if (strlen(otg))
		{
	        if (pvtBuildByPath(hVal, nodeId,
   	        	"message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.sourceCircuitID.group.group", strlen(otg), otg) < 0)
        	{
           		PrintErrorTime();
	            fprintf(stderr, "cmHookSend(): ");
   	         	fprintf(stderr, "pvtBuildByPath() failed to build "
                    	"originating trunk group\n");
        	}
		}

		if (strlen(dtg))
		{
        	      if (pvtBuildByPath(hVal, nodeId,
            	                               "message.setup.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.setup.circuitInfo.destinationCircuitID.group.group", 
					       strlen(dtg), dtg) < 0)
        	      {
            	          PrintErrorTime(); 
			  fprintf(stderr, "cmHookSend(): "); 
			  fprintf(stderr, "pvtBuildByPath() failed to build " 
			                             "destination trunk group\n");
        	      }
		}
    }

	if ((errorScenario == Scene_eBadH245Address) && 
			!strcmp(msgname,"facility"))
	{
		if (((facnodeId = pvtGetNodeIdByPath(hVal, nodeId, 
			"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) >= 0) &&
			((pvtGetNodeIdByPath(hVal, facnodeId, "reason.starth245") >= 0) || 
			(pvtGetNodeIdByPath(hVal, facnodeId, "reason.startH245") >= 0) ))
		{
			port = htons(12345);
			if (pvtBuildByPath(hVal, facnodeId,
				"h245Address.ipAddress.port", port, NULL) < 0)
			{ 
			       PrintErrorTime(); 
			       fprintf(stderr, "cmHookSend(): "); 
			       fprintf(stderr, 
			                     "pvtBuildByPath() failed to set H.245 port\n");
			}
		}
	}

	return 0;
}

BOOL CALLCONV
cmHookRecv(	IN HPROTCONN hConn,
			IN int nodeId,
			IN BOOL error)
{
	char				msgname[80];

	cb_counters.protocol.HookRecv_counter++;

	strcpy(msgname,cmGetProtocolMessageName(hApp, nodeId));
	printf(" -- cmHookRecv -- %s \n", msgname);

	return 0;
}

BOOL CALLCONV
cmHookSendTo(	IN HPROTCONN hConn,
				IN int nodeId,
				IN int addrTo,
				IN BOOL error)
{
	char msgname[256];
	char 				*msgptr;

	cb_counters.protocol.HookSendTo_counter++;

	printf(" -- cmHookSendTo -- \n");

	if(!(msgptr = cmGetProtocolMessageName(hApp, nodeId)))
	{
        PrintErrorTime();
        fprintf(stderr, "cmHookSend(): ");
        fprintf(stderr, "cmGetProtocolMessageName() failed\n");
		return 0;
	}

	nx_strlcpy(msgname,msgptr,256);

	if(!strcmp(msgname,"admissionRequest"))
	{
		if (pvtBuildByPath(cmGetValTree(hApp), nodeId,
			"admissionRequest.canMapAlias",
			1, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "cmHookSend(): ");
            fprintf(stderr,
                    "pvtBuildByPath() failed to build mapAlias\n");
		}

		if (pvtBuildByPath(cmGetValTree(hApp), nodeId,
			"admissionRequest.callModel",
			cmCallModelTypeDirect, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "cmHookSend(): ");
            fprintf(stderr, "pvtBuildByPath() failed to build callModel\n");
		}

		if (strlen(otg))
		{
	    	if (pvtBuildByPath(cmGetValTree(hApp), nodeId,
   	     		"admissionRequest.circuitInfo.sourceCircuitID.group.group",
        		strlen(otg), otg) < 0)
    		{
            	PrintErrorTime();
            	fprintf(stderr, "cmHookSend(): ");
            	fprintf(stderr, "pvtBuildByPath() failed to build "
                    "originating trunk group\n");
    		}
		}

		if (strlen(dtg))
		{
    		if (pvtBuildByPath(cmGetValTree(hApp), nodeId,
        		"admissionRequest.circuitInfo.destinationCircuitID.group.group",
        		strlen(dtg), dtg) < 0)
    		{
           		PrintErrorTime();
            	fprintf(stderr, "cmHookSend(): ");
            	fprintf(stderr, "pvtBuildByPath() failed to build "
                	"destination trunk group\n");
    		}
		}
	}

	return 0;
}

BOOL CALLCONV
cmHookRecvFrom(	IN HPROTCONN hConn,
				IN int nodeId,
				IN int addrFrom,
				IN BOOL multicast,
				IN BOOL error)
{
	cb_counters.protocol.HookRecvFrom_counter++;

	printf(" -- cmHookRecvFrom -- \n");
	return 0;
}

void CALLCONV
cmHookClose(IN HPROTCONN hConn)
{
	cb_counters.protocol.HookClose_counter++;

	printf(" -- cmHookClose -- \n");
	return;
}

//*===
//* Routines to place calls from callgenThread() thread
//*===

// spawn calls in the range [low..high]
int
SpawnOutgoingCalls(int low, int high)
{
	int i;

	// Create nCalls outgoing calls

	for (i = low; i < high; i++)
	{
		// If the totalMaxCalls limit is reached, disconnect calls and gracefully exit.

                if ((maxCallsMode == 1) && (callCounterTx >= totalMaxCalls))
                {
                    fprintf(stdout, "Total Max Calls reached.\n");
			// sigsend(P_PID, P_MYID, SIGINT); replacing with POSIX kill
					kill(getpid(), SIGINT);

                }

		SpawnOutgoingCall( i );
		callCounterTx ++;

		if ( shutdown_calls )
		{
			return( (high - low) );
			break;
		}
	}

	return( ( high - low ) );
}

// spawn the ith call
void
SpawnOutgoingCall(int i)
{
	HCALL	hsCall;
	char	srcAddress[256] = { 0 }, dstAddress[256] = {0};
	int		failed = 0;
	HPROTCONN protoHandle;
	hrtime_t hrtime1, hrtime2;

	// Create nCalls outgoing calls

	CallsOut[i].nulltcs = 0;

	if (CallsOut[i].tsOriginate != 0)
	{
        PrintErrorTime();
        fprintf(stderr, "SpawnOutgoingCall(): ");
        fprintf(stderr, "Spawning call # %d, tsUse is non-zero\n", i);
		return;
	}

	{
		if (cmCallNew( hApp, (HAPPCALL) &CallsOut[i], &hsCall) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SpawnOutgoingCall(): ");
            fprintf(stderr, "cmCallNew() failed for call # %d\n", i); 
			cmCallNewFailed_counter++;
			return;
		}
		else
			cmCallNew_counter++;

		ListInitElem(&CallsOut[i]);

		CallsOut[i].hsCall = hsCall;

		sprintf(dstAddress, "TA:%s:%d,TEL:%s",
				gwAddr, gwPort, xferMode ? xferTgtNum : calledpna[i]);

		if (strlen(h323Id))
		{
			sprintf(srcAddress, "TEL:%s,NAME:%s", callingpna[i], h323Id);
		}
		else
		{
			sprintf(srcAddress, "TEL:%s", callingpna[i]);
		}


        if (cmCallSetParam(hsCall,
                 cmParamIsMultiplexed,
                 0,
                 1,
                 NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SpawnOutgoingCall(): ");
            fprintf(stderr, "cmCallSetParam() failed for multiplex calls\n");
        }

        if (cmCallSetParam(hsCall,
                 cmParamShutdownEmptyConnection,
                 0,
                 shutEmptyConnection,
                 NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SpawnOutgoingCall(): ");
            fprintf(stderr, 
                    "cmCallSetParam() failed for Shutdown Empty\n");
        }

		hrtime1 = nx_gethrtime();
		if (cmCallMake( hsCall, 64000, 0, dstAddress, srcAddress,
					   displayStr, userUserStr, sizeof(userUserStr)) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SpawnOutgoingCall(): ");
            fprintf(stderr, "cmCallMake() failed for call # %d\n", i);

			fprintf( 	trace_desc,
						"cmCallMake failed for call TA:%s:%d,TEL:%s,%s\n",
					 	gwAddr,
						gwPort,
						callingpna[i],
						xferMode ? xferTgtNum: calledpna[i]);

			fflush( trace_desc );
			cmCallMakeFailed_counter++;
		}
		else
		{
			CallsOut[i].ntimes ++;
			time( &CallsOut[i].tsOriginate );

			CallsOut[i].hrOriginate = hrtimeLast = hrtime2 = nx_gethrtime();

			CallsOut[i].timeNextEvt = CallsOut[i].tsOriginate + tdConnect;
			
			// Insert this into the list
			if (tdCallDuration)
			{
				CallInsertIntoTimerList(&CallsOut[i]);
			}

			cmCallMake_counter++;
			CurrentCallsOut++;
		}
	}

	return;
}

int
TimeoutCalls(void)
{
	time_t now, start, delta;
	Call *centry, *call;
	int newcalls;

	if (timerHead == NULL)
	{
		return 0;
	}

	time(&now);
	start = now;
	newcalls = 0;

	while ((centry = timerHead) && (now >= centry->timeNextEvt))
	{
		time(&now);
		CallDeleteFromTimerList(centry);

		// either the call needs to be disconnected, or re-made

		call = centry;

		switch (call->state)
		{
		case cmCallStateDialtone:

			if (call->tsSetup == 0)
			{
                PrintErrorTime();
                fprintf(stderr, "TimeoutCalls(): ");
                fprintf(stderr, "Call # %d not stamped in setup state\n",
                        call->cNum);
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			{
				nErrors++;
				nFailedSetups++;
				cmFailedSetups++;

				time(&call->tsDrop);
				cmCallDrop( call->hsCall );

			}

			break;

		case cmCallStateProceeding:
			if (call->tsProceeding == 0)
			{
                PrintErrorTime();
                fprintf(stderr, "TimeoutCalls(): ");
                fprintf(stderr,
                        "Call # %d not stamped in proceeding state\n",
                        call->cNum);
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			{
				nErrors++;

				nFailedProceeding++;

				time(&call->tsDrop);
				cmCallDrop( call->hsCall );
			}

			break;

		case cmCallStateRingBack:

			if (call->tsRingBack == 0)
			{
                PrintErrorTime();
                fprintf(stderr, "TimeoutCalls(): ");
                fprintf(stderr,
                        "Call # %d not stamped in ringback state\n",
                        call->cNum);
				break;
			}

			if (call->tsDrop)
			{
				break;
			}

			{
				nErrors++;

				nFailedRingBack++;

				time(&call->tsDrop);
				cmCallDrop( call->hsCall );
			}

			break;

		case cmCallStateConnected:

			if (call->tsDrop)
			{
				break;
			}

			{
				time(&call->tsDrop);
				cmCallDrop(call->hsCall);
			}
			break;

		default:
			break;
		}
	}
}

int
CallDeleteFromTimerList(Call *call)
{
	if (tdCallDuration == 0)
	{
		return 0;
	}

	if ((timerHead == call) &&
		(timerHead == call->next))
	{
		// only element in the list
		timerHead = NULL;
	}
	else if (timerHead == call)
	{
		timerHead = call->next;
	}

	ListDelete(call);
	ListInitElem(call);

	return 0;
}

int
CallInsertIntoTimerList(Call *call)
{
	Call *centry;

	ListInitElem(call);

	// Based on Next Evt, insert
	if (timerHead == NULL)
	{
		timerHead = call;
		return 0;
	}
	
	centry = (Call *)timerHead;

	while (centry->next != (Call *)timerHead)
	{
		if (centry->timeNextEvt <= call->timeNextEvt)
		{
			centry = centry->next;
		}
		else
		{
			break;
		}
	}

	// Insert at the current entry
	ListInsert(centry, call);

	return 0;
}

//=========================================================================
//                  MAIN
//=========================================================================

#ifdef _WIN32

	void
	main()
	{
		DWORD ThreadId;

		HANDLE thread = (HANDLE) _beginthreadex(NULL, 0, WMain, NULL, 0, &ThreadId);
		if (!thread)
            perror("main() - error starting the thread ");
		while (1)
		{
			int c, len;

			c = getch();
			if (c == 'x')
			{
				SendMessage(hwnd, WM_USER, 0, 0L);
				break;
			}
			if (c == 'm')
			{
				printf("Enter Destination Address: ");
				fgets(Destination, 1024, stdin);
				len = strlen(input);
				if (input[len-1]=='\n')
				  input[len-1]='\0';
				SendMessage(hwnd, WM_USER, 1, 0L);
			}
		}

	}

#else /* Non-Windows system */

int
launchThread(void *(*fn) (void *arg), void *arg, pthread_t *thread_handle, char* name)
{
	pthread_attr_t thread_attr;

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
	pthread_attr_setscope( &thread_attr, PTHREAD_SCOPE_SYSTEM );
	pthread_attr_setguardsize( &thread_attr, (size_t) (2*getpagesize()) );
	pthread_attr_setstacksize( &thread_attr, 10*1024*1024 );

	if ( pthread_create( thread_handle, &thread_attr, fn, arg) != 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "launchThread(): ");
        fprintf(stderr, "Failed to launch %s() thread...exiting\n", name);
		abort();
	}

}

void *
stdinLoop(void * data)
{
	struct pollfd	filedes;
	int				ntimes = 0;
	int				nready;			// return value from poll 
									//	values equate to :
									//
									//		nready = 0		timeout
									//		nready > 0		# of ready sockets
									//		nready < 0		poll error condition

	//
	// Set up stdinLoop thread as a Realtime thread with a
	// 100 millisecond time quantum and Realtime priority of 50
	//

//   thread_set_rt( 100000000, 50 );

	memset( &filedes, (int32_t) 0, sizeof( struct pollfd ) );

	if (!idaemon)
	{
		filedes.fd = 0;
		filedes.events = ( POLLIN | POLLERR );
	}

	for (;;)
	{
		if ( exit_main_loop )
			break;

		nready = poll( &filedes, 1, 10000 );	

		switch (nready)
		{
		case -1:
			{
				int lerrno = errno;
				// Handle error event

				if ( errno == EINTR && exit_main_loop )
					goto stdin_loop_exit;

                PrintErrorTime();
                fprintf(stderr, "stdinLoop(): ");
                fprintf(stderr, "Poll failure - errno %d\n", lerrno);
			}
			break;

		case 0:
			// Handle timeout event

			if ( exit_main_loop )
				goto stdin_loop_exit;

			if (idaemon)
			{
				ntimes ++;
				fflush(stdout); 
				fflush(stderr);
				if (ntimes > 1000000)
				{
					ntimes = 0;
					system("/bin/echo > ./genout.log");
					system("/bin/echo > ./generr.log");
					PrintStartState();
				}
			}

			break;

		default:

			// Handle ready file descriptors
			ReadInput();

			if ( exit_main_loop )
				goto stdin_loop_exit;

			break;
		}
	}

stdin_loop_exit:

	if ( h323_fds_inuse )
	{
		// Take mainLoop() out of poll

		close( h323_pollfd_array[2].fd ); 
	}

	return;
}

void
mainLoop(void)
{
	static struct timeval	tout;
	uint32_t 				msec = (uint32_t) -1;
	time_t					monTime, now, delta;
	hrtime_t				hrtimeNow;

	int						status;
	int						rc = 0;
	int 					retval;
	int						nready;			// return value from poll 
											//	values equate to :
											//
											//		nready = 0	timeout
											//		nready > 0	# of ready sockets
											//		nready < 0	poll error

	//
	// Allocate and Initialize h323_pollfd_array
	//

	retval = 0;

	if ( (h323_pollfd_array = pollArrayInit( rvmaxfds )) == (struct pollfd *) NULL )
	{
        PrintErrorTime();
        fprintf(stderr, "mainLoop(): ");
        fprintf(stderr, "Failed to allocate poll array of size %d\n",
                rvmaxfds);

		cv_signal( &main_loop_cv, THRD_READY );
		goto main_loop_exit;
	}

	retval = 0;
	cv_signal( &main_loop_cv, THRD_READY );

	time(&monTime);

	for (;;)
	{
		if (relcomp)
		{
			// send relcomps
			HangupCalls();
			relcomp = 0;
		}
		if ( exit_main_loop )
		{
			break;
		}

		//
		// the pollA we pass to this function must
		// be maxfds long, as the api does not process it
		// otherwise.
		//

		h323_pollfd_array[0].fd = notifyPipe[NOTIFY_READ];
		h323_pollfd_array[0].revents = 0;
		h323_pollfd_array[0].events |=POLLIN;
		
		h323_fds_inuse = 0;
		nready = poll( h323_pollfd_array, h323_fds_inuse+1, 0);

		switch (nready)
		{
		case -1:
			{
				// Handle error event

				int lerrno = errno;

				if ( errno == EINTR )
				{
					if (relcomp)
					{
						// send relcomps
						HangupCalls();
						relcomp = 0;
					}
					if ( exit_main_loop )
					{
						goto main_loop_exit;
					}
				}

                PrintErrorTime();
                fprintf(stderr, "mainLoop(): ");
                fprintf(stderr, "Poll failure - errno %d\n", lerrno);
			}
			break;

		case 0:
			// Handle timeout event

			if (relcomp)
			{
				// send relcomps
				HangupCalls();
				relcomp = 0;
			}

			if ( exit_main_loop )
				goto main_loop_exit;

			TimeoutCalls();

			break;

		default:

			// Handle ready file descriptors

			if (relcomp)
			{
				// send relcomps
				HangupCalls();
				relcomp = 0;
			}
			if ( exit_main_loop )
				goto main_loop_exit;

			if ( nready > max_nready )
				max_nready = nready;

			hrtimeNow = nx_gethrtime();
			if ((nready > 0) &&
				h323_pollfd_array[0].revents & (POLLIN|POLLHUP) &&
				(hrtimeNow-hrtimeLast >= callTimePeriod))
			{
				// process the pipe event
				nready --;
				callgenProcessPipe();
			}

			break;
		}
		seliSelect();
	}

main_loop_exit:
    cmEnd(hApp);

	if (mode & MODE_IXIA)
	{
		int trash;
		kill( ixiaGlobal.pid, SIGINT );
		/* wait( ixiaGlobal.pid, &trash, 0 ); */
	}

	return;
}

void*
launchMainLoop(void* arg)
{
	int		error;
	char	ipStr[80];
	int		i, j;
	int		flags;
	unsigned long	**ipA;
	unsigned long	gwAddrNum;
	struct stat buf;
	char ixiaCommand[IXIA_COMMAND_LEN];

	if (mode & MODE_IXIA)
	{
		if( getenv( "IXIA_VERSION" ) == NULL )
		{
			fprintf( stderr, "Error - Ixia Environment variables not found.\n" );
			exit( -1 );
		}
		/* Check to see if the Test script file exists */
		if( stat( ixia_testScriptName, &buf ) != 0 )
		{
			fprintf( stderr, "Test script %s not found.\n", ixia_testScriptName );
			exit( -1 );
		}
		if (stat (IXIA_TCLSH, &buf) != 0)
		{
			fprintf (stderr, "stat failed for %s errno = %d\n", IXIA_TCLSH, 
						errno);
			exit (-1);
		}

		ixiaInitCallAccum();
		IXIA_OPEN( IXIA_TCLSH );

		sprintf( ixiaCommand, "source %s\n", ixia_testScriptName );
		IXIA_SEND_INSTR( ixiaCommand );
		IXIA_WAIT_DONE( );
	
	}

	pthread_sigmask( SIG_BLOCK, &threads_signal_mask, NULL );

	//
	// Set up mainLoop thread as a Realtime thread with a
	// 100 millisecond time quantum and Realtime priority of 50
	//

    // thread_set_rt( 100000000, 50 );

	// Set value of maxfds statically defined inside libcommon.a

	NetFdsSetMax( maxfds );

	// Set maximum # of file descriptors in radvision h323 stack

	seliSetMaxDescs( rvmaxfds );

	// Initialize Radvision Syntax Tree from asn.1 data in
	// config.val file

	SetupConfigs(config_template_file);

	if ( (error = cmInitialize( config_file, &hApp )) < 0 )
	{
        PrintErrorTime();
        fprintf(stderr, "launchMainLoop(): ");
        fprintf(stderr, "cmInitialize() failed with error %d...exiting\n",
                error);
		exit(1);
	}

	GetH323IDFromConfig();

	ipA = (unsigned long **) liGetHostAddrs();
	//localIP = (ipA && *ipA && **ipA) ? ntohl(**ipA) : 0;
	

	//
	// If radvision logging was specified for any modules
	// add them and log to memory.
	//

	if ( module_count )
	{
		//
		// Direct logging to trc_line() function
		// trc_line() outputs Radvision H.323 stack
		// logging to a circular in-memory buffer
		//

		//msSetStackNotify( trc_line );
		msSinkAdd("file genh323debug.log");

		for ( i = 0; i < module_count; i++ )
		{
			msAdd(modules[i]);
		}
	}
	msSetDebugLevel(rvdebug);

	hCfg = ciConstruct(config_file);

	if (hCfg == NULL)
	{
		// Yes, Give up the ghost
        PrintErrorTime();
        fprintf(stderr, "launchMainLoop(): ");
        fprintf(stderr, "No config file...exiting\n");
		exit(-1);
	}

	// Get maximum concurrent calls value - maxCalls

	ciGetValue( hCfg, "system.maxCalls" , NULL, &maxCalls);

	// Get maximum channels per call value - maxChannels

	ciGetValue( hCfg, "system.maxChannels" , NULL, &maxChannels);

	// Get maximum channels allowed for stack instance

	ciGetValue( hCfg, "system.allocations.channels" , NULL, &channels);

	// Is value set in config.val

	if ( !channels )
	{
		// No, assume default value was set by stack

		channels = (2 * (maxCalls * (7 + maxChannels))) + 10;
	}

	// Get maximum messages allowed for stack instance

	ciGetValue( hCfg, "system.allocations.messages" , NULL, &messages);

	// Is value set in config.val

	if ( !messages )
	{
		// No, assume default value was set by stack

		messages = (2 * (maxCalls * (7 + maxChannels))) + 10;
	}

	// Get maximum vtNodeCount allowed for stack instance

	ciGetValue( hCfg, "system.allocations.vtNodeCount" , NULL, &vtNodeCount);

	// Is value set in config.val

	if ( !vtNodeCount )
	{
		// No, assume default value was set by stack

		vtNodeCount = (maxCalls * (750 + maxChannels*100)) + 900;
	}

	// Get maximum PDLAPI protocols allowed for stack instance

	ciGetValue( hCfg, "system.allocations.protocols" , NULL, &protocols);

	// Is value set in config.val

	if ( !protocols )
	{
		// No, assume default value was set by stack

		protocols = (maxCalls * (7 + maxChannels)) + 5;
	}

	// Get maximum # of PDL state machines in allowed for stack instance

	ciGetValue( hCfg,"system.allocations.maxProcs" , NULL, &maxProcs);

	if ( !maxProcs )
	{
		// No, assume default value was set by stack

		maxProcs = (maxCalls * (7 + maxChannels)) + 5;
	}

	ciGetValue( hCfg,"system.allocations.tpktChans" , NULL, &tpktChans );

	if ( !tpktChans )
	{
		// No, assume default value was set by stack

		tpktChans = (maxCalls * 3) + 1;
	}

	fprintf( trace_desc, "\nRadvision H323 values used for system syntax tree :\n" );

	fprintf( trace_desc, "    maxCalls      : %10d\n", maxCalls );
	fprintf( trace_desc, "    maxChannels   : %10d\n", maxChannels );
	fprintf( trace_desc, "    channels      : %10d\n", channels );
	fprintf( trace_desc, "    messages      : %10d\n", messages );
	fprintf( trace_desc, "    vtNodeCount   : %10d\n", vtNodeCount );
	fprintf( trace_desc, "    protocols     : %10d\n", protocols );
	fprintf( trace_desc, "    maxProcs      : %10d\n", maxProcs );
	fprintf( trace_desc, "    tpktChans     : %10d\n\n", tpktChans );

	fflush( trace_desc );

	// Extract Local Call Signal Address from asn.1 data

	error = cmGetLocalCallSignalAddress( hApp, &trAddr );

	if (error >= 0)
    	{
        fprintf(stdout,"Local Call Signaling IP/Port = %s/%d\n",
				liIpToString(trAddr.ip, ipStr), trAddr.port);

	// If Local Signaling address specified, assign that to localIP
       // localIP = ntohl (trAddr.ip);

	}
	else
	{
        PrintErrorTime();
        fprintf(stderr, "launchMainLoop(): ");
        fprintf(stderr, "cmGetLocalCallSignalingAddress() failed\n" );
	}

	if (!chanLocalAddr)
	{
		chanLocalAddr = ntohl(trAddr.ip);
	}

	if (!gwAddrFlag)
	{
		gwAddrNum = trAddr.ip;		// Set GW address to local address
		if (cmGetGKRASAddress(hApp, &trAddr) >= 0)
		{
			gwAddrNum = trAddr.ip;		// Update GW address
		}
		nx_strlcpy(gwAddr, ULIPtostring(ntohl(gwAddrNum)), 16);
	}

	if (!gwPort)
	{
		gwPort = 1720;
	}

	// Initialize calling party number array
	callingpna = (char **) malloc( nCalls * sizeof(char *) );
	for (i = 0; i < nCalls; i++)
	{
		callingpna[i] = (char *) malloc(256);
		if (incCallingPn)
		{
			nx_strlcpy(callingpna[i], callingpn, 256);
			getNextPhone(callingpn);	// Increment the number
		}
		else if(readcallingpnfromfile )
		{
        		if((i < nCalls) && !(getNextPhoneFromFile(callingpn,srcfp)))
            		{
            		fprintf(stderr,"InitCalls(): Phone Numbers in file are less than total numbers of call\n");
                	fclose(srcfp);
                	exit(1);
            	}
            	nx_strlcpy(callingpna[i], callingpn, 256);

        	}
        	else
            		nx_strlcpy(callingpna[i], callingpn, 256);
	}

	// Initialize called party number array
	calledpna = (char **) malloc( nCalls * sizeof(char *) );
	for (i = 0; i < nCalls; i++)
	{
		calledpna[i] = (char *) malloc(256);
		if (incCalledPn)
		{
			nx_strlcpy(calledpna[i], calledpn, 256);
			getNextPhone(calledpn);		// Increment the number
		}
		else if(readcalledpnfromfile )
        	{
			if((i < nCalls) && !(getNextPhoneFromFile(calledpn,dstfp)))
            		{
				fprintf(stderr,"InitCalls(): Phone Numbers in file are less than total numbers of call\n");
                		fclose(dstfp);
                		exit(1);
            	}
            	nx_strlcpy(calledpna[i], calledpn, 256);

        }
        else
		nx_strlcpy(calledpna[i], calledpn, 256);
	}
	if(srcfp)
		fclose(srcfp);
	if(dstfp)
		fclose(dstfp);

	// Allocate heap space for calls specified - both outgoing and incoming

	Calls = (Call *) calloc( nCalls, sizeof(Call) );
	CallsOut = (Call *) calloc( nCalls, sizeof(Call) );

	for (i = 0; i < nCalls; i++)
	{
		Calls[i].cNum = i;
		Calls[i].state = cmCallStateIdle;

		Calls[i].chOut[0].state = cmChannelStateIdle;
		Calls[i].chOut[0].chNum = 0;
		Calls[i].chOut[0].haCall = (HAPPCALL) &Calls[i];
		Calls[i].chOut[0].port = chanLocalPort;
		Calls[i].chOut[0].ip = chanLocalAddr;

		Calls[i].chIn[0].state = cmChannelStateIdle;
		Calls[i].chIn[0].chNum = 0;
		Calls[i].chIn[0].haCall = (HAPPCALL) &Calls[i];
		Calls[i].chIn[0].port = chanLocalPort;
		Calls[i].chIn[0].ip = chanLocalAddr;

		CallsOut[i].cNum = i;
		CallsOut[i].state = cmCallStateIdle;

		CallsOut[i].chOut[0].state = cmChannelStateIdle;
		CallsOut[i].chOut[0].chNum = 0;
		CallsOut[i].chOut[0].haCall = (HAPPCALL) &CallsOut[i];

		CallsOut[i].chIn[0].state = cmChannelStateIdle;
		CallsOut[i].chIn[0].chNum = 0;
		CallsOut[i].chIn[0].haCall = (HAPPCALL) &CallsOut[i];
		CallsOut[i].chIn[0].port = chanLocalPort;
		CallsOut[i].chIn[0].ip = chanLocalAddr;

		CallsOut[i].tsSetup = 0;
		CallsOut[i].tsConnect = 0;
		CallsOut[i].tsIdle = 0;

		chanLocalPort += 2;
	}

    if (i > 0)
    {
        chanLocalPort -= 2; 
    }

	#ifdef  USE_RTP
		rtpTestInit();
	#endif 
        
    /* If we are in GK mode then Register the RAS request event handler */
    if (gkMode)
        cmRASSetEventHandler(hApp, &cmRASEvent, sizeof(cmRASEvent));

	//
	// Register General Event handler callback routines:
	//
	//		cmEvNewCall()
	//		cmEvRegEvent()
	//

	cmSetGenEventHandler( hApp, &cmEvent, sizeof(cmEvent) );

	//
	// Register Call Event handler callback routines:
	//
	//		cmEvCallStateChanged()
	//		cmEvCallNewRate()
	//		cmEvCallInfo()
	//		cmCallNonStandardParam()
	//		cmEvCallFacility()
	//		cmEvCallFastStartSetup()
	//

	cmSetCallEventHandler( hApp, &cmCallEvent, sizeof(cmCallEvent) );

	//
	// Register Control Event handler callback routines:
	//
	//	cmEvCallCapabilitiesExt()
	//	cmEvCallNewChannel()
	//	cmEvCallCapabilitiesResponse()
	//	cmEvCallMasterSlaveStatus()
	//	cmEvCallRoundTripDelay()
	//	cmEvCallUserInput()
	//	cmEvCallRequestMode()
	//	cmEvCallMiscStatus()
	//	cmEvCallControlStateChanged()
	//	cmEvCallMasterSlave()
	//

	cmSetControlEventHandler( hApp, &cmControlEvent, sizeof(cmControlEvent) );

	//
	// Register channel Event handler callback routines:
	//
	//	cmEvChannelStateChanged()
	//	cmEvChannelNewRate()
	//	cmEvChannelMaxSkew()
	//	cmEvChannelSetAddress()
	//	cmEvChannelSetRTCPAddress()
	//	cmEvChannelParameters()
	//	cmEvChannelRTPDynamicPayloadType()
	//	cmEvChannelVideoFastUpdatePicture()
	//	cmEvChannelVideoFastUpdateGOB()
	//	cmEvChannelVideoFastUpdateMB()
	//	cmEvChannelHandle()
	//	cmEvChannelGetRTCPAddress()
	//	cmEvChannelRequestCloseStatus()
	//	cmEvChannelTSTO()
	//	cmEvChannelMediaLoopStatus()
	//	cmEvChannelReplace()
	//	cmEvChannelFlowControlToZero()
	//

	cmSetChannelEventHandler( hApp, &cmChannelEvent, sizeof(cmChannelEvent) );

	cmSetProtocolEventHandler(hApp,&cmProtocolEvent,sizeof(cmProtocolEvent) );

	// State what has been specified for test
	PrintStartState();

	if (mode & MODE_MGCP)
	{
		// depending on how many calls we can handle,
		// we will have to wait for that many ports
		launchThread( mgenServerThread, NULL, &mgen_thread, "mgenThread" );
	}

	// open a pipe between the main loop and the call generator
	// thread, which both can use when they come up
	if (pipe(notifyPipe) < 0)
	{
        perror("launchMainLoop() - pipe error ");
	}
	else	
	{
		if((flags = fcntl(notifyPipe[NOTIFY_READ],F_GETFL,0)) <0)
		{
            perror("launchMainLoop() - fcntl notify read/get error ");
		}
	}

	if (!(mode & MODE_MGCP))
	{
		launchThread( callgenThread, NULL, &callgen_thread, "callgenThread" );
	}

	launchThread( 	stdinLoop, NULL, &stdinLoop_thread, "stdinLoop" );
	h323Initialized = 1;

	mainLoop();

	cv_signal( &exit_cgen_cv, THRD_DONE );

	return NULL;
}

int
callgenProcessPipe()
{
	char buff[8];
	int low, high;

	if (read(notifyPipe[NOTIFY_READ], buff, 8) < 8)
	{
        perror("callgenProcessPipe() - read error ");
	}

	memcpy(&low, buff, 4);
	memcpy(&high, buff+4, 4);
	
	call_count += SpawnOutgoingCalls(low, high);

	return 1;
}

void *
callgenThread( void *arg )
{
	int				status;
	int				i, j;
	time_t			start, now, delta;
	Call			*call;
	int 			newcalls = 0, callsGenerated = 0;
    time_t 			startTime;         
    char 			startTimeStr[32];      
	int				tmp_nCalls = 0;

	tmp_nCalls = nCalls;

	// Are we generating calls on this end

	if (mode & MODE_TRANSMIT)
	{
		int	low = 0;
		int	high;
		char buff[8];

		// Set up as RT SCHED_RR queue thread with quantum
		// of 50 ms.

        // thread_set_rt( 50000000, 30 );

		pthread_sigmask( SIG_BLOCK, &threads_signal_mask, NULL );

		cv_wait( &main_loop_cv, THRD_READY );
		millisleep( 1000 );

    	time(&startTime);
    	//cftime(startTimeStr, "%T on %Y/%m/%d", &startTime); replacing this with POSIX strftime
    	strftime(startTimeStr, sizeof(startTimeStr), "%T on %Y/%m/%d", localtime(&startTime));
    	
		fprintf(stdout, "Starting calls at %s\n", startTimeStr);
       	fprintf(stdout, ">\n");

		// Yes, Then do it to it!! Inundate the receiver!!

		//
		//  	If burst is not set in input arguments spawnOutgoingCalls()
		// 	burst is set to nCalls in test. SpawnOutgoingCalls() will
		// 	be called 1 time with low set to 0 and high set to nCalls.
		//		Otherwise, each burst of size specified will sleep for
		//	burstInterval milliseconds.
		//

		time(&start);

		/* spawn one call less than specified on the commandline if running call transfer option*/
		if(setModeTransfer)
		{
			tmp_nCalls--;
		}
		while (low < tmp_nCalls)
		{
			time( &CallsOut[low].tsUse );
			memcpy(buff, &low, 4);
			low++;
			memcpy(buff+4, &low, 4);
			if (write(notifyPipe[NOTIFY_WRITE], buff, 8) < 8)
			{
                perror("callgenThread() - write error 1 ");
			}

			// see how much time its currently taking...
			millisleep( callTimePeriod/1000000);
		}

		time(&now);
		delta = now-start;

		if (tdCallDuration > delta)
		{
            if (!automode)
            {
                fprintf(stdout, "Sleeping for %ds\n", tdCallDuration-delta+5);
            }
			sleep(tdCallDuration-delta+5);
		}
		else if (tdCallDuration == 0)
		{
			goto _end;
		}

		callsGenerated = nCalls;
_start:
		if (xCalls && (callsGenerated >= xCalls))
		{
			goto _finish;
		}

		newcalls = 0;
		for (i=0; i<nCalls; i++)
		{
			time(&now);

			call = &CallsOut[i];
			
			if (call->state != cmCallStateIdle)
			{
				continue;
			}

			if (call->tsUse)
			{
				continue;
			}

			if (call->timeNextEvt > now)
			{
				continue;
			}
	
			ResetCall( call );

			low = call->cNum;
			time( &CallsOut[low].tsUse );
			memcpy(buff, &low, 4);
			low++;
			memcpy(buff+4, &low, 4);
			if (write(notifyPipe[NOTIFY_WRITE], buff, 8) < 8)
			{
                perror("callgenThread() - write error 2 ");
			}
			newcalls++;
			callsGenerated++;

			millisleep( callTimePeriod/1000000);
		}

		if (newcalls == 0)
		{
            if (rvdebug)
            {
                fprintf(stdout, "no new calls to generate...\n");
            }                
			millisleep(300);
		}
		goto _start;

_finish:
        fprintf(stdout, "Total calls placed %d\n", call_count);

        fprintf(trace_desc, "==== Status after last call made\n" );

		fprintf(trace_desc, "total calls placed as of now %d\n", call_count );

		fprintf(trace_desc, "channels       inuse now %10d,  total used %10d, max %10d\n",
				cmSizeCurChannels(hApp), cmSizeMaxChannels(hApp), channels );

		fprintf(trace_desc, "protocols      inuse now %10d,  total used %10d, max %10d\n",
				cmSizeCurProtocols(hApp), cmSizeMaxProtocols(hApp), protocols );

		fprintf(trace_desc, "procs          inuse now %10d,  total used %10d, max %10d\n",
				cmSizeCurProcs(hApp), cmSizeMaxProcs(hApp), maxProcs );

		fprintf(trace_desc, "tpkt channels  inuse now %10d,  total used %10d, max %10d\n",
				cmSizeCurTpktChans(hApp), cmSizeMaxTpktChans(hApp), tpktChans );

		fprintf(trace_desc, "events         inuse now %10d,  total used %10d\n",
				cmSizeCurEvents(hApp), cmSizeMaxEvents(hApp));

		fprintf(trace_desc, "timers         inuse now %10d,  total used %10d\n",
				cmSizeCurTimers(hApp), cmSizeMaxTimers(hApp));

		fprintf(trace_desc,
				"\nRx Calls = %d, Tx Calls = %d/%d, Rx Channels %d, Tx Channels %d "
				"Errors %d, max_nready %d\n\n",
				CurrentCalls, CurrentCallsOut, TotalCallsOut, nChannelsIn, nChannelsOut,
				nErrors, max_nready );

		PrintCallbackCounters();
	}

	cv_signal( &callgen_complete_cv, THRD_DONE );

	// Wait for the appointed time to exit
_end:

	for (;;)
	{
		sleep( 100 );

		if ( shutdown_calls )
			break;
	}

//	sigsend( P_PID, getpid(), SIGINT ); replacing with POSIX kill
//	sigsend( P_PID, getpid(), SIGINT );
	kill(getpid(), SIGINT);
	kill(getpid(), SIGINT);
	return((void*) NULL);
}

int
mgenInform(Call *call, int command)
{
	MgenControlMsg mcm;
	int mfd;
	static int nxia = 0;

	// Command 0 is sent to mgen to stop media for a call
	// Command 1 is sent to mgen to begin media for a call
	// Command 2 is sent to mgen to get its media statistics

	if (mode & MODE_IXIA)
	{
		if (call == NULL)
		{
			return (-1);
		}
		if (command == 0) 
		{
			// add code to delete from list and stop media
			nxia--;
		}
		else if (command == 1)
		{

			nxia++;

			ixiaAccumCall( call->chIn[0].ip,
			               call->chOut[0].ip,
			               call->chIn[0].port,
			               call->chOut[0].port );

			if( ((nxia % IXIA_BATCH_LEN) == 0) || (nxia == nCalls) )
			{

				ixiaSendAccumCalls();

				IXIA_WAIT_DONE(  );
			}
		}
		else
		{
			printf ("statics generated by ixia\n");
			return (0);
		}
		mfd = call->chOut[0].mgenFd;
		mcm.rcvport = htons(call->chIn[0].port);
		mcm.rcvip = htonl(call->chIn[0].ip);
		mcm.sndport = htons(call->chOut[0].port);
		mcm.sndip = htonl(call->chOut[0].ip);
	}
	else
	{
        if ((command == 0) || (command == 1))
		{
			if (call == NULL)
			{
				return -1;
			}
			mfd = call->chOut[0].mgenFd;
			mcm.rcvport = htons(call->chIn[0].port);
			mcm.rcvip = htonl(call->chIn[0].ip);
			mcm.sndport = htons(call->chOut[0].port);
			mcm.sndip = htonl(call->chOut[0].ip);
		}
		else if ((command == 2) || (command == 3))
		{
			mfd = mgenFd;	
		}
		else
		{
			return -1;
		}

		if (mfd <= 0)
		{
			return -1;
		}
	
		mcm.command = htonl(command);

		if (write(mfd, (char *)&mcm, sizeof(MgenControlMsg)) < 4)
		{
   	    	perror("mgenInform() - no connection with mgen, write error ");
			exit_main_loop = 1;
			shutdown_calls = 1;
			relcomp = 1;
			return -1;
		}
	}

	return 0;
}

int
InitializePorts(unsigned long chanLocalAddr, unsigned short chanLocalPort,
				int mgenFd, int ncalls)
{
	int i, n=0;

	// re-initialize the ports

	if (!(mode & MODE_TRANSMIT))
	{
		for (i = 0; (i < nCalls)&&(n<ncalls); i++)
		{
			if (Calls[i].chOut[0].mgenFd)
			{
				continue;
			}

			Calls[i].chIn[0].ip = chanLocalAddr;
			Calls[i].chIn[0].port = chanLocalPort;
			chanLocalPort += 2;
			Calls[i].chOut[0].mgenFd = mgenFd;
			n++;
		}
	}

	if (mode & MODE_TRANSMIT)
	{
		for (i = 0; (i < nCalls)&&(n<ncalls); i++)
		{
			if (CallsOut[i].chOut[0].mgenFd)
			{
				continue;
			}

			CallsOut[i].chIn[0].ip = chanLocalAddr;
			CallsOut[i].chIn[0].port = chanLocalPort;
			chanLocalPort += 2;
			CallsOut[i].chOut[0].mgenFd = mgenFd;
			n++;
		}
	}

    if (n > 0)
    {
		chanLocalPort -= 2;
    }
 
	return n;
}

void *
mgenServerThread( void *arg )
{
	struct sockaddr_in myaddr;
	int nbio = 1;
	int flags;
	struct sockaddr_in	client;
	int	clilen, ncalls, totalcalls = 0;
	int i=1, len, j;
    int rc = 0;
    DtmfInfo info;
    MgenControlMsg mcm;

	mgenServerFd = socket (AF_INET, SOCK_STREAM, 0 );

	if (setsockopt(mgenServerFd, SOL_SOCKET, SO_REUSEADDR,  
				(void *)&i, sizeof(i)) < 0)
	{
        perror ("mgenServerThread() - setsockopt error ");         
	}

	memset (&myaddr, 0, sizeof(myaddr)); 

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (mgenPort);
	myaddr.sin_addr.s_addr  = htonl (chanLocalAddr);

	/* Bind */
	
	if (bind (mgenServerFd, (struct sockaddr *)&myaddr, sizeof(myaddr))< 0 )
	{
        perror ("mgenServerThread() - socket bind error ");
	}

	/* Listen */
	if (listen (mgenServerFd, 10) < 0)
	{
        perror ("mgenServerThread() - socket listen error ");
	}
	
	clilen = sizeof(client);
	memset(&client, 0, sizeof(client));

	if (!idaemon)
	{
		fprintf(stdout, "Starting media generator\n");
		fprintf(stdout, ">\n");
		StartMgen();
	}
	else
	{
		fprintf(stdout, "Waiting for media generator\n");
		fprintf(stdout, ">\n");
	}

_start:

	// First wait for the right number of port connections
	mgenFd = accept(mgenServerFd, (struct sockaddr *) &client, &clilen);

    /* disable nagle */
	i = 1;
	setsockopt(mgenServerFd, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));

	if (mgenFd  <= 0)
	{
		//close(mgenFd); dont close the fd, just ignore it
		mgenFd = -1;
		goto _start;
	}

	// Initialize the connection
	rc = read(mgenFd, (char *)&chanLocalAddr, 4);
	chanLocalAddr = ntohl(chanLocalAddr);

	rc = read(mgenFd, (char *)&chanLocalPort, 2);
	chanLocalPort = ntohs(chanLocalPort);

	rc = read(mgenFd, (char *)&ncalls, 4);
	ncalls = ntohl(ncalls);

	totalcalls += InitializePorts(chanLocalAddr, chanLocalPort, mgenFd, ncalls);

	// Ack
	write(mgenFd, (char *)&ncalls, 4);

	fprintf(stdout, "Media generator configured to handle %d calls\n", 
			totalcalls);
	fprintf(stdout, ">\n");

	if (totalcalls < nCalls)
	{
		// We have to wait for another mgen to connect
		fprintf(stdout, "Waiting for another media generator\n");
		fprintf(stdout, ">\n");
		goto _start;
	}

	launchThread( callgenThread, NULL, &callgen_thread, "callgenThread" );
    rc = 1;
    while (rc > 0)
    {
        memset(&mcm, 0xff, sizeof(MgenControlMsg));
        rc = read(mgenFd, (char *)&mcm, sizeof(MgenControlMsg));

        if (ntohl(mcm.command) == dtmf)
        {
            // Received DTMF digit in the RTP packet
            rc = read(mgenFd, (char *)&info, sizeof(DtmfInfo));
            //dtmfInfo.digit = ntohl(info.digit);
            //dtmfInfo.duration = ntohl(info.duration);
            //dtmfInfo.volume = ntohl(info.volume);
            fprintf(stdout, "Received signal Inband DTMF: '%c', %dms, -%ddBm0\n>\n",
                    info.digit, ntohl(info.duration), 
                    ntohl(info.volume));
            continue;
        }
        else if (ntohl(mcm.command) == 2)
        {
            // Read mgen stats
            if (read(mgenFd, (char *)&mgenStats, sizeof(MgenStats)) < 0)
            {
                perror("PrintMgenStats() - no connection with mgen, read error ");
                exit_main_loop = 1;
                shutdown_calls = 1;
                relcomp = 1;
            }
            PrintMgenStats();
        }
    }
}

int
main(int argc, char **argv)
{
	char			log_file[256],stats_file[256]={0};
	int				rc;
	struct  tm 		tm_build = { 0 };
	time_t			buildtime, now, duration;
	char 			tmpchar;
	LsMemStruct 	tmpLsMem = {0};
	int             cnt;

	lsMem =	&tmpLsMem;

	#ifdef _DEBUG_MALLOC_INC

		// Indicate we are done with the preamble so its
		// ok to use the real memset function.
	
		malloc_preamble = 0;

	#endif

#ifndef NO_LIC
	if(license_init() < 0)
	{
		// hack to make it work on compile m/c
		if (gethostid()!= hostid)
		{
            PrintErrorTime();
            fprintf(stderr, "main(): ");
            fprintf(stderr, "Missing license file or invalid "
                            "or expired license...exiting\n");
			exit(1);
		}
	}
	else if(!genEnabled())
	{
        PrintErrorTime();
        fprintf(stderr, "main(): ");
        fprintf(stderr,"Gen feature not in license file...exiting\n");
		exit(1);
	}
#endif

	//
	// Parse arguments and setup defaults
	//

	ParseArguments( argc, argv );
	getGTDData();
	read_serviceControl_list_cfg_file();
	//read_ep_list_cfg_file(cnt);
	//exit(0);

	if (idaemon)
	{
		PrintStartState();
		fprintf(stdout, "Running as daemon\n");

		freopen( "/dev/null", "r", stdin );
    	freopen( "./genout.log", "a", stdout );
    	freopen( "./generr.log", "a", stderr );

		daemonize ();
	}

	// Increase resource limits from system perspective
	// and set global variable maxfds - get 512 more than
	// what we need and divide them up giving the rv stack
	// 256 more than what should be needed and the system
	// the other 256.

	// maxfds = NetSetMaxFds( ((nCalls * 2) + 512 )  );

	// There seems to be a problem, in the radvision stack
	// that it starts using socket id's greater than the max we give
	// it, or there is some kind of overflow
	maxfds = NetSetMaxFds( 0 );

	#ifdef _DEBUG_MALLOC_INC

		m.str = "./malloc.log";
		dbmallopt( MALLOC_ERRFILE, &m );

	#endif

	ErrorScenariosInit();

	// Setup Signal handling so it is run form the
	// main thread's context

	SignalInit();

	rvmaxfds = maxfds - 256;

	// Open trace log

	memset( log_file, (int32_t) 0, 256 );

	sprintf(log_file, "./gentrace.log");

	trace_desc = fopen( log_file, "a" );

	trc_init();

	if(stats)
	{
		sprintf(stats_file, "./gen.stats");
		stats_desc = fopen( stats_file, "a");
		fprintf(stats_desc,"start time\ttcp-delay\tsetup2Proc\tsetup2Alert\tsetup2Conn\t\n");
	}

	launchThread( launchMainLoop, NULL, &mainloop_thread, "launchMainLoop" );

	cv_wait( &exit_cgen_cv, THRD_DONE );

    PrintCallSummary();

	return 0;
}								/*  main */

void
PrintCallbackCounters(void)
{
	fprintf( trace_desc, "max_nready %d\n", max_nready );

	fprintf( trace_desc, "\nOutgoing Call invocation stats\n" );

	fprintf( 	trace_desc,	"     cmCallNew() succeeded               :  %10d\n", 
				cmCallNew_counter );

	fprintf( 	trace_desc,	"     cmCallNew() failed                  :  %10d\n", 
				cmCallNewFailed_counter );

	fprintf( 	trace_desc,	"     cmCallMake() succeeded              :  %10d\n", 
				cmCallMake_counter );

	fprintf( 	trace_desc,	"     cmCallMake() failed                 :  %10d\n", 
				cmCallMakeFailed_counter );

	fprintf( trace_desc, "\nGeneral Callbacks\n\n" );

	fprintf( 	trace_desc,	"     cmEvNewCall()                       :  %10d\n", 
				cb_counters.gen.EvNewCall_counter );

	fprintf( 	trace_desc,	"     cmEvRegEvent()                      :  %10d\n", 
				cb_counters.gen.EvRegEvent_counter );

	fprintf( trace_desc, "\nCall-related Callbacks\n\n" );

	fprintf( 	trace_desc,	"     cmEvCallStateChanged()              :  %10d\n", 
				cb_counters.call.EvCallStateChanged_counter );

	fprintf( 	trace_desc,	"     cmEvCallNewRate()                   :  %10d\n", 
				cb_counters.call.EvCallNewRate_counter );

	fprintf( 	trace_desc,	"     cmEvCallInfo()                      :  %10d\n", 
				cb_counters.call.EvCallInfo_counter );

	fprintf( 	trace_desc,	"     cmEvNonStandardParam()              :  %10d\n", 
				cb_counters.call.CallNonStandardParam_counter );

	fprintf( 	trace_desc,	"     cmCallFacility()                    :  %10d\n", 
				cb_counters.call.EvCallFacility_counter );

	fprintf( 	trace_desc,	"     cmEvCallFastStartSetup()            :  %10d\n", 
				cb_counters.call.EvCallFastStartSetup_counter );

	fprintf( trace_desc, "\nCall-related state changes\n\n" );

	fprintf( 	trace_desc,	"     StateDialtone                       :  %10d\n", 
				cb_counters.call_state.StateDialtone );

	fprintf( 	trace_desc,	"     StateProceeding                     :  %10d\n", 
				cb_counters.call_state.StateProceeding );

	fprintf( 	trace_desc,	"     StateRingBack                       :  %10d\n", 
				cb_counters.call_state.StateRingBack );

	fprintf( 	trace_desc,	"     StateConnected                      :  %10d\n", 
				cb_counters.call_state.StateConnected );

	fprintf( 	trace_desc,	"     StateDisconnected                   :  %10d\n", 
				cb_counters.call_state.StateDisconnected );

	fprintf( 	trace_desc,	"     StateIdle                           :  %10d\n", 
				cb_counters.call_state.StateIdle );

	fprintf( 	trace_desc,	"     StateOffering                       :  %10d\n", 
				cb_counters.call_state.StateOffering );

	fprintf( 	trace_desc,	"     StateTranfering                     :  %10d\n", 
				cb_counters.call_state.StateTransfering );

	fprintf( 	trace_desc,	"     StateAdmissionConfirm               :  %10d\n", 
				cb_counters.call_state.StateAdmissionConfirm );

	fprintf( 	trace_desc,	"     StateAdmissionReject                :  %10d\n", 
				cb_counters.call_state.StateAdmissionReject );

	fprintf( 	trace_desc,	"     StateIncompleteAddress              :  %10d\n", 
				cb_counters.call_state.StateIncompleteAddress );

	fprintf( 	trace_desc,	"     StateWaitAddressAck                 :  %10d\n", 
				cb_counters.call_state.StateWaitAddressAck );

	fprintf( 	trace_desc,	"     StateFastStartReceived              :  %10d\n", 
				cb_counters.call_state.StateFastStartReceived );

	fprintf( 	trace_desc,	"     Unknown State                       :  %10d\n", 
				cb_counters.call_state.Unknown );

	fprintf( trace_desc, "\nCall-related state mode counters\n\n" );

	fprintf( 	trace_desc,	"     DisconnectedBusy                    :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedBusy );

	fprintf( 	trace_desc,	"     DisconnectedNormal                  :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedNormal );

	fprintf( 	trace_desc,	"     DisconnectedReject                  :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedReject );

	fprintf( 	trace_desc,	"     DisconnectedUnreachable             :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedUnreachable );

	fprintf( 	trace_desc,	"     DisconnectedUnknown                 :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedUnknown );

	fprintf( 	trace_desc,	"     DisconnectedLocal                   :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedLocal );

	fprintf( 	trace_desc,	"     ConnectedControl                    :  %10d\n", 
				cb_counters.call_state_mode.ConnectedControl );

	fprintf( 	trace_desc,	"     ConnectedCallSetup                  :  %10d\n", 
				cb_counters.call_state_mode.ConnectedCallSetup );

	fprintf( 	trace_desc,	"     ConnectedConference                 :  %10d\n", 
				cb_counters.call_state_mode.ConnectedConference );

	fprintf( 	trace_desc,	"     OfferingCreate                      :  %10d\n", 
				cb_counters.call_state_mode.OfferingCreate );

	fprintf( 	trace_desc,	"     OfferingInvite                      :  %10d\n", 
				cb_counters.call_state_mode.OfferingInvite );

	fprintf( 	trace_desc,	"     OfferingJoin                        :  %10d\n", 
				cb_counters.call_state_mode.OfferingJoin );

	fprintf( 	trace_desc,	"     OfferingCapabilityNegotiation       :  %10d\n", 
				cb_counters.call_state_mode.OfferingCapabilityNegotiation );

	fprintf( 	trace_desc,	"     OfferingCallIndependentSupplService :  %10d\n", 
				cb_counters.call_state_mode.OfferingCallIndependentSupplementaryService );

	fprintf( 	trace_desc,	"     DisconnedtedIncompleteAddress       :  %10d\n", 
				cb_counters.call_state_mode.DisconnectedIncompleteAddress );

	fprintf( 	trace_desc,	"     Unknown Mode                        :  %10d\n", 
				cb_counters.call_state_mode.Unknown );

	fprintf( trace_desc, "\nControl Callbacks\n\n" );

	fprintf( 	trace_desc,	"     cmEvCallCapabilities()              :  %10d\n", 
				cb_counters.control.EvCallCapabilities_counter );

	fprintf( 	trace_desc,	"     cmEvCallCapabilitiesExt()           :  %10d\n", 
				cb_counters.control.EvCallCapabilitiesExt_counter );

	fprintf( 	trace_desc,	"     cmEvCallNewChannel()                :  %10d\n", 
				cb_counters.control.EvCallNewChannel_counter );

	fprintf( 	trace_desc,	"     cmEvCallCapabilitiesResponse()      :  %10d\n", 
				cb_counters.control.EvCallCapabilitiesResponse_counter );

	fprintf( 	trace_desc,	"     cmEvCallCapabilitiesResponse()      :  %10d\n", 
				cb_counters.control.EvCallMasterSlaveStatus_counter );

	fprintf( 	trace_desc,	"     cmEvCallMasterSlaveStatus()         :  %10d\n", 
				cb_counters.control.EvCallMasterSlaveStatus_counter );

	fprintf( 	trace_desc,	"     cmEvCallRoundTripDelay()            :  %10d\n", 
				cb_counters.control.EvCallRoundTripDelay_counter );

	fprintf( 	trace_desc,	"     cmEvCallUserInput()                 :  %10d\n", 
				cb_counters.control.EvCallUserInput_counter );

	fprintf( 	trace_desc,	"     cmEvCallRequestMode()               :  %10d\n", 
				cb_counters.control.EvCallRequestMode_counter );

	fprintf( 	trace_desc,	"     cmEvCallMiscStatus()                :  %10d\n", 
				cb_counters.control.EvCallMiscStatus_counter );

	fprintf( 	trace_desc,	"     cmEvCallControlStateChanged()       :  %10d\n", 
				cb_counters.control.EvCallControlStateChanged_counter );

	fprintf( 	trace_desc,	"     cmEvCallMasterSlave()               :  %10d\n", 
				cb_counters.control.EvCallMasterSlave_counter );

	fprintf( trace_desc, "\nControl state changes\n\n" );

	fprintf( 	trace_desc,	"     StateConnected                      :  %10d\n", 
				cb_counters.control_state.StateConnected );

	fprintf( 	trace_desc,	"     StateConference                     :  %10d\n", 
				cb_counters.control_state.StateConference );

	fprintf( 	trace_desc,	"     StateTransportConnected             :  %10d\n", 
				cb_counters.control_state.StateTransportConnected );

	fprintf( 	trace_desc,	"     StateTransportDisconnected          :  %10d\n", 
				cb_counters.control_state.StateTransportDisconnected );

	fprintf( 	trace_desc,	"     StateFastStart                      :  %10d\n", 
				cb_counters.control_state.StateFastStart );

	fprintf( 	trace_desc,	"     Unknown State                       :  %10d\n", 
				cb_counters.control_state.Unknown );

	fprintf( trace_desc, "\nChannel Callbacks\n\n" );

	fprintf( 	trace_desc,	"     cmEvChannelStateChanged()           :  %10d\n", 
				cb_counters.channel.EvChannelStateChanged_counter );

	fprintf( 	trace_desc,	"     cmEvChannelNewRate()                :  %10d\n", 
				cb_counters.channel.EvChannelNewRate_counter );

	fprintf( 	trace_desc,	"     cmEvChannelMaxSkew()                :  %10d\n", 
				cb_counters.channel.EvChannelMaxSkew_counter );

	fprintf( 	trace_desc,	"     cmEvChannelSetAddress()             :  %10d\n", 
				cb_counters.channel.EvChannelSetAddress_counter );

	fprintf( 	trace_desc,	"     cmEvChannelSetRTCPAddress()         :  %10d\n", 
				cb_counters.channel.EvChannelSetRTCPAddress_counter );

	fprintf( 	trace_desc,	"     cmEvChannelParameters()             :  %10d\n", 
				cb_counters.channel.EvChannelParameters_counter );

	fprintf( 	trace_desc,	"     cmEvChannelRtpDynamicPayloadType()  :  %10d\n", 
				cb_counters.channel.EvChannelRTPDynamicPayloadType_counter );

	fprintf( 	trace_desc,	"     cmEvChannelVideoFastUpdatePicture() :  %10d\n", 
				cb_counters.channel.EvChannelVideoFastUpdatePicture_counter );

	fprintf( 	trace_desc,	"     cmEvChannelVideoFastUpdateGOB()     :  %10d\n", 
				cb_counters.channel.EvChannelVideoFastUpdateGOB_counter );

	fprintf( 	trace_desc,	"     cmEvChannelVideoFastUpdateMB()      :  %10d\n", 
				cb_counters.channel.EvChannelVideoFastUpdateMB_counter );

	fprintf( 	trace_desc,	"     cmEvChannelHandle()                 :  %10d\n", 
				cb_counters.channel.EvChannelHandle_counter );

	fprintf( 	trace_desc,	"     cmEvChannelGetRTCPAddress()         :  %10d\n", 
				cb_counters.channel.EvChannelGetRTCPAddress_counter );

	fprintf( 	trace_desc,	"     cmEvChannelGetRequestCloseStatus()  :  %10d\n", 
				cb_counters.channel.EvChannelRequestCloseStatus_counter );

	fprintf( 	trace_desc,	"     cmEvChannelTSTO()                   :  %10d\n", 
				cb_counters.channel.EvChannelTSTO_counter );

	fprintf( 	trace_desc,	"     cmEvChannelMediaLoopStatus()        :  %10d\n", 
				cb_counters.channel.EvChannelMediaLoopStatus_counter );

	fprintf( 	trace_desc,	"     cmEvChannelReplace()                :  %10d\n", 
				cb_counters.channel.EvChannelReplace_counter );

	fprintf( 	trace_desc,	"     cmEvChannelFlowControlToZero()      :  %10d\n", 
				cb_counters.channel.EvChannelFlowControlToZero_counter );

	fprintf( trace_desc, "\nChannel state changes\n\n" );

	fprintf( 	trace_desc,	"     StateDialtone                       :  %10d\n", 
				cb_counters.channel_state.StateDialtone );

	fprintf( 	trace_desc,	"     StateRingBack                       :  %10d\n", 
				cb_counters.channel_state.StateRingBack );

	fprintf( 	trace_desc,	"     StateConnected                      :  %10d\n", 
				cb_counters.channel_state.StateConnected );

	fprintf( 	trace_desc,	"     StateDisconnected                   :  %10d\n", 
				cb_counters.channel_state.StateDisconnected );

	fprintf( 	trace_desc,	"     StateIdle                           :  %10d\n", 
				cb_counters.channel_state.StateIdle );

	fprintf( 	trace_desc,	"     StateOffering                       :  %10d\n", 
				cb_counters.channel_state.StateOffering );

	fprintf( 	trace_desc,	"     Unknown State                       :  %10d\n", 
				cb_counters.channel_state.Unknown );

	fprintf( trace_desc, "\nChannel state mode counters\n\n" );

	fprintf( 	trace_desc,	"     On                                  :  %10d\n", 
				cb_counters.channel_state_mode.On );

	fprintf( 	trace_desc,	"     Off                                 :  %10d\n", 
				cb_counters.channel_state_mode.Off );

	fprintf( 	trace_desc,	"     DisconnectedLocal                   :  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedLocal );

	fprintf( 	trace_desc,	"     DisconnectedRemote                  :  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedRemote );

	fprintf( 	trace_desc,	"     DisconnectedMasterSlaveConflict     :  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedMasterSlaveConflict );

	fprintf( 	trace_desc,	"     Duplex                              :  %10d\n", 
				cb_counters.channel_state_mode.Duplex );

	fprintf( 	trace_desc,	"     DisconnectedReasonUnknown           :  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedReasonUnknown );

	fprintf( 	trace_desc,	"     DisconnectedReasonReopen            :  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedReasonReopen );

	fprintf( 	trace_desc,	"     DisconnectedReasonReservationFailure:  %10d\n", 
				cb_counters.channel_state_mode.DisconnectedReasonReservationFailure );

	fprintf( 	trace_desc,	"     Unknown Mode                        :  %10d\n", 
				cb_counters.channel_state_mode.Unknown );

	fprintf( trace_desc, "\nProtocol Callbacks\n\n" );

	fprintf( 	trace_desc,	"     cmEvCallHookListen()                :  %10d\n", 
				cb_counters.protocol.HookListen_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookListening()             :  %10d\n", 
				cb_counters.protocol.HookListening_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookConnecting()            :  %10d\n", 
				cb_counters.protocol.HookConnecting_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookInConn()                :  %10d\n", 
				cb_counters.protocol.HookInConn_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookRecv()                  :  %10d\n", 
				cb_counters.protocol.HookSend_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookRecv()                  :  %10d\n", 
				cb_counters.protocol.HookRecv_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookSendto()                :  %10d\n", 
				cb_counters.protocol.HookSendTo_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookRecvFrom()              :  %10d\n", 
				cb_counters.protocol.HookRecvFrom_counter );

	fprintf( 	trace_desc,	"     cmEvCallHookClose()                 :  %10d\n", 
				cb_counters.protocol.HookClose_counter );

	fflush(	trace_desc );

}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

//
//	Function	:
//		SignalInit()
//
//	Arguments	:
//		None
//
//	Description	:
//			Initialize signal handling for the
//		media gateway.	This routine initializes
//		signal processing settings for the process
//		and creates	a thread to handle signals for
//		the process. This thread is the only thread
//		that actually receives signals.
//
void
SignalInit(void)
{
	static			sigset_t	signal_handler_blocked_mask;
	static	struct	sigaction	sigact;
	static			stack_t		sigstk;

	if ((sigstk.ss_sp = (char *)malloc(SIGSTKSZ)) == NULL)
	{
        PrintErrorTime();
        fprintf(stderr, "SignalInit(): ");
        fprintf(stderr, "Failed to allocate space for signal stack\n");
	}
	else
	{
		sigstk.ss_size = SIGSTKSZ;
		sigstk.ss_flags = 0;
		if (sigaltstack(&sigstk, (stack_t *)0) < 0)
        {
            perror("SignalInit() - sigaltstack error ");
        }
	}

	sigemptyset( &sigact.sa_mask );
	sigact.sa_handler = SIG_IGN;
	sigact.sa_flags = (SA_RESTART|SA_NOCLDSTOP|SA_ONSTACK);
	sigaction( SIGCHLD, &sigact, NULL );

	sigemptyset( &signal_handler_blocked_mask );
	sigaddset( &signal_handler_blocked_mask, SIGHUP );

	sigemptyset( &threads_signal_mask );
	sigaddset( &threads_signal_mask, SIGTERM );
	sigaddset( &threads_signal_mask, SIGALRM );
	sigaddset( &threads_signal_mask, SIGHUP );
	sigaddset( &threads_signal_mask, SIGPIPE);

	sigact.sa_handler = &sigdummy_handler;
	sigact.sa_mask = signal_handler_blocked_mask;
	sigact.sa_flags = (SA_RESTART|SA_ONSTACK);
	sigaction( SIGALRM, &sigact,	NULL );
	sigaction( SIGTERM,	&sigact,	NULL );

	sigact.sa_flags = (SA_RESETHAND|SA_NODEFER|SA_ONSTACK);
	sigaction( SIGBUS,	&sigact,	NULL );
	sigaction( SIGSEGV, &sigact,	NULL );
	sigaction( SIGILL,	&sigact,	NULL );
	sigaction( SIGCLD,	&sigact,	NULL );

	sigact.sa_flags = SA_ONSTACK;
	sigaction( SIGINT,	&sigact,	NULL );
	sigaction( SIGPIPE,	&sigact,	NULL );

	//
	// Setup pthread signal mask for main() thread.
	// It will be inheritted by any other spawned threads
	//

	pthread_sigmask( SIG_BLOCK, &signal_handler_blocked_mask, NULL );
}

//
//	Function	:
//		SignalHandler()
//
//	Description	:
//		signal handler thread that handles all signal
//		processing for the Media Gateway daemon.
//
static inline void
SignalHandler(int32_t sig)
{
	char			*c_sig;
	char			sigstr[50];
	int32_t			warn = 0;
	int32_t			fatal = 0;

	if ( sig == SIGALRM )
	{
        PrintErrorTime();
        fprintf(stderr, "SignalHandler(): ");
        fprintf(stderr, "Signal SIGALARM received\n");
	}
	else if (sig == SIGPIPE)
	{
        PrintErrorTime();
        fprintf(stderr, "SignalHandler(): ");
        fprintf(stderr, "Signal SIGPIPE received\n");
	}
	else
	{
		switch ( sig )
		{
		case SIGTERM:
			c_sig = "SIGTERM";
			warn = 1;
			break;
		case SIGINT:
			c_sig = "SIGINT";
			if ( shutdown_calls )
				return;
			relcomp = 1;
			break;
		case SIGHUP:
			c_sig = "SIGHUP";
			warn = 1;
			break;
		case SIGBUS:
			c_sig = "SIGBUS";
			warn = 1;
			fatal = 1;
			break;
		case SIGSEGV:
			c_sig = "SIGSEGV";
			warn = 1;
			fatal = 1;
			break;
		case SIGILL:
			c_sig = "SIGILL";
			warn = 1;
			fatal = 1;
			break;
		case SIGCLD:
			if (mode |= MODE_IXIA)
			{
				wait (NULL);
				c_sig = "SIGCLD";
				warn = 1;
				fatal = 1;
				break;
			}
		default:
			sprintf( sigstr, "%d", sig );
			c_sig = sigstr;
			break;
		}

		if (warn)
		{
            PrintErrorTime();
            fprintf(stderr, "SignalHandler(): ");
            fprintf(stderr, "Signal %s received...terminating process\n",
                    c_sig );
		}

		millisleep(1000);

		trc_dump();

		shutdown_calls = 1;
		exit_main_loop = 1;

		if ( fatal )
		{
            PrintErrorTime();
            fprintf(stderr, "SignalHandler(): ");
            fprintf(stderr, "Signal %s received...aborting\n", c_sig );
            PrintCallSummary();
			abort();
		}
	}
	return;
}

void
sigdummy_handler(int signo) 
{
	fprintf( trace_desc, "Received signal %d\n", signo );
	fflush( trace_desc );
	SignalHandler(signo);
	return;
}

static  int32_t prio_inited = 0;
#ifndef NETOID_LINUX //RT scheduling is not to be porrted on linux
//
//  Function    :
//      get_priocntl_info()
//
//  Arguments   :
//      None.
//
//
//  Description :
//
//      This routine is called by the main()
//      routine of the Media Gateway process
//      at initialization time. It gets
//      gets RT information from the system
//      via priocntl() call to be used through
//      out the media gateway processes operation.
//
//  Return values:
//      None.
//
static  void
get_priocntl_info( void )
{
	if ( !prio_inited )
	{

		rtinfo_t    *rtinfop = (struct rtinfo *) rt_pcinfo.pc_clinfo;

		(void) strcpy( rt_pcinfo.pc_clname, "RT" );
		priocntl( (int32_t) 0, (idtype_t) 0, (id_t) PC_GETCID, (caddr_t) &rt_pcinfo);

		prio_inited = 1;
	}

	return;
}
#endif //NETOID_LINUX

void
ParseArguments( int argc, char **argv )
{
	char*	mptr1;
	char*	mptr2;
	int32_t	i;
        int herror=0;
	memset(dtg,0,256);
	memset(otg,0,256);
	memset(g_destGWAddr,0,256);

	// parse the args and adjust defaults accordingly

	while ( --argc > 0 )
	{
        if ((argv[argc][0] == '-') || (argv[argc][0] == '+'))
        {
			switch (argv[argc][1])
			{
			case 'h':
				PrintUsage();
				exit(0);

			case 'Z':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(g_destGWAddr, argv[argc + 1], 256);
					g_destGWAddrLong = inet_addr(argv[argc + 1]);
				        g_hairPin = 0;
				}
				break;

			case 'A':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(config_template_file, argv[argc + 1], 256);
				}
				break;

			case 'a':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(config_file, argv[argc + 1], 256);
				}
				break;

			case 'I':
                if (argv[argc][2] == 'x')
                {
                    mode |= MODE_IXIA;
                    if( argv[argc + 1] && 
                            (argv[argc + 1][0] != '+') && 
                            (argv[argc + 1][0] != '-') ) 
                    {
                        strncpy( ixia_testScriptName, 
                                argv[argc + 1], 
                                IXIA_TESTSCRIPT_NAME_LEN );
                    }
                    else
                    {
                        strcpy( ixia_testScriptName,
                                IXIA_DEF_TESTSCRIPT );
                    }
                    break;
                }
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
                    if (argv[argc][2] == 'h')
                    {
                        nx_strlcpy(h323Id, argv[argc + 1], 256);
                        h323AliasCmdLine = 1;
                    }
                    else if (argv[argc][2] == 'e')
                    {
                        nx_strlcpy(e164Id, argv[argc + 1], 256);
                        e164AliasCmdLine = 1;
                    }
				}
				break;

			case 'o':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(otg, argv[argc + 1], 256);
				}
				break;

			case 'O':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(dtg, argv[argc + 1], 256);
				}
				break;

			case 'n':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nCalls = atoi(argv[argc + 1]);
				}
				break;
	
			case 'Y':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					xCalls = atoi(argv[argc + 1]);
				}
				break;

			case 'b':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					burst = atoi(argv[argc + 1]);
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						burstInterval = atoi(argv[argc + 2]);
						call_rate = (float)burst*1000/(float)burstInterval;
					}
				}
				break;
	
			case 'd':  // specify called party number
				if(argv[argc][2] =='f')
               	{
					if (argv[argc + 1] && 
						(argv[argc + 1][0] != '+') && 
						(argv[argc + 1][0] != '-'))
					{
						if((dstfp = fopen(argv[argc + 1]
,"r"))==NULL)
                    	{
							fprintf(stderr,"ParseArguments(): Error opening file %s\n",argv[argc + 1]);
							exit(1);
                     	}
                     	readcalledpnfromfile=1;
						break;
                   	}
               	}

				else if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(calledpn, argv[argc + 1], 256);
					if (argv[argc][2] == '+')
					{
						incCalledPn = 1;
					}
				}
				break;

			case 's': // specify calling party number
				if(argv[argc][2] == 'f')
				{
					if (argv[argc + 1] && 
						(argv[argc + 1][0] != '+') && 
						(argv[argc + 1][0] != '-'))
					{
						if((srcfp = fopen(argv[argc + 1]
,"r"))==NULL)
						{
							fprintf(stderr,"ParseArguments(): Error opening file %s\n",argv[argc + 1]);
							exit(1);
						}
						readcallingpnfromfile=1;
						break;
					}
				}

				else if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(callingpn, argv[argc + 1], 256);
					if (argv[argc][2] == '+')
					{
						incCallingPn = 1;
					}
				}
				break;

			case 'g':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(gwAddr, argv[argc + 1], 256);
					gwAddrLong = inet_addr(argv[argc + 1]);
					gwAddrFlag = 1;
				}
				break;
	
			case 'G':
                if ((strlen(argv[argc]) == 3) && (argv[argc][2] == 'K'))
                {
                    if (argv[argc + 1] && 
                            (argv[argc + 1][0] != '+') && 
                            (argv[argc + 1][0] != '-'))
                    {
                        nx_strlcpy(gk_cfg_file, argv[argc + 1], 256);
                    }
                    gkMode = 1;
                }
                else
                {
                    autoReg = 1;
                }
				break;
	
			case 'p':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					gwPort = atoi(argv[argc + 1]);
				}
				break;
	
			case 'm':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					chanLocalAddr = ntohl(inet_addr(argv[argc + 1]));
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						chanLocalPort = atoi(argv[argc + 2]);
						mediaStartPort = chanLocalPort;
					}
				}
				break;
	
			case 'M':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					mgenPort = atoi(argv[argc + 1]);
				}
				break;
	
			case 'D':
				dropISDN = 1;
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					isdnCode = atoi(argv[argc + 1]);
				}
				break;
	
			case 'f':
				if (argv[argc][0] == '-')	// historical reasons
				{
					fastStart = 0;	// slow start
					doh245 = 1;		// do H.245
					if (argv[argc][2] == '+')
					{
						doh245 = 0;		// No H.245
						fastStart = 1;	
					}
				}

				/* Code added for disabling h245 tunneling */

				if (argv[argc][0] == 't')
				{
					h245TunnFlag = 1;
					fprintf(stdout, "H245 Tunneling disabled.\n");
				}
				break;

            case 'L':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					if((isalpha(argv[argc+1][0])))
					{
						localIp = ntohl(ResolveDNS(argv[argc + 1], &herror));
						if(herror)
						{
							fprintf(stdout,"Error resolving name %s\n",argv[argc + 1]);
							exit(1);
						}
					}
					else
					{
						localIp = inet_addr(argv[argc + 1]);
					}
					localIpFlag = 1;
				}
	       		break;
	
            case 'l':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
                    if (argv[argc][2] == 'r')
                        localRasPort = atoi(argv[argc + 1]);
                    if (argv[argc][2] == 'q')
                        localQ931Port = atoi(argv[argc + 1]);
					localPortFlag = 1;
				}
	       		break;
	
			case 'i':
				mode |= MODE_ITERATIVE;
	
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					tdMonitor = atoi(argv[argc + 1]);
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						tdSetup = atoi(argv[argc + 2]);
						tdProceeding = tdSetup;
						tdRingBack = tdSetup;
						if (argv[argc + 3] && 
							(argv[argc + 3][0] != '+') && 
							(argv[argc + 3][0] != '-'))
						{
							tdConnect = atoi(argv[argc + 3]);
							if (argv[argc + 4] && 
								(argv[argc + 4][0] != '+') && 
								(argv[argc + 4][0] != '-'))
							{
								tdIdle = atoi(argv[argc + 4]);
							}
							tdCallDuration = tdIdle + tdConnect;
						}
					}
				}
				break;

			case 'j':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					ncodecs = atoi(argv[argc+1]);
					if (ncodecs < 1)
					{
						ncodecs = 1;
					}
					if (ncodecs > 5)
					{
						ncodecs = 5;
					}
				}
				break;
	
			case 'r':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					rvdebug = atoi(argv[argc + 1]);
					debug = rvdebug;
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						mptr1 = argv[argc + 2];
						if ( mptr1 == (char*) 0 )
						{
							break;
						}
						if (strcmp(mptr1, "-") == 0)
						{
							break;
						}
	
						while (mptr2 = strchr(mptr1, ','))
						{
							memset( modules[module_count], 0, 256 );
							strncpy(modules[module_count], mptr1, 
									(mptr2 - mptr1));
							mptr1 = mptr2 + 1;
							module_count++;
						}
	
						// add the last one
						nx_strlcpy(modules[module_count], mptr1, 
								sizeof(modules[module_count]));
						module_count++;
					}
				}
				break;
	
			case 'y':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					errorScenario = atoi(argv[argc + 1]);
				}
				break;
	
			case 'c':
				mode |= MODE_MGCP;
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(mgen_file, argv[argc + 1], 256);
				}
				break;
	
			case 'P':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(mgenPayloadLen, argv[argc + 1], 32);
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						nx_strlcpy(mgenPayloadInterval, argv[argc + 2], 8);
					}
				}
				break;
	
			case 'Q':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(mgenNumThreads, argv[argc + 1], 8);
				}
				break;

			case 'R':
				strcpy(mgenTxRxSamePort, "-s");
				break;

			case 'B':
				strcpy(mgenLoopback, "-o");
				break;

			case 't':
				mode |= MODE_TRANSMIT;
				break;
	
			case 'C':
				shutEmptyConnection = 0;
				break;
	
			case 'e':
				reportAllDisconnects = 1;
				break;
	
			case 'S': 
				stats = 1;
				break;
	
			case 'x':
				if(argv[argc][2] == 'f')
				{
					setModeTransfer = 1;					
				}
				else
				{
					fax = 1;
				}
				break;
	
			case 'X':
				multipleCalls = 1;
				break;
	
			case 'z':
				idaemon = 1;
				break;
	
			case 'T':
				automode = 1;
				break;

			case 'U':
                                asrMode = 1;
                                if (argv[argc + 1] &&
                                        (argv[argc + 1][0] != '+') &&
                                        (argv[argc + 1][0] != '-'))
                                {
                                        iprobability = atof(argv[argc + 1]);

                                        if (iprobability > 1)
                                        {
                                                PrintErrorTime();
                                                fprintf(stderr, "ParseArguments(): ");
                                                fprintf(stderr, "Probability value greater than 100%."
                                                        "ASR reponse option ignored\n");

                                                asrMode = 0;
                                        }
                                        else
                                                GenerateSeed();
                                }
                                break;

			 case 'N':
                                maxCallsMode = 1;

                                if (argv[argc + 1] &&
                                        (argv[argc + 1][0] != '+') &&
                                        (argv[argc + 1][0] != '-'))
                                {
                                        totalMaxCalls = atof(argv[argc + 1]);
                                }
                                break;


			case 'v':
                                fprintf(stdout, "\nVersion : %s\n", version);
                                exit(0);


	
			default:
				break;
			}
		}
	}

	if (fax && !doh245)
	{
		doh245 = 1;
	}

	if (call_rate && tdCallDuration)
	{
		nCalls = call_rate*tdCallDuration+1;
	}
	else if (call_rate && nCalls && (mode & MODE_ITERATIVE))
	{
		tdCallDuration = nCalls/call_rate;
		tdIdle = 0.2*tdCallDuration;
		tdConnect = tdCallDuration-tdIdle;
	}
	else if (tdCallDuration && nCalls)
	{
		call_rate = nCalls/tdCallDuration;
	}

	if (!call_rate) call_rate = 10;

	if (xCalls && (xCalls < nCalls)) xCalls = nCalls;

	if (!burst)
	{
		burstInterval = 100;
		burst = (call_rate*burstInterval)/1000;
	}

	callTimePeriod = ((hrtime_t)burstInterval*1000000)/((hrtime_t)burst);

    if (mode & MODE_TRANSMIT)
    {
        if (strlen(callingpn) == 0)
        {
            strcpy(callingpn, "555");
        }
        if (strlen(calledpn) == 0)
        {
            strcpy(calledpn, "666");
        }
    }
    else
    {
        if (strlen(callingpn) == 0)
        {
            strcpy(callingpn, "666");
        }
        if (strlen(calledpn) == 0)
        {
            strcpy(calledpn, "555");
        }
    }

    nx_strlcpy(startCallingPn, callingpn, 256);
    nx_strlcpy(startCalledPn, calledpn, 256);
}

static void
GenerateSeed()
{
        time_t seconds;

        time(&seconds);

        srand48((unsigned int)seconds);
}


static void
PrintUsage(void)
{
	fprintf(stdout, "\nUsage :\n");
	fprintf( stdout, 
"gen [-a <.val config file>] - config file (default config.val)\n" );
	fprintf( stdout, 
"    [-A <.val template file>] - config template file (default config-template.val)\n" );
	fprintf( stdout, 
"    [-b <burst-size> <burst-interval>] - default 1/100msec\n" );
	fprintf( stdout, 
"    [-c [mgen executable name]] - if running as daemon, wait for media gen, otherwise start it\n" );
	fprintf( stdout, 
"    [-C] - maintain connection\n" );
    fprintf(stdout,
"    [-d[+] <called party #>] - default 666 if transmitter, 555 if receiver; use + to increment number for multiple calls\n");
	fprintf(stdout, 
"    [-df <dst numbers file>] - destination numbers in file should be equal or more than the number of calls specified using \'-n\' option. \n Each number should be on seperate line \n");
	fprintf( stdout, 
"    [-D <ISDN cause code>] - drop call with code; default 16 (normal)\n" );
	fprintf( stdout, 
"    [-e] - report all disconnects\n" );
	fprintf( stdout, 
"    [-f] - slow start; default is fast start and H.245\n" );
	fprintf( stdout, 
"    [-f+] - fast start but no H.245\n" );
 	fprintf( stdout,
"    [-ft] - fast start but no H.245 Tunneling; default is fast start and H.245 Tunneling\n" );
	fprintf( stdout, 
"    [-g <dest gateway IP>] - default 127.0.0.1\n" );
	fprintf( stdout, 
"    [-Z <dest gateway IP>] - default hair-pin,This option is only valid in GK mode\n" );
	fprintf( stdout, 
"    [-G <register with default GK>] - For dynamic EP\n" );
	fprintf( stdout, 
"    [-GK <configuration file for building EP list>] - GK mode for gen\n" );
	fprintf(stdout, 
"    [-h] - help for usage\n" );
	fprintf( stdout, 
"    [-i <monitor> <setup> <connect> <idle>] - default 0 seconds\n" );
	fprintf( stdout, 
"    [-Ie] - E164-ID to be sent in RAS registration messages\n" );
	fprintf( stdout, 
"    [-Ih] - H323-ID to be sent in RAS registration messages\n" );
	fprintf( stdout, 
"    [-Ix] - use ixia for media\n" );
	fprintf( stdout, 
"    [-j <# of fast start channels>]\n" );
	fprintf( stdout, 
"    [-lq] - Local Q.931 signalling port for the gen\n" );
	fprintf( stdout, 
"    [-lr] - Local RAS signalling port for the gen\n" );
	fprintf( stdout, 
"    [-L] - Local call signalling address for the gen\n" );
	fprintf( stdout, 
"    [-m <media IP> <media start port>] - defaults are local IP and port 49200\n" );
	fprintf( stdout, 
"    [-M <mgen port>] - default 49160\n" );
	fprintf(stdout, 
"    [-n <ncalls>] - default 25\n" ); 
    fprintf(stdout,
"    [-N] - Maximum number of Calls\n");
	fprintf( stdout, 
"    [-o <orig. trunk group>] - by default orig. trunk group is not sent\n" );
        fprintf( stdout,
"                               In GK mode,it will sent as source circuit ID\n");
	fprintf( stdout, 
"    [-O <dest. trunk group>] - by default dest. trunk group is not sent\n" );
        fprintf( stdout,
"                               In GK mode,it will sent as destination circuit ID\n");
	fprintf( stdout, 
"    [-p <dest gateway port>] - default 1720\n" );
	fprintf( stdout, 
"    [-r <debug level> <module1,module2,...>]\n" );
	fprintf(stdout, 
"    [-s[+] <calling party #>] - default 555 if transmitter, 666 if receiver; use + to increment number for multiple calls\n");
	fprintf(stdout, 
"    [-sf <src numbers file>] - source numbers in file should be equal or more than the number of calls specified using \'-n\' option. \n Each number should be on seperate line \n");
	fprintf( stdout, 
"    [-S] - print per call statistics\n" );
	fprintf(stdout, 
"    [-t] - run as transmitter; default is receiver\n" );
	fprintf( stdout, 
"    [-T] - automated testing mode\n" );
	fprintf(stdout,
"    [-U] - (ASR probability (<1) - default 0\n");
    fprintf(stdout,
"    [-v] - version number\n");
	fprintf( stdout, 
"    [-x] - fax call; H.245 is always enabled for this\n" );
	fprintf( stdout, 
"    [-X] - multiplex calls over same TCP connection\n" );
	fprintf( stdout, 
"    [-y <error scenario #>] - activate error scenario\n" );
	fprintf( stdout, 
"    [-Y] - number of fax calls\n" );
	fprintf( stdout, 
"    [-z] - run as daemon\n" );
	fprintf( stdout, "\n");

	PrintMgenOptions();

	fprintf(stdout, "\n");
	PrintInteractiveCmds();

	fprintf(stdout, "\n");
	PrintErrorScenarios();
}

int
PrintStartState()
{
    if (automode)
    {
        fprintf(stdout,
                "\nTotalCalls          :    %d"
		"\nTotalMaxCalls       :    %d"
                "\nConcurrentCalls     :    %d"
                "\nCallRate(per sec)   :    %f"
                "\nBurst               :    %d"
                "\nBurstInterval(msec) :    %d"
                "\nConnectTime(sec)    :    %d"
                "\nIdleTime(sec)       :    %d"
                "\nStartCallingParty   :    %s"
                "\nStartCalledParty    :    %s"
                "\nIncCallingParty     :    %s"
                "\nIncCalledParty      :    %s"
                "\nFunctioningAs       :    %s"
                "\nGatewayIP           :    %s"
                "\nGatewayPort         :    %d"
                "\nFax                 :    %s"
                "\nFastStart           :    %s"
                "\nConfigTemplateFile  :    %s"
                "\nConfigFile          :    %s"
                "\nErrorScenario       :    %s"
                "\nAutoTestingMode     :    %s"
		"\nASR Probability     :    %f"
                "\n",
                nCalls,totalMaxCalls, 
                (tdCallDuration)?(tdConnect*nCalls)/tdCallDuration:nCalls,
                call_rate,
                burst, burstInterval,
                tdConnect, tdIdle,
                startCallingPn, startCalledPn,
				incCallingPn ? "yes" : "no", incCalledPn ? "yes" : "no",
                (mode & MODE_TRANSMIT) ? "transmitter" : "receiver",
                gwAddr, gwPort, 
				fax ? "yes" : "no",
                fastStart ? "yes" : "no",
                config_template_file, config_file, 
				ErrorScenarios(errorScenario),
                automode ? "yes" : "no", iprobability);
    }
    else
    {
        fprintf(stdout, 
                "ncalls = %d, concurrent calls = %d, burst = %d:%d, call rate = %f/s\nconnect time = %ds, idle time = %ds, callingpn = %s, calledpn = %s\ntx = %s, rx = %s, gw ip = %s, gw port = %d\nfax = %s, fast start = %s, auto mode = %s\nconfig file = %s\n asr probability = %f \n\n",
                nCalls, 
                (tdCallDuration)?(tdConnect*nCalls)/tdCallDuration:nCalls,
                burst, burstInterval, call_rate, 
                tdConnect, tdIdle,
                callingpn, calledpn,
                (mode & MODE_TRANSMIT) ? "yes" : "no",
                (mode & MODE_RECEIVE) ? "yes" : "no",
                gwAddr, gwPort,
                fax?"yes":"no",
                fastStart?"yes":"no",
                automode?"yes":"no",
                config_file , iprobability);

        fprintf(stdout, "error scenario=%s\n", ErrorScenarios(errorScenario));
        fprintf(stdout, "T=%lldms\n", callTimePeriod/1000000);
	}

	fprintf(stdout, ">\n");
}

int
getNextPhone(char *phone)
{
	int len, done = 0;
	char last;

	len = strlen(phone);

	while (!done && (len>0))
	{
		last = phone[len-1]-'0';
		if (last == 9)
		{
			phone[len-1]='0';
			len--;
		}
		else
		{
			last ++;
			phone[len-1] = last+'0';
			done = 1;
		}
	}

	if (!done)
		return( -1 );
	else
		return( 1 );
}

int getNextPhoneFromFile(char *phone,FILE *fp)
{
	if((fp == NULL) || ((fgets(phone,256,fp))==NULL))
			return 0;
	phone[strlen(phone)-1]='\0';	//removing the new line character from the array
	return 1;
}

int PrintCallStats(Call *call,int stats)
{
	int m = 1000;
	static int i = 0;
	int	tcpdelay = 0,setup2Proc = 0,setup2Alert = 0,setup2Conn = 0;

	if(stats)
	{
		i++;
			tcpdelay = ((call->hrSetup - call->hrOriginate)/m);
		 if(call->hrProceeding)
			 setup2Proc = ((call->hrProceeding - call->hrSetup)/m);
		 if(call->hrRingBack)
			setup2Alert = ((call->hrRingBack - call->hrSetup)/m);
			setup2Conn = ((call->hrConnect - call->hrSetup)/m);
		fprintf( stats_desc,"%d\t%d\t\t%d\t\t%d\t\t%d\n",
			call->tsOriginate,tcpdelay,setup2Proc,setup2Alert,setup2Conn);

		call->tsSetup=0;
		call->tsOriginate=0;
		call->tsProceeding=0;
		call->tsConnect=0;
		call->hrSetup=0;
		call->hrOriginate=0;
		call->hrProceeding=0;
		call->hrConnect=0;
		call->ntimes=0;
	}
	
	return 0;
}

int
PrintStatus(int debug)
{
	static	int debug_counter;
	time_t now;

    if (automode)
	{
		fprintf(stdout,
       		      "CurrentCallsTx     :    %d"
       		    "\nTotalCallsTx       :    %d"
       		    "\nCurrentCallsRx     :    %d"
       		    "\nTotalCallsRx       :    %d"
       		    "\nFailedSetups       :    %d"
       		    "\nFailedFastStarts   :    %d"
       		    "\nFailedProceedings  :    %d"
       		    "\nFailedAlertings    :    %d"
       		    "\nReleaseCompletes   :    %d"
       		    "\nNumErrors          :    %d"
       		    "\n",
				CurrentCallsOut, TotalCallsOut, CurrentCalls, TotalCalls, 
				nFailedSetups, nFailedFaststarts, nFailedProceeding, 
       		    nFailedRingBack, nRelComps, nErrors);
        return 0;
	}

	if (h323Initialized == 0)
	{
        fprintf(stdout, "Not ready yet\n");
		return 0;
	}
    
	if (debug)
	{
		fprintf(stdout,
				"\nRx Calls %d/%d, Tx Calls %d/%d, Rx Channels %d, Tx Channels %d "
					"Errors %d - [ rc %d, su %d, fst %d, pr %d, al %d ], max_ready %d\n",
				CurrentCalls,
				TotalCalls,
				CurrentCallsOut,
				TotalCallsOut,
				nChannelsIn,
				nChannelsOut,
				nErrors,
				nRelComps,
				nFailedSetups,
				nFailedFaststarts,
				nFailedProceeding,
				nFailedRingBack,
				max_nready );

		time(&now);
		fprintf(stdout, "next call timeout %ds, wave #%d\n", timerHead?timerHead->timeNextEvt-now:-1, wave);

#ifdef _print_trc_
		fprintf( 	trace_desc,
					"\n==== Status #  %10d ====\n", debug_counter++ );

		fprintf(	trace_desc,
					"\nRx Calls %d, Tx Calls %d/%d, Rx Channels %d, Tx Channels %d "
						"Errors %d - [ rc %d, su %d, fst %d ], max_ready %d\n\n",
					CurrentCalls,
					CurrentCallsOut,
					TotalCallsOut,
					nChannelsIn,
					nChannelsOut,
					nErrors,
					nRelComps,
					nFailedSetups,
					nFailedFaststarts,
					max_nready );

		if (mode & MODE_RECEIVE)
			fprintf(	trace_desc,
						"total calls received %d\n", TotalCalls );
		else
		if (mode & MODE_TRANSMIT)
			fprintf(	trace_desc,
						"total calls placed %d\n",
						call_count );

		fprintf(	trace_desc,
					"channels       inuse now %10d,  total used %10d, max %10d\n",
					cmSizeCurChannels(hApp),
					cmSizeMaxChannels(hApp),
					channels );

		fprintf(	trace_desc,
					"protocols      inuse now %10d,  total used %10d, max %10d\n",
					cmSizeCurProtocols(hApp),
					cmSizeMaxProtocols(hApp),
					protocols );

		fprintf(	trace_desc,
					"procs          inuse now %10d,  total used %10d, max %10d\n",
					cmSizeCurProcs(hApp),
					cmSizeMaxProcs(hApp),
					maxProcs );

		fprintf(	trace_desc,
					"tpkt channels  inuse now %10d,  total used %10d, max %10d\n",
					cmSizeCurTpktChans(hApp),
					cmSizeMaxTpktChans(hApp),
					tpktChans );

		fprintf(	trace_desc,
					"events         inuse now %10d,  total used %10d\n",
					cmSizeCurEvents(hApp),
					cmSizeMaxEvents(hApp) );

		fprintf(	trace_desc,
					"timers         inuse now %10d,  total used %10d\n",
					cmSizeCurTimers(hApp),
					cmSizeMaxTimers(hApp) );

		PrintCallbackCounters();
#endif
	}
}

int 
ReadInput(void)
{
	static char input[1024];
	char *s;
	int whitespaces;
	char cmd[256];
	Call *call = NULL;
	char alphanumeric = '1';	// dtmf alphanumeric
	char signalType[256];		// dtmf signal
	int duration = 500;			// dtmf signal duration
	int i, len;
    int volume;                 // in band dtmf volume
	char xferType[64];

	if (fgets(input, sizeof(input), stdin) == NULL)
	{
		return -1;
	}
	len = strlen(input);
	if (input[len-1]=='\n')
	  input[len-1]='\0';
	  
	s = input;

	if ((whitespaces = strspn(s, " ")) > 0)
	{
		s += whitespaces;
	}

   	if (strcmp(s, "exit") == 0)
	{
   		// Exit without tearing down calls
		PrintCallSummary();
        fprintf(trace_desc, "\nExiting without tearing down calls\n");
        fprintf(stdout, "\nExiting without tearing down calls\n");
		exit(0);
	}
	// SH - Quit command added to keep it synchronized with sgen.
   	else if ((strcmp(s, "stop") == 0) || (strcmp(s, "quit") == 0) )
   	{
		// Tear down calls before exiting
        fprintf(trace_desc, "\nTearing down calls and exiting\n");
        fprintf(stdout, "\nTearing down calls and exiting\n");
//      	sigsend(P_PID, P_MYID, SIGINT); replacing this call with POSIX kill
		kill(getpid(), SIGINT);
    }
    else if (strcmp(s, "help") == 0)
	{
		PrintInteractiveCmds();
	}
	else if (strncmp(s, "dtmfa", 5) == 0)
	{
		sscanf(s, "%s %c", cmd, &alphanumeric);
		if (mode & MODE_TRANSMIT)
		{
			for (i = 0; i < nCalls; i++)
			{
				call = &CallsOut[i];
				if (IsCallConnected(call) >= 0)
				{
					SendAlphaDtmf(call, &alphanumeric);
				}
			}
		}
		else
		{
			for (i = 0; i < nCalls; i++)
			{
				call = &Calls[i];
				if (IsCallConnected(call) >= 0)
				{
					SendAlphaDtmf(call, &alphanumeric);
				}
			}
		}
	}
	else if (strncmp(s, "dtmfs", 5) == 0)
	{
		signalType[0] = '1';	// default signalType
		sscanf(s, "%s %s %d", cmd, signalType, &duration);
		if (mode & MODE_TRANSMIT)
		{
			for (i = 0; i < nCalls; i++)
			{
				call = &CallsOut[i];
				if (IsCallConnected(call) >= 0)
				{
					SendSignalDtmf(call, signalType, duration);
				}
			}
		}
		else
		{
			for (i = 0; i < nCalls; i++)
			{
				call = &Calls[i];
				if (IsCallConnected(call) >= 0)
				{
					SendSignalDtmf(call, signalType, duration);
				}
			}
		}
	}
    else if (strncmp (s, "indtmf", 6) == 0)
	{
		duration = 200;	// seconds
        volume  = 30;
		signalType[0] = '1';	// default signalType
		sscanf(s, "%s %s %d %d", cmd, signalType, &duration, &volume);
        SendMgenDtmf(signalType, duration, volume);
	}
	else if (strcmp (s, "current-calls") == 0)
	{
		if (mode & MODE_TRANSMIT)
			fprintf(stdout, "%d calls connected.\n", CurrentCallsOut);
		else
			fprintf(stdout, "%d calls connected.\n", CurrentCalls);
	}

	else if (strcmp (s, "failed-fast-starts") == 0)
	{
		fprintf(stdout, "%d calls failed.\n", nFailedFaststarts);
	}

   	else if (strncmp(s, "mstat", 5) == 0)
	{
		if (mode & MODE_MGCP)
		{
            // Send a command to mgen to get its stats
			if(!mstatArg)
				mstatArg = calloc(5, sizeof(char));
			else
				memset(mstatArg, 0, 5);
			sscanf(s,"%s %s", cmd, mstatArg);
            mgenInform(NULL, 2);
			return 0;
		}
		else
		{
			PrintErrorTime();
			fprintf(stderr, "ReadInput() - ");
			fprintf(stderr, "Media generator has not been started\n");
		}
	}
	else if ((strcmp(s, "hold") == 0) || (strcmp (s, "resume") == 0))
	{
		if (mode & MODE_TRANSMIT)
                {
                        for (i = 0; i < nCalls; i++)
                        {
                                call = &CallsOut[i];
                                if (IsCallConnected(call) >= 0)
                                {
                                        // Ticket-35162: In case of resume, media ip & port is required
                                        if (strcmp (s, "resume") == 0)
                                           SendNewTCS(call, s, call->chIn[0].ip, call->chIn[0].port);
                                        else
                                           SendNewTCS(call, s, 0, 0);
                                }
                        }
                }
                else
                {
                        for (i = 0; i < nCalls; i++)
                        {
                                call = &Calls[i];
                                if (IsCallConnected(call) >= 0)
                                {
                                        // Ticket-35162: In case of resume, media ip & port is required
                                        if (strcmp (s, "resume") == 0)
                                           SendNewTCS(call, s, call->chIn[0].ip, call->chIn[0].port);
                                        else
                                           SendNewTCS(call, s, 0, 0);
                                }
                        }
                }


	}
	else if ((strncmp(s, "transfer", 8) == 0))
	{
		sscanf(s, "%s %s %s", cmd, xferType, xferTgtNum);
		xferMode = 1;
		if(strncmp (xferType, "unatt", 5) == 0)
		{
			// perform blind transfer
			if(xferType[5] == 'w')
			{
				waitToResume = 1;
			}
			SendXfer(BLIND_XFER);
		}
		else
		{	
			if (strcmp (xferType, "att") == 0)
			{
				// perform attended transfer
				SendXfer(ATT_XFER);
			}
			else
			{	
				if (strcmp(xferType, "final") == 0)
				{
					SendTCSForXfer(ATT_XFER);
				}
				else
				{
					if(strcmp(xferType, "abandon") == 0)
					{
						fprintf(stdout, "Sent abandon call transfer.\n");
						fprintf(stdout, ">\n");
						// resume the earlier call as the transfer has been abandoned
						startTransfer = 0;
                        SendNewTCS(&CallsOut[0], "resume", 0, 0);
					}
					else
					{
						fprintf(stdout, "\nTransfer Type not supported.\n" );
						fprintf(stdout, "\nTransfer operation not performed.\n" );
					}
				}
			}
		}
	}
	else 
	{
		PrintStatus(3);
	}

	fprintf(stdout, ">\n");

	return 0;
}

int
ResetCall(Call * call)
{
	int i = call->cNum;

	call->cNum = i;
	call->tsUse = call->tsOriginate = 0;
	call->tsIdle = call->tsSetup =  0;
	call->tsDrop = call->tsProceeding = 0;
	call->tsRingBack = call->tsConnect = 0;
	call->state = cmCallStateIdle;
	call->controlState = cmControlStateTransportDisconnected;
	call->fastStart = 0;
	call->chOut[0].state = cmChannelStateIdle;
	call->chOut[0].chNum = 0;
	call->chOut[0].hsChan = NULL;
	call->chOut[0].haCall = (HAPPCALL) call;
	call->chOut[0].handle = -1;
	call->chIn[0].state = cmChannelStateIdle;
	call->chIn[0].chNum = 0;
	call->chIn[0].hsChan = NULL;
	call->chIn[0].haCall = (HAPPCALL) call;
	call->chIn[0].handle = -1;
	call->hsCall=NULL;
}

char *
ULIPtostring(unsigned long ipaddress)
{
	static char outstring[16];

	sprintf(outstring, "%u.%u.%u.%u",
			(unsigned int) (ipaddress & 0xff000000) >> 24,
			(unsigned int) (ipaddress & 0x00ff0000) >> 16,
			(unsigned int) (ipaddress & 0x0000ff00) >> 8, 
			(unsigned int) (ipaddress & 0x000000ff));

	return outstring;
}

int
NetSetMaxFds( int32_t numfds )
{
	struct rlimit rl_data;

	getrlimit(RLIMIT_NOFILE, &rl_data);


	if ( numfds > ( rl_data.rlim_max - 1 ) )
	{
        PrintErrorTime();
        fprintf(stderr, "NetSetMaxFds(): ");
        fprintf(stderr, "File descriptors needed is greater than "
                "system defined max\nIncrease rlim_fd_max to at least "
                "%d in /etc/system...exiting\n", (numfds + 1));
		exit(1);
	}
	else if (numfds == 0)
	{
		// set the system to use the max fds available
		rl_data.rlim_cur = rl_data.rlim_max - 1;
	}
	else
	{
		rl_data.rlim_cur = numfds;
	}

	setrlimit(RLIMIT_NOFILE, &rl_data);

	memset(&rl_data, (int32_t) 0, sizeof(struct rlimit));

	getrlimit(RLIMIT_NOFILE, &rl_data);

    fprintf(stdout,
            "Maximum file descriptors set to %d\n",
            (uint32_t) rl_data.rlim_cur);

	return rl_data.rlim_cur;
}

/* 
//
//  Function    :
//      thread_set_rt()
//
//  Arguments   :
//      ns_quantum  time quantum value to be used when
//                  setting calling thread to RT
//                  Value is specified in nanoseconds.
//
//		rt_pri		real-time priority
//
//  Description :
//
//      This routine called by threads that
//      want to promote themselves to RT threads.
//      The priocntl() call is used to promote
//      threads. Currently they are promoted to
//      a middle of the road scheduling priority.
//
//  Return values:
//      None.
//
void
thread_set_rt( int32_t ns_quantum, int16_t priority )
{
	lwpid_t     lwpid = _lwp_self();
	pcparms_t   pcparms;
	rtparms_t   *rtparmsp = (struct rtparms *) pcparms.pc_clparms;
	rtinfo_t    *rtinfop = (struct rtinfo *) rt_pcinfo.pc_clinfo;

	get_priocntl_info();

	memset( &pcparms, (int32_t) 0, sizeof(pcparms_t) );

	pcparms.pc_cid = rt_pcinfo.pc_cid;

	if ( priority >= rtinfop->rt_maxpri )
	{
        PrintErrorTime();
        fprintf(stderr, "thread_set_rt(): ");
        fprintf(stderr, 
                "Realtime priority %d must be less than maximum rt priority %d\n",
				priority, rtinfop->rt_maxpri );
		exit(0);
	}

	rtparmsp->rt_pri = priority;

	rtparmsp->rt_tqsecs = 0;
	rtparmsp->rt_tqnsecs = ns_quantum;

    if ( priocntl(  (int32_t) P_LWPID, (idtype_t) lwpid,
                    (id_t) PC_SETPARMS, (caddr_t) &pcparms ) == -1L )
    {
        PrintErrorTime();
        fprintf(stderr, "thread_set_rt(): ");   
        fprintf(stderr,
                "Failed to set thread [%d, lwp %d] to real-time\n",
                pthread_self(), lwpid);
        return;
    }

    return;
}
*/

void*
pollArrayInit( int32_t poll_array_size )
{
	return( (void*) calloc(	poll_array_size, sizeof(struct pollfd)) );
}

int
daemonize (void)
{
	pid_t	pid;

	if ( (pid = fork()) < 0 )
	{
        perror ("daemonize() - fork error ");
		return -1;
	}
	else if (pid != 0)
		exit (0);	/* for the parent */

	/* Child chugs on... */
	setpgrp();

	setsid ();	/* become session leader */

	umask (0);	/* Clear file mode creation mask */

	return (0);
}

int
SetupConfigs(char *configfile)
{
	HCFG	hCfg = NULL;
	int 	maxCalls;
	int		value, isstr;
    int     i = 0;

	hCfg = ciConstructEx(configfile, 256, 256);

	if (hCfg == NULL)
	{
        PrintErrorTime();
        fprintf(stderr, "SetupConfigs(): ");
        fprintf(stderr, "Config file %s not found\n", configfile);

		return -1;
	}

	maxCalls = nCalls+2*call_rate*.01*nCalls+20;

    // SET in RAS
    if (!autoReg)
    {
        if (ciSetValue(hCfg, "RAS.manualRAS", 0, 0, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.manualRAS\n");
        }
        if (ciSetValue(hCfg, "RAS.allowCallsWhenNonReg", 0, 0, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.allowCallsWhenNonReg\n");
        }
    }

    if (gkMode)
    {
        if (ciSetValue(hCfg, "RAS.manualDiscovery", 0, 0, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.manualDiscovery\n");
        }
        if (ciSetValue(hCfg, "RAS.manualRAS", 0, 0, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.manualRAS\n");
        }
    }

	// SET in CONF
	if (ciSetValue(hCfg, "system.maxCalls", 0, maxCalls, NULL) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "SetupConfigs(): ");
        fprintf(stderr, "ciSetValue() failed for system.maxCalls\n");
	}

	if (ciSetValue(hCfg, "Q931.maxCalls", 0, maxCalls, NULL) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "SetupConfigs(): ");
        fprintf(stderr, "ciSetValue() failed for Q931.maxCalls\n");
	}
	
	/*
	if (ciSetValue(hCfg, "system.allocations.maxBuffSize", 0,4096, NULL) < 0)
	{
	       NETERROR(MINIT, ("ciSetValue failed for system.allocations.maxBuffSize"));
	}
	*/

	/* code added for h245 tunneling. */
	if (h245TunnFlag)
	{
		if (ciSetValue(hCfg, "Q931.h245Tunneling", 0, 0, NULL) < 0)
		{
        	PrintErrorTime();
        	fprintf(stderr, "SetupConfigs(): ");
        	fprintf(stderr, "ciSetValue() failed for Q931.h245Tunneling\n");
        	}
	}


	if (tdSetup)
	{
		if (ciSetValue(hCfg, "Q931.responseTimeOut", 0, 
			tdSetup, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for Q931.responseTimeout\n");
		}

		if (ciSetValue(hCfg, "Q931.connectTimeOut", 0, 
			tdSetup, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for Q931.connectTimeout\n");
		}
	}

    if (localIpFlag)
    {
		sprintf(localIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&localIp)[0],
                (int) ((unsigned char*)&localIp)[1],
                (int) ((unsigned char*)&localIp)[2],
                (int) ((unsigned char*)&localIp)[3]);
        if (ciSetValue(hCfg, "system.localIPAddress", 1, 4, localIpStr) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for system.localIPAddress\n");
        }
    }

    if (localPortFlag)
    {
        if (ciSetValue(hCfg, "RAS.rasPort", 0, localRasPort, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.rasPort\n");
        }
        if (ciSetValue(hCfg, "Q931.callSignalingPort", 0, localQ931Port, NULL) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.rasPort\n");
        }
    }

    if (gwAddrFlag)
    {
		sprintf(gwIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&gwAddrLong)[0],
                (int) ((unsigned char*)&gwAddrLong)[1],
                (int) ((unsigned char*)&gwAddrLong)[2],
                (int) ((unsigned char*)&gwAddrLong)[3]);
        if (ciSetValue(hCfg, "RAS.manualDiscovery.defaultGatekeeper.ipAddress.ip", 1, 4, gwIpStr) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.manualDiscovery.defaultGatekeeper.ipAddress.ip\n");
        }
    }

    if (h323AliasCmdLine)
    {
        BYTE *bmpStr;
        bmpStr = malloc(sizeof(BYTE)*256);
        i = utlChr2Bmp(h323Id, (BYTE *)bmpStr);
        ciSetValue(hCfg, "RAS.registrationInfo.terminalAlias", 0, 0, NULL);
        if (ciSetValue(hCfg, "RAS.registrationInfo.terminalAlias.*.h323-ID", 1, i, bmpStr) < 0)
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.registrationInfo.terminalAlias.*.h323-ID\n");
            fprintf(stdout, "ciSetValue() failed for RAS.registrationInfo.terminalAlias.*.h323-ID\n");
        }
        free(bmpStr);
    }
    if (e164AliasCmdLine)
    {
        int ret = 0;
        if (!h323AliasCmdLine)
            ret = ciSetValue(hCfg, "RAS.registrationInfo.terminalAlias.*.e164", 1, strlen(e164Id), e164Id);
        else
            ret = ciSetValue(hCfg, "RAS.registrationInfo.terminalAlias..e164", 1, strlen(e164Id), e164Id);
        if (ret < 0) 
        {
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for RAS.registrationInfo.terminalAlias..e164\n");
        }
    }

	// Based on any error scenarios we are trying to generate,
	// set up the config file
	switch(errorScenario)
	{
	case Scene_eDiscAfterARQ:
	case Scene_eDiscAfterSetup:
		if (ciSetValue(hCfg, "Q931.manualCallProceeding", 0, 0, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr,
                    "ciSetValue() failed for Q931.manualCallProceeding\n");
		}

		if (ciSetValue(hCfg, "Q931.manualAccept", 0, 0, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for Q931.manualAccept\n");
		}

		break;
	case Scene_eDiscAfterProceeding:
		if (ciGetValue(hCfg, "Q931.manualCallProceeding", NULL, &value) >= 0)
		{
			if (ciDeleteValue(hCfg, "Q931.manualCallProceeding") < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "SetupConfigs(): ");
                fprintf(stderr,
                        "ciDeleteValue() failed for Q931.manualCallProceeding\n");
			}
		}

		if (ciSetValue(hCfg, "Q931.manualAccept", 0, 0, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for Q931.manualAccept\n");
		}

		break;
	case Scene_eDiscAfterAlerting:
		if (ciGetValue(hCfg, "Q931.manualCallProceeding", NULL, &value) >= 0)
		{
			if (ciDeleteValue(hCfg, "Q931.manualCallProceeding") < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "SetupConfigs(): ");
                fprintf(stderr,
                        "ciDeleteValue() failed for Q931.manualCallProceeding\n");
			}
		}

		if (ciGetValue(hCfg, "Q931.manualAccept", NULL, &value) >= 0)
		{
			if (ciDeleteValue(hCfg, "Q931.manualAccept") < 0)
			{
                PrintErrorTime();
                fprintf(stderr, "SetupConfigs(): ");
                fprintf(stderr, "ciDeleteValue() failed for Q931.manualAccept\n");
			}
		}

		break;
	case Scene_eNoMSD:
	case Scene_eNoTCS:
		if (ciSetValue(hCfg, "h245.masterSlave.manualOperation", 0, 0, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for h245.manualOperation\n");
		}

		break;
	case Scene_eNoMSDAck:
	case Scene_eNoTCSAck:
		if (ciSetValue(hCfg, "h245.masterSlave.manualResponse", 0, 0, NULL) < 0)
		{
            PrintErrorTime();
            fprintf(stderr, "SetupConfigs(): ");
            fprintf(stderr, "ciSetValue() failed for h245.manualOperation\n");
		}

		break;
	default:
		break;
	}

	if (ciSave(hCfg, config_file) < 0)
	{
        PrintErrorTime();
        fprintf(stderr, "SetupConfigs(): ");
        fprintf(stderr, "ciSave() failed for config file %s\n", config_file);

		goto _error;
	}

	ciDestruct(hCfg);

	return 1;

_error:
	return -1;
}

char *
ErrorScenarios(Scene_eError no)
{
	switch (no)
	{
	case Scene_eDiscAfterARQ:
		return "Disconnect After ARQ";
		break;
	case Scene_eDiscAfterSetup:
		return "Disconnect After Setup";
		break;
	case Scene_eDiscAfterProceeding:
		return "Disconnect After Proceeding";
		break;
	case Scene_eDiscAfterAlerting:
		return "Disconnect After Alerting";
		break;
	case Scene_eNoMSD:
		return "No MSD";
		break;
	case Scene_eNoMSDAck:
		return "No MSD Ack";
		break;
	case Scene_eNoTCS:
		return "No TCS";
		break;
	case Scene_eNoTCSAck:
		return "No TCS Ack";
		break;
	case Scene_eBadH245Address:
		return "Bad H245 Address";
		break;
	case Scene_eNoAutoAnswer:
		return "No Auto Answer";
		break;
	default:
		return "none";
		break;
	}
}

int
PrintErrorScenarios()
{
	int i;

	fprintf(stdout, "Error Scenarios:\n");
	for (i=0; i<Scene_eNone; i++)
	{
           fprintf(stdout, "    %d : %s\n", i, ErrorScenarios(i));
	}
}

int
ErrorScenariosInit(void)
{
	struct sockaddr_in myaddr;

	if (errorScenario == Scene_eBadH245Address)
	{
		int fd;

		fd = socket (AF_INET, SOCK_STREAM, 0 );

		if (fd < 0)
		{
            perror ("ErrorScenariosInit() - unable to create stream socket ");
			return -1;
		}

		memset(&myaddr, 0, sizeof(myaddr)); 

		myaddr.sin_family = AF_INET;
		myaddr.sin_port  = htons (12345);
		myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

		bind (fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
		listen (fd, 100);
	}
}

// Prints timestamp for errors printed to stderr
static void 
PrintErrorTime()
{
    static time_t errorTime;         
    static char errorTimeStr[120];      

    time(&errorTime);
	// cftime(errorTimeStr, "%Y/%m/%d %T", &errorTime); replacing this with POSIX strftime
    strftime(errorTimeStr, sizeof(errorTimeStr), "%Y/%m/%d %T", localtime(&errorTime));
    fprintf(stderr, "[%s] ", errorTimeStr);    
}

static void
PrintCallSummary()
{
    if (mode & MODE_TRANSMIT)
    {
        if (automode)
        {
            fprintf(stdout, "\nCurrentCallsConnected : %d\n", CurrentCallsOut);
            fprintf(stdout, "TotalCallsPlaced    : %d\n", TotalCallsOut);
        }
        else
        {
            fprintf(stdout,
                    "\nTotal Calls : %d/%d (connected/total_placed)\n",
                    CurrentCallsOut, TotalCallsOut);
        }
    }
	else
    {
        if (automode)
        {
            fprintf(stdout, "\nCurrentCallsConnected : %d\n", CurrentCalls);
            fprintf(stdout, "TotalCallsReceived  : %d\n", TotalCalls);
        }
        else
        {
            fprintf(stdout,
                    "\nTotal Calls : %d/%d (connected/received)\n",
                    CurrentCalls, TotalCalls);
        } 
    }
}

static void
PrintMgenOptions()
{
	fprintf(stdout, "\nOptions to pass to mgen if it is started from gen:\n" );
	fprintf(stdout, "  [-B] - send and receive (loopback)\n");
	fprintf(stdout, "  [-R] - send and receive on same port\n");
	fprintf(stdout, "  [-Q <no. of media threads>] - default 1\n");
	fprintf(stdout, "  [-P <payload length (bytes)> <payload interval (nanosec)>] - default 30, 30000000\n");
}

static void
PrintInteractiveCmds()
{
	fprintf(stdout, "Interactive commands:\n" );
	fprintf(stdout, "  dtmfs [signal - default 1] [duration - default 500ms] : Send an out-of-band signal DTMF\n");
	fprintf(stdout, "  dtmfa [signal - default 1] : Send an out-of-band alphanumeric DTMF\n");
	fprintf(stdout, "  indtmf [signal - default 1] [duration - default 200ms] [volume - default -30dBm] : Send an in-band DTMF according to RFC 2833\n");
	fprintf(stdout, "  mstat [Rx/Tx] : If media gen is started from gen, display its statistics\n");
	fprintf(stdout, "  current-calls : Return the number of current connected calls.\n");
	fprintf(stdout, "  failed-fast-starts : Return the number of failed fast start calls.\n");
	fprintf(stdout, "  stop : Attempt to tear down calls and exit\n");
	fprintf(stdout, "  exit : Exit without tearing down calls\n");
	fprintf(stdout, "  help : Print interactive commands\n");
	fprintf(stdout, "  hold : Send a hold.\n");
	fprintf(stdout, "  resume : Send a command to release the hold.\n");
	fprintf(stdout, "  transfer unatt <num>  : Perform an unattended(blind) transfer to <num>.\n");
	fprintf(stdout, "  transfer unattw <num>  : Perform an unattended(blind) transfer to <num> and wait to resume if xfer fails.\n");
	fprintf(stdout, "  transfer att <num>  : Perform an attended transfer to <num>.\n");
	fprintf(stdout, "  transfer final <num>  : Complete an attended transfer to <num>.\n");
	fprintf(stdout, "  transfer abandon <num>  : Abandon an attended transfer to <num>.\n");
}

static int
SendNewTCS (Call *call, char *input, int media_ip, int media_port)
{

	time_t start = 0, end = 0, delta = 0;
        int nodeId = -1;
	char fn[] = "SendNewTCS()";

	HPST                    hSyn = cmGetSynTreeByRootName(hApp,"capData");
        HPVT                    hVal = cmGetValTree(hApp);
        int                     strbuffer;
        HPVT                    nullhVal;
        static int              capNodeId = 0;
        INT32 h245Conf;


	time(&start);   // Start time


	// Build the TCS	
	
	if(strcmp (input, "hold") == 0)	
	{

                cmMeiEnter(hApp);

                capNodeId = pvtAddRoot(hVal,
                                        hSyn,
                                        0,
                                        NULL);

        	h245Conf = cmGetH245ConfigurationHandle(hApp);

 	       nodeId = pvtGetNodeIdByPath(hVal, h245Conf,
                                ".capabilities.terminalCapabilitySet");

                if (nodeId < 0)
                {
                                fprintf(stderr,"%s Could not obtain Local capabilities\n", fn);
                                cmMeiExit(hApp);
                                return -1;
                }


                pvtSetTree(hVal,capNodeId,hVal,nodeId);
		
		// delete multiplexCapability

                if((nodeId = pvtGetNodeIdByPath(hVal,capNodeId,".multiplexCapability")) <0)
                {
                                fprintf (stderr, "%s Could not obtain capNodeId .multiplexCapability= %d \n", fn,nodeId);
                }

		else if(pvtDelete(hVal,nodeId) <0)
		{
                        fprintf (stderr, "%s Could not do addsubtree\n", fn);
                        cmMeiExit(hApp);
                        return -1;
                }

		// delete capability table

		if((nodeId = pvtGetNodeIdByPath(hVal,capNodeId,".capabilityTable")) <0)
                {
                                fprintf (stderr, "%s Could not obtain capNodeId .capabilityTable= %d \n", fn,nodeId);
                }

                else if(pvtDelete(hVal,nodeId) <0)
                {
                        fprintf (stderr, "%s Could not do addsubtree\n", fn);
                        cmMeiExit(hApp);
                        return -1;
                }

		// delete capability descriptor.

		if((nodeId = pvtGetNodeIdByPath(hVal,capNodeId,".capabilityDescriptors")) <0)
                {
                                fprintf (stderr, "%s Could not obtain capNodeId .capabilityDescriptors= %d \n", fn,nodeId);
                }

                else if(pvtDelete(hVal,nodeId) <0)
                {
                        fprintf (stderr, "%s Could not do addsubtree\n", fn);
                        cmMeiExit(hApp);
                        return -1;
                }

                cmMeiExit(hApp);

        	if( (cmCallSendCapability(call->hsCall,capNodeId)< 0) )
        	{
                	fprintf(stderr, "%s %x cmCallCapabilitiesSend failed\n",fn,call->hsCall);
                	return -1;
        	}

		// Close the current channel
		if (cmChannelDrop(call->chOut[0].hsChan) < 0)
		{
            		fprintf(stderr, "cmChannelDrop() failed\n");
			return -1;
                }
		nChannelsOut --;

		mgenInform(call, 0);

		fprintf(stdout, "Sent Hold for call %d\n", call->cNum);

	}
	else if(strcmp (input, "resume") == 0)
	{
		// Send a TCS to resume the call.

                cmMeiEnter(hApp);

                capNodeId = pvtAddRoot(hVal,
                                        hSyn,
                                        0,
                                        NULL);

        	h245Conf = cmGetH245ConfigurationHandle(hApp);

 	       nodeId = pvtGetNodeIdByPath(hVal, h245Conf,
                                ".capabilities.terminalCapabilitySet");

                if (nodeId < 0)
                {
                                fprintf(stderr,"%s Could not obtain Local capabilities\n", fn);
                                cmMeiExit(hApp);
                                return -1;
                }


                pvtSetTree(hVal,capNodeId,hVal,nodeId);
		
                cmMeiExit(hApp);

        	if( (cmCallSendCapability(call->hsCall,capNodeId)< 0) )
        	{
                	fprintf(stderr, "%s %x cmCallCapabilitiesSend failed\n",fn,call->hsCall);
                	return -1;
        	}

		// Send OLC
		int sNum = 0;

						if (cmChannelNew(call->hsCall,
                                 (HAPPCHAN) & (call->chOut[sNum]),
                                 &call->chOut[sNum].hsChan) < 0)
                        {
                		PrintErrorTime();
                		fprintf(stderr, "cmChannelNew() failed for call # %d\n",
                        	call->cNum );
                        }
						sleep(2);

                                // Ticket-35162: In case of resume send media ip and port of the source ep
                                if(media_ip && media_port)
                                {
                                  if (call->chIn[sNum].hsChan)
                                    {
                                      if (cmChannelSetRTCPAddress(call->chIn[sNum].hsChan,
                                          htonl(media_ip),
                                          media_port + 1) < 0)
                                      {
                                        PrintErrorTime();
                                        fprintf(stderr,
                                        "cmChannelSetRTCPAddress() failed for call # %d\n",
                                        call->cNum );
                                      }
                                    }
                                  else
                                  {
                                    if (cmChannelSetRTCPAddress(call->chOut[sNum].hsChan,
                                        htonl(media_ip),
                                        media_port + 1) < 0)
                                     {
                                         PrintErrorTime();
                                         fprintf(stderr,
                                         "cmChannelSetRTCPAddress() failed for call # %d\n",
                                          call->cNum );
                                     }
                                   }
                                }
                                else
                                {

                                if (cmChannelSetRTCPAddress(    call->chIn[sNum].hsChan,
                                htonl(call->chIn[sNum].ip),
                                call->chIn[sNum].port + 1) < 0)
                                {
                                                PrintErrorTime();
                                                fprintf(stderr,
                                                "cmChannelSetRTCPAddress() failed for call # %d\n",
                                                call->cNum );
                                }
                                }

                        if (cmChannelOpen(      call->chOut[sNum].hsChan,
                                                                "g711Ulaw64k",
                                                                NULL,
                                                                NULL,
                                                                0) < 0)
                        {
                		PrintErrorTime();
                		fprintf(stderr, "cmChannelOpen() failed for call # %d\n",
                        	call->cNum );
                        }

                        nChannelsOut++;

						fprintf(stdout, "Sent Resume for call %d\n", call->cNum);
	}
}

static int
SendFacilityForXfer()
{
	HPVT hVal = (HPVT)NULL;
	int facnodeId = -1, oldMessage = -1, newMessage = -1, nodeId = -1;
	int index = 0;
	
    hVal = cmGetValTree(hApp);

	if ((newMessage = pvtAddRoot(hVal, NULL, 0, NULL))<0)
	{
    	PrintErrorTime();
		fprintf(stderr,"SendFacility(): failed to add root node\n");
	}
    /* Get the message to send */
    if ((oldMessage = callGetMessage(CallsOut[0].hsCall,cmQ931facility)) < 0)
	{
    	PrintErrorTime();
		fprintf(stderr,"SendFacility(): failed to get property node id.\n");
	}

	if ((facnodeId= pvtGetNodeIdByPath(hVal, oldMessage,
	"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to build facility message.\n");
	}

	//fprintf(stdout, "fac:%d, message: %d, newmess:%d", facnodeId, message, newMessage);

	pvtSetTree(hVal, newMessage, hVal, oldMessage);

	if ((facnodeId= pvtGetNodeIdByPath(hVal, newMessage,
	"message.facility.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.facility")) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to build facility new message.\n");
	}
	//fprintf(stdout, "fac:%d, message: %d, newmess:%d", facnodeId, message, newMessage);

	if ((pvtBuildByPath(hVal, facnodeId,	"reason.callForwarded", 0, NULL)) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to set reason.\n");
	}

	if ((nodeId = pvtBuildByPath(hVal, facnodeId,"alternativeAliasAddress", 0, NULL)) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to set alternativeAliasAddress.\n");
	}

	if ((nodeId = pvtAdd(hVal, nodeId, -1, 0, NULL, &index)) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtAdd Failed.\n");
	}

	if ((pvtBuildByPath(hVal, nodeId,"e164", strlen(xferTgtNum), xferTgtNum)) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to set e164.\n");
	}
	if ((pvtBuildByPath(hVal, facnodeId,	"callIdentifier.guid", 0, NULL)) < 0)
	{
    	PrintErrorTime();
        fprintf(stderr, "SendFacility(): ");
        fprintf(stderr,	"pvtBuildByPath() failed to set callidentifier.\n");
	}
	cmCallFacility(CallsOut[0].hsCall, newMessage);
}

static int
SendXfer(int xferType)
{

	//send null tcs to b
    SendNewTCS(&CallsOut[0], "hold", 0, 0);
	//spawn call for c
	SpawnOutgoingCall(1);
	sleep(2);
	if (xferType == BLIND_XFER)
	{
		if(!nRelComps)
		{
			sleep(3);
			SendTCSForXfer(BLIND_XFER);
		}
		else
		{
			if(waitToResume)
			{	
        		SendNewTCS(&CallsOut[0], "resume", 0, 0);
			}
			else
			{
				HangupCalls();
			}
		}
	}
        return 1;
}

static int	
SendTCSForXfer(int xferType)
{
	startTransfer = 1;
	//send null tcs to c
    SendNewTCS(&CallsOut[1], "hold", 0,0);
	//send tcs to c with media of b
	sleep(2);
    startTransfer = 0;
    // Ticket-32961: Send the media IP and port of the src ep
    SendNewTCS(&CallsOut[0], "resume", CallsOut[1].chIn[0].ip, CallsOut[1].chIn[0].port);
	//send tcs to b with media of c
	sleep(2);
    SendNewTCS(&CallsOut[1], "resume", CallsOut[0].chIn[0].ip, CallsOut[0].chIn[0].port);
}
static int
SendSignalDtmf(Call *call, char *signalType, int duration)
{
	time_t start = 0, end = 0, delta = 0;
	int nodeId = -1;
	cmUserInputSignalStruct uis;

	time(&start);	// Start time

	// Build and send signal DTMF in H.245 user input message

	memset (&uis, 0, sizeof(uis));
	uis.signalType = signalType[0];
	uis.duration = 4000;

	if ((nodeId = cmUserInputSignalBuild(hApp, &uis)) < 0)
	{
		PrintErrorTime();
		fprintf(stderr, "SendSignalDtmf(): ");
       	fprintf(stderr, "cmUserInputSignalBuild() failed - "
                       	"error building signal DTMF\n");
		return -1;
	}		

	if (SendUserInput(call->hsCall, nodeId) < 0)
	{
		return -1;
	}

	fprintf (stdout, "Sent signal DTMF for call %d : '%c', %dms\n", 
			 call->cNum, uis.signalType, uis.duration);

	time(&end);		// End time

	// Wait to send signal DTMF update
	delta = duration - (end - start);
	if (delta > 0)
	{
		millisleep(delta);
	}

	// Build and send signal DTMF update in H.245 user input message

	memset (&uis, 0, sizeof(uis));
	uis.signalType = signalType[0];
	uis.duration = duration;

	if ((nodeId = cmUserInputSignalUpdateBuild(hApp, &uis)) < 0)
	{
		PrintErrorTime();
		fprintf(stderr, "SendSignalDtmf(): ");
       	fprintf(stderr, "cmUserInputSignalUpdateBuild() failed - "
                       	"error building signal DTMF update\n");
		return -1;
	}		

	if (SendUserInput(call->hsCall, nodeId) < 0)
	{
		return -1;
	}

	fprintf (stdout, "Sent signal DTMF update for call %d : %dms\n", 
			call->cNum, uis.duration);

	return 0;
}

static int
SendAlphaDtmf(Call *call, char *alphanumeric)
{
	int nodeId = -1;
	cmUserInputData userData = {alphanumeric, sizeof(char)};

	// Build and send alphanumeric DTMF in H.245 user input message

	if ((nodeId = cmUserInputBuildAlphanumeric(hApp, &userData)) < 0)
	{
		PrintErrorTime();
		fprintf(stderr, "SendAlphaDtmf(): ");
        fprintf(stderr, "cmUserInputSignalBuild() failed - "
                        "error building alphanumeric DTMF\n");
		return -1;
	}		

	if (SendUserInput(call->hsCall, nodeId) < 0)
	{
		return -1;
	}

	fprintf (stdout, "Sent alphanumeric DTMF for call %d : '%c'\n", 
			call->cNum, alphanumeric[0]);

	return 0;
}

static int
SendUserInput(HCALL hsCall, int nodeId)
{
	if (cmCallSendUserInput(hsCall, nodeId) < 0)
	{
       	PrintErrorTime();
		fprintf(stderr, "SendUserInput(): ");
      	fprintf(stderr, "cmCallSendUserInput() failed - "
                        "error sending user input\n");
		return -1;
	}

	return 0;
}

static int
IsCallConnected(Call *call)
{
	if (call && call->hsCall && 
		(call->state == cmCallStateConnected))
	{
		if (call->controlState == cmControlStateConnected)
		{
			return 0;
		}
		else
		{
			PrintErrorTime();
			fprintf(stderr, "IsCallConnected(): Control state not connected\n");
			return -1;
		}
	}

	PrintErrorTime();
	fprintf(stderr, "IsCallConnected(): Call state not connected\n");
	return -1;
}

static pid_t
StartMgen()
{
	pid_t	pid;
	char nCallsStr[16];
	char mgenPortStr[8];
	char mediaStartPortStr[8];
	char chanLocalAddrStr[16];

	pid = fork();		// Fork a child process

	if (pid == 0)		// In child process
	{
		// Get the arguments to be passed to mgen
		snprintf(nCallsStr, 16, "%d", nCalls);
		snprintf(mgenPortStr, 8, "%d", mgenPort);
		snprintf(mediaStartPortStr, 8, "%d", mediaStartPort);
		nx_strlcpy(chanLocalAddrStr, ULIPtostring(chanLocalAddr), 16);

		// Redirect or close necessary file descriptors
		freopen("./mgenout.log", "a", stdout);
		freopen("./mgenerr.log", "a", stderr);
		close(0);							// Close stdin

		// Start mgen
		execlp(mgen_file, mgen_file, "-T", "-z", "-n", 
			  nCallsStr, "-L", chanLocalAddrStr, "-c", chanLocalAddrStr, 
			  "-m", mediaStartPortStr, "-M", mgenPortStr, 
			  "-b", mgenPayloadLen, mgenPayloadInterval,
			  "-x", mgenNumThreads, mgenTxRxSamePort, mgenLoopback, NULL);

		perror("StartMgen() - exec error ");
		fprintf(stderr, "Media generator process exiting\n");
		exit(-1);
	}
	else if (pid < 0)	// Error
	{
        perror ("StartMgen() - fork error ");
		fprintf(stderr, "Exiting\n");
		exit(-1);
	}
	
	return pid;
}

// Get the H323-ID from config file
static int
GetH323IDFromConfig()
{
	char line[1024];
	FILE *fd = NULL;
	char *pattern = "h323-ID = ";
	char *pos = NULL;
	int len = 0, retval = 0;

	if ((fd = fopen(config_file, "r")) != NULL) 
	{
		while(fgets(line, 1024, fd) != NULL)
		{
			// Search for h323-ID (remove surrounding quotes)
			if ((pos = strstr(line, pattern)) != NULL)
			{
				if ((len = strcspn(pos + strlen(pattern) + 1, "\"")) > 0)
				{
					nx_strlcpy(h323Id, pos + strlen(pattern) + 1, len + 1);	
				}
				break;
			}
		}
		fclose(fd);
	}
	else
	{
		retval = -1;
	}

	return retval;
}

int getAppIp( void *p)
{
	return ntohl(localIp);
}

static int
PrintMgenStats()
{
	Call *call;
	if((mode & MODE_TRANSMIT) && CallsOut)
	{
		call = CallsOut;
	}
	else if((mode & MODE_RECEIVE) && Calls)
	{
		call = Calls;
	}
	/* if receiver media stat is requested */
	if(call && mstatArg && (strncmp(mstatArg, "Rx", 2)==0))
	{
 	/* Check whether media is beig received from the IP specified in SDP. Though the Ip specified in the
                 * connection line of SDP(in INVITE/200 OK) is the IP where the  proxy/destination UA will be receiving
                 * media,  But there is no way to know at signaling
                 * level where the proxy/other UA will be sending media out from. So, here we are making a big
                 * assumption(always true in case of Nextone MSW, if using NSF) that Media firewall will use same IP
				 * for sending and 
                 * receiving media to/from UA. This is true most of the time, but we need to check this in case of
                 * Brookrou/Snowshore firewall  */

		if(call[0].chOut[0].ip == mgenStats.receivingFromIp)
		{
			fprintf(stdout,"%lld bytes from %s\n",mgenStats.rxBytes, ULIPtostring(mgenStats.receivingFromIp));
		}
		else
		{
			fprintf(stdout,"0 bytes from %s; %lld from %s\n",ULIPtostring(call[0].chOut[0].ip),
				   	mgenStats.rxBytes, ULIPtostring(mgenStats.receivingFromIp));
		}

		goto _return;
	}	
	/* if sender media stat is requested */
	if(mstatArg && (strncmp(mstatArg, "Tx", 2)==0))
	{
		fprintf(stdout,"%lld bytes to %s\n",mgenStats.txBytes, ULIPtostring(mgenStats.sentToIp));
		goto _return;
	}	

	fprintf(stdout, 
			  "TxBytes            :    %lld"
			"\nTxPkts             :    %lld"
			"\nTxBitRate(Kbits/s) :    %f"
			"\nTxPktRate(Kpkts/s) :    %f"
			"\nRxBytes            :    %lld"
			"\nRxPkts             :    %lld"
			"\nRxBitRate(Kbits/s) :    %f"
			"\nRxPktRate(Kpkts/s) :    %f"
   		    "\n",
			mgenStats.txBytes, mgenStats.txPkts,
			mgenStats.txBitRate, mgenStats.txPktRate,
			mgenStats.rxBytes, mgenStats.rxPkts,
			mgenStats.rxBitRate, mgenStats.rxPktRate);
_return:
	fprintf(stdout,">\n");
	return 0;
}

int
SendMgenDtmf(char *signalType, int duration, int volume)
{
    DtmfInfo info;
    DtmfInfo rinfo;

	if (!(mode & MODE_MGCP))
	{
		return -1;
	}

	if (mgenFd <= 0)
	{
		return -1;
	}

	// Send a command to mgen to get its stats
	mgenInform(NULL, dtmf);

    info.digit = signalType[0];
    info.duration = htonl(duration);
    info.volume = htonl(volume);

	if (write(mgenFd, (char *)&info, sizeof(DtmfInfo)) < 0)
	{
       	perror("SendMgenDtmf() - no connection with mgen, write error ");
		return -1;
	}
    fprintf(stdout, "Sent signal Inband DTMF: '%s', %dms, -%ddBm0\n",
            signalType, duration, volume);

	return 0;
}

static INT16* getParamPath(void)
{
    RAS_RETURN_PATH({_q931(alternateEndpoints) LAST_TOKEN});
}

static INT16* getParamPathServiceControl(void)
{
    RAS_RETURN_PATH({_q931(serviceControl) LAST_TOKEN});
}

int
genHandleLRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
	 char fn[] = "genHandleLRQ():";
	 int rc = -cmRASReasonRequestDenied, callError = 0;
	 INT32 tlen;
	 cmTransportAddress 
		  dstSignalAddr = { 0, 0, 1720, cmRASTransportTypeIP },
		  dstRASAddr = { 0, 0, 1719, cmRASTransportTypeIP },
		  rasaddr;
	 int addrlen = sizeof(cmTransportAddress);
	cmAlias alias;
	BOOL canMapAlias, isstring;
	int nodeId, altNodeId;
	HPVT hVal = cmGetValTree(hApp);
    HPST hPst = cmGetSynTreeByRootName(hApp, "ras");
	int domainIp,sd,realmId;
	unsigned short domainPort;
    int index, fieldId, temp;
    int rootNodeId, parentId;
    int tmpNodeId, addrNodeId, tempNodeId;
    FILE *fp_dump = NULL;
    INT16   fieldPath[] = {_nul(0) LAST_TOKEN};
    unsigned long       altEpIp;
    char                altEpIpStr[5];
    int i;
    int ep_count = -1, aliasType, aliasLen;
    BYTE bmpStr[256];

    read_ep_list_cfg_file(&ep_count);

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
					  cmRASParamSocketDesc, 0, 
					  (int *)&sd, NULL) < 0)
	{
		 fprintf(stdout, "%s No socket\n", fn);
	}

	cmMeiEnter(hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	if (pvtGetByPath(hVal, nodeId,
		"request.locationRequest.canMapAlias", NULL, 
			&canMapAlias, &isstring) < 0)
	{
		fprintf(stderr, ("%s could not build MapAlias\n", fn));
	}

	cmMeiExit(hApp);

 _reply:
    dstSignalAddr.ip = inet_addr(ep_list[0].callSignalIp);
	dstSignalAddr.port = ep_list[0].callSignalPort;

	 /* Set the Call Signal Address */
	 if (cmRASSetParam(hsRas,
					   cmRASTrStageConfirm,
					   cmRASParamCallSignalAddress, 0, 
					   addrlen, (char *)&dstSignalAddr) < 0)
	 {
         fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed\n", 
                 fn);
	 }

	/* Set the RAS Endpoint address */
	dstRASAddr.ip = inet_addr(ep_list[0].rasIp);
	dstRASAddr.port = ep_list[0].rasPort;

	if (cmRASSetParam(hsRas,
					   cmRASTrStageConfirm,
					   cmRASParamRASAddress, 0, 
					   addrlen, (char *)&dstRASAddr) < 0)
	{
		fprintf(stdout, "%s cmRASSetParam cmRASParamRASAddress failed\n", fn);
	}

	alias.type = getAliasType(ep_list[0].aliasType, 0);
    if (alias.type == cmAliasTypeH323ID)
    {
        i = utlChr2Bmp(ep_list[0].alias, (BYTE *)bmpStr);
        alias.string = bmpStr;
        alias.length = i;
    }
    else
    {
        alias.string = ep_list[0].alias;
        alias.length = strlen(alias.string);
    }

	if (cmRASSetParam(hsRas, cmRASTrStageConfirm,
			cmRASParamDestInfo, 0, sizeof(cmAlias), 
			(char *)&alias) < 0)
	{
		fprintf(stdout, "cmRASSetParam cmRASParamDestInfo failed\n");
	}

	cmMeiEnter(hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

    // Now add the alternateEndpoints
    //
    if (ep_count > 0) 
    {
        if ((altNodeId = pvtGetNodeIdByPath(hVal, nodeId,
                        "response.locationConfirm")) < 0)
        {
            fprintf(stdout, "No alternateEndpoints\n");
        }
        hPst = cmGetSynTreeByRootName(hApp, "ras");
        if (hPst == NULL)
            fprintf(stdout, "No HPST\n"); 
        
        if ((parentId = pvtBuildByFieldIds(hVal, altNodeId, getParamPath(), 1, NULL)) < 0)
        {
            fprintf(stdout, "Failed to add alternateEndpoints\n");
        }
        rootNodeId = parentId;
        pvtBuildByPath(hVal, rootNodeId, "alternateEndpoints.priority", 
	                   ep_list[1].priority, NULL);
        if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                                "alternateEndpoints.callSignalAddress", 
					1, NULL)) < 0)
        {
            fprintf(stdout, "Could not add callSignalAddress to altEP\n");
        }

        if ((parentId = pvtAdd(hVal, tmpNodeId, 
	                __q931(callSignalAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add callSignalAddress in list\n");
        }

        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[1].callSignalIp);
	sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[1].callSignalPort, NULL);

        // Add rasAddress now
	//
        if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                              "alternateEndpoints.rasAddress", 1, NULL)) < 0)
        {
            fprintf(stdout, "Could not add rasAddress to altEP 1\n");
        }

        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(rasAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add rasAddress in list\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[1].rasIp);
	sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[1].rasPort, NULL);

        // Add Alias address now
	//
        if ((aliasType = getAliasType(ep_list[1].aliasType, 1)) != -1)
        {
            if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                         "alternateEndpoints.aliasAddress", 1, NULL)) < 0)
	    {
                fprintf(stdout, "Could not add alias to altEP 1\n");
	    }

            if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(aliasAddress), 
	                                 -1, NULL,&temp)) < 0)
            {
                fprintf(stdout, "Failed to add alias in list\n");
            }
            switch (aliasType)
            {
                case cmAliasTypeH323ID:
                    aliasLen = utlChr2Bmp(ep_list[1].alias, (BYTE *)bmpStr);
                    if (pvtBuildByPath(hVal, parentId, "h323-ID", aliasLen, bmpStr) < 0)
                        fprintf(stdout, "Could not add h323-ID in alias\n");
                    break;

                case cmAliasTypeE164:
                    if (pvtBuildByPath(hVal, parentId, "e164", 
		                       strlen(ep_list[1].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add dialedDigits in alias\n");
                    break;

                case cmAliasTypeURLID:
                    if (pvtBuildByPath(hVal, parentId, "url-ID", 
		                       strlen(ep_list[1].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add url-ID in alias\n");
                    break;
            }
        }
    }
    //

    for (i = 2; i <= ep_count; i++)
    {
        if ((parentId = pvtBuildByFieldIds(hVal, rootNodeId, fieldPath, 1, NULL)) < 0)
        {
            fprintf(stdout, "Failed to add alternateEndpoints\n");
        }
        tempNodeId = parentId;
        pvtBuildByPath(hVal, parentId, "priority", ep_list[i].priority, NULL);
        if ((tmpNodeId = pvtBuildByPath(hVal, parentId, "callSignalAddress", 1, NULL)) < 0)
            fprintf(stdout, "Could not add callSignalAddress to altEP1\n");
        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(callSignalAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add callSignalAddress in list1\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[i].callSignalIp);
		sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[i].callSignalPort, NULL);
        // Add rasAddress now
        if ((tmpNodeId = pvtBuildByPath(hVal, tempNodeId, "rasAddress", 1, NULL)) < 0)
            fprintf(stdout, "Could not add rasAddress to altEP %d\n", i);
        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(rasAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add rasAddress in list\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[i].rasIp);
		sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[i].rasPort, NULL);
        // Add Alias address now
        if ((aliasType = getAliasType(ep_list[i].aliasType, i)) != -1)
        {
            if ((tmpNodeId = pvtBuildByPath(hVal, tempNodeId, "aliasAddress", 1, NULL)) < 0)
                fprintf(stdout, "Could not add alias to altEP 1\n");
            if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(aliasAddress), -1, NULL,&temp)) < 0)
            {
                fprintf(stdout, "Failed to add alias in list\n");
            }
            switch (aliasType)
            {
                case cmAliasTypeH323ID:
                    aliasLen = utlChr2Bmp(ep_list[i].alias, (BYTE *)bmpStr);
                    if (pvtBuildByPath(hVal, parentId, "h323-ID", aliasLen, bmpStr) < 0)
                        fprintf(stdout, "Could not add h323-ID in alias\n");
                    break;
                case cmAliasTypeE164:
                    if (pvtBuildByPath(hVal, parentId, "e164", strlen(ep_list[i].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add dialedDigits in alias\n");
                    break;
                case cmAliasTypeURLID:
                    if (pvtBuildByPath(hVal, parentId, "url-ID", strlen(ep_list[i].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add url-ID in alias\n");
                    break;
            }
        }
    }
    cmMeiExit(hApp);

 _return:

	return rc;
}

void read_ep_list_cfg_file(int *ep_count)
{
    FILE* fp_cfg = NULL;
    char new_line[200];
    char elem0[20], elem1[20], elem2[20];
    int  i = 0;
    *ep_count = -1;
    fprintf(stdout, "Inside read_ep_list_cfg_file\n");

    fp_cfg = fopen(gk_cfg_file, "r");
    if (fp_cfg == NULL)
    {
        fprintf(stdout, "Could not open the Endpoint List configuration file. Exit!\n");
	return;
        //exit(0);
    }

    while (fgets(new_line, 199, fp_cfg) != NULL)
    {
        //fprintf(stdout, "Line read [%s]\n",new_line);

        sscanf(new_line, "%s %s %s", &elem0, &elem1, &elem2);
        if (!strcmp(elem0, "EP"))
        {
            (*ep_count)++;
            ep_list[*ep_count].priority = 0;
        }
        else if (!strcmp(elem0, "callSignalAddress"))
        {
            strncpy(ep_list[*ep_count].callSignalIp, elem1, 19);
            ep_list[*ep_count].callSignalPort = atoi(elem2);
        }
        else if (!strcmp(elem0, "rasAddress"))
        {
            strncpy(ep_list[*ep_count].rasIp, elem1, 19);
            ep_list[*ep_count].rasPort = atoi(elem2);
        }
        else if (!strcmp(elem0, "aliasAddress"))
        {
            strncpy(ep_list[*ep_count].aliasType, elem1, 14);
            strncpy(ep_list[*ep_count].alias, elem2, 19);
        }
        else if (!strcmp(elem0, "priority"))
        {
            if (elem1 != NULL)
                ep_list[*ep_count].priority = atoi(elem1);
            else
                fprintf(stdout, "No priority specified for EP %d\n", *ep_count);
        }
    }

    for (i = 0; i <= *ep_count; i++)
    {
        fprintf(stdout, "EP q931[%d]: %s %d, ras: %s %d, alias: %s %s\n", 
                i,ep_list[i].callSignalIp, ep_list[i].callSignalPort,
                ep_list[i].rasIp, ep_list[i].rasPort,
                ep_list[i].aliasType, ep_list[i].alias);
    }
    fclose(fp_cfg);
}

int getAliasType(char *aliasStr, int ep_id)
{
    if (!strcmp(aliasStr, "dialedDigits"))
        return cmAliasTypeE164;
    else if (!strcmp(aliasStr, "h323-ID"))
        return cmAliasTypeH323ID;
    else if (!strcmp(aliasStr, "url-ID"))
        return cmAliasTypeURLID;
    else
    {
        fprintf(stdout, "No valid alias type found for EP %d\n", ep_id);
        return -1;
    }
}

void 
read_serviceControl_list_cfg_file()
{
    FILE* fp_cfg = NULL;
    char new_line[2000];
    char elem0[20], elem1[20], elem2[1500];
    int  i = 0;

    fp_cfg = fopen(serviceCtrl_cfg_file, "r");
    if (fp_cfg == NULL)
    {
        fprintf(stdout, "Could not open the service control[serviceCtrl_cfg] file.\n");
	g_isServiceCtrl = 0;
	return;
        //exit(0);
    }

    g_sr_count = 1;
    while (fgets(new_line, 1999, fp_cfg) != NULL)
    {
        if("" == new_line)
	{
	   continue;
	}
        memset(elem0,0,20); memset(elem1,0,20); memset(elem2,0,1500);
        sscanf(new_line, "%s %s %s", &elem0, &elem1, &elem2);

        if (0 == strncmp(elem0, "ACF_serviceControl",17))
        {
	    srCtrl_list[g_sr_count-1].isACF = 1;

            srCtrl_list[g_sr_count-1].sessionId = atoi(elem1);
	    memset(srCtrl_list[g_sr_count-1].serviceDesc,0,1024);
            strncpy(srCtrl_list[g_sr_count-1].serviceDesc, elem2, 1023);

	    ++g_sr_count;
        }
        else if (0 == strncmp(elem0, "ARJ_serviceControl",17))
	{
	    srCtrl_list[g_sr_count-1].isACF = 0;

            srCtrl_list[g_sr_count-1].sessionId = atoi(elem1);
	    memset(srCtrl_list[g_sr_count-1].serviceDesc,0,1024);
            strncpy(srCtrl_list[g_sr_count-1].serviceDesc, elem2, 1023);

	    ++g_sr_count;
	}
    }
    g_sr_count--;

    for (i = 0; i < g_sr_count; i++)
    {
        #if 0
        fprintf(stdout, "Service Control: [%d]:-->[%d] [%s]\n", 
                        g_sr_count,
                       srCtrl_list[i].sessionId, 
			srCtrl_list[i].serviceDesc);
	#endif
    }
    fclose(fp_cfg);
}

int
genHandleARQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
        char fn[] = "genHandleARQ():";

	int rc = -cmRASReasonRequestDenied, callError = 0;
	INT32 tlen;
	cmTransportAddress 
		  dstSignalAddr = { 0, 0, 1720, cmRASTransportTypeIP },
		  dstRASAddr    = { 0, 0, 1719, cmRASTransportTypeIP },
		  rasaddr;
	int addrlen = sizeof(cmTransportAddress);
	cmAlias alias;
	BOOL canMapAlias, isstring;
	int nodeId, altNodeId;
	int domainIp,sd,realmId;
	unsigned short domainPort;
        int index=0, fieldId, temp;
        int rootNodeId, parentId;
        int tmpNodeId, addrNodeId, tempNodeId;
        FILE *fp_dump = NULL;
        INT16   fieldPath[] = {_nul(0) LAST_TOKEN};
		unsigned long       altEpIp;
    char                altEpIpStr[5];
        int i;
        int ep_count = -1, aliasType, aliasLen;
        BYTE bmpStr[256];
	HPVT hVal = cmGetValTree(hApp);
        HPST hPst = cmGetSynTreeByRootName(hApp, "ras");
	BYTE endptId[128], string[80];
	cmAlias number;
	cmAlias sourceIP;
	int     aliasSize=sizeof(sourceIP);
	INT32 localIP=0;
        char                destGWIpStr[5];

	//fprintf(stdout,"Entering genHandleARQ\n");


	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
				 cmRASParamSocketDesc, 0, 
				 (int *)&sd, NULL) < 0)
	{
	     fprintf(stdout, "%s No socket\n", fn);
	}

	cmMeiEnter(hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	if (pvtGetByPath(hVal, nodeId,
		"request.admissionRequest.canMapAlias", NULL, 
			&canMapAlias, &isstring) < 0)
	{
		fprintf(stderr, ("%s could not build MapAlias\n", fn));
	}

        read_ep_list_cfg_file(&ep_count);
	cmMeiExit(hApp);

        tlen = sizeof(cmAlias);
	number.length = 80;
	number.string = (char *)string;
        while (cmRASGetParam(hsRas, 
	                     cmRASTrStageRequest,
			     cmRASParamDestInfo,
			     index++,
			     &tlen,
			     (char *)&number) >= 0)
        {
	     if (number.type == cmAliasTypeE164)
	     {
                break;
	     }
        }
        fprintf(stdout,"Destination info contains [%s] as dialled number\n",
		                number.string);
	if(0 != strncmp(number.string,"1",1))
	{
	     return genHandleARJ(hsRas,
	                         hVal,
				 nodeId,
				 -cmRASReasonResourceUnavailable);
	}
	                        
 _reply:
    dstSignalAddr.ip = inet_addr(ep_list[0].callSignalIp);
    dstSignalAddr.port = ep_list[0].callSignalPort;

    // Set the Call Signal Address 
    // By default hair-pinning destination call signalling address
    //
    if (cmRASSetParam(hsRas,
		   cmRASTrStageConfirm,
		   cmRASParamDestCallSignalAddress, 0, 
		   addrlen, (char *)srcAddress) < 0)
    {
           fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed\n", fn);
    }

    cmMeiEnter(hApp);
    nodeId = cmGetProperty((HPROTOCOL)hsRas);

    if ((altNodeId = pvtGetNodeIdByPath(hVal, nodeId,
                                      "response.admissionConfirm")) < 0)
    {
            fprintf(stdout, "No response.admissionConfirm \n");
    }
	 
    // if destination call signalling address is not hair-pinned, then we need to set it
    //
    if(0 == g_hairPin)
    {
        memset(destGWIpStr,0,5);
	sprintf(destGWIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&g_destGWAddrLong)[0],
                (int) ((unsigned char*)&g_destGWAddrLong)[1],
                (int) ((unsigned char*)&g_destGWAddrLong)[2],
                (int) ((unsigned char*)&g_destGWAddrLong)[3]);
        if(pvtBuildByPath(hVal, altNodeId, 
	                  "destCallSignalAddress.ipAddress.ip", 
			  4, destGWIpStr) < 0)
	{
	    fprintf(stdout, "Could not set destCallSignalAddress ip\n");
        }
    }

    // Set destCallSignalAddress's port
    //
    //if(pvtBuildByPath(hVal, altNodeId, "destCallSignalAddress.ipAddress.port", 1720, NULL) < 0)
    if(pvtBuildByPath(hVal, altNodeId, "destCallSignalAddress.ipAddress.port", gwPort, NULL) < 0)
    {
	    fprintf(stdout, "Could not set destCallSignalAddress port\n");
    }
    //

    // Add BandWidth
    //
    if(pvtBuildByPath(hVal, altNodeId, "bandWidth", 640, NULL) < 0)
    {
	    fprintf(stdout, "Could not add BandWidth\n");
    }
    //

    // Add CallModel
    //
    //if(pvtBuildByPath(hVal, altNodeId, "callModel.gatekeeperRouted", -1, NULL) < 0)
    if(pvtBuildByPath(hVal, altNodeId, "callModel.direct", -1, NULL) < 0)
    {
	    fprintf(stdout, "Could not add Call Model\n");
    }
    //

    //
    if(0 == strcmp(gtd_list[0].gtd,"test_gtd"))
    {
             fprintf(stdout,"no GTD for ACF\n");
    }
    else
    {
             encodeACFNonStandardInfo(hsRas,altNodeId);
    }

    // IRR Frequency
    //
    /*
    if (cmRASSetParam(hsRas,
			   cmRASTrStageConfirm,
			   cmRASParamIrrFrequency, 0, 
			   240, NULL) < 0)
    {
	fprintf(stdout, "%s cmRASSetParam cmRASParamIrrFrequency failed\n", fn);
    }
    */
    //

    // DestinationInfo
    //
    if(pvtBuildByPath(hVal, altNodeId, "destinationInfo.1.e164", number.length, number.string) < 0)
    {
	    fprintf(stdout, "Could not add destinationInfo \n");
    }
    //

    // willRespondToIRR
    //
    if (cmRASSetParam(hsRas,
			   cmRASTrStageConfirm,
			   cmRASParamWillRespondToIRR, 0, 
			   0, NULL) < 0)
    {
	fprintf(stdout, "%s cmRASSetParam cmRASParamRASAddress failed\n", fn);
    }
    //

    // uuiesRequested
    //
    if((tmpNodeId = pvtBuildByPath(hVal, altNodeId, 
	                                "uuiesRequested", -1, NULL)) < 0)
    {
	    fprintf(stdout, "Could not add uuiesRequested\n");
    }

    if(
	    (pvtBuildByPath(hVal, tmpNodeId, "setup", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "callProceeding", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "connect", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "alerting", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "information", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "releaseComplete", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "facility", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "progress", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "empty", 0, NULL) < 0) ||
	    (pvtBuildByPath(hVal, tmpNodeId, "status", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "statusInquiry", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "setupAcknowledge", 0, NULL) < 0) || 
	    (pvtBuildByPath(hVal, tmpNodeId, "notify", 0, NULL) < 0))
     {
	    fprintf(stdout, "Could not add one of the UUIEsRequested field\n");
     }
     else
     {
	    //fprintf(stdout, "Added UUIEsRequested fields\n");
     }

     // Add origin circuit id
     //
     if (strlen(otg))
     {
     	      if (pvtBuildByPath(hVal, altNodeId,
                                 "circuitInfo.sourceCircuitID.group.group", 
				 strlen(otg), 
				 otg) < 0)
       	      {
		  fprintf(stderr, "pvtBuildByPath() failed to build " 
			                      "source trunk group\n");
       	      }
     }
     //


     // Add destination circuit id
     //
     if (strlen(dtg))
     {
     	      if (pvtBuildByPath(hVal, altNodeId,
                                 "circuitInfo.destinationCircuitID.group.group", 
				 strlen(dtg), 
				 dtg) < 0)
       	      {
		  fprintf(stderr, "pvtBuildByPath() failed to build " 
			                      "destination trunk group\n");
       	      }
     }
     //

     // Service Control
     //
     if(1 == g_isServiceCtrl)
     {
         if(attachServiceControl(hVal,altNodeId,1) < 0)
         {
	    fprintf(stdout, "Could not add service control field\n");
         }
     }
     //
/****************************************************/
    // Now add the alternateEndpoints
    //
    if (ep_count > 0) 
    {
        if ((altNodeId = pvtGetNodeIdByPath(hVal, nodeId,
                        "response.admissionConfirm")) < 0)
        {
            fprintf(stdout, "No alternateEndpoints\n");
        }
        hPst = cmGetSynTreeByRootName(hApp, "ras");
        if (hPst == NULL)
            fprintf(stdout, "No HPST\n"); 
        
        if ((parentId = pvtBuildByFieldIds(hVal, altNodeId, getParamPath(), 1, NULL)) < 0)
        {
            fprintf(stdout, "Failed to add alternateEndpoints\n");
        }
        rootNodeId = parentId;
        pvtBuildByPath(hVal, rootNodeId, "alternateEndpoints.priority", 
	                   ep_list[0].priority, NULL);
        if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                                "alternateEndpoints.callSignalAddress", 
					1, NULL)) < 0)
        {
            fprintf(stdout, "Could not add callSignalAddress to altEP\n");
        }

        if ((parentId = pvtAdd(hVal, tmpNodeId, 
	                __q931(callSignalAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add callSignalAddress in list\n");
        }

        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[0].callSignalIp);
	sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[0].callSignalPort, NULL);

        // Add rasAddress now
	//
        if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                              "alternateEndpoints.rasAddress", 1, NULL)) < 0)
        {
            fprintf(stdout, "Could not add rasAddress to altEP 1\n");
        }

        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(rasAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add rasAddress in list\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[0].rasIp);
	sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[0].rasPort, NULL);

        // Add Alias address now
	//
        if ((aliasType = getAliasType(ep_list[0].aliasType, 1)) != -1)
        {
            if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                         "alternateEndpoints.aliasAddress", 1, NULL)) < 0)
	    {
                fprintf(stdout, "Could not add alias to altEP 1\n");
	    }

            if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(aliasAddress), 
	                                 -1, NULL,&temp)) < 0)
            {
                fprintf(stdout, "Failed to add alias in list\n");
            }
            switch (aliasType)
            {
                case cmAliasTypeH323ID:
                    aliasLen = utlChr2Bmp(ep_list[0].alias, (BYTE *)bmpStr);
                    if (pvtBuildByPath(hVal, parentId, "h323-ID", aliasLen, bmpStr) < 0)
                        fprintf(stdout, "Could not add h323-ID in alias\n");
                    break;

                case cmAliasTypeE164:
                    if (pvtBuildByPath(hVal, parentId, "e164", 
		                       strlen(ep_list[0].alias), ep_list[0].alias) < 0)
                        fprintf(stdout, "Could not add dialedDigits in alias\n");
                    break;

                case cmAliasTypeURLID:
                    if (pvtBuildByPath(hVal, parentId, "url-ID", 
		                       strlen(ep_list[0].alias), ep_list[0].alias) < 0)
                        fprintf(stdout, "Could not add url-ID in alias\n");
                    break;
            }
        }
    }
    //

    for (i = 1; i <= ep_count; i++)
    {
        if ((parentId = pvtBuildByFieldIds(hVal, rootNodeId, fieldPath, 1, NULL)) < 0)
        {
            fprintf(stdout, "Failed to add alternateEndpoints\n");
        }
        tempNodeId = parentId;
        pvtBuildByPath(hVal, parentId, "priority", ep_list[i].priority, NULL);
        if ((tmpNodeId = pvtBuildByPath(hVal, parentId, "callSignalAddress", 1, NULL)) < 0)
            fprintf(stdout, "Could not add callSignalAddress to altEP1\n");
        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(callSignalAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add callSignalAddress in list1\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[i].callSignalIp);
		sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[i].callSignalPort, NULL);
        // Add rasAddress now
        if ((tmpNodeId = pvtBuildByPath(hVal, tempNodeId, "rasAddress", 1, NULL)) < 0)
            fprintf(stdout, "Could not add rasAddress to altEP %d\n", i);
        if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(rasAddress), -1, NULL,&temp)) < 0)
        {
            fprintf(stdout, "Failed to add rasAddress in list\n");
        }
        addrNodeId = pvtBuildByPath(hVal, parentId, "ipAddress", 0, NULL);
        altEpIp = inet_addr(ep_list[i].rasIp);
		sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);
        pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
        pvtBuildByPath(hVal, addrNodeId, "port", ep_list[i].rasPort, NULL);
        // Add Alias address now
        if ((aliasType = getAliasType(ep_list[i].aliasType, i)) != -1)
        {
            if ((tmpNodeId = pvtBuildByPath(hVal, tempNodeId, "aliasAddress", 1, NULL)) < 0)
                fprintf(stdout, "Could not add alias to altEP 1\n");
            if ((parentId = pvtAdd(hVal, tmpNodeId, __q931(aliasAddress), -1, NULL,&temp)) < 0)
            {
                fprintf(stdout, "Failed to add alias in list\n");
            }
            switch (aliasType)
            {
                case cmAliasTypeH323ID:
                    aliasLen = utlChr2Bmp(ep_list[i].alias, (BYTE *)bmpStr);
                    if (pvtBuildByPath(hVal, parentId, "h323-ID", aliasLen, bmpStr) < 0)
                        fprintf(stdout, "Could not add h323-ID in alias\n");
                    break;
                case cmAliasTypeE164:
                    if (pvtBuildByPath(hVal, parentId, "e164", strlen(ep_list[i].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add dialedDigits in alias\n");
                    break;
                case cmAliasTypeURLID:
                    if (pvtBuildByPath(hVal, parentId, "url-ID", strlen(ep_list[i].alias), ep_list[1].alias) < 0)
                        fprintf(stdout, "Could not add url-ID in alias\n");
                    break;
            }
        }
    }
	 
	 /*******************************************************/
     //
     cmMeiExit(hApp);

 _return:
	return -rc;
}

int
genHandleARJ(
		IN	HRAS			hsRas,
		IN      HPVT                    hVal,
		IN      int                     nodeId,
		IN      cmRASReason             reason)
{
                int altNodeId = 0;

		//fprintf(stdout,"Entering genHandleARJ\n");

     	        if (cmRASSetParam(hsRas,
			   cmRASTrStageReject,
			   cmRASParamRejectReason, 0, 
			   //cmRASReasonInvalidEndpointID, NULL) < 0)
			   reason, NULL) < 0)
	        {
                    fprintf(stdout, "No cmRASParamRejectReason\n");
	        }

                if ((altNodeId = pvtGetNodeIdByPath(hVal, nodeId,
                                     "response.admissionReject")) < 0)
                {
                    fprintf(stdout, "No response.admissionReject\n");
                }
		else
		{ 
		     if(1 == g_isServiceCtrl)
		     {
		         if(attachServiceControl(hVal,altNodeId,0) < 0) 
		         { 
		             fprintf(stdout, "Could not add service control field\n"); 
		         }
		     }
		}

		return reason;
}

int
encodeACFNonStandardInfo(
				IN	HRAS	hsRas,
				IN      int     contentNodeId)
{
	static char 			fn[] = "encodeACFNonStandardInfo";
        HPVT 				hVal;
	int				message = -1, nodeId = -1;
	int 				nonstdMsgId = -1;
	HPST 	                        synTreeCisco = NULL;
        int 				iBufLen = 0;
        BYTE				*encodeBuffer;
	int				oldNonstdNodeId = -1;
	char 				callIdStr[CALL_ID_STRLEN];
	int				len = 0;
	char 				confID[GUID_LEN] = {0};
        //char*                           gtd = NULL; 
        //int                             gtdLen;
	unsigned char                   buf1[1024],buf2[1024];
	int                             h221NodeId=0,rootNodeId =0;
	int                             tmpNodeId=0,nonStdId=0;
	int                             reasonNodeId=0;
	int                             sr_count=0;
	
	NETDEBUG(MLRQ, NETLOG_DEBUG4, ("%s Entering\n", fn));
	//fprintf(stdout,"%s Entering\n", fn);

	//
        //read_serviceControl_list_cfg_file(&sr_count);

        //fprintf(stdout,"Before test\n");
	if(NULL == hApp)
	{
	        fprintf(stdout,"hApp is NULL\n");
	}
        if (NULL == hsRas)
	{
	        fprintf(stdout,"hsRas is NULL\n");
	}

        //fprintf(stdout,"Before cmGetValTree\n");
        hVal = cmGetValTree(hApp);
        if (NULL == hVal)
	{
	        fprintf(stdout,"hVal is NULL\n");
	}
        //fprintf(stdout,"After cmGetValTree\n");

	synTreeCisco = cmGetSynTreeByRootName(hApp, "ciscoacf");

        if(NULL == synTreeCisco)
	{
	        fprintf(stdout,"synTreeCisco is NULL\n");
	}

        if ((nonstdMsgId = pvtAddRoot(hVal, synTreeCisco, 0, NULL))<0)
	{
		NETERROR(MARQ,
				("%s failed to add root node\n", fn));
    	        return -1;
	}
	else
	{
	 //       fprintf(stdout,"Added ciscoacf\n");
	}

	if ((nodeId = pvtBuildByPath(hVal, nonstdMsgId,
				"srcTerminalAlias", -1, NULL)) > 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s added source alias nodeId= %d\n",fn, nodeId));
		//fprintf(stdout,"%s added source alias nodeId= %d\n",fn, nodeId);
	}
	else
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Could not add source alias\n",fn));
		fprintf(stdout,"%s Could not add source alias\n",fn);
	}
        
	if ((nodeId = pvtBuildByPath(hVal, nonstdMsgId,
				"dstTerminalAlias", -1, NULL)) > 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s added source ext alias nodeId= %d\n",fn, nodeId));
		//fprintf(stdout,"%s added source ext alias nodeId= %d\n",fn, nodeId);
	}
	else
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Could not add source ext alias\n",fn));
		fprintf(stdout,"%s Could not add source ext alias\n",fn);
	}

	// Other parameters are optional
	//

	//
	//read_GTD_cfg_file(&gtd,&gtdLen);

        // add gtd
	//
	if (pvtBuildByPath(hVal, nonstdMsgId,
				//"gtd.gtdData", gtdLen, gtd) < 0)
				"gtd.gtdData", 
				strlen(gtd_list[0].gtd), 
				gtd_list[0].gtd) < 0)
	{
		fprintf(stdout,"[%s]: Error in adding gtd node\n",fn);
	}
	else
	{
		//fprintf(stdout,"[%s]: Added sucessfully gtd node\n",fn);
	}

        if (cmRASGetParam(hsRas,
		cmRASTrStageRequest,
		cmRASParamNonStandardData, 
		0, 
		&oldNonstdNodeId,
		NULL) < 0)
	{
		NETDEBUG(MARQ, NETLOG_DEBUG4,
			("%s Could not get old non standard parameter\n", fn));
		//fprintf(stdout,"[%s]: Could not get old non standard parameter\n", fn);
	}
	else
	{
		fprintf(stdout,"[%s]: Got old non standard parameter\n", fn);

		if (oldNonstdNodeId > 0)
		{
			pvtDelete(hVal, oldNonstdNodeId);
			NETDEBUG(MARQ, NETLOG_DEBUG4,
				("%s Deleted old non standard parameter\n", fn));
			fprintf(stdout,"%s Deleted old non standard parameter\n", fn);
		}
	}

        getEncodeDecodeBuffer(MAX_ENCODEDECODE_BUFFERSIZE, &encodeBuffer);
        if (!encodeBuffer)
	{
		NETERROR(MARQ,
			("%s: Could not get encode buffer\n", fn));
		fprintf(stdout,"[%s]: Could not get encode buffer\n", fn);
    	        return -1;
	}

//	memset(encodeBuffer,65,MAX_ENCODEDECODE_BUFFERSIZE);
//	iBufLen = MAX_ENCODEDECODE_BUFFERSIZE;
	
	if (!
	     (cmEmEncode(hVal, 
	                 nonstdMsgId, 
			 encodeBuffer,
			 MAX_ENCODEDECODE_BUFFERSIZE, 
			 &iBufLen) < 0)
	)
	{
            fprintf(stdout, "Encoding successfull\n");
        if ((nonStdId = pvtBuildByPath(hVal, contentNodeId, 
                                        "nonStandardData", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add nonStandard to serviceControl\n");
        }
	else
	{
            //fprintf(stdout, "Added nonStandard to serviceControl\n");
	}

        char oid[4] = {'a','b','c','d'};
        if ((tmpNodeId = pvtBuildByPath(hVal, nonStdId, 
                                        "nonStandardIdentifier.object", 
					4, 
					oid)) < 0)
        {
            fprintf(stdout, "Could not add nonStandardIdentifier.object to serviceControl\n");
        }


        if ((h221NodeId = pvtBuildByPath(hVal, nonStdId, 
                                        "nonStandardIdentifier.h221NonStandard", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add nonStandardIdentifier.h221NonStandard to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "t35CountryCode", 
					COUNTRY_CODE_181, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add t35CountryCode to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "t35Extension", 
					0, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add t35Extension to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "manufacturerCode", 
					MANUFACTURER_CISCO, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add manufacturerCode to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, nonStdId,
	                                "data", 
					iBufLen,
					encodeBuffer)) < 0)

        {
            fprintf(stdout, "Could not add data to serviceControl\n");
        }
	else
        {
            //fprintf(stdout, "Added data to serviceControl\n");
        }

        #if 0
		fprintf(stdout,"[%s] Encoding Sucessfull!\n",fn);

                /* That's it, now we can add the encoded ARQNonstandardInfo 
		in the ARQ message */
	
		cmNonStandardIdentifier 	*nsid;
		cmNonStandardParam 		nsparam;
		int				tlen =0;
		
		nsid = &(nsparam.info);	
		nsparam.length = iBufLen;
		nsparam.data = (char *)encodeBuffer;
	
		/* setup the h221 info for cisco */
		nsid->objectLength = 0;
		nsid->t35CountryCode = COUNTRY_CODE_181;
		nsid->t35Extension = 0;
		nsid->manufacturerCode = MANUFACTURER_CISCO;	/* cisco */

		tlen = sizeof(nsparam);
		if (cmRASSetParam(hsRas,
			cmRASTrStageRequest,
			cmRASParamNonStandard, 0, tlen,
			(char*)&nsparam) < 0)
		{
			NETERROR(MARQ,
				("%s Could not set non standard parameter\n", fn));
		        fprintf(stdout,"Error in adding GTD Non standard\n");
		}
		else
		{
		        fprintf(stdout,"Added GTD Non standard\n");
		}
	#endif
	}
        else
	{
		NETERROR(MARQ,
			("%s Encoding Problems!\n",fn));
		fprintf(stdout,"[%s] Encoding Problems!\n",fn);
	}
        //pvtDelete(hVal, nonstdMsgId);
    
        return 0;
}

int
attachServiceControl(
			IN OUT	HPVT   hVal,
			IN	int    altNodeId,
			IN      int    isACF)
{
	static char 			fn[] = "attachServiceControl";
        int 				iBufLen = 0;
        BYTE				*encodeBuffer;
	char 				callIdStr[CALL_ID_STRLEN];
	int				len = 0;
	FILE                            *fd = NULL;
	unsigned char                   buf1[1024],buf2[1024];

	int                             i = 0, parentId = 0,cnt = 0;
	int				message = -1, nodeId = -1;
	int                             h221NodeId=0,rootNodeId =0;
	int                             tmpNodeId=0,nonStdId=0;
	int                             contentNodeId=0,reasonNodeId=0;
	int 				nonstdMsgId = -1;
	int				oldNonstdNodeId = -1;
	HPST hPst = NULL;
	
	NETDEBUG(MLRQ, NETLOG_DEBUG4, ("%s Entering\n", fn));

    // Now add the service control
    //
    i = 1;
    if (g_sr_count > 0) 
    {
      //
      for (cnt = 0; cnt < g_sr_count; cnt++)
      {
        if(1 == isACF)
	{
	   if(1 != srCtrl_list[cnt].isACF)
	   {
	        continue;
	   }
	}
	else if(0 == isACF)
	{
	   if(0 != srCtrl_list[cnt].isACF)
	   {
	        continue;
	   }
	}
	else
	{
	     continue;
	}

        char serCtrl[50];
	memset(serCtrl,0,50);
	sprintf(serCtrl,"serviceControl.%d",i);
	i++;

        //fprintf(stdout, "Service Control [%d],[%s] \n",i,serCtrl);
	
        if ((parentId = pvtBuildByPath(hVal, altNodeId, 
	                  serCtrl, -1, NULL)) < 0)
        {
            fprintf(stdout, "Failed to add serviceControl\n");
        }
        rootNodeId = parentId;

	//
        if ((tmpNodeId = pvtBuildByPath(hVal, rootNodeId, 
	                                "sessionId", 
					srCtrl_list[cnt].sessionId,
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add sessionId to serviceControl\n");
        }

        if ((contentNodeId = pvtBuildByPath(hVal, rootNodeId, 
                                        "contents", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add contents to serviceControl\n");
        }

        if ((reasonNodeId = pvtBuildByPath(hVal, rootNodeId, 
                                        "reason", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add reason to serviceControl\n");
        }

        if ((nonStdId = pvtBuildByPath(hVal, contentNodeId, 
                                        "nonStandard", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add nonStandard to serviceControl\n");
        }
	else
	{
            //fprintf(stdout, "Added nonStandard to serviceControl\n");
	}

        char oid[4] = {'a','b','c','d'};
        if ((tmpNodeId = pvtBuildByPath(hVal, nonStdId, 
                                        "nonStandardIdentifier.object", 
					4, 
					oid)) < 0)
        {
            fprintf(stdout, "Could not add nonStandardIdentifier.object to serviceControl\n");
        }


        if ((h221NodeId = pvtBuildByPath(hVal, nonStdId, 
                                        "nonStandardIdentifier.h221NonStandard", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add nonStandardIdentifier.h221NonStandard to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "t35CountryCode", 
					COUNTRY_CODE_181, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add t35CountryCode to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "t35Extension", 
					0, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add t35Extension to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, h221NodeId,
                                       "manufacturerCode", 
					//MANUFACTURER_CISCO, 
					0, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add manufacturerCode to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, nonStdId,
	                                "data", 
					strlen(srCtrl_list[cnt].serviceDesc), 
					srCtrl_list[cnt].serviceDesc)) < 0)
        {
            fprintf(stdout, "Could not add data to serviceControl\n");
        }
	else
        {
            //fprintf(stdout, "Added data to serviceControl\n");
        }

        if ((tmpNodeId = pvtBuildByPath(hVal, reasonNodeId,
                                        "open", 
					-1, 
					NULL)) < 0)
        {
            fprintf(stdout, "Could not add open to reason\n");
        }
	else
        {
            //fprintf(stdout, "Added open to reason\n");
        }

      }
    }
    else
    {
            fprintf(stdout, "No service control info\n");
    }

    return 1;
}

int
genHandleRRQ(
		IN	HRAS			hsRas,
		IN	HCALL			hsCall,
		OUT	LPHAPPRAS		lphaRas,
		IN	cmTransportAddress	*srcAddress,
		IN	HAPPCALL		haCall)
{
        char fn[] = "genHandleRRQ()";
	int rc = -cmRASReasonRequestDenied, callError = 0;
	INT32 tlen;
	cmTransportAddress 
		  dstSignalAddr = { 0, 0, 1720, cmRASTransportTypeIP },
		  dstRASAddr    = { 0, 0, 1719, cmRASTransportTypeIP },
		  rasaddr;
	int addrlen = sizeof(cmTransportAddress);
	cmAlias alias;
	BOOL canMapAlias, isstring;
	int nodeId, altNodeId;
	HPVT hVal = cmGetValTree(hApp);
        HPST hPst = cmGetSynTreeByRootName(hApp, "ras");
	int domainIp,sd,realmId;
	unsigned short domainPort;
        int index, fieldId, temp;
        int rootNodeId, parentId;
        int tmpNodeId, addrNodeId, tempNodeId;
        FILE *fp_dump = NULL;
        INT16   fieldPath[] = {_nul(0) LAST_TOKEN};
        unsigned long       altEpIp;
        char                altEpIpStr[5];
        int i;
        int ep_count = -1, aliasType, aliasLen;
        BYTE bmpStr[256];
	int bmpStrLen = 0;
	fprintf(stdout, "Entered function [%s]\n", fn);

        read_ep_list_cfg_file(&ep_count);
	memset(altEpIpStr,0,5);

	if (cmRASGetParam(hsRas, cmRASTrStageRequest,
				 cmRASParamSocketDesc, 0, 
				 (int *)&sd, NULL) < 0)
	{
		 fprintf(stdout, "%s No socket\n", fn);
	}

	cmMeiEnter(hApp);

	nodeId = cmGetProperty((HPROTOCOL)hsRas);

	cmMeiExit(hApp);

 _reply:
        // willRespondToIRR
	//
	if (cmRASSetParam(hsRas,
			   cmRASTrStageConfirm,
			   cmRASParamWillRespondToIRR, 0, 
			   0, NULL) < 0)
	{
		fprintf(stdout, "%s cmRASSetParam cmRASParamRASAddress failed\n", fn);
	}
	else
	{
		fprintf(stdout, "%s cmRASSetParam cmRASParamRASAddress pass\n", fn);
	}
	//

	//  maintainConnection
	//
	if (cmRASSetParam(hsRas,
			   cmRASTrStageConfirm,
			   cmRASParamMaintainConnection, 0, 
			   0, NULL) < 0)
	{
	   fprintf(stdout, "%s cmRASSetParam cmRASParamMaintainConnection failed\n", fn);
	}
	else
	{
	   fprintf(stdout, "%s cmRASSetParam cmRASParamMaintainConnection pass\n", fn);
	}
	//

        //
        if ((altNodeId = pvtGetNodeIdByPath(hVal, nodeId,
                                        "response.registrationConfirm")) < 0)
        {
            fprintf(stdout, "No alternateEndpoints\n");
        }

        //hPst = cmGetSynTreeByRootName(hApp, "ras");
        //if (hPst == NULL)
        //    fprintf(stdout, "No HPST\n"); 

        // Add protocolIdentifier
        //
	char* protoIdentifier = "1.2.3.4";
        if(pvtBuildByPath(hVal, altNodeId, "protocolIdentifier", 
                              strlen(protoIdentifier), protoIdentifier) < 0)
	{
	    fprintf(stdout, "Could not add protoIdentifier\n");
	}
	else
	{
	    //fprintf(stdout, "added protoIdentifier\n");
	}
	//

        // Add endpointIdentifier
        //
	bmpStrLen = utlChr2Bmp("test", (BYTE *)bmpStr);
        if(pvtBuildByPath(hVal, altNodeId, "endpointIdentifier", 
                              bmpStrLen, bmpStr) < 0)
	{
	    fprintf(stdout, "Could not add endpointIdentifier\n");
	}
	else
	{
	    //fprintf(stdout, "added endpointIdentifier\n");
	}
	//

	// Set the Call Signal Address 
	//
        dstSignalAddr.ip = inet_addr(ep_list[0].callSignalIp);
	dstSignalAddr.port = ep_list[0].callSignalPort;

        tmpNodeId = pvtBuildByPath(hVal, altNodeId, "callSignalAddress.1", -1, NULL);
        //tmpNodeId = pvtBuildByPath(hVal, altNodeId, "callSignalAddress.1", addrlen, 
	//                           &dstSignalAddr);
	if(tmpNodeId < 0)
	{
             fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed 1\n", fn);
	}

        addrNodeId = pvtBuildByPath(hVal, tmpNodeId, "ipAddress", 0, NULL);
	if(addrNodeId < 0)
	{
             fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed 2\n", fn);
	}

        altEpIp = inet_addr(ep_list[1].callSignalIp);
	sprintf(altEpIpStr, "%c%c%c%c", 
                (int) ((unsigned char*)&altEpIp)[0],
                (int) ((unsigned char*)&altEpIp)[1],
                (int) ((unsigned char*)&altEpIp)[2],
                (int) ((unsigned char*)&altEpIp)[3]);

        fprintf(stderr, "%s Call Signalling Address [%s],[%s]\n", fn,
	                        ep_list[1].callSignalIp,
				altEpIpStr);
        tmpNodeId = pvtBuildByPath(hVal, addrNodeId, "ip", 4, (char*)altEpIpStr);
	if(tmpNodeId < 0)
	{
             fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed 4\n", fn);
	}

        tmpNodeId = pvtBuildByPath(hVal, addrNodeId, "port", 7777, NULL);
	if(tmpNodeId < 0)
	{
             fprintf(stderr, "%s cmRASSetParam cmRASParamCallSignalAddress failed 3[%d]\n", 
	                       fn,tmpNodeId);
	}

	//
        cmMeiExit(hApp);

	return rc;
}

static int SendGtd( IN HAPP hApp,int nodeId, char* gtd, int gtdLen)
{
        char fn[] = "SendGtd";
        int gtdNodeId; 
	cmTunnelledProtocolID     tunnelledProtocol={0}; 
	cmOctetString   msgContent[2]={0}; 
	int    msgBodyNode =-1; 
	int    tunnelednodeId =-1; HPVT hVal;

        //fprintf(stdout,"Entering SendGtd\n");
	if(0 == strcmp(gtd,"test_gtd"))
	{
	     NETERROR(MH323,("Exiting SendGtd, no GTD for this message\n"));
	     return 0;
	}

        if ( !hApp)
        {
              NETERROR(MH323,("%s hApp is null \n",fn));
              return -1;
        }
	else
	{
              NETDEBUG(MH323,NETLOG_DEBUG4,("%s hApp is %p \n",fn,hApp)); 
	}

         hVal = cmGetValTree(hApp); 
	 nx_strlcpy(tunnelledProtocol.protocolType, "gtd",sizeof("gtd")); 
	 tunnelledProtocol.protocolTypeLength= sizeof("gtd"); 
	 msgContent[0].message = gtd; 
	 msgContent[0].size = gtdLen; 
	 msgContent[1].message = NULL; 
	 msgContent[1].size =0; 
	 if ((gtdNodeId = cmCallCreateAnnexMMessage(hApp, FALSE,
	                       &tunnelledProtocol,msgContent,NULL)) >0)
         { 
	    /* position on the UU-IE part of the message */
            msgBodyNode = pvtGetNodeIdByPath(hVal, nodeId, 
	                        "message.*.userUser.h323-UserInformation.h323-uu-pdu");                    
	    if (msgBodyNode > 0) 
	    { 
	            NETDEBUG(MH323,NETLOG_DEBUG4,("%s tunneled message uu nodeId :%d \n",
		                      fn,msgBodyNode)); 
	    }
	    else
	    { 
	            NETDEBUG(MH323,NETLOG_DEBUG4,("%s no uu nodeId found :%d \n",
		                      fn,msgBodyNode)); 
		    return -1; 
	    } 

	    __pvtBuildByFieldIds(tunnelednodeId, 
	                         hVal, 
				 msgBodyNode, 
				 {_q931(tunnelledSignallingMessage) LAST_TOKEN}, 
				 0, NULL); 

	    if (tunnelednodeId>=0) 
	    { 
	            if ( pvtMoveTree(hVal,tunnelednodeId, gtdNodeId) >0) 
		    { 
		           NETERROR(MH323,("%s Success sending tunneled message nodeId :%d message length:%d \n",fn,gtdNodeId,gtdLen)); 
		    }
		    else
		    { 
		           NETERROR(MH323,("%s Error sending tunneled message nodeId :%d message length:%d \n",fn,gtdNodeId,gtdLen)); 
		    } 
	    } 
	    else 
	    { 
	            NETERROR(MH323,("%s Error sending tunneled message nodeId :%d message length:%d \n",fn,gtdNodeId,gtdLen)); 
	    } 
	 }
	 else
	 { 
	        NETERROR(MH323,("%s Error creating  tunneled message nodeId :%d\n",fn,gtdNodeId)); 
	 } 
	 return 0; 
}

int
getGTDMsgOffset(char* msgname)
{
     if (!strcmp(msgname, "setup"))
            return 1;
     if (!strcmp(msgname, "connect"))
            return 2;
     if (!strcmp(msgname, "facility"))
            return 3;
     if (!strcmp(msgname, "callProceeding"))
            return 4;
     if (!strcmp(msgname, "alerting"))
            return 5;
     if (!strcmp(msgname, "releaseComplete"))
            return 6;
     if (!strcmp(msgname, "requestModeAck"))
            return 7;
     if (!strcmp(msgname, "requestModeReject"))
            return 8;
     else
     {
            // Returning gtd for acf in case invalid entry found to 
	    // avoid possibility of core dump
	    //
            return 0;
     }
}

void
getGTDData()
{
    FILE* fp_cfg = NULL; 
    char new_line[2000]; 
    int  sr_count=0; 
    int  i = 0; 
    int  line_read_len = 0;

    // Following listed *.gtd files are order specific
    // as getGTDMsgOffset() function depends upon their
    // sequence number
    //
    char *gtd_files[9] = {
                        "acf.gtd",
                        "setup.gtd",
			"connect.gtd",
			"facility.gtd",
			"callProceeding.gtd",
			"alerting.gtd",
			"releaseComplete.gtd",
			"requestModeAck.gtd",
			"requestModeReject.gtd"};

    for(i=0;gtd_files[i];++i)
    {
          //fprintf(stdout,"GTD file name [%s]\n",gtd_files[i]);
          fp_cfg = fopen(gtd_files[i], "r"); 
	  if (fp_cfg == NULL) 
	  {
             //fprintf(stdout, "Could not open [%s] file.\n",gtd_files[i]); 
             memset(gtd_list[i].gtd,0,2048);
	     memcpy(gtd_list[i].gtd, "test_gtd",8);
	     gtd_list[i].gtd_len = 8;
	  }
	  else
	  {
             memset(gtd_list[i].gtd,0,2048);
             memset(new_line,0,2000);

             while (fgets(new_line, 1999, fp_cfg) != NULL)
             {
	         line_read_len = strlen(new_line);

		 //fprintf(stdout, "Line read length [%d]\n",strlen(new_line));
		 new_line[line_read_len-1] = '\r';
		 new_line[line_read_len] = '\n';

	         //strncat(gtd_list[i].gtd, new_line, 2047); 
	         strcat(gtd_list[i].gtd, new_line);

                 memset(new_line,0,2000);

	         sr_count += line_read_len + 1;
             }
	     //strncat(gtd_list[i].gtd, "\r\n",2); 
	     //gtd_list[i].gtd_len = sr_count + 2;
	     gtd_list[i].gtd_len = sr_count;

	     //fprintf(stdout, "File [%s]\n",gtd_list[i].gtd);
	     //fprintf(stdout, "File Length [%d]\n",gtd_list[i].gtd_len);
             fclose(fp_cfg); 
	  }

	  sr_count = 0;
    }

    for(i=0;gtd_files[i];++i)
    {
	 //fprintf(stdout, "File [%s]\n",gtd_list[i].gtd);
	 //fprintf(stdout, "File Length [%d]\n",gtd_list[i].gtd_len);
    }
}

#endif 
/*  ! _WIN32 */
