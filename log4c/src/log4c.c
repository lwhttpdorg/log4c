#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_MSC_VER)

#include <direct.h>
#include <io.h>
#include <windows.h>

#define F_OK 0

#endif

#ifdef __linux__

#include <unistd.h>
#include <sys/stat.h>

#endif

#include <stdbool.h>

#include "log4c.h"
#include "layout_pattern.h"
#include "log4c_lock.h"
#include "log4c_utils.h"

#ifdef _MSC_VER

#define STDOUT_FILENO _fileno(stdout)
#define STDERR_FILENO _fileno(stderr)

#endif

#define DEFAULT_LAYOUT_PATTERN "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}${ms} [${TH}] [${L}] -- ${W}"

static struct log4c *layout_ptr = NULL;
static struct log4c layout_instance;

static void layout_initialize() {
	log4c_lock_init(&layout_instance.lock);
	strcpy(layout_instance.layout_pattern, DEFAULT_LAYOUT_PATTERN);
	layout_instance.console_apender = -1;
	layout_instance.file_appender = NULL;
}

static void layout_finalize() {
	log4c_lock_fini(&layout_instance.lock);
	if (layout_instance.file_appender != NULL) {
		fflush(layout_instance.file_appender);
		fclose(layout_instance.file_appender);
	}
}

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
	layout_finalize();
}

INITIALIZER(initialize) {
	layout_initialize();
	atexit(lo4c_finalize);
}

#endif

#ifdef __GNUC__

__attribute__((constructor))
static void lo4c_initialize() {
	layout_initialize();
}

__attribute__((destructor))
static void lo4c_finalize() {
	layout_finalize();
}

#endif

static FILE *open_log(const char *log_file) {
	if (strlen(log_file) > LOG_FILE_NAME_MAX) {
#ifdef _WIN32
		fprintf(stderr, "the length of log file name over limit, %llu > %d", strlen(log_file), LOG_FILE_NAME_MAX);
#else
		fprintf(stderr, "the length of log file name over limit, %lu > %d", strlen(log_file), LOG_FILE_NAME_MAX);
#endif
		return NULL;
	}
	char *pos = strrchr(log_file, '/');
	if (pos != NULL) {
		size_t path_len = (size_t) pos - (size_t) log_file;
		char path[LOG_FILE_NAME_MAX + 1];
		strncpy(path, log_file, path_len);
		path[path_len] = '\0';
#ifdef _WIN32
		if (0 != _access(path, F_OK))
#else
		if (0 != access(path, F_OK))
#endif
		{
#if defined(_MSC_VER) || defined(_WIN32)
			(void)_mkdir(path);
#endif
#if defined(__linux__)
			mkdir(path, 0755);
#endif
		}
	}
	FILE *file = fopen(log_file, "a");
	if (file == NULL) {
		perror("can not open the log file");
	}
	return file;
}

#define CONF_KEY_LAYOUT_PATTERN "layout_pattern"
#define CONF_KEY_CONSOLE_APENDER "console_appender"
#define CONF_KEY_FILE_APPENDER "file_appender"
#define STDOUT_APPENDER "stdout"
#define STDERR_APPENDER "stderr"

#define CONF_VALUE_MIN_LENGTH 1UL

bool get_value(const char *line, const char *key, char *value) {
	size_t pos = str_find(line, key);
	if (END_POS == pos) {
		value[0] = '\0';
		return false;
	}
	pos += strlen(key);
	const char *value_start = skip_whitespace(line + pos);
	// Skip the leading double quotes
	if ('\"' == *value_start) {
		++value_start;
	}
	const char *value_end = strchr(value_start, ';');
	size_t value_len;
	if (value_end != NULL) {
		value_len = value_end - value_start;
	}
	else {
		fprintf(stderr, "warning: missing trailing semicolon ';' '%s'\n", line);
		value_len = strlen(value_start);
	}
	if (CONF_VALUE_MIN_LENGTH > value_len) {
		fprintf(stderr, "error: malformed value length for key '%s'\n", key);
		return false;
	}
	if (CONF_MAX_LINE_LENGTH < value_len) {
		value_len = CONF_MAX_LINE_LENGTH;
	}
	// Skip the trailing double quotes
	if ('\"' == value_start[value_len - 1]) {
		--value_len;
	}
	strncpy(value, value_start, value_len);
	value[value_len] = '\0';
	return true;
}

void parse_line(const char *line) {
	char value[CONF_MAX_LINE_LENGTH + 1];
	if (get_value(line, CONF_KEY_LAYOUT_PATTERN, value)) {
		strncpy(layout_instance.layout_pattern, value, strlen(value));
		return;
	}
	if (get_value(line, CONF_KEY_CONSOLE_APENDER, value)) {
		if (0 == strncmp(value, STDOUT_APPENDER, strlen(STDOUT_APPENDER))) {
			layout_instance.console_apender = STDOUT_FILENO;
		}
		else if (0 == strncmp(value, STDERR_APPENDER, strlen(STDERR_APPENDER))) {
			layout_instance.console_apender = STDERR_FILENO;
		}
		return;
	}
	if (get_value(line, CONF_KEY_FILE_APPENDER, value)) {
		layout_instance.file_appender = open_log(value);
		return;
	}
}

void read_conf(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Failed to open configuration file");
		return;
	}
	char line[CONF_MAX_LINE_LENGTH];
	while (fgets(line, sizeof(line), file)) {
		parse_line(line);
	}
	fclose(file);
}

const char *BANNER = "   __    ___   ___  _  _      ___\n"
                     "  / /   /___\\ / _ \\| || |    / __\\\n"
                     " / /   //  /// /_\\/| || |_  / /\n"
                     "/ /___/ \\_/// /_\\\\ |__   _|/ /___\n"
                     "\\____/\\___/ \\____/    |_|  \\____/\n";

struct log4c *get_layout(const char *conf_file) {
	if (NULL == conf_file) {
		return layout_ptr;
	}
	if (NULL == layout_ptr) {
		lock(&layout_instance.lock);
		if (NULL == layout_ptr) {
			fprintf(stdout, "%s\n", BANNER);
			fflush(stdout);
			read_conf(conf_file);
			layout_ptr = &layout_instance;
		}
		unlock(&layout_instance.lock);
	}
	return layout_ptr;
}

void log_out_va(struct log4c *layout, enum log_level level, const char *fmt, va_list args) {
	char buffer[LOG_LINE_MAX];
	size_t buf_len = sizeof(buffer);
	buffer[0] = '\0';
	size_t used_len = vformat(layout->layout_pattern, buffer, buf_len, level, fmt, args);
	lock(&layout->lock);
	if (layout->console_apender != -1) {
#ifdef _MSC_VER
		(void)_write(layout->console_apender, buffer, (unsigned int)used_len);
#else
		(void) write(layout->console_apender, buffer, used_len);
#endif
	}
	if (layout->file_appender != NULL) {
		(void) fwrite(buffer, 1, used_len, layout->file_appender);
		fflush(layout->file_appender);
	}
	unlock(&layout->lock);
}

void log_out(struct log4c *layout, enum log_level level, const char *fmt, ...) {
	char buffer[LOG_LINE_MAX];
	size_t buf_len = sizeof(buffer);
	buffer[0] = '\0';
	va_list args;
	va_start(args, fmt);
	size_t used_len = vformat(layout->layout_pattern, buffer, buf_len, level, fmt, args);
	va_end(args);
	lock(&layout->lock);
	if (layout->console_apender != -1) {
#ifdef _MSC_VER
		(void)_write(layout->console_apender, buffer, (unsigned int)used_len);
#else
		(void) write(layout->console_apender, buffer, used_len);
#endif
	}
	if (layout->file_appender != NULL) {
		(void) fwrite(buffer, 1, used_len, layout->file_appender);
		fflush(layout->file_appender);
	}
	unlock(&layout->lock);
}

void log_fatal(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_FATAL, fmt, args);
	va_end(args);
}

void log_error(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_ERROR, fmt, args);
	va_end(args);
}

void log_warn(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_WARN, fmt, args);
	va_end(args);
}

void log_info(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_INFO, fmt, args);
	va_end(args);
}

void log_debug(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_DEBUG, fmt, args);
	va_end(args);
}

void log_trace(struct log4c *layout, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_out_va(layout, LOG_TRACE, fmt, args);
	va_end(args);
}
