
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef _CM_AUTORAS_INTR_H
#define _CM_AUTORAS_INTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmintr.h>
#include <cmAutoRas.h>


/************************************************************************
 * autorasRegState
 * Registration state of the automatic RAS
 * autorasRegNotTried   - Haven't tried to register yet
 * autorasRegTrying     - Currently trying t register
 * autorasRegistered    - Registered to a gatekeeper
 * autorasRegFailed     - Failed to register to a gatekeeper
 ************************************************************************/
typedef enum
{
    autorasRegNotTried,
    autorasRegTrying,
    autorasRegistered,
    autorasRegFailed
} autorasRegState;


/************************************************************************
 * autorasCallState
 * Call state of the automatic RAS
 * autorasCallIdle          - Call not present
 * autorasCallConnecting    - ARQ was sent - no response received yet
 * autorasCallConnected     - Call is connected
 * autorasCallDisconnecting - Disconnecting the call at current moment
 ************************************************************************/
typedef enum
{
    autorasCallIdle,
    autorasCallConnecting,
    autorasCallConnected,
    autorasCallDisconnecting
} autorasCallState;



/************************************************************************
 * autorasEndpoint struct
 * hApp             - Application's stack handle
 * hVal             - Value tree handle
 * confNode         - RAS configuration node
 * hTimers          - Timers pool handle
 *
 * state            - State of the endpoint (registered, discovered or idle)
 * internalState    - Internal registration state of the endpoint
 * discoveryRequired- Indicates if the endpoint has to send a GRQ, after receiving
 *                    a RRJ with reason discoveryRequired
 * discoveryComplete- Indicates if the endpoint got a GCF for this registration
 * regTries         - Number of regsitration tries that have occured
 * regTimer         - Registration timer. Used mainly for lightweight RRQs
 * registrationTx   - Registration transaction handle. Used for RRQ, GRQ
 *
 * event            - Event handler called on automatic RAS events
 ************************************************************************/
typedef struct
{
    HAPP                hApp;
    HPVT                hVal;
    int                 confNode;
    HSTIMER             hTimers;

    cmRegState          state;
    autorasRegState     internalState;
    BOOL                discoveryRequired;
    BOOL                discoveryComplete;
    int                 regTries;
    HTI                 regTimer;
    HRAS                registrationTx;

    cmiEvAutoRASEventT  event;
} autorasEndpoint;


/************************************************************************
 * autorasCall struct
 * tx           - Pending RAS transaction on call
 * irrTx        - Unsolicited IRR transactions
 * timer        - Timer handle used for RAS transactions on this call.
 *                1. For waiting for the endpoint to register on a GK
 *                2. For unsolicited IRR messages on this call
 * callState    - Indicates the current state of the RAS call
 ************************************************************************/
typedef struct
{
    HRAS                tx;
    HRAS                irrTx;
    HTI                 timer;
    autorasCallState    callState;
} autorasCall;



#ifdef __cplusplus
}
#endif

#endif  /* _CM_AUTORAS_INTR_H */
