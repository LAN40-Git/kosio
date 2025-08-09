#pragma once
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "common/util/nocopyable.h"

namespace coruring::runtime::detail {
// 默认配置

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

// 工作线程每次从本地队列中恢复的 io 事件的最大数量
static inline constexpr std::size_t IO_INTERVAL = 64;

// 工作线程每次从 io_uring 完成队列中收割的完成事件的最大数量
static inline constexpr std::size_t PEEK_BATCH_SIZE = 128;

// 窃取因子，以平均任务为基准计算窃取区间，当且仅当工作线程的任务数量在此区间时窃取任务
static inline constexpr float STEAL_FACTOR = 1.05;

struct Config {
    // io_uring 队列纵深
    std::size_t entries{ENTRIES};

    // io_uring 事件提交间隔
    std::size_t submit_interval{SUBMIT_INTERVAL};
};
} // namespace coruring::runtime::detail

// TODO: 添加对Config的格式化特化以支持读取配置文件