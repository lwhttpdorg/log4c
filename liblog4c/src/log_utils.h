#pragma once

#include <stdarg.h>
#include <stddef.h>

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);
