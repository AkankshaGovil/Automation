#ifndef _arq_h_
#define _arq_h_

#include <pthread.h>
#include "timedef.h"
#include "calldefs.h"
#include "sccdefs.h"
#include "callsm.h"
#include "confdefs.h"
#include "usipinc.h"
#include "ssip.h"
#include "codecs.h"
#include "mfcp.h"

#define FL_CPN_POTS				0x00000001
#define FL_CALL_TAP				0x00000002
#define FL_CALL_FAX				0x00000004
#define FL_CALL_CISCO_ANI		0x00000008
#define FL_CALL_FCPEND			0x00000010

// Specifically to do with stateless and MSW<->MSW calling
#define FL_CALL_NOSTATE			0x80000000
#define FL_CALL_NOSRCPORTALLOC	0x40000000
#define FL_CALL_NODSTPORTALLOC	0x20000000

#define FL_CALL_H245CTRLCONN	0x10000000	// H.245 control is connected

#define FL_CALL_RXRELCOMP		0x01000000	// Release complete is received

// These flags used only for FCE
#define FL_CALL_H245APPINIT		0x08000000	// H.245 App Handle has beeb initialized
											// Used only for FCE
#define FL_CALL_H245CTRLIN		0x04000000	// H.245 control channel dir is incoming
											// Used only for FCE
#define FL_FREE_DESTPORT		0x02000000	// Used for 3xx/LCF/ACF processing

#define MAX_LOGICAL_CHAN 3
typedef struct
{
	HCHAN				hsChan;
	int					dataTypeHandle;
	cmCapDataType		dataType;
	unsigned long		ip;
	unsigned short		port;
	int 				codec;
	int					param;
	unsigned long		rtcpip;
	unsigned short		rtcpport;
	int					active;
	int					mediaChange; /* This flag gets set if there is a change in rtp */
	int					flags; // follows values from flags fields RTPSet

} ChanInfo;
	
// SPLIT into protocol(H323/SIP) independent part and protocol dependant part
typedef struct
{
	char					callID[CALL_ID_LEN];
	char					confID[CONF_ID_LEN];
	unsigned int			fastStartError;
	unsigned int			flags;
	HPROTCONN 				h245Conn;
	int				localIP; /* host byte order */

} UH323CallAppHandle;

typedef struct
{
	CodecType		codecType;

	unsigned long	rtpaddr; // Stored in host byte order
	unsigned short 	rtpport; // Stored in host byte order
	int				param;

	int				direction;
	int				index;	//Channel index from radvision stack
	int                             dataTypeHandle;
	int				flags; // follows values from flags fields RTPSet
} H323RTPSet;

typedef struct
{
	int						callModel;
	int						inCRV;
	int						outCRV;
	
	/* number of phones in the list
	 * which follows.
	 */
	int 					nphones;	
	char					phones[LRQ_MAX_PHONES][PHONE_NUM_LEN];


	unsigned short			callsigport;
	unsigned short			rasport;
	unsigned long 			rasip;
	unsigned long			h245ip;
	unsigned long			h245port;
	
	
	unsigned long 			rfrasip;
	unsigned short			rfrasport;

	unsigned short			racallsigport;
 	unsigned long 			rarasip;
	unsigned short			rarasport;
	short				doOlc;

	/* RAS and H.323 Call handles for this transaction */
	HRAS					inhsRas;
	HCALL					hsCall;
	HRAS					outhsRas;
	
	/* Channel Handles for this call */
	int					controlState;
	HCHAN				inhsChannel;
	HCHAN				outhsChannel;

	unsigned char 		userUserStr[USER_USER_STR_SIZE];
	int					userUserStrLen;
	int					waitForDRQ; //tells SCC to not to delete CallHandle before DARQ
	/* Address to whom we sent the ARQ */
	unsigned long		arqip;
	unsigned long		arqcrid;
	ChanInfo			inChan[MAX_LOGICAL_CHAN];
	ChanInfo			outChan[MAX_LOGICAL_CHAN];
	UH323CallAppHandle	*appHandle;	
	char				h323Id[H323ID_LEN];
	// LocalSet is the rtp of this leg i.e out chan
	H323RTPSet      	*localSet; 
	int             	nlocalset;
	H323RTPSet      	*remoteSet;
	int             	nremoteset;
	int					remoteTCSNodeId;
	int					chanEvt;

	/* H245 FCE pinhole bundle Ids */
	unsigned int 		h245remoteBundleId;
	unsigned int 		h245localBundleId;
	unsigned long 		h245localTranslatedIp;
	unsigned short 		h245localTranslatedPort;
	short				infoTransCap;	// information transfer capability
	int					tokenNodeId; /* Node ID of token used for ingress ARQ */
	int					egressTokenNodeId; /* Node ID of token received in ACF/LCF that will be sent in Setup */
	int					reqMode; /* RequestMode in Progress */

	int 				msdStatus;	// Any MSD Error
	int					stackCallState;	// RAdvision call state
//	int					stackCallCtrlState; // RAdvision call state
//	char				msgname[ERROR_MSG_LEN];	// Last message on this leg
	unsigned char		q931IE[Q931IE_LEN];	// Q.931 Information Element
	unsigned char 		bcap[BCAP_LEN]; // bearer capability
	unsigned char		guid[GUID_LEN]; // GUID/callid on the actual message
	unsigned char		origCalledPartyNumType;
	unsigned char		origCallingPartyNumType;
	unsigned char		unused;
	unsigned int		h323flags;
	short				lcnNumber;
#define H323_FLAG_AUTHARQ_SENT		1 // Set when ARQ sent to SGK for inbound SETUPs
#define H323_FLAG_NULLTCS_RECVD		2 // Set on getting NULL TCS from Network.
} H323CallHandle;

#if 0 /* moved to call_handle.x in ls/rpc */
typedef struct FceMediaHoleHandleStruct
{
	unsigned int bundleId;         // bundleId
	unsigned long translatedIp;
	unsigned short translatedPort;
	unsigned long untranslatedIp;
	unsigned short untranslatedPort;

	MFCP_Session *session;
	MFCP_ResourceId resourceId;
} FceMediaHoleHandle;

typedef struct CallHandleStruct
{
	char					callID[CALL_ID_LEN];
	char					confID[CONF_ID_LEN];
	/* PhoNode place-holders */
	PhoNode					phonode;
	PhoNode					rfphonode;
	
	unsigned short			rfcallsigport;

	// needed for NAT traversal
	unsigned long				natip;
	unsigned short				natport;

	/* Call State Machine information */
	// This can be moved into protocol specific part if need be
	SCC_CallState 		state;

	//Leg on which this call is - gk originated or ep originated
	SCC_CallLeg			leg;
	
	// No of Call in the conference. Index into confHandle.
	int					callNo;

	/* Creation and refresh time */
	time_t				iTime;
	time_t				rTime;
	timedef				callStartTime; /* Time when the call starts for billing purposes */
	timedef				callEndTime; /* Time when the call ends for billing purposes */
	timedef				callConnectTime; /* Time when the call starts for billing purposes */

	/* Time when the ringback (alerting/progress) is received for billing purposes */
	timedef				callRingbackTime; 

	SCC_EventProcessor	bridgeEventProcessor; // Pointer to bridge StateMachine for this call 
	SCC_EventProcessor	networkEventProcessor; // Pointer to network StateMachine for this call
	SCC_CallHandleType	handleType; // Sip or H323

// Common things needed for sip to H323 and H323 to H323 
	char				callingPartyNumber[PHONE_NUM_LEN];
	int					callingPartyNumberLen;
	//destination info
	int					peerPort; // tcp port to which we are connected 
	unsigned long		peerIp; // tcp address to which we are connected 

	//This is the original number as input by the user/gw
	char				inputANI[PHONE_NUM_LEN];

	//This is the original number as input by the user/gw
	char				inputNumber[PHONE_NUM_LEN];

	// This is the number after the application of the src calling plan,
	// and will be used for the rollover
	char				dialledNumber[PHONE_NUM_LEN];

	 // This is the number after applying the transit route
        char                            *transRouteNumber;

	// Route being used for the call
	char				crname[CALLPLAN_ATTR_LEN];

	// Routes used for the call
	char				*srccrname; // src route name
	char				*transitcrname; // transit route name
	char				*destcrname; // destination route name

	int				routeflag;

	//Things related to fast start
	short 				fastStart;  // 1 if we are doing fast start
	short 				fastStartStatus;  	// 1 if fast start was success. This is
											// is valid only for Leg1 as of now, but
											// can be easily extended. Does not depend
											// on the doOlc setting on this call, which
											// can vary during the call. Initially set by
											// the application, it can also set to the return
											// value from the stack (not so as of now)
	union {
			SipCallHandle	sipCallHandle;
			H323CallHandle h323CallHandle;
			} handle;
	
	int					rolledOver; /* Set when the called is rolled over */
	tid					rolloverTimer;
	int					callSource; /* Indicates whether this callhandle is the source of the call. */

	// FCE related fields
	FceMediaHoleHandle fceHandle;
	char 				relatedCallID[CALL_ID_LEN];

	// generic flags
	unsigned int		flags;

	List				evtList; /* List of pending events */
	int					maxHunts; /* Max Allowed Number of Attempts */
	int					vendor; /* vendor for call source */

	// Radius information
	char				acct_session_id[9];		/* radius accounting session id */
	char				*conf_id;				/* radius accounting conference id */
	char				*incoming_conf_id;		/* radius accounting incoming conference id */
	char 				*custID;		// custID of peer
	char 				*tg;			// tg of peer
	char 				*destTg;		// destination tg of peer
	char				*dtgInfo;		// leg2 dtg for CDR purposes

	// List of potential alternate destinations
	ListEntry			*destRejectList;
	int					nhunts;		// No of times we have done dynamic
									// hunting for this call
	int					ecaps1;		// extended capabilities of the endpoint

	// Call details for leg 1 - will be consolidated in the future
	int					callError;
	int					cause;
	int					h225Reason;
	int					rasReason;
	int					responseCode;
	int					lastEvent;	// Last recorded event
	header_url_list		*remotecontact_list;
	int					mediaTurning;
	int					dtmf_detect;
	int					dtmf_detect_param;

	// Call details for leg 2
	CallDetails			callDetails2;

	CdrArgs				*prevcdr;	// Last attempts CDR, if not already logged
	int					lastMediaPort;
	int					lastMediaIp;

	// Call Timers
	tid					max_call_duration_tid;
	CallRealmInfo                   *realmInfo;

	char 				*vpnName;
	char				*zone;
	char				*cpname;
	// Session timer info.
	int                             timerSupported;
	int                             sessionExpires;
	int                             minSE;
	int                             refresher;

	List				fcevtList; /* List of pending events in FC calls */

	// Q.931 calling party number type on leg2
	unsigned char		destCallingPartyNumType;

	// Q.931 called party number type on leg2
	unsigned char		destCalledPartyNumType;

} ARQHandle, LRQHandle, SetupHandle, CallHandle;
#endif /* if 0 */

#include "call_handle.h"

#define CallSetCallID(call, _callID_)	(memcpy((call)->callID, \
											_callID_, CALL_ID_LEN))
#define CallSetConfID(call, _confID_)	(memcpy((call)->confID, \
											_confID_, CONF_ID_LEN))

#define CallCallModel(call)				((call)->h323Handle.callModel)
#define	CallSetInCRV(call, _CRV_)		((call)->h323Handle.inCRV = _CRV_)
#define	CallSetOutCRV(call, _CRV_)		((call)->h323Handle.outCRV = _CRV_)
#define CallCallID(call)	((call)->callID)
#define CallConfID(call)	((call)->confID)
#define CallState(call)		((call)->state)

#define Calldestport(call)	(call->rfcallsigport)
#define Calldestip(call)	(call->rfphonode.ipaddress.uc)
#define CallcalledPartyNumber(call)	(call->rfphonode.phone)
#define CallcallingPartyNumber(call)	((call)->callingPartyNumber)
#define CallcallingPartyNumberLen(call)	((call)->callingPartyNumberLen)
#define Callregid(call) ((call)->regid)
#define Calluport(call) ((call)->uport)
#define CallFceBundleId(call) ((call)->fceHandle.bundleId)
// The translated address are valid fields only if the CallFceGetRxCount is 1
#define CallFceNatDstIp(call) ((call)->fceHandle.translatedIp)
#define CallFceNatDstPort(call) ((call)->fceHandle.translatedPort)
#define CallFceTranslatedIp(call) (((call)->fceHandle.refCount & 0x1)*(CallFceNatDstIp(call)))
#define CallFceTranslatedPort(call) (((call)->fceHandle.refCount & 0x1)*(CallFceNatDstPort(call)))
#define CallFceTranslatedIpPort(call) (((call)->fceHandle.refCount & 0x1)*(CallFceNatDstIp(call) || CallFceNatDstPort(call)))
#define CallFceNatSrcIp(call) ((call)->fceHandle.natSrcIp)
#define CallFceNatSrcPort(call) ((call)->fceHandle.natSrcPort)
#define CallFceNatSrcPool(call) ((call)->fceHandle.natSrcPool)
#define CallFceUntranslatedIp(call) ((call)->fceHandle.untranslatedIp)
#define CallFceUntranslatedPort(call) ((call)->fceHandle.untranslatedPort)
#define CallFceSession(call) ((call)->fceHandle.session)
#define CallFceResourceId(call) ((call)->fceHandle.resourceId)
#define CallFceSrcCallID(call) ((call)->fceHandle.srcCallID)
#define CallFceRefCount(call) ((call)->fceHandle.refCount)
#define CallFceGetRxCount(call) ((call)->fceHandle.refCount & 0x1)
#define CallFceSetRxCount(call, v) ((call)->fceHandle.refCount |= (0x1 & v))
#define CallDstSym(call)   ( ( natEnabled() && ( (call)->ecaps1 & ECAPS1_NATDETECT ) ) ? MFCP_DEST_SYM_DISC : MFCP_DEST_SYM_NONE )

#define SipCallHandle(call)	(&((call)->handle.sipCallHandle))
#define SiplocalSet(call)       ((call)->handle.sipCallHandle.localSet)
#define SipremoteSet(call)      ((call)->handle.sipCallHandle.remoteSet)
#define SiplocalSetAddr(call, index) ((call)->handle.sipCallHandle.localSet[index].rtpaddr)
#define SipremoteSetAddr(call, index) ((call)->handle.sipCallHandle.remoteSet[index].rtpaddr)

#define H323callModel(call)	((call)->handle.h323CallHandle.callModel)
#define H323inCRV(call)	((call)->handle.h323CallHandle.inCRV)
#define H323outCRV(call)	((call)->handle.h323CallHandle.outCRV)

#define H323nphones(call)	((call)->handle.h323CallHandle.nphones)
#define H323phones(call)	((call)->handle.h323CallHandle.phones)
#define H323callsigport(call)	((call)->handle.h323CallHandle.callsigport)
#define H323rfcallsigport(call)	((call)->rfcallsigport)
#define H323racallsigport(call)	((call)->handle.h323CallHandle.racallsigport)

#define H323infotranscap(call)	((call)->handle.h323CallHandle.infoTransCap)
#define H323flags(call)			((call)->handle.h323CallHandle.h323flags)

#define H323rasport(call)	((call)->handle.h323CallHandle.rasport)
#define H323rfrasport(call)	((call)->handle.h323CallHandle.rfrasport)
#define H323rarasport(call)	((call)->handle.h323CallHandle.rarasport)
#define H323rasip(call)	((call)->handle.h323CallHandle.rasip)
#define H323rfrasip(call)	((call)->handle.h323CallHandle.rfrasip)
#define H323rarasip(call)	((call)->handle.h323CallHandle.rarasip)
#define H323h245ip(call)	((call)->handle.h323CallHandle.h245ip)
#define H323h245port(call)	((call)->handle.h323CallHandle.h245port)
	
#define H323hsCall(call)	((call)->handle.h323CallHandle.hsCall)
#define H323inhsRas(call)	((call)->handle.h323CallHandle.inhsRas)
#define H323outhsRas(call)	((call)->handle.h323CallHandle.outhsRas)
#define H323inhsChannel(call)	((call)->handle.h323CallHandle.inhsChannel)
#define H323outhsChannel(call)	((call)->handle.h323CallHandle.outhsChannel)
#define H323inChan(call)	((call)->handle.h323CallHandle.inChan)
#define H323outChan(call)	((call)->handle.h323CallHandle.outChan)

#define H323controlState(call)	((call)->handle.h323CallHandle.controlState)
#define H323useruserStr(call)	((call)->handle.h323CallHandle.useruserStr)
#define H323useruserStrLen(call)	((call)->handle.h323CallHandle.useruserStrLen)
#define H323callingPartyNumber(call)	((call)->callingPartyNumber)
#define H323callingPartyNumberLen(call)	((call)->callingPartyNumberLen)
#define H323waitForDRQ(call)	((call)->handle.h323CallHandle.waitForDRQ)
#define H323arqip(call)	((call)->handle.h323CallHandle.arqip)
#define H323arqcrid(call)	((call)->handle.h323CallHandle.arqcrid)
#define H323appHandle(call)	((call)->handle.h323CallHandle.appHandle)
#define H323h323Id(call)	((call)->handle.h323CallHandle.h323Id)
#define H323dialledNumber(call)	((call)->dialledNumber)
#define CallInputNumber(call)	((call)->inputNumber)
#define CallInputANI(call)	((call)->inputANI)
#define H323localSet(call) ((call)->handle.h323CallHandle.localSet)
#define H323nlocalset(call) ((call)->handle.h323CallHandle.nlocalset)
#define H323remoteSet(call) ((call)->handle.h323CallHandle.remoteSet)
#define H323nremoteset(call) ((call)->handle.h323CallHandle.nremoteset)
#define H323doOlc(call) ((call)->handle.h323CallHandle.doOlc)
#define H323remoteTCSNodeId(call) ((call)->handle.h323CallHandle.remoteTCSNodeId)
#define H323chanEvt(call) ((call)->handle.h323CallHandle.chanEvt)
#define H323localSetAddr(call, index) ((call)->handle.h323CallHandle.localSet[index].rtpaddr)
#define H323remoteSetAddr(call, index) ((call)->handle.h323CallHandle.remoteSet[index].rtpaddr)

#define H323remoteBundleId(call) ((call)->handle.h323CallHandle.h245remoteBundleId)
#define H323localBundleId(call) ((call)->handle.h323CallHandle.h245localBundleId)
#define H323localTranslatedIp(call) ((call)->handle.h323CallHandle.h245localTranslatedIp)
#define H323localTranslatedPort(call) ((call)->handle.h323CallHandle.h245localTranslatedPort)
#define H323tokenNodeId(call) ((call)->handle.h323CallHandle.tokenNodeId)
#define H323egressTokenNodeId(call) ((call)->handle.h323CallHandle.egressTokenNodeId)
#define H323reqMode(call) ((call)->handle.h323CallHandle.reqMode)

#define H323MsdStatus(call) ((call)->handle.h323CallHandle.msdStatus)
#define H323StackCallState(call) ((call)->handle.h323CallHandle.stackCallState)
#define H323StackCallCtrlState(call) ((call)->handle.h323CallHandle.stackCallCtrlState)
#define H323CallMsgName(call) ((call)->handle.h323CallHandle.msgname)
#define H323Callq931IE(call) ((call)->handle.h323CallHandle.q931IE)
#define H323Callbcap(call) ((call)->handle.h323CallHandle.bcap)

#define H323OrigCallingPartyNumType(call) ((call)->handle.h323CallHandle.origCallingPartyNumType)
#define H323OrigCalledPartyNumType(call) ((call)->handle.h323CallHandle.origCalledPartyNumType)
#define H323DestCallingPartyNumType(call) ((call)->destCallingPartyNumType)
#define H323DestCalledPartyNumType(call) ((call)->destCalledPartyNumType)

extern int ncalls;


UH323CallAppHandle * uh323CallAllocAppHandle(void);
int uh323CallInitAppHandle(HCALL, UH323CallAppHandle *);
void uh323CallFreeAppHandle(UH323CallAppHandle *appHandle);
int sccUh323InitEventBlock(SCC_EventBlock *,UH323CallAppHandle *);
int GkHandleACF(IN HAPPRAS haRas, IN HRAS hsRas);
int GkHandleIRR(IN HAPPRAS haRas, IN HRAS hsRas);
int GkHandleARJ(IN HAPPRAS haRas, IN HRAS hsRas, IN cmRASReason reason);
int GkHandleARQTimeout(IN HAPPRAS haRas, IN HRAS hsRas);
int GkHandleARQ(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int GkHandleDRQ(IN HRAS hsRas, IN HCALL hsCall, OUT LPHAPPRAS lphaRas,
		                IN cmTransportAddress *srcAddress, IN HAPPCALL haCall);
int pvtGetNodeFromSeq( HPVT hVal, int nodeId, char *startPath, char *itemPath, 
		int *index, int *len);
int GkHandleARQFailure(IN HAPPRAS haRas, IN HRAS hsRas, IN cmRASReason reason);


#define ARQGetRASHandle(handle)				((handle)->outhsRas)
#define ARQSetRASHandle(handle, _hsRas_)	((handle)->outhsRas = (_hsRas_))

#endif /* _arq_h_ */ 
