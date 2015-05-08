/* file moved from gk/include/arq.h */
#ifdef RPC_HDR
%#include "mem.h"
%#include "list.h"
%#include "uh323.h"
%#include "timedef.h"
%#include "callsm.h"
%#include "confdefs.h"
%#include "usipinc.h"
%#include "serverp.h"
%#include "ssip.h"
%#include "codecs.h"
%#include "arq.h"
%
%typedef union _handle Handle;
%
%extern bool_t xdr_SCC_CallState (XDR *, SCC_CallState *); 
%extern bool_t xdr_SCC_CallLeg (XDR *, SCC_CallLeg *); 
%extern bool_t xdr_SCC_CallHandleType (XDR *, SCC_CallHandleType *); 
%extern bool_t xdr_Handle(XDR *, Handle *);
%
%union _handle {
%	SipCallHandle	sipCallHandle;
%	H323CallHandle h323CallHandle;
%};
%
%
%
%typedef ListEntry *pListEntry;
#endif /* RPC_HDR */


#ifdef RPC_XDR

%#include "norepl.h"

typedef long time_t;
typedef unsigned int MFCP_ResourceId;
typedef long suseconds_t;
typedef u_longlong_t tid;
struct timedef  {
	time_t			tv_sec;
	suseconds_t		tv_usec;
};
#endif /* RPC_XDR */

struct _FceMediaHoleHandleStruct
{
	unsigned int	bundleId;		/* bundleId */
	unsigned long	translatedIp;	/* nat_dst_ip */
	unsigned short	translatedPort;	/* nat_dst_port */
	unsigned char	refCount;		/* rx/tx count */
	unsigned long	natSrcIp;		/* nat_src_ip */
	unsigned short	natSrcPort;		/* nat_src_port */
	unsigned int	natSrcPool;		/* nat_src_pool */
	char srcCallID[CALL_ID_LEN];	/* This CallHandle contains the nat_src in its translatedIp field */

	unsigned long untranslatedIp;  		/* dst_ip */ 
	unsigned short untranslatedPort; 	/* dst_port */


	MFCP_ResourceId resourceId;
	pMFCP_Session session;
};

typedef struct _FceMediaHoleHandleStruct FceMediaHoleHandle;

struct CallHandleStruct
{
	char					callID[CALL_ID_LEN];
	char					confID[CONF_ID_LEN];
	/* PhoNode place-holders */
	PhoNode					phonode;
	PhoNode					rfphonode;
	
	unsigned short			rfcallsigport;

        /* needed for NAT traversal */
        unsigned long                           natip;
        unsigned short                          natport;

	/* Call State Machine information */
	/* This can be moved into protocol specific part if need be */
	SCC_CallState 		state;

	/* Leg on which this call is - gk originated or ep originated */
	SCC_CallLeg			leg;
	
	/* No of Call in the conference. Index into confHandle. */
	int					callNo;

	/* Creation and refresh time */
	time_t				iTime;
	time_t				rTime;
	timedef				callStartTime; /* Time when the call starts for billing purposes */
	timedef				callEndTime; /* Time when the call ends for billing purposes */
	timedef				callConnectTime; /* Time when the call starts for billing purposes */

	/* Time when the ringback (alerting/progress) is received for billing purposes */
	timedef				callRingbackTime; 

	SCC_EventProcessor	bridgeEventProcessor; /* Pointer to bridge StateMachine for this call */
	SCC_EventProcessor	networkEventProcessor; /* Pointer to network StateMachine for this call */
	SCC_CallHandleType	handleType; /* Sip or H323 */

/* Common things needed for sip to H323 and H323 to H323 */
	char				callingPartyNumber[PHONE_NUM_LEN];
	int					callingPartyNumberLen;
	/* destination info */
	int					peerPort; /* tcp port to which we are connected */
	unsigned long		peerIp; /* tcp address to which we are connected */

	/* This is the original number as input by the user/gw */
	char				inputANI[PHONE_NUM_LEN];

	/* This is the original number as input by the user/gw */
	char				inputNumber[PHONE_NUM_LEN];

	/*
	** This is the number after the application of the src calling plan,
	** and will be used for the rollover
	*/
	char				dialledNumber[PHONE_NUM_LEN];

	 /* This is the number after applying the transit route */
	string				transRouteNumber<>;

	/* Route being used for the call */
	char				crname[CALLPLAN_ATTR_LEN];

	/* Routes used for the call */
	string				srccrname<>; /* src route name */
	string				transitcrname<>; /* transit route name */
	string				destcrname<>; /* destination route name */

	int				routeflag;

	/* Things related to fast start */
	short 				fastStart;  /* 1 if we are doing fast start */
	short 				fastStartStatus;  	/* 1 if fast start was success. This is
											** is valid only for Leg1 as of now, but
											** can be easily extended. Does not depend
											** on the doOlc setting on this call, which
											** can vary during the call. Initially set by
											** the application, it can also set to the return
											** value from the stack (not so as of now) */
	Handle				handle;
	
	int					rolledOver; /* Set when the called is rolled over */
	tid					rolloverTimer;
	int					callSource; /* Indicates whether this callhandle is the source of the call. */

	/* FCE related fields */
	FceMediaHoleHandle fceHandle;

	/* generic flags */
	unsigned int		flags;

	List				evtList; /* List of pending events */
	int					maxHunts; /* Max Allowed Number of Attempts */
	int					vendor; /* vendor for call source */

	/* Radius information */
	char				acct_session_id[9];		/* radius accounting session id */
	string				conf_id<>;				/* radius accounting conference id */
	string				incoming_conf_id<>;		/* radius accounting incoming conference id */
	string 				custID<>;		/* custID of peer */
	string 				tg<>;			/* tg of peer */
	string 				destTg<>;		/* destination tg of peer */
	string 				dtgInfo<>;		/* destination tg of peer */
	string                          ogprefix<>;             /* outgoing prefix */

	/* List of potential alternate destinations */
	pListEntry			destRejectList;
	int					nhunts;		/* No of times we have done dynamic */
									/* hunting for this call */
    unsigned short 		cap;		/* Capabilities of the endpoint */
	int					ecaps1;		/* extended capabilities of the endpoint */

	/* Call details for leg 1 - will be consolidated in the future */
	int					callError;
	int					cause;
	int					h225Reason;
	int					rasReason;
	int					responseCode;
	int					lastEvent;	/* Last recorded event */
	pheader_url_list	remotecontact_list;
	int					mediaTurning;
	int					dtmf_detect;
	int					dtmf_detect_param;

	/* Call details for leg 2 */
	CallDetails			callDetails2;

	pCdrArgs			prevcdr;	/* Last attempts CDR, if not already logged */
	int					lastMediaPort;
	int					lastMediaIp;

	/* Call Timers */
	tid					max_call_duration_tid;
	CallRealmInfo                   *realmInfo;

	string 				vpnName<>;
	string				zone<>;
	string				cpname<>;
	/* Session timer info. */
	int                             sessionTimerActive;
	int                             timerSupported;
	int                             sessionExpires;
	int                             minSE;
	int                             refresher;

	List				fcevtList; /* List of pending events in FC calls */

	unsigned char		destCalledPartyNumType;
	unsigned char		destCallingPartyNumType;
    char                gkXlatedCgn[PHONE_NUM_LEN];
};

typedef struct CallHandleStruct CallHandle;

#ifdef RPC_HDR
%typedef struct CallHandleStruct ARQHandle;
%typedef struct CallHandleStruct LRQHandle;
%typedef struct CallHandleStruct SetupHandle;
#endif
