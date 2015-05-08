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

#ifndef _LOGGING
#define _LOGGING

#include <rvinternal.h>

void lgWriteToLog(IN char*line);
void lgWriteToLogWithParams( IN char *line,
                 IN int param1,
                 IN int param2,
                 IN int param3,
                 IN int param4,
                 IN int param5,
                 IN int param6
                 );
#endif
#ifdef __cplusplus
}
#endif



