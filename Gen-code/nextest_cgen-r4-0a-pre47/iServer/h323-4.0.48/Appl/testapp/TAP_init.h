/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_init.h
 *
 * This file is used for initiating the test application.
 * It also used for defining global variables and common functions.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/



#ifndef _INIT_H
#define _INIT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <tcl.h>
#include <tk.h>

#include "TAP_defs.h"



/* TCL resources */
extern Tcl_Interp*  interp;

/* Application resources */
extern RV_Resource  NumAllocations;
extern RV_Resource  SizeAllocations;

extern BOOL         LogWrappers;

extern HAPP         hApp;


/********************************************************************************************
 * PutError
 * purpose : Notify the user about errors that occured
 * input   : title  - Title of the error
 *           reason - Reason that caused the error
 * output  : none
 * return  : none
 ********************************************************************************************/
void PutError(const char* title, const char* reason);


/********************************************************************************************
 * InitTcl
 * purpose : Initialize the TCL part of the test application
 * input   : executable     - Program executable
 *           versionString  - Stack version string
 * output  : reason         - Reason of failure on failure
 * return  : Tcl_Interp interpreter for tcl commands
 *           NULL on failure
 ********************************************************************************************/
Tcl_Interp* InitTcl(const char* executable, char* versionString, char** reason);

/********************************************************************************************
 * InitStack
 * purpose : Initialize the H.323 stack
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitStack(void);

/********************************************************************************************
* EndStack
* purpose : End the H.323 stack
* input   : none
* output  : none
* return  : none
********************************************************************************************/
void EndStack(void);

/********************************************************************************************
 * InitApp
 * purpose : Initialize the test application
 *           This includes parts as RTP/RTCP support, etc.
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitApp(void);


#ifdef __cplusplus
}
#endif

#endif

