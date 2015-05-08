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

#include <log.h>


#ifndef NOLOGSUPPORT
void printHexBuff(BYTE* buf, int len, RVHLOG msa);
#else
#define printHexBuff(buf,len,msa)
#endif







#ifdef __cplusplus
}
#endif



