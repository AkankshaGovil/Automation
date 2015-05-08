#ifdef __cplusplus
extern "C" {
#endif



/*

 NOTICE:
 This document contains information that is proprietary to RADVISION LTD..
 No part of this publication may be reproduced in any form whatsoever without
 written prior approval by RADVISION LTD..

  RADVISION LTD. reserves the right to revise this publication and make changes
  without obligation to notify any person of such revisions or changes.

    */
#ifndef _CMCOTROL_
#define _CMCOTROL_

#include <cm.h>
#include <mti.h>
#include <cmCrossReference.h>
#include <cmictrl.h>


typedef enum {
    capReleased,
    capSent,
    capAcknowledged,
    capRejected
} capStatus;


/************************************************************************
 * outCapT struct
 * Outgoing capabilities negotiated for the call
 ************************************************************************/
typedef struct {
    char    pID[12];        /* Protocol Identifier of outgoing capability. This fields */
    int     pIDLen;         /* are automatically set by the CM to their default values. */
    BOOL    waiting;
    HTI     timer;          /* Timer used for negotiations of capabilities */
    BYTE    sq;
    int     termNodeId;
    int     rejectCause;    /* Reject cause if rejected */
} outCapT;

typedef struct {
    char    pID[12];        /* Protocol Identifier of incoming capability. */
    int     pIDLen;
    BYTE    sq;
    int     termNodeId;
    BOOL    manualResponse;
} inCapT;

typedef struct {
    BYTE    sq;
    HTI     timer;
} outRM_T;

typedef struct {
    BYTE    sq;
} inRM_T;

typedef enum {
    idle,
    incomingAwaitingManualAcknoledge,
    incomingAwaitingResponse,
    outgoingAwaitingResponse
} msdState;

typedef enum {
    indeterminate,
    master,
    slave
} msdStatus;


typedef struct {
    msdState   state;
    HTI        timer;
    msdStatus  status;
    int        myTerminalType;
    int        terminalType;
    int        myStatusDeterminationNumber;
    int        statusDeterminationNumber;
    int        count;
    BOOL       manualResponse;
} msdT;

typedef struct {
    HTI     timer;
    int     timeStart;
    int     sqNumber;
    BOOL    waiting;
} rtdT;



typedef enum
{
    released = -1,
    channelIdle,
    awaitingEstablishment,
    faststart,
    fsAwaitingEstablish,
    established,
    awaitingRelease,
    awaitingConfirmation

} chanState;


/************************************************************************
 * controlElem struct
 * This structure holds the H.245 control information necessary for
 * connecting a regular call with H245 channel.
 ************************************************************************/
typedef struct __controlElem {
    controlState    state;          /* Control channel's state */
    outCapT         outCap;         /* Outgoing capabilities */
    inCapT          inCap;          /* Incoming capabilities */
    msdT            msd;            /* MasterSlaveDetermination process structure */
    rtdT            rtd;
    outRM_T         out_RM;
    inRM_T          in_RM;
    int             mpMode;

    int             lcnOut;
    capStatus       inCapStatus;
    capStatus       outCapStatus;
    BOOL            isMasterSlave;  /* TRUE if at least 1 MSD procedure was finished */
    BOOL            isMaster;       /* TRUE if local endpoint is master in this call */
    BOOL            isDerived;

    BOOL            firstAudioIn;
    BOOL            firstVideoIn;
    BOOL            firstDataIn;
    BOOL            firstAudioOut;
    BOOL            firstVideoOut;
    BOOL            firstDataOut;
    int             conflictChannels;
    int             nextFreeSID;
    cmTerminalLabel myTermLabel;
    cmTransportAddress mcLocationTA;
    int             incomingOLCs[2]; /*node IDs of stored incoming OLCs that were received before control got connected */
} controlElem;

typedef struct channelS channelElem;
struct channelS
{
    int             fastStartChannelIndex; /* -1 for normal channels. For faststart channels,
                                              this is the index of the approval in callElem. */
    int             dynamicPayloadNumber;
    channelElem*    partner;                /* Opposite channel with the same session ID */
    channelElem*    base;
    channelElem*    associated;
    channelElem*    replacementCh;
    BOOL            isDuplex; /* by default it is simplex (uni-directional) channel. */
    int             sid;                    /* Session ID */
    BOOL            flowControlToZero;
    int             myLCN;
    int             rvrsLCN;
    HCONTROL        ctrl;
    BOOL            origin; /* TRUE if we're the origin of this channel, FALSE otherwise */
    int             port;
    int             remotePortNumber;

    /* RTP and RTCP addresses for the channel */
    int             recvRtpAddressID;
    int             recvRtcpAddressID;
    int             sendRtpAddressID;
    int             sendRtcpAddressID;

    int             redEncID;
    int             transCapID;
    chanState       state;                  /* The state of the channel */
    int             rc_paramID;
    HTI             timer;
    HTI             rc_timer;
    HTI             ml_timer;
    int             dataTypeID;
    int             separateStackID;
    int             externalReferenceLength;
    char            externalReference[256];
    BOOL            isAssociated;
    cmT120SetupProcedure
                t120SetupProcedure;
    cmTerminalLabel source;
    cmTerminalLabel destination;
};

#endif
#ifdef __cplusplus
}
#endif
