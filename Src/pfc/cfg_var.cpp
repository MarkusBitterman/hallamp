#define STRICT
#ifdef _WIN32
#include <windows.h>
#else
#include "bfc/platform/platform.h"
#endif
#include "cfg_var.h"
#include "string_unicode.h"
#include <stdio.h>
#include <string.h>

static const char *m_inifile, *m_section;

/*
** Persistence on Windows is backed by the .ini "private profile" API (and a
** little registry). There is no portable equivalent, and the Linux config
** store is slated for a later redesign, so on non-Windows these read as
** "no stored value" (callers fall back to defaults) and writes are no-ops.
** The control flow below (config_read/write, cfg_int/cfg_string) is portable
** and drives these accessors unchanged on both platforms.
*/

int cfg_var::reg_read_int(HKEY hk, int def) {
#ifdef _WIN32
  return GetPrivateProfileIntA(m_section, var_get_name(), def, m_inifile);
#else
  return def;
#endif
}

void cfg_var::reg_write_int(HKEY hk, int val) {
#ifdef _WIN32
  /*	long temp=val;
          RegSetValueEx(hk,var_get_name(),0,REG_DWORD,(const BYTE*)&temp,4);*/
  char tmp[512] = {0};
  wsprintfA(tmp, "%d", val);
  WritePrivateProfileStringA(m_section, var_get_name(), tmp, m_inifile);
#endif
}

void cfg_var::reg_write_struct(HKEY hk, const void *ptr, UINT size) {
#ifdef _WIN32
  WritePrivateProfileStructA(m_section, var_get_name(), (void *)ptr, size,
                             m_inifile);
#endif
}

bool cfg_var::reg_read_struct(HKEY hk, void *ptr, UINT size) {
#ifdef _WIN32
  GetPrivateProfileStructA(m_section, var_get_name(), ptr, size, m_inifile);
  return 1;
#else
  return 0;
#endif
}

int cfg_var::reg_get_struct_size(HKEY hk) {
#ifdef _WIN32
  DWORD sz = 0, t = 0;
  if (RegQueryValueExA(hk, var_get_name(), 0, &t, 0, &sz) != ERROR_SUCCESS)
    return 0;
  return sz;
#else
  return 0;
#endif
}

bool string_a::reg_read(HKEY hk, const char *name) {
#ifdef _WIN32
  char tmp[4096] = {0};
  GetPrivateProfileStringA(m_section, name, "|||", tmp, sizeof(tmp) - 1,
                           m_inifile);
  if (strstr(tmp, "|||") == tmp)
    return 0;
  lstrcpyA(buffer_get(strlen(tmp) + 1), tmp);
  buffer_done();
  return 1;
#else
  return 0;
#endif
}

void string_a::reg_write(HKEY hk, const char *name) {
#ifdef _WIN32
  WritePrivateProfileStringA(m_section, name, (const char *)*this, m_inifile);
#endif
}

cfg_var *cfg_var::list = 0;

/*HKEY cfg_var::reg_open(const char * regname)
{
        HKEY hk;
        RegCreateKey(HKEY_CURRENT_USER,regname,&hk);
        return hk;
}*/

void cfg_var::config_read(const char *inifile, const char *section) {
  HKEY hk = 0; // reg_open(regname);
  m_inifile = inifile;
  m_section = section;
  cfg_var *ptr;
  for (ptr = list; ptr; ptr = ptr->next)
    ptr->read(hk);
  // RegCloseKey(hk);
}

void cfg_var::config_write(const char *inifile, const char *section) {
  HKEY hk = 0; // reg_open(regname);
  m_inifile = inifile;
  m_section = section;
  cfg_var *ptr;
  for (ptr = list; ptr; ptr = ptr->next)
    ptr->write(hk);
  // RegCloseKey(hk);
}

void cfg_var::config_reset() {
  cfg_var *ptr;
  for (ptr = list; ptr; ptr = ptr->next)
    ptr->reset();
}

void cfg_int::read(HKEY hk) { val = reg_read_int(hk, def); }

void cfg_int::write(HKEY hk) {
  if (val != reg_read_int(hk, def))
    reg_write_int(hk, val);
}

void cfg_string::read(HKEY hk) {
  string_a temp;
  if (temp.reg_read(hk, var_get_name()))
    val = temp;
}

void cfg_string::write(HKEY hk) {
  string_a temp = def;
  string_a name = var_get_name();

  if (!temp.reg_read(hk, name) || strcmp(val, temp))
    val.reg_write(hk, name);
}

#ifdef PFC_UNICODE

void cfg_string_w::read(HKEY hk) {
  string_w temp;
  if (temp.reg_read(hk, string_w(var_get_name())))
    val = temp;
}

void cfg_string_w::write(HKEY hk) {
  string_w temp = def;
  string_w name = var_get_name();
  string_w val_w = val;

  if (!temp.reg_read(hk, name) || wcscmp(val_w, temp))
    val_w.reg_write(hk, name);
}

#endif
