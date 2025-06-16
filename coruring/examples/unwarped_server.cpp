#include "core.h"
#include "net.h"
#include "log.h"
#include "timer.h"
#include <string_view>
using namespace coruring::async;
using namespace coruring::socket::net;
using namespace coruring::log;
using namespace coruring::timer;
using namespace coruring::scheduler;

Scheduler sched{8};

constexpr std::string_view response = R"(
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 13

Hello, World!
)";

auto handle_client(int fd) -> Task<> {
    char buf[128];
    while (true) {
        if (auto ret = co_await coruring::io::recv(fd, buf, sizeof(buf), 0); !ret || ret.value() == 0) {
            break;
        }
        if (auto ret = co_await coruring::io::send(fd, response.data(), response.size(), 0); !ret) {
            break;
        }
    }
    close(fd);
}

auto server(int server_fd) -> Task<> {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    while (true) {
        auto fd = co_await coruring::io::accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len, 0);
        if (!fd) {
            console.error("Failed to accept connection");
            continue;
        }
        sched.spawn(handle_client(fd.value()));
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    listen(server_fd, SOMAXCONN);
    for (std::size_t i = 0; i < 512; ++i) {
        sched.spawn(server(server_fd));
    }
    sched.run();
    int opt;
    while (true) {
        std::cin >> opt;
        sched.stop();
    }
}