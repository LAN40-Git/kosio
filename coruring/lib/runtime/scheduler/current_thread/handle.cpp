#include "runtime/scheduler/current_thread/handle.h"
#include "common/util/thread.h"

coruring::runtime::scheduler::current_thread::Handle::Handle(const runtime::detail::Config &config)
    : worker_(this, config) {
    util::set_current_thread_name("CORURING-WORKER-0");
}

coruring::runtime::scheduler::current_thread::Handle::~Handle() {
    close();
}

void coruring::runtime::scheduler::current_thread::Handle::schedule_task(std::coroutine_handle<> task) {
    tasks_.insert({task, 0});
    worker_.schedule_remote(task);
}

void coruring::runtime::scheduler::current_thread::Handle::close() {
    worker_.shutdown();
}

void coruring::runtime::scheduler::current_thread::Handle::wait() {
    worker_.run();
}
