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


#ifndef _CMCHANGETBYXXX_
#define _CMCHANGETBYXXX_
#include <cmControl.h>

/* Return the channel by LCN. The reverse direction of the bidirectional channel is not considered separate channel */
channelElem* getInChanByLCN(HCONTROL ctrl,int lcn);
channelElem* getOutChanByLCN(HCONTROL ctrl,int lcn);

/* Return the channel by LCN. The reverse direction of the bidirectional channel is considered separate channel */
channelElem* getInSubChanByLCN(HCONTROL ctrl,int lcn);
channelElem* getOutSubChanByLCN(HCONTROL ctrl,int lcn);
channelElem* getOutChanBySID(HCONTROL ctrl,int sid);
channelElem* getInChanBySID(HCONTROL ctrl,int sid);
channelElem* getNextOutChanByBase(HCONTROL ctrl,channelElem* channel, void** ptr);

channelElem* getNextOutChan(HCONTROL ctrl, void** ptr);
channelElem* getNextInChan(HCONTROL ctrl, void** ptr);

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
channelElem* getNextChan(IN HCONTROL ctrl, INOUT void** currentChannel);

BOOL checkChanSIDConsistency(HCONTROL ctrl,channelElem* channel);
int getFreeSID(HCONTROL ctrl);

#endif
#ifdef __cplusplus
}
#endif
