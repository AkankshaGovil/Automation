#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/


#ifndef __CMCFGRT_H
#define __CMCFGRT_H



#include <rvcommon.h>

RVAPI
void RVCALLCONV cmGetCfgRootNameList(
                IN      HAPP                hApp,
                OUT     int *               count,
                OUT     const char ***      list);

RVAPI const unsigned char * RVCALLCONV
cmGetSyntaxFromCfgName(
                    IN   HAPP               hApp,
                    IN   const char *       rootName);


RVAPI
const char * RVCALLCONV cmGetAsnNameFromCfgName(
                IN      HAPP                hApp,
                IN      const char *        rootName);

RVAPI
const char * RVCALLCONV cmGetCfgNameFromAsnName(
                IN      HAPP                hApp,
                IN      const char *        asnName);




#endif  /* __CMCFGRT_H */
#ifdef __cplusplus
}
#endif



