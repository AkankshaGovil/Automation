/* moved from ssip/inclide/sipcall.h */
#ifdef RPC_HDR
%#include "timer.h"
%#include "codecs.h"
%#include "sipkey.h"
%#include "calldefs.h"
%#include "mem.h"
%#include "list.h"
%#include "header_url.h"
%
%
%typedef struct _SipEventHandle SipEventHandle;
%typedef SipEventHandle *pSipEventHandle;
%
%extern bool_t xdr_header_url(XDR *, header_url *);
%extern bool_t xdr_CallRealmInfo(XDR *, CallRealmInfo *);
%extern bool_t xdr_pSipEventHandle(XDR *, pSipEventHandle *);
%extern bool_t xdr_tid(XDR *, tid *);
#endif

typedef string rpc_string <>; 

#ifdef RPC_HDR
%
%struct _nvpair {
%	char *name;
%	char *val;
%}	_nvpair;
%
%typedef struct _nvpair nvpair;
%
%struct _SipMsgHandle
%{
%	char			*callid;	/* sip call id */
%	header_url 		*local;
%	header_url 		*remote;
%
%	header_url		*requri;
%	header_url		*localcontact;
%	header_url		*remotecontact;
%	pheader_url_list	remotecontact_list;
%	header_url		*referto;
%	header_url		*referby;
%       char                    *replaces;
%
%	/* the host and port where the request came from, extracted from top via */
%	char			*srchost;
%	unsigned short		srcport;
%
%	int				cseqno;
%	char 			*method;	/* csequence method */
%
%	int				msgType;
%
%	int				responseCode;
%	char			*responseMsgStr;	/* May be NULL */
%
%	/* Dynamic Array to store record routes */
%	char			**routes;
%	int				nroutes;
%
%	int				origin;		/* 1 = local, 0 = remote */
%
%	char			*xConnId;
%	char			*also;
%       char                    *alert_info;
%
%};
%
%typedef struct _SipMsgHandle SipMsgHandle;
%
%
%#define SipMsgFrom(_handle_)		(((_handle_)->msgType == SipMessageRequest) ? (((_handle_)->origin == 0) ? (_handle_)->remote: (_handle_)->local):(((_handle_)->origin == 0) ? (_handle_)->local:(_handle_)->remote) )
%
%#define SetSipMsgFrom(_handle_, from_url)	if((_handle_)->msgType == SipMessageRequest) { if((_handle_)->origin == 0) (_handle_)->remote = from_url; else (_handle_)->local = from_url; } else { if((_handle_)->origin == 0) (_handle_)->local = from_url; else (_handle_)->remote = from_url; }
%
%#define SipMsgTo(_handle_)			(((_handle_)->msgType == SipMessageRequest) ? (((_handle_)->origin == 0) ? (_handle_)->local: (_handle_)->remote): (((_handle_)->origin == 0) ? (_handle_)->remote: (_handle_)->local) )
%									
%#define SetSipMsgTo(_handle_, to_url)	if((_handle_)->msgType == SipMessageRequest) { if((_handle_)->origin == 0) (_handle_)->local = to_url; else (_handle_)->remote = to_url; } else { if((_handle_)->origin == 0) (_handle_)->remote = to_url; else (_handle_)->local = to_url; } 

%#define SipMsgCallID(_handle_)		((_handle_)->callid)
%#define SipMsgCallCSeqNo(_handle_)	((_handle_)->cseqno)
%#define SipMsgCallCseqMethod(_handle_)	((_handle_)->method)
%
%#define DTMF_DEFAULT_DURATION	200	/* ms */
%
%enum _SipTranError
%{
%	tsmError_NO_RESPONSE = 1000,	/* no response received for sent request */
%	tsmError_NO_ACK,		/* no ack received for sent final response */
%	tsmError_UNDEFINED,
%
%	csmError_KILL_TRAN = 2000	/* tell TSM to kill a transaction */
%};
%
%typedef enum _SipTranError SipTranError;
%
%struct _DTMFParams
%{
%	int    sig; /* dtmf signal  */
%	int    duration;
%};
%
%typedef struct _DTMFParams DTMFParams;
%
/* Privacy related enums */
%typedef enum {
%
%  privacyTypeNone,     
%  privacyTypeRFC3325,
%  privacyTypeDraft01,
%
%} SipPrivacyTypes;
%
%
%typedef enum {
%
%  privTranslateNone ,
%  privTranslateRFC3325ToDraft01, 
%  privTranslateDraft01ToRFC3325
%} SipPrivacyTranslateTypes;
%
%typedef enum {
%
%  privacyLevelNone,
%  privacyLevelId,
%  privacyLevelHeader,
%  privacyLevelSession,
%  privacyLevelUser,
%  privacyLevelCritical
%} SipPrivacyLevel;
%
%typedef enum {
%        cid_noaction,
%        cid_block,
%        cid_unblock
%} SipCidPresentation;
%struct _SipAppMsgHandle
%{
%	char				confID[CONF_ID_LEN];	/* app conf id */
%	char				callID[CALL_ID_LEN];	/* app call id */
%	
%	/* Called party number or URI */
%	header_url			*calledpn;
%
%	/* Calling party number, or URI */
%	header_url			*callingpn;
%
%	/* Determined destination of the call */
%	header_url			*requri;
%	
%	/* If local contact is specified, use this, o/w the
%	** callingpn is used */
%	header_url			*localContact;		/* Local contact */
%
%	/* ip destination of packet - may be different from request uri
%	 * Must be used ONLY when both ipaddr and port are non-zero
%	 */
%	unsigned long		dipaddr;
%	unsigned short		dipport;
%	
%	/* Will be set to non-null, if there is a message
%	 * associated with this event
%	 */
%	SipMsgHandle 		*msgHandle;
%
%	/* SDP is considered changed when the sdpChanged
%	 * flag is set. The flag gets set when the version gets
%	 * changed or the contents of the SDP get changed
%	 */	
%#define MEDIA_VERSION	0x00000001
%#define MEDIA_CODEC		0x00000002
%#define MEDIA_TRANSPORT	0x00000004
%#define MEDIA_ALL		0xffffffff
%
%	unsigned long		sdpVersion;
%	unsigned long		mediaChanged;
%
%	/* DTMF params */
%	DTMFParams                      *dtmf;
%	int                             ndtmf;
%
%	/* RTP Params */
%	RTPSet				*localSet;
%	int					nlocalset;
%	int					attr_count;
%	SDPAttr				*attr;
%	char 				*pOriginIpAddress;
%
%	/* ISUP message */
%	unsigned char       *isup_msg;
%	int                 isup_msg_len;
%	char 				*isupTypeVersion;
%	char				*isupTypeBase;
%	char				*isupDisposition;
%	char				*isupHandling;
%
%	/* QSIG message */
%	unsigned char       *qsig_msg;
%	int                 qsig_msg_len;
%	char 				*qsigTypeVersion;
%	char				*qsigDisposition;
%	char				*qsigHandling;
%
%	/* authorization and proxyauth hdr */
%	char			*hdrAuthorization;
%	char			*hdrProxyauthorization;
%	/* authenticate and wwwauthenticate hdr */
%	char			*hdrProxyauthenticate;
%	char			*hdrWwwauthenticate;
%
%	int					responseCode;
%	char				*responseMsgStr;	/* May be NULL */
%
%	SipTranError		tsmError;	/* set by TSM to inform CSM of error */
%	SipTranError		csmError;	/* set by CSM to inform TSM of error */
%
%#define SIPAPP_USELOCALSDP  0x1		/* if no sdp is there in the app msg handle */
%									/* ua will generate it from whatever it has */
%#define SIPAPP_LOCALINVITE	0x2		/* this invite is locally generated by ua */
%
%	unsigned int		flags;
%
%	int					maxForwards;
%	CallRealmInfo		*realmInfo;
%
%	/* Session timer paramters..*/
%	int                     timerSupported;
%	int                     minSE;
%	int                     sessionExpires;
%
%#define SESSION_REFRESHER_NONE  0
%#define SESSION_REFRESHER_UAC   1
%#define SESSION_REFRESHER_UAS   2
%
%	int                     refresher;
%
%	/* value of any expires header coming in or sent out */
%	int					expires;
%
%	// store the NAT ip and port
%	unsigned long		natip;
%	unsigned short		natport;
%
%	 /*sip privacy related */
%	 header_url            		*pAssertedID_Sip;
%	 char            		*pAssertedID_Tel; 
%	 header_url              	*original_from_hdr;
%	 header_url              	*rpid_hdr; /* We need to correctly populate appmsghandle->callingpn */
%					 
%	 char                    	*rpid_url;
%	 char                    	*proxy_req_hdr;
%	 char                  		*priv_value;  
%						 
%	 SipPrivacyTypes       		 incomingPrivType;
%	 SipPrivacyTranslateTypes        privTranslate;
%	 SipPrivacyLevel                 privLevel;
%	 SipCidPresentation              generate_cid;
%        SipPrivacyTypes                 dest_priv_type;
%	/*REFER - Call Transfer */       
%        char*                           sip_frag;
%	 int                             sip_frag_len;
%        char*                           allow;
%        char*                           supported;
%        char*                           event;
%        char*                           sub_state;
%        char*                           content_type;
%
%	/* Dynamic Array to store unknown headers */
%	nvpair			*unkhdrs;
%	int				nunkhdr;
%};
%
%typedef struct _SipAppMsgHandle SipAppCallHandle;
%typedef struct _SipAppMsgHandle SipAppMsgHandle;
%
%/* Transaction s/m sends msg to Call SM */
%int
%SipTransRecvMsgHandle(SipAppMsgHandle *);
%
%/* Call SM sends message to TSM */
%int
%SipTransSendMsgHandle(SipAppMsgHandle *);
%
%enum _SipEventType
%{
%	Sip_eNetworkEvent = 0,
%	Sip_eBridgeEvent
%};
%
%typedef enum _SipEventType SipEventType;
%
%struct _SipEventHandle
%{
%	int 				type;
%	int					event;
%	
%	SipAppCallHandle 	*handle;
%
%	CallDetails			callDetails;
%
%};
%
%
%
%
%/* This structure is contained within the application call
% * handle. All memory inside this handle must be allocated
% * using the cache allocation functions.
% */
%#define UAF_PENDBINVITE	0x1	// When an INVITE from Bridge is pending in evb
%#define UAF_TIMEDINVITE	0x2	// A Timer is running possibly due to 491
%#define UAF_LOCALINVITE	0x4	// When an INVITE generated locally by UA is ongoing
%#define UAF_NETIDINVITE	0x8	// Invite from network w/o change
%
#endif /* RPC_HDR */

struct _SipCallHandle
{
	SipCallLegKey		callLeg;

	int					inviteOrigin;	/* Did we send out an invite
										* for this call at any point?
										*/
	int					successfulInvites;	/* How many successful ? */

	/* Sum of the pending stuff should be at most 1 */
	pSipEventHandle		pendBInvite;	/* Stuff from bridge which has
										** been queued because there were
										** pending invites */

	int					localCSeqNo;
	int					remoteCSeqNo;

	header_url			*localContact;		/* Local contact */
	header_url			*remoteContact;		/* contact of remote */
	header_url			*requri;
	header_url			*inrequri;

	/* Codec/SDP stuff */
	unsigned long		rsdpVersion;
	unsigned long		lsdpVersion;

	/* Remote is always opposite of local */
	RTPSet				remoteSet<>;
	SDPAttr				remoteAttr<>;
	/* Local is the side for which this call is
	 * proxying for. Peer is the keyword
	 */
	RTPSet				localSet<>;
	SDPAttr				localAttr<>;

	char 				*pOriginIpAddress;
	
	/* Did we add any codecs which were not already 
	 * in the local set.
	 */
	RTPSet				localAddSet<>;

	/* Dynamic Array to store record routes */
	rpc_string			routes<>;
	

#ifdef RPC_HDR
%#define UAF_NETIDINVITE 0x8 /* Invite from network w/ same SDP */
#endif

	unsigned int        uaflags;

	tid					timerC;			/* timer C */
	tid					sessionTimer;	/* session timer on call */

	/* the host and port where the request came from, from tsm */
	string			srchost<>;

	int				maxForwards;
        /*sip privacy related */
	header_url            		*pAssertedID_Sip;
	char            		*pAssertedID_Tel;
	header_url              	*original_from_hdr;
	header_url              	*rpid_hdr; /* We need to correctly populate appmsghandle->callingpn */
	       			 
	char                    	*rpid_url;
	char                    	*proxy_req_hdr;
	char                  		*priv_value;  
        
        int       		        incomingPrivType;
	int                             privTranslate;
	int                             privLevel;

};

typedef struct _SipCallHandle SipCallHandle;

#ifdef RPC_HDR


%#define SIPEVENT_CALLSM		0
%#define SIPEVENT_REGSM		1
%
%#define SipEventAppHandle(_handle_)		((_handle_)->handle)
%#define SipEventMsgHandle(_handle_)		((_handle_)->handle->msgHandle)
%#define SipEventHandleType(_handle_)	((_handle_)->type)
%#define SipEventHandleEvent(_handle_)	((_handle_)->event)
%
%/* Call states for the Sip SM */
%typedef enum
%{
%	Sip_sIdle = 0,
%	Sip_sWORRing,		/* WOR=WaitOnRemote */
%	Sip_sRingWOR,
%	Sip_sConnecting,	/* Waiting for Ack */
%	Sip_sConnectedOK,	/* Got the OK, but havent send the Ack */
%	Sip_sConnectedAck,	/* Completely connected */
%	Sip_sConnectedWOR,
%
%	Sip_sMaxStates,
%} Sip_State;
%
%/* Events for the Sip SM */
%typedef enum
%{
%	// Call State Machine events are listed first 
%
%	Sip_eNetworkInvite = 0,
%	Sip_eNetwork1xx,			/* Non 100 */
%	Sip_eNetwork200,		
%	Sip_eNetworkAck,
%	Sip_eNetwork3xx,
%	Sip_eNetworkFinalResponse,	/* 4xx, 5xx, 6xx */
%	Sip_eNetworkBye,
%	Sip_eNetworkCancel,
%	Sip_eNetworkInfo,
%	Sip_eNetworkInfoFinalResponse,	/* >= 200 */
%	Sip_eNetworkError,			/* Non-protocol errors */
%	Sip_eNetworkNoResponseError,		/* Request timed out */
%       Sip_eNetworkRefer,
%	Sip_eNetwork202,
%	Sip_eNetworkNotify,
%	Sip_eNetworkNotifyResponse,
%	Sip_eMaxNetworkEvents,
%
%} Sip_NetworkEvents;
%
%/* 3xx event will not be in Bridge */
%typedef enum
%{
%	Sip_eBridgeInvite = 0,
%	Sip_eBridge1xx,				/* Non 100 */
%	Sip_eBridge200,
%	Sip_eBridgeAck,
%	Sip_eBridge3xx,				/* NOT USED */
%	Sip_eBridgeFinalResponse,	/* Final Response ONLY for Invite */
%	Sip_eBridgeBye,
%	Sip_eBridgeCancel,
%	Sip_eBridgeInfo,
%	Sip_eBridgeInfoFinalResponse,	/* eBridge200 implies INVITE, use separate one */
%	Sip_eBridgeError,			/* Non-protocol errors */
%	Sip_eBridge491Expired,		/* Timer running due to 491 has expired */
%	Sip_eBridgeCExpired,		/* Timer C has expired */
%	Sip_eBridgeSessionExpired,		/* Session Timer has expired */
%	Sip_eBridgeNoResponseError,	/* Request timed out */
%       Sip_eBridgeRefer,
%       Sip_eBridge202,
%	Sip_eBridgeNotify,
%       Sip_eBridgeNotifyResponse,
%	Sip_eMaxBridgeEvents,
%} Sip_BridgeEvents;
%
%typedef int (*SipActionRoutine)(SipEventHandle *);
%typedef struct
%{
%	int					nextState;
%	SipActionRoutine	action;
%} SIP_StateMachineEntry;
%
%typedef SIP_StateMachineEntry 	SipNetworkSMEntry;
%typedef SIP_StateMachineEntry 	SipBridgeSMEntry;
%
%/* Sip Network Side State m/c - Events coming in from Network to SIP */
%extern SipNetworkSMEntry SipNetworkSM[Sip_sMaxStates][Sip_eMaxNetworkEvents];
%
%/* Bridge Side - Events coming in from Bridge to SIP */
%extern SipBridgeSMEntry SipBridgeSM[Sip_sMaxStates][Sip_eMaxBridgeEvents];
%
#endif /* RPC_HDR */
