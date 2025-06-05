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

class Open2 : public IoRegistrator<Open2> {
public:
    Open2(int dfd, const char *path, struct open_how *how)
        : IoRegistrator{io_uring_prep_openat2, dfd, path, how} {}

    Open2(const char *path, struct open_how *how)
        : Open2{AT_FDCWD, path, how} {}
};

[[nodiscard]]
auto open(const char *path, int flags, mode_t mode) {
    return Open{path, flags, mode};
}

[[nodiscard]]
static inline auto open2(const char *path, struct open_how *how) {
    return Open2{path, how};
}

[[nodiscard]]
static inline auto openat(int dfd, const char *path, int flags, mode_t mode) {
    return Open{dfd, path, flags, mode};
}

[[nodiscard]]
static inline auto openat2(int dfd, const char *path, struct open_how *how) {
    return Open2{dfd, path, how};
}
}