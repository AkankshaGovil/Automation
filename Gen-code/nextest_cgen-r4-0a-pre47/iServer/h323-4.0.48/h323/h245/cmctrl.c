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
#include <cm.h>
#include <cmictrl.h>
#include <cmintr.h>
#include <cmCall.h>
#include <ms.h>
#include <conf.h>
#include <caputils.h>
#include <cmutils.h>
#include <netutl.h>
#include <strutils.h>
#include <prnutils.h>
#include <cmchan.h>
#include <oidutils.h>
#include <h245.h>
#include <cmCtrlCap.h>
#include <cmCtrlMSD.h>
#include <cmCtrlRTD.h>
#include <cmCtrlMMode.h>
#include <cmCtrlRMode.h>
#include <cmCtrlMPoint.h>
#include <cmChanOperations.h>
#include <cmdebprn.h>
#include <pvaltree.h>


int userInput(controlElem*ctrl, int message);


/* General __________________________________________________________________________________*/
void notifyControlState(HCALL hsCall);
int cmiReportControl(IN HCALL               hsCall,
                     IN cmControlState      state,
                     IN cmControlStateMode  stateMode)
{
    callElem*call=(callElem*)hsCall;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    int nesting;

    {
#ifndef NOLOGSUPPORT
        static char *stateModes[]=
        {    (char*)"cmControlStateModeNull",(char*)""};

        static char *states[]=
        {
            (char*)"cmControlStateConnected",
            (char*)"cmControlStateConference",
            (char*)"cmControlStateTransportConnected",
            (char*)"cmControlStateTransportDisconnected",
            (char*)"cmControlStateFastStart"
        };

        if(app->cmMySessionEvent.cmEvCallControlStateChanged)
            cmiCBEnter((HAPP)app,(char*)"cmEvCallControlStateChanged(haCall=0x%lx,hsCall=0x%lx,state=%s,stateMode=%s)",emaGetApplicationHandle((EMAElement)hsCall),hsCall,nprn(states[state]),nprn(stateModes[stateMode+1]));
#endif
    }
    nesting=emaPrepareForCallback((EMAElement)hsCall);
    ifE(app->cmMySessionEvent.cmEvCallControlStateChanged)((HAPPCALL)emaGetApplicationHandle((EMAElement)hsCall),hsCall,state,stateMode);
    emaReturnFromCallback((EMAElement)hsCall,nesting);
#ifndef NOLOGSUPPORT
    if(app->cmMySessionEvent.cmEvCallControlStateChanged)
        cmiCBExit((HAPP)app,(char*)"cmEvCallControlStateChanged");
#endif

    if (call->state == cmCallStateConnected)
        notifyControlState(hsCall);

    if ((app->mibEvent.h341AddControl != NULL) && ((state == cmControlStateConnected) || (state == cmControlStateFastStart)))
          app->mibEvent.h341AddControl(app->mibHandle, hsCall);

    return 0;
}


RVAPI int RVCALLCONV
cmSetSessionEventHandler(
             /* Set user callbacks functions for control session. */
             IN HAPP        hApp,
             IN     CMSESSIONEVENT  cmSessionEvent,
             IN     int     size)
{
  cmElem *app = (cmElem *)hApp;

  if (!app) return RVERROR;

  cmiAPIEnter((HAPP)app, "cmSetSessionEventHandler: hApp=0x%lx, cmSessionEvent=0x%lx, size=%d.",
     hApp, cmSessionEvent, size);

  app->dynamicPayloadNumber = 0;

  memset(&(app->cmMySessionEvent), 0, sizeof(app->cmMySessionEvent));
  memcpy(&(app->cmMySessionEvent), cmSessionEvent, min((int)sizeof(app->cmMySessionEvent), size));

  cmiAPIExit((HAPP)app, "cmSetSessionEventHandler: [0].");
  return 0;
}


RVAPI int RVCALLCONV
cmGetControlEventHandler(
             /* Set user callbacks functions for control session. */
             IN HAPP        hApp,
             OUT    CMSESSIONEVENT  cmSessionEvent,
             IN     int     size)
{
  cmElem *app = (cmElem *)hApp;

  if (!app) return RVERROR;

  cmiAPIEnter((HAPP)app, "cmGetControlEventHandler: hApp=0x%lx, cmSessionEvent=0x%lx, size=%d.",
     hApp, cmSessionEvent, size);

  memset(cmSessionEvent, 0, size);
  memcpy(cmSessionEvent,&(app->cmMySessionEvent),  min((int)sizeof(app->cmMySessionEvent), size));

  cmiAPIExit((HAPP)app, "cmGetControlEventHandler: [0].");
  return 0;
}


/* returns the value-tree node id of h245 configuration root node */
RVAPI
INT32 RVCALLCONV cmGetH245ConfigurationHandle(
                IN  HAPP             hApp)
{
  cmElem *app = (cmElem *)hApp;
  if (!app) return RVERROR;

  cmiAPIEnter((HAPP)app, "cmGetH245ConfigurationHandle: hApp=0x%lx.", hApp);

  cmiAPIExit((HAPP)app, "cmGetH245ConfigurationHandle: [%d].", app->h245Conf);
  return app->h245Conf;
}


int
cmcReadyEvent(
          /* Check if session reached ready state and if so inform application. */
          controlElem* ctrl
          )
{
  callElem *call=(callElem *)cmiGetByControl((HCONTROL)ctrl);
  cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);

  if (!app) return RVERROR;
  if ((ctrl->outCapStatus == capAcknowledged) && (ctrl->inCapStatus == capAcknowledged) &&
      ctrl->isMasterSlave && ctrl->state==ctrlInitialized)
  {
      if ((!cmTransIsParallel(call->hsTransSession) || m_callget(call,fastStartFinished)))
      {
          ctrl->state=ctrlConnected;
          cmiReportControl(cmiGetByControl((HCONTROL)ctrl),  cmControlStateConnected,(cmControlStateMode)0);
          if (ctrl->incomingOLCs[0] >= 0)
          {
              openLogicalChannel(ctrl, ctrl->incomingOLCs[0], pvtChild(app->hVal, pvtChild(app->hVal, ctrl->incomingOLCs[0])));
              pvtDelete(app->hVal, ctrl->incomingOLCs[0]);
              ctrl->incomingOLCs[0] = -1;
          }
          if (ctrl->incomingOLCs[1] >= 0)
          {
              openLogicalChannel(ctrl, ctrl->incomingOLCs[1], pvtChild(app->hVal, pvtChild(app->hVal, ctrl->incomingOLCs[1])));
              pvtDelete(app->hVal, ctrl->incomingOLCs[1]);
              ctrl->incomingOLCs[1] = -1;
          }
      }
  }
  return TRUE;
}



void cmH245Start(HAPP hApp)
{
    cmElem *app = (cmElem *)hApp;
    app->t101=9000;
    pvtGetChildValue2(app->hVal,app->h245Conf,__h245(capabilities),__h245(timeout),(INT32 *)&(app->t101),NULL);
}

void cmH245Stop(HAPP hApp)
{
    cmElem *app = (cmElem *)hApp;
    if (app);
}


void initControl(HCONTROL ctrl, int lcnOut)
{
    controlElem*ctrlE=(controlElem*)ctrl;

    memset((void *)ctrl,0,sizeof(controlElem));

    ctrlE->lcnOut=lcnOut;
    ctrlE->outCapStatus=capReleased;
    ctrlE->inCapStatus=capReleased;
    ctrlE->isMasterSlave=0;
    ctrlE->isMaster=0;

    ctrlE->conflictChannels=0;

    ctrlE->myTermLabel.mcuNumber=255;
    ctrlE->myTermLabel.terminalNumber=255;
    capInit(ctrlE);
    msdInit(ctrlE);
    rtdInit(ctrlE);
    rmInit(ctrlE);		/* NexTone: part of Radvision patch */
    ctrlE->mpMode=0;
    ctrlE->state=ctrlInitialized;
/* 	ctrlE->out_RM.timer = (HTI)RVERROR; NexTone: part of Radvision patch */ 
    ctrlE->incomingOLCs[0] = -1;
    ctrlE->incomingOLCs[1] = -1;
}

void startControl(HCONTROL ctrl)
{
    controlElem*ctrlE=(controlElem*)ctrl;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
    HPVT hVal=app->hVal;
    int capNodeId, res = 0;

    if (pvtGetChild2(hVal,app->h245Conf,__h245(capabilities),__h245(manualOperation))<0 &&
        (capNodeId=(pvtGetChild2(hVal,app->h245Conf,__h245(capabilities),__h245(terminalCapabilitySet)))))
    {
        int nodeId=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        int tcsNodeId=pvtAddBranch2(hVal,nodeId,__h245(request),__h245(terminalCapabilitySet));
        res = pvtSetTree(hVal,tcsNodeId,hVal,capNodeId);
        if (res >= 0)
            res = outCapTransferRequest(ctrlE, nodeId);
        pvtDelete(hVal,nodeId);
    }

    if ((res >= 0) && (pvtGetChild2(hVal,app->h245Conf,__h245(masterSlave),__h245(manualOperation))<0))
        msdDetermineRequest(ctrlE, -1, -1);
}


/************************************************************************
 * stopControl
 * purpose: Stop the H245 Control connection for a call.
 * input  : ctrl    - Control object
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
void stopControl(IN HCONTROL ctrl)
{
    controlElem*ctrlE=(controlElem*)ctrl;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
    int nodeId, message;
    HPVT hVal=app->hVal;

    /* Make sure we close all the channels */
    closeChannels(ctrl);

    /* Send an endSession message if we have to */
    if ((ctrlE->state != ctrlNotInitialized) && (ctrlE->state != ctrlEndSession))
    {
        /* Create an endSession command message */
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message,__h245(command), __h245(endSessionCommand) );
        pvtAddBranch(hVal,nodeId,__h245(disconnect));

        /* Send the endSession message */
        sendMessageH245(ctrl, message);

        /* Kill the created message */
        pvtDelete(app->hVal,message);

        /* Make sure we're in the endSession state */
        ctrlE->state=ctrlEndSession;
    }

    /* Finish with the capabilities, MSD and RTD procedures */
    capEnd(ctrlE);
    msdEnd(ctrlE);
    rtdEnd(ctrlE);
    rmEnd(ctrlE);	/* NexTone: part of Radvision patch */

    if (ctrlE->incomingOLCs[0] >= 0)
    {
        pvtDelete(hVal, ctrlE->incomingOLCs[0]);
        ctrlE->incomingOLCs[0] = -1;
    }
    if (ctrlE->incomingOLCs[1] >= 0)
    {
        pvtDelete(hVal, ctrlE->incomingOLCs[1]);
        ctrlE->incomingOLCs[1] = -1;
    }

    /* report that the control has disconnected */
    cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateTransportDisconnected,(cmControlStateMode)0);
}

controlState controlGetState(HCONTROL ctrl)
{
   controlElem*ctrlE=(controlElem*)ctrl;
   return ctrlE->state;
}

int endSessionCommand(controlElem*ctrl, int message)
{
    callElem*call=(callElem*)cmiGetByControl((HCONTROL)ctrl);
    if (message);
    
    /* See if we need to stop the control at all - it might have been closed already by an
       outgoing endSessionCommand message */
    if (m_callget(call, control))
    {
        /* First update the status so we don't get into stopControl() twice from 2 different
           threads: one on the incoming endSessionCommand and one for an outgoing one */
        m_callset(call, control, FALSE);
        stopControl((HCONTROL)ctrl);
    }

    return TRUE;
}


typedef enum
{
    h245Request=1,
    h245Response,
    h245Command,
    h245Indication
}h245MsgType;

typedef enum
{
    h245nonStandard=1               ,
    h245masterSlaveDetermination    ,
    h245terminalCapabilitySet       ,
    h245openLogicalChannel          ,
    h245closeLogicalChannel         ,
    h245requestChannelClose         ,
    h245multiplexEntrySend          ,
    h245requestMultiplexEntry       ,
    h245requestMode                 ,
    h245roundTripDelayRequest       ,
    h245maintenanceLoopRequest      ,
    h245communicationModeRequest    ,
    h245conferenceRequest           ,
    h245multilinkRequest            ,
    h245logicalChannelRateRequest
}h245MsgRequestType;

typedef enum
{
    /*h245nonStandard=1               ,*/
    h245masterSlaveDeterminationAck=2   ,
    h245masterSlaveDeterminationReject  ,
    h245terminalCapabilitySetAck    ,
    h245terminalCapabilitySetReject ,
    h245openLogicalChannelAck   ,
    h245openLogicalChannelReject    ,
    h245closeLogicalChannelAck  ,
    h245requestChannelCloseAck  ,
    h245requestChannelCloseReject   ,
    h245multiplexEntrySendAck   ,
    h245multiplexEntrySendReject    ,
    h245requestMultiplexEntryAck    ,
    h245requestMultiplexEntryReject ,
    h245requestModeAck  ,
    h245requestModeReject   ,
    h245roundTripDelayResponse  ,
    h245maintenanceLoopAck  ,
    h245maintenanceLoopReject   ,
    h245communicationModeResponse   ,
    h245conferenceResponse  ,
    h245multilinkResponse   ,
    h245logicalChannelRateAcknowledge   ,
    h245logicalChannelRateReject
}h245MsgResponseType;

typedef enum
{
    /*h245nonStandard=1             ,*/
    h245maintenanceLoopOffCommand=2,
    h245sendTerminalCapabilitySet,
    h245encryptionCommand,
    h245flowControlCommand,
    h245endSessionCommand,
    h245miscellaneousCommand,
    h245communicationModeCommand,
    h245conferenceCommand,
    h245h223MultiplexReconfiguration,
    h245newATMVCCommand
}h245MsgCommandType;

typedef enum
{
    /*h245nonStandard=1             ,*/
    h245functionNotUnderstood=2,
    h245masterSlaveDeterminationRelease,
    h245terminalCapabilitySetRelease,
    h245openLogicalChannelConfirm,
    h245requestChannelCloseRelease,
    h245multiplexEntrySendRelease,
    h245requestMultiplexEntryRelease,
    h245requestModeRelease,
    h245miscellaneousIndication,
    h245jitterIndication,
    h245h223SkewIndication,
    h245newATMVCIndication,
    h245userInput,
    h245h2250MaximumSkewIndication,
    h245mcLocationIndication,
    h245conferenceIndication,
    h245vendorIdentification,
    h245functionNotSupported,
    h245multilinkIndication,
    h245logicalChannelRateRelease,
    h245flowControlIndication
}h245MsgIndicationType;

typedef enum
{
    h245equaliseDelay=1 ,
    h245zeroDelay   ,
    h245multipointModeCommand,
    h245cancelMultipointModeCommand ,
    h245videoFreezePicture  ,
    h245videoFastUpdatePicture  ,
    h245videoFastUpdateGOB  ,
    h245videoTemporalSpatialTradeOff    ,
    h245videoSendSyncEveryGOB   ,
    h245videoSendSyncEveryGOBCancel ,
    h245videoFastUpdateMB   ,
    h245maxH223MUXPDUsize   ,
    h245encryptionUpdate    ,
    h245encryptionUpdateRequest ,
    h245switchReceiveMediaOff   ,
    h245switchReceiveMediaOn    ,
    h245progressiveRefinementStart  ,
    h245progressiveRefinementAbortOne       ,
    h245progressiveRefinementAbortContinuous
}h245MsgMiscType;

typedef enum
{
    h245logicalChannelActive=1,
    h245logicalChannelInactive,
    h245multipointConference,
    h245cancelMultipointConference,
    h245multipointZeroComm,
    h245cancelMultipointZeroComm,
    h245multipointSecondaryStatus,
    h245cancelMultipointSecondaryStatus,
    h245videoIndicateReadyToActivate,
    h245videoTemporalSpatialTradeOffInd,
    h245videoNotDecodedMBs,
    h245transportCapability
}h245MsgMiscIndType;

void h245ProcessIncomingMessage(HCONTROL ctrl, int message)
{
    controlElem*ctrlE=(controlElem*)ctrl;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
    HPVT hVal=app->hVal;
    int child=pvtChild(hVal,message);
    int grandchild=pvtChild(hVal,child);
    int type=pvtGetSyntaxIndex(hVal,child);
    int messageId=pvtGetSyntaxIndex(hVal,grandchild);
    switch(type)
    {
        case    h245Request:
            switch(messageId)
            {
                case h245nonStandard                :break;
                case h245masterSlaveDetermination   :  masterSlaveDetermination(ctrlE,grandchild);          break;
                case h245terminalCapabilitySet      :  terminalCapabilitySet(ctrlE,grandchild);             break;
                case h245openLogicalChannel         :  openLogicalChannel(ctrlE,message,grandchild);        break;
                case h245closeLogicalChannel        :  closeLogicalChannel(ctrlE,message,grandchild);       break;
                case h245requestChannelClose        :  requestChannelClose(ctrlE,message,grandchild);       break;
                case h245multiplexEntrySend         :break;
                case h245requestMultiplexEntry      :break;
                case h245requestMode                :  requestMode(ctrlE,grandchild);                       break;
                case h245roundTripDelayRequest      :  roundTripDelayRequest(ctrlE,grandchild);             break;
                case h245maintenanceLoopRequest     :  maintenanceLoopRequest(ctrlE,message,grandchild);    break;
                case h245communicationModeRequest   :break;
                case h245conferenceRequest          :break;
                case h245multilinkRequest           :break;
                case h245logicalChannelRateRequest  :break;
            }
        break;
        case    h245Response:
            switch(messageId)
            {
                case h245nonStandard                    :break;
                case h245masterSlaveDeterminationAck    :  masterSlaveDeterminationAck(ctrlE,grandchild);    break;
                case h245masterSlaveDeterminationReject :  masterSlaveDeterminationReject(ctrlE,grandchild); break;
                case h245terminalCapabilitySetAck       :  terminalCapabilitySetAck(ctrlE,grandchild);       break;
                case h245terminalCapabilitySetReject    :  terminalCapabilitySetReject(ctrlE,grandchild);    break;
                case h245openLogicalChannelAck          :  openLogicalChannelAck(ctrlE,message,grandchild);  break;
                case h245openLogicalChannelReject       :  openLogicalChannelReject(ctrlE,message,grandchild);break;
                case h245closeLogicalChannelAck         :  closeLogicalChannelAck(ctrlE,message,grandchild); break;
                case h245requestChannelCloseAck         :  requestChannelCloseAck(ctrlE,message,grandchild); break;
                case h245requestChannelCloseReject      :  requestChannelCloseReject(ctrlE,message,grandchild);break;
                case h245multiplexEntrySendAck          :break;
                case h245multiplexEntrySendReject       :break;
                case h245requestMultiplexEntryAck       :break;
                case h245requestMultiplexEntryReject    :break;
                case h245requestModeAck                 :  requestModeAck(ctrlE,grandchild);                 break;
                case h245requestModeReject              :  requestModeReject(ctrlE,grandchild);              break;
                case h245roundTripDelayResponse         :  roundTripDelayResponse(ctrlE,grandchild);         break;
                case h245maintenanceLoopAck             :  maintenanceLoopAck(ctrlE,message,grandchild);     break;
                case h245maintenanceLoopReject          :  maintenanceLoopReject(ctrlE,message,grandchild);  break;
                case h245communicationModeResponse      :break;
                case h245conferenceResponse             :break;
                case h245multilinkResponse              :break;
                case h245logicalChannelRateAcknowledge  :break;
                case h245logicalChannelRateReject       :break;
            }
        break;
        case    h245Command:
            switch(messageId)
            {
                case h245nonStandard:                                                                    break;
                case h245maintenanceLoopOffCommand:     maintenanceLoopOffCommand(ctrlE,grandchild);     break;
                case h245sendTerminalCapabilitySet: break;
                case h245encryptionCommand:         break;
                case h245flowControlCommand:            flowControlCommand(ctrlE,message,grandchild);            break;
                case h245endSessionCommand:             endSessionCommand(ctrlE,grandchild);             break;
                case h245miscellaneousCommand           :
                {

                    INT32 lcn;
                    int grandgrandchild=pvtChild(hVal,pvtGetChild(hVal,grandchild,__h245(type),NULL));
                    int mctype=pvtGetSyntaxIndex(hVal,grandgrandchild);
                    pvtGetChildValue(hVal,grandchild,__h245(logicalChannelNumber),&lcn,NULL);
                    switch(mctype)
                    {
                        case h245equaliseDelay                       :break;
                        case h245zeroDelay                           :break;
                        case h245multipointModeCommand               :  multipointModeCommand(ctrlE,lcn);                       break;
                        case h245cancelMultipointModeCommand         :  cancelMultipointModeCommand(ctrlE,lcn);                 break;
                        case h245videoFreezePicture                  :  videoFreezePicture(ctrlE,message,lcn);                  break;
                        case h245videoFastUpdatePicture              :  videoFastUpdatePicture(ctrlE,message,lcn);              break;
                        case h245videoFastUpdateGOB                  :  videoFastUpdateGOB(ctrlE,message,grandgrandchild,lcn);  break;
                        case h245videoTemporalSpatialTradeOff        :  videoTemporalSpatialTradeOff(ctrlE,message,grandgrandchild,lcn);break;
                        case h245videoSendSyncEveryGOB               :  videoSendSyncEveryGOB(ctrlE,message,lcn);               break;
                        case h245videoSendSyncEveryGOBCancel         :  videoSendSyncEveryGOBCancel(ctrlE,message,lcn);         break;
                        case h245videoFastUpdateMB                   :  videoFastUpdateMB(ctrlE,message,grandgrandchild,lcn);   break;
                        case h245maxH223MUXPDUsize                   :break;
                        case h245encryptionUpdate                    :break;
                        case h245encryptionUpdateRequest             :break;
                        case h245switchReceiveMediaOff               : switchReceiveMediaOff(ctrlE, message, lcn);break;
                        case h245switchReceiveMediaOn                : switchReceiveMediaOn(ctrlE, message, lcn);break;
                        case h245progressiveRefinementStart          :break;
                        case h245progressiveRefinementAbortOne       :break;
                        case h245progressiveRefinementAbortContinuous:break;
                    }
                }
                break;
                case h245communicationModeCommand:  communicationModeCommand(ctrlE,grandchild); break;
                case h245conferenceCommand:break;
                case h245h223MultiplexReconfiguration:break;
                case h245newATMVCCommand:break;
            }
        break;
        case    h245Indication:
            switch(messageId)
            {
                case h245nonStandard:break;
                case h245functionNotUnderstood:break;
                case h245masterSlaveDeterminationRelease:  masterSlaveDeterminationRelease(ctrlE,grandchild);break;
                case h245terminalCapabilitySetRelease:break;
                case h245openLogicalChannelConfirm      :  openLogicalChannelConfirm(ctrlE,message,grandchild);break;
                case h245requestChannelCloseRelease:break;
                case h245multiplexEntrySendRelease:break;
                case h245requestMultiplexEntryRelease:break;
                case h245requestModeRelease:break;
                case h245miscellaneousIndication:
                {
                    INT32 lcn;
                    int grandgrandchild=pvtChild(hVal,pvtGetChild(hVal,grandchild,__h245(type),NULL));
                    int mctype=pvtGetSyntaxIndex(hVal,grandgrandchild);
                    pvtGetChildValue(hVal,grandchild,__h245(logicalChannelNumber),&lcn,NULL);
                    switch(mctype)
                    {
                        case h245logicalChannelActive           :logicalChannelActive(ctrlE,message,lcn);break;
                        case h245logicalChannelInactive         :logicalChannelInactive(ctrlE,message,lcn);break;
                        case h245multipointConference           :multipointConference(ctrlE,lcn); break;
                        case h245cancelMultipointConference     :cancelMultipointConference(ctrlE,lcn); break;
                        case h245multipointZeroComm             :break;
                        case h245cancelMultipointZeroComm       :break;
                        case h245multipointSecondaryStatus      :break;
                        case h245cancelMultipointSecondaryStatus:break;
                        case h245videoIndicateReadyToActivate   :break;
                        case h245videoTemporalSpatialTradeOffInd:videoTemporalSpatialTradeOffInd(ctrlE,message,grandgrandchild,lcn); break;
                        case h245videoNotDecodedMBs             :break;
                        case h245transportCapability            :transportCapability(ctrlE,message,grandgrandchild,lcn); break;
                    }
                }
                break;
                case h245jitterIndication:break;
                case h245h223SkewIndication:break;
                case h245newATMVCIndication:break;
                case h245userInput                      :  userInput(ctrlE,grandchild);                    break;
                case h245h2250MaximumSkewIndication     :  h2250MaximumSkewIndication(ctrlE,grandchild);    break;
                case h245mcLocationIndication           :  mcLocationIndication(ctrlE,grandchild);          break;
                case h245conferenceIndication           :  conferenceIndication(ctrlE,grandchild);          break;
                case h245vendorIdentification:break;
                case h245functionNotSupported:break;
                case h245multilinkIndication:break;
                case h245logicalChannelRateRelease:break;
                case h245flowControlIndication:break;
            }
        break;
    }
}

void h245DecodingFailed(HCONTROL ctrl,void*buff,int len)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl(ctrl));
    HPVT hVal=app->hVal;
    int message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
    int nodeId=pvtAddBranch2(hVal,message, __h245(indication), __h245(functionNotSupported));
    pvtAddBranch2(hVal,nodeId, __h245(cause), __h245(syntaxError));
    pvtAdd(hVal,nodeId,__h245(returnedFunction),len,(const char *)buff,NULL);
    sendMessageH245(ctrl, message);
    pvtDelete(hVal,message);

}


#ifdef __cplusplus
}
#endif
