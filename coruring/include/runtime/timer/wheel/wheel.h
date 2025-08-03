#pragma once
#include <array>
#include <list>
#include "runtime/config.h"
#include "runtime/timer/entry.h"
#include "runtime/timer/wheel/level.h"
#include "common/error.h"
#include "common/util/static_math.h"

namespace coruring::runtime::timer::wheel {
namespace detail {
// 编译期计算出每层时间轮的时间跨度
static constexpr auto compute_precision() {
    std::array<std::size_t, runtime::detail::NUM_LEVELS> precision{};
    precision[0] = 64;
    for (std::size_t i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        precision[i] = precision[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return precision;
}
} // namespace detail

class Wheel : util::Noncopyable {
public:
    Wheel();
    ~Wheel();

public:
    // 时间轮最大时间跨度
    static constexpr uint64_t MAX_DURATION = (1ULL << (6 * runtime::detail::NUM_LEVELS)) - 1;

    // 时间轮每层时间跨度
    static constexpr auto PRECISION = detail::compute_precision();

    // 掩码，X & MASK = X % LEVEL_MULT
    static constexpr std::size_t MASK = runtime::detail::LEVEL_MULT - 1;

    // 位移数，X >> SHIFT = X / LEVEL_MULT
    static constexpr std::size_t SHIFT = std::countr_zero(runtime::detail::LEVEL_MULT);

public:

    [[nodiscard]]
    auto insert(std::unique_ptr<Entry> entry, uint64_t when) noexcept -> std::expected<Entry*, std::error_code>;
    void remove(Entry* entry, uint64_t when) noexcept;
    [[nodiscard]]
    auto next_expiration() const noexcept -> std::optional<uint64_t>;
    void poll() noexcept;

private:
    void handle_expired_entries() noexcept;
    void cascade_entries(std::size_t level) noexcept;

private:
    using Level = std::array<detail::Level, runtime::detail::NUM_LEVELS>;
    uint64_t                 elapsed_{0}; // 自时间轮创建起过去的时间（ms）
    Level                    levels_;     // 分层时间轮的各个层级
    timer::detail::EntryList pending_;    // 到期的事件
};
} // namespace coruring::runtime::timer::wheel
