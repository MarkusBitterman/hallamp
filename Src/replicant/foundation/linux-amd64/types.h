#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <wchar.h>

typedef unsigned int UINT;
typedef signed int SINT;
typedef unsigned char UCHAR;
typedef signed char SCHAR;

typedef uint32_t ARGB32;
typedef uint32_t RGB32;
typedef uint32_t ARGB24;
typedef uint32_t RGB24;
typedef uint16_t ARGB16;
typedef uint16_t RGB16;

typedef uint32_t FOURCC;

/* On Linux we use UTF-8 char strings throughout */
typedef char nsxml_char_t;
typedef char ns_char_t;
typedef char nsfilename_char_t;

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID;
#endif

#ifndef NULL
#define NULL 0
#endif
