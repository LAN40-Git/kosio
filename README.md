# kosio

 ![C++](https://img.shields.io/badge/standard-C++23-00599C?logo=cplusplus&logoColor=white) ![Linux](https://img.shields.io/badge/platform-linux-dimgray)

```
  _  __   ___    ____    ___    ___  
 | |/ /  / _ \  / ___|  |_ _|  / _ \ 
 | ' /  | | | | \___ \   | |  | | | |
 | . \  | |_| |  ___) |  | |  | |_| |
 |_|\_\  \___/  |____/  |___|  \___/ 
                                     
```

## 简介

本项目是一个使用 c++ 实现的异步运行时，大量使用 `c++` 编译期编程技术以减少运行时开销，通过任务窃取来实现负载均衡，能够安全的在高并发高负载下高效调度各种 `io` 和定时任务。

本项目参照 [tokio](https://github.com/tokio-rs/tokio) 和 [zedio](https://github.com/8sileus/zedio) 实现，非常感谢 [zedio](https://github.com/8sileus/zedio) 的作者为我解惑。



## 特点

- 具有工作窃取负载平衡的多线程调度器。
- 基于 `io_uring` (为异步 IO 设计的框架) 实现的 `Proactor` 模式。
- 零成本抽象: 无虚函数、无运行时多态、无动态调度。



## Example

```c++
// an echo server
// ignore all errors
#include "core.h"
#include "net.h"
using namespace kosio;
using namespace kosio::async;
using namespace kosio::socket::net;

auto process(TcpStream stream) -> Task<void> {
    char buf[1024]{};
    while (true) {
        auto len = (co_await (stream.read(buf))).value();
        if (len == 0) {
            break;
        }
        co_await stream.write_all({buf, len});
    }
}

auto server() -> Task<void> {
    auto addr = SocketAddr::parse("localhost", 8080).value();
    auto listener = TcpListener::bind(addr).value();
    while (true) {
        auto [stream, addr] = (co_await listener.accept()).value();
        spawn(process(std::move(stream)));
    }
}

auto main() -> int {
    kosio::runtime::MultiThreadBuilder::default_create().block_on(server());
}
```



## 子模块

- **IO**
- **NetWorking**
- **Time**
- **Log**



## Todo Lists

- **完善文档**
- **改进任务调度功能**
- **添加 FIleSystem 模块**
- **添加 Sync 模块**
- **重构 NetWorking 模块**
- **添加数据监控功能**
