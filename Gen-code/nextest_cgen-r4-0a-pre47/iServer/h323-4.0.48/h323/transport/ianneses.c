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
#include "annexe.h"
#include "iannexe.h"
#include <hash.h>

tNode*
get_node( tAnnexE* pAnnexE, UINT32 ip, UINT16 port ) {

    tNodeKey    NodeKey;
    tNode*      pNode;
    void*       loc;

    NodeKey.nIP     = ip;
    NodeKey.nPort   = port;

    loc = hashFind(
          pAnnexE->ActiveNodesHash,
          &NodeKey);

    if (!loc)
        return NULL;

    pNode = *(tNode**)hashGetElement(
            pAnnexE->ActiveNodesHash,
            loc);

    return pNode;
}

tNode*
add_node( tAnnexE* pAnnexE, UINT32 ip, UINT16 port ) {

    tNodeKey    NodeKey;
    tNode*      pNode = NULL;

    RV_ASSERT( get_node( pAnnexE, ip, port ) == NULL );

    NodeKey.nIP     = ip;
    NodeKey.nPort   = port;

    if (!IsListEmpty( &pAnnexE->FreeNodesList )) {
        PLIST_ENTRY plink;

        plink   = RemoveHeadList( &pAnnexE->FreeNodesList );
        pNode   = CONTAINING_RECORD( plink, tNode, lNodeLink );

        pNode->nRef             = 1;
        pNode->RemoteHost.nIP   = ip;
        pNode->RemoteHost.nPort = port;
        pNode->nLast_Ack_SEQ    = 0xffffffff;
        pNode->nNext_SEQ        = timerGetTimeInMilliseconds() & 0xffffff;
        pNode->nRetry           = 0;
        pNode->fDontSend        = FALSE;


        RV_VERIFY(hashAdd(pAnnexE->ActiveNodesHash, &NodeKey, &pNode, FALSE));
    };

    return pNode;
}

void
del_node( tNode* pNode ) {
    tAnnexE*    pAnnexE = pNode->pAnnexE;
    void*       loc;

    RV_ASSERT( pNode == get_node( pNode->pAnnexE, pNode->RemoteHost.nIP, pNode->RemoteHost.nPort ) );

    pNode->nRef--;
    RV_ASSERT( pNode->nRef >= 0 );

    if (pNode->nRef > 0)
        return;

    stop_retransmit_or_ima_timer( pNode );

    if (pNode->pCurrentPDU != NULL) {
        AEHClr_A( pNode->pCurrentPDU->PDU ); /* !!!! */
        send_current_pdu( pNode );
    }

    if (pNode->pWaitingForAckPDU != NULL)
    {
        tPDU* pPDU = pNode->pWaitingForAckPDU;
        pNode->pWaitingForAckPDU = NULL;
        pNode->nRef++; /* protect node */
        free_pdu( pAnnexE, pPDU );
        pNode->nRef--; /* unprotect node */
    }

    RV_ASSERT( pNode->pCurrentPDU == NULL );
    RV_ASSERT( pNode->pWaitingForAckPDU == NULL );

    loc = hashFind(pAnnexE->ActiveNodesHash, &pNode->RemoteHost );
    if (loc)
        hashDelete(pAnnexE->ActiveNodesHash, loc);

    InsertTailList( &pNode->pAnnexE->FreeNodesList, &pNode->lNodeLink );
}

void* remove_one_node(HHASH hHash, void* elem, void *param) {
    tNode* pNode;
    pNode = *(tNode**)hashGetElement(hHash, elem);

    if ( (pNode) && (pNode->pAnnexE->events.AnnexEEvConnectionBroken != NULL)) {
        pNode->pAnnexE->events.AnnexEEvConnectionBroken(
                AsHANNEXE( pNode->pAnnexE ),
                pNode->pAnnexE->hAppAnnexE,
                pNode->RemoteHost.nIP,
                pNode->RemoteHost.nPort
            );
    }
    else
        del_node( pNode );

    return param;
}

void
del_all_nodes( tAnnexE* pAnnexE ) {
    hashDoAll(pAnnexE->ActiveNodesHash, remove_one_node, NULL);
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/
