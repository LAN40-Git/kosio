#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Worker::Worker(
    std::size_t index,
    Handle* handle,
    const runtime::detail::Config &config)
    : index_(index)
    , handle_(handle)
    , driver_(config) {
    handle_->shared_.workers_.push_back(this);
    t_worker = this;
    t_shared = &handle_->shared_;
}

coruring::runtime::scheduler::multi_thread::Worker::~Worker() {
    t_worker = nullptr;
    t_shared = nullptr;
    handle_->shared_.shutdown_.arrive_and_wait();
}

void coruring::runtime::scheduler::multi_thread::Worker::run() {
    std::array<std::coroutine_handle<>, runtime::detail::IO_INTERVAL> io_buf;
    while (!is_shutdown_) [[likely]] {
        auto count = local_queue_.try_dequeue_bulk(io_buf.data(), io_buf.size());
        for (std::size_t i = 0; i < count; ++i) {
            io_buf[i].resume();
            // 最外层的协程完成了，说明任务完成，销毁协程
            if (io_buf[i].done() && handle_->tasks_.erase(io_buf[i])) {
                io_buf[i].destroy();
                owned_tasks_.fetch_sub(1);
            }
        }

        if (poll()) {
            continue;
        }

        sleep();
    }
}

void coruring::runtime::scheduler::multi_thread::Worker::wake_up() const {
    driver_.wake_up();
}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_to_sleepling() -> bool {

}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_from_sleepling() -> bool {

}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_to_searching() -> bool {

}

void coruring::runtime::scheduler::multi_thread::Worker::transition_from_searching() {

}

auto coruring::runtime::scheduler::multi_thread::Worker::should_notify_others() -> bool {

}

auto coruring::runtime::scheduler::multi_thread::Worker::steal_work() -> std::size_t {

}

void coruring::runtime::scheduler::multi_thread::Worker::check_shutdown() {

}

auto coruring::runtime::scheduler::multi_thread::Worker::poll() -> bool {
    if (!driver_.poll(local_queue_)) {
        return false;
    }
    if (should_notify_others()) {
        handle_->shared_.wake_up_one();
    }
    return true;
}

void coruring::runtime::scheduler::multi_thread::Worker::sleep() {
    check_shutdown();
    if (transition_to_sleepling()) {
        while (!is_shutdown_) [[likely]] {
            driver_.wait(local_queue_);
            check_shutdown();
            if (transition_from_sleepling()) {
                break;
            }
        }
    }
}
