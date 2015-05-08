
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************
 * cmAutoRasCall
 * -------------
 * This file provides the calls part of the automatic RAS module.
 * This includes the following features:
 *
 * - Handling incoming DRQs
 * - Responding automatically to IRQ with IRR on calls
 * - Responding automatically to BRQ with BCF on calls
 ************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include <q931asn1.h>
#include <cmintr.h>
#include <cmdebprn.h>
#include <cmras.h>
#include <cmCall.h>
#include <cmCrossReference.h>
#include <cmAutoRasCall.h>


/* Private functions declaration */
int autoRasPrepareARQ(IN autorasEndpoint* autoras, IN HCALL hsCall);





/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/

/************************************************************************
 * autoRasWaitRegistration
 * purpose: Wait to see if endpoint got registered before beginning to
 *          dial this call. This function is called for the call every
 *          second to see if the registration status has changed.
 * input  : context - autoras call handle
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV autoRasWaitRegistration(IN void* context)
{
    autorasEndpoint* autoras;
    autorasCall* call = (autorasCall *)context;
    HCALL hsCall;
    BOOL done = FALSE;

    hsCall = cmiGetByAutoRas((HAUTORASCALL)call);
    if (emaLock((EMAElement)hsCall))
    {
        autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

        /* Let's check the registration status once more */
        switch (autoras->internalState)
        {
            case autorasRegistered:
                /* We're registered - send ARQ */
                if (autoRasPrepareARQ(autoras, hsCall) < 0)
                {
                    /* Notify that we can't make this call */
                    autoras->event(hsCall, NULL, cmiAutoRasEvFailedOnARQ);
                }
                done = TRUE;
                break;

            case autorasRegFailed:
                /* Endpoint failed to register... */
                done = TRUE;
                autoras->event(hsCall, NULL, cmiAutoRasEvCantRegister);
                break;

            case autorasRegNotTried:
                /* This case should never relly happen */

            case autorasRegTrying:
                /* Do nothing in this case - we'll get back to here in a second with the timer */
                break;
        }

        if (done)
        {
            /* We've got to stop the timer */
            mtimerReset(autoras->hTimers, call->timer);
            call->timer = (HTI)RVERROR;
        }
        emaUnlock((EMAElement)hsCall);
    }
}


/************************************************************************
 * autoRasCallIdleState
 * purpose: Set the automatic RAS information of the call to the idle state
 *          This will close any pending transactions for the call.
 * input  : autoras - Automatic RAS handle
 *          call    - Automatic RAS information for the call
 * output : none
 * return : none
 ************************************************************************/
void autoRasCallIdleState(IN autorasEndpoint* autoras, IN autorasCall*   call)
{
    if (call->tx != NULL)
    {
        /* Remove the transaction */
        cmRASClose(call->tx);
        call->tx = NULL;
    }

    if (call->irrTx != NULL)
    {
        /* Remove the transaction */
        cmRASClose(call->irrTx);
        call->irrTx = NULL;
    }

    if (call->timer != (HTI)RVERROR)
    {
        /* Stop the IRR timer */
        mtimerReset(autoras->hTimers, call->timer);
        call->timer = (HTI)RVERROR;
    }
    call->callState = autorasCallIdle;
}


/************************************************************************
 * autoRasIrrTimeout
 * purpose: Callback function which sends an unsolicited IRR on a call
 * input  : context - autoras call handle
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV autoRasIrrTimeout(IN void* context)
{
    autorasCall* call = (autorasCall *)context;

    /* todo: see if cmRASRequest/Confirm works with unsolicited IRRs */
    cmRASRequest(call->irrTx);
}


/************************************************************************
 * autoRasCallTransaction
 * purpose: Callback function used for automatic RAS for dealing with
 *          incoming transactions related to specific calls
 * input  : hApp        - Stack instance handle
 *          hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle
 *          notifyCb    - Callback for application about the request
 * output : none
 * return : TRUE if message was processed by this callback and shouldn't
 *          be processed by the manual RAS callbacks
 *          FALSE if message wasn't processed by this callback
 ************************************************************************/
BOOL RVCALLCONV autoRasCallTransaction(
    IN  HAPP                hApp,
    IN  HRAS                hsRas,
    IN  HCALL               hsCall,
    IN  cmRASTransaction    transaction,
    IN  cmRASTransport*     srcAddress,
    IN  HAPPCALL            haCall,
    IN  cmEvAutoRASRequestT notifyCb)
{
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);
    if (haCall || srcAddress);

    /* Notify the application if we have to */
    if (notifyCb != NULL)
    {
        switch (transaction)
        {
            case cmRASDisengage:
            case cmRASBandwidth:
            case cmRASInfo:
                cmiCBEnter(hApp, "cmEvAutoRASRequest(hsRas=0x%x,hsCall=0x%x,trans=%d)", hsRas, hsCall, transaction);
                notifyCb(hsRas, hsCall, transaction, srcAddress, haCall);
                cmiCBExit(hApp, "cmEvAutoRASRequest(hsRas=0x%x)", hsRas);
                break;
            default:
                /* Not through automatic RAS... */
                break;
        }
    }

    /* See what we've got */
    if (emaLock((EMAElement)hsCall))
    {
        switch (transaction)
        {
            case cmRASDisengage:
            {
                /* DRQ - automatically confirm it */
                autorasEndpoint* autoras;
                autorasCallState state = call->callState;
                call->callState = autorasCallDisconnecting;
                autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

                /* Always send DCF on this one and close the transaction */
                cmRASConfirm(hsRas);
                cmRASClose(hsRas);

                /* We should get back to the idle state of the transaction */
                autoRasCallIdleState(autoras, call);

                if (state != autorasCallIdle)
                {
                    /* Notify application about this situation */
                    autoras->event(hsCall, hsRas, cmiAutoRasEvCallDropForced);
                }
                break;
            }

            case cmRASBandwidth:
            {
                /* BRQ - always confirm it */
                autorasEndpoint* autoras;
                autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));
                autoras->event(hsCall, hsRas, cmiAutoRasEvRateChanged);
                cmRASConfirm(hsRas);
                cmRASClose(hsRas);
                break;
            }

            case cmRASInfo:
            {
                /* IRQ on call - always confirm it */
                cmRASConfirm(hsRas);
                cmRASClose(hsRas);
                break;
            }

            default:
                /* Not processed by automatic RAS */
                return FALSE;
        }
        emaUnlock((EMAElement)hsCall);
    }

    /* Make sure manual RAS knowns we have processed this mesage */
    return TRUE;
}

/************************************************************************
 * autoRasBandwidthResponse
 * purpose: Callback function invoked when a response for BRQ arrives
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasBandwidthResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras;
    autorasCall* call = (autorasCall *)haRas;
    HCALL hsCall;
    int rv=RVERROR;

    if (trStage == cmRASTrStageTimeout)
    {
        cmRASClose(hsRas);
        if (hsRas == call->tx)
            call->tx = NULL;
        return 0;
    }

    hsCall = cmiGetByAutoRas((HAUTORASCALL)call);
    if (emaLock((EMAElement)hsCall))
    {
        autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

        /* Notify the application about this one */
        rv=autoras->event(hsCall, hsRas, cmiAutoRasEvRateChanged);
        emaUnlock((EMAElement)hsCall);
    }
    return rv;
}



/************************************************************************
 * autoRasDisengageResponse
 * purpose: Callback function invoked when a response for DRQ arrives
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasDisengageResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras;
    autorasCall* call = (autorasCall *)haRas;
    HCALL hsCall;
    int rv=RVERROR;

    if (trStage);

    hsCall = cmiGetByAutoRas((HAUTORASCALL)call);
    if (emaLock((EMAElement)hsCall))
    {
        autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

        /* Make sure call is IDLE */
        autoRasCallIdleState(autoras, call);

        /* Notify the application about this one */
        rv=autoras->event(hsCall, hsRas, cmiAutoRasEvCallDropped);
        emaUnlock((EMAElement)hsCall);
    }
    return rv;
}


/************************************************************************
 * autoRasAdmissionResponse
 * purpose: Callback function invoked when a response for ARQ arrives
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasAdmissionResponse(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras;
    autorasCall* call = (autorasCall *)haRas;
    HCALL hsCall;

    hsCall = cmiGetByAutoRas((HAUTORASCALL)call);

    if (emaLock((EMAElement)hsCall))
    {
        autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

        /* See what we've got for the ARQ */
        switch(trStage)
        {
            case cmRASTrStageConfirm:
            {
                /* ACF - go on with the call */
                INT32 irrFrequency;
                int responseId = cmiRASGetResponse(hsRas);

                /* Notify the CM */
                autoras->event(hsCall, hsRas, cmiAutoRasEvGotACF);

                /* Make sure the application was kind enough to leave this transaction and
                   not drop this call barbarically. */
                if (!emaWasDeleted((EMAElement)hsRas))
                {
                    /* Get inside the request message */
                    responseId = pvtChild(autoras->hVal, responseId);

                    /* Check if we have an IRR frequency to work with */
                    if (pvtGetChildValue(autoras->hVal, responseId, __q931(irrFrequency), &irrFrequency, NULL) >= 0)
                    {
                        cmiAutoRASCallSetUnsolicitedIRR(hsCall, irrFrequency);
                    }
                    call->callState = autorasCallConnected;
                    if (((callElem*)hsCall)->state != cmCallStateWaitAddressAck)
                        cmSetupEnd((callElem *)hsCall);
                }
                break;
            }

            case cmRASTrStageReject:
            {
                /* ARJ - notify the CM about it */
                autoras->event(hsCall, hsRas, cmiAutoRasEvCallRejected);
                break;
            }

            case cmRASTrStageTimeout:
            {
                /* timedout... */
                cmElem* app = (cmElem *)emaGetInstance((EMAElement)hsCall);

                autoras->event(hsCall, hsRas, cmiAutoRasEvTimedout);

                /* See if we should try to register again */
                if (pvtGetChild(app->hVal, app->rasConf, __q931(dontRegisterOnTimeout), NULL) < 0)
                {
                    /* We should change the status to not registered */
                    autoras->state = cmIdle;
                    autoRasCallIdleState(autoras, call);

                    /* Make sure we're trying to register again */
                    if ((autoras->internalState != autorasRegTrying) && (autoras->internalState != autorasRegFailed))
                        cmRegister(autoras->hApp);
                }

                break;
            }

            default:
                /* Shouldn't happen */
                break;
        }

        /* Make sure we close this transaction if it wasn't already closed */
        if (call->tx == hsRas)
        {
            cmRASClose(hsRas);
            call->tx = NULL;
        }
        emaUnlock((EMAElement)hsCall);
    }

    return 0;
}



/************************************************************************
 * autoRasPrepareARQ
 * purpose: Prepare and send an ARQ message to start a call (or continue
 *          after receiving a Setup)
 * input  : autoras     - Automatic RAS module
 *          hsCall      - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int autoRasPrepareARQ(IN autorasEndpoint* autoras, IN HCALL hsCall)
{
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);
    cmCallType callType;
    cmAlias alias;
    cmTransportAddress ta;
    char buff[512];
    char partyNumber[256];
    int pnLength = 0;
    INT32 rate;
    int i=0,j=0;
    int ret;
    BOOL outgoingCall = FALSE;

    if (call->tx != NULL)
        cmRASClose(call->tx);

    /* Start an ARQ transaction */
    if (cmRASStartTransaction(autoras->hApp, (HAPPRAS)call, &call->tx, cmRASAdmission, NULL, hsCall) < 0)
    {
        call->tx = NULL;
        return RVERROR;
    }

    /*Copy the required parameters from the call object */

    if (cmCallGetParam(hsCall, cmParamCallType, 0, (INT32 *)&callType, NULL) >= 0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamCallType, 0, callType, NULL);

    cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamCallModel, 0, cmCallModelTypeGKRouted, NULL);

    alias.type = (cmAliasType)0;
    alias.string = buff;
    alias.length = sizeof(buff);

    /* Put the party number in the destination if we're the origin of the call */
    cmCallGetOrigin(hsCall, &outgoingCall);
    if (outgoingCall)
    {
        if (cmCallGetParam(hsCall, cmParamCalledPartyNumber, 0, NULL, (char*)&alias)>=0 && alias.type==cmAliasTypeE164)
        {
            pnLength = min(alias.length, 256); /* CalledPartyNumber cannot be larger than 256 anyway */
            cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamDestInfo, i++, 0, (char*)&alias);
            memcpy(partyNumber, alias.string, pnLength);
        }
    }
    alias.length = sizeof(buff);
    while(cmCallGetParam(hsCall, cmParamDestinationAddress, j++, NULL, (char*)&alias)>=0)
    {
        /* make sure we are not inserting the same number twice */
        if ((pnLength != 0) && (alias.type==cmAliasTypeE164))
        {
            if (!memcmp(partyNumber, alias.string, pnLength))
            {
                /* found the same one twice. don't add this one, and stop checking */
                pnLength = 0;
                continue;
            }
        }
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamDestInfo, i++, 0, (char*)&alias);
    }

    if (cmCallGetParam(hsCall, cmParamRouteCallSignalAddress, 0, NULL, (char*)&ta)>=0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamDestCallSignalAddress, 0, 0, (char*)&ta);
    else
    if (cmCallGetParam(hsCall, cmParamDestCallSignalAddress, 0, NULL, (char*)&ta)>=0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamDestCallSignalAddress, 0, 0, (char*)&ta);

    i=0; j=0;
    alias.length = sizeof(buff);
    while(cmCallGetParam(hsCall, cmParamDestExtraCallInfo, i++, NULL, (char*)&alias)>=0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamDestExtraCallInfo, j++, 0, (char*)&alias);

    i=0; j=0;
    alias.length = sizeof(buff);
    while(cmCallGetParam(hsCall, cmParamSourceAddress, i++, NULL, (char*)&alias)>=0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamSrcInfo, j++, 0, (char*)&alias);

    if (cmCallGetParam(hsCall, cmParamSrcCallSignalAddress, 0, NULL, (char*)&ta)>=0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamSrcCallSignalAddress, 0, 0, (char*)&ta);


    /* Set the bandwidth for this call */
    if (cmCallGetParam(hsCall, cmParamRequestedRate, 0, &rate, NULL) >= 0)
        cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamBandwidth, 0, rate * 2, NULL);

    /* Make sure automatic RAS will get the responses */
    cmiRASSetTrEventHandler(call->tx, autoRasAdmissionResponse);

    /* Send the request */
    ret = cmRASRequest(call->tx);
    if (ret >= 0)
        call->callState = autorasCallConnecting;
    else
    {
        cmRASClose(call->tx);
        call->tx = NULL;
    }

    return ret;
}








/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * cmiAutoRASCallStart
 * purpose: Start the calls part of the automatic RAS.
 *          This function sets internal callbacks with the RAS module and
 *          initializes some autoRAS related variables.
 * input  : hAutoRas - Automatic RAS instance
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASCallStart(IN HAUTORASMGR  hAutoRas)
{
    autorasEndpoint* autoras = (autorasEndpoint *)hAutoRas;

    /* Set the event handler for the endpoint related RAS transactions */
    cmiRASSetCallTrEventHandler(autoras->hApp, autoRasCallTransaction);
}


/************************************************************************
 * cmiAutoRASCallCreate
 * purpose: Create the call information needed for automatic RAS for
 *          a specified call
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallCreate(IN HCALL   hsCall)
{
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);
    memset(call, 0, sizeof(autorasCall));
    call->timer = (HTI)RVERROR;
    call->callState = autorasCallIdle;

    return 0;
}


/************************************************************************
 * cmiAutoRASCallDial
 * purpose: Send an ARQ on a call to start the call with the GK
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallDial(IN HCALL   hsCall)
{
    int ret = 0;
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));

    switch (autoras->internalState)
    {
        case autorasRegistered:
            /* We're registered - send ARQ */
            ret = autoRasPrepareARQ(autoras, hsCall);
            if (ret < 0)
            {
                /* Notify that we can't make this call */
                autoras->event(hsCall, NULL, cmiAutoRasEvFailedOnARQ);
            }
            break;

        case autorasRegFailed:
            /* Endpoint failed to register... */
            autoras->event(hsCall, NULL, cmiAutoRasEvCantRegister);
            break;

        case autorasRegNotTried:
            /* Make sure we're trying to register... */
            ret = cmRegister(autoras->hApp);

            /* No break statement needed here - we want to wait with this call for a while... */

        case autorasRegTrying:
        {
            /* Let's wait a little bit */
            autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);

            if (call->timer != (HTI)RVERROR)
            {
                mtimerReset(autoras->hTimers, call->timer);
                call->timer = (HTI)RVERROR;
            }

            /* Make sure to recheck the registration status every second */
            call->timer = mtimerSet(autoras->hTimers, autoRasWaitRegistration, call, 1000);
            break;
        }
    }

    return ret;
}


/************************************************************************
 * cmiAutoRASCallSetRate
 * purpose: Send a BRQ on a call to change the call bandwidth
 * input  : hsCall  - Stack's call handle
 *          rate    - requested rate for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallSetRate(IN HCALL hsCall, int rate)
{
    int res = 0;
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);

    /* Make sure we're dealing with a connected call */
    if (call->callState == autorasCallConnected)
    {
        /* We should initiate a BRQ */
        if (call->tx != NULL)
            cmRASClose(call->tx);

        res = cmRASStartTransaction(autoras->hApp, (HAPPRAS)call, &call->tx, cmRASBandwidth, NULL, hsCall);
        if (res >= 0)
        {
            /* Make sure automatic RAS will get the responses */
            cmiRASSetTrEventHandler(call->tx, autoRasBandwidthResponse);

            cmRASSetParam(call->tx, cmRASTrStageRequest, cmRASParamBandwidth, 0, rate * 2, NULL);

            /* Send the request */
            res = cmRASRequest(call->tx);
        }
        else
            call->tx = NULL;
    }
    else
        res = RVERROR;

    return res;
}

/************************************************************************
 * cmiAutoRASCallSetUnsolicitedIRR
 * purpose: Initilises send of unsolicited IRRs (used for pregranted ARQ only
 * input  : hsCall - Stack's call handle
 *          irrFrequency - requested frequency ofIRRs
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallSetUnsolicitedIRR(IN HCALL hsCall, int irrFrequency)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);

    if (irrFrequency>0)
    {
        int res;

        /* Create an IRR transaction and fill it in with the call's information
           We're leaving this transaction open as we're sending it on each timeout
           event for the IRR */
        if (call->irrTx != NULL)
            cmRASClose(call->irrTx);

        res = cmRASStartTransaction(autoras->hApp, (HAPPRAS)call, &call->irrTx,
            cmRASUnsolicitedIRR, NULL, hsCall);

        if (res >= 0)
        {
            /* Set a timer for IRR messages sent to the GK */
            call->timer = mtimerSet(autoras->hTimers, autoRasIrrTimeout, call, irrFrequency * 1000);
        }
    }
    return 0;
}



/************************************************************************
 * cmiAutoRASCallDrop
 * purpose: Send a DRQ on a call to start disengaging the call from the GK
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallDrop(IN HCALL hsCall)
{
    int ret = 0;
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);

    /* Make sure we haven't already handled a DRQ for this call - we should ignore it */
    switch(call->callState)
    {
    case autorasCallConnected:
    case autorasCallConnecting:
        /* We should initiate a DRQ, but first let's close all outgoing transactions related
           to this call, as they are not needed anymore and they might cause us to loose
           resources when we are working stressfully. */
        if (call->tx != NULL)
        {
            /* Remove the transaction */
            cmRASClose(call->tx);
            call->tx = NULL;
        }
        /* also remove the IRR tx, to free up RAS transactions. */
        if (call->irrTx != NULL)
        {
            /* Remove the transaction */
            cmRASClose(call->irrTx);
            call->irrTx = NULL;
        }

        ret = cmRASStartTransaction(autoras->hApp, (HAPPRAS)call, &call->tx, cmRASDisengage, NULL, hsCall);
        if (ret >= 0)
        {
            /* Make sure automatic RAS will get the responses */
            cmiRASSetTrEventHandler(call->tx, autoRasDisengageResponse);

            /* Send the request */
            ret = cmRASRequest(call->tx);
        }
        else
            call->tx = NULL;

        if (ret >= 0)
            call->callState = autorasCallDisconnecting;
        else if (call->tx != NULL)
        {
            cmRASClose(call->tx);
            call->tx = NULL;
        }
        break;
    case autorasCallDisconnecting:
        /* do not close the call, wait for DCF */
        return 0;
    case autorasCallIdle:
        /* close the call */
        return RVERROR;
    }

    return ret;
}

/************************************************************************
 * cmiAutoRASCallClose
 * purpose: Free auto ras resources for the call
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallClose(IN HCALL hsCall)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle((HAPP)emaGetInstance((EMAElement)hsCall));
    autorasCall* call = (autorasCall *)cmiGetAutoRas(hsCall);
    autoRasCallIdleState(autoras, call);
    return 0;
}



#ifdef __cplusplus
}
#endif


