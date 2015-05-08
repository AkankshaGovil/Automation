#ifndef _ipc_h_
#define _ipc_h_

#include "pho_node.h"

/*
 * Other includes
 */
#include <time.h>
#include <stdio.h>

/*
 * Inter Protoid Communications [IPC].
 */

/*
 * Port numbers of interest to the NexTone System
 */

/* Port numbers for the clients */
#define RX_PORT         		9444 	/* RTP/UDP start port for voice */
#define RX_MAXPORT				9831	/* Wrap around after this */
#define RX_FAXPORT         		9632 	/* RTP/UDP start port for fax */
#define RX_MAXFAXPORT			9999	/* Wrap around after this */
#define DATA_PORT				RX_PORT
#define WAKE_PORT       		9393
#define CONTROL_PORT			WAKE_PORT

/*
 * Port numbers for the Aloid Server complex.
 * This includes the VPNS complex, right now.
 */
/* Port number Aloid (LS) is listening on */
#define ALOID_LOOKUP_PORT_NUMBER		10000 /* TCP */
#define VPNS_LOOKUP_PORT_NUMBER	        10001 /* TCP */
#define ALOID_AGING_PORT_NUMBER         10000 /* UDP */
#define VPNS_AGING_PORT_NUMBER          10001 /* UDP - not used > 1.1*/
#define BCS_PORT_NUMBER					10003 /* UDP */
#define LUS_REDUNDS_PORT_NUMBER			10004 /* TCP */
#define VPNS_REDUNDS_PORT_NUMBER		10005 /* TCP */
#define GIS_REDUNDS_PORT_NUMBER			LUS_REDUNDS_PORT_NUMBER

#define IPC_MGR_PORT_NUMBER             ALOID_LOOKUP_PORT_NUMBER
 
/* Length of phone number record
 * Actual lengths exchanged in messages include a 
 * final null termination character, adding a +1
 */ 
#define PHONE_NUM_LEN           64
#define VPN_LEN 				PHONE_NUM_LEN
#define VPN_GROUP_LEN			80
#define ZONE_LEN				30
#define EMAIL_LEN				80
#define ANIGEN_LEN				48

#define ENDPOINTID_LEN			128
#define GKID_LEN 				256
#define SIPDOMAIN_LEN			256
#define SIPURL_LEN				256
#define MSWNAME_LEN					256
#define	SIPAUTHPASSWORD_LEN		32

#define	RADSERVERADDR_LEN		256
#define SECRET_LEN				256

#define ENUMDOMAIN_LEN			256

#define CALLID_LEN				256
#define H323ID_LEN				128

#define MAX_IEDGE_PORTS			256

/* VPN and Client attributes length */
#define CLIENT_ATTR_LEN 	28
#define VPNS_ATTR_LEN   	28
#define REG_ID_LEN          68
#define	PASSWORD_LEN		16
#define TAG_NAME_LEN		32
#define CALLPLAN_ATTR_LEN	60
#define TRIGGER_ATTR_LEN	28
#define REALM_NAME_LEN      32
#define IGRP_NAME_LEN		32
#define CID_BLK_UNBLK_LEN       16
#define VNET_NAME_LEN           32
/* used for sflags, to indicate what all is set etc */
#define ISSET_REGID				1
#define ISSET_IPADDRESS			2
#define ISSET_UPORT				3
#define ISSET_PHONE				4
#define ISSET_VPNPHONE			5
#define ISSET_DURATION  		6
/*
 * If password is "crypt()" ed, then the first two characters of
 * registration id is the "salt". Also, if the password field is
 * encrypted, then the first two characters of password represent
 * the salt itself.
 */
#define ISSET_PASSWORD				7
#define ISSET_PASSWORD_ENCRYPTED	8

#define ISSET_EMAIL					9
#define ISSET_NATIP					10
#define ISSET_NATPORT					11


typedef struct
{
	unsigned long 		ip;
	unsigned short 		port;	
} IPTransportAddr;

/*
 * Possible states of a Netoid.
 * Note that these must comform to the BIT settings just below.
 * These affect the stateFlags.
 * Warning - do not exceed a short size - see PhoNode, and 
 * InfoEntry structures.
 */
#define CL_ACTIVE 		0x1	/* Active, breathing endpoint */
#define	CL_IDLE	    		0x2
#define	CL_STATIC		0x4	/* Statically defined endpoints */
#define	CL_ALIVE		0x8	/* Alive, but not active */
#define	CL_BUSY     		0x10	/* Other busy -- dialing etc. */
#define	CL_INCONF   		0x20	/* In conference */
#define	CL_DND      		0x40	/* Do not disturb */
#define	CL_FORWARD  		0x80	/* Forward all incoming calls */
#define	CL_REGISTERED 		0x100	/* Registered once at least */
#define	CL_INCALL   		0x200	/* In call */
#define	CL_FORWARDDEST 		0x400	/* Represents a forwarded number */
#define	CL_PROXY    		0x800	/* Someone will proxy me */
#define	CL_PROXYING		0x1000	/* I am proxying someone */
				/* If both above bits are set,
				* it indicates that I am being
				* proxied by someone
				*/
#define	CL_HOLDORIG		0x2000	
#define	CL_FORWARDSTATIC 	0x4000	/* Statically configured forwarding */
#define CL_TAP			0x80000

#define CL_DYNAMIC		0x100000	/* A dynamicly created entry */
#define CL_SIPDOMAIN	0x200000	/* A dynamicly created entry */
#define CL_STICKY		0x400000	/* Last stop endpoint */
#define CL_UAREG		0x800000	/* Needs to be registered to a registrar */
#define CL_UAREGSM		0x1000000	/* Registrar s/m enabled */

typedef unsigned long ClientState;

/* When a client is eligible for a cache entry */
#define CLIENT_GRADUATE(p) ((p)->stateFlags & (CL_PROXY | CL_ACTIVE))
#define CLIENT_ACTIVE(p) ((p)->stateFlags & CL_ACTIVE)

/* Packet Types */
#define PKT_REGISTER 			1
#define PKT_UNREGISTER			2
#define	PKT_PROXYREGISTER		3	/* I am the proxy */
#define PKT_REGISTER_SUCCESS		4
#define PKT_REGISTER_FAILURE		5
#define PKT_FIND			6
#define PKT_FOUND			7
#define PKT_NOTFOUND			8

#define	PKT_WAKEUP        		11
#define	PKT_HANGUP			12

#define	PKT_CONNECT			16
#define	PKT_DND				17	/* Do not disturb */
#define	PKT_REDIRECT			18
#define	PKT_CONTROL			19
#define	PKT_TRANSFER			20
#define PKT_TRANSFERRED			21	/* I am transferred to you */
#define	PKT_PROFILE			22
#define	PKT_SERVERMSG			23	/* Generic Message */
#define	PKT_NIKE               		24	/* Key Exchange protocol 
						 * (IKE) 
						 */
#define	PKT_PEF                		25	/* Payload Encapsulation 
						 * Format (ISAKMP) 
						 */ 
#define	PKT_PROXY			26	/* Proxy me */
#define	PKT_ERROR              		27	/* For unsupported packet 
						 * types 
						 */
#define	PKT_ALERTED			28
#define	PKT_BUSY			29
#define PKT_HEARTBEAT			30	/* Heartbeat */

#define PKT_NEXTIME			31	/* Time Protocol */
#define PKT_CACHEENTRY			32	/* Binaty Cache Update */
#define PKT_XML				33	/* XML Encoded packet */

#define PKT_MAX_TYPE			34	/* Don't exceed this */

/* Temporary way to detect a PC. The way to do this
 * is to use TLV's, but that will be in the next major rev
 * of the protocol
 */
#define CAP_UCC				0	/* Registration based */
#define CAP_IMOBILE			1	/* Same as H.323T */
#define CAP_IGATEWAY		2
#define CAP_I1000			3	/* 1000's special caps */
#define CAP_LRQ				4	/* LRQ Capable */
#define CAP_IRQ				5	/* Will respond to server IRQs */
#define CAP_ARQ				6	/* ARQ Capable */
#define CAP_SIP				7	/* SIP Capable device */
#define CAP_H323			8	/* H.323 Capable device */
#define CAP_ENUMS			9	/* ENUM Services Capable device */
#define CAP_GRQ				10	/* GRQ capable */
#define CAP_TPG				11	/* Tech Prefix Gateway */
#define CAP_RAI				12	/* RAI Capable device */
#define CAP_MEDIAROUTE		       	13	/* Media routing */
#define CAP_HIDEADDRESSCHANGE	       	14	/* hide mid-call address change */
#define CAP_MSW				15	/* iServer/MSW */

/* Extended Capabilities */
#define ECAPS1_NOH323DISPLAY 	0x1	/* Disable h323 display in outgoing setup*/
#define ECAPS1_NODIRECTCALLS	0x2	/* Direct Calls forbidden */
#define ECAPS1_MAPALIAS			0x4	/* Remap Alias */
#define ECAPS1_FORCEH245 		0x8	/* Send facility to start H.245 */
#define ECAPS1_NOCONNH245		0x10	/* Remove h.245 address from connect */
#define ECAPS1_NOMEDIAROUTE     0x20    /* always disable media routing */
#define ECAPS1_SETDESTTG    	0x40    /* do not set destination trunk group id */
#define ECAPS1_PIONFASTSTART	0x80
#define ECAPS1_DELTCST38		0x100	/* remove T38 from TCS */
#define ECAPS1_DELTCST38DFT		0x200	/* remove T38 from TCS, set to 1 for not using system default */
#define ECAPS1_DELTCSRFC2833	0x400	/* remove RFC2833 cap from TCS */
#define ECAPS1_DELTCSRFC2833DFT	0x800	/* remove RFC2833 cap from TCS, set to 1 for not using system default */
#define ECAPS1_NOTG				0x1000	/* do not send src trunk group id */
#define ECAPS1_CAP2833_KNOWN	0x8000	/* endpoint's 2833 capability: 0 - unknown, 1 -known */
#define ECAPS1_CAP2833			0x4000	/* endpoint supports 2833: 0 - no, 1 - yes */
#define ECAPS1_MAPISDNCC		0x2000	/* map ISDN CC */
#define ECAPS1_NATDETECT		0x010000	/* supports obp/nat detection if set to 1 */

/* 
   SIP_PRIVACY support. Ticket 5377 enchancement requires providing support for 
   sip privacy rfcs and drafts. These two extended capabilities are meant to reflect sip privacy
   supported by the destination endpoint.
*/

#define ECAPS1_SIP_PRIVACY_RFC3325          0x00020000   // SIP PRIVACY RFC 3325 support
#define ECAPS1_SIP_PRIVACY_DRAFT01          0x00040000   // SIP PRIVACY draft-ietf-sip-privacy-draft-01.txt 

typedef struct
{
	unsigned long	ipaddress;	/* IP Address */
	unsigned long	realmId;	/* realmid */
#define REALM_ID_TBD        0 /* Default realm */
} RealmIP;

typedef struct
{
	unsigned long	subnetip;	/* Subnet Id */
	unsigned long	mask;		/* IP mask */
} Subnet;

typedef struct
{
	unsigned long	subnetip;	/* Subnet Id */
	unsigned long	mask;		/* IP mask */
	unsigned long	realmId;	/* realmid */
} RealmSubnet;

typedef struct
{
	unsigned long	ipaddress;	/* IP Address */
	unsigned short	port;
} IPPort;

#if 0 /* moved to ls/rpc/pho_node.h */
/* IP address type */
typedef union
{
	unsigned long		l;
	unsigned char 		uc[4];
} IPaddr;

/*
 * Client credentials.
 * Also a Phonode structure. A phonode structure identifies
 * a phone endpoint on a node.
 */
typedef struct
{
	char 			regid[REG_ID_LEN];
	unsigned long	uport;
	IPaddr			ipaddress;	/* Physical IP Address */
	unsigned long	realmId;	/* realmid of the phonode */

	char			phone[PHONE_NUM_LEN];

  	char 			vpnPhone[VPN_LEN]; 	/* Vpn Id - sent to the vpns only */
  	unsigned long 	vpnExtLen;
  	unsigned short 	cap;			/* Capabilities of the terminal */

	unsigned short 	sflags;			/* Indicates what all is set in this */
	unsigned long	clientState;

#if 0	// password field eliminated in 2.1
	/* Pin or password, either encrypted or plain-text */
	/* Encryption is TBD. */
	char			password[PASSWORD_LEN];	
#endif

} ClientCredo, PhoNode;
#endif

typedef struct
{
	unsigned long ipaddress;
	unsigned short cport;
	unsigned short flags;
} SrvrNode;

#define NODE_PHONODE	1
#define NODE_SRVR		2
#define NODE_INTERNAL	3

typedef struct
{
	int		type;
	union 
	{
		SrvrNode 	srvr;
		PhoNode 	phonode;
	} unode;
} Node;

typedef enum
{
	nextoneNoError = 0		,	/* O is reserved */
	nextoneResourceUnavailable	,	/* Error unknown */
	nextoneInvalidEndpointID	,	/* Invalid Ser # or port */
	nextoneInvalidAlias		,	/* Bad phone num ? */
	nextoneMismatchedAlias		,	/* Mismatch w/ server cfg */
	nextoneNoAlias			,	/* No alias found */
	nextoneFirewallMismatch		,	/* Address mismatch */
	nextoneForwarded,
	nextoneRollover,
	nextoneProxied,
	nextoneDND,
	nextoneVpnGroupMismatch,
	nextoneZoneMismatch,
	nextoneMaxRecursion,
	nextoneNoEntry,
	nextoneInvalidVpn,
	nextoneGatewayInactive,
	nextoneGatewayInCall,
	nextoneHasReject,
	nextoneNoRoute,
	nextoneHasBetterMatch,
	nextoneRegistration,
	nextoneForwardedCP,
	nextoneRolloverCP,
	nextoneTimeMismatch,
	nextonePreferCallingPlan,
	nextonePreferPriority,
	nextonePreferLRU,
	nextonePreferVpn,
	nextonePreferZone,
	nextoneInferiorCallingPlan,
	nextoneInferiorPriority,
	nextoneInferiorLRU,
	nextoneInferiorVpn,
	nextoneInferiorZone,
	nextoneHigherUtilz,
	nextoneLowerUtilz,
	nextoneRouteLenMismatch,
	nextoneRouteTypeMismatch,
	nextoneNoPorts,
	nextoneNoRouteBinding,
	nextoneNoGateway,
	nextoneRejectRoute,
	nextonePreferSticky,
	nextoneStickyExists,
	nextoneReasonMax,	/* This should always be the last entry */
} NextoneReason;

typedef struct
{
	int branch;

	PhoNode xphonode, yphonode;	/* adjacency */

	// just pointers into permanent memory
	char *cpname;
	char *crname;
	unsigned int crflags;

	NextoneReason rejectReason;
	NextoneReason forwardReason;
} RouteNode;

/*
 * Register
 */
typedef struct
{
	PhoNode		node;
	NextoneReason	reason;
} RegisterInfo;

typedef struct
{
#define MAX_EMAIL_LEN	128
	char	email_address[MAX_EMAIL_LEN];
	char	fax_file_name[128];
	char	fax_directory[132];
	char	fax_user[20];
	char	fax_password[20];
} FaxServerData;

/*
 * Find
 */
typedef struct
{
	PhoNode		node;
	PhoNode		fnode;
	PhoNode		anode;
	
     	int		forward;   /* Forwarded number */
     	int		tickle;	   /* Waking up ... */
     	int		waitTime;  /* Wait before wakeup */
     	int		state;	   /* State of fnode */
        int             calltype;  /* Used by Aloid for number analysis */

#define CALL_NONE		0
#define CALL_H323V1		2
#define CALL_H323V2		3
#define CALL_PSTN		4
#define CALL_ITSP		5
#define CALL_NEXTONE_LUS	6
#define CALL_NEXTONE_INTRAVPN	7
#define CALL_NEXTONE_INTERVPN	8

	FaxServerData faxserverdata; /* Used for FAX Service */
#define find_email_address faxserverdata.email_address
} Find;

/*
 * Error message returned to client
 * Error codes and strings defined in ipcerror.h
 */
#define ERROR_MSG_LEN 80
typedef struct
{
	Node	node;
	Node	snode;			/* sender */
	int	errortype;
        int     errornumber;
        char    msg[ERROR_MSG_LEN];
} ErrorMsg;

/*
 * How our wakeup and hangup packet is structured.
 * This packet is sent to initiate connection from one Netoid to
 * another.
 */
typedef struct
{
	PhoNode		snode;
	PhoNode		dnode;
   	unsigned short	snetport;		/* Our UDP port for data */
    unsigned short	dnetport;		/* Our UDP port for data */
	FaxServerData faxserverdata; /* Used for FAX Service */
} Wakeup, Alerted, Busy, Connect, Hangup;

/*
 * DND packet.
 */
typedef struct
{
	PhoNode	node;
	int	enableCall;			/* Enable call to v-mail */
} DND;

/*
 * Duration types.
 */
typedef enum
{
	DR_UNTIL_REVOKED = 0,
	DR_PERMANENT,
	DR_TIMED,
} Duration_t;

/*
 * Redirect information.
 */
typedef struct
{
       	PhoNode		onode; 
       	PhoNode		nnode; 

	Duration_t	duration;	/* How long this stays in effect */

#define NEXTONE_REDIRECT_OFF		-1
#define NEXTONE_REDIRECT_FORWARD	0
#define NEXTONE_REDIRECT_ROLLOVER	1
#define NEXTONE_REDIRECT_PROXY		2
	int		protocol;

#define REDIRECT_GET	-1		/* Get the redirect information */
#define REDIRECT_OFF	0
#define REDIRECT_ON	1
	/* valid field is the one used to indicate redirect
	 * on/off/get by the client. It is never modified by
	 * the server. The server uses the protocol field,
	 * to indicate that fwding is off. This is a bit confusing,
	 * but is there for backward compatibility reasons.
	 */
	int		valid;		/* Toggle switch */
} Redirect;

/*
 * Control of stream.
 */
typedef struct
{
	PhoNode	snode;
	PhoNode	dnode;
	/* Control actions */
	int	hold;
	int	mute;
} Control;

/*
 * Transfer information.
 * The role field is redundant information in the packet.
 * It can be derived from the credo inside the header and the
 * 3 phonodes mentioned in the packet.
 */
typedef struct
{
    	PhoNode		transferer; 
    	PhoNode		transferee; 
    	PhoNode		dest; 
     	unsigned short	transfererTransfereeNetport;
     	unsigned short	transfererDestNetport;
     	unsigned short	transfereeNetport;
     	unsigned short	destNetport;
     
	unsigned short	role;		/* Role of PhoNode receiving this */
	unsigned short	unused;		/* Role of PhoNode receiving this */
#define TRANSFER_TRANSFERER	0	/* Negotiator */
#define TRANSFER_TRANSFEREE	1
#define TRANSFER_DEST		2
     	int        	localState;    /* local state which the transferer was with
				 	* dest
				 	*/
} Transfer;

/* LI values */
#define NEXTIME_OK		0
#define NEXTIME_ALARM		3	/* Clock not synchronized */

/* VN values */
#define NEXTIME_VERSION		3

/* MODE values */
#define NEXTIME_CLIENT		3
#define NEXTIME_SERVER		4

/* STRATUM values */
#define NEXTIME_UNAVAIL 	0
#define NEXTIME_RADIO		1
#define NEXTIME_NTP		14
#define NEXTIME_UTCLOCAL	15

typedef struct
{
#if NX_BYTE_ORDER == 21
	unsigned char 	li:2, vn:3, mode:3;
#else
	unsigned char 	mode:3, vn:3, li:2;
#endif
	unsigned char 	stratum;
	unsigned char 	reserved[15];		/* See SNTP, rfc 2030 */
	unsigned int 	referenceTimestamp[2];
	unsigned int 	originateTimestamp[2];
	unsigned int 	receiveTimestamp[2];
	unsigned int 	transmitTimestamp[2];
	unsigned long	clientReference;	/* not in rfc 2030 */
} NexTime;

/*
 * Profile information on call 
 */
#define CDR_CALLATTEMPT	0
#define CDR_CALLSETUP	1
#define CDR_CALLINP	2
#define CDR_CALLDROPPED	3
#define CDR_ORIGINATOR	4
#define CDR_CALLHUNT	5

/* If the following structure is sent as part of
 * a heartbeat, the remote, sRate, rRate, ctime, sltime,
 * rltime are all zeroed out. ftime in this case will represent
 * the local system time, on the system
 */
typedef struct {
     PhoNode local;
     PhoNode remote;

     unsigned long aloidIpAddress;
     unsigned long vpnsIpAddress;
     time_t ftime;      /* Finish Time - time at which call was finished */

     unsigned short lastSeqNo;

     unsigned short dspLoad;		/* Load value on DSP */
     unsigned long sRate;		/* sending rate */
     unsigned long rRate;		/* receiving rate */
     unsigned short flags;
     unsigned short unused;

     time_t ctime;      /* Creation Time - time at which call was made */
     time_t sltime;      /* Last Activity Start Time (LAST) - 
			 * time at which last activity started. 
			 * This mat be different from ctime, especially 
			 * in cases when the sending and receiving 
			 * activities may have different start times 
			 * than ctime... 
			 */
     time_t rltime;      
} CDR500, Profile, Heartbeat;

/*
 * QueryClient packet is sent with QUERYCLIENTS request.
 * 
 * restart:   set to 1 to start from beginning. If not set to 1, then
 * 		Aloid sends back from "nusers+1"
 * nusers :   if set to -1, then send back all users.
 *
 * phone  :   Optional field that indicates the phone number of a client
 *		that is requested.  Eventually, this field will be a way
 *		to support phone number wildcards.  If the requestor
 *		wishes to get all the registered clients, then this field
 *		should be cleared.
 */
typedef struct
{
    	char 		vpnId[VPN_LEN];
	char 		vpnExt[PHONE_NUM_LEN];
	char		phone[PHONE_NUM_LEN]; 
	unsigned long 	ipAddress;	
	unsigned long	uport;

} QueryClientIndex;

typedef struct
{
	char 		clFname[CLIENT_ATTR_LEN];
  	char 		clLname[CLIENT_ATTR_LEN];
  	char 		clLoc[CLIENT_ATTR_LEN]; /* City / State */
  	char 		clCountry[CLIENT_ATTR_LEN];
  	char 		clComments[CLIENT_ATTR_LEN];

} QueryClientData;

#define ALOID_MAXPKTSIZE_OUT 2048

#define QUERY_CLIENT_SIZE	\
                    (sizeof(QueryClientIndex)+sizeof(QueryClientData))
#define QUERY_CLIENT_MAX	\
     ((ALOID_MAXPKTSIZE_OUT-sizeof(Pkt))/(QUERY_CLIENT_SIZE))

typedef struct
{
  	char 		vpnId[VPN_LEN];

} QueryVpnsIndex;

typedef struct
{
  	unsigned long 	vpnExtLen;
  	unsigned long 	nUsers; 		/* No of users (make this long) */
  	char 		vpnName[VPNS_ATTR_LEN];
  	char 		vpnLoc[VPNS_ATTR_LEN];
  	char 		vpnContact[VPNS_ATTR_LEN];

} QueryVpnsData;

#define QUERY_VPNS_SIZE	(sizeof(QueryVpnsIndex)+sizeof(QueryVpnsData))
#define QUERY_VPNS_MAX	\
     			((ALOID_MAXPKTSIZE_OUT-sizeof(Pkt))/(QUERY_VPNS_SIZE))

typedef Node QueryNodeIndex;

#define CACHE_UPDATE_LOCAL	1
#define CACHE_UPDATE_LUS	2
#define CACHE_UPDATE_VPNS	3
#define CACHE_UPDATE_ADD	4
#define CACHE_UPDATE_DELETE	5
#define CACHE_UPDATE_MODIFY	6

typedef struct
{
	short flags;
	short unused;
	unsigned long srcIp, nodeIp;
} CacheIndex;

typedef struct
{
	unsigned long	uport;	 
} QueryCPIndex;

typedef struct
{
	char		cpname[CALLPLAN_ATTR_LEN];
} QueryCPData;

#define QUERY_CP_SIZE	\
                    (sizeof(QueryCPIndex)+sizeof(QueryCPData))
#define QUERY_CP_MAX	\
     ((ALOID_MAXPKTSIZE_OUT-sizeof(Pkt))/(QUERY_CP_SIZE))

typedef struct
{
	char            cpname[CALLPLAN_ATTR_LEN];
} QueryCRIndex;

typedef struct
{
	 char 		src[PHONE_NUM_LEN];
	 char 		dest[PHONE_NUM_LEN];
	 char 		prefix[PHONE_NUM_LEN];
} QueryCRData;

#define QUERY_CR_SIZE	\
                    (sizeof(QueryCRIndex)+sizeof(QueryCRData))
#define QUERY_CR_MAX	\
     ((ALOID_MAXPKTSIZE_OUT-sizeof(Pkt))/(QUERY_CR_SIZE))

/*
 * register srvr message contains the following index,
 * and the data contains ports, which are 4 bytes
 * each.
 */
typedef struct
{
	char 		regid[REG_ID_LEN];
	unsigned long	ipaddress;
} RegisterIndex;

typedef struct
{
	unsigned long	uport;
	char		phone[PHONE_NUM_LEN];

  	char 		vpnPhone[VPN_LEN]; 	/* Vpn Id - sent to the vpns only */
  	unsigned long 	vpnExtLen;
  	unsigned short 	cap;				/* Capabilities of the terminal */

	unsigned short 	sflags;				/* Indicates what all is set in 
											 * this */
	unsigned long	clientState;
	NextoneReason	reason;

} PortStatusData;
	
#define SRVR_REG_SIZE	(sizeof(long))
#define SRVR_REG_MAX	\
     ((ALOID_MAXPKTSIZE_OUT-sizeof(PktHeader)-sizeof(ServerMsg))/(SRVR_REG_SIZE))

#define SRVR_PORTSTATUS_SIZE	(sizeof(PortStatusData))
#define SRVR_PORTSTATUS_MAX	\
     ((ALOID_MAXPKTSIZE_OUT-sizeof(PktHeader)-sizeof(ServerMsg))/(SRVR_PORTSTATUS_SIZE))

typedef union
{
	QueryClientIndex 	clientIndex;
	QueryVpnsIndex		vpnsIndex;
	QueryNodeIndex		nodeIndex;
	CacheIndex		cacheIndex;
	QueryCPIndex		cpIndex;
	QueryCRIndex		crIndex;
	RegisterIndex		regIndex;

} Index, QueryIndex;

typedef enum
{
	ServerMsg_eQueryClient = 1,
	ServerMsg_eQueryVpns = 2,
	ServerMsg_eQueryNode = 3,
	ServerMsg_eQueryCP = 4,
	ServerMsg_eQueryCR = 5,
	ServerMsg_eRegister = 6,

	ServerMsg_eMax

} ServerMsg_tType;

typedef enum
{
  ServerMsg_eCont = 1,
  ServerMsg_eOk = 0,
  ServerMsg_eGenError = -1,
  ServerMsg_eOpUnsupp = -2, /* Op field is invalid */
  ServerMsg_eTypeUnsupp = -3, /* Type is unsupported */
  ServerMsg_eBadFormat = -4, /* Len is invalid etc */
  ServerMsg_eBadField = -5, /* Some field other than op 
			     * and type is invalid 
			     */
} ServerMsg_tStatus;

typedef enum
{
  Server_eOpGet = 1,
  Server_eOpGetNext,
  Server_eOpGetAll

} ServerMsg_tOp;
    
typedef struct
{
	ServerMsg_tOp 		opn;		/* Operation - 
						 * e.g., Get/GetN/GetAll 
						 */
    ServerMsg_tStatus 	status;		/* Response status */
	unsigned long 		reqId;		/* Request Id */	
	ServerMsg_tType 	subType;	/* SubType - 
						 * e.g., Client/Vpn 
						 */
	Index			index;		/* Index */
	unsigned long		indexMap; 	/* What is set... */
	unsigned long		blockLen; 	/* Length of each 
						 * block (index+data) 
						 */
} ServerMsgHeader, ServerMsg;

typedef char *ServerMsgFooter;

/* General Payload description.
 * 3 kinds of Packets:
 * Old Pkt Style, NIKE, PEF.
 * Each of the three has a common set of
 * header bytes, defined in PktHeader.
 */
/*
 * Definition of our 'Pkt' 
 */
typedef union
{
        RegisterInfo    reginfo;
        Find            find;
        ErrorMsg        errormsg;
        Wakeup          wakeup;
        Connect         connect;
        Hangup          hangup;
        DND             dnd;
        Redirect        redirect;
        Control         control;
        Transfer        transfer;
        Profile         profile;
        ServerMsg       serverMsg;
	NexTime		nextime;
} PktData;

/* First 2 bytes of type contain the version of
 * the protocol, Higher bytes is the major version, and
 * lower byte is the minor version.
 * This file describes version 0.0
 * Lower 2 bytes of the type field contain the actual
 * packet type, in network byte order.
 */
#define NEXPRO_V0_0	0x0000

typedef struct
{
        int  		type;
	unsigned long	totalLen; /* Total Length */

	ClientCredo	credo;
        PktData data;
} Pkt;

/* This PktHeader must be identical to the first section of Pkt
 */
typedef struct
{
  int  		type;
  unsigned long	totalLen; /* Total Length */

  ClientCredo	credo;    
} PktHeader;

typedef struct
{
  int  		type;
  unsigned long	totalLen; /* Total Length */
} GPktHeader;

#define COOKIE_LEN 8

typedef struct
{
     int  		type;
     unsigned long	totalLen; /* Total Length */

     ClientCredo	credo; 

     /* All the PEF stuff goes on here */
     unsigned char init_cookie[COOKIE_LEN];
     unsigned char resp_cookie[COOKIE_LEN];
     int next_payload;
#ifdef i386
     unsigned char minver:4,
	  majver:4;
#else
     unsigned char majver:4,
	  minver:4;
#endif
     unsigned char exch;
#define PEF_HDR_ENCR_BIT	0x0001
#define PEF_HDR_COLL_BIT	0x0002
     unsigned char flags;
     short unused;
     unsigned long mess_id;
     
     /* Reserved Fields */
     unsigned long reserved1;
     unsigned long reserved2;
} PefHeader;

#define PEF10_SetEncryption(x) (x->flags |= PEF_HDR_ENCR_BIT)
#define PEF10_GetEncryption(x) (x->flags & PEF_HDR_ENCR_BIT)

PefHeader *
PEF10_GetFrame(int, void **, int);

#define PEF10_GetPktFrame(x, y) \
              PEF10_GetFrame(x, (void **)y, sizeof(Pkt))

/* This comment is entirely for the Nextone Call Agents,
 * wherever they are, in the client, or in the server
 */

/*
 * DIGIT MAP RULES:
 * These rules are in general to be supplied by the
 * Aloid/Vpns. However, we for now hardcode this
 * in the netoid.
 * The rules are as follows:
 * Calling thru Aloid:
 * 	{7}{{7digits}|{10digits}|{1{10digits}}|{011{any#digits}}}
 * Calling thru the Vpns:
 * 	{HomeVpnDigits}|{Vpn-A{Vpn-A-#Digits}}|{Vpn-B{Vpn-B-#Digits}}
 * Calling thru POTS:
 *	{9}{{7digits}|{10digits}|{1{10digits}}|{011{any#digits}}}
 */

/* Error Codes For system */

/* Error Types */
#define XERRT_CONFIG_INTERNAL	1
#define XERRT_SERPLEX		3

/* Serplex Errors */
#define XERRS_REGISTERSERV	1
#define XERRS_REGISTER		2

#define XERRS_MAX		3

extern char 
	XErrorSStrings[XERRS_MAX][ERROR_MSG_LEN];

#define TM_ANY                  -1

// Generic structure used to store information about a file
typedef struct
{
	char filename[PHONE_NUM_LEN]; /* File name - max length 63 */
	FILE *fileptr;               /* File stream */
	time_t modtime;              /* File modification time */ 
} FileInfo;

#endif  /* _IPC_H_ */
