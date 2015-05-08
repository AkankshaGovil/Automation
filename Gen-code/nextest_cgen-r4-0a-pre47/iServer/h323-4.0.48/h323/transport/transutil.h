#ifndef TRANSUTIL_H
#define TRANSUTIL_H

/******************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

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
                   int               *node);

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
                            IN BOOL                 isAnnexE);

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
                             int            *encMsgSize);

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
                             IN cmTransSession *session);

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
void removeSessionFromPendingList(IN cmTransGlobals *transGlobals,
                                  IN cmTransSession *session);


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
 void *saveMessageToPool(cmTransGlobals *transGlobals,
                            void            *element,
                            BYTE            *buffer,
                            int             encMsgSize,
                            BOOL            isTunneled,
                            BOOL            addToTop,
                            UINT16          CRV);

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
                             BOOL           isTunneled);

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
                                INOUT int               pvtNode);

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
                            IN    TRANSTYPE         type);

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
void transSessionClose(cmTransSession *session);

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
void transHostClose(HSTRANSHOST hsTransHost, BOOL killHost);

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
 * returned value: TRUE - an error occured, FALSE - all is OK (that doesn't mean that we
 *                                                  have a session ).
 *
 **************************************************************************************/
BOOL findSessionByMessage(cmTransGlobals *transGlobals,
                          cmTransHost    *host,
                          int            pvtNode,
                          BOOL           isAnnexE,
                          cmTransSession **session);

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
TRANSERR setRemoteAddress(cmTransHost *host, cmTransportAddress *remoteAddress);

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
BOOL canWeAcceptTheCall(cmTransGlobals *transGlobals, cmTransHost *host, int pvtNode);

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
BOOL encodeAndSend(cmTransGlobals *transGlobals, cmTransHost *host, int newMessage);

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
void sendGeneralFacility(cmTransGlobals *transGlobals, cmTransHost *host);

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
                               cmTransHost    *host);

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
                                INTPTR          reasonNameId);

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
                    cmTransportAddress *addr);

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
void sendStartH245Facility(cmTransGlobals *transGlobals, cmTransSession *session);

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
BOOL transCompareAddresses(cmTransportAddress *addr1, cmTransportAddress *addr2);

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
TRANSERR closeHostInternal(IN HSTRANSHOST hsTransHost, IN BOOL reportToUser);


#ifdef __cplusplus
}
#endif


#endif
