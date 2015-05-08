#ifdef __cplusplus
extern "C" {
#endif

#ifndef __H341_COMMON_H
#define __H341_COMMON_H

#include <rvcommon.h>

#define CONNECTION_KEY_SIZE 26
#define SUPPORT_SUBTREE_SIZE 5
#define MAX_ID_SIZE         50

#define ipv4    1
#define invalidCall 100


typedef enum
{
    asnInt,
    asnMibOctetString,
    asnAddressString,
    asnError
}h341AsnTypeT;

typedef struct
{
    UINT8 * value;
    h341AsnTypeT type;
    int valueSize;

}mibDataT;


typedef struct
{
    int * id;
    int length;
    mibDataT data;
}snmpObjectT;



typedef enum
{
    okRc = 0,
    noSuchRootRc = 1,
    noSuchFieldRc,
    noSuchIndexRc,
    noRegisterInstanceRc = 10,
    noRequestNumberRc,
    noWriteAccessRc,
    noMemoryRc,
    notImplementedRc = 20,
    outOfMibRc,
    noSuchInstanceRc = 50
}h341ErrorT;

typedef int h341memAllocT; /*max size of oid to be returned or passed into module*/

typedef enum
{
 mmRoot,
  mmH323Root,
   h225callSignaling,
    callSignalConfig,
        callSignalConfigTable,
            callSignalConfigEntry, /*ifIndex*/
             callSignalConfigMaxConnections,
             callSignalConfigAvailableConnections,
             callSignalConfigT303,
             callSignalConfigT301,
             callSignalConfigEnableNotifications,
    callSignalStats,
        callSignalStatsTable ,
            callSignalStatsEntry, /* ifIndex*/
                 callSignalStatsCallConnectionsIn,
                 callSignalStatsCallConnectionsOut,
                 callSignalStatsAlertingMsgsIn,
                 callSignalStatsAlertingMsgsOut,
                 callSignalStatsCallProceedingsIn,
                 callSignalStatsCallProceedingsOut,
                 callSignalStatsSetupMsgsIn,
                 callSignalStatsSetupMsgsOut,
                 callSignalStatsSetupAckMsgsIn,
                 callSignalStatsSetupAckMsgsOut,
                 callSignalStatsProgressMsgsIn,
                 callSignalStatsProgressMsgsOut,
                 callSignalStatsReleaseCompleteMsgsIn,
                 callSignalStatsReleaseCompleteMsgsOut,
                 callSignalStatsStatusMsgsIn,
                 callSignalStatsStatusMsgsOut,
                 callSignalStatsStatusInquiryMsgsIn,
                 callSignalStatsStatusInquiryMsgsOut,
                 callSignalStatsFacilityMsgsIn,
                 callSignalStatsFacilityMsgsOut,
                 callSignalStatsInfoMsgsIn,
                 callSignalStatsInfoMsgsOut,
                 callSignalStatsNotifyMsgsIn,
                 callSignalStatsNotifyMsgsOut,
                 callSignalStatsAverageCallDuration,

    connections ,
        connectionsActiveConnections,
            connectionsTable,
                connectionsTableEntry,/*ifIndex*/


              connectionsSrcTransporTAddressTag,
             connectionsSrcTransporTAddress,
              connectionsCallIdentifier,
             connectionsRole ,
              connectionsState,
              connectionsDestTransporTAddressTag,
              connectionsDestTransporTAddress,
              connectionsDestAliasTag,
              connectionsDestAlias,
              connectionsSrcH245SigTransporTAddressTag,
              connectionsSrcH245SigTransporTAddress,
              connectionsDestH245SigTransporTAddressTag,
              connectionsDestH245SigTransporTAddress,
              connectionsConfId,
              connectionsCalledPartyNumber,
              connectionsDestXtraCallingNumber1,
              connectionsDestXtraCallingNumber2,
              connectionsDestXtraCallingNumber3,
              connectionsDestXtraCallingNumber4,
              connectionsDestXtraCallingNumber5,
              connectionsFastCall,
              connectionsSecurity,
              connectionsH245Tunneling,
              connectionsCanOverlapSend,
              connectionsCRV,
              connectionsCallType,
              connectionsRemoteExtensionAddress,
              connectionsExtraCRV1,
              connectionsExtraCRV2,
              connectionsConnectionStartTime,
              connectionsEndpointType,
              connectionsReleaseCompleteReason,



    callSignalEvents,
        callReleaseComplete,

   ras,
    rasConfiguration,
      rasConfigurationTable,
        rasConfigurationTableEntry,
              rasConfigurationGatekeeperIdentifier,
              rasConfigurationTimer,
              rasConfigurationMaxNumberOfRetries,
              rasConfigurationGatekeeperDiscoveryAddressTag,
              rasConfigurationGatekeeperDiscoveryAddress,

  mmH320Root,
  mmH245Root,
   h245,
     h245Configuration,
      h245ConfigurationTable,
         h245ConfigurationTableEntry,
            h245ConfigT101Timer,
            h245ConfigT102Timer,
            h245ConfigT103Timer,
            h245ConfigT104Timer,
            h245ConfigT105Timer,
            h245ConfigT106Timer,
            h245ConfigT107Timer,
            h245ConfigT108Timer,
            h245ConfigT109Timer,
            h245ConfigN100Counter,
   h245ControlChannel,
      h245ControlChannelStatsTable,
         h245ControlChannelStatsTableEntry,
            h245ControlChannelNumberOfListenPorts,
            h245ControlChannelMaxConnections,
            h245ControlChannelNumberOfListenFails,
            h245ControlChannelNumberOfActiveConnections,
            h245ControlChannelMasterSlaveMaxRetries,
            h245ControlChannelConnectionAttemptsFail,
            h245ControlChanneMasterSlavelDeterminations,
            h245ControlChannelMasterSlaveAcks,
            h245ControlChannelMasterSlaveRejects,
            h245ControlChannelMasterSlaveT106Rejects,
            h245ControlChannelMasterSlaveMSDRejects,
            h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects,
            h245ControlChannelMasterSlaveMaxCounterRejects,
            h245ControlChannelMasterSlaveReleases,
            h245ControlChannelNumberOfTunnels,
      h245ControlChannelMasterSlaveTable,
         h245ControlChannelMasterSlaveTableEntry,
            h245ControlChannelSrcAddressTag,
            h245ControlChannelSrcTransporTAddress,
            h245ControlChannelDesTAddressTag,
            h245ControlChannelDestTransporTAddress,
            h245ControlChannelIndex,
            h245ControlChannelMSDState,
            h245ControlChannelTerminalType,
            h245ControlChannelNumberOfMSDRetries,
            h245ControlChannelIsTunneling,
   h245CapExchange,
      h245CapExchangeStatsTable,
         h245CapExchangeStatsTableEntry,
            h245CapExchangeSets,
            h245CapExchangeAcks,
            h245CapExchangeRejects,
            h245CapExchangeRejectUnspecified,
            h245CapExchangeRejectUndefinedTableEntryUsed,
            h245CapExchangeRejectDescriptorCapacityExceeded,
            h245CapExchangeRejectTableEntryCapacityExeeded,
            h245CapExchangeReleases,
      h245CapExchangeCapabilityTable,
         h245CapExchangeCapabilityTableEntry,
            h245CapExchangeDirection,
            h245CapExchangeState,
            h245CapExchangeProtocolId,
            h245CapExchangeRejectCause,
            h245CapExchangeMultiplexCapability,
            h245CapExchangeCapability,
            h245CapExchangeCapabilityDescriptors,
   h245LogChannels,
      h245LogChannelsChannelTable,
         h245LogChannelsChannelTableEntry,
            h245LogChannelsChannelNumber,
            h245LogChannelsChannelDirection,
            h245LogChannelsIndex,
            h245LogChannelsChannelState,
            h245LogChannelsMediaTableType,
      h245LogChannelsH225Table,
         h245LogChannelsH225TableEntry,
            h245LogChannelsSessionId,
            h245LogChannelsAssociateSessionId,
            h245LogChannelsMediaChannel,
            h245LogChannelsMediaGuaranteedDelivery,
            h245LogChannelsMediaControlChannel,
            h245LogChannelsMediaControlGuaranteedDelivery,
            h245LogChannelsSilenceSuppression,
            h245LogChannelsDestination,
            h245LogChannelsDynamicRTPPayloadType,
            h245LogChannelsH261aVideoPacketization,
            h245LogChannelsRTPPayloadDescriptor,
            h245LogChannelsRTPPayloadType,
            h245LogChannelsTransportCapability,
            h245LogChannelsRedundancyEncoding,
            h245LogChannelsSrcTerminalLabel,
      h245LogChannelOpenLogicalChannelTable,
         h245LogChannelOpenLogicalChannelTableEntry,
            h245LogChanOpenLogChanTotalRequests,
            h245LogChanOpenLogChanAcks,
            h245LogChanOpenLogChanConfirms,
            h245LogChanOpenLogChanRejects,
            h245LogChanOpenLogChanRejectUnspecified,
            h245LogChanOpenLogChanRejectUnsuitableReverseParameters,
            h245LogChanOpenLogChanRejectDataTypeNotSupported,
            h245LogChanOpenLogChanRejectDataTypeNotAvailable,
            h245LogChanOpenLogChanRejectUnknownDataType,
            h245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported,
            h245LogChanOpenLogChanRejectMulticastChannelNotAllowed,
            h245LogChanOpenLogChanRejectInsuffientBandwdith,
            h245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed,
            h245LogChanOpenLogChanRejectInvalidSessionID,
            h245LogChanOpenLogChanRejectMasterSlaveConflict,
            h245LogChanOpenLogChanRejectWaitForCommunicationMode,
            h245LogChanOpenLogChanRejectInvalidDependentChannel,
            h245LogChansOpenLogChanRejectReplacementForRejected,
      h245LogChannelCloseLogicalChannelTable,
         h245LogChannelCloseLogicalChannelTableEntry,
            h245LogChanCloseLogChannels,
            h245LogChanCloseLogChanAcks,
            h245LogChanCloseLogChanRequests,
            h245LogChanCloseLogChanRequestsAcks,
            h245LogChanCloseLogChanRequestRejects,
            h245LogChanCloseLogChanRequestReleases,
   h245Conference,
      h245ConferenceTerminalTable,
         h245ConferenceTerminalTableEntry,
            h245ConferenceConferenceId,
            h245ConferenceTerminalLabel,
            h245ConferenceControlChannelIndex,
            h245ConferenceBroadcaster,
            h245ConferenceConferenceChair,
            h245ConferenceMultipoint,
      h245ConferenceStatsTable,
         h245ConferenceStatsTableEntry,
            h245ConferenceBroadcastMyLogicalChannel,
            h245ConferenceCancelBroadcastMyLogicalChannel,
            h245ConferenceSendThisSource,
            h245ConferenceCancelSendThisSource,
            h245ConferenceDropConference,
            h245ConferenceEqualiseDelay,
            h245ConferenceZeroDelay,
            h245ConferenceMultipointModeCommand,
            h245ConferenceCancelMultipointModeCommand,
            h245ConferenceVideoFreezePicture,
            h245ConferenceVideoFastUpdatePicture,
            h245ConferenceVideoFastUpdateGOB,
            h245ConferenceVideoTemporalSpatialTradeOff,
            h245ConferenceVideoSendSyncEveryGOB,
            h245ConferenceVideoFastUpdateMB,
   h245Misc,
      h245MiscRoundTripDelayTable,
         h245MiscRoundTripDelayTableEntry,
            h245MiscRTDState,
            h245MiscT105TimerExpired,
            h245MiscLastRTDRequestSent,
            h245MiscLastRTDRequestRcvd,
            h245MiscLastRTDResponseSent,
            h245MiscLastRTDResponseRcvd,
      h245MiscMaintenanceLoopTable,
         h245MiscMaintenanceLoopTableEntry,
            h245MiscMaintainenceLoopDirection,
            h245MiscMLState,
            h245MiscNumberOfRequests,
            h245MiscNumberOfAcks,
            h245MiscLastMLRequestOrAckType,
            h245MiscMLMediaOrLogicalChannelLoopRejectChannelNumber,
            h245MiscNumberOfRejects,
            h245MiscLastRejectType,
            h245MiscErrorCode

}h341ParameterName;


typedef enum
{
    retrieveParam,
    retrieveByIndex,
    retrieveNextIndex
}dataRequestTypeT;


typedef struct
{
    UINT8 index[MAX_ID_SIZE];
    int indexSize;
    h341ParameterName name;
    dataRequestTypeT reqType;
    UINT8 nextIndex[MAX_ID_SIZE];
    int nextIndexSize;
    mibDataT  pData;
    int result;
}instanceRequestDataT;


typedef instanceRequestDataT * instanceRequestDataPtrT;

#endif

#ifdef __cplusplus
}
#endif
