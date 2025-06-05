#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Recv : public IoRegistrator<Recv> {
public:
    Recv(int sockfd, void *buf, size_t len, int flags)
        : IoRegistrator{io_uring_prep_recv, sockfd, buf, len, flags} {}
};

[[nodiscard]]
static inline auto recv(int sockfd, void *buf, size_t len, int flags) {
    return Recv{sockfd, buf, len, flags};
}
}