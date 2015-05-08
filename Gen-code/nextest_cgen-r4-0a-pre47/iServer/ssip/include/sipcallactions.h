#ifndef _sipcallactions_h_
#define _sipcallactions_h_

int SipHandleBridgeInvite(SipEventHandle *);
int SipHandleBridgeReInvite(SipEventHandle *);
int SipHandleBridgeAlerting(SipEventHandle *);
int SipHandleBridgeConnect(SipEventHandle *);
int SipHandleBridgeAck(SipEventHandle *);
int SipQueueBridgeInvite(SipEventHandle *);
int SipHandleBridgeBye(SipEventHandle *);
int SipHandleBridgeFinalResponse(SipEventHandle *);
int SipHandleBridgeCancel(SipEventHandle *);
int SipHandleBridgeInfo(SipEventHandle *);
int SipHandleBridgeInfoResponse(SipEventHandle *);
int SipHandleBridgeError(SipEventHandle *evb);
int SipHandleBridgeNoOp(SipEventHandle *evb);
int SipBridgeRestart491Timer(SipEventHandle *evb);
int SipHandleBridge491Expired(SipEventHandle *evb);
int SipHandleBridgeCExpired(SipEventHandle *evb);
int SipBridgeRestartSessionTimer(SipEventHandle *evb);
int SipHandleBridgeSessionExpired(SipEventHandle *evb);
int SipBridgeError408(SipEventHandle *evb);

int SipBridgeErrorCancel(SipEventHandle *evb);
int SipBridgeErrorBye(SipEventHandle *evb);
int SipBridgeError500(SipEventHandle *evb);
int SipBridgeError408(SipEventHandle *evb);
int SipBridgeError504(SipEventHandle *evb);
int SipHandleBridgeCExpired(SipEventHandle *evb);

int SipHandleNetworkInvite(SipEventHandle *);
int SipHandleNetworkReInvite(SipEventHandle *);
int SipHandleNetworkAlerting(SipEventHandle *);
int SipHandleNetworkConnect(SipEventHandle *);
int SipHandleNetworkAck(SipEventHandle *);
int SipHandleNetworkBye(SipEventHandle *);
int SipHandleNetworkFinalResponse(SipEventHandle *);
int SipHandleNetworkCancel(SipEventHandle *);
int SipHandleNetworkInfo(SipEventHandle *);
int SipHandleNetworkInfoResponse(SipEventHandle *);
int SipHandleNetwork3xx(SipEventHandle *evb);
int SipHandleNetworkErrorCancel(SipEventHandle *evb);
int SipHandleNetworkErrorBye(SipEventHandle *evb);
int SipHandleNetworkError500(SipEventHandle *evb);
int SipHandleNetworkError(SipEventHandle *evb);
int SipHandleNetworkNoOp(SipEventHandle *evb);
int SipQueueNetworkInvite(SipEventHandle *);

int SipSendNetwork491(SipEventHandle *evb);
int SipSendNetwork488(SipEventHandle *evb);
int SipHandleNetworkNoResponseError(SipEventHandle *evb);

int SipAuthCallDurationExpires(tid timerid);

extern void SipStopSessionTimer (CallHandle *callHandle);
extern void SipStopTimerC (CallHandle *callHandle);

extern int SipStartSessionTimer (CallHandle *callHandle);
extern int SipHandleNetwork422 (SipEventHandle *evb);
extern int SipHandleNetwork491 (SipEventHandle *evb);
extern int SipCloseCall (CallHandle *callHandle, int forceDelete);
extern int SipSendNetworkCancel (SipEventHandle *evb);
extern int SipSendNetworkBye (SipEventHandle *evb);
extern int SipSendNetworkFinalResponse (SipEventHandle *evb, int response);
extern int SipTerminateTrans2 (SipEventHandle *evb, 
                               SipCallHandle *sipCallHandle, char *method);
extern int SipTerminateTrans (SipEventHandle *evb);
extern int SipDetermineCallError (int responseCode);
extern int SipStartTimerC (CallHandle *callHandle);

int SipHandleNetworkRefer(SipEventHandle*);
int SipHandleBridgeRefer(SipEventHandle*);
int SipHandleNetwork202(SipEventHandle*);
int SipHandleBridge202(SipEventHandle*);
int SipHandleBridgeNotify(SipEventHandle*);
int SipHandleNetworkNotify(SipEventHandle*);
int SipHandleNetworkNotifyResponse(SipEventHandle*);
int SipHandleBridgeNotifyResponse(SipEventHandle*);
#endif /* _sipcallactions_h_ */
