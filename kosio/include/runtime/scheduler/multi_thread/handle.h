#pragma once
#include "shared.h"
#include "runtime/config.h"
#include "runtime/scheduler/multi_thread/worker.h"

namespace kosio::runtime::scheduler::multi_thread {
class Handle {
public:
    explicit Handle(const runtime::detail::Config& config);
    ~Handle();

public:
    void schedule_task(std::coroutine_handle<> task);
    void close() const;
    void wait();

public:
    std::vector<std::thread> threads_;
    Shared                   shared_;
};

static inline void schedule_local(std::coroutine_handle<> handle) {
    if (t_worker == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_worker->schedule_local(handle);
}

static inline void schedule_remote(std::coroutine_handle<> handle) {
    if (t_shared == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_shared->schedule_remote(handle);
}

template <typename It>
static inline void schedule_remote_batch(It itemFirst, std::size_t count) {
    if (t_shared == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_shared->schedule_remote_batch(itemFirst, count);
}
} // namespace kosio::runtime::scheduler::multi_thread
