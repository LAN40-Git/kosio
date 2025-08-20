#pragma once
#include "queue.h"
#include "runtime/config.h"
#include "runtime/scheduler/multi_thread/idel.h"
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
    void close() const;
    void schedule_remote(std::coroutine_handle<> task);
    void schedule_remote_batch();


private:
    const runtime::detail::Config config_;
    Idel                          idel_;
    GlobalQueue                   global_queue_;
    std::latch                    shutdown_;
    std::vector<Worker*> workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
