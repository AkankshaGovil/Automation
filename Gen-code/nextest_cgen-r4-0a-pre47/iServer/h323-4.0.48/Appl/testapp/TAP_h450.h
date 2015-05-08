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
*                                TAP_h450.h
*
* This file contains all the functions which enable the use of h450 functions. The functions
* are activated either in the H450 tab in the TestApp, of in the incoming call window (only
* some may be thus activated).
*
*
*      Written by                        Version & Date                        Change
*     ------------                       ---------------                      --------
*       Ran Arad                          18-Dec-2000
*
********************************************************************************************/


#ifndef _TAP_H450_H
#define _TAP_H450_H

#include "TAP_general.h"
#include "TAP_defs.h"
#include "TAP_call.h"

#ifdef USE_H450

#include "sse.h"
#include "sssize.h"
#include "h450.h"

#endif


#ifndef USE_H450
/* Define some of the functions in this interface as macros - this will
   not compile them at all */

#define H450_sendNameID(Call, Operation);

#endif


/* defined on call.c */
extern char* CallInfoStr(CallInfo* CurrentCall, char* StateStr);


/********************************************************************************************
 * H450_Init
 * purpose : This function initializes the H450 sepplementay services. does nothing if
 *           USE_H450 is not defined.
 * input   : name - name of the configuration file
 *           lphApp - pointer to the application handle
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int H450_init(char * name, LPHAPP lphApp);

/********************************************************************************************
 * H450_end
 * purpose : This function terminates the H450 sepplementay services. does nothing if
 *           USE_H450 is not defined.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void H450_end(void);

/********************************************************************************************
 * H450_IsInitialized
 * purpose : This procedure checks to see if H450 can be used.
 * input   : none
 * output  : none
 * return  : TRUE if H450 can be used
 *           FALSE otherwise
 ********************************************************************************************/
BOOL H450_IsInitialized(void);

/********************************************************************************************
 * H450_status
 * purpose : This function displays the current status of H450 module. It is used by the
 *           status window of the test application. does nothing if USE_H450 is not defined.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void H450_status(void);

/********************************************************************************************
 * cmEvCallH450SupplServ
 * purpose : callback for received H450 messages, Notifies the H450 state machine
 *           that an H450 message has been recieved does nothing if USE_H450 is not defined.
 * input   : haCall - aplication handle to the call
 *           hsCall - stack handle of the call
 *           msgType - type of the recieved Q931 message
 *           nodeId - root node of the message
 *           size - size of the message
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int RVCALLCONV cmEvCallH450SupplServ(IN HAPPCALL haCall,
                                     IN HCALL hsCall,
                                     cmCallQ931MsgType msgType,
                                     int nodeId,
                                     int size);

/********************************************************************************************
 * H450_CallStateChanged
 * purpose : notifys the H450 state machines when a call changes state, does nothing if
 *           USE_H450 is not defined.
 * input   : Call - pointer to information about the call
 *           state - current state of the call
 *           stateMode - type of the state
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int H450_CallStateChanged(IN CallInfo* Call,
                          IN UINT32 state,
                          IN UINT32 stateMode);

/********************************************************************************************
 * H450_CreateCall
 * purpose : Creates the H450 call object for the call and implements services. does nothing
 *           if USE_H450 is not defined.
 * input   : Call - pointer to information about the call
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int H450_CreateCall(CallInfo * Call);

/********************************************************************************************
 * H450_CloseCall
 * purpose : Destroys the H450 call object for the call. does nothing
 *           if USE_H450 is not defined.
 * input   : Call - pointer to information about the call
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
void H450_CloseCall(CallInfo * Call);

#ifdef USE_H450
/********************************************************************************************
 * H450_CloseCall
 * purpose : Sends the name indication
 * input   : Call - pointer to information about the call
 *           Operation - in what message will the name be inserted to.
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
void H450_sendNameID(CallInfo* Call, niOperation Operation);
#endif

/*****************************************************************************
* Local Functions
*****************************************************************************/

/* Call Transfer - H450.2
 ********************************/

/* Initiates Call Transfer service, called by the TCL */
int H450_callTransfer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Call Reroute - H450.3
 ********************************/

/* Initiates Call Reroute (deflect) service, called by the TCL */
int H450_callReroute(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Initiates Forwarding service, called by the TCL */
int H450_forwardActivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Deactivates Forwarding service, called by the TCL */
int H450_forwardDeactivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Interrogates server as to Forwarding service, called by the TCL */
int H450_forwardInterrogate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Call Hold - H450.4
 ********************************/

/* Initiates Call Hold service, called by the TCL */
int H450_callHold(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Retrieves a call from Hold, called by the TCL */
int H450_callHoldRtrv(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Answers a Remote Hold request, called by the TCL */
int H450_HoldSendNonDefaultResponse(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Park Pickup - H450.5
 ********************************/

/* Initiates Parking service, called by the TCL */
int H450_callPark(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Initiates Pickup service, called by the TCL - does not work yet */
int H450_callPick(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Notify group as to call */
int H450_groupNotification(IN CallInfo * inCall,
                           IN int        position,
                           IN BOOL       initInAlerting);

/* Call Waiting - H450.6
 ********************************/

/* Initiates Call Waiting service, called by the TCL */
int H450_callWait(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Message Waiting Indication - H450.7
 *******************************************/

/* Sends Message Indication to the Served User */
int H450_MC_ActivateMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Sends CallBack Indication to the Served User */
int H450_MC_ActivateCallBack(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Sends CallBack or Message Indication Deactivate to the Served User */
int H450_MC_Deactivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Sends Interogation Message to the Message Center */
int H450_SU_Interogate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* General function for sending non deafult response to MWI messages */
int H450_MWISendNonDefaultResponse(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/* Call Completion - H450.9
 ********************************/

/* Sends a CallCompletion Setup message */
int H450_callCompletion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Sends a CallCompletion message other than Setup */
int H450_callCompletionAction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/* Call Offer - H450.10
 ********************************/

/* Sends a CallOffer Setup message */
int H450_callOffer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/*  */
int H450_remoteUserAlerting(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/* Call Intrusion - H450.11
 ********************************/

int H450_getCallID(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

int H450_callIntrusion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

#endif

#ifdef __cplusplus
}
#endif
