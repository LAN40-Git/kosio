#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Worker::Worker(Shared &shared, std::size_t index)
    : index_(index)
    , shared_(shared)
    , driver_(shared.config_) {

}
