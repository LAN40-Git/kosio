# CorUring

 ![C++](https://img.shields.io/badge/standard-C++23-00599C?logo=cplusplus&logoColor=white) ![Linux](https://img.shields.io/badge/platform-linux-dimgray)

## **Introduction**

Inspired by [zedio](https://github.com/8sileus/zedio), I redesigned the **scheduler** and **timer subsystem** to optimize for **low-latency** and **high-concurrency** workloads.

### **Key Improvements**

**Scheduler Optimization**

- Implemented a **work-stealing** multithreaded scheduler with **lock-free task queues**, reducing cross-thread contention.
- Achieved **92% lower P99 latency** (500ms → 40ms) under **10,000 concurrent connections**, with only a **4.2% throughput drop** (480k → 460k QPS).
- Improved **low-concurrency throughput** by **4.3%** (460k → 480k QPS at 2,000 connections), demonstrating balanced efficiency.

## Features

- Multithreaded scheduler with work-stealing load balancing. (reference [zedio](https://github.com/8sileus/zedio))
- Proactor pattern via io_uring for async I/O.
- Zero-cost abstraction:
  - No virtual function overhead
  - No runtime polymorphism
  - No dynamic dispatch



## Example

```c++
// an echo server
// ignore all errors
#include "core.h"
#include "net.h"
using namespace coruring::async;
using namespace coruring::socket::net;
using namespace coruring::scheduler;
Scheduler sched{1}; // single thread

auto process(TcpStream stream) -> Task<void> {
    char buf[128];
    while (true) {
        auto len = co_await stream.recv(buf);
        if (len == 0) {
            break;
        }
        co_await stream.send({buf, len});
    }
}

auto server() -> Task<void> {
	auto addr = SocketAddr::parse("127.0.0.1", 8080).value();
    auto listener = TcpListener::bind(addr).value();
    while (true) {
        auto [stream, peer_addr] = (co_await listener.accept()).value();
        sched.spawn(process(std::move(stream)));
    }
}

int main() {
    sched.spawn(server());
    sched.run();
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
```

