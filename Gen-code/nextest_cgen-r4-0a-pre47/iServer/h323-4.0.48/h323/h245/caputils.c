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
   caputils.c

   Ron S.
   18 Sep. 1996
   */

#include <pvaltree.h>
#include <psyntreeStackApi.h>
#include <caputils.h>
#include <conf.h>
#include <h245.h>

#ifndef NOLOGSUPPORT
/*
static char *capDirectionA[] = {
  (char*)"", (char*)"Receive", (char*)"Transmit", (char*)"Receive and Transmit"
};

static char *capDataTypeA[] = {
  (char*)"Empty", (char*)"Audio", (char*)"Video", (char*)"Data", (char*)"Non standard",(char*)"User Input",
  (char*)"Conference",(char*)"H235 Security Capability",(char*)"Max Pending Replacement For",(char*)"Generic"};
*/
#endif

typedef enum {
  capabilityNonStandard=1,
  capabilityReceiveVideoCapability,
  capabilityTransmitVideoCapability,
  capabilityReceiveAndTransmitVideoCapability,
  capabilityReceiveAudioCapability,
  capabilityTransmitAudioCapability,
  capabilityReceiveAndTransmitAudioCapability,
  capabilityReceiveDataApplicationCapability,
  capabilityTransmitDataApplicationCapability,
  capabilityReceiveAndTransmitDataApplicationCapability,
  capabilityH233EncryptionTransmitCapability,
  capabilityH233EncryptionReceiveCapability,
  capabilityConferenceCapability,
  capabilityH235SecurityCapability,
  capabilityMaxPendingReplacementFor,
  capabilityReceiveUserInputCapability,
  capabilityTransmitUserInputCapability,
  capabilityReceiveAndTransmitUserInputCapability,
  capabilityGenericControlCapability,
  capabilityreceiveMultiplexedStreamCapability,
  capabilitytransmitMultiplexedStreamCapability,
  capabilityreceiveAndTransmitMultiplexedStreamCapability,
  capabilityreceiveRTPAudioTelephonyEventCapability,
  capabilityreceiveRTPAudioToneCapability
} capabilityType;



/*_________________________________________________________________________________*/
int
capStructBuild(
           IN  HPVT hVal,
           IN  int capEntryId, /* the entry Id of the requested capability  */
           cmCapStruct *capability
           )
{
  HPST synCap = pvtGetSynTree(hVal, capEntryId);
  int capEntryNumberId, capId, capItemId;
  int choice;
  INTPTR fieldId;

  capEntryNumberId = pvtGetByIndex(hVal, capEntryId, 1, NULL); /* entry number of the requested CapabilityTableEntry */
  pvtGet(hVal, capEntryNumberId, NULL, NULL, &(capability->capabilityId), NULL); /* get requested CapabilityTableEntry from capabilityTable */
  capId = pvtGetByIndex(hVal, capEntryId, 2, NULL); /* get capability from CapabilityTableEntry*/

  capability->direction = cmCapReceiveAndTransmit; /* set default */

  if (capId<0)
  {
    /* no capability entry - return an empty struct */
    capability->name = (char*)"empty";
    capability->type = cmCapEmpty;
    capability->capabilityHandle = RVERROR;
    return TRUE;
  }

  capability->name = (char*)"Error"; /* set default in case we didn't implement a case */
  capability->type = cmCapNonStandard; /* set default */

  capItemId = pvtChild(hVal, capId); /* get the capability CHOICE */
  capability->capabilityHandle = pvtChild(hVal, capItemId); /* get the capability itself */
  choice = pvtGetSyntaxIndex(hVal, capItemId); /* get the index of the CHICE for the switch */

  switch(choice) {
  case capabilityNonStandard:
    pvtGet(hVal, capItemId, &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId); /* set name */
    capability->direction = cmCapReceiveAndTransmit; /* set direction */
    capability->type = cmCapNonStandard; /* set type */
    capability->capabilityHandle = capItemId; /* go one up - why? */
    break;

  case capabilityReceiveVideoCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapVideo;
    break;
  case capabilityTransmitVideoCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapTransmit;
    capability->type = cmCapVideo;
    break;
  case capabilityReceiveAndTransmitVideoCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapVideo;
    break;

  case capabilityReceiveAudioCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapAudio;
    break;
  case capabilityTransmitAudioCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapTransmit;
    capability->type = cmCapAudio;
    break;
  case capabilityReceiveAndTransmitAudioCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapAudio;
    break;

  case capabilityReceiveDataApplicationCapability:
    pvtGet(hVal, pvtChild(hVal, pvtChild(hVal, capItemId)), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapData;
    capability->capabilityHandle = pvtChild(hVal, capability->capabilityHandle); /* get the application CHOICE */
    break;
  case capabilityTransmitDataApplicationCapability:
    pvtGet(hVal, pvtChild(hVal, pvtChild(hVal, capItemId)), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapTransmit;
    capability->type = cmCapData;
    capability->capabilityHandle = pvtChild(hVal, capability->capabilityHandle); /* get the application CHOICE */
    break;
  case capabilityReceiveAndTransmitDataApplicationCapability:
    pvtGet(hVal, pvtChild(hVal, pvtChild(hVal, capItemId)), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapData;
    capability->capabilityHandle = pvtChild(hVal, capability->capabilityHandle); /* get the application CHOICE */
    break;
  case capabilityH233EncryptionTransmitCapability:
    break;
  case capabilityH233EncryptionReceiveCapability:
    break;
  case capabilityConferenceCapability:
    capability->name = (char*)"Conference";
    capability->type = cmCapConference;
    capability->capabilityHandle=capId; /* output the whole capability - why? */
    break;
  case capabilityH235SecurityCapability:
    capability->name = (char*)"H235";
    capability->type = cmCapH235;
    capability->capabilityHandle=capId; /* output the whole capability - why? */
    break;
  case capabilityMaxPendingReplacementFor:
    capability->name = (char*)"MaxPendingReplacementFor";
    capability->type = cmCapMaxPendingReplacementFor;
    capability->capabilityHandle=capId; /* output the whole capability - why? */
    break;
  case capabilityReceiveUserInputCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapUserInput;
    break;
  case capabilityTransmitUserInputCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapTransmit;
    capability->type = cmCapUserInput;
    break;
  case capabilityReceiveAndTransmitUserInputCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapUserInput;
    break;
  case capabilityGenericControlCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapGeneric;
    break;

  case capabilityreceiveMultiplexedStreamCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapMultiplexedStream;
    break;
  case capabilitytransmitMultiplexedStreamCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapTransmit;
    capability->type = cmCapMultiplexedStream;
    break;
  case capabilityreceiveAndTransmitMultiplexedStreamCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceiveAndTransmit;
    capability->type = cmCapMultiplexedStream;
    break;
  case capabilityreceiveRTPAudioTelephonyEventCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapAudioTelephonyEvent;
    break;
  case capabilityreceiveRTPAudioToneCapability:
    pvtGet(hVal, pvtChild(hVal, capItemId), &fieldId, NULL, NULL, NULL);
    capability->name = pstGetFieldNamePtr(synCap, fieldId);
    capability->direction = cmCapReceive;
    capability->type = cmCapAudioTone;
    break;

  default:
    break;
  }

  return TRUE;
}


int
capSetBuild(
        /* build array of capability set */
        IN  HPVT hVal,
        IN  int termCapSetId, /* terminalCapabilitySet node id */
        IN  int capabilitiesSize, /* number of elements in capabilities array */
        OUT cmCapStruct** capabilities /* cap names array */
        )
{
  int capTableId, capEntryId;
  int i;

  capTableId = pvtGetChild(hVal, termCapSetId, __h245(capabilityTable),NULL);

  if (capTableId <0)
  {
        capabilities[0]=NULL;
        return RVERROR;
  }
  for (i=1; i<=min(capabilitiesSize+1, pvtNumChilds(hVal, capTableId)); i++) { /* loop all caps */
    capEntryId = pvtGetByIndex(hVal, capTableId, i, NULL); /* entry */
    capStructBuild(hVal, capEntryId, capabilities[i-1]);
  }
  capabilities[i-1]=NULL;

  return TRUE;
}



cmCapStruct *
capSetFind(
       IN  cmCapStruct **capabilities,
       IN  int capTableEntryNumber
       )
{
  int i;

  for (i=0; capabilities[i]; i++)
    if (capabilities[i]->capabilityId == capTableEntryNumber)
      return capabilities[i];
  return NULL;
}




int
capDescBuild(
         /* build array of capability set */
         IN  HPVT hVal,
         IN  int termCapSetId, /* terminalCapabilitySet node id */
         IN  cmCapStruct** capabilities, /* cap names array */
         IN  int capDescSize, /* number of elements in capDesc array */
         OUT void** capDesc /* descriptors */
         )

  /*
     A: Alternative
     S: Simulaneous
     D: Descriptor
     -: null

     (capDesc)
     \/
     ---------------------------------------------------------
     | D D ..D - S S S - S S - ==>  ...  <== A A - A A A A - |
     ---------------------------------------------------------
     */

{
  int capDescId;
  int Spos=0, Apos=capDescSize, i;

  capDescId = pvtGetChild(hVal, termCapSetId, __h245(capabilityDescriptors),NULL);

  if (capDescId <0)
  {
        capDesc[0]=NULL;
        return RVERROR;
  }
  Spos=pvtNumChilds(hVal, capDescId)+1;
  if (Spos>capDescSize) return RVERROR;
  capDesc[Spos-1] = NULL; /* D delimeter */

  for (i=0; i<pvtNumChilds(hVal, capDescId); i++) {
    int capDescEntryId, capSimId, j;

    capDesc[i] = &capDesc[Spos];
    capDescEntryId = pvtGetByIndex(hVal, capDescId, i+1, NULL); /* entry */
    capSimId = pvtGetByIndex(hVal, capDescEntryId, 2, NULL); /* sim. capability */

    for (j=0; j<pvtNumChilds(hVal, capSimId); j++) {
      int k, capSimEntryId;

      capSimEntryId = pvtGetByIndex(hVal, capSimId, j+1, NULL); /* entry */
      Apos = Apos - pvtNumChilds(hVal, capSimEntryId) - 1;
      capDesc[Spos++] = &capDesc[Apos];
      if (Spos>=Apos) return RVERROR; /* overflow */

      for (k=0; k<pvtNumChilds(hVal, capSimEntryId); k++) {
    int capAltEntryId;
    INT32 capTableEntryNumber;

    capAltEntryId = pvtGetByIndex(hVal, capSimEntryId, k+1, NULL); /* entry */
    pvtGet(hVal, capAltEntryId, NULL, NULL, &capTableEntryNumber, NULL);
    capDesc[Apos+k] = (void*)capSetFind(capabilities, (int)capTableEntryNumber);
      }
      capDesc[Apos+k] = NULL;

    }
    capDesc[Spos++] = NULL;
  }

  return TRUE;
}


/*_________________________________________________________________________________*/

int
capStructBuildFromStruct(
             /* build single capability entry */
             IN  HPVT hVal,
             IN  int confRootId, /* configuration root id */
             OUT int capId,
             IN  cmCapStruct *capability
             )
{
  HPST synConf = pvtGetSynTree(hVal, confRootId);
  int ret;
  int dataTypeId=-1, capDataId=-1, capsetDataId=-1;
  INT16 typeNameId=LAST_TOKEN;
  int iTypeNameId;

  if (!capability || capId<0) return RVERROR;

  if (capability->name) { /* -- name */
    dataTypeId = pvtAddRootByPath(hVal,synConf, (char *)"channels.0.dataType", 0, NULL);
    if (confGetDataType(hVal, confRootId, capability->name, dataTypeId, NULL, FALSE) <0) return RVERROR;
    capDataId = pvtChild(hVal, dataTypeId);
  }
  else {
    if (capability->capabilityHandle <0) return RVERROR;
    capDataId=capability->capabilityHandle;
  }

  switch(capability->type) {
  case cmCapNonStandard:
    typeNameId = __h245(nonStandard);
    break;

  case cmCapVideo:
    switch (capability->direction) {
    case cmCapReceive: typeNameId = __h245(receiveVideoCapability); break;
    case cmCapTransmit: typeNameId = __h245(transmitVideoCapability); break;
    case cmCapReceiveAndTransmit: typeNameId = __h245(receiveAndTransmitVideoCapability); break;
    default: break;
    }
    break;

  case cmCapAudio:
    switch (capability->direction) {
    case cmCapReceive: typeNameId = __h245(receiveAudioCapability); break;
    case cmCapTransmit: typeNameId = __h245(transmitAudioCapability); break;
    case cmCapReceiveAndTransmit: typeNameId = __h245(receiveAndTransmitAudioCapability); break;
    default: break;
    }
    break;

  case cmCapData:
    switch (capability->direction) {
    case cmCapReceive: typeNameId = __h245(receiveDataApplicationCapability); break;
    case cmCapTransmit: typeNameId = __h245(transmitDataApplicationCapability); break;
    case cmCapReceiveAndTransmit: typeNameId = __h245(receiveAndTransmitDataApplicationCapability); break;
    default: break;
    }
    break;

  case cmCapUserInput:
    switch (capability->direction) {
    case cmCapReceive: typeNameId = __h245(receiveUserInputCapability); break;
    case cmCapTransmit: typeNameId = __h245(transmitUserInputCapability); break;
    case cmCapReceiveAndTransmit: typeNameId = __h245(receiveAndTransmitUserInputCapability); break;
    default: break;
    }
    break;
  case cmCapConference:
    typeNameId = __h245(conferenceCapability);
    break;
  case cmCapH235:
    typeNameId = __h245(h235SecurityCapability);
    break;
  case cmCapMaxPendingReplacementFor:
    typeNameId = __h245(maxPendingReplacementFor);
    break;
  case cmCapGeneric:
    typeNameId = __h245(genericControlCapability);
    break;
  case cmCapMultiplexedStream:
    switch (capability->direction) {
    case cmCapReceive: typeNameId = __h245(receiveMultiplexedStreamCapability); break;
    case cmCapTransmit: typeNameId = __h245(transmitMultiplexedStreamCapability); break;
    case cmCapReceiveAndTransmit: typeNameId = __h245(receiveAndTransmitMultiplexedStreamCapability); break;
    }
    break;
  case cmCapAudioTelephonyEvent:
    typeNameId = __h245(receiveRTPAudioTelephonyEventCapability);
    break;
  case cmCapAudioTone:
    typeNameId = __h245(receiveRTPAudioToneCapability);
    break;

  default: break;
  }

  if (typeNameId == LAST_TOKEN) {
    pvtDelete(hVal, dataTypeId);
    return RVERROR;
  }

  iTypeNameId = typeNameId;
  capsetDataId = pvtAdd(hVal, capId, iTypeNameId, 0, NULL, NULL);
  if (capsetDataId <0)
      return capsetDataId ;
  if ((ret=pvtAddChilds(hVal, capsetDataId, hVal, capDataId)) <0) return ret;
  pvtDelete(hVal, dataTypeId);
  return TRUE;
}




int
capSetBuildFromStruct(
              /* Build capability table from capability structure array.
             - The capabilityId field is updated here.
             - if name != 0 then the configuration channel data definition is used.
             - if name == 0 and capabilityHandle >=0 then the specified data tree is used.
             - type and direction values shall be set.
             */
              IN  HPVT hVal,
              IN  int confRootId, /* configuration root id */
              OUT int termCapSetId, /* terminalCapabilitySet node id */
              IN  cmCapStruct** capabilities /* cap names array */
              )
{
  int capTableId, capEntryId, capId;
  int i,ret;

  if (!capabilities || termCapSetId<0) return RVERROR;

  capTableId = pvtAdd(hVal, termCapSetId, __h245(capabilityTable), 0, NULL, NULL);
  if (capTableId <0) return capTableId;

  for (i=1; capabilities[i-1]; i++) { /* loop all caps */
    if ( (capEntryId = pvtAdd(hVal, capTableId, 0, -556, NULL, NULL)) <0) return capEntryId;
    if ( (ret=pvtAdd(hVal, capEntryId, __h245(capabilityTableEntryNumber), i, NULL, NULL)) <0) return ret;
    capabilities[i-1]->capabilityId = i;
    if ( (capId = pvtAdd(hVal, capEntryId, __h245(capability), i, NULL, NULL)) <0) return capId;

    capStructBuildFromStruct(hVal, confRootId, capId, capabilities[i-1]);
  }

  return TRUE;
}


int
capDescBuildFromStruct(
               /* build capability combinations from nested array.
              - The capabilityId shall be set to correct value, meaning
              this is called after capStructBuildFromStruct().
              */
               IN  HPVT hVal,
               OUT int termCapSetId, /* terminalCapabilitySet node id */
               IN  cmCapStruct*** capabilities[] /* cap names array */
               )
{
  int i, j, k,ret;
  int capDescId, capDescEntryId, capSimId, capSimEntryId;

  if (!capabilities || termCapSetId<0) return RVERROR;

  capDescId = pvtAdd(hVal, termCapSetId, __h245(capabilityDescriptors), 0, NULL, NULL);
  if (capDescId <0) return capDescId;

  for (i=0; capabilities[i]; i++) {
    if ( (capDescEntryId = pvtAdd(hVal, capDescId, 0, -556, NULL, NULL)) <0) return capDescEntryId;
    if ( (ret = pvtAdd(hVal, capDescEntryId, __h245(capabilityDescriptorNumber), i, NULL, NULL)) <0) return ret;
    if ( (capSimId = pvtAdd(hVal, capDescEntryId, __h245(simultaneousCapabilities), 0, NULL, NULL)) <0)
      return capSimId;

    for (j=0; capabilities[i][j]; j++) {
      if ( (capSimEntryId = pvtAdd(hVal, capSimId, 0, -556, NULL, NULL)) <0) return capSimEntryId;

      for (k=0; capabilities[i][j][k]; k++) {
    if ((ret=pvtAdd(hVal, capSimEntryId, 0, capabilities[i][j][k]->capabilityId, NULL, NULL)) <0)
      return ret;
      }
    }
  }

  return TRUE;
}

#ifdef __cplusplus
}
#endif



