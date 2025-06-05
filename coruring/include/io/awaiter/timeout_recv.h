#pragma once
#include "io/base/registrator.h"

namespace coruring::io
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
        io_uring_sqe *sqe = IoUring::instance().get_sqe();
        if (!sqe) [[unlikely]] {
            return;
        }
        io_uring_prep_link_timeout(sqe, ts, 0);
        IoUring::instance().pend_submit();
    }
};
static inline auto timeout_recv(int sockfd, void *buf, size_t len, int flags, __kernel_timespec *ts) {
    return TimeoutRecv(sockfd, buf, len, flags, ts);
}
}