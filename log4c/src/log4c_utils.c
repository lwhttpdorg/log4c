#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log4c_utils.h"

#include <ctype.h>

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
	int i = vsnprintf(buf, size, fmt, args);
	if (i > 0) {
		return i;
	}
	return 0;
}

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	if (i > 0) {
		return i;
	}
	return 0;
}

size_t str_find(const char *haystack, const char *needle) {
	size_t haystack_len = strlen(haystack);
	size_t needle_len = strlen(needle);
	if (needle_len > haystack_len) {
		return END_POS;
	}
	size_t haystack_pos = END_POS;
	for (size_t i = 0; i < haystack_len; i++) {
		if (0 == strncmp(haystack + i, needle, needle_len)) {
			haystack_pos = i;
			break;
		}
	}
	return haystack_pos;
}

const char *skip_whitespace(const char *str) {
	while (*str && isspace(*str)) {
		++str;
	}
	return str;
}

size_t replace(char *original, size_t length, const char *target, const char *replace) {
	size_t target_len = strlen(target);
	size_t replace_len = strlen(replace);
	size_t origin_len = strlen(original);
	if (target_len == 0 || origin_len == length) {
		return origin_len;
	}

	char *pos = strstr(original, target);
	if (NULL == pos) {
		return origin_len;
	}

	size_t new_len;
	// There is not enough size after the replacement position to accommodate the string to be replaced.
	if (pos + replace_len > original + length - 1) {
		size_t copy_len = original + length - pos - 1;
		memcpy(pos, replace, copy_len);
		original[length - 1] = '\0';
		new_len = length - 1;
	}
	else {
		// The lengths of target and replace are not equal, so the subsequent string needs to be moved
		if (replace_len != target_len) {
			size_t move_len;
			// If the subsequent length is insufficient, the string after target will be truncated
			if (origin_len + replace_len - target_len > length) {
				move_len = origin_len - (pos - original) - replace_len;
			}
			else {
				move_len = origin_len - (pos - original) - target_len;
			}
			memmove(pos + replace_len, pos + target_len, move_len);
			new_len = pos - original + replace_len + move_len;
			original[new_len] = '\0';
		}
		else {
			new_len = origin_len;
		}
		memcpy(pos, replace, replace_len);
	}
	return new_len;
}
