#pragma once
#include "third_party/concurrentqueue.h"
#include "runtime/config.h"
#include <coroutine>

namespace coruring::runtime::scheduler::multi_thread {
namespace detail {
class BaseQueue {
public:
    auto enqueue(std::coroutine_handle<>&& task) -> bool;

    template <typename It>
    auto enqueue_bulk(It itemFirst, std::size_t count) -> bool {
        return tasks_.enqueue_bulk(itemFirst, count);
    }

    [[nodiscard]]
    auto try_dequeue(std::coroutine_handle<>& task) -> bool;

    template <typename It>
    [[nodiscard]]
    auto try_dequeue_bulk(It itemFirst, std::size_t max)
    -> std::size_t {
        return tasks_.try_dequeue_bulk(itemFirst, max);
    }

    [[nodiscard]]
    auto size_approx() const -> std::size_t;

    [[nodiscard]]
    auto empty() const -> bool;

private:
    moodycamel::ConcurrentQueue<std::coroutine_handle<>> tasks_;
};
} // namespace detail

class LocalQueue : public detail::BaseQueue {

};

class GlobalQueue : public detail::BaseQueue {

};
} // namespace coruring::runtime::scheduler::multi_thread
