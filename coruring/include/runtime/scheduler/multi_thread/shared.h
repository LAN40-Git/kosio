#pragma once
#include "runtime/config.h"
#include "runtime/scheduler/multi_thread/worker.h"

namespace coruring::runtime::scheduler::multi_thread {
class Shared {
public:
    Shared();
    ~Shared();

private:
    const runtime::detail::Config config_;
    GlobalQueue                   global_queue_;
    std::vector<Worker*>          workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
