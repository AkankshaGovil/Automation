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

#include <rvinternal.h>
#include <cmControl.h>
#include <cmCrossReference.h>


channelElem* getInChanByLCN(HCONTROL ctrl,int lcn)
{
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    int i;
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (f_channel[i] && !f_channel[i]->origin && f_channel[i]->myLCN==lcn)
            return f_channel[i];
    }
    return NULL;
}

channelElem* getOutChanByLCN(HCONTROL ctrl,int lcn)
{
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    int i;
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (f_channel[i] && f_channel[i]->origin && f_channel[i]->myLCN==lcn)
            return f_channel[i];
    }
    return NULL;
}

channelElem* getOutSubChanByLCN(HCONTROL ctrl,int lcn)
{
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    int i;
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if ((f_channel[i] && f_channel[i]->origin && f_channel[i]->myLCN==lcn) ||
            (f_channel[i] && !f_channel[i]->origin && f_channel[i]->rvrsLCN==lcn))
            return f_channel[i];
    }
    return NULL;
}

channelElem* getInSubChanByLCN(HCONTROL ctrl,int lcn)
{
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    int i;
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if ((f_channel[i] && !f_channel[i]->origin && f_channel[i]->myLCN==lcn) ||
            (f_channel[i] && f_channel[i]->origin && f_channel[i]->rvrsLCN==lcn))
            return f_channel[i];
    }
    return NULL;
}

channelElem* getOutChanBySID(HCONTROL ctrl,int sid)
{
    int i;
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (f_channel[i] && f_channel[i]->origin && f_channel[i]->sid==sid)
            return f_channel[i];
    }
    return NULL;
}

channelElem* getInChanBySID(HCONTROL ctrl,int sid)
{
    int i;
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (f_channel[i] && !f_channel[i]->origin && f_channel[i]->sid==sid)
            return f_channel[i];
    }
    return NULL;
}

/* Tests that there is no channel in the same direction with the same non-zero SID */
BOOL checkChanSIDConsistency(HCONTROL ctrl,channelElem* channel)
{
    int i;
    channelElem** f_channel=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);

    if (channel->sid==0)
        return FALSE;

    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (f_channel[i] && f_channel[i]!=channel && f_channel[i]->origin==channel->origin && f_channel[i]->sid==channel->sid && (f_channel[i]->state != released))
            return TRUE;
    }
    return FALSE;
}

channelElem* getNextOutChanByBase(HCONTROL ctrl,channelElem* channel,void** currentChannel)
{
    int currentChannelNumber=0;
    channelElem** channelsCollection=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);

    if (*currentChannel)
        currentChannelNumber=(channelElem**)*currentChannel-channelsCollection
                    /* because this one is already not of our interest */+1;

    for (;currentChannelNumber<cmiGetNumberOfChannelsForCtrl(ctrl);currentChannelNumber++)
    {
        if (channelsCollection[currentChannelNumber] && channelsCollection[currentChannelNumber]->origin &&
            channelsCollection[currentChannelNumber]->base==channel)
        {
            *currentChannel=(void*)&(channelsCollection[currentChannelNumber]);
            return channelsCollection[currentChannelNumber];
        }
    }
    return NULL;
}

channelElem* getNextOutChan(HCONTROL ctrl, void** currentChannel)
{
    int currentChannelNumber=0;
    channelElem** channelsCollection=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);

    if (*currentChannel)
        currentChannelNumber=(channelElem**)*currentChannel-channelsCollection
                    /* because this one is already not of our interest */+1;

    for (;currentChannelNumber<cmiGetNumberOfChannelsForCtrl(ctrl);currentChannelNumber++)
    {
        if (channelsCollection[currentChannelNumber] && channelsCollection[currentChannelNumber]->origin)
        {
            *currentChannel=(void*)&(channelsCollection[currentChannelNumber]);
            return channelsCollection[currentChannelNumber];
        }
    }
    return NULL;
}

channelElem* getNextInChan(HCONTROL ctrl, void** currentChannel)
{
    int currentChannelNumber=0;
    channelElem** channelsCollection=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);

    if (*currentChannel)
        currentChannelNumber=(channelElem**)*currentChannel-channelsCollection
                    /* because this one is already not of our interest */+1;

    for (;currentChannelNumber<cmiGetNumberOfChannelsForCtrl(ctrl);currentChannelNumber++)
    {
        if (channelsCollection[currentChannelNumber] && !channelsCollection[currentChannelNumber]->origin)
        {
            *currentChannel=(void*)&(channelsCollection[currentChannelNumber]);
            return channelsCollection[currentChannelNumber];
        }
    }
    return NULL;
}

/************************************************************************
 * getNextChan
 * purpose: Get the next channel for a given control object.
 *          This function can be used to perform a single task on all
 *          the channels.
 * input  : ctrl            - Control object
 *          currentChannel  - Current channel we have.
 *                            If the contents of this pointer is NULL, then the
 *                            first channel will be returned
 * output : currentChannel  - Next channel in list
 * return : Next channel in list on success
 *          NULL when there are no more channels
 ************************************************************************/
channelElem* getNextChan(IN HCONTROL ctrl, INOUT void** currentChannel)
{
    int currentChannelNumber=0;
    channelElem** channelsCollection=(channelElem**)cmiGetChannelsCollectionForCtrl(ctrl);

    /* Calculate the index value of the current given channel if not NULL */
    if (*currentChannel)
        currentChannelNumber=(channelElem**)*currentChannel-channelsCollection
                    /* because this one is already not of our interest */+1;

    /* Look for a channel whose pointer is not NULL */
    for (;currentChannelNumber<cmiGetNumberOfChannelsForCtrl(ctrl);currentChannelNumber++)
    {
        if (channelsCollection[currentChannelNumber])
        {
            *currentChannel=(void*)&(channelsCollection[currentChannelNumber]);
            return channelsCollection[currentChannelNumber];
        }
    }

    /* We're done here */
    return NULL;
}


int getFreeSID(HCONTROL ctrl)
{
    controlElem* ctrlE = (controlElem*)ctrl;
    ctrlE->nextFreeSID++;
    if (ctrlE->nextFreeSID>=0x100 || ctrlE->nextFreeSID<=0x20)
        ctrlE->nextFreeSID=0x20;
    return ctrlE->nextFreeSID;
}
#ifdef __cplusplus
}
#endif
