#include <stdlib.h>
#include <pthread.h>
#include <sys/resource.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <netdb.h>

#include "gis.h"
#include "srvrlog.h"
#include "sgen.h"
#include "mgen.h"
#include <malloc.h>
#include "nxosd.h"
#include <ctype.h>

// Ixia includes
#include "expect.h"
#include "ixia.h"
#include <sys/types.h>
#include <sys/stat.h>

static char version[] = "$Id: sgen.c,v 1.36.2.60.24.1 2005/10/13 14:28:28 smallonee Exp $";

SipStats *sipStats = NULL;	// for linking
int iserverPrimary = 1;		// for sparc build

// External variables
extern long 		hostid;
extern int 			maxcalls; 

// External function declarations
extern void *		IcmpdInit(void *arg);
extern char *		ULIPtostring (unsigned long ipaddress);

// Global variables

// Threads
pthread_t			initgenThread = (pthread_t)-1;
pthread_t	 		inpThread = (pthread_t)-1;
pthread_t			sipThread = (pthread_t)-1;
pthread_t 			monThread = (pthread_t)-1;
pthread_t			timerThread = (pthread_t)-1;
pthread_t			mgen_thread;

// Flags
int 				mode = MODE_RECEIVE;
int 				manualAccept;
int 				fax;
int 				idaemon;
int 				debug;
int				inviteType = INVITE_REGULAR, reInvType ;
int				holdType = HOLD_NONE;
int 				finalRespCode;
int 				automode;
int 				vendorSonusGSX;
int                             asrMode = 0, sipPrivRfcMode = 0, sipPrivDftMode = 0, sipPrivDualMode = 0;
int                             maxCallsMode = 0, retainCalls = 0;
int				termEpMode = 0, reInviteNewSdp = 0;
int                             isupMode = 0, qsigMode = 0;
int				blindXferMode = 0 ,attXferMode = 0 , transferFlag = 0, setModeTransfer = 0, waitForNotify = 0;
int				att_transfer_status = 1, unatt_transfer_status = 1;		//default is success
int 			cm_mode = 0;			//variable indicating that sgen will be running as call manager and will be able to handle call transfers
int				regExpires = 3600;
int				unregWhenExit=0;
tid				regTid;

// Call related variables
Call *				Calls, *CallsOut;
unsigned int			nCalls = 25;
int             		totalMaxCalls;
int 				incCallingPn, incCalledPn;
char 				startCallingPn[256], startCalledPn[256];
char				xferTgtNum[256];
char 				callingpn[256],	calledpn[256];
char 				replaceStr[1024];
char **				callingpna, **calledpna, **callIDIn, **callIDOut;
char				privTag[256];
unsigned int 		tdSetup, tdConnect, tdIdle, tdMonitor;
uint32_t			tdCallDuration;
int 				burst, burstInterval = 100;
float				call_rate;
hrtime_t			callTimePeriod;
char 				otg[256];	
char 				dtg[256];
char 				contact[256];
char *				contactUser, *contactHost;
char *				uriUser, *uriHost;
int 				refresher = SESSION_REFRESHER_NONE;
double 				probability;	// probability of dropping packets
double 				aprobability;	// probability of dropping acks
double 				bprobability;	// probability of dropping byes
double                          iprobability;   // probability of dropping invites
int					readcallingpnfromfile=0;
FILE				*srcfp; //file poiner for source number file
int					readcalledpnfromfile=0;
FILE				*dstfp; //file pointer for destination number file

// IP addresses and ports
unsigned long 		localIP;
unsigned long 		chanLocalAddr;
char 				gwAddr[256];
unsigned short 		chanLocalPort = 49200;
unsigned short 		mediaStartPort = 49200;
unsigned short 		mgenPort = 49160;
unsigned short 		gwPort = 5060;
int 				cport = 5060;
int 				contactPort = 5060, uriPort = 5060;

// Mgen options
char 			mgenPayloadLen[8] = "30";	
char 			mgenPayloadInterval[32] = "30000000"; 
char			mgenNumThreads[8] = "1";
char			mgenTxRxSamePort[4];
char			mgenLoopback[4];
char			*mstatArg = NULL;

// Ixia Structures
ixiaInfo	ixiaGlobal;
char		ixia_testScriptName[IXIA_TESTSCRIPT_NAME_LEN];

// Statistics
int 				nErrors, nFailedSetups;
int                             nFailedInvites = 0;
MgenStats			mgenStats;

// Misc
int 				maxfds = 1024;
int 				inputfd = -1;
int					mgenServerFd = -1;
int					mgenFd = -1;
NetFds 				lsnetfds, lsinfds, lstimerfds;
Lock 				callmutex, transmutex, realmmutex, sipregmutex;
unsigned long 		histid1, histid2, orig_size, current_size;
int 				poolid, lpcid, hpcid;
static 				int nsigints;
static                          int callCounterTx = 0;
CacheRealmEntry	*		mydefaultRealm;
char 				inputfilename[256];
char				mgenFile[256] = "mgen";

// Function declarations
static int 			CheckLicense(void);
static int 			ParseArguments(int argc, char **argv);
static int 			PrintErrorTime(void);
static int			InitCalls(void);
void * 				mgenServerThread(void *arg);
void * 				callgenThread(void *arg);
int					DisconnectCallsWithBurstRate(void *arg);
static void 		SignalInit(void);
static void * 		AsyncSigHandlerThread(void * args);
static void 		SyncSigHandler(int signo);
static void 		DummySigHandler(int signo);
int 				HandleNotify(int fd, FD_MODE rw, void *data);
int					IServerNotify(void *data);
static void			PrintInteractiveCmds(void);
static void 		PrintMgenOptions();
static pid_t		StartMgen();
static int			PrintMgenStats();
static void             	GenerateSeed();
int					getNextPhoneFromFile(char *,FILE *);
int					SendUnregistrations();
static int          SendMgenDtmf(char *signalType, int duration, int volume);
int					sgenInform(SipEventHandle *evb);

int
mainThread()
{
	pthread_t self;

	self = pthread_self();

	return (pthread_equal(timerThread, self) ||
		pthread_equal(inpThread, self) ||
		pthread_equal(initgenThread, self) ||
		pthread_equal(monThread, self) ||
		pthread_equal(sipThread, self));
}

int
GisSetUdpSockOpts(int fd)
{
	int i=1, len;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_REUSEADDR failed errno %d\n",
				errno ));
	}

	len = sizeof(int);
	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
				(char *)&i, &len) < 0)
	{
		NETERROR(MDEF, ("getsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;
	i=262144;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_SNDBUF failed errno %d\n",
				errno ));
	}

	i=65536;
	i=262144;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
				(void *)&i, sizeof(i)) < 0)
	{
		NETERROR(MDEF, ("setsockopt SO_RCVBUF failed errno %d\n",
				errno ));
	}

	return 0;
}

//Function added for Sending Info with Isup Message
static void
SendInfoForSipT()
{
	
	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	char mimeType[5];
	char bodyISUP[17] ={0x01, 0x00, 0x49, 0x00, 0x00, 0x03, 0x02, 0x00, 0x07, 0x04, 0x10, 0x00, 0x33, 0x63, 0x21, 0x43, 0x00};
	char bodyQSIG[27] = {0x08,0x02,0x55,0x55, 0x05, 0x04, 0x02, 0x90, 0x90, 0x18, 0x03, 0xa1, 0x83, 0x01, 0x70, 0x0a, 0x89, 0x31, 0x34, 0x30, 0x38, 0x34, 0x39, 0x35, 0x30, 0x37, 0x32};

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, &sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, (void **)callHandlePtr);

		sipCallHandle = SipCallHandle(callHandle);

		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "SendInfo(): Malformed call handle\n");
		}

		// Setup the appMsgHandle
		appMsgHandle = SipAllocAppMsgHandle();
		appMsgHandle->csmError = 0;
		appMsgHandle->maxForwards = sipmaxforwards;
		appMsgHandle->calledpn = UrlDup(sipCallHandle->callLeg.remote,
									    sipCallCache->malloc);
		appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.local,
									     sipCallCache->malloc);
		memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);

		// Fill in ISUP/QSIG message here
		if (isupMode)
		{
			strcpy(mimeType, "ISUP");
			CreateIsupMessage(appMsgHandle);

		}
		if (qsigMode)
		{
			strcpy(mimeType, "QSIG");
			CreateQsigMessage(appMsgHandle);

		}

        	appMsgHandle->realmInfo = (CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
        	appMsgHandle->realmInfo->rsa = localIP;
        	appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

		// Setup the event handle
		evHandle = SipAllocEventHandle();
		evHandle->type = Sip_eBridgeEvent;
		evHandle->event = Sip_eBridgeInfo;
		evHandle->handle = appMsgHandle;

		SipUAProcessEvent(evHandle);
 
		fprintf(stdout, "Sent %s message for call %d\n", 
				mimeType, callHandle->callNo);
	}
	CacheReleaseLocks(callCache);

}

static void
SendInfo(char *signalType, int duration)
{
	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, &sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, (void **)callHandlePtr);

		sipCallHandle = SipCallHandle(callHandle);

		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "SendInfo(): Malformed call handle\n");
		}

		// Setup the appMsgHandle
		appMsgHandle = SipAllocAppMsgHandle();
		appMsgHandle->csmError = 0;
		appMsgHandle->maxForwards = sipmaxforwards;
		appMsgHandle->calledpn = UrlDup(sipCallHandle->callLeg.remote,
									    sipCallCache->malloc);
		appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.local,
									     sipCallCache->malloc);
		memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);

		appMsgHandle->ndtmf = 1;
		appMsgHandle->dtmf = (DTMFParams *) calloc (1, sizeof(DTMFParams));
		appMsgHandle->dtmf->sig = signalType[0];
		appMsgHandle->dtmf->duration = duration;

        appMsgHandle->realmInfo = (CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
        appMsgHandle->realmInfo->rsa = localIP;
        appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

		// Setup the event handle
		evHandle = SipAllocEventHandle();
		evHandle->type = Sip_eBridgeEvent;
		evHandle->event = Sip_eBridgeInfo;
		evHandle->handle = appMsgHandle;

		SipUAProcessEvent(evHandle);
 
		fprintf(stdout, "Sent signal DTMF for call %d : '%c', %dms\n", 
				callHandle->callNo, signalType[0], duration);
	}

	CacheReleaseLocks(callCache);
}

int
SendBlindXfer()
{
	// send hold reinvite and then refer
	SendReinvite(REINVITE_TYPE_HOLD);
	sleep (3);
	SendReferMessage(BLIND_XFER);
}

int
SendAttendedXfer()
{
	int i;

	SendReinvite(REINVITE_TYPE_HOLD);
	nCalls++;
	sleep(3);
	//i = SgenGetFreeCallNo(1);
	SpawnOutgoingCall(1);
	fprintf(stdout, "Sent att call transfer.\n");
	fprintf(stdout, ">\n");
	sleep(3);
}

int
SendReferMessage(int referType)
{

	CallHandle *callHandle, *newCallHandle;
	SipCallHandle *sipCallHandle, *newSipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	int len = 0, max = 1024;
	char callidstr[128] = {0};
    char *tmp_callid;

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	
	callHandle = CacheGet(callCache, callIDOut[0]);
	if (callHandle == NULL)
	{
		CacheReleaseLocks(callCache);
		return 0;
	}

	sipCallHandle = SipCallHandle(callHandle);
	
	if(referType == ATTENDED_XFER)
	{
		newCallHandle = CacheGet(callCache, callIDOut[1]);
		if (newCallHandle == NULL)
		{
			CacheReleaseLocks(callCache);
			return 0;
		}

		newSipCallHandle = SipCallHandle(newCallHandle);
	}

	if (callHandle->handleType != SCC_eSipCallHandle)
	{
		PrintErrorTime();
		fprintf(stderr, "SendRefer(): Malformed call handle\n");
	}

	// Setup the appMsgHandle
	appMsgHandle = SipAllocAppMsgHandle();
	appMsgHandle->csmError = 0;
	appMsgHandle->maxForwards = sipmaxforwards;
	appMsgHandle->calledpn = UrlDup(sipCallHandle->callLeg.remote,
									    sipCallCache->malloc);
	appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.local,
									sipCallCache->malloc);
	memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);

	// Fill in the refer header
	appMsgHandle->msgHandle = SipAllocMsgHandle();

	if (referType == ATTENDED_XFER)
	{
		len+= snprintf(replaceStr+len, max-len, "Replaces=");
		tmp_callid = strdup(newSipCallHandle->callLeg.callid);
		len+= snprintf(replaceStr+len, max-len, "%s", tmp_callid);
		len+= snprintf(replaceStr+len, max-len, ";from-tag=");
		len+= snprintf(replaceStr+len , max-len , "%s", newSipCallHandle->callLeg.local->tag);
		len+= snprintf(replaceStr+len, max-len, ";to-tag=");
		len+= snprintf(replaceStr+len , max-len , "%s", newSipCallHandle->callLeg.remote->tag);
	}
	appMsgHandle->msgHandle->referto = UrlAlloc(MEM_LOCAL);
	appMsgHandle->msgHandle->referto->name = strdup(xferTgtNum);
	appMsgHandle->msgHandle->referto->host = strdup(gwAddr);
	appMsgHandle->msgHandle->referto->header = replaceStr;

	appMsgHandle->msgHandle->referby = UrlDup(sipCallHandle->callLeg.local,
												sipCallCache->malloc);	

    appMsgHandle->realmInfo = (CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
    appMsgHandle->realmInfo->rsa = localIP;
    appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);


	// Setup the event handle
	evHandle = SipAllocEventHandle();
	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = Sip_eBridgeRefer;
	evHandle->handle = appMsgHandle;

	SipUAProcessEvent(evHandle);

	CacheReleaseLocks(callCache);

	fprintf(stdout, "Sent %s call transfer.\n", referType ? "final" : (waitForNotify ? "unattw" : "unatt"));
	fprintf(stdout, ">\n");

	if(referType == BLIND_XFER && !waitForNotify)
	{
		sleep(3);
		DisconnectCall(callIDOut[0],Sip_eBridgeBye);
	}	

}

int
SendNotifyMessage(int code)
{

	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	
	callHandle = CacheGet(callCache, callIDIn[0]);
	if (callHandle == NULL)
	{
		CacheReleaseLocks(callCache);
		return 0;
	}
	sipCallHandle = SipCallHandle(callHandle);

	if (callHandle->handleType != SCC_eSipCallHandle)
	{
		PrintErrorTime();
		fprintf(stderr, "SendNotifyMessage(): Malformed call handle\n");
	}

	// Setup the appMsgHandle
	appMsgHandle = SipAllocAppMsgHandle();
	appMsgHandle->csmError = 0;
	appMsgHandle->maxForwards = sipmaxforwards;
	appMsgHandle->calledpn = UrlDup(sipCallHandle->callLeg.remote,
									sipCallCache->malloc);
	appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.local,
									   sipCallCache->malloc);
	memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);

	// Fill in the refer header
	switch(code)
	{
		case 486:
			appMsgHandle->sip_frag = strdup("SIP/2.0 486 Busy Here");
			if(blindXferMode)
			{
				unatt_transfer_status = 0;		//transfer failed
			}
			break;
		case 404:
			appMsgHandle->sip_frag = strdup("SIP/2.0 404 Not Found");
			if(blindXferMode)
			{
				unatt_transfer_status = 0;		//transfer failed
			}
			break;
		default:
			appMsgHandle->sip_frag = strdup("SIP/2.0 200OK");
			break;
	}
	appMsgHandle->sip_frag_len = strlen(appMsgHandle->sip_frag);
	appMsgHandle->event = strdup("Event: refer");
	appMsgHandle->content_type = strdup("Content-Type: message/sipfrag");

    appMsgHandle->realmInfo = (CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
    appMsgHandle->realmInfo->rsa = localIP;
    appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);


	// Setup the event handle
	evHandle = SipAllocEventHandle();
	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = Sip_eBridgeNotify;
	evHandle->handle = appMsgHandle;

	SipUAProcessEvent(evHandle);

	CacheReleaseLocks(callCache);

	if(!code)
	{
		sleep(3);
		DisconnectCall(callIDIn[0],Sip_eBridgeBye);
	}	
}
	
int
InitializePorts(unsigned long chanLocalAddr, unsigned short chanLocalPort,
				int mgenFd, int ncalls)
{
	int i, n=0;

	// re-initialize the ports

		for (i = 0; (i < nCalls)&&(n<ncalls); i++)
		{
			if (Calls[i].chOut[0].mgenFd)
			{
				continue;
			}

			Calls[i].chIn[0].ip = chanLocalAddr;
			Calls[i].chIn[0].port = chanLocalPort;
			Calls[i].chOut[0].mgenFd = mgenFd;
			if(cm_mode)
			{
				/* Call transfer Scenario A------>B------>C */
				/* When sgen is running in call manager mode (acting as B), it is started with n=2 and in receiver mode and hence 2 media streams.
				 * But after call transfer is completed, it starts acting as a transmitter. For mgen to use 2nd media stream for the transmitted call, we need to
				 * use Receiving port for this stream as chanLocalPort+2. This is because when mgen is requested to do anything (start/stop), it
				 * identifies the stream number using the (chanLocalPort-startPort)/2 logic. If we had used the same stream (stream 1 flowing
				 * between A and B), the bye call between A and B would have stopped thier associated media stream as well which now is a
				 * media stream between B and C (in case of unattended transfer)
				 */
				chanLocalPort += 2;
				CallsOut[i].chIn[0].ip = chanLocalAddr;
				CallsOut[i].chIn[0].port = chanLocalPort;
				chanLocalPort += 2;
				CallsOut[i].chOut[0].mgenFd = mgenFd;
			}
			else
			{
				CallsOut[i].chIn[0].ip = chanLocalAddr;
				CallsOut[i].chIn[0].port = chanLocalPort;
				CallsOut[i].chOut[0].mgenFd = mgenFd;
				chanLocalPort += 2;
			}
				
			n++;
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
		PrintErrorTime();
		fprintf(stderr, "mgenServerThread(): setsockopt error\n");
		perror("mgenServerThread(): setsockopt error ");
	}

	memset(&myaddr, 0, sizeof(myaddr));  /*  Zeroes the struct */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port  = htons (mgenPort);
	myaddr.sin_addr.s_addr  = htonl (chanLocalAddr);

	/* Bind */

	if (bind (mgenServerFd, (struct sockaddr *)&myaddr, sizeof(myaddr))< 0 )
	{
		PrintErrorTime();
		fprintf(stderr, "mgenServerThread(): bind error\n");
		perror ("mgenServerThread() - bind error ");
	}

	/* Listen */
	if (listen (mgenServerFd, 10) < 0)
	{
		PrintErrorTime();
		fprintf(stderr, "mgenServerThread(): listen error\n");
		perror ("mgenServerThread() - listen error ");
	}

	clilen = sizeof(client);
	memset (&client, 0, sizeof(client));

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

	/* diable nagle */
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

	totalcalls += InitializePorts(chanLocalAddr, 
					chanLocalPort, mgenFd, ncalls);

	// Ack
	write(mgenFd, (char *)&ncalls, 4);

	fprintf(stdout, "Media generator configured to handle %d calls\n", totalcalls);
	fprintf(stdout, ">\n");

	if (totalcalls < nCalls)
	{
		// We have to wait for another mgen to connect
		fprintf(stdout, "Waiting for another media generator\n");
		fprintf(stdout, ">\n");
		goto _start;
	}

	LaunchThread(callgenThread, NULL);
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
            fprintf(stdout, "Received signal Inband DTMF: '%c', %dms, -%ddBm0\n>\n", info.digit, ntohl(info.duration), 
                    ntohl(info.volume));
            continue;
        }
        else if (ntohl(mcm.command) == 2)
        {
            // Read mgen stats
            if (read(mgenFd, (char *)&mgenStats, sizeof(MgenStats)) < 0)
            {
                perror("PrintMgenStats() - no connection with mgen, read error ");
            }
            PrintMgenStats();
        }
    }
}

int
SendReinvite(int type)
{
	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	char reinviteType[128];

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, &sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, (void **)callHandlePtr);
		sipCallHandle = SipCallHandle(callHandle);

		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "SendReinvite(): Malformed call handle\n");
		}

		appMsgHandle = SipAllocAppMsgHandle();
		evHandle = SipAllocEventHandle();

		evHandle->type = Sip_eBridgeEvent;
		evHandle->event = Sip_eBridgeInvite;
		evHandle->handle = appMsgHandle;

		// Setup the appMsgHandle
		memcpy(appMsgHandle->callID, callHandle->callID, CALL_ID_LEN);
		appMsgHandle->maxForwards = sipmaxforwards;
		appMsgHandle->minSE = sipminSE;
		appMsgHandle->sessionExpires = sipsessionexpiry;
		appMsgHandle->refresher = SESSION_REFRESHER_NONE;
		appMsgHandle->calledpn = UrlDup(sipCallHandle->callLeg.remote,
									sipCallCache->malloc);
		appMsgHandle->callingpn = UrlDup(sipCallHandle->callLeg.local,
									sipCallCache->malloc);
        appMsgHandle->realmInfo = 
			(CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
        appMsgHandle->realmInfo->rsa = localIP;
        appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

		if(type != REINVITE_TYPE_NOSDP)
		{
			if (type == REINVITE_TYPE_HOLD)
			{
				appMsgHandle->nlocalset = 1;
				appMsgHandle->localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
				appMsgHandle->localSet[0].rtpport = 
					sipCallHandle->remoteSet.remoteSet_val[0].rtpport;
				strcpy(reinviteType, "hold reinvite");
			}
			else if (type == REINVITE_TYPE_HOLD_3264)
			{
				// RFC-3264 based hold
				appMsgHandle->nlocalset = 1;
				appMsgHandle->localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
				appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
				appMsgHandle->localSet[0].rtpport = 
					sipCallHandle->remoteSet.remoteSet_val[0].rtpport;
				appMsgHandle->localSet[0].direction == SendOnly;	

				appMsgHandle->attr_count = 1;
				appMsgHandle->attr = (SDPAttr *) calloc(1, sizeof(SDPAttr));
				SipCreateMediaAttrib(&appMsgHandle->attr[0], 0, "sendonly", NULL);
				strcpy(reinviteType, "RFC3264 hold reinvite");
			}
			else
			{
				appMsgHandle->nlocalset = 2;
				appMsgHandle->localSet = (RTPSet *)calloc(2, sizeof(RTPSet));
				appMsgHandle->localSet[1].codecType = 101;
				appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
				appMsgHandle->localSet[1].rtpaddr = chanLocalAddr;

				appMsgHandle->attr_count = 3;
				appMsgHandle->attr = (SDPAttr *) calloc(3, sizeof(SDPAttr));
				SipCreateMediaAttrib(&appMsgHandle->attr[0], 0, "rtpmap", "0 PCMU/8000");
				SipCreateMediaAttrib(&appMsgHandle->attr[1], 0, "rtpmap", "101 telephone-event/8000");
				SipCreateMediaAttrib(&appMsgHandle->attr[2], 0, "fmtp", "101 0-15");

				if (type == REINVITE_TYPE_SAMEPORT)
				{
					appMsgHandle->localSet[0].rtpport = 
						sipCallHandle->remoteSet.remoteSet_val[0].rtpport;
					appMsgHandle->localSet[1].rtpport = 
						sipCallHandle->remoteSet.remoteSet_val[1].rtpport;
					strcpy(reinviteType, "reinvite with same media port");
				}
				else if (type == REINVITE_TYPE_NEWPORT)
				{
					//chanLocalPort +=2;
					appMsgHandle->localSet[0].rtpport = chanLocalPort;
					appMsgHandle->localSet[1].rtpport = chanLocalPort;

					if (reInviteNewSdp)
					{

						appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
                                		appMsgHandle->localSet[1].rtpaddr = chanLocalAddr;

						appMsgHandle->sdpVersion = 99;
						reInviteNewSdp = 0;
					}

					// Stop media on old port and update it with new port
					if (mode & MODE_TRANSMIT)
					{
						mgenInform(&CallsOut[callHandle->callNo], 0);
						CallsOut[callHandle->callNo].chIn[0].port = chanLocalPort;
					}
					else
					{
						mgenInform(&Calls[callHandle->callNo], 0);
						Calls[callHandle->callNo].chIn[0].port = chanLocalPort;
					}
					strcpy(reinviteType, "reinvite with new media port");
				}
				else if (type == REINVITE_TYPE_RESUME)
				{
                    appMsgHandle->localSet[0].rtpport =
                        sipCallHandle->remoteSet.remoteSet_val[0].rtpport;
                    appMsgHandle->localSet[1].rtpport =
                        sipCallHandle->remoteSet.remoteSet_val[1].rtpport;
					strcpy(reinviteType, "resume reinvite");
				}
			}
		}
		else
		{
			strcpy(reinviteType, "reinvite without SDP");
		}

        fprintf(stdout, "Sent %s for call %d\n", 
				reinviteType, callHandle->callNo);
 
		SipUAProcessEvent(evHandle);
	}

	CacheReleaseLocks(callCache);
}

static void
sig_int_ls(int signo)
{
	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	int result, nbyes = 0;
	int nconnected =0;
	List l = NULL;
	char *callid;
	void *timerdata;

	//If unregWhenExit was requested, then send unregistration request
	if((mode & MODE_REGISTER)&& (unregWhenExit==1))
	{
		SendUnregistrations();
	}
	sleep(1);

	l = listInit();

	if (signo == SIGINT)
	{
		// sigint's are special cases, to be treated like exit 
		nsigints ++;
	}

	// For all ongoing calls, do a disconnect

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, 
			&sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		nbyes++; 	//just keeping count of calls to disconnect
		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, 
						(void **)callHandlePtr);

		sipCallHandle = SipCallHandle(callHandle);

		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "sig_int_ls(): Malformed call handle\n");
		}

		if (timedef_iszero(&callHandle->callStartTime) && timedef_iszero(&callHandle->callConnectTime))
		{
			// we should not be disconnecting this call
			continue;
		}

		if (callHandle->rolloverTimer)
		{
			if (timerDeleteFromList(&localConfig.timerPrivate,
								    callHandle->rolloverTimer, &timerdata))
			{
				if (timerdata)
				{
					free(timerdata);
				}
				callHandle->rolloverTimer = 0;
			}
		}
#if 0
		if (mode & MODE_TRANSMIT)
		{
			mgenInform(&CallsOut[callHandle->callNo], 0);
		}
		else
		{
			mgenInform(&Calls[callHandle->callNo], 0);
		}
#endif
		listAddItem(l, (void *)MemDup(callHandle->callID, CALL_ID_LEN, malloc));
	}

	CacheReleaseLocks(callCache);

	if(nbyes)
		DisconnectCallsWithBurstRate(l);

	if(nsigints == 1)
	{
		fprintf(stdout,"Disconnected %d calls. Total Tx: %d ; Total disconnected: %d\n",nbyes,callCounterTx,callsdisconnected);
	}

	if ((nbyes == 0) || (nsigints > 1))
	{
		fflush(stdout);
		exit(0);
	}
}

void
handleQuitMsg()
{

	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	int result, nbyes = 0;
	int nconnected =0;
	List l = NULL;
	char *callid;
	void *timerdata;

	//If unregWhenExit was requested, then send unregistration request
	if((mode & MODE_REGISTER)&& (unregWhenExit==1))
	{
		SendUnregistrations();
		sleep(1);		//give UA some time to send unregistration
	}
	/* increment nsigints so that all other functions know that user has requested to quit */
	nsigints++;
	l = listInit();

	// For all ongoing calls, do a disconnect

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, 
			&sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		nbyes++; 	//just keping a count of number of calls to disconnect
		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, 
						(void **)callHandlePtr);

		sipCallHandle = SipCallHandle(callHandle);

		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "sig_int_ls(): Malformed call handle\n");
		}

		if (timedef_iszero(&callHandle->callStartTime) && timedef_iszero(&callHandle->callConnectTime))
		{
			// we should not be disconnecting this call
			continue;
		}

		if (callHandle->rolloverTimer)
		{
			if (timerDeleteFromList(&localConfig.timerPrivate,
								    callHandle->rolloverTimer, &timerdata))
			{
				if (timerdata)
				{
					free(timerdata);
				}
				callHandle->rolloverTimer = 0;
			}
		}
#if 0
		if (mode & MODE_TRANSMIT)
		{
			mgenInform(&CallsOut[callHandle->callNo], 0);
		}
		else
		{
			mgenInform(&Calls[callHandle->callNo], 0);
		}
#endif

		listAddItem(l, (void *)MemDup(callHandle->callID, CALL_ID_LEN, malloc));
	}

	CacheReleaseLocks(callCache);

	if(nbyes)
		DisconnectCallsWithBurstRate(l);

	fprintf(stdout,"Disconnected %d calls. Total Tx: %d ; Total disconnected: %d\n",nbyes,callCounterTx,callsdisconnected);

	if( mode & MODE_IXIA )
	{
		IXIA_CLOSE();
	}
	
	exit(0);

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
	{
		return -1;
	}
	else
	{
		return 1;
	}
}
int getNextPhoneFromFile(char *phone,FILE *fp)
{
	if((fp == NULL) || ((fgets(phone,256,fp))==NULL))
			return 0;
	phone[strlen(phone)-1]='\0';	//removing the new line character from the array
	return 1;
}

int
NetSetMaxFds(void)
{
    struct rlimit rl_data;

    getrlimit(RLIMIT_NOFILE, &rl_data);

    rl_data.rlim_cur = rl_data.rlim_max - 1;

    setrlimit(RLIMIT_NOFILE, &rl_data);

    memset(&rl_data, (int32_t) 0, sizeof(struct rlimit));

    getrlimit(RLIMIT_NOFILE, &rl_data);

    fprintf(stdout, "Maximum file descriptors set to %d\n", 
	   (uint32_t) rl_data.rlim_cur);

	return rl_data.rlim_cur;
}

int
LusSetupTimeout(TimerPrivate *timerPrivate, struct timeval *tout)
{
	static struct timeval now;
	static struct Timer front;
	int retval = 0;
	long secs, usecs;

	tout->tv_sec = 0xfffffff;
	tout->tv_usec = 0xfffffff;

	/* Set up the timeout based on our own timer list */
	if (timerFront(timerPrivate, &front) != 0)
	{
		gettimeofday(&now, NULL);

		/* compute front - now, to return the delta */
		secs = front.expire.tv_sec - now.tv_sec;
		usecs = front.expire.tv_usec - now.tv_usec;

		if (usecs < 0)
		{
			secs --;
			usecs += 1000000;
		}

		if ((secs < 0) || (!secs && !usecs))
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, 
				("LusSetupTimeout:: secs is negative\n"));
			return -1;
		}

		tout->tv_sec = secs;
		tout->tv_usec = usecs;

		retval = 1;
	}

	/* we are done... */
	return retval;
}

int
LusAdjustTimeout(struct timeval *tout, int *msec)
{
	int m1;

	// Find min
	if ((tout->tv_sec == (unsigned long)-1) &&
		(tout->tv_usec == (unsigned long)-1))
	{
		NETERROR(MTMR, ("LusAdjustTimeout, bad arg\n"));
		return -1;
	}

	m1 = tout->tv_sec*1000 + tout->tv_usec/1000;	

	if ((*msec == -1) || (m1 < *msec))
	{
		*msec = m1;
		return 1;
	}

	return 0;
}


int
SgenThreadSetPriority(pthread_t thread, int policy, int priority)
{
	struct sched_param param = { 0 };
	int rc = 0;

//	rc = pthread_setschedparam(thread, policy, &param);

	return rc;
}

int
mainLoop()
{
	static struct timeval tout;
	unsigned int msec  = (unsigned int)-1;
	int num, rc, retval, ptimer;
	int fd, result = 0;

	SgenThreadSetPriority(pthread_self(), SCHED_RR, 50);
	for (; ;)
	{
		NetFdsZero(&lsnetfds);

		NetFdsSetup(&lsnetfds, MSEL, NETLOG_DEBUG4);

		retval = select (FD_SETSIZE, 
			&lsnetfds.readfds, &lsnetfds.writefds, NULL, NULL);

		switch (retval)
		{
		case -1:
			PrintErrorTime();
			fprintf(stderr, "mainLoop(): select error\n");
			perror("mainLoop() - select error ");
			/* Here we need to figure out which fd is bad */
			fd = NetDetectBadFd(&lsnetfds, &result, 0, 0);
			if (result > 0)
			{
				PrintErrorTime();
				fprintf(stderr, "mainLoop(): ");
				fprintf(stderr, "Found bad fd %d, deactivating it\n", fd);
				NetFdsDeactivate(&lsnetfds, fd, FD_RW);
			}
		
			break;
		case 0:
			PrintErrorTime();
			fprintf(stderr, "mainLoop(): select timed out\n");
			break;
		default:
			NetFdsProcess(&lsnetfds, MSEL, NETLOG_DEBUG4);
			break;
		}
	}
}

int
LaunchThread(void *(*fn)(void*), void *arg)
{
	// MUST use the MSW api - has no leaks
	ThreadLaunch(fn, arg, 1);
}

// Process the timers

int
SgenIdleTimerProcessor(struct Timer *timer)
{
	char fn[]="SgenIdleTimerProcessor()";
	int callNo = (int)(timer->data);

	if (nsigints <= 0)
	{
		SpawnOutgoingCall(callNo);
	}

	timerFreeHandle(timer);
}

int
SgenRegTimerProcessor(struct Timer *timer)
{
	char fn[] ="SgenRegTimerProcessor()";
	int callNo = (int)(timer->data);
	
	if(nsigints <= 0)
	{
		SendRegistration(callNo);
	}
	timerFreeHandle(timer);
}

int
SgenTimerProcessor(struct Timer *timer)
{
	char fn[]="SgenTimerProcessor()";
	char *callid = (char *) (timer->data);
	CallHandle *callHandle = NULL;
	int event = Sip_eBridgeError;
 	struct 	itimerval calltmr;

	if (nsigints)
	{
		goto _return;
 	}

	CacheGetLocks(callCache, LOCK_WRITE, LOCK_BLOCK);
	
	callHandle = CacheGet(callCache, callid);
	if (callHandle == NULL)
	{
		CacheReleaseLocks(callCache);
		goto _return;
	}

	callHandle->rolloverTimer = 0;

	switch (callHandle->state)
	{
	case Sip_sRingWOR:
	case Sip_sConnectedWOR:
		
		nErrors++;

		nFailedSetups++;

		PrintErrorTime();
		fprintf(stderr, "SgenTimerProcessor(): Setup failed for call %d\n", 
				callHandle->callNo);

		break;

	default:

		event = Sip_eBridgeBye;

		break;
	}
	
	DisconnectCall(callid, event);
	
	if (tdIdle)
	{
		memset(&calltmr, 0, sizeof(struct itimerval));
		calltmr.it_value.tv_sec = tdIdle;

        callHandle->rolloverTimer =
            timerAddToList(&localConfig.timerPrivate, &calltmr,
		 	    0, PSOS_TIMER_REL, "SgenTimer", 
			    (TimerFn) SgenIdleTimerProcessor,
                (void *) (callHandle->callNo));
	}
	else
	{
		SpawnOutgoingCall(callHandle->callNo);
	}

	CacheReleaseLocks(callCache);

_return:

	free(callid);
	timerFreeHandle(timer);
}

void *
timerProcessThread(void *arg)
{
	char fn[] = "timerProcessThread():";
	int	retval = 0;
	sigset_t o_signal_mask, p_signal_mask;
	static struct timeval tout;
	int msec  = -1;
	int num = 0, rc = 0, numus = 0;

	timerThread = pthread_self();

#ifndef NETOID_LINUX

		ThreadSetRT();

#endif // NETOID_LINUX

	ThreadSetPriority(pthread_self(), SCHED_RR, 50);

	/* Loop forever */
	for (; ;)
	{
		retval = LusSetupTimeout(&localConfig.timerPrivate, &tout);

		if (retval < 0)
		{
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("timeout already\n"));
			serviceTimers(&localConfig.timerPrivate);
			continue;
		}

		msec  = -1;
		if (retval)
        {
              // Find the timeout
              LusAdjustTimeout(&tout, &msec);
        }

		// Find out how many entries we have used up first
		numus = NetFdsSetupPoll(&lstimerfds, MSEL, NETLOG_DEBUG4);
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%d ready for gis\n", numus));
		NETDEBUG(MSEL, NETLOG_DEBUG4, ("%d/%d ready in all\n", num+numus, 
				msec));

		retval = poll(lstimerfds.pollA, num+numus, msec);

		switch (retval)
		{
		case -1:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll failure %d", errno));
		
			break;
		case 0:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll timeout"));
			serviceTimers(&localConfig.timerPrivate);

			break;
		default:
			NETDEBUG(MSEL, NETLOG_DEBUG4, ("poll process"));
			retval = NetFdsProcessPoll(&lstimerfds, MSEL, NETLOG_DEBUG4);

			if (retval < 0)
			{
				NETERROR(MSEL,
					("Found a bad fd %d, deactivating it!\n", -retval));
				NetFdsDeactivate(&lstimerfds, -retval, FD_RW);
			}

			break;
		}
	}

	return 0;
}

void *
inputProcessThread(void *arg)
{
	unsigned int msec  = (unsigned int)-1;
	int num, rc, retval;
	int				ntimes = 0;
	struct timeval tmout;

#ifndef NETOID_LINUX

		ThreadSetRT();

#endif // NETOID_LINUX

	inpThread = pthread_self();

	for (; ;)
	{
		NetFdsZero(&lsinfds);

		NetFdsSetup(&lsinfds, MSEL, NETLOG_DEBUG4);

		tmout.tv_sec = 10;
		tmout.tv_usec = 0;

		retval = select (FD_SETSIZE, 
			(idaemon)?NULL:&lsinfds.readfds, (idaemon)?NULL:&lsinfds.writefds, 
			NULL, &tmout);

		switch (retval)
		{
		case -1:
			PrintErrorTime();
			fprintf(stderr, "inputProcessThread(): select error\n");
			perror("inputProcessThread() - select error ");
			break;
		case 0:
			if (idaemon)
			{
				ntimes ++;
				PrintStatus();
				fprintf(stdout, ">\n");
				fflush(stdout); 
				fflush(stderr);
				if (ntimes > 1000000)
				{
					ntimes = 0;
					system("/bin/echo > ./sgenout.log");
					system("/bin/echo > ./sgenerr.log");
					printStartState();
				}
			}
			break;
		default:
			NetFdsProcess(&lsinfds, MSEL, NETLOG_DEBUG4);
			break;
		}
	}
}

void *
DisconnectCalls(void * fnl)
{
	List l = (List)l;
	char *callid;
	struct  timespec    delay;
	int i = 0;
	while (callid = listGetFirstItem(l))
	{
		DisconnectCall(callid, Sip_eBridgeError);
		listDeleteItem(l, callid);
		free(callid);

		i++;

		if (i%burst == 0)
		{
			delay.tv_sec = burstInterval/1000;
			delay.tv_nsec = (burstInterval%1000)*1000000;
			nanosleep(&delay, NULL);
		}
	}
	listReset(l);
}
int 
DisconnectCallsWithBurstRate(void * fnl)
{
	List l = (List)fnl;
	char *callid;
	struct  timespec    delay;
	int i = 0;
	while (callid = listGetFirstItem(l))
	{
		DisconnectCall(callid, Sip_eBridgeError);
		listDeleteItem(l, callid);
		free(callid);

		i++;

		if (i%burst == 0)
		{
			delay.tv_sec = burstInterval/1000;
			delay.tv_nsec = (burstInterval%1000)*1000000;
			nanosleep(&delay, NULL);
		}
	}
	listReset(l);
	return 0;
}

// thread to monitor calls
void *
callmonThread( void *arg )
{
	CallHandle *callHandle, **callHandlePtr;
	SipCallHandle *sipCallHandle;
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	int result, nbyes = 0;
	List l = NULL;
	char *callid;
	time_t now, delta = 0;
	int n1;

	monThread = pthread_self();

#ifndef NETOID_LINUX

		ThreadSetRT();

#endif // NETOID_LINUX

	SgenThreadSetPriority(pthread_self(), SCHED_FIFO, 51);

	l = listInit();

_start:
//	fprintf(stdout, "sleeping\n");
	sleep(tdMonitor);

	nbyes=0;

	time( &now );

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	// record the number of items in the call
	n1 = callCache->nitems;

	callHandle = CacheGetFirst(sipCallCache);
	sipCallHandle = SipCallHandle(callHandle);
	callHandlePtr = (CallHandle **)CacheGetFast(sipCallCache, (void **)&sipCallHandle->callLeg);

	while (callHandlePtr)
	{
		if (nsigints)
		{
			// terminate this session
			CacheReleaseLocks(callCache);
			goto _start;			
		}

		callHandle = *callHandlePtr;
		callHandlePtr = (CallHandle **)CacheGetNextFast(sipCallCache, (void **)callHandlePtr);

		sipCallHandle = SipCallHandle(callHandle);

		if (sipCallHandle->inviteOrigin==0)
		{
			continue;
		}

		delta = 0;
		switch (callHandle->state)
		{
		case Sip_sRingWOR:
		case Sip_sConnectedWOR:
		
			if (!(timedef_iszero(&callHandle->callStartTime)))
			{
				delta = now - timedef_sec(&callHandle->callStartTime);
			}

			if ( delta > tdSetup )
			{
				nErrors++;

				nFailedSetups++;
			}
			else
			{
				continue;
			}

			break;

		case Sip_sConnectedAck:

			if (!(timedef_iszero(&callHandle->callStartTime)))
			{
				delta = now - timedef_sec(&callHandle->callStartTime);
			}

			if ( delta > tdCallDuration )
			{
				// nothing yet
			}
			else
			{
				continue;
			}

			break;

		default:
			continue;
		}

		listAddItem(l, (void *)MemDup(callHandle->callID, CALL_ID_LEN, malloc));
	}

	CacheReleaseLocks(callCache);

	nbyes += l->head->nitems;

	ThreadDispatch(poolid, lpcid, DisconnectCalls, l, 1, 0, 0, 0);

	if (n1 < nCalls)
	{
		// do a burst again
		BurstOutgoingCalls(nCalls-n1);
	}

	goto _start;
}

void *
callgenThread(void *arg)
{
	struct  timespec    delay;

#ifndef NETOID_LINUX

		ThreadSetRT();

#endif // NETOID_LINUX

	initgenThread = pthread_self();

	if (mode & MODE_TRANSMIT)
        {
        	/*************************************************************************/
                // If registration option specified, register first (only once) and then place a call/s

                if (mode & MODE_REGISTER)
                {
                	SendRegistration(0);
                }

                /*************************************************************************/
				if (setModeTransfer)
                	BurstOutgoingCalls(nCalls-1);
				else
                	BurstOutgoingCalls(nCalls);
         }

	else
	{
		if (mode & MODE_REGISTER)
		{
			SendRegistrations(nCalls);
		}
	}

	initgenThread = (pthread_t)-1;
}

int
SendRegistrations(int ncalls)
{
	int i;

	for (i = 0; i < ncalls; i++)	
	{
	 	SendRegistration(i);
 		millisleep(burstInterval);
		
	 	if (nsigints)
	 	{
			break;
  	 	}
	}

	return 0;
}

int
SendRegistration(int callNo)
{
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	char localIPStr[16];

        //Convert localIP to string to pass in localContact
        nx_strlcpy(localIPStr, ULIPtostring(localIP), 16);

	// Setup the appMsgHandle
	appMsgHandle = SipAllocAppMsgHandle();

	appMsgHandle->requri = UrlAlloc(MEM_LOCAL);
	appMsgHandle->requri->host = strdup(gwAddr);

	appMsgHandle->calledpn = UrlAlloc(MEM_LOCAL);
	appMsgHandle->calledpn->host = strdup(gwAddr);
	appMsgHandle->calledpn->name = strdup(callingpna[callNo]);

	if (! (inviteType & INVITE_THRU_OBP))
                nx_strlcpy(SgenGwIpAddr, gwAddr, 256);

	appMsgHandle->callingpn = UrlAlloc(MEM_LOCAL);

	if (termEpMode)
		appMsgHandle->callingpn->host = strdup(gwAddr);
	else	
		appMsgHandle->callingpn->host = strdup(sipdomain);

	appMsgHandle->callingpn->name = strdup(callingpna[callNo]);

	// Contact info filled in.
	appMsgHandle->localContact = UrlAlloc(MEM_LOCAL);
        appMsgHandle->localContact->name = strdup(callingpna[callNo]);
        appMsgHandle->localContact->host = strdup(localIPStr);


	appMsgHandle->realmInfo = 
		(CallRealmInfo *) calloc(1, sizeof(CallRealmInfo));
	appMsgHandle->realmInfo->rsa = localIP;
	appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

	appMsgHandle->minSE = sipminSE;
	appMsgHandle->sessionExpires = sipsessionexpiry;
	appMsgHandle->refresher = SESSION_REFRESHER_NONE;
	appMsgHandle->maxForwards = sipmaxforwards;

	//Added for privacy
        appMsgHandle->incomingPrivType = privacyTypeNone;
        appMsgHandle->privTranslate    = privTranslateNone;
        appMsgHandle->privLevel        = privacyLevelNone;

	// Set the expires header if regExpires specified by user.
	// Else default value is 3600 seconds.
		appMsgHandle->expires = regExpires;


	// Setup the SIP event handle
	evHandle = SipAllocEventHandle();
	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = SipReg_eBridgeRegister;
	evHandle->handle = appMsgHandle;

	// Fill up the call handle
	PopulateCallHandle(callNo, appMsgHandle, evHandle);

	if(unregWhenExit && (regExpires==0))
		fprintf(stdout,"Sending Unregistration for %s\n",callingpna[callNo]);

	return SipRegProcessEvent(evHandle);		
}

int
SendUnregistrations()
{
	regExpires=0;	//set registration expires time to 0
	//if registrations were send for different endpoints, then send unreistration
	//for all the registrations sent 
	if((incCallingPn==1) && !(mode & MODE_TRANSMIT))
	{
		SendRegistrations(nCalls);
	}
	//otherwise send unregistration only once
	else
		SendRegistration(0);
	return 0;
}

int
BurstOutgoingCalls(int ncalls)
{
	struct  timespec    delay;
	int i, low, high = 0;

	low = 0;
	for (i=0; i<ncalls/burst; i++)	
	{
		high = low+burst;
	 	SpawnOutgoingCalls(low, high);
		
	 	if (nsigints)
	 	{
			return 0;
  	 	}

		low = high;
	}

	// take care of the fact that ncalls may not
	// be a multiple of burst
	if (high<ncalls)
	{
		SpawnOutgoingCalls(high, ncalls);
	}
}

int
SpawnOutgoingCalls(int low, int high)
{
	int i;

 	for (i=low; i<high; i++)
 	{
	 	if (nsigints)
	 	{
			return 0;
  		}

 		SpawnOutgoingCall(i);
 		millisleep( callTimePeriod/1000000);
	}
}

int
DisconnectCall(char *callid, int event)
{
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;

	if(mode & MODE_MGCP)
		mgenDeleteCall(callid);

	appMsgHandle = SipAllocAppMsgHandle();
	evHandle = SipAllocEventHandle();

	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = event;
	evHandle->handle = appMsgHandle;

	// Setup the appMsgHandle
	memcpy(appMsgHandle->callID, callid, CALL_ID_LEN);
	appMsgHandle->maxForwards = sipmaxforwards;

    appMsgHandle->realmInfo = 
		(CallRealmInfo *) calloc (1, sizeof(CallRealmInfo));
    appMsgHandle->realmInfo->rsa = localIP;
    appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

	SipUAProcessEvent(evHandle);
	return 0;
}

int
ManipulateReplacesHeader(char *str, char *tmp_str)
{
	while(*str != '\0')
	{
		if(*str == '=')
		{
			*tmp_str = ':';
			str++;
			tmp_str++;
			*tmp_str = ' ';
			tmp_str++;
			
		}
		else
		{
			if(*str == '%')
			{
				str+=2;

				if(*str == 'B')
				{
					*tmp_str = ';';
				}
				if(*str == 'D')
				{
					*tmp_str = '=';
				}
				str++;
				tmp_str++;

			}
			else
			{
				*tmp_str = *str;
				tmp_str++;
				str++;
			}
		}
	}
}

int
SpawnOutgoingCall(int callNo)
{
	SipAppMsgHandle *appMsgHandle;
	SipEventHandle	*evHandle = NULL;
	static int printLine = 0;
	char localIPStr[16];
	char rpIdStr[256];
	char tmp_str[1024] = {0};

        //Convert localIP to string to pass in localContact
        nx_strlcpy(localIPStr, ULIPtostring(localIP), 16);
	
	// If not in OBP mode, Invite should go to gwAddr
	if (!(inviteType & INVITE_THRU_OBP))
		nx_strlcpy(SgenGwIpAddr, gwAddr, 256);	

	 // If the totalMaxCalls limit is reached, disconnect calls and gracefully exit.
	 // No more calls to be generated.

         if ((maxCallsMode == 1) && (callCounterTx >= totalMaxCalls))
         {
	      if ((callCounterTx == totalMaxCalls) && (printLine == 0))
	      {
              	fprintf(stdout, "Total Max Calls reached.\n");
		printLine = 1;
	      }
	      if (!retainCalls)
              	sig_int_ls(SIGINT);

         }

	else
	{

	// Setup the appMsgHandle
	appMsgHandle = SipAllocAppMsgHandle();

	appMsgHandle->maxForwards = sipmaxforwards;
	appMsgHandle->calledpn = UrlAlloc(MEM_LOCAL);

	if (inviteType & INVITE_URI_DIAL)
	{
		appMsgHandle->calledpn->name = strdup(uriUser);
		appMsgHandle->calledpn->host = strdup(uriHost);
	}
	else
	{
		if(blindXferMode || attXferMode || transferFlag)
			appMsgHandle->calledpn->name = strdup(xferTgtNum);
		else
			appMsgHandle->calledpn->name = strdup(calledpna[callNo]);

		appMsgHandle->calledpn->host = strdup(gwAddr);
	}

	// If Sip Privacy enabled on the sgen, the From Header changes to Anonymous@Anonymoud.invalid
	// The From header contents are filed up in the p-assert headr.
	// The privLevel is set to either 'id' or 'none'

	appMsgHandle->callingpn = UrlAlloc(MEM_LOCAL);
	appMsgHandle->callingpn->name = strdup(callingpna[callNo]);

	if (termEpMode)
		appMsgHandle->callingpn->host = strdup(gwAddr);
	else
		appMsgHandle->callingpn->host = strdup(sipdomain);
	
	appMsgHandle->localContact = UrlAlloc(MEM_LOCAL);
	appMsgHandle->localContact->name = strdup(callingpna[callNo]);
        appMsgHandle->localContact->host = strdup(localIPStr);

	if (sipPrivRfcMode || sipPrivDualMode)
	{

		appMsgHandle->pAssertedID_Sip = UrlAlloc(MEM_LOCAL);
		appMsgHandle->original_from_hdr = UrlAlloc(MEM_LOCAL);

		appMsgHandle->pAssertedID_Sip->name = strdup(callingpna[callNo]);
		appMsgHandle->pAssertedID_Sip->host = strdup(sipdomain);

		appMsgHandle->original_from_hdr->name = strdup("anonymous");
		appMsgHandle->original_from_hdr->host = strdup("anonymous.invalid");
		
		appMsgHandle->callingpn->name = strdup("anonymous");
		appMsgHandle->callingpn->host = strdup("anonymous.invalid");

		appMsgHandle->incomingPrivType = privacyTypeRFC3325;
		appMsgHandle->privTranslate    = privTranslateNone;
	
		if (strcmp(privTag, "id") == 0)
		{
			appMsgHandle->privLevel = privacyLevelId; 
			appMsgHandle->priv_value = strdup(privTag);
		}
		else
		{
			appMsgHandle->privLevel = privacyLevelNone;	
			appMsgHandle->priv_value = NULL;
		}
		
	}
	else if (sipPrivDftMode)
	{
			
		appMsgHandle->rpid_hdr = UrlAlloc(MEM_LOCAL);
		appMsgHandle->original_from_hdr = UrlAlloc(MEM_LOCAL);

                appMsgHandle->rpid_hdr->name = strdup(callingpna[callNo]);
                appMsgHandle->rpid_hdr->host = strdup(sipdomain);
		
		appMsgHandle->original_from_hdr->name = strdup("anonymous");
                appMsgHandle->original_from_hdr->host = strdup("localhost");

                appMsgHandle->callingpn->name = strdup("anonymous");
                appMsgHandle->callingpn->host = strdup("localhost");

		appMsgHandle->incomingPrivType = privacyTypeDraft01;
                appMsgHandle->privTranslate    = privTranslateNone;

		strcpy (rpIdStr,"Remote-Party-Id: <sip:");
		strcat (rpIdStr, callingpna[callNo]);
		strcat (rpIdStr,"@");
		strcat (rpIdStr, sipdomain);
		if (strcmp(privTag, "id") == 0)
			strcat (rpIdStr, ">;screen=yes;party=calling;privacy=full");
		else
			strcat (rpIdStr, ">;screen=yes;party=calling;privacy=off");

		appMsgHandle->rpid_url = strdup(rpIdStr);

                if (strcmp(privTag, "id") == 0)
		{
                        //appMsgHandle->privLevel = privacyLevelId;
			appMsgHandle->proxy_req_hdr = strdup("Proxy-Require: privacy");
		}
                else
		{
                        //appMsgHandle->privLevel = privacyLevelNone;
			appMsgHandle->proxy_req_hdr = NULL;
		}
	}

	appMsgHandle->requri = UrlAlloc(MEM_LOCAL);

	if (inviteType & INVITE_URI_DIAL)
        {
                appMsgHandle->requri->name = strdup(uriUser);
                appMsgHandle->requri->host = strdup(uriHost);
		appMsgHandle->requri->port = uriPort;
        }
	else
	{
		if(blindXferMode || attXferMode || transferFlag)
			appMsgHandle->requri->name = strdup(xferTgtNum);
		else
			appMsgHandle->requri->name = strdup(calledpna[callNo]);	
		appMsgHandle->requri->host = strdup(gwAddr);
		appMsgHandle->requri->port = gwPort;
	}

	appMsgHandle->minSE = sipminSE;
	appMsgHandle->sessionExpires = sipsessionexpiry;
	appMsgHandle->refresher = SESSION_REFRESHER_NONE;
	appMsgHandle->maxForwards = sipmaxforwards;
	
	appMsgHandle->sdpVersion = 66;

	// If INVITE_NOSDP, Do not fill the SDP.

	if (holdType & INVITE_HOLD)
	{
		appMsgHandle->nlocalset = 1;
                appMsgHandle->localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
                appMsgHandle->localSet[0].rtpport =
			CallsOut[callNo].chIn[0].port;
	}
	else if ( holdType & INVITE_HOLD_3264)
	{

		// RFC-3264 based hold
                appMsgHandle->nlocalset = 1;
                appMsgHandle->localSet = (RTPSet *)calloc(1, sizeof(RTPSet));
                appMsgHandle->localSet[0].rtpaddr = chanLocalAddr;
                appMsgHandle->localSet[0].rtpport =
			CallsOut[callNo].chIn[0].port;
                appMsgHandle->localSet[0].direction == SendOnly;

                appMsgHandle->attr_count = 1;
                appMsgHandle->attr = (SDPAttr *) calloc(1, sizeof(SDPAttr));
                SipCreateMediaAttrib(&appMsgHandle->attr[0], 0, "sendonly", NULL);
	}

	else if ( !(inviteType & INVITE_NOSDP))
	{
		// Regular invite.
                appMsgHandle->nlocalset = 2;
                appMsgHandle->localSet = (RTPSet *) calloc(2, sizeof(RTPSet));

                appMsgHandle->localSet[0].codecType = 0;
                appMsgHandle->localSet[0].rtpaddr = CallsOut[callNo].chIn[0].ip;
                appMsgHandle->localSet[0].rtpport = CallsOut[callNo].chIn[0].port;
                appMsgHandle->localSet[0].mLineNo = 0;

                appMsgHandle->localSet[1].codecType = 101;
                appMsgHandle->localSet[1].rtpaddr = CallsOut[callNo].chIn[0].ip;
                appMsgHandle->localSet[1].rtpport = CallsOut[callNo].chIn[0].port;
                appMsgHandle->localSet[1].mLineNo = 0;

		appMsgHandle->attr_count = 3;
        	appMsgHandle->attr = (SDPAttr *) calloc(3, sizeof(SDPAttr));
        	SipCreateMediaAttrib(&appMsgHandle->attr[0], 0, "rtpmap", "0 PCMU/8000");
        	SipCreateMediaAttrib(&appMsgHandle->attr[1], 0, "rtpmap", "101 telephone-event/8000");
        	SipCreateMediaAttrib(&appMsgHandle->attr[2], 0, "fmtp", "101 0-15");

	}	

	appMsgHandle->realmInfo = (CallRealmInfo *) calloc(1, sizeof(CallRealmInfo));
	appMsgHandle->realmInfo->rsa = localIP;
	appMsgHandle->realmInfo->sipdomain = strdup(sipdomain);

	// SH - code added for Sip-T
	// Create the ISUP/QSIG header
	if (isupMode)
		CreateIsupMessage(appMsgHandle);

	if (qsigMode)
		CreateQsigMessage(appMsgHandle);

	if(attXferMode)
	{
		//Fill the replaces header
		ManipulateReplacesHeader(replaceStr, tmp_str);

		appMsgHandle->msgHandle = SipAllocMsgHandle();
		appMsgHandle->msgHandle->replaces = tmp_str;
	}

	// Setup the SIP event handle
	evHandle = SipAllocEventHandle();
	evHandle->type = Sip_eBridgeEvent;
	evHandle->event = Sip_eBridgeInvite;
	evHandle->handle = appMsgHandle;

	// Fill up the call handle
	PopulateCallHandle(callNo, appMsgHandle, evHandle);

	callIDOut[callNo] = (char *) calloc(CALL_ID_LEN, sizeof(char));
	memcpy(callIDOut[callNo], appMsgHandle->callID, CALL_ID_LEN);

	callCounterTx++;

	return SipUAProcessEvent(evHandle);		
	}
}

int
PopulateCallHandle(int callNo, SipAppMsgHandle *appMsgHandle, SipEventHandle *evHandle)
{
    struct  itimerval calltmr;
    CallHandle *callHandle;
    int origin = 1;
    char tgrp[256] = "";
    struct itimerval regtmr;


	CacheGetLocks(callCache,LOCK_WRITE,LOCK_BLOCK);

	callHandle = SipInitAppCallHandleForEvent(evHandle);

	callHandle->realmInfo->sipdomain = strdup(sipdomain);

	callHandle->realmInfo->rsa = localIP;

	callHandle->callNo = callNo;
	memcpy(callHandle->confID, &callNo, 4);
	memcpy(callHandle->confID+4, &origin, 4);

	callHandle->timerSupported = 1;
	callHandle->minSE = sipminSE;
	callHandle->sessionExpires = sipsessionexpiry;
	callHandle->refresher = SESSION_REFRESHER_NONE;

    if (strlen(dtg) > 0)
	{
		callHandle->ecaps1 |= ECAPS1_SETDESTTG;
        callHandle->destTg = CStrdup(callCache, dtg);
        if (vendorSonusGSX)
		{
            callHandle->vendor = Vendor_eSonusGSX;
		}
		else
		{
    		snprintf(tgrp, sizeof(tgrp), "%s;tgrp=%s", calledpna[callNo], dtg);
			appMsgHandle->requri->name = strdup(tgrp);	
		}
	}

    if (strlen(otg) > 0)
	{
        callHandle->tg = CStrdup(callCache, otg);
        if (vendorSonusGSX)
		{
            callHandle->vendor = Vendor_eSonusGSX;
		}
	}

	if (tdConnect)
	{
		memset(&calltmr, 0, sizeof(struct itimerval));
		calltmr.it_value.tv_sec = tdConnect;

        callHandle->rolloverTimer =
            timerAddToList(&localConfig.timerPrivate, &calltmr,
   			   	0, PSOS_TIMER_REL, "SgenTimer", (TimerFn) SgenTimerProcessor,
                (void *)(MemDup(appMsgHandle->callID, CALL_ID_LEN, malloc)));
	}
	if (regExpires && (mode & MODE_REGISTER))
	{
		memset(&regtmr, 0 , sizeof(struct itimerval));
		regtmr.it_value.tv_sec = regExpires;

		regTid = 
			timerAddToList(&localConfig.timerPrivate, &regtmr,
                                0, PSOS_TIMER_REL, "SgenTimer", (TimerFn) SgenRegTimerProcessor,
		(void *) (callHandle->callNo));
		
	}

	CacheReleaseLocks(callCache);
	
	return 0;
}

int
CreateIsupMessage(SipAppMsgHandle* appMsgHandle)
{
	char isupBody[17] = {0x01, 0x00, 0x49, 0x00, 0x00, 0x03, 0x02, 0x00, 0x07, 0x04, 0x10, 0x00, 0x33, 0x63, 0x21, 0x43, 0x00};

	appMsgHandle->isup_msg = (char*) malloc(256);
        appMsgHandle->isup_msg_len = sizeof(isupBody);
        memcpy (appMsgHandle->isup_msg, isupBody, appMsgHandle->isup_msg_len);
        appMsgHandle->isupTypeVersion = strdup("nxv3");
        appMsgHandle->isupTypeBase = strdup("etsi121");
        appMsgHandle->isupDisposition = strdup ("signal");
        appMsgHandle->isupHandling = strdup ("optional");
	
	return 1;
}

int
CreateQsigMessage(SipAppMsgHandle* appMsgHandle)
{
	char qsigBody[27] = {0x08,0x02,0x55,0x55, 0x05, 0x04, 0x02, 0x90, 0x90, 0x18, 0x03, 0xa1, 0x83, 0x01, 0x70, 0x0a, 0x89, 0x31, 0x34, 0x30, 0x38, 0x34, 0x39, 0x35, 0x30, 0x37, 0x32};

	appMsgHandle->qsig_msg = (char*) malloc(256);
        appMsgHandle->qsig_msg_len = sizeof(qsigBody);
        memcpy (appMsgHandle->qsig_msg, qsigBody, appMsgHandle->qsig_msg_len);
        appMsgHandle->qsigTypeVersion = strdup("iso");
        appMsgHandle->qsigDisposition = strdup ("signal");
        appMsgHandle->qsigHandling = strdup ("optional");
	
	return 1;

}

int
InitCache()
{
	LockInit(&callmutex, 1);
	LockInit(&transmutex, 1);
	LockInit(&realmmutex, 1);
	LockInit(&sipregmutex, 1);

	transCache = CacheCreate(1);
	transCache->dt = CACHE_DT_AVL;
	CacheSetName(transCache, "TRANS Cache");
	transCache->cachecmp = TransCmp;
    transCache->cacheinscmp = TransInsCmp;
    transCache->cachedup = TransDup;
    transCache->lock = &transmutex;
	CacheInstantiate(transCache);

    /* Initialize the sip call cache set */
    sipCallCache = CacheCreate(1);
	sipCallCache->dt = CACHE_DT_AVLTR;
    CacheSetName(sipCallCache, "SIP Call Cache");
    sipCallCache->cachecmp = SipCallCmp;
    sipCallCache->cacheinscmp = SipCallInsCmp;
    sipCallCache->cachedup = SipCallDup;
    sipCallCache->lock = &callmutex;
	CacheInstantiate(sipCallCache);

	callCache = CacheCreate(1);
	callCache->dt = CACHE_DT_AVLTR;
    CacheSetName(callCache, "CALL Cache");
    callCache->cachecmp = CallCmp;
    callCache->cacheinscmp = CallInsCmp;
    callCache->cachedup = CallDup;
	callCache->pre_cond = CallCachePreCb;
    callCache->lock = &callmutex;
	CacheInstantiate(callCache);

    sipregCache = CacheCreate(1);
    sipregCache->dt = CACHE_DT_AVL;
    CacheSetName(sipregCache, "SIP REG Cache");
    sipregCache->cachecmp = SipRegCmp;
    sipregCache->cacheinscmp = SipRegInsCmp;
    sipregCache->cachedup = SipRegDup;
    sipregCache->lock = &sipregmutex;
    CacheInstantiate(sipregCache);

	// realm cache

	realmCache = CacheCreate(1);
	realmCache->dt = CACHE_DT_AVL;
	CacheSetName(realmCache, "Realm Cache");
	realmCache->cachecmp = RealmCmpFnId;
	realmCache->cacheinscmp = RealmInsCmpFnId;
	realmCache->lock = &realmmutex;

	rsaCache = CacheCreate(1);
	rsaCache->dt = CACHE_DT_AVL;
	CacheSetName(rsaCache, "Rsa Cache");
	rsaCache->cachecmp = RsaCmpFnId;
	rsaCache->cacheinscmp = RsaInsCmpFnId;
	rsaCache->lock = &realmmutex;

	CacheInstantiate(realmCache);
	CacheInstantiate(rsaCache);

	defaultRealm = &mydefaultRealm;
	AddDefaultRealm();
}

int
PrintStatus()
{
	int nconnected = 0, nworring = 0, nringwor = 0, nwack = 0;
	CallHandle *callHandle;
	SipCallHandle *sipCallHandle = NULL;
	SipTrans *siptranptr=NULL;
	char str1[CALL_ID_LEN], str2[CALL_ID_LEN];
	SipTransKey *keyptr;
	tid t;
	struct timeval res;

	// print the cache statistics
	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);

	for (callHandle = CacheGetFirst(sipCallCache);
		 callHandle;
		 callHandle = CacheGetNext(sipCallCache, &sipCallHandle->callLeg))
	{
		if (callHandle->handleType != SCC_eSipCallHandle)
		{
			PrintErrorTime();
			fprintf(stderr, "PrintStatus(): Malformed call handle\n");
		}
		if (callHandle->state == Sip_sConnectedAck)
		{
			nconnected++;
		}
		if (callHandle->state == Sip_sWORRing)
		{
			nworring++;
		}
		if (callHandle->state == Sip_sRingWOR)
		{
			nringwor++;
		}
		if (callHandle->state == Sip_sConnecting)
		{
			nwack++;
		}

		if (debug && !automode)
		{

		fprintf(stdout, "call state %s\n", GetSipState(callHandle->state));
		sipCallHandle = SipCallHandle(callHandle);
		fprintf(stdout, "sip call:\n");
		fprintf(stdout, "cid %s cfid %s leg %d\n",
			CallID2String(callHandle->callID, str1),
			CallID2String(callHandle->confID, str2),
			callHandle->leg);
		if (sipCallHandle->callLeg.local)
			fprintf(stdout, "local: %s@%s:%d; tag=%s\n", 
				SVal(sipCallHandle->callLeg.local->name),
				SVal(sipCallHandle->callLeg.local->host), 
				sipCallHandle->callLeg.local->port,
				SVal(sipCallHandle->callLeg.local->tag));
		if (sipCallHandle->callLeg.remote)
			fprintf(stdout, "remote: %s@%s:%d; tag=%s\n", 
				SVal(sipCallHandle->callLeg.remote->name),
				SVal(sipCallHandle->callLeg.remote->host),
				sipCallHandle->callLeg.remote->port,
				SVal(sipCallHandle->callLeg.remote->tag));
		fprintf(stdout, "callid: %s\n", sipCallHandle->callLeg.callid);

		if (sipCallHandle->requri)
			fprintf(stdout, "requri: %s@%s:%d\n", 
				SVal(sipCallHandle->requri->name),
				SVal(sipCallHandle->requri->host), 
				sipCallHandle->requri->port);
		if (sipCallHandle->localContact)
			fprintf(stdout, "local contact: %s@%s:%d\n", 
				SVal(sipCallHandle->localContact->name),
				SVal(sipCallHandle->localContact->host), 
				sipCallHandle->localContact->port);
		if (sipCallHandle->remoteContact)
			fprintf(stdout, "remote contact: %s@%s:%d\n", 
				SVal(sipCallHandle->remoteContact->name),
				SVal(sipCallHandle->remoteContact->host), 
				sipCallHandle->remoteContact->port);
		fprintf(stdout, "\n");
		}
	}

	fprintf(stdout,
       	    "CurrentCalls        :    %d"
   	    	"\nConnectedCalls      :    %d"
       	    "\nPendingCalls        :    %d"
           	"\nPendingIncoming     :    %d"
           	"\nPendingAcks         :    %d"
           	"\nNumFailedSetups     :    %d"
		"\nNumFailedInvites    :    %d"
                "\nTotalCallsTx        :    %d"
           	"\n",
			callCache->nitems, nconnected, 
			nringwor, nworring, nwack, nFailedSetups,  nFailedInvites, callCounterTx);

	CacheFreeIterator(sipCallCache);

	CacheReleaseLocks(callCache);

	if (debug && !automode)
	{
	CacheGetLocks(transCache, LOCK_READ, LOCK_BLOCK);
	
	for (siptranptr = CacheGetFirst(transCache);
			siptranptr; 
			siptranptr = CacheGetNext(transCache, &siptranptr->key))
	{
		keyptr = &siptranptr->key;
		fprintf(stdout, "callid =%s Local={user=%s host=%s port=%d tag=%s} Remote={user=%s host=%s port=%d tag=%s} Cseq=%d %s type=%d\n", 
				       (SipTranKeyCallid(keyptr)), 
				       (SipTranKeyLocal(keyptr)->name), 
				       (SipTranKeyLocal(keyptr)->host), 
				       SipTranKeyLocal(keyptr)->port, 
				       SVal(SipTranKeyLocal(keyptr)->tag),
				       (SipTranKeyRemote(keyptr)->name),
				       (SipTranKeyRemote(keyptr)->host),
				       SipTranKeyRemote(keyptr)->port,
				       SVal(SipTranKeyRemote(keyptr)->tag),
				       SipTranKeyCseqno(keyptr),
				       (SipTranKeyMethod(keyptr)), 
					   SipTranKeyType(keyptr) );
	}

	CacheFreeIterator(transCache);

	CacheReleaseLocks(transCache);
	}
}

int 
ReadInput(int fd, FD_MODE rw, void *data)
{
	static char input[1024];
	char *s;
	static unsigned long mark;
	char cmd[128];
	char signalType[128], tmpAddr[256];
	char xferType[64];
	int duration;		// msec
    int volume;
	int whitespaces;
	int tmpPort;
	int len;
	SgenThreadSetPriority(pthread_self(), SCHED_OTHER, 40);

	if (fgets(input, sizeof(input), stdin) == NULL)
	{
		// we are done with input, remove this fd
		NetFdsDeactivate(&lsinfds, 0, FD_READ);
		return 0;
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
		if( mode & MODE_IXIA )
		{
			IXIA_CLOSE();
		}
		exit (0);
	}
	else if (strcmp(s, "stop") == 0)
	{
		if( mode & MODE_IXIA )
		{
			IXIA_CLOSE();
		}
		sig_int_ls(SIGINT);
	} 
	else if (strcmp(s, "quit") == 0)
	{
		handleQuitMsg();
	}
	else if (strcmp(s, "bye") == 0)
	{
		sig_int_ls(-1);
	} 
	else if (strncmp(s, "reinvite-nosdp", 14) == 0)
	{
		if (strlen(s) > 15)
		{
			//Media address specified to be sent in the Ack with SDP
			sscanf(s, "%s %s %d", cmd, tmpAddr, &tmpPort);
                	chanLocalAddr = ntohl(inet_addr(tmpAddr));
			chanLocalPort = tmpPort;
		}
		reInvType = REINVITE_TYPE_NOSDP;
		SendReinvite(REINVITE_TYPE_NOSDP);
	}
	else if (strcmp(s, "reinvite") == 0)
	{
		reInvType = REINVITE_TYPE_NEWPORT;
		SendReinvite(REINVITE_TYPE_NEWPORT);
	}
	else if (strcmp(s, "reinvite-sameport") == 0)
	{
		reInvType = REINVITE_TYPE_SAMEPORT;
		SendReinvite(REINVITE_TYPE_SAMEPORT);
	}
	else if (strcmp(s, "hold") == 0)
	{
		reInvType = REINVITE_TYPE_HOLD;
		SendReinvite(REINVITE_TYPE_HOLD);
	}
	else if (strcmp(s, "hold-3264") == 0)
	{
		reInvType = REINVITE_TYPE_HOLD_3264;
		SendReinvite(REINVITE_TYPE_HOLD_3264);
	}
	else if (strcmp(s, "resume") == 0)
	{
		reInvType = REINVITE_TYPE_RESUME;
		SendReinvite(REINVITE_TYPE_RESUME);
	}
	else if (strcmp(s, "ss7signal") == 0)
        {
                if (isupMode || qsigMode)
                        SendInfoForSipT();
                else
                        fprintf(stdout, "Callgen not in Sip-T mode. Cannot send INFO message. \n");
        }

	else if (strncmp (s, "reinvite-newsdp", 15) == 0)
	{
		reInviteNewSdp = 1;
		sscanf(s, "%s %s %d", cmd, tmpAddr, &tmpPort);
		chanLocalAddr = ntohl(inet_addr(tmpAddr));	
		chanLocalPort = (unsigned short) tmpPort;
		reInvType = REINVITE_TYPE_NEWPORT;
		SendReinvite(REINVITE_TYPE_NEWPORT);	
	}
	else if (strcmp(s , "current-calls") == 0)
	{
		CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
         	fprintf(stdout, "%d calls connected.\n", callCache->nitems);
		CacheReleaseLocks(callCache);

	}
	else if (strncmp(s, "mstat",5) == 0)
	{
		if (mode & MODE_MGCP)
		{
			if(!mstatArg)
				mstatArg = calloc(5, sizeof(char));
			else
				memset(mstatArg,0,5);
			sscanf(s,"%s %s",cmd,mstatArg);
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
	else if (strncmp(s, "sleep", 5) == 0)
	{
		duration = 1;	// seconds
		sscanf(s, "%s %d", cmd, &duration);
		sleep(duration);
	}
	else if (strncmp (s, "dtmf", 4) == 0)
	{
		duration = 200;	// seconds
		signalType[0] = '1';	// default signalType
		sscanf(s, "%s %s %d", cmd, signalType, &duration);
		SendInfo(signalType, duration);
	}
	else if (strncmp (s, "indtmf", 6) == 0)
	{
		duration = 200;	// seconds
        volume  = 30;
		signalType[0] = '1';	// default signalType
		sscanf(s, "%s %s %d %d", cmd, signalType, &duration, &volume);
        SendMgenDtmf(signalType, duration, volume);
	}
	else if (strncmp (s, "transfer", 8) == 0)
	{
		sscanf(s, "%s %s %s", cmd, xferType, xferTgtNum);
		if(strncmp (xferType, "unatt", 5) == 0)
		{
			// perform blind transfer
			if(xferType[5] == 'w')
			{
				waitForNotify = 1;
			}
			SendBlindXfer();
		}
		else
		{	
			if (strcmp (xferType, "att") == 0)
			{
				// perform attended transfer
				transferFlag = 1;
				SendAttendedXfer();
			}
			else
			{	
				if (strcmp(xferType, "final") == 0)
				{
					SendReferMessage(ATTENDED_XFER);
				}
				else
				{
					if(strcmp(xferType, "abandon") == 0)
					{
						fprintf(stdout, "Sent abandon call transfer.\n");
						fprintf(stdout, ">\n");
						SendReinvite(REINVITE_TYPE_RESUME);
						att_transfer_status = 0;
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
	else if (strcmp(s, "help") == 0)
	{
		PrintInteractiveCmds();
	}
	else if (strcmp(s, "mem_start") == 0)
	{
#ifdef _DMALLOC_
		dmalloc_message("starting new log");
		mark = dmalloc_mark();
#elif _DBMALLOC_
		orig_size = malloc_inuse(&histid1);
#endif
	}
	else if (strcmp(s, "mem_stop") == 0)
	{
#ifdef _DMALLOC_
		dmalloc_log_changed(mark, 1, 0, 1);
		dmalloc_message("end of log");
#elif _DBMALLOC_ 
		current_size = malloc_inuse(&histid2);

		if(current_size != orig_size)
		{
			fd = open("malloc.inuse", O_CREAT|O_RDWR|O_TRUNC);
			malloc_list(fd, histid1, histid2);
			close(fd);
		}
#endif
	}
	else 
	{
		PrintStatus();
	}

	fprintf(stdout, ">\n");		// Ready to accept input

	return 0;
}

int
HandleNotify(int fd, FD_MODE rw, void *data)
{
	char buff[100];
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;
	int res;

	tcbdata->rnotify++;
	res = read(fd, buff, 1);	

	NETDEBUG(MINIT, NETLOG_DEBUG4,
		("Read %d notifications\n", res));

	return 0;
}

int
IServerNotify(void *data)
{
	char fn[] = "IServerNotify():";
	TimerNotifyCBData *tcbdata = (TimerNotifyCBData *)data;

	if (tcbdata->wnotify != tcbdata->rnotify)
	{
		// already notification in queue
		NETDEBUG(MINIT, 4, ("%s Already Notified r=%x w=%x\n",
			fn, tcbdata->rnotify, tcbdata->wnotify));
		return 0;
	}

	if (write(tcbdata->notifyPipe[NOTIFY_WRITE]," ",1) < 0)
	{
		// Check the error
		if (errno == EAGAIN)
		{
			NETERROR(MINIT, ("Blocking error in notification\n"));
		}
	}
	else
	{
		tcbdata->wnotify ++;
	}
}

int
main(int argc, char **argv)
{
	int nbio = 1;
	int flags;
	TimerNotifyCBData *tcbdata;
	LsMemStruct tmpLsMem = {0};

	allowSrcAll = 1;
	forwardSrcAddr = 1;
	lsMem = &tmpLsMem;

	struct stat buf;
	char ixiaCommand[IXIA_COMMAND_LEN];

	sipStats = calloc(1, sizeof(SipStats));

#ifdef _DMALLOC_
	{
		extern char *_dmalloc_logpath;
		_dmalloc_logpath = "./malloc.inuse";
	}

#elif _DBMALLOC_ 

	//extern int malloc_preamble;
	{
		union dbmalloptarg val;

		val.i = 1;
		val.i = 0;
		dbmallopt(MALLOC_CKCHAIN, &val);

		val.i = 1;
		dbmallopt(MALLOC_SHOWLINKS, &val);

		val.i = 1;
		dbmallopt(MALLOC_DETAIL, &val);

		val.i = 129;
		val.i = 0;
		dbmallopt(MALLOC_FATAL, &val);

		val.str = "malloc.errs";
		//dbmallopt(MALLOC_ERRFILE, &val);

		val.i = 0;
		dbmallopt(MALLOC_REUSE, &val);

		val.i = 0;
		dbmallopt(MALLOC_CKDATA, &val);
	}
	
#endif

#ifndef NO_LIC
	CheckLicense();
#endif

	ParseArguments(argc, argv);

	SignalInit();

	maxfds = NetSetMaxFds();
	strcpy(sipservername, "SIP-GEN");

	NetFdsSetMax(maxfds);

	libavl_init(1);

	identMain = mainThread;

	if (idaemon)
	{
		printStartState();
		fprintf(stdout, "Running as daemon\n");

		freopen("/dev/null", "r", stdin );
    	freopen("./sgenout.log", "a", stdout);
    	freopen("./sgenerr.log", "a", stderr);

		daemonize ();
	}

#ifndef NETOID_LINUX

		ThreadInitRT();
		ThreadSetRT();

#endif // NETOID_LINUX

	SipUAInit();

	NetSyslogOpen(argv[0], NETLOG_TERMINAL|NETLOG_SIPTERMINAL);

	if (debug)
	{
    	NETLOG_SETLEVEL(MSIP, NETLOG_DEBUG4);
    	NETLOG_SETLEVEL(MTMR, NETLOG_DEBUG1);
    	NETLOG_SETLEVEL(MSEL, NETLOG_DEBUG4);
	}

	InitCalls();

	InitCache();
	lsMem->lsVportAlarm = calloc(MAX_LS_ALARM, sizeof(long));
	lsMem->lsMRVportAlarm = calloc(MAX_LS_ALARM, sizeof(long));
	lsMem->maxCalls = 1000;

	/* Initialize the fd list */
	NetFdsInit(&lsnetfds);
	NetFdsInit(&lsinfds);
	NetFdsInit(&lstimerfds);

	NetFdsZero(&lsnetfds);
	NetFdsZero(&lsinfds);
	NetFdsZero(&lstimerfds);

	timerLibInit();

	timerInit(&localConfig.timerPrivate, 10000, 0);

	tcbdata = TimerNotifyCBDataAlloc();

	/* Add the callback */
	setTimerNotifyCb(&localConfig.timerPrivate, IServerNotify, tcbdata);

	/* Open notification pipe */
	if (pipe(tcbdata->notifyPipe) < 0)
	{
		PrintErrorTime();
		fprintf(stderr, "main(): pipe error\n");
		perror("main() - pipe error ");
		return -1;
	}

	if((flags = fcntl(tcbdata->notifyPipe[NOTIFY_READ],F_GETFL,0)) <0)
	{
		PrintErrorTime();
		fprintf(stderr, "main(): fcntl notify read/get error\n");
	 	perror("main() - fcntl notify read/get error ");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(tcbdata->notifyPipe[NOTIFY_READ],F_SETFL,flags)) <0)
	{
		PrintErrorTime();
		fprintf(stderr, "main(): fcntl notify read/set error\n");
	 	perror("main() - fcntl notify read/set error ");
	 	return -1;
	}

	if((flags = fcntl(tcbdata->notifyPipe[NOTIFY_WRITE],F_GETFL,0)) <0)
	{
		PrintErrorTime();
		fprintf(stderr, "main(): fcntl notify write/get error\n");
	 	perror("main() - fcntl notify write/get error ");
	 	return -1;
	}

	flags |= O_NONBLOCK;

	if((fcntl(tcbdata->notifyPipe[NOTIFY_WRITE],F_SETFL,flags)) <0)
	{
		PrintErrorTime();
		fprintf(stderr, "main(): fcntl notify write/set error\n");
	 	perror("main() - fcntl notify write/set error ");
	 	return -1;
	}

	/* Open a notification pipe */
	NetFdsAdd(&lstimerfds, tcbdata->notifyPipe[NOTIFY_READ], FD_READ, 
		(NetFn) HandleNotify, (NetFn) NULL,
		tcbdata, NULL);

	SSIPInit();
	SSIPInitCont(cport);

	if (debug)
	{
		_sipSetTraceLevel(3);
	}

	NetFdsAdd(&lsinfds, 0, FD_READ, (NetFn) ReadInput, (NetFn) NULL,
			NULL, NULL);

	if (inputfd > 0)
	{
		NetFdsAdd(&lsinfds, inputfd, FD_READ, (NetFn) ReadInput, (NetFn) NULL,
			NULL, NULL);
	}

	sipThread = pthread_self();

	// Launch the timers thread
	LaunchThread(timerProcessThread, NULL);
	LaunchThread(inputProcessThread, NULL);
	LaunchThread(IcmpdInit, NULL);

	printStartState();

	if (mode & MODE_IXIA)
	{
		char ixiaCommand[IXIA_COMMAND_LEN];

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

	if (mode & MODE_MGCP)
	{
		// depending on how many calls we can handle,
		// we will have to wait for that many ports
		LaunchThread(mgenServerThread, NULL);
	}

	if (!(mode & MODE_MGCP))
	{
		LaunchThread(callgenThread, NULL);
	}

	
	mainLoop();
}

int
printStartState()
{
	fprintf(stdout,
            "\nTotalConcurrentCalls:    %d"
	    "\nTotalMaxCalls       :    %d"
            "\nCallRate(per sec)   :    %f"
            "\nBurstsInInterval    :    %d"
            "\nBurstInterval(msec) :    %d"
            "\nConnectTime(sec)    :    %d"
            "\nIdleTime(sec)       :    %d"
            "\nDropPktsProbability :    %f"
            "\nDropAcksProbability :    %f"
            "\nDropByesProbability :    %f"
            "\nCallingParty        :    %s"
            "\nCalledParty         :    %s"
            "\nIncCallingParty     :    %s"
            "\nIncCalledParty      :    %s"
            "\nGatewayIP           :    %s"
            "\nGatewayPort         :    %d"
            "\nFunctioningAs       :    %s"
            "\nAutoTestingMode     :    %s"
	    "\nASR Probability     :    %f"
            "\n",
   			nCalls, totalMaxCalls, call_rate,
			burst, burstInterval, 
            tdConnect, tdIdle,
			probability, aprobability, bprobability,
            startCallingPn, startCalledPn, 
            incCallingPn ? "yes" : "no", 
            incCalledPn ? "yes" : "no", 
    		gwAddr, gwPort, 
    		(mode & MODE_TRANSMIT) ? "transmitter" : "receiver",
    		automode ? "yes" : "no", iprobability);

	fprintf(stdout, ">\n");		// Ready to accept input
}

int
PrintUsage(void)
{
	fprintf(stdout, 
"sgen [-n <no. of calls>] - default 25\n");
	fprintf( stdout, 
"     [-h] - help for usage\n");
	fprintf(stdout, 
"     [-s[+] <calling party #>] - default 555 if transmitter, 666 if receiver; use + to increment number for multiple calls\n");
	fprintf(stdout, 
"     [-sf <src numbers file>] - source numbers in file should be equal or more than the number of calls specified using \'-n\' option. \n Each number should be on seperate line \n");
    fprintf(stdout,
"     [-d[+] <called party #>] - default 666 if transmitter, 555 if receiver; use + to increment number for multiple calls\n");
		fprintf(stdout, 
"     [-df <dst numbers file>] - destination numbers in file should be equal or more than the number of calls specified using \'-n\' option. \n Each number should be on seperate line \n");
    fprintf(stdout,
"     [-u <user@host:port>] - dialing using a uri.\n");
    fprintf(stdout,
"     [-D <final resp code> <user@host:port>] - used on receiver to send final response; if 200 < respcode < 400, contact needs to be specified)\n");
	fprintf( stdout, 
"     [-L <local SIP signaling IP>] - default equal to local IP\n");
	fprintf(stdout, 
"     [-l <local SIP signaling port>] - default 5060\n");
	fprintf( stdout, 
"     [-g <dest gateway IP>] - default equal to local IP\n");
	fprintf( stdout, 
"     [-p <dest gateway port>] - default 5060\n");
	fprintf(stdout, 
"     [-G] - send as many registration requests as number of calls; do not place any calls\n");
	fprintf(stdout, 
"     [-m <media IP> <media start port>] - defaults are local IP, port 49200\n");
    fprintf (stdout,
"     [-M <mgen port>] - default 49160\n");
	fprintf( stdout, 
"     [-t] - run as transmitter, default is receiver\n");
	fprintf( stdout, 
"     [-b <burst-size> <burst-interval>] - default 1 burst/100msec\n");
	fprintf( stdout, 
"     [-i <monitor> <setup> <connect> <idle>] - default 0 sec\n");
	fprintf(stdout, 
"     [-y[a|b] <pkt drop probability(-y)/acks(-ya)/byes(-yb)>] - default 0\n");
    fprintf (stdout,
"     [-E <SIP minSE value>]\n");
    fprintf (stdout,
"     [-S <session expires value>]\n");
    fprintf (stdout,
"     [-A <sip domain>]\n");
	fprintf(stdout, 
"     [-x] - fax\n");
	fprintf(stdout, 
"     [-j] - do not send SDP in Invite\n");
        fprintf(stdout,
"     [-J] - send initial Invite on Hold as per RFC 2543.\n");
	fprintf(stdout, 
"     [-k] - manual accept (no 200 OK)\n");
	fprintf(stdout,
"     [-K] - send initial Invite on Hold as per RFC 3264.\n");
	fprintf(stdout, 
"     [-X] - send trunk groups for vendor SonusGSX\n");
	fprintf(stdout, 
"     [-o <orig trunk group>] - e.g. -o otgrp (SonusGSX) & -o local=otgrp (non-SonusGSX)\n");
	fprintf(stdout, 
"     [-O <dest trunk group>] - e.g. -O dtgrp (SonusGSX) & -O local=dtgrp (non-SonusGSX)\n");
	fprintf(stdout, 
"     [-e <xthreads> <threadstack>]\n");
	fprintf(stdout, 
"     [-q <input file name>]\n");
	fprintf(stdout, 
"     [-r <level> ] - debug level\n");
	fprintf(stdout, 
"     [-z] - run as daemon\n");
	fprintf(stdout, 
"     [-c] - if running as daemon, wait for media gen, otherwise start it\n");
	fprintf(stdout,
"    [-U <probability>] - (ASR probability (<1) - default 0\n");
        fprintf(stdout,
"    [-v] - version number\n");
        fprintf(stdout,
"    [-N[+] <number>] - Maximum number of Calls; use + to maintain calls for specified call duration.\n");
	fprintf(stdout,
"    [-ep] - if running as a terminal endpoint.\n");
	fprintf(stdout,
"    [-spr <privacy id>] - RFC 3325 mode.\n");
	fprintf(stdout,
"    [-spd <proxy-require tag>] - Draft mode.\n");
	fprintf(stdout,
"    [-spb <privacy id>] - dual mode. If Tx, sends RFC 3325 format. If Rx, receives both formats. \n");
	fprintf(stdout,
"    [-obp <ip address of obp>] - Running the sgen in OBP mode. \n");
	fprintf(stdout,
"    [-sti] -Enable Sip-T + ISUP\n");
        fprintf(stdout,
"    [-stq] -Enable Sip-T + QSIG\n");
	fprintf(stdout, 
"    [-Gu] - send as many registration requests as number of calls and unregister when exit;\n Note: use \'quit\' to exit\n");
	fprintf(stdout, 
"    [-xfer] - attended xfer mode\n");
	fprintf(stdout, 
"    [-cm] - used for sgen to act as call manager. Mainly used for media to work in case of call transfers\n");

	PrintMgenOptions();
	fprintf(stdout, "\n");
	PrintInteractiveCmds();
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
	{
		PrintErrorTime();
		fprintf(stderr, "SignalInit(): sigaltstack error\n");
		perror("SignalInit() - sigaltstack error ");
	}

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
#endif // NETOID_LINUX
	sigaction(SIGFPE,	&sigact, NULL );
	sigaction(SIGILL,	&sigact, NULL );
	sigaction(SIGSEGV,	&sigact, NULL );
	sigaction(SIGCLD,	&sigact, NULL );

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

	memset( c_sig, (int) 0, sizeof(c_sig) );

	for (;;)
	{

		sigwait( &async_signal_mask, &signo );

		nx_sig2str( signo, c_sig, sizeof(c_sig));

		switch (signo)
		{
		case SIGINT:
			sig_int_ls(signo);
			break;

		case SIGTERM:
		exit(0);
			break;

		case SIGCLD:
			if( mode & MODE_IXIA )
			{
				IXIA_CLOSE();
				break;
			}

		case SIGHUP:
			//break;

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
//	int32_t thread = _lwp_self();
	int32_t thread =  pthread_self();
	char	c_sig[32];

	nx_sig2str( signo, c_sig, sizeof(c_sig) );

	switch (signo)
	{
	case SIGBUS:
#ifndef NETOID_LINUX
	case SIGEMT:
#endif // NETOID_LINUX
	case SIGFPE:
	case SIGILL:
	case SIGSEGV:
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

int
SgenGetFreeCallNo(int dir)
{
	Call *call;
	int i;

	if (dir == 1)
	{
		call = CallsOut;
	}
	else
	{
		call = Calls;
	}

	for (i=0; i<nCalls; i++)
	{
		if (call[i].state == Sip_sIdle)
		{
			return i;
		}
	}

	return -1;
}

int
mgenDeleteCall(char *callid) 
{
	CallHandle 		*callHandle = NULL;
	int cindex;

	CacheGetLocks(callCache, LOCK_READ, LOCK_BLOCK);
	callHandle = CacheGet(callCache, callid);
	cindex = callHandle->callNo;
	CacheReleaseLocks(callCache);

	/* appears to only be called for outgoing calls */ 
	//mgenInform(&CallsOut[cindex],0);
	if (mode & MODE_TRANSMIT)
	{
		mgenInform(&CallsOut[cindex], 0);
	}
	else
	{
		mgenInform(&Calls[cindex], 0);
	}
}

int sgenInform(SipEventHandle *evb)
{
	if(evb == NULL)
	{
		return -1;
	}
	SipAppMsgHandle *appMsgHandle = NULL;
	static int nbyes=0;

	if((appMsgHandle = SipEventAppHandle(evb)) == NULL)
	{
		return -1;
	}

	/* Check what type of packet was received on the network */
	switch(evb->event)
	{
		case Sip_eNetworkBye:	
			/* At this point, 200 Ok for the Bye received would have already been sent */
			callsdisconnected++;
			break;
	}
}
	
int
mgenInform(Call *call, int command)
{
	MgenControlMsg mcm;
	int mfd;
	static int nxia;

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

			if( ((nxia % IXIA_BATCH_LEN) == 0) || (nxia >= nCalls) )
			{
	
				IXIA_DEBUG( "Calling ixiaSendAccumCalls()\n" );
				ixiaSendAccumCalls();

				IXIA_WAIT_DONE(  );
			}
		}
		else
		{
			printf ("statics generated by ixia\n");
			return (0);
		}
	}

	else if( mode & MODE_MGCP )
	{
		// Command 0 is sent to mgen to stop media for a call
		// Command 1 is sent to mgen to begin media for a call
		// Command 2 is sent to mgen to get its media statistics

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
		else if (command == 2 || command == 3)
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
			return -1;
		}
	}
	else
	{
		return( -1 );
	}
	return 0;
}

int
AddDefaultRealm()
{
	RealmEntry		 defaultEntry;
	CacheRealmEntry *cacheRealmEntry;

	memset(&defaultEntry, 0, sizeof(RealmEntry));

	nx_strlcpy(defaultEntry.realmName, "default", REALM_NAME_LEN);
	defaultEntry.adminStatus = 1;
	defaultEntry.rsa = localIP;
	defaultEntry.realmId = 0;
	defaultEntry.addrType = ENDPOINT_PUBLIC_ADDRESS;
	defaultEntry.intraRealm_mr = MEDIA_ROUTING_DONT_CARE;
	defaultEntry.flags |= REALMF_DEFAULT;
	cacheRealmEntry = CacheDupRealmEntry(&defaultEntry);

	/* lock the cache */
	CacheGetLocks(realmCache, LOCK_WRITE, LOCK_BLOCK);

	if (AddRealm(cacheRealmEntry) < 0)
	{
		NETERROR(MCACHE, ("Could not add default realm\n"));
	}

	CacheReleaseLocks(realmCache);
}

int
ParseArguments(int argc, char **argv)
{
	char hname[256];
	int nargs = argc;
	int herror=0;
	char *contactPortStr = NULL;
	int localipFlag = 0, chanlocaladdrFlag = 0; 
	int sipdomainFlag = 0, gwaddrFlag = 0;

	// Parse arguments
    while (--argc > 0)
    {
		if ((argv[argc][0] == '-') || (argv[argc][0] == '+'))
		{
	  		switch (argv[argc][1])
	  		{
	  		case 'h':
				PrintUsage();
				exit(0);

	  		case 'n':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
	       			nCalls = atoi(argv[argc + 1]);
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

	  		case 's':
	   			if (argv[argc][2] == '+')
	   			{
					incCallingPn = 1;
	   			}
				if (argv[argc][2] == 'p')
                                {
                              	 	// In SipPrivacy Mode.
                               		// Check if in RFC or DRAFT mode
                                	if (argv[argc][3] == 'r')
                                	{
                                   		sipPrivRfcMode = 1;

                                   		if (argv[argc + 1])
							nx_strlcpy(privTag, argv[argc+1], 256);
							
                                        }
                                        else if(argv[argc][3] == 'd')
                                        {
                                        	sipPrivDftMode = 1;

                                               	if (argv[argc + 1])
                                                        nx_strlcpy(privTag, argv[argc+1], 256);

                                        }
					else if(argv[argc][3] == 'b')
                                        {
                                                sipPrivDualMode = 1;

                                                if (argv[argc + 1])
                                                        nx_strlcpy(privTag, argv[argc+1], 256);

                                        }
                                 }

				else if(argv[argc][2] == 'f')
				{
					if(argv[argc + 1] && (argv[argc + 1][0] != '+') && (argv[argc + 1][0] != '-'))
					{
						if((srcfp = fopen(argv[argc + 1],"r"))==NULL)
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
				}
				if (argv[argc][2] == 't')
				{
					// SIP-T enabled
					if (argv[argc][3] == 'i')
						isupMode = 1;
					if (argv[argc][3] == 'q')
						qsigMode = 1;
				}	
	       		break;

	  		case 'd':
	   			if (argv[argc][2] == '+')
	   			{
					incCalledPn = 1;
	   			}
				if(argv[argc][2] =='f')
				{
					if(argv[argc + 1] && (argv[argc + 1][0] != '+') && (argv[argc + 1][0] != '-'))
					{
						if((dstfp = fopen(argv[argc + 1],"r"))==NULL)
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
				}
	       		break;

	  		case 'A':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy (sipdomain, argv[argc + 1], SIPDOMAIN_LEN);
					sipdomainFlag = 1;
				}
				break;

	  		case 'D':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
       				finalRespCode = atoi(argv[argc + 1]);
					if ((finalRespCode > 200) && (finalRespCode < 400))
					{
						// we need a contact to be specified here
						// contact will be in the form of a user@address:port
						if (argv[argc + 2] && 
							(argv[argc + 2][0] != '+') && 
							(argv[argc + 2][0] != '-'))
						{
							nx_strlcpy(contact, argv[argc + 2], 256);
							contactUser = strtok_r(contact, "@", &contactHost);
							if (!contactUser || !contactHost)
							{
								PrintErrorTime();
								fprintf(stderr, "ParseArguments(): ");
								fprintf(stderr, "No user found in contact, "
										"final reponse option ignored\n");
	   		    				finalRespCode = 0;
							}
							else
							{
								contactHost = 
									strtok_r(contactHost, ":", &contactPortStr);
								if (contactPortStr)
								{
									contactPort = atoi(contactPortStr);
								}
							}
						}
					}
				}
	       		break;

	  		case 'l':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
	       			cport = atoi(argv[argc + 1]);
				}
	       		break;

	  		case 'e':
				if (argv[argc][2] == 'p')
				{
       					termEpMode = 1;
                                       	// fprintf(stdout, "\n in termep mode");
					
                                }
	
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					xthreads = atoi(argv[argc + 1]);
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						threadstack = atoi(argv[argc + 2]);
					}
				}
				break;

	  		case 'E':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
			  		sipminSE = atoi (argv[argc + 1]);
				}
		  		break;

	  		case 'g':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
		   			nx_strlcpy(gwAddr, argv[argc + 1], 256);
					gwaddrFlag = 1;
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

			case 'L':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					if((isalpha(argv[argc+1][0])))
					{
						localIP = ResolveDNS(argv[argc + 1], &herror);
						if(herror)
						{
							fprintf(stdout,"Error resolving name %s\n",argv[argc + 1]);
							exit(1);
						}
					}
					else
					{
						localIP = ntohl(inet_addr(argv[argc + 1]));
					}
					localipFlag = 1;
				}
	       		break;

	  		case 'm':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
	       			chanLocalAddr = ntohl(inet_addr(argv[argc + 1]));
					chanlocaladdrFlag = 1;
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
								tdCallDuration = tdIdle + tdConnect;
							}
						}
					}
				}
	       		break;

	  		case 'o':
				 if (argv[argc][2] == 'b')
                                {
					inviteType |= INVITE_THRU_OBP;
					nx_strlcpy(SgenGwIpAddr, argv[argc + 1], 256);
					GenMode = 1;

                                }

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

	  		case 'q':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(inputfilename, argv[argc + 1], 256);
				}
				break;

	  		case 'r':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					debug = atoi(argv[argc + 1]);
				}
	       		break;

	  		case 'S':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
		  			sipsessionexpiry = atoi(argv[argc + 1]);
				}
		  		break;

	  		case 'y':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
	  				if (strlen(argv[argc]) > 2)
					{
	  					switch (argv[argc][2])
						{
						case 'a':
							aprobability = atof(argv[argc + 1]);
							break;
						case 'b':
							bprobability = atof(argv[argc + 1]);
							break;
						default:
							break;
						}
					}
					else
					{
						probability = atof(argv[argc + 1]);
					}
				}
				break;

	  		case 'c':
	  			if (strlen(argv[argc]) > 2)
				{
					switch (argv[argc][2])
					{
						case 'm':						//call manager
							cm_mode = 1;
							break;
					}
				}
				else
				{
					mode |= MODE_MGCP;
					if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
					{
						nx_strlcpy(mgenFile, argv[argc + 1], 256);
					}
				}
				break;

			case 'P':
				if (argv[argc + 1] && 
					(argv[argc + 1][0] != '+') && 
					(argv[argc + 1][0] != '-'))
				{
					nx_strlcpy(mgenPayloadLen, argv[argc + 1], 8);
					if (argv[argc + 2] && 
						(argv[argc + 2][0] != '+') && 
						(argv[argc + 2][0] != '-'))
					{
						nx_strlcpy(mgenPayloadInterval, argv[argc + 2], 32);
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
				GenMode = 1;
	       			break;

	  		case 'G':
	       			mode |= MODE_REGISTER;
				if (argv[argc + 1] &&
                                        (argv[argc + 1][0] != '+') &&
                                        (argv[argc + 1][0] != '-'))
                                {
					// Default value of regExpires is 3600 
                                        regExpires = atoi(argv[argc + 1]);
                                }
				//Send an unregistration request while exiting
				if (argv[argc][2]== 'u')
				{
					unregWhenExit=1;
				}
	       			break;

	 	 	case 'j':
				inviteType |= INVITE_NOSDP;
				break;

			case 'J':
				holdType |= INVITE_HOLD;
				break;

	  		case 'k':
		   		manualAccept = 1;
		   		break;

			case 'K':
				holdType |= INVITE_HOLD_3264;
                                break;

		 	case 'X':
				vendorSonusGSX = 1;
	   			break;

	  		case 'x':
				if (argv[argc][2] == 'f')
				{
					setModeTransfer = 1;
				}
				else
				{
					fax = 1;
				}
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

			 case 'u': 
				//option added for URI dialing
				inviteType |= INVITE_URI_DIAL;
		
                                if (argv[argc + 1] &&
                                	(argv[argc + 1][0] != '+') &&
                                	(argv[argc + 1][0] != '-'))
                                       {
                                       		nx_strlcpy(contact, argv[argc + 1], 256);
                                                uriUser = strtok_r(contact, "@", &uriHost);
                                                if (!uriUser || !uriHost)
                                                {
                                                 	PrintErrorTime();
                                                        fprintf(stderr, "ParseArguments(): ");
                                                        fprintf(stderr, "Not a valid uri format\
																(user@host[:port])\n");
														exit(1);
                                                }
                                                else
                                                {
                                                	uriHost =
                                                                 strtok_r(uriHost, ":", &contactPortStr);
                                                                if (contactPortStr)
                                                                {
                                                                        uriPort = atoi(contactPortStr);
                                                                }
                                                        }
                                                }

				break;
			
			 case 'N':
                                maxCallsMode = 1;
				if (argv[argc][2] == '+')
                                {
                                        retainCalls = 1;
                                }


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

			case 'I':
				// Ixia option
				IXIA_DEBUG( "Ixia mode detected\n" );
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


	  		default:
	       		break;
	  		}
		}
	}

	// update ip addresses
	if (!localipFlag)
	{
		gethostname(hname, sizeof(hname));
		localIP = ResolveDNS(hname, &herror);
	}
	if (!gwaddrFlag)
	{
	    nx_strlcpy(gwAddr, ULIPtostring(localIP), 16);
	}
	if (!sipdomainFlag)
	{
		nx_strlcpy(sipdomain, ULIPtostring(localIP), SIPDOMAIN_LEN);
	}
	if (!chanlocaladdrFlag)
	{
		chanLocalAddr = localIP;
	}

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

	initCallId(localIP);

	// update other call parameters
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

	if (!call_rate) 
	{
		call_rate = 10;
	}

	if (!burst)
	{
		burstInterval = 100;
		burst = (call_rate*burstInterval)/1000;
	}

	callTimePeriod = ((hrtime_t)burstInterval*1000000)/((hrtime_t)burst);

	// When in registration mode, send out nCalls number of registration
	// requests. Do not make any calls. Ignore call distribution values.
	if (mode & MODE_REGISTER)
	{
		tdSetup = 0;
		tdCallDuration = 0;
		tdIdle = 0;
		tdConnect = 0;
	}

	return 0;
}

static int
InitCalls(void)
{
	int i, j;

	callingpna = (char **)calloc(nCalls, sizeof(char *));
	calledpna = (char **)calloc(nCalls, sizeof(char *));
	callIDIn = (char **)calloc(nCalls, sizeof(char *));
	callIDOut = (char **)calloc((nCalls), sizeof(char *));

	for (i=0; i<nCalls; i++)
	{
		callingpna[i] = (char *) calloc(256, sizeof(char));
		if (incCallingPn)
		{
			nx_strlcpy(callingpna[i], callingpn, 256);
			getNextPhone(callingpn);
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

	for (i=0; i<nCalls; i++)
	{
		calledpna[i] = (char *) calloc(256, sizeof(char));
		if (incCalledPn)
		{
			nx_strlcpy(calledpna[i], calledpn, 256);
			getNextPhone(calledpn);
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

	// open the input file
	if (strlen(inputfilename))
	{
		if ((inputfd = open(inputfilename, O_RDONLY)) < 0)
		{
			PrintErrorTime();
			fprintf(stderr, "InitCalls(): Could not open file %s\n", inputfilename);
		}
	}

	// Allocate heap space for calls specified - both outgoing and incoming
	Calls = (Call *) calloc(nCalls, sizeof(Call));
	CallsOut = (Call *) calloc(nCalls, sizeof(Call));

	for (i = 0; i < nCalls; i++)
	{
		Calls[i].cNum = i;
		Calls[i].state = Sip_sIdle;
		Calls[i].chIn[0].chNum = 0;
		Calls[i].chIn[0].port = chanLocalPort;
		Calls[i].chIn[0].ip = chanLocalAddr;

		CallsOut[i].cNum = i;
		CallsOut[i].state = Sip_sIdle;
		CallsOut[i].tsSetup = 0;
		CallsOut[i].tsConnect = 0;
		CallsOut[i].tsIdle = 0;
		CallsOut[i].chOut[0].chNum = 0;
		CallsOut[i].chIn[0].port = chanLocalPort;
		CallsOut[i].chIn[0].ip = chanLocalAddr;

		chanLocalPort += 2;
	}
	
	if (i > 0)
	{
		chanLocalPort -= 2;
	}
}

static int
CheckLicense(void)
{
	if(license_init() < 0)
	{
		// hack to make it work on compile m/c
		if (gethostid() != hostid)
		{
			PrintErrorTime();
			fprintf(stderr, "CheckLicense(): ");
			fprintf(stderr, "Missing/invalid/expired license...exiting\n");
			exit(1);
		}
	}
	else if(!genEnabled())
	{
		PrintErrorTime();
		fprintf(stderr, "CheckLicense(): ");
		fprintf(stderr, "Gen feature not in license file...exiting\n");
		exit(1);
	}
}

static int 
PrintErrorTime(void)
{
    static time_t errorTime;         
    static char errorTimeStr[120];      

    time(&errorTime);
//    cftime(errorTimeStr, "%Y/%m/%d %T", &errorTime); Replacing with POSIX strftime
	strftime(errorTimeStr, sizeof(errorTimeStr), "%Y/%m/%d %T", localtime(&errorTime));
    fprintf(stderr, "[%s] ", errorTimeStr);    
    return 0;
}

static void
PrintMgenOptions()
{
	fprintf(stdout, "\nOptions to pass to mgen if it is started from sgen:\n" );
	fprintf(stdout, "  [-B] - send and receive (loopback)\n");
	fprintf(stdout, "  [-R] - send and receive on same port\n");
	fprintf(stdout, "  [-Q <no. of media threads>] - default 1\n");
	fprintf(stdout, "  [-P <payload length (bytes)> <payload interval (nanosec)>] - default 60, 90000000\n");
}

static void
PrintInteractiveCmds(void)
{
	fprintf(stdout, "Interactive commands:\n");
	fprintf(stdout, "  hold : Send a hold reinvite\n");
	fprintf(stdout, "  hold-3264 : Send a hold reinvite as per RFC3264\n");
	fprintf(stdout, "  resume : Send a reinvite to release the hold\n");
	fprintf(stdout, "  reinvite : Send a reinvite with new media port\n");
	fprintf(stdout, "  reinvite-sameport : Send a reinvite with same media port\n");
	fprintf(stdout, "  reinvite-nosdp  [media ip addr (dot notation)  media port (optional)]: Send a reinvite with no SDP. \n");
	fprintf(stdout, "  Note: If specifying media address, media port also needs to be specified. \n");
	fprintf(stdout, "  reinvite-newsdp [media ip addr (dot notation)  media port] : Send a reinvite with new SDP\n");
	fprintf(stdout, "  dtmf [signal (default 1)] [duration (default 200msec)] : Send an out-of-band signal DTMF\n");
	fprintf(stdout, "  current-calls : Return the number of current connected calls.\n");
	fprintf(stdout, "  ss7signal : Send a ss7-signal (use when in ISUP/QSIG mode only).\n");
	fprintf(stdout, "  transfer unatt <num>  : Perform an unattended(blind) transfer to <num>.\n");
	fprintf(stdout, "  transfer unattw <num>  : Perform an unattended(blind) transfer to <num> and wait for Notify.\n");
	fprintf(stdout, "  transfer att <num>  : Perform an attended transfer to <num>.\n");
	fprintf(stdout, "  transfer final <num>  : Complete an attended transfer to <num>.\n");
	fprintf(stdout, "  transfer abandon <num>  : Abandon an attended transfer to <num>.\n");
	fprintf(stdout, "  sleep [duration (in seconds)] : Sleep (default 1 sec)\n");
	fprintf(stdout, "  mstat [Rx/Tx] : If media gen is started from sgen, display its statistics\n");
	fprintf(stdout, "  bye : Attempt to tear down calls\n");
	fprintf(stdout, "  stop : Attempt to tear down calls (exit if cmd is repeated)\n");
	fprintf(stdout, "  exit : Exit without tearing down calls\n");
	fprintf(stdout, "  help : Prints interactive commands\n");
}

static pid_t
StartMgen()
{
	pid_t	pid;
	char localIPStr[16];
	char nCallsStr[16];
	char mgenPortStr[8];
	char mediaStartPortStr[8];

	pid = fork();		// Fork a child process

	if (pid == 0)		// In child process
	{
		// Get the arguments to be passed to mgen
		nx_strlcpy(localIPStr, ULIPtostring(localIP), 16);
		snprintf(nCallsStr, sizeof (nCallsStr), "%d", nCalls);
		snprintf(mgenPortStr, sizeof (mgenPortStr), "%d", mgenPort);
		snprintf(mediaStartPortStr, sizeof (mediaStartPortStr), "%d", 
			 	 mediaStartPort);

		// Redirect or close necessary file descriptors
		freopen("./mgenout.log", "a", stdout);
		freopen("./mgenerr.log", "a", stderr);
		close(0);

		// Start mgen
		execlp(mgenFile, mgenFile, "-T", "-z", 
			  "-n", nCallsStr, "-L", localIPStr, "-c", localIPStr, 
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

int
PrintMgenStats()
{
	Call *call = NULL;
	int i = 0;
	if(((blindXferMode && unatt_transfer_status == 1) || (attXferMode && att_transfer_status == 1)) && CallsOut)
	{
		/* Once the call transfer is requested (REFER received), sgen starts acting as a transmitter */
		call = CallsOut;
	}
	else if ((mode & MODE_TRANSMIT) && CallsOut)
	{
		call = CallsOut;
	}
	else if((mode & MODE_RECEIVE) && Calls)
	{
		call = Calls;
	}
	if (transferFlag && att_transfer_status)
	{
		i = 1;
	}
	/* If the receiver's media stats are requested */
	if(call && mstatArg && (strncmp(mstatArg,"Rx",2)==0))
	{
		/* Check whether media is beig received from the IP specified in SDP. Though the Ip specified in the
		 * connection line of SDP(in INVITE/200 OK) is the IP where the  proxy/destination UA will be receiving
		 * media,  But there is no way to know at signaling
		 * level where the proxy/other UA will be sending media out from. So, here we are making a big
		 * assumption(always true in case of Nextone MSW, if using NSF) that Media firewall will use same IP 
		 * for sending and
		 * receiving media to/from UA. This is true most of the time, but we need to check this in case of
		 * Brookrout/Snowshore firewall  */
		if (call[i].chOut[0].ip == mgenStats.receivingFromIp)
		{
			/* In case of call transfer, if MSW is involved, call gen will receive media from the same IP (MSW's media realm), but from a different
			 * port because it is a new call between call transferee and transfer target. It is necessary to check the port to verify that the
			 * media is flowing after call transfer. The following check help to verify media on call tranferor and call tranferee  
			 */
			if((blindXferMode && unatt_transfer_status) || attXferMode || (transferFlag && att_transfer_status))
			{
				/* after call is transfered, media should be received from a different. The following check is very much bound to MSW because when a proxy is
				 * involved in call transfer, we might still receive media from same IP(in case of Nextone MSW at least; if call tranferee, call
				 * transferor and transfer target were all configured on same realm) but it should definitely be different port */
				if(mgenStats.fromPortChanged)	
				{
					fprintf(stdout,"%lld bytes from %s: %d after call transfer\n",mgenStats.rxBytes, ULIPtostring(mgenStats.receivingFromIp),mgenStats.receivingFromPort);
				}
				else
				{
					fprintf(stdout,"0 bytes after call transfer\n");
				}
			}
			else 	
			{
				fprintf(stdout,"%lld bytes from %s: %d\n",mgenStats.rxBytes, ULIPtostring(mgenStats.receivingFromIp),mgenStats.receivingFromPort);
			}
		}
		else
		{
#if 0
			fprintf(stdout,"0 bytes from %s ; %lld from %s\n",ULIPtostring(call[0].chOut[0].ip),
				   	mgenStats.rxBytes, ULIPtostring(mgenStats.receivingFromIp));
#endif
			//fprintf(stdout,"0 bytes from %s\n",ULIPtostring(call[i].chOut[0].ip));
			fprintf(stdout,"0 bytes from %lld. %lld from %lld\n",call[i].chOut[0].ip, mgenStats.rxBytes, mgenStats.receivingFromIp);
		}

		goto _return;
	}
	/* If the sender's media stats are requested */
	if(mstatArg && (strncmp(mstatArg,"Tx",2)==0))
	{
		fprintf(stdout,"%lld bytes to %s: %d \n",mgenStats.txBytes, ULIPtostring(mgenStats.sentToIp), mgenStats.sentToPort);
		goto _return;
	}
	fprintf(stdout, 
			  "TxBytes             :    %lld"
			"\nTxPkts              :    %lld"
			"\nTxBitRate(Kbits/s)  :    %f"
			"\nTxPktRate(Kpkts/s)  :    %f"
			"\nRxBytes             :    %lld"
			"\nRxPkts              :    %lld"
			"\nRxBitRate(Kbits/s)  :    %f"
			"\nRxPktRate(Kpkts/s)  :    %f"
   		    "\n",
			mgenStats.txBytes, mgenStats.txPkts,
			mgenStats.txBitRate, mgenStats.txPktRate,
			mgenStats.rxBytes, mgenStats.rxPkts,
			mgenStats.rxBitRate, mgenStats.rxPktRate);

_return:
	fprintf(stdout, ">\n");		// Ready to accept input
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

static void
GenerateSeed()
{
    time_t seconds;

    time(&seconds);

    srand48((unsigned int)seconds);
}

