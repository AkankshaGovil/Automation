
/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************
 * cmAutoRasEP
 * -----------
 * This file provides the endpoint part of the automatic RAS module.
 * This includes the following features:
 *
 * - Automatic discovery and registration to a gatekeeper
 * - Unregistration of a gatekeeper
 * - Lightweight RRQs (timeToLive feature)
 * - Sending NSM and RAI messages
 * - Automatic responses to IRQ messages
 * - Enabling manual RAS to work with automatic RAS for endpoint related
 *   transactions
 ************************************************************************/


#ifndef _CM_AUTORAS_EP_H
#define _CM_AUTORAS_EP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmintr.h>
#include <cmAutoRasIntr.h>



/************************************************************************
 * cmiAutoRASEPStart
 * purpose: Start the endpoint part of the automatic RAS.
 *          This function sets internal callbacks with the RAS module and
 *          initializes some autoRAS related variables.
 * input  : hAutoRas - Automatic RAS instance
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASEPStart(IN HAUTORASMGR   hAutoRas);

/************************************************************************
 * cmiAutoRASGetEndpointID
 * purpose: Retrieves the EndpointID stored in the ras 
 *
 * input  : hApp    - Application's stack handle
 *          eId	    - pointer to the buffer for endpoint ID
 *                    buffer should be big enough for longest EID 
 *					  possible (256 byte)
 * output : none
 * return : The length of EID in bytes on success 
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASGetEndpointID(
    IN  HAPP    hApp,
	IN  void*	eId);


#ifdef __cplusplus
}
#endif

#endif  /* _CM_AUTORAS_EP_H */
