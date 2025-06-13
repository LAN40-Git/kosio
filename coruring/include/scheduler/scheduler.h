#pragma once
#include <set>
#include "runtime/worker/worker.h"

namespace coruring::scheduler
{
class Scheduler : public util::Noncopyable {
    using Worker = std::unique_ptr<runtime::detail::Worker>;
public:
    explicit Scheduler(std::size_t worker_nums)
        : worker_nums_(worker_nums) {}

public:
    void run();
    void stop();
    
private:
    std::atomic<bool> is_running_{false};
    std::mutex        mutex_;
    std::size_t       worker_nums_;
    std::set<Worker>  workers_;
};
}
