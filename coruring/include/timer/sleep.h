#pragma once
#include "common/debug.h"
#include "io/base/callback.h"
#include "common/macros.h"
#include "runtime/timer/timer.h"

namespace coruring::timer {
namespace detail {
class Sleep {
public:
    Sleep(uint64_t deadline)
        : sqe_{runtime::io::t_ring->get_sqe()} {
        cb_.deadline_ = deadline;
        io_uring_sqe_set_data(sqe_, &this->cb_);
    }

public:
    [[nodiscard]]
    auto deadline() const noexcept {
        return cb_.deadline_;
    }

    auto await_ready() const noexcept -> bool {
        return sqe_ == nullptr;
    }

    auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool {
        auto ret = runtime::timer::t_timer->insert(&this->cb_, this->cb_.deadline_);
        if (ret.value()) [[likely]] {
            this->cb_.entry_ = ret.value();
            this->cb_.handle_ = handle;
            return true;
        } else {
            result_ = std::unexpected{ret.error()};
            return false;
        }
    }

    auto await_resume() const noexcept -> Result<void, TimerError> {
        return result_;
    }


private:
    io::detail::Callback     cb_{};
    io_uring_sqe            *sqe_;
    Result<void, TimerError> result_{};
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
// sleep `duration` ms
static auto inline sleep(uint64_t duration) {
    return detail::Sleep{util::current_ms() + duration};
}

[[REMEMBER_CO_AWAIT]]
static auto inline sleep_until(uint64_t deadline) {
    return detail::Sleep{deadline};
}
} // namespace coruring::timer
