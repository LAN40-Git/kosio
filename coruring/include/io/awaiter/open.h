#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Open : public IoRegistrator<Open> {
public:
    Open(int dfd, const char *path, int flags, mode_t mode)
        : IoRegistrator{io_uring_prep_openat, dfd, path, flags, mode} {}

    Open(const char *path, int flags, mode_t mode)
        : Open{AT_FDCWD, path, flags, mode} {}
};
}