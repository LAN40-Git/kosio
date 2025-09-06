#pragma once
#include "kosio/include/common/util/noncopyable.h"
#include "kosio/include/runtime/scheduler/driver.h"
#include "kosio/include/runtime/scheduler/current_thread/queue.h"

namespace kosio::runtime::scheduler::current_thread {
class Worker;
class Handle;
inline thread_local Worker *t_worker{nullptr};

class Worker : util::Noncopyable {
public:
    Worker(Handle* handle, const runtime::detail::Config &config);

public:
    void run();
    void shutdown();
    void wake_up() const;
    void schedule_local(std::coroutine_handle<> task);
    void schedule_remote(std::coroutine_handle<> task);
    void schedule_remote_batch(std::list<std::coroutine_handle<>> &&handles, [[maybe_unused]] std::size_t n);

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
    LocalQueue              local_queue_;
    GlobalQueue             global_queue_;
    bool                    is_shutdown_{false};
    bool                    is_searching_{false};
};
} // namespace kosio::runtime::scheduler::current_thread
