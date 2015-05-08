/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/************************************************************************
 *
 *
 *              Unix flavors
 *
 * __VXWORKS__
 * _ALPHA_
 ************************************************************************/

#ifndef __RVDEFUNIX_H
#define __RVDEFUNIX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __VXWORKS__

/* VxWorks */
#include <vxWorks.h>

#elif RV_OS_OSE
/* -------------------------- OSE ------------------------- */
#include <ose.h>
#elif __PSOSPNA__

/* pSOS - empty */

#else  /* __VXWORKS__ / __PSOSPNA__ */

/* Not pSOS or VxWorks */
#include <unistd.h>
#include <memory.h>

#endif  /* __VXWORKS__ / __PSOSPNA__ */

/* Any */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/************************************************************************
 * Function charecteristics definitions
 ************************************************************************/
#define CDECL
#define RVAPI
#define RVCALLCONV
#define CALLCONVC


/************************************************************************
 * Type definitions
 ************************************************************************/
typedef int                     ENUM;

#ifndef BYTE
typedef unsigned char           BYTE;
#endif

#ifndef __VXWORKS__

#ifndef __PSOSPNA__

#ifndef UINT
typedef unsigned int            UINT;
#endif

#else

#ifndef _TYPES_H
typedef unsigned int            UINT;
#endif

#endif /* __PSOSPNA__ */

#ifndef BOOL
typedef int                     BOOL;
#endif

typedef unsigned char           UINT8;
typedef unsigned short          UINT16;
typedef unsigned int            UINT32;
typedef char                    INT8;
typedef short                   INT16;
typedef int                     INT32;

#endif  /* __VXWORKS__ */

#ifdef _ALPHA_

typedef long long               INTPTR;
typedef unsigned long long      UINTPTR;

#else  /* _ALPHA */

typedef int                     INTPTR;
typedef unsigned int            UINTPTR;

#endif  /* _ALPHA */

#ifdef IS_PLATFORM_HPUX
#define sizeof(x) ((int)sizeof(x))
#define strlen(x) ((int)strlen(x))
#endif

/************************************************************************
 * Alignment macro
 ************************************************************************/
#ifdef _ALPHA_
#define RV_ALIGN(numBytes) (((numBytes + 7) / 8) * 8)
#else
#define RV_ALIGN(numBytes) (((numBytes + 3) / 4) * 4)
#endif


#ifdef __cplusplus
}
#endif


#endif  /* __RVDEFUNIX_H */
