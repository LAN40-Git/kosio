#pragma once
#include "runtime/scheduler/multi_thread/worker.h"

namespace coruring::runtime::scheduler {
class Scheduler {
public:
    Scheduler();
    ~Scheduler();

private:
    multi_thread::Handle handle_;
};
} // namespace coruring::runtime::scheduler
