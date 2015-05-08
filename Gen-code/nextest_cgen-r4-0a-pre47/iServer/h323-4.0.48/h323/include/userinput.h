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



#ifndef _USER_INPUT_H
#define _USER_INPUT_H

#include <rvcommon.h>

typedef enum
{
  cmUserInputNonStandard=0,
  cmUserInputAlphanumeric,
  cmUserInputSupport,
  cmUserInputSignal,
  cmUserInputSignalUpdate
}cmUserInputIndication;


typedef enum
{
  cmSupportNonStandard=0,
  cmSupportBasicString,
  cmSupportIA5String,
  cmSupportGeneralString

}cmUserInputSupportIndication;

typedef struct
{
  int timestamp;      /* -1 if optional; */
  int expirationTime; /* -1 if optional;*/
  int logicalChannelNumber;
}cmUserInputSignalRtpStruct;

typedef struct
{
  char signalType;
  int duration;
  cmUserInputSignalRtpStruct cmUserInputSignalRtp;/* if logicalchannel == 0 -optional*/
}cmUserInputSignalStruct;

typedef struct
{
  int duration;
  cmUserInputSignalRtpStruct cmUserInputSignalRtp;
  /* if logicalchannel == 0 -optional only logicalchannel element is used */
}cmUserInputSignalUpdateStruct;


RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSupportIndicationBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 IN cmUserInputSupportIndication userInputSupport,
           OUT int * nodeId  /* nodeId of nonstandart UserInputSupportIndication */
                 );
RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSignalBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 cmUserInputSignalStruct *userInputSignalStruct
                 );

RVAPI int RVCALLCONV  /* userInput message node id or RVERROR */
cmUserInputSignalUpdateBuild(
                 /* Build userUser message with alphanumeric data */
                 IN  HAPP hApp,
                 cmUserInputSignalStruct *userInputSignalStruct
                 );


RVAPI int RVCALLCONV  /* TRUE or RVERROR */
cmUserInputGetDetail(
           IN  HAPP hApp,
           IN  INT32 userInputId,
         OUT cmUserInputIndication * userInputIndication
           );
RVAPI int RVCALLCONV
cmUserInputGetSignal(
           IN  HAPP hApp,
           IN  INT32 signalUserInputId,
         OUT cmUserInputSignalStruct * userInputSignalStruct
           );
RVAPI int RVCALLCONV
cmUserInputGetSignalUpdate(
           IN  HAPP hApp,
           IN  INT32 signalUserInputId,
         OUT cmUserInputSignalStruct * userInputSignalStruct
           );
RVAPI int RVCALLCONV
cmUserInputSupportGet(
           IN  HAPP hApp,
           IN  INT32 supportUserInputId,
         OUT cmUserInputSupportIndication * userInputSupportIndication
           );




#endif
#ifdef __cplusplus
}
#endif



