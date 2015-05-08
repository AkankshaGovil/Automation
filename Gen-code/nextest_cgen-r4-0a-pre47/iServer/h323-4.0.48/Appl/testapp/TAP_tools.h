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
 *                                TAP_tools.h
 *
 * Application tools handling
 *
 * 1. Log
 *      The user interface for log operations.
 *      - Setting which filters are displayed in the log
 *      - Looking which filters are displayed in the log
 *      - Logging messages from the stack using a specific PRINT_API declared and implemented
 *        in this file.
 * 2. Status
 *      Status information about the stack (All the resources information)
 * 3. Options
 *      Default option values of the stack that can be configured while running.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Tsahi Levent-Levi                   21-May-2000
 *
 ********************************************************************************************/



#ifndef _TAP_tools_H
#define _TAP_tools_H


/* Log Hooks
 ********************************/

BOOL RVCALLCONV hookSendTo(IN      HPROTCONN           hConn,
                           IN      int                 nodeId,
                           IN      int                 nodeIdTo,
                           IN      BOOL                error);

BOOL RVCALLCONV hookRecvFrom(IN      HPROTCONN           hConn,
                             IN      int                 nodeId,
                             IN      int                 nodeIdFrom,
                             IN      BOOL                multicast,
                             IN      BOOL                error);

int RVCALLCONV cmEvCallRecvMessage(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN int      message);

int RVCALLCONV cmEvCallSendMessage(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN int      message);


/********************************************************************************************
 * Log_Init
 * purpose : Initialize the PRINT_API to use the test application's printing facility
 *           and not the one supplied with the stack
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void Log_Init(void);



/********************************************************************************************
 * Log_SetFilter
 * purpose : Set the log filters for modules
 *           This function is activated from the TCL scripts whenever a chechbutton of a log
 *           filter is clicked.
 * syntax  : Log.SetFilter <levelChar> <on/off> <filter>
 *           <levelChar>    - Character of message type (Warn/Info/Detailed/Bug/Fatal)
 *           <on/off>       - "1" if we should log it, "0" if we shouldn't
 *           <filter>       - String of the filter to handle (all/ASN/RPOOL/NETMGR...)
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Log_SetFilter(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Log_FetchFilter
 * purpose : Get the log filters for modules and set their checkbuttons in the log window
 *           of the TCL.
 * syntax  : Log.FetchFilter <filter>
 *           <filter>       - String of the filter to handle (all/ASN/RPOOL/NETMGR...)
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Log_FetchFilter(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Options_GetLocalIP
 * purpose : Get the Local IP address of the machine
 * syntax  : Options.GetLocalIP
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : Local IP string
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Options_GetLocalIP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Status_Display
 * purpose : This function get the resources of Timers, ASN, RAS etc and displays them inside
 *           the status window
 * syntax  : Status.Display
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Status_Display(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


#endif



#ifdef __cplusplus
}
#endif
