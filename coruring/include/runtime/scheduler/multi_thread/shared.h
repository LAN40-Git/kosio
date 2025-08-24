#pragma once
#include "runtime/config.h"
#include "runtime/scheduler/queue.h"
#include "runtime/scheduler/multi_thread/idle.h"
#include <latch>

namespace coruring::runtime::scheduler::multi_thread {
class Worker;
class Shared;

static inline thread_local Shared *t_shared{nullptr};

class Shared {
    friend class Worker;
public:
    explicit Shared(const runtime::detail::Config &config);
    ~Shared();

public:
    void wake_up_one();
    void wake_up_all() const;
    void wake_up_if_work_pending();
    void close() const;
    auto schedule_remote(std::coroutine_handle<> task) -> bool;
    template <typename It>
    auto schedule_remote_batch(It itemFirst, std::size_t count) -> bool {
        return global_queue_.enqueue_bulk(itemFirst, count);
    }

private:
    const runtime::detail::Config config_;
    Idle                          idle_;
    detail::TaskQueue             global_queue_;
    std::latch                    shutdown_;
    std::vector<Worker*>          workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
