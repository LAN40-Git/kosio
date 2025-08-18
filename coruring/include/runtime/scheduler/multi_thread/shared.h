#pragma once
#include "queue.h"
#include "runtime/config.h"

namespace coruring::runtime::scheduler::multi_thread {
class Worker;

class Shared {
    friend class Worker;
public:
    Shared();
    ~Shared();

private:
    const runtime::detail::Config config_;
    GlobalQueue                   global_queue_;
    std::vector<Worker*>          workers_;
};
} // namespace coruring::runtime::scheduler::multi_thread
