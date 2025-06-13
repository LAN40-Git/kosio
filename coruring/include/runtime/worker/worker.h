#pragma once
#include <mutex>
#include <thread>
#include <coroutine>
#include "queue.h"
#include "runtime/timer/timer.h"
#include "runtime/config.h"

namespace coruring::runtime::detail
{
class Worker : public util::Noncopyable {
    using TaskQueue = Queue<std::coroutine_handle<>>;
public:
    void run();
    void stop();
    bool is_running() const { return is_running_; }

public:
    auto local_queue() -> TaskQueue & { return local_queue_; }

private:
    void event_loop();
    
private:
    std::atomic<bool> is_running_{false};
    std::mutex        mutex_;
    std::thread       thread_;
    Timer             timer_;
    TaskQueue         local_queue_;
};
}
