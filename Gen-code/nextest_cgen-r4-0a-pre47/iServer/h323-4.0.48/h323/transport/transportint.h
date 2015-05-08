/******************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

******************************************************************************/

#ifndef TRANSPORTINT_H
#define TRANSPORTINT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <mti.h>

#include <mei.h>
#include <log.h>
#include <ema.h>
#include <hash.h>
#include <tpkt.h>
#include <annexe.h>
#include <pvaltree.h>
#include <rpool.h>
#include <cat.h>
#include <transport.h>


#define TPKT_HEADER_SIZE 4
#define MSG_HEADER_SIZE RV_ALIGN(4)
#define TRANS_RPOOL_CHUNK_SIZE 512

#define annexEHost 0x1
#define incomingHost 0x10
#define outgoingHost 0x20

/* Types of connection */
typedef enum {
    cmTransTPKTAnnexECompetition = 0,
    cmTransTPKTConn,
    cmTransAnnexEConn
} TRANSINTCONNTYPE;

typedef enum {
    hostIdle,
    hostListenning,
    hostConnecting,
    hostListenningConnecting,
    hostConnected,
    hostBusy,
    hostClosing,
    hostClosed
} HOSTSTATE;

typedef enum {
    tunnNo,
    tunnPossible,
    tunnOffered,
    tunnApproved,
    tunnRejected
}TUNNSTATE;

typedef enum {
    parllNo,
    parllOffered,
    parllApproved
}PARTUNNSTATE;

typedef enum {
    fsNo,
    fsOffered,
    fsApproved,
    fsRejected
}FSSTATE;


typedef struct
{
    UINT32  bufferSize;
    BYTE*   buffer;
} transThreadCoderLocalStorage;

typedef struct _cmTransSession cmTransSession;
typedef struct {
    TRANSCONNTYPE       type;
    BOOL                closeOnNoSessions;
    BOOL                isMultiplexed;
    BOOL                remoteCloseOnNoSessions;
    BOOL                remoteIsMultiplexed;
    BOOL                hostIsApproved;
    BOOL                incoming;
    BOOL                reported;
	BOOL				dummyHost;
    cmAnnexEUsageMode   annexEUsageMode;
    HOSTSTATE           state;
    cmTransportAddress  remoteAddress;
    cmTransportAddress  localAddress;
    HTPKT               hTpkt;
    HTPKT               h245Listen;
    void                *hashEntry;
    cmTransSession      *firstSession;
    void                *firstMessage;
    void                *lastMessage;
    void                *incomingMessage;
    HTI                connectTimer; /* Mark interval between event "accept" and message arrival */
} cmTransHost;

struct _cmTransSession {
    cmTransHost         *Q931Connection;
    cmTransHost         *annexEConnection;
    cmTransHost         *H245Connection;
    BOOL                isTunnelingSupported;
    BOOL                isParallelTunnelingSupported;
    BOOL                notEstablishControl;
    BOOL                closeOnNoSessions;
    BOOL                isMultiplexed;
    BOOL                firstMessageSent;
    BOOL                connectedToHost;
    BOOL                reportedH245Connect;
    BOOL                reportedQ931Connect;
    BOOL                openControlWasAsked;
    BOOL                switchToSeparateWasAsked;
    BOOL                outgoing;
    BOOL                holdTunneling;
    BOOL                forceTunneledMessage;
    BOOL                needToSendFacilityStartH245;
    BOOL                notFirstTunnelingMessage;
    BOOL                hostWasSet;
    TUNNSTATE           tunnelingState;
    PARTUNNSTATE        parallelTunnelingState;
    FSSTATE             faststartState;
    cmAnnexEUsageMode   annexEUsageMode;
    cmH245Stage         h245Stage;
    BYTE                callId[16];
    UINT16              CRV;
    void                *hashEntry;
    cmTransSession      *nextSession;
    cmTransSession      *prevSession;
    cmTransSession      *nextPending;
    cmTransSession      *prevPending;
    void                *firstMessage;
    void                *lastMessage;
    int                 h450Element;
    int                 annexMElement;
    int                 annexLElement;
	HTI					muxConnectTimer;
	HTI					h245ConnectTimer;
};

typedef struct {
    HAPPATRANS          hAppATrans;
    HMEI                lock;
    HMEI                tablesLock;
    RVHLOG              hLog;
    RVHLOG              hTPKTCHAN;
    HPVT                hPvt;
    HPST                synProtQ931;
    HPST                synProtH245;
    HPST                synProtH450;
    HRPOOL              messagesRPool;
    HEMA                hEmaSessions;
    HEMA                hEmaHosts;
    HHASH               hHashHosts;
    HHASH               hHashSessions;
    TRANSSESSIONEVENTS  sessionEventHandlers;
    TRANSHOSTEVENTS     hostEventHandlers;
    HTPKTCTRL           tpktCntrl;
    HANNEXE             annexECntrl;
    cmTransHost         *hTPKTListen;
    int                 codingBufferSize;
    cmTransSession      *pendingList;
    int                 localIPAddress;
    HSTIMER				hTimers;
	int					maxUsedNumOfMessagesInRpool;
	int				    curUsedNumOfMessagesInRpool;
    BOOL                annexESupported;

} cmTransGlobals;

#define sizeof_hostKey 12
typedef struct {
    UINT32          ip;
    UINT32          port;
    BOOL            type;
} hostKey;

#define sizeof_sessionKey 4
typedef struct {
    INT32       CRV;
} sessionKey;

#ifdef __cplusplus
}
#endif


#endif
