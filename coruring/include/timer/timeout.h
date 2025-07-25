#pragma once
#include "runtime/timer/timer.h"

namespace coruring::io::detail {
template <class T>
class IoRegistrator;
} // namespace coruring::io::detail

namespace coruring::timer
{
namespace detail
{
template <class T>
    requires std::derived_from<T, io::detail::IoRegistrator<T>>
class Timeout : public T {
    public:
        Timeout(T&& io)
            : T{std::move(io)} {}

    public:
        auto await_suspend(std::coroutine_handle<> handle) -> bool {
            // 尝试创建定时任务
            auto ret = runtime::Timer::instance().add_entry(&this->cb_, this->cb_.deadline_);
            if (ret) [[likely]] {
                this->cb_.entry_ = ret.value();
                T::await_suspend(handle);
                return true;
            } else {
                this->cb_.result_ = -ret.error().value();
                io_uring_prep_nop(this->sqe_);
                io_uring_sqe_set_data(this->sqe_, nullptr);
                return false;
            }
        }
    };
} // namespace detail

template <class T>
    requires std::derived_from<T, io::detail::IoRegistrator<T>>
auto timeout_at(T &&io, uint64_t deadline) {
    return io.set_timeout_at(deadline);
}

template <class T>
    requires std::derived_from<T, io::detail::IoRegistrator<T>>
auto timeout(T &&io, uint64_t timeout_ms) {
    return io.set_timeout(timeout_ms);
}
} // namespace coruring::time::detail