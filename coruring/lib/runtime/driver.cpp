#include "runtime/driver.h"

coruring::runtime::detail::Driver::Driver(const Config &config)
    : ring_(config) {
    assert(t_driver == nullptr);
    t_driver = this;
}

coruring::runtime::detail::Driver::~Driver() {
    t_driver = nullptr;
}

void coruring::runtime::detail::Driver::wait() {
    ring_.wait(timer_.next_expiration_time());
}
