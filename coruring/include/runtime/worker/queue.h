#pragma once
#include <queue>

#include "third_party/concurrentqueue.h"

namespace coruring::scheduler::detail
{
template <typename T>
class Queue {
public:
    bool enqueue(T&& item) {
        return queue_.enqueue(std::forward<T>(item));
    }

    bool try_dequeue(T& item) {
        return queue_.try_dequeue(item);
    }

    template <typename It>
    requires std::input_iterator<It> &&
         std::convertible_to<std::iter_value_t<It>, T>
    bool enqueue_bulk(It itemFirst, std::size_t count) {
        return queue_.enqueue_bulk(std::make_move_iterator(itemFirst), count);
    }

    template <typename It>
    requires std::forward_iterator<It> &&
        std::assignable_from<std::iter_reference_t<It>, T>
    std::size_t try_dequeue_bulk(It itemFirst, std::size_t max) {
        return queue_.try_dequeue_bulk(itemFirst, max);
    }

    std::size_t size() const noexcept {
        return queue_.size_approx();
    }

    bool empty() const noexcept {
        return queue_.size_approx() == 0;
    }

private:
    moodycamel::ConcurrentQueue<T> queue_;
};
}
