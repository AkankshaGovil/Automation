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



/* multipoint mode command on/off____________________________________________________________ */

int multipointModeCommand(controlElem* ctrl,int lcn)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    int nesting;

    if (!hApp) return RVERROR;
    if (lcn);
    ctrl->mpMode=1;

    cmiCBEnter(hApp, "cmEvCallMiscStatus: haCall=0x%lx, hsCall=0x%lx, status=%s.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), "mpModeOn");
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallMiscStatus)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),   cmMiscMpOn);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallMiscStatus.");
    return TRUE;
}

int cancelMultipointModeCommand(controlElem* ctrl,int lcn)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    int nesting;
    if (!hApp) return RVERROR;
    if (lcn);
    ctrl->mpMode=0;

    cmiCBEnter(hApp, "cmEvCallMiscStatus: haCall=0x%lx, hsCall=0x%lx, status=%s.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl), "mpModeOff");
    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    ifE(((cmElem*)hApp)->cmMySessionEvent.cmEvCallMiscStatus)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), (HCALL)cmiGetByControl((HCONTROL)ctrl),   cmMiscMpOff);
    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
    cmiCBExit(hApp, "cmEvCallMiscStatus.");
    return TRUE;
}



RVAPI int RVCALLCONV
cmCallMultipointCommand(
          /* Send multipoint command (on or off) message */
          IN  HCALL hsCall,
          IN  BOOL isModeOn /* TRUE: mp mode (on), FALSE: cancel mp mode (off) */
          )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    HCONTROL ctrl=cmiGetControl(hsCall);
    int nodeId, message;
    HPVT hVal=cmGetValTree(hApp);
    int res = 0;
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCallMultipointCommand: hsCall=0x%lx, %s.", hsCall, (isModeOn)?"on":"off");

    if (emaLock((EMAElement)hsCall))
    {
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message,__h245(command), __h245(miscellaneousCommand) );
        pvtAdd(hVal,nodeId,__h245(logicalChannelNumber),10000,NULL,NULL);
        pvtAddBranch2(hVal,nodeId,__h245(type),(isModeOn)?__h245(multipointModeCommand):__h245(cancelMultipointModeCommand));
        res = sendMessageH245(ctrl, message);
        pvtDelete(hVal,message);
        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
    cmiAPIExit(hApp, "cmCallMultipointCommand: [%d].", res);
    return res;
}



RVAPI int RVCALLCONV
cmCallMultipointStatus(
          /* Get the multipoint mode status (on or off) */
          IN  HCALL hsCall,
          OUT BOOL *isModeOn /* TRUE: mp mode (on), FALSE: cancel mp mode (off) */
          )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);

    if (!hsCall || !hApp || !isModeOn) return RVERROR;

    cmiAPIEnter(hApp, "cmCallMultipointStatus: hsCall=0x%lx, &isModeOn=%p", hsCall, isModeOn);
    if (emaLock((EMAElement)hsCall))
    {
        *isModeOn = ctrl->mpMode;
        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
    cmiAPIExit(hApp, "cmCallMultipointStatus: [%s (%d)].", (ctrl->mpMode)?"on":"off", ctrl->mpMode);
    return TRUE;
}

#ifdef __cplusplus
}
#endif
