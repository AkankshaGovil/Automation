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

#include <cm.h>
#include <cmintr.h>
#include <cmictrl.h>
#include <cmCall.h>
#include <callobj.h>
#include <log.h>
#include <ms.h>
#include <q931asn1.h>
#include <stkutils.h>
#include <cmdebprn.h>
#include <cmutils.h>
#include <cmCrossReference.h>
#include <cmQ931.h>
#include <cmiFastStart.h>
#include <cmAutoRasCall.h>
#include <cmAutoRasEP.h>
#include <cmParam.h>
#include <cmControl.h>
#include <psyntreeStackApi.h>
#include <statistic.h>
#include <prnutils.h>
#include <oidutils.h>

void enterCallInitState(IN callElem* call);
void enterCallSetupState(IN callElem* call);
void enterCallConnectedState(IN callElem* call);
void enterCallIdleState(IN callElem* call);
void notifyControlState(IN HCALL hsCall);
int  cmcReadyEvent(IN controlElem* ctrl);
int  callAnswer(callElem*call);

char* getCID(RvRandomGenerator*seed);

int cmCallStatusMsg2Struct(HPVT hVal,int handle,cmCallStatusMessage * callStatusMsg);


void RVCALLCONV controlDiedTimerEventsHandler(void*context);



/************************************************************************
 * simulatedMessageH245
 * purpose: define if message was simulated (was not sent actually)
 * input  : ctrl  - Stack handle for the H245 call control
 * output : none
 * return : true if simulated
 *          false - otherwise
 ************************************************************************/
BOOL simulatedMessageH245(IN HCONTROL ctrl)
{
    callElem*call=(callElem*)cmiGetByControl(ctrl);
    HSTRANSSESSION   hsTransSession=call->hsTransSession;

    return dummyControlSession(hsTransSession);
}


/************************************************************************
 * sendMessage
 * purpose: Send TCP (Q931/H245) message for a given call.
 *          This function also adds specific call-related information
 * input  : hsCall  - Stack handle for the call
 *          vNodeId - root node ID of the message to send
 *          type    - Type of channel to send on
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendMessage(IN HCALL hsCall, IN int vNodeId, IN cmTpktChanHandleType type)
{
    callElem*call=(callElem*)hsCall;
    TRANSERR result;

    /* Message should be processed - send it out */
    result = cmTransSendMessage(call->hsTransSession, vNodeId, (type==cmQ931TpktChannel)?cmTransQ931Type:cmTransH245Type);
    if ((result == cmTransWouldBlock) || (result < 0))
        return RVERROR;
    return 0;
}

/************************************************************************
 * sendCallMessage
 * purpose: Send Q931 message for a given call.
 *          This function also adds specific call-related information
 * input  : hsCall  - Stack handle for the Q931 call
 *          message - root node ID of the message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendCallMessage(IN HCALL hsCall, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    int tmpNodeId;
    int msgNodeId,uuieId;
    HPVT hVal=cmGetValTree(hApp);
    /*Add to the message the identification parameters*/
    /*CRV*/
    __pvtBuildByFieldIds(tmpNodeId,hVal,message, {_q931(callReferenceValue) _q931(twoBytes) LAST_TOKEN},call->crv,NULL);

    __pvtGetNodeIdByFieldIds(uuieId,hVal,message, {_q931(message) _anyField _q931(userUser)
                                                   _q931(h323_UserInformation) _q931(h323_uu_pdu) LAST_TOKEN});
    __pvtGetNodeIdByFieldIds(msgNodeId,hVal,uuieId, {_q931(h323_message_body) _anyField LAST_TOKEN});
    /*Call ID*/
    __pvtBuildByFieldIds(tmpNodeId,hVal,msgNodeId, {_q931(callIdentifier) _q931(guid) LAST_TOKEN},16,(char*)call->callId);
    /*CID*/
    if (m_callget(call, overrideCID))
    {
        __pvtBuildByFieldIds(tmpNodeId,hVal,msgNodeId, {_q931(conferenceID) LAST_TOKEN},16,(char*)call->cId);
    }
    /*Add dest info field*/
    if (call->destinationInfo>=0)
        pvtSetTree(hVal,pvtAddBranch(hVal,msgNodeId, __q931(destinationInfo)),
               hVal,call->destinationInfo);

    /*Add fast start messages in*/
    addFastStart(call,message);

    /* Send the message on Q931 channel */
    return sendMessage(hsCall,message,cmQ931TpktChannel);
}


/************************************************************************
 * sendMessageH245
 * purpose: Send H245 message for a given call.
 * input  : ctrl    - Stack handle for the H245 call control
 *          message - root node ID of the message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendMessageH245(
    IN HCONTROL ctrl,
    IN int      message)
{
    callElem*call=(callElem*)cmiGetByControl(ctrl);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    int nesting;

    if (((cmElem*)hApp)->cmMyCallEvent.cmEvCallSendMessage)
    {
        cmiCBEnter(hApp, "cmEvCallSendMessage(haCall=0x%p,hsCall=0x%p,msg=%d)",
            (HAPPCALL)emaGetApplicationHandle((EMAElement)call), (HCALL)call, message);
        nesting = emaPrepareForCallback((EMAElement)call);
        (((cmElem*)hApp)->cmMyCallEvent.cmEvCallSendMessage)(
            (HAPPCALL)emaGetApplicationHandle((EMAElement)call),
            (HCALL)call,
            message);
        emaReturnFromCallback((EMAElement)call, nesting);
        cmiCBExit(hApp, "cmEvCallSendMessage");
    }

    return sendMessage(cmiGetByControl(ctrl),message,cmH245TpktChannel);
}


/************************************************************************
 * sendMessageH245Chan
 * purpose: Send H245 message for a given channel.
 * input  : ctrl    - Stack handle for the H245 call control
 *          hsChan  - Channel handle for the H245 channel
 *          message - root node ID of the message to send
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int sendMessageH245Chan(
    IN HCONTROL ctrl,
    IN HCHAN    hsChan,
    IN int      message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    int nesting;

    if (((cmElem*)hApp)->cmMyChannelEvent.cmEvChannelSendMessage)
    {
        cmiCBEnter(hApp, "cmEvChannelSendMessage(haChan=0x%p,hsChan=0x%p,msg=%d)",
            (HAPPCHAN)emaGetApplicationHandle((EMAElement)hsChan), hsChan, message);
        nesting = emaPrepareForCallback((EMAElement)hsChan);
        (((cmElem*)hApp)->cmMyChannelEvent.cmEvChannelSendMessage)(
            (HAPPCHAN)emaGetApplicationHandle((EMAElement)hsChan),
            hsChan,
            message);
        emaReturnFromCallback((EMAElement)hsChan, nesting);
        cmiCBExit(hApp, "cmEvChannelSendMessage");
    }

    return sendMessageH245(ctrl, message);
}


/************************************************************************
 * cmCallGetMessageContext
 * purpose: Get the message context for a given call. This function is
 *          used for checking security results of messages for a call.
 * input  : hsCall      - Stack handle for the call
 * output : hMsgContext - Message context for call (of the last message)
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallGetMessageContext(
    IN  HCALL            hsCall,
    OUT void**           hMsgContext)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);

    cmiAPIEnter(hApp,(char*)"cmCallGetMessageContext: hsCall=0x%x ",hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        if (hMsgContext)
            *hMsgContext=call->hMsgContext;
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallGetMessageContext: [0]");

    return 0;
}


TRANSERR cmEvTransHostListen(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;
    int addr;
    BOOL proceed=TRUE;

    if (type || haTransHost);

    if (hsTransHost && hApp)
    {
        addr=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);

        if (addr>=0 && address)
        {
            cmTAToVt(app->hVal,addr,address);

            if (app->cmMyProtocolEvent.hookListen!=NULL)
                proceed=!app->cmMyProtocolEvent.hookListen((HPROTCONN)hsTransHost,addr);

            pvtDelete(app->hVal,addr);
        }
    }
    return (proceed)?cmTransOK:cmTransIgnoreMessage;
}

TRANSERR cmEvTransHostListening(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address,
            IN BOOL                 error)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;
    int addr;
    BOOL proceed=TRUE;

    if (type || haTransHost);

    if (hsTransHost && hApp)
    {
        addr=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);

        if (addr>=0 && address)
        {
            cmTAToVt(app->hVal,addr,address);

            if (app->cmMyProtocolEvent.hookListening!=NULL)
                proceed=!app->cmMyProtocolEvent.hookListening((HPROTCONN)hsTransHost,addr,error);

            pvtDelete(app->hVal,addr);
        }
    }
    return (proceed)?cmTransOK:cmTransIgnoreMessage;
}

TRANSERR cmEvTransHostConnecting(
            IN HSTRANSHOST          hsTransHost,
            IN HATRANSHOST          haTransHost,
            IN TRANSCONNTYPE        type,
            IN cmTransportAddress   *address)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;
    int addr;
    BOOL proceed=TRUE;

    if (type || haTransHost);

    addr=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);

    if (addr>=0 && address)
    {
        cmTAToVt(app->hVal,addr,address);

        if (app->cmMyProtocolEvent.hookConnecting!=NULL)
            proceed=!app->cmMyProtocolEvent.hookConnecting((HPROTCONN)hsTransHost,addr);

        pvtDelete(app->hVal,addr);
    }
    return (proceed)?cmTransOK:cmTransIgnoreMessage;
}


TRANSERR cmEvTransHostConnected(IN HSTRANSHOST   hsTransHost,
                                IN HATRANSHOST   haTransHost,
                                IN TRANSCONNTYPE type,
                                IN BOOL          isOutgoing)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;
    cmTransportAddress ta;
    int addrRemote,addrLocal;

    if (type || haTransHost);

    /* Make sure application wants to deal with these hooks at all */
    if ((isOutgoing && (app->cmMyProtocolEvent.hookOutConn == NULL)) ||
        (!isOutgoing && (app->cmMyProtocolEvent.hookInConn == NULL)))
        return cmTransOK;

    addrRemote=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
    addrLocal=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
    if (addrRemote>=0 && addrLocal>=0)
    {
        if (cmTransGetHostParam(hsTransHost, cmTransHostParam_remoteAddress, (void*)&ta)==cmTransOK)
        {
            cmTAToVt(app->hVal,addrRemote,&ta);
            if (cmTransGetHostParam(hsTransHost, cmTransHostParam_localAddress, (void*)&ta)==cmTransOK)
                cmTAToVt(app->hVal,addrLocal,&ta);

            /* No need to check if hooks exist - we already did that at the beginning of this function */
            if (isOutgoing)
                app->cmMyProtocolEvent.hookOutConn((HPROTCONN)hsTransHost,addrLocal,addrRemote,FALSE);
            else
                app->cmMyProtocolEvent.hookInConn((HPROTCONN)hsTransHost,addrRemote,addrLocal);
        }
    }
    pvtDelete(app->hVal,addrRemote);
    pvtDelete(app->hVal,addrLocal);
    return cmTransOK;
}


TRANSERR cmEvTransHostClosed(   IN HSTRANSHOST hsTransHost,
                                IN HATRANSHOST haTransHost,
                                IN BOOL        wasConnected)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;

    if (haTransHost);

    if (!wasConnected)
    {
        cmTransportAddress ta;
        int addrRemote,addrLocal;

        addrRemote=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
        addrLocal=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
        if (addrRemote>=0 && addrLocal>=0)
            if (cmTransGetHostParam(hsTransHost, cmTransHostParam_remoteAddress, (void*)&ta)==cmTransOK)
            {
                cmTAToVt(app->hVal,addrRemote,&ta);
                if (cmTransGetHostParam(hsTransHost, cmTransHostParam_localAddress, (void*)&ta)==cmTransOK)
                    cmTAToVt(app->hVal,addrLocal,&ta);

                if (app->cmMyProtocolEvent.hookOutConn!=NULL)
                    app->cmMyProtocolEvent.hookOutConn((HPROTCONN)hsTransHost,addrLocal,addrRemote,FALSE);
            }
        pvtDelete(app->hVal,addrRemote);
        pvtDelete(app->hVal,addrLocal);
    }

    if (app->cmMyProtocolEvent.hookClose!=NULL)
        app->cmMyProtocolEvent.hookClose((HPROTCONN)hsTransHost);

    cmTransCloseHost(hsTransHost);
    return cmTransOK;
}


TRANSERR cmEvTransNewRawMessage(IN  HSTRANSHOST hsTransHost,
                                IN  HATRANSHOST haTransHost,
                                IN  TRANSTYPE   type,
                                INOUT int       pvtNode,
                                IN  BYTE        *msg,
                                IN  int         msgSize,
                                OUT int         *decoded,
                                OUT void        **hMsgContext)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    cmElem* app=(cmElem*)hApp;
    TRANSERR err;
    cmProtocol prot;

    if (haTransHost);

    cmCallPreCallBack(hApp);

    /* Decode the message. We call a callback function is Q931 case, since we might be using
       H.235v2 module in this part */
    if (app->cmEvCallNewRawMessage && type==cmTransQ931Type)
        err=(app->cmEvCallNewRawMessage(hApp, (HPROTCONN)hsTransHost, pvtNode, msg, msgSize, decoded, hMsgContext)<0)?cmTransErr:cmTransOK;
    else
        err=(cmEmDecode(app->hVal, pvtNode, msg, msgSize, decoded)<0)?cmTransErr:cmTransOK;
    if (err != cmTransOK)
    {
        logPrint(app->logTPKT,RV_INFO,
            (app->logTPKT,RV_INFO, "New message (channel %d) ignored (error)", emaGetIndex((EMAElement)hsTransHost)));
        logPrint(app->logTPKT, RV_INFO,(app->logTPKT, RV_INFO, "Binary:"));
        printHexBuff(msg, msgSize, app->logTPKT);
        return err;
    }

    logPrint(app->logTPKT,RV_INFO,
        (app->logTPKT,RV_INFO, "New message (channel %d) recv <-- %s:", emaGetIndex((EMAElement)hsTransHost), nprn(cmGetProtocolMessageName((HAPP)app,pvtNode))));

#ifndef NOLOGSUPPORT
    if  (logGetDebugLevel(app->logTPKT)>1)
    {
        logPrint(app->logTPKT, RV_INFO,(app->logTPKT, RV_INFO, "Binary:"));
        printHexBuff(msg, msgSize, app->logTPKT);

        logPrint(app->logTPKT,RV_INFO,(app->logTPKT,RV_INFO, "Message:"));
        pvtPrintStd(app->hVal, pvtNode, (int)app->logTPKT);
    }
#endif

    /* MIB - incoming message */
    if (type == cmTransQ931Type)
        prot = cmProtocolQ931;
    else
        prot = cmProtocolH245;
    addStatistic(app->hStatistic, prot, app->hVal, pvtNode, TRUE);

    if (app->cmMyProtocolEvent.hookRecv!=NULL)
        err=(app->cmMyProtocolEvent.hookRecv((HPROTCONN)hsTransHost,pvtNode,FALSE))?cmTransIgnoreMessage:err;

    return err;
}

TRANSERR cmEvTransSendRawMessage(   IN  HSTRANSHOST     hsTransHost,
                                    IN  HATRANSHOST     haTransHost,
                                    IN  HSTRANSSESSION  hsTransSession,
                                    IN  HATRANSSESSION  haTransSession,
                                    IN  int             pvtNode,
                                    IN  int             size,
                                    OUT BYTE            *msg,
                                    OUT int             *msgSize)
{
    HAPP hApp=(HAPP)cmTransGetHApp(hsTransHost);
    callElem*call=(callElem*)haTransSession;
    cmElem* app=(cmElem*)hApp;
    TRANSERR err;
    BOOL process=TRUE;

    if (hsTransSession || haTransHost);

    /* See if we've got a hook and if the outgoing message should be send or not */
    if (app->cmMyProtocolEvent.hookSend!=NULL)
        process=!app->cmMyProtocolEvent.hookSend((HPROTCONN)hsTransHost,pvtNode,FALSE);
    if (process)
    {
        cmProtocol prot = cmProtocolGetProtocol(hApp,pvtNode);

        if (call && (prot == cmProtocolQ931))
        {
            int nesting;
            if(app->cmMyCallEvent.cmEvCallSendMessage)
            {
                cmiCBEnter((HAPP)app,(char*)"cmEvCallSendMessage(haCall=0x%lx,hsCall=0x%lx)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
                nesting=emaPrepareForCallback((EMAElement)call);
                (app->cmMyCallEvent.cmEvCallSendMessage)((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call, pvtNode);
                emaReturnFromCallback((EMAElement)call,nesting);
                cmiCBExit((HAPP)app,(char*)"cmEvCallSendMessage");
            }
        }

        if ((call != NULL) && (emaWasDeleted((EMAElement)call)))
        {
            logPrint(app->logTPKT, RV_INFO,
             (app->logTPKT, RV_INFO, "New message (channel %d) Not sent (call deleted) --> %s:", emaGetIndex((EMAElement)hsTransHost), nprn(cmGetProtocolMessageName((HAPP)app,pvtNode))));
            return cmTransIgnoreMessage;
        }

        if (app->cmEvCallSendRawMessage)
            err=(app->cmEvCallSendRawMessage(hApp,(HPROTCONN)hsTransHost,(HCALL)call, pvtNode, size, msg, msgSize)<0)?cmTransErr:cmTransOK;
        else
            err=(cmEmEncode(app->hVal, pvtNode, msg, size, msgSize)<0)?cmTransErr:cmTransOK;
        if (err == cmTransOK)
        {
            logPrint(app->logTPKT, RV_INFO,
             (app->logTPKT, RV_INFO, "New message (channel %d) sent --> %s:", emaGetIndex((EMAElement)hsTransHost), nprn(cmGetProtocolMessageName((HAPP)app,pvtNode))));

            /* Make sure we notify the MIB component of the stack */
            addStatistic(app->hStatistic, prot, app->hVal, pvtNode, FALSE);
        }
        else
        {
            logPrint(app->logTPKT, RV_ERROR,
             (app->logTPKT, RV_ERROR, "New message (channel %d) Not sent (encoding error) --> %s:", emaGetIndex((EMAElement)hsTransHost), nprn(cmGetProtocolMessageName((HAPP)app,pvtNode))));
        }

#ifndef NOLOGSUPPORT
        if  (logGetDebugLevel(app->logTPKT) > 1)
        {
            logPrint(app->logTPKT,RV_INFO,(app->logTPKT,RV_INFO, "Message:"));
            pvtPrintStd(app->hVal, pvtNode, (int)app->logTPKT);

            logPrint(app->logTPKT, RV_INFO,(app->logTPKT, RV_INFO, "Binary:"));
            printHexBuff(msg, *msgSize, (RVHLOG)app->logTPKT);
        }
#endif
    }
    else
        err = cmTransIgnoreMessage;

    return err;
}


/********************************************************************************************
 * cmCallSimulateMessage
 * purpose : "Push" a message into the stack as if it was received from the network.
 * input   : hsCall     - Call the message is received on
 *           message    - PVT node ID of the received message
 *                        This node must be a Q931 or H245 message.
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSimulateMessage(
                         IN HCALL               hsCall,
                         IN int                 message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    HPVT hVal=cmGetValTree(hApp);

    cmiAPIEnter(hApp,(char*)"cmCallSimulateMessage: hsCall=0x%x, message=%d",hsCall,message);

    if (!emaLock((EMAElement)call))
    {
        cmiAPIExit(hApp,(char*)"cmCallSimulateMessage: -1");
        return cmTransErr;
    }

    switch(cmProtocolGetProtocol(hApp,message))
    {
        case cmProtocolQ931:
        {
            int msgNodeId,nodeId;
            INTPTR fieldId;

            __pvtGetNodeIdByFieldIds(msgNodeId,hVal,message, {_q931(message) _anyField _q931(userUser)
                                                              _q931(h323_UserInformation) _q931(h323_uu_pdu)
                                                              _q931(h323_message_body) _anyField LAST_TOKEN});

            /* make sure the message isn't a call-proceeding message */
            pvtGet(hVal, msgNodeId, &fieldId, NULL, NULL, NULL);

            if ((m_callget(call,remoteVersion) == 0) &&
                ((nodeId=pvtGetChild(hVal,msgNodeId,__q931(protocolIdentifier),NULL)) >= 0) &&
                (fieldId != __q931(callProceeding)))
            {
                char buff[20];
                int object[6];
                int buffLen;
                buffLen=pvtGetString(hVal, nodeId, sizeof(buff), buff);
                if (utlDecodeOIDInt(buffLen, buff, sizeof(object)/sizeof(*object), object)>=6)
                    /*itu-t recomendation h 2250 version (version number)*/
                    /*0     1             2 3    4        5              */
                    m_callset(call,remoteVersion,object[5]);
            }
            q931ProcessMessage(cmiGetQ931((HCALL)call),message);
        }
        break;
        case cmProtocolH245:
        {
            if(m_callget(call,control))
                h245ProcessIncomingMessage(cmiGetControl((HCALL)call), message);
        }
        break;
        default:
      break;
    }
    emaUnlock((EMAElement)call);
    cmiAPIExit(hApp,(char*)"cmCallSimulateMessage: 0");
    return 0;
}



TRANSERR cmEvTransNewMessage(IN HSTRANSSESSION hsTransSession,
                             IN HATRANSSESSION haTransSession,
                             IN TRANSTYPE      type,
                             IN int            message,
                             IN void           *hMsgContext)
{
    callElem*call=(callElem*)haTransSession;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;
    if (hsTransSession);

    cmCallPreCallBack(hApp);
    if (!emaLock((EMAElement)call))
        return cmTransErr;

    {
        int nesting;
        if(app->cmMyCallEvent.cmEvCallRecvMessage)
        {
            cmiCBEnter((HAPP)app,(char*)"cmEvCallRecvMessage(haCall=0x%lx,hsCall=0x%lx)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
            nesting=emaPrepareForCallback((EMAElement)call);
            (app->cmMyCallEvent.cmEvCallRecvMessage)((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call, message);
            emaReturnFromCallback((EMAElement)call,nesting);
            cmiCBExit((HAPP)app,(char*)"cmEvCallRecvMessage");
        }
    }

    if (!emaWasDeleted((EMAElement)call) && (type==cmTransQ931Type))
    {
        /*Release old message context, if any */
        if ((call->hMsgContext != NULL) && app->cmEvCallReleaseMessageContext)
        {
            int nesting=emaPrepareForCallback((EMAElement)call);
            app->cmEvCallReleaseMessageContext(hMsgContext);
            emaReturnFromCallback((EMAElement)call,nesting);
        }
        call->hMsgContext=hMsgContext;
    }

    if (!emaWasDeleted((EMAElement)call))
        cmCallSimulateMessage((HCALL)call,message);
    emaUnlock((EMAElement)call);
    return cmTransOK;
}




/**************************************************************************************
 * cmEvTransBadMessage
 *
 * Purpose: To report to the user that a message for a session could not be decoded
 *          or encoded.
 *          Here, undecoded Q931 messages on a session cause a STATUS message to be
 *          sent in reply.
 *
 * Input:   hsTransSession  - The stack handle of the session.
 *          haTransSession  - The application handle of the session.
 *          type            - The type of the message (Q.931/H.245).
 *          msg             - The encoded message
 *          msgSize         - The encoded message size
 *          outgoing        - TRUE: outgoing message, FALSE-incoming message
 *
 **************************************************************************************/
TRANSERR cmEvTransBadMessage(IN HSTRANSSESSION  hsTransSession,
                             IN HATRANSSESSION  haTransSession,
                             IN TRANSTYPE       type,
                             BYTE               *msg,
                             int                msgSize,
                             BOOL               outgoing)
{
    if (hsTransSession);
    if (msg);
    if (msgSize);

    if ((type == cmTransQ931Type) && (!outgoing))
    {
        /* Q931... We should send a STATUS message for this one */
        q931DecodingFailure(cmiGetQ931((HCALL)haTransSession));
    }

    return cmTransOK;
}


/************************************************************************
 * enqueueDummyStates
 * purpose: Put an artificial state inside an empty states queue.
 *          This is used for callbacks other than cmEvCallStateChange such
 *          as cmEvNewCall.
 * input  : call        - Stack's call object
 * output : none
 * return : none
 ************************************************************************/
void enqueueDummyState(IN callElem* call)
{
    if (call->q_numStates != 0)
    {
        /* There's no reason to enqueue an Idle state if there are already states in this queue */
        return;
    }

    call->q_states[call->q_nextState] = RV_H323CALL_STATE_DUMMY;
    call->q_numStates = 1;
}


/************************************************************************
 * dequeueCallStates
 * purpose: Dequeue all pending states in the list of states
 * input  : app         - Stack instance
 *          call        - Stack's call object
 *          state       - State of call we want to process first
 *                        If set to cmCallStateIdle, then it is ignored
 *          stateMode   - Mode of the state we want to process first
 * output : none
 * return : none
 ************************************************************************/
void dequeueCallStates(
    IN cmElem*              app,
    IN callElem*            call,
    IN cmCallState_e        state,
    IN cmCallStateMode_e    stateMode)
{
    int nesting;

    call->q_numStates++;
    while ((call->q_numStates > 0) && (call->state != cmCallStateIdle)) /* Idle is the "last stop" */
    {
        /* cmCallStateInit is reserved for the use of the queue as a state that should be
           in the queue, but always not-handled. This way, we can for example use cmEvNewCall()
           as if it is a queued state to make sure all event callbacks are handled after it */
        if (state != cmCallStateInit)
        {
            /* handle the next state - its a real one */
#ifndef NOLOGSUPPORT
             static char *stateModes[]=
               {(char*)"",
                (char*)"cmCallStateModeDisconnectedBusy",
                (char*)"cmCallStateModeDisconnectedNormal",
                (char*)"cmCallStateModeDisconnectedReject",
                (char*)"cmCallStateModeDisconnectedUnreachable",
                (char*)"cmCallStateModeDisconnectedUnknown",
                (char*)"cmCallStateModeDisconnectedLocal",
                (char*)"cmCallStateModeConnectedControl",


                (char*)"cmCallStateModeConnectedCallSetup",
                (char*)"cmCallStateModeConnectedCall",
                (char*)"cmCallStateModeConnectedConference",

                (char*)"cmCallStateModeOfferingCreate",
                (char*)"cmCallStateModeOfferingInvite",
                (char*)"cmCallStateModeOfferingJoin",
                (char*)"cmCallStateModeOfferingCapabilityNegotiation",
                (char*)"cmCallStateModeOfferingCallIndependentSupplementaryService",
                (char*)"cmCallStateModeDisconnectedIncompleteAddress"};


            static char *states[]=
               {(char*)"cmCallStateDialtone",
                (char*)"cmCallStateProceeding",
                (char*)"cmCallStateRingBack",
                (char*)"cmCallStateConnected",
                (char*)"cmCallStateDisconnected",
                (char*)"cmCallStateIdle",
                (char*)"cmCallStateOffering",
                (char*)"cmCallStateTransfering",
                (char*)"",(char*)"",
                (char*)"cmCallStateIncompleteAddress",
                (char*)"cmCallStateWaitAddressAck"};

            int use_statemode;
#endif
#ifndef NOLOGSUPPORT
            use_statemode = ((state==cmCallStateDisconnected)||(state==cmCallStateConnected)||(state==cmCallStateOffering));
            cmiCBEnter((HAPP)app,"cmEvCallStateChanged(haCall=0x%p,hsCall=0x%p,state=%s,stateMode=%s)",
                emaGetApplicationHandle((EMAElement)call), call, states[state],
                use_statemode ? stateModes[stateMode+1] : "");
#endif
            nesting=emaPrepareForCallback((EMAElement)call);
            {
                call->state=state;
                call->stateMode=stateMode;

                if (app->cmMyCallEvent.cmEvCallStateChanged)
                    app->cmMyCallEvent.cmEvCallStateChanged((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,state,stateMode);
                else if (state==cmCallStateIdle)
                {
                    /* If the application has no state-change callback, we should close the call on IDLE state */
                    cmCallClose((HCALL)call);
                }

                /* If callback didn't delete the call and it's a connected one, check
                   if the control has already connected as well */
                if ( (!emaWasDeleted((EMAElement)call)) &&
                     (state == cmCallStateConnected)    &&
                     (stateMode == cmCallStateModeConnectedCallSetup) )
                   notifyControlState((HCALL)call);
            }
#ifndef NOLOGSUPPORT
            cmiCBExit((HAPP)app,"cmEvCallStateChanged");
#endif
            emaReturnFromCallback((EMAElement)call,nesting);
        }


        /* reduce state number */
        call->q_numStates--;
        if(call->q_numStates)
        {
            /* get next state from the queue */
            if (call->q_states[call->q_nextState] != RV_H323CALL_STATE_DUMMY)
            {
                state = (cmCallState_e) call->q_states[call->q_nextState];
                stateMode = (cmCallStateMode_e) call->q_stateModes[call->q_nextState];
            }
            else
                state = cmCallStateInit;
            call->q_nextState = (BYTE) ( (call->q_nextState+1)%RV_H323CALL_STATE_MAX );
        }
    }
}


/************************************************************************
 * notifyState
 * purpose: Notify the application about the state of the call
 * input  : hsCall      - Stack handle for the call
 *          state       - State of call
 *          stateMode   - Mode of the state
 * output : none
 * return : none
 ************************************************************************/
void notifyState(IN HCALL hsCall, IN cmCallState_e state, IN cmCallStateMode_e stateMode)
{
    callElem*call=(callElem*)hsCall;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

    /* check if a state is already being handled */
    if (call->q_numStates)
    {
        int newState;
        int oldState;
        if (call->q_numStates >= RV_H323CALL_STATE_MAX)
        {
            /* we may be in a state-loop - this shouldn't happen! */
            logPrint(app->logERR, RV_ERROR,
                     (app->logERR, RV_ERROR, "notifyState: We have a loop for hsCall=0x%p", hsCall));
            return;
        }

        /* place the new state in the state queue */
        newState = (call->q_nextState + call->q_numStates - 1) % RV_H323CALL_STATE_MAX;
        oldState = (newState+RV_H323CALL_STATE_MAX-1)%RV_H323CALL_STATE_MAX;
        if((call->q_states[oldState] != state) || (call->q_stateModes[oldState] != stateMode))
        {
            /* Seems like this state is not the same state as its predecessor - add it in */
            call->q_states[newState] = (BYTE) state;
            call->q_stateModes[newState] = (BYTE) stateMode;
            call->q_numStates++;
        }
        return;
    }

    /* No current states - handle it as is */

    /* don't report the same state twice */
    if ( (call->state == state) && (call->stateMode == stateMode) )
        return;

    /* enter state handle */
    dequeueCallStates(app, call, state, stateMode);
}


/************************************************************************
 * notifyControlState
 * purpose: Notify the application about the state of the control
 *          This function checks the control state and uses the call state
 *          notification function.
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void notifyControlState(IN HCALL hsCall)
{
    callElem*call=(callElem*)hsCall;
    if (m_callget(call,control))
    {
        switch(controlGetState(cmiGetControl((HCALL)call)))
        {
            case ctrlConnected:
                notifyState((HCALL)call,cmCallStateConnected,cmCallStateModeConnectedCall);
            break;
            case ctrlConference:
                notifyState((HCALL)call,cmCallStateConnected,cmCallStateModeConnectedConference);
            break;
            default:
                /* Do nothing */
            break;
        }
    }
}


/************************************************************************
 * callStartOK
 * purpose: Deal with an outgoing or incoming call in the Q931 side, after
 *          RAS was done (or skiped)
 * input  : call    - Stack handle for the call to dial
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callStartOK(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    int nodeId;
    cmTransportAddress tpkt;
    cmTransportAddress annexE;
    int ret=0;
    catStruct      callObj;

    /* See what's the direction of the call */
    if (m_callget(call,callInitiator))
    {
        /* Outgoing call - make sure we add it to the CAT */
        if ((ret = callObjectCreate(app, (HCALL)call, Outgoing, &callObj))<0)
            return ret;
        if ((call->hCatCall = catAdd(app->hCat, &callObj, (HCALL)call))==NULL)
            return RESOURCES_PROBLEM;

        {
            cmAnnexEUsageMode annexEBehavior=cmTransNoAnnexE;
            cmCallGetParam((HCALL)call,cmParamAnnexE,0,(INT32 *)&annexEBehavior,NULL);
            cmTransSetSessionParam(call->hsTransSession, cmTransParam_isAnnexESupported,&annexEBehavior);
        }

        /* Findout the address of the call's Q931 destination and try to connect to it */
        nodeId=((call->routeCallSignalAddress>=0))?
                    call->routeCallSignalAddress:
                    call->callSignalAddress;
        cmVtToTA(app->hVal,nodeId, &tpkt);

        if (call->annexECallSignalAddress>=0)
            cmVtToTA(app->hVal,call->annexECallSignalAddress, &annexE);
        else
            annexE=tpkt;

        cmTransSetAddress(call->hsTransSession, NULL, &tpkt,&annexE, NULL, cmTransQ931Conn, FALSE);

        ret = cmTransQ931Connect(call->hsTransSession);
    }
    else
    {
        /* Incoming call */
        BOOL incompleteAddress=0;
        {
            RVHCATCALL     hCatCall;

            /* CAT changes */
            if ((ret=callObjectCreate(app, (HCALL)call, Incoming, &callObj))<0)
                return ret;

            /* See if we've already got this call */
            if ((hCatCall=catFind(app->hCat, &callObj))!=NULL)
            {
                callElem*foundCall = (callElem*)catGetCallHandle(app->hCat, hCatCall);
                if (foundCall && m_callget(foundCall,callWithoutQ931))
                {
                    catUpdate(app->hCat, hCatCall, &callObj);
                    *foundCall=*call;
                    call=foundCall;
                }
            }
            else
            {
                /* No such call - add it to CAT as a new call */
                if ((call->hCatCall = catAdd(app->hCat, &callObj, (HCALL)call))==NULL)
                    return RESOURCES_PROBLEM;
            }

            /* Check if we've got a complete address or not */
            if (m_callget(call,remoteCanOverlapSend) && m_callget(call,enableOverlapSending))
                   incompleteAddress=1;
        }


        if (incompleteAddress)
        {
            /* Notify the application about the incomplete address */
            notifyState((HCALL)call,cmCallStateWaitAddressAck, (cmCallStateMode_e)0);
        }
        else
        {
            /* Continue to handle the incoming setup */
            enterCallSetupState(call);
        }
    }
    return ret;
}


typedef enum
{
    reasonReleaseComplete,
    reasonH245ConnectionClosed,
    reasonApplicationDrop,
    reasonDRQ,
    reasonQ931ConnectionClosed,
    reasonProtocolError
} dropReasonType;

/************************************************************************
 * reportDisconnectedState
 * purpose: Notify the application about a disconnected call
 * input  : call        - Stack handle for the call
 *          dropReason  - Reason the call was disconnected
 * output : none
 * return : none
 ************************************************************************/
void reportDisconnectedState(callElem*call,dropReasonType dropReason)
{
    cmCallState_e state=cmCallStateDisconnected;
    cmCallStateMode_e stateMode=cmCallStateModeDisconnectedNormal;
    BOOL doNotReport=FALSE;

    /* Make sure we notify application about disconnections only if it knowns about this call */
    if (m_callget(call,notified))
    {
        switch(dropReason)
        {
            case reasonReleaseComplete:
            {
                INT32 causeValue;
                cmReasonType reason;
                cmCallGetParam((HCALL)call,cmParamReleaseCompleteCause,0,&causeValue,NULL);
                cmCallGetParam((HCALL)call,cmParamReleaseCompleteReason,0,(INT32 *)&reason,NULL);
                if (causeValue==17 || reason==cmReasonTypeInConf)
                    stateMode=cmCallStateModeDisconnectedBusy;
                if (causeValue==28)
                    stateMode=cmCallStateModeDisconnectedIncompleteAddress;
            }
            break;
            case reasonDRQ:
            break;
            case reasonApplicationDrop:
                stateMode=cmCallStateModeDisconnectedLocal;
            break;
            case reasonQ931ConnectionClosed:
            break;
            case reasonH245ConnectionClosed:
            break;
        default:
            break;
        }
        if (stateMode==cmCallStateModeDisconnectedNormal)
            /*The state mode still not determined properly*/
        {
            switch(call->state)
            {
                case cmCallStateInit:
                case cmCallStateDialtone:
                    /* make sure this side initiated the call */
                    if (m_callget(call,callInitiator))
                        stateMode=cmCallStateModeDisconnectedUnreachable;
                break;
                case cmCallStateProceeding:
                case cmCallStateRingBack:
                    stateMode=cmCallStateModeDisconnectedReject;
                break;
                case cmCallStateTransfering:
                case cmCallStateDisconnected:
                case cmCallStateIdle:
                    /*If we in one of this states, then we already reported the disconnected state*/
                    doNotReport=TRUE;
                break;
            default:
                break;
            }
        }
        if (!doNotReport)
            notifyState((HCALL)call,state,stateMode);
    }
}

/************************************************************************
 * dropControl
 * purpose: Drop the H245 Control connection for a call.
 *          This function is used when calls are disconnected
 * input  : call    - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void dropControl(IN callElem* call)
{
    HCONTROL hCtrl;
    hCtrl = cmiGetControl((HCALL)call);

    /* See if we have to stop the control at all */
    if (m_callget(call, control))
    {
        /* First update the status so we don't get into stopControl() twice from 2 different
           threads: one on the incoming endSessionCommand and one for an outgoing one */
        m_callset(call, control, FALSE);
        stopControl(hCtrl);
    }

    /* Close all channels of this call */
    closeChannels(hCtrl);
}

/************************************************************************
 * dropRas
 * purpose: Notify the gatekeeper the call was disconnected before finishing
 *          off with this call
 * input  : call    - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void dropRas(callElem*call)
{
    int ret = 0;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

    if (!emaWasDeleted((EMAElement)call))
    {
        if (!app->manualRAS && !m_callget(call,dummyRAS))
        {
            /* Automatic RAS - send DRQ on call if necessary */
            ret = cmiAutoRASCallDrop((HCALL)call);
        }

        if (app->manualRAS || m_callget(call,dummyRAS) || (ret < 0))
        {
            if (!app->manualRAS)
                cmiAutoRASCallClose((HCALL)call);

            /* Manual RAS - continue to drop the call */
            callStopOK(call);
        }
    }
}


int callStartError(callElem*call)
{
    dropControl(call);
    reportDisconnectedState(call,reasonDRQ);
    if (!m_callget(call,callInitiator))
        q931CallDrop(cmiGetQ931((HCALL)call),-1);
    /* If GK has rejected our call then just close the call*/
    /* No RAS drop required*/
    callStopOK(call);
    return 0;
}

int callIncompleteAddress(callElem*call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    if (m_callget(call,callInitiator))
    {
        if (m_callget(call,enableOverlapSending))
        {
            if (app->cmMyCallEvent.cmEvCallIncompleteAddress)
            {
                int nesting;
                cmiCBEnter((HAPP)app,(char*)"cmEvCallIncompleteAddress(haCall=0x%lx,hsCall=0x%lx)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
                nesting=emaPrepareForCallback((EMAElement)call);
                app->cmMyCallEvent.cmEvCallIncompleteAddress((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
                emaReturnFromCallback((EMAElement)call,nesting);
                cmiCBExit((HAPP)app,(char*)"cmEvCallIncompleteAddress");
            }
        }
        else
        {
            dropControl(call);
            reportDisconnectedState(call,reasonDRQ);
            if (!m_callget(call,callInitiator))
                q931CallDrop(cmiGetQ931((HCALL)call),-1);
            /* If GK has rejected our call then just close the call*/
            /* No RAS drop required*/
            callStopOK(call);
        }
    }
    return 0;
}

int callStartRoute(callElem*call)
{
    cmTransportAddress ta;
    if (!m_callget(call,callInitiator))
    {
        cmCallSetParam((HCALL)call,cmParamFacilityReason,0,cmReasonTypeRouteCallToGatekeeper,NULL);
        cmGetGKCallSignalAddress((HAPP)emaGetInstance((EMAElement)call),&ta);
        cmCallSetParam((HCALL)call,cmParamAlternativeAddress,0,0,(char*)&ta);
        q931CallFacility(cmiGetQ931((HCALL)call),-1);
    }
    else
        callStartError(call);
    return 0;
}

/************************************************************************
 * callDial
 * purpose: Dials a call or answers incoming Setup requests.
 *          This function checks if we have to ARQ the GK on this call or
 *          not and continues the call setup process.
 * input  : call    - Stack handle for the call to dial
 * output : none
 * return : negative on error
 ************************************************************************/
static int callDial(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    cmTransportAddress ta;
    m_callset(call,dummyRAS,FALSE);

    if (app->manualRAS)
        m_callset(call,dummyRAS,TRUE);
    else
    {
        if (call->preGrantedArqUse>=0)
        {
            /* PreGrantedARQ is used */
            if (m_callget(call,callInitiator))
            {
                /* Let's see if we've got a PreGranted call making on this outgoing call */
                if (app->rasGranted&makeCall)
                {
                    if ( !(app->rasGranted&useGKCallSignalAddressToMakeCall) && (call->preGrantedArqUse==cmPreGrantedArqDirect))
                    {
                        /*direct call without ARQ is allowed*/
                        if (cmCallGetParam((HCALL)call,cmParamDestCallSignalAddress,0,0,(char*)&ta)>=0)
                        {
                            cmCallSetParam((HCALL)call,cmParamDestinationIpAddress,0,0,(char*)&ta);
                        }
                        else
                        {
                            /*if not exist address do call via GK.*/
                            if (app->rasGrantedProtocol==1)
                            {
                                cmCallSetParam((HCALL)call,cmParamAnnexE,0,cmTransUseAnnexE,NULL);
                                cmCallSetParam((HCALL)call,cmParamDestinationAnnexEAddress,0,0,(char*)&app->annexEAddress);
                            }
                            else
                            {
                                cmCallSetParam((HCALL)call,cmParamAnnexE,0,cmTransNoAnnexE,NULL);
                                cmGetGKCallSignalAddress((HAPP)app,&ta);
                                cmCallSetParam((HCALL)call,cmParamDestinationIpAddress,0,0,(char*)&ta);
                            }
                        }
                    }
                    else
                    {
                        /*routed call without ARQ   */
                        if (app->rasGrantedProtocol==1)
                        {
                            cmCallSetParam((HCALL)call,cmParamAnnexE,0,cmTransUseAnnexE,NULL);
                            cmCallSetParam((HCALL)call,cmParamDestinationAnnexEAddress,0,0,(char*)&app->annexEAddress);
                        }
                        else
                        {
                            cmCallSetParam((HCALL)call,cmParamAnnexE,0,cmTransNoAnnexE,NULL);
                            cmGetGKCallSignalAddress((HAPP)app,&ta);
                            cmCallSetParam((HCALL)call,cmParamDestinationIpAddress,0,0,(char*)&ta);
                        }
                    }
                    m_callset(call,dummyRAS,TRUE);
                }
            }
            else if ((app->rasGranted&answerCall) && !(app->rasGranted&useGKCallSignalAddressToAnswer))
            {
                /* It's an incoming call and we can answer the Setup without an ARQ to the GK */
                m_callset(call,dummyRAS,TRUE);
            }
        }
    }

    call->rate=call->newRate;

    if (!m_callget(call,dummyRAS))
    {
        /*Automatic RAS*/
        return cmiAutoRASCallDial((HCALL)call);
    }
    else
    {
        cmiAutoRASCallSetUnsolicitedIRR((HCALL)call, app->irrFrequencyInCall);
        /* No need to deal with any RAS messages here - continue with the call */
        return callStartOK(call);
    }
}

/**********************************************************************
* cmSetupEnd
* This function is responsible for the end of the scenario of receiving
* a setup message. It was taken out of the cmSetup in order to give the
* endpoint time to receive ACF before sending AutoConnect (when autoRas
* and autoConnect are on).
************************************************************************/
int cmSetupEnd(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int autoAnswer,manualAccept,ret = 0;

    manualAccept         = (pvtGet(hVal,pvtGetChild(hVal,app->q931Conf,__q931(manualAccept),NULL),NULL,NULL,NULL,NULL)>=0);
    if (pvtGetChild(hVal,app->q931Conf,__q931(autoAnswer),NULL)>=0)
    {
        /* Configuration set to automatically answer any incoming call */
        autoAnswer = 1;
    }
    else
    {
        autoAnswer = 0;
        /* Check if we automatically send Alerting for incoming calls */
        if (!manualAccept)
            q931CallAccept(cmiGetQ931((HCALL)call),-1);
    }

    /*If we are configured to answer the call automatically then do it*/
    if (autoAnswer && call->state!=cmCallStateConnected)
        ret = callAnswer(call);

    return ret;
}

int cmSetup(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int manualCallProceeding, manualAccept, ret;

    manualCallProceeding = (pvtGet(hVal,pvtGetChild(hVal,app->q931Conf,__q931(manualCallProceeding),NULL),NULL,NULL,NULL,NULL)>=0);
    manualAccept         = (pvtGet(hVal,pvtGetChild(hVal,app->q931Conf,__q931(manualAccept),NULL),NULL,NULL,NULL,NULL)>=0);

    /* in case of automatic alerting we don't want to send the callProceeding, the
    alerting message will be enough */
    if ( (manualAccept) && (!manualCallProceeding) )
    {
        /* Build and send the CALL PROCEEDING message */
        q931CallCallProceeding(cmiGetQ931((HCALL)call),-1);
        m_callset(call,enableOverlapSending,FALSE);
    }

    /* We need to analyze the fast start before sending the automatic CONNECT message back */
    if ((message >=0) && !emaWasDeleted((EMAElement)call))
    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            analyzeFastStartMsg(call, message);
    }

    if (emaWasDeleted((EMAElement)call))
        return 0;

    ret = callDial(call);
    if ((ret >= 0) && (m_callget(call,dummyRAS)) && (call->state != cmCallStateWaitAddressAck))
        ret = cmSetupEnd(call);

    return ret;
}

int cmCallProceeding(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int tmpNodeId,tmpNodeId1;

    __pvtGetNodeIdByFieldIds(tmpNodeId,hVal,message,
                {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation)
                 _q931(h323_uu_pdu) _q931(h323_message_body) _anyField LAST_TOKEN});

    tmpNodeId1=pvtGetChild(hVal,tmpNodeId,__q931(destinationInfo),NULL);
    cmCallSetParam((HCALL)call,cmParamFullDestinationInfo,0,tmpNodeId1,NULL);

    notifyState((HCALL)call,cmCallStateProceeding,(cmCallStateMode_e)0);

    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            if (cmFastStartReply(call,tmpNodeId) >= 0)
                deleteFastStart(call);
    }
    return 0;
}

int cmAlerting(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int tmpNodeId,tmpNodeId1;

    __pvtGetNodeIdByFieldIds(tmpNodeId,hVal,message,
                {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation)
                 _q931(h323_uu_pdu) _q931(h323_message_body) _anyField LAST_TOKEN});

    tmpNodeId1=pvtGetChild(hVal,tmpNodeId,__q931(destinationInfo),NULL);
    cmCallSetParam((HCALL)call,cmParamFullDestinationInfo,0,tmpNodeId1,NULL);

    notifyState((HCALL)call,cmCallStateRingBack,(cmCallStateMode_e)0);

    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            if (cmFastStartReply(call,tmpNodeId) >= 0)
                deleteFastStart(call);
    }
    return 0;
}

int cmConnect(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    controlElem *ctrl=(controlElem *)cmiGetControl((HCALL)call);
    HPVT hVal=app->hVal;
    int messageBody,destInfo;

    __pvtGetNodeIdByFieldIds(messageBody,hVal,message,
                 {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation)
                 _q931(h323_uu_pdu) _q931(h323_message_body) _anyField LAST_TOKEN});


    destInfo=pvtGetChild(hVal,messageBody,__q931(destinationInfo),NULL);
    cmCallSetParam((HCALL)call,cmParamFullDestinationInfo,0,destInfo,NULL);

    enterCallConnectedState(call);
    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            cmFastStartReply(call,messageBody);
    }

    deleteFastStart(call);
    m_callset(call, fastStartFinished, TRUE);
    cmcReadyEvent(ctrl);
    return 0;
}


int cmReleaseComplete(callElem*call, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (message);
    cmTimerReset(hApp,&(call->timer));

    /* Drop Transactions - put it in Idle state */
    cmTransDrop(call->hsTransSession);

    dropControl(call);
    reportDisconnectedState(call,reasonReleaseComplete);
    dropRas(call);
    return 0;
}

int cmStatus(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;

    if (app->cmMyCallEvent.cmEvCallStatus)
    {
        cmCallStatusMessage callStatusMsg;
        int handle,nesting;
        handle=pvtGetChild2(hVal, message, __q931(message),__q931(status));
        cmCallStatusMsg2Struct(hVal,handle,&callStatusMsg);
        cmiCBEnter((HAPP)app,(char*)"cmEvCallStatus(haCall=0x%lx,hsCall=0x%lx,handle=%ld)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,handle);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallStatus((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,&callStatusMsg);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallStatus ");
    }
    return 0;
}

int cmFacility(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int uuNodeId;
    BOOL proceed=1;
    int tmp;
    INTPTR fieldId=RVERROR;
    BOOL empty=FALSE;
    int  i=0,j=0;
    BOOL firstE164=TRUE;

    __pvtGetNodeIdByFieldIds(uuNodeId,hVal,message,
        {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu) LAST_TOKEN});

    tmp=pvtGetChild(hVal,uuNodeId,__q931(h323_message_body),NULL);
    empty=(pvtGetChild(hVal,tmp,__q931(empty),NULL)>=0);

    tmp=pvtChild(hVal,tmp);

    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            if (cmFastStartReply(call,tmp) >= 0)
                deleteFastStart(call);
    }

    if (!empty)
    __pvtGetByFieldIds(tmp,hVal,tmp,
        {_q931(reason) _anyField LAST_TOKEN},&fieldId, NULL,NULL);


    if (fieldId!=__q931(routeCallToGatekeeper))
    if (app->cmMyCallEvent.cmEvCallFacility)
    {
        int nesting;

        cmiCBEnter((HAPP)app,(char*)"cmEvCallFacility(haCall=0x%lx,hsCall=0x%lx,handle=%ld,proceed=&)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallFacility((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message,&proceed);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallFacility proceed=%s",(proceed)?"TRUE":"FALSE");
    }

    if (proceed && !empty && !emaWasDeleted((EMAElement)call))
    {
        cmCallStateMode_e stateMode = 0;

        switch(fieldId)
        {
            case __q931(callForwarded):
            {
                /* remove the destination addresses before setting the new ones */
                i = cmCallGetNumOfParams((HCALL)call, cmParamDestinationAddress);
                for(; i>0; i--)
                {
                    cmCallDeleteParam((HCALL)call,cmParamDestinationAddress,i);
                }
                /* fix for AudioCodes - also remove the source addresses if we are the call's destination */
                if (!m_callget(call,callInitiator))
                {
                    i = cmCallGetNumOfParams((HCALL)call, cmParamSourceAddress);
                    for(; i>0; i--)
                    {
                        cmCallDeleteParam((HCALL)call,cmParamSourceAddress,i);
                    }
                }
                i=0;
                m_callset(call,newCIDRequired,TRUE);
                stateMode = cmCallStateModeTransferingForwarded;
            }
            case __q931(routeCallToMC):
                cmCallDeleteParam((HCALL)call,cmParamCalledPartyNumber,0);
                cmCallDeleteParam((HCALL)call,cmParamDestCallSignalAddress,0);
            case __q931(routeCallToGatekeeper):
            {
                if (fieldId==__q931(routeCallToMC))
                {
                    char cid[16];
                    INT32 size=16;
                    cmCallSetParam((HCALL)call,cmParamConferenceGoal,0,cmJoin,NULL);
                    cmCallGetParam((HCALL)call,cmParamFacilityCID,0,&size,cid);
                    cmCallSetParam((HCALL)call,cmParamCID,0,size,cid);
                    stateMode = cmCallStateModeTransferingRouteToMC;
                }
                if (fieldId==__q931(routeCallToGatekeeper))
                {
                    stateMode = cmCallStateModeTransferingRouteToGatekeeper;
                }

                while(1)
                {
                    char buffer[256];
                    cmAlias alias;
                    alias.string=buffer;
                    buffer[0]=0;

                    if (cmCallGetParam((HCALL)call,cmParamAlternativeAliasAddress,i++,NULL,(char*)&alias)>=0)
                    {
                        /* if this is the first e164 alias */
                        if (alias.type==cmAliasTypeE164 && firstE164)
                        {
                            cmCallSetParam((HCALL)call,cmParamCalledPartyNumber,0,0,(char*)&alias);
                            firstE164 = FALSE;
                        }
                        cmCallSetParam((HCALL)call,cmParamDestinationAddress,j++,0,(char*)&alias);
                    }
                    else
                        break;
                }
                cmCallDeleteParam((HCALL)call,cmParamCallSignalAddress,0);
                m_callset(call,remoteVersion,0);
                {
                    cmTransportAddress ta;

                    if ((cmCallGetParam((HCALL)call,cmParamAlternativeAddress,0,NULL,(char*)&ta)>=0) &&
                        (ta.ip != 0) && (ta.port != 0))
                    {
                        if (fieldId==__q931(routeCallToGatekeeper))
                        {
                            cmCallSetParam((HCALL)call,cmParamRouteCallSignalAddress,0,0,(char*)&ta);
                        }
                        cmCallSetParam((HCALL)call,cmParamDestinationIpAddress,0,0,(char*)&ta);
                        if (fieldId!=__q931(routeCallToGatekeeper))
                        {
                            cmCallDeleteParam((HCALL)call,cmParamRouteCallSignalAddress,0);
                            cmCallSetParam((HCALL)call,cmParamDestCallSignalAddress,0,0,(char*)&ta);
                        }
                    }
                }
                dropControl(call);
                cmCallSetParam((HCALL)call,cmParamReleaseCompleteCause,0,16,NULL);
                cmCallSetParam((HCALL)call,cmParamReleaseCompleteReason,0,cmReasonTypeFacilityCallDeflection,NULL);
                q931CallDrop(cmiGetQ931((HCALL)call),-1);
                notifyState((HCALL)call,cmCallStateTransfering, stateMode);
                dropRas(call);
            }
            break;
        }
    }
    return 0;
}

int cmProgress(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int tmpNodeId,tmpNodeId1;
    if (message || call);

    __pvtGetNodeIdByFieldIds(tmpNodeId,app->hVal,message,
                {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation)
                 _q931(h323_uu_pdu) _q931(h323_message_body) _anyField LAST_TOKEN});

    tmpNodeId1=pvtGetChild(hVal,tmpNodeId,__q931(destinationInfo),NULL);
    cmCallSetParam((HCALL)call,cmParamFullDestinationInfo,0,tmpNodeId1,NULL);
    {
        INT32 establish=1;
        cmCallGetParam((HCALL)call,cmParamEstablishH245,0,&establish,NULL);
        if (establish)
            if (cmFastStartReply(call,tmpNodeId) >= 0)
                deleteFastStart(call);
    }
    if (app->cmMyCallEvent.cmEvCallProgress)
    {
        int nesting;
        cmiCBEnter((HAPP)app,(char*)"cmEvCallProgress(haCall=0x%lx,hsCall=0x%lx,handle=%ld)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallProgress((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallProgress");
    }
    return 0;
}

int cmSetupAcknowledge(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

    if (message);

    if (m_callget(call,enableOverlapSending))
    {
        if (app->cmMyCallEvent.cmEvCallIncompleteAddress)
        {
            int nesting;
            cmiCBEnter((HAPP)app,(char*)"cmEvCallIncompleteAddress(haCall=0x%lx,hsCall=0x%lx)",emaGetApplicationHandle((EMAElement)call),call);
            nesting=emaPrepareForCallback((EMAElement)call);
            app->cmMyCallEvent.cmEvCallIncompleteAddress((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
            emaReturnFromCallback((EMAElement)call,nesting);
            cmiCBExit((HAPP)app,(char*)"cmEvCallIncompleteAddress");
        }
    }

    else
    {
        int sMessage=callGetMessage((HCALL)call,cmQ931status);
        {
            HPVT hVal=cmGetValTree((HAPP)app);
            int tmpNodeId,tmpNodeId1;
            __pvtGetNodeIdByFieldIds(tmpNodeId,hVal,sMessage, {_q931(message) _anyField LAST_TOKEN});
            __pvtBuildByFieldIds(tmpNodeId1,hVal,tmpNodeId,
                                {_q931(cause) _q931(octet4) _q931(causeValue) LAST_TOKEN},98,NULL);
            __pvtBuildByFieldIds(tmpNodeId1,hVal,tmpNodeId,
                                {_q931(callState) _q931(callStateValue) LAST_TOKEN},q931GetCallState(cmiGetQ931((HCALL)call)),NULL);
        }
        sendCallMessage((HCALL)call,message);
        callReleaseMessage((HCALL)call,cmQ931status);
    }
    return 0;
}

int cmInformation(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int tmpNodeId,tmpNodeId1,q931NodeId;
    int nesting;

    if (call->state == cmCallStateWaitAddressAck)
    {
        int size;
        char address[256];
        BOOL sendingComplete=FALSE;
        int nesting;
        address[0]=0;
        __pvtGetNodeIdByFieldIds(q931NodeId,hVal,message, {_q931(message) _anyField LAST_TOKEN});
        tmpNodeId=pvtGetChild(hVal, q931NodeId, __q931(calledPartyNumber), NULL);
        if (tmpNodeId>0)
        {
            tmpNodeId1 = pvtGetChild(app->hVal,tmpNodeId,__q931(numberDigits),NULL);
            size=pvtGetString(app->hVal,tmpNodeId1,256,address);
            address[size]=0;
        }
        if (pvtGetChild(hVal, q931NodeId,__q931(sendingComplete),NULL)>=0)
            sendingComplete=TRUE;
        cmiCBEnter((HAPP)app,(char*)"cmEvCallAdditionalAddress (haCall=0x%lx,hsCall=0x%x,message=%d)",emaGetApplicationHandle((EMAElement)call),call,message);
        nesting=emaPrepareForCallback((EMAElement)call);
        ifE(app->cmMyCallEvent.cmEvCallAdditionalAddress)((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,address,sendingComplete);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallAdditionalAddress");
    }

    /* report the information only if the previous callback did not change the state to
       Disconnected or Idle */
    if ((app->cmMyCallEvent.cmEvCallUserInfo) && !emaWasDeleted((EMAElement)call) &&
        (call->state != cmCallStateDisconnected) && (call->state != cmCallStateIdle))
    {
        cmiCBEnter((HAPP)app,(char*)"cmEvCallUserInfo (haCall=0x%lx,hsCall=0x%x,message=%d)",emaGetApplicationHandle((EMAElement)call),call,message);
        nesting=emaPrepareForCallback((EMAElement)call);
        (app->cmMyCallEvent.cmEvCallUserInfo)((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallUserInfo");
    }
    return 0;
}

int cmNotify(callElem*call, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    if (app->cmMyCallEvent.cmEvCallNotify)
    {
        int nesting;
        cmiCBEnter((HAPP)app,(char*)"cmEvCallNotify(haCall=0x%lx,hsCall=0x%lx,handle=%ld)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallNotify((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,message);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallNotify");
    }
    return 0;
}

/************************************************************************
 * cmIndicate
 * purpose: Indicate and process an incoming Q931 message
 * input  : call    - Stack handle for the call
 *          message - Message root node
 *          msgType - Type of the message
 * output : none
 * return : Non-negative value on success
 *          negative value on failure
 ************************************************************************/
int cmIndicate(IN HCALL call, IN int message, IN int msgType)
{
    callElem* callE=(callElem*)call;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);

    /* Reset the call's timer - we've got a message from the net
       unless we have disconnected the control and we wait just for releaseComplete using
       the timer as the postControlDisconnectionDelay timer */
    if (!( m_callget(callE,controlDisconnected) && (msgType !=cmQ931releaseComplete) ))
        cmTimerReset(hApp,&(callE->timer));

    /* Make sure we ignore incoming messages if we're already in the idle state */
    if ((msgType != cmQ931releaseComplete) && (callE->state == cmCallStateIdle))
        return 0;

    /* Act by the message itself */
    switch(msgType)
    {
        case cmQ931setup            :cmSetup(callE,message);            break;
        case cmQ931callProceeding   :cmCallProceeding(callE,message);   break;
        case cmQ931connect          :cmConnect(callE,message);          break;
        case cmQ931alerting         :cmAlerting(callE,message);         break;
        case cmQ931releaseComplete  :cmReleaseComplete(callE,message);  break;
        case cmQ931status           :cmStatus(callE,message);           break;
        case cmQ931facility         :cmFacility(callE,message);         break;
        case cmQ931setupAck         :cmSetupAcknowledge(callE,message); break;
        case cmQ931progress         :cmProgress(callE,message);         break;
        case cmQ931information      :cmInformation(callE,message);      break;
        case cmQ931notify           :cmNotify(callE,message);           break;

        default :   /*Process Q.931 protocol errors*/
            dropControl(callE);
            reportDisconnectedState(callE,reasonProtocolError);
            dropRas(callE);
        break;
    }
    return 0;
}

/************************************************************************
 * cmEvRASNewCall
 * purpose: Handle a new call that was received by a RAS ARQ message
 * input  : hApp        - Application's handle for a stack instance
 *        : lphCatCall  - the pointer where to put the CAT handle
 *        : callObj     - call parameters
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmEvRASNewCall(
    IN HAPP         hApp,
    IN RVHCATCALL*  lphCatCall,
    IN catStruct*   callObj)
{
    callElem call,*ce;
    int someId;
    INT32 maxCalls;
    BOOL earlyH245;
    cmElem*app=(cmElem*)hApp;
    HPVT hVal=app->hVal;

    /* Check that it is possible to accept the call*/
    pvtGet(hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(maxCalls),NULL),NULL,NULL,&(maxCalls),NULL);
    if (app->busy<maxCalls)
    {
        /* Set a new call object information */
        memset(&call,0xff,sizeof(callElem));

        call.appHandles[cmQ931TpktChannel]=NULL;
        call.appHandles[cmH245TpktChannel]=NULL;

        call.hMsgContext=NULL;

        call.flags=0;
        /*call.enableOverlapSending=0;
        call.control=FALSE;
        call.notified=FALSE;
        call.callInitiator=0;
        call.control=FALSE;
        call.callWithoutQ931=TRUE;
        call.isParallelTunneling     = FALSE;
        call.isMultiplexed           = FALSE;
        */

        call.hCatCall=NULL;
        call.state=cmCallStateInit;
        call.newRate = 0;
        call.rate = 0;
        m_callset(&call, overrideCID, TRUE);
        m_callset(&call, callWithoutQ931, TRUE);

        call.q_nextState = 0;
        call.q_numStates = 0;

        /* transport layer defaults */
        call.hsTransSession=NULL;
        m_callset(&call,h245Tunneling ,(pvtGetChild(hVal,app->q931Conf,__q931(h245Tunneling),NULL)>=0));
        m_callset(&call,notEstablishControl , (pvtGetChild(hVal,app->q931Conf,__q931(notEstablishControl),NULL)>=0));
        m_callset(&call,isMultiplexed,FALSE); /* Default for incoming calls is non-multiplexed */
        m_callset(&call,shutdownEmptyConnection ,FALSE);
        earlyH245 = (pvtGetChild(hVal,app->q931Conf,__q931(earlyH245),NULL)>=0);
        m_callset(&call,h245Stage , (earlyH245)?cmTransH245Stage_early:cmTransH245Stage_connect);

        someId=pvtGetChild(app->hVal,app->rasConf,__q931(preGrantedArqUse),NULL);
        if (pvtGetChild(app->hVal,someId,__q931(routed),NULL) >= 0)  call.preGrantedArqUse=cmPreGrantedArqRouted;
        if (pvtGetChild(app->hVal,someId,__q931(direct),NULL) >= 0)  call.preGrantedArqUse=cmPreGrantedArqDirect;


        {
            int nodeId;
            INT32 annexE=0;

            call.annexE=cmTransNoAnnexE;

            if ((nodeId=pvtGetChild(app->hVal,app->q931Conf,__q931(useAnnexE),NULL))>=0)
            {
                int usageMode = pvtGetSyntaxIndex(app->hVal,pvtChild(app->hVal,pvtGetChild(app->hVal,nodeId,__q931(protocolPreference),NULL)))-1;

                call.annexE = (cmAnnexEUsageMode)usageMode;
                if (((int)call.annexE) < 0)
                {
                    call.annexE=cmTransNoAnnexE;
                    pvtGetChildValue(app->hVal,nodeId,__q931(defaultProtocol),&annexE,NULL);
                    if (annexE)
                        call.annexE=cmTransUseAnnexE;
                }
            }
        }

        /* Add the new call to the calls database */
        ce=(callElem *)emaAdd(app->hCalls,NULL);
        if (ce == NULL)
            return RVERROR;

        memcpy(ce,&call,sizeof(call));
        fastStartCallInit(ce);
        cmiAutoRASCallCreate((HCALL)ce);
        cmiSetNumberOfChannelsForCall((HCALL)ce,app->maxChannels);

        if (ce)
        {
            /* Add this call to CAT */
            *lphCatCall=ce->hCatCall = catAdd(app->hCat,callObj,(HCALL)ce);
            {
                /* Set the call's parameters from the CAT struct */
                cmCallSetParam((HCALL)ce,cmParamCID,0,0,(char*)callObj->cid);
                cmCallSetParam((HCALL)ce,cmParamCallID,0,0,(char*)callObj->callID);
                cmCallSetParam((HCALL)ce,cmParamRASCRV,0,(int)(callObj->rasCRV),NULL);
            }

            /* Notify application about the new incoming call */
            m_callset(ce,notified,TRUE);
            if (app->cmMyEvent.cmEvNewCall)
            {
                HAPPCALL haCall = NULL;
                enqueueDummyState(ce);
                cmiCBEnter((HAPP)app,(char*)"cmEvNewCall(hApp=0x%p,hsCall=0x%p,lphaCall)",app,call);
                app->cmMyEvent.cmEvNewCall((HAPP)app,(HCALL)ce,&haCall);
                emaSetApplicationHandle((EMAElement)ce,(void*)haCall);
                cmiCBExit((HAPP)app,(char*)"cmEvNewCall(*lphaCall=0x%p)",emaGetApplicationHandle((EMAElement)ce));
                dequeueCallStates(app, ce, cmCallStateInit, (cmCallStateMode_e)-1);

                if (app->mibEvent.h341AddNewCall)
                    app->mibEvent.h341AddNewCall(app->mibHandle, (HCALL)ce);
            }

            /* We've got a new call here... */
            app->busy++;

            return cmTransOK;
        }
    }
    return RVERROR;
}

/**************************************************************************************
 * cmEvTransNewSession
 * Purpose: To report to the user that a new session was created due to a new
 *          incoming message. This callback is called only when a new message actually
 *          arrives and not when the connection is established.
 *          The CM creates a new incoming call object in this event
 * Input:   hsTrans         - The stack handle of the instance.
 *          haTrans         - The application handle of the instance.
 *          hsTransSession  - The stack handle of the session.
 *          pvtNode         - the node of the SETUP message that caused the creation.
 * Output:  haTransSession  - The application handle of the session.
 *          cause           - In case of rejection, the cause (as in RELEASE COMPLETE).
 *          reasonNameId    - In case of rejection, the reason name Id (as in RELEASE COMPLETE).
 **************************************************************************************/
TRANSERR cmEvTransNewSession(IN  HAPPTRANS        hsTrans,
                             IN  HAPPATRANS       haTrans,
                             IN  HSTRANSSESSION   hsTransSession,
                             OUT HATRANSSESSION   *haTransSession,
                             IN  int              message,
                             OUT int              *cause,
                             OUT INTPTR           *reasonNameId)
{
    cmElem* app=(cmElem*)haTrans;
    callElem call,*ce;
    int someId;
    INT32 maxCalls;
    HPVT hVal=app->hVal;
    catStruct callObj;
    RVHCATCALL hCatCall;
    BOOL rasCall = FALSE;

    if (hsTrans);
    if (callObjectCreateFromMessage(app, message, &callObj)<0)
        return cmTransErr;

    /* If we need to reject the call the cause and reason for the rejecten are taken from the configuration*/
    if (cause)
    {
        *cause=17;
        pvtGetChildValue(hVal,app->q931Conf,__q931(busyCause),(INT32 *)cause,NULL);
    }
    if (reasonNameId)
    {
        *reasonNameId=__q931(inConf);
        pvtGet(hVal,pvtChild(hVal, pvtGetChild(hVal,app->q931Conf,__q931(busyReason),NULL)),reasonNameId,NULL,NULL,NULL);
    }

    /*It is possible that there are already call started from RAS*/
    if ((hCatCall=catFind(app->hCat, &callObj))!=NULL)
    {
        /* Found a call - update it if it's a RAS call */
        ce = (callElem*)catGetCallHandle(app->hCat, hCatCall);
        rasCall = TRUE;
        if (ce && m_callget(ce,callWithoutQ931))
            catUpdate(app->hCat, hCatCall, &callObj);
    }
    else
    {
        /* Check that it is possible to accept the call*/
        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(maxCalls),NULL),NULL,NULL,&(maxCalls),NULL);
        if (app->busy<maxCalls)
        {
            /* Create the new call object */
            memset(&call,0xff,sizeof(callElem));

            call.appHandles[cmQ931TpktChannel]=NULL;
            call.appHandles[cmH245TpktChannel]=NULL;
            call.flags=0;
            /*
            call.enableOverlapSending=0;
            call.preGrantedArqUse=0;
            call.control=FALSE;
            call.notified=FALSE;
            call.callInitiator=0;
            call.control=FALSE;
            */

            call.hMsgContext=NULL;
            call.hCatCall=NULL;
            call.state=cmCallStateInit;
            call.newRate = 0;
            call.rate = 0;
            m_callset(&call, overrideCID, TRUE);

            call.q_nextState = 0;
            call.q_numStates = 0;

            someId=pvtGetChild(app->hVal,app->rasConf,__q931(preGrantedArqUse),NULL);
            if (pvtGetChild(app->hVal,someId,__q931(routed),NULL) >= 0)  call.preGrantedArqUse=cmPreGrantedArqRouted;
            if (pvtGetChild(app->hVal,someId,__q931(direct),NULL) >= 0)  call.preGrantedArqUse=cmPreGrantedArqDirect;

            ce=(callElem *)emaAdd(app->hCalls,NULL);
            if (ce)
            {
                memcpy(ce,&call,sizeof(call));
                fastStartCallInit(ce);
                cmiAutoRASCallCreate((HCALL)ce);
                cmiSetNumberOfChannelsForCall((HCALL)ce,app->maxChannels);
            }
        }
        else return cmTransErr;
    }

    if (ce && emaLock((EMAElement)ce))
    {
        int q931NodeId,perNodeId;
        int tmpNodeId;
        INT32 t302;

        m_callset(ce,callWithoutQ931,FALSE);
        ce->hsTransSession=hsTransSession;
        *haTransSession=(HATRANSSESSION)ce;
        callInitParameters((HCALL)ce);
        __pvtGetNodeIdByFieldIds(someId,app->hVal,app->rasConf, {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN});

        /* for a call that has just been created set the initial transport states are according to the
           config.val file, otherwise set it to what is written on the call (and might have been changed
           by setParam) */
        if (!rasCall)
        {
            BOOL earlyH245;

            /* Add as new call */
            app->busy++;

            m_callset(ce,h245Tunneling , (pvtGetChild(hVal,app->q931Conf,__q931(h245Tunneling),NULL)>=0));
            m_callset(ce,isParallelTunneling ,FALSE);
            m_callset(ce,isMultiplexed,FALSE); /* Default for incoming calls is non-multiplexed */
            m_callset(ce,shutdownEmptyConnection ,FALSE);

            m_callset(ce,notEstablishControl ,(pvtGetChild(hVal,app->q931Conf,__q931(notEstablishControl),NULL)>=0));
            earlyH245 = (pvtGetChild(hVal,app->q931Conf,__q931(earlyH245),NULL)>=0);
            m_callset(ce,h245Stage ,(earlyH245)?cmTransH245Stage_early:cmTransH245Stage_connect);
        }
        cmCallSetParam((HCALL)ce, cmParamH245Tunneling, 0, (INT32)m_callget(ce,h245Tunneling), NULL);
        cmCallSetParam((HCALL)ce, cmParamH245Parallel, 0, (INT32)m_callget(ce,isParallelTunneling), NULL);
        cmCallSetParam((HCALL)ce, cmParamIsMultiplexed, 0, (INT32)m_callget(ce,isMultiplexed), NULL);
        cmCallSetParam((HCALL)ce, cmParamShutdownEmptyConnection, 0, (INT32)m_callget(ce,shutdownEmptyConnection), NULL);
        cmCallSetParam((HCALL)ce, cmParamEstablishH245, 0, (INT32)(!m_callget(ce,notEstablishControl)), NULL);
        cmCallSetParam((HCALL)ce, cmParamH245Stage, 0, (INT32)m_callget(ce,h245Stage), NULL);

        if (someId >= 0)
        {
            cmCallSetParam((HCALL)ce,cmParamFullDestinationInfo,0,someId,NULL);
            cmCallSetParam((HCALL)ce,cmParamFullSourceInfo,0,someId,NULL);
        }
        else
        {
            __pvtBuildByFieldIds(someId, app->hVal, app->rasConf, {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN}, 0, NULL);
            pvtAdd(app->hVal, someId, __q931(mc), 0, NULL, NULL);
            pvtAdd(app->hVal, someId, __q931(undefinedNode), 0, NULL, NULL);
            cmCallSetParam((HCALL)ce,cmParamFullDestinationInfo,0,someId,NULL);
            cmCallSetParam((HCALL)ce,cmParamFullSourceInfo,0,someId,NULL);
        }

        m_callset(ce,enableOverlapSending ,(pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(overlappedSending),NULL),NULL,NULL,NULL,NULL) >= 0));

        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(responseTimeOut),NULL),NULL,NULL,&t302,NULL);
        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(t302),NULL),NULL,NULL,&t302,NULL);
        t302*=1000;

        q931CallCreate((HQ931)cmiGetQ931((HCALL)ce), -1, t302, -1, -1, -1, -1);

        /* Get Call Identification Parameters from the message */
        /* CRV */
        ce->crv=callObj.crv;

        __pvtGetNodeIdByFieldIds(q931NodeId,hVal,message, {_q931(message) _anyField LAST_TOKEN});
        __pvtGetNodeIdByFieldIds(perNodeId,hVal,q931NodeId, {_q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu)
                                                             _q931(h323_message_body) _anyField LAST_TOKEN});

        /* CID*/
        memcpy(ce->cId,callObj.cid,16);

        /* CallID*/
        if (callObj.flags & catCallId)
            memcpy(ce->callId,callObj.callID,16);
        else
            /* In the case call has no CallID generate one */
            memcpy(ce->callId,getCID(&(app->seed)),16);


        /* Call may be accepted */

        {/*Process Call Rate*/
            INT32 rateMult=0;
            INT32 rate=0;
            /*Calculate the call rate from the bearerCapability IE if present */
            __pvtGetNodeIdByFieldIds(tmpNodeId,hVal,q931NodeId, {_q931(bearerCapability) _q931(octet4) LAST_TOKEN});
            pvtGet(hVal,pvtGetChild(hVal,tmpNodeId,__q931(rateMultiplier),NULL),NULL,NULL,&rateMult,NULL);
            pvtGet(hVal,pvtGetChild(hVal,tmpNodeId,__q931(informationTransferRate),NULL),NULL,NULL,&rate,NULL);
            switch(rate)
            {
                case 0:ce->newRate=0;break;
                case 16:ce->newRate=64000;break;
                case 17:ce->newRate=128000;break;
                case 19:ce->newRate=384000;break;
                case 21:ce->newRate=1536000;break;
                case 23:ce->newRate=1920000;break;
                case 24:ce->newRate=64000*rateMult;break;
                /*Set Rate to be 128000 as default*/
                default: ce->newRate=128000;break;
            }
            if (rateMult>1) m_callset(ce,multiRate,TRUE);
        }
        {/* Process Overlap Sending */
            int remoteCOS;
            /* Remote should both specify canOverlapSend==TRUE and omit sendingComplete, */
            /*  to be considered as able to Overlap Send                                 */
            pvtGetChildValue(hVal,perNodeId,__q931(canOverlapSend),(INT32*)&remoteCOS,NULL);
            if (remoteCOS==TRUE)
                m_callset(ce,remoteCanOverlapSend,!(pvtGetChild(hVal,q931NodeId,__q931(sendingComplete),NULL)>=0));
        }
        /* Get the Conference Goal */
        tmpNodeId=pvtGetChild(hVal,perNodeId,__q931(conferenceGoal),NULL) ;
        {
            int iConferenceGoal = pvtGetSyntaxIndex(hVal,pvtChild(hVal,tmpNodeId))-1;
            ce->conferenceGoal=(cmConferenceGoalType)iConferenceGoal;
            if (ce->conferenceGoal==cmCallIndependentSupplementaryService)
                cmCallSetParam((HCALL)ce,cmParamH245Stage,0,cmTransH245Stage_facility,NULL);
        }

        /* CRV for specific calls should be between 1-32767 */
        app->crv %= 0x7fff;
        app->crv++;
        if (ce->rascrv<0 && !app->gatekeeper)
            ce->rascrv=app->crv;

        /*Getting the connection addresses*/
        {
            HSTRANSHOST hsTransHost;
            cmTransportAddress ta;
            cmTransGetSessionParam(hsTransSession,cmTransParam_host,(void*)&hsTransHost);
            cmTransGetHostParam(hsTransHost,cmTransHostParam_localAddress,&ta);
            if (ce->callSignalAddress<0)
                ce->callSignalAddress=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
            cmTAToVt(app->hVal,ce->callSignalAddress,&ta);

            cmTransGetHostParam(hsTransHost,cmTransHostParam_remoteAddress,&ta);
            if (ce->remoteCallSignalAddress<0)
                ce->remoteCallSignalAddress=pvtAddRoot(app->hVal,app->hAddrSyn,0,NULL);
            cmTAToVt(app->hVal,ce->remoteCallSignalAddress,&ta);
        }
        /*Processing the Setup message*/
        {
            int nodeId=pvtAddRoot(hVal,NULL,0,NULL);
            pvtSetTree(hVal,nodeId,hVal,message);
            /* put the new SETUP message into the call property */
            callSetMessage((HCALL)ce, cmQ931setup, nodeId);
            q931SimulateSetup(cmiGetQ931((HCALL)ce));
            pvtDelete(hVal,nodeId);
        }

        /* Notify application about the new incoming call */
        if(!m_callget(ce,notified))
        {
            m_callset(ce,notified,TRUE);
            if(app->cmMyEvent.cmEvNewCall)
            {
                HAPPCALL haCall;
                enqueueDummyState(ce);
                cmiCBEnter((HAPP)app,(char*)"cmEvNewCall(hApp=0x%p,hsCall=0x%p,lphaCall)",app,ce);
                app->cmMyEvent.cmEvNewCall((HAPP)app,(HCALL)ce,&haCall);
                emaSetApplicationHandle((EMAElement)ce,(void*)haCall);
                cmiCBExit((HAPP)app,(char*)"cmEvNewCall(*lphaCall=0x%p)",emaGetApplicationHandle((EMAElement)ce));
                dequeueCallStates(app, ce, cmCallStateInit, (cmCallStateMode_e)-1);

                if (app->mibEvent.h341AddNewCall)
                    app->mibEvent.h341AddNewCall(app->mibHandle, (HCALL)ce);
            }
        }
        emaUnlock((EMAElement)ce);
        return cmTransOK;
    }
    return cmTransErr;
}


TRANSERR cmEvTransConnectionOnSessionClosed(IN HSTRANSSESSION hsTransSession,
                                            IN HATRANSSESSION haTransSession,
                                            IN TRANSCONNTYPE  type)
{
    callElem* call=(callElem*)haTransSession;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);

    if (hsTransSession);
    switch(type)
    {
        case cmTransQ931Conn:
        {
            if (emaLock((EMAElement)call))
            {
                /* see if configuration (keepCallOnQ931Close), the call state and the control state allow us
                to drop the call */
                if (call->state!=cmCallStateIdle &&
                    (pvtGetChild(cmGetValTree(hApp),((cmElem*)hApp)->q931Conf,__q931(keepCallOnQ931Close),NULL)<0 ||
                    !cmTransHasControlSession(call->hsTransSession)))
                {
                    BOOL hasControl = m_callget(call, control);

                    dropControl(call);
                    reportDisconnectedState(call,reasonQ931ConnectionClosed);
                    if(!hasControl)
                    {
                        q931CallDrop(cmiGetQ931((HCALL)call),-1);
                        dropRas(call);
                    }
                }
                /* Let the world know that this call no longer has a Q931 session (no matter if we dropped it or
                not - the other side did) */
                m_callset(call,callWithoutQ931,TRUE);
                emaUnlock((EMAElement)call);
            }
        }
        break;
        case cmTransH245Conn:
        {
            if (emaLock((EMAElement)call))
            {
                dropControl(call);
                cmTimerReset(hApp,&(call->timer));

                /* In case that this is a real call whose control has been disconnected, we
                   want to give a chance to the releaseComplete to arrive.
                   If it's just a dummy call for the control, no releaseComplete will ever
                   arrive */
                if (!emaWasDeleted((EMAElement)call) && (call->state != cmCallStateInit))
                {
                    if (m_callget(call,callWithoutQ931))
                        /* There is no Q931 - no use in waiting for RelComp */
                        call->timer=cmTimerSet(hApp,controlDiedTimerEventsHandler,call,1);
                    else
                        call->timer=cmTimerSet(hApp,controlDiedTimerEventsHandler,call,((cmElem*)hApp)->postControlDisconnectionDelay);
                    m_callset(call,controlDisconnected,TRUE);
                }
                emaUnlock((EMAElement)call);
            }
        }
        break;
    }
    return cmTransOK;
}




/************************************************************************
 * enterCallInitState
 * purpose: Set the state of the call to its init state.
 *          This state is the first the call enters
 * input  : call    - Call object
 * output : none
 * return : none
 ************************************************************************/
void enterCallInitState(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    INT32 t301,t303,t304;

    /* Get the timeout values for call setup related timeouts */
    pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(responseTimeOut),NULL),NULL,NULL,&t304,NULL);
    t303=t304*1000;
    pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(t304),NULL),NULL,NULL,&t304,NULL);
    t304*=1000;
    pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(connectTimeOut), NULL),NULL,NULL,&t301,NULL);
    t301*=1000;

    /* Create the Q931 "object" of the call */
    q931CallCreate((HQ931)cmiGetQ931((HCALL)call), t301, -1, t303, t304, t301, 100000);

    /* Set the CID and CallID for the call */
    memcpy(call->callId,getCID(&(app->seed)),16);
    if (m_callget(call,newCIDRequired))
    {
        memcpy(call->cId,getCID(&(app->seed)),16);
        m_callset(call,newCIDRequired,0);
    }

    /* Set the CRV for the call and update it in the stack */
    /* CRV for specific calls should be between 1-32767 */
    app->crv %= 0x7fff;
    app->crv++;
    call->crv = app->crv;

    /* For non-gatekeeper apps, we set the Q931-CRV and the RAS-CRV to the same value */
    if (!app->gatekeeper)
        call->rascrv=app->crv;
    m_callset(call,callInitiator,1);
    cmTransCreateSession(app->hTransport, (HATRANSSESSION)call,&(call->hsTransSession));
    cmCallSetParam((HCALL)call, cmParamEstablishH245, 0, !m_callget(call,notEstablishControl), NULL);
}


TRANSERR cmEvTransSessionNewConnection( IN HSTRANSSESSION   hsTransSession,
                                        IN HATRANSSESSION   haTransSession,
                                        IN TRANSCONNTYPE    type)
{
    callElem* call=(callElem*)haTransSession;
    if (hsTransSession);
    if (!emaLock((EMAElement)call))
        return cmTransErr;
    switch(type)
    {
        case cmTransQ931Conn:
        {
            cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

            int message=callGetMessage((HCALL)call,cmQ931setup);
            int tmpNodeId;

            __pvtBuildByFieldIds(tmpNodeId,app->hVal,message , {_q931(message) _q931(setup)
                _q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu) _q931(h323_message_body) _q931(setup)
                LAST_TOKEN},0,NULL);

            if (cmCallGetParam((HCALL)call,cmParamSrcCallSignalAddress,0,NULL,NULL)<0)
            {
                HSTRANSHOST host = NULL;
                int srcAddrNodeId;
                cmTransportAddress ta={0,0,0,cmTransportTypeIP};

                cmTransGetSessionParam(call->hsTransSession, cmTransParam_host, (void *)&host);
                if (host)
                    cmTransGetHostParam(host, cmTransHostParam_localAddress, (void *)&ta);

                srcAddrNodeId=pvtAdd(app->hVal,tmpNodeId,__q931(sourceCallSignalAddress),0,NULL,NULL);
                cmTAToVt(app->hVal, srcAddrNodeId, &ta);
            }

            {
                char eId[256];
                int eIdLen=cmiAutoRASGetEndpointID((HAPP)app,eId);
                if (eIdLen>0)
                    pvtAdd(app->hVal,tmpNodeId,__q931(endpointIdentifier),eIdLen,eId,NULL);
            }

            __pvtBuildByFieldIds(tmpNodeId,app->hVal,message, {_q931(message) _q931(setup)
                _q931(bearerCapability) _q931(octet4) LAST_TOKEN},0,NULL);
            if (m_callget(call,multiRate))
            {
                pvtAdd(app->hVal,tmpNodeId,__q931(informationTransferRate),(call->rate<=64000)?16:24,NULL,NULL);
                if (call->rate>64000)
                    pvtAdd(app->hVal,tmpNodeId,__q931(rateMultiplier),(call->rate-1)/64000+1,NULL,NULL);

            }
            else
            {
                int rateVal=0;
                if (call->rate==0) rateVal=0;
                else if (call->rate<=64000) rateVal=16;
                else if (call->rate<=128000) rateVal=17;
                else if (call->rate<=384000) rateVal=19;
                else if (call->rate<=1536000) rateVal=21;
                else if (call->rate<=1920000) rateVal=23;
                pvtAdd(app->hVal,tmpNodeId,__q931(informationTransferRate),rateVal,NULL,NULL);
            }

            notifyState((HCALL)call,cmCallStateDialtone,(cmCallStateMode_e)0);
            if (!emaWasDeleted((EMAElement)call))
            {
                q931CallDial(cmiGetQ931((HCALL)call),message);
                if (app->mibEvent.h341AddNewCall)
                    app->mibEvent.h341AddNewCall(app->mibHandle, (HCALL)call);
            }
        }
        break;

        case cmTransH245Conn:
        {
            HCONTROL ctrl = cmiGetControl((HCALL)call);
            initControl(ctrl,call->lcnOut);
            m_callset(call,control,TRUE);
            cmiReportControl((HCALL)call,cmControlStateTransportConnected,(cmControlStateMode)0);
            startControl(ctrl);
        }
        break;
    }
    emaUnlock((EMAElement)call);
    return cmTransOK;
}


/************************************************************************
 * cmCallNew
 * purpose: Creates a new call object that belongs to a particular Stack instance.
 *          This function does not launch any action on the network. It only causes
 *          the application and the Stack to exchange handles.
 * input  : hApp        - Stack handle for the application
 *          haCall      - Application's handle for the call
 * output : lphsCall    - Stack handle for the call to dial
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallNew(
        IN   HAPP        hApp,
        IN   HAPPCALL    haCall,
        OUT  LPHCALL     lphsCall)
{
    cmElem* app=(cmElem*)hApp;
    HPVT hVal=app->hVal;
    callElem call;
    int someId;
    BOOL boolParam;
    if (!hApp) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmCallNew(hApp=0x%x,haCall=0x%x,lphsCall)",hApp,haCall);

    /* Create a call struct and set the default values inside it */
    memset(&call,0xff,sizeof(callElem));
    call.appHandles[cmQ931TpktChannel]=NULL;
    call.appHandles[cmH245TpktChannel]=NULL;
    call.flags=0;
    call.hsTransSession=NULL;
    /*
    call.enableOverlapSending=0;
    call.control=FALSE;
    */
    m_callset(&call,notified,TRUE); /* No need to notify app about this call - app created it */
    call.newRate = 0;
    call.rate = 0;

    call.q_nextState = 0;
    call.q_numStates = 0;

    m_callset(&call, overrideCID, TRUE);
    m_callset(&call,newCIDRequired,1);
    call.hMsgContext=NULL;
    call.hCatCall=NULL;
    call.state=cmCallStateInit;
    m_callset(&call,callWithoutQ931,FALSE);
    call.timer=(HTI)RVERROR;
    m_callset(&call, enableOverlapSending, (pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(overlappedSending),NULL),NULL,NULL,NULL,NULL) >= 0));
    m_callset(&call, notEstablishControl, (pvtGetChild(hVal,app->q931Conf,__q931(notEstablishControl),NULL)>=0));
    m_callset(&call, multiRate, TRUE);

    {
        int nodeId;
        INT32 annexE=0;

        call.annexE = cmTransNoAnnexE;
        if ((nodeId=pvtGetChild(app->hVal,app->q931Conf,__q931(useAnnexE),NULL))>=0)
        {
            int iAnnexE = pvtGetSyntaxIndex(app->hVal,pvtChild(app->hVal,pvtGetChild(app->hVal,nodeId,__q931(protocolPreference),NULL)))-1;
            call.annexE=(cmAnnexEUsageMode)iAnnexE;

            if (((int)call.annexE) < 0)
            {
                call.annexE=cmTransNoAnnexE;
                pvtGetChildValue(app->hVal,nodeId,__q931(defaultProtocol),&annexE,NULL);
                if (annexE)
                    call.annexE=cmTransUseAnnexE;
            }
        }
    }


    someId=pvtGetChild(app->hVal,app->rasConf,__q931(preGrantedArqUse),NULL);
    if (pvtGetChild(app->hVal,someId,__q931(routed),NULL) >= 0)
        call.preGrantedArqUse=cmPreGrantedArqRouted;
    if (pvtGetChild(app->hVal,someId,__q931(direct),NULL) >= 0)
        call.preGrantedArqUse=cmPreGrantedArqDirect;

    {
        INT32 maxCalls;
        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(maxCalls),NULL),NULL,NULL,&(maxCalls),NULL);
        if (app->busy>=maxCalls)
        {
            cmiAPIExit((HAPP)app,(char*)"cmCallNew [%d]",-4);
            return -4;
        }
    }
    app->busy++;

    /* Allocate a call in EMA */
    *lphsCall=(HCALL)emaAdd(app->hCalls,(void*)haCall);
    if (!*lphsCall)
    {
        cmiAPIExit((HAPP)app,(char*)"cmCallNew [%d]",RESOURCES_PROBLEM);
        return RESOURCES_PROBLEM;
    }
    memcpy((void*)*lphsCall,&call,sizeof(call));

    /* Initialize Fast-Start Parameters */
    fastStartCallInit((callElem*)*lphsCall);

    /* Initialize the property database for the call if we should */
    callInitParameters(*lphsCall);

    /* Initialize the RAS call */
    cmiAutoRASCallCreate((HCALL)*lphsCall);

    /* Set the terminal type of the call */
    __pvtGetNodeIdByFieldIds(someId,app->hVal,app->rasConf, {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN});
    if (someId >= 0)
        cmCallSetParam((HCALL)*lphsCall, cmParamFullSourceInfo, 0, someId, NULL);
    else
    {
        __pvtBuildByFieldIds(someId, app->hVal, app->rasConf, {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN}, 0, NULL);
        pvtAdd(app->hVal, someId, __q931(mc), 0, NULL, NULL);
        pvtAdd(app->hVal, someId, __q931(undefinedNode), 0, NULL, NULL);
        cmCallSetParam((HCALL)*lphsCall, cmParamFullSourceInfo, 0, someId, NULL);
    }

    /* Set the number of supported channels for this call */
    /* todo: If it's in app, why do we need it inside HCALL? */
    cmiSetNumberOfChannelsForCall((HCALL)*lphsCall,app->maxChannels);

    /* Make sure call is in its init state */
    enterCallInitState((callElem*)*lphsCall);

    /* transport layer defaults - we set them after enterCallInitState() since only here we've
       got a session for them */
    boolParam = (pvtGetChild(hVal,app->q931Conf,__q931(h245Tunneling),NULL) >= 0);
    cmCallSetParam((HCALL)*lphsCall, cmParamH245Tunneling, 0, (INT32)boolParam, NULL);

    cmCallSetParam((HCALL)*lphsCall, cmParamH245Parallel, 0, FALSE, NULL);
    cmCallSetParam((HCALL)*lphsCall, cmParamIsMultiplexed, 0, FALSE, NULL);
    cmCallSetParam((HCALL)*lphsCall, cmParamShutdownEmptyConnection, 0, TRUE, NULL);

    boolParam = (pvtGetChild(hVal,app->q931Conf,__q931(earlyH245),NULL)>=0);
    cmCallSetParam((HCALL)*lphsCall, cmParamH245Stage, 0,
        (INT32)((boolParam)?cmTransH245Stage_early:cmTransH245Stage_connect), NULL);


    /* Make sure we notify the MIB about this call. */
    if (app->mibEvent.h341AddNewCall)
        app->mibEvent.h341AddNewCall(app->mibHandle, (HCALL)*lphsCall);

    cmiAPIExit((HAPP)app,(char*)"cmCallNew(*lphsCall=0x%lx)=%d",*lphsCall,0);
    return 0;
}



/************************************************************************
 * callAnswer
 * purpose: Connect an incoming call.
 *          Set the display parameter to the host's name if we've got
 *          display information from the originator of the call
 * input  : call    - Stack handle for the call to dial
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callAnswer(callElem*call)
{
    int status = RVERROR;

#if 0 /* nextone - do not put default display */
    {
        UINT32 size=1;
        char display[2];
        strcpy(display, "");

        cmCallGetParam((HCALL)call,cmParamConnectDisplay,0,(INT32*)&size,display);
        if (!display[0])
        {
            /* Set the display parameter for the Connect message to the local host's name */
            char* hostName = liGetHostName();
            cmCallSetParam((HCALL)call,cmParamConnectDisplay,0,(INT32)strlen(hostName),hostName);
        }
    }
#endif

    /* Answer with CONNECT */
    status = q931CallAnswer(cmiGetQ931((HCALL)call),-1);

    /* Make sure the state of the call is connected */
    if(status>=0)
        enterCallConnectedState(call);

    return status;
}

/************************************************************************
 * enterCallSetupState
 * purpose: Handle an incoming setup message
 *          See if we're replying automatically and notify the application
 * input  : call    - Stack handle for the call to dial
 * output : none
 * return : none
 ************************************************************************/
void enterCallSetupState(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

    /*We enter offering state only once*/
    if (call->state!=cmCallStateOffering)
    {
        /* trigger H.245 connection */
        cmTransTryControlAfterACF(call->hsTransSession);

        /* Notify the application about the incoming call's parameters */
        if (app->cmMyCallEvent.cmEvCallInfo)
        {
            char display[128];
            char userUser[128];
            INT32 displaySize=sizeof(display)-1;
            INT32 userUserSize=sizeof(userUser);
            int nesting;

            if (cmCallGetParam((HCALL)call,cmParamDisplay,0,&displaySize,display) < 0)
                displaySize = 0;
            if (displaySize < 128)
                display[displaySize] = 0;
            if (cmCallGetParam((HCALL)call,cmParamUserUser,0,&userUserSize,userUser) < 0)
                userUserSize = 0;
            if (userUserSize < 128)
                userUser[userUserSize] = 0;

            cmiCBEnter((HAPP)app,(char*)"cmEvCallInfo(haCall=0x%lx,hsCall=0x%lx,display=%s,userUser=%s,userUserSize=%d)",
                emaGetApplicationHandle((EMAElement)call),
                call, nprn(display), nprn(userUser), userUserSize);
            nesting=emaPrepareForCallback((EMAElement)call);
            app->cmMyCallEvent.cmEvCallInfo((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,display,userUser,(int)userUserSize);
            emaReturnFromCallback((EMAElement)call,nesting);
            cmiCBExit((HAPP)app,(char*)"cmEvCallInfo");
        }

        /* Notify the change of state (=offering) */
        if (!emaWasDeleted((EMAElement)call))
        {
            /* The state mode of this state is the conference goal */
            static const cmCallStateMode_e confGoalStateModes[] = {
                cmCallStateModeOfferingCreate,
                cmCallStateModeOfferingJoin,
                cmCallStateModeOfferingInvite,
                cmCallStateModeOfferingCapabilityNegotiation,
                cmCallStateModeOfferingCallIndependentSupplementaryService};
            if (((int)call->conferenceGoal >= 0) && ((int)call->conferenceGoal < (int)cmLastCG))
                notifyState((HCALL)call,cmCallStateOffering,confGoalStateModes[call->conferenceGoal]);
        }
    }
}

void RVCALLCONV timerEventsHandler(void*context)
{
    callElem* call=(callElem*)context;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);

    cmCallPreCallBack(hApp);

    if (emaLock((EMAElement)call))
    {
        cmTimerReset(hApp,&(call->timer));
        if (cmCallGetParam((HCALL)call,cmParamReleaseCompleteCause,0,NULL,NULL)<0)
            cmCallSetParam((HCALL)call,cmParamReleaseCompleteCause,0,16,NULL);
        q931CallDrop(cmiGetQ931((HCALL)call),-1);
        dropControl(call);
        reportDisconnectedState(call,reasonProtocolError);
        dropRas(call);
        emaUnlock((EMAElement)call);
    }
}


/************************************************************************
 * callNewRate
 * purpose: Notify the application about the rate of the call
 * input  : call      - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void callNewRate(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    if (app->cmMyCallEvent.cmEvCallNewRate)
    {
        int nesting;
        cmiCBEnter((HAPP)app,(char*)"cmEvCallNewRate(haCall=0x%lx,hsCall=0x%lx,rate=%ld)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,call->rate);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallNewRate((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,call->rate);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit((HAPP)app,(char*)"cmEvCallNewRate");
    }
}


/************************************************************************
 * enterCallConnectedState
 * purpose: Dials a call. This function together with cmCallSetParam()
 *          can be used instead of cmCallMake(). cmCallSetParam() should
 *          be called before cmCallDial() and the required parameters of
 *          the call should be set
 * input  : call      - Stack handle for the call
 * output : none
 * return : none
 ************************************************************************/
void enterCallConnectedState(IN callElem* call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    emaLock((EMAElement)call);

    /* Reset the call's timer */
    cmTimerReset((HAPP)app,&(call->timer));

    /* Notify application about the state */
    notifyState((HCALL)call,cmCallStateConnected,cmCallStateModeConnectedCallSetup);
    /* Application didn't drop the call in the callback - we continue notifying */
    if (!emaWasDeleted((EMAElement)call))
        callNewRate(call);
    emaUnlock((EMAElement)call);
}


/************************************************************************
 * cmCallDial
 * purpose: Dials a call. This function together with cmCallSetParam()
 *          can be used instead of cmCallMake(). cmCallSetParam() should
 *          be called before cmCallDial() and the required parameters of
 *          the call should be set
 * input  : hsCall      - Stack handle for the call to dial
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallDial(
        IN  HCALL       hsCall)
{

    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    cmElem* app=(cmElem*)hApp;
    callElem*call=(callElem*)hsCall;
    INT32 timeout;
    int ret=-1;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallDial(hsCall=0x%lx)",hsCall);

    if (emaLock((EMAElement)call))
    {
        ret = 0;
        {
            if (!app->diffSrcAddressInSetupAndARQ)
            {
                cmTransportAddress ta={0,0,0,cmTransportTypeIP};

                if (cmTransSetAddress(call->hsTransSession, &ta, NULL, NULL, NULL, cmTransQ931Conn, FALSE) != cmTransOK)
                    ret = -1;
                if (cmCallGetParam((HCALL)call,cmParamSrcCallSignalAddress,0,NULL,NULL)<0)
                    cmCallSetParam((HCALL)call,cmParamSrcCallSignalAddress,0,0,(char*)&ta);
            }

            cmCallSetParam((HCALL)call,cmParamSetupCanOverlapSending,0,(INT32)m_callget(call,enableOverlapSending),NULL);
            if (m_callget(call,enableOverlapSending))
                cmCallSetParam((HCALL)call,cmParamSetupSendingComplete,0,FALSE,NULL);
            if (cmCallGetParam((HCALL)call,cmParamFullSourceAddress,0,NULL,NULL)<0)
                cmCallSetParam((HCALL)call,cmParamFullSourceAddress,0,
                        pvtGetChild2(app->hVal,app->rasConf,__q931(registrationInfo),__q931(terminalAlias)),NULL);
        }

        /* Set a timer for a response for the Setup */
        pvtGet(app->hVal,pvtGetChild(app->hVal,app->q931Conf,__q931(responseTimeOut),NULL),NULL,NULL,&timeout,NULL);
        timeout*=1000;
        cmTimerReset(hApp,&(call->timer));
        call->timer=cmTimerSet(hApp,timerEventsHandler,call,timeout);

        /* Dial the call */
        if (ret >= 0)
            ret = callDial(call);
        emaUnlock((EMAElement)call);
    }
    cmiAPIExit(hApp,(char*)"cmCallDial=%d", ret);
    return ret;
}

/************************************************************************
 * cmCallAnswer
 * purpose: Answers an incoming call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallAnswer(
        IN    HCALL       hsCall)
{
    int status = RVERROR;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;

    if (!hsCall || !app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallAnswer(hsCall=0x%lx)",hsCall);

    if (emaLock((EMAElement)call))
    {
        BOOL outgoingCall = FALSE;
        cmCallGetOrigin(hsCall, &outgoingCall);

        if (!outgoingCall)
        {
            cmCallState_e oldState;

            if( (oldState = call->state) == cmCallStateWaitAddressAck )
                cmCallAddressComplete(hsCall);

            if(((oldState == cmCallStateInit) && (call->state == cmCallStateConnected)) ||
                /* This happens when we're working with H450.9.
                   In this case, the call to cmCallAnswer() from the SSE will force the
                   StateOffering to be generated by the call to cmCallAddressComplete above.
                   If the application on top of it decides to call cmCallAnswer() from there,
                   then the original call to cmCallAnswer() will fail.
                   This if statement, makes sure that the original call to cmCallAnswer() succeeds. */
                ((oldState == cmCallStateWaitAddressAck) && (call->state == cmCallStateConnected)))
                /* This happens when we are on auto answer mode, and when we choose to
                   complete the address, we also answer the call automatically */
            {
                status = 0;
            }
            else
            {
                /* This is the regular path for such actions. We'll take it most of the times */

                /* Open any Accepted fast start channels */
                cmFastStartChannelsReady(hsCall);

                /* Connect the call */
                status = callAnswer(call);

                /* Delete fast start information - we won't need it any more */
                deleteFastStart(call);
            }
        }
        emaUnlock((EMAElement)call);
    }

    cmiAPIExit((HAPP)app,(char*)"cmCallAnswer=%d", status);
    return status;
}

/************************************************************************
 * cmCallAccept
 * purpose: Accepts an incoming call and sends Q.931 ALERTING message to
 *          calling party
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallAccept(
        IN    HCALL       hsCall)
{
    int status = RVERROR;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;

    if (!hsCall) return RVERROR;
    if (!app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallAccept(hsCall=0x%lx)",hsCall);

    if (emaLock((EMAElement)call))
    {
        /* Send Alerting message */
        status = q931CallAccept((HQ931)cmiGetQ931(hsCall),-1);

        emaUnlock((EMAElement)call);
    }

    cmiAPIExit((HAPP)app,(char*)"cmCallAccept=%d", status);
    return status;
}

/************************************************************************
 * cmCallSendCallProceeding
 * purpose: Sends Q.931 CALL PROCEEDING message
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendCallProceeding(IN HCALL hsCall)
{
    int status = RVERROR;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallSendCallProceeding: hsCall=0x%x ",hsCall);

    if (emaLock((EMAElement)call))
    {
        /* Send CallProceeding message */
        status = q931CallCallProceeding((HQ931)cmiGetQ931(hsCall),-1);

        emaUnlock((EMAElement)call);
    }

    cmiAPIExit(hApp,(char*)"cmCallSendCallProceeding=%d", status);
    return status;

}

/************************************************************************
 * callStopOK
 * purpose: We've finished dropping the call.
 *          Notify the application we've reached the Idle state on this call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callStopOK(IN callElem* call)
{
    enterCallIdleState(call);
    return 0;
}

int callStopError(callElem*call)
{
    enterCallIdleState(call);/*??????????????*/
    return 0;
}

/* The timeout was reached after H245 connection drop*/
void RVCALLCONV controlDiedTimerEventsHandler(void*context)
{
    callElem* call=(callElem*)context;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmCallPreCallBack(hApp);
    if (emaLock((EMAElement)call))
    {
        cmTimerReset(hApp,&(call->timer));
        /* in multithreaded applications, we might be getting here right
            after we finished answering FS channels, so to be on the
            safe side, we'll drop control again. We should consider
            removing line 2198, to avoid doing so twice, but even if we
            do, no harm done. */
        dropControl(call);
        reportDisconnectedState(call,reasonH245ConnectionClosed);
        q931CallDrop(cmiGetQ931((HCALL)call),-1);
        dropRas(call);
        emaUnlock((EMAElement)call);
    }
}

/************************************************************************
 * enterCallIdleState
 * purpose: Notify the application we've reached the Idle state on a call
 * input  : call      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
void enterCallIdleState(IN callElem* call)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;

    emaLock((EMAElement)call);

    /* Reset the call's timer */
    cmTimerReset(hApp,&(call->timer));

    /* On transfer, we initiate a new call dialing with the same handle */
    if (call->state==cmCallStateTransfering)
    {
        INT32 establish;
        INT32 tunneling;
        INT32 parallel;
        INT32 stage;
        BOOL resetCall = ((call->stateMode == cmCallStateModeTransferingForwarded) && !m_callget(call,callInitiator));
        int keepH450 = -1;

        /* Get parameters about this call before closing its session */
        cmCallGetParam((HCALL)call, cmParamEstablishH245,0,&establish,NULL);
        cmCallGetParam((HCALL)call, cmParamH245Parallel,0,&parallel,NULL);
        cmCallGetParam((HCALL)call, cmParamH245Tunneling,0,&tunneling,NULL);
        cmCallGetParam((HCALL)call, cmParamH245Stage,0,&stage,NULL);

        if (call->stateMode == cmCallStateModeTransferingRouteToGatekeeper)
        {
            int h4501SupplementaryService;

            __pvtGetByFieldIds(h4501SupplementaryService, app->hVal, cmGetProperty((HPROTOCOL)call),
                { _q931(setup) _q931(message) _q931(setup) _q931(userUser) _q931(h323_UserInformation)
                  _q931(h323_uu_pdu) _q931(h4501SupplementaryService) LAST_TOKEN }, NULL, NULL, NULL);
            if(h4501SupplementaryService >= 0)
            {
                keepH450 = pvtAddRoot(app->hVal, NULL, 0, NULL);
                pvtMoveTree(app->hVal, keepH450, h4501SupplementaryService);
            }
        }

        /* Close the transport session of the call */
        if (call->hsTransSession)
        {
            cmTransCloseSession(call->hsTransSession);
            call->hsTransSession=NULL;
        }

        if (resetCall)
        {
            /* we have to re-initialize the call */
            int sourceNode, addressNodeDest = -1, infoNodeDest = -1, addressNodeSrce = -1;
            INT32 sourceNodeParam;
            cmTransportAddress ta;
            cmAlias cpn;
            char cpnStr[128];

            /* remove the answering fast start, if it exists, and re-initialize it */
            deleteFastStart(call);
            fastStartCallInit(call);

            /* get the destination aliases node */
            if (cmCallGetParam((HCALL)call, cmParamFullDestinationAddress, 0, &sourceNodeParam, (char*)NULL) >= 0)
            {
                sourceNode = (int)sourceNodeParam;
                addressNodeDest = pvtAddRoot(app->hVal, NULL, 0, NULL);
                pvtSetTree(app->hVal, addressNodeDest, app->hVal, sourceNode);
            }

            /* get the destination info node */
            if (cmCallGetParam((HCALL)call, cmParamFullDestinationInfo, 0, &sourceNodeParam, (char*)NULL) >= 0)
            {
                sourceNode = (int)sourceNodeParam;
                infoNodeDest = pvtAddRoot(app->hVal, NULL, 0, NULL);
                pvtSetTree(app->hVal, infoNodeDest, app->hVal, sourceNode);
            }

            /* get the dest IP address */
            if (cmCallGetParam((HCALL)call, cmParamDestinationIpAddress, 0, NULL, (char*)&ta) < 0)
            {
                ta.port = 0;
            }

            /* get the calling party number */
            cpn.string = cpnStr;
            if (cmCallGetParam((HCALL)call, cmParamCalledPartyNumber, 0, NULL, (char*)&cpn) < 0)
            {
                cpn.length = 0;
            }

            /* fix for AudioCodes - check if we have a source address */
            if (cmCallGetNumOfParams((HCALL)call, cmParamSourceAddress) > 0)
            {
                /* someone set the source address in cmCallStateTransfering - keep it */
                cmCallGetParam((HCALL)call, cmParamFullSourceAddress, 0, &sourceNodeParam, (char*)NULL);
                sourceNode = (int)sourceNodeParam;
                addressNodeSrce = pvtAddRoot(app->hVal, NULL, 0, NULL);
                pvtSetTree(app->hVal, addressNodeSrce, app->hVal, sourceNode);
            }

            /* Initialize the property database for the call if we should */
            callInitParameters((HCALL)call);

            /* Set the terminal type of the endpoint */
            __pvtGetNodeIdByFieldIds(sourceNode, app->hVal, app->rasConf,
                {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN});
            if (sourceNode >= 0)
                cmCallSetParam((HCALL)call, cmParamFullSourceInfo, 0, (INT32)sourceNode, NULL);
            else
            {
                __pvtBuildByFieldIds(sourceNode, app->hVal, app->rasConf, {_q931(registrationInfo) _q931(terminalType) LAST_TOKEN}, 0, NULL);
                pvtAdd(app->hVal, sourceNode, __q931(mc), 0, NULL, NULL);
                pvtAdd(app->hVal, sourceNode, __q931(undefinedNode), 0, NULL, NULL);
                cmCallSetParam((HCALL)call, cmParamFullSourceInfo, 0, (INT32)sourceNode, NULL);
            }

            /* set the destination aliases */
            if (addressNodeDest >= 0)
            {
                cmCallSetParam((HCALL)call, cmParamFullDestinationAddress, 0, addressNodeDest, (char *)NULL);
                pvtDelete(app->hVal, addressNodeDest);
            }

            /* set the destination info */
            if (infoNodeDest >= 0)
            {
                cmCallSetParam((HCALL)call, cmParamFullDestinationInfo, 0, infoNodeDest, (char *)NULL);
                pvtDelete(app->hVal, infoNodeDest);
            }

            /* set the dest IP address */
            if (ta.port != 0)
            {
                cmCallSetParam((HCALL)call, cmParamDestinationIpAddress, 0, sizeof(ta), (char *)&ta);
            }

            /* set the calling party number */
            if (cpn.length != 0)
            {
                cmCallSetParam((HCALL)call, cmParamCalledPartyNumber, 0, sizeof(cpn), (char *)&cpn);
            }

            /* fix for AudioCodes - set the source address */
            if (addressNodeSrce >= 0)
            {
                /* someone set the source address in cmCallStateTransfering - set it */
                cmCallSetParam((HCALL)call, cmParamFullSourceAddress, 0, addressNodeSrce, (char *)NULL);
                pvtDelete(app->hVal, addressNodeSrce);
            }
        }
        else
        {
            /* just reset the fast connect state, if the call used fast connect */
            if(call->fastStartState != fssNo)
                call->fastStartState = fssRequest;
        }

        /* Delete CAT association of the call */
        if (call->hCatCall)
        {
            RVHCATCALL cat = call->hCatCall;
            call->hCatCall = NULL;
            catDelete(app->hCat, cat);
        }

        /* Close Q931 call object */
        q931CallClose((HQ931)cmiGetQ931((HCALL)call));

        /* Start the new call to finish off with the transfer procedure */
        enterCallInitState(call);
        cmCallSetParam((HCALL)call, cmParamEstablishH245,0,establish,NULL);
        cmCallSetParam((HCALL)call, cmParamH245Parallel,0,parallel,NULL);
        cmCallSetParam((HCALL)call, cmParamH245Tunneling,0,tunneling,NULL);
        cmCallSetParam((HCALL)call, cmParamH245Stage,0,stage,NULL);
        m_callset(call,controlDisconnected,FALSE);

        if (keepH450 >= 0)
        {
            cmTransSetH450Element(call->hsTransSession, keepH450);
        }
        cmCallDial((HCALL)call);
    }
    else
    {
        /* We should really notify the application that this call is IDLE */
        notifyState((HCALL)call,cmCallStateIdle,(cmCallStateMode_e)0);
    }
    emaUnlock((EMAElement)call);
}

/************************************************************************
 * rasCallDrop
 * purpose: Call drop was initiated by an incoming DRQ message from the
 *          gatekeeper
 * input  : call      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int rasCallDrop(IN callElem* call)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);

    cmTimerReset(hApp,&(call->timer));

    /* Make sure to close H245 connection */
    dropControl(call);

    /* Drop Q931 connection */
    q931CallDrop(cmiGetQ931((HCALL)call),-1);

    /* Tell APP about disconnection */
    reportDisconnectedState(call,reasonDRQ);

    /* Go IDLE */
    enterCallIdleState(call);
    return 0;
}


/************************************************************************
 * cmCallDrop
 * purpose: Drops a call
 * input  : hsCall      - Stack handle for the call
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallDrop(IN  HCALL       hsCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    HQ931 q931Call=cmiGetQ931((HCALL)hsCall);
    int ret = RVERROR;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallDrop(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)call))
    {

        /* Reset the timer used by the call */
        cmTimerReset(hApp,&(call->timer));

        /* Close H245 connection */
        dropControl(call);

        /* Drop Q931 connection */
        /* q931CallDrop will do nothing if there was no Q.931 activity before,
           for example if it is called right after ARQ-ACF procedure */
        ret = q931CallDrop(q931Call, -1);

        /* Drop Transactions - put it in Idle state */
        cmTransDrop(call->hsTransSession);

        /* Notify application about the disconnection */
        reportDisconnectedState(call,reasonApplicationDrop);

        /* Get rid of RAS - this will also continue to drop the call */
        dropRas(call);
        emaUnlock((EMAElement)call);
    }

    cmiAPIExit(hApp,(char*)"cmCallDrop=%d",ret);
    return ret;
}


RVAPI
int RVCALLCONV cmCallFacility(
                            IN HCALL        hsCall,
                            IN int      message)
{
    int ret = 0;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall ||  !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallFacility(hsCall=0x%lx,handle=%d)",hsCall,message);
    if (emaLock((EMAElement)hsCall))
    {
        ret = q931CallFacility(cmiGetQ931(hsCall),message);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallFacility=%d", ret);
    return ret;
}


/************************************************************************************
 *
 * cmCallProgress
 *
 * Purpose: To send PROGRESS message
 *
 * Input:   hsCall                  - The stack handle to the call
 *          message                 - The pvt nodeId of the PROGRESS message to send
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/
RVAPI
int RVCALLCONV cmCallProgress(
                            IN HCALL        hsCall,
                            IN int      message)
{
    int status = RVERROR;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall ||  !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallProgress(hsCall=0x%lx,handle=%d)",hsCall,message);
    if (emaLock((EMAElement)hsCall))
    {
        status = q931CallProgress(cmiGetQ931(hsCall),message);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallProgress=%d",status);
    return status;
}

/************************************************************************************
 *
 * cmCallNotify
 *
 * Purpose: To send NOTIFY message
 *
 * Input:   hsCall                  - The stack handle to the call
 *          message                 - The pvt nodeId of the NOTIFY message to send
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/

RVAPI
int RVCALLCONV cmCallNotify(
                            IN HCALL        hsCall,
                            IN int      message)
{
    int result = RVERROR;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall ||  !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallNotify(hsCall=0x%lx,handle=%d)",hsCall,message);
    if (emaLock((EMAElement)hsCall))
    {
        result = q931CallNotify(cmiGetQ931(hsCall),message);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallNotify=%d",result);
    return result;
}


RVAPI
int RVCALLCONV cmCallDropParam(
                             IN     HCALL       hsCall,
                             IN      cmReasonType    cmReason)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    char *prnt= (char*)"<0";
    int ret = RVERROR;

    if (!hsCall) return RVERROR;
    if (!app) return RVERROR;
#ifndef NOLOGSUPPORT
    if (((int)cmReason) >= 0)
    {
        INT32 nameId;
        nameId = getParamFieldName(cmParamReleaseCompleteReason)[cmReason].nameId;
        prnt = pstGetFieldNamePtr(app->synProtRAS,nameId);
    }
#endif
    cmiAPIEnter((HAPP)app,(char*)"cmCallDropParam(hsCall=0x%lx,cmReason=%s)",hsCall,nprn(prnt));
    if (emaLock((EMAElement)hsCall))
    {
        if (((int)cmReason) >= 0)
            cmCallSetParam((HCALL)hsCall,cmParamReleaseCompleteReason,0,cmReason,NULL);
        ret = cmCallDrop(hsCall);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit((HAPP)app,(char*)"cmCallDropParam=%d",ret);
    return ret;
}

RVAPI
int RVCALLCONV cmCallClose(
                         IN     HCALL       hsCall)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    int ret = 0;

    if (!hsCall) return RVERROR;
    if (!app) return RVERROR;
    cmiAPIEnter((HAPP)app,(char*)"cmCallClose(hsCall=0x%lx)",hsCall);

    if (emaLock((EMAElement)call))
    {
        if (app->mibEvent.h341DeleteControl)
            app->mibEvent.h341DeleteControl(app->mibHandle,hsCall);

        if (app->mibEvent.h341DeleteCall)
            app->mibEvent.h341DeleteCall(app->mibHandle,hsCall);

        if (call->hCatCall)
        {
            RVHCATCALL cat = call->hCatCall;
            call->hCatCall = NULL;
            catDelete(app->hCat, cat);
        }
        if (call->hsTransSession)
        {
            HSTRANSSESSION session = call->hsTransSession;
            call->hsTransSession = NULL;
            cmTransCloseSession(session);
        }
        callEndParameters((HCALL)call);
        if(call->routeCallSignalAddress >= 0)
        {
            pvtDelete(app->hVal, call->routeCallSignalAddress);
            call->routeCallSignalAddress = RVERROR;
        }
        deleteFastStart(call);
        if (!app->manualRAS)
            cmiAutoRASCallClose((HCALL)call);

        if ((call->hMsgContext!=NULL) && (app->cmEvCallReleaseMessageContext))
        {
            int nesting=emaPrepareForCallback((EMAElement)call);
            app->cmEvCallReleaseMessageContext(call->hMsgContext);
            emaReturnFromCallback((EMAElement)call,nesting);
        }
        ret = q931CallClose((HQ931)cmiGetQ931(hsCall));
        app->busy--;

        cmTimerReset((HAPP)app,&(call->timer));
        /* Delete, Unlock and Release */
        emaDelete((EMAElement)call);
        emaUnlock((EMAElement)call);
    }
    else
    {
        logPrint(app->logERR, RV_ERROR,
                 (app->logERR, RV_ERROR, "cmCallClose: Call handle 0x%x already closed", hsCall));
        ret = RVERROR;
    }

    cmiAPIExit((HAPP)app,(char*)"cmCallClose=%d",ret);
    return ret;
}


RVAPI
int RVCALLCONV cmCallGetOrigin(
                             IN     HCALL       hsCall,
                             OUT    BOOL*       origin)
{
    INT32 org=0;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    if (!hsCall || !hApp) return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmCallGetOrigin(hsCall=0x%lx,origin)",hsCall);
    org = m_callget(call,callInitiator);
    if (origin) *origin=(int)org;
    cmiAPIExit(hApp,(char*)"cmCallGetOrigin(origin=%d)=%ld",nprnd(origin),org);
    return (int)org;
}

RVAPI
int RVCALLCONV cmCallStatusEnquiry(
                                 IN     HCALL   hsCall,
                                 IN     UINT8 * display
                                 )
{
    int id,message;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    if (!hsCall) return RVERROR;
    if (!app) return RVERROR;

    cmiAPIEnter((HAPP)app,(char*)"cmCallStatusEnquiry(hsCall=0x%lx,display %s)",hsCall,nprn(((char*)display)));
    if (emaLock((EMAElement)hsCall))
    {
        message=callGetMessage(hsCall,cmQ931statusEnquiry);

        if (display != NULL)
        {
            __pvtBuildByFieldIds(id, app->hVal, message,
                                    {_q931(message)
                                     _q931(statusEnquiry)
                                     _q931(display)
                                     LAST_TOKEN},(INT32)strlen((char*)display),(char*)display);
            if (id<0)
            {
                emaUnlock((EMAElement)hsCall);
                cmiAPIExit((HAPP)app,(char*)"cmCallStatusEnquiry [%d]",id);
                return id;
            }
        }

        q931CallStatusEnquiry(cmiGetQ931(hsCall),message);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit((HAPP)app,(char*)"cmCallStatusEnquiry: [1]");
    return TRUE;

}


RVAPI
int RVCALLCONV cmCallIncompleteAddress(IN HCALL hsCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    if (!hsCall || !hApp) return RVERROR;
    cmiAPIEnter(hApp,"cmCallIncompleteAddress: hsCall=0x%x ",hsCall);

    if (emaLock((EMAElement)call))
    {
        if (m_callget(call,remoteCanOverlapSend) && m_callget(call,enableOverlapSending))
            q931CallMoreInfo(cmiGetQ931(hsCall),-1);
        emaUnlock((EMAElement)call);
    }

    cmiAPIExit(hApp,"cmCallIncompleteAddress: [1]");
    return TRUE;

}

RVAPI
int RVCALLCONV cmCallUserInformationSend(IN HCALL hsCall, IN int nodeId)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int res = RVERROR;

    if (!hsCall || !hApp)
        return RVERROR;
    cmiAPIEnter(hApp, "cmCallUserInformationSend: hsCall=0x%p, nodeId=%d", hsCall, nodeId);

    if (emaLock((EMAElement)hsCall))
    {
        res = sendCallMessage(hsCall, nodeId);
        pvtDelete(cmGetValTree(hApp), nodeId);

        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallUserInformationSend: [%d]", res);
    return res;

}


RVAPI
int RVCALLCONV cmCallAddressComplete(IN HCALL hsCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallAddressComplete: hsCall=0x%p",hsCall);

    if (emaLock((EMAElement)call))
    {
        cmTimerReset(hApp,&(call->timer));
        enterCallSetupState(call);
        cmSetupEnd(call);
        emaUnlock((EMAElement)call);
    }

    cmiAPIExit(hApp,(char*)"cmCallAddressComplete: [0]");
    return 0;
}

RVAPI
int RVCALLCONV cmCallOverlapSendingExt(IN HCALL hsCall,char * address,BOOL sendingComplete)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal;
    int status = RVERROR;

    if (!hsCall || !hApp) return RVERROR;
    hVal=cmGetValTree(hApp);

    cmiAPIEnter(hApp,(char*)"cmCallOverlapSendingExt: hsCall=0x%p address %s sendingcomplete %d",hsCall,nprn(address), sendingComplete);

    if (emaLock((EMAElement)hsCall))
    {
        /* If this function has been called before Q931,then do overlap sending on RAS*/
        if (q931GetCallState(cmiGetQ931(hsCall))==cs_Null)
        {
            char buffer[256];
            cmAlias alias;
            alias.string=buffer;
            buffer[0]=0;

            if (cmCallGetParam(hsCall,cmParamCalledPartyNumber,0,0,(char *)&alias)>=0)
                strcat(alias.string,address);
            else
                strcpy(alias.string,address);

            alias.length=(UINT16)strlen(alias.string);
            cmCallSetParam(hsCall,cmParamCalledPartyNumber,0,sizeof(cmAlias),(char*)&alias);
            status = cmiAutoRASCallDial(hsCall);
            if ((status >= 0) && (sendingComplete))
                cmCallSetParam(hsCall,cmParamSetupSendingComplete,0,TRUE,NULL);
        }
        /* If this function has been called during Q931 and in the correct state, then do overlap sending on Q931*/
        if (q931GetCallState(cmiGetQ931(hsCall))==cs_Overlap_sending)
        {
            int message=callGetMessage(hsCall,cmQ931information);
            int ieNodeId,tmpNodeId;
            int infoNodeId=pvtGetChild2(hVal,message, __q931(message), __q931(information));
            if (address!=NULL)
            {
                ieNodeId=pvtAddBranch(hVal,infoNodeId,__q931(calledPartyNumber));
                pvtAdd(hVal,ieNodeId,__q931(numberDigits),(INT32)strlen(address),address,NULL);
                tmpNodeId=pvtAddBranch(hVal,ieNodeId,__q931(octet3));
                pvtAdd(hVal,tmpNodeId,__q931(typeOfNumber),2,NULL,NULL);
                pvtAdd(hVal,tmpNodeId,__q931(numberingPlanIdentification),1,NULL,NULL);
            }
            if (sendingComplete)
                pvtAddBranch(hVal,infoNodeId,__q931(sendingComplete));

            status = q931CallInfo(cmiGetQ931(hsCall),message);
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp,(char*)"cmCallOverlapSendingExt: [%d]", status);
    return status;

}




/*Sends/Recv H.450 Messages*/
TRANSERR cmEvTransNewH450Message(IN HSTRANSSESSION  hsTransSession,
                                 IN HATRANSSESSION  haTransSession,
                                 IN int             msg,
                                 IN int             msgSize,
                                 IN int             msgType)
{
    callElem*call=(callElem*)haTransSession;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;

    if (hsTransSession);

    if (app->cmMyCallEvent.cmEvCallH450SupplServ)
    {
        cmiCBEnter(hApp,(char*)"cmEvCallH450SupplServ (haCall=0x%lx,hsCall=0x%x)",emaGetApplicationHandle((EMAElement)call),call);
        app->cmMyCallEvent.cmEvCallH450SupplServ((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,(cmCallQ931MsgType)msgType,msg,msgSize);
        cmiCBExit(hApp,(char*)"cmEvCallH450SupplServ");
    }
    return cmTransOK;
}


/************************************************************************
 * cmCallSendSuppServiceManually
 * purpose: Sends an H.450 APDU on the next Q.931 message that the
 *          application initiates. It will not initiate a FACILITY message
 *          on its own as cmCallSendSupplamentaryService() might send.
 * input  : hsCall      - Stack handle for the call
 *          buffer      - Buffer in which the APDU has been placed
 *          bufferSize  - Size of the buffer
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendSuppServiceManually(
    IN  HCALL       hsCall,
    IN  void*       buffer,
    IN  int         bufferSize)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    HPVT hVal;
    TRANSERR err=cmTransOK;

    if (!hsCall || !hApp) return RVERROR;
    hVal=cmGetValTree(hApp);
    cmiAPIEnter(hApp,(char*)"cmCallSendSuppServiceManually(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        err=cmTransSendH450Message(call->hsTransSession, (BYTE *)buffer, bufferSize, FALSE);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallSendSuppServiceManually=[%d]", err);
    return err;
}


/************************************************************************
 * cmCallSendSupplementaryService
 * purpose: Sends an H.450 APDU. This function takes a buffer of a
 *          Supplementary Service and puts an H.450 message in the buffer
 * input  : hsCall      - Stack handle for the call
 *          buffer      - Buffer in which the APDU has been placed
 *          bufferSize  - Size of the buffer
 *          force       - If TRUE do not wait for the next Q.931 message
 *                        before sending this APDU. If FALSE, it will wait
 *                        for the application to send a Q.931 message on
 *                        its own if the call isn't in the connected state
 *                        yet.
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallSendSupplementaryService(
    IN  HCALL       hsCall,
    IN  void*       buffer,
    IN  int         bufferSize,
    IN  BOOL        force)
{
    callElem*call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    HPVT hVal;
    TRANSERR err=cmTransOK;

    if (!hsCall || !hApp) return RVERROR;
    hVal=cmGetValTree(hApp);
    cmiAPIEnter(hApp,(char*)"cmCallSendSupplementaryService(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        err=cmTransSendH450Message(call->hsTransSession, (BYTE *)buffer, bufferSize,
                                   force || (q931GetCallState(cmiGetQ931(hsCall))==cs_Active));
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallSendSupplementaryService=[%d]", err);
    return err;
}


/*
cmGetH225RemoteVersion - gets H225 remote side version.
IN parameters:
hsCall: Call handle
OUT parameters
version: user allocated string,pass version number retrieved from Q931 messages.
Return: TRUE if succeeded,
RVERROR  if error is occured.
*/
RVAPI
int RVCALLCONV cmGetH225RemoteVersion(IN HCALL    hsCall,OUT char * version)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    int ver=1;
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmGetH225RemoteVersion: hsCall=0x%x",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        if (version)
        {
            ver=m_callget(call,remoteVersion);
            version[0]=(char)(ver+'0');
            version[1]=0;
        }
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmGetH225RemoteVersion: version %d",ver);
    return ver;
}


/************************************************************************
 * cmCallGetHandle
 * purpose: Returns the application handle for a call from the call handle.
 * input  : hsCall      - Stack handle for the call
 * output : haCall      - Application handle for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI
int RVCALLCONV cmCallGetHandle(IN HCALL hsCall, OUT HAPPCALL* haCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, (char*)"cmCallGetHandle: hsCall=0x%x", hsCall);
    if (haCall)
        *haCall=(HAPPCALL)emaGetApplicationHandle((EMAElement)hsCall);
    cmiAPIExit(hApp, (char*)"cmCallGetHandle: 1 (haCall=0x%x)",*haCall);
    return TRUE;

}

RVAPI
int RVCALLCONV cmSetCallHandle(
                             IN HCALL                hsCall,
                             IN HAPPCALL             haCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallSetHandle(hsCall=0x%lx,haCall=0x%x)",hsCall,haCall);
    if (haCall)
        emaSetApplicationHandle((EMAElement)hsCall,(void*)haCall);
    cmiAPIExit(hApp,(char*)"cmCallSetHandle=%d",0);
    return 0;
}

/************************************************************************************
 *
 * cmCallMultiplex
 *
 * Purpose: To specify call's Q.931 multiplex behavior.
 *          This function call marks the call as supporting the Q.931 multiplexing
 *
 * Input:   hsCall                  - The stack handle to the call
 *          hsSameConnectionCall    - The stack handle to the call which Q.931
 *                                    connection is the prefered connection for the hsCall
 *                                    If this parameter is not NULL and hsSameConnectionCall
 *                                    has Q.931 connection, which remote address equal to the
 *                                    hsCall's desireable Q.931 connection remote address then
 *                                    the same Q.931 connection will be used for both calls.
 *
 * Output:  None.
 *
 * Returned value: A nonnegative value on success
 *
 **************************************************************************************/

RVAPI
int RVCALLCONV cmCallMultiplex(
                        IN      HCALL               hsCall,
                        IN      HCALL               hsSameConnectionCall)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    callElem*sameConnectionCall=(callElem*)hsSameConnectionCall;
    HSTRANSHOST host;
    int value=TRUE;
    if (!hsCall || !hsSameConnectionCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp,(char*)"cmCallMultiplex(hsCall=0x%lx,hsSameConnectionCall=0x%x)",hsCall,hsSameConnectionCall);

    if (emaLock((EMAElement)hsCall))
    {
        if (emaLock((EMAElement)hsSameConnectionCall))
        {
            cmTransSetSessionParam(call->hsTransSession,cmTransParam_isMultiplexed,&value);
            if (hsSameConnectionCall)
            {
                cmTransGetSessionParam(sameConnectionCall->hsTransSession,cmTransParam_host,(void*)&host);
                cmTransSetAddress(call->hsTransSession, NULL, NULL, NULL, host, cmTransQ931Conn, FALSE);
            }
            emaUnlock((EMAElement)hsSameConnectionCall);
        }
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp,(char*)"cmCallMultiplex=%d",0);
    return 0;
}




TRANSERR cmEvTransNewAnnexLMessage(IN HSTRANSSESSION    hsTransSession,
                                   IN HATRANSSESSION    haTransSession,
                                   IN int               annexLElement,
                                   IN int               msgType)
{
    callElem*call=(callElem*)haTransSession;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;

    if (hsTransSession);
    {
        int nesting;
        cmiCBEnter(hApp,(char*)"cmEvCallNewAnnexLMessage(haCall=0x%lx,hsCall=0x%lx, annexL=%d,msgType=%d)",
            (HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,annexLElement,msgType);
        nesting=emaPrepareForCallback((EMAElement)call);
        ifE(app->cmMyCallEvent.cmEvCallNewAnnexLMessage)
            ((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,annexLElement,msgType);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit(hApp,(char*)"cmEvCallNewAnnexLMessage");
    }
    return cmTransOK;
}

TRANSERR cmEvTransNewAnnexMMessage(IN HSTRANSSESSION    hsTransSession,
                                   IN HATRANSSESSION    haTransSession,
                                   IN int               annexMElement,
                                   IN int               msgType)
{
    callElem*call=(callElem*)haTransSession;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;

    if (hsTransSession);
    {
        int nesting;
        cmiCBEnter(hApp,(char*)"cmEvCallNewAnnexMMessage(haCall=0x%lx,hsCall=0x%lx, annexM=%d,msgType=%d)",
            (HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,annexMElement,msgType);
        nesting=emaPrepareForCallback((EMAElement)call);
        ifE(app->cmMyCallEvent.cmEvCallNewAnnexMMessage)
            ((HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call,annexMElement,msgType);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit(hApp,(char*)"cmEvCallNewAnnexMMessage");
    }
    return cmTransOK;
}


/********************************************************************************************
 * cmCallCreateAnnexLMessage
 * purpose : This function creates an Annex L message. It actually creates a PVT node of
 *           type StimulusControl, and allows setting the values of this type.
 * input   : hApp           - Stack instance handle
 *           isText         - TRUE if it's a text message
 *                            FALSE otherwise
 *                          - This is a field inside the structure of StimulusControl (=Annex L)
 *           h248Message    - The buffer of the Annex L message
 *                            NULL if not needed
 *           h248MessageLength  - Length of the message in bytes
 *           nonStandard    - Non standard data associated with the Annex L message
 *                            NULL if not needed
 * output  : none
 * return  : PVT node of the annex L message on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallCreateAnnexLMessage(
    IN HAPP                hApp,
    IN BOOL                isText,
    IN char*               h248Message,
    IN int                 h248MessageLength,
    IN cmNonStandardParam* nonStandard)
{
    HPVT hVal=cmGetValTree(hApp);

    int nodeId=pvtAddRoot(hVal,((cmElem*)hApp)->synAnnexL, 0,NULL);
    int tmpNodeId;

    cmiAPIEnter(hApp,(char*)"cmCallCreateAnnexLMessage(hApp=0x%x,isText=%d,h248Message=%.50s,h248MessageLength=%d)",
        hApp,isText,h248Message,h248MessageLength);
    if (nodeId<0)
    {
        cmiAPIExit(hApp,(char*)"cmCallCreateAnnexLMessage = -1");
        return RVERROR;
    }

    if (isText)
        pvtAdd(hVal,nodeId,__q931(isText),0,NULL,NULL);
    if (h248Message)
        pvtAdd(hVal,nodeId,__q931(h248Message),h248MessageLength,h248Message,NULL);
    if (nonStandard)
    {
        tmpNodeId=pvtAdd(hVal,nodeId,__q931(nonStandard),0,NULL,NULL);
        setNonStandardParam(hVal,tmpNodeId,nonStandard);
    }
    cmiAPIExit(hApp,(char*)"cmCallCreateAnnexLMessage = %d", nodeId);
    return nodeId;
}


/********************************************************************************************
 * cmCallSendAnnexLMessage
 * purpose : This function sends an Annex L message on the network.
 * input   : hsCall     - Call to send the message in
 *           message    - PVT node of the message to send (of type StimulusControl)
 *           force      - TRUE if the message must be sent immediatly on a Facility message
 *                        FALSE if we want to send it on the next Q931 message
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSendAnnexLMessage(
    IN HCALL                hsCall,
    IN int                  message,
    IN BOOL                 force)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    int ret = RVERROR;

    if (!hApp) return RVERROR;
    cmiAPIEnter(hApp,(char*)"cmCallSendAnnexLMessage(hsCall=0x%lx,message=%d,force=%d)",
        hsCall,message,force);
    if (message<0 || !hsCall)
    {
        cmiAPIExit(hApp,(char*)"cmCallSendAnnexLMessage = -1");
        return RVERROR;
    }

    if (emaLock((EMAElement)hsCall))
    {
        TRANSERR trans=cmTransSendTunneledMessage(call->hsTransSession,message,cmTransAnnexLType,force);
        emaUnlock((EMAElement)hsCall);
        if (trans==cmTransOK)
            ret = 0;
    }

    cmiAPIExit(hApp,(char*)"cmCallSendAnnexLMessage = %d", ret);
    return ret;
}


/********************************************************************************************
 * cmCallGetAnnexLMessage
 * purpose : This function checks the values of an Annex L message.
 *           Annex L messages are piggy-backed on top of Q931 messages, inside a field
 *           called StimulusControl.
 *           This function should be used when cmEvCallNewAnnexLMessage is called.
 * input   : hApp           - Stack instance handle
 *           nodeId         - PVT Node ID of the Annex L message. It's of type StimulusControl
 *           h248MessageLength  - Maximum length of the message buffer in bytes
 * output  : isText         - TRUE if it's a text message
 *                            FALSE otherwise
 *                          - This is a field inside the structure of StimulusControl (=Annex L)
 *           h248Message    - The buffer of the Annex L message
 *           h248MessageLength  - Length of the message in bytes
 *           nonStandard    - Non standard data associated with the Annex L message
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexLMessage(
    IN    HAPP                hApp,
    IN    int                 nodeId,
    OUT   BOOL*               isText,
    OUT   char*               h248Message,
    INOUT int*                h248MessageLength,
    OUT   cmNonStandardParam* nonStandard)
{
    HPVT hVal=cmGetValTree(hApp);
    int tmpNodeId;

    cmiAPIEnter(hApp,(char*)"cmCallGetAnnexLMessage(hApp=0x%x,nodeId=%d)",hApp,nodeId);
    if (nodeId<0)
    {
        cmiAPIExit(hApp,(char*)"cmCallGetAnnexLMessage = -1");
        return RVERROR;
    }

    if (isText)
    {
        *isText=(pvtGetChild(hVal,nodeId,__q931(isText),NULL)>=0);
    }

    if (h248MessageLength)
    {
        INT32 len=0;
        tmpNodeId=pvtGetChildValue(hVal,nodeId,__q931(h248Message),&len,NULL);
        if (h248Message)
            pvtGetString(hVal,tmpNodeId,min(len,*h248MessageLength),h248Message);
        *h248MessageLength=len;
    }
    if (nonStandard)
    {
        tmpNodeId=pvtGetChild(hVal,nodeId,__q931(nonStandard),NULL);
        getNonStandardParam(hVal,tmpNodeId,nonStandard);
    }
    cmiAPIExit(hApp,(char*)"cmCallGetAnnexLMessage = 0");
    return 0;
}



/********************************************************************************************
 * cmCallCreateAnnexMMessage
 * purpose : This function creates an Annex M message. It actually creates a PVT node of
 *           type tunnelledSignallingMessage, and allows setting the values of this type.
 * input   : hApp               - Stack instance handle
 *           tunnellingRequired - TRUE if tunneling of the messages is required
 *                                FALSE otherwise
 *           tunnelledProtocolID- The identifier of the protocol being tunneled
 *                                This is a structure of type cmTunnelledProtocolID.
 *           messageContent     - The message being tunneled. It is an array of strings with
 *                                variable length. Last element in this array must point
 *                                to a NULL string.
 *           nonStandard        - Non standard data associated with the Annex M message
 * output  : none
 * return  : PVT node of the annex M message on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallCreateAnnexMMessage(
    IN HAPP                     hApp,
    IN BOOL                     tunnellingRequired,
    IN cmTunnelledProtocolID*   tunnelledProtocolID,
    IN cmOctetString*           messageContent,
    IN cmNonStandardParam*      nonStandard)
{
    HPVT hVal=cmGetValTree(hApp);

    int nodeId=pvtAddRoot(hVal,((cmElem*)hApp)->synAnnexM, 0,NULL);
    int protIdNodeId;
    int tmpNodeId;
    if (nodeId<0) return RVERROR;

    cmiAPIEnter(hApp, "cmCallCreateAnnexMMessage(hApp=0x%x)", hApp);
    if (tunnelledProtocolID)
    {
        protIdNodeId=pvtAdd(hVal,nodeId,__q931(tunnelledProtocolID),0,NULL,NULL);
        if (tunnelledProtocolID->subIdentifierLength>0)
        {

            pvtAdd(hVal,protIdNodeId,__q931(subIdentifier),tunnelledProtocolID->subIdentifierLength,tunnelledProtocolID->subIdentifier,NULL);
        }
        tmpNodeId=pvtAddBranch(hVal,protIdNodeId,__q931(id));
        if (tunnelledProtocolID->tunnelledProtocolObjectID[0])
        {
            char buff[128];
            int buffLen;

            buff[0]=0;

            buffLen = oidEncodeOID(sizeof(buff), buff, tunnelledProtocolID->tunnelledProtocolObjectID);
            pvtAdd(hVal,tmpNodeId,__q931(tunnelledProtocolObjectID), buffLen, buff,NULL);
        }
        else
        {
            tmpNodeId=pvtAddBranch(hVal,tmpNodeId,__q931(tunnelledProtocolAlternateID));
            pvtAdd(hVal,tmpNodeId,__q931(protocolType), tunnelledProtocolID->protocolTypeLength, tunnelledProtocolID->protocolType,NULL);
            if (tunnelledProtocolID->protocolVariantLength>0)
            {
                pvtAdd(hVal,tmpNodeId,__q931(protocolVariant), tunnelledProtocolID->protocolVariantLength, tunnelledProtocolID->protocolVariant,NULL);
            }
        }
    }
    if (tunnellingRequired)
        pvtAddBranch(hVal,nodeId,__q931(tunnellingRequired));
    if (nonStandard)
    {
        tmpNodeId=pvtAdd(hVal,nodeId,__q931(nonStandardData),0,NULL,NULL);
        setNonStandardParam(hVal,tmpNodeId,nonStandard);
    }
    {
        int i=0;
        tmpNodeId=pvtAddBranch(hVal,nodeId,__q931(messageContent));
        while(messageContent[i].message)
        {
            pvtAdd(hVal,tmpNodeId,-800,messageContent[i].size,messageContent[i].message,NULL);
            i++;
        }
    }
    cmiAPIExit(hApp, "cmCallCreateAnnexMMessage()=%d", nodeId);
    return nodeId;
}


/********************************************************************************************
 * cmCallSendAnnexMMessage
 * purpose : This function sends an Annex M message on the network.
 * input   : hsCall     - Call to send the message in
 *           message    - PVT node of the message to send (of type tunnelledSignallingMessage)
 *           force      - TRUE if the message must be sent immediatly on a Facility message
 *                        FALSE if we want to send it on the next Q931 message
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallSendAnnexMMessage(
    IN HCALL                hsCall,
    IN int                  message,
    IN BOOL                 force)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    callElem*call=(callElem*)hsCall;
    int ret = RVERROR;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCallSendAnnexMMessage(hsCall=0x%x,message=%d,force=%d)",hApp,message,force);
    if (message<0 || !hsCall)
    {
        cmiAPIExit(hApp, "cmCallSendAnnexMMessage() = -1");
        return RVERROR;
    }

    if (emaLock((EMAElement)hsCall))
    {
        TRANSERR trans=cmTransSendTunneledMessage(call->hsTransSession,message,cmTransAnnexMType,force);
        emaUnlock((EMAElement)hsCall);
        if (trans==cmTransOK)
            ret = 0;
    }

    cmiAPIExit(hApp, "cmCallSendAnnexMMessage() = %d", ret);
    return ret;
}


/********************************************************************************************
 * cmCallGetAnnexMMessage
 * purpose : This function checks the values of an Annex M message.
 *           Annex M messages are piggy-backed on top of Q931 messages, inside a field
 *           called tunnelledSignallingMessage.
 *           This function should be used when cmEvCallNewAnnexMMessage is called.
 *           To get the actual tunneled message, use cmCallGetAnnexMMessageContent
 * input   : hApp           - Stack instance handle
 *           nodeId         - PVT Node ID of the Annex M message.
 * output  : tunnellingRequired - TRUE if tunneling of the messages is required
 *                                FALSE otherwise
 *           tunnelledProtocolID- The identifier of the protocol being tunneled
 *                                This is a structure of type cmTunnelledProtocolID.
 *           nonStandard        - Non standard data associated with the Annex M message
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexMMessage(
    IN  HAPP                    hApp,
    IN  int                     nodeId,
    OUT BOOL*                   tunnellingRequired,
    OUT cmTunnelledProtocolID*  tunnelledProtocolID,
    OUT cmNonStandardParam*     nonStandard)
{
    HPVT hVal=cmGetValTree(hApp);

    int tmpNodeId;
    int protIdNodeId;
    cmiAPIEnter(hApp, "cmCallGetAnnexMMessage(hsCall=0x%x,nodeId=%d)",hApp,nodeId);
    if (nodeId<0)
    {
        cmiAPIExit(hApp, "cmCallGetAnnexMMessage() = -1");
        return RVERROR;
    }

    if (tunnellingRequired)
    {
        *tunnellingRequired=(pvtGetChild(hVal,nodeId,__q931(tunnellingRequired),NULL)>=0);
    }
    if (tunnelledProtocolID)
    {
        protIdNodeId=pvtGetChild(hVal,nodeId,__q931(tunnelledProtocolID),NULL);
        if (pvtGetChildValue(hVal,protIdNodeId,__q931(subIdentifier),(INT32 *)&tunnelledProtocolID->subIdentifierLength,NULL)>=0)
            pvtGetString(hVal,protIdNodeId,tunnelledProtocolID->subIdentifierLength, tunnelledProtocolID->subIdentifier);

        tmpNodeId=pvtChild(hVal,pvtGetChild(hVal,protIdNodeId,__q931(id),NULL));

        tunnelledProtocolID->tunnelledProtocolObjectID[0]=0;
        switch(pvtGetSyntaxIndex(hVal,tmpNodeId))
        {
            case 1/*tunnelledProtocolObjectID*/:
                {
                    char buff[128];
                    int buffLen;

                    buff[0]=0;
                    buffLen=pvtGetString(hVal, tmpNodeId, sizeof(buff), buff);
                    oidDecodeOID(buffLen, buff, sizeof(tunnelledProtocolID->tunnelledProtocolObjectID), tunnelledProtocolID->tunnelledProtocolObjectID, numberForm);

                }
                break;
            case 2/*tunnelledProtocolAlternateID*/:
                {
                    if (pvtGetChildValue(hVal,tmpNodeId,__q931(protocolType),(INT32 *)&tunnelledProtocolID->protocolTypeLength,NULL)>=0)
                        pvtGetString(hVal,tmpNodeId,tunnelledProtocolID->protocolTypeLength, tunnelledProtocolID->protocolType);

                    if (pvtGetChildValue(hVal,tmpNodeId,__q931(protocolVariant),(INT32 *)&tunnelledProtocolID->protocolVariantLength,NULL)>=0)
                        pvtGetString(hVal,tmpNodeId,tunnelledProtocolID->protocolVariantLength, tunnelledProtocolID->protocolVariant);
                }
                break;
        }
    }
    if (nonStandard)
    {
        tmpNodeId=pvtGetChild(hVal,nodeId,__q931(nonStandardData),NULL);
        getNonStandardParam(hVal,tmpNodeId,nonStandard);
    }
    cmiAPIExit(hApp, "cmCallGetAnnexMMessage() = 0");
    return 0;
}


/********************************************************************************************
 * cmCallGetAnnexMMessageContent
 * purpose : This function checks the values of an Annex M message.
 *           Annex M messages are piggy-backed on top of Q931 messages, inside a field
 *           called tunnelledSignallingMessage.
 *           This function should be used when cmEvCallNewAnnexMMessage is called.
 *           To get more information about the specific message being tunneled, use
 *           cmCallGetAnnexMMessage.
 * input   : hApp                   - Stack instance handle
 *           nodeId                 - PVT Node ID of the Annex M message.
 *           index                  - Specific tunneled message to get (1-based)
 *           messageContentLength   - Maximum length of buffer in bytes
 * output  : messageContentLength   - Length of message
 *           messageContent         - The message buffer itself
 * return  : Non-negative value on success
 *           Negative value on failure
 * Note    : Any of the output fields may be set to NULL.
 ********************************************************************************************/
RVAPI
int RVCALLCONV cmCallGetAnnexMMessageContent(
    IN    HAPP                hApp,
    IN    int                 nodeId,
    IN    int                 index,
    INOUT int*                messageContentLength,
    OUT   char*               messageContent)
{
    HPVT hVal=cmGetValTree(hApp);
    int ret = 0;

    int tmpNodeId;
    cmiAPIEnter(hApp, "cmCallGetAnnexMMessageContent(hsCall=0x%x,nodeId=%d,index=%d)",hApp,nodeId,index);
    if (nodeId<0)
    {
        cmiAPIExit(hApp, "cmCallGetAnnexMMessageContent() = -1");
        return RVERROR;
    }

    tmpNodeId=pvtGetByIndex(hVal,pvtGetChild(hVal,nodeId,__q931(messageContent),NULL),index,NULL);

    if (tmpNodeId>=0)
    {
        INT32 len;
        pvtGet(hVal,tmpNodeId,NULL,NULL,&len,NULL);
        if (messageContentLength)
        {
            if (messageContent)
                pvtGetString(hVal,tmpNodeId,min(len,*messageContentLength),messageContent);
            *messageContentLength=len;
        }
    }
    else
        ret = tmpNodeId;

    cmiAPIExit(hApp, "cmCallGetAnnexMMessageContent() = %d", ret);
    return ret;

}


/************************************************************************
 * callMibGetSession
 * purpose: Get the session of a handle for the MIB.
 *          This function is used to access the transport layer for MIB
 *          specific information
 * input  : call    - Stack handle for the call
 * output : none
 * return : Session of the call on success
 *          NULL on failure
 ************************************************************************/
HSTRANSSESSION callMibGetSession(IN HCALL call)
{
    return ((callElem *)call)->hsTransSession;
}


/************************************************************************
 * cmIsRoutedCall
 * purpose: Check if the call is routed or direct. Used to create perCallInfo
 *          inside IRR messages by the ras module.
 * input  : hsCall  - Call handle to check
 * output : none
 * return : TRUE if call is routed
 ************************************************************************/
BOOL cmIsRoutedCall(IN HCALL hsCall)
{
    callElem* call = (callElem *)hsCall;

    return (m_callget(call, gatekeeperRouted) != 0);
}




#ifdef __cplusplus
}
#endif
