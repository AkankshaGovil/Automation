/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_wrapper.c
 *
 * This file contains all the functions that are used for
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


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <ctype.h>
#include "q931asn1.h"
#include "TAP_h450.h"
#include "TAP_utils.h"
#include "TAP_general.h"
#include "TAP_call.h"
#include "TAP_channel.h"
#include "TAP_wrapper.h"


BOOL     LogWrappers = FALSE;


/********************************************************************************************
*
*  General Functions
*
*********************************************************************************************/


/********************************************************************************************
 * WrapperEnter
 * purpose : Log wrapper function on entry
 * input   : interp     - Interperter to use
 *           usage      - The right parameters
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperEnter(Tcl_Interp* interp, int argc, char *argv[])
{
    if (LogWrappers)
    {
        char buf[4096];
        char* ptr = buf+7;
        int i;
        strcpy(buf, "Enter: ");

        for (i = 0; i < argc; i++)
        {
            strcpy(ptr, argv[i]);
            ptr += strlen(ptr);
            *ptr = ' ';
            ptr++;
        }
        *ptr = '\0';

        cmLogMessage(hApp, buf);
    }

    Tcl_SetResult(interp, NULL, TCL_STATIC);
    return TCL_OK;
}


/********************************************************************************************
 * WrapperExit
 * purpose : Log wrapper function on exit
 * input   : interp     - Interperter to use
 *           argc       - Number of arguments
 *           argv       - Argument values
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperExit(Tcl_Interp* interp, int argc, char *argv[])
{
    if (LogWrappers)
    {
        char buf[4096];
        char* ptr = buf+6;
        int i;
        strcpy(buf, "Exit: ");

        for (i = 0; i < argc; i++)
        {
            strcpy(ptr, argv[i]);
            ptr += strlen(ptr);
            *ptr = ' ';
            ptr++;
        }
        *ptr = '\0';

        if (strlen(Tcl_GetStringResult(interp)) != 0)
            sprintf(ptr, " (%s)", Tcl_GetStringResult(interp));
        cmLogMessage(hApp, buf);
    }

    return TCL_OK;
}


/********************************************************************************************
 * WrapperBadParams
 * purpose : Return an error result due to bad function parameters
 * input   : interp     - Interperter to use
 *           usage      - The right parameters
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperBadParams(Tcl_Interp* interp, const char* usage)
{
    char buf[1024];
    sprintf(buf, "Bad parameters given. Usage: %s", usage);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_ERROR;
}


/********************************************************************************************
 * WrapperReply
 * purpose : Create and use the return result from the API and sent it back to TCL
 * input   : interp     - Interperter to use
 *           status     - Return status got from API
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperReply(Tcl_Interp* interp, int status, int argc, char *argv[])
{
    char buf[4096];
    char* ptr = buf;
    int i;
    Tcl_CmdInfo info;

    if (status >= 0)
        return TCL_OK;

    if (Tcl_GetCommandInfo(interp, (char *)"cb:app:Error", &info) == 0)
    {
        sprintf(buf, "Error executing %s - %s",
                argv[0], Status2String(status));
        /* Error command doesn't exist - we can throw an exception */
        Tcl_SetResult(interp, buf, TCL_VOLATILE);
        return TCL_ERROR;
    }

    /* We have to call the error function */
    for (i = 0; i < argc; i++)
    {
        strcpy(ptr, argv[i]);
        ptr += strlen(ptr);
        *ptr = ' ';
        ptr++;
    }
    *ptr = '\0';

    TclExecute("cb:app:Error {%s} {%s}",
               buf,
               Status2String(status));

    Tcl_SetResult(interp, (char *)"", TCL_STATIC);
    return TCL_OK;
}


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
int WrapperFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    Tcl_CmdProc*    cmd;
    int             result = TCL_ERROR;

    WrapperEnter(interp, argc, argv);

    cmd = (Tcl_CmdProc *)clientData;
    if (cmd != NULL)
        result = cmd((ClientData)hApp, interp, argc, argv);

    WrapperExit(interp, argc, argv);

    return result;
}


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
int CallbackFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char* spacer;
    int result;
    Tcl_CmdInfo info;

    if (argc != 2)
    {
        PutError(argv[0], "Callback wrapper called with too many arguments");
    }

    spacer = strchr(argv[1], ' ');

    if (spacer != NULL) *spacer = '\0';
    result = Tcl_GetCommandInfo(interp, argv[1], &info);
    *spacer = ' ';

    if (result == 0)
    {
        /* Callback command doesn't exist - skip it */
        return TCL_OK;
    }

    if (LogWrappers);
/*        LOG_Print(wrapperLog, RV_INFO,
                  (wrapperLog, RV_INFO, "Enter: %s", argv[1]));*/

    result = TclExecute(argv[1]);
    if (result == TCL_OK)
        Tcl_SetResult(interp, NULL, TCL_STATIC);

    if (LogWrappers);
/*        LOG_Print(wrapperLog, RV_INFO,
                  (wrapperLog, RV_INFO, "Exit: %s", argv[1]));*/

    return result;
}






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
int api_cm_GetVersion(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char* ver;

    ver = cmGetVersion();
    if (ver != NULL)
        Tcl_SetResult(interp, ver, TCL_VOLATILE);

    return WrapperReply(interp, 0, argc, argv);
}

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
int api_cm_Stop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;

    status = cmStop(hApp);

    if (status >= 0)
        TclExecute("test:SetStackStatus Stopped");

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_Start(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;

    status = cmStart(hApp);

    if (status >= 0)
        TclExecute("test:SetStackStatus Running");

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_LogMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:LogMessage <msg>");

    cmLogMessage(hApp, argv[1]);

    return WrapperReply(interp, 0, argc, argv);
}

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
int api_cm_Encode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    int     encBytes, status;
    BYTE    buffer[2048], result[4096];

    if (argc != 3)
        return WrapperBadParams(interp, "api:cm:Encode <valTree> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = atoi(argv[2]);

    if (nodeId >= 0)
    {
        status = cmEmEncode(hVal, nodeId, buffer, sizeof(buffer), &encBytes);
        if (status >= 0)
        {
            Buffer2String(buffer, (char*)result, encBytes);
            Tcl_SetResult(interp, (char*)result, TCL_VOLATILE);
        }
    }
    else
        status = nodeId;

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_Decode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    int     status, len;
    BYTE    buffer[2048];

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:Decode <valTree> <nodeId> <buffer>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = atoi(argv[2]);

    if (nodeId >= 0)
    {
        len = String2Buffer(argv[3], buffer, sizeof(buffer));
        status = cmEmDecode(hVal, nodeId, buffer, len, &len);
    }
    else
        status = nodeId;

    return WrapperReply(interp, status, argc, argv);
}


/********************************************************************************************
*
* Call api
*
*********************************************************************************************/



/********************************************************************************************
 * api_cm_CallNew
 * purpose : wrapping the original CM api function - CallNew
 * syntax  : api:cm:CallNew
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallNew(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{

    CallInfo    *Call;
    char        returnedHandle[20];

    if (argc != 1)
        return WrapperBadParams(interp, "api:cm:CallNew");

    /* Create a new call */
    Call = CreateCallObject();
    if (Call == NULL)
        return WrapperReply(interp, RVERROR, argc, argv);

    Call->scriptCall = TRUE;
    ColorCall(Call);

    sprintf(returnedHandle, "0x%p", Call);

    if (LogWrappers)
    {
        char buf[100];
        sprintf(buf, "Call: app=0x%p ==> cm=0x%p", Call, Call->hsCall);
        cmLogMessage(hApp, buf);
    }

    Tcl_SetResult(interp, returnedHandle, TCL_VOLATILE);
    return WrapperReply(interp, 0, argc, argv);
}


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
int api_cm_CallDial(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* Call;
    int       retVal;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallDial <callHandle>");

    sscanf(argv[1], "0x%p", &Call);

    retVal = cmCallDial(Call->hsCall);

    return WrapperReply(interp, retVal, argc, argv);
}


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
int api_cm_CallProceeding(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* Call;
    int       retVal;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallProceeding <callHandle>");

    sscanf(argv[1], "0x%p", &Call);

    retVal = cmCallSendCallProceeding(Call->hsCall);

    return WrapperReply(interp, retVal, argc, argv);
}

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
int api_cm_CallAccept(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* CallHandle;
    int retVal;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallAccept <callHandle> ");

    sscanf(argv[1], "0x%p", &CallHandle);

    retVal = cmCallAccept(CallHandle->hsCall);

    return WrapperReply(interp, retVal, argc, argv);
}


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
int api_cm_CallAnswer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* CallHandle;
    int       retVal;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallAnswer <callHandle>");

    sscanf(argv[1], "0x%p", &CallHandle);

    retVal = cmCallAnswer(CallHandle->hsCall);

    return WrapperReply(interp, retVal, argc, argv);
}


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
int api_cm_CallClose(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    CallInfo* CallHandle;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallClose <callHandle>");

    sscanf(argv[1], "0x%p", &CallHandle);
    H450_CloseCall(CallHandle);
    status = cmCallClose(CallHandle->hsCall);
    CallHandle->hsCall = NULL;

    /* Free our call handle */
    AppFree(CallHandle);

    return WrapperReply(interp, status, argc, argv);
}




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
int api_cm_CallDrop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo    *CallHandle;
    int         retVal;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:CallDrop <callHandle> ");

    if(sscanf(argv[1], "0x%p", &CallHandle) != 1) return TCL_OK;

    retVal = cmCallDrop(CallHandle->hsCall);

    return WrapperReply(interp, retVal, argc, argv);
}


/********************************************************************************************
 * api_cm_CallSetParam
 * purpose : wrapping the original CM api function - CallSetParam
 * syntax  : api:cm:CallSetParam <handle> <RV323_CM_CallParam> <str>
 *         : <handle>         - handle of the call
 *         : <RV323_CM_CallParam>    - determine which call param
 *         : <RV_BOOL>            - for boolean parameters
 *         : <str>                - display or UserUser or TransportAddress or
 *                                      RV323_CM_CallParamCallState
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallSetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmCallParam         CallParam;
    int                 intValue;
    CallInfo*           CallHandle;
    int                 status = 0;
    cmTransportAddress  TransportAddress;
    cmAlias             Alias;

    if (argc < 4)
        return WrapperBadParams(interp, "api:cm:CallSetParam <callHandle> <callParam> <value> [<index> <isNewAlias>]");

    sscanf(argv[1],"0x%p",&CallHandle);
    CallParam = String2CMCallParam(argv[2]);

    switch(CallParam)
    {
        case cmParamEarlyH245:
        case cmParamH245Tunneling:
        case cmParamH245Parallel:
        case cmParamCanOverlapSending:
        case cmParamSetupCanOverlapSending:
        case cmParamEstablishH245:
        case cmParamRate:
        case cmParamIsMultiplexed:
        case cmParamSetupSendingComplete:
            {
                intValue = atoi(argv[3]);
                status = cmCallSetParam(CallHandle->hsCall,CallParam,0,intValue,NULL);
                break;
            }
        case cmParamDisplay:
        case cmParamUserUser:
            {
                status = cmCallSetParam(CallHandle->hsCall,
                                        CallParam,
                                        0,
                                        strlen(argv[3]),
                                        argv[3]);
                break;
            }
        case cmParamDestCallSignalAddress:
        case cmParamDestinationIpAddress:
        case cmParamDestinationAnnexEAddress:
            {
                /* building Address */
                String2TransportAddress(argv[3], &TransportAddress);

                status = cmCallSetParam(CallHandle->hsCall, CallParam, 0,
                    sizeof(cmTransportAddress), (char *) &TransportAddress);

                break;
            }
        case cmParamDestinationAddress:
        case cmParamSourceAddress:
        case cmParamConnectedAddress:
            {
                int index;
                if (argc != 5)
                    return WrapperBadParams(interp, "api:cm:CallSetParam <callHandle> <callParam> <value> <index>");

                Alias.string = NULL;
                String2Alias(argv[3],&Alias);
                index = atoi(argv[4]);
                status = cmCallSetParam(CallHandle->hsCall, CallParam, index,
                    sizeof(cmAlias), (char *) &Alias);
                if(Alias.string) AppFree(Alias.string);
                break;
            }
        case cmParamH245Stage:
            {
                status = cmCallSetParam(CallHandle->hsCall, CallParam, 0,
                    String2CMH245Stage(argv[3]), NULL);
                break;
            }
        case cmParamAnnexE:
            {
                status = cmCallSetParam(CallHandle->hsCall, CallParam, 0,
                    String2CMAnnexEUsageMode(argv[3]), NULL);
                break;
            }
        default: break;
    }

    /* TODO : add more cases to the switch */
    return WrapperReply(interp, status, argc, argv);
}



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
int api_cm_CallGetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int                 status = RVERROR;
    int                 intValue;
    CallInfo*           CallHandle;
    cmCallParam         CallParam;
    char                *returnedValue;
    char                data[1024];

    if ((argc < 3) || (argc > 4))
       return WrapperBadParams(interp, "api:cm:CallGetParam <callHandle> <callParam> [<index>]");

    if (sscanf(argv[1],"0x%p",&CallHandle) != 1) return RVERROR;

    CallParam = String2CMCallParam(argv[2]);
    returnedValue = data;

    switch(CallParam)
    {
    case cmParamEarlyH245:
    case cmParamH245Tunneling:
    case cmParamH245Parallel:
    case cmParamCanOverlapSending:
    case cmParamSetupCanOverlapSending:
    case cmParamEstablishH245:
    case cmParamRate:
    case cmParamIsMultiplexed:
    case cmParamSetupSendingComplete:
        {
            status = cmCallGetParam(CallHandle->hsCall, CallParam, 0, &intValue, (char *) NULL);
            sprintf(returnedValue, "%d", (int)intValue);
            break;
        }
    case cmParamDisplay:
    case cmParamUserUser:
        {
            intValue = sizeof(data);
            status = cmCallGetParam(CallHandle->hsCall, CallParam, 0, &intValue, data);
            break;
        }
    case cmParamDestCallSignalAddress:
    case cmParamDestinationIpAddress:
    case cmParamDestinationAnnexEAddress:
        {
            cmTransportAddress addr;
            intValue = sizeof(addr);
            status = cmCallGetParam(CallHandle->hsCall, CallParam, 0, &intValue, (char *) &addr);
            /* building Address */
            TransportAddress2String(&addr, data);
            break;
        }
    case cmParamDestinationAddress:
    case cmParamSourceAddress:
    case cmParamConnectedAddress:
        {
            int index;
            cmAlias alias;
            char string[128];

            if (argc != 4)
                return WrapperBadParams(interp, "api:cm:CallSetParam <callHandle> <callParam> <index>");

            index = atoi(argv[3]);
            alias.string = string;
            intValue = sizeof(alias);
            status = cmCallGetParam(CallHandle->hsCall, CallParam, index, &intValue, (char *) &alias);
            Alias2String(&alias, data);
            break;
        }
    case cmParamH245Stage:
        {
            status = cmCallGetParam(CallHandle->hsCall, CallParam, 0, &intValue, (char *) NULL);
            returnedValue = CMH245Stage2String((cmH245Stage) intValue);
            break;
        }
    case cmParamAnnexE:
        {
            status = cmCallGetParam(CallHandle->hsCall, CallParam, 0, &intValue, (char *) NULL);
            returnedValue = CMAnnexEUsageMode2String((cmAnnexEUsageMode) intValue);
            break;
        }
    default: break;
    }

    /* todo: add some cases */
    if (status >= 0)
        Tcl_SetResult(interp, returnedValue, TCL_VOLATILE);
    return WrapperReply(interp, status, argc, argv);
}



/***********************************************************************************
 * api_cm_CallForward
 * purpose: wrapping the original CM api function - cmCallForward
 * syntax : api:cm:CallForward <handle> <destAddress>
 *        : <handle>      - handle of the call
 *        : <destAddress> - string of the destination adress
 * input  : clientData - used for creating new command in tcl
 *          interp - interpreter for tcl commands
 *          argc - number of parameters entered to the new command
 *          argv - the parameters entered to the tcl command
 * output : none
 * return : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ***********************************************************************************/
int api_cm_CallForward(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    CallInfo* CallHandle;

    sscanf(argv[1], "0x%p", &CallHandle);

    status = cmCallForward(CallHandle->hsCall,argv[2]);
    return WrapperReply(interp, status, argc, argv);
}



/***********************************************************************************
 * api_cm_CallStatusInquiry
 * purpose: wrapping the original CM api function - CallStatusInquiry
 * syntax : api:cm:CallStatusInquiry <handle> <DisplayInfo>
 *        : <handle>         - handle of the call
 *        : <DisplayInfo>    - Display Info string
 * input  : clientData - used for creating new command in tcl
 *          interp      - interpreter for tcl commands
 *          argc - number of parameters entered to the new command
 *          argv - the parameters entered to the tcl command
 * output : none
 * return : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ***********************************************************************************/
int api_cm_CallStatusInquiry(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*   CallHandle;
    int         status;

    sscanf(argv[1], "0x%p", &CallHandle);

    status = cmCallStatusEnquiry(CallHandle->hsCall,(UINT8 *)argv[2]);

    return WrapperReply(interp, status, argc, argv);
}


/********************************************************************************************
 * api_cm_CallConnectControl
 * purpose : wrapping the original CM api function - CallConnectControl
 * syntax  : api:cm:CallConnectControl <handle>
 *           <handle>                  - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_CallConnectControl(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int       status;
    CallInfo* CallHandle;

    sscanf(argv[1], "0x%p", &CallHandle);

    status = cmCallConnectControl(CallHandle->hsCall);
    return WrapperReply(interp, status, argc, argv);
}


/********************************************************************************************
*
* H245 api
*
*********************************************************************************************/

/********************************************************************************************
* api_cm_H245DeleteCapabilityMessage
* purpose : wrapping the original CM api function - cmFreeCapability
* syntax  : api:cm:H245DeleteCapabilityMessage <callHandle>
*           <callHandle>   - app handle of the call
* input   : clientData - used for creating new command in tcl
*           interp - interpreter for tcl commands
*           argc - number of parameters entered to the new command
*           argv - the parameters entered to the tcl command
* output  : none
* return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
********************************************************************************************/
int api_cm_H245DeleteCapabilityMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int       status;
    CallInfo* CallHandle;

    sscanf(argv[1], "0x%p", &CallHandle);

    status = cmFreeCapability(CallHandle->hsCall);
    return WrapperReply(interp, status, argc, argv);
}


/********************************************************************************************
*
* Channel api
*
*********************************************************************************************/




/********************************************************************************************
 * api_cm_ChannelNew
 * purpose : wrapping the original CM api function - ChannelCreate
 * syntax  : api:cm:ChannelNew <CallHandle>
 *           <CallHandle>         - handle of the call
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelNew(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int                    status;
    ChannelInfo*           CurrentChannel;
    CallInfo*              CallHandle;
    char                   returnedHandle[20];


    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:ChannelNew <callHandle> ");

    sscanf(argv[1], "0x%p", &CallHandle);

    CurrentChannel = ChannelCreate(NULL, TRUE, TRUE, CallHandle);

    /* the APPHChannel parameter is the application handle of the call */
    status = cmChannelNew(CallHandle->hsCall, (HAPPCHAN) CurrentChannel, (LPHCHAN)&CurrentChannel->hChan);

    if (status >= 0)
    {
#if 0
        LOG_Print(wrapperLog, RV_INFO,
                  (wrapperLog, RV_INFO,
                  "Channel: app=0x%p ==> cm=0x%p",
                  CurrentChannel,
                  CurrentChannel->handle));
#endif
        sprintf(returnedHandle, "0x%p", CurrentChannel);
        Tcl_SetResult(interp, returnedHandle, TCL_VOLATILE);
    }
    else
    {
        FreeChannel(CurrentChannel);
        Tcl_SetResult(interp, "", TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}



/********************************************************************************************
 * api_cm_ChannelOpen
 * purpose : wrapping the original CM api function - ChannelOpen
 * syntax  : api:cm:ChannelOpen <CallHandle> <ChannelHandle>
 *                <MimmicedCahnnel> <dataType>
 *           <CallHandle>     - application handle of the Call
 *           <ChannelHandle>  - app handle of the channel
 *           <MimmicedCahnnel> - app handle of the channle mimmiced
 *           <MediaType> - Media type of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelOpen(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status = 0;
    CallInfo *    CallHandle;
    ChannelInfo * CurrentChannel;
    ChannelInfo * MimicChannel;
    char *        dataType;
    HCHAN         SourceChan;

    if (argc < 5)
        return WrapperBadParams(interp, "api:cm:ChannelOpen <callHandle> <ChannelHandle> <MimmicedChannel> <dataType>");

    sscanf(argv[1], "0x%p", &CallHandle);
    sscanf(argv[2], "0x%p", &CurrentChannel);
    sscanf(argv[3], "0x%p", &MimicChannel);

    dataType = argv[4];


    if (argv[3][0] == 0)
    {
        /* Open an RTP session for this channel */
        CurrentChannel->rtpSession = RTP_TestOpen("testChannel");
        RTP_TestSetAction(CurrentChannel->rtpSession, CallHandle->action);
        SourceChan = NULL;
    }
    else
    {
        /* Channel is mimiced */
        CurrentChannel->rtpSession = MimicChannel->rtpSession;
        SourceChan = MimicChannel->hChan;
        RTP_TestOpenSecondChannel(CurrentChannel->rtpSession);
    }
    /* set RTCP address  and open the channel*/
    cmChannelSetRTCPAddress(CurrentChannel->hChan, 0, (UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession) + 1));
    status = cmChannelOpen(CurrentChannel->hChan, dataType, SourceChan, NULL, 0);

    if (status >= 0)
    {
        /* set data type */
        strcpy(CurrentChannel->dataType, dataType);
    }
    /* We do nothing if cmChannelOpen() fails since such a failure also changes the state of
       the channel to disconnected and idle internally. */

    return WrapperReply(interp, status, argc, argv);
}



/********************************************************************************************
 * api_cm_ChannelAnswer
 * purpose : wrapping the original CM api function - ChannelAnswer
 * syntax  : api:cm:ChannelAnswer <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelAnswer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);
    status = cmChannelAnswer(CurrentChannel->hChan);

    return WrapperReply(interp, status, argc, argv);
}



/********************************************************************************************
 * api_cm_ChannelDrop
 * purpose : wrapping the original CM api function - ChannelDrop
 * syntax  : api:cm:ChannelDrop <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelDrop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);

    status = cmChannelDrop(CurrentChannel->hChan);
    return WrapperReply(interp, status, argc, argv);
}



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
int api_cm_ChannelRequestCloseReject(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);
    status = cmChannelRequestCloseReject(CurrentChannel->hChan);

    return WrapperReply(interp, status, argc, argv);
}


 /********************************************************************************************
 * api_cm_ChannelClose
 * purpose : wrapping the original CM api function - ChannelClose
 * syntax  : api:cm:ChannelClose <ChannelHandle>
 *           <ChannelHandle>     - cm handle of the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int api_cm_ChannelClose(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* don't forget that the drop reason is hard coded */
    status = cmChannelClose(CurrentChannel->hChan);

    CurrentChannel->hChan = NULL;

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_ChannelGetCallHandle(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;
    HCALL         hsCall;
    HAPPCALL      appCall;
    char          handle[32];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the call handles */
    status = cmChannelGetCallHandles(CurrentChannel->hChan, &hsCall, &appCall);

    if (status >= 0)
    {
        sprintf(handle, "0x%p", appCall);
        Tcl_SetResult(interp, handle, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelGetDependency(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;
    HCHAN         hschan;
    HAPPCHAN      appchan;
    char          handle[32];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the channel handles */
    status = cmChannelGetDependency(CurrentChannel->hChan, &appchan, &hschan);

    if (status >= 0)
    {
        sprintf(handle, "0x%p", appchan);
        Tcl_SetResult(interp, handle, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelGetDuplexAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int                status;
    ChannelInfo      * CurrentChannel;
    cmTransportAddress address;
    char               addrStr[32];
    char               extRef[64];
    BOOL               isAssociated;
    char               result[128];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the address of a bi - directional channel */
    status = cmChannelGetDuplexAddress(CurrentChannel->hChan, &address, 64, extRef, &isAssociated);
    TransportAddress2String(&address, addrStr);

    if (status >= 0)
    {
        sprintf(result, "{ {%s} {%s} {%d} }", addrStr, extRef, isAssociated);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelGetNumber(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;
    char          result[32];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the channel number and convert it to string */
    status = cmChannelGetNumber(CurrentChannel->hChan);

    if (status >= 0)
    {
        sprintf(result, "%d", status);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelGetOrigin(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int           status;
    ChannelInfo * CurrentChannel;
    BOOL          origin;
    char          result[32];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the origin of the channel */
    status = cmChannelGetOrigin(CurrentChannel->hChan, &origin);

    if (status >= 0)
    {
        sprintf(result, "%d", origin);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelGetSource(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int             status;
    ChannelInfo   * CurrentChannel;
    cmTerminalLabel label;
    char            result[32];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* get the origin of the channel */
    status = cmChannelGetSource(CurrentChannel->hChan, &label);

    if (status >= 0)
    {
        sprintf(result, "{ {%d} {%d} }", label.mcuNumber, label.terminalNumber);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }
    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_ChannelIsDuplex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int             status;
    ChannelInfo   * CurrentChannel;
    char            result[4];

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* find out if the channel is bi - directional */
    status = cmChannelIsDuplex(CurrentChannel->hChan);

    if (status >= 0)
    {
        sprintf(result, "%d", status);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}







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
int api_app_SetCallMode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*       CurrentCall;
    ChannelInfo*    CurrentChannel;
    BOOL            isScript;

    sscanf(argv[1], "0x%p", &CurrentCall);

    if (tolower(argv[2][0]) == 's')
        isScript = TRUE;
    else
        isScript = FALSE;

    CurrentCall->scriptCall = isScript;
    CurrentChannel = (ChannelInfo *)CurrentCall->firstChannel;
    while (CurrentChannel != NULL)
    {
        CurrentChannel->scriptChannel = isScript;
        CurrentChannel = CurrentChannel->nextChannel;
    }

    return TCL_OK;
}




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
int api_app_SetChannelMode(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    ChannelInfo*    CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);

    if (tolower(argv[2][0]) == 's')
        CurrentChannel->scriptChannel = TRUE;
    else
        CurrentChannel->scriptChannel = FALSE;

    return TCL_OK;
}


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
int api_app_GetDataTypes(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int numChannels;
    char *DataTypeNameList[20], DataTypeNames[20][20];
    char TclDataType[20 * 21], *temp;
    int i;

    /* preparing DataTypeNameList */
    for (i = 0; i < 20; i++)
        DataTypeNameList[i] = DataTypeNames[i];

    /* get DataType name list from configuration file */
    numChannels = cmGetConfigChannels(hApp, 20, 20, DataTypeNameList);
    if (numChannels >= 0)
    {
        /*preparing string of DataTypes for the combo box in the fast start window*/
        strcpy(TclDataType, DataTypeNameList[0]);
        temp = TclDataType;

        if (numChannels > 20) numChannels = 20;
        for(i = 0; i < numChannels; i++)
            temp += sprintf(temp, "%s ", DataTypeNameList[i]);

        Tcl_SetResult(interp, TclDataType, TCL_VOLATILE);
    }
    else
    {
        Tcl_SetResult(interp, (char *)"", TCL_STATIC);
        numChannels = 0;
    }

    return WrapperReply(interp, numChannels, argc, argv);
}


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
int api_app_LoadMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    /* todo: write it down */
#if 0
    RV_Status               status;
    RV323_ASN_Handle        hAsn;
    RV323_ASN_Message       hMessage1, hMessage2;
    RV323_CFG_Handle        hCfg;
    RV323_ASN_Definition    def;
    RV323_ASN_Node          root;
    char                    result[12];

    if (sscanf(argv[1], "0x%p", &hAsn) != 1)
        return WrapperReply(interp, RV_InvalidHandle, argc, argv);

    hCfg = extApi.cfgInitInterface.CFG_Allocate(extApi.logHandle);
    if (hCfg == NULL)
        return WrapperReply(interp, RV_OutOfResources, argc, argv);

//    RV323_CFG_SetAsnMessageInterface(hCfg, sizeof(RV323_ASN_MessageInterface), &api.asnMessageInterface);
//    RV323_CFG_SetAsnFieldInterface(hCfg, sizeof(RV323_ASN_FieldInterface), &api.asnFieldInterface);

    def = api.asnDefinition;
    extApi.cfgInitInterface.CFG_SetConfig(hCfg, hAsn);
    extApi.cfgInitInterface.CFG_SetAnotherDefinition(hCfg, "Q931", def, RV323_ASN_NODE(Q931Message));
    extApi.cfgInitInterface.CFG_SetAnotherDefinition(hCfg, "RAS", def, RV323_ASN_NODE(RasMessage));
    extApi.cfgInitInterface.CFG_SetAnotherDefinition(hCfg, "H245", def, RV323_ASN_NODE(MultimediaSystemControlMessage));
    status = extApi.cfgInitInterface.CFG_Init(hCfg);

    if (status == RV_Success)
        status = extApi.cfgAsnInterface.CFG_ReadFile(hCfg, argv[2]);

    if (status == RV_Success)
    {
        api.asnMessageInterface.ASN_GetRootNode(&root);
        hMessage1 = extApi.cfgAsnInterface.CFG_GetASN(hCfg, "Q931");

        if (api.asnFieldInterface.ASN_Check(hMessage1, &root) != RV_Success)
        {
            hMessage1 = extApi.cfgAsnInterface.CFG_GetASN(hCfg, "RAS");
            if (api.asnFieldInterface.ASN_Check(hMessage1, &root) != RV_Success)
            {
                hMessage1 = extApi.cfgAsnInterface.CFG_GetASN(hCfg, "H245");
                if (api.asnFieldInterface.ASN_Check(hMessage1, &root) != RV_Success)
                {
                    Tcl_SetResult(interp, "Bad ASN message inside file", TCL_VOLATILE);
                    extApi.cfgInitInterface.CFG_End(hCfg);
                    return TCL_ERROR;
                }
            }
        }
    }

    if (status == RV_Success)
    {
        hMessage2 = api.asnMessageInterface.ASN_DuplicateMessage(hMessage1, hAsn);

        if (hMessage2 == NULL)
            status = RV_OutOfResources;
        else
        {
            sprintf(result, "0x%p", hMessage2);
            Tcl_SetResult(interp, result, TCL_VOLATILE);
        }
    }
    extApi.cfgInitInterface.CFG_End(hCfg);
    return WrapperReply(interp, status, argc, argv);
#endif
    return TCL_OK;
}


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
int api_app_ChannelKill(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int             status;
    ChannelInfo*    CurrentChannel;

    sscanf(argv[1], "0x%p", &CurrentChannel);

    /* Drop this as if it was Idle... */
    cmEvChannelStateChanged((HAPPCHAN)CurrentChannel,
        CurrentChannel->hChan,
        cmChannelStateIdle,
        0);
    status = 0;
    return WrapperReply(interp, status, argc, argv);
}


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
int api_app_GetChannelList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    ChannelInfo* chan;
    CallInfo* Call;
    char* temp, ChanList[2048];
    int args;

    args = sscanf(argv[1], "0x%p", &Call);
    if (args < 0)
        return TCL_OK;

    /* Rewrite the channels information inside the list */
    temp = ChanList;
    chan = Call->firstChannel;
    /* if the list is empty */
    if (chan == NULL)
    {
        Tcl_SetResult(interp, (char *)"", TCL_VOLATILE);
        return TCL_OK;
    }

    while (chan != NULL)
    {
        if (chan->isOutgoing)
        {
            sprintf(temp, "0x%p ", chan->hChan);
            temp = temp + strlen(temp);
        }
        chan = chan->nextChannel;

    }

     Tcl_SetResult(interp, ChanList, TCL_VOLATILE);

    return TCL_OK;
}


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
int api_app_Vt2Address(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    cmTransportAddress  addr;
    HPVT                hVal;
    INT32               len, nodeId, addrNodeId = RVERROR;
    int                 status = 0;
    char                result[256];

    if (argc != 3)
        return WrapperBadParams(interp, "api:app:Vt2Address <valTree> <nodeId>");

    memset(&addr, 0, sizeof(addr));

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = atoi(argv[2]);

    if (nodeId < 0)
        status = RVERROR;
    else
    {
        addrNodeId = pvtChild(hVal, nodeId);
        if (addrNodeId < 0) status = addrNodeId;
        if (pvtGetSyntaxIndex(hVal, addrNodeId) != 1) status = RVERROR;
    }

    if (status >= 0)
    {
        /* Get IP */
        addr.type = cmTransportTypeIP;
        if (pvtGetChild(hVal, addrNodeId, __q931(ip), &nodeId) < 0) status = RVERROR;
        if (pvtGet(hVal, nodeId, NULL, NULL, &len, NULL) < 0) status = RVERROR;
        if (len == sizeof(UINT32))
            pvtGetString(hVal, nodeId, sizeof(addr.ip), (char*)&addr.ip);

        /* Get port */
        if (pvtGetChild(hVal, addrNodeId, __q931(port), &nodeId) < 0) status = RVERROR;
        if (pvtGet(hVal, nodeId, NULL, NULL, &len, NULL) < 0) status = RVERROR;
        if (status >= 0)
            addr.port = len;
    }

    if (status >= 0)
    {
        TransportAddress2String(&addr, result);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_app_Address2Vt(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    cmTransportAddress  addr;
    HPVT                hVal;
    INT32               nodeId, addrNodeId;
    int                 status;

    if (argc != 4)
        return WrapperBadParams(interp, "api:app:Address2Vt <valTree> <address> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = atoi(argv[3]);

    status = String2TransportAddress(argv[2], &addr);

    if ((status < 0) || (nodeId < 0) || (addr.type != cmTransportTypeIP))
        status = RVERROR;
    else
    {
        addrNodeId = pvtAdd(hVal, nodeId, __q931(ipAddress), 0, NULL, NULL);
        pvtAdd(hVal, addrNodeId, __q931(ip), sizeof(addr.ip), (char*)&addr.ip, NULL);
        status = pvtAdd(hVal, addrNodeId, __q931(port), addr.port, NULL, NULL);
    }

    return WrapperReply(interp, status, argc, argv);
}










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
int api_cm_GetLocalRASAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmTransportAddress  addr;
    char                result[128];
    int                 status;

    status = cmGetLocalRASAddress(hApp, &addr);
    if (status >= 0)
    {
        TransportAddress2String(&addr, result);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_GetLocalCallSignalAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmTransportAddress  addr;
    char                result[128];
    int                 status;

    status = cmGetLocalCallSignalAddress(hApp, &addr);
    if (status >= 0)
    {
        TransportAddress2String(&addr, result);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_GetLocalAnnexEAddress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmTransportAddress  addr;
    char                result[128];
    int                 status;

    status = cmGetLocalAnnexEAddress(hApp, &addr);
    if (status >= 0)
    {
        TransportAddress2String(&addr, result);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}





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
int api_cm_Register(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    if (argc != 1) return WrapperBadParams(interp, "api:cm:Register");

    status = cmRegister(hApp);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_Unregister(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    if (argc != 1) return WrapperBadParams(interp, "api:cm:Unregister");

    status = cmUnregister(hApp);

    return WrapperReply(interp, status, argc, argv);
}







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
int api_cmras_StartTransaction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS                hRas;
    cmRASTransaction    transaction;
    cmTransportAddress  destAddr;
    CallInfo*           Call;
    HCALL               hsCall;
    int                 status;
    char                result[20];

    if ((argc < 3) || (argc > 4))
        return WrapperBadParams(interp, "api:cmras:StartTransaction <transaction> <destAddr> [<hsCall>]");

    transaction = String2RASTransaction(argv[1]);
    status = String2TransportAddress(argv[2], &destAddr);
    if ((argc == 4) && (sscanf(argv[3], "0x%p", &Call) == 1) && (Call != NULL))
        hsCall = Call->hsCall;
    else
        hsCall = NULL;

    if (status >= 0)
        status = cmRASStartTransaction(hApp, NULL, &hRas, transaction, &destAddr, hsCall);

    if (status >= 0)
    {
        sprintf(result, "0x%p", hRas);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


/************************************************************************
 * api_cmras_SetParam
 * purpose : wrapping the original ASN api function - cmRASSetParam
 * syntax  : api:cmras:SetParam <hsRas> <trStage> <paramType> <index> <value>
 *           <hsRas>    - Stack's transaction handle
 *           <trStage>  - Stage of transaction to set
 *           <paramType>- Parameter type to set
 *           <index>    - Index to set
 *           <value>    - Value to set
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_SetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS            hRas;
    int             status = 0, freeAlias = 0;
    cmRASTrStage    stage;
    cmRASParam      param;
    int             index;
    INT32           value = 0;
    char*           svalue;
    char            buffer[1024];
    svalue = buffer;

    if (argc != 6)
        return WrapperBadParams(interp, "api:cmras:SetParam <hRas> <trStage> <paramType> <index> <value>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    stage = String2RASTrStage(argv[2]);
    param = String2RASParam(argv[3]);
    index = atoi(argv[4]);

    switch (param)
    {
        case cmRASParamDiscoveryComplete:
        case cmRASParamActiveMC:
        case cmRASParamAnswerCall:
        case cmRASParamMulticastTransaction:
        case cmRASParamAnsweredCall:
        case cmRASParamAlmostOutOfResources:
        case cmRASParamIrrFrequency:
        case cmRASParamBandwidth:
        case cmRASParamCRV:
        case cmRASParamKeepAlive:
        case cmRASParamTimeToLive:
        case cmRASParamDelay:
        case cmRASParamAltGKisPermanent:
        case cmRASParamNonStandardData:
        case cmRASParamNeedResponse:
        case cmRASParamMaintainConnection:
        case cmRASParamMultipleCalls:
        case cmRASParamWillRespondToIRR:
        case cmRASParamSupportsAltGk:
        case cmRASParamAdditiveRegistration:
        case cmRASParamSupportsAdditiveRegistration:
        case cmRASParamSegmentedResponseSupported:
        case cmRASParamNextSegmentRequested:
        case cmRASParamCapacityInfoRequested:
        case cmRASParamHopCount:
            /* BOOL/int */
            svalue = NULL;
            value = atoi(argv[5]);
            break;

        case cmRASParamRASAddress:
        case cmRASParamCallSignalAddress:
        case cmRASParamDestCallSignalAddress:
        case cmRASParamSrcCallSignalAddress:
        case cmRASParamReplyAddress:
        case cmRASParamDestinationIpAddress:
            /* cmTransportAddress */
            value = sizeof(cmRASTransport);
            status = String2TransportAddress(argv[5], (cmRASTransport*)svalue);
            break;

        case cmRASParamGatekeeperID:
        case cmRASParamEndpointID:
        {
            cmRASAlias* alias;
            char * aliasString = (buffer + sizeof(cmAlias));
            alias = (cmRASAlias *)svalue;

            value = sizeof(cmRASAlias);
            alias->length = 2*strlen(argv[5]);
            if (param == cmRASParamEndpointID)
                alias->type = cmAliasTypeEndpointID;
            else
                alias->type = cmAliasTypeGatekeeperID;
            String2Bmp(argv[5], aliasString, strlen(argv[5]));
            alias->string = aliasString;
            break;
        }

        case cmRASParamEndpointAlias:
        case cmRASParamTerminalAlias:
        case cmRASParamDestInfo:
        case cmRASParamSrcInfo:
        case cmRASParamDestExtraCallInfo:
        case cmRASParamRejectedAlias:
        case cmRASParamSourceInfo:
        case cmRASParamInvalidTerminalAlias:
            /* cmRASAlias */
            value = sizeof(cmRASAlias);
            status = String2Alias(argv[5], (cmRASAlias*)svalue);
            freeAlias = (status >= 0);
            break;

        case cmRASParamNonStandard:
        {
            cmNonStandardParam* nsdParam;
            nsdParam = (cmNonStandardParam *)buffer;
            nsdParam->data = buffer + sizeof(cmNonStandardParam);
            value = sizeof(*nsdParam);

            status = String2CMNonStandardParam(argv[5], nsdParam);
            break;
        }

        case cmRASParamAlternateGatekeeper:
        case cmRASParamAltGKInfo:
        {
            cmAlternateGatekeeper* altGk;
            altGk = (cmAlternateGatekeeper *)buffer;
            value = sizeof(*altGk);

            status = String2CMAlternateGatekeeper(argv[5], altGk);
            break;
        }

        case cmRASParamEndpointType:
        case cmRASParamTerminalType:
            {
                value = (INT32) String2EndpointType(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(EndpointType2String((cmEndpointType) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, EndpointType2String((cmEndpointType) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }

        case cmRASParamCallType:
            {
                value = (INT32) String2CallType(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(CallType2String((cmCallType) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, CallType2String((cmCallType) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
        case cmRASParamCallModel:
            {
                value = (INT32) String2CallModelType(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(CallModelType2String((cmCallModelType) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, CallModelType2String((cmCallModelType) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
        case cmRASParamDisengageReason:
            {
                value = (INT32) String2DisengageReason(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(DisengageReason2String((cmRASDisengageReason) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, DisengageReason2String((cmRASDisengageReason) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
        case cmRASParamRejectReason:
            {
                value = (INT32) String2RASReason(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(RASReason2String((cmRASReason) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, RASReason2String((cmRASReason) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
        case cmRASParamUnregReason:
            {
                value = (INT32) String2UnregReason(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(UnregReason2String((cmRASUnregReason) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, UnregReason2String((cmRASUnregReason) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
        case cmRASParamTransportQOS:
            {
                value = (INT32) String2TransportQOS(argv[5]);
                if(value == -1)
                {
                    int i = 0;

                    sprintf(buffer, "Enum value incorrect. Possible values: ");
                    while(strcmp(TransportQOS2String((cmTransportQOS) i), "-unknown-"))
                    {
                        sprintf(buffer, "%s %s,", buffer, TransportQOS2String((cmTransportQOS) i));
                        i++;
                    }
                    buffer[strlen(buffer)-1] = '.';

                    Tcl_SetResult(interp, buffer, TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }
            /* enums */
        case cmRASParamIrrStatus:
            {
                value = (INT32) String2IrrStatus(argv[5]);
                if(value == -1)
                {
                    Tcl_SetResult(interp, (char *)"Enum value incorrect. Possible values: "
                        "Complete, Incomplete, InvalidCall.", TCL_VOLATILE);
                    return TCL_ERROR;
                }
                break;
            }

        case cmRASParamCID:
        case cmRASParamCallID:
            {
                int i;
                for(i=0; i<16; i++)
                {
                    svalue[i] = argv[5][i];
                }
                value = 16;
                break;
            }
            /* misc/check */

            /* HCALL */
        case cmRASParamCallHandle:
            /* This parameter is READ ONLY */
            status = RVERROR;
            break;

        case cmRASParamEndpointVendor:
        case cmRASParamEmpty:
        default:
            break;
    }

    if (status >= 0)
        status = cmRASSetParam(hRas, stage, param, index, value, svalue);

    if(freeAlias && ((cmRASAlias*)svalue)->string) AppFree(((cmRASAlias*)svalue)->string);

    return WrapperReply(interp, status, argc, argv);
}


/************************************************************************
 * api_cmras_GetParam
 * purpose : wrapping the original ASN api function - cmRASGetParam
 * syntax  : api:cmras:GetParam <hsRas> <trStage> <paramType> <index>
 *           <hsRas>    - Stack's transaction handle
 *           <trStage>  - Stage of transaction to set
 *           <paramType>- Parameter type to set
 *           <index>    - Index to set
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : value
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cmras_GetParam(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS            hRas;
    int             status;
    cmRASTrStage    stage;
    cmRASParam      param;
    int             index;
    INT32           value;
    char*           svalue;
    char            buffer[1024];
    char*           result;
    char            resultBuf[1024];
    svalue = buffer;
    result = resultBuf;

    if (argc != 5)
        return WrapperBadParams(interp, "api:cmras:GetParam <hRas> <trStage> <paramType> <index>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    stage = String2RASTrStage(argv[2]);
    param = String2RASParam(argv[3]);
    index = atoi(argv[4]);

    /* Make sure the svalue parameter is initialized properly */
    memset(buffer, 0, sizeof(buffer));
    switch (param)
    {
        case cmRASParamAlternateGatekeeper:
        case cmRASParamAltGKInfo:
        {
            cmAlternateGatekeeper* altGk = (cmAlternateGatekeeper *)svalue;
            altGk->gatekeeperIdentifier.string = svalue + sizeof(cmAlternateGatekeeper);
            break;
        }
        default:
            /* do nothing */
            break;
    }

    switch(param)
    {
        case (cmRASParamNonStandard):
            ((cmNonStandardParam *) svalue)->data = (svalue + sizeof(cmNonStandardParam));
            break;
        case cmRASParamGatekeeperID:
        case cmRASParamEndpointID:
        case cmRASParamEndpointAlias:
        case cmRASParamTerminalAlias:
        case cmRASParamDestInfo:
        case cmRASParamSrcInfo:
        case cmRASParamDestExtraCallInfo:
        case cmRASParamRejectedAlias:
        case cmRASParamSourceInfo:
        case cmRASParamInvalidTerminalAlias:
            ((cmAlias *) svalue)->string = (svalue + sizeof(cmAlias));
            break;
        default:
            break;
    }

    status = cmRASGetParam(hRas, stage, param, index, &value, svalue);

    if (status >= 0)
    {
        switch (param)
        {
            case cmRASParamDiscoveryComplete:
            case cmRASParamActiveMC:
            case cmRASParamAnswerCall:
            case cmRASParamMulticastTransaction:
            case cmRASParamAnsweredCall:
            case cmRASParamAlmostOutOfResources:
            case cmRASParamIrrFrequency:
            case cmRASParamBandwidth:
            case cmRASParamCRV:
            case cmRASParamKeepAlive:
            case cmRASParamTimeToLive:
            case cmRASParamDelay:
            case cmRASParamAltGKisPermanent:
            case cmRASParamNonStandardData:
            case cmRASParamNeedResponse:
            case cmRASParamMaintainConnection:
            case cmRASParamMultipleCalls:
            case cmRASParamWillRespondToIRR:
            case cmRASParamSupportsAltGk:
            case cmRASParamAdditiveRegistration:
            case cmRASParamSupportsAdditiveRegistration:
            case cmRASParamSegmentedResponseSupported:
            case cmRASParamNextSegmentRequested:
            case cmRASParamCapacityInfoRequested:
            case cmRASParamHopCount:
                /* BOOL/int */
                sprintf(resultBuf, "%d", value);
                break;

            case cmRASParamRASAddress:
            case cmRASParamDestCallSignalAddress:
            case cmRASParamSrcCallSignalAddress:
            case cmRASParamReplyAddress:
            case cmRASParamDestinationIpAddress:
            {   /* cmTransportAddress */
                cmTransportAddress * address = (cmTransportAddress *) svalue;
                TransportAddress2String(address, result);
                break;
            }

            case cmRASParamEndpointAlias:
            case cmRASParamTerminalAlias:
            case cmRASParamDestInfo:
            case cmRASParamSrcInfo:
            case cmRASParamDestExtraCallInfo:
            case cmRASParamRejectedAlias:
            case cmRASParamSourceInfo:
            case cmRASParamInvalidTerminalAlias:
            {   /*cmAlias */
                cmAlias * alias = (cmAlias *) svalue;
                Alias2String(alias, result);
                break;
            }
            case cmRASParamGatekeeperID:
            case cmRASParamEndpointID:
                {   /*cmAlias BMP */
                    cmAlias * alias = (cmAlias *) svalue;
                    Bmp2String(alias->string, result, alias->length);
                    break;
                }
            case cmRASParamNonStandard:
            {
                cmNonStandardParam* nsdParam = (cmNonStandardParam *)svalue;
                result = CMNonStandardParam2String(nsdParam);
                break;
            }

            case cmRASParamAlternateGatekeeper:
            case cmRASParamAltGKInfo:
            {
                cmAlternateGatekeeper* altGk = (cmAlternateGatekeeper *)svalue;
                result = CMAlternateGatekeeper2String(altGk);
                break;
            }

            case cmRASParamEndpointType:
            case cmRASParamTerminalType:
                {
                    result = EndpointType2String((cmEndpointType)value);
                    break;
                }
            case cmRASParamCallType:
                {
                    result = CallType2String((cmCallType)value);
                    break;
                }
            case cmRASParamCallModel:
                {
                    result = CallModelType2String((cmCallModelType)value);
                    break;
                }
            case cmRASParamDisengageReason:
                {
                    result = DisengageReason2String((cmRASDisengageReason)value);
                    break;
                }
            case cmRASParamRejectReason:
                {
                    result = RASReason2String((cmRASReason)value);
                    break;
                }
            case cmRASParamUnregReason:
                {
                    result = UnregReason2String((cmRASUnregReason)value);
                    break;
                }
            case cmRASParamTransportQOS:
                {
                    result = TransportQOS2String((cmTransportQOS)value);
                    break;
                }
                /* enums */
            case cmRASParamIrrStatus:
                {
                    result = IrrStatus2String(value);
                    break;
                }

            case cmRASParamCID:
            case cmRASParamCallID:
                {
                    result = svalue;
                    result[16] = '\0';
                    break;
                }

                /* misc/check */

                /* HCALL */
            case cmRASParamCallHandle:
                {
                    HCALL       hsCall    = *(HCALL*)svalue;
                    HAPPCALL    haCall;
                    if (hsCall != NULL)
                        cmCallGetHandle(hsCall, &haCall);
                    else
                        haCall = NULL;
                    sprintf(result, "0x%p", haCall);
                }

            case cmRASParamEndpointVendor:
            case cmRASParamEmpty:

            default:
                break;
        }
    }

    if (status >= 0)
        Tcl_SetResult(interp, result, TCL_VOLATILE);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_GetNumOfParams(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS            hRas;
    int             status;
    cmRASTrStage    stage;
    cmRASParam      param;
    char            result[20];

    if (argc != 4)
        return WrapperBadParams(interp, "api:cmras:GetNumOfParams <hRas> <trStage> <paramType>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    stage = String2RASTrStage(argv[2]);
    param = String2RASParam(argv[3]);

    status = cmRASGetNumOfParams(hRas, stage, param);

    if (status >= 0)
    {
        sprintf(result, "%d", status);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_Request(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cmras:Request <hRas>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASRequest(hRas);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_DummyRequest(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cmras:DummyRequest <hRas>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASDummyRequest(hRas);

    return WrapperReply(interp, status, argc, argv);
}



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
int api_cmras_Confirm(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cmras:Confirm <hRas>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASConfirm(hRas);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_Reject(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;

    if (argc != 3)
        return WrapperBadParams(interp, "api:cmras:Reject <hRas> <reason>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASReject(hRas, String2RASReason(argv[2]));

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_InProgress(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;
    int     delay;

    if (argc != 3)
        return WrapperBadParams(interp, "api:cmras:InProgress <hRas> <delay>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    sscanf(argv[2], "%d", &delay);

    status = cmRASInProgress(hRas, delay);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cmras_Close(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS    hRas;
    int     status;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cmras:Close <hRas>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASClose(hRas);

    return WrapperReply(interp, status, argc, argv);
}



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
int api_cmras_GetTransaction(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    HRAS                hRas;
    cmRASTransaction    transaction;
    int                 status;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cmras:GetTransaction <hRas>");

    sscanf(argv[1], "0x%p", (void**)&hRas);
    status = cmRASGetTransaction(hRas, &transaction);
    if (status >= 0)
        Tcl_SetResult(interp, RASTransaction2String(transaction), TCL_STATIC);

    return WrapperReply(interp, status, argc, argv);
}







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
int api_cm_Vt2Alias(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    char    aliasStr[256];
    cmAlias alias;
    INT32   nodeId;
    int     status;
    char    result[256];
    HPVT    hVal;

    if (argc != 3)
        return WrapperBadParams(interp, "api:cm:Vt2Alias <valTree> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    alias.length = sizeof(aliasStr);
    alias.string = aliasStr;

    nodeId = atoi(argv[2]);
    status = cmVt2Alias(hVal, &alias, nodeId);
    if (status >= 0)
    {
        Alias2String(&alias, result);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_Alias2Vt(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    cmAlias alias;
    INT32   nodeId;
    int     status;
    char    result[20];
    HPVT    hVal;

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:Alias2Vt <valTree> <alias> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    status = String2Alias(argv[2], &alias);
    nodeId = atoi(argv[3]);

    if (status >= 0)
    {
        status = cmAlias2Vt(hVal, &alias, nodeId);
        if(alias.string)
            AppFree(alias.string);
    }

    if (status >= 0)
    {
        sprintf(result, "%d", status);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


/************************************************************************
 * api_cm_GetProperty
 * purpose : wrapping the original ASN api function - cmGetProperty
 * syntax  : api:cm:GetProperty <handle> [CALL]
 *           <handle>   - Ras or Call handle whose property we want
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : Alias node ID
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ************************************************************************/
int api_cm_GetProperty(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPROTOCOL   handle;
    int         status;
    char        result[20];

    if ((argc < 2) || (argc > 3))
        return WrapperBadParams(interp, "api:cm:GetProperty <handle> [CALL]");

    sscanf(argv[1], "0x%p", (void**)&handle);

    if ((argc == 3) && (strcmp("CALL", argv[2]) == 0))
    {
        /* Call handle - from CallInfo */
        CallInfo* call = (CallInfo*)handle;
        status = cmGetProperty((HPROTOCOL)call->hsCall);
    }
    else
    {
        /* RAS handle - take directly */
        status = cmGetProperty(handle);
    }

    if (status >= 0)
    {
        sprintf(result, "%d", status);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_GetValTree(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    char    result[20];

    if (argc != 1)
        return WrapperBadParams(interp, "api:cm:GetValTree");

    sprintf(result, "0x%p", cmGetValTree(hApp));
    Tcl_SetResult(interp, result, TCL_VOLATILE);

    return WrapperReply(interp, 0, argc, argv);
}


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
int api_cm_GetRASConfigurationHandle(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    int     nodeId;
    char    result[20];

    if (argc != 1)
        return WrapperBadParams(interp, "api:cm:GetRASConfigurationHandle");

    nodeId = cmGetRASConfigurationHandle(hApp);

    sprintf(result, "%d", nodeId);
    Tcl_SetResult(interp, result, TCL_VOLATILE);

    return WrapperReply(interp, nodeId, argc, argv);
}






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
int api_pvt_AddRoot(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    char    result[20];

    if (argc != 4)
        return WrapperBadParams(interp, "api:pvt:AddRoot <valTree> <rootName> <value>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = pvtAddRoot(hVal, cmGetSynTreeByRootName(hApp, argv[2]), atoi(argv[3]), NULL);

    if (nodeId >= 0)
    {
        sprintf(result, "%d", nodeId);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, nodeId, argc, argv);
}


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
int api_pvt_GetByPath(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    char    result[20];

    if (argc != 4)
        return WrapperBadParams(interp, "api:pvt:GetByPath <valTree> <nodeId> <path>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = pvtGetByPath(hVal, atoi(argv[2]), argv[3], NULL, NULL, NULL);

    if (nodeId >= 0)
    {
        sprintf(result, "%d", nodeId);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, nodeId, argc, argv);
}


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
int api_pvt_BuildByPath(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    char    result[20];

    if (argc != 5)
        return WrapperBadParams(interp, "api:pvt:BuildByPath <valTree> <nodeId> <path> <value>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = pvtBuildByPath(hVal, atoi(argv[2]), argv[3], 0, NULL);

    if (nodeId >= 0)
    {
        int synNodeId;

        pvtGet(hVal, nodeId, NULL, &synNodeId, NULL, NULL);
        switch (pstGetNodeType(pvtGetSynTree(hVal, nodeId), synNodeId))
        {
            case pstInteger:
            case pstNull:
            case pstBoolean:
            case pstEnumeration:
                pvtSet(hVal, nodeId, -1, atoi(argv[4]), NULL);
                break;

            case pstObjectIdentifier:
            case pstOctetString:
            case pstBitString:
            case pstGeneralString:
            case pstUniversalString:
            case pstIA5String:
            case pstVisibleString:
            case pstNumericString:
            case pstPrintableString:
                pvtSet(hVal, nodeId, -1, strlen(argv[4]), argv[4]);
                break;

            case pstBMPString:
            {
                char buffer[1024];
                String2Bmp(argv[4], buffer, strlen(argv[4]));
                pvtSet(hVal, nodeId, -1, strlen(argv[4])*2, buffer);
                break;
            }

            case pstChoice:
            case pstSequence:
            case pstSet:
            case pstOf:
            case pstSequenceOf:
            case pstSetOf:
                break;
        }

        sprintf(result, "%d", nodeId);
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, nodeId, argc, argv);
}


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
int api_pvt_Delete(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT    hVal;
    int     status;

    if (argc != 3)
        return WrapperBadParams(interp, "api:pvt:Delete <valTree> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    status = pvtDelete(hVal, atoi(argv[2]));

    return WrapperReply(interp, status, argc, argv);
}

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
int api_pvt_GetString(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT    hVal;
    INT32   nodeId;
    char    result[256];
    INT32   length = 256;

    if (argc != 3)
        return WrapperBadParams(interp, "api:pvt:GetString <valTree> <nodeId>");

    if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
        return RVERROR;

    nodeId = atoi(argv[2]);
    length = pvtGetString(hVal, nodeId, length, (char *) result);

    if (length > 0)
    {
        int synNodeId;

        pvtGet(hVal, nodeId, NULL, &synNodeId, NULL, NULL);
        if (pstGetNodeType(pvtGetSynTree(hVal, nodeId), synNodeId) == pstBMPString)
        {
            int i;
            for(i=0; i<=length; i++)
                result[i] = result[i*2+1];
            result[length/2] = '\0';
        }
        Tcl_SetResult(interp, result, TCL_VOLATILE);
    }

    return WrapperReply(interp, nodeId, argc, argv);
}

/************************************************************************
 * myPrintFunc
 * purpose : Printing function used by pvtPrint()
 * input   : type   - Parameter used (here it's the pointer to the
 *                    callback's function name
 *           line   - The line to print
 * output  : none
 * return  : none
 ************************************************************************/
void myPrintFunc(IN int type, IN const char*line, ...)
{
    TclExecute("%s {%s}", (char *)type, TclString(line));
}


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
int api_pvt_Print(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    int     status;

    if (argc != 4)
        return WrapperBadParams(interp, "api:pvt:Print <valTree> <nodeId> <callback>");

#ifdef _DEBUG
    {
        HPVT    hVal;

        if (sscanf(argv[1], "0x%p", (void**)&hVal) != 1)
            return RVERROR;

        status = pvtPrint(hVal, atoi(argv[2]), myPrintFunc, (int)argv[3]);
    }
#else
    status = 0;
#endif

    return WrapperReply(interp, status, argc, argv);
}















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
int api_cm_FastStartBuild(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* CallHandle;
    int status;
    char * chanDir, * chanType, nodeID[16];
    cmCapDataType DataType = cmCapEmpty;
    cmChannelDirection direction = dirReceive;
    cmFastStartChannel fsChannel;

    if (argc != 6)
        return WrapperBadParams(interp, "api:app:FastStartBuild <handle> <chanNum> <chanDir> <chanName> <chanType>");

    sscanf(argv[1], "0x%p", &CallHandle);
    fsChannel.index = atoi(argv[2]);
    chanDir = argv[3];
    fsChannel.channelName = argv[4];
    chanType = argv[5];

    if(chanDir[0] == 't')
    {
        /* Add this channle to the alternatives of this partnerChannel */
        direction = dirTransmit;
        fsChannel.rtp.port = 0;
        fsChannel.rtp.ip = 0;
        fsChannel.rtcp.type = cmTransportTypeIP;
        fsChannel.rtcp.port = 2327;
        fsChannel.rtcp.ip = 0;
        fsChannel.rtcp.type = cmTransportTypeIP;
        fsChannel.dataTypeHandle = -1;
    }
    if(chanDir[0] == 'r')
    {
        /* Add this channle to the alternatives of this partnerChannel */
        direction = dirReceive;
        fsChannel.rtp.port = 2326;
        fsChannel.rtp.ip = 0;
        fsChannel.rtcp.type = cmTransportTypeIP;
        fsChannel.rtcp.port = 2327;
        fsChannel.rtcp.ip = 0;
        fsChannel.rtcp.type = cmTransportTypeIP;
        fsChannel.dataTypeHandle = -1;
    }

    switch(chanType[0])
    {
    case('e'):
        {
            DataType = cmCapEmpty;
            break;
        }
    case('a'):
        {
            DataType = cmCapAudio;
            break;
        }
    case('v'):
        {
            DataType = cmCapVideo;
            break;
        }
    case('d'):
        {
            DataType = cmCapData;
            break;
        }
    case('n'):
        {
            DataType = cmCapNonStandard;
            break;
        }
    case('u'):
        {
            DataType = cmCapUserInput;
            break;
        }
    case('c'):
        {
            DataType = cmCapConference;
            break;
        }
    case('h'):
        {
            DataType = cmCapH235;
            break;
        }
    case('m'):
        {
            DataType = cmCapMaxPendingReplacementFor;
            break;
        }
    case('g'):
        {
            DataType = cmCapGeneric;
            break;
        }
    }
    status = cmFastStartBuild(CallHandle->hsCall, DataType, direction, &fsChannel);

    if (status >= 0)
    {
        sprintf(nodeID, "%d", status);
        Tcl_SetResult(interp, nodeID, TCL_VOLATILE);
    }

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_CallAddFastStartMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * CallHandle;
    int nodeID, status = RVERROR;

    sscanf(argv[1], "0x%p", &CallHandle);
    nodeID = atoi(argv[2]);

    if(nodeID >= 0)
        status = cmCallAddFastStartMessage(CallHandle->hsCall, nodeID);

    return WrapperReply(interp, status, argc, argv);
}



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
int api_app_FSGetChanNum(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    char numChan[16];

    if (argc != 2)
        return WrapperBadParams(interp, "api:app:FSGetChanNum <fsMessage>");

    sscanf(argv[1], "0x%p", &fsMessage);
    sprintf(numChan, "%d", fsMessage->partnerChannelsNum);

    Tcl_SetResult(interp, numChan, TCL_VOLATILE);
    return TCL_OK;
}

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
int api_app_FSGetAltChanNum(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum;
    char altChanNum[16];

    if (argc != 4)
        return WrapperBadParams(interp, "api:app:FSGetAltChanNum <fsMessage> <chanNum><chanDir>");

    sscanf(argv[1], "0x%p", &fsMessage);
    chanNum = atoi(argv[2]);
    if(argv[3][0] == 't')
        sprintf(altChanNum, "%d", fsMessage->partnerChannels[chanNum].transmit.altChannelNumber);
    if(argv[3][0] == 'r')
        sprintf(altChanNum, "%d", fsMessage->partnerChannels[chanNum].receive.altChannelNumber);
    Tcl_SetResult(interp, altChanNum, TCL_VOLATILE);
    return TCL_OK;
}

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
int api_app_FSGetChanIndex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum, altChanNum;
    char chanIndex[16];

    if (argc != 5)
        return WrapperBadParams(interp, "api:app:FSGetChanIndex <fsMessage> <chanNum> <chanDir> <altChan>");

    sscanf(argv[1], "0x%p", &fsMessage);
    chanNum = atoi(argv[2]);
    altChanNum = atoi(argv[4]);
    if(argv[3][0] == 'r')
        sprintf(chanIndex, "%d", fsMessage->partnerChannels[chanNum].receive.channels[altChanNum].index);
    if(argv[3][0] == 't')
        sprintf(chanIndex, "%d", fsMessage->partnerChannels[chanNum].transmit.channels[altChanNum].index);
    Tcl_SetResult(interp, chanIndex, TCL_VOLATILE);
    return TCL_OK;
}

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
int api_app_FSGetChanName(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum, altChanNum;

    if (argc != 5)
        return WrapperBadParams(interp, "api:app:FSGetChanName <fsMessage> <chanNum> <chanDir> <altChan>");

    sscanf(argv[1], "0x%p", &fsMessage);
    chanNum = atoi(argv[2]);
    altChanNum = atoi(argv[4]);
    if(argv[3][0] == 'r') Tcl_SetResult(interp,
        fsMessage->partnerChannels[chanNum].receive.channels[altChanNum].channelName,
        TCL_VOLATILE);
    if(argv[3][0] == 't') Tcl_SetResult(interp,
        fsMessage->partnerChannels[chanNum].transmit.channels[altChanNum].channelName,
        TCL_VOLATILE);
    return TCL_OK;
}

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
int api_app_FSGetChanRTCP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum, altChanNum;
    char rtcpString[32];

    if (argc != 5)
        return WrapperBadParams(interp, "api:app:FSGetChanRTCP <fsMessage> <chanNum> <chanDir> <altChan>");

    sscanf(argv[1], "0x%p", &fsMessage);
    chanNum = atoi(argv[2]);
    altChanNum = atoi(argv[4]);
    if(argv[3][0] == 'r')
        TransportAddress2String(&fsMessage->partnerChannels[chanNum].receive.channels[altChanNum].rtcp,
        rtcpString);
    if(argv[3][0] == 't')
        TransportAddress2String(&fsMessage->partnerChannels[chanNum].transmit.channels[altChanNum].rtcp,
        rtcpString);
    Tcl_SetResult(interp, rtcpString, TCL_VOLATILE);
    return TCL_OK;
}

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
int api_app_FSGetChanRTP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum, altChanNum;
    char rtpString[32];

    if (argc != 5)
        return WrapperBadParams(interp, "api:app:FSGetChanRTP <fsMessage> <chanNum> <chanDir> <altChan>");

    sscanf(argv[1], "0x%p", &fsMessage);
    chanNum = atoi(argv[2]);
    altChanNum = atoi(argv[4]);
    if(argv[3][0] == 'r')
        TransportAddress2String(&fsMessage->partnerChannels[chanNum].receive.channels[altChanNum].rtp,
        rtpString);
    if(argv[3][0] == 't')
        TransportAddress2String(&fsMessage->partnerChannels[chanNum].transmit.channels[altChanNum].rtp,
        rtpString);
    Tcl_SetResult(interp, rtpString, TCL_VOLATILE);
    return TCL_OK;
}


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
int api_cm_FastStartChannelsAck(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmFastStartMessage * fsMessage;
    int chanNum, altChanNum;
    int status = RVERROR;
    CallInfo* CallHandle;

    if (argc != 6)
        return WrapperBadParams(interp, "api:cm:FastStartChannelsAck <handle> <fsMessage> <chanNum> <chanDir> <altChan>");

    sscanf(argv[1], "0x%p", &CallHandle);
    sscanf(argv[2], "0x%p", &fsMessage);
    chanNum = atoi(argv[3]);
    altChanNum = atoi(argv[5]);
    if(argv[4][0] == 'r')
        status = cmFastStartChannelsAck(CallHandle->hsCall, &fsMessage->partnerChannels[chanNum].receive.channels[altChanNum]);
    if(argv[4][0] == 't')
        status = cmFastStartChannelsAck(CallHandle->hsCall, &fsMessage->partnerChannels[chanNum].transmit.channels[altChanNum]);

    return TCL_OK;
}

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
int api_cm_FastStartChannelsAckIndex(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int index, status;
    CallInfo* CallHandle;
    cmTransportAddress rtp, rtcp;

    if (argc != 5)
        return WrapperBadParams(interp, "api:cm:FastStartChannelsAckIndex <handle> <index> <rtcp> <rtp>");

    sscanf(argv[1], "0x%p", &CallHandle);
    index = atoi(argv[2]);
    String2TransportAddress(argv[3], &rtcp);
    String2TransportAddress(argv[4], &rtp);
    status = cmFastStartChannelsAckIndex(CallHandle->hsCall, index, &rtcp, &rtp);

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_FastStartChannelsReady(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    CallInfo* CallHandle;

    if (argc != 2)
        return WrapperBadParams(interp, "api:cm:FastStartChannelsAckIndex <handle>");

    sscanf(argv[1], "0x%p", &CallHandle);
    status = cmFastStartChannelsReady(CallHandle->hsCall);

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_CallSendSupplementaryService(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    char buf[1024];
    CallInfo* CallHandle;

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:CallSendSupplementaryService <handle> <buffer> <force>");

    sscanf(argv[1], "0x%p", &CallHandle);
    String2Buffer(argv[2], buf, sizeof(buf));
    status = cmCallSendSupplementaryService(CallHandle->hsCall, buf, strlen(argv[2])/2, (BOOL)atoi(argv[3]));

    return WrapperReply(interp, status, argc, argv);
}

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
int api_cm_CallSimulateMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int status;
    CallInfo* CallHandle;
    int nodeId;

    if (argc != 3)
        return WrapperBadParams(interp, "api:cm:CallSimulateMessage <handle> <nodeId>");

    sscanf(argv[1], "0x%p", &CallHandle);
    nodeId = atoi(argv[2]);
    status = cmCallSimulateMessage(CallHandle->hsCall, nodeId);

    return WrapperReply(interp, status, argc, argv);
}


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
int api_cm_CallCreateAnnexMMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    BOOL requiered;
    cmTunnelledProtocolID tunnelledProtocolID;
    cmNonStandardParam  nsParam;
    cmOctetString message[10];
    int i, retVal;
    char retStr[32];

    if (argc != 17)
        return WrapperBadParams(interp, "api:cm:api_cm_SendAnnexMMessage <requiered> <objectID> <altIDType> <altIDVariant> <subID> <nsdString> <messages>...");

    requiered = (argv[1][0] == '1');

    strcpy(tunnelledProtocolID.tunnelledProtocolObjectID, argv[2]);
    strcpy(tunnelledProtocolID.protocolType, argv[3]);
    tunnelledProtocolID.protocolTypeLength = strlen(argv[3]);
    strcpy(tunnelledProtocolID.protocolVariant, argv[4]);
    tunnelledProtocolID.protocolVariantLength = strlen(argv[4]);
    strcpy(tunnelledProtocolID.subIdentifier, argv[5]);
    tunnelledProtocolID.subIdentifierLength = strlen(argv[5]);

    String2CMNonStandardParam(argv[6], &nsParam);

    for(i=0; i<10; i++)
    {
        if(argv[i+7][0])
        {
            message[i].message = argv[i+7];
            message[i].size = strlen(argv[i+7]);
        }
        else
        {
            message[i].message = NULL;
            message[i].size = 0;
            break;
        }
    }

    retVal = cmCallCreateAnnexMMessage(hApp,requiered,&tunnelledProtocolID,message,&nsParam);
    sprintf(retStr, "%d", (UINT32)retVal);
    Tcl_SetResult(interp, retStr, TCL_VOLATILE);

    return TCL_OK;
}

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
int api_cm_CallSendAnnexMMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * Call;
    BOOL force;
    int AnnexMmessage;

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:api_cm_SendAnnexMMessage <callInfo> <force> <AnnexMmessage>");

    sscanf(argv[1], "0x%p", &Call);

    AnnexMmessage = atoi(argv[3]);

    force = (argv[2][0] == '1');

    cmCallSendAnnexMMessage(Call->hsCall, AnnexMmessage ,force);
    return TCL_OK;
}

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
int api_cm_CallCreateAnnexLMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    BOOL isText;
    cmNonStandardParam  nsParam;
    int retVal;
    char retStr[32];

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:api_cm_SendAnnexLMessage <callInfo> <isText> <force> <nsdString> <messages>");

    isText = (argv[1][0] == '1');

    String2CMNonStandardParam(argv[2], &nsParam);

    retVal = cmCallCreateAnnexLMessage(hApp, isText, argv[3], strlen(argv[3]), &nsParam);
    sprintf(retStr, "%d", (UINT32)retVal);
    Tcl_SetResult(interp, retStr, TCL_VOLATILE);

    return TCL_OK;
}

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
int api_cm_CallSendAnnexLMessage(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo * Call;
    BOOL force;
    int AnnexLMessage;

    if (argc != 4)
        return WrapperBadParams(interp, "api:cm:api_cm_SendAnnexLMessage <callInfo> <force> <AnnexLmessage>");

    sscanf(argv[1], "0x%p", &Call);
    force = (argv[2][0] == '1');
    AnnexLMessage = atoi(argv[3]);

    cmCallSendAnnexLMessage(Call->hsCall, AnnexLMessage, force);
    return TCL_OK;
}


#ifdef __cplusplus
}
#endif


