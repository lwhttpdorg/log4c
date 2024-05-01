#pragma once

#include <stdbool.h>

#ifdef _MSC_VER

#include <windows.h>
#include <synchapi.h>

#define  log4c_lock CRITICAL_SECTION
#else

#include <pthread.h>

#define log4c_lock pthread_spinlock_t

#endif

void log_lock_init(log4c_lock *lock);

void log_lock_fini(log4c_lock *lock);

void log_lock(log4c_lock *lock);

void log_unlock(log4c_lock *lock);
