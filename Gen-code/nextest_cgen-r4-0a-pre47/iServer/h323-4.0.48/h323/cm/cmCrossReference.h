/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef CMCROSSREFERENCE_H
#define CMCROSSREFERENCE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ema.h>
#include <cat.h>
#include <cm.h>


/* RAS module handle */
DECLARE_OPAQUE(HRASMGR);

/* Automatic RAS module handle */
DECLARE_OPAQUE(HAUTORASMGR);

DECLARE_OPAQUE(HPCALL);
DECLARE_OPAQUE(HCONTROL);
DECLARE_OPAQUE(HQ931);

/* Automatic RAS call handle */
DECLARE_OPAQUE(HAUTORASCALL);


#define RAS_OUT_TX    (1)
#define RAS_IN_TX     (2)
#define CALL          (3)
#define CHAN          (4)




/************************************************************************
 *
 *                       Stack instance related
 *
 ************************************************************************/


/* todo: comment */
HEMA cmiInitCalls(HAPP hApp,int maxCalls,int maxChannels, RVHLOGMGR logMgr);
void cmiEndCalls(HEMA calls);

HEMA cmiInitChannels(HAPP hApp,int maxCalls,int maxChannels, RVHLOGMGR logMgr);
void cmiEndChannels(HEMA channels);

/************************************************************************
 * cmiGetRasHandle
 * purpose: Get the RAS module object from the stack's instance
 * input  : hApp    - Stack's application handle
 * output : none
 * return : RAS module handle on success
 *          NULL on failure
 ************************************************************************/
HRASMGR cmiGetRasHandle(IN HAPP hApp);


/************************************************************************
 * cmiGetAutoRasHandle
 * purpose: Get the Automatic RAS module object from the stack's instance
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Automatic RAS module handle on success
 *          NULL on failure
 ************************************************************************/
HAUTORASMGR cmiGetAutoRasHandle(IN HAPP hApp);


/************************************************************************
 * cmiGetRasHooks
 * purpose: Get the hook functions set by the application of the CM
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Pointer to the hook functions set by the application
 ************************************************************************/
CMPROTOCOLEVENT cmiGetRasHooks(IN HAPP hApp);







/************************************************************************
 *
 *                              Call related
 *
 ************************************************************************/


/* todo: comment */
HPCALL cmiGetCall(HCALL call);
HCALL cmiGetByCall(HPCALL call);
HQ931 cmiGetQ931(HCALL call);
HCALL cmiGetByQ931(HQ931 qCall);
HCONTROL cmiGetControl(HCALL call);
HCALL cmiGetByControl(HCONTROL ctrl);
HCHAN cmiGetChannelForCtrl(HCONTROL ctrl, int i);
HCHAN cmiGetChannelForCall(HCALL call, int i);
HCHAN* cmiGetChannelsCollectionForCtrl(HCONTROL ctrl);
HCHAN* cmiGetChannelsCollectionForCall(HCALL call);
int cmiGetNumberOfChannelsForCtrl(HCONTROL ctrl);
int cmiGetNumberOfChannelsForCall(HCALL call);
int cmiSetNumberOfChannelsForCtrl(HCONTROL ctrl,int i);
int cmiSetNumberOfChannelsForCall(HCALL call,int i);

/************************************************************************
 * cmiGetCatForCall
 * purpose: Get the CAT call handle from the stack's call handle
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Cat call handle on success
 *          NULL on failure
 ************************************************************************/
RVHCATCALL cmiGetCatForCall(IN HCALL hsCall);


/************************************************************************
 * cmiGetAutoRas
 * purpose: Get the Automatic RAS call object from the stack's call handle
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Automatic RAS call handle on success
 *          NULL on failure
 ************************************************************************/
HAUTORASCALL cmiGetAutoRas(IN HCALL hsCall);


/************************************************************************
 * cmiGetByAutoRas
 * purpose: Get the stack's call handle from the Automatic RAS call object
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Stack's call handle
 *          NULL on failure
 ************************************************************************/
HCALL cmiGetByAutoRas(IN HAUTORASCALL aras);


#ifdef __cplusplus
}
#endif

#endif

