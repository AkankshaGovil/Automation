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

#ifndef CMCTRL_H
#define CMCTRL_H

#ifndef CM_H
#error cmctrl.h included improperly. Use #include <cm.h> instead.
#else

#ifndef _TRANSPCAP_
#include <transpcap.h>
#endif



DECLARE_OPAQUE(HAPPCHAN);
DECLARE_OPAQUE(HCHAN);


typedef
int
(*cmNewProtocolEH)(void);

/*
typedef
int
(*cmNewProtocolEH)(
        IN  HPdlAProtocol       haProtocolParent,
        IN  HPdlSProtocol       hsProtocol,
        OUT HPdlAProtocol*      lphaProtocol,
        IN  char*           protocolName,
        OUT newProtocolEH*      newProtocol,
        OUT newMessageEH*       newMessage,
                OUT     BOOL*                   selfDestructing
);
------------------------------------------------------------------------- */


typedef enum
{
    cmChannelStateDialtone,
    cmChannelStateRingBack,
    cmChannelStateConnected,
    cmChannelStateDisconnected,
    cmChannelStateIdle,
    cmChannelStateOffering
} cmChannelState_e;

typedef enum
{
    cmChannelStateModeOn,
    cmChannelStateModeOff,

    cmChannelStateModeDisconnectedLocal,
    cmChannelStateModeDisconnectedRemote,
    cmChannelStateModeDisconnectedMasterSlaveConflict,
    cmChannelStateModeDuplex,
/* marina */
    cmChannelStateModeDisconnectedReasonUnknown,
    cmChannelStateModeDisconnectedReasonReopen,
    cmChannelStateModeDisconnectedReasonReservationFailure

} cmChannelStateMode_e;



typedef enum {
  cmCapReceive=1,
  cmCapTransmit=2,
  cmCapReceiveAndTransmit=3
} cmCapDirection;


typedef enum {
  cmCapEmpty=0,
  cmCapAudio=1,
  cmCapVideo=2,
  cmCapData=3,
  cmCapNonStandard=4,
  cmCapUserInput=5,
  cmCapConference=6,
  cmCapH235=7,
  cmCapMaxPendingReplacementFor=8,
  cmCapGeneric=9,
  cmCapMultiplexedStream=10,
  cmCapAudioTelephonyEvent=11,
  cmCapAudioTone=12
} cmCapDataType;



typedef struct {
  char *name;
  INT32 capabilityId; /* capabilityTableEntryNumber */
  int capabilityHandle; /* capability item message tree (video/audio/data/nonStandard) */
  cmCapDirection direction;
  cmCapDataType type;
} cmCapStruct;


typedef enum {
  cmCapAccept=1,
  cmCapReject
} cmCapStatus;


typedef enum {
  cmMSSlave=1,
  cmMSMaster,
  cmMSError
} cmMSStatus;



typedef enum {
  cmRequestCloseRequest, /* Received request to close channel */
  cmRequestCloseConfirm, /* Request to close this channel is confirmed */
  cmRequestCloseReject   /* Request to close this channel is rejected */
} cmRequestCloseStatus;


typedef enum {
  cmMediaLoopRequest,    /* Received request for a media loop on a channel */
  cmMediaLoopConfirm,    /* Media loop confirmed */
  cmMediaLoopOff         /* Media loop on channel is canceled */
} cmMediaLoopStatus;



typedef struct {
  char *data; /* general string formatted data (4 bytes/char) */
  int length; /* in bytes */
} cmUserInputData;


typedef enum {
  /* path in paranthesis indicates the nodeId syntax */
  cmReqModeRequest,    /* request mode (requestMode.requestedModes) */
  cmReqModeAccept,     /* request acknoledged (requestModeAck.response) */
  cmReqModeReject,     /* request rejected (requestModeReject.cause) */
  cmReqModeReleaseOut, /* Outgoing request released due to timeout (N/A) */
  cmReqModeReleaseIn   /* Incoming request released due to timeout (N/A) */
} cmReqModeStatus;


typedef enum {
  cmMiscMpOn,  /* multipoint mode command is on (set) */
  cmMiscMpOff  /* multipoint mode command is off (cancel) */
} cmMiscStatus;


typedef struct {
  char name[32];
  INT32 entryId; /* ModeElement node id */
} cmReqModeEntry;

typedef struct {
  UINT8 mcuNumber; /* constraint: 0..192 */
  UINT8 terminalNumber; /* constraint: 0..192 */
} cmTerminalLabel;


typedef enum {
  cmControlStateConnected,
  cmControlStateConference,
  cmControlStateTransportConnected,
  cmControlStateTransportDisconnected,
/* marina */
  cmControlStateFastStart
} cmControlState;


typedef enum {
  cmControlStateModeNull
} cmControlStateMode;

/* marina */
typedef enum
{
  cmIssueJoin   =0,
  cmIssueQueryAndJoin,
  cmIssueCreate ,
  cmIssueInvite ,
  cmWaitForInvite,
  cmOriginateCall,
  cmWaitForCall,
  cmIssueQuery
} cmT120SetupProcedure;

typedef enum
{
  cmCloseReasonUnknown=0,
  cmCloseReasonReopen   ,
  cmCloseReasonReservationFailure,
  cmCloseReasonNormal /* for request close only */
}cmCloseLcnReason;

typedef struct
{
  int redundancyEncodingMethodId;
  int secondaryEncodingId;
}cmRedundancyEncoding;

typedef enum
{
    cmVideoFreezePicture=0,
    cmVideoSendSyncEveryGOB,
    cmVideoSendSyncEveryGOBCancel,
    cmSwitchReceiveMediaOff,
    cmSwitchReceiveMediaOn,
    cmVideoFastUpdatePicture
}
cmMiscellaneousCommand;


#ifndef _FAST_START_H
#include <faststart.h>
#endif
#ifndef _REDENCOD_H
#include <redencod.h>
#endif
#ifndef _USER_INPUT_H
#include <userinput.h>
#endif

/* Callback function prototypes____________________________________________________________________________*/






/* --- Control session callback functions --- */

typedef int
    (RVCALLCONV *cmEvCallCapabilitiesT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      cmCapStruct*        capabilities[]);

typedef int
    (RVCALLCONV *cmEvCallCapabilitiesExtT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      cmCapStruct***      capabilities[]);

typedef int
    (RVCALLCONV *cmEvCallNewChannelT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      HCHAN               hsChan,
        OUT     LPHAPPCHAN          lphaChan);

typedef int
    (RVCALLCONV *cmEvCallCapabilitiesResponseT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              status);

typedef int
    (RVCALLCONV *cmEvCallMasterSlaveStatusT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              status);

typedef int
    (RVCALLCONV *cmEvCallRoundTripDelayT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      INT32               delay); /* positive: msec delay, negative: expiration */

typedef int
    (RVCALLCONV *cmEvCallUserInputT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      INT32               userInputId /* user input node id */
        );

typedef int
    (RVCALLCONV *cmEvCallRequestModeT)(
         /* Request to transmit the following modes preferences */
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      cmReqModeStatus     status,
        IN      INT32               nodeId /* pvt nodeId handle according to status */
        );


typedef int
    (RVCALLCONV *cmEvCallMiscStatusT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      cmMiscStatus        status);


typedef int
    (RVCALLCONV *cmEvCallControlStateChangedT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              state,
        IN      UINT32              stateMode);


/* marina */

typedef int
    (RVCALLCONV *cmEvCallMasterSlaveT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              terminalType,
        IN      UINT32              statusDeterminationNumber);







RVAPI int RVCALLCONV
cmCallMasterSlaveDetermineExt(
               /*  Start master slave determination request ,
          Acknowledge to master slave determination request
          set terminalType and determinationNumber .
          */
               IN   HCALL       hsCall,
               IN   int         terminalType,
               IN   int         determinationNumber);

typedef  struct
{
  cmEvCallCapabilitiesT           cmEvCallCapabilities;
  cmEvCallCapabilitiesExtT        cmEvCallCapabilitiesExt;
  cmEvCallNewChannelT             cmEvCallNewChannel;
  cmEvCallCapabilitiesResponseT   cmEvCallCapabilitiesResponse;
  cmEvCallMasterSlaveStatusT      cmEvCallMasterSlaveStatus;
  cmEvCallRoundTripDelayT         cmEvCallRoundTripDelay;
  cmEvCallUserInputT              cmEvCallUserInput;
  cmEvCallRequestModeT            cmEvCallRequestMode;
  cmEvCallMiscStatusT             cmEvCallMiscStatus;
  cmEvCallControlStateChangedT    cmEvCallControlStateChanged;
/* marina */
  cmEvCallMasterSlaveT            cmEvCallMasterSlave;
} SCMCONTROLEVENT,*CMCONTROLEVENT;

typedef SCMCONTROLEVENT SCMSESSIONEVENT; /* for backward compatability */
typedef CMCONTROLEVENT CMSESSIONEVENT;




/* --- Control channels callback functions --- */


typedef int
    (RVCALLCONV *cmEvChannelStateChangedT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      UINT32              state,
        IN      UINT32              stateMode);

typedef int
    (RVCALLCONV *cmEvChannelNewRateT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      UINT32              rate);

typedef int
    (RVCALLCONV *cmEvChannelMaxSkewT)(
        IN      HAPPCHAN            haChan1,
        IN      HCHAN               hsChan1,
        IN      HAPPCHAN            haChan2,
        IN      HCHAN               hsChan2,
        IN      UINT32              skew);

typedef int
    (RVCALLCONV *cmEvChannelSetAddressT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      UINT32              ip,
        IN      UINT16              port);

typedef int
    (RVCALLCONV *cmEvChannelSetRTCPAddressT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      UINT32              ip,
        IN      UINT16              port);

typedef int
    (RVCALLCONV *cmEvChannelParametersT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      char*               channelName,
        IN      HAPPCHAN            haChanSameSession,
        IN      HCHAN               hsChanSameSession,
        IN      HAPPCHAN            haChanAssociated,
        IN      HCHAN               hsChanAssociated,
        IN      UINT32              rate);

typedef int
    (RVCALLCONV *cmEvChannelRTPDynamicPayloadTypeT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      INT8                dynamicPayloadType);

typedef int
    (RVCALLCONV *cmEvChannelVideoFastUpdatePictureT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan);

typedef int
    (RVCALLCONV *cmEvChannelVideoFastUpdateGOBT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 firstGOB,
        IN      int                 numberOfGOBs);

typedef int
    (RVCALLCONV *cmEvChannelVideoFastUpdateMBT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 firstGOB,
        IN      int                 firstMB,
        IN      int                 numberOfMBs);

typedef int
    (RVCALLCONV *cmEvChannelHandleT)(
          /* provide data type tree handle for this channel */
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 dataTypeHandle,
        IN      cmCapDataType       dataType);


typedef int
    (RVCALLCONV *cmEvChannelGetRTCPAddressT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      UINT32*             ip,
        IN      UINT16*             port);

typedef int
    (RVCALLCONV *cmEvChannelRequestCloseStatusT)(
        IN      HAPPCHAN              haChan,
        IN      HCHAN                 hsChan,
        IN      cmRequestCloseStatus  status);

typedef int
    (RVCALLCONV *cmEvChannelTSTOT)(
        IN      HAPPCHAN              haChan,
        IN      HCHAN                 hsChan,
        IN      INT8                  isCommand, /* 0: indication, 1: command */
        IN      INT8                  tradeoffValue); /* 0-31 trade off value */

typedef int
    (RVCALLCONV *cmEvChannelMediaLoopStatusT)(
        IN      HAPPCHAN              haChan,
        IN      HCHAN                 hsChan,
        IN      cmMediaLoopStatus     status);



typedef int
    (RVCALLCONV * cmEvChannelReplaceT)(
         IN     HAPPCHAN              haChan,
         IN     HCHAN                 hsChan,
         IN     HAPPCHAN              haReplacedChannel,
         IN     HCHAN                 hsReplacedChannel);


typedef int
    (RVCALLCONV * cmEvChannelFlowControlToZeroT)(
         IN      HAPPCHAN             haChan,
         IN      HCHAN                hsChan);

typedef int
    (RVCALLCONV * cmEvChannelMiscCommandT)(
         IN      HAPPCHAN             haChan,
         IN      HCHAN                hsChan,
         IN      cmMiscellaneousCommand miscCommand);

typedef int
    (RVCALLCONV * cmEvChannelTransportCapIndT)(
         IN      HAPPCHAN             haChan,
         IN      HCHAN                hsChan,
         IN      int nodeId
                     );


typedef int
    (RVCALLCONV *cmEvChannelSetNSAPAddressT)(
        IN      HAPPCHAN              haChan,
        IN      HCHAN                 hsChan,
        IN      char*                 address,
        IN      int                   length,
        IN      BOOL                  multicast);


typedef int
    (RVCALLCONV *cmEvChannelSetATMVCT)(
        IN      HAPPCHAN              haChan,
        IN      HCHAN                 hsChan,
        IN      int                   portNumber);


/******************************************************************************
 * cmEvChannelRecvMessageT
 * ----------------------------------------------------------------------------
 * General: Notification function for the application that an incoming H.245
 *          message related to a specific channel was received by the stack and
 *          is about to be processed.
 * Remarks: This callback will be invoked after the callback indicating a new
 *          message for a call is invoked (cmEvCallRecvMessage in H.323 or
 *          Rv3G324mCallReceiveMessageEv in 3G-324M).
 *          This callback will not be invoked for Fast start messages in H.323.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  haChan   - Application's handle for the channel.
 *         hsChan   - Stack's handle for the channel.
 *         message  - PVT node ID of the message that was received.
 * Output: None.
 *****************************************************************************/
typedef int
    (RVCALLCONV*cmEvChannelRecvMessageT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 message);


/******************************************************************************
 * cmEvChannelSendMessageT
 * ----------------------------------------------------------------------------
 * General: Notification function for the application that an outgoing H.245
 *          message related to a specific channel is about to be sent by the
 *          stack and.
 * Remarks: This callback will be invoked before the callback indicating
 *          sending a message for a call is invoked (cmEvCallSendMessage in
 *          H.323 or Rv3G324mCallSendMessageEv in 3G-324M).
 *          This callback will not be invoked for Fast start messages in H.323.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  haChan   - Application's handle for the channel.
 *         hsChan   - Stack's handle for the channel.
 *         message  - PVT node ID of the message that is about to be sent.
 * Output: None.
 *****************************************************************************/
typedef int
    (RVCALLCONV*cmEvChannelSendMessageT)(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 message);



typedef struct
{
  cmEvChannelStateChangedT            cmEvChannelStateChanged;
  cmEvChannelNewRateT                 cmEvChannelNewRate;
  cmEvChannelMaxSkewT                 cmEvChannelMaxSkew;
  cmEvChannelSetAddressT              cmEvChannelSetAddress;
  cmEvChannelSetRTCPAddressT          cmEvChannelSetRTCPAddress;
  cmEvChannelParametersT              cmEvChannelParameters;
  cmEvChannelRTPDynamicPayloadTypeT   cmEvChannelRTPDynamicPayloadType;
  cmEvChannelVideoFastUpdatePictureT  cmEvChannelVideoFastUpdatePicture;
  cmEvChannelVideoFastUpdateGOBT      cmEvChannelVideoFastUpdateGOB;
  cmEvChannelVideoFastUpdateMBT       cmEvChannelVideoFastUpdateMB;
  cmEvChannelHandleT                  cmEvChannelHandle;
  cmEvChannelGetRTCPAddressT          cmEvChannelGetRTCPAddress;
  cmEvChannelRequestCloseStatusT      cmEvChannelRequestCloseStatus;
  cmEvChannelTSTOT                    cmEvChannelTSTO;
  cmEvChannelMediaLoopStatusT         cmEvChannelMediaLoopStatus;
  cmEvChannelReplaceT                 cmEvChannelReplace;
  cmEvChannelFlowControlToZeroT       cmEvChannelFlowControlToZero;
  cmEvChannelMiscCommandT             cmEvChannelMiscCommand;
  cmEvChannelTransportCapIndT         cmEvChannelTransportCapInd;
  cmEvChannelSetNSAPAddressT          cmEvChannelSetNSAPAddress;
  cmEvChannelSetATMVCT                cmEvChannelSetATMVC;
  cmEvChannelRecvMessageT             cmEvChannelRecvMessage;
  cmEvChannelSendMessageT             cmEvChannelSendMessage;
} SCMCHANEVENT,*CMCHANEVENT;









/* General_______________________________________________________________________________________________*/






RVAPI int RVCALLCONV /* Real number of channels in configuration or RVERROR */
cmGetConfigChannels(
            /* build array containing the channel names as appear in
               the configuration. The strings are copied into array elements */
            IN  HAPP        hApp,
            IN  int         arraySize,
            IN  int         elementLength, /* sizeof each string in array */
            OUT char*       array[] /* allocated with elements */
            );

/************************************************************************
 * cmSetControlEventHandler
 * purpose: Used only for backward compatibility. Also known as cmSetSessionEventHandler.
 *          Installs the control event handler
 * input  : hApp            - Stack handle for the application
 *          cmControlEvent  - Pointer to array of pointers to general event callback functions
 *          size            - Size of *cmControlEvent in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmSetControlEventHandler(
        IN      HAPP                hApp,
        IN      CMCONTROLEVENT      cmControlEvent,
        IN      int                 size);

RVAPI
int RVCALLCONV cmGetControlEventHandler(
        IN      HAPP                hApp,
        OUT     CMCONTROLEVENT      cmControlEvent,
        IN      int                 size);

#define cmSetSessionEventHandler cmSetControlEventHandler /* for backward compatability */


/************************************************************************
 * cmSetChannelEventHandler
 * purpose: Installs the channel event handler
 * input  : hApp            - Stack handle for the application
 *          cmChannelEvent  - Pointer to array of pointers to general event callback functions
 *          size            - Size of *cmChannelEvent in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmSetChannelEventHandler(
        IN      HAPP                hApp,
        IN      CMCHANEVENT         cmChannelEvent,
        IN      int                 size);


RVAPI
int RVCALLCONV cmGetChannelEventHandler(
        IN      HAPP                hApp,
        OUT      CMCHANEVENT         cmChannelEvent,
        IN      int                 size);



/* returns the value-tree node id of h245 configuration root node */
RVAPI
INT32 RVCALLCONV cmGetH245ConfigurationHandle(
                IN  HAPP             hApp);


/* Control Session API_______________________________________________________________________________*/





RVAPI
int RVCALLCONV cmCallCreateControlSession(
        IN   HCALL           hsCall,
        IN OUT   cmTransportAddress* addr);



RVAPI int RVCALLCONV
cmCallCloseControlSession(
              /* Close the H.245 PDL protocol tree */
              IN     HCALL               hsCall);


RVAPI int RVCALLCONV
cmCallHasControlSession(
              /* Checks for the H.245 PDL protocol tree */
              IN     HCALL               hsCall);

/* --- master/slave --- */


RVAPI
int RVCALLCONV cmCallMasterSlaveDetermine(
                    /* Start master slave determination request */
                    IN  HCALL       hsCall,
                    IN      int             terminalType
                    );
#define cmMasterSlaveDetermine cmCallMasterSlaveDetermine


RVAPI int RVCALLCONV /* master/slave status of this call */
cmCallMasterSlaveStatus(
            IN  HCALL       hsCall
            );








/* --- capabilities --- */

RVAPI int RVCALLCONV /* remote terminal capability set node id, or RVERROR */
cmCallGetRemoteCapabilities(
                /* Get latest remote terminal capability message */
                IN  HCALL       hsCall
                );

RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmCallSendCapability(
             /* Send terminal capability set tree to remote terminal */
             /* Note: cap tree is not deleted! */
             IN  HCALL hsCall,
             IN  int termCapSet  /* local terminal capabiliy set Id */
             );


RVAPI int RVCALLCONV /* new terminalCapabilitySet node id. */
cmCallCapabilitiesBuild(
            /* Build terminalCapabilitySet value tree including the capability set
               and capability descriptors. */
            IN  HCALL           hsCall,
            IN      cmCapStruct*        capSet[],
            IN      cmCapStruct***      capDesc[]
            );

RVAPI int RVCALLCONV
cmCallCapabilitiesSend(
               IN      HCALL               hsCall,
               IN      cmCapStruct*        capSet[],
               IN      cmCapStruct***      capDesc[]
               );

RVAPI int RVCALLCONV
cmCallGetLocalCapabilities(
               /* translate local caps (from CESE_OUT machine) to application structures.
                  Usage:
                   cmCapStruct capA[capSetASize];
                   cmCapStruct *capSetA[capSetASize];
                   void* capDescA[capDescASize];
                   cmCapStruct** capSet;
                   cmCapStruct**** capDesc;
                   int i;

                   for (i=0; i<capSetASize; i++) capSetA[i] = capA+i;
                   cmCallGetLocalCapabilities(hsCall,
                                              capSetA, capSetSize,
                                              capDescA, capDescASize,
                              &capSet, &capDesc);
                              */

               IN      HCALL        hsCall,
               IN      cmCapStruct*         capSetA[], /* user allocated */
               IN      int                  capSetASize,
               IN      void*                capDescA[], /* user allocated */
               IN      int                  capDescASize,
               OUT     cmCapStruct**        capSet[],
               OUT     cmCapStruct****      capDesc[]
               );





RVAPI int RVCALLCONV
cmCallCapabilitiesAck(
               IN HCALL hsCall);


/* --- alive --- */

RVAPI
int RVCALLCONV cmCallRoundTripDelay(
                  /* Measure the round trip delay to the remote terminal */
                  /* Note: maxDelay=0 --> delay taken from configuration */
                  IN    HCALL       hsCall,
                  IN    INT32           maxDelay /* maximum waiting delay in seconds */
                  );



/* --- user input --- */


RVAPI int RVCALLCONV
cmCallSendUserInput(
            /* send user input msg: userInputId tree is deleted */
            /* Note: Select one of nonStandard or userData options */
            IN  HCALL hsCall,
            IN  INT32 userInputId /* indication.userInput tree node id */
            );

RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputBuildNonStandard(
                /* Build userUser message with non-standard data */
                IN  HAPP hApp,
                IN  cmNonStandardIdentifier *identifier,
                IN  char *data,
                IN  int dataLength /* in bytes */
                );

RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputBuildAlphanumeric(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 IN  cmUserInputData *userData
                 );


RVAPI int RVCALLCONV  /* TRUE or RVERROR */
cmUserInputGet(
           /* Note: One of dataLength or userData->length is NULL (the other exists) */
           /* Note: pointers are set to addresses in the user input value tree */
           IN  HAPP hApp,
           IN  INT32 userInputId,

           /* 1) nonStandard */
           IN  cmNonStandardIdentifier *identifier, /* user allocated */
           IN  char *data, /* user allocated */
           INOUT INT32 *dataLength,  /* (in bytes) IN: data allocation. OUT: correct size */

           /* 2) userData */
           IN  cmUserInputData *userData /*.length: IN: data allocation. OUT: correct size */
           );


/* --- request mode --- */

RVAPI int RVCALLCONV
cmCallRequestMode(
          /* send request mode msg */
          /* Note: id is deleted when function returns */
          IN  HCALL hsCall,
          IN  INT32 modeDescriptorsId /* (requestMode.requestedModes) */
          );

RVAPI int RVCALLCONV
cmCallRequestModeAck(
             /* Acknowledge the request */
             IN  HCALL hsCall,
             IN  char* responseName /* (requestModeAck.response) */
             );

RVAPI int RVCALLCONV
cmCallRequestModeReject(
            /* Reject the request */
            IN  HCALL hsCall,
            IN  char* causeName /* requestModeReject.cause */
            );



RVAPI int RVCALLCONV/* request mode node id or RVERROR */
cmRequestModeBuild(
           /* Build request mode msg */
           /* Note: entryId overrides name */
           IN  HAPP hApp,
           IN  cmReqModeEntry **modes[] /* modes matrix in preference order. NULL terminated arrays */
           );

RVAPI int RVCALLCONV/* TRUE or RVERROR */
cmRequestModeStructBuild(
             /* Build request mode matrix structure from msg */
             /* Note: entry name as in h.245 standard */
             IN  HAPP hApp,
             IN  INT32 descId, /* requestMode.requestedModes node id */
             IN  cmReqModeEntry **entries, /* user allocated entries */
             IN  int entriesSize, /* number of entries */
             IN  void **ptrs, /* pool of pointers to construct modes */
             IN  int ptrsSize, /* number of ptrs */

             /* modes matrix in preference order. NULL terminated arrays */
             OUT cmReqModeEntry ***modes[]
             );




/* Channels____________________________________________________________________________________*/





RVAPI int RVCALLCONV
cmChannelGetCallHandles(
            /* Get the stack and application call handles by the channel handle */
            IN  HCHAN hsChan,
            OUT HCALL *hsCall,
            OUT HAPPCALL *haCall
            );




RVAPI int RVCALLCONV
cmChannelNew(
         IN      HCALL               hsCall,
         IN      HAPPCHAN            haChan,
         OUT     LPHCHAN             lphsChan);

RVAPI int RVCALLCONV
cmChannelSetHandle(
           /* let haChan be the new application handle of this channel */
           IN HCHAN                    hsChan,
           IN HAPPCHAN                 haChan);



RVAPI
int RVCALLCONV cmChannelOpen(
        IN      HCHAN               hsChan,
        IN      char*               channelName,
        IN      HCHAN               hsChanSameSession,
        IN      HCHAN               hsChanAssociated,
        IN      UINT32              rate);

RVAPI
int RVCALLCONV cmChannelOpenDynamic(
        IN      HCHAN               hsChan,
        IN      int                 dataTypeHandle, /* data type (HPVT) tree handle */
        IN      HCHAN               hsChanSameSession,
        IN      HCHAN               hsChanAssociated,
        IN      BOOL                isDynamicPaloadType);


RVAPI
int RVCALLCONV cmChannelOn(
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelOff(
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelAnswer(
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelDrop(
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelClose(
         IN     HCHAN               hsChan);


RVAPI int RVCALLCONV
cmChannelRequestCloseReject(
                /* Reject the request for closing the outgoing channel. */
                IN  HCHAN       hsChan
                );


/* --- Duplex channels --- */
RVAPI
int RVCALLCONV cmChannelDuplex(
                 /* declare channel as duplex. Meaning full duplex. Composed of
                two uni-directional channels in opposite directions. */
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelIsDuplex(
                   /* Returns TRUE if channel is duplex. FALSE if not. and negative value
                  in case of error */
         IN     HCHAN               hsChan);



RVAPI
int RVCALLCONV cmChannelSetDuplexAddress(
                       /* Set address of duplex connection */
         IN     HCHAN               hsChan,
         IN     cmTransportAddress  address,
         IN     int                 externalReferenceLength,
         IN     char*               externalReference, /* NULL if not exists */
         IN     BOOL                isAssociated /* TRUE if associated */
         );

RVAPI
int RVCALLCONV /* actual size of external reference or RVERROR */
cmChannelGetDuplexAddress(
              /* Get address of duplex connection */
         IN     HCHAN               hsChan,
         OUT    cmTransportAddress* address, /* User allocated structure */
         IN     int                 externalReferenceLength, /* size of extRef buffer */
         OUT    char*               externalReference, /* User allocated buffer */
         OUT    BOOL*               isAssociated /* TRUE if associated */
         );








/* Channel commands_____________________________________________________________________________________*/








RVAPI
int RVCALLCONV cmChannelMaxSkew(
        IN      HCHAN               hsChan1,
        IN      HCHAN               hsChan2,
        IN      UINT32              skew);

RVAPI
int RVCALLCONV cmChannelFlowControl(
        IN      HCHAN               hsChan,
        IN      UINT32              rate);

RVAPI
int RVCALLCONV cmChannelSetAddress(
        IN      HCHAN               hsChan,
        IN      UINT32              ip,
        IN      UINT16              port);

RVAPI
int RVCALLCONV cmChannelSetRTCPAddress(
        IN      HCHAN               hsChan,
        IN      UINT32              ip,
        IN      UINT16              port);

RVAPI
int RVCALLCONV cmChannelGetOrigin(
        IN      HCHAN               hsChan,
        OUT     BOOL*               origin);

RVAPI
int RVCALLCONV cmChannelSetDynamicRTPPayloadType(
        IN      HCHAN           hsChan,
        IN      INT8            dynamicPayloadType);

RVAPI
int RVCALLCONV cmChannelVideoFastUpdatePicture(
        IN      HCHAN               hsChan);

RVAPI
int RVCALLCONV cmChannelVideoFastUpdateGOB(
        IN      HCHAN               hsChan,
        IN      int                 firstGOB,
        IN      int                 numberOfGOBs);

RVAPI
int RVCALLCONV cmChannelVideoFastUpdateMB(
        IN      HCHAN               hsChan,
        IN      int                 firstGOB,
        IN      int                 firstMB,
        IN      int                 numberOfMBs);



RVAPI int RVCALLCONV
cmChannelTSTOCommand(
             /* Send temporal spatial trade off command. Request the remote terminal to change
              the tradeoff. */
             IN HCHAN hsChan, /* incoming channel */
             IN int tradeoffValue /* 0-31 */
             );

RVAPI int RVCALLCONV
cmChannelTSTOIndication(
             /* Send temporal spatial trade off indication. Indicates the current tradeoff value
              of the local terminal. */
             IN HCHAN hsChan, /* outgoing channel */
             IN int tradeoffValue /* 0-31 */
             );



/* Media loop____________________________________________________________________________*/

RVAPI int RVCALLCONV
cmChannelMediaLoopRequest(
              /* Request media loop on this channel */
              IN HCHAN hsChan /* outgoing channel */
              );

RVAPI int RVCALLCONV
cmChannelMediaLoopConfirm(
              /* Confirm media loop request on this channel */
              IN HCHAN hsChan /* incoming channel */
              );

RVAPI int RVCALLCONV
cmChannelMediaLoopReject(
             /* Reject media loop request on this channel */
             IN HCHAN hsChan /* incoming channel */
             );

RVAPI int RVCALLCONV
cmCallMediaLoopOff(
           /* Release all media loops in this call */
           IN HCALL hsCall
           );




RVAPI
int RVCALLCONV cmChannelSameSession(
             /* get the same session opposite channel of hsChan */
             IN     HCHAN           hsChan,
             OUT    HAPPCHAN*       haSameSession,
             OUT    HCHAN*          hsSameSession
             );

RVAPI int RVCALLCONV /* returns the session id for this channel */
cmChannelSessionId(
           /* get the session id of channel */
           IN   HCHAN           hsChan
           );



/* replace exist channel with new */
RVAPI int RVCALLCONV
cmChannelReplace(
                     IN      HCHAN          hsChan, /* openning outgoing channel,that proposed to replaced exist channel*/
                     IN      HCHAN          hsChanReplace);/* exist outgoing channel,that must be replaced */



/* check if channel is replacing  incoming exist channel
return TRUE if  it is and return in haReplaceChannel and hsReplaceChannel
handles of replaced  channel.
*/

RVAPI int RVCALLCONV
cmIsChannelReplace(
             IN     HCHAN           hsChan,
             OUT    HAPPCHAN*       haReplaceChannel,
             OUT    HCHAN*          hsReplaceChannel
             );


/* set establishing outgoing channel hsChan dependent on base channel with handle hsChanBase*/
RVAPI int RVCALLCONV
cmChannelSetDependency(
             IN      HCHAN               hsChan,
         IN      HCHAN               hsChanBase     );


/* check if channel is dependent on another channel
return TRUE if  it is and return in haBaseChannel and hsBaseChannel
handles of base channel.
*/
RVAPI int RVCALLCONV
cmChannelGetDependency(
                     IN     HCHAN           hsChan,
                     OUT    HAPPCHAN*       haBaseChannel,
                     OUT    HCHAN*          hsBaseChannel);



RVAPI int RVCALLCONV
cmChannelSetT120Setup(
             IN      HCHAN               hsChan,
         IN      cmT120SetupProcedure t120SetupProc);

RVAPI int RVCALLCONV
cmChannelGetT120Setup(
             IN      HCHAN               hsChan,
         OUT      cmT120SetupProcedure *t120SetupProc);


/* set FlowControlToZero on incoming channel during establising process
  used before answering */

RVAPI int RVCALLCONV
cmChannelSetFlowControlToZero(
             IN      HCHAN               hsChan,
         IN      BOOL   flowControl);



RVAPI int RVCALLCONV
cmChannelSetTransportCapability(
             IN      HCHAN               hsChan,
         int transportCapId    );

RVAPI int RVCALLCONV
cmChannelGetTransportCapabilityId(
             IN      HCHAN               hsChan);

RVAPI int RVCALLCONV
cmChannelSetRedundancyEncoding(
             IN      HCHAN               hsChan,
         IN     cmRedundancyEncoding * redundancyEncoding);

RVAPI int RVCALLCONV
cmChannelGetRedundancyEncoding(
             IN      HCHAN               hsChan,
         OUT     cmRedundancyEncoding * redundancyEncoding);


RVAPI int RVCALLCONV
cmChannelSetSource(
             IN      HCHAN               hsChan,
         IN     cmTerminalLabel *terminalLabel);

RVAPI int RVCALLCONV
cmChannelGetSource(
             IN      HCHAN               hsChan,
         OUT     cmTerminalLabel *terminalLabel);


RVAPI int RVCALLCONV
cmChannelSetDestination(
         IN      HCHAN               hsChan,
         IN     cmTerminalLabel *terminalLabel);


RVAPI int RVCALLCONV
cmChannelGetSource(
         IN      HCHAN               hsChan,
         OUT     cmTerminalLabel *terminalLabel);

/* -------------------- close logical channels ----------------- */

RVAPI int RVCALLCONV
cmChannelDropReason(
          /*
        - Close outgoing channel
        - Reject incoming channel open request
        - New: Request to close incoming channel
        - On the other side the close lcn reason is passed via rejectStateMode to the
          cmEvChannelStateChanged callback function.
          */
            IN  HCHAN       hsChan,
            IN  cmCloseLcnReason reason
          );


RVAPI int RVCALLCONV
cmChannelRequestClose(
          /*
        - New: Request to close incoming channel
          */
          IN    HCHAN       hsChan,
        IN  cmCloseLcnReason reason,
        cmQosCapability * cmQOSCapability
          );

/* get on the outgoing side request to close channel ,reason and Qos capability of remoteendpoint */
RVAPI int RVCALLCONV
cmChannelGetRequestCloseParam(  IN  HCHAN       hsChan,
                                OUT  cmCloseLcnReason *reason,
                                OUT cmQosCapability * cmQOSCapability
                               );

/* Multipoint operations_______________________________________________________________________________*/




RVAPI int RVCALLCONV
cmCallStartConference(
              /* enter call conference mode */
              IN      HCALL               hsCall
              );

RVAPI int RVCALLCONV
cmCallCancelConference(
               /* cancel the call conference mode */
               IN      HCALL               hsCall
               );



RVAPI int RVCALLCONV
cmCallSetTerminalLabel(
               /* Set the remote endpoint terminal label. */
               IN      HCALL               hsCall,
               IN      cmTerminalLabel*    terminalLabel
               );

RVAPI int RVCALLCONV /* RVERROR if terminal label is not defined for this terminal */
cmCallGetTerminalLabel(
               /* Get the local endpoint terminal label. */
               IN      HCALL               hsCall,
               OUT     cmTerminalLabel*    terminalLabel /* user allocated */
               );


/* When the conference becomes active, the master (MC) terminal shall call cmCallDeclareMC() to
   indicate the new conference status. */
RVAPI
int RVCALLCONV cmCallDeclareMC(
                 /* declare this terminal to be the MC of the call */
        IN      HCALL               hsCall
        );




/* When the conference becomes active, this function provides the address of the Active MC. */
RVAPI
int RVCALLCONV cmCallGetMCAddress(
                /* get active MC address */
        IN      HCALL               hsCall,
        OUT     UINT32*             ip,
        OUT     UINT16*             port
        );





typedef struct {
  HCHAN hsChan; /* outgoing: new reverse channel. incoming: keep the existing channel
           null: new not associated reverse channel. */

  /* ! set the following fields only for new outgoing channels */

  HCHAN hsChanAssociate; /* associate channel. FFS */

  char *channelName; /* channel name from configuration or NULL for handle setting. */
  INT32 channelDataTypeHandle; /* data type (HPVT) tree handle. channelName should be NULL. */

  cmTerminalLabel terminalLabel; /* Use 0xffff if label unavailable */

  char *sessionDescription;
  int sessionDescriptionLength; /* number of bytes in sessionDescription */

  cmTransportAddress rtpAddress;
  cmTransportAddress rtcpAddress;
/* marina used only in cmCallSetChannelsExt function */
  int uid; /* uniq id may be index in array.*/
  int dependency; /* uniq id of element,which describe the channel,that this channel is dependent on */
  /* if its value is equal -1 ,parameter is not used. */
  int redEncodingId; /* node id of redundancyEncoding pvt,built by user. */

} cmChannelStruct;


/* The master (MC) terminal shall use cmCallSetChannels() to force the remote terminal to open
   the indicated channels for transmit. */
RVAPI
int RVCALLCONV cmCallSetChannels(
                 /* As an Active MC, set transfer channels for remote terminal */
        IN      HCALL               hsCall,
        IN      int                 channelSetSize, /* number of elements in channelSet */
        IN      cmChannelStruct     channelSet[]
        );

/*marina*/
/* The master (MC) terminal shall use cmCallSetChannels() to force the remote terminal to open
   the indicated channels for transmit.Add few parameters specified in version 3,such as redEncoding
   and sessionDependancy */

RVAPI
int RVCALLCONV cmCallSetChannelsExt(
                 /* As an Active MC, set transfer channels for remote terminal */
        IN      HCALL               hsCall,
        IN      int                 channelSetSize, /* number of elements in channelSet */
        IN      cmChannelStruct     channelSet[]
        );



/*   multipoint mode set/cancel operations */

RVAPI int RVCALLCONV
cmCallMultipointCommand(
          /* Send multipoint command (on or off) message */
          IN  HCALL hsCall,
          IN  BOOL isModeOn /* TRUE: mp mode (on), FALSE: cancel mp mode (off) */
          );


RVAPI int RVCALLCONV
cmCallMultipointStatus(
          /* Get the multipoint mode status (on or off) */
          IN  HCALL hsCall,
          OUT BOOL *isModeOn /* TRUE: mp mode (on), FALSE: cancel mp mode (off) */
          );


RVAPI int RVCALLCONV
cmChannelSendTransportCapInd(
         IN     HCHAN       hsChan,
         IN     int         nodeId);  /* node Id of type TransportCapability */

RVAPI int RVCALLCONV
cmChannelSendMiscCommand(
         IN     HCHAN       hsChan,
         IN     cmMiscellaneousCommand miscCommand);


/* delete pvt of remote capability to reduce memory*/
RVAPI
int RVCALLCONV cmFreeCapability(  IN HCALL hsCall);


RVAPI
int RVCALLCONV cmChannelSetNSAPAddress(
        IN      HCHAN               hsChan,
        IN      char*               address,
        IN      int                 length,
        IN      BOOL                multicast);





RVAPI
int RVCALLCONV cmChannelSetATMVC(
        IN      HCHAN               hsChan,
        IN      int                 portNumber);


RVAPI
INT32 RVCALLCONV cmChannelGetNumber(IN HCHAN hsChan);





#endif
#endif
#ifdef __cplusplus
}
#endif



