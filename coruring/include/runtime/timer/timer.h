#pragma once
#include "wheel.h"
#include "runtime/config.h"

namespace coruring::runtime {
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : public util::Noncopyable {
public:
    Timer() = default;
    ~Timer() = default;

public:
    [[nodiscard]]
    auto add_entry(io::detail::Callback *data, uint64_t expiration_time) noexcept
    -> std::expected<detail::Entry*, std::error_code>;
    void remove_entry(detail::Entry* entry) noexcept;

private:
    uint64_t      start_{util::current_ms()};
    detail::Wheel wheel_;
};
} // namespace coruring::runtime
