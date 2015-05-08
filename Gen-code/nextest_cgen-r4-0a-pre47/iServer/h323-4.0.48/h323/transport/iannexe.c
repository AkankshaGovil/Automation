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


#include <rvinternal.h>
#include "annexe.h"
#include "iannexe.h"

#ifdef _DEBUG
    /* safe typecast functions (only in DEBUG mode) */
    tAnnexE*        AsAnnexE( HANNEXE hannexe ) { return (tAnnexE*)hannexe; }
    HANNEXE         AsHANNEXE( tAnnexE* pannexe ) { return (HANNEXE)pannexe; }
#endif

void
hton16( UINT16 h, BYTE* ptr ) {
    ptr[1] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[0] = (BYTE)(h & 0xff);
}

void
hton24( UINT32 h, BYTE* ptr ) {
    ptr[2] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[1] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[0] = (BYTE)(h & 0xff);
}

void
hton32( UINT32 h, BYTE* ptr ) {
    ptr[3] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[2] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[1] = (BYTE)(h & 0xff);
    h >>= 8;
    ptr[0] = (BYTE)(h & 0xff);
}

UINT16
ntoh16( BYTE* ptr ) {
    UINT16 res = *ptr++;
    res <<= 8;
    res |= *ptr;
    return res;
}

UINT32
ntoh24( BYTE* ptr ) {
    UINT32 res = *ptr++;
    res = (res << 8) + *ptr++;
    return (res << 8) + *ptr;
}

UINT32
ntoh32( BYTE* ptr ) {
    UINT32 res = *ptr++;
    res = (res << 8) + *ptr++;
    res = (res << 8) + *ptr++;
    return (res << 8) + *ptr;
}

#ifdef __cplusplus
}
#endif /* __cplusplus*/
