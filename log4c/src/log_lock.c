#include "log_lock.h"

void log_lock_init(log_lock *lock) {
#ifdef _MSC_VER
	(void)InitializeCriticalSectionAndSpinCount(lock, 0x00000400);
#else
	pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
#endif
}

void log_lock_fini(log_lock *lock) {
#ifdef _MSC_VER
	DeleteCriticalSection(lock);
#else
	pthread_spin_destroy(lock);
#endif
}

void lock(log_lock *lock) {
#ifdef _MSC_VER
	EnterCriticalSection(lock);
#else
	pthread_spin_lock(lock);
#endif
}

void unlock(log_lock *lock) {
#ifdef _MSC_VER
	LeaveCriticalSection(lock);
#else
	pthread_spin_unlock(lock);
#endif
}
