/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                init.c
 *
 * This file include functions that initiate the stack and the tcl.
 *
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

#ifdef WIN32
/* Windows */
#include <windows.h>
#include "resource.h"

#else
/* Unix */
#include <time.h>
#endif

#include <stdlib.h>
#include "TAP_general.h"
#include "TAP_snmp.h"
#include "TAP_security.h"
#include "TAP_h450.h"
#include "TAP_channel.h"
#include "TAP_call.h"
#include "TAP_ras.h"
#include "TAP_tools.h"
#include "TAP_utils.h"
#include "TAP_wrapper.h"
#include "TAP_init.h"


#define TCL_FILENAME    "TAP_testapp.tcl"
#define TCL_LIBPATH     "lib/tcl8.3"
#define TK_LIBPATH      "lib/tk8.3"

#ifdef WIN32
#define CONFIG_FILE     ".\\config.val"
#else
#define CONFIG_FILE     "./config.val"
#endif

HAPP    hApp;


/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * tclGetFile
 * purpose : This function is automatically generated with the tcl scripts array in
 *           TAP_scripts.tcl.
 *           It returns the buffer holding the .tcl file information
 * input   : name   - Name of file to load
 * output  : none
 * return  : The file's buffer if found
 *           NULL if file wasn't found
 ********************************************************************************************/
char* tclGetFile(IN char* name);


/********************************************************************************************
 * test_Source
 * purpose : This function replaces the "source" command of the TCL
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Source(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    FILE* exists;
    char* fileBuf;

    if (argc != 2)
    {
        Tcl_SetResult(interp, (char *)"wrong # args: should be \"source <filename>\"", TCL_STATIC);
        return TCL_ERROR;
    }

    /* First see if we've got such a file on the disk */
    exists = fopen(argv[1], "r");
    if (exists == NULL)
    {
        /* File doesn't exist - get from compiled array */
        fileBuf = tclGetFile(argv[1]);
        if (fileBuf == NULL)
        {
            /* No such luck - we don't have a file to show */
            char error[300];
            sprintf(error, "file %s not found", argv[1]);
            Tcl_SetResult(interp, error, TCL_VOLATILE);
            return TCL_ERROR;
        }
        else
        {
            /* Found! */
            int retCode;
            retCode = Tcl_Eval(interp, fileBuf);
            if (retCode == TCL_ERROR)
            {
                char error[300];
                sprintf(error, "\n    (file \"%s\" line %d)", argv[1], interp->errorLine);
                Tcl_AddErrorInfo(interp, error);
            }
            return retCode;
        }
    }

    /* File exists - evaluate from the file itself */
    return Tcl_EvalFile(interp, argv[1]);
}


/********************************************************************************************
 * test_File
 * purpose : This function replaces the "file" command of the TCL, to ensure that
 *           when checking if a file exists, we also look inside our buffers.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_File(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int i, retCode;
    Tcl_DString str;

    if ((argc == 3) && (strncmp(argv[1], "exis", 4)) == 0)
    {
        /* "file exist" command - overloaded... */
        if (tclGetFile(argv[2]) != NULL)
        {
            Tcl_SetResult(interp, (char *)"1", TCL_STATIC);
            return TCL_OK;
        }
    }

    /* Continue executing the real "file" command */
    Tcl_DStringInit(&str);
    Tcl_DStringAppendElement(&str, "fileOverloaded");
    for(i = 1; i < argc; i++)
        Tcl_DStringAppendElement(&str, argv[i]);
    retCode = Tcl_Eval(interp, Tcl_DStringValue(&str));
    Tcl_DStringFree(&str);
    return retCode;
}


/********************************************************************************************
 * test_Quit
 * purpose : This function is called when the test application is closed from the GUI.
 *           It is responsible for closing the application gracefully.
 * syntax  : test.Quit
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Quit(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    TclExecute("init:SaveOptions 0");

    /* We should kill the stack only after we save the application's options */
    EndStack();

    /* Finish with the interperter */
    Tcl_DeleteInterp(interp);
    exit(0);
    return TCL_OK;
}



/********************************************************************************************
 * test_Restart
 * purpose : This function restarts the stack while in the middle of execution.
 * syntax  : test.Restart
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Restart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    EndStack();
    hApp = NULL;

    TclExecute("after 5000 test.Init");

    return TCL_OK;
}


/********************************************************************************************
 * test_Init
 * purpose : This function initialized the stack as part of the restart operation.
 *           It should not be called from TCL scripts, but from test_Restart only
 * syntax  : test.Restart
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Init(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    if (InitStack() < 0)
    {
        TclExecute("msgbox Error picExclamation {Unable to restart the stack. Stopping the application} { { Ok -1 <Key-Return> } }");
        TclExecute("test.Quit");
    }

    TclExecute("test:updateGui");
    TclExecute("msgbox Restart picInformation {Stack restarted successfully} { { Ok -1 <Key-Return> } }");

    return TCL_OK;
}


/********************************************************************************************
 * test_Support
 * purpose : This function check for support of specific stack functionality
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Support(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    if (strcmp(argv[0], "test.RtpSupported") == 0)
        Tcl_SetResult(interp, BooleanStr(RTP_IsInitialized()), TCL_STATIC);
    else if (strcmp(argv[0], "test.SnmpSupported") == 0)
        Tcl_SetResult(interp, BooleanStr(SNMP_IsInitialized()), TCL_STATIC);
    else if (strcmp(argv[0], "test.SecSupported") == 0)
        Tcl_SetResult(interp, BooleanStr(SEC_IsInitialized()), TCL_STATIC);
    else if (strcmp(argv[0], "test.H450Supported") == 0)
        Tcl_SetResult(interp, BooleanStr(H450_IsInitialized()), TCL_STATIC);

    return TCL_OK;
}


/********************************************************************************************
 * setIcon
 * purpose : Set the icon for the Windows test application
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int setIcon(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
#ifdef WIN32
    HWND        hWnd;
    HINSTANCE   hInstance;
    HICON       hIcon;

    if (sscanf(argv[1], "0x%p", (UINTPTR *)&hWnd) == 1)
    {
        hInstance = GetModuleHandle(NULL);
        hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RVSN_LOGO));
        SetClassLong(hWnd, GCL_HICONSM, (LONG)hIcon);
        SetClassLong(hWnd, GCL_HICON, (LONG)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

#endif  /* WIN32 */
    return TCL_OK;
}








#define WRAPPER_COMMAND(tclName, cName) \
    Tcl_CreateCommand(interp, (char *)tclName, WrapperFunc, (ClientData)cName, (Tcl_CmdDeleteProc *)NULL)

#define CREATE_COMMAND(tclName, cName) \
    Tcl_CreateCommand(interp, (char *)tclName, cName, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL)


/********************************************************************************************
 * CreateTclCommands
 * purpose : Create all tcl commands that was written in c language.
 * input   : interp - interpreter for tcl commands
 * output  : none
 * return  : none
 ********************************************************************************************/
void CreateTclCommands(Tcl_Interp* interp)
{
    /******************
     * CALLS: Creation
     ******************/
    CREATE_COMMAND("Call.Dial", Call_Dial);
    CREATE_COMMAND("Call.Make", Call_Make);
    CREATE_COMMAND("Call.FastStartDial", Call_FastStartDial);
    CREATE_COMMAND("Call.FastStartMake", Call_FastStartMake);
    CREATE_COMMAND("Call.Accept", Call_Accept);
    CREATE_COMMAND("Call.Drop", Call_Drop);
    CREATE_COMMAND("Call.SendAlerting", Call_SendAlerting);
    CREATE_COMMAND("Call.SendCallProceeding", Call_SendCallProceeding);
    CREATE_COMMAND("Call.SendConnect", Call_SendConnect);

    /*******************
     * CALLS: Faststart
     *******************/
    CREATE_COMMAND("Call.OpenOutgoingFaststart", Call_OpenOutgoingFaststart);
    CREATE_COMMAND("Call.BuildOutgoingFaststart", Call_BuildOutgoingFaststart);
    CREATE_COMMAND("Call.ApproveFaststart", Call_ApproveFaststart);
    CREATE_COMMAND("Call.RefuseFaststart", Call_RefuseFaststart);
    CREATE_COMMAND("Call.ConnectControl", Call_ConnectControl);

    /**************
     * CALLS: Misc
     **************/
    CREATE_COMMAND("Call.SetRate", Call_SetRate);
    CREATE_COMMAND("Call.SendStatusInquiry", Call_SendStatusInquiry);
    CREATE_COMMAND("Call.SendProgress", Call_SendProgress);
    CREATE_COMMAND("Call.SendNotify", Call_SendNotify);
    CREATE_COMMAND("Call.SendUserInformation", Call_SendUserInformation);
    CREATE_COMMAND("Call.SendFacility", Call_SendFacility);
    CREATE_COMMAND("Call.SeperateControl", Call_SeperateControl);
    CREATE_COMMAND("Call.CloseControl", Call_CloseControl);
    CREATE_COMMAND("Call.SendCaps", Call_SendCaps);
    CREATE_COMMAND("Call.SendEmptyCaps", Call_SendEmptyCaps);
    CREATE_COMMAND("Call.SendCapsAck", Call_SendCapsAck);
    CREATE_COMMAND("Call.SendMSD", Call_SendMSD);
    CREATE_COMMAND("Call.CreateH245", Call_CreateH245);
    CREATE_COMMAND("Call.UII", Call_UII);
    CREATE_COMMAND("Call.LoopOff", Call_LoopOff);
    CREATE_COMMAND("Multiplex.Update", Multiplex_Update);
    CREATE_COMMAND("RAS.SendBRQ", RAS_SendBRQ);
    CREATE_COMMAND("RAS.SendNSM", RAS_SendNSM);
    CREATE_COMMAND("RAS.SendRAI", RAS_SendRAI);

    /*************************
     * CALLS: Overlap sending
     *************************/
    CREATE_COMMAND("Call.IncompleteAddress", Call_IncompleteAddress);
    CREATE_COMMAND("Call.AddressComplete", Call_AddressComplete);
    CREATE_COMMAND("Call.SendAdditionalAddr", Call_SendAdditionalAddr);

    /*********************
     * CALLS: GUI Display
     *********************/
    CREATE_COMMAND("Call.GetWindowHandle", Call_GetWindowHandle);
    CREATE_COMMAND("Call.DisplayInfo", Call_DisplayInfo);

    /***********
     * CHANNELS
     ***********/
    CREATE_COMMAND("Channel.ConnectOutgoing", Channel_ConnectOutgoing);
    CREATE_COMMAND("Channel.OpenOutgoingWindow", Channel_OpenOutgoingWindow);
    CREATE_COMMAND("Channel.DisplayChannelList", Channel_DisplayChannelList);
    CREATE_COMMAND("Channel.ResponseForOLC", Channel_ResponseForOLC);
    CREATE_COMMAND("Channel.ResponseForCLC", Channel_ResponseForCLC);
    CREATE_COMMAND("Channel.Drop", Channel_Drop);
    CREATE_COMMAND("Channel.Answer", Channel_Answer);
    CREATE_COMMAND("Channel.MediaLoop", Channel_MediaLoop);
    CREATE_COMMAND("Channel.Rate", Channel_Rate);
    
    /********
     * TOOLS
     ********/
    CREATE_COMMAND("test.Quit", test_Quit);
    CREATE_COMMAND("test.Restart", test_Restart);
    CREATE_COMMAND("test.Init", test_Init);
    CREATE_COMMAND("test.RtpSupported", test_Support);
    CREATE_COMMAND("test.SnmpSupported", test_Support);
    CREATE_COMMAND("test.SecSupported", test_Support);
    CREATE_COMMAND("test.H450Supported", test_Support);
#if 0
    CREATE_COMMAND("Log.FetchFilter", Log_FetchFilter);
    CREATE_COMMAND("Log.SetFilter", Log_SetFilter);
#endif
    CREATE_COMMAND("Options.GetLocalIP", Options_GetLocalIP);
    CREATE_COMMAND("Status.Display", Status_Display);
    CREATE_COMMAND("setIcon", setIcon);

    /******
     * H450
     ******/
    CREATE_COMMAND("H450.callTransfer", H450_callTransfer);
    CREATE_COMMAND("H450.callReroute", H450_callReroute);
    CREATE_COMMAND("H450.forwardActivate", H450_forwardActivate);
    CREATE_COMMAND("H450.forwardDeactivate", H450_forwardDeactivate);
    CREATE_COMMAND("H450.forwardInterrogate", H450_forwardInterrogate);
    CREATE_COMMAND("H450.callHold", H450_callHold);
    CREATE_COMMAND("H450.callHoldRtrv", H450_callHoldRtrv);
    CREATE_COMMAND("H450.HoldSendNonDefaultResponse", H450_HoldSendNonDefaultResponse);
    CREATE_COMMAND("H450.callWait", H450_callWait);
    CREATE_COMMAND("H450.MC.ActivateMessage", H450_MC_ActivateMessage);
    CREATE_COMMAND("H450.MC.ActivateCallBack", H450_MC_ActivateCallBack);
    CREATE_COMMAND("H450.MC.Deactivate", H450_MC_Deactivate);
    CREATE_COMMAND("H450.SU.Interogate", H450_SU_Interogate);
    CREATE_COMMAND("H450.MWISendNonDefaultResponse", H450_MWISendNonDefaultResponse);
    CREATE_COMMAND("H450.callCompletion", H450_callCompletion);
    CREATE_COMMAND("H450.callCompletionAction", H450_callCompletionAction);
    CREATE_COMMAND("H450.callOffer", H450_callOffer);
    CREATE_COMMAND("H450.remoteUserAlerting", H450_remoteUserAlerting);
    CREATE_COMMAND("H450.callIntrusion", H450_callIntrusion);
    CREATE_COMMAND("H450.getCallID", H450_getCallID);
    CREATE_COMMAND("H450.callPark", H450_callPark);
    CREATE_COMMAND("H450.callPick", H450_callPick);
    
    /**********
     * Security
     **********/

    CREATE_COMMAND("SEC.notifyModeChange", SEC_notifyModeChange);

    /************************
     * WRAPPER API FUNCTIONS
     ************************/
    Tcl_CreateCommand(interp, (char *)"script:cb", CallbackFunc, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    WRAPPER_COMMAND("api:cm:CallNew", api_cm_CallNew);
    WRAPPER_COMMAND("api:cm:CallDial", api_cm_CallDial);
    WRAPPER_COMMAND("api:cm:CallProceeding", api_cm_CallProceeding);
    WRAPPER_COMMAND("api:cm:CallAccept", api_cm_CallAccept);
    WRAPPER_COMMAND("api:cm:CallDrop", api_cm_CallDrop);
    WRAPPER_COMMAND("api:cm:CallAccept", api_cm_CallAccept);
    WRAPPER_COMMAND("api:cm:CallAnswer", api_cm_CallAnswer);
    WRAPPER_COMMAND("api:cm:CallClose", api_cm_CallClose);
    WRAPPER_COMMAND("api:cm:CallSetParam", api_cm_CallSetParam);
    WRAPPER_COMMAND("api:cm:CallGetParam", api_cm_CallGetParam);
    WRAPPER_COMMAND("api:cm:CallForward", api_cm_CallForward);
    WRAPPER_COMMAND("api:cm:CallStatusInquiry", api_cm_CallStatusInquiry);
    WRAPPER_COMMAND("api:cm:CallConnectControl", api_cm_CallConnectControl);
    WRAPPER_COMMAND("api:cm:FastStartBuild", api_cm_FastStartBuild);
    WRAPPER_COMMAND("api:cm:CallAddFastStartMessage", api_cm_CallAddFastStartMessage);
    WRAPPER_COMMAND("api:cm:FastStartChannelsAck", api_cm_FastStartChannelsAck);
    WRAPPER_COMMAND("api:cm:FastStartChannelsAckIndex", api_cm_FastStartChannelsAckIndex);
    WRAPPER_COMMAND("api:cm:FastStartChannelsReady", api_cm_FastStartChannelsReady);
    WRAPPER_COMMAND("api:cm:CallSendSupplementaryService", api_cm_CallSendSupplementaryService);
    WRAPPER_COMMAND("api:cm:CallSimulateMessage", api_cm_CallSimulateMessage);

    WRAPPER_COMMAND("api:cm:ChannelNew", api_cm_ChannelNew);
    WRAPPER_COMMAND("api:cm:ChannelOpen", api_cm_ChannelOpen);
    WRAPPER_COMMAND("api:cm:ChannelAnswer", api_cm_ChannelAnswer);
    WRAPPER_COMMAND("api:cm:ChannelDrop", api_cm_ChannelDrop);
    WRAPPER_COMMAND("api:cm:ChannelRequestCloseReject", api_cm_ChannelRequestCloseReject);
    WRAPPER_COMMAND("api:cm:ChannelClose", api_cm_ChannelClose);
    WRAPPER_COMMAND("api:cm:ChannelGetCallHandle", api_cm_ChannelGetCallHandle);
    WRAPPER_COMMAND("api:cm:ChannelGetDependency", api_cm_ChannelGetDependency);
    WRAPPER_COMMAND("api:cm:ChannelGetDuplexAddress", api_cm_ChannelGetDuplexAddress);
    WRAPPER_COMMAND("api:cm:ChannelGetNumber", api_cm_ChannelGetNumber);
    WRAPPER_COMMAND("api:cm:ChannelGetOrigin", api_cm_ChannelGetOrigin);
    WRAPPER_COMMAND("api:cm:ChannelGetSource", api_cm_ChannelGetSource);
    WRAPPER_COMMAND("api:cm:ChannelGetIsDuplex", api_cm_ChannelIsDuplex);
    WRAPPER_COMMAND("api:cm:H245DeleteCapabilityMessage", api_cm_H245DeleteCapabilityMessage);

    /* Automatic RAS */
    WRAPPER_COMMAND("api:cm:Register", api_cm_Register);
    WRAPPER_COMMAND("api:cm:Unregister", api_cm_Unregister);

    /* Manual RAS */
    WRAPPER_COMMAND("api:cmras:StartTransaction", api_cmras_StartTransaction);
    WRAPPER_COMMAND("api:cmras:SetParam", api_cmras_SetParam);
    WRAPPER_COMMAND("api:cmras:GetParam", api_cmras_GetParam);
    WRAPPER_COMMAND("api:cmras:GetNumOfParams", api_cmras_GetNumOfParams);
    WRAPPER_COMMAND("api:cmras:Request", api_cmras_Request);
    WRAPPER_COMMAND("api:cmras:DummyRequest", api_cmras_DummyRequest);
    WRAPPER_COMMAND("api:cmras:Confirm", api_cmras_Confirm);
    WRAPPER_COMMAND("api:cmras:Reject", api_cmras_Reject);
    WRAPPER_COMMAND("api:cmras:InProgress", api_cmras_InProgress);
    WRAPPER_COMMAND("api:cmras:Close", api_cmras_Close);
    WRAPPER_COMMAND("api:cmras:GetTransaction", api_cmras_GetTransaction);

    /* General functions */
    WRAPPER_COMMAND("api:cm:Start", api_cm_Start);
    WRAPPER_COMMAND("api:cm:Stop", api_cm_Stop);
    WRAPPER_COMMAND("api:cm:GetVersion", api_cm_GetVersion);
    WRAPPER_COMMAND("api:cm:Vt2Alias", api_cm_Vt2Alias);
    WRAPPER_COMMAND("api:cm:Alias2Vt", api_cm_Alias2Vt);
    WRAPPER_COMMAND("api:cm:GetProperty", api_cm_GetProperty);
    WRAPPER_COMMAND("api:cm:GetValTree", api_cm_GetValTree);
    WRAPPER_COMMAND("api:cm:GetRASConfigurationHandle", api_cm_GetRASConfigurationHandle);
    WRAPPER_COMMAND("api:cm:GetLocalRASAddress", api_cm_GetLocalRASAddress);
    WRAPPER_COMMAND("api:cm:GetLocalCallSignalAddress", api_cm_GetLocalCallSignalAddress);
    WRAPPER_COMMAND("api:cm:GetLocalAnnexEAddress", api_cm_GetLocalAnnexEAddress);
    WRAPPER_COMMAND("api:cm:LogMessage", api_cm_LogMessage);
    WRAPPER_COMMAND("api:cm:Encode", api_cm_Encode);
    WRAPPER_COMMAND("api:cm:Decode", api_cm_Decode);

    /* PVT functions */
    WRAPPER_COMMAND("api:pvt:AddRoot", api_pvt_AddRoot);
    WRAPPER_COMMAND("api:pvt:GetByPath", api_pvt_GetByPath);
    WRAPPER_COMMAND("api:pvt:BuildByPath", api_pvt_BuildByPath);
    WRAPPER_COMMAND("api:pvt:Delete", api_pvt_Delete);
    WRAPPER_COMMAND("api:pvt:GetString", api_pvt_GetString);
    WRAPPER_COMMAND("api:pvt:Print", api_pvt_Print);

    /* Application functions */
    WRAPPER_COMMAND("api:app:SetCallMode", api_app_SetCallMode);
    WRAPPER_COMMAND("api:app:SetChannelMode", api_app_SetChannelMode);
    WRAPPER_COMMAND("api:app:GetDataTypes", api_app_GetDataTypes);
    WRAPPER_COMMAND("api:app:LoadMessage", api_app_LoadMessage);
    WRAPPER_COMMAND("api:app:ChannelKill", api_app_ChannelKill);
    WRAPPER_COMMAND("api:app:GetChannelList", api_app_GetChannelList);
    WRAPPER_COMMAND("api:app:Vt2Address", api_app_Vt2Address);
    WRAPPER_COMMAND("api:app:Address2Vt", api_app_Address2Vt);
    WRAPPER_COMMAND("api:app:FSGetChanNum", api_app_FSGetChanNum);
    WRAPPER_COMMAND("api:app:FSGetAltChanNum", api_app_FSGetAltChanNum);
    WRAPPER_COMMAND("api:app:FSGetChanIndex", api_app_FSGetChanIndex);
    WRAPPER_COMMAND("api:app:FSGetChanName", api_app_FSGetChanName);
    WRAPPER_COMMAND("api:app:FSGetChanRTCP", api_app_FSGetChanRTCP);
    WRAPPER_COMMAND("api:app:FSGetChanRTP", api_app_FSGetChanRTP);

    /* Annexes */
    WRAPPER_COMMAND("api:app:CallCreateAnnexMMessage", api_cm_CallCreateAnnexMMessage);
    WRAPPER_COMMAND("api:app:CallSendAnnexMMessage", api_cm_CallSendAnnexMMessage);
    WRAPPER_COMMAND("api:app:CallCreateAnnexLMessage", api_cm_CallCreateAnnexLMessage);
    WRAPPER_COMMAND("api:app:CallSendAnnexLMessage", api_cm_CallSendAnnexLMessage);
}






/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * PutError
 * purpose : Notify the user about errors that occured
 * input   : title  - Title of the error
 *           reason - Reason that caused the error
 * output  : none
 * return  : none
 ********************************************************************************************/
void PutError(const char* title, const char* reason)
{
    static BOOL TAP_inError = FALSE;
    if ((reason == NULL) || (strlen(reason) == 0))
        reason = "Undefined error was encountered";

    fprintf(stderr, "%s: %s\n", title, reason);

    /* Make sure we don't get into an endless loop over this one */
    if (TAP_inError) return;
    TAP_inError = TRUE;

#if defined (WIN32) && defined (_WINDOWS)
    if (interp == NULL)
        MessageBox(NULL, reason, title, MB_OK | MB_ICONERROR);
#endif  /* WIN32 */

    if (interp != NULL)
    {
        TclExecute("Log:Write {%s: %s}", title, reason);
        TclExecute("update;msgbox {%s} picExclamation {%s} {{Ok -1 <Key-Return>}}", title, reason);
    }

    TAP_inError = FALSE;
}


/********************************************************************************************
 * InitTcl
 * purpose : Initialize the TCL part of the test application
 * input   : executable     - Program executable
 *           versionString  - Stack version string
 * output  : reason         - Reason of failure on failure
 * return  : Tcl_Interp interpreter for tcl commands
 *           NULL on failure
 ********************************************************************************************/
Tcl_Interp* InitTcl(const char* executable, char* versionString, char** reason)
{
    static char strBuf[1024];
    int retCode;

    /* Find TCL executable and create an interpreter */
    Tcl_FindExecutable(executable);
    interp = Tcl_CreateInterp();

    if (interp == NULL)
    {
        *reason = (char*)"Failed to create Tcl interpreter";
        return NULL;
    }

    /* Overload file and source commands */
    TclExecute("rename file fileOverloaded");
    CREATE_COMMAND("file", test_File);
    CREATE_COMMAND("source", test_Source);

    /* Reroute tcl libraries - we'll need this one later */
    /*TclSetVariable("tcl_library", TCL_LIBPATH);
    TclSetVariable("env(TCL_LIBRARY)", TCL_LIBPATH);
    TclSetVariable("tk_library", TK_LIBPATH);
    TclSetVariable("env(TK_LIBRARY)", TK_LIBPATH);*/

    /* Initialize TCL */
    retCode = Tcl_Init(interp);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error in Tcl_Init: %s", Tcl_GetStringResult(interp));
        *reason = strBuf;
        Tcl_DeleteInterp(interp);
        return NULL;
    }

    /* Initialize TK */
    retCode = Tk_Init(interp);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error in Tk_Init: %s", Tcl_GetStringResult(interp));
        *reason = strBuf;
        Tcl_DeleteInterp(interp);
        return NULL;
    }

    /* Set argc and argv parameters for the script.
       This allows us to work with C in the scripts. */
    retCode = TclExecute("set tmp(version) {Test Application: %s }", versionString);
    if (retCode != TCL_OK)
    {
        *reason = (char*)"Error setting stack's version for test application";
        return interp;
    }

    /* Create new commands that are used in the tcl script */
    CreateTclCommands(interp);

    Tcl_LinkVar(interp, (char *)"scriptLogs", (char *)&LogWrappers, TCL_LINK_BOOLEAN);

    /* Evaluate the Tcl script of the test application */
    retCode = Tcl_Eval(interp, (char*)"source " TCL_FILENAME);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error reading testapp script (line %d): %s\n", interp->errorLine, Tcl_GetStringResult(interp));
        *reason = strBuf;
        return NULL;
    }

    /* Return the created interpreter */
    *reason = NULL;
    return interp;
}


/********************************************************************************************
 * InitStack
 * purpose : Initialize the H.323 stack
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitStack(void)
{
    int status;
    SCMEVENT                        cmEvent;
    SCMCALLEVENT                    cmCallEvent;
    SCMCHANEVENT                    cmChanEvent;
    SCMCONTROLEVENT                 cmControlEvent;
    SCMPROTOCOLEVENT                cmProtocolEvent;
    SCMRASEVENT                     cmRasEvent;
    SCMAUTORASEVENT                 cmAutoRasEvent;

    cmStartUp();

    /* Initialize the stack */
    status = cmInitialize((char *)CONFIG_FILE,&hApp);
    if (status < 0) return status;

    /* Initialize supserv (does nothing if not h450 enabled) */
    status = H450_init((char *)CONFIG_FILE,&hApp);
    if (status < 0) return status;

    /* Initialize rtp/rtcp (does nothing if not enabled) */
    RTP_TestInit();

    memset(&cmEvent, 0, sizeof(SCMEVENT));
    cmEvent.cmEvNewCall = cmEvNewCall;
    cmEvent.cmEvRegEvent = cmEvRegEvent;
    cmSetGenEventHandler(hApp,&cmEvent,sizeof(SCMEVENT));

    memset (&cmCallEvent,0,sizeof(SCMCALLEVENT));
    cmCallEvent.cmCallNonStandardParam = NULL; /* Obsolete. Stack never calls it */
    cmCallEvent.cmEvCallAdditionalAddress = cmEvCallAdditionalAddress;
    cmCallEvent.cmEvCallFacility = cmEvCallFacility;
    cmCallEvent.cmEvCallFastStart = cmEvCallFastStart;
    cmCallEvent.cmEvCallFastStartSetup = cmEvCallFastStartSetup;
    cmCallEvent.cmEvCallH450SupplServ = cmEvCallH450SupplServ;/* does nothing if h450 not enabled */
    cmCallEvent.cmEvCallIncompleteAddress = cmEvCallIncompleteAddress;
    cmCallEvent.cmEvCallInfo = cmEvCallInfo;
    cmCallEvent.cmEvCallNewAnnexLMessage = cmEvCallNewAnnexLMessage;
    cmCallEvent.cmEvCallNewAnnexMMessage = cmEvCallNewAnnexMMessage;
    cmCallEvent.cmEvCallNewRate = cmEvCallNewRate;
    cmCallEvent.cmEvCallNotify = cmEvCallNotify;
    cmCallEvent.cmEvCallProgress = cmEvCallProgress;
    cmCallEvent.cmEvCallRecvMessage = cmEvCallRecvMessage;
    cmCallEvent.cmEvCallSendMessage = cmEvCallSendMessage;
    cmCallEvent.cmEvCallStateChanged = cmEvCallStateChanged;
    cmCallEvent.cmEvCallStatus = cmEvCallStatus;
    cmCallEvent.cmEvCallTunnNewMessage = cmEvCallTunnNewMessage;
    cmCallEvent.cmEvCallUserInfo = cmEvCallUserInfo;
    cmSetCallEventHandler(hApp,&cmCallEvent,sizeof(SCMCALLEVENT));

    memset (&cmChanEvent,0,sizeof(SCMCHANEVENT));
    cmChanEvent.cmEvChannelStateChanged             = cmEvChannelStateChanged;
    cmChanEvent.cmEvChannelNewRate                  = cmEvChannelNewRate;
    cmChanEvent.cmEvChannelMaxSkew                  = cmEvChannelMaxSkew;
    cmChanEvent.cmEvChannelSetAddress               = cmEvChannelSetAddress;
    cmChanEvent.cmEvChannelSetRTCPAddress           = cmEvChannelSetRTCPAddress;
    cmChanEvent.cmEvChannelParameters               = cmEvChannelParameters;
    cmChanEvent.cmEvChannelRTPDynamicPayloadType    = cmEvChannelRTPDynamicPayloadType;
    cmChanEvent.cmEvChannelVideoFastUpdatePicture   = cmEvChannelVideoFastUpdatePicture;
    cmChanEvent.cmEvChannelVideoFastUpdateGOB       = cmEvChannelVideoFastUpdateGOB;
    cmChanEvent.cmEvChannelVideoFastUpdateMB        = cmEvChannelVideoFastUpdateMB;
    cmChanEvent.cmEvChannelHandle                   = cmEvChannelHandle;
    cmChanEvent.cmEvChannelGetRTCPAddress           = cmEvChannelGetRTCPAddress;
    cmChanEvent.cmEvChannelRequestCloseStatus       = cmEvChannelRequestCloseStatus;
    cmChanEvent.cmEvChannelTSTO                     = cmEvChannelTSTO;
    cmChanEvent.cmEvChannelMediaLoopStatus          = cmEvChannelMediaLoopStatus;
    cmChanEvent.cmEvChannelReplace                  = cmEvChannelReplace;
    cmChanEvent.cmEvChannelFlowControlToZero        = cmEvChannelFlowControlToZero;
    cmChanEvent.cmEvChannelMiscCommand              = cmEvChannelMiscCommand;
    cmChanEvent.cmEvChannelTransportCapInd          = cmEvChannelTransportCapInd;
    cmChanEvent.cmEvChannelSetNSAPAddress           = cmEvChannelSetNSAPAddress;
    cmChanEvent.cmEvChannelSetATMVC                 = cmEvChannelSetATMVC;
    cmChanEvent.cmEvChannelRecvMessage              = cmEvChannelRecvMessage;
    cmChanEvent.cmEvChannelSendMessage              = cmEvChannelSendMessage;
    cmSetChannelEventHandler(hApp,&cmChanEvent,sizeof(SCMCHANEVENT));

    memset (&cmControlEvent,0,sizeof(SCMCONTROLEVENT));
    cmControlEvent.cmEvCallCapabilities             = cmEvCallCapabilities;
    cmControlEvent.cmEvCallCapabilitiesExt          = cmEvCallCapabilitiesExt;
    cmControlEvent.cmEvCallNewChannel               = cmEvCallNewChannel;
    cmControlEvent.cmEvCallCapabilitiesResponse     = cmEvCallCapabilitiesResponse;
    cmControlEvent.cmEvCallMasterSlaveStatus        = cmEvCallMasterSlaveStatus;
    cmControlEvent.cmEvCallRoundTripDelay           = cmEvCallRoundTripDelay;
    cmControlEvent.cmEvCallUserInput                = cmEvCallUserInput;
    cmControlEvent.cmEvCallRequestMode              = cmEvCallRequestMode;
    cmControlEvent.cmEvCallMiscStatus               = cmEvCallMiscStatus;
    cmControlEvent.cmEvCallControlStateChanged      = cmEvCallControlStateChanged;
    cmControlEvent.cmEvCallMasterSlave              = cmEvCallMasterSlave;
    cmSetControlEventHandler(hApp,&cmControlEvent,sizeof(SCMCONTROLEVENT));

    memset (&cmProtocolEvent,0,sizeof(SCMPROTOCOLEVENT));
    cmProtocolEvent.hookRecvFrom = hookRecvFrom;
    cmProtocolEvent.hookSendTo = hookSendTo;
    cmSetProtocolEventHandler(hApp,&cmProtocolEvent,sizeof(SCMPROTOCOLEVENT));

    memset(&cmRasEvent, 0, sizeof(SCMRASEVENT));
    cmRasEvent.cmEvRASConfirm = cmEvRASConfirm;
    cmRasEvent.cmEvRASReject = cmEvRASReject;
    cmRasEvent.cmEvRASRequest = cmEvRASRequest;
    cmRasEvent.cmEvRASTimeout = cmEvRASTimeout;
    cmRASSetEventHandler(hApp, &cmRasEvent, sizeof(SCMRASEVENT));

    memset(&cmAutoRasEvent, 0, sizeof(SCMAUTORASEVENT));
    cmAutoRasEvent.cmEvAutoRASConfirm = cmEvAutoRASConfirm;
    cmAutoRasEvent.cmEvAutoRASReject = cmEvAutoRASReject;
    cmAutoRasEvent.cmEvAutoRASRequest = cmEvAutoRASRequest;
    cmAutoRasEvent.cmEvAutoRASTimeout = cmEvAutoRASTimeout;
    cmAutoRASSetEventHandler(hApp, &cmAutoRasEvent, sizeof(SCMAUTORASEVENT));

    cmStart(hApp);
    if (interp != NULL)
        TclExecute("test:SetStackStatus Running");

    /* Used for host color randomization */
    srand(time(NULL));    

    return 0;
}

/********************************************************************************************
 * EndStack
 * purpose : End the H.323 stack
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void EndStack(void)
{
    TclExecute("Window close .status");

    SNMP_end();
    RTP_TestEnd();
    if (hApp != NULL) cmEnd(hApp);
    H450_end();

    TclExecute("test:SetStackStatus Finished");
}



/********************************************************************************************
 * InitApp
 * purpose : Initialize the test application
 *           This includes parts as RTP/RTCP support, etc.
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitApp(void)
{
    /* Initialize security (does nothing if not security enabled) */
    SEC_init(hApp);

    /* Initialize MIB support */
    SNMP_init();

    memset(&NumAllocations, 0, sizeof(NumAllocations));
    memset(&SizeAllocations, 0, sizeof(SizeAllocations));

    TclExecute("test:updateGui");

    return 0;
}



#ifdef __cplusplus
}
#endif

