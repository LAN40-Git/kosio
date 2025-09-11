#pragma once
#include "kosio/sync/condition_variable.hpp"

namespace kosio::sync {
class SharedMutex : util::Noncopyable {
public:
    SharedMutex() = default;

    // Delete move
    SharedMutex(SharedMutex &&) = delete;
    auto operator=(SharedMutex &&) -> SharedMutex & = delete;

public:
    [[REMEMBER_CO_AWAIT]]
    auto lock() -> async::Task<void> {
        co_await mutex_.lock();
        std::lock_guard lock{mutex_, std::adopt_lock};

        co_await cond_.wait(mutex_, [this]() -> bool { return !has_writer(); });
        state_ |= WRITER_FLAG;
        co_await writer_cond_.wait(mutex_, [this]() -> bool { return count_reader() == 0; });
    }

    [[REMEMBER_CO_AWAIT]]
    auto unlock() -> async::Task<void> {
        co_await mutex_.lock();
        std::lock_guard lock{mutex_, std::adopt_lock};

        assert(state_ == WRITER_FLAG);
        state_ = 0;
        cond_.notify_all();
    }

    [[REMEMBER_CO_AWAIT]]
    auto lock_shared() -> async::Task<void> {
        co_await mutex_.lock();
        std::lock_guard lock{mutex_, std::adopt_lock};

        co_await cond_.wait(mutex_, [this]() -> bool { return state_ < READER_MASK; });
        state_ += 1;
        mutex_.unlock();
    }

    [[REMEMBER_CO_AWAIT]]
    auto unlock_shared() -> async::Task<void> {
        co_await mutex_.lock();
        std::lock_guard lock{mutex_, std::adopt_lock};

        auto prev{state_};
        state_ -= 1;
        if (has_writer() && count_reader() == 0) {
            writer_cond_.notify_one();
        } else if (prev == READER_MASK) {
            cond_.notify_one();
        }
    }

private:
    [[nodiscard]]
    auto has_writer() -> bool {
        return (state_ & WRITER_FLAG) != 0ull;
    }

    [[nodiscard]]
    auto count_reader() -> uint64_t {
        return state_ & READER_MASK;
    }

private:
    static constexpr uint64_t WRITER_FLAG{1ull << (sizeof(uint64_t) * 8 - 1)};
    static constexpr uint64_t READER_MASK{~WRITER_FLAG};

private:
    Mutex             mutex_{};
    ConditionVariable cond_{};
    ConditionVariable writer_cond_{};
    uint64_t          state_{};
};
} // namespace kosio::sync