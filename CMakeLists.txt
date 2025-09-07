cmake_minimum_required(VERSION 3.21)
project(kosio VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

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

# ====== LIBRARY ======
add_library(kosio SHARED ${SOURCE_FILES})
target_include_directories(kosio PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kosio/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(kosio PUBLIC
        pthread
        uring
)

# ====== INSTALL ======
include(GNUInstallDirs)

install(TARGETS kosio
        EXPORT kosioTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY kosio/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kosio
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
add_executable(main kosio/lib/main.cpp)
target_link_libraries(main PRIVATE kosio)