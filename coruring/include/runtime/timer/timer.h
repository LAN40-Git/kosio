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
    [[nodiscard]]
    static auto instance() -> Timer& {
        thread_local Timer instance;
        return instance;
    }

    [[nodiscard]]
    auto add_entry(io::detail::Callback *data, int64_t expiration_ms)
    -> std::expected<detail::Entry*, std::error_code> {
        int64_t remaining_ms = expiration_ms - util::current_ms();
        if (remaining_ms <= 0) [[unlikely]] {
            return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
                 std::system_category()}};
        }
        if (remaining_ms >=
            detail::TimingWheel<detail::Config::MAX_LEVEL, detail::Config::SLOTS>::PRECISION[detail::Config::MAX_LEVEL-1])
            [[unlikely]] {
            return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
                 std::system_category()}};
        }

        auto entry = std::make_unique<detail::Entry>(detail::Entry{data, expiration_ms});
        return wheel_.add_entry(std::move(entry), (remaining_ms));
    }

    void tick() {
        wheel_.tick();
    }

    [[nodiscard]]
    bool is_idle() const noexcept {
        return wheel_.is_idle();
    }


private:
    detail::TimingWheel<detail::Config::MAX_LEVEL, detail::Config::SLOTS> wheel_;
};
}
