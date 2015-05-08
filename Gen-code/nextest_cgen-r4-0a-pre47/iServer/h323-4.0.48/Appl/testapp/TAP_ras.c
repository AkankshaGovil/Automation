/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_ras.c
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


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "TAP_general.h"
#include "TAP_utils.h"
#include "TAP_ras.h"



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
                            IN int                regEventHandle)
{
    static cmRegState g_prevState = cmIdle;

    switch (State)
    {
        case cmIdle:
            TclExecute("test:SetGatekeeperStatus {Not Registered}\n"
                        "test:Log {Not Registered}");
            if(g_prevState == cmRegistered)
            {
                if(atoi(TclGetVariable("app(ras,rereg)")))
                    cmUnregister(hApp);
            }
            break;
        case cmRegistered:
            TclExecute("test:SetGatekeeperStatus Registered\n"
                       "test:Log {Registered}");
            break;
        case cmDiscovered:
            TclExecute("test:SetGatekeeperStatus Discoverd\n"
                       "test:Log {Discoverd}");
            break;
    }
    g_prevState = State;
    return 0;
}



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
    IN  HAPPCALL         haCall)
{
    BOOL        scriptMode;
    CallInfo*   Call;

    Call = (CallInfo *)haCall;

    /* Check who handles this call - the Application or an external script */
    if (Call != NULL)
        scriptMode = Call->scriptCall;
    else
        scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        char buf[128];
        TransportAddress2String(srcAddress, buf);

        /* We have to notify the script */
        TclExecute("script:cb {cb:cmras:EvRequest 0x%p 0x%p %s %s}",
            hsRas,
            haCall,
            RASTransaction2String(transaction),
            buf);
    }

    return 0;
}


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
    IN  HRAS             hsRas)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmras:EvConfirm 0x%p}", hsRas);
    }

    return 0;
}


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
    IN  cmRASReason      reason)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmras:EvReject 0x%p %s}",
                   hsRas,
                   RASReason2String(reason));
    }

    return 0;
}



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
    IN  HRAS             hsRas)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmras:EvTimeout 0x%p}", hsRas);
    }

    return 0;
}




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
    IN  HAPPCALL         haCall)
{
    BOOL        scriptMode;
    CallInfo*   Call;

    Call = (CallInfo *)haCall;

    /* Check who handles this call - the Application or an external script */
    if (Call != NULL)
        scriptMode = Call->scriptCall;
    else
        scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        char buf[128];
        TransportAddress2String(srcAddress, buf);

        /* We have to notify the script */
        TclExecute("script:cb {cb:cmautoras:EvRequest 0x%p 0x%p %s %s}",
            hsRas,
            haCall,
            RASTransaction2String(transaction),
            buf);
    }

    return 0;
}


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
    IN  HRAS             hsRas)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmautoras:EvConfirm 0x%p}", hsRas);
    }

    return 0;
}


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
    IN  cmRASReason      reason)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmautoras:EvReject 0x%p %s}",
                   hsRas,
                   RASReason2String(reason));
    }

    return 0;
}


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
    IN  HRAS             hsRas)
{
    BOOL scriptMode;

    /* Check who handles this call - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    if (scriptMode)
    {
        /* We have to notify the script */
        TclExecute("script:cb {cb:cmautoras:EvTimeout 0x%p}", hsRas);
    }

    return 0;
}


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
int RAS_SendBRQ(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
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
int RAS_SendNSM(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmNonStandardParam nsParam;

    if(atoi(TclGetVariable("app(options,nonStandardData)")))
    {
        String2CMNonStandardParam(argv[1], &nsParam);
        cmSendNonStandardMessage(hApp, &nsParam);
    }

    return TCL_OK;
}

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
int RAS_SendRAI(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    BOOL aoor;

    aoor = atoi(argv[1]);
    cmSendRAI(hApp, aoor);

    return TCL_OK;
}


#ifdef __cplusplus
}
#endif

