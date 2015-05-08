#ifndef _server_db_h
#define _server_db_h

#define PORT_ARRAY_LEN	(sizeof(long)*(MAX_IEDGE_PORTS/sizeof(long)+1))

/* Info entries in the database must be stored in network
 * byte order. In the cache these entries are stored in network
 * byte order also.
 */
typedef enum {
		Vendor_eGeneric,
		Vendor_eClarent,
		Vendor_eSonusGSX,
		Vendor_eBroadsoft,
		Vendor_eCisco,
		Vendor_eVocalTec,
		Vendor_eLucentTnt,
		Vendor_eExcel,
		Vendor_eAvaya,
		Vendor_eNortel,
} Vendor;
		
// vendor description macros
#define VendorSupportsFirstInviteNoSDP(vendor)	((vendor != Vendor_eSonusGSX) && \
												(vendor != Vendor_eBroadsoft))
#define VendorSupportsNonStdAckSDP(vendor)	(vendor == Vendor_eSonusGSX)

#define PASSWD_LEN 64 

#define Q931IE_LEN 4 
#define	Q931IE_CDPN 0
#define	Q931IE_CGPN 1

#define BCAP_LEN	4
#define BCAP_LAYER1 0
#define BCAP_XFERCAP 1
#define BCAP_XFERRATE 2

typedef struct {
     	/* Global information about the netoid */
     	char              	regid[REG_ID_LEN];		// leave as first two entries
     	unsigned long     	uport;

     	IPaddr            	ipaddress;

		/* PHONE of this iedge (derived from vpnPhone, vpn) */
     	char              	phone[PHONE_NUM_LEN]; 

		/* VPN this iedge belongs to */
		char				vpnName[VPNS_ATTR_LEN];

     	char 				vpnPhone[VPN_LEN]; 	
     	unsigned long 		vpnExtLen;

     	unsigned short     	sflags;
     	unsigned short     	dflags;  

     	long               	stateFlags;
     
     	/* Redirect information */
     	char               	nphone[PHONE_NUM_LEN];
     	char 			nvpnPhone[VPN_LEN]; 	
     	unsigned long 		nvpnExtLen;
     	int	       		protocol;

     	unsigned short     	nsflags;
     	unsigned short		ispcorgw;

#define IEDGE_500		0
#define IEDGE_IMOBILE	1
#define IEDGE_510		2
#define IEDGE_1000		3
#define IEDGE_XGW		4	/* External Gateway device */
#define IEDGE_ISERVER	5	/* Peer iServer */
#define IEDGE_XGK		6	/* Peer Gatekeeper */
#define IEDGE_SGK		7	/* Super Gatekeeper */
#define IEDGE_USERACC	8	/* User Account (may be device/user) */
#define IEDGE_ENUMS		9	/* Enum Server */
#define IEDGE_IPPHONE	10	/* IP Phone */
#define IEDGE_SIPPROXY	11	/* Sip Proxy server */
#define IEDGE_SOFTSW	12	/* Softswitch */
#define IEDGE_SIPGW		13	/* Sip Gateway */
#define IEDGE_1000G		14
#define IEDGE_SUBNET	15	/* Not a specific endpoint, but the whole subnet */

#define IEDGE_MAXTYPES	16	/* One more than the last */
     
     	/* Various timing information */
     	time_t  		iTime;          /* Inception Time */
     	time_t  		mTime;          /* Last Management Timestamp */
     	time_t  		cTime;          /* Last Connection Time */
     	time_t  		rTime;          /* Last Refresh Time (Aging) */
		time_t			iaTime;			/* Last time the endpt became inactive */
		time_t			attTime;		/* Last call attempt (candidate) */
		time_t			dndTime;		/* Last time the endpt reported dnd/ran
										 * out of ports, when we were routing calls
										 * to it
										 */
     
     	char 			zone[ZONE_LEN];
     
     	char 			cpname[CALLPLAN_ATTR_LEN];

     	unsigned short 		cap;	/* Capabilities of the terminal */

	unsigned short		rasport;
	unsigned long		rasip;

	unsigned char		ports[PORT_ARRAY_LEN];
	unsigned short		nports;		/* Used only in IP address cache */

	unsigned char		cfgports[PORT_ARRAY_LEN];
	unsigned short		ncfgports;		/* Used only in reg id cache */

	unsigned short		callsigport;	/* Q.931 port */
	
    char 			email[EMAIL_LEN];

	/* h.323 Id */
	/* This will contain what is registered by this endpoint, 
	* or whatis registered by us to this endpoint, in case this
	* endpoint is a gatekeeper to whom we do ARQs.
	*/
	char			h323id[H323ID_LEN];
	char			pgkid[GKID_LEN];/* GKID for peer gatekeeper */

	char			uri[SIPURL_LEN];/* Full SIP URI */
	char			contact[SIPURL_LEN];

	/* used for SIP NAT traversal */
	unsigned long		natIp;
	unsigned short		natPort;

	// macros defined below to access these fields
	short			ncallsIn;	/* Current no of ingress calls active */
	short			ncallsOut;	/* Current no of egress calls active */
	short			ncalls;		/* sum of ncallsIn and ncallsOut, maintained here
								 * to optmimize on the no of adds done at runtime
								 */
	short			xcalls;		/* Max active calls allowed */
	short			xcallsIn;	/* Max active calls ingress */
	short			xcallsOut;	/* Max active calls egress */

	int				priority;	/* priority of choosing this gateway */
	char			techprefix[PHONE_NUM_LEN];

	// for the time, keep the subnet ip and mask separate from ip
	unsigned long		subnetip;
	unsigned long		subnetmask;
	int					vendor; /* Endpoint vendor */
	char			passwd[PASSWD_LEN]; /* password for H.235 */
	
	int				maxHunts;	// Endpoint level max hunting
	unsigned int	ecaps1;  /* Extended capabilities */

	unsigned int	crId;		// if endpoint, then sgatekeeper's id
								// if sgatekeeper, then index for the entry
	char			custID[CLIENT_ATTR_LEN];
	unsigned char	q931IE[Q931IE_LEN]; // Q.931 Information Element
	unsigned char	bcap[BCAP_LEN]; // Bearer Capability

	// Trunk group to be used for ingress/egress calls on this port
	// Not a prefix
	char			tg[PHONE_NUM_LEN];
	char			srcIngressTG[PHONE_NUM_LEN]; // new source ingress trunk group ID

	// Outgoing prefix to be attached on all calls using this gw
	char			ogprefix[PHONE_NUM_LEN];
    time_t  		rejectTime;          /* Used for managing reject routes */
	short			infoTransCap;		/* Q.391 Information Transfer Capability */

	unsigned char	realmName[REALM_NAME_LEN]; // User configurable
	unsigned int	realmId;  // Assigned

	unsigned char	igrpName[IGRP_NAME_LEN];
	char			qval[4];			/* For SIP registration */

	char			srcEgressTG[PHONE_NUM_LEN];
	char			dtg[PHONE_NUM_LEN];
        unsigned short          cidblock;        
} InfoEntry;


#define	Q931CDPN_Default		0x0 	// Default is Public Unknown
#define	Q931CDPN_Unknown 		0x1
#define	Q931CDPN_International 	0x2
#define	Q931CDPN_National		0x3
#define	Q931CDPN_Specific		0x4
#define	Q931CDPN_Subscriber		0x5
#define	Q931CDPN_Abbreviated	0x7
#define Q931CDPN_Pass			0x50

// The #defs are off by 1
#define	Q931CDPNCode(x) ((x-1))
		
#define	Q931CGPN_Pass			0x0 	// Default is Pass-through
#define	Q931CGPN_Unknown 		0x1
#define	Q931CGPN_International 	0x2
#define	Q931CGPN_National		0x3
#define	Q931CGPN_Specific		0x4
#define	Q931CGPN_Subscriber		0x5
#define	Q931CGPN_Abbreviated	0x7

#define BCAPLAYER1_Default 	0
#define BCAPLAYER1_G711ulaw	2
#define BCAPLAYER1_G711alaw	3
#define BCAPLAYER1_H221		5
#define BCAPLAYER1_Pass	 	33	


/* Extra attributes each client may have.
 * This is indexed on regid/uport in the database
 * as well as cache
 */ 
typedef struct
{
     	/* Each client Attributes */
     	char 			clFname[CLIENT_ATTR_LEN];
     	char 			clLname[CLIENT_ATTR_LEN];
     	char 			clLoc[CLIENT_ATTR_LEN]; /* City / State */
     	char 			clCountry[CLIENT_ATTR_LEN];
     	char 			clComments[CLIENT_ATTR_LEN];

} ClientAttribs;

typedef InfoEntry NetoidInfoEntry;

/*
 * Convenience macros.
 */
#define InitInfoEntry(x) 		memset(x, 0, sizeof(InfoEntry))
#define InitNetoidInfoEntry(x) 	InitInfoEntry(x)

#define GetNewInfoEntry() 		(InfoEntry *)malloc(sizeof(InfoEntry))
#define GetNewNetInfoEntry()  	(NetoidInfoEntry *)GetNewInfoEntry()

#define IsGateway(infoEntry)	(BIT_TEST((infoEntry)->cap, CAP_IGATEWAY))
#define IsSGatekeeper(infoEntry)	(BIT_TEST((infoEntry)->cap, CAP_ARQ))
#define IsPGatekeeper(infoEntry)	(BIT_TEST((infoEntry)->cap, CAP_LRQ))
#define IsGatekeeper(infoEntry)	(IsSGatekeeper(infoEntry) || IsPGatekeeper(infoEntry))

#define IsSubnetEndpoint(infoEntry)	((infoEntry)->ispcorgw == IEDGE_SUBNET)
#define IsIServer(infoEntry)	((infoEntry)->ispcorgw == IEDGE_ISERVER)

#define IsMediaRoutingEnabled(infoEntry)	(BIT_TEST((infoEntry)->cap, CAP_MEDIAROUTE))
#define IsHideAddressChangeEnabled(infoEntry)	(BIT_TEST((infoEntry)->cap, CAP_HIDEADDRESSCHANGE))
#define IsNeverMediaRouteEnabled(infoEntry)     ((infoEntry)->ecaps1 & ECAPS1_NOMEDIAROUTE)

#define Is2833CapKnown(ecaps1)	((ecaps1) & ECAPS1_CAP2833_KNOWN)
#define Is2833Supported(ecaps1)	( (Is2833CapKnown(ecaps1)) && ((ecaps1) & ECAPS1_CAP2833) )
#define Is2833NotSupported(ecaps1)	( (Is2833CapKnown(ecaps1)) && (!((ecaps1) & ECAPS1_CAP2833)) )

/* prototype of function called by routing library for logging/display */
typedef int (*RouteLogFn)(RouteNode *);
extern char NextoneReasonNames[nextoneReasonMax][ERROR_MSG_LEN];
char *IedgeName(int type);

// Define the types allowed to do registration as all except :
#define IsRegistrable(dtype)	(((dtype) != IEDGE_SGK) &&\
								((dtype) != IEDGE_ISERVER) &&\
								((dtype) != IEDGE_XGK) &&\
								((dtype) != IEDGE_ENUMS)) 

#define IedgeInCalls(infoEntry)		((infoEntry)->ncallsIn)
#define IedgeOutCalls(infoEntry)	((infoEntry)->ncallsOut)
#define IedgeCalls(infoEntry)		((infoEntry)->ncalls)
#define IedgeXCalls(infoEntry)		((infoEntry)->xcalls)
#define IedgeXInCalls(infoEntry)	((infoEntry)->xcallsIn)
#define IedgeXOutCalls(infoEntry)	((infoEntry)->xcallsOut)
#define IedgeInfoTransCap(infoEntry)	((infoEntry)->infoTransCap)

#define	INFO_TRANSCAP_BASE				100
#define	INFO_TRANSCAP_SPEECH			(INFO_TRANSCAP_BASE + 0)	/* Speech */
#define	INFO_TRANSCAP_UNRESTRICTED		(INFO_TRANSCAP_BASE + 8)	/* Unrestricted digital information */
#define	INFO_TRANSCAP_RESTRICTED		(INFO_TRANSCAP_BASE + 9)	/* Restricted digital information */
#define	INFO_TRANSCAP_AUDIO				(INFO_TRANSCAP_BASE + 16)	/* 3.1 kHz audio */
#define	INFO_TRANSCAP_UNRESTRICTEDTONES	(INFO_TRANSCAP_BASE + 17)	/* Unrestricted digital information with tones/announcements */
#define	INFO_TRANSCAP_VIDEO				(INFO_TRANSCAP_BASE + 24)	/* Video */

#define INFO_TRANSCAP_PASS				1	/* pass through what ever it gets from caller */
#define INFO_TRANSCAP_DEFAULT			0

typedef struct enumString_tag {
	char *str;
	int  number;
}enumString;
extern enumString infoTransCapOptions[];
extern int str2enum( enumString *enumStrArray,  char *str );
extern char * enum2str( enumString *enumStrArray,  int number );

#endif /* _server_db_h */

