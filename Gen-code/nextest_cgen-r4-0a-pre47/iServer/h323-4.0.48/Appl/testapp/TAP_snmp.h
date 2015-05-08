/***********************************************************************************************************************

  Notice:
  This document contains information that is proprietary to RADVISION LTD..
  No part of this publication may be reproduced in any form whatsoever without
  written prior approval by RADVISION LTD..

    RADVISION LTD. reserves the right to revise this publication and make changes
    without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
*                                TAP_snmp.h
*
* This file contains all the functions which enable the use of security modes
*
*
*      Written by                        Version & Date                        Change
*     ------------                       ---------------                      --------
*  Tsahi Levent-Levi                      02-Mar-2001
*
********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TAP_SNMP_H
#define _TAP_SNMP_H


#include "TAP_general.h"
#include "TAP_defs.h"


/********************************************************************************************
 * SNMP_Init
 * purpose : This function initializes the H.341 MIB SNMP. does nothing if USE_SNMP
 *           is not defined.
 * input   : lphApp - pointer to the application handle
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int SNMP_init(void);


/********************************************************************************************
 * SNMP_end
 * purpose : Deinitialize the SNMP
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void SNMP_end(void);


/********************************************************************************************
 * SNMP_IsInitialized
 * purpose : Indicates if SNMP package is working
 * input   : none
 * output  : none
 * return  : TRUE if SNMP package can be used
 ********************************************************************************************/
BOOL SNMP_IsInitialized(void);




#endif /* _TAP_SNMP_H */

#ifdef __cplusplus
}
#endif
