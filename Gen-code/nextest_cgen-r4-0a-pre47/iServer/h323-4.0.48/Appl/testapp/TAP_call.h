#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                call.h
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


#ifndef _CALL_H
#define _CALL_H

#include "TAP_init.h"


/********************************************************************************************
 *
 *  TCL procedures
 *
 ********************************************************************************************/


/********************************************************************************************
 * SetCallVars
 * purpose : Set the parameters of the Call/conference window
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int SetCallVars(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Call_GetCounterValue
 * purpose : Get new number for a call
 * input   : none
 * output  : none
 * return  : Counter value
 ********************************************************************************************/
int Call_GetCounterValue(void);

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
int Call_GetWindowHandle(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_DisplayInfo(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Call_SetOutgoingParams
 * purpose : Set the parameters of an outgoing call before dialing
 * input   : CurrentCall    - Call to set
 * output  : none
 * return  : none
 ********************************************************************************************/
void Call_SetOutgoingParams(CallInfo* CurrentCall);


/********************************************************************************************
 * Call_Drop
 * purpose : Drop the current selected call
 * syntax  : Call.Drop <callInfo> <reason>
 *           <callInfo> - Call information (counter and handle)
 *           <reason>   - Reason string for dropping the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Drop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_SendCallProceeding(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_SendAlerting(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_SendConnect(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_SendStatusInquiry(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SendProgress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * cmEvCallProgress
 * purpose : Notify application that call progress was recieved.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallProgress(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle);

/********************************************************************************************
 * Call_SendUserInformation
 * purpose : Send Notify message
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
int Call_SendUserInformation(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * cmEvCallNotify
 * purpose : Notify application that call notify was recieved.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallUserInfo(IN HAPPCALL haCall,
                                IN HCALL    hsCall,
                                IN int      handle);

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
int Call_SendNotify(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * cmEvCallNotify
 * purpose : Notify application that call notify was recieved.
 * input   : haCall - application handle for the call.
 *           hsCall - stack handle for the call.
 *           handle - message handle.
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RVCALLCONV cmEvCallNotify(IN HAPPCALL haCall,
                              IN HCALL    hsCall,
                              IN int      handle);

/********************************************************************************************
 * Call_CreateH245
 * purpose : Create an H245 session is got ip and port, listen for an H245 connection if not.
 * syntax  : Call.CreateH245 <callInfo> ?<ipAddress>?
 *           <callInfo>  - Call information (counter and handle)
 *           <ipAddress> - Address to connect to. if not present, will listen.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_CreateH245(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_LoopOff(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_ConnectControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SeperateControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_CloseControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SendCaps(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SendEmptyCaps(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SendCapsAck(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_SendMSD(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Call_UII
 * purpose : Request to send a user input indication.
 * syntax  : Call.SendNotify <callInfo> <non-standard data> <user input>
 *           <callInfo> - Call information (counter and handle)
 *           <non-standard data> - string containing the non-standard data params
 *           <user input> - Input from the user.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_UII(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

int RVCALLCONV cmEvCallUserInput(IN HAPPCALL haCall,
                               IN HCALL hsCall,
                               IN INT32 userInputId);

/********************************************************************************************
 * CreateCallObject
 * purpose : Create a complete call object, and sets outgoing params.
 * input   : none
 * output  : none
 * return  : the CallInfo struct of the call, NULL if failed.
 ********************************************************************************************/
CallInfo * CreateCallObject(void);

/********************************************************************************************
 * Call_Dial
 * purpose : Dial a new call to a destination.
 * syntax  : Call.Dial <callInfo> <aliases>
 *           <callInfo> - Call information (counter and handle)
 *           <aliases>  - Destination aliases for the call
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_Dial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Call_FastStartDial
 * purpose : Finishes the Call_Dial procces after the user has selected channels.
 * syntax  : Call.FastStart.Dial <callInfo>
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Call_FastStartDial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* Call_Make
* purpose : Make a quick new call to a destination.
* syntax  : Call.Dial <callInfo> <aliases>
*           <alias> - whom to call
* input   : clientData - used for creating new command in tcl
*         : interp - interpreter for tcl commands
*         : argc - number of parameters entered to the new command
*         : argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully.
********************************************************************************************/
int Call_Make(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_FastStartMake(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_Accept(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_IncompleteAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_AddressComplete(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Call_SendAdditionalAddr
 * purpose : This procedure is called when the "Send" button is pressend on the redial
 *           window. It sends additional address information to the destination of the call.
 * syntax  : Call.SendAdditionalAddr <callInfo>
 *           <callInfo>   - Call information (counter and handle)
 *           <isARJ>      - RV_TRUE if message sent was ARJ
 *                          RV_FALSE if message sent was SETUP ACK
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
int Call_SendAdditionalAddr(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_OpenOutgoingFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_BuildOutgoingFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
int Call_ApproveFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_RefuseFaststart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Call_SendFacility(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 *
 *  NG Stack callbacks
 *
 ********************************************************************************************/


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
                                    IN UINT32    state,
                                    IN UINT32    stateMode);

/*******************************************************************************************
 * ColorCall
 * Description: Color the call by the confID and hCon. will also set the AppHandle of the connection
 * Input : Call - pointer to the callInfo structure of the call.
 * Output: none
 * return: none
 ******************************************************************************************/
void ColorCall(CallInfo * Call);

/*******************************************************************************************
 * ColorCall
 * Description: Maintain the number of current and total calls.
 * Input : raise - true = ++; talse = --
 * Output: none
 * return: none
 ******************************************************************************************/
void UpdateCallNum(BOOL raise);

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
                               IN UINT32   rate);

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
int Call_SetRate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/*******************************************************************************************
 * cmEvNewCall
 * purpose : Notify the application about new incoming call (Setup message was recieved).
 *           Note: after this event, CM_EvCallStateChanged with new state
 *           RV323_CM_CallStateOffering is called. The application can check wheather the setup
 *           message include fast connect element.
 * input   : CmHCall - CM handle to the new call.
 * output  : AppHCall - Pointer for the application handle to the new call.
 * return  : none
 ******************************************************************************************/
int RVCALLCONV cmEvNewCall(IN HAPP hApp,
                           IN HCALL hsCall,
                           OUT LPHAPPCALL lphaCall);


/********************************************************************************************
* cmEvCallIncompleteAddress
* purpose : Notify the application at the outgoing side, when setup Ack / ARJ with
*           reason IncompleteAddress was recieved, which means that address is incomplete
*           and the application should use the api function CM_CallSendAdditionalAddress
*           to send addtional information to the remote.
* Input : haCall - The Application handle of the call.
*         hsCall - The Stack handle of the call.
* Output: (-)
* Return: If an error occurs, the function should return a negative value.
*         If no error occurs, the function should return a non-negative value.
********************************************************************************************/
int RVCALLCONV cmEvCallIncompleteAddress(IN  HAPPCALL    haCall,
                                         IN  HCALL       hsCall);


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
                                         IN BOOL            SendingComplete);


/********************************************************************************************
 * cmEvCallStatus
 * purpose : Notify the application when the a new Status message is recieved.
 *           The status message may be a response to Status Inquiry message that was
 *           sent, or as response to an unknown message that the remote side recieved.
 * input   : haCall - Application handle to the call.
 *           hsCall - CM handle to the call.
 *           callStatusMsg - status message.
 * output  : callStatusMsg - status message.
 * return  : negative on error.
 ********************************************************************************************/
int RVCALLCONV cmEvCallStatus(IN    HAPPCALL haCall,
                              IN    HCALL hsCall,
                              OUT IN    cmCallStatusMessage *callStatusMsg);


/***********************************************************************************
 * Routine Name: cmEvCallFacility
 * Description : Notify the application when the a new facility message is recieved.
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
                                OUT IN  BOOL        *proceed);

/***********************************************************************************
 * Routine Name: cmEvCallFastStart
 * Description : Does nothing
 * Input : haCall  - Application handle to the call.
 *         hsCall   - CM handle to the call.
 * Output: (-)
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallFastStart(IN  HAPPCALL                haCall,
                                 IN  HCALL                   hsCall);

/***********************************************************************************
 * Routine Name: cmEvCallFastStartSetup
 * Description : Notify the application at the incoming side of the call that fast start
 *               proposals were recieved in the current message.
 *               The application can use the api function CM_CallGetFastStartProposal
 *               to retrieve the proposals one by one.
 *
 * Input : haCall  - Application handle to the call.
 *         hsCall   - CM handle to the call.
 *         fsMessage - pointer to the FS message.
 * Output: (-)
 * Return: none
 ***********************************************************************************/
int RVCALLCONV cmEvCallFastStartSetup(IN HAPPCALL haCall,
                                      IN HCALL hsCall,
                                      INOUT cmFastStartMessage *fsMessage);


/****************************************************************
* Call_InitFastStart                                            *
* Purpose: Initializes the structure that stores a list of      *
*          proposed Fast Start channels, RTP and RTCP           *
*          addresses. The application opens Fast Start channels *
* input  : Call :The App handle for the call.                   *
****************************************************************/
int Call_InitFastStart(CallInfo * Call);

int RVCALLCONV cmEvCallNewAnnexMMessage(IN      HAPPCALL            haCall,
                                      IN      HCALL               hsCall,
                                      IN      int                 annexMElement,
                                      IN      int                 msgType);

int RVCALLCONV cmEvCallNewAnnexLMessage(IN      HAPPCALL            haCall,
                                      IN      HCALL               hsCall,
                                      IN      int                 annexLElement,
                                      IN      int                 msgType);

/********************************************************************************************
 * Multiplex_Update
 * purpose : This procedure is called when the Update button is pressend on the muliplex frame.
 *           It sets the multiplex and maintain parameters of a call.
 * syntax  : Call.Accept <callInfo>
 *           <callInfo> - Call information (counter and handle)
 * input   : clientData - used for creating new command in tcl
 *         : interp - interpreter for tcl commands
 *         : argc - number of parameters entered to the new command
 *         : argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Multiplex_Update(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

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
                                         IN UINT32   status);

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
                            IN int      userUserSize);

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
                                      OUT BOOL*    wait);





/********************************************************************************************
 *
 *  Stack callbacks with an empty implmenetation
 *
 ********************************************************************************************/

int RVCALLCONV cmEvCallCapabilities(IN HAPPCALL      haCall,
                                    IN HCALL         hsCall,
                                    IN cmCapStruct * capabilities[]);

int RVCALLCONV cmEvCallCapabilitiesExt(IN HAPPCALL          haCall,
                                       IN HCALL             hsCall,
                                       IN cmCapStruct * * * capabilities[]);

int RVCALLCONV cmEvCallCapabilitiesResponse(IN HAPPCALL haCall,
                                            IN HCALL    hsCall,
                                            IN UINT32   status);

int RVCALLCONV cmEvCallMasterSlave(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN UINT32   terminalType,
                                   IN UINT32   statusDeterminationNumber);

int RVCALLCONV cmEvCallRoundTripDelay(IN HAPPCALL haCall,
                                      IN HCALL    hsCall,
                                      IN INT32    delay);

int RVCALLCONV cmEvCallRequestMode(IN HAPPCALL        haCall,
                                   IN HCALL           hsCall,
                                   IN cmReqModeStatus status,
                                   IN INT32           nodeId);

int RVCALLCONV cmEvCallMiscStatus(IN HAPPCALL     haCall,
                                  IN HCALL        hsCall,
                                  IN cmMiscStatus status);

int RVCALLCONV cmEvCallControlStateChanged(IN HAPPCALL haCall,
                                           IN HCALL    hsCall,
                                           IN UINT32   state,
                                           IN UINT32   stateMode);

#endif

#ifdef __cplusplus
}
#endif
