#include "scheduler/thread.h"

void coruring::scheduler::detail::Thread::run() {
    std::lock_guard lock(mutex_);
    if (is_running_) {
        return;
    }
    is_running_.store(true, std::memory_order_release);
    thread_ = std::thread([&]() {
        event_loop();
    });
}

void coruring::scheduler::detail::Thread::stop() {
    std::lock_guard lock(mutex_);
    if (!is_running_) {
        return;
    }
    is_running_.store(false, std::memory_order_release);
    if (thread_.joinable()) {
        thread_.join();
    }
}

void coruring::scheduler::detail::Thread::event_loop() {
    std::array<std::coroutine_handle<>, >
    while (is_running_.load(std::memory_order_relaxed)) {
    }
}
