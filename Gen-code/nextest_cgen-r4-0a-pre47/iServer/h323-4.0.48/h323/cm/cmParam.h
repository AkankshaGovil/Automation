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

#ifndef CMPARAM_H
#define CMPARAM_H

#include <cm.h>
#include <cmintr.h>


/************************************************************************
 * initParamSyntax
 * purpose: Initialize any information related to PST that is needed to
 *          hold parameters without a properties database.
 * input  : app         - Stack instance handle
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int initParamSyntax(IN cmElem*  app);

/************************************************************************
 * endParamSyntax
 * purpose: Deinitialize any information related to PST that is needed to
 *          hold parameters without a properties database.
 * input  : app         - Stack instance handle
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
int endParamSyntax(IN cmElem*  app);


/************************************************************************
 * callInitParameters
 * purpose: Initialize the property database for the call according to the
 *          mode used in the configuration.
 * input  : hsCall  - Stack handle for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callInitParameters(IN HCALL hsCall);

/************************************************************************
 * callEndParameters
 * purpose: Destroy the property database for the call, with all of its
 *          PVT roots in different modes
 * input  : hsCall  - Stack handle for the call
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callEndParameters(IN HCALL hsCall);


/************************************************************************
 * callGetMessage
 * purpose: Get a Q931 message for the given call. This function will
 *          create the necessary message or just take it from the property
 *          database of the call - all according to the current property
 *          mode used for the stack.
 * input  : hsCall      - Stack's call handle
 *          messageType - Type of message to get
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callGetMessage(IN HCALL hsCall, IN cmCallQ931MsgType messageType);


/************************************************************************
 * callReleaseMessage
 * purpose: This function is called by the stack when it doesn't need a
 *          message for a given call anymore. If the property mode used
 *          allows it, the message will be deleted.
 * input  : hsCall      - Stack's call handle
 *          messageType - Type of message to get
 * output : none
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int callReleaseMessage(IN HCALL hsCall, IN cmCallQ931MsgType messageType);


int callSetMessage(HCALL hsCall, cmCallQ931MsgType messageType, int message);
int getCallProperty(HCALL hsCall);

#endif
#ifdef __cplusplus
}
#endif



