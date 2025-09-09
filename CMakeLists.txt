cmake_minimum_required(VERSION 3.21)
project(KOSIO VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Version: " "${CMAKE_PROJECT_VERSION}")
message(STATUS "C++ standard: " "${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: " "${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build type: " "${CMAKE_BUILD_TYPE}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

# ====== LIBRARY ======
add_library(kosio SHARED
        kosio/io/io.cpp
        kosio/socket/socket.cpp
        kosio/socket/net/socket.cpp
        kosio/runtime/io/io_uring.cpp
        kosio/runtime/timer/level.cpp
        kosio/runtime/timer/timer.cpp
        kosio/runtime/task/waker.cpp
        kosio/runtime/scheduler/driver.cpp
        kosio/runtime/scheduler/multi_thread/idle.cpp
        kosio/runtime/scheduler/multi_thread/worker.cpp
        kosio/runtime/scheduler/multi_thread/handle.cpp
        kosio/runtime/scheduler/multi_thread/shared.cpp
        kosio/runtime/scheduler/current_thread/worker.cpp
        kosio/runtime/scheduler/current_thread/handle.cpp
)

set(DEPEND_LIBS
        pthread
        ${LIBURING_LIBRARIES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ====== INSTALL ======
include(GNUInstallDirs)

install(TARGETS kosio
        EXPORT kosioTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY kosio/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kosio
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.cpp" EXCLUDE
)

install(EXPORT kosioTargets
        FILE kosioTargets.cmake
        NAMESPACE kosio::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kosio
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/kosioConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/kosioConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/kosioConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kosio
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kosioConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/kosioConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kosio
)

# ====== TEST ======
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)