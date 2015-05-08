
/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                main.c
 *
 * This file is the main file of the Test Application.
 * This file generates the gui that was written in tcl/tk.
 * It also creates new tcl commands.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/

/*****************************************************************
 * DEFINITIONS for ADDONS
 * ----------------------
 * USE_H450     - H.450 Supplamentary Services
 * USE_SECURITY - H.235 Security
 * USE_RTP      - RTP (loopback of channels)
 * USE_SNMP     - H.341 MIB
 *****************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <windows.h>
#include <locale.h>

#else
#include "seli.h"
#include "mti.h"
#endif  /* WIN32 */


#include <stdlib.h>
#include "TAP_general.h"
#include "TAP_defs.h"
#include "TAP_init.h"

/* Global variables declarations */
Tcl_Interp* interp;
RV_Resource NumAllocations;
RV_Resource SizeAllocations;


#ifndef WIN32

static HSTIMER unixGuiTimers;
static HTI unixGuiTimer;


/********************************************************************************************
 * handleGuiEvent
 * purpose : Handle an X GUI event on Unix machines
 * input   : fdHandle   - File descriptor handle used for GUI events
 *           event      - Event that occured
 *           userData   - User specific data (unused here)
 *           error      - Error indication on event
 * output  : none
 ********************************************************************************************/
void handleGuiEvent(IN int          fdHandle,
                    IN seliEvents   event,
                    IN BOOL         error)
{
    BOOL notFinished = TRUE;
    if (fdHandle || error); /* Remove warnings from compilation */

    /* Handle GUI events until we don't have any */
    while (notFinished)
    {
        if (!Tk_DoOneEvent(TK_DONT_WAIT))
            notFinished = FALSE;
    }
}

/********************************************************************************************
 * handleTimedGuiEvents
 * purpose : Handle an X GUI event on Unix machines in specific intervals of a timer
 * input   : context    - Unused
 * output  : none
 ********************************************************************************************/
void RVCALLCONV handleTimedGuiEvents(IN void* context)
{
    BOOL notFinished = TRUE;

    /* Handle GUI events until we don't have any */
    while (notFinished)
    {
        if (!Tk_DoOneEvent(TK_DONT_WAIT))
            notFinished = FALSE;
    }
}

#endif  /* WIN32 */




/********************************************************************************************
 * main
 * purpose : the main function of the test application.
 * input   : argc - number of prarmeters entered to main
 * input   : argv - parameters entered to test application
 * output  : none
 ********************************************************************************************/
int main(int argc, char *argv[])
{
    char verStr[60];
    char* reason = NULL;
    int status;

    sprintf(verStr, "H.323 Protocol Toolkit v%s", cmGetVersion());

    /* Initialize the TCL part of the test application */
    interp = InitTcl(argv[0], verStr, &reason);
    if (reason != NULL)
    {
        PutError("Error initializing Tcl/Tk", reason);
        if (interp)
            Tcl_DeleteInterp(interp);
        exit(-1);
    }

    /* Initialize H.323 stack */
    if ((status = InitStack()) < 0)
    {
        switch (status)
        {
            case -2: reason = (char*)"Resource problem"; break;
            case -10: reason = (char*)"Memory problem"; break;
            case -11: reason = (char*)"Configuration problem"; break;
            case -13: reason = (char*)"Network problem"; break;
        }
        PutError("Error initializing H.323 Stack", reason);
        exit(-1);
    }

    /* Initialize other application packages */
    if (InitApp() < 0)
    {
        PutError("InitApp", "Error initializing test application");
        exit(-1);
    }

    /* Run the test application */
#ifdef WIN32
    Tk_MainLoop();

#else  /* WIN32 */

    /* We should most probably initialize seli... */
    seliInit();

    /* Get a nice timer to work for us. Every 50ms should be enough */
    unixGuiTimers = mtimerInit(1, (HAPPTIMER)&unixGuiTimer);

    {
        int appFd = XConnectionNumber(Tk_Display(Tk_MainWindow(interp)));
        printf("Application is using fd=%d for X Windows events\n", appFd);

        /* Unix - use seliSelect() loop */
        seliCallOn(appFd, seliEvRead, handleGuiEvent);

        /* Also use a timer, otherwise we take all the CPU of the poor Unix machine. */
        /* There are probably other good ways of doing it through the X Windows, but */
        /* it's not really part of our job to do it good enough (it's only the TCL */
        /* part of the test application after all. */
        unixGuiTimer = mtimerSet(unixGuiTimers, handleTimedGuiEvents, NULL, 50);
    }

    while (TRUE)
    {
        seliSelect();
    }

    /* Remove our GUI timer */
    mtimerReset(unixGuiTimers, unixGuiTimer);
    mtimerEnd(unixGuiTimers);

    seliEnd();

#endif  /* WIN32 */

    return 0;
}




#if defined (WIN32) && defined (_WINDOWS)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int nullLoc;
    LPSTR cmdLine;

    /* Find out the executable's name */
    setlocale(LC_ALL, "C");
    cmdLine = GetCommandLine();
    if (strlen(lpCmdLine) > 0)
    {
        nullLoc = strlen(cmdLine) - strlen(lpCmdLine);
        cmdLine[nullLoc] = '\0';
    }

    /* Deal with it as if we're running a console or Unix application */
    main(0, (char**)&cmdLine);
}



#endif  /* Win32 App */




#ifdef __cplusplus
}
#endif


