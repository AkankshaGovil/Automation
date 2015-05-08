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
 *                                TAP_wrapper.h
 *
 *  This file contains all the functions that are used for
 * wrapping the CM api functions so we can use them as tcl commands.
 * This is done for writing scripts in tcl that have specific scenarios.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/


#ifndef _WRAPPER_H
#define _WRAPPER_H

#include <tcl.h>
#include <tk.h>


/********************************************************************************************
 * WrapperFunc
 * purpose : wrapping stack functions - this function is called whenever we wrap a
 *           function for the scripts' use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int WrapperFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * CallbackFunc
 * purpose : wrapping callback functions - this function is called whenever we wrap a
 *           callback function for the scripts' use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int CallbackFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);




/********************************************************************************************
*
* Generic api
*
*********************************************************************************************/


/********************************************************************************************
 * api_cm_GetVersion
 * purpose : wrapping the original CM api function - GetVersion
 * syntax  : api:cm:GetVersion
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_GetVersion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_Stop
 * purpose : wrapping the original CM api function - cmStop
 * syntax  : api:cm:Stop
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_Stop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_Start
 * purpose : wrapping the original CM api function - cmStart
 * syntax  : api:cm:Start
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_Start(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_LogMessage
 * purpose : wrapping the original CM api function - cmLogMessage
 * syntax  : api:cm:LogMessage <msg>
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_LogMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_Encode
 * purpose : wrapping the original CM api function - cmEmEncode
 * syntax  : api:cm:Encode <hVal> <nodeId>
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_Encode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_Decode
 * purpose : wrapping the original CM api function - cmEmDecode
 * syntax  : api:cm:Decode <hVal> <nodeId> <buffer>
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_Decode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
*
* Call api
*
*********************************************************************************************/



/********************************************************************************************
 * api_cm_CM_CallNew
 * purpose : wrapping the original CM api function - CallNew
 * syntax  : api:cm:CallNew
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallNew(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_CallDial
 * purpose : wrapping the original CM api function - CallDial
 * syntax  : api:cm:CallDial <handle>
 *           <handle>         - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallDial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_CallProceeding
 * purpose : wrapping the original CM api function - CallProceeding
 * syntax  : api:cm:CallProceeding <handle>
 *           <handle>         - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallProceeding(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_CallAccept
 * purpose : wrapping the original CM api function - CallAccept
 * syntax  : api:cm:CallAccept <handle>
 *           <handle>         - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallAccept(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
* api_cm_CallAnswer
* purpose : wrapping the original CM api function - CallAnswer
* syntax  : api:cm:CallAnswer <handle>
*           <handle>         - handle of the call
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_CallAnswer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
* api_cm_CallDrop
* purpose : wrapping the original CM api function - CallDrop
* syntax  : api:cm:CallDrop <handle>
*           <handle>         - handle of the call
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_CallDrop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 * api_cm_CallSetParam
 * purpose : wrapping the original CM api function - CallSetParam
 * syntax  : api:cm:CallAnswer <handle> <RV323_CM_CallParam> <str>
 *         : <handle>         - handle of the call
 *         : <RV323_CM_CallParam>    - determine which call param
 *         : <RV_BOOL>            - for boolean parameters
 *         : <str>                - display or UserUser or TransportAddress
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallSetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 * api_cm_CallClose
 * purpose : wrapping the original CM api function - CallClose
 * syntax  : api:cm:CallClose <handle>
 *           <handle>         - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallClose(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/***********************************************************************************
 * api_cm_CallForward
 * purpose: wrapping the original CM api function - CallFacilityForward
 * syntax : api:cm:CallForward <handle> <AlternativeAddressString> <AlternativeAliases> <FacilityString>
 *        : <handle>                   - handle of the call
 *        : <AlternativeAddressString> - string of the alternative adress (transport address)
 *        : <AlternativeAliases>       - string of the alternative alias adresses (array of alias address).
 *        : <FacilityString>           - Facility string.
 * input  : clientData - used for creating new command in tcl
 *          interp - interpreter for tcl commands
 *          argc - number of parameters entered to the new command
 *          argv - the parameters entered to the tcl command
 * output : none
 * return : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ***********************************************************************************/
int api_cm_CallForward(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/***********************************************************************************
 * api_cm_CallStatusInquiry
 * purpose: wrapping the original CM api function - CallStatusInquiry
 * syntax : api:cm:CallStatusInquiry <CallHandle> <DisplayInfo>
 *        : <CallHandle>     - handle of the call
 *        : <DisplayInfo>    - Display Info string
 * input  : clientData - used for creating new command in tcl
 *          interp - interpreter for tcl commands
 *          argc - number of parameters entered to the new command
 *          argv - the parameters entered to the tcl command
 * output : none
 * return : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ***********************************************************************************/
int api_cm_CallStatusInquiry(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_CallConnectControl
 * purpose : wrapping the original CM api function - CallConnectControl
 * syntax  : api_cm_CallConnectControl <handle>
 *           <handle>                  - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallConnectControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);






/********************************************************************************************
*
* Channel api
*
*********************************************************************************************/




/********************************************************************************************
 * api_cm_ChannelNew
 * purpose : wrapping the original CM api function - ChannelCreate
 * syntax  : api_cm_ChannelNew <CallHandle> <Direction>
 *           <CallHandle>         - handle of the call
 *           <Direction>          - Direction of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelNew(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_ChannelOpen
 * purpose : wrapping the original CM api function - ChannelOpen
 * syntax  : api_cm_ChannelOpen <CallHandle> <ChannelHandle> <RTP> <RTCP> <MediaType>
 *           <CallHandle>     - application handle of the Call
 *           <ChannelHandle>  - cm handle of the channel
 *           <RTP> - RTP address
 *           <RTCP> - RTCP address
 *           <MediaType> - Media type of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelOpen(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelAnswer
 * purpose : wrapping the original CM api function - ChannelAnswer
 * syntax  : api_cm_ChannelAnswer <ChannelHandle> <Response>
 *           <ChannelHandle>     - cm handle of the channel
 *           <Response>          - ack or reject
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelAnswer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_ChannelDrop
 * purpose : wrapping the original CM api function - ChannelDrop
 * syntax  : api_cm_ChannelDrop <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelDrop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_ChannelRequestCloseReject
 * purpose : wrapping the original CM api function - cmChannelRequestCloseReject
 * syntax  : api:cm:ChannelRequestCloseReject <ChannelHandle>
 *           <ChannelHandle> - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelRequestCloseReject(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


 /********************************************************************************************
 * api_cm_ChannelClose
 * purpose : wrapping the original CM api function - ChannelClose
 * syntax  : api_cm_ChannelClose <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelClose(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_ChannelGetCallHandle
 * purpose : wrapping the original CM api function - cmChannelGetCallHandles
 * syntax  : api:cm:ChannelGetCallHandle <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetCallHandle(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelGetDependency
 * purpose : wrapping the original CM api function - cmChannelGetDependency
 * syntax  : api:cm:ChannelGetDependency <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetDependency(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelGetDuplexAddress
 * purpose : wrapping the original CM api function - cmChannelGetDuplexAddress
 * syntax  : api:cm:ChannelGetDuplexAddress <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetDuplexAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelGetNumber
 * purpose : wrapping the original CM api function - cmChannelGetNumber
 * syntax  : api:cm:ChannelGetNumber <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetNumber(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelGetOrigin
 * purpose : wrapping the original CM api function - cmChannelGetOrigin
 * syntax  : api:cm:ChannelGetOrigin <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetOrigin(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelGetSource
 * purpose : wrapping the original CM api function - cmChannelGetSource
 * syntax  : api:cm:ChannelGetSource <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelGetSource(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_ChannelIsDuplex
 * purpose : wrapping the original CM api function - cmChannelIsDuplex
 * syntax  : api:cm:ChannelIsDuplex <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelIsDuplex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_CallGetParam
 * purpose : wrapping the original CM api function - cmCallGetParam
 * syntax  : api:cm:CallGetParam <handle> <callParam> <index>
 *         : <handle>       - handle of the call
 *         : <callParam>    - determine which call param
 *         : <index>        - index of the parameter
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallGetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
*
* H245 api
*
*********************************************************************************************/


/********************************************************************************************
 * api_cm_H245BuildMsg
 * purpose : wrapping the original CM api function - BuildeH245Msg
 * syntax  : api:cm:H245BuildMsg <callHandle> <channelHandle> <forwardRev> <h245MessageType>
 *           <callHandle>       - cm handle of the call
 *           <channelHandle>    - cm handle of the channel
 *           <forwardRev>       - channel type (uni,bi,forward,reverse)
 *           <h245MessageType>  - message type to build
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245BuildMsg(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_cm_H245SetField
 * purpose : wrapping the original CM api function - H245SetField
 * syntax  : api_cm_H245SetField <CallHandle> <ChannelHandle> <hMessage> <MsgType>
 *           <Field> <Value> <ValueSize>
 *           <CallHandle> - cm handle of the call
 *           <ChannelHandle>     - cm handle of the channel
 *           <hMessage> - ASN message
 *           <MsgType> - H245 message type
 *           <Field> - H245 field type
 *           <Value> - value to be set in the field
 *           <ValueSize> - size of the value that we are going to set
 *         : <GenParam>          - parameter type
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
 int api_cm_H245SetField(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);





/********************************************************************************************
 * api_cm_H245AddElementToMsg
 * purpose : wrapping the original CM api function - H245AddElementToMsg
 * syntax  : api:cm:H245AddElementToField <CmHCall> <CmHChannel> <ForwardReverse> <MsgType>
 *                                         <hMessage> <hField>  <elemType> <element> <elementSize>
 *           <CmHCall>      - CM handle of the call.
 *           <CmHChannel>   - CM handle of the channel.
 *           <ForwardReverse>- Indicate whether the message is related to the forward or
 *                              reverse channel.
 *           <MsgType>      - The type of message.
 *           <hMessage>     - The message handle.
 *           <hField>       - Handle to the field's element.
 *           <elemType>     - The element type .
 *           <element>      - The element itself.
 *           <elementSize>  - The element size.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : None.
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245AddElementToMsg(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 * api_cm_H245GetField
 * purpose : wrapping the original CM api function - H245GetField
 * syntax  : api:cm:H245GetField <callHandle> <channelHandle> <forwardRev> <hMessage>
 *                               <h245MessageType> <h245FieldType>
 *           <callHandle>       - cm handle of the call
 *           <channelHandle>    - cm handle of the channel
 *           <forwardRev>       - Channel type (uni,bi,forward,reverse)
 *           <hMessage>         - ASN message to use
 *           <h245MessageType>  - H245 message type
 *           <field>            - H245 field to set
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245GetField(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 * api_cm_H245GetElementFromField
 * purpose : wrapping the original CM api function - H245GetElementFromField
 * syntax  : api:cm:H245GetElementFromField <CmHCall> <CmHChannel> <ForwardReverse> <MsgType>
 *                                         <hMessage> <hField>  <elemType> <index> <elementSize>
 *           <CmHCall>      - CM handle of the call.
 *           <CmHChannel>   - CM handle of the channel.
 *           <ForwardReverse>- Indicate whether the message is related to the forward or
 *                              reverse channel.
 *           <MsgType>      - The type of message.
 *           <hMessage>     - The message handle.
 *           <hField>       - Handle to the field's element.
 *           <elemType>     - The element type .
 *           <index>        - The index of the element (in case the field is sequence).
 *           <elementSize>  - The element size.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : None.
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245GetElementFromField(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_H245SendMsg
 * purpose : wrapping the original CM api function - H245SendMsg
 * syntax  : api:cm:H245SendMsg <callHandle> <channelHandle> <forwardRev>
 *                              <h245MessageType> <hMessage>
 *           <callHandle>       - cm handle of the call
 *           <channelHandle>    - cm handle of the channel
 *           <forwardRev>       - Channel type (uni,bi,forward,reverse)
 *           <h245MessageType>  - H245 message type
 *           <hMessage>         - ASN message to send
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new comand
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245SendMsg(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_H245SetCapabilityAllFields
 * purpose : wrapping the original CM api function - H245SetCapabilityAllFields
 * syntax  : api:cm:H245SetCapabilityAllFields <callHandle> <capMsg> <capParam>
 *           <callHandle>   - cm handle of the call
 *           <capMsg>       - ASN message handle
 *           <capParam>     - Capability parameter to set
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245SetCapabilityAllFields(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_H245DeleteCapabilityMessage
 * purpose : wrapping the original CM api function - H245DeleteCapabilityMessage
 * syntax  : api:cm:H245DeleteCapabilityMessage <callHandle> <capType>
 *           <callHandle>   - cm handle of the call
 *           <capType>      - 1:remote, 2:local
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245DeleteCapabilityMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);





/********************************************************************************************
 * api_cm_CheckCapabilityMsg
 * purpose : wrapping the original CM api function - CheckCapabilityMsg
 * syntax  : api:cm:CheckCapabilityMsg  <callHandle> <capMsg>
 *           <callHandle>   - cm handle of the call
 *           <capMsg>       - msg handle (a capability message)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CheckCapabilityMsg(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);



/********************************************************************************************
 * api_cm_SetSameSessionID
 * purpose : wrapping the original CM api function - SetSameSessionID
 * syntax  : api:cm:SetSameSessionID  <callHandle> <hMsg> <updateMsg> <srcChannel> <dstChannel>
 *           <callHandle>   - cm handle of the call
 *           <hMsg>         - handle  of the OLC msg.
 *           <updateMsg>    - A flag indicating if user wants to set the sessionID and RTCP address in the
 *                              OpenLogicalChannel msg, or not.
 *           <srcChannel>   - CM handle of the channel with the sessionID to copy from.
 *           <dstChannel>   - CM handle of the new channel opened, which we want to put the sessionID in.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_SetSameSessionID(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);




/********************************************************************************************
 * api_cm_H245SetChannelInformation
 * purpose : wrapping the original CM api function - H245SetChannelInformation
 * syntax  : api:cm:H245SetChannelInformation  <callHandle> <channelHandle> <channelType>
 *                  <channelInfoType> <information> <sizeofInfo>
 *           <callHandle>     - cm handle of the call
 *           <channelHandle>  - handle  of the OLC msg.
 *           <channelType>    - the channel type :Indicate whether the message is related
 *                              to the forward or reverse channel in case channel is BiDirectional.
 *                              If channel is Unidirectional this parameter is ignored.
 *           <channelInfoType>- the type of information the user set in the h245 database.
 *           <information >   - the details of the information.
 *           <sizeofInfo>     - the size of the information.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_H245SetChannelInformation(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);




/********************************************************************************************
*
* Application api
*
*********************************************************************************************/


/********************************************************************************************
 * api_app_SetCallMode
 * purpose : wrapping an application script function - SetCallMode
 * syntax  : api:app:SetCallMode <callhandle> <mode>
 *           <callhandle>   - handle of the call.
 *           <mode>         - Script or Normal
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_app_SetCallMode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_app_SetChannelMode
 * purpose : wrapping an application script function - SetChannelMode
 * syntax  : api:app:SetChannelMode <channelhandle> <mode>
 *           <channelhandle>    - handle of the channel.
 *           <mode>             - Script or Normal
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_app_SetChannelMode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_app_GetDataTypes
 * purpose : wrapping an application script function - GetDataTypes
 * syntax  : api:app:GetDataTypes
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_app_GetDataTypes(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_app_LoadMessage
 * purpose : wrapping an application script function - LoadMessage
 * syntax  : api:app:LoadMessage <hAsn> <filename>
 *           <hAsn>         - ASN pool handle
 *           <filename>     - Filename of message
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_app_LoadMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * api_app_ChannelKill
 * purpose : wrapping an application script function - ChannelKill
 * syntax  : api:app:ChannelKill <channelHandle>
 *           <channelHandle>    - Channel handle to kill
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_app_ChannelKill(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_app_GetChannelList
 * purpose : Get the list of channels of certain call
 * syntax  : api:app:GetChannelList <Call>
 *           <Call> - the struct that holds the call parameters
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int api_app_GetChannelList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_app_Vt2Address
 * purpose : Convert a node ID of a TransportAddress to a string
 * syntax  : api:app:Vt2Address <valTree> <nodeId>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - TransportAddress type node id
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Address string
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_app_Vt2Address(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_app_Address2Vt
 * purpose : Convert a TransportAddress to a node id
 * syntax  : api:app:Address2Vt <valTree> <address> <nodeId>
 *           <valTree>  - Value tree to use
 *           <address>  - Address string to convert
 *           <nodeId>   - TransportAddress type node id
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Address string
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_app_Address2Vt(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);





/************************************************************************
 * api_cm_GetLocalRASAddress
 * purpose : wrapping the original ASN api function - cmGetLocalRASAddress
 * syntax  : api:cm:GetLocalRASAddress
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetLocalRASAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cm_GetLocalCallSignalAddress
 * purpose : wrapping the original ASN api function - cmGetLocalCallSignalAddress
 * syntax  : api:cm:GetLocalCallSignalAddress
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetLocalCallSignalAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cm_GetLocalAnnexEAddress
 * purpose : wrapping the original ASN api function - cmGetLocalAnnexEAddress
 * syntax  : api:cm:GetLocalAnnexEAddress
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetLocalAnnexEAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);





/********************************************************************************************
*
* Automatic RAS api
*
*********************************************************************************************/


/************************************************************************
 * api_cm_Register
 * purpose : wrapping the original ASN api function - cmRegister
 * syntax  : api:cm:Register
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_Register(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cm_Unregister
 * purpose : wrapping the original ASN api function - cmUnregister
 * syntax  : api:cm:Unregister
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_Unregister(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);








/********************************************************************************************
*
* CMRAS api
*
*********************************************************************************************/


/************************************************************************
 * api_cmras_StartTransaction
 * purpose : wrapping the original ASN api function - cmRASStartTransaction
 * syntax  : api:cmras:StartTransaction <trasnaction> <destAddr> [<hsCall>]
 *           <transaction>  - Transaction type
 *           <destAddr>     - Destination address
 *           <hsCall>       - Call handle of transaction
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The ras transcation handle
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_StartTransaction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_GetParam
 * purpose : wrapping the original ASN api function - cmRASGetParam
 * syntax  : api:cmras:GetParam <hsRas> <trStage> <param> <index>
 *           <hsRas>    - Stack's transaction handle
 *           <trStage>  - Stage to get from
 *           <param>    - Parameter to get
 *           <index>    - Index to use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The parameter's value
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_GetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_SetParam
 * purpose : wrapping the original ASN api function - cmRASSetParam
 * syntax  : api:cmras:SetParam <hsRas> <trStage> <param> <index> <value>
 *           <hsRas>    - Stack's transaction handle
 *           <trStage>  - Stage to get from
 *           <param>    - Parameter to set
 *           <index>    - Index to use
 *           <value>    - Value to set
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_SetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_GetNumOfParams
 * purpose : wrapping the original ASN api function - cmRASGetParam
 * syntax  : api:cmras:GetNumOfParams <hsRas> <trStage> <param>
 *           <hsRas>    - Stack's transaction handle
 *           <trStage>  - Stage to get from
 *           <param>    - Parameter to get
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : The number of parameters
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_GetNumOfParams(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_Request
 * purpose : wrapping the original ASN api function - cmRASRequest
 * syntax  : api:cmras:Request <hsRas>
 *           <hsRas>    - Stack's transaction handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_Request(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_DummyRequest
 * purpose : wrapping the original ASN api function - cmRASDummyRequest
 * syntax  : api:cmras:DummyRequest <hsRas>
 *           <hsRas>    - Stack's transaction handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_DummyRequest(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_Confirm
 * purpose : wrapping the original ASN api function - cmRASConfirm
 * syntax  : api:cmras:Confirm <hsRas>
 *           <hsRas>    - Stack's transaction handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_Confirm(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_Reject
 * purpose : wrapping the original ASN api function - cmRASReject
 * syntax  : api:cmras:Reject <hsRas> <reason>
 *           <hsRas>    - Stack's transaction handle
 *           <reason>   - Reject reason
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_Reject(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_InProgress
 * purpose : wrapping the original ASN api function - cmRASInProgress
 * syntax  : api:cmras:InProgress <hsRas> <delay>
 *           <hsRas>    - Stack's transaction handle
 *           <delay>    - Delay in milliseconds
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_InProgress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_Close
 * purpose : wrapping the original ASN api function - cmRASClose
 * syntax  : api:cmras:Close <hsRas>
 *           <hsRas>    - Stack's transaction handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_Close(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/************************************************************************
 * api_cmras_GetTransaction
 * purpose : wrapping the original ASN api function - cmRASGetTransaction
 * syntax  : api:cmras:GetTransaction <hsRas>
 *           <hsRas>    - Stack's transaction handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Transaction's type
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_GetTransaction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);






/********************************************************************************************
*
* General CM api
*
*********************************************************************************************/


/************************************************************************
 * api_cm_Vt2Alias
 * purpose : wrapping the original ASN api function - cmVt2Alias
 * syntax  : api:cm:Vt2Alias <valTree> <nodeId>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - AliasAddress type node id
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Alias string
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_Vt2Alias(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_cm_Alias2Vt
 * purpose : wrapping the original ASN api function - cmAlias2Vt
 * syntax  : api:cm:Alias2Vt <valTree> <alias> <nodeId>
 *           <valTree>  - Value tree to use
 *           <alias>    - Alias string to convert
 *           <nodeId>   - AliasAddress node Id under which the alias is built
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Alias node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_Alias2Vt(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_cm_GetProperty
 * purpose : wrapping the original ASN api function - cmGetProperty
 * syntax  : api:cm:GetProperty <handle> [CALL]
 *           <handle>   - Ras or Call handle whose property we want
 *           CALL       - Indication that the handle is a call handle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Alias node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetProperty(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_cm_GetValTree
 * purpose : wrapping the original ASN api function - cmGetValTree
 * syntax  : api:cm:GetValTree
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Value tree
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetValTree(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_cm_GetRASConfigurationHandle
 * purpose : wrapping the original ASN api function - cmGetRASConfigurationHandle
 * syntax  : api:cm:GetRASConfigurationHandle
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : RAS configuration node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetRASConfigurationHandle(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);






/********************************************************************************************
*
* General CM api
*
*********************************************************************************************/


/************************************************************************
 * api_pvt_AddRoot
 * purpose : wrapping the original ASN api function - pvtAddRoot
 * syntax  : api:pvt:AddRoot <valTree> <rootName> <value>
 *           <valTree>  - Value tree to use
 *           <rootName> - Name of root
 *           <value>    - Value of root node
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Root node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_AddRoot(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_pvt_GetByPath
 * purpose : wrapping the original ASN api function - pvtGetByPath
 * syntax  : api:pvt:GetByPath <valTree> <nodeId> <path>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - Node id to start from
 *           <path>     - Path to traverse
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_GetByPath(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_pvt_BuildByPath
 * purpose : wrapping the original ASN api function - pvtBuildByPath
 * syntax  : api:pvt:BuildByPath <valTree> <nodeId> <path> <value>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - Node id to start from
 *           <path>     - Path to traverse
 *           <value>    - Value of node
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_BuildByPath(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/************************************************************************
 * api_pvt_Delete
 * purpose : wrapping the original ASN api function - pvtDelete
 * syntax  : api:pvt:Delete <valTree> <nodeId>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - Node id to start from
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_Delete(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);

/************************************************************************
 * api_pvt_GetString
 * purpose : wrapping the original ASN api function - pvtGetString
 * syntax  : api:pvt:GetString <valTree> <nodeId>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - Node id to start from
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : String
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_GetString(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);

/************************************************************************
 * api_pvt_Print
 * purpose : wrapping the original ASN api function - pvtPrint
 * syntax  : api:pvt:Print <valTree> <nodeId> <callback>
 *           <valTree>  - Value tree to use
 *           <nodeId>   - Node id to start from
 *           <callback> - Callback function in TCL to use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_pvt_Print(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);







/********************************************************************************************
*
* Faststart API
*
*********************************************************************************************/

/********************************************************************************************
 * api_cm_FastStartBuild
 * purpose : building a single FS channel.
 * syntax  : api:cm:FastStartBuild <handle> <chanNum> <chanDir> <chanName> <chanType>
 *           <handle>   - handle of the call
 *           <chanNum>  - number of the channel
 *           <chanDir>  - direction of the channel (transmit or recieve)
 *           <chanName> - name of the channel
 *           <chanType> - type of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : node ID of the channle if ok, negative o.w.
 ********************************************************************************************/
int api_cm_FastStartBuild(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_CallAddFastStartMessage
 * purpose : adding an FS channel to the setup message.
 * syntax  : api:cm:FastStartBuild <handle> <nodeID>
 *           <handle>   - handle of the call
 *           <nodeID>   - node ID of the built channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int api_cm_CallAddFastStartMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetChanNum
* purpose :
* syntax  : api:app:FSGetChanNum <fsMessage>
*           <fsMessage> - pointer to the FS message
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetChanNum(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetAltChanNum
* purpose :
* syntax  : api:app:FSGetAltChanNum <fsMessage> <chanNum> <chanDir>
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetAltChanNum(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetChanIndex
* purpose :
* syntax  : api:app:FSGetChanIndex <fsMessage> <chanNum> <altChan> <chanDir>
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
*           <altChan> - alternateChannel number
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetChanIndex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetChanName
* purpose :
* syntax  : api:app:FSGetChanName <fsMessage> <chanNum> <chanDir> <altChan>
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
*           <altChan> - alternateChannel number
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetChanName(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetChanRTCP
* purpose :
* syntax  : api:app:FSGetChanRTCP <fsMessage> <chanNum> <chanDir> <altChan>
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
*           <altChan> - alternateChannel number
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetChanRTCP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_app_FSGetChanRTP
* purpose :
* syntax  : api:app:FSGetChanRTP <fsMessage> <chanNum> <chanDir> <altChan>
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
*           <altChan> - alternateChannel number
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_app_FSGetChanRTP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_FastStartChannelsAck
* purpose : wrapping cmFastStartChannelsAck.
*           acknowledging a faststart channel from the fsMessage struct.
* syntax  : api:cm:FastStartChannelsAck <handle> <fsMessage> <chanNum> <chanDir> <altChan>
*           <handle> - handle of the call
*           <fsMessage> - pointer to the FS message
*           <chanNum> - partnerChannel number
*           <chanDir> - partnerChannel direction
*           <altChan> - alternateChannel number
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_FastStartChannelsAck(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_FastStartChannelsAckIndex
* purpose : wrapping cmFastStartChannelsAckIndex.
*           acknowledging a faststart channel by index.
* syntax  : api:cm:FastStartChannelsAckIndex <handle> <index> <rtcp> <rtp>
*           <handle> - handle of the call
*           <index> - index of the channel to be acknowledged
*           <rtcp> - RTCP address for the channel
*           <rtp> - RTP address for the channel
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_FastStartChannelsAckIndex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_FastStartChannelsReady
* purpose : wrapping cmFastStartChannelsReady.
*           notifys the cm that no more FS channels wil be acked.
* syntax  : api:cm:FastStartChannelsReady <handle>
*           <handle> - handle of the call
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_FastStartChannelsReady(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_CallSendSupplementaryService
* purpose : wrapping cmCallSendSupplementaryService.
*           Sends H450 message on a call.
* syntax  : api:cm:CallSendSupplementaryService <handle> <buffer> <force>
*           <handle> - handle of the call
*           <buffer> - buffer to send
*           <force>  - force message sending or not
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_CallSendSupplementaryService(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * api_cm_CallSimulateMessage
 * purpose : wrapping cmCallSimulateMessage.
 *           Simulates a received message on a call.
 * syntax  : api:cm:CallSimulateMessage <handle> <nodeId>
 *           <handle> - handle of the call
 *           <nodeId> - Node ID to "received"
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallSimulateMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_CallCreateAnnexMMessage
* purpose : Create an annex M message
* syntax  : Call.SendAnnexMMessage <requiered> <objectID> <altIDType> <altIDVariant> <subID> <nsdString> <messages>...
*           <callInfo> - Call information (counter and handle)
*           <requiered> - message needs to be understood by host
*           <force> - send it now or wait for message to be sent
*           <objectID> <altIDType> <altIDVariant> <subID> - components of the cmTunnelledProtocolID struct
*           <nsdString> - formatted string containing the non standard data
*           <messages> - the messages to send, each message in an arg.
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully.
********************************************************************************************/
int api_cm_CallCreateAnnexMMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_CallSendAnnexMMessage
* purpose : Send an annex M message
* syntax  : Call.SendAnnexMMessage <callInfo> <force> <AnnexMmessage>
*           <callInfo> - Call information (counter and handle)
*           <force> - send it now or wait for message to be sent
*           <AnnexMmessage> - value returned by CreateAnnexMMessage
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully.
********************************************************************************************/
int api_cm_CallSendAnnexMMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_CallCreateAnnexLMessage
* purpose : Create an annex L message
* syntax  : Call.SendAnnexMMessage <isText> <nsdString> <message>
*           <isText> - message is text
*           <nsdString> - formatted string containing the non standard data
*           <message> - the message text to send.
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully.
********************************************************************************************/
int api_cm_CallCreateAnnexLMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
* api_cm_CallSendAnnexLMessage
* purpose : Send an annex L message
* syntax  : Call.SendAnnexMMessage <callInfo> <force> <AnnexLmessage>
*           <callInfo> - Call information (counter and handle)
*           <force> - send it now or wait for message to be sent
*           <AnnexLmessage> - the message created by CreateAnnexLMessage.
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully.
********************************************************************************************/
int api_cm_CallSendAnnexLMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


#endif

#ifdef __cplusplus
}
#endif

