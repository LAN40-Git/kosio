#pragma once
#include "kosio/io/io.hpp"
#include <stack>

namespace kosio::fs {
class Dir {
public:
    explicit Dir(mode_t permission = 0771)
        : permission_(permission) {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto create_dir(std::string_view dir_path) const {
        return io::mkdir(dir_path.data(), permission_);
    }

    [[REMEMBER_CO_AWAIT]]
    auto create_dir_all(std::string_view dir_path) const -> async::Task<Result<void, IoError>> {
        std::stack<std::string_view> dir_stack;
        std::string_view current_path = dir_path;

        auto ret = co_await create_dir(current_path);
        if (ret || ret.error().value() != EEXIST) {
            co_return ret;
        }

        while (true) {
            dir_stack.push(current_path);
            if (auto pos = current_path.rfind('/'); pos != std::string_view::npos) {
                current_path = current_path.substr(0, pos);
            } else {
                break;
            }

            ret = co_await create_dir(current_path);
            if (ret) {
                break;
            } else if (ret.error().value() != EEXIST) {
                co_return std::unexpected{ret.error()};
            }
        }

        while (!dir_stack.empty()) {
            ret = co_await create_dir(dir_stack.top());
            if (!ret) {
                co_return std::unexpected{ret.error()};
            }
            dir_stack.pop();
        }

        co_return Result<void, IoError>{};
    }

    void permission(mode_t permission) noexcept {
        permission_ = permission;
    }

private:
    mode_t permission_{0};
};

[[REMEMBER_CO_AWAIT]]
static inline auto create_dir(std::string_view dir_path, mode_t mode = 0771) {
    return io::mkdir(dir_path.data(), mode);
}

[[REMEMBER_CO_AWAIT]]
static inline auto remove_dir(std::string_view dir_path) {
    return io::unlink(dir_path.data(), AT_REMOVEDIR);
}
} // namespace kosio::fs