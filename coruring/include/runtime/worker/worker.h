#pragma once
#include <mutex>
#include <thread>
#include "queue.h"
#include "runtime/timer/timer.h"
#include "runtime/config.h"
#include "async/coroutine/task.h"

namespace coruring::scheduler
{
class Scheduler;
}

namespace coruring::runtime::detail
{
class Worker : public util::Noncopyable {
    using TaskQueue = Queue<std::coroutine_handle<>>;
public:
    explicit Worker(scheduler::Scheduler& scheduler) : scheduler_(scheduler) {}
    ~Worker() { stop(); }

public:
    void run();
    void stop();
    [[nodiscard]]
    bool is_running() const { return is_running_; }

public:
    [[nodiscard]]
    auto local_queue() -> TaskQueue & { return local_queue_; }

private:
    void event_loop();
    
private:
    std::atomic<bool>     is_running_{false};
    std::mutex            mutex_;
    std::thread           thread_;
    Timer                 timer_;
    TaskQueue             local_queue_;
    scheduler::Scheduler& scheduler_;
};
}
