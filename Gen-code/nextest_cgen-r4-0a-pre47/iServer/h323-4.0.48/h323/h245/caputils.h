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
   caputils.h

   Ron S.
   19 Sep. 1996
   */

#ifndef _CAPUTILS_
#define _CAPUTILS_

#include <rvinternal.h>
#include <cm.h>
#include <ms.h>


int
capStructBuild(
           IN  HPVT hVal,
           IN  int capEntryId,
           cmCapStruct *capability
           );

int
capSetBuild(
        /* build array of capability set */
        IN  HPVT hVal,
        IN  int termCapSetId, /* terminalCapabilitySet node id */
        IN  int capabilitiesSize, /* number of elements in capabilities array */
        OUT cmCapStruct** capabilities /* cap names array */
        );

cmCapStruct *
capSetFind(
       IN  cmCapStruct **capabilities,
       IN  int capTableEntryNumber
       );

int
capDescBuild(
         /* build array of capability set */
         IN  HPVT hVal,
         IN  int termCapSetId, /* terminalCapabilitySet node id */
         IN  cmCapStruct** capabilities, /* cap names array */
         IN  int capDescSize, /* number of elements in capDesc array */
         OUT void** capDesc /* descriptors */
         );


int
capStructBuildFromStruct(
             /* build single capability entry */
             IN  HPVT hVal,
             IN  int confRootId, /* configuration root id */
             OUT int capId,
             IN  cmCapStruct *capability
             );
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
              );

int
capDescBuildFromStruct(
               /* build capability combinations from nested array.
              - The capabilityId shall be set to correct value, meaning
              this is called after capStructBuildFromStruct().
              */
               IN  HPVT hVal,
               OUT int termCapSetId, /* terminalCapabilitySet node id */
               IN  cmCapStruct*** capabilities[] /* cap names array */
               );


#endif
#ifdef __cplusplus
}
#endif



