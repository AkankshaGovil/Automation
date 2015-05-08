/***********************************************************************************************************************

  Notice:
  This document contains information that is proprietary to RADVISION LTD..
  No part of this publication may be reproduced in any form whatsoever without
  written prior approval by RADVISION LTD..

    RADVISION LTD. reserves the right to revise this publication and make changes
    without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
*                                TAP_h450.c
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


#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include "TAP_init.h"
#include "TAP_call.h"
#include "TAP_utils.h"
#include "TAP_h450.h"

#ifdef USE_H450

HSSEAPP hSSEApp;
HSSAPP hSSApp;
HSSECALL liatHsseCall;

static int serviceCounter = 1;


/********************************************************************************************
 * CallCompletionStruct
 * Used to hold information about a CallCompletion service (H450.9)
 * hService             - SSE service handle
 * destAddr             - Destination address
 * connectionRetained   - Indication if call signaling connection is retained or not
 ********************************************************************************************/
typedef struct
{
    HSSESERVICE         hService;
    char                destAddr[256];
    BOOL                connectionRetained;
} CallCompletionStruct;


#endif /* USE_H450 */


/* Indication if H450 is initialized or not */
BOOL h450Initialized = FALSE;



/********************************************************************************************
 *                                                                                          *
 *                               Private internal functions                                 *
 *                                                                                          *
 ********************************************************************************************/

#ifdef USE_H450

/********************************************************************************************
 * convertToH450Address
 * purpose : Convert an address represented by a string to an address string that H450
 *           recognizes.
 * input   : inString       - String to convert
 * output  : resultString   - Converted string
 * return  : none
 ********************************************************************************************/
void convertToH450Address(const char* inString, char* resultString)
{
    if (isdigit((int)inString[0]))
    {
        /* If it's a transport address, we'll add the TA: prefix...
           Otherwise, we should add the TEL: prefix... */
        int a, b, c, d, e;
        if (sscanf(inString, "%d.%d.%d.%d:%d", &a, &b, &c, &d, &e) == 5)
            sprintf(resultString, "TA:%s", inString);
        else sprintf(resultString, "TEL:%s", inString);
    }
    else
        strcpy(resultString, inString);
}


/********************************************************************************************
 * convertAddrToStr
 * purpose : Convert a transport address into a string valid for H450 functions
 * input   : addr   - Address to convert
 * output  : str    - Converted string
 * return  : none
 ********************************************************************************************/
void convertAddrToStr(cmTransportAddress* addr, char* str)
{
    sprintf(str, "TA:%d.%d.%d.%d:%d",
        (int)((unsigned char *)&(addr->ip))[0], (int)((unsigned char *)&(addr->ip))[1],
        (int)((unsigned char *)&(addr->ip))[2], (int)((unsigned char *)&(addr->ip))[3],
        addr->port);
}


/********************************************************************************************
 * H450_CreateUnmarkedCMCall
 * purpose : Creates a Call Object, complete with stack handle and h450, only not numbered.
 * input   : none
 * output  : none
 * return  : the CallInfo struct of the call, NULL if failed.
 ********************************************************************************************/
CallInfo * H450_CreateUnmarkedCMCall(void)
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
    Call->counter = Call_GetCounterValue();
    Call->scriptCall = FALSE;
    Call->incoming = FALSE;
    Call->hsCall = hsCall;
    /* Set up SupServ */
    sseCreateCall(hSSEApp, &Call->hSSECall, (HSSEAPPCALL) Call, Call->hsCall);

    return Call;
}


#endif  /* USE_H450 */



/********************************************************************************************
 *                                                                                          *
 *                               Implementation of services                                 *
 *                                                                                          *
 ********************************************************************************************/

/* Call Transfer - H450.2
 ********************************/

#ifdef USE_H450

int RVCALLCONV sseEvCallTransfer(IN HSSEAPPCALL hSSEaCallPri,
                                 IN HSSECALL hSSECallPri,
                                 OUT HSSECALL * hSSECallSec)
{
    CallInfo * PriCall = (CallInfo *) hSSEaCallPri;
    CallInfo * SecCall;

    /* Create a new call handle */
    SecCall = CreateCallObject();
    if(SecCall == NULL) return -1;

    /* Set the call parameters */
    Call_SetOutgoingParams(SecCall);
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
        Call_InitFastStart(SecCall);

    /* send calling name if we have to */
    H450_sendNameID(SecCall, calling);

    /* Change state of the primery call */
    TclExecute("Call:AddRemark %d Transfered", PriCall->counter);

    /* output the call handle */
    *hSSECallSec = SecCall->hSSECall;

    return 0;
}

#endif /* USE_H450 */

int H450_callTransfer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char destAddr[256];
    /* Get Call Handle */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;
    /* make sure SupServ was implemented for the call */
    if(!Call->hSSECall) return TCL_OK;

    if ((argv[2] == NULL) || (strlen(argv[2]) == 0))
    { /* no dest address */
        CallInfo * tras2call;
        /* Get the "transferred to" call handle from argument 3 */
        if ((argv[3] == NULL) || (strlen(argv[3]) == 0)) return TCL_OK;
        if (sscanf(argv[3], "%*d 0x%p:", &tras2call) != 1) return TCL_OK;
        /* make sure SupServ was implemented for the call */
        if(!tras2call->hSSECall) return TCL_OK;
        /* and go a-go-go */
        if( (retVal = sseCallTransfer(Call->hSSECall, tras2call->hSSECall, NULL)) < 0 )
            return retVal;
    }
    else
    { /* we have a dest address, this is a blind transfer */
        /* Cosmetics the dest Address, if needed */
        convertToH450Address(argv[2], destAddr);

        /* and go a-go-go */
        if( (retVal = sseCallTransfer(Call->hSSECall, NULL, destAddr)) < 0 )
        {
            Tcl_SetResult(interp, (char *)"Couldn't initiate Call Transfer service (probably problem with the call)", TCL_STATIC);
            return TCL_ERROR;
        }
    }
#endif /* USE_H450 */
    return TCL_OK;
}

/* Call Deflection - H450.3
 ********************************/

#ifdef USE_H450

int RVCALLCONV sseEvCallReroute(IN HSSEAPPCALL hSSEaCallPri,
                                IN HSSECALL hSSECallPri,
                                OUT HSSECALL * hSSECallSec)
{
    CallInfo * PriCall = (CallInfo *) hSSEaCallPri;
    CallInfo * SecCall;

    /* Create a new call handle */
    SecCall = CreateCallObject();
    if(SecCall == NULL) return -1;

    /* Set the call parameters */
    Call_SetOutgoingParams(SecCall);
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
        Call_InitFastStart(SecCall);

    /* send calling name if we have to */
    H450_sendNameID(SecCall, calling);

    /* Change state of the primery call */
    TclExecute("Call:AddRemark %d Rerouted", PriCall->counter);

    /* output the call handle */
    *hSSECallSec = SecCall->hSSECall;

    return 0;
}

#endif /* USE_H450 */

int H450_callReroute(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char destAddr[256], locAddr[32], srcAddr[512];
    cmTransportAddress csAddr;
    cmAlias srcAlias;
    reason_e rrtRes;

    /* Get Call Handle */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0) ||
        (argv[2] == NULL) || (strlen(argv[2]) == 0) ) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    /* Cosmetics the dest (deflect to) Address, if needed */
    convertToH450Address(argv[2], destAddr);

    /* Setting the fwdCond */
    rrtRes = String2reason_e(argv[3]);

    /* Get local address */
    cmGetLocalCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, locAddr);

    /* Get calling EP address or alias */
    if(cmCallGetParam(Call->hsCall, cmParamSrcCallSignalAddress, 0,
        (int *) sizeof(cmTransportAddress), (char *) &csAddr) < 0)
    {
        /* todo: bug... */
        cmCallGetParam(Call->hsCall, cmParamSourceAddress, 0,
            (int *) sizeof(srcAlias), (char *) &srcAlias);
        Alias2String(&srcAlias, srcAddr);
    }
    else
        convertAddrToStr(&csAddr, srcAddr);

    /* make sure the message is sent in a facility by making sure alerting was sent.
     would also make sure reroute affter reroute works well */
    H450_sendNameID(Call, alerting);
    cmCallAccept(Call->hsCall);

    /* and go a-go-go */
    retVal = sseCallReroute(Call->hSSECall, rrtRes, destAddr, locAddr, srcAddr);

    /* error detection and exit */
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

int H450_forwardActivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char destAddr[256], GKAddr[32];
    cmTransportAddress csAddr;
    proc_e fwdCond;

    /* Get dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0) ||
        (argv[2] == NULL) || (strlen(argv[2]) == 0)) return TCL_OK;

    /* Cosmetics the dest (forward to) Address, if needed */
    convertToH450Address(argv[1], destAddr);

    fwdCond = String2proc_e(argv[2]);

    /* Get the Server address */
    cmGetGKCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, GKAddr);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return TCL_ERROR;
    sseCallImplementForward(Call->hSSECall);

    /* send forward request */
    retVal = sseForwardActivate(Call->hSSECall, destAddr, fwdCond, (char *)"", (char *)"", GKAddr);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

int H450_forwardDeactivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char destAddr[256], GKAddr[32];
    cmTransportAddress csAddr;
    proc_e fwdCond;

    /* Get dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0) ||
        (argv[2] == NULL) || (strlen(argv[2]) == 0)) return TCL_OK;

    /* Cosmetics the dest (forward to) Address, if needed */
    convertToH450Address(argv[1], destAddr);

    /* Setting the fwdCond */
    fwdCond = String2proc_e(argv[2]);

    /* Get the Server address */
    cmGetGKCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, GKAddr);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return TCL_ERROR;
    sseCallImplementForward(Call->hSSECall);

    /* send forward request */
    retVal = sseForwardDeactivate(Call->hSSECall, fwdCond, (char*)"", destAddr, GKAddr);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

int H450_forwardInterrogate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char locAddr[32], GKAddr[32];
    cmTransportAddress csAddr;
    proc_e fwdCond;

    /* Get condition */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* Setting the fwdCond */
    fwdCond = String2proc_e(argv[1]);

    /* Get local address */
    cmGetLocalCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, locAddr);

    /* Get the Server address */
    cmGetGKCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, GKAddr);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return TCL_ERROR;
    sseCallImplementForward(Call->hSSECall);

    /* send forward request */
    retVal = sseForwardInterogate(Call->hSSECall, locAddr, fwdCond, locAddr, GKAddr);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

#ifdef USE_H450

int RVCALLCONV sseEvForwardActivated(IN HSSEAPPCALL hSSEaCall,
                                     IN HSSECALL hSSECall)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    /* Close the call we used */
    cmCallDrop(Call->hsCall);
    return 0;
}

int RVCALLCONV sseEvForwardDeactivated(IN HSSEAPPCALL hSSEaCall,
                                       IN HSSECALL hSSECall)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    /* Wrap things up */
    cmCallDrop(Call->hsCall);
    return 0;
}

#endif /* USE_H450 */

/* Call Hold - H450.4
 ********************************/

/* Initiates Call Hold service, called by the TCL */
int H450_callHold(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    BOOL Near;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;
    Near = (argv[2][0] == '1');

    if ( (retVal = sseCallHoldInit(Call->hSSECall, Near)) < 0)
    {
        Tcl_SetResult(interp, (char *)"Couldn't initiate Call Hold service (probably wrong state)", TCL_STATIC);
        return TCL_ERROR;
    }

    if(Near)
        TclExecute("Call:AddRemark %d NearHold", Call->counter);
#endif /* USE_H450 */

    return TCL_OK;
}

/* Retrieves a call from Hold, called by the TCL */
int H450_callHoldRtrv(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;
    char remark[32];
    BOOL Near;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p: %*s %*s %s", &Call, remark) != 2) return TCL_OK;
    Near = (remark[0] == 'N');

    if( (retVal = sseCallHoldRetrieve(Call->hSSECall, Near)) < 0)
        return TCL_ERROR;

    if (Near) TclExecute("Call:RemoveRemark %d", Call->counter);
#endif /* USE_H450 */

    return TCL_OK;
}

int H450_HoldSendNonDefaultResponse(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    CallInfo * Call;
    BOOL ok;
    int err, retVal;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[2], "0x%p", &Call) != 1) return TCL_OK;
    ok = atoi(argv[3]);
    if(ok)
    {
        if(argv[1][0] == 'h') /*hold*/
            TclExecute("after idle {Call:AddRemark %d NearHeld}", Call->counter);
        if(argv[1][0] == 'r') /*retrieve*/
            TclExecute("after idle {Call:RemoveRemark %d}", Call->counter);
    }
        err = atoi(argv[4]);
    if( (retVal = sseHoldSendNonDefaultResponse(Call->hSSECall, ok, err)) < 0)
        return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

#ifdef USE_H450

/* Signals that Remote Hold Request has been received. will either answer or ask user
what to do */
int RVCALLCONV sseEvRemoteHoldInd(IN HSSEAPPCALL hSSEaCallPri,
                                  IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    if( (TclGetVariable("tmp(h450,ask)"))[0] == '0' )
    {/* answer default answer */
        if( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
            return ssDefaultNack;
        TclExecute("after idle {Call:AddRemark %d NearHeld}", Call->counter);
        return ssDefaultAck;
    }
    /* Notify User as to choice */
    TclExecute("gui:askQuestion {%d} {Remote Hold for call %d} "
        "{Other side has signaled Remote Hold request. Would you like to comply?} "
        "{H450.HoldSendNonDefaultResponse hold 0x%p 1 -1} "
        "{H450.HoldSendNonDefaultResponse hold 0x%p 0 1008 }",
        Call->counter, Call->counter, Call, Call);
    TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    return ssWait;
}

/* Signals that the remote side has either confirmed the request or has returned an error */
void RVCALLCONV sseEvRemoteHoldConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                       IN HSSECALL hSSECallPri,
                                       IN int errCode,
                                       IN UINT32 ok)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    if(ok)
        TclExecute("Call:AddRemark %d FarHold", Call->counter);
    else if(errCode == -1)
        TclExecute("test:Log {Hold request for call %d has timed out}", Call->counter);
    else
        TclExecute("test:Log {Hold request for call %d responded with errCode = %d}", Call->counter, errCode);
}

/* Signals that Remote Hold Retrieve has been received. will either answer or ask user
what to do */
int RVCALLCONV sseEvRemoteHoldRetrieveInd(IN HSSEAPPCALL hSSEaCallPri,
                                          IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    if( (TclGetVariable("tmp(h450,ask)"))[0] == '0' )
    {/* answer default answer */
        if( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
            return ssDefaultNack;
        TclExecute("after idle {Call:RemoveRemark %d }", Call->counter);
        return ssDefaultAck;
    }
    /* Notify User as to choice */
    TclExecute("gui:askQuestion {%d} {Remote Hold for call %d} "
        "{Other side has signaled Remote Hold retrieve. Would you like to comply?} "
        "{H450.HoldSendNonDefaultResponse retr 0x%p 1 -1} "
        "{H450.HoldSendNonDefaultResponse retr 0x%p 0 1008 }",
        Call->counter, Call->counter, Call, Call);
    TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    return ssWait;
}

/* Signals that the remote side has either confirmed the request or has returned an error */
void RVCALLCONV sseEvRemoteRetrieveConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                           IN HSSECALL hSSECallPri,
                                           IN int errCode,
                                           IN UINT32 ok)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    if(ok)
        TclExecute("Call:RemoveRemark %d", Call->counter);
    else if(errCode == -1)
        TclExecute("call:Log %d {Hold retrieve has timed out}", Call->counter);
    else
        TclExecute("call:Log %d {Hold retrieve responded with errCode = %d}", Call->counter, errCode);
}

/* Signals that Near Hold Request has been received. */
int RVCALLCONV sseEvNearHoldInd(IN HSSEAPPCALL hSSEaCallPri,
                                  IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    TclExecute("Call:AddRemark %d FarHeld", Call->counter);
    return 0;
}

/* Signals that Near Hold Retrieve has been received. */
int RVCALLCONV sseEvNearHoldRetrieveInd(IN HSSEAPPCALL hSSEaCallPri,
                                        IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    TclExecute("Call:RemoveRemark %d", Call->counter);
    return 0;
}

#endif /* USE_H450 */

/* Park Pickup - H450.5
 ********************************/

/* Initiates Parking service, called by the TCL */
int H450_callPark(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    CallInfo* Call;
    cmTransportAddress csAddr;
    char locAddr[128], callAddr[128], parkAddr[128];
    INT32 size = sizeof(csAddr);

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    cmGetLocalCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, locAddr);
    cmCallGetParam(Call->hsCall, cmParamCallSignalAddress, 0, &size, (char *) &csAddr);
    convertAddrToStr(&csAddr, callAddr);
    convertToH450Address(argv[2], parkAddr);

    sseCallParkInit(Call->hSSECall, locAddr, callAddr, parkAddr, 1);
    TclExecute("Call:AddRemark %d parking", Call->counter);
#endif /* USE_H450 */

    return TCL_OK;
}

/* Initiates Pickup service, called by the TCL - does not work yet */
int H450_callPick(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int i, parkPos;
    CallInfo* Call;
    cmTransportAddress csAddr;
    char locAddr[128], callAddr[128], parkAddr[128], callId[16];
    BOOL isLocal;

    cmGetLocalCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, locAddr);

    /*check if local pickup*/
    isLocal = atoi(TclGetVariable("app(h450,localPickup)"));
    if(isLocal)
    {
        int  size=16;

        /* Get a call to send with */
        if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
        if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;
        
        cmCallGetParam(Call->hsCall, cmParamCallID, 0, &size, (char *) callId);
        
        sseCallLocalPickupInit(Call->hSSECall,callId,locAddr);
    } 
    else
    {
        /* Make sure a parked call is selected */
        if (!strcmp(argv[2], "Pick")) return TCL_OK;

        /* get park position */
        parkPos = atoi(argv[2]);
        /* get the Call ID */
        for(i=0; i<16; i++)
        {
            UINT32 tmpVal;
            sscanf(argv[3]+2*i, "%02X", &tmpVal);
            callId[i] = (char)tmpVal;
        }
        
        /* if not registered */
        if(!strncmp(argv[4], "TNAME:", 6))
            sprintf(callAddr, "TA:%s", argv[3]+6);
        else
            strcpy(callAddr, argv[4]);
        if(!strncmp(argv[5], "TNAME:", 6))
            sprintf(parkAddr, "TA:%s", argv[5]+6);
        else
            strcpy(parkAddr, argv[4]);
        
        /* Make a call to send with */
        Call = CreateCallObject();
        if(Call == NULL) return -1;

        /* Set the call parameters */
        Call_SetOutgoingParams(Call);
        if(atoi(TclGetVariable("app(newcall,fastStart)")))
            Call_InitFastStart(Call);

        /* send calling name if we have to */
        H450_sendNameID(Call, calling);

        liatHsseCall = Call->hSSECall;
        sseCallPickupInit(Call->hSSECall, callId, locAddr, callAddr, parkAddr, parkPos);
        cmCallMake(Call->hsCall,0,0,parkAddr,locAddr,(char *) "picking up call",NULL,0);
    }
    TclExecute("Call:AddRemark %d {Picking Up} ", Call->counter);
#endif /* USE_H450 */

    return TCL_OK;
}

#ifdef USE_H450

int RVCALLCONV sseEvCParkRequestInd(IN  HSSEAPPCALL hSSEaCallPri,
                                    IN  HSSECALL    hSSECallPri,
                                    OUT HSSECALL*   callSec)
{
    BOOL possible;
    CallInfo * PriCall = (CallInfo *) hSSEaCallPri;
    CallInfo * SecCall;

    /*check if park is possible*/
    possible = atoi(TclGetVariable("app(h450,enablePrk)"));
    if (!possible) return RVERROR;

    /* Create a new call handle */
    SecCall = CreateCallObject();
    if(SecCall == NULL) return RVERROR;

    /* Set the call parameters */
    Call_SetOutgoingParams(SecCall);
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
        Call_InitFastStart(SecCall);

    /* send calling name if we have to */
    H450_sendNameID(SecCall, calling);

    /* Add remarks to the calls */
    TclExecute("Call:AddRemark %d ToPark", PriCall->counter);
    TclExecute("after idle {Call:AddRemark %d Parking}", SecCall->counter);

    /* output the call handle */
    *callSec = SecCall->hSSECall;

    return 0;
}

int RVCALLCONV sseEvCPickExeInd(IN  HSSEAPPCALL hSSEaCall,
                                   IN  HSSECALL    call,
                                   IN  char*       callIdentifier,
                                   OUT int*        reason)
{
    return 0;
}

int RVCALLCONV sseEvCPickupRequestInd(IN  HSSEAPPCALL hSSEaCall,
                                   IN  HSSECALL    call,
                                   IN  char*       callIdentifier)
{
    CallInfo * reqCall = (CallInfo *) hSSEaCall;
    cmCallSendCallProceeding(reqCall->hsCall);
    return 0;
}

int RVCALLCONV sseEvCPickupFindCall(IN  HSSEAPPCALL hSSEaCall,
                                       IN  HSSECALL    call,
                                       IN  char*       callIdentifier,
                                       IN  BOOL        IsIncoming,
                                       OUT HSSECALL  * oldHSSCall)
{
    CallInfo * parkedCall = (CallInfo *) hSSEaCall;
    char CallIDstr[33];
    char * callStr;
    CallInfo * pickedCall;
    int i;

    for(i=0; i<16; i++)
        sprintf(CallIDstr+2*i, "%02X", (unsigned char)callIdentifier[i]);
    CallIDstr[32] = 0;

    TclExecute("after idle {Call:AddRemark %d Pickup-Call}", parkedCall->counter);
    if (IsIncoming)
    {
        TclExecute("findIncomingCallID {%s}", CallIDstr);
    }
    else
    {
        TclExecute("findOutgoingCallID {%s}", CallIDstr);
        *oldHSSCall = liatHsseCall;
        return 0;
    }

    callStr = Tcl_GetStringResult(interp);
    if (sscanf(callStr, "%*d 0x%p:", &pickedCall) != 1) return -1;
    *oldHSSCall = pickedCall->hSSECall;

    return 0;
}

int RVCALLCONV sseEvCPickupInd(IN  HSSEAPPCALL hSSEaCall,
                                  IN  HSSECALL    call,
                                  OUT char*       locAddr,
                                  OUT HSSECALL*   pickupCall)
{
    CallInfo * pickedCall;
    CallInfo * pickingCall = (CallInfo *) hSSEaCall;
    cmTransportAddress csAddr;

    if (pickingCall->callState != cmCallStateConnected) /*in case of remote pickup*/
    {

        /* Create a new call handle */
        pickedCall = CreateCallObject();
        if(pickedCall == NULL)
            return TclExecute("test:Log {Error creating call}");

        /* Set the call parameters */
        Call_SetOutgoingParams(pickedCall);
        if(atoi(TclGetVariable("app(newcall,fastStart)")))
            Call_InitFastStart(pickedCall);

        /* send calling name if we have to */
        H450_sendNameID(pickedCall, calling);

        TclExecute("after idle {Call:AddRemark %d Picked}", pickedCall->counter);
        TclExecute("after idle {Call:AddRemark %d Picking}", pickingCall->counter);

        *pickupCall = pickedCall->hSSECall;
        /*get local adress*/
        cmGetLocalCallSignalAddress(hApp,&csAddr);
        convertAddrToStr(&csAddr, locAddr);
    }
    return 0;
}

int RVCALLCONV sseEvCPSetupInd(IN  HSSEAPPCALL       hSSEaCall,
                               IN  HSSECALL          call,
                               OUT int             * parkPosition,
                               OUT cpCallCondition * parkCondition)
{
    CallInfo * setupCall = (CallInfo *) hSSEaCall;

    /*check if park is possible*/
    int possible = atoi(TclGetVariable("app(h450,enablePrk)"));
    if (!possible) return RVERROR;

    *parkPosition = setupCall->counter;
    *parkCondition = parkedToGroup;

    cmCallSendCallProceeding(setupCall->hsCall);
    return H450_groupNotification(setupCall, *parkPosition, FALSE);
}

#endif  /* USE_H450 */

int H450_groupNotification(IN CallInfo * inCall,
                           IN int        position,
                           IN BOOL       initInAlerting)
{
#ifdef USE_H450
    CallInfo * sendCall;
    char callId[16], callIdStr[33];
    cmTransportAddress csAddr;
    char locAddr[128], callAddr[128], groupAddr[128], * groupPtr;
    INT32 size = sizeof(callId);
    int i, stop;

    cmCallGetParam(inCall->hsCall, cmParamCallID, 0, &size, (char *) callId);
    for(i=0; i<16; i++)
        sprintf(callIdStr+2*i, "%02X", (unsigned char)callId[i]);
    callIdStr[32] = 0;

    cmGetLocalCallSignalAddress(hApp,&csAddr);
    convertAddrToStr(&csAddr, locAddr);
    cmCallGetParam(inCall->hsCall, cmParamCallSignalAddress, 0, &size, (char *) &csAddr);
    convertAddrToStr(&csAddr, callAddr);

    /* notify group as to parked call */
    TclExecute("$tmp(h450Tab).service.prkpku.eps.01 index last");
    stop = atoi(Tcl_GetStringResult(interp));
    for(i=0; i <= stop; i++)
    {
        TclExecute("$tmp(h450Tab).service.prkpku.eps.01 entrycget %d -label", i);
        groupPtr = Tcl_GetStringResult(interp);
        if(!groupPtr[0]) break;
        convertToH450Address(groupPtr, groupAddr);

        /* Make a call to send with */
        sendCall = H450_CreateUnmarkedCMCall();
        if(sendCall == NULL) return -1;

        if (sseCallGroupIndOn(sendCall->hSSECall, callId, groupAddr, callAddr, locAddr, parkedCall, position) >= 0)
        {
            cmCallMake(sendCall->hsCall, 0, 0, groupAddr, (char *) locAddr, (char *) "Call Parked", NULL, 0);
        }
        else
        {
            cmCallDrop(sendCall->hsCall);
        }
    }

    if (initInAlerting)
        sseCallAlertParkInt(inCall->hSSECall);
#endif  /* USE_H450 */
    return 0;
}


#ifdef USE_H450

int RVCALLCONV sseEvCPGroupIndication(IN HSSEAPPCALL hSSEaCall,
                                      IN HSSECALL    call,
                                      IN int         parameters,
                                      IN BOOL        IsIndOn)
{
    INTPTR intptr;
    BOOL isString;
    cmAlias alias;
    char callIdentifier[16], callIdStr[33], callAddr[128], parkAddr[128];
    int position, i, nodeId, callIdentifierSize;
    HPVT hVal = ssGetValTree(hSSApp);

    /*get the call Identifier */
    strcpy(callIdentifier, "");
    nodeId = pvtGetByPath(hVal, parameters, "callPickupId.guid", NULL, &callIdentifierSize,NULL);
    
    if(nodeId >= 0)
    {
        pvtGetString(hVal, nodeId, callIdentifierSize, callIdentifier);
        for(i=0; i<16; i++)
            sprintf(callIdStr+2*i, "%02X", (unsigned char)callIdentifier[i]);
        callIdStr[32] = 0;
    }

    if(IsIndOn)
    {
        if(cmVt2Alias(hVal, &alias, pvtGetNodeIdByPath(hVal, parameters, "partyToRetrieve.destinationAddress.1")) >= 0)
            Alias2String(&alias, callAddr);

        if(cmVt2Alias(hVal, &alias, pvtGetNodeIdByPath(hVal, parameters, "retrieveAddress.destinationAddress.1")) >= 0)
            Alias2String(&alias, parkAddr);

        pvtGetByPath(hVal, parameters, "parkPosition", &intptr, (INT32 *) &position, &isString);

        TclExecute("$tmp(h450Tab).service.prkpku.calls.01 add radiobutton -indicatoron 0 -value {%d %s %s %s} -variable tmp(h450,callPku) -label {%d %s %s %s}",
            position, callIdStr, callAddr, parkAddr,
            position, callIdStr, callAddr, parkAddr);
    }
    else
    {
        TclExecute("h4505DelGropupInd %s", callIdStr);
    }

    return 0;
}

int RVCALLCONV sseEvCPRequestConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                     IN HSSECALL    hSSECallPri,
                                     IN int         errCode,
                                     IN BOOL        ok)
{
    CallInfo * confCall = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received Call Park request confirmation: err=%d, ok=%d}", confCall->counter, errCode, ok);
    return 0;
}

int RVCALLCONV sseEvCPPickRequConfirm(IN HSSEAPPCALL hSSEaCall,
                                      IN HSSECALL    call,
                                      IN int         errCode,
                                      IN int         ok,
                                      IN char*       callIdentifier)
{
    CallInfo * confCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received Call PickRequ confirmation: err=%d, ok=%d}", confCall->counter, errCode, ok);
    return 0;

}

int RVCALLCONV sseEvCPPickupConfirm(IN HSSEAPPCALL    hSSEaCall,
                                    IN HSSECALL       call,
                                    IN int            errCode,
                                    IN int            ok,
                                    IN char*          callIdentifier)
{
    CallInfo * setupCall = (CallInfo *) hSSEaCall, * sendCall;
    char locAddr[128],  groupAddr[128], * groupPtr;
    int i, stop;

    if (ok)
    {
        /* notify group as to parked call */
        TclExecute("$tmp(h450Tab).service.prkpku.eps.01 index last");
        stop = atoi(Tcl_GetStringResult(interp));
        for(i=0; i <= stop; i++)
        {
            TclExecute("$tmp(h450Tab).service.prkpku.eps.01 entrycget %d -label", i);
            groupPtr = Tcl_GetStringResult(interp);
            if(!groupPtr[0]) break;
            if(strncmp(groupPtr,"TEL",3) != 0)
            {
                sprintf(groupAddr, "TA:%s", groupPtr);
            }
            else
            {
                sprintf(groupAddr,"%s",groupPtr);
            }

            /* Make a call to send with */
            sendCall = CreateCallObject();
            if(sendCall == NULL) return -1;

            /* Set the call parameters */
            Call_SetOutgoingParams(sendCall);
            if(atoi(TclGetVariable("app(newcall,fastStart)")))
                Call_InitFastStart(sendCall);

            /* send calling name if we have to */
            H450_sendNameID(sendCall, calling);

            if (sseCallGroupIndOff(sendCall->hSSECall, callIdentifier, groupAddr) >= 0)
            {
                cmCallMake(sendCall->hsCall, 0, 0, groupAddr, (char *) locAddr,
                    (char *) "Call Parked", NULL, 0);
            }
            else
            {
                cmCallDrop(sendCall->hsCall);
            }
        }
        TclExecute("call:Log %d {Received Call Pick request confirmation: err=%d, ok=%d}", setupCall->counter, errCode, ok);
    }
    return 0;
}

int RVCALLCONV sseEvCPSetupConfirm(IN HSSEAPPCALL hSSEaCall,
                                   IN HSSECALL    call,
                                   IN int         errCode,
                                   IN int         ok,
                                   IN int*        reason)
{
                    /*if the OK is true but the error code is 0 
                so the application will know that in that case
                it means the timer had finished and no pickup
                was done*/

    CallInfo * confCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received Call Park setup confirmation: err=%d, ok=%d}", confCall->counter, errCode, ok);
    return 0;
}

int RVCALLCONV sseEvCPGroupIndicationConfirm(IN HSSEAPPCALL hSSEaCall,
                                             IN HSSECALL    call,
                                             IN int         errCode,
                                             IN int         ok,
                                             IN BOOL        indOn)
{
    CallInfo * confCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received Call Park group indication confirmation: err=%d, ok=%d, ind=%d}",
        confCall->counter, errCode, ok, indOn);
    cmCallDrop(confCall->hsCall);
    return 0;
}

#endif /* USE_H450 */

/* Call Waiting - H450.6
 ********************************/

/* Initiates Call Waiting service, called by the TCL */
int H450_callWait(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    if ( (retVal = sseCallWaitInit(Call->hSSECall)) < 0)
    {
        Tcl_SetResult(interp, (char *)"Cannot execute Call Waiting for some reason", TCL_STATIC);
        return TCL_ERROR;
    }

    TclExecute("Call:AddRemark %d Wait", Call->counter);
#endif /* USE_H450 */

    return TCL_OK;
}

#ifdef USE_H450

/* called when Wait service has been activated, notifies user. */
void RVCALLCONV sseEvWaitInd(IN HSSEAPPCALL hSSEaCallPri,
                             IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    TclExecute("after 400 {Call:AddRemark %d Waiting}", Call->counter);
}

/* Called when Wait service's timer has expired. drops call. */
void RVCALLCONV sseEvWaitTimerExpired(IN HSSEAPPCALL hSSEaCallPri,
                                      IN HSSECALL hSSECallPri)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;

    cmCallDrop(Call->hsCall);
}

#endif /* USE_H450 */

/* Message Waiting Indication - H450.7
 *******************************************/

/* Sends Message Indication to the Served User */
int H450_MC_ActivateMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo * Call;
    char ServedUser[256];

    /* Check dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* Cosmetics the dest (Served User) Address, if needed */
    convertToH450Address(argv[1], ServedUser);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return -1;
    /* send forward request */
    retVal = sseMWIActivate(Call->hSSECall, ServedUser, (char*)"allServices", NULL, 1, NULL, NULL, 0);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

/* Sends CallBack Indication to the Served User */
int H450_MC_ActivateCallBack(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo * Call;
    char ServedUser[256];

    /* Check dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* Cosmetics the dest (Served User) Address, if needed */
    convertToH450Address(argv[1], ServedUser);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return -1;
    /* send forward request */
    retVal = sseMWIActivateCallBack(Call->hSSECall, ServedUser, (char*)"allServices", (char*)"TA:127.0.0.1:1271", NULL, NULL, 0);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

/* Sends CallBack or Message Indication Deactivate to the Served User */
int H450_MC_Deactivate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo * Call;
    char ServedUser[256];
    BOOL CallBack;

    /* Check dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* Cosmetics the dest (Served User) Address, if needed */
    convertToH450Address(argv[1], ServedUser);

    /* Detemine if to deactivate CallBacks or messages */
    CallBack = (argv[2][0] == '1');
    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return -1;
    /* send forward request */
    retVal = sseMWIDeactivate(Call->hSSECall, ServedUser, (char*)"allServices", NULL, CallBack, 1);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

/* Sends Interogation Message to the Message Center */
int H450_SU_Interogate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo * Call;
    char MessageCenter[256];

    /* Check dest Adress */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* Cosmetics the dest (Served User) Address, if needed */
    convertToH450Address(argv[1], MessageCenter);

    /* Make a call to send with */
    Call = H450_CreateUnmarkedCMCall();
    if(Call == NULL) return -1;
    /* send forward request */
    retVal = sseMWIInterrogate(Call->hSSECall, MessageCenter, (char*)"allServices", NULL, 0, 0);
    if(retVal < 0 ) return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

/* General function for sending non deafult response to MWI messages */
int H450_MWISendNonDefaultResponse(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    CallInfo * Call;
    BOOL ok;
    int err, retVal;
    interResT garb[1] = { { "allServices", { 0, "", "", "" }, 0, "", "", 9 } }  ;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "0x%p", &Call) != 1) return TCL_OK;
    ok = atoi(argv[2]);
    err = atoi(argv[3]);
    if( (retVal = sseMWISendNonDefaultResponse(Call->hSSECall, ok, 1, garb, err)) < 0)
        return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}

#ifdef USE_H450

int RVCALLCONV sseEvMWIActivateInd(IN HSSEAPPCALL hSSEaCallPri,
                                   IN HSSECALL hSSECallPri,
                                   IN char * servedUserNr,
                                   IN int numOfMsgs,
                                   IN char * service,
                                   IN MsgCtrIdT * msgCtrId,
                                   IN char * origNum,
                                   IN char * time,
                                   IN int priority)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;
    TclExecute("test:Log {Received MWI Activate Indication: service: %s, %d messages}",
        service, numOfMsgs);

    if( (TclGetVariable("tmp(h450,ask)"))[0] == '0' )
    {/* answer default answer */
        if( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
            return ssDefaultNack;
        return ssDefaultAck;
    }
    /* Notify User as to choice */
    TclExecute("gui:askQuestion {%d} {MWI Activate} "
        "{MessageCenter has activated a Message Waiting indication. Confirm?} "
        "{H450.MWISendNonDefaultResponse 0x%p 1 -1} "
        "{H450.MWISendNonDefaultResponse 0x%p 0 1008 }",
        Call->counter, Call, Call);
    TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    return ssWait;
}

void RVCALLCONV sseEvMWIActivateConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                        IN HSSECALL hSSECallPri,
                                        IN int errCode,
                                        IN UINT32 ok)
{
    if(ok)
        TclExecute("test:Log {Received MWI Activate Confirmation}");
    else if(errCode == -1)
        TclExecute("test:Log {MWI Activate for has timed out}");
    else
        TclExecute("test:Log {MWI Activate for responded with errCode = %d}", errCode);
}

int RVCALLCONV sseEvMWIDeactivateInd(IN HSSEAPPCALL hSSEaCallPri,
                                     IN HSSECALL hSSECallPri,
                                     IN char * servedUserNr,
                                     IN char * service,
                                     IN MsgCtrIdT * msgCtrId,
                                     IN int isCallBackReq)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;
    TclExecute("test:Log {Received MWI Deactivate Indication: service: %s}", service);

    if( (TclGetVariable("tmp(h450,ask)"))[0] == '0' )
    {/* answer default answer */
        if( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
            return ssDefaultNack;
        return ssDefaultAck;
    }
    /* Notify User as to choice */
    if(isCallBackReq)
    {
        TclExecute("gui:askQuestion {%d} {MWI Deactivate} "
            "{MessageCenter has Deactivated a CallBack indication. Confirm?} "
            "{H450.MWISendNonDefaultResponse 0x%p 1 -1} "
            "{H450.MWISendNonDefaultResponse 0x%p 0 1008 }",
            Call->counter, Call, Call);
        TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    }
    else
    {
        TclExecute("gui:askQuestion {%d} {MWI Deactivate} "
            "{MessageCenter has Deactivated a Message Waiting indication. Confirm?} "
            "{H450.MWISendNonDefaultResponse 0x%p 1 -1} "
            "{H450.MWISendNonDefaultResponse 0x%p 0 1008 }",
            Call->counter, Call, Call);
        TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    }
    return ssWait;
}

void RVCALLCONV sseEvMWIDeactivateConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                          IN HSSECALL hSSECallPri,
                                          IN int errCode,
                                          IN UINT32 ok)
{
    if(ok)
        TclExecute("test:Log {Received MWI Deactivate Confirmation}");
    else if(errCode == -1)
        TclExecute("test:Log {MWI Deactivate for has timed out}");
    else
        TclExecute("test:Log {MWI Deactivate for responded with errCode = %d}", errCode);
}

void RVCALLCONV sseEvMWIInterrogateConfirm(IN HSSEAPPCALL hSSEaCallPri,
                                           IN HSSECALL hSSECallPri,
                                           IN int resultElements,
                                           IN int errCode,
                                           IN int ok)
{
    if(ok)
        TclExecute("test:Log {Received MWI Interrogate Confirmation}");
    else if(errCode == -1)
        TclExecute("test:Log {MWI Interrogate for has timed out}");
    else
        TclExecute("test:Log {MWI Interrogate for responded with errCode = %d}", errCode);
}

int RVCALLCONV sseEvMWIInterrogateInd(IN HSSEAPPCALL hSSEaCallPri,
                                      IN HSSECALL hSSECallPri,
                                      IN char * servedUserNr,
                                      IN char * service,
                                      IN MsgCtrIdT * msgCtrId,
                                      IN int isCallBackReq)
{
    CallInfo * Call;
    Call = (CallInfo *) hSSEaCallPri;
    TclExecute("test:Log {Received MWI Interrogation Indication: service: %s}", service);

    if( (TclGetVariable("tmp(h450,ask)"))[0] == '0' )
    {/* answer default answer */
        if( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
            return ssDefaultNack;
        return ssDefaultAck;
    }
    /* Notify User as to choice */
    TclExecute("gui:askQuestion {%d} {MWI Activate} "
        "{Client had sent an interrogation message. Confirm?} "
        "{H450.MWISendNonDefaultResponse 0x%p 1 -1} "
        "{H450.MWISendNonDefaultResponse 0x%p 0 1008 }",
        Call->counter, Call, Call);
    TclExecute("Call:addWindowToList 0x%p .q%d", Call, Call->counter);
    return ssWait;
}

#endif /* USE_H450 */

/* Name ID - H450.8
 ********************************/

#ifdef USE_H450

/* sends the name indication */
void H450_sendNameID(CallInfo * Call, niOperation Operation)
{
    char * Type;

    if(Call->hSSECall == NULL) return;

    Type = TclGetVariable("app(h450,nameType)");
    if( (Type[0] == 'N') && (Type[2] == 'n') ) /* None */
        return;
    if(Type[0] == 'N') /* NotAvailable */
        sseNISend(Call->hSSECall, Operation, (char*)"", (char*)"", notAvailable, FALSE);
    else
    {
        niNameChoice Choice = notAvailable; /* allowed restricted notAvailable */
        char * Name=NULL;

        if(Type[0] == 'R') Choice = restricted;
        else if(Type[0] == 'A') Choice = allowed;

        Name = TclGetVariable("app(h450,nameID)");
        if ((Name == NULL) || (strlen(Name) == 0)) return;

        sseNISend(Call->hSSECall, Operation, Name, (char*)"", Choice, FALSE);
    }
}

char * niNameChoice2String(niNameChoice choice)
{
    if(choice == allowed) return (char*)"allowed";
    if(choice == restricted) return (char*)"restricted";
    if(choice == notAvailable) return (char*)"notAvaliable";
    return (char*)"";
}

void RVCALLCONV sseEvNIAlertingNameInd(IN HSSEAPPCALL hSSEaCall,
                                       IN HSSECALL hSSECall,
                                       IN char * simple_name,
                                       IN char * character_set,
                                       IN niNameChoice choice)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    TclExecute("test:Log {Received %s name for call %d : %s}",
        niNameChoice2String(choice),
        Call->counter,
        (choice == notAvailable) ? "" : simple_name);
    simple_name[0] = 0;
}

void RVCALLCONV sseEvNIBusyNameInd(IN HSSEAPPCALL hSSEaCall,
                                   IN HSSECALL hSSECall,
                                   IN char * simple_name,
                                   IN char * character_set,
                                   IN niNameChoice choice)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    TclExecute("test:Log {Received %s name for call %d : %s}",
        niNameChoice2String(choice),
        Call->counter,
        (choice == notAvailable) ? "" : simple_name);
    simple_name[0] = 0;
}

void RVCALLCONV sseEvNICallingNameInd(IN HSSEAPPCALL hSSEaCall,
                                      IN HSSECALL hSSECall,
                                      IN char * simple_name,
                                      IN char * character_set,
                                      IN niNameChoice choice)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    TclExecute("test:Log {Received %s name for call %d : %s}",
        niNameChoice2String(choice),
        Call->counter,
        (choice == notAvailable) ? "" : simple_name);
    simple_name[0] = 0;
}

void RVCALLCONV sseEvNIConnectedNameInd(IN HSSEAPPCALL hSSEaCall,
                                        IN HSSECALL hSSECall,
                                        IN char * simple_name,
                                        IN char * character_set,
                                        IN niNameChoice choice)
{
    CallInfo * Call = (CallInfo *) hSSEaCall;
    TclExecute("test:Log {Received %s name for call %d : %s}",
        niNameChoice2String(choice),
        Call->counter,
        (choice == notAvailable) ? "" : simple_name);
    simple_name[0] = 0;
}

#endif /* USE_H450 */



/* Call Completion - H450.9
 ********************************/

/* Sends a CallCompletion Setup message */
int H450_callCompletion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    CallCompletionStruct*   ccInfo;
    HSSESERVICE             hService;
    char                    srcAddr[32];
    cmTransportAddress      locAddr;
    CallInfo*               Call;
    CallInfo*               OriginalCall;
    HCALL                   hsOrigCall = NULL;
    BOOL                    canRetainService;
    BOOL                    isBusy;

    /* Validity checks */
    if ((argv[2] == NULL) || (strlen(argv[2]) == 0)) return TCL_OK;

    /* Create a call for this service */
    Call = CreateCallObject();
    if(Call == NULL)
        return TCL_ERROR;

    /* Set the call parameters */
    Call_SetOutgoingParams(Call);
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
        Call_InitFastStart(Call);
    
    /* send calling name if we have to */
    H450_sendNameID(Call, calling);

    ccInfo = (CallCompletionStruct *)AppAlloc(sizeof(CallCompletionStruct));
    memset(ccInfo, 0, sizeof(CallCompletionStruct));

    /* Get the original call if we have one */
    if ((argc > 3) && (argv[3] != NULL) && (strlen(argv[3]) > 0))
    {
        if (sscanf(argv[3], "%*d 0x%p", &OriginalCall) == 1)
            hsOrigCall = OriginalCall->hsCall;
    }

    convertToH450Address(argv[2], ccInfo->destAddr);
    cmGetLocalCallSignalAddress(hApp, &locAddr);
    convertAddrToStr(&locAddr, srcAddr);

    isBusy = atoi(argv[1]);
    canRetainService = atoi(TclGetVariable("app(h450,canRetainServ)"));
    ccInfo->connectionRetained = TRUE;

    /* Complete the call... */
    hService = sseCallCompletion(Call->hSSECall, (HSSEAPPSERVICE)ccInfo,
        srcAddr, ccInfo->destAddr, hsOrigCall, (char*)"allServices", isBusy,
        canRetainService, ccRetentionDontCare);
    if (hService == NULL)
    {
        AppFree(ccInfo);
        return TCL_ERROR;
    }
    ccInfo->hService = hService;
    TclExecute("h4509AddService {%d 0x%p - Outgoing}", serviceCounter++, ccInfo);

#endif /* USE_H450 */
    return TCL_OK;
}

/* Sends a CallCompletion message other than Setup */
int H450_callCompletionAction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    CallCompletionStruct*   ccInfo;
    int                     ret = 0;
    HSSECALL                hsseCall;
    CallInfo*               Call;
    char*                   pSrcAddr;
    char                    srcAddr[256];
    cmTransportAddress      locAddr;

    /* Validity checks */
    if ((argc != 3) || (strlen(argv[2]) == 0)) return TCL_OK;

    /* Get the service for this call */
    if (sscanf(argv[2], "0x%p", &ccInfo) != 1) return TCL_OK;

    if (!ccInfo->connectionRetained || (strcmp("Ringout", (argv[1])) == 0))
    {
        /* We have to create and use a new call for this one */
        Call = CreateCallObject();
        hsseCall = Call->hSSECall;
        cmGetLocalCallSignalAddress(hApp, &locAddr);
        convertAddrToStr(&locAddr, srcAddr);
        pSrcAddr = srcAddr;
        /* Set the call parameters */
        Call_SetOutgoingParams(Call);
        if(atoi(TclGetVariable("app(newcall,fastStart)")))
            Call_InitFastStart(Call);
        /* send calling name if we have to */
        H450_sendNameID(Call, calling);
    }
    else
    {
        /* We have a call in SSE already... */
        hsseCall = NULL;
        pSrcAddr = NULL;
    }

    if (strcmp("Cancel", (argv[1])) == 0)
        ret = sseCallCompletionCancel(ccInfo->hService, hsseCall, pSrcAddr, ccInfo->destAddr);
    else if (strcmp("Resume", (argv[1])) == 0)
        ret = sseCallCompletionResume(ccInfo->hService, hsseCall, pSrcAddr, ccInfo->destAddr);
    else if (strcmp("Ringout", (argv[1])) == 0)
    {
        ret = sseCallCompletionRingout(ccInfo->hService, hsseCall, pSrcAddr, ccInfo->destAddr);
    }
    else if (strcmp("ExecPossible", (argv[1])) == 0)
        ret = sseCallCompletionExecPossible(ccInfo->hService, hsseCall, pSrcAddr, ccInfo->destAddr);

    if (ret < 0)
        return TCL_ERROR;
#endif /* USE_H450 */
    return TCL_OK;
}


#ifdef USE_H450
/********************************************************************************************
 * sseEvServiceHandleCreated
 * purpose: Creation of a service handle indication for the application
 *          Called only for the services in sseServiceType, which were created by network events.
 * input  : servType    - Service type that was created
 *          hSSEaCall   - The application's handle of the SSE call that caused the creation
 *                        of this service
 *          hSSECall    - The SSE handle of the SSE call that caused the creation
 *                        of this service
 *          hSSEService - HSSE Service created
 * output : hSSEaService- Application's handle for the service
 ********************************************************************************************/
int RVCALLCONV sseEvServiceHandleCreated(
        IN      sseServiceType  servType,
        IN      HSSEAPPCALL     hSSEaCall,
        IN      HSSECALL        hSSECall,
        IN      HSSESERVICE     hSSEService,
        OUT     HSSEAPPSERVICE* hSSEaService)
{
    CallCompletionStruct* ccInfo;
    CallInfo* CurrentCall = (CallInfo *)hSSEaCall;

    /* Make sure it's the only service we can handle */
    if (servType != sseServiceCallCompletion)
        return RVERROR;

    ccInfo = (CallCompletionStruct *)AppAlloc(sizeof(CallCompletionStruct));
    memset(ccInfo, 0, sizeof(CallCompletionStruct));
    ccInfo->hService = hSSEService;
    ccInfo->connectionRetained = TRUE;

    {
        /* We're the destination of the service - see who's the caller if we'll need to
           contact him later on a different call */
        cmAlias             alias;
        char                aliasStr[256];
        int                 ccSize = sizeof(alias);

        alias.string = aliasStr;
        if (cmCallGetParam(CurrentCall->hsCall, cmParamSourceAddress, 0, &ccSize, (char*)&alias) >= 0)
        {
            /* Get only the first alias of the source - because we're lazy */
            Alias2String(&alias, ccInfo->destAddr + strlen(ccInfo->destAddr));
        }
    }

    TclExecute("h4509AddService {%d 0x%p - Incoming}", serviceCounter++, ccInfo);
    TclExecute("test:Log {Creating CallCompletion service 0x%p}", ccInfo);

    *hSSEaService = (HSSEAPPSERVICE)ccInfo;
    return 0;
}


/********************************************************************************************
 * sseEvServiceHandleIdle
 * purpose: Notification for the application to cancel the service handle
 *          Called only for the services in sseServiceType.
 * input  : hSSEaService- Application's handle of the SSE service
 *          hSSEService - HSSE Service to cancel
 *          servType    - Service type that was created
 * output : none
 ********************************************************************************************/
int RVCALLCONV sseEvServiceHandleIdle(
        IN      HSSEAPPSERVICE  hSSEaService,
        IN      HSSESERVICE     hSSEService,
        IN      sseServiceType  servType)
{
    switch (servType)
    {
        case sseServiceCallCompletion:
        {
            /* Let's clean the application's information about this service */
            CallCompletionStruct* ccInfo = (CallCompletionStruct *)hSSEaService;

            TclExecute("test:Log {Killing CallCompletion service 0x%p}", ccInfo);
            TclExecute("h4509RemoveService 0x%p", ccInfo);

            AppFree(ccInfo);
            break;
        }

        default:
            /* This option should never happen */
            return RVERROR;
    }

    return 0;
}


/************************************************************************
 * sseEvCCFindService
 * purpose: Callback function that is called when the stack a CallCompletion (H450.9)
 *			message that it can't a relevant service for.
 *			This is the case whenever the call signalling connection is not retained,
 *			and there's no CallIdentifier inside the messages of the service.
 * input  : hSSEaCall   - The application's handle of the SSE call
 *          hSSECall    - The SSE handle of the SSE call
 *          hSSEaService- SSE Application's handle of the service, that the stack thinks
 *					      is the right one.
 *						  NULL if stack couldn't find any service that matches
 *          hSSEService - SSE Service of CallCompletion, that the stack thinks is the right
 *						  one (can be overridden by the application)
 *						  NULL if stack couldn't find any service that matches
 *			argNodeId	- Argument nodeId, holding the information received in the
 *					      H450.9 message.
 * output : hSSEService - SSE Service this message belongs to
 *						  If set to NULL by the application, then this message won't
 *						  be processed any further and the service will be canceled.
 * return : none
 ************************************************************************/
void RVCALLCONV sseEvCCFindService(
        IN      HSSEAPPCALL     hSSEaCall,
        IN      HSSECALL        hSSECall,
        IN      HSSEAPPSERVICE  hSSEaService,
        INOUT   HSSESERVICE*    hSSEService,
		IN		int				argNodeId)
{
    CallCompletionStruct* ccInfo;

    if (*hSSEService != NULL)
    {
        /* SSE found a matching service - no need for the application to trouble itself */
        return;
    }

    /* The way we do it in the test application is just see if the GUI has a selected service.
       Otherwise, we just clear it up. */
    if (sscanf(TclGetVariable("tmp(h450,compFoundService)"), "0x%p", &ccInfo) == 1)
        *hSSEService = ccInfo->hService;
    else
    {
        TclExecute("test:Log {Ignoring message received for CallCompletion (as decided by the user)}");
        *hSSEService = NULL;
    }
}


/************************************************************************
 * sseEvCCRequestInd
 * purpose: Callback function that is called when the stack receives a
 *          CallCompletion (H450.9) request.
 * input  : hSSEaCall       - The application's handle of the SSE call
 *          hSSECall        - The SSE handle of the SSE call
 *          hSSEaService    - SSE Application's handle of the service
 *          hSSEService     - SSE Service of CallCompletion
 *          isBusy          - TRUE if ccbsRequest was received
 *                            FALSE if ccnrRequest was received
 *          retainService   - TRUE if user A can retain the service, FALSE if it can't
 *          retentionType   - Retention type chosen by user A
 * output : retainService   - Application can set this value to TRUE or FALSE if it was
 *                            TRUE, meaning that user B chooses to retain the service or
 *                            not. If *retainService was given by SSE as FALSE, this
 *                            parameter is discarded and service retention is not supported.
 *          retainConnection- TRUE if application wants to retain the signaling connection
 *                            FALSE if it doesn't
 * return : ssDefaultAck    - To activate the service
 *          ssDefaultNack   - To reject the service
 *          ssWait          - If the user wants to answer later on
 *                            In this case, it should call sseCancelCallCompletion()
 *                            if it decided not to handle this service at all
 ************************************************************************/
int RVCALLCONV sseEvCCRequestInd(
        IN      HSSEAPPCALL     hSSEaCall,
        IN      HSSECALL        hSSECall,
        IN      HSSEAPPSERVICE  hSSEaService,
        IN      HSSESERVICE     hSSEService,
        IN      BOOL            isBusy,
        IN      ccRetentionType retentionType,
        INOUT   BOOL*           retainService,
        OUT     BOOL*           retainConnection)
{
    static const char* ccServiceStr[] = {"NR", "BS"};
    CallCompletionStruct* ccInfo;
    CallInfo* Call;
    Call = (CallInfo *)hSSEaCall;

    /* See if we should retain the service or not */
    *retainService = atoi(TclGetVariable("app(h450,retainServ)"));

    /* See if we'd like to retain the connection or not */
    if (retentionType == ccRetainConnection)
        *retainConnection = TRUE;
    else if (retentionType == ccDontRetainConnection)
        *retainConnection = FALSE;
    else
        *retainConnection = atoi(TclGetVariable("app(h450,retainConn)"));

    TclExecute("test:Log {Received CC%s Request Indication: retention: %d}",
               ccServiceStr[isBusy], retentionType);
    ccInfo = (CallCompletionStruct *)hSSEaService;
    ccInfo->connectionRetained = *retainConnection;

    TclExecute("after idle {Call:AddRemark %d Completion-B}", Call->counter);

    if ( (TclGetVariable("tmp(h450,defAns)"))[0] == '0' )
        return ssDefaultNack;

    return ssDefaultAck;
}


/************************************************************************
 * sseEvCCExecPossibleInd
 * purpose: Callback function that is called when User A receives an
 *          ExecPossible.inv message from User B on a CallCompletion service.
 *          This means that User A can now complete the call.
 * input  : hSSEaCall   - The application's handle of the SSE call
 *          hSSECall    - The SSE handle of the SSE call
 *          hSSEaService- SSE Application's handle of the service
 *          hSSEService - SSE Service of CallCompletion
 * output : isBusy      - TRUE if User A is currently busy
 *                        FALSE if service can be completed since User A isn't busy
 * return : Non-negative value on success. In this case, the user has to
 *          call sseCallCompletionRingout() if it indicated that the user isn't busy. If
 *          the user isn't busy, this will cause the stack to automatically send out
 *          ccSuspend.inv message for this service, and the user will have to call
 *          sseCallCompletionResume() when he's no longer busy.
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV sseEvCCExecPossibleInd(
        IN      HSSEAPPCALL     hSSEaCall,
        IN      HSSECALL        hSSECall,
        IN      HSSEAPPSERVICE  hSSEaService,
        IN      HSSESERVICE     hSSEService,
        OUT     BOOL*           isBusy)
{
    TclExecute("test:Log {Received ExecPossible for CallCompletion on service 0x%p}", hSSEaService);

    /* We only need to state if we're busy or not - the result doesn't matter in this case... */
    *isBusy = atoi(TclGetVariable("app(h450,compBusy)"));
    return ssDefaultAck;
}


/************************************************************************
 * sseEvCCStageInd
 * purpose: Callback function that is called for different stages of the
 *          call completion service. This function should be implemented by
 *          both User A and User B of this service, since some of the stages
 *          can occur in both sides.
 * input  : hSSEaCall   - The application's handle of the SSE call
 *                        Can be NULL if timeout
 *          hSSECall    - The SSE handle of the SSE call
 *          hSSEaService- SSE Application's handle of the service
 *          hSSEService - SSE Service of CallCompletion
 *          indication  - The indication fo the stage that we're in
 *                        If this is ccTimeOut, then hSSEaCall and hSSECall
 *                        may be NULL, if no signaling connection is retained
 *                        for this service
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int RVCALLCONV sseEvCCStageInd(
        IN      HSSEAPPCALL     hSSEaCall,
        IN      HSSECALL        hSSECall,
        IN      HSSEAPPSERVICE  hSSEaService,
        IN      HSSESERVICE     hSSEService,
        IN      ccIndication    indication)
{
    int ret = 0;

    TclExecute("test:Log {Received %s for CallCompletion on service 0x%p}",
               CCIndication2String(indication) , hSSEaService);

    switch (indication)
    {
        case ccRingout:
        {
            /* Check if we're busy - if we are, we should return an error message back
               to the SSE */
            if (atoi(TclGetVariable("app(h450,compBusy)")))
                ret = RVERROR;
            break;
        }

        case ccTimeOut:
        {
            /* Cancel the service on timeout */
            TclExecute("H450.callCompletionAction Cancel 0x%p", hSSEaService);
            break;
        }

        case ccCallDropped:
        {
            /* Call was dropped, but service exists. This means that we don't use
               connection retention on this service anymore, so just update the
               service information about it */
            CallCompletionStruct*   ccInfo = (CallCompletionStruct *)hSSEaService;
            ccInfo->connectionRetained = FALSE;
            break;
        }

        default:
            break;
    }
    return ret;
}



#endif /* USE_H450 */



/* Call Offer - H450.10
 ********************************/

/* Sends a CallOffer Setup message */
int H450_callOffer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int                   overrideCFB;
    char                 destAddr[256], *UU,ownCSstr[128];
    int                   waitingListFull;
    cmTransportAddress    ownCSaddr;
    CallInfo             *CurrentCall;
    HSSECALL             hssecall;
    strcpy(ownCSstr, "TA:");

    /* Make sure received the arguments */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if ((argv[2] == NULL) || (strlen(argv[2]) == 0)) return TCL_OK;
    /* converting from a string to useable variable */
    overrideCFB = (argv[2][0] != '0');

    /* Getting variables from the TCL */
    waitingListFull = atoi( TclGetVariable("app(h450,waitListFull)") );

    /* Create a new call handle */
    CurrentCall = CreateCallObject();
    if(CurrentCall == NULL)
        return TclExecute("test:Log {Error creating call}");

    cmGetLocalCallSignalAddress(hApp, &ownCSaddr);
    TransportAddress2String(&ownCSaddr, &ownCSstr[3]);
    convertToH450Address(argv[1], destAddr);

    /* send calling name if we have to */
    H450_sendNameID(CurrentCall, calling);
    /* prepre a callOfferRequest invoke apdu to be send in the setup*/
    hssecall = CurrentCall->hSSECall;
    sseCallOfferInit(hssecall, overrideCFB);

    /* Initiate FastStart Proceedures if needs be */
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
    {
        CurrentCall->isFastConnectCall = TRUE;
        if(atoi(TclGetVariable("app(options,autoAcceptFs)")))
            Call_InitFastStart(CurrentCall);
        else
        {
            if(CurrentCall->action == RTP_Replay)
            {
                /* create audio, video and data sessions */
                CurrentCall->rtpSessions[0] = RTP_TestOpen("FastStartAudio");
                TclExecute("set tmp(faststart,audioPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[0]));
                CurrentCall->rtpSessions[1] = RTP_TestOpen("FastStartVideo");
                TclExecute("set tmp(faststart,videoPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[1]));
                CurrentCall->rtpSessions[2] = RTP_TestOpen("FastStartData");
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
                CurrentCall->counter, CurrentCall,
                CurrentCall->counter, CurrentCall, destAddr);
        }
    }

    /* Get UUI */
    UU = TclGetVariable("app(options,userUser)");

    /* Create the call */
    if (cmCallMake(CurrentCall->hsCall,
        0, atoi(TclGetVariable("app(test,bw)")),
        destAddr, ownCSstr,
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
        /*cmCallClose(CurrentCall->hsCall);*/
        cmEvCallStateChanged((HAPPCALL)CurrentCall, CurrentCall->hsCall, cmCallStateIdle, 0);
    }

#endif /* USE_H450 */
    return TCL_OK;
}

int H450_remoteUserAlerting(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    int retVal;
    CallInfo* Call;

    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;

    if( (retVal = sseRemoteUserAlerting(Call->hSSECall)) < 0)
        return retVal;

#endif /* USE_H450 */

    return TCL_OK;
}


#ifdef USE_H450

/* there are three scenarios:
 * 1: the line is free - it should follow regular 323 protocol.
 * 2: the line is busy and the is no cfb service- start callWait service
 * 3: the line is busy, there is a cfb service but no cfbOverride massege
 *    had been sent - the standard callDiversion service should be implemented
 *                    and in addition to divertingLegInfo2 message, the rerouting
 *                    endpoint should send the callOfferRequest to the diverted-to
 *                    endpoint.
 * 4: the line is busy, there is a cfb service and a cfbOverride message had been
 *    sent - start callWaiting service.
 */

/* caled when call offer has been activated. inform user */
int RVCALLCONV sseEvCfbOverrideInd(IN HSSEAPPCALL hSSEaCallPri,
                                     IN HSSECALL hSSECallPri)
{
    CallInfo * Call = (CallInfo *) hSSEaCallPri;

    TclExecute("after idle {Call:AddRemark %d {Call Offer: Overide CFB indicated}}", Call->counter);
    return 0;
}

/* caled when call offer has been ended. inform user */
void RVCALLCONV sseEvRemoteUserAlertingInd(IN HSSEAPPCALL hSSEaCallPri,
                                           IN HSSECALL hSSECallPri)
{
    CallInfo * Call = (CallInfo *) hSSEaCallPri;

    TclExecute("after idle {Call:AddRemark %d {Remote User Alerting}}", Call->counter);
}


#endif /* USE_H450 */


/* Call Intrusion - H450.11
 ********************************/

/* Sends a Call Intrusion Setup message */
int H450_callIntrusion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef USE_H450
    char                 destAddr[256], *UU,ownCSstr[128];
    int                  i;
    char *               callIDstr, callID[16] = {0};
    cmTransportAddress   ownCSaddr;
    CallInfo             *CurrentCall;
    ciPrimitivesEnum     IntrusionType = (ciPrimitivesEnum)0;
    BOOL                 CreateCall = TRUE;
    strcpy(ownCSstr, "TA:");

    /* Make sure received the arguments */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    /* convert the argument from a string to useable variable */
    switch(argv[2][0])
    {
    case('f'):
        IntrusionType = ssCIForcedRelease;
        break;
    case('i'):
        if (argv[3] == NULL) return TCL_OK;
        if (strlen(argv[3])==0)
        {
            IntrusionType = ssCIRequest;
        }
        else
        {
            switch(argv[3][0])
            {
            case('f'):
                IntrusionType = ssCIForcedRelease;
                break;
            case('w'):
                IntrusionType = ssCIWOBRequest;
                break;
            case('i'):
                IntrusionType = ssCIIsolated;
                break;
            }
            CreateCall = FALSE;
        }
        break;
    case('s'):
        IntrusionType = ssCISilentMoitor;
        callIDstr = TclGetVariable("app(h450,intrCallID)");
        for(i=0; i<16; i++)
        {
            UINT32 tmpVal;
            sscanf(callIDstr+2*i, "%02X", &tmpVal);
            callID[i] = (char)tmpVal;
        }
        break;
    }

    if(!CreateCall)
    {
        CallInfo* ServCall;

        /* Get Call Handle */
        if ((argv[5] == NULL) || (strlen(argv[5]) == 0)) return TCL_OK;
        if (sscanf(argv[5], "%*d 0x%p:", &ServCall) != 1) return TCL_OK;

        /* prepre a call intrusion invoke apdu to be send in the setup*/
        sseCallIntrusion2(ServCall->hSSECall, IntrusionType);
        return TCL_OK;
    }
    if(argv[4][0] == '0') /* argv[4] is Use Waiting Call checkbox */
    {
        /* Create a new call handle */
        CurrentCall = CreateCallObject();
        if(CurrentCall == NULL)
            return TCL_ERROR;

        cmGetLocalCallSignalAddress(hApp, &ownCSaddr);
        TransportAddress2String(&ownCSaddr, &ownCSstr[3]);
        convertToH450Address(argv[1], destAddr);

        /* send calling name if we have to */
        H450_sendNameID(CurrentCall, calling);

        /* prepre a call intrusion invoke apdu to be send in the setup*/
        if (sseCallIntrusionInit(CurrentCall->hSSECall, IntrusionType, callID)<0)
            return TCL_ERROR;
    }
    else
    {
        if ((argv[5] == NULL) || (strlen(argv[5]) == 0)) return TCL_OK;
        if (sscanf(argv[5], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

        /* prepre a call intrusion invoke apdu to be send in the setup*/
        sseCallIntrusionInit(CurrentCall->hSSECall, IntrusionType, callID);
        return TCL_OK;
    }

    /* Initiate FastStart Proceedures if needs be */
    if(atoi(TclGetVariable("app(newcall,fastStart)")))
    {
        CurrentCall->isFastConnectCall = TRUE;
        if(atoi(TclGetVariable("app(options,autoAcceptFs)")))
            Call_InitFastStart(CurrentCall);
        else
        {
            if(CurrentCall->action == RTP_Replay)
            {
                /* create audio, video and data sessions */
                CurrentCall->rtpSessions[0] = RTP_TestOpen("FastStartAudio");
                TclExecute("set tmp(faststart,audioPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[0]));
                CurrentCall->rtpSessions[1] = RTP_TestOpen("FastStartVideo");
                TclExecute("set tmp(faststart,videoPort) %d", RTP_TestGetLocalPort(CurrentCall->rtpSessions[1]));
                CurrentCall->rtpSessions[2] = RTP_TestOpen("FastStartData");
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
                CurrentCall->counter, CurrentCall,
                CurrentCall->counter, CurrentCall, destAddr);
        }
    }

    /* Get UUI */
    UU = TclGetVariable("app(options,userUser)");

    /* Create the call */
    if (cmCallMake(CurrentCall->hsCall,
        0, atoi(TclGetVariable("app(test,bw)")),
        destAddr, ownCSstr,
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

#endif /* USE_H450 */
    return TCL_OK;
}

int H450_getCallID(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int len = 16, i;
    CallInfo * Call;
    char callID[16];
    char result[33];

    if (sscanf(argv[1], "%*d 0x%p:", &Call) != 1) return TCL_OK;
    cmCallGetParam(Call->hsCall, cmParamCallID, 0, &len, callID);
    for(i=0; i<16; i++)
        sprintf(result+2*i, "%02X", (unsigned char)callID[i]);
    result[32] = 0;
    Tcl_SetResult(interp, result, TCL_VOLATILE);

    return TCL_OK;
}


#ifdef USE_H450
int RVCALLCONV sseEvCIInitialRequestInd(IN  HSSEAPPCALL      hSSEaCall,
                                        IN  HSSECALL         call,
                                        IN  ciPrimitivesEnum opcode,
                                        IN  char*            callIdentifier,
                                        OUT int *            reason,
                                        OUT BOOL *           possible,
                                        OUT BOOL *           IsIsolated,
                                        OUT HSSECALL *       estCall)
{
    char * opStr, * callStr;
    CallInfo * appCall = (CallInfo *) hSSEaCall;
    CallInfo * intrCall;

    opStr = SSPrimitive2String((int)opcode);

    TclExecute("call:Log %d {Received call intrusion request of type %s}", appCall->counter, opStr);
    TclExecute("after idle {Call:AddRemark %d {%s}}", appCall->counter, opStr);

    /* RAN - have to do it only if the intrCalll is not connected*/
    /* Send CallProceeding - without it, the SSE won't be able to send a FACILITY
       message on this call. */
    cmCallSendCallProceeding(appCall->hsCall);

    *possible = atoi(TclGetVariable("app(h450,intrPossible)"));
    *IsIsolated = atoi(TclGetVariable("app(h450,intrIsIsolated)"));
    *reason = 0;
    if(opcode == ssCISilentMoitor)
    {
        char callID[33];
        int i;

        for(i=0; i<16; i++)
            sprintf(callID+2*i, "%02X", (unsigned char)callIdentifier[i]);
        callID[32] = 0;
        TclExecute("findCallID {%s}", callID);
    }
    else
        TclExecute("getConnectedCall");
    callStr = Tcl_GetStringResult(interp);
    if (opcode == ssCISilentMoitor && !(strcmp(callStr,"1")))
    {
        *possible = FALSE;
        return 0;
    }
    if (sscanf(callStr, "%*d 0x%p:", &intrCall) != 1) return -1;
    *estCall = intrCall->hSSECall;
    if( (opcode == ssCIRequest) && (!*IsIsolated) )
        TclExecute("$tmp(h450Tab).service.intr.callHandle configure -text 0x%p", intrCall);

    return 1;
}

int RVCALLCONV sseEvCIIsolatedInd(IN  HSSEAPPCALL hSSEaCall,
                                  IN  HSSECALL    call,
                                  OUT int *       parameter)
{
    CallInfo * appCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received call intrusion isolated indication}", appCall->counter);
    *parameter = -1;
    return ssDefaultAck;
}

int RVCALLCONV sseEvCIForcedReleaseInd(IN  HSSEAPPCALL hSSEaCall,
                                       IN  HSSECALL    call,
                                       OUT BOOL *      possible,
                                       OUT int *       parameter)
{
    CallInfo * appCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received call intrusion forced release indication}", appCall->counter);
    *possible = atoi(TclGetVariable("app(h450,intrPossible)"));
    *parameter = -1;
    return ssDefaultAck;
}

int RVCALLCONV sseEvCIWOBInd(IN  HSSEAPPCALL hSSEaCall,
                             IN  HSSECALL    call,
                             OUT BOOL *      possible,
                             OUT int *       parameter)
{
    CallInfo * appCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received call intrusion wait on busy indication}", appCall->counter);
    *possible = atoi(TclGetVariable("app(h450,intrPossible)"));
    *parameter = -1;
    return ssDefaultAck;
}
int RVCALLCONV sseEvCIMakeSilentInd(  IN HSSEAPPCALL            hSSEaCallEst,
                                      IN HSSECALL               callEst)
{
    CallInfo * appCall = (CallInfo *) hSSEaCallEst;
    cmCallAnswer(appCall->hsCall);    
    TclExecute("Call:AddRemark %d monitoring", appCall->counter);
    return ssDefaultAck;
}

int RVCALLCONV sseEvCINotificationInd(IN HSSEAPPCALL  hSSEaCall,
                                      IN HSSECALL     call)
{
    CallInfo * appCall = (CallInfo *) hSSEaCall;
    TclExecute("call:Log %d {Received call intrusion notification indication}", appCall->counter);
    return ssDefaultAck;
}

void RVCALLCONV sseEvCIRequestConfirm(IN  HSSEAPPCALL     hSSEaCallPri,
                                      IN  HSSECALL        hSSECallPri,
                                      IN  int             errCode,
                                      IN  int             ok)
{
    CallInfo * call = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received call intrusion Request confirm, errCode:%d, ok:%d}",
        call->counter, errCode, ok);
}

void RVCALLCONV sseEvCISilentMonitorConfirm(IN  HSSEAPPCALL     hSSEaCallPri,
                                            IN  HSSECALL        hSSECallPri,
                                            IN  int             errCode,
                                            IN  int             ok)
{
    CallInfo * call = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received call intrusion Silent Monitor confirm, errCode:%d, ok:%d}",
        call->counter, errCode, ok);
}

void RVCALLCONV sseEvCIWOBRequestConfirm(IN  HSSEAPPCALL     hSSEaCallPri,
                                         IN  HSSECALL        hSSECallPri,
                                         IN  int             errCode,
                                         IN  int             ok)
{
    CallInfo * call = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received call intrusion WOB Request confirm, errCode:%d, ok:%d}",
        call->counter, errCode, ok);
}

void RVCALLCONV sseEvCIForcedReleaseConfirm(IN  HSSEAPPCALL     hSSEaCallPri,
                                            IN  HSSECALL        hSSECallPri,
                                            IN  int             errCode,
                                            IN  int             ok)
{
    CallInfo * call = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received call intrusion Forced Release confirm, errCode:%d, ok:%d}",
        call->counter, errCode, ok);
}

void RVCALLCONV sseEvCIIsolatedConfirm(IN  HSSEAPPCALL     hSSEaCallPri,
                                       IN  HSSECALL        hSSECallPri,
                                       IN  int             errCode,
                                       IN  int             ok)
{
    CallInfo * call = (CallInfo *) hSSEaCallPri;
    TclExecute("call:Log %d {Received call intrusion Isolated confirm, errCode:%d, ok:%d}",
        call->counter, errCode, ok);
}

int RVCALLCONV sseEvCIFindService(IN  HSSEAPPCALL  hSSEaCall,
                                  IN  HSSECALL     call,
                                  OUT HSSECALL*    oldHSSECall)
{
    char * string;
    CallInfo * OldCall, * Call = (CallInfo *) hSSEaCall;
    TclExecute("$tmp(h450Tab).service.intr.callHandle cget -text");
    string = Tcl_GetStringResult(interp);
    if (sscanf(string, "0x%p:", &OldCall) != 1) return TCL_OK;
    *oldHSSECall = OldCall->hSSECall;
    TclExecute("call:Log %d {Received call intrusion conferance join to call %d}", Call->counter, OldCall->counter);
    TclExecute("$tmp(h450Tab).service.intr.callHandle configure -text \"\"");
    return 1;
}


#endif  /* USE_H450 */





/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/

/********************************************************************************************
 * H450_Init
 * purpose : This function initializes the H450 sepplementay services. does nothing if
 *           USE_H450 is not defined.
 * input   : name - name of the configuration file
 *           lphApp - pointer to the application handle
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int H450_init(char * name, LPHAPP lphApp)
{
    int retVal = 0;
#ifdef USE_H450
    cmRASTransport tr;
    char str[32];
    SSSECALLEVENT sseCallEvent;

    /* Initializing the H450 service */
    if( (retVal = sseInit(name,&hSSEApp,NULL,*lphApp,100)) < 0)
        return retVal;
    hSSApp=sseGetSSHandle(hSSEApp);

    /* Setting the Callback functions */
    memset (&sseCallEvent,0,sizeof(sseCallEvent));
    sseCallEvent.sseEvRemoteHoldInd            = sseEvRemoteHoldInd;
    sseCallEvent.sseEvRemoteHoldConfirm        = sseEvRemoteHoldConfirm;
    sseCallEvent.sseEvRemoteHoldRetrieveInd    = sseEvRemoteHoldRetrieveInd;
    sseCallEvent.sseEvRemoteRetrieveConfirm    = sseEvRemoteRetrieveConfirm;
    sseCallEvent.sseEvNearHoldInd              = sseEvNearHoldInd;
    sseCallEvent.sseEvNearHoldRetrieveInd      = sseEvNearHoldRetrieveInd;
    sseCallEvent.sseEvWaitInd                  = sseEvWaitInd;
    sseCallEvent.sseEvWaitTimerExpired         = sseEvWaitTimerExpired;
    sseCallEvent.sseEvNIAlertingNameInd        = sseEvNIAlertingNameInd;
    sseCallEvent.sseEvNIBusyNameInd            = sseEvNIBusyNameInd;
    sseCallEvent.sseEvNICallingNameInd         = sseEvNICallingNameInd;
    sseCallEvent.sseEvNIConnectedNameInd       = sseEvNIConnectedNameInd;
    sseCallEvent.sseEvCallTransfer             = sseEvCallTransfer;
    sseCallEvent.sseEvCallReroute              = sseEvCallReroute;
    sseCallEvent.sseEvForwardActivated         = sseEvForwardActivated;
    sseCallEvent.sseEvForwardDeactivated       = sseEvForwardDeactivated;
    sseCallEvent.sseEvMWIActivateConfirm       = sseEvMWIActivateConfirm;
    sseCallEvent.sseEvMWIActivateInd           = sseEvMWIActivateInd;
    sseCallEvent.sseEvMWIDeactivateConfirm     = sseEvMWIDeactivateConfirm;
    sseCallEvent.sseEvMWIDeactivateInd         = sseEvMWIDeactivateInd;
    sseCallEvent.sseEvMWIInterrogateConfirm    = sseEvMWIInterrogateConfirm;
    sseCallEvent.sseEvMWIInterrogateInd        = sseEvMWIInterrogateInd;
    sseCallEvent.sseEvRemoteUserAlertingInd    = sseEvRemoteUserAlertingInd;
    sseCallEvent.sseEvCCFindService            = sseEvCCFindService;
    sseCallEvent.sseEvCCRequestInd             = sseEvCCRequestInd;
    sseCallEvent.sseEvCCExecPossibleInd        = sseEvCCExecPossibleInd;
    sseCallEvent.sseEvCCStageInd               = sseEvCCStageInd;
    sseCallEvent.sseEvServiceHandleCreated     = sseEvServiceHandleCreated;
    sseCallEvent.sseEvServiceHandleIdle        = sseEvServiceHandleIdle;
    sseCallEvent.sseEvCIInitialRequestInd      = sseEvCIInitialRequestInd;
    sseCallEvent.sseEvCIForcedReleaseConfirm   = sseEvCIForcedReleaseConfirm;
    sseCallEvent.sseEvCIForcedReleaseInd       = sseEvCIForcedReleaseInd;
    sseCallEvent.sseEvCIIsolatedConfirm        = sseEvCIIsolatedConfirm;
    sseCallEvent.sseEvCIIsolatedInd            = sseEvCIIsolatedInd;
    sseCallEvent.sseEvCIMakeSilentInd          = sseEvCIMakeSilentInd;
    sseCallEvent.sseEvCINotificationInd        = sseEvCINotificationInd;
    sseCallEvent.sseEvCIRequestConfirm         = sseEvCIRequestConfirm;
    sseCallEvent.sseEvCISilentMonitorConfirm   = sseEvCISilentMonitorConfirm;
    sseCallEvent.sseEvCIWOBInd                 = sseEvCIWOBInd;
    sseCallEvent.sseEvCIWOBRequestConfirm      = sseEvCIWOBRequestConfirm;
    sseCallEvent.sseEvCIFindService            = sseEvCIFindService;
    sseCallEvent.sseEvCfbOverrideInd           = sseEvCfbOverrideInd;
    sseCallEvent.sseEvCParkRequestInd          = sseEvCParkRequestInd;
    sseCallEvent.sseEvCPGroupIndication        = sseEvCPGroupIndication;
    sseCallEvent.sseEvCPGroupIndicationConfirm = sseEvCPGroupIndicationConfirm;
    sseCallEvent.sseEvCPPickRequConfirm        = sseEvCPPickRequConfirm;
    sseCallEvent.sseEvCPPickupConfirm          = sseEvCPPickupConfirm;
    sseCallEvent.sseEvCPRequestConfirm         = sseEvCPRequestConfirm;
    sseCallEvent.sseEvCPSetupInd               = sseEvCPSetupInd;
    sseCallEvent.sseEvCPSetupConfirm           = sseEvCPSetupConfirm;
    sseCallEvent.sseEvCPickExeInd              = sseEvCPickExeInd;
    sseCallEvent.sseEvCPickupInd               = sseEvCPickupInd;
    sseCallEvent.sseEvCPickupRequestInd        = sseEvCPickupRequestInd;
    sseCallEvent.sseEvCPickupFindCall          = sseEvCPickupFindCall;
    sseCallEvent.sseEvServiceHandleCreated     = sseEvServiceHandleCreated;
    sseCallEvent.sseEvServiceHandleIdle        = sseEvServiceHandleIdle;

    if( (retVal = sseSetCallEventHandler(hSSEApp,&sseCallEvent,sizeof(sseCallEvent))) < 0)
    {
        sseEnd(hSSEApp);
        return retVal;
    }

    h450Initialized = TRUE;

    /* Setting the default local address */
    cmGetLocalCallSignalAddress(*lphApp,&tr);
    convertAddrToStr(&tr, str);

    retVal = sseSetDefaultAddress(hSSEApp, str);
#endif /* USE_H450 */
    return retVal;
}

/********************************************************************************************
 * H450_end
 * purpose : This function terminates the H450 sepplementay services. does nothing if
 *           USE_H450 is not defined.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void H450_end(void)
{
#ifdef USE_H450
    if (h450Initialized)
    {
        sseEnd(hSSEApp);
        h450Initialized = FALSE;
    }
#endif /* USE_H450 */
}


/********************************************************************************************
 * H450_IsInitialized
 * purpose : This procedure checks to see if H450 can be used.
 * input   : none
 * output  : none
 * return  : TRUE if H450 can be used
 *           FALSE otherwise
 ********************************************************************************************/
BOOL H450_IsInitialized(void)
{
    return h450Initialized;
}


/********************************************************************************************
 * H450_status
 * purpose : This function displays the current status of H450 module. It is used by the
 *           status window of the test application. does nothing if USE_H450 is not defined.
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void H450_status(void)
{
#ifdef USE_H450
    HPVT hVal;

    hVal = ssGetValTree(sseGetSSHandle(hSSEApp));
    TclExecute("status:InsertLine {H450} {PVT Nodes} %d %d %d",
        pvtCurSize(hVal), pvtMaxUsage(hVal), pvtMaxSize(hVal));

#endif  /* USE_H450 */
}


/********************************************************************************************
 * cmEvCallH450SupplServ
 * purpose : callback for received H450 messages, Notifies the H450 state machine
 *           that an H450 message has been received does nothing if USE_H450 is not defined.
 * input   : haCall - aplication handle to the call
 *           hsCall - stack handle of the call
 *           msgType - type of the received Q931 message
 *           nodeId - root node of the message
 *           size - size of the message
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int RVCALLCONV cmEvCallH450SupplServ(IN HAPPCALL haCall,
                                     IN HCALL hsCall,
                                     IN cmCallQ931MsgType msgType,
                                     IN int nodeId,
                                     IN int size)
{
    int retVal = 0;
#ifdef USE_H450
    CallInfo * Call = (CallInfo *) haCall;
    TclExecute("call:Log %d {<H450 message received>}", Call->counter);
    if(Call->hSSECall)
        retVal = sseCallH450SupplServ(Call->hSSECall, msgType, nodeId, size);
    else
        TclExecute("call:Log %d {<NO H450 OBJECT - H450 MESSAGE NOT PROCESSED>}", Call->counter);
#endif /* USE_H450 */

    return retVal;
}

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
                          IN UINT32 stateMode)
{
    int retVal = 0;
#ifdef USE_H450
    if (Call->hSSECall != NULL)
        retVal = sseCallStateChanged(Call->hSSECall, state, stateMode);
#endif /* USE_H450 */
    return retVal;
}

/********************************************************************************************
 * H450_CreateCall
 * purpose : Creates the H450 call object for the call and implements services. does nothing
 *           if USE_H450 is not defined.
 * input   : Call - pointer to information about the call
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int H450_CreateCall(CallInfo * Call)
{
#ifdef USE_H450
    if (sseCreateCall(hSSEApp, &Call->hSSECall, (HSSEAPPCALL) Call, Call->hsCall) < 0)
    {
        Call->hSSECall = NULL;
        return RVERROR;
    }
    else
    {
        sseCallImplementForward(Call->hSSECall);
        sseCallImplementHold(Call->hSSECall);
        sseCallImplementMWIServed(Call->hSSECall);
        sseCallImplementMWIMsgCtr(Call->hSSECall);
        sseCallImplementNI(Call->hSSECall);
        sseCallImplementTransfer(Call->hSSECall,0);
        sseCallImplementCallOffer(Call->hSSECall);
        sseCallImplementCompletion(Call->hSSECall);
        sseCallImplementIntrusion(Call->hSSECall);
        sseCallImplementParkPickup(Call->hSSECall);
        if( (TclGetVariable("app(h450,disableWait)"))[0] == '0' )
            sseCallImplementWait(Call->hSSECall);
    }
#endif /* USE_H450 */
    return 0;
}


/********************************************************************************************
 * H450_CloseCall
 * purpose : Destroys the H450 call object for the call. does nothing
 *           if USE_H450 is not defined.
 * input   : Call - pointer to information about the call
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
void H450_CloseCall(CallInfo * Call)
{
#ifdef USE_H450
    if(Call->hSSECall)
        sseCallClose(Call->hSSECall);
#endif /* USE_H450 */
}

#ifdef __cplusplus
}
#endif
