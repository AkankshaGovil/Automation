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

#ifndef CM_H
#define CM_H



#include <rvcommon.h>
#include <rv_defs.h>
#include <coder.h>
#include <pvaltree.h>
#include <ci.h>


#ifndef RVAPI
#define RVAPI
#endif

#ifndef RVCALLCONV
#define RVCALLCONV
#endif

DECLARE_OPAQUE(HAPP);
DECLARE_OPAQUE(HAPPAPP);
DECLARE_OPAQUE(HAPPCALL);
DECLARE_OPAQUE(HCALL);
DECLARE_OPAQUE(HPROTCONN);
DECLARE_OPAQUE(HPROTOCOL);
DECLARE_OPAQUE(HAPPCONN);
DECLARE_OPAQUE(HAPPPARTNER);


typedef enum
{
    cmQ931TpktChannel=0,
    cmH245TpktChannel
}cmTpktChanHandleType;

typedef enum
{
    cmRasUdpChannel=0,
    cmRasUdpChannelMCast
}cmUdpChanHandleType;


typedef struct
{
    UINT8 cmCodingStandard;
    UINT8 cmSpare;
    UINT8 cmLocation;
    UINT8 cmRecomendation; /*  optional set to -1 if not used */
    UINT8 cmCauseValue;
}
cmCallCauseInfo;

typedef struct
{
    UINT8 cmCodingStandard;
    UINT8 cmCallStateValue;
}
cmCallStateInfo;

typedef struct
{
    cmCallCauseInfo  callCauseInfo;
    cmCallStateInfo callStateInfo;
    char display[128];
}cmCallStatusMessage;

typedef struct {
  /* Set one of the following choises */

  /* 1. object id */
  char object[128]; /* in object identifier ASN format: "0 1 2" */
  int objectLength; /* in bytes. <=0 if not set */

  /* 2. h.221 id */
  UINT8 t35CountryCode;
  UINT8 t35Extension;
  UINT16 manufacturerCode;
} cmNonStandardIdentifier;

typedef struct
{
    BYTE    t35CountryCode;
    BYTE    t35Extension;
    UINT16  manufacturerCode;
} cmNonStandard;

typedef struct
{
    cmNonStandard   info;
    int             productLen;
    int             versionLen;
    char            productID[256];
    char            versionID[256];
} cmVendor;

typedef struct
{
    cmNonStandardIdentifier   info;
    int               length;
    char*             data;
} cmNonStandardParam;



typedef enum
{
    cmAliasTypeE164=cmTransportTypeIPLooseRoute+5,
    cmAliasTypeH323ID,
    cmAliasTypeEndpointID,
    cmAliasTypeGatekeeperID,
    cmAliasTypeURLID,
    cmAliasTypeTransportAddress,
    cmAliasTypeEMailID,
    cmAliasTypePartyNumber
} cmAliasType;


typedef enum
{
            /* the numbering plan is according to
               Recommendations E.163 and E.164.*/
    cmPartyNumberPublicUnknown,
        /* if used number digits carry prefix indicating type
           of number according to national recommendations*/
    cmPartyNumberPublicInternationalNumber  ,
    cmPartyNumberPublicNationalNumber   ,
    cmPartyNumberPublicNetworkSpecificNumber,
        /* not used, value reserved */
    cmPartyNumberPublicSubscriberNumber ,
    cmPartyNumberPublicAbbreviatedNumber    ,
        /* valid only for called party number at the outgoing
           access, network substitutes appropriate number. */

    cmPartyNumberDataPartyNumber,
        /* not used, value reserved */
    cmPartyNumberTelexPartyNumber,
        /* not used, value reserved*/
    cmPartyNumberPrivateUnknown,
    cmPartyNumberPrivateLevel2RegionalNumber,
    cmPartyNumberPrivateLevel1RegionalNumber,
    cmPartyNumberPrivatePISNSpecificNumber,
    cmPartyNumberPrivateLocalNumber,
    cmPartyNumberPrivateAbbreviatedNumber,
    cmPartyNumberNationalStandardPartyNumber
} cmPartyNumberType;

typedef struct
{
    cmAliasType   type;
    UINT16        length; /* string in bytes */
    char*         string;
    cmPartyNumberType pnType;
    cmTransportAddress    transport;
} cmAlias;

typedef enum
{
    cmEndpointTypeTerminal,
    cmEndpointTypeGateway,
    cmEndpointTypeMCU,
    cmEndpointTypeGK,
    cmEndpointTypeUndefined,
    cmEndpointTypeSET
} cmEndpointType;

typedef enum
{
    cmCallTypeP2P,
    cmCallTypeOne2N,
    cmCallTypeN2One,
    cmCallTypeN2Nw
} cmCallType;


typedef enum
{
    cmCallModelTypeDirect,
    cmCallModelTypeGKRouted
} cmCallModelType;

typedef enum
{
    cmTransportQOSEndpointControlled,
    cmTransportQOSGatekeeperControlled,
    cmTransportQOSNoControl
} cmTransportQOS;

#include <cmras.h>
#include <cmctrl.h>
#include <cmhook.h>
#include <faststart.h>

/************************************************************************
 * cmPreGrantedArqUse
 * Indicates the way PreGranted ARQ is used by the EP for a call, even when the
 * gatekeeper (using preGrantedArq parameter) allows direct calls.
 * cmPreGrantedArqDirect - PreGranted ARQ feature is used for both direct and routed calls.
 * cmPreGrantedArqRouted - PreGranted ARQ feature is used only for routed calls.
 ************************************************************************/
typedef enum
{
    cmPreGrantedArqDirect=0,
    cmPreGrantedArqRouted
}cmPreGrantedArqUse;

typedef enum
{
    cmCallStateInit=-1,
    cmCallStateDialtone,
    cmCallStateProceeding,
    cmCallStateRingBack,
    cmCallStateConnected,
    cmCallStateDisconnected,
    cmCallStateIdle,
    cmCallStateOffering,
    cmCallStateTransfering,
    cmCallStateIncompleteAddress = cmCallStateTransfering + 3,
    cmCallStateWaitAddressAck
} cmCallState_e;

typedef enum
{
    cmCallStateModeDisconnectedBusy,
    cmCallStateModeDisconnectedNormal,
    cmCallStateModeDisconnectedReject,
    cmCallStateModeDisconnectedUnreachable,
    cmCallStateModeDisconnectedUnknown,
    cmCallStateModeDisconnectedLocal,

    cmCallStateModeConnectedControl, /* not used. reserved for backward compatability */
    cmCallStateModeConnectedCallSetup,
    cmCallStateModeConnectedCall,
    cmCallStateModeConnectedConference,

    /* The next 3 state modes are here for backward compatability */
    cmCallStateModeControlConnected=cmCallStateModeConnectedControl,
    cmCallStateModeCallSetupConnected,
    cmCallStateModeCallConnected,

    cmCallStateModeOfferingCreate=cmCallStateModeConnectedConference+1,
    cmCallStateModeOfferingInvite,
    cmCallStateModeOfferingJoin,
    cmCallStateModeOfferingCapabilityNegotiation,
    cmCallStateModeOfferingCallIndependentSupplementaryService,

    cmCallStateModeDisconnectedIncompleteAddress,

    cmCallStateModeTransferingForwarded,
    cmCallStateModeTransferingRouteToMC,
    cmCallStateModeTransferingRouteToGatekeeper
} cmCallStateMode_e;

typedef enum
{
    cmReasonTypeNoBandwidth,
    cmReasonTypeGatekeeperResource,
    cmReasonTypeUnreachableDestination,
    cmReasonTypeDestinationRejection,
    cmReasonTypeInvalidRevision,
    cmReasonTypeNoPermision,
    cmReasonTypeUnreachableGatekeeper,
    cmReasonTypeGatewayResource,
    cmReasonTypeBadFormatAddress,
    cmReasonTypeAdaptiveBusy,
    cmReasonTypeInConf,
    cmReasonTypeUndefinedReason,
    cmReasonTypeRouteCallToGatekeeper,
    cmReasonTypeCallForwarded,
    cmReasonTypeRouteCallToMC,
    cmReasonTypeFacilityCallDeflection,
    cmReasonTypeSecurityDenied,
    cmReasonTypeCalledPartyNotRegistered,
    cmReasonTypeCallerNotregistered,
    cmReasonTypeConferenceListChoice,
    cmReasonTypeStartH245,
    cmReasonTypeNewConnectionNeeded,
    cmReasonTypeNoH245,
    cmReasonTypeNewTokens,                  /* FacilityReason */
    cmReasonTypeFeatureSetUpdate,           /* FacilityReason */
    cmReasonTypeForwardedElements,          /* FacilityReason */
    cmReasonTypeTransportedInformation      /* FacilityReason */
} cmReasonType;



typedef enum
{
    cmCreate,
    cmJoin,
    cmInvite,
    cmCapabilityNegotiation,
    cmCallIndependentSupplementaryService,
    cmLastCG
} cmConferenceGoalType;


/************************************************************************
 * cmInformationTransferCapability
 * The Information transfer capability that is part of the Bearer Capability
 * element of Q931 messages.
 * cmITCSpeech              - Speech
 * cmITCUnrestricted        - Unrestricted digital information
 * cmITCRestricted          - Restricted digital information
 * cmITCAudio               - 3.1 kHz audio
 * cmITCUnrestrictedTones   - Unrestricted digital information with tones/announcements
 * cmITCVideo               - Video
 ************************************************************************/
typedef enum
{
    cmITCSpeech=0,
    cmITCUnrestricted=8,
    cmITCRestricted=9,
    cmITCAudio=16,
    cmITCUnrestrictedTones=17,
    cmITCVideo=24
} cmInformationTransferCapability;

typedef enum
{
    cmParamFirst=0x1000,
    cmParamSourceAddress,                   /* cmAlias */
    cmParamSourceInfo,                      /* cmEndpointType enum */
    cmParamDestinationAddress,              /* cmAlias */
    cmParamDestCallSignalAddress,           /* cmTransportAddress */
    cmParamSrcCallSignalAddress,            /* cmTransportAddress */
    cmParamDestExtraCallInfo,               /* cmAlias */
    cmParamActiveMc,                        /* BOOL */
    cmParamCID,                             /* char[16] */
    cmParamConferenceGoal,                  /* cmConferenceGoalType enum */
    cmParamCallType,                        /* cmCallType */
    cmParamSetupH245Address,                /* cmTransportAddress */
    cmParamH245Address,                     /* cmTransportAddress */
    cmParamDestinationInfo,                 /* cmEndpointType enum */
    cmParamReleaseCompleteReason,           /* cmReasonType */
    cmParamReleaseCompleteCause,            /* int */
    cmParamRate,                            /* int */
    cmParamRequestedRate,                   /* int */
    cmParamInformationTransferCapability,   /* cmInformationTransferCapability enum */
    cmParamMultiRate,                       /* BOOL */
    cmParamCalledPartyNumber,               /* cmAlias */
    cmParamCalledPartySubAddress,           /* cmAlias */
    cmParamCallingPartyNumber,              /* cmAlias */
    cmParamCallingPartySubAddress,          /* cmAlias */
    cmParamExtention,                       /* cmAlias */
    cmParamAlternativeAddress,              /* cmTransportAddress */
    cmParamAlternativeAliasAddress,         /* cmAlias */
    cmParamFacilityReason,                  /* cmReasonType */
    cmParamDestinationIpAddress,            /* cmTransportAddress - physical address, used to detrmine actual destination of call */
    cmParamRemoteIpAddress,                 /* cmTransportAddress */
    cmParamCRV,                             /* int */
    cmParamRASCRV,                          /* int */
    cmParamCallSignalAddress,               /* cmTransportAddress */
    cmParamEstablishH245,                   /* BOOL */
    cmParamDisplay,                         /* char*        */
    cmParamFacilityCID,                     /* char[16] */
    cmParamConnectDisplay,                  /* char*        */
    cmParamUserUser,                        /* char*        */
    cmParamConnectUserUser,                 /* char*        */
    cmParamFullSourceInfo,                  /* int (nodeId) */
    cmParamFullDestinationInfo,             /* int (nodeId) */
    cmParamSetupNonStandardData,            /* int (nodeId) */
    cmParamCallProceedingNonStandardData,   /* int (nodeId) */
    cmParamAlertingNonStandardData,         /* int (nodeId) */
    cmParamConnectNonStandardData,          /* int (nodeId) */
    cmParamReleaseCompleteNonStandardData,  /* int (nodeId) */
    cmParamSetupNonStandard,                /* cmNonStandardParam */
    cmParamCallProceedingNonStandard,       /* cmNonStandardParam */
    cmParamAlertingNonStandard,             /* cmNonStandardParam */
    cmParamConnectNonStandard,              /* cmNonStandardParam */
    cmParamReleaseCompleteNonStandard,      /* cmNonStandardParam */
    cmParamAlternativeDestExtraCallInfo,    /* cmAlias */
    cmParamAlternativeExtention,            /* cmAlias */
    cmParamSetupFastStart,                  /* int (nodeId) */
    cmParamConnectFastStart,                /* int (nodeId) */
    cmParamEarlyH245,                       /* BOOL */
    cmParamCallID,                          /* char[16] */
    cmParamPreGrantedArq,                   /* cmPreGrantedArqUse */
    cmParamAlertingFastStart,               /* int (nodeId) */
    cmParamCallProcFastStart,               /* int (nodeId) */
    cmParamAlertingH245Address,             /* cmTransportAddress*/
    cmParamCallProcH245Address,             /* cmTransportAddress*/
    cmParamCanOverlapSending,               /* BOOL */
    cmParamIncompleteAddress,               /* Resrved for backward compatability - not used*/
    cmParamH245Tunneling,                   /* BOOL */
    cmParamFastStartInUseObsolete,          /* Resrved for backward compatability - not used*/
    cmParamSetupCanOverlapSending,          /* BOOL */
    cmParamSetupSendingComplete,            /* BOOL */
    cmParamFullSourceAddress,               /* int (nodeId) */
    cmParamFullDestinationAddress,          /* int (nodeId) */
    cmParamRouteCallSignalAddress,          /* cmTransportAddress*/
    cmParamH245Stage,                       /* cmH245Stage */
    cmParamH245Parallel,                    /* BOOL */
    cmParamShutdownEmptyConnection,         /* BOOL */
    cmParamIsMultiplexed,                   /* BOOL */
    cmParamAnnexE,                          /* cmAnnexEUsageMode */
    cmParamDestinationAnnexEAddress,        /* cmTransportAddress - physical address used to determine actual destination of call */
    cmParamConnectedAddress,                 /* cmAlias */
	cmParamLocalIp							/* NexTone localIp address */
} cmCallParam;


typedef int cmParam;

typedef int
(*cmNewCallbackEH)(void);


typedef enum
{
   cmIdle,
   cmDiscovered,
   cmRegistered
} cmRegState;

typedef enum
{
   cmGatekeeperConfirm,
   cmGatekeeperReject,
   cmRegistrationConfirm,
   cmRegistrationReject,
   cmUnregistrationRequest,
   cmUnregistrationConfirm,
   cmUnregistrationReject,
   cmNonStandardMessage,
   cmResourceAvailabilityConfirmation
} cmRegEvent;



typedef int
    (RVCALLCONV *cmEvNewCallT)(
        IN      HAPP                hApp,
        IN      HCALL               hsCall,
        OUT     LPHAPPCALL          lphaCall);


typedef int
    (RVCALLCONV *cmEvRegEventT)(
        IN      HAPP                hApp,
        IN      cmRegState          regState,
        IN      cmRegEvent          regEvent,
        IN      int                 regEventHandle);

typedef  struct
{
    cmEvNewCallT cmEvNewCall;
    cmEvRegEventT cmEvRegEvent;
} SCMEVENT,*CMEVENT;

typedef int
    (RVCALLCONV *cmEvCallStateChangedT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              state,
        IN      UINT32              stateMode);


typedef int
    (RVCALLCONV *cmEvCallNewRateT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      UINT32              rate);

typedef int
    (RVCALLCONV*cmEvCallInfoT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      char*               display,
        IN      char*               userUser,
                IN      int                 userUserSize);

/* This callback is obsolete. It is here for backward compatability. It is never
   called by the stack */
typedef int
    (RVCALLCONV*cmCallNonStandardParamT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      char*               data,
        IN      int                 dataSize);

typedef int
    (RVCALLCONV*cmEvCallFacilityT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 handle,
        OUT IN  BOOL                *proceed);

typedef int
    (RVCALLCONV * cmEvCallFastStartSetupT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        OUT IN  cmFastStartMessage  *fsMessage);

typedef int
    (RVCALLCONV*cmEvCallFastStartT)(
        IN  HAPPCALL                haCall,
        IN  HCALL                   hsCall);


typedef int
    (RVCALLCONV * cmEvCallStatusT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        OUT IN  cmCallStatusMessage *callStatusMsg);

typedef int
    (RVCALLCONV * cmEvCallUserInfoT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 nodeId);

typedef int
    (RVCALLCONV * cmEvCallH450SupplServT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      cmCallQ931MsgType   msgType,
        IN      int                 nodeId,
        IN      int                 size);

typedef int
    (RVCALLCONV * cmEvCallIncompleteAddressT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall);

typedef int
    (RVCALLCONV * cmEvCallAdditionalAddressT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      char*               address,
        IN      BOOL                sendingComplete);

typedef int
    (RVCALLCONV * cmEvCallTunnNewMessageT)(
         IN     HAPPCALL            haCall,
         IN     HCALL               hsCall,
         IN     int                 vtNodeId,
         IN     int                 vtAddNodeId,
         OUT    BOOL*               wait);

/********************************************************************************************
 * cmEvCallProgressT
 * purpose : Notification function for the application that a Q931 Progress message was
 *           received on a call.
 * input   : haCall         - Application's handle for the call
 *           hsCall         - Stack's handle for the call
 *           handle         - The PVT root node ID of the Progress message
 * output  : none
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallProgressT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 handle);

/********************************************************************************************
 * cmEvCallNotiftyT
 * purpose : Notification function for the application that a Q931 Notify message was
 *           received on a call.
 * input   : haCall         - Application's handle for the call
 *           hsCall         - Stack's handle for the call
 *           handle         - The PVT root node ID of the Notify message
 * output  : none
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallNotifyT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 handle);

/********************************************************************************************
 * cmEvCallNewAnnexLMessageT
 * purpose : Notification function for the application that there has been a new Annex L
 *           message inside an incoming Q931 message.
 *           Applications that want to implement Annex L capabilities should implement
 *           this function.
 * input   : haCall         - Application's handle for the call
 *           hsCall         - Stack's handle for the call
 *           annexLElement  - PVT node ID of the Annex L element inside a Q931 message
 *           msgType        - The type of the message from which we extracted the Annex L
 *                            message
 * output  : none
 * note    : cmCallGetAnnexLMessage can be used to analyze the Annex L message
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallNewAnnexLMessageT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 annexLElement,
        IN      int                 msgType);

/********************************************************************************************
 * cmEvCallNewAnnexMMessageT
 * purpose : Notification function for the application that there has been a new Annex M
 *           message inside an incoming Q931 message.
 *           Applications that want to implement Annex M capabilities should implement
 *           this function.
 * input   : haCall         - Application's handle for the call
 *           hsCall         - Stack's handle for the call
 *           annexMElement  - PVT node ID of the Annex M element inside a Q931 message
 *           msgType        - The type of the message from which we extracted the Annex M
 *                            message
 * output  : none
 * note    : cmCallGetAnnexMMessage can be used to analyze the Annex M message
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallNewAnnexMMessageT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 annexMElement,
        IN      int                 msgType);

/********************************************************************************************
 * cmEvCallRecvMessageT
 * purpose : Notification function for the application that an incoming Q931 or H245 message
 *           was received by the stack and is about to be processed.
 * input   : haCall     - Application's handle for the call
 *           hsCall     - Stack's handle for the call
 *           message    - PVT node ID of the message that was received
 * output  : none
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallRecvMessageT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 message);

/********************************************************************************************
 * cmEvCallSendMessageT
 * purpose : Notification function for the application that an outgoing Q931 or H245 message
 *           is about to be sent by the stack.
 * input   : haCall     - Application's handle for the call
 *           hsCall     - Stack's handle for the call
 *           message    - PVT node ID of the message that is about to be sent
 * output  : none
 ********************************************************************************************/
typedef int
    (RVCALLCONV*cmEvCallSendMessageT)(
        IN      HAPPCALL            haCall,
        IN      HCALL               hsCall,
        IN      int                 message);

typedef  struct
{
    cmEvCallStateChangedT       cmEvCallStateChanged;
    cmEvCallNewRateT            cmEvCallNewRate;
    cmEvCallInfoT               cmEvCallInfo;
    cmCallNonStandardParamT     cmCallNonStandardParam;     /* This callback is not used */
    cmEvCallFacilityT           cmEvCallFacility;
    cmEvCallFastStartSetupT     cmEvCallFastStartSetup;
    cmEvCallStatusT             cmEvCallStatus;
    cmEvCallUserInfoT           cmEvCallUserInfo;
    cmEvCallH450SupplServT      cmEvCallH450SupplServ;
    cmEvCallIncompleteAddressT  cmEvCallIncompleteAddress;
    cmEvCallAdditionalAddressT  cmEvCallAdditionalAddress;

    cmEvCallTunnNewMessageT     cmEvCallTunnNewMessage;
    cmEvCallFastStartT          cmEvCallFastStart;
    cmEvCallProgressT           cmEvCallProgress;
    cmEvCallNotifyT             cmEvCallNotify;
    cmEvCallNewAnnexLMessageT   cmEvCallNewAnnexLMessage;
    cmEvCallNewAnnexMMessageT   cmEvCallNewAnnexMMessage;
    cmEvCallRecvMessageT        cmEvCallRecvMessage;
    cmEvCallSendMessageT        cmEvCallSendMessage;
} SCMCALLEVENT,*CMCALLEVENT;


/********************************************************************************************
 * cmStartUp
 * purpose : This function should be called prior to any cmInit() or cmInitialize() calls.
 *           It must be called when several cmInitialize() functions are called from
 *           different threads at the same time.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
RVAPI
void RVCALLCONV cmStartUp(void);


/************************************************************************
 * cmInitialize
 * purpose: Initializes the Conference Manager instance.
 *          This function must be called before any other H.323 function
 *          except cmGetVersion().
 * input  : name    - Configuration file name to use
 * output : lphApp  - Application handle created for the initialized
 *                    stack instance
 * return : non-negative value on success
 *          negative value on failure
 *          RESOURCES_PROBLEM   - Resource problem
 *          -10                 - Memory problem
 *          -11                 - Configuration problem
 *          -13                 - Network problem
 ************************************************************************/
RVAPI
int RVCALLCONV cmInitialize(IN char * name, OUT LPHAPP lphApp);

/************************************************************************
 * cmInit
 * purpose: Initializes the Conference Manager instance.
 *          This function must be called before any other H.323 function
 *          except cmGetVersion().
 * input  : name    - Configuration file name to use
 * output : none
 * return : Stack's application handle on success
 *          NULL on failure
 ************************************************************************/
RVAPI
HAPP RVCALLCONV cmInit(IN char * name);

/************************************************************************
 * cmEnd
 * purpose: Shuts down the Conference Manager instance and releases all
 *          resources that were in use by the Conference Manager instance.
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmEnd(
        IN   HAPP        hApp);


/************************************************************************
 * cmStart
 * purpose: Starts the stack's activity
 *
 *          This function is only applicable when system.manualStart key
 *          is defined in the configuration. In manualStart mode cmInitialize()
 *          function does not automatically start Stack activity and accesses only
 *          the "system" configuration tree. To start the Stack use cmStart() function.
 *          The application may change configuration settings between cmInitialize()
 *          and cmStart() using cmGetXXXConfigurationHandle() functions.
 *
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmStart(
        IN  HAPP        hApp);

/************************************************************************
 * cmStop
 * purpose: Stops the stack's activity
 *
 *          After the Stack is stopped by cmStop(), the application may change
 *          configuration settings and then use cmStart() to start Stack
 *          activity again.
 *
 * input  : hApp    - Stack handle for the application
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmStop(
        IN  HAPP        hApp);

/************************************************************************
 * cmRegister
 * purpose: Registers the endpoint with the gatekeeper.
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRegister(
        IN  HAPP        hApp);

/************************************************************************
 * cmUnregister
 * purpose: Unregisters the endpoint from the Gatekeeper
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmUnregister(
        IN  HAPP        hApp);


/************************************************************************
 * cmGetRASConfigurationHandle
 * purpose: Gets the root Node ID of the RAS configuration tree.
 *          The application can then get and change configuration parameters
 *          for the control procedures.
 * input  : hApp     - Application's stack handle
 * output : none
 * return : The PVT Node ID of the RASConfiguration subtree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetRASConfigurationHandle(
        IN  HAPP             hApp);



/************************************************************************
 * cmSendNonStandardMessage
 * purpose: Sends a nonstandard message to the Gatekeeper
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 *          nsParam - Nonstandard parameter to be used in non standard message
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmSendNonStandardMessage(
     IN      HAPP                hApp,
     IN      cmNonStandardParam* nsParam);

/************************************************************************
 * cmGetVersion
 * purpose: Returns the version of the Conference Manager in use.
 * input  : none
 * output : none
 * return : Pointer to the string representing the version of the
 *          Conference Manager. For example, "3.0.0.0" or "2.5".
 ************************************************************************/
RVAPI
char* RVCALLCONV cmGetVersion(void);

/************************************************************************
 * cmGetVersionName
 * purpose: Returns the version of the Conference Manager in use.
 * input  : buff    - Buffer to set the version in
 *          length  - Maximum length of the buffer in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmGetVersionName(
        IN    char*   buff,
        IN    int     length);


RVAPI
int RVCALLCONV
cmSetPreCallbackEvent(
        IN      HAPP                hApp,
        IN      cmNewCallbackEH     newCallback
        );

/************************************************************************
 * cmSetGenEventHandler
 * purpose: Installs the general event handler
 * input  : hApp    - Stack handle for the application
 *          cmEvent - Pointer to array of pointers to general event callback functions
 *          size    - Size of *cmEvent in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmSetGenEventHandler(
        IN      HAPP                hApp,
        IN      CMEVENT             cmEvent,
        IN      int                 size);

RVAPI
int RVCALLCONV cmGetGenEventHandler(
        IN      HAPP                hApp,
        IN      CMEVENT             cmEvent,
        IN      int                 size);


/************************************************************************
 * cmSetCallEventHandler
 * purpose: Installs the call event handler
 * input  : hApp        - Stack handle for the application
 *          cmCallEvent - Pointer to array of pointers to general event callback functions
 *          size        - Size of *cmCallEvent in bytes
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmSetCallEventHandler(
        IN      HAPP                hApp,
        IN      CMCALLEVENT         cmCallEvent,
        IN      int                 size);

RVAPI
int RVCALLCONV cmGetCallEventHandler(
        IN      HAPP                hApp,
        OUT     CMCALLEVENT         cmCallEvent,
        IN      int                 size);


/************************************************************************
 * cmCallNew
 * purpose: Creates a new call object that belongs to a particular Stack instance.
 *          This function does not launch any action on the network. It only causes
 *          the application and the Stack to exchange handles.
 * input  : hApp        - Stack handle for the application
 *          haCall      - Application's handle for the call
 * output : lphsCall    - Stack handle for the call to dial
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallNew(
        IN   HAPP        hApp,
        IN   HAPPCALL    haCall,
        OUT  LPHCALL     lphsCall);


/************************************************************************
 * cmCallDial
 * purpose: Dials a call. This function together with cmCallSetParam()
 *          can be used instead of cmCallMake(). cmCallSetParam() should
 *          be called before cmCallDial() and the required parameters of
 *          the call should be set
 * input  : hsCall      - Stack handle for the call to dial
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallDial(
        IN  HCALL       hsCall);


/************************************************************************
 * cmCallMake
 * purpose: Starts a new call setup with the given parameters
 * input  : hsCall      - Stack handle for the new call
 *          maxRate     - Maximum rate allowed for the new call
 *          minRate     - Minimum rate allowed for the new call.
 *                        This parameter is not currently in use, so set it to zero (0).
 *          destAddress - Called party address list
 *          srcAddress  - Calling party address list
 *          display     - String representing display information for reporting
 *                        to the called party
 *          userUser    - String representing user-to-user information for reporting
 *                        to the called party
 *          userUserSize- Length of the string representing user-to-user information
 *                        to report to the called party
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallMake(
        IN  HCALL       hsCall,
        IN  UINT32      maxRate,
        IN  UINT32      minRate,
        IN  char*       destAddress,
        IN  char*       srcAddress,
        IN  char*       display,
        IN  char*       userUser,
        IN  int         userUserSize);


RVAPI
int RVCALLCONV cmCallFacility(
           IN HCALL     hsCall,
           IN int       handle);

RVAPI
int RVCALLCONV cmSetCallHandle(
        IN HCALL                    hsCall,
        IN HAPPCALL                 haCall);


RVAPI
int RVCALLCONV cmCallGetDisplayInfo(
        IN  HCALL           hsCall,
        OUT char*           display,
        IN  int             displaySize);

RVAPI
int RVCALLCONV cmCallGetUserUserInfo(
        IN  HCALL           hsCall,
        OUT char*           userUser,
        IN  int             userUserSize);

RVAPI
int RVCALLCONV cmCallAnswerExt(
        IN  HCALL           hsCall,
                IN      char*               display,
                IN      char*               userUser,
                IN      int                 userUserSize);

RVAPI
int RVCALLCONV cmCallAnswerDisplay(
        IN  HCALL           hsCall,
                IN      char*               display);

/************************************************************************
 * cmCallAnswer
 * purpose: Answers an incoming call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallAnswer(
        IN    HCALL       hsCall);


/************************************************************************
 * cmCallAccept
 * purpose: Accepts an incoming call and sends Q.931 ALERTING message to
 *          calling party
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallAccept(
        IN    HCALL       hsCall);


/************************************************************************
 * cmCallDrop
 * purpose: Drops a call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallDrop(IN  HCALL       hsCall);


RVAPI
int RVCALLCONV cmCallDropParam(
        IN  HCALL           hsCall,
                IN      cmReasonType        cmReason);
RVAPI
int RVCALLCONV cmCallClose(
        IN      HCALL               hsCall);

RVAPI
int RVCALLCONV cmCallGetCallingPartyId(
        IN      HCALL               hsCall,
        OUT     char*               callerId,
        IN      int                 size);

#define cmCallGetCallerId cmCallGetCallingPartyId

RVAPI
int RVCALLCONV cmCallGetCalledPartyId(
        IN      HCALL               hsCall,
        OUT     char*               calledPartyId,
        IN      int                 size);

RVAPI
int RVCALLCONV cmCallGetRate(
        IN      HCALL               hsCall,
        OUT     UINT32*             rate);

RVAPI
int RVCALLCONV cmCallGetOrigin(
        IN      HCALL               hsCall,
        OUT     BOOL*               origin);

RVAPI
int RVCALLCONV cmCallSetRate(
        IN      HCALL               hsCall,
        IN      UINT32              rate);


RVAPI
int RVCALLCONV cmCallForward(
        IN  HCALL       hsCall,
        IN  char*       destAddress);


RVAPI
int RVCALLCONV cmCallGetParam(
        IN      HCALL               hsCall,
        IN      cmCallParam         param,
        IN      int                 index,
        INOUT   INT32*              value,
        OUT     char *              svalue);

RVAPI
int RVCALLCONV cmCallSetParam(
        IN      HCALL               hsCall,
        IN      cmCallParam         param,
        IN      int                 index,
        IN      INT32               value,
        IN      char *              svalue);


/************************************************************************
 * cmCallDeleteParam
 * purpose: Delete a parameter that was set in the call.
 * input  : hsCall      - Stack handle for the call
 *          param       - Parameter to delete
 *          index       - Index of parameter to delete (1-based)
 *                        Should be 0 for non-array parameters
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallDeleteParam(
        IN      HCALL               hsCall,
        IN      cmCallParam         param,
        IN      int                 index);


RVAPI
HPVT RVCALLCONV cmGetValTree(
                IN  HAPP             hApp);



RVAPI
int RVCALLCONV cmCallGetNumOfParams(
        IN      HCALL               hsCall,
        IN      cmCallParam         param);


RVAPI
HPST RVCALLCONV cmGetSynTreeByRootName(
                IN  HAPP            hApp,
                IN  char*           name);

RVAPI
INT32 RVCALLCONV cmGetQ931ConfigurationHandle(
                IN  HAPP            hApp);


RVAPI
int RVCALLCONV cmCallJoin(
                IN      HCALL               hsCall,
                IN      HCALL               hsSameConferenceCall);

RVAPI
int RVCALLCONV cmCallInvite(
                IN      HCALL               hsCall,
                IN      HCALL               hsSameConferenceCall);

RVAPI
int RVCALLCONV cmCallIsSameConference(
        IN  HCALL           hsCall,
                IN  HCALL           hsAnotherCall);

RVAPI char* RVCALLCONV
cmGetProtocolMessageName(
                IN      HAPP                hApp,
                IN      int                 msg);



RVAPI
int RVCALLCONV cmCallGetRemoteEndpointInfoHandle(
        IN  HCALL       hsCall);

RVAPI
int RVCALLCONV cmCallGetLocalEndpointInfoHandle(
        IN  HCALL       hsCall);

RVAPI
int RVCALLCONV cmGetEndpointInfoHandle(
        IN  HAPP        hApp);



RVAPI
int RVCALLCONV cmGetParam(
        IN HAPP         hApp,
        IN cmParam      param,
        IN int          index,
        IN INT32*       value,
        IN char *       svalue);


RVAPI int RVCALLCONV
cmSetHandle(
        IN      HAPP                hApp,
        IN      HAPPAPP     haApp);

RVAPI int RVCALLCONV
cmGetHandle(
        IN      HAPP                hApp,
        IN      HAPPAPP*        haApp);

/* nonStandard handling_______________________________________________________________*/

RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmNonStandardParameterCreate(
                 /* Create NonStandardParameter tree */
                 IN  HPVT hVal,
                 IN  int rootId, /* nonStandardData */

                 IN  cmNonStandardIdentifier *identifier,
                 IN  char *data,
                 IN  int dataLength /* in bytes */
                 );

RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmNonStandardParameterGet(
              /* Convert NonStandardParameter tree to C structures */
              IN  HPVT hVal,
              IN  int rootId, /* nonStandardData */

              OUT cmNonStandardIdentifier *identifier,
              OUT char *data,
              INOUT INT32 *dataLength /* (in bytes) IN: data allocation. OUT: correct size */
              );
RVAPI
int RVCALLCONV cmCallConnectControl(
                IN      HCALL               hsCall
                );

RVAPI
int RVCALLCONV
cmCallSw2SeparateH245(IN      HCALL               hsCall);

RVAPI
int RVCALLCONV cmCallSetIndependentSupplementaryService(
                IN      HCALL               hsCall
                );

RVAPI
int RVCALLCONV cmCallSetControlRemoteAddress(
                IN      HCALL               hsCall,
                IN      cmTransportAddress  *addr
                );

/************************************************************************
 * cmCallSendSuppServiceManually
 * purpose: Sends an H.450 APDU on the next Q.931 message that the
 *          application initiates. It will not initiate a FACILITY message
 *          on its own as cmCallSendSupplamentaryService() might send.
 * input  : hsCall      - Stack handle for the call
 *          buffer      - Buffer in which the APDU has been placed
 *          bufferSize  - Size of the buffer
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendSuppServiceManually(
    IN  HCALL       hsCall,
    IN  void*       buffer,
    IN  int         bufferSize);

/************************************************************************
 * cmCallSendSupplementaryService
 * purpose: Sends an H.450 APDU. This function takes a buffer of a
 *          Supplementary Service and puts an H.450 message in the buffer
 * input  : hsCall      - Stack handle for the call
 *          buffer      - Buffer in which the APDU has been placed
 *          bufferSize  - Size of the buffer
 *          force       - If TRUE do not wait for the next Q.931 message
 *                        before sending this APDU. If FALSE, it will wait
 *                        for the application to send a Q.931 message on
 *                        its own if the call isn't in the connected state
 *                        yet.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendSupplementaryService(
    IN  HCALL       hsCall,
    IN  void*       buffer,
    IN  int         bufferSize,
    IN  BOOL        force);


RVAPI
int RVCALLCONV cmAlias2Vt(
            IN      HPVT                valH,
            IN      cmAlias*            alias,
            IN      int                 nodeId);



RVAPI
INT32 RVCALLCONV cmGetLocalCallSignalAddress(
                IN  HAPP             hApp,
                OUT     cmTransportAddress*  tr);


/************************************************************************
 * cmGetLocalAnnexEAddress
 * purpose: Gets the local AnnexE address
 *          Annex E is used to send Q931 messages on UDP.
 * input  : hApp    - Application's stack handle
 * output : tr      - The local Annex E transport address.
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetLocalAnnexEAddress(
               IN   HAPP                hApp,
               OUT  cmTransportAddress* tr);


/************************************************************************
 * cmGetLocalRASAddress
 * purpose: Gets the local RAS address
 * input  : hApp    - Application's stack handle
 * output : tr      - The local RAS signal transport address.
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
INT32 RVCALLCONV cmGetLocalRASAddress(
                IN  HAPP             hApp,
                OUT     cmTransportAddress*  tr);

RVAPI
INT32 RVCALLCONV cmGetGKCallSignalAddress(
                IN  HAPP             hApp,
                OUT     cmTransportAddress*  tr);

RVAPI
INT32 RVCALLCONV cmGetGKRASAddress(
                IN  HAPP             hApp,
                OUT     cmTransportAddress*  tr);

RVAPI
HPROTCONN RVCALLCONV cmGetRASConnectionHandle(
                IN  HAPP             hApp);


/************************************************************************
 * cmVt2Alias
 * purpose: Converts a PVT subtree of AliasAddress type into an Alias
 * input  : hVal    - The PVT handle
 *          nodeId  - PVT subtree of AliasAddress type we want to convert
 * output : alias   - Alias struct converted. The application is responsible
 *                    to supply the string field inside the cmAlias struct
 *                    with enough allocated size.
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV cmVt2Alias(
    IN  HPVT       hVal,
    OUT cmAlias*   alias,
    IN  int        nodeId);


RVAPI int RVCALLCONV cmGetProperty(
                IN      HPROTOCOL        hsProtocol);

RVAPI HAPP RVCALLCONV cmGetAHandle(
                IN      HPROTOCOL            hsProtocol);

RVAPI
int RVCALLCONV cmCallStatusEnquiry(
                    IN  HCALL       hsCall,
                    UINT8 * display);


/*
cmGetH225RemoteVersion
return H225 version number of remote side via OUT parameter
version.
version is user allocated string*/

RVAPI
int RVCALLCONV cmGetH225RemoteVersion(IN HCALL        hsCall,OUT char * version);

/*
cmCallUserInformationCreate
create root of information message .Build all must parameters.
Add display and nonstandard parameter if they are and return root id to user to
do changes if it need
*/
RVAPI
int RVCALLCONV cmCallUserInformationCreate(IN HCALL hsCall,char * display,cmNonStandardParam* param);

/*
cmCallUserInformationSend
send user information message
*/
RVAPI
int RVCALLCONV cmCallUserInformationSend(IN HCALL hsCall, IN int nodeId);

/*
cmCallUserInformationGet
Get rootId of the user information message and retrieve display and nonStandard parameter
Root Id of incoming user information message is passed to callback function of  cmEvCallUserInfoT type
if it is set by user.
*/
RVAPI
int RVCALLCONV cmCallUserInformationGet(IN HAPP hApp,IN int nodeId,
                      OUT char *display,IN int displaySize,
                      OUT cmNonStandardParam* param);

/************************************************************************
 * cmCallSendCallProceeding
 * purpose: Sends Q.931 CALL PROCEEDING message
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendCallProceeding(IN HCALL hsCall);

/* delete pvt of connect,setup,alerting and callProceeding from call property  to reduce memory
 can be run only in connected state.All information from this messages will be lost after runing
this function.*/

RVAPI
int RVCALLCONV cmFreeProperty(    IN HCALL hsCall);


RVAPI
int RVCALLCONV cmCallOverlapSending(IN HCALL hsCall,char * address);



RVAPI
HPROTCONN RVCALLCONV cmGetTpktChanHandle( IN HCALL hsCall,cmTpktChanHandleType  tpktChanHandleType);

RVAPI
HPROTCONN RVCALLCONV cmGetUdpChanHandle(  IN HCALL hsCall,cmUdpChanHandleType  udpChanHandleType);


RVAPI
int RVCALLCONV cmSetUdpChanApplHandle(    IN HPROTCONN hCon,HAPPCONN hAppConn);

RVAPI
int RVCALLCONV cmSetTpktChanApplHandle(   IN HPROTCONN hCon,HAPPCONN hAppConn);

RVAPI
int RVCALLCONV cmGetUdpChanApplHandle(    IN HPROTCONN hCon,HAPPCONN * hAppConn);

RVAPI
int RVCALLCONV cmGetTpktChanApplHandle(   IN HPROTCONN hCon,HAPPCONN * hAppConn);

RVAPI
int RVCALLCONV cmCallIncompleteAddress(IN HCALL hsCall);


RVAPI
int RVCALLCONV cmCallAddressComplete(IN HCALL hsCall);

RVAPI
int RVCALLCONV cmCallOverlapSendingExt(IN HCALL hsCall,char * address,BOOL sendingComplete);

/************************************************************************
 * cmSendRAI
 * purpose: Sends a RAI message to the gatekeeper.
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp                    - Application's handle of the stack
 *          almoustOutOfResources   - Indicates that it is or is not almost
 *                                    out of resources
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI int RVCALLCONV
cmSendRAI(
       IN      HAPP                hApp,
       IN      BOOL     almostOutOfResources);

#define cmCallSetHandle cmSetCallHandle
RVAPI
int RVCALLCONV cmSetCallHandle(IN HCALL hsCall, IN HAPPCALL haCall);


/************************************************************************
 * cmCallGetHandle
 * purpose: Returns the application handle for a call from the call handle.
 * input  : hsCall      - Stack handle for the call
 * output : haCall      - Application handle for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallGetHandle(IN HCALL hsCall, OUT HAPPCALL* haCall);


/************************************************************************
 * cmGetConfiguration
 * purpose: Gives access to the session's configuration context.
 *          The returned HCFG should be considered read-only, and should
 *          never be destroyed.
 * input  : hApp    - Stack instance handle
 * output : none
 * return : HCFG handle of the configuration on success
 *          NULL on failure
 ************************************************************************/
RVAPI
HCFG RVCALLCONV cmGetConfiguration(IN HAPP hApp);


/************************************************************************
 * cmGetDelimiter
 * purpose: Returns the character used as a delimiter between fields,
 *          such as in the destAddress and srcAddress fields in the
 *          cmCallMake function.
 * input  : hApp    - Stack instance handle
 * output : none
 * return : Delimiter character used
 ************************************************************************/
RVAPI char RVCALLCONV
cmGetDelimiter(HAPP hApp);


RVAPI int RVCALLCONV
cmSetPartnerHandle(HAPP hApp, HAPPPARTNER hAppPartner);

RVAPI int RVCALLCONV
cmGetPartnerHandle(HAPP hApp, HAPPPARTNER* hAppPartner);


/************************************************************************************
 *
 * cmCallMultiplex
 *
 * Purpose: To specify call's Q.931 multiplex behavior.
 *          This function call marks the call as supporting the Q.931 multiplexing
 *
 * Input:   hsCall                  - The stack handle to the call
 *          hsSameConnectionCall    - The stack handle to the call which Q.931
 *                                    connection is the prefered connection for the hsCall
 *                                    If this parameter is not NULL and hsSameConnectionCall
 *                                    has Q.931 connection, which remote address equal to the
 *                                    hsCall's desireable Q.931 connection remote address then
 *                                    the same Q.931 connection will be used for both calls.
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/

RVAPI
int RVCALLCONV cmCallMultiplex(
            IN      HCALL               hsCall,
            IN      HCALL               hsSameConnectionCall);


/************************************************************************************
 * progressInd_IE enum
 * Progress indication information element inside Q931.
 * The PROGRESS element can use tag value 30 or 31. When creating a progress record,
 * the application can specify which of the tags to use.
 * use30asPI_IE     - Uses tag value 30 for the ProgressIndication
 * use31asPI_IE     - Uses tag value 31 for the ProgressIndication
 * use30_31asPI_IE  - Use both tags. ie - create them both
 ************************************************************************************/
typedef enum
{
    use30asPI_IE=1,
    use31asPI_IE=2,
    use30_31asPI_IE=3
} progressInd_IE;


/************************************************************************************
 *
 * cmCallProgressCreate
 *
 * Purpose: To create PROGRESS message and fill it with the specified parameters
 *
 * Input:   hsCall                  - The stack handle to the call
 *          display                 - The display field
 *          cause                   - The causeValue (-1 if not used)
 *          pi_IE                   - Indicator whether IE to use for progressIndicator IE
 *                                    30 , 31 or both
 *          progressDescription     - The value of the progressIndicator's progressDescription (-1 if not used)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if not used)
 *          param                   - The nonStandardParam to be put into the message (NULL if not used)
 *
 * Output:  None.
 *
 * Returned value: The root node ID of the created progress message on success
 *                 This root value should be passed to cmCallProgress() function.
 *                 Negative value on failure
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgressCreate(
        IN HCALL                hsCall,
        IN char*                display,
        IN int                  cause,
        IN progressInd_IE       pi_IE,
        IN int                  progressDescription,
        IN int                  notificationDescription,
        IN cmNonStandardParam*  param);


/************************************************************************************
 *
 * cmCallProgressGet
 *
 * Purpose: To decode PROGRESS message
 *
 * Input:   hApp                    - The stack instance handle
 *          message                 - Progress message to decode
 *          displaySize             - The display field size in bytes.
 *
 * Output:  display                 - The display field (user allocated memory)
 *          cause                   - The causeValue (-1 if absent)
 *          pi_IE                   - Indicator whether IE to use for progressIndicator IE
 *                                    30 , 31 or both
 *          progressDescription     - The value of the progressIndicator's progressDescription (-1 if absent)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if absent)
 *          param                   - The nonStandardParam from the message (param->length==RVERROR if it wasn't there)
 *          ** Any of the output fields can be passed as NULL if the
 *             application doesn't care about their values
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgressGet(
        IN  HAPP                hApp,
        IN  int                 message,
        OUT char*               display,
        IN  int                 displaySize,
        OUT int*                cause,
        OUT progressInd_IE*     pi_IE,
        OUT int*                progressDescription,
        OUT int*                notificationDescription,
        OUT cmNonStandardParam* param);


/************************************************************************************
 *
 * cmCallNotifyCreate
 *
 * Purpose: To create NOTIFY message and fill it with the specified parameters
 *
 * Input:   hsCall                  - The stack handle to the call
 *          display                 - The display field
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if not used)
 *          param                   - The nonStandardParam to be put into the message
 *
 * Output:  None.
 *
 * Returned value: The root node ID of the created notify message on success
 *                 This root value should be passed to cmCallNotify() function.
 *                 Negative value on failure
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallNotifyCreate(
        IN HCALL                hsCall,
        IN char*                display,
        IN int                  notificationDescription,
        IN cmNonStandardParam*  param);


/************************************************************************************
 *
 * cmCallNotifyGet
 *
 * Purpose: To decode NOTIFY message
 *
 * Input:   hApp                    - The stack instance handle
 *          message                 - Progress message to decode
 *          displaySize             - The display field size in bytes.
 *
 * Output:  display                 - The display field (user allocated memory)
 *          notificationDescription - The value of the notifyIndicator's notifyDescription (-1 if absent)
 *          param                   - The nonStandardParam from the message (param->length==RVERROR if it wasn't there)
 *          ** Any of the output fields can be passed as NULL if the
 *             application doesn't care about their values
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallNotifyGet(
        IN  HAPP                hApp,
        IN  int                 message,
        OUT char*               display,
        IN  int                 displaySize,
        OUT int*                notificationDescription,
        OUT cmNonStandardParam* param);


/************************************************************************************
 *
 * cmCallProgress
 *
 * Purpose: To send PROGRESS message
 *
 * Input:   hsCall                  - The stack handle to the call
 *          message                 - The pvt nodeId of the PROGRESS message to send
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgress(
                            IN HCALL    hsCall,
                            IN int      message);

/************************************************************************************
 *
 * cmCallNotify
 *
 * Purpose: To send NOTIFY message
 *
 * Input:   hsCall                  - The stack handle to the call
 *          message                 - The pvt nodeId of the NOTIFY message to send
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallNotify(
                            IN HCALL    hsCall,
                            IN int      message);


/********************************************************************************************
 * cmCallCreateAnnexLMessage
 * purpose : This function creates an Annex L message. It actually creates a PVT node of
 *           type StimulusControl, and allows setting the values of this type.
 * input   : hApp           - Stack instance handle
 *           isText         - TRUE if it's a text message
 *                            FALSE otherwise
 *                          - This is a field inside the structure of StimulusControl (=Annex L)
 *           h248Message    - The buffer of the Annex L message
 *                            NULL if not needed
 *           h248MessageLength  - Length of the message in bytes
 *           nonStandard    - Non standard data associated with the Annex L message
 *                            NULL if not needed
 * output  : none
 * return  : PVT node of the annex L message on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallCreateAnnexLMessage(
    IN HAPP                hApp,
    IN BOOL                isText,
    IN char*               h248Message,
    IN int                 h248MessageLength,
    IN cmNonStandardParam* nonStandard);


/********************************************************************************************
 * cmCallSendAnnexLMessage
 * purpose : This function sends an Annex L message on the network.
 * input   : hsCall     - Call to send the message in
 *           message    - PVT node of the message to send (of type StimulusControl)
 *           force      - TRUE if the message must be sent immediatly on a Facility message
 *                        FALSE if we want to send it on the next Q931 message
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSendAnnexLMessage(
    IN HCALL                hsCall,
    IN int                  message,
    IN BOOL                 force);


/********************************************************************************************
 * cmCallGetAnnexLMessage
 * purpose : This function checks the values of an Annex L message.
 *           Annex L messages are piggy-backed on top of Q931 messages, inside a field
 *           called StimulusControl.
 *           This function should be used when cmEvCallNewAnnexLMessage is called.
 * input   : hApp           - Stack instance handle
 *           nodeId         - PVT Node ID of the Annex L message. It's of type StimulusControl
 *           h248MessageLength  - Maximum length of the message buffer in bytes
 * output  : isText         - TRUE if it's a text message
 *                            FALSE otherwise
 *                          - This is a field inside the structure of StimulusControl (=Annex L)
 *           h248Message    - The buffer of the Annex L message
 *           h248MessageLength  - Length of the message in bytes
 *           nonStandard    - Non standard data associated with the Annex L message
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexLMessage(
    IN    HAPP                hApp,
    IN    int                 nodeId,
    OUT   BOOL*               isText,
    OUT   char*               h248Message,
    INOUT int*                h248MessageLength,
    OUT   cmNonStandardParam* nonStandard);




/********************************************************************************************
 * cmTunnelledProtocolID struct
 * Protocol identifier of a tunneled message. This structure is used for Annex M messages.
 * It comes to simplify the use of the TunnelledProtocol type in the ASN.1 definition.
 * tunnelledProtocolObjectID    - Object identifier of the tunneled protocol.
 *                                This is a NULL terminated string of the identifier.
 *                                Its length is 0 if the id of the tunneled protocol uses
 *                                an alternate ID and not an object ID.
 * protocolType                 - Type inside the alternate ID of the tunneled protocol.
 *                                Only used when tunnelledProtocolObjectID is of length 0.
 *                                This field is not NULL terminated.
 * protocolTypeLength           - Length of protocolType in bytes.
 *                                Only used when tunnelledProtocolObjectID is of length 0.
 * protocolVariant              - Variant inside the alternate ID of the tunneled protocol.
 *                                Only used when tunnelledProtocolObjectID is of length 0.
 *                                This field is not NULL terminated.
 * protocolVariantLength        - Length of protocolVariant in bytes.
 *                                Only used when tunnelledProtocolObjectID is of length 0.
 * subIdentifier                - Sub identifier of the tunneled protocol.
 * subIdentifierLength          - Length of subIdentifier in bytes
 *                                Doesn't exist in the TunneledProtocol type of set to 0.
 ********************************************************************************************/
typedef struct
{
    char tunnelledProtocolObjectID[128];
    char protocolType[64];
    int  protocolTypeLength;
    char protocolVariant[64];
    int  protocolVariantLength;
    char subIdentifier[64];
    int  subIdentifierLength;
} cmTunnelledProtocolID;



/********************************************************************************************
 * cmOctetString struct
 * This structure is used by some of the API functions of the CM. It allows setting several
 * octet strings at once, by using an array of this struct.
 * message  - message string buffer. If set to NULL, it indicates the last one in the
 *            array of such elements.
 * size     - Size of the message string
 ********************************************************************************************/
typedef struct
{
    char* message;
    int   size;
} cmOctetString;


/********************************************************************************************
 * cmCallCreateAnnexMMessage
 * purpose : This function creates an Annex M message. It actually creates a PVT node of
 *           type tunnelledSignallingMessage, and allows setting the values of this type.
 * input   : hApp               - Stack instance handle
 *           tunnellingRequired - TRUE if tunneling of the messages is required
 *                                FALSE otherwise
 *           tunnelledProtocolID- The identifier of the protocol being tunneled
 *                                This is a structure of type cmTunnelledProtocolID.
 *           messageContent     - The message being tunneled. It is an array of strings with
 *                                variable length. Last element in this array must point
 *                                to a NULL string.
 *           nonStandard        - Non standard data associated with the Annex M message
 * output  : none
 * return  : PVT node of the annex M message on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallCreateAnnexMMessage(
    IN HAPP                     hApp,
    IN BOOL                     tunnellingRequired,
    IN cmTunnelledProtocolID*   tunnelledProtocolID,
    IN cmOctetString*           messageContent,
    IN cmNonStandardParam*      nonStandard);


/********************************************************************************************
 * cmCallSendAnnexMMessage
 * purpose : This function sends an Annex M message on the network.
 * input   : hsCall     - Call to send the message in
 *           message    - PVT node of the message to send (of type tunnelledSignallingMessage)
 *           force      - TRUE if the message must be sent immediatly on a Facility message
 *                        FALSE if we want to send it on the next Q931 message
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSendAnnexMMessage(
    IN HCALL                hsCall,
    IN int                  message,
    IN BOOL                 force);


/********************************************************************************************
 * cmCallGetAnnexMMessage
 * purpose : This function checks the values of an Annex M message.
 *           Annex M messages are piggy-backed on top of Q931 messages, inside a field
 *           called tunnelledSignallingMessage.
 *           This function should be used when cmEvCallNewAnnexMMessage is called.
 *           To get the actual tunneled message, use cmCallGetAnnexMMessageContent
 * input   : hApp           - Stack instance handle
 *           nodeId         - PVT Node ID of the Annex M message.
 * output  : tunnellingRequired - TRUE if tunneling of the messages is required
 *                                FALSE otherwise
 *           tunnelledProtocolID- The identifier of the protocol being tunneled
 *                                This is a structure of type cmTunnelledProtocolID.
 *           nonStandard        - Non standard data associated with the Annex M message
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexMMessage(
    IN  HAPP                    hApp,
    IN  int                     nodeId,
    OUT BOOL*                   tunnellingRequired,
    OUT cmTunnelledProtocolID*  tunnelledProtocolID,
    OUT cmNonStandardParam*     nonStandard);


/********************************************************************************************
 * cmCallGetAnnexMMessageContent
 * purpose : This function checks the values of an Annex M message.
 *           Annex M messages are piggy-backed on top of Q931 messages, inside a field
 *           called tunnelledSignallingMessage.
 *           This function should be used when cmEvCallNewAnnexMMessage is called.
 *           To get more information about the specific message being tunneled, use
 *           cmCallGetAnnexMMessage.
 * input   : hApp                   - Stack instance handle
 *           nodeId                 - PVT Node ID of the Annex M message.
 *           index                  - Specific tunneled message to get (1-based)
 *           messageContentLength   - Maximum length of buffer in bytes
 * output  : messageContentLength   - Length of message
 *           messageContent         - The message buffer itself
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexMMessageContent(
    IN    HAPP                hApp,
    IN    int                 nodeId,
    IN    int                 index,
    INOUT int*                messageContentLength,
    OUT   char*               messageContent);


/********************************************************************************************
 * cmCallSimulateMessage
 * purpose : "Push" a message into the stack as if it was received from the network.
 * input   : hsCall     - Call the message is received on
 *           message    - PVT node ID of the received message
 *                        This node must be a Q931 or H245 message.
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSimulateMessage(
                         IN HCALL               hsCall,
                         IN int                 message);


/************************************************************************
 * cmMeiEnter
 * purpose: Enters critical section for the specified stack instance.
 * input  : hApp        - Stack's application handle
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMeiEnter(IN HAPP hApp);


/************************************************************************
 * cmMeiExit
 * purpose: Exits critical section for the specified stack instance.
 * input  : hApp        - Stack's application handle
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMeiExit(IN HAPP hApp);


/************************************************************************
 * cmThreadAttach
 * purpose: Indicate that the current running thread can be used to catch
 *          events from the network.
 *          Threads that called cmInitialize() don't have to use this
 *          function.
 *          Although this function has hApp as a parameter, the result
 *          will take place for all stack instances
 * input  : hApp        - Stack's application handle
 *          threadId    - Unused
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmThreadAttach(HAPP hApp,UINT32 threadId);


/************************************************************************
 * cmThreadDetach
 * purpose: Indicate that the current running thread cannot be used to catch
 *          events from the network.
 *          Although this function has hApp as a parameter, the result
 *          will take place for all stack instances
 * input  : hApp        - Stack's application handle
 *          threadId    - Unused
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmThreadDetach(HAPP hApp,UINT32 threadId);

/************************************************************************
 * cmLogMessage
 * purpose: Writes a user message into the log file under the APPL filter
 * input  : hApp        - Stack's application handle
 *          line        - The message to be printed (must be null-terminated)
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmLogMessage(IN HAPP hApp, IN const char *line);


/************************************************************************************
 *
 * MIB Related
 * This part handles the communication between the MIB module of the stack and the
 * SNMP application (applSnmp).
 * Applications that want to use applSnmp should NOT use this API functions, as they
 * are used by applSnmp.
 *
 ************************************************************************************/

DECLARE_OPAQUE(HMibHandleT);

typedef int (*h341AddNewCallT )(HMibHandleT mibHandle,HCALL hsCall);
typedef void (*h341DeleteCallT  )(HMibHandleT mibHandle,HCALL hsCall);

typedef int (*h341AddControlT  )(HMibHandleT mibHandle,HCALL hsCall);
typedef void (*h341DeleteControlT  )(HMibHandleT mibHandle,HCALL hsCall);

typedef int (*h341AddNewLogicalChannelT  )(HMibHandleT mibHandle,HCHAN hsChan);
typedef void (*h341DeleteLogicalChannelT )(HMibHandleT mibHandle,HCHAN hsChan);


typedef struct
{
    h341AddNewCallT h341AddNewCall;
    h341DeleteCallT h341DeleteCall;
    h341AddControlT h341AddControl;
    h341DeleteControlT h341DeleteControl;
    h341AddNewLogicalChannelT h341AddNewLogicalChannel;
    h341DeleteLogicalChannelT h341DeleteLogicalChannel;
} MibEventT;


/************************************************************************
 * cmMibEventSet
 * purpose: Set MIB notifications from the stack to an SNMP application
 * input  : hApp        - Stack's application handle
 *          mibEvent    - Event callbacks to set
 *          mibHandle   - Handle of MIB instance for the stack
 * output : none
 * return : Non-negative on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmMibEventSet(IN HAPP hApp, IN MibEventT* mibEvent, IN HMibHandleT mibHandle);



#endif

#ifdef __cplusplus
}
#endif





