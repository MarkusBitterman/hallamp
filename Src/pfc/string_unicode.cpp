#define STRICT
#ifdef _WIN32
#include <windows.h>
#else
#include "bfc/platform/platform.h"
#endif
#include "string_unicode.h"
#include <stdio.h>
#include <string.h>

// WCHAR is 16-bit (UTF-16) on every platform (see bfc/platform/platform.h),
// which differs from Linux wchar_t (32-bit). The libc wide-string routines
// (wcslen/wcscmp/vswprintf) therefore cannot be used on WCHAR here; the
// portable codecs below operate on UTF-16 directly.
static int utf82wide(int *wide, const BYTE *utf8);
static int wide2utf8(char *target, int wide, int max);

#ifdef _WIN32
static bool is_nt() {
  OSVERSIONINFO os;
  memset(&os, 0, sizeof(os));
  os.dwOSVersionInfoSize = sizeof(os);
  GetVersionEx(&os);
  return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}
#endif

#ifdef _WIN32
void string_w::s_GetWindowText(HWND w) {
  if (!is_nt()) {
    set_string_a(string_a(w));
  } else {
    reset();
    int len = GetWindowTextLengthW(w) + 1;
    GetWindowTextW(w, string_buffer_w(*this, len), len);
  }
}
#else
// Win32 GUI accessors; no native equivalent until the Qt6 GUI port (Phase 4).
void string_w::s_GetWindowText(HWND) { reset(); }
#endif

void string_w::set_string_a(const char *c) {
#ifdef _WIN32
  int len = (int)strlen(c) + 1;
  MultiByteToWideChar(CP_ACP, 0, c, -1, string_buffer_w(*this, len), len);
#else
  // Treat narrow input as UTF-8 (the Linux convention) and decode to UTF-16.
  reset();
  const BYTE *src = (const BYTE *)c;
  while (src && *src) {
    int cur = 0;
    int t = utf82wide(&cur, src);
    if (!t || !cur || cur > 0xFFFF)
      break;
    src += t;
    add_char((WCHAR)cur);
  }
#endif
}

void string_w::add_string_a(const char *c) { add_string(string_w(c)); }

#ifdef _WIN32
void string_w::s_SetWindowText(HWND w) {
  if (!is_nt()) {
    string_a(*this).s_SetWindowText(w);
  } else {
    SetWindowTextW(w, *this);
  }
}
#else
void string_w::s_SetWindowText(HWND) {}
#endif

void string_a::set_string_w(const WCHAR *c) {
#ifdef _WIN32
  int len = ((int)wcslen(c) + 1) * 2;
  WideCharToMultiByte(CP_ACP, 0, c, -1, string_buffer_a(*this, len), len, 0, 0);
#else
  // Encode UTF-16 to UTF-8. WCHAR is 16-bit so wcslen/WideCharToMultiByte
  // are unavailable; walk the units directly.
  reset();
  char temp[8] = {0};
  while (c && *c) {
    int n = wide2utf8(temp, (int)(unsigned short)*c, 7);
    if (!n)
      break;
    temp[n] = 0;
    add_string(temp);
    c++;
  }
#endif
}

void string_a::add_string_w(const WCHAR *c) { add_string(string_a(c)); }

// utf8 stuff

static const BYTE mask_tab[6] = {0x80, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};

static const BYTE val_tab[6] = {0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

static int utf82wide(int *wide, const BYTE *utf8) {
  int res = 0;
  int n;
  int cnt = 0;
  while (1) {
    if ((*utf8 & mask_tab[cnt]) == val_tab[cnt])
      break;
    if (++cnt == 6)
      return 0;
  }
  cnt++;

  if (cnt == 2 && !(*utf8 & 0x1E))
    return 0;

  if (cnt == 1)
    res = *utf8;
  else
    res = (0xFF >> (cnt + 1)) & *utf8;

  for (n = 1; n < cnt; n++) {
    if ((utf8[n] & 0xC0) != 0x80)
      return 0;
    if (!res && n == 2 && !((utf8[n] & 0x7F) >> (7 - cnt)))
      return 0;

    res = (res << 6) | (utf8[n] & 0x3F);
  }

  if (wide)
    *wide = res;

  return cnt;
}

static int wide2utf8(char *target, int wide, int max) {
  int count;

  if (wide < 0x80)
    count = 1;
  else if (wide < 0x800)
    count = 2;
  else if (wide < 0x10000)
    count = 3;
  else if (wide < 0x200000)
    count = 4;
  else if (wide < 0x4000000)
    count = 5;
  else if (wide <= 0x7FFFFFFF)
    count = 6;
  else
    return 0;
  if (count > max)
    return 0;

  if (target == 0)
    return count;

  switch (count) {
  case 6:
    target[5] = 0x80 | (wide & 0x3F);
    wide = wide >> 6;
    wide |= 0x4000000;
  case 5:
    target[4] = 0x80 | (wide & 0x3F);
    wide = wide >> 6;
    wide |= 0x200000;
  case 4:
    target[3] = 0x80 | (wide & 0x3F);
    wide = wide >> 6;
    wide |= 0x10000;
  case 3:
    target[2] = 0x80 | (wide & 0x3F);
    wide = wide >> 6;
    wide |= 0x800;
  case 2:
    target[1] = 0x80 | (wide & 0x3F);
    wide = wide >> 6;
    wide |= 0xC0;
  case 1:
    target[0] = wide;
  }

  return count;
}

void string_w::add_string_utf8(const char *z) {
  string_w temp;
  temp.set_string_utf8(z);
  add_string(temp);
}

void string_w::set_string_utf8(const char *src)
// static int UTF8ToWide(const char* src,WCHAR* dst,int len,int out_len)
{
  int cur_wchar;
  for (;;) {
    cur_wchar = 0;
    int t = utf82wide(&cur_wchar, (const BYTE *)src);
    if (!t || !cur_wchar || cur_wchar > 0xFFFF)
      break;
    src += t;
    add_char((WCHAR)cur_wchar);
  }
}

string_printf_a::string_printf_a(const char *fmt, ...) {
  va_list list;
  va_start(list, fmt);
  vsprintf(string_buffer_a(*this, 1024), fmt, list);
  va_end(list);
}

string_printf_w::string_printf_w(const WCHAR *fmt, ...) {
#ifdef _WIN32
  va_list list;
  va_start(list, fmt);
  vswprintf(string_buffer_w(*this, 1024), 1024, fmt, list);
  va_end(list);
#else
  // vswprintf works on wchar_t (32-bit on Linux); WCHAR is 16-bit (UTF-16),
  // so libc's wide printf is unusable here. No Linux consumer yet; left empty
  // until a UTF-16 formatter is actually needed.
  (void)fmt;
#endif
}

string_a::string_a(const string_w &z) { add_string_w(z); }

void string_utf8::convert(const WCHAR *src) {
  char temp[8] = {0};
  while (src && *src) {
    int len = wide2utf8(temp, (int)(unsigned short)*src, 7);
    if (!len)
      break;
    temp[len] = 0;
    add_string(temp);
    src++;
  }
}

bool string_w::reg_read(HKEY hk, const WCHAR *name) {
#ifdef _WIN32
  if (!is_nt()) {
    string_a temp(*this);
    if (temp.reg_read(hk, string_a(name))) {
      set_string_a(temp);
      return 1;
    } else
      return 0;
  } else {
    DWORD sz = 0, t = 0;
    if (RegQueryValueExW(hk, name, 0, &t, 0, &sz) != ERROR_SUCCESS)
      return 0;
    if (sz == 0 || t != REG_SZ)
      return 0;
    RegQueryValueExW(hk, name, 0, 0, (BYTE *)buffer_get(sz >> 1), &sz);
    buffer_done();
    return 1;
  }
#else
  return 0;
#endif
}

void string_w::reg_write(HKEY hk, const WCHAR *name) {
#ifdef _WIN32
  if (!is_nt()) {
    string_a(*this).reg_write(hk, string_a(name));
  } else {
    RegSetValueExW(hk, name, 0, REG_SZ, (const BYTE *)(const WCHAR *)*this,
                   ((DWORD)length() + 1) * 2);
  }
#endif
}

bool string_w::test_os() {
#ifdef _WIN32
  return is_nt();
#else
  return true;
#endif
}
