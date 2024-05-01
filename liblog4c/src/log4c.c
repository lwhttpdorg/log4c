#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#if defined(_WIN32) || defined(_MSC_VER)

#include <windows.h>
#include <direct.h>
#include <io.h>
#include <processthreadsapi.h>

#define F_OK 0

#ifdef ERROR
#undef ERROR
#endif

#endif

#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#endif

#include "log4c.h"
#include "log_utils.h"

static struct log4c t_log;

#if defined(_MSC_VER)
#pragma section(".CRT$XCU", read)
#define INITIALIZER2_(f, p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f,"")
#else
#define INITIALIZER(f) INITIALIZER2_(f,"_")
#endif

static void lo4c_finalize(void) {
	log_lock_fini(&t_log.lock);
	if (t_log.file != NULL) {
		fflush(t_log.file);
		fclose(t_log.file);
	}
}

INITIALIZER(initialize) {
	log_lock_init(&t_log.lock);
	t_log.file = NULL;
	atexit(lo4c_finalize);
}

#endif

#ifdef __linux__

__attribute__((constructor))
static void lo4c_initialize() {
	log_lock_init(&t_log.lock);
	t_log.file = NULL;
}

__attribute__((destructor))
static void lo4c_finalize() {
	log_lock_fini(&t_log.lock);
	if (t_log.file != NULL) {
		fflush(t_log.file);
		fclose(t_log.file);
	}
}
#endif

static FILE *open_file(const char *file_name, bool append) {
	if (strlen(file_name) > LOG_FILE_NAME_MAX) {
		fprintf(stderr, "the length of log file name over limit, %llu > %d", strlen(file_name), LOG_FILE_NAME_MAX);
		return NULL;
	}
	char *pos = strrchr(file_name, '/');
	if (pos != NULL) {
		size_t path_len = (size_t)pos - (size_t)file_name;
		char path[LOG_FILE_NAME_MAX + 1];
		strncpy(path, file_name, path_len);
		path[path_len] = '\0';
		if (0 != access(path, F_OK)) {
#if defined(_MSC_VER) || defined(_WIN32)
			(void)_mkdir(path);
#endif
#if defined(__linux__)
			mkdir(path, 0755);
#endif
		}
	}
	const char *mode = "w";
	if (append) {
		mode = "a";
	}
	FILE *file = fopen(file_name, mode);
	if (file == NULL) {
		perror("can not open the log file");
	}
	return file;
}

struct log4c *get_log(const char *log_file, bool append) {
	if (t_log.file == NULL) {
		log_lock(&t_log.lock);
		if (t_log.file == NULL) {
			t_log.file = open_file(log_file, append);
		}
		log_unlock(&t_log.lock);
		return t_log.file == NULL ? NULL : &t_log;
	}
	else {
		return &t_log;
	}
}

const char *LOG_LEVEL_FATAL = "FATAL";
const char *LOG_LEVEL_ERROR = "ERROR";
const char *LOG_LEVEL_WARN = "WARN";
const char *LOG_LEVEL_INFO = "INFO";
const char *LOG_LEVEL_DEBUG = "DEBUG";
const char *LOG_LEVEL_TRACE = "TRACE";

const char *log_level_str(enum log_level level) {
	const char *str = NULL;
	switch (level) {
		case FATAL:
			str = LOG_LEVEL_FATAL;
			break;
		case ERROR:
			str = LOG_LEVEL_ERROR;
			break;
		case WARN:
			str = LOG_LEVEL_WARN;
			break;
		case INFO:
			str = LOG_LEVEL_INFO;
			break;
		case DEBUG:
			str = LOG_LEVEL_DEBUG;
			break;
		case TRACE:
			str = LOG_LEVEL_TRACE;
			break;
	}
	return str;
}

#define THREAD_NAME_MAX_LEN  16

static size_t build_prefix(enum log_level level, char *buf, size_t len) {
	size_t used_len = 0;
	time_t tm_now = time(NULL);
	struct tm *local = localtime(&tm_now);
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "%04d-%02d-%02d %02d:%02d:%02d ", 1900 + local->tm_year,
	                            local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	char thread_name[THREAD_NAME_MAX_LEN];
	thread_name[0] = '\0';
#if defined(_MSC_VER) || defined(_WIN32)
	unsigned long tid = GetCurrentThreadId();
#endif
#ifdef __linux__
	pid_t tid = gettid();
#endif

#ifdef _PTHREAD_H
	pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
#elif __linux__
	prctl(PR_GET_NAME, reinterpret_cast<unsigned long>( thread_name));
#endif
	if (thread_name[0] != '\0') {
		used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%*.*s] ", THREAD_NAME_MAX_LEN / 2,
		                            THREAD_NAME_MAX_LEN / 2, thread_name);
	}
	else {
		used_len += log4c_scnprintf(buf + used_len, len - used_len, "[T%05u] ", tid);
	}
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%-5s] - ", log_level_str(level));
	return used_len;
}

void log_out_va(struct log4c *log, enum log_level level, const char *fmt, va_list args) {
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += build_prefix(level, buffer, buf_len);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	log_lock(&log->lock);
	(void)fwrite(buffer, 1, used_len, log->file);
	fflush(log->file);
	log_unlock(&log->lock);
}

void log_out(struct log4c *log, enum log_level level, const char *fmt, ...) {
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += build_prefix(level, buffer, buf_len);
	va_list args;
			va_start(args, fmt);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
			va_end(args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	log_lock(&log->lock);
	(void)fwrite(buffer, 1, used_len, log->file);
	fflush(log->file);
	log_unlock(&log->lock);
}

void log_fatal(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, FATAL, fmt, args);
			va_end(args);
}

void log_error(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, ERROR, fmt, args);
			va_end(args);
}

void log_warn(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, WARN, fmt, args);
			va_end(args);
}

void log_info(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, INFO, fmt, args);
			va_end(args);
}

void log_debug(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, DEBUG, fmt, args);
			va_end(args);
}

void log_trace(struct log4c *log, const char *fmt, ...) {
	va_list args;
			va_start(args, fmt);
	log_out_va(log, TRACE, fmt, args);
			va_end(args);
}