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
#include <pvaltreeStackApi.h>
#include <conf.h>
#include <caputils.h>
#include <netutl.h>
#include <strutils.h>
#include <cmutils.h>
#include <oidutils.h>
#include <stkutils.h>
#include <copybits.h>
#include <h245.h>
#include <cmChanGetByXXX.h>
#include <cmdebprn.h>
#include <cmchan.h>
#define ifE(a) if(a)(a)
    
    
int startEstablishment(cmElem* app, channelElem*channel);
int notifyChannelState(channelElem*channel,cmChannelState_e state, cmChannelStateMode_e stateMode);


/* Multipoint __________________________________________________________________________________*/

int multipointConference(controlElem* ctrl,int lcn)
{
    if (lcn);

    ctrl->state=ctrlConference;

    cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConference,(cmControlStateMode)0);
    return TRUE;
}

int cancelMultipointConference(controlElem* ctrl,int lcn)
{
    if (lcn);

    ctrl->state=ctrlConnected;

    cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConnected,(cmControlStateMode)0);
    return TRUE;
}

RVAPI int RVCALLCONV
cmCallStartConference(
              /* enter call conference mode */
              IN      HCALL               hsCall
              )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    int nodeId, message, res = RVERROR;
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    if (!hsCall || !hApp) return RVERROR;

    if (cmCallMasterSlaveStatus(hsCall) != cmMSMaster) return RVERROR; /* master only operation */

    cmiAPIEnter(hApp, "cmCallStartConference: hsCall=0x%lx", hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication), __h245(miscellaneousIndication));
        pvtAddBranch2(hVal,nodeId, __h245(type), __h245(multipointConference));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),1,NULL,NULL);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            ctrl->state=ctrlConference;
            cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConference,(cmControlStateMode)0);
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallStartConference=%d", res);
    return res;
}




RVAPI int RVCALLCONV
cmCallCancelConference(
               /* cancel the call conference mode */
               IN      HCALL               hsCall
               )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int nodeId, message, res = RVERROR;

    if (!hsCall || !hApp) return RVERROR;


    if (cmCallMasterSlaveStatus(hsCall) != cmMSMaster)
        return RVERROR; /* only master can cancel conference mode */

    cmiAPIEnter(hApp, "cmCallCancelConference: hsCall=0x%lx", hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        if (ctrl->state!=ctrlConference)
        {
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit(hApp, "cmCallCancelConference: [-1]. (Wrong state)");
            return RVERROR;
        }

        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication), __h245(miscellaneousIndication));
        pvtAddBranch2(hVal,nodeId, __h245(type), __h245(cancelMultipointConference));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),1,NULL,NULL);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            ctrl->state=ctrlConnected;
            cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConnected,(cmControlStateMode)0);
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallCancelConference=%d", res);
    return res;
}





/* When the conference becomes active, the master (MC) terminal shall call cmCallDeclareMC() to
   indicate the new conference status. */
RVAPI int RVCALLCONV
cmCallDeclareMC(
        /* declare this terminal to be the MC of the call */
        IN      HCALL               hsCall
        )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int nodeId, message, res = RVERROR;
    cmTransportAddress ta;

    if (!hsCall || !hApp) return RVERROR;

    if (cmCallMasterSlaveStatus(hsCall) != cmMSMaster) return RVERROR; /* only master can be active MC */

    cmiAPIEnter(hApp, "cmCallDeclareMC: hsCall=0x%lx", hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        cmGetLocalCallSignalAddress(hApp,&ta);
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(indication) _h245(mcLocationIndication)
                                               _h245(signalAddress) LAST_TOKEN}, 0, NULL);

        cmTAToVt_H245(hVal, nodeId, &ta);

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            ctrl->state=ctrlConference;
            cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConference,(cmControlStateMode)0);
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallDeclareMC=%d", res);
    return res;
}



int mcLocationIndication(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    cmVtToTA_H245(hVal,pvtGetChild(hVal,message,__h245(signalAddress),NULL), &(ctrl->mcLocationTA));
    cmiReportControl(cmiGetByControl((HCONTROL)ctrl),cmControlStateConference,(cmControlStateMode)0);
    ctrl->state=ctrlConference;
    return TRUE;
}

/* When the conference becomes active, this function provides the address of the Active MC. */
RVAPI
int RVCALLCONV cmCallGetMCAddress(
                /* get active MC address */
                IN      HCALL               hsCall,
                OUT     UINT32*             ip,
                OUT     UINT16*             port
                )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    if (!hsCall || !hApp) return RVERROR;

    if (((int)ctrl->mcLocationTA.distribution) < 0)
        return RVERROR;

    cmiAPIEnter(hApp, "cmCallGetMCAddress: hsCall=0x%lx, &ip=0x%lx, &port=0x%lx", hsCall, ip, port);

    if (emaLock((EMAElement)hsCall))
    {
        if (ip) *ip = ctrl->mcLocationTA.ip;
        if (port) *port = (UINT16)ctrl->mcLocationTA.port;
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallGetMCAddress: ip=0x%lx, port=%d [%d].", 0, (ip)?*ip:0, (port)?*port:-1);
    return 0;
}


/* This enumeration is used as the CHOICE values of the ConferenceIndication
   type in the H.245 ASN.1. The order of this enumeration should not be different
   than that of the choices in this type and should begin from 1. */
typedef enum
{
    sbeNumber = 1,
    terminalNumberAssign,
    terminalJoinedConference,
    terminalLeftConference,
    seenByAtLeastOneOther,
    cancelSeenByAtLeastOneOther,
    seenByAll,
    cancelSeenByAll,
    terminalYouAreSeeing,
    requestForFloor,
    withdrawChairToken,
    floorRequested,
    terminalYouAreSeeingInSubPictureNumber,
    videoIndicateCompose
} ctrlMPConfFlags;



int conferenceIndication(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    int childId=pvtChild(hVal,message);
    switch(pvtGetSyntaxIndex(hVal,childId))
    {
        case sbeNumber              :break;
        case terminalNumberAssign   :
        {
            INT32 tmp;
            pvtGetChildValue(hVal,childId,__h245(mcuNumber),&tmp,NULL);
            ctrl->myTermLabel.mcuNumber=(BYTE)tmp;
            pvtGetChildValue(hVal,childId,__h245(terminalNumber),&tmp,NULL);
            ctrl->myTermLabel.terminalNumber=(BYTE)tmp;
        }
        break;
        case terminalJoinedConference:break;
        case terminalLeftConference :break;
        case seenByAtLeastOneOther  :break;
        case cancelSeenByAtLeastOneOther:break;
        case seenByAll              :break;
        case cancelSeenByAll        :break;
        case terminalYouAreSeeing   :break;
        case requestForFloor        :break;
        case withdrawChairToken     :break;
        case floorRequested         :break;
        case terminalYouAreSeeingInSubPictureNumber:break;
        case videoIndicateCompose   :break;
    }
    return TRUE;
}




RVAPI int RVCALLCONV
cmCallSetTerminalLabel(
               /* Set the remote endpoint terminal label. */
               IN      HCALL               hsCall,
               IN      cmTerminalLabel*    terminalLabel
               )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int ret=0;
    int nodeId,message, res = RVERROR;
    int iMCU, iTerminal;

    if (!hsCall || !hApp) return RVERROR;

    if (!terminalLabel) return RVERROR;

    if (terminalLabel->mcuNumber>192) return RVERROR;
    if (terminalLabel->terminalNumber>192) return RVERROR;

    cmiAPIEnter(hApp, "cmCallSetTerminalLabel: hsCall=0x%lx label=(%d, %d)",
          hsCall, terminalLabel->mcuNumber, terminalLabel->terminalNumber);

    if (emaLock((EMAElement)hsCall))
    {
        if (cmCallMasterSlaveStatus(hsCall) == cmMSMaster)
        {
            /* master only operation */
            message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
            __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(indication) _h245(conferenceIndication)
                                                   _h245(terminalNumberAssign) LAST_TOKEN}, 0, NULL);

            iMCU = terminalLabel->mcuNumber;
            iTerminal = terminalLabel->terminalNumber;
            pvtAdd(hVal,nodeId,__h245(mcuNumber),iMCU,NULL,NULL);
            pvtAdd(hVal,nodeId,__h245(terminalNumber),iTerminal,NULL,NULL);

            res = sendMessageH245((HCONTROL)ctrl, message);
            pvtDelete(hVal,message);
        }
        else
            res = RVERROR;

        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmCallSetTerminalLabel: [%d]", ret);
    return ret;
}



RVAPI int RVCALLCONV /* RVERROR if terminal label is not defined for this terminal */
cmCallGetTerminalLabel(
               /* Get the local endpoint terminal label. */
               IN      HCALL               hsCall,
               OUT     cmTerminalLabel*    terminalLabel /* user allocated */
               )
{
    int ret=0;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);

    if (!hsCall || !hApp) return RVERROR;
    if (!terminalLabel) return RVERROR;

    cmiAPIEnter(hApp, "cmCallSetTerminalLabel: hsCall=0x%lx &label=%p", hsCall, terminalLabel);

    if (emaLock((EMAElement)hsCall))
    {
        if (cmCallMasterSlaveStatus(hsCall) == cmMSMaster)
        {
            terminalLabel->mcuNumber = 255;
            terminalLabel->terminalNumber = 255;
            ret = RVERROR;
        }
        else
        {
            *terminalLabel=ctrl->myTermLabel;
            if (terminalLabel->mcuNumber>192) ret=RVERROR;
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallSetTerminalLabel: label=(%d, %d) [%d].",terminalLabel->mcuNumber, terminalLabel->terminalNumber, ret);
    return ret;
}



static
int cmCallSetChannelsInternal(
                 /* As an Active MC, set transfer channels for remote terminal */
        IN      HCALL               hsCall,
        IN      int                 channelSetSize, /* number of elements in channelSet */
        IN      cmChannelStruct     channelSet[],
        IN      BOOL                useNewFields
        );





/* The master (MC) terminal shall use cmCallSetChannels() to force the remote terminal to open
   the indicated channels for transmit. */
RVAPI
int RVCALLCONV cmCallSetChannels(
                   /* As an Active MC, set transfer channels for remote terminal */
                   IN      HCALL               hsCall,
                   IN      int                 channelSetSize, /* number of elements in channelSet */
                   IN      cmChannelStruct     channelSet[]
                   )
{
    int ret;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    cmiAPIEnter(hApp, "cmCallSetChannels: hsCall=0x%lx channelSetSize=%d", hsCall, channelSetSize);
    ret = cmCallSetChannelsInternal(hsCall,channelSetSize,channelSet,FALSE);
    cmiAPIExit(hApp, "cmCallSetChannels: [%d].",ret);
    return ret;
}

RVAPI
int RVCALLCONV cmCallSetChannelsExt(
                   /* As an Active MC, set transfer channels for remote terminal */
                   IN      HCALL               hsCall,
                   IN      int                 channelSetSize, /* number of elements in channelSet */
                   IN      cmChannelStruct     channelSet[]
                   )
{
    int ret;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    cmiAPIEnter(hApp, "cmCallSetChannelsExt: hsCall=0x%lx channelSetSize=%d", hsCall, channelSetSize);
    ret = cmCallSetChannelsInternal(hsCall,channelSetSize,channelSet,TRUE);
    cmiAPIExit(hApp, "cmCallSetChannelsExt: [%d].",ret);
    return ret;
}

#define markSID(sids,sid) setBit((sids),(sid),1)
#define checkSID(sids,sid)  getBit((sids),(sid))

int markFreeSID(BYTE sids[32],cmCapDataType type)
{
    int sid;
    if (type)
        if (!checkSID(sids,type))
        {
            markSID(sids,type);
            return type;
        }
    sid=get1st0BitNumber(sids, 32, 255);
    if (sid<=255)
        return sid;
    return -1;

}

static
int cmCallSetChannelsInternal(
                 /* As an Active MC, set transfer channels for remote terminal */
        IN      HCALL               hsCall,
        IN      int                 channelSetSize, /* number of elements in channelSet */
        IN      cmChannelStruct     channelSet[],
        IN      BOOL                useNewFields
        )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HPVT hVal=cmGetValTree(hApp);
    int i, ret=TRUE;
    int message, tableId,  elemId=-1;
    BOOL dependencySet=TRUE;
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    BYTE sids[32];

    if (!hsCall || !hApp || channelSetSize<=0) return RVERROR;



    cmiAPIEnter(hApp, "cmCallSetChannels: hsCall=0x%lx, setSize=%d, &set=0x%lx", hsCall, channelSetSize, channelSet);

    if (emaLock((EMAElement)hsCall))
    {
        if (ctrl->state!=ctrlConference || !ctrl->isMaster)
        {
            emaUnlock((EMAElement)hsCall);
            cmiAPIExit(hApp, "cmCallSetChannelsExt: [-1]. (Wrong state)");
            return RVERROR;
        }

        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        __pvtBuildByFieldIds(tableId,hVal,message, {_h245(command) _h245(communicationModeCommand)
                                               _h245(communicationModeTable) LAST_TOKEN}, 0, NULL);

        memset(sids,0,sizeof(sids));
        for (i=0; i<channelSetSize; i++)
        {
            /*Go through all the incoming channels */
            /*Their SID must be preserved */
            channelElem*channel=(channelElem*)channelSet[i].hsChan;

            elemId=pvtAddBranch(hVal, tableId, -800);

            if (channel && !channel->origin && channel->sid!=0)
            {
                pvtAdd(hVal, elemId, __h245(sessionID), channel->sid, NULL, NULL);
                markSID(sids,channel->sid);
            }
        }
        elemId = pvtChild(hVal,tableId);
        for (i=0; i<channelSetSize; i++)
        {
            /*Now go through all the outgoing channels */
            /*Try to preserve their SID*/
            channelElem*channel=(channelElem*)channelSet[i].hsChan;

            if (channel && channel->origin && channel->sid!=0 && !checkSID(sids,channel->sid))
            {
                pvtAdd(hVal, elemId, __h245(sessionID), channel->sid, NULL, NULL);
                markSID(sids,channel->sid);
            }
            elemId = pvtBrother(hVal,elemId);
        }

        elemId = pvtChild(hVal,tableId);
        for (i=0; i<channelSetSize; i++)
        {
            channelElem*channel=(channelElem*)channelSet[i].hsChan;
            logPrint(((cmElem*)hApp)->log, RV_DEBUG,
                    (((cmElem*)hApp)->log, RV_DEBUG, "cmCallSetChannels: [%d] hs=0x%lx '%s'(%d) label: %d/%d desc:'%s' rtp=0x%lx:%d rtcp=0x%lx:%d.",
                            i, channelSet[i].hsChan,
                            nprn(channelSet[i].channelName), channelSet[i].channelDataTypeHandle,
                            channelSet[i].terminalLabel.mcuNumber, channelSet[i].terminalLabel.terminalNumber,
                            nprn(channelSet[i].sessionDescription),
                            channelSet[i].rtpAddress.ip, channelSet[i].rtpAddress.port,
                            channelSet[i].rtcpAddress.ip, channelSet[i].rtcpAddress.port));



            /* ADD dataType field */
            {
                int dataId = pvtAddBranch(hVal, elemId, __h245(dataType));
                if (channelSet[i].channelName != NULL || channelSet[i].channelDataTypeHandle >=0)
                { /* new type */
                    if (channelSet[i].channelName)
                    {
                        if (confGetDataType(hVal, ((cmElem*)hApp)->h245Conf, channelSet[i].channelName, dataId, NULL, TRUE) <0)
                        {
                            logPrint(((cmElem*)hApp)->logERR, RV_ERROR,
                                (((cmElem*)hApp)->logERR, RV_ERROR, "cmCallSetChannels: No configuration data type available for %s",nprn(channelSet[i].channelName)));
                            ret=-1;
                            break;
                        }
                    }
                    else
                    {
                        ret = pvtAddTree(hVal, dataId, hVal, (int)channelSet[i].channelDataTypeHandle);
                    }
                }
                else
                    if (channel && channel->dataTypeID>=0)
                    {
                        int nonH235DataTypeId=pvtGetChild2(hVal,channel->dataTypeID,__h245(h235Media) , __h245(mediaType));
                        pvtAddTree(hVal, dataId, hVal, pvtChild(hVal,(int)(nonH235DataTypeId>=0)?nonH235DataTypeId:channel->dataTypeID));
                    }
                    else
                    { /* dataType must be set in some way*/
                        logPrintFormat(((cmElem*)hApp)->logERR, RV_ERROR, "cmCallSetChannels: No data type available");
                        ret=-1;
                        break;
                    }

                /* Add sessionID  field in the cases it was impossible to determine it before*/
                if (pvtGetChild(hVal,elemId,__h245(sessionID),NULL)<=0)
                {
                    INTPTR fieldId;
                    cmCapDataType type=cmCapEmpty;
                    int sid;
                    pvtGet(hVal,pvtChild(hVal,dataId),&fieldId,NULL,NULL,NULL);
                    switch(fieldId)
                    {
                        case    __h245(audioData):  type=cmCapAudio;break;
                        case    __h245(videoData):  type=cmCapVideo;break;
                        case    __h245(data):       type=cmCapData; break;
                    }
                    sid=markFreeSID(sids,type);
                    pvtAdd(hVal, elemId, __h245(sessionID), sid, NULL, NULL);
                }
            }

            /* ADD redundancyEncoding field */
            if (useNewFields)
            {
                int redEncId, redEncodingId=RVERROR;
                if (channelSet[i].redEncodingId>=0)
                    redEncodingId=channelSet[i].redEncodingId;
                else
                    if (channel && channel->redEncID>=0)
                        redEncodingId=channel->redEncID;
                if (redEncodingId>=0)
                {
                    redEncId=pvtAddBranch(hVal, elemId, __h245(redundancyEncoding));
                    pvtSetTree(hVal,redEncId,hVal,redEncodingId);
                }
            }

            /* ADD associatedSessionID field */
            if (channelSet[i].hsChanAssociate)
            {
                channelElem* channel=(channelElem*) channelSet[i].hsChanAssociate;
                pvtAdd(hVal, elemId, __h245(associatedSessionID), channel->sid, NULL, NULL);
            }

            /* ADD terminalLabel field */
            if (channelSet[i].terminalLabel.mcuNumber <= 192 && channelSet[i].terminalLabel.terminalNumber <= 192)
            {
                int nid = pvtAdd(hVal, elemId, __h245(terminalLabel), 0, NULL, NULL);
                int iMCU = channelSet[i].terminalLabel.mcuNumber;
                int iTerminal = channelSet[i].terminalLabel.terminalNumber;
                pvtAdd(hVal, nid, __h245(mcuNumber), iMCU, NULL, NULL);
                ret=pvtAdd(hVal, nid, __h245(terminalNumber), iTerminal, NULL, NULL);
            }
            /* ADD sessionDescription field */
            {
                int l=0;
                char tmpBuf[258]={0,0};
                if (channelSet[i].sessionDescription != NULL && channelSet[i].sessionDescriptionLength>0)
                    l=chrn2bmp(channelSet[i].sessionDescription, min(channelSet[i].sessionDescriptionLength, 128), (BYTE*)tmpBuf);
                if (l<2) l=2;
                ret=pvtAdd(hVal, elemId, __h245(sessionDescription), l, tmpBuf, NULL);
            }

            /* ADD sessionDependency field, if possible */
            if (channelSet[i].dependency>=0) dependencySet=TRUE;
            else if (channel && channel->base)
                ret=pvtAdd(hVal, elemId, __h245(sessionDependency), channel->base->sid, NULL, NULL);

            /* ADD mediaControlChannel field */
            if (channelSet[i].rtcpAddress.ip !=0 || channelSet[i].rtcpAddress.port != 0)
            {
                int tmpId = pvtAdd(hVal, elemId, __h245(mediaControlChannel), 0, NULL, NULL);
                getGoodAddressForCtrl((HCONTROL)ctrl,&channelSet[i].rtcpAddress);
                cmTAToVt_H245(hVal,tmpId,&channelSet[i].rtcpAddress);
            }
            else if (channel)
        {
                    if (channel->partner && channel->partner->sendRtcpAddressID>=0)
                    {
                        int tmpId = pvtAdd(hVal, elemId, __h245(mediaControlChannel), 0, NULL, NULL);
                        pvtSetTree(hVal,tmpId,hVal,channel->partner->sendRtcpAddressID);
                    }
                    else if (channel->sendRtcpAddressID>=0)
                    {
                        int tmpId = pvtAdd(hVal, elemId, __h245(mediaControlChannel), 0, NULL, NULL);
                        pvtSetTree(hVal,tmpId,hVal,channel->sendRtcpAddressID);
                    }
        }


            /* ADD mediaChannel field */
            if (channelSet[i].rtpAddress.ip !=0 || channelSet[i].rtpAddress.port != 0)
            {
                int tmpId = pvtAdd(hVal, elemId, __h245(mediaChannel), 0, NULL, NULL);
                getGoodAddressForCtrl((HCONTROL)ctrl,&channelSet[i].rtpAddress);
                cmTAToVt_H245(hVal,tmpId,&channelSet[i].rtpAddress);
            }
            else if (channel)
        {
                    if (channel->partner && channel->partner->sendRtpAddressID>=0)
                    {
                        int tmpId = pvtAdd(hVal, elemId, __h245(mediaChannel), 0, NULL, NULL);
                        pvtSetTree(hVal,tmpId,hVal,channel->partner->sendRtpAddressID);
                    }
                    else if (channel->sendRtpAddressID>=0)
                    {
                        int tmpId = pvtAdd(hVal, elemId, __h245(mediaChannel), 0, NULL, NULL);
                        pvtSetTree(hVal,tmpId,hVal,channel->sendRtpAddressID);
                    }
        }
            elemId = pvtBrother(hVal,elemId);
        }
        if (ret>=0)
        {
            /* ADD sessionDependency field */
            if (useNewFields && dependencySet)
            {
                int i,j, childNodeId_I,childNodeId_J;
				INT32 sid;
                childNodeId_I=pvtChild(hVal,tableId);
                for (i=0; i<channelSetSize; i++)
                {
                    childNodeId_J=pvtChild(hVal,tableId);
                    for (j=0; j<channelSetSize; j++)
                    {
                        if (channelSet[i].dependency==channelSet[j].uid)
                        {
                            pvtGetChildValue(hVal,childNodeId_J,__h245(sessionID),&sid,NULL);
                            ret=pvtAdd(hVal, childNodeId_J, __h245(sessionDependency), channelSet[i].dependency, NULL, NULL);
                        }
                        childNodeId_J=pvtBrother(hVal,childNodeId_J);
                    }
                    childNodeId_I=pvtBrother(hVal,childNodeId_I);
                }
            }

            ret = sendMessageH245((HCONTROL)ctrl, message);
        }
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallSetChannelsExt: [%d].", ret);
    return ret;
}


int communicationModeCommand(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    void* ptr=NULL;
    channelElem* channel;
    int tableNodeId,elemNodeId;

    if (ctrl->isMaster) return TRUE;

    tableNodeId = pvtGetChild(hVal,message, __h245(communicationModeTable), NULL);

    /* go over all outgoing channels */
    while((channel=getNextOutChan((HCONTROL)ctrl, &ptr)))
    {
        if (emaLock((EMAElement)channel))
        {
            /* try to match each channel to an entry in the communication mode table */
            elemNodeId = pvtChild(hVal,tableNodeId); /* elemNodeId = communicationModeTable.1 */
            while(elemNodeId>=0)
            {
                /* first try to match Session ID */
                INT32 sid;
                pvtGetChildValue(hVal, elemNodeId, __h245(sessionID), &sid, NULL);
                if (sid==channel->sid)
                {
                    /* now see if the data type is identical */
                    int dataTypeId=pvtGetChild(hVal, elemNodeId, __h245(dataType), NULL);
                    if (pvtCompareTree(hVal, pvtChild(hVal,dataTypeId), hVal, pvtChild(hVal,channel->dataTypeID))==TRUE)
                    {
                        /* and make seure the RTCP address is identical */
                        int mediaControlChannelId=pvtGetChild(hVal, elemNodeId, __h245(mediaControlChannel), NULL);
                        if (pvtCompareTree(hVal, mediaControlChannelId, hVal, channel->recvRtcpAddressID)==TRUE)
                        {
                            /* and the redundancy encoding */
                            int redundancyEncodingId=pvtGetChild(hVal, elemNodeId, __h245(redundancyEncoding), NULL);
                            if (pvtCompareTree(hVal, redundancyEncodingId, hVal, channel->redEncID)==TRUE)
                            {
                                /* and lastly, make sure the session dependency is the same */
                                INT32 sessionDependency;
                                int sdPresent=(pvtGetChildValue(hVal, elemNodeId, __h245(sessionDependency), &sessionDependency, NULL)>=0);
                                if (!sdPresent && !channel->base)
                                    /* Found */
                                    break;
                                if (channel->base && channel->base->sid==sessionDependency)
                                    /* Found */
                                    break;
                            }
                        }
                    }
                }
                elemNodeId = pvtBrother(hVal,elemNodeId); /* elemNodeId = communicationModeTable.(i++) */
            }

            if (elemNodeId<0)/* Not Found */
            {
                /* Drop the channel that is not in the table of the communicationModeCommand */
                cmChannelDrop((HCHAN)channel);
            }
            emaUnlock((EMAElement)channel);
        }
    }

    elemNodeId = pvtChild(hVal,tableNodeId); /* elemNodeId = communicationModeTable.1 */
    while(elemNodeId>=0)
    {
        INT32 sid;
        int dataTypeId;
        int terminalLabelId;
        int mediaControlChannelId,mediaChannelId;
        int redundancyEncodingId;
        INT32 sessionDependency=-1;

        cmTerminalLabel termLabel={0,0};

        /* get the terminal lable the current table entry is reffering to, if present */
        if ((terminalLabelId=pvtGetChild(hVal, elemNodeId, __h245(terminalLabel), NULL))>=0)
        {
            INT32 tmp;
            pvtGetChildValue(hVal, terminalLabelId, __h245(mcuNumber), &tmp, NULL);
            termLabel.mcuNumber=(BYTE)tmp;
            pvtGetChildValue(hVal, terminalLabelId, __h245(terminalNumber), &tmp, NULL);
            termLabel.terminalNumber=(BYTE)tmp;
        }

        /* also get the session ID, data type, RTCP, redundancy encoding and dependency */
        pvtGetChildValue(hVal, elemNodeId, __h245(sessionID), &sid, NULL);
        dataTypeId=pvtGetChild(hVal, elemNodeId, __h245(dataType), NULL);
        mediaControlChannelId=pvtGetChild(hVal, elemNodeId, __h245(mediaControlChannel), NULL);
        redundancyEncodingId=pvtGetChild(hVal, elemNodeId, __h245(redundancyEncoding), NULL);
        pvtGetChildValue(hVal, elemNodeId, __h245(sessionDependency), &sessionDependency, NULL);

        /* see if this entry refers to this terminal */
        if (terminalLabelId<0 || (termLabel.mcuNumber==ctrl->myTermLabel.mcuNumber && termLabel.terminalNumber==ctrl->myTermLabel.terminalNumber))
        {
            /* try to find the channel it refers to */
            ptr=NULL;
            while((channel=getNextOutChan((HCONTROL)ctrl, &ptr)))
            {
                if (sid==channel->sid)
                    if (pvtCompareTree(hVal, pvtChild(hVal,dataTypeId), hVal, pvtChild(hVal,channel->dataTypeID))==TRUE)
                        if (pvtCompareTree(hVal, mediaControlChannelId, hVal, channel->recvRtcpAddressID)==TRUE)
                            if (pvtCompareTree(hVal, redundancyEncodingId, hVal, channel->redEncID)==TRUE)
                                if ((sessionDependency==-1 && !channel->base) ||
                                    (channel->base && channel->base->sid==sessionDependency))
                                    /* Found */
                                    break;
            }
            if (channel==NULL) /* Not Found */
            {
                channelElem* newChannel=allocateChannel((HCONTROL)ctrl);
                
                if (!newChannel)
                {
                    logPrint(((cmElem*)hApp)->logERR, RV_ERROR,
                        (((cmElem*)hApp)->logERR, RV_ERROR, "communicationModeCommand: Unable to allocate channel"));
                    return TRUE;
                }
                
                if(emaLock((EMAElement)newChannel))
                {
                    newChannel->ctrl=(HCONTROL)ctrl;
                    newChannel->origin=TRUE;
                    
                    if (((cmElem*)hApp)->cmMySessionEvent.cmEvCallNewChannel)
                    {
                        cmiCBEnter(hApp, "cmEvCallNewChannel:IN: haCall=0x%lx, hsCall=0x%lx. hsChan=0x%lx", emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl),newChannel);
                        {
                            HAPPCHAN haChan=NULL;
                            int nesting = emaPrepareForCallback((EMAElement)newChannel);
                            ((cmElem*)hApp)->cmMySessionEvent.cmEvCallNewChannel((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),(HCHAN)newChannel, &haChan);
                            emaReturnFromCallback((EMAElement)newChannel, nesting);
                            emaSetApplicationHandle((EMAElement)newChannel,(void*)haChan);
                        }
                        cmiCBExit(hApp, "cmEvCallNewChannel:IN: haChan=0x%lx.", emaGetApplicationHandle((EMAElement)newChannel));
                    }
                    if (emaWasDeleted((EMAElement)newChannel))
                        continue;
                }

                if (dataTypeId>=0)
                {
                    confDataType type;

                    newChannel->dataTypeID=pvtAddRoot(hVal,NULL,0,NULL);
                    pvtMoveTree(hVal,newChannel->dataTypeID,dataTypeId);

                    cmcCallChannelParametersCallback(hApp, (HCHAN)newChannel, newChannel->dataTypeID, &type);
                    if (!emaWasDeleted((EMAElement)newChannel))
                        cmcCallDataTypeHandleCallback(hApp, (HCHAN)newChannel, newChannel->dataTypeID, type);
                    if (emaWasDeleted((EMAElement)newChannel))
                        continue;
                }
                else
                {
                    logPrint(((cmElem*)hApp)->logERR, RV_ERROR,
                        (((cmElem*)hApp)->logERR, RV_ERROR, "communicationModeCommand: no data type."));
                    newChannel->state = released;
                    notifyChannelState(newChannel,cmChannelStateDisconnected,cmChannelStateModeDisconnectedReasonUnknown);
                    notifyChannelState(newChannel,cmChannelStateIdle, cmChannelStateModeOff);
                    continue;
                }

                if (mediaControlChannelId>=0)
                {
                    newChannel->recvRtcpAddressID=pvtAddRoot(hVal,NULL,0,NULL);
                    pvtMoveTree(hVal,newChannel->recvRtcpAddressID,mediaControlChannelId);
                }

                mediaChannelId=pvtGetChild(hVal, elemNodeId, __h245(mediaChannel), NULL);
                if (mediaChannelId>=0)
                {
                    newChannel->recvRtpAddressID=pvtAddRoot(hVal,NULL,0,NULL);
                    pvtMoveTree(hVal,newChannel->recvRtpAddressID,mediaChannelId);
                }

                if (redundancyEncodingId>=0)
                {
                    newChannel->redEncID=pvtAddRoot(hVal,NULL,0,NULL);
                    pvtMoveTree(hVal,newChannel->redEncID,redundancyEncodingId);
                }

                if (sessionDependency>=0)
                {
                    int elemNodeId1 = pvtChild(hVal,tableNodeId); /* elemNodeId1 = communicationModeTable.1 */
                    while(elemNodeId1>=0)
                    {
                        INT32 sid1;
                        pvtGetChildValue(hVal, elemNodeId1, __h245(sessionID), &sid1, NULL);
                        if (sid1==sessionDependency) newChannel->base=getOutChanBySID((HCONTROL)ctrl,sid1);
                        elemNodeId1 = pvtBrother(hVal,elemNodeId1); /* elemNodeId = communicationModeTable.(i++) */
                    }
                }
                newChannel->sid=sid;

                startEstablishment((cmElem*)hApp,newChannel);
                emaUnlock((EMAElement)newChannel);
            }
        }
        elemNodeId = pvtBrother(hVal,elemNodeId); /* elemNodeId = communicationModeTable.(i++) */
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif
