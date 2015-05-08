
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

#include <cmutils.h>
#include <rasutils.h>
#include <rasparams.h>
#include <rasout.h>
#include <cmdebprn.h>

/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * rasTimeoutEvent
 * purpose: Callback function that is called when the timeout expires
 *          on an outgoing transaction message, making sure that a
 *          retransmission on this transaction will occur if necessary.
 * input  : context - Context to use
 *                    This is the outgoing transaction object
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV rasTimeoutEvent(IN void* context)
{
    rasModule*          ras;
    rasOutTx*           tx = (rasOutTx *)context;
    void*               message;
    int                 retryCount;
    HTI                 timer = (HTI)RVERROR;

    /* Lock the transaction so no one will close it */
    if (emaLock((EMAElement)tx))
    {
        /* This element exists... */
        ras = (rasModule *)emaGetUserData((EMAElement)tx);

        cmCallPreCallBack((HAPP)ras->app);

        logPrint(ras->log, RV_DEBUG,
                 (ras->log, RV_DEBUG,
                 "rasTimeoutEvent: On outTx=0x%x, retries=%d", tx, tx->retryCount));

        /* Decrease the retry count */
        tx->retryCount--;
        if (tx->retryCount == 0)
        {
            tx->state = rasTxStateTimedOut;
            timer = tx->timer;
            tx->timer = (HTI)RVERROR;
        }

        /* Make sure we've got all the necessary parameters */
        retryCount = tx->retryCount;
        message = tx->encodedMsg;

        if (retryCount == 0)
        {
            /* Transaction has timed out */
            cmiEvRASResponseT   evResponse;

            /* Stop the timer - we don't need it anymore */
            mtimerReset(ras->timers, timer);

            evResponse = tx->evResponse;

            /* Unlock it before we're done */
            emaMark((EMAElement)tx);
            emaUnlock((EMAElement)tx);

            /* See which of the response callbacks we have to call */
            if (evResponse != NULL)
            {
                if (ras->evAutoRas.cmEvAutoRASTimeout != NULL)
                {
                    cmiCBEnter(ras->app, "cmEvAutoRASTimeout(hsRas=0x%x)", tx);
                    ras->evAutoRas.cmEvAutoRASTimeout((HRAS)tx);
                    cmiCBExit(ras->app, "cmEvAutoRASTimeout(hsRas=0x%x)", tx);
                }

                if (!emaWasDeleted((EMAElement)tx))
                    evResponse((HAPPRAS)emaGetApplicationHandle((EMAElement)tx), (HRAS)tx, cmRASTrStageTimeout);
            }
            else if (ras->evApp.cmEvRASTimeout != NULL)
            {
                HAPPRAS haRas = (HAPPRAS)emaGetApplicationHandle((EMAElement)tx);
                cmiCBEnter(ras->app, "cmEvRASTimeout(haRas=0x%x,hsRas=0x%x)", haRas, tx);
                ras->evApp.cmEvRASTimeout(haRas, (HRAS)tx);
                cmiCBExit(ras->app, "cmEvRASTimeout(haRas=0x%x,hsRas=0x%x)", haRas, tx);
            }

            emaRelease((EMAElement)tx);
        }
        else if (message != NULL)
        {
            /* Retransmit */

            if (tx->state == rasTxStateRipReceived)
            {
                /* Reset the timer for regular intervals after RIP */
                mtimerReset(ras->timers, tx->timer);
                tx->timer = mtimerSet(ras->timers, rasTimeoutEvent, tx, rasCfgGetTimeout(ras));
            }

            /* Send the message */
            rasRetransmit(ras, (HRAS)tx, message, &tx->destAddress, "request");

            /* Unlock it before we're done */
            emaUnlock((EMAElement)tx);
        }
        else
        {
            /* No need to leave it locked */
            emaUnlock((EMAElement)tx);
        }
    }
}


/************************************************************************
 * rasHandleIncomingRIP
 * purpose: Handle an incoming RIP on an outgoing request
 * input  : ras             - RAS module to use
 *          tx              - Outgoing transaction we're dealing with
 *          messageNodeId   - Node ID of the RIP message
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasHandleIncomingRIP(
    IN rasModule*   ras,
    IN rasOutTx*    tx,
    IN int          messageNodeId)
{
    int nodeId;
    INT32 delay; /* in ms */

    /* Make sure we're not in a multicast transaction */
    if (!tx->isMulticast)
    {
        /* Find the timeout */
        __pvtGetByFieldIds(nodeId, ras->hVal, messageNodeId, {_anyField _q931(delay) LAST_TOKEN}, NULL, &delay, NULL);

        /* Set the timeout to its new value */
        if (tx->timer != (HTI)RVERROR)
            mtimerReset(ras->timers, tx->timer);
        tx->timer = mtimerSet(ras->timers, rasTimeoutEvent, tx, delay);
        tx->state = rasTxStateRipReceived;
    }
    else
    {
        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN,
                 "rasHandleIncomingRIP: outTx=0x%x is multicast. Ignoring RIP message"));
    }

    /* Delete the message and return */
    pvtDelete(ras->hVal, messageNodeId);
    return 0;
}






/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * rasOutgoingHashFunc
 * purpose: Outgoing transactions hash function
 *          This function returns as the hash key the actual sequence
 *          number of the transaction without fooling around with it
 * input  : param       - Parameter we're going to hash
 *                        This time it's a UINT32 of the sequence number
 *          paramSize   - Size of the parameter (4 here)
 *          hashSize    - Size of the hash table itself
 * output : none
 * return : Hash value to use
 ************************************************************************/
UINT32 rasOutgoingHashFunc(
    IN void *param,
    IN int paramSize,
    IN int hashSize)
{
    if (paramSize);
    return ((*(UINT32*)param) % hashSize);
}





/************************************************************************
 * rasCreateOutTx
 * purpose: Create an outgoing transaction and return its pointer
 * input  : ras         - RAS module to use
 *          haRas       - Application's handle for the RAS transaction
 *          transaction - The transaction type we want to start
 *          destAddress - Address of the destination.
 *                        If set to NULL, then it's for the gatekeeper
 * output : none
 * return : Pointer to an outgoing RAS transaction on success
 *          NULL on failure
 ************************************************************************/
rasOutTx* rasCreateOutTx(
    IN rasModule*       ras,
    IN HAPPRAS          haRas,
    IN cmRASTransaction transaction,
    IN cmRASTransport*  destAddress)
{
    rasOutTx*   tx;
    int         srcNodeId, requestNode;
    int         status; /* = 0 if no errors occured */

    /* Make sure we're working with outgoing transactions */
    if (ras->outRa == NULL)
        return NULL;

    /* Create the RAS transaction */
    tx = (rasOutTx *)emaAdd(ras->outRa, (void*)haRas);
    if (tx == NULL)
        return NULL;
    memset(tx, 0, sizeof(rasOutTx));

    /* Grab a node as root for the property db */
    tx->txProperty = pvtAddRoot(ras->hVal, ras->synProperty, 0, NULL);
    if (tx->txProperty < 0)
    {
        emaDelete(tx);
        return NULL;
    }

    /* Set some of the transaction's values */
    tx->isMulticast     = FALSE;
    tx->transactionType = transaction;
    tx->state           = rasTxStateIdle;
    tx->timer           = (HTI)RVERROR;

    /* Set the destination address for the transaction */
    status = pvtAdd(ras->hVal, tx->txProperty, __q931(address), 0, NULL, NULL);
    if ((destAddress == NULL) && (status >= 0))
    {
        /* Take it from configuration - the gatekeeper's address */

        srcNodeId = ras->gatekeeperRASAddress;
        if (ras->gatekeeperRASAddress < 0)
            __pvtGetNodeIdByFieldIds(srcNodeId, ras->hVal, ras->confNode, {_q931(manualDiscovery) _q931(defaultGatekeeper) LAST_TOKEN});

        /* Save the GK RAS address*/
        if ((ras->gatekeeperRASAddress < 0) && (srcNodeId >= 0))
        {
            ras->gatekeeperRASAddress = pvtAddRoot(ras->hVal, NULL, 0, NULL);
            pvtSetTree(ras->hVal,ras->gatekeeperRASAddress,ras->hVal,srcNodeId);
        }
        status=0;
        if (srcNodeId>=0)
            status = cmVtToTA(ras->hVal, srcNodeId, &tx->destAddress);
        else
            memset(&(tx->destAddress),0,sizeof(cmTransportAddress));
    }
    else if (destAddress != NULL)
    {
        /* Get the address the user gave us */
        tx->destAddress = *destAddress;
    }

    /* Build the default request message */
    if (status >= 0)
    {
        requestNode = pvtAdd(ras->hVal, tx->txProperty, __q931(request), 0, NULL, NULL);
        status = pvtSetTree(ras->hVal, requestNode, ras->hVal, ras->defaultMessages[transaction][cmRASTrStageRequest]);
    }

    /* Calculate the number of retries for this transaction */
    tx->retryCount = rasCfgGetRetries(ras);

    if (status < 0)
    {
        emaDelete(tx);
        return NULL;
    }
    return tx;
}


/************************************************************************
 * rasSendRequestMessage
 * purpose: Send an outgoing request message
 * input  : ras             - RAS module to use
 *          tx              - Outgoing transaction to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasSendRequestMessage(
    IN rasModule*       ras,
    IN rasOutTx*        tx)
{
    UINT32      reqSeqNum;
    int         nodeId, reqNodeId;
    int         status = 0;

    /* Set the sequence number of the transaction */
    meiEnter(ras->lockOutHash);
    if (ras->requestSeqNum == 65535)
        ras->requestSeqNum = 2;
    else
        ras->requestSeqNum++;
    reqSeqNum = ras->requestSeqNum;

    /* Update the outgoing hash table with this transaction */
    if (tx->hashValue != NULL)
        hashDelete(ras->outHash, tx->hashValue);
    tx->hashValue = hashAdd(ras->outHash, &reqSeqNum, &tx, TRUE);
    if (tx->hashValue == NULL)
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR, "rasSendRequestMessage: Cannot insert to hash 0x%x (requestSeqNum=%d)",
                 tx, reqSeqNum));
        status = RVERROR;
    }

    /* Lock the transaction and then unlock the hash */
    if (status >= 0)
        emaLock((EMAElement)tx);
    meiExit(ras->lockOutHash);
    if (status < 0)
        return status;

    if (tx->timer != (HTI)RVERROR)
    {
        /* Looks like somebody already sent this message - we should clear this timer... */
        mtimerReset(ras->timers, tx->timer);
        tx->timer = (HTI)RVERROR;
    }

    /* Get the request message */
    reqNodeId = pvtGetChild(ras->hVal, tx->txProperty, __q931(request), NULL);
    if (reqNodeId < 0) status = reqNodeId;

    /* Set the sequence number inside the outgoing request message */
    __pvtBuildByFieldIds(nodeId, ras->hVal, reqNodeId, {_anyField _q931(requestSeqNum) LAST_TOKEN}, (INT32)reqSeqNum, NULL);

    /* Mark transaction before using any of the callbacks */
    emaMark((EMAElement)tx);

    if (status >= 0)
        status = rasEncodeAndSend(ras, (HRAS)tx, cmRASTrStageRequest, reqNodeId, tx->isMulticast, &tx->destAddress, TRUE, &tx->encodedMsg);

    if ((emaRelease((EMAElement)tx) == TRUE) && (status >= 0))
    {
        /* No one closed this transaction - set the timer and finish with it */

        switch (tx->transactionType)
        {
            case cmRASNonStandard:
                /* Non-associated transaction */
                tx->state = rasTxStateIdle;
                tx->retryCount = 0;
                break;

            case cmRASUnsolicitedIRR:
            {
                /* Check if this IRR is waiting for a reply of some kind */
                INT32 needResponse;
                __pvtGetByFieldIds(nodeId, ras->hVal, reqNodeId, {_anyField _q931(needResponse) LAST_TOKEN}, NULL, &needResponse, NULL);
                if (!needResponse)
                {
                    /* We're not looking for a response - change state and end switch */
                    tx->state = rasTxStateIdle;
                    tx->retryCount = 0;
                    break;
                }

                /* If we got here, then we should wait for the
                   response as the rest of the messages: */
            }

            default:
            {
                /* We'll be waiting for a reply... */
                tx->state = rasTxStateRequestSent;

                /* Set the timer for retries */
                tx->timer = mtimerSet(ras->timers, rasTimeoutEvent, tx, rasCfgGetTimeout(ras));
            }
        }
    }

    emaUnlock((EMAElement)tx);

    return status;
}


/************************************************************************
 * rasDummyRequest
 * purpose: Handle incoming unsolicited IRRs as responses
 * input  : ras             - RAS module to use
 *          tx              - Outgoing transaction to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDummyRequest(
    IN rasModule*       ras,
    IN rasOutTx*        tx)
{
    /* We only act as if a message was sent so we'll be waiting for responses */

    /* Make sure it's an unsolicited IRR transaction */
    if ((tx->transactionType != cmRASUnsolicitedIRR) || (tx->hsCall == NULL))
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "rasDummyRequest: Not an unsolicited IRR transaction (tx=0x%x)", tx));
        return RVERROR;
    }

    if(emaLock((EMAElement)tx))
    {
        tx->state = rasTxStateRequestSent;
        emaUnlock((EMAElement)tx);
    }

    /* Make sure we've got this handle in the CAT */
    return catSetUnsolicitedIRR(ras->hCat, cmiGetCatForCall(tx->hsCall), (HRAS)tx);
}


/************************************************************************
 * rasHandleReply
 * purpose: Handle a reply of an outgoing request message
 * input  : ras             - RAS module to use
 *          srcAddress      - Address of the sender
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 *          messageNodeId   - Node ID of message root. If negative, then
 *                            message is decoded from given buffer and hook
 *                            is called
 *          messageType     - Message type of the reply
 *          seqNum          - requestSeqNum field value of the message
 * output : hMsgContext     - Incoming message context. Used mostly by security
 *                            If the returned value is different than NULL,
 *                            then the message context is not used by the
 *                            transaction and should be released
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasHandleReply(
    IN  rasModule*      ras,
    IN  cmRASTransport* srcAddress,
    IN  BYTE*           messageBuf,
    IN  UINT32          messageLength,
    IN  int             messageNodeId,
    IN  rasMessages     messageType,
    IN  UINT32          seqNum,
    OUT void**          hMsgContext)
{
    rasOutTx*       tx;
    void*           hashValue;
    int             status = 0;

    /* Lock hash to find the transaction */
    meiEnter(ras->lockOutHash);

    /* Look for this transaction */
    hashValue = hashGetElement(ras->outHash, hashFind(ras->outHash, &seqNum));
    if (hashValue != NULL)
        tx = *((rasOutTx **)hashValue);
    else
        tx = NULL;
    if (tx == NULL)
    {
        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN,
                 "rasHandleReply: Transaction not found for seqNum=%d", seqNum));
        if (messageNodeId >= 0)
            pvtDelete(ras->hVal, messageNodeId);
        meiExit(ras->lockOutHash);
        return RVERROR;
    }

    /* Lock the transaction and unlock the hash table */
    emaLock((EMAElement)tx);
    meiExit(ras->lockOutHash);

    /* Make sure we're still waiting for a reply of some kind */
    if (!((tx->state == rasTxStateRequestSent) || (tx->state == rasTxStateRipReceived) ||
          ((tx->isMulticast == TRUE) && (tx->state == rasTxStateReplyReceived))))
    {
        logPrint(ras->log, RV_WARN,
                 (ras->log, RV_WARN,
                 "rasHandleReply: Transaction 0x%x not waiting for replies. Reply is discarded", tx));
        if (messageNodeId >= 0)
            pvtDelete(ras->hVal, messageNodeId);
        status = RVERROR;
    }

    /* Make sure this message belongs to the transaction */
    if ((status >= 0) && (tx->transactionType != rasMessageInfo[messageType].transaction))
    {
        switch (messageType)
        {
            case rasMsgUnknownMessageResponse:
            case rasMsgRequestInProgress:
                /* We're fine - let these messages continue */
                break;
            default:
                logPrint(ras->log, RV_WARN,
                         (ras->log, RV_WARN,
                         "rasHandleReply: Reply (%d) doesn't match this transaction type (%d)", messageType, tx->transactionType));
                status = RVERROR;
        }
    }

    if ((messageNodeId < 0) && (status >= 0))
    {
        /* We're in business - decode the whole message if we have to */
        rasMessages newType;
        UINT32      newSeqNum;

        status =
            rasDecodeAndRecv(ras, messageBuf, messageLength, FALSE, srcAddress,
                             &messageNodeId, &newType, &newSeqNum, hMsgContext);

        /* Make sure the application didn't change the sequence number or type of message */
        if ((status >= 0) && ((newSeqNum != seqNum) || (messageType != newType)))
        {
            /* Reroute the message to check it again... */
            emaUnlock((EMAElement)tx);
            return rasRouteMessage(ras, rasChanUnicast, srcAddress, messageBuf, messageLength, messageNodeId, newType, newSeqNum, hMsgContext);
        }
    }

    if (status < 0)
    {
        /* Probably couldn't decode this message... */
        if (messageNodeId >= 0)
            pvtDelete(ras->hVal, messageNodeId);
        emaUnlock((EMAElement)tx);
        return status;
    }

    if (messageType == rasMsgRequestInProgress)
    {
        /* We've got a RIP - handle it differently */
        status = rasHandleIncomingRIP(ras, tx, messageNodeId);
        emaUnlock((EMAElement)tx);
        return status;
    }

    /* We found the transaction - start handling it properly */
    /* - No need to unlock the element - rasHandleTxResponse does that automatically */
    return rasHandleTxResponse(ras, tx, messageNodeId, messageType, hMsgContext);
}


/************************************************************************
 * rasHandleTxResponse
 * purpose: Handle a reply of an outgoing request message, when we already
 *          know the exact transaction. This function is used internally
 *          by rasHandleReply() and when we've got an unsolicited IRR on
 *          a dummy request.
 *          This function doesn't lock the transaction when handling it.
 *          It assumes that the transaction is already locked from somewhere
 *          else. It will unlock the transaction when finished.
 * input  : ras             - RAS module to use
 *          tx              - Transaction we're dealing with
 *          messageNodeId   - Node ID of message root. If negative, then
 *                            message is decoded from given buffer and hook
 *                            is called
 *          messageType     - Message type of the reply
 * output : hMsgContext     - Incoming message context. Used mostly by security
 *                            If the returned value is different than NULL,
 *                            then the message context is not used by the
 *                            transaction and should be released
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasHandleTxResponse(
    IN  rasModule*      ras,
    IN  rasOutTx*       tx,
    IN  int             messageNodeId,
    IN  rasMessages     messageType,
    OUT void**          hMsgContext)
{
    HAPPRAS         haRas;
    int             rootId;
    int             status;
    cmRASTrStage    stage;

    if (rasMessageInfo[messageType].msgType == rasTxMsgConfirm)
        stage = cmRASTrStageConfirm;
    else
        stage = cmRASTrStageReject;

    /* Make sure to notify the application of an incoming RAS message */
    if (ras->evMessages.cmEvRasMessageReceive != NULL)
    {
        /* An internal callback - no log for this one */
        ras->evMessages.cmEvRasMessageReceive(messageNodeId, stage, (HRAS)tx, ras->evMessages.hAppRasMsg);
    }

    /* Add the decoded message to the transaction's DB */
    rootId = pvtAdd(ras->hVal, tx->txProperty, __q931(response), 0, NULL, NULL);
    status = pvtMoveTree(ras->hVal, rootId, messageNodeId);
    messageNodeId = rootId;

    /* Make sure timer is released */
    if (tx->timer != (HTI)RVERROR)
    {
        mtimerReset(ras->timers, tx->timer);
        tx->timer = (HTI)RVERROR;
    }

    /* Update the transaction's state only if it's not multicast.
       For multicast transactions we want to receive any incoming message */
    if ((status >= 0) && (!tx->isMulticast))
        tx->state = rasTxStateReplyReceived;

    /* Mark the transaction before the callback and unlock it */
    emaMark((EMAElement)tx);

    emaUnlock((EMAElement)tx);

    if (status >= 0)
    {
        haRas = (HAPPRAS)emaGetApplicationHandle((EMAElement)tx);

        /* If we get GCF then save the GK RAS address*/
        if (messageType == rasMsgGatekeeperConfirm)
        {
            int srcNodeId;
            if (ras->gatekeeperRASAddress < 0)
                ras->gatekeeperRASAddress = pvtAddRoot(ras->hVal, NULL, 0, NULL);
            __pvtGetNodeIdByFieldIds(srcNodeId, ras->hVal, messageNodeId, {_anyField _q931(rasAddress) LAST_TOKEN});
            pvtSetTree(ras->hVal, ras->gatekeeperRASAddress, ras->hVal, srcNodeId);
        }

        /* Make sure we update the registration information if we have to */
        if (messageType == rasMsgRegistrationConfirm)
        {
            rasUpdateRegInfo(ras, messageNodeId);

            /* Let's get the GK address - just in case... */
            if (ras->gatekeeperRASAddress < 0)
                ras->gatekeeperRASAddress = pvtAddRoot(ras->hVal, NULL, 0, NULL);
            cmTAToVt(ras->hVal, ras->gatekeeperRASAddress, &tx->destAddress);
        }
        if (messageType == rasMsgUnregistrationConfirm)
        {
			rasUpdateRegInfo(ras, -1);
        }
        /* Check if we have an automatic RAS callback - if we do, this will
           override any manual RAS callback... */

        /* Automatic RAS callback */
        if (tx->evResponse != NULL)
        {
            /* We should overide the manual RAS callbacks - the automatic RAS should decide
               if it should be called */

            /* Associate message context with transaction */
            tx->hMsgContext = *hMsgContext;
            *hMsgContext = NULL;

            /* Callback... */
            if ((rasMessageInfo[messageType].msgType == rasTxMsgConfirm) && (ras->evAutoRas.cmEvAutoRASConfirm != NULL))
            {
                cmiCBEnter(ras->app, "cmEvAutoRASConfirm(hsRas=0x%x)", tx);
                ras->evAutoRas.cmEvAutoRASConfirm((HRAS)tx);
                cmiCBExit(ras->app, "cmEvAutoRASConfirm(hsRas=0x%x)", tx);
            }
            else if ((rasMessageInfo[messageType].msgType == rasTxMsgReject) && (ras->evAutoRas.cmEvAutoRASReject != NULL))
            {
                /* Find out the reject reason and then set the callback */
                INT32 reason;
                if (messageType == rasMsgUnknownMessageResponse)
                    reason = cmRASReasonUnknownMessageResponse; /* Reason is XRS */
                else
                {
                    /* Get the reason field from the reject message */
                    status = rasGetParam(ras, (HRAS)tx, cmRASTrStageReject, cmRASParamRejectReason, 0, &reason, NULL);
                }
                cmiCBEnter(ras->app, "cmEvAutoRasReject(hsRas=0x%x)", tx);
                ras->evAutoRas.cmEvAutoRASReject((HRAS)tx, (cmRASReason)reason);
                cmiCBExit(ras->app, "cmEvAutoRasReject(hsRas=0x%x)", tx);
            }

            if (!emaWasDeleted((EMAElement)tx))
                tx->evResponse(haRas, (HRAS)tx, stage);
        }
        else
        {
            /* Manual RAS callbacks */

            /* Start with the callbacks */
            if ((rasMessageInfo[messageType].msgType == rasTxMsgConfirm) && (ras->evApp.cmEvRASConfirm != NULL))
            {
                /* Associate message context with transaction */
                tx->hMsgContext = *hMsgContext;
                *hMsgContext = NULL;

                /* xCF message */
                cmiCBEnter(ras->app, "cmEvRASConfirm(haRas=0x%x,hsRas=0x%x)", haRas, tx);
                ras->evApp.cmEvRASConfirm(haRas, (HRAS)tx);
                cmiCBExit(ras->app, "cmEvRASConfirm(haRas=0x%x,hsRas=0x%x)", haRas, tx);
            }
            else if ((rasMessageInfo[messageType].msgType == rasTxMsgReject) && (ras->evApp.cmEvRASReject != NULL))
            {
                /* xRJ message */

                /* Find out the reject reason and then set the callback */
                INT32 reason;

                if (messageType == rasMsgUnknownMessageResponse)
                    reason = cmRASReasonUnknownMessageResponse; /* Reason is XRS */
                else
                {
                    /* Get the reason field from the reject message */
                    status = rasGetParam(ras, (HRAS)tx, cmRASTrStageReject, cmRASParamRejectReason, 0, &reason, NULL);
                }

                /* Associate message context with transaction */
                tx->hMsgContext = *hMsgContext;
                *hMsgContext = NULL;

                /* Make sure we notify the application with a callback... */
                cmiCBEnter(ras->app, "cmEvRasReject(haRas=0x%x,hsRas=0x%x)", haRas, tx);
                ras->evApp.cmEvRASReject(haRas, (HRAS)tx, (cmRASReason)reason);
                cmiCBExit(ras->app, "cmEvRasReject(haRas=0x%x,hsRas=0x%x)", haRas, tx);
            }
        }
    }

    /* Release the transaction and we're done */
    emaRelease((EMAElement)tx);

    return status;
}


/************************************************************************
 * rasCloseOutTx
 * purpose: Close an outgoing transaction
 * input  : ras             - RAS module to use
 *          tx              - Outgoing transaction to close
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasCloseOutTx(
    IN rasModule*       ras,
    IN rasOutTx*        tx)
{
    int     nodeId = -1;
    void*   hMsgContext = NULL;

    /* Lock the hash table first */
    meiEnter(ras->lockOutHash);

    /* Lock the transaction itself and mark it - we'll delete it before unlocking it... */
    if(emaLock((EMAElement)tx))
    {
        /* Delete the hash entry */
        if (tx->hashValue != NULL)
            hashDelete(ras->outHash, tx->hashValue);

        /* Unlock the hash table before killing the transaction */
        meiExit(ras->lockOutHash);

        /* Get all the handle we need to remove after unlocking */
        nodeId = tx->txProperty;
        hMsgContext = tx->hMsgContext;

        /* Kill the timer if there is one - before unlocking... */
        if (tx->timer != (HTI)RVERROR)
            mtimerReset(ras->timers, tx->timer);

        /* Remove RPOOL element */
        if (tx->encodedMsg != NULL)
        {
            meiEnter(ras->lockMessages);
            rpoolFree(ras->messages, tx->encodedMsg);
            meiExit(ras->lockMessages);
        }

        /* Delete, Unlock and Release */
        emaDelete((EMAElement)tx);
        emaUnlock((EMAElement)tx);
    }
    /* Free some nodes in the PVT db */
    pvtDelete(ras->hVal, nodeId);

    /* Dispose the message context for this transaction if we've got one */
    if (hMsgContext != NULL)
        ras->cmiEvRASReleaseMessageContext(hMsgContext);

    return 0;
}




#ifdef __cplusplus
}
#endif

