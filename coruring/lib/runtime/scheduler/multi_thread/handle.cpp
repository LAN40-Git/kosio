#include "runtime/scheduler/multi_thread/handle.h"
#include "runtime/scheduler/multi_thread/worker.h"
#include "common/util/thread.h"

coruring::runtime::scheduler::multi_thread::Handle::Handle(const runtime::detail::Config &config)
    : shared_(config) {
    for (std::size_t i = 0; i < config.num_workers; i++) {
        std::latch sync{2};
        auto thread_name = "CORURING-WORKER-" + std::to_string(i);
        threads_.emplace_back([i, this, config, &thread_name, &sync]() {
            Worker worker{i, this, config};

            util::set_current_thread_name(thread_name);

            sync.count_down();

            worker.run();
        });
        sync.arrive_and_wait();
    }
}

coruring::runtime::scheduler::multi_thread::Handle::~Handle() {
    shared_.close();
    for (auto &thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void coruring::runtime::scheduler::multi_thread::Handle::schedule_task(std::coroutine_handle<> task) {
    shared_.schedule_remote(task);
}

void coruring::runtime::scheduler::multi_thread::Handle::close() const {
    shared_.close();
}

void coruring::runtime::scheduler::multi_thread::Handle::wait() {
    for (auto &thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
