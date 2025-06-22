cmake_minimum_required(VERSION 3.21)
project(coruring VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

find_package(nlohmann_json 3.5 REQUIRED)

find_package(TBB REQUIRED)
if(TARGET TBB::tbb)
    add_library(TBB::TBB ALIAS TBB::tbb)
elseif(NOT TARGET TBB::TBB)
    message(FATAL_ERROR "TBB found but no valid target available")
endif()

# ====== INSTALL ======

add_library(coruring STATIC
    coruring/src/runtime/io/io_uring.cpp
    coruring/src/io/io.cpp
    coruring/src/socket/net/socket.cpp
    coruring/src/socket/socket.cpp
    coruring/src/runtime/scheduler.cpp
    coruring/src/runtime/worker/worker.cpp
)

target_include_directories(coruring PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/coruring/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}/coruring>
)

target_link_libraries(coruring PRIVATE
    ${LIBURING_LIBRARIES}
    nlohmann_json::nlohmann_json
    TBB::TBB
)

configure_package_config_file(
    cmake/coruringConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/coruringConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/coruring
)

install(TARGETS coruring
    EXPORT coruringTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT coruringTargets
    FILE coruringTargets.cmake
    NAMESPACE coruring::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/coruring
)

install(DIRECTORY coruring/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/coruringConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/coruring
)

# ====== INSTALL ======

# ====== TEST ======

#set(SOURCE_FILES
#        coruring/src/runtime/io/io_uring.cpp
#        coruring/src/io/io.cpp
#        coruring/src/socket/net/socket.cpp
#        coruring/src/socket/socket.cpp
#        coruring/src/runtime/scheduler.cpp
#        coruring/src/runtime/worker/worker.cpp
#)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/coruring/include)
#
#add_executable(main
#        coruring/src/main.cpp
#        ${SOURCE_FILES}
#)
#target_include_directories(main PUBLIC
#        ${LIBURING_INCLUDE_DIRS}
#)
#target_link_libraries(main PRIVATE
#        ${LIBURING_LIBRARIES}
#        nlohmann_json::nlohmann_json
#        TBB::TBB
#)

# ====== TEST ======