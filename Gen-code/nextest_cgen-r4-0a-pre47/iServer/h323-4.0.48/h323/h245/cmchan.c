#ifdef __cplusplus
extern "C" {
#endif



/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/


#include <stdlib.h>

#include <rvinternal.h>
#include <mti.h>
#include <cmictrl.h>
#include <cmintr.h>
#include <cmdebprn.h>
#include <ms.h>
#include <conf.h>
#include <caputils.h>
#include <netutl.h>
#include <cmchan.h>
#include <h245.h>
#include <cmutils.h>
#include <cmCall.h>
#include <cmChanGetByXXX.h>
#include <transpcap.h>
#include <cmCrossReference.h>
#include "pvaltreeStackApi.h"

#define ifE(a) if(a)(a)

void deriveChannels(HCONTROL ctrl);
static void setFirstChannels(channelElem * channel);



/************************************************************************
 * channelFreeMemory
 * purpose: Free any PVT nodes held by a channel when not necessary
 *          anymore. This function is called only when working without
 *          a properties database.
 * input  : hsChan  - Channel object
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
static
int channelFreeMemory(
           IN   HCHAN       hsChan)
{
    cmElem* app=(cmElem *)emaGetInstance((EMAElement)hsChan);
    HPVT hVal;

    if (!app)
        return RVERROR;

    if (emaLock((EMAElement)hsChan))
    {
        channelElem* channel=(channelElem*)hsChan;
        hVal = cmGetValTree((HAPP)app);
        pvtDelete(hVal,channel->redEncID);          channel->redEncID=RVERROR;
        pvtDelete(hVal,channel->dataTypeID);        channel->dataTypeID=RVERROR;
        pvtDelete(hVal,channel->transCapID);        channel->transCapID=RVERROR;
        pvtDelete(hVal,channel->recvRtpAddressID);  channel->recvRtpAddressID=RVERROR;
        pvtDelete(hVal,channel->recvRtcpAddressID); channel->recvRtcpAddressID=RVERROR;
        pvtDelete(hVal,channel->sendRtpAddressID);  channel->sendRtpAddressID=RVERROR;
        pvtDelete(hVal,channel->sendRtcpAddressID); channel->sendRtcpAddressID=RVERROR;
        pvtDelete(hVal,channel->separateStackID);   channel->separateStackID=RVERROR;
        emaUnlock((EMAElement)hsChan);
    }

    return TRUE;
}



/************************************************************************
 * notifyChannelState
 * purpose: Notify the application about the state of a channel
 * input  : channel     - Channel object
 *          state       - State of channel
 *          stateMode   - State mode of channel
 * output : none
 * return : none
 ************************************************************************/
int notifyChannelState(
    IN channelElem*         channel,
    IN cmChannelState_e     state,
    IN cmChannelStateMode_e stateMode)
{
    HCONTROL ctrl;
    cmElem* app;
    HAPP hApp;
    int nesting;

    if (!channel) return RVERROR;
    if (!emaWasDeleted((EMAElement)channel))
    {
        ctrl=channel->ctrl;
        if (!ctrl) return RVERROR;
        hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
        if (!hApp)   return RVERROR;
        app=(cmElem*)hApp;
#ifndef NOLOGSUPPORT
        {

            static char *cmChannelStateA[] = {
              (char*)"cmChannelStateDialtone",
              (char*)"cmChannelStateRingBack",
              (char*)"cmChannelStateConnected",
              (char*)"cmChannelStateDisconnected",
              (char*)"cmChannelStateIdle",
              (char*)"cmChannelStateOffering",
            };

           static char *cmChannelStateModeA[] = {
              (char*)"cmChannelStateModeOn",
              (char*)"cmChannelStateModeOff",
              (char*)"cmChannelStateModeDisconnectedLocal",
              (char*)"cmChannelStateModeDisconnectedRemote",
              (char*)"cmChannelStateModeDisconnectedMasterSlaveConflict",
              (char*)"cmChannelStateModeDuplex",
              (char*)"cmChannelStateModeDisconnectedReasonUnknown",
              (char*)"cmChannelStateModeDisconnectedReasonReopen",
              (char*)"cmChannelStateModeDisconnectedReasonReservationFailure"
            };

            cmiCBEnter(hApp, "cmEvChannelStateChanged(haChan=0x%lx, hsChan=0x%lx,state=%s,stateMode=%s)",
                        emaGetApplicationHandle((EMAElement)channel), channel, nprn(cmChannelStateA[state]),nprn(cmChannelStateModeA[stateMode]));
        }
#endif
        if (state == cmChannelStateIdle)
        {
            /* free the default channel Ids. if needed */
            setFirstChannels(channel);
        }

        if (app->cmMyChannelEvent.cmEvChannelStateChanged)
        {
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelStateChanged((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, state, stateMode);
            emaReturnFromCallback((EMAElement)channel, nesting);
        }
        else if (state == cmChannelStateIdle)
        {
            /* We automatically close the channel if application doesn't handle this callback */
            cmChannelClose((HCHAN)channel);
        }

        if ((state==cmChannelStateConnected) && (app->callPropertyMode==pmodeDoNotUseProperty))
        {
            /* Since we're not using a properties database, we can free information from the
               channel when it gets to the connected state. */
            channelFreeMemory((HCHAN)channel);
        }
#ifndef NOLOGSUPPORT
        cmiCBExit(hApp, "cmEvChannelStateChanged.");
#endif
    }
    return 0;
}



int
cmcCallDataTypeHandleCallback(
                  /* Call the data type handle callback */
                  IN  HAPP hApp,
                  IN  HCHAN hsChan, /* channel protocol */
                  IN  int dataType, /* channel data type node id */
                  IN  confDataType type
                  )
{
  /* -- data type handle */
  {
    cmElem* app=(cmElem*)hApp;
    int dataTypeHandle=-1;
    cmCapDataType dataTypeId=(cmCapDataType)0;
    HPVT hVal=app->hVal;
    int nesting;

    switch(type) {
    case confNonStandard:
      dataTypeId=cmCapNonStandard;
      dataTypeHandle = pvtChild(hVal, dataType);
      break;
    case confNullData:
      dataTypeId=(cmCapDataType)0;
      dataTypeHandle = pvtChild(hVal, (INT32)dataType);
      break;
    case confVideoData:
      dataTypeId=cmCapVideo;
      dataTypeHandle = pvtChild(hVal, pvtChild(hVal, dataType));
      break;
    case confAudioData:
      dataTypeId=cmCapAudio;
      dataTypeHandle = pvtChild(hVal, pvtChild(hVal, dataType));
      break;
    case confData:
      dataTypeId=cmCapData;
      dataTypeHandle = pvtChild(hVal, pvtChild(hVal, pvtChild(hVal, dataType)));
      break;
    }

    cmiCBEnter(hApp, "cmEvChannelHandle: haChan=0x%x, hsChan=0x%x, handle=%d, type=%d.",
           emaGetApplicationHandle((EMAElement)hsChan), hsChan, dataTypeHandle, dataTypeId);
    nesting = emaPrepareForCallback((EMAElement)hsChan);
    ifE (app->cmMyChannelEvent.cmEvChannelHandle) ((HAPPCHAN)emaGetApplicationHandle((EMAElement)hsChan), hsChan, dataTypeHandle, dataTypeId);
    emaReturnFromCallback((EMAElement)hsChan, nesting);
    cmiCBExit(hApp, "cmEvChannelHandle.");
  }

  return TRUE;
}


int
cmcCallChannelParametersCallback(
                 /* Call the channel parameter callback */
                 IN  HAPP  hApp,
                 IN  HCHAN hsChan, /* channel protocol */
                 IN  int dataType, /* channel data type node id */
                 OUT confDataType* type
                 )
{
    cmElem* app=(cmElem*)hApp;
    char channelName[64];
    INT32 rate=0;
    confDataType _type;
    HCHAN hsSameSessionH;
    HAPPCHAN haSameSessionH;
    HPVT hVal=app->hVal;
    int nesting;

    /* -- channel name */
    strcpy(channelName, "null");
    if(confGetDataTypeName(hVal, dataType, sizeof(channelName), channelName, &_type, NULL) < 0)
    {
        logPrint(app->log, RV_ERROR, (app->log, RV_ERROR, "cmcCallChannelParametersCallback: error in confGetDataTypeName(), dataType=%d", dataType));
        if (type != NULL)
            *type = (confDataType)-1;
        return RVERROR;
    }
    logPrint(app->log, RV_DEBUG, (app->log, RV_DEBUG, "cmcCallChannelParametersCallback: new channel with name %s", channelName));

    /* -- bit rate */
    if (_type == confData)
    {
        int nid;
        __pvtGetByFieldIds(nid,hVal, dataType,
                            {_h245(data)
                            _h245(maxBitRate)
                            LAST_TOKEN},
                            NULL, &rate, NULL);
    }

    /* -- same session handle */
    cmChannelSameSession(hsChan, &haSameSessionH, &hsSameSessionH);

    cmiCBEnter(hApp, "cmEvChannelParameters: haChan=0x%p, hsChan=0x%p, channelName=%s, "
         "AppSes=0x%p, Ses=0x%p, NULL, NULL, rate=%d.",
         emaGetApplicationHandle((EMAElement)hsChan), hsChan, channelName, haSameSessionH, hsSameSessionH, rate);
    nesting = emaPrepareForCallback((EMAElement)hsChan);
    ifE (app->cmMyChannelEvent.cmEvChannelParameters)
        ((HAPPCHAN)emaGetApplicationHandle((EMAElement)hsChan), hsChan, channelName, haSameSessionH, hsSameSessionH, NULL, NULL, rate);
    emaReturnFromCallback((EMAElement)hsChan, nesting);
    cmiCBExit(hApp, "cmEvChannelParameters.");

    if (type) *type = _type;
    return TRUE;
}



/* Channel Operations______________________________________________________________________*/




RVAPI int RVCALLCONV
cmSetChannelEventHandler(
             /* Set user callbacks functions for control channels. */
             IN HAPP        hApp,
             IN     CMCHANEVENT cmChannelEvent,
             IN     int     size)
{
  cmElem *app = (cmElem *)hApp;

  if (!app) return RVERROR;

  cmiAPIEnter(hApp, "cmSetChannelEventHandler: hApp=0x%lx, cmChannelEvent=0x%lx, size=%d.", hApp, cmChannelEvent, size);
  memset(&app->cmMyChannelEvent,0,sizeof(app->cmMyChannelEvent));
  memcpy(&app->cmMyChannelEvent,cmChannelEvent,min((int)sizeof(app->cmMyChannelEvent),size));
  cmiAPIExit(hApp, "cmSetChannelEventHandler: [0].");
  return 0;
}

RVAPI
int RVCALLCONV cmGetChannelEventHandler(
        IN      HAPP                hApp,
        OUT      CMCHANEVENT         cmChannelEvent,
        IN      int                 size)
{
  cmElem *app = (cmElem *)hApp;

  if (!app) return RVERROR;

  cmiAPIEnter(hApp, "cmGetChannelEventHandler: hApp=0x%lx, cmChannelEvent=0x%lx, size=%d.", hApp, cmChannelEvent, size);
  memset(cmChannelEvent,0,sizeof(app->cmMyChannelEvent));
  memcpy(cmChannelEvent,&app->cmMyChannelEvent,min((int)sizeof(app->cmMyChannelEvent),size));
  cmiAPIExit(hApp, "cmGetChannelEventHandler: [0].");
  return 0;
}

channelElem* allocateChannel(HCONTROL ctrl)
{
    channelElem channel,*ch = NULL;
    memset(&channel,0xff,sizeof(channelElem));
    channel.fastStartChannelIndex=-1;
    channel.dynamicPayloadNumber=-1;
    channel.partner=NULL;
    channel.base=NULL;
    channel.associated=NULL;
    channel.replacementCh=NULL;
    channel.isDuplex=0; /* by default it is simplex (uni-directional) channel. */
    channel.sid=0;
    channel.flowControlToZero=0;
    if (((controlElem*)ctrl)->lcnOut<1)
        ((controlElem*)ctrl)->lcnOut=1;
    channel.myLCN=++((controlElem*)ctrl)->lcnOut;
    channel.rvrsLCN=0;
    channel.ctrl=ctrl;
    channel.origin=TRUE;
    channel.recvRtcpAddressID=RVERROR;
    channel.recvRtpAddressID=RVERROR;
    channel.sendRtcpAddressID=RVERROR;
    channel.sendRtpAddressID=RVERROR;
    channel.redEncID=RVERROR;
    channel.transCapID=RVERROR;
    channel.separateStackID=RVERROR;
    channel.t120SetupProcedure=RVERROR;
    channel.rc_paramID=RVERROR;
    channel.dataTypeID=RVERROR;
    channel.state=channelIdle;
    channel.port=RVERROR;
    channel.remotePortNumber=RVERROR;
    channel.timer = (HTI)RVERROR;
    channel.rc_timer = (HTI)RVERROR;
    channel.ml_timer = (HTI)RVERROR;
    ch = (channelElem *)emaAdd(((cmElem*)(emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl))))->hChannels,NULL);
    if (ch)
    {
        emaLinkToElement((EMAElement)ch, (EMAElement)cmiGetByControl(ctrl));

        memcpy(ch,&channel,sizeof(channelElem));
        if (!addChannelForCtrl(ctrl,(HCHAN)ch))
        {
            emaDelete(ch);
            return NULL;
        }
    }
    return ch;
}


RVAPI int RVCALLCONV
cmChannelNew (
          IN    HCALL       hsCall,
          IN    HAPPCHAN    haChan,
          OUT   LPHCHAN     lphsChan)
{
    HCONTROL ctrl=cmiGetControl(hsCall);
    channelElem*channel;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelNew: hsCall=0x%lx, haChan=0x%lx, lphsChan=0x%lx.", hsCall, haChan, lphsChan);
    if (emaLock((EMAElement)hsCall))
    {
        channel=allocateChannel(ctrl);
        if (!channel)
        {
            *lphsChan = NULL;
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit(hApp, "cmChannelNew: [RESOURCE PROBLEM].");
            return RESOURCES_PROBLEM;
        }
        emaSetApplicationHandle((EMAElement)channel,(void*)haChan);
        channel->origin=TRUE;
        channel->ctrl=ctrl;
        *lphsChan=(HCHAN)channel;
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmChannelNew: *lphsChan=0x%lx [OK].", nprnd((int*)lphsChan));
    return 0;
}



RVAPI int RVCALLCONV
cmChannelGetCallHandles(
            /* Get the stack and application call handles by the channel handle */
            IN  HCHAN hsChan,
            OUT HCALL *hsCall,
            OUT HAPPCALL *haCall
            )
{
    channelElem*channel=(channelElem*)hsChan;
    HCONTROL ctrl;
    HCALL call;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);

    if (!hApp || !channel) return RVERROR;
    cmiAPIEnter(hApp, "cmChannelGetCallHandles:hsChan=0x%x",hsChan );

    if (emaLock((EMAElement)hsChan))
    {
        ctrl=(HCONTROL)channel->ctrl;
        call=cmiGetByControl(ctrl);

        if (ctrl)
        {
            if (hsCall) *hsCall = call;
            if (haCall) *haCall = (HAPPCALL)emaGetApplicationHandle((EMAElement)call);
        }
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelGetCallHandles:hsCall =0x%x",hsCall );
    return TRUE;
}

void RVCALLCONV channelTimeoutEventsHandler(void*hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    cmElem* app=(cmElem*)hApp;
    HPVT hVal=cmGetValTree(hApp);
    int message,nodeId;

    if (!hApp) return;

    if (emaLock((EMAElement)hsChan))
    {
        cmTimerReset(hApp,&(channel->timer));
        if (channel->state!=released)
        {
            emaMark((EMAElement)hsChan);
            if (channel->state==awaitingEstablishment)
            {
                    message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
                    nodeId=pvtAddBranch2(hVal,message, __h245(request),__h245(closeLogicalChannel));
                    pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
                    pvtAddBranch2(hVal,nodeId,__h245(source),__h245(lcse));

                    sendMessageH245Chan(channel->ctrl, hsChan, message);
                    pvtDelete(hVal,message);
                    channel->state=released;
                    {
                        void* ptr=NULL;
                        channelElem* dependent;
                        while((dependent=getNextOutChanByBase(channel->ctrl,channel,&ptr)))
                        {/* release the dependent channels because the base timed out */
                            if (dependent->state!=released)
                            {
                                ptr=NULL;
                                emaLock((EMAElement)dependent);
                                emaUnlock((EMAElement)hsChan);
                                dependent->state=released;
                                notifyChannelState(dependent,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                                notifyChannelState(dependent,cmChannelStateIdle, cmChannelStateModeOff);
                                emaLock((EMAElement)hsChan);
                                emaUnlock((EMAElement)dependent);
                            }
                        }
                    }
                    ((controlElem*)channel->ctrl)->conflictChannels--;
            }
            channel->state=released;
            notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
            notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            emaRelease((EMAElement)hsChan);
        }
        emaUnlock((EMAElement)hsChan);
    }
}

void RVCALLCONV channelRC_TimeoutEventsHandler(void*hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message,nodeId;
    cmElem*app=(cmElem*)hApp;
    int nesting;

    if (!hApp)   return;

    if (emaLock((EMAElement)hsChan))
    {
        cmTimerReset(hApp,&(channel->rc_timer));

        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(requestChannelCloseRelease));
        pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);

        sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);

        cmiCBEnter(hApp, "cmEvChannelRequestCloseStatus: haChan=0x%lx, hsChan=0x%lx, status=reject.",
                         (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)hsChan);
        ifE(app->cmMyChannelEvent.cmEvChannelRequestCloseStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmRequestCloseReject);
        emaReturnFromCallback((EMAElement)hsChan, nesting);
        emaUnlock((EMAElement)hsChan);
        cmiCBExit(hApp, "cmEvChannelRequestCloseStatus.");
    }
}

int startEstablishment(cmElem* app, channelElem*channel)
{
    HPVT hVal=app->hVal;
    int ret;
    int message,olcID, forwardLCP_ID,reverseLCP_ID,h225ID,dataType,tmpID;
    message = pvtAddRoot(hVal,app->synProtH245,0,NULL);
    olcID=pvtAddBranch2(hVal, message, __h245(request), __h245(openLogicalChannel));
    forwardLCP_ID=pvtAddBranch(hVal, olcID, __h245(forwardLogicalChannelParameters));
    /* reverse data type. set only if not set and in duplex mode.*/
    reverseLCP_ID=(channel->isDuplex) ? pvtAddBranch(hVal, olcID, __h245(reverseLogicalChannelParameters)): RVERROR;

    dataType=pvtAddBranch(hVal, forwardLCP_ID, __h245(dataType));

    ret = pvtSetTree(hVal, dataType, hVal, channel->dataTypeID);
    /* -- notify application */

    h225ID=pvtAddBranch2(hVal, forwardLCP_ID, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));

    /* -- dynamic payload */
    if (channel->dynamicPayloadNumber>=0)
    {
        pvtAdd(hVal,h225ID,__h245(dynamicRTPPayloadType) ,channel->dynamicPayloadNumber ,NULL,NULL);
    }


    if (channel->base && channel->base->state==established)
    {
        pvtAdd(hVal,forwardLCP_ID,__h245(forwardLogicalChannelDependency),channel->base->myLCN,NULL,NULL);
        if (channel->base->rvrsLCN)
            pvtAdd(hVal,reverseLCP_ID,__h245(reverseLogicalChannelDependency),channel->base->rvrsLCN,NULL,NULL);

    }

    if (channel->replacementCh && channel->replacementCh->state==established)
    {
        pvtAdd(hVal,forwardLCP_ID,__h245(replacementFor),channel->replacementCh->myLCN,NULL,NULL);
        if (channel->replacementCh->rvrsLCN)
            pvtAdd(hVal,reverseLCP_ID,__h245(replacementFor),channel->replacementCh->rvrsLCN,NULL,NULL);
    }

    /* startEstablish */
    pvtAdd(hVal,olcID,__h245(forwardLogicalChannelNumber), channel->myLCN,NULL,NULL);

    /* set sid in message */
    pvtAdd(hVal,h225ID,__h245(sessionID),channel->sid,NULL,NULL);

    if (channel->associated)
        pvtAdd(hVal,h225ID,__h245(associatedSessionID),channel->associated->sid,NULL,NULL);



    /* add new entry to session db */
    if (channel->redEncID>=0)
        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(redundancyEncoding)),hVal, channel->redEncID);


    if (channel->recvRtpAddressID>=0)
        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaChannel)),hVal, channel->recvRtpAddressID);
    if (channel->port>0 && channel->port<65536)
        pvtAdd(hVal,forwardLCP_ID,__h245(portNumber),channel->port,NULL,NULL);
    if (channel->transCapID>=0)
        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(transportCapability)),hVal, channel->transCapID);
    if (channel->source.mcuNumber!=0xff)
    {
        int sID=pvtAddBranch(hVal,h225ID,__h245(source));
        int iMCU = channel->source.mcuNumber;
        int iTerminal = channel->source.terminalNumber;
        pvtAdd(hVal,sID,__h245(mcuNumber),iMCU,NULL,NULL);
        pvtAdd(hVal,sID,__h245(terminalNumber),iTerminal,NULL,NULL);
    }
    if (channel->destination.mcuNumber!=0xff)
    {
        int sID=pvtAddBranch(hVal,h225ID,__h245(source));
        int iMCU = channel->destination.mcuNumber;
        int iTerminal = channel->destination.terminalNumber;
        pvtAdd(hVal,sID,__h245(mcuNumber),iMCU,NULL,NULL);
        pvtAdd(hVal,sID,__h245(terminalNumber),iTerminal,NULL,NULL);
    }

    if (channel->separateStackID >= 0)
    {
        int separateStack;

        separateStack=pvtAddBranch(hVal,olcID,__h245(separateStack));
        pvtSetTree(hVal,pvtAddBranch2(hVal,separateStack,__h245(networkAddress), __h245(localAreaAddress)),hVal, channel->separateStackID);

        if (channel->externalReferenceLength > 0)
            pvtAdd(hVal,separateStack,__h245(externalReference),channel->externalReferenceLength,channel->externalReference,NULL);
        pvtAdd(hVal,separateStack,__h245(associateConference),channel->isAssociated,NULL,NULL);
        if ((channel->t120SetupProcedure >= cmOriginateCall) &&
            (channel->t120SetupProcedure <= cmIssueQuery))
        {
            int t120s[]={__h245(originateCall),__h245(waitForCall),__h245(issueQuery)};
            pvtAddBranch2(hVal,separateStack,__h245(t120SetupProcedure),t120s[channel->t120SetupProcedure-cmOriginateCall]);
        }

    }

    /* duplex default parameter setting.*/
    if (channel->isDuplex)
    {
        int mpNodeId;
        if ((tmpID=pvtAddBranch(hVal,reverseLCP_ID,__h245(dataType)))>=0)
            pvtSetTree(hVal,tmpID,hVal,dataType);
        mpNodeId = pvtAddBranch2(hVal, reverseLCP_ID, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));
        pvtAdd(hVal,mpNodeId,__h245(sessionID),channel->sid,NULL,NULL);
    }
    else
    {
        if (channel->partner)
        {
            if (channel->partner->recvRtcpAddressID>=0)
                pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaControlChannel)),hVal,channel->partner->recvRtcpAddressID);
            else if (channel->recvRtcpAddressID>=0)
                pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaControlChannel)),hVal,channel->recvRtcpAddressID);
        }
        else if (channel->recvRtcpAddressID>=0)
            pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaControlChannel)),hVal,channel->recvRtcpAddressID);
        /* associateSessionId() */

    }

    /* Notify channel's states. These are used to simulate earlier version behavior on channels */
    channel->state = channelIdle;
    notifyChannelState(channel,cmChannelStateDialtone, cmChannelStateModeOff);
    notifyChannelState(channel,cmChannelStateRingBack, cmChannelStateModeOff);

    /* send message */
    if (!emaWasDeleted((EMAElement)channel))
    {
        ret = sendMessageH245Chan(channel->ctrl, (HCHAN)channel, message);
        pvtDelete(hVal,message);
        cmTimerReset((HAPP)app,&(channel->timer));

        if (ret >= 0)
        {
            /* Set a timer on this channel while we wait for an ack */
            INT32 timeout=29;
            pvtGetChildValue(hVal, app->h245Conf, __h245(channelsTimeout), &(timeout), NULL);
            channel->timer = cmTimerSet((HAPP)app, channelTimeoutEventsHandler, (void*)channel, timeout * 1000);
        }

        /* Change the state of this channel */
        channel->state = awaitingEstablishment;
    }

    return ret;
}


RVAPI int RVCALLCONV
cmChannelOpenDynamic(
             IN      HCHAN               hsChan,
             IN      int                 dataTypeHandle, /* data type (HPVT) tree handle */
             IN      HCHAN               hsChanSameSession,
             IN      HCHAN               hsChanAssociated,
             IN      BOOL                isDynamicPayloadType
             )
{
    cmElem *app=(cmElem *)emaGetInstance((EMAElement)hsChan);
    HPVT hVal=cmGetValTree((HAPP)app);

    if (!app)
        return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelOpenDynamic: hsChan=0x%lx, dataTypeHandle=%d, hsChanSameSession=0x%lx, hsChanAssociated=0x%lx, isDynamicPayloadType=%d.",
                hsChan, dataTypeHandle, hsChanSameSession, hsChanAssociated, isDynamicPayloadType);

    if (emaLock((EMAElement)hsChan))
    {
        HCONTROL ctrl;
        controlElem* ctrlE;
        channelElem* channel=(channelElem*)hsChan;
        callElem * call;
        ctrl = channel->ctrl;
        call = (callElem *)cmiGetByControl(ctrl);
        ctrlE = (controlElem*)ctrl;
        if (!m_callget(call,control) || ((ctrlE->state != ctrlConnected) && (ctrlE->state != ctrlConference)))
        {
            channel->state = released;
            notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
            notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: Control is not initialized (probably in FastStart)");
            return RVERROR;
        }

        deriveChannels(channel->ctrl);
        if (hsChanSameSession != NULL)
        {
            channel->partner=(channelElem*)hsChanSameSession;  /* -- join session */
            channel->sid=channel->partner->sid;
        }
        if (hsChanAssociated != NULL)  channel->associated=(channelElem*)hsChanAssociated;/* -- associate session */

        if (channel->base && channel->base->state==awaitingRelease)
        {
            channel->state = released;
            notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
            notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: Base channel released [Invalid Circumstances]");
            return RVERROR;
        }
        if (channel->dataTypeID<0)
            channel->dataTypeID=pvtAddRoot(hVal,app->h245DataType,0,NULL);
        pvtAddTree(hVal,channel->dataTypeID,hVal,dataTypeHandle);

        if (isDynamicPayloadType)
        {
            cmLock((HAPP)app);
            app->dynamicPayloadNumber = (char)((app->dynamicPayloadNumber+1)%32);
            channel->dynamicPayloadNumber=app->dynamicPayloadNumber+96;
            cmUnlock((HAPP)app);
        }

        if (channel->base && channel->base->state==awaitingEstablishment)
        {
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: Waiting for base channel [OK]");
            return TRUE;
        }
        if (!channel->isDuplex)
        {
            if (!channel->partner)
            { /* new id */

                INTPTR fieldId;

                int nodeId = pvtGet(hVal,pvtChild(hVal,channel->dataTypeID),&fieldId,NULL,NULL,NULL);
                if (fieldId==__h245(h235Media))
                    pvtGet(hVal,pvtChild(hVal,pvtGetChild(hVal,nodeId,__h245(mediaType),NULL)),&fieldId,NULL,NULL,NULL);

                channel->sid=0;

                if (ctrlE->firstAudioOut && fieldId==__h245(audioData))     { ctrlE->firstAudioOut=0; channel->sid=1; channel->partner=getInChanBySID(ctrl,channel->sid);}
                else if (ctrlE->firstVideoOut && fieldId==__h245(videoData)){ ctrlE->firstVideoOut=0; channel->sid=2; channel->partner=getInChanBySID(ctrl,channel->sid);}
                else if (ctrlE->firstDataOut && fieldId==__h245(data))     { ctrlE->firstDataOut=0;  channel->sid=3; channel->partner=getInChanBySID(ctrl,channel->sid);}
                else if (ctrlE->isMaster)
                {
                    if ((channel->sid = getFreeSID(ctrl))<0)
                    {
                        channel->state = released;
                        notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                        notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
                        emaUnlock((EMAElement)hsChan);
                        cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: Out of session IDs [Resource Problem]");
                        return RESOURCES_PROBLEM;
                    }
                }

            }
            else
            /* check database for partner lcn. In case of first audio or video */
            { /* use partner sid */
                if (channel->sid==1) ctrlE->firstAudioOut=0;
                if (channel->sid==2) ctrlE->firstVideoOut=0;
                if (channel->sid==3) ctrlE->firstDataOut=0;
            }
        }
        else
        {
            if (!channel->partner)
            { /* new id */

                INTPTR fieldId;

                int nodeId = pvtGet(hVal,pvtChild(hVal,channel->dataTypeID),&fieldId,NULL,NULL,NULL);
                if (fieldId==__h245(h235Media))
                    pvtGet(hVal,pvtChild(hVal,pvtGetChild(hVal,nodeId,__h245(mediaType),NULL)),&fieldId,NULL,NULL,NULL);

                channel->sid=0;

                if (ctrlE->firstAudioOut && fieldId==__h245(audioData))     { ctrlE->firstAudioOut=0; ctrlE->firstAudioIn=0;    channel->sid=1;}
                else if (ctrlE->firstVideoOut && fieldId==__h245(videoData)){ ctrlE->firstVideoOut=0; ctrlE->firstVideoIn=0;    channel->sid=2;}
                else if (ctrlE->firstDataOut && fieldId==__h245(data))     { ctrlE->firstDataOut=0;  ctrlE->firstDataIn=0;  channel->sid=3;}
                else if (ctrlE->isMaster)
                {
                    if ((channel->sid = getFreeSID(ctrl))<0)
                    {
                        channel->state = released;
                        notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                        notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
                        emaUnlock((EMAElement)hsChan);
                        cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: Out of session IDs [Resource Problem]");
                        return RESOURCES_PROBLEM;
                    }
                }
            }
            else
            /* check database for partner lcn. In case of first audio or video */
            { /* use partner sid */
                if (channel->sid==1) { ctrlE->firstAudioOut=0; ctrlE->firstAudioIn=0;}
                if (channel->sid==2) { ctrlE->firstVideoOut=0; ctrlE->firstVideoIn=0;}
                if (channel->sid==3) { ctrlE->firstDataOut=0;  ctrlE->firstDataIn=0;}
            }
        }
        ctrlE->conflictChannels++;
        if(startEstablishment(app, channel)<0)
        {
            channel->state = released;
            notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
            notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: startEstablishment failed");
            return RVERROR;
        }
        emaUnlock((EMAElement)hsChan);
    }

    if (app->mibEvent.h341AddNewLogicalChannel)
        app->mibEvent.h341AddNewLogicalChannel(app->mibHandle, hsChan);

    cmiAPIExit((HAPP)app, "cmChannelOpenDynamic: [1].");
    return TRUE;
}

/* NOTE: origin should be the origin of the MESSAGE - as the recerive and transmit address assosiation is
determined by the EP to send the message */
int cmcCallAddressCallbacks(cmElem* app, channelElem* channel, int h225ParamNodeId, BOOL origin)
{
    HAPP hApp=(HAPP)app;
    int tmpNodeId;
    HPVT hVal=app->hVal;
    int nesting;
    int iLength;

    tmpNodeId=pvtGetChild(hVal,h225ParamNodeId,__h245(mediaControlChannel),NULL);
    if (tmpNodeId>=0)
    { /* -- RTCP */
        cmTransportAddress ta;
        if (cmVtToTA_H245(hVal,tmpNodeId, &ta) <0)
        {
            logPrint(app->logERR, RV_WARN,
                (app->logERR, RV_WARN, "Channel:establishIndication:RTCP: no IP address."));
        }
        else
        {
            if(app->cmMyChannelEvent.cmEvChannelSetRTCPAddress)
            {
                cmiCBEnter(hApp, "cmEvChannelSetRTCPAddress: haChan=0x%x, hsChan=0x%x, ip=0x%x, port=%d.",
                    emaGetApplicationHandle((EMAElement)channel), channel, ta.ip, ta.port);
                nesting = emaPrepareForCallback((EMAElement)channel);
                app->cmMyChannelEvent.cmEvChannelSetRTCPAddress((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, ta.ip, ta.port);
                emaReturnFromCallback((EMAElement)channel, nesting);
                cmiCBExit(hApp, "cmEvChannelSetRTCPAddress.");
            }
        }
        if (!emaWasDeleted((EMAElement)channel))
        {
            int* tmpNodeId1;
            if (origin) tmpNodeId1 = &channel->recvRtcpAddressID;
            else        tmpNodeId1=  &channel->sendRtcpAddressID;
            if ((*tmpNodeId1) < 0) *tmpNodeId1 = pvtAddRoot(hVal,NULL,0,NULL);
            pvtSetTree(hVal, *tmpNodeId1, hVal, tmpNodeId);
        }
    }
    tmpNodeId=pvtGetChild(hVal,h225ParamNodeId,__h245(mediaChannel),NULL);
    if (tmpNodeId>=0)
    { /* -- RTP */
        cmTransportAddress ta;
        if (cmVtToTA_H245(hVal,tmpNodeId, &ta) <0)
        {
            logPrint(app->logERR, RV_WARN,
                     (app->logERR, RV_WARN, "Channel:establishIndication:RTP: no IP address."));
        }
        else
        {
            switch(ta.type)
            {
                case cmTransportTypeIP:
                    cmiCBEnter(hApp, "cmEvChannelSetAddress: haChan=0x%x, hsChan=0x%x, ip=0x%x, port=%d.",
                        emaGetApplicationHandle((EMAElement)channel), channel, ta.ip, ta.port);
                    nesting = emaPrepareForCallback((EMAElement)channel);
                    ifE (app->cmMyChannelEvent.cmEvChannelSetAddress)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, ta.ip, ta.port);
                    emaReturnFromCallback((EMAElement)channel, nesting);
                    cmiCBExit(hApp, "cmEvChannelSetAddress.");
                break;
                case cmTransportTypeNSAP:
                    cmiCBEnter(hApp, "cmEvChannelSetNSAPAddress: haChan=0x%x, hsChan=0x%x, addressLength=%d.", emaGetApplicationHandle((EMAElement)channel), channel, ta.length);
                    nesting = emaPrepareForCallback((EMAElement)channel);
                    iLength = ta.length;
                    ifE (app->cmMyChannelEvent.cmEvChannelSetNSAPAddress)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, (char *)ta.route,iLength,
                                                                ta.distribution==cmDistributionMulticast);
                    emaReturnFromCallback((EMAElement)channel, nesting);
                    cmiCBExit(hApp, "cmEvChannelSetNSAPAddress.");
                    if (!emaWasDeleted((EMAElement)channel) && channel->remotePortNumber>=0)
                    {
                        cmiCBEnter(hApp, "cmEvChannelSetATMVC: haChan=0x%x, hsChan=0x%x, portNumber=%d.", emaGetApplicationHandle((EMAElement)channel), channel, channel->remotePortNumber);
                        nesting = emaPrepareForCallback((EMAElement)channel);
                        ifE (app->cmMyChannelEvent.cmEvChannelSetATMVC) ((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, channel->remotePortNumber);
                        emaReturnFromCallback((EMAElement)channel, nesting);
                        cmiCBExit(hApp , "cmEvChannelSetATMVC.");
                    }
                break;
                default:
                break;
            }
        }
        {
            int* tmpNodeId1;
            if (origin) tmpNodeId1 = &channel->recvRtpAddressID;
            else        tmpNodeId1=  &channel->sendRtpAddressID;
            if ((*tmpNodeId1) < 0) *tmpNodeId1 = pvtAddRoot(hVal,NULL,0,NULL);
            pvtSetTree(hVal, *tmpNodeId1, hVal, tmpNodeId);
        }
    }
    return TRUE;
}

static
int rejectChannel(channelElem* channel, int rejectReason, int rejectMode)
{
    HCONTROL ctrl=channel->ctrl;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
    HPVT hVal=app->hVal;
    int message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
    int nodeId=pvtAddBranch2(hVal,message, __h245(response),__h245(openLogicalChannelReject));
    int res;

    pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
    pvtAddBranch2(hVal,nodeId,__h245(cause),rejectReason);

    res = sendMessageH245Chan(ctrl, (HCHAN)channel, message);
    pvtDelete(hVal,message);

    channel->state = released;
    notifyChannelState(channel,cmChannelStateDisconnected,(cmChannelStateMode_e)rejectMode);
    notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
    return res;
}

static
int openLogicalChannelEvent(IN cmElem* app, IN HPVT hVal, IN channelElem* channel, IN int message)
{
    /* This function assumes that the channel element is locked */

    /*openLCNevent*/
    controlElem* ctrl;
    INT32 baseLCN,replaceLCN;
    int forwardLCP_ID,reverseLCP_ID,h225ID;
    int nesting;

    if (app->mibEvent.h341AddNewLogicalChannel)
          app->mibEvent.h341AddNewLogicalChannel(app->mibHandle, (HCHAN)channel);

    ctrl = (controlElem *)channel->ctrl;
    forwardLCP_ID=pvtGetChild(hVal,message, __h245(forwardLogicalChannelParameters), NULL);
    pvtGetChildValue(hVal,forwardLCP_ID,__h245(portNumber),(INT32 *)&channel->remotePortNumber,NULL);

    if (pvtGetChildValue(hVal,forwardLCP_ID, __h245(forwardLogicalChannelDependency), &baseLCN, NULL)>=0)
    {
        channel->base=getInChanByLCN((HCONTROL)ctrl,baseLCN);
        if (!channel->base)
            return rejectChannel(channel,__h245(invalidDependentChannel),cmChannelStateModeDisconnectedLocal);
    }
    if (pvtGetChildValue(hVal,forwardLCP_ID, __h245(replacementFor), &replaceLCN, NULL)>=0)
    {
        channel->replacementCh=getInChanByLCN((HCONTROL)ctrl,replaceLCN);
        if (!channel->replacementCh)
            return rejectChannel(channel,__h245(replacementForRejected),cmChannelStateModeDisconnectedLocal);
    }

    reverseLCP_ID=pvtGetChild(hVal,message, __h245(reverseLogicalChannelParameters), NULL);

    /* check for bi-directional channnel.*/
    if (reverseLCP_ID>=0)
        channel->isDuplex=TRUE;

    {
        int tmpId;
        tmpId=pvtGetChild(hVal,forwardLCP_ID,__h245(dataType),NULL);
        if (tmpId>=0)
        {
            if (channel->dataTypeID<0)
                channel->dataTypeID=pvtAddRoot(hVal,NULL,0,NULL);
            pvtMoveTree(hVal,channel->dataTypeID,tmpId);
        }
    }

    /* separateStack */
    if (channel->isDuplex)
    {
        int seperateStackNode, tmpId;

        if(pvtGetChild(hVal, message, __h245(separateStack), &seperateStackNode) >= 0)
        {
            __pvtGetNodeIdByFieldIds(tmpId, hVal, seperateStackNode, {_h245(networkAddress) _h245(localAreaAddress) LAST_TOKEN});
            if (tmpId >= 0)
            {
                if (channel->separateStackID < 0)
                    channel->separateStackID = pvtAddRoot(hVal, NULL, 0, NULL);
                pvtMoveTree(hVal, channel->separateStackID, tmpId);
            }
            if(pvtGetChild(hVal, seperateStackNode, __h245(associateConference), &tmpId) >= 0)
                pvtGet(hVal, tmpId, NULL, NULL, (INT32*)&channel->isAssociated, NULL);
            if(pvtGetChild(hVal, seperateStackNode, __h245(externalReference), &tmpId) >= 0)
                channel->externalReferenceLength = pvtGetString(hVal, tmpId, 256, channel->externalReference);
        }
    }

    /* H.225 section. */
    h225ID=pvtGetChild2(hVal,forwardLCP_ID, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));
    if (h225ID>=0)
    {
        /* handle session id.*/
        pvtGetChildValue(hVal,h225ID,__h245(sessionID),(INT32 *)&channel->sid,NULL);
        if (channel->sid==0)
        {
            if (!channel->isDuplex)
            {
                if (ctrl->conflictChannels)
                  /* conflict */
                    return rejectChannel(channel,__h245(masterSlaveConflict),cmChannelStateModeDisconnectedMasterSlaveConflict);

            }

            /* new session */
            if (ctrl->isMaster || channel->isDuplex)
            { /* master*/
                INTPTR fieldId;
                int nodeId = pvtGet(hVal,pvtChild(hVal,channel->dataTypeID),&fieldId,NULL,NULL,NULL);
                if (fieldId==__h245(h235Media))
                    pvtGet(hVal,pvtChild(hVal,pvtGetChild(hVal,nodeId,__h245(mediaType),NULL)),&fieldId,NULL,NULL,NULL);

                if (ctrl->firstAudioIn && fieldId==__h245(audioData))     { ctrl->firstAudioIn=0; channel->sid=1;}
                else if (ctrl->firstVideoIn && fieldId==__h245(videoData)){ ctrl->firstVideoIn=0; channel->sid=2;}
                else if (ctrl->firstDataIn && fieldId==__h245(data))      { ctrl->firstDataIn=0;  channel->sid=3;}
                else
                {
                    if ((channel->sid = getFreeSID((HCONTROL)ctrl))<0)
                        return rejectChannel(channel,__h245(invalidSessionID),cmChannelStateModeDisconnectedLocal);
                }
           }
           else
            /* slave */
                return rejectChannel(channel,__h245(invalidSessionID),cmChannelStateModeDisconnectedLocal);
        }
        else
        { /* existing session association */
           /* check db consistancy */
            if (checkChanSIDConsistency((HCONTROL)ctrl,channel))
                /* in chan exists */
                return rejectChannel(channel,__h245(invalidSessionID),cmChannelStateModeDisconnectedLocal);

            /* update db.first */
            if (channel->sid==1) ctrl->firstAudioIn=0;
            if (channel->sid==2) ctrl->firstVideoIn=0;
            if (channel->sid==3) ctrl->firstDataIn=0;

            /* link partner rtcp address */
            channel->partner = getOutChanBySID((HCONTROL)ctrl, channel->sid);
        }
        /* link associated session */
        {
            INT32 sid;
            if (pvtGetChildValue(hVal,h225ID,__h245(associatedSessionID),&sid,NULL)>=0)
                channel->associated = getInChanBySID((HCONTROL)ctrl, sid);
        }

    } /* handleSessionId() */

    {
        int tmpId;
        tmpId=pvtGetChild(hVal,h225ID,__h245(transportCapability),NULL);
        if (tmpId>=0)
        {
            if (channel->transCapID<0)
                channel->transCapID=pvtAddRoot(hVal,NULL,0,NULL);
            pvtMoveTree(hVal,channel->transCapID,tmpId);
        }
        else if (channel->transCapID >= 0)
        {
            pvtDelete(hVal, channel->transCapID);
            channel->transCapID = -1;
        }
        tmpId=pvtGetChild(hVal,h225ID,__h245(redundancyEncoding),NULL);
        if (tmpId>=0)
        {
            if (channel->redEncID<0)
                channel->redEncID=pvtAddRoot(hVal,NULL,0,NULL);
            pvtMoveTree(hVal,channel->dataTypeID,tmpId);
        }
        tmpId=pvtGetChild(hVal,h225ID,__h245(source),NULL);
        if (tmpId>=0)
        {
            INT32 tmp;
            pvtGetChildValue(hVal,tmpId,__h245(mcuNumber), &(tmp),NULL);
            channel->source.mcuNumber=(char)tmp;
            pvtGetChildValue(hVal,tmpId,__h245(terminalNumber), &(tmp),NULL);
            channel->source.terminalNumber=(char)tmp;

        }
        tmpId=pvtGetChild(hVal,h225ID,__h245(destination),NULL);
        if (tmpId>=0)
        {
            INT32 tmp;
            pvtGetChildValue(hVal,tmpId,__h245(mcuNumber), &(tmp),NULL);
            channel->destination.mcuNumber=(char)tmp;
            pvtGetChildValue(hVal,tmpId,__h245(terminalNumber), &(tmp),NULL);
            channel->destination.terminalNumber=(char)tmp;

        }
    }
    if (!channel->isDuplex)
        cmcCallAddressCallbacks(app, channel, h225ID, FALSE);
    if (!emaWasDeleted((EMAElement)channel))
    {
        if (channel->replacementCh && app->cmMyChannelEvent.cmEvChannelReplace)
        {
            cmiCBEnter((HAPP)app, "cmEvChannelReplace: haCall=0x%x, hsCall=0x%x, haReplaceCall=0x%x, hsReplaceCall=0x%x.",
                            emaGetApplicationHandle((EMAElement)channel),               channel,
                            emaGetApplicationHandle((EMAElement)channel->replacementCh),channel->replacementCh);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelReplace((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel),               (HCHAN)channel,
                                                     (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel->replacementCh),(HCHAN)channel->replacementCh);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelReplace.");

        }
    }

    if (!emaWasDeleted((EMAElement)channel))
    {
        /* -- Parameters */
        if (channel->dataTypeID <0)
        {
            logPrint(app->logERR, RV_ERROR,
                     (app->logERR, RV_ERROR, "Channel:establishIndication: no data type."));
            return RVERROR;
        }
        else
        {
            confDataType type;
            cmcCallChannelParametersCallback((HAPP)app, (HCHAN)channel, channel->dataTypeID, &type);
            if (!emaWasDeleted((EMAElement)channel))
                cmcCallDataTypeHandleCallback((HAPP)app, (HCHAN)channel, channel->dataTypeID, type);
        }
    }
    if (!emaWasDeleted((EMAElement)channel))
    {
        if (!channel->isDuplex)
        { /* -- dynamic RTP payload type */
            INT32 num;

            if((pvtGetChildValue(hVal,h225ID,__h245(dynamicRTPPayloadType),&num,NULL)>=0)&&
               (app->cmMyChannelEvent.cmEvChannelRTPDynamicPayloadType))
            {
                cmiCBEnter((HAPP)app, "cmEvChannelRTPDynamicPayloadType: haCall=0x%x, hsCall=0x%x, type=%d.", emaGetApplicationHandle((EMAElement)channel), channel, num);
                nesting = emaPrepareForCallback((EMAElement)channel);
                app->cmMyChannelEvent.cmEvChannelRTPDynamicPayloadType((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, (INT8)num);
                emaReturnFromCallback((EMAElement)channel, nesting);
                cmiCBExit((HAPP)app, "cmEvChannelRTPDynamicPayloadType.");
            }
        }
        channel->state=awaitingEstablishment;

        /* -- state changed */
        notifyChannelState(channel,cmChannelStateOffering,(channel->isDuplex)?cmChannelStateModeDuplex:cmChannelStateModeOff);
    }

    return TRUE;
}

int openLogicalChannel(controlElem*ctrl, int base, int message)
{
    INT32 lcn = 0;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    cmElem* app=(cmElem*)hApp;
    channelElem* channel = NULL;
    int nesting;
    int ret = TRUE;
    BOOL rejectChannel = FALSE;

    if (!hApp) return RVERROR;

    hVal = ((cmElem *)hApp)->hVal;

    if (ctrl->state == ctrlEndSession)
    {
        /* control was not even initialized (could only happen when tunneling is approved, and FastStart
        connected, and other side is working non-standard) or it has already been dropped. Reject the
        channel and exit.*/
        logPrint(app->logERR, RV_ERROR,
            (app->logERR, RV_ERROR, "Received OLC when control is not in connected state (haCall=0x%lx, hsCall=0x%lx)",
            emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl)));
        rejectChannel = TRUE;
    }
    else if ((ctrl->state != ctrlConnected) && (ctrl->state != ctrlConference))
    {
        /* received OLC when control is not connected yet (did not finish MSD and Cap-Ex), keep the
        message for later (if we have room), and exit */
        if (ctrl->incomingOLCs[0] < 0)
        {
            ctrl->incomingOLCs[0] = pvtAddRoot(hVal,NULL,0,NULL);
            pvtShiftTree(hVal,ctrl->incomingOLCs[0],base);
            return 0;
        }
        else if (ctrl->incomingOLCs[1] < 0)
        {
            ctrl->incomingOLCs[1] = pvtAddRoot(hVal,NULL,0,NULL);
            pvtShiftTree(hVal,ctrl->incomingOLCs[1],base);
            return 0;
        }
        rejectChannel = TRUE;
    }

    if (!rejectChannel)
    {
        hVal=cmGetValTree(hApp);
        deriveChannels((HCONTROL)ctrl);
        pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
        channel=allocateChannel((HCONTROL)ctrl);
        if (!channel)
        {
            logPrint(app->logERR, RV_ERROR,
                     (app->logERR, RV_ERROR, "Unable to allocate channel"));
            rejectChannel = TRUE;
        }
    }

    if (rejectChannel)
    {
        int rejMessage, nodeId;

        pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);

        rejMessage=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,rejMessage, __h245(response),__h245(openLogicalChannelReject));
        pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),lcn,NULL,NULL);
        pvtAddBranch2(hVal,nodeId,__h245(cause),__h245(unspecified));

        sendMessageH245((HCONTROL)ctrl, rejMessage);
        pvtDelete(hVal,rejMessage);

        return RVERROR;
    }

    if(emaLock((EMAElement)channel))
    {
        channel->ctrl=(HCONTROL)ctrl;
        channel->myLCN=lcn;
        channel->origin=FALSE;

        if (app->cmMySessionEvent.cmEvCallNewChannel)
        {
            cmiCBEnter(hApp, "cmEvCallNewChannel:IN: haCall=0x%lx, hsCall=0x%lx. hsChan=0x%lx", emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl),channel);
            {
                HAPPCHAN haChan=NULL;
                nesting = emaPrepareForCallback((EMAElement)channel);
                app->cmMySessionEvent.cmEvCallNewChannel((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),(HCHAN)channel, &haChan);
                emaReturnFromCallback((EMAElement)channel, nesting);
                emaSetApplicationHandle((EMAElement)channel,(void*)haChan);
            }
            cmiCBExit(hApp, "cmEvCallNewChannel:IN: haChan=0x%lx.", emaGetApplicationHandle((EMAElement)channel));
        }

        incomingChannelMessage(hApp, channel, base);

        if (!emaWasDeleted((EMAElement)channel))
            ret = openLogicalChannelEvent(app, hVal, channel, message);

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}


static
void establishConfirmed(channelElem*channel,int h225ID, int origin)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)channel);
    HPVT hVal=cmGetValTree(hApp);
    cmElem* app=(cmElem*)hApp;
    int nesting;

    cmTimerReset(hApp,&(channel->timer));
    if (!channel->isDuplex)
    {
        int tmpId;

        cmcCallAddressCallbacks(app, channel, h225ID, origin);

        tmpId=pvtGetChild(hVal,h225ID,__h245(transportCapability),NULL);
        if (tmpId>=0)
        {
            if (channel->transCapID < 0)
                channel->transCapID=pvtAddRoot(hVal,NULL,0,NULL);
            pvtMoveTree(hVal,channel->transCapID,tmpId);
        }
        else if (channel->transCapID >= 0)
        {
            pvtDelete(hVal, channel->transCapID);
            channel->transCapID = -1;
        }

        { /* -- dynamic RTP payload type */
            INT32 num;

            if((pvtGetChildValue(hVal,h225ID,__h245(dynamicRTPPayloadType),&num,NULL)>=0)&&(app->cmMyChannelEvent.cmEvChannelRTPDynamicPayloadType))
            {
                cmiCBEnter(hApp, "cmEvChannelRTPDynamicPayloadType: haCall=0x%x, hsCall=0x%x, type=%d.", emaGetApplicationHandle((EMAElement)channel), channel, num);
                nesting = emaPrepareForCallback((EMAElement)channel);
                app->cmMyChannelEvent.cmEvChannelRTPDynamicPayloadType((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, (INT8)num);
                emaReturnFromCallback((EMAElement)channel, nesting);
                cmiCBExit(hApp, "cmEvChannelRTPDynamicPayloadType.");
            }
        }
    }
    if (!emaWasDeleted((EMAElement)channel))
    { /* -- Parameters */
      /* Report the same session association handles */

        HCHAN hsSameSessionH=(HCHAN)channel->partner;
        HAPPCHAN haSameSessionH=NULL;

        if (channel->partner)
            haSameSessionH=(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel->partner);
        /* -- same session handle */
        cmiCBEnter(hApp, "cmEvChannelParameters: haChan=0x%p, hsChan=0x%p, channelName=NULL, AppSes=0x%p, Ses=0x%p, NULL, NULL, rate=-1.",
            emaGetApplicationHandle((EMAElement)channel), channel, haSameSessionH, hsSameSessionH);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE (app->cmMyChannelEvent.cmEvChannelParameters)
            ((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, NULL, haSameSessionH, hsSameSessionH, NULL, NULL, (UINT32)RVERROR);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelParameters.");
    }
    notifyChannelState(channel,cmChannelStateConnected, cmChannelStateModeOn);
    if (!emaWasDeleted((EMAElement)channel))
    if (channel->flowControlToZero)
    {
        cmiCBEnter(hApp, "cmEvChannelFlowControlToZero: haChan=0x%lx, hsChan=0x%lx", emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE (app->cmMyChannelEvent.cmEvChannelFlowControlToZero)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelFlowControlToZero.");

    }

}

int openLogicalChannelAck(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int forwardLCP_ID,reverseLCP_ID,h225ID;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);

    channel = getOutChanByLCN((HCONTROL)ctrl, lcn);
    if (!channel)
        return TRUE;

    if (emaLock((EMAElement)channel))
    {
        incomingChannelMessage((HAPP)app, channel, base);

        switch(channel->state)
        {
            case released:
            case channelIdle:
                logPrint(app->logERR, RV_INFO,
                         (app->logERR, RV_INFO, "Channel 0x%p in released state. Message discarded", channel));
                break;

            case awaitingEstablishment:
            {
                /* complete dependent channel establishment*/
                void* ptr=NULL;
                channelElem* dependent;
                while((dependent = getNextOutChanByBase((HCONTROL)ctrl,channel, &ptr)))
                    startEstablishment(app, dependent);
                if (!emaWasDeleted((EMAElement)channel))
                {
                    forwardLCP_ID=pvtGetChild(hVal,message, __h245(forwardMultiplexAckParameters), NULL);
                    h225ID=pvtGetChild(hVal,forwardLCP_ID, __h245(h2250LogicalChannelAckParameters), NULL);
                    if (h225ID>=0)
                    {
                        reverseLCP_ID=pvtGetChild(hVal,message, __h245(reverseLogicalChannelParameters),NULL);
                        pvtGetChildValue(hVal,reverseLCP_ID,__h245(portNumber),(INT32 *)&channel->remotePortNumber,NULL);

                        pvtGetChildValue(hVal,h225ID, __h245(sessionID),(INT32 *)&(channel->sid),NULL);

                        if (channel->isDuplex)
                            pvtGetChildValue(hVal,reverseLCP_ID, __h245(reverseLogicalChannelNumber),(INT32 *)&(channel->rvrsLCN),NULL);
                        else /* only for uni-directional channel. */
                        {
                            /* slave and new */
                            /* check database for partner lcn. In case of first audio or video */
                            if (!ctrl->isMaster && !channel->partner)
                                channel->partner = getInChanBySID((HCONTROL)ctrl,channel->sid);
                        }

                        /* -- get dynamic payload type */
                        pvtGetChildValue(hVal,h225ID,__h245(dynamicRTPPayloadType),(INT32 *)&channel->dynamicPayloadNumber,NULL);
                        pvtGetChildValue(hVal,h225ID,__h245(flowControlToZero),(INT32*)&channel->flowControlToZero,NULL);
                    }

                    if (channel->isDuplex)
                    {
                        /* Update separateStack information of this channel if necessary */
                        int tmpId, seperateStackNode;
                        __pvtGetNodeIdByFieldIds(seperateStackNode, hVal, message, {_h245(separateStack) LAST_TOKEN});

                        __pvtGetNodeIdByFieldIds(tmpId, hVal, seperateStackNode, {_h245(networkAddress) _h245(localAreaAddress) LAST_TOKEN});
                        if (tmpId >= 0)
                        {
                            if (channel->separateStackID < 0)
                                channel->separateStackID = pvtAddRoot(hVal, NULL, 0, NULL);
                            pvtMoveTree(hVal, channel->separateStackID, tmpId);
                        }
                        else if (channel->separateStackID >= 0)
                        {
                            pvtDelete(hVal, channel->separateStackID);
                            channel->separateStackID = -1;
                        }

                        if(pvtGetChild(hVal, seperateStackNode, __h245(associateConference), &tmpId) >= 0)
                            pvtGet(hVal, tmpId, NULL, NULL, (INT32*)&channel->isAssociated, NULL);
                        else channel->isAssociated = FALSE;

                        if(pvtGetChild(hVal, seperateStackNode, __h245(externalReference), &tmpId) >= 0)
                            channel->externalReferenceLength = pvtGetString(hVal, tmpId, 256, channel->externalReference);
                        else
                        {
                            channel->externalReferenceLength = 0;
                            channel->externalReference[0] = 0;
                        }
                    }

                    channel->state=established;
                    establishConfirmed(channel,h225ID,FALSE);
                }
                if (!emaWasDeleted((EMAElement)channel))
                {
                    ctrl->conflictChannels--;

                    if (channel->isDuplex)
                    { /* send confirm message. */
                        int message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
                        int nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(openLogicalChannelConfirm));
                        pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);

                        sendMessageH245Chan((HCONTROL)ctrl, (HCHAN)channel, message);
                        pvtDelete(hVal,message);
                    }
                }
                break;
            }

            default:
                logPrint(app->logERR, RV_INFO,
                         (app->logERR, RV_INFO, "Channel 0x%p in %d state. Message discarded", channel, channel->state));
                break;
        }

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}

int openLogicalChannelReject(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    cmChannelStateMode_e stateMode = cmChannelStateModeDisconnectedRemote;
    int rejectReason;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);
    if (!channel) return TRUE;

    if (pvtGetChildValue(hVal,message,__h245(cause),&rejectReason,NULL) >= 0)
    {
        if (rejectReason == __h245(masterSlaveConflict))
            stateMode = cmChannelStateModeDisconnectedMasterSlaveConflict;
    }

    if(emaLock((EMAElement)channel))
    {
        cmTimerReset((HAPP)app,&(channel->timer));

        incomingChannelMessage((HAPP)app, channel, base);

        if (channel->state==awaitingEstablishment)
        {
            ctrl->conflictChannels--;
            {
                void* ptr=NULL;
                channelElem* dependent;
                while((dependent=getNextOutChanByBase((HCONTROL)ctrl,channel,&ptr)))
                {/* release the dependent channels because the base is rejected */
                    dependent->state=released;
                    notifyChannelState(dependent,cmChannelStateDisconnected,stateMode);
                    notifyChannelState(dependent,cmChannelStateIdle, cmChannelStateModeOff);
                }
            }
        }
        channel->state=released;
        notifyChannelState(channel,cmChannelStateDisconnected,stateMode);
        notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}
int openLogicalChannelConfirm(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int forwardLCP_ID,h225ID;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getInChanByLCN((HCONTROL)ctrl,lcn);

    if (!channel) return TRUE;
    {
        if(emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            switch(channel->state)
            {
            case awaitingEstablishment:
                channel->state=released;
                notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedReasonUnknown);
                notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);

                break;
            case awaitingConfirmation:
                forwardLCP_ID=pvtGetChild(hVal,message, __h245(forwardLogicalChannelParameters), NULL);
                h225ID=pvtGetChild(hVal,forwardLCP_ID, __h245(h2250LogicalChannelAckParameters), NULL);
                channel->state=established;
                establishConfirmed(channel,h225ID,FALSE);
                break;
            default:
                break;
            }
            emaUnlock((EMAElement)channel);
        }
    }
    return TRUE;
}


int closeLogicalChannel(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INTPTR reason;
    cmChannelStateMode_e stateMode=cmChannelStateModeDisconnectedReasonUnknown;
    int rmessage, nodeId, res = RVERROR;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getInChanByLCN((HCONTROL)ctrl,lcn);

    if (!channel) return TRUE;

    if(emaLock((EMAElement)channel))
    {
        incomingChannelMessage((HAPP)app, channel, base);

        rmessage=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,rmessage, __h245(response),__h245(closeLogicalChannelAck));
        pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
        res = sendMessageH245Chan((HCONTROL)ctrl, (HCHAN)channel, rmessage);
        pvtDelete(hVal,rmessage);
        pvtGet(hVal,pvtChild(hVal,pvtGetChild(hVal,message,__h245(reason),NULL)),&reason,NULL,NULL,NULL);

        switch(reason)
        {
        case __h245(unknown)              :stateMode =cmChannelStateModeDisconnectedReasonUnknown;            break;
        case __h245(reopen)               :stateMode =cmChannelStateModeDisconnectedReasonReopen;             break;
        case __h245(reservationFailure)   :stateMode =cmChannelStateModeDisconnectedReasonReservationFailure; break;
        }
        channel->state=released;
        notifyChannelState(channel,cmChannelStateDisconnected,stateMode);
        notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
    }
    emaUnlock((EMAElement)channel);

    return res;
}


int closeLogicalChannelAck(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);
    if (!channel) return TRUE;

    if(emaLock((EMAElement)channel))
    {
        cmTimerReset((HAPP)app,&(channel->timer));

        incomingChannelMessage((HAPP)app, channel, base);

        switch(channel->state)
        {
            case established:
            case awaitingRelease:
                channel->state = released;
                notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            break;
            default:
                break;
        }
        channel->state=released;

        emaUnlock((EMAElement)channel);
    }

    return TRUE;
}



int requestChannelClose(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    cmElem* app=(cmElem*)hApp;
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;
    hVal=cmGetValTree(hApp);

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);
    if (!channel) return TRUE;
    {
        if(emaLock((EMAElement)channel))
        {
            incomingChannelMessage(hApp, channel, base);

            channel->rc_paramID=message;
            cmiCBEnter(hApp, "cmEvChannelRequestCloseStatus: haChan=0x%lx, hsChan=0x%lx, status=request.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            ifE(app->cmMyChannelEvent.cmEvChannelRequestCloseStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmRequestCloseRequest);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit(hApp, "cmEvChannelRequestCloseStatus.");
            channel->rc_paramID=RVERROR;
            emaUnlock((EMAElement)channel);
        }
    }
    return TRUE;
}

int requestChannelCloseAck(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    cmElem* app=(cmElem*)hApp;
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getInChanByLCN((HCONTROL)ctrl,lcn);
    if (!channel) return TRUE;

    if(emaLock((EMAElement)channel))
    {
        cmTimerReset(hApp,&(channel->rc_timer));

        incomingChannelMessage(hApp, channel, base);

        cmiCBEnter(hApp, "cmEvChannelRequestCloseStatus: haChan=0x%lx, hsChan=0x%lx, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE(app->cmMyChannelEvent.cmEvChannelRequestCloseStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmRequestCloseConfirm);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelRequestCloseStatus.");

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}

int requestChannelCloseReject(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    cmElem* app=(cmElem*)hApp;
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getInChanByLCN((HCONTROL)ctrl,lcn);

    if (!channel) return TRUE;

    if(emaLock((EMAElement)channel))
    {
        cmTimerReset(hApp,&(channel->rc_timer));

        incomingChannelMessage(hApp, channel, base);

        cmiCBEnter(hApp, "cmEvChannelRequestCloseStatus: haChan=0x%lx, hsChan=0x%lx, status=reject.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE(app->cmMyChannelEvent.cmEvChannelRequestCloseStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmRequestCloseReject);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelRequestCloseStatus.");

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}

int requestChannelCloseRelease(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    cmElem* app=(cmElem*)hApp;
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;
    hVal=app->hVal;

    pvtGetChildValue(hVal,message,__h245(forwardLogicalChannelNumber),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

    if (!channel) return TRUE;

    if(emaLock((EMAElement)channel))
    {
        cmTimerReset(hApp,&(channel->rc_timer));

        incomingChannelMessage(hApp, channel, base);

        cmiCBEnter(hApp, "cmEvChannelRequestCloseStatus: haChan=0x%lx, hsChan=0x%lx, status=reject.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE(app->cmMyChannelEvent.cmEvChannelRequestCloseStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmRequestCloseReject);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelRequestCloseStatus.");

        emaUnlock((EMAElement)channel);
    }
    return TRUE;
}

/*========================================A P I ====================================================================*/


RVAPI int RVCALLCONV
cmChannelOpen(
          IN    HCHAN       hsChan,
          IN    char*       channelName,
          IN    HCHAN       hsChanSameSession,
          IN    HCHAN       hsChanAssociated,
          IN    UINT32      rate)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem* channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);

    BOOL isDynamic=FALSE;
    int nodeId=-1, ret=-1;

    if (!channelName || !channel || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelOpen: hsChan=0x%lx, channelName=%s, hsChanSameSession=0x%lx, hsChanAssociated=0x%lx, rate=%d.",
                hsChan, nprn(channelName), hsChanSameSession, hsChanAssociated, rate);

    nodeId = pvtAddRoot(hVal, ((cmElem*)hApp)->h245DataType, 0, NULL);
    if (nodeId<0)
    {
        cmiAPIExit(hApp, "cmChannelOpen: return %d",nodeId);
        return nodeId;
    }

    if (emaLock((EMAElement)hsChan))
    {
        confGetDataType(hVal, cmGetH245ConfigurationHandle(hApp), channelName, nodeId, &isDynamic, FALSE);
        ret = cmChannelOpenDynamic(hsChan, pvtChild(hVal, nodeId), hsChanSameSession, hsChanAssociated, isDynamic);
        pvtDelete(hVal, nodeId);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelOpen: [%d].", ret);
    return ret;
}


RVAPI int RVCALLCONV
cmChannelSetHandle(
           /* let haChan be the new application handle of this channel */
           IN HCHAN                    hsChan,
           IN HAPPCHAN                 haChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem* channel=(channelElem*)hsChan;

    if (!channel || !hApp)   return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetHandle: hsChan=0x%lx, haChan=0x%lx.", hsChan, haChan);
    emaSetApplicationHandle((EMAElement)channel,(void*)haChan);
    cmiAPIExit(hApp, "cmChannelSetHandle: [%d].", 0);
    return 0;
}


RVAPI int RVCALLCONV
cmChannelAnswer(
        /*
           - Acknowledge incoming channel request.
           - Acknowledge outgoing channel close request.
        */
        IN  HCHAN       hsChan
        )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    HPVT hVal=cmGetValTree(hApp);
    cmElem*app=(cmElem*)hApp;
    int message, olcaID, reverseLCN_ID, h225ID, nodeId, res = RVERROR;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelAnswer: hsChan=0x%lx.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        HCONTROL ctrl;
        controlElem* ctrlE;
        channelElem* channel=(channelElem*)hsChan;
        ctrl = channel->ctrl;
        ctrlE = (controlElem*)ctrl;
        if (channel->state==established && channel->origin)
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(requestChannelCloseAck) _h245(forwardLogicalChannelNumber) LAST_TOKEN},
                                 channel->myLCN,NULL);
            res = sendMessageH245Chan((HCONTROL)ctrl, hsChan, message);
            pvtDelete(hVal,message);
        }
        else if (!channel->origin)
        {
            if (channel->state==awaitingEstablishment)
            {
                message = pvtAddRoot(hVal, app->synProtH245, 0, NULL);

                olcaID=pvtAddBranch2(hVal,message, __h245(response), __h245(openLogicalChannelAck));
                h225ID=pvtAddBranch2(hVal,olcaID, __h245(forwardMultiplexAckParameters), __h245(h2250LogicalChannelAckParameters));
                {
                  /* Link partner rtcp address */
                    if (channel->partner && channel->partner->recvRtcpAddressID>=0)
                        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaControlChannel)),hVal,channel->partner->recvRtcpAddressID);
                    else if (channel->recvRtcpAddressID>=0)
                        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaControlChannel)),hVal,channel->recvRtcpAddressID);

                    if (channel->recvRtpAddressID>=0)
                        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(mediaChannel)),hVal,channel->recvRtpAddressID);

                    if (channel->transCapID>=0)
                        pvtSetTree(hVal,pvtAddBranch(hVal,h225ID,__h245(transportCapability)),hVal,channel->transCapID);

                    if (channel->port>0 && channel->port<65536)
                        pvtAdd(hVal,h225ID,__h245(portNumber),channel->port,NULL,NULL);

                    pvtAdd(hVal,h225ID,__h245(sessionID), channel->sid,NULL,NULL);
                }
                /* -- dynamic payload */
                if (channel->dynamicPayloadNumber>=0)
                    pvtAdd(hVal,h225ID,__h245(dynamicRTPPayloadType) ,channel->dynamicPayloadNumber ,NULL,NULL);
                /* -- flow control to 0 */
                if (channel->flowControlToZero)
                    pvtAdd(hVal,h225ID,__h245(flowControlToZero),channel->flowControlToZero,NULL,NULL);
                if (channel->isDuplex)
                { /* (for duplex channels). Declare reverse channel with lcn. */
                    ctrlE->lcnOut++;
                    channel->rvrsLCN=ctrlE->lcnOut;
                    reverseLCN_ID=pvtAddBranch(hVal,olcaID, __h245(reverseLogicalChannelParameters));
                    pvtAdd(hVal,reverseLCN_ID, __h245(reverseLogicalChannelNumber), channel->rvrsLCN,NULL,NULL);

                    if (channel->replacementCh && channel->replacementCh->rvrsLCN)
                      pvtAdd(hVal,reverseLCN_ID, __h245(replacementFor), channel->replacementCh->rvrsLCN, NULL,NULL);
                    if (channel->separateStackID>=0)
                    {
                        int separateStack;
                        if (channel->redEncID<0)
                            channel->redEncID=pvtAddRoot(hVal,NULL,0,NULL);
                        separateStack=pvtAddBranch(hVal,olcaID,__h245(separateStack));
                        pvtMoveTree(hVal,pvtAddBranch2(hVal,separateStack,__h245(networkAddress), __h245(localAreaAddress)), channel->separateStackID);
                        channel->separateStackID=RVERROR;
                        if (channel->externalReferenceLength>0)
                            pvtAdd(hVal,separateStack,__h245(externalReference),channel->externalReferenceLength,channel->externalReference,NULL);
                        pvtAdd(hVal,separateStack,__h245(associateConference),channel->isAssociated,NULL,NULL);
                        if ((channel->t120SetupProcedure >= cmOriginateCall) &&
                            (channel->t120SetupProcedure <= cmIssueQuery))
                        {
                            int t120s[]={__h245(originateCall),__h245(waitForCall),__h245(issueQuery)};
                            pvtAddBranch2(hVal,separateStack,__h245(t120SetupProcedure),t120s[channel->t120SetupProcedure-cmOriginateCall]);
                        }

                    }
                }
                pvtAdd(hVal,olcaID, __h245(forwardLogicalChannelNumber), channel->myLCN,NULL,NULL);
                res = sendMessageH245Chan((HCONTROL)ctrl, hsChan, message);
                pvtDelete(hVal,message);

                if (res >= 0)
                {
                    if (channel->isDuplex)
                    { /* B-LCSE */
                        INT32 timeout=29;
                        pvtGetChildValue(hVal,app->h245Conf,__h245(channelsTimeout),&(timeout),NULL);
                        cmTimerReset(hApp,&(channel->timer));
                        channel->timer=cmTimerSet(hApp,channelTimeoutEventsHandler,(void*)channel,timeout*1000);
                        channel->state=awaitingConfirmation;
                    }
                    else  /* LCSE */
                        channel->state=established;
                }
            }

            if (!channel->origin && !channel->isDuplex && (res >= 0))/* simplex connection */
                /*  ==> connected */
                notifyChannelState(channel,cmChannelStateConnected, cmChannelStateModeOn);
        }
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelAnswer=%d", res);
    return res;
}


RVAPI int RVCALLCONV
cmChannelDrop(
          /*
        - Close outgoing channel
        - Reject incoming channel open request
        - New: Request to close incoming channel
          */
          IN    HCHAN       hsChan
          )
{
    return cmChannelDropReason(hsChan,cmCloseReasonUnknown);
}


RVAPI int RVCALLCONV
cmChannelClose(
           IN   HCHAN       hsChan)
{
    cmElem* app=(cmElem *)emaGetInstance((EMAElement)hsChan);
    if (!app)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelClose: hsChan=0x%lx.", hsChan);

    if (app->mibEvent.h341DeleteLogicalChannel)
          app->mibEvent.h341DeleteLogicalChannel(app->mibHandle,hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        channelElem* channel=(channelElem*)hsChan;
        controlElem* ctrl;

        ctrl = (controlElem *)channel->ctrl;
        {
            channelElem* someChannel;
            void* ptr=NULL;

            /* Make sure nobody will remember us after death */
            while((someChannel=getNextChan((HCONTROL)ctrl, &ptr)))
            {
                if(emaLock((EMAElement)someChannel))
                {
                    if (someChannel->associated==channel)       someChannel->associated=NULL;
                    if (someChannel->partner==channel)          someChannel->partner=NULL;
                    if (someChannel->base==channel)             someChannel->base=NULL;
                    if (someChannel->replacementCh==channel)    someChannel->replacementCh=NULL;
                    emaUnlock((EMAElement)someChannel);
                }
            }

        }
        
        cmTimerReset((HAPP)app, &(channel->timer));
        cmTimerReset((HAPP)app, &(channel->rc_timer));
        channelFreeMemory(hsChan);

        emaDelete((EMAElement)hsChan);
        deleteChannelForCtrl((HCONTROL)ctrl,hsChan);
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelClose: [1].");
    return TRUE;
}





RVAPI int RVCALLCONV
cmChannelSetAddress(
            IN  HCHAN       hsChan,
            IN  UINT32      ip,
            IN  UINT16      port)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    HPVT hVal=cmGetValTree(hApp);
    cmTransportAddress ta;

    if (!hApp)
        return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetAddress: hsChan=0x%p, ip=0x%x, port=%d.", hsChan, ip, port);

    if (emaLock((EMAElement)hsChan))
    {
        cmElem* app=(cmElem*)hApp;
        channelElem* channel=(channelElem*)hsChan;
        callElem * call = (callElem *) cmiGetByControl(channel->ctrl);

        /* Create a new root for the RTP address if necessary */
        if (channel->recvRtpAddressID < 0)
            channel->recvRtpAddressID = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);

        /* Create the address structure */
        ta.type = cmTransportTypeIP;
        ta.distribution = cmDistributionUnicast;
        ta.ip = ip;
        ta.port = port;
        ta.length = 0;
        getGoodAddressForCtrl(channel->ctrl, &ta);

        /* Update the PVT of the RTP address in the channel element */
        cmTAToVt_H245(hVal, channel->recvRtpAddressID, &ta);

        /* Check to see if we are in the middle of answering a FS proposal */
        if( (call->fastStartState == fssAck) && (channel->fastStartChannelIndex >= 0) &&
            ((call->state == cmCallStateInit) || (call->state == cmCallStateOffering)) )
        {
            /* now set it in the OLC */
            int tmpNode=pvtGetChild(hVal, call->fastStartNodesAck[channel->fastStartChannelIndex], __h245(reverseLogicalChannelParameters),NULL);
            if (tmpNode < 0)
                tmpNode=pvtGetChild(hVal, call->fastStartNodesAck[channel->fastStartChannelIndex], __h245(forwardLogicalChannelParameters),NULL);

            __pvtBuildByFieldIds(tmpNode, hVal, tmpNode, {_h245(multiplexParameters) _h245(h2250LogicalChannelParameters) _h245(mediaChannel) LAST_TOKEN}, 0, NULL );
            if (ta.ip || ta.port)
                cmTAToVt_H245(hVal, tmpNode, &ta);
       }

        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetAddress: [1].");
    return TRUE;
}

RVAPI int RVCALLCONV
cmChannelSetRTCPAddress(
            IN  HCHAN       hsChan,
            IN  UINT32      ip,
            IN  UINT16      port)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    HPVT hVal=cmGetValTree(hApp);
    cmTransportAddress ta;
    int ret = RVERROR;

    if (!hApp)
        return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetRTCPAddress: hsChan=0x%p, ip=0x%x, port=%d.", hsChan, ip, port);

    if (emaLock((EMAElement)hsChan))
    {
        cmElem* app=(cmElem*)hApp;
        channelElem* channel=(channelElem*)hsChan;
        callElem * call = (callElem *) cmiGetByControl(channel->ctrl);

        if ((channel->partner == NULL) || (channel->partner->recvRtcpAddressID < 0))
        {
            /* Create a new root for the RTCP address if necessary */
            if (channel->recvRtcpAddressID < 0)
                channel->recvRtcpAddressID = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);

            /* Create the address structure */
            ta.type = cmTransportTypeIP;
            ta.distribution = cmDistributionUnicast;
            ta.ip = ip;
            ta.port = port;
            ta.length = 0;
            getGoodAddressForCtrl(channel->ctrl, &ta);

            /* Update the PVT of the RTP address */
            ret = cmTAToVt_H245(hVal, channel->recvRtcpAddressID, &ta);
        }
        else
        {
            int tmpNode = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);

            /* Create the address structure */
            ta.type = cmTransportTypeIP;
            ta.distribution = cmDistributionUnicast;
            ta.ip = ip;
            ta.port = port;
            ta.length = 0;
            getGoodAddressForCtrl(channel->ctrl, &ta);

            /* Update the PVT of the RTP address */
            ret = cmTAToVt_H245(hVal, tmpNode, &ta);

            if(ret >= 0)
                ret = pvtCompareTree(hVal, channel->partner->recvRtcpAddressID, hVal, tmpNode);

            pvtDelete(hVal, tmpNode);
        }

        /* Check to see if we are in the middle of answering a FS proposal */
        if( (call->fastStartState == fssAck) && (channel->fastStartChannelIndex >= 0) &&
            ((call->state == cmCallStateOffering) || (call->state == cmCallStateInit)) )
        {
            /* now set it in the OLC */
            int tmpNode=pvtGetChild(hVal, call->fastStartNodesAck[channel->fastStartChannelIndex], __h245(reverseLogicalChannelParameters),NULL);
            if (tmpNode < 0)
                tmpNode=pvtGetChild(hVal, call->fastStartNodesAck[channel->fastStartChannelIndex], __h245(forwardLogicalChannelParameters),NULL);

            __pvtBuildByFieldIds(tmpNode, hVal, tmpNode, {_h245(multiplexParameters) _h245(h2250LogicalChannelParameters) _h245(mediaControlChannel) LAST_TOKEN}, 0, NULL );
            if (ta.ip || ta.port)
                cmTAToVt_H245(hVal, tmpNode, &ta);
        }

        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetRTCPAddress: [%d].", ret);
    return ret;
}


RVAPI int RVCALLCONV
cmChannelGetOrigin(
           IN   HCHAN       hsChan,
           OUT  BOOL*       origin)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    int ori=RVERROR;

    if (!channel)return RVERROR;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetOrigin: hsChan=0x%lx, origin=0x%lx.", hsChan, origin);
    if (emaLock((EMAElement)hsChan))
    {
        ori=channel->origin;
        emaUnlock((EMAElement)hsChan);
    }
    if (origin)
      *origin=ori;
    cmiAPIExit(hApp, "cmChannelGetOrigin: origin=%d. [1].", nprnd(origin));
    return ori;
}


RVAPI int RVCALLCONV
cmChannelSetDynamicRTPPayloadType(
                  IN    HCHAN       hsChan,
                  IN    INT8        dynamicPayloadType)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetDynamicRTPPayloadType: hsChan=0x%lx, dynamicPayloadType=%d.",
          hsChan, dynamicPayloadType);

    if (emaLock((EMAElement)hsChan))
    {
        if (dynamicPayloadType <96)
        {
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelSetDynamicRTPPayloadType: [Invalid Parameter].");
            return RVERROR;
        }
        channel->dynamicPayloadNumber=dynamicPayloadType;
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetDynamicRTPPayloadType: [1].");
    return TRUE;
}


RVAPI int RVCALLCONV
cmChannelSameSession(
             /* get the same session opposite channel of hsChan */
             IN     HCHAN           hsChan,
             OUT    HAPPCHAN*       haSameSession,
             OUT    HCHAN*          hsSameSession
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSameSession: hsChan=0x%p, haSameSession=0x%p, hsSameSession=0x%p.", hsChan, haSameSession, hsSameSession);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->partner)
        {
            if (hsSameSession)  *hsSameSession=(HCHAN)channel->partner;
            if (haSameSession && hsSameSession && *hsSameSession)  *haSameSession=(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel->partner);
        }
        else
        {
            if (haSameSession) *haSameSession=NULL;
            if (hsSameSession) *hsSameSession=NULL;
        }
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSameSession: ha=0x%p hs=0x%p [0].", (haSameSession)?*haSameSession:0, (hsSameSession)?*hsSameSession:0);
    return 0;
}


RVAPI int RVCALLCONV /* returns the session id for this channel */
cmChannelSessionId(
           /* get the session id of channel */
           IN   HCHAN           hsChan
           )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    int sid=RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSessionId: hsChan=0x%lx.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        sid = channel->sid;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSessionId: [%d].", sid);
    return sid;
}

/* -- Duplex channels -- */

RVAPI
int RVCALLCONV cmChannelDuplex(
                 /* declare channel as duplex. Meaning full duplex. Composed of
                two uni-directional channels in opposite directions. */
        IN      HCHAN               hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelDuplex: hsChan=0x%lx.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        channel->isDuplex=1;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelDuplex: [1].");
    return TRUE;
}

RVAPI
int RVCALLCONV cmChannelIsDuplex(
                   /* Returns TRUE if channel is duplex. FALSE if not. and negative value
                  in case of error */
         IN     HCHAN               hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    int isDuplex=RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelIsDuplex: hsChan=0x%lx.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        isDuplex = channel->isDuplex;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelIsDuplex: [%d].", isDuplex);

  return isDuplex;
}


RVAPI
int RVCALLCONV cmChannelSetDuplexAddress(
                       /* Set address of duplex connection */
         IN     HCHAN               hsChan,
         IN     cmTransportAddress  address,
         IN     int                 externalReferenceLength,
         IN     char*               externalReference, /* NULL if not exists */
         IN     BOOL                isAssociated /* TRUE if associated */
         )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    HPVT hVal;

    if (!hApp || !hsChan) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetDuplexAddress: hsChan=0x%lx, ip=0x%lx, port=%d, %s, ext=%d '%s' %s.",
          hsChan, address.ip, address.port,
          (address.distribution==cmDistributionUnicast)?"unicast":"multicast",
          externalReferenceLength, (externalReference==NULL)?"(null)":externalReference,
          (isAssociated==TRUE)?"Associated":"Not Associated");
    if (emaLock((EMAElement)hsChan))
    {
        cmElem* app=(cmElem*)hApp;
        channelElem* channel=(channelElem*)hsChan;
        hVal = cmGetValTree(hApp);

        /* check if we were given an address */
        if ((address.ip == 0) && (address.port == 0))
        {
            /* address was zero - delete the information */
            if (channel->separateStackID >= 0)
                pvtDelete(hVal, channel->separateStackID);
            channel->separateStackID = RVERROR;
            channel->externalReferenceLength = 0;
            channel->externalReference[0] = '\0';
            channel->isAssociated = FALSE;
        }
        else
        {
            /* address was given, update information */
            if (channel->separateStackID < 0)
                channel->separateStackID=pvtAddRoot(hVal,app->hAddrSynH245,0,NULL);
            getGoodAddressForCtrl(channel->ctrl,&address);
            cmTAToVt_H245(hVal,channel->separateStackID,&address);
            channel->externalReferenceLength=min(externalReferenceLength,255);
            memcpy(channel->externalReference,externalReference,channel->externalReferenceLength);
            channel->isAssociated=isAssociated;
        }
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSetDuplexAddress: [1].");
    return TRUE;
}


RVAPI
int /* actual size of external reference or RVERROR */
RVCALLCONV cmChannelGetDuplexAddress(
           /* Get address of duplex connection */
           IN     HCHAN               hsChan,
           OUT    cmTransportAddress* address, /* User allocated structure */
           IN     int                 externalReferenceLength, /* size of extRef buffer */
           OUT    char*               externalReference, /* User allocated buffer */
           OUT    BOOL*               isAssociated /* TRUE if associated */
                   )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetDuplexAddress: hsChan=0x%p, address=0x%p, ext=%d:0x%lx, isAssociated=0x%lx",
          hsChan, address, externalReferenceLength, externalReference, isAssociated);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->separateStackID <0)
        {
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelGetDuplexAddress: Error: No separate stack id.");
            return RVERROR;
        }

        if (address)
            cmVtToTA_H245(hVal, channel->separateStackID, address);

        if( (externalReference) && (externalReferenceLength > channel->externalReferenceLength) )
            memcpy(externalReference, channel->externalReference, channel->externalReferenceLength+1);

        if (isAssociated)
            *isAssociated= channel->isAssociated;

        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetDuplexAddress: ip=0x%x, port=%d, %s, ext=%d '%s' %s.",
         (address)?address->ip:0, (address)?address->port:0,
         (address)?((address->distribution==cmDistributionUnicast)?"unicast":"multicast"):("N/A"),
         0, (externalReference==NULL)?"(null)":externalReference,
         (isAssociated)?((*isAssociated==TRUE)?"Associated":"Not Associated"):"N/A");
    return 0;
}







RVAPI int RVCALLCONV
cmChannelGetDependency(
             IN     HCHAN           hsChan,
             OUT    HAPPCHAN*       haBaseChannel,
             OUT    HCHAN*          hsBaseChannel
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetDependency: hsChan=0x%p, haBaseChannel=0x%p, hsBaseChannel=0x%p.", hsChan, haBaseChannel, hsBaseChannel);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->base)
        {
            if (hsBaseChannel)  *hsBaseChannel=(HCHAN)channel->base;
            if (haBaseChannel && hsBaseChannel && *hsBaseChannel)  *haBaseChannel=(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel->base);
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelGetDependency: ha=0x%p hs=0x%p [%d].", (haBaseChannel)?*haBaseChannel:0, (hsBaseChannel)?*hsBaseChannel:0, 0);
            return 0;
        }
        else
        {
            if (haBaseChannel) *haBaseChannel=NULL;
            if (hsBaseChannel) *hsBaseChannel=NULL;
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelGetDependency: channel is independent");
            return 0;
        }
    }
    cmiAPIExit(hApp, "cmChannelGetDependency: channel does not exist");
    return RVERROR;
}

RVAPI int RVCALLCONV
cmIsChannelReplace(
             IN     HCHAN           hsChan,
             OUT    HAPPCHAN*       haReplaceChannel,
             OUT    HCHAN*          hsReplaceChannel
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelReplace: hsChan=0x%p, haReplaceChannel=0x%p, hsReplaceChannel=0x%p.", hsChan, haReplaceChannel, hsReplaceChannel);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->replacementCh)
        {
            if (hsReplaceChannel)  *hsReplaceChannel=(HCHAN)channel->replacementCh;
            if (haReplaceChannel && hsReplaceChannel && *hsReplaceChannel)  *haReplaceChannel=(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel->replacementCh);
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelReplace: ha=0x%p hs=0x%p [%d].", (haReplaceChannel)?*haReplaceChannel:0, (hsReplaceChannel)?*hsReplaceChannel:0, 0);
            return 0;
        }
        else
        {
            if (haReplaceChannel) *haReplaceChannel=NULL;
            if (hsReplaceChannel) *hsReplaceChannel=NULL;
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelReplace: ");
            return 0;
        }
    }
    cmiAPIExit(hApp, "cmChannelReplace: channel does not exist");
    return RVERROR;
}




/*
  add to events to out open log channels proc
  --closeBaseIndication when base channel have got releaserequest .
  -- noBaseChannel when base channel is not exist or have ot releaseRequest or timeout before
    establish proc start.
*/

RVAPI int RVCALLCONV
cmChannelSetDependency(
        IN      HCHAN               hsChan,
        IN      HCHAN               hsChanBase     )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetDependency: hsChan=0x%lx, hsChanBase=0x%lx", hsChan, hsChanBase);
    if (emaLock((EMAElement)hsChan))
    {
        channel->base=(channelElem*)hsChanBase;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSetDependency: [1].");
    return TRUE;
}


RVAPI int RVCALLCONV
cmChannelReplace(
         IN      HCHAN               hsChan,
         IN      HCHAN               hsChanReplace     )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelReplace: hsChan=0x%lx, hsChanReplace=0x%lx", hsChan, hsChanReplace);
    if (emaLock((EMAElement)hsChan))
    {
        channel->replacementCh=(channelElem*)hsChanReplace;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelReplace: [1].");
    return TRUE;
}



RVAPI int RVCALLCONV
cmChannelSetT120Setup(
         IN      HCHAN               hsChan,
         IN      cmT120SetupProcedure t120SetupProc)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetT120Setup: hsChan=0x%lx, cmT120SetupProcedure=%d", hsChan, t120SetupProc);

    if (emaLock((EMAElement)hsChan))
    {
        if (t120SetupProc>=cmOriginateCall && t120SetupProc<=cmIssueQuery)
        {
            channel->t120SetupProcedure = t120SetupProc;
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelSetT120Setup: [1].");
            return TRUE;
        }
        else
        {
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit(hApp, "cmChannelSetT120Setup: invlid parameter ");
            return RVERROR;
        }
    }
    cmiAPIExit(hApp, "cmChannelSetT120Setup: channel does not exist.");
    return RVERROR;
}


RVAPI int RVCALLCONV
cmChannelGetT120Setup(
             IN     HCHAN                   hsChan,
             OUT    cmT120SetupProcedure*   t120SetupProc)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    cmT120SetupProcedure t120sp=(cmT120SetupProcedure)RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetT120Setup: hsChan=0x%lx ", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        t120sp = channel->t120SetupProcedure;
        emaUnlock((EMAElement)hsChan);
    }
    if (t120SetupProc)
        *t120SetupProc=t120sp;
    cmiAPIExit(hApp, "cmChannelGetT120Setup: [%d].", t120sp);
    return t120sp;
}

RVAPI int RVCALLCONV
cmChannelSetFlowControlToZero(
         IN      HCHAN               hsChan,
         IN      BOOL   flowControl)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetFlowControlToZero: hsChan=0x%lx, flowControl=%d", hsChan, flowControl);
    if (emaLock((EMAElement)hsChan))
    {
        channel->flowControlToZero=flowControl;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSetFlowControlToZero: [1].");
    return TRUE;
}


RVAPI int RVCALLCONV
cmChannelSetTransportCapability(
             IN HCHAN   hsChan,
             IN int     transportCapId    )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    cmElem* app=(cmElem*)hApp;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetTransportCapability: hsChan=0x%lx transpCapId=[%d] ", hsChan,transportCapId);
    if (emaLock((EMAElement)hsChan))
    {
        if (channel->transCapID<0)
            channel->transCapID = pvtAddRoot(hVal, app->h245TransCap, 0, NULL);
        pvtMoveTree(hVal,channel->transCapID,transportCapId);
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSetTransportCapability: [1].");
    return TRUE;

}



RVAPI int RVCALLCONV
cmChannelGetTransportCapabilityId(
             IN      HCHAN               hsChan)
{

    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    int nodeId=RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetTransportCapabilityId: hsChan=0x%lx ", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        nodeId = channel->transCapID;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetTransportCapabilityId: [%d].",nodeId);
    return nodeId;
}







RVAPI int RVCALLCONV
cmChannelSetRedundancyEncoding(
         IN     HCHAN               hsChan,
         IN     cmRedundancyEncoding * redundancyEncoding)

{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int encMethodId,secEncodingId;
    cmElem* app=(cmElem*)hApp;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetRedundancyEncoding: hsChan=0x%lx redEncMethodId=[%d] dataTypeHandle = [%d]",
            hsChan,redundancyEncoding->redundancyEncodingMethodId,redundancyEncoding->secondaryEncodingId);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->redEncID<0)
            channel->redEncID = pvtAddRoot(hVal, app->h245TransCap, 0, NULL);
        encMethodId = pvtAdd(hVal, channel->redEncID, __h245(redundancyEncodingMethod),0, NULL, NULL);
        pvtMoveTree(hVal,encMethodId,redundancyEncoding->redundancyEncodingMethodId);
        if (redundancyEncoding->secondaryEncodingId >=0)
        {
          secEncodingId = pvtAdd(hVal, channel->redEncID, __h245(secondaryEncoding),0, NULL, NULL);
          pvtMoveTree(hVal,secEncodingId,redundancyEncoding->secondaryEncodingId );
        }
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetRedundancyEncoding: [1].");
    return TRUE;

}


RVAPI int RVCALLCONV
cmChannelGetRedundancyEncoding(
             IN      HCHAN               hsChan,
         OUT     cmRedundancyEncoding * redundancyEncoding)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int nodeId=-1;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetRedundancyEncoding: hsChan=0x%lx ", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->redEncID >=0)
        {
            redundancyEncoding->redundancyEncodingMethodId = pvtGetChild(hVal,channel->redEncID,__h245(redundancyEncodingMethod), NULL);
            redundancyEncoding->secondaryEncodingId = pvtGetChild(hVal,channel->redEncID,__h245(secondaryEncoding),NULL);
        }
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelGetRedundancyEncoding: redEncId [%d] redEncMethodId [%d] secondaryEncId [%d]",
               channel->redEncID, redundancyEncoding->redundancyEncodingMethodId,redundancyEncoding->secondaryEncodingId);
    return nodeId;

}






RVAPI int RVCALLCONV
cmChannelSetSource(
             IN      HCHAN               hsChan,
         IN     cmTerminalLabel *terminalLabel)

{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel || !terminalLabel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetSource: hsChan=0x%lx mcuNumber=[%d] terminalNumber = [%d]",
            hsChan,terminalLabel->mcuNumber,terminalLabel->terminalNumber);

    if (emaLock((EMAElement)hsChan))
    {
        channel->source=*terminalLabel;
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetSource: [OK].");
    return TRUE;

}


RVAPI int RVCALLCONV
cmChannelGetSource(
         IN      HCHAN               hsChan,
         OUT     cmTerminalLabel *terminalLabel)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel || !terminalLabel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetSource: hsChan=0x%lx ", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        if (terminalLabel)
            *terminalLabel=channel->source;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetSource: mcuNumber [%d] terminalNumber [%d]",
                   terminalLabel->mcuNumber,terminalLabel->terminalNumber);
    return TRUE;

}


RVAPI int RVCALLCONV
cmChannelSetDestination(
             IN      HCHAN               hsChan,
         IN     cmTerminalLabel *terminalLabel)

{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel || !terminalLabel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetDestination: hsChan=0x%lx mcuNumber=[%d] terminalNumber = [%d]",
            hsChan,terminalLabel->mcuNumber,terminalLabel->terminalNumber);

    if (emaLock((EMAElement)hsChan))
    {
        channel->destination=*terminalLabel;
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetDestination: [OK].");
    return TRUE;

}


RVAPI int RVCALLCONV
cmChannelGetDestination(
         IN      HCHAN               hsChan,
         OUT     cmTerminalLabel *terminalLabel)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetDestination: hsChan=0x%lx ", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        if (terminalLabel)
            *terminalLabel=channel->destination;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetDestination: mcuNumber [%d] terminalNumber [%d]",
                   terminalLabel->mcuNumber,terminalLabel->terminalNumber);
  return TRUE;

}

static const int reasons[]={__h245(unknown),__h245(reopen),__h245(reservationFailure),__h245(normal)};

RVAPI int RVCALLCONV
cmChannelDropReason(
          /*
        - Close outgoing channel
        - Reject incoming channel open request
        - New: Request to close incoming channel
          */
          IN  HCHAN            hsChan,
          IN  cmCloseLcnReason reason
          )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    cmElem* app=(cmElem*)hApp;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;

    if (!hApp || !channel || (int)reason<(int)cmCloseReasonUnknown || (int)reason>(int)cmCloseReasonNormal) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelDropReason: hsChan=0x%lx.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        controlElem *ctrl = (controlElem *)channel->ctrl;

        /* Make sure this isn't a faststart channel without any connected control */
        if (ctrl->state == ctrlNotInitialized)
        {
            logPrint(app->logERR, RV_ERROR,
                     (app->logERR, RV_ERROR, "cmChannelDropReason: Control is not initialized (probably in faststart mode)"));
            res = RVERROR;
        }

        else if (channel->origin)
        {
            if (channel->state==released)
            {
                notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeOff);
                notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
                emaUnlock((EMAElement)hsChan);
                cmiAPIExit(hApp, "cmChannelDropReason: [OK].");
                return TRUE;
            }

            if (reason==cmCloseReasonNormal)
                reason = cmCloseReasonUnknown;
            if (channel->state==awaitingEstablishment)
                ctrl->conflictChannels--;
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            nodeId=pvtAddBranch2(hVal,message, __h245(request),__h245(closeLogicalChannel));
            pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
            pvtAddBranch2(hVal,nodeId,__h245(reason),reasons[reason]);
            pvtAddBranch2(hVal,nodeId,__h245(source),__h245(user));

            res = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);

            if (res >= 0)
            {
                {        /* base channel released -- drop dependent channel*/
                    void* ptr=NULL;
                    channelElem* dependent;
                    while((dependent=getNextOutChanByBase(channel->ctrl,channel,&ptr)))
                        cmChannelDrop((HCHAN)dependent);
                }
                {
                    INT32 timeout=29;
                    pvtGetChildValue(hVal,app->h245Conf,__h245(channelsTimeout),&(timeout),NULL);
                    cmTimerReset(hApp,&(channel->timer));
                    channel->timer=cmTimerSet(hApp,channelTimeoutEventsHandler,(void*)channel,timeout*1000);
                }

                channel->state=awaitingRelease;
            }
        }
        else
        switch(channel->state)
        {
            case awaitingEstablishment:
                message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
                nodeId=pvtAddBranch2(hVal,message, __h245(response),__h245(openLogicalChannelReject));
                pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
                pvtAddBranch2(hVal,nodeId,__h245(cause),__h245(unspecified));
                /* the following line just returns error   -Ran
                pvtAddBranch2(hVal,nodeId,__h245(source),__h245(user)); */

                res = sendMessageH245Chan(channel->ctrl, hsChan, message);
                pvtDelete(hVal,message);

                if (res >= 0)
                {
                    channel->state=released;
                    notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                    notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
                }
                break;
            case established:
                res = cmChannelRequestClose(hsChan, reason, NULL);
                break;
            default:
                break;
        }

        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelDropReason=%d", res);
    return res;
}




/*
cmChannelSetNSAPAddress pass atm address for channel to state machine
Parameters:
hsChan          handle of channel
address         octet string atm nsap address
length          length of nsap address in bytes
multicast       TRUE if it is multicast address.
Return:         in case of error return negative value

*/

RVAPI
int RVCALLCONV cmChannelSetNSAPAddress(
        IN      HCHAN               hsChan,
        IN      char*               address,
        IN      int                 length,
        IN      BOOL                multicast)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    cmTransportAddress ta;
    cmElem* app=(cmElem*)hApp;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetNSAPAddress: hsChan=0x%lx, length=%d.", hsChan, length);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->recvRtpAddressID<0)
            channel->recvRtpAddressID = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);

        ta.type=cmTransportTypeNSAP;
        ta.distribution=(multicast)?cmDistributionMulticast:cmDistributionUnicast;
        memcpy(ta.route,address,min(length,(int)sizeof(ta.route)));
        ta.length=(UINT16)length;
        cmTAToVt_H245(hVal,channel->recvRtpAddressID,&ta);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelSetNSAPAddress: [1].");
    return TRUE;
}

/*
cmChannelSetATMVC pass port number  to state machine
Parameters:
hsChan          handle of channel
portNumber      portNumber acording to asn 2 bytes length
Return:         in case of error return negative value

*/


RVAPI
int RVCALLCONV cmChannelSetATMVC(
        IN      HCHAN   hsChan,
        IN      int     portNumber)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelSetATMVC: hsChan=0x%lx, port=%d.", hsChan, portNumber);
    if (emaLock((EMAElement)hsChan))
    {
        channel->port=portNumber;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelSetATMVC: [1].");
    return TRUE;
}

RVAPI
INT32 RVCALLCONV cmChannelGetNumber(IN HCHAN hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    int lcn=RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetNumber: hsChan=0x%lx.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        lcn =channel->myLCN;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetNumber: [%d].",lcn);

    return lcn;
}


/*________________________________request channel close handling_____________________________________*/


RVAPI int RVCALLCONV
cmChannelRequestClose(
          /*
        - New: Request to close incoming channel
          */
            IN  HCHAN       hsChan,
            IN  cmCloseLcnReason reason,
            cmQosCapability* cmQOSCapability
          )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    cmElem* app=(cmElem*)hApp;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmNonStandardParam *nonStandardPtr=NULL;
    cmRSVPParameters * rsvpParametersPtr=NULL;
    cmATMParameters  * atmParametersPtr=NULL;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelRequestClose: hsChan=0x%lx.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        controlElem *ctrl = (controlElem *)channel->ctrl;

        /* Make sure this isn't a faststart channel without any connected control */
        if (ctrl->state == ctrlNotInitialized)
        {
            logPrint(app->logERR, RV_ERROR,
                     (app->logERR, RV_ERROR, "cmChannelRequestClose: Control is not initialized (probably in faststart mode)"));
            res = RVERROR;
        }

        else
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            nodeId=pvtAddBranch2(hVal,message, __h245(request),__h245(requestChannelClose));
            pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
            pvtAddBranch2(hVal,nodeId,__h245(reason),reasons[reason]);
            if (cmQOSCapability!=NULL)
            {
                if (cmQOSCapability->cmQosCapabilityParamUsed & cmQosCapabilityNonStandard)
                    nonStandardPtr = &cmQOSCapability->cmNonStandard;
                if (cmQOSCapability->cmQosCapabilityParamUsed & cmQosCapabilityAtmParameters)
                    atmParametersPtr = &cmQOSCapability->cmAtmParameters;
                if (cmQOSCapability->cmQosCapabilityParamUsed & cmQosCapabilityRsvpParameters)
                    rsvpParametersPtr = &cmQOSCapability->cmRsvpParameters;
                buildQosCapability( hVal,pvtAdd(hVal, nodeId, __h245(qosCapability), 0, NULL, NULL),rsvpParametersPtr,atmParametersPtr,nonStandardPtr);
            }

            res = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);

            if (res >= 0)
            {
                INT32 timeout=29;
                pvtGetChildValue(hVal,app->h245Conf,__h245(requestCloseTimeout),&(timeout),NULL);
                cmTimerReset(hApp,&(channel->rc_timer));
                channel->rc_timer=cmTimerSet(hApp,channelRC_TimeoutEventsHandler,(void*)channel,timeout*1000);
            }
        }

        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelRequestClose=%d", res);
    return res;
}

RVAPI int RVCALLCONV
cmChannelGetRequestCloseParam(  IN  HCHAN       hsChan,
                                OUT  cmCloseLcnReason *reason,
                                INOUT cmQosCapability * cmQOSCapability
                               )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);

    if (!hApp || !channel || !channel->rc_paramID<0) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelGetRequestCloseParam: hsChan=0x%lx haChan=0x%lx", hsChan,emaGetApplicationHandle((EMAElement)channel));
    if (emaLock((EMAElement)hsChan))
    {

        getQosParameters(hVal,pvtGetChild(hVal,channel->rc_paramID,__h245(qosCapability), NULL),cmQOSCapability);
        if (reason)
        {
            int iReason = pvtGetSyntaxIndex(hVal,pvtGetChild(hVal,channel->rc_paramID,__h245(reason), NULL))-1;
            *reason=(cmCloseLcnReason)iReason;
        }
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit(hApp, "cmChannelGetRequestCloseParam: [1].");
    return TRUE;
}

RVAPI int RVCALLCONV
cmChannelRequestCloseReject(
                /* Reject the request for closing the outgoing channel. */
                IN  HCHAN       hsChan
                )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    cmElem* app=(cmElem*)hApp;
    int message, nodeId, res = RVERROR;

    if (!hApp || !channel) return RVERROR;

    cmiAPIEnter(hApp, "cmChannelRequestCloseReject: hsChan=0x%lx.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(response),__h245(requestChannelCloseReject));
        pvtAdd(hVal,nodeId,__h245(forwardLogicalChannelNumber),channel->myLCN,NULL,NULL);
        pvtAddBranch2(hVal,nodeId,__h245(cause),__h245(unspecified));

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit(hApp, "cmChannelRequestCloseReject=%d", res);
    return res;
}


/****************************************************************************************************************/
/* This function shall be called before the first channel is opened using the H.245 procedures.                 */
/* It is protected from the second call so it may be called each time the new channel is about to be established*/
/* The main purpose of the function is to mark well-known session IDs used by the fast start and                */
/* prevent in this way usage of them in H.245                                                                   */
/****************************************************************************************************************/
void deriveChannels(HCONTROL ctrl)
{
    controlElem*ctrlE=(controlElem*)ctrl;
    channelElem* channel;
    void* ptr=NULL;
    if (!ctrlE->isDerived)
    {
        ctrlE->firstAudioIn=1;
        ctrlE->firstVideoIn=1;
        ctrlE->firstAudioOut=1;
        ctrlE->firstVideoOut=1;
        ctrlE->firstDataIn=1;
        ctrlE->firstDataOut=1;
        while((channel=getNextChan(ctrl, &ptr)))
        {
            if (channel->sid==1 && channel->origin) ctrlE->firstAudioOut=0;
            if (channel->sid==1 && !channel->origin) ctrlE->firstAudioIn=0;
            if (channel->sid==2 && channel->origin) ctrlE->firstVideoOut=0;
            if (channel->sid==2 && !channel->origin) ctrlE->firstVideoIn=0;
            if (channel->sid==3 && channel->origin) ctrlE->firstDataOut=0;
            if (channel->sid==3 && !channel->origin) ctrlE->firstDataIn=0;
        }
        ctrlE->isDerived=TRUE;
    }
}

/****************************************************************************************************************/
/* This function shall be called before a channel becomes idle, in order to enable subsequent channels to use   */
/* the default session IDs 1, 2 and 3.                                                                          */
/****************************************************************************************************************/
static void setFirstChannels(channelElem * channel)
{
    controlElem * ctrl = (controlElem *)channel->ctrl;

    if (ctrl != NULL)
    {
        if (channel->origin)
        {
            /* Make sure the default used session Id's are updated if we close them */
            switch (channel->sid)
            {
            case 1: ctrl->firstAudioOut = TRUE; break;
            case 2: ctrl->firstVideoOut = TRUE; break;
            case 3: ctrl->firstDataOut = TRUE; break;
            default: break; /* Do nothing */
            }
        }
        else
        {
            /* Make sure the default used session Id's are updated if we close them */
            switch (channel->sid)
            {
            case 1: ctrl->firstAudioIn = TRUE; break;
            case 2: ctrl->firstVideoIn = TRUE; break;
            case 3: ctrl->firstDataIn = TRUE; break;
            default: break; /* Do nothing */
            }
        }
    }
}


/************************************************************************
 * closeChannels
 * purpose: Close any channels of a given control object.
 *          This function only notifies the application about closing the
 *          channels due to disconnection.
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void closeChannels(IN HCONTROL ctrl)
{
    channelElem* channel;
    void* ptr=NULL;

    /* Notify the application about every channel it has about its closing */
    while((channel=getNextChan(ctrl, &ptr)))
    {
        if(emaLock((EMAElement)channel))
        {
            if (channel->state != released)
            {
            /* Avoid calling these callbacks if we're already in released state for the channel.
                Otherwise, we might get an endless loop */
                channel->state = released;
                notifyChannelState(channel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedLocal);
                notifyChannelState(channel,cmChannelStateIdle, cmChannelStateModeOff);
            }
            emaUnlock((EMAElement)channel);
        }
    }
}


/******************************************************************************
 * incomingChannelMessage
 * ----------------------------------------------------------------------------
 * General: Make sure cmEvChannelRecvMessage() is invoked in necessary on
 *          incoming channel related messages.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  hApp     - Stack instance used
 *         channel  - Channel this message belongs to
 *         message  - Message received
 * Output: None.
*****************************************************************************/
void incomingChannelMessage(
    IN HAPP             hApp,
    IN channelElem*     channel,
    IN int              message)
{
    int nesting;
    if (((cmElem*)hApp)->cmMyChannelEvent.cmEvChannelRecvMessage)
    {
        cmiCBEnter(hApp, "cmEvChannelRecvMessage(haChan=0x%p,hsChan=0x%p,msg=%d)",
            (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, message);
        nesting = emaPrepareForCallback((EMAElement)channel);
        (((cmElem*)hApp)->cmMyChannelEvent.cmEvChannelRecvMessage)(
            (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel),
            (HCHAN)channel,
            message);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit(hApp, "cmEvChannelRecvMessage");
    }
}


/************************************************************************
 * chanGetMibParam
 * purpose: Get channel related MIB parameters
 * input  : hsChan      - Channel to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int chanGetMibParam(
    IN  HCHAN                   hsChan,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value)
{
    channelElem* channel = (channelElem *)hsChan;

    switch (type)
    {
        case enumh245LogChannelsChannelState:
            switch (channel->state)
            {
                case released:              return RVERROR;
                case awaitingEstablishment: *valueSize = lcseAwaitingEstablishment; break;
                case faststart:             *valueSize = lcseEstablished;           break;
                case fsAwaitingEstablish:   *valueSize = lcseAwaitingEstablishment; break;
                case established:           *valueSize = lcseEstablished;           break;
                case awaitingRelease:       *valueSize = lcseAwaitingRelease;       break;
                case awaitingConfirmation:  *valueSize = lcseAwaitingConfirmation;  break;
                default:                    return RVERROR;
            }
            return 0;

        case enumh245LogChannelsMediaTableType:
        {
            HPVT    hVal;
            int     nodeId, index;
            INTPTR  fieldId;

            hVal = cmGetValTree((HAPP)emaGetInstance((EMAElement)channel));
            nodeId = pvtChild(hVal, channel->dataTypeID);
            if (pvtGet(hVal, nodeId, &fieldId, NULL, NULL, NULL) < 0)
                return RVERROR;

            switch (fieldId)
            {
                case __h245(nonStandard):
                    *valueSize = 1;
                    return 0;
                case __h245(videoData): *valueSize = 1; break;
                case __h245(audioData): *valueSize = 6; break;
                case __h245(data):      *valueSize = 26; break;
                default:
                    return RVERROR;
            }

            index = pvtGetSyntaxIndex(hVal, pvtChild(hVal, nodeId));
            (*valueSize) += index;
            return 0;
        }

        case enumh245LogChannelsSessionId:
            *valueSize = channel->sid;
            return 0;

        case enumh245LogChannelsAssociateSessionId:
            if (channel->associated != NULL)
            {
                *valueSize = channel->associated->sid;
                return 0;
            }
            break;

        case enumh245LogChannelsMediaChannel:
            if (channel->recvRtpAddressID >= 0)
            {
                if (cmVtToTA_H245(cmGetValTree((HAPP)emaGetInstance((EMAElement)channel)), channel->recvRtpAddressID, (cmTransportAddress *)value) >= 0)
                    return 0;
            }
            break;

        case enumh245LogChannelsMediaControlChannel:
        {
            if (channel->recvRtcpAddressID >= 0)
            {
                if (cmVtToTA_H245(cmGetValTree((HAPP)emaGetInstance((EMAElement)channel)), channel->recvRtcpAddressID, (cmTransportAddress *)value) >= 0)
                    return 0;
            }
            break;
        }

        case enumh245LogChannelsDestination:
            value[0] = (UINT8)(channel->destination.mcuNumber & 0xff);
            value[1] = (UINT8)(channel->destination.terminalNumber & 0xff);
            *valueSize = 2;
            return 0;

        case enumh245LogChannelsSrcTerminalLabel:
            value[0] = (UINT8)(channel->source.mcuNumber & 0xff);
            value[1] = (UINT8)(channel->source.terminalNumber & 0xff);
            *valueSize = 2;
            return 0;

        case enumh245LogChannelsDynamicRTPPayloadType:
            if (channel->dynamicPayloadNumber >= 0)
            {
                *valueSize = channel->dynamicPayloadNumber;
                return 0;
            }
            break;

        case enumh245LogChannelsSilenceSuppression:
        case enumh245LogChannelsMediaGuaranteedDelivery:
        case enumh245LogChannelsMediaControlGuaranteedDelivery:
        case enumh245LogChannelsH261aVideoPacketization:
        case enumh245LogChannelsRTPPayloadDescriptor:
        case enumh245LogChannelsRTPPayloadType:
        case enumh245LogChannelsTransportCapability:
        case enumh245LogChannelsRedundancyEncoding:
            /* parameters not supported */
            break;

        default:
            break;
    }

    return RVERROR;
}


#ifdef __cplusplus
}
#endif
