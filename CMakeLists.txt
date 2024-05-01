cmake_minimum_required(VERSION 3.11)

project(log4c)

set(INSTALL_HEAD_FILES ${INCLUDE_DIRECTORIES}/log4c.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set(CMAKE_C_STANDARD 17)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -march=native -pipe")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    endif ()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif ()

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wpointer-arith -Wwrite-strings \
	-Wno-uninitialized -Wno-unused-parameter -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable \
	-Wno-unused-value -pipe")
endif ()

option(ENABLE_DEMO "Enable demo" OFF)
option(ENABLE_TESTS "Enable test" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# AddressSanitizer
if (ENABLE_ASAN)
    set(ASAN_FLAGS "-fsanitize=address")
    if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        set(ASAN_FLAGS "${ASAN_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment")
    endif ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
endif ()

if (ENABLE_DEMO)
    add_subdirectory(demo)
endif ()

if (CMAKE_HOST_WIN32)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set(BOOST_ROOT "D:/OpenCode/boost/gcc")
    elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        set(BOOST_ROOT "D:/OpenCode/boost/msvc")
    endif ()
else ()
    set(BOOST_ROOT "/usr/local/boost")
endif ()

add_subdirectory(liblog4c)

target_include_directories(log4c PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/liblog4c/include> PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
