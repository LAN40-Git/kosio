#include "kosio/net.hpp"
#include "kosio/core.hpp"
#include "kosio/signal.hpp"
using namespace kosio;
using namespace kosio::net;
using namespace kosio::async;

auto client() -> Task<void> {
    auto addr = SocketAddr::parse("localhost", 8080).value();
    auto has_stream = co_await TcpStream::connect(addr);
    if (!has_stream) {
        LOG_ERROR("{}", has_stream.error());
        co_return;
    }
    LOG_INFO("connect to {}", addr);
    co_await signal::ctrl_c();
}

auto main() -> int {
    kosio::runtime::MultiThreadBuilder::default_create().block_on(client());
}
