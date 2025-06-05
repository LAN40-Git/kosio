#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class TimeoutRecv : public IoRegistrator<TimeoutRecv> {
public:
    TimeoutRecv(int sockfd, void *buf, size_t len, int flags)
        : IoRegistrator{io_uring_prep_recv, sockfd, buf, len, flags} {}
};
}