include(FetchContent)

FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_MakeAvailable(googletest)

set(TEST_TARGET_NAME "Log4cTests")

add_test(NAME mainTest COMMAND ${TEST_TARGET_NAME} --exe $<TARGET_FILE:${TEST_TARGET_NAME}> WORKING_DIRECTORY "./")

add_executable(${TEST_TARGET_NAME} mainTest.cpp)

target_link_libraries(${TEST_TARGET_NAME} GTest::gtest_main)
target_link_libraries(${TEST_TARGET_NAME} log4c)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME} DISCOVERY_TIMEOUT 30)
