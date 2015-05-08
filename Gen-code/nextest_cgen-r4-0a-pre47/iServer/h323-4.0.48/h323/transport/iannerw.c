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


UINT32
get_next_seqn( tNode* pNode ) {
    UINT32 res = pNode->nNext_SEQ++;
    if (pNode->nNext_SEQ >= 0x01000000U)
        pNode->nNext_SEQ = 0;
    return res;
}

BOOL
is_old_seqn( UINT32 seqn, UINT32 prev_seqn ) {
    if (prev_seqn == 0xffffffffU)
        return FALSE;
    if (prev_seqn >= 0x7fffffU) {
        if ((seqn <= prev_seqn) && (seqn >= (prev_seqn - 0x7fffffU)))
            return TRUE;
        return FALSE;
    }
    else {
        if ((seqn <= prev_seqn) || (seqn >= (0x7fffffU + prev_seqn)))
            return TRUE;
        return FALSE;
    }
}

void
send_pdu( tAnnexE* pAnnexE, tPDU* pPDU ) {
    if (pPDU->fPDUIsWaitingForSend)
        return;
    if ((!IsListEmpty( &pAnnexE->WaitingForTransmissionList )) || (!pAnnexE->fReadyToSend)) {
        pPDU->fPDUIsWaitingForSend = TRUE;
        InsertTailList( &pAnnexE->WaitingForTransmissionList, &pPDU->lPDULink );
        pPDU = NULL;
    }
    if (pAnnexE->fReadyToSend) {
        if (pAnnexE->fReadyToSend) {
            int res;
            if (pPDU == NULL) {
                PLIST_ENTRY plink;
                if (IsListEmpty( &pAnnexE->WaitingForTransmissionList ))
                    return;
                plink = RemoveHeadList( &pAnnexE->WaitingForTransmissionList );
                pPDU = CONTAINING_RECORD( plink, tPDU, lPDULink );
                pPDU->fPDUIsWaitingForSend = FALSE;
            }
            res = liUdpSend(
                    pAnnexE->sock,
                    &pPDU->PDU[0],
                    pPDU->nSize,
                    pPDU->nIP,
                    pPDU->nPort
                );
            if (res < 0) {
                pPDU->fPDUIsWaitingForSend = TRUE;
                InsertHeadList( &pAnnexE->WaitingForTransmissionList, &pPDU->lPDULink );
                pAnnexE->fReadyToSend = FALSE;
                return;
            }
            if (AEHGet_A( pPDU->PDU ) == 0)
                free_pdu( pAnnexE, pPDU );

            pPDU = NULL;
        }
    }
}

void
free_pdu( tAnnexE* pAnnexE, tPDU* pPDU ) {
    if (pPDU->fPDUIsWaitingForSend) {
        RemoveEntryList( &pPDU->lPDULink );
        pPDU->fPDUIsWaitingForSend = FALSE;
    }
    pPDU->nSize = 0;
    InsertTailList( &pAnnexE->FreePDUList, &pPDU->lPDULink );
    unblock_res_blocked_nodes( pAnnexE );
}

void
unblock_res_blocked_nodes( tAnnexE* pAnnexE ) {
    PLIST_ENTRY last;
    tNode*      lastPNode;
    BOOL        finished = FALSE;

    if (pAnnexE->State != RUNNING)
        return;

    last = GetLastList(&pAnnexE->ResBlockedNodeList);
    lastPNode = CONTAINING_RECORD( last, tNode, lPendLink );

    while ((!IsListEmpty( &pAnnexE->FreePDUList )) &&
           (!IsListEmpty( &pAnnexE->ResBlockedNodeList )) &&
           (!finished)) {

        tNode*      pNode;

        PLIST_ENTRY plink = RemoveHeadList( &pAnnexE->ResBlockedNodeList );
        pNode = CONTAINING_RECORD( plink, tNode, lPendLink );

        if (pAnnexE->events.AnnexEEvWriteable != NULL) {
            pNode->nRef++; /* protect node */
            pAnnexE->events.AnnexEEvWriteable(
                    AsHANNEXE( pNode->pAnnexE ),
                    pNode->pAnnexE->hAppAnnexE,
                    pNode->RemoteHost.nIP,
                    pNode->RemoteHost.nPort
                );
            if (pNode->nRef == 1) {
                del_node( pNode );
                continue;
            }
            pNode->nRef--; /* unprotect node */
        }

        if (pNode == lastPNode)
            finished = TRUE;
    }
}

void
send_current_pdu( tNode* pNode ) {
    tPDU* pPDU;
    RV_ASSERT( pNode->pCurrentPDU != NULL );
    if (AEHGet_A( pNode->pCurrentPDU->PDU ) != 0) {
        RV_ASSERT( pNode->pWaitingForAckPDU == NULL );
        pNode->nRetry = 0;
        pNode->pWaitingForAckPDU = pNode->pCurrentPDU;
        start_retransmit_or_ima_timer( pNode );
    };
    pPDU = pNode->pCurrentPDU;
    pNode->pCurrentPDU = NULL;
    send_pdu( pNode->pAnnexE, pPDU );
}

BOOL
send_payload(
        tNode*      pNode,
        void*       payload_header,
        int         payload_header_size,
        void*       payload_data,
        int         payload_data_size,
        void*       payload_exdata,
        int         payload_exdata_size,
        BOOL        AckFlag,
        BOOL        HFlag,
        BOOL        SendNow
        )
{
    tPDU*   pCurrentPDU;
    BOOL    fFirstPayload = FALSE;

    /* if can't send add to the list of pended nodes */
    if (AckFlag && (pNode->pWaitingForAckPDU != NULL)) {
        return FALSE;
    }

    /* get free PDU if needed   */
    if (pNode->pCurrentPDU == NULL) {
        PLIST_ENTRY plink;
        if (IsListEmpty( &pNode->pAnnexE->FreePDUList )) {
            InsertTailList( &pNode->pAnnexE->ResBlockedNodeList, &pNode->lPendLink );
            return FALSE;
        }
        fFirstPayload = TRUE;
        plink = RemoveHeadList( &pNode->pAnnexE->FreePDUList );
        pNode->pCurrentPDU = CONTAINING_RECORD( plink, tPDU, lPDULink );

        pCurrentPDU = pNode->pCurrentPDU;

        pCurrentPDU->nIP    = pNode->RemoteHost.nIP;
        pCurrentPDU->nPort  = pNode->RemoteHost.nPort;

        pCurrentPDU->PDU[IAnnexEHeader_FLAGS] = 0;

        hton24( get_next_seqn( pNode ), &pCurrentPDU->PDU[IAnnexEHeader_SEQN] );

        pCurrentPDU->nSize = sizeof_IAnnexEHeader;

        if (payload_header_size + payload_data_size + payload_exdata_size >= (pCurrentPDU->nMaxSize - pCurrentPDU->nSize)) {
            RV_ASSERT( FALSE ); /* this is serious APPLICATION bug (payload is greather than PDU size)!!! */
            return FALSE;
        }
    }
    else
        pCurrentPDU = pNode->pCurrentPDU;

    /* check for free space in PDU */
    if (payload_header_size + payload_data_size + payload_exdata_size < (pCurrentPDU->nMaxSize - pCurrentPDU->nSize)) {
        /* append payload to the PDU */
        if (payload_header_size > 0) {
            memcpy( &pCurrentPDU->PDU[pCurrentPDU->nSize], payload_header, payload_header_size );
            pCurrentPDU->nSize += payload_header_size;
        }
        if (payload_data_size > 0) {
            memcpy( &pCurrentPDU->PDU[pCurrentPDU->nSize], payload_data, payload_data_size );
            pCurrentPDU->nSize += payload_data_size;
        }
        if (payload_exdata_size > 0) {
            memcpy( &pCurrentPDU->PDU[pCurrentPDU->nSize], payload_exdata, payload_exdata_size );
            pCurrentPDU->nSize += payload_exdata_size;
        }

        AEHOr_A( pCurrentPDU->PDU, AckFlag );
        AEHOr_H( pCurrentPDU->PDU, HFlag );

        if ( (SendNow) && (!pNode->fDontSend) ) {
            send_current_pdu( pNode );
        }

        return TRUE;
    }
    else {
        /* no place for that payload => send current PDU!   */
        send_current_pdu( pNode );

        if (AckFlag && (pNode->pWaitingForAckPDU != NULL))
            return FALSE;
        else
            return send_payload( pNode, payload_header, payload_header_size, payload_data, payload_data_size, payload_exdata, payload_exdata_size, AckFlag, HFlag, SendNow );
    }
}

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
) {
    tAnnexE *pAnnexE = AsAnnexE( hAnnexE );
    tNode   *pNode;

    if (!pAnnexE)
        return annexEStatusBadParameter;

    pNode = get_node( pAnnexE, ip, port );
    if (pNode == NULL) {
        pNode = add_node( pAnnexE, ip, port );
    }
    if (pNode == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexESendMessage() - annexEStatusResourceProblem!" );
        return annexEStatusResourceProblem;
    }

    if (pNode->fLocalAddressUsed) {
        IAnnexET10PayloadSA pheader[sizeof_IAnnexET10PayloadSA_ND];

        pheader[IAnnexET10PayloadSA_Header + IAnnexET10Header_PFLAGS] = AEP_MASK_A | AEP_MASK_S | (AEPT_StaticMessage << AEP_SHFT_T);
        /*
        pheader.Header.A            = 1;
        pheader.Header.S            = 1;
        pheader.Header.RES          = 0;
        pheader.Header.T            = AEPT_StaticMessage;
        */
        pheader[IAnnexET10PayloadSA_Header + IAnnexET10Header_STATIC_TYPE]  = 0; /* h.225 messages */

        hton32( pNode->nLocalAddress, &pheader[IAnnexET10PayloadSA_ADDRESS] );
        hton16( CRV, &pheader[IAnnexET10PayloadSA_SESSION] );
        hton16( (UINT16)size, &pheader[IAnnexET10PayloadSA_DATA_LENGTH] );

        if (send_payload( pNode, pheader, sizeof_IAnnexET10PayloadSA_ND, message, size, NULL, 0, fAckHint, fReplyHint, fSendHint )) {
            return annexEStatusNormal;
        }
        else {
            return annexEStatusWouldBlock;
        }
    }
    else {
        IAnnexET10PayloadS  pheader[sizeof_IAnnexET10PayloadS_ND];

        pheader[IAnnexET10PayloadS_Header + IAnnexET10Header_PFLAGS] = AEP_MASK_S | (AEPT_StaticMessage << AEP_SHFT_T);
        /*
        pheader.Header.A            = 0;
        pheader.Header.S            = 1;
        pheader.Header.RES          = 0;
        pheader.Header.T            = AEPT_StaticMessage;
        */
        pheader[IAnnexET10PayloadS_Header + IAnnexET10Header_STATIC_TYPE]   = 0; /* h.225 messages  */
        hton16( CRV, &pheader[IAnnexET10PayloadS_SESSION] );
        hton16( (UINT16)size, &pheader[IAnnexET10PayloadS_DATA_LENGTH] );

        if (send_payload( pNode, pheader, sizeof_IAnnexET10PayloadS_ND, message, size, NULL, 0, fAckHint, fReplyHint, fSendHint )) {
            return annexEStatusNormal;
        }
        else {
            return annexEStatusWouldBlock;
        }
    }
}

RVAPI annexEStatus RVCALLCONV
annexECloseNode(
    IN  HANNEXE                 hAnnexE,
    IN  UINT32                  ip,
    IN  UINT16                  port
) {
    tAnnexE *pAnnexE = AsAnnexE( hAnnexE );
    tNode   *pNode;

    if (!pAnnexE)
        return annexEStatusBadParameter;

    pNode = get_node( pAnnexE, ip, port );
    if (pNode == NULL) {
        msaPrintFormat( pAnnexE->msaType, "annexECloseNode() - no such Remote Host(%08x:%i)!",
                   ip, port);
        return annexEStatusBadParameter;
    }
    else {
        msaPrintFormat( pNode->pAnnexE->msaType, "Remote Host(%08x:%i) closed!", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );
        del_node( pNode );
    }

    return annexEStatusNormal;
}

void
remote_host_is_dead( tNode* pNode ) {
    msaPrintFormat( pNode->pAnnexE->msaType, "Remote Host(%08x:%i) is dead!", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );
    if (pNode->pAnnexE->events.AnnexEEvConnectionBroken != NULL) {
        pNode->pAnnexE->events.AnnexEEvConnectionBroken(
                AsHANNEXE( pNode->pAnnexE ),
                pNode->pAnnexE->hAppAnnexE,
                pNode->RemoteHost.nIP,
                pNode->RemoteHost.nPort
            );
    }
    else
        del_node( pNode );
}

void
remote_host_is_alive( tNode* pNode ) {
    if ((pNode->pWaitingForAckPDU == NULL)) {

        pNode->nRetry = 0;
        start_retransmit_or_ima_timer( pNode );
    }
}

void
EvRead( int socketId, liEvents event, int error, void* context ) {
    tAnnexE*    pAnnexE = (tAnnexE*)context;
    tNode*      pNode;

    error = error;
    event = event;
    socketId = socketId;

    if ((pAnnexE->pRecvPDU->nSize = liUdpRecv(
                pAnnexE->sock,
                &pAnnexE->pRecvPDU->PDU[0],
                pAnnexE->pRecvPDU->nMaxSize,
                &pAnnexE->pRecvPDU->nIP,
                &pAnnexE->pRecvPDU->nPort
                )) > 0) {

        /* check AnnexE PDU header! */
        if ((pAnnexE->pRecvPDU->nSize <= sizeof_IAnnexEHeader) ||
            (AEHGet_VER( pAnnexE->pRecvPDU->PDU ) != 0) ||
            (AEHGet_RES( pAnnexE->pRecvPDU->PDU ) != 0)) {

            /* reject!  */
            msaPrintFormat( pAnnexE->msaType, "(incoming PDU) bad header, rejected!" );
            return;
        }

        /* search for node  */
        pNode = get_node( pAnnexE, pAnnexE->pRecvPDU->nIP, pAnnexE->pRecvPDU->nPort );

        if (pNode == NULL) {
            pNode = add_node( pAnnexE, pAnnexE->pRecvPDU->nIP, pAnnexE->pRecvPDU->nPort );

            if (pNode == NULL) {
                /* no free resources!
                 reject! */
                msaPrintFormat( pAnnexE->msaType, "(incoming PDU) no free resources, rejected!" );
                return;
            }
        }

        pNode->nRef++; /* protect node  */
        process_pdu( pNode, pAnnexE->pRecvPDU );
        if (pNode->nRef == 1) {
            del_node( pNode );
        }
        else
            pNode->nRef--; /* unprotect node */
    }
}

void
send_ima( tNode* pNode, BOOL fSendNow ) {
    IAnnexET00IAmAlive  ima[sizeof_IAnnexET00IAmAlive_ND];
    BYTE                cookie[4];

    ima[IAnnexET00IAmAlive_Header+IAnnexET00Header_PFLAGS]  = 0;
    AEPOr_T( ima[IAnnexET00IAmAlive_Header+IAnnexET00Header_PFLAGS], AEPT_TransportMessage );
    /*
    ima.Header.S    = 0;
    ima.Header.A    = 0;
    ima.Header.RES  = 0;
    */
    ima[IAnnexET00IAmAlive_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_IAmAlive;

    IMASet_P( ima );

    hton16( (UINT16)get_ima_interval( pNode ), &ima[IAnnexET00IAmAlive_VALIDITY] );

    IMASet_CookieLen( ima, sizeof( UINT32 ) );

    hton32( timerGetTimeInMilliseconds(), &cookie[0] );

    send_payload(
            pNode,
            ima,
            sizeof_IAmAliveHeader( &ima ),
            &cookie[0],
            4,
            NULL,
            0,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
i_am_alive_request_received( tNode* pNode, IAnnexET00IAmAlive* pPayload ) {
    msaPrintFormat( pNode->pAnnexE->msaType, "IAmAlive request from Host(%08x:%i)!", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );
    IMAClr_P( pPayload );
    send_payload(
            pNode,
            pPayload,
            sizeof_IAmAliveHeader( pPayload ),
            &pPayload[IAnnexET00IAmAlive_COOKIE],
            sizeof_IAmAliveCookie( pPayload ),
            NULL,
            0,
            FALSE,
            FALSE,
            TRUE
        );
}

void
i_am_alive_response_received( tNode* pNode, IAnnexET00IAmAlive* pPayload ) {
    /* do nothing! remote_host_is_alive() is called from process_pdu() ;-)  */
    if (sizeof_IAmAliveCookie( pPayload ) >= 4) {
        UINT32 send_time = ntoh32( &pPayload[IAnnexET00IAmAlive_COOKIE] );
        msaPrintFormat( pNode->pAnnexE->msaType, "IAmAlive response from Host(%08x:%i), delay( %ims )", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort, timerGetTimeInMilliseconds() - send_time );
    }
}

void
use_alternate_port( tNode* pNode, UINT32 ip, UINT16 port ) {
    cmTransportAddress      newAddress;

    msaPrintFormat( pNode->pAnnexE->msaType, "Use_Alternate_Port received from Host(%08x:%i), New Host(%08x:%i)", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort, ip, port );
    if (pNode->pAnnexE->events.AnnexEEvUseOtherAddress == NULL)
        return;
    newAddress.distribution = cmDistributionUnicast;
    newAddress.type         = cmTransportTypeIP;
    newAddress.ip           = ip;
    newAddress.port         = port;

    pNode->pAnnexE->events.AnnexEEvUseOtherAddress(
            AsHANNEXE( pNode->pAnnexE ),
            pNode->pAnnexE->hAppAnnexE,
            pNode->RemoteHost.nIP,
            pNode->RemoteHost.nPort,
            &newAddress
    );
}

void
static_type0_is_not_supported( tNode* pNode ) {

    msaPrintFormat( pNode->pAnnexE->msaType, "Static Type=0 is not supported from remote Host(%08x:%i)", pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );
    if (pNode->pAnnexE->events.AnnexEEvNotSupported == NULL)
        return;

    pNode->pAnnexE->events.AnnexEEvNotSupported(
            AsHANNEXE( pNode->pAnnexE ),
            pNode->pAnnexE->hAppAnnexE,
            pNode->RemoteHost.nIP,
            pNode->RemoteHost.nPort
    );
}

void
send_ack( tNode* pNode, UINT32 seqn, BOOL fSendNow ) {
    IAnnexET00Ack ack[sizeof_IAnnexET00Ack_ND+sizeof_IAnnexEAckData];
    ack[IAnnexET00Ack_Header+IAnnexET00Header_PFLAGS] = 0;
    /*
    ack.Header.T = 0;
    ack.Header.A = 0;
    ack.Header.S = 0;
    ack.Header.RES = 0;
    */
    ack[IAnnexET00Ack_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_Ack;
    hton16( 1, &ack[IAnnexET00Ack_ACK_COUNT] );
    hton24( seqn, &ack[IAnnexET00Ack_ACK+IAnnexEAckData_SEQNUM] );
    ack[IAnnexET00Ack_ACK+IAnnexEAckData_RESERVED] = 0;

    send_payload(
            pNode,
            ack,
            sizeof_AckHeader( &ack ),
            &ack[IAnnexET00Ack_ACK],
            sizeof_IAnnexEAckData,
            NULL,
            0,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
send_nack3( tNode* pNode, UINT32 seqn, int transport_type, BOOL fSendNow ) {
    /* send NACK REASON_3:"Transport-payload not supported" */
    IAnnexET00NAckHeader header[sizeof_IAnnexET00NAckHeader];
    IAnnexENAckReason3   reason[sizeof_IAnnexENAckReason3];

    msaPrintFormat( pNode->pAnnexE->msaType, "Send NAck3(Transport-payload-type=%i not supported, SEQN=%i) to Host(%08x:%i)", transport_type, seqn, pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );

    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_PFLAGS] = 0;
    /*
    header.Header.T = 0;
    header.Header.A = 0;
    header.Header.S = 0;
    header.Header.RES = 0;
    */
    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_NAck;
    hton16( 1, &header[IAnnexET00NAckHeader_NACK_COUNT] );

    hton24( seqn, &reason[IAnnexENAckReason3_SEQNUM] );
    reason[IAnnexENAckReason3_DATA_LENGTH] = 1;
    hton16( 3, &reason[IAnnexENAckReason3_REASON] );
    reason[IAnnexENAckReason3_TRANSPORT_TYPE] = (BYTE)transport_type;

    send_payload(
            pNode,
            header,
            sizeof_IAnnexET00NAckHeader,
            reason,
            sizeof_IAnnexENAckReason3,
            NULL,
            0,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
send_nack4( tNode* pNode, UINT32 seqn, int static_type, BOOL fSendNow ) {
    /* send NACK REASON_4:"Static-payload type not supported"   */
    IAnnexET00NAckHeader header[sizeof_IAnnexET00NAckHeader];
    IAnnexENAckReason4   reason[sizeof_IAnnexENAckReason4];

    msaPrintFormat( pNode->pAnnexE->msaType, "Send NAck4(Static-payload-type=%i not supported, SEQN=%i) to Host(%08x:%i)", static_type, seqn, pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );

    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_PFLAGS] = 0;
    /*
    header.Header.T = 0;
    header.Header.A = 0;
    header.Header.S = 0;
    header.Header.RES = 0;
    */
    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_NAck;
    hton16( 1, &header[IAnnexET00NAckHeader_NACK_COUNT] );

    hton24( seqn, &reason[IAnnexENAckReason4_SEQNUM] );
    reason[IAnnexENAckReason4_DATA_LENGTH] = 1;
    hton16( 4, &reason[IAnnexENAckReason4_REASON] );
    reason[IAnnexENAckReason4_STATIC_TYPE] = (BYTE)static_type;

    send_payload(
            pNode,
            header,
            sizeof_IAnnexET00NAckHeader,
            reason,
            sizeof_IAnnexENAckReason4,
            NULL,
            0,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
send_nack5( tNode* pNode,UINT32 seqn, int oid_length, BYTE* poid, BOOL fSendNow ) {
    /* send NACK REASON_5:"ObjectID-payload not supported!" */
    IAnnexET00NAckHeader header[sizeof_IAnnexET00NAckHeader];
    IAnnexENAckReason    reason[sizeof_IAnnexENAckReason_ND];

    msaPrintFormat( pNode->pAnnexE->msaType, "Send NAck5(ObjectID-payload not supported, SEQN=%i) to Host(%08x:%i)", seqn, pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );

    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_PFLAGS] = 0;
    /*
    header.Header.T = 0;
    header.Header.A = 0;
    header.Header.S = 0;
    header.Header.RES = 0;
    */
    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_NAck;
    hton16( 1, &header[IAnnexET00NAckHeader_NACK_COUNT] );

    hton24( seqn, &reason[IAnnexENAckReason_SEQNUM] );
    reason[IAnnexENAckReason_DATA_LENGTH] = (BYTE)(oid_length & 0xff);
    hton16( 5, &reason[IAnnexENAckReason_REASON] );

    send_payload(
            pNode,
            header,
            sizeof_IAnnexET00NAckHeader,
            reason,
            sizeof_IAnnexENAckReason_ND,
            poid,
            oid_length,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
send_nack6( tNode* pNode, UINT32 seqn, int nPayload, BOOL fSendNow ) {
    /* send NACK REASON_6:"Payload Corrupted"   */
    IAnnexET00NAckHeader header[sizeof_IAnnexET00NAckHeader];
    IAnnexENAckReason6   reason[sizeof_IAnnexENAckReason6];

    msaPrintFormat( pNode->pAnnexE->msaType, "Send NAck6(Payload corrupted, Id=%i, SEQN=%i) to Host(%08x:%i)", nPayload, seqn, pNode->RemoteHost.nIP, pNode->RemoteHost.nPort );

    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_PFLAGS] = 0;
    /*
    header.Header.T = 0;
    header.Header.A = 0;
    header.Header.S = 0;
    header.Header.RES = 0;
    */
    header[IAnnexET00NAckHeader_Header+IAnnexET00Header_TRANSPORT_TYPE] = AEPT00_NAck;
    hton16( 1, &header[IAnnexET00NAckHeader_NACK_COUNT] );

    hton24( seqn, &reason[IAnnexENAckReason6_SEQNUM] );
    reason[IAnnexENAckReason6_DATA_LENGTH] = 1;
    hton16( 6, &reason[IAnnexENAckReason6_REASON] );
    reason[IAnnexENAckReason6_PAYLOAD_NUMBER] = (BYTE)nPayload;

    send_payload(
            pNode,
            header,
            sizeof_IAnnexET00NAckHeader,
            reason,
            sizeof_IAnnexENAckReason6,
            NULL,
            0,
            FALSE,
            FALSE,
            fSendNow
        );
}

void
ack_received( tNode* pNode ) {
    tPDU* pPDU = pNode->pWaitingForAckPDU;
    pNode->pWaitingForAckPDU = NULL;

    /* notify user that the node is no longer waiting for ack */
    if (pNode->pAnnexE->events.AnnexEEvWriteable != NULL) {
        pNode->nRef++; /* protect node */
        pNode->pAnnexE->events.AnnexEEvWriteable(
                AsHANNEXE( pNode->pAnnexE ),
                pNode->pAnnexE->hAppAnnexE,
                pNode->RemoteHost.nIP,
                pNode->RemoteHost.nPort
            );
        if (pNode->nRef == 1) {
            del_node( pNode );
        }
        pNode->nRef--; /* unprotect node */
    }
    /* start IMA timer  */
    pNode->nRetry = 0;
    start_retransmit_or_ima_timer( pNode );
    /* free PDU! */
    free_pdu( pNode->pAnnexE, pPDU );
}

#define MoveToNextPayload( payload, payload_size ) \
    ((IAnnexEPayloadHeader*)( ((BYTE*)(payload)) + (payload_size) ))

#define MoveToNextReason( reason, reason_size ) \
    ((IAnnexENAckReason*)( ((BYTE*)(reason)) + (reason_size) ))

void process_pdu( tNode* pNode, tpPDU pPDU ) {

    IAnnexEPayloadHeader*   pPayload;
    BOOL                    fError = FALSE;
    int                     nCurPayload = 0;
    int                     nBytesAvail = pPDU->nSize;
    int                     i, cnt, nSz;
    BOOL                    fResendForced = FALSE;
    UINT32                  seqn;

    remote_host_is_alive( pNode );

    seqn = ntoh24( &pPDU->PDU[IAnnexEHeader_SEQN] );
    /* Check for duplicated PDU's. That check is posible only for PDU's with set A-flag!*/
    if (AEHGet_A( pPDU->PDU ) != 0) {
        if (is_old_seqn( seqn, pNode->nLast_Ack_SEQ )) {
            /* duplicate!
             An Ack is send immediately in order to unblock remote annexe module */
            msaPrintFormat( pNode->pAnnexE->msaType, "Duplicate PDU received! send Ack immediately." );
            send_ack( pNode, seqn, TRUE );
            return;
        }
        if (check_pdu( pPDU )) {
            pNode->nLast_Ack_SEQ = seqn;

            if (pNode->pWaitingForAckPDU != NULL)
                send_ack( pNode, seqn, TRUE );
            else
                send_ack( pNode, seqn, FALSE );
        }
    }

    /* get first payload!   */
    if (AEHGet_L( pPDU->PDU ) != 0) {
        /* this is valid only for Annex E over TCP, but it is possible that
            the host implementation of Annex E to not understand it.*/
        pPayload = (IAnnexEPayloadHeader*)(pPDU->PDU + 8);
    }
    else {
        pPayload = (IAnnexEPayloadHeader*)(pPDU->PDU + 4);
        nBytesAvail -= 4;
    }

    /* a cycle to walk through all payloads in the PDU  */
    while ((void*)pPayload < (void*)(&pPDU->PDU[pPDU->nSize])) {
        switch (AEPGet_T( *pPayload )) {
        case AEPT_TransportMessage: {
            /***************************************************************/
            /* T == 00 * Annex E Transport Messages                        */
            /***************************************************************/
            IAnnexET00Header* pTrPayload = (IAnnexET00Header*)(pPayload);

            /* check 'Source/Dest' and 'Session' flags in payload header */
            if ((AEPGet_A( *pPayload ) != 0) || (AEPGet_S( *pPayload ) != 0)) {
                /* this is not a valid Annex E Transport message!
                   send NACK REASON_6:"Payload Corrupted" */
                send_nack6( pNode, seqn, nCurPayload, TRUE );

                /* break the processing of the rest payloads as there is no
                   guarantee that the size of packet may be found correctly. */
                fError = TRUE;
                break;
            }

            switch (pTrPayload[IAnnexET00Header_TRANSPORT_TYPE]) {
            case AEPT00_IAmAlive:
                /* check size of payload! */
                nSz = sizeof_IAmAlive( pTrPayload );
                nBytesAvail -= nSz;
                if (nBytesAvail < 0) {
                    /* incorrect payload size! payload corrupted! */
                    send_nack6( pNode, seqn, nCurPayload, TRUE );
                    fError = TRUE;
                    break;
                }
                if (IMAGet_P( AsIAmAlive( pTrPayload ) )) {
                    /* I_Am_Alive message from remote AnnexE module, return
                       the same payload, with 'P' flag cleared!*/
                    i_am_alive_request_received( pNode, AsIAmAlive( pTrPayload ) );
                }
                else {
                    /* answer to our I-Am-Alive payload!    */
                    i_am_alive_response_received( pNode, AsIAmAlive( pTrPayload ) );
                }
                pPayload = MoveToNextPayload( pPayload, sizeof_IAmAlive( pTrPayload ) );
                break;
            case AEPT00_Ack:
                /*  check size of payload! */
                nSz = sizeof_Ack( pTrPayload );
                nBytesAvail -= nSz;
                if (nBytesAvail < 0) {
                    /*  incorrect payload size! payload corrupted! */
                    send_nack6( pNode, seqn, nCurPayload, TRUE );
                    fError = TRUE;
                    break;
                }
                /* as the current implementation of Annex E fulfills only the
                 'Serial' model, we are interested only of ACK.SEQN equal to
                 that waiting for acknowledge.*/
                if (pNode->pWaitingForAckPDU != NULL) {
                    UINT32  waiting_seqn = ntoh24( &pNode->pWaitingForAckPDU->PDU[IAnnexEHeader_SEQN] );
                    cnt = countof_AckData( pTrPayload );
                    for (i = 0; i < cnt; i++) {
                        UINT32 ack_seqn = ntoh24( &(AsAck( pTrPayload )[IAnnexET00Ack_ACK+
                                                                        i*sizeof_IAnnexEAckData+
                                                                        IAnnexEAckData_SEQNUM]) );
                        if (ack_seqn == waiting_seqn) {
                            ack_received( pNode );
                            break;
                        }
                    }
                }
                pPayload = MoveToNextPayload( pPayload, nSz );
                break;
            case AEPT00_NAck: {
                    IAnnexENAckReason*  pReason = AsNAckReason( GetNAckReasonPtr( pTrPayload ) );
                    /*  check size of payload! */
                    nSz = sizeof_IAnnexET00NAckHeader;
                    nBytesAvail -= nSz;
                    if (nBytesAvail < 0) {
                        /* incorrect payload size! payload corrupted! */
                        send_nack6( pNode, seqn, nCurPayload, TRUE );
                        fError = TRUE;
                        break;
                    }
                    cnt = countof_NAckReasons( pTrPayload );
                    for (i = 0; i < cnt; i++) {
                        UINT16  nReason;
                        UINT32  reason_seqn;
                        /* check size of reason */
                        nSz = sizeof_NAckReason( pReason );
                        nBytesAvail -= nSz;
                        if (nBytesAvail < 0) {
                            /* incorrect payload size! payload corrupted! */
                            send_nack6( pNode, seqn, nCurPayload, TRUE );
                            fError = TRUE;
                            break;
                        }

                        /*  process pReason */
                        nReason = ntoh16( &pReason[IAnnexENAckReason_REASON] );
                        reason_seqn = ntoh24( &pReason[IAnnexENAckReason_SEQNUM] );
                        switch (nReason) {
                        case 0:
                            /* NAck.REASON = Non-Standart Reason
                             not supported by this implementation of Annex E! Skipped!*/
                            break;
                        case 1:
                            /* NAck.REASON = Request the sender to use an alternate port
                             for the specified static payload type. That Nack is of
                             interest only if static type == 0 => (H.225) payload!*/
                            if (AsNAckReason1( pReason )[IAnnexENAckReason1_STATIC_TYPE] == 0) {
                                use_alternate_port( pNode,
                                                    ntoh32( &AsNAckReason1(pReason)[IAnnexENAckReason1_ALTERNATE_IP] ),
                                                    ntoh16( &AsNAckReason1(pReason)[IAnnexENAckReason1_ALTERNATE_PORT] ) );
                            }
                            break;
                        case 2:
                            /* NAck.REASON = Request the sender to use an alternate port
                             for the specified ObjectID payload type

                             not supported in that implementation of Annex E => it is not possible
                             for us to have sent such payload. Possible line error.
                             No reaction currently. It is possible that the next payload is
                             corrupted and the if the Nack is for the PDU with set A flag it
                             should be retransmitted.*/
                            break;
                        case 3:
                            /* NAck.REASON = Transport-payload not supported
                             Supporting only types to 3 including ('Restart Message'), their
                             implementation is necessary. Do not pay attention to that Nack!*/
                            break;
                        case 4:
                            /* NAck.REASON = Static-payload type not supported
                             the NAck is of interest only if it is a static type == 0 => (H.225) payload!*/
                            if (AsNAckReason4( pReason )[IAnnexENAckReason4_STATIC_TYPE] == 0) {
                                static_type0_is_not_supported( pNode );
                            }
                            break;
                        case 5:
                            /* NAck.REASON = OID-payload not supported
                               Do not pay attention to that Nack!*/
                            break;
                        case 6:
                            /* NAck.REASON = Payload Corrupted
                             The NAck is of interest only if it is about a PDU waiting for pWaitingForAckPDU!*/
                            if (!fResendForced && (pNode->pWaitingForAckPDU != NULL) && (reason_seqn == ntoh24( &pNode->pWaitingForAckPDU->PDU[IAnnexEHeader_SEQN] ))) {
                                fResendForced = TRUE;
                                stop_retransmit_or_ima_timer( pNode );
                                pNode->nRetry--;
                                retransmit_or_ima_timer_event( pNode );
                            }
                        default:
                            /* reserved for future use => skipped! */
                            break;
                        }

                        /* goto next reason */
                        pReason = MoveToNextReason( pReason, nSz );
                    }
                    pPayload = (IAnnexEPayloadHeader*)pReason;
                }
                break;
            case AEPT00_Restart:
                /*  check size of payload! */
                nSz = sizeof_IAnnexET00Restart;
                nBytesAvail -= nSz;
                if (nBytesAvail < 0) {
                    /* incorrect payload size! payload corrupted! */
                    send_nack6( pNode, seqn, nCurPayload, TRUE );
                    fError = TRUE;
                    break;
                }
                /* notify application!*/
                if (pNode->pAnnexE->events.AnnexEEvRestart != NULL) {
                    cmTransportAddress SrcAddr;
                    SrcAddr.distribution    = cmDistributionUnicast;
                    SrcAddr.type            = cmTransportTypeIP;
                    SrcAddr.ip              = pNode->RemoteHost.nIP;
                    SrcAddr.port            = pNode->RemoteHost.nPort;
                    pNode->pAnnexE->events.AnnexEEvRestart(
                            AsHANNEXE( pNode->pAnnexE ),
                            pNode->pAnnexE->hAppAnnexE,
                            &SrcAddr
                        );
                }
                else
                {
                    msaPrintFormat( pNode->pAnnexE->msaType,
                                    "Restart command arrived - no CB to treat it was given");

                }
                pPayload = MoveToNextPayload( pPayload, nSz );
                break;
            default:
                {
                    int transType = pTrPayload[IAnnexET00Header_TRANSPORT_TYPE];
                    /* reserved for future use.... (not supported by this version of AnnexE)*/
                    send_nack3( pNode, seqn, transType, TRUE );

                    /* break the processing of the rest payloads in the PDU as there
                    is no guarantee to find the exact size of the payload.*/
                    fError = TRUE;
                }
                break;
            }
        };
        break;
        case AEPT_OIDTypedMessage: {
            /***************************************************************/
            /* T == 01 * OBJECT IDENTIFIER typed messages                  */
            /***************************************************************/
            int iLength;

            /*  check size of payload! */
            if ((int)sizeof_OIDHeader( pPayload ) >= nBytesAvail) {
                /*  incorrect payload size! payload corrupted! */
                send_nack6( pNode, seqn, nCurPayload, TRUE );
                fError = TRUE;
                break;
            }
            nSz = sizeof_OID( pPayload );
            nBytesAvail -= nSz;
            if (nBytesAvail < 0) {
                /* incorrect payload size! payload corrupted! */
                send_nack6( pNode, seqn, nCurPayload, TRUE );
                fError = TRUE;
                break;
            }

            /* that type of messages is not supported in the current implementation of
            the Annex E => return corresponding NAck (REASON_5: "Object-ID payload
            not supported)! As it is possible to find the exact size of the payload
            the PDU parsing is not broken, but the current payload is skipped!*/
            iLength = AsOIDHeader(pPayload)[IAnnexET01Header_OID_LENGTH];
            send_nack5( pNode,
                        seqn,
                        iLength,
                        &AsOIDHeader(pPayload)[IAnnexET01Header_OID],
                        FALSE );

            pPayload = MoveToNextPayload( pPayload, sizeof_OID( pPayload ) );
        };
        break;
        case AEPT_StaticMessage: {
            /***************************************************************/
            /* T == 10 * Static-payload typed messages                     */
            /***************************************************************/

            /* typecast to the Annex E Static-typed message payload */
            IAnnexET10Payload*  pStPayload = (IAnnexET10Payload*)pPayload;

            BOOL                fSessionField = pStPayload[IAnnexET10Payload_Header+IAnnexET10Header_PFLAGS] & AEP_MASK_S;
            BOOL                fAddressField = pStPayload[IAnnexET10Payload_Header+IAnnexET10Header_PFLAGS] & AEP_MASK_A;
            UINT16              nSessionID;
            UINT32              nAddress = 0;
            BYTE*               pData;
            UINT16              nDataSize;
            int                 staticType;
            int                 iMsgSize;

            /*  check size of payload! */
            nSz = sizeof_Static(pPayload);
            nBytesAvail -= nSz;
            if (nBytesAvail < 0) {
                /*  incorrect payload size! payload corrupted! */
                send_nack6( pNode, seqn, nCurPayload, TRUE );
                fError = TRUE;
                break;
            }

            if (pStPayload[IAnnexET10Payload_Header+IAnnexET10Header_STATIC_TYPE] != 0) {
                /* this implementation of the Annex E supports only H.225 messages,
                 all the rest are unknown currently.*/
                staticType = pStPayload[IAnnexET10Payload_Header+IAnnexET10Header_STATIC_TYPE];
                send_nack4( pNode, seqn, staticType, FALSE );

                /* skip payload*/
                pPayload = MoveToNextPayload( pPayload, sizeof_Static(pStPayload) );
                break;
            }

            if (fSessionField) {
                if (fAddressField) {
                    nSessionID = ntoh16( &AsStaticSA(pStPayload)[IAnnexET10PayloadSA_SESSION] );
                    nAddress = ntoh32( &AsStaticSA(pStPayload)[IAnnexET10PayloadSA_SESSION] );
                    pData = &AsStaticSA(pStPayload)[IAnnexET10PayloadSA_DATA];
                    nDataSize = ntoh16( &AsStaticSA(pStPayload)[IAnnexET10PayloadSA_DATA_LENGTH] );
                }
                else {
                    nSessionID = ntoh16( &AsStaticS(pStPayload)[IAnnexET10PayloadS_SESSION] );
                    pData = &AsStaticS(pStPayload)[IAnnexET10PayloadS_DATA];
                    nDataSize = ntoh16( &AsStaticS(pStPayload)[IAnnexET10PayloadS_DATA_LENGTH] );
                }

                /* Turn the last bit of the session Id for incoming msgs */
                nSessionID ^= 0x8000U;
            }
            else {
                /* this h.225 message has no session fields defined. From AnnexE API those
                messages are only session oriented => we suppose the payload is
                broken while transferring.*/
                send_nack6( pNode, seqn, nCurPayload, FALSE );

                /* Do not break the processing of the PDU as that mistake may be in
                result of a bug in the remote Annex E module implementation. It is
                possible to follow valid payloads in the PDU!

                skip payload*/
                pPayload = MoveToNextPayload( pPayload, sizeof_Static(pStPayload) );
                break;
            }

            /* everything is alright and we may notify the user for the received payload*/
            if (pNode->pAnnexE->events.AnnexEEvNewMessage != NULL) {
                pNode->fDontSend = TRUE; /* block sends until all the treatment is done by the user */
                pNode->nRef++; /* protect the node*/
                iMsgSize = nDataSize;
                pNode->pAnnexE->events.AnnexEEvNewMessage(
                        AsHANNEXE( pNode->pAnnexE ),
                        pNode->pAnnexE->hAppAnnexE,
                        pNode->RemoteHost.nIP,
                        pNode->RemoteHost.nPort,
                        pData,
                        iMsgSize
                    );
                if (pNode->nRef == 1) {
                    /* node is closed by the application ?!?!*/
                    del_node( pNode );
                }
                else
                {
                    pNode->nRef--; /* unprotect the nodee*/
                    pNode->fDontSend = FALSE; /* allow sends again */

                    /* send any messages that the user has sent during the callback */
                    if (pNode->pCurrentPDU)
                        send_current_pdu( pNode );
                }
            }

            /* static-payload processing is done! now skip to the next one!*/
            pPayload = MoveToNextPayload( pPayload, nSz );
        };
        break;
        case AEPT_ReservedForFutureUse: {
            /***************************************************************/
            /* T == 11 * Reserved for future use                           */
            /***************************************************************/

            /* this is not a valid Annex E Transport message!*/
            send_nack6( pNode, seqn, nCurPayload, TRUE );

            /* break the processing of the rest payloads in the PDU as there is no
             guarantee to find the exact size of the payload.*/
            fError = TRUE;
        };
        break;
        }

        if (fError) {
            /* during the parsing of the current payload in the PDU there
            was a serious error! => break further processing!*/
            break;
        }

        nCurPayload++;
    }

    if ((!fError) && (AEHGet_A( pPDU->PDU ) != 0) && (AEHGet_H( pPDU->PDU ) == 0) && (pNode->pCurrentPDU != NULL)) {
        /* send current pdu forced*/
        send_current_pdu( pNode );
    }
}


BOOL
check_pdu( tpPDU pPDU ) {
    IAnnexEPayloadHeader*   pPayload;
    int                     nBytesAvail = pPDU->nSize;
    int                     nSz, i, cnt;

    /* get first payload!*/
    if (AEHGet_L( pPDU->PDU ) != 0) {
        pPayload = (IAnnexEPayloadHeader*)(pPDU->PDU + 8);
        nBytesAvail -= 4 + 4;
    }
    else {
        pPayload = (IAnnexEPayloadHeader*)(pPDU->PDU + 4);
        nBytesAvail -= 4;
    }

    if (nBytesAvail <= 0)
        return FALSE;

    while ((void*)pPayload < (void*)(&pPDU->PDU[pPDU->nSize])) {
        switch (AEPGet_T( *pPayload )) {
        case AEPT_TransportMessage: {
            IAnnexET00Header* pTrPayload = (IAnnexET00Header*)(pPayload);

            /* check 'Source/Dest' and 'Session' flags in payload header*/
            if ((AEPGet_A( *pPayload ) != 0) || (AEPGet_S( *pPayload ) != 0)) {
                /* this is not a valid Annex E Transport message!*/
                return FALSE;
            }

            switch (pTrPayload[IAnnexET00Header_TRANSPORT_TYPE]) {
            case AEPT00_IAmAlive:
                nSz = sizeof_IAmAlive( pTrPayload );
                nBytesAvail -= nSz;
                if (nBytesAvail < 0)
                    return FALSE;
                pPayload = MoveToNextPayload( pPayload, nSz );
                break;
            case AEPT00_Ack:
                nSz = sizeof_Ack( pTrPayload );
                nBytesAvail -= nSz;
                if (nBytesAvail < 0)
                    return FALSE;
                pPayload = MoveToNextPayload( pPayload, nSz );
                break;
            case AEPT00_NAck: {
                    IAnnexENAckReason*  pReason = AsNAckReason( GetNAckReasonPtr( pTrPayload ) );
                    nSz = sizeof_IAnnexET00NAckHeader;
                    nBytesAvail -= nSz;
                    if (nBytesAvail < 0)
                        return FALSE;
                    cnt = countof_NAckReasons( pTrPayload );
                    for (i = 0; i < cnt; i++) {
                        /* goto next reason*/
                        nSz = sizeof_NAckReason( pReason );
                        nBytesAvail -= nSz;
                        if (nBytesAvail < 0)
                            return FALSE;
                        pReason = MoveToNextReason( pReason, nSz );
                    }
                    pPayload = (IAnnexEPayloadHeader*)pReason;
                }
                break;
            case AEPT00_Restart:
                nSz = sizeof_IAnnexET00Restart;
                nBytesAvail -= nSz;
                if (nBytesAvail < 0)
                    return FALSE;
                pPayload = MoveToNextPayload( pPayload, nSz );
                break;
            default:
                return FALSE;
            }
        };
        break;
        case AEPT_OIDTypedMessage: {
            if ((int)sizeof_OIDHeader( pPayload ) >= nBytesAvail)
                return FALSE;
            nSz = sizeof_OID( pPayload );
            nBytesAvail -= nSz;
            if (nBytesAvail < 0)
                return FALSE;
            pPayload = MoveToNextPayload( pPayload, nSz );
        };
        break;
        case AEPT_StaticMessage: {
            nSz = sizeof_Static(pPayload);
            nBytesAvail -= nSz;
            if (nBytesAvail < 0)
                return FALSE;
            pPayload = MoveToNextPayload( pPayload, nSz );
        };
        break;
        case AEPT_ReservedForFutureUse: {
            return FALSE;
        };
        }
    }
    RV_ASSERT( nBytesAvail == 0);
    return TRUE;
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/
