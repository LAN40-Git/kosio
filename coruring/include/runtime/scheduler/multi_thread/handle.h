#pragma once
#include "shared.h"
#include "runtime/driver.h"

namespace coruring::runtime::scheduler::multi_thread {
class Handle {
public:
    Handle();
    ~Handle();

private:
    Shared                   shared_;
    std::vector<std::thread> threads_;
};
} // namespace coruring::runtime::scheduler::multi_thread
