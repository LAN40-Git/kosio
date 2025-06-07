#include "net.h"
#include "log.h"

using namespace coruring::async;
using namespace coruring::log;
using namespace coruring::socket;

auto process(net::TcpStream stream) -> Task<void> {
    char buf[1024];

    while (true) {
        auto ok = co_await stream.read(buf);
        if (!ok) {
            console.error("{} {}", ok.value(), ok.error().message());
            break;
        }
        if (ok.value() == 0) {
            break;
        }

        auto len = ok.value();
        buf[len] = 0;
        console.info("read: {}", buf);

        ok = co_await stream.write({buf, len});
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
        auto has_stream = co_await listener.accept();
        if (has_stream) {
            auto &[stream, peer_addr] = has_stream.value();
            console.info("Accept a connection from {}", peer_addr);

        }

    }
}

void event_loop() {
    auto h = server().take();
    h.resume();
    while (!h.done()) {
        std::array<io_uring_cqe*, 16> cqes;
        auto count = coruring::io::detail::IoUring::instance().peek_batch({cqes.data(), cqes.size()});
        for (auto i = 0u; i < count; ++i) {
            
        }
    }
}

int main() {
    event_loop();
    return 0;
}