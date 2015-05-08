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

/*
  cmchannel.h

  Ron S.
  15 Jan 1997.

  */

#ifndef _CHCHANNELH_
#define _CHCHANNELH_

#include <cmControl.h>
#include <cmmib.h>
#include <conf.h>

int openLogicalChannel(controlElem* ctrl, int base, int message);
int openLogicalChannelAck(controlElem* ctrl, int base, int message);
int openLogicalChannelConfirm(controlElem* ctrl, int base, int message);
int openLogicalChannelReject(controlElem* ctrl, int base, int message);
int closeLogicalChannel(controlElem* ctrl, int base, int message);
int closeLogicalChannelAck(controlElem* ctrl, int base, int message);
int flowControlCommand(controlElem* ctrl, int base, int message);
int requestChannelClose(controlElem* ctrl, int base, int message);
int requestChannelCloseAck(controlElem* ctrl, int base, int message);
int requestChannelCloseReject(controlElem* ctrl, int base, int message);
int requestChannelCloseRelease(controlElem* ctrl, int base, int message);
int maintenanceLoopRequest(controlElem* ctrl, int base, int message);
int maintenanceLoopAck(controlElem* ctrl, int base, int message);
int maintenanceLoopReject(controlElem* ctrl, int base, int message);
int maintenanceLoopOffCommand(controlElem* ctrl, int message);

channelElem*allocateChannel(HCONTROL  ctrl);
void deriveChannels(HCONTROL ctrl);
void closeChannels(HCONTROL ctrl);

/******************************************************************************
 * incomingChannelMessage
 * ----------------------------------------------------------------------------
 * General: Make sure cmEvChannelRecvMessage() is invoked in necessary on
 *          incoming channel related messages.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  hApp     - Stack instance used
 *         channel  - Channel this message belongs to
 *         message  - Message received
 * Output: None.
 *****************************************************************************/
void incomingChannelMessage(
    IN HAPP             hApp,
    IN channelElem*     channel,
    IN int              message);


int
cmcCallDataTypeHandleCallback(
                  /* Call the data type handle callback */
                  IN  HAPP  hApp,
                  IN  HCHAN hsChan, /* channel protocol */
                  IN  int dataType, /* channel data type node id */
                  IN  confDataType type
                  );

int
cmcCallChannelParametersCallback(
                 /* Call the channel parameter callback */
                 IN  HAPP  hApp,
                 IN  HCHAN hsChan, /* channel protocol */
                 IN  int dataType, /* channel data type node id */
                 OUT confDataType* type
                 );


/************************************************************************
 * chanGetMibParam
 * purpose: Get channel related MIB parameters
 * input  : hsChan      - Channel to check
 *          type        - Parameter type to get
 * output : valueSize   - Value, if numeric
 *                        String's length if string value type
 *          value       - String value if applicable
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
int chanGetMibParam(
    IN  HCHAN                   hsChan,
    IN  mibControlParamTypeT    type,
    OUT int*                    valueSize,
    OUT UINT8*                  value);


#endif
#ifdef __cplusplus
}
#endif



