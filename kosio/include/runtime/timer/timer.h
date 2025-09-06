#pragma once
#include <array>
#include <list>
#include "runtime/config.h"
#include "level.h"
#include "common/error.h"
#include "common/util/time.h"

namespace kosio::runtime::timer {
class Timer;

inline thread_local Timer *t_timer{nullptr};

class Timer : util::Noncopyable {
public:
    Timer();
    ~Timer() = default;
    // Delete move
    Timer(Timer &&) = delete;
    auto operator=(Timer &&) -> Timer& = delete;

public:
    // Add a timeout entry
    [[nodiscard]]
    auto insert(kosio::io::detail::Callback *data, uint64_t expiration_time)
    const noexcept -> Result<Entry*, TimerError>;
    [[nodiscard]]
    auto insert(std::coroutine_handle<> handle, uint64_t expiration_time)
    const noexcept -> Result<Entry*, TimerError>;
    // Remove a timeout entry
    static void remove(Entry* entry) noexcept;
    [[nodiscard]]
    auto next_expiration() const noexcept -> std::optional<Expiration>;
    [[nodiscard]]
    auto next_expiration_time() const noexcept -> std::optional<uint64_t>;
    template <typename LocalQueue, typename GlobalQueue>
    [[nodiscard]]
    auto handle_expired_entries(LocalQueue& local_queue, GlobalQueue& global_queue) noexcept -> std::size_t {
        auto now = util::current_ms() - start_time_;
        std::size_t count = 0;
        while (true) {
            // 处理到期事件
            count += pending_.size();
            while (!pending_.empty()) {
                auto entry = std::move(pending_.front());
                pending_.pop_front();
                entry->execute(local_queue, global_queue);
            }

            // 获取下一个到期信息
            auto expiration = next_expiration();
            // 若到期信息中的最小到期时间 deadline（具体含义可见 level.cpp 中的注释）
            // 大于当前分层时间轮运行的时间，则说明当前已经没有到期事件了，直接退出循环即可
            if (!expiration || expiration.value().deadline > now) {
                break;
            }

            // 处理到期信息
            process_expiration(expiration.value());

            // 推进分层时间轮时间
            elapsed_ = expiration.value().deadline;
        }

        // 推进分层时间轮时间
        elapsed_ = now;
        return count;
    }

public:
    [[nodiscard]]
    auto start_time() const noexcept -> uint64_t;
    [[nodiscard]]
    auto elapsed() const noexcept -> uint64_t;

private:
    [[nodiscard]]
    auto level_for(uint64_t when) const noexcept -> std::size_t;
    [[nodiscard]]
    static auto level_for(uint64_t elapsed, uint64_t when) noexcept -> std::size_t;
    [[nodiscard]]
    auto take_entries(const Expiration& expiration) const noexcept -> EntryList;
    void process_expiration(const Expiration& expiration) noexcept;

private:
    using Level = std::array<std::unique_ptr<detail::Level>, runtime::detail::NUM_LEVELS>;
    uint64_t  start_time_; // 分层时间轮启动时间
    uint64_t  elapsed_;    // 自时间轮创建起过去的时间（ms）
    Level     levels_{};   // 分层时间轮的各个层级
    EntryList pending_{};  // 到期的事件
};
} // namespace kosio::runtime::timer
