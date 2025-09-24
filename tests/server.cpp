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
    while (true) {
        co_await time::sleep(3000);
        auto ret = co_await stream.write_vectored(
            std::span<const char>(response.data(), response.size()),
            std::span<const char>(response.data(), response.size()),
            std::span<const char>(response.data(), response.size())
        );

        if (!ret) {
            console.error("{}", ret.error());
            break;
        }
    }
}

auto server() -> Task<> {
    auto has_addr = SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        LOG_ERROR("{}", has_addr.error());
        co_return;
    }

    auto has_listener = TcpListener::bind(has_addr.value());
    if (!has_listener) {
        LOG_ERROR("{}", has_listener.error());
        co_return;
    }

    auto listener = std::move(has_listener.value());
    while (true) {
        auto has_stream = co_await listener.accept();
        if (!has_stream) {
            LOG_ERROR("{}", has_stream.error());
            break;
        }
        auto& [stream, peer_addr] = has_stream.value();
        LOG_INFO("Accept connection from {}", peer_addr);
        spawn(process(std::move(stream)));
    }
}

auto main_loop() -> Task<> {
    spawn(server());
    co_await ctrl_c();
}

auto main() -> int {
    runtime::MultiThreadBuilder::default_create().block_on(main_loop());
}