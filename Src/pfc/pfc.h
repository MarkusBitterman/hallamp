#ifndef STRICT
#define STRICT
#endif

#ifdef _WIN32
#include <windows.h>
#define NOVTABLE _declspec(novtable)
#else
#define NOVTABLE
#endif

#define tabsize(x) (sizeof(x) / sizeof(*x))

#include "string.h"
#ifdef PFC_UNICODE
#include "string_unicode.h"
#endif
#include "cfg_var.h"
#include "critsec.h"
#include "grow_buf.h"
#include "mem_block.h"
#include "ptr_list.h"