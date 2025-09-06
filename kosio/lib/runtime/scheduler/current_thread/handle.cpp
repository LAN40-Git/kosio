#include "kosio/include/runtime/scheduler/current_thread/handle.h"
#include "kosio/include/common/util/thread.h"

kosio::runtime::scheduler::current_thread::Handle::Handle(const runtime::detail::Config &config)
    : worker_(this, config) {
    util::set_current_thread_name("kosio-WORKER-0");
}

kosio::runtime::scheduler::current_thread::Handle::~Handle() {
    close();
}

void kosio::runtime::scheduler::current_thread::Handle::schedule_task(std::coroutine_handle<> task) {
    worker_.schedule_remote(task);
}

void kosio::runtime::scheduler::current_thread::Handle::close() {
    worker_.shutdown();
}

void kosio::runtime::scheduler::current_thread::Handle::wait() {
    worker_.run();
}
