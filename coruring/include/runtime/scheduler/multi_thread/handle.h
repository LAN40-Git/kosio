#pragma once
#include "shared.h"
#include "runtime/config.h"
#include "runtime/scheduler/driver.h"
#include <tbb/concurrent_hash_map.h>

namespace coruring::runtime::scheduler::multi_thread {
class Handle {
public:
    explicit Handle(const runtime::detail::Config& config);
    ~Handle();

public:
    void schedule_task(std::coroutine_handle<> task);
    void close();
    void wait();

public:
    using TaskMap = tbb::concurrent_hash_map<std::coroutine_handle<>, uint64_t>;
    std::vector<std::thread> threads_;
    TaskMap                  tasks_;
    Shared                   shared_;
};
} // namespace coruring::runtime::scheduler::multi_thread
