#pragma once
#include "runtime/config.h"
#include "runtime/scheduler/current_thread/worker.h"

namespace coruring::runtime::scheduler::current_thread {
class Handle : util::Noncopyable {
public:
    explicit Handle(const runtime::detail::Config& config);
    ~Handle();

public:
    void schedule_task(std::coroutine_handle<> task);
    void close();
    void wait();

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
} // namespace coruring::runtime::scheduler::current_thread
