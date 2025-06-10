#pragma once
#include "timingwheel.h"
#include "runtime/config.h"

namespace coruring::runtime
{
// 线程不安全
class Timer : public util::Noncopyable {
public:
    Timer() = default;
    ~Timer() = default;

public:
    void add_entry(std::unique_ptr<detail::Entry> &&entry, int64_t expiration_ms_) {
        wheel_.add_entry(std::move(entry), expiration_ms_);
    }

    void tick() {
        wheel_.tick();
    }

private:
    detail::TimingWheel<detail::Config::MAX_LEVEL, detail::Config::SLOTS> wheel_;
};
}
