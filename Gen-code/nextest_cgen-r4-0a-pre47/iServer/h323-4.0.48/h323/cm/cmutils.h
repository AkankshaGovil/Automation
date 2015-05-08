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

#ifndef CMUTILS_H
#define CMUTILS_H

#include <cmintr.h>
#include <cm.h>

int setNonStandard(
        IN  HPVT  hVal,
        IN  int   nodeId,
        IN  cmNonStandard*nonStandard
);
int setNonStandardParam(
        IN  HPVT  hVal,
        IN  int   vNodeId,
        IN  cmNonStandardParam*nonStandard
);

int getNonStandard(
        IN  HPVT  hVal,
        IN  int   nodeId,
        OUT cmNonStandard*nonStandard
);

int getNonStandardParam(
        IN  HPVT  hVal,
        IN  int   vNodeId,
        OUT cmNonStandardParam*nonStandard
);

INT32 getEnumNameId(
          IN    fieldNames* enumFieldNames,
          IN    int     enumFieldNamesSize,
          IN    unsigned    enumValue
);

int getEnumValueByNameId(
                    IN  fieldNames* enumFieldNames,
            IN  INT32       nameId
);
int aliasToVt(IN  HPVT    hVal,
          IN  cmAlias*alias,
          IN  int     nodeId);

int vtToAlias(IN  HPVT    hVal,
          IN  cmAlias*alias,
          IN  int     nodeId);


int cmVtToTA(HPVT hVal,int nodeId, cmTransportAddress* ta);

int cmTAToVt(HPVT hVal,int nodeId, cmTransportAddress* ta);

int cmVtToTA_H245(HPVT hVal,int nodeId, cmTransportAddress* ta);

int cmTAToVt_H245(HPVT hVal,int nodeId, cmTransportAddress* ta);



/**************************************************************************************
 * getEncodeDecodeBuffer
 *
 * Purpose: To get the buffer for coding purposes.
 *
 * Input:   bufferSize   - The size of buffer required.
 *          buffer       - A pointer to the buffer.
 *          
 *
 * Output:  buffer       - A pointer to the buffer .
 *
 **************************************************************************************/
void getEncodeDecodeBuffer(IN int bufferSize, OUT BYTE **buffer);

/**************************************************************************************
 * cleanMessage
 *
 * Purpose: Deletes all tunneling elements (H.245, H.450, annex M, annex L) from the
 *          given message. This is to avoid resending tunneled messages when reusing 
 *          messages from the cm Property tree.
 *
 * Input:   hPvt - handle to the PVT of the message
 *          message  - A pvt node to the message.
 *
 * Output:  None.
 *
 **************************************************************************************/
void cleanMessage(IN HPVT hPvt, IN int message);

#endif
#ifdef __cplusplus
}
#endif



