#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Read : public IoRegistrator<Read> {
public:
    Read(int fd, void* buf, unsigned nbytes, __u64 offse)
        : IoRegistrator{io_uring_prep_read, fd, buf, nbytes, offse} {}
};
}