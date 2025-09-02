#pragma once
#include <coroutine>
#include <stdexcept>
#include <optional>
#include <cassert>
#include <iostream>
#include <format>

namespace coruring::async {
template <typename T>
class Task;
struct TaskPromiseBase {
    struct TaskFinalAwaiter {
        constexpr auto await_ready() const noexcept -> bool { return false; }
        template <typename T>
        auto await_suspend(std::coroutine_handle<T> callee) const noexcept -> std::coroutine_handle<> {
            if (callee.promise().caller_) {
                return callee.promise().caller_;
            } else {
                if (callee.promise().exception_ != nullptr) [[unlikely]] {
                    try {
                        std::rethrow_exception(callee.promise().exception_);
                    } catch (const std::exception &e) {
                        std::cerr << std::format("catch a exception: {}", e.what());
                        #ifndef NDEBUG
                            std::terminate();
                        #endif
                    } catch (...) {
                        std::cerr << "catch a unknown exception\n";
                        #ifndef NDEBUG
                            std::terminate();
                        #endif
                    }
                }
                callee.destroy();
                return std::noop_coroutine();
            }
        }
        constexpr void await_resume() const noexcept {}
    };

    constexpr auto initial_suspend() const noexcept -> std::suspend_always {
        return {};
    }

    constexpr auto final_suspend() noexcept -> TaskFinalAwaiter {
        return {};
    }

    void unhandled_exception() noexcept {
        exception_ = std::move(std::current_exception());
        assert(exception_ != nullptr);
    }

    // 记录当前协程调用者的句柄
    std::coroutine_handle<> caller_{nullptr};
    // 记录错误
    std::exception_ptr exception_{nullptr};
};

template <typename T>
struct TaskPromise final : public TaskPromiseBase {
    auto get_return_object() noexcept -> Task<T>;

    template <typename F>
    requires std::is_convertible_v<F&&, T> && std::is_constructible_v<T, F&&>
    void return_value(F &&value) {
        value_ = std::forward<F>(value);
    }

    auto result() & -> T& {
        if (exception_ != nullptr) [[unlikely]] {
            std::rethrow_exception((exception_));
        }
        assert(value_.has_value());
        return value_.value();
    }

    auto result() && -> T&& {
        if (exception_ != nullptr) [[unlikely]] {
            std::rethrow_exception(exception_);
        }
        assert(value_.has_value());
        return std::move(value_.value());
    }

private:
    std::optional<T> value_{std::nullopt};
};

// 无返回值特化（co_return无返回值）
template <>
struct TaskPromise<void> final : public TaskPromiseBase {
    auto get_return_object() noexcept -> Task<void>;

    constexpr void return_void() const noexcept {};

    void result() const {
        if (exception_ != nullptr) [[unlikely]] {
            std::rethrow_exception(exception_);
        }
    }
};

template <typename T = void>
class [[nodiscard]] Task {
public:
    using promise_type = TaskPromise<T>;

private:
    struct AwaitableBase {
        // 当前协程句柄
        std::coroutine_handle<promise_type> callee_;

        AwaitableBase(std::coroutine_handle<promise_type> callee) noexcept
            : callee_(callee) {}

        auto await_ready() const noexcept -> bool {
            return !callee_ || callee_.done();
        }

        template <typename PromiseType>
        auto await_suspend(std::coroutine_handle<PromiseType> caller) -> std::coroutine_handle<> {
            // 记录当前协程的调用者
            callee_.promise().caller_ = caller;
            return callee_;
        }
    };

public:
    Task() noexcept = default;

    explicit Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

    ~Task() {
        if(handle_) handle_.destroy();
    }

    // 禁止拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // 允许移动
    Task(Task&& other) noexcept : handle_(std::move(other.handle_))  {
        other.handle_ = nullptr;
    }
    Task& operator=(Task&& other) noexcept {
        if (std::addressof(other) != this) [[likely]] {
            if (handle_) { handle_.destroy(); }
            handle_ = std::move(other.handle_);
            other.handle_ = nullptr;
        }
        return *this;
    }

    auto operator co_await() const & noexcept {
        struct Awaitable final : public AwaitableBase {
            decltype(auto) await_resume() {
                if (!this->callee_) [[unlikely]] {
                    throw std::logic_error("m_handle is nullptr");
                }
                return this->callee_.promise().result();
            }
        };
        return Awaitable{ handle_ };
    }

    auto operator co_await() && noexcept {
        struct Awaitable final : public AwaitableBase {
            decltype(auto) await_resume() {
                if (!this->callee_) [[unlikely]] {
                    throw std::logic_error("m_handle is nullptr");
                }
                if constexpr (std::is_same_v<T, void>) {
                    return this->callee_.promise().result();
                } else {
                    return std::move(this->callee_.promise().result());
                }
            }
        };
        return Awaitable{ handle_ };
    };

    auto take() -> std::coroutine_handle<promise_type> {
        if (handle_ == nullptr) [[unlikely]] {
            throw std::logic_error("m_hanlde is nullptr");
        }
        auto res = std::move(handle_);
        handle_ = nullptr;
        return res;
    }

    auto handle() -> std::coroutine_handle<promise_type> {
        return handle_;
    }

    void resume() const {
        handle_.resume();
    }

private:
    std::coroutine_handle<promise_type> handle_{nullptr};
};

template <typename T>
inline auto TaskPromise<T>::get_return_object() noexcept -> Task<T> {
    return Task<T> { std::coroutine_handle<TaskPromise>::from_promise(*this) };
}

inline auto TaskPromise<void>::get_return_object() noexcept -> Task<void> {
    return Task<void> { std::coroutine_handle<TaskPromise>::from_promise(*this) };
}
} // namespace coruring::async