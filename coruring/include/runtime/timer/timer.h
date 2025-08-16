#pragma once
#include <array>
#include <list>
#include "runtime/config.h"
#include "level.h"
#include "common/error.h"
#include "common/util/time.h"

namespace coruring::runtime::timer {
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
    auto insert(coruring::io::detail::Callback *data, uint64_t expiration_time)
    const noexcept -> Result<Entry*, TimerError>;
    // Remove a timeout entry
    static void remove(Entry* entry) noexcept;
    [[nodiscard]]
    auto next_expiration() const noexcept -> std::optional<Expiration>;
    [[nodiscard]]
    auto next_expiration_time() const noexcept -> std::optional<uint64_t>;
    [[nodiscard]]
    auto handle_expired_entries(uint64_t now) noexcept -> std::size_t;

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
    [[nodiscard]]
    auto process_expiration(const Expiration& expiration) noexcept -> std::size_t;
    [[nodiscard]]
    auto handle_pending_entries() noexcept -> std::size_t;;

private:
    using Level = std::array<std::unique_ptr<detail::Level>, runtime::detail::NUM_LEVELS>;
    uint64_t  start_time_; // 分层时间轮启动时间
    uint64_t  elapsed_;    // 自时间轮创建起过去的时间（ms）
    Level     levels_{};   // 分层时间轮的各个层级
    EntryList pending_{};  // 到期的事件
};
} // namespace coruring::runtime::timer
