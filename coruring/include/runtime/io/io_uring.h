#pragma once
#include "runtime/config.h"
#include <cstring>
#include <format>
#include <liburing.h>
#include <stdexcept>
#include <coroutine>
#include <unordered_set>

namespace coruring::runtime::detail
{
class IoUring {
public:
    IoUring(const IoUring&) = delete;
    IoUring& operator=(const IoUring&) = delete;

public:
    // 获取线程局部 io_uring 实例
    static IoUring& instance();

public:
    [[nodiscard]]
    auto submit_interval() const noexcept -> uint32_t { return submit_interval_; }
    [[nodiscard]]
    auto ring() -> io_uring& { return ring_; }
    [[nodiscard]]
    auto get_sqe() -> io_uring_sqe* { return io_uring_get_sqe(&ring_); }
    [[nodiscard]]
    auto peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t;
    [[nodiscard]]
    auto peek_cqe(io_uring_cqe **cqe) -> int;

    // 阻塞等待一个请求完成（无限等待）
    void wait();
    // 阻塞等待一个请求完成（超时等待）
    void wait(long long tv_sec, long long tv_nsec);
    // 收割多个请求
    void consume(std::size_t count) { io_uring_cq_advance(&ring_, count); }
    // 收割一个请求
    void seen(io_uring_cqe *cqe) { io_uring_cqe_seen(&ring_, cqe); }
    // 暂存一个请求
    void pend_submit();
    // 暂存多个请求
    void pend_submit_batch(std::size_t count);
    // 立即提交请求
    void submit();

private:
    explicit IoUring(const runtime::detail::Config& config);
    ~IoUring() {
        io_uring_queue_exit(&ring_);
    }

private:
    struct io_uring ring_{};
    uint32_t submit_tick_{0};
    uint32_t submit_interval_;
};
}
