#include "runtime/scheduler/multi_thread/queue.h"

auto coruring::runtime::scheduler::multi_thread::detail::BaseQueue::enqueue(std::coroutine_handle<> &&task) -> bool {
    return tasks_.enqueue(task);
}

auto coruring::runtime::scheduler::multi_thread::detail::BaseQueue::try_dequeue(std::coroutine_handle<> &task) -> bool {
    return tasks_.try_dequeue(task);
}

auto coruring::runtime::scheduler::multi_thread::detail::BaseQueue::size_approx() const -> std::size_t {
    return tasks_.size_approx();
}

auto coruring::runtime::scheduler::multi_thread::detail::BaseQueue::empty() const -> bool {
    return tasks_.size_approx() == 0;
}
