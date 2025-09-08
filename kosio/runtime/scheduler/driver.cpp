#include "kosio/runtime/scheduler/driver.h"

kosio::runtime::scheduler::detail::Driver::Driver(const runtime::detail::Config &config)
    : ring_(config) {
    assert(t_driver == nullptr);
    t_driver = this;
}

kosio::runtime::scheduler::detail::Driver::~Driver() {
    t_driver = nullptr;
}

void kosio::runtime::scheduler::detail::Driver::wake_up() const {
    waker_.wake_up();
}
