#pragma once
#include "common/util/noncopyable.h"
#include "runtime/scheduler/driver.h"
#include "runtime/scheduler/queue.h"

namespace coruring::runtime::scheduler::current_thread {
class Worker;
class Handle;
static inline thread_local Worker *t_worker{nullptr};

class Worker : util::Noncopyable {
public:
    Worker(Handle* handle, const runtime::detail::Config &config);

public:
    void run();
    void shutdown();
    void wake_up() const;
    void schedule_local(std::coroutine_handle<> task);
    void schedule_remote(std::coroutine_handle<> task);
    template <typename It>
    void schedule_remote_batch(It itemFirst, std::size_t count) {
        global_queue_.enqueue_bulk(itemFirst, count);
    }

private:
    void run_task(std::coroutine_handle<> task);
    void tick();
    void maintenance();
    [[nodiscard]]
    auto next_task() -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto next_local_task() -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto next_remote_task() -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto poll() -> bool;
    void sleep();

private:
    uint32_t                tick_{0};
    Handle*                 handle_;
    runtime::detail::Config config_;
    detail::Driver          driver_;
    std::optional<std::coroutine_handle<>> lifo_slot_;
    detail::TaskQueue       local_queue_;
    detail::TaskQueue       global_queue_;
    bool                    is_shutdown_{false};
    bool                    is_searching_{false};
};
} // namespace coruring::runtime::scheduler::current_thread
