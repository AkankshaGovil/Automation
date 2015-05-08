
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************
 * cmAutoRasCall
 * -------------
 * This file provides the calls part of the automatic RAS module.
 * This includes the following features:
 *
 * -
 ************************************************************************/


#ifndef _CM_AUTORAS_CALL_H
#define _CM_AUTORAS_CALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cm.h>
#include <cmAutoRasIntr.h>



/************************************************************************
 * cmiAutoRASCallStart
 * purpose: Start the calls part of the automatic RAS.
 *          This function sets internal callbacks with the RAS module and
 *          initializes some autoRAS related variables.
 * input  : hAutoRas - Automatic RAS instance
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASCallStart(IN HAUTORASMGR hAutoRas);


/************************************************************************
 * cmiAutoRASCallCreate
 * purpose: Create the call information needed for automatic RAS for
 *          a specified call
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallCreate(IN HCALL   hsCall);


/************************************************************************
 * cmiAutoRASCallDial
 * purpose: Send an ARQ on a call to start the call with the GK
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallDial(IN HCALL   hsCall);

/************************************************************************
 * cmiAutoRASCallSetRate
 * purpose: Send a BRQ on a call to change the call bandwidth
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallSetRate(IN HCALL hsCall, int rate);


/************************************************************************
 * cmiAutoRASCallSetUnsolicitedIRR
 * purpose: Initilises send of unsolicited IRRs (used for pregranted ARQ only
 * input  : hsCall - Stack's call handle
 *          irrFrequency - requested frequency ofIRRs
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallSetUnsolicitedIRR(IN HCALL hsCall, int irrFrequency);


/************************************************************************
 * cmiAutoRASCallDial
 * purpose: Send a DRQ on a call to start disengaginh the call from the GK
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallDrop(IN HCALL hsCall);



/************************************************************************
 * cmiAutoRASCallDial
 * purpose: Free auto ras resources for the call
 * input  : hsCall  - Stack's call handle
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASCallClose(IN HCALL hsCall);

#ifdef __cplusplus
}
#endif

#endif  /* _CM_AUTORAS_CALL_H */
