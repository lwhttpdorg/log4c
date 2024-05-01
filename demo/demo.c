#include <pthread.h>

#include "log4c.h"

void *thread_routine(void *arg) {
	pthread_setname_np(pthread_self(), "child");
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	return NULL;
}

int main() {
	pthread_t id;
	pthread_create(&id, NULL, thread_routine, NULL);
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	pthread_join(id, NULL);
	return 0;
}
