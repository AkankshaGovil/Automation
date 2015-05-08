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



#ifndef _FAST_START_H
#define _FAST_START_H

#ifndef CM_H
#error faststart.h included improperly. Use #include <cm.h> instead.
#else

#define MAX_ALTERNATIVE_CHANNEL 10
#define MAX_FS_CHANNELS 3 /* restricted by well known sessionId 1 for audio,2 for video, 3 for data*/

typedef struct
{
  cmTransportAddress rtp; /* not used for outgoing (transmit) channels */
  cmTransportAddress rtcp;
  int dataTypeHandle;
  char *channelName;
  int index;
}cmFastStartChannel;


typedef enum
{
    dirReceive,
    dirTransmit,
    dirBoth
} cmChannelDirection;


typedef struct
{
  cmFastStartChannel channels[MAX_ALTERNATIVE_CHANNEL];
  int altChannelNumber;
}cmAlternativeChannels; /* alternative proposes channel list.
                      Only one channel must be chose by called endpoint.*/


typedef struct
{
  cmCapDataType type;
  cmAlternativeChannels transmit;
  cmAlternativeChannels receive;
}cmPartnerChannels;

typedef struct
{
  cmPartnerChannels partnerChannels[MAX_FS_CHANNELS];
  int partnerChannelsNum;

}cmFastStartMessage;


RVAPI int RVCALLCONV
cmFastStartChannelsAckIndex(    HCALL hsCall,int index,cmTransportAddress *rtcp,cmTransportAddress *rtp);

RVAPI int RVCALLCONV
cmFastStartChannelsAck( HCALL hsCall,
                      IN  cmFastStartChannel *pFastChannel);


/******************************************************************************************
 * cmFastStartOpenChannels
 *
 * Purpose:  This API function enables the caller to supply a structure with data about
 *           the offered logical channels for fast start procedure. The structure includes
 *           all offered channels, both incoming and outgoing, arranged by their type, i.e.
 *           Audio channels, Video channels, etc.
 *
 * Input:    hsCall - A handle to the call whose setup message shall carry the
 *                    fast start offer.
 *
 *           fsMessage - A pointer to the structure containing the channels data.
 *
 * Reurned Value: TRUE or RVERROR.
 *
 ****************************************************************************************/
RVAPI int RVCALLCONV
cmFastStartOpenChannels(IN HCALL hsCall, IN cmFastStartMessage* fsMessage);


RVAPI int RVCALLCONV
cmFastStartChannelsReady(   HCALL hsCall);

RVAPI int RVCALLCONV
cmFastStartChannelsRefuse(HCALL hsCall);







/************************************************************************
 *
 *          FASTSTART API: Supporting more than 10 channels
 *
 ************************************************************************/


/************************************************************************
 * cmFastStartBuild
 * purpose: Build a single OpenLogicalChannel message for use in fast start
 *          proposals
 * input  : hsCall      - Stack handle for the call
 *          type        - DataType of the proposed channel
 *          direction   - Direction of the proposed channel
 *          fsChannel   - FastStart channel information
 * return : Node ID of created OpenLogicalChannel on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmFastStartBuild(
    IN HCALL                hsCall,
    IN cmCapDataType        type,
    IN cmChannelDirection   direction,
    IN cmFastStartChannel*  fsChannel);


/************************************************************************
 * cmCallAddFastStartMessage
 * purpose: Add an OpenLogicalChannel message to the fast start proposal
 *          on the origin side of the call.
 * input  : hsCall      - Stack handle for the call
 *          fsMessageId - Node ID of the OpenLogicalChannel to add
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmCallAddFastStartMessage(IN HCALL hsCall, IN int fsMessageId);


/************************************************************************
 * cmFastStartGetByIndex
 * purpose: Get faststart information of a single OpenLogicalChannel
 *          message from the faststart proposal string
 * input  : hsCall  - Stack handle for the call
 *          index   - Index of the faststart proposal to get (0-based)
 * return : Node ID of the OpenLogicalChannel message proposal on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmFastStartGetByIndex(IN HCALL hsCall, IN int index);


/************************************************************************
 * cmFastStartGet
 * purpose: Get faststart information of a single OpenLogicalChannel
 *          message from the faststart proposal string
 * input  : hsCall      - Stack handle for the call
 *          fsChannelId - PVT node ID of the root of the faststart proposal
 *                        of one of the channels
 * output : type        - DataType of the proposed channel
 *          direction   - Direction of the proposed channel
 *          fsChannel   - FastStart channel information
 * return : Non-negative value on success
 *          Negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmFastStartGet(
        IN  HCALL               hsCall,
        IN  int                 fsChannelId,
        OUT cmCapDataType*      type,
        OUT cmChannelDirection* direction,
        OUT cmFastStartChannel* fsChannel);


/************************************************************************
 * cmCallFastStartOpenChannels
 * purpose: Set the answered information to be sent for a fast start
 *          proposal and open the channels on the destination side of the
 *          call. This function should be called after receiving
 *          cmEvCallFastStart() or cmEvCallFastStartSetup()
 * input  : hsCall      - Call to use
 *          proposed    - List of proposed channels. This list is searched for
 *                        their RTP and RTCP addresses.
 *                        Each parameter in this list is a root node of an
 *                        OpenLogicalChannel message to propose in faststart
 *                        The last element in this list should be a negative value.
 *          accepted    - List of accepted channels.
 *                        Each paramenter in this list is a root node of an
 *                        OpenLogicalChannel message sent to the origin of the call
 *                        by this endpoint.
 *                        The last element in this list should be a negative value.
 *          origin      - TRUE if this application is the origin of the call
 *                        FALSE if this application is not the origin of the call
 * output : none
 * return : non-negative value on success
 *          negative value on failure
 ************************************************************************/
RVAPI int RVCALLCONV
cmCallFastStartOpenChannels(
    IN HCALL    hsCall,
    IN int*     proposed,
    IN int*     accepted,
    IN BOOL     origin);


#endif  /* CM_H */

#endif  /* _FAST_START_H */

#ifdef __cplusplus
}
#endif



