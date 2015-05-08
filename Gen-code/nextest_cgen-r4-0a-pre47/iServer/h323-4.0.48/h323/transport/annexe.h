/******************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

******************************************************************************/

/*
    AnnexE.h

    Authors:
        Assen Stoyanov (softa@biscom.net)
        Kalin Bogatzevski (kalin@biscom.net)

    Created:
        10 May 2000

    Overview:

    Revised:

    Bugs:

    ToDo:
*/

#ifndef _AnnexE_H_
#define _AnnexE_H_

#ifdef __cplusplus
extern "C" {
#endif /* _cplusplus */

#include <rvinternal.h>
#include <mti.h>
#include <mei.h>
#include <log.h>


DECLARE_OPAQUE( HANNEXE );              /* Annex E instance handle                      */
DECLARE_OPAQUE( HAPPANNEXE );           /* Application handle of the Annex E instance   */

typedef enum {
    annexEStatusNormal,
    annexEStatusMemoryProblem,
    annexEStatusResourceProblem,
    annexEStatusSocketProblem,
    annexEStatusWouldBlock,
    annexEStatusAddressAlreadyInUse,
    annexEStatusMustBeStopped,
    annexEStatusMustBeStarted,
    annexEStatusBadParameter
} annexEStatus;

/* Event Handler Function Prototypes */
typedef annexEStatus (RVCALLCONV* annexEEvRestartT)(
    IN  HANNEXE                 hAnnexE,
    IN  HAPPANNEXE              hAppAnnexE,
    IN  cmTransportAddress*     pSourceAddress
);

typedef annexEStatus (RVCALLCONV* annexEEvNewMessageT)(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort,
    IN  void*       pMessage,
    IN  int         nSize
);

typedef annexEStatus (RVCALLCONV* annexEEvConnectionBrokenT)(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort
);

typedef annexEStatus (RVCALLCONV* annexEEvUseOtherAddressT)(
    IN  HANNEXE                 hAnnexE,
    IN  HAPPANNEXE              hAppAnnexE,
    IN  UINT32                  nIP,
    IN  UINT16                  nPort,
    IN  cmTransportAddress*     pNewAddress
);

typedef annexEStatus (RVCALLCONV* annexEEvNotSupportedT)(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort
);

typedef annexEStatus (RVCALLCONV* annexEEvWriteableT)(
    IN  HANNEXE     hAnnexE,
    IN  HAPPANNEXE  hAppAnnexE,
    IN  UINT32      nIP,
    IN  UINT16      nPort
);

/* Callback Structure Definition */
typedef struct {
    annexEEvRestartT            AnnexEEvRestart;
    annexEEvNewMessageT         AnnexEEvNewMessage;
    annexEEvConnectionBrokenT   AnnexEEvConnectionBroken;
    annexEEvUseOtherAddressT    AnnexEEvUseOtherAddress;
    annexEEvNotSupportedT       AnnexEEvNotSupported;
    annexEEvWriteableT          AnnexEEvWriteable;
} annexEEvents;

/* package init & done functions
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexEInit(
    OUT HANNEXE*                hAnnexE,
    IN  HAPPANNEXE              hAppAnnexE,
    IN  int                     nMaxSessions,
    IN  int                     nMaxSupportedMessageSize,
    IN  int                     nMaxStoredPackets,
	IN  HSTIMER					timersPool,
    IN  RVHLOGMGR               cmLogMgr
);

RVAPI annexEStatus RVCALLCONV
annexEEnd(
    IN  HANNEXE                 hAnnexE
);

/* event handler & protocol parameter functions
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexESetProtocolParams(
    IN  HANNEXE                 hAnnexE,
    IN  int                     t_R1,
    IN  int                     t_R2,
    IN  int                     n_R1,
    IN  int                     t_IMA1,
    IN  int                     n_IMA1,
    IN  int                     t_DT
);

RVAPI annexEStatus RVCALLCONV
annexESetLocalAddress(
    IN  HANNEXE                 hAnnexE,
    IN  cmTransportAddress*     pLocalAddress
);

RVAPI annexEStatus RVCALLCONV
annexEGetLocalAddress(
    IN  HANNEXE                 hAnnexE,
    IN  cmTransportAddress*     pLocalAddress
);

RVAPI annexEStatus RVCALLCONV
annexEGetSocketHandle(
    IN  HANNEXE                 hAnnexE,
    OUT UINT32*                 pnSocketHandle
);

RVAPI annexEStatus RVCALLCONV
annexESetEventHandler(
    IN  HANNEXE                 hAnnexE,
    IN  annexEEvents*           pAnnexEEventHandler
);

/* package start & stop functions
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexEStart(
    IN  HANNEXE                 hAnnexE
);

RVAPI annexEStatus RVCALLCONV
annexEStop(
    IN  HANNEXE                 hAnnexE
);

/* package state functions
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexEGetHAStateSize(
    IN  HANNEXE                 hAnnexE,
    OUT UINT32*                 pnSize
);

RVAPI annexEStatus RVCALLCONV
annexEGetHAState(
    IN  HANNEXE                 hAnnexE,
    OUT void*                   pState,
    IN  int                     nSize
);

RVAPI annexEStatus RVCALLCONV
annexESetHAState(
    IN  HANNEXE                 hAnnexE,
    IN  void*                   pState,
    IN  int                     nSize
);

/* send 'Restart' message
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexERestart(
    IN  HANNEXE                 hAnnexE,
    IN  cmTransportAddress*     pDestAddress
);

/* AnnexE API functions
****************************************************************************
***************************************************************************/


RVAPI annexEStatus RVCALLCONV
annexESendMessage(
    IN  HANNEXE                 hAnnexE,
    IN  UINT32                  ip,
    IN  UINT16                  port,
    IN  UINT16                  CRV,
    IN  void*                   message,
    IN  int                     size,
    IN  BOOL                    fAckHint,
    IN  BOOL                    fReplyHint,
    IN  BOOL                    fSendHint
);

RVAPI annexEStatus RVCALLCONV
annexECloseNode(
    IN  HANNEXE                 hAnnexE,
    IN  UINT32                  ip,
    IN  UINT16                  port
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AnnexE_H_ */
