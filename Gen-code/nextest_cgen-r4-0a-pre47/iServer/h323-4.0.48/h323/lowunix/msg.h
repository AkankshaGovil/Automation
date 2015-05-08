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
  msg.h

  Messages printing and filtering routines

  Ron Shpilman
  15 August 1995
  Revised: 14 July 1997

 */

#ifndef __MSG__
#define __MSG__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NOLOGSUPPORT


void msOpen(void);
void msClose(void);
void msPrint(int type, char *line);
void msSetStackNotify(void (*sN)(char *line,...));

/* -- filtering */

BOOL /* TRUE if type debugging should be printed */
msIsSelected(int type);


RVAPI int RVCALLCONV msGetDebugLevel(void);

#if 0
char* msGetPrintBuffer(void);
#else
#define msGetPrintBuffer() THREAD_GetMSABuffer(NULL)
#endif



int /* TRUE or RVERROR */
msAdd(
      /* Add module to debug printing list */
      char* moduleName
      );

int /* TRUE or RVERROR */
msDelete(
     /* Delete module from debug printing list */
     char* moduleName
     );

int /* TRUE or RVERROR */
msDeleteAll(void
        /* Delete all modules from debug printing list */
        );

int
msGetDebugLevel(void);

int
msSetDebugLevel(int debugLevel);


/* -- sinks */

int /* TRUE if valid sink name */
msSinkAdd(
      /* Add sink */
      char* sinkName
      );

int /* TRUE if valid sink name */
msSinkDelete(
         /* Delete sink */
         char* sinkName
         );

void
msFile(
       /* Set the configuration file name. */
       char *name
       );

void
msLogFile(
      /* Set the log outpug file name.  */
      char *name
      );



#endif  /* NOLOGSUPPORT */

#ifdef __cplusplus
}
#endif

#endif  /* __MSG__ */
