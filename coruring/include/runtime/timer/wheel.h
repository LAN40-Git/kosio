#pragma once
#include <array>
#include <bitset>
#include <list>
#include "runtime/config.h"
#include "common/util/time.h"
#include "common/error.h"
#include "entry.h"

namespace coruring::runtime::detail {
class Wheel : util::Noncopyable {
public:
    static constexpr auto make_precision() -> std::array<uint64_t, NUM_LEVELS> {
        std::array<uint64_t, NUM_LEVELS> precision{};
        precision[0] = 64;
        for (auto i = 1; i < NUM_LEVELS; ++i) {
            precision[i] = precision[i - 1] * 64;
        }
        return precision;
    }

    static constexpr uint64_t MAX_DURATION = (1 << (6 * NUM_LEVELS)) - 1;
    static constexpr auto PRECISION = make_precision();
    static constexpr std::size_t MASK = LEVEL_MULT - 1;
    static constexpr std::size_t SHIFT = std::countr_zero(LEVEL_MULT);

private:
    using EntryList = std::list<std::unique_ptr<Entry>>;
    struct Level {
        std::size_t level{};

        // 最低有效位表示时隙零
        uint64_t occupied{};

        // 槽位，用于存储任务队列
        std::array<EntryList, LEVEL_MULT> slots{};
    };

public:
    [[nodiscard]]
    auto add_entry(std::unique_ptr<Entry> entry, uint64_t remaining_ms) noexcept -> std::expected<Entry*, std::error_code>;
    void remove_entry(Entry* entry, uint64_t remaining_ms) noexcept;
    [[nodiscard]]
    auto next_expiration() const noexcept -> std::optional<uint64_t>;
    void poll() noexcept;

private:
    void handle_expired_entries() noexcept;
    void cascade_entries(std::size_t level) noexcept;

private:
    uint64_t                      elapsed_{0}; // 自时间轮创建起过去的时间（ms）
    std::array<Level, NUM_LEVELS> levels_;     // 时间轮
    EntryList                     pending_;    // 到期的事件
};
} // namespace coruring::runtime::detail
