#include "runtime/scheduler/multi_thread/idle.h"

coruring::runtime::scheduler::multi_thread::IdleState::IdleState(
    std::size_t num_workers)
    : state_(num_workers << WORKING_SHIFT) {}

auto coruring::runtime::scheduler::multi_thread::IdleState::num_searching(
    std::memory_order order) const -> std::size_t {
    return state_.load(order) & SEARCHING_MASK;
}

auto coruring::runtime::scheduler::multi_thread::IdleState::num_working(
    std::memory_order order) const -> std::size_t {
    return (state_.load(order) & WORKING_MASK) >> WORKING_SHIFT;
}

auto coruring::runtime::scheduler::multi_thread::IdleState::num_searching_and_working(
    std::memory_order order) -> std::pair<std::size_t, std::size_t> {
    auto state = state_.fetch_add(0, order);
    return {(state & WORKING_MASK) >> WORKING_SHIFT, state & SEARCHING_MASK};
}

void coruring::runtime::scheduler::multi_thread::IdleState::inc_num_searching(std::memory_order order) {
    state_.fetch_add(1, order);
}

auto coruring::runtime::scheduler::multi_thread::IdleState::dec_num_searching() -> bool {
    auto prev = state_.fetch_sub(1, std::memory_order::seq_cst);
    return (prev & SEARCHING_MASK) == 1;
}

void coruring::runtime::scheduler::multi_thread::IdleState::wake_up_one(std::size_t num_searching) {
    // 增加一个工作中线程：(1 << WORKING_SHIFT)
    // 增加 num_searching 个搜索线程
    state_.fetch_add(num_searching | (1 << WORKING_SHIFT), std::memory_order::seq_cst);
}

auto coruring::runtime::scheduler::multi_thread::IdleState::dec_num_working(bool is_searching) -> bool {
    auto dec = 1 << WORKING_SHIFT;
    if (is_searching) {
        dec += 1;
    }
    auto prev = state_.fetch_sub(dec, std::memory_order::seq_cst);
    return is_searching && (prev & SEARCHING_MASK) == 1;
}

coruring::runtime::scheduler::multi_thread::Idle::Idle(std::size_t num_workers)
    : state_(num_workers)
    , num_workers_(num_workers) {}

auto coruring::runtime::scheduler::multi_thread::Idle::worker_to_notify() -> std::optional<std::size_t> {
    if (!notify_should_wakeup()) {
        return std::nullopt;
    }

    std::lock_guard lock(mutex_);

    if (!notify_should_wakeup()) {
        return std::nullopt;
    }

    state_.wake_up_one(1);
    auto result = *sleeping_workers_.begin();
    sleeping_workers_.erase(result);
    return result;
}

auto coruring::runtime::scheduler::multi_thread::Idle::transition_worker_to_sleeping(std::size_t worker,
    bool is_searching) -> bool {
    std::lock_guard lock(mutex_);
    auto result = state_.dec_num_working(is_searching);
    sleeping_workers_.emplace(worker);
    return result;
}

auto coruring::runtime::scheduler::multi_thread::Idle::transition_worker_to_searching() -> bool {
    if (2 * state_.num_searching(std::memory_order::seq_cst) >= num_workers_) {
        return false;
    }
    state_.inc_num_searching(std::memory_order::seq_cst);
    return true;
}

auto coruring::runtime::scheduler::multi_thread::Idle::transition_worker_from_searching() -> bool {
    return state_.dec_num_searching();
}

auto coruring::runtime::scheduler::multi_thread::Idle::remove(std::size_t worker) -> bool {
    std::lock_guard lock(mutex_);
    if (sleeping_workers_.erase(worker) > 0) {
        state_.wake_up_one(0);
        return true;
    }
    return false;
}

auto coruring::runtime::scheduler::multi_thread::Idle::contains(std::size_t worker) -> bool {
    std::lock_guard lock(mutex_);
    return sleeping_workers_.contains(worker);
}

auto coruring::runtime::scheduler::multi_thread::Idle::notify_should_wakeup() -> bool {
    auto [num_searching, num_working] =
        state_.num_searching_and_working(std::memory_order::seq_cst);
    return num_searching == 0 && num_working < num_workers_;
}
