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
#include <cmictrl.h>

BOOL addChannelForCtrl(HCONTROL ctrl,HCHAN ch)
{
    int i;
    HCHAN* chan=cmiGetChannelsCollectionForCtrl(ctrl);
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (!chan[i])
        {
            chan[i]=ch;
            return  TRUE;
        }
    }
    return FALSE;
}

BOOL deleteChannelForCtrl(HCONTROL ctrl,HCHAN ch)
{
    int i;
    HCHAN* chan=cmiGetChannelsCollectionForCtrl(ctrl);
    for (i=0;i<cmiGetNumberOfChannelsForCtrl(ctrl);i++)
    {
        if (chan[i]==ch)
        {
            chan[i]=NULL;
            return  TRUE;
        }
    }
    return FALSE;
}

#ifdef __cplusplus
}
#endif
