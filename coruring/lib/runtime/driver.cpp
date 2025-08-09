#include "runtime/driver.h"

coruring::runtime::detail::Driver::Driver(const Config &config)
    : ring_(config) {
    assert(t_driver == nullptr);
    t_driver = this;
}

coruring::runtime::detail::Driver::~Driver() {
    t_driver = nullptr;
}
