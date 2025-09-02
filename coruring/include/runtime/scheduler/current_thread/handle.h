#pragma once
#include "runtime/config.h"
#include "runtime/scheduler/current_thread/worker.h"
#include <tbb/concurrent_hash_map.h>

namespace coruring::runtime::scheduler::current_thread {
class Handle : util::Noncopyable {
public:
    explicit Handle(const runtime::detail::Config& config);
    ~Handle();

public:
    void schedule_task(std::coroutine_handle<> task);
    void close();
    void wait();

public:
    using TaskMap = tbb::concurrent_hash_map<std::coroutine_handle<>, uint64_t>;
    Worker  worker_;
    TaskMap tasks_;
};
} // namespace coruring::runtime::scheduler::current_thread
