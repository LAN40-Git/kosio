cmake_minimum_required(VERSION 3.21)
project(coruring VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

# liburing
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

# json
find_package(nlohmann_json 3.5 REQUIRED)

# tbb
find_package(TBB REQUIRED)
if(TARGET TBB::tbb)
    add_library(TBB::TBB ALIAS TBB::tbb)
elseif(NOT TARGET TBB::TBB)
    message(FATAL_ERROR "TBB found but no valid target available")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/coruring/include)

# ====== INSTALL ======

## 添加库目标
#add_library(coruring SHARED
#        coruring/src/runtime/io/io_uring.cpp
#        coruring/src/io/io.cpp
#        coruring/src/socket/net/socket.cpp
#        coruring/src/socket/socket.cpp
#        coruring/src/scheduler/scheduler.cpp
#        coruring/src/runtime/worker/worker.cpp
#)
#
## 包含目录
#target_include_directories(coruring PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:include>
#)
#
## 链接库
#target_link_libraries(coruring PRIVATE
#    ${LIBURING_LIBRARIES}
#    nlohmann_json::nlohmann_json
#    TBB::TBB
#)
#
## 安装规则
#install(DIRECTORY coruring/include/ DESTINATION include/coruring) # 安装头文件
#install(TARGETS coruring DESTINATION lib)                # 安装库文件

# ====== INSTALL ======

# ====== TEST ======

#set(SOURCE_FILES
#        coruring/src/runtime/io/io_uring.cpp
#        coruring/src/io/io.cpp
#        coruring/src/socket/net/socket.cpp
#        coruring/src/socket/socket.cpp
#        coruring/src/scheduler/scheduler.cpp
#        coruring/src/runtime/worker/worker.cpp
#)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/coruring/include)

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