#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_utils.h
 *
 * Application utility functions
 *
 * 1. Enumeration to string conversion functions
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Tsahi Levent-Levi                   26-Jun-2000
 *
 ********************************************************************************************/



#ifndef _TAP_utils_H
#define _TAP_utils_H

#include "cm.h"

#ifdef USE_H450
#include "sse.h"
#endif  /* USE_H450 */


char* BooleanStr(BOOL value);
int hStrcmp(const char* a, const char* b);


/******************/
/* Convert Tables */
/******************/

/* General */

char* Status2String(IN int status);
int String2Status(IN char* string);

cmPartyNumberType String2PartyNumber(char *string);
char* PartyNumber2String(cmPartyNumberType value);


/* CM */

cmCallState_e String2CMCallState(char* string);
char* CMCallState2String(cmCallState_e state);

char* CMCallStateMode2String(cmCallStateMode_e stateMode);

cmChannelState_e String2CMChannelState(char* string);
char* CMChannelState2String(cmChannelState_e state);

cmChannelStateMode_e String2CMChannelStateMode(char* string);
char* CMChannelStateMode2String(cmChannelStateMode_e stateMode);

cmCallParam String2CMCallParam(char* string);
char* CMCallParam2String(cmCallParam param);

cmH245Stage String2CMH245Stage(char* string);
char* CMH245Stage2String(cmH245Stage stage);

cmAnnexEUsageMode String2CMAnnexEUsageMode(char* string);
char* CMAnnexEUsageMode2String(cmAnnexEUsageMode stage);


/* RAS */

cmRASTransaction String2RASTransaction(char* string);
char* RASTransaction2String(cmRASTransaction transaction);

cmRASReason String2RASReason(char* string);
char* RASReason2String(cmRASReason reason);

cmEndpointType String2EndpointType(char* string);
char* EndpointType2String(cmEndpointType stage);

cmCallType String2CallType(char* string);
char* CallType2String(cmCallType stage);

cmCallModelType String2CallModelType(char* string);
char* CallModelType2String(cmCallModelType val);

cmRASDisengageReason String2DisengageReason(char* string);
char* DisengageReason2String(cmRASDisengageReason val);

cmRASReason String2Reason(char* string);
char* Reason2String(cmRASReason val);

cmRASUnregReason String2UnregReason(char* string);
char* UnregReason2String(cmRASUnregReason val);

cmTransportQOS String2TransportQOS(char* string);
char* TransportQOS2String(cmTransportQOS val);

int String2IrrStatus(char* string);
char* IrrStatus2String(int val);

cmRASParam String2RASParam(char* string);
char* RASParam2String(cmRASParam reason);

cmRASTrStage String2RASTrStage(char* string);
char* RASTrStage2String(cmRASTrStage reason);



/* Conversions */

int String2CMNonStandardParam(IN char* string, OUT cmNonStandardParam* param);
char* CMNonStandardParam2String(IN cmNonStandardParam* param);

int String2CMAlternateGatekeeper(IN char* string, OUT cmAlternateGatekeeper* param);
char* CMAlternateGatekeeper2String(IN cmAlternateGatekeeper* param);



/* H450 */
#ifdef USE_H450

int String2SSPrimitive(char* string);
char* SSPrimitive2String(int opcode);

ccIndication String2CCIndication(char* string);
char* CCIndication2String(ccIndication indication);

proc_e String2proc_e(char* string);
char* proc_e2String(proc_e proc);

reason_e String2reason_e(char* string);
char* reason_e2String(reason_e reason);

#endif  /* USE_H450 */




#endif  /* _TAP_utils_H */

#ifdef __cplusplus
}
#endif
