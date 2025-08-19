#pragma once
#include "shared.h"
#include "runtime/config.h"
#include "runtime/driver.h"
#include "worker.h"

namespace coruring::runtime::scheduler::multi_thread {
class Handle {
public:
    explicit Handle(const runtime::detail::Config& config);
    ~Handle();

public:


private:
    Shared                   shared_;
    std::vector<std::thread> threads_;
};
} // namespace coruring::runtime::scheduler::multi_thread
