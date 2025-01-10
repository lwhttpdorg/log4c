#pragma once

#include <stdio.h>
#include "log4c_lock.h"

#define LOG_FILE_NAME_MAX 512

#define LOG_LINE_MAX 512

#define CONF_MAX_LINE_LENGTH 128

struct log4c {
	log4c_lock lock;
	char layout_pattern[CONF_MAX_LINE_LENGTH + 1];
	int console_apender;
	FILE *file_appender;
};

enum log_level {
	LOG_FATAL = 0, LOG_ERROR = 1, LOG_WARN = 2, LOG_INFO = 3, LOG_DEBUG = 4, LOG_TRACE = 5, LOG_MAX = 6
};

struct log4c *get_layout(const char *conf_file);

void set_layout_pattern(struct log4c *layout, const char *pattern);

void log_out(struct log4c *layout, enum log_level level, const char *fmt, ...);

void log_fatal(struct log4c *layout, const char *fmt, ...);

void log_error(struct log4c *layout, const char *fmt, ...);

void log_warn(struct log4c *layout, const char *fmt, ...);

void log_info(struct log4c *layout, const char *fmt, ...);

void log_debug(struct log4c *layout, const char *fmt, ...);

void log_trace(struct log4c *layout, const char *fmt, ...);
