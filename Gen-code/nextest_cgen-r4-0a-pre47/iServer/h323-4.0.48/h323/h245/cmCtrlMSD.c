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
#include <intutils.h>

int  cmcReadyEvent  (controlElem* ctrl);


BOOL simulatedMessageH245(IN HCONTROL ctrl);


/**************************************************************************************/
/*           Internal routines                                                        */
/**************************************************************************************/

/************************************************************************
 * reportError
 * purpose: To report of an error condition to the user, and return the state 
 *			of the procedure to idle.
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
static void reportError(controlElem* ctrl)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
	cmElem *app = (cmElem*)hApp;
    msdT* msd=&ctrl->msd;
    int nesting;

	if (app->cmMySessionEvent.cmEvCallMasterSlaveStatus)
	{
		cmiCBEnter(hApp, "cmEvCallMasterSlaveStatus: haCall=0x%lx, hsCall=0x%lx, status=error.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)), cmiGetByControl((HCONTROL)ctrl));
	    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
	    (app->cmMySessionEvent.cmEvCallMasterSlaveStatus) ((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),
			                                               (HCALL)cmiGetByControl((HCONTROL)ctrl), 
														   cmMSError);
	    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
	    cmiCBExit(hApp, "cmEvCallMasterSlaveStatus.");
	}

	/* return the state to idle, the procedure is over */
    msd->state=idle;
}

/************************************************************************
 * sendErrorResponse
 * purpose: To send a reject message due to an error condition. 
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
static void sendErrorResponse(controlElem* ctrl)
{
    HAPP  hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT  hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;
    int   nodeId;
	int   message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);

	/* close the timer, no response is expected to this message */
    cmTimerReset(hApp,&(msd->timer));

	/* report the error to the user and reset the state to idle */
    reportError(ctrl);

	/* build and send the reject message */
    __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(masterSlaveDeterminationReject)
                                               _h245(cause) _h245(identicalNumbers) LAST_TOKEN}, 0, NULL);
    sendMessageH245((HCONTROL)ctrl, message);
    pvtDelete(hVal,message);
}

/************************************************************************
 * msdTimeoutEventsHandler
 * purpose: Call back in case that no response was received after timeout has expired
 * input  : ctrl            - Control object
 * output : None.
 * return : None.
 ************************************************************************/
void RVCALLCONV msdTimeoutEventsHandler(void*_ctrl)
{
    controlElem* ctrl=(controlElem*)_ctrl;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;

    if (emaLock((EMAElement)cmiGetByControl((HCONTROL)ctrl)))
    {
		/* if it's on the first response from the remote, cancel the procedure */
        if (msd->state==outgoingAwaitingResponse)
        {
			/* send a release message */
            int message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
            pvtAddBranch2(hVal,message, __h245(indication), __h245(masterSlaveDeterminationRelease));
            sendMessageH245((HCONTROL)ctrl, message);
            pvtDelete(hVal,message);

			/* inform the user of the error and return to idle state */
            reportError(ctrl); /* Error A */
        }
		/* if it's after we sent an ack just report to the user and return to idle state */
        else if (msd->state==incomingAwaitingResponse)
              reportError(ctrl); /* Error A */
        emaUnlock((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    }
}


/************************************************************************
 * determineStatus
 * purpose: The actual algorithm to determine the master & slave according to
 *			the terminal type and determination number.
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
static void determineStatus(controlElem* ctrl)
{
    msdT* msd=&ctrl->msd;

	/* if the terminat type is the same we need to decide according to the determination
	   number */
    if (msd->myTerminalType == msd->terminalType)
    {
		/* calculate the difference between the determination numbers using lower 24bits */
        int n=(msd->statusDeterminationNumber - msd->myStatusDeterminationNumber + 0x1000000) % 0x1000000;

		/* if the difference is zero, we can't decide,
		   otherwise if the difference is within the 24 bits, i.e. my number is smaller, we are slave
		   else we are master */
		msd->status= (n & 0x7fffff) ?  ((n<0x800000) ? master : slave)  : indeterminate;
    }
    else
		/* if there are different terminal types the bigger is masetr */
        msd->status =  (msd->terminalType < msd->myTerminalType) ? master : slave;
}

/************************************************************************
 * setAndSendMasterSlave
 * purpose: To set the resonse into the control data structure,
 *			send an ACK to the remote,
 *			report the result to the user.
 * input  : ctrl    - Control object
 *			isMaster - TRUE if we are master
 * output : none
 * return : none
 ************************************************************************/
static void setAndSendMasterSlave(controlElem* ctrl,int isMaster, int report)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
	cmElem *app = (cmElem*)hApp;
    msdT* msd=&ctrl->msd;
    int message=pvtAddRoot(hVal,app->synProtH245,0,NULL);
    int nodeId, nesting, res;

	/* set the result to the control DB */
    ctrl->isMaster=isMaster;
    msd->status=(isMaster) ? master : slave;

	/* send a response to the remote with the result */
    __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(masterSlaveDeterminationAck)
                                               _h245(decision) LAST_TOKEN}, 0, NULL);
    pvtAddBranch(hVal,nodeId,(isMaster)?__h245(slave):__h245(master));
    res = sendMessageH245((HCONTROL)ctrl, message);
    pvtDelete(hVal,message);

    if (report)
    {
        /* The procedure is finished */
        msd->state=idle;

	    /* report the result to the user */
	    if ((app->cmMySessionEvent.cmEvCallMasterSlaveStatus) && (res >= 0))
	    {
	        cmiCBEnter(hApp, "cmEvCallMasterSlaveStatus: haCall=0x%lx, hsCall=0x%lx, status=%s.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl), (isMaster)?"master":"slave");
		    nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
		    (app->cmMySessionEvent.cmEvCallMasterSlaveStatus)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),(HCALL)cmiGetByControl((HCONTROL)ctrl), (isMaster)?cmMSMaster:cmMSSlave);
		    emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
		    cmiCBExit(hApp, "cmEvCallMasterSlaveStatus.");
	    }
    }

	/* check if the H.245 openning formalities (i.e. TCS & MSD procedures) have ended,
	   if so, report to the user that the H.245 is ready for operation */
    if (res >= 0)
        cmcReadyEvent(ctrl);
}


/************************************************************************
 * countCheckAndSend
 * purpose: Retry mechanism. Check if retries expired, if not send a new
 *			request with a new determination number, else send a reject and
 *			finish the procedure.
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
static void countCheckAndSend(controlElem* ctrl)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;
    int nodeId, message, res;

	/* check if retry expired, it's always 3 */
    if (msd->count>=3)
    { /* >= N100 */ /*Error F */
		/* send reject and terminate the procedure */
        sendErrorResponse(ctrl);
    }
    else
    { /* < N100 */
		/* regenerate a new random 24 bit determination number */
        msd->myStatusDeterminationNumber=UTILS_RandomGeneratorGetValue(&(((cmElem*)hApp)->seed))*(timerGetTimeInMilliseconds()+0xffff)%0xffffff;

		/* build a new request with the new number */
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message,__h245(request), __h245(masterSlaveDetermination));
        pvtAdd(hVal,nodeId,__h245(statusDeterminationNumber),msd->myStatusDeterminationNumber, NULL,NULL);
        pvtAdd(hVal,nodeId,__h245(terminalType),msd->myTerminalType, NULL, NULL);

		/* advance the retries counter */
        msd->count++;

		/* send the message */
        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
			/* set the timer to wait for a response on the request */
            INT32 timeout=9;
            pvtGetChildValue2(hVal,((cmElem*)hApp)->h245Conf,__h245(masterSlave),__h245(timeout),&(timeout),NULL);
            cmTimerReset(hApp, &(msd->timer));
            msd->timer=cmTimerSet(hApp,msdTimeoutEventsHandler,(void*)ctrl,timeout*1000);
        }

        /* change the state so we know we are waiting for a response to our request */
        msd->state=outgoingAwaitingResponse;
    }
}

/*******************************************************************************
 * processMSD
 *
 * Purpose: To respond to a MSD by building the appropriate message and sending
 *			it according to the status of the procedure 
 *
 * Input:   ctrl - The control elemnt 
 *
 * Output: None.
 *
 * Return: Non-negative value on success, other on failure
 ******************************************************************************/
static int processMSD(controlElem* ctrl)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;
    int nodeId, message, res = RVERROR;

	/* Initiate THE algorithm to determine master/slave */
    determineStatus(ctrl);

    if (msd->status==indeterminate)
    { /* indeterminate */
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(masterSlaveDeterminationReject)
          _h245(cause) _h245(identicalNumbers) LAST_TOKEN}, 0, NULL);
        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

		/* all is over */
        msd->state=idle;
    }
    else
    { /* master or slave */
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        __pvtBuildByFieldIds(nodeId,hVal,message, {_h245(response) _h245(masterSlaveDeterminationAck)
          _h245(decision) LAST_TOKEN}, 0, NULL);
        pvtAddBranch(hVal,nodeId,(msd->status==master) ? __h245(slave) : __h245(master));

        /* Update the control element as well */
        ctrl->isMaster = (msd->status==master);
        ctrl->isMasterSlave = 1;

        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
	    {
			/* set a response timer since we need an ack back on our decision */
	        INT32 timeout=9;
	        pvtGetChildValue2(hVal,((cmElem*)hApp)->h245Conf,__h245(masterSlave),__h245(timeout),&(timeout),NULL);
	        cmTimerReset(hApp,&(msd->timer));
	        msd->timer=cmTimerSet(hApp,msdTimeoutEventsHandler,(void*)ctrl,timeout*1000);

		    /* waiting for the ack on the ack */
	        msd->state=incomingAwaitingResponse;
        }
    }

    return res;
}


/**************************************************************************************/
/*           Internal routines used outside this file                                                        */
/**************************************************************************************/

/************************************************************************
 * msdInit
 * purpose: Initialize the master slave determination process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void msdInit(controlElem* ctrl)
{
    cmElem* app = (cmElem *)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal = cmGetValTree((HAPP)app);
    msdT* msd=&ctrl->msd;
    msd->status=indeterminate;
    msd->state=idle;
    msd->timer=(HTI)RVERROR;
    msd->myTerminalType=50;
    pvtGetChildValue2(hVal,app->h245Conf, __h245(masterSlave), __h245(terminalType),
        (INT32 *)&(msd->myTerminalType), NULL);    
    msd->myStatusDeterminationNumber=UTILS_RandomGeneratorGetValue(&(app->seed))*(timerGetTimeInMilliseconds()+0xffff)%0xffffff;
    msd->manualResponse=(pvtGetChild2(hVal,app->h245Conf,__h245(masterSlave),__h245(manualResponse))>=0);
    logPrint(app->log, RV_INFO,
            (app->log, RV_INFO, "MSD manual response on %x = %d ",ctrl,msd->manualResponse));
}

/************************************************************************
 * msdEnd
 * purpose: Stop the master slave determination process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void msdEnd(IN controlElem* ctrl)
{
    msdT* msd=&ctrl->msd;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));

    /* Get rid of the timer if one exists */
    cmTimerReset(hApp,&(msd->timer));
}

/*************************************************************************************
 * msdDetermineRequest
 *
 * Purpose: Main routine to initiate or respond to a MSD request.
 *
 * Input:	ctrl						- The ctrl element of the call
 *			terminalType				- The given terminal type of the call, may be -1.
 *			statusDeterminationNumber	- The determination numer, may be -1 and then needs to be
 *										  generated.
 * Output: None.
 *
 * return : Non-negative value on success
 *          Negative value on failure
 ******************************************************************************************/
int msdDetermineRequest(controlElem* ctrl, int terminalType, int statusDeterminationNumber)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;
    int nodeId, message, res = RVERROR;

	if((msd->state!=idle) && (msd->state!=incomingAwaitingManualAcknoledge))
		return RVERROR;

    /* if this is a new MSD process, re-generate the random determenition number */
    if (msd->myStatusDeterminationNumber < 0)
    {
        /* if the determination number is -1, generate a new random 24 bit determination number */
        if (statusDeterminationNumber<0)
            statusDeterminationNumber=UTILS_RandomGeneratorGetValue(&(((cmElem*)hApp)->seed))*(timerGetTimeInMilliseconds()+0xffff)%0xffffff;
        msd->myStatusDeterminationNumber=statusDeterminationNumber;
    }

	/* if terminal type is not given, i.e. -1, get it from the configuration or set it to default 50 */
    if (terminalType >= 0)
    {
        msd->myTerminalType=terminalType;
    }

	/* if the procedure hasn't started yet initiate it */
	if (msd->state==idle)
    {
		/* build the request message with the terminaType and determinationNumber */
        message=pvtAddRoot(hVal,((cmElem*)hApp)->synProtH245,0,NULL);
        nodeId=pvtAddBranch2(hVal,message,__h245(request), __h245(masterSlaveDetermination));
        pvtAdd(hVal,nodeId,__h245(statusDeterminationNumber),msd->myStatusDeterminationNumber, NULL,NULL);
        pvtAdd(hVal,nodeId,__h245(terminalType),msd->myTerminalType, NULL,NULL);

		/* Initialize the retry mechanism and send the message */
        msd->count=1;
        res = sendMessageH245((HCONTROL)ctrl, message);
        pvtDelete(hVal,message);

        if (res >= 0)
        {
            /* Make sure message was actually sent and we didn't use a "dummy" H245 (as in the GK's MC) */
			if (!simulatedMessageH245((HCONTROL)ctrl))
			{
                /* We're only here if we really sent the message */
                /* start a timer to wait for response */
                INT32 timeout=9;
                pvtGetChildValue2(hVal,((cmElem*)hApp)->h245Conf,__h245(masterSlave),__h245(timeout),&(timeout),NULL);
                cmTimerReset(hApp,&(msd->timer));
                msd->timer=cmTimerSet(hApp,msdTimeoutEventsHandler,(void*)ctrl,timeout*1000);
            }

            /* change the state of the procedure */
            msd->state=outgoingAwaitingResponse;
        }
    }
    else if (msd->state==incomingAwaitingManualAcknoledge)
    {
		/* in case that we are in a state waiting to respond to a request, build the response and send it */
        res = processMSD(ctrl);
    }

    return res;
}

/************************************************************************
 * msdGetMibParams
 * purpose: Get parameters related to the MIB for an MSD object in the
 *          control
 * input  : ctrl            - Control object
 * output : state           - State of MSD
 *          status          - Status of MSD
 *          retries         - Number of MSD retries
 *          terminalType    - Local terminal's type in MSD
 * return : Non-negative value on success
 *          Negative value on failure
 * Any of the output parameters can be passed as NULL if not relevant to caller
 ************************************************************************/
int msdGetMibParams(
    IN  controlElem*    ctrl,
    OUT msdState*       state,
    OUT msdStatus*      status,
    OUT int*            retries,
    OUT int*            terminalType)
{
    if (state)
        *state = ctrl->msd.state;
    if (status)
        *status = ctrl->msd.status;
    if (retries)
        *retries = ctrl->msd.count;
    if (terminalType)
        *terminalType = ctrl->msd.myTerminalType;

    return 0;
}

/**************************************************************************************/
/*           Handling incoming MSD messages                                           */
/**************************************************************************************/


/************************************************************************
 * masterSlaveDetermination
 * purpose: Handle an incoming MasterSlaveDetermination request,
 *          notifying the application or answering automatically.
 * input  : ctrl    - Control object
 *          message - MSD request message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDetermination(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;
    int res = 0;

	/* get the remote numbers from the message */
    pvtGetChildValue(hVal,message,__h245(terminalType),(INT32 *)&(msd->terminalType),NULL);
    pvtGetChildValue(hVal,message,__h245(statusDeterminationNumber),(INT32 *)&(msd->statusDeterminationNumber),NULL);

	/* if we have not initiated yet an MSD procedure */
    if (msd->state==idle)
    {
        cmElem *app = (cmElem*)hApp;

        /* in case of manual response, inform the application of the initiation */
        if ( (msd->manualResponse) && (app->cmMySessionEvent.cmEvCallMasterSlave) )
        {
            int nesting;
            msd->state=incomingAwaitingManualAcknoledge;
            cmiCBEnter(hApp, "cmEvCallMasterSlave: haCall=0x%lx, hsCall=0x%lx, terminalType =%d, statusDeterminationNumber = %d.",
                       (HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl), ctrl->msd.terminalType,ctrl->msd.statusDeterminationNumber);
            nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
            (app->cmMySessionEvent.cmEvCallMasterSlave)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),(HCALL)cmiGetByControl((HCONTROL)ctrl),ctrl->msd.terminalType,ctrl->msd.statusDeterminationNumber);
            emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
            cmiCBExit(hApp, "cmEvCallMasterSlave.");
        }
        else
			/* Automatic response: Determine the ,aster/slave and issue a response */
            res = processMSD(ctrl);
    }
	/* We have already initiated an MSD procedure from our side */
    else if (msd->state==outgoingAwaitingResponse)
    {
		/* close the timer, this message is as good as an ack */
        cmTimerReset(hApp,&(msd->timer));

		/* activate THE algorithm to determine the master/slave */
        determineStatus(ctrl);

		/* According to the result respond to the remote request */
        if (msd->status==indeterminate)
        { /* indeterminate: retry sending the request with a new detrmination number */
            countCheckAndSend(ctrl);
        }
        else
        { /* master or slave */

			/* set the result into the control DB, send ack to the remote with the result
			   and report the result to the user */
            setAndSendMasterSlave(ctrl,msd->status==master, FALSE);
            {
				/* we need an ack on the ack so set a response timer */
                INT32 timeout=9;
                pvtGetChildValue2(hVal,((cmElem*)hApp)->h245Conf,__h245(masterSlave),__h245(timeout),&(timeout),NULL);
                msd->timer=cmTimerSet(hApp,msdTimeoutEventsHandler,(void*)ctrl,timeout*1000);
            }
			/* we are waiting for ack on the ack */
            msd->state=incomingAwaitingResponse;
        }
    }
	/* the remote has already initiated an MSD procedure with us, this is a mistake */
    else if (msd->state==incomingAwaitingResponse)
          sendErrorResponse(ctrl); /* Error C */
    else if (msd->state==incomingAwaitingManualAcknoledge)
          reportError(ctrl);  /* Error C */

    return res;
}


/************************************************************************
 * masterSlaveDeterminationAck
 * purpose: Handle an incoming MasterSlaveDeterminationAck message
 * input  : ctrl    - Control object
 *          message - MSD.Ack message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationAck(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    HPVT hVal=cmGetValTree(hApp);
    msdT* msd=&ctrl->msd;

	/* Get the remote decision */
    BOOL isMaster=(pvtGetChild2(hVal,message,__h245(decision),__h245(master))>=0);

	/* reset the timer, we got response */
    cmTimerReset(hApp,&(msd->timer));

	/* we are witing for first ACK of our request */
    if (msd->state==outgoingAwaitingResponse)
    {
		/* mark that we have finished at least one MSD procedure, so H.245 may start operate */
        ctrl->isMasterSlave=1;

		/* Set the result given by the remote to our control DB, send an ack on this ack 
		   and report the result to the user */
        setAndSendMasterSlave(ctrl, (int)isMaster, TRUE);
    }
	/* This is actually the ack on the ack we sent */
    else if (msd->state==incomingAwaitingResponse)
    {
        BOOL localIsMaster = (msd->status==master);

		/* check if the remote agrees with the result we sent it */
        if (isMaster == localIsMaster)
        {   /* There is an agreement, so the procedure is finished */
            int nesting;
			cmElem *app = (cmElem*)hApp;

			/* mark that we have finished at least one MSD procedure, so H.245 may start operate */
            ctrl->isMasterSlave=1;

			/* The procedure is finished - change the state and notify the application */
            msd->state=idle;

			/* Report the result to the user */
			if(app->cmMySessionEvent.cmEvCallMasterSlaveStatus)
			{
                cmiCBEnter(hApp, "cmEvCallMasterSlaveStatus: haCall=0x%lx, hsCall=0x%lx, status=%s.",(HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),cmiGetByControl((HCONTROL)ctrl), (isMaster)?"master":"slave");
                nesting=emaPrepareForCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl));
                (app->cmMySessionEvent.cmEvCallMasterSlaveStatus)((HAPPCALL)emaGetApplicationHandle((EMAElement)cmiGetByControl((HCONTROL)ctrl)),
					                                              (HCALL)cmiGetByControl((HCONTROL)ctrl), 
																  (isMaster)?cmMSMaster:cmMSSlave);
                emaReturnFromCallback((EMAElement)cmiGetByControl((HCONTROL)ctrl),nesting);
                cmiCBExit(hApp, "cmEvCallMasterSlaveStatus.");
            }
        }
        else  
		  /* oops! We disagree with the remote, i.e. error state */
          sendErrorResponse(ctrl); /* Error E */
    }

	/* check if the H.245 openning formalities (i.e. TCS & MSD procedures) have ended,
	   if so, report to the user that the H.245 is ready for operation */
    cmcReadyEvent(ctrl);
    return TRUE;
}


/************************************************************************
 * masterSlaveDeterminationReject
 * purpose: Handle an incoming MasterSlaveDeterminationReject message
 * input  : ctrl    - Control object
 *          message - MSD.Reject message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationReject(IN controlElem* ctrl, IN int message)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)cmiGetByControl((HCONTROL)ctrl));
    msdT* msd=&ctrl->msd;
    if (message);

	/* in case it's a reject on our request */
    if (msd->state==outgoingAwaitingResponse)
    {
		/* reset the timer, we got response */
        cmTimerReset(hApp,&(msd->timer));
        /* retry sending the request with a new detrmination number */
        countCheckAndSend(ctrl);
    }
	/* if its a reject on the ack that we've sent, it's an error */
    else if (msd->state==incomingAwaitingResponse)
          sendErrorResponse(ctrl); /* Error D */
    else if (msd->state==incomingAwaitingManualAcknoledge)
          reportError(ctrl);  /* Error D */
    return TRUE;
}


/************************************************************************
 * masterSlaveDeterminationRelease
 * purpose: Handle an incoming MasterSlaveDeterminationRelease message
 * input  : ctrl    - Control object
 *          message - MSD.Release message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationRelease(IN controlElem* ctrl, IN int message)
{
    msdT* msd=&ctrl->msd;
    if (message);

	/* this isi always an error state if a release was received during the procedure */
    switch(msd->state)
    {
        case outgoingAwaitingResponse:
        case incomingAwaitingResponse:
          sendErrorResponse(ctrl); /* Error B */
        break;
        case incomingAwaitingManualAcknoledge:
          reportError(ctrl);  /* Error B */
        break;
        default:
            break;
    }
    return TRUE;
}


/************************************************************************/
/*                             API                                      */
/************************************************************************/

/************************************************************************
 * cmCallMasterSlaveDetermine
 * purpose: Start master slave determination request
 * input  : hsCall			- The call handle
 *			terminalType    - Our terminal type
 * output : None
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmCallMasterSlaveDetermine(
               IN   HCALL       hsCall,
               IN   int         terminalType)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int res = RVERROR;

    if (!hsCall ||  !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmMasterSlaveDetermine: hsCall=0x%lx, terminalType=%d.", hsCall, terminalType);
    if (emaLock((EMAElement)hsCall))
    {
        res = msdDetermineRequest(ctrl, terminalType, -1);
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmMasterSlaveDetermine=%d", res);
    return res;
}

/************************************************************************
 * cmCallMasterSlaveDetermineExt
 * purpose: Start master slave determination request,
 *          or Acknowledge to master slave determination request
 *          and set terminalType and determinationNumber
 * input  : hsCall				- The call handle
 *			terminalType		- A given terminal type
 *			determinationNumber - A given determination number
 * output : None
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmCallMasterSlaveDetermineExt(
               IN   HCALL   hsCall,
               IN   int     terminalType,
               IN   int     determinationNumber)
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    int res = RVERROR;

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmMasterSlaveDetermineExt: hsCall=0x%lx, terminalType=%d determinationNumber = %d. ", hsCall, terminalType,determinationNumber);
    if (emaLock((EMAElement)hsCall))
    {
        res = msdDetermineRequest(ctrl, terminalType, determinationNumber);
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmMasterSlaveDetermineExt=%d", res);
    return res;
}


/************************************************************************
 * cmCallMasterSlaveStatus
 * purpose: Returns the master/slave status of this call
 * input  : hsCall				- The call handle
 * output : None
 * return : 1 - slave, 2 - master
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmCallMasterSlaveStatus(
            IN  HCALL       hsCall
            )
{
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    int ret=0;
    controlElem* ctrl=(controlElem*)cmiGetControl(hsCall);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCallMasterSlaveStatus: hsCall=0x%lx.", hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        if ((ctrl->isMasterSlave == 0) || (ctrl->state == ctrlNotInitialized))
            ret = 0;
		else if (ctrl->isMaster)
			ret = 2;
		else
			ret = 1;
        emaUnlock((EMAElement)hsCall);
    }

    {
        char *msStatusA[] = {(char*)"error", (char*)"slave", (char*)"master"};
        cmiAPIExit(hApp, "cmCallMasterSlaveStatus: [%s].", nprn(msStatusA[ret]));
    }
	if (ret==0)
		return RVERROR;
	else
	    return ret;
}


#ifdef __cplusplus
}
#endif
