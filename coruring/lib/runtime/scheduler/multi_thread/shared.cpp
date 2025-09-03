#include "runtime/scheduler/multi_thread/shared.h"
#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Shared::Shared(const runtime::detail::Config &config)
    : config_(config)
    , idle_(config.num_workers)
    , shutdown_{static_cast<std::ptrdiff_t>(config.num_workers)} {
    t_shared = this;
}

coruring::runtime::scheduler::multi_thread::Shared::~Shared() {
    t_shared = nullptr;
}

void coruring::runtime::scheduler::multi_thread::Shared::wake_up_one() {
    if (auto index = idle_.worker_to_notify(); index) {
        workers_[index.value()]->wake_up();
    }
}

void coruring::runtime::scheduler::multi_thread::Shared::wake_up_all() const {
    for (auto& worker : workers_) {
        worker->wake_up();
    }
}

void coruring::runtime::scheduler::multi_thread::Shared::wake_up_if_work_pending() {
    for (auto& worker : workers_) {
        if (!worker->local_queue_.empty()) {
            worker->wake_up();
            return;
        }
    }
    if (!global_queue_.empty()) {
        wake_up_one();
    }
}

void coruring::runtime::scheduler::multi_thread::Shared::close() const {
    for (auto& worker : workers_) {
        worker->shutdown();
    }
    wake_up_all();
}

void coruring::runtime::scheduler::multi_thread::Shared::schedule_remote(std::coroutine_handle<> task) {
    global_queue_.enqueue(std::move(task));
    wake_up_one();
}
