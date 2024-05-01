set(TARGET_NAME demo)

add_executable(${TARGET_NAME} demo.c)

target_include_directories(${TARGET_NAME} PUBLIC "../liblog4c/include")

target_link_libraries(${TARGET_NAME} log4c)

if (CMAKE_HOST_UNIX)
    target_link_libraries(demo pthread)
endif ()
