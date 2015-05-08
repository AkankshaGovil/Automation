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



#include <cmintr.h>


/************************************************************************
 * msdInit
 * purpose: Initialize the master slave determination process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void msdInit(controlElem* ctrl);

/************************************************************************
 * msdEnd
 * purpose: Stop the master slave determination process on a control channel
 * input  : ctrl    - Control object
 * output : none
 * return : none
 ************************************************************************/
void msdEnd(IN controlElem* ctrl);

/*************************************************************************************
 * msdDetermineRequest
 *
 * Purpose: Main routine to initiate or respond to a MSD request.
 *
 * Input:	ctrl						- The ctrl element of the call
 *			terminalType				- The given terminal type of the call, may be -1.
 *			statusDeterminationNumber	- The determination numer, may be -1 and then needs to be
 *										  generated.
 * Output: None.
 *
 * return : Non-negative value on success
 *          Negative value on failure
 ******************************************************************************************/
int msdDetermineRequest(controlElem* ctrl, int terminalType, int statusDeterminationNumber);

/************************************************************************
 * masterSlaveDetermination
 * purpose: Handle an incoming MasterSlaveDetermination request,
 *          notifying the application or answering automatically.
 * input  : ctrl    - Control object
 *          message - MSD request message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDetermination(IN controlElem* ctrl, IN int message);

/************************************************************************
 * masterSlaveDeterminationAck
 * purpose: Handle an incoming MasterSlaveDeterminationAck message
 * input  : ctrl    - Control object
 *          message - MSD.Ack message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationAck(IN controlElem* ctrl, IN int message);

/************************************************************************
 * masterSlaveDeterminationReject
 * purpose: Handle an incoming MasterSlaveDeterminationReject message
 * input  : ctrl    - Control object
 *          message - MSD.Reject message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationReject(IN controlElem* ctrl, IN int message);

/************************************************************************
 * masterSlaveDeterminationRelease
 * purpose: Handle an incoming MasterSlaveDeterminationRelease message
 * input  : ctrl    - Control object
 *          message - MSD.Release message node
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int masterSlaveDeterminationRelease(IN controlElem* ctrl, IN int message);

/************************************************************************
 * msdGetMibParams
 * purpose: Get parameters related to the MIB for an MSD object in the
 *          control
 * input  : ctrl            - Control object
 * output : state           - State of MSD
 *          status          - Status of MSD
 *          retries         - Number of MSD retries
 *          terminalType    - Local terminal's type in MSD
 * return : Non-negative value on success
 *          Negative value on failure
 * Any of the output parameters can be passed as NULL if not relevant to caller
 ************************************************************************/
int msdGetMibParams(
    IN  controlElem*    ctrl,
    OUT msdState*       state,
    OUT msdStatus*      status,
    OUT int*            retries,
    OUT int*            terminalType);



#ifdef __cplusplus
}
#endif
