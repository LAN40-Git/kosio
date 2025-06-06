#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class TimeoutRecv : public IoRegistrator<TimeoutRecv> {
    public:
        TimeoutRecv(int sockfd, void *buf, size_t len, int flags, __kernel_timespec *ts)
            : IoRegistrator{io_uring_prep_recv, sockfd, buf, len, flags} {
            if (!sqe_) [[unlikely]] {
                return;
            }
            sqe_->flags |= IOSQE_IO_LINK; // 标记为链接模式
            // 链接超时请求
            io_uring_sqe *sqe = detail::IoUring::instance().get_sqe();
            if (!sqe) [[unlikely]] {
                return;
            }
            io_uring_prep_link_timeout(sqe, ts, 0);
            detail::IoUring::instance().pend_submit();
        }

        auto await_resume() noexcept -> std::expected<std::size_t, std::error_code> {
            detail::IoUring::callback_map().erase(&this->cb_);
            if (this->cb_.result_ >= 0) [[likely]] {
                return static_cast<std::size_t>(this->cb_.result_);
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto timeout_recv(int sockfd, void *buf, size_t len, int flags, __kernel_timespec *ts) {
    return detail::TimeoutRecv(sockfd, buf, len, flags, ts);
}
}