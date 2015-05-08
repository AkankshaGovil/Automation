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



#include <stdlib.h>

#include <rvinternal.h>
#include <cmictrl.h>
#include <ms.h>
#include <conf.h>
#include <caputils.h>
#include <netutl.h>
#include <strutils.h>
#include <cmchan.h>
#include <oidutils.h>
#include <stkutils.h>
#include <h245.h>
#include <cmCtrlCap.h>
#include <cmdebprn.h>

#define ifE(a) if(a)(a)


int  cmcReadyEvent  (controlElem* ctrl);
BOOL simulatedMessageH245(IN HCONTROL ctrl);
#define capSetSize 100
#define capDescSize 100

/*Out Capibility*/
void RVCALLCONV capTimeoutEventsHandler(void*hsCall);

void capInit(controlElem* ctrl)
{
    cmElem* app = (cmElem *)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree((HAPP)app);
    outCapT* outcap=&ctrl->outCap;
    inCapT* incap=&ctrl->inCap;
    outcap->sq=0;

    outcap->pIDLen=utlEncodeOID(sizeof(outcap->pID),outcap->pID,"itu-t recommendation h(8) 245 0 7");
    outcap->waiting=FALSE;
    outcap->termNodeId=RVERROR;
    incap->termNodeId=RVERROR;
    incap->manualResponse=(pvtGetChild2(hVal,app->h245Conf,__h245(capabilities),__h245(manualResponse))>=0);
    logPrint(app->log, RV_INFO,
            (app->log, RV_INFO, "TCS manual response on %x = %d ",ctrl,incap->manualResponse));

    outcap->timer=(HTI)RVERROR;
    outcap->rejectCause = RVERROR;
}

/************************************************************************
 * capEnd
 * purpose: Finish with the capabilities exchange of a control object
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void capEnd(IN controlElem* ctrl)
{
    outCapT* outcap=&ctrl->outCap;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);

    /* Reset the outgoing capabilities timer is one exists */
    cmTimerReset(hApp,&(outcap->timer));

    /* Delete PVT tree nodes of capabilities */
    if (ctrl->outCap.termNodeId>=0)
    {
        pvtDelete(hVal,  ctrl->outCap.termNodeId);
        ctrl->outCap.termNodeId = RVERROR;
    }
    if (ctrl->inCap.termNodeId>=0)
    {
        pvtDelete(hVal,  ctrl->inCap.termNodeId);
        ctrl->inCap.termNodeId = RVERROR;
    }
}

int outCapTransferRequest(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    outCapT* outcap=&ctrl->outCap;
    int msgId;
    INT32 timeout=9;
    int res;
    int iSq;

    outcap->sq++;

    msgId=pvtGetChild2(hVal,message, __h245(request), __h245(terminalCapabilitySet));
    iSq = outcap->sq;
    pvtAdd(hVal, msgId, __h245(sequenceNumber) , iSq, NULL, NULL);
    pvtAdd(hVal, msgId, __h245(protocolIdentifier) , outcap->pIDLen, outcap->pID, NULL);

    res = sendMessageH245((HCONTROL)ctrl, message);
    if (res >= 0)
    {
        /* Update the outgoing capabilities of this terminal */
        if (outcap->termNodeId < 0)
            outcap->termNodeId = pvtAddRoot(hVal, NULL, 0, NULL);
        pvtMoveTree(hVal, outcap->termNodeId, msgId);

        if (!simulatedMessageH245((HCONTROL)ctrl))
        {
            /* We're only here if we really sent the message */
            /* Set the timer for this transaction from the configuration */
            pvtGetChildValue2(hVal,((cmElem*)hApp)->h245Conf,__h245(capabilities),__h245(timeout),&timeout,NULL);
            cmTimerReset(hApp,&(outcap->timer));
            outcap->timer = cmTimerSet(hApp,capTimeoutEventsHandler,(void*)ctrl,timeout*1000);
        }
        /* Update control state because of this TCS */
        outcap->waiting = TRUE;
        ctrl->outCapStatus = capSent;
    }

    return res;
}


/************************************************************************
 * terminalCapabilitySetAck
 * purpose: Handle an incoming TerminalCapabilitySetAck message
 * input  : ctrl    - Control object
 *          message - TCS.Ack message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int terminalCapabilitySetAck(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    cmElem* app=(cmElem*)hApp;
    outCapT* outcap=&ctrl->outCap;
    if (outcap->waiting)
    {
        INT32 sq;
        pvtGetChildValue(hVal, message, __h245(sequenceNumber), &sq, NULL);
        if (sq==outcap->sq)
        {
            int nesting;
            cmTimerReset(hApp,&(outcap->timer));
            ctrl->outCapStatus = capAcknowledged;
            outcap->waiting = FALSE;

            cmiCBEnter(hApp, "cmEvCallCapabilitiesResponse: haCall=0x%lx, hsCall=0x%lx, cmCapAccept.", (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl));
            emaMark((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            ifE(app->cmMySessionEvent.cmEvCallCapabilitiesResponse) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmCapAccept);
            emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
            cmiCBExit(hApp, "cmEvCallCapabilitiesResponse.");

            if (!emaWasDeleted((EMAElement)cmiGetByControl((HCONTROL)ctrl)))
            {
                cmcReadyEvent(ctrl);
            }
            emaRelease((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        }
    }
    return TRUE;
}


/************************************************************************
 * terminalCapabilitySetReject
 * purpose: Handle an incoming TerminalCapabilitySetReject message
 * input  : ctrl    - Control object
 *          message - TCS.Reject message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int terminalCapabilitySetReject(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    outCapT* outcap=&ctrl->outCap;
    cmElem* app=(cmElem*)hApp;

    if (outcap->waiting)
    {
        INT32 sq;
        pvtGetChildValue(hVal, message, __h245(sequenceNumber), &sq, NULL);
        if (sq==outcap->sq)
        {
            int tmpNodeId;
            int nesting;

            cmTimerReset(hApp,&(outcap->timer));
            ctrl->outCapStatus = capRejected;

            /* Find out the cause */
            __pvtGetByFieldIds(tmpNodeId, hVal, message, {_h245(cause) _anyField LAST_TOKEN}, NULL, NULL, NULL);
            ctrl->outCap.rejectCause = pvtGetSyntaxIndex(hVal, tmpNodeId);

            cmiCBEnter(hApp, "cmEvCallCapabilitiesResponse: haCall=0x%lx, hsCall=0x%lx, cmCapReject.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl));
            emaMark((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            ifE(app->cmMySessionEvent.cmEvCallCapabilitiesResponse) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmCapReject);
            emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
            cmiCBExit(hApp, "cmEvCallCapabilitiesResponse.");
            outcap->waiting=FALSE;
            emaRelease((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        }
    }
    return TRUE;
}

void RVCALLCONV capTimeoutEventsHandler(void*_ctrl)
{
    if (emaLock((EMAElement)cmiGetByControl((HCONTROL)_ctrl)))
    {
        controlElem* ctrl=(controlElem*)_ctrl;
        HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        HPVT hVal=cmGetValTree(hApp);
        outCapT* outcap=&ctrl->outCap;
        cmElem* app=(cmElem*)hApp;
        int nodeId=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        int nesting;

        cmTimerReset(hApp,&(outcap->timer));
        ctrl->outCapStatus = capRejected;
        ctrl->outCap.rejectCause = RVERROR; /* No actual cause on timeout */
        pvtAddBranch2(hVal,nodeId,__h245(indication),__h245(terminalCapabilitySetRelease));
        sendMessageH245((HCONTROL)ctrl, nodeId);
        pvtDelete(hVal,nodeId);

        outcap->waiting=FALSE;

        cmiCBEnter(hApp, "cmEvCallCapabilitiesResponse: haCall=0x%lx, hsCall=0x%lx, cmCapReject.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl));
        nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        ifE(app->cmMySessionEvent.cmEvCallCapabilitiesResponse) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), cmCapReject);
        emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
        cmiCBExit(hApp, "cmEvCallCapabilitiesResponse.");

        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
}


/*In Capability*/



/************************************************************************
 * terminalCapabilitySet
 * purpose: Handle an incoming TerminalCapabilitySet message
 * input  : ctrl    - Control object
 *          message - TCS.Request message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int terminalCapabilitySet(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    inCapT* incap=&ctrl->inCap;
    cmElem* app=(cmElem*)hApp;
    int nodeId;
    INT32 sq=0;

    /* Get the parameters from the incoming cap */
    pvtGetChildValue(hVal,message,__h245(sequenceNumber),&sq,NULL);
    incap->sq=(BYTE)sq;
    nodeId = pvtGetChildByFieldId(hVal, message, __h245(protocolIdentifier), (INT32 *)&incap->pIDLen, NULL);
    if ((nodeId >= 0) && (incap->pIDLen > (int)sizeof(incap->pID)))
        pvtGetString(hVal, nodeId, incap->pIDLen, incap->pID);
    else
        incap->pIDLen = -1;

	if (incap->termNodeId < 0)
        incap->termNodeId=pvtAddRoot(hVal,NULL,0,NULL);
    pvtShiftTree(hVal,incap->termNodeId,message);

    ctrl->inCapStatus = capSent;

    emaMark((EMAElement)cmiGetByControl((HCONTROL)ctrl));

    /* Handle callbacks of capability exchange message if we have to */
    if (app->cmMySessionEvent.cmEvCallCapabilities || app->cmMySessionEvent.cmEvCallCapabilitiesExt)
    {
        int i;
        cmCapStruct capSet[capSetSize];
        cmCapStruct *pcapSet[capSetSize];
        void* capDesc[capDescSize];
        int nesting;

        for (i=0; i<capSetSize; i++) pcapSet[i] = capSet+i;
        for (i=0; i<capDescSize; i++) capDesc[i] = capDesc-1;

        capSetBuild(hVal, incap->termNodeId, capSetSize, pcapSet);
        cmiCBEnter(hApp, "cmEvCallCapabilities: haCall=0x%lx, hsCall=0x%lx, capSet=0x%p.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), pcapSet);
        nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        ifE(app->cmMySessionEvent.cmEvCallCapabilities)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),(HCALL)cmiGetByControl((HCONTROL)ctrl),  pcapSet);
        emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
        cmiCBExit(hApp, "cmEvCallCapabilities.");

        if (!emaWasDeleted((EMAElement)cmiGetByControl((HCONTROL)ctrl)) &&
                app->cmMySessionEvent.cmEvCallCapabilitiesExt)
        {
            int nesting;

            capDescBuild(hVal, incap->termNodeId, pcapSet, capDescSize, capDesc);
            cmiCBEnter(hApp, "cmEvCallCapabilitiesExt: haCall=0x%lx, hsCall=0x%lx, capDesc=0x%p.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),
               cmiGetByControl((HCONTROL)ctrl), capDesc);
            nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            app->cmMySessionEvent.cmEvCallCapabilitiesExt((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),(HCALL)cmiGetByControl((HCONTROL)ctrl),(cmCapStruct****)capDesc);
            emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
            cmiCBExit(hApp, "cmEvCallCapabilitiesExt.");
        }
    }

    if ( (!emaWasDeleted((EMAElement)cmiGetByControl((HCONTROL)ctrl))) && 
         (!incap->manualResponse) )
    {
        logPrint(app->log, RV_INFO,
                (app->log, RV_INFO, "Invoking automatic TCS response %d",incap->manualResponse));

        inCapTransferRequest(ctrl);
    }

    emaRelease((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    return TRUE;
}

RVAPI int RVCALLCONV
cmCallCapabilitiesAck(
               IN HCALL hsCall)
{
    HAPP hApp=NULL;
    controlElem* ctrl=NULL;
    inCapT* incap=NULL;
    int  ret;
    BOOL ok = TRUE;

    /* check validity of data */
    if (!hsCall) ok = FALSE;

    if (ok)
    {
        hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
        if (!hApp)  ok = FALSE;
    }
    
    cmiAPIEnter(hApp, "cmCallCapabilitiesAck: hsCall=0x%lx.", hsCall);

    if (ok)
    {
        ctrl=(controlElem*)cmiGetControl(hsCall);
        if (!ctrl)  ok = FALSE;
    }

    if (ok)
    {
        incap=&ctrl->inCap;
        if (!incap)  ok = FALSE;
    }

    /* check if we are in manual response, otherwise this API is forbidden */
    if (ok)
    {
        if (!incap->manualResponse) ok = FALSE;
    }

    /* send the TCS Ack */
    if (ok)
    {
        if (emaLock((EMAElement)hsCall))
        {
            emaMark((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            inCapTransferRequest(ctrl);
            emaRelease((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            emaUnlock((EMAElement)hsCall);
        }
    }

    if (ok)
        ret = 1;
    else
        ret = RVERROR;

    cmiAPIExit(hApp, "cmCallCapabilitiesAck: [%d].", ret);
    return ret;
}

void inCapTransferRequest(controlElem* ctrl)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    inCapT* incap=&ctrl->inCap;
    int res;
    int nodeId=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
    int iSq = incap->sq;
    __pvtBuildByFieldIds(res,hVal,nodeId,{_h245(response) _h245(terminalCapabilitySetAck) _h245(sequenceNumber) LAST_TOKEN},iSq,NULL);

    res = sendMessageH245((HCONTROL)ctrl, nodeId);
    pvtDelete(hVal,nodeId);

    if (res >= 0)
    {
        ctrl->inCapStatus = capAcknowledged;
        cmcReadyEvent(ctrl);
    }
}


/************************************************************************
 * terminalCapabilitySetRelease
 * purpose: Handle an incoming TerminalCapabilitySetRelease message
 * input  : ctrl    - Control object
 *          message - TCS.Release message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int terminalCapabilitySetRelease(IN controlElem* ctrl, IN int message)
{
    if (message);
    ctrl->inCapStatus = capReleased;
    return TRUE;
}


RVAPI int RVCALLCONV /* TRUE or RVERROR */
cmCallSendCapability(
             /* Send terminal capability set tree to remote terminal */
             /* Note: cap tree is not deleted! */
             IN  HCALL hsCall,
             IN  int termCapSet  /* local terminal capabiliy set Id */
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int message;
    int ret=TRUE;
    if (!hsCall || !hApp) return RVERROR;


    cmiAPIEnter(hApp, "cmCallSendCapability: hsCall=0x%lx, capSet=%d.", hsCall, termCapSet);
    if (emaLock((EMAElement)hsCall))
    {
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        ret=pvtSetTree(hVal, pvtAddBranch2(hVal, message, __h245(request), __h245(terminalCapabilitySet)), hVal, termCapSet);
        if (ret>=0)
            ret = outCapTransferRequest(ctrl, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmCallSendCapability: [%d].", ret);
    return ret;
}


RVAPI int RVCALLCONV /* new terminalCapabilitySet node id. */
cmCallCapabilitiesBuild(
            /* Build terminalCapabilitySet value tree including the capability set
               and capability descriptors. */
            IN  HCALL           hsCall,
            IN      cmCapStruct*        capSet[],
            IN      cmCapStruct***      capDesc[]
            )
{
    int rootId=-1;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);

    if (!hApp || !hsCall || !capSet || !capDesc) return RVERROR;

    cmiAPIEnter(hApp, "cmCallCapabilitiesBuild: hsCall=0x%lx, capSet=0x%p, capDesc=0x%p.", hsCall, capSet, capDesc);

    if (emaLock((EMAElement)hsCall))
    {
        if ((rootId=pvtAddRootByPath(hVal, ((cmElem*)hApp)->synProtH245, (char*)"request.terminalCapabilitySet", 0, NULL)) <0)
        {
            cmiAPIExit(hApp, "cmCallCapabilitiesBuild: [-1].");
            return rootId;
        }

        capSetBuildFromStruct(hVal, ((cmElem*)hApp)->h245Conf, rootId, capSet);
        capDescBuildFromStruct(hVal, rootId, capDesc);
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallCapabilitiesBuild: [%d].", rootId);
    return rootId;
}


RVAPI int RVCALLCONV
cmCallCapabilitiesSend(
               IN   HCALL           hsCall,
               IN      cmCapStruct*        capSet[],
               IN      cmCapStruct***      capDesc[])
{
    int rootId=-1;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    if (!hApp || !hsCall || !capSet || !capDesc) return RVERROR;

    cmiAPIEnter(hApp, "cmCallCapabilitiesSend: hsCall=0x%lx, capSet=0x%p, capDesc=0x%p.", hsCall, capSet, capDesc);

    if (emaLock((EMAElement)hsCall))
    {
        if ((rootId = cmCallCapabilitiesBuild(hsCall, capSet, capDesc)) <0)
        {
            cmiAPIExit(hApp, "cmCallCapabilitiesSend: [-1].");
            return RVERROR;
        }

        cmCallSendCapability(hsCall, rootId);
        pvtDelete(hVal, rootId);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmCallCapabilitiesSend: [1].");
    return TRUE;
}



RVAPI int RVCALLCONV /* remote terminal capability set node id, or RVERROR */
cmCallGetRemoteCapabilities(
                /* Get latest remote terminal capability message */
                IN  HCALL       hsCall
                )
{
    int termCapId=-1;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);

    if (!hsCall || !hApp) return RVERROR;
    cmiAPIEnter(hApp, "cmCallGetRemoteCapabilities: hsCall=0x%p.", hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        if (ctrl->state != ctrlNotInitialized)
            termCapId=ctrl->inCap.termNodeId;
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallGetRemoteCapabilities: [%d].", termCapId);
    return termCapId;
}




RVAPI int RVCALLCONV
cmCallGetLocalCapabilities(
               /* translate local caps (from CESE_OUT machine) to application structures.
                  Usage:
                   cmCapStruct capA[capSetASize];
                   cmCapStruct *capSetA[capSetASize];
                   void* capDescA[capDescASize];
                   cmCapStruct** capSet;
                   cmCapStruct**** capDesc;
                   int i;

                   for (i=0; i<capSetASize; i++) capSetA[i] = capA+i;
                   cmCallGetLocalCapabilities(hsCall,
                                              capSetA, capSetASize,
                                              capDescA, capDescASize,
                              &capSet, &capDesc);
                              */

               IN      HCALL        hsCall,
               IN      cmCapStruct*         capSetA[], /* user allocated */
               IN      int                  capSetASize,
               IN      void*                capDescA[], /* user allocated */
               IN      int                  capDescASize,
               OUT     cmCapStruct**        capSet[],
               OUT     cmCapStruct****      capDesc[]
               )
{
    int capId=-1, i;
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    int ret = TRUE;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCallGetLocalCapabilities: hsCall=0x%p, capSetA=0x%p, capSetASize=%d, capDescA=0x%p, capDescASize=%d, capSet=0x%p, capDesc=0x%p.",
          hsCall, capSetA, capSetASize, capDescA, capDescASize, capSet, capDesc);

    if ((ctrl->state != ctrlNotInitialized) && (ctrl->outCap.termNodeId >= 0))
    {
        capId = ctrl->outCap.termNodeId;
    }
    else
    {
        capId = pvtGetChild2(hVal,((cmElem*)hApp)->h245Conf,__h245(capabilities),__h245(terminalCapabilitySet));
    }

    /* Some validity checks */
    if ((capId < 0) || (capSetASize < 1) || (capDescASize < 1))
        ret = RVERROR;

    if (ret >= 0)
    {
        if (capSetA == NULL)
            ret = TRUE;
        else
        {
            /* We should probably start building the capabilities */
            if (emaLock((EMAElement)hsCall))
            {
                capSetBuild(hVal, capId, capSetSize, capSetA);
                if (capSet)
                    *capSet = capSetA;

                if (capDescA != NULL)
                {
                    for (i = 0; i < capDescASize; i++)
                        capDescA[i] = capDescA-1;

                    if (capDescBuild(hVal, capId, capSetA, capDescASize, capDescA) <0)
                    {
                        logPrintFormat(((cmElem*)hApp)->logERR, RV_ERROR, "cmcGetLocalCaps:capDescBuild failed.");
                        ret = RVERROR;
                    }
                    else
                    {
                        if (capDesc)
                            *capDesc = (cmCapStruct****)capDescA;
                    }
                }

                emaUnlock((EMAElement)hsCall);
            }
        }
    }

    cmiAPIExit(hApp, "cmCallGetLocalCapabilities: [%d].", ret);
    return ret;
}

/* delete pvt of remote capability to reduce memory*/

RVAPI
int RVCALLCONV cmFreeCapability(IN HCALL hsCall)
{
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);

    if (!hsCall || !hApp) return RVERROR;
    cmiAPIEnter(hApp, "cmFreeCapability: hsCall=0x%lx.", hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        if (ctrl->state != ctrlNotInitialized)
        {
            if (ctrl->outCap.termNodeId>=0)
            {
                pvtDelete(hVal,  ctrl->outCap.termNodeId);
                ctrl->outCap.termNodeId = RVERROR;
            }
            if (ctrl->inCap.termNodeId>=0)
            {
                pvtDelete(hVal,  ctrl->inCap.termNodeId);
                ctrl->inCap.termNodeId = RVERROR;
            }
        }
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmFreeCapability: [1].");
    return TRUE;
}




/************************************************************************
 * tcsGetMibParams
 * purpose: Get parameters related to the MIB for a TCS object in the
 *          control
 * input  : ctrl            - Control object
 *          inDirection     - TRUE if incoming TCS should be checked. FALSE for outgoing
 * output : state           - Status of TCS
 *          rejectCause     - Reject cause of TCS.Reject
 * return : Non-negative value on success
 *          Negative value on failure
 * Any of the output parameters can be passed as NULL if not relevant to caller
 ************************************************************************/
int tcsGetMibParams(
    IN  controlElem*    ctrl,
    IN  BOOL            inDirection,
    OUT capStatus*      status,
    OUT int*            rejectCause)
{
    if (status)
    {
        if (inDirection)
            *status = ctrl->inCapStatus;
        else
            *status = ctrl->outCapStatus;
    }
    if (rejectCause)
    {
        if (inDirection)
            return RVERROR; /* We never send TCS.Reject */
        else
        {
            if (ctrl->outCapStatus == capRejected)
                *rejectCause = ctrl->outCap.rejectCause;
            else
                return RVERROR;
        }
    }

    return 0;
}


/************************************************************************
 * tcsGetMibProtocolId
 * purpose: Get the protocol identifier of the capabilities.
 * input  : ctrl            - Control object
 *          inDirection     - TRUE if incoming TCS should be checked. FALSE for outgoing
 * output : valueSize       - Size in bytes of the protocol identifier
 *          value           - The protocol identifier
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int tcsGetMibProtocolId(
    IN  controlElem*    ctrl,
    IN  BOOL            inDirection,
    OUT int*            valueSize,
    OUT UINT8*          value)
{
    if (inDirection)
    {
        if (ctrl->inCap.pIDLen < 0)
            return RVERROR;
        *valueSize = ctrl->inCap.pIDLen;
        memcpy(value, ctrl->inCap.pID, *valueSize);
    }
    else
    {
        *valueSize = ctrl->outCap.pIDLen;
        memcpy(value, ctrl->outCap.pID, *valueSize);
    }
    return 0;
}



#ifdef __cplusplus
}
#endif
