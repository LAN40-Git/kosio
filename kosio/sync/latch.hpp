#pragma once
#include "kosio/common/macros.hpp"
#include "kosio/runtime/runtime.hpp"

namespace kosio::sync {
class Latch : util::Noncopyable {
    class Awaiter {
        friend class Latch;

    public:
        explicit Awaiter(Latch &latch)
            : latch_{latch} {}

    public:
        auto await_ready() const noexcept -> bool {
            return latch_.try_wait();
        }

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool {
            handle_ = handle;
            next_ = latch_.head_.load(std::memory_order_relaxed);
            while (!latch_.head_.compare_exchange_weak(next_,
                                                       this,
                                                       std::memory_order::acq_rel,
                                                       std::memory_order::relaxed)) {}
            return !latch_.try_wait();
        }

        constexpr void await_resume() const noexcept {}

    private:
        Latch&                  latch_;
        std::coroutine_handle<> handle_{nullptr};
        Awaiter                *next_{nullptr};
    };

public:
    explicit Latch(std::ptrdiff_t expected)
        : expected_(expected) {}

    // Delete move
    Latch(Latch &&) = delete;
    auto operator=(Latch &&) -> Latch & = delete;

    void count_down(std::ptrdiff_t update = 1) {
        auto odd = expected_.fetch_sub(update, std::memory_order::release);
        if (odd == update) {
            // notify all
            notify_all();
        }
    }

public:
    [[REMEMBER_CO_AWAIT]]
    auto wait() {
        return Awaiter{*this};
    }

    [[REMEMBER_CO_AWAIT]]
    auto try_wait() const -> bool {
        return expected_.load(std::memory_order::acquire) == 0;
    }

    [[REMEMBER_CO_AWAIT]]
    auto arrive_and_wait(std::ptrdiff_t update = 1) {
        count_down(update);
        return wait();
    }

private:
    void notify_all() const {
        auto head = head_.load(std::memory_order::acquire);
        while (head != nullptr) {
            runtime::detail::schedule_local(head->handle_);
            head = head->next_;
        }
    }

private:
    std::atomic<std::ptrdiff_t> expected_;
    std::atomic<Awaiter *>      head_{nullptr};
};
} // namespace kosio::sync