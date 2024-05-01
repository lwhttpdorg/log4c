include_directories(include)

add_subdirectory(src)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()