
#include "gtest/gtest.h"

#include "log4c.h"


int main() {
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}

TEST(log4cTest, getLogTest) {
	struct log4c *log1 = get_log("./log/demo.log", true);
	struct log4c *log2 = get_log("./log/demo.log", true);
	EXPECT_EQ(log1, log2);
}

void *thread_routine(void *arg) {
	(void) arg;
	pthread_setname_np(pthread_self(), "child");
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	return nullptr;
}


TEST(log4cTest, logOut) {
	pthread_t id;
	pthread_create(&id, nullptr, thread_routine, nullptr);
	struct log4c *log = get_log("./log/demo.log", true);
	log_trace(log, "this is a trace log...");
	log_debug(log, "this is a debug log...");
	log_info(log, "this is a info log...");
	log_warn(log, "this is a warn log...");
	log_error(log, "this is a error log...");
	log_fatal(log, "this is a fatal log...");
	log_out(log, ERROR, "this is a trace log...");
	pthread_join(id, nullptr);
}
