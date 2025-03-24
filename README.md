# log4c

## CMakeLists.xtx

```cmake
include(FetchContent)
FetchContent_Declare(log4c GIT_REPOSITORY https://github.com/lwhttpdorg/log4c.git GIT_TAG v0.0.1)
FetchContent_MakeAvailable(log4c)
target_link_libraries(${YOUR_TARGET_NAME} log4c)
```

## Configuration

```ini
layout_pattern "${yyyy}-${MM}-${dd} ${hh} : ${mm}:${ss}:${ms} [${TH}] [${L}] -- ${W}";
console_appender stdout;
file_appender "log/log4c.log";
```

## Example

```c
#include "log4c.h"

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __linux__
#include <pthread.h>
#endif

void layout_test() {
	struct log4c *log = get_layout(NULL);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, LOG_ERROR, "this is a trace log...");
}

#ifdef __GNUC__

void *thread_routine(void *arg) {
	layout_test();
	return NULL;
}

#endif

#ifdef _MSC_VER

DWORD WINAPI thread_routine(LPVOID lpParam) {
	(void)lpParam;
	layout_test();
	return 0;
}

#endif

int main() {
	log4c_init("log4c.conf");
#ifdef _MSC_VER
	DWORD thread_id;
	HANDLE handle = CreateThread(NULL, 0, thread_routine, NULL, 0, &thread_id);
#endif
#ifdef __GNUC__
	pthread_t id;
	pthread_create(&id, NULL, thread_routine, NULL);
#endif
	struct log4c *log = get_layout();
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, LOG_ERROR, "this is a trace log...");
#ifdef _MSC_VER
	WaitForSingleObject(handle, INFINITE);
#endif
#ifdef __linux__
	pthread_join(id, NULL);
#endif
	return 0;
}
```