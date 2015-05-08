#ifdef __cplusplus
extern "C" {
#endif
/*
***********************************************************************************

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

***********************************************************************************
*/



#ifndef CMI_FAST_START_H
#define CMI_FAST_START_H

#include <cmCall.h>

/******************************************************************************************
 * analyzeFastStartMsg
 *
 * Purpose:  This function is called upon receipt of a setup message.
 *           The function checks for any fast-start channels in the message,
 *           decodes the data and builds it into a structure that is passed in a CallBack
 *           to the application, such that the application may ack some of them.
 *
 * Input:    call       - call object instance
 *           message    - The node id to the setup message
 *
 * Reurned Value: Non-negative value on success
 *                Negative value on failure
 ****************************************************************************************/
int analyzeFastStartMsg(
                IN  callElem*       call,
                IN  int             message
                );


/******************************************************************************************
 * cmFastStartReply
 *
 * Purpose:  This function is called from the CM whenever a response message after SETUP
 *           is received (CallProceeding, Alerting, Connect, Facility and Progress).
 *           It checks for FastStart response. If such exists, it processes it and opens
 *           the relevant channels.
 *
 * Input:    call       - call object instance
 *           uuNodeId   - UserUser node ID of the message
 *
 * Reurned Value: Non-negative value on success
 *                Negative value on failure
 ****************************************************************************************/
int  cmFastStartReply(
    IN callElem*    call,
    IN int          uuNodeId);


void deleteFastStart(callElem*call);
void addFastStart(callElem*call,int message);
int  fastStartInit(cmElem*app, int maxCalls, int proposed, int accepted);
void fastStartEnd(cmElem*app);
void fastStartCallInit(callElem*call);

#endif

#ifdef __cplusplus
}
#endif



