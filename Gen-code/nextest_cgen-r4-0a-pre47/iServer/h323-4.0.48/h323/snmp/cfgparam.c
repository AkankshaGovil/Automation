#ifdef __cplusplus
extern "C" {
#endif

#include <mib.h>
#include <cm.h>
#include <cmmib.h>

void int2index(int entry,UINT8 * key);

mibStatisticParamEnumT  name2statType(h341ParameterName name)
{
    switch (name)
    {
    case callSignalStatsCallConnectionsIn:
        return enumcallSignalStatsCallConnectionsIn;
    case callSignalStatsCallConnectionsOut:
        return enumcallSignalStatsCallConnectionsOut;
    case callSignalStatsAlertingMsgsIn:
        return enumcallSignalStatsAlertingMsgsIn;
    case callSignalStatsAlertingMsgsOut:
        return enumcallSignalStatsAlertingMsgsOut;
    case callSignalStatsCallProceedingsIn:
        return enumcallSignalStatsCallProceedingsIn;
    case callSignalStatsCallProceedingsOut:
        return enumcallSignalStatsCallProceedingsOut;
    case callSignalStatsSetupMsgsIn:
        return enumcallSignalStatsSetupMsgsIn;
    case callSignalStatsSetupMsgsOut:
        return enumcallSignalStatsSetupMsgsOut;
    case callSignalStatsSetupAckMsgsIn:
        return enumcallSignalStatsSetupAckMsgsIn;
    case callSignalStatsSetupAckMsgsOut:
        return enumcallSignalStatsSetupAckMsgsOut;
    case callSignalStatsProgressMsgsIn:
        return enumcallSignalStatsProgressMsgsIn;
    case callSignalStatsProgressMsgsOut:
        return enumcallSignalStatsProgressMsgsOut;
    case callSignalStatsReleaseCompleteMsgsIn:
        return enumcallSignalStatsReleaseCompleteMsgsIn;
    case callSignalStatsReleaseCompleteMsgsOut:
        return enumcallSignalStatsReleaseCompleteMsgsOut;
    case callSignalStatsStatusMsgsIn:
        return enumcallSignalStatsStatusMsgsIn;
    case callSignalStatsStatusMsgsOut:
        return enumcallSignalStatsStatusMsgsOut;
    case callSignalStatsStatusInquiryMsgsIn:
        return enumcallSignalStatsStatusInquiryMsgsIn;
    case callSignalStatsStatusInquiryMsgsOut:
        return enumcallSignalStatsStatusInquiryMsgsOut;
    case callSignalStatsFacilityMsgsIn:
        return enumcallSignalStatsFacilityMsgsIn;
    case callSignalStatsFacilityMsgsOut:
        return enumcallSignalStatsFacilityMsgsOut;
    case callSignalStatsInfoMsgsIn:
        return enumcallSignalStatsInfoMsgsIn;
    case callSignalStatsInfoMsgsOut:
        return enumcallSignalStatsInfoMsgsOut;
    case callSignalStatsNotifyMsgsIn:
        return enumcallSignalStatsNotifyMsgsIn;
    case callSignalStatsNotifyMsgsOut:
        return enumcallSignalStatsNotifyMsgsOut;
    case callSignalStatsAverageCallDuration:
        return enumcallSignalStatsAverageCallDuration;


    case h245ControlChannelNumberOfListenPorts:
        return   enumh245ControlChannelNumberOfListenPorts ;
    case h245ControlChannelMaxConnections:
        return   enumh245ControlChannelMaxConnections ;
    case h245ControlChannelNumberOfListenFails:
        return   enumh245ControlChannelNumberOfListenFails ;
    case h245ControlChannelNumberOfActiveConnections:
        return   enumh245ControlChannelNumberOfActiveConnections ;
    case h245ControlChannelMasterSlaveMaxRetries:
        return   enumh245ControlChannelMasterSlaveMaxRetries ;
    case h245ControlChannelConnectionAttemptsFail:
        return   enumh245ControlChannelConnectionAttemptsFail ;
    case h245ControlChanneMasterSlavelDeterminations:
        return   enumh245ControlChanneMasterSlavelDeterminations ;
    case h245ControlChannelMasterSlaveAcks:
        return   enumh245ControlChannelMasterSlaveAcks ;
    case h245ControlChannelMasterSlaveRejects:
        return   enumh245ControlChannelMasterSlaveRejects ;
    case h245ControlChannelMasterSlaveT106Rejects:
        return   enumh245ControlChannelMasterSlaveT106Rejects ;
    case h245ControlChannelMasterSlaveMSDRejects:
        return   enumh245ControlChannelMasterSlaveMSDRejects ;
    case h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects:
        return   enumh245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects ;
    case h245ControlChannelMasterSlaveMaxCounterRejects:
        return   enumh245ControlChannelMasterSlaveMaxCounterRejects ;
    case h245ControlChannelMasterSlaveReleases:
        return   enumh245ControlChannelMasterSlaveReleases ;
    case h245ControlChannelNumberOfTunnels:
        return   enumh245ControlChannelNumberOfTunnels ;



    case h245CapExchangeSets:
        return   enumh245CapExchangeSets ;
    case h245CapExchangeAcks:
        return   enumh245CapExchangeAcks ;
    case h245CapExchangeRejects:
        return   enumh245CapExchangeRejects ;
    case h245CapExchangeRejectUnspecified:
        return   enumh245CapExchangeRejectUnspecified ;
    case h245CapExchangeRejectUndefinedTableEntryUsed:
        return   enumh245CapExchangeRejectUndefinedTableEntryUsed ;
    case h245CapExchangeRejectDescriptorCapacityExceeded:
        return   enumh245CapExchangeRejectDescriptorCapacityExceeded ;
    case h245CapExchangeRejectTableEntryCapacityExeeded:
        return   enumh245CapExchangeRejectTableEntryCapacityExeeded ;
    case h245CapExchangeReleases:
        return   enumh245CapExchangeReleases ;



    case h245LogChanOpenLogChanTotalRequests:
        return   enumh245LogChanOpenLogChanTotalRequests ;
    case h245LogChanOpenLogChanAcks:
        return   enumh245LogChanOpenLogChanAcks ;
    case h245LogChanOpenLogChanConfirms:
        return   enumh245LogChanOpenLogChanConfirms ;
    case h245LogChanOpenLogChanRejects:
        return   enumh245LogChanOpenLogChanRejects ;
    case h245LogChanOpenLogChanRejectUnspecified:
        return   enumh245LogChanOpenLogChanRejectUnspecified ;
    case h245LogChanOpenLogChanRejectUnsuitableReverseParameters:
        return   enumh245LogChanOpenLogChanRejectUnsuitableReverseParameters ;
    case h245LogChanOpenLogChanRejectDataTypeNotSupported:
        return   enumh245LogChanOpenLogChanRejectDataTypeNotSupported ;
    case h245LogChanOpenLogChanRejectDataTypeNotAvailable:
        return   enumh245LogChanOpenLogChanRejectDataTypeNotAvailable ;
    case h245LogChanOpenLogChanRejectUnknownDataType:
        return   enumh245LogChanOpenLogChanRejectUnknownDataType ;
    case h245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported:
        return   enumh245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported ;
    case h245LogChanOpenLogChanRejectMulticastChannelNotAllowed:
        return   enumh245LogChanOpenLogChanRejectMulticastChannelNotAllowed ;
    case h245LogChanOpenLogChanRejectInsuffientBandwdith:
        return   enumh245LogChanOpenLogChanRejectInsuffientBandwdith ;
    case h245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed:
        return   enumh245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed ;
    case h245LogChanOpenLogChanRejectInvalidSessionID:
        return   enumh245LogChanOpenLogChanRejectInvalidSessionID ;
    case h245LogChanOpenLogChanRejectMasterSlaveConflict:
        return   enumh245LogChanOpenLogChanRejectMasterSlaveConflict ;
    case h245LogChanOpenLogChanRejectWaitForCommunicationMode:
        return   enumh245LogChanOpenLogChanRejectWaitForCommunicationMode ;
    case h245LogChanOpenLogChanRejectInvalidDependentChannel:
        return   enumh245LogChanOpenLogChanRejectInvalidDependentChannel ;
    case h245LogChansOpenLogChanRejectReplacementForRejected:
        return   enumh245LogChansOpenLogChanRejectReplacementForRejected ;
    case h245LogChanCloseLogChannels:
        return   enumh245LogChanCloseLogChannels ;
    case h245LogChanCloseLogChanAcks:
        return   enumh245LogChanCloseLogChanAcks ;
    case h245LogChanCloseLogChanRequests:
        return   enumh245LogChanCloseLogChanRequests ;
    case h245LogChanCloseLogChanRequestsAcks:
        return   enumh245LogChanCloseLogChanRequestsAcks ;
    case h245LogChanCloseLogChanRequestRejects:
        return   enumh245LogChanCloseLogChanRequestRejects ;
    case h245LogChanCloseLogChanRequestReleases:
        return   enumh245LogChanCloseLogChanRequestReleases ;
    default :
        return (mibStatisticParamEnumT)-1;
    }

}

h341ErrorT h341InstGetParameter(h341InstanceHandle hSnmp,h341ParameterName name,mibDataT *data)
{
    HCFG hCfg;
	HPVT hPvt;
	int rasNodeId,q931NodeId,h245NodeId;
    BOOL isString;
    mibStatisticParamEnumT type;
    h341InstanceHandleT * hSnmpInst = (h341InstanceHandleT *) hSnmp;
    data->type = asnError;

    hCfg=cmGetConfiguration((HAPP)hSnmpInst->h341hApp);
	hPvt=cmGetValTree((HAPP)hSnmpInst->h341hApp);
	rasNodeId=cmGetRASConfigurationHandle((HAPP)hSnmpInst->h341hApp);
	q931NodeId=cmGetQ931ConfigurationHandle((HAPP)hSnmpInst->h341hApp);
    h245NodeId=cmGetH245ConfigurationHandle((HAPP)hSnmpInst->h341hApp);
    switch(name)
    {

    case			 callSignalConfigMaxConnections:

        data->type = asnInt;
        if (ciGetValue(hCfg,"system.maxCalls" ,&isString,&data->valueSize)<0)
            data->valueSize=0;
        break;
    case	 callSignalConfigT303:
        data->type = asnInt;
        if (pvtGetByPath(hPvt,q931NodeId,"responseTimeOut",NULL,&data->valueSize,&isString)<0)
            data->valueSize=0;
        break;
    case		 callSignalConfigT301:
        data->type = asnInt;
        if (pvtGetByPath(hPvt,q931NodeId,"connectTimeOut",NULL,&data->valueSize,&isString)<0)
            data->valueSize=0;
        break;
        /*			 callSignalConfigAvailableConnections,
            case		 callSignalConfigEnableNotifications:*/



    case              rasConfigurationGatekeeperIdentifier:
        {
			int tmpNodeId;
            data->type = asnMibOctetString;
            if ((tmpNodeId=pvtGetByPath(hPvt,rasNodeId,"registrationInfo.gatekeeperIdentifier",NULL,&data->valueSize,&isString))<0)
                data->valueSize=0;
            else
                if (pvtGetString(hPvt,tmpNodeId,data->valueSize,(char *)(data->value))<0)
                    data->valueSize=0;
        }
        break;
    case          rasConfigurationTimer:
        data->type = asnInt;
        if (pvtGetByPath(hPvt,rasNodeId,"responseTimeOut",NULL,&data->valueSize,&isString)<0)
            data->valueSize=0;
        break;
    case              rasConfigurationMaxNumberOfRetries:
        data->type = asnInt;
        if (pvtGetByPath(hPvt,rasNodeId,"maxRetries",NULL,&data->valueSize,&isString)<0)
            data->valueSize=3;
        break;
    case              rasConfigurationGatekeeperDiscoveryAddressTag:
        data->type = asnInt;
        data->valueSize=ipv4;
        break;
    case          rasConfigurationGatekeeperDiscoveryAddress:
        {
            UINT32 port;
			int tmpNodeId;
            data->valueSize = 4;
            data->type = asnAddressString;
			data->type = asnMibOctetString;
			if ((tmpNodeId=pvtGetByPath(hPvt,rasNodeId,"manualDiscovery.defaultGatekeeper.ipAddress.ip",NULL,&data->valueSize,&isString))<0)
				data->valueSize=0;
			else
				if (pvtGetString(hPvt,tmpNodeId,data->valueSize,(char *)(data->value))<0)
					data->valueSize=0;
				else
					if (pvtGetByPath(hPvt,rasNodeId,"manualDiscovery.defaultGatekeeper.ipAddress.port",NULL,(int *)&port,&isString)<0)
						data->valueSize=0;
					else
					{
			            int2index(port<<16,data->value+4);
			            data->valueSize=6;
					}
		}
        break;


    case h245ConfigT101Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"capabilities.timeout",NULL,&data->valueSize,&isString)<0)
            data->valueSize = 9;
        break;
    case h245ConfigT102Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"mediaLoopTimeout",NULL,&data->valueSize,&isString)<0)
            data->valueSize = 10;

        break;

    case h245ConfigT103Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"channelsTimeout",NULL,&data->valueSize,&isString)<0)
            data->valueSize = 29;
        break;
    case h245ConfigT105Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"roundTripTimeout",NULL,&data->valueSize,&isString)<0)
             data->valueSize = 10;    
        break;
    case h245ConfigT106Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"masterSlave.timeout",NULL,&data->valueSize,&isString)<0)
             data->valueSize = 9;
        break;
    case h245ConfigT109Timer:
        data->type=asnInt;
        if (pvtGetByPath(hPvt,h245NodeId,"requestModeTimeout",NULL,&data->valueSize,&isString)<0)
             data->valueSize = 10;
        break;


    case h245ConfigN100Counter:

        data->type = asnInt;
        data->valueSize=3;
        break;

       
    case h245ConfigT104Timer:
    case h245ConfigT107Timer:
    case h245ConfigT108Timer:
        return (h341ErrorT)notImplemented;
    break;

    default:
        type = name2statType(name);
        if (type>=0)
        {
            data->valueSize = mibGetStatistic((HAPP)hSnmpInst->h341hApp,type);
            if (data->valueSize>=0)
                data->type = asnInt;
            else
                return (h341ErrorT)notImplemented;
        }
        else
            return (h341ErrorT)notImplemented;

    }

    return (h341ErrorT)0;
}

h341ErrorT h341InstSetRequest(h341InstanceHandle hSnmp,h341ParameterName name,IN mibDataT *data)

{
    int ret=TRUE;
    char *buffer;
    int  bufferSize;
	HPVT hPvt;
	int rasNodeId,q931NodeId,h245NodeId;
    h341InstanceHandleT * hSnmpInst = (h341InstanceHandleT *) hSnmp;
    buffer = (char *)(data->value);
    bufferSize = data->valueSize;
    if (hSnmpInst->h341EvReadWriteSet)
        ret = hSnmpInst->h341EvReadWriteSet(hSnmpInst->h341hApp,name,buffer,bufferSize);
    if (!ret)
        return (h341ErrorT)ret;
	hPvt=cmGetValTree((HAPP)hSnmpInst->h341hApp);
	rasNodeId=cmGetRASConfigurationHandle((HAPP)hSnmpInst->h341hApp);
	q931NodeId=cmGetQ931ConfigurationHandle((HAPP)hSnmpInst->h341hApp);
    h245NodeId=cmGetH245ConfigurationHandle((HAPP)hSnmpInst->h341hApp);
    switch(name)
    {
        case callSignalConfigT303:
             pvtBuildByPath(hPvt,q931NodeId,"responseTimeOut",(INT32)bufferSize,NULL);
             break;
        case callSignalConfigT301:
             pvtBuildByPath(hPvt,q931NodeId,"connectTimeOut",(INT32)bufferSize,NULL);
            break;



        case rasConfigurationTimer:
             pvtBuildByPath(hPvt,rasNodeId,"responseTimeOut",(INT32)bufferSize,NULL);
            break;
        case rasConfigurationMaxNumberOfRetries:
             pvtBuildByPath(hPvt,rasNodeId,"maxRetries",(INT32)bufferSize,NULL);
            break;


        case h245ConfigT101Timer:
             pvtBuildByPath(hPvt,h245NodeId,"capabilities.timeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT102Timer:
            pvtBuildByPath(hPvt,h245NodeId,"mediaLoopTimeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT103Timer:
            pvtBuildByPath(hPvt,h245NodeId,"channelsTimeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT105Timer:
            pvtBuildByPath(hPvt,h245NodeId,"roundTripTimeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT106Timer:
            pvtBuildByPath(hPvt,h245NodeId,"masterSlave.timeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT109Timer:
            pvtBuildByPath(hPvt,h245NodeId,"requestModeTimeout",(INT32)bufferSize,NULL);
            break;
        case h245ConfigT104Timer:
        case h245ConfigT107Timer:
        case h245ConfigT108Timer:
        case h245ConfigN100Counter:
            break;
        default :
            return noWriteAccessRc;

    }
	return (h341ErrorT)0;
}

#ifdef __cplusplus
}
#endif
