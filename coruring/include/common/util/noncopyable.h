#pragma once

namespace coruring::util {
class Noncopyable {
public:
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() noexcept = default;
};
} // namespace coruring::util
