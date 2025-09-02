#pragma once
#include "runtime/scheduler/fired_tasks.h"
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
    auto local_queue() -> detail::TaskQueue&;

private:
    [[nodiscard]]
    auto transition_to_sleepling() -> bool;
    [[nodiscard]]
    auto transition_from_sleepling() -> bool;
    auto transition_to_searching() -> bool;
    void transition_from_searching();
    [[nodiscard]]
    auto should_notify_others() const -> bool;
    void tick();
    void take_tasks();
    void steal_tasks();
    void handle_tasks();
    void maintenance();
    [[nodiscard]]
    auto poll() -> bool;
    void sleep();

private:
    std::size_t        index_;
    uint32_t           tick_{0};
    detail::FiredTasks fired_tasks_;
    Handle*            handle_;
    detail::Driver     driver_;
    detail::TaskQueue  local_queue_;
    bool               is_shutdown_{false};
    bool               is_searching_{false};
};
} // namespace coruring::runtime::scheduler::multi_thread
