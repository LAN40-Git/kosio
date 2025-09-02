#include "runtime/scheduler/queue.h"

auto coruring::runtime::scheduler::detail::TaskQueue::enqueue(std::coroutine_handle<> &&task) -> bool {
    return tasks_.enqueue(task);
}

auto coruring::runtime::scheduler::detail::TaskQueue::try_dequeue(std::coroutine_handle<> &task) -> bool {
    return tasks_.try_dequeue(task);
}

auto coruring::runtime::scheduler::detail::TaskQueue::size_approx() const -> std::size_t {
    return tasks_.size_approx();
}

auto coruring::runtime::scheduler::detail::TaskQueue::empty() const -> bool {
    return tasks_.size_approx() == 0;
}

void coruring::runtime::scheduler::detail::TaskQueue::put_into(TaskQueue &dst, std::size_t max) {
    auto count = this->try_dequeue_bulk(
        task_buffer_.data(),
        std::min(task_buffer_.size(), max));
    dst.enqueue_bulk(task_buffer_.data(), count);
}
