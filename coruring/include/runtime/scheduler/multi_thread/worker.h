#pragma once
#include "runtime/scheduler/multi_thread/handle.h"
#include "common/util/random.h"

namespace coruring::runtime::scheduler::multi_thread {
static inline thread_local Worker *t_worker{nullptr};

class Worker : util::Noncopyable {
    friend class Shared;
public:
    Worker(std::size_t index, Handle* handle, const runtime::detail::Config &config);
    ~Worker();

public:
    void run();
    void shutdown();
    void wake_up() const;
    void schedule_local(std::coroutine_handle<> task);
    auto local_queue() -> detail::TaskQueue&;

private:
    void run_task(std::coroutine_handle<> task);
    [[nodiscard]]
    auto transition_to_sleepling() -> bool;
    [[nodiscard]]
    auto transition_from_sleepling() -> bool;
    auto transition_to_searching() -> bool;
    void transition_from_searching();
    [[nodiscard]]
    auto should_notify_others() const -> bool;
    void tick();
    [[nodiscard]]
    auto next_task() -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto next_local_task() -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto next_remote_task() const -> std::optional<std::coroutine_handle<>>;
    [[nodiscard]]
    auto steal_task() -> std::optional<std::coroutine_handle<>>;
    void maintenance();
    [[nodiscard]]
    auto poll() -> bool;
    void sleep();

private:
    using TaskBuffer = std::array<std::coroutine_handle<>, runtime::detail::MAX_QUEUE_BATCH_SIZE>;
    std::size_t                            index_;
    uint32_t                               tick_{0};
    util::FastRand                         rand_{};
    Handle*                                handle_;
    detail::Driver                         driver_;
    std::optional<std::coroutine_handle<>> lifo_slot_;
    detail::TaskQueue                      local_queue_;
    bool                                   is_shutdown_{false};
    bool                                   is_searching_{false};
};
} // namespace coruring::runtime::scheduler::multi_thread
