#ifndef _handles_h_
#define _handles_h_

#include "ipcerror.h"

/* The maximum number of times a phone
 * can be forwarded
 */
#define MAX_FIND_RECURSION	5

#define PKT_FOUND_NODE(pkt, branch) \
	((branch == 0) ? &pkt->data.find.fnode : \
		&reply_pkt->data.find.anode)

#define RH_FOUND_NODE(handle, branch) \
	((branch == 0) ? ((handle)->rfphonodep) : \
		((handle)->rfphonodep))
#define RH_FOUND_CACHE(handle, branch) \
	((branch == 0) ? (&((handle)->rfCacheInfoEntry)) : \
		(&((handle)->rfCacheInfoEntry)))

#define LS_READ_SINGLESHOT	4096	/* Attempt to read 4k */

#define GIS_MAX_HUNTNODES	10

/* ClientHandle is the general entry point
 * into the iServer, for any client request,
 * coming in, or made by the iServer.
 * It has a common part, and then protocol specific
 * parts.
 */
typedef struct
{
	/* File descriptor for the connection handle */
	int 			fd;

	/* input packet for this handle */
	long	  		data_in[LS_READ_SINGLESHOT/sizeof(long)+1];

} UCCClientHandle;

typedef struct
{
	int 			type;
	void			*h323handle;

} H323ClientHandle;

/* Client handle types */
#define CLIENTHANDLE_UCC		1
#define CLIENTHANDLE_H323LRQ	2
#define CLIENTHANDLE_H323ARQ	3
#define CLIENTHANDLE_H323CALL	5
#define CLIENTHANDLE_H323SETUP 	6

typedef struct
{
	/* This handle is in the process of being freed, OR is freed */
	int			isFree;

	/* Origin = 1, means iServer is the origin, else, client is */
	int			origin;

	/* Type of ClientHandle */
	int 			type;
	void *			handle;

	/* Timestamp on when the connection was started, ie.
	 * this entry was created.
	 */
	time_t			ctime;

	/* Timestamp for last time when there was any
	 * communication with the client.
	 */
	time_t			rtime;

	/* remote address */
	struct sockaddr_in client;

	/* Resolution handle for FINDs/LRQs */
	void *			rhandle;

} ClientHandle;

#define CH(chandle)				((ClientHandle *)(chandle))
#define CHFree(chandle)			(CH(chandle)->isFree)
#define CHOrigin(chandle)		(CH(chandle)->origin)
#define CHType(chandle)			(CH(chandle)->type)
#define	CHRHandle(chandle)		(((ResolveHandle *)CH(chandle)->rhandle))
#define	CHUCCHandle(chandle)	((UCCClientHandle *)(CH(chandle)->handle))
#define CHH323Handle(chandle)	((H323ClientHandle *)(CH(chandle)->handle))
#define H323LRQHandle(_handle_)		((LRQHandle *)((_handle_)->h323handle))
#define H323ARQHandle(_handle_)		((ARQHandle *)((_handle_)->h323handle))
#define H323SetupHandle(_handle_)	((SetupHandle *)((_handle_)->h323handle))
#define H323CallHandle(_handle_)	((CallHandle *)((_handle_)->h323handle))

#if 0
// Replacement of the PhoNode structure
// the use of the PhoNode struct is on the network,
// internally we will use this struct.
// This structure will NOT have any policy information
// unlike the InfoEntry structure, from database
// also this structure may be partially filled, depending
// on usage.
typedef struct
{
	// nextone
	char 			*regid;
	unsigned int 	uport;

	// common
	char 			*phone;
	unsigned int 	ipaddr;

	// h.323
	char 			*h323id;
	unsigned long	rasip;
	unsigned short	rasport;
	unsigned short	callsigport;

	// sip
	char 			*host;	
	header_url		*uri;
	header_url		*contact;

} EndptInfo;

typedef struct
{
	EndptInfo *src;
	EndptInfo *dst;

} CallEndptInfo;
#endif

/* List of Active ClientHandles, which may need
 * monitoring for activity
 */
/* All information about how the FIND has been
 * processed so far. ResolveHandle, is used
 * only for FINDs and LRQs, and not for ARQs
 * or CallSetups.
 */
typedef struct
{
	/* FOUND or NOT_FOUND, or UNINITIALIZED */
	int				result;

	/* Source Information, if there is any */
	PhoNode *		phonodep;
	VpnEntry		sVpn;
	char			sZone[ZONE_LEN];
	char			*scpname;

	/* Destination information. This is initialized to
	 * what we want to find, and returned modified by the
	 * library to the new destination which we need
	 * to call
	 */
	CacheTableInfo	rfCacheInfoEntry;

	/* The following information is derived from
	 * rfCacheInfoEntry
	 */
	PhoNode 		*rfphonodep;
	unsigned long	rfrasaddr;
	unsigned short 	rfrasport;
	unsigned short	rfcallsigport;

	/* Configuration of the destination which
	 * we found is contained in the following entry
	 */
	CacheTableInfo	fCacheInfoEntry;

	/* Policy settings */
	int				checkZone;
	int				checkVpnGroup;

	/* Handles */
	ClientHandle 	*chandle;			/* Incoming client handle */
	ClientHandle 	*rfchandle;			/* Outgoing client request handle */

	int				primary;			/* 0=primary, 1=rollover, -1=both */
	int				reservePort;  		/* Reserve the gateway port. Represents
										 * the logical value of routeCall, ie.,
										 * runtime value of routeCall
										 */
	int 			resolveRemote; 		/* If not present in local cache - try*/
										/* for a remote resolution */
	int				phoneChange;		/* Should we change the phone and
										 * then tell the caller about new phone
										 * based on the gw calling plan ?
										 */
	int				callError;			/* If no entry found, reason */

	int				findMSW;			/* Look for an MSW */
	ListEntry		*destRejectList;	/* list of dests not to be considered */
	char			*dtg;				/* destination trunk group */

	char			*crname;			/* Route used in the resolve - points to caller API struct */

	/* these fields record information about the routes applied */
	char			*srccrname;
	char			*transitcrname;
	char			*destcrname;

#define SRC_ROUTE_APPLIED               0x00000001
#define TRANSIT_ROUTE_APPLIED           0x00000002
#define DEST_ROUTE_APPLIED              0x00000004

	int			routeflag;
	char			*transRouteNumber; /* number after applying the transit route */

} ResolveHandle;

#define PhoneChange(rhandle) ((rhandle)->reservePort || !routecall || \
								(rhandle)->phoneChange)

ResolveHandle *		GisAllocRHandle			(void);
void 				GisFreeRHandle			(void *ptr);
void GisFreeEgressTokenNodeId(void *ptr);

int
ResolvePhoneCache (
	ResolveHandle *rhandle,	/* resolve handle */
	CacheTableInfo *cacheInfo, /* Cache entry for phone which we found */
	RouteLogFn logfn
);

int
ResolvePhoneLocally(
	ResolveHandle *rhandle,
	RouteLogFn logfn
);

int
ResolvePhone(
	ResolveHandle *rhandle
);

#define IsPrimaryResolve(rhandle)	((rhandle->primary == 0) || \
										(rhandle->primary == -1))
#define IsSecondaryResolve(rhandle)	((rhandle->primary == 1) || \
										(rhandle->primary == -1))

int IsRolledOver(char * phone);

void RealmInfoFree(CallRealmInfo *ri, int freefn);

CallRealmInfo * RealmInfoDup (CallRealmInfo *ri, int mallocfn);

#endif /* _handles_h_ */
