cmake_minimum_required(VERSION 3.21)
project(kosio VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(NEED_kosio_LOG)

include(GNUInstallDirs)
set(kosio_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/kosio")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)
find_package(nlohmann_json 3.5 REQUIRED)

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
        kosio/lib/runtime/runtime.cpp
        kosio/lib/runtime/scheduler/current_thread/worker.cpp
        kosio/lib/runtime/scheduler/current_thread/handle.cpp
)

#target_include_directories(kosio PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kosio/include>
#    $<INSTALL_INTERFACE:include/kosio>
#)
#
#target_link_libraries(kosio PUBLIC
#    liburing::liburing
#    nlohmann_json::nlohmann_json
#)

include(CMakePackageConfigHelpers)

# ====== INSTALL ======

#configure_package_config_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/kosioConfig.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/kosioConfig.cmake"
#    INSTALL_DESTINATION "${kosio_INSTALL_CMAKEDIR}"
#)
#
#write_basic_package_version_file(
#    "${CMAKE_CURRENT_BINARY_DIR}/kosioConfigVersion.cmake"
#    VERSION "${PROJECT_VERSION}"
#    COMPATIBILITY SameMajorVersion
#)
#
#install(TARGETS kosio
#    EXPORT kosioTargets
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    INCLUDES DESTINATION include
#)
#
#install(DIRECTORY kosio/include/
#    DESTINATION include/kosio
#    FILES_MATCHING PATTERN "*.h"
#)
#
#install(EXPORT kosioTargets
#    FILE kosioTargets.cmake
#    NAMESPACE kosio::
#    DESTINATION "${kosio_INSTALL_CMAKEDIR}"
#)
#
#install(FILES
#    "${CMAKE_CURRENT_BINARY_DIR}/kosioConfig.cmake"
#    "${CMAKE_CURRENT_BINARY_DIR}/kosioConfigVersion.cmake"
#    DESTINATION "${kosio_INSTALL_CMAKEDIR}"
#)
#
#export(EXPORT kosioTargets
#    FILE "${CMAKE_CURRENT_BINARY_DIR}/kosioTargets.cmake"
#    NAMESPACE kosio::
#)

# ====== INSTALL ======

# ====== TEST ======

add_executable(main
        kosio/lib/main.cpp
        ${SOURCE_FILES}
)

target_include_directories(main PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kosio/include>
        $<INSTALL_INTERFACE:include/kosio>
)

target_link_libraries(main PUBLIC
        liburing::liburing
        nlohmann_json::nlohmann_json
)

# ====== TEST ======