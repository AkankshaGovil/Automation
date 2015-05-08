#ifndef _gis_h_
#define _gis_h_

/*
 * NOTE: THIS FILE SHOULD BE SELF SUFFICIENT
 * IN TERMS OF OTHER HEADER INCLUDES.
 * ie., IT SHOULD COMPILE ON ITS OWN.
 */

#include <netinet/in.h>
#include <time.h>
#include "timedef.h"
#include "bits.h"
#include "ipc.h"
#include "ipcerror.h"
#include "list.h"
#include "timer.h"
#include "mem.h"
#include "serverp.h"
#include "mealeysm.h"
#include "calldefs.h"
#ifdef ALLOW_ISERVER_SIP
#include "ssip.h"
#endif
#include "uh323.h"
#include "callcache.h"
#include "arq.h"
#include "lrq.h"
#include "callconf.h"

#include "handles.h"
#include "gw.h"

#include "siputils.h"

#include "codemap.h"

#define REGISTERED_ENTRY		0x1

extern List gisActiveCHandles;

extern List ANIFileList;

/* SVH */
extern char SgenGwIpAddr[256];
extern int GenMode;
extern int callsdisconnected;

/* Command string that GIS can issue to execd */
extern char  *GisCmdStr[];

/* Index into GisCmdStr array for command strings */
#define GIS_REALM_OPEN_ALL_CMDID        0
#define GIS_REALM_CLOSE_ALL_CMDID       1
#define GIS_REALM_UP_ALL_CMDID          2
#define GIS_REALM_DOWN_ALL_CMDID        3

extern unsigned long iServerIP;

/* Prototypes */
ResolveHandle *		GisGetRHandle			(void);
void 				GisFreeRHandle			(void *ptr);
void 				GisFreeCHandle			(void *ptr);
ClientHandle *		GisAllocNativeCHandle	(void);
ClientHandle *		GisAllocCHandle			(ClientHandle *chandle, int type);
int 				GisDeleteCHandle		(ClientHandle *handle);
int 				GisDisableCHandle		(ClientHandle *handle);
int 				GisExamineList			(List rList, time_t tm);
int 				GisInitRemoteList		(void);
ClientHandle *		GisAddRemote			(List rList, int fd, struct sockaddr_in *client);
UCCClientHandle *	GisAllocUCCHandle		(void);
void 				GisFreeUCCHandle		(void *ptr);
int 				GisDisableUCCHandle		(UCCClientHandle *handle);
void 				GisFreeLRQHandle		(void *ptr);
int 				GisDisableLRQHandle		(LRQHandle *lrqHandle);
LRQHandle *			GisAllocLRQHandle		(void);
H323ClientHandle *	GisAllocNativeH323Handle(void);
H323ClientHandle *	GisAllocH323Handle		(H323ClientHandle *, int);
void 				GisFreeH323Handle		(void *ptr);
int 				GisDisableH323Handle	(H323ClientHandle *handle);
ResolveHandle *		GisAllocRHandle			(void);
void 				GisFreeRHandle			(void *ptr);
int 				GisSetupLRQFromResolve	(ResolveHandle *rhandle, LRQHandle *lrqHandle, int type);
CallHandle *		GisAllocCallHandle		(void);
void 				GisFreeCallHandle		(void *ptr);
void 				GisDisableCallHandle	(void *ptr);
ConfHandle *		GisAllocConfHandle		(void);
ClientHandle *		SipSearchClientListByCallId(List rList, char *callID);
int					HandleRemoteClient		(int fd, FD_MODE rw, void *data);
void				GisPurgeCHandle			(void *ptr);
#ifdef MEDIATION
SipCallHandle *	GisAllocSIPHandle		(void);
void				GisFreeSIPHandle		(void *ptr);
int					GisDisableSIPHandle		(SipCallHandle *handle);
int GkSendLRQ(PhoNode *rfphonodep, ARQHandle *arqHandle2, void *data);

#endif

int GisStartElem(void *userData, int tag, int depth, const char **atts);

int GisEndElem(void *userData, int tag, int depth);

int GisExecuteCmd(unsigned short index);

#if 0
/* GIS <-> CLI Q interface related functions */
int InitGisCliQIntf(MsgQ *q);

void* GisCliQReader(void *rock);

int GisCliQRegister(int mtype, ThreadDispatchEntry *dispatch);

int GisCliQRegister(int mtype, ThreadDispatchEntry *dispatch);

int GisCliQDeRegister(int mtype);

int GisCliQSendResp(RdWrThrdHandoff *ho);

void ShutGisCliQIntf(MsgQ *q);
#endif

#endif /* _gis_h_ */



