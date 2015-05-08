
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
#include <rasparams.h>
#include <rasutils.h>
#include <rasin.h>
#include <rasout.h>
#include <rasirr.h>


/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * rasFindCallHandleForIrr
 * purpose: Find the call handle for an IRR message with a single call
 *          information inside it. This function is used to determine if
 *          the IRR message is solicited or unsolicited
 * input  : ras         - RAS module to use
 *          rootId      - Node ID of the message to check
 *          srcAddress  - Address of the sender
 * output : none
 * return : Call handle of a call matches the IRR
 *          NULL otherwise
 ************************************************************************/
HCALL rasFindCallHandleForIrr(
    IN  rasModule*      ras,
    IN  int             rootId,
    IN  cmRASTransport* srcAddress)
{
    catStruct   key;
    int         nodeId, tmpNodeId;
    RVHCATCALL  hCatCall;
    INT32       isOrigin;

    /* We surely know the source address of the message */
    key.flags = catRasSrcAddr;
    memcpy(&key.rasSrcAddr, srcAddress, sizeof(cmRASTransport));

    /* Find the call information... */
    __pvtGetByFieldIds(nodeId, ras->hVal, rootId, {_anyField _q931(perCallInfo) _nul(1) LAST_TOKEN}, NULL, NULL, NULL);

    /* RAS-CRV */
    tmpNodeId = pvtGetChild(ras->hVal, nodeId, __q931(callReferenceValue), NULL);
    if (tmpNodeId >= 0)
    {
        int crv = 0;
        if (pvtGet(ras->hVal, tmpNodeId, NULL, NULL, (INT32*)&crv, NULL) >= 0)
        {
            key.rasCRV = (UINT32)(0x8000^crv);
            key.flags |= catRasCRV;
        }
    }
    
    /* Originator (answerCall) */
    tmpNodeId = pvtGetChildByFieldId(ras->hVal, nodeId, __q931(originator), &isOrigin, NULL);
    if (tmpNodeId >= 0)
    {
        key.answerCall = !isOrigin;
        key.flags |= catAnswerCall;
    }

    /* CID */
    tmpNodeId = pvtGetChild(ras->hVal, nodeId, __q931(conferenceID), NULL);
    if (tmpNodeId >= 0)
        if (pvtGetString(ras->hVal, tmpNodeId, sizeof(key.cid), (char*)key.cid) >= 0)
            key.flags |= catCID;

    /* CallId */
    __pvtGetByFieldIds(tmpNodeId, ras->hVal, nodeId, {_q931(callIdentifier) _q931(guid) LAST_TOKEN}, NULL, NULL, NULL);
    if (tmpNodeId >= 0)
        if (pvtGetString(ras->hVal, tmpNodeId, sizeof(key.callID), (char*)key.callID) >= 0)
            key.flags |= catCallId;

    /* Find this call using CAT */
    hCatCall = catFind(ras->hCat, &key);

    if (hCatCall != NULL)
        return catGetCallHandle(ras->hCat, hCatCall);

    /* No such luck - no call found */
    return NULL;
}


/************************************************************************
 * rasFindIrqTxAndLock
 * purpose: Find an IRQ transaction related to an incoming IRR and
 *          lock that transaction if found
 * input  : ras             - RAS module to use
 *          requestSeqNum   - Sequence number in decoded message after hook
 * return : Solicited IRR transaction handle if found a possible IRQ matching
 *          NULL if 100% unsolicited IRR
 ************************************************************************/
rasOutTx* rasFindIrqTxAndLock(
    IN  rasModule*      ras,
    IN  UINT32          requestSeqNum)
{
    rasOutTx*   tx;
    void*       hashValue;

    /* Check in the hash table */
    meiEnter(ras->lockOutHash);
    hashValue = hashGetElement(ras->outHash, hashFind(ras->outHash, &requestSeqNum));
    if (hashValue != NULL)
        tx = *((rasOutTx **)hashValue);
    else
        tx = NULL;
    if (tx == NULL)
    {
        /* No outgoing transaction for this one - we can say for certain it's an unsolicited IRR */
        meiExit(ras->lockOutHash);
        return NULL;
    }

    /* Make sure the transaction's type matches an IRQ */
    emaLock((EMAElement)tx);
    meiExit(ras->lockOutHash);

    if (tx->transactionType != cmRASInfo)
    {
        /* Not an IRQ - its certainly an unsolicited IRR */
        emaUnlock((EMAElement)tx);
        return NULL;
    }

    return tx;
}


/************************************************************************
 * rasIsSolicited
 * purpose: Determines if a message is a solicited or an unsolicited IRR
 * input  : ras             - RAS module to use
 *          rootId          - Node ID of the message to check
 *          srcAddress      - Address of the sender
 *          requestSeqNum   - Sequence number in decoded message after hook
 * output : isSolicited     - TRUE if message is a solicited IRR
 *                            FALSE if message is an unsolicted IRR
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasIsSolicited(
    IN  rasModule*      ras,
    IN  int             rootId,
    IN  cmRASTransport* srcAddress,
    IN  UINT32          requestSeqNum,
    OUT BOOL*           isSolicited)
{
    rasOutTx*   tx;
    INT32       unsolicited, needResponse;
    int         status;
    int         tmpNodeId, calls;
    HCALL       hsCall;

    /* Check if we're using Version 4: This message includes indication of this is a
       solicited or an unsolicited IRR */
    __pvtGetByFieldIds(status, ras->hVal, rootId, {_anyField _q931(unsolicited) LAST_TOKEN}, NULL, &unsolicited, NULL);
    if (status >= 0)
    {
        *isSolicited = (unsolicited == FALSE);
        return 0;
    }

    /* No such luck - it's not Version 4. Check the amount of callInfo objects we've got.
       Other than 1 means it's a solicited IRR */
    __pvtGetByFieldIds(tmpNodeId, ras->hVal, rootId, {_anyField _q931(perCallInfo) LAST_TOKEN}, NULL, NULL, NULL);
    if (tmpNodeId < 0)
    {
        /* no perCallInfo field - it's a solicited IRR */
        *isSolicited = TRUE;
        return 0;
    }
    calls = pvtNumChilds(ras->hVal, tmpNodeId);
    if (calls < 0) return calls; /* error */
    if (calls != 1)
    {
        /* more than 1 call - it's a solicited IRR */
        *isSolicited = TRUE;
        return 0;
    }

    /* We've got a single call - check if needResponse = TRUE */
    __pvtGetByFieldIds(status, ras->hVal, rootId, {_anyField _q931(needResponse) LAST_TOKEN}, NULL, &needResponse, NULL);
    if ((status >= 0) && needResponse)
    {
        /* needResponse=TRUE - it's unsolicited IRR */
        *isSolicited = FALSE;
        return 0;
    }

    /* See if we can find a matching IRQ transaction for the IRR */
    tx = rasFindIrqTxAndLock(ras, requestSeqNum);
    if (tx == NULL)
    {
        *isSolicited = FALSE;
        return 0;
    }

    /* Seems like we have to find out if this call matches the one we've got */
    hsCall = rasFindCallHandleForIrr(ras, rootId, srcAddress);

    /* This is a solicited IRR if the call matches the one this IRQ is looking for, or if
       the IRQ was not called on a specific call. */
    *isSolicited = (tx->hsCall == NULL) || (hsCall == tx->hsCall);

    emaUnlock((EMAElement)tx);

    if (hsCall != NULL)
        return 0;
    else
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "rasIsSolicited: No call matched the IRR message (root=%d)", rootId));
        return RVERROR;
    }
}



/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * rasHandleIrr
 * purpose: Handle an incoming IRR message.
 *          This function first has to determine if this is a solicited
 *          or an unsolicited IRR and then handle it through incoming or
 *          outgoing transactions.
 * input  : ras             - RAS module to use
 *          srcAddress      - Address of the sender
 *          messageBuf      - The message buffer to send
 *          messageLength   - The length of the message in bytes
 *          messageNodeId   - Node ID of message root. If negative, then
 *                            message is decoded from given buffer and hook
 *                            is called
 *          requestSeqNum   - Sequence number in decoded message after hook
 * output : hMsgContext     - Incoming message context. Used mostly by security
 *                            If the returned value is different than NULL,
 *                            then the message context is not used by the
 *                            transaction and should be released
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasHandleIrr(
    IN  rasModule*      ras,
    IN  cmRASTransport* srcAddress,
    IN  BYTE*           messageBuf,
    IN  UINT32          messageLength,
    IN  int             messageNodeId,
    IN  UINT32          requestSeqNum,
    OUT void**          hMsgContext)
{
    int         status = 0;
    BOOL        isSolicited = FALSE;
    rasMessages newType;
    UINT32      newSeqNum;

    /* See if we have to decode this message or not */
    if (messageNodeId < 0)
    {
        status =
            rasDecodeAndRecv(ras, messageBuf, messageLength, FALSE, srcAddress,
                             &messageNodeId, &newType, &newSeqNum, hMsgContext);

        /* Make sure the application didn't change the sequence number or type of message */
        if ((status >= 0) && ((newType != rasMsgInfoRequestResponse) || (newSeqNum != requestSeqNum)))
        {
            /* Reroute the message to check it again... */
            return rasRouteMessage(ras, rasChanUnicast, srcAddress, messageBuf, messageLength, messageNodeId, newType, newSeqNum, hMsgContext);
        }
    }

    /* Check if message is solicited or unsolicited */
    if (status >= 0)
        status = rasIsSolicited(ras, messageNodeId, srcAddress, requestSeqNum, &isSolicited);

    if (status < 0)
    {
        if (messageNodeId >= 0)
            pvtDelete(ras->hVal, messageNodeId);
        return status;
    }

    if (isSolicited)
    {
        /* We've got a Solicited IRR - just handle it as a regular reply message */
        return rasHandleReply(ras, srcAddress, messageBuf, messageLength, messageNodeId, rasMsgInfoRequestResponse, requestSeqNum, hMsgContext);
    }


    /* If we're here, then we've got an unsolicited IRR.
       The problem we're facing is deciding whether this IRR is for a dummy request
       which is handled by the outgoing transactions as responses, or an incoming
       request transaction. */

    {
        HRAS        tx;
        HCALL       hsCall;
        RVHCATCALL  hCatCall;

        /* Find the call handle for this unsolicited IRR */
        hsCall = rasFindCallHandleForIrr(ras, messageNodeId, srcAddress);
        if (hsCall == NULL)
        {
            /* No call was found for this IRR - ignore it at this point */
            logPrint(ras->log, RV_ERROR,
                     (ras->log, RV_ERROR,
                     "rasHandleIrr: No call found for unsolicited IRR message - ignoring message (root=%d)", messageNodeId));
            pvtDelete(ras->hVal, messageNodeId);
            return RVERROR;
        }

        hCatCall = cmiGetCatForCall(hsCall);

        /* Find out if this call has a dummy request on it */
        tx = catGetUnsolicitedIRR(ras->hCat, hCatCall);
        if (tx != NULL)
        {
            /* There's a dummy transaction - handle as a response to it */
            emaLock((EMAElement)tx);

            return rasHandleTxResponse(ras, (rasOutTx *)tx, messageNodeId, rasMsgInfoRequestResponse, hMsgContext);
        }
        else
        {
            /* No dummy transaction - handle as an incoming request */
            return rasHandleRequest(ras, rasChanUnicast, srcAddress, messageBuf, messageLength, messageNodeId, rasMsgInfoRequestResponse, requestSeqNum, hsCall, hMsgContext);
        }
    }
}


/************************************************************************
 * rasSetIrrFields
 * purpose: Set the fields inside IRR messages, to be sent later on
 * input  : ras         - RAS module to use
 *          hsRas       - RAS transaction to set
 *          irrNode     - node on which to build the IRR
 *          hsCall      - Call related with the transaction
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasSetIrrFields(
    IN  rasModule*      ras,
    IN  HRAS            hsRas,
    IN  int             irrNode,
    IN  HCALL           hsCall)
{
    char                callid[16];
    INT32               rasCrv, callidLen, iVal;
    int                 ret;
    int                 rootId, destNodeId;
    BOOL                isOrigin;
    cmTransportAddress  addr;

    isOrigin = cmCallGetOrigin(hsCall,NULL);

    /* Make sure we've got a response node */
    rasSetParam(ras, hsRas, cmRASTrStageConfirm, cmRASParamEmpty, 0, 0, NULL);

    __pvtBuildByFieldIds(rootId, ras->hVal, irrNode,
        {_q931(infoRequestResponse) _q931(perCallInfo) _nul(1) LAST_TOKEN}, 0, NULL);

    /* callReferenceValue */
    if (cmCallGetParam(hsCall, cmParamRASCRV, 0, &rasCrv, NULL) >= 0)
        pvtAdd(ras->hVal, rootId, __q931(callReferenceValue), rasCrv, NULL, NULL);

    /* conferenceID */
    callidLen = sizeof(callid);
    if (cmCallGetParam(hsCall, cmParamCID, 0, &callidLen, callid) >= 0)
        pvtAdd(ras->hVal, rootId, __q931(conferenceID), callidLen, callid, NULL);

    /* callIdentifier */
    callidLen = sizeof(callid);
    if (cmCallGetParam(hsCall, cmParamCallID, 0, &callidLen, callid) >= 0)
    {
        __pvtBuildByFieldIds(ret, ras->hVal, rootId, {_q931(callIdentifier) _q931(guid) LAST_TOKEN}, callidLen, callid);
    }

    /* originator */
    pvtAdd(ras->hVal, rootId, __q931(originator), isOrigin, NULL, NULL);

    /* callType */
    if (cmCallGetParam(hsCall, cmParamCallType, 0, &iVal, NULL) >= 0)
    {
        destNodeId = pvtAdd(ras->hVal, rootId, __q931(callType), 0, NULL, NULL);
        switch ((cmCallType)iVal)
        {
            case cmCallTypeP2P:     pvtAdd(ras->hVal, destNodeId, __q931(pointToPoint), 0, NULL, NULL); break;
            case cmCallTypeOne2N:   pvtAdd(ras->hVal, destNodeId, __q931(oneToN), 0, NULL, NULL); break;
            case cmCallTypeN2One:   pvtAdd(ras->hVal, destNodeId, __q931(nToOne), 0, NULL, NULL); break;
            case cmCallTypeN2Nw:    pvtAdd(ras->hVal, destNodeId, __q931(nToN), 0, NULL, NULL); break;
        }
    }

    /* bandWidth */
    if (cmCallGetParam(hsCall, cmParamRate, 0, &iVal, NULL) >= 0)
        pvtAdd(ras->hVal, rootId, __q931(bandWidth), iVal/50, NULL, NULL);

    /* callModel */
    destNodeId = pvtAdd(ras->hVal, rootId, __q931(callModel), 0, NULL, NULL);
    if (cmIsRoutedCall(hsCall))
        pvtAdd(ras->hVal, destNodeId, __q931(gatekeeperRouted), 0, NULL, NULL);
    else
        pvtAdd(ras->hVal, destNodeId, __q931(direct), 0, NULL, NULL);

    /* h245, substituteConfIDs */
    pvtAdd(ras->hVal, rootId, __q931(h245), 0, NULL, NULL);
    pvtAdd(ras->hVal, rootId, __q931(substituteConfIDs), 0, NULL, NULL);

    /* callSignaling */
    destNodeId = pvtAdd(ras->hVal, rootId, __q931(callSignaling), 0, NULL, NULL);
    if (isOrigin)
    {
        if (cmCallGetParam(hsCall, cmParamSrcCallSignalAddress, 0, NULL, (char*)&addr) >= 0)
            cmTAToVt(ras->hVal, pvtAdd(ras->hVal, destNodeId, __q931(recvAddress), 0, NULL, NULL), &addr);
        if (cmCallGetParam(hsCall, cmParamDestCallSignalAddress, 0, NULL, (char*)&addr) >= 0)
            cmTAToVt(ras->hVal, pvtAdd(ras->hVal, destNodeId, __q931(sendAddress), 0, NULL, NULL), &addr);
    }
    else
    {
        if (cmCallGetParam(hsCall, cmParamSrcCallSignalAddress, 0, NULL, (char*)&addr) >= 0)
            cmTAToVt(ras->hVal, pvtAdd(ras->hVal, destNodeId, __q931(sendAddress), 0, NULL, NULL), &addr);
        if (cmCallGetParam(hsCall, cmParamDestCallSignalAddress, 0, NULL, (char*)&addr) >= 0)
            cmTAToVt(ras->hVal, pvtAdd(ras->hVal, destNodeId, __q931(recvAddress), 0, NULL, NULL), &addr);
    }

    return 0;
}



#ifdef __cplusplus
}
#endif

