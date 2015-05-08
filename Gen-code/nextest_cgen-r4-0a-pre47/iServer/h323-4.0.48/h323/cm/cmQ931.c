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


#include <mti.h>
#include <cm.h>
#include <cmintr.h>
#include <q931asn1.h>
#include <cmQ931.h>
#include <cmiQ931.h>
#include <cmCrossReference.h>
#include <cmCall.h>
#include <cmParam.h>



/***************************/
/*  I  N  I  T  /  E  N  D */
/***************************/
int q931CallCreate(HQ931 call, int t301, int t302, int t303, int t304, int t310, int t322)
{
    q931Elem* callE=(q931Elem*)call;
    callE->t301=t301;
    callE->t302=t302;
    callE->t303=t303;
    callE->t304=t304;
    callE->t310=t310;
    callE->t322=t322;
    callE->callState=cs_Null;
    callE->timer=(HTI)RVERROR;
    callE->timerSE=(HTI)RVERROR;
    return TRUE;
}
int q931CallClose(HQ931 call)
{
    q931Elem* callE=(q931Elem*)call;
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timerSE));
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
    return 0;
}

callStateE q931GetCallState(HQ931 call)
{
    q931Elem* callE=(q931Elem*)call;
    return callE->callState;
}
/**********************/
/*  T  I  M  E  R  S  */
/**********************/

/* t301 - connectTimeOut (outgoing call, after alerting received) */
void RVCALLCONV q931T301Timeout(void* context)
{
    HQ931 call=(HQ931)context;
    q931Elem* callE=(q931Elem*)call;

    if (emaLock((EMAElement)cmiGetByQ931(call)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        q931CallDrop(call,-1);
        cmIndicate(cmiGetByQ931(call),-1, -1);
        if (!emaWasDeleted((EMAElement)cmiGetByQ931(call)))
            callE->callState=cs_Null;
        emaUnlock((EMAElement)cmiGetByQ931(call));
    }
}

/* t302 - responseTimeOut or t302 (incoming call waiting for incomplete address to complete) */
void RVCALLCONV q931T302Timeout(void* context)
{
    HQ931 call=(HQ931)context;
    q931Elem* callE=(q931Elem*)call;

    if (emaLock((EMAElement)cmiGetByQ931(call)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        q931CallDrop(call,-1);
        cmIndicate(cmiGetByQ931(call),-1, -1);
        if (!emaWasDeleted((EMAElement)cmiGetByQ931(call)))
            callE->callState=cs_Null;
        emaUnlock((EMAElement)cmiGetByQ931(call));
    }
}

/* t303 - responseTimeOut (outgoing call after dial) */
void RVCALLCONV q931T303Timeout(void* context)
{
    HQ931 call=(HQ931)context;
    q931Elem* callE=(q931Elem*)call;

    if (emaLock((EMAElement)cmiGetByQ931(call)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        q931CallDrop(call,-1);
        cmIndicate(cmiGetByQ931(call),-1, -1);
        if (!emaWasDeleted((EMAElement)cmiGetByQ931(call)))
            callE->callState=cs_Null;
        emaUnlock((EMAElement)cmiGetByQ931(call));
    }
}

/* t304 - responseTimeOut or t304 (outgoing call in overlap sending process) */
void RVCALLCONV q931T304Timeout(void* context)
{
    HQ931 call=(HQ931)context;
    q931Elem* callE=(q931Elem*)call;

    if (emaLock((EMAElement)cmiGetByQ931(call)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        q931CallDrop(call,-1);
        cmIndicate(cmiGetByQ931(call),-1, -1);
        if (!emaWasDeleted((EMAElement)cmiGetByQ931(call)))
            callE->callState=cs_Null;
        emaUnlock((EMAElement)cmiGetByQ931(call));
    }
}

/* t310 - connectTimeOut (outgoing call after callProceeding received) */
void RVCALLCONV q931T310Timeout(void* context)
{
    HQ931 call=(HQ931)context;
    q931Elem* callE=(q931Elem*)call;

    if (emaLock((EMAElement)cmiGetByQ931(call)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        q931CallDrop(call,-1);
        cmIndicate(cmiGetByQ931(call),-1, -1);
        if (!emaWasDeleted((EMAElement)cmiGetByQ931(call)))
            callE->callState=cs_Null;
        emaUnlock((EMAElement)cmiGetByQ931(call));
    }
}

 /* t322 - 100 seconds (outgoing call waiting for reply to status inquiery) */
void RVCALLCONV q931T322Timeout(void* context)
{
    q931Elem* callE=(q931Elem*)context;
    if (emaLock((EMAElement)cmiGetByQ931((HQ931)callE)))
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timerSE));
        emaUnlock((EMAElement)cmiGetByQ931((HQ931)callE));
    }
}

/*************************/
/*  A  C  T  I  O  N  S  */
/*************************/
int q931CallDial(HQ931 call, int message)
{
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Null)
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),q931T303Timeout,callE,callE->t303);
        sendCallMessage(cmiGetByQ931((HQ931)callE),message);
        callReleaseMessage(cmiGetByQ931((HQ931)callE),cmQ931setup);
        callE->callState=cs_Call_initiated;
    }
    return 0;
}
int q931CallInfo(HQ931 call, int message)
{
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Overlap_sending)
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),q931T304Timeout,callE,callE->t304);
        sendCallMessage(cmiGetByQ931(call),message);
        callReleaseMessage(cmiGetByQ931(call),cmQ931information);
    }
    else
    if (callE->callState!=cs_Null &&
        callE->callState!=cs_Call_initiated &&
        callE->callState!=cs_Call_present)
    {
        sendCallMessage(cmiGetByQ931(call),message);
        callReleaseMessage(cmiGetByQ931(call),cmQ931information);
    }
    return 0;
}
int q931CallMoreInfo(HQ931 call,int message)
{
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Call_present)
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),q931T302Timeout,callE,callE->t302);
        if (message<0)
            message=callGetMessage(cmiGetByQ931(call),cmQ931setupAcknowledge);
        sendCallMessage(cmiGetByQ931(call),message);
        callReleaseMessage(cmiGetByQ931(call),cmQ931setupAcknowledge);
        callE->callState=cs_Overlap_receiving;
    }
    return 0;
}


/************************************************************************
 * q931CallCallProceeding
 * purpose: Send Q931 CallProceeding message on a call
 * input  : call    - Stack handle for the Q931 call
 *          message - CallProceeding message to send
 *                    if set to -1, then create the message from the call's
 *                    property database
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallCallProceeding(IN HQ931 call, IN int message)
{
    int status = RVERROR;
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Call_present||
        callE->callState==cs_Overlap_receiving)
    {
        /* Get the message to send */
        if (message<0)
            message=callGetMessage(cmiGetByQ931(call),cmQ931callProceeding);

        /* Reset the call's Q931 timer */
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));

        /* Send the Progress message */
        status = sendCallMessage(cmiGetByQ931(call),message);

        /* Release the message if not needed anymore */
        callReleaseMessage(cmiGetByQ931(call),cmQ931callProceeding);

        /* Change call's Q931 state to IncomingCallProceeding */
        callE->callState=cs_Incoming_call_proceeding;
    }
    return status;
}


/************************************************************************
 * q931CallProgress
 * purpose: Send Q931 Progress message on a call
 * input  : call    - Stack handle for the Q931 call
 *          message - Progress message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallProgress(IN HQ931 call, IN int message)
{
    int         status = RVERROR;
    q931Elem*   callE  = (q931Elem*)call;
    HPVT        hVal   = cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)));

    if (callE->callState==cs_Call_present ||
        callE->callState==cs_Incoming_call_proceeding ||
        callE->callState==cs_Overlap_receiving ||
        callE->callState==cs_Call_received)
    {
        /* Send the Progress message */
        status = sendCallMessage(cmiGetByQ931(call),message);

        /* Release the message if not needed anymore */
        pvtDelete(hVal, message);
        callReleaseMessage(cmiGetByQ931(call),cmQ931progress);
    }
    return status;
}


/************************************************************************
 * q931CallNotify
 * purpose: Send Q931 Notify message on a call
 * input  : call    - Stack handle for the Q931 call
 *          message - Notify message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallNotify(IN HQ931 call, IN int message)
{
    int       result = RVERROR;
    HPVT      hVal = cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)));

    /* Send the Notify message */
    result = sendCallMessage(cmiGetByQ931(call),message);

    /* Release the message if not needed anymore */
    pvtDelete(hVal, message);
    callReleaseMessage(cmiGetByQ931(call),cmQ931notify);

    return result;
}


/************************************************************************
 * q931CallAccept
 * purpose: Send Q931 Alerting message on a call
 * input  : call    - Stack handle for the Q931 call to accept
 *          message - Alerting message to send
 *                    if set to -1, then create the message from the call's
 *                    property database
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallAccept(IN HQ931 call, IN int message)
{
    int status = RVERROR;
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Call_present||
        callE->callState==cs_Incoming_call_proceeding||
        callE->callState==cs_Overlap_receiving)
    {
        /* Get the message to send */
        if (message<0)
            message=callGetMessage(cmiGetByQ931(call),cmQ931alerting);

        /* Reset the call's Q931 timer */
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));

        /* Send the Alerting message */
        status = sendCallMessage(cmiGetByQ931(call),message);

        /* Release the message if not needed anymore */
        callReleaseMessage(cmiGetByQ931(call),cmQ931alerting);

        /* Change call's Q931 state to CallReceived */
        callE->callState=cs_Call_received;
    }
    return status;
}


/************************************************************************
 * q931CallAnswer
 * purpose: Send Q931 Connect message on a call
 * input  : call    - Stack handle for the Q931 call to connect
 *          message - Connect message to send
 *                    if set to -1, then create the message from the call's
 *                    property database
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallAnswer(IN HQ931 call, IN int message)
{
    int status = RVERROR;
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Call_present||
        callE->callState==cs_Call_received||
        callE->callState==cs_Incoming_call_proceeding||
        callE->callState==cs_Overlap_receiving)
    {
        /* Get the message to send */
        if (message<0)
            message=callGetMessage(cmiGetByQ931(call),cmQ931connect);

        /* Reset the call's Q931 timer */
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));

        /* Send the Connect message */
        status = sendCallMessage(cmiGetByQ931(call),message);

        /* Release the message if not needed anymore */
        callReleaseMessage(cmiGetByQ931(call),cmQ931connect);

        /* Change call's Q931 state to Active */
        callE->callState=cs_Active;
    }
    return status;
}


/************************************************************************
 * q931CallDrop
 * purpose: Drops a Q931 connection of a call
 * input  : call        - Stack handle for the Q931 call
 *          message     - Release message to send
 *                        If -1, then message will be created
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931CallDrop(IN HQ931 call, IN int message)
{
    int status = 0;
    q931Elem* callE=(q931Elem*)call;

    /*Do this only if something already has happened on this call*/
    if (callE->callState!=cs_Null)
    {
        /* Get the message to send */
        if (message<0)
            message=callGetMessage(cmiGetByQ931(call),cmQ931releaseComplete);
        /* Reset the call's Q931 timer */
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timer));

        /* Send the ReleaseComplete message */
        status = sendCallMessage(cmiGetByQ931(call),message);

        /* Release the message if not needed anymore */
        callReleaseMessage(cmiGetByQ931(call),cmQ931releaseComplete);

        /* Change call's Q931 state to Null */
        callE->callState=cs_Null;
    }
    return status;
}

int q931CallStatusEnquiry(HQ931 call,int message)
{
    q931Elem* callE=(q931Elem*)call;
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),&(callE->timerSE));
    callE->timerSE=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)),q931T322Timeout,callE,callE->t322);
    sendCallMessage(cmiGetByQ931(call),message);
    callReleaseMessage(cmiGetByQ931(call),cmQ931statusEnquiry);
    return 0;
}

int q931CallFacility(HQ931 call,int message)
{
    int  ret = 0;
    int  q931NodeId, reasonNodeId, facilityNodeId, emptyNodeId;
    INT32  reasonId;
    HPVT hVal=cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)));

    if (message<0)
        message=callGetMessage(cmiGetByQ931(call),cmQ931facility);

    /* check if we have the Q.931 facility IE */
    __pvtGetNodeIdByFieldIds(facilityNodeId,hVal,message,
                {_q931(message) 
                 _q931(facility) 
                 _q931(facility) 
                 LAST_TOKEN});

    if (facilityNodeId < 0)
    {
        /* build the Q.931 facility IE */
        __pvtBuildByFieldIds(facilityNodeId,hVal,message,
                            {_q931(message) 
                             _q931(facility) 
                             _q931(facility) 
                             LAST_TOKEN},0,NULL);
        if (facilityNodeId < 0)
          return -1;
    }

    /* check if we have a reason (valid only in case of non-empty facility message) */    
    q931NodeId=pvtChild(hVal,pvtGetChild(hVal,message,__q931(message),NULL));

    __pvtGetNodeIdByFieldIds(emptyNodeId,hVal,q931NodeId,
                {_q931(userUser) _q931(h323_UserInformation) 
                 _q931(h323_uu_pdu) _q931(h323_message_body) 
                 _q931(empty) LAST_TOKEN});

    if (emptyNodeId < 0)
    {
        __pvtGetNodeIdByFieldIds(reasonNodeId,hVal,q931NodeId,
                    {_q931(userUser) _q931(h323_UserInformation) 
                     _q931(h323_uu_pdu) _q931(h323_message_body) 
                     _q931(facility) _q931(reason) LAST_TOKEN});

        /* if we dont have it take the reason from the parameter of the call */
        if (reasonNodeId < 0)
        {
            cmCallGetParam(cmiGetByQ931(call), cmParamFacilityReason, 0, &reasonId, NULL);

            if (reasonId >= 0)
            {
                /* if we have a valid reason build the necessary fields in the message */
                __pvtBuildByFieldIds(reasonNodeId,hVal,q931NodeId,
                        {_q931(userUser) _q931(h323_UserInformation) 
                         _q931(h323_uu_pdu) _q931(h323_message_body) 
                         _q931(facility) _q931(reason) LAST_TOKEN}, 0, NULL);
                if (reasonNodeId >= 0)
                {
                    /* convert the reason to the proper fieldId and build it in place */
                    INT32 nameId = getParamFieldName(cmParamFacilityReason)[reasonId].nameId;

                    if (pvtAdd(hVal, reasonNodeId, nameId, 0, NULL, NULL)<0)
                        pvtDelete(hVal,reasonNodeId);
                }
            }
       }
    }

    ret = sendCallMessage(cmiGetByQ931(call),message);
    callReleaseMessage(cmiGetByQ931(call),cmQ931facility);
    return ret;
}

/****************************/
/*  M  E  S  S  A  G  E  S  */
/****************************/
int q931SimulateSetup(HQ931 call)
{
    q931Elem* callE=(q931Elem*)call;
    if (callE->callState==cs_Null)
    {
        callE->callState=cs_Call_present;
    }
    return 0;
}

int q931Setup(q931Elem*callE, int message)
{
    /* process the setup, even if we already sent call-proceeding or alerting on the call */
    if (callE->callState==cs_Null || callE->callState==cs_Call_present ||
        callE->callState==cs_Incoming_call_proceeding || callE->callState==cs_Call_received)
    {
        if (callE->callState == cs_Null)
            callE->callState=cs_Call_present;
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931setup);
    }
    return 0;
}

int q931CallProceeding(q931Elem*callE, int message)
{
    if (callE->callState==cs_Call_initiated||
        callE->callState==cs_Overlap_sending)
    {
        callE->callState=cs_Outgoing_call_proceeding;
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),q931T310Timeout,callE,callE->t310);
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931callProceeding);
    }
    return 0;
}

int q931Alerting(q931Elem*callE, int message)
{
    if (callE->callState==cs_Call_initiated||
        callE->callState==cs_Overlap_sending||
        callE->callState==cs_Outgoing_call_proceeding)
    {
        callE->callState=cs_Call_delivered;
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),q931T301Timeout,callE,callE->t301);
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931alerting);
    }
    return 0;
}

int q931Connect(q931Elem*callE, int message)
{
    if (callE->callState==cs_Call_initiated||
        callE->callState==cs_Overlap_sending||
        callE->callState==cs_Outgoing_call_proceeding||
        callE->callState==cs_Call_delivered)
    {
        callE->callState=cs_Active;
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931connect);
    }
    return 0;
}

int q931ReleaseComplete(q931Elem*callE, int message)
{
    callE->callState=cs_Null;
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timerSE));
    cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931releaseComplete);
    return 0;
}

int q931Status(q931Elem*callE, int message)
{
    cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timerSE));
    cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931status);
    return 0;
}

int q931Facility(q931Elem*callE, int message)
{
    cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931facility);
    return 0;
}
int q931StatusEnquiry(q931Elem*callE, int message)
{
    message=callGetMessage(cmiGetByQ931((HQ931)callE),cmQ931status);
    {
        HPVT hVal=cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)));
        int tmpNodeId,tmpNodeId1;
        __pvtGetNodeIdByFieldIds(tmpNodeId,hVal,message, {_q931(message) _anyField LAST_TOKEN});
        __pvtBuildByFieldIds(tmpNodeId1,hVal,tmpNodeId,
                            {_q931(cause) _q931(octet4) _q931(causeValue) LAST_TOKEN},30,NULL);
        __pvtBuildByFieldIds(tmpNodeId1,hVal,tmpNodeId,
                            {_q931(callState) _q931(callStateValue) LAST_TOKEN},callE->callState,NULL);
    }
    sendCallMessage(cmiGetByQ931((HQ931)callE),message);
    callReleaseMessage(cmiGetByQ931((HQ931)callE),cmQ931status);
    return 0;
}
int q931Progress(q931Elem*callE, int message)
{
    if (callE-> callState==cs_Call_initiated || 
        callE->callState==cs_Overlap_sending ||
        callE->callState==cs_Outgoing_call_proceeding ||
        callE->callState==cs_Call_delivered )
    {
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931progress);
    }
    return 0;
}
int q931SetupAcknowledge(q931Elem*callE, int message)
{
    if (callE->callState==cs_Call_initiated)
    {
        callE->callState=cs_Overlap_sending;
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),q931T304Timeout,callE,callE->t304);
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931setupAcknowledge);
    }
    return 0;
}
int q931Information(q931Elem*callE, int message)
{
    if (callE->callState==cs_Overlap_receiving)
    {
        emaMark((EMAElement)cmiGetByQ931((HQ931)callE));
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931information);
        if (emaWasDeleted((EMAElement)cmiGetByQ931((HQ931)callE)))
        {
            emaRelease((EMAElement)cmiGetByQ931((HQ931)callE));
            return 0;
        }
        cmTimerReset(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),&(callE->timer));
        callE->timer=cmTimerSet(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)),q931T302Timeout,callE,callE->t302);
        callE->callState=cs_Overlap_receiving;
        emaRelease((EMAElement)cmiGetByQ931((HQ931)callE));
    }
    if (callE->callState!=cs_Null &&
        callE->callState!=cs_Call_initiated &&
        callE->callState!=cs_Call_present &&
        callE->callState!=cs_Overlap_receiving)
    {
        cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931information);
    }
    return 0;
}

int q931Notify(q931Elem*callE, int message)
{
    cmIndicate(cmiGetByQ931((HQ931)callE),message, cmQ931notify);
    return 0;
}

/************************************************************************
 * q931DecodingFailure
 * purpose: Handle incoming Q931 messages that can't be decoded
 *          This automatically sends back a STATUS message
 * input  : callE       - Stack handle for the Q931 call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int q931DecodingFailure(IN HQ931 call)
{
    int message;
    q931Elem* callE = (q931Elem *)call;

    /* Create a STATUS message to send back */
    message = callGetMessage(cmiGetByQ931((HQ931)callE), cmQ931status);
    {
        /* The causeValue used is 95 - Invalid message, Unspecified */
        HPVT hVal = cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931((HQ931)callE)));
        int tmpNodeId, tmpNodeId1;
        __pvtGetNodeIdByFieldIds(tmpNodeId, hVal, message, {_q931(message) _anyField LAST_TOKEN});
        __pvtBuildByFieldIds(tmpNodeId1,hVal, tmpNodeId,
                            {_q931(cause) _q931(octet4) _q931(causeValue) LAST_TOKEN}, 95, NULL);
        __pvtBuildByFieldIds(tmpNodeId1, hVal, tmpNodeId,
                            {_q931(callState) _q931(callStateValue) LAST_TOKEN}, callE->callState, NULL);
    }

    /* Send the STATUS message and wrap things up */
    sendCallMessage(cmiGetByQ931((HQ931)callE), message);
    callReleaseMessage(cmiGetByQ931((HQ931)callE), cmQ931status);

    return 0;
}

int q931ProcessMessage(HQ931 call, int message)
{
    q931Elem* callE=(q931Elem*)call;
    HPVT hVal=cmGetValTree(cmGetAHandle((HPROTOCOL)cmiGetByQ931(call)));
    int msgType=pvtGetChildTagByPath(hVal,message,"message",1);
    /*Move the message tree into call database*/
    if (msgType!=cmQ931setup)
        message=callSetMessage(cmiGetByQ931(call),(cmCallQ931MsgType)msgType,message);
    switch(msgType)
    {
        case cmQ931setup            :q931Setup(callE,message);              break;
        case cmQ931callProceeding   :q931CallProceeding(callE,message);     break;
        case cmQ931connect          :q931Connect(callE,message);            break;
        case cmQ931alerting         :q931Alerting(callE,message);           break;
        case cmQ931releaseComplete  :q931ReleaseComplete(callE,message);    break;
        case cmQ931status           :q931Status(callE,message);             break;
        case cmQ931facility         :q931Facility(callE,message);           break;
        case cmQ931statusEnquiry    :q931StatusEnquiry(callE,message);      break;
        case cmQ931progress         :q931Progress(callE,message);           break;
        case cmQ931setupAck         :q931SetupAcknowledge(callE,message);   break;
        case cmQ931information      :q931Information(callE,message);        break;
        case cmQ931notify           :q931Notify(callE,message);             break;
        default :                    pvtDelete(hVal,message);               break;
    }
    return TRUE;
}
#ifdef __cplusplus
}
#endif
