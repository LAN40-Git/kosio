#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Cancel : public IoRegistrator<Cancel> {
public:
    Cancel(int fd, int flags)
        : IoRegistrator{io_uring_prep_cancel_fd, fd, flags} {}
};

[[nodiscard]]
static inline auto cancel(int fd, int flags) {
    return Cancel{fd, flags};
}
} // namespace coruring::io
