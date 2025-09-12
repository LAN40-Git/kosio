# Usage

## fs

### File

读取文件的所有内容

```c++
auto ret = co_await kosio::fs::read_to_end<void>("./example");
// 检查是否成功
if (!ret) {...}
```

重命名文件

```c++
// 将当前文件夹中的 "example1" 文件重命名为 "example2"
auto ret = co_await kosio::fs::rename("./example1", "./example2");
// 检查是否成功
if (!ret) {...}
```

删除文件

```c++
auto ret = co_await kosio::fs::remove_file("./example");
// 检查是否成功
if (!ret) {...}
```

创建硬链接

```c++
// 在当前文件夹中为 example 文件创建一个硬链接 example_hard_link
auto ret = co_await kosio::fs::hard_link("./example", "./example_hard_link");
// 检查是否成功
if (!ret) {...}
```

创建符号链接

```c++
// 在当前文件夹中为 example 文件创建一个符号链接 example_sym_link
auto ret = co_await kosio::fs::sym_link("./example", "./example_hard_link");
// 检查是否成功
if (!ret) {...}
```

使用 `kosio::fs::File`

```c++
// TODO
```



### Dir

创建文件夹

```c++
// 在当前文件夹下创建 example 文件夹
auto ret =co_await kosio::fs::create_dir("./example");
// 检查是否成功
if (!ret) {...}
```



## net

此库提供以下工具

- TcpStream
- TcpSocket
- TcpListener
- UdpDatagram



### tcp

获取 TcpListener

```c++
// 解析地址（127.0.0.1:8080）
auto has_addr = kosio::net::SocketAddr::parse("127.0.0.1", 8080);
// 检查是否有效
if (!has_addr) {...}
// 绑定有效的地址
auto has_listener = kosio::net::TcpListener::bind(has_addr.value());
// 检查是否有效
if (!has_listener) {...}
// 获取 TcpListener
auto listener = std::move(has_listener.value());
```

获取到的 TcpListener 已经处于监听状态，可以这样获取一个 TcpStream 以及其对应的地址

```c++
auto ret = co_await listener.accept();
// 检查是否有效
if (!ret) {...}
// 获取 TcpStream 和地址的引用
auto& [stream, peer_addr] = ret.value();
// 获取 TcpStream 的所有权
auto owned_stream = std::move(stream);
// 输出地址（通过对std::formatter特化实现）
LOG_INFO("{}", peer_addr);
```



### udp





## time

睡眠一段时间

```c++
// 睡眠 1s
co_await kosio::time::sleep(1000); // ms
```

睡眠到某个时间点

```c++
// 睡眠到 1s 之后
co_await kosio::time::sleep_until(kosio::util::current_ms() + 1000);
```

为io操作设置超时

```c++
// 为当前的读操作设置 1s 超时
co_await kosio::io::read(...).timeout(1000);
// or
co_await kosio::io::read(...).timeout_at(kosio::util::current_ms() + 1000);
```
