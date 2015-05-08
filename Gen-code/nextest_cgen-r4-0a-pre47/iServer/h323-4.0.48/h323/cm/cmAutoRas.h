

/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef _CM_AUTORAS_H
#define _CM_AUTORAS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <mti.h>
#include <cm.h>
#include <cmCrossReference.h>



/************************************************************************
 * cmiAutoRasEvent enum
 * Events that are notified to the CM by automatic RAS.
 * cmiAutoRasEvGotRCF           - Indication about incoming RCF
 * cmiAutoRasEvCallDropFroced   - Indication about incoming DRQ on call
 * cmiAutoRasEvCallDropped      - Indication about incoming DCF, DRJ on call
 * cmiAutoRasEvRateChanged      - Indication about incoming BRQ or BCF on call
 * cmiAutoRasEvGotACF           - Indication about incoming ACF
 * cmiAutoRasEvCantRegister     - Indication about inability to register
 *                                while trying to make a call
 * cmiAutoRasEvFailedOnARQ      - Couldn't create an ARQ transaction
 * cmiAutoRasEvCallRejected     - Indication about incoming ARJ
 * cmiAutoRasEvTimedout         - Indication about timeout on transaction
 * cmiAutoRasEvPrepareIRR       - Ask the CM to fill in an unsolicited IRR
 *                                message information
 ************************************************************************/
typedef enum
{
    cmiAutoRasEvGotRCF,
    cmiAutoRasEvCallDropForced,
    cmiAutoRasEvCallDropped,
    cmiAutoRasEvRateChanged,
    cmiAutoRasEvGotACF,
    cmiAutoRasEvCantRegister,
    cmiAutoRasEvFailedOnARQ,
    cmiAutoRasEvCallRejected,
    cmiAutoRasEvTimedout,
    cmiAutoRasEvPrepareIRR
} cmiAutoRasEvent;


/************************************************************************
 * cmiEvAutoRASEventT
 * purpose: Automatic RAS event handler for notifying the CM about specific
 *          events in RAS.
 * input  : hsCall  - Call handle of the transaction (NULL if not applicable)
 *          hsRas   - RAS transaction handle
 *          event   - Event that occured
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
typedef int (RVCALLCONV *cmiEvAutoRASEventT) (
    IN HCALL            hsCall,
    IN HRAS             hsRas,
    IN cmiAutoRasEvent  event);






/************************************************************************
 * cmiAutoRASInit
 * purpose: Initialize the RAS module and all the network related with
 *          RAS.
 * input  : hApp        - Application's stack handle
 *          logMgr      - Log manager used by the stack
 *          hTimers     - Timers pool used by the stack
 *          hVal        - Value tree handle
 *          rasConfNode - RAS configuration tree
 *          evFunc      - Event handler to set
 * output : none
 * return : Automatic RAS handle created on success
 *          NULL on failure
 ************************************************************************/
HAUTORASMGR cmiAutoRASInit(
    IN HAPP                 hApp,
    IN RVHLOGMGR            logMgr,
    IN HSTIMER              hTimers,
    IN HPVT                 hVal,
    IN int                  rasConfNode,
    IN cmiEvAutoRASEventT   evFunc);


/************************************************************************
 * cmiAutoRASStart
 * purpose: Start the RAS module and all the network related with
 *          RAS.
 * input  : hApp    - Application's stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASStart(IN HAPP hApp);


/************************************************************************
 * cmiAutoRASEnd
 * purpose: End the automatic RAS module
 * input  : hApp        - Application's stack handle
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASEnd(IN HAPP hApp);


/************************************************************************
 * cmiAutoRAS
 * purpose: Returns the RAS mode
 *
 * input  : hApp    - Application's stack handle
 * output : none
 * return : TRUE for automatic RAS
 *          FALSE for manual RAS
 ************************************************************************/
BOOL cmiAutoRAS(IN HAPP hApp);



#ifdef __cplusplus
}
#endif

#endif  /* _CM_AUTORAS_H */
