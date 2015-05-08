
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASIRR_H
#define RASIRR_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



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
    OUT void**          hMsgContext);


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
    IN  HCALL           hsCall);


#ifdef __cplusplus
}
#endif

#endif  /* RASIRR_H */

