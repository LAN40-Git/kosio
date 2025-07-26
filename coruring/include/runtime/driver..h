#pragma once
#include "config.h"
#include "runtime/io/io_uring.h"
#include "runtime/timer/timer.h"

namespace coruring::runtime::detail {
class Driver;

inline thread_local Driver* t_driver{nullptr};

class Driver {
public:
    Driver(const Config& config)
        : ring_(config) {
        assert(t_driver == nullptr);
        t_driver = this;
    }

    ~Driver() {
        t_driver = nullptr;
    }

public:
    void wait()

private:
    IoUring ring_;
    Timer   timer_;
};
} // namespace coruring::runtime::detail
