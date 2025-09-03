#pragma once
#include "runtime/config.h"
#include "runtime/io/io_uring.h"
#include "runtime/timer/timer.h"
#include "runtime/task/waker.h"
#include "queue.h"

namespace coruring::runtime::scheduler::detail {
class Driver;

inline thread_local Driver* t_driver{nullptr};

class Driver {
public:
    explicit Driver(const runtime::detail::Config& config);
    ~Driver();

public:
    void wait(TaskQueue &local_queue);
    auto poll(TaskQueue &local_queue) -> bool;
    void wake_up() const;

private:
    io::IoUring        ring_;
    timer::Timer       timer_;
    task::waker::Waker waker_;
};
} // namespace coruring::runtime::scheduler
