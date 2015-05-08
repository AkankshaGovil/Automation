#ifndef _siptrans_h_
#define _siptrans_h_

#include "lock.h"

#define SipTranRequest(siptranptr) (((siptranptr)->request).msg)
#define SipTranResponse(siptranptr) (((siptranptr)->response).msg)
#define SipTranAck(siptranptr) (((siptranptr)->ack).msg)
#define SipTranNewresponse(siptranptr) (((siptranptr)->newresponse).msg)

#define SipTranRequestContext(siptranptr) (((siptranptr)->request).context)
#define SipTranResponseContext(siptranptr) (((siptranptr)->response).context)
#define SipTranAckContext(siptranptr) (((siptranptr)->ack).context)
#define SipTranNewresponseContext(siptranptr) (((siptranptr)->newresponse).context)

#define SipTranResponseCode(siptranptr) (((siptranptr)->response).statuscode)
#define SipTranNewresponseCode(siptranptr) (((siptranptr)->newresponse).statuscode)

#define SipTranRequestMethod(siptranptr) (((siptranptr)->request).method)
#define SipTranResponseMethod(siptranptr) (((siptranptr)->response).method)
#define SipTranNewresponseMethod(siptranptr) (((siptranptr)->newresponse).method)
#define SipTranAckMethod(siptranptr) (((siptranptr)->ack).method)

#define SipTranRequestSendhost(siptranptr) (((siptranptr)->request).sendhost)
#define SipTranResponseSendhost(siptranptr) (((siptranptr)->response).sendhost)
#define SipTranAckSendhost(siptranptr) (((siptranptr)->ack).sendhost)

#define SipTranRequestSendport(siptranptr) (((siptranptr)->request).sendport)
#define SipTranResponseSendport(siptranptr) (((siptranptr)->response).sendport)
#define SipTranAckSendport(siptranptr) (((siptranptr)->ack).sendport)

#define SipTranRequestSdp(siptranptr) (((siptranptr)->request).sdp)

#define SipTranTimer(siptranptr) ((siptranptr)->timerid)

typedef struct
{
	SipMessage		*msg;
	SipEventContext		*context;
	char			*method;	/* can be "RESPONSE" */
	int			statuscode;
	unsigned long		sendhost;
	unsigned short		sendport;
	int			sdp;
} SipMsgBlock;

typedef struct SipTrans
{
	/* Keep this as first entry, if not all Get
	 * routines need to specifically pass in a key.
	 */
	SipTransKey 	key;
    
	struct SipTrans	*prev, *next;
	
	SipMsgBlock	request;
	SipMsgBlock	response;
	SipMsgBlock	ack;
	SipMsgBlock	newresponse;

	int		currState;
	char		*StateName;
	int		event;
	char		*EventName;
	int		count_ReTx;
	int		done;
	int		error;
	int		(*CSMCallFn) (void *); 
	SipMessage	*CSMCallParam;

	tid		timerid;

	Lock	lock;
	List	msgs;
	int		inuse;

} SipTrans;

typedef struct
{
	unsigned long srcip;
	unsigned long destip;
	unsigned short destport;

} SipTransDestKey;

SipMessage *
SipTransGetMsg(SipTransKey *key, int msgType);

/* Called when an incoming message is received
 * from network, meant for the TSM
 */
int
SipTransIncomingMsg(
	SipMessage *msg, 
	char *method, 
	SipEventContext *context
);

/* Called when an incoming message is received
 * from network, meant for the TSM
 */
int
SipTransIncomingMsgForUADest(
	SipMessage *msg, 
	char *method, 
	SipEventContext *context,
	CacheTableInfo *cacheInfo
);

/* Gets the transaction corresponding to a message
 * This is not lock protected
 */
SipTrans *
SipTransGetTrans(SipMessage *msg);

#define TsmCheckState(siptranptr)
#if 0
		if ( siptranptr->currState < 0 || \
			 siptranptr->currState > SipInviteSM_ServerMaxStates ) \
		{ \
			NETERROR(	MSIP, \
						( "%s : Error current server Invite state out of range - %d\n", \
						fn, \
						siptranptr->currState )); \
		}
#endif

#define TsmCheckEvent(siptranptr)
#if 0
		if ( siptranptr->event < 0 || \
			 siptranptr->event > SipInviteSM_ServerMaxEvents ) \
		{ \
			NETERROR(	MSIP, \
						( "%s : Error current server Invite event out of range - %d\n", \
						fn, \
						siptranptr->currState )); \
		}
#endif

#define TsmAssignInvServerStateName(siptranptr, state)	\
		siptranptr->StateName = \
			(char*) &SipInviteSM_ServerStates[state][0];

#define TsmAssignOthServerStateName(siptranptr, state)	\
		siptranptr->StateName = \
			(char*) &SipByeCancelSM_ServerStates[state][0];

#define TsmAssignInvClientStateName(siptranptr, state)	\
		siptranptr->StateName = \
			(char*) &SipInviteSM_ClientStates[state][0];

#define TsmAssignOthClientStateName(siptranptr, state)	\
		siptranptr->StateName = \
			(char*) &SipByeCancelSM_ClientStates[state][0];

#define TsmAssignInfoServerStateName(siptranptr, state) \
                siptranptr->StateName = \
                        (char *)&SipInfoSM_ServerStates[state][0];

#define TsmAssignInfoClientStateName(siptranptr, state) \
                siptranptr->StateName = \
                        (char *)&SipInfoSM_ClientStates[state][0];

#define TsmAssignInvServerEventName(siptranptr, event)	\
		siptranptr->EventName = \
			(char*) &SipInviteSM_ServerEvents[event][0];

#define TsmAssignOthServerEventName(siptranptr, event)	\
		siptranptr->EventName = \
			(char*) &SipByeCancelSM_ServerEvents[event][0];

#define TsmAssignInvClientEventName(siptranptr, event)	\
		siptranptr->EventName = \
			(char*) &SipInviteSM_ClientEvents[event][0];

#define TsmAssignOthClientEventName(siptranptr, event)	\
		siptranptr->EventName = \
			(char*) &SipByeCancelSM_ClientEvents[event][0];

#define TsmAssignInfoServerEventName(siptranptr, event) \
                siptranptr->EventName = \
                        (char *)&SipInfoSM_ServerEvents[event][0];

#define TsmAssignInfoClientEventName(siptranptr, event) \
                siptranptr->EventName = \
                        (char *)&SipInfoSM_ClientEvents[event][0];
#endif /* _siptrans_h_ */
