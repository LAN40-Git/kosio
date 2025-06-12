#pragma once
#include <thread>
#include <coroutine>
#include "queue.h"

namespace coruring::scheduler::detail
{
class Thread {
public:
    
private:
    std::thread thread_;
    Queue<std::coroutine_handle<>> local_queue_;
};
}
