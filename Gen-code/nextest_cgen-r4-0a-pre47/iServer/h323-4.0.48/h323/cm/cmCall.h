#ifdef __cplusplus
extern "C" {
#endif


/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/


#ifndef _CMCALL_
#define _CMCALL_

#include <transport.h>
#include <mti.h>
#include <cat.h>


/* Maximum number of states in the call's state queue.
 * We do our best not to have a state-change of a call from within another state-change
 * callback to give the best backward-compatibility we can and to reduce call-stack nesting
 */
#define RV_H323CALL_STATE_MAX (5)

/* Dummy state definition, as held inside the queue */
#define RV_H323CALL_STATE_DUMMY (0xff)


/************************************************************************
 * cmCallParamEnum
 * Enumeration value of parameters as properties stored inside a call when
 * the property DB is not used as a single root with several messages
 * beneith it.
 * Each group of parameters that belong to the same type of message is
 * written one after the other, so we know when we're done with the
 * parameters of a specific message type.
 ************************************************************************/
typedef enum
{
    /* SETUP */
        cmEnumSetupSendingComplete   ,
        cmEnumInformationTransferCapability ,
        cmEnumDisplay                ,
        cmEnumCalledPartyNumber      ,
        cmEnumCalledPartySubAddress  ,
        cmEnumCallingPartyNumber     ,
        cmEnumCallingPartySubAddress ,
        cmEnumFullSourceAddress      ,
        cmEnumFullDestinationAddress ,
        cmEnumDestExtraCallInfo      ,
        cmEnumExtention              ,

        cmEnumFullSourceInfo         ,

        cmEnumDestCallSignalAddress  ,
        cmEnumSrcCallSignalAddress   ,
        cmEnumSetupH245Address       ,

        cmEnumActiveMc               ,
        cmEnumConferenceGoal         ,
        cmEnumCallType               ,

        cmEnumUserUser               ,

        cmEnumSetupNonStandardData   ,
        cmEnumSetupNonStandard       ,

        cmEnumSetupFastStart         ,
        cmEnumSetupCanOverlapSending ,

    /* CALL PROCEEDING */
        cmEnumCallProcH245Address    ,
        cmEnumCallProceedingNonStandardData,
        cmEnumCallProceedingNonStandard,

        cmEnumCallProcFastStart      ,

    /* ALERTING */
        cmEnumAlertingH245Address    ,
        cmEnumAlertingNonStandardData,
        cmEnumAlertingNonStandard    ,
        cmEnumAlertingFastStart      ,

    /* CONNECT */
        cmEnumH245Address            ,
        cmEnumConnectDisplay         ,
        cmEnumConnectUserUser        ,
        cmEnumConnectNonStandardData ,
        cmEnumConnectNonStandard     ,
        cmEnumConnectFastStart       ,
        cmEnumConnectConnectedAddress,

    /* RELEASE COMPLETE */
        cmEnumReleaseCompleteReason  ,
        cmEnumReleaseCompleteCause   ,
        cmEnumReleaseCompleteNonStandardData,
        cmEnumReleaseCompleteNonStandard,

    /* FACILITY */
        cmEnumAlternativeAddress     ,
        cmEnumAlternativeAliasAddress,
        cmEnumAlternativeDestExtraCallInfo
                                           ,
        cmEnumAlternativeExtention   ,
        cmEnumFacilityCID            ,
        cmEnumFacilityReason         ,

    /* LAST ITEM - used to calculate size on compilation */
        cmEnumLast
} cmCallParamEnum;



/* This list holds the positions of all the fields inside the flags field of a callElem. */
typedef enum
{
    e_remoteVersion = 0,
    e_newCIDRequired = e_remoteVersion+5,

    e_callInitiator,
    e_dummyRAS,
    e_enableOverlapSending,
    e_remoteCanOverlapSend,
    e_multiRate,
    e_overrideCID,
    e_gatekeeperRouted,

    e_notified,
    e_callWithoutQ931,
    e_control,
    e_h245Tunneling,
    e_notEstablishControl,
    e_isParallelTunneling,
    e_h245Stage,
    e_isMultiplexed = e_h245Stage+5,
    e_shutdownEmptyConnection,
    e_fastStartFinished,
    e_controlDisconnected
} callElemEnumPosition;

/* This list holds the mask to use for each of the flags. There are fields that are not
   boolean, whose flags have a bigger mask than those of the boolean fields. */
typedef enum
{
    s_remoteVersion=0x1f,
    s_newCIDRequired=1,
             
    s_callInitiator=1,
    s_dummyRAS=1,
    s_enableOverlapSending=1,
    s_remoteCanOverlapSend=1,
    s_multiRate=1,
    s_overrideCID=1,
    s_gatekeeperRouted=1,

    s_notified=1,
    s_callWithoutQ931=1,
    s_control=1,
    s_h245Tunneling=1,
    s_notEstablishControl=1,
    s_isParallelTunneling=1,
    s_h245Stage=0x1f,
    s_isMultiplexed=1,
    s_shutdownEmptyConnection=1,
    s_fastStartFinished=1,
    s_controlDisconnected=1
} callElemEnumSize;


/* field_mask(field) defines the mask to use on the flags to handle the given field
 * example:
 *  field_mask(h245Stage);
 * The above returns the mask for the h245Stage boolean field inside callInfo.flags
 */
#define field_mask(field) (s_##field<<(e_##field))

/* Set and get macros for fields inside callInfo.flags */
#define m_callset(call,field,value) \
    ((call)->flags = ( ((call)->flags  & ~(field_mask(field))) | (field_mask(field) & ((int)(value) << (e_##field))) ))
#define m_callget(call,field) \
    (((call)->flags  & (field_mask(field))) >> (e_##field))



/************************************************************************
 * callElem
 * Call element. Holds the information about the call.
 * Check cmCrossReference.c for details about the calls database.
 ************************************************************************/
typedef struct __callElem {
    UINT32                  flags;
#if 0    
        int                     remoteVersion;
        BOOL                    newCIDRequired;         /* TRUE if we need to create a new CID.
                                                           This happens when we're forwarding the call */
        BOOL                    callInitiator;          /* TRUE if this is the initiator of the call, FALSE otherwise */
        BOOL                    dummyRAS;               /* There is no RAS in this call */
        BOOL                    enableOverlapSending;
        BOOL                    remoteCanOverlapSend;
        BOOL                    multiRate;
        BOOL                    overrideCID;            /* Indication that the CID parameter should be overriden in outgoing messages
                                                           when sendCallMessage() is called. This parameter is false only when
                                                           cmCallJoin() is called. */
        BOOL                    gatekeeperRouted;       /* TRUE if callModel is gatekeeperRouted */

        BOOL                    notified;               /* TRUE if application was already notified about this new incoming call */
        BOOL                    callWithoutQ931;        /* TRUE if call started with incoming ARQ and no SETUP was received yet */
        BOOL                    control;                /* TRUE if there is an H245 TCP connection for this call */
        BOOL                    h245Tunneling;
        BOOL                    notEstablishControl;
        BOOL                    isParallelTunneling;
        cmH245Stage             h245Stage;
        BOOL                    isMultiplexed;
        BOOL                    shutdownEmptyConnection;
#endif
    HSTRANSSESSION          hsTransSession;
    cmConferenceGoalType    conferenceGoal;
    cmCallState_e           state;
    cmCallStateMode_e       stateMode;


    BYTE                    callId[16];             /* CallIdentifier of the call */
    BYTE                    cId[16];                /* ConferenceIdentifier of the call */

    HAPPCONN                appHandles[2];          /* Application handles for the Q931 and H245 connections */

    void*                   hMsgContext;            /* Message context for last message received on call. Used for security checks */
    int                     crv;                    /* Q931 CRV of the call */
    int                     rascrv;                 /* RAS CRV of the call */

    int                     rate;
    int                     newRate;

    HTI                     timer;
    int                     preGrantedArqUse;
    cmAnnexEUsageMode       annexE;
    RVHCATCALL              hCatCall;               /* CallAssociationTable handle for this call */
    int                     routeCallSignalAddress;
    int                     callSignalAddress;		/* The address of destination of the callSignaling connection */
													/* The remote for outgoing call */
													/* The local for outgoing call */
	int						remoteCallSignalAddress;/* The remote callSignaling address for the incoming call */
    int                     annexECallSignalAddress;
    int                     destinationInfo;


    /*Control*/
    int                     lcnOut; /* todo: I want another explanation of this mechanism (tsahi) */

    /* transport related parameters */

    /* fast start */
    int                     fastStartState;         /* State of fastStart procedure of this call (fssNo, fssRequest, fssAck) */
    int*                    fastStartNodes;
    int                     fastStartNodesCount;
    int*                    fastStartNodesAck;
    int                     fastStartNodesAckCount;
    BYTE*                   fastStartIndexes;

    /* parameters */
    int                     property;               /* PVT root of the property database for the call */
    int                     paramNodeId[cmEnumLast];/* Parameters that are used if we're running without a property DB */
    int                     lastSentMessageNodeId;  /* NodeID of the last send message. This parameter is used when we're
                                                       running without a property DB */
    /* state queue - this is a cyclic queue */
    BYTE                    q_nextState; /* Index of the next state we should handle */
    BYTE                    q_numStates; /* Number of state currently in the queue */
    BYTE                    q_states[RV_H323CALL_STATE_MAX];
    BYTE                    q_stateModes[RV_H323CALL_STATE_MAX];
} callElem;



/************************************************************************
 * callStartOK
 * purpose: Deal with an outgoing or incoming call in the Q931 side, after
 *          RAS was done (or skiped)
 * input  : call    - Stack handle for the call to dial
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callStartOK(IN callElem* call);


int callStartRoute(callElem*call);
int callStartError(callElem*call);
int callIncompleteAddress(callElem*call);

/************************************************************************
 * rasCallDrop
 * purpose: Call drop was initiated by an incoming DRQ message from the
 *          gatekeeper
 * input  : call      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasCallDrop(IN callElem* call);

/************************************************************************
 * callStopOK
 * purpose: We've finished dropping the call.
 *          Notify the application we've reached the Idle state on this call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callStopOK(IN callElem* call);

int callStopError(callElem*call);

/************************************************************************
 * callNewRate
 * purpose: Notify the applicatino about the rate of the call
 * input  : call      - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void callNewRate(IN callElem* call);

/************************************************************************
 * sendCallMessage
 * purpose: Send Q931 message for a given call.
 *          This function also adds specific call-related information
 * input  : hsCall  - Stack handle for the Q931 call
 *          message - root node ID of the message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendCallMessage(IN HCALL hsCall, IN int message);

/************************************************************************
 * sendMessage
 * purpose: Send TCP (Q931/H245) message for a given call.
 *          This function also adds specific call-related information
 * input  : hsCall  - Stack handle for the call
 *          vNodeId - root node ID of the message to send
 *          type    - Type of channel to send on
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendMessage(IN HCALL hsCall, IN int vNodeId, IN cmTpktChanHandleType type);

int cmEvRASNewCall(HAPP hApp, RVHCATCALL* lphCatCall, catStruct* callObj);

/************************************************************************
 * cmIndicate
 * purpose: Indicate and process an incoming Q931 message
 * input  : call    - Stack handle for the call
 *          message - Message root node
 *          msgType - Type of the message
 * output : none
 * return : Non-negative value on success
 *          negative value on failure
 ************************************************************************/
int cmIndicate(IN HCALL call, IN int message, IN int msgType);


/************************************************************************
 * callMibGetSession
 * purpose: Get the session of a handle for the MIB.
 *          This function is used to access the transport layer for MIB
 *          specific information
 * input  : call    - Stack handle for the call
 * output : none
 * return : Session of the call on success
 *          NULL on failure
 ************************************************************************/
HSTRANSSESSION callMibGetSession(IN HCALL call);


/**********************************************************************
* cmSetupEnd
* This function is responsible for the end of the scenatio of receiving 
* a setup message. It was taken out of the cmSetup in order to give the 
* endpoint time to receive ACF before sending AutoConnect (when autoRas
* and autoConnect are on).
************************************************************************/
int cmSetupEnd(IN callElem* call);

#endif
#ifdef __cplusplus
}
#endif

