cmake_minimum_required(VERSION 3.21)
project(kosio VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_compile_definitions(NEED_kosio_LOG)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

if(NOT TARGET liburing::liburing)
    add_library(liburing::liburing INTERFACE IMPORTED)
    set_target_properties(liburing::liburing PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LIBURING_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${LIBURING_LIBRARIES}"
    )
endif()

set(SOURCE_FILES
        kosio/lib/io/io.cpp
        kosio/lib/socket/net/socket.cpp
        kosio/lib/socket/socket.cpp
        kosio/lib/runtime/scheduler/multi_thread/worker.cpp
        kosio/lib/runtime/io/io_uring.cpp
        kosio/lib/runtime/timer/level.cpp
        kosio/lib/runtime/timer/timer.cpp
        kosio/lib/runtime/task/waker.cpp
        kosio/lib/runtime/scheduler/multi_thread/handle.cpp
        kosio/lib/runtime/scheduler/multi_thread/shared.cpp
        kosio/lib/runtime/scheduler/multi_thread/idle.cpp
        kosio/lib/runtime/scheduler/driver.cpp
        kosio/lib/runtime/scheduler/current_thread/worker.cpp
        kosio/lib/runtime/scheduler/current_thread/handle.cpp
)

add_executable(main
        kosio/lib/main.cpp
        ${SOURCE_FILES}
)

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(main PUBLIC
        liburing::liburing
)
