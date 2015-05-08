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

#ifndef _REDENCOD_H
#define _REDENCOD_H

#include <rvcommon.h>
#include <cm.h>


typedef enum
{
  cmRedEncNonStandard=0,
  cmRedEncRtpAudio,
  cmRedEncH263Video
}cmRedundancyEncodingMethod;


typedef enum
{
  cmRoundrobin=0 ,
  cmCustom
}cmFrameToThreadMappingEnum;

typedef struct
{
  UINT8 cmContainedThreads[256];
  UINT8 cmContainedThreadsSize;
}cmContainedThreadsStruct;

typedef struct
{
  UINT8  cmNumberOfThreads;
  UINT16 cmFramesBetweenSyncPoints;
  cmContainedThreadsStruct cmContainedThreads;
  cmFrameToThreadMappingEnum cmFrameToThreadMapping;
}cmRTPH263VideoRedundancyEncoding;

typedef struct
{
  UINT8 cmThreadNumber;
  UINT8 cmFrameSequence[256];
  UINT8 cmFrameSequenceSize;
}cmRTPH263RedundancyFrameMapping;


RVAPI int RVCALLCONV
cmCreateNonStandardRedMethod(   IN   HAPP            hApp,
                                IN  cmNonStandardParam *nonStandard);

RVAPI int RVCALLCONV
cmAddH263VCustomFrameMapping( IN     HAPP            hApp,
                              int nodeId,
                              cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                              int rtpH263RedundancyFrameMappingSize);


RVAPI int RVCALLCONV
cmCreateRtpAudioRedMethod( IN    HAPP            hApp);

RVAPI int RVCALLCONV
cmCreateH263VideoRedMethod( IN   HAPP            hApp,
                            IN cmRTPH263VideoRedundancyEncoding * h263VRedundancyEncoding);

RVAPI int RVCALLCONV
cmGetRedundancyEncodingMethod (IN    HAPP            hApp,
                               IN int redEncMethodId,
                               OUT cmRedundancyEncodingMethod * encodingMethod);
RVAPI int RVCALLCONV
cmGetH263RedundancyEncoding (IN  HAPP            hApp,
                             IN int h263EncMethodId,
                             OUT cmRTPH263VideoRedundancyEncoding * rtpH263RedundancyEncoding);
RVAPI int RVCALLCONV
cmGetCustomFrameToThreadMapping (IN  HAPP            hApp,
                             IN int h263EncMethodId,
                             INOUT cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                             INOUT int * rtpH263RedundancyFrameMappingSize );

RVAPI int RVCALLCONV
cmAddH263VCustomFrameMapping( IN     HAPP            hApp,
                              int nodeId,
                              cmRTPH263RedundancyFrameMapping * rtpH263RedundancyFrameMapping,
                              int rtpH263RedundancyFrameMappingSize);

#endif
#ifdef __cplusplus
}
#endif



