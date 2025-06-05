#pragma once
#include "io/io_uring.h"
#include "callback.h"
#include <functional>

namespace coruring::io
{
template<class IO>
class IoRegistrator {
public:
    template<typename F, typename... Args>
        requires std::is_invocable_v<F, io_uring_sqe *, Args...>
    IoRegistrator(F&& f, Args&&... args)
        : sqe_{IoUring::instance().get_sqe()} {
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
        io_uring_sqe_set_data(sqe_, &this->cb_);
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
        IoUring::instance().pend_submit();
        IoUring::callback_map().emplace(&this->cb_);
    }

    auto await_resume() noexcept -> int {
        void *user_data = &this->cb_;
        IoUring::callback_map().erase(user_data);
        return cb_.result_;
    }

protected:
    Callback cb_{};
    io_uring_sqe *sqe_;
};
}
