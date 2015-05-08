/************************************************************************/
/*                                                                      */
/* Copyright (C) Teja Technologies, Inc. 2002.                          */
/*                                                                      */
/* All Rights Reserved.                                                 */
/*                                                                      */
/* This software is the property of Teja Technologies, Inc.  It is      */
/* furnished under a specific licensing agreement.  It may be used or   */
/* copied only under terms of the licensing agreement.                  */
/*                                                                      */
/* For more information, contact info@teja.com                          */
/*                                                                      */
/************************************************************************/

#ifndef TEJA_OS_TYPES_H
#define TEJA_OS_TYPES_H

typedef int32_t  TEJA_INT32;
typedef uint32_t TEJA_U_INT32;

typedef long long          TEJA_INT64;
typedef unsigned long long TEJA_U_INT64;

#define TEJA_INT64_DEFINED

#define TEJA_CONST const
#define TEJA_UNUSED_ATTRIBUTE __attribute__ ((unused))

#define TEJA_INLINE inline
#define TEJA_INLINE_ATTRIBUTE __attribute__ ((always_inline))

#endif /* TEJA_OS_TYPES_H */
