#pragma once
#include "common/util/time.h"
#include "wheel/wheel.h"
#include "runtime/config.h"

namespace coruring::runtime::timer {
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : public util::Noncopyable {
public:
    Timer() {
        t_timer = this;
    }
    ~Timer() {
        t_timer = nullptr;
    }

public:
    // Add a timeout entry
    [[nodiscard]]
    auto insert(io::detail::Callback *data, uint64_t expiration_time) const noexcept
    -> Result<Entry*, TimerError>;
    // Remove a timeout entry
    static void remove(Entry* entry) noexcept;

private:
    uint64_t     start_{util::current_ms()};
    wheel::Wheel wheel_;
};
} // namespace coruring::runtime::timer
