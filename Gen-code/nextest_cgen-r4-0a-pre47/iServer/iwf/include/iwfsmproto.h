#ifndef _iwfsmproto_h_
#define _iwfsmproto_h_

#include "iwfsm.h"
#include "calldefs.h"
#include "sccdefs.h"
#include "uh323inc.h"

extern char * IWF_CallStateToStr(int state, char str[80]);
extern char * IWF_EventToStr(int state, char str[80]);

/* A Call leg is identified as either leg1 or leg2,
 * as an gateway/proxy would identify two links of
 * a logical call, which pass through it. Leg1, is
 * usually initiated by a remote, and leg2 is initiated
 * by the proxy.
 */

/* Action Routine Prototypes */

// Action routines for Incoming Events from H323 Network
int iwfH323Ignore(SCC_EventBlock *);
int iwfInitiateInvite(SCC_EventBlock *);
int iwfH323LogEvent(SCC_EventBlock *);
int iwfAlertingTo1xx(SCC_EventBlock *);
int iwfProceedingTo1xx(SCC_EventBlock *);
int iwfProgressTo1xx(SCC_EventBlock *);
int iwfConnectTo200Ok(SCC_EventBlock *);
int iwfChanConnectToAck(SCC_EventBlock *);
int iwfChanConnectTo200Ok(SCC_EventBlock *);
int iwfInitiateCancel(SCC_EventBlock *);
int iwfInitiateBye(SCC_EventBlock *);
int iwfInitiateAckBye(SCC_EventBlock *);
int iwfReInviteBridgeTCS(SCC_EventBlock *);
int iwfReInviteOLC(SCC_EventBlock *);
int iwfReInviteOLCAck(SCC_EventBlock *);
int iwfReInviteTCSAck(SCC_EventBlock *);
int iwfReInviteReqModeAck(SCC_EventBlock *);
int iwfInitiateFinalResponse(SCC_EventBlock *);
int iwfControlConnected(SCC_EventBlock *);
int iwfReqModeOLC(SCC_EventBlock *);
int iwfReqMode(SCC_EventBlock *);
int iwfCallConnectedTxConnected(SCC_EventBlock *);
int iwfConnectedNullTCS(SCC_EventBlock *);
int	iwfNullTCSCapabilitiesRx(SCC_EventBlock *);
int iwfNullTCSReleaseComp(SCC_EventBlock *evtPtr);
int iwfNullTCSOLC(SCC_EventBlock *);
int iwfReqModeOLCAck(SCC_EventBlock *);
int iwfAnnexFOLC(SCC_EventBlock *evtPtr);
int iwfAnnexFCLC(SCC_EventBlock *evtPtr);
int iwfCallConnectedCLC(SCC_EventBlock *evtPtr);

// Action routines for Incoming Events from Sip Network
int iwfSipIgnore(SCC_EventBlock *);
int iwfInitiateSetup(SCC_EventBlock *);
int iwfSipLogEvent(SCC_EventBlock *);
int iwfInitiateReleaseComp(SCC_EventBlock *);
int iwfCancelInitiateReleaseComp(SCC_EventBlock *);
int iwfInitiateAlerting(SCC_EventBlock *);
int iwfInitiateConnect(SCC_EventBlock *);
int iwfSipAck(SCC_EventBlock *);
int iwfConnectedReInvite(SCC_EventBlock *);
int iwfSipHeldReInvite(SCC_EventBlock *);
int iwfH323HeldReInvite(SCC_EventBlock *);
int iwfBothHeldReInvite(SCC_EventBlock *);
int iwfOLC2ChanConnect(SCC_EventBlock *);
int iwfHeldBySipTCSAck(SCC_EventBlock *);
int iwfCallConnectedReInvite(SCC_EventBlock *);
int iwfCallConnectedTCS(SCC_EventBlock *);
int iwfReInvite200Ok(SCC_EventBlock *);
int iwfReqMode200Ok(SCC_EventBlock *);
int iwfReInviteFinalResponse(SCC_EventBlock *);
int iwfReInviteRelComp(SCC_EventBlock *);
int iwfInitiateConnectOnNewOffer(SCC_EventBlock *);
int iwfNullTCS200Ok(SCC_EventBlock *);
int iwfAnnexF200Ok(SCC_EventBlock *evtPtr);
int iwfAnnexFReInvite(SCC_EventBlock *evtPtr);

// Action routines for Incoming Events from Sip Network

// For Each sm Event takers
int iwfSipNetworkEventProcessor(SCC_EventBlock *);
int iwfSipBridgeEventProcessor(SCC_EventBlock *);
int iwfH323NetworkEventProcessor(SCC_EventBlock *);
int iwfH323BridgeEventProcessor(SCC_EventBlock *);
int iwfRcvBridgeTcs(SCC_EventBlock *evtPtr);
int iwfRcvBridgeOlc(SCC_EventBlock *evtPtr);
int iwfRcvBridgeTcsAck(SCC_EventBlock *evtPtr);

// Utility routines to conver event to eventNames
char * iwfSipNetworkEvent2Str(int event,char str[EVENT_STRLEN]);

// Info functions...

int iwfHandleH323Dtmf (SCC_EventBlock *);
int iwfHandleSipInfo (SCC_EventBlock *);

void FreeH323EvtQueue(List *list);

int iwfSend200OkHold (char *sipCallID);
int iwfSend200OkHold (char *sipCallID);
int iwfSend200Ok (char *sipCallID);
int iwfSend200OkMedia (char *confID, char *sipCallID, RTPSet *localSet, int nlocalset);
int iwfSend200OkMedia (char *confID, char *sipCallID, RTPSet *localSet, int nlocalset);
int iwfSendCapabilities (char *confID, char *h323CallID, SipAppCallHandle *pSipData, 
		int nset);
int iwfSendBridgeOLC (char *confID, char *h323CallID, RTPSet *pRtpSet, int nset);
int iwfSendBridgeOLCAck (char *confID, char *h323CallID, RTPSet *pRtpSet, int nset);
int iwfSendBridgeCLC (char *confID, char *h323CallID, int sid);
int iwfSendBridgeOLCAck (char *confID, char *h323CallID, RTPSet *pRtpSet, int nset);
int iwfSendReqMode (char *confID, char *h323CallID, int h323MediaType, 
		SipAppCallHandle *pSipData);
int iwfSendReqModeAck (char *confID, char *h323CallID);
int iwfSendReqModeAck (char *confID, char *h323CallID);
int iwfSendFinalResponse (char *confID, char *sipCallID);
int iwfSendSipError (char *confID, char *sipCallID);
int iwfSendRelComp (char *confID, char *h323CallID);
int iwfSendAck (char *confID, char *sipCallID, RTPSet *localSet, int nlocalset);
int iwfSend200WithSDP (char *confID, char *sipCallID, RTPSet *localSet, int nlocalset);
void FreeH323EvtQueue (List *list);
void setDefaultRtpParams(RTPSet rtpSet[],int nset);
void sanitizeCodecs4sip(SipAppCallHandle *pSipData, int *pnset, int vendor);
char * sanitizeSipNumber(char *number);
void copySipRtpset2H323Data(H323EventData *pH323Data, RTPSet *localSet, 
		SipAppCallHandle *pSipData, int nset);
void AddG729AnnexbToAttr(SipAppCallHandle *pSipData, int flag, int vendor);
void CheckG729AnnexbAttr(SipAppCallHandle *pSipData, int *support);
char* SipDataCodec2StrForH323Stack(SipAppCallHandle *pSipData);
int  QueueH323Evt(List *list, void *item);
void *DeQueueH323Evt(List list);
int matchOlcItem(const void *v1, const void *v2);
int iwfSipEventProcessor(SCC_EventBlock *);
int iwfH323EventProcessor(SCC_EventBlock *);

int iwfInit();
	

#endif /* _iwfsm_h_ */
