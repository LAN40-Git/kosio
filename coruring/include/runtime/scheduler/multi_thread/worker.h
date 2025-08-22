#pragma once
#include "queue.h"
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
    void wake_up() const;

private:
    void shutdown();
    void handle_tasks();
    [[nodiscard]]
    auto transition_to_sleepling() -> bool;
    [[nodiscard]]
    auto transition_from_sleepling() -> bool;
    auto transition_to_searching() -> bool;
    void transition_from_searching();
    auto should_notify_others() const -> bool;
    void tick();
    [[nodiscard]]
    auto steal_work() -> std::size_t;
    void maintenance();
    [[nodiscard]]
    auto poll() -> bool;
    void sleep();

private:
    std::size_t              index_;
    uint32_t                 tick_{0};
    Handle*                  handle_;
    runtime::detail::Driver  driver_;
    LocalQueue               local_queue_;
    std::atomic<std::size_t> owned_tasks_;
    std::size_t              average_tasks_{0};
    bool                     is_shutdown_{false};
    bool                     is_searching_{false};
};
} // namespace coruring::runtime::scheduler::multi_thread
