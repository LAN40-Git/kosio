#pragma once
#include "wheel/wheel.h"
#include "runtime/config.h"

namespace coruring::runtime::detail {
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : public util::Noncopyable {
public:
    Timer() = default;
    ~Timer() = default;

public:
    // Add a timeout entry
    [[nodiscard]]
    auto add_entry(io::detail::Callback *data, uint64_t expiration_time) noexcept
    -> std::expected<Entry*, std::error_code>;

    // Remove a timeout entry
    void remove_entry(Entry* entry) noexcept;

    //

private:
    uint64_t      start_{util::current_ms()};
    Wheel wheel_;
};
} // namespace coruring::runtime
