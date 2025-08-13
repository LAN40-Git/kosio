#pragma once
#include <array>
#include <list>
#include "runtime/config.h"
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
    [[nodiscard]]
    auto next_expiration() const noexcept -> std::optional<Expiration>;
    void poll(uint64_t now);

private:
    [[nodiscard]]
    static auto level_for(uint64_t when) noexcept -> std::size_t;
    [[nodiscard]]
    auto take_entries(Expiration expiration) const noexcept -> EntryList;
    void process_expiration(const Expiration& expiration);

private:
    using Level = std::array<std::unique_ptr<detail::Level>, runtime::detail::NUM_LEVELS>;
    uint64_t  elapsed_{0}; // 自时间轮创建起过去的时间（ms）
    Level     levels_{};   // 分层时间轮的各个层级
    EntryList pending_{};  // 到期的事件
};
} // namespace coruring::runtime::timer::wheel
