#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class MkDir : public IoRegistrator<MkDir> {
public:
    MkDir(int dfd, const char *path, mode_t mode)
        : IoRegistrator{io_uring_prep_mkdirat, dfd, path, mode} {}

    MkDir(const char *path, mode_t mode)
        : MkDir{AT_FDCWD, path, mode} {}
};

[[nodiscard]]
static inline auto mkdir(const char *path, mode_t mode) {
    return MkDir{path, mode};
}

[[nodiscard]]
static inline auto mkdirat(int dfd, const char *path, mode_t mode) {
    return MkDir{dfd, path, mode};
}

}