#ifndef UA_H
#define UA_H
#include "gis.h"
#include "sipcall.h"

extern void SipRegStartRegSM (NetoidInfoEntry *infoEntry, char *auth);

extern int SipUASendToBridge (SipEventHandle *evb);
extern int sipBridgeEventProcessor (SCC_EventBlock *evPtr);

extern int SipUAProcessEventWorker (SipEventHandle *evHandle);
extern int SipUAProcessEvent (SipEventHandle *evHandle);
extern int SipUACheckStateForCallID (char *callID, int *state);
extern int SipUAChangeStateForCallID (SipEventHandle *evHandle, char *callID);
extern int SipUAChangeState (SipEventHandle *evHandle, CallHandle *callHandle);
extern int SipQueueEvent (SipEventHandle *evHandle, void *(*cfn) (void *));

void MFCPCallbackOnBkup(MFCP_Request *rPtr);
int openPinholeOnBkup(CallHandle *newCallh, CallHandle *oldCallh, 
                      ConfHandle *confh);
int CmpFceMediaHole(FceMediaHoleHandle *mh1, FceMediaHoleHandle *mh2);

extern void scmCallHandleUpdate (CallHandle *callHandle);
extern void scmCallHandleDelete (char *callid);

extern int SipGetEventFromMsgHandle (SipAppMsgHandle *appMsgHandle, int *smtype);
int SipUAInit();
int disconnectCallAtMaxCallDuration(struct Timer*);
int SipHandleTimerC(struct Timer*);
int SipHandleSessionTimer(struct Timer*);
int SipHandle491Timer(struct Timer*);
int processreplaces(char *tgt, char *buffer);

void parsereplaces(char* buffer, char* callid, char* fromtag, char* totag);
char* createAuth(NetoidInfoEntry *infoEntry, char *method, char *authenticate);
SipMsgHandle * SipBridgeCreateMsgHandle2(SipCallHandle *sipCallHandle, 
					 char *method, int msgType, 
                                         int respCode);
int ExtractContactHost(char *entry_contact, char *contacthostname, int* port);

#endif
