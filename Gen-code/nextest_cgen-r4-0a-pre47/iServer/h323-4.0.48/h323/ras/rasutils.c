
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

#include <psyntreeStackApi.h>
#include <pvaltreeStackApi.h>
#include <pvaltree.h>
#include <prnutils.h>
#include <cmutils.h>
#include <rasdef.h>
#include <rasin.h>
#include <rasout.h>
#include <rasirr.h>
#include <rasparams.h>
#include <rasutils.h>
#include <cmintr.h>



/************************************************************************
 * rasGetOutgoing
 * purpose: Get the outgoing RAS transaction from its handle
 * input  : hsRas   - Application handle
 * output : none
 * return : RAS outgoing transaction on success
 *          NULL on failure
 ************************************************************************/
#ifdef RV_RAS_DEBUG
rasOutTx* rasGetOutgoing(IN HRAS hsRas)
{
    rasOutTx*   tx;

    if (hsRas == NULL) return NULL;

    tx = (rasOutTx *)hsRas;

    /* Make sure the pointer is an outgoing transaction */
    if (emaGetType((EMAElement)hsRas) != RAS_OUT_TX)
    {
        RVHLOG log;
        log = ((rasModule *)emaGetUserData((EMAElement)hsRas))->log;

        logPrint(log, RV_EXCEP,
                 (log, RV_EXCEP,
                 "rasGetOutgoing: Not an outgoing transaction 0x%x", hsRas));
        return NULL;
    }

    return tx;
}
#endif  /* RV_RAS_DEBUG */



/************************************************************************
 * rasGetIncoming
 * purpose: Get the incoming RAS transaction from its handle
 * input  : hsRas   - Application handle
 * output : none
 * return : RAS incoming transaction on success
 *          NULL on failure
 ************************************************************************/
#ifdef RV_RAS_DEBUG
rasInTx* rasGetIncoming(IN HRAS hsRas)
{
    rasInTx*   tx;

    if (hsRas == NULL) return NULL;

    tx = (rasInTx *)hsRas;

    /* Make sure the pointer is an incoming transaction */
    if (emaGetType((EMAElement)hsRas) != RAS_IN_TX)
    {
        RVHLOG log;
        log = ((rasModule *)emaGetUserData((EMAElement)hsRas))->log;

        logPrint(log, RV_EXCEP,
                 (log, RV_EXCEP,
                 "rasGetIncoming: Not an incoming transaction 0x%x", hsRas));
        return NULL;
    }

    return tx;
}
#endif  /* RV_RAS_DEBUG */



/************************************************************************
 * rasGetParamName
 * purpose: Get the parameter's name
 * input  : param   - Parameter enumeration value
 * output : none
 * return : Parameter's name on success
 *          NULL on failure
 ************************************************************************/
char *rasGetParamName(IN cmRASParam param)
{
#ifndef NOLOGSUPPORT
    switch(param)
    {
        case cmRASParamGatekeeperID:                 return (char*)"gatekeeperIdentifier";
        case cmRASParamRASAddress:                   return (char*)"rasAddress";
        case cmRASParamCallSignalAddress:            return (char*)"callSignalAddress";
        case cmRASParamEndpointType:                 return (char*)"endpointType";
        case cmRASParamTerminalType:                 return (char*)"terminalType";
        case cmRASParamEndpointAlias:                return (char*)"endpointAlias";
        case cmRASParamTerminalAlias:                return (char*)"terminalAlias";
        case cmRASParamDiscoveryComplete:            return (char*)"discoveryComplete";
        case cmRASParamEndpointVendor:               return (char*)"endpointVendor";
        case cmRASParamCallType:                     return (char*)"callType";
        case cmRASParamCallModel:                    return (char*)"callModel";
        case cmRASParamEndpointID:                   return (char*)"endpointIdentifier";
        case cmRASParamDestInfo:                     return (char*)"destinationInfo";
        case cmRASParamSrcInfo:                      return (char*)"srcInfo";
        case cmRASParamDestExtraCallInfo:            return (char*)"destExtraCallInfo";
        case cmRASParamDestCallSignalAddress:        return (char*)"destCallSignalAddress";
        case cmRASParamSrcCallSignalAddress:         return (char*)"srcCallSignalAddress";
        case cmRASParamBandwidth:                    return (char*)"bandWidth";
        case cmRASParamActiveMC:                     return (char*)"activeMC";
        case cmRASParamAnswerCall:                   return (char*)"answerCall";
        case cmRASParamIrrFrequency:                 return (char*)"irrFrequency";
        case cmRASParamReplyAddress:                 return (char*)"replyAddress";
        case cmRASParamDisengageReason:              return (char*)"disengageReason";
        case cmRASParamRejectedAlias:                return (char*)"rejectReason.duplicateAlias";
        case cmRASParamRejectReason:                 return (char*)"rejectReason";
        case cmRASParamCID:                          return (char*)"conferenceID";
        case cmRASParamDestinationIpAddress:         return (char*)"destinationIpAddress";
        case cmRASParamNonStandard:                  return (char*)"nonStandard";
        case cmRASParamNonStandardData:              return (char*)"nonStandardData";
        case cmRASParamCRV:                          return (char*)"callReferenceValue";
        case cmRASParamMulticastTransaction:         return (char*)"multicast";
        case cmRASParamTransportQOS:                 return (char*)"transportQOS";
        case cmRASParamKeepAlive:                    return (char*)"keepAlive";
        case cmRASParamTimeToLive:                   return (char*)"timeToLive";
        case cmRASParamDelay:                        return (char*)"delay";
        case cmRASParamCallID:                       return (char*)"callIdentifier.guid";
        case cmRASParamAnsweredCall:                 return (char*)"answeredCall";
        case cmRASParamAlmostOutOfResources:         return (char *)"almostOutOfResources";
        case cmRASParamAlternateGatekeeper:          return (char *)"alternateGatekeeper";
        case cmRASParamAltGKInfo:                    return (char *)"altGKInfo.alternateGatekeeper";
        case cmRASParamAltGKisPermanent:             return (char *)"altGKInfo.altGKisPermanent";
        case cmRASParamEmpty:                        return NULL;
        case cmRASParamSourceInfo:                   return (char*)"sourceInfo";
        case cmRASParamNeedResponse:                 return (char*)"needResponse";
        case cmRASParamMaintainConnection:           return (char*)"maintainConnection";
        case cmRASParamMultipleCalls:                return (char*)"multipleCalls";
        case cmRASParamWillRespondToIRR:             return (char*)"willRespondToIRR";
        case cmRASParamSupportsAltGk:                return (char*)"supportsAltGk";
        case cmRASParamAdditiveRegistration:         return (char*)"additiveRegistration";
        case cmRASParamSupportsAdditiveRegistration: return (char*)"supportsAdditiveRegistration";
        case cmRASParamSegmentedResponseSupported:   return (char*)"segmentedResponseSupported";
        case cmRASParamNextSegmentRequested:         return (char*)"nextSegmentRequested";
        case cmRASParamCapacityInfoRequested:        return (char*)"capacityInfoRequested";
        case cmRASParamHopCount:                     return (char*)"hopCount";
        case cmRASParamInvalidTerminalAlias:         return (char*)"invalidTerminalAlias";
        case cmRASParamUnregReason:                  return (char*)"unregReason";
        case cmRASParamIrrStatus:                    return (char*)"irrStatus";
        case cmRASParamCallHandle:                   return (char*)"callHandle";
        default:                                     break;
    }
#else
    if (param);
#endif  /* NOLOGSUPPORT */

    return NULL;
}


/************************************************************************
 * rasCfgGetTimeout
 * purpose: Get the timeout of a transaction before a retry is sent from
 *          the configuration
 * input  : ras     - RAS instance handle
 * output : none
 * return : Timeout in milliseconds
 ************************************************************************/
int rasCfgGetTimeout(IN rasModule* ras)
{
    int nodeId, status;
    INT32 timeout;

    /* Check in the configuration */
    nodeId = pvtGetChild(ras->hVal, ras->confNode, __q931(responseTimeOut), NULL);
    status = pvtGet(ras->hVal, nodeId, NULL, NULL, &timeout, NULL);

    if (status >= 0)
        return (timeout * 1000);
    else
        return 4000; /* Default of 4 seconds */
}


/************************************************************************
 * rasCfgGetRetries
 * purpose: Get the number of retries for a transaction before a timeout
 *          from the configuration
 * input  : ras     - RAS instance handle
 * output : none
 * return : Number of retries
 ************************************************************************/
int rasCfgGetRetries(IN rasModule* ras)
{
    int nodeId, status;
    INT32 retries;

    /* Set the number of retries */
    nodeId = pvtGetChild(ras->hVal, ras->confNode, __q931(maxRetries), NULL);
    status = pvtGet(ras->hVal, nodeId, NULL, NULL, &retries, NULL);

    if (status >= 0)
        return retries;
    else
        return 3; /* 3 retries on default */
}


/************************************************************************
 * rasRetransmit
 * purpose: Retransmit a message to the other side
 *          This function is used for both requests and responses.
 * input  : ras         - RAS instance handle
 *          rasTx       - RAS transaction of the message
 *          rpootMsg    - RPOOL message handle to retransmit
 *          destAddr    - Destination address
 *          typeStr     - Type of retransmission
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasRetransmit(
    IN  rasModule*          ras,
    IN  HRAS                rasTx,
    IN  void*               rpoolMsg,
    IN  cmTransportAddress* destAddr,
    IN  const char*         typeStr)
{
    int messageSize, status;
    BYTE* buffer;
    if (rasTx || typeStr);

    logPrint(ras->log, RV_DEBUG,
             (ras->log, RV_DEBUG,
             "rasRetransmit: Retransmitting %s on 0x%x", typeStr, rasTx));

    getEncodeDecodeBuffer((int)ras->bufferSize, &buffer);

    /* Get message from RPOOL to a single buffer */
    messageSize = rpoolChunkSize(ras->messages, rpoolMsg);
    rpoolCopyToExternal(ras->messages, buffer, rpoolMsg, 0, messageSize);

    /* Send the message. We always send through the unicast port */
    status = ras->evSendMessage(ras->app, rasChanUnicast, destAddr, buffer, messageSize);
    if (status < 0)
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "rasRetransmit: Couldn't send the message 0x%x: %d", rasTx, status));
    }

    return status;
}


/************************************************************************
 * rasEncodeAndSend
 * purpose: Encode and send a message on the net
 * input  : ras         - RAS instance handle
 *          rasTx       - RAS transaction of the message
 *          stage       - Stage of the RAS transaction
 *          nodeId      - nodeId of the message to send out
 *          isMulticast - Are we sending it to a multicast address
 *          destAddr    - Destination address
 *          storeInRPOOL- Indicate if we want the message to be stored in RPOOL
 * output : rpoolHandle - RPOOL handle storing the message
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasEncodeAndSend(
    IN  rasModule*          ras,
    IN  HRAS                rasTx,
    IN  cmRASTrStage        stage,
    IN  int                 nodeId,
    IN  BOOL                isMulticast,
    IN  cmTransportAddress* destAddr,
    IN  BOOL                storeInRPOOL,
    OUT void**              rpoolHandle)
{
    cmHookSendToT   sendHook;
    int             status = 0;
    int             addrNodeId;
    int             encodedSize;
    BOOL            process, printMsg;
    INTPTR          fieldId;
	int 			socketDesc = 0;
    if (isMulticast);

    sendHook = cmiGetRasHooks(ras->app)->hookSendTo;
    printMsg = logIsSelected(ras->logChan, RV_DEBUG);

    if (printMsg || (sendHook != NULL))
    {
        /* Create a transport address node id */
        /* todo: Don't use cmElem */
        addrNodeId = pvtAddRoot(ras->hVal, ((cmElem*)(ras->app))->hAddrSyn, 0, NULL);
        if (addrNodeId < 0) return addrNodeId;
        cmTAToVt(ras->hVal, addrNodeId, destAddr);
    }
    else
        addrNodeId = RVERROR;

    /* Make sure to notify the application of an outgoing RAS message */
    if (ras->evMessages.cmEvRasMessageSend != NULL)
    {
        /* An internal callback - no log for this one */
        ras->evMessages.cmEvRasMessageSend(nodeId, stage, rasTx, ras->evMessages.hAppRasMsg);
    }

    /* Call the hook before we encode this message */
    if (sendHook != NULL)
        process = !sendHook((HPROTCONN)&ras->unicastAppHandle, nodeId, addrNodeId, FALSE);
    else
        process = TRUE;

    /* Make sure we're still sending out this message */
    if (process)
    {
        BYTE*           buffer;

        getEncodeDecodeBuffer((int)ras->bufferSize, &buffer);

        /* Encode the message in PER */
        if (ras->cmiEvRASSendRawMessage)
        {
            /* We've got an encoding callback - probably from security app. Call it */
            status = ras->cmiEvRASSendRawMessage(
                ras->app,
                (HPROTCONN)&ras->unicastAppHandle,
                rasTx,
                (HAPPRAS)emaGetApplicationHandle((EMAElement)rasTx),
                nodeId,
                (int)ras->bufferSize,
                buffer,
                &encodedSize);
        }
        else
        {
            /* We encode the message on our own */
            status = cmEmEncode(ras->hVal, nodeId, buffer, (int)ras->bufferSize, &encodedSize);
        }

        if (status >= 0)
        {
            if (storeInRPOOL)
            {
                /* Make sure we store this message somewhere */
                meiEnter(ras->lockMessages);
                *rpoolHandle = rpoolAllocCopyExternal(ras->messages, buffer, encodedSize);
                meiExit(ras->lockMessages);
            }

            /* Print the message */
            pvtGet(ras->hVal, pvtChild(ras->hVal, nodeId), &fieldId, NULL, NULL, NULL);
            logPrint(ras->logChan, RV_INFO,
                     (ras->logChan, RV_INFO, "New message (channel %d)  sent --> %s:",
                     isMulticast, nprn(pstGetFieldNamePtr(ras->synMessage, fieldId))));

            if (printMsg)
            {
                logPrintFormat(ras->logChan, RV_DEBUG, "Address:");
                pvtPrintStd(ras->hVal, addrNodeId, (int)ras->logChan);
                logPrintFormat(ras->logChan, RV_DEBUG, "Message:");
                pvtPrintStd(ras->hVal, nodeId, (int)ras->logChan);
                logPrintFormat(ras->logChan, RV_DEBUG, "Binary:");
                printHexBuff(buffer, encodedSize, ras->logChan);
            }

            /* Send the message. We always send through the unicast port */
			socketDesc = getRasSd(rasTx,stage);

            status = ras->evSendMessage(ras->app, (socketDesc <<1) | rasChanUnicast, destAddr, buffer, encodedSize);
            if (status < 0)
            {
                logPrint(ras->log, RV_ERROR,
                         (ras->log, RV_ERROR,
                         "rasEncodeAndSend: Couldn't send the message 0x%x: %d", rasTx, status));
            }
        }
        else
        {
            pvtGet(ras->hVal, pvtChild(ras->hVal, nodeId), &fieldId, NULL, NULL, NULL);
            logPrint(ras->logChan, RV_INFO,
                     (ras->logChan, RV_INFO, "New message (channel %d) Not sent (error) --> %s:",
                     isMulticast, nprn(pstGetFieldNamePtr(ras->synMessage, fieldId))));

            if (printMsg)
            {
                logPrintFormat(ras->logChan, RV_DEBUG, "Address:");
                pvtPrintStd(ras->hVal, addrNodeId, (int)ras->logChan);
                logPrintFormat(ras->logChan, RV_DEBUG, "Message:");
                pvtPrintStd(ras->hVal, nodeId, (int)ras->logChan);
            }

            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR,
                     "rasEncodeAndSend: Failed to encode the message 0x%x: %d", rasTx, status));
        }
    }
    else
    {
        logPrint(ras->log, RV_DEBUG,
                 (ras->log, RV_DEBUG,
                 "rasEncodeAndSend: Application doesn't want this message processed (root=%d)", nodeId));
    }

    /* Make sure we clear any nodes needed for address printing */
    if (addrNodeId >= 0)
        pvtDelete(ras->hVal, addrNodeId);

    if (status > 0)
        status = 0; /* Make sure return value is not positive - gatekeeper depends on it */
    return status;
}


/************************************************************************
 * rasDecodeAndRecv
 * purpose: Decode and receive a message from the net
 *          This function is called after we already know if its an incoming
 *          or outgoing message.
 *          It returns as parameters the sequence number and message type,
 *          allowing the caller to know if these parameters were changed in
 *          the hook function to the application
 * input  : ras             - RAS instance handle
 *          messageBuf      - Message buffer
 *          messageLength   - Length of received message
 *          isMulticast     - Are we sending it to a multicast address
 *          srcAddr         - Source address
 * output : srcAddr         - Reply address if found inside the message
 *          nodeId          - Root where we placed the message
 *          messageType     - Message type after hook
 *          requestSeqNum   - Sequence number in decoded message after hook
 * return : TRUE if message should be processed
 *          FALSE if message souldn't be processed
 *          Negative value on failure
 ************************************************************************/
int rasDecodeAndRecv(
    IN     rasModule*          ras,
    IN     BYTE*               messageBuf,
    IN     UINT32              messageLength,
    IN     BOOL                isMulticast,
    INOUT  cmTransportAddress* srcAddr,
    OUT    int*                nodeId,
    OUT    rasMessages*        messageType,
    OUT    UINT32*             requestSeqNum,
    OUT    void**              hMsgContext)
{
    cmHookRecvFromT recvHook;
    HPROTCONN       protConn;
    int             status = 0;
    int             addrNodeId, replyNodeId, msgNodeId;
    int             bytesDecoded = 0;
    BOOL            process, printMsg;
    INTPTR          fieldId;

    *nodeId = pvtAddRoot(ras->hVal, ras->synMessage, 0, NULL);
    if ((*nodeId) < 0)
        return *nodeId;

    if (isMulticast)
        protConn = (HPROTCONN)&ras->multicastAppHandle;
    else
        protConn = (HPROTCONN)&ras->unicastAppHandle;

    /* Decode the message */
    if (ras->cmiEvRASNewRawMessage)
    {
        /* We've got a decoding callback - probably from security app. Call it */
        logPrint(ras->log, RV_INFO,
                (ras->log, RV_INFO,
                "cmEvRASNewRawMessage(protConn = %x, pvt=%d)",
                protConn, *nodeId));

        status = ras->cmiEvRASNewRawMessage(
                                    (HAPP)ras->app,
                                    protConn,
                                    *nodeId, messageBuf, (int)messageLength, &bytesDecoded,
                                    hMsgContext);
    }
    else
    {
        /* Let's decode it on our own */
        status = cmEmDecode(ras->hVal, *nodeId, messageBuf, (int)messageLength, &bytesDecoded);
    }

    if ((status >= 0) && ((int)messageLength != bytesDecoded))
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR, "rasDecodeAndRecv: Bad message length"));
        pvtDelete(ras->hVal, *nodeId);
        return RVERROR;
    }
    if (status < 0)
    {
        pvtDelete(ras->hVal, *nodeId);
        return status;
    }

    recvHook = cmiGetRasHooks(ras->app)->hookRecvFrom;
    printMsg = logIsSelected(ras->logChan, RV_DEBUG);

    if (printMsg || (recvHook != NULL))
    {
        /* Create a transport address node id */
        addrNodeId = pvtAddRoot(ras->hVal, ((cmElem*)(ras->app))->hAddrSyn, 0, NULL);
        if (addrNodeId < 0)
        {
            pvtDelete(ras->hVal, *nodeId);
            return addrNodeId;
        }
        cmTAToVt(ras->hVal, addrNodeId, srcAddr);
    }
    else
        addrNodeId = RVERROR;

    /* Call the hook after decoding this message */
    if (recvHook != NULL)
    {
        process = !recvHook(protConn, *nodeId, addrNodeId, isMulticast, FALSE);
    }
    else
        process = TRUE;

    /* Make sure we're still sending out this message */
    if (process)
    {
        int tmpNodeId;

        /* Print the message */
        pvtGet(ras->hVal, pvtChild(ras->hVal, *nodeId), &fieldId, NULL, NULL, NULL);
        logPrint(ras->logChan, RV_INFO,
                 (ras->logChan, RV_INFO, "New message (channel %d)  recv <-- %s:",
                 isMulticast, nprn(pstGetFieldNamePtr(ras->synMessage, fieldId))));

        if (printMsg)
        {
            logPrintFormat(ras->logChan, RV_DEBUG, "Address:");
            pvtPrintStd(ras->hVal, addrNodeId, (int)ras->logChan);
            logPrintFormat(ras->logChan, RV_DEBUG, "Binary:");
            printHexBuff(messageBuf, bytesDecoded, ras->logChan);
            logPrintFormat(ras->logChan, RV_DEBUG, "Message:");
            pvtPrintStd(ras->hVal, *nodeId, (int)ras->logChan);
        }

        /* Get the index and sequence number again */
        msgNodeId = pvtGetByIndex(ras->hVal, *nodeId, 1, NULL);
        if (msgNodeId >= 0)
            *messageType = (rasMessages)(pvtGetSyntaxIndex(ras->hVal, msgNodeId) - 1);
        tmpNodeId = pvtGetChild(ras->hVal, msgNodeId, __q931(requestSeqNum), NULL);
        pvtGet(ras->hVal, tmpNodeId, NULL, NULL, (INT32*)requestSeqNum, NULL);

        /* See if we've got any specific reply address in the message */
        switch (*messageType)
        {
            case rasMsgRegistrationRequest:
                __pvtGetNodeIdByFieldIds(replyNodeId, ras->hVal, msgNodeId, {_q931(rasAddress) _nul(1) LAST_TOKEN});
                break;

            case rasMsgGatekeeperRequest:
            case rasMsgInfoRequestResponse:
                replyNodeId = pvtGetChild(ras->hVal, msgNodeId, __q931(rasAddress), NULL);
                break;

            case rasMsgLocationRequest:
            case rasMsgInfoRequest:
                replyNodeId = pvtGetChild(ras->hVal, msgNodeId, __q931(replyAddress), NULL);
                break;

            default:
                replyNodeId = -1;
        }
        if (replyNodeId >= 0)
            cmVtToTA(ras->hVal, replyNodeId, srcAddr);
    }
    else
    {
        logPrint(ras->log, RV_DEBUG,
                 (ras->log, RV_DEBUG,
                 "rasDecodeAndRecv: Application doesn't want this message processed (root=%d)", *nodeId));
        pvtDelete(ras->hVal, *nodeId);
        *nodeId = -1;
    }

    if (addrNodeId >= 0)
        pvtDelete(ras->hVal, addrNodeId);

    return (process?0:-1);   /* Look! I made a smiley-thingy! */
}


/************************************************************************
 * rasRouteMessage
 * purpose: Route the message to the right transaction, making sure if
 *          it's incoming, outgoing or IRR.
 * input  : ras             - RAS instance handle
 *          srcAddr         - Source address
 *          chanType        - Type of channel to send through
 *          messageBuf      - Message buffer
 *          messageLength   - Length of received message
 *          messageNodeId   - Node ID of the message.
 *                            If this value is negative, the message is
 *                            decoded and checked, otherwise, the decoded
 *                            message will be processed without calling the
 *                            hook functions.
 *          messageType     - Message type after hook
 *          requestSeqNum   - Sequence number in decoded message after hook
 * output : hMsgContext     - Incoming message context. Used mostly by security
 *                            If the returned value is different than NULL,
 *                            then the message context is not used by the
 *                            transaction and should be released
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasRouteMessage(
    IN  rasModule*          ras,
    IN  rasChanType         chanType,
    IN  cmTransportAddress* srcAddr,
    IN  BYTE*               messageBuf,
    IN  UINT32              messageLength,
    IN  int                 messageNodeId,
    IN  rasMessages         messageType,
    IN  UINT32              requestSeqNum,
    OUT void**              hMsgContext)
{
    /* IRR messages are handled differently */
    if (messageType == rasMsgInfoRequestResponse)
        return rasHandleIrr(ras, srcAddr, messageBuf, messageLength, messageNodeId, requestSeqNum, hMsgContext);

    /* Check if it's an incoming or outgoing message */
    switch (rasMessageInfo[messageType].trType)
    {
        case RAS_OUT_TX:
            /* Outgoing transaction reply */
            return rasHandleReply(ras, srcAddr, messageBuf, messageLength, messageNodeId, messageType, requestSeqNum, hMsgContext);
        case RAS_IN_TX:
            /* Incoming transaction request */
            return rasHandleRequest(ras, chanType, srcAddr, messageBuf, messageLength, messageNodeId, messageType, requestSeqNum, NULL, hMsgContext);
        default:
            logPrint(ras->log, RV_EXCEP,
                     (ras->log, RV_EXCEP, "rasRouteMessage: Transaction type unknown for %d", messageType));
            return RVERROR;
    }
}


/************************************************************************
 * rasCreateCatKey
 * purpose: Create the key struct for CAT from an incoming message
 *          transaction.
 * input  : ras     - RAS instance handle
 *          tx      - Incoming transaction to use
 * output : catKey  - Filled CAT key struct for this transaction
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasCreateCatKey(
    IN  rasModule*  ras,
    IN  rasInTx*    tx,
    OUT catStruct*  catKey)
{
    /* Make sure we start empty handed with this CAT key */
    memset(catKey, 0, sizeof(catStruct));

    /* Clear the flags */
    catKey->flags = catRasSrcAddr;

    /* RAS source address */
    memcpy(&catKey->rasSrcAddr, &tx->destAddress, sizeof(cmTransportAddress));

    /* RAS-CRV */
    switch (tx->transactionType)
    {
        INT32 crv;
        case cmRASAdmission:
        case cmRASDisengage:
        case cmRASBandwidth:
        case cmRASInfo:
            if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCRV,
                            0, &crv, NULL) >= 0)
            {
                catKey->rasCRV = (UINT32)(0x8000^crv);
                catKey->flags |= catRasCRV;
                break;
            }

        case cmRASServiceControl:
        default:
            break;
    }

    /* CallID */
    if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCallID,
                    0, NULL, (char*)catKey->callID) >= 0)
        catKey->flags |= catCallId;

    if (tx->transactionType == cmRASAdmission)
    {
        /* SourceCallSignalAddress */
        if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamSrcCallSignalAddress,
                        0, NULL, (char*)&catKey->srcCallSignalAddr) >= 0)
            catKey->flags |= catSrcCallSignalAddr;

        /* DestCallSignalAddress */
        if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamDestCallSignalAddress,
                        0, NULL, (char*)&catKey->destCallSignalAddr) >= 0)
            catKey->flags |= catDestCallSignalAddr;
    }

    switch (tx->transactionType)
    {
        case cmRASAdmission:
        case cmRASDisengage:
        case cmRASBandwidth:
        case cmRASServiceControl:
        {
            INT32 answerCall;

            /* CID */
            if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCID,
                            0, NULL, (char*)catKey->cid) >= 0)
                catKey->flags |= catCID;

            /* answerCall */
            if (rasGetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamAnswerCall,
                            0, &answerCall, NULL) >= 0)
            {
                catKey->answerCall = !answerCall;
                catKey->flags |= catAnswerCall;
            }
        }

        case cmRASInfo:
        default:
            break;
    }

    /* Just an address will not be considered as CAT keys by us */
    if (catKey->flags == catRasSrcAddr)
        return RVERROR;

    return 0;
}


/************************************************************************
 * rasUpdateRegInfo
 * purpose: Update the registration information of our RAS configuration
 *          from an incoming RCF message
 * input  : ras             - RAS instance handle
 *          messageNodeId   - Incoming message that caused this call
 *                            For unregistration, this value will be negative
 * output : none
 * return : none
 ************************************************************************/
void rasUpdateRegInfo(
    IN  rasModule*  ras,
    IN  int         messageNodeId)
{
    int srcNode;

    /* GK rasAddress - remove if we have to */
    if (messageNodeId < 0)
    {
        if (ras->gatekeeperRASAddress >= 0)
            pvtDelete(ras->hVal, ras->gatekeeperRASAddress);
        ras->gatekeeperRASAddress = RVERROR;
        if (ras->gatekeeperCallSignalAddress >= 0)
            pvtDelete(ras->hVal, ras->gatekeeperCallSignalAddress);
        ras->gatekeeperCallSignalAddress = RVERROR;
    }

    /* GK callSignalAddress*/
    if (messageNodeId >= 0)
    {
        if (ras->gatekeeperCallSignalAddress < 0)
            ras->gatekeeperCallSignalAddress = pvtAddRoot(ras->hVal, NULL, 0, NULL);
        __pvtGetNodeIdByFieldIds(srcNode,ras->hVal,messageNodeId, {_anyField _q931(callSignalAddress) _anyField LAST_TOKEN});
        pvtSetTree(ras->hVal, ras->gatekeeperCallSignalAddress, ras->hVal, srcNode);
    }

    /* terminalAlias */
    if (messageNodeId < 0)
    {
        /* Delete the terminal's aliases for now */
        if (ras->termAliasesNode >= 0)
            pvtDelete(ras->hVal, ras->termAliasesNode);
        ras->termAliasesNode = RVERROR;
    }
    else
    {
        if (ras->termAliasesNode < 0)
        {
            ras->termAliasesNode = pvtAddRoot(ras->hVal, NULL, 0, NULL);
            __pvtGetNodeIdByFieldIds(srcNode, ras->hVal, ras->confNode, {_q931(registrationInfo) _q931(terminalAlias) LAST_TOKEN});
            if (srcNode >= 0)
                pvtSetTree(ras->hVal, ras->termAliasesNode, ras->hVal, srcNode);
        }
        __pvtGetNodeIdByFieldIds(srcNode, ras->hVal, messageNodeId, {_anyField _q931(terminalAlias) LAST_TOKEN});
        if (srcNode >= 0)
            pvtSetTree(ras->hVal, ras->termAliasesNode, ras->hVal, srcNode);
    }

    /* endpointIdentifier */
    __pvtGetByFieldIds(srcNode, ras->hVal, messageNodeId, {_anyField _q931(endpointIdentifier) LAST_TOKEN}, NULL, (INT32 *)&ras->epIdLen, NULL);
    if (srcNode >= 0)
        pvtGetString(ras->hVal, srcNode, ras->epIdLen, ras->epId);
    else
        ras->epIdLen = 0;

    /* gatekeeperIdentifier */
    if(messageNodeId >= 0)
    {
        __pvtGetByFieldIds(srcNode, ras->hVal, messageNodeId, {_anyField _q931(gatekeeperIdentifier) LAST_TOKEN}, NULL, (INT32 *)&ras->gkIdLen, NULL);
        if (srcNode >= 0)
            pvtGetString(ras->hVal, srcNode, ras->gkIdLen, ras->gkId);
        else
            ras->gkIdLen = 0;
    }
    else
    {
        __pvtGetByFieldIds(srcNode, ras->hVal, ras->confNode, {_q931(registrationInfo) _q931(gatekeeperIdentifier) LAST_TOKEN}, NULL, (INT32 *)&ras->gkIdLen, NULL);
        if (srcNode >= 0)
            pvtGetString(ras->hVal, srcNode, ras->gkIdLen, ras->gkId);
        else
            ras->gkIdLen = 0;
    }

    /* Make sure the default messages of the RAS are also updated */
    cmiRASUpdateRegInfo((HRASMGR)ras, FALSE);
}


int getRasSd(HRAS rasTx,cmRASTrStage stage)
{
	int sd;
	if(!rasTx)
		return 0;

	if(stage == cmRASTrStageRequest)
	{
		rasOutTx*     tx = (rasOutTx *)rasTx;
		sd = tx->sd;
	} 
	else 
	{
			rasInTx*     tx = (rasInTx *)rasTx;
			sd = tx->sd;
	}
	return sd;
}

#ifdef __cplusplus
}
#endif


