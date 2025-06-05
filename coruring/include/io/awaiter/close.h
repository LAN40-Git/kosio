#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Close : public IoRegistrator<Close> {
public:
    Close(int fd)
        : IoRegistrator(io_uring_prep_close, fd) {}
};

[[nodiscard]]
static inline auto close(int fd) {
    return Close {fd};
}
} // namespace coruring::io
