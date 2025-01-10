#pragma once

#include <stdarg.h>

#include "log4c.h"

size_t vformat(const char *pattern, char *__restrict buf, size_t buf_len, enum log_level level, const char *fmt,
				va_list args);

size_t format(const char *pattern, char *__restrict buf, size_t buf_len, enum log_level level, const char *fmt, ...);

struct daytime {
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned short millisecond;
};

void get_time_now(struct daytime *now);

unsigned long get_thread_id();
