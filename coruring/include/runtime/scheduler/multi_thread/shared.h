#pragma once
#include "runtime/scheduler/multi_thread/queue.h"
#include "runtime/scheduler/multi_thread/idle.h"
#include <latch>

namespace coruring::runtime::scheduler::multi_thread {
class Worker;
class Shared;

inline thread_local Shared *t_shared{nullptr};

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
    void schedule_remote(std::coroutine_handle<> task);
    void schedule_remote_batch(std::list<std::coroutine_handle<>> &&handles, std::size_t n);

private:
    const runtime::detail::Config config_;
    Idle                          idle_;
    GlobalQueue                   global_queue_;
    std::latch                    shutdown_;
    std::vector<Worker*>          workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
