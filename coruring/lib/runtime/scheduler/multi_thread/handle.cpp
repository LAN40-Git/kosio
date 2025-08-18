#include "runtime/scheduler/multi_thread/handle.h"
#include <latch>

coruring::runtime::scheduler::multi_thread::Handle::Handle(const runtime::detail::Config &config) {
    for (std::size_t i = 0; i < config.num_threads; i++) {
        std::latch sync{2};
        auto thread_name = std::format("{}-{}", "CORURING-WORKER", i);
        threads_.emplace_back([this, i, &thread_name, &sync]() {
            Worker worker{shared_, i};

            util::set_current_thread_name(thread_name);

            sync.count_down();

            worker.run();
        });
        sync.arrive_and_wait();
    }
}
