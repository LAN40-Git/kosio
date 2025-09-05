#pragma once
#include <mutex>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>

namespace coruring::runtime::detail {
// 分层时间轮最大层数
static inline constexpr std::size_t NUM_LEVELS = 6;
static_assert(NUM_LEVELS > 0, "MAX_LEVEL must be greater than 0");

// 分层时间轮每层槽位数
static inline constexpr std::size_t LEVEL_MULT = 64;
static_assert((LEVEL_MULT & (LEVEL_MULT - 1)) == 0, "SLOTS must be a power of 2");

static inline constexpr std::size_t LOCAL_QUEUE_CAPACITY = 256;

struct Config {
    // io_uring 队列纵深
    std::size_t entries{1024};

    // io_uring 事件提交间隔
    std::size_t submit_interval{64};

    // 工作线程重复轮询 io_uring 完成的 IO 事件的间隔
    std::size_t io_interval{61};

    // 工作现成从全局队列取出任务的 tick 次数
    std::size_t global_queue_interval{61};

    // 工作线程数量
    std::size_t num_workers{std::thread::hardware_concurrency()};
};
} // namespace coruring::runtime::detail

// TODO: 添加对Config的格式化特化以支持读取配置文件