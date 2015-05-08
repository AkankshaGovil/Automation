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


#include <ema.h>

#include <cmAutoRasIntr.h>
#include <cmCall.h>
#include <cmQ931.h>
#include <cmiQ931.h>
#include <cmControl.h>
#include <cmCrossReference.h>


/********************/
/*The structure of  */
/*the call element  */
/********************/
/*callElem          */
/*aRASElem          */
/*q931Elem          */
/*controlElem       */
/*number of channels*/
/*channelPtr[0]     */
/*channelPtr[1]     */
/*channelPtr[2]     */
/*.............     */
/********************/


/************************************************************************
 *
 *                       Stack instance related
 *
 ************************************************************************/


HEMA cmiInitCalls(HAPP hApp,int maxCalls,int maxChannels, RVHLOGMGR logMgr)
{
    return emaConstruct(sizeof(callElem)+sizeof(autorasCall)+sizeof(q931Elem)+sizeof(controlElem)+sizeof(INTPTR)+maxChannels*sizeof(void*),
                        maxCalls,emaNormalLocks,logMgr,"CM Calls",CALL,NULL,(void*)hApp);
}

void cmiEndCalls(HEMA calls)
{
    emaDestruct(calls);
}

HEMA cmiInitChannels(HAPP hApp,int maxCalls,int maxChannels, RVHLOGMGR logMgr)
{
    return emaConstruct(sizeof(channelElem),maxCalls*maxChannels, emaLinkedLocks, logMgr, "CM Channels",CHAN,NULL,(void*)hApp);
}

void cmiEndChannels(HEMA channels)
{
    emaDestruct(channels);
}


/************************************************************************
 * cmiGetRasHandle
 * purpose: Get the RAS module object from the stack's instance
 * input  : hApp    - Stack's application handle
 * output : none
 * return : RAS module handle on success
 *          NULL on failure
 ************************************************************************/

HRASMGR cmiGetRasHandle(IN HAPP hApp)
{
    return ((cmElem*)hApp)->rasManager;
}


/************************************************************************
 * cmiGetAutoRasHandle
 * purpose: Get the Automatic RAS module object from the stack's instance
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Automatic RAS module handle on success
 *          NULL on failure
 ************************************************************************/
HAUTORASMGR cmiGetAutoRasHandle(IN HAPP hApp)
{
    return ((cmElem*)hApp)->hAutoRas;
}


/************************************************************************
 * cmiGetRasHooks
 * purpose: Get the hook functions set by the application of the CM
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Pointer to the hook functions set by the application
 ************************************************************************/
CMPROTOCOLEVENT cmiGetRasHooks(IN HAPP hApp)
{
    return &((cmElem*)hApp)->cmMyProtocolEvent;
}




/************************************************************************
 *
 *                              Call related
 *
 ************************************************************************/

/**/
typedef enum {
    callOffset = 0,
    rasOffset  = sizeof(callElem),
    q931Offset = rasOffset  + sizeof(autorasCall),
    ctrlOffset = q931Offset + sizeof(q931Elem),
    numOffset  = ctrlOffset + sizeof(controlElem),
    chanOffset = numOffset  + sizeof(INTPTR)
}callOffsetsFlags;
/**/
HPCALL cmiGetCall(HCALL call)
{
    return (HPCALL)((char*)call + callOffset);
}


HCALL cmiGetByCall(HPCALL call)
{
    return (HCALL)((char*)call - callOffset);
}

/************************************************************************
 * cmiGetAutoRas
 * purpose: Get the Automatic RAS call object from the stack's call handle
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Automatic RAS call handle on success
 *          NULL on failure
 ************************************************************************/
HAUTORASCALL cmiGetAutoRas(IN HCALL call)
{
    return (HAUTORASCALL)((char*)(call) + rasOffset);
}


/************************************************************************
 * cmiGetByAutoRas
 * purpose: Get the stack's call handle from the Automatic RAS call object
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Stack's call handle
 *          NULL on failure
 ************************************************************************/
HCALL cmiGetByAutoRas(IN HAUTORASCALL aras)
{
    return (HCALL)((char*)(aras) - rasOffset);
}

HQ931 cmiGetQ931(HCALL call)
{
    return (HQ931)((char*)(call) + q931Offset);
}

HCALL cmiGetByQ931(HQ931 qCall)
{
    return (HCALL)((char*)(qCall) - q931Offset);
}


HCONTROL cmiGetControl(HCALL call)
{
    return (HCONTROL)((char*)(call) + ctrlOffset);
}

HCALL cmiGetByControl(HCONTROL ctrl)
{
    return (HCALL)((char*)(ctrl) - ctrlOffset);
}

HCHAN cmiGetChannelForCtrl(HCONTROL ctrl, int i)
{
    return cmiGetChannelForCall(cmiGetByControl(ctrl), i);
}

HCHAN cmiGetChannelForCall(HCALL call, int i)
{
    return *(HCHAN*)((char*)(call) + chanOffset + (i)*sizeof(void*));
}

HCHAN* cmiGetChannelsCollectionForCtrl(HCONTROL ctrl)
{
    return cmiGetChannelsCollectionForCall(cmiGetByControl(ctrl));
}

HCHAN* cmiGetChannelsCollectionForCall(HCALL call)
{
    return (HCHAN*)((char*)(call) + chanOffset);
}


int cmiGetNumberOfChannelsForCtrl(HCONTROL ctrl)
{
    return cmiGetNumberOfChannelsForCall(cmiGetByControl(ctrl));
}

int cmiGetNumberOfChannelsForCall(HCALL call)
{
    return *(int*)((char*)(call)+numOffset);
}

int cmiSetNumberOfChannelsForCtrl(HCONTROL ctrl,int i)
{
    return cmiSetNumberOfChannelsForCall(cmiGetByControl(ctrl),i);
}

int cmiSetNumberOfChannelsForCall(HCALL call,int i)
{
    return *(INTPTR*)((char*)(call) + numOffset)=i;
}

/************************************************************************
 * cmiGetCatForCall
 * purpose: Get the CAT call handle from the stack's call handle
 * input  : hApp    - Stack's application handle
 * output : none
 * return : Cat call handle on success
 *          NULL on failure
 ************************************************************************/
RVHCATCALL cmiGetCatForCall(IN HCALL hsCall)
{
    return ((callElem*)hsCall)->hCatCall;
}





#ifdef __cplusplus
}
#endif
