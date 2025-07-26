#pragma once
#include "wheel.h"
#include "runtime/config.h"

namespace coruring::runtime {
// 线程不安全
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : public util::Noncopyable {
public:
    Timer() = default;
    ~Timer() = default;

public:
    [[nodiscard]]
    auto add_entry(io::detail::Callback *data, uint64_t expiration_ms)
    -> std::expected<detail::Entry*, std::error_code>;

private:
    detail::Wheel<detail::MAX_LEVEL, detail::SLOTS> wheel_;
};
} // namespace coruring::runtime
