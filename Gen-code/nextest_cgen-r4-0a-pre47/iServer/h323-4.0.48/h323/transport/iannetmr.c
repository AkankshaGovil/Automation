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

UINT32 get_delay_interval( tNode* pNode ) {
    return pNode->pAnnexE->t_DT;
}

UINT32 get_retransmit_interval( tNode* pNode ) {
    tAnnexE* pAnnexE = pNode->pAnnexE;
    return pAnnexE->t_R1 + (pAnnexE->t_R2 - pAnnexE->t_R1) * pNode->nRetry;
}

UINT32 get_ima_interval( tNode* pNode ) {
    return pNode->pAnnexE->t_IMA1;
}

void start_retransmit_or_ima_timer( tNode* pNode ) {
    if (pNode->hResendAndIMATimer != EMPTYHTI) {
        mtimerReset( pNode->pAnnexE->hTimers, pNode->hResendAndIMATimer );
    }
    pNode->hResendAndIMATimer = mtimerSet(
            pNode->pAnnexE->hTimers,
            retransmit_or_ima_timer_event,
            pNode,
            (pNode->pWaitingForAckPDU != NULL) ? get_retransmit_interval( pNode ) : get_ima_interval( pNode )
        );
}

void stop_retransmit_or_ima_timer( tNode* pNode ) {
    if (pNode->hResendAndIMATimer != EMPTYHTI) {
        mtimerReset( pNode->pAnnexE->hTimers, pNode->hResendAndIMATimer );
        pNode->hResendAndIMATimer = EMPTYHTI;
    }
}

void RVCALLCONV delay_timer_event(void* context) {
    tNode* pNode = (tNode*)context;
    RV_ASSERT( pNode->pCurrentPDU != NULL );
    send_current_pdu( pNode );
}

void RVCALLCONV retransmit_or_ima_timer_event(void* context) {
    tNode* pNode = (tNode*)context;
    if (pNode->pWaitingForAckPDU != NULL) {
        /* retransmit timer event! */
        if (pNode->nRetry++ == pNode->pAnnexE->n_R1) {
            remote_host_is_dead( pNode );
            return;
        }
        msaPrintFormat( pNode->pAnnexE->msaType, "Retransmit PDU to Host(%08x:%i), retry=%i\n", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort, pNode->nRetry-1 );
        send_pdu( pNode->pAnnexE, pNode->pWaitingForAckPDU );
    }
    else {
        /* ima timer event! */
        if (pNode->nRetry++ == pNode->pAnnexE->n_IMA1) {
            remote_host_is_dead( pNode );
            return;
        }
        msaPrintFormat( pNode->pAnnexE->msaType, "IAmAlive timer event - Send IMA to Host(%08x:%i), retry=%i ", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort, pNode->nRetry-1 );

        /* send ima payload*/
        msaPrintFormat( pNode->pAnnexE->msaType, "send!\n" );
        send_ima( pNode, TRUE );
    }
    start_retransmit_or_ima_timer( pNode );
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/
