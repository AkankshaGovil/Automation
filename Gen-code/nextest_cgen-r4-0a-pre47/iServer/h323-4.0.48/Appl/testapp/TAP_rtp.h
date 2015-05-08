/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_rtp.h
 *
 * This file is used for RTP/RTCP support in the test application.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                    02-Aug-2000
 *
 ********************************************************************************************/



#ifndef _TAP_RTP_H
#define _TAP_RTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rvcommon.h"
#include "rtpctrl.h"



/********************************************************************************************
 * RTP_TestInit
 * purpose : Initialize the RTP/RTCP package of the test application
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestInit(void);


/********************************************************************************************
 * RTP_TestEnd
 * purpose : Deinitialize the RTP/RTCP package of the test application
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestEnd(void);


/********************************************************************************************
 * RTP_IsInitialized
 * purpose : Indicates if RTP/RTCP package is working
 * input   : none
 * output  : none
 * return  : TRUE if RTP/RCTP package can be used
 ********************************************************************************************/
BOOL RTP_IsInitialized(void);




/********************************************************************************************
 * RTP_TestOpen
 * purpose : Open an RTP channel
 * input   : name       - Name to use for the connection
 * output  : none
 * return  : Session number allocated
 *           Negative number on failure
 ********************************************************************************************/
int RTP_TestOpen(const char* name);


/********************************************************************************************
 * RTP_TestOpenSecondChannel
 * purpose : Open a second RTP channel on the same connection
 * input   : session    - Session number to use
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestOpenSecondChannel(int session);


/********************************************************************************************
 * RTP_TestClose
 * purpose : Close an RTP channel
 * input   : session    - Session number to use
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestClose(int session);




/********************************************************************************************
 * RTP_TestGetLocalPort
 * purpose : Return the local port used by the session
 * input   : session    - Session number to use
 * output  : none
 * return  : Port number used
 ********************************************************************************************/
int RTP_TestGetLocalPort(int session);



/********************************************************************************************
 * RTP_TestSetRemoteRTP
 * purpose : Open an RTP channel
 * input   : session        - Session number to use
 *           ip             - IP number of remote side
 *           port           - Port number of remote side
 *           isMulticast    - Is it a multicast address or not
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetRemoteRTP(int session, UINT32 ip, UINT16 port, BOOL isMulticast);



/********************************************************************************************
 * RTP_TestSetRemoteRTCP
 * purpose : Open an RTP channel
 * input   : session        - Session number to use
 *           ip             - IP number of remote side
 *           port           - Port number of remote side
 *           isMulticast    - Is it a multicast address or not
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetRemoteRTCP(int session, UINT32 ip, UINT16 port, BOOL isMulticast);



/********************************************************************************************
 * RTP_TestSetAction
 * purpose : Set an action on the given session: playback, record, replay, etc.
 * input   : session    - Session number to use
 *           action     - The action to set
 * output  : none
 * return  : none
 ********************************************************************************************/
void RTP_TestSetAction(int session, RTP_Action action);



/********************************************************************************************
 * RTP_TestRate
 * purpose : Return the rate of a session
 * input   : none
 * output  : none
 * return  : Rate of the session
 ********************************************************************************************/
UINT32 RTP_TestRate(int session);




#ifdef __cplusplus
}
#endif

#endif



