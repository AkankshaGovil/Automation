
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASUTILS_H
#define RASUTILS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



/************************************************************************
 * rasGetOutgoing
 * purpose: Get the outgoing RAS transaction from its handle
 * input  : hsRas   - Application handle
 * output : none
 * return : RAS outgoing transaction on success
 *          NULL on failure
 ************************************************************************/
#ifdef RV_RAS_DEBUG
rasOutTx* rasGetOutgoing(IN HRAS hsRas);
#else
#define rasGetOutgoing(hsRas)   ((rasOutTx*)hsRas)
#endif


/************************************************************************
 * rasGetIncoming
 * purpose: Get the incoming RAS transaction from its handle
 * input  : hsRas   - Application handle
 * output : none
 * return : RAS incoming transaction on success
 *          NULL on failure
 ************************************************************************/
#ifdef RV_RAS_DEBUG
rasInTx* rasGetIncoming(IN HRAS hsRas);
#else
#define rasGetIncoming(hsRas)   ((rasInTx*)hsRas)
#endif


/************************************************************************
 * rasGetParamName
 * purpose: Get the parameter's name
 * input  : param   - Parameter enumeration value
 * output : none
 * return : Parameter's name on success
 *          NULL on failure
 ************************************************************************/
char *rasGetParamName(IN cmRASParam param);


/************************************************************************
 * rasCfgGetTimeout
 * purpose: Get the timeout of a transaction before a retry is sent from
 *          the configuration
 * input  : ras     - RAS instance handle
 * output : none
 * return : Timeout in milliseconds
 ************************************************************************/
int rasCfgGetTimeout(IN rasModule* ras);


/************************************************************************
 * rasCfgGetRetries
 * purpose: Get the number of retries for a transaction before a timeout
 *          from the configuration
 * input  : ras     - RAS instance handle
 * output : none
 * return : Number of retries
 ************************************************************************/
int rasCfgGetRetries(IN rasModule* ras);


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
    IN  const char*         typeStr);


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
    OUT void**              rpoolHandle);


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
    OUT int*                nodeId,
    OUT rasMessages*        messageType,
    OUT UINT32*             requestSeqNum,
    OUT void**              hMsgContext);


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
 * output : none
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
    OUT void**              hMsgContext);


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
    OUT catStruct*  catKey);


/************************************************************************
 * rasUpdateRegInfo
 * purpose: Update the registration information of our RAS configuration
 *          from an incoming RCF message
 * input  : ras             - RAS instance handle
 *          messageNodeId   - Incoming message that caused this call
 *                            For unregistration, this value will be negative
 * output : catKey  - Filled CAT key struct for this transaction
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
void rasUpdateRegInfo(
    IN  rasModule*  ras,
    IN  int         messageNodeId);




#ifdef __cplusplus
}
#endif

#endif  /* RASUTILS_H */

