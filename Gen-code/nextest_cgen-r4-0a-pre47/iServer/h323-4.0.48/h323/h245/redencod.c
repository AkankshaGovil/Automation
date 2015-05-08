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



#include <stdlib.h>

#include <rvinternal.h>
#include <pvaltree.h>
#include <conf.h>
#include <netutl.h>
#include <cmchan.h>
#include <stkutils.h>
#include <h245.h>
#include <cmintr.h>
#include <cmdebprn.h>
#include <redencod.h>


int createNonStandardRedMethod( IN  HAPP            hApp,
                                IN  cmNonStandardIdentifier *identifier,
                                IN  char *data,
                                IN  int dataLength )

 {

    HPVT hVal=cmGetValTree(hApp);
    int nodeId,nonStandardId;
    int ret;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "createNonStandardRedMethod: hApp=0x%lx",hApp);


    nodeId = pvtAddRoot(hVal, ((cmElem*)hApp)->h245RedEnc, 0, NULL);
    if (nodeId<0)
    {
        cmiAPIExit(hApp,"createNonStandardRedMethod: [%d]",nodeId);
        return nodeId;
    }

    nonStandardId = pvtAdd(hVal, nodeId, __h245(nonStandard), 0, NULL, NULL);
    ret = cmNonStandardParameterCreate(hVal,nonStandardId,identifier,data,dataLength);


    cmiAPIExit(hApp, "createNonStandardRedMethod: [%d].", ret);
    if (ret<0)
      return ret;
    return nodeId;

}

int createRtpAudioRedMethod( IN  HAPP            hApp)
{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId,ret;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "createRtpAudioRedMethod: hApp=0x%lx",hApp);


    nodeId = pvtAddRoot(hVal, ((cmElem*)hApp)->h245RedEnc, 0, NULL);
    if (nodeId<0)
    {
        cmiAPIExit(hApp,"createRtpAudioRedMethod: [%d]",nodeId);
        return nodeId;
    }
    ret=  pvtAdd(hVal, nodeId, __h245(rtpAudioRedundancyEncoding), 0, NULL, NULL);
    cmiAPIExit(hApp, "createRtpAudioRedMethod: [%d].", ret);
    if (ret<0)
      return ret;
    return nodeId;
}

int createH263VideoRedMethod( IN     HAPP            hApp,
                              IN cmRTPH263VideoRedundancyEncoding * h263VRedundancyEncoding)
{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId,videoId,ii,threadId,ret;
    int cmFramesBetweenSyncPoints;
    int iNumOfThreads;
    int iFrames;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "createH263VideoRedMethod: hApp=0x%lx",hApp);


    nodeId = pvtAddRoot(hVal, ((cmElem*)hApp)->h245RedEnc, 0, NULL);
    if (nodeId<0)
    {
        cmiAPIExit(hApp,"createH263VideoRedMethod: [%d]",nodeId);
        return nodeId;
    }
    videoId = pvtAdd(hVal, nodeId, __h245(rtpH263VideoRedundancyEncoding), 0, NULL, NULL);
    iNumOfThreads = h263VRedundancyEncoding->cmNumberOfThreads;
    pvtAdd(hVal, videoId, __h245(numberOfThreads),
                  iNumOfThreads, NULL, NULL);
    cmFramesBetweenSyncPoints=h263VRedundancyEncoding->cmFramesBetweenSyncPoints;
    if (!cmFramesBetweenSyncPoints)
        cmFramesBetweenSyncPoints=256;
    iFrames = h263VRedundancyEncoding->cmFramesBetweenSyncPoints;
    ret=pvtAdd(hVal, videoId, __h245(framesBetweenSyncPoints),
        iFrames, NULL, NULL);
    switch(h263VRedundancyEncoding->cmFrameToThreadMapping)
    {
        case cmRoundrobin:
            ret=pvtAdd(hVal, videoId, __h245(frameToThreadMapping), 0, NULL, NULL);
            ret=pvtAdd(hVal, ret, __h245(roundrobin), 0, NULL, NULL);
        break;
        case cmCustom:
            ret=pvtAdd(hVal, videoId, __h245(frameToThreadMapping), 0, NULL, NULL);
            ret=pvtAdd(hVal, ret, __h245(custom), 0, NULL, NULL);
        break;
    }
    if (h263VRedundancyEncoding->cmContainedThreads.cmContainedThreadsSize)
    {
        threadId = pvtAdd(hVal, videoId, __h245(containedThreads), 0, NULL, NULL);
        for (ii=0;ii< h263VRedundancyEncoding->cmContainedThreads.cmContainedThreadsSize;ii++)
        {
            int iContainedThreads = h263VRedundancyEncoding->cmContainedThreads.cmContainedThreads[ii];
            __pvtBuildByFieldIds(ret, hVal, threadId, {_nul(0) LAST_TOKEN}, iContainedThreads,NULL);
        }

    }
    cmiAPIExit(hApp, "createH263VideoRedMethod: [%d].", ret);
    if (ret<0)
      return ret;
    return nodeId;
}


int addH263VCustomFramMaping( IN     HAPP            hApp,
                              int nodeId,
                              cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                              int rtpH263RedundancyFrameMappingSize)
{
    HPVT hVal=cmGetValTree(hApp);

    int customId,ii,frameMapId,jj,resId,frameSeqId;
    int ret=1;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "addH263VCustomFramMaping: hApp=0x%lx",hApp);
    __pvtGetNodeIdByFieldIds(customId, hVal,nodeId,
                            {_h245(rtpH263VideoRedundancyEncoding)
                             _h245(frameToThreadMapping)
                             _h245(custom)
                             LAST_TOKEN});
    if (customId < 0)
    {
        cmiAPIExit(hApp, "addH263VCustomFramMaping: [%d].", customId);
        return customId;
    }
    for (ii=0;ii<rtpH263RedundancyFrameMappingSize;ii++)
    {
        int iThreadNumber;
        __pvtBuildByFieldIds(frameMapId, hVal, customId, {_nul(0) LAST_TOKEN}, 0 ,NULL);
        iThreadNumber = rtpH263RedundancyFrameMapping[ii].cmThreadNumber;
        __pvtBuildByFieldIds(resId, hVal, frameMapId, {_h245(threadNumber) LAST_TOKEN}, iThreadNumber, NULL);
        __pvtBuildByFieldIds(frameSeqId, hVal, frameMapId, {_h245(frameSequence) LAST_TOKEN}, 0,NULL);
        ret=frameSeqId;
        for (jj = 0;jj<rtpH263RedundancyFrameMapping[ii].cmFrameSequenceSize;jj++)
        {
            int iFrameSeq = rtpH263RedundancyFrameMapping[ii].cmFrameSequence[jj];
            __pvtBuildByFieldIds(ret, hVal, frameSeqId, {_nul(0) LAST_TOKEN}, iFrameSeq,NULL);
        }
    }

    cmiAPIExit(hApp, "addH263VCustomFramMaping: [%d].", ret);
    if(ret<0)
        return ret;
    return nodeId;

}

RVAPI int RVCALLCONV
cmGetRedundancyEncodingMethod (IN    HAPP            hApp,
                               IN int redEncMethodId,
                               OUT cmRedundancyEncodingMethod * encodingMethod)

{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId;

    cmiAPIEnter(hApp, "cmGetRedundancyEncodingMethod: redEncMethodId = %d ",redEncMethodId);

    nodeId = pvtGetChild(hVal,redEncMethodId,__h245(nonStandard),NULL);
    if (nodeId>=0)
      * encodingMethod = cmRedEncNonStandard;
    nodeId = pvtGetChild(hVal,redEncMethodId,__h245(rtpAudioRedundancyEncoding),NULL);
    if (nodeId>=0)
      * encodingMethod = cmRedEncRtpAudio;
    nodeId = pvtGetChild(hVal,redEncMethodId,__h245(rtpH263VideoRedundancyEncoding),NULL);
    if (nodeId>=0)
      * encodingMethod = cmRedEncH263Video;
    cmiAPIExit(hApp, "cmGetRedundancyEncodingMethod: encodingMethod [%d] nodeId [%d].", *encodingMethod,nodeId);
    return nodeId;
}

RVAPI int RVCALLCONV
cmGetH263RedundancyEncoding (IN  HAPP            hApp,
                             IN int h263EncMethodId,
                             OUT cmRTPH263VideoRedundancyEncoding * rtpH263RedundancyEncoding)
{
    HPVT hVal=cmGetValTree(hApp);
    int nodeId,ii,containedThreadsId;
    INT32 castValue;

    cmiAPIEnter(hApp, "cmGetH263RedundancyEncoding: h263EncMethodId = %d ",h263EncMethodId);
    pvtGetChildByFieldId(hVal,h263EncMethodId,__h245(numberOfThreads),
        &castValue ,NULL);
    rtpH263RedundancyEncoding->cmNumberOfThreads=(UINT8)castValue;
    pvtGetChildByFieldId(hVal,h263EncMethodId,__h245(framesBetweenSyncPoints),
        &castValue ,NULL);
    rtpH263RedundancyEncoding->cmFramesBetweenSyncPoints=(UINT8)castValue;

    nodeId = pvtGetChild(hVal,h263EncMethodId,__h245(containedThreads),NULL);
    if (nodeId >=0)
    {
      rtpH263RedundancyEncoding->cmContainedThreads.cmContainedThreadsSize = (BYTE)pvtNumChilds(hVal,nodeId);
      for (ii=1;ii<= rtpH263RedundancyEncoding->cmContainedThreads.cmContainedThreadsSize;ii++)
      {
        pvtGetByIndex(hVal,nodeId,ii,&containedThreadsId);
        pvtGet(hVal,containedThreadsId,NULL,NULL,
          &castValue ,NULL);
      rtpH263RedundancyEncoding->cmContainedThreads.cmContainedThreads[ii-1]=(UINT8)castValue;
      }

    }
    __pvtGetNodeIdByFieldIds(nodeId,hVal,h263EncMethodId,
                            {_h245(frameToThreadMapping) _h245(custom) LAST_TOKEN});
    if (nodeId >=0)
      rtpH263RedundancyEncoding->cmFrameToThreadMapping=cmCustom;
    else
    {
      nodeId = 0;
      rtpH263RedundancyEncoding->cmFrameToThreadMapping=cmRoundrobin;
    }

    cmiAPIExit(hApp, "cmGetH263RedundancyEncoding: [%d]",nodeId);
    return nodeId; /* return customId. if it is roundrobin return -1 */

}

/*
  rtp263RedundancyFrameMapping is array,allocated by user. Max size is 256 according to ASN definition
  in rtp263RedundancyFrameMappingSize as IN param the size of allocated array is passed.
  in rtp263RedundancyFrameMappingSize as OUT param  the real array size,according to the h245 message,
  is returned
*/
RVAPI int RVCALLCONV
cmGetCustomFrameToThreadMapping (IN  HAPP            hApp,
                             IN int h263EncMethodId,
                             INOUT cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                             INOUT int * rtpH263RedundancyFrameMappingSize )
{
    HPVT hVal=cmGetValTree(hApp);
    int ii,customId,childs,frmMapId,frmSeqId,jj,frmSeqElemId;
    INT32 castValue;

    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmGetCustomFrameToThreadMapping: h263EncMethodId = %d ",h263EncMethodId);
    __pvtGetNodeIdByFieldIds(customId,hVal,h263EncMethodId,
                            {_h245(frameToThreadMapping) _h245(custom) LAST_TOKEN});
    if (customId>=0)
    {
      childs = pvtNumChilds(hVal,customId);
      if ( childs < *rtpH263RedundancyFrameMappingSize)
        *rtpH263RedundancyFrameMappingSize = childs;
      for (ii=1;ii<= *rtpH263RedundancyFrameMappingSize;ii++)
      {
        pvtGetByIndex(hVal,customId,ii,&frmMapId);
        pvtGetChildByFieldId(hVal,frmMapId,__h245(threadNumber),&castValue,NULL);
        rtpH263RedundancyFrameMapping[ii-1].cmThreadNumber=(UINT8)castValue;
        frmSeqId = pvtGetChild(hVal,frmMapId,__h245(frameSequence),NULL);
        rtpH263RedundancyFrameMapping[ii-1].cmFrameSequenceSize=(BYTE)pvtNumChilds(hVal,frmSeqId);
        for (jj=1;jj<=rtpH263RedundancyFrameMapping[ii-1].cmFrameSequenceSize;jj++)
        {
          pvtGetByIndex(hVal,frmSeqId,jj,&frmSeqElemId);
          pvtGet(hVal,frmSeqElemId,NULL,NULL,
          &castValue ,NULL);
          rtpH263RedundancyFrameMapping[ii-1].cmFrameSequence[jj-1]=(UINT8)castValue;

        }


      }

    }
    else
      *rtpH263RedundancyFrameMappingSize=0;
    cmiAPIExit(hApp, "cmGetCustomFrameToThreadMapping: [%d]",*rtpH263RedundancyFrameMappingSize);
    return *rtpH263RedundancyFrameMappingSize;
}


RVAPI int RVCALLCONV
cmCreateNonStandardRedMethod( IN     HAPP            hApp,
                             IN cmNonStandardParam *nonStandard)
{
    cmElem* app=(cmElem*)hApp;
    int ret;
    if (!app)
    return RVERROR;
    cmiAPIEnter(hApp, "cmCreateNonStandardRedMethod: hApp=0x%lx",hApp);

    if (nonStandard!=NULL)
        ret = createNonStandardRedMethod(hApp,&nonStandard->info,nonStandard->data,nonStandard->length);
    else
        ret = createNonStandardRedMethod(hApp,NULL,NULL,0);
    cmiAPIExit(hApp, "cmCreateNonStandardRedMethod: [%d]",ret);
    return ret;
}

RVAPI int RVCALLCONV
cmCreateRtpAudioRedMethod( IN    HAPP            hApp)
{
    int result;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCreateRtpAudioRedMethod: hApp=0x%lx",hApp);
    result = createRtpAudioRedMethod( hApp);
    cmiAPIExit(hApp, "cmCreateRtpAudioRedMethod: [$d]",result);

    return result;
}

RVAPI int RVCALLCONV
cmCreateH263VideoRedMethod( IN   HAPP            hApp,
                              IN cmRTPH263VideoRedundancyEncoding * h263VRedundancyEncoding)
{
    int result;
    if (!hApp) return RVERROR;

    cmiAPIEnter(hApp, "cmCreateH263VideoRedMethod: hApp=0x%lx",hApp);
    result =  createH263VideoRedMethod( hApp,h263VRedundancyEncoding);
    cmiAPIExit(hApp, "cmCreateH263VideoRedMethod: [%d]",result);

    return result;
}



RVAPI int RVCALLCONV
cmAddH263VCustomFrameMapping( IN     HAPP            hApp,
                              int nodeId,
                              cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                              int rtpH263RedundancyFrameMappingSize)

{
    int result;
    if (!hApp) return RVERROR;
    cmiAPIEnter(hApp, "cmAddH263VCustomFrameMapping: hApp=0x%lx",hApp);
    result = addH263VCustomFramMaping( hApp,nodeId,rtpH263RedundancyFrameMapping,
                                     rtpH263RedundancyFrameMappingSize);
    cmiAPIExit(hApp, "cmAddH263VCustomFrameMapping: [%d]",result);

    return result;
}

#ifdef __cplusplus
}
#endif



