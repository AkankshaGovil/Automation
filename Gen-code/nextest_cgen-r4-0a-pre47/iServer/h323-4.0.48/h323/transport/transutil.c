#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/


/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#include <transport.h>
#include <transportint.h>
#include <transnet.h>
#include <transutil.h>
#include <transStates.h>
#include <q931asn1.h>
#include <h245.h>
#include <cm.h>
#include <emanag.h>
#include <cat.h>
#include <netutl.h>
#include <cmutils.h>
#include <prnutils.h>
#include <psyntreeStackApi.h>
#include <Threads_API.h>
#include <tls.h>



/**************************************************************************************
 * createMessage
 *
 * Purpose: creates a skeleton message of the given type and fills in the CRV and callId
 *          of the given session.
 * Input:   transGlobals    - The global data of the module.
 *          msgType         - which message to create
 *          CRV             - CRV to use.
 *          callId          - call Identifier to use.
 *
 * Output:  node - the created message.
 *
 * Returned Value:  TRUE - success
 **************************************************************************************/
BOOL createMessage(cmTransGlobals    *transGlobals,
                   cmCallQ931MsgType msgType,
                   UINT16            CRV,
                   BYTE              *callId,
                   int               *node)
{
    int newMessage, messageNode = -1, res;

    /* create a skeleton message */
    if (transGlobals->sessionEventHandlers.cmEvTransGetMessageNode)
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmEvTransGetMessageNode(hPvt = %x, msgType=%d)",
                transGlobals->hPvt, msgType));

        transGlobals->sessionEventHandlers.cmEvTransGetMessageNode(
                                                        transGlobals->hAppATrans,
                                                        msgType,
                                                        &messageNode);
    }

    /* copy the message and add session data to the message */
    newMessage = pvtAddRoot(transGlobals->hPvt, NULL, 0, NULL);

    if (newMessage >= 0)
    {
        INT32 temp = CRV;
        pvtSetTree(transGlobals->hPvt, newMessage, transGlobals->hPvt, messageNode);

        __pvtBuildByFieldIds(res, transGlobals->hPvt, newMessage,
                                {
                                  _q931(callReferenceValue)
                                  _q931(twoBytes)
                                  LAST_TOKEN
                                },
                             temp, NULL);

        if (res >= 0)
        {
            __pvtBuildByFieldIds(res, transGlobals->hPvt, newMessage,
                                    { _q931(message)
                                      _anyField
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _anyField
                                      _q931(callIdentifier)
                                      _q931(guid)
                                      LAST_TOKEN
                                    },
                                16, (char *)callId);
        }
        else
        {
            pvtDelete(transGlobals->hPvt, newMessage);
            return FALSE;
        }
    }
    else
    {
        pvtDelete(transGlobals->hPvt, newMessage);
        return FALSE;
    }

    cleanMessage(transGlobals->hPvt, newMessage);

    if (node)
        *node = newMessage;

    return TRUE;
}

/**************************************************************************************
 * findHostInHash
 *
 * Purpose: To look for an existing host with the same address as the given one.
 *          If a suitable host is found, i.e. identical in address and type,
 *          we check it to be connected (the prefered state). If it's not we look for
 *          another one which is connected. If none was found we bring a suitable host
 *          that is not connected. If such a host doesn't exist either, return NULL.
 * Input:   transGlobals    - The global data of the module.
 *          remoteAddress   - The address of the looked for host.
 *          isMultiplexed   - Do we need a host which is multiplexed or not
 *          isAnnexE        - the type of the connection (TPKT or annex E).
 *
 * Output:  None.
 *
 * Returned Value:  A pointer to the found host element or NULL if not found
 **************************************************************************************/
cmTransHost *findHostInHash(IN cmTransGlobals       *transGlobals,
                            IN cmTransportAddress   *remoteAddress,
                            IN BOOL                 isMultiplexed,
                            IN BOOL                 isAnnexE)
{
    void *loc;
    hostKey        key;
    BOOL           found = FALSE;
    cmTransHost    *host = NULL;
    cmTransHost    *lastNonActiveHost = NULL;

    /* build the key to look by */
    key.ip   = remoteAddress->ip;
    key.port = (UINT32)remoteAddress->port;
    key.type = isAnnexE;
    /* lock the hash for the search */

    /* for non annex E go over all hosts that satisfy the key, i.e. address+type */
    meiEnter(transGlobals->tablesLock);
    if (!isAnnexE)
    for (loc = hashFind(transGlobals->hHashHosts,(void *)&key);
         (loc != NULL) && (!found);
         loc = hashFindNext(transGlobals->hHashHosts, (void *)&key, loc))
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "hashFind = %x [hostKey: ip=%x port=%d type=%d]",
                loc,key.ip, key.port, key.type));

        host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);

        /* if both host and session are multiplexed, we found our host */
        if ( (host->isMultiplexed) && (host->remoteIsMultiplexed) && (isMultiplexed) )
            found = TRUE;
        else
        /* if both host and session are not multiplexed, but the host is available for
           the session, i.e. not servicing another session, we found it */
        if ( (!host->isMultiplexed) && (!isMultiplexed) && (!host->firstSession) )
            found = TRUE;

        if (found)
        {
            /* if the host is not yet connected, try to look for a connected one,
               but remember the last non connected one to be used if no connected
               one would be found */
            if ( (host->state != hostConnected) && (host->state != hostBusy) )
            {
                found = FALSE;
                lastNonActiveHost = host;
            }
        }
    }
    else
    {
        /* for annex E hosts there should be just one suitable host element */
        loc = hashFind(transGlobals->hHashHosts,(void *)&key);
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "hashFind = %x [hostKey: ip=%x port=%d type=%d]",
                loc,key.ip, key.port, key.type));
        if (loc)
        {
            host = *(cmTransHost **)hashGetElement(transGlobals->hHashHosts, loc);
            found = TRUE;
        }
    }
    meiExit(transGlobals->tablesLock);

    /* if we found a suitable host, return it, else return a NULL */
    if (found)
        return host;
    else
        return lastNonActiveHost;
}

/**************************************************************************************
 * transEncodeMessage
 *
 * Purpose: to get the given message encoded, either by a callback or by the cmEmEncode.
 * Input:   host            - The host on which the message is to be sent
 *          session         - The session on whose behalf the message is to be sent.
 *          transGlobals    - The global data of the transport module.
 *          pvtNode         - the message to be encoded
 *          buffer          - the buffer into which to encode
 *
 * Output:  encMsgSize - The size of the encoded message.
 *
 * Returned Value: cmTransErr - in case that the encoding failed, cmTransOK - otherwise
 **************************************************************************************/
 TRANSERR transEncodeMessage(cmTransHost    *host,
                             cmTransSession *session,
                             cmTransGlobals *transGlobals,
                             int            pvtNode,
                             BYTE           *buffer,
                             int            *encMsgSize)
 {
     HATRANSHOST    haTransHost = NULL;
     HATRANSSESSION haTransSession;
     int            encSize;
     int            encRes = 0;
     TRANSERR       encTransRes = cmTransOK;
     int            headersSize = TPKT_HEADER_SIZE + MSG_HEADER_SIZE;
     TRANSTYPE      type;

    /* determine the application handles of the host and session */
    haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);
    if (host)
        haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

    /* determine the type of message, if there is no host then it's a H.245 tunneled message,
       otherwise it's either a H.245 message or a Q.931 according to the host's type */
    type = cmTransH245Type;
    if (host)
        if ( (host->type == cmTransQ931Conn) )
            type = cmTransQ931Type;

    /* Report and encode the given message */
    if (!host)
    {
        INTPTR fieldId;
        int nodeId;
        char *ret;

        nodeId = pvtChild(transGlobals->hPvt, pvtNode);
        nodeId = pvtChild(transGlobals->hPvt, nodeId);

        pvtGet(transGlobals->hPvt,
               nodeId,
               &fieldId,
               NULL, NULL, NULL);
        ret = pstGetFieldNamePtr(transGlobals->synProtH245, fieldId);

        logPrint(transGlobals->hTPKTCHAN, RV_DEBUG,
                (transGlobals->hTPKTCHAN, RV_DEBUG,
                "New TUNNELED message (call %d-%x) sent --> %s:",
                emaGetIndex((EMAElement)haTransSession), haTransSession, (ret)?(ret):NULL));
        pvtPrintStd(transGlobals->hPvt, pvtNode, (int)transGlobals->hTPKTCHAN);
    }

    if ( (transGlobals->hostEventHandlers.cmEvTransSendRawMessage) && (host) )
    {
        int sessNumOfLocks, hostNumOfLocks;
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmEvTransSendRawMessage(host=%d(%x), haHost=%x, session=%d, hsSession=%x, pvtNode=%d)",
                emaGetIndex((EMAElement)host), host, haTransHost, emaGetIndex((EMAElement)session),
                haTransSession, pvtNode));

        sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
        hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
        encTransRes = transGlobals->hostEventHandlers.cmEvTransSendRawMessage(
                        (HSTRANSHOST)host,
                        haTransHost,
                        (HSTRANSSESSION) session,
                        haTransSession,
                        pvtNode,
                        transGlobals->codingBufferSize - headersSize,
                        &buffer[headersSize],
                        &encSize);
        emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
    }
    else
        encRes = cmEmEncode(transGlobals->hPvt,
                            pvtNode,
                            &buffer[headersSize],
                            transGlobals->codingBufferSize - headersSize,
                            &encSize);
    if (encMsgSize)
        *encMsgSize = encSize;

    if ( (encRes < 0) || ( (encTransRes != cmTransOK) && (encTransRes != cmTransIgnoreMessage) ) )
    {
        if (transGlobals->sessionEventHandlers.cmEvTransBadMessage)
        {
            int numOfLocks;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransBadMessage(hsSession = %d(%p), haSession=%p, type=%d,outgoing=TRUE)",
                    emaGetIndex((EMAElement)session), session, haTransSession, type));

            numOfLocks = emaPrepareForCallback((EMAElement)session);
            transGlobals->sessionEventHandlers.cmEvTransBadMessage(
                        (HSTRANSSESSION) session,
                        haTransSession,
                        type,
                        &buffer[headersSize],
                        encSize,
                        TRUE);
            emaReturnFromCallback((EMAElement)session, numOfLocks);
        }

        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "transEncodeMessage Failed on encoding"));
        return cmTransErr;
    }
    else
    if (encTransRes == cmTransIgnoreMessage)
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "transEncodeMessage Mesage rejected by user"));
        return cmTransIgnoreMessage;
    }

    /* print just for tunneled messages here */
    if (!host)
        printHexBuff(&buffer[headersSize], encSize, transGlobals->hTPKTCHAN);
    return cmTransOK;
 }

/**************************************************************************************
 * addSessionToPendingList
 *
 * Purpose: Add a session to the list of sessions that were unable to send a message
 *          due to lack of rpool resources. Such sessions should be notified when
 *          resources become available again.
 * Input:   transGlobals - The global data of the transport module.
 *          session      - The session which failed to send the message.
 *
 * Output:  None.
 *
 **************************************************************************************/
void addSessionToPendingList(IN cmTransGlobals *transGlobals,
                             IN cmTransSession *session)
{
    cmTransSession *oldFirstSession;

    meiEnter(transGlobals->lock);

    oldFirstSession                 = transGlobals->pendingList;
    session->nextPending            = oldFirstSession;
    session->prevPending            = NULL;
    if (oldFirstSession)
        oldFirstSession->prevPending    = session;
    transGlobals->pendingList       = session;

    meiExit(transGlobals->lock);

}

/**************************************************************************************
 * removeSessionFromPendingList
 *
 * Purpose: Remove a session from the list of sessions that were unable to send a message
 *          due to lack of rpool resources. Such sessions are removed when notified that
 *          resources become available again.
 * Input:   transGlobals - The global data of the transport module.
 *          session      - The session which failed to send the message.
 *
 * Output:  None.
 *
 **************************************************************************************/
void removeSessionFromPendingList(
    IN cmTransGlobals *transGlobals,
    IN cmTransSession *session)
{
    cmTransSession *previousSession;

    meiEnter(transGlobals->lock);

    previousSession = session->prevPending;

    /* disconnect the session from the pending list */
    if (previousSession)
        previousSession->nextPending    = session->nextPending;
    if (session->nextPending)
        session->nextPending->prevPending = previousSession;

    /* update the list header if session was the head of the list */
    if (transGlobals->pendingList == session)
        transGlobals->pendingList = session->nextPending;

    meiExit(transGlobals->lock);
}

/**************************************************************************************
 * notifyPendingSessions
 *
 * Purpose: This routine notifies that space is free for pending messages.
 * Input:   transGlobals - The global data of the transport module.
 *          numOfSession - The number of sessions that can be released from the list.
 *
 * Output:  None.
 *
 **************************************************************************************/
void notifyPendingSessions(cmTransGlobals *transGlobals, int numOfSessions)
{
    cmTransSession  *session = transGlobals->pendingList;
    int i;

    for (i=0; (i<numOfSessions && (session)); i++,session=session->nextPending)
    {
        if (transGlobals->sessionEventHandlers.cmEvTransWrite)
        {
            int numOfLocks;
            HATRANSSESSION haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "cmEvTransWrite hsTransSession=%d(%x) haTransSession=%x",
                    emaGetIndex((EMAElement)session), session,  haTransSession));

            numOfLocks = emaPrepareForCallback((EMAElement)session);
            transGlobals->sessionEventHandlers.cmEvTransWrite((HSTRANSSESSION) session,
                                                              haTransSession);
            emaReturnFromCallback((EMAElement)session, numOfLocks);

            removeSessionFromPendingList(transGlobals, session);
        }
    }
}

/**************************************************************************************
 * saveMessageToPool
 *
 * Purpose: This routine gets an encoded message and saves it, until it can send it.
 * Input:   transGlobals - The global data of the transport module.
 *          session      - The session which wants to send the message.
 *          host         - The host on which the message is to be sent.
 *          buffer       - The encoded message.
 *          encMsgSize   - Its size.
 *          isTunneled   -  TRUE: save to the tunneled message queue,
 *                          FALSE: to the sending message queue
 *          addToTop     - TRUE: Add the message to the start of the queue
 *                         FALSE: Add it at the end.
 *          CRV          - The CRV of the call on which behalf the message is sent
 *                         (used for annex E session ID field).
 *
 * Output:  None.
 *
 **************************************************************************************/
 void * saveMessageToPool(cmTransGlobals    *transGlobals,
                            void            *element,
                            BYTE            *buffer,
                            int             encMsgSize,
                            BOOL            isTunneled,
                            BOOL            addToTop,
                            UINT16          CRV)
 {
     void           *newMsg;
     void           *firstMsg;
     void           *lastMsg;
     cmTransHost    *host       = (cmTransHost *)element;
     cmTransSession *session    = (cmTransSession *)element;

    /* clear the admin header & TPKT header */
    memset(&buffer[0], 0, MSG_HEADER_SIZE);
    memset(&buffer[MSG_HEADER_SIZE], 0, TPKT_HEADER_SIZE);

    /* lock the rpool */
    meiEnter(transGlobals->tablesLock);

    /* get rpool buffer and copy the encoded message to it */
    newMsg  = rpoolAllocCopyExternal(transGlobals->messagesRPool,
                                     buffer,
                                     encMsgSize+MSG_HEADER_SIZE+TPKT_HEADER_SIZE);
    if (newMsg)
    {

        /* determine which queue of messages we work on (tunneled, or to be sent)*/
        if (isTunneled)
        {
            firstMsg = session->firstMessage;
            lastMsg  = session->lastMessage;
        }
        else
        {
            firstMsg = host->firstMessage;
            lastMsg  = host->lastMessage;
        }

        /* update the pointer from the element to the saved messages */
        if (!firstMsg)
        {
                firstMsg = newMsg;
                lastMsg  = newMsg;
        }

        if (addToTop)
        {
            if (firstMsg != newMsg)
            {
                rpoolCopyFromExternal(transGlobals->messagesRPool,
                                      newMsg,
                                      (void *)&firstMsg,
                                      0,
                                      MSG_HEADER_SIZE);
                firstMsg = newMsg;
            }
        }
        else
        {
            /* update the last saved message to point to the new message
               and make the new message to be the last message */
            if (lastMsg != newMsg)
            {
                rpoolCopyFromExternal(transGlobals->messagesRPool,
                                      lastMsg,
                                      (void *)&newMsg,
                                      0,
                                      MSG_HEADER_SIZE);
                lastMsg = newMsg;
            }
        }

        /* update the CRV on the TPKT header */
        rpoolCopyFromExternal(transGlobals->messagesRPool,
                              newMsg,
                              (void *)&CRV,
                              MSG_HEADER_SIZE,
                              sizeof(CRV));


        /* Update the last message pointer in the appropriate element */
        if (isTunneled)
        {
            session->firstMessage = firstMsg;
            session->lastMessage  = lastMsg;
        }
        else
        {
            host->firstMessage = firstMsg;
            host->lastMessage  = lastMsg;
        }


        {
            INT32 poolSize, availableSize, allocatedSize;

            if (rpoolStatistics(transGlobals->messagesRPool,
                                &poolSize,
                                &availableSize,
                                &allocatedSize))
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "saveMessageToPool[isTunneled=%d] statistics: max=%d, available=%d, allocated=%d",
                        isTunneled, poolSize, availableSize, allocatedSize));
            }
        }

        logPrint(transGlobals->hLog, RV_DEBUG,
                (transGlobals->hLog, RV_DEBUG,
                        "saveMessageToPool[isTunneled=%d] first = %x last = %x new = %x",
                        isTunneled, host->firstMessage, host->lastMessage, newMsg));

		/* update num of messages kept in rpool for sending */
		transGlobals->curUsedNumOfMessagesInRpool++;
		if (transGlobals->curUsedNumOfMessagesInRpool > transGlobals->maxUsedNumOfMessagesInRpool)
			transGlobals->maxUsedNumOfMessagesInRpool++;

        /* unlock the rpool */
        meiExit(transGlobals->tablesLock);
    }
    else
    {
        /* NO BUFFERS */

        {
            INT32 poolSize, availableSize, allocatedSize;

            if (rpoolStatistics(transGlobals->messagesRPool,
                                &poolSize,
                                &availableSize,
                                &allocatedSize))
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "saveMessageToPool (no buffers) statistics: max=%d, available=%d, allocated=%d",
                        poolSize,availableSize, allocatedSize));
            }
        }

        /* unlock the rpool */
        meiExit(transGlobals->tablesLock);

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "saveMessageToPool has no buffers"));
        return NULL;
    }
    return newMsg;
 }

/**************************************************************************************
 * extractMessageFromPool
 *
 * Purpose: This routine removes an encoded message from the head of the host list.
 * Input:   transGlobals - The global data of the transport module.
 *          element      - The host or session which wants to send the message.
 *          isTunneled   -  TRUE: remove from the tunneled message queue,
 *                          FALSE: remove from the sending message queue
 *
 * Output:  None.
 *
 * Return Value: next message.
 *
 **************************************************************************************/
 void *extractMessageFromPool(cmTransGlobals    *transGlobals,
                             void           *element,
                             BOOL           isTunneled)

 {
     void           *nextMsg;
     void           *firstMsg;
     cmTransHost    *host       = (cmTransHost *)element;
     cmTransSession *session    = (cmTransSession *)element;
     INT32          poolSize, availableSize, allocatedSize;

    /* determine which queue of messages we work on (tunneled, or to be sent)*/
    if (isTunneled)
        firstMsg = session->firstMessage;
    else
        firstMsg = host->firstMessage;

    /* lock the rpool */
    meiEnter(transGlobals->tablesLock);

    /* get the pointer to the next message */
    rpoolCopyToExternal(transGlobals->messagesRPool, &nextMsg, firstMsg, 0, MSG_HEADER_SIZE);

    /* delete the first message */
    rpoolFree(transGlobals->messagesRPool, firstMsg);

    /* set the next message as the new first message */
    if (isTunneled)
        session->firstMessage = nextMsg;
    else
        host->firstMessage = nextMsg;

    if (rpoolStatistics(transGlobals->messagesRPool,
                        &poolSize,
                        &availableSize,
                        &allocatedSize))
    {

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "extractMessageFromPool[isTunneled=%d] statistics: max=%d, available=%d, allocated=%d",
                isTunneled, poolSize, availableSize, allocatedSize));

        if (transGlobals->pendingList)
        {
            int numOfSessionsToRelease;
            if ((numOfSessionsToRelease = availableSize/transGlobals->codingBufferSize) > 0)
                notifyPendingSessions(transGlobals, numOfSessionsToRelease);
        }
    }

   logPrint(transGlobals->hLog, RV_DEBUG,
           (transGlobals->hLog, RV_DEBUG,
                  "extractMessageFromPool[isTunneled=%d] first = %x last = %x",
                  isTunneled, host->firstMessage, host->lastMessage));

	/* update num of messages kept in rpool for sending */
	transGlobals->curUsedNumOfMessagesInRpool--;

    /* unlock the rpool */
    meiExit(transGlobals->tablesLock);

    return nextMsg;
 }

/**************************************************************************************
 * processQ931OutgoingMessage
 *
 * Purpose: This routine gets a decoded outgoing Q.931 message and modifies it according to
 *          the different tasks at hand:
 *          - Add fields such as multiplexing flags.
 *          - Insert H.245 addresses, if necessary to the messages.
 *          - Insert tunneled messages (H.245, Annex M, Annex L).
 * Input:   session     - The session which wants to send the message.
 *          host        - The host on which the message is to be sent.
 *          pvtNode     - The message.
 *
 * Output:  None.
 *
 * Returned Value: TRUE - send the message, FALSE - don't send it.
 *
 **************************************************************************************/
BOOL processQ931OutgoingMessage(IN    cmTransSession    *session,
                                IN    cmTransHost       *host,
                                INOUT int               pvtNode)
{
    cmTransGlobals  *transGlobals;
    int             messageBodyNode;
    int             msgType;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* check what type of Q.931 message we have here */
    msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);
    if (msgType < 0)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "processQ931OutgoingMessage failed to get message type tag"));
        return FALSE;
    }

    /* position on the UU-IE part of the message */
    __pvtGetNodeIdByFieldIds(   messageBodyNode,
                                transGlobals->hPvt,
                                pvtNode,
                                {   _q931(message)
                                    _anyField
                                    _q931(userUser)
                                    _q931(h323_UserInformation)
                                    _q931(h323_uu_pdu)
                                    LAST_TOKEN
                                });
    /* if the message doesn't have UU-IE part, we have nothing to do with it, just
       send it */
    if (messageBodyNode < 0)
        return TRUE;

    /***********************************************************************************/
    /* for first message on the session get the CRV and callID and insert to the hash  */
    /***********************************************************************************/
    if (!session->firstMessageSent)
    {
        if (!findSessionByMessage(transGlobals, host, pvtNode, FALSE, &session))
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "processQ931OutgoingMessage failed to locate session in HASH, ignore emssage"));
            return FALSE;
        }
        else
        if (session)
        {
            /* if session is NULL here this means that we are sending a CRV==0
               message so no session is associated with it */
            if ( (msgType != cmQ931setupAck) &&
                 (msgType != cmQ931information) &&
                 (msgType != cmQ931progress) &&
                 (msgType != cmQ931status) &&
                 (msgType != cmQ931statusInquiry)
                )
                /* mark that we've already sent a call that could have approved tunneling */
                session->firstMessageSent = TRUE;
        }
    }

    /***********************************************************************/
    /* set the multipleCalls and maintainConnection parameters (multiplexing stuff) */
    /***********************************************************************/
    setMultiplexedParams(transGlobals, session, host, pvtNode, msgType);

    /***********************************************************************/
    /* check the fast start state of the session */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        setTheFastStartStateByMessage(transGlobals, session, messageBodyNode, msgType);

    /***********************************************************************/
    /* check the parallel tunneling state of the session  */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        setTheParallelTunnelingStateByMessage(transGlobals,
                                              session,
                                              msgType,
                                              messageBodyNode,
                                              TRUE /* outgoing */);

    /***********************************************************************/
    /* handling whether to start listenning or establish connection for H.245 */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        if (!session->notEstablishControl)
            if (!determineIfToOpenH245(TRUE /* outgoing message */,
                                       transGlobals,
                                       session,
                                       messageBodyNode,
                                       msgType))
                return FALSE;

    /***********************************************************************/
    /* handle reporting of a new H.245 connection to the user    */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
        if (!session->notEstablishControl)
            reportH245(TRUE /* outgoing message */, 
            transGlobals, 
            session,
            host,
            messageBodyNode,
            msgType);

    /***********************************************************************/
    /* handle insertion of tunneled H.245 messages to the Q.931 message    */
    /***********************************************************************/
    if ( (session) && !emaWasDeleted((EMAElement)session) )
    {
		insertH245TunnelingFlag(transGlobals, session, messageBodyNode, msgType);
        if (!session->notEstablishControl)
            insertH245TunneledMessages(transGlobals, session, messageBodyNode, msgType);
        insertH450TunneledMessages(transGlobals, session, messageBodyNode);
        insertAnnexLTunneledMessages(transGlobals, session, messageBodyNode);
        insertAnnexMTunneledMessages(transGlobals, session, messageBodyNode);
    }

    return TRUE;
}

/**************************************************************************************
 * processH245OutgoingMessage
 *
 * Purpose: This routine gets a decoded outgoing H.245 message and modifies it according to
 *          the different tasks at hand:
 *          - If tunneling: encodes it and puts it into the H.245 tunneled messages sylo
 *                          and inhibits its sending.
 *          - If not tunneling: do nothing.
 *
 * Input:   session     - The session which wants to send the message.
 *          host        - The host on which the message is to be sent.
 *          pvtNode     - The message.
 *
 * Output:  None.
 *
 * Returned Value: TRUE - send the message, FALSE - don't send it.
 *
 **************************************************************************************/
BOOL processH245OutgoingMessage(IN    cmTransSession    *session,
                                IN    cmTransHost       *host,
                                INOUT int               pvtNode)
{
    cmTransGlobals  *transGlobals;
    TRANSERR        res;
    void            *msg;
    BYTE            *buffer;
    int             encMsgSize;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    /* find out if we are still in tunneling procedure */
    if (!host)
    {
        BOOL addToTop;
        int  node;

        /* determine if this is a TCS Ack in parallel tunneling response,
           in that case put it first */
        __pvtGetNodeIdByFieldIds(node, transGlobals->hPvt, pvtNode,
                                { _h245(response)
                                  _h245(terminalCapabilitySetAck)
                                  LAST_TOKEN});

        if ( (node >=0) && (session->parallelTunnelingState == parllApproved) )
            addToTop = TRUE;
        else
            addToTop = FALSE;

        /* Start the encoding process */
        getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);
        res = transEncodeMessage(host,
                                 session,
                                 transGlobals,
                                 pvtNode,
                                 buffer,
                                 &encMsgSize);
        if (res != cmTransOK)
            return FALSE;

        /* save the tunneled encoded message for later transmit */
        msg = saveMessageToPool(transGlobals,
                                (void *)session,
                                buffer,
                                encMsgSize,
                                TRUE,
                                addToTop,
                                session->CRV);
        if (!msg)
        {
            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "processH245OutgoingMessage failed on tunneled message - no buffers"));
            return FALSE;
        }

    }
    else
        return TRUE;

    return FALSE;
}
/**************************************************************************************
 * processOutgoingMessage
 *
 * Purpose: This routine gets a decoded outgoing message and modifies it according to
 *          the different tasks at hand:
 *          - Add fields to Q.931 messages, such as multiplexing flags.
 *          - Save H.245 tunneled messages and eliminate their actual sending
 *          - Insert H.245 addresses, if necessary to the messages.
 *          - Insert H.245 tunneled messages.
 * Input:   session     - The session which wants to send the message.
 *          host        - The host on which the message is to be sent.
 *          pvtNode     - The message.
 *          type        - The type of the message (Q.931/H.245)
 *
 * Output:  None.
 *
 * Returned Value: TRUE - send the message, FALSE - don't send it.
 *
 **************************************************************************************/
BOOL processOutgoingMessage(IN    cmTransSession    *session,
                            IN    cmTransHost       *host,
                            INOUT int               pvtNode,
                            IN    TRANSTYPE         type)
{
    cmTransGlobals  *transGlobals;
    BOOL            sendMessage = TRUE;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

    switch (type)
    {
        /* handle Q.931 messages */
        case cmTransQ931Type:
        {
            sendMessage = processQ931OutgoingMessage(session, host, pvtNode);
            break;
        }

        /* handle H.245 messages */
        case cmTransH245Type:
        {
            sendMessage = processH245OutgoingMessage(session, host, pvtNode);
            break;
        }

        default:
            break;
    }

    return sendMessage;
}

/**************************************************************************************
 * transSessionClose
 *
 * Purpose: This routine closes a session and breaks all its associations.
 *
 * Input/Output:   hsTransSession - The session to be closed.
 *
 * Output:  None.
 *
 **************************************************************************************/
void transSessionClose(cmTransSession *session)
{
    cmTransGlobals *transGlobals;
    void           *msg;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)session);

	/* delete session timer if exist */
	mtimerReset(transGlobals->hTimers, session->muxConnectTimer);
	session->muxConnectTimer = (HTI)-1;
	mtimerReset(transGlobals->hTimers, session->h245ConnectTimer);
	session->h245ConnectTimer = (HTI)-1;


    /* remove it from the pending list, if it's there */
    removeSessionFromPendingList(transGlobals, (cmTransSession *)session);

    /* close the H.245 connection of the session */
    if (session->H245Connection)
        closeHostInternal((HSTRANSHOST)session->H245Connection, TRUE);

    /* Release all nodes held on the session */
    if (session->h450Element >= 0)
        pvtDelete(transGlobals->hPvt, session->h450Element);

    if (session->annexMElement >= 0)
        pvtDelete(transGlobals->hPvt, session->annexMElement);

    if (session->annexLElement >= 0)
        pvtDelete(transGlobals->hPvt, session->annexLElement);

    /* remove session from host list of sessions */
    {
        cmTransHost    *host;

        /* update, if necessary, the list header for the host */
        if (session->Q931Connection)
        {
            host = session->Q931Connection;

            /* remove the session from the host */
            meiEnter(transGlobals->lock);
            if (host->firstSession == session)
                host->firstSession = session->nextSession;
            meiExit(transGlobals->lock);

            /* if no more sessions on host, close host, if necessary */
            if (!host->firstSession)
                if ((host->closeOnNoSessions) || (host->remoteCloseOnNoSessions) ||
                    (!host->isMultiplexed) || (!host->remoteIsMultiplexed))
                {
                    closeHostInternal((HSTRANSHOST)host, TRUE);
                }
            session->Q931Connection = NULL;

        }

        if (session->annexEConnection)
        {
            host = session->annexEConnection;

            /* remove the session from the host */
            meiEnter(transGlobals->lock);
            if (host->firstSession == session)
                host->firstSession = session->nextSession;
            meiExit(transGlobals->lock);

            /* if no more sessions on host, close host, if necessary */
            if (!host->firstSession)
            {
                host->hostIsApproved = FALSE;
                if ((host->closeOnNoSessions) || (host->remoteCloseOnNoSessions))
                    closeHostInternal((HSTRANSHOST)host, TRUE);
            }

            session->annexEConnection = NULL;
        }

        /* remove it from the list */
        meiEnter(transGlobals->lock);
        if (session->nextSession)
            session->nextSession->prevSession = session->prevSession;
        if (session->prevSession)
            session->prevSession->nextSession = session->nextSession;
        meiExit(transGlobals->lock);
    }

    /* remove the session from the hash table */
    meiEnter(transGlobals->tablesLock);
    if (session->hashEntry)
    {
        hashDelete(transGlobals->hHashSessions, session->hashEntry);
        session->hashEntry = NULL;
    }
    meiExit(transGlobals->tablesLock);

    /* go over the tunneled messages still stored for the session and free them from the pool */
    msg = session->firstMessage;
    meiEnter(transGlobals->tablesLock);
    while (msg)
        msg = extractMessageFromPool(transGlobals, session, TRUE);
    meiExit(transGlobals->tablesLock);

}


/**************************************************************************************
 * transHostClose
 *
 * Purpose: This routine reports a host close and tells the sessions connected to
 *          it that the connection has ended and releasing all the messages kept for it
 *          in the pool.
 *
 * Input/Output:   hsTransHost      - The host to be deleted.
 *                 killHost         - Should the host be removed from the hash and close
 *                                    its TPKT element or just disconnect from session?
 *
 * Output:  None.
 *
 **************************************************************************************/
void transHostClose(HSTRANSHOST hsTransHost, BOOL killHost)
{
    cmTransHost *host;
    cmTransGlobals *transGlobals;
    void        *sess;
    void        *msg;
    void        *nextSess;
    int         numOfLocks;

    if (!hsTransHost)
        return;

    host = (cmTransHost *)hsTransHost;
    if (!host)
        return;

    /* retrieve the transport module global area */
    transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

    /* close the hosts tpkt elements */
    if ( (host->h245Listen) && (killHost) )
    {
        tpktClose(host->h245Listen);
        host->h245Listen = NULL;
    }

    if ( (host->hTpkt) && (killHost) )
    {
        tpktClose(host->hTpkt);
        host->hTpkt = NULL;
    }

    /* close the hosts annex E node */
    if ( (host->annexEUsageMode == cmTransUseAnnexE) && (killHost) )
    {
        annexEStatus eStat = annexECloseNode(transGlobals->annexECntrl,
                                             host->remoteAddress.ip,
                                             host->remoteAddress.port);
        if (eStat != annexEStatusNormal)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "failed to close annex E node on host=%x",host));
        }
    }

    /* We have to cancel timer which was set at "accept" time - we're closing this host anyway */
    if (host->connectTimer != (HTI)-1)
    {
        mtimerReset(transGlobals->hTimers, host->connectTimer);
        host->connectTimer = (HTI)-1;
    }
    
    meiEnter(transGlobals->lock);
    sess = (void *)host->firstSession;
    msg  = (void *)host->firstMessage;
    meiExit(transGlobals->lock);

    logPrint(transGlobals->hLog, RV_INFO,
        (transGlobals->hLog, RV_INFO,
        "transHostClose(hsTransHost = %d-%x)",
        emaGetIndex((EMAElement)hsTransHost), hsTransHost));

    /* go over the messages still stored for the host and free them from the pool */
    meiEnter(transGlobals->tablesLock);
    while (msg)
        msg = extractMessageFromPool(transGlobals, host, FALSE);
    if (host->incomingMessage)
    {
        rpoolFree(transGlobals->messagesRPool, host->incomingMessage);
        {
            INT32 poolSize, availableSize, allocatedSize;

            if (rpoolStatistics(transGlobals->messagesRPool,
                                &poolSize,
                                &availableSize,
                                &allocatedSize))
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "rpoolFree[transHostClose] statistics: max=%d, available=%d, allocated=%d",
                        poolSize,availableSize, allocatedSize));
            }
        }
        host->incomingMessage = NULL;
    }
    meiExit(transGlobals->tablesLock);

    /* go over all the sessions that are connected to this host */

    /* unlock the host so it won't be locked before the sessions */
    numOfLocks = emaPrepareForCallback((EMAElement)host);

    while ((sess != NULL) && (emaLock((EMAElement)sess)))
    {
        /* report to the user that the connection of the session was closed */
        HATRANSSESSION haTransSession;
        cmTransSession *session = (cmTransSession *)sess;
        BOOL           hostReported;
        BOOL           anotherQ931HostExists;


        {
            haTransSession = (HATRANSSESSION)emaGetApplicationHandle((EMAElement)session);

            meiEnter(transGlobals->lock);
            nextSess = session->nextSession;
            meiExit(transGlobals->lock);

            /* determine whether the closing host was reported to the user, so we need to report its
               closing too */
            hostReported = ( ((session->Q931Connection == host)   && (session->reportedQ931Connect)) ||
                             ((session->annexEConnection == host) && (session->reportedQ931Connect)) ||
                             ((session->H245Connection == host)   && (session->reportedH245Connect)) );

            /* check if it's a Q.931 host and if ther are two (TPKT & annex E) so the closing is due to
               a race between them, and no reporting is needed to the user at this stage */
            anotherQ931HostExists = ( ((session->Q931Connection == host) || (session->annexEConnection == host)) &&
                                      (session->Q931Connection && session->annexEConnection) );

            if ( (transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed) &&
                 hostReported && (!anotherQ931HostExists))
            {
                /* in case of tunneling report also the close of the H.245 tunneled connection */
                if ( (host->type == cmTransQ931Conn) &&
                     (session->tunnelingState == tunnApproved) &&
                     (session->reportedH245Connect) )
                {
                    int numOfLocks;
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransConnectionOnSessionClosed(hsSession = %d(%x), haSession=%x type = cmTransH245Conn)",
                            emaGetIndex((EMAElement)session), session, haTransSession, cmTransH245Conn));

                    numOfLocks = emaPrepareForCallback((EMAElement)session);
                    transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed(
                                                              (HSTRANSSESSION)session,
                                                              haTransSession,
                                                              cmTransH245Conn);
                    emaReturnFromCallback((EMAElement)session, numOfLocks);
                }

                if (!emaWasDeleted((EMAElement)session))
                {
                    int numOfLocks;

                    /* report the actual disconnect */
                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransConnectionOnSessionClosed(hsSession = %d(%x), haSession=%x type = %d)",
                            emaGetIndex((EMAElement)session), session, haTransSession, host->type));

                    numOfLocks = emaPrepareForCallback((EMAElement)session);
                    transGlobals->sessionEventHandlers.cmEvTransConnectionOnSessionClosed(
                                                              (HSTRANSSESSION)session,
                                                              haTransSession,
                                                              host->type);
                    emaReturnFromCallback((EMAElement)session, numOfLocks);
                }
            }


            if (!emaWasDeleted((EMAElement)session))
            {
                /* disconnect the session from the host */
                if (host->type == cmTransH245Conn)
                {
                    session->H245Connection      = NULL;

                    /* for H.245 there is only one session per host and it's the first that the host points too */
                    nextSess = NULL;
                }
                else
                {
                    if (host == session->Q931Connection)
                        session->Q931Connection = NULL;
                    else
                    if (host == session->annexEConnection)
                        session->annexEConnection = NULL;
                }
            }

            emaUnlock((EMAElement)session);
        }

        /* Get the next session */
        sess = (void *)nextSess;
    }

    /* reinstate the host lock */
    emaReturnFromCallback((EMAElement)host, numOfLocks);

    /* remove it from its hash table and delete it from EMA */
    meiEnter(transGlobals->tablesLock);
    if ( (host->hashEntry) && (killHost) )
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "hashDelete = %x",host->hashEntry));

        hashDelete(transGlobals->hHashHosts, host->hashEntry);
        host->hashEntry = NULL;
    }
    if (!emaWasDeleted((EMAElement)host))
        emaDelete((EMAElement)host);
    meiExit(transGlobals->tablesLock);
}

/**************************************************************************************
 * findSessionByMessage
 *
 * Purpose: looks according to the CRV and callID of the message for an entry of the session
 *          in the host's hash table. If none was found, creates an entry.
 *          In case of non-session releated messages (CRV = 0) the routine treats it
 *          as an error and expect the upper layer to handle the case.
 *
 * Input:   transGlobals    - The global data of the transport module.
 *          host            - The host on which the message arrived.
 *          pvtNode         - The decoded message.
 *          isAnnexE        - is the host on which the message came is an annex E host?
 *
 * Output:  session - The session found or created.
 *
 * returned value: FALSE - an error occured, TRUE - all is OK (that doesn't mean that we
 *                                                  have a session ).
 *
 **************************************************************************************/
BOOL findSessionByMessage(cmTransGlobals *transGlobals,
                          cmTransHost    *host,
                          int            pvtNode,
                          BOOL           isAnnexE,
                          cmTransSession **session)
{
    UINT16          CRV = 0;
    INT32           temp = 0;
    BYTE            callID[16];
    BYTE            nullCallID[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int             msgType;
    BOOL            bNullCallId = FALSE;
    int             res;

    void            *loc;
    sessionKey      key;
    BOOL            found;

    /* check what type of Q.931 message we have here */
    msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);
    if (msgType < 0)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "findSessionByMessage failed to get message type tag"));
        return FALSE;
    }

    /* get the CRV and callID (if exists) of the message and build the key for CAT */
    __pvtGetByFieldIds(res, transGlobals->hPvt,pvtNode,
                        {
                          _q931(callReferenceValue)
                          _q931(twoBytes)
                          LAST_TOKEN
                        },
                        NULL,(INT32 *)&temp,NULL);
    CRV = (UINT16)temp;


    __pvtGetNodeIdByFieldIds(res, transGlobals->hPvt, pvtNode,
                            { _q931(message)
                              _anyField
                              _q931(userUser)
                              _q931(h323_UserInformation)
                              _q931(h323_uu_pdu)
                              _q931(h323_message_body)
                              _anyField
                              _q931(callIdentifier)
                              _q931(guid)
                              LAST_TOKEN
                            });
    if (res>=0)
    {
        pvtGetString(transGlobals->hPvt, res, sizeof(callID), (char *)callID);
        if (memcmp(nullCallID, callID, sizeof(callID))==0)
            bNullCallId = TRUE;
    }
    else
    {
        bNullCallId = TRUE;
        memcpy(callID, nullCallID, sizeof(callID));
    }

    /* check if this is a message not attached to a particular session,
       i.e. CRV = 0, callID = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} */

    if ( (CRV == 0) && (bNullCallId) )
    {
        *session = NULL;

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "findSessionByMessage found a general message"));
        return TRUE;
    }

    if (!*session)
    {
        /* no session exists yet, this is an incoming message */

        /* since its an incoming message change the bit direction of the CRV */
        CRV ^= 0x8000;

        /* look for the entry in the hash */
        key.CRV  = CRV;
        found    = FALSE;

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "findSessionByMessage incoming CRV (findHash) = %x host=%d(%x)",
                key.CRV,emaGetIndex((EMAElement)host), host));

        /* go over all the sessions with the same CRV from this host, hopefully
           there is only one (in most cases) */
        meiEnter(transGlobals->tablesLock);
        for (loc = hashFind(transGlobals->hHashSessions,(void *)&key);
             (loc != NULL) && (!found);
             loc = hashFindNext(transGlobals->hHashSessions, (void *)&key, loc))
        {
            cmTransSession *sess = *(cmTransSession **)hashGetElement(transGlobals->hHashSessions,
                                                                      loc);
            /* check that the session has the same call ID as the message */
            if (memcmp(sess->callId, callID, sizeof(callID)) == 0)
            {
                *session = sess;
                found = TRUE;
            }
            else
            /* if we don't have call ID (version 1 or CRV0 messages), check that
               the session is from our host */
            if (bNullCallId)
            {
                /* check first for the TPKT connection */
                if  ( (!isAnnexE) &&
                      (sess->Q931Connection) &&
                      (sess->Q931Connection->remoteAddress.ip   == host->remoteAddress.ip) &&
                      (sess->Q931Connection->remoteAddress.port == host->remoteAddress.port) )
                {
                    *session = sess;
                    found = TRUE;
                }

                /* our last chance that it's from the annex E connection */
                if  ( (isAnnexE) &&
                      (sess->annexEConnection) &&
                      (sess->annexEConnection->remoteAddress.ip   == host->remoteAddress.ip) &&
                      (sess->annexEConnection->remoteAddress.port == host->remoteAddress.port) )
                {
                    *session = sess;
                    found = TRUE;
                }
            }
        }
        meiExit(transGlobals->tablesLock);

        /* no session was found, this must be a new incoming message */
        if (!found)
        {
            TRANSERR err = cmTransErr;

            /* we may create new session only for new incoming SETUP message */
            if (msgType == cmQ931setup)
                /* create a new session */
                err = cmTransCreateSession((HAPPTRANS)transGlobals,
                                            NULL,
                                            (HSTRANSSESSION *)session);
            else
            {
                logPrint(transGlobals->hLog, RV_WARN,
                        (transGlobals->hLog, RV_WARN,
                        "findSessionByMessage new incoming message which is not SETUP, type=%d", msgType));
                return FALSE;
            }

            if (err != cmTransOK)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "findSessionByMessage failed to create a new session"));
                return FALSE;
            }
            else
            {
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "findSessionByMessage created a new session %d(%x)",
                        emaGetIndex((EMAElement)*session), *session));


                /* add the new session to the hash table */
                key.CRV  = CRV;
                logPrint(transGlobals->hLog, RV_INFO,
                        (transGlobals->hLog, RV_INFO,
                        "findSessionByMessage incoming CRV (AddHash)= %x host=%d(%x) session=%d(%x)",
                        key.CRV,emaGetIndex((EMAElement)host), host, emaGetIndex((EMAElement)*session), *session));
                meiEnter(transGlobals->tablesLock);
                loc = hashAdd(transGlobals->hHashSessions,(void *)&key,(void *)session, FALSE);
                meiExit(transGlobals->tablesLock);
                if (!loc)
                {
                    cmTransCloseSession((HSTRANSSESSION)*session);

                    logPrint(transGlobals->hLog, RV_ERROR,
                            (transGlobals->hLog, RV_ERROR,
                            "findSessionByMessage failed to add session to hash table"));
                    return FALSE;
                }
                else
                {
                    /* update the sessions callIs and CRV and mark it as incoming session */
                    memcpy((*session)->callId, callID, sizeof(callID));
                    (*session)->CRV = CRV;
                    (*session)->hashEntry = loc;
                    (*session)->outgoing = FALSE;
                }
            }
        }
        else
        if (msgType == cmQ931setup)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "findSessionByMessage found session %d (%x) for an incoming setup message !!!!",
                    emaGetIndex((EMAElement)*session), *session));
            return FALSE;
        }


        /* connect the session to the host */
        if (!isAnnexE)
            (*session)->Q931Connection   = host;
        else
            (*session)->annexEConnection = host;

        /* add the session to the hosts list of session.
           Do it just for the first time, i.e. when the SETUP
           message arrives.
           (in case of TPKT & annex E race, done just for TPKT host,
           for annex E this will be done if and when it will win the race */
        if ( (!isAnnexE) && (msgType == cmQ931setup) )
        {
            meiEnter(transGlobals->lock);
            /* connect the session to the host */
            (*session)->nextSession = host->firstSession;
            if ((*session)->nextSession)
                (*session)->nextSession->prevSession = (*session);
            host->firstSession = (*session);
            meiExit(transGlobals->lock);
        }

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "findSessionByMessage found session %d (%x)",
                emaGetIndex((EMAElement)*session), *session));


    }
    else
    {
        /* if session exists this means this is a first outgoing message on a session
            created by the user, add it to the hash table
        */
        if (!(*session)->hashEntry)
        {
            void *loc  = NULL;

            key.CRV  = CRV;

            logPrint(transGlobals->hLog, RV_INFO,
                    (transGlobals->hLog, RV_INFO,
                    "findSessionByMessage outgoing CRV (AddHAsh) = %x host=%d(%x) session=%d(%x)",
                    key.CRV,emaGetIndex((EMAElement)host), host, emaGetIndex((EMAElement)*session), *session));

            meiEnter(transGlobals->tablesLock);
            loc = hashAdd(transGlobals->hHashSessions,(void *)&key,(void *)session, FALSE);
            meiExit(transGlobals->tablesLock);

            if (!loc)
            {
                cmTransCloseSession((HSTRANSSESSION)*session);

                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "findSessionByMessage failed to add session to hash table"));
                return FALSE;
            }
            else
            {
                memcpy((*session)->callId, callID, sizeof(callID));
                (*session)->CRV = CRV;
                (*session)->hashEntry = loc;
            }
        }
    }

    return TRUE;
}

/**************************************************************************************
 * setRemoteAddress
 *
 * Purpose: sets the remote address to the host and adds an entry to the host hash table
 *
 * Input:   host            - The host to which the address is to be set.
 *          remoteAddress   - The address to be set.
 *
 * Return Value: cmTransOK - if all is well; cmTranErr - otherwise
 *
 **************************************************************************************/
TRANSERR setRemoteAddress(cmTransHost *host, cmTransportAddress *remoteAddress)
{
    void *res;
    hostKey key;

    /* retrieve the transport module global area */
    cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)host);

	if (!remoteAddress)
		return cmTransErr;
	
    if ((int)remoteAddress->type != -1)
    {
        host->remoteAddress = *remoteAddress;

        key.ip   = host->remoteAddress.ip;
        key.port = (UINT32)host->remoteAddress.port;
        key.type = (host->annexEUsageMode != cmTransNoAnnexE);

        meiEnter(transGlobals->tablesLock);
        res = hashAdd(transGlobals->hHashHosts,(void *)&key, (void *)&host, FALSE);
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "hashAdd = %x [hostKey: ip=%x port=%d type=%d]",
                res,key.ip, key.port, key.type));
        meiExit(transGlobals->tablesLock);

        if (!res)
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "setRemoteAddress failed on updating hosts hash table"));
            return cmTransErr;
        }
        else
            host->hashEntry = res;
    }
    return cmTransOK;
}

/************************************************************************************
 * canWeAcceptTheCall
 *
 * Purpose: to check if this is the first call on this host, or if this host supports
 *          multiplexing. If this host doesn't support multiplexing and already has a
 *          session attached to it, refuse the message and send releaseComplete on
 *          the call.
 *
 * Input: transGlobals - The global variables of the module
 *        host         - the host through which the message was received.
 *        pvtNode      - the decoded message
 *
 * Output: None
 *
 * returned value: TRUE  - accept the message
 *                 FALSE - refuse it and disconnect the call.
 *
 **************************************************************************************/
BOOL canWeAcceptTheCall(cmTransGlobals *transGlobals, cmTransHost *host, int pvtNode)
{
    int     msgType;
    UINT16  CRV = 0;
    BYTE    callId[16];
    BOOL    sendReleaseComplete = FALSE;
    BOOL    sendMessage = TRUE;
    BOOL    oor = FALSE;

    /* check what type of Q.931 message we have here */
    msgType = pvtGetChildTagByPath(transGlobals->hPvt, pvtNode, "message", 1);
    if (msgType < 0)
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "canWeAcceptTheCall failed to get message type tag"));
        return FALSE;
    }

    /* if first incoming message on the call */
    if (msgType == cmQ931setup)
    {
        /* if host already has one or more calls on it */
        if (host->firstSession)
        {
            /* if host no longer (or never) supports multiplexing */
            if ( (!host->isMultiplexed) || (!host->remoteIsMultiplexed) )
            {
                sendMessage         = FALSE;
                sendReleaseComplete = TRUE;
            }
        }
        else
        {
            /* See if we have an aveliable seesion to accept the new call on */
            emaStatistics stats;
            meiEnter(transGlobals->lock);
            if( (emaGetStatistics(transGlobals->hEmaSessions, &stats) < 0) ||
                (stats.elems.cur == stats.elems.max) )
            {
                /* we cannot accept call */
                sendMessage         = FALSE;
                sendReleaseComplete = TRUE;
                oor = TRUE;
            }
            meiExit(transGlobals->lock);
        }
    }

    if (sendReleaseComplete)
    {
        /* get the callId and CRV from the SETUP message */
        INT32 temp;
        int res;

        __pvtGetByFieldIds(res, transGlobals->hPvt,pvtNode,
                            {
                              _q931(callReferenceValue)
                              _q931(twoBytes)
                              LAST_TOKEN
                            },
                            NULL,(INT32 *)&temp,NULL);
        if (res>=0)
        {
            CRV = (UINT16)temp;
            /* it's incoming message, so turn the last bit */
            CRV ^= 0x8000;

            __pvtGetNodeIdByFieldIds(res, transGlobals->hPvt, pvtNode,
                                    { _q931(message)
                                      _anyField
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _anyField
                                      _q931(callIdentifier)
                                      _q931(guid)
                                      LAST_TOKEN
                                    });
            if (res>=0)
                pvtGetString(transGlobals->hPvt, res, sizeof(callId), (char *)callId);
        }

        if (res < 0)
        {
            /* something is wrong */
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "canWeAcceptTheCall Failed to extract CRV & callID from SETUP"));
        }
        else
        {
            if(oor)
                sendReleaseCompleteMessage(transGlobals, host, CRV, callId, -1, __q931(adaptiveBusy));
            else
                sendReleaseCompleteMessage(transGlobals, host, CRV, callId, -1, __q931(newConnectionNeeded));
        }
    }

    return sendMessage;
}

/************************************************************************************
 * encodeAndSend
 *
 * Purpose: encodes and sends a given decoded message on a given host. No checks and
 *          no processing is done on the message.
 *          Note: The node of the decoded message remains untouched.
 *
 * Input: transGlobals - The global variables of the module
 *        host         - the host through which the message is to be sent.
 *        newMessage   - the decoded message.
 *
 * Output: None
 *
 * return value: TRUE: messahe was sent, FALSE: sending failed.
 *
 **************************************************************************************/
BOOL encodeAndSend(cmTransGlobals *transGlobals, cmTransHost *host, int newMessage)
{
    BYTE        *buffer;
    int         res = 0;
    int         encoded;
    void        *msg;
    TRANSERR    err;
    int         headersSize = TPKT_HEADER_SIZE + MSG_HEADER_SIZE;

    getEncodeDecodeBuffer(transGlobals->codingBufferSize, &buffer);

    if ( (transGlobals->hostEventHandlers.cmEvTransSendRawMessage) && (host) )
    {
        int         hostNumOfLocks;
        TRANSERR    encTransRes;
        HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)host);

        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmEvTransSendRawMessage(host=%d(%x), haHost=%x, session=NULL, hsSession=NULL, pvtNode=%d)",
                emaGetIndex((EMAElement)host), host, haTransHost, newMessage));

        hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
        encTransRes = transGlobals->hostEventHandlers.cmEvTransSendRawMessage(
                        (HSTRANSHOST)host,
                        haTransHost,
                        NULL,
                        NULL,
                        newMessage,
                        transGlobals->codingBufferSize - headersSize,
                        &buffer[headersSize],
                        &encoded);
        emaReturnFromCallback((EMAElement)host, hostNumOfLocks);
        if (encTransRes != cmTransOK)
            res = -1;
    }
    else
        res = cmEmEncode(transGlobals->hPvt,
                         newMessage,
                         &buffer[headersSize],
                         transGlobals->codingBufferSize-headersSize,
                         &encoded);

    if (res < 0)
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "encodeAndSend failed to encode for host %d(%x)",emaGetIndex((EMAElement)host), host));
        return FALSE;
    }

    msg = saveMessageToPool(transGlobals,
                            (void *)host,
                            buffer,
                            encoded,
                            FALSE,
                            FALSE,
                            0);

    if (!msg)
    {
        logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "encodeAndSend has no buffers for host %d(%x)",emaGetIndex((EMAElement)host), host));
        return FALSE;
    }

    /* initiate the actual send through the appropriate protocol */
    err = sendMessageOnHost(transGlobals, host);
    if ( (err != cmTransOK) && (err != cmTransConnectionBusy) )
    {
        logPrint(transGlobals->hLog, RV_ERROR,
                (transGlobals->hLog, RV_ERROR,
                "encodeAndSend Failed on send for host %d(%x)",emaGetIndex((EMAElement)host), host));
        return FALSE;
    }

    return TRUE;
}

/************************************************************************************
 * sendGeneralFacility
 *
 * Purpose: creates a general purpose faclity message with CRV and callId set to zero.
 *          sets in itthe host's current multiplex parameters, encodes it and send it
 *          on the host.
 *
 * Input: transGlobals - The global variables of the module
 *        host         - the host through which the message is to be sent.
 *
 * Output: None
 *
 **************************************************************************************/
void sendGeneralFacility(cmTransGlobals *transGlobals, cmTransHost *host)
{
    int      newMessage = -1;
    BYTE     nullCallId[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    /* create facility with CRV and callId = 0 */
    if (createMessage(transGlobals, cmQ931facility, 0, nullCallId, &newMessage))
    {
        int res;

        /* set the reason */
        __pvtBuildByFieldIds(res, transGlobals->hPvt, newMessage,
                                { _q931(message)
                                  _q931(facility)
                                  _q931(userUser)
                                  _q931(h323_UserInformation)
                                  _q931(h323_uu_pdu)
                                  _q931(h323_message_body)
                                  _anyField
                                  _q931(facility)
                                  _q931(undefinedReason)
                                 LAST_TOKEN
                                },
                             0, NULL);
        /* set the current multiplex params into the message */
        setMultiplexedParams(transGlobals,NULL,host,newMessage,cmQ931facility);

        if (!encodeAndSend(transGlobals, host, newMessage))
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "sendGeneralFacility Failed on send for host %d(%x)",
                    emaGetIndex((EMAElement)host), host));
        }

        pvtDelete(transGlobals->hPvt, newMessage);
    }
}

/************************************************************************************
 * initiateTunnelingFacility
 *
 * Purpose: if we are in a 'stable' state we need to initiate a facility send
 *          to get rid of the tunneled messages, and initiate it.
 *              The condition for that are:
 *              a. We have tunneled messages to send
 *              b. At least one message was already sent from this end
 *              c. Tunneling was approved
 *
 * Input: transGlobals - The global variables of the module
 *        session      - the session whose state is tested.
 *        host         - the related locked host, if at all
 *
 * Output: None
 *
 **************************************************************************************/
void initiateTunnelingFacility(cmTransGlobals *transGlobals,
                               cmTransSession *session,
                               cmTransHost    *host)
{
    BOOL messagesExist = ( (session->firstMessage && !session->holdTunneling) ||
                           (session->h450Element >= 0) ||
                           (session->annexMElement >= 0) ||
                           (session->annexLElement >= 0) );
    
    BOOL tunnelingIsStable = (  (   (session->tunnelingState == tunnApproved) ||
                                    (session->tunnelingState == tunnNo) )
                                &&
                                (   (session->parallelTunnelingState == parllNo) ||
                                    (session->parallelTunnelingState == parllApproved)) );
    
    if (messagesExist  &&
        session->firstMessageSent &&
        (tunnelingIsStable || session->forceTunneledMessage))
    {
        int      newMessage = -1;
        TRANSERR err;

        if (createMessage(transGlobals,
            cmQ931facility,
            session->CRV,
            session->callId,
            &newMessage)
            )
        {
            int res;
            char remoteVer[8];
            int sessNumOfLocks, hostNumOfLocks=0;

            sessNumOfLocks = emaPrepareForCallback((EMAElement)session);
            if (host != NULL) 
                hostNumOfLocks = emaPrepareForCallback((EMAElement)host);
            cmGetH225RemoteVersion((HCALL)emaGetApplicationHandle((EMAElement)session),remoteVer);
            emaReturnFromCallback((EMAElement)session, sessNumOfLocks);
            if (host != NULL)
                emaReturnFromCallback((EMAElement)host, hostNumOfLocks);

            if (strcmp(remoteVer, "4") >= 0)
            {
                /* version 4 or laster */
                __pvtBuildByFieldIds(res, transGlobals->hPvt, newMessage,
                                        { _q931(message)
                                          _q931(facility)
                                          _q931(userUser)
                                          _q931(h323_UserInformation)
                                          _q931(h323_uu_pdu)
                                          _q931(h323_message_body)
                                          _q931(facility)
                                          _q931(reason)
                                          _q931(transportedInformation)
                                          LAST_TOKEN
                                        },
                                        0, NULL);
            }
            else
            {
                /* pre version 4 */
                __pvtBuildByFieldIds(res, transGlobals->hPvt, newMessage,
                                        { _q931(message)
                                          _q931(facility)
                                          _q931(userUser)
                                          _q931(h323_UserInformation)
                                          _q931(h323_uu_pdu)
                                          _q931(h323_message_body)
                                          _q931(empty)
                                          LAST_TOKEN
                                        },
                                        0, NULL);
            }
            err = cmTransSendMessage((HSTRANSSESSION)session, newMessage, cmTransQ931Type);
            pvtDelete(transGlobals->hPvt, newMessage);
            if (err != cmTransOK)
            {
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "initiateTunnelingFacility failed to send facility (transportedInformation) message session=%d(%x)",
                        emaGetIndex((EMAElement)session), session));
            }
            else
            {
                session->forceTunneledMessage = FALSE;
            }
        }
        else
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "initiateTunnelingFacility failed to create facility (transportedInformation) message session=%d(%x)",
                    emaGetIndex((EMAElement)session), session));
        }
    }
}

/************************************************************************************
 * sendReleaseCompleteMessage
 *
 * Purpose: initiate a send of releaseComplete in case that:
 *              a.  A call was initiated on a non multiplexed connection that
 *                  has already other call(s).
 *              b.  We ran out of resources (mainly sessions)
 *
 * Input: transGlobals - The global variables of the module
 *        host         - the host through which the message is to be sent.
 *        CRV          - the CRV to attach to the message
 *        callId       - the call identifier to attach to th emessage
 *        cause        - The call release cause
 *        reasonNameId - field Id of the disconnect reason.
 *
 * Output: None
 *
 **************************************************************************************/
void sendReleaseCompleteMessage(cmTransGlobals  *transGlobals,
                                cmTransHost     *host,
                                UINT16          CRV,
                                BYTE            *callId,
                                int             cause,
                                INTPTR          reasonNameId)
{
    BOOL sendReleaseComplete = TRUE;
    int  newMessage = -1;
    int  node = -1;


    /* create releaseComplete with the CRV and callID and
       put the disconnection reason into it */
    if (sendReleaseComplete)
    {
        if (createMessage(transGlobals, cmQ931releaseComplete, CRV, callId, &newMessage))
        {

            if (reasonNameId >= 0)
            {
                __pvtBuildByFieldIds(node,transGlobals->hPvt, newMessage,
                                { _q931(message)
                                  _q931(releaseComplete)
                                  _q931(userUser)
                                  _q931(h323_UserInformation)
                                  _q931(h323_uu_pdu)
                                  _q931(h323_message_body)
                                  _q931(releaseComplete)
                                  _q931(reason)
                                  LAST_TOKEN
                                },
                                0, NULL);

                if (node >= 0)
                    node = pvtAdd(transGlobals->hPvt, node, reasonNameId, 0, NULL, NULL);
            }

            if ( (node >= 0) && (cause >= 0) )
            {
                __pvtBuildByFieldIds(node, transGlobals->hPvt, newMessage,
                                        {   _q931(message)
                                            _q931(releaseComplete)
                                            _q931(cause)
                                            _q931(octet3)
                                            _q931(codingStandard)
                                            LAST_TOKEN
                                        },
                                        0, NULL);

                __pvtBuildByFieldIds(node, transGlobals->hPvt, newMessage,
                                        {   _q931(message)
                                            _q931(releaseComplete)
                                            _q931(cause)
                                            _q931(octet3)
                                            _q931(spare)
                                            LAST_TOKEN
                                        },
                                        0, NULL);

                __pvtBuildByFieldIds(node, transGlobals->hPvt, newMessage,
                                        {   _q931(message)
                                            _q931(releaseComplete)
                                            _q931(cause)
                                            _q931(octet3)
                                            _q931(location)
                                            LAST_TOKEN
                                        },
                                        0, NULL);

                __pvtBuildByFieldIds(node, transGlobals->hPvt, newMessage,
                                        {   _q931(message)
                                            _q931(releaseComplete)
                                            _q931(cause)
                                            _q931(octet4)
                                            _q931(causeValue)
                                            LAST_TOKEN
                                        },
                                        cause, NULL);
            }
        }

        if (node< 0)
        {
            /* something is wrong */
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "sendReleaseCompleteMessage Failed to insert cause and reason to message"));

            sendReleaseComplete = FALSE;
        }
    }


    /* Start the encoding process */
    if (sendReleaseComplete)
    {
        if (!encodeAndSend(transGlobals, host, newMessage))
        {
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "sendReleaseCompleteMessage Failed to send releaseComplete message"));
        }
    }

    /* delete the message */
    if (newMessage)
        pvtDelete(transGlobals->hPvt, newMessage);

}

/************************************************************************************
 * getGoodAddress
 *
 * Purpose: Calculate the ip address as follows:
 *
 *          a. Try and get the allocated ip address from the TCP/IP stack.
 *          b. if 0, try and get the Q.931 connection ip address.
 *          c. if 0, use the localIPAddress as was given in cmTransStart.
 *          d. if 0, take the first ip address of the machine.
 *
 *          Note: for Q.931 connections, step a is being skipped.
 *
 * Input: transGlobals - The global variables of the module.
 *        hTpkt        - The tpkt element of the socket whose address we want to find.
 *        q931         - The Q.931 connection of a given H.245 socket (relevent only when
 *                       type is cmTransH245Conn).
 *        type         - The type of the connection: Q.931, H.245 or annexE.
 *
 * Output: addr - The calculated good ip address.
 *
 **************************************************************************************/
void getGoodAddress(cmTransGlobals     *transGlobals,
                    HTPKT              hTpkt,
                    cmTransHost        *q931Host,
                    TRANSCONNTYPE      type,
                    cmTransportAddress *addr)
{
    int   ip;

	if (!addr)
		return;

    ip = addr->ip;

    /* calculate the right ip address */
    if (addr->type!=cmTransportTypeNSAP)
        addr->type=cmTransportTypeIP;
    if (addr->type==cmTransportTypeIP)
    {
        /* if none was given try getting the TCP/IP allocated one */
        if (!ip)
            if (hTpkt)
                ip = liGetSockIP(tpktGetSock(hTpkt));

        /* if that failed, just for H.245 sockets try and get the
           Q.931 allocated ip */
        if ( (!ip) && (type == cmTransH245Conn) )
            if (q931Host)
                ip = q931Host->localAddress.ip;

        /* if no address yet, use the given localIPAddress */
        if (!ip)
            ip = transGlobals->localIPAddress;

        /* if still no address, try and get the first ip address of the machine */
        if (!ip)
        {
           UINT32 **ipA=liGetHostAddrs();
           if (ipA && *ipA && **ipA)
                 ip = **ipA;
        }
    }

    /* update the address components */
    addr->ip           = ip;
    if (hTpkt)
        addr->port     = liGetSockPort(tpktGetSock(hTpkt));
    addr->distribution = (isMulticastIP(addr->ip))?cmDistributionMulticast:cmDistributionUnicast;
}

/************************************************************************************
 * sendStartH245Facility
 *
 * Purpose: to create and send a facility message with reason startH245.
 *
 *
 * Input: transGlobals - The global variables of the module.
 *        session      - The session on which the facility is to be sent.
 *
 * Output: None.
 *
 **************************************************************************************/
void sendStartH245Facility(cmTransGlobals *transGlobals, cmTransSession *session)
{
    int newMessage = -1;
    int res;
    TRANSERR err;
    UINT16   CRV;

    if (session->needToSendFacilityStartH245)
    {
        session->needToSendFacilityStartH245 = FALSE;

        /* make sure we're listenning on the local address */
        /* start a listenning process on it (if we don't have a connection already) */
        if (emaLock((EMAElement)session->H245Connection))
        {
            if (session->H245Connection->state == hostIdle)
            {
                TRANSERR reply = cmTransOK;

                getGoodAddress(transGlobals,
                               session->H245Connection->h245Listen,
                               session->Q931Connection,
                               cmTransH245Conn,
                               &session->H245Connection->localAddress);
                session->H245Connection->localAddress.port = 0;

                /* start listenning */
                if (transGlobals->hostEventHandlers.cmEvTransHostListen)
                {
                    int         numOfLocks;
                    HATRANSHOST haTransHost =
                                 (HATRANSHOST)emaGetApplicationHandle(((EMAElement)session->H245Connection));

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostListen(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d))",
                            emaGetIndex((EMAElement)session->H245Connection),
                            session->H245Connection,
                            haTransHost,
                            session->H245Connection->localAddress.ip,
                            session->H245Connection->localAddress.port));

                    numOfLocks = emaPrepareForCallback((EMAElement)session->H245Connection);
                    reply = transGlobals->hostEventHandlers.cmEvTransHostListen(
                                            (HSTRANSHOST)session->H245Connection,
                                            haTransHost,
                                            cmTransH245Conn,
                                            &session->H245Connection->localAddress);
                    emaReturnFromCallback((EMAElement)session->H245Connection, numOfLocks);
                    session->H245Connection->reported = TRUE;
                }

                if (reply == cmTransOK)
                    session->H245Connection->h245Listen =
                            tpktOpen(transGlobals->tpktCntrl,
                                     session->H245Connection->localAddress.ip,
                                     session->H245Connection->localAddress.port,
                                     tpktServer,
                                     transH245AcceptHandler,
                                     (void *)session->H245Connection);
                else
                    session->H245Connection->h245Listen = NULL;

                /* get the allocated local address */
                getGoodAddress(transGlobals,
                               session->H245Connection->h245Listen,
                               session->Q931Connection,
                               cmTransH245Conn,
                               &session->H245Connection->localAddress);

                if (transGlobals->hostEventHandlers.cmEvTransHostListening)
                {
                    int         numOfLocks;
                    HATRANSHOST haTransHost =
                                 (HATRANSHOST)emaGetApplicationHandle(((EMAElement)session->H245Connection));

                    logPrint(transGlobals->hLog, RV_INFO,
                            (transGlobals->hLog, RV_INFO,
                            "cmEvTransHostListening(hsHost = %d(%x), haHost=%x, type=cmTransH245Conn, address = (ip:%x,port:%d) error = %d)",
                            emaGetIndex((EMAElement)session->H245Connection),
                            session->H245Connection,
                            haTransHost,
                            session->H245Connection->localAddress.ip,
                            session->H245Connection->localAddress.port,
                            (session->H245Connection->h245Listen == NULL)));

                    numOfLocks = emaPrepareForCallback((EMAElement)session->H245Connection);
                    transGlobals->hostEventHandlers.cmEvTransHostListening(
                                            (HSTRANSHOST)session->H245Connection,
                                            haTransHost,
                                            cmTransH245Conn,
                                            &session->H245Connection->localAddress,
                                            (session->H245Connection->h245Listen == NULL));
                    emaReturnFromCallback((EMAElement)session->H245Connection, numOfLocks);
                    session->H245Connection->reported = TRUE;
                }

                if ( (!session->H245Connection->h245Listen) && (reply != cmTransIgnoreMessage) )
                {
                    emaUnlock((EMAElement)session->H245Connection);
                    logPrint(transGlobals->hLog, RV_ERROR,
                             (transGlobals->hLog, RV_ERROR,
                             "sendStartH245Facility failed to initiate listen on H.245 TPKT"));
                    return;
                }
                else
                if (reply == cmTransIgnoreMessage)
                {
                    emaUnlock((EMAElement)session->H245Connection);
                    logPrint(transGlobals->hLog, RV_INFO,
                             (transGlobals->hLog, RV_INFO,
                             "sendStartH245Facility initiate listen on H.245 TPKT refused by user"));
                    return;
                }
                else
                {
                    /* mark that the h245 host is in listenning mode */
                    session->H245Connection->state = hostListenning;
                }
            }
            emaUnlock((EMAElement)session->H245Connection);
        }
        else
            return;

        /* send facility */
        CRV = session->CRV;
        if (!session->outgoing)
            CRV |= 0x8000;
        logPrint(transGlobals->hLog, RV_INFO,
                 (transGlobals->hLog, RV_INFO,
                 "sendStartH245Facility CRV = %x outgoing=%d", CRV, session->outgoing));
        if (createMessage(transGlobals,
                          cmQ931facility,
                          session->CRV,
                          session->callId,
                          &newMessage)
           )
        {
            int nodeId;
            /* set reason */
            __pvtBuildByFieldIds(nodeId, transGlobals->hPvt, newMessage,
                                    { _q931(message)
                                      _q931(facility)
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _q931(facility)
                                      _q931(reason)
                                      _q931(startH245)
                                     LAST_TOKEN
                                    },
                                 0, NULL);

            /* set address */
            __pvtBuildByFieldIds(nodeId, transGlobals->hPvt, newMessage,
                                    { _q931(message)
                                      _q931(facility)
                                      _q931(userUser)
                                      _q931(h323_UserInformation)
                                      _q931(h323_uu_pdu)
                                      _q931(h323_message_body)
                                      _q931(facility)
                                      _q931(h245Address)
                                     LAST_TOKEN
                                    },
                                    0, NULL);
            res = -1;
            if (nodeId >= 0)
            {
                res = cmTAToVt(transGlobals->hPvt,
                               nodeId,
                               &session->H245Connection->localAddress);
            }
            if (res < 0)
            {
                if (emaLock((EMAElement)session->H245Connection))
                {
                    tpktClose(session->H245Connection->h245Listen);
                    session->H245Connection->h245Listen = NULL;
                    emaUnlock((EMAElement)session->H245Connection);
                }
                pvtDelete(transGlobals->hPvt, newMessage);
                logPrint(transGlobals->hLog, RV_ERROR,
                         (transGlobals->hLog, RV_ERROR,
                         "sendStartH245Facility failed to create address in facility (startH245) "));
                return;
            }

            err = cmTransSendMessage((HSTRANSSESSION)session, newMessage, cmTransQ931Type);
            pvtDelete(transGlobals->hPvt, newMessage);
            if (err != cmTransOK)
            {
                if (emaLock((EMAElement)session->H245Connection))
                {
                    tpktClose(session->H245Connection->h245Listen);
                    session->H245Connection->h245Listen = NULL;
                    emaUnlock((EMAElement)session->H245Connection);
                }
                logPrint(transGlobals->hLog, RV_ERROR,
                        (transGlobals->hLog, RV_ERROR,
                        "sendStartH245Facility failed to send facility (startH245) message"));
                return;
            }
        }
        else
        {
            if (emaLock((EMAElement)session->H245Connection))
            {
                tpktClose(session->H245Connection->h245Listen);
                session->H245Connection->h245Listen = NULL;
                emaUnlock((EMAElement)session->H245Connection);
            }
            logPrint(transGlobals->hLog, RV_ERROR,
                    (transGlobals->hLog, RV_ERROR,
                    "sendStartH245Facility failed to create facility (startH245) message"));
            return;
        }
    }
}

/************************************************************************************
 * transCompareAddresses
 *
 * Purpose: compare to addresses according to their type.
 *
 *
 * Input: addr1, addr2 - teh cmTransportAddress to compare.
 *
 * Output: None.
 *
 * Returned Value: TRUE - the addresses are identical; FALSE - Otherwise
 **************************************************************************************/
BOOL transCompareAddresses(cmTransportAddress *addr1, cmTransportAddress *addr2)
{
    if (addr1->type == addr2->type)
    {
        switch (addr1->type)
        {
            case cmTransportTypeIP:
            {
                if ( (addr1->ip == addr2->ip) && (addr1->port == addr2->port) )
                    return TRUE;
                break;
            }

            default:
                break;
        }
    }

    return FALSE;
}

/**************************************************************************************
 * closeHostInternal
 *
 * Purpose: Deletes a host element. Will notify all its associates sessions.
 *
 * Input:   hsTranHost  - An handle to the host element
 *
 * Output:  None.
 *
 **************************************************************************************/
TRANSERR closeHostInternal(IN HSTRANSHOST hsTransHost, IN BOOL reportToUser)
{
    TRANSERR res = cmTransErr;
    cmTransHost *host = (cmTransHost *)hsTransHost;

    /* check if an element exists */
    if ( (hsTransHost) && emaLock((EMAElement)hsTransHost) )
    {
        /* retrieve the transport module global area */
        cmTransGlobals *transGlobals = (cmTransGlobals *)emaGetUserData((EMAElement)hsTransHost);

        if ( (transGlobals->hostEventHandlers.cmEvTransHostClosed) && (reportToUser) &&
              (host->reported) && (host->state != hostClosing) && (host->state != hostClosed) )
        {
            BOOL wasConnected       = ( (host->state == hostConnected) ||
                                        (host->state == hostBusy) );

            HATRANSHOST haTransHost = (HATRANSHOST)emaGetApplicationHandle((EMAElement)hsTransHost);
            int numOfLocks;

            logPrint(transGlobals->hLog, RV_INFO,
                (transGlobals->hLog, RV_INFO,
                "cmEvTransHostClosed(hsHost = %d(%x), haHost=%x, wasConnected = %d)",
            emaGetIndex((EMAElement)hsTransHost), hsTransHost, haTransHost, wasConnected));

            numOfLocks = emaPrepareForCallback((EMAElement)hsTransHost);
            transGlobals->hostEventHandlers.cmEvTransHostClosed((HSTRANSHOST)hsTransHost,
                                                                haTransHost,
                                                                wasConnected);
            emaReturnFromCallback((EMAElement)hsTransHost, numOfLocks);
            res = cmTransOK;
        }
        else
        {
            /* lock the host for this major change */
            if (host->state != hostClosed)
            {
                host->state = hostClosed;
                transHostClose(hsTransHost, TRUE);
            }
            res = cmTransOK;
        }
        emaUnlock((EMAElement)hsTransHost);
    }

    return res;
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/

