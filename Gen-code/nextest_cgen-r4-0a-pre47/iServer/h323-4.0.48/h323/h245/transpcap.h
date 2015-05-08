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



#ifndef _TRANSPCAP_
#define _TRANSPCAP_

#include <rvcommon.h>
#include <cm.h>

#define cmQosCapabilityNonStandard  0x1
#define cmQosCapabilityAtmParameters    0x2
#define cmQosCapabilityRsvpParameters   0x4

typedef enum
{
  cmNoneQos=0,
  cmGuaranteedQOS,
  cmControlledLoad

}cmQOSMode;

typedef struct
{
  cmQOSMode cmQosMode;
  BOOL  cmQosModeValue;
  INT32 cmTokenRate;
  INT32 cmBucketSize;
  INT32 cmPeakRate;
  INT32 cmMinPoliced;
  INT32 cmMaxPktSize;

}cmRSVPParameters;

typedef struct
{
  INT16 cmMaxNTUSize;
  BOOL  cmAtmUBR;
  BOOL  cmAtmrtVBR;
  BOOL  cmAtmnrtVBR;
  BOOL  cmAtmABR;
  BOOL  cmAtmCBR;
}cmATMParameters;



typedef enum
{
  cmIP_UDP=0,
  cmIP_TCP,
  cmAtm_AAL5_UNIDIR,
  cmAtm_AAL5_BIDIR
}cmMediaTransportType;


typedef struct
{
  cmMediaTransportType cmTransportType;
}cmMediaChannelCapability;




typedef struct
{
  cmNonStandardParam cmNonStandard;
  cmRSVPParameters cmRsvpParameters;
  cmATMParameters  cmAtmParameters;
  UINT8 cmQosCapabilityParamUsed;
}cmQosCapability;




RVAPI int RVCALLCONV
cmCreateTranspCapability( IN     HAPP            hApp,
                            cmNonStandardParam *nonStandard
                            );

RVAPI int RVCALLCONV
cmAddQosCapability( IN   HAPP            hApp,
                      IN int capTransport,
                      IN  cmQosCapability *qosCapability);



RVAPI int RVCALLCONV
cmAddMediaChannelCap( IN     HAPP            hApp,
                        IN int capTransport,
                        IN  cmMediaChannelCapability * mediaCap
                       );


RVAPI int RVCALLCONV
cmGetTransportCapabilities(             IN  HCALL       hsCall);


RVAPI int RVCALLCONV
cmGetTransportCapNonStandard(   IN   HAPP            hApp,
                              IN int transpId,
                              OUT cmNonStandardParam *nonStandard   );

RVAPI int RVCALLCONV
cmGetTransportCapQosParameters(IN    HAPP            hApp,
                              IN int transpId,
                              INOUT  cmQosCapability * cmQOSCapability,
                              INOUT int  * cmQOSCapabilitySize);

RVAPI int RVCALLCONV
cmGetTransportCapMedia(IN    HAPP            hApp,
                              IN int transpId,
                              INOUT cmMediaChannelCapability * mediaChannelCapability ,
                              OUT int *mediaChannelCapabilitySize);

#endif
#ifdef __cplusplus
}
#endif



