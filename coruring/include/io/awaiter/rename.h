#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Rename : public IoRegistrator<Rename> {
public:
    Rename(int olddfd, const char *oldpath, int newdfd, const char *newpath, int flags)
        : IoRegistrator{io_uring_prep_renameat, olddfd, oldpath, newdfd, newpath, flags} {}

    Rename(const char *oldpath, const char *newpath)
        : IoRegistrator{io_uring_prep_rename, oldpath, newpath} {}
};

[[nodiscard]]
static inline auto
renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath, int flags) {
    return Rename{olddfd, oldpath, newdfd, newpath, flags};
}

[[nodiscard]]
static inline auto rename(const char *oldpath, const char *newpath) {
    return Rename{oldpath, newpath};
}
}