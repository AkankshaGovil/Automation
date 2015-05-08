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

#include <rvcommon.h>
#include "bitfield.h"
#include <li.h>

#include "rtp.h"
#include "payload.h"

RVAPI
INT32 RVCALLCONV rtpPCMUPack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);
    p->payload=PCMU;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpPCMUUnpack(
                                OUT void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
  if(buf || len || p || param);

    return 0;
}

RVAPI
INT32 RVCALLCONV rtpPCMUGetHeaderLength(void)
{
    return rtpGetHeaderLength();
}

RVAPI
INT32 RVCALLCONV rtpPCMAPack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);

    p->payload=PCMA;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpPCMAUnpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT     rtpParam*p,
                                OUT void*param)
{
  if(buf || len || p || param);
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpPCMAGetHeaderLength(void)
{
    return rtpGetHeaderLength();
}

RVAPI
INT32 RVCALLCONV rtpG722Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);

    p->payload=G722;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG722Unpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
  if(buf || len || p || param);

    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG722GetHeaderLength(void)
{
    return rtpGetHeaderLength();
}

RVAPI
INT32 RVCALLCONV rtpG728Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);

    p->payload=G728;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG728Unpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
  if(buf || len || p || param);

    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG728GetHeaderLength(void)
{
    return rtpGetHeaderLength();
}

RVAPI
INT32 RVCALLCONV rtpG729Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);

    p->payload=G729;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG729Unpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
  if(buf || len || p || param);

    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG729GetHeaderLength(void)
{
    return rtpGetHeaderLength();
}

RVAPI
INT32 RVCALLCONV rtpG7231Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
  if(buf || len || param);

    p->payload=G7231;
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG7231Unpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
 if(buf || len || param || p);
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpG7231GetHeaderLength(void)
{
    return rtpGetHeaderLength();
}


RVAPI
INT32 RVCALLCONV rtpH261Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
    H261param*h261=(H261param*)param;
    UINT32*hPtr;
    p->sByte-=4;
    hPtr=(UINT32*)((BYTE*)buf+p->sByte);
    hPtr[0]=bitfieldSet(0,h261->v,24,1);
    hPtr[0]=bitfieldSet(hPtr[0],h261->i,25,1);
    if(len);

    if (h261->gobN)
    {
        hPtr[0]=bitfieldSet(hPtr[0],h261->vMvd,0,5);
        hPtr[0]=bitfieldSet(hPtr[0],h261->hMvd,5,5);
        hPtr[0]=bitfieldSet(hPtr[0],h261->quant,10,5);
        hPtr[0]=bitfieldSet(hPtr[0],h261->mbaP,15,5);
        hPtr[0]=bitfieldSet(hPtr[0],h261->gobN,20,4);
    }
    hPtr[0]=bitfieldSet(hPtr[0],h261->eBit,26,3);
    hPtr[0]=bitfieldSet(hPtr[0],h261->sBit,29,3);
    p->payload=H261;
    liConvertHeader2l((BYTE*)hPtr,0,1);
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpH261Unpack(
                                OUT void*buf,
                                IN  INT32 len,
                                OUT rtpParam*p,
                                OUT void*param)
{
    H261param*h261=(H261param*)param;
    UINT32*hPtr=(UINT32*)((BYTE*)buf+p->sByte);
    if(len);

    p->sByte+=4;
    liConvertHeader2h((BYTE*)hPtr,0,1);
    h261->vMvd=bitfieldGet(hPtr[0],0,5);
    h261->hMvd=bitfieldGet(hPtr[0],5,5);
    h261->quant=bitfieldGet(hPtr[0],10,5);
    h261->mbaP=bitfieldGet(hPtr[0],15,5);
    h261->gobN=bitfieldGet(hPtr[0],20,4);
    h261->v=bitfieldGet(hPtr[0],24,1);
    h261->i=bitfieldGet(hPtr[0],25,1);
    h261->eBit=bitfieldGet(hPtr[0],26,3);
    h261->sBit=bitfieldGet(hPtr[0],29,3);
    return 0;
}

RVAPI
INT32 RVCALLCONV rtpH261GetHeaderLength()
{
    return rtpGetHeaderLength()+4;
}


INT32 RVCALLCONV rtpH263Pack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
    H263param*h263=(H263param*)param;
    UINT32*hPtr=NULL;
    int dwords=0;
    if(len);

    if (h263->p)
    {
        dwords=1;
        p->sByte-=4;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);

        hPtr[0]=bitfieldSet(0,h263->dbq,11,2);
        hPtr[0]=bitfieldSet(hPtr[0],h263->trb,8,3);
        hPtr[0]=bitfieldSet(hPtr[0],h263->tr,0,8);
    }

    if (h263->f)
    {
        dwords+=2;
        p->sByte-=8;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);
        hPtr[0]=bitfieldSet(0,h263->mbaP,0,8);
        hPtr[0]=bitfieldSet(hPtr[0],h263->gobN,8,5);
        hPtr[0]=bitfieldSet(hPtr[0],h263->quant,16,5);


        hPtr[1]=bitfieldSet(      0,h263->vMv2, 0,8);
        hPtr[1]=bitfieldSet(hPtr[1],h263->hMv2, 8,8);
        hPtr[1]=bitfieldSet(hPtr[1],h263->vMv1,16,8);
        hPtr[1]=bitfieldSet(hPtr[1],h263->hMv1,24,8);
    }

    if (!h263->f && !h263->p)
    {
        dwords=1;
        p->sByte-=4;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);
        hPtr[0]=0;
    }

    hPtr[0]=bitfieldSet(hPtr[0],h263->f,31,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263->p,30,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263->sBit,27,3);
    hPtr[0]=bitfieldSet(hPtr[0],h263->eBit,24,3);
    hPtr[0]=bitfieldSet(hPtr[0],h263->src,21,3);
    hPtr[0]=bitfieldSet(hPtr[0],h263->i,15,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263->a,14,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263->s,13,1);
    p->payload=H263;
    liConvertHeader2l((BYTE*)hPtr,0,dwords);
    return 0;
}

INT32 RVCALLCONV rtpH263Unpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT     rtpParam*p,
                                OUT void*param)
{
    H263param*h263=(H263param*)param;
    UINT32*hPtr=(UINT32*)((BYTE*)buf+p->sByte);
    if(len);

    p->sByte+=4;

    liConvertHeader2h((BYTE*)hPtr,0,1);
    h263->f=bitfieldGet(hPtr[0],31,1);
    h263->p=bitfieldGet(hPtr[0],30,1);

    h263->sBit=bitfieldGet(hPtr[0],27,3);
    h263->eBit=bitfieldGet(hPtr[0],24,3);
    h263->src=bitfieldGet(hPtr[0],21,3);
    h263->i=bitfieldGet(hPtr[0],15,1);
    h263->a=bitfieldGet(hPtr[0],14,1);
    h263->s=bitfieldGet(hPtr[0],13,1);

    if (h263->f)
    {
        int aDwords=h263->p+1;
        liConvertHeader2h((BYTE*)hPtr,1,aDwords);
        p->sByte+=4*aDwords;
        h263->mbaP=bitfieldGet(hPtr[0],0,8);
        h263->gobN=bitfieldGet(hPtr[0],8,5);
        h263->quant=bitfieldGet(hPtr[0],16,5);


        h263->vMv2=bitfieldGet(hPtr[1],0,8);
        h263->hMv2=bitfieldGet(hPtr[1],8,8);
        h263->vMv1=bitfieldGet(hPtr[1],16,8);
        h263->hMv1=bitfieldGet(hPtr[1],24,8);
        hPtr=(UINT32*)((BYTE*)buf+p->sByte-4);
    }

    if (h263->p)
    {

        h263->dbq=bitfieldGet(hPtr[0],11,2);
        h263->trb=bitfieldGet(hPtr[0],8,3);
        h263->tr=bitfieldGet(hPtr[0],0,8);
    }


    return 0;
}

INT32 RVCALLCONV rtpH263GetHeaderLength()
{
    return rtpGetHeaderLength()+12;
}

INT32 RVCALLCONV rtpH263aPack(
                              IN    void*buf,
                              IN    INT32 len,
                              IN    rtpParam*p,
                              IN    void*param)
{
    H263aparam*h263a=(H263aparam*)param;
    UINT32*hPtr=NULL;
    int dwords=0;
    if(len);

    if (h263a->p)
    {
        dwords=1;
        p->sByte-=4;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);

        hPtr[0]=bitfieldSet(0,h263a->dbq,11,2);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->trb,8,3);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->tr,0,8);
    }

    if (h263a->f)
    {
        dwords+=2;
        p->sByte-=8;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);
        hPtr[0]=bitfieldSet(0,h263a->mbaP,2,9);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->gobN,11,5);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->quant,16,5);


        hPtr[1]=bitfieldSet(      0,h263a->vMv2, 0,7);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->hMv2, 7,7);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->vMv1,14,7);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->hMv1,21,7);

    hPtr[1]=bitfieldSet(hPtr[1],h263a->a,28,1);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->s,29,1);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->u,30,1);
        hPtr[1]=bitfieldSet(hPtr[1],h263a->i,31,1);
    }

    if (!h263a->f && !h263a->p)
    {
        dwords=1;
        p->sByte-=4;
        hPtr=(UINT32*)((BYTE*)buf+p->sByte);
        hPtr[0]=bitfieldSet(      0,h263a->a,17,1);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->s,18,1);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->u,19,1);
        hPtr[0]=bitfieldSet(hPtr[0],h263a->i,20,1); ;
    }

    hPtr[0]=bitfieldSet(hPtr[0],h263a->f,31,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263a->p,30,1);
    hPtr[0]=bitfieldSet(hPtr[0],h263a->sBit,27,3);
    hPtr[0]=bitfieldSet(hPtr[0],h263a->eBit,24,3);
    hPtr[0]=bitfieldSet(hPtr[0],h263a->src,21,3);

    p->payload=H263;
    liConvertHeader2l((BYTE*)hPtr,0,dwords);
    return 0;
}

INT32 RVCALLCONV rtpH263aUnpack(
                                OUT     void*buf,
                                IN  INT32 len,
                                OUT     rtpParam*p,
                                OUT void*param)
{
    H263aparam*h263a=(H263aparam*)param;
    UINT32*hPtr=(UINT32*)((BYTE*)buf+p->sByte);
    if(len);

    p->sByte+=4;

    liConvertHeader2h((BYTE*)hPtr,0,1);
    h263a->f=bitfieldGet(hPtr[0],31,1);
    h263a->p=bitfieldGet(hPtr[0],30,1);

    h263a->sBit=bitfieldGet(hPtr[0],27,3);
    h263a->eBit=bitfieldGet(hPtr[0],24,3);
    h263a->src=bitfieldGet(hPtr[0],21,3);


    if (h263a->f)
    {
        int aDwords=h263a->p+1;
        liConvertHeader2h((BYTE*)hPtr,1,aDwords);
        p->sByte+=4*aDwords;
        h263a->mbaP=bitfieldGet(hPtr[0],2,9);
        h263a->gobN=bitfieldGet(hPtr[0],11,5);
        h263a->quant=bitfieldGet(hPtr[0],16,5);


        h263a->vMv2=bitfieldGet(hPtr[1],0,7);
        h263a->hMv2=bitfieldGet(hPtr[1],7,7);
        h263a->vMv1=bitfieldGet(hPtr[1],14,7);
        h263a->hMv1=bitfieldGet(hPtr[1],21,7);

    h263a->i=bitfieldGet(hPtr[1],31,1);
    h263a->u=bitfieldGet(hPtr[1],30,1);
    h263a->s=bitfieldGet(hPtr[1],29,1);
    h263a->a=bitfieldGet(hPtr[1],28,1);

        hPtr=(UINT32*)((BYTE*)buf+p->sByte-4);
    }

    if (h263a->p)
    {

        h263a->dbq=bitfieldGet(hPtr[0],11,2);
        h263a->trb=bitfieldGet(hPtr[0],8,3);
        h263a->tr=bitfieldGet(hPtr[0],0,8);
    }

    if (!h263a->f && !h263a->p)
    {
      h263a->i=bitfieldGet(hPtr[0],20,1);
      h263a->u=bitfieldGet(hPtr[0],19,1);
      h263a->s=bitfieldGet(hPtr[0],18,1);
      h263a->a=bitfieldGet(hPtr[0],17,1);

    }


    return 0;
}

INT32 RVCALLCONV rtpH263aGetHeaderLength()
{
    return rtpGetHeaderLength()+12;
}






#ifdef __cplusplus
}
#endif



