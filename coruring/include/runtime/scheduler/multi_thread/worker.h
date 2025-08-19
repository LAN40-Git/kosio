#pragma once
#include "queue.h"
#include "common/util/thread.h"
#include "runtime/driver.h"
#include "runtime/scheduler/multi_thread/handle.h"

namespace coruring::runtime::scheduler::multi_thread {
class Worker : util::Noncopyable {
public:
    Worker(std::size_t index, Handle* handle, const runtime::detail::Config &config);
    ~Worker();

public:
    void run();

private:
    std::size_t             index_;
    Handle*                 handle_;
    runtime::detail::Driver driver_;
    LocalQueue              local_queue_;
};
} // namespace coruring::runtime::scheduler::multi_thread
