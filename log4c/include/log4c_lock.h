#pragma once

#ifdef _MSC_VER
#include <windows.h>
#define  log4c_lock CRITICAL_SECTION
#else
#include <pthread.h>
#define log4c_lock pthread_spinlock_t
#endif

void log4c_lock_init(log4c_lock *lock);

void log4c_lock_fini(log4c_lock *lock);

void lock(log4c_lock *lock);

void unlock(log4c_lock *lock);
