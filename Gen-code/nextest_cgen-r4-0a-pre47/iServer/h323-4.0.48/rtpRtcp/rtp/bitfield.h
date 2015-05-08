#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef __BITFIELD_H
#define __BITFIELD_H

#include <rvcommon.h>

UINT32 bitfieldSet(
    IN  UINT32  value,
    IN  UINT32  bitfield,
    IN  int     nStartBit,
    IN  int     nBits);

UINT32 bitfieldGet(
    IN  UINT32  value,
    IN  int     nStartBit,
    IN  int     nBits);

#endif /* __BITFIELD_H */
#ifdef __cplusplus
}
#endif



