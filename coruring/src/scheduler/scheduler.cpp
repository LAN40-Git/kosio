#include "scheduler/scheduler.h"

void coruring::scheduler::Scheduler::run() {
    std::lock_guard lock(mutex_);
    if (is_running_) {
        return;
    }
    is_running_.store(true, std::memory_order_release);
    for (std::size_t i = 0; i < worker_nums_; ++i) {
        auto worker = std::make_unique<runtime::detail::Worker>(config_, *this);
        worker->run();
        workers_.emplace_back(std::move(worker));
    }
}

void coruring::scheduler::Scheduler::stop() {
    std::lock_guard lock(mutex_);
    if (!is_running_) {
        return;
    }
    is_running_.store(false, std::memory_order_release);
    for (auto& worker : workers_) {
        worker->stop();
    }
    workers_.clear();
    // 清理协程资源
    for (auto& handle : handles_ | std::views::keys) {
        if (handle && !handle.done()) {
            handle.destroy();
        }
    }
    handles_.clear();
    // 清空全局队列
    while (global_queue_.size_approx() > 0) {
        global_queue_.try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
    }
}
