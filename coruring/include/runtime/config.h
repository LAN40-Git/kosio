#pragma once
#include <mutex>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>

namespace coruring::runtime::detail {
// io_uring 队列纵深
static inline constexpr std::size_t ENTRIES = 2048;

// io_uring 事件提交间隔
static inline constexpr std::size_t SUBMIT_INTERVAL = 64;

// 分层时间轮最大层数
static inline constexpr std::size_t NUM_LEVELS = 6;
static_assert(NUM_LEVELS > 0, "MAX_LEVEL must be greater than 0");

// 分层时间轮每层槽位数
static inline constexpr std::size_t LEVEL_MULT = 64;
static_assert((LEVEL_MULT & (LEVEL_MULT - 1)) == 0, "SLOTS must be a power of 2");

// 工作线程每次执行的任务的最大数量
static inline constexpr std::size_t HANDLE_BATCH_SIZE = 128;

// 工作线程每次尝试收割的 IO 事件的最大数量
static inline constexpr std::size_t PEEK_BATCH_SIZE = 128;

struct Config {
    // io_uring 队列纵深
    std::size_t entries{ENTRIES};

    // io_uring 事件提交间隔
    std::size_t submit_interval{SUBMIT_INTERVAL};

    // 工作线程重复轮询 io_uring 完成的 IO 事件的间隔
    std::size_t io_interval{64};

    // 工作线程数量
    std::size_t num_workers{std::thread::hardware_concurrency()};
};
} // namespace coruring::runtime::detail

// TODO: 添加对Config的格式化特化以支持读取配置文件