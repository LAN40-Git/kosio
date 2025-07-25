#pragma once

#include <mutex>
#include <thread>
#include "runtime/timer/timer.h"
#include "runtime/config.h"
#include "async/coroutine/task.h"
#include "third_party/concurrentqueue.h"

namespace coruring::runtime::detail
{
class Scheduler;
} // namespace coruring::runtime::detail

namespace coruring::runtime::multi_thread::detail
{
class Worker : public util::Noncopyable {
    using TaskQueue = moodycamel::ConcurrentQueue<std::coroutine_handle<>>;
public:
    explicit Worker(const runtime::detail::Config& config, runtime::detail::Scheduler& scheduler)
        : config_(config), scheduler_(scheduler) {}

public:
    void run();
    void stop();
    [[nodiscard]]
    auto is_running() const -> bool { return is_running_.load(std::memory_order_relaxed); }

public:
    [[nodiscard]]
    auto local_queue() -> TaskQueue & { return local_queue_; }
    [[nodiscard]]
    auto local_tasks() const noexcept -> std::size_t { return local_tasks_.load(std::memory_order_relaxed); }
    void add_tasks(std::size_t count) { local_tasks_.fetch_add(count, std::memory_order_relaxed); }
    void remove_tasks(std::size_t count) { local_tasks_.fetch_sub(count, std::memory_order_relaxed); }

private:
    void event_loop();
    
private:
    const runtime::detail::Config& config_;
    std::atomic<bool>              is_running_{false};
    std::mutex                     mutex_;
    std::thread                    thread_;
    TaskQueue                      local_queue_;
    runtime::detail::Scheduler&    scheduler_;
    std::atomic<std::size_t>       local_tasks_{0};
};
} // namespace coruring::runtime::multi_thread::detail
