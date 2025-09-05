#pragma once
#include "log.h"
#include "common/error.h"
#include "runtime/io/io_uring.h"
#include "io/base/callback.h"
#include <optional>
#include <list>

namespace coruring::runtime::timer {
namespace detail {
static constexpr auto compute_slot_range() -> std::array<uint64_t, runtime::detail::NUM_LEVELS> {
    std::array<uint64_t, runtime::detail::NUM_LEVELS> slot_range{};
    slot_range[0] = 1;
    for (auto i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        slot_range[i] = slot_range[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return slot_range;
}

static constexpr auto compute_level_range() -> std::array<uint64_t, runtime::detail::NUM_LEVELS> {
    std::array<uint64_t, runtime::detail::NUM_LEVELS> level_range{};
    level_range[0] = 64;
    for (auto i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        level_range[i] = level_range[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return level_range;
}
} // namespace detail

// 分层时间轮支持的
static constexpr uint64_t MAX_DURATION = (1ULL << (6 * runtime::detail::NUM_LEVELS)) - 1;

// 每层槽位对应的时间跨度
static constexpr auto SLOT_RANGE = detail::compute_slot_range();

// 每层对应的时间跨度
static constexpr auto LEVEL_RANGE = detail::compute_level_range();

class Entry {
public:
    Entry() = default;
    explicit Entry(coruring::io::detail::Callback *data, uint64_t expiration_time)
        : data_{data}
        , expiration_time_{expiration_time} {}

    explicit Entry(std::coroutine_handle<> handle, uint64_t expiration_time)
        : handle_{handle}
        , expiration_time_{expiration_time} {}

public:
    template <typename LocalQueue, typename GlobalQueue>
    void execute(LocalQueue &local_queue, GlobalQueue &global_queue) const {
        // 若 io 事件已完成，则 data_ 会被设置为 nullptr
        // 此时不需要进行操作
        if (data_) {
            // 若 io 事件未完成，则提交取消请求
            if (auto sqe = io::t_ring->get_sqe()) [[likely]] {
                io_uring_prep_cancel(sqe, data_, 0);
                io_uring_sqe_set_data(sqe, nullptr);
                io::t_ring->pend_submit();
            }
            // 将 entry_ 设置为 nullptr
            // 告知 worker 不需要将事件从分层时间轮中移除
            data_->entry_ = nullptr;
        } else if (handle_) {
            // 睡眠时间未提交到 io_uring，需要手动将句柄加入任务队列
            local_queue.push_back_or_overflow(handle_, global_queue);
        }
    }

public:
    template <typename T>
        requires std::constructible_from<Entry, T, uint64_t>
    [[nodiscard]]
    static auto make(T pargma, uint64_t expiration_time)
    -> std::pair<std::unique_ptr<Entry>, Entry*> {
        auto entry = std::make_unique<Entry>(pargma, expiration_time);
        return std::make_pair(std::move(entry), entry.get());
    }

public:
    coruring::io::detail::Callback *data_{nullptr};
    std::coroutine_handle<>         handle_{nullptr};
    uint64_t                        expiration_time_{};
};

struct Expiration {
    std::size_t level;
    std::size_t slot;
    uint64_t deadline;
};

using EntryList = std::list<std::unique_ptr<Entry>>;
using Slots = std::array<EntryList, runtime::detail::LEVEL_MULT>;
} // namespace coruring::runtime::timer
