#pragma once
#include <array>
#include <list>
#include "runtime/config.h"
#include "runtime/timer/entry.h"
#include "runtime/timer/wheel/level.h"
#include "common/error.h"

namespace coruring::runtime::timer::wheel {
class Wheel : util::Noncopyable {
public:
    Wheel();
    ~Wheel() = default;

public:
    [[nodiscard]]
    auto insert(std::unique_ptr<Entry> entry, uint64_t when) const noexcept
    -> Result<Entry*, TimerError>;
    static void remove(Entry* entry) noexcept;
    auto next_expiration_time() -> std::optional<uint64_t>;

private:
    [[nodiscard]]
    auto level_for(uint64_t remaining_ms) const noexcept -> std::size_t;
    void handle_expired_entries() noexcept;
    void cascade_entries(std::size_t level) noexcept;

private:
    using Level = std::array<std::unique_ptr<detail::Level>, runtime::detail::NUM_LEVELS>;
    uint64_t                 elapsed_{0}; // 自时间轮创建起过去的时间（ms）
    Level                    levels_{};   // 分层时间轮的各个层级
    timer::detail::EntryList pending_{};  // 到期的事件
};
} // namespace coruring::runtime::timer::wheel
