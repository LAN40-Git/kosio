#pragma once
#include "queue.h"
#include "runtime/driver.h"

namespace coruring::runtime::scheduler::multi_thread {
class Worker {
public:
    Worker();
    ~Worker();

public:
    void run();

private:
    std::size_t             index_;
    LocalQueue              local_queue_;
    runtime::detail::Driver driver_;
};
} // namespace coruring::runtime::scheduler::multi_thread
