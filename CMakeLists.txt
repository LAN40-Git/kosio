cmake_minimum_required(VERSION 3.21)
project(KOSIO VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(${CMAKE_INSTALL_PREFIX} /usr/local)

message(STATUS "Version: " "${CMAKE_PROJECT_VERSION}")
message(STATUS "C++ standard: " "${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: " "${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build type: " "${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX: " "${CMAKE_INSTALL_PREFIX}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

# ====== LIBRARY ======
set(DEPEND_LIBS
        pthread
        ${LIBURING_LIBRARIES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ====== INSTALL ======
include(GNUInstallDirs)

install(DIRECTORY kosio/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kosio
)

# ====== EXAMPLES ======
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
# ====== TESTS ======
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)