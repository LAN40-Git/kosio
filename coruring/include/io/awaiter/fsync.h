#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Fsync : public IoRegistrator<Fsync> {
public:
    Fsync(int fd, unsigned fsync_flags)
        : IoRegistrator{io_uring_prep_fsync, fd, fsync_flags} {}
};
} // namespace coruring::io