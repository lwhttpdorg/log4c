#include "log4c.h"

#ifdef _MSC_VER

#include <windows.h>

DWORD WINAPI thread_routine(LPVOID lpParam) {
	(void)lpParam;
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	return 0;
}

#endif

#ifdef __linux__

#include <pthread.h>

void *thread_routine(void *arg) {
	pthread_setname_np(pthread_self(), "child");
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	return NULL;
}

#endif

int main() {
#ifdef _MSC_VER
	DWORD thread_id;
	HANDLE handle = CreateThread(NULL, 0, thread_routine, NULL, 0, &thread_id);
#endif
#ifdef __linux__
	pthread_t id;
	pthread_create(&id, NULL, thread_routine, NULL);
#endif
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
#ifdef _MSC_VER
	WaitForSingleObject(handle, INFINITE);
#endif
#ifdef __linux__
	pthread_join(id, NULL);
#endif
	return 0;
}
