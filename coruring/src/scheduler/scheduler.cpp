#include "scheduler/scheduler.h"

void coruring::scheduler::Scheduler::run() {
    std::lock_guard lock(mutex_);
    if (is_running_) {
        return;
    }
    is_running_.store(true, std::memory_order_release);
    for (std::size_t i = 0; i < worker_nums_; ++i) {
        auto worker = std::make_unique<runtime::detail::Worker>(*this);
        worker->run();
        workers_.emplace(std::move(worker));
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
}
