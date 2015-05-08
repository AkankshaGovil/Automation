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
#include <cmCtrlMSD.h>
#include <ti.h>
#include <cmdebprn.h>

#define ifE(a) if(a)(a)
#define CMERRMSG(f) {f; return RVERROR;}

void RVCALLCONV rtdTimeoutEventsHandler(void*hsCall);


/* round trip delay________________________________________________________________________________ */

RVAPI int RVCALLCONV
cmCallRoundTripDelay(
             /* Measure the round trip delay to the remote terminal */
             /* Note: maxDelay=0 --> delay taken from configuration */
             IN     HCALL       hsCall,
             IN     INT32       maxDelay /* maximum waiting delay in seconds */
             )
{
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int ret=0;
    HPVT hVal=cmGetValTree(hApp);
    int nodeId,message;
    rtdT* rtd;
    if (!hsCall) return RVERROR;

    cmiAPIEnter(hApp, "cmCallRoundTripDelay: hsCall=0x%lx, delay=%d", hsCall, maxDelay);

    if (emaLock((EMAElement)hsCall))
    {
        rtd=&ctrl->rtd;

        cmTimerReset(hApp,&(rtd->timer));

        if (maxDelay)
            pvtGetChildValue(hVal,((cmElem*)hApp)->h245Conf,__h245(roundTripTimeout),&maxDelay,NULL);

        rtd->sqNumber++;
        rtd->sqNumber%=256;

        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(request) _h245(roundTripDelayRequest)
                                                   _h245(sequenceNumber) LAST_TOKEN}, rtd->sqNumber, NULL);
        ret = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);
        cmTimerReset(hApp,&(rtd->timer));

        if (ret >= 0)
        {
            rtd->timer=cmTimerSet(hApp,rtdTimeoutEventsHandler,(void*)ctrl,maxDelay*1000);
            rtd->timeStart=timerGetTimeInMilliseconds();
            rtd->waiting=TRUE;
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmCallRoundTripDelay: [%d]",ret);
    return ret;
}


int roundTripDelayRequest(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    int nodeId, rmessage, res;
    INT32 sqNumber;

    pvtGetChildValue(hVal,message, __h245(sequenceNumber),&sqNumber,NULL);
    rmessage=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
    __pvtBuildByFieldIds(nodeId,hVal,rmessage, {_h245(response) _h245(roundTripDelayResponse) 
                                                _h245(sequenceNumber) LAST_TOKEN}, sqNumber, NULL);
    res = sendMessageH245((HCONTROL)ctrl, rmessage);
    pvtDelete(hVal,rmessage);
    return res;
}

int roundTripDelayResponse(controlElem* ctrl, int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    rtdT* rtd=&ctrl->rtd;
    INT32 sqNumber;
    int delay;

    pvtGetChildValue(hVal,message, __h245(sequenceNumber),&sqNumber,NULL);

    cmTimerReset(hApp,&(rtd->timer));
    if (sqNumber==rtd->sqNumber)
    {
        int nesting;

        delay=timerGetTimeInMilliseconds()-rtd->timeStart;
        cmiCBEnter(hApp, "cmEvCallRoundTripDelay: haCall=0x%lx, hsCall=0x%lx, delay=%d.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), delay);
        nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRoundTripDelay)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), delay);
        emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
        cmiCBExit(hApp, "cmEvCallRoundTripDelay.");
    }
    return TRUE;
}


void RVCALLCONV rtdTimeoutEventsHandler(void*_ctrl)
{
    controlElem* ctrl=(controlElem*)_ctrl;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    rtdT* rtd=&ctrl->rtd;

    if (emaLock((EMAElement)cmiGetByControl((HCONTROL)ctrl)))
    {
        int nesting;

        cmTimerReset(hApp,&(rtd->timer));
        cmiCBEnter(hApp, "cmEvCallRoundTripDelay: haCall=0x%lx, hsCall=0x%lx, delay=%d.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), -1);
        nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
        ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallRoundTripDelay)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl), -1);
        emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
        cmiCBExit(hApp, "cmEvCallRoundTripDelay.");
        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
}

/************************************************************************
 * rtdInit
 * purpose: Initialize the round trip delay process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
int rtdInit(controlElem* ctrl)
{
    rtdT* rtd=&ctrl->rtd;
    rtd->sqNumber=0;
    rtd->timer=(HTI)RVERROR;
    return 0;
}


/************************************************************************
 * rtdEnd
 * purpose: Stop the round trip delay process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void rtdEnd(IN controlElem* ctrl)
{
    rtdT*   rtd=&ctrl->rtd;
    HAPP    hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));

    /* Get rid of the timer if one exists */
    cmTimerReset(hApp,&rtd->timer);
}


#ifdef __cplusplus
}
#endif

