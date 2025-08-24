#include "runtime/scheduler/driver.h"

coruring::runtime::scheduler::detail::Driver::Driver(const runtime::detail::Config &config)
    : ring_(config) {
    assert(t_driver == nullptr);
    t_driver = this;
}

coruring::runtime::scheduler::detail::Driver::~Driver() {
    t_driver = nullptr;
}

void coruring::runtime::scheduler::detail::Driver::wake_up() const {
    waker_.wake_up();
}
