#include "kosio/core.hpp"
#include "kosio/log.hpp"
#include "kosio/net.hpp"
#include "kosio/fs.hpp"
#include "kosio/signal.hpp"
#include "kosio/io/buf/stream.hpp"

using namespace kosio::io;
using namespace kosio::fs;
using namespace kosio::net;
using namespace kosio::log;
using namespace kosio::time;
using namespace kosio::async;
using namespace kosio::signal;
using namespace kosio::runtime;

constexpr std::string_view response = R"(
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 13

Hello, World!
)";

auto process(TcpStream stream) -> Task<void> {
    char buf[128];
    auto buf_stream = BufStream{std::move(stream)};
    while (true) {
        if (auto ret = co_await buf_stream.read(buf); !ret || ret.value() == 0) {
            break;
        }
        if (auto ret = co_await buf_stream.write_all(response); !ret) {
            break;
        }
    }
}

auto server() -> Task<void> {
    auto has_addr = SocketAddr::parse("127.0.0.1", 8080);
    if (!has_addr) {
        console.error("Failed to parse IP address.");
        co_return;
    }
    auto has_listener = TcpListener::bind(has_addr.value());
    if (!has_listener) {
        console.error("Failed to bind listener.");
        co_return;
    }
    auto listener = std::move(has_listener.value());
    while (true) {
        if (auto has_stream = co_await listener.accept()) {
            auto &[stream, peer_addr] = has_stream.value();
            // LOG_INFO("Accept a connection from {}", peer_addr);
            kosio::spawn(process(std::move(stream)));
        } else {
            // LOG_ERROR("{}", has_stream.error());
            break;
        }
    }
}

auto main_loop() -> Task<void> {
    kosio::spawn(server());
    co_await ctrl_c();
}

auto main(int argc, char **argv) -> int {
    if (argc != 2) {
        std::cerr << "usage: main num_threas\n";
        return -1;
    }
    SET_LOG_LEVEL(kosio::log::LogLevel::Verbose);
    auto num_threads = std::stoi(argv[1]);
    if (num_threads > 1) {
        MultiThreadBuilder::options()
            .num_workers(num_threads)
            .submit_interval(0)
            .build()
            .block_on(main_loop());
    } else {
        CurrentThreadBuilder::default_create().block_on(main_loop());
    }
    return 0;
}