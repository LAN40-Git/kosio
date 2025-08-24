#pragma once
#include "queue.h"
#include <array>
#include <coroutine>

namespace coruring::runtime::scheduler::detail {
// 工作线程每次执行的任务的最大数量
static inline constexpr std::size_t HANDLE_BATCH_SIZE = 128;

class FiredTasks {
    using PendTasks = std::array<std::coroutine_handle<>, HANDLE_BATCH_SIZE>;
public:
    auto pend_tasks(TaskQueue& tasks) -> std::size_t;
    [[nodiscard]]
    auto pending_tasks() noexcept -> PendTasks&;
    [[nodiscard]]
    auto size() const noexcept -> std::size_t;
    [[nodiscard]]
    auto full() const noexcept -> bool;
    void reset();

private:
    PendTasks   pending_tasks_{};
    std::size_t size_{0};
};
} // namespace coruring::runtime::scheduler::detail
