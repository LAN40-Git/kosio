# Modules

## 定时器（timer）

采用**分层时间轮**实现毫秒级精度的异步超时管理，通过位图标记优化空槽检测，支持动态任务降级和跨层时间对齐，配合双等待模式实现超时事件触发与取消机制。

- 精度为1ms，层级默认为6级，每层64槽位（可通过`runtime/config.h`文件配置）
- 优先递归下放高层任务以避免其被延迟处理
- 事件中存有io操作提交到`io_uring`的数据的指针，若io事件完成，则会将事件存储的此数据指针置空，当事件到期后检测到数据指针为空会立即返回，否则向`io_uring`提交取消请求。

此定时器通过线程局部变量保证线程安全，并且维护成本低（除非任务数量巨大，否则时间复杂度可视为O(1)），能够在最小化资源占用的情况下保证高精度的定时功能。

提供以下功能（示例）：

1. `sleep` 

   ```c++
   // 异步睡眠1s+300ns，后继参数见io_uring_prep_timeout
   co_await coruring::timer::sleep(1, 300, 0, 0);
   ```

2. `timeout`

   ```c++
   // 从套接字中读取消息并设置超时为100ms
   co_await coruring::io::recv(fd, buf, sizeof(buf), 0).set_timeout(100);
   ```

   



## 网络封装（net）

- 使用CRTP模式实现零开销抽象，无动态分配
- 支持IPv4和IPv6地址静态解析，配合`Listener`（用于绑定和监听本地地址并接收客户端连接的抽象类）和C++的`std::expected`简化了原始套接字的操作，并且加强了错误处理。
- 将套接字封装成`Stream`类，提供类似文件的`recv`和`send`等方法，并且可将`Stream`拆分为`Reader`和`Writer`，隔离读写操作，保证安全性并支持优雅关闭。

**优点：**使用方便，API简单

**缺点：**调用`send_all`等方法时会多创建一个协程，在任务数量大时可能引入些许开销。

最简回声服务器示例：

```c++
// ignore all errors
#include "core.h"
#include "net.h"
using namespace coruring::async;
using namespace coruring::socket::net;
using namespace coruring::scheduler;
Scheduler sched{1};

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



## 异步日志（log）

此日志系统采用**多级缓冲区**和智能指针保证资源安全，基于C++20格式化功能，提供可扩展架构并同时支持控制台和文件输出。

- 日志缓冲区（`LogBuffer`）
  - 固定大小的字符数组缓冲区
  - 提供写入、重置、查询容量/大小等操作
  - 使用迭代器跟踪当前写入位置
- 日志文件管理（`LogFile`）
  - 支持按时间和大小滚动日志文件
  - 自动生成带时间戳的日志文件名
  - 内置64KB写缓冲区
  - 可配置最大文件大小（默认10MB）
- 日志级别系统
  - 定义Debug/Info/Warn/Error/Fatal五个级别
- 基础日志框架
  - 使用CRTP模式实现可扩展的日志器基类
  - 支持带源码位置的格式化日志
  - 提供不同级别的日志方法，包括控制台、文件日志方法