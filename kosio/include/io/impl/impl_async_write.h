#pragma once
#include "kosio/include/io/io.h"

namespace kosio::io::detail {
template<class T>
struct ImplAsyncWrite {
    [[REMEMBER_CO_AWAIT]]
    auto write(std::span<const char> buf) noexcept {
        return Write{static_cast<T *>(this)->fd(),
                     buf.data(),
                     static_cast<unsigned int>(buf.size_bytes()),
                     static_cast<std::size_t>(-1)};
    }

    [[REMEMBER_CO_AWAIT]]
    auto write_all(std::span<const char> buf) noexcept
    -> async::Task<std::expected<void, std::error_code>> {
        std::expected<std::size_t, std::error_code> ret{0uz};
        while (!buf.empty()) {
            ret = co_await this->write(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                co_return std::unexpected{std::make_error_code(std::errc::connection_reset)};
            }
            buf = buf.subspan( ret.value(), buf.size_bytes() - ret.value());
        }
        co_return std::expected<void, std::error_code>{};
    }
};
} // namespace kosio::io::detail
