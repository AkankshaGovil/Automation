
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

#include <cmCrossReference.h>
#include <rasdef.h>
#include <rasutils.h>
#include <rasin.h>
#include <rasout.h>
#include <rasirr.h>
#include <rasparams.h>
#include <cmras.h>
#include <cmdebprn.h>
#include <cmutils.h>


/************************************************************************
 *
 *                              Public functions
 *
 ************************************************************************/

/************************************************************************
 * cmRASStartTransaction
 * purpose: Create a RAS transaction.
 *          This function exchanges handles with the application and connects
 *          between the transaction and the call (if applicable).
 * input  : hApp        - Application's handle for a stack instance
 *          haRas       - Application's handle for the RAS transaction
 *          transaction - The transaction type we want to start
 *          destAddress - Address of the destination.
 *                        If set to NULL, then it's for the gatekeeper
 *          hsCall      - Stack's call handle if the transaction is related
 *                        to a call. NULL otherwise.
 * output : lphsRas     - The stack's RAS transaction handle that was
 *                        created.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASStartTransaction(
    IN  HAPP             hApp,
    IN  HAPPRAS          haRas,
    OUT LPHRAS           lphsRas,
    IN  cmRASTransaction transaction,
    IN  cmRASTransport*  destAddress,
    IN  HCALL            hsCall)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    rasOutTx* tx;
    int ret = 0;

    if (lphsRas != NULL) *lphsRas = NULL;

    cmiAPIEnter(hApp, "cmRASStartTransaction(hApp=0x%x,haRas=0x%x,transaction=%d,hsCall=0x%x)",
                         hApp, haRas, transaction, hsCall);

    /* Create the transaction */
    tx = rasCreateOutTx(ras, haRas, transaction, destAddress);

    /* See what happened */
    if (tx != NULL)
    {
        /* Transaction was created successfuly */
        tx->hsCall = hsCall;
        
        /* IRR transactions get some special treatment */
        if ((transaction == cmRASUnsolicitedIRR) && (hsCall != NULL))
        {
            int irrNode = pvtAdd(ras->hVal, tx->txProperty, __q931(request), 0, NULL, NULL);
            ret = rasSetIrrFields(ras, (HRAS)tx, irrNode, hsCall);
        }

        /* See if we have to set any call related information */
        if ((hsCall != NULL) && (ret >= 0))
        {
            char    callid[16];
            INT32   rasCrv, callidLen;

            callidLen = sizeof(callid);
            if (cmCallGetParam(hsCall, cmParamCallID, 0, &callidLen, callid) >= 0)
                rasSetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCallID, 0, callidLen, callid);

            callidLen = sizeof(callid);
            if (cmCallGetParam(hsCall, cmParamCID, 0, &callidLen, callid) >= 0)
                rasSetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCID, 0, callidLen, callid);

            if (cmCallGetParam(hsCall, cmParamRASCRV, 0, &rasCrv, NULL) >= 0)
                rasSetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamCRV, 0, rasCrv, NULL);

            rasSetParam(ras, (HRAS)tx, cmRASTrStageRequest, cmRASParamAnswerCall, 0, !cmCallGetOrigin(hsCall,NULL), NULL);
       }
    }
    else
        ret = RVERROR;

    /* Check the return value and clear the transaction if we failed somewhere */
    if (ret < 0)
    {
        if (tx != NULL)
        {
            rasCloseOutTx(ras, tx);
            tx = NULL;
        }
    }
    else
    {
        /* Success value should always be 0 for this function */
        ret = 0;
    }

    /* Return this transaction to the application */
    if (lphsRas != NULL) *lphsRas = (HRAS)tx;

    cmiAPIExit(hApp, "cmRASStartTransaction(haRas=0x%x,hsRas=0x%x,ret=%d)",
                        haRas, tx, ret);
    return ret;
}


/************************************************************************
 * cmRASSetHandle
 * purpose: Sets or changes the application handle for a RAS transaction.
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          haRas   - Application's handle for the RAS transaction to be set
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetHandle(
    IN  HRAS            hsRas,
    IN  HAPPRAS         haRas)
{
    rasModule* ras;
    int ret;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASSetHandle(hsRas=0x%x,haRas=0x%x)", hsRas, haRas);

    /* Update the application's handle */
    ret = emaSetApplicationHandle((EMAElement)hsRas, (void*)haRas);

    cmiAPIExit(ras->app, "cmRASSetHandle(hsRas=0x%x)=%d", hsRas, ret);
    return ret;
}


/************************************************************************
 * cmRASGetParam
 * purpose: Get a parameter about the RAS transaction
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - If the parameter value is a structure, the value
 *                    represents the length of the parameter.
 * output : value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself. Can be set to NULL if we're only interested
 *                    in its length.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetParam(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  OUT INT32*       value,
    IN  char*            svalue)
{
    rasModule* ras;
    int status = -1;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app,
                         "cmRASGetParam(hsRas=0x%x,trStage=%d,%s[%d])",
                         hsRas, trStage, nprn(rasGetParamName(param)), index);

    if(emaLock((EMAElement)hsRas))
    {
        status = rasGetParam(ras, hsRas, trStage, param, index, value, svalue);
        emaUnlock((EMAElement)hsRas);
    }

    cmiAPIExit(ras->app, "cmRASGetParam(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASSetParam
 * purpose: Set a parameter about the RAS transaction
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 *          index   - If the parameter has several instances, the index
 *                    that identifies the specific instance (0-based).
 *                    0 otherwise.
 *          value   - For a simple integer - the parameter's value.
 *                    For a structure - the length of the parameter.
 *          svalue  - For a structure - svalue represents the parameter
 *                    itself.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetParam(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param,
    IN  int              index,
    IN  INT32            value,
    IN  char*            svalue)
{
    rasModule* ras;
    int status = -1;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app,
                         "cmRASSetParam(hsRas=0x%x,trStage=%d,%s[%d]=%d)",
                         hsRas, trStage, nprn(rasGetParamName(param)), index, value);

    if(emaLock((EMAElement)hsRas))
    {
        status = rasSetParam(ras, hsRas, trStage, param, index, value, svalue);
        emaUnlock((EMAElement)hsRas);
    }
    if (status > 0)
        status = 0; /* We want the gatekeeper to work, so we set successful return values to 0 */

    cmiAPIExit(ras->app, "cmRASSetParam(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASGetNumOfParams
 * purpose: Returns the number of params in sequences on the property
 *          tree.
 * input  : hsRas   - Stack's handle for the RAS transaction
 *          trStage - The transaction stage the parameters
 *          param   - Type of the RAS parameter
 * output : none
 * return : Number of params in sequence on the property tree on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetNumOfParams(
    IN  HRAS             hsRas,
    IN  cmRASTrStage     trStage,
    IN  cmRASParam       param)
{
    rasModule* ras;
    int status = -1;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app,
                         "cmRASGetNumOfParams(hsRas=0x%x,trStage=%d,%s)",
                         hsRas, trStage, nprn(rasGetParamName(param)));

    if(emaLock((EMAElement)hsRas))
    {
        status = rasGetNumOfParams(ras, hsRas, trStage, param);
        emaUnlock((EMAElement)hsRas);
    }
    cmiAPIExit(ras->app, "cmRASGetNumOfParams(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASRequest
 * purpose: Send an outgoing RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASRequest(
    IN  HRAS             hsRas)
{
    rasModule*  ras;
    rasOutTx*   tx;
    int         status;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASRequest(hsRas=0x%x)", hsRas);

    /* Get the transaction */
    tx = rasGetOutgoing(hsRas);
    if (tx != NULL)
        status = rasSendRequestMessage(ras, tx);
    else
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "cmRASRequest: Bad outgoing transaction handle (0x%x)", hsRas));
        status = RVERROR;
    }

    cmiAPIExit(ras->app, "cmRASRequest(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASDummyRequest
 * purpose: Called after cmRASStartTransaction() on cmRASUnsolicitedIRR.
 *          It allows the application to wait for an unsolicited IRR on
 *          a specific call.
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASDummyRequest(
    IN  HRAS         hsRas)
{
    rasModule*  ras;
    rasOutTx*   tx;
    int         status;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASDummyRequest(hsRas=0x%x)", hsRas);

    /* Get the transaction */
    tx = rasGetOutgoing(hsRas);
    if (tx != NULL)
        status = rasDummyRequest(ras, tx);
    else
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "cmRASDummyRequest: Bad outgoing transaction handle (0x%x)", hsRas));
        status = RVERROR;
    }

    cmiAPIExit(ras->app, "cmRASDummyRequest(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASConfirm
 * purpose: Sends a confirm response on an incoming RAS request
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASConfirm(
    IN  HRAS             hsRas)
{
    rasModule*  ras;
    rasInTx*    tx;
    int         status;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASConfirm(hsRas=0x%x)", hsRas);

    /* Get the transaction */
    tx = rasGetIncoming(hsRas);
    if (tx != NULL)
        status = rasSendConfirmMessage(ras, tx);
    else
    {
        rasOutTx* tx = rasGetOutgoing(hsRas);
        /* check if this is an unsolicitated IRR without an incoming transaction */
        if ((tx != NULL) && (tx->transactionType == cmRASUnsolicitedIRR))
        {
            /* use "cmRASRequest()" instead, done for backwards competability */
            status = cmRASRequest(hsRas);
        }
        else
        {
            logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "cmRASConfirm: Bad outgoing transaction handle (0x%x)", hsRas));
            status = RVERROR;
        }
    }

    cmiAPIExit(ras->app, "cmRASConfirm(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASReject
 * purpose: Sends a reject response on an incoming RAS request
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          reason      - The reject reason to use
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASReject(
    IN  HRAS             hsRas,
    IN  cmRASReason      reason)
{
    rasModule*  ras;
    rasInTx*    tx;
    int         status;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASReject(hsRas=0x%x)", hsRas);

    /* Get the transaction */
    tx = rasGetIncoming(hsRas);
    if (tx != NULL)
        status = rasSendRejectMessage(ras, tx, reason);
    else
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "cmRASReject: Bad outgoing transaction handle (0x%x)", hsRas));
        status = RVERROR;
    }

    cmiAPIExit(ras->app, "cmRASReject(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASInProgress
 * purpose: Sends a RIP (ReplyInProgress) message on a transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          delay       - Delay to use in RIP message (in milliseconds)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASInProgress(
    IN  HRAS         hsRas,
    IN  int          delay)
{
    rasModule*  ras;
    rasInTx*    tx;
    int         status = 0;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASInProgress(hsRas=0x%x, delay=%d)", hsRas, delay);

    /* Get the transaction */
    tx = rasGetIncoming(hsRas);
    if (tx != NULL)
        status = rasSendRIP(ras, tx, delay, TRUE);
    else
    {
        logPrint(ras->log, RV_ERROR,
                 (ras->log, RV_ERROR,
                 "cmRASInProgress: Bad outgoing transaction handle (0x%x)", hsRas));
        status = RVERROR;
    }

    cmiAPIExit(ras->app, "cmRASInProgress(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}


/************************************************************************
 * cmRASClose
 * purpose: Close a RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASClose(
    IN  HRAS             hsRas)
{
    rasModule*  ras;
    int         status = 0;
    if (hsRas == NULL) return RVERROR;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASClose(hsRas=0x%x)", hsRas);

    /* Check if it's an incoming or an outgong transaction */
    switch (emaGetType((EMAElement)hsRas))
    {
        case RAS_OUT_TX:
        {
            rasOutTx* tx;
            tx = rasGetOutgoing(hsRas);

            if (tx != NULL)
                status = rasCloseOutTx(ras, tx);
            else
            {
                logPrint(ras->log, RV_ERROR,
                         (ras->log, RV_ERROR,
                         "cmRASClose: Bad outgoing transaction handle (0x%x)", hsRas));
                status = RVERROR;
            }
            break;
        }
        case RAS_IN_TX:
        {
            rasInTx* tx;
            tx = rasGetIncoming(hsRas);

            if (tx != NULL)
                status = rasCloseInTx(ras, tx);
            else
            {
                logPrint(ras->log, RV_ERROR,
                         (ras->log, RV_ERROR,
                         "cmRASClose: Bad incoming transaction handle (0x%x)", hsRas));
                status = RVERROR;
            }
            break;
        }
        default:
            status = RVERROR;
    }

    cmiAPIExit(ras->app, "cmRASClose(hsRas=0x%x,ret=%d)", hsRas, status);
    return status;
}






/************************************************************************
 * cmRASGetHandle
 * purpose: Returns the stack's handle of the transaction from the
 *          application's handle.
 *          This function is slow and should not be used frequently
 * input  : hApp        - Application's handle for a stack instance
 *          haRas       - Application's handle for the RAS transaction
 * output : lphsRas     - The stack's RAS transaction handle
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetHandle(
    IN  HAPP    hApp,
    IN  HAPPRAS haRas,
    OUT LPHRAS  hsRas)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    HRAS hRas;

    /* Search in incoming transactions */
    hRas = NULL;
    while ( (hRas = (HRAS)emaGetNext(ras->inRa, (EMAElement)hRas)) )
        if ((HAPPRAS)emaGetApplicationHandle((EMAElement)hRas) == haRas)
        {
            if (hsRas) *hsRas = hRas;
            return 0;
        }

    /* Search in outgoing transactions */
    while ( (hRas = (HRAS)emaGetNext(ras->outRa, (EMAElement)hRas)) )
        if ((HAPPRAS)emaGetApplicationHandle((EMAElement)hRas) == haRas)
        {
            if (hsRas) *hsRas = hRas;
            return 0;
        }

    /* If we're here, then we haven't found a matching transaction */
    return RVERROR;
}


/************************************************************************
 * cmRASGetTransaction
 * purpose: Returns the type of RAS transaction
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : transaction - The type of transaction
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASGetTransaction(
    IN  HRAS                hsRas,
    OUT cmRASTransaction*   transaction)
{
    rasModule*  ras;
    int         status = 0;

    ras = (rasModule *)emaGetUserData((EMAElement)hsRas);
    cmiAPIEnter(ras->app, "cmRASGetTransaction(hsRas=0x%x)", hsRas);

    /* Get the transaction type */
    if(emaLock((EMAElement)hsRas))
    {
        /* Check if it's an incoming or an outgong transaction */
        switch (emaGetType((EMAElement)hsRas))
        {
        case RAS_OUT_TX:
            {
                rasOutTx* tx;
                tx = rasGetOutgoing(hsRas);
                
                if (tx != NULL)
                    *transaction = tx->transactionType;
                else
                {
                    logPrint(ras->log, RV_ERROR,
                        (ras->log, RV_ERROR,
                            "cmRASGetTransaction: Bad outgoing transaction handle (0x%x)", hsRas));
                    status = RVERROR;
                }
                break;
            }
        case RAS_IN_TX:
            {
                rasInTx* tx;
                tx = rasGetIncoming(hsRas);
                
                if (tx != NULL)
                    *transaction = tx->transactionType;
                else
                {
                    logPrint(ras->log, RV_ERROR,
                        (ras->log, RV_ERROR,
                            "cmRASGetTransaction: Bad incoming transaction handle (0x%x)", hsRas));
                    status = RVERROR;
                }
                break;
            }
        default:
            status = RVERROR;
        }
        
        emaUnlock((EMAElement)hsRas);
    }
    cmiAPIExit(ras->app, "cmRASGetTransaction(hsRas=0x%x,tx=%d,ret=%d)",
                        hsRas, *transaction, status);
    return status;
}


/************************************************************************
 * cmRASGetLastError
 * purpose: This function does absolutly nothing.
 *          It is only here for backward compatibility.
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : 0
 ************************************************************************/
RVAPI
cmRASError RVCALLCONV cmRASGetLastError(
    IN  HRAS             hsRas)
{
    if (hsRas);
    return (cmRASError)0;
}



/************************************************************************
 * cmRASSetEventHandler
 * purpose: Sets the callbacks the application wishes to use
 * input  : hApp        - Application's handle for a stack instance
 *          cmRASEvent  - RAS callbacks to set
 *          size        - Size of callbacks struct (*CMRASEVENT)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmRASSetEventHandler(
    IN  HAPP hApp,
    IN  CMRASEVENT cmRASEvent,
    IN  int size)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);

    if (hApp == NULL) return RVERROR;

    cmiAPIEnter(hApp, "cmRASSetEventHandler(hApp=0x%x,size=%d)", hApp, size);

    memset(&ras->evApp, 0, sizeof(SCMRASEVENT));
    memcpy(&ras->evApp, cmRASEvent, min(size, (int)sizeof(SCMRASEVENT)));

    cmiAPIExit(hApp, "cmRASSetEventHandler(hApp=0x%x,ret=0)", hApp);
    return 0;
}


/************************************************************************
 * cmAutoRASSetEventHandler
 * purpose: Sets the callbacks the application wishes to use for automatic
 *          RAS. Catching these callbacks allows the application to
 *          know about the messages that the automatic RAS receives.
 *          It doesn't allow the application to act on them - only to
 *          know about them.
 * input  : hApp            - Application's handle for a stack instance
 *          cmAutoRASEvent  - Automatic RAS callbacks to set
 *          size            - Size of callbacks struct (*CMRASEVENT)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
RVAPI
int RVCALLCONV cmAutoRASSetEventHandler(
    IN  HAPP            hApp,
    IN  CMAUTORASEVENT  cmAutoRASEvent,
    IN  int             size)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);

    if (hApp == NULL) return RVERROR;

    cmiAPIEnter(hApp, "cmAutoRASSetEventHandler(hApp=0x%x,size=%d)", hApp, size);

    memset(&ras->evAutoRas, 0, sizeof(SCMAUTORASEVENT));
    memcpy(&ras->evAutoRas, cmAutoRASEvent, min(size, (int)sizeof(SCMAUTORASEVENT)));

    cmiAPIExit(hApp, "cmAutoRASSetEventHandler(hApp=0x%x,ret=0)", hApp);
    return 0;
}




RVAPI
INT32 RVCALLCONV cmGetGKCallSignalAddress(
                                        IN  HAPP             hApp,
                                        OUT     cmTransportAddress*  tr)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    int rc;
    if (hApp == NULL) return RVERROR;

    cmiAPIEnter(hApp, (char*)"cmGetGKCallSignalAddress: hApp=0x%x.", hApp);
    rc= cmVtToTA(ras->hVal,ras->gatekeeperCallSignalAddress, tr);
    cmiAPIExit(hApp, (char*)"cmGetGKCallSignalAddress: [%d].", rc);
    return rc;
}

RVAPI
INT32 RVCALLCONV cmGetGKRASAddress(
        IN  HAPP                hApp,
        OUT cmTransportAddress* tr)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    int rc;
    if (hApp == NULL) return RVERROR;

    cmiAPIEnter(hApp, (char*)"cmGetGKRASAddress: hApp=0x%x.", hApp);
    rc = cmVtToTA(ras->hVal,ras->gatekeeperRASAddress, tr);
    cmiAPIExit(hApp, (char*)"cmGetGKRASAddress: [%d].", rc);
    return rc;
}

/* todo: comment */
RVAPI
HPROTCONN RVCALLCONV cmGetRASConnectionHandle(
                IN  HAPP             hApp)
{
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    if (hApp == NULL) return NULL;

    cmiAPIEnter(hApp, (char*)"cmGetRASConnectionHandle: hApp=0x%x.", hApp);
    cmiAPIExit(hApp, (char*)"cmGetRASConnectionHandle: [OK].");
    return (HPROTCONN)&(ras->unicastAppHandle);
}


RVAPI
HPROTCONN RVCALLCONV cmGetUdpChanHandle(    IN HCALL hsCall,cmUdpChanHandleType udpChanHandleType)
{
    HPROTCONN hProtCon=NULL;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    rasModule* ras = (rasModule *)cmiGetRasHandle(hApp);
    if (hApp == NULL) return NULL;

    cmiAPIEnter(hApp,(char*)"cmGetUdpChanHandle: hsCall=0x%x udpChanHandleType = %d",hsCall,udpChanHandleType);
    switch(udpChanHandleType)
    {
        case    cmRasUdpChannel:
            hProtCon =  (HPROTCONN)&(ras->unicastAppHandle);
        break;
        case    cmRasUdpChannelMCast:
            hProtCon =  (HPROTCONN)&(ras->multicastAppHandle);
        break;
    }
    cmiAPIExit(hApp,(char*)"cmGetUdpChanHandle: hProtConn 0x%x",hProtCon);

    return hProtCon;
}

RVAPI
int RVCALLCONV cmSetUdpChanApplHandle(  IN HPROTCONN hCon,HAPPCONN hAppConn)
{
    /* hCon is actually a pointer to (ras->unicastAppHandle) or (ras->multicastAppHandle). if we want
    to set the value of the uni/multicastAppHandle, we should set the value of *hCon to hAppCon. */
    *(HAPPCONN*)hCon=hAppConn;
    return TRUE;
}

RVAPI
int RVCALLCONV cmGetUdpChanApplHandle(  IN HPROTCONN hCon,HAPPCONN * hAppConn)
{
    /* hCon points to (ras->unicastAppHandle) or (ras->multicastAppHandle). if we want the Application
    handle, we should set *hAppCon to the value of *hCon. */
    if (!hCon)
    {
        *hAppConn=NULL;
        return RVERROR;
    }

    *hAppConn = *(HAPPCONN*)hCon;
    return 0;

}


#ifdef __cplusplus
}
#endif


