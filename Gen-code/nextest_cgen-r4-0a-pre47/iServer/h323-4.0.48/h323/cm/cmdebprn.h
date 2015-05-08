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


#ifndef _CMDEBPRN_
#define _CMDEBPRN_
#include <cm.h>

void cmiAPIEnter(HAPP hApp,const char*line,...);
void cmiAPIExit(HAPP hApp,const char*line,...);
void cmiCBEnter(HAPP hApp,const char*line,...);
void cmiCBExit(HAPP hApp,const char*line,...);
#ifdef NOLOGSUPPORT
#define cmiAPIEnter if (0) cmiAPIEnter
#define cmiAPIExit if (0) cmiAPIExit
#define cmiCBEnter if (0) cmiCBEnter
#define cmiCBExit if (0) cmiCBExit
#endif


#endif
#ifdef __cplusplus
}
#endif
