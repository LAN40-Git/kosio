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
public:
    Shared(const runtime::detail::Config &config);
    ~Shared();

private:
    const runtime::detail::Config config_;
    Idel                          idel_;
    GlobalQueue                   global_queue_;
    std::latch                    shutdown_{0};
    std::vector<Worker*>          workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
