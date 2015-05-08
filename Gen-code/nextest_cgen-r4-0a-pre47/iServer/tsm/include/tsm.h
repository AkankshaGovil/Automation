#ifndef _tsm_h_
#define _tsm_h_
/* for SipInviteSM_Server */
#define IDLE_STATE 0

typedef enum {
	SipInvite_ServerIdle=IDLE_STATE,
	SipInvite_ServerProc,
	SipInvite_ServerSuccess,	/* can be merged with failure state */
	SipInvite_ServerFailure,
	SipInvite_ServerConfirmed,
	SipInvite_ServerCompleted,
	SipInviteSM_ServerMaxStates,

	SipInvite_ClientIdle=IDLE_STATE,
	SipInvite_ClientCalling,
	SipInvite_ClientProc,
	SipInvite_ClientConfirmed,
	SipInvite_ClientCompleted,
	SipInviteSM_ClientMaxStates,

	SipByeCancel_ClientIdle=IDLE_STATE,
	SipByeCancel_ClientProc,
	SipByeCancel_ClientCompleted,
	SipByeCancelSM_ClientMaxStates,

	SipByeCancel_ServerIdle=IDLE_STATE,
	SipByeCancel_ServerProc,
	SipByeCancel_ServerConfirmed,
	SipByeCancel_ServerCompleted,
	SipByeCancelSM_ServerMaxStates,

	SipInfo_ClientIdle=IDLE_STATE,
	SipInfo_ClientProc,
	SipInfo_ClientCompleted,
	SipInfoSM_ClientMaxStates,

	SipInfo_ServerIdle=IDLE_STATE,
	SipInfo_ServerProc,
	SipInfo_ServerConfirmed,
	SipInfoSM_ServerMaxStates
} SipTranSMStates;

typedef enum {
	/* INVITE server s/m */
	SipRequestFrNet=0,
	Sip1xxFrCSM,
	Sip2xxFrCSM,
	Sip4xxFrCSM,
	SipAckFrNet,
	SipInviteServerReTxExpire,
	SipInviteServerTermExpire,
	SipInviteSM_ServerMaxEvents,

	/* INVITE Client s/m */
	SipRequestFrCSM=0,
	SipInviteClientReTxExpire,
	SipInvite7Sent,
	Sip1xxFrNet,
	SipFinalFrNet,
	SipAckFrCSM,
	SipInviteClientTermExpire,
	SipInviteSM_ClientMaxEvents,

	/* ByeCancel Client s/m */
	SipByeCancelFrCSM=0,
	SipByeCancelClientReTxExpire,
	SipByeCancel11Sent,
	SipByeCancel1xxFrNet,
	SipByeCancelFinalFrNet,
	SipByeCancelSM_ClientMaxEvents,

	/* ByeCancel Server s/m */
	SipByeCancelFrNet=0,
	SipByeCancel1xxFrCSM,
	SipByeCancelFinalFrCSM,
	SipByeCancelServerTermExpire,
	SipByeCancelSM_ServerMaxEvents,

	/* Info Client s/m */
	SipInfoFromCSM=0,
	SipInfoClientReTxExpire,
	SipInfoAllSent,
	SipInfo1xxFrNet,
	SipInfoFinalFrNet,
	SipInfoClientTermExpire,
	SipInfoSM_ClientMaxEvents,

	/* Info Server s/m */
	SipInfoFromNet=0,
	SipInfo1xxFrCSM,
	SipInfoFinalFrCSM,
	SipInfoServerTermExpire,
	SipInfoSM_ServerMaxEvents,
} SipTranSMEvents;

typedef int (*SipTranSMActions) (SipTrans *);

#define SipTranSM_MaxActions 6

/* 1 = Yes, 0 = No */
#define ALWAYS_RESPOND_WITH_200_TO_BYECANCEL 1
//#define ALWAYS_RESPOND_WITH_202_TO_REFER 1

typedef struct
{
	SipTranSMStates		nextState;
	SipTranSMActions	actions[SipTranSM_MaxActions];
} SipTranSMEntry;

extern SipTranSMEntry 
SipInviteSM_Server[SipInviteSM_ServerMaxStates][SipInviteSM_ServerMaxEvents];

extern SipTranSMEntry 
SipByeCancelSM_Server[SipByeCancelSM_ServerMaxStates][SipByeCancelSM_ServerMaxEvents];

extern SipTranSMEntry 
SipInfoSM_Server[SipInfoSM_ServerMaxStates][SipInfoSM_ServerMaxEvents];

extern SipTranSMEntry 
SipInviteSM_Client[SipInviteSM_ClientMaxStates][SipInviteSM_ClientMaxEvents];

extern SipTranSMEntry 
SipByeCancelSM_Client[SipByeCancelSM_ClientMaxStates][SipByeCancelSM_ClientMaxEvents];

extern SipTranSMEntry 
SipInfoSM_Client[SipInfoSM_ClientMaxStates][SipInfoSM_ClientMaxEvents];

extern char SipInviteSM_ClientStates[][32];
extern char SipInviteSM_ServerStates[][32];
extern char SipByeCancelSM_ServerStates[][32];
extern char SipByeCancelSM_ClientStates[][32];
extern char SipInfoSM_ServerStates[][32];
extern char SipInfoSM_ClientStates[][32];
extern char SipInviteSM_ClientEvents[][32];
extern char SipInviteSM_ServerEvents[][32];
extern char SipByeCancelSM_ServerEvents[][32];
extern char SipByeCancelSM_ClientEvents[][32];
extern char SipInfoSM_ServerEvents[][32];
extern char SipInfoSM_ClientEvents[][32];

/* actions prototype */
int SipSendResp(SipTrans *);
int SipNotifyCSM(SipTrans *);
int SipNotifyCSM2(SipTrans *);
int SipNop(SipTrans *);
int SipInviteServerTermTimer(SipTrans *);
int SipInviteServerReTxTimer(SipTrans *);
int SipAckCSM(SipTrans *);
int SipAckCSM2(SipTrans *);
int SipRemoveTSM(SipTrans *);
int SipSendXferMsg(SipTrans *);

/* invite Client side */
int SipSendRequest(SipTrans *);
int SipInviteClientReTxTimer(SipTrans *);
int SipDeleteTimer(SipTrans *);
int SipInviteClientTermTimer(SipTrans *);
int SipSendAck(SipTrans *);
int SipErrorCSM(SipTrans *);
int SipErrorCSM2(SipTrans *);
int SipRespCSM(SipTrans *);
int SipRespCSM2(SipTrans *);

/* ByeCancel Client Side */
int SipByeCancelClientReTxTimer(SipTrans *);
int SipByeCancelRespCSM2(SipTrans *);
int SipByeCancelErrorCSM2(SipTrans *);
int SipByeCancelRemoveTSM(SipTrans*);

/* ByeCancel Server Side */

typedef struct
{
	SipTrans *siptranptr;
	SipMessage *msg;
	struct Timer *timer;
	char *method;
	SipEventContext *context;
	int from;
	int newentry;
} SipTransOperateArg;

int SipByeCancelServerTermTimer(SipTrans *);
SipTransKey * TsmDupTsmKey(SipTransKey *siptrankey);
int TsmFreeTsmKey(SipTransKey *siptrankey);
int SipTransOperate( int from, SipTrans *siptranptr, 
	SipMessage *msg,
	struct Timer *timer,
	int newentry,
	char *method, 
	SipEventContext *context
);

/* Info Client Side */
int SipInfoClientReTxTimer(SipTrans *);
int SipInfoClientRespCSM(SipTrans *);
int SipInfoClientTermTimer(SipTrans *);
int SipInfoErrorCSM2(SipTrans *);

/* Info (and others?) Server Side */
int SipInfoServerTermTimer(SipTrans *);
int SipInfoNotifyError (SipTrans *);

/* Miscellaneous prototypes */
SipEventContext *SipDupContext (SipEventContext *context);
int SipFormRTPParamFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle);
int SipFormIsupFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle, SipHeader *pContentType, SipHeader *pContentDisp);
int SipFormQsigFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle, SipHeader *pContentType, SipHeader *pContentDisp);
int SipFormDtmfParamFromMsg (SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle);
int SipFormDtmfFromMsgHandle (SipMsgBody **msgbody, SipAppMsgHandle *appMsgHandle);
int SipFormSDPFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle);
int SipFormISUPFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle);
int SipFormQSIGFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle);
int SipTransOperateIncomingMsg(SipTrans *siptranptr, SipMessage *msg, int newentry,
		char *method, SipEventContext *context);
int SipTransOperateMsgHandle(SipTrans *siptranptr, SipMessage *msg, int newentry);
int SipTransOperateUnmarshall(SipTransOperateArg *arg);
int SipOperateIcmpDestUnreach( SipTrans *siptranptr);
int TranFreeQueue(SipTrans *siptranptr);
int SipGetMethod(SipMessage *msg, SIP_S8bit **pMethod);
int SipGetStatusCode(SipMessage *msg, int *pCode);
int SipTranSMProcessor(SipTrans *siptran, SipTranSMEntry *siptransm);
int SipFormMsgFromMsgHandle(SipMessage **s, SipAppMsgHandle *AppMsgHandle);
int SipTranKeyFromOutgoingMsg(SipMessage *s, SipTransKey *siptrankey, int nbytes);
int PrintSipTranKey(SipTransKey *keyptr);
SipTrans * SipAllocateTranHandle(void);
int SipFreeTranKeyMember(SipTransKey *siptrankey);
int SipFormCallRealmInfo(SipEventContext *context, SipAppMsgHandle *appMsgHandle);
int SipFormMsgHandleFromMsg(SipMessage *m, SipAppMsgHandle *appMsgHandle);
int SipTransProcIncomingMsg(SipMessage *msg, char *method, SipEventContext *context,
		CacheTableInfo *cacheInfo);
int SipTranKeyFromIncomingMsg(SipMessage *s, SipTransKey *siptrankey, int nbytes);
int SipTransOperateMarshall(int from, SipTrans *siptranptr, SipMessage *msg,
		struct Timer *timer, int newentry, char *method, SipEventContext *context);
int SipTranSendThreadLaunch(SipMessage *s, SipEventContext *context, int type, char *host,
		unsigned short port, unsigned long *sendhost, unsigned short *sendport);
int SipTranInformCSM(SipMessage *msg, SipEventContext *context);
int SipTransGetTransOrCallForIncomingMsg (SipMessage *msg);
int SipFormSDPBodyFromMsgHandle(SipMessage *m, SipAppMsgHandle *appMsgHandle, 
		SipMsgBody **pMsgbody);
int TsmLogCache(int where, int module, int level);
int SipFormTranKey(int type, header_url *remote, header_url *local, 
		   char *callid, int cseqno, char *method, 
		   SipTransKey *siptrankey);	

void SipTsmIcmpInit (void);
void DelTSMDest(SipTrans *siptranptr);
void AddTSMDest(SipTrans *siptranptr);

int SipFormSipFragFromMsg(SipMsgBody *msgbody, SipAppMsgHandle *appMsgHandle);

#endif /* _tsm_h_ */
