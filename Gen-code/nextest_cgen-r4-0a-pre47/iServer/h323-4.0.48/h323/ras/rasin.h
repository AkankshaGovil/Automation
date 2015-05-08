
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASIN_H
#define RASIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



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
    IN int hashSize);


/************************************************************************
 * rasOutgoingHashCompare
 * purpose: Compare keys in the outgoing hash table
 * input  : key1, key2  - Keys to compare
 *          keySize     - Size of each key
 * return : TRUE if elements are the same
 *          FALSE otherwise
 ************************************************************************/
BOOL rasOutgoingHashCompare(IN void *key1, IN void* key2, IN UINT32 keySize);


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
 * output : none
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
    OUT void**          hMsgContext);


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
    IN rasInTx*     tx);


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
    IN cmRASReason  reason);


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
    IN BOOL         updateStopTime);


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
    IN rasInTx*         tx);


#ifdef __cplusplus
}
#endif

#endif  /* RASIN_H */

