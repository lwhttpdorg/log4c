set(TARGET_NAME log4c)

file(GLOB SRC_FILES "*.c")

if (CMAKE_HOST_WIN32)
    add_library(${TARGET_NAME} STATIC ${SRC_FILES})
else ()
    add_library(${TARGET_NAME} SHARED ${SRC_FILES})
endif ()
