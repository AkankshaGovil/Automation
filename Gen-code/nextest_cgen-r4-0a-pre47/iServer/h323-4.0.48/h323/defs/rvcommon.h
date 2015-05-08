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
 * rvcommon.h
 *
 *
 * Common definitions for RADVISION project
 *
 ************************************************************************/

#ifndef __RVCOMMON_H
#define __RVCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32)

#ifndef WIN32
#define WIN32
#endif

#if defined(__RV_SYS) || defined(__RV_VXD)
            /* Kernel mode drivers for RTP in WinNT and Win95 */
#include <rvdefring0.h>

#else

#ifdef UNDER_CE
            /* Windows CE */
#include <rvdefwince.h>
#else
            /* Windows OS */
#include <rvdefwin.h>
#endif  /* UNDER_CE */
#endif  /* defined(__RV_SYS) || defined(__RV_VXD) */


#else
#include <rvdefunix.h>
#endif  /* defined(WIN32) && defined(_WIN32) */



/************************************************************************
 * Function parameter indications
 ************************************************************************/
#define IN
#define OUT
#define INOUT
#define OUTIN
#define OPTIONAL




/************************************************************************
 * Standard return values
 *
 * Any non-negative value indicates success
 * SUCCESS              - Function succeeded
 * OK                   - Function succeeded
 *
 * Any negative value indicates error
 * RVERROR              - General error has occured
 * RESOURCES_PROBLEM    - Error allocating memory due to resources problem
 * OBJECT_WAS_NOT_FOUND - Couldn't found the object desired
 ************************************************************************/
#ifdef SUCCESS
#undef SUCCESS
#endif
#define SUCCESS 1

#ifndef OK
#define OK 0
#endif

#ifdef IS_ERROR
#undef IS_ERROR
#endif
#define IS_ERROR(error) ((error)<0)

#ifdef RVERROR
#undef RVERROR
#endif
#define RVERROR -1

#ifdef RESOURCES_PROBLEM
#undef RESOURCES_PROBLEM
#endif
#define RESOURCES_PROBLEM -2

#ifdef OBJECT_WAS_NOT_FOUND
#undef OBJECT_WAS_NOT_FOUND
#endif
#define OBJECT_WAS_NOT_FOUND -3



/************************************************************************
 * DECLARE_OPAQUE
 * Definition used to create a general purpose handle. All of the stack's
 * handles are defined using this macro.
 ************************************************************************/
#ifndef DECLARE_OPAQUE
#define DECLARE_OPAQUE(name)    typedef struct { int unused; } name##__ ; \
                typedef const name##__ * name; \
                typedef name*  LP##name
#endif


/************************************************************************
 * Boolean declarations
 ************************************************************************/
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/************************************************************************
 * nprn  - Makes sure a string is printed and not core dumps on NULL strings
 * nprnd - Returns the value inside a pointer or an error if pointer is NULL
 ************************************************************************/
#ifndef nprn
#define nprn(s) ((s)?(s):"(null)")
#define nprnd(d) ((d)?(*d):RVERROR)
#endif



/************************************************************************
 * Here for backward compatability...
 * Applications that don't use the definitions written below (CALLCONV,
 * RVERROR, etc), can define NONOTRVDEFS and compile their code.
 * This will ensure that these macros can be defined by the OS itself.
 * In Windows, for example, ERROR is defined to 0 and not -1.
 ************************************************************************/
#ifndef NONOTRVDEFS

#ifdef CALLCONV
#undef CALLCONV
#endif
#define CALLCONV RVCALLCONV

#ifdef ERROR
#undef ERROR
#endif
#define ERROR RVERROR

#endif  /* NONOTRVDEFS */



#ifdef __cplusplus
}
#endif

#endif  /* __RVCOMMON_H */

