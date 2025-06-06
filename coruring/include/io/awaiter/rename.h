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

    auto await_resume() noexcept -> std::expected<void, std::error_code> {
        IoUring::callback_map().erase(&this->cb_);
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};

[[REMEMBER_CO_AWAIT]]
static inline auto
renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath, int flags) {
    return Rename{olddfd, oldpath, newdfd, newpath, flags};
}

[[REMEMBER_CO_AWAIT]]
static inline auto rename(const char *oldpath, const char *newpath) {
    return Rename{oldpath, newpath};
}
}