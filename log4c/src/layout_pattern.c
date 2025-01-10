#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/time.h>
#endif

#include "layout_pattern.h"
#include "log4c.h"
#include "log4c_utils.h"

const char *MONTH_ABBR_NAME[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char *LOG_LEVEL_NAMES[LOG_MAX] = {
	[LOG_FATAL] = "FATAL",
	[LOG_ERROR] = "ERROR",
	[LOG_WARN] = "WARN ",
	[LOG_INFO] = "INFO ",
	[LOG_DEBUG] = "DEBUG",
	[LOG_TRACE] = "TRACE",
};

/* A two digit representation of a year. e.g. 99 or 03 */
const char *SHORT_YEAR = "${yy}";
/* A full numeric representation of a year, at least 4 digits, with - for years BCE. e.g. -0055, 0787, 1999, 2003, 10191 */
const char *FULL_YEAR = "${yyyy}";
/* Numeric representation of a month, without leading zeros. 1 through 12 */
const char *SHORT_MONTH = "${M}";
/* Numeric representation of a month, with leading zeros. 01 through 12 */
const char *FULL_MONTH = "${MM}";
/* A short textual representation of a month, three letters. Jan through Dec */
const char *ABBR_MONTH = "${MMM}";
/* Day of the month without leading zeros. 1 to 31 */
const char *SHORT_DAY = "${d}";
/* Day of the month, 2 digits with leading zeros. 01 to 31 */
const char *FULL_DAY = "${dd}";
/* 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. e.g. AM 01 or PM 11 */
const char *SHORT_HOUR = "${h}";
/* 24-hour format of an hour with leading zeros. 00 through 23 */
const char *FULL_HOUR = "${hh}";
/* Minutes without leading zeros. 1 to 59 */
const char *SHORT_MINUTES = "${m}";
/* Minutes with leading zeros. 01 to 59 */
const char *FULL_MINUTES = "${mm}";
/* Seconds without leading zeros. 1 to 59 */
const char *SHORT_SECOND = "${s}";
/* Seconds with leading zeros. 01 to 59 */
const char *FULL_SECOND = "${ss}";
/* Milliseconds with leading zeros. 001 to 999 */
const char *MILLISECOND = "${ms}";
/* An 8 digit representation of thread ID, with leading zeros. e.g. T01234567*/
const char *THREAD_ID = "${TH}";
/* Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE */
const char *LOG_LEVEL = "${L}";
/* Log message, e.g.: hello world! */
const char *LOG_MESSAGE = "${W}";

void get_time_now(struct daytime *now) {
#ifdef _WIN32
	SYSTEMTIME system_time;
	GetLocalTime(&system_time);
	now->year = system_time.wYear;
	now->month = (unsigned char)system_time.wMonth;
	now->day = (unsigned char)system_time.wDay;
	now->hour = (unsigned char)system_time.wHour;
	now->minute = (unsigned char)system_time.wMinute;
	now->second = (unsigned char)system_time.wSecond;
	now->millisecond = system_time.wMilliseconds;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	const time_t tm_now = tv.tv_sec;
	struct tm tm_info;
	localtime_r(&tm_now, &tm_info);
	now->year = tm_info.tm_year + 1900;
	now->month = tm_info.tm_mon + 1;
	now->day = tm_info.tm_mday;
	now->hour = tm_info.tm_hour;
	now->minute = tm_info.tm_min;
	now->second = tm_info.tm_sec;
	now->millisecond = tv.tv_usec / 1000;
#endif
}

unsigned long get_thread_id() {
#ifdef _WIN32
	unsigned long tid = GetCurrentThreadId();
#endif
#ifdef __linux__
	unsigned long tid = pthread_self();
#endif
	return tid;
}

static size_t pattern_format(const char *pattern, char *buf, size_t len, enum log_level level, const char *message) {
	struct daytime now;
	get_time_now(&now);
	size_t pattern_len = strlen(pattern);
#ifdef _MSC_VER
	strncpy_s(buf, len, pattern, pattern_len);
#else
	strncpy(buf, pattern, pattern_len);
#endif
	buf[pattern_len] = '\0';
	if (END_POS != str_find(pattern, FULL_YEAR)) {
		char year[6];
		log4c_scnprintf(year, sizeof(year), "%04d", now.year);
		replace(buf, len, FULL_YEAR, year);
	}
	if (END_POS != str_find(pattern, SHORT_YEAR)) {
		char year[3];
		log4c_scnprintf(year, sizeof(year), "%02d", now.year);
		replace(buf, len, SHORT_YEAR, year);
	}
	if (END_POS != str_find(pattern, FULL_MONTH)) {
		char month[3];
		log4c_scnprintf(month, sizeof(month), "%02d", now.month);
		replace(buf, len, FULL_MONTH, month);
	}
	if (END_POS != str_find(pattern, SHORT_MONTH)) {
		char month[3];
		log4c_scnprintf(month, sizeof(month), "%d", now.month);
	}
	if (END_POS != str_find(pattern, ABBR_MONTH)) {
		replace(buf, len, ABBR_MONTH, MONTH_ABBR_NAME[now.month]);
	}
	if (END_POS != str_find(pattern, SHORT_DAY)) {
		char day[3];
		log4c_scnprintf(day, sizeof(day), "%02d", now.day);
		replace(buf, len, SHORT_DAY, day);
	}
	if (END_POS != str_find(pattern, FULL_DAY)) {
		char day[3];
		log4c_scnprintf(day, sizeof(day), "%02d", now.day);
		replace(buf, len, FULL_DAY, day);
	}
	if (END_POS != str_find(pattern, SHORT_HOUR)) {
		char hour[3];
		log4c_scnprintf(hour, sizeof(hour), "%d", now.hour);
		replace(buf, len, SHORT_HOUR, hour);
	}
	if (END_POS != str_find(pattern, FULL_HOUR)) {
		char hour[3];
		log4c_scnprintf(hour, sizeof(hour), "%02d", now.hour);
		replace(buf, len, FULL_HOUR, hour);
	}
	if (END_POS != str_find(pattern, SHORT_MINUTES)) {
		char minute[3];
		log4c_scnprintf(minute, sizeof(minute), "%d", now.minute);
		replace(buf, len, SHORT_MINUTES, minute);
	}
	if (END_POS != str_find(pattern, FULL_MINUTES)) {
		char minute[3];
		log4c_scnprintf(minute, sizeof(minute), "%02d", now.minute);
		replace(buf, len, FULL_MINUTES, minute);
	}
	if (END_POS != str_find(pattern, SHORT_SECOND)) {
		char second[3];
		log4c_scnprintf(second, sizeof(second), "%d", now.second);
		replace(buf, len, SHORT_SECOND, second);
	}
	if (END_POS != str_find(pattern, FULL_SECOND)) {
		char second[3];
		log4c_scnprintf(second, sizeof(second), "%02d", now.second);
		replace(buf, len, FULL_SECOND, second);
	}
	if (END_POS != str_find(pattern, MILLISECOND)) {
		char millisecond[4];
		log4c_scnprintf(millisecond, sizeof(millisecond), "%03d", now.millisecond);
		replace(buf, len, MILLISECOND, millisecond);
	}
	if (END_POS != str_find(pattern, THREAD_ID)) {
		char thread_id[10];
		unsigned long tid = get_thread_id();
		log4c_scnprintf(thread_id, sizeof(thread_id), "T%08u ", tid);
		replace(buf, len, THREAD_ID, thread_id);
	}
	if (END_POS != str_find(pattern, LOG_LEVEL)) {
		replace(buf, len, LOG_LEVEL, LOG_LEVEL_NAMES[level]);
	}
	if (END_POS != str_find(pattern, LOG_MESSAGE)) {
		replace(buf, len, LOG_MESSAGE, message);
	}
	return strlen(buf);
}

size_t vformat(const char *pattern, char *__restrict buf, size_t buf_len, enum log_level level,
				const char *fmt, va_list args) {
	char message[LOG_LINE_MAX];
	message[0] = '\0';
	log4c_scnprintf(message, sizeof(message), fmt, args);
	size_t len = pattern_format(pattern, buf, buf_len, level, message);
	len += log4c_scnprintf(buf + len, buf_len - len, "\n");
	return len;
}

size_t format(const char *pattern, char *__restrict buf, size_t buf_len, enum log_level level,
			const char *fmt, ...) {
	char message[LOG_LINE_MAX];
	message[0] = '\0';
	va_list args;
	va_start(args, fmt);
	log4c_scnprintf(message, sizeof(message), fmt, args);
	va_end(args);
	size_t len = pattern_format(pattern, buf, buf_len, level, message);
	len += log4c_scnprintf(buf + len, buf_len - len, "\n");
	return len;
}
