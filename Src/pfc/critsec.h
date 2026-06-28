#ifndef _PFC_CRITSEC_H_
#define _PFC_CRITSEC_H_

#ifdef _WIN32

class critical_section : public CRITICAL_SECTION {
public:
  inline void enter() { EnterCriticalSection(this); }
  inline void leave() { LeaveCriticalSection(this); }
  critical_section() { InitializeCriticalSection(this); }
  ~critical_section() { DeleteCriticalSection(this); }
};

#else // POSIX

#include <pthread.h>

class critical_section {
  pthread_mutex_t m_mutex;

public:
  inline void enter() { pthread_mutex_lock(&m_mutex); }
  inline void leave() { pthread_mutex_unlock(&m_mutex); }
  critical_section() {
    // Windows CRITICAL_SECTION is re-entrant; a default pthread_mutex is not.
    // Use a recursive mutex so code that re-enters a held lock doesn't
    // deadlock. Matches in-tree precedent in Src/Wasabi/bfc/critsec.cpp.
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
  }
  ~critical_section() { pthread_mutex_destroy(&m_mutex); }
};

#endif // _WIN32

#endif // _PFC_CRITSEC_H_