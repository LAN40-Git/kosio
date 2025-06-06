#include "socket/net/listener.h"
#include "log/log.h"
#include <thread>
#include <arpa/inet.h>

coruring::async::Task<> handle_connection(coruring::socket::Socket&& s) {
    auto socket = std::move(s);
    char buffer[1024];
    __kernel_timespec ts {.tv_sec = 3};
    while (true) {
        auto result = co_await coruring::io::timeout_recv(socket.fd(), buffer, sizeof(buffer), 0, &ts);
        if (result) {
            if (result == 0) {
                coruring::log::console.info("Client closed connection or timeout");
                break;
            }
            buffer[result.value()] = 0;
            std::cout << buffer;
            // 回显消息
            co_await coruring::io::send(socket.fd(), buffer, result.value(), 0);
        } else {
            coruring::log::console.error("Failed to recv: {}", result.error().message());
            break;
        }
    }
}

coruring::async::Task<> server() {
    auto has_addr = coruring::socket::net::SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        coruring::log::console.error(has_addr.error().message());
        co_return;
    }
    auto has_listener = coruring::socket::net::TcpListener::bind(has_addr.value());
    if (!has_listener) {
        coruring::log::console.error("Failed to bind tcp listener");
        co_return;
    }
    auto listener = std::move(has_listener.value());
    coruring::log::console.info("Listening on port 8080");

    while (true) {
        auto has_sock = co_await listener.accept();
        if (!has_sock) {
            coruring::log::console.error("Failed to accept connection");
            continue;
        }
        auto &[socket, peer_addr] = has_sock.value();
        coruring::log::console.info("New connection from {}-{}", peer_addr, socket.fd());

        // 为每个连接创建独立任务
        auto h = handle_connection(std::move(socket)).take();
        h.resume();
    }
    co_await listener.close();
}

int main() {
    auto h = server().take();
    h.resume();

    // 完整的事件循环
    while (!h.done()) {
        io_uring_cqe* cqe = nullptr;
        int ret = coruring::io::IoUring::instance().peek_cqe(&cqe);

        if (ret == 0 && cqe) {
            auto cb = static_cast<coruring::io::Callback*>(io_uring_cqe_get_data(cqe));
            if (cb && cb->handle_) {
                cb->result_ = cqe->res;
                cb->handle_.resume();
            }
            coruring::io::IoUring::instance().seen(cqe);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}