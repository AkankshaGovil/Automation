
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************
 * cmAutoRasEP
 * -----------
 * This file provides the endpoint part of the automatic RAS module.
 * This includes the following features:
 *
 * - Automatic discovery and registration to a gatekeeper
 * - Unregistration of a gatekeeper
 * - Lightweight RRQs (timeToLive feature)
 * - Sending NSM and RAI messages
 * - Automatic responses to IRQ messages
 * - Enabling manual RAS to work with automatic RAS for endpoint related
 *   transactions
 ************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include <q931asn1.h>
#include <cmintr.h>
#include <cmras.h>
#include <cmCrossReference.h>
#include <cmAutoRasEP.h>
#include <cmdebprn.h>

/* Private functions declaration */
void RVCALLCONV autoRasSendLightweightRRQ(IN void* context);
void RVCALLCONV autoRasRetry(IN void* context);
int autoRasPrepareRRQ(IN autorasEndpoint* autoras);
int autoRasTryToRegister(IN autorasEndpoint* autoras);
void RVCALLCONV autorasTryToRegisterOnError(IN void* context);
void autoRasIdleAndRegister(IN autorasEndpoint* autoras);
void RVCALLCONV autoRasRetryOnFail(IN autorasEndpoint* autoras);





/************************************************************************
 *
 *                              Private functions
 *
 ************************************************************************/


/************************************************************************
 * autoRasChangeState
 * purpose: Notify the application about the state of the registration
 * input  : autoras     - Automatic RAS instance
 *          regEvent    - Registration event being notified
 *                        can be RVERROR if event is not caused by any message
 *                        but by a timeout
 *          message     - Message that caused the event
 *                        RVERROR if not applicable
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int autoRasChangeState(
    IN  autorasEndpoint*    autoRas,
    IN  cmRegEvent          regEvent,
    IN  int                 message)
{
    /* todo: get cmElem from somewhere else */
    cmEvRegEventT   evFunc;

#ifndef NOLOGSUPPORT
    static const char *autorasStates[] = {"cmIdle", "cmDiscovered", "cmRegistered"};
    static const char *autorasEvents[] =
    {
        "cmGatekeeperConfirm",
        "cmGatekeeperReject",
        "cmRegistrationConfirm",
        "cmRegistrationReject",
        "cmUnregistrationRequest",
        "cmUnregistrationConfirm",
        "cmUnregistrationReject",
        "cmNonStandardMessage",
        "cmResourceAvailabilityConfirmation"
    };
#endif

    /* Notify the application through the callback */
    evFunc = ((cmElem *)autoRas->hApp)->cmMyEvent.cmEvRegEvent;
    if (evFunc != NULL)
    {
        int status;
        if ((int)regEvent == RVERROR)
            message = RVERROR;

#ifndef NOLOGSUPPORT
        cmiCBEnter(autoRas->hApp,
                   "cmEvRegEvent(hApp=0x%x, regState=%s, regEvent=%s, regEventHandle=%d)",
                   autoRas->hApp,
                   nprn(autorasStates[autoRas->state]),
                   nprn(autorasEvents[regEvent]),
                   message);
#endif
        status = evFunc(autoRas->hApp, autoRas->state, regEvent, message);
        cmiCBExit(autoRas->hApp, "cmEvRegEvent=%d", status);
    }

    return 0;
}


/************************************************************************
 * autoRasGatekeeperResponse
 * purpose: Callback function indicating the response of an GRQ
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasGatekeeperResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras = (autorasEndpoint *)haRas;

    /* See what we've got */
    switch(trStage)
    {
        case cmRASTrStageConfirm:
            /* GCF - we should continue to RRQ */
            autoras->discoveryComplete = TRUE;

            /* Change the state ant notify application */
            autoras->state = cmDiscovered;
            autoRasChangeState(autoras, cmGatekeeperConfirm, cmiRASGetResponse(hsRas));

            /* Go on to RRQ */
            autoRasPrepareRRQ(autoras);
            break;

        case cmRASTrStageReject:
        {
            /* GRJ - idle... */
            INT32 isMulticast = FALSE;
            autoras->discoveryComplete = TRUE;

            /* If we're in multicast - we don't continue... */
            cmRASGetParam(hsRas, cmRASTrStageRequest, cmRASParamMulticastTransaction, 0, &isMulticast, NULL);

            /* Change the state and notify application */
            autoras->state = cmIdle;
            autoRasChangeState(autoras, cmGatekeeperReject, cmiRASGetResponse(hsRas));

            if (isMulticast)
            {
                /* Ignore and wait for another reply */
                return 0;
            }

            /* Try all over again */
            autoRasTryToRegister(autoras);
            break;
        }

        case cmRASTrStageTimeout:
        {
            /* We've timed out - try again */
            autoRasIdleAndRegister(autoras);
            return 0;
        }

        default:
            break;
    }

    if(hsRas == autoras->registrationTx)
    {
        /* no one removed this tx yet. kill it. */
        cmRASClose(hsRas);
        autoras->registrationTx = NULL;
    }
    return 0;
}


/************************************************************************
 * autoRasRegistrationResponse
 * purpose: Callback function indicating the response of an RRQ
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasRegistrationResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras = (autorasEndpoint *)haRas;
    int responseId = cmiRASGetResponse(hsRas);
    int messageId;

    /* Skip the type of message and get to the message itself */
    messageId = pvtChild(autoras->hVal, responseId);

    /* See what we've got */
    switch(trStage)
    {
        case cmRASTrStageConfirm:
        {
            /* RCF received - update information and we're done */
            if (pvtGetChild(autoras->hVal, messageId, __q931(timeToLive), NULL) >= 0)
            {
                /* We've got timeToLive - start counting for lightweight RRQs */
                INT32 timeToLive;
                pvtGet(autoras->hVal, pvtGetChild(autoras->hVal, messageId, __q931(timeToLive), NULL), NULL, NULL, &timeToLive, NULL);

                autoras->regTimer = mtimerSet(autoras->hTimers, autoRasSendLightweightRRQ, autoras, max(timeToLive-1, 1) * 1000);
            }

            /* Notify the application about the registration */
            autoras->event(NULL, hsRas, cmiAutoRasEvGotRCF);

            /* Change the state and notify the application */
            autoras->state = cmRegistered;
            autoras->internalState = autorasRegistered; /* We're registered! */
            autoRasChangeState(autoras, cmRegistrationConfirm, responseId);

            /* Initialize the number of tries - when we fail, we should start from the beginning */
            autoras->regTries = 0;
            
            break;
        }

        case cmRASTrStageReject:
        {
            /* RRJ received */
            int chNodeId;

            /* Change the state and notify the application */
            autoras->state = cmIdle;
            autoRasChangeState(autoras, cmRegistrationReject, responseId);
            cmiRASUpdateRegInfo(cmiGetRasHandle(autoras->hApp), TRUE);

            /* Make sure this transaction wasn't killed by the user */
            if (!emaWasDeleted((EMAElement)hsRas))
            {
                /* Check out the reason */
                __pvtGetNodeIdByFieldIds(chNodeId, autoras->hVal, messageId, {_q931(rejectReason) _q931(discoveryRequired) LAST_TOKEN});
                if (chNodeId >= 0)
                {
                    /* We have to try all over again, this time for GRQ */
                    autoras->discoveryRequired = TRUE;
                    autoRasIdleAndRegister(autoras);
                    return 0;
                }
                else
                {
                    /* Timeout and wait a little before trying again */
                    INT32 timeout;
                    pvtGet(autoras->hVal, pvtGetChild(autoras->hVal, autoras->confNode, __q931(responseTimeOut), NULL), NULL, NULL, &timeout, NULL);
                    autoras->regTimer = mtimerSet(autoras->hTimers, autoRasRetry, autoras, timeout * 1000);
                }
            }
            break;
        }

        case cmRASTrStageTimeout:
            /* We've timed out - reset the registration data and try again */
            cmiRASUpdateRegInfo(cmiGetRasHandle(autoras->hApp), TRUE);
            autoRasIdleAndRegister(autoras);
            return 0;

        default:
            break;
    }

    if(hsRas == autoras->registrationTx)
    {
        /* no one removed this tx yet. kill it. */
        cmRASClose(hsRas);
        autoras->registrationTx = NULL;
    }
    return 0;
}


/************************************************************************
 * autoRasUnregistrationResponse
 * purpose: Callback function indicating the response of a URQ
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasUnregistrationResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras = (autorasEndpoint *)haRas;
    cmRegEvent event;

    /* See what we've got - the event will be set accordingly */
    switch(trStage)
    {
        case cmRASTrStageConfirm:
            event = cmUnregistrationConfirm;
            break;
        case cmRASTrStageReject:
            event = cmUnregistrationReject;
            break;
        default:
            event = (cmRegEvent)RVERROR;
            break;
    }

    /* Set the state to IDLE */
    autoras->state = cmIdle;
    autoRasChangeState(autoras, event, cmiRASGetResponse(hsRas));

    if(hsRas == autoras->registrationTx)
    {
        /* no one removed this tx yet. kill it. */
        cmRASClose(hsRas);
        autoras->registrationTx = NULL;
    }
    
    return 0;
}


/************************************************************************
 * autoRasRaiResponse
 * purpose: Callback function indicating the response of a RAI
 * input  : haRas   - Application's handle (autoras in this context)
 *          hsRas   - RAS Transaction handle
 *          trStage - Stage of response
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int RVCALLCONV autoRasRaiResponse
(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage)
{
    autorasEndpoint* autoras = (autorasEndpoint *)haRas;

    /* See what we've got - the event will be set accordingly */
    switch(trStage)
    {
        case cmRASTrStageConfirm:
            autoRasChangeState(autoras, cmResourceAvailabilityConfirmation, cmiRASGetResponse(hsRas));
            break;

        case cmRASTrStageTimeout:
            autoRasChangeState(autoras, cmResourceAvailabilityConfirmation, -1);
            break;

        default:
            break;
    }

    /* Close the transaction and we're done */
    cmRASClose(hsRas);

    return 0;
}


/************************************************************************
 * autoRasSendLightweightRRQ
 * purpose: Callback function which sends an RRQ for keepalive messages
 * input  : context - autoras
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
void RVCALLCONV autoRasSendLightweightRRQ(IN void* context)
{
    autorasEndpoint* autoras = (autorasEndpoint *)context;
    int ret;

    /* Reset the timer - we don't need it until we get an RCF */
    if (autoras->regTimer != (HTI)RVERROR)
    {
        mtimerReset(autoras->hTimers, autoras->regTimer);
        autoras->regTimer = (HTI)RVERROR;
    }

    /* Close a ras transaction if we have one */
    if (autoras->registrationTx != NULL)
    {
        cmRASClose(autoras->registrationTx);
        autoras->registrationTx = NULL;
    }

    /* Create an RRQ transaction */
    ret = cmRASStartTransaction(autoras->hApp, (HAPPRAS)autoras, &autoras->registrationTx, cmRASRegistration, NULL, NULL);

    if (ret >= 0)
    {
        /* Set the parameters to make sure it's a lightweight RRQ */
        cmRASSetParam(autoras->registrationTx, cmRASTrStageRequest, cmRASParamDiscoveryComplete, 0, autoras->discoveryComplete, NULL);
        cmRASSetParam(autoras->registrationTx, cmRASTrStageRequest, cmRASParamKeepAlive, 0, TRUE, NULL);
        autoras->discoveryComplete = FALSE;

        cmiRASSetTrEventHandler(autoras->registrationTx, autoRasRegistrationResponse);

        /* Send the request */
        ret = cmRASRequest(autoras->registrationTx);
    }

    if (ret < 0)
    {
        autoRasRetryOnFail(autoras);
    }
}


/************************************************************************
 * autoRasRetry
 * purpose: Retry callback. Called after a request transaction timedout
 *          and a specified time has passed.
 * input  : context - Context used (automatic RAS handle)
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV autoRasRetry(IN void* context)
{
    autorasEndpoint* autoras = (autorasEndpoint *)context;

    /* Make sure there's a timer to reset */
    if (autoras->regTimer != (HTI)RVERROR)
    {
        mtimerReset(autoras->hTimers, autoras->regTimer);
        autoras->regTimer = (HTI)RVERROR;
    }

    /* Retry after setting to idle */
    autoRasIdleAndRegister(autoras);
}


/************************************************************************
 * autoRasPrepareGRQ
 * purpose: Send a GRQ message to a gatekeeper trying to register it
 * input  : autoras - Automatic RAS instance to register
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int autoRasPrepareGRQ(IN autorasEndpoint* autoras)
{
    int ret;

    /* Make sure we have no pending transactions */
    if (autoras->registrationTx != NULL)
    {
        cmRASClose(autoras->registrationTx);
        autoras->registrationTx = NULL;
    }

    /* Create the transaction - default destination (=gk) */
    ret = cmRASStartTransaction(autoras->hApp, (HAPPRAS)autoras, &autoras->registrationTx, cmRASGatekeeper, NULL, NULL);

    if (ret >= 0)
    {
        /* Set the multicast parameter to indicate this as a multicast message */
        cmRASSetParam(autoras->registrationTx, cmRASTrStageRequest, cmRASParamMulticastTransaction, 0, TRUE, NULL);

        /* Set the response handler for this message */
        cmiRASSetTrEventHandler(autoras->registrationTx, autoRasGatekeeperResponse);

        /* Send the request */
        ret = cmRASRequest(autoras->registrationTx);
    }

    if (ret < 0)
    {
        autoRasRetryOnFail(autoras);
    }

    return ret;
}


/************************************************************************
 * autoRasPrepareRRQ
 * purpose: Send an RRQ message to a gatekeeper trying to register it
 * input  : autoras - Automatic RAS instance to register
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int autoRasPrepareRRQ(IN autorasEndpoint* autoras)
{
    int ret;

    /* Make sure we have no pending transactions */
    if (autoras->registrationTx != NULL)
    {
        cmRASClose(autoras->registrationTx);
        autoras->registrationTx = NULL;
    }
    
    /* Create the transaction - default destination (=gk) */
    ret = cmRASStartTransaction(autoras->hApp, (HAPPRAS)autoras, &autoras->registrationTx, cmRASRegistration, NULL, NULL);

    if (ret >= 0)
    {
        /* Set the parameters of the message */
        cmRASSetParam(autoras->registrationTx, cmRASTrStageRequest, cmRASParamDiscoveryComplete, 0, autoras->discoveryComplete, NULL);
        cmRASSetParam(autoras->registrationTx, cmRASTrStageRequest, cmRASParamKeepAlive, 0, 0, NULL);

        /* Make sure next time we know nothing about a GCF */
        autoras->discoveryComplete = FALSE;

        /* Set the response handler for this message */
        cmiRASSetTrEventHandler(autoras->registrationTx, autoRasRegistrationResponse);

        ret = cmRASRequest(autoras->registrationTx);
    }

    if (ret < 0)
    {
        autoRasRetryOnFail(autoras);
    }

    /* Send the request */
    return ret;
}


/************************************************************************
 * autoRasTryToRegister
 * purpose: Start registration process of the endpoint
 * input  : autoras - Automatic RAS instance to register
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int autoRasTryToRegister(IN autorasEndpoint* autoras)
{
    INT32 maxFail;
    int status;

    if (autoras->internalState != autorasRegFailed)
        autoras->internalState = autorasRegTrying;
    autoras->regTries++; /* another try... */

    /* Make sure we reset the timer if we have any */
    if (autoras->regTimer != (HTI)RVERROR)
    {
        mtimerReset(autoras->hTimers, autoras->regTimer);
        autoras->regTimer = (HTI)RVERROR;
    }

    /* Make sure we have no pending transactions */
    if (autoras->registrationTx != NULL)
    {
        cmRASClose(autoras->registrationTx);
        autoras->registrationTx = NULL;
    }

    /* See if we're done trying */
    if ((pvtGet(autoras->hVal, pvtGetChild(autoras->hVal, autoras->confNode, __q931(maxFail), NULL), NULL, NULL, &maxFail, NULL) < 0) || (maxFail == 0))
        maxFail = 1;
    if ((autoras->regTries > maxFail) && (autoras->internalState != autorasRegFailed))
    {
        autoras->internalState = autorasRegFailed;
        /* We continue on trying to register although we failed - here, we just allow calls
           to be created */

        /* Notify the application that we're not registered although the user wanted to */
        status = autoRasChangeState(autoras, cmRegistrationConfirm, -1);

        /* In case someone called cmUnregister or cmRegister in the callback, do not send another RRQ */
        if ( (autoras->internalState == autorasRegNotTried) || (autoras->internalState == autorasRegTrying) )
            return 0;
    }

    /* See if we're in GRQ or RRQ */
    __pvtGetByFieldIds(status, autoras->hVal, autoras->confNode, {_q931(manualDiscovery) _q931(defaultGatekeeper) LAST_TOKEN}, NULL, NULL, NULL)
    if ((status >= 0) && (!autoras->discoveryRequired))
    {
        /* We've got a GK address that didn't reject us yet - start from RRQ */
        return autoRasPrepareRRQ(autoras);
    }
    else
    {
        /* We don't have any GK address, or were rejected by GK - start from GRQ */
        return autoRasPrepareGRQ(autoras);
    }
}


/************************************************************************
 * autorasTryToRegisterOnError
 * purpose: Timeout function after simulating a timeout on a transaction
 *          that we couldn't get started properly.
 * input  : context - Automatic RAS handle
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV autorasTryToRegisterOnError(IN void* context)
{
    /* The timer itself will be reset in autoRasTryToRegister() */
    autoRasIdleAndRegister((autorasEndpoint *)context);
}


/************************************************************************
 * autoRasRetryOnFail
 * purpose: Retry callback. Called after a RAS request failed (usually 
 *          when someone pulls the cable from the back of the computer)
 * input  : autoras - Automatic RAS instance to register
 * output : none
 * return : none
 ************************************************************************/
void RVCALLCONV autoRasRetryOnFail(IN autorasEndpoint* autoras)
{
    /* We're not going to get any events on this transaction. We might as well
       just simulate waiting for a result and timing-out on that one to continue
       on trying */
    INT32 timeout, retries;

    if (pvtGetChildByFieldId(autoras->hVal, autoras->confNode, __q931(responseTimeOut), &timeout, NULL) < 0)
        timeout = 4000;
    else
        timeout *= 1000;
    if (pvtGetChildByFieldId(autoras->hVal, autoras->confNode, __q931(maxRetries), &retries, NULL) < 0)
        retries = 3;

    if (autoras->regTimer != (HTI)RVERROR)
        mtimerReset(autoras->hTimers, autoras->regTimer);
    autoras->regTimer = mtimerSet(autoras->hTimers, autorasTryToRegisterOnError, autoras, (UINT32)(timeout * retries));
}


/************************************************************************
 * autoRasIdleState
 * purpose: Change the state of the Automatic RAS endpoint handling to
 *          idle and retry registration
 * input  : autoras - Automatic RAS handle
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
void autoRasIdleAndRegister(IN autorasEndpoint* autoras)
{
    /* Change the state back to Idle if we have to */
    if (autoras->state != cmIdle)
    {
        autoras->state = cmIdle;
        autoras->internalState = autorasRegTrying;
        autoRasChangeState(autoras, (cmRegEvent)RVERROR, RVERROR);
        if(autoras->internalState == autorasRegNotTried)
            /* someone decided to unregister in the above event */
            return;
    }

    /* Try to register again */
    autoRasTryToRegister(autoras);
}


/************************************************************************
 * autoRasEPTransaction
 * purpose: Callback function that handles incoming endpoint-related
 *          transactions
 * input  : hApp        - Stack instance handle
 *          hsRas       - RAS transaction handle
 *          transaction - Transaction type of incoming message
 *          srcAddress  - Address of the message's origin
 *          notifyCb    - Callback for application about the request
 * output : none
 * return : TRUE if message was processed by this callback and shouldn't
 *          be processed by the manual RAS callbacks
 *          FALSE if message wasn't processed by this callback
 ************************************************************************/
BOOL RVCALLCONV autoRasEPTransaction(
    IN  HAPP                hApp,
    IN  HRAS                hsRas,
    IN  cmRASTransaction    transaction,
    IN  cmRASTransport*     srcAddress,
    IN  cmEvAutoRASRequestT notifyCb)
{
    if (srcAddress); /* Remove warning from compilation */

    /* Tell the application about this request... */
    if (notifyCb != NULL)
    {
        switch (transaction)
        {
            case cmRASUnregistration:
            case cmRASNonStandard:
            case cmRASInfo:
                cmiCBEnter(hApp, "cmEvAutoRASRequest(hsRas=0x%x,hsCall=0x0,trans=%d)", hsRas, transaction);
                notifyCb(hsRas, NULL, transaction, srcAddress, NULL);
                cmiCBExit(hApp, "cmEvAutoRASRequest(hsRas=0x%x)", hsRas);
                break;
            default:
                /* Not handled by Automatic RAS... */
                break;

        }
    }

    /* Check the type of transaction we're dealing with */
    switch (transaction)
    {
        case cmRASUnregistration:
        {
            /* URQ message - always send back a UCF */
            autorasEndpoint* autoras;
            autoras = (autorasEndpoint *)cmiGetAutoRasHandle(cmiRASGetHAPP(hsRas));
            
            /* First - let's move to idle */
            autoras->state = cmIdle;
            autoRasChangeState(autoras, cmUnregistrationRequest, cmiRASGetRequest(hsRas));

            /* now, let's confirm it */
            cmRASConfirm(hsRas);
            cmRASClose(hsRas);

            /* We're unregistered - we might try to register again :-) */
            if(autoras->internalState == autorasRegNotTried)
                /* someone decided to unregister in the "unregistered" event above */
                break;
            cmiRASUpdateRegInfo(cmiGetRasHandle(hApp), TRUE);
            autoRasIdleAndRegister(autoras);
            break;
        }

        case cmRASNonStandard:
        {
            /* NSM message - notify the state to application and close it */
            autorasEndpoint* autoras;
            autoras = (autorasEndpoint *)cmiGetAutoRasHandle(cmiRASGetHAPP(hsRas));
            autoRasChangeState(autoras, cmNonStandardMessage, cmiRASGetResponse(hsRas));
            cmRASClose(hsRas);
            break;
        }

        case cmRASInfo:
        {
            /* IRQ message - send back an IRR and close the transaction */
            cmRASConfirm(hsRas);
            cmRASClose(hsRas);
            break;
        }

        default:
            /* Not processed by automatic RAS */
            return FALSE;
    }

    /* Make sure manual RAS knowns we have processed this mesage */
    return TRUE;
}





/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/


/************************************************************************
 * cmiAutoRASEPStart
 * purpose: Start the endpoint part of the automatic RAS.
 *          This function sets internal callbacks with the RAS module and
 *          initializes some autoRAS related variables.
 * input  : hAutoRas - Automatic RAS instance
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASEPStart(IN HAUTORASMGR  hAutoRas)
{
    autorasEndpoint* autoras = (autorasEndpoint *)hAutoRas;

    /* Set the event handler for the endpoint related RAS transactions */
    cmiRASSetEPTrEventHandler(autoras->hApp, autoRasEPTransaction);

    autoras->discoveryRequired = FALSE;
}



/************************************************************************
 * cmiAutoRASGetEndpointID
 * purpose: Retrieves the EndpointID stored in the ras 
 *
 * input  : hApp    - Application's stack handle
 *          eId	    - pointer to the buffer for endpoint ID
 *                    buffer should be big enough for longest EID 
 *					  possible (256 byte)
 * output : none
 * return : The length of EID in bytes on success 
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASGetEndpointID(
    IN  HAPP    hApp,
	IN  void*	eId)
{
    HRASMGR ras=cmiGetRasHandle(hApp);
	return cmiRASGetEndpointID(ras,eId);
}




/************************************************************************
 *
 *                          Public API functions
 *
 ************************************************************************/

/************************************************************************
 * cmRegister
 * purpose: Registers the endpoint with the gatekeeper.
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRegister(
        IN  HAPP        hApp)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle(hApp);
    int status;

    if (!hApp || !autoras) return RVERROR;
    cmiAPIEnter(hApp, "cmRegister(hApp=0x%x)", hApp);

    autoras->regTries = 0;
    cmiRASUpdateRegInfo(cmiGetRasHandle(hApp), TRUE);
    status = autoRasTryToRegister(autoras);

    /* Make sure the timer is not started... */
    if ((status < 0) && (autoras->regTimer != (HTI)RVERROR))
    {
        mtimerReset(autoras->hTimers, autoras->regTimer);
        autoras->regTimer = (HTI)RVERROR;
    }

    cmiAPIExit(hApp, "cmRegister: %d", status);
    return status;
}


/************************************************************************
 * cmUnregister
 * purpose: Unregisters the endpoint from the Gatekeeper
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmUnregister(
        IN  HAPP        hApp)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle(hApp);
    int status = 0;

    if (!hApp || !autoras) return RVERROR;
    cmiAPIEnter(hApp, "cmUnregister(hApp=0x%x)", hApp);

    /* Stop the registration timer if we have one */
    if (autoras->regTimer != (HTI)RVERROR)
    {
        mtimerReset(autoras->hTimers, autoras->regTimer);
        autoras->regTimer = (HTI)RVERROR;
    }

    /* Make sure we have no pending transactions */
    if (autoras->registrationTx != NULL)
    {
        cmRASClose(autoras->registrationTx);
        autoras->registrationTx = NULL;
    }

    /* If we're registered - we should unregister... */
    if (autoras->state == cmRegistered)
    {
        status = cmRASStartTransaction(autoras->hApp, (HAPPRAS)autoras, &autoras->registrationTx, cmRASUnregistration, NULL, NULL);
        if (status >= 0)
        {
            cmiRASSetTrEventHandler(autoras->registrationTx, autoRasUnregistrationResponse);
            status = cmRASRequest(autoras->registrationTx);
        }
    }

    autoras->internalState = autorasRegNotTried;
    autoras->regTries = 0;

    cmiAPIExit(hApp, "cmUnregister: %d", status);
    return status;

}


/************************************************************************
 * cmSendNonStandardMessage
 * purpose: Sends a nonstandard message to the Gatekeeper
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp    - Application's handle of the stack
 *          nsParam - Nonstandard parameter to be used in non standard message
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmSendNonStandardMessage(
     IN      HAPP                hApp,
     IN      cmNonStandardParam* nsParam)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle(hApp);
    int status;
    HRAS hRas;

    if (!hApp || !autoras) return RVERROR;
    cmiAPIEnter(hApp, "cmSendNonStandardMessage(hApp=0x%x)", hApp);

    /* Start the transaction and set the non standard information */
    status = cmRASStartTransaction(autoras->hApp, NULL, &hRas, cmRASNonStandard, NULL, NULL);

    if (status >= 0)
    {
        status = cmRASSetParam(hRas, cmRASTrStageRequest, cmRASParamNonStandard, 0, 0, (char*)nsParam);

        if(status >= 0)
        {
            /* Send the request */
            status = cmRASRequest(hRas);
        }

        /* Close the transaction - it's not waiting for any reply */
        cmRASClose(hRas);
    }


    cmiAPIExit(hApp, "cmSendNonStandardMessage: %d", status);
    return status;
}


/************************************************************************
 * cmSendRAI
 * purpose: Sends a RAI message to the gatekeeper.
 *          It is only applicable when the RAS.manualRAS key is not
 *          defined in the configuration (automatic RAS mode).
 *          It reads registration information from RAS.registrationInfo
 *          configuration key.
 * input  : hApp                    - Application's handle of the stack
 *          almoustOutOfResources   - Indicates that it is or is not almost
 *                                    out of resources
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI int RVCALLCONV
cmSendRAI(
       IN      HAPP             hApp,
       IN      BOOL             almostOutOfResources)
{
    autorasEndpoint* autoras = (autorasEndpoint *)cmiGetAutoRasHandle(hApp);
    int status;

    if (!hApp || !autoras) return RVERROR;
    cmiAPIEnter(hApp, "cmSendRAI(hApp=0x%x, almostOutOfResources=%d)", hApp, almostOutOfResources);

    /* Make sure we're already registered */
    if (autoras->state == cmRegistered)
    {
        HRAS hRas;

        /* Send a RAI transaction */
        status = cmRASStartTransaction(autoras->hApp, (HAPPRAS)autoras, &hRas, cmRASResourceAvailability, NULL, NULL);
        if (status >= 0)
        {
            cmRASSetParam(hRas, cmRASTrStageRequest, cmRASParamAlmostOutOfResources, 0, almostOutOfResources, NULL);
            cmiRASSetTrEventHandler(hRas, autoRasRaiResponse);
            status = cmRASRequest(hRas);
        }
    }
    else
        status = RVERROR;

    cmiAPIExit(hApp, "cmSendRAI: %d", status);
    return status;
}




#ifdef __cplusplus
}
#endif


