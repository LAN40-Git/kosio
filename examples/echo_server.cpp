// an echo server
// ignore all errors
#include "kosio/core.hpp"
#include "kosio/net.hpp"
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