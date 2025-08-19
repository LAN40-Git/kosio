#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Worker::Worker(
    std::size_t index,
    Handle* handle,
    const runtime::detail::Config &config)
    : index_(index)
    , handle_(handle)
    , driver_(config) {}
