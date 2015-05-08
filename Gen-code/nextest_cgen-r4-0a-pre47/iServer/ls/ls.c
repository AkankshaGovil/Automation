

/*
 * FILE:  ls.c
 *
 * DATE:  MARCH 8th 1998
 *
 * Copyright (c) 1998 Netoids Inc.
 ***************************************************************************/ 	
#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <limits.h>
#ifdef SUNOS
#include <sys/sockio.h>
#include <sys/filio.h>
#else
#include <linux/sockios.h>
#endif
#include <string.h>
#ifdef _QNX
#include <sys/select.h>
#endif
#include <sys/uio.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "license.h"
#include "licenseIf.h"
#include "pmpoll.h"

#define CLID_PORT   10005

#include "spversion.h"
#include "defaultpath.h"

#include "generic.h"
#include "bits.h"
#include "ipc.h"
#include "srvrlog.h"
#include "serverdb.h"
#include "key.h"
#include "mem.h"

#include "dh.h"
#include "bn.h"
#include "isakmp.h"
#include "isadb.h"
#include "protocol.h"
#include "lsprocess.h"
#include "entry.h"
#include "pef.h"
#include "lsconfig.h"
#include "phone.h"
#include "serverp.h"
#include "pids.h"
#include "ifs.h"
#include "gw.h"
#include "timer.h"
#include "fdsets.h"
#include "db.h"
#include "connapi.h"
#include "shm.h"
#include "shmapp.h"
#include "xmltags.h"
#include "sconfig.h"

#include "gis.h"
#include "firewallcontrol.h"
#include "execd.h"
#include "nxosd.h"
#include <sched.h>
#include <malloc.h>
#include "ls.h"
#include "scm_call.h"
#include "callid.h"
#include "cacheinit.h"
#include "cmsize.h"
#include "radclient.h"
#include "uh323.h"
#include "callstats.h"
#include "cdr.h"
#include "dbs.h"
#include "gk.h"
#include "bridge.h"
#include "ssip.h"
#include "ua.h"
#include "packets.h"
#include "iwfsmproto.h"
#include "server.h"


static int InitNetPort (void);
static int ProcessConfig(void);
static void SignalInit(void);
static void* AsyncSigHandlerThread( void* args );
static void SyncSigHandler(int signo);
static void DummySigHandler(int signo);


/* pidfile accessible globally, once the server is up */

NetFds 		lsnetfds;
NetFds		*lsh323fds;
NetFds 		lstimerfds;
NetFds		lsqfds;
Config 		localConfig;

char 		pidfile[256];
int 		shmId;
MemoryMap 	*map;
LsMemStruct 	*lsMem = 0,tmpLsMem = {0};
SipStats	*sipStats;
int	*shmH323PollQ;

// Will be the main thread id
pthread_t	lsThread = (pthread_t)-1, 
			timerThread = (pthread_t)-1,
			*h323Threads = NULL;

char 		config_file[60] = CONFIG_FILENAME; 	
				/* Default value for config_file */
int 		debug = 0;

int shutdown_inprogress = 0;

/* network order */
unsigned long 	iServerIP;


/* Following two variables are defined solely for backward compatibility
 * with release 1.0
 * These essentially have relevence for the last incoming request and 
 * identifies to the child as to what kind of request came in.
 * Note that for clients which are > rel 1.1, both should be set.
 */
int isLusReq = 0, isVpnsReq = 0;
int ncalls = 0;
int idaemon = 1;
int maxfds = 1024;
int poolid, lpcid, hpcid;
extern int *h323inPool, *h323stackinClass;
int uh323init = -1, uh323ready = -1;
int h323initsem, h323waitsem;

void *HandlePollPM(void *);
void *IpcHandleTimers(void *);
void *IcmpdInit(void *arg);
extern void* SipRegStart(void*);
extern void *SCM_Main(void *);

char  *GisCmdStr[]=
{
    "/usr/local/nextone/bin/cli realm open all",
    "/usr/local/nextone/bin/cli realm close all",
    "/usr/local/nextone/bin/ifmgmt up",
    "/usr/local/nextone/bin/ifmgmt down"
};
/* Pef Opaque structure passed to higher layer
 * application routines. Note that the header (similar to
 * PktHeader, must be the first field in all of the
 * opaque structures.
 */
typedef struct 
{
     PktHeader *hdr;
     SA *sa;
} PktOpaque;

int
GisSetMaxFds(int maxfds)
{
    struct  rlimit  rl_data;

    getrlimit( RLIMIT_NOFILE, &rl_data );

	if (maxfds >  (rl_data.rlim_max - 1))
	{
		NETERROR(MINIT, ("file descriptors maximum is %lu, requirement is %d\n",
			rl_data.rlim_max, maxfds+1));
		fprintf(stderr, "file descriptors maximum is %lu, requirement is %d\n",
			rl_data.rlim_max, maxfds+1);

		// keep going
		maxfds = rl_data.rlim_max - 1;
	}

    rl_data.rlim_cur = maxfds;

    if (setrlimit( RLIMIT_NOFILE, &rl_data ) < 0)
	{
		 NETERROR(MINIT, ("error in setting setrlimit %d\n", errno))
	}

    memset( &rl_data, (int32_t) 0, sizeof(struct rlimit) );

    getrlimit( RLIMIT_NOFILE, &rl_data );

    NETDEBUG( MINIT, NETLOG_DEBUG1,
        ("Maximum file descriptors set to %d\n", (uint32_t) rl_data.rlim_cur ));

	return rl_data.rlim_cur;
}

/* call end function, should end all calls by sending Sip Byes. */

static void
CallEnd()
{
	char fn[] = "CallEnd()";
	SCC_EventBlock *evtPtr;
	CallHandle *callHandle;
	SipAppCallHandle *pSipData;
	SipTrans *siptransptr;
	int all_freed = 0;
	int byes_inq  = 0, byes_sent=0, cur_byes = 0; 
	int count = 0, i;

	if (sipStats == NULL ) /* sipStats == NULL means shared memory not attached
				  return right away */
		return;

	for (i=0;i<xthreads;i++)
		cur_byes += sipStats[i].byec;

	CacheGetLocks (callCache, LOCK_WRITE, LOCK_BLOCK);

	for (callHandle = CacheGetFirst (callCache); callHandle;
	     callHandle = CacheGetNext (callCache, callHandle->callID)) 
	{
		evtPtr = (SCC_EventBlock *) malloc (sizeof (SCC_EventBlock));
		memset (evtPtr, 0, sizeof (SCC_EventBlock));
		switch (callHandle->handleType) 
		{
		case SCC_eSipCallHandle:
			if (SCMCALL_CheckState(callHandle->callID))
			{
				// call has been replciated, it should not be hung up
				break;
			}

			evtPtr->event = Sip_eBridgeError;
			memcpy (evtPtr->confID, callHandle->confID, CONF_ID_LEN);
			memcpy (evtPtr->callID, callHandle->callID, CALL_ID_LEN);
			evtPtr->callDetails.callError = SCC_errorShutdown;
			evtPtr->callDetails.lastEvent = callHandle->lastEvent;

			pSipData = (SipAppCallHandle *) malloc (sizeof (SipAppCallHandle));
			memset (pSipData, 0, sizeof (SipAppCallHandle));

			memcpy (pSipData->confID, evtPtr->confID, CONF_ID_LEN);
		        memcpy (pSipData->callID, evtPtr->callID, CALL_ID_LEN);

			evtPtr->data = pSipData;
			byes_inq ++;
			GisDeleteCallFromConf (pSipData->callID, evtPtr->confID);
			if (sipBridgeEventProcessor (evtPtr) != 0)
			{
				NETDEBUG (MBRIDGE, NETLOG_DEBUG4,
					  ("%s sipBridgeEventProcessor Failed\n", fn));
			}
			break;
		default:
			break;
		}
	}
	CacheReleaseLocks (callCache);

	/* Need to wait for threads to finish their jobs... */
	while (1) {
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 250000000; /* 250 millisecond. */
		nanosleep (&ts, NULL);
		byes_sent = 0;
		count ++;
		/*		if (count > 5) break; */
		byes_sent = 0;
		for (i=0;i<xthreads;i++)
			byes_sent += sipStats[i].byec;
		NETDEBUG (MBRIDGE, NETLOG_DEBUG4, 
			  ("byes sent out : %d  total placed in q: %d",
			   byes_sent - cur_byes , byes_inq));
		if (byes_sent - cur_byes == byes_inq) break;
	}
	
}

/* Special for GIS, as it has to destoy
 * all the shared memory
 * eventually call the general sig_int
 */
static void
sig_int_ls(int signo)
{
	if (signo != SIGINT)
	{
		return;
	}

	shutdown_inprogress = 1;

	NETERROR(	MDEF,
				("Caught INT signal - terminating gis\n" ));

	// All actions of the iServer visible to
	// the end user must be sone between these two log stmts
	NETINFOMSG(MDEF, ("*** NexTone GIS Server shutdown: started ***\n"));

	if(IServerIsPrimary())
	{
		// CDRs only if priamry
		ConfEnd();
		CdrEnd();
		CallEnd();
	}

	if (nH323Threads && h323Threads)
	{
		int i;

		for (i=0; i<nH323Threads; i++)
		{
			if (h323Threads[i] != (pthread_t)-1)
			{
				pthread_cancel(h323Threads[i]);
			}
		}
		
		// free not needed as we are shutting down
	}

	stopRadiusClient();

	GisExecuteCmd(GIS_REALM_DOWN_ALL_CMDID);
	/* shutdown the firewall control thread and release resources */
	FCEStop();

	UnlinkPid(pidfile);

	exit(0);
}

static void
sig_term(int signo)
{
	shutdown_inprogress = 1;

	NETERROR(	MDEF,
				("Caught TERM signal - terminating gis\n"));

	stopRadiusClient();

	GisExecuteCmd(GIS_REALM_DOWN_ALL_CMDID);
	/* shutdown the firewall control thread and release resources */
	FCEStop();

	UnlinkPid(pidfile);

	exit(0);
}


// returns true if we are in of the "main loop" threads
int
mainThread()
{
	pthread_t self;

	self = pthread_self();

	return (pthread_equal(timerThread, self) ||
		pthread_equal(lsThread, self) ||
		pthread_equal(h323Threads[0], self));
}

int
iServerPoll(tid t)
{
	// Always do this task
	iServerUpdateH323Allocations(NULL);

	// Check the locks
	iServerCheckHealth();

	// do sendpoll
	Sendpoll(t);
	return(0);
}

int
iServerCheckHealth(void)
{
	char fn[] = "iServerCheckHealth():";

	// If MSW has not initialized yet, do not start checks
	if (!CHECK_STATUS(lsMem, STATUS_ALL_INIT))
	{
		NETDEBUG(MINIT, 1, 
			("%s MSW has not initialized yet\n", fn));
		return 0;
	}

	// CALLS
	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	CacheReleaseLocks(callCache);

	// TSM
	CacheGetLocks(transCache, LOCK_WRITE, LOCK_BLOCK);
	CacheReleaseLocks(transCache);

	// database cache
	CacheGetLocks(regCache, LOCK_WRITE, LOCK_BLOCK);
	CacheReleaseLocks(regCache);

	// policy
	CacheGetLocks(cpCache, LOCK_WRITE, LOCK_BLOCK);
	CacheReleaseLocks(cpCache);

	CacheGetLocks(cpbCache, LOCK_WRITE, LOCK_BLOCK);
	CacheReleaseLocks(cpbCache);

	// no need to check the database locks ??

	return 0;
}

int
iServerUpdateH323Allocations(struct Timer* t)
{
	AllocStats *a;
	int i;

	if ((uh323ready>=0) && updateAllocations)
	{
		for (i=0; i<nh323Instances; i++)
		{
			HAPP 	hApp = uh323Globals[i].hApp;
			HPVT	hVal = cmGetValTree(hApp);

			a = lsMem->allocStats[i];

			// update the shared memory with h.323 alloc
			// protocol = 60 bytes each
			a->xprotocols = cmSizeMaxProtocols(hApp);
			a->nprotocols = cmSizeCurProtocols(hApp);

			// procs = 1024 bytes
			a->xprocs = cmSizeMaxProcs(hApp);
			a->nprocs = cmSizeCurProcs(hApp);

			// The number of PDL state machine events .
			a->xevents = cmSizeMaxEvents(hApp);
			a->nevents = cmSizeCurEvents(hApp);

			// The number of PDL state machine timers .
			a->xtimers = cmSizeMaxTimers(hApp);
			a->ntimers = cmSizeCurTimers(hApp);

			// tpktchans = 4200 bytes
			a->xtpktchans = cmSizeMaxTpktChans(hApp);
			a->ntpktchans = cmSizeCurTpktChans(hApp);

			// channels = 40 bytes
			//A channel refers to the link between a PDL state machine and the network.
			a->xchannels = cmSizeMaxChannels(hApp);
			a->nchannels = cmSizeCurChannels(hApp);

			// messages = 20 bytes
			a->xmessages = cmSizeMaxMessages(hApp);
			a->nmessages = cmSizeCurMessages(hApp);
			//cmSizeCurMessages()

			a->xudpchans = cmSizeMaxUdpChans(hApp);
			a->nudpchans = cmSizeCurUdpChans(hApp);

			// tpktchans = 4200 bytes
			a->xchandescs = cmSizeMaxChanDescs(hApp);
			a->nchandescs = cmSizeCurChanDescs(hApp);
			// vt node count
			cmMeiEnter(hApp);
			a->vtnodecount = pvtCurSize(hVal);
			cmMeiExit(hApp);
		}
	}

	return(0);
}

int
main (int argc, char * argv[])
{
	int		retval = 0;
	char	* penv;
	int		mypid;
	struct 	itimerval polltmr;
	tid 	pmtid;
	struct 	itimerval licensetmr;
	tid 	lmtid = 0;
	extern	int checkMapReady; 
	extern	void	start_ispd(void);
	char 	*basedir;
	TimerNotifyCBData *tcbdata;

	// Set up default logging before we read configs
	NetSyslogOpen("gis", NETLOG_ASYNC);

	srand48(time(0));

	/* Parse command line arguments */
	IpcParse (argc, argv);

	myConfigServerType = CONFIG_SERPLEX_GIS;

	/* To get rid of shared Memory dependancy. 
	This structure is used temporarily till the shared Memory is created.
	*/
	memset (&tmpLsMem, 0, sizeof(LsMemStruct));
	lsMem = &tmpLsMem;

	/* Ensure iServer is not running. 
	*  We cannot store the pid here however - since we might daemonize 
	*/
	sprintf(pidfile, "%s/%s", PIDS_DIRECTORY, GIS_PID_FILE);

	mypid = ReadPid(pidfile);

	if ((mypid > 0) && (kill(mypid, 0) == 0))
	{
		NETERROR(MINIT, ("%s seems to be running already\n", argv[0]));
		fprintf(stderr, "%s seems to be running already\n", argv[0]);
		exit(0);
	}

    /* Change the directory to the DEF_MSW_BASE_DIR */
    if ( ((basedir = getenv("SERPLEXPATH")) || (basedir = DEF_MSW_BASE_DIR) ) &&
            chdir(basedir) ) {
		NETERROR(MINIT, ("Error changing to dir - %s: %s\n", basedir, 
			strerror(errno)));
        fprintf(stderr, "Error changing to dir - %s: %s\ngis exiting ...", 
			basedir, strerror(errno));
		exit(1);
    }

	/* 
		Set the config file path if SERPLEXPATH is defined. 
		This can also be modified with the -f switch given in 
		the command line. This is tested in IpcParse.
	*/
	setConfigFile();

	memset(&localConfig, 0, sizeof(Config));

	ifihead = initIfs();

	iServerIP = getLocalIf(ifihead);

	if (license_init() )
	{
		NETERROR(MINIT, ("Could not obtain valid license\n"));
		system("/usr/local/nextone/bin/iserver all stop");
		exit(-1);
	}
	if (nlm_getInitTime() == 0)
	{
		nlm_setInitTime(time(NULL));
	}

	/* If the env variable exists and a +ve number..,
	 * set debug to that value.
	 */
	penv = (char *)getenv ("ALOID_DEBUG");

	if ((penv && (atoi(penv) > 0)) )
	{
		debug = atoi(penv);
	}

	// Setup dummy functions to record debug information
	// in the beginning
	_msAdd = _dummymsAdd;
	_msSetDebugLevel = _dummymsSetDebugLevel;
	_sipSetTraceLevel = _dummysipSetTraceLevel;

	DoConfig(ProcessConfig);


	if (idaemon)
	{
    	(void) freopen( "/dev/null", "r", stdin );
    	(void) freopen( "/var/log/iserverout.log", "a", stdout );
    	(void) freopen( "/var/log/iservererr.log", "a", stderr );

		daemonize ();
	}

	/* Init udp client and the keep alive message structure for poll */
	Initpoll(GIS_ID,SERPLEX_GID);
	
	/* Get Some lease of life from pm */
	Sendpoll(0);

	// Setup process wide signal handling

	SignalInit();

	/* Initialize the fd list */
	/* 4 times doesn't seem to be enough - set it to 8 times */
	maxfds = GisSetMaxFds(lsMem->maxCalls*8+100);

	NetFdsSetMax(maxfds);
	
	seliSetMaxDescs(maxfds);

	NetFdsInit(&lsnetfds);
	NetFdsInit(&lstimerfds);
	NetFdsInit(&lsqfds);

	initCallId(iServerIP);
#ifndef NETOID_LINUX
	if (nprocs < 2)
	{
		ThreadSetUniprocessorMode();
	}
#endif
	StorePid(pidfile);

	NETDEBUG(MINIT, NETLOG_DEBUG4, ("Creating the shared memory .... \n"));

	checkMapReady = 0; // disable check for us...
	if (SHM_Init(ISERVER_CACHE_INDEX) < 0)
	{
		NETERROR(MINIT, ("SHM_Init failed\n"));
		exit(-1);
	}

	// ONLY the gis or the cli can mark the shared memory
	// as ready. The jServer cannot do that, so we must handle
	// it here
	if (segowner != CONFIG_SERPLEX_CLI)
	{
		checkMapReady = 0; // disable check for us...
	}

	if (segowner != myConfigServerType)
	{
		while (CacheAttach() < 0)
		{
			NETDEBUG(MINIT, NETLOG_DEBUG4,
				("CacheAttach failed\n"));

			Sendpoll(0);

			sleep(1);
		}
	}
	else
	{
		if (CacheInit() < 0)
		{
			NETERROR(MINIT, ("CacheInit failed\n"));
			exit(-1);
		}
	}

	// Initialize the local memory portion of lsMem
	LsMemStructInitLocal(lsMem);
	
	NetH323FdsInit();
#ifndef NETOID_LINUX
	ThreadInitRT();
#endif
	/* The license initialization is beling done twice to get around a
	*  shared memory dependancy
	*/

	if (license_init() )
	{
		NETERROR(MINIT, ("Error getting license \n"));
		system("/usr/local/nextone/bin/iserver all stop");
		exit(-1);
	}

	// CODE which can cause blocks

	/* Get Some lease of life from pm */
	Sendpoll(0);

	// We must launch the timer thread here, as
	// initialization might be a long time.
	// THis enables pm polls
	timerLibInit();

	timerInit(&localConfig.timerPrivate, 1256, 0);

	/* Launch the health thread */
	ThreadLaunch(HandlePollPM, NULL, 1);

	// CODE which will not cause blocks

	tcbdata = TimerNotifyCBDataAlloc();
	InitPipe(tcbdata->notifyPipe);

	/* Add the callback */
	setTimerNotifyCb(&localConfig.timerPrivate, IServerNotify, tcbdata);

	/* Open a notification pipe */
	NetFdsAdd(&lstimerfds, tcbdata->notifyPipe[NOTIFY_READ], FD_READ, 
		(NetFn) HandleNotify, (NetFn) NULL,
		tcbdata, NULL);

	/* Launch the timer thread */
	ThreadLaunch(IpcHandleTimers, NULL, 1);
	ThreadLaunch(IcmpdInit, NULL, 1);

	if ((segowner == CONFIG_SERPLEX_JSERVER) ||
		(segowner == myConfigServerType))
	{
		// IDEALLY this should be done inside whoever is the
		// segowner, but we dont want the jServer to do this
		// as it may be slow, pm may kill it
		LsMemPopulate(lsMem);
	}

	sipStats = lsMem->sipStats;
	shmH323PollQ = &lsMem->callStats->h323PollQ;

	InitCfgParms(lsMem->cfgParms);

	ShmSetReady();

	// basic initialization for stack must be done for
	// logging initialization. However port opening and callback
	// install phase will be postponed until shared memory and
	// other resources are completely initialized

	GisExecuteCmd(GIS_REALM_OPEN_ALL_CMDID);

	// Only after licensing
	if(fceEnabled())
	{
		FCEStart();
	}
	else {
		NETDEBUG(MLMGR,NETLOG_DEBUG4, ("FCE Not Licensed\n"));
	}

	if(h323Enabled())
	{
		// if (strlen(sipdomain)) liSetVHostName(sipdomain); - must change this
		initWaitSdEntry(maxfds);
		uh323init = UH323Init();
	}
	else {
		NETDEBUG(MLMGR,NETLOG_DEBUG4, ("H323 Functionality Not Licensed\n"));
	}

	/* Initialize SIP stack */
	if(sipEnabled())
	{
		char hname[256];
		struct hostent *hentp;

		gethostname(hname, sizeof(hname));
		hentp = gethostbyname(hname);
		if (hentp)
		{
			nx_strlcpy(sipdomain, hentp->h_name, SIPDOMAIN_LEN);
		}

		SSIPInit();
	}
	else {
		sipAdminStatus = 0;
		NETDEBUG(MLMGR,NETLOG_DEBUG4, ("SIP Functionality Not Licensed\n"));
	}

    if (GisQInit() < 0)
	{
		NETERROR(MINIT, ("GisCliQ init failed\n"));
		exit(-1);
	}
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	CdrInit();

	// configure any cdrs
	CdrCfg();

	identMain=mainThread;

	lm_ChkExpiry(NULL);

	CliDInit(CLID_PORT);

	DbInit();

	callStatsInit();

	SCC_Init();
	BridgeInit();
	SipUAInit();
	iwfInit();
	/* Initialize Radius Client */
	if(radiusEnabled())
	{
		if(startRadiusClient())
		{
			NETERROR(MINIT, ("Error initialising radius \n"));
			system("/usr/local/nextone/bin/iserver all stop");
			exit(-1);
		}
	}

	if (h323Enabled() && (uh323init >= 0))
	{
		UH323InitCont();
		NETDEBUG(MINIT, NETLOG_DEBUG4, ("h245 = %d\n",routeH245));
	}

	if(sipEnabled())
	{
		SSIPInitCont(SIP_PORT);
		ThreadLaunch(SipRegStart, NULL, 1);

	}

	//Initialize the lsage thread
	LsAgeInit();

	// Configure mapping and hunting for ISDN/SIP codes
	CodeMapConfig();

	// Initialize phone number file information list.
	ANIFileList = listInit();

	/* initialize firewall control thread */
	NETINFOMSG(MDEF, ("*** NexTone GIS Server started ***\n"));
#ifndef NETOID_LINUX
	ThreadSetRT();
#endif
	SET_STATUS(lsMem, STATUS_ALL_INIT);  // initialization done

	/* Start ispd processing */
#ifndef NETOID_LINUX // temporarily removing till we fix PD
	start_ispd();
#endif 
	// Launch the replication process if necessary
	ThreadLaunch(SCM_Main, NULL, 1);

	if (h323Enabled() && (uh323init >= 0)) 
	{
		int *mainInstance, i;

		/* All H.323 stuff happens in the main thread */
		/* Launch a thread for all iServer activities.
		 * This includes SIP for now
		 */

		// Create the H.323 initialization semaphore
		if (sm_create(IPC_PRIVATE, 1, 0, &h323initsem))
		{
			NETERROR(MH323, ("H.323 sem init failed\n"));
			return -1;
		}

		if (sm_create(IPC_PRIVATE, nh323Instances-1, 0, &h323waitsem))
		{
			NETERROR(MH323, ("H.323 sem init failed\n"));
			return -1;
		}

		for (i=1; i<nh323Instances; i++)
		{
			sm_p(h323waitsem, 0, 0);
		}

		ThreadLaunch((void *(*)(void*))IpcMainLoop, NULL, 1);
		IpcH323LaunchLoops();

		mainInstance = (int *)malloc(sizeof(int));
		*mainInstance = 0;
		IpcH323MainLoop(mainInstance);
	}
	else {
		IpcMainLoop();
	}

	/* Should never get here */
	IpcTerminate ();

	return 0;
}

int
NetH323FdsInit(void)
{
	int i;

	lsh323fds = (NetFds *)malloc(nH323Threads*sizeof(NetFds));

	for (i=0; i<nH323Threads; i++)
	{
		NetFdsInit(&lsh323fds[i]);
	}

	// Initialize the timers also
	h323timerPrivate = (TimerPrivate *)malloc(nH323Threads*sizeof(TimerPrivate));

	for (i=0; i<nH323Threads; i++)
	{
		timerInit(&h323timerPrivate[i], 1256, 0);
	}

	if (nH323Threads > 0)
	{
		h323Threads = (pthread_t *)malloc(sizeof(pthread_t)*(nH323Threads));
		for (i=0; i<nH323Threads; i++)
		{
			h323Threads[i] = (pthread_t)-1;
		}
	}

	return 0;
}

static int
IpcInit (void)
{

#ifdef support_1000s

	/* Open socket and listen for commands */
	if (InitNetPort () < 0)
	{
		fprintf (stderr, "Unable to initialize network port \n");
		return -1;
	}

	NetFdsAdd(&lsnetfds, localConfig.netfd, FD_READ, 
		(NetFn) HandleCentrex, (NetFn) NULL,
		NULL, NULL);

	/* For backward compatibility */
	NetFdsAdd(&lsnetfds, localConfig.vpnsfd, FD_READ, 
		(NetFn) HandleCentrex, (NetFn) NULL,
		NULL, NULL);

#ifdef _ALLOW_REDUNDANCY_
	NetFdsAdd(&lsnetfds, localConfig.redundsfd, FD_READ, 
		(NetFn) RedundsConnReceive, (NetFn) NULL,
		NULL, NULL);
#endif
#endif

	printf ("%s %s.%s, %s\n%s\n",
		GIS_NAME,
		GIS_VERSION,
		GIS_MINOR,
		GIS_BUILDDATE,
		GIS_COPYRIGHT);

	printf("\n");

	fflush(stdout);

	return 0;
}

static int
InitNetPort (void)
{
	int	retval = 0, i = 1;
	struct sockaddr_in myaddr;
	struct hostent * hent = 0x0;
	int nbio = 1;
	int flags;

	/* Create Socket */
	localConfig.netfd = socket (AF_INET, SOCK_STREAM, 0 );

	if (localConfig.netfd < 0)
	{
		perror ("Unable to create stream socket ");
		return -1;
	}

	GisSetSockOpts(localConfig.netfd);

	/* Make the listening socket non blocking. This
	 * is a denial of service attack fix, when the client
	 * closes the connection before the server does the
	 * accept, leading accept to block (Stevens)
	 */

	if((flags = fcntl(localConfig.netfd,F_GETFL,0)) <0)
	{
		 perror("netfds fcntl");
		 return -1;
	}	
	flags |= O_NONBLOCK;

	if((fcntl(localConfig.netfd,F_SETFL,flags)) <0)
	{
		 perror("netfds fcntl");
		 return -1;
	}

	bzero ((char *)&myaddr, sizeof(myaddr));  /*  Zeroes the struct */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (IPC_MGR_PORT_NUMBER);
	myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

	/* Bind */
	retval = bind (localConfig.netfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	if ( retval < 0 )
	{
		perror ("Unable to bind socket netfd");
		return -1;
	}

	/* Listen */
	retval = listen (localConfig.netfd, 10);
	if ( retval < 0 )
	{
		perror ("Unable to listen on socket");
		return -1;
	}
	
	/* Create Socket */
	localConfig.vpnsfd = socket (AF_INET, SOCK_STREAM, 0 );

	if (localConfig.vpnsfd < 0)
	{
		perror ("Unable to create stream socket ");
		return -1;
	}

	GisSetSockOpts(localConfig.vpnsfd);

	/* Make the listening socket non blocking. This
	 * is a denial of service attack fix, when the client
	 * closes the connection before the server does the
	 * accept, leading accept to block (Stevens)
	 */
	if (ioctl(localConfig.vpnsfd, FIONBIO, &nbio) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	bzero ((char *)&myaddr, sizeof(myaddr));  /*  Zeroes the struct */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (VPNS_LOOKUP_PORT_NUMBER);
	myaddr.sin_addr.s_addr  = htonl (INADDR_ANY);

	/* Bind */
	retval = bind (localConfig.vpnsfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	
	if ( retval < 0 )
	{
		perror ("Unable to bind socket vpnsfd");
		return -1;
	}

	/* Listen */
	retval = listen (localConfig.vpnsfd, 10);
	if ( retval < 0 )
	{
		perror ("Unable to listen on socket");
		return -1;
	}
	
	return (retval);
}

int IpcParse (int argc, char * argv[] )
{
	int	i = argc;
	char *progname = argv[0];

	argv ++;
	argc --;

	while (argc > 0)
	{
		if (argv[0][0] == '-')
		{
			switch (argv[0][1])
			{
			case 'v':
				fprintf (stdout, "\n");
				fprintf (stdout, "%s %s.%s, %s\n%s\n",
					GIS_NAME,
					GIS_VERSION,
					GIS_MINOR,
					GIS_BUILDDATE,
					GIS_COPYRIGHT);
				fprintf (stdout, "\n");

				/* NOTE that this exits */
				exit (0);

				break;
			
			case 'f':
				strcpy(config_file, argv[1]);
		
				argc --;
				argv ++;
				
				break;
			}
		}

		argc --;
		argv ++;
	}

	return 0;
}

int
IpcTerminate (void)
{
	if (localConfig.netfd)
		close (localConfig.netfd);

	return 0;
}

int
InitPipe(int notifyPipe[])
{
	int flags;

	/* Open notification pipe */
	if (pipe(notifyPipe) < 0)
	{
		perror("Unable to open pipe");
		return -1;
	}

	if((flags = fcntl(notifyPipe[NOTIFY_READ],F_GETFL,0)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(notifyPipe[NOTIFY_READ],F_SETFL,flags)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	if((flags = fcntl(notifyPipe[NOTIFY_WRITE],F_GETFL,0)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(notifyPipe[NOTIFY_WRITE],F_SETFL,flags)) <0)
	{
	 	perror("notify fcntl");
	 	return -1;
	}

	return 0;
}

#if 0
int
HandleCentrex(int fd, FD_MODE rw, void *data)
{
	struct sockaddr_in	client;
	int	clilen;
	int	newsockfd;
	pid_t	pid;
	int 	nbio = 1;
	int 	flags;
#if 0

	clilen = sizeof(client);
	bzero (&client, sizeof(client));

	/* Tell the child where the request came in from.
	 * For backward compatibility with 1.0 clients
	 */
	if (fd == localConfig.netfd)
	{
		isLusReq = 1;
		isVpnsReq = 0;
	}
	else if (fd == localConfig.vpnsfd)
	{
		isLusReq = 0;
		isVpnsReq = 1;
	}

	newsockfd = accept(fd, (struct sockaddr *) &client, 
		&clilen);

	if (newsockfd <0)
	{
		perror("accept!!");
		return -1;
	}

	GisSetSockOpts(newsockfd);

	/* Make this socket blocking */
	if (ioctl(newsockfd, FIONBIO, &nbio) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	if ((flags = fcntl(newsockfd, F_GETFL, 0)) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

	/* 
	 * Used blocking pre 1.2
	 * Now do non-blocking.
	 * flags &= ~(O_NONBLOCK|O_NDELAY);
	 */

	flags |= (O_NONBLOCK|O_NDELAY);
	if (fcntl(newsockfd, F_SETFL, flags) < 0)
	{
		perror("Failed to make socket non-blocking\n");
	}

#ifdef USE_FORK
	NETDEBUG(MSEL, NETLOG_DEBUG1,
		 ("Accepting connection...Spawning\n"));

	if ((pid = fork()) == 0)
	{
		/* In general  we have to close all open ones */
		close(localConfig.netfd);

		spthread_init();

		IpcChildLoop(newsockfd, &client);

		close (newsockfd);

		spthread_exit();

		NETDEBUG(MSEL, NETLOG_DEBUG4,
			("child %d Terminating... \n", getpid()));
		exit(0);
	}
	else
	{
		NETDEBUG(MSEL, NETLOG_DEBUG1,
			("Spawned off pid %d\n", pid));
		close(newsockfd);
	}
#else /* USE_FORK */

	/* Add the remote to our list of handles */
	GisAddRemote(gisActiveCHandles, newsockfd, &client);
				
#endif /* USE_FORK */
#endif
	return(0);
}

int
HandleRemoteClient(int fd, FD_MODE rw, void *data)
{
	 char fn[] = "HandleRemoteClient():";
	 ClientHandle *handle = (ClientHandle *)data;

	 if (handle == NULL)
	 {
		  NETERROR(MSEL, ("%s handle is NULL\n", fn));
		  return -1;
	 }

	 if (ProcessData (handle, &handle->client) != 0)
	 {
		  NETDEBUG(MSEL, NETLOG_DEBUG1,
				("%s Terminating Connection with client \n", fn));
		  GisDisableCHandle(handle);
		  GisDeleteCHandle(handle);
	 }

	 /* Refresh the connection */
	 time(&handle->rtime);
	 return(0);

}

int
PktLen(char *buf)
{
	PktHeader *hdr = (PktHeader *)buf;

	NETDEBUG(MPKT, NETLOG_DEBUG4,
		("Receving header of packet type %d\n",
			ntohl(hdr->type)));

	return (ntohl(hdr->totalLen));
}

int 

WriteCallback(int sockfd,
	      PktOpaque *ghdr,
	      int ghdrlen,
	      char *payload,
	      int plen,
	      int type)
{
#if 0
     struct iovec bufs[2];
     PefHeader *pef;

     /* First figure out looking at the header, if we have to do
      * any post-processing of the payload.
      */
     if (ghdr && ghdrlen)
     {
	  /* We do have a header, which is in addition to the payload
	   */
	  switch (ghdr->hdr->type)
	  {
	  case PKT_PEF:
	  {
	       PefHeader *pef = (PefHeader *)(ghdr->hdr);
	       SA *sa = ghdr->sa;
	       pef->next_payload = type;
	       ghdrlen = sizeof(PefHeader);

	       if (PEF10_GetEncryption(pef))
	       {
		    /* Now encrypt the packet... */
		    PEF10_EncryptPayload(sa, payload, plen);
	       }

	       htonl_pefhdr(pef);
	  }
	  break;
	  default:
	       break;
	  }
     }

     bufs[0].iov_base = (char *)((ghdr) ? ghdr->hdr : 0);
     bufs[0].iov_len = ghdrlen;
     bufs[1].iov_base = payload;
     bufs[1].iov_len = plen;

     if (writev(sockfd, bufs, 2) < 0)
     {
	  printf("WRITEV call failed...\n");
     }
#endif
	if (send(sockfd, payload, plen, 0) != plen)
	{
		NETERROR(MDEF, ("WriteCallback: send failed error %d\n",
				errno));
	}
     return 1;
}

/* Just handle the encapsulations, and transfer control
 * to lower layer, if necessary. All packet replies
 * to be sent back, by this layer
 */
int
ProcessData (ClientHandle *chandle, struct sockaddr_in *client)
{
	 int 		sockfd = CHUCCHandle(chandle)->fd;
	 char 		*buffer = (char *)&CHUCCHandle(chandle)->data_in[0];
	 char 		*start, *end;
	 PktHeader 	*pkt_hdr;
	 Pkt		*data_pkt, reply_pkt;
	 int		retval = 1;
	 int     	read_bytes = 0;
	 int 		type;
	 sigset_t 	o_signal_mask, n_signal_mask;
	 PefHeader 	*pef;
	 PktOpaque 	opaque;
	 struct sockaddr_in me;
         char buf[INET_ADDRSTRLEN];
	 int 		addrlen = sizeof(struct sockaddr_in);

	bzero (&reply_pkt, sizeof(Pkt));

	NETDEBUG(MSEL, NETLOG_DEBUG2, ("Before read from %s\n", 
		inet_ntop( AF_INET, &client->sin_addr, buf, INET_ADDRSTRLEN)));	

	/* Read from ephemeral socket */
	start = end = (char *)&buffer[0];
	read_bytes = retval = get_next_packet(sockfd, buffer, &start, &end, PktLen, 
			sizeof(GPktHeader), LS_READ_SINGLESHOT);

	NETDEBUG(MSEL, NETLOG_DEBUG2, ("After reading %d bytes from %s\n", 
		retval, inet_ntop( AF_INET, &client->sin_addr, buf, INET_ADDRSTRLEN)));	

	if (retval <= 0)
	{
	     /* We dont know now, where we are in the stream... */
	     retval = 1;	/* terminate... */
	     goto _do_nothing;
	}

	if (getpeername(sockfd, (struct sockaddr *)&me, &addrlen) < 0)
	{
		int lerrno = errno;

		NETERROR(	MSEL,
			("ProcessData(): getpeername failed for sock %d - errno %d - %s\n",
			sockfd,
			lerrno,
			strerror(lerrno) ) );

		NETERROR(MSEL,
			("Local Interface is not bound, ip address is 0!\n"));	
	}

	/* We have a valid PktHeader now,
	 * but we will typecast both quantities.
	 */
	
	pkt_hdr = (PktHeader *)buffer;
	data_pkt = (Pkt *)buffer;

	type = ntohl(pkt_hdr->type);

	switch (type)
	{
	case PKT_REGISTER:
	case PKT_PROXYREGISTER:
	case PKT_FIND:
	case PKT_PROXY:
	case PKT_UNREGISTER:
	case PKT_REDIRECT:
	case PKT_DND:
	case PKT_PROFILE:
	case PKT_SERVERMSG:
	case PKT_NEXTIME:
	     retval = ProcessPayloadData(chandle, (char *)data_pkt, 0, 0,
					 WriteCallback);

	     if (retval == 1)
	     {
		  /* Its our job to transmit the data now... */
	     }

	     break;
	case PKT_XML:
	  NETDEBUG(MRED, NETLOG_DEBUG4,
			   ("ProcessRedundsPacket: Received XML\n"));

	  ntohPkt(type, (Pkt *)buffer);

	  ProcessXMLEncoded(sockfd, buffer, NULL,
						0, 0, 0);

	  retval = 0;
	  break; 
	default:
	     fprintf (stderr, 
		      "Unhandled case [0x%x] from %s/%d -- not implemented yet \n", 
		      type, ULIPtostring(ntohl(me.sin_addr.s_addr)),
			ntohs(me.sin_port));
	     /* Reflect this packet back with the type changed to error */
	     data_pkt->type = htonl(PKT_ERROR);
	     PktSend(sockfd, data_pkt);
	     
	     break;
	}
	
 _do_nothing:
	return (retval);
}

int
ProcessPayloadData (ClientHandle *chandle, char *data_in, 
			void *opaque,
		    int opaquelen,
		    int (*writecb)())
{
	 int 		sockfd = CHUCCHandle(chandle)->fd;
	 long 		buffer[LS_READ_SINGLESHOT/sizeof(long)+1];
	 char 		*start, *end;
	 PktHeader 	*pkt_hdr;
	 Pkt		*data_pkt, reply_pkt;
	 int		retval = 1;
	 int     	read_bytes = 0;
	 int 		type;
	 sigset_t 	o_signal_mask, n_signal_mask;
	 struct sockaddr_in me;
	 int 		addrlen = sizeof(struct sockaddr_in);

	memset(&reply_pkt, 0, sizeof(Pkt));

	/* We have a valid PktHeader now,
	 * but we will typecast both quantities.
	 */
	
	if (getpeername(sockfd, (struct sockaddr *)&me, &addrlen) < 0)
	{
		int lerrno = errno;

		NETERROR(	MSEL,
			("ProcessPayloadData(): getpeername failed for sock %d - errno %d - %s\n",
			sockfd,
			lerrno,
			strerror(lerrno) ) );

		NETERROR(MSEL,
			("Local Interface is not bound, ip address is 0!\n"));	
	}

	pkt_hdr = (PktHeader *)data_in;
	data_pkt = (Pkt *)data_in;	
	type = ntohl(pkt_hdr->type);

	ntohPkt(type, data_pkt);

	switch (type)
	{
	case PKT_REGISTER:
	case PKT_PROXYREGISTER:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_REGISTER\n"));
	     
	     ProcessRegister (sockfd, data_pkt, data_pkt, 
			      opaque, opaquelen, writecb);
	     retval = 0;
	     break;
	     
	case PKT_FIND:
	     /* Look up the AL table to find the destination point IP Addr
	      * using the destination point Phone Number 
	      */
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_FIND \n"));

	     ProcessFindPhone (chandle, data_pkt, data_pkt,
			  opaque, opaquelen, writecb);			  
	     retval = 0;
	     break;
	     
	case PKT_UNREGISTER:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_UNREGISTER \n"));

	     ProcessUnregister (sockfd, data_pkt, &reply_pkt,
				opaque, opaquelen, writecb);
	     
	     break;
	     
	case PKT_REDIRECT:
	case PKT_PROXY:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_REDIRECT/PROXY \n"));

	     ProcessRedirect (sockfd, data_pkt, &reply_pkt,
			      opaque, opaquelen, writecb);			      
	     
	     break;
	     
	case PKT_DND:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_DND \n"));

	     ProcessDND (sockfd, data_pkt, &reply_pkt,
			 opaque, opaquelen, writecb);			 
	     
	     break;
	     
	case PKT_PROFILE:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_PROFILE \n"));
		if (routecall != 1)
	     ProcessProfile(sockfd, data_pkt, &reply_pkt,
			    opaque, opaquelen, writecb);			    
	     break;
	     
	case PKT_SERVERMSG:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_QUERYCLIENTS \n"));

	     ProcessServerMsg (sockfd, data_pkt, &reply_pkt,
			       opaque, opaquelen, writecb);			       
	     retval = 0;
	     break;
	     
	case PKT_NEXTIME:
	     NETDEBUG(MPKT, NETLOG_DEBUG1, ("Recd. PKT_NEXTIME \n"));
	     ProcessNexTime(sockfd, data_pkt, data_pkt,
				opaque, opaquelen, writecb);
	     retval = 0;
	     break;

	default:
		NETERROR(MPKT,
		      ("Unhandled case [0x%x] from %s/%d -- not implemented yet \n", 
		      type, ULIPtostring(ntohl(me.sin_addr.s_addr)),
			ntohs(me.sin_port)));
	     	/* Reflect this packet back with the type changed to error */
	     	data_pkt->type = PKT_ERROR;
		htonPkt(PKT_ERROR, data_pkt);	
	     	PktSend(sockfd, data_pkt);
	     
	    	break;
	}
	
 _do_nothing:
	return (retval);
}
#endif

/**
 * this method is called when a reconfig request is received by gis
 *
 * only the ./iserver all reconfig command gets here
 */
static int
ProcessReconfig (void)
{
	CdrCfg();

	/* reconfigure firewall controll thread */
	FCEReconfig();

	if (iserverPrimary)
	{
		GisExecuteCmd(GIS_REALM_UP_ALL_CMDID);
	}

	InitCfgParms(lsMem->cfgParms);
	if (h323Enabled())
	{
		UH323Reconfig();
	}

	// Reconfigure mapping and hunting for ISDN/SIP codes
	CodeMapReconfig();

	return(0);
}

/**
 * this method is called during startup and during the reconfig request
 */
static int
ProcessConfig(void)
{
	int match = -1;
	static int firstTime = 1;

	/* Process Configuration, read from the config file */
	match = FindServerConfig();

	if (match == -1)
	{
		fprintf(stderr, "Not Configured to run...\n");
		exit(0);
	}

	iserver = &serplexes[match];

	if (iserver == NULL)
	{
		fprintf(stderr, "Iserver Index Invalid %d\n", match);
		exit(0);
	}

	/* Set the priority */
#ifdef print_priority
	fprintf(stderr, "setting priority to %d\n",  serplexes[match].prio);
#endif
	setpriority(PRIO_PROCESS, 0, serplexes[match].prio);

	if (serplexes[match].max_endpts > 0)
	{
		max_endpts = serplexes[match].max_endpts;
	}

	if (serplexes[match].max_gws > 0)
	{
		max_gws = serplexes[match].max_gws;
	}

	idaemon = serplexes[match].daemonize;
	if (serplexes[match].threadstack > 4)
	{
		threadstack = serplexes[match].threadstack;
	}

	ServerSetLogging("gis", &serplexes[match]);

	if (xthreads == 0)
	{
		NETDEBUG(MINIT, NETLOG_DEBUG4, ("threads disabled\n"));
	}
	else
	{
		NETDEBUG(MINIT, NETLOG_DEBUG4, ("threads enabled\n"));
	}

#ifdef _ALLOW_REDUNDANCY_
	RedundsCfg();
#endif

	/* Read the timeout */
	cacheTimeout = serplexes[match].age.cache_timeout;	

	/* is this a reconfig request or the initial startup time request? */
	if (firstTime == 1)
	   firstTime = 0;
	else
	   ProcessReconfig();
	return(0);
}

int lm_ChkExpiry(struct Timer *lmtimer)
{
        static char fn[] = "lm_ChkExpiry";
        time_t  now;
        double  diff;
        struct  itimerval licensetmr;
		tid lmtid;

		if (lmtimer)
		{
			timerFreeHandle(lmtimer);
		}

        /* if timeless license don't set any timer */
        if(lsMem->expiry_time == 0)
                return 0;

        now = time(&now);
        diff = difftime(lsMem->expiry_time,now);
        DEBUG(MLMGR,NETLOG_DEBUG4,("%s:License expiry at %s.Current time = %s",
                fn,ctime(&lsMem->expiry_time),ctime(&now)));
        if(diff<0)
        {
                NETERROR(MLMGR,("License expired at %s. Current time =  %s",
                        ctime(&lsMem->expiry_time),ctime(&now)));
                system("/usr/local/nextone/bin/iserver all stop");
                exit(-1);
        }

        memset(&licensetmr, 0, sizeof(struct itimerval));

	/* Use Value TIMERS, if we dont want to DELETE
	 * when timer fires, and we want to ADD.
	 */
        if(diff > LONG_MAX)
                licensetmr.it_value.tv_sec = LONG_MAX;
        else
                licensetmr.it_value.tv_sec = diff;

        lmtid = timerAddToList(&localConfig.timerPrivate, &licensetmr,
                1,PSOS_TIMER_REL, "LMTMR", lm_ChkExpiry, NULL);

        if(!lmtid)
        {
                NETERROR(MLMGR,("lm_ChkExpiry Failed to set the timer\n"));
                return -1;
        }

        return 0;
}

sigset_t 			async_signal_mask;
struct sigaction	sigact;
stack_t				s;

//
//	Function :
//		SignalInit()
//
//	Description :
//		Setup process wide signal handling, adhered to
//		by all threads via sigaction(2). This routine
//		should be called very early in the startup
//		process after daemonizing and prior to starting
//		any threads. In this way the blocked signal mask
//		will be inheritted by any threads.
//
static void
SignalInit(void)
{

	// Setup up alternate signal stack to be 
	// used by syncronous signal handler

	s.ss_sp = malloc( SIGSTKSZ );
	s.ss_size = SIGSTKSZ;
	s.ss_flags = 0;

	if ( sigaltstack( &s, 0 ) == -1 )
		perror( "sigaltstack" );

	// Create mask for blocking asyncronous signals
	// They must be blocked for sigwait() to work!!

	sigemptyset( &async_signal_mask );

	sigaddset( &async_signal_mask, SIGTERM );
	sigaddset( &async_signal_mask, SIGPOLL );
	sigaddset( &async_signal_mask, SIGCHLD );
	sigaddset( &async_signal_mask, SIGINT );
	sigaddset( &async_signal_mask, SIGHUP );
	sigaddset( &async_signal_mask, SIGALRM );

	// Setup SIGCHLD special case

	sigemptyset( &sigact.sa_mask );
	sigact.sa_flags = (SA_RESTART|SA_NOCLDSTOP|SA_ONSTACK);
	sigact.sa_handler = SIG_IGN;

	sigaction( SIGCHLD, &sigact, NULL );

	sigset( SIGCHLD, DummySigHandler );

	// Setup async signals to be ignored

	sigact.sa_flags = (SA_RESTART|SA_ONSTACK);

	sigaction( SIGPIPE, &sigact, NULL );
	sigaction( SIGWINCH, &sigact, NULL );
	sigaction( SIGTTIN,	&sigact, NULL );
	sigaction( SIGTTOU, &sigact, NULL );

	// Setup Signal handler to be called for
	// syncronous signals (fatal traps)

	sigact.sa_handler = SyncSigHandler;
	sigact.sa_flags |= (SA_RESTART|SA_ONSTACK);

	sigaction(SIGBUS,	&sigact, NULL );
#ifndef NETOID_LINUX
	sigaction(SIGEMT,	&sigact, NULL );
#endif
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );

	// Setup block mask for asyncronous signals to be
	// processed

	pthread_sigmask( SIG_BLOCK, &async_signal_mask, NULL );

	// Launch thread to process asyncronous signals

	ThreadLaunch( AsyncSigHandlerThread, NULL, 1);
}


//
//	Function :
//		AsyncSigHandlerThread()
//
//	Description :
//		This routine is contains logic for a thread
//		to handle asyncronous signals in a syncronous
//		fashion for the gis process. The thread is 
//		spawned at initialization time. sigwait(2)
//		is used to process the signals correctly.
//		The routine is not a signal handler so any
//		function call may be called from it.
//
static void *
AsyncSigHandlerThread( void * args )
{
	char			c_sig[256];
	int				signo;
	extern	void	restart_ispd(void);

	memset( c_sig, (int) 0, 256 );

	for (;;)
	{

		sigwait( &async_signal_mask, &signo );

		nx_sig2str( signo, c_sig , sizeof(c_sig));

		switch (signo)
		{
		case SIGINT:
			sig_int_ls(signo);
			break;

		case SIGTERM:
			sig_term(signo);
			break;

		case SIGCHLD:
			sig_chld(signo);
			break;

		case SIGHUP:
			sig_hup(signo);
			break;

		default:
			NETERROR(	MDEF,
						("Caught %s signal - ignoring\n",
						c_sig ));
			break;
		}
	}
}

//
//	Function :
//		DummySigHandler()
//
//	Description :
//		This routine is a dummy signal catcher used for
//		SIGCHLD signals. (never called)
//
static void
DummySigHandler(int signo)
{
	NETERROR(	MDEF,
						("DummySigHandler() : Caught %d signal\n",
						signo ));
	return;
}

//
//	Function :
//		SyncSigHandler()
//
//	Description :
//		This routine is a signal catcher used for
//		Synchronous signals. (fatal traps)
//
static void
SyncSigHandler(int signo)
{
//	int32_t thread = _lwp_self(); //Replaced with POSIX alternate
	int32_t thread = pthread_self();

	char	c_sig[32];
	int		pid;

	nx_sig2str( signo, c_sig, sizeof(c_sig));

	switch (signo)
	{
	case SIGBUS:
#ifndef NETOID_LINUX
	case SIGEMT:
#endif
	case SIGFPE:
	case SIGILL:
	case SIGSEGV:

	#if 0
		if ((pid = fork()) == 0)
		{
			NETERROR(	MDEF,
						("Caught fatal %s signal in LWP %d\n",
						c_sig,
						thread ));
			exit(0);
		}
		else
	#endif
			abort();
		break;
	default:
		NETERROR(	MDEF,
					("Caught %s signal in LWP %d\n",
					c_sig,
					thread ));
		break;
	}
}

/* index: one of the GIS_REALM_XXX_ALL_CMDID */
int
GisExecuteCmd(unsigned short index) 
{
	int msgqid, rc, selfid;
	char msg[MAX_MSGLEN], cmd[MAX_MSGLEN];
	q_msg *m = (q_msg *)msg;

	selfid = (((getpid() & 0xffff) << 16) | (pthread_self() & 0xffff));

	if ((msgqid = open_execd()) < 0) {
		NETERROR(MINIT, ("Error connecting to execd: %s\n", strerror(errno)));
		return -1;
	}

    //send message
    if ((rc = sys_execd(msgqid, selfid, SRVR_MSG_TYP, (1<<REQ_BIT), 
                    GisCmdStr[index], msg, MAX_MSGLEN)) < 0) {
        NETERROR(MINIT,("Error executing command\n  %s", GisCmdStr[index]));
        return -1;
    }
    return 0;
}
