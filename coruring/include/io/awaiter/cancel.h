#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Cancle : public IoRegistrator<Cancle> {
public:
    Cancle(int fd, int flags)
        : IoRegistrator{io_uring_prep_cancel_fd, fd, flags} {}
};
static inline auto cancle(int fd, int flags) {
    return Cancle{fd, flags};
}
} // namespace coruring::io
