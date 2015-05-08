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

/************************************************************************
 *
 * MESSAGES API
 * This is the old LOGGING API, declared and handled here to allow
 * the source code to use these functions until we remove them
 * permanently from the code
 *
 *
 ************************************************************************/

#ifndef MS
#define MS

#include <rvinternal.h>
#include <log.h>

#undef msaPrintFormat

/* High Level Messaging API */


RVAPI void RVCALLCONV msaPrintFormat(int type,const char*line,...);
RVAPI int RVCALLCONV msGetDebugLevel(void);


/* The following functions are defined as macros that use the
   log API */
#define msaRegister(key,name,description)   (int)logRegister((RVHLOGMGR)1,name,description)
#define msaUnregister(type)                 logUnregister((RVHLOG)type)
#define msaGetName(type)                    (char *)logGetName((RVHLOG)type)
#define msaGetDebugLevel()                  msGetDebugLevel()
#define msaIsSelected(type)                 logIsSelected((RVHLOG)type,RV_INFO)






#ifndef NOLOGSUPPORT

#define msaPrintFormat if (msaGetDebugLevel()>0) msaPrintFormat

#else  /* NOLOGSUPPORT */

#define msaPrintFormat if (0) msaPrintFormat
#define msGetDebugLevel() (0)
#define msOpen()
#define msClose()

#endif  /* NOLOGSUPPORT */


#endif  /* MS */

#ifdef __cplusplus
}
#endif



