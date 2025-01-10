include_directories(include)

add_subdirectory(src)

if (ENABLE_TESTS)
    enable_testing()
endif ()