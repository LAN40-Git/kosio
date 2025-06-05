#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Timeout : public IoRegistrator<Timeout> {
public:
    Timeout(__kernel_timespec *ts, unsigned count, unsigned flags)
        : IoRegistrator{io_uring_prep_timeout, ts, count, flags} {}
};

[[nodiscard]]
static inline auto timeout(__kernel_timespec *ts, unsigned count, unsigned flags) {
    return Timeout{ts, count, flags};
}
} // namespace coruring::io
