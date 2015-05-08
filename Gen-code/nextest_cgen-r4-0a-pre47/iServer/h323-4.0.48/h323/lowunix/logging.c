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

#include <stdio.h>
#include <string.h>

#include <rvinternal.h>

#ifdef __VXWORKS__
#include <logLib.h>
#endif


void lgWriteToLog( IN char *line)
{
  if(line);
#ifdef __VXWORKS__
  line[30]=0;   /* The logger can't work well on long messages*/
  logMsg(line, 1,2,3,4,5,6);
#endif
}

void lgWriteToLogWithParams( IN char *line,
                 IN int param1,
                 IN int param2,
                 IN int param3,
                 IN int param4,
                 IN int param5,
                 IN int param6
                 )
{
  if(line)
    param1=param2=param3=param4=param5=param6;
#ifdef __VXWORKS__
  line[30]=0;   /* The logger can't work well on long messages*/
  logMsg(line, param1, param2, param3, param4, param5, param6);
#endif
}
#ifdef __cplusplus
}
#endif



