
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


#include <cmintr.h>
#include <cmutils.h>
#include <rasdef.h>
#include <rasin.h>
#include <rasout.h>
#include <rasutils.h>
#include <rasparams.h>




/************************************************************************
 *
 *                              Private constants
 *
 ************************************************************************/

static fieldNames rasEndpointTypes[] =
{
    {/*cmEndpointTypeTerminal*/  __q931(terminal)},
    {/*cmEndpointTypeGateway*/   __q931(gateway)},
    {/*cmEndpointTypeMCU*/       __q931(mcu)},
    {/*cmEndpointTypeGK*/        __q931(gatekeeper)},
    {/*cmEndpointTypeSET*/       __q931(set)},
    {LAST_TOKEN}
};

static fieldNames rasCallTypes[] =
{
    {/*cmCallTypeP2P*/   __q931(pointToPoint)},
    {/*cmCallTypeOne2N*/ __q931(oneToN)},
    {/*cmCallTypeN2One*/ __q931(nToOne)},
    {/*cmCallTypeN2Nw*/  __q931(nToN)},
    {LAST_TOKEN}
};

static fieldNames rasCallModels[] =
{
    {/*cmCallModelTypeDirect*/   __q931(direct)},
    {/*cmCallModelTypeGKRouted*/ __q931(gatekeeperRouted)},
    {LAST_TOKEN}
};

static fieldNames rasDisengageReasons[] =
{
    {/*cmRASDisengageReasonForcedDrop*/      __q931(forcedDrop)},
    {/*cmRASDisengageReasonNormalDrop*/      __q931(normalDrop)},
    {/*cmRASDisengageReasonUndefinedReason*/ __q931(undefinedReason)},
    {LAST_TOKEN}
};

static fieldNames rasRejectReasons[] =
{
    {/*cmRASReasonResourceUnavailable*/             __q931(resourceUnavailable)},
    {/*cmRASReasonInsufficientResources*/           __q931(insufficientResources)},
    {/*cmRASReasonInvalidRevision*/                 __q931(invalidRevision)},
    {/*cmRASReasonInvalidCallSignalAddress*/        __q931(invalidCallSignalAddress)},
    {/*cmRASReasonInvalidRASAddress*/               __q931(invalidRASAddress)},
    {/*cmRASReasonInvalidTerminalType*/             __q931(invalidTerminalType)},
    {/*cmRASReasonInvalidPermission*/               __q931(invalidPermission)},
    {/*cmRASReasonInvalidConferenceID*/             __q931(invalidConferenceID)},
    {/*cmRASReasonInvalidEndpointID*/               __q931(invalidEndpointIdentifier)},
    {/*cmRASReasonCallerNotRegistered*/             __q931(callerNotRegistered)},
    {/*cmRASReasonCalledPartyNotRegistered*/        __q931(calledPartyNotRegistered)},
    {/*cmRASReasonDiscoveryRequired*/               __q931(discoveryRequired)},
    {/*cmRASReasonDuplicateAlias*/                  __q931(duplicateAlias)},
    {/*cmRASReasonTransportNotSupported*/           __q931(transportNotSupported)},
    {/*cmRASReasonCallInProgress*/                  __q931(callInProgress)},
    {/*cmRASReasonRouteCallToGatekeeper*/           __q931(routeCallToGatekeeper)},
    {/*cmRASReasonRequestToDropOther*/              __q931(requestToDropOther)},
    {/*cmRASReasonNotRegistered*/                   __q931(notRegistered)},
    {/*cmRASReasonUndefined*/                       __q931(undefinedReason)},
    {/*cmRASReasonTerminalExcluded*/                __q931(terminalExcluded)},
    {/*cmRASReasonNotBound*/                        __q931(notBound)},
    {/*cmRASReasonNotCurrentlyRegistered*/          __q931(notCurrentlyRegistered)},
    {/*cmRASReasonRequestDenied*/                   __q931(requestDenied)},
    {/*cmRASReasonLocationNotFound*/                __q931(requestDenied)},
    {/*cmRASReasonSecurityDenial*/                  __q931(securityDenial)},
    {/*cmRASReasonTransportQOSNotSupported*/        __q931(transportQOSNotSupported)},
    {/*cmRASResourceUnavailable*/                   __q931(resourceUnavailable)},
    {/*cmRASReasonInvalidAlias*/                    __q931(invalidAlias)},
    {/*cmRASReasonPermissionDenied*/                __q931(permissionDenied)},
    {/*cmRASReasonQOSControlNotSupported*/          __q931(qosControlNotSupported)},
    {/*cmRASReasonIncompleteAddress*/               __q931(incompleteAddress)},
    {/*cmRASReasonFullRegistrationRequired*/        __q931(fullRegistrationRequired)},
    {/*cmRASReasonRouteCallToSCN*/                  __q931(routeCallToSCN)},
    {/*cmRASReasonAliasesInconsistent*/             __q931(aliasesInconsistent)},
    {/*cmRASReasonAdditiveRegistrationNotSupported*/__q931(additiveRegistrationNotSupported)},
    {/*cmRASReasonInvalidTerminalAliases*/          __q931(invalidTerminalAliases)},
    {/*cmRASReasonExceedsCallCapacity*/             __q931(exceedsCallCapacity)},
    {/*cmRASReasonCollectDestination*/              __q931(collectDestination)},
    {/*cmRASReasonCollectPIN*/                      __q931(collectPIN)},
    {/*cmRASReasonGenericData*/                     __q931(genericDataReason)},
    {/*cmRASReasonNeededFeatureNotSupported*/       __q931(neededFeatureNotSupported)},
    {/*cmRASReasonUnknownMessageResponse*/          __q931(unknownMessageResponse)},
    {/*cmRASReasonHopCountExceeded*/                __q931(hopCountExceeded)},
    {LAST_TOKEN}
};

static fieldNames rasTransportQOSs[] =
{
    {/*cmTransportQOSEndpointControlled*/   __q931(endpointControlled)},
    {/*cmTransportQOSGatekeeperControlled*/ __q931(gatekeeperControlled)},
    {/*cmTransportQOSNoControl*/            __q931(noControl)},
    {LAST_TOKEN}
};

static fieldNames rasUnregReasons[] =
{
    {/*cmRASUnregReasonReregistrationRequired*/ __q931(reregistrationRequired)},
    {/*cmRASUnregReasonTtlExpired*/             __q931(ttlExpired)},
    {/*cmRASUnregReasonSecurityDenial*/         __q931(securityDenial)},
    {/*cmRASUnregReasonUndefinedReason*/        __q931(undefinedReason)},
    {/*cmRASUnregReasonMaintenance*/            __q931(maintenance)},
    {LAST_TOKEN}
};








/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * RAS_RETURN_PATH
 * purpose: Make sure a path can be returned by a parameter
 *          This macro is used only by rasGetParamPath().
 * input  : param   - Parameter enumeration value
 * output : none
 * return : The given path as a static variable value
 ************************************************************************/
#define RAS_RETURN_PATH(path) \
{                                                                       \
    static INT16 fieldPath[] = path;                                    \
    return fieldPath;                                                   \
}


/************************************************************************
 * rasGetParamPath
 * purpose: Get the parameter's path inside a RAS message
 * input  : txType  - Type of transaction we're talking about
 *          param   - Parameter enumeration value
 * output : none
 * return : Path to the parameter on success
 *          NULL on failure
 ************************************************************************/
static INT16* rasGetParamPath(IN cmRASTransaction txType, IN cmRASParam param)
{
    switch (param)
    {
        case cmRASParamGatekeeperID:            RAS_RETURN_PATH({_q931(gatekeeperIdentifier) LAST_TOKEN});
        case cmRASParamRASAddress:              RAS_RETURN_PATH({_q931(rasAddress) LAST_TOKEN});
        case cmRASParamCallSignalAddress:       RAS_RETURN_PATH({_q931(callSignalAddress) LAST_TOKEN});
        case cmRASParamDiscoveryComplete:       RAS_RETURN_PATH({_q931(discoveryComplete) LAST_TOKEN});
        case cmRASParamEndpointVendor:          RAS_RETURN_PATH({_q931(endpointVendor) LAST_TOKEN});
        case cmRASParamCallType:                RAS_RETURN_PATH({_q931(callType) LAST_TOKEN});
        case cmRASParamCallModel:               RAS_RETURN_PATH({_q931(callModel) LAST_TOKEN});
        case cmRASParamEndpointID:              RAS_RETURN_PATH({_q931(endpointIdentifier) LAST_TOKEN});
        case cmRASParamDestInfo:                RAS_RETURN_PATH({_q931(destinationInfo) LAST_TOKEN});
        case cmRASParamDestExtraCallInfo:       RAS_RETURN_PATH({_q931(destExtraCallInfo) LAST_TOKEN});
        case cmRASParamDestCallSignalAddress:   RAS_RETURN_PATH({_q931(destCallSignalAddress) LAST_TOKEN});
        case cmRASParamSrcCallSignalAddress:    RAS_RETURN_PATH({_q931(srcCallSignalAddress) LAST_TOKEN});
        case cmRASParamBandwidth:               RAS_RETURN_PATH({_q931(bandWidth) LAST_TOKEN});
        case cmRASParamActiveMC:                RAS_RETURN_PATH({_q931(activeMC) LAST_TOKEN});
        case cmRASParamIrrFrequency:            RAS_RETURN_PATH({_q931(irrFrequency) LAST_TOKEN});
        case cmRASParamReplyAddress:            RAS_RETURN_PATH({_q931(replyAddress) LAST_TOKEN});
        case cmRASParamDisengageReason:         RAS_RETURN_PATH({_q931(disengageReason) LAST_TOKEN});
        case cmRASParamRejectedAlias:           RAS_RETURN_PATH({_q931(rejectReason) _q931(duplicateAlias) LAST_TOKEN});
        case cmRASParamDestinationIpAddress:    return NULL;
        case cmRASParamNonStandard:             RAS_RETURN_PATH({_q931(nonStandardData) LAST_TOKEN});
        case cmRASParamNonStandardData:         RAS_RETURN_PATH({_q931(nonStandardData) LAST_TOKEN});
        case cmRASParamCRV:                     RAS_RETURN_PATH({_q931(callReferenceValue) LAST_TOKEN});
        case cmRASParamMulticastTransaction:    return NULL;
        case cmRASParamTransportQOS:            RAS_RETURN_PATH({_q931(transportQOS) LAST_TOKEN});
        case cmRASParamKeepAlive:               RAS_RETURN_PATH({_q931(keepAlive) LAST_TOKEN});
        case cmRASParamTimeToLive:              RAS_RETURN_PATH({_q931(timeToLive) LAST_TOKEN});
        case cmRASParamDelay:                   RAS_RETURN_PATH({_q931(delay) LAST_TOKEN});
        case cmRASParamAlmostOutOfResources:    RAS_RETURN_PATH({_q931(almostOutOfResources) LAST_TOKEN});
        case cmRASParamAlternateGatekeeper:     RAS_RETURN_PATH({_q931(alternateGatekeeper) LAST_TOKEN});
        case cmRASParamAltGKInfo:               RAS_RETURN_PATH({_q931(altGKInfo) _q931(alternateGatekeeper) LAST_TOKEN});
        case cmRASParamAltGKisPermanent:        RAS_RETURN_PATH({_q931(altGKInfo) _q931(altGKisPermanent) LAST_TOKEN});
        case cmRASParamEmpty:                   return NULL;
        case cmRASParamNeedResponse:            RAS_RETURN_PATH({_q931(needResponse) LAST_TOKEN});
        case cmRASParamMaintainConnection:      RAS_RETURN_PATH({_q931(maintainConnection) LAST_TOKEN});
        case cmRASParamMultipleCalls:           RAS_RETURN_PATH({_q931(multipleCalls) LAST_TOKEN});
        case cmRASParamWillRespondToIRR:        RAS_RETURN_PATH({_q931(willRespondToIRR) LAST_TOKEN});
        case cmRASParamSupportsAltGk:           RAS_RETURN_PATH({_q931(supportsAltGK) LAST_TOKEN});
        case cmRASParamAdditiveRegistration:    RAS_RETURN_PATH({_q931(additiveRegistration) LAST_TOKEN});
        case cmRASParamSupportsAdditiveRegistration:RAS_RETURN_PATH({_q931(supportsAdditiveRegistration) LAST_TOKEN});
        case cmRASParamSegmentedResponseSupported:  RAS_RETURN_PATH({_q931(segmentedResponseSupported) LAST_TOKEN});
        case cmRASParamNextSegmentRequested:    RAS_RETURN_PATH({_q931(nextSegmentRequested) LAST_TOKEN});
        case cmRASParamCapacityInfoRequested:   RAS_RETURN_PATH({_q931(capacityInfoRequested) LAST_TOKEN});
        case cmRASParamHopCount:                RAS_RETURN_PATH({_q931(hopCount) LAST_TOKEN});
        case cmRASParamInvalidTerminalAlias:    RAS_RETURN_PATH({_q931(rejectReason) _q931(invalidTerminalAliases) _q931(terminalAlias) LAST_TOKEN});
        case cmRASParamUnregReason:             RAS_RETURN_PATH({_q931(reason) LAST_TOKEN});

        case cmRASParamRejectReason:
            if (txType != cmRASUnsolicitedIRR)
            {
                RAS_RETURN_PATH({_q931(rejectReason) LAST_TOKEN});
            }
            else /* INAK */
            {
                RAS_RETURN_PATH({_q931(nakReason) LAST_TOKEN});
            }

        case cmRASParamCID:
            if (txType != cmRASServiceControl)
            {
                RAS_RETURN_PATH({_q931(conferenceID) LAST_TOKEN});
            }
            else /* SCI */
                RAS_RETURN_PATH({_q931(callSpecific) _q931(conferenceID) LAST_TOKEN});

        case cmRASParamCallID:
            if (txType != cmRASServiceControl)
            {
                RAS_RETURN_PATH({_q931(callIdentifier) _q931(guid) LAST_TOKEN});
            }
            else /* SCI */
                RAS_RETURN_PATH({_q931(callSpecific) _q931(callIdentifier) _q931(guid) LAST_TOKEN});

        case cmRASParamEndpointType:
        case cmRASParamTerminalType:
            if (txType == cmRASRegistration) /* RRQ */
            {
                RAS_RETURN_PATH({_q931(terminalType) LAST_TOKEN});
            }
            else /* GRQ, IRR */
                RAS_RETURN_PATH({_q931(endpointType) LAST_TOKEN});

        case cmRASParamEndpointAlias:
        case cmRASParamTerminalAlias:
            if (txType == cmRASRegistration) /* RRQ, RCF */
            {
                RAS_RETURN_PATH({_q931(terminalAlias) LAST_TOKEN});
            }
            else /* GRQ, URQ, IRR */
                RAS_RETURN_PATH({_q931(endpointAlias) LAST_TOKEN});

        case cmRASParamAnswerCall:
        case cmRASParamAnsweredCall:
            switch (txType)
            {
                case cmRASAdmission: /* ARQ */
                    RAS_RETURN_PATH({_q931(answerCall) LAST_TOKEN});
                case cmRASServiceControl: /* SCI */
                    RAS_RETURN_PATH({_q931(callSpecific) _q931(answeredCall) LAST_TOKEN});
                default: /* BRQ, DRQ */
                    RAS_RETURN_PATH({_q931(answeredCall) LAST_TOKEN});
            }

        case cmRASParamSrcInfo:
        case cmRASParamSourceInfo:
            if (txType == cmRASAdmission) /* ARQ */
            {
                RAS_RETURN_PATH({_q931(srcInfo) LAST_TOKEN});
            }
            else /* LRQ */
                RAS_RETURN_PATH({_q931(sourceInfo) LAST_TOKEN});

        default:                                break;
    }

    return NULL;
}


/************************************************************************
 * rasGetParamEnumerationValues
 * purpose: Get a pointer to the list of enumeration values in their
 *          fieldId values. Each index in an array represents the enum
 *          value in the API.
 * input  : param   - Parameter enumeration value
 * output : none
 * return : Pointer to the enumeration's field IDs on success
 *          NULL on failure
 ************************************************************************/
fieldNames* rasGetParamEnumerationValues(IN cmRASParam param)
{
    switch (param)
    {
        case cmRASParamTerminalType:
        case cmRASParamEndpointType:            return rasEndpointTypes;
        case cmRASParamCallType:                return rasCallTypes;
        case cmRASParamCallModel:               return rasCallModels;
        case cmRASParamDisengageReason:         return rasDisengageReasons;
        case cmRASParamRejectReason:            return rasRejectReasons;
        case cmRASParamTransportQOS:            return rasTransportQOSs;
        case cmRASParamUnregReason:             return rasUnregReasons;
        default:                                return NULL;
    }
}


/************************************************************************
 * rasGetParamEnumerationSize
 * purpose: Get the size of the list of enumeration values.
 * input  : param   - Parameter enumeration value
 * output : none
 * return : Number of values in list
 ************************************************************************/
int rasGetParamEnumerationSize(IN cmRASParam param)
{
    switch(param)
    {
        case cmRASParamTerminalType:
        case cmRASParamEndpointType:            return sizeof(rasEndpointTypes);
        case cmRASParamCallType:                return sizeof(rasCallTypes);
        case cmRASParamCallModel:               return sizeof(rasCallModels);
        case cmRASParamDisengageReason:         return sizeof(rasDisengageReasons);
        case cmRASParamRejectReason:            return sizeof(rasRejectReasons);
        case cmRASParamTransportQOS:            return sizeof(rasTransportQOSs);
        case cmRASParamUnregReason:             return sizeof(rasUnregReasons);
        default:                                return RVERROR;
    }
}


/************************************************************************
 * rasBuildElement
 * purpose: Set a node inside a SEQUENCE OF. This function will return
 *          the nodeId of the existing index, or build a new one if the
 *          index given is just after the last one.
 * input  : ras     - RAS instance used
 *          nodeId  - Node ID of the SEQUENCE OF
 *          index   - Index value to build (0-based)
 * output : none
 * return : Node ID created on success
 *          Negative-value on failure
 ************************************************************************/
int rasBuildElement(
    IN rasModule*   ras,
    IN int          nodeId,
    IN int          index)
{
    int tempId;

    /* Make sure index is within range */
    if (index > pvtNumChilds(ras->hVal, nodeId))
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR, "rasBuildElement: Index %d out of range", index));
        return RVERROR;
    }

    if (index >= 0)
    {
        /* See if it exists */
        tempId = pvtGetByIndex(ras->hVal, nodeId, index+1, NULL);
    }
    else
    {
        /* We must be able to handle negative index values to support the
           way the gatekeeper adds his parameters...
           This means that we aloways add it in a negative value case. */
        tempId = -1;
    }

    if (tempId < 0)
    {
        /* Element doesn't exist - create is as the last element */
        if ((tempId=pvtAdd(ras->hVal, nodeId, RVERROR, 0, NULL, NULL)) <0)
            return RVERROR;
    }

    return tempId;
}





/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * rasGetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (1-based).
 *                    0 otherwise.
 *          value   - If the parameter value is a structure, the value
 *                    represents the length of the parameter.
 * output : value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself. Can be set to NULL if we're only interested
 *                    in its length.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasGetParam(
    IN  rasModule*       ras,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  OUT INT32*       value,
    IN  char*            svalue)
{
    int                 txType;
    rasInTx*            inTx = NULL;
    rasOutTx*           outTx = NULL;
    cmRASTransaction    transactionType;
    int                 rootId, txProperty, nodeId;
    int                 status;

    /* Get the right transaction type and the property database */
    txType = emaGetType((EMAElement)hsRas);
    switch (txType)
    {
        case RAS_OUT_TX:
        {
            outTx = rasGetOutgoing(hsRas);
            txProperty = outTx->txProperty;
            transactionType = outTx->transactionType;
            break;
        }
        case RAS_IN_TX:
        {
            inTx = rasGetIncoming(hsRas);
            txProperty = inTx->txProperty;
            transactionType = inTx->transactionType;
            break;
        }
        default:
            return RVERROR;
    }

    /* First, we check parameters that are not in the PVT */
    switch (param)
    {
        case cmRASParamDestinationIpAddress:
            if (txType == RAS_OUT_TX)
                memcpy(svalue, &outTx->destAddress, sizeof(cmRASTransport));
            else
                memcpy(svalue, &inTx->destAddress, sizeof(cmRASTransport));
            return 0;
        case cmRASParamMulticastTransaction:
            if (value != NULL)
            {
                if (txType == RAS_OUT_TX)
                    *value = outTx->isMulticast;
                else
                    *value = inTx->isMulticast;
            }
            return 0;
        case cmRASParamCallHandle:
            if (svalue != NULL)
            {
                if (txType == RAS_OUT_TX)
                    memcpy(svalue, &outTx->hsCall, sizeof(outTx->hsCall));
                else
                    memcpy(svalue, &inTx->hsCall, sizeof(inTx->hsCall));
            }
            return 0;
		case cmRASParamSocketDesc:
            if (value != NULL)
            {
                if (txType == RAS_OUT_TX)
                    *value = outTx->sd;
                else
                    *value = inTx->sd;
            }
            return 0;
        default:
            /* Go on - we've got to look inside the PVT */
            break;
    }

    /* Make sure we're dealing with the right root by the stage we're looking for */
    switch (trStage)
    {
        case cmRASTrStageRequest:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(request) _anyField LAST_TOKEN});
            break;
        case cmRASTrStageProgress:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(progress) _anyField LAST_TOKEN});
            break;
        case cmRASTrStageConfirm:
        case cmRASTrStageReject:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(response) _anyField LAST_TOKEN});
            break;
        default:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasGetParam: Bad stage given to function (%d)", trStage));
            return RVERROR;
    }

    /* Get the field for the parameter we're dealing with */
    nodeId = pvtGetByFieldIds(ras->hVal, rootId, rasGetParamPath(transactionType, param), NULL, NULL, NULL);

    /* We check the NULL OPTIONAL parameters here, since there's reason to do it
       before we know if the node we're holding is valid */
    switch (param)
    {
        case cmRASParamSupportsAltGk:
        case cmRASParamAdditiveRegistration:
        case cmRASParamSupportsAdditiveRegistration:
        case cmRASParamSegmentedResponseSupported:
        case cmRASParamCapacityInfoRequested:
            /* NULL OPTIONAL parameters - handled here */

            /* 0 - if node doesn't exist
               1 - if node exists */
            *value = (nodeId >= 0);
            return 0;

        default:
            break;
    }

    /* Make sure we've got a valid node */
    if (nodeId < 0)
    {
        switch (param)
        {
            case cmRASParamDestInfo:
            case cmRASParamSrcInfo:
            case cmRASParamSourceInfo:
            case cmRASParamTerminalAlias:
            case cmRASParamEndpointAlias:
            case cmRASParamDestExtraCallInfo:
            case cmRASParamRejectedAlias:
            case cmRASParamInvalidTerminalAlias:
            case cmRASParamEndpointID:
            case cmRASParamGatekeeperID:
                /* In case we're looking for an alias, we notify the application that
                   the length is 0 if we couldn't find it - the GK won't be able to
                   work properly without it... */
                if (svalue != NULL)
                    ((cmAlias *)svalue)->length = 0;
                break;

            case cmRASParamRASAddress:
            case cmRASParamCallSignalAddress:
            case cmRASParamDestCallSignalAddress:
            case cmRASParamSrcCallSignalAddress:
            case cmRASParamReplyAddress:
                /* Well, it seems like the GK and other applications just like getting their
                   error values with the parameters blanked out, so we had to add this memset()
                   over here - especially for them... */
                if (svalue != NULL)
                    memset(svalue, 0, sizeof(cmTransportAddress));
                break;

            default:
                break;
        }

        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN, "rasGetParam: Bad path for parameter %s in transaction 0x%x",
                 rasGetParamName(param), hsRas));
        return nodeId;
    }

    /* Check the parameter's value */
    switch (param)
    {
        case cmRASParamRASAddress:
        case cmRASParamCallSignalAddress:
        case cmRASParamDestCallSignalAddress:
        case cmRASParamSrcCallSignalAddress:
        case cmRASParamReplyAddress:
        {
            cmRASTransport* addr;
            int             synNodeId;
            pstNodeType     nodeType;

            if (svalue == NULL) return RVERROR;
            addr = (cmRASTransport *)svalue;
            memset(addr, 0, sizeof(cmRASTransport));

            /* See if it's a SEQUENCE OF or not */
            pvtGet(ras->hVal, nodeId, NULL, &synNodeId, NULL, NULL);
            nodeType = pstGetNodeType(ras->synProperty, synNodeId);
            if ((nodeType == pstSequenceOf) || (nodeType == pstSetOf))
                nodeId = pvtGetByIndex(ras->hVal, nodeId, index + 1, NULL);
            else
            {
                /* Make sure the index is 0 */
                if (index != 0)
                    return RVERROR;
            }

            /* Convert the address to a C struct */
            if (value) *value = sizeof(cmRASTransport);
            return cmVtToTA(ras->hVal, nodeId, addr);
        }

        case cmRASParamEndpointID:
        case cmRASParamGatekeeperID:
        {
            cmAlias* alias;
            INT32 length = 0;

            if (svalue == NULL) return RVERROR;
            alias = (cmAlias *)svalue;
            if (index != 0)
            {
                alias->length = 0;
                return RVERROR;
            }
            if (pvtGet(ras->hVal, nodeId, NULL, NULL, &length, NULL) < 0)
            {
                alias->length = 0;
                return RVERROR;
            }
            alias->length = (UINT16)length;
            pvtGetString(ras->hVal, nodeId, length, alias->string);

            if (param == cmRASParamEndpointID)
                alias->type = cmAliasTypeEndpointID;
            else
                alias->type = cmAliasTypeGatekeeperID;
            if (value) *value = sizeof(cmAlias);

            return 0;
        }

        case cmRASParamDestInfo:
        case cmRASParamSrcInfo:
        case cmRASParamSourceInfo:
        case cmRASParamTerminalAlias:
        case cmRASParamEndpointAlias:
        case cmRASParamDestExtraCallInfo:
        case cmRASParamRejectedAlias:
        case cmRASParamInvalidTerminalAlias:
        {
            cmAlias*    alias;
            int         synNodeId;
            pstNodeType nodeType;

            if (svalue == NULL) return RVERROR;
            alias = (cmAlias *)svalue;

            /* See if it's a SEQUENCE OF or not */
            pvtGet(ras->hVal, nodeId, NULL, &synNodeId, NULL, NULL);
            nodeType = pstGetNodeType(ras->synProperty, synNodeId);
            if ((nodeType == pstSequenceOf) || (nodeType == pstSetOf))
                nodeId = pvtGetByIndex(ras->hVal, nodeId, index + 1, NULL);
            else
            {
                /* Make sure the index is 0 */
                if (index != 0)
                {
                    alias->length = 0;
                    return RVERROR;
                }
            }

            /* Convert the alias */
            if (nodeId >= 0)
                return vtToAlias(ras->hVal, alias, nodeId);
            if (value) *value = sizeof(cmAlias);
            return RVERROR;
        }

        case cmRASParamIrrFrequency:
        case cmRASParamDiscoveryComplete:
        case cmRASParamActiveMC:
        case cmRASParamAnswerCall:
        case cmRASParamAnsweredCall:
        case cmRASParamBandwidth:
        case cmRASParamCRV:
        case cmRASParamKeepAlive:
        case cmRASParamTimeToLive:
        case cmRASParamDelay:
        case cmRASParamAlmostOutOfResources:
        case cmRASParamAltGKisPermanent:
        case cmRASParamNeedResponse:
        case cmRASParamMaintainConnection:
        case cmRASParamMultipleCalls:
        case cmRASParamWillRespondToIRR:
        case cmRASParamNextSegmentRequested:
        case cmRASParamHopCount:
            /* Integer parameters - handled here */
            if ((pvtGet(ras->hVal, nodeId, NULL, NULL, value, NULL) >= 0) && (index == 0))
            {
                if (param == cmRASParamBandwidth)
                    (*value) *= 100;
                return 0;
            }
            return RVERROR;

        case cmRASParamCID:
        case cmRASParamCallID:
            /* Strings of 16 bytes length */
            if (svalue == NULL) return RVERROR;
            if ((pvtGetString(ras->hVal, nodeId, 16, svalue) >= 0) && (index == 0))
                return 0;
            memset(svalue, 0, 16);
            return RVERROR;

        case cmRASParamEndpointVendor:
        {
            cmVendor*   vendor;
            INT32       length = 0;
            int         tempId;

            if ((svalue == NULL) || (index != 0)) return RVERROR;
            vendor = (cmVendor *)svalue;

            /* Get vendor information */
            tempId = pvtGetChild(ras->hVal, nodeId, __q931(vendor), NULL);

            memset(vendor, 0, sizeof(cmVendor));
            getNonStandard(ras->hVal, tempId, &vendor->info);

            tempId = pvtGetChildByFieldId(ras->hVal, nodeId, __q931(productId), &length, NULL);
            if (tempId >= 0)
            {
                pvtGetString(ras->hVal, tempId, sizeof(vendor->productID), vendor->productID);
                vendor->productLen = (int)length;
            }

            tempId = pvtGetChildByFieldId(ras->hVal, nodeId, __q931(versionId), &length, NULL);
            if (tempId >= 0)
            {
                pvtGetString(ras->hVal, tempId, sizeof(vendor->versionID), vendor->versionID);
                vendor->versionLen = (int)length;
            }
            return 0;
        }

        case cmRASParamAltGKInfo:
        case cmRASParamAlternateGatekeeper:
        {
            cmAlternateGatekeeper* altGk;
            int tempId;
            INT32 length = 0;

            if (svalue == NULL) return RVERROR;
            altGk = (cmAlternateGatekeeper *)svalue;

            /* Make sure we pass the SEQUENCE OF */
            nodeId = pvtGetByIndex(ras->hVal, nodeId, index + 1, NULL);

            /* Build the struct */
            tempId = pvtGetChild(ras->hVal, nodeId, __q931(rasAddress), NULL);
            if (tempId >= 0)
                status = cmVtToTA(ras->hVal, tempId, &altGk->rasAddress);
            else
                return RVERROR;

            altGk->gatekeeperIdentifier.length = 0;
            tempId = pvtGetChild(ras->hVal, nodeId, __q931(gatekeeperIdentifier), NULL);
            if (tempId >= 0)
            {
                if (pvtGet(ras->hVal, tempId, NULL, NULL, &length, NULL) < 0) return RVERROR;
                pvtGetString(ras->hVal, tempId, length, altGk->gatekeeperIdentifier.string);
                altGk->gatekeeperIdentifier.length = (UINT16)length;
            }

            __pvtGetByFieldIds(status, ras->hVal, nodeId,
                               {_q931(needToRegister) LAST_TOKEN}, NULL, &length, NULL);
            altGk->needToRegister = (BOOL)length;
            __pvtGetByFieldIds(status, ras->hVal, nodeId,
                               {_q931(priority) LAST_TOKEN}, NULL, &length, NULL);
            altGk->priority = (BYTE)length;

            if (value) *value = sizeof(cmAlternateGatekeeper);
            return status;
        }

        case cmRASParamNonStandardData:
            /* Return the nodeId's value */
            if (index != 0) return RVERROR;
            *value = nodeId;
            return 0;

        case cmRASParamNonStandard:
            if ((svalue == NULL) || (index != 0)) return RVERROR;
            if (value) *value = sizeof(cmNonStandardParam);
            return getNonStandardParam(ras->hVal, nodeId, (cmNonStandardParam*)svalue);

        case cmRASParamCallType:
        case cmRASParamCallModel:
        case cmRASParamDisengageReason:
        case cmRASParamRejectReason:
        case cmRASParamTransportQOS:
        case cmRASParamUnregReason:
        {
            /* Enumerations are handled here */
            INTPTR fieldId;
            if ((pvtGet(ras->hVal, pvtChild(ras->hVal, nodeId), &fieldId, NULL, NULL, NULL) >= 0) && (index == 0))
            {
                /* Match the fieldId with the possible ones and set the value */
                *value = getEnumValueByNameId(rasGetParamEnumerationValues(param), fieldId);
                return 0;
            }
            *value = RVERROR;
            return RVERROR;
        }

        case cmRASParamTerminalType:
        case cmRASParamEndpointType:
        {
            /* Terminal type - we check each of the values.
             * The first one we find is the right one - except for Gateway.
             */
            fieldNames* names;
            int i;
            *value = RVERROR;
            names = rasGetParamEnumerationValues(param);

            for(i = 0; names[i].nameId != LAST_TOKEN; i++)
                if (pvtGetChild(ras->hVal, nodeId, names[i].nameId, NULL) >= 0)
                {
                    *value = i;
                    if (i == cmEndpointTypeGateway) continue;
                    return 0;
                }

            if (*value == RVERROR)
                return RVERROR;
            return 0;
        }

        default:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasGetParam: Bad parameter %d", param));
            return RVERROR;
    }
}



/************************************************************************
 * rasSetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (1-based).
 *                    0 otherwise.
 *          value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSetParam(
    IN  rasModule*      ras,
    IN  HRAS            hsRas,
    IN  cmRASTrStage    trStage,
    IN  cmRASParam      param,
    IN  int             index,
    IN  INT32           value,
    IN  char*           svalue)
{
    int                 txType;
    rasInTx*            inTx = NULL;
    rasOutTx*           outTx = NULL;
    cmRASTransaction    transactionType;
    int                 rootId, txProperty, nodeId;
    int                 status;
    int                 iLen;

    /* Get the right transaction type and the property database */
    txType = emaGetType((EMAElement)hsRas);
    switch (txType)
    {
        case RAS_OUT_TX:
        {
            outTx = rasGetOutgoing(hsRas);
            txProperty = outTx->txProperty;
            transactionType = outTx->transactionType;
            break;
        }
        case RAS_IN_TX:
        {
            inTx = rasGetIncoming(hsRas);
            txProperty = inTx->txProperty;
            transactionType = inTx->transactionType;

            /* Build a response if one doesn't already exist */
            if ((inTx->responseSet == rasNoResponseSet) && (trStage != cmRASTrStageRequest))
            {
                int responseNodeId;
                responseNodeId = pvtAdd(ras->hVal, txProperty, __q931(response), 0, NULL, NULL);
                pvtSetTree(ras->hVal, responseNodeId, ras->hVal, ras->defaultMessages[inTx->transactionType][trStage]);

                switch(trStage)
                {
                case (cmRASTrStageConfirm) : inTx->responseSet = rasConfirmSet; break;
                case (cmRASTrStageReject)  : inTx->responseSet = rasRejectSet; break;
                default                    : break;
                }
            }
            else if( ((trStage == cmRASTrStageConfirm) && (inTx->responseSet == rasRejectSet)) ||
                     ((trStage == cmRASTrStageReject) && (inTx->responseSet == rasConfirmSet)) )
            {
                int responseNodeId;                
                pvtGetChild(ras->hVal, txProperty, __q931(response), &responseNodeId);
                pvtSetTree(ras->hVal, responseNodeId, ras->hVal, ras->defaultMessages[inTx->transactionType][trStage]);
                
                switch(trStage)
                {
                case (cmRASTrStageConfirm) : inTx->responseSet = rasConfirmSet; break;
                case (cmRASTrStageReject)  : inTx->responseSet = rasRejectSet; break;
                default                    : break;
                }
            }                       
            break;
        }
        default:
            return RVERROR;
    }

    /* First, we check parameters that are not in the PVT */
    switch (param)
    {
        case cmRASParamDestinationIpAddress:
            if (txType == RAS_OUT_TX)
                memcpy(&outTx->destAddress, svalue, sizeof(cmRASTransport));
            else
                memcpy(&inTx->destAddress, svalue, sizeof(cmRASTransport));
            return 0;
        case cmRASParamMulticastTransaction:
            if (txType == RAS_OUT_TX)
            {
                outTx->isMulticast = value;
                if (outTx->isMulticast)
                {
                    /* Set the address as the one we recognize as the multicast address */
                    int srcNodeId;
                    srcNodeId = pvtGetChild(ras->hVal, ras->confNode, __q931(rasMulticastAddress), NULL);
                    cmVtToTA(ras->hVal, srcNodeId, &outTx->destAddress);
                }
            }
            else
                inTx->isMulticast = value;
            return 0;
        case cmRASParamCallHandle:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasSetParam: cmRASParamCallHandle is read only"));
            return RVERROR;
        case cmRASParamSocketDesc:
            if (txType == RAS_OUT_TX)
                outTx->sd = value;
            else
                inTx->sd = value;
            return 0;
        case cmRASParamEmpty:
            /* Make sure we don't process this one - it's only here so we'll have a response... */
            return 0;
        default:
            /* Go on - we've got to look inside the PVT */
            break;
    }

    /* Make sure we're dealing with the right root by the stage we're looking for */
    switch (trStage)
    {
        case cmRASTrStageRequest:
            __pvtBuildByFieldIds(rootId, ras->hVal, txProperty, {_q931(request) _anyField LAST_TOKEN}, 0, NULL);
            break;
        case cmRASTrStageProgress:
            __pvtBuildByFieldIds(rootId, ras->hVal, txProperty, {_q931(progress) _anyField LAST_TOKEN}, 0, NULL);
            break;
        case cmRASTrStageConfirm:
        case cmRASTrStageReject:
            __pvtBuildByFieldIds(rootId, ras->hVal, txProperty, {_q931(response) _anyField LAST_TOKEN}, 0, NULL);
            break;
        default:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasSetParam: Bad stage given to function (%d)", trStage));
            return RVERROR;
    }

    /* Get the field for the parameter we're dealing with */
    nodeId = pvtBuildByFieldIds(ras->hVal, rootId, rasGetParamPath(transactionType, param), 0, NULL);
    if (nodeId < 0)
    {
        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN, "rasSetParam: Bad path for parameter %s in transaction 0x%x",
                 rasGetParamName(param), hsRas));
        return nodeId;
    }

    switch (param)
    {
        case cmRASParamRASAddress:
        case cmRASParamCallSignalAddress:
        case cmRASParamDestCallSignalAddress:
        case cmRASParamSrcCallSignalAddress:
        case cmRASParamReplyAddress:
        {
            int             synNodeId;
            pstNodeType     nodeType;

            /* See if it's a SEQUENCE OF or not */
            pvtGet(ras->hVal, nodeId, NULL, &synNodeId, NULL, NULL);
            nodeType = pstGetNodeType(ras->synProperty, synNodeId);
            if ((nodeType == pstSequenceOf) || (nodeType == pstSetOf))
            {
                /* We've got an index inside the SEQUENCE OF */
                nodeId = rasBuildElement(ras, nodeId, index);
            }
            else
            {
                /* Make sure the index is 0 */
                if (index != 0)
                    return RVERROR;
            }

            /* Convert the C struct to an address */
            return cmTAToVt(ras->hVal, nodeId, (cmTransportAddress *)svalue);
        }

        case cmRASParamEndpointID:
        case cmRASParamGatekeeperID:
        {
            cmAlias* alias;

            if (svalue == NULL) return RVERROR;
            alias = (cmAlias *)svalue;

            /* Make sure the index is 0 */
            if (index != 0) return RVERROR;

            iLen = alias->length;
            return pvtSet(ras->hVal, nodeId, -1, iLen, alias->string);
        }

        case cmRASParamDestInfo:
        case cmRASParamSrcInfo:
        case cmRASParamSourceInfo:
        case cmRASParamTerminalAlias:
        case cmRASParamEndpointAlias:
        case cmRASParamDestExtraCallInfo:
        case cmRASParamRejectedAlias:
        case cmRASParamInvalidTerminalAlias:
        {
            cmAlias*    alias;
            int         synNodeId;
            pstNodeType nodeType;

            if (svalue == NULL) return RVERROR;
            alias = (cmAlias *)svalue;

            /* See if it's a SEQUENCE OF or not */
            pvtGet(ras->hVal, nodeId, NULL, &synNodeId, NULL, NULL);
            nodeType = pstGetNodeType(ras->synProperty, synNodeId);
            if ((nodeType == pstSequenceOf) || (nodeType == pstSetOf))
            {
                /* We've got an index inside the SEQUENCE OF */
                nodeId = rasBuildElement(ras, nodeId, index);
            }
            else
            {
                /* Make sure the index is 0 */
                if (index != 0)
                {
                    alias->length = 0;
                    return RVERROR;
                }
            }

            /* Convert the alias */
            return aliasToVt(ras->hVal, alias, nodeId);
        }

        case cmRASParamSupportsAltGk:
        case cmRASParamAdditiveRegistration:
        case cmRASParamSupportsAdditiveRegistration:
        case cmRASParamSegmentedResponseSupported:
        case cmRASParamCapacityInfoRequested:
            /* NULL OPTIONAL parameters - handled here */
            if (value == 0)
            {
                /* We actually need to remove this parameter... */
                return pvtDelete(ras->hVal, nodeId);
            }

            /* If we built it - it's fine */
            return 0;

        case cmRASParamBandwidth:
            /* We make sure the bandwidth parameter is divided by 100 */
            value /= 100;

        case cmRASParamIrrFrequency:
        case cmRASParamDiscoveryComplete:
        case cmRASParamActiveMC:
        case cmRASParamAnswerCall:
        case cmRASParamAnsweredCall:
        case cmRASParamCRV:
        case cmRASParamKeepAlive:
        case cmRASParamTimeToLive:
        case cmRASParamDelay:
        case cmRASParamAlmostOutOfResources:
        case cmRASParamAltGKisPermanent:
        case cmRASParamNeedResponse:
        case cmRASParamMaintainConnection:
        case cmRASParamMultipleCalls:
        case cmRASParamWillRespondToIRR:
        case cmRASParamNextSegmentRequested:
        case cmRASParamHopCount:
            /* Integer parameters - handled here */
            return pvtSet(ras->hVal, nodeId, -1, value, NULL);

        case cmRASParamCID:
        case cmRASParamCallID:
            /* Strings of 16 bytes length */
            return pvtSet(ras->hVal, nodeId, -1, 16, svalue);

        case cmRASParamEndpointVendor:
        {
            cmVendor* vendor;
            int tempId;
            int status;
            vendor = (cmVendor *)svalue;

            tempId = pvtAdd(ras->hVal, nodeId, __q931(vendor), 0, NULL, NULL);
            status = setNonStandard(ras->hVal, tempId, &vendor->info);

            if ((status >= 0) && (vendor->productLen > 0))
                status = pvtAdd(ras->hVal, nodeId, __q931(productId), vendor->productLen, vendor->productID, NULL);
            if ((status >= 0) && (vendor->versionLen > 0))
                status = pvtAdd(ras->hVal, nodeId, __q931(versionId), vendor->versionLen, vendor->versionID, NULL);
            return status;
        }

        case cmRASParamAltGKInfo:
        case cmRASParamAlternateGatekeeper:
            {
                int tempId;
                cmAlternateGatekeeper* altGk = (cmAlternateGatekeeper *)svalue;

                nodeId = rasBuildElement(ras, nodeId, index);

                tempId = pvtAdd(ras->hVal, nodeId, __q931(rasAddress), 0, NULL, NULL);
                status = cmTAToVt(ras->hVal, tempId, &altGk->rasAddress);

                if (status >= 0)
                {
                    tempId = pvtAdd(ras->hVal, nodeId, __q931(gatekeeperIdentifier), 0, NULL, NULL);
                    status = aliasToVt(ras->hVal, &altGk->gatekeeperIdentifier, tempId);
                }

                if (status >= 0)
                    status = pvtAdd(ras->hVal, nodeId, __q931(needToRegister), altGk->needToRegister, NULL, NULL);
                if (status >= 0)
                {
                    int priority = altGk->priority;
                    status = pvtAdd(ras->hVal, nodeId, __q931(priority), priority, NULL, NULL);
                }
                return status;
            }

        case cmRASParamNonStandardData:
            /* NodeIDs replacing old ones */
            return pvtShiftTree(ras->hVal, nodeId, value);

        case cmRASParamNonStandard:
            return setNonStandardParam(ras->hVal, nodeId, (cmNonStandardParam *)svalue);

        case cmRASParamCallType:
        case cmRASParamCallModel:
        case cmRASParamDisengageReason:
        case cmRASParamRejectReason:
        case cmRASParamTransportQOS:
        case cmRASParamEndpointType:
        case cmRASParamTerminalType:
        case cmRASParamUnregReason:
            return pvtAdd(ras->hVal, nodeId,
                          getEnumNameId(rasGetParamEnumerationValues(param),
                                        rasGetParamEnumerationSize(param),
                                        (int)value),
                          0, NULL, NULL);

        default:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasSetParam: Bad parameter %d", param));
            return RVERROR;
    }
}


/************************************************************************
 * rasGetNumOfParams
 * purpose: Returns the number of params in sequences on the property
 *          tree.
 * input  : ras     - RAS instance we're using
 *          hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 * output : none
 * return : Number of params in sequence on the property tree on success
 *          Negative value on failure
 ************************************************************************/
int rasGetNumOfParams(
    IN  rasModule*      ras,
    IN  HRAS            hsRas,
    IN  cmRASTrStage    trStage,
    IN  cmRASParam      param)
{
    int                 txType;
    rasInTx*            inTx = NULL;
    rasOutTx*           outTx = NULL;
    cmRASTransaction    transactionType;
    int                 rootId, txProperty, nodeId;
    pstNodeType         nodeType;
    int                 synNodeId;

    /* Get the right transaction type and the property database */
    txType = emaGetType((EMAElement)hsRas);
    switch (txType)
    {
        case RAS_OUT_TX:
        {
            outTx = rasGetOutgoing(hsRas);
            txProperty = outTx->txProperty;
            transactionType = outTx->transactionType;
            break;
        }
        case RAS_IN_TX:
        {
            inTx = rasGetIncoming(hsRas);
            txProperty = inTx->txProperty;
            transactionType = inTx->transactionType;
            break;
        }
        default:
            return RVERROR;
    }

    /* Make sure we're dealing with the right root by the stage we're looking for */
    switch (trStage)
    {
        case cmRASTrStageRequest:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(request) _anyField LAST_TOKEN});
            break;
        case cmRASTrStageProgress:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(progress) _anyField LAST_TOKEN});
            break;
        case cmRASTrStageConfirm:
        case cmRASTrStageReject:
            __pvtGetNodeIdByFieldIds(rootId, ras->hVal, txProperty, {_q931(response) _anyField LAST_TOKEN});
            break;
        default:
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasGetNumOfParams: Bad stage given to function (%d)", trStage));
            return RVERROR;
    }

    /* Get the field for the parameter we're dealing with */
    nodeId = pvtGetByFieldIds(ras->hVal, rootId, rasGetParamPath(transactionType, param), NULL, NULL, NULL);
    if (nodeId < 0)
    {
        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN, "rasGetNumOfParams: Bad path for parameter %s in transaction 0x%x",
                 rasGetParamName(param), hsRas));
        return nodeId;
    }

    /* Make sure this parameter is actually a SEQUENCE OF */
    pvtGet(ras->hVal, nodeId, NULL, &synNodeId, NULL, NULL);
    nodeType = pstGetNodeType(ras->synProperty, synNodeId);
    if ((nodeType == pstSequenceOf) || (nodeType == pstSetOf))
        return pvtNumChilds(ras->hVal, nodeId);

    logPrint(ras->log, RV_ERROR,
             (ras->log, RV_ERROR, "rasGetNumOfParams: Parameter %s is not an array type",
             rasGetParamName(param)));
    return RVERROR;
}



#ifdef __cplusplus
}
#endif


