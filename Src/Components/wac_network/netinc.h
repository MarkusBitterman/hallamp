/*
** JNetLib
** Copyright (C) 2000-2006 Nullsoft, Inc.
** Author: Justin Frankel
** File: netinc.h - network includes and portability defines (used internally)
** License: see jnetlib.h
*/

#ifndef NULLSOFT_WAC_NETWORK_NETINC_H
#define NULLSOFT_WAC_NETWORK_NETINC_H

#ifdef _WIN32

// #include <time.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <wspiapi.h>

#include "..\replicant\foundation\types.h"

#define strtoull(x, y, z) _strtoui64(x, y, z)
#define strcasecmp(x, y) _stricmp(x, y)
#define strcasecmpn(x, y, count) _strnicmp(x, y, count)
#define strncasecmp(x, y, count) _strnicmp(x, y, count)
#define HTONS(val)                                                             \
  ((((unsigned short)(val) & (unsigned short)0x00ffU) << 8) |                  \
   (((unsigned short)(val) & (unsigned short)0xff00U) >> 8))
#define ERRNO (WSAGetLastError())
#define SET_SOCK_BLOCK(s, block)                                               \
  {                                                                            \
    unsigned long __i = block ? 0 : 1;                                         \
    ioctlsocket(s, FIONBIO, &__i);                                             \
  }

#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEWOULDBLOCK

#else
#ifndef THREAD_SAFE
#define THREAD_SAFE
#endif
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ERRNO errno
#define closesocket(s) close(s)
#define SET_SOCK_BLOCK(s, block)                                               \
  {                                                                            \
    int __flags;                                                               \
    if ((__flags = fcntl(s, F_GETFL, 0)) != -1) {                              \
      if (!block)                                                              \
        __flags |= O_NONBLOCK;                                                 \
      else                                                                     \
        __flags &= ~O_NONBLOCK;                                                \
      fcntl(s, F_SETFL, __flags);                                              \
    }                                                                          \
  }

#define _stricmp(x, y) strcasecmp(x, y)
#define _strnicmp(x, y, z) strncasecmp(x, y, z)
#define _strdup(x) strdup(x)
#define wsprintf sprintf
typedef int SOCKET;

#endif // !_WIN32

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#endif //! NULLSOFT_WAC_NETWORK_NETINC_H
