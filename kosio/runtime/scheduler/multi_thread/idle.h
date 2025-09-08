#pragma once
#include <algorithm>
#include <atomic>
#include <limits>
#include <optional>
#include <unordered_set>
#include <utility>

namespace kosio::runtime::scheduler::multi_thread {
class IdleState {
public:
    explicit IdleState(std::size_t num_workers);
    ~IdleState() = default;

public:
    [[nodiscard]]
    auto num_searching(std::memory_order order) const -> std::size_t;
    [[nodiscard]]
    auto num_working(std::memory_order order) const -> std::size_t;
    [[nodiscard]]
    auto num_working_and_searching(std::memory_order order) -> std::pair<std::size_t, std::size_t>;
    void inc_num_searching(std::memory_order order);
    /// Returns `true` if this is the final searching worker
    [[nodiscard]]
    auto dec_num_searching() -> bool;
    void wake_up_one(std::size_t num_searching);
    /// Returns `true` if this is the final searching worker.
    [[nodiscard]]
    auto dec_num_working(bool is_searching) -> bool;

private:
    static constexpr std::size_t WORKING_SHIFT{16};
    static constexpr std::size_t SEARCHING_MASK{(1 << WORKING_SHIFT) - 1};
    static constexpr std::size_t WORKING_MASK{std::numeric_limits<std::size_t>::max()
                                              ^ SEARCHING_MASK};

    static_assert((WORKING_MASK & SEARCHING_MASK) == 0);
    static_assert((WORKING_MASK | SEARCHING_MASK) == std::numeric_limits<std::size_t>::max());

private:
    /// from https://github.com/8sileus/zedio
    /// --------------------------------
    /// | high 48 bit  | low  16 bit   |
    /// | working num  | searching num |
    /// --------------------------------
    std::atomic<std::size_t> state_;
};

class Idle {
public:
    Idle(std::size_t num_workers);
    ~Idle() = default;

public:
    [[nodiscard]]
    auto worker_to_notify() -> std::optional<std::size_t>;
    [[nodiscard]]
    auto transition_worker_to_sleeping(std::size_t worker, bool is_searching) -> bool;
    [[nodiscard]]
    auto transition_worker_to_searching() -> bool;
    [[nodiscard]]
    auto transition_worker_from_searching() -> bool;
    /// Returns `true` if the worker was sleeped before calling the method.
    [[nodiscard]]
    auto remove(std::size_t worker) -> bool;
    [[nodiscard]]
    auto contains(std::size_t worker) -> bool;

private:
    [[nodiscard]]
    auto notify_should_wakeup() -> bool;

private:
    IdleState                       state_;
    std::size_t                     num_workers_;
    std::unordered_set<std::size_t> sleeping_workers_;
    std::mutex                      mutex_;
};
} // namespace kosio::runtime::scheduler::multi_thread
