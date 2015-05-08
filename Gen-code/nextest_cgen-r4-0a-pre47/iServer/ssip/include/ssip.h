#ifndef _ssip_h_
#define _ssip_h_

/*
 * NOTE: THIS FILE SHOULD BE SELF SUFFICIENT
 * IN TERMS OF OTHER HEADER INCLUDES.
 * ie., IT SHOULD COMPILE ON ITS OWN.
 */

/* First include sip stack header files */

#include "portlayer.h"
#include "sipdecode.h"
#include "sipstruct.h"
#include "sipsendmessage.h"
#include "siplist.h"
#include "sipinit.h"
#include "sipfree.h"
#include "general.h"
#include "request.h"
#include "response.h"
#include "entity.h"
#include "ccp.h"
#include "rpr.h"
#include "header.h"
#include "sipstatistics.h"
#include "siptimer.h"
#include "siptrace.h"

#include "netloop.h"
#include "timer.h"
#include "header_url.h"


#define SipGetLocalDomain() (sipdomain)

#define SIP_MCAST_ADDR	"224.0.1.75"
#define SIP_PORT 		sipport
#define SIPMSG_LEN    	3000

#define SIPPROTO_UDP	0
#define SIPPROTO_TCP	1

#define SIPMSG_REQUEST	0
#define SIPMSG_RESPONSE	1
#define SIPMSG_REQUESTL	2	// Local message to TSM

/* Receive functions */
int SipControlPacketReceive (int csock, FD_MODE rw, void *data);
int sip_SigConnReceive (int csock, FD_MODE rw, void *data);

/* timers */
typedef SipBool (*siptimeoutfn) (SipTimerKey * key, SIP_Pvoid p_buf);

typedef struct {
	SipTimerKey * key;
	tid			sip_timerid;
	SIP_U32bit 	duration;
	SIP_S8bit	restart;
	siptimeoutfn	fn;
	SIP_Pvoid	buf;
} SIPTimerEntry;

#define HEADER_ADDRESS_H323	1
#define HEADER_ADDRESS_SIP	0

#if 0 /* moved to ls/rpc/header_url.h */

typedef struct
{
	SIP_S8bit *name;
	SIP_S8bit *value;
} SipUrlParameter;

#define SIP_MAXURLPARMS		10

typedef struct
{
    SIP_S8bit *name;
    SIP_S8bit *host;
    SIP_S8bit *tag;
	SIP_S8bit *display_name;
    SIP_S8bit *maddr;
	SipUrlParameter url_parameters[SIP_MAXURLPARMS];

	SIP_U16bit port;
	SIP_U16bit type;	/* SIP or H.323 */
	int realmId;
} header_url, SipHeaderUrl, SipHeaderUri;

typedef struct header_url_list
{
	struct header_url_list *prev, *next;
	header_url  *url;
} header_url_list;
#endif

SIPTimerEntry * SipSearchTimerInList(SipTimerKey * key);
int sipTimerExpiry(struct Timer*);

#define SVal(_x_)	((_x_)?(_x_):"")

typedef struct
{
	SipMessage *s;
	SipEventContext *context;
	int type;
	char *host;
	unsigned short port;
} SendArgs;

void 
SipIncomingRequest ( 
	SipMessage *s, 
	char *method,
	SipEventContext *context 
);

void 
SipIncomingResponse ( 
	SipMessage *s, 
	SipEventContext *context 
);

typedef struct
{
	int csock;
	char *pkt;
	int nbytes;
	SipEventContext *context;
} ReceiveArgs;

void * SipProcessIncomingPacket(void *threadArg);

int
SipDispatchIncomingPacket(
	int csock,
	char *pkt,
	int nbytes,
	SipEventContext *context
);

#define SipSendThreadLaunch(s, context,type, host, port)        \
			do { SipSendMsgToHost(s, context, type, host, port); \
				s = NULL; context = NULL; host = NULL; } while (0)

#define SipSendMsgToHost2(s, context,type, host, port)	\
			do { SipSendMsgToHost(s, context, type, host, port); \
				s = NULL; context = NULL; host = NULL; } while (0)

int
SipSendMsgToHost(
	SipMessage *s, 
	SipEventContext *context,
	int type,	/* SIPMSG defines */
	char *host,
	unsigned short port);

#include "codecs.h"
#include "sipkey.h"
#include "sipcall.h"
#include "siptrans.h"
#include "sipreg.h"

extern List SipStackTimers;
extern int lSipPort;
extern SipStats *sipStats;

int SipAddParm(SipHeaderUrl *url, char *parm, char *val, int memfn);
char * SipExtractParmVal(SipHeaderUrl *url, char *parm, int *exists);
void SipFreeRemotecontactList(header_url_list *remotecontact_list);
int SipChallengeMethod (char *method, unsigned int authFlags);
SipEventContext * SipDupContext(SipEventContext *context);
int SipInitNet (int sipport);
int SipSendMessage(SipMessage *s, unsigned long ip, unsigned short port,
        void *data, int proto);
int SipExtractMaddrFromUrl(SipUrl *sipurl, char **tmpptr, SipError *err);
int SipSetRoute(SipMessage *m, int nroutes, char **routes, header_url *rcontact);

int SipGetSentByHost(SipMessage *s, SipEventContext *context,
		int position,   /* Via Header position */
		int check_receive_parameter, 
		char **host, unsigned short *port, SipError *err);
int SipCheckMaxForwards(SipMessage *s);
int SipFormatResponse(SipMessage *s,
		SipEventContext *context,       /* context of request */
		SipMessage **r, int reason, char *reasonStr, 
		SIP_S8bit **hostname, SIP_U16bit *port, SipError *err);
SipBool SipHashViaBranch(SipMessage *m, SIP_S8bit *branchtoken, SipError *err);
int SipRegisterContact(SipMessage *s, SipEventContext **context, 
		CacheTableInfo *cacheInfo, int rxedFromNat, int generate_response);
SipBool SipInsertRealmVia (SipMessage *m, int protocol, SIP_S8bit *branchtoken, 
		char *rsadomain, SipError *err);
int SipCheckAuthorization(SipMessage *s, char *method);
int SipInsertAuthenticateHdr(SipMessage *s, char *method);
int SipCheckSourceAddressWithVia(SipMessage *s, SipEventContext *context,
		int viaPosition, int *rxedFromNat, SipError *err);
int SipCheckLoopUsingBranch(SipMessage *s,SIP_S8bit *branchtoken, SipError *err);
SipBool SipInsertRecordRoute(SipMessage *m, SipError *err);
SipBool SipPopRoute(SipMessage *m, SIP_S8bit **hostname, unsigned short *port,
		SipError *err);
int SipResolveCache(SipMessage *s, SipEventContext *context, char *fphone,
		CacheTableInfo *cacheInfo);
int SipReqUriFromIedgeEntry(InfoEntry *entry, header_url **newreq_uri_in);
int SipCheckTopVia(SipMessage *s, SipError *err, SipEventContext *context);
int SipMakeAuthorizationString(SipMessage *s, char *authenticate, en_HeaderType dType,
		char *authorization);
int SipReqUriFromPhonode (PhoNode *phonode, char *contact, header_url **newreq_uri_in);
void HashString (char *string, char *stringout);
SipBool sip_form_callid_inmsg (SipMessage *msg, SIP_S8bit *callid, SipError *err);
SipBool sip_form_cseqnum_inmsg (SipMessage *msg, int seqnum, char *method, SipError *err);
SipBool sip_form_contacthdr_inmsg (SipMessage *msg, header_url *contact, SipError *err);
int SSIPInit ();
int SSIPInitCont (int sipport);
int SipRealmReconfig(u_long rsa);
int SipExtractPrivacyParamsFromUrl(SipHeader *siphdr, SipPrivacyParams *priv_params);
int SipHandleIncomingNotifyMessage(SipMessage* s, char* method, SipEventContext* context);
	
SipError GetSipUnknownHeaders(SipMessage *s, SipAppMsgHandle *msghandle);
SipError SetSipUnknownHeaders(SipMessage *s, SipAppMsgHandle *msghandle);

#endif /* _ssip_h_ */
