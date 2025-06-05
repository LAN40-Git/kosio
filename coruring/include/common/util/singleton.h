#pragma once
#include <utility>

namespace coruring::util
{
template <typename T>
class Singleton {
    Singleton() = delete;
    ~Singleton() = delete;
    
public:
    template <typename... Args>
    [[nodiscard]]
    static auto instance(Args&&... args) -> T& {
        static T instance(std::forward<Args>(args)...);
        return instance;
    }
};
} // namespace coruring::utils
