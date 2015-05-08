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

#include <log.h>
#include <prnutils.h>

#ifndef NOLOGSUPPORT
void printHexBuff(BYTE* buf, int len, RVHLOG msa)
{
    char tempBuff[128];
    int tempLen;
    int i ,j;

    for (i=0;i<len;i++)
    {
        tempLen=sprintf(tempBuff,"%5.5d   ",i);
        for (j=i;j<i+16;j++)
            tempLen+=sprintf(tempBuff+tempLen,(j<len)?"%2.2x ":"   ", (j<len)?buf[j]:0);
        tempLen+=sprintf(tempBuff+tempLen,"   |");
        for (j=i;j<min(i+16,len);j++)
            tempLen+=sprintf(tempBuff+tempLen,"%c", (buf[j]>=' ')?buf[j]:'.');
        tempLen+=sprintf(tempBuff+tempLen,"|");
        logPrint(msa,RV_DEBUG,(msa,RV_DEBUG,"%s",tempBuff));
        i=j-1;
    }
}
#endif
#ifdef __cplusplus
}
#endif



