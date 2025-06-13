#pragma once
#include <mutex>
#include <thread>
#include <coroutine>
#include "queue.h"

namespace coruring::scheduler::detail
{
class Thread {
public:
    void wait();
    void run();
    void stop();

private:
    void event_loop();
    
private:
    std::atomic<bool> is_running_{false};
    std::mutex mutex_;
    std::thread thread_;
    Queue<std::coroutine_handle<>> local_queue_;
};
}
