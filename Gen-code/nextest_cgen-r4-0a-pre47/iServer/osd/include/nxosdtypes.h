#ifndef NXOSDTYPES_H
#define NXOSDTYPES_H
#include "osdconfig.h"
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if !HAVE_LONGLONG_T
typedef long long longlong_t;
#endif

#if !HAVE_HRTIME_T
typedef longlong_t hrtime_t;
#endif
#if !HAVE_UCHAR_T
typedef unsigned char uchar_t;
#endif

#if !HAVE_ULONG_T
typedef unsigned long ulong_t;
#endif

//Macro to cast argument to unsigned long
#define ULONG_FMT(arg) (unsigned long)arg

#endif /* NXOSDTYPES_H */
