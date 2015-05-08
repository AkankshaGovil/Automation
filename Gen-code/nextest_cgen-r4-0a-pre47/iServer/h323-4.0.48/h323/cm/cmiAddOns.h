/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CMIADDONS_H
#define CMIADDONS_H

#ifdef __cplusplus
extern "C" {
#endif

/* This file contains the internal API used between the H.323 stack and its addons */

#include <cm.h>


/************************************************************************************
 *
 * H235v2 Addon
 * This part handles the communication between the H235v2 addon and the stack.
 *
 ************************************************************************************/

typedef int (RVCALLCONV*cmEvCallNewRawMessageT)(
    IN  HAPP        hApp,
    IN  HPROTCONN   protConn,
    IN  int         nodeId,
    IN  BYTE*       messageBuf,
    IN  int         messageLength,
    OUT int*        bytesDecoded,
    OUT void**      hMsgContext);

typedef int (RVCALLCONV*cmEvCallSendRawMessageT)(
    IN  HAPP        hApp,
    IN  HPROTCONN   protConn,
    IN  HCALL       hsCall,
    IN  int         nodeId,
    IN  int         messageLength,
    OUT BYTE*       messageBuf,
    OUT int*        bytesEncoded);


typedef int (RVCALLCONV*cmEvCallReleaseMessageContextT)(
    IN  void*       hMsgContext);



typedef int (RVCALLCONV*cmiEvRASNewRawMessageT)(
    IN  HAPP        hApp,
    IN  HPROTCONN   protConn,
    IN  int         nodeId,
    IN  BYTE*       messageBuf,
    IN  int         messageLength,
    OUT int*        bytesDecoded,
    OUT void**      hMsgContext);

typedef int (RVCALLCONV*cmiEvRASSendRawMessageT)(
    IN  HAPP        hApp,
    IN  HPROTCONN   protConn,
    IN  HRAS        hsRas,
    IN  HAPPRAS     haRas,
    IN  int         nodeId,
    IN  int         messageLength,
    OUT BYTE*       messageBuf,
    OUT int*        bytesEncoded);


typedef int (RVCALLCONV*cmiEvRASReleaseMessageContextT)(
    IN  void*       hMsgContext);



RVAPI
int RVCALLCONV cmSetMessageEventHandler(
    IN HAPP hApp,
    IN cmEvCallNewRawMessageT   cmEvCallNewRawMessage,
    IN cmEvCallSendRawMessageT  cmEvCallSendRawMessage,
    IN cmEvCallReleaseMessageContextT
                                cmEvCallReleaseMessageContext);


/************************************************************************
 * cmiRASSetMessageEventHandler
 * purpose: Sets messages events for the use of the security module.
 *          These events allows the application later on to check on
 *          specific messages if they passed the security or not.
 * input  : hApp                    - Application's handle for a stack instance
 *          cmEviRASNewRawMessage   - Indication of a new incoming buffer for RAS
 *          cmEviRASSendRawMessage  - Indication of an outgoing RAS message
 *          cmEviRASReleaseMessageContext   - Indication that the RAS module
 *                                            is through with a specific message
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmiRASSetMessageEventHandler(
    IN HAPP                             hApp,
    IN cmiEvRASNewRawMessageT            cmiEvRASNewRawMessage,
    IN cmiEvRASSendRawMessageT           cmiEvRASSendRawMessage,
    IN cmiEvRASReleaseMessageContextT    cmiEvRASReleaseMessageContext);


RVAPI
int RVCALLCONV cmCallGetMessageContext(
    IN  HCALL            hCall,
    OUT void**           hMsgContext);


/************************************************************************
 * cmiRASGetMessageContext
 * purpose: Returns the message context for a RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : hMsgContext - Message context for the incoming request/response
 *                        message of the given transaction.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmiRASGetMessageContext(
    IN  HRAS             hsRas,
    OUT void**           hMsgContext);




#ifdef __cplusplus
}
#endif

#endif  /* CMIADDONS_H */
