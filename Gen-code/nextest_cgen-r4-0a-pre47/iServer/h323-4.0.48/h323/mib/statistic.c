
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <rvinternal.h>
#include <cm.h>
#include <q931asn1.h>
#include <h245.h>
#include <statistic.h>


/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


HSTATISTIC mibCreateStatistic(void)
{
    h341StatisticParametersT * pStatistic = (h341StatisticParametersT *)malloc(sizeof(h341StatisticParametersT));
    memset((void *)pStatistic,0,sizeof(h341StatisticParametersT));
    return (HSTATISTIC) pStatistic;
}


void mibDestroyStatistic(IN HSTATISTIC statistic)
{
    free ((h341StatisticParametersT *)statistic);
}


/************************************************************************
 * addStatistic
 * purpose: Update the statistics information about current state of the
 *          stack. This information is taken from incoming and outgoing
 *          messages.
 * input  : hStatistic  - Statistics information handle
 *          mType       - Type of message to check
 *          hVal        - Value tree of the message to check
 *          vNodeId     - Root ID of the message to check
 *          directionIn - TRUE if this is an incoming message, FALSE if outgoing.
 * output : none
 * return : none
 ************************************************************************/
void addStatistic(
    IN HSTATISTIC       hStatistic,
    IN cmProtocol       mType,
    IN HPVT             hVal,
    IN int              vNodeId,
    IN BOOL             directionIn)
{
    int chNodeId, index, req;
    h341StatisticParametersT* pStatistic;

    pStatistic = (h341StatisticParametersT *)hStatistic;

    switch (mType)
    {
        case cmProtocolQ931:
        {
            /* Q931 message - increase incoming/outgoing message */
            chNodeId = pvtGetChild(hVal, vNodeId, __q931(message), NULL);
            if (chNodeId < 0)
                return;
            chNodeId = pvtChild(hVal, chNodeId);
            index = pvtGetSyntaxIndex(hVal, chNodeId);
            index--;
            if (directionIn)
                pStatistic->q931StatisticIn[index]++;
            else
                pStatistic->q931StatisticOut[index]++;
            break;
        }

        case cmProtocolH245:
        {
            INTPTR fieldId;
            chNodeId = pvtChild(hVal, vNodeId);
            req = pvtGetSyntaxIndex(hVal, chNodeId); /*1 request,2 response,4 for indication*/
            chNodeId = pvtChild(hVal, chNodeId);
            pvtGet(hVal, chNodeId, &fieldId, NULL, NULL, NULL);
            if (req == 1)
            {
                switch(fieldId)
                {
                    case __h245(masterSlaveDetermination):
                        if (!directionIn)
                            pStatistic->h245ControlChanneMasterSlavelDeterminations++;
                        break;
                    case __h245(terminalCapabilitySet):
                        pStatistic->h245CapExchangeSets++;
                        break;
                    case __h245(openLogicalChannel):
                        pStatistic->h245LogChanOpenLogChanTotalRequests++;
                        break;
                    case __h245(closeLogicalChannel):
                        pStatistic->h245LogChanCloseLogChannels++;
                        break;
                    case __h245(requestChannelClose):
                        pStatistic->h245LogChanCloseLogChanRequests++;
                    default:
                        break;
                }
            }
            else if (req == 2)
            {
                switch(fieldId)
                {
                    case __h245(masterSlaveDeterminationAck):
                        pStatistic->h245ControlChannelMasterSlaveAcks++;
                        break;
                    case __h245(masterSlaveDeterminationReject):
                        pStatistic->h245ControlChannelMasterSlaveRejects++;
                        pStatistic->h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects++;
                        break;
                    case __h245(terminalCapabilitySetAck):
                        pStatistic->h245CapExchangeAcks++;
                        break;
                    case __h245(terminalCapabilitySetReject):
                        pStatistic->h245CapExchangeRejects++;

                        chNodeId = pvtGetChild(hVal, chNodeId, __h245(cause), NULL);
                        if(chNodeId >= 0)
                        {
                            chNodeId= pvtChild(hVal, chNodeId);
                            index = pvtGetSyntaxIndex(hVal, chNodeId);
                            pStatistic->h245CapExchangeRejectCause[index-1]++;
                        }
                        break;
                    case __h245(openLogicalChannelAck):
                        pStatistic->h245LogChanOpenLogChanAcks++;
                        break;
                    case __h245(openLogicalChannelReject):
                        pStatistic->h245LogChanOpenLogChanRejects++;

                        chNodeId = pvtGetChild(hVal, chNodeId, __h245(cause), NULL);
                        if(chNodeId >= 0)
                        {
                            chNodeId= pvtChild(hVal, chNodeId);
                            index = pvtGetSyntaxIndex(hVal, chNodeId);
                            pStatistic->h245LogChanOpenLogChanRejectCause[index-1]++;
                        }
                        break;
                    case __h245(closeLogicalChannelAck):
                        pStatistic->h245LogChanCloseLogChanAcks++;
                        break;
                    case __h245(requestChannelCloseAck):
                        pStatistic->h245LogChanCloseLogChanRequestsAcks++;
                        break;
                    case __h245(requestChannelCloseReject):
                        pStatistic->h245LogChanCloseLogChanRequestRejects++;
                        break;
                    default:
                        break;
                }
            }
            else if (req == 4)
            {
                switch(fieldId)
                {
                    case __h245(masterSlaveDeterminationRelease):
                        pStatistic->h245ControlChannelMasterSlaveReleases++;
                    break;
                    case __h245(terminalCapabilitySetRelease):
                        pStatistic->h245CapExchangeReleases++;
                        break;
                    case __h245(openLogicalChannelConfirm):
                        pStatistic->h245LogChanOpenLogChanConfirms++;
                        break;
                    case __h245(requestChannelCloseRelease):
                        pStatistic->h245LogChanCloseLogChanRequestReleases++;
                        break;
                    default:
                        break;
                }
            }
            break;
        }

        case cmProtocolRAS:
        case cmProtocolUnknown:
            break;
    }

    return;
}


/************************************************************************
 * getStatistic
 * purpose: Get a statistics parameter value for the MIB
 * input  : pStatistic  - Current statistics information
 *          type        - Type of parameter to get
 * output : none
 * return : Parameter's value on success
 *          Negative value on failure
 ************************************************************************/
int getStatistic(IN h341StatisticParametersT* pStatistic, IN mibStatisticParamEnumT type)
{
    switch(type)
    {
       case enumcallSignalStatsAlertingMsgsOut:
           return pStatistic->q931StatisticOut[alerting];
       case enumcallSignalStatsAlertingMsgsIn:
           return pStatistic->q931StatisticIn[alerting];
       case enumcallSignalStatsCallProceedingsIn:
           return pStatistic->q931StatisticIn[callProceeding];
       case enumcallSignalStatsCallProceedingsOut:
           return pStatistic->q931StatisticOut[callProceeding];

       /* Connections enumerations are reverse than the collected data, since CONNECT
          messages are sent by the callee. */
       case enumcallSignalStatsCallConnectionsIn:
           return pStatistic->q931StatisticOut[connect];
       case enumcallSignalStatsCallConnectionsOut:
           return pStatistic->q931StatisticIn[connect];

       case enumcallSignalStatsSetupMsgsIn:
           return pStatistic->q931StatisticIn[setup];
       case enumcallSignalStatsSetupMsgsOut:
           return pStatistic->q931StatisticOut[setup];
       case enumcallSignalStatsSetupAckMsgsIn:
           return pStatistic->q931StatisticIn[setupAck];
       case enumcallSignalStatsSetupAckMsgsOut:
           return pStatistic->q931StatisticOut[setupAck];
       case enumcallSignalStatsProgressMsgsIn:
           return pStatistic->q931StatisticIn[progress];
       case enumcallSignalStatsProgressMsgsOut:
           return pStatistic->q931StatisticOut[progress];
       case enumcallSignalStatsReleaseCompleteMsgsIn:
           return pStatistic->q931StatisticIn[releaseComplete];
       case enumcallSignalStatsReleaseCompleteMsgsOut:
           return pStatistic->q931StatisticOut[releaseComplete];
       case enumcallSignalStatsStatusMsgsIn:
           return pStatistic->q931StatisticIn[status];
       case enumcallSignalStatsStatusMsgsOut:
           return pStatistic->q931StatisticOut[status];
       case enumcallSignalStatsStatusInquiryMsgsIn:
           return pStatistic->q931StatisticIn[statusEnquiry];
       case enumcallSignalStatsStatusInquiryMsgsOut:
           return pStatistic->q931StatisticOut[statusEnquiry];
       case enumcallSignalStatsFacilityMsgsIn:
           return pStatistic->q931StatisticIn[facility];
       case enumcallSignalStatsFacilityMsgsOut:
           return pStatistic->q931StatisticOut[facility];
       case enumcallSignalStatsInfoMsgsIn:
           return pStatistic->q931StatisticIn[information];
       case enumcallSignalStatsInfoMsgsOut:
           return pStatistic->q931StatisticOut[information];
       case enumcallSignalStatsNotifyMsgsIn:
           return pStatistic->q931StatisticIn[q931notify];
       case enumcallSignalStatsNotifyMsgsOut:
           return pStatistic->q931StatisticOut[q931notify];

       case enumcallSignalStatsAverageCallDuration:
           return -1;
       case enumh245ControlChannelNumberOfListenPorts:
           return -1   ;/*not Implemented*/
       case enumh245ControlChannelMaxConnections:  return -1   ;/*not Implemented*/
       case enumh245ControlChannelNumberOfListenFails:  return -1   ;/*not Implemented*/
       case enumh245ControlChannelNumberOfActiveConnections:  return -1   ;/*not Implemented*/
       case enumh245ControlChannelMasterSlaveMaxRetries:  return -1   ;/*not Implemented*/
       case enumh245ControlChannelConnectionAttemptsFail:  return -1   ;/*not Implemented*/
       case enumh245ControlChannelNumberOfTunnels:  return -1   ;/*not Implemented*/

       case enumh245ControlChanneMasterSlavelDeterminations:
           return pStatistic->h245ControlChanneMasterSlavelDeterminations;
       case enumh245ControlChannelMasterSlaveAcks:
           return pStatistic->h245ControlChannelMasterSlaveAcks;
       case enumh245ControlChannelMasterSlaveRejects:
           return pStatistic->h245ControlChannelMasterSlaveRejects;
       case enumh245ControlChannelMasterSlaveReleases :
           return pStatistic->h245ControlChannelMasterSlaveReleases;
       case enumh245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects:
           return pStatistic->h245ControlChannelNumberOfMasterSlaveInconsistentFieldRejects;


       case enumh245ControlChannelMasterSlaveMSDRejects :  return -1   ;/*not Implemented*/
       case enumh245ControlChannelMasterSlaveT106Rejects:  return -1   ;       /*not Implemented*/
       case enumh245ControlChannelMasterSlaveMaxCounterRejects:  return -1   ;/*not Implemented*/



       case enumh245CapExchangeSets:
           return pStatistic->h245CapExchangeSets;
       case enumh245CapExchangeAcks:
           return pStatistic->h245CapExchangeAcks;
       case enumh245CapExchangeRejects:
           return pStatistic->h245CapExchangeRejects;
       case enumh245CapExchangeReleases:
           return pStatistic->h245CapExchangeReleases;

       case enumh245CapExchangeRejectUnspecified:
           return pStatistic->h245CapExchangeRejectCause[h245CapExchangeRejectUnspecified];
       case enumh245CapExchangeRejectUndefinedTableEntryUsed:
           return pStatistic->h245CapExchangeRejectCause[h245CapExchangeRejectUndefinedTableEntryUsed];
       case enumh245CapExchangeRejectDescriptorCapacityExceeded:
           return pStatistic->h245CapExchangeRejectCause[h245CapExchangeRejectDescriptorCapacityExceeded];
       case enumh245CapExchangeRejectTableEntryCapacityExeeded:
           return pStatistic->h245CapExchangeRejectCause[h245CapExchangeRejectTableEntryCapacityExeeded];


       case enumh245LogChanOpenLogChanTotalRequests:
           return pStatistic->h245LogChanOpenLogChanTotalRequests;
       case enumh245LogChanOpenLogChanAcks:
           return pStatistic->h245LogChanOpenLogChanAcks;
       case enumh245LogChanOpenLogChanConfirms:
           return pStatistic->h245LogChanOpenLogChanConfirms;
       case enumh245LogChanOpenLogChanRejects:
           return pStatistic->h245LogChanOpenLogChanRejects;


       case enumh245LogChanOpenLogChanRejectUnspecified:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectUnspecified];
       case enumh245LogChanOpenLogChanRejectUnsuitableReverseParameters:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectUnsuitableReverseParameters];
       case enumh245LogChanOpenLogChanRejectDataTypeNotSupported:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectDataTypeNotSupported];
       case enumh245LogChanOpenLogChanRejectDataTypeNotAvailable:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectDataTypeNotAvailable];

       case enumh245LogChanOpenLogChanRejectUnknownDataType:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectUnknownDataType];

       case enumh245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectDataTypeALCombinationNotSupported];
       case enumh245LogChanOpenLogChanRejectMulticastChannelNotAllowed:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectMulticastChannelNotAllowed];
       case enumh245LogChanOpenLogChanRejectInsuffientBandwdith:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectInsuffientBandwdith];
       case enumh245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectSeparateStackEstablishmentFailed];
       case enumh245LogChanOpenLogChanRejectInvalidSessionID:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectInvalidSessionID];
       case enumh245LogChanOpenLogChanRejectMasterSlaveConflict:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectMasterSlaveConflict];
       case enumh245LogChanOpenLogChanRejectWaitForCommunicationMode:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectWaitForCommunicationMode];
       case enumh245LogChanOpenLogChanRejectInvalidDependentChannel:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChanOpenLogChanRejectInvalidDependentChannel];
       case enumh245LogChansOpenLogChanRejectReplacementForRejected:
           return pStatistic->h245LogChanOpenLogChanRejectCause[h245LogChansOpenLogChanRejectReplacementForRejected];

       case enumh245LogChanCloseLogChannels:
           return pStatistic->h245LogChanCloseLogChannels;
       case enumh245LogChanCloseLogChanAcks:
           return pStatistic->h245LogChanCloseLogChanAcks;
       case enumh245LogChanCloseLogChanRequests:
           return pStatistic->h245LogChanCloseLogChanRequests;
       case enumh245LogChanCloseLogChanRequestsAcks:
           return pStatistic->h245LogChanCloseLogChanRequestsAcks;
       case enumh245LogChanCloseLogChanRequestRejects:
           return pStatistic->h245LogChanCloseLogChanRequestRejects;
       case enumh245LogChanCloseLogChanRequestReleases:
           return pStatistic->h245LogChanCloseLogChanRequestReleases;
       default:
           return -1;
    }
}



#ifdef __cplusplus
}
#endif

