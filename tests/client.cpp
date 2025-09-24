#include "kosio/core.hpp"
#include "kosio/net.hpp"
#include "kosio/time.hpp"
#include "kosio/log.hpp"
#include "kosio/signal.hpp"

using namespace kosio;
using namespace kosio::net;
using namespace kosio::log;
using namespace kosio::async;
using namespace kosio::signal;

constexpr std::string_view response = "Hello, world\n";

auto process(TcpStream stream) -> Task<> {
    std::array<char, 1024> buffer;

    while (true) {
        auto ret = co_await stream.read_exact({buffer.data(), response.size()});
        if (!ret) {
            console.error("Failed to read response : {}", ret.error());
            break;
        }
        console.info("Received {}", std::string_view(buffer.data(), response.size()));
    }
}

auto main_loop() -> Task<> {
    auto has_addr = SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        LOG_ERROR("{}", has_addr.error());
        co_return;
    }

    auto has_stream = co_await TcpStream::connect(has_addr.value());
    if (!has_stream) {
        LOG_ERROR("{}", has_listener.error());
        co_return;
    }

    spawn(process(std::move(has_stream.value())));
    co_await ctrl_c();
}

auto main() -> int {
    runtime::MultiThreadBuilder::default_create().block_on(main_loop());
}