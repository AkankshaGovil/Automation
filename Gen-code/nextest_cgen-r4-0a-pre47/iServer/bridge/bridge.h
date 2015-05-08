#ifndef _bridge_h_
#define _bridge_h_

#include "gis.h"
#include <malloc.h>

#include "callconf.h"
#include "sipcall.h"
#include "uh323inc.h"
#include "iwfsm.h"
#include "packets.h"
#include <netdb.h>
#include <malloc.h>
#include <errno.h>
#include "net.h"

#define SipEventNeedsFCE(event) ( \
			((event) == Sip_eBridgeInvite) || \
			((event) == Sip_eBridge200) || \
			((event) == Sip_eBridge1xx) || \
			((event) == Sip_eBridge3xx) || \
			((event) == Sip_eBridgeAck) \
			)

#define SipEventFinalForLeg(event, callHandle) ( \
		((event) == Sip_eBridgeBye) || \
		((event) == Sip_eBridgeCancel) || \
		(((event) == Sip_eBridgeFinalResponse) && !callHandle) || \
		((event) == Sip_eBridgeNoResponseError) || \
		((event) == Sip_eBridgeError) \
		) 

#define H323EventNeedsFCE(event) ( \
		((event) == SCC_eBridgeSetup) || \
		((event) == SCC_eBridgeAlerting) || \
		((event) == SCC_eBridgeProceeding) || \
		((event) == SCC_eBridgeProgress) || \
		((event) == SCC_eBridgeOLC) || \
		((event) == SCC_eBridgeChanConnect) || \
		((event) == SCC_eBridgeConnect) \
		)

extern char zeroCallID[CALL_ID_LEN];

int FindH323CpnPOTS(SCC_EventBlock *evtPtr);
int FindSipCpnPOTS(SCC_EventBlock *evtPtr);
int bridgeH323EventProcessor(SCC_EventBlock *);
int sipBridgeEventProcessor(SCC_EventBlock *);
ConfType DetermineConfType(SCC_CallHandleType c1,SCC_CallHandleType c2);

int bridgeSipResolveCache( 
		CallHandle *callHandle, 
		SipAppCallHandle *pSipData,
		int *callError,
		InfoEntry *sinfo,
		InfoEntry *dinfo
	);

int bridgeResolveH323Cache(
		CallHandle *callHandle, 
		H323EventData *pH323Data,
		int reservePort,
		int resolveRemote, 
		int *callError,
		InfoEntry *sinfo,
		InfoEntry *dinfo);

#if 0
int
openFirewallOnSipEvents (SCC_EventBlock *evtPtr,
			     CallHandle *callHandle1,
			     CallHandle *callHandle2,
			     ConfHandle *confHandle);

#endif

FCEStatusStruct
openPinholes (SCC_EventBlock *evtPtr, CallHandle *callHandle1, CallHandle *callHandle2, ConfHandle *confHandle);

#define openFirewallOnSipEvents openPinholes
#define openFirewallOnH323Events openPinholes

#define reopenFirewallOnSipEvents reopenPinholes
#define reopenFirewallOnH323Events reopenPinholes

int CallFceRxPortUsed(CallHandle *);
int CallFceRxPortFreed(CallHandle *);
int CallFceTxPortUsed(CallHandle *);
int CallFceTxPortFreed(CallHandle *);

CallRealmInfo *getRealmInfo (int realmId, int mallocfn);

int bridgeSipEventProcessorWorker(SCC_EventBlock *evtPtr);
int bridgeSipEventProcessor(SCC_EventBlock *evtPtr);
int bridgeH323EventProcessorWorker(SCC_EventBlock *evtPtr);

void bridgeFceH323Callback (SCC_EventBlock *evtPtr);
void bridgeFceSipCallback (SCC_EventBlock *evtPtr);

int getConfAndPeerCallID(char * confID, char *callID, ConfHandle *conf, char *peerCallID);
int getPeerCallIDFromConf(char *callID, ConfHandle *conf, char *peerCallID);
int getPeerCallID(char * confID, char *callID,char *peerCallID);

int CallTap(CallHandle 	*callHandle, int status);

CallHandle *GetMediaSrcFromTip(unsigned int *ip);

int bridgeSendH323Event (SCC_EventBlock *evtPtr);
void * BridgeEndCalls(void *arg);
void * BridgeInitCalls(void *arg);

int disconnectCall(char *callid, int sccError);
int disconnectCallAtMaxCallDuration(struct Timer *timer);

int BridgeCheckPendingFCEvents(CallHandle *callHandle);

extern char *GetSipBridgeEvent(int event);
extern char *GetH323Event(int event);

extern int allocateFCELicense (ConfHandle *confHandle);
extern int UpdateFCEInfo (SCC_EventBlock *evtPtr, CallHandle *callHandle1, CallHandle *callHandle2, ConfHandle *confHandle);

extern int bridgeQueueEvent (SCC_EventBlock *evtPtr);
extern void disconnectCallLeg (CallHandle *callHandle, int sccError);
extern int bridgeFindCallLegsInfoForEvent (SCC_EventBlock *evtPtr, CallHandle **callHandle1, CallHandle **callHandle2, ConfHandle **confHandle);
extern int RealmLocate (int srealmId, char *host, long unsigned int hostip, int *mydomain);
extern void AddCodecsAttribs (SipAppCallHandle *pSipData, CodecType *codecs, int numCodecs, char *(*AttribPairs)[2], int numAttribs, int vendor);
extern void RemoveCodecsAttribs (SipAppCallHandle *pSipData, CodecType *codecs, int numCodecs, char *(*AttribPairs)[2], int numAttribs, int vendor);

int BridgeInit(void);
int disconnectCallsOnRealm(unsigned long rsa, int sccError);
	
	

#endif /* bridge_h */
