#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Write : public IoRegistrator<Write> {
public:
    Write(int fd, void* buf, unsigned nbytes, __u64 offse)
        : IoRegistrator{io_uring_prep_write, fd, buf, nbytes, offse} {}
};

[[nodiscard]]
static inline auto write(int fd, void* buf, unsigned nbytes, __u64 offse) {
    return Write(fd, buf, nbytes, offse);
}
}