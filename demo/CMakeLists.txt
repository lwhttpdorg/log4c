set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.c)

target_include_directories(${TARGET_NAME} PUBLIC "../log4c/include")

target_link_libraries(${TARGET_NAME} log4c)

if (CMAKE_HOST_UNIX)
    target_link_libraries(demo pthread)
endif ()

file(GLOB TEST_CONFIG_FILES "*.conf")

file(COPY ${TEST_CONFIG_FILES} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
