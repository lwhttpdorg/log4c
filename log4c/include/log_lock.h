#pragma once

#ifdef _MSC_VER
#include <windows.h>
#define  log_lock CRITICAL_SECTION
#else
#include <pthread.h>
#define log4c_lock pthread_spinlock_t
#endif

void log_lock_init(log_lock *lock);

void log_lock_fini(log_lock *lock);

void lock(log_lock *lock);

void unlock(log_lock *lock);
