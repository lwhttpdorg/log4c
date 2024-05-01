#include <string.h>
#include <stdbool.h>

#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdio.h>

#endif

#include "log4c.h"
#include "log_utils.h"

static struct log4c t_log;

__attribute__((constructor))
static void lo4c_init() {
	log_lock_init(&t_log.lock);
	t_log.fd = -1;
}

__attribute__((destructor))
static void lo4c_fini() {
	log_lock_fini(&t_log.lock);
	if (t_log.fd != -1) {
		fsync(t_log.fd);
		close(t_log.fd);
	}
}

static int open_file(const char *file_name, bool append) {
	char *pos = strrchr(file_name, '/');
	if (pos != NULL) {
		size_t path_len = (size_t) pos - (size_t) file_name;
		char path[path_len + 1];
		strncpy(path, file_name, path_len);
		path[path_len] = '\0';
		if (0 != access(path, F_OK)) {
#if defined(_MSC_VER) || defined(_WIN32)
			(void)_mkdir(path.c_str());
#endif
#if defined(__linux__)
			mkdir(path, 0755);
#endif
		}
	}
	int oflag = O_RDWR | O_CREAT;
	if (append) {
		oflag |= O_APPEND;
	}
#if defined(_MSC_VER) || defined(_WIN32)
	int mode = _S_IREAD|_S_IWRITE;
#endif

#ifdef __linux__
	oflag |= O_CLOEXEC;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
	int fd = open(file_name, oflag, mode);
	if (fd ==-1) {
		perror("can not open the log file\n");
	}
	return fd;
}

struct log4c *get_log(const char *log_file, bool append) {
	if (t_log.fd == -1) {
		log_lock(&t_log.lock);
		if (t_log.fd == -1) {
			t_log.fd = open_file(log_file, append);
		}
		log_unlock(&t_log.lock);
		return t_log.fd == -1 ? NULL : &t_log;
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
	(void) write(log->fd, buffer, used_len);
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
	(void) write(log->fd, buffer, used_len);
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