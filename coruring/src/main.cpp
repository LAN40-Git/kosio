#include "net.h"
#include "log.h"
#include "time.h"
using namespace coruring::async;
using namespace coruring::log;
using namespace coruring::socket;

auto process(net::TcpStream stream) -> Task<void> {
    char buf[1024];
    auto [reader, writer] = stream.split();
    while (true) {
        auto ok = co_await reader.read(buf).set_timeout(1000);
        if (!ok) {
            console.error("{}", ok.error().message());
            break;
        }
        if (ok.value() == 0) {
            console.info("Connection closed : {}", reader.local_addr().value());
            break;
        }

        auto len = ok.value();
        buf[len] = 0;
        console.info("read: {}", buf);

        ok = co_await writer.write({buf, len});
        if (!ok) {
            console.error(ok.error().message());
            break;
        }
    }
}

auto server() -> Task<void> {
    // 1. 设置监听地址和端口
    auto has_addr = net::SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        console.error(has_addr.error().message());
        co_return;
    }
    // 2. 开始监听
    auto has_listener = net::TcpListener::bind(has_addr.value());
    if (!has_listener) {
        console.error(has_listener.error().message());
        co_return;
    }
    auto listener = std::move(has_listener.value());
    while (true) {
        // 3. 接收连接
        auto has_stream = co_await listener.accept();
        if (has_stream) {
            auto &[stream, peer_addr] = has_stream.value();
            console.info("Accept a connection from {}", peer_addr);
            // 4. 回声服务
            auto h = process(std::move(stream)).take();
            h.resume();
        } else {
            console.error(has_stream.error().message());
            break;
        }
    }
}

void event_loop() {
    auto h = server().take();
    h.resume();
    while (!h.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        coruring::runtime::Timer::instance().tick();
        std::array<io_uring_cqe*, 16> cqes{};
        auto count = coruring::runtime::detail::IoUring::instance().peek_batch({cqes.data(), cqes.size()});
        for (auto i = 0u; i < count; i++) {
            if (!cqes[i]) {
                continue;
            }
            auto cb = static_cast<coruring::io::detail::Callback*>(io_uring_cqe_get_data(cqes[i]));
            if (!cb) {
                continue;
            }
            if (cb->entry_) {
                cb->entry_->data_ = nullptr;
            }
            cb->result_ = cqes[i]->res;
            cb->handle_.resume();
        }
        if (count > 0) {
            coruring::runtime::detail::IoUring::instance().consume(count);
        }
    }
    h.destroy();
}

int main() {
    event_loop();
    return 0;
}