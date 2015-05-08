
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifdef __cplusplus
extern "C" {
#endif

#include <cmintr.h>
#include <cm.h>
#include <q931asn1.h>
#include <h245.h>
#include <cmCall.h>
#include <cmQ931.h>
#include <cmchan.h>
#include <cmControl.h>
#include <cmCtrlMSD.h>
#include <cmCtrlCap.h>
#include <transport.h>
#include <statistic.h>


/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * mibGetCallParameters
 * purpose: Get call related MIB parameters
 * input  : hsCall      - Call to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetCallParameters(
    IN  HCALL               hsCall,
    IN  mibCallParamTypeT   type,
    OUT int*                valueSize,
    OUT UINT8*              value)
{
    if (value);
    
    switch (type)
    {
        case enumconnectionsState:
        {
            switch (q931GetCallState(cmiGetQ931(hsCall)))
            {
                case cs_Call_initiated:             *valueSize = callInitiated; break;
                case cs_Call_delivered:             *valueSize = callDelivered; break;
                case cs_Call_present:               *valueSize = callPresent; break;
                case cs_Call_received:              *valueSize = callReceived; break;
                case cs_Connect_request:            *valueSize = connectRequest; break;
                case cs_Outgoing_call_proceeding:   *valueSize = cscallProceeding; break;
                case cs_Active:                     *valueSize = active; break;
                case cs_Disconnect_request:         *valueSize = disconnectRequest; break;
                case cs_Disconnect_indication:      *valueSize = disconnectIndication; break;
                case cs_Release_request:            *valueSize = releaseRequest; break;
                default:
                    return RVERROR;
            }
            return 0;
        }
        case enumconnectionsFastCall:
            break;
        case enumconnectionsH245Tunneling:
            break;
    }

    /* No such luck in finding the parameter */
    return RVERROR;
}


/************************************************************************
 * cmCallGetH245Address
 * purpose: Get H245 addresses for a call
 * input  : hsCall      - Call to check
 * output : trSrc       - Source H245 address
 *          trDest      - Destination H245 address
 *          isConnected - Indication if H245 channel is connected or not
 * return : Non-negative value on success
 *          Negative value on failure
 * Note: If H.245 is not connected through a dedicated connection (i.e
 *       uses fast start or tunneling), then the returned addresses are
 *       the Q931 addresses
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallGetH245Address(
    IN  HCALL               hsCall,
    OUT cmTransportAddress* trSrc,
    OUT cmTransportAddress* trDest,
    OUT int*                isConnected)
{
    HSTRANSSESSION  hSession;
    HSTRANSHOST     hHost;
    int             status;
    INT32           isTunneling;
    BOOL            isOrigin;

    *isConnected = 0;

    /* See if we're working with tunneling on this call */
    status = cmCallGetParam(hsCall, cmParamH245Tunneling, 0, &isTunneling, NULL);
    if (status < 0) return status;

    if (!isTunneling)
    {
        /* No tunneling. See if we're working in faststart or if we're connected */
        /* TODO: What should I do with it? */
    }

    if (!(*isConnected))
    {
        /* Get the Q931 addresses */
        INT32 length = sizeof(cmTransportAddress);
        status = cmCallGetParam(hsCall, cmParamSrcCallSignalAddress, 0, &length, (char *)trSrc);
        if (status < 0)
            return RVERROR;

        length = sizeof(cmTransportAddress);
        status = cmCallGetParam(hsCall, cmParamDestCallSignalAddress, 0, &length, (char *)trDest);
        if (status < 0)
            return RVERROR;
        return 0;
    }

    /* Get H245 address */
    if (cmCallGetOrigin(hsCall, &isOrigin) < 0)
        return RVERROR;

    hSession = callMibGetSession(hsCall);
    if (cmTransGetSessionParam(hSession, cmTransParam_H245Connection, (void *)&hHost) != cmTransOK)
        return RVERROR;

    if (isOrigin)
    {
        cmTransGetHostParam(hHost, cmTransHostParam_localAddress, trSrc);
        cmTransGetHostParam(hHost, cmTransHostParam_remoteAddress, trDest);
    }
    else
    {
        cmTransGetHostParam(hHost, cmTransHostParam_localAddress, trDest);
        cmTransGetHostParam(hHost, cmTransHostParam_remoteAddress, trSrc);
    }

    return 0;
}


/************************************************************************
 * mibGetControlParameters
 * purpose: Get control related MIB parameters
 * input  : hsCall      - Call to check
 *          inDirection - Direction to get
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetControlParameters(
    IN  HCALL                   hsCall,
    IN  int                     inDirection,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value)
{
    int status = RVERROR;

    if (hsCall == NULL) return RVERROR;

    switch (type)
    {
        case enumh245ControlChannelMSDState:
        {
            msdState    state;
            msdStatus   mStatus;
            status = msdGetMibParams((controlElem *)cmiGetControl(hsCall), &state, &mStatus, NULL, NULL);
            if (status < 0) return status;

            switch (mStatus)
            {
                case indeterminate:
                    switch (state)
                    {
                        case idle:
                                *valueSize = msdseIdle; return 0;
                        case incomingAwaitingResponse:
                                *valueSize = msdseIncomingWaitingResponse; return 0;
                        case incomingAwaitingManualAcknoledge:
                        case outgoingAwaitingResponse:
                                *valueSize = msdseOutgoingWaitingResponse; return 0;
                    }
                    break;
                case master:    *valueSize = msdseMaster; return 0;
                case slave:     *valueSize = msdseSlave; return 0;
            }
            break;
        }
        case enumh245ControlChannelTerminalType:
        {
            int termType;
            status = msdGetMibParams((controlElem *)cmiGetControl(hsCall), NULL, NULL, NULL, &termType);
            if (status >= 0)
            {
                *valueSize = termType;
                return 0;
            }
            break;
        }
        case enumh245ControlChannelNumberOfMSDRetries:
        {
            int retries;
            status = msdGetMibParams((controlElem *)cmiGetControl(hsCall), NULL, NULL, &retries, NULL);
            if (status >= 0)
            {
                *valueSize = retries;
                return 0;
            }
            break;
        }
        case enumh245ControlChannelIsTunneling:
        {
            INT32 isTunneling;
            status = cmCallGetParam(hsCall, cmParamH245Tunneling, 0, &isTunneling, NULL);
            if (status >= 0)
            {
                *valueSize = isTunneling;
                return 0;
            }
            break;
        }
        case enumh245CapExchangeState:
        {
            capStatus cStatus;
            status = tcsGetMibParams((controlElem *)cmiGetControl(hsCall), inDirection, &cStatus, NULL);
            if (status >= 0)
            {
                switch (cStatus)
                {
                    case capReleased:       *valueSize = ceseRelease; return 0;
                    case capSent:           *valueSize = ceseSent; return 0;
                    case capAcknowledged:   *valueSize = ceseAck; return 0;
                    case capRejected:       *valueSize = ceseReject; return 0;
                }
            }
            break;
        }
        case enumh245CapExchangeProtocolId:
        {
            status = tcsGetMibProtocolId((controlElem *)cmiGetControl(hsCall), inDirection, valueSize, value);
            if (status >= 0) return 0;
            break;
        }
        case enumh245CapExchangeRejectCause:
        {
            status = tcsGetMibParams((controlElem *)cmiGetControl(hsCall), inDirection, NULL, valueSize);
            if (status >= 0)
            {
                if (*valueSize < 0)
                    status = *valueSize;
                else
                    return 0;
            }
            break;
        }
        default:
            status = RVERROR;
    }

    return status;
}


/************************************************************************
 * mibGetChannelParameters
 * purpose: Get channel related MIB parameters
 * input  : hsChan      - Channel to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV mibGetChannelParameters(
    IN  HCHAN                   hsChan,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value)
{
    if (hsChan == NULL) return RVERROR;

    return chanGetMibParam(hsChan, type, valueSize, value);
}



#ifdef __cplusplus
}
#endif

