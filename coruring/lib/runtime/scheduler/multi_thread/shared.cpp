#include "runtime/scheduler/multi_thread/shared.h"

#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Shared::Shared(const runtime::detail::Config &config)
    : config_(config)
    , idel_(config.num_threads)
    , shutdown_{static_cast<std::ptrdiff_t>(config.num_threads)} {
    t_shared = this;
}

coruring::runtime::scheduler::multi_thread::Shared::~Shared() {
    t_shared = nullptr;
}

void coruring::runtime::scheduler::multi_thread::Shared::wake_up_one() {

}

void coruring::runtime::scheduler::multi_thread::Shared::wake_up_all() const {
    for (auto& worker : workers_) {
        worker->wake_up();
    }
}

void coruring::runtime::scheduler::multi_thread::Shared::close() const {
    wake_up_all();
}

void coruring::runtime::scheduler::multi_thread::Shared::schedule_remote(std::coroutine_handle<> task) {
    global_queue_.enqueue(std::move(task));
}

void coruring::runtime::scheduler::multi_thread::Shared::schedule_remote_batch() {

}
