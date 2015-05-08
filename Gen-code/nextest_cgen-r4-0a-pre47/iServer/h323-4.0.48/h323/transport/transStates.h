#ifndef TRANSSTATES_H
#define TRANSSTATES_H

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

#include <transportint.h>


/**************************************************************************************
 * setMultiplexedParams
 *
 * putrpose: to set the multiplexed parameters into an outgoing message. The parameters
 *           are taken from the host parameters which are constatntly updated by incoming
 *           messages and user API calls.
 *
 * Input: transGlobals - The global data of the module.
 *        session      - the session from which the message is sent.
 *        host         - the host on which the message is to be sent.
 *        pvtNode      - the message into which the parameters are to be set.
 *        msgType      - which message we are dealing with.
 *
 ***************************************************************************************/
void setMultiplexedParams(cmTransGlobals *transGlobals,
                          cmTransSession *session,
                          cmTransHost *host,
                          int pvtNode,
                          int msgType);

/**************************************************************************************
 * setTheFastStartStateByMessage
 *
 * putrpose: to determine the faststart state of the session according to its previous
 *           state and the data within the outgoing message. This routine handles all
 *           outgoing messages (setup for outgoing calls and all the rest for incoming
 *           calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void setTheFastStartStateByMessage(cmTransGlobals *transGlobals,
                                   cmTransSession *session,
                                   int            messageBodyNode,
                                   int            msgType);

/**************************************************************************************
 * setTheTunnelingStateByMessage
 *
 * putrpose: to determine the tunneling state of the session according to its previous
 *           state and the data within the outgoing message. This routine handles all
 *           outgoing messages (setup for outgoing calls and all the rest for incoming
 *           calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void setTheTunnelingStateByMessage(cmTransGlobals *transGlobals,
                                   cmTransSession *session,
                                   int            messageBodyNode,
                                   int            msgType);

/**************************************************************************************
 * setTheParallelTunnelingStateByMessage
 *
 * putrpose: to determine the parallel tunneling state of the session according to its previous
 *           state and the faststart and tunneling states of the session.
 *           This routine handles all outgoing messages (setup for outgoing calls and all
 *           the rest for incoming calls. The routine modifies the session state variables!
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgType           - The type of the message (setup or other)
 *        msgBody           - the node to the message UU-IE part
 *        outgoing          - TRUE: an outgoing message; FALSE - an incoming message.
 *
 ***************************************************************************************/
void setTheParallelTunnelingStateByMessage(cmTransGlobals *transGlobals,
                                           cmTransSession *session,
                                           int            msgType,
                                           int            msgBody,
                                           BOOL           outgoing);

/**************************************************************************************
 * needToOpenH245
 *
 * putrpose: to determine according to the session params whether
 *           we shall want to open an H.245 (actually doing the connect).
 * Input: session           - the session on which the message is to be sent.
 *        forceOpen         - we must open (due to a startH245 facility).
 *
 ***************************************************************************************/
BOOL needToOpenH245(cmTransSession *session, BOOL forceOpen);


/**************************************************************************************
 * determineIfToOpenH245
 *
 * putrpose: This routine checks if it's time to start a H.245 connection either by listenning
 *           if we have a H.245 address and it's time according to the H.245 stage to send
 *           that address. If so, the address is sent and the module starts listenning on
 *           that address. Or by trying to connect, if we have the remotes address.
 *
 * Input: outgoing          - TRUE - if an outgoing message, FALSE - if incoming one.
 *        transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        messageBodyNode   - the UU-IE part of the message to be inspected.
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
BOOL determineIfToOpenH245(BOOL           outgoing,
                           cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType);

/**************************************************************************************
 * reportH245
 *
 * putrpose: This routine checks if it is necessary to report to the user that an H.245 is opened.
 *           It bases its decision accrding to the state of the session in regard to tunneling
 *           and parallel tunneling and to the faststart state of the session as well.
 *           If the connection was already reported, we eliminate the notification.
 *           The routine handles both incoming and outgoing messages.
 *
 * Input: outgoing          - TRUE - if an outgoing message, FALSE - if incoming one.
 *        transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        host              - the host on which the message arrived
 *        messageBodyNode   - The body of the message (incoming or outgoing)
 *        msgType           - The type of the message (setup or other)
 *
 ***************************************************************************************/
void reportH245(BOOL            outgoing,
                cmTransGlobals  *transGlobals,
                cmTransSession  *session,
                cmTransHost     *host,
                int             messageBodyNode,
                int             msgType);

/**************************************************************************************
 * insertH245TunnelingFlag
 *
 * putrpose: This routine fills the H.245 tunneling flag 'h245Tunneling' into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *        msgType           - The message type into which the tunneled messages are inserted
 *
 ***************************************************************************************/
void insertH245TunnelingFlag(cmTransGlobals  *transGlobals,
                             cmTransSession  *session,
                             int             messageBodyNode,
                             int             msgType);

/**************************************************************************************
 * insertH245TunneledMessages
 *
 * putrpose: This routine fills the H.245 tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *        msgType           - The message type into which the tunneled messages are inserted
 *
 ***************************************************************************************/
void insertH245TunneledMessages(cmTransGlobals  *transGlobals,
                                cmTransSession  *session,
                                int             messageBodyNode,
                                int             msgType);

/**************************************************************************************
 * insertH450TunneledMessages
 *
 * putrpose: This routine fills the H.450 tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertH450TunneledMessages(cmTransGlobals  *transGlobals,
                                cmTransSession  *session,
                                int             messageBodyNode);

/**************************************************************************************
 * insertAnnexLTunneledMessages
 *
 * putrpose: This routine fills the annex L tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertAnnexLTunneledMessages(cmTransGlobals    *transGlobals,
                                  cmTransSession    *session,
                                  int               messageBodyNode);

/**************************************************************************************
 * insertAnnexMTunneledMessages
 *
 * putrpose: This routine fills the annex M tunneled messages into the outgoing Q.931 message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message is to be sent.
 *        msgBodyNode       - The messages UU-IE part.
 *
 ***************************************************************************************/
void insertAnnexMTunneledMessages(cmTransGlobals    *transGlobals,
                                  cmTransSession    *session,
                                  int               messageBodyNode);

/**************************************************************************************
 * getMultiplexedParams
 *
 * putrpose: to get the multiplexed parameters from an incoming message. The parameters
 *           are taken from the message and set into the host parameters which are constatntly
 *           updated by the incoming messages and user API calls.
 *
 * Input: transGlobals - The global data of the module.
 *        host         - the host on which the message was received.
 *        pvtNode      - the message from which the parameters are to be get.
 *        msgType      - The type of the message.
 ***************************************************************************************/
 void getMultiplexedParams(cmTransGlobals *transGlobals, cmTransHost *host, int pvtNode, int msgType);

/**************************************************************************************
 * extractH245Messages
 *
 * putrpose: extracts, decodes and reports the tunneled H.245 messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        host              - the host on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractH245Messages(cmTransGlobals *transGlobals,
                         cmTransSession *session,
                         cmTransHost    *host,
                         int            messageBodyNode,
                         int            msgType);

/**************************************************************************************
 * extractH450Messages
 *
 * putrpose: extracts and reports the tunneled H.450 messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        pvtNode           - The Q.931 messgae.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractH450Messages(cmTransGlobals *transGlobals,
                         cmTransSession *session,
                         int            pvtNode,
                         int            msgType);


/**************************************************************************************
 * extractAnnexMMessages
 *
 * putrpose: extracts and reports the tunneled Annex M messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractAnnexMMessages(cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType);

/**************************************************************************************
 * extractAnnexLMessages
 *
 * putrpose: extracts and reports the tunneled Annex L messages from the given message.
 *
 * Input: transGlobals      - The global data of the module.
 *        session           - the session on which the message was received.
 *        messageBodyNode   - the UU-IE part of the message.
 *        msgType           - The type of the message from which we extract the tunneled msgs.
 ***************************************************************************************/
void extractAnnexLMessages(cmTransGlobals *transGlobals,
                           cmTransSession *session,
                           int            messageBodyNode,
                           int            msgType);



#ifdef __cplusplus
}
#endif


#endif
