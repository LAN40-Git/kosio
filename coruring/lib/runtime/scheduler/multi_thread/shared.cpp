#include "runtime/scheduler/multi_thread/shared.h"

coruring::runtime::scheduler::multi_thread::Shared::Shared(const runtime::detail::Config &config)
    : config_(config)
    , idel_(config.num_threads)
    , shutdown_{static_cast<std::ptrdiff_t>(config.num_threads)} {
    t_shared = this;
}

coruring::runtime::scheduler::multi_thread::Shared::~Shared() {
    t_shared = nullptr;
}
