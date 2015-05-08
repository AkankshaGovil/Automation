
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CMRAS_H
#define CMRAS_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef CM_H
#error cmras.h included improperly. Use #include <cm.h> instead.
#endif

#include <cm.h>



/************************************************************************
 * Definitions for backward compatibility
 ************************************************************************/
#define cmRASTrPart                     cmRASTrStage

#define cmRASCallType                   cmCallType
#define cmRASCallTypeP2P                cmCallTypeP2P
#define cmRASCallTypeOne2N              cmCallTypeOne2N
#define cmRASCallTypeN2One              cmCallTypeN2One
#define cmRASCallTypeN2Nw               cmCallTypeN2Nw

#define cmRASCallModelTypeDirect        cmCallModelTypeDirect
#define cmRASCallModelTypeGKRouted      cmCallModelTypeGKRouted
#define cmRASCallModelType              cmCallModelType

#define cmRASRouteSize                  cmRouteSize
#define cmRASAliasType                  cmAliasType
#define cmRASAlias                      cmAlias
#define cmRASTransportType              cmTransportType
#define cmRASTransport                  cmTransportAddress

#define cmRASTransportTypeIP            cmTransportTypeIP /* No route */
#define cmRASTransportTypeIPStrictRoute cmTransportTypeIPStrictRoute
#define cmRASTransportTypeIPLooseRoute  cmTransportTypeIPLooseRoute

#define cmRASAliasTypeE164              cmAliasTypeE164
#define cmRASAliasTypeH323ID            cmAliasTypeH323ID
#define cmRASAliasTypeEndpointID        cmAliasTypeEndpointID
#define cmRASAliasTypeGatekeeperID      cmAliasTypeGatekeeperID

#define cmRASEndpointType               cmEndpointType
#define cmRASEndpointTypeTerminal       cmEndpointTypeTerminal
#define cmRASEndpointTypeGateway        cmEndpointTypeGateway
#define cmRASEndpointTypeMCU            cmEndpointTypeMCU
#define cmRASEndpointTypeGK             cmEndpointTypeGK
#define cmRASEndpointTypeUndefined      cmEndpointTypeUndefined
#define cmRASEndpointTypeSET            cmEndpointTypeSET

#define cmRASTransportQOSNotSupported   cmRASReasonTransportQOSNotSupported
#define cmRASInvalidAlias               cmRASReasonInvalidAlias
#define cmRASPermissionDenied           cmRASReasonPermissionDenied
#define cmRASQOSControlNotSupported     cmRASReasonQOSControlNotSupported
#define cmRASIncompleteAddress          cmRASReasonIncompleteAddress
#define cmRASFullRegistrationRequired   cmRASReasonFullRegistrationRequired
#define cmRASRouteCallToSCN             cmRASReasonRouteCallToSCN
#define cmRASAliasesInconsistent        cmRASReasonAliasesInconsistent




/************************************************************************
 * cmRASTransaction enum:
 * Indicates the type of RAS transaction we're dealing with.
 ************************************************************************/
typedef enum
{
    cmRASGatekeeper=1,          /* GRQ transaction */
    cmRASRegistration,          /* RRQ transaction */
    cmRASUnregistration,        /* URQ transaction */
    cmRASAdmission,             /* ARQ transaction */
    cmRASDisengage,             /* DRQ transaction */
    cmRASBandwidth,             /* BRQ transaction */
    cmRASLocation,              /* LRQ transaction */
    cmRASInfo,                  /* IRQ-IRR transaction */
    cmRASNonStandard,           /* NSM */
    cmRASUnknown,               /* XRS */
    cmRASResourceAvailability,  /* RAI-RAC transaction */
    cmRASUnsolicitedIRR,        /* Unsolicited IRR */
    cmRASServiceControl,        /* SCI-SCR transaction */
    cmRASMaxTransaction
} cmRASTransaction;


/************************************************************************
 * cmRASReason enum:
 * Indicates the reject reason of the transaction. We use a single
 * enumeration for all possible reasons in Reject messages.
 ************************************************************************/
typedef enum
{
    cmRASReasonResourceUnavailable,             /* GRJ, RRJ, ARJ, LRJ - gatekeeper resources exhausted */
    cmRASReasonInsufficientResources,           /* BRJ */
    cmRASReasonInvalidRevision,                 /* GRJ, RRJ, BRJ */
    cmRASReasonInvalidCallSignalAddress,        /* RRJ */
    cmRASReasonInvalidRASAddress,               /* RRJ - supplied address is invalid */
    cmRASReasonInvalidTerminalType,             /* RRJ */
    cmRASReasonInvalidPermission,               /* ARJ - permission has expired */
                                                /* BRJ - true permission violation */
                                                /* LRJ - exclusion by administrator or feature */
    cmRASReasonInvalidConferenceID,             /* BRJ - possible revision */
    cmRASReasonInvalidEndpointID,               /* ARJ */
    cmRASReasonCallerNotRegistered,             /* ARJ */
    cmRASReasonCalledPartyNotRegistered,        /* ARJ - can't translate address */
    cmRASReasonDiscoveryRequired,               /* RRJ - registration permission has aged */
    cmRASReasonDuplicateAlias,                  /* RRJ - alias registered to another endpoint */
    cmRASReasonTransportNotSupported,           /* RRJ - one or more of the transports */
    cmRASReasonCallInProgress,                  /* URJ */
    cmRASReasonRouteCallToGatekeeper,           /* ARJ */
    cmRASReasonRequestToDropOther,              /* DRJ - can't request drop for others */
    cmRASReasonNotRegistered,                   /* DRJ, LRJ, INAK - not registered with gatekeeper */
    cmRASReasonUndefined,                       /* GRJ, RRJ, URJ, ARJ, BRJ, LRJ, INAK */
    cmRASReasonTerminalExcluded,                /* GRJ - permission failure, not a resource failure */
    cmRASReasonNotBound,                        /* BRJ - discovery permission has aged */
    cmRASReasonNotCurrentlyRegistered,          /* URJ */
    cmRASReasonRequestDenied,                   /* ARJ - no bandwidrg available */
                                                /* LRJ - cannot find location */
    cmRASReasonLocationNotFound,                /* LRJ - cannot find location */
    cmRASReasonSecurityDenial,                  /* GRJ, RRJ, URJ, ARJ, BRJ, LRJ, DRJ, INAK */
    cmRASReasonTransportQOSNotSupported,        /* RRJ */
    cmRASResourceUnavailable,                   /* Same as cmRASReasonResourceUnavailable */
    cmRASReasonInvalidAlias,                    /* RRJ - alias not consistent with gatekeeper rules */
    cmRASReasonPermissionDenied,                /* URJ - requesting user not allowed to unregister specified user */
    cmRASReasonQOSControlNotSupported,          /* ARJ */
    cmRASReasonIncompleteAddress,               /* ARJ, LRJ */
    cmRASReasonFullRegistrationRequired,        /* RRJ - registration permission has expired */
    cmRASReasonRouteCallToSCN,                  /* ARJ, LRJ */
    cmRASReasonAliasesInconsistent,             /* ARJ, LRJ - multiple aliases in request identify distinct people */
    cmRASReasonAdditiveRegistrationNotSupported,/* RRJ */
    cmRASReasonInvalidTerminalAliases,          /* RRJ */
    cmRASReasonExceedsCallCapacity,             /* ARJ - destination does not have the capacity for this call */
    cmRASReasonCollectDestination,              /* ARJ */
    cmRASReasonCollectPIN,                      /* ARJ */
    cmRASReasonGenericData,                     /* GRJ, RRJ, ARJ, LRJ */
    cmRASReasonNeededFeatureNotSupported,       /* GRJ, RRJ, ARJ, LRJ */
    cmRASReasonUnknownMessageResponse,          /* XRS message was received for the request */
    cmRASReasonHopCountExceeded                 /* LRJ */

} cmRASReason;


/************************************************************************
 * cmRASDisengageReason enum:
 * Indicates the reason given in the DRQ message (DisengageRequest).
 ************************************************************************/
typedef enum
{
    cmRASDisengageReasonForcedDrop,
    cmRASDisengageReasonNormalDrop,
    cmRASDisengageReasonUndefinedReason
}  cmRASDisengageReason;


/************************************************************************
 * cmRASUnregReason enum:
 * Indicates the reason given in the URQ message (UnregistrationRequest).
 ************************************************************************/
typedef enum
{
    cmRASUnregReasonReregistrationRequired,
    cmRASUnregReasonTtlExpired,
    cmRASUnregReasonSecurityDenial,
    cmRASUnregReasonUndefinedReason,
    cmRASUnregReasonMaintenance
}  cmRASUnregReason;


/************************************************************************
 * cmRASIrrXXX
 * These definitions are part of the irrStatus field in the IRR message,
 * added on version 4.
 * The segment field is not part of these definition, since the values
 * 0-65535 can be used for each of the segments as a value.
 ************************************************************************/
#define cmRASIrrComplete    (-1)
#define cmRASIrrIncomplete  (-2)
#define cmRASIrrInvalidCall (-3)



/************************************************************************
 * cmRASTrStage enum:
 * Indicates the transaction state we want to use in the API function.
 * cmRASTrStageRequest  - Request message information
 * cmRASTrStageConfirm  - Confirm message information
 * cmRASTrStageReject   - Reject message information
 * cmRASTrStageTimeout  - Time-out occurred
 * cmRASTrStateProgress - RIP message information
 * Others               - unused
 ************************************************************************/
typedef enum
{
    cmRASTrStageRequest=1,
    cmRASTrStageConfirm,
    cmRASTrStageReject,
    cmRASTrStageTimeout,
    cmRASTrStageBegin,
    cmRASTrStageEnd,
    cmRASTrStageCreate,
    cmRASTrStageDummy,

    cmRASTrStageProgress,


    cmRASTrPartRequest=1, /* for backward compatability */
    cmRASTrPartConfirm,
    cmRASTrPartReject,
    cmRASTrPartTimeout

} cmRASTrStage;





/************************************************************************
 * cmRASParam enum:
 * Indicates the parameter we want to set/get from a RAS message
 ************************************************************************/
typedef enum
{
    cmRASParamGatekeeperID,                 /* cmAlias                          */
    cmRASParamRASAddress,                   /* cmTransportAddress               */
    cmRASParamCallSignalAddress,            /* cmTransportAddress               */
    cmRASParamEndpointType,                 /* cmEndpointType                   */
    cmRASParamTerminalType,                 /* cmEndpointType                   */
    cmRASParamEndpointAlias,                /* cmAlias                          */
    cmRASParamTerminalAlias,                /* cmAlias                          */
    cmRASParamDiscoveryComplete,            /* BOOL                             */
    cmRASParamEndpointVendor,               /* cmVendor                         */
    cmRASParamCallType,                     /* cmCallType                       */
    cmRASParamCallModel,                    /* cmCallModelType                  */
    cmRASParamEndpointID,                   /* cmAlias                          */
    cmRASParamDestInfo,                     /* cmAlias                          */
    cmRASParamSrcInfo,                      /* cmAlias                          */
    cmRASParamDestExtraCallInfo,            /* cmAlias                          */
    cmRASParamDestCallSignalAddress,        /* cmTransportAddress               */
    cmRASParamSrcCallSignalAddress,         /* cmTransportAddress               */
    cmRASParamBandwidth,                    /* int                              */
    cmRASParamActiveMC,                     /* BOOL                             */
    cmRASParamAnswerCall,                   /* BOOL                             */
    cmRASParamIrrFrequency,                 /* int                              */
    cmRASParamReplyAddress,                 /* cmTransportAddress               */
    cmRASParamDisengageReason,              /* cmRASDisengageReason             */
    cmRASParamRejectedAlias,                /* cmAlias                          */
    cmRASParamRejectReason,                 /* cmRASReason                      */
    cmRASParamCID,                          /* char[16]                         */
    cmRASParamDestinationIpAddress,         /* cmTransportAddress: the actual
                                               address to send message to       */
    cmRASParamNonStandard,                  /* cmNonStandardParam               */
    cmRASParamNonStandardData,              /* int - pvt nodeId                 */
    cmRASParamCRV,                          /* int                              */
    cmRASParamMulticastTransaction,         /* BOOL: is this a multicast
                                               transaction?                     */
    cmRASParamTransportQOS,                 /* cmTransportQOS                   */
    cmRASParamKeepAlive,                    /* BOOL                             */
    cmRASParamTimeToLive,                   /* int                              */
    cmRASParamDelay,                        /* int (in seconds)                 */
    cmRASParamCallID,                       /* char[16]                         */
    cmRASParamAnsweredCall,                 /* BOOL                             */
    cmRASParamAlmostOutOfResources,         /* BOOL                             */
    cmRASParamAlternateGatekeeper,          /* cmAlternateGatekeeper            */
    cmRASParamAltGKInfo,                    /* cmAlternateGatekeeper            */
    cmRASParamAltGKisPermanent,             /* BOOL                             */
    cmRASParamEmpty,                        /* none                             */
    cmRASParamSourceInfo,                   /* cmAlias                          */
    cmRASParamNeedResponse,                 /* BOOL                             */
    cmRASParamMaintainConnection,           /* BOOL                             */
    cmRASParamMultipleCalls,                /* BOOL                             */
    cmRASParamWillRespondToIRR,             /* BOOL                             */
    cmRASParamSupportsAltGk,                /* BOOL                             */
    cmRASParamAdditiveRegistration,         /* BOOL                             */
    cmRASParamSupportsAdditiveRegistration, /* BOOL                             */
    cmRASParamSegmentedResponseSupported,   /* BOOL                             */
    cmRASParamNextSegmentRequested,         /* int                              */
    cmRASParamCapacityInfoRequested,        /* BOOL                             */
    cmRASParamHopCount,                     /* int                              */
    cmRASParamInvalidTerminalAlias,         /* cmAlias                          */
    cmRASParamUnregReason,                  /* cmRASUnregReason                 */
    cmRASParamIrrStatus,                    /* int, cmRASIrrComplete,
                                               cmRASIrrIncomplete,
                                               cmRASIrrInvalidCall              */
    cmRASParamCallHandle,                   /* HCALL - set in svalue: the call
                                               handle of this transaction       */
	cmRASParamSocketDesc,
    cmRASParamLast
} cmRASParam;




typedef struct
{
    cmTransportAddress  rasAddress;
    cmRASAlias          gatekeeperIdentifier;
    BOOL                needToRegister;
    BYTE                priority;
}cmAlternateGatekeeper;

typedef enum
{
    cmRASErrorParamNotFound
} cmRASError;





/* Application's handle of a RAS transaction */
DECLARE_OPAQUE(HAPPRAS);

/* Stack's handle of a RAS transaction */
DECLARE_OPAQUE(HRAS);




/************************************************************************
 *
 *                              API functions
 *
 ************************************************************************/

/************************************************************************
 * cmRASStartTransaction
 * purpose: Create a RAS transaction.
 *          This function exchanges handles with the application and connects
 *          between the transaction and the call (if applicable).
 * input  : hApp        - Application's handle for a stack instance
 *          haRas       - Application's handle for the RAS transaction
 *          transaction - The transaction type we want to start
 *          destAddress - Address of the destination.
 *                        If set to NULL, then it's for the gatekeeper
 *          hsCall      - Stack's call handle if the transaction is related
 *                        to a call. NULL otherwise.
 * output : lphsRas     - The stack's RAS transaction handle that was
 *                        created.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASStartTransaction(
    IN  HAPP             hApp,
    IN  HAPPRAS          haRas,
    OUT LPHRAS           lphsRas,
    IN  cmRASTransaction transaction,
    IN  cmRASTransport*  destAddress,
    IN  HCALL            hsCall);


/************************************************************************
 * cmRASSetHandle
 * purpose: Sets or changes the application handle for a RAS transaction.
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          haRas   - Application's handle for the RAS transaction to be set
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetHandle(
    IN  HRAS            hsRas,
    IN  HAPPRAS         haRas);


/************************************************************************
 * cmRASGetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - If the parameter value is a structure, the value
 *                    represents the length of the parameter.
 * output : value   - For a BOOL or int - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself. Can be set to NULL if we're only interested
 *                    in its length.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetParam(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  OUT INT32*       value, /* value or length */
    IN  char*            svalue);


/************************************************************************
 * cmRASSetParam
 * purpose: Set a parameter about the RAS transaction
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - For a BOOL or int - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetParam(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  INT32            value,
    IN  char*            svalue);


/************************************************************************
 * cmRASGetNumOfParams
 * purpose: Returns the number of params in sequences on the property
 *          tree.
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 * output : none
 * return : Number of params in sequence on the property tree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetNumOfParams(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param);


/************************************************************************
 * cmRASRequest
 * purpose: Send an outgoing RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASRequest(
    IN  HRAS             hsRas);


/************************************************************************
 * cmRASDummyRequest
 * purpose: Called after cmRASStartTransaction() on cmRASUnsolicitedIRR.
 *          It allows the application to wait for an unsolicited IRR on
 *          a specific call.
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASDummyRequest(
    IN  HRAS         hsRas);


/************************************************************************
 * cmRASConfirm
 * purpose: Sends a confirm response on an incoming RAS request
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASConfirm(
    IN  HRAS             hsRas);


/************************************************************************
 * cmRASReject
 * purpose: Sends a reject response on an incoming RAS request
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          reason      - The reject reason to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASReject(
    IN  HRAS             hsRas,
    IN  cmRASReason      reason);


/************************************************************************
 * cmRASInProgress
 * purpose: Sends a RIP (ReplyInProgress) message on a transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          delay       - Delay to use in RIP message (in milliseconds)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASInProgress(
    IN  HRAS         hsRas,
    IN  int          delay);


/************************************************************************
 * cmRASClose
 * purpose: Close a RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASClose(
    IN  HRAS             hsRas);






/************************************************************************
 * cmRASGetHandle
 * purpose: Returns the stack's handle of the transaction from the
 *          application's handle.
 *          This function is slow and should not be used frequently
 * input  : hApp        - Application's handle for a stack instance
 *          haRas       - Application's handle for the RAS transaction
 * output : lphsRas     - The stack's RAS transaction handle
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetHandle(
    IN  HAPP    hApp,
    IN  HAPPRAS haRas,
    OUT LPHRAS  hsRas);


/************************************************************************
 * cmRASGetTransaction
 * purpose: Returns the type of RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : transaction - The type of transaction
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetTransaction(
    IN  HRAS                hsRas,
    OUT cmRASTransaction*   transaction);


/************************************************************************
 * cmRASGetLastError
 * purpose: This function does absolutly nothing.
 *          It is only here for backward compatibility.
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : 0
 ************************************************************************/
RVAPI
cmRASError RVCALLCONV cmRASGetLastError(
    IN  HRAS             hsRas);









/************************************************************************
 *
 *                           CALLBACK functions
 *
 ************************************************************************/

/************************************************************************
 * cmEvRASRequestT
 * purpose: Callback function that the RAS calls when a new incoming
 *          RAS request has to be handled
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *                        Set to NULL if not applicable for this type
 *                        of transaction
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle (if applicable)
 * output : lphsRas     - The application's RAS transaction handle
 *                        This should be set by the application to find
 *                        this transaction in future callbacks.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRASRequestT)(
    IN  HRAS                hsRas,
    IN  HCALL               hsCall,
    OUT LPHAPPRAS           lphaRas,
    IN  cmRASTransaction    transaction,
    IN  cmTransportAddress* srcAddress,
    IN  HAPPCALL            haCall);


/************************************************************************
 * cmEvRASConfirmT
 * purpose: Callback function that the RAS calls when a confirm on a
 *          RAS request is received
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRASConfirmT)(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas);


/************************************************************************
 * cmEvRASRejectT
 * purpose: Callback function that the RAS calls when a reject on a
 *          RAS request is received
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 *          reason      - Reject reason
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRASRejectT)(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASReason      reason);


/************************************************************************
 * cmEvRASTimeoutT
 * purpose: Callback function that the RAS calls when a timeout on a
 *          RAS request occured.
 *          This will be the case when no reply arrives or when an
 *          outgoing multicast transaction is being handled (even if
 *          replies arrived).
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvRASTimeoutT)(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas);



/************************************************************************
 * CMRASEVENT struct
 * This struct holds the event handlers of the RAS.
 * There are 4 callback functions in this struct:
 * cmEvRASRequest   - Notifies the application of a new incoming
 *                    transaction
 * cmEvRASConfirm   - Notifies the application of a confirm reply on an
 *                    application's request
 * cmEvRASReject    - Notifies the application of a reject reply on an
 *                    application's request
 * cmEvRASTimeout   - Notifies the application of a timed-out request
 ************************************************************************/
typedef struct
{
    cmEvRASRequestT cmEvRASRequest;
    cmEvRASConfirmT cmEvRASConfirm;
    cmEvRASRejectT  cmEvRASReject;
    cmEvRASTimeoutT cmEvRASTimeout;
} SCMRASEVENT,*CMRASEVENT;


/************************************************************************
 * cmRASSetEventHandler
 * purpose: Sets the callbacks the application wishes to use
 *          This callback is used to set the manual RAS events.
 * input  : hApp        - Application's handle for a stack instance
 *          cmRASEvent  - RAS callbacks to set
 *          size        - Size of callbacks struct (*CMRASEVENT)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetEventHandler(
    IN  HAPP        hApp,
    IN  CMRASEVENT  cmRASEvent,
    IN  int         size);





/************************************************************************
 *
 *                    Automatic RAS CALLBACK functions
 *
 ************************************************************************/

/************************************************************************
 * cmEvAutoRASRequestT
 * purpose: Callback function that the RAS calls when a new incoming
 *          automatic RAS request has to be handled.
 *          No action can be taken by the application - this is only
 *          for notification purposes.
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *                        Set to NULL if not applicable for this type
 *                        of transaction
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle (if applicable)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvAutoRASRequestT)(
    IN  HRAS             hsRas,
    IN  HCALL            hsCall,
    IN  cmRASTransaction transaction,
    IN  cmRASTransport*  srcAddress,
    IN  HAPPCALL         haCall);


/************************************************************************
 * cmEvAutoRASConfirmT
 * purpose: Callback function that the RAS calls when a confirm on an
 *          Automatic RAS request is received
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvAutoRASConfirmT)(
    IN  HRAS             hsRas);


/************************************************************************
 * cmEvAutoRASRejectT
 * purpose: Callback function that the RAS calls when a reject on an
 *          Automatic RAS request is received
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          reason      - Reject reason
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvAutoRASRejectT)(
    IN  HRAS             hsRas,
    IN  cmRASReason      reason);


/************************************************************************
 * cmEvAutoRASTimeoutT
 * purpose: Callback function that the RAS calls when a timeout on an
 *          Automatic RAS request occured.
 *          This will be the case when no reply arrives or when an
 *          outgoing multicast transaction is being handled (even if
 *          replies arrived).
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
typedef int (RVCALLCONV *cmEvAutoRASTimeoutT)(
    IN  HRAS             hsRas);



/************************************************************************
 * CMAUTORASEVENT struct
 * This struct holds the event handlers of the Automatic RAS.
 * There are 4 callback functions in this struct:
 * cmEvAutoRASRequest   - Notifies the application of a new incoming
 *                        transaction that is handled by the automatic RAS
 * cmEvAutoRASConfirm   - Notifies the application of a confirm reply on an
 *                        automatic RAS request
 * cmEvAutoRASReject    - Notifies the application of a reject reply on an
 *                        automatic RAS s request
 * cmEvAutoRASTimeout   - Notifies the application of a timed-out on an
 *                        automatic RAS request
 ************************************************************************/
typedef struct
{
    cmEvAutoRASRequestT cmEvAutoRASRequest;
    cmEvAutoRASConfirmT cmEvAutoRASConfirm;
    cmEvAutoRASRejectT  cmEvAutoRASReject;
    cmEvAutoRASTimeoutT cmEvAutoRASTimeout;
} SCMAUTORASEVENT,*CMAUTORASEVENT;


/************************************************************************
 * cmAutoRASSetEventHandler
 * purpose: Sets the callbacks the application wishes to use for automatic
 *          RAS. Catching these callbacks allows the application to
 *          know about the messages that the automatic RAS receives.
 *          It doesn't allow the application to act on them - only to
 *          know about them.
 * input  : hApp            - Application's handle for a stack instance
 *          cmAutoRASEvent  - Automatic RAS callbacks to set
 *          size            - Size of callbacks struct (*CMRASEVENT)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmAutoRASSetEventHandler(
    IN  HAPP            hApp,
    IN  CMAUTORASEVENT  cmAutoRASEvent,
    IN  int             size);



#ifdef __cplusplus
}
#endif


#endif  /* CMRAS_H */
