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

/*
  conf.h

  Ron S.
  10 Sep. 1996

  */

#ifndef _CONFH_
#define _CONFH_

#include <rvinternal.h>
#include <pvaltree.h>


typedef enum {
  confNonStandard=1,
  confNullData,
  confVideoData,
  confAudioData,
  confData
} confDataType;



int /* TRUE or RVERROR */
confLoad(
     /* Load configuration parameters */
     IN  HPVT hVal,
     IN  int confRootId,
     IN  char *prefix /* to configuration parameters */
     );

int /* terminal type or RVERROR */
confGetTerminalType(
            /* in master/slave conf */
            IN  HPVT hVal,
            IN  int confRootId
            );

int /* term cap set node id, or RVERROR */
confGetCapSet(
          /* capabilities set */
          IN  HPVT hVal,
          IN  int confRootId
          );

int /* TRUE or RVERROR */
confGetDataTypeName(
            /* Generate dataName using field name as in H.245 standard. */
            IN  HPVT hVal,
            IN  int dataTypeId, /* Data type node id */
            IN  int dataNameSize,
            OUT char *dataName, /* copied into user allocate space [128 chars] */
            OUT confDataType* type, /* of data */
            OUT INT32* typeId /* node id of media type */
            );

int
confGetDataType(
        /* Search channel name in channels conf. and create appropriate dataType tree */
        IN  HPVT hVal,
        IN  int confRootId,
        IN  char* channelName, /* in channels conf */
        OUT int dataTypeId, /* node id: user supplied */
        OUT BOOL* isDynamicPayload, /* true if dynamic */
        BOOL nonH235 /*If true means remove h235Media.mediaType level */
        );

int /* real number of channels or RVERROR */
confGetChannelNames(
            /* build array of channels names as in configuration */
            IN  HPVT hVal,
            IN  int confRootId,
            IN  int nameArSize, /* number of elements in nameArray */
            IN  int nameLength, /* length of each name in array */
            OUT char** nameArray /* user allocated array of strings */
            );


int /* TRUE if found. RVERROR if not found */
confGetModeEntry(
         /* Search mode name in configuration. */
         IN  HPVT hVal,
         IN  int confRootId,
         IN  char *modeName, /* in conf. */
         OUT INT32 *entryId /* mode entry id */
         );

#endif
#ifdef __cplusplus
}
#endif



