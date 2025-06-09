#pragma once
#include "io/base/registrator.h"

namespace coruring::time
{
namespace detail
{
    class Sleep : public io::detail::IoRegistrator<Sleep> {
    public:
        Sleep(long long tv_sec, long long tv_nsec, unsigned count, unsigned flags)
            : IoRegistrator{io_uring_prep_timeout, &ts_, count, flags}
            , ts_{.tv_sec = tv_sec, .tv_nsec = tv_nsec} {}

    private:
        __kernel_timespec ts_;
    };
}

[[REMEMBER_CO_AWAIT]]
static auto inline sleep(long long tv_sec, long long tv_nsec, unsigned count, unsigned flags) {
    return detail::Sleep{tv_sec, tv_nsec, count, flags};
}
}
