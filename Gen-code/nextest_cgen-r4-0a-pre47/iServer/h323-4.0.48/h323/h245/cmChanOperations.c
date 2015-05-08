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
#include <ms.h>
#include <conf.h>
#include <caputils.h>
#include <netutl.h>
#include <cmchan.h>
#include <h245.h>
#include <cmutils.h>
#include <cmChanGetByXXX.h>
#include <transpcap.h>
#include <cmdebprn.h>
#include <cmCrossReference.h>

void RVCALLCONV channelML_TimeoutEventsHandler(IN void* hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message;
    cmElem*app=(cmElem*)hApp;
    int nesting;

    if (!hApp)   return;

    if (emaLock((EMAElement)hsChan))
    {
        cmTimerReset(hApp,&(channel->ml_timer));

        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        pvtAddBranch2(hVal,message, __h245(command),__h245(maintenanceLoopOffCommand));

        sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);

        cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE(app->cmMyChannelEvent.cmEvChannelMediaLoopStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopOff);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");

        emaUnlock((EMAElement)hsChan);
    }
}

int flowControlCommand(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INT32 rate;
	int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelNewRate != NULL)
    {
        hVal=app->hVal;

        pvtGetChildValue2(hVal,message,__h245(scope),__h245(logicalChannelNumber),&lcn,NULL);
        channel=getOutSubChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            pvtGetChildValue2(hVal,message,__h245(restriction),__h245(maximumBitRate),&rate,NULL);
            cmiCBEnter((HAPP)app, "cmEvChannelNewRate: haCall=0x%p, hsCall=0x%p, rate=%d.", emaGetApplicationHandle((EMAElement)channel), channel, rate);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelNewRate(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, rate);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelNewRate.");
    
            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}


int maintenanceLoopRequest(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMediaLoopStatus != NULL)
    {
        hVal=app->hVal;

        pvtGetChildValue2(hVal,message,__h245(type),__h245(mediaLoop),&lcn,NULL);
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);
            
            cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=request.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMediaLoopStatus(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopRequest);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int maintenanceLoopAck(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;

    hVal=app->hVal;

    pvtGetChildValue2(hVal,message,__h245(type),__h245(mediaLoop),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

    if (emaLock((EMAElement)channel))
    {
        cmTimerReset((HAPP)app, &channel->ml_timer);

        incomingChannelMessage((HAPP)app, channel, base);
        
        if (app->cmMyChannelEvent.cmEvChannelMediaLoopStatus != NULL)
        {
            cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMediaLoopStatus(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopConfirm);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
        }

        emaUnlock((EMAElement)channel);
    }

    return TRUE;
}

int maintenanceLoopReject(controlElem*ctrl, int base, int message)
{
    INT32 lcn;
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;

    hVal=app->hVal;

    pvtGetChildValue2(hVal,message,__h245(type),__h245(mediaLoop),&lcn,NULL);
    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

    if (emaLock((EMAElement)channel))
    {
        cmTimerReset((HAPP)app, &channel->ml_timer);

        incomingChannelMessage((HAPP)app, channel, base);
        
        if (app->cmMyChannelEvent.cmEvChannelMediaLoopStatus != NULL)
        {
            cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=off.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMediaLoopStatus(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopOff);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
        }

        emaUnlock((EMAElement)channel);
    }

    return TRUE;
}

int maintenanceLoopOffCommand(controlElem*ctrl, int message)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    void* ptr=NULL;
    channelElem* channel;
    int nesting;

    if (message);

    if (app->cmMyChannelEvent.cmEvChannelMediaLoopStatus != NULL)
    {
        while ((channel=getNextInChan((HCONTROL)ctrl,&ptr)))
        {
            if (emaLock((EMAElement)channel))
            {
                cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
                nesting = emaPrepareForCallback((EMAElement)channel);
                app->cmMyChannelEvent.cmEvChannelMediaLoopStatus(
                    (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopOff);
                emaReturnFromCallback((EMAElement)channel, nesting);
                cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");

                emaUnlock((EMAElement)channel);
            }
        }
    }
    return TRUE;
}


int videoFreezePicture(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMiscCommand != NULL)
    {
        hVal=app->hVal;
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmVideoFreezePicture);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMiscCommand(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmVideoFreezePicture);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int videoSendSyncEveryGOB(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMiscCommand != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmVideoSendSyncEveryGOB);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMiscCommand(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmVideoSendSyncEveryGOB);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int videoSendSyncEveryGOBCancel(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMiscCommand != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmVideoSendSyncEveryGOBCancel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMiscCommand(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmVideoSendSyncEveryGOBCancel);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int switchReceiveMediaOff(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMiscCommand != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmSwitchReceiveMediaOff);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMiscCommand((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmSwitchReceiveMediaOff);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int switchReceiveMediaOn(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMiscCommand != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmSwitchReceiveMediaOn);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMiscCommand((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmSwitchReceiveMediaOn);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");

            emaUnlock((EMAElement)channel);
        }          
    }

    return TRUE;
}


int videoTemporalSpatialTradeOff(controlElem* ctrl, int base, int message, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INT32 value;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelTSTO != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            pvtGet(app->hVal, message, NULL, NULL, &value, NULL);

            cmiCBEnter((HAPP)app, "cmEvChannelTSTO: haChan=0x%p, hsChan=0x%p, isCommand=1 (command), value=%d",
                   emaGetApplicationHandle((EMAElement)channel), channel, value);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelTSTO(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, 1, (INT8)value);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelTSTO.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int videoTemporalSpatialTradeOffInd(controlElem* ctrl, int base, int message, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INT32 value;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelTSTO != NULL)
    {
        hVal=app->hVal;
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            pvtGet(app->hVal, message, NULL, NULL, &value, NULL);

            cmiCBEnter((HAPP)app, "cmEvChannelTSTO: haChan=0x%p, hsChan=0x%p, isCommand=0 (indication), value=%d",
                   emaGetApplicationHandle((EMAElement)channel), channel, value);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelTSTO(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, 0, (INT8)value);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelTSTO.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}


int videoFastUpdatePicture(controlElem* ctrl, int base, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;
    hVal=app->hVal;

    channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

    if (emaLock((EMAElement)channel))
    {
        incomingChannelMessage((HAPP)app, channel, base);

        cmiCBEnter((HAPP)app, "cmEvChannelVideoFastUpdatePicture: haCall=0x%p, hsCall=0x%p.",
                emaGetApplicationHandle((EMAElement)channel), channel);
        nesting = emaPrepareForCallback((EMAElement)channel);
        ifE (app->cmMyChannelEvent.cmEvChannelVideoFastUpdatePicture)
          ((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel);
        emaReturnFromCallback((EMAElement)channel, nesting);
        cmiCBExit((HAPP)app, "cmEvChannelVideoFastUpdatePicture.");

        if (!emaWasDeleted((EMAElement)channel))
        {
            cmiCBEnter((HAPP)app, "cmEvChannelMiscCommand: haChan=0x%p, hsChan=0x%p,  type=%d",   emaGetApplicationHandle((EMAElement)channel), channel, cmVideoFastUpdatePicture);
            nesting = emaPrepareForCallback((EMAElement)channel);
            ifE (app->cmMyChannelEvent.cmEvChannelMiscCommand)    ((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmVideoFastUpdatePicture);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMiscCommand.");
        }

        emaUnlock((EMAElement)channel);
    }

    return TRUE;
}

int videoFastUpdateGOB(controlElem* ctrl, int base, int message, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INT32 firstGOB, numOfGOBS;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelVideoFastUpdateGOB != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            pvtGetChildValue(hVal, message, __h245(firstGOB), &firstGOB, NULL);
            pvtGetChildValue(hVal,  message, __h245(numberOfGOBs), &numOfGOBS, NULL);

            cmiCBEnter((HAPP)app, "cmEvChannelVideoFastUpdateGOB: haCall=0x%p, hsCall=0x%p, firstGOB=%d, #GOBS=%d.",
                   emaGetApplicationHandle((EMAElement)channel), channel, firstGOB, numOfGOBS);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelVideoFastUpdateGOB(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, (int)firstGOB, (int)numOfGOBS);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelVideoFastUpdateGOB.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int videoFastUpdateMB(controlElem* ctrl, int base, int message, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    INT32 firstGOB, firstMB, numOfMBS;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelVideoFastUpdateMB != NULL)
    {
        hVal=app->hVal;
        channel=getOutChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            pvtGetChildValue(hVal, message, __h245(firstGOB), &firstGOB, NULL);
            pvtGetChildValue(hVal,  message, __h245(firstMB), &firstMB, NULL);
            pvtGetChildValue(hVal,  message, __h245(numberOfMBs), &numOfMBS, NULL);
            cmiCBEnter((HAPP)app, "cmEvChannelVideoFastUpdateMB: haCall=0x%p, hsCall=0x%p, firstGOB=%d, firstMB=%d, #MS=%d.",
                   emaGetApplicationHandle((EMAElement)channel), channel, firstGOB, firstMB, numOfMBS);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelVideoFastUpdateMB(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, (int)firstGOB, (int)firstMB, (int)numOfMBS);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelVideoFastUpdateMB.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int h2250MaximumSkewIndication(controlElem* ctrl, int message)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel1,* channel2;
    INT32 lcn1, lcn2, maxSkew;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelMaxSkew != NULL)
    {
        EMAElement callE = (EMAElement)cmiGetByControl((HCONTROL)ctrl);

        hVal=app->hVal;

        if (emaLock(callE))
        {
            pvtGetChildValue(hVal, message, __h245(logicalChannelNumber1), &lcn1, NULL);
            pvtGetChildValue(hVal, message, __h245(logicalChannelNumber2), &lcn2, NULL);
            pvtGetChildValue(hVal, message, __h245(maximumSkew), &maxSkew, NULL);

            channel1=getInChanByLCN((HCONTROL)ctrl,lcn1);
            channel2=getInChanByLCN((HCONTROL)ctrl,lcn2);

            cmiCBEnter((HAPP)app, "cmEvChannelMaxSkew: haCall=0x%p, hsCall=0x%p, chan1=0x%p, chan2=0x%p, skew=%d.",
             emaGetApplicationHandle((EMAElement)channel1), channel1, emaGetApplicationHandle((EMAElement)channel2), channel2, maxSkew);

            nesting = emaPrepareForCallback(callE);
            app->cmMyChannelEvent.cmEvChannelMaxSkew(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel1), (HCHAN)channel1,
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel2), (HCHAN)channel2, maxSkew);
            emaReturnFromCallback(callE, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMaxSkew.");

            emaUnlock(callE);
        }
    }
    return 0;
}

int logicalChannelActive(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelStateChanged != NULL)
    {
        hVal=app->hVal;
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelStateChanged: haChan=0x%p, hsChan=0x%p, state=Connected, mode=On", emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelStateChanged(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmChannelStateConnected, cmChannelStateModeOn);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelStateChanged.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int logicalChannelInactive(controlElem* ctrl, int base, int lcn)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    HPVT hVal;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelStateChanged)
    {
        hVal=app->hVal;
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelStateChanged: haChan=0x%p, hsChan=0x%p, state=Connected, mode=Off", emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelStateChanged(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmChannelStateConnected, cmChannelStateModeOff);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelStateChanged.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}

int transportCapability(controlElem* ctrl, int base, int message, int lcn)
{
    HPVT hVal;
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    channelElem* channel=NULL;
    int nesting;

    if (!app) return RVERROR;

    if (app->cmMyChannelEvent.cmEvChannelTransportCapInd != NULL)
    {
        hVal=app->hVal;
        channel=getInChanByLCN((HCONTROL)ctrl,lcn);

        if (emaLock((EMAElement)channel))
        {
            incomingChannelMessage((HAPP)app, channel, base);

            cmiCBEnter((HAPP)app, "cmEvChannelTransportCapInd: haChan=0x%p, hsChan=0x%p,  (indication), nodeId=%d",
              emaGetApplicationHandle((EMAElement)channel), channel, message);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelTransportCapInd(
                (HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, message);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelTransportCapInd.");

            emaUnlock((EMAElement)channel);
        }
    }

    return TRUE;
}


RVAPI int RVCALLCONV
cmChannelOn(
        IN  HCHAN       hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message,nodeId;
    cmElem*app=(cmElem*)hApp;
    int ret = RVERROR;

    if (!hApp)
        return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelOn: hsChan=0x%p.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->origin)
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(miscellaneousIndication));

            pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
            pvtAddBranch2(hVal,nodeId,__h245(type),__h245(logicalChannelActive));

            ret = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);
        }
        else
            ret = RVERROR;
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelOn: [%d].", ret);
    return ret;
}



RVAPI int RVCALLCONV
cmChannelOff(
         IN     HCHAN       hsChan)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message,nodeId;
    cmElem*app=(cmElem*)hApp;
    int ret = RVERROR;

    if (!hApp)
        return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelOff: hsChan=0x%p.", hsChan);
    if (emaLock((EMAElement)hsChan))
    {
        if (channel->origin)
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(miscellaneousIndication));

            pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
            pvtAddBranch2(hVal,nodeId,__h245(type),__h245(logicalChannelInactive));

            ret = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);
        }
        else
            ret = RVERROR;
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelOff: [%d].", ret);
    return ret;
}

RVAPI int RVCALLCONV
cmChannelFlowControl(
             IN     HCHAN       hsChan,
             IN     UINT32      rate)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelFlowControl: hsChan=0x%p, rate=%d.", hsChan, rate);
    if (emaLock((EMAElement)hsChan))
    {
        if (channel->origin && !channel->isDuplex)
        {
            emaUnlock((EMAElement)hsChan);
            cmiAPIExit((HAPP)app, "cmChannelFlowControl: [Invalid circumstances].");
            return RVERROR;
        }
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command),__h245(flowControlCommand));

        pvtAdd(hVal,pvtAddBranch(hVal,nodeId,__h245(restriction)),__h245(maximumBitRate),(INT32)rate,NULL,NULL);
        pvtAdd(hVal,pvtAddBranch(hVal,nodeId,__h245(scope)),__h245(logicalChannelNumber), (channel->origin)?channel->rvrsLCN:channel->myLCN,NULL,NULL);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelFlowControl=%d", res);
    return res;
}

RVAPI int RVCALLCONV
cmChannelVideoFastUpdatePicture(
                IN  HCHAN           hsChan)
{
    return cmChannelSendMiscCommand(hsChan,cmVideoFastUpdatePicture);
}


RVAPI int RVCALLCONV
cmChannelVideoFastUpdateGOB(
                IN  HCHAN           hsChan,
                IN  int             firstGOB,
                IN  int             numberOfGOBs)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, id, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelVideoFastUpdateGOB: hsChan=0x%p, firstGOB=%d, numberOfGOBs=%d.",hsChan, firstGOB, numberOfGOBs);
    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command),__h245(miscellaneousCommand));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
        id=pvtAddBranch2(hVal,nodeId,__h245(type),__h245(videoFastUpdateGOB));
        pvtAdd(app->hVal, id, __h245(firstGOB), firstGOB, NULL, NULL);
        pvtAdd(app->hVal, id, __h245(numberOfGOBs), numberOfGOBs, NULL, NULL);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelVideoFastUpdateGOB=%d", res);
    return res;
}


RVAPI int RVCALLCONV
cmChannelVideoFastUpdateMB(
               IN   HCHAN           hsChan,
               IN   int             firstGOB,
               IN   int             firstMB,
               IN   int             numberOfMBs)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, id, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelVideoFastUpdateMB: hsChan=0x%p, firstGOB=%d, firstMB=%d, numberOfMBs=%d.",
          hsChan, firstGOB, firstMB, numberOfMBs);

    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command),__h245(miscellaneousCommand));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
        id=pvtAddBranch2(hVal,nodeId,__h245(type),__h245(videoFastUpdateMB));
        pvtAdd(app->hVal, id, __h245(firstGOB), firstGOB, NULL, NULL);
        pvtAdd(app->hVal, id, __h245(firstMB), firstMB, NULL, NULL);
        pvtAdd(app->hVal, id, __h245(numberOfMBs), numberOfMBs, NULL, NULL);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelVideoFastUpdateMB=%d", res);
    return res;
}






RVAPI int RVCALLCONV
cmChannelMaxSkew(
         IN     HCHAN           hsChan1,
         IN     HCHAN           hsChan2,
         IN     UINT32          skew)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan1);
    channelElem* channel1=(channelElem*)hsChan1;
    channelElem* channel2=(channelElem*)hsChan2;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelMaxSkew: hsChan1=0x%p, hsChan2=0x%p, skew=%d.", hsChan1, hsChan2, skew);

    if (emaLock((EMAElement)hsChan1))
    {
        if (emaLock((EMAElement)hsChan2))
        {
            if (!channel1->origin || !channel2->origin)
            {
              emaUnlock((EMAElement)hsChan2);
              emaUnlock((EMAElement)hsChan1);
              cmiAPIExit((HAPP)app, "cmChannelMaxSkew: [Invalid Circumstances].");
              return RVERROR;
            }
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(h2250MaximumSkewIndication));
            pvtAdd(hVal,nodeId,__h245(logicalChannelNumber1),channel1->myLCN,NULL,NULL);
            pvtAdd(hVal,nodeId,__h245(logicalChannelNumber2),channel2->myLCN,NULL,NULL);
            pvtAdd(hVal,nodeId,__h245(maximumSkew),(INT32)skew,NULL,NULL);

            res = sendMessageH245(channel1->ctrl, message);
            pvtDelete(hVal,message);
            emaUnlock((EMAElement)hsChan2);
        }
        emaUnlock((EMAElement)hsChan1);
    }


    cmiAPIExit((HAPP)app, "cmChannelMaxSkew=%d", res);
    return res;
}

/*   temporal spatial trade off__________________________________________________________*/



RVAPI int RVCALLCONV
cmChannelTSTOCommand(
             /* Send temporal spatial trade off command. Request the remote terminal to change
              the tradeoff. */
             IN HCHAN hsChan, /* incoming channel */
             IN int tradeoffValue /* 0-31 */
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hsChan || tradeoffValue<0 || tradeoffValue>31) return RVERROR;
    if (!hApp)   return RVERROR;

    /* Maybe I need to check the remote terminal capabilities? */

    cmiAPIEnter((HAPP)app, "cmChannelTSTOCommand: hsChan=0x%p, tradeoff=%d.", hsChan, tradeoffValue);
    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command),__h245(miscellaneousCommand));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
        pvtAdd(hVal,pvtAddBranch(hVal,nodeId,__h245(type)),__h245(videoTemporalSpatialTradeOff),tradeoffValue,NULL,NULL);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelTSTOCommand=%d", res);
    return res;
}



RVAPI int RVCALLCONV
cmChannelTSTOIndication(
             /* Send temporal spatial trade off indication. Indicates the current tradeoff value
              of the local terminal. */
             IN HCHAN hsChan, /* outgoing channel */
             IN int tradeoffValue /* 0-31 */
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hsChan || tradeoffValue<0 || tradeoffValue>31) return RVERROR;
    if (!hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelTSTOIndication: hsChan=0x%p, tradeoff=%d.", hsChan, tradeoffValue);

    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(miscellaneousIndication));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
        pvtAdd(hVal,pvtAddBranch(hVal,nodeId,__h245(type)),__h245(videoTemporalSpatialTradeOff),tradeoffValue,NULL,NULL);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelTSTOIndication=%d");
    return res;
}

/* Media Loop_____________________________________________________________________________*/




RVAPI int RVCALLCONV
cmChannelMediaLoopRequest(
              /* Request media loop on this channel */
              IN HCHAN hsChan /* outgoing channel */
              )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId;
    int ret = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hsChan || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelMediaLoopRequest: hsChan=0x%p.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        if (channel->origin)
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(request) _h245(maintenanceLoopRequest) _h245(type) _h245(mediaLoop) LAST_TOKEN},
                                 channel->myLCN,NULL);

            ret = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);
            {
                INT32 timeout=10;
                pvtGetChildValue(app->hVal,app->h245Conf,__h245(mediaLoopTimeout),&(timeout),NULL);
                cmTimerReset(hApp,&(channel->ml_timer));
                channel->ml_timer=cmTimerSet(hApp,channelML_TimeoutEventsHandler,(void*)channel,timeout*1000);
            }
        }
        else
        {
            /* This operation is valid only for outgoing channels */
            ret = RVERROR;
        }
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelMediaLoopRequest: [%d].", ret);
    return ret;
}



RVAPI int RVCALLCONV
cmChannelMediaLoopConfirm(
              /* Confirm media loop request on this channel */
              IN HCHAN hsChan /* incoming channel */
              )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;
    int nesting;

    if (!hsChan || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelMediaLoopConfirm: hsChan=0x%p.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        if (app->cmMyChannelEvent.cmEvChannelMediaLoopStatus)
        {
            cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            app->cmMyChannelEvent.cmEvChannelMediaLoopStatus((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopConfirm);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
        }
        if (!emaWasDeleted((EMAElement)channel))
        {
            message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
            __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(maintenanceLoopAck) _h245(type) _h245(mediaLoop) LAST_TOKEN},
                                 channel->myLCN,NULL);

            res = sendMessageH245Chan(channel->ctrl, hsChan, message);
            pvtDelete(hVal,message);
        }
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelMediaLoopConfirm=%d", res);
    return res;
}


RVAPI int RVCALLCONV
cmChannelMediaLoopReject(
             /* Reject media loop request on this channel */
             IN HCHAN hsChan /* incoming channel */
             )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;
    int nesting;

    if (!hsChan || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelMediaLoopReject: hsChan=0x%p.", hsChan);

    if (emaLock((EMAElement)hsChan))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(response), __h245(maintenanceLoopReject));
        pvtAdd(hVal,pvtAddBranch(hVal,nodeId,__h245(type)), __h245(mediaLoop),channel->myLCN,NULL,NULL);
        pvtAddBranch2(hVal,nodeId,__h245(cause), __h245(canNotPerformLoop));

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
            nesting = emaPrepareForCallback((EMAElement)channel);
            ifE(app->cmMyChannelEvent.cmEvChannelMediaLoopStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopOff);
            emaReturnFromCallback((EMAElement)channel, nesting);
            cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
        }
        emaUnlock((EMAElement)hsChan);
    }
    cmiAPIExit((HAPP)app, "cmChannelMediaLoopReject=%d");
    return res;
}


RVAPI int RVCALLCONV
cmCallMediaLoopOff(
           /* Release all media loops in this call */
           IN HCALL hsCall
           )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HCONTROL ctrl=cmiGetControl(hsCall);
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;
    channelElem*channel;
    void* ptr=NULL;
    int nesting;

    if (!hsCall || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmCallMediaLoopOff: hsCall=0x%p.", hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command), __h245(maintenanceLoopOffCommand));

        res = sendMessageH245(ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            while((channel=getNextOutChan(ctrl, &ptr)))
            {
                cmiCBEnter((HAPP)app, "cmEvChannelMediaLoopStatus: haChan=0x%p, hsChan=0x%p, status=confirm.",(HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), channel);
                nesting = emaPrepareForCallback((EMAElement)channel);
                ifE(app->cmMyChannelEvent.cmEvChannelMediaLoopStatus)((HAPPCHAN)emaGetApplicationHandle((EMAElement)channel), (HCHAN)channel, cmMediaLoopOff);
                emaReturnFromCallback((EMAElement)channel, nesting);
                cmiCBExit((HAPP)app, "cmEvChannelMediaLoopStatus.");
            }
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit((HAPP)app, "cmCallMediaLoopOff=%d", res);
    return res;
}

RVAPI int RVCALLCONV
cmChannelSendMiscCommand(
         IN     HCHAN       hsChan,
         IN     cmMiscellaneousCommand miscCommand)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;
    int fieldId;

    if (!hsChan || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelSendMiscCommand: hsChan=0x%p. command %d", hsChan,miscCommand);
    if (emaLock((EMAElement)hsChan))
    {
        switch(miscCommand)
        {
            case cmVideoFreezePicture:          fieldId=__h245(videoFreezePicture);         break;
            case cmVideoSendSyncEveryGOB:       fieldId=__h245(videoSendSyncEveryGOB);      break;
            case cmVideoSendSyncEveryGOBCancel: fieldId=__h245(videoSendSyncEveryGOBCancel);break;
            case cmSwitchReceiveMediaOff:       fieldId=__h245(switchReceiveMediaOff);      break;
            case cmSwitchReceiveMediaOn:        fieldId=__h245(switchReceiveMediaOn);       break;
            case cmVideoFastUpdatePicture:      fieldId=__h245(videoFastUpdatePicture);     break;
            default :
                emaUnlock((EMAElement)hsChan);
                cmiAPIExit((HAPP)app, "cmChannelSendMiscCommand: [Invalid Parameter]");
                return RVERROR;
        }

        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(command),__h245(miscellaneousCommand));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);

        pvtAddBranch2(hVal,nodeId,__h245(type),fieldId);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelSendMiscCommand=%d");
    return res;
}


RVAPI int RVCALLCONV
cmChannelSendTransportCapInd(
         IN     HCHAN       hsChan,
         IN     int         transCapNodeId)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsChan);
    channelElem*channel=(channelElem*)hsChan;
    HPVT hVal=cmGetValTree(hApp);
    int message, nodeId, res = RVERROR;
    cmElem*app=(cmElem*)hApp;

    if (!hsChan || !hApp)   return RVERROR;

    cmiAPIEnter((HAPP)app, "cmChannelSendTransportCapInd: hsChan=0x%p Capability nodeId %d. ", hsChan,transCapNodeId);

    if (emaLock((EMAElement)hsChan))
    {

        message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message, __h245(indication),__h245(miscellaneousIndication));
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),channel->myLCN,NULL,NULL);
        pvtMoveTree(app->hVal,pvtAddBranch2(hVal,nodeId,__h245(type),__h245(transportCapability)),transCapNodeId);

        res = sendMessageH245Chan(channel->ctrl, hsChan, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)hsChan);
    }

    cmiAPIExit((HAPP)app, "cmChannelSendTransportCapInd=%d", res);
    return res;

}


#ifdef __cplusplus
}
#endif
