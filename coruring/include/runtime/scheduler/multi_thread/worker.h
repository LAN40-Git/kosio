#pragma once
#include "queue.h"
#include "async/coroutine/task.h"
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
    auto local_queue() -> LocalQueue&;

private:
    [[nodiscard]]
    auto transition_to_sleepling() -> bool;
    [[nodiscard]]
    auto transition_from_sleepling() -> bool;
    auto transition_to_searching() -> bool;
    void transition_from_searching();
    auto should_notify_others() const -> bool;
    void tick();
    /// 返回从本地队列中取出的任务数量
    void take_tasks();
    /// 返回窃取并完成的任务数量
    void steal_tasks();
    void handle_tasks() const;
    void maintenance();
    [[nodiscard]]
    auto poll() -> bool;
    void sleep();

private:
    // 这个结构体封装了每轮循环需要处理的任务
    struct TaskRemain {
        std::array<std::coroutine_handle<>,
                   runtime::detail::HANDLE_BATCH_SIZE> tasks{};
        // 剩余需要处理的任务数量
        std::size_t size{runtime::detail::HANDLE_BATCH_SIZE};
        // 下次取任务时 tasks 对应的下标
        std::size_t index{0};
    };

    std::size_t              index_;
    uint32_t                 tick_{0};
    TaskRemain               task_remain_;
    Handle*                  handle_;
    runtime::detail::Driver  driver_;
    LocalQueue               local_queue_;
    bool                     is_shutdown_{false};
    bool                     is_searching_{false};
};
} // namespace coruring::runtime::scheduler::multi_thread
