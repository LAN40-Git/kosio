#pragma once
#include "io/io.h"

namespace coruring::io::detail
{
template<class T>
struct ImplAsyncRead {
    [[REMEMBER_CO_AWAIT]]
    auto read(std::span<char> buf) const noexcept {
        return Read{static_cast<const T*>(this)->fd(),
                    buf.data(),
                    static_cast<unsigned int>(buf.size_bytes()),
                    static_cast<std::size_t>(-1)};
    }

    [[REMEMBER_CO_AWAIT]]
    auto read_exact(std::span<char> buf) const noexcept
    -> async::Task<std::expected<void, std::error_code>> {
        std::expected<std::size_t, std::error_code> ret{0uz};
        while (!buf.empty()) {
            ret = co_await this->read(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                // TODO: 完善错误处理: 读取中断
                co_return std::unexpected{ret.error()};
            }
            buf = buf.subspan( ret.value(), buf.size_bytes() - ret.value());
        }
        co_return {};
    }
};
}
