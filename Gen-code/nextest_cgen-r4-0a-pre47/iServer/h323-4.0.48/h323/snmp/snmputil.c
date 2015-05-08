#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <mib.h>
#include <snmpI.h>
#include <snmputil.h>
FILE *fpDebug;


const char * parameterName(h341ParameterName param)
{
switch(param)
{
 case	mmRoot:
	return "mmRoot";
 case mmH323Root:
	return "mmH323Root";
 case h225callSignaling:
 	return "h225callSignaling";
 case callSignalConfig:
 	return "callSignalConfig";
 case callSignalConfigTable:
 	return "callSignalConfigTable";
 case callSignalConfigEntry:
 	return "callSignalConfigEntry";
 case callSignalConfigMaxConnections:
 	return "callSignalConfigMaxConnections";
 case callSignalConfigAvailableConnections:
 	return "callSignalConfigAvailableConnections";
 case callSignalConfigT303:
 	return "callSignalConfigT303";
 case callSignalConfigT301:
 	return "callSignalConfigT301";
 case callSignalConfigEnableNotifications:
 	return "callSignalConfigEnableNotifications";
 case callSignalStats:
 	return "callSignalStats";
 case callSignalStatsTable :
 	return "callSignalStatsTable ";
 case callSignalStatsEntry:
 	return "callSignalStatsEntry";
 case callSignalStatsCallConnectionsIn:
 	return "callSignalStatsCallConnectionsIn";
 case callSignalStatsCallConnectionsOut:
 	return "callSignalStatsCallConnectionsOut";
 case callSignalStatsAlertingMsgsIn:
 	return "callSignalStatsAlertingMsgsIn";
 case callSignalStatsAlertingMsgsOut:
 	return "callSignalStatsAlertingMsgsOut";
 case callSignalStatsCallProceedingsIn:
 	return "callSignalStatsCallProceedingsIn";
 case callSignalStatsCallProceedingsOut:
 	return "callSignalStatsCallProceedingsOut";
 case callSignalStatsSetupMsgsIn:
 	return "callSignalStatsSetupMsgsIn";
 case callSignalStatsSetupMsgsOut:
 	return "callSignalStatsSetupMsgsOut";
 case callSignalStatsSetupAckMsgsIn:
 	return "callSignalStatsSetupAckMsgsIn";
 case callSignalStatsSetupAckMsgsOut:
 	return "callSignalStatsSetupAckMsgsOut";
 case callSignalStatsProgressMsgsIn:
 	return "callSignalStatsProgressMsgsIn";
 case callSignalStatsProgressMsgsOut:
 	return "callSignalStatsProgressMsgsOut";
 case callSignalStatsReleaseCompleteMsgsIn:
 	return "callSignalStatsReleaseCompleteMsgsIn";
 case callSignalStatsReleaseCompleteMsgsOut:
 	return "callSignalStatsReleaseCompleteMsgsOut";
 case callSignalStatsStatusMsgsIn:
 	return "callSignalStatsStatusMsgsIn";
 case callSignalStatsStatusMsgsOut:
 	return "callSignalStatsStatusMsgsOut";
 case callSignalStatsStatusInquiryMsgsIn:
 	return "callSignalStatsStatusInquiryMsgsIn";
 case callSignalStatsStatusInquiryMsgsOut:
 	return "callSignalStatsStatusInquiryMsgsOut";
 case callSignalStatsFacilityMsgsIn:
 	return "callSignalStatsFacilityMsgsIn";
 case callSignalStatsFacilityMsgsOut:
 	return "callSignalStatsFacilityMsgsOut";
 case callSignalStatsInfoMsgsIn:
 	return "callSignalStatsInfoMsgsIn";
 case callSignalStatsInfoMsgsOut:
 	return "callSignalStatsInfoMsgsOut";
 case callSignalStatsNotifyMsgsIn:
 	return "callSignalStatsNotifyMsgsIn";
 case callSignalStatsNotifyMsgsOut:
 	return "callSignalStatsNotifyMsgsOut";
 case callSignalStatsAverageCallDuration:
 	return "callSignalStatsAverageCallDuration";
 case connections :
 	return "connections ";
 case connectionsActiveConnections:
 	return "connectionsActiveConnections";
 case connectionsTable:
 	return "connectionsTable";
 case connectionsTableEntry:
 	return "connectionsTableEntry";
 case connectionsSrcTransporTAddressTag:
 	return "connectionsSrcTransporTAddressTag";
 case connectionsSrcTransporTAddress:
 	return "connectionsSrcTransporTAddress";
 case connectionsCallIdentifier:
 	return "connectionsCallIdentifier";
 case connectionsRole :
 	return "connectionsRole ";
 case connectionsState:
 	return "connectionsState";
 case connectionsDestTransporTAddressTag:
 	return "connectionsDestTransporTAddressTag";
 case connectionsDestTransporTAddress:
 	return "connectionsDestTransporTAddress";
 case connectionsDestAliasTag:
 	return "connectionsDestAliasTag";
 case connectionsDestAlias:
 	return "connectionsDestAlias";
 case connectionsSrcH245SigTransporTAddressTag:
 	return "connectionsSrcH245SigTransporTAddressTag";
 case connectionsSrcH245SigTransporTAddress:
 	return "connectionsSrcH245SigTransporTAddress";
 case connectionsDestH245SigTransporTAddressTag:
 	return "connectionsDestH245SigTransporTAddressTag";
 case connectionsDestH245SigTransporTAddress:
 	return "connectionsDestH245SigTransporTAddress";
 case connectionsConfId:
 	return "connectionsConfId";
 case connectionsCalledPartyNumber:
 	return "connectionsCalledPartyNumber";
 case connectionsDestXtraCallingNumber1:
 	return "connectionsDestXtraCallingNumber1";
 case connectionsDestXtraCallingNumber2:
 	return "connectionsDestXtraCallingNumber2";
 case connectionsDestXtraCallingNumber3:
 	return "connectionsDestXtraCallingNumber3";
 case connectionsDestXtraCallingNumber4:
 	return "connectionsDestXtraCallingNumber4";
 case connectionsDestXtraCallingNumber5:
 	return "connectionsDestXtraCallingNumber5";
 case connectionsFastCall:
 	return "connectionsFastCall";
 case connectionsSecurity:
 	return "connectionsSecurity";
 case connectionsH245Tunneling:
 	return "connectionsH245Tunneling";
 case connectionsCanOverlapSend:
 	return "connectionsCanOverlapSend";
 case connectionsCRV:
 	return "connectionsCRV";
 case connectionsCallType:
 	return "connectionsCallType";
 case connectionsRemoteExtensionAddress:
 	return "connectionsRemoteExtensionAddress";
 case connectionsExtraCRV1:
 	return "connectionsExtraCRV1";
 case connectionsExtraCRV2:
 	return "connectionsExtraCRV2";
 case connectionsConnectionStartTime:
 	return "connectionsConnectionStartTime";
 case connectionsEndpointType:
 	return "connectionsEndpointType";
 case connectionsReleaseCompleteReason:
 	return "connectionsReleaseCompleteReason";
 case callSignalEvents:
 	return "callSignalEvents";
 case callReleaseComplete:
 	return "callReleaseComplete";
case mmH245Root:
	return "mmH245Root:";
case h245:
   return  "h245";
case h245Configuration:
   return  "h245Configuration";
case h245ConfigurationTable:
   return  "h245ConfigurationTable";
case h245ConfigurationTableEntry:
   return  "h245ConfigurationTableEntry";
case h245ConfigT101Timer:
   return  "h245ConfigT101Timer";
case h245ConfigT102Timer:
   return  "h245ConfigT102Timer";
case h245ConfigT103Timer:
   return  "h245ConfigT103Timer";
case h245ConfigT104Timer:
   return  "h245ConfigT104Timer";
case h245ConfigT105Timer:
   return  "h245ConfigT105Timer";
case h245ConfigT106Timer:
   return  "h245ConfigT106Timer";
case h245ConfigT107Timer:
   return  "h245ConfigT107Timer";
case h245ConfigT108Timer:
   return  "h245ConfigT108Timer";
case h245ConfigT109Timer:
   return  "h245ConfigT109Timer";
case h245ConfigN100Counter:
   return  "h245ConfigN100Counter";
case h245ControlChannel:
   return  "h245ControlChannel";
case h245ControlChannelStatsTable:
   return  "h245ControlChannelStatsTable";
case h245ControlChannelStatsTableEntry:
   return  "h245ControlChannelStatsTableEntry";
case h245ControlChannelNumberOfListenPorts:
   return  "h245ControlChannelNumberOfListenPorts";
case h245ControlChannelMaxConnections:
   return  "h245ControlChannelMaxConnections";
case h245ControlChannelNumberOfListenFails:
   return  "h245ControlChannelNumberOfListenFails";
case h245ControlChannelNumberOfActiveConnections:
   return  "h245ControlChannelNumberOfActiveConnections";
case h245ControlChannelMasterSlaveMaxRetries:
   return  "h245ControlChannelMasterSlaveMaxRetries";
case h245ControlChannelConnectionAttemptsFail:
   return  "h245ControlChannelConnectionAttemptsFail";
case h245ControlChanneMasterSlavelDeterminations:
   return  "h245ControlChanneMasterSlavelDeterminations";
case h245ControlChannelMasterSlaveAcks:
   return  "h245ControlChannelMasterSlaveAcks";
case h245ControlChannelMasterSlaveRejects:
   return  "h245ControlChannelMasterSlaveRejects";
case h245ControlChannelMasterSlaveT106Rejects:
   return  "h245ControlChannelMasterSlaveT106Rejects";
case h245ControlChannelMasterSlaveMSDRejects:
   return  "h245ControlChannelMasterSlaveMSDRejects";
case h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects:
   return  "h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects";
case h245ControlChannelMasterSlaveMaxCounterRejects:
   return  "h245ControlChannelMasterSlaveMaxCounterRejects";
case h245ControlChannelMasterSlaveReleases:
   return  "h245ControlChannelMasterSlaveReleases";
case h245ControlChannelNumberOfTunnels:
   return  "h245ControlChannelNumberOfTunnels";
case h245ControlChannelMasterSlaveTable:
   return  "h245ControlChannelMasterSlaveTable";
case h245ControlChannelMasterSlaveTableEntry:
   return  "h245ControlChannelMasterSlaveTableEntry";
case h245ControlChannelSrcAddressTag:
   return  "h245ControlChannelSrcAddressTag";
case h245ControlChannelSrcTransporTAddress:
   return  "h245ControlChannelSrcTransporTAddress";
case h245ControlChannelDesTAddressTag:
   return  "h245ControlChannelDesTAddressTag";
case h245ControlChannelDestTransporTAddress:
   return  "h245ControlChannelDestTransporTAddress";
case h245ControlChannelIndex:
   return  "h245ControlChannelIndex";
case h245ControlChannelMSDState:
   return  "h245ControlChannelMSDState";
case h245ControlChannelTerminalType:
   return  "h245ControlChannelTerminalType";
case h245ControlChannelNumberOfMSDRetries:
   return  "h245ControlChannelNumberOfMSDRetries";
case h245ControlChannelIsTunneling:
   return  "h245ControlChannelIsTunneling";
case h245CapExchange:
   return  "h245CapExchange";
case h245CapExchangeStatsTable:
   return  "h245CapExchangeStatsTable";
case h245CapExchangeStatsTableEntry:
   return  "h245CapExchangeStatsTableEntry";
case h245CapExchangeSets:
   return  "h245CapExchangeSets";
case h245CapExchangeAcks:
   return  "h245CapExchangeAcks";
case h245CapExchangeRejects:
   return  "h245CapExchangeRejects";
case h245CapExchangeRejectUnspecified:
   return  "h245CapExchangeRejectUnspecified";
case h245CapExchangeRejectUndefinedTableEntryUsed:
   return  "h245CapExchangeRejectUndefinedTableEntryUsed";
case h245CapExchangeRejectDescriptorCapacityExceeded:
   return  "h245CapExchangeRejectDescriptorCapacityExceeded";
case h245CapExchangeRejectTableEntryCapacityExeeded:
   return  "h245CapExchangeRejectTableEntryCapacityExeeded";
case h245CapExchangeReleases:
   return  "h245CapExchangeReleases";
case h245CapExchangeCapabilityTable:
   return  "h245CapExchangeCapabilityTable";
case h245CapExchangeCapabilityTableEntry:
   return  "h245CapExchangeCapabilityTableEntry";
case h245CapExchangeDirection:
   return  "h245CapExchangeDirection";
case h245CapExchangeState:
   return  "h245CapExchangeState";
case h245CapExchangeProtocolId:
   return  "h245CapExchangeProtocolId";
case h245CapExchangeRejectCause:
   return  "h245CapExchangeRejectCause";
case h245CapExchangeMultiplexCapability:
   return  "h245CapExchangeMultiplexCapability";
case h245CapExchangeCapability:
   return  "h245CapExchangeCapability";
case h245CapExchangeCapabilityDescriptors:
   return  "h245CapExchangeCapabilityDescriptors";
case h245LogChannels:
   return  "h245LogChannels";
case h245LogChannelsChannelTable:
   return  "h245LogChannelsChannelTable";
case h245LogChannelsChannelTableEntry:
   return  "h245LogChannelsChannelTableEntry";
case h245LogChannelsChannelNumber:
   return  "h245LogChannelsChannelNumber";
case h245LogChannelsChannelDirection:
   return  "h245LogChannelsChannelDirection";
case h245LogChannelsIndex:
   return  "h245LogChannelsIndex";
case h245LogChannelsChannelState:
   return  "h245LogChannelsChannelState";
case h245LogChannelsMediaTableType:
   return  "h245LogChannelsMediaTableType";
case h245LogChannelsH225Table:
   return  "h245LogChannelsH225Table";
case h245LogChannelsH225TableEntry:
   return  "h245LogChannelsH225TableEntry";
case h245LogChannelsSessionId:
   return  "h245LogChannelsSessionId";
case h245LogChannelsAssociateSessionId:
   return  "h245LogChannelsAssociateSessionId";
case h245LogChannelsMediaChannel:
   return  "h245LogChannelsMediaChannel";
case h245LogChannelsMediaGuaranteedDelivery:
   return  "h245LogChannelsMediaGuaranteedDelivery";
case h245LogChannelsMediaControlChannel:
   return  "h245LogChannelsMediaControlChannel";
case h245LogChannelsMediaControlGuaranteedDelivery:
   return  "h245LogChannelsMediaControlGuaranteedDelivery";
case h245LogChannelsSilenceSuppression:
   return  "h245LogChannelsSilenceSuppression";
case h245LogChannelsDestination:
   return  "h245LogChannelsDestination";
case h245LogChannelsDynamicRTPPayloadType:
   return  "h245LogChannelsDynamicRTPPayloadType";
case h245LogChannelsH261aVideoPacketization:
   return  "h245LogChannelsH261aVideoPacketization";
case h245LogChannelsRTPPayloadDescriptor:
   return  "h245LogChannelsRTPPayloadDescriptor";
case h245LogChannelsRTPPayloadType:
   return  "h245LogChannelsRTPPayloadType";
case h245LogChannelsTransportCapability:
   return  "h245LogChannelsTransportCapability";
case h245LogChannelsRedundancyEncoding:
   return  "h245LogChannelsRedundancyEncoding";
case h245LogChannelsSrcTerminalLabel:
   return  "h245LogChannelsSrcTerminalLabel";
case h245LogChannelOpenLogicalChannelTable:
   return  "h245LogChannelOpenLogicalChannelTable";
case h245LogChannelOpenLogicalChannelTableEntry:
   return  "h245LogChannelOpenLogicalChannelTableEntry";
case h245LogChanOpenLogChanTotalRequests:
   return  "h245LogChanOpenLogChanTotalRequests";
case h245LogChanOpenLogChanAcks:
   return  "h245LogChanOpenLogChanAcks";
case h245LogChanOpenLogChanConfirms:
   return  "h245LogChanOpenLogChanConfirms";
case h245LogChanOpenLogChanRejects:
   return  "h245LogChanOpenLogChanRejects";
case h245LogChanOpenLogChanRejectUnspecified:
   return  "h245LogChanOpenLogChanRejectUnspecified";
case h245LogChanOpenLogChanRejectUnsuitableReverseParameters:
   return  "h245LogChanOpenLogChanRejectUnsuitableReverseParameters";
case h245LogChanOpenLogChanRejectDataTypeNotSupported:
   return  "h245LogChanOpenLogChanRejectDataTypeNotSupported";
case h245LogChanOpenLogChanRejectDataTypeNotAvailable:
   return  "h245LogChanOpenLogChanRejectDataTypeNotAvailable";
case h245LogChanOpenLogChanRejectUnknownDataType:
   return  "h245LogChanOpenLogChanRejectUnknownDataType";
case h245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported:
   return  "h245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported";
case h245LogChanOpenLogChanRejectMulticastChannelNotAllowed:
   return  "h245LogChanOpenLogChanRejectMulticastChannelNotAllowed";
case h245LogChanOpenLogChanRejectInsuffientBandwdith:
   return  "h245LogChanOpenLogChanRejectInsuffientBandwdith";
case h245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed:
   return  "h245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed";
case h245LogChanOpenLogChanRejectInvalidSessionID:
   return  "h245LogChanOpenLogChanRejectInvalidSessionID";
case h245LogChanOpenLogChanRejectMasterSlaveConflict:
   return  "h245LogChanOpenLogChanRejectMasterSlaveConflict";
case h245LogChanOpenLogChanRejectWaitForCommunicationMode:
   return  "h245LogChanOpenLogChanRejectWaitForCommunicationMode";
case h245LogChanOpenLogChanRejectInvalidDependentChannel:
   return  "h245LogChanOpenLogChanRejectInvalidDependentChannel";
case h245LogChansOpenLogChanRejectReplacementForRejected:
   return  "h245LogChansOpenLogChanRejectReplacementForRejected";
case h245LogChannelCloseLogicalChannelTable:
   return  "h245LogChannelCloseLogicalChannelTable";
case h245LogChannelCloseLogicalChannelTableEntry:
   return  "h245LogChannelCloseLogicalChannelTableEntry";
case h245LogChanCloseLogChannels:
   return  "h245LogChanCloseLogChannels";
case h245LogChanCloseLogChanAcks:
   return  "h245LogChanCloseLogChanAcks";
case h245LogChanCloseLogChanRequests:
   return  "h245LogChanCloseLogChanRequests";
case h245LogChanCloseLogChanRequestsAcks:
   return  "h245LogChanCloseLogChanRequestsAcks";
case h245LogChanCloseLogChanRequestRejects:
   return  "h245LogChanCloseLogChanRequestRejects";
case h245LogChanCloseLogChanRequestReleases:
   return  "h245LogChanCloseLogChanRequestReleases";
case h245Conference:
   return  "h245Conference";
case h245ConferenceTerminalTable:
   return  "h245ConferenceTerminalTable";
case h245ConferenceTerminalTableEntry:
   return  "h245ConferenceTerminalTableEntry";
case h245ConferenceConferenceId:
   return  "h245ConferenceConferenceId";
case h245ConferenceTerminalLabel:
   return  "h245ConferenceTerminalLabel";
case h245ConferenceControlChannelIndex:
   return  "h245ConferenceControlChannelIndex";
case h245ConferenceBroadcaster:
   return  "h245ConferenceBroadcaster";
case h245ConferenceConferenceChair:
   return  "h245ConferenceConferenceChair";
case h245ConferenceMultipoint:
   return  "h245ConferenceMultipoint";
case h245ConferenceStatsTable:
   return  "h245ConferenceStatsTable";
case h245ConferenceStatsTableEntry:
   return  "h245ConferenceStatsTableEntry";
case h245ConferenceBroadcastMyLogicalChannel:
   return  "h245ConferenceBroadcastMyLogicalChannel";
case h245ConferenceCancelBroadcastMyLogicalChannel:
   return  "h245ConferenceCancelBroadcastMyLogicalChannel";
case h245ConferenceSendThisSource:
   return  "h245ConferenceSendThisSource";
case h245ConferenceCancelSendThisSource:
   return  "h245ConferenceCancelSendThisSource";
case h245ConferenceDropConference:
   return  "h245ConferenceDropConference";
case h245ConferenceEqualiseDelay:
   return  "h245ConferenceEqualiseDelay";
case h245ConferenceZeroDelay:
   return  "h245ConferenceZeroDelay";
case h245ConferenceMultipointModeCommand:
   return  "h245ConferenceMultipointModeCommand";
case h245ConferenceCancelMultipointModeCommand:
   return  "h245ConferenceCancelMultipointModeCommand";
case h245ConferenceVideoFreezePicture:
   return  "h245ConferenceVideoFreezePicture";
case h245ConferenceVideoFastUpdatePicture:
   return  "h245ConferenceVideoFastUpdatePicture";
case h245ConferenceVideoFastUpdateGOB:
   return  "h245ConferenceVideoFastUpdateGOB";
case h245ConferenceVideoTemporalSpatialTradeOff:
   return  "h245ConferenceVideoTemporalSpatialTradeOff";
case h245ConferenceVideoSendSyncEveryGOB:
   return  "h245ConferenceVideoSendSyncEveryGOB";
case h245ConferenceVideoFastUpdateMB:
   return  "h245ConferenceVideoFastUpdateMB";
case h245Misc:
   return  "h245Misc";
case h245MiscRoundTripDelayTable:
   return  "h245MiscRoundTripDelayTable";
case h245MiscRoundTripDelayTableEntry:
   return  "h245MiscRoundTripDelayTableEntry";
case h245MiscRTDState:
   return  "h245MiscRTDState";
case h245MiscT105TimerExpired:
   return  "h245MiscT105TimerExpired";
case h245MiscLastRTDRequestSent:
   return  "h245MiscLastRTDRequestSent";
case h245MiscLastRTDRequestRcvd:
   return  "h245MiscLastRTDRequestRcvd";
case h245MiscLastRTDResponseSent:
   return  "h245MiscLastRTDResponseSent";
case h245MiscLastRTDResponseRcvd:
   return  "h245MiscLastRTDResponseRcvd";
case h245MiscMaintenanceLoopTable:
   return  "h245MiscMaintenanceLoopTable";
case h245MiscMaintenanceLoopTableEntry:
   return  "h245MiscMaintenanceLoopTableEntry";
case h245MiscMaintainenceLoopDirection:
   return  "h245MiscMaintainenceLoopDirection";
case h245MiscMLState:
   return  "h245MiscMLState";
case h245MiscNumberOfRequests:
   return  "h245MiscNumberOfRequests";
case h245MiscNumberOfAcks:
   return  "h245MiscNumberOfAcks";
case h245MiscLastMLRequestOrAckType:
   return  "h245MiscLastMLRequestOrAckType";
case h245MiscMLMediaOrLogicalChannelLoopRejectChannelNumber:
   return  "h245MiscMLMediaOrLogicalChannelLoopRejectChannelNumber";
case h245MiscNumberOfRejects:
   return  "h245MiscNumberOfRejects";
case h245MiscLastRejectType:
   return  "h245MiscLastRejectType";
case h245MiscErrorCode:
   return  "h245MiscErrorCode";
case mmH320Root:
	return "mmH320Root";
case   ras:
	return "ras";
case    rasConfiguration:
	return "rasConfiguration";
case    	  rasConfigurationTable:
	return "rasConfigurationTable";
case    	    rasConfigurationTableEntry:
	return "rasConfigurationTableEntry";
case                  rasConfigurationGatekeeperIdentifier:
	return "rasConfigurationGatekeeperIdentifier";
case                  rasConfigurationTimer:
	return "rasConfigurationTimer";
case                  rasConfigurationMaxNumberOfRetries:
	return "rasConfigurationMaxNumberOfRetries";
case                  rasConfigurationGatekeeperDiscoveryAddressTag:
	return "rasConfigurationGatekeeperDiscoveryAddressTag";
case                  rasConfigurationGatekeeperDiscoveryAddress:
	return "rasConfigurationGatekeeperDiscoveryAddress";


 default:
	 return "error";
}
}

void dPrintParameter(h341ParameterName param)

{
	char s[300];
	sprintf(s,"parameter %d %s\n",param,parameterName(param));		
/*	OutputDebugString(s);*/
/*	fwrite(s,strlen(s),1,fpDebug);*/
}

void dPrintOid(snmpObjectT *oid,char * buffer)
{
	int ii;
	char s[300];
	s[0]=0;
	sprintf (s,"%s ",buffer);
	for (ii = 0;ii<oid->length;ii++)
		sprintf(s+strlen(s),"%4d",oid->id[ii]);
	sprintf(s+strlen(s),"\n");

/*	OutputDebugString(s);*/
/*	fwrite(s,strlen(s),1,fpDebug);*/

}

void dPrintIndex(UINT8 *index,int size,char *buffer)
{
	int ii;
	char s[300];
	sprintf (s,"%s ",buffer);
	for (ii = 0;ii<size;ii++)
		sprintf(s+strlen(s),"%5d ",index[ii]);
	sprintf(s+strlen(s),"\n");
/*	OutputDebugString(s);*/
/*	fwrite(s,strlen(s),1,fpDebug);*/
	
}

int str2oid(char * str,int * id)
{
	int ii=0;
	char * ptr1,*ptr2;
	ptr1= str;
	ptr2= strchr(ptr1,'.');
	while(ptr2!=NULL)
	{
		/* *ptr2=(char)0;*/
		id[ii]=atoi(ptr1);
		ptr1= ptr2+1;
		ptr2= strchr(ptr1,'.');
		ii++;	
	}
	id[ii]=atoi(ptr1);
	return ii+1;
}


void dPrintMib(mibNodeT * node,char * oid,int offset,int num)
{
	int ii;
	char s[200],oidN[100];
	if (node !=NULL)
	{
		memset(s,' ',200);
		offset+=2;
		sprintf(oidN,"%s.%d",oid,num);
		sprintf(s+offset,"%s (%s)\n",parameterName(node->name),oidN);
/*		OutputDebugString(s);*/
/*		fwrite(s,strlen(s),1,fpDebug);*/

		if (!node->table)			
		{
			for (ii=0;ii<node->childNum;ii++)			
				dPrintMib( &(node->children[ii]),oidN,offset,ii+1);
		}	
		else
		{
			offset+=2;
			for (ii=0;ii<node->childNum;ii++)						
			{
				memset(s,' ',offset+1);
				sprintf(s+offset,"%s (%s.%d)\n",parameterName((h341ParameterName)(node->name+ii+1)),oidN,ii+1);
/*				OutputDebugString(s);	*/
	/*				fwrite(s,strlen(s),1,fpDebug);*/

			}
		}
		
	}


}
#ifdef __cplusplus
}
#endif
