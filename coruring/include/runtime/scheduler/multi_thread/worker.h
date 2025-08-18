#pragma once
#include "queue.h"
#include "common/util/thread.h"
#include "runtime/driver.h"
#include "runtime/scheduler/multi_thread/shared.h"

namespace coruring::runtime::scheduler::multi_thread {
class Worker : util::Noncopyable {
    friend class Shared;
public:
    Worker(Shared& shared, std::size_t index);
    ~Worker();

public:
    void run();

private:
    std::size_t             index_;
    Shared&                 shared_;
    runtime::detail::Driver driver_;
    LocalQueue              local_queue_;
};
} // namespace coruring::runtime::scheduler::multi_thread
