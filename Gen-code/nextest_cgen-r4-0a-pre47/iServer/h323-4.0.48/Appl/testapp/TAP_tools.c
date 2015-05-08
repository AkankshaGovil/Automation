

/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_tools.c
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


#ifdef __cplusplus
extern "C" {
#endif

#include <cmsize.h>
#include <pvaltree.h>
#include "TAP_init.h"
#include "TAP_general.h"
#include "TAP_h450.h"
#include "TAP_tools.h"


/* Log Hooks
 ********************************/


BOOL RVCALLCONV hookSendTo(IN      HPROTCONN           hConn,
                           IN      int                 nodeId,
                           IN      int                 nodeIdTo,
                           IN      BOOL                error)
{
    char text[128];

    /* format message */
    sprintf(text,"New message sent -> %s on %s",
        cmGetProtocolMessageName(hApp,nodeId),
        cmProtocolGetProtocolName(cmProtocolGetProtocol(hApp,nodeId)));
    /* print to log */
    TclExecute("test:Log {%s}", text);
    return 0;
}


BOOL RVCALLCONV hookRecvFrom(IN      HPROTCONN           hConn,
                             IN      int                 nodeId,
                             IN      int                 nodeIdFrom,
                             IN      BOOL                multicast,
                             IN      BOOL                error)
{
    char text[128];

    /* format message */
    sprintf(text,"New message recv <- %s on %s",
        nprn(cmGetProtocolMessageName(hApp,nodeId)),
        nprn(cmProtocolGetProtocolName(cmProtocolGetProtocol(hApp,nodeId))));
    /* print to log */
    TclExecute("test:Log {%s}", text);
    return 0;
}

int RVCALLCONV cmEvCallRecvMessage(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN int      message)
{
    char text[128];
    CallInfo * Call = (CallInfo *) haCall;
    /* format message */
    sprintf(text,"New message recv <- %s on %s",
        nprn(cmGetProtocolMessageName(hApp, message)),
        nprn(cmProtocolGetProtocolName(cmProtocolGetProtocol(hApp, message))));

    if (Call)
        /* print to call log */
        TclExecute("call:Log %d {%s}", Call->counter, text);
    else
        /* print to log */
        TclExecute("test:Log {%s}", text);
    return 0;
}

int RVCALLCONV cmEvCallSendMessage(IN HAPPCALL haCall,
                                   IN HCALL    hsCall,
                                   IN int      message)
{
    char text[128];
    char * messageStr = cmGetProtocolMessageName(hApp, message);
    CallInfo * Call = (CallInfo *) haCall;
    /* format message */
    sprintf(text,"New message sent -> %s on %s",
        nprn(messageStr), nprn(cmProtocolGetProtocolName(cmProtocolGetProtocol(hApp, message))));
    
    if (Call)
        /* print to call log */
        TclExecute("call:Log %d {%s}", Call->counter, text);
    else
        /* print to log */
        TclExecute("test:Log {%s}", text);
    
    return 0;
}




/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/

#if 0
/********************************************************************************************
 * LogMessage
 * purpose : Internal callback used to log a message into the log window of the TCL
 * input   : context - Context given by the application when sink was created
 *           hLog    - Log handle of the source notifying this message
 *           mType   - Type of message printed
 *           line    - String to print
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVCALLCONV LogMessage(
    IN void*            context,
    IN RVHLOG           hLog,
    IN logMessageType   mType,
    IN char*            line)
{
    /* Log any non-info messages to the screen logger as well */
    if ((mType != RV_INFO) && (mType != RV_DEBUG))
    {
        BOOL finished = FALSE;

        /* Removed any problematic characters */
        char* p = (char *)line;
        while (!finished)
        {
            switch (*p)
            {
                case '\0':  finished = TRUE; break;
                case '\'':
                case '"':
                case '[':
                case ']':
                case '{':
                case '}':
                case ';':   *p = '.'; break;
                default:    break;
            }
            p++;
        }

        TclExecute("Log:Insert {%s}", line);
    }
}






/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/

/********************************************************************************************
 * Log_Init
 * purpose : Initialize the application's log sink
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void Log_Init(void)
{
    RVHLOGMGR hLogMgr;
    logApplicationSinkStruct sinkData;

    sinkData.context = NULL;
    sinkData.printFunction = LogMessage;

    hLogMgr = cmGetLogMgr(hApp);
    logAddSink(hLogMgr, logSinkApplication, &sinkData);
}


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
int Log_SetFilter(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RVHLOGMGR hLogMgr;
    RVHLOG hLog;
    char c = *argv[1];
    logMessageType messageType;
    BOOL global;

    /* Check if we're setting a global one */
    global = (strcmp(argv[3], "all") == 0);

    switch (c)
    {
        case('W'): messageType = RV_WARN; break;
        case('I'): messageType = RV_INFO; break;
        case('F'): messageType = RV_ERROR; break;
        case('B'): messageType = RV_EXCEP; break;
        case('D'): messageType = RV_DEBUG; break;
        default:   messageType = RV_DEBUG;
    }

    hLogMgr = cmGetLogMgr(hApp);

    if (!global)
    {
        hLog = logRegister(hLogMgr, argv[3], "");
        logSetSelection(hLog, logSinkAny, messageType, (*argv[2] == '1'));
    }
    else
    {
        logSetGlobalSelection(hLogMgr, logSinkAny, messageType, (*argv[2] == '1'));
    }

    return TCL_OK;
}


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
int Log_FetchFilter(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    static const char* isSelected[] = {"deselect", "select"};
    RVHLOG hLog;

    /* Check if we're getting a global one - in this case, we'll set all of them to deselected... */
    if (strcmp(argv[1], "all") == 0)
    {
        return TclExecute(".log.main.filters.detailed deselect\n"
                          ".log.main.filters.inf deselect\n"
                          ".log.main.filters.fatal deselect\n"
                          ".log.main.filters.bug deselect\n"
                          ".log.main.filters.warning deselect");
    }

    /* Specific one - handle it */
    hLog = logRegister(cmGetLogMgr(hApp), argv[1], "");
    TclExecute(".log.main.filters.detailed %s\n"
               ".log.main.filters.inf %s\n"
               ".log.main.filters.fatal %s\n"
               ".log.main.filters.bug %s\n"
               ".log.main.filters.warning %s",
               isSelected[logIsSelected(hLog, RV_DEBUG)],
               isSelected[logIsSelected(hLog, RV_INFO)],
               isSelected[logIsSelected(hLog, RV_ERROR)],
               isSelected[logIsSelected(hLog, RV_EXCEP)],
               isSelected[logIsSelected(hLog, RV_WARN)]);
    return TCL_OK;
}
#endif


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
int Options_GetLocalIP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int result;
    char my_ip_addr[20];
    cmTransportAddress transportAddress;

    result = cmGetLocalCallSignalAddress(hApp, &transportAddress);
    if (result >= 0)
    {
        IpToString(transportAddress.ip , my_ip_addr);
        Tcl_SetResult(interp, my_ip_addr, TCL_VOLATILE);
    }

    return TCL_OK;
}





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
int Status_Display(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    HPVT hVal;

    /* Delete listbox contents */
    TclExecute("status:Clean");

    hVal = cmGetValTree(hApp);
    TclExecute("status:InsertLine {CM} {TpktChans} %d %d {}\n"
        "status:InsertLine {} {Messages} %d %d {}\n"
        "status:InsertLine {} {PVT Nodes} %d %d %d\n"
        "status:InsertLine {} {Timers} %d %d {}",
        cmSizeCurTpktChans(hApp), cmSizeMaxTpktChans(hApp),
        cmSizeCurMessages(hApp), cmSizeMaxMessages(hApp),
        pvtCurSize(hVal), pvtMaxUsage(hVal), pvtMaxSize(hVal),
        cmSizeCurTimers(hApp), cmSizeMaxTimers(hApp));

    H450_status();

    /* Handle the application's resources - always last... */
    TclExecute("status:InsertLine {APP} {Allocations} %d %d {}\n"
        "status:InsertLine {} {Memory} %d %d {}",
        NumAllocations.numOfUsed, NumAllocations.maxUsage,
        SizeAllocations.numOfUsed, SizeAllocations.maxUsage);
    
    TclExecute("status:DrawGraphs");
    return TCL_OK;
}

#ifdef __cplusplus
}
#endif


