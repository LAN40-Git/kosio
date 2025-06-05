#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Send : public IoRegistrator<Send> {
public:
    Send(int sockfd, const void *buf, size_t len, int flags)
        : IoRegistrator{io_uring_prep_send, sockfd, buf, len, flags} {}
};

[[nodiscard]]
static inline auto send(int sockfd, const void *buf, size_t len, int flags) {
    return Send{sockfd, buf, len, flags};
}
}