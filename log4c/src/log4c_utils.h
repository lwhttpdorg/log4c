#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * format a string
 * @param buf: the buffer to store the formatted string
 * @param size: the size of the buffer
 * @param fmt: the format string
 * @param args: the arguments
 * @return: the length of the formatted string
 */
size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

/**
 * format to a string
 * @param buf: the buffer to store the formatted string
 * @param size: the size of the buffer
 * @param fmt: the format string
 * @param ...: arguments
 * @return the length of the formatted string
 */
size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);

#define END_POS ((size_t)-1)

size_t str_find(const char *haystack, const char *needle);

const char *skip_whitespace(const char *str);

size_t replace(char *original, size_t length, const char *target, const char *replace);
