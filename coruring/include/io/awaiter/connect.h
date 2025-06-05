#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Connect : public IoRegistrator<Connect> {
public:
    Connect(int fd, sockaddr *addr, socklen_t addrlen)
        : IoRegistrator{io_uring_prep_connect, fd, addr, addrlen} {}
};
}