/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_call.c
 *
 * This file contains all the commands that are used for the
 * call\conference window of the test application.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <ctype.h>
#include "stkutils.h"
#include "TAP_h450.h"
#include "TAP_general.h"
#include "TAP_channel.h"
#include "TAP_utils.h"
#include "TAP_call.h"



/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * Call_GetCounterValue
 * purpose : Get new number for a call
 * input   : none
 * output  : none
 * return  : Counter value
 ********************************************************************************************/
int Call_GetCounterValue(void)
{
   static int CallCounter = 0;

   CallCounter++;
   return CallCounter;
}


/*******************************************************************************************
 * FreeCallObject
 * purpose : Free and deallocate a call object
 * input   : CurrentCall    - Current call information to free
 * output  : none
 * return  : none
 *******************************************************************************************/
void FreeCallObject(CallInfo* CurrentCall)
{
    /* Free application call object */
    AppFree(CurrentCall);
}


/*******************************************************************************************
 * CallInfoStr
 * purpose : Create a string represnting call information to display on calls listbox in the
 *           main window
 * input   : CurrentCall    - Current call information to use
 *           StateStr       - State string to use
 * output  : none
 * return  : none
 *******************************************************************************************/
char* CallInfoStr(CallInfo* CurrentCall, char* StateStr)
{
    static char data[100];

    sprintf(data, "%d 0x%p: %s (%s)",
            CurrentCall->counter,
            CurrentCall,
            StateStr,
            (CurrentCall->incoming ? "in" : "out") );

    return data;
}

/*******************************************************************************************
 * PrepareIncomingCallWindow
 * purpose : Fill an incoming call window with all the relevant fields
 * input   : CurrentCall    - Current call information to use
 * output  : none
 * return  : none
 *******************************************************************************************/
void PrepareIncomingCallWindow(CallInfo* CurrentCall)
{
    static char* incallInfo = (char*)".incall.info.sourDest.lis";
    static char* checkboxState[] = {(char*)"deselect", (char*)"select"};

    INT32 IsEarly245 = FALSE;
    INT32 IsFastConnect = FALSE;
    INT32 IsTunneling = FALSE;
    INT32 IsComplete = FALSE;
    cmAlias Alias;
    char AliasStr[512];
    int i, AliasStatus;
    cmTransportAddress EarlyH245Addr;

    /* Get some boolean parameters about this call */

    IsEarly245 = (cmCallGetParam(CurrentCall->hsCall, cmParamSetupH245Address, 0,
        NULL, (char *)&EarlyH245Addr) >= 0);
    if(cmCallGetParam(CurrentCall->hsCall, cmParamSetupSendingComplete, 0, &IsComplete, (char *) NULL) < 0)
        IsComplete = FALSE;
    IsTunneling = (cmCallHasControlSession(CurrentCall->hsCall) > 0);
    IsFastConnect = (cmCallGetParam(CurrentCall->hsCall, cmParamSetupFastStart, 0, &IsFastConnect, NULL)>=0);

    /* Clear source-dest information window */
    TclExecute("%s delete 0 end", incallInfo);

    /* Get Source information */
    TclExecute("%s insert end {Source:}\n"
               "%s itemconfigure end -foreground red",
               incallInfo, incallInfo);
    AliasStatus = 0;
    i = 0;
    Alias.string = (char *) AppAlloc(sizeof(char)*250);
    while (AliasStatus >= 0)
    {
        /* Get an alias from the CM */
        AliasStatus = cmCallGetParam(CurrentCall->hsCall, cmParamSourceAddress, i,
            (int *) sizeof(Alias), (char *) &Alias);
        if (AliasStatus >= 0)
        {
            /* Convert the alias into a string */
            Alias2String(&Alias, AliasStr);
            /* Add the string to the source-dest information */
            TclExecute("%s insert end {%s}", incallInfo, AliasStr);
            i++;
        }
    }

    /* Get Destination information */
    TclExecute("%s insert end {} {Destination:}\n"
               "%s itemconfigure end -foreground red",
               incallInfo, incallInfo);
    AliasStatus = 0;
    i = 0;
    while (AliasStatus >= 0)
    {
        /* Get an alias from the CM */
        AliasStatus =  cmCallGetParam(CurrentCall->hsCall, cmParamDestinationAddress, i,
            (int *) sizeof(Alias), (char *) &Alias);
        if (AliasStatus >= 0)
        {
            /* Convert the alias into a string */
            Alias2String(&Alias, AliasStr);
            /* Add the string to the source-dest information */
            TclExecute("%s insert end {%s}", incallInfo, AliasStr);
            i++;
        }
    }
    AppFree(Alias.string);

    /* Set the remote parameters to the incoming call window */
    TclExecute(".incall.remote.fs %s\n"
               ".incall.remote.e245 %s\n"
               ".incall.remote.sendCmplt %s\n"
               ".incall.remote.tun %s\n",
               checkboxState[IsFastConnect],
               checkboxState[IsEarly245],
               checkboxState[IsComplete],
               checkboxState[IsTunneling]);
}


/********************************************************************************************
 * SetCallMedia
 * purpose : Set a call's media parameters from TCL/TK variables
 * input   : CurrentCall    - Call to set
 * output  : none
 * return  : none
 ********************************************************************************************/
void SetCallMedia(CallInfo* CurrentCall)
{
#ifdef USE_RTP
    if (atoi(TclGetVariable("app(options,replayMedia)")))
        CurrentCall->action = RTP_Replay;
    else
        CurrentCall->action = RTP_None;
#endif
}


/********************************************************************************************
 * Call_SetOutgoingParams
 * purpose : Set the parameters of an outgoing call before dialing
 * input   : CurrentCall    - Call to set
 * output  : none
 * return  : none
 ********************************************************************************************/
void Call_SetOutgoingParams(CallInfo* CurrentCall)
{
    char* variableStr;
    BOOL boolParam;

    /* Set incomplete address if we have to */
    boolParam = atoi(TclGetVariable("app(newcall,canOvsp)"));

    cmCallSetParam(CurrentCall->hsCall,cmParamCanOverlapSending,0,boolParam,NULL);

    /* Set bandwidth */
    variableStr = TclGetVariable("app(test,bw)");
    if (strlen(variableStr) > 0)
    {
        UINT32 bw = atoi(variableStr);
        cmCallSetParam(CurrentCall->hsCall,cmParamRequestedRate,0,bw,NULL);
    }

    /* set rateMultiplier */
    boolParam = (atoi(TclGetVariable("tmp(newcall,IsMultiRate)")) == 1);
    cmCallSetParam(CurrentCall->hsCall,cmParamMultiRate,0,boolParam,NULL);

    /* Set H.245 stage */
    cmCallSetParam(CurrentCall->hsCall, cmParamH245Stage, 0,
        String2CMH245Stage(TclGetVariable("app(h245,stage)")), NULL);

    /* set Multiplex and Maintain */
    boolParam = atoi(TclGetVariable("app(newcall,multiplexed)"));
    cmCallSetParam(CurrentCall->hsCall, cmParamIsMultiplexed, 0, boolParam, NULL);
    boolParam = atoi(TclGetVariable("app(newcall,maintain)"));
    cmCallSetParam(CurrentCall->hsCall, cmParamShutdownEmptyConnection, 0, !boolParam, NULL);

    /* Set parallel faststart and tunneling */
    boolParam = atoi(TclGetVariable("app(newcall,parallel)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamH245Parallel,0,boolParam,NULL);

    /* Set tunneling */
    boolParam = atoi(TclGetVariable("app(newcall,tunneling)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamH245Tunneling,0,boolParam,NULL);

    /* Set sending complete */
    CurrentCall->sendComplete = atoi(TclGetVariable("app(newcall,sendComplete)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamSetupSendingComplete,0,CurrentCall->sendComplete,NULL);


    /* Set User User Info */
    variableStr = TclGetVariable("app(options,userUser)");
    cmCallSetParam(CurrentCall->hsCall,cmParamUserUser,0,strlen(variableStr),variableStr);

    /* Set Display Info */
    variableStr = TclGetVariable("app(options,display)");
    cmCallSetParam(CurrentCall->hsCall,cmParamDisplay,0,strlen(variableStr),variableStr);

    /* Determine what to do with the media channels for this call */
    SetCallMedia(CurrentCall);
}


/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * Call_Accept
 * purpose : This procedure is called when the OK button is pressend on the incoming call
 *           window. It sets the parameters of an incoming call.
 * syntax  : Call.Accept <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Accept(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    if(atoi(TclGetVariable("app(newcall,earlyH245)")))
        cmCallSetParam(Call->hsCall, cmParamEarlyH245, 0, 1, NULL);
    if(atoi(TclGetVariable("app(newcall,tunneling)")))
        cmCallSetParam(Call->hsCall, cmParamH245Tunneling, 0, 1, NULL);

    /* Allow handling of media as in new call */
    SetCallMedia(Call);

    TclExecute("Window close .incall");

    return TCL_OK;
}


/********************************************************************************************
 * Call_IncompleteAddress
 * purpose : This procedure is called when the "Incomplete Address" button is pressend on
 *           the incoming call window. It sends incomplete address message to the originator
 *           of the call
 * syntax  : Call.IncompleteAddress <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_IncompleteAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int         retVal;
    CallInfo    *Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    TclExecute("Window close .incall");

    retVal = cmCallIncompleteAddress(Call->hsCall);

    return TCL_OK;
}


/********************************************************************************************
 * Call_AddressComplete
 * purpose : This procedure is called when the "Address Complete" button is pressend on
 *           the incoming call window. It sends Address Complete message to the originator
 *           of the call
 * syntax  : Call.AddressComplete <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_AddressComplete(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int         retVal;
    CallInfo    *Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    TclExecute("Window close .incall");

    retVal = cmCallAddressComplete(Call->hsCall);

    return TCL_OK;
}


/********************************************************************************************
 * Call_SendAdditionalAddr
 * purpose : This procedure is called when the "Send" button is pressend on the redial
 *           window. It sends additional address information to the destination of the call.
 * syntax  : Call.SendAdditionalAddr <callInfo>
 *           <callInfo>   - Call information (counter and handle)
 *           <isARJ>      - TRUE if message sent was ARJ
 *                          FALSE if message sent was SETUP ACK
 *           <digits>     - In case we're dealing with SETUP ACK, we also get the digits
 *           <aliases>    - List of the aliases for the ARJ case
 *           <resaendARQ> - resend ARQ or just send additional address
 *
 * note    : if the case is not ARJ, the last two parameters can be omited
 *
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendAdditionalAddr(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int         retVal;
    CallInfo*   Call;
    BOOL        isARJ;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if ((argv[3] == NULL) || (strlen(argv[3]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    isARJ = (atoi(argv[2]) != 0);

    Call->sendComplete = (atoi(TclGetVariable("tmp(redial,sendComplete)")) == 1);

    retVal = cmCallOverlapSendingExt(Call->hsCall,argv[3],Call->sendComplete);
    TclExecute("call:Log %d {INFORMATION sent (status=%s, digits=%s)}",
        Call->counter, Status2String(retVal), argv[3]);

    if (Call->sendComplete)
        TclExecute("Window delete .redial%p", (UINTPTR) Call->hsCall);

    return TCL_OK;
}


/********************************************************************************************
 * cmEvCallStateChanged
 * purpose : Handle changes in the state of calls.
 *           This is a callback function called by the CM.
 * input   : AppHCalll - call struct of the application
 *         : CmHCall - handle for the call
 *         : NewState - current state of the call
 * output  : none
 * return  : none
 ********************************************************************************************/
int RVCALLCONV cmEvCallStateChanged(IN HAPPCALL  haCall,
                                    IN HCALL     hsCall,
                                    IN UINT32    NewState,
                                    IN UINT32    stateMode)
{
    CallInfo*        CurrentCall;
    cmCallState_e    CurrState;

    CurrentCall = (CallInfo *)haCall;
    CurrentCall->hsCall = hsCall;
    CurrState = (cmCallState_e)NewState;

    CurrentCall->insideState++;
    CurrentCall->callState = NewState;

    /* Notify H450 state machines as to call state change.
    does nothing if H450 is not enabled */
    H450_CallStateChanged(CurrentCall, NewState, stateMode);

    /* Make sure that the call wasn't dropped already from the H450 */
    CurrentCall->insideState--;
    if (CurrentCall->hsCall == NULL)
    {
        if (CurrentCall->insideState == 0)
        {
            /* Now we should get rid of this call - no one will be looking for it */
            FreeCallObject(CurrentCall);
        }
        return 0;
    }

    TclExecute("Call:ChangeState %d {%s}",
        CurrentCall->counter,
        CallInfoStr(CurrentCall,
            ( (NewState==cmCallStateConnected)||(NewState==cmCallStateDisconnected)||(NewState==cmCallStateOffering) ) ?
            CMCallStateMode2String((cmCallStateMode_e)stateMode) :  CMCallState2String((cmCallState_e)NewState) ));

    switch (CurrState)
    {
        case  cmCallStateIdle:
        {
            int i;

            /* Make sure we clean the call from the main window */
            TclExecute("set index [Call.FindCallIndex %d]\n"
                       ".test.calls.list delete $index\n"
                       ".test.calls.conf delete $index\n"
                       ".test.calls.conn delete $index\n"
                       "call:Log %d {Closed.}",
                       CurrentCall->counter, CurrentCall->counter);

            /* Close all windows that belong to that call */
            TclExecute("Call:cleanup 0x%p", (UINTPTR)CurrentCall);

            /* Delete the call's information window */
            TclExecute("Window delete .call%d", CurrentCall->counter);

            UpdateCallNum(FALSE);

            for(i=0;i<3;i++)
                if(CurrentCall->rtpSessions[i]>=0)
                {
                    RTP_TestClose(CurrentCall->rtpSessions[i]);
                    CurrentCall->rtpSessions[i] = -1;
                }

            /* Close call in CM */
            if (CurrentCall->scriptCall == FALSE)
            {
                H450_CloseCall(CurrentCall);
                cmCallClose(hsCall);
                CurrentCall->hsCall = NULL;
                if (CurrentCall->insideState == 0)
                {
                    /* We can't free the call object if we're inside a state callback
                       in our call-stack somewhere */
                    FreeCallObject(CurrentCall);
                }
                return 0;
            }
            break;
        }

        case cmCallStateConnected:
        {
            if(stateMode == cmCallStateModeConnectedCallSetup)
            {
                TclExecute("after idle {Window delete .redial%p;faststart:terminate 0x%p}", (UINTPTR)CurrentCall->hsCall, (UINTPTR)CurrentCall);
                TclExecute("after 100 {incall:terminate {%d 0x%p}}", CurrentCall->counter, (UINTPTR)CurrentCall);

                if(!CurrentCall->sendComplete)
                /* in case the config uses auto-answer, offering will not be passed at */
                    CurrentCall->sendComplete = 1;
                ColorCall(CurrentCall);
            }
            break;
        }
        case cmCallStateDialtone:
        {
            ColorCall(CurrentCall);
            break;
        }
        case cmCallStateProceeding:
            {
                TclExecute("Window delete .redial%p", (UINTPTR) CurrentCall->hsCall);
                break;
            }
        case cmCallStateRingBack:
            {
                TclExecute("Window delete .redial%p", (UINTPTR) CurrentCall->hsCall);
                break;
            }
        case cmCallStateOffering:
            {
                int isAutoFS = ( atoi(TclGetVariable("app(options,autoAcceptFs)")) );
                
                CurrentCall->sendComplete = 1;

                ColorCall(CurrentCall);

                if(isAutoFS)
                    cmFastStartChannelsReady(hsCall);

                if((CurrentCall->scriptCall == FALSE) &&
                   atoi(TclGetVariable("app(newcall,autoAns)")))
                {
                    H450_sendNameID(CurrentCall, connected);
                    cmCallAnswer(CurrentCall->hsCall);
                }
                break;
            }
        case cmCallStateDisconnected:
            {
                /* Signal to the channel Idle proceedure not the reprint the channels */
                CurrentCall->numOfChannels = -1;
                break;
            }
        case cmCallStateWaitAddressAck:
            {
                if(atoi(TclGetVariable("app(newcall,autoAns)")) == 0)
                {
                    TclExecute(".incall.remote.canOvsp select");
                    CurrentCall->sendComplete = 0;
                }
                else
                    cmCallAnswer(hsCall);
                break;
            }
        case cmCallStateTransfering:
            CurrentCall->incoming = FALSE;
            break;
        default:
            break;
    }

    if (CurrentCall->scriptCall)
    {
        /* Advanced script handling */
        TclExecute("script:cb {cb:cm:EvCallStateChanged 0x%p %s %s}",
            (UINTPTR)CurrentCall,
            CMCallState2String((cmCallState_e)NewState),
            CMCallStateMode2String((cmCallStateMode_e)stateMode));
    }
    return 0;
}

/*******************************************************************************************
 * ColorCall
 * Description: Color the call by the confID and hCon. will also set the AppHandle of the connection
 * Input : Call - pointer to the callInfo structure of the call.
 * Output: none
 * return: none
 ******************************************************************************************/
void ColorCall(CallInfo * Call)
{
    HPROTCONN hCon;
    HAPPCONN appCon;
    BYTE CID[16], xorRes;
    INT32 size = 16;
    BYTE * con = (BYTE *) &appCon;

    /* Match the connection with a random number */
    hCon = cmGetTpktChanHandle(Call->hsCall, cmQ931TpktChannel);
    cmGetTpktChanApplHandle(hCon, &appCon);
    if((appCon == NULL) || ( (Call->hCon != hCon) && (Call->hCon != NULL) ) )
    {
        Call->hCon = hCon;
        appCon = (HAPPCONN) (rand()*rand());
        cmSetTpktChanApplHandle(hCon, appCon);
    }
    Call->hCon = hCon;

    /* create the colors */
    cmCallGetParam(Call ->hsCall, cmParamCID, 0, &size, (char*)CID);
    xorRes = CID[9] & 0xf;
    xorRes = xorRes << 4;
    TclExecute("Call:AddColor %d #%02X%02X%02X #%02X%02X%02X",
        Call->counter, CID[5]^xorRes, CID[6]^xorRes, CID[7]^xorRes, con[2], con[1], con[0]);
}

/*******************************************************************************************
 * ColorCall
 * Description: Maintain the number of current and total calls.
 * Input : raise - true = ++; talse = --
 * Output: none
 * return: none
 ******************************************************************************************/
void UpdateCallNum(BOOL raise)
{
    static UINT32 curCalls = 0;
    static UINT32 totalCalls = 0;

    if(raise)
    {
        curCalls ++;
        totalCalls++;
    }
    else
        curCalls--;
    TclExecute("test:SetCalls %d %d", curCalls, totalCalls);
}

/*******************************************************************************************
 * cmEvCallNewRate
 * Description : Notify the application about a call rate (bandwidth) change.
 * Input :  haCall - App handle to the call.
 *          hsCall - CM handle to the call.
 *          rate   - new rate of the call.
 * Output:  none
 * return:  0
 ******************************************************************************************/
int RVCALLCONV cmEvCallNewRate(IN HAPPCALL haCall,
                               IN HCALL    hsCall,
                               IN UINT32   rate)
{
    CallInfo * call = (CallInfo *) haCall;
    TclExecute("call:Log %d {Rate changed. new rate: %d}", call->counter, rate);
    return 0;
}

/********************************************************************************************
 * Call_SetRate
 * purpose : Set the rate of the call
 * syntax  : Call.SetRate <callInfo> <desiredBandwidth>
 *           <callInfo>         - Call information (counter and handle)
 *           <desiredBandwidth> - Amount of bandwidth to request
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SetRate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * Call;
    UINT32 bw;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    bw = atoi(argv[2]);
    cmCallSetRate(Call->hsCall, bw);

    return TCL_OK;
}

/*******************************************************************************************
 * cmEvNewCall
 * Description : Notify the application about new incoming call (Setup message was received).
 *               Note: after this event, CM_EvCallStateChanged with new state
 *               RV323_CM_CallStateOffering is called. The application can check wheather the setup
 *               message include fast connect element.
 * Input :  CmHCall - CM handle to the new call.
 * Output:  AppHCall - Pointer for the application handle to the new call.
 * return  : none
 ******************************************************************************************/
int RVCALLCONV cmEvNewCall(IN HAPP hApp,
                           IN HCALL hsCall,
                           OUT LPHAPPCALL lphaCall)
{
    CallInfo*   Call;
    int         scriptMode;
    BOOL        isTunneling, isParallel;
    BOOL        isOVSP, boolParam;
    int         bw;
    char*       defaultCallMode;
    char*       nextCallMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    /* Create the call's object */
    Call = (CallInfo *)AppAlloc(sizeof(CallInfo));
    memset(Call, 0, sizeof(CallInfo));
    Call->hsCall = hsCall;
    Call->counter = Call_GetCounterValue();
    Call->scriptCall = scriptMode;
    Call->incoming = TRUE;
    Call->insideState = 0;
    Call->isFastConnectCall = FALSE;
    memset(Call->rtpSessions, 0xff, (sizeof(Call->rtpSessions)));
    SetCallMedia(Call);
    UpdateCallNum(TRUE);
    *lphaCall = (HAPPCALL)Call;

    /* Sets up SupServ, does nothing if H450 is not enabled */
    H450_CreateCall(Call);

    if (scriptMode == TRUE)
    {
        /* Advanced script handling */
        TclExecute("script:cb {cb:cm:EvNewCall 0x%p}", (UINTPTR)Call);
        return 0;
    }

    /* This call is handled by the application: */

    /* Set some default incoming call parameters */
    /* H245 stage*/
    cmCallSetParam(hsCall, cmParamH245Stage, 0,
        String2CMH245Stage(TclGetVariable("app(h245,stage)")), NULL);
    /* tunneling*/
    isTunneling = atoi(TclGetVariable("app(newcall,tunneling)"));
    cmCallSetParam(hsCall, cmParamH245Tunneling, 0, isTunneling, NULL);
    /* Set parallel faststart and tunneling */
    isParallel = atoi(TclGetVariable("app(newcall,parallel)"));
    cmCallSetParam(hsCall,cmParamH245Parallel,0,isParallel,NULL);
    /* overlap sending */
    isOVSP = atoi(TclGetVariable("app(newcall,canOvsp)"));
    cmCallSetParam(hsCall, cmParamCanOverlapSending, 0, isOVSP, NULL);
    /* bandwidth */
    bw = atoi(TclGetVariable("app(test,bw)"));
    cmCallSetParam(hsCall, cmParamRate, 0, bw, NULL);
    /* set Multiplex and Maintain */
    boolParam = atoi(TclGetVariable("app(newcall,multiplexed)"));
    cmCallSetParam(Call->hsCall, cmParamIsMultiplexed, 0, boolParam, NULL);
    boolParam = atoi(TclGetVariable("app(newcall,maintain)"));
    cmCallSetParam(Call->hsCall, cmParamShutdownEmptyConnection, 0, !boolParam, NULL);
    
    /* Get call mode parameter from options window */
    defaultCallMode = TclGetVariable("app(options,defaultCallMode)");
    nextCallMode = TclGetVariable("app(options,nextCallMode)");

    /* Open the incoming call window if we're not in auto-answer mode and pop-up are enabled */
    if ( (atoi(TclGetVariable("app(newcall,autoAns)")) == 0) &&
         (atoi(TclGetVariable("app(options,popUp)"))  != 0) )
    {
        TclExecute("Window open .incall\n"
                   ".incall.buttons.callInfo configure -text {%d 0x%p}",
                   Call->counter, (UINTPTR)Call);
        TclExecute("Call:addWindowToList 0x%p .incall",(UINTPTR)Call);
        PrepareIncomingCallWindow(Call);
    }

    /* Open call information window if we have to */
    if (atoi(TclGetVariable("app(options,autoOpenCall)")) != 0)
    {
        TclExecute("Window open .call .%d {Call: %d}",
                   Call->counter, Call->counter);

    }
    return 0;
}


/********************************************************************************************
 * cmEvCallIncompleteAddress
 * purpose : Notify the application at the outgoing side, when setup Ack  was received,
 *           which means that address is incomplete,
 *           and the application should use the api function CM_CallSendAdditionalAddress
 *           to send addtional information to the remote.
 * Input : haCall - The Application handle of the call.
 *         hsCall - The Stack handle of the call.
 * Output: (-)
 * Return: If an error occurs, the function should return a negative value.
 *         If no error occurs, the function should return a non-negative value.
 ********************************************************************************************/
int RVCALLCONV cmEvCallIncompleteAddress(IN  HAPPCALL    haCall,
                                         IN  HCALL       hsCall)
{
    CallInfo* Call;


    Call = (CallInfo *)haCall;
    Call->hsCall = hsCall;

    if (Call->scriptCall)
    {
        TclExecute("script:cb {cb:cm:EvCallIncompleteAddress 0x%p}", (UINTPTR)Call);
        return 0;
    }

    /* Open the Redial window */
    TclExecute("Window open .redial %p\n"
        ".redial%p.callInfo configure -text {%d 0x%p}",
        (UINTPTR)Call->hsCall, (UINTPTR)Call->hsCall, Call->counter, (UINTPTR)Call);
    TclExecute("Call:addWindowToList 0x%p .redial%p",(UINTPTR)Call, (UINTPTR)Call->hsCall);
    return 0;

}


/***********************************************************************************
 * cmEvCallAdditionalAddress
 * purpose : Notify the application at the incoming side of the call that additional
 *           address was sent from the remote.
 * input   : AppHCall - Application handle to the call.
 *           CmHCall  - CM handle to the call.
 *           Address  - The additional address.
 *           SendingComplete - if True, indicates that the other side has no more digits
 *                             to send and this is the last part of the address.
 * output: none
 * return: If an error occurs, the function should return a negative value.
 *         If no error occurs, the function should return a non-negative value.
 ***********************************************************************************/
int RVCALLCONV cmEvCallAdditionalAddress(IN HAPPCALL        haCall,
                                         IN HCALL           hsCall,
                                         IN char*           Address,
                                         IN BOOL            SendingComplete)
{
    CallInfo* Call;
    Call = (CallInfo *)haCall;

    if (Call->scriptCall)
    {
        TclExecute("script:cb {cb:cm:EvCallAdditionalAddress 0x%p %s %d}",
                   (UINTPTR)Call,
                   Address,
                   SendingComplete);
        return 0;
    }

    if (!Call->sendComplete && (atoi(TclGetVariable("app(options,popUp)")) != 0))
    {
        TclExecute("Window open .incall\n"
                   ".incall.buttons.callInfo configure -text {%d 0x%p}\n"
                   ".incall.info.sourDest.lis insert end {Digits: %s}",
                   Call->counter, (UINTPTR)Call,
                   Address);
        PrepareIncomingCallWindow(Call);
        if(SendingComplete)
            TclExecute(".incall.remote.sendCmplt select");
        else
            TclExecute(".incall.remote.sendCmplt deselect");
        TclExecute(".incall.info.sourDest.lis insert 0 {Digits: %s}",
            Address);
        TclExecute("Call:addWindowToList 0x%p .incall",(UINTPTR)Call);
    }

    return 0;

}


/********************************************************************************************
 * cmEvCallStatus
 * purpose : Notify the application when the a new Status message is received.
 *           The status message may be a response to Status Inquiry message that was
 *           sent, or as response to an unknown message that the remote side received.
 * input   : haCall - Application handle to the call.
 *           hsCall - CM handle to the call.
 *           callStatusMsg - status message.
 * output  : callStatusMsg - status message.
 * return  : negative on error.
 ********************************************************************************************/
int RVCALLCONV cmEvCallStatus(IN    HAPPCALL haCall,
                              IN    HCALL hsCall,
                              OUT IN    cmCallStatusMessage *callStatusMsg)
{
    CallInfo* Call;

    Call = (CallInfo *)haCall;

    TclExecute("call:Log %d {Received STATUS (state=%d %d, cause=%d %d %d %d %d, display=%s)}", Call->counter,
        callStatusMsg->callStateInfo.cmCallStateValue, callStatusMsg->callStateInfo.cmCodingStandard,
        callStatusMsg->callCauseInfo.cmCauseValue , callStatusMsg->callCauseInfo.cmCodingStandard,
        callStatusMsg->callCauseInfo.cmLocation, callStatusMsg->callCauseInfo.cmRecomendation,
        callStatusMsg->callCauseInfo.cmSpare, callStatusMsg->display);
    return 0;
}


/********************************************************************************************
 * Call_GetWindowHandle
 * purpose : get the handle of the call window
 * syntax  : Call.GetWindowHandle <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 *           Thif procedure also sets a result string for TCL, holding the call window's handle
 ********************************************************************************************/
int Call_GetWindowHandle(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int     CallNum;
    char    buf[16];

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%d 0x%*p:", &CallNum) != 1) return TCL_OK;

    /* Parse call handle and set result for this procedure */
    sprintf(buf, "call%d", CallNum);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);

    return TCL_OK;
}




/********************************************************************************************
 * Call_DisplayInfo
 * purpose : Display the information of a call inside the call information window
 * syntax  : Call.DisplayInfo <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_DisplayInfo(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo    *Call;
    char        chanFrame[16];

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    sprintf(chanFrame, ".call%d.chan", Call->counter);
    DisplayChannelList(Call, chanFrame);

    return TCL_OK;
}


/********************************************************************************************
 * Call_Drop
 * purpose : Drop the current selected call
 * syntax  : Call.drop <callInfo> <reason>
 *           <callInfo> - Call information (counter and handle)
 *           <reason>   - Reason string for dropping the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Drop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo     *CurrentCall;
    cmReasonType dropReasonType = cmReasonTypeUndefinedReason;
    char         reason[32];

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Choose the drop reason */
    switch (atoi(argv[2]))
    {
        case 0:
            dropReasonType = cmReasonTypeNoBandwidth;
            strcpy(reason,"NoBandwidth");
            break;

        case 1:
            dropReasonType = cmReasonTypeGatekeeperResource;
            strcpy(reason,"GatekeeperResource");
            break;
        case 2:
            dropReasonType = cmReasonTypeUnreachableDestination;
            strcpy(reason,"UnreachableDestination");
            break;
        case 3:
            dropReasonType = cmReasonTypeDestinationRejection;
            strcpy(reason,"DestinationRejection");
            break;
        case 4:
            dropReasonType = cmReasonTypeInvalidRevision;
            strcpy(reason,"InvalidRevision");
            break;
        case 5:
            dropReasonType = cmReasonTypeNoPermision;
            strcpy(reason,"NoPermision");
            break;
        case 6:
            dropReasonType = cmReasonTypeUnreachableGatekeeper;
            strcpy(reason,"UnreachableGatekeeper");
            break;
        case 7:
            dropReasonType = cmReasonTypeGatewayResource;
            strcpy(reason,"GatewayResource");
            break;
        case 8:
            dropReasonType = cmReasonTypeBadFormatAddress;
            strcpy(reason,"BadFormatAddress");
            break;
        case 9:
            dropReasonType = cmReasonTypeAdaptiveBusy;
            strcpy(reason,"AdaptiveBusy");
            H450_sendNameID(CurrentCall, busy);
            break;
        case 10:
            dropReasonType = cmReasonTypeInConf;
            strcpy(reason,"InConf");
            break;
        case 11:
            dropReasonType = cmReasonTypeUndefinedReason;
            strcpy(reason,"UndefinedReason");
            break;
        case 12:
            dropReasonType = cmReasonTypeRouteCallToGatekeeper;
            strcpy(reason,"RouteCallToGatekeeper");
            break;
        case 13:
            dropReasonType = cmReasonTypeCallForwarded;
            strcpy(reason,"CallForwarded");
            break;
        case 14:
            dropReasonType = cmReasonTypeRouteCallToMC;
            strcpy(reason,"RouteCallToMC");
            break;
        case 15:
            dropReasonType = cmReasonTypeFacilityCallDeflection;
            strcpy(reason,"FacilityCallDeflection");
            break;
        case 16:
            dropReasonType = cmReasonTypeSecurityDenied;
            strcpy(reason,"SecurityDenied");
            break;
        case 17:
            dropReasonType = cmReasonTypeCalledPartyNotRegistered;
            strcpy(reason,"CalledPartyNotRegistered");
            break;
        case 18:
            dropReasonType = cmReasonTypeCallerNotregistered;
            strcpy(reason,"CallerNotregistered");
            break;
        case 19:
            dropReasonType = cmReasonTypeConferenceListChoice;
            strcpy(reason,"ConferenceListChoice");
            break;
        case 20:
            dropReasonType = cmReasonTypeStartH245;
            strcpy(reason,"StartH245");
            break;
        case 21:
            dropReasonType = cmReasonTypeNewConnectionNeeded;
            strcpy(reason,"NewConnectionNeeded");
            break;
        case 22:
            dropReasonType = cmReasonTypeNoH245;
            strcpy(reason,"NoH245");
            break;
    }

    /* Signal to the channel Idle proceedure not the reprint the channels */
    CurrentCall->numOfChannels = -1;
    /* Drop call from the stack with the given reason */
    TclExecute("call:Log %d {Dropping. reason=%s}", CurrentCall->counter, reason);
    cmCallDropParam(CurrentCall->hsCall, dropReasonType);

    return TCL_OK;
}


/********************************************************************************************
 * Call_SendCallProceeding
 * purpose : send call proceeding message
 * syntax  : Call.SendCallProceeding <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendCallProceeding(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int         retVal;
    CallInfo*   CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Send call proceeding message */
    retVal = cmCallSendCallProceeding(CurrentCall->hsCall);

    TclExecute("call:Log %d {Proceeding. result=%s}", CurrentCall->counter, Status2String(retVal));

    return TCL_OK;
}


/********************************************************************************************
 * Call_SendAlerting
 * purpose : send alert message
 * syntax  : Call.SendAlerting <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendAlerting(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int         retVal;
    CallInfo*   CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Send alerting message */
    H450_sendNameID(CurrentCall, alerting);
    retVal = cmCallAccept(CurrentCall->hsCall);
    if( (retVal>=0) && atoi(TclGetVariable("app(h450,alertingPrk)")) )
        H450_groupNotification(CurrentCall, CurrentCall->counter, TRUE);

    TclExecute("call:Log %d {Alerting. result=%s}", CurrentCall->counter, Status2String(retVal));

    return TCL_OK;
}


/********************************************************************************************
 * Call_SendConnect
 * purpose : send connect message
 * syntax  : Call.SendConnect <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendConnect(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int retVal;
    CallInfo * CurrentCall;
    char * UserUser,* Display;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    sscanf(argv[1], "%*d 0x%p", &CurrentCall);

    /* see if Ad-Hoc confrencing is indicated */
    if(atoi(TclGetVariable("tmp(adHoc,use)")))
    {
        CallInfo * SameConferenceCall;
        char * selectedCall = TclGetVariable("tmp(adHoc,call)");
        TclSetVariable("tmp(adHoc,use)", "0");

        if (sscanf(selectedCall, "%*d 0x%p:", &SameConferenceCall) == 1)
            if (cmCallJoin(CurrentCall->hsCall, SameConferenceCall->hsCall) >= 0)
                return TCL_OK;
    }

    /* Get default UserUser and Display fields */
    UserUser = TclGetVariable("app(options,userUser)");
    Display = TclGetVariable("app(options,display)");

    /* Send connect message */
    H450_sendNameID(CurrentCall, connected);
    retVal = cmCallAnswerExt(CurrentCall->hsCall, Display, UserUser, strlen(UserUser) );

    TclExecute("call:Log %d {Connect. result=%s}", CurrentCall->counter, Status2String(retVal));
    return TCL_OK;
}


/********************************************************************************************
 * Call_SendStatusInquiry
 * purpose : Send Status Inquiry message
 * syntax  : Call.SendStatusInquiry <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendStatusInquiry(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int retVal;
    CallInfo* CurrentCall;
    char* Display;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    Display = TclGetVariable("app(options,display)");

    /* Send Status Inquiry message */
    retVal = cmCallStatusEnquiry(CurrentCall->hsCall,(UINT8 *)Display);

    TclExecute("call:Log %d {StatusInquiry. result=%d}", CurrentCall->counter, retVal);


    return TCL_OK;
}


/********************************************************************************************
 * Call_SendProgress
 * purpose : Send Progress message
 * syntax  : Call.SendProgress <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendProgress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    char * display;
    int nodeID;
    int status;
    cmNonStandardParam param, *p_param = NULL;

    /* Get CallInfo and display */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;
    display = TclGetVariable("app(options,display)");

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        char * nsdStr;
        TclExecute("nsd:getParam");
        nsdStr = Tcl_GetStringResult(interp);
        if(String2CMNonStandardParam(nsdStr, &param) >= 0)
            p_param = &param;
    }

    /* Create Progress message */
    nodeID = cmCallProgressCreate(CurrentCall->hsCall,
        display, -1, use30asPI_IE, -1, -1, p_param);

    /* Send Progress message */
    status = cmCallProgress(CurrentCall->hsCall, nodeID);

    TclExecute("call:Log %d {Progress. result=%s}", CurrentCall->counter, Status2String(status));

    return TCL_OK;
}

/********************************************************************************************
 * cmEvCallProgress
 * purpose : Notify application that call progress was received.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallProgress(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle)
{
    CallInfo * Call = (CallInfo *) haCall;
    char display[128], data[64];
    int cause, progressDescription, notificationDescription;
    progressInd_IE pi_IE;
    cmNonStandardParam param;
    int status;
    param.data = data;
    param.length = 64;

    status =  cmCallProgressGet(hApp, handle, display, 128, &cause, &pi_IE,
        &progressDescription, &notificationDescription, &param);

    if(status >= 0)
    {
        TclExecute("call:Log %d {Received Progress: display=%s; cause=%d; pi_IE=%d}",
            Call->counter, display, cause, pi_IE);
        if (param.length >= 0)
            TclExecute("call:Log %d {NSD: %s}", Call->counter, CMNonStandardParam2String(&param));
    }
    return status;
}



/********************************************************************************************
 * Call_SendNotify
 * purpose : Send Notify message
 * syntax  : Call.SendNotify <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendNotify(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    char * display;
    int    status, nodeID;
    cmNonStandardParam param, *p_param = NULL;

    /* Get CallInfo and display */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;
    display = TclGetVariable("app(options,display)");

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        char * nsdStr;
        TclExecute("nsd:getParam");
        nsdStr = Tcl_GetStringResult(interp);
        if(String2CMNonStandardParam(nsdStr, &param) >= 0)
            p_param = &param;
    }

    /* Create Notify message */
    nodeID = cmCallNotifyCreate(CurrentCall->hsCall, display, 7, p_param);

    /* Send Notify message */
    status = cmCallNotify(CurrentCall->hsCall, nodeID);
    TclExecute("call:Log %d {Notify. result=%s}", CurrentCall->counter, Status2String(status));

    return TCL_OK;
}

/********************************************************************************************
 * cmEvCallNotify
 * purpose : Notify application that call notify was received.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallNotify(IN HAPPCALL haCall,
                              IN HCALL    hsCall,
                              IN int      handle)
{
    CallInfo * Call = (CallInfo *) haCall;
    char display[128], data[64];
    int notificationDescription;
    cmNonStandardParam param;
    int status;
    param.data = data;
    param.length = 64;

    status =  cmCallNotifyGet(hApp, handle, display, 128, &notificationDescription, &param);

    if(status >= 0)
    {
        TclExecute("call:Log %d {Received Notify: display=%s; description=%d}",
            Call->counter, display, notificationDescription);
        if (param.length >= 0)
            TclExecute("call:Log %d {NSD: %s}", Call->counter, CMNonStandardParam2String(&param));
    }
    return status;
}

/********************************************************************************************
 * Call_SendUserInformation
 * purpose : Send user info message
 * syntax  : Call.SendNotify <callInfo> <display>
 *           <callInfo> - Call information (counter and handle)
 *           <display> - Display string
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendUserInformation(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    char * display;
    cmNonStandardParam param, *p_param = NULL;
    int    status, nodeID;

    /* Get CallInfo and display */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;
    display = argv[2];

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        char * nsdStr;
        TclExecute("nsd:getParam");
        nsdStr = Tcl_GetStringResult(interp);
        if(String2CMNonStandardParam(nsdStr, &param) >= 0)
            p_param = &param;
    }

    /* Create Notify message */
    nodeID = cmCallUserInformationCreate(CurrentCall->hsCall,  display, p_param);

    /* Send Notify message */
    status = cmCallUserInformationSend(CurrentCall->hsCall, nodeID);
    TclExecute("call:Log %d {UserInfo. result=%s}", CurrentCall->counter, Status2String(status));

    return TCL_OK;
}

/********************************************************************************************
 * cmEvCallUserInfo
 * purpose : Notify application that user info was received.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallUserInfo(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle)
{
    CallInfo * Call = (CallInfo *) haCall;
    char display[128], data[64];
    cmNonStandardParam param;
    int status;
    param.data = data;
    param.length = 64;

    status =  cmCallUserInformationGet(hApp, handle, display, 128, &param);

    if(status >= 0)
    {
        TclExecute("call:Log %d {Received User Information: display=%s}",
            Call->counter, display);
        if (param.length >= 0)
            TclExecute("call:Log %d {NSD: %s}", Call->counter, CMNonStandardParam2String(&param));
    }
    return status;
}

/********************************************************************************************
 * Call_CreateH245
 * purpose : Create an H245 session is got ip and port, listen for an H245 connection if not.
 * syntax  : Call.CreateH245 <callInfo> <ipAddress>
 *           <callInfo>  - Call information (counter and handle)
 *           <ipAddress> - Address to connect to. if not present, will listen.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_CreateH245(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    int    status, zero;
    cmTransportAddress trans = {0}, *pTrans = NULL;
    char addrStr[64];

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    if( argv[2][0] && (String2TransportAddress(argv[2], &trans) >= 0) )
        pTrans = &trans;
    zero = trans.ip;

    /* create/lisen control session */
    status = cmCallCreateControlSession(CurrentCall->hsCall, pTrans);
    TclExecute("call:Log %d {CallCreateControlSession. result=%s}", CurrentCall->counter,
        Status2String(status));

    if(pTrans)
    {
        if(zero==0)
        {
            TransportAddress2String(pTrans, addrStr);
            TclExecute("set tmp(h245,address) {%s}", addrStr);
        }
        else TclExecute("set tmp(h245,address) {0.0.0.0:0}");
    }

    return TCL_OK;
}


/********************************************************************************************
 * Call_LoopOff
 * purpose : Releases all media loops in this call. Sends a maintenanceLoopOffCommand message
 *           to the remote terminal.
 * syntax  : Call.LoopOff <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_LoopOff(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    int    status;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Send LoopOff message */
    status = cmCallMediaLoopOff(CurrentCall->hsCall);
    TclExecute("call:Log %d {LoopOff. result=%s}", CurrentCall->counter, Status2String(status));

    return TCL_OK;
}


/********************************************************************************************
 * Call_ConnectControl
 * purpose : Request to establish H245 control channel.
 *           This function may be called after call was established with fast start,
 *           and h245 connection wasn't opened yet
 * syntax  : Call.ConnectControl <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_ConnectControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    cmCallConnectControl(CurrentCall->hsCall);

    return TCL_OK;
}

/********************************************************************************************
 * Call_SeperateControl
 * purpose : Request to establish H245 control channel.
 *           This function may be called after call was established with tunneling,
 *           and h245 connection wasn't opened yet
 * syntax  : Call.SeperateControl <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SeperateControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    cmCallSw2SeparateH245(CurrentCall->hsCall);

    return TCL_OK;
}

/********************************************************************************************
 * Call_CloseControl
 * purpose : Request to close H245 control channel.
 * syntax  : Call.CloseControl <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_CloseControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    cmCallCloseControlSession(CurrentCall->hsCall);

    return TCL_OK;
}

/********************************************************************************************
 * Call_SendCaps
 * purpose : Request to send a new capabilities message.
 * syntax  : Call.SendCaps <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendCaps(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    int nodeId;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Taken from Test.c: gets the configuration nodeID for the capability struct */
    nodeId=pvtGetNodeIdByPath(cmGetValTree(hApp),cmGetH245ConfigurationHandle(hApp),"capabilities.terminalCapabilitySet");
    /* Sends the capability struct */
    cmCallSendCapability(CurrentCall->hsCall, nodeId);

    return TCL_OK;
}

/********************************************************************************************
 * Call_SendEmptyCaps
 * purpose : Request to send an empty capabilities message.
 * syntax  : Call.SendEmptyCaps <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendEmptyCaps(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    int        emptyCaps;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    emptyCaps = pvtAddRoot (cmGetValTree(hApp), cmGetSynTreeByRootName(hApp, "h245"), 0, NULL);
    if (emptyCaps < 0)
        return TCL_ERROR;

    if (cmCallSendCapability (CurrentCall->hsCall, emptyCaps) < 0)
      return TCL_ERROR;

    pvtDelete(cmGetValTree(hApp), emptyCaps);
    return TCL_OK;
}

/********************************************************************************************
 * Call_SendCapsAck
 * purpose : Request to send a new capabilities ack message.
 * syntax  : Call.SendCapsAck <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendCapsAck(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Sends the capability struct */
    cmCallCapabilitiesAck(CurrentCall->hsCall);

    return TCL_OK;
}

/********************************************************************************************
 * Call_SendMSD
 * purpose : Request to send a new master slave determenition message.
 * syntax  : Call.SendMSD <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendMSD(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CurrentCall;
    int termType = 50;
    int node, synNode;
    INTPTR fieldID;
    BOOL string;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Sends the capability struct */
    node = pvtGetNodeIdByPath(cmGetValTree(hApp),cmGetH245ConfigurationHandle(hApp),"masterSlave.terminalType");
    pvtGet(cmGetValTree(hApp), node, &fieldID, &synNode, &termType, &string);
    cmCallMasterSlaveDetermine(CurrentCall->hsCall, termType);

    return TCL_OK;
}

/********************************************************************************************
 * Call_UII
 * purpose : Request to send a user input indication.
 * syntax  : Call.UII <callInfo> <user input>
 *           <callInfo> - Call information (counter and handle)
 *           <user input> - Input from the user.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_UII(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int nodeId;
    CallInfo * CurrentCall;
    cmNonStandardParam param;
    cmNonStandardIdentifier *p_info = NULL;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        char * nsdStr;
        TclExecute("nsd:getParam");
        nsdStr = Tcl_GetStringResult(interp);
        if(String2CMNonStandardParam(nsdStr, &param) >= 0)
            p_info = &param.info;
    }

    /* create the UII node ID */
    nodeId = cmUserInputBuildNonStandard(hApp, p_info, argv[2], strlen(argv[2]));

    /* Sends the UII */
    cmCallSendUserInput(CurrentCall->hsCall, nodeId);

    return TCL_OK;
}

int RVCALLCONV cmEvCallUserInput(IN HAPPCALL haCall,
                                 IN HCALL hsCall,
                                 IN INT32 userInputId)
{
    char display[128];
    INT32 len = 128;
    cmNonStandardParam nsData;
    cmUserInputData userData;
    char userDataStr[128];
    userData.data = userDataStr;
    userData.length = 128;

    cmUserInputGet(hApp, userInputId, &nsData.info, display, &len, &userData);

    if (userData.length) TclExecute("test:Log {UII received: %s}", userData.data);
    if (len) TclExecute("test:Log {Display: %s}", display);
    return 0;
}

/********************************************************************************************
 * CreateCallObject
 * purpose : Create a complete call object, and sets outgoing params.
 * input   : none
 * output  : none
 * return  : the CallInfo struct of the call, NULL if failed.
 ********************************************************************************************/
CallInfo * CreateCallObject(void)
{
    CallInfo * Call;
    int retVal;
    HCALL hsCall;

    Call = (CallInfo*)AppAlloc(sizeof(CallInfo));
    memset(Call, 0, sizeof(CallInfo));
    /* Create new call */
    retVal = cmCallNew(hApp,(HAPPCALL) Call, &hsCall);
    if (retVal < 0)
    {
        AppFree(Call);
        return NULL;
    }
    UpdateCallNum(TRUE);
    Call->counter = Call_GetCounterValue();
    Call->scriptCall = FALSE;
    Call->incoming = FALSE;
    Call->isFastConnectCall = FALSE;
    Call->hsCall = hsCall;
    memset(Call->rtpSessions, 0xff, (sizeof(Call->rtpSessions)));

    /* Set up SupServ */

    if(H450_CreateCall(Call) < 0)
    {
        TclExecute("test:Log {Error creating H450 object}");
        cmCallDrop(Call->hsCall);
        return NULL;
    }

    /* Insert the call to the list box */
    TclExecute(".test.calls.list insert end {%s}\n", CallInfoStr(Call, (char *)"Allocated"));
    TclExecute(".test.calls.conn insert end {}");
    TclExecute(".test.calls.conf insert end {}");

    return Call;
}


/********************************************************************************************
 * Call_Dial
 * purpose : Dial a new call to a destination.
 * syntax  : Call.Dial <callInfo> <aliases>
 *           <aliases box>  - location of the aliases
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Dial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char                **AliasList;
    int                 status;
    int                 AliasListSize;
    CallInfo            *CurrentCall;
    cmAlias             **aliasPtr;
    cmAlias             **destAliases;
    int                 index = 0;
    char                *GetVar, *TclAliasList;
    int                 retVal;
    cmTransportAddress  TransportAddress;

    /* Create a new call handle */
    CurrentCall = CreateCallObject();
    if(CurrentCall == NULL) return TCL_OK;

    /* Set the call parameters */
    Call_SetOutgoingParams(CurrentCall);

    TransportAddress.type = cmTransportTypeIP;

    /* get the destination address */
    GetVar = TclGetVariable("tmp(newcall,address)");
    while ((GetVar != NULL) && (strlen(GetVar) > 0))
    {
        /* now parse the address, in case Amit decided to use ETA: */
        BOOL ta=(strncmp("TA:",GetVar,3)==0);
        BOOL eta=(strncmp("ETA:",GetVar,4)==0);

        /* mover the pointer to the address itself, if needed */
        if (ta)
            GetVar += 3;
        if (eta)
            GetVar += 4;

        /* get the TA */
        status = String2TransportAddress(GetVar, &TransportAddress);

        if (status >= 0)
        {
            /* set the param */
            if (eta)
            {
                /* Set the destAnnexEAddress */
                retVal = cmCallSetParam(CurrentCall->hsCall,cmParamDestinationAnnexEAddress,0, 
                    sizeof(TransportAddress),(char*)&TransportAddress);
            }
            else
            {
                /* Set the destCallSignalAddress */
                retVal = cmCallSetParam(CurrentCall->hsCall,cmParamDestCallSignalAddress,0,
                    sizeof(TransportAddress),(char *)&TransportAddress);
                retVal = cmCallSetParam(CurrentCall->hsCall,cmParamDestinationIpAddress,0,
                    sizeof(TransportAddress),(char *)&TransportAddress);
            }
        }
        /* see if there are more addressses */
        if ((GetVar = strchr(GetVar,',')) != NULL)
            GetVar++;
    }

    /* Get the alias list from the TCL (in a somewhat wierd way */
    TclExecute("set temp [%s get 0 end]", argv[1]);
    TclAliasList = TclGetVariable("temp");
    /* Proccess the list */
    Tcl_SplitList(interp, TclAliasList, &AliasListSize, &AliasList);
    destAliases = (cmAlias**)AppAlloc((AliasListSize+1)*sizeof(cmAlias*));
    destAliases[AliasListSize] = NULL;

    /* Set destination information */
    BuildAliasList(destAliases, AliasListSize, AliasList);
    aliasPtr = destAliases;
    while (*aliasPtr != NULL)
    {
        cmCallSetParam(CurrentCall->hsCall,cmParamDestinationAddress,index,
            sizeof(cmAlias),(char *)*aliasPtr);
        index++;
        aliasPtr++;
    }
    Tcl_Free((char*)AliasList);
    FreeAliasList(destAliases);

    /* send calling name if we have to */
    H450_sendNameID(CurrentCall, calling);

    /* Initiate FastStart procedures if needs be */
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
    {
        CurrentCall->isFastConnectCall = TRUE;
        if(CurrentCall->action == RTP_Replay)
        {
            /* init all the rtp sessions, we may need them later. */
            CurrentCall->rtpSessions[0] = RTP_TestOpen("FastStartAudio");
            CurrentCall->rtpSessions[1] = RTP_TestOpen("FastStartVideo");
            CurrentCall->rtpSessions[2] = RTP_TestOpen("FastStartData");
        }
        if(atoi(TclGetVariable("app(options,autoAcceptFs)")))
            Call_InitFastStart(CurrentCall);
        else
        {
            if(CurrentCall->action == RTP_Replay)
            {
                /* set audio, video and data ports */
                TclExecute("set tmp(faststart,audioPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[0]));
                TclExecute("set tmp(faststart,videoPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[1]));
                TclExecute("set tmp(faststart,dataPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[2]));
            }
            else
            {
                /* use fictive RTP */
                TclExecute("set tmp(faststart,audioPort) 2536");
                TclExecute("set tmp(faststart,videoPort) 2536");
                TclExecute("set tmp(faststart,dataPort) 2536");
            }
            return TclExecute("Call.OpenOutgoingFaststart {%d 0x%p} {Call.FastStartDial {%d 0x%p}}",
                CurrentCall->counter, (UINTPTR) CurrentCall,
                CurrentCall->counter, (UINTPTR) CurrentCall);
        }
    }

    /* Create the call */
    if (cmCallDial(CurrentCall->hsCall) > -1)
    {
        /* Open call information window if we have to */
        if (atoi(TclGetVariable("app(options,autoOpenCall)")) != 0)
            TclExecute("Window open .call .%d {Call: %d}",
            CurrentCall->counter, CurrentCall->counter);
    }
    else
    {
        /* Make sure we close this call if we didn't succeed */
        cmEvCallStateChanged((HAPPCALL)CurrentCall, CurrentCall->hsCall, cmCallStateIdle, 0);
    }
    return TCL_OK;
}

/********************************************************************************************
 * Call_FastStart_Dial
 * purpose : Finishes the Call_Dial procces after the user has selected channels.
 * syntax  : Call.FastStart.Dial <callInfo>
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_FastStartDial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo            *CurrentCall;

    /* Get the call handle */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Create the call */
    if (cmCallDial(CurrentCall->hsCall) > -1)
    {
        /* Open call information window if we have to */
        if (atoi(TclGetVariable("app(options,autoOpenCall)")) != 0)
            TclExecute("Window open .call .%d {Call: %d}",
            CurrentCall->counter, CurrentCall->counter);
    }
    else
    {
        /* Make sure we close this call if we didn't succeed */
        cmEvCallStateChanged((HAPPCALL)CurrentCall, CurrentCall->hsCall, cmCallStateIdle, 0);
    }
    return TCL_OK;
}

/********************************************************************************************
 * Call_Make
 * purpose : Make a quick new call to a destination.
 * syntax  : Call.Make <callInfo> <aliases>
 *           <alias> - whom to call
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Make(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo            *CurrentCall;
    char                destAddr[256], *UU;
    BOOL                boolParam;

    /* Create a new call handle */
    CurrentCall = CreateCallObject();
    if(CurrentCall == NULL) return TCL_OK;

    /* Set the call parameters */
    boolParam = atoi(TclGetVariable("app(newcall,canOvsp)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamCanOverlapSending,0,boolParam,NULL);
    boolParam = atoi(TclGetVariable("app(newcall,parallel)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamH245Parallel,0,boolParam,NULL);
    boolParam = atoi(TclGetVariable("app(newcall,tunneling)"));
    cmCallSetParam(CurrentCall->hsCall,cmParamH245Tunneling,0,boolParam,NULL);
    /* H245 stage*/
    cmCallSetParam(CurrentCall->hsCall, cmParamH245Stage, 0,
        String2CMH245Stage(TclGetVariable("app(h245,stage)")), NULL);
    /* set Multiplex and Maintain */
    boolParam = atoi(TclGetVariable("app(newcall,multiplexed)"));
    cmCallSetParam(CurrentCall->hsCall, cmParamIsMultiplexed, 0, boolParam, NULL);
    boolParam = atoi(TclGetVariable("app(newcall,maintain)"));
    cmCallSetParam(CurrentCall->hsCall, cmParamShutdownEmptyConnection, 0, !boolParam, NULL);

    if(isdigit((int)argv[1][0]))
    {
        int a, b, c, d, e;
        if(sscanf(argv[1], "%d.%d.%d.%d:%d", &a, &b, &c, &d, &e) == 5)
            sprintf(destAddr, "TA:%s", argv[1]);
        else sprintf(destAddr, "TEL:%s", argv[1]);
    }
    else strcpy(destAddr, argv[1]);

    /* send calling name if we have to */
    H450_sendNameID(CurrentCall, calling);

    /* see if Ad-Hoc conferencing is indicated */
    if(atoi(TclGetVariable("tmp(adHoc,use)")))
    {
        CallInfo * SameConferenceCall;
        char * selectedCall = TclGetVariable("tmp(adHoc,call)");

        if (sscanf(selectedCall, "%*d 0x%p:", &SameConferenceCall) == 1)
            cmCallInvite(CurrentCall->hsCall, SameConferenceCall->hsCall);
        TclSetVariable("tmp(adHoc,use)", "0");
    }

    /* see if Connection Re-use is indicated */
    if(atoi(TclGetVariable("tmp(multiplex,use)")))
    {
        CallInfo * SameConnectionCall;
        char * selectedCall = TclGetVariable("tmp(multiplex,call)");

        if (sscanf(selectedCall, "%*d 0x%p:", &SameConnectionCall) == 1)
            cmCallMultiplex(CurrentCall->hsCall, SameConnectionCall->hsCall);
        TclExecute("$tmp(basic,base).cnc.useMultiplex invoke");
    }

    /* set call media, if supported */
    SetCallMedia(CurrentCall);

    /* Initiate FastStart Proceedures if needs be */
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
    {
        CurrentCall->isFastConnectCall = TRUE;
        if(CurrentCall->action == RTP_Replay)
        {
            /* init all the rtp sessions, we may need them later. */
            CurrentCall->rtpSessions[0] = RTP_TestOpen("FastStartAudio");
            CurrentCall->rtpSessions[1] = RTP_TestOpen("FastStartVideo");
            CurrentCall->rtpSessions[2] = RTP_TestOpen("FastStartData");
        }
        if(atoi(TclGetVariable("app(options,autoAcceptFs)")))
            Call_InitFastStart(CurrentCall);
        else
        {
            if(CurrentCall->action == RTP_Replay)
            {
                /* set audio, video and data ports */
                TclExecute("set tmp(faststart,audioPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[0]));
                TclExecute("set tmp(faststart,videoPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[1]));
                TclExecute("set tmp(faststart,dataPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[2]));
            }
            else
            {
                /* use fictive RTP */
                TclExecute("set tmp(faststart,audioPort) 2536");
                TclExecute("set tmp(faststart,videoPort) 2536");
                TclExecute("set tmp(faststart,dataPort) 2536");
            }
            return TclExecute("Call.OpenOutgoingFaststart {%d 0x%p} {Call.FastStartMake {%d 0x%p} %s}",
                CurrentCall->counter, (UINTPTR) CurrentCall,
                CurrentCall->counter, (UINTPTR) CurrentCall, destAddr);
        }
    }

    /* Get UUI */
    UU = TclGetVariable("app(options,userUser)");

    /* Create the call */
    if (cmCallMake(CurrentCall->hsCall,
        atoi(TclGetVariable("app(test,bw)")), 0,
        destAddr, NULL,
        TclGetVariable("app(options,display)"),
        UU, strlen(UU) ) > -1)
    {
        /* Open call information window if we have to */
        if (atoi(TclGetVariable("app(options,autoOpenCall)")) != 0)
            TclExecute("Window open .call .%d {Call: %d}",
            CurrentCall->counter, CurrentCall->counter);
    }
    else
    {
        TclExecute("test:Log {Call Make failed}");
        /* Make sure we close this call if we didn't succeed */
        cmEvCallStateChanged((HAPPCALL)CurrentCall, CurrentCall->hsCall, cmCallStateIdle, 0);
    }
    return TCL_OK;
}

/********************************************************************************************
 * Call_FastStartMake
 * purpose : Finishes the Call_Make procces after the user has selected channels.
 * syntax  : Call.FastStart.Dial <callInfo> <destAddress> (formatted)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_FastStartMake(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo          * CurrentCall;
    char *              UU;

    /* Get the call handle */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Get UUI */
    UU = TclGetVariable("app(options,userUser)");

    /* Create the call */
    if (cmCallMake(CurrentCall->hsCall,
        0, atoi(TclGetVariable("app(test,bw)")),
        argv[2], NULL,
        TclGetVariable("app(options,display)"),
        UU, strlen(UU) ) > -1)
    {
        /* Open call information window if we have to */
        if (atoi(TclGetVariable("app(options,autoOpenCall)")) != 0)
            TclExecute("Window open .call .%d {Call: %d}",
            CurrentCall->counter, CurrentCall->counter);
    }
    else
    {
        /* Make sure we close this call if we didn't succeed */
        cmEvCallStateChanged((HAPPCALL)CurrentCall, CurrentCall->hsCall, cmCallStateIdle, 0);
    }
    return TCL_OK;
}


/********************************************************************************************
 * Call_OpenOutgoingFaststart
 * purpose : Open faststart options for an outgoing call
 * syntax  : Call.OpenOutgoingFaststart <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_OpenOutgoingFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int retVal;
    CallInfo * CurrentCall;
    char *DataTypeNameList[20], DataTypeNames[20][20];
    char TclDataType[20*20], *temp;
    int i;

    /* Make sure we've got a call */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* preparing DataTypeNameList */
    for (i=0;i<20;i++)
        DataTypeNameList[i] = DataTypeNames[i];

    /* get DataType name list from configuration file */
    retVal = cmGetConfigChannels(hApp, 20, 20, DataTypeNameList);

    if (retVal < 0)
        return TCL_OK;

    /*preparing string of DataTypes for the combo box in the fast start window*/
    strcpy(TclDataType,DataTypeNameList[0]);
    temp = TclDataType;
    if (retVal > 20) retVal = 20;
    for(i=1;i<retVal;i++)
    {
        temp = temp + strlen(DataTypeNameList[i-1])-1 ;
        *(++temp) = ' ';
        ++temp;
        strcpy(temp,DataTypeNameList[i]);
    }

    TclExecute("Window open .faststart {%s} 0 {%s} {%s}", argv[1], TclDataType, argv[2]);
    TclExecute("Call:addWindowToList 0x%p .faststart",(UINTPTR)CurrentCall);

    return TCL_OK;
}

/********************************************************************************************
 * Call_BuildOutgoingFaststart
 * purpose : build the faststart message with the new API.
 * syntax  : Call.BuildOutgoingFaststart <callInfo> <followUp>
 *           <callInfo> - Call information (counter and handle)
 *           <followUp> - the command that will be executed after done
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_BuildOutgoingFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*                   CurrentCall;
    int                         i, status, FSnode;
    cmCapDataType               DataType;
    cmChannelDirection          direction = (cmChannelDirection)-1;
    cmFastStartChannel          fsChannel;
    char                        *line, name[32], dir[8], rtp[32], rtcp[32];

    /* Make sure we've got a call */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    for (i = 0; ; i++)
    {
        TclExecute(".faststart.chan.lb get %d", i);
        line = Tcl_GetStringResult(interp);
        if(sscanf(line, "%s : %s ; %s , %s", name, dir, rtcp, rtp) != 4) break;

        if(dir[0] == 'I') direction = dirReceive;
        else if(dir[0] == 'O') direction = dirTransmit;
        else if(dir[0] == 'B') direction = dirBoth;

        String2TransportAddress(rtp,  &fsChannel.rtp);
        String2TransportAddress(rtcp, &fsChannel.rtcp);
        fsChannel.dataTypeHandle = -1;
        fsChannel.channelName = name;
        switch (tolower(name[0])) /* todo: fix someday */
        {
        case 'g':
            DataType = cmCapAudio;
            break;
        case 'h':
            DataType = cmCapVideo;
            break;
        case 't':
            DataType = cmCapData;
            direction = dirBoth; /* support bi-direction */
            break;
        default:
            DataType = cmCapEmpty;
            break;
        }
        FSnode = cmFastStartBuild(CurrentCall->hsCall, DataType, direction, &fsChannel);
        if(FSnode >= 0)
        {
            status = cmCallAddFastStartMessage(CurrentCall->hsCall, FSnode);
            if(status < 0)
            {
                pvtDelete(cmGetValTree(hApp), FSnode);
                return(status);
            }
        }
        else
            return(FSnode);
    }

    /* Close this window when we're done... */
    TclExecute("after idle {Window delete .faststart}");
    return TclExecute(argv[2]);
}


/********************************************************************************************
 * Call_ApproveFaststart
 * purpose : approve the channels that are going to be opened in faststart
 * syntax  : Call.ApproveFaststart <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_ApproveFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{

    int                         i, index;
    CallInfo*                   CurrentCall;
    cmTransportAddress          rtp, rtcp;
    char                        *line, name[32], dir[8], rtpS[32], rtcpS[32];

    /* Make sure we've got a call */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "0x%p", &CurrentCall) != 1) return TCL_OK;

    for (i = 0; ; i++)
    {
        TclExecute(".faststart.chan.lb get %d", i);
        line = Tcl_GetStringResult(interp);
        if(sscanf(line, "%d. %s : %s ; %s , %s", &index, name, dir, rtcpS, rtpS) != 5) break;

        String2TransportAddress(rtcpS, &rtcp);
        String2TransportAddress(rtpS,  &rtp);
        cmFastStartChannelsAckIndex(CurrentCall->hsCall, index, &rtcp, &rtp);
    }

    /* Respond to the request */
    cmFastStartChannelsReady(CurrentCall->hsCall);

    TclExecute("Window close .faststart");

    return TCL_OK;
}

/********************************************************************************************
 * Call_RefuseFaststart
 * purpose : Refuse the channels that are going to be opened in faststart
 * syntax  : Call.RefuseFaststart <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_RefuseFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*                   CurrentCall;
    
    /* Make sure we've got a call */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "0x%p", &CurrentCall) != 1) return TCL_OK;

     /* reject the FS */
    cmFastStartChannelsRefuse(CurrentCall->hsCall);
    
    TclExecute("Window close .faststart");
    
    return TCL_OK;
}
    

/***********************************************************************************
 * Routine Name: cmEvCallFastStart
 * Description : Does nothing
 * Input : haCall  - Application handle to the call.
 *         hsCall   - CM handle to the call.
 * Output: (-)
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallFastStart(IN  HAPPCALL                haCall,
                                 IN  HCALL                   hsCall)
{
    CallInfo                    *CurrentCall;
    BOOL                        bIsAuto, bAcceptFS;
    char                        rtpAddr[32], rtcpAddr[32], string[128];
    const char                  *dir;
    int                         i;
	cmTransportAddress          ownCSAddr;
    int                         channelId = 0;
    cmCapDataType               dataType;
    cmChannelDirection          chanDir;
    cmFastStartChannel          fsChan;

    CurrentCall = (CallInfo*)haCall;
    CurrentCall->hsCall = hsCall;
    CurrentCall->isFastConnectCall = TRUE;
    
    bAcceptFS = atoi(TclGetVariable("app(newcall,fastStart)"));
    if(!bAcceptFS) return 0; /* if we do not support FS, do nothing */
    
    cmGetLocalCallSignalAddress(hApp, &ownCSAddr);

    bIsAuto = ( atoi(TclGetVariable("app(options,autoAcceptFs)")) );
    if (atoi(TclGetVariable("app(newcall,autoAns)")) && (!bIsAuto))
        return 0; /* We ignore FastStart if it's not automatic when AutoAnswer is on */

    if (bIsAuto)
    {/* answer all channels automatically */
        for ( i=0; channelId >= 0; i++)
        {
            channelId = cmFastStartGetByIndex(hsCall, i);
            if(channelId < 0) break;
            cmFastStartGet(hsCall, channelId, &dataType, &chanDir, &fsChan);

            fsChan.rtcp.ip = ownCSAddr.ip;
            fsChan.rtcp.port = 0;
            fsChan.rtp.ip = ownCSAddr.ip;
            fsChan.rtp.port = 0;

            cmFastStartChannelsAckIndex(hsCall, i, &fsChan.rtcp, &fsChan.rtp);
        }
        return 0;
    }

    /* user answers all channels */
    TclExecute("Window open .faststart 0x%p 1 {} {}",(UINTPTR)CurrentCall);
    TclExecute("Call:addWindowToList 0x%p .faststart",(UINTPTR)CurrentCall);

    for ( i=0; channelId >= 0; i++)
    {
        channelId = cmFastStartGetByIndex(hsCall, i);
        if(channelId < 0) break;
        cmFastStartGet(hsCall, channelId, &dataType, &chanDir, &fsChan);
        /* set colors of entries according to the fact that they are transmitted or received */
        if (chanDir == dirReceive)
        {
            dir = "In";
        }
        if (chanDir == dirTransmit)
        {
            dir = "Out";
        }
        if (chanDir == dirBoth)
        {
            dir = "Both";
        }

        fsChan.rtp.ip = ownCSAddr.ip;
        fsChan.rtcp.ip = ownCSAddr.ip;
        TransportAddress2String(&fsChan.rtcp, rtcpAddr);
        TransportAddress2String(&fsChan.rtp,  rtpAddr);
        sprintf(string, "%d. %s : %s ; %s , %s",
            i, fsChan.channelName, dir, rtcpAddr, rtpAddr);
        TclExecute(".faststart.chan.lb insert end {%s}", string);
    }
    return 0;
}


/***********************************************************************************
 * Routine Name: cmEvCallFastStartSetup
 * Description : Notify the application at the incoming side of the call that fast start
 *               proposals were received in the current message.
 *               The application can use the api function CM_CallGetFastStartProposal
 *               to retrieve the proposals one by one.
 *
 * Input : haCall  - Application handle to the call.
 *         hsCall   - CM handle to the call.
 *         fsMessage - pointer to the FS message.
 * Output: (-)
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallFastStartSetup(IN  HAPPCALL                haCall,
                                      IN  HCALL                   hsCall,
                                      OUT IN cmFastStartMessage   *fsMessage)
{
    CallInfo* CurrentCall = (CallInfo*)haCall;
    if (CurrentCall->scriptCall == TRUE)
    {
        /* Handle with the scripts... */
        TclExecute("script:cb {cb:cm:EvCallFastStartRecv 0x%p 0x%p}",
                   (UINTPTR)CurrentCall, (UINTPTR)fsMessage);
    }
    return 0;
}


/********************************************************************************************
 * Call_SendFacility
 * purpose : Send Facility message
 * syntax  : Call.SendFacility <callInfo>
 *           <callInfo>         - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_SendFacility(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*   CurrentCall;
    int         status=0;
    HPVT        hVal = cmGetValTree(hApp);
    int         tmpNodeId;
    int         uuNodeId;
    char        oid[128];
    int         oidSize;

    int nodeId = pvtAddRoot(hVal, cmGetSynTreeByRootName(hApp, (char *)"q931"), 0, NULL);

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    oidSize = sizeof(oid);
    pvtBuildByPath(hVal, nodeId, "protocolDiscriminator", 8, NULL);

    tmpNodeId = pvtBuildByPath(hVal, nodeId, "message.facility.userUser.protocolDiscriminator", 5, NULL);
    oidSize = utlEncodeOID(oidSize, oid, "itu-t recommendation h 2250 0 4");

    uuNodeId = pvtBuildByPath(hVal, pvtParent(hVal, tmpNodeId), "h323-UserInformation.h323-uu-pdu", 0, NULL);
    tmpNodeId = pvtBuildByPath(hVal, uuNodeId, "h323-message-body.facility.protocolIdentifier", oidSize, oid);
    tmpNodeId = pvtParent(hVal, tmpNodeId);

    cmCallSetParam(CurrentCall->hsCall,cmParamFacilityReason,0,cmReasonTypeUndefinedReason,NULL);

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        cmNonStandardIdentifier  info;
        int                      length;
        char *                   data;

        tmpNodeId = pvtBuildByPath(hVal, uuNodeId, "nonStandardData", 0, NULL);

        if((TclGetVariable("app(nsd,choiceVar)"))[0] == 'H')
        { /* get the H221 vars fron the MiscTab */
            info.objectLength=0;
            info.manufacturerCode = atoi(TclGetVariable("app(nsd,manVar)"));
            info.t35CountryCode = atoi(TclGetVariable("app(nsd,counVar)"));
            info.t35Extension = atoi(TclGetVariable("app(nsd,extenVar)"));
        }
        else
        { /* get the Object vars fron the MiscTab */
            strcpy(info.object,TclGetVariable("app(nsd,objectVar)"));
            info.objectLength = strlen(info.object);
        }
        /* Get the User Input string from the facility screen */
        data = TclGetVariable("tmp(facility,userInput)");
        length = strlen(data);

        cmNonStandardParameterCreate(hVal, tmpNodeId, &info, data, length);
    }
    if( (TclGetVariable("tmp(facility,type)"))[0] == 'u' )
        /* send an undefined facility message */
        status = cmCallFacility(CurrentCall->hsCall, nodeId);
    else
    {/* send a forwarding message */
        char fwdAddr[256];
        char * ipAddr = TclGetVariable("tmp(.facility.transAddress,address)");
        fwdAddr[0] = '\0';
        if (ipAddr[0]) /* if the transportAddress field has been filled */
            sprintf(fwdAddr, "TA:%s", TclGetVariable("tmp(.facility.transAddress,address)"));
        if(argv[2][0] != '\0')
        {/* if there are any aliases */
            int i,j;
            /* add after the ip, if present */
            j = strlen(fwdAddr);
            if(j)
            {
                fwdAddr[j] = ',';
                j++;
            }
            /* transfer the aliases while formatting */
            for(i=0; i <= (int) strlen(argv[2]); i++)
            {
                if(argv[2][i] == ' ') fwdAddr[j] = ',';
                else if(argv[2][i] == '{') j--;
                else if(argv[2][i] == '}') j--;
                else
                {
                    fwdAddr[j] = argv[2][i];
                    if (fwdAddr[j] == '\0')
                        break;
                }
                j++;
            }
        }
        /* sent call forward */
        status = cmCallForward(CurrentCall->hsCall, fwdAddr);
    }

    TclExecute("Window close .facility \n"
               "call:Log %d {Facility. result=%s}",
               CurrentCall->counter, Status2String(status));

    return TCL_OK;
}

/***********************************************************************************
 * Routine Name: cmEvCallFacility
 * Description : Notify the application when the a new facility message is received.
 *               The applcation should check for parameters Param and Display that
 *               they are not NULL.
 * Input : haCall - Application handle to the call.
 *         hsCall  - CM handle to the call.
 *         handle - node ID of the message.
 * Output: proceed - if set to true, the stack will process the message. o.w,
 *                   the app will
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallFacility(IN      HAPPCALL haCall,
                                IN      HCALL    hsCall,
                                IN      int      handle,
                                INOUT   BOOL*    proceed)
{
    if (haCall || hsCall || handle);
#if 0
    int status;
    cmAlias altalias;
    cmTransportAddress altaddr;
    int size = sizeof(cmTransportAddress);
    status = cmCallGetParam(hsCall, cmParamAlternativeAddress, 0,
        (INT32 *) &size, (char *) &altaddr);
    if(status < 0)
    {
        size = sizeof(cmAlias);
        status = cmCallGetParam(hsCall, cmParamAlternativeAliasAddress, 0,
            (INT32 *) &size, (char *) &altalias);
    }
    /* Make sure the application processes this facility message,
       only if it has an alternate address or alias in it. */
    *proceed = (status >= 0);
#endif
    *proceed = TRUE;
    return 0;
}


/*****************************************************************
 * Call_InitFastStart                                            *
 * Purpose: Initializes the structure that stores a list of      *
 *          proposed Fast Start channels, RTP and RTCP           *
 *          addresses. The application opens Fast Start channels *
 * input  : Call :The App handle for the call.                   *
 *****************************************************************/
int Call_InitFastStart(CallInfo * Call)
{
    int retVal;
    /* Initializes the structure */
    cmFastStartMessage fsMessage;
    UINT16 audioPort;
    
    if(Call->action == RTP_Replay)
    {
        audioPort = (UINT16)RTP_TestGetLocalPort(Call->rtpSessions[0]);
    }
    else
        audioPort = 2536;

    fsMessage.partnerChannelsNum=1;
    fsMessage.partnerChannels[0].receive.altChannelNumber=1;

    fsMessage.partnerChannels[0].receive.channels[0].rtp.port= audioPort;
    fsMessage.partnerChannels[0].receive.channels[0].rtp.ip =  0;
    fsMessage.partnerChannels[0].receive.channels[0].rtcp.port= ++audioPort;
    fsMessage.partnerChannels[0].receive.channels[0].rtcp.ip = 0;
    fsMessage.partnerChannels[0].receive.channels[0].dataTypeHandle = -1;
    fsMessage.partnerChannels[0].receive.channels[0].channelName = (char *)"g711Alaw64k";

    fsMessage.partnerChannels[0].transmit.altChannelNumber=1;

    fsMessage.partnerChannels[0].transmit.channels[0].rtp.port= 0;
    fsMessage.partnerChannels[0].transmit.channels[0].rtp.ip =  0;
    fsMessage.partnerChannels[0].transmit.channels[0].rtcp.port= audioPort;
    fsMessage.partnerChannels[0].transmit.channels[0].rtcp.ip = 0;
    fsMessage.partnerChannels[0].transmit.channels[0].dataTypeHandle = -1;
    fsMessage.partnerChannels[0].transmit.channels[0].channelName = (char *)"g711Alaw64k";

    fsMessage.partnerChannels[0].type = cmCapAudio;

    /* Opens Fast Start channels */
    retVal = cmFastStartOpenChannels(Call->hsCall, &fsMessage);
    return retVal;
}

int RVCALLCONV cmEvCallNewAnnexMMessage(IN      HAPPCALL            haCall,
                                        IN      HCALL               hsCall,
                                        IN      int                 annexMElement,
                                        IN      int                 msgType)
{
    char message[100];
    int size=100;
    int i=1;
    char NonStandardData[100];
    BOOL tunnelingRequired;
    cmTunnelledProtocolID tunnelledProtocolID;
    cmNonStandardParam param;

    param.length = (int)sizeof(NonStandardData);
    param.data = (char*)NonStandardData;
    strcpy(param.info.object, "1 1 1 1");
    param.info.objectLength = 7;

    TclExecute("test:Log {AnnexM message received}");

    cmCallGetAnnexMMessage(hApp, annexMElement,&tunnelingRequired,&tunnelledProtocolID,&param);
    while(cmCallGetAnnexMMessageContent(hApp, annexMElement,i++,&size,message)>=0)
    {
        if(size < 100) message[size] = 0;
        TclExecute("test:Log { \"%s\"}", message);
        size=100;
    }
    TclExecute("test:Log {NSD: %s}", CMNonStandardParam2String(&param));

    return 0;
}

int RVCALLCONV cmEvCallNewAnnexLMessage(IN      HAPPCALL            haCall,
                                        IN      HCALL               hsCall,
                                        IN      int                 annexLElement,
                                        IN      int                 msgType)
{
    char message[100];
    int size=100;
    char NonStandardData[100];
    BOOL isText;
    cmNonStandardParam param;

    param.length = (int)sizeof(NonStandardData);
    param.data = (char*)NonStandardData;
    strcpy(param.info.object, "1 1 1 1");
    param.info.objectLength = 7;

    TclExecute("test:Log {AnnexL message received}");

    cmCallGetAnnexLMessage(hApp, annexLElement,&isText,message,&size,&param);
    if (isText)
    {
        if(size < 100) message[size] = 0;
        TclExecute("test:Log { \"%s\"}", message);
    }
    TclExecute("test:Log {NSD: %s}", CMNonStandardParam2String(&param));
    return 0;
}

/********************************************************************************************
 * Multiplex_Update
 * purpose : This procedure is called when the Update button is pressend on the muliplex frame.
 *           It sets the multiplex and maintain parameters of a call.
 * syntax  : Call.Accept <callInfo> <multiplex> <maintain>
 *           <callInfo> - Call information (counter and handle)
 *           <multiplex> - value of the multiplex check box.
 *           <maintain> - value of the maintain check box.
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Multiplex_Update(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    /* set Multiplex and Maintain */
    cmCallSetParam(Call->hsCall, cmParamIsMultiplexed, 0,
        atoi(argv[2]), NULL);
    cmCallSetParam(Call->hsCall, cmParamShutdownEmptyConnection, 0,
        !atoi(argv[3]), NULL);

    /* Give out indication */
    TclExecute("test:Log {Updated: Multiplex = %s Maintain = %s}", argv[2], argv[3]);

    return TCL_OK;
}

/***********************************************************************************
 * Routine Name: cmEvCallMasterSlave
 * Description : Notify the application as to the M/S status of a call.
 * Input : haCall - Application handle to the call.
 *         hsCall - CM handle to the call.
 *         status - M/S state of the remote station.
 *         statusDeterminationNumber - random number.
 * Output: 0
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallMasterSlaveStatus(IN HAPPCALL haCall,
                                         IN HCALL    hsCall,
                                         IN UINT32   status)
{
    CallInfo * call = (CallInfo *) haCall;
    const char * message;
    switch(status)
    {
        case cmMSMaster:
            message = "MSD: terminal is Master";
            break;
        case cmMSSlave:
            message = "MSD: terminal is Slave";
            break;
        case cmMSError:
            message = "MSD: ERROR";
            break;
        default:
            message = "MSD: ???";
    }
    TclExecute("call:Log %d {%s}", call->counter, message);
    return 0;
}

/*******************************************************************************************
 * Routine Name: cmEvCallInfo
 * Description : Fill the incoming call window (if it exists) with display and useruser field.
 * input   : haCall  - Aplication handle for the call
 *           hsCall  - Stack handle for the call
 *           display - Display string
 *           userUser - user user info string
 *           userUserSize - Size of user user info string
 * output  : (-)
 * return  : none
 *******************************************************************************************/
int RVCALLCONV cmEvCallInfo(IN HAPPCALL haCall,
                            IN HCALL    hsCall,
                            IN char*    display,
                            IN char*    userUser,
                            IN int      userUserSize)
{
    char * exists = (char *)"0";
    userUser[userUserSize] = 0;
    TclExecute("winfo exists .incall");
    exists = Tcl_GetStringResult(interp);
    if(exists[0] == '1')
        TclExecute(".incall.info.uui configure -text {%s}\n"
            ".incall.info.disp configure -text {%s}",
            userUser, display);
    return 0;
}

/*******************************************************************************************
 * Routine Name: cmEvCallTunnNewMessage
 * Description :
 * input   : haCall - Aplication handle for the call
 *           hsCall - Stack handle for the call
 *           vtNodeId -
 *           vtAddNodeId -
 * output  : wait -
 * return  : none
 *******************************************************************************************/
int RVCALLCONV cmEvCallTunnNewMessage(IN  HAPPCALL haCall,
                                      IN  HCALL    hsCall,
                                      IN  int      vtNodeId,
                                      IN  int      vtAddNodeId,
                                      OUT BOOL*    wait)
{
    CallInfo * call = (CallInfo *) haCall;
    TclExecute("call:Log %d {*TunnNewMessage*}", call->counter);
    return 0;
}






/********************************************************************************************
 *
 *  Stack callbacks with an empty implmenetation
 *
 ********************************************************************************************/

int RVCALLCONV cmEvCallCapabilities(IN HAPPCALL      haCall,
                                    IN HCALL         hsCall,
                                    IN cmCapStruct * capabilities[])
{ return 0; }

int RVCALLCONV cmEvCallCapabilitiesExt(IN HAPPCALL          haCall,
                                       IN HCALL             hsCall,
                                       IN cmCapStruct * * * capabilities[])
{ return 0; }

int RVCALLCONV cmEvCallCapabilitiesResponse(IN HAPPCALL haCall,
                                            IN HCALL    hsCall,
                                            IN UINT32   status)
{ return 0; }

int RVCALLCONV cmEvCallMasterSlave(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN UINT32   terminalType,
                                   IN UINT32   statusDeterminationNumber)
{ return 0; }

int RVCALLCONV cmEvCallRoundTripDelay(IN HAPPCALL haCall,
                                      IN HCALL    hsCall,
                                      IN INT32    delay)
{ return 0; }

int RVCALLCONV cmEvCallRequestMode(IN HAPPCALL        haCall,
                                   IN HCALL           hsCall,
                                   IN cmReqModeStatus status,
                                   IN INT32           nodeId)
{ return 0; }

int RVCALLCONV cmEvCallMiscStatus(IN HAPPCALL     haCall,
                                  IN HCALL        hsCall,
                                  IN cmMiscStatus status)
{ return 0; }

int RVCALLCONV cmEvCallControlStateChanged(IN HAPPCALL haCall,
                                           IN HCALL    hsCall,
                                           IN UINT32   state,
                                           IN UINT32   stateMode)
{ return 0; }

#ifdef __cplusplus
}
#endif


