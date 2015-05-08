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
 *                                defs.h
 *
 * This file provides definitions for the entire test application.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/

#ifndef _DEFS_H
#define _DEFS_H

#define BUFFER_SIZE 1024
#include "cm.h"

#ifdef USE_H450
#include "sse.h"
#endif

#include "TAP_rtp.h"


/* Alow recursive use of these structs */
typedef struct CallInfo_tag CallInfo;
typedef struct ChannelInfo_tag ChannelInfo;




/********************************************************************************************
 *                             struct CallInfo
 *
 * This struct includes all the call parameters that
 * are being exported to the  call/conference window.
 *
 * hsCall           - Call handle in the CM for this call
 * counter          - Counter value given to this call
 * scriptCall       - Indicates if this call is handled by the script or by the GUI
 * incoming         - RV_TRUE if the call is an incoming call
 *                    RV_FALSE if the call is an outgoing call
 * numOfChannels    - Number of channels opened in this call
 * firstChannel     - First channel opened (regarded as a linked-list)
 * lastChannel      - Last channel opened (regarded as a linked-list)
 * sendComplete     - Indicates if we have to set "sendingComplete" field inside SETUP and
 *                    INFORMATION messages
 * action           - Action to take on media channels
 * insideState      - Indicate the level of recursion inside the state event handler of the
 *                    call. It is used to protect from freeing the memory due to H450 events
 *                    that might close this call.
 * hCon             - Connection used for this call. Used for coloring the connection to
 *                    distinguish those calls that are multiplexed
 *
 * hSSECall         - used for H450: Call handle for SSE
 ********************************************************************************************/
struct CallInfo_tag
{
    HCALL                       hsCall;
    int                         counter;
    BOOL                        scriptCall;
    BOOL                        incoming;
    int                         numOfChannels;
    ChannelInfo*                firstChannel;
    ChannelInfo*                lastChannel;
    BOOL                        sendComplete;
    RTP_Action                  action;
    int                         insideState;
    HPROTCONN                   hCon;
    UINT32                      callState;
    int                         rtpSessions[3];
    BOOL                        isFastConnectCall;

#ifdef USE_H450
    HSSECALL                    hSSECall;
#endif
};



/********************************************************************************************
 *                             struct ChannelInfo
 *
 * This struct holds the application's information about a channel.
 *
 * hChan                - CM Handle for the channel
 * scriptChannel        - Indicates if this channel is controled by the scripts or not
 * channelId            - Channel ID. This is used to associate a channel with the GUI on the
 *                        screen
 * direction            - Indication if this channel is an outgoing channel or not
 * rtpSession           - RTP Session of the channel
 * dataType             - Channel's data type. Used for auto-mimic and fast start channels
 * isFastStart          - Indicates if the channel is a fast start channel or a regular channel
 * dynPayloadType       - If other than 0, it indicates the dynamic payload type of an incoming
 *                        channel
 * dataTypeNode         - Node holding the DataType information. Used when mimicing a channel
 * call                 - Pointer to the CallInfo for the call holding this channel
 * prevChannel          - Previous channel in list of channels assocated with the call
 * nextChannel          - Next channel in list of channels associated with the call
 * state                - Latest channel state
 ********************************************************************************************/
struct ChannelInfo_tag
{
    HCHAN                       hChan;
    BOOL                        scriptChannel;
    INT32                       channelId;
    BOOL                        isOutgoing;
    int                         rtpSession;
    char                        dataType[20];
    BOOL                        isFastStart;
    INT8                        dynPayloadType;
    int                         dataTypeNode;
    CallInfo*                   call;
    ChannelInfo*                prevChannel;
    ChannelInfo*                nextChannel;
    BOOL                        mark; /* Used for printing the channels */
    cmChannelState_e            state;
};


/********************************************************************************************
 * Definition of RV_Resource:
 * numOfUsed            - Current number of resources being used
 * maxUsage             - Maximum number of resources ever used in the same time
 * maxNumOfElements     - Maximum number of resources in the system
 ********************************************************************************************/
typedef struct
{
    UINT32  numOfUsed;
    UINT32  maxUsage;
    UINT32  maxNumOfElements;
} RV_Resource;



#endif

#ifdef __cplusplus
}
#endif
