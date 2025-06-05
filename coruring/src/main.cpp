#include "io/io.h"
#include "log/log.h"
#include <thread>
#include <arpa/inet.h>

std::vector<std::coroutine_handle<coruring::async::Task<>::promise_type>> handles(100);

coruring::async::Task<> handle_connection(int fd) {
    char buffer[1024]{};
    while (true) {
        int recv_bytes = co_await coruring::io::recv(fd, buffer, sizeof(buffer), 0);
        if (recv_bytes <= 0) {
            if (recv_bytes == 0) {
                coruring::log::console.info("Client closed connection");
            } else {
                coruring::log::console.error("Recv error: {}", strerror(-recv_bytes));
            }
            break;
        }
        buffer[recv_bytes] = '\0';
        coruring::log::console.info("Received: {}", buffer);

        // 示例：回显消息
        co_await coruring::io::send(fd, buffer, recv_bytes, 0);
    }
    close(fd);
}

coruring::async::Task<> server() {
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9190);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(server_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr));
    listen(server_fd, 5);

    coruring::log::console.info("Listening on port 9190...");

    while (true) {
        sockaddr_in clnt_addr{};
        socklen_t clnt_len = sizeof(clnt_addr);
        int fd = co_await coruring::io::accept(server_fd,
            reinterpret_cast<sockaddr*>(&clnt_addr), &clnt_len, 0);

        if (fd < 0) {
            coruring::log::console.error("Accept error: {}", strerror(-fd));
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clnt_addr.sin_addr, ip, sizeof(ip));
        coruring::log::console.info("New connection from {}", ip);

        // 为每个连接创建独立任务
        auto h = handle_connection(fd).take();
        handles.push_back(h);
        h.resume();
    }

    close(server_fd);
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
                cb->handle_.resume();
            }
            coruring::io::IoUring::instance().seen(cqe);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return 0;
}