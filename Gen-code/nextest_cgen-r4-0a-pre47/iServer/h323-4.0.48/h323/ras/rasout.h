
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASOUT_H
#define RASOUT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



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
    IN int hashSize);


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
    IN cmRASTransport*  destAddress);


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
    IN rasOutTx*        tx);


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
    IN rasOutTx*        tx);


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
    OUT void**          hMsgContext);


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
    OUT void**          hMsgContext);


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
    IN rasOutTx*        tx);



#ifdef __cplusplus
}
#endif

#endif  /* RASOUT_H */

