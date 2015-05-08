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


#include <pvaltree.h>
#include <psyntreeStackApi.h>
#include <mti.h>
#include <cm.h>
#include <cmictrl.h>
#include <cmdebprn.h>
#include <cmintr.h>
#include <netutl.h>
#include <conf.h>
#include <ms.h>
#include <li.h>
#include <q931asn1.h>
#include <h245.h>
#include <cmutils.h>
#include <copybits.h>
#include <prnutils.h>
#include <cmCrossReference.h>
#include <cmchan.h>
#include <cmCall.h>
#include <cmChanGetByXXX.h>
#include <syslog.h>

#define ifE(a) if(a)(a)


    /*These are theoretical maxima, these numbers are used in the lcn allocation procedure and is assumed to never be changed */
/*The main assumption here is that (MAX_FASTSTART_CHANNELS+1)*MAX_LCN_S <= 64K(the number of possible LCNs)*/
#define MAX_FASTSTART_CHANNELS  511


/* Maximum number of session ids that fast start can handle */
#define MAX_FS_SESSION_ID 10



/********************************************************************************************
 * Description of THREAD_FSLocalStorage
 * This structure holds thread related information for FASTSTART
 * bufferSize   - The size of allocated buffer. We might need that when we'll have to realloc
 * buffer       - The encoding/decoding buffer
 ********************************************************************************************/
typedef struct
{
    UINT32  bufferSize;
    BYTE*   buffer;
} THREAD_FSLocalStorage;



int cmcCallAddressCallbacks(
                  IN cmElem* app,
                  IN channelElem* channel,
                  IN int h225ParamNodeId,
                  IN BOOL origin);

int notifyChannelState(channelElem*channel,cmChannelState_e state, cmChannelStateMode_e stateMode);

int  cmcReadyEvent  (controlElem* ctrl);

void deleteFastStart(callElem*call);

static void buildLCParameters(callElem* call, int elemNodeId, cmCapDataType type, cmFastStartChannel* fsChannel)
{

    int paramNodeId,nodeId;
    cmElem*app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal;

    hVal=app->hVal;
    /* Build and fill the dataType either from the configuration or from the given
       structure */
    nodeId=pvtAddBranch(hVal,elemNodeId,__h245(dataType));

    if (fsChannel->dataTypeHandle<0)
        confGetDataType(hVal, app->h245Conf,fsChannel->channelName, nodeId, NULL, FALSE);
    else
        pvtSetTree(hVal,nodeId,hVal,fsChannel->dataTypeHandle);

    paramNodeId=pvtAddBranch2(hVal,elemNodeId,__h245(multiplexParameters),__h245(h2250LogicalChannelParameters));

    /* Build and set the mediaChannel subtree (rtp) */
    if (fsChannel->rtp.ip || fsChannel->rtp.port)
    {
        nodeId=pvtAddBranch(hVal,paramNodeId,__h245(mediaChannel));
        getGoodAddressForCall((HCALL)call,&fsChannel->rtp);
        cmTAToVt_H245(hVal, nodeId, &fsChannel->rtp);
    }

    /* Build and set the mediaControlChannel subtree (rtcp) */

    if (fsChannel->rtcp.ip || fsChannel->rtcp.port)
    {
        nodeId=pvtAddBranch(hVal,paramNodeId,__h245(mediaControlChannel));
        getGoodAddressForCall((HCALL)call,&fsChannel->rtcp);
        cmTAToVt_H245(hVal, nodeId, &fsChannel->rtcp);
    }


    /* fill in the session id according to the transalation table */
    pvtAdd(hVal,paramNodeId,__h245(sessionID),type,NULL,NULL);
}


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
    IN cmFastStartChannel*  fsChannel)
{
    callElem* call=(callElem*)hsCall;
    /*controlElem* ctrl=getControl(hsCall);*/
    cmElem*app=(cmElem*)emaGetInstance((EMAElement)hsCall);
    HPVT hVal;
    int outElemId,openMsgId=RVERROR;


    if (!hsCall || !app) return RVERROR;
    hVal=app->hVal;

    cmiAPIEnter((HAPP)app, "cmFastStartBuild: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        /* Build a OLC message for the channel */
        openMsgId = pvtAddRoot(hVal, app->synOLC, 0, NULL);
        /* Build the forward logical channel subtree (should be dummy for receiving channel) */
        outElemId=pvtAddBranch(hVal,openMsgId,__h245(forwardLogicalChannelParameters));

        if (direction==dirBoth)
        {
			/* Nextone - added to check DataType == NULL data in
                        ** facility Faststart (Avaya Interop)*/
        	if ((cmCapDataType)type != cmCapEmpty)
			{
            	buildLCParameters(call, outElemId, type, fsChannel);
			}
			else
			{
            	pvtAddBranch2(hVal,outElemId, __h245(multiplexParameters),__h245(none));
            	/* Write nullData in the dataType part and dummy number as lcn */
	            pvtAddBranch2(hVal,outElemId,__h245(dataType),__h245(nullData));
			}
			/* Nextone - added to check DataType == NULL data in
                        ** facility Faststart (Avaya Interop)*/
            /* Now build the reverse logical channel subtree */
            outElemId=pvtAddBranch(hVal,openMsgId,__h245(reverseLogicalChannelParameters));
        }
        if (direction==dirReceive)
        {
            pvtAddBranch2(hVal,outElemId, __h245(multiplexParameters),__h245(none));

            /* Write nullData in the dataType part and dummy number as lcn */
            pvtAddBranch2(hVal,outElemId,__h245(dataType),__h245(nullData));
            pvtAdd(hVal,openMsgId,__h245(forwardLogicalChannelNumber),323,NULL,NULL);

            /* Now get to business and build the reverse logical channel subtree */
            outElemId=pvtAddBranch(hVal,openMsgId,__h245(reverseLogicalChannelParameters));
        }

		/* Nextone - added to check DataType == NULL data in facility
                ** Faststart (Avaya Interop)*/
        if ((cmCapDataType)type != cmCapEmpty)
		{
        	buildLCParameters(call, outElemId, type, fsChannel);
		}
		else
		{
            if (direction==dirTransmit)
           		pvtAddBranch2(hVal,outElemId, __h245(multiplexParameters),__h245(none));
           	/* Write nullData in the dataType part and dummy number as lcn */
	        pvtAddBranch2(hVal,outElemId,__h245(dataType),__h245(nullData));
		}
		/* Nextone - added to check DataType == NULL data in facility
                ** Faststart (Avaya Interop)*/

        pvtDelete(hVal, fsChannel->dataTypeHandle);
        fsChannel->dataTypeHandle = -1;

        if (direction!=dirReceive)
        {
            /* allocate a logical channel number for the current channel and write it into the
               appEvent message */
            pvtAdd(hVal,openMsgId,__h245(forwardLogicalChannelNumber),++call->lcnOut,NULL,NULL);
        }
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit((HAPP)app, "cmFastStartBuild: [%d]",openMsgId);
    return openMsgId;
}


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
cmCallAddFastStartMessage(IN HCALL hsCall, IN int fsMessageId)
{
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);

    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCallAddFastStartMessage: hsCall=0x%lx",hsCall);
    if (call->fastStartNodesCount == ((cmElem *)hApp)->maxFsProposed)
    {
        /* array is maxFsProposed+1 size, must leave the last one -1 */
        cmiAPIExit(hApp, "cmCallAddFastStartMessage: no more room [-1]");
        return RVERROR;
    }

    if (emaLock((EMAElement)hsCall))
    {
        if (m_callget(call,callInitiator))
        {
            call->fastStartNodes[call->fastStartNodesCount++]=fsMessageId;
            call->fastStartState=fssRequest;
        }
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmCallAddFastStartMessage: [1]");

    return TRUE;
}


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
cmFastStartOpenChannels(IN HCALL hsCall, IN cmFastStartMessage* fsMessage)
{
    int openMsgId,index,channelType,ret=1;
    callElem* call=(callElem*)hsCall;
    /*controlElem* ctrl=getControl(hsCall);*/
    HAPP hApp;
    HPVT hVal;

    if (!hsCall || fsMessage->partnerChannelsNum<=0) return RVERROR;
    hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hApp) return RVERROR;
    hVal=cmGetValTree(hApp);
    cmiAPIEnter(hApp, "cmFastStartOpenChannels: hsCall=0x%lx",hsCall);

    if (emaLock((EMAElement)hsCall))
    {
        /* first, delete previous FS offers, if they exist. */
        deleteFastStart(call);
        /* This is the main loop that goes over the offered channels in the given structure
        and build from it the sub-tree to be saved in the H245 machine and attached to the
        SETUP message. The order is according to the channel type (Audio, Video etc.) */
        for (channelType = 0; channelType< fsMessage->partnerChannelsNum ;channelType++)
        {
            cmAlternativeChannels* aChannel;
            /* We currently handle only audio and video channels in faststart */
            if ( ((int)fsMessage->partnerChannels[channelType].type < (int)cmCapEmpty) || ((int)fsMessage->partnerChannels[channelType].type > (int)cmCapData) )
                continue;

            aChannel=&fsMessage->partnerChannels[channelType].receive;

            /* Go over the offered receive channels */
            for (index=0;index<aChannel->altChannelNumber;index++)
            {
                /* Build logicalChannel message */
                openMsgId = cmFastStartBuild(hsCall, fsMessage->partnerChannels[channelType].type, dirReceive, &aChannel->channels[index]);
                /* The OLC is ready for the receive channel, encode it */
                cmCallAddFastStartMessage(hsCall, openMsgId);
            }

            aChannel=&fsMessage->partnerChannels[channelType].transmit;

            /* Now go over the offered transmit channels */
            for (index=0;index<aChannel->altChannelNumber;index++)
            {
                /* Build logicalChannel message */
                openMsgId = cmFastStartBuild(hsCall, fsMessage->partnerChannels[channelType].type, dirTransmit, &aChannel->channels[index]);
                /* The OLC is ready for the receive channel, encode it */
#if 0 /* Nextone */
                cmCallAddFastStartMessage(hsCall, openMsgId);
#endif
				/*@@Netmodule@@MH@@ Added check if all nodes could be added to the message (RFE 3337) */
				if (cmCallAddFastStartMessage(hsCall, openMsgId) != TRUE) {
					if (hVal != 0) {pvtDelete(hVal, openMsgId);}
					ret = RVERROR;
				}
            }
        }
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmFastStartOpenChannels: [%d]",ret);

    return ret;
}


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
cmFastStartGetByIndex(IN HCALL hsCall, IN int index)
{
    int nodeId=RVERROR;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);

    if (!hApp || !hsCall) return RVERROR;

    cmiAPIEnter(hApp, "cmFastStartGetByIndex: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        if ((index < 0) || (index >= ((callElem*)hsCall)->fastStartNodesCount))
            nodeId = RVERROR; /* out of bounds */
        else
            nodeId=((callElem*)hsCall)->fastStartNodes[index];
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmFastStartGetByIndex: [%d]",nodeId);
    return nodeId;
}


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
        OUT cmFastStartChannel* fsChannel)
{
    callElem* call=(callElem*)hsCall;
    int dirNodeId,paramNodeId,tempNodeId, sid;
    HAPP hApp;
    HPVT hVal;

    if (!hsCall) return RVERROR;
    hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hApp) return RVERROR;
    hVal=cmGetValTree(hApp);

    cmiAPIEnter(hApp, "cmFastStartGet: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        /* Get the reverse part of the OLC message to determine whether it is
         a receiving channel or a transmitting channel (from the viewpoint of
         the remote that sent the SETUP) */
        dirNodeId=pvtGetChild(hVal, fsChannelId, __h245(reverseLogicalChannelParameters),NULL);

        {
            /* in or bidirectional channel */
            /* Get the forward parameters */
            int tmpNodeId=pvtGetChild(hVal, fsChannelId,__h245(forwardLogicalChannelParameters),NULL);
            if (pvtGetChild2(hVal, tmpNodeId, __h245(dataType),__h245(nullData))<0)
            {                        /* Forward DataType is OK */
                if (dirNodeId >= 0) /* and there are reverse parameters */
                {   /* bidirectional channel */
                  if (direction)
                        *direction=dirBoth;
                }
                else                /* no reverse parameters */
                {
                    /* in channel */
                    dirNodeId = tmpNodeId;
                    if (direction)
                        *direction=dirReceive;
                }
            }
            else                    /* no forward DataType */
            {   /* out channel */

              if (direction)
                    *direction=dirTransmit;
            }
        }

        /* Get the H2550 parameters of the channel */
        paramNodeId = pvtGetChild2(hVal, dirNodeId, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));

        sid = 0;

        /* according to the dataType determine if it's an Audio or a Video
         channel and set the session Id accordingly.
         Note: This part needs to be enhanced for support of other types (data, fax etc.)*/

        tempNodeId = pvtChild(hVal,pvtGetChild(hVal, dirNodeId, __h245(dataType),NULL));
        {
          static char const SID_From_Index[]={0,0,0,2,1,3,0,0,0};
          int index=pvtGetSyntaxIndex(hVal,tempNodeId);
          if (index>=0)
            sid=SID_From_Index[index];
        }

        if (type)
            *type=(cmCapDataType)sid;

        /* set the pvt of the dataType subtree */
        fsChannel->dataTypeHandle=pvtGetChild(hVal, dirNodeId, __h245(dataType),NULL);


		/* Nextone - added to check DataType == NULL data in facility
                ** Faststart (Avaya Interop)*/
        if ((cmCapDataType)sid != cmCapEmpty)
		{
	        /* Get the codec type name and fill it into the table */
    	    {
        	    int capNodeId;
	            INTPTR fieldId;

    	        /* For Data capabilities, we have to go one step further... */
        	    capNodeId = pvtChild(hVal, pvtChild(hVal, fsChannel->dataTypeHandle));
	            if ((cmCapDataType)sid == cmCapData)
    	            capNodeId = pvtChild(hVal, capNodeId);

        	    pvtGet(hVal, capNodeId, &fieldId,NULL,NULL,NULL);
	            fsChannel->channelName = pstGetFieldNamePtr(pvtGetSynTree(hVal, dirNodeId),fieldId);
    	    }

	        /* Get the rtcp ip address and fill that into the table */
    	    cmVtToTA_H245(hVal,pvtGetChild(hVal,paramNodeId, __h245(mediaControlChannel), NULL) , &fsChannel->rtcp);
	
    	    /* Get the rtp ip address and fill that into the table */
	        cmVtToTA_H245(hVal,pvtGetChild(hVal,paramNodeId, __h245(mediaChannel), NULL) , &fsChannel->rtp);
		} 
		/* Nextone - added to check DataType == NULL data in facility
                ** Faststart (Avaya Interop)*/

        /* We have no way of knowing the channel index when we are here, so we'll just put in with -1 */
        fsChannel->index = -1;

        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmFastStartGet: [0]");
    return 0;
}


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
int
analyzeFastStartMsg(
                IN  callElem*       call,
                IN  int             message)
{
    cmFastStartMessage fsMessage;
    int nodeId;
    int nesting;

    BYTE lcnAllocationBuff[(MAX_FASTSTART_CHANNELS+1)/sizeof(char)];

    int decoded_length, buff_len, freeLoc=0;
    int index=0;
    int typeLoc[MAX_FS_CHANNELS+1];
    int encodedOctedStrings;
    cmAlternativeChannels *pAltChannel;
    HPVT hVal;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    cmElem* app=(cmElem*)hApp;

    if (!app->cmMyCallEvent.cmEvCallFastStartSetup && !app->cmMyCallEvent.cmEvCallFastStart) return TRUE;
    if ((call->fastStartState==fssAck) || (call->fastStartState==fssRej))   return TRUE;
    hVal=cmGetValTree(hApp);

    /* Find out if we've got faststart at all in this message */
#if 1
    __pvtGetNodeIdByFieldIds(encodedOctedStrings,hVal,message,
                        {_q931(message)
                         _anyField
                         _q931(userUser)
                         _q931(h323_UserInformation)
                         _q931(h323_uu_pdu)
                         _q931(h323_message_body)
                         _anyField
                         _q931(fastStart)
                         LAST_TOKEN});
#else
	cmCallGetParam((HCALL)call,cmParamSetupFastStart,0,&encodedOctedStrings,NULL);
#endif
    if (encodedOctedStrings<0) return RVERROR;
    call->fastStartState=fssRequest;

    /* Initialized the table to be built */
    memset(&fsMessage,0,sizeof(cmFastStartMessage));
    memset(typeLoc,0xff,sizeof(typeLoc));
    memset(lcnAllocationBuff,0,sizeof(lcnAllocationBuff));

    /* Go over the received encoded OLC message from the SETUP,
       decode them for subsequent analysis */
    nodeId = pvtChild(hVal, encodedOctedStrings);
    while (nodeId > 0)
    {
        BYTE* encodeBuffer;

        getEncodeDecodeBuffer(app->encodeBufferSize, &encodeBuffer);
        buff_len = pvtGetString(hVal, nodeId, app->encodeBufferSize, (char*)encodeBuffer);
        if (buff_len > 0)
        {
            /* Decode an OpenLogicalChannel faststart message */
            int chanMsgId=call->fastStartNodes[index]=pvtAddRoot(hVal, app->synOLC, 0, NULL);
            call->fastStartNodesCount++;
            
            if (cmEmDecode(hVal, chanMsgId, encodeBuffer, buff_len, &decoded_length)>=0)
            {
#ifndef NOLOGSUPPORT
                if  (logGetDebugLevel(app->logFastStart)>1)
                {
                    /* Print the message received */
                    logPrintFormat(app->logFastStart, RV_DEBUG, "Suggested faststart channel decoded:");
                    pvtPrintStd(hVal, chanMsgId, (int)app->logFastStart);
                    logPrintFormat(app->logFastStart, RV_DEBUG, "Binary:");
                    printHexBuff(encodeBuffer, decoded_length, app->logFastStart);
                }
#endif
                {
                    /* Make the logical channel's number as taken */
                    INT32 lcn;
                    pvtGetChildValue(hVal, chanMsgId,__h245(forwardLogicalChannelNumber),&lcn,NULL);
                    setBit(lcnAllocationBuff,lcn%(MAX_FASTSTART_CHANNELS+1),1);
                }
            }
            else
            {
                logPrint(app->logFastStart, RV_ERROR,
                    (app->logFastStart, RV_ERROR, "Decoding Problems!"));
                nodeId = pvtBrother(hVal, nodeId);
                continue;
            }

            if (app->cmMyCallEvent.cmEvCallFastStartSetup)
            {
                cmFastStartChannel fsChannel;
                cmCapDataType type;
                cmChannelDirection direction;
                int loc;

                /* We now have an offered channel */
                /* Go over the OLC message and analyse it */

                cmFastStartGet((HCALL)call, chanMsgId, &type, &direction, &fsChannel);

                /* Fill the session Id into the channel table being prepared for the user */
                if (typeLoc[(int)type] == -1)
                {
                    loc=typeLoc[(int)type] = freeLoc++; /* todo: How is freeLoc used? */
                    fsMessage.partnerChannels[loc].type = type;
                    fsMessage.partnerChannelsNum++;
                }
                else
                    loc=typeLoc[(int)type];

                /* Set a pointer to the right part of the channel tables
                 according to the channels direction */
                if (direction==dirTransmit)
                                    pAltChannel    = &(fsMessage.partnerChannels[loc].transmit);
                else                pAltChannel    = &(fsMessage.partnerChannels[loc].receive);


                /* Now fill all the data from the OLC message to the table */
                if (pAltChannel->altChannelNumber < MAX_ALTERNATIVE_CHANNEL)
                {

                    /* Give the channel an index */
                    fsChannel.index = index;

                    pAltChannel->channels[pAltChannel->altChannelNumber]=fsChannel;

                    /* Advance the channels counter in the table */
                    pAltChannel->altChannelNumber++;

                }
            }
        }
        nodeId = pvtBrother(hVal, nodeId);
        index++;
        if (index == app->maxFsProposed)
        {
            /* we were offered more channels than we have room for - ignore the rest */
            /* array is maxFsProposed+1 size, must leave the last one -1 */
            break;
        }
    }
    /*If the number of fast start proposals was less then MAX_FASTSTART_CHANNELS then at least one range will be available */
    call->lcnOut=(get1st0BitNumber(lcnAllocationBuff, 1, MAX_FASTSTART_CHANNELS)-1);

    if (app->cmMyCallEvent.cmEvCallFastStartSetup)
    {
        /* We call the callback with the table and supply it to the user.
         The user is supposed to acke the desired channels from that callback */
        cmiCBEnter(hApp,(char*)"cmEvCallFastStartSetup(haCall=0x%lx,hsCall=0x%lx)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
        nesting=emaPrepareForCallback((EMAElement)call);
        ifE(app->cmMyCallEvent.cmEvCallFastStartSetup)((HAPPCALL)emaGetApplicationHandle((EMAElement)call), (HCALL)call,&fsMessage);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit(hApp,(char*)"cmEvCallFastStartSetup");
    }

    if (!emaWasDeleted((EMAElement)call) && app->cmMyCallEvent.cmEvCallFastStart)
    {
        cmiCBEnter(hApp,(char*)"cmEvCallFastStart(haCall=0x%lx,hsCall=0x%lx)",(HAPPCALL)emaGetApplicationHandle((EMAElement)call),(HCALL)call);
        nesting=emaPrepareForCallback((EMAElement)call);
        app->cmMyCallEvent.cmEvCallFastStart((HAPPCALL)emaGetApplicationHandle((EMAElement)call), (HCALL)call);
        emaReturnFromCallback((EMAElement)call,nesting);
        cmiCBExit(hApp,(char*)"cmEvCallFastStart");
    }

    return  TRUE;
}


RVAPI int RVCALLCONV
cmFastStartChannelsAckIndex(    HCALL hsCall,int index,cmTransportAddress *rtcp,cmTransportAddress *rtp)

{
    int ret=1;
    int lcpNodeId;
    int h2250ParametersNodeId;
    int outOLCNodeId;
    callElem* call=(callElem*)hsCall;
    /*controlElem* ctrl=getControl(hsCall);*/
    HAPP hApp;
    HPVT hVal;
    cmElem* app;
    if (!hsCall) return RVERROR;
    hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hApp) return RVERROR;
    hVal=cmGetValTree(hApp);
    app=(cmElem*)hApp;

    cmiAPIEnter(hApp, "cmFastStartChannelsAckIndex: hsCall=0x%lx,index = %d",hsCall,index);

    if (call->fastStartNodesAckCount == app->maxFsAccepted)
    {
        /* array is maxFsAccepted+1 size, must leave the last one -1 */
        cmiAPIExit(hApp, "cmFastStartChannelsAckIndex: out of room [-1]");
        return RVERROR;
    }

    if (emaLock((EMAElement)hsCall))
    {

        /* Start build OLC for ack */
        outOLCNodeId = pvtAddRoot(hVal, app->synOLC, 0, NULL);

        if (outOLCNodeId >= 0)
        {
            call->fastStartIndexes[call->fastStartNodesAckCount] = (BYTE) index;
            call->fastStartNodesAck[call->fastStartNodesAckCount++] = outOLCNodeId;

            /* Copy the OLC*/
            pvtSetTree(hVal,outOLCNodeId, hVal, call->fastStartNodes[index]);

            /* check the channel direction, only proposal for outgoing and bi-directional channels have reverse parameters*/
            lcpNodeId=pvtGetChild(hVal, outOLCNodeId, __h245(reverseLogicalChannelParameters),NULL);

            if (lcpNodeId >= 0)
            {
                int tempNode;
                __pvtGetNodeIdByFieldIds(tempNode, hVal, outOLCNodeId,
                    {_h245(forwardLogicalChannelParameters) _h245(dataType) _h245(nullData) LAST_TOKEN});

                if (tempNode >= 0)
                {
                    /* outgoing (reverse) channel */
                    /* Overwrite the forward LCN */
                    pvtAdd(hVal, outOLCNodeId, __h245(forwardLogicalChannelNumber),++call->lcnOut, NULL,NULL);
                }
                else
                {
                    /* bi-directional channel */
                }
            }
            else
            {
                /* incoming (forward) channel */
                lcpNodeId=pvtGetChild(hVal, outOLCNodeId, __h245(forwardLogicalChannelParameters),NULL);
            }

            h2250ParametersNodeId=pvtGetChild2(hVal, lcpNodeId, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));

            if (h2250ParametersNodeId>=0)
            {
                int tmpNodeId;
                tmpNodeId = pvtAddBranch(hVal, h2250ParametersNodeId, __h245(mediaChannel));
                if ((rtp != NULL) && (rtp->ip || rtp->port))
                {
                    getGoodAddressForCall((HCALL)call,rtp);
                    cmTAToVt_H245(hVal,tmpNodeId, rtp);
                }
                else
                    pvtDelete(hVal, tmpNodeId);

                tmpNodeId = pvtAddBranch(hVal, h2250ParametersNodeId, __h245(mediaControlChannel));
                if ((rtcp != NULL) && (rtcp->ip || rtcp->port))
                {
                    getGoodAddressForCall((HCALL)call,rtcp);
                    cmTAToVt_H245(hVal,tmpNodeId, rtcp);
                }
                else
                    pvtDelete(hVal, pvtGetChild(hVal, h2250ParametersNodeId, __h245(mediaControlChannel), NULL));
            }
        }
        else
	{
            ret = outOLCNodeId;
    	    syslog(LOG_LOCAL3|LOG_NOTICE, "Ack Index synOLC=%d", app->synOLC);
	}

        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmFastStartChannelsAckIndex: [%d]",ret);
    return ret;
}

RVAPI int RVCALLCONV
cmFastStartChannelsAck( HCALL hsCall,
                        IN  cmFastStartChannel *pFastChannel)
{
    int ret=1;
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmFastStartChannelsAck: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        ret = cmFastStartChannelsAckIndex(hsCall,pFastChannel->index, &(pFastChannel->rtcp), &(pFastChannel->rtp));
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp, "cmFastStartChannelsAck: [%d]",ret);

    return ret;
}


RVAPI int RVCALLCONV
cmFastStartChannelsReady(HCALL hsCall)
{
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;
    
    cmiAPIEnter(hApp, "cmFastStartChannelsReady: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        controlElem *ctrl = (controlElem *)cmiGetControl(hsCall);
        int fastStartPrpd[2*MAX_FS_SESSION_ID];
        int i;
        int limit = min(call->fastStartNodesAckCount, (2*MAX_FS_SESSION_ID - 1)); /* must keep last entry -1 */
        
        memset(fastStartPrpd, 0xff, sizeof(fastStartPrpd));
        for(i=0; i<limit; i++)
        {
            fastStartPrpd[i] = call->fastStartNodes[call->fastStartIndexes[i]];
        }
        
        cmCallFastStartOpenChannels((HCALL)hsCall, fastStartPrpd, call->fastStartNodesAck, FALSE);
        m_callset(call, fastStartFinished, TRUE);
        cmcReadyEvent(ctrl);
        emaUnlock((EMAElement)hsCall);
    }
    
    cmiAPIExit(hApp, "cmFastStartChannelsReady: [OK]");
    
    return TRUE;
}

RVAPI int RVCALLCONV
cmFastStartChannelsRefuse(HCALL hsCall)
{
    callElem* call=(callElem*)hsCall;
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)call);
    if (!hsCall || !hApp) return RVERROR;
    
    cmiAPIEnter(hApp, "cmFastStartChannelsRefuse: hsCall=0x%lx",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        controlElem *ctrl = (controlElem *)cmiGetControl(hsCall);

        call->fastStartState = fssRej;
        m_callset(call, fastStartFinished, TRUE);
        cmcReadyEvent(ctrl);
        emaUnlock((EMAElement)hsCall);
    }
    cmiAPIExit(hApp, "cmFastStartChannelsRefuse: [OK]");
    
    return TRUE;
}
    
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
int cmFastStartReply(
    IN callElem*    call,
    IN int          uuNodeId)
{
    cmElem*app=(cmElem*)emaGetInstance((EMAElement)call);
    controlElem *ctrl=(controlElem *)cmiGetControl((HCALL)call);
    HPVT hVal=app->hVal;
    int fsNodeId;
	BOOL fsChannelsExist;
    int  rc;

    /* Are we in the right fast-start state? */
    if ((call->fastStartState==fssAck) || (call->fastStartState==fssRej))
        return RVERROR;

    /* Make sure we've got fast start information here and get the first one */
    fsNodeId=pvtGetChild(hVal,uuNodeId,__q931(fastStart),NULL);
	fsChannelsExist = (fsNodeId >= 0);
    fsNodeId=pvtChild(hVal, fsNodeId);

    /* Loop through all of the acknowledgements and put them into the call's structure */
    while (fsNodeId >= 0)
    {
        BYTE* encodeBuffer;
        int length;
        int decoded_length;
        int chanMsgId;
        
        /* Decode a single OLC.Ack message */
        chanMsgId = pvtAddRoot(hVal, app->synOLC, 0, NULL);
        getEncodeDecodeBuffer(app->encodeBufferSize, &encodeBuffer);
        length=pvtGetString(hVal, fsNodeId, app->encodeBufferSize, (char*)encodeBuffer);
        if (cmEmDecode(hVal, chanMsgId, encodeBuffer, length, &decoded_length)>=0)
        {
#ifndef NOLOGSUPPORT
            if  (logGetDebugLevel(app->logFastStart)>1)
            {
                logPrintFormat(app->logFastStart, RV_DEBUG, "Accepted faststart channel decoded:");
                pvtPrintStd(hVal , chanMsgId, (int)app->logFastStart);
                logPrintFormat(app->logFastStart, RV_DEBUG, "Binary:");
                printHexBuff(encodeBuffer, decoded_length, app->logFastStart);
            }
#endif
            /* Add the decoded OLC.Ack into the fast start acknowledgments of the call */
            call->fastStartNodesAck[call->fastStartNodesAckCount++] = chanMsgId;
        }
        else
        {
            logPrintFormat(app->logFastStart, RV_ERROR, "Decoding Problems!");
            pvtDelete(hVal, chanMsgId);
        }

        /* Goto the next one in the next entry to the while loop */
        if (call->fastStartNodesAckCount == app->maxFsAccepted)
            /* array is maxFsAccepted+1 size, must leave the last one -1 */
            break;
        fsNodeId=pvtBrother(hVal, fsNodeId);
    }

	if (fsChannelsExist)
    {
	    rc = cmCallFastStartOpenChannels((HCALL)call, call->fastStartNodes, call->fastStartNodesAck, TRUE);
        m_callset(call, fastStartFinished, TRUE);
        cmcReadyEvent(ctrl);
    }
	else
		rc = RVERROR;

    return rc;
}

void deleteFastStart(callElem*call)
{
    cmElem*app=(cmElem*)emaGetInstance((EMAElement)call);
    int i;
    for (i=0;i<call->fastStartNodesCount;i++)
    {
        pvtDelete(app->hVal,call->fastStartNodes[i]);
        call->fastStartNodes[i]=RVERROR;
    }
    call->fastStartNodesCount=0;
    for (i=0;i<call->fastStartNodesAckCount;i++)
    {
        pvtDelete(app->hVal,call->fastStartNodesAck[i]);
        call->fastStartNodesAck[i]=RVERROR;
    }
    call->fastStartNodesAckCount=0;
}
void addFastStart(callElem*call,int message)
{
    cmElem*app=(cmElem*)emaGetInstance((EMAElement)call);
    HPVT hVal=app->hVal;
    int nodeCount,*nodes;

    if (m_callget(call,callInitiator))
    {
        nodeCount=call->fastStartNodesCount;
        nodes=call->fastStartNodes;

        /* Make sure it's a setup message - otherwise we shouldn't add the fast start nodes */
        if (pvtGetChildTagByPath(hVal,message,"message",1) != cmQ931setup)
            return;
    }
    else
    {
        if (call->fastStartState == fssRej)
        {
            int tmpNodeId;
            /* reject the fast start - was approved with no channels */
            __pvtBuildByFieldIds(tmpNodeId, hVal, message,
                {_q931(message)
                _anyField
                _q931(userUser)
                _q931(h323_UserInformation)
                _q931(h323_uu_pdu)
                _q931(h323_message_body)
                _anyField
                _q931(fastConnectRefused)
                LAST_TOKEN},0,NULL);
            return;
        }
        if (!m_callget(call, fastStartFinished))
            return;
        nodeCount=call->fastStartNodesAckCount;
        nodes=call->fastStartNodesAck;
    }

    if (nodeCount > 0)
    {
        int i;
        int tmpNodeId;

        /* first, we'll check if we already have a fast start field in the message */
        __pvtGetByFieldIds(tmpNodeId, hVal, message,
            {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu)
            _q931(h323_message_body) _anyField _q931(fastStart) LAST_TOKEN},NULL,NULL,NULL);
        if (tmpNodeId >= 0)
        {
            /* we have a FS element in the message already. this may be because the call was transferred,
            or it may be because the user used setParam() to set a faststart element. we can do one of 
            two things: either delete the node and put our own FS content, or we could keep the existing
            FS and return. adding to the current FS message is out of the question. */
#if 1
            /* we assume that the user will not use both setParam() and FS functions, or that this is a
            transfer, and we better update our information */
            pvtDelete(hVal, tmpNodeId);
#else
            /* we assume that we should keep whatever was set, and leave things up to the user */
            return;
#endif
        }

        /* Build the fast start SEQUENCE OF node */
        __pvtBuildByFieldIds(tmpNodeId, hVal, message,
            {_q931(message) _anyField _q931(userUser) _q931(h323_UserInformation) _q931(h323_uu_pdu)
             _q931(h323_message_body) _anyField _q931(fastStart) LAST_TOKEN},0,NULL);

        for (i = 0; i < nodeCount; i++)
        {
            int iBufLen;
            BYTE* encodeBuffer;

            getEncodeDecodeBuffer(app->encodeBufferSize, &encodeBuffer);
#ifndef NOLOGSUPPORT
            if  (logGetDebugLevel(app->logFastStart)>1)
            {
                logPrint(app->logFastStart, RV_DEBUG,
                            (app->logFastStart, RV_DEBUG, (m_callget(call,callInitiator))?"Suggested %s":"Accepted %s", "faststart channel encoded:"));
                pvtPrintStd(hVal , nodes[i], (int)app->logFastStart);
            }
#endif
            if (! (cmEmEncode(hVal, nodes[i], encodeBuffer, app->encodeBufferSize, &iBufLen) < 0) )
            {
                if  (logGetDebugLevel(app->logFastStart)>1)
                {
                    logPrintFormat(app->logFastStart, RV_DEBUG, "Binary:");
                    printHexBuff(encodeBuffer, iBufLen, app->logFastStart);
                }

                /* That's it, now we can add the encoded OLC message to the SETUP message */
                pvtAdd(hVal,tmpNodeId,-800,iBufLen,(char*)encodeBuffer,NULL);
            }
            else
                logPrintFormat(app->logFastStart, RV_ERROR, "Encoding Problems!");

        }
    }
}

#define MAX_FS_SESSION_ID 10

/************************************************************************
 * cmCallFastStartOpenChannels
 * purpose: Set the answered information to be sent for a fast start
 *          proposal and open the channels on the destination side of the
 *          call. This function should be called after receiving
 *          cmEvCallFastStart() or cmEvCallFastStartSetup().
 *          It is automatically called on the initiator of the call.
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
    IN BOOL     origin)
{
    callElem* call=(callElem*)hsCall;
    HPVT hVal;
    cmElem* app;
    int i;
    int ret = OK;
    int rtcpAddressID[MAX_FS_SESSION_ID];
    int rtpAddressID[MAX_FS_SESSION_ID];
    HAPP hApp=(HAPP)emaGetInstance((EMAElement)hsCall);
    if (!hApp) return RVERROR;
    hVal=cmGetValTree(hApp);
    app=(cmElem*)hApp;

    if (!hsCall || !hApp) return RVERROR;

    if ((call->fastStartState==fssAck) || (call->fastStartState==fssRej))
        return RVERROR;

    cmiAPIEnter(hApp,"cmCallFastStartOpenChannels(hsCall=0x%lx)",hsCall);
    if (emaLock((EMAElement)hsCall))
    {
        /*Go through all proposed channels and collect RTCP addresses*/
        /*One RTCP address per sessionID*/
        memset(rtcpAddressID,0xff,sizeof(rtcpAddressID));
        memset(rtpAddressID,0xff,sizeof(rtpAddressID));

        for (i=0;(proposed[i]>=0)&&(i<MAX_FS_SESSION_ID);i++)
        {
            int inReverseLCPNodeId,inForwardLCPNodeId;
            int inH2250ParametersNodeId;
            BOOL nullDataType;
            INT32 sessionID;

            inReverseLCPNodeId=pvtGetChild(hVal, proposed[i], __h245(reverseLogicalChannelParameters),NULL);
            inForwardLCPNodeId=pvtGetChild(hVal, proposed[i], __h245(forwardLogicalChannelParameters),NULL);

            nullDataType=(pvtGetChild2(hVal, inForwardLCPNodeId, __h245(dataType),__h245(nullData))>=0);
            if (inReverseLCPNodeId >= 0)
                inH2250ParametersNodeId=pvtGetChild2(hVal, inReverseLCPNodeId, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));
            else
                inH2250ParametersNodeId=pvtGetChild2(hVal, inForwardLCPNodeId, __h245(multiplexParameters), __h245(h2250LogicalChannelParameters));

            if (pvtGetChildValue(hVal, inH2250ParametersNodeId,__h245(sessionID),&sessionID, NULL)>=0)
                if (sessionID<=MAX_FS_SESSION_ID)
                {
                    if (rtcpAddressID[sessionID]<0)
                    {
                        int rtcpNodeID=pvtGetChild(hVal,inH2250ParametersNodeId,__h245(mediaControlChannel),NULL);
                        if (rtcpNodeID>=0)
                        {
                            rtcpAddressID[sessionID]=pvtAddRoot(hVal,app->hAddrSynH245,0,NULL);
                            pvtSetTree(hVal,rtcpAddressID[sessionID],hVal,rtcpNodeID);
                        }
                    }
                    if (rtpAddressID[sessionID]<0)
                    {
                        int rtpNodeID=pvtGetChild(hVal,inH2250ParametersNodeId,__h245(mediaChannel),NULL);
                        if (rtpNodeID>=0)
                        {
                            rtpAddressID[sessionID]=pvtAddRoot(hVal,app->hAddrSynH245,0,NULL);
                            pvtSetTree(hVal,rtpAddressID[sessionID],hVal,rtpNodeID);
                        }
                    }
                }
        }

        {
			BOOL controlReported = FALSE;
            BYTE lcnAllocationBuff[(MAX_FASTSTART_CHANNELS+1)/sizeof(char)];
            memset(lcnAllocationBuff,0,sizeof(lcnAllocationBuff));

            /*channelElem** f_channel=cmiGetChannelsCollectionForCall(call);*/
            call->fastStartState=fssAck;
            for (i=0;(accepted[i]>=0)&&(i<call->fastStartNodesAckCount);i++)
            {
                int inReverseLCPNodeId,inForwardLCPNodeId;
                int inLCPNodeId;
                BOOL nullDataType;
                channelElem* channel;
                confDataType type = (confDataType)0;
                HCONTROL control;

                /* Allocate the channel */
                control = cmiGetControl((HCALL)call);
                channel = allocateChannel(control);
                if (!channel)
                {
                    /* Don't continue... */
                    ret = RESOURCES_PROBLEM;
                    break;
                }
                channel->fastStartChannelIndex = i;

				if (!controlReported)
				{
					/* Notify application, that the call is in cmControlStateFastStart */
					cmiReportControl(hsCall,cmControlStateFastStart,(cmControlStateMode)0);

					controlReported = TRUE;
				}

                /* check the channel direction, only proposal for outgoing channels have reverse parameters*/
                inReverseLCPNodeId=pvtGetChild(hVal, accepted[i], __h245(reverseLogicalChannelParameters),NULL);
                inForwardLCPNodeId=pvtGetChild(hVal, accepted[i], __h245(forwardLogicalChannelParameters),NULL);

                nullDataType=(pvtGetChild2(hVal, inForwardLCPNodeId, __h245(dataType),__h245(nullData))>=0);

                if (nullDataType) /*Incoming channel*/
                    channel->origin=(origin)?FALSE:TRUE;
                else
                    channel->origin=(origin)?TRUE:FALSE;

                if (inReverseLCPNodeId>=0 &&
                    inForwardLCPNodeId>=0 && !nullDataType)
                    channel->isDuplex=TRUE;

                /* Get the logical channel number */
                pvtGetChildValue(hVal, accepted[i], __h245(forwardLogicalChannelNumber),(INT32 *)&(channel->myLCN), NULL);
                if (channel->isDuplex)
                    channel->rvrsLCN = channel->myLCN;
                setBit(lcnAllocationBuff,channel->myLCN%(MAX_FASTSTART_CHANNELS+1),1);

                if (inForwardLCPNodeId>=0)
                    pvtGetChildValue(hVal,inForwardLCPNodeId,__h245(portNumber),(INT32 *)&channel->remotePortNumber,NULL);
				
				inLCPNodeId=(inReverseLCPNodeId>=0)?inReverseLCPNodeId:inForwardLCPNodeId;
				
                if (inLCPNodeId>=0)
                {
                /* Now that we're in the right parameters for this channel -
                    let's get down to the business of opening this channel for the application */
                    int inDataTypeId;
                    int tmpNodeId;
                    
                    /* Get the dataType of this channel and store it */
                    inDataTypeId=pvtGetChild(hVal, inLCPNodeId,__h245(dataType),NULL);
                    channel->dataTypeID=pvtAddRoot(hVal,app->h245DataType,0,NULL);
                    pvtSetTree(hVal,channel->dataTypeID,hVal, inDataTypeId);

                    /* Get the session ID of this channel */
                    __pvtGetByFieldIds(tmpNodeId, hVal, inLCPNodeId,
                        {_h245(multiplexParameters) _h245(h2250LogicalChannelParameters) _h245(sessionID) LAST_TOKEN}, NULL, (INT32 *)&channel->sid, NULL);

                    if ((channel->sid > 0) && (channel->sid <= MAX_FS_SESSION_ID))
                    {
                        int* addrNodeId;

                        /* Get the proposed RTP and RTCP addresses for this channel, and
                           insert them into the channel object */
                        if (rtpAddressID[channel->sid] >= 0)
                        {
                            if (origin) addrNodeId = &channel->recvRtpAddressID;
                            else        addrNodeId = &channel->sendRtpAddressID;
                            if ((*addrNodeId) < 0) *addrNodeId = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);
                            pvtSetTree(hVal, *addrNodeId, hVal, rtpAddressID[channel->sid]);
                        }
                        if (rtcpAddressID[channel->sid]>=0)
                        {
                            if (origin) addrNodeId = &channel->recvRtcpAddressID;
                            else        addrNodeId = &channel->sendRtcpAddressID;
                            if ((*addrNodeId) < 0) *addrNodeId = pvtAddRoot(hVal, app->hAddrSynH245, 0, NULL);
                            pvtSetTree(hVal, *addrNodeId, hVal, rtcpAddressID[channel->sid]);
                        }
                    }

                    /* Find a partner for this channel if there is one */
                    if (channel->origin)
                        channel->partner = getInChanBySID(control, channel->sid);
                    else
                        channel->partner = getOutChanBySID(control, channel->sid);
                    if (channel->partner != NULL) 
                    {
                        /* Link it the other way around as well. No need to lock the second
                           channel, since the channels are locked by their call already */
                        channel->partner->partner = channel;
                    }
                    
                    
                    /* Lock this channel - we're about to make callbacks in this place */
                    if(emaLock((EMAElement)channel))
                    {
                        if (app->cmMySessionEvent.cmEvCallNewChannel)
                        {
                            HAPPCALL haCall = emaGetApplicationHandle((EMAElement)call);
                            HAPPCHAN haChan = NULL;
                            int nesting;
                            
                            cmiCBEnter(hApp, "cmEvCallNewChannel:%s: haCall=0x%p, hsCall=0x%p. hsChan=0x%p",
                                (channel->origin ? "OUT" : "IN"), haCall, call, channel);
                            
                            nesting = emaPrepareForCallback((EMAElement)channel);
                            app->cmMySessionEvent.cmEvCallNewChannel(haCall, (HCALL)call, (HCHAN)channel, &haChan);
                            emaSetApplicationHandle((EMAElement)channel, (void*)haChan);
                            emaReturnFromCallback((EMAElement)channel, nesting);
                            
                            cmiCBExit(hApp, "cmEvCallNewChannel:%s: haChan=0x%p.",
                                (channel->origin ? "OUT" : "IN"), haChan);
                        }
                        
                        if (!emaWasDeleted((EMAElement)channel))
                        {
                            if (app->mibEvent.h341AddNewLogicalChannel)
                                app->mibEvent.h341AddNewLogicalChannel(app->mibHandle, (HCHAN)channel);
                            
                            cmcCallChannelParametersCallback((HAPP)app, (HCHAN)channel, channel->dataTypeID, &type);
                        }
                        
                        if (!emaWasDeleted((EMAElement)channel))
                        {
                            /* Now call the dataType callback if we still have this channel */
                            cmcCallDataTypeHandleCallback((HAPP)app, (HCHAN)channel, channel->dataTypeID, type);
                        }
                        
                        if (!emaWasDeleted((EMAElement)channel))
                        {
                            /* Get the session ID of this channel and the node of the parameters -
                            we'll need it later for RTP and RTCP addresses */
                            
                            /* Find the nodeId of the logical channel parameters from the 
                            proposed part of the call - we want to notify the application
                            about the remote's RTP and RTCP addresses */
                            int paramsNodeId;
                            
                            if (!origin)
                            {
                                /* This means we're the side that should send the CONNECT - we want the
                                proposed nodeId */
                                paramsNodeId = pvtGetChild(hVal, proposed[i], __h245(reverseLogicalChannelParameters), NULL);
                                if (paramsNodeId < 0)
                                    paramsNodeId = pvtGetChild(hVal, proposed[i], __h245(forwardLogicalChannelParameters), NULL);
                            }
                            else
                            {
                                /* This means we're the side that received the CONNECT. We already have the parameters
                                node id in inLCPNodeId. */
                                paramsNodeId = inLCPNodeId;
                            }
                            
                            /* Now let's find the parameters... */
                            __pvtGetNodeIdByFieldIds(paramsNodeId, hVal, paramsNodeId, {_h245(multiplexParameters) _h245(h2250LogicalChannelParameters) LAST_TOKEN});
                            
                            /* Notify the application about the remote's RTP and RTCP addresses */
                            cmcCallAddressCallbacks(app, channel, paramsNodeId, origin);
                        }
                        
                        if (!emaWasDeleted((EMAElement)channel))
                        {
                            /* Now that we're done - notify the state of this channel is connected */
                            channel->state = established;
                            notifyChannelState(channel,cmChannelStateConnected,cmChannelStateModeOn);
                        }
                        
                        /* We're done with all the callbacks for this channel - unlock it */
                        emaUnlock((EMAElement)channel);
                    }

                    /* If someone deleted this call, we should get out of this for loop */
                    if (emaWasDeleted((EMAElement)hsCall))
                        break;
                }
            }
            /*If the number of fast start proposals was less then MAX_FASTSTART_CHANNELS then at least one range will be available */
            call->lcnOut=(get1st0BitNumber(lcnAllocationBuff, 1, MAX_FASTSTART_CHANNELS)-1);
        }

        /* No need for the addresses anymore - delete them */
        for (i = 0; i < MAX_FS_SESSION_ID; i++)
        {
            if (rtcpAddressID[i] >= 0)
                pvtDelete(hVal, rtcpAddressID[i]);
            if (rtpAddressID[i] >= 0)
                pvtDelete(hVal, rtpAddressID[i]);
        }
        
        emaUnlock((EMAElement)hsCall);
    }

    cmiAPIExit(hApp,"cmCallFastStartOpenChannels: [%d]", ret);
    return ret;
}


int  fastStartInit(cmElem*app, int maxCalls, int proposed, int accepted)
{
    app->logFastStart=logRegister(app->logMgr,"FASTSTART","Fast Start Messages");

    /* Allocate the buffer of fast start nodes */
    /* add one to values, last in array must be -1 */
    app->fastStartBuff=(int *)malloc(maxCalls*(proposed+accepted+2)*sizeof(int));
    app->fastStartBuff2=(BYTE *)malloc(maxCalls*(accepted+1)*sizeof(BYTE));

    /* Set the values in the stack's instance */
    app->maxFsProposed=proposed;
    app->maxFsAccepted=accepted;

    return ((app->fastStartBuff != NULL)&&(app->fastStartBuff2 != NULL));
}

void fastStartEnd(cmElem*app)
{
    if (app->fastStartBuff)
        free(app->fastStartBuff);
    if (app->fastStartBuff2)
        free(app->fastStartBuff2);
    /*logUnregister(app->logFastStart);*/
}

void fastStartCallInit(callElem*call)
{
    cmElem* app=(cmElem*)emaGetInstance((EMAElement)call);
    int callNumber;
    call->fastStartState=fssNo;
    call->fastStartNodesCount=0;
    call->fastStartNodesAckCount=0;
    call->lcnOut=0;

    /* Find out the call's index: We'll use that to access it's fast start channels */
    callNumber = emaGetIndex((EMAElement)call);

    /* Set the pointers to the right places inside the fast start buffer */
    call->fastStartNodes = 
        app->fastStartBuff + (callNumber * (app->maxFsProposed + app->maxFsAccepted + 2));
    call->fastStartNodesAck = call->fastStartNodes + app->maxFsProposed + 1;
    call->fastStartIndexes = app->fastStartBuff2 + (callNumber * (app->maxFsAccepted + 1));

    /* Clear the nodes to -1 */
    memset(call->fastStartNodes, 0xff, sizeof(int)*(app->maxFsProposed + app->maxFsAccepted + 2));
    memset(call->fastStartIndexes, 0xff, sizeof(BYTE)*(app->maxFsAccepted + 1));
}



#ifdef __cplusplus
}
#endif
