#pragma once
#include "kosio/include/runtime/io/io_uring.h"
#include "kosio/include/common/concepts.h"
#include "kosio/include/common/macros.h"
#include "kosio/include/common/error.h"
#include "kosio/include/common/util/time.h"
#include "kosio/include/common/debug.h"
#include "kosio/include/io/base/callback.h"
#include "kosio/include/time/timeout.h"
#include <functional>

namespace kosio::io::detail {
template <class IO>
class IoRegistrator {
public:
    template<typename F, typename... Args>
        requires std::is_invocable_v<F, io_uring_sqe *, Args...>
    IoRegistrator(F&& f, Args&&... args)
        : sqe_{runtime::io::t_ring->get_sqe()} {
        if (sqe_ != nullptr) [[likely]] {
            std::invoke(std::forward<F>(f), sqe_, std::forward<Args>(args)...);
            io_uring_sqe_set_data(sqe_, &this->cb_);
        } else {
            cb_.result_ = -ENOMEM;
        }
    }

    IoRegistrator(const IoRegistrator&) = delete;
    auto operator=(const IoRegistrator&) -> IoRegistrator& = delete;
    IoRegistrator(IoRegistrator&& other) noexcept
        : cb_{other.cb_}
        , sqe_{other.sqe_} {
        io_uring_sqe_set_data(sqe_, &this->cb_);
        other.sqe_ = nullptr;
    }
    auto operator=(IoRegistrator&& other) noexcept -> IoRegistrator& {
        cb_ = other.cb_;
        sqe_ = other.sqe_;
        if (sqe_) io_uring_sqe_set_data(sqe_, &this->cb_);
        other.sqe_ = nullptr;
        return *this;
    }

public:
    auto await_ready() const noexcept -> bool {
        return sqe_ == nullptr;
    }

    auto await_suspend(std::coroutine_handle<> handle) {
        assert(sqe_);
        cb_.handle_ = handle;
        runtime::io::t_ring->pend_submit();
    }

    [[REMEMBER_CO_AWAIT]]
    auto set_timeout_at(uint64_t deadline) noexcept {
        cb_.deadline_ = static_cast<int64_t>(deadline);
        return time::detail::Timeout{std::move(*static_cast<IO*>(this))};
    }

    [[REMEMBER_CO_AWAIT]]
    auto set_timeout(uint64_t timeout) noexcept {
        return set_timeout_at(util::current_ms() + static_cast<int64_t>(timeout));
    }

protected:
    Callback      cb_{};
    io_uring_sqe *sqe_;
};
} // namespace kosio::io::detail
