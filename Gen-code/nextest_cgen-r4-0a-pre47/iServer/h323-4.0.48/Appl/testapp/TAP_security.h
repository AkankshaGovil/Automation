/***********************************************************************************************************************

  Notice:
  This document contains information that is proprietary to RADVISION LTD..
  No part of this publication may be reproduced in any form whatsoever without
  written prior approval by RADVISION LTD..

    RADVISION LTD. reserves the right to revise this publication and make changes
    without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
*                                TAP_security.h
*
* This file contains all the functions which enable the use of security modes
*
*
*      Written by                        Version & Date                        Change
*     ------------                       ---------------                      --------
*                                         10-Jan-2001
*
********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TAP_SECURITY_H
#define _TAP_SECURITY_H

#include "TAP_general.h"
#include "TAP_defs.h"

/********************************************************************************************
 * SEC_Init
 * purpose : This function initializes the security sepplementay services. does nothing if
 *           USE_SECURITY is not defined.
 * input   : lphApp - pointer to the application handle
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int SEC_init(HAPP lphApp);

/********************************************************************************************
 * SEC_IsInitialized
 * purpose : Indicates if SECURITY package is working
 * input   : none
 * output  : none
 * return  : TRUE if SECURITY package can be used
 ********************************************************************************************/
BOOL SEC_IsInitialized(void);

/********************************************************************************************
 * SEC_notifyModeChange
 * purpose : This procedure will be called every time the Security mode changes.
 *           argv[1] will contain the new mode string.
 * syntax  : SEC.notifyModeChange <newMode>
 * input   : none
 * output  : none
 * return  : TCL_OK on ok, TCL_ERROR when not ok.
 ********************************************************************************************/
int SEC_notifyModeChange(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);




#endif /* _TAP_SECURITY_H */

#ifdef __cplusplus
}
#endif
