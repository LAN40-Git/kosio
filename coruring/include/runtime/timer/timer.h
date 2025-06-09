#pragma once
#include "timingwheel.h"
#include "runtime/config.h"

namespace coruring::runtime
{
class Timer {
public:
    Timer();
    ~Timer();

public:
    void add_entry();

private:
    detail::TimingWheel<detail::Config::MAX_LEVEL, detail::Config::SLOTS> wheel_;
};
}
