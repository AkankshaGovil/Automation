
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


#include <ti.h>
#include <cmintr.h>
#include <cmdebprn.h>
#include <rasutils.h>
#include <rasparams.h>
#include <rasirr.h>
#include <rasin.h>


/*todo: put this in some better place*/
int addrhash(
    IN cmTransportAddress*  addr,
    IN UINT32               hashKey);
int addrcmp(
    IN cmTransportAddress* addr1,
    IN cmTransportAddress* addr2);



/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * rasKillInTx
 * purpose: Kill an incoming transaction through the garbage collection
 *          mechanism.
 * input  : ras             - RAS module to use
 *          tx              - Incoming transaction to kill
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasKillInTx(
    IN rasModule*       ras,
    IN rasInTx*         tx)
{
    /* Lock the transaction itself - we'll delete it before unlocking it... */
    if(emaLock((EMAElement)tx))
    {
        /* We assume that the hash table is already locked from outside */

        /* Delete the hash entry - locked from outside */
        hashDelete(ras->inHash, tx->hashValue);

        /* Delete, Unlock and Release */
        emaDelete((EMAElement)tx);

        /* Free RPOOL block */
        if (tx->encodedMsg != NULL)
        {
            meiEnter(ras->lockMessages);
            rpoolFree(ras->messages, tx->encodedMsg);
            meiExit(ras->lockMessages);
        }

        emaUnlock((EMAElement)tx);
    }
    return 0;
}


/************************************************************************
 * rasGarbageCollection
 * purpose: Clean the incoming transaction list from old transactions that
 *          are no more relevant to us.
 * input  : ras             - RAS module to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasGarbageCollection(
    IN rasModule*   ras)
{
    int currTime;
    rasInTx* tx;

    meiEnter(ras->lockGarbage);

    /* We go through the list of transactions until we find a transaction whose
       stopTime is greater than the current time - this means that this transaction
       should not be closed and so are all of the other transactions after it */
    currTime = timerGetTimeInMilliseconds();
    tx = ras->firstTx;

    /* Continue until there are no transactions left - we'll stop in the middle
       if we'll have to */
    while (tx != NULL)
    {
        if (tx->stopTime > currTime)
        {
            /* We shouldn't continue anymore */
            break;
        }

        /* Deallocate this transaction */
        ras->firstTx = tx->next;
        rasKillInTx(ras, tx);

        tx = ras->firstTx;
    }

    if (ras->firstTx == NULL) ras->lastTx = NULL;

    meiExit(ras->lockGarbage);
    return 0;
}


/************************************************************************
 * rasNewTx
 * purpose: Create a new transaction for an incoming message
 * input  : ras             - RAS module to use
 *          rootId          - Message node for transaction
 * output : newRoot         - New message node for transaction
 * return : Transaction's handle on success
 *          NULL on failure
 ************************************************************************/
static rasInTx* rasNewTx(
    IN  rasModule*  ras,
    IN  int         rootId,
    OUT int*        newRoot)
{
    rasInTx*    tx = NULL;
    int         status = 0;
    int         propId, requestId;
    *newRoot = RVERROR;

    /* Make sure we're supporting incoming transactions */
    if (ras->inRa == NULL)
        return NULL;

    /* Create a root and decode this message */
    propId = pvtAddRoot(ras->hVal, ras->synProperty, 0, NULL);
    requestId = pvtAdd(ras->hVal, propId, __q931(request), 0, NULL, NULL);
    if (requestId >= 0)
        status = pvtMoveTree(ras->hVal, requestId, rootId);
    else
    {
        pvtDelete(ras->hVal, rootId);
        status = requestId;
    }

    /* New transaction - allocate a place for it */
    if (status >= 0)
    {
        *newRoot = requestId;

        tx = (rasInTx *)emaAdd(ras->inRa, NULL);
        if (tx == NULL)
        {
            /* Let's see if we have a closed transaction we can discard of */
            if (ras->firstTx != NULL)
            {
                /* Kill this one and take its place */
                meiEnter(ras->lockInHash);
                meiEnter(ras->lockGarbage);
                tx = ras->firstTx;
                ras->firstTx = tx->next;
                if (ras->firstTx == NULL) ras->lastTx = NULL;
                rasKillInTx(ras, tx);
                meiExit(ras->lockGarbage);
                meiExit(ras->lockInHash);

                /* Try to allocate a space again */
                tx = (rasInTx *)emaAdd(ras->inRa, NULL);
            }

            if (tx == NULL)
                status = RESOURCES_PROBLEM;
        }
    }
    if (status < 0)
    {
        pvtDelete(ras->hVal, propId);
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR, "rasNewTx: Not enough memory to allocate a new incoming transaction"));
        return NULL;
    }
    memset(tx, 0, sizeof(rasInTx));
    tx->txProperty = propId;

    return tx;
}


/************************************************************************
 * rasLookupCall
 * purpose: Find a call matching the incoming transaction
 * input  : ras         - RAS module to use
 *          tx          - Incoming transaction
 * output : hsCall      - Call handle for the call
 *                        NULL if message is not related to any call
 *          callRelated - Indication if this transaction is call related or not
 * return : Non-negative value on success (even if call wasn't found)
 *          Negative value on failure
 ************************************************************************/
static int rasLookupCall(
    IN  rasModule*  ras,
    IN  rasInTx*    tx,
    OUT HCALL*      hsCall,
    OUT BOOL*       callRelated)
{
    RVHCATCALL  hCatCall;
    catStruct   key;
    *hsCall = NULL;

    switch (tx->transactionType)
    {
        case cmRASGatekeeper:
        case cmRASRegistration:
        case cmRASUnregistration:
        case cmRASUnknown:
        case cmRASNonStandard:
        case cmRASResourceAvailability:
        case cmRASUnsolicitedIRR:
        case cmRASLocation:
            /* These are never related to calls */
            *callRelated = FALSE;
            return 0;

        case cmRASAdmission:
        case cmRASDisengage:
        case cmRASBandwidth:
        case cmRASInfo:
        case cmRASServiceControl:
        {
            /* Get the keys for CAT */
            if (rasCreateCatKey(ras, tx, &key) < 0)
            {
                *callRelated = FALSE; /* No actual key for CAT was found */
                return 0;
            }

            /* Make sure the transaction is call specific (not IRQ with CRV=0) */
            if ((tx->transactionType == cmRASInfo) && (key.rasCRV == 0x8000))
            {
                *callRelated = FALSE; /* IRQ not on a specific call */
                return 0;
            }

            break;
        }

        default:
            logPrint(ras->log, RV_EXCEP,
                     (ras->log, RV_EXCEP,
                     "rasLookupCall: Bad transaction type %d", tx->transactionType));
            return RVERROR;
    }

    /* Search for the call */
    *callRelated = TRUE;
    hCatCall = catFind(ras->hCat, &key);
    if (hCatCall != NULL)
    {
        /* Update the list of keys on ARQs */
        if (tx->transactionType == cmRASAdmission)
            catUpdate(ras->hCat, hCatCall, &key);

        *hsCall = catGetCallHandle(ras->hCat, hCatCall);

        /* call was found */
        return 0;
    }

    if (tx->transactionType == cmRASAdmission)
    {
        /* The New call arrived */
        if (ras->evRASNewCall(ras->app, &hCatCall, &key) >= 0)
        {
            *hsCall = catGetCallHandle(ras->hCat, hCatCall);
            return 0;
        }
    }

    logPrint(ras->log, RV_WARN,
             (ras->log, RV_WARN,
             "rasLookupCall: Can't find call for this transaction (%d)", tx->transactionType));

    /* Even without a call for this transaction we're going to give it to the user */
    return 0;
}


/************************************************************************
 * rasSendResponse
 * purpose: Sends a response message on an incoming request, whether its
 *          a confirm or a reject message
 * input  : ras             - RAS module to use
 *          tx              - Incoming transaction
 *          trStage         - Transaction stage. Indicates if it's a reject
 *                            or a confirm
 *          nodeId          - Message to send if not done through stage
 *          storeInRPOOL    - Indicate if we want to store this message
 *                            for future retransmissions
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSendResponse(
    IN rasModule*   ras,
    IN rasInTx*     tx,
    IN cmRASTrStage trStage,
    IN int          nodeId,
    IN BOOL         storeInRPOOL)
{
    rasInTxKey* key;
    int responseNodeId;
    int status = 0;

    if (nodeId < 0)
    {
        /* No specific message - send the response in the property database */
        if (tx->responseSet == rasNoResponseSet)
        {
            /* The application didn't set any response - let's take the default one */
            __pvtBuildByFieldIds(responseNodeId, ras->hVal, tx->txProperty, {_q931(response) LAST_TOKEN}, 0, NULL);
            pvtSetTree(ras->hVal, responseNodeId, ras->hVal, ras->defaultMessages[tx->transactionType][trStage]);
        }
        else
            responseNodeId = pvtGetChild(ras->hVal, tx->txProperty, __q931(response), NULL);
    }
    else
        responseNodeId = nodeId;

    if (responseNodeId < 0)
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR, "rasSendResponse: Problems in handling the response message (transaction 0x%x)", tx));
        return responseNodeId;
    }

    if (trStage == cmRASTrStageConfirm)
    {
        int         destNodeId = 1;
        cmRASParam  param = (cmRASParam)-1;

        /* See if we have to create Q931 addresses in confirm messages */
        switch (tx->transactionType)
        {
            case cmRASRegistration:
                param = cmRASParamCallSignalAddress;
                __pvtGetNodeIdByFieldIds(destNodeId, ras->hVal, tx->txProperty, {_q931(response) _anyField _q931(callSignalAddress) LAST_TOKEN});
                break;

            case cmRASAdmission:
                param = cmRASParamDestCallSignalAddress;
                __pvtGetNodeIdByFieldIds(destNodeId, ras->hVal, tx->txProperty, {_q931(response) _anyField _q931(destCallSignalAddress) LAST_TOKEN});
                break;

            default:
                destNodeId = 1;
        }

        if (destNodeId < 0)
        {
            cmTransportAddress addr;
            if (cmGetLocalCallSignalAddress(ras->app, &addr) >= 0)
                rasSetParam(ras, (HRAS)tx, cmRASTrStageConfirm, param, 0, sizeof(addr), (char*)&addr);
        }
    }

    /* Set the sequence number in the response to match the one in the request */
    key = (rasInTxKey *)hashGetKey(ras->inHash, tx->hashValue);
    __pvtBuildByFieldIds(status, ras->hVal, responseNodeId, {_anyField _q931(requestSeqNum) LAST_TOKEN}, (INT32)(key->seqNumber), NULL);
    if (status < 0) return status;

    /* Send the message. We always send through the unicast port */
    status = rasEncodeAndSend(ras, (HRAS)tx, trStage, responseNodeId, FALSE, &tx->destAddress, storeInRPOOL, &tx->encodedMsg);
    if (status < 0)
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "rasSendResponse: Couldn't send the message 0x%x: %d", tx, status));
        return status;
    }

    /* Change the state of this transaction */
    tx->state = rasTxStateReplySent;

    return 0;
}








/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * rasIncomingHashFunc
 * purpose: Incoming transactions hash function
 *          This function returns as the hash key the actual sequence
 *          number of the transaction without fooling around with it
 * input  : param       - Parameter we're going to hash
 *          paramSize   - Size of the parameter
 *          hashSize    - Size of the hash table itself
 * output : none
 * return : Hash value to use
 ************************************************************************/
UINT32 rasIncomingHashFunc(
    IN void *param,
    IN int paramSize,
    IN int hashSize)
{
    rasInTxKey* key = (rasInTxKey *)param;
    if (paramSize);
    return (addrhash(&key->address, key->seqNumber) % hashSize);
}


/************************************************************************
 * rasOutgoingHashCompare
 * purpose: Compare keys in the outgoing hash table
 * input  : key1, key2  - Keys to compare
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
BOOL rasOutgoingHashCompare(IN void *key1, IN void* key2, IN UINT32 keySize)
{
    rasInTxKey* txKey1 = (rasInTxKey *)key1;
    rasInTxKey* txKey2 = (rasInTxKey *)key2;

    if (keySize);

    return
        ((txKey1->seqNumber == txKey2->seqNumber) &&
         (txKey1->msgType == txKey2->msgType) &&
         (addrcmp(&txKey1->address, &txKey2->address) == 0));
}


/************************************************************************
 * rasHandleRequest
 * purpose: Handle an incoming request message
 * input  : ras             - RAS module to use
 *          chanType        - Channel the message came from (uni/multi)
 *          srcAddress      - Address of the sender
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 *          messageNodeId   - Node ID of message root. If negative, then
 *                            message is decoded from given buffer and hook
 *                            is called
 *          messageType     - Message type of the reply
 *          seqNum          - requestSeqNum field value of the message
 *          hsCall          - Call related to this transaction if it's an unsolicited IRR
 *                            NULL on any other case
 * output : hMsgContext     - Incoming message context. Used mostly by security
 *                            If the returned value is different than NULL,
 *                            then the message context is not used by the
 *                            transaction and should be released
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasHandleRequest(
    IN  rasModule*      ras,
    IN  rasChanType     chanType,
    IN  cmRASTransport* srcAddress,
    IN  BYTE*           messageBuf,
    IN  UINT32          messageLength,
    IN  int             messageNodeId,
    IN  rasMessages     messageType,
    IN  UINT32          seqNum,
    IN  HCALL           hsCall,
    OUT void**          hMsgContext)
{
    rasInTx*    tx;
    rasInTxKey  key;
    void*       hashValue;
    int         status;

    meiEnter(ras->lockInHash);

    /* Make sure we clean up the incoming database */
    status = rasGarbageCollection(ras);

    /* Find the transaction */
    memcpy(&key.address, srcAddress, sizeof(cmRASTransport));
    key.seqNumber = seqNum;
    key.msgType = messageType;
    hashValue = hashGetElement(ras->inHash, hashFind(ras->inHash, &key));
    if (hashValue != NULL)
        tx = *((rasInTx **)hashValue);
    else
        tx = NULL;

    if (tx == NULL)
    {
        /* New message to handle */
        HAPPRAS     haRas = NULL;
        BOOL        callRelated = FALSE;
        rasMessages newType;
        UINT32      newSeqNum;
        BOOL        notifyApp = TRUE;
        HAPPCALL    haCall = NULL;

        meiExit(ras->lockInHash);

        if (messageNodeId < 0)
        {
            /* Decode the message - it's first time we're here */
            status =
                rasDecodeAndRecv(ras, messageBuf, messageLength, chanType == rasChanMulticast, srcAddress,
                                 &messageNodeId, &newType, &newSeqNum, hMsgContext);

            /* Make sure the application didn't change the sequence number or type of message */
            if ((status >= 0) && ((newSeqNum != seqNum) || (messageType != newType)))
            {
                /* Reroute the message to check it again... */
                return rasRouteMessage(ras, chanType, srcAddress, messageBuf, messageLength, messageNodeId, newType, newSeqNum, hMsgContext);
            }
        }
        else
            status = 0;

        if (status >= 0)
        {
            /* Decode the message and put it in a new transaction */
            tx = rasNewTx(ras, messageNodeId, &messageNodeId);
        }

        if (tx == NULL)
            return RVERROR;

        /* Make sure to notify the application of an incoming RAS message */
        if (ras->evMessages.cmEvRasMessageReceive != NULL)
        {
            /* An internal callback - no log for this one */
            ras->evMessages.cmEvRasMessageReceive(messageNodeId, cmRASTrStageRequest, (HRAS)tx, ras->evMessages.hAppRasMsg);
        }

        /* Insert the key to the hash and unlock it */
        meiEnter(ras->lockInHash);
        tx->hashValue = hashAdd(ras->inHash, &key, &tx, FALSE);
        meiExit(ras->lockInHash);

        if (tx->hashValue == NULL)
        {
            if (tx->txProperty >= 0)
                pvtDelete(ras->hVal, tx->txProperty);
            emaDelete((EMAElement)tx);
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR, "rasHandleRequest: Error inserting hash value for a new incoming transaction"));
            return RESOURCES_PROBLEM;
        }

        /* Set known values for this transaction */
        tx->isMulticast = (chanType == rasChanMulticast);
        tx->sd = chanType >> 1; /* NexTone */
        memcpy(&tx->destAddress, srcAddress, sizeof(cmRASTransport));
        tx->responseSet = rasNoResponseSet;
        tx->state = rasTxStateIdle;

        if (hsCall == NULL)
        {
            /* This transaction is not an unsolicited IRR - check the table for this
               transaction's type */
            tx->transactionType = rasMessageInfo[messageType].transaction;

            /* Check if the TX belongs to a call */
            status = rasLookupCall(ras, tx, &tx->hsCall, &callRelated);
        }
        else
        {
            /* We've got an incoming unsolicited IRR and a request transaction */
            tx->transactionType = cmRASUnsolicitedIRR;
            tx->hsCall = hsCall;
            callRelated = TRUE;
        }

        /* Lock the call object if we have one */
        if ((status < 0) || (callRelated && (tx->hsCall != NULL) && !emaLock((EMAElement)tx->hsCall)))
        {
            /* Remove this transaction from the hash table */
            meiEnter(ras->lockInHash);
            hashDelete(ras->inHash, tx->hashValue);
            meiExit(ras->lockInHash);

            /* Kill the Tx and get the hell out of here */
            if (tx->txProperty >= 0)
                pvtDelete(ras->hVal, tx->txProperty);
            emaDelete((EMAElement)tx);
            return status;
        }

        /* Calculate the time this transaction will timeout */
        tx->stopTime = timerGetTimeInMilliseconds() + (rasCfgGetRetries(ras) * rasCfgGetTimeout(ras));

        /* Use callback to notify the application about the incoming request */
        emaMark((EMAElement)tx);

        /* Make sure we update the registration information if we have to */
        if (messageType == rasMsgUnregistrationRequest)
        {
            rasUpdateRegInfo(ras, -1);
        }

        if (callRelated && (tx->hsCall != NULL))
        {
            haCall = (HAPPCALL)emaGetApplicationHandle((EMAElement)tx->hsCall);
            emaUnlock((EMAElement)tx->hsCall);

            /* For IRQ - we'll set the perCallInfo inside IRR by the call information */
            if (tx->transactionType == cmRASInfo)
            {
                int irrNode = pvtAdd(ras->hVal, tx->txProperty, __q931(response), 0, NULL, NULL);
                rasSetIrrFields(ras, (HRAS)tx, irrNode, tx->hsCall);
            }

            if (ras->evCallRequest != NULL)
            {
                /* Associate message context with transaction */
                tx->hMsgContext = *hMsgContext;
                *hMsgContext = NULL;

                notifyApp = !ras->evCallRequest(ras->app,
                                                (HRAS)tx,
                                                tx->hsCall,
                                                tx->transactionType,
                                                srcAddress,
                                                haCall,
                                                ras->evAutoRas.cmEvAutoRASRequest);
            }
        }
        else
        {
            if (ras->evEpRequest != NULL)
            {
                /* Associate message context with transaction */
                tx->hMsgContext = *hMsgContext;
                *hMsgContext = NULL;
                notifyApp = !ras->evEpRequest(ras->app,
                                              (HRAS)tx,
                                              tx->transactionType,
                                              srcAddress,
                                              ras->evAutoRas.cmEvAutoRASRequest);
            }

        }

        if (notifyApp)
        {
            if (ras->evApp.cmEvRASRequest != NULL)
            {
                /* Associate message context with transaction */
                tx->hMsgContext = *hMsgContext;
                *hMsgContext = NULL;
                cmiCBEnter(ras->app, "cmEvRASRequest(hsRas=0x%x,hsCall=0x%x,trans=%d)", tx, tx->hsCall, tx->transactionType);
                ras->evApp.cmEvRASRequest((HRAS)tx,
                                          tx->hsCall,
                                          &haRas,
                                          tx->transactionType,
                                          srcAddress,
                                          haCall);
                cmiCBExit(ras->app, "cmEvRASRequest(hsRas=0x%x,haRas=0x%x)", tx, haRas);

                /* Update the application's handle */
                emaSetApplicationHandle((EMAElement)tx, (void*)haRas);
            }
            else
            {
                /* Automatic RAS mode, and there's no callback for manual RAS messages.
                   We should kill this transaction. */
                status = rasCloseInTx(ras, tx);
            }
        }

        /* Release the transaction when we're done */
        emaRelease((EMAElement)tx);
    }
    else if (emaLock((EMAElement)tx))
    {
        /* This is a retransmission - see if we have a reply for it */
        meiExit(ras->lockInHash);

        switch (tx->state)
        {
            case rasTxStateReplySent:
                /* Retransmit the reply to the sender */
                status = rasRetransmit(ras, (HRAS)tx, tx->encodedMsg, &tx->destAddress, "response");
                break;

            case rasTxStateRipSent:
            {
                int timeLeft = tx->ripStopTime - timerGetTimeInMilliseconds();
                if (timeLeft > 0)
                {
                    /* Let's send a RIP retransmission after calculating the delay */
                    status = rasSendRIP(ras, tx, timeLeft, FALSE);
                }
                else
                    status = RVERROR;
                break;
            }

            default:
                /* Do nothing - retransmission of something that the application didn't
                   Reply to yet */
                logPrint(ras->log, RV_DEBUG,
                         (ras->log, RV_DEBUG,
                         "rasHandleRequest: Retrasmission ignored for transaction 0x%d", tx));
        }

        emaUnlock((EMAElement)tx);

        pvtDelete(ras->hVal, messageNodeId);
    }
    else
    {
        meiExit(ras->lockInHash);
    }

    return status;
}


/************************************************************************
 * rasSendConfirmMessage
 * purpose: Sends a confirm response on an incoming RAS request
 * input  : ras             - RAS module to use
 *          tx              - Incoming transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSendConfirmMessage(
    IN rasModule*   ras,
    IN rasInTx*     tx)
{
    int status = -1;

    /* Lock the transaction before dealing with it */
    if(emaLock((EMAElement)tx))
    {
        status = rasSendResponse(ras, tx, cmRASTrStageConfirm, -1, TRUE);

        /* Unlock and release the transaction when we're done */
        emaUnlock((EMAElement)tx);
    }
    return status;
}


/************************************************************************
 * rasSendRejectMessage
 * purpose: Sends a reject response on an incoming RAS request
 * input  : ras             - RAS module to use
 *          tx              - Incoming transaction
 *          reason          - The reject reason to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSendRejectMessage(
    IN rasModule*   ras,
    IN rasInTx*     tx,
    IN cmRASReason  reason)
{
    int status = -1;

    /* Lock the transaction before dealing with it */
    if(emaLock((EMAElement)tx))
    {
        if (reason == cmRASReasonUnknownMessageResponse)
        {
            int nodeId;

            /* Create an XRS message */
            __pvtBuildByFieldIds(nodeId, ras->hVal, tx->txProperty,
                {_q931(response) _q931(unknownMessageResponse) LAST_TOKEN}, 0, NULL);
                tx->responseSet = rasRejectSet;
        }
        else
        {
            /* Set the reject reason of this message */
            rasSetParam(ras, (HRAS)tx, cmRASTrStageReject, cmRASParamRejectReason, 0, reason, NULL);
        }

        status = rasSendResponse(ras, tx, cmRASTrStageReject, -1, TRUE);

        /* Unlock and release the transaction when we're done */
        emaUnlock((EMAElement)tx);
    }
    return status;
}


/************************************************************************
 * rasSendRIP
 * purpose: Sends a RIP response on an incoming RAS request
 * input  : ras     - RAS module to use
 *          tx      - Incoming transaction
 *          delay   - Delay for RIP message in milliseconds
 *          updateStopTime  - Indicate if we're updating the stop time
 *                            and increasing the transaction's time or not
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int rasSendRIP(
    IN rasModule*   ras,
    IN rasInTx*     tx,
    IN int          delay,
    IN BOOL         updateStopTime)
{
    int status, nodeId, progressNodeId;

    /* Make sure delay is within range */
    if ((delay < 1) || (delay > 65535))
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "rasSendRIP: Delay %d out of range for tx=0x%x (1..65535)", delay, tx));
        return RVERROR;
    }

    /* Build the RIP message */
    nodeId = pvtAddRoot(ras->hVal, ras->synMessage, 0, NULL);
    if (nodeId < 0) return nodeId;
    __pvtBuildByFieldIds(status, ras->hVal, nodeId, {_q931(requestInProgress) _q931(delay) LAST_TOKEN}, delay, NULL);
    if (status < 0)
    {
        pvtDelete(ras->hVal, nodeId);
        return status;
    }

    /* Lock the transaction before dealing with it */
    if(emaLock((EMAElement)tx))
    {
        /* Link the RIP message to the transaction's database */
        progressNodeId = pvtAdd(ras->hVal, tx->txProperty, __q931(progress), 0, NULL, NULL);
        status = pvtMoveTree(ras->hVal, progressNodeId, nodeId);
        if (status < 0)
            pvtDelete(ras->hVal, nodeId);

        /* Send this message */
        if (status >= 0)
            status = rasSendResponse(ras, tx, cmRASTrStageConfirm, progressNodeId, FALSE);

        if (status >= 0)
        {
            tx->state = rasTxStateRipSent;

            if (updateStopTime)
            {
                /* Recalculate the time this transaction will timeout */
                tx->stopTime += delay;
                tx->ripStopTime = timerGetTimeInMilliseconds() + delay;
            }
        }

        /* Unlock and release the transaction when we're done */
        emaUnlock((EMAElement)tx);
    }
    return status;
}


/************************************************************************
 * rasCloseInTx
 * purpose: Close an incoming transaction
 *          This won't actually free resources, only mark the transaction
 *          as a possibility for removal in the garbage collection.
 * input  : ras             - RAS module to use
 *          tx              - Incoming transaction to close
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasCloseInTx(
    IN rasModule*       ras,
    IN rasInTx*         tx)
{
    int nodeId = -1;
    void* hMsgContext = NULL;

    meiEnter(ras->lockGarbage);
    if(emaLock((EMAElement)tx))
    {
        /* Set garbage collection list to match the required order */
        if (ras->lastTx != NULL) ras->lastTx->next = tx;
        ras->lastTx = tx;
        if (ras->firstTx == NULL) ras->firstTx = tx;

        nodeId = tx->txProperty;
        hMsgContext = tx->hMsgContext;

        emaUnlock((EMAElement)tx);
    }
    meiExit(ras->lockGarbage);

    /* Free some nodes in the PVT db */
    if (nodeId >= 0)
        pvtDelete(ras->hVal, nodeId);

    /* Dispose of the message context for this transaction if we've got any */
    if (hMsgContext != NULL)
        ras->cmiEvRASReleaseMessageContext(hMsgContext);

    return 0;
}



#ifdef __cplusplus
}
#endif


