cmake_minimum_required(VERSION 3.21)
project(coruring VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(NEED_CORURING_LOG)

include(GNUInstallDirs)
set(CORURING_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/coruring")

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
        coruring/lib/io/io.cpp
        coruring/lib/socket/net/socket.cpp
        coruring/lib/socket/socket.cpp
        coruring/lib/runtime/scheduler/multi_thread/worker.cpp
        coruring/lib/runtime/io/io_uring.cpp
        coruring/lib/runtime/timer/entry.cpp
        coruring/lib/runtime/timer/level.cpp
        coruring/lib/runtime/timer/timer.cpp
        coruring/lib/runtime/task/waker.cpp
        coruring/lib/runtime/scheduler/multi_thread/handle.cpp
        coruring/lib/runtime/scheduler/multi_thread/shared.cpp
        coruring/lib/runtime/scheduler/multi_thread/idle.cpp
        coruring/lib/runtime/scheduler/driver.cpp
        coruring/lib/runtime/runtime.cpp
        coruring/lib/runtime/scheduler/current_thread/worker.cpp
        coruring/lib/runtime/scheduler/current_thread/handle.cpp
)

#target_include_directories(coruring PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/coruring/include>
#    $<INSTALL_INTERFACE:include/coruring>
#)
#
#target_link_libraries(coruring PUBLIC
#    liburing::liburing
#    nlohmann_json::nlohmann_json
#)

include(CMakePackageConfigHelpers)

# ====== INSTALL ======

#configure_package_config_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/coruringConfig.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/coruringConfig.cmake"
#    INSTALL_DESTINATION "${CORURING_INSTALL_CMAKEDIR}"
#)
#
#write_basic_package_version_file(
#    "${CMAKE_CURRENT_BINARY_DIR}/coruringConfigVersion.cmake"
#    VERSION "${PROJECT_VERSION}"
#    COMPATIBILITY SameMajorVersion
#)
#
#install(TARGETS coruring
#    EXPORT coruringTargets
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    INCLUDES DESTINATION include
#)
#
#install(DIRECTORY coruring/include/
#    DESTINATION include/coruring
#    FILES_MATCHING PATTERN "*.h"
#)
#
#install(EXPORT coruringTargets
#    FILE coruringTargets.cmake
#    NAMESPACE coruring::
#    DESTINATION "${CORURING_INSTALL_CMAKEDIR}"
#)
#
#install(FILES
#    "${CMAKE_CURRENT_BINARY_DIR}/coruringConfig.cmake"
#    "${CMAKE_CURRENT_BINARY_DIR}/coruringConfigVersion.cmake"
#    DESTINATION "${CORURING_INSTALL_CMAKEDIR}"
#)
#
#export(EXPORT coruringTargets
#    FILE "${CMAKE_CURRENT_BINARY_DIR}/coruringTargets.cmake"
#    NAMESPACE coruring::
#)

# ====== INSTALL ======

# ====== TEST ======

add_executable(main
        coruring/lib/main.cpp
        ${SOURCE_FILES}
)

target_include_directories(main PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/coruring/include>
        $<INSTALL_INTERFACE:include/coruring>
)

target_link_libraries(main PUBLIC
        liburing::liburing
        nlohmann_json::nlohmann_json
)

# ====== TEST ======