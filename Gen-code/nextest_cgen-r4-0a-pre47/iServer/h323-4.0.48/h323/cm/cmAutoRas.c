#ifdef __cplusplus
extern "C" {
#endif




/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#include <rvinternal.h>
#include <li.h>
#include <cmintr.h>
#include <cmiras.h>
#include <q931asn1.h>
#include <cmutils.h>
#include <cmAutoRasEP.h>
#include <cmAutoRasCall.h>
#include <cmAutoRas.h>





/************************************************************************
 *
 *                          Private functions
 *
 ************************************************************************/






/************************************************************************
 *
 *                          Public functions
 *
 ************************************************************************/


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
    IN cmiEvAutoRASEventT   evFunc)
{
    autorasEndpoint* autoras;
    if (logMgr);

    autoras = (autorasEndpoint *)malloc(sizeof(autorasEndpoint));
    if (autoras == NULL) return NULL;
    memset(autoras, 0, sizeof(autorasEndpoint));

    /* Set automatic RAS parameters */
    autoras->hApp = hApp;
    autoras->hTimers = hTimers;
    autoras->hVal = hVal;
    autoras->confNode = rasConfNode;
    autoras->event = evFunc;

    autoras->state = cmIdle;
    autoras->regTimer = (HTI)RVERROR;

    return (HAUTORASMGR)autoras;
}


/************************************************************************
 * cmiAutoRASStart
 * purpose: Start the RAS module and all the network related with
 *          RAS.
 * input  : hApp    - Application's stack handle
 * output : none
 * return : Non negative value on success
 *          Negative value on failure
 ************************************************************************/
int cmiAutoRASStart(IN HAPP hApp)
{
    HAUTORASMGR hAutoRas = cmiGetAutoRasHandle(hApp);

    cmiAutoRASEPStart(hAutoRas);
    cmiAutoRASCallStart(hAutoRas);

    return 0;
}


/************************************************************************
 * cmiAutoRASEnd
 * purpose: End the automatic RAS module
 * input  : hApp        - Application's stack handle
 * output : none
 * return : none
 ************************************************************************/
void cmiAutoRASEnd(IN HAPP hApp)
{
    autorasEndpoint* autoras = (autorasEndpoint *)((cmElem*)hApp)->hAutoRas;

    if (autoras != NULL)
        free(autoras);
}


/************************************************************************
 * cmiAutoRAS
 * purpose: Returns the RAS mode
 * input  : hApp    - Application's stack handle
 * output : none
 * return : TRUE for automatic RAS
 *          FALSE for manual RAS
 ************************************************************************/
BOOL cmiAutoRAS(IN HAPP hApp)
{
      return (((cmElem*)hApp)->hAutoRas != NULL);
}




#ifdef __cplusplus
}
#endif

