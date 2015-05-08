#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                channel.h
 *
 * This file provides functions for handling the channels of a call.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "TAP_init.h"
#include "TAP_general.h"
#include "TAP_defs.h"





/********************************************************************************************
 *
 *  TCL procedures
 *
 ********************************************************************************************/


/********************************************************************************************
 * Channel_OpenOutgoingWindow
 * purpose : Open the "New Channel" window
 *           This function is called from the TCL when the "New Channel" button is pressed in
 *           the main application window.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_OpenOutgoingWindow(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Channel_ConnectOutgoing
 * purpose : Connect a new outgoing channel
 *           This function is called from the TCL when the "Ok" button is pressed in the
 *           "New Channel" window.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ConnectOutgoing(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Channel_DisplayChannelList
 * purpose : Display the list of channels inside the channels window by the selected call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_DisplayChannelList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Channel_ResponseForOlC
 * purpose : send ACK or REJECT after receiving OLC
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ResponseForOLC(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Channel_Answer
 * purpose : answers a logical chennel
 * syntax  : Channel.Drop <channelHandle1> <channelHandle2>
 *           <channelHandle?>   - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Answer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Channel_Drop
 * purpose : drops a logical chennel
 * syntax  : Channel.Drop <channelHandle1> <channelHandle2> <dropReason>
 *           <channelHandle?>   - Channel information (call and channel id)
 *           <dropReason>       - Drop reason for the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Drop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Channel_MediaLoop
 * purpose : Requests media loop on the specified channel. Sends a maintenanceLoopRequest
 *           message to the remote terminal.
 * syntax  : Channel.MediaLoop <channelHandle1> <channelHandle2>
 *           <channelHandle?>   - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_MediaLoop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Channel_Rate
 * purpose : Changes the channel rate
 * syntax  : Channel.Rate <rate> <channelHandle1> <channelHandle2>
 *           <rate>           - rate of the channel
 *           <channelHandle?> - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Rate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * Channel_ResponseForClC
 * purpose : send ACK or REJECT after receiving CLC
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ResponseForCLC(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);





/********************************************************************************************
 *
 *  Stack callbacks
 *
 ********************************************************************************************/


/********************************************************************************************
 * Channel_StateChangeCB
 * purpose : the function that invoked when event ChannelStateChange occurs
 * input   : AppHChannel    - Channel struct of the application
 *         : CmHChannel     - handle for the call
 *         : NewState       - current state of the channel
 * output  : none
 * return  : none
 ********************************************************************************************/
int RVCALLCONV cmEvChannelStateChanged(IN HAPPCHAN haChan,
                                       IN HCHAN hsChan,
                                       IN UINT32 state,
                                       IN UINT32 stateMode);

int RVCALLCONV cmEvChannelParameters(
                        IN      HAPPCHAN            haChan,
                        IN      HCHAN               hsChan,
                        IN      char*               channelName,
                        IN      HAPPCHAN            haChanSameSession,
                        IN      HCHAN               hsChanSameSession,
                        IN      HAPPCHAN            haChanAssociated,
                        IN      HCHAN               hsChanAssociated,
                        IN      UINT32              rate);

/********************************************************************************************
* cmEvCallNewChannel
* Description : the function that is invoked when the event EvCmCallNewChannel occurs, i.e.
*               when a new channel is recieved for a call.
* Input : haCall    - Application handle to the call.
*         hsCall    - CM handle to the call.
*         hsChan    - CM handle to the channel.
* Output: lphaChan  - Pointer to the application handle to the channel.
* Return: none
********************************************************************************************/
int RVCALLCONV cmEvCallNewChannel(IN  HAPPCALL   haCall,
                                  IN  HCALL      hsCall,
                                  IN  HCHAN      hsChan,
                                  OUT LPHAPPCHAN lphaChan);

int RVCALLCONV cmEvChannelRequestCloseStatus(IN      HAPPCHAN              haChan,
                                             IN      HCHAN                 hsChan,
                                             IN      cmRequestCloseStatus  status);

int RVCALLCONV cmEvChannelSetAddress(IN      HAPPCHAN            haChan,
                                     IN      HCHAN               hsChan,
                                     IN      UINT32              ip,
                                     IN      UINT16              port);

int RVCALLCONV cmEvChannelSetRTCPAddress(IN      HAPPCHAN            haChan,
                                         IN      HCHAN               hsChan,
                                         IN      UINT32              ip,
                                         IN      UINT16              port);

int RVCALLCONV cmEvChannelRTPDynamicPayloadType(IN HAPPCHAN haChan,
                                                IN HCHAN    hsChan,
                                                IN INT8     dynamicPayloadType);







/********************************************************************************************
 *
 *  Test Application related
 *
 ********************************************************************************************/


/********************************************************************************************
 * ChannelCreate
 * purpose : Preparing application handle for new channel(fast start or regular)
 * input   : CmHChannel    - CM handle for the channel
 *                           Can be set to NULL
 *           IsIncoming    - Check if it is incoming or outgoing channel (for fast start)
 *           IsScript      - Is this channel controlled by scripts or the appication's GUI
 *           CurrentCall   - Application handle for the call
 * output  : none
 * return  : Channel handle allocated
 *           NULL if failed
********************************************************************************************/
ChannelInfo* ChannelCreate(HCHAN        CmHChannel,
                           BOOL         IsOutgoing,
                           BOOL         IsScript,
                           CallInfo*    CurrentCall);


/********************************************************************************************
 * FreeChannel
 * purpose : Deallocate a channel from the application's database
 * input   : CurrentChannel - Channel to free
 * output  : none
 * return  : result from CM_ChannelClose
 ********************************************************************************************/
int FreeChannel(ChannelInfo*   CurrentChannel);


/********************************************************************************************
 * DisplayChannelList
 * purpose : Display the list of channels inside the channels window by the selected call
 * input   : Call   - Call information holding the channels
 *           Widget - Listbox widget to fill with channel's information
 *                    If set to NULL, the default main channels list is used
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int DisplayChannelList(CallInfo* Call, char* Widget);






/********************************************************************************************
 *
 *  Stack callbacks with an empty implmenetation
 *
 ********************************************************************************************/

int RVCALLCONV cmEvChannelNewRate(IN HAPPCHAN haChan,
                                  IN HCHAN    hsChan,
                                  IN UINT32   rate);

int RVCALLCONV cmEvChannelMaxSkew(IN HAPPCHAN haChan1,
                                  IN HCHAN    hsChan1,
                                  IN HAPPCHAN haChan2,
                                  IN HCHAN    hsChan2,
                                  IN UINT32   skew);

int RVCALLCONV cmEvChannelVideoFastUpdatePicture(IN HAPPCHAN haChan,
                                                 IN HCHAN    hsChan);

int RVCALLCONV cmEvChannelVideoFastUpdateGOB(IN HAPPCHAN haChan,
                                             IN HCHAN    hsChan,
                                             IN int      firstGOB,
                                             IN int      numberOfGOBs);

int RVCALLCONV cmEvChannelVideoFastUpdateMB(IN HAPPCHAN haChan,
                                            IN HCHAN    hsChan,
                                            IN int      firstGOB,
                                            IN int      firstMB,
                                            IN int      numberOfMBs);

int RVCALLCONV cmEvChannelHandle(IN HAPPCHAN      haChan,
                                 IN HCHAN         hsChan,
                                 IN int           dataTypeHandle,
                                 IN cmCapDataType dataType);

int RVCALLCONV cmEvChannelGetRTCPAddress(IN HAPPCHAN haChan,
                                         IN HCHAN    hsChan,
                                         IN UINT32 * ip,
                                         IN UINT16 * port);

int RVCALLCONV cmEvChannelTSTO(IN HAPPCHAN haChan,
                               IN HCHAN    hsChan,
                               IN INT8     isCommand,
                               IN INT8     tradeoffValue);

int RVCALLCONV cmEvChannelMediaLoopStatus(IN HAPPCHAN          haChan,
                                          IN HCHAN             hsChan,
                                          IN cmMediaLoopStatus status);

int RVCALLCONV cmEvChannelReplace(IN HAPPCHAN haChan,
                                  IN HCHAN    hsChan,
                                  IN HAPPCHAN haReplacedChannel,
                                  IN HCHAN    hsReplacedChannel);

int RVCALLCONV cmEvChannelFlowControlToZero(IN HAPPCHAN haChan,
                                            IN HCHAN    hsChan);

int RVCALLCONV cmEvChannelMiscCommand(IN HAPPCHAN               haChan,
                                      IN HCHAN                  hsChan,
                                      IN cmMiscellaneousCommand miscCommand);

int RVCALLCONV cmEvChannelTransportCapInd(IN HAPPCHAN haChan,
                                          IN HCHAN    hsChan,
                                          IN int      nodeId);

int RVCALLCONV cmEvChannelSetNSAPAddress(IN HAPPCHAN haChan,
                                         IN HCHAN    hsChan,
                                         IN char*    address,
                                         IN int      length,
                                         IN BOOL     multicast);

int RVCALLCONV cmEvChannelSetATMVC(IN HAPPCHAN haChan,
                                   IN HCHAN    hsChan,
                                   IN int      portNumber);

int RVCALLCONV cmEvChannelRecvMessage(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 message);

int RVCALLCONV cmEvChannelSendMessage(
        IN      HAPPCHAN            haChan,
        IN      HCHAN               hsChan,
        IN      int                 message);

#endif

#ifdef __cplusplus
}
#endif
