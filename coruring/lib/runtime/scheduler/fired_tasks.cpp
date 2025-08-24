#include "runtime/scheduler/fired_tasks.h"

auto coruring::runtime::scheduler::detail::FiredTasks::pend_tasks(TaskQueue &tasks) -> std::size_t {
    // 外界需要检查是否已满
    auto count = tasks.try_dequeue_bulk(
        pending_tasks_.data() + size_,
        HANDLE_BATCH_SIZE - size_);
    size_ += count;
    return count;
}

auto coruring::runtime::scheduler::detail::FiredTasks::pending_tasks() noexcept -> PendTasks& {
    return pending_tasks_;
}

auto coruring::runtime::scheduler::detail::FiredTasks::size() const noexcept -> std::size_t {
    return size_;
}

auto coruring::runtime::scheduler::detail::FiredTasks::full() const noexcept -> bool {
    return size_ >= HANDLE_BATCH_SIZE;
}

void coruring::runtime::scheduler::detail::FiredTasks::reset() {
    size_ = 0;
}
