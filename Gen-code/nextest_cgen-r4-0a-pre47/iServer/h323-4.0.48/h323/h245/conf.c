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
  conf.c

  Ron S.
  15 Sep. 1996

  */

#include <stdlib.h>

#include <ms.h>
#include <intutils.h>
#include <psyntree.h>
#include <conf.h>
#include <h245.h>


typedef struct {
  HPVT hVal; /* conf value tree */
  int rootId; /* configuration root id */
} ConfContext;


int /* terminal type or RVERROR */
confGetTerminalType(
            /* in master/slave conf */
            IN  HPVT hVal,
            IN  int confRootId
            )
{
  INT32 termType=RVERROR;
  pvtGetChildValue2(hVal, confRootId, __h245(masterSlave), __h245(terminalType), &termType, NULL);
  return (int)termType;
}


int /* term cap set node id, or RVERROR */
confGetCapSet(
          /* capabilities set */
          IN  HPVT hVal,
          IN  int confRootId
          )
{
  return pvtGetChild2(hVal, confRootId, __h245(capabilities), __h245(terminalCapabilitySet));
}


int /* TRUE or RVERROR */
confGetDataTypeName(
            /* Generate dataName using field name as in H.245 standard. */
            IN  HPVT hVal,
            IN  int dataTypeId, /* Data type node id */
            IN  int dataNameSize,
            OUT char *dataName, /* copied into user allocate space [128 chars] */
            OUT confDataType* type, /* of data */
            OUT INT32* typeId /* node id of media type */
            )
{
  HPST synConf = pvtGetSynTree(hVal, dataTypeId);
  int dataId = pvtChild(hVal, dataTypeId);
  int choice=-1;
  INTPTR fieldId=-1;

  if( (synConf == NULL) || (dataId < 0) )
      return RVERROR;

  pvtGet(hVal, dataId, &fieldId, NULL, NULL, NULL);
  switch(fieldId)
  {
  case __h245(nonStandard):
      choice = confNonStandard;
      break;
  case __h245(nullData):
      choice = confNullData;
      break;
  case __h245(videoData):
      choice = confVideoData;
      dataId = pvtChild(hVal, dataId);
      break;
  case __h245(audioData):
      choice = confAudioData;
      dataId = pvtChild(hVal, dataId);
      break;
  case __h245(data):
      choice = confData;
      dataId = pvtChild(hVal, pvtChild(hVal, dataId));
      break;
  default:
      strncpy(dataName, "encryptionData - not supported", (size_t)dataNameSize);
      break;
  }
  
  if (dataName)
  {
      pvtGet(hVal, dataId, &fieldId, NULL, NULL, NULL);
      pstGetFieldName(synConf, fieldId, dataNameSize, dataName);
  }
  
  if (type) *type=(confDataType)choice;
  if (typeId) *typeId = dataId;
  
  return TRUE;
}



int
confGetDataType(
        /* Search channel name in channels conf. and create appropriate dataType tree */
        IN  HPVT hVal,
        IN  int confRootId,
        IN  char* channelName, /* in channels conf */
        OUT int dataTypeId, /* node id: user supplied */
        OUT BOOL* isDynamicPayload, /* true if dynamic */
        BOOL nonH235 /*If true means remove h235Media.mediaType level */
        )
{
  int pathId, dataId,ret;
  HPST synConf = pvtGetSynTree(hVal, confRootId);

  pathId = pvtAddRoot(hVal, synConf, 0, NULL);
  if (pathId<0)
      return pathId;
  __pvtBuildByFieldIds(dataId, hVal, pathId,
                    {_h245(channels) _nul(1) _h245(name) LAST_TOKEN},
                    (INT32)strlen(channelName), channelName);

  if (pvtSearchPath(hVal, confRootId, hVal, pathId, TRUE) == TRUE)  { /* found entry */
    int channelTableId = pvtParent(hVal, pvtParent(hVal, dataId));
    INT32 index;
    int tableEntryId, dtId, nonH235DtId;
    int confChanTableId;

    pvtGet(hVal, channelTableId, NULL, NULL, &index, NULL);
    confChanTableId = pvtGetChild(hVal, confRootId, __h245(channels), NULL);
    tableEntryId = pvtGetByIndex(hVal, confChanTableId, index, NULL);
    dtId = pvtGetChild(hVal, tableEntryId, __h245(dataType), NULL);
    if (isDynamicPayload)
      *isDynamicPayload =
    (pvtGetChild(hVal, tableEntryId, __h245(isDynamicPayloadType), NULL) >=0)?TRUE:FALSE;

    if (nonH235 && (nonH235DtId=pvtGetChild2(hVal,dtId,__h245(h235Media) , __h245(mediaType)))>=0)
        ret =pvtSetTree(hVal, dataTypeId, hVal, nonH235DtId);
    else
        ret =pvtSetTree(hVal, dataTypeId, hVal, dtId);
    pvtDelete(hVal, pathId);
    if(ret<0)
    return ret;
    return TRUE;
  }


  pvtDelete(hVal, pathId);
  return RVERROR; /* not found */
}


int /* real number of channels or RVERROR */
confGetChannelNames(
            /* build array of channels names as in configuration */
            IN  HPVT hVal,
            IN  int confRootId,
            IN  int nameArSize, /* number of elements in nameArray */
            IN  int nameLength, /* length of each name in array */
            OUT char** nameArray /* user allocated array of strings */
            )
{
  int chanSetId, chanEntryId;
  int i, len;

  if (!hVal || confRootId<0 || nameArSize<1 || !nameArray) return RVERROR;

  chanSetId = pvtGetChild(hVal, confRootId, __h245(channels), NULL);
  if (chanSetId <0) return RVERROR;


  for (i=1; i<=min(nameArSize, pvtNumChilds(hVal, chanSetId)); i++) { /* loop all channels */
    chanEntryId = pvtGetByIndex(hVal, chanSetId, i, NULL); /* entry */
    len = pvtGetString(hVal, pvtGetChild(hVal, chanEntryId, __h245(name), NULL), nameLength, nameArray[i-1]);
    nameArray[i-1][len]=0;
  }
  nameArray[i-1]=NULL;

  return pvtNumChilds(hVal, chanSetId);
}











int /* TRUE if found. RVERROR if not found */
confGetModeEntry(
         /* Search mode name in configuration. */
         IN  HPVT hVal,
         IN  int confRootId,
         IN  char *modeName, /* in conf. */
         OUT INT32 *entryId /* mode entry id */
         )
{
  int pathId, dataId;
  HPST synConf = pvtGetSynTree(hVal, confRootId);
  INT32 index=RVERROR;
  int ret = RVERROR;

  if (!hVal || !synConf || !entryId || !modeName) return RVERROR;

  pathId = pvtAddRoot(hVal, synConf, 0, NULL);
  if (pathId<0)
      return pathId;
  __pvtBuildByFieldIds(dataId, hVal, pathId,
                       {_h245(modes) _nul(1) _h245(name) LAST_TOKEN},
                       (INT32)strlen(modeName), modeName);

  if (pvtSearchPath(hVal, confRootId, hVal, pathId, TRUE) == TRUE)  { /* found entry */
    pvtGet(hVal, pvtParent(hVal, pvtParent(hVal, dataId)), NULL, NULL, &index, NULL);
    ret = TRUE;
  }

  if (entryId) {
    int modesId = pvtGetChild(hVal, confRootId, __h245(modes), NULL);
    *entryId=pvtGetByIndex(hVal, pvtGetByIndex(hVal, modesId, index, NULL), 2, NULL);
  }

  pvtDelete(hVal, pathId);
  return ret;
}
#ifdef __cplusplus
}
#endif



