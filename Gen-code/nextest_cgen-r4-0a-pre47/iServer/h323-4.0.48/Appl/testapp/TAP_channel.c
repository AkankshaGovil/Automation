/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_channel.c
 *
 * This file provides functions for handling the channels of a call.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "TAP_utils.h"
#include "TAP_channel.h"




/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * DisplayChannelInfo
 * purpose : Display the information of a channel inside a listbox widget
 *           This function is called by Channel.DisplayChannelList or call related C functions
 * input   : CurrentChannel - Channel handle of the application
 *           ListboxWidget  - Name of the listbox widget to fill
 * output  : none
 * return  : none
 ********************************************************************************************/
void DisplayChannelInfo(ChannelInfo* CurrentChannel, char* ListboxWidget)
{
    CallInfo* Call;
    int isDuplex;
    int channelId;

    Call = CurrentChannel->call;

    /* Get the channel's parameters */
    isDuplex = cmChannelIsDuplex(CurrentChannel->hChan);
    channelId = cmChannelSessionId(CurrentChannel->hChan);

    TclExecute("%s delete 1.0 end\n"
               "%s insert end {"
               "Call: %d, 0x%p\n"
               "Channel ID: %d\n"
               "Type: %s}",
               ListboxWidget,
               ListboxWidget,
               Call->counter, Call,
               channelId,
               (isDuplex) ? "Bidirectional" : "Unidirectional");
}


/********************************************************************************************
 * DisplayChannelList
 * purpose : Display the list of channels inside the channels window by the selected call
 * input   : Call   - Call information holding the channels
 *           Widget - Listbox widget to fill with channel's information
 *                    If set to NULL, the default main channels list is used
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int DisplayChannelList(CallInfo* Call, char* Widget)
{
    int status;
    ChannelInfo* chan;
    static char* mainChans = (char *)".test.chan";
    char* chanWidget;
    BOOL Mark = FALSE;

    if (Widget == NULL)
    {
        char * result;
        chanWidget = mainChans;

        /* Make sure the selected call is the one that we're updating */
        TclExecute(".test.calls.list selection includes [Call.FindCallIndex %d]",
                   Call->counter);
        result = Tcl_GetStringResult(interp);
        if(result[0] == '0') return 0;
    }
    else
        chanWidget = Widget;

    /* Delete channels list */
    TclExecute("%s.inList delete 0 end\n"
               "%s.outList delete 0 end\n",
               chanWidget, chanWidget);

    /* Rewrite the channels information inside the list */
    chan = Call->firstChannel;
    if(chan != NULL) Mark = chan->mark; /* coloring system. used for printing */
    while (chan != NULL)
    {
        BOOL isOutgoing;
        int isDuplex;

        /* Get channel parameters */
        status = cmChannelGetOrigin(chan->hChan, &isOutgoing);
        isDuplex = cmChannelIsDuplex(chan->hChan);

        if( (chan->mark == Mark) && (status >= 0) )
        {
            ChannelInfo * SameSession;
            BOOL found = FALSE;
            int sessionID = cmChannelSessionId(chan->hChan);

            /* Display channel's information */
            TclExecute("%s.%s insert end {%d 0x%p %s %s %s}",
                chanWidget,
                (isOutgoing ? "outList" : "inList"),
                chan->channelId,
                chan,
                chan->dataType,
                (isDuplex ? "Duplex" : "Simplex"),
                CMChannelState2String(chan->state));
            SameSession = chan->nextChannel;
            while(SameSession!=NULL)
            {
                BOOL SameSessOutgoing;
                cmChannelGetOrigin(SameSession->hChan, &SameSessOutgoing);
                if((SameSessOutgoing != isOutgoing) &&
                   (cmChannelSessionId(SameSession->hChan) == sessionID) &&
                   (SameSession->mark == Mark))
                {
                    TclExecute("%s.%s insert end {%d 0x%p %s %s %s}",
                        chanWidget,
                        (isOutgoing ? "inList" : "outList"),
                        SameSession->channelId,
                        SameSession,
                        SameSession->dataType,
                        (isDuplex ? "Duplex" : "Simplex"),
                        CMChannelState2String(chan->state));
                    SameSession->mark = !Mark;
                    found = TRUE;
                    break;
                }
                SameSession = SameSession->nextChannel;
            }
            if(!found) TclExecute("%s.%s insert end {}",
                chanWidget,
                (isOutgoing ? "inList" : "outList"));
            chan->mark = !Mark;
        }
        chan = chan->nextChannel;
    }

    return TCL_OK;
}


/********************************************************************************************
 * ChannelCreate
 * purpose : Preparing application handle for new channel(fast start or regular)
 * input   : CmHChannel    - CM handle for the channel
 *                           Can be set to NULL
 *           IsIncoming    - Check if it is incoming or outgoing channel (for fast start)
 *           IsScript      - Is this channel controlled by scripts or the appication's GUI
 *           CurrentCall   - Application handle for the call
 * output  : none
 * return  : Channel handle allocated
 *           NULL if failed
 ********************************************************************************************/
ChannelInfo* ChannelCreate(HCHAN        CmHChannel,
                           BOOL         IsOutGoing,
                           BOOL         IsScript,
                           CallInfo*    CurrentCall)
{
    ChannelInfo* LastChannel;
    ChannelInfo* CurrentChannel;

    /* Create a new channel handle */
    CurrentChannel = (ChannelInfo *)AppAlloc(sizeof(ChannelInfo));
    memset(CurrentChannel, 0, sizeof(ChannelInfo));
    CurrentChannel->rtpSession = -1;
    CurrentChannel->dataTypeNode = -1;

    /* Connect this channel to the list of current channels for the call */
    LastChannel = (ChannelInfo *)CurrentCall->lastChannel;
    CurrentCall->numOfChannels++;
    if (LastChannel == NULL)
    {
        CurrentCall->firstChannel = CurrentChannel;
        CurrentCall->lastChannel = CurrentChannel;
    }
    else
    {
        LastChannel->nextChannel = CurrentChannel;
        CurrentChannel->prevChannel = LastChannel;
        CurrentCall->lastChannel = CurrentChannel;
    }

    /* Set channel information in application database */
    CurrentChannel->hChan = CmHChannel;
    CurrentChannel->call = CurrentCall;

    /* Set the direction of the channel */
    CurrentChannel->isOutgoing = IsOutGoing;
    CurrentChannel->scriptChannel = IsScript;
    CurrentChannel->mark = CurrentCall->firstChannel->mark;

    if (CmHChannel != NULL)
        CurrentChannel->channelId = cmChannelSessionId(CmHChannel);

    return CurrentChannel;
}


/********************************************************************************************
 * FreeChannel
 * purpose : Deallocate a channel from the application's database
 * input   : CurrentChannel - Channel to free
 * output  : none
 * return  : result from CM_ChannelClose
 ********************************************************************************************/
int FreeChannel(ChannelInfo*   CurrentChannel)
{
    CallInfo*       call;
    ChannelInfo*    prev;
    ChannelInfo*    next;
    int             status;

    call = CurrentChannel->call;
    prev = CurrentChannel->prevChannel;
    next = CurrentChannel->nextChannel;

    if(call != NULL)
    {
        if(call->numOfChannels >= 0) call->numOfChannels--;

        /* Fix call's channels list */
        if (call->firstChannel == CurrentChannel)
            call->firstChannel = next;
        if (call->lastChannel == CurrentChannel)
            call->lastChannel = prev;

        /* Fix linkage between neighbor channels */
        if (prev != NULL)
            prev->nextChannel = next;
        if (next != NULL)
            next->prevChannel = prev;
    }

    /* Close the RTP session */
    RTP_TestClose(CurrentChannel->rtpSession);
    pvtDelete(cmGetValTree(hApp), CurrentChannel->dataTypeNode);

    /* Close the channel in the CM */
    if (CurrentChannel->hChan != NULL)
        status = cmChannelClose(CurrentChannel->hChan);
    else
        status = 0;

    AppFree(CurrentChannel);

    return status;
}


/********************************************************************************************
 * OpenChannel
 * purpose : Open an outgoing channel
 * input   : Call           - Call to associate with the channel
 *           MediaType      - Type of media to use for the channel
 *           DataType       - Data type parameter for the channel
 *           MimicChannel   - Pointer of a channel we mimic. NULL if not mimicing
 *           ReplaceChannel - Pointer of a channel we replace. NULL not replacing
 * output  : none
 * return  : TCL_OK on success
 ********************************************************************************************/
int OpenChannel(CallInfo *    Call,
                char *        DataType,
                ChannelInfo * MimicChannel,
                ChannelInfo * ReplaceChannel)
{
    ChannelInfo*       NewChan;
    int                status;

    /* Make sure we're not in an endless loop of mimicing channels */
    if (MimicChannel != NULL)
    {
        HAPPCHAN haSameSession = NULL;
        HCHAN hsSameSession = NULL;
        cmChannelSameSession(MimicChannel->hChan, &haSameSession, &hsSameSession);
        if(haSameSession != NULL) return 0;
    }

    /* Increase the number of channels for this call */
    NewChan = ChannelCreate(NULL, TRUE, FALSE, Call);

    /* Create and open channel */
    status = cmChannelNew(Call->hsCall, (HAPPCHAN) NewChan, (LPHCHAN)&NewChan->hChan);

    if (status >= 0)
    {
        /* See if we're mimicing this channel */
        if (MimicChannel == NULL)
        {
            /* Open an RTP session for this channel */
            if(Call->action == RTP_Replay)
                NewChan->rtpSession = RTP_TestOpen("testChannel");
            if (ReplaceChannel != NULL)
                cmChannelReplace(NewChan->hChan, ReplaceChannel->hChan);
        }
        else
        {
            /* Channel is mimiced */
            NewChan->rtpSession = MimicChannel->rtpSession;
            RTP_TestSetAction(NewChan->rtpSession, Call->action);
            RTP_TestOpenSecondChannel(NewChan->rtpSession);
        }

        /* set RTCP address  and open the channel*/
        if (DataType[0] == 't')
        {
            /* Data channel (Duplex) */
            cmTransportAddress ta={0,0x0e2472c0,(UINT16)1503/*port*/,cmTransportTypeIP};
            cmChannelDuplex(NewChan->hChan);
            cmChannelSetDuplexAddress(NewChan->hChan,ta,1,(char *)"1",FALSE);
        }
        else
            cmChannelSetRTCPAddress(NewChan->hChan, 0,
                (UINT16) (RTP_TestGetLocalPort(NewChan->rtpSession) + 1));
        
        /* set data type */
        strncpy(NewChan->dataType, DataType, sizeof(NewChan->dataType));

        if (MimicChannel)
        {
            if (MimicChannel->dynPayloadType != 0)
                cmChannelSetDynamicRTPPayloadType(NewChan->hChan, MimicChannel->dynPayloadType);

            /* Since this is a mimiced channel, we'll open it in exactly the same manner as the original channel */
            status = cmChannelOpenDynamic(NewChan->hChan, MimicChannel->dataTypeNode, MimicChannel->hChan, NULL, FALSE);
        }
        else
            status = cmChannelOpen(NewChan->hChan, DataType, NULL, NULL, 0);
        
        TclExecute("call:Log %d {ChannelOpen. result=%s}", Call->counter, Status2String(status));
    }
    else
    {
        FreeChannel(NewChan);
        TclExecute("call:Log %d {ChannelNew. result=%s}", Call->counter, Status2String(status));
    }

    /* Add the new channel to the channels list on the main window */
    return DisplayChannelList(Call, NULL);
}


/********************************************************************************************
 * AutoMimic
 * purpose : Automatically open a channel for an incoming channel
 * input   : InChan - The incoming channel to mimic
 * output  : none
 * return  : none
 ********************************************************************************************/
void AutoMimic(ChannelInfo* InChan)
{
    CallInfo * CurrentCall;
    char *     isAutoMimic;
    int        isDuplex;
    char       DataType[20];

    /* Check if auto-mimic option is set or not */
    isAutoMimic = TclGetVariable("app(options,autoMimic)");
    if ((isAutoMimic == NULL) || (atoi(isAutoMimic) != 1)) return;

    /* We only mimic unidirectional channels */
    isDuplex = cmChannelIsDuplex(InChan->hChan);
    if (isDuplex) return;

    /* Check this channel's information and mimic them */
    CurrentCall = InChan->call;

    strncpy(DataType, InChan->dataType, sizeof(DataType));

    /* Open the channel */
    OpenChannel(CurrentCall,
                DataType,
                InChan,
                NULL);
}








/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * Channel_OpenOutgoingWindow
 * purpose : Open the "New Channel" window
 *           This function is called from the TCL when the "New Channel" button is pressed in
 *           the main application window.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_OpenOutgoingWindow(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo* CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    TclExecute("Window open .newchan\n"
               ".newchan.callInfo configure -text {%d 0x%p}",
               CurrentCall->counter, CurrentCall);
    TclExecute("Call:addWindowToList 0x%p .newchan", CurrentCall);
    return TCL_OK;
}


/********************************************************************************************
 * Channel_ConnectOutgoing
 * purpose : Connect a new outgoing channel
 *           This function is called from the TCL when the "Ok" button is pressed in the
 *           "New Channel" window.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ConnectOutgoing(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char *     dataType;
    int        status;
    CallInfo * CurrentCall;
    ChannelInfo * MimicChan = NULL;
    ChannelInfo * ReplaceChan = NULL;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;
    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

   /* set the dataType */
    dataType = TclGetVariable("tmp(newchan,dataType)");

    /* see if replace or mimic operation is called for */
    if(argv[2][0])
    {/* argv[2] is not "" */
        if(argv[2][0] == 'S') /* "SameAs" -> mimic chan */
            sscanf(argv[3], "%*d 0x%p", &MimicChan);
        else if(argv[2][0] == 'R') /* "Replace" -> replace chan */
            sscanf(argv[3], "%*d 0x%p", &ReplaceChan);
        else return TCL_ERROR; /* something not right with the command */
    }
    status = OpenChannel(CurrentCall, dataType, MimicChan, ReplaceChan);
    if((argv[2][0] == 'R') && (status >= 0))
    /* The app is reponsible for opening the channle, so here it is: */
        TclExecute("after 3000 {Channel.Drop \"\" {%s} 3}", argv[3]);
    return status;
}


/********************************************************************************************
 * Channel_DisplayChannelList
 * purpose : Display the list of channels inside the channels window by the selected call
 * syntax  : Channel.DisplayChannelList <callInfo> <frameWidget>
 *           <callInfo>     - Call information (counter and handle)
 *           <frameWidget>  - Frame widget holding channel listboxes
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_DisplayChannelList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char* Widget;
    CallInfo* CurrentCall;

    /* Make sure we've got a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    if (sscanf(argv[1], "%*d 0x%p:", &CurrentCall) != 1) return TCL_OK;

    /* Check if we have the second paremeter (listbox widget to display in) */
    if ((argc < 2) || (argv[2] == NULL) || (strlen(argv[2]) == 0))
        Widget = NULL;
    else
        Widget = argv[2];

    /* Display the channels */
    return DisplayChannelList(CurrentCall, Widget);
}




/********************************************************************************************
 * cmEvChannelStateChanged
 * purpose : the function that invoked when event ChannelStateChange occurs
 * input   : haChan    - Channel struct of the application
 *         : hsChan    - handle for the channel
 *         : state     - current state of the channel
 *         : stateMode - New state mode. This parameter describes why or how the channel
 *                       enters a new state
 * output  : none
 * return  : negative on error
 ********************************************************************************************/
int RVCALLCONV cmEvChannelStateChanged(IN HAPPCHAN haChan,
                                       IN HCHAN    hsChan,
                                       IN UINT32   state,
                                       IN UINT32   stateMode)
{
    ChannelInfo* CurrentChannel;
    CallInfo* Call;
    int AutoAction;
    int  isDuplex;
    BOOL isOutgoing;

    CurrentChannel = (ChannelInfo *)haChan;
    Call = CurrentChannel->call;

    CurrentChannel->state = (cmChannelState_e)state;

    if (CurrentChannel->scriptChannel == TRUE)
    {
        /* Advanced script handling */
        TclExecute("script:cb {cb:cm:EvChannelStateChanged 0x%p %s %s}",
                   CurrentChannel,
                   CMChannelState2String((cmChannelState_e)state),
                   CMChannelStateMode2String((cmChannelStateMode_e)stateMode));

        if (state == cmChannelStateIdle)
        {
            FreeChannel(CurrentChannel);
            if(Call->numOfChannels >= 0)
                /* Redisplay the list of channels */
                DisplayChannelList(Call, NULL);
        }
        return 0;
    }

    /* get channelid and direction and afterwards searching the channel in the channels array */
    cmChannelGetOrigin(hsChan, &isOutgoing);
    isDuplex = cmChannelIsDuplex(hsChan);

    switch (state)
    {
        case cmChannelStateOffering:
        {
            HCHAN           hsChanSS = NULL;
            HAPPCHAN        haChanSS = NULL;
                
            CurrentChannel->channelId = cmChannelSessionId(hsChan);
            
            cmChannelSameSession(hsChan, &haChanSS, &hsChanSS);
            /* Check to see if this channel has another of the same session, which was not already
              connected to it in cmEvCallNewChannel, and which has an RTP session */
            if((hsChanSS) && (Call->action == RTP_Replay) &&
                (CurrentChannel->rtpSession != ((ChannelInfo *) haChanSS)->rtpSession) &&
                (((ChannelInfo *) haChanSS)->rtpSession >= 0))
            {
                if (CurrentChannel->rtpSession >= 0)
                {
                    /* We've got an open session already - add it as a second channel to that session.
                       This happens when we're the side that initiated the OLC, which never happens for
                       the test application with real media. */
                    RTP_TestClose(CurrentChannel->rtpSession);
                }
                CurrentChannel->rtpSession = ((ChannelInfo *) haChanSS)->rtpSession;
				RTP_TestSetAction(CurrentChannel->rtpSession, Call->action);
                RTP_TestOpenSecondChannel(CurrentChannel->rtpSession);
            }
            
            /* Make sure we open the incoming channel window */
            AutoAction = atoi(TclGetVariable("app(options,autoAccept)"));
            if (AutoAction == 1)
            {
                /* Automatically accept the channel */
                cmChannelSetAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession)));
                cmChannelSetRTCPAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession) + 1));
                cmChannelAnswer(CurrentChannel->hChan);

                /* Mimic channel opening if we have to */
                AutoMimic(CurrentChannel);
            }
            else if (atoi(TclGetVariable("app(options,popUp)")) != 0)
            {
                /* Manually open the channel (ask user to accept it) */
                TclExecute("Window open .inchan");
                TclExecute(".inchan.callInfo configure -text {0x%p}",
                           CurrentChannel);
                TclExecute("Call:addWindowToList 0x%p .inchan", Call);

                /* Display channel information inside the listbox */
                DisplayChannelInfo(CurrentChannel, (char *)".inchan.chanInfo.txt");
            }
            break;
        }

        case cmChannelStateConnected:
            {
                /* Update the channel ID now that the channel is connected */
                CurrentChannel->channelId = cmChannelSessionId(hsChan);
                /* terminate the inchan window */
                TclExecute("inchan:terminate {0x%p}", CurrentChannel);

                if (CurrentChannel->isOutgoing)
                {
                    /* todo: Put RTP information here using a timer of 4/5 seconds */
                }
                break;
            }

        case cmChannelStateDisconnected:
            {
                break;
            }

        case cmChannelStateDialtone:
            {
                break;
            }

        case cmChannelStateRingBack:
            {
                break;
            }

        case cmChannelStateIdle:
        {
            /* terminate the dropchan window */
            TclExecute("inchan:terminate {0x%p}", CurrentChannel);
            /* free and close the channel */
            FreeChannel(CurrentChannel);
            break;
        }

        default:
        {
            break;
        }
    }
    /* Redisplay the list of channels */
    DisplayChannelList(Call, NULL);
    return 0;
}


/********************************************************************************************
 * Channel_ResponseForOlC
 * purpose : send ACK or REJECT after receiving OLC
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ResponseForOLC(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*       CurrentCall;
    ChannelInfo*    CurrentChannel;

    /* Get channel handle */
    if (sscanf(argv[1], "0x%p", &CurrentChannel) != 1)
        return TCL_OK;

    if (CurrentChannel == NULL) return TCL_OK;
    CurrentCall = CurrentChannel->call;

    /* Check response value */
    if (strcmp(argv[2], "Accept") == 0)
    {
		cmChannelSetAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession)));
		cmChannelSetRTCPAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession) + 1));
        cmChannelAnswer(CurrentChannel->hChan);

        /* Mimic channel opening if we have to */
        AutoMimic(CurrentChannel);
    }
    else
    {
        cmChannelDrop(CurrentChannel->hChan);
    }

    /* Close incoming channel window */
    TclExecute("Window close .inchan");

    /* Refresh list of channels */
    return DisplayChannelList(CurrentCall, NULL);
}


/********************************************************************************************
 * Channel_ResponseForClC
 * purpose : send ACK or REJECT after receiving CLC
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_ResponseForCLC(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    CallInfo*       CurrentCall;
    ChannelInfo*    CurrentChannel;

    /* Get channel handle */
    if (sscanf(argv[1], "0x%p", &CurrentChannel) != 1)
        return TCL_OK;

    if (CurrentChannel == NULL) return TCL_OK;
    CurrentCall = CurrentChannel->call;

    /* Check response value */
    if (strcmp(argv[2], "Accept") == 0)
    {
        cmChannelAnswer(CurrentChannel->hChan);
        cmChannelDrop(CurrentChannel->hChan);
    }
    else
        cmChannelRequestCloseReject(CurrentChannel->hChan);


    /* Close incoming channel window */
    TclExecute("Window close .dropchan");


    /* Refresh list of channels */
    return DisplayChannelList(CurrentCall, NULL);
}


/********************************************************************************************
 * Channel_Drop
 * purpose : drops a logical chennel
 * syntax  : Channel.Drop <channelHandle1> <channelHandle2> <dropReason>
 *           <channelHandle?>   - Channel information (call and channel id)
 *           <dropReason>       - Drop reason for the channel
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Drop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    cmCloseLcnReason DropReason;
    ChannelInfo* CurrentChannel;

    /* Get drop reason */
    DropReason = (cmCloseLcnReason)atoi(argv[3]);

    if (sscanf(argv[1], "%*d 0x%p", &CurrentChannel) == 1)
        if(cmChannelRequestClose(CurrentChannel->hChan, DropReason, NULL)<0)
            TclExecute("test:Log {Incoming channel drop failed}");
        
    if (sscanf(argv[2], "%*d 0x%p", &CurrentChannel) == 1)
        if(cmChannelDropReason(CurrentChannel->hChan, DropReason)<0)
        {
            TclExecute("test:Log {Outgoing channel drop failed}");
        }
    return TCL_OK;
}

/********************************************************************************************
 * Channel_Answer
 * purpose : answers a logical chennel
 * syntax  : Channel.Drop <channelHandle1> <channelHandle2>
 *           <channelHandle?>   - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Answer(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    ChannelInfo* CurrentChannel;

    if (sscanf(argv[1], "%*d 0x%p", &CurrentChannel) == 1)
	{
		cmChannelSetAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession)));
		cmChannelSetRTCPAddress(CurrentChannel->hChan,0,(UINT16)(RTP_TestGetLocalPort(CurrentChannel->rtpSession) + 1));
		cmChannelAnswer(CurrentChannel->hChan);
	}

    return TCL_OK;
}

/********************************************************************************************
 * Channel_MediaLoop
 * purpose : Requests media loop on the specified channel. Sends a maintenanceLoopRequest
 *           message to the remote terminal.
 * syntax  : Channel.MediaLoop <channelHandle1> <channelHandle2>
 *           <channelHandle?>   - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_MediaLoop(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    ChannelInfo* CurrentChannel;

    if (sscanf(argv[1], "%*d 0x%p", &CurrentChannel) == 1)
        cmChannelMediaLoopRequest(CurrentChannel->hChan);

    return TCL_OK;
}

/********************************************************************************************
 * Channel_Rate
 * purpose : Changes the channel rate
 * syntax  : Channel.Rate <rate> <channelHandle1> <channelHandle2>
 *           <rate>           - rate of the channel
 *           <channelHandle?> - Channel information (call and channel id)
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Channel_Rate(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    ChannelInfo* CurrentChannel;
    UINT32 rate = (UINT32) atoi(argv[1]);

    if (sscanf(argv[2], "%*d 0x%p", &CurrentChannel) == 1)
        cmChannelFlowControl(CurrentChannel->hChan, rate);
    if (sscanf(argv[3], "%*d 0x%p", &CurrentChannel) == 1)
        cmChannelFlowControl(CurrentChannel->hChan, rate);
    
    return TCL_OK;
}


/************************************************************************************************
*   cmEvChannelParameters
*
*   This callback Informs the application that a new OpenLogicalChannel request had come, the type
*   of the channel, handles and IDs of the forward and reverse that was allready openned.
*   The user should response to this callback with ack or reject...
*
*   input:  haChan - Application handle to the Channel.
*           hsChan  - CM handle to the Channel.
*           channelName - type of the channel.
*           haChanSameSession - Application handle of the channel that belongs to the same session
*                               as the new one.
*           hsChanSameSession - Stack handle of the channel that belongs to the same session as the
*                               new one.
*           haChanAssociated - Application handle to incoming channel that is to be associated with
*                              the new one (may be NULL). Currently not implemented.
*           hsChanAssociated - Stack handle of incoming channel that is to be associated with the
*                              new one (may be NULL). Currently not implemented.
*           rate - rate of the channel.
*   output: AppHChannel -  The application handle of the channel.
*************************************************************************************************/
int RVCALLCONV cmEvChannelParameters(IN HAPPCHAN haChan,
                                     IN HCHAN    hsChan,
                                     IN char*    channelName,
                                     IN HAPPCHAN haChanSameSession,
                                     IN HCHAN    hsChanSameSession,
                                     IN HAPPCHAN haChanAssociated,
                                     IN HCHAN    hsChanAssociated,
                                     IN UINT32   rate)
{
    ChannelInfo* CurrentChannel = (ChannelInfo *)haChan;
    CallInfo * CurrentCall = CurrentChannel->call;
    char info[256];
    BOOL origin;

    /* there is implementation for this call back in a script */
    if (CurrentChannel->scriptChannel == TRUE)
    {
        /* olc answer will be manually in script mode */
        TclSetVariable("app(options,autoAccept)","0");
        /* show additional information */
        sprintf(info, "ChannelName : %s \n ChannelHandle : 0x%p \n SameSessionChannelHandle : 0x%p \n ChannelRate : %d",
                     channelName, hsChan, hsChanSameSession, rate);
        /* ToDo: fix TCL to correspond */
        TclExecute("script:cb {cb:cm:EvH245ChannelEstablishRequest 0x%p 0x%p {%s}}",
                    CurrentChannel->call,
                    CurrentChannel,
                    info);
    }
    /* Make sure we remember this channel - it will help us to mimic it */
    if ((CurrentChannel != NULL) && (CurrentChannel->dataType[0] == 0) &&
        (channelName != NULL) )
    {
        strcpy(CurrentChannel->dataType, channelName);
    }

    cmCallGetOrigin(CurrentCall->hsCall, &origin);

    /* if we are the origin of a Fast Start call, and we are replaying RTP, we now connect the RTP
      according to the RTP sessions we kept when we created the FS message*/
    if (origin && (CurrentCall->action == RTP_Replay) && CurrentCall->isFastConnectCall && (CurrentChannel->rtpSession < 0) &&
        ((CurrentCall->rtpSessions[0]>=0) || (CurrentCall->rtpSessions[1]>=0) || (CurrentCall->rtpSessions[2]>=0)))
    {
        /* we'll set the RTP session according to the type */
        if(CurrentChannel->dataType[0] == 'g')
        {
            CurrentChannel->rtpSession = CurrentCall->rtpSessions[0];
            CurrentCall->rtpSessions[0] = -1;
        }
        else if(CurrentChannel->dataType[0] == 'h')
        {
            CurrentChannel->rtpSession = CurrentCall->rtpSessions[1];
            CurrentCall->rtpSessions[1] = -1;
        }
        else if(CurrentChannel->dataType[0] == 't')
        {
            CurrentChannel->rtpSession = CurrentCall->rtpSessions[2];
            CurrentCall->rtpSessions[2] = -1;
        }
    }
    return 0;
}


/********************************************************************************************
* cmEvCallNewChannel
* Description : the function that is invoked when the event EvCmCallNewChannel occurs, i.e.
*               when a new channel is recieved for a call.
* Input : haCall    - Application handle to the call.
*         hsCall    - CM handle to the call.
*         hsChan    - CM handle to the channel.
* Output: lphaChan  - Pointer to the application handle to the channel.
* Return: none
********************************************************************************************/
int RVCALLCONV cmEvCallNewChannel(IN  HAPPCALL   haCall,
                                  IN  HCALL      hsCall,
                                  IN  HCHAN      hsChan,
                                  OUT LPHAPPCHAN lphaChan)
{
    ChannelInfo*    CurrentChannel;
    CallInfo*       CurrentCall = (CallInfo*)haCall;
    int             scriptMode;
    BOOL            origin;
    UINT16          port;
    ChannelInfo*    ssHapp;
    HCHAN           ssHs;

    /* Check who handles this channel - the Application or an external script */
    scriptMode = atoi(TclGetVariable("tmp(options,scriptMode)"));

    cmChannelGetOrigin(hsChan, &origin);

    CurrentChannel = ChannelCreate(hsChan, origin, scriptMode, CurrentCall);

    /* if we already opened an RTP session for the opposite channel, include this one in it */
    cmChannelSameSession(hsChan, (HAPPCHAN *)&ssHapp, &ssHs);
    if (ssHapp && (ssHapp->rtpSession>=0))
    {
        /* we did, so we'll just connect this channel to it */
        CurrentChannel->rtpSession = ssHapp->rtpSession;
        RTP_TestSetAction(ssHapp->rtpSession, CurrentCall->action);
        RTP_TestOpenSecondChannel(ssHapp->rtpSession);
    }
    else
    {
        
        if (CurrentCall->action == RTP_Replay)
        {
            if (CurrentCall->isFastConnectCall)
            {
                BOOL callOrigin;
                /* if we are the destination of a Fast Start call, and are replaying RTP, we now connect
                   the RTP. if we are not the destination, we will do it when we know the channel type */

                cmCallGetOrigin(CurrentCall->hsCall, &callOrigin);
                if (!callOrigin)
                {
                    /* we are, so we'll open a new RTP session */
                    CurrentChannel->rtpSession = RTP_TestOpen("testChannel");
                    port = (UINT16) RTP_TestGetLocalPort(CurrentChannel->rtpSession);
                    cmChannelSetAddress(hsChan, 0, port);
                    cmChannelSetRTCPAddress(hsChan, 0, ++port);
                }
            }
            else
            {
                /* Normal call with a normal channel - just open the RTP session for it */
                CurrentChannel->rtpSession = RTP_TestOpen("testChannel");
            }
        }
    }
    
    /* Set the application handle */
    *lphaChan = (HAPPCHAN)CurrentChannel;

    if (scriptMode == TRUE)
    {
        TclExecute("script:cb {cb:cm:EvChannelNew 0x%p 0x%p}",
                   CurrentCall,
                   CurrentChannel);
    }
    return 0;
}

int RVCALLCONV cmEvChannelRequestCloseStatus(IN      HAPPCHAN              haChan,
                                             IN      HCHAN                 hsChan,
                                             IN      cmRequestCloseStatus  status)
{
    ChannelInfo * CurrentChannel = (ChannelInfo *) haChan;
    if(status == cmRequestCloseRequest) /* Received request to close channel */
    {
        int AutoAction;
        AutoAction = atoi(TclGetVariable("app(options,autoDrop)"));
        if (AutoAction == 1)
        {
            /* Automatically drop the channel */
            cmChannelAnswer(hsChan);
            cmChannelDrop(hsChan);
        }
        else
        {
            /* Manual drop - we should interact with the user */
            TclExecute("Window open .inchan .dropchan {Request Channel Close} Channel.ResponseForCLC");
            TclExecute(".dropchan.callInfo configure -text {0x%p}", haChan);
            TclExecute("Call:addWindowToList 0x%p .dropchan",CurrentChannel->call);
            
            /* Display channel information inside the listbox */
            DisplayChannelInfo(CurrentChannel, (char *)".dropchan.chanInfo.txt");
        }
    }
    return 0;
}

int RVCALLCONV cmEvChannelSetAddress(IN      HAPPCHAN            haChan,
                                     IN      HCHAN               hsChan,
                                     IN      UINT32              ip,
                                     IN      UINT16              port)
{
    ChannelInfo* chan=(ChannelInfo*)haChan;

    if (!haChan) return RVERROR;

    RTP_TestSetRemoteRTP(chan->rtpSession, ip, port, FALSE);

    return 0;
}


int RVCALLCONV cmEvChannelSetRTCPAddress(IN      HAPPCHAN            haChan,
                                         IN      HCHAN               hsChan,
                                         IN      UINT32              ip,
                                         IN      UINT16              port)
{
    ChannelInfo* chan=(ChannelInfo*)haChan;
    if (!haChan) return RVERROR;

    RTP_TestSetRemoteRTCP(chan->rtpSession, ip, port, FALSE);

    return 0;
}


int RVCALLCONV cmEvChannelRTPDynamicPayloadType(IN HAPPCHAN haChan,
                                                IN HCHAN    hsChan,
                                                IN INT8     dynamicPayloadType)
{
    ChannelInfo* chan = (ChannelInfo *)haChan;
    if (!haChan) return RVERROR;

    chan->dynPayloadType = dynamicPayloadType;

    return 0;
}






/********************************************************************************************
 *
 *  Stack callbacks with an empty implmenetation
 *
 ********************************************************************************************/


int RVCALLCONV cmEvChannelNewRate(IN HAPPCHAN haChan,
                                  IN HCHAN    hsChan,
                                  IN UINT32   rate)
{ return 0; }

int RVCALLCONV cmEvChannelMaxSkew(IN HAPPCHAN haChan1,
                                  IN HCHAN    hsChan1,
                                  IN HAPPCHAN haChan2,
                                  IN HCHAN    hsChan2,
                                  IN UINT32   skew)
{ return 0; }

int RVCALLCONV cmEvChannelVideoFastUpdatePicture(IN HAPPCHAN haChan,
                                                 IN HCHAN    hsChan)
{ return 0; }

int RVCALLCONV cmEvChannelVideoFastUpdateGOB(IN HAPPCHAN haChan,
                                             IN HCHAN    hsChan,
                                             IN int      firstGOB,
                                             IN int      numberOfGOBs)
{ return 0; }

int RVCALLCONV cmEvChannelVideoFastUpdateMB(IN HAPPCHAN haChan,
                                            IN HCHAN    hsChan,
                                            IN int      firstGOB,
                                            IN int      firstMB,
                                            IN int      numberOfMBs)
{ return 0; }

int RVCALLCONV cmEvChannelHandle(IN HAPPCHAN      haChan,
                                 IN HCHAN         hsChan,
                                 IN int           dataTypeHandle,
                                 IN cmCapDataType dataType)
{
    ChannelInfo* chan = (ChannelInfo *)haChan;
    HPVT hVal = cmGetValTree(hApp);

    if (dataType == cmCapData)
        dataTypeHandle = pvtParent(cmGetValTree(hApp), dataTypeHandle);
    dataTypeHandle = pvtParent(cmGetValTree(hApp), dataTypeHandle);

    if (chan->dataTypeNode < 0)
        chan->dataTypeNode = pvtAddRoot(hVal, NULL, 0, NULL);
    pvtSetTree(hVal, chan->dataTypeNode, hVal, dataTypeHandle);

    return 0;
}

int RVCALLCONV cmEvChannelGetRTCPAddress(IN HAPPCHAN haChan,
                                         IN HCHAN    hsChan,
                                         IN UINT32 * ip,
                                         IN UINT16 * port)
{ return 0; }

int RVCALLCONV cmEvChannelTSTO(IN HAPPCHAN haChan,
                               IN HCHAN    hsChan,
                               IN INT8     isCommand,
                               IN INT8     tradeoffValue)
{ return 0; }

int RVCALLCONV cmEvChannelMediaLoopStatus(IN HAPPCHAN          haChan,
                                          IN HCHAN             hsChan,
                                          IN cmMediaLoopStatus status)
{
    if(status==cmMediaLoopRequest)
        cmChannelMediaLoopConfirm(hsChan);
    return 0;
}

int RVCALLCONV cmEvChannelReplace(IN HAPPCHAN haChan,
                                  IN HCHAN    hsChan,
                                  IN HAPPCHAN haReplacedChannel,
                                  IN HCHAN    hsReplacedChannel)
{ return 0; }

int RVCALLCONV cmEvChannelFlowControlToZero(IN HAPPCHAN haChan,
                                            IN HCHAN    hsChan)
{ return 0; }

int RVCALLCONV cmEvChannelMiscCommand(IN HAPPCHAN               haChan,
                                      IN HCHAN                  hsChan,
                                      IN cmMiscellaneousCommand miscCommand)
{ return 0; }

int RVCALLCONV cmEvChannelTransportCapInd(IN HAPPCHAN haChan,
                                          IN HCHAN    hsChan,
                                          IN int      nodeId)
{ return 0; }

int RVCALLCONV cmEvChannelSetNSAPAddress(IN HAPPCHAN haChan,
                                         IN HCHAN    hsChan,
                                         IN char*    address,
                                         IN int      length,
                                         IN BOOL     multicast)
{ return 0; }

int RVCALLCONV cmEvChannelSetATMVC(IN HAPPCHAN haChan,
                                   IN HCHAN    hsChan,
                                   IN int      portNumber)
{ return 0; }

int RVCALLCONV cmEvChannelRecvMessage(
    IN      HAPPCHAN            haChan,
    IN      HCHAN               hsChan,
    IN      int                 message)
{
    return 0;
}

int RVCALLCONV cmEvChannelSendMessage(
    IN      HAPPCHAN            haChan,
    IN      HCHAN               hsChan,
    IN      int                 message)
{
    return 0;
}


#ifdef __cplusplus
}
#endif
