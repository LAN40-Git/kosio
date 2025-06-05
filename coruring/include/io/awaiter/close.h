#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Close : public IoRegistrator<Close> {
public:
    Close(int fd)
        : IoRegistrator(io_uring_prep_close, fd) {}
};
} // namespace coruring::io
