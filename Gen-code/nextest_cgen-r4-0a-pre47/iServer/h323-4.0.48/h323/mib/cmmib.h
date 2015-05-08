
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef __CMMIB_H
#define __CMMIB_H

#ifdef __cplusplus
extern "C" {
#endif


#include <rvcommon.h>
#include <cm.h>


DECLARE_OPAQUE(HSTATISTIC);

typedef enum
{
    enumcallSignalStatsAlertingMsgsIn,
    enumcallSignalStatsAlertingMsgsOut,
    enumcallSignalStatsCallProceedingsIn,
    enumcallSignalStatsCallProceedingsOut,
    enumcallSignalStatsCallConnectionsIn,
    enumcallSignalStatsCallConnectionsOut,

    enumcallSignalStatsSetupMsgsIn,
    enumcallSignalStatsSetupMsgsOut,
    enumcallSignalStatsSetupAckMsgsIn,
    enumcallSignalStatsSetupAckMsgsOut,
    enumcallSignalStatsProgressMsgsIn,
    enumcallSignalStatsProgressMsgsOut,
    enumcallSignalStatsReleaseCompleteMsgsIn,
    enumcallSignalStatsReleaseCompleteMsgsOut,
    enumcallSignalStatsStatusMsgsIn,
    enumcallSignalStatsStatusMsgsOut,
    enumcallSignalStatsStatusInquiryMsgsIn,
    enumcallSignalStatsStatusInquiryMsgsOut,
    enumcallSignalStatsFacilityMsgsIn,
    enumcallSignalStatsFacilityMsgsOut,
    enumcallSignalStatsInfoMsgsIn,
    enumcallSignalStatsInfoMsgsOut,
    enumcallSignalStatsNotifyMsgsIn,
    enumcallSignalStatsNotifyMsgsOut,

    enumcallSignalStatsAverageCallDuration,/*not Implemented*/
    enumh245ControlChannelNumberOfListenPorts,/*not Implemented*/
    enumh245ControlChannelMaxConnections,/*not Implemented*/
    enumh245ControlChannelNumberOfListenFails,/*not Implemented*/
    enumh245ControlChannelNumberOfActiveConnections,/*not Implemented*/
    enumh245ControlChannelMasterSlaveMaxRetries,/*not Implemented*/
    enumh245ControlChannelConnectionAttemptsFail,/*not Implemented*/
    enumh245ControlChannelNumberOfTunnels,/*not Implemented*/

    enumh245ControlChanneMasterSlavelDeterminations,
    enumh245ControlChannelMasterSlaveAcks,
    enumh245ControlChannelMasterSlaveRejects,
    enumh245ControlChannelMasterSlaveReleases ,
    enumh245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects ,


    enumh245ControlChannelMasterSlaveMSDRejects ,/*not Implemented*/
    enumh245ControlChannelMasterSlaveT106Rejects,       /*not Implemented*/
    enumh245ControlChannelMasterSlaveMaxCounterRejects,/*not Implemented*/



    enumh245CapExchangeSets,
    enumh245CapExchangeAcks,
    enumh245CapExchangeRejects,
    enumh245CapExchangeReleases,

    enumh245CapExchangeRejectUnspecified,
    enumh245CapExchangeRejectUndefinedTableEntryUsed,
    enumh245CapExchangeRejectDescriptorCapacityExceeded,
    enumh245CapExchangeRejectTableEntryCapacityExeeded,


    enumh245LogChanOpenLogChanTotalRequests,
    enumh245LogChanOpenLogChanAcks,
    enumh245LogChanOpenLogChanConfirms,
    enumh245LogChanOpenLogChanRejects,



    enumh245LogChanOpenLogChanRejectUnspecified,
    enumh245LogChanOpenLogChanRejectUnsuitableReverseParameters,
    enumh245LogChanOpenLogChanRejectDataTypeNotSupported,
    enumh245LogChanOpenLogChanRejectDataTypeNotAvailable,
    enumh245LogChanOpenLogChanRejectUnknownDataType,
    enumh245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported,
    enumh245LogChanOpenLogChanRejectMulticastChannelNotAllowed,
    enumh245LogChanOpenLogChanRejectInsuffientBandwdith,
    enumh245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed,
    enumh245LogChanOpenLogChanRejectInvalidSessionID,
    enumh245LogChanOpenLogChanRejectMasterSlaveConflict,
    enumh245LogChanOpenLogChanRejectWaitForCommunicationMode,
    enumh245LogChanOpenLogChanRejectInvalidDependentChannel,
    enumh245LogChansOpenLogChanRejectReplacementForRejected,

    enumh245LogChanCloseLogChannels,
    enumh245LogChanCloseLogChanAcks,
    enumh245LogChanCloseLogChanRequests,
    enumh245LogChanCloseLogChanRequestsAcks,
    enumh245LogChanCloseLogChanRequestRejects,
    enumh245LogChanCloseLogChanRequestReleases
}mibStatisticParamEnumT;

typedef enum
{
    callInitiated=1,
    callDelivered,
    callPresent,
    callReceived,
    connectRequest,
    cscallProceeding,
    active,
    disconnectRequest,
    disconnectIndication,
    releaseRequest

}callStateT;

typedef enum
{
    enumh245ControlChannelMSDState,
    enumh245ControlChannelTerminalType,
    enumh245ControlChannelNumberOfMSDRetries,
    enumh245ControlChannelIsTunneling,

    enumh245CapExchangeState,
    enumh245CapExchangeProtocolId,
    enumh245CapExchangeRejectCause,
    enumh245CapExchangeMultiplexCapability,
    enumh245CapExchangeCapability,
    enumh245CapExchangeCapabilityDescriptors,

    enumh245LogChannelsChannelState,
    enumh245LogChannelsMediaTableType,
    enumh245LogChannelsSessionId,
    enumh245LogChannelsAssociateSessionId,
    enumh245LogChannelsMediaChannel,
    enumh245LogChannelsMediaGuaranteedDelivery,
    enumh245LogChannelsMediaControlChannel,
    enumh245LogChannelsMediaControlGuaranteedDelivery,
    enumh245LogChannelsSilenceSuppression,
    enumh245LogChannelsDestination,
    enumh245LogChannelsDynamicRTPPayloadType,
    enumh245LogChannelsH261aVideoPacketization,
    enumh245LogChannelsRTPPayloadDescriptor,
    enumh245LogChannelsRTPPayloadType,
    enumh245LogChannelsTransportCapability,
    enumh245LogChannelsRedundancyEncoding,
    enumh245LogChannelsSrcTerminalLabel

}mibControlParamTypeT;

typedef enum
{
    enumconnectionsState,
    enumconnectionsFastCall,
    enumconnectionsH245Tunneling
}mibCallParamTypeT;

typedef enum
{
    msdseOutgoingWaitingResponse = 1,
    msdseIncomingWaitingResponse,
    msdseIdle,
    msdseMaster,
    msdseSlave

}msdseStateT;

typedef enum
{
    ceseSent=1,
    ceseAck,
    ceseReject,
    ceseRelease

}ceseStateT;

typedef enum
{
    lcseAwaitingEstablishment=1,
    lcseEstablished,
    lcseAwaitingRelease,
    lcseAwaitingConfirmation
}lcseStateT;



/************************************************************************
 * mibGetStatistic
 * purpose: Get the value of a statistics parameter for the MIB
 * input  : hApp    - Stack's application handle
 *          type    - Parameter type to get
 * output : none
 * return : Parameter's value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetStatistic(IN HAPP hApp, IN mibStatisticParamEnumT type);


/************************************************************************
 * mibGetCallParameters
 * purpose: Get call related MIB parameters
 * input  : hsCall      - Call to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetCallParameters(
    IN  HCALL               hsCall,
    IN  mibCallParamTypeT   type,
    OUT int*                valueSize,
    OUT UINT8*              value);


/************************************************************************
 * cmCallGetH245Address
 * purpose: Get H245 addresses for a call
 * input  : hsCall      - Call to check
 * output : trSrc       - Source H245 address
 *          trDest      - Destination H245 address
 *          isConnected - Indication if H245 channel is connected or not
 * return : Non-negative value on success
 *          Negative value on failure
 * Note: If H.245 is not connected through a dedicated connection (i.e
 *       uses fast start or tunneling), then the returned addresses are
 *       the Q931 addresses
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallGetH245Address(
    IN  HCALL               hsCall,
    OUT cmTransportAddress* trSrc,
    OUT cmTransportAddress* trDest,
    OUT int*                isConnected);



/************************************************************************
 * mibGetControlParameters
 * purpose: Get control related MIB parameters
 * input  : hsCall      - Call to check
 *          inDirection - Direction to get
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetControlParameters(
    IN  HCALL                   hsCall,
    IN  int                     inDirection,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value);


/************************************************************************
 * mibGetChannelParameters
 * purpose: Get channel related MIB parameters
 * input  : hsChan      - Channel to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetChannelParameters(
    IN  HCHAN                   hsChan,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value);



#ifdef __cplusplus
}
#endif


#endif  /* __CMMIB_H */
