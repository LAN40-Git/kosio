#pragma once
#include "io/io.h"

namespace coruring::io::detail
{
template<class T>
struct ImplAsyncSend {
    [[REMEMBER_CO_AWAIT]]
    auto send(std::span<const char> buf) noexcept {
        return Send{static_cast<T*>(this)->fd(),
                    buf.data(), buf.size(), 0};
    }

    [[REMEMBER_CO_AWAIT]]
    auto send_all(std::span<const char> buf) noexcept
    -> async::Task<std::expected<void, std::error_code>> {
        std::expected<std::size_t, std::error_code> ret{0uz};
        while (!buf.empty()) {
            ret = co_await this->send(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                // TODO: 完善错误处理: 写入中断
                co_return std::unexpected{ret.error()};
            }
            buf = buf.subspan( ret.value(), buf.size_bytes() - ret.value());
        }
        co_return std::expected<void, std::error_code>{};
    }
};
}
