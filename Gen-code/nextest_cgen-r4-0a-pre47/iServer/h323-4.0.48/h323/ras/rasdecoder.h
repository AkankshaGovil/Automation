
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef RASDECODER_H
#define RASDECODER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <cmiras.h>
#include <rasdef.h>



/************************************************************************
 * rasDecoderInit
 * purpose: Initialize the decoder. Checks out the information inside
 *          the PST tree to allow faster runtime decoding of the first
 *          2 fields.
 * input  : ras             - RAS instance to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecoderInit(IN  rasModule*  ras);


/************************************************************************
 * rasDecoderEnd
 * purpose: Deinitialize the decoder.
 * input  : ras             - RAS instance to use
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecoderEnd(IN  rasModule*  ras);






/************************************************************************
 * rasDecodePart
 * purpose: Decode the first 2 fields of an incoming message. This way
 *          We know the message's type and its sequence number.
 * input  : ras             - RAS instance to use
 *          messageBuffer   - The encoded message to check
 *          messageSize     - The size of the message in bytes
 * output : index           - Index of the RAS message
 *          requestSeqNum   - Request sequence number of the message
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasDecodePart(
    IN  rasModule*  ras,
    IN  BYTE*       messageBuffer,
    IN  UINT32      messageSize,
    OUT UINT32*     index,
    OUT UINT32*     requestSeqNum);





#ifdef __cplusplus
}
#endif

#endif  /* RASDECODER_H */

