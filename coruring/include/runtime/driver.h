#pragma once
#include "config.h"
#include "runtime/io/io_uring.h"
#include "runtime/timer/timer.h"
#include "runtime/task/waker.h"

namespace coruring::runtime::detail {
class Driver;

inline thread_local Driver* t_driver{nullptr};

class Driver {
public:
    Driver(const Config& config);
    ~Driver();

public:
    void wait();

private:
    io::IoUring        ring_;
    timer::Timer       timer_;
    task::waker::Waker waker_;
};
} // namespace coruring::runtime::detail
