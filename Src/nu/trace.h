#ifndef NULLOSFT_MEDIALIBRARY_TRACE_HEADER
#define NULLOSFT_MEDIALIBRARY_TRACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef _DEBUG

#ifdef _WIN32
#include <wtypes.h>
#else
// wtypes.h is a Windows SDK header that does not exist on Linux. Provide the
// minimal surface trace uses. trace's wide path is native wchar_t (see the
// strsafe.h wide port), so LPCWSTR is const wchar_t*, NOT the 16-bit WCHAR used
// elsewhere in the codebase. OutputDebugString{,A,W} are declared here
// (canonical home is bfc/platform/linux.h); the test harness provides a stderr
// sink until Wasabi itself builds.
typedef const char *LPCSTR;
typedef const wchar_t *LPCWSTR;
#ifdef __cplusplus
extern "C" {
#endif
void OutputDebugString(const char *s);
void OutputDebugStringA(const char *s);
void OutputDebugStringW(const wchar_t *s);
#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

void DebugPrintfA(LPCSTR format, ...);
void DebugPrintfW(LPCWSTR format, ...);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define DebugPrintf DebugPrintfW
#else
#define DebugPrintf DebugPrintfA
#endif // !UNICODE

#define aTRACE OutputDebugStringA
#define aTRACE_FMT DebugPrintfA
#define aTRACE_LINE(x) aTRACE_FMT("%s\n", (x))

#define wTRACE OutputDebugStringW
#define wTRACE_FMT DebugPrintfW
#define wTRACE_LINE(x) wTRACE_FMT(L"%s\n", (x))

#define TRACE OutputDebugString
#define TRACE_FMT DebugPrintf
#define TRACE_LINE(x) TRACE_FMT(TEXT("%s\n"), (x))

#else // _DEBUG

#define aTRACE
#define aTRACE_FMT
#define aTRACE_LINE

#define wTRACE
#define wTRACE_FMT
#define wTRACE_LINE

#define TRACE
#define TRACE_FMT
#define TRACE_LINE

#endif // _DEBUG
#endif // NULLOSFT_MEDIALIBRARY_TRACE_HEADER