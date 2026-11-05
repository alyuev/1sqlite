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

/* SQLite 3.54+ os_win calls GetFileSizeEx/SetFilePointerEx (Win2000+) and
** GetTickCount64 (Vista+): absent from VC6's SDK and kernel32.lib. Resolve
** them at runtime with fallbacks, so the DLL still loads on XP too. */
#ifdef _WIN32
#include <windows.h>
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif
static BOOL WINAPI vc6_GetFileSizeEx(HANDLE h, PLARGE_INTEGER p){
  static BOOL (WINAPI *f)(HANDLE,PLARGE_INTEGER) = 0; static int init = 0;
  DWORD hi = 0, lo;
  if( !init ){ f = (BOOL(WINAPI*)(HANDLE,PLARGE_INTEGER))GetProcAddress(GetModuleHandleA("kernel32.dll"),"GetFileSizeEx"); init = 1; }
  if( f ) return f(h,p);
  lo = GetFileSize(h,&hi);
  if( lo==INVALID_FILE_SIZE && GetLastError()!=NO_ERROR ) return FALSE;
  p->LowPart = lo; p->HighPart = (LONG)hi; return TRUE;
}
static BOOL WINAPI vc6_SetFilePointerEx(HANDLE h, LARGE_INTEGER d, PLARGE_INTEGER pNew, DWORD m){
  static BOOL (WINAPI *f)(HANDLE,LARGE_INTEGER,PLARGE_INTEGER,DWORD) = 0; static int init = 0;
  LONG hi = d.HighPart; DWORD lo;
  if( !init ){ f = (BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,PLARGE_INTEGER,DWORD))GetProcAddress(GetModuleHandleA("kernel32.dll"),"SetFilePointerEx"); init = 1; }
  if( f ) return f(h,d,pNew,m);
  lo = SetFilePointer(h,(LONG)d.LowPart,&hi,m);
  if( lo==INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR ) return FALSE;
  if( pNew ){ pNew->LowPart = lo; pNew->HighPart = hi; }
  return TRUE;
}
static ULONGLONG WINAPI vc6_GetTickCount64(VOID){
  static ULONGLONG (WINAPI *f)(VOID) = 0; static int init = 0;
  if( !init ){ f = (ULONGLONG(WINAPI*)(VOID))GetProcAddress(GetModuleHandleA("kernel32.dll"),"GetTickCount64"); init = 1; }
  return f ? f() : (ULONGLONG)GetTickCount();
}
#define GetFileSizeEx    vc6_GetFileSizeEx
#define SetFilePointerEx vc6_SetFilePointerEx
#define GetTickCount64   vc6_GetTickCount64
#endif
#endif
