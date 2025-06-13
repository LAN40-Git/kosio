#pragma once
#include <mutex>
#include <thread>
#include <coroutine>
#include "queue.h"
#include "runtime/timer/timer.h"
#include "runtime/config.h"

namespace coruring::runtime::detail
{
class Thread {
public:
    Thread(const Config &config) : config_(config) {}

public:
    void run();
    void stop();

private:
    void event_loop();
    
private:
    const Config&                  config_;
    std::atomic<bool>              is_running_{false};
    std::mutex                     mutex_;
    std::thread                    thread_;
    Timer                          timer_;
    Queue<std::coroutine_handle<>> local_queue_;
};
}
