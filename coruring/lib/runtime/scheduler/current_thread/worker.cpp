#include "runtime/scheduler/current_thread/worker.h"
#include "runtime/scheduler/current_thread/handle.h"

coruring::runtime::scheduler::current_thread::Worker::Worker(Handle *handle, const runtime::detail::Config &config)
    : handle_(handle)
    , driver_(config) {
    t_worker = this;
}

void coruring::runtime::scheduler::current_thread::Worker::run() {
    while (!is_shutdown_) [[likely]] {
        tick();

        maintenance();

        
    }
}

void coruring::runtime::scheduler::current_thread::Worker::shutdown() {
    is_shutdown_ = true;
    wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::wake_up() const {
    driver_.wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::schedule_local(std::coroutine_handle<> task) {

}

void coruring::runtime::scheduler::current_thread::Worker::schedule_remote(std::coroutine_handle<> task) {

}

void coruring::runtime::scheduler::current_thread::Worker::tick() {
    tick_ += 1;
}

void coruring::runtime::scheduler::current_thread::Worker::maintenance() {

}

auto coruring::runtime::scheduler::current_thread::Worker::next_task()
-> std::optional<std::coroutine_handle<>> {
    if (tick_ % config_.global_queue_interval == 0) {
        return next_remote_task().or_else([this] {
            return next_local_task();
        });
    } else {
        if (auto task = next_local_task(); task) {
            return task;
        }

        if (global_queue_.empty()) {
            return std::nullopt;
        }

        global_queue_.put_into(local_queue_, runtime::detail::MAX_QUEUE_BATCH_SIZE);
        return next_local_task();
    }
}

auto coruring::runtime::scheduler::current_thread::Worker::next_local_task()
-> std::optional<std::coroutine_handle<>> {

}

auto coruring::runtime::scheduler::current_thread::Worker::next_remote_task()
-> std::optional<std::coroutine_handle<>> {

}

auto coruring::runtime::scheduler::current_thread::Worker::poll() -> bool {

}

void coruring::runtime::scheduler::current_thread::Worker::sleep() {

}
