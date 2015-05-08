#ifndef _key_h_
#define _key_h_

#include "unpifi.h"

typedef struct {
	char vpnName[VPNS_ATTR_LEN];
} VpnKey;

/* For the memory constrained Netoid box */
typedef struct {
	char vpnId[VPN_LEN];
	unsigned int vpnExtLen;
} VpnBasicEntry;

/* For the PC maybe */
typedef struct {
	char 		vpnName[VPNS_ATTR_LEN];

	char 		vpnId[VPN_LEN]; /* This is kind of index */
	unsigned int vpnExtLen;
	char 		cpname[CALLPLAN_ATTR_LEN];

	/* Prefix is used to define both the internal and 
	 * external route of the vpn. The internal route is
	 * implicitly ""(extLen)/prefix, and the external route
	 * is vpnId(extLen)/prefix. The external route is 
	 * associated with the vpn group. If this prefix is
	 * changed, the phone numbers of all iedges will also
	 * have to be changed.
	 */
	char 		prefix[PHONE_NUM_LEN];
	char		sipdomain[SIPURL_LEN];

	unsigned int nUsers; /* No of users */
	char 		vpnGroup[VPN_GROUP_LEN];
	char 		vpnLoc[VPNS_ATTR_LEN];
	char 		vpnContact[VPNS_ATTR_LEN];

	time_t  	mTime; /* Last Management Timestamp */
} VpnEntry;

/* Netoid Database Keys:
 * Combo of serial number abd port number
 * ip address and port number (network byte order... )
 */
/*** NOT 4 BYTE aligned ***/
typedef struct {
	char regid[REG_ID_LEN];
	unsigned int uport;
} NetoidSNKey;

typedef struct {
	unsigned int uport;
	unsigned int ipaddress;
} NetoidIPKey;

typedef struct {
	char phone[PHONE_NUM_LEN]; /* Last byte is null terminated */
} NetoidPhoneKey;

typedef struct {
	NetoidSNKey	snKey;
} NetoidPhoneEntry;

typedef struct {
	char email[EMAIL_LEN]; /* Last byte is null terminated */
} NetoidEmailKey;

typedef struct {
	NetoidSNKey	snKey;
} NetoidEmailEntry;

typedef struct {
	char vpnGroup[VPN_GROUP_LEN];
} VpnGroupKey;

typedef struct {
	char vpnGroup[VPN_GROUP_LEN];
	char cpname[CALLPLAN_ATTR_LEN];

	time_t  mTime;                      /* Last Management Timestamp */
} VpnGroupEntry;

/* The VpnGroupEntry will be packed up with 
 * vpn id's which will be VPN_LEN in length.
 */
	
#define NETOID_SN_KEY    0
#define NETOID_IP_KEY    1
#define NETOID_PHONE_KEY 2
#define NETOID_EMAIL_KEY 2
#define VPN_VPNID_KEY 0

/* Fax Database Key */
typedef struct {
	char dest_file_name[128];

} FaxKey;

/* Fax Entry */
typedef struct {
	int  fax_retry_count;
	int fax_phone_numtype;
#define LUS_TYPE				0x1
#define VPNS_TYPE				0x2
	char fax_phone_number[PHONE_NUM_LEN];
	char src_email_addr[128];
	char dest_email_addr[128];
	char dest_file_name[128];

} FaxEntry;

#define APPLY_PREFIX	0
#define APPLY_DEST		1

/* Call Route/Binding Flags */
#define CRF_POSITIVE	0x0002
#define CRF_CALLORIGIN	0x0010
#define CRF_CALLDEST	0x0020
#define CRF_VPNINT		0x0040
#define CRF_VPNEXT		0x0080
#define CRF_FORWARD		0x0100	/* This flag is only for the cp-cr binding */
#define CRF_ROLLOVER	0x0200	/* This flag is only for the cp-cr binding */
#define CRF_URIDEST		0x0400
#define CRF_TIME		0x0800
#define CRF_DNISDEFAULT	0x1000	/* Destination is default */
#define CRF_REJECT		0x2000
#define CRF_TRANSIT		0x4000
#define CRF_DYNAMICLRU	0x8000	/* Dynamic Route, LRU based */
#define CRF_TEMPLATE	0x10000	/* Template Route, not used for routing */
#define	CRF_STICKY		0x20000	/* Last stop route */

/* Vpn Routes */
typedef struct {
	char		crname[CALLPLAN_ATTR_LEN];

} VpnRouteKey, CallPlanRouteKey, RouteKey;

typedef struct {
	char		crname[CALLPLAN_ATTR_LEN];

	char 		src[VPN_LEN];
	short		srclen;	/* len=0 means infinite */
	short		destlen;	/* len=0 means infinite */
	char 		srcprefix[PHONE_NUM_LEN];
	char		tmpsrcprefix[ANIGEN_LEN];	// internal for storing last assignments
	char 		dest[VPN_LEN];
	char 		prefix[PHONE_NUM_LEN];
	time_t		startTime, endTime;	/* Applicable time */
	unsigned int crflags;
		
	time_t  	mTime;		/* Last Management Timestamp */

	time_t		rTime;		/* Last Use Time */

	char    	cpname[CALLPLAN_ATTR_LEN];	/* option plan name to have a binding */
	char		trname[TRIGGER_ATTR_LEN];	/* any trigger name associated with this route */

} VpnRouteEntry, CallPlanRouteEntry, RouteEntry;

typedef struct {
	char    	cpname[CALLPLAN_ATTR_LEN];
} CallPlanKey;

// Will be part of calling plans eventually
#define CP_ROUTE_LRU	0	// Least Recently Used
#define CP_ROUTE_UTILZ	1	// Utilization

typedef struct {
	char		cpname[CALLPLAN_ATTR_LEN];
	char		pcpname[CALLPLAN_ATTR_LEN];	/* parent cp */
	char		vpnGroup[VPN_GROUP_LEN];

	time_t  	mTime;		/* Last Management Timestamp */
} CallPlanEntry;

typedef struct
{
	char		cpname[CALLPLAN_ATTR_LEN];
	char		crname[CALLPLAN_ATTR_LEN];

} CallPlanBindKey;

typedef struct
{
	char		cpname[CALLPLAN_ATTR_LEN];
	char		crname[CALLPLAN_ATTR_LEN];

	struct tm	sTime, fTime;	/* Interval during which the route applies */
	int			priority;		/* Priority of the route in the cp */
	unsigned int crflags;

	time_t		mTime;

} CallPlanBindEntry;

#define TRIGGER_EVENT_H323REQMODEFAX	0
#define TRIGGER_EVENT_H323T38FAX	1

#define TRIGGER_ACTION_INSERTROUTE		0

/*
** when this flag is enabled, the trigger route is applied directly on the dialled number.
** otherwise, the route is applied in series with the existing route that was applied on
** the call during which the trigger happened.
*/
#define TRIGGER_FLAG_ROUTE_OVERRIDE		1

typedef struct
{
	char		name[TRIGGER_ATTR_LEN];

} TriggerKey;

typedef struct
{
	char		name[TRIGGER_ATTR_LEN];
	
	// event information
	int			event;
	int			srcvendor;
	int			dstvendor;

	// action information
	int			action;
	char		actiondata[TRIGGER_ATTR_LEN];
	int			actionflags;

} TriggerEntry;

/*
 * Realm Cache and DB related declarations
 */
typedef enum 
{
    ENDPOINT_PUBLIC_ADDRESS,
    ENDPOINT_PRIVATE_ADDRESS
  
} Address_e;


typedef struct
{
	char		realmNameKey[REALM_NAME_LEN];
} RealmKey;

// none - no realm assigned
#define REALM_UNASSIGNED		""
#define REALM_ID_UNASSIGNED		-1

#define REALM_ANY				"any"
#define REALM_ID_ANY			-2

#define REALM_ID_INVALID		0xdeadbeef

#define RealmIdIsValid(_x_)		((_x_ != REALM_ID_UNASSIGNED) && \
								(_x_ != REALM_ID_ANY) && \
								(_x_ != REALM_ID_INVALID))

#define REALMF_DEFAULT		0x00000001

#define VNET_UNASSIGNED	      ""
#define VLANID_NONE_STR       "None"
#define VNET_VLANID           0
#define VNET_RTGTBLID         1
#define VNET_GATEWAY          2

typedef struct
{
	unsigned char		vnetNameKey[VNET_NAME_LEN];
} VnetKey;

typedef struct
{
    unsigned char   vnetName[VNET_NAME_LEN]; 
    char            ifName[IFI_NAME];
    unsigned short  vlanid;   /* VLAN Identifier */
    unsigned short  rtgTblId; /* Routing table ID */
    unsigned long   gateway;  /* Gateway IP Address */
} VnetEntry;

typedef struct
{
    unsigned long   realmId;  /* The cache key */
    unsigned char   realmName[REALM_NAME_LEN]; 
    unsigned long   rsa;      /* Realm Signalling IP Address */
    unsigned long   mask;      /* Realm Signalling Net Mask */
    unsigned int    sigPoolId;
    unsigned int    medPoolId;
    unsigned int    adminStatus;
	unsigned int	operStatus;
    Address_e       addrType;
    unsigned short 	interRealm_mr;
    unsigned short 	intraRealm_mr;
#define MR_OFF_VALUE		0	
#define MR_ON_VALUE			1	
#define MR_DEFAULT_LEVEL	0
#define MR_ALWAYS_LEVEL		1		
#define MR_EP_LEVEL			1

#define MEDIA_ROUTING_DONT_CARE    	((1 << MR_DEFAULT_LEVEL) | (MR_OFF_VALUE))
#define MEDIA_ROUTING_ON    		((1 << MR_DEFAULT_LEVEL) | (MR_ON_VALUE))
#define MEDIA_ROUTING_ALWAYS_OFF    ((1 << MR_ALWAYS_LEVEL) | (MR_OFF_VALUE))
#define MEDIA_ROUTING_ALWAYS_ON    	((1 << MR_ALWAYS_LEVEL) | (MR_ON_VALUE))
#define MEDIA_ROUTING_EP_OFF    	((1 << MR_EP_LEVEL) | (MR_OFF_VALUE))
#define MEDIA_ROUTING_EP_ON    		((1 << MR_EP_LEVEL) | (MR_ON_VALUE))
    char           	vipName[IFI_NAME]; 
	unsigned long	flags;

        PhoNode      mp; // MirrorProxy node        
	unsigned int	authFlags;
#define REALM_SIP_AUTH_INVITE	0x00000001
#define REALM_SIP_AUTH_REGISTER 0x00000002
#define REALM_SIP_AUTH_BYE	0x00000004

    char            cidblk[CID_BLK_UNBLK_LEN];
    char            cidunblk[CID_BLK_UNBLK_LEN];        

    unsigned char   vnetName[VNET_NAME_LEN]; 
    char            ifName[IFI_NAME];

} RealmEntry;

#define FCE_MAX(a,b)    			((a) > (b) ? (a) : (b))
#define RealmMediaRouting(mr1, mr2) ((FCE_MAX(mr1, mr2))%2)
#define EP_mr_cap(ep, r)  ( (IsMediaRoutingEnabled(ep)) ? (MEDIA_ROUTING_EP_ON) : \
			( (IsNeverMediaRouteEnabled(ep)) ? (MEDIA_ROUTING_EP_OFF) : (r) ) )

#define RealmBasedMR(r1,r2) ( ((r1)->realmId == (r2)->realmId) ? \
			(RealmMediaRouting((r1)->intraRealm_mr, (r2)->intraRealm_mr)) : \
			(RealmMediaRouting((r1)->interRealm_mr, (r2)->interRealm_mr)) )

// ep1, ep2 may either be callhandle or netinfo. both have a cap flag with the MR capability
#define EPBasedMR(r1,r2,ep1,ep2) ( (((r1)->realmId == (r2)->realmId) ? \
			(RealmMediaRouting(EP_mr_cap(ep1, (r1)->intraRealm_mr), EP_mr_cap(ep2, (r2)->intraRealm_mr))) : \
			(RealmMediaRouting(EP_mr_cap(ep1, (r1)->interRealm_mr), EP_mr_cap(ep2, (r2)->interRealm_mr))) ) )

typedef struct
{
	unsigned char	igrpName[IGRP_NAME_LEN];
	/* Configurable fields */
	short	nMaxCallsIn; 	/* max ingress calls */
	short	nMaxCallsOut; 	/* max egress calls */
	short	nMaxCallsTotal; /* Total ingress + egress calls */

	/* Accounting fields */
	short	nCallsIn; 		/* ingress calls */
	short	nCallsOut; 		/* egress calls */
	short	nCallsTotal; 	/* nCallsIn + nCallsOut */

	time_t			dndTime; /* Last time we ran out of calls */
}IgrpInfo;

#define IgrpInCalls(p)		((p)->nCallsIn)
#define IgrpOutCalls(p)		((p)->nCallsOut)
#define IgrpCallsTotal(p)	((p)->nCallsTotal)

#define IgrpXCallsTotal(p)	((p)->nMaxCallsTotal)
#define IgrpXInCalls(p)		((p)->nMaxCallsIn)
#define IgrpXOutCalls(p)	((p)->nMaxCallsOut)

typedef struct
{
	unsigned char	igrpNameKey[IGRP_NAME_LEN];
} IgrpKey;
#endif /* _key_h_ */

