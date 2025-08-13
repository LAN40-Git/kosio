#pragma once
#include "common/util/time.h"
#include "wheel/wheel.h"
#include "runtime/config.h"

namespace coruring::runtime::timer {
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : public util::Noncopyable {
public:
    Timer() noexcept;
    ~Timer();
    Timer(Timer &&) = delete;
    auto operator=(Timer &&) -> Timer& = delete;

public:
    // Add a timeout entry
    [[nodiscard]]
    auto insert(coruring::io::detail::Callback *data, uint64_t expiration_time) const noexcept
    -> Result<Entry*, TimerError>;
    // Remove a timeout entry
    static void remove(Entry* entry) noexcept;
    [[nodiscard]]
    // 这里返回的是相对时间，即当前时间距离下一个事件到期的时间间隔（ms）
    auto next_expiration() const noexcept -> std::optional<Expiration>;

public:
    void poll();

private:
    uint64_t     start_{util::current_ms()};
    wheel::Wheel wheel_;
};
} // namespace coruring::runtime::timer
