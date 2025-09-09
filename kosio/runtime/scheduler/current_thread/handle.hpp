#pragma once
#include "kosio/runtime/config.hpp"
#include "kosio/runtime/scheduler/current_thread/worker.hpp"

namespace kosio::runtime::scheduler::current_thread {
class Handle : util::Noncopyable {
public:
    explicit Handle(const runtime::detail::Config& config)
    : worker_(this, config) {
        util::set_current_thread_name("kosio-WORKER-0");
    }

    ~Handle() {
        close();
    }

public:
    void schedule_task(std::coroutine_handle<> task) {
        worker_.schedule_remote(task);
    }

    void close() {
        worker_.shutdown();
    }

    void wait() {
        worker_.run();
    }

public:
    Worker  worker_;
};

static inline void schedule_local(std::coroutine_handle<> handle) {
    if (t_worker == nullptr) [[unlikely]] {
        std::unreachable();
    }
    t_worker->schedule_local(handle);
}

static inline void schedule_remote(std::coroutine_handle<> handle) {
    if (t_worker == nullptr) [[unlikely]] {
        std::unreachable();
    }
    t_worker->schedule_remote(handle);
}

template <typename It>
static inline void schedule_remote_batch(It itemFirst, std::size_t count) {
    if (t_worker == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_worker->schedule_remote_batch(itemFirst, count);
}
} // namespace kosio::runtime::scheduler::current_thread
