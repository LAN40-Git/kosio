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

auto coruring::runtime::scheduler::multi_thread::LocalQueue::steal_into(LocalQueue &dst, std::size_t count) -> bool {
    static std::array<std::coroutine_handle<>,
        runtime::detail::HANDLE_BATCH_SIZE> buf = {};
    auto steal_count = try_dequeue_bulk(buf.data(), count);
    return dst.enqueue_bulk(buf.data(), steal_count);
}
