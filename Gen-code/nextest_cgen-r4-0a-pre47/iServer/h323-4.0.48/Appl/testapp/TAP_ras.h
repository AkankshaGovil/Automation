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
 *                                TAP_ras.h
 *
 * This file contains all the commands that are used for the
 * GRQ/RRQ window of the test application.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/

#ifndef _TAP_RAS_H
#define _TAP_RAS_H

#include "TAP_init.h"
#include "TAP_defs.h"

/************************************************************************************************************
 * cmEvRegEvent
 * purpose : Notify the application about the fail reason and the registration status.
 *
 * input   : hAPP           - The application handle to the EPM module.
 *           State          - Registration status
 *           regEvent       - Event that occured
 *           regEventHandle - Node ID of message that caused the event
 * output  : None.
 * return  : None.
 ************************************************************************************************************/
int RVCALLCONV cmEvRegEvent(IN HAPP               hApp,
                            IN cmRegState         State,
                            IN cmRegEvent         regEvent,
                            IN int                regEventHandle);


/************************************************************************
 * cmEvRASRequest
 * purpose: Callback function that the RAS calls when a new incoming
 *          RAS request has to be handled
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *                        Set to NULL if not applicable for this type
 *                        of transaction
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle (if applicable)
 * output : lphsRas     - The application's RAS transaction handle
 *                        This should be set by the application to find
 *                        this transaction in future callbacks.
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvRASRequest(
    IN  HRAS             hsRas,
    IN  HCALL            hsCall,
    OUT LPHAPPRAS        lphaRas,
    IN  cmRASTransaction transaction,
    IN  cmRASTransport*  srcAddress,
    IN  HAPPCALL         haCall);


/************************************************************************
 * cmEvRASConfirm
 * purpose: Callback function that the RAS calls when a confirm on a
 *          RAS request is received
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvRASConfirm(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas);


/************************************************************************
 * cmEvRASReject
 * purpose: Callback function that the RAS calls when a reject on a
 *          RAS request is received
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvRASReject(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas,
    IN  cmRASReason      reason);


/************************************************************************
 * cmEvRASTimeout
 * purpose: Callback function that the RAS calls when a timeout on a
 *          RAS request occured.
 *          This will be the case when no reply arrives or when an
 *          outgoing multicast transaction is being handled (even if
 *          replies arrived).
 * input  : haRas       - The application's RAS transaction handle
 *          hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvRASTimeout(
    IN  HAPPRAS          haRas,
    IN  HRAS             hsRas);


/************************************************************************
 * cmEvAutoRASRequest
 * purpose: Callback function that the RAS calls when a new incoming
 *          automatic RAS request has to be handled.
 *          No action can be taken by the application - this is only
 *          for notification purposes.
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          hsCall      - Stack's call handle
 *                        Set to NULL if not applicable for this type
 *                        of transaction
 *          transaction - The type of transaction that arrived
 *          srcAddress  - Address of the source
 *          haCall      - Application's call handle (if applicable)
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvAutoRASRequest(
    IN  HRAS             hsRas,
    IN  HCALL            hsCall,
    IN  cmRASTransaction transaction,
    IN  cmRASTransport*  srcAddress,
    IN  HAPPCALL         haCall);


/************************************************************************
 * cmEvAutoRASConfirm
 * purpose: Callback function that the RAS calls when a confirm on an
 *          Automatic RAS request is received
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvAutoRASConfirm(
    IN  HRAS             hsRas);


/************************************************************************
 * cmEvAutoRASReject
 * purpose: Callback function that the RAS calls when a reject on an
 *          Automatic RAS request is received
 * input  : hsRas       - Stack's handle for the RAS transaction
 *          reason      - Reject reason
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvAutoRASReject(
    IN  HRAS             hsRas,
    IN  cmRASReason      reason);


/************************************************************************
 * cmEvAutoRASTimeout
 * purpose: Callback function that the RAS calls when a timeout on an
 *          Automatic RAS request occured.
 *          This will be the case when no reply arrives or when an
 *          outgoing multicast transaction is being handled (even if
 *          replies arrived).
 * input  : hsRas       - Stack's handle for the RAS transaction
 * output : none
 * return : If an error occurs, the function returns a negative value.
 *          If no error occurs, the function returns a non-negative value.
 ************************************************************************/
int RVCALLCONV cmEvAutoRASTimeout(
    IN  HRAS             hsRas);

/********************************************************************************************
 * RAS_SendBRQ
 * purpose : Send a BRQ request for more bandwidth
 * syntax  : RAS.SendBRQ <callInfo> <desiredBandwidth>
 *           <callInfo>         - Call information (counter and handle)
 *           <desiredBandwidth> - Amount of bandwidth to request
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RAS_SendBRQ(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * RAS_SendNSM
 * purpose : Send a Non Standard Message
 * syntax  : RAS.SendNSM <nsParam>
 *           <nsParam> - non standard param
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RAS_SendNSM(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * RAS_SendRAI
 * purpose : Send a Non Standard Message
 * syntax  : RAS.SendRAI <aoor>
 *           <aoor> - boolian, is almost out of resources
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int RAS_SendRAI(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


#endif  /* _TAP_RAS_H */

#ifdef __cplusplus
}
#endif
