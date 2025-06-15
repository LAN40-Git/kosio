#pragma once

#include <mutex>
#include <thread>
#include "runtime/timer/timer.h"
#include "runtime/config.h"
#include "async/coroutine/task.h"
#include "third_party/concurrentqueue.h"

namespace coruring::scheduler
{
class Scheduler;
}

namespace coruring::runtime::detail
{
class Worker : public util::Noncopyable {
    using TaskQueue = moodycamel::ConcurrentQueue<std::coroutine_handle<>>;
    using IoBuf = std::array<std::coroutine_handle<>, Config::IO_BATCH_SIZE>;
public:
    explicit Worker(const Config& config, scheduler::Scheduler& scheduler)
        : config_(config), scheduler_(scheduler) {}

public:
    void run();
    void stop();
    [[nodiscard]]
    bool is_running() const { return is_running_.load(std::memory_order_relaxed); }

public:
    [[nodiscard]]
    auto local_queue() -> TaskQueue & { return local_queue_; }

private:
    void event_loop();
    void clear() noexcept;
    
private:
    const Config&         config_;
    std::atomic<bool>     is_running_{false};
    std::mutex            mutex_;
    std::thread           thread_;
    TaskQueue             local_queue_;
    scheduler::Scheduler& scheduler_;
    IoBuf                 io_buf_;
};
}
