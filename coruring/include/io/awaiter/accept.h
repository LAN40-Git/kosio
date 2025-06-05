#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Accept : public IoRegistrator<Accept> {
public:
    Accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags)
        : IoRegistrator(io_uring_prep_accept, fd, addr, addrlen, flags) {}
};
} // namespace coruring::io