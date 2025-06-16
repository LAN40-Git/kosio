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

auto process(TcpStream stream) -> Task<void> {
    char buf[128];
    while (true) {
        if (auto ret = co_await stream.recv(buf); !ret || ret.value() == 0) {
            break;
        }
        if (auto ret = co_await stream.send_all(response); !ret) {
            break;
        }
    }
}

auto server() -> Task<void> {
    auto has_addr = SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        console.error(has_addr.error().message());
        co_return;
    }
    auto has_listener = TcpListener::bind(has_addr.value());
    if (!has_listener) {
        console.error(has_listener.error().message());
        co_return;
    }
    auto listener = std::move(has_listener.value());
    while (true) {
        auto has_stream = co_await listener.accept();
        if (has_stream) {
            auto &[stream, peer_addr] = has_stream.value();
            console.info("Accept connection from {}-{}", stream.fd(), peer_addr);
            sched.spawn(process(std::move(stream)));
        } else {
            console.error("{}", has_stream.error().message());
            break;
        }
    }
}

int main() {
    sched.spawn(server());
    sched.run();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
}