#ifdef __cplusplus
extern "C" {
#endif


/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#include <rvinternal.h>
#include <hash.h>
#include "annexe.h"
#include "iannexe.h"

void free_resources( tAnnexE* pAnnexE );

/* package init & done functions
 ****************************************************************************
 ***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexEInit(
    OUT HANNEXE*                hAnnexE,
    IN  HAPPANNEXE              hAppAnnexE,
    IN  int                     nMaxNodes,
    IN  int                     nMaxSupportedMessageSize,
    IN  int                     nMaxStoredPackets,
	IN  HSTIMER					timersPool,
    IN  RVHLOGMGR               cmLogMgr
) {

    tAnnexE*    pAnnexE;
    int         i;

    if (hAnnexE == NULL) {
        return annexEStatusBadParameter;
    }

    *hAnnexE = NULL;

    /* allocate annex E context */
    pAnnexE = (tAnnexE*)malloc( sizeof( tAnnexE ) );
    if (pAnnexE == NULL) {
        return annexEStatusMemoryProblem;
    }
    memset( pAnnexE, 0, sizeof( tAnnexE ) );

    /* initialize tAnnexE */
    pAnnexE->State = NOT_INITIALIZED;

    pAnnexE->msaType = msaRegister(0, "AnnexE", "H323 AnnexE module" );

    msaPrintFormat( pAnnexE->msaType, "annexEInit( MaxSes:%i, MaxMsgSize:%i, MaxPDU's:%i ) Begin", nMaxNodes, nMaxSupportedMessageSize, nMaxStoredPackets );

    InitializeListHead( &pAnnexE->FreeNodesList );
    InitializeListHead( &pAnnexE->FreePDUList );
    InitializeListHead( &pAnnexE->WaitingForTransmissionList );
    InitializeListHead( &pAnnexE->ResBlockedNodeList );

    pAnnexE->t_R1   = ANNEXE_TR1;
    pAnnexE->t_R2   = ANNEXE_TR1;
    pAnnexE->n_R1   = ANNEXE_NR1;
    pAnnexE->t_IMA1 = ANNEXE_TIMA1;
    pAnnexE->n_IMA1 = ANNEXE_NIMA1;
    pAnnexE->t_DT   = ANNEXE_TR1 / 4;

    pAnnexE->ActiveNodesHash = hashConstruct(nMaxNodes + 1,
                                             nMaxNodes,
                                             hashstr,
                                             hashDefaultCompare,
                                             sizeof_tNodeKey,
                                             sizeof(tNode*),
                                             cmLogMgr,
                                             "Annex E nodes header");

    if (pAnnexE->ActiveNodesHash == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexEInit() - MemoryProblem (ActiveNodes hash)!" );
        msaUnregister( pAnnexE->msaType );
        free( pAnnexE );
        return annexEStatusMemoryProblem;
    }

    pAnnexE->hAppAnnexE = hAppAnnexE;

    /* initialize tNode's */
    for (i = 0; i < nMaxNodes; i++) {
        tNode*      pNode;

        /* allocate tNode */
        pNode = (tNode*)malloc( sizeof( tNode ) );
        if (pNode == NULL) {
            msaPrintFormat( pAnnexE->msaType, "annexEInit() - MemoryProblem (nodes)!" );
            free_resources( pAnnexE );
            return annexEStatusMemoryProblem;
        }

        /* initialize tNode */
        memset( pNode, 0, sizeof( tNode ) );

        pNode->pAnnexE = pAnnexE;

        pNode->hResendAndIMATimer = EMPTYHTI;

        /* append tNode to FreeNodesList */
        InsertTailList( &pAnnexE->FreeNodesList, &pNode->lNodeLink );
    }

    /* initialize PDU's */
    for (i = 0; i < nMaxStoredPackets; i++) {
        tPDU*       pPDU;
        int         nPDUmemsize = sizeof( tPDU ) - sizeof_IAnnexEHeader + nMaxSupportedMessageSize;

        /* allocate tPDU;*/
        pPDU = (tPDU*)malloc( nPDUmemsize );
        if (pPDU == NULL) {
            msaPrintFormat( pAnnexE->msaType, "annexEInit() - MemoryProblem (send PDU's)!" );
            free_resources( pAnnexE );
            return annexEStatusMemoryProblem;
        }

        /* initialize tPDU;*/
        memset( pPDU, 0, nPDUmemsize );
        pPDU->nMaxSize = nMaxSupportedMessageSize;

        /* append tPDU to FreePDUList   */
        InsertTailList( &pAnnexE->FreePDUList, &pPDU->lPDULink );
    }

    /* initialize pRecvPDU! */
    pAnnexE->pRecvPDU = (tPDU*)malloc( sizeof( tPDU ) - sizeof_IAnnexEHeader + nMaxSupportedMessageSize/*MAX_UDP_FRAME*/ );
    if (pAnnexE->pRecvPDU == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexEInit() - MemoryProblem (recv PDU)!" );
        free_resources( pAnnexE );
        return annexEStatusMemoryProblem;
    }
    pAnnexE->pRecvPDU->nMaxSize = nMaxSupportedMessageSize /*MAX_UDP_FRAME*/;

    /* initialize Timers    */
	pAnnexE->hTimers = timersPool;

    if (pAnnexE->hTimers == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexEInit() - MemoryProblem!" );
        free_resources( pAnnexE );
        return annexEStatusMemoryProblem;
    }

    /* initialize Sockets */
    if (liInit() < 0) {
        msaPrintFormat( pAnnexE->msaType, "annexEInit() - liInit() Problem!" );
        free_resources( pAnnexE );
        return annexEStatusMemoryProblem;
    }

    pAnnexE->State = INITIALIZED;

    *hAnnexE = AsHANNEXE( pAnnexE );

    msaPrintFormat( pAnnexE->msaType, "annexEInit() End! - successful!" );

    return annexEStatusNormal;
}

void
free_resources( tAnnexE* pAnnexE ) {
    /* free pRecvPDU    */
    if (pAnnexE->pRecvPDU)
        free( pAnnexE->pRecvPDU );

    /* free PDUList */
    while (!IsListEmpty( &pAnnexE->FreePDUList )) {
        tPDU*       pPDU;
        PLIST_ENTRY plink;

        plink   = RemoveHeadList( &pAnnexE->FreePDUList );
        pPDU    = CONTAINING_RECORD( plink, tPDU, lPDULink );

        free( pPDU );
    }

    /* free Nodes   */
    while (!IsListEmpty( &pAnnexE->FreeNodesList )) {
        tNode*  pNode;
        PLIST_ENTRY plink;

        plink   = RemoveHeadList( &pAnnexE->FreeNodesList );
        pNode   = CONTAINING_RECORD( plink, tNode, lNodeLink );

        free( pNode );
    }

    if (pAnnexE->ActiveNodesHash != NULL) {
        hashDestruct( pAnnexE->ActiveNodesHash );
    }

    msaUnregister( pAnnexE->msaType );

    /* free AnnexE  */
    free( pAnnexE );
}

RVAPI annexEStatus RVCALLCONV
annexEEnd(
    IN  HANNEXE                 hAnnexE
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    annexEStop( hAnnexE );
    liEnd();
    pAnnexE->State = NOT_INITIALIZED;

    msaPrintFormat( pAnnexE->msaType, "annexEEnd() - successful!" );

    free_resources( pAnnexE );

    return annexEStatusNormal;
}

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
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    if (pAnnexE->State != INITIALIZED) {
        msaPrintFormat( pAnnexE->msaType, "annexESetProtocolParams() - annexEStatusMustBeStopped!" );
        return annexEStatusMustBeStopped;
    }

    pAnnexE->t_R1   = (t_R1 > 0) ? t_R1 : ANNEXE_TR1;
    pAnnexE->t_R2   = (t_R2 > 0) ? t_R2 : pAnnexE->t_R1;
    pAnnexE->n_R1   = (n_R1 > 0) ? n_R1 : ANNEXE_NR1;
    pAnnexE->t_IMA1 = (t_IMA1 > 0) ? t_IMA1 : ANNEXE_TIMA1;
    pAnnexE->n_IMA1 = (n_IMA1 > 0) ? n_IMA1 : ANNEXE_NIMA1;
    pAnnexE->t_DT   = (t_DT > 0) ? t_DT : (pAnnexE->t_R1 / 4);

    msaPrintFormat( pAnnexE->msaType, "annexESetProtocolParams( tR1=%i, tR2=%i, nR1=%i, tIMA1=%i, nIMA1=%i, tDT=%i ) - successful!",
            pAnnexE->t_R1,
            pAnnexE->t_R2,
            pAnnexE->n_R1,
            pAnnexE->t_IMA1,
            pAnnexE->n_IMA1,
            pAnnexE->t_DT
        );

    return annexEStatusNormal;
}

RVAPI annexEStatus RVCALLCONV
annexESetLocalAddress(
    IN  HANNEXE                 hAnnexE,
    IN  cmTransportAddress*     pLocalAddress
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    if (pAnnexE->State != INITIALIZED) {
        msaPrintFormat( pAnnexE->msaType, "annexESetLocalAddress() - annexEStatusMustBeStopped!" );
        return annexEStatusMustBeStopped;
    }

    if (pLocalAddress == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexESetLocalAddress() - annexEStatusBadParameter(pLocalAddress == NULL)!" );
        return annexEStatusBadParameter;
    }

    if ((pLocalAddress->distribution != cmDistributionUnicast) ||
        (pLocalAddress->type != cmTransportTypeIP)) {
        msaPrintFormat( pAnnexE->msaType, "annexESetLocalAddress() - annexEStatusBadParameter(distribution or type error)!" );
        return annexEStatusBadParameter;
    }

    pAnnexE->LocalIP    = pLocalAddress->ip;
    pAnnexE->LocalPort  = pLocalAddress->port;

    msaPrintFormat( pAnnexE->msaType, "annexESetLocalAddress( IP:%08x, Port:%i ) - successful!",
            pAnnexE->LocalIP,
            pAnnexE->LocalPort
        );

    return annexEStatusNormal;
}

RVAPI annexEStatus RVCALLCONV
annexEGetLocalAddress(
    IN  HANNEXE                 hAnnexE,
    IN  cmTransportAddress*     pLocalAddress
) {
    tAnnexE* pAnnexE = AsAnnexE( hAnnexE );

    if (pLocalAddress == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexEGetLocalAddress() - annexEStatusBadParameter(pLocalAddress == NULL)!" );
        return annexEStatusBadParameter;
    }

    pLocalAddress->distribution = cmDistributionUnicast;
    pLocalAddress->type         = cmTransportTypeIP;
    pLocalAddress->ip           = pAnnexE->LocalIP;
    pLocalAddress->port         = pAnnexE->LocalPort;

    return annexEStatusNormal;
}

RVAPI annexEStatus RVCALLCONV
annexEGetSocketHandle(
    IN  HANNEXE                 hAnnexE,
    OUT UINT32*                 pnSocketHandle
) {
    tAnnexE* pAnnexE = AsAnnexE( hAnnexE );

    if (pnSocketHandle == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexEGetSocketHandle() - annexEStatusBadParameter(pnSocketHandle == NULL)!" );
        return annexEStatusBadParameter;
    }
    if (pAnnexE->State != RUNNING) {
        msaPrintFormat( pAnnexE->msaType, "annexEGetSocketHandle() - annexEStatusMustBeStarted!" );
        return annexEStatusMustBeStarted;
    }

    *pnSocketHandle = (UINT32)pAnnexE->sock;

    return annexEStatusNormal;
}

RVAPI annexEStatus RVCALLCONV
annexESetEventHandler(
    IN  HANNEXE             hAnnexE,
    IN  annexEEvents*       pAnnexEEventHandler
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    if (pAnnexE->State != INITIALIZED) {
        msaPrintFormat( pAnnexE->msaType, "annexESetEventHandler() - annexEStatusMustBeStopped!" );
        return annexEStatusMustBeStopped;
    }

    if (pAnnexEEventHandler == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexESetEventHandler() - annexEStatusBadParameter(pAnnexEEventHandler == NULL)!" );
        return annexEStatusBadParameter;
    }

    pAnnexE->events = *pAnnexEEventHandler;

    return annexEStatusNormal;
}

/* start & stop functions
****************************************************************************
***************************************************************************/

RVAPI annexEStatus RVCALLCONV
annexEStart(
    IN  HANNEXE                 hAnnexE
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    if (pAnnexE->State != INITIALIZED) {
        msaPrintFormat( pAnnexE->msaType, "annexEStart() - annexEStatusMustBeStopped!" );
        return annexEStatusMustBeStopped;
    }

    pAnnexE->sock = liOpen( pAnnexE->LocalIP, pAnnexE->LocalPort, LI_UDP );
    if (pAnnexE->sock < 0) {
        return annexEStatusSocketProblem;
    }

    pAnnexE->LocalIP    = liGetSockIP( pAnnexE->sock );
    pAnnexE->LocalPort  = liGetSockPort( pAnnexE->sock );

    pAnnexE->State = RUNNING;

    liCallOn( pAnnexE->sock, liEvRead, EvRead, pAnnexE );

    pAnnexE->fReadyToSend = TRUE;

    msaPrintFormat( pAnnexE->msaType, "annexEStart() - successfuly bind on IP:%08x, Port:%i", pAnnexE->LocalIP, pAnnexE->LocalPort );

    return annexEStatusNormal;
}

RVAPI annexEStatus RVCALLCONV
annexEStop(
    IN  HANNEXE                 hAnnexE
) {

    tAnnexE*    pAnnexE = AsAnnexE( hAnnexE );

    if (pAnnexE->State != RUNNING) {
        msaPrintFormat( pAnnexE->msaType, "annexEStop() - annexEStatusMustBeStarted!" );
        return annexEStatusMustBeStarted;
    }

    liClose( pAnnexE->sock );

    del_all_nodes( pAnnexE );

    /* clear WaitingForTransmissionList! */
    {
        PLIST_ENTRY plink;
        tPDU*       pPDU;
        while (!IsListEmpty( &pAnnexE->WaitingForTransmissionList )) {
            plink = RemoveHeadList( &pAnnexE->WaitingForTransmissionList );
            pPDU = CONTAINING_RECORD( plink, tPDU, lPDULink );
            pPDU->fPDUIsWaitingForSend = FALSE;
            pPDU->nSize = 0;
            InsertTailList( &pAnnexE->FreePDUList, &pPDU->lPDULink );
        }
    }

    pAnnexE->State = INITIALIZED;

    msaPrintFormat( pAnnexE->msaType, "annexEStop() - successful!" );

    return annexEStatusNormal;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
