#pragma once

#include <stdio.h>
#include "log4c_lock.h"

#define LOG_FILE_NAME_MAX 512

#define LOG_LINE_MAX 512

struct log4c {
	log4c_lock lock;
	FILE *file;
};

#ifdef _MSC_VER
#ifdef ERROR
#undef ERROR
#endif
#endif

enum log_level {
	FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
};

struct log4c *get_log(const char *log_file, bool append);

void log_out(struct log4c *log, enum log_level level, const char *fmt, ...);

void log_fatal(struct log4c *log, const char *fmt, ...);

void log_error(struct log4c *log, const char *fmt, ...);

void log_warn(struct log4c *log, const char *fmt, ...);

void log_info(struct log4c *log, const char *fmt, ...);

void log_debug(struct log4c *log, const char *fmt, ...);

void log_trace(struct log4c *log, const char *fmt, ...);
