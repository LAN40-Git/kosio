#pragma once
#include "kosio/runtime/config.hpp"
#include "kosio/common/debug.hpp"
#include "kosio/common/util/noncopyable.hpp"
#include <cstring>
#include <format>
#include <stdexcept>
#include <liburing.h>

namespace kosio::runtime::io {
class IoUring;

inline thread_local IoUring* t_ring{nullptr};

class IoUring : util::Noncopyable {
public:
    explicit IoUring(const detail::Config& config)
    : submit_interval_{static_cast<uint32_t>(config.submit_interval)} {
        if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
            throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
        }
        t_ring = this;
    }

    ~IoUring() {
        t_ring = nullptr;
        io_uring_queue_exit(&ring_);
    }

public:
    [[nodiscard]]
    auto submit_interval() const noexcept -> uint32_t { return submit_interval_; }

    [[nodiscard]]
    auto ring() -> io_uring& { return ring_; }

    [[nodiscard]]
    auto get_sqe() -> io_uring_sqe* { return io_uring_get_sqe(&ring_); }

    [[nodiscard]]
    auto peek_cqe(io_uring_cqe **cqe) -> int {
        return io_uring_peek_cqe(&ring_, cqe);
    }

    [[nodiscard]]
    auto peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
        return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
    }

    // 阻塞等待一个请求完成
    void wait(std::optional<time_t> timeout_ms) {
        io_uring_cqe *cqe{nullptr};
        if (timeout_ms) {
            __kernel_timespec ts{
                .tv_sec = timeout_ms.value() /1000,
                .tv_nsec = timeout_ms.value() % 1000 * 1000'000
            };
            if (auto ret = io_uring_wait_cqe_timeout(&ring_, &cqe, &ts); ret != 0 && ret != -ETIME)
                [[unlikely]] {
                    LOG_ERROR("io_uring_wait_cqe_timeout failed, error: {}", strerror(-ret));
            }
        } else {
            if (auto ret = io_uring_wait_cqe(&ring_, &cqe); ret != 0) [[unlikely]] {
                LOG_ERROR("io_uring_wait_cqe failed, error: {}", strerror(-ret));
            }
        }
    }

    // 收割多个请求
    void consume(std::size_t count) { io_uring_cq_advance(&ring_, count); }

    // 收割一个请求
    void seen(io_uring_cqe *cqe) { io_uring_cqe_seen(&ring_, cqe); }

    // 暂存一个请求
    void pend_submit() {
        submit_tick_++;
        if (submit_tick_ >= submit_interval_) {
            submit();
        }
    }

    // 暂存多个请求
    void pend_submit_batch(std::size_t count) {
        submit_tick_ += count;
        if (submit_tick_ >= submit_interval_) {
            submit();
        }
    }

    // 立即提交请求
    void submit() {
        submit_tick_ = 0;
        if (auto ret = io_uring_submit(&ring_); ret < 0) [[unlikely]] {
            LOG_ERROR("submit sqes failed, {}", strerror(-ret));
        }
    }

private:
    io_uring ring_{};
    uint32_t submit_tick_{0};
    uint32_t submit_interval_;
};
} // namespace kosio::runtime::io
