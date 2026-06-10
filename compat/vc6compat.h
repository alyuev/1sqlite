/* vc6compat.h -- shims so modern SQLite builds with VC6's 1998 SDK. Force-included via /FI. */
#ifndef VC6COMPAT_H
#define VC6COMPAT_H
#include <math.h>
#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif
/* pointer-sized integer types absent in VC6's old basetsd (32-bit target) */
#ifndef _BASETSD_H_
#define _BASETSD_H_
typedef long           LONG_PTR;
typedef unsigned long  ULONG_PTR;
typedef unsigned long  DWORD_PTR;
typedef int            INT_PTR;
typedef unsigned int   UINT_PTR;
typedef unsigned long  SIZE_T;
typedef long           SSIZE_T;
#endif
/* GetNativeSystemInfo missing in VC6 SDK; on win32 GetSystemInfo is equivalent */
#define GetNativeSystemInfo GetSystemInfo

/* VC6 cannot convert unsigned __int64 -> double (C2520). Full-range helper. */
#ifndef VC6_U64_TO_DBL
#define VC6_U64_TO_DBL(u) ((double)(__int64)((u)>>1)*2.0 + (double)(__int64)((u)&1))
#endif
#endif
