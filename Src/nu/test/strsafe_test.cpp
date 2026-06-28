// strsafe runtime proof for the Linux port.
//
// strsafe.h is header-inline here (no STRSAFE_LIB / STRSAFE_LIB_IMPL), which is
// exactly the path nu/trace.cpp uses. This test asserts the bounded-string
// contracts that justify strsafe existing at all — truncation is reported AND
// the destination stays null-terminated — for both the ANSI path (vsnprintf)
// and the freshly un-gated wide path (native-wchar_t vswprintf). It then drives
// the real nu/trace.cpp DebugPrintfA/W end to end through its inline
// StringCchVPrintfEx* calls into a local OutputDebugString sink.
//
// "Builds != correct": a green compile of strsafe.c proves the lib path links,
// but only these assertions prove the wide printf path actually behaves.

#include <strsafe.h>

#include "trace.h" // DebugPrintfA/W (declared under _DEBUG)

#include <stdio.h>
#include <string.h>
#include <wchar.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                                       \
  do {                                                                         \
    if (cond) {                                                                \
      fprintf(stderr, "  [ok]   %s\n", msg);                                   \
    } else {                                                                   \
      fprintf(stderr, "  [FAIL] %s\n", msg);                                   \
      ++g_failures;                                                            \
    }                                                                          \
  } while (0)

// --- trace sink: nu/trace.cpp links against these (Wasabi linux.cpp isn't
// built yet). Capture content so the wide printf path can be asserted, not just
// run. ---
static char g_lastA[256];
static char g_lastW[256];

extern "C" void OutputDebugString(const char *s) { (void)s; }
extern "C" void OutputDebugStringA(const char *s) {
  size_t i = 0;
  for (; s && s[i] && i < sizeof(g_lastA) - 1; ++i)
    g_lastA[i] = s[i];
  g_lastA[i] = '\0';
}
extern "C" void OutputDebugStringW(const wchar_t *s) {
  // tests use ASCII content; narrow each native wchar_t for comparison
  size_t i = 0;
  for (; s && s[i] && i < sizeof(g_lastW) - 1; ++i)
    g_lastW[i] = (char)s[i];
  g_lastW[i] = '\0';
}

int main() {
  fprintf(stderr, "=== nu strsafe unit test ===\n");

  // --- StringCchCopyA (ANSI, hand-rolled worker) ---
  {
    char d[8];
    CHECK(StringCchCopyA(d, 8, "hello") == S_OK && strcmp(d, "hello") == 0,
          "StringCchCopyA fit -> S_OK + content");

    char t[4];
    HRESULT hr = StringCchCopyA(t, 4, "hello");
    CHECK(hr == STRSAFE_E_INSUFFICIENT_BUFFER,
          "StringCchCopyA truncation -> INSUFFICIENT_BUFFER");
    CHECK(t[3] == '\0' && strcmp(t, "hel") == 0,
          "StringCchCopyA truncation null-terminated ('hel')");

    char z[4];
    CHECK(StringCchCopyA(z, 0, "x") == STRSAFE_E_INVALID_PARAMETER,
          "StringCchCopyA cchDest==0 -> INVALID_PARAMETER");
  }

  // --- StringCchCopyW (wide, hand-rolled worker) ---
  {
    wchar_t d[8];
    CHECK(StringCchCopyW(d, 8, L"hello") == S_OK && wcscmp(d, L"hello") == 0,
          "StringCchCopyW fit -> S_OK + content");

    wchar_t t[4];
    HRESULT hr = StringCchCopyW(t, 4, L"hello");
    CHECK(hr == STRSAFE_E_INSUFFICIENT_BUFFER,
          "StringCchCopyW truncation -> INSUFFICIENT_BUFFER");
    CHECK(t[3] == L'\0' && wcscmp(t, L"hel") == 0,
          "StringCchCopyW truncation null-terminated (L'hel')");

    wchar_t z[4];
    CHECK(StringCchCopyW(z, 0, L"x") == STRSAFE_E_INVALID_PARAMETER,
          "StringCchCopyW cchDest==0 -> INVALID_PARAMETER");
  }

  // --- StringCchPrintfA (ANSI vsnprintf path) ---
  {
    char d[16];
    CHECK(StringCchPrintfA(d, 16, "%d-%d", 1, 2) == S_OK &&
              strcmp(d, "1-2") == 0,
          "StringCchPrintfA fit -> S_OK + content");

    char t[4];
    HRESULT hr = StringCchPrintfA(t, 4, "%d", 99999);
    CHECK(hr == STRSAFE_E_INSUFFICIENT_BUFFER,
          "StringCchPrintfA truncation -> INSUFFICIENT_BUFFER");
    CHECK(t[3] == '\0', "StringCchPrintfA truncation null-terminated");
  }

  // --- StringCchPrintfW (wide vswprintf path — the un-gated code) ---
  {
    wchar_t d[16];
    CHECK(StringCchPrintfW(d, 16, L"%d-%d", 1, 2) == S_OK &&
              wcscmp(d, L"1-2") == 0,
          "StringCchPrintfW fit -> S_OK + content");

    wchar_t t[4];
    HRESULT hr = StringCchPrintfW(t, 4, L"%d", 99999);
    CHECK(hr == STRSAFE_E_INSUFFICIENT_BUFFER,
          "StringCchPrintfW truncation -> INSUFFICIENT_BUFFER");
    CHECK(t[3] == L'\0', "StringCchPrintfW truncation null-terminated");
  }

  // --- trace.cpp end to end: DebugPrintfA/W -> StringCchVPrintfEx[A/W] -> sink
  // ---
  {
    DebugPrintfA("y=%d", 7);
    CHECK(strcmp(g_lastA, "y=7") == 0,
          "DebugPrintfA -> trace -> sink content 'y=7'");

    DebugPrintfW(L"x=%d", 42);
    CHECK(strcmp(g_lastW, "x=42") == 0,
          "DebugPrintfW -> wide trace -> sink content 'x=42'");
  }

  if (g_failures == 0) {
    fprintf(stderr, "ALL STRSAFE TESTS PASSED\n");
    return 0;
  }
  fprintf(stderr, "%d STRSAFE TEST(S) FAILED\n", g_failures);
  return 1;
}
